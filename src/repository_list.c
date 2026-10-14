#include "repository_list.h"

#include "string_utils.h"

#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 8

void init_repository_list(RepositoryList *list) {
    if (list == NULL) {
        return;
    }

    list->repositories = NULL;
    list->count = 0U;
    list->capacity = 0U;
    list->links = NULL;
    list->link_count = 0U;
    list->link_capacity = 0U;
}

int repository_list_add(RepositoryList *list, const char *reference,
                        const char *name) {
    Repository *new_repositories;
    Repository *new_repository;

    if (list == NULL || reference == NULL || name == NULL) {
        return 0;
    }

    if (list->count >= list->capacity) {
        size_t new_capacity;

        new_capacity =
            (list->capacity == 0U) ? INITIAL_CAPACITY : list->capacity * 2U;
        new_repositories = realloc(list->repositories,
                                   new_capacity * sizeof(*new_repositories));
        if (new_repositories == NULL) {
            return 0;
        }
        memset(&new_repositories[list->count], 0,
               (new_capacity - list->count) * sizeof(*new_repositories));
        list->repositories = new_repositories;
        list->capacity = new_capacity;
    }

    new_repository = &list->repositories[list->count];
    new_repository->reference = xstrdup(reference);
    new_repository->name = xstrdup(name);
    if (new_repository->reference == NULL || new_repository->name == NULL) {
        free(new_repository->reference);
        free(new_repository->name);
        new_repository->reference = NULL;
        new_repository->name = NULL;
        return 0;
    }

    gedcom_excerpt_init(&new_repository->gedcom);
    list->count++;
    return 1;
}

int repository_list_set_name(RepositoryList *list, const char *reference,
                             const char *name) {
    size_t index;
    char *name_copy;

    if (list == NULL || reference == NULL || name == NULL) {
        return 0;
    }

    for (index = 0U; index < list->count; index++) {
        if (strcmp(list->repositories[index].reference, reference) == 0) {
            name_copy = xstrdup(name);
            if (name_copy == NULL) {
                return 0;
            }
            free(list->repositories[index].name);
            list->repositories[index].name = name_copy;
            return 1;
        }
    }

    return 0;
}

int repository_list_add_link(RepositoryList *list, const char *source_reference,
                             const char *repository_reference,
                             const char *context) {
    RepositoryLink *new_links;
    RepositoryLink *new_link;

    if (list == NULL || source_reference == NULL ||
        repository_reference == NULL || context == NULL) {
        return 0;
    }

    if (list->link_count >= list->link_capacity) {
        size_t new_capacity;

        new_capacity = (list->link_capacity == 0U) ? INITIAL_CAPACITY
                                                   : list->link_capacity * 2U;
        new_links = realloc(list->links, new_capacity * sizeof(*new_links));
        if (new_links == NULL) {
            return 0;
        }
        memset(&new_links[list->link_count], 0,
               (new_capacity - list->link_count) * sizeof(*new_links));
        list->links = new_links;
        list->link_capacity = new_capacity;
    }

    new_link = &list->links[list->link_count];
    new_link->source_reference = xstrdup(source_reference);
    new_link->repository_reference = xstrdup(repository_reference);
    new_link->context = xstrdup(context);
    if (new_link->source_reference == NULL ||
        new_link->repository_reference == NULL || new_link->context == NULL) {
        free(new_link->source_reference);
        free(new_link->repository_reference);
        free(new_link->context);
        new_link->source_reference = NULL;
        new_link->repository_reference = NULL;
        new_link->context = NULL;
        return 0;
    }

    list->link_count++;
    return 1;
}

void free_repository_list(RepositoryList *list) {
    size_t index;

    if (list == NULL) {
        return;
    }

    for (index = 0U; index < list->count; index++) {
        free(list->repositories[index].reference);
        free(list->repositories[index].name);
        gedcom_excerpt_free(&list->repositories[index].gedcom);
    }

    for (index = 0U; index < list->link_count; index++) {
        free(list->links[index].source_reference);
        free(list->links[index].repository_reference);
        free(list->links[index].context);
    }

    free(list->repositories);
    free(list->links);
    list->repositories = NULL;
    list->count = 0U;
    list->capacity = 0U;
    list->links = NULL;
    list->link_count = 0U;
    list->link_capacity = 0U;
}