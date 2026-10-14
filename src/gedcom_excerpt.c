#include "gedcom_excerpt.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 128U

void gedcom_excerpt_init(GedcomExcerpt *excerpt) {
    if (excerpt == NULL) {
        return;
    }

    excerpt->text = NULL;
    excerpt->length = 0U;
    excerpt->capacity = 0U;
}

int gedcom_excerpt_append_line(GedcomExcerpt *excerpt, const char *line) {
    size_t line_length;
    size_t required_capacity;
    char *new_text;

    if (excerpt == NULL || line == NULL) {
        return 0;
    }

    line_length = strlen(line);
    if (excerpt->length > SIZE_MAX - 2U ||
        line_length > SIZE_MAX - excerpt->length - 2U) {
        return 0;
    }
    required_capacity = excerpt->length + line_length + 2U;

    if (required_capacity > excerpt->capacity) {
        size_t new_capacity;

        new_capacity =
            (excerpt->capacity == 0U) ? INITIAL_CAPACITY : excerpt->capacity;
        while (new_capacity < required_capacity) {
            if (new_capacity > SIZE_MAX / 2U) {
                new_capacity = required_capacity;
                break;
            }
            new_capacity *= 2U;
        }

        new_text = realloc(excerpt->text, new_capacity);
        if (new_text == NULL) {
            return 0;
        }
        excerpt->text = new_text;
        excerpt->capacity = new_capacity;
    }

    memcpy(excerpt->text + excerpt->length, line, line_length);
    excerpt->text[excerpt->length + line_length] = '\n';
    excerpt->length += line_length + 1U;
    excerpt->text[excerpt->length] = '\0';
    return 1;
}

void gedcom_excerpt_free(GedcomExcerpt *excerpt) {
    if (excerpt == NULL) {
        return;
    }

    free(excerpt->text);
    excerpt->text = NULL;
    excerpt->length = 0U;
    excerpt->capacity = 0U;
}