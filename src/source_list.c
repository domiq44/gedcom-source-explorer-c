#include "source_list.h"

#include "string_utils.h"

#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 10

void init_source_list(SourceList *list) {
    if (list == NULL) {
        return;
    }

    list->sources = NULL;
    list->count = 0U;
    list->capacity = 0U;
}

int source_list_add(SourceList *list, const SourceInput *input) {
    Source *new_sources;
    Source *new_source;

    if (list == NULL || input == NULL || input->description == NULL) {
        return 0;
    }

    if (list->count >= list->capacity) {
        size_t new_capacity;

        new_capacity =
            (list->capacity == 0U) ? INITIAL_CAPACITY : list->capacity * 2U;
        new_sources =
            realloc(list->sources, new_capacity * sizeof(*new_sources));
        if (new_sources == NULL) {
            return 0;
        }
        memset(&new_sources[list->count], 0,
               (new_capacity - list->count) * sizeof(*new_sources));
        list->sources = new_sources;
        list->capacity = new_capacity;
    }

    new_source = &list->sources[list->count];
    new_source->description = xstrdup(input->description);
    if (new_source->description == NULL) {
        return 0;
    }

    new_source->reference =
        (input->reference != NULL) ? xstrdup(input->reference) : NULL;
    new_source->kind = (input->kind != NULL) ? xstrdup(input->kind) : NULL;
    new_source->context =
        (input->context != NULL) ? xstrdup(input->context) : NULL;
    if ((input->reference != NULL && new_source->reference == NULL) ||
        (input->kind != NULL && new_source->kind == NULL) ||
        (input->context != NULL && new_source->context == NULL)) {
        free(new_source->description);
        free(new_source->reference);
        free(new_source->kind);
        free(new_source->context);
        memset(new_source, 0, sizeof(*new_source));
        return 0;
    }
    gedcom_excerpt_init(&new_source->gedcom);

    list->count++;
    return 1;
}

int source_list_set_title(SourceList *list, const char *reference,
                          const char *title) {
    size_t index;
    char *title_copy;

    if (list == NULL || reference == NULL || title == NULL) {
        return 0;
    }

    for (index = 0U; index < list->count; index++) {
        Source *source = &list->sources[index];
        if (source->kind != NULL && strcmp(source->kind, "entity") == 0 &&
            source->reference != NULL &&
            strcmp(source->reference, reference) == 0) {
            title_copy = xstrdup(title);
            if (title_copy == NULL) {
                return 0;
            }
            free(source->title);
            source->title = title_copy;
            return 1;
        }
    }

    return 0;
}

void free_source_list(SourceList *list) {
    size_t i;

    if (list == NULL) {
        return;
    }

    for (i = 0U; i < list->count; i++) {
        free(list->sources[i].description);
        free(list->sources[i].reference);
        free(list->sources[i].kind);
        free(list->sources[i].context);
        free(list->sources[i].title);
        gedcom_excerpt_free(&list->sources[i].gedcom);
    }

    free(list->sources);
    list->sources = NULL;
    list->count = 0U;
    list->capacity = 0U;
}