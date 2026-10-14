#include "source_list.h"
#include "string_utils.h"

#include <stdio.h>
#include <string.h>

/* Helpers de test */

/* Exécute un test et rapporte son résultat sous son nom. */
static int run_test(const char *test_name, int (*test_function)(void)) {
    int passed = test_function();

    printf("[%s] %s\n", passed ? "PASS" : "FAIL", test_name);
    return passed;
}

/* Vérifie que l'initialisation crée une liste vide. */
static int test_initial_state(void) {
    /* Given */
    const size_t expected_count = 0U;
    SourceList sources;

    /* When */
    init_source_list(&sources);

    /* Then */
    if (sources.sources == NULL && sources.count == expected_count &&
        sources.capacity == expected_count) {
        free_source_list(&sources);
        printf("[PASS] Initialisation de la liste\n");
        return 1;
    }
    printf("[FAIL] Initialisation de la liste\n");
    free_source_list(&sources);
    return 0;
}

/* Vérifie qu'une liste NULL est rejetée à l'ajout. */
static int test_rejects_null_list(void) {
    /* Given */
    const int expected_add_result = 0;
    const SourceInput valid_input = {
        .description = "Description",
        .reference = "@S0@",
        .kind = "entity",
        .context = "entity",
    };

    /* When */
    int add_result = source_list_add(NULL, &valid_input);

    /* Then */
    if (add_result == expected_add_result) {
        printf("[PASS] Rejet d'une liste NULL\n");
        return 1;
    }
    printf("[FAIL] Rejet d'une liste NULL\n");
    return 0;
}

/* Vérifie qu'une entrée sans description est rejetée. */
static int test_rejects_missing_description(void) {
    /* Given */
    const int expected_add_result = 0;
    const SourceInput invalid_input = {
        .reference = "@S0@",
        .kind = "entity",
        .context = "entity",
    };
    SourceList sources;
    int add_result;

    /* When */
    init_source_list(&sources);
    add_result = source_list_add(&sources, &invalid_input);

    /* Then */
    free_source_list(&sources);
    if (add_result == expected_add_result) {
        printf("[PASS] Rejet d'une description manquante\n");
        return 1;
    }
    printf("[FAIL] Rejet d'une description manquante\n");
    return 0;
}

/* Vérifie la croissance de la liste et les valeurs de la dernière entrée. */
static int test_grows_and_stores_values(void) {
    /* Given */
    const size_t expected_source_count = 12U;
    const size_t expected_minimum_capacity = 12U;
    const size_t expected_last_index = 11U;
    const char *expected_description = "Description 11";
    const char *expected_reference = "@S11@";
    const char *expected_kind = "entity";
    const char *expected_context = "entity";
    SourceList sources;
    size_t index;
    int added_all_sources = 1;

    /* When */
    init_source_list(&sources);
    for (index = 0U; index < expected_source_count; index++) {
        char description[32];
        char reference[32];
        SourceInput input;

        snprintf(description, sizeof(description), "Description %zu", index);
        snprintf(reference, sizeof(reference), "@S%zu@", index);
        input.description = description;
        input.reference = reference;
        input.kind = "entity";
        input.context = "entity";
        if (!source_list_add(&sources, &input)) {
            added_all_sources = 0;
            break;
        }
    }

    /* Then */
    if (added_all_sources && sources.count == expected_source_count &&
        sources.capacity >= expected_minimum_capacity &&
        strcmp(sources.sources[expected_last_index].description,
               expected_description) == 0 &&
        strcmp(sources.sources[expected_last_index].reference,
               expected_reference) == 0 &&
        strcmp(sources.sources[expected_last_index].kind, expected_kind) == 0 &&
        strcmp(sources.sources[expected_last_index].context,
               expected_context) == 0) {
        free_source_list(&sources);
        printf("[PASS] Croissance et valeurs stockées dans la liste\n");
        return 1;
    }
    printf("[FAIL] Croissance ou valeurs stockées dans la liste\n");
    free_source_list(&sources);
    return 0;
}

/* Vérifie qu'une entrée partielle est annulée après un échec d'allocation. */
static int test_cleans_up_after_allocation_failure(void) {
    /* Given */
    const size_t expected_count = 0U;
    const int expected_add_result = 0;
    const char *expected_unallocated_value = NULL;
    const SourceInput allocation_failure_input = {
        .description = "Description sans référence allouée",
        .reference = "@S-fail@",
        .kind = "entity",
        .context = "entity",
    };
    SourceList sources;
    int add_result;
    int partial_entry_cleared;

    /* When */
    init_source_list(&sources);
    string_utils_test_fail_after(1U);
    add_result = source_list_add(&sources, &allocation_failure_input);
    string_utils_test_reset_failure();
    partial_entry_cleared =
        sources.count == expected_count &&
        sources.sources[0].description == expected_unallocated_value &&
        sources.sources[0].reference == expected_unallocated_value &&
        sources.sources[0].kind == expected_unallocated_value &&
        sources.sources[0].context == expected_unallocated_value;

    /* Then */
    if (add_result == expected_add_result && partial_entry_cleared) {
        free_source_list(&sources);
        printf("[PASS] Nettoyage après échec d'allocation\n");
        return 1;
    }
    printf("[FAIL] Nettoyage après échec d'allocation\n");
    free_source_list(&sources);
    return 0;
}

/* Vérifie que la libération remet la liste dans son état initial. */
static int test_free_resets_list(void) {
    /* Given */
    const size_t expected_count = 0U;
    SourceList sources;

    /* When */
    init_source_list(&sources);
    free_source_list(&sources);

    /* Then */
    if (sources.sources == NULL && sources.count == expected_count &&
        sources.capacity == expected_count) {
        printf("[PASS] Libération et remise à zéro de la liste\n");
        return 1;
    }
    printf("[FAIL] Libération et remise à zéro de la liste\n");
    return 0;
}

/* Exécute les tests unitaires de SourceList. */
int main(void) {
    const int total_tests = 6;
    int passed_tests = 0;

    passed_tests += run_test("test_initial_state", test_initial_state);
    passed_tests += run_test("test_rejects_null_list", test_rejects_null_list);
    passed_tests += run_test("test_rejects_missing_description",
                             test_rejects_missing_description);
    passed_tests +=
        run_test("test_grows_and_stores_values", test_grows_and_stores_values);
    passed_tests += run_test("test_cleans_up_after_allocation_failure",
                             test_cleans_up_after_allocation_failure);
    passed_tests += run_test("test_free_resets_list", test_free_resets_list);

    printf("Résultat source_list: %d/%d réussis.\n", passed_tests, total_tests);
    return passed_tests == total_tests ? 0 : 1;
}