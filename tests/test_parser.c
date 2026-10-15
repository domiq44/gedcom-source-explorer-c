#include "gedcom_parser.h"
#include "string_utils.h"
#include "test_runner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *filename;
    const char *invalid_line;
} InvalidGedcomLevelCase;

// --- Fonctions utilitaires de test ---

/* Écrit une fixture et garantit sa fermeture avant le test. */
static int write_text_fixture(const char *filename, const char *contents) {
    FILE *file = fopen(filename, "w");
    int write_status;
    int close_status;

    if (file == NULL) {
        return 0;
    }
    write_status = fputs(contents, file);
    close_status = fclose(file);
    if (write_status == EOF || close_status != 0) {
        remove(filename);
        return 0;
    }
    return 1;
}

/* Écrit la fixture commune aux tests de niveaux invalides. */
static int write_invalid_level_fixture(InvalidGedcomLevelCase test_case) {
    FILE *file = fopen(test_case.filename, "w");
    int write_status;
    int close_status;

    if (file == NULL) {
        return 0;
    }
    write_status = fputs("0 @I1@ INDI\n", file);
    if (write_status != EOF && fputs(test_case.invalid_line, file) == EOF) {
        write_status = EOF;
    }
    close_status = fclose(file);
    if (write_status == EOF || close_status != 0) {
        remove(test_case.filename);
        return 0;
    }
    return 1;
}

static const char repository_records_fixture[] =
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

/* Écrit la fixture commune aux tests des liens et extraits. */
static int write_repository_fixture(const char *filename) {
    return write_text_fixture(filename, repository_records_fixture);
}

/* Vérifie l'extraction des entités SOUR d'une fixture GEDCOM. */
static int test_embedded_parser_fixture(void) {
    /* Given */
    const int expected_parse_result = 0;
    const size_t expected_source_count = 3U;
    static const char *expected_descriptions[] = {"@S1@", "@S2@", "@S3@"};
    static const char fixture[] = "0 HEAD\n"
                                  "1 CHAR ANSI\n"
                                  "0 @S1@ SOUR\n"
                                  "0 @S2@ SOUR\n"
                                  "0 @S3@ SOUR\n"
                                  "0 TRLR\n";
    const char *filename = "tmp_parser_input.ged";
    SourceList sources;
    RepositoryList repositories;
    int fixture_written;
    int parse_result = -1;
    int passed;

    init_source_list(&sources);
    init_repository_list(&repositories);
    fixture_written = write_text_fixture(filename, fixture);

    /* When */
    if (fixture_written) {
        parse_result = parse_gedcom(filename, &sources, &repositories);
    }

    /* Then */
    passed =
        fixture_written && parse_result == expected_parse_result &&
        sources.count == expected_source_count &&
        strcmp(sources.sources[0].description, expected_descriptions[0]) == 0 &&
        strcmp(sources.sources[1].description, expected_descriptions[1]) == 0 &&
        strcmp(sources.sources[2].description, expected_descriptions[2]) == 0;
    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(filename);
    return passed;
}

/* Vérifie qu'un fichier absent produit une erreur d'ouverture. */
static int test_file_not_found(void) {
    /* Given */
    const GedcomParseErrorCode expected_error_code = GEDCOM_PARSE_ERROR_OPEN;
    const size_t expected_line_number = 0U;
    const char *filename = "non_existent_file_xyz.ged";
    SourceList sources;
    RepositoryList repositories;
    GedcomParseError error;
    int parse_result;
    int passed;

    init_source_list(&sources);
    init_repository_list(&repositories);

    /* When */
    parse_result =
        parse_gedcom_with_error(filename, &sources, &repositories, &error);

    /* Then */
    passed = parse_result != 0 && error.code == expected_error_code &&
             error.line_number == expected_line_number;
    free_source_list(&sources);
    free_repository_list(&repositories);
    return passed;
}

