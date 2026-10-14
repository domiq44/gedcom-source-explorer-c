#include "repository_list.h"

#include <stdio.h>
#include <string.h>

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

/* Vérifie l'ajout et la mise à jour des dépôts et de leurs liens. */
static int test_add_and_update(void) {
    /* Given */
    const size_t expected_repository_count = 2U;
    const size_t expected_link_count = 1U;
    const char *expected_first_reference = "@R1@";
    const char *expected_first_name = "@R1@";
    const char *expected_second_name = "Archives";
    const char *expected_source_reference = "@S1@";
    const char *expected_repository_reference = "@R1@";
    const char *expected_context = "@S1@ SOUR / REPO";
    RepositoryList repositories;
    int added;
    int updated;
    int passed;

    /* When */
    init_repository_list(&repositories);
    added = repository_list_add(&repositories, "@R1@", "@R1@") &&
            repository_list_add(&repositories, "@R2@", "Nom temporaire") &&
            repository_list_add_link(&repositories, "@S1@", "@R1@",
                                     "@S1@ SOUR / REPO");
    updated = repository_list_set_name(&repositories, "@R2@", "Archives");

    /* Then */
    passed =
        added && updated && repositories.count == expected_repository_count &&
        strcmp(repositories.repositories[0].reference,
               expected_first_reference) == 0 &&
        strcmp(repositories.repositories[0].name, expected_first_name) == 0 &&
        strcmp(repositories.repositories[1].name, expected_second_name) == 0 &&
        repositories.link_count == expected_link_count &&
        strcmp(repositories.links[0].source_reference,
               expected_source_reference) == 0 &&
        strcmp(repositories.links[0].repository_reference,
               expected_repository_reference) == 0 &&
        strcmp(repositories.links[0].context, expected_context) == 0;
    free_repository_list(&repositories);
    return passed;
}

/* Vérifie que les arguments invalides sont rejetés. */
static int test_rejects_invalid_arguments(void) {
    /* Given */
    const int expected_rejected_result = 0;
    RepositoryList repositories;
    int invalid_add_rejected;
    int unknown_name_rejected;

    /* When */
    init_repository_list(&repositories);
    invalid_add_rejected = repository_list_add(&repositories, NULL, "Nom") ==
                           expected_rejected_result;
    unknown_name_rejected =
        repository_list_set_name(&repositories, "@R3@", "Inconnu") ==
        expected_rejected_result;
    free_repository_list(&repositories);

    /* Then */
    if (invalid_add_rejected && unknown_name_rejected) {
        printf("[PASS] Rejet des arguments invalides\n");
        return 1;
    }
    printf("[FAIL] Rejet des arguments invalides\n");
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
    const int total = 4;
    int passed = 0;

    passed += test_initial_state();
    passed += test_add_and_update();
    passed += test_rejects_invalid_arguments();
    passed += test_free_resets_list();

    if (passed) {
        printf("[PASS] Initialisation, ajout, nom et libération de "
               "RepositoryList\n");
    }
    return passed == total ? 0 : 1;
}