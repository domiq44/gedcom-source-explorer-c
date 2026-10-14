#include "gedcom_parser.h"

#include "gedcom_context.h"
#include "string_utils.h"

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 512U
#define MAX_CONTEXT_LENGTH 512
#define MAX_CONTEXT_LEVELS 100U

static int read_full_gedcom_line(FILE *file, char **line_buffer,
                                 size_t *line_capacity,
                                 GedcomParseErrorCode *error_code) {
    char chunk[MAX_LINE_LENGTH];
    size_t length = 0U;

    if (error_code != NULL) {
        *error_code = GEDCOM_PARSE_ERROR_MEMORY;
    }
    if (file == NULL || line_buffer == NULL || line_capacity == NULL) {
        return -1;
    }

    if (*line_buffer == NULL || *line_capacity == 0U) {
        *line_capacity = MAX_LINE_LENGTH;
        *line_buffer = malloc(*line_capacity);
        if (*line_buffer == NULL) {
            return -1;
        }
    }

    (*line_buffer)[0] = '\0';
    while (fgets(chunk, sizeof(chunk), file) != NULL) {
        size_t chunk_length = strlen(chunk);
        size_t required_capacity;
        char *new_buffer;

        required_capacity = length + chunk_length + 1U;
        if (required_capacity > *line_capacity) {
            size_t new_capacity = *line_capacity;
            while (new_capacity < required_capacity) {
                if (new_capacity > SIZE_MAX / 2U) {
                    new_capacity = required_capacity;
                    break;
                }
                new_capacity *= 2U;
            }
            new_buffer = realloc(*line_buffer, new_capacity);
            if (new_buffer == NULL) {
                return -1;
            }
            *line_buffer = new_buffer;
            *line_capacity = new_capacity;
        }

        memcpy(*line_buffer + length, chunk, chunk_length + 1U);
        length += chunk_length;

        if (chunk_length > 0U && chunk[chunk_length - 1U] == '\n') {
            return 1;
        }
    }

    if (ferror(file)) {
        if (error_code != NULL) {
            *error_code = GEDCOM_PARSE_ERROR_READ;
        }
        return -1;
    }

    return (length > 0U) ? 1 : 0;
}

static int is_header_record(int level, const char *tag) {
    return level == 0 && tag != NULL && strcmp(tag, "HEAD") == 0;
}

static int is_entity_source_record(int level, const char *tag,
                                   const char *record_id_start,
                                   const char *record_id_end) {
    return level == 0 && tag != NULL && strcmp(tag, "SOUR") == 0 &&
           record_id_start != NULL && record_id_end != NULL;
}

static int is_citation_source_record(int level, const char *tag) {
    return level > 0 && tag != NULL && strcmp(tag, "SOUR") == 0;
}

static int is_repository_record(int level, const char *tag,
                                const char *record_id_start,
                                const char *record_id_end) {
    return level == 0 && tag != NULL && strcmp(tag, "REPO") == 0 &&
           record_id_start != NULL && record_id_end != NULL;
}

static int add_entity_source(SourceList *sources, SourceInput source) {
    char description[MAX_CONTEXT_LENGTH];

    if (source.description == NULL || *source.description == '\0') {
        snprintf(description, sizeof(description), "%s",
                 (source.reference != NULL) ? source.reference : "");
    } else {
        snprintf(description, sizeof(description), "%s", source.description);
    }

    source.description = description;
    source.kind = "entity";
    source.context = "entity";
    return source_list_add(sources, &source);
}

static int add_citation_source(SourceList *sources, SourceInput citation) {
    citation.description = citation.reference;
    citation.kind = "citation";
    return source_list_add(sources, &citation);
}

static void set_parse_error(GedcomParseError *error, GedcomParseError value) {
    if (error != NULL) {
        *error = value;
    }
}

/**
 * @brief Analyse un fichier GEDCOM pour extraire les entrées SOUR.
 *
 * @param filename Le chemin vers le fichier GEDCOM à analyser.
 * @param sources Pointeur vers la SourceList où les résultats seront stockés.
 * @return int 0 si l'analyse est réussie, -1 en cas d'erreur.
 */
