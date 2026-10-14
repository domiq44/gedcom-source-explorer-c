#include "gedcom_parser.h"
#include "string_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { TEST_SKIPPED = -1, TEST_FAILED = 0, TEST_PASSED = 1 } TestResult;

// --- Fonctions utilitaires de test ---

/**
 * @brief Teste l'égalité de deux chaînes de caractères.
 * @param actual La chaîne résultante.
 * @param expected La chaîne attendue.
 * @param test_name Nom du test.
 * @return int 1 si succès, 0 si échec.
 */
static int assert_string_equal(const char *actual, const char *expected,
                               const char *test_name) {
    if (actual == NULL || expected == NULL) {
        printf("[FAIL] %s - Erreur de NULL dans les entrées.\n", test_name);
        return 0;
    }
    if (strcmp(actual, expected) == 0) {
        printf("[PASS] %s\n", test_name);
        return 1;
    } else {
        printf("[FAIL] %s - Attendu: '%s', Obtenu: '%s'\n", test_name, expected,
               actual);
        return 0;
    }
}

/**
 * @brief Teste le comportement de la fonction parse_gedcom.
 * @param filename Le fichier à tester.
 * @param expected_count Le nombre attendu de sources.
 * @return int 1 si le test passe, 0 sinon.
 */
static int run_parser_test(const char *filename, int expected_count) {
    /* Given */
    static const char *expected_values[] = {"@S1@", "@S2@", "@S3@"};
    const int expected_parse_result = 0;
    SourceList sources;
    RepositoryList repositories;
    init_source_list(&sources);
    init_repository_list(&repositories);
    int passed = 0;

    /* When */
    int result = parse_gedcom(filename, &sources, &repositories);

    /* Then */
    if (result == expected_parse_result) {
        if ((int)sources.count == expected_count) {
            int values_ok = 1;
            size_t i;

            for (i = 0U; i < (size_t)expected_count && i < 3U; i++) {
                values_ok &= assert_string_equal(
                    sources.sources[i].description, expected_values[i],
                    "Vérification de la source extraite");
            }

            if (values_ok) {
                printf("[PASS] Nombre de sources correct (%d) pour '%s'.\n",
                       expected_count, filename);
                passed = 1;
            } else {
                printf("[FAIL] Valeurs de sources incohérentes pour '%s'.\n",
                       filename);
            }
        } else {
            printf("[FAIL] Nombre de sources incorrect pour '%s'. Attendu: %d, "
                   "Obtenu: %zu.\n",
                   filename, expected_count, sources.count);
        }
    } else {
        printf("[FAIL] parse_gedcom a retourné une erreur (%d) pour '%s'.\n",
               result, filename);
    }

    free_source_list(&sources);
    free_repository_list(&repositories);
    return passed;
}

/* Vérifie l'extraction des entités SOUR d'une fixture GEDCOM. */
static int test_embedded_parser_fixture(void) {
    /* Given */
    const int expected_source_count = 3;
    static const char fixture[] = "0 HEAD\n"
                                  "1 CHAR ANSI\n"
                                  "0 @S1@ SOUR\n"
                                  "0 @S2@ SOUR\n"
                                  "0 @S3@ SOUR\n"
                                  "0 TRLR\n";
    const char *filename = "tmp_parser_input.ged";
    FILE *file = fopen(filename, "w");
    int passed;

    /* When */
    if (file == NULL) {
        printf("[FAIL] Impossible de créer le fichier temporaire du test "
               "parser.\n");
        return 0;
    }

    if (fputs(fixture, file) == EOF) {
        printf("[FAIL] Impossible d'écrire la fixture GEDCOM temporaire.\n");
        fclose(file);
        remove(filename);
        return 0;
    }

    if (fclose(file) != 0) {
        printf("[FAIL] Impossible de fermer la fixture GEDCOM temporaire.\n");
        remove(filename);
        return 0;
    }

    /* Then */
    passed = run_parser_test(filename, expected_source_count);
    remove(filename);
    return passed;
}

