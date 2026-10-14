#include "html_report.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helpers de test */

typedef struct {
    const char *payload;
    const char *special_value;
} ReportInput;

typedef struct {
    const char *contents;
    const char *template_filename;
    const char *output_filename;
} TemplateInput;

/* Prépare les sources communes aux tests du rendu HTML. */
static int prepare_report_inputs(SourceList *sources,
                                 RepositoryList *repositories,
                                 const ReportInput *input) {
    const SourceInput source = {
        .description = input->payload,
        .reference = "@S1@",
        .kind = "entity",
        .context = "entity",
    };
    const SourceInput special_citation = {
        .description = input->special_value,
        .reference = "@S99@",
        .kind = "citation",
        .context = "@I99@ INDI",
    };

    init_source_list(sources);
    init_repository_list(repositories);
    if (!source_list_add(sources, &source) ||
        !source_list_add(sources, &special_citation) ||
        !source_list_set_title(sources, "@S1@", "Titre démo")) {
        return 0;
    }
    return 1;
}

/* Génère le rapport, le lit dans contents et supprime le fichier temporaire. */
static int read_generated_report(const char *filename,
                                 const SourceList *sources,
                                 const RepositoryList *repositories,
                                 char *contents, size_t contents_capacity) {
    FILE *report;
    size_t bytes_read;
    int read_succeeded;

    if (write_html_report("web/report.html", filename, sources, repositories) !=
        0) {
        remove(filename);
        return 0;
    }
    report = fopen(filename, "r");
    if (report == NULL) {
        remove(filename);
        return 0;
    }

    bytes_read = fread(contents, 1U, contents_capacity - 1U, report);
    contents[bytes_read] = '\0';
    read_succeeded = !ferror(report);
    if (fclose(report) != 0) {
        read_succeeded = 0;
    }
    remove(filename);
    return read_succeeded;
}

/* Écrit un template puis indique si le générateur le rejette. */
static int rejects_template(const TemplateInput *input) {
    FILE *template_file = fopen(input->template_filename, "w");
    SourceList sources;
    RepositoryList repositories;
    int write_status;
    int close_status;
    int rejected;

    if (template_file == NULL) {
        return 0;
    }
    write_status = fputs(input->contents, template_file);
    close_status = fclose(template_file);
    if (write_status == EOF || close_status != 0) {
        remove(input->template_filename);
        return 0;
    }

    init_source_list(&sources);
    init_repository_list(&repositories);
    rejected =
        write_html_report(input->template_filename, input->output_filename,
                          &sources, &repositories) != 0;
    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(input->template_filename);
    remove(input->output_filename);
    return rejected;
}

/* Exécute un test et rapporte son résultat sous son nom. */
static int run_test(const char *test_name, int (*test_function)(void)) {
    int passed = test_function();

    printf("[%s] %s\n", passed ? "PASS" : "FAIL", test_name);
    return passed;
}

/* Vérifie qu'un chemin de template vide est rejeté. */
static int test_rejects_empty_template_path(void) {
    /* Given */
    const int expected_rejected = 1;
    const char *output_filename = "tmp_html_report_empty_template.html";
    ReportData empty_data = {0};
    int rejected;

    /* When */
    rejected = write_html_report_data("", output_filename, &empty_data) != 0;

    /* Then */
    remove(output_filename);
    if (rejected == expected_rejected) {
        printf("[PASS] Rejet d'un chemin de template vide.\n");
        return 1;
    }
    printf("[FAIL] Rejet d'un chemin de template vide.\n");
    return 0;
}

/* Vérifie qu'un chemin de sortie vide est rejeté. */
static int test_rejects_empty_output_path(void) {
    /* Given */
    const int expected_rejected = 1;
    const char *template_filename = "web/report.html";
    ReportData empty_data = {0};
    int rejected;

    /* When */
    rejected = write_html_report_data(template_filename, "", &empty_data) != 0;

    /* Then */
    if (rejected == expected_rejected) {
        printf("[PASS] Rejet d'un chemin de sortie vide.\n");
        return 1;
    }
    printf("[FAIL] Rejet d'un chemin de sortie vide.\n");
    return 0;
}

/* Vérifie qu'un pointeur de données NULL est rejeté. */
static int test_rejects_null_report_data(void) {
    /* Given */
    const int expected_rejected = 1;
    const char *template_filename = "web/report.html";
    const char *output_filename = "tmp_html_report_null_data.html";
    int rejected;

    /* When */
    rejected =
        write_html_report_data(template_filename, output_filename, NULL) != 0;

    /* Then */
    remove(output_filename);
    if (rejected == expected_rejected) {
        printf("[PASS] Rejet d'un rapport NULL.\n");
        return 1;
    }
    printf("[FAIL] Rejet d'un rapport NULL.\n");
    return 0;
}

/* Vérifie l'échappement des valeurs GEDCOM avant leur insertion dans le JSON.
 */