/* Vérifie qu'un fichier GEDCOM vide est accepté sans résultat. */
static int test_empty_gedcom_file(void) {
    /* Given */
    const int expected_parse_result = 0;
    const size_t expected_source_count = 0U;
    const size_t expected_repository_count = 0U;
    const char *filename = "tmp_empty_gedcom.ged";
    SourceList sources;
    RepositoryList repositories;
    int fixture_written;
    int parse_result = -1;
    int passed;

    init_source_list(&sources);
    init_repository_list(&repositories);
    fixture_written = write_text_fixture(filename, "");

    /* When */
    if (fixture_written) {
        parse_result = parse_gedcom(filename, &sources, &repositories);
    }

    /* Then */
    passed = fixture_written && parse_result == expected_parse_result &&
             sources.count == expected_source_count &&
             repositories.count == expected_repository_count;

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
    const char *filename = "tmp_false_positive.ged";
    static const char fixture[] = "\xEF\xBB\xBF"
                                  "0 HEAD\n"
                                  "1 SOUR unregistered\n"
                                  "1 NOTE SOUR should be ignored because it "
                                  "is just text\n"
                                  "0 @I1@ INDI\n"
                                  "1 SOUR @S1@\n"
                                  "0 @I2@ INDI\n"
                                  "1 SOUR Source Name\n"
                                  "0 TRLR\n";
    SourceList sources;
    RepositoryList repositories;
    int fixture_written;
    int parse_result = -1;
    int passed;

    init_source_list(&sources);
    init_repository_list(&repositories);
    fixture_written = write_text_fixture(filename, fixture);

    /* When */
    if (fixture_written) {
        parse_result = parse_gedcom(filename, &sources, &repositories);
    }

    /* Then */
    passed = fixture_written && parse_result == expected_parse_result &&
             sources.count == expected_source_count &&
             strcmp(sources.sources[0].description,
                    expected_first_description) == 0 &&
             strcmp(sources.sources[1].description,
                    expected_second_description) == 0;

    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(filename);
    return passed;
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
    const char *filename = "tmp_source_citation.ged";
    static const char fixture[] = "0 @S1@ SOUR\n"
                                  "1 TITL Source principale\n"
                                  "0 @I1@ INDI\n"
                                  "1 NAME Jean /DOE/\n"
                                  "1 SOUR @S1@\n"
                                  "0 TRLR\n";
    SourceList sources;
    RepositoryList repositories;
    int fixture_written;
    int parse_result = -1;
    int passed;

    init_source_list(&sources);
    init_repository_list(&repositories);
    fixture_written = write_text_fixture(filename, fixture);

    /* When */
    if (fixture_written) {
        parse_result = parse_gedcom(filename, &sources, &repositories);
    }

    /* Then */
    passed = fixture_written && parse_result == expected_parse_result &&
             sources.count == expected_source_count &&
             strcmp(sources.sources[0].kind, expected_entity_kind) == 0 &&
             strcmp(sources.sources[0].reference, expected_reference) == 0 &&
             sources.sources[0].title != NULL &&
             strcmp(sources.sources[0].title, expected_title) == 0 &&
             strcmp(sources.sources[1].kind, expected_citation_kind) == 0 &&
             strcmp(sources.sources[1].reference, expected_reference) == 0 &&
             strcmp(sources.sources[1].context, expected_context) == 0;

    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(filename);
    return passed;
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
    SourceList sources;
    RepositoryList repositories;
    int fixture_written;
    int parse_result = -1;
    int passed;

    init_source_list(&sources);
    init_repository_list(&repositories);
    fixture_written = write_text_fixture(filename, fixture);

    /* When */
    if (fixture_written) {
        parse_result = parse_gedcom(filename, &sources, &repositories);
    }

    /* Then */
    passed = fixture_written && parse_result == expected_parse_result &&
             sources.count == expected_source_count &&
             strcmp(sources.sources[0].context, expected_context) == 0 &&
             strstr(sources.sources[0].gedcom.text, expected_excerpt) != NULL;

    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(filename);
    return passed;
}