/**
 * @brief Teste l'isolation des ressources (cleanup).
 * @param filename Le fichier à tester.
 * @return int 1 si le test passe, 0 sinon.
 */
/* Vérifie qu'un fichier absent produit une erreur d'ouverture. */
static int test_file_not_found(void) {
    /* Given */
    const GedcomParseErrorCode expected_error_code = GEDCOM_PARSE_ERROR_OPEN;
    const size_t expected_line_number = 0U;
    const char *filename = "non_existent_file_xyz.ged";
    printf("\n--- Test Fichier Inexistant ---\n");
    SourceList sources;
    RepositoryList repositories;
    GedcomParseError error;
    init_source_list(&sources);
    init_repository_list(&repositories);
    /* When */
    int result =
        parse_gedcom_with_error(filename, &sources, &repositories, &error);

    /* Then */
    if (result != 0 && error.code == expected_error_code &&
        error.line_number == expected_line_number) {
        printf("[PASS] Détection d'erreur de fichier inexistant réussie.\n");
        free_source_list(&sources);
        free_repository_list(&repositories);
        return 1;
    } else {
        printf("[FAIL] parse_gedcom n'a pas signalé l'erreur de fichier "
               "inexistant.\n");
        free_source_list(&sources);
        free_repository_list(&repositories);
        return 0;
    }
}

/* Vérifie qu'un fichier GEDCOM vide est accepté sans résultat. */
static int test_empty_gedcom_file(void) {
    /* Given */
    const int expected_parse_result = 0;
    const size_t expected_source_count = 0U;
    const size_t expected_repository_count = 0U;
    printf("\n--- Test Fichier GEDCOM Vide ---\n");

    const char *filename = "tmp_empty_gedcom.ged";
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("[FAIL] Impossible de créer le fichier GEDCOM vide.\n");
        return 0;
    }

    if (fclose(file) != 0) {
        printf("[FAIL] Impossible de fermer le fichier GEDCOM vide.\n");
        remove(filename);
        return 0;
    }

    SourceList sources;
    RepositoryList repositories;
    init_source_list(&sources);
    init_repository_list(&repositories);
    /* When */
    int result = parse_gedcom(filename, &sources, &repositories);

    /* Then */
    int passed = result == expected_parse_result &&
                 sources.count == expected_source_count &&
                 repositories.count == expected_repository_count;

    if (passed) {
        printf("[PASS] Un fichier vide est accepté sans source.\n");
    } else {
        printf("[FAIL] Résultat inattendu pour un fichier vide (code %d, %zu "
               "sources).\n",
               result, sources.count);
    }

    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(filename);
    return passed;
}

/**
 * @brief Vérifie qu'un mot SOUR dans un texte non-taggué n'est pas détecté
 * comme source.
 */
/* Vérifie qu'un mot SOUR dans une valeur n'est pas pris pour un tag. */
static int test_false_positive_tag_detection(void) {
    /* Given */
    const size_t expected_source_count = 2U;
    const int expected_parse_result = 0;
    const char *expected_first_description = "@S1@";
    const char *expected_second_description = "Source Name";
    printf("\n--- Test Faux Positif de Tag ---\n");

    const char *filename = "tmp_false_positive.ged";
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf(
            "[FAIL] Impossible de créer le fichier de test de faux positif.\n");
        return 0;
    }

    fputc(0xEF, file);
    fputc(0xBB, file);
    fputc(0xBF, file);
    fprintf(file, "0 HEAD\n");
    fprintf(file, "1 SOUR unregistered\n");
    fprintf(file, "1 NOTE SOUR should be ignored because it is just text\n");
    fprintf(file, "0 @I1@ INDI\n");
    fprintf(file, "1 SOUR @S1@\n");
    fprintf(file, "0 @I2@ INDI\n");
    fprintf(file, "1 SOUR Source Name\n");
    fprintf(file, "0 TRLR\n");
    fclose(file);

    SourceList sources;
    RepositoryList repositories;
    init_source_list(&sources);
    init_repository_list(&repositories);
    /* When */
    int result = parse_gedcom(filename, &sources, &repositories);
    int ok = 0;

    /* Then */
    if (result == expected_parse_result &&
        sources.count == expected_source_count &&
        strcmp(sources.sources[0].description, expected_first_description) ==
            0 &&
        strcmp(sources.sources[1].description, expected_second_description) ==
            0) {
        ok = 1;
        printf("[PASS] Détection exacte du tag SOUR sans faux positif.\n");
    } else {
        printf("[FAIL] Faux positif détecté ou compteur incorrect. Attendu: 2 "
               "sources, obtenu: %zu.\n",
               sources.count);
    }

    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(filename);
    return ok;
}

