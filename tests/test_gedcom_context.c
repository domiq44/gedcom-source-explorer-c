#include "gedcom_context.h"
#include "test_runner.h"

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
static const TestCase test_cases[] = {
    {"test_context_with_parent_tag", test_context_with_parent_tag},
    {"test_context_without_parent_tag", test_context_without_parent_tag},
    {"test_context_without_record_identifier",
     test_context_without_record_identifier},
};

int main(void) {
    return run_test_suite("gedcom_context", test_cases,
                          sizeof(test_cases) / sizeof(test_cases[0]));
}