#ifndef GEDCOM_PARSER_H
#define GEDCOM_PARSER_H

#include <stddef.h>

#include "repository_list.h"
#include "source_list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    GEDCOM_PARSE_ERROR_NONE,
    GEDCOM_PARSE_ERROR_INVALID_ARGUMENT,
    GEDCOM_PARSE_ERROR_OPEN,
    GEDCOM_PARSE_ERROR_INVALID_LEVEL,
    GEDCOM_PARSE_ERROR_READ,
    GEDCOM_PARSE_ERROR_MEMORY
} GedcomParseErrorCode;

typedef struct {
    GedcomParseErrorCode code;
    size_t line_number;
} GedcomParseError;

/**
 * @brief Analyse un fichier GEDCOM et extrait les entrées SOUR et REPO.
 *
 * Cette fonction lit le fichier ligne par ligne, repère les lignes de niveau
 * GEDCOM correspondant au tag SOUR et stocke la description associée.
 *
 * @param filename Chemin du fichier GEDCOM à analyser.
 * @param sources Pointeur vers la liste où stocker les résultats.
 * @param repositories Pointeur vers la liste où stocker les dépôts.
 * @return 0 en cas de succès, -1 en cas d'erreur.
 */
int parse_gedcom(const char *filename, SourceList *sources,
                 RepositoryList *repositories);
int parse_gedcom_with_error(const char *filename, SourceList *sources,
                            RepositoryList *repositories,
                            GedcomParseError *error);

#ifdef __cplusplus
}
#endif

#endif /* GEDCOM_PARSER_H */