/* Vérifie la distinction entre entité source et citation avec contexte. */
static int test_source_entity_and_citation_context(void) {
    /* Given */
    const size_t expected_source_count = 2U;
    const int expected_parse_result = 0;
    const char *expected_entity_kind = "entity";
    const char *expected_citation_kind = "citation";
    const char *expected_reference = "@S1@";
    const char *expected_title = "Source principale";
    const char *expected_context = "@I1@ INDI";
    printf("\n--- Test Entité Source et Citation ---\n");

    const char *filename = "tmp_source_citation.ged";
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("[FAIL] Impossible de créer le fichier de test d'entité et de "
               "citation.\n");
        return 0;
    }

    fprintf(file, "0 @S1@ SOUR\n");
    fprintf(file, "1 TITL Source principale\n");
    fprintf(file, "0 @I1@ INDI\n");
    fprintf(file, "1 NAME Jean /DOE/\n");
    fprintf(file, "1 SOUR @S1@\n");
    fprintf(file, "0 TRLR\n");
    fclose(file);

    SourceList sources;
    RepositoryList repositories;
    init_source_list(&sources);
    init_repository_list(&repositories);
    /* When */
    int result = parse_gedcom(filename, &sources, &repositories);
    int ok = 0;

    /* Then */
    if (result == expected_parse_result &&
        sources.count == expected_source_count &&
        strcmp(sources.sources[0].kind, expected_entity_kind) == 0 &&
        strcmp(sources.sources[0].reference, expected_reference) == 0 &&
        sources.sources[0].title != NULL &&
        strcmp(sources.sources[0].title, expected_title) == 0 &&
        strcmp(sources.sources[1].kind, expected_citation_kind) == 0 &&
        strcmp(sources.sources[1].reference, expected_reference) == 0 &&
        strcmp(sources.sources[1].context, expected_context) == 0) {
        ok = 1;
        printf("[PASS] Distinction entre entité source et citation avec "
               "contexte.\n");
    } else {
        printf(
            "[FAIL] Entité/citation/context incorrects. Obtenu: %zu entrées.\n",
            sources.count);
    }

    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(filename);
    return ok;
}

/* Vérifie le contexte direct d'une citation GEDCOM imbriquée. */
static int test_nested_citation_context(void) {
    /* Given */
    const size_t expected_source_count = 1U;
    const int expected_parse_result = 0;
    const char *expected_context = "@I1@ INDI / BIRT";
    const char *expected_excerpt = "2 SOUR @S1@\n3 PAGE 12\n";
    static const char fixture[] = "0 @I1@ INDI\n"
                                  "1 BIRT\n"
                                  "2 DATE 1900\n"
                                  "2 SOUR @S1@\n"
                                  "3 PAGE 12\n"
                                  "0 TRLR\n";
    const char *filename = "tmp_nested_citation_context.ged";
    FILE *file = fopen(filename, "w");
    int write_status;
    int close_status;
    int ok = 0;

    if (file == NULL) {
        printf("[FAIL] Impossible de créer le fichier de contexte imbriqué.\n");
        return 0;
    }
    write_status = fputs(fixture, file);
    close_status = fclose(file);
    if (write_status == EOF || close_status != 0) {
        printf("[FAIL] Impossible d'écrire le fichier de contexte imbriqué.\n");
        remove(filename);
        return 0;
    }

    SourceList sources;
    RepositoryList repositories;
    init_source_list(&sources);
    init_repository_list(&repositories);
    /* When */
    int parse_result = parse_gedcom(filename, &sources, &repositories);

    /* Then */
    if (parse_result == expected_parse_result &&
        sources.count == expected_source_count &&
        strcmp(sources.sources[0].context, expected_context) == 0 &&
        strstr(sources.sources[0].gedcom.text, expected_excerpt) != NULL) {
        ok = 1;
        printf(
            "[PASS] Le contexte imbriqué utilise le parent GEDCOM direct.\n");
    } else {
        printf("[FAIL] Le contexte imbriqué ne correspond pas au parent GEDCOM "
               "direct.\n");
    }

    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(filename);
    return ok;
}

