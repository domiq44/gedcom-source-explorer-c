#include "source_order.h"

#include <stdio.h>

typedef struct {
    const char *left;
    const char *right;
} ReferencePair;

/* Helpers de test */

/* Exécute un test et rapporte son résultat sous son nom. */
static int run_test(const char *test_name, int (*test_function)(void)) {
    int passed = test_function();

    printf("[%s] %s\n", passed ? "PASS" : "FAIL", test_name);
    return passed;
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
    return actual_before == expected_before;
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
    return actual_before == expected_before;
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
    return actual_before == expected_before;
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
    return actual_before == expected_before;
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
    return actual_before == expected_before;
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
    return actual_before == expected_before;
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
    return actual_before == expected_before;
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
    return actual_before == expected_before;
}

/* Exécute les tests unitaires de comparaison des références GEDCOM. */
int main(void) {
    const int total = 8;
    int passed = 0;

    passed += run_test("test_numeric_order", test_numeric_order);
    passed += run_test("test_letter_order", test_letter_order);
    passed += run_test("test_leading_zero_order", test_leading_zero_order);
    passed += run_test("test_large_suffix_order", test_large_suffix_order);
    passed += run_test("test_null_order", test_null_order);
    passed += run_test("test_missing_suffix_order", test_missing_suffix_order);
    passed += run_test("test_malformed_reference_order",
                       test_malformed_reference_order);
    passed +=
        run_test("test_case_insensitive_order", test_case_insensitive_order);

    printf("Résultat source_order: %d/%d réussis.\n", passed, total);
    return (passed == total) ? 0 : 1;
}