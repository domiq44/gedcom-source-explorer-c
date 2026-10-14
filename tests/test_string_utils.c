#include "string_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helpers de test */

/* Exécute un test et rapporte son résultat sous son nom. */
static int run_test(const char *test_name, int (*test_function)(void)) {
    int passed = test_function();

    printf("[%s] %s\n", passed ? "PASS" : "FAIL", test_name);
    return passed;
}

/* Vérifie que xstrdup retourne la valeur attendue. */
static int test_string_duplication(void) {
    /* Given */
    const char *expected_copy = "GEDCOM";
    char *copy;

    /* When */
    copy = xstrdup(expected_copy);

    /* Then */
    if (copy != NULL && strcmp(copy, expected_copy) == 0) {
        free(copy);
        printf("[PASS] Duplication de chaîne\n");
        return 1;
    }
    printf("[FAIL] Duplication de chaîne\n");
    free(copy);
    return 0;
}

/* Vérifie que trim_whitespace retire les espaces périphériques. */
static int test_trim_whitespace(void) {
    /* Given */
    const char *expected_trimmed_text = "GEDCOM data";
    char padded_text[] = " \tGEDCOM data \r\n";
    const char *trimmed_text;

    /* When */
    trimmed_text = trim_whitespace(padded_text);

    /* Then */
    if (strcmp(trimmed_text, expected_trimmed_text) == 0) {
        printf("[PASS] Suppression des espaces périphériques\n");
        return 1;
    }
    printf("[FAIL] Suppression des espaces périphériques\n");
    return 0;
}

/* Vérifie le remplacement d'une chaîne déjà affectée. */
static int test_string_assignment(void) {
    /* Given */
    const char *initial_text = "old value";
    const char *expected_assigned_text = "new value";
    char *assigned;

    /* When */
    assigned = xstrdup(initial_text);
    assign_string(&assigned, expected_assigned_text);

    /* Then */
    if (assigned != NULL && strcmp(assigned, expected_assigned_text) == 0) {
        printf("[PASS] Remplacement d'une chaîne\n");
        free(assigned);
        return 1;
    }
    printf("[FAIL] Remplacement d'une chaîne\n");
    free(assigned);
    return 0;
}

/* Vérifie que assign_string efface une valeur lorsque la source est NULL. */
static int test_string_assignment_clears_null(void) {
    /* Given */
    const char *initial_text = "old value";
    const char *expected_cleared_value = NULL;
    char *assigned;

    /* When */
    assigned = xstrdup(initial_text);
    assign_string(&assigned, NULL);

    /* Then */
    if (assigned == expected_cleared_value) {
        printf("[PASS] Effacement d'une chaîne avec NULL\n");
        free(assigned);
        return 1;
    }
    printf("[FAIL] Effacement d'une chaîne avec NULL\n");
    free(assigned);
    return 0;
}

/* Vérifie le retrait du BOM UTF-8 au début d'une chaîne. */
static int test_strip_utf8_bom(void) {
    /* Given */
    const char *expected_bom_text = "GED";
    char bom_text[] = {(char)0xEF, (char)0xBB, (char)0xBF, 'G', 'E', 'D', '\0'};

    /* When */
    strip_utf8_bom(bom_text);

    /* Then */
    if (strcmp(bom_text, expected_bom_text) == 0) {
        printf("[PASS] Retrait du BOM UTF-8\n");
        return 1;
    }
    printf("[FAIL] Retrait du BOM UTF-8\n");
    return 0;
}

/* Vérifie qu'une chaîne sans BOM reste inchangée. */
static int test_strip_utf8_bom_preserves_plain_text(void) {
    /* Given */
    const char *expected_plain_text = "GEDCOM";
    char plain_text[] = "GEDCOM";

    /* When */
    strip_utf8_bom(plain_text);

    /* Then */
    if (strcmp(plain_text, expected_plain_text) == 0) {
        printf("[PASS] Conservation d'une chaîne sans BOM\n");
        return 1;
    }
    printf("[FAIL] Conservation d'une chaîne sans BOM\n");
    return 0;
}

/* Exécute les tests unitaires des utilitaires de chaînes. */
int main(void) {
    const int total_tests = 6;
    int passed_tests = 0;

    passed_tests +=
        run_test("test_string_duplication", test_string_duplication);
    passed_tests += run_test("test_trim_whitespace", test_trim_whitespace);
    passed_tests += run_test("test_string_assignment", test_string_assignment);
    passed_tests += run_test("test_string_assignment_clears_null",
                             test_string_assignment_clears_null);
    passed_tests += run_test("test_strip_utf8_bom", test_strip_utf8_bom);
    passed_tests += run_test("test_strip_utf8_bom_preserves_plain_text",
                             test_strip_utf8_bom_preserves_plain_text);

    printf("Résultat string_utils: %d/%d réussis.\n", passed_tests,
           total_tests);
    return passed_tests == total_tests ? 0 : 1;
}