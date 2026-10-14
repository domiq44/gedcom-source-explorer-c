#include "html_report.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prépare les sources communes aux tests du rendu HTML. */
static int prepare_report_inputs(SourceList *sources,
                                 RepositoryList *repositories,
                                 const char *payload,
                                 const char *special_value) {
    const SourceInput source = {
        .description = payload,
        .reference = "@S1@",
        .kind = "entity",
        .context = "entity",
    };
    const SourceInput special_citation = {
        .description = special_value,
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

/* Vérifie que les arguments invalides du générateur sont rejetés. */
static int test_rejects_invalid_report_arguments(void) {
    /* Given */
    const char *valid_template_filename = "web/report.html";
    const char *valid_output_filename = "tmp_html_report_invalid_args.html";
    const int expected_success_status = 0;
    const int expected_rejected = 1;
    ReportData empty_data = {0};
    int empty_template_rejected;
    int empty_output_rejected;
    int null_data_rejected;

    /* When */
    empty_template_rejected =
        write_html_report_data("", valid_output_filename, &empty_data) !=
        expected_success_status;
    empty_output_rejected =
        write_html_report_data(valid_template_filename, "", &empty_data) !=
        expected_success_status;
    null_data_rejected =
        write_html_report_data(valid_template_filename, valid_output_filename,
                               NULL) != expected_success_status;

    /* Then */
    if (empty_template_rejected == expected_rejected &&
        empty_output_rejected == expected_rejected &&
        null_data_rejected == expected_rejected) {
        printf("[PASS] Les erreurs de saisie sur le rapport sont rejetées.\n");
        return 1;
    }
    printf(
        "[FAIL] Les erreurs de saisie sur le rapport ne sont pas rejetées.\n");
    remove(valid_output_filename);
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
    SourceList sources;
    RepositoryList repositories;
    char contents[8192];
    int inputs_ready;
    int report_read;
    int passed;

    /* When */
    inputs_ready =
        prepare_report_inputs(&sources, &repositories, payload, special_value);
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
    SourceList sources;
    RepositoryList repositories;
    char contents[8192];
    int inputs_ready;
    int report_read;
    int all_markers_found = 1;
    size_t marker_index;

    /* When */
    inputs_ready =
        prepare_report_inputs(&sources, &repositories, payload, special_value);
    report_read =
        inputs_ready && read_generated_report(filename, &sources, &repositories,
                                              contents, sizeof(contents));
    if (report_read) {
        for (marker_index = 0U;
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
    size_t marker_index;

    /* When */
    inputs_ready =
        prepare_report_inputs(&sources, &repositories, payload, special_value);
    report_read =
        inputs_ready && read_generated_report(filename, &sources, &repositories,
                                              contents, sizeof(contents));
    if (report_read) {
        for (marker_index = 0U;
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
    const char *previous = NULL;
    size_t marker_index;

    /* When */
    inputs_ready =
        prepare_report_inputs(&sources, &repositories, payload, special_value);
    report_read =
        inputs_ready && read_generated_report(filename, &sources, &repositories,
                                              contents, sizeof(contents));
    if (report_read) {
        for (marker_index = 0U;
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

/* Vérifie le rejet des templates sans marqueur unique de données. */
static int test_rejects_invalid_templates(void) {
    /* Given */
    const char *invalid_template_filename = "tmp_html_report_template.html";
    const char *invalid_output_filename = "tmp_html_report_invalid.html";
    const int expected_success_status = 0;
    const int expected_rejected = 1;
    const char *template_contents[] = {
        "<!doctype html><p>Missing data marker</p>\n",
        "{{GEDCOM_REPORT_DATA}} {{GEDCOM_REPORT_DATA}}\n",
    };
    SourceList sources;
    RepositoryList repositories;
    size_t template_index;
    int all_templates_rejected = 1;

    /* When */
    init_source_list(&sources);
    init_repository_list(&repositories);
    for (template_index = 0U; template_index < sizeof(template_contents) /
                                                   sizeof(template_contents[0]);
         template_index++) {
        FILE *template_file = fopen(invalid_template_filename, "w");
        int write_status;
        int close_status;

        if (template_file == NULL) {
            all_templates_rejected = 0;
            break;
        }
        write_status = fputs(template_contents[template_index], template_file);
        close_status = fclose(template_file);
        if (write_status == EOF || close_status != 0) {
            all_templates_rejected = 0;
            remove(invalid_template_filename);
            break;
        }
        if (write_html_report(invalid_template_filename,
                              invalid_output_filename, &sources,
                              &repositories) == expected_success_status) {
            all_templates_rejected = 0;
        }
        remove(invalid_template_filename);
        remove(invalid_output_filename);
    }

    /* Then */
    if (all_templates_rejected == expected_rejected) {
        printf("[PASS] Les templates sans marqueur unique sont rejetés.\n");
    } else {
        printf("[FAIL] Un template invalide a été accepté ou n'a pas pu être "
               "testé.\n");
    }
    free_source_list(&sources);
    free_repository_list(&repositories);
    remove(invalid_template_filename);
    remove(invalid_output_filename);
    return all_templates_rejected;
}

/* Exécute les tests unitaires du rapport HTML. */
int main(void) {
    const int total_tests = 6;
    int passed_tests = 0;

    passed_tests += test_rejects_invalid_report_arguments();
    passed_tests += test_escapes_report_data();
    passed_tests += test_renders_navigation_controls();
    passed_tests += test_renders_scroll_destinations();
    passed_tests += test_preserves_control_order();
    passed_tests += test_rejects_invalid_templates();

    printf("Résultat html_report: %d/%d réussis.\n", passed_tests, total_tests);
    return passed_tests == total_tests ? 0 : 1;
}