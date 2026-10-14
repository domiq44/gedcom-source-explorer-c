#ifndef SOURCE_LIST_H
#define SOURCE_LIST_H

#include <stddef.h>

#include "gedcom_excerpt.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Données d'entrée copiées lors de l'ajout d'une source.
 */
typedef struct {
    const char *description;
    const char *reference;
    const char *kind;
    const char *context;
} SourceInput;

/**
 * @brief Représente une source (SOUR) trouvée dans un fichier GEDCOM.
 */
typedef struct {
    char *description;
    char *reference;
    char *kind;
    char *context;
    char *title;
    GedcomExcerpt gedcom;
} Source;

/**
 * @brief Conteneur pour la collection de sources extraites.
 */
typedef struct {
    Source *sources;
    size_t count;
    size_t capacity;
} SourceList;

void init_source_list(SourceList *list);
int source_list_add(SourceList *list, const SourceInput *input);
int source_list_set_title(SourceList *list, const char *reference,
                          const char *title);
void free_source_list(SourceList *list);

#ifdef __cplusplus
}
#endif

#endif /* SOURCE_LIST_H */