/* Vérifie qu'une erreur de lecture est distinguée d'une fin de fichier. */
static TestResult test_read_error(void) {
    /* Given */
    const GedcomParseErrorCode expected_error_code = GEDCOM_PARSE_ERROR_READ;
    const int expected_parse_failure = 1;
#if defined(__linux__)
    SourceList sources;
    RepositoryList repositories;
    FILE *directory;
    char probe[2];
    int result;
    GedcomParseError error;

    directory = fopen(".", "r");
    if (directory == NULL) {
        printf("[SKIP] Le répertoire courant ne peut pas être ouvert en "
               "lecture.\n");
        return TEST_SKIPPED;
    }
    if (fgets(probe, sizeof(probe), directory) != NULL || !ferror(directory)) {
        fclose(directory);
        printf("[SKIP] La lecture d'un répertoire ne produit pas d'erreur sur "
               "cette plateforme.\n");
        return TEST_SKIPPED;
    }
    fclose(directory);

    init_source_list(&sources);
    init_repository_list(&repositories);
    /* When */
    result = parse_gedcom_with_error(".", &sources, &repositories, &error);
    free_source_list(&sources);
    free_repository_list(&repositories);
    /* Then */
    if ((result != 0) == expected_parse_failure &&
        error.code == expected_error_code) {
        printf("[PASS] Une erreur de lecture est signalée au lieu d'un EOF.\n");
        return TEST_PASSED;
    }
    printf("[FAIL] Une erreur de lecture a été traitée comme une fin de "
           "fichier.\n");
    return TEST_FAILED;
#else
    printf(
        "[SKIP] Le test de lecture d'un répertoire est spécifique à Linux.\n");
    return TEST_SKIPPED;
#endif
}

/* Vérifie qu'une ligne longue est conservée sans troncature. */
static int test_long_gedcom_line(void) {
    /* Given */
    const size_t expected_source_count = 1U;
    const int expected_parse_result = 0;
    printf("\n--- Test Ligne GEDCOM Très Long ---\n");

    const char *filename = "tmp_long_line.ged";
    FILE *file = fopen(filename, "w");
    char long_title[4096];
    size_t index;
    int ok = 0;

    if (file == NULL) {
        printf(
            "[FAIL] Impossible de créer le fichier de test de ligne longue.\n");
        return 0;
    }

    for (index = 0U; index < sizeof(long_title) - 1U; index++) {
        long_title[index] = (index % 10U == 0U) ? 'A' : 'B';
    }
    long_title[sizeof(long_title) - 1U] = '\0';

    fprintf(file, "0 HEAD\n");
    fprintf(file, "0 @S1@ SOUR\n");
    fprintf(file, "1 TITL %s\n", long_title);
    fprintf(file, "0 TRLR\n");
    fclose(file);

    /* When */
    {
        SourceList sources;
        RepositoryList repositories;
        init_source_list(&sources);
        init_repository_list(&repositories);

        /* When */
        int parse_result = parse_gedcom(filename, &sources, &repositories);

        /* Then */
        if (parse_result == expected_parse_result &&
            sources.count == expected_source_count &&
            sources.sources[0].title != NULL &&
            strcmp(sources.sources[0].title, long_title) == 0) {
            ok = 1;
            printf("[PASS] Une ligne GEDCOM longue est conservée sans "
                   "troncature.\n");
        } else {
            printf(
                "[FAIL] Une ligne GEDCOM longue a été tronquée ou ignorée.\n");
        }

        free_source_list(&sources);
        free_repository_list(&repositories);
    }

    remove(filename);
    return ok;
}

