#include "gedcom_context.h"
#include "test_reporter.h"

#include <stdio.h>
#include <string.h>

/* Helpers de test */

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
    int test_result;

    test_result = test_context_with_parent_tag();
    report_test_result("test_context_with_parent_tag", test_result);
    passed += test_result;
    test_result = test_context_without_parent_tag();
    report_test_result("test_context_without_parent_tag", test_result);
    passed += test_result;
    test_result = test_context_without_record_identifier();
    report_test_result("test_context_without_record_identifier", test_result);
    passed += test_result;

    printf("Résultat context: %d/%d réussis.\n", passed, total);
    return (passed == total) ? 0 : 1;
}