/* Vérifie qu'une erreur de lecture est distinguée d'une fin de fichier. */
static int test_read_error(void) {
    /* Given */
#if defined(__linux__)
    const GedcomParseErrorCode expected_error_code = GEDCOM_PARSE_ERROR_READ;
    const int expected_parse_failure = 1;
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
    const char *filename = "tmp_long_line.ged";
    char long_title[4096];
    char fixture[4200];
    size_t index;
    SourceList sources;
    RepositoryList repositories;
    int fixture_written;
    int parse_result = -1;
    int passed;

    for (index = 0U; index < sizeof(long_title) - 1U; index++) {
        long_title[index] = (index % 10U == 0U) ? 'A' : 'B';
    }
    long_title[sizeof(long_title) - 1U] = '\0';
    snprintf(fixture, sizeof(fixture),
             "0 HEAD\n0 @S1@ SOUR\n1 TITL %s\n0 TRLR\n", long_title);
    init_source_list(&sources);
    init_repository_list(&repositories);
    fixture_written = write_text_fixture(filename, fixture);

    /* When */
    if (fixture_written) {
        parse_result = parse_gedcom(filename, &sources, &repositories);
    }

    /* Then */
    passed = fixture_written && parse_result == expected_parse_result &&
             sources.count == expected_source_count &&
             sources.sources[0].title != NULL &&
             strcmp(sources.sources[0].title, long_title) == 0;
    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(filename);
    return passed;
}

/* Vérifie le rejet d'un niveau GEDCOM qui dépasse la capacité numérique. */
static int test_rejects_overflowing_gedcom_level(void) {
    /* Given */
    const int expected_parse_failure = 1;
    const GedcomParseErrorCode expected_error_code =
        GEDCOM_PARSE_ERROR_INVALID_LEVEL;
    const size_t expected_error_line = 2U;
    const InvalidGedcomLevelCase test_case = {
        .filename = "tmp_overflowing_gedcom_level.ged",
        .invalid_line = "999999999999999999999999999999999999 SOUR @S1@\n"};
    SourceList sources;
    RepositoryList repositories;
    GedcomParseError error;
    int fixture_written;
    int parse_result = -1;
    int rejected;

    init_source_list(&sources);
    init_repository_list(&repositories);
    fixture_written = write_invalid_level_fixture(test_case);

    /* When */
    if (fixture_written) {
        parse_result = parse_gedcom_with_error(test_case.filename, &sources,
                                               &repositories, &error);
    }

    /* Then */
    rejected = fixture_written &&
               (parse_result != 0) == expected_parse_failure &&
               error.code == expected_error_code &&
               error.line_number == expected_error_line;
    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(test_case.filename);
    return rejected;
}

/* Vérifie le rejet d'un niveau GEDCOM hors de la profondeur permise. */
static int test_rejects_excessive_gedcom_level(void) {
    /* Given */
    const int expected_parse_failure = 1;
    const GedcomParseErrorCode expected_error_code =
        GEDCOM_PARSE_ERROR_INVALID_LEVEL;
    const size_t expected_error_line = 2U;
    const InvalidGedcomLevelCase test_case = {
        .filename = "tmp_excessive_gedcom_level.ged",
        .invalid_line = "100 SOUR @S1@\n"};
    SourceList sources;
    RepositoryList repositories;
    GedcomParseError error;
    int fixture_written;
    int parse_result = -1;
    int rejected;

    init_source_list(&sources);
    init_repository_list(&repositories);
    fixture_written = write_invalid_level_fixture(test_case);

    /* When */
    if (fixture_written) {
        parse_result = parse_gedcom_with_error(test_case.filename, &sources,
                                               &repositories, &error);
    }

    /* Then */
    rejected = fixture_written &&
               (parse_result != 0) == expected_parse_failure &&
               error.code == expected_error_code &&
               error.line_number == expected_error_line;
    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(test_case.filename);
    return rejected;
}

