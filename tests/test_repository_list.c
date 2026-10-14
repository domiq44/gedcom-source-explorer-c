#include "repository_list.h"

#include <stdio.h>
#include <string.h>

/* Helpers de test */

/* Exécute un test et rapporte son résultat sous son nom. */
static int run_test(const char *test_name, int (*test_function)(void)) {
    int passed = test_function();

    printf("[%s] %s\n", passed ? "PASS" : "FAIL", test_name);
    return passed;
}

/* Vérifie l'état initial d'une liste de dépôts. */
static int test_initial_state(void) {
    /* Given */
    const size_t expected_count = 0U;
    RepositoryList repositories;

    /* When */
    init_repository_list(&repositories);
    /* Then */
    if (repositories.repositories == NULL &&
        repositories.count == expected_count &&
        repositories.capacity == expected_count && repositories.links == NULL &&
        repositories.link_count == expected_count &&
        repositories.link_capacity == expected_count) {
        free_repository_list(&repositories);
        return 1;
    }
    printf("[FAIL] Initialisation de la liste des dépôts\n");
    free_repository_list(&repositories);
    return 0;
}

/* Vérifie l'ajout d'un dépôt avec sa référence et son nom. */
static int test_add_repository(void) {
    /* Given */
    const size_t expected_repository_count = 1U;
    const char *expected_reference = "@R1@";
    const char *expected_name = "Archives";
    RepositoryList repositories;
    int passed;

    /* When */
    init_repository_list(&repositories);
    passed =
        repository_list_add(&repositories, expected_reference, expected_name);

    /* Then */
    passed = passed && repositories.count == expected_repository_count &&
             strcmp(repositories.repositories[0].reference,
                    expected_reference) == 0 &&
             strcmp(repositories.repositories[0].name, expected_name) == 0;
    free_repository_list(&repositories);
    return passed;
}

/* Vérifie la mise à jour du nom d'un dépôt existant. */
static int test_updates_repository_name(void) {
    /* Given */
    const int expected_update_result = 1;
    const char *expected_name = "Archives";
    RepositoryList repositories;
    int update_result;

    /* When */
    init_repository_list(&repositories);
    repository_list_add(&repositories, "@R2@", "Nom temporaire");
    update_result =
        repository_list_set_name(&repositories, "@R2@", expected_name);

    /* Then */
    update_result =
        update_result == expected_update_result &&
        strcmp(repositories.repositories[0].name, expected_name) == 0;
    free_repository_list(&repositories);
    return update_result;
}

/* Vérifie l'ajout d'un lien entre une source et un dépôt. */
static int test_adds_repository_link(void) {
    /* Given */
    const size_t expected_link_count = 1U;
    const char *expected_source_reference = "@S1@";
    const char *expected_repository_reference = "@R1@";
    const char *expected_context = "@S1@ SOUR / REPO";
    RepositoryList repositories;
    int added;

    /* When */
    init_repository_list(&repositories);
    repository_list_add(&repositories, "@R1@", "Archives");
    added = repository_list_add_link(&repositories, expected_source_reference,
                                     expected_repository_reference,
                                     expected_context);

    /* Then */
    added = added && repositories.link_count == expected_link_count &&
            strcmp(repositories.links[0].source_reference,
                   expected_source_reference) == 0 &&
            strcmp(repositories.links[0].repository_reference,
                   expected_repository_reference) == 0 &&
            strcmp(repositories.links[0].context, expected_context) == 0;
    free_repository_list(&repositories);
    return added;
}

/* Vérifie qu'une référence de dépôt NULL est rejetée. */
static int test_rejects_null_reference(void) {
    /* Given */
    const int expected_add_result = 0;
    RepositoryList repositories;
    int add_result;

    /* When */
    init_repository_list(&repositories);
    add_result = repository_list_add(&repositories, NULL, "Nom");

    /* Then */
    free_repository_list(&repositories);
    if (add_result == expected_add_result) {
        printf("[PASS] Rejet d'une référence NULL\n");
        return 1;
    }
    printf("[FAIL] Rejet d'une référence NULL\n");
    return 0;
}

/* Vérifie que le nom d'un dépôt inconnu ne peut pas être mis à jour. */
static int test_rejects_unknown_repository_name(void) {
    /* Given */
    const int expected_update_result = 0;
    RepositoryList repositories;
    int update_result;

    /* When */
    init_repository_list(&repositories);
    update_result = repository_list_set_name(&repositories, "@R3@", "Inconnu");

    /* Then */
    free_repository_list(&repositories);
    if (update_result == expected_update_result) {
        printf("[PASS] Rejet de la mise à jour d'un dépôt inconnu\n");
        return 1;
    }
    printf("[FAIL] Rejet de la mise à jour d'un dépôt inconnu\n");
    return 0;
}

/* Vérifie que libérer une liste remplie remet ses champs à zéro. */
static int test_free_resets_list(void) {
    /* Given */
    const int expected_add_result = 1;
    const size_t expected_count = 0U;
    RepositoryList repositories;
    int add_result;

    /* When */
    init_repository_list(&repositories);
    add_result = repository_list_add(&repositories, "@R1@", "Archives");
    free_repository_list(&repositories);

    /* Then */
    if (add_result != expected_add_result ||
        repositories.repositories != NULL ||
        repositories.count != expected_count ||
        repositories.capacity != expected_count || repositories.links != NULL ||
        repositories.link_count != expected_count ||
        repositories.link_capacity != expected_count) {
        printf("[FAIL] Libération et remise à zéro de la liste\n");
        return 0;
    }
    printf("[PASS] Libération et remise à zéro de la liste\n");
    return 1;
}

/* Exécute les tests unitaires de RepositoryList. */
int main(void) {
    const int total = 7;
    int passed = 0;

    passed += run_test("test_initial_state", test_initial_state);
    passed += run_test("test_add_repository", test_add_repository);
    passed +=
        run_test("test_updates_repository_name", test_updates_repository_name);
    passed += run_test("test_adds_repository_link", test_adds_repository_link);
    passed +=
        run_test("test_rejects_null_reference", test_rejects_null_reference);
    passed += run_test("test_rejects_unknown_repository_name",
                       test_rejects_unknown_repository_name);
    passed += run_test("test_free_resets_list", test_free_resets_list);

    if (passed) {
        printf("[PASS] Initialisation, ajout, nom et libération de "
               "RepositoryList\n");
    }
    return passed == total ? 0 : 1;
}