/* Vérifie le rejet des niveaux GEDCOM invalides. */
static int test_invalid_gedcom_levels(void) {
    /* Given */
    const GedcomParseErrorCode expected_error_code =
        GEDCOM_PARSE_ERROR_INVALID_LEVEL;
    const size_t expected_error_line = 2U;
    const int expected_parse_success = 0;
    static const char *invalid_lines[] = {
        "999999999999999999999999999999999999 SOUR @S1@\n", "100 SOUR @S1@\n",
        "1x SOUR @S1@\n"};
    const char *filename = "tmp_invalid_gedcom_level.ged";
    size_t index;

    for (index = 0U; index < sizeof(invalid_lines) / sizeof(invalid_lines[0]);
         index++) {
        FILE *file = fopen(filename, "w");
        SourceList sources;
        RepositoryList repositories;
        int write_status;
        int close_status;
        int parse_status;
        GedcomParseError error;

        if (file == NULL) {
            printf(
                "[FAIL] Impossible de créer le fichier de niveau invalide.\n");
            return 0;
        }

        write_status = fputs("0 @I1@ INDI\n", file);
        if (write_status != EOF && fputs(invalid_lines[index], file) == EOF) {
            write_status = EOF;
        }
        close_status = fclose(file);
        if (write_status == EOF || close_status != 0) {
            printf("[FAIL] Impossible d'écrire le cas de niveau invalide.\n");
            remove(filename);
            return 0;
        }

        init_source_list(&sources);
        init_repository_list(&repositories);
        /* When */
        parse_status =
            parse_gedcom_with_error(filename, &sources, &repositories, &error);
        /* Then */
        free_source_list(&sources);
        free_repository_list(&repositories);
        if (parse_status == expected_parse_success ||
            error.code != expected_error_code ||
            error.line_number != expected_error_line) {
            printf(
                "[FAIL] Le cas de niveau invalide %zu n'est pas diagnostiqué "
                "à la ligne 2.\n",
                index + 1U);
            remove(filename);
            return 0;
        }
    }

    remove(filename);
    printf("[PASS] Dépassement, profondeur excessive et niveau mal formé "
           "rejetés.\n");
    return 1;
}

/* Vérifie qu'un échec d'allocation est rapporté avec sa ligne. */
static int test_memory_allocation_error(void) {
    /* Given */
    const GedcomParseErrorCode expected_error_code = GEDCOM_PARSE_ERROR_MEMORY;
    const size_t expected_error_line = 1U;
    const int expected_parse_failure = 1;
    static const char fixture[] = "0 @S1@ SOUR\n";
    const char *filename = "tmp_memory_failure.ged";
    FILE *file = fopen(filename, "w");
    int write_status;
    int close_status;
    SourceList sources;
    RepositoryList repositories;
    GedcomParseError error;
    int result;

    if (file == NULL) {
        printf("[FAIL] Impossible de créer le fixture d'allocation\n");
        return 0;
    }
    write_status = fputs(fixture, file);
    close_status = fclose(file);
    if (write_status == EOF || close_status != 0) {
        printf("[FAIL] Impossible de créer le fixture d'allocation\n");
        remove(filename);
        return 0;
    }

    init_source_list(&sources);
    init_repository_list(&repositories);
    string_utils_test_fail_after(0U);
    /* When */
    result = parse_gedcom_with_error(filename, &sources, &repositories, &error);
    string_utils_test_reset_failure();

    /* Then */
    if ((result != 0) == expected_parse_failure &&
        error.code == expected_error_code &&
        error.line_number == expected_error_line) {
        printf("[PASS] Un échec d'allocation est rapporté avec sa ligne.\n");
    } else {
        printf("[FAIL] L'échec d'allocation n'est pas diagnostiqué "
               "correctement.\n");
        free_source_list(&sources);
        free_repository_list(&repositories);
        remove(filename);
        return 0;
    }

    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(filename);
    return 1;
}