/* Vérifie le rejet d'un niveau GEDCOM dont le texte est mal formé. */
static int test_rejects_malformed_gedcom_level(void) {
    /* Given */
    const int expected_parse_failure = 1;
    const GedcomParseErrorCode expected_error_code =
        GEDCOM_PARSE_ERROR_INVALID_LEVEL;
    const size_t expected_error_line = 2U;
    const InvalidGedcomLevelCase test_case = {
        .filename = "tmp_malformed_gedcom_level.ged",
        .invalid_line = "1x SOUR @S1@\n"};
    SourceList sources;
    RepositoryList repositories;
    GedcomParseError error;
    int fixture_written;
    int parse_result = -1;
    int rejected;

    init_source_list(&sources);
    init_repository_list(&repositories);
    fixture_written = write_invalid_level_fixture(test_case);

    /* When */
    if (fixture_written) {
        parse_result = parse_gedcom_with_error(test_case.filename, &sources,
                                               &repositories, &error);
    }

    /* Then */
    rejected = fixture_written &&
               (parse_result != 0) == expected_parse_failure &&
               error.code == expected_error_code &&
               error.line_number == expected_error_line;
    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(test_case.filename);
    return rejected;
}

/* Vérifie qu'un échec d'allocation est rapporté avec sa ligne. */
static int test_memory_allocation_error(void) {
    /* Given */
    const GedcomParseErrorCode expected_error_code = GEDCOM_PARSE_ERROR_MEMORY;
    const size_t expected_error_line = 1U;
    const int expected_parse_failure = 1;
    static const char fixture[] = "0 @S1@ SOUR\n";
    const char *filename = "tmp_memory_failure.ged";
    SourceList sources;
    RepositoryList repositories;
    GedcomParseError error;
    int fixture_written;
    int parse_result = -1;
    int passed;

    init_source_list(&sources);
    init_repository_list(&repositories);
    fixture_written = write_text_fixture(filename, fixture);
    string_utils_test_fail_after(0U);

    /* When */
    if (fixture_written) {
        parse_result =
            parse_gedcom_with_error(filename, &sources, &repositories, &error);
    }
    string_utils_test_reset_failure();

    /* Then */
    passed = fixture_written && (parse_result != 0) == expected_parse_failure &&
             error.code == expected_error_code &&
             error.line_number == expected_error_line;
    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(filename);
    return passed;
}

/* Vérifie qu'un chemin GEDCOM vide est rejeté. */
static int test_empty_filename(void) {
    /* Given */
    const int expected_rejected = 1;
    SourceList sources;
    RepositoryList repositories;
    int parse_result;
    int rejected;

    init_source_list(&sources);
    init_repository_list(&repositories);

    /* When */
    parse_result = parse_gedcom("", &sources, &repositories);

    /* Then */
    rejected = (parse_result != 0) == expected_rejected;
    free_source_list(&sources);
    free_repository_list(&repositories);
    return rejected;
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
    int fixture_written;
    int parse_result = -1;
    int passed;

    init_source_list(&sources);
    init_repository_list(&repositories);
    fixture_written = write_text_fixture(filename, fixture);

    /* When */
    if (fixture_written) {
        parse_result = parse_gedcom(filename, &sources, &repositories);
    }

    /* Then */
    passed =
        fixture_written && parse_result == expected_orphan_parse_result &&
        sources.count == expected_orphan_count &&
        strcmp(sources.sources[0].kind, expected_orphan_kind) == 0 &&
        strcmp(sources.sources[0].reference, expected_orphan_reference) == 0;
    if (!passed) {
        printf("[FAIL] La citation orpheline est perdue ou mal typée.\n");
    }

    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(filename);
    return passed;
}

