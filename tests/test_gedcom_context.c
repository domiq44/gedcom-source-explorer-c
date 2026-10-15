#include "gedcom_context.h"

#include <stdio.h>
#include <string.h>

/* Helpers de test */

/* Exécute un test et rapporte son résultat sous son nom. */
static int run_test(const char *test_name, int (*test_function)(void)) {
    int passed = test_function();

    printf("[%s] %s\n", passed ? "PASS" : "FAIL", test_name);
    return passed;
}

/* Vérifie le formatage du contexte avec identifiant et tag parent. */
static int test_context_with_parent_tag(void) {
    /* Given */
    const char *expected_context = "@I1@ INDI / NAME";
    char context[128];

    /* When */
    build_gedcom_context("@I1@", "INDI", "NAME", context, sizeof(context));

    /* Then */
    return strcmp(context, expected_context) == 0;
}

/* Vérifie le formatage du contexte sans tag parent. */
static int test_context_without_parent_tag(void) {
    /* Given */
    const char *expected_context = "@I1@ INDI";
    char context[128];

    /* When */
    build_gedcom_context("@I1@", "INDI", "", context, sizeof(context));

    /* Then */
    return strcmp(context, expected_context) == 0;
}

/* Vérifie le contexte de repli quand l'identifiant manque. */
static int test_context_without_record_identifier(void) {
    /* Given */
    const char *expected_context = "unknown";
    char context[128];

    /* When */
    build_gedcom_context("", "INDI", "NAME", context, sizeof(context));

    /* Then */
    return strcmp(context, expected_context) == 0;
}

/* Exécute les tests unitaires de construction du contexte GEDCOM. */
int main(void) {
    const int total = 3;
    int passed = 0;

    passed +=
        run_test("test_context_with_parent_tag", test_context_with_parent_tag);
    passed += run_test("test_context_without_parent_tag",
                       test_context_without_parent_tag);
    passed += run_test("test_context_without_record_identifier",
                       test_context_without_record_identifier);

    printf("Résultat context: %d/%d réussis.\n", passed, total);
    return (passed == total) ? 0 : 1;
}