static int test_escapes_report_data(void) {
    /* Given */
    const char *filename = "tmp_html_report_escaping.html";
    const char *payload = "</script><script>probe()</script>";
    const char *special_value =
        "quote \" slash \\ backspace \b formfeed \f newline \n carriage \r "
        "tab \t control \001 amp &";
    const char *expected_escaped_payload =
        "\\u003c/script\\u003e\\u003cscript\\u003eprobe()\\u003c/script\\u003e";
    const char *expected_unescaped_payload = "</script><script>probe()";
    const char *expected_title = "\"title\":\"Titre démo\"";
    const char *expected_escaped_special_value =
        "\"description\":\"quote \\\" slash \\\\ backspace \\b formfeed \\f "
        "newline \\n carriage \\r tab \\t control \\u0001 amp \\u0026\"";
    const ReportInput input = {.payload = payload,
                               .special_value = special_value};
    SourceList sources;
    RepositoryList repositories;
    char contents[8192];
    int inputs_ready;
    int report_read;
    int passed;

    /* When */
    inputs_ready = prepare_report_inputs(&sources, &repositories, &input);
    report_read =
        inputs_ready && read_generated_report(filename, &sources, &repositories,
                                              contents, sizeof(contents));

    /* Then */
    passed = report_read &&
             strstr(contents, expected_escaped_payload) != NULL &&
             strstr(contents, expected_unescaped_payload) == NULL &&
             strstr(contents, expected_title) != NULL &&
             strstr(contents, expected_escaped_special_value) != NULL;
    if (!inputs_ready) {
        printf("[FAIL] Impossible de préparer les données du rapport.\n");
    } else if (passed) {
        printf("[PASS] Les valeurs GEDCOM sont échappées dans le rapport.\n");
    } else {
        printf(
            "[FAIL] Le rapport contient des valeurs GEDCOM mal échappées.\n");
    }

    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(filename);
    return passed;
}

/* Vérifie que les commandes de navigation et leurs pas sont générés. */
static int test_renders_navigation_controls(void) {
    /* Given */
    const char *filename = "tmp_html_report_navigation.html";
    const char *expected_navigation_markers[] = {
        "id=\"first-source\"",
        "id=\"previous-source\"",
        "id=\"next-source\"",
        "id=\"last-source\"",
        "id=\"jump-back\"",
        "id=\"jump-forward\"",
        "title=\"Page suivante\">»</button>",
        "title=\"Page précédente\">«</button>",
        "<option value=\"10\">",
        "<option value=\"20\">",
        "<option value=\"30\">",
        "<option value=\"40\">",
        "<option value=\"50\">"};
    const char *payload = "navigation";
    const char *special_value = "citation";
    const ReportInput input = {.payload = payload,
                               .special_value = special_value};
    SourceList sources;
    RepositoryList repositories;
    char contents[8192];
    int inputs_ready;
    int report_read;
    int all_markers_found = 1;

    /* When */
    inputs_ready = prepare_report_inputs(&sources, &repositories, &input);
    report_read =
        inputs_ready && read_generated_report(filename, &sources, &repositories,
                                              contents, sizeof(contents));
    if (report_read) {
        for (size_t marker_index = 0U;
             marker_index < sizeof(expected_navigation_markers) /
                                sizeof(expected_navigation_markers[0]);
             marker_index++) {
            if (strstr(contents, expected_navigation_markers[marker_index]) ==
                NULL) {
                all_markers_found = 0;
                break;
            }
        }
    } else {
        all_markers_found = 0;
    }

    /* Then */
    if (all_markers_found) {
        printf("[PASS] Navigation début/fin et pas de 10 à 50 présents dans le "
               "HTML.\n");
    } else {
        printf("[FAIL] Les commandes de navigation ne sont pas toutes "
               "générées.\n");
    }
    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(filename);
    return all_markers_found;
}

/* Vérifie que les destinations du défilement sont des choix radio. */
static int test_renders_scroll_destinations(void) {
    /* Given */
    const char *filename = "tmp_html_report_scroll.html";
    const char *payload = "scroll";
    const char *special_value = "citation";
    const ReportInput input = {.payload = payload,
                               .special_value = special_value};
    const char *expected_scroll_markers[] = {
        "role=\"radiogroup\" aria-label=\"Défilement automatique\"",
        "id=\"scroll-none\" type=\"radio\" name=\"section-scroll-target\" "
        "value=\"\" checked",
        "id=\"scroll-repositories\" type=\"radio\" "
        "name=\"section-scroll-target\" value=\"repository-section\"",
        "id=\"scroll-citations\" type=\"radio\" "
        "name=\"section-scroll-target\" value=\"citation-section\""};
    SourceList sources;
    RepositoryList repositories;
    char contents[8192];
    int inputs_ready;
    int report_read;
    int all_markers_found = 1;

    /* When */
    inputs_ready = prepare_report_inputs(&sources, &repositories, &input);
    report_read =
        inputs_ready && read_generated_report(filename, &sources, &repositories,
                                              contents, sizeof(contents));
    if (report_read) {
        for (size_t marker_index = 0U;
             marker_index < sizeof(expected_scroll_markers) /
                                sizeof(expected_scroll_markers[0]);
             marker_index++) {
            if (strstr(contents, expected_scroll_markers[marker_index]) ==
                NULL) {
                all_markers_found = 0;
                break;
            }
        }
    } else {
        all_markers_found = 0;
    }

    /* Then */
    if (all_markers_found) {
        printf("[PASS] Les destinations de défilement sont des choix radio "
               "exclusifs.\n");
    } else {
        printf("[FAIL] Les destinations de défilement ne sont pas toutes "
               "générées.\n");
    }
    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(filename);
    return all_markers_found;
}

