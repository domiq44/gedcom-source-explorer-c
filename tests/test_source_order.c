#include "source_order.h"

#include <stdio.h>

typedef struct {
    const char *left;
    const char *right;
} ReferencePair;

/* Vérifie et affiche l'ordre attendu d'une paire de références. */
static int expect_before(ReferencePair references, const char *label) {
    const char *left_text =
        (references.left != NULL) ? references.left : "(NULL)";
    const char *right_text =
        (references.right != NULL) ? references.right : "(NULL)";

    if (compare_gedcom_references(references.left, references.right) < 0) {
        printf("[PASS] %s\n", label);
        return 1;
    }

    printf("[FAIL] %s: '%s' should sort before '%s'\n", label, left_text,
           right_text);
    return 0;
}

/* Vérifie que les références sont ordonnées selon leur suffixe numérique. */
static int test_numeric_order(void) {
    /* Given */
    const ReferencePair references = {"@S2@", "@S10@"};
    const int expected_before = 1;
    int actual_before;

    /* When */
    actual_before =
        compare_gedcom_references(references.left, references.right) < 0;

    /* Then */
    return actual_before == expected_before &&
           expect_before(references, "Ordre numérique, pas lexicographique");
}

/* Vérifie que la lettre de référence précède le suffixe numérique. */
static int test_letter_order(void) {
    /* Given */
    const ReferencePair references = {"@S9@", "@T1@"};
    const int expected_before = 1;
    int actual_before;

    /* When */
    actual_before =
        compare_gedcom_references(references.left, references.right) < 0;

    /* Then */
    return actual_before == expected_before &&
           expect_before(references, "Ordre par lettre puis par nombre");
}

/* Vérifie le départage lexical des références aux suffixes équivalents. */
static int test_leading_zero_order(void) {
    /* Given */
    const ReferencePair references = {"@S002@", "@S2@"};
    const int expected_before = 1;
    int actual_before;

    /* When */
    actual_before =
        compare_gedcom_references(references.left, references.right) < 0;

    /* Then */
    return actual_before == expected_before &&
           expect_before(references,
                         "Départage lexical stable des zéros initiaux");
}

/* Vérifie le tri des suffixes trop grands pour un entier natif. */
static int test_large_suffix_order(void) {
    /* Given */
    const ReferencePair references = {"@S99999999999999999999@",
                                      "@S100000000000000000000@"};
    const int expected_before = 1;
    int actual_before;

    /* When */
    actual_before =
        compare_gedcom_references(references.left, references.right) < 0;

    /* Then */
    return actual_before == expected_before &&
           expect_before(references,
                         "Grand identifiant sans débordement numérique");
}

/* Vérifie que NULL précède une référence GEDCOM valide. */
static int test_null_order(void) {
    /* Given */
    const ReferencePair references = {NULL, "@S1@"};
    const int expected_before = 1;
    int actual_before;

    /* When */
    actual_before =
        compare_gedcom_references(references.left, references.right) < 0;

    /* Then */
    return actual_before == expected_before &&
           expect_before(references,
                         "Référence NULL avant une référence valide");
}

/* Vérifie l'ordre d'une référence sans suffixe numérique. */
static int test_missing_suffix_order(void) {
    /* Given */
    const ReferencePair references = {"@S@", "@S1@"};
    const int expected_before = 1;
    int actual_before;

    /* When */
    actual_before =
        compare_gedcom_references(references.left, references.right) < 0;

    /* Then */
    return actual_before == expected_before &&
           expect_before(references, "Référence sans suffixe numérique");
}

/* Vérifie le tri lexical de références mal formées. */
static int test_malformed_reference_order(void) {
    /* Given */
    const ReferencePair references = {"@S1@", "@Sx@"};
    const int expected_before = 1;
    int actual_before;

    /* When */
    actual_before =
        compare_gedcom_references(references.left, references.right) < 0;

    /* Then */
    return actual_before == expected_before &&
           expect_before(references, "Référence mal formée triée lexicalement");
}

/* Vérifie que la lettre de référence est comparée sans distinction de casse. */
static int test_case_insensitive_order(void) {
    /* Given */
    const ReferencePair references = {"@s2@", "@S10@"};
    const int expected_before = 1;
    int actual_before;

    /* When */
    actual_before =
        compare_gedcom_references(references.left, references.right) < 0;

    /* Then */
    return actual_before == expected_before &&
           expect_before(references,
                         "Lettre de référence insensible à la casse");
}

/* Exécute les tests unitaires de comparaison des références GEDCOM. */
int main(void) {
    const int total = 8;
    int passed = 0;

    passed += test_numeric_order();
    passed += test_letter_order();
    passed += test_leading_zero_order();
    passed += test_large_suffix_order();
    passed += test_null_order();
    passed += test_missing_suffix_order();
    passed += test_malformed_reference_order();
    passed += test_case_insensitive_order();

    printf("Résultat source_order: %d/%d réussis.\n", passed, total);
    return (passed == total) ? 0 : 1;
}