/* Vérifie qu'un chemin GEDCOM vide est rejeté. */
static int test_empty_filename(void) {
    /* Given */
    const int expected_rejected = 1;
    SourceList sources;
    RepositoryList repositories;
    int rejected;

    /* When */
    init_source_list(&sources);
    init_repository_list(&repositories);
    rejected = parse_gedcom("", &sources, &repositories) != 0;

    /* Then */
    if (rejected == expected_rejected) {
        printf(
            "[PASS] Une chaîne vide de chemin est rejetée par le parseur.\n");
        free_source_list(&sources);
        free_repository_list(&repositories);
        return 1;
    }
    printf("[FAIL] Un chemin vide n'a pas été rejeté par le parseur.\n");
    free_source_list(&sources);
    free_repository_list(&repositories);
    return 0;
}

/* Vérifie qu'une citation sans source parente est conservée. */
static int test_orphan_citation(void) {
    /* Given */
    const int expected_orphan_parse_result = 0;
    const size_t expected_orphan_count = 1U;
    const char *expected_orphan_kind = "citation";
    const char *expected_orphan_reference = "@S999@";
    const char *filename = "tmp_orphan_citation.ged";
    static const char fixture[] = "0 @I1@ INDI\n"
                                  "1 SOUR @S999@\n"
                                  "0 TRLR\n";
    SourceList sources;
    RepositoryList repositories;
    FILE *file;
    int write_status;
    int close_status;
    int parse_result;
    int passed;

    /* When */
    init_source_list(&sources);
    init_repository_list(&repositories);
    file = fopen(filename, "w");
    if (file == NULL) {
        printf("[FAIL] Impossible de créer le fichier de test de citation "
               "orpheline.\n");
        free_source_list(&sources);
        free_repository_list(&repositories);
        return 0;
    }
    write_status = fputs(fixture, file);
    close_status = fclose(file);
    if (write_status == EOF || close_status != 0) {
        printf("[FAIL] Impossible d'écrire le fichier de test de citation "
               "orpheline.\n");
        free_source_list(&sources);
        free_repository_list(&repositories);
        remove(filename);
        return 0;
    }
    parse_result = parse_gedcom(filename, &sources, &repositories);

    /* Then */
    passed =
        parse_result == expected_orphan_parse_result &&
        sources.count == expected_orphan_count &&
        strcmp(sources.sources[0].kind, expected_orphan_kind) == 0 &&
        strcmp(sources.sources[0].reference, expected_orphan_reference) == 0;
    if (passed) {
        printf("[PASS] Une citation orpheline est conservée sans source "
               "parente.\n");
    } else {
        printf("[FAIL] La citation orpheline est perdue ou mal typée.\n");
    }

    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(filename);
    return passed;
}