int parse_gedcom_with_error(const char *filename, SourceList *sources,
                            RepositoryList *repositories,
                            GedcomParseError *error) {
    FILE *file;
    char *line = NULL;
    size_t line_capacity = 0U;
    size_t line_number = 0U;
    char current_record_id[MAX_CONTEXT_LENGTH] = "";
    char current_record_tag[MAX_CONTEXT_LENGTH] = "";
    char context_tags[MAX_CONTEXT_LEVELS][32] = {{0}};
    size_t active_source_index = 0U;
    size_t active_repository_index = 0U;
    size_t active_citation_index = 0U;
    int has_active_source = 0;
    int has_active_repository = 0;
    int has_active_citation = 0;
    int active_citation_level = -1;
    int in_header = 0;
    int read_status;
    int result = -1;
    GedcomParseErrorCode error_code = GEDCOM_PARSE_ERROR_MEMORY;
    GedcomParseErrorCode read_error_code = GEDCOM_PARSE_ERROR_MEMORY;

    set_parse_error(error, (GedcomParseError){
                               .code = GEDCOM_PARSE_ERROR_NONE,
                               .line_number = 0U,
                           });
    if (filename == NULL || *filename == '\0' || sources == NULL ||
        repositories == NULL) {
        set_parse_error(error, (GedcomParseError){
                                   .code = GEDCOM_PARSE_ERROR_INVALID_ARGUMENT,
                                   .line_number = 0U,
                               });
        return -1;
    }

    init_source_list(sources);
    init_repository_list(repositories);

    file = fopen(filename, "r");
    if (file == NULL) {
        set_parse_error(error, (GedcomParseError){
                                   .code = GEDCOM_PARSE_ERROR_OPEN,
                                   .line_number = 0U,
                               });
        return -1;
    }

    while ((read_status = read_full_gedcom_line(file, &line, &line_capacity,
                                                &read_error_code)) > 0) {
        line_number++;
        char *cursor;
        const char *line_content;
        const char *tag_start;
        char *level_end;
        const char *record_id_start = NULL;
        const char *record_id_end = NULL;
        const char *value_start;
        size_t tag_length;
        char tag[32];
        int level = 0;
        long parsed_level;
        const char *value;
        char reference[MAX_CONTEXT_LENGTH];

        size_t len = strlen(line);
        if (len > 0U && line[len - 1U] == '\n') {
            line[len - 1U] = '\0';
        }

        cursor = trim_whitespace(line);
        if (*cursor == '\0') {
            continue;
        }

        strip_utf8_bom(cursor);
        if (*cursor == '\0') {
            continue;
        }
        line_content = cursor;

        if (!isdigit((unsigned char)*cursor)) {
            continue;
        }

        errno = 0;
        parsed_level = strtol(cursor, &level_end, 10);
        if (errno == ERANGE || level_end == cursor || parsed_level < 0L ||
            parsed_level >= (long)MAX_CONTEXT_LEVELS ||
            (*level_end != '\0' && !isspace((unsigned char)*level_end))) {
            error_code = GEDCOM_PARSE_ERROR_INVALID_LEVEL;
            goto cleanup;
        }
        level = (int)parsed_level;

        cursor = trim_whitespace(level_end);
        if (*cursor == '\0') {
            continue;
        }

        if (level == 0) {
            has_active_source = 0;
            has_active_repository = 0;
        }
        if (has_active_citation && level <= active_citation_level) {
            has_active_citation = 0;
        }

        if (has_active_source &&
            !gedcom_excerpt_append_line(
                &sources->sources[active_source_index].gedcom, line_content)) {
            goto cleanup;
        }
        if (has_active_repository &&
            !gedcom_excerpt_append_line(
                &repositories->repositories[active_repository_index].gedcom,
                line_content)) {
            goto cleanup;
        }
        if (has_active_citation &&
            !gedcom_excerpt_append_line(
                &sources->sources[active_citation_index].gedcom,
                line_content)) {
            goto cleanup;
        }

        if (*cursor == '@') {
            record_id_start = cursor;
            while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t') {
                cursor++;
            }
            record_id_end = cursor;
            cursor = trim_whitespace(cursor);
        }

        if (*cursor == '\0') {
            continue;
        }

        tag_start = cursor;
        while (*cursor != '\0' && !isspace((unsigned char)*cursor)) {
            cursor++;
        }

        if (cursor == tag_start) {
            continue;
        }

        tag_length = (size_t)(cursor - tag_start);
        if (tag_length >= sizeof(tag)) {
            continue;
        }

        memcpy(tag, tag_start, tag_length);
        tag[tag_length] = '\0';

        if (is_header_record(level, tag)) {
            in_header = 1;
            continue;
        }

        if (in_header) {
            if (level == 0) {
                in_header = 0;
            } else {
                continue;
            }
        }

        value_start = trim_whitespace(cursor);
        value = value_start;

        if (strcmp(tag, "SOUR") == 0) {
            if (is_entity_source_record(level, tag, record_id_start,
                                        record_id_end)) {
                char id_buffer[MAX_CONTEXT_LENGTH];
                size_t id_length;

                id_length = (size_t)(record_id_end - record_id_start);
                if (id_length >= sizeof(id_buffer)) {
                    id_length = sizeof(id_buffer) - 1U;
                }
                memcpy(id_buffer, record_id_start, id_length);
                id_buffer[id_length] = '\0';

                snprintf(reference, sizeof(reference), "%s", id_buffer);

                SourceInput source = {
                    .description = value,
                    .reference = reference,
                };

                if (!add_entity_source(sources, source)) {
                    goto cleanup;
                }
                active_source_index = sources->count - 1U;
                if (!gedcom_excerpt_append_line(
                        &sources->sources[active_source_index].gedcom,
                        line_content)) {
                    goto cleanup;
                }
                has_active_source = 1;
            } else if (is_citation_source_record(level, tag)) {
                const char *citation_ref = value;
                char context_ref[MAX_CONTEXT_LENGTH] = "";
                char parent_tag[MAX_CONTEXT_LENGTH] = "";

                if (*citation_ref != '\0') {
                    snprintf(reference, sizeof(reference), "%s", citation_ref);
                } else {
                    snprintf(reference, sizeof(reference), "%s", "unknown");
                }

                if (level == 1) {
                    snprintf(parent_tag, sizeof(parent_tag), "%s",
                             current_record_tag);
                } else if (level > 1 && level < (int)MAX_CONTEXT_LEVELS) {
                    snprintf(parent_tag, sizeof(parent_tag), "%s",
                             context_tags[level - 1]);
                }
                build_gedcom_context(current_record_id, current_record_tag,
                                     parent_tag, context_ref,
                                     sizeof(context_ref));

                SourceInput citation = {
                    .reference = reference,
                    .context = context_ref,
                };

                if (!add_citation_source(sources, citation)) {
                    goto cleanup;
                }
                active_citation_index = sources->count - 1U;
                active_citation_level = level;
                if (!gedcom_excerpt_append_line(
                        &sources->sources[active_citation_index].gedcom,
                        line_content)) {
                    goto cleanup;
                }
                has_active_citation = 1;
            }
        } else if (is_repository_record(level, tag, record_id_start,
                                        record_id_end)) {
            char id_buffer[MAX_CONTEXT_LENGTH];
            size_t id_length = (size_t)(record_id_end - record_id_start);

            if (id_length >= sizeof(id_buffer)) {
                id_length = sizeof(id_buffer) - 1U;
            }
            memcpy(id_buffer, record_id_start, id_length);
            id_buffer[id_length] = '\0';

            if (!repository_list_add(repositories, id_buffer,
                                     (*value != '\0') ? value : id_buffer)) {
                goto cleanup;
            }
            active_repository_index = repositories->count - 1U;
            if (!gedcom_excerpt_append_line(
                    &repositories->repositories[active_repository_index].gedcom,
                    line_content)) {
                goto cleanup;
            }
            has_active_repository = 1;
        } else if (level == 1 && strcmp(tag, "NAME") == 0 &&
                   strcmp(current_record_tag, "REPO") == 0 &&
                   current_record_id[0] != '\0' && *value != '\0') {
            if (!repository_list_set_name(repositories, current_record_id,
                                          value)) {
                goto cleanup;
            }
        } else if (level == 1 && strcmp(tag, "TITL") == 0 &&
                   strcmp(current_record_tag, "SOUR") == 0 &&
                   current_record_id[0] != '\0' && *value != '\0') {
            if (!source_list_set_title(sources, current_record_id, value)) {
                goto cleanup;
            }
        } else if (level == 1 && strcmp(tag, "REPO") == 0 &&
                   strcmp(current_record_tag, "SOUR") == 0 &&
                   current_record_id[0] != '\0' && *value == '@') {
            char context[MAX_CONTEXT_LENGTH];

            build_gedcom_context(current_record_id, current_record_tag, tag,
                                 context, sizeof(context));
            if (!repository_list_add_link(repositories, current_record_id,
                                          value, context)) {
                goto cleanup;
            }
        }

        if (level == 0) {
            memset(context_tags, 0, sizeof(context_tags));
            if (record_id_start != NULL && record_id_end != NULL) {
                size_t id_length = (size_t)(record_id_end - record_id_start);
                if (id_length >= sizeof(current_record_id)) {
                    id_length = sizeof(current_record_id) - 1U;
                }
                memcpy(current_record_id, record_id_start, id_length);
                current_record_id[id_length] = '\0';
            } else {
                current_record_id[0] = '\0';
            }
            snprintf(current_record_tag, sizeof(current_record_tag), "%s", tag);
        } else if (level > 0) {
            if (level < (int)MAX_CONTEXT_LEVELS) {
                size_t context_level;

                for (context_level = (size_t)level + 1U;
                     context_level < MAX_CONTEXT_LEVELS; context_level++) {
                    context_tags[context_level][0] = '\0';
                }
                snprintf(context_tags[level], sizeof(context_tags[level]), "%s",
                         tag);
            } else {
                memset(context_tags, 0, sizeof(context_tags));
            }
        }
    }

    if (read_status < 0) {
        error_code = read_error_code;
        line_number++;
        goto cleanup;
    }
    result = 0;

cleanup:
    free(line);
    if (fclose(file) != 0) {
        result = -1;
        error_code = GEDCOM_PARSE_ERROR_READ;
        line_number = 0U;
    }
    if (result != 0) {
        set_parse_error(error, (GedcomParseError){
                                   .code = error_code,
                                   .line_number = line_number,
                               });
    }
    return result;
}

int parse_gedcom(const char *filename, SourceList *sources,
                 RepositoryList *repositories) {
    return parse_gedcom_with_error(filename, sources, repositories, NULL);
}
