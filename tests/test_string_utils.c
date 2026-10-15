#include "string_utils.h"
#include "test_runner.h"

#include <stdlib.h>
#include <string.h>

/* Vérifie que xstrdup retourne la valeur attendue. */
static int test_string_duplication(void) {
    /* Given */
    const char *input_text = "GEDCOM";
    const char *expected_copy = "GEDCOM";
    char *copy;
    int is_independent_copy;

    /* When */
    copy = xstrdup(input_text);

    /* Then */
    is_independent_copy =
        copy != NULL && copy != input_text && strcmp(copy, expected_copy) == 0;
    free(copy);
    return is_independent_copy;
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
    return strcmp(trimmed_text, expected_trimmed_text) == 0;
}

/* Vérifie le remplacement d'une chaîne déjà affectée. */
static int test_string_assignment(void) {
    /* Given */
    const char *initial_text = "old value";
    const char *expected_assigned_text = "new value";
    char *assigned;
    int assigned_expected_text;

    assigned = xstrdup(initial_text);

    /* When */
    assign_string(&assigned, expected_assigned_text);

    /* Then */
    assigned_expected_text =
        assigned != NULL && strcmp(assigned, expected_assigned_text) == 0;
    free(assigned);
    return assigned_expected_text;
}

/* Vérifie que assign_string efface une valeur lorsque la source est NULL. */
static int test_string_assignment_clears_null(void) {
    /* Given */
    const char *initial_text = "old value";
    const char *expected_cleared_value = NULL;
    char *assigned;
    int cleared;

    assigned = xstrdup(initial_text);

    /* When */
    assign_string(&assigned, NULL);

    /* Then */
    cleared = assigned == expected_cleared_value;
    free(assigned);
    return cleared;
}

/* Vérifie le retrait du BOM UTF-8 au début d'une chaîne. */
static int test_strip_utf8_bom(void) {
    /* Given */
    const char *expected_bom_text = "GED";
    char bom_text[] = {(char)0xEF, (char)0xBB, (char)0xBF, 'G', 'E', 'D', '\0'};

    /* When */
    strip_utf8_bom(bom_text);

    /* Then */
    return strcmp(bom_text, expected_bom_text) == 0;
}

/* Vérifie qu'une chaîne sans BOM reste inchangée. */
static int test_strip_utf8_bom_preserves_plain_text(void) {
    /* Given */
    const char *expected_plain_text = "GEDCOM";
    char plain_text[] = "GEDCOM";

    /* When */
    strip_utf8_bom(plain_text);

    /* Then */
    return strcmp(plain_text, expected_plain_text) == 0;
}

/* Exécute les tests unitaires des utilitaires de chaînes. */
static const TestCase test_cases[] = {
    {"test_string_duplication", test_string_duplication},
    {"test_trim_whitespace", test_trim_whitespace},
    {"test_string_assignment", test_string_assignment},
    {"test_string_assignment_clears_null", test_string_assignment_clears_null},
    {"test_strip_utf8_bom", test_strip_utf8_bom},
    {"test_strip_utf8_bom_preserves_plain_text",
     test_strip_utf8_bom_preserves_plain_text},
};

int main(void) {
    return run_test_suite("string_utils", test_cases,
                          sizeof(test_cases) / sizeof(test_cases[0]));
}