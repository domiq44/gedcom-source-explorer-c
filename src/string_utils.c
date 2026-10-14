#include "string_utils.h"

#include <stdlib.h>
#include <string.h>

#ifdef STRING_UTILS_TESTING
static size_t successful_allocations_before_failure;
static int allocation_failure_enabled;

void string_utils_test_fail_after(size_t successful_allocations) {
    successful_allocations_before_failure = successful_allocations;
    allocation_failure_enabled = 1;
}

void string_utils_test_reset_failure(void) { allocation_failure_enabled = 0; }

static int should_fail_xstrdup(void) {
    if (!allocation_failure_enabled) {
        return 0;
    }
    if (successful_allocations_before_failure > 0U) {
        successful_allocations_before_failure--;
        return 0;
    }
    return 1;
}
#endif

char *xstrdup(const char *text) {
    size_t length;
    char *copy;

    if (text == NULL) {
        return NULL;
    }

    length = strlen(text) + 1U;
#ifdef STRING_UTILS_TESTING
    if (should_fail_xstrdup()) {
        return NULL;
    }
#endif
    copy = malloc(length);
    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, text, length);
    return copy;
}

char *trim_whitespace(char *text) {
    char *end;

    if (text == NULL) {
        return NULL;
    }

    while (*text != '\0' &&
           (*text == ' ' || *text == '\t' || *text == '\r' || *text == '\n')) {
        text++;
    }

    if (*text == '\0') {
        return text;
    }

    end = text + strlen(text) - 1;
    while (end > text &&
           (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')) {
        *end = '\0';
        end--;
    }

    return text;
}

void assign_string(char **field, const char *value) {
    if (field == NULL) {
        return;
    }

    if (*field != NULL) {
        free(*field);
    }

    *field = (value == NULL) ? NULL : xstrdup(value);
}

void strip_utf8_bom(char *text) {
    if (text == NULL) {
        return;
    }

    if ((unsigned char)text[0] == 0xEF && (unsigned char)text[1] == 0xBB &&
        (unsigned char)text[2] == 0xBF) {
        memmove(text, text + 3, strlen(text + 3) + 1U);
    }
}
