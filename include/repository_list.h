#ifndef REPOSITORY_LIST_H
#define REPOSITORY_LIST_H

#include <stddef.h>

#include "gedcom_excerpt.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char *reference;
    char *name;
    GedcomExcerpt gedcom;
} Repository;

typedef struct {
    char *source_reference;
    char *repository_reference;
    char *context;
} RepositoryLink;

typedef struct {
    Repository *repositories;
    size_t count;
    size_t capacity;
    RepositoryLink *links;
    size_t link_count;
    size_t link_capacity;
} RepositoryList;

void init_repository_list(RepositoryList *list);
int repository_list_add(RepositoryList *list, const char *reference,
                        const char *name);
int repository_list_set_name(RepositoryList *list, const char *reference,
                             const char *name);
int repository_list_add_link(RepositoryList *list, const char *source_reference,
                             const char *repository_reference,
                             const char *context);
void free_repository_list(RepositoryList *list);

#ifdef __cplusplus
}
#endif

#endif /* REPOSITORY_LIST_H */