/* Vérifie que les commandes apparaissent dans leur ordre visuel prévu. */
static int test_preserves_control_order(void) {
    /* Given */
    const char *filename = "tmp_html_report_control_order.html";
    const char *payload = "order";
    const char *special_value = "citation";
    const ReportInput input = {.payload = payload,
                               .special_value = special_value};
    const char *expected_control_order[] = {
        "id=\"scroll-none\"",      "id=\"scroll-repositories\"",
        "id=\"scroll-citations\"", "id=\"first-source\"",
        "id=\"jump-back\"",        "id=\"previous-source\"",
        "id=\"page-position\"",    "id=\"next-source\"",
        "id=\"jump-forward\"",     "id=\"last-source\"",
        "id=\"page-step\""};
    SourceList sources;
    RepositoryList repositories;
    char contents[8192];
    int inputs_ready;
    int report_read;
    int order_is_correct = 1;

    /* When */
    inputs_ready = prepare_report_inputs(&sources, &repositories, &input);
    report_read =
        inputs_ready && read_generated_report(filename, &sources, &repositories,
                                              contents, sizeof(contents));
    if (report_read) {
        const char *previous = NULL;

        for (size_t marker_index = 0U;
             marker_index <
             sizeof(expected_control_order) / sizeof(expected_control_order[0]);
             marker_index++) {
            const char *current =
                strstr(contents, expected_control_order[marker_index]);
            if (current == NULL || (previous != NULL && current <= previous)) {
                order_is_correct = 0;
                break;
            }
            previous = current;
        }
    } else {
        order_is_correct = 0;
    }

    /* Then */
    if (order_is_correct) {
        printf("[PASS] Ordre gauche-droite des commandes conforme.\n");
    } else {
        printf("[FAIL] Ordre inattendu des commandes de navigation.\n");
    }
    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(filename);
    return order_is_correct;
}

/* Vérifie le rejet d'un template sans marqueur de données. */
static int test_rejects_template_without_marker(void) {
    /* Given */
    const int expected_rejected = 1;
    const TemplateInput input = {
        .contents = "<!doctype html><p>Missing data marker</p>\n",
        .template_filename = "tmp_html_report_template_without_marker.html",
        .output_filename = "tmp_html_report_invalid_without_marker.html"};
    int rejected;

    /* When */
    rejected = rejects_template(&input);

    /* Then */
    if (rejected == expected_rejected) {
        printf("[PASS] Rejet d'un template sans marqueur.\n");
        return 1;
    }
    printf("[FAIL] Rejet d'un template sans marqueur.\n");
    return 0;
}

/* Vérifie le rejet d'un template avec plusieurs marqueurs de données. */
static int test_rejects_template_with_duplicate_markers(void) {
    /* Given */
    const int expected_rejected = 1;
    const TemplateInput input = {
        .contents = "{{GEDCOM_REPORT_DATA}} {{GEDCOM_REPORT_DATA}}\n",
        .template_filename = "tmp_html_report_template_duplicate_markers.html",
        .output_filename = "tmp_html_report_invalid_duplicate_markers.html"};
    int rejected;

    /* When */
    rejected = rejects_template(&input);

    /* Then */
    if (rejected == expected_rejected) {
        printf("[PASS] Rejet d'un template avec marqueurs dupliqués.\n");
        return 1;
    }
    printf("[FAIL] Rejet d'un template avec marqueurs dupliqués.\n");
    return 0;
}

/* Exécute les tests unitaires du rapport HTML. */
int main(void) {
    const int total_tests = 9;
    int passed_tests = 0;

    passed_tests += run_test("test_rejects_empty_template_path",
                             test_rejects_empty_template_path);
    passed_tests += run_test("test_rejects_empty_output_path",
                             test_rejects_empty_output_path);
    passed_tests += run_test("test_rejects_null_report_data",
                             test_rejects_null_report_data);
    passed_tests +=
        run_test("test_escapes_report_data", test_escapes_report_data);
    passed_tests += run_test("test_renders_navigation_controls",
                             test_renders_navigation_controls);
    passed_tests += run_test("test_renders_scroll_destinations",
                             test_renders_scroll_destinations);
    passed_tests +=
        run_test("test_preserves_control_order", test_preserves_control_order);
    passed_tests += run_test("test_rejects_template_without_marker",
                             test_rejects_template_without_marker);
    passed_tests += run_test("test_rejects_template_with_duplicate_markers",
                             test_rejects_template_with_duplicate_markers);

    printf("Résultat html_report: %d/%d réussis.\n", passed_tests, total_tests);
    return passed_tests == total_tests ? 0 : 1;
}