/* Vérifie l'extraction des dépôts et de leurs liens de source. */
static int test_extracts_repository_links(void) {
    /* Given */
    const size_t expected_repository_count = 2U;
    const size_t expected_link_count = 2U;
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
    const char *filename = "tmp_repository_links.ged";
    SourceList sources;
    RepositoryList repositories;
    int fixture_written;
    int parse_result = -1;
    int passed;

    init_source_list(&sources);
    init_repository_list(&repositories);
    fixture_written = write_repository_fixture(filename);

    /* When */
    if (fixture_written) {
        parse_result = parse_gedcom(filename, &sources, &repositories);
    }

    /* Then */
    passed =
        fixture_written && parse_result == expected_parse_result &&
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
               expected_second_link_repository_reference) == 0;

    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(filename);
    return passed;
}

/* Vérifie que l'extrait d'une source inclut ses sous-tags sans l'individu
 * suivant. */
static int test_extracts_source_record_excerpt(void) {
    /* Given */
    const int expected_parse_result = 0;
    const size_t expected_source_count = 3U;
    const size_t expected_minimum_excerpt_length = 128U;
    const char *expected_source_start = "0 @S1@ SOUR\n";
    const char *expected_source_continuation =
        "2 CONT Première partie du titre\n";
    const char *expected_repository_media = "3 MEDI Livre\n";
    const char *unexpected_individual_record = "0 @I1@ INDI\n";
    const char *filename = "tmp_source_excerpts.ged";
    SourceList sources;
    RepositoryList repositories;
    int fixture_written;
    int parse_result = -1;
    int passed;

    init_source_list(&sources);
    init_repository_list(&repositories);
    fixture_written = write_repository_fixture(filename);

    /* When */
    if (fixture_written) {
        parse_result = parse_gedcom(filename, &sources, &repositories);
    }

    /* Then */
    passed =
        fixture_written && parse_result == expected_parse_result &&
        sources.count == expected_source_count &&
        strstr(sources.sources[0].gedcom.text, expected_source_start) != NULL &&
        strstr(sources.sources[0].gedcom.text, expected_source_continuation) !=
            NULL &&
        strstr(sources.sources[0].gedcom.text, expected_repository_media) !=
            NULL &&
        strstr(sources.sources[0].gedcom.text, unexpected_individual_record) ==
            NULL &&
        sources.sources[0].gedcom.length > expected_minimum_excerpt_length;
    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(filename);
    return passed;
}

/* Vérifie que l'extrait d'une citation s'arrête avant l'individu suivant. */
static int test_extracts_citation_excerpt(void) {
    /* Given */
    const int expected_parse_result = 0;
    const size_t expected_source_count = 3U;
    const char *expected_citation_excerpt =
        "1 SOUR @S1@\n2 PAGE Page 12\n3 NOTE Mention manuscrite\n";
    const char *unexpected_second_individual = "0 @I2@ INDI\n";
    const char *filename = "tmp_citation_excerpt.ged";
    SourceList sources;
    RepositoryList repositories;
    int fixture_written;
    int parse_result = -1;
    int passed;

    init_source_list(&sources);
    init_repository_list(&repositories);
    fixture_written = write_repository_fixture(filename);

    /* When */
    if (fixture_written) {
        parse_result = parse_gedcom(filename, &sources, &repositories);
    }

    /* Then */
    passed = fixture_written && parse_result == expected_parse_result &&
             sources.count == expected_source_count &&
             strstr(sources.sources[1].gedcom.text,
                    expected_citation_excerpt) != NULL &&
             strstr(sources.sources[1].gedcom.text,
                    unexpected_second_individual) == NULL;
    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(filename);
    return passed;
}