/* Vérifie les dépôts, leurs liens et les extraits par enregistrement. */
static int test_repository_records(void) {
    /* Given */
    const size_t expected_source_count = 3U;
    const size_t expected_repository_count = 2U;
    const size_t expected_link_count = 2U;
    const size_t expected_minimum_excerpt_length = 128U;
    const int expected_parse_result = 0;
    const char *expected_first_repository_reference = "@R1@";
    const char *expected_first_repository_name = "Archives Municipales";
    const char *expected_second_repository_reference = "@R2@";
    const char *expected_second_repository_name = "Le Bistrot du Coin";
    const char *expected_first_link_source_reference = "@S1@";
    const char *expected_first_link_repository_reference = "@R1@";
    const char *expected_link_context = "@S1@ SOUR / REPO";
    const char *expected_second_link_source_reference = "@S1@";
    const char *expected_second_link_repository_reference = "@R2@";
    const char *expected_first_citation_reference = "@S1@";
    const char *expected_orphan_citation_reference = "@S9@";
    const char *expected_source_start = "0 @S1@ SOUR\n";
    const char *expected_source_continuation =
        "2 CONT Première partie du titre\n";
    const char *expected_repository_media = "3 MEDI Livre\n";
    const char *unexpected_individual_record = "0 @I1@ INDI\n";
    const char *expected_citation_excerpt =
        "1 SOUR @S1@\n2 PAGE Page 12\n3 NOTE Mention manuscrite\n";
    const char *unexpected_second_individual = "0 @I2@ INDI\n";
    const char *expected_orphan_excerpt =
        "1 SOUR @S9@\n2 PAGE Référence orpheline\n";
    const char *expected_repository_start =
        "0 @R1@ REPO\n1 NAME Archives Municipales\n";
    const char *expected_repository_city = "2 CITY Saint-Denis\n";
    const char *unexpected_second_repository = "0 @R2@ REPO\n";
    printf("\n--- Test Enregistrements Dépôt ---\n");

    static const char fixture[] =
        "0 @S1@ SOUR\n"
        "1 TITL Source principale\n"
        "2 CONT Première partie du titre\n"
        "1 NOTE Note détaillée sur la source\n"
        "2 CONT Informations supplémentaires sur le document\n"
        "1 REPO @R1@\n"
        "2 CALN Étagère 4\n"
        "3 MEDI Livre\n"
        "1 REPO @R2@\n"
        "0 @I1@ INDI\n"
        "1 SOUR @S1@\n"
        "2 PAGE Page 12\n"
        "3 NOTE Mention manuscrite\n"
        "0 @I2@ INDI\n"
        "1 SOUR @S9@\n"
        "2 PAGE Référence orpheline\n"
        "0 @R1@ REPO\n"
        "1 NAME Archives Municipales\n"
        "1 ADDR Mairie annexe\n"
        "2 CITY Saint-Denis\n"
        "1 NOTE Horaires d'ouverture\n"
        "0 @R2@ REPO\n"
        "1 NAME Le Bistrot du Coin\n"
        "0 TRLR\n";
    const char *filename = "tmp_repository_records.ged";
    FILE *file = fopen(filename, "w");
    int write_status;
    int close_status;
    if (file == NULL) {
        printf("[FAIL] Impossible de créer le fichier de test des dépôts.\n");
        return 0;
    }

    write_status = fputs(fixture, file);
    close_status = fclose(file);
    if (write_status == EOF || close_status != 0) {
        printf("[FAIL] Impossible d'écrire le fichier de test des dépôts.\n");
        remove(filename);
        return 0;
    }

    SourceList sources;
    RepositoryList repositories;
    init_source_list(&sources);
    init_repository_list(&repositories);
    /* When */
    int result = parse_gedcom(filename, &sources, &repositories);

    /* Then */
    int passed =
        result == expected_parse_result &&
        sources.count == expected_source_count &&
        repositories.count == expected_repository_count &&
        repositories.link_count == expected_link_count &&
        strcmp(repositories.repositories[0].reference,
               expected_first_repository_reference) == 0 &&
        strcmp(repositories.repositories[0].name,
               expected_first_repository_name) == 0 &&
        strcmp(repositories.repositories[1].reference,
               expected_second_repository_reference) == 0 &&
        strcmp(repositories.repositories[1].name,
               expected_second_repository_name) == 0 &&
        strcmp(repositories.links[0].source_reference,
               expected_first_link_source_reference) == 0 &&
        strcmp(repositories.links[0].repository_reference,
               expected_first_link_repository_reference) == 0 &&
        strcmp(repositories.links[0].context, expected_link_context) == 0 &&
        strcmp(repositories.links[1].source_reference,
               expected_second_link_source_reference) == 0 &&
        strcmp(repositories.links[1].repository_reference,
               expected_second_link_repository_reference) == 0 &&
        strcmp(sources.sources[1].reference,
               expected_first_citation_reference) == 0 &&
        strcmp(sources.sources[2].reference,
               expected_orphan_citation_reference) == 0 &&
        strstr(sources.sources[0].gedcom.text, expected_source_start) != NULL &&
        strstr(sources.sources[0].gedcom.text, expected_source_continuation) !=
            NULL &&
        strstr(sources.sources[0].gedcom.text, expected_repository_media) !=
            NULL &&
        strstr(sources.sources[0].gedcom.text, unexpected_individual_record) ==
            NULL &&
        sources.sources[0].gedcom.length > expected_minimum_excerpt_length &&
        strstr(sources.sources[1].gedcom.text, expected_citation_excerpt) !=
            NULL &&
        strstr(sources.sources[1].gedcom.text, unexpected_second_individual) ==
            NULL &&
        strstr(sources.sources[2].gedcom.text, expected_orphan_excerpt) !=
            NULL &&
        strstr(repositories.repositories[0].gedcom.text,
               expected_repository_start) != NULL &&
        strstr(repositories.repositories[0].gedcom.text,
               expected_repository_city) != NULL &&
        strstr(repositories.repositories[0].gedcom.text,
               unexpected_second_repository) == NULL;

    if (passed) {
        printf("[PASS] Références et noms des dépôts extraits.\n");
    } else {
        printf(
            "[FAIL] Extraction incorrecte des dépôts (code %d, %zu dépôts).\n",
            result, repositories.count);
    }

    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(filename);
    return passed;
}

