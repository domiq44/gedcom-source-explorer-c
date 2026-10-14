#include "string_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    char *trimmed_text;

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

/* Vérifie le remplacement d'une chaîne puis son effacement par NULL. */
static int test_string_assignment(void) {
    /* Given */
    const char *expected_assigned_text = "new value";
    const char *expected_cleared_value = NULL;
    char *assigned;
    int assigned_value_matches;
    int cleared_value_matches;

    /* When */
    assigned = xstrdup("old value");
    assign_string(&assigned, expected_assigned_text);
    assigned_value_matches =
        assigned != NULL && strcmp(assigned, expected_assigned_text) == 0;
    assign_string(&assigned, NULL);
    cleared_value_matches = assigned == expected_cleared_value;

    /* Then */
    if (assigned_value_matches && cleared_value_matches) {
        printf("[PASS] Remplacement et effacement d'une chaîne\n");
        free(assigned);
        return 1;
    }
    printf("[FAIL] Remplacement ou effacement d'une chaîne\n");
    free(assigned);
    return 0;
}

/* Vérifie le retrait du BOM sans modifier une chaîne ordinaire. */
static int test_strip_utf8_bom(void) {
    /* Given */
    const char *expected_bom_text = "GED";
    const char *expected_plain_text = "GEDCOM";
    char bom_text[] = {(char)0xEF, (char)0xBB, (char)0xBF, 'G', 'E', 'D', '\0'};
    char plain_text[] = "GEDCOM";

    /* When */
    strip_utf8_bom(bom_text);
    strip_utf8_bom(plain_text);

    /* Then */
    if (strcmp(bom_text, expected_bom_text) == 0 &&
        strcmp(plain_text, expected_plain_text) == 0) {
        printf("[PASS] Retrait du BOM UTF-8\n");
        return 1;
    }
    printf("[FAIL] Retrait du BOM UTF-8\n");
    return 0;
}

/* Exécute les tests unitaires des utilitaires de chaînes. */
int main(void) {
    const int total_tests = 4;
    int passed_tests = 0;

    passed_tests += test_string_duplication();
    passed_tests += test_trim_whitespace();
    passed_tests += test_string_assignment();
    passed_tests += test_strip_utf8_bom();

    printf("Résultat string_utils: %d/%d réussis.\n", passed_tests,
           total_tests);
    return passed_tests == total_tests ? 0 : 1;
}