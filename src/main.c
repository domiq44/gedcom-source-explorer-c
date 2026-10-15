#include "gedcom_parser.h"
#include "html_report.h"
#include "source_order.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

static int ensure_directory(const char *path) {
    struct stat status;

    if (mkdir(path, 0777) == 0) {
        return 1;
    }
    return errno == EEXIST && stat(path, &status) == 0 &&
           S_ISDIR(status.st_mode);
}

static int ensure_default_report_directory(void) {
    return ensure_directory("build") && ensure_directory("build/report");
}

static int is_source_entity(const Source *source) {
    return source->kind != NULL && strcmp(source->kind, "entity") == 0;
}

static int is_citation(const Source *source) {
    return source->kind == NULL || strcmp(source->kind, "citation") == 0;
}

/* qsort imposes deux paramètres de type const void *. */
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
static int compare_source_references(const void *left_value,
                                     const void *right_value) {
    const Source *left = *(const Source *const *)left_value;
    const Source *right = *(const Source *const *)right_value;
    return compare_gedcom_references(left->reference, right->reference);
}

static int has_source_entity(const SourceList *sources, const char *reference) {
    size_t index;

    for (index = 0U; index < sources->count; index++) {
        if (is_source_entity(&sources->sources[index]) &&
            sources->sources[index].reference != NULL &&
            strcmp(sources->sources[index].reference, reference) == 0) {
            return 1;
        }
    }

    return 0;
}

static const Repository *find_repository(const RepositoryList *repositories,
                                         const char *reference) {
    size_t index;

    for (index = 0U; index < repositories->count; index++) {
        if (strcmp(repositories->repositories[index].reference, reference) ==
            0) {
            return &repositories->repositories[index];
        }
    }

    return NULL;
}

static void print_gedcom_excerpt(const GedcomExcerpt *excerpt,
                                 int base_indentation) {
    const char *line;

    if (excerpt == NULL || excerpt->text == NULL) {
        return;
    }

    line = excerpt->text;
    while (*line != '\0') {
        const char *line_end = strchr(line, '\n');
        size_t line_length =
            (line_end != NULL) ? (size_t)(line_end - line) : strlen(line);
        char *level_end;
        long level;

        errno = 0;
        level = strtol(line, &level_end, 10);
        if (errno == ERANGE || level_end == line || level < 0L ||
            level > (INT_MAX - base_indentation) / 2 ||
            (*level_end != ' ' && *level_end != '\t')) {
            level = 0;
        }
        printf("%*s%.*s\n", base_indentation + (int)level * 2, "",
               (int)line_length, line);

        if (line_end == NULL) {
            break;
        }
        line = line_end + 1;
    }
}

static int print_source_groups(const SourceList *sources,
                               const RepositoryList *repositories,
                               size_t source_count) {
    Source **ordered_sources = NULL;
    size_t source_index;
    size_t source_number = 0U;

    if (source_count > 0U) {
        ordered_sources = malloc(source_count * sizeof(*ordered_sources));
        if (ordered_sources == NULL) {
            return 0;
        }
        for (source_index = 0U; source_index < sources->count; source_index++) {
            if (is_source_entity(&sources->sources[source_index])) {
                ordered_sources[source_number++] =
                    &sources->sources[source_index];
            }
        }
        qsort(ordered_sources, source_count, sizeof(*ordered_sources),
              compare_source_references);
        source_number = 0U;
    }

    printf("\n==================================================\n");
    printf("SOURCES ET ÉLÉMENTS ASSOCIÉS\n");
    printf("==================================================\n");

    for (source_index = 0U; source_index < source_count; source_index++) {
        const Source *source = ordered_sources[source_index];
        size_t link_index;
        size_t citation_index;
        size_t linked_repository_count = 0U;
        size_t linked_citation_count = 0U;

        source_number++;
        printf("\nSource %2zu | ref=%s | valeur=%s\n", source_number,
               (source->reference != NULL) ? source->reference : "-",
               (source->description != NULL) ? source->description : "-");
        printf("  Extrait GEDCOM source :\n");
        print_gedcom_excerpt(&source->gedcom, 4);

        for (link_index = 0U; link_index < repositories->link_count;
             link_index++) {
            const RepositoryLink *link = &repositories->links[link_index];
            const Repository *repository;

            if (source->reference == NULL ||
                strcmp(link->source_reference, source->reference) != 0) {
                continue;
            }

            repository =
                find_repository(repositories, link->repository_reference);
            printf("  Dépôt | ref=%s | nom=%s | contexte=%s\n",
                   link->repository_reference,
                   (repository != NULL) ? repository->name : "nom inconnu",
                   link->context);
            if (repository != NULL) {
                printf("    Extrait GEDCOM dépôt :\n");
                print_gedcom_excerpt(&repository->gedcom, 6);
            }
            linked_repository_count++;
        }

        if (linked_repository_count == 0U) {
            printf("  Dépôt associé : aucun\n");
        }

        for (citation_index = 0U; citation_index < sources->count;
             citation_index++) {
            const Source *citation = &sources->sources[citation_index];

            if (!is_citation(citation) || source->reference == NULL ||
                citation->reference == NULL ||
                strcmp(citation->reference, source->reference) != 0) {
                continue;
            }

            printf("  Citation | ref=%s | contexte=%s\n", citation->reference,
                   (citation->context != NULL) ? citation->context : "-");
            printf("    Extrait GEDCOM citation :\n");
            print_gedcom_excerpt(&citation->gedcom, 6);
            linked_citation_count++;
        }

        if (linked_citation_count == 0U) {
            printf("  Citation : aucune\n");
        }
    }

    if (source_number == 0U) {
        printf("Aucune entité source trouvée.\n");
    }
    free(ordered_sources);
    return 1;
}

