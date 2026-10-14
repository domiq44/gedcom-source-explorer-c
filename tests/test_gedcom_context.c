#include "gedcom_context.h"

#include <stdio.h>
#include <string.h>

/* Compare le contexte produit à la valeur attendue et affiche le résultat. */
static int check_context(const char *actual, const char *expected,
                         const char *test_name) {
    if (strcmp(actual, expected) == 0) {
        printf("[PASS] %s\n", test_name);
        return 1;
    }

    printf("[FAIL] %s - attendu '%s', obtenu '%s'\n", test_name, expected,
           actual);
    return 0;
}

/* Vérifie le formatage du contexte avec identifiant et tag parent. */
static int test_context_with_parent_tag(void) {
    /* Given */
    const char *expected_context = "@I1@ INDI / NAME";
    char context[128];

    /* When */
    build_gedcom_context("@I1@", "INDI", "NAME", context, sizeof(context));

    /* Then */
    return check_context(context, expected_context, "Contexte avec tag parent");
}

/* Vérifie le formatage du contexte sans tag parent. */
static int test_context_without_parent_tag(void) {
    /* Given */
    const char *expected_context = "@I1@ INDI";
    char context[128];

    /* When */
    build_gedcom_context("@I1@", "INDI", "", context, sizeof(context));

    /* Then */
    return check_context(context, expected_context, "Contexte sans tag parent");
}

/* Vérifie le contexte de repli quand l'identifiant manque. */
static int test_context_without_record_identifier(void) {
    /* Given */
    const char *expected_context = "unknown";
    char context[128];

    /* When */
    build_gedcom_context("", "INDI", "NAME", context, sizeof(context));

    /* Then */
    return check_context(context, expected_context,
                         "Contexte sans identifiant d'enregistrement");
}

/* Exécute les tests unitaires de construction du contexte GEDCOM. */
int main(void) {
    const int total = 3;
    int passed = 0;

    passed += test_context_with_parent_tag();
    passed += test_context_without_parent_tag();
    passed += test_context_without_record_identifier();

    printf("Résultat context: %d/%d réussis.\n", passed, total);
    return (passed == total) ? 0 : 1;
}