/* Vérifie que l'extrait d'une citation orpheline reste dans son record. */
static int test_extracts_orphan_citation_excerpt(void) {
    /* Given */
    const int expected_parse_result = 0;
    const size_t expected_source_count = 3U;
    const char *expected_orphan_excerpt =
        "1 SOUR @S9@\n2 PAGE Référence orpheline\n";
    const char *filename = "tmp_orphan_excerpt.ged";
    SourceList sources;
    RepositoryList repositories;
    int fixture_written;
    int parse_result = -1;
    int passed;

    init_source_list(&sources);
    init_repository_list(&repositories);
    fixture_written = write_repository_fixture(filename);

    /* When */
    if (fixture_written) {
        parse_result = parse_gedcom(filename, &sources, &repositories);
    }

    /* Then */
    passed =
        fixture_written && parse_result == expected_parse_result &&
        sources.count == expected_source_count &&
        strstr(sources.sources[2].gedcom.text, expected_orphan_excerpt) != NULL;
    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(filename);
    return passed;
}

/* Vérifie que l'extrait d'un dépôt exclut le dépôt qui le suit. */
static int test_extracts_repository_excerpt(void) {
    /* Given */
    const int expected_parse_result = 0;
    const size_t expected_repository_count = 2U;
    const char *expected_repository_start =
        "0 @R1@ REPO\n1 NAME Archives Municipales\n";
    const char *expected_repository_city = "2 CITY Saint-Denis\n";
    const char *unexpected_second_repository = "0 @R2@ REPO\n";
    const char *filename = "tmp_repository_excerpt.ged";
    SourceList sources;
    RepositoryList repositories;
    int fixture_written;
    int parse_result = -1;
    int passed;

    init_source_list(&sources);
    init_repository_list(&repositories);
    fixture_written = write_repository_fixture(filename);

    /* When */
    if (fixture_written) {
        parse_result = parse_gedcom(filename, &sources, &repositories);
    }

    /* Then */
    passed = fixture_written && parse_result == expected_parse_result &&
             repositories.count == expected_repository_count &&
             strstr(repositories.repositories[0].gedcom.text,
                    expected_repository_start) != NULL &&
             strstr(repositories.repositories[0].gedcom.text,
                    expected_repository_city) != NULL &&
             strstr(repositories.repositories[0].gedcom.text,
                    unexpected_second_repository) == NULL;
    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(filename);
    return passed;
}

/* Exécute l'ensemble des tests unitaires du parseur GEDCOM. */
static const TestCase test_cases[] = {
    {"test_embedded_parser_fixture", test_embedded_parser_fixture},
    {"test_file_not_found", test_file_not_found},
    {"test_empty_gedcom_file", test_empty_gedcom_file},
    {"test_false_positive_tag_detection", test_false_positive_tag_detection},
    {"test_source_entity_and_citation_context",
     test_source_entity_and_citation_context},
    {"test_nested_citation_context", test_nested_citation_context},
    {"test_read_error", test_read_error},
    {"test_long_gedcom_line", test_long_gedcom_line},
    {"test_rejects_overflowing_gedcom_level",
     test_rejects_overflowing_gedcom_level},
    {"test_rejects_excessive_gedcom_level",
     test_rejects_excessive_gedcom_level},
    {"test_rejects_malformed_gedcom_level",
     test_rejects_malformed_gedcom_level},
    {"test_memory_allocation_error", test_memory_allocation_error},
    {"test_empty_filename", test_empty_filename},
    {"test_orphan_citation", test_orphan_citation},
    {"test_extracts_repository_links", test_extracts_repository_links},
    {"test_extracts_source_record_excerpt",
     test_extracts_source_record_excerpt},
    {"test_extracts_citation_excerpt", test_extracts_citation_excerpt},
    {"test_extracts_orphan_citation_excerpt",
     test_extracts_orphan_citation_excerpt},
    {"test_extracts_repository_excerpt", test_extracts_repository_excerpt},
};

int main(void) {
    return run_test_suite("parser", test_cases,
                          sizeof(test_cases) / sizeof(test_cases[0]));
}