static void print_unmatched_citations(const SourceList *sources) {
    size_t index;
    size_t unmatched_count = 0U;

    for (index = 0U; index < sources->count; index++) {
        const Source *citation = &sources->sources[index];

        if (is_citation(citation) &&
            (citation->reference == NULL ||
             !has_source_entity(sources, citation->reference))) {
            unmatched_count++;
        }
    }

    if (unmatched_count == 0U) {
        return;
    }

    printf("\n==================================================\n");
    printf("CITATIONS SANS ENTITÉ SOURCE CORRESPONDANTE\n");
    printf("==================================================\n");

    for (index = 0U; index < sources->count; index++) {
        const Source *citation = &sources->sources[index];

        if (is_citation(citation) &&
            (citation->reference == NULL ||
             !has_source_entity(sources, citation->reference))) {
            printf("Citation | ref=%s | contexte=%s | valeur=%s\n",
                   (citation->reference != NULL) ? citation->reference : "-",
                   (citation->context != NULL) ? citation->context : "-",
                   (citation->description != NULL) ? citation->description
                                                   : "-");
            printf("  Extrait GEDCOM citation :\n");
            print_gedcom_excerpt(&citation->gedcom, 4);
        }
    }
}

static const char *parse_error_message(GedcomParseErrorCode code) {
    switch (code) {
    case GEDCOM_PARSE_ERROR_INVALID_ARGUMENT:
        return "arguments du parseur invalides";
    case GEDCOM_PARSE_ERROR_OPEN:
        return "impossible d'ouvrir le fichier";
    case GEDCOM_PARSE_ERROR_INVALID_LEVEL:
        return "niveau GEDCOM invalide ou hors plage (0 à 99)";
    case GEDCOM_PARSE_ERROR_READ:
        return "erreur de lecture ou de fermeture du fichier";
    case GEDCOM_PARSE_ERROR_MEMORY:
        return "échec d'allocation mémoire";
    case GEDCOM_PARSE_ERROR_NONE:
    default:
        return "erreur inconnue";
    }
}

static void print_usage(FILE *output, const char *program_name) {
    fprintf(output,
            "Usage :\n"
            "  %s <fichier.ged>\n"
            "  %s --html <fichier.ged> [rapport.html]\n"
            "  %s --help\n",
            program_name, program_name, program_name);
}

int main(int argc, char *argv[]) {
    const char *filename;
    const char *report_filename = NULL;
    SourceList sources;
    RepositoryList repositories;
    GedcomParseError parse_error;
    int result_code = 0;
    int parse_status;
    int report_mode = 0;

    if (argc == 1 || strcmp(argv[1], "--help") == 0) {
        print_usage(stdout, argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "--html") == 0) {
        if (argc < 3 || argc > 4) {
            print_usage(stderr, argv[0]);
            return 2;
        }
        report_mode = 1;
        filename = argv[2];
        report_filename = (argc == 4) ? argv[3] : "build/report/index.html";
        if (argc == 3 && !ensure_default_report_directory()) {
            fprintf(stderr,
                    "\n[ERREUR] Impossible de créer le dossier du rapport HTML.\n");
            return 1;
        }
    } else {
        filename = argv[1];
        printf("==================================================\n");
        printf("Analyseur GEDCOM - Extraction des Sources (SOUR)\n");
        printf("==================================================\n");
        printf("Analyse du fichier spécifié: '%s'\n", filename);
    }

    init_source_list(&sources);
    init_repository_list(&repositories);
    parse_status = parse_gedcom_with_error(filename, &sources, &repositories,
                                           &parse_error);

    if (parse_status != 0) {
        if (parse_error.line_number > 0U) {
            fprintf(stderr,
                    "\n[ERREUR] Ligne %zu du fichier GEDCOM '%s' : %s.\n",
                    parse_error.line_number, filename,
                    parse_error_message(parse_error.code));
        } else {
            fprintf(stderr, "\n[ERREUR] Fichier GEDCOM '%s' : %s.\n", filename,
                    parse_error_message(parse_error.code));
        }
        result_code = 1;
    } else if (report_mode) {
        if (write_html_report("web/report.html", report_filename, &sources,
                              &repositories) != 0) {
            fprintf(stderr,
                    "\n[ERREUR] Impossible de créer le rapport HTML '%s'.\n",
                    report_filename);
            result_code = 1;
        } else {
            printf("Rapport HTML créé : %s\n", report_filename);
        }
    } else {
        size_t index;
        size_t source_count = 0U;
        size_t citation_count = 0U;

        for (index = 0U; index < sources.count; index++) {
            if (is_source_entity(&sources.sources[index])) {
                source_count++;
            } else if (is_citation(&sources.sources[index])) {
                citation_count++;
            }
        }

        printf("--- Analyse terminée | entités source : %zu | dépôts : %zu | "
               "citations : %zu\n",
               source_count, repositories.count, citation_count);
        if (!print_source_groups(&sources, &repositories, source_count)) {
            fprintf(stderr,
                    "\n[ERREUR] Impossible de trier les entités source.\n");
            result_code = 1;
        }
        print_unmatched_citations(&sources);
    }

    free_source_list(&sources);
    free_repository_list(&repositories);
    return result_code;
}