/* Vérifie que l'initialisation crée une liste de sources vide. */
static int test_source_list_initial_state(void) {
    /* Given */
    const size_t expected_source_count = 0U;
    SourceList sources;

    /* When */
    init_source_list(&sources);

    /* Then */
    if (sources.count == expected_source_count && sources.sources == NULL) {
        printf("[PASS] Initialisation de SourceList réussie.\n");
        free_source_list(&sources);
        return 1;
    }
    printf("[FAIL] Initialisation de SourceList a échoué.\n");
    free_source_list(&sources);
    return 0;
}

/* Exécute l'ensemble des tests unitaires du parseur GEDCOM. */
int main(void) {
    printf("==================================================\n");
    printf("Démarrage des tests unitaires du parser GEDCOM\n");
    printf("==================================================\n");

    int total_passed = 0;
    int total_tests = 0;

    // --- Test 1: Parsing réussi sur un fichier de test dédié ---
    total_tests++;
    if (test_embedded_parser_fixture()) {
        total_passed++;
    }

    // --- Test 2: Gestion du fichier inexistant ---
    total_tests++;
    if (test_file_not_found()) {
        total_passed++;
    }

    total_tests++;
    if (test_empty_gedcom_file()) {
        total_passed++;
    }

    // --- Test 3: Faux positif sur un mot SOUR dans du texte ---
    total_tests++;
    if (test_false_positive_tag_detection()) {
        total_passed++;
    }

    // --- Test 4: Entité source et citation avec contexte ---
    total_tests++;
    if (test_source_entity_and_citation_context()) {
        total_passed++;
    }

    total_tests++;
    if (test_nested_citation_context()) {
        total_passed++;
    }

    {
        TestResult read_error_result = test_read_error();

#if defined(__linux__)
        // cppcheck-suppress knownConditionTrueFalse
        if (read_error_result != TEST_SKIPPED) {
            total_tests++;
            total_passed += read_error_result == TEST_PASSED;
        }
#else
        (void)read_error_result;
#endif
    }

    total_tests++;
    if (test_long_gedcom_line()) {
        total_passed++;
    }

    total_tests++;
    if (test_invalid_gedcom_levels()) {
        total_passed++;
    }

    total_tests++;
    if (test_memory_allocation_error()) {
        total_passed++;
    }

    total_tests++;
    if (test_empty_filename()) {
        total_passed++;
    }

    total_tests++;
    if (test_orphan_citation()) {
        total_passed++;
    }

    total_tests++;
    if (test_repository_records()) {
        total_passed++;
    }

    printf("\n--- Test Initialisation/Nettoyage ---\n");
    total_tests++;
    if (test_source_list_initial_state()) {
        total_passed++;
    }

    printf("\n==================================================\n");
    printf("Tests terminés. Résultat: %d/%d réussis.\n", total_passed,
           total_tests);
    printf("==================================================\n");

    return (total_passed == total_tests) ? 0 : 1;
}
