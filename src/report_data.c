#include "report_data.h"

#include <stdio.h>
#include <string.h>

static int write_json_string(FILE *output, const char *value) {
    const unsigned char *cursor =
        (const unsigned char *)((value != NULL) ? value : "");

    if (fputc('"', output) == EOF) {
        return 0;
    }

    while (*cursor != '\0') {
        switch (*cursor) {
        case '"':
            if (fputs("\\\"", output) == EOF)
                return 0;
            break;
        case '\\':
            if (fputs("\\\\", output) == EOF)
                return 0;
            break;
        case '\b':
            if (fputs("\\b", output) == EOF)
                return 0;
            break;
        case '\f':
            if (fputs("\\f", output) == EOF)
                return 0;
            break;
        case '\n':
            if (fputs("\\n", output) == EOF)
                return 0;
            break;
        case '\r':
            if (fputs("\\r", output) == EOF)
                return 0;
            break;
        case '\t':
            if (fputs("\\t", output) == EOF)
                return 0;
            break;
        case '<':
            if (fputs("\\u003c", output) == EOF)
                return 0;
            break;
        case '>':
            if (fputs("\\u003e", output) == EOF)
                return 0;
            break;
        case '&':
            if (fputs("\\u0026", output) == EOF)
                return 0;
            break;
        default:
            if (*cursor < 0x20U) {
                if (fprintf(output, "\\u%04x", (unsigned int)*cursor) < 0)
                    return 0;
            } else if (fputc(*cursor, output) == EOF) {
                return 0;
            }
            break;
        }
        cursor++;
    }

    return fputc('"', output) != EOF;
}

static int is_source_entity(const Source *source) {
    return source->kind != NULL && strcmp(source->kind, "entity") == 0;
}

static int is_citation(const Source *source) {
    return source->kind == NULL || strcmp(source->kind, "citation") == 0;
}

typedef enum { SOURCE_ARRAY_ENTITIES, SOURCE_ARRAY_CITATIONS } SourceArrayKind;

static int write_source_array(FILE *output, const ReportData *report_data,
                              SourceArrayKind array_kind) {
    const Source *sources = report_data->sources;
    size_t source_count = report_data->source_count;
    size_t index;
    int first = 1;

    if (fputs("[", output) == EOF)
        return 0;
    for (index = 0U; index < source_count; index++) {
        const Source *source = &sources[index];
        int include_source = (array_kind == SOURCE_ARRAY_ENTITIES)
                                 ? is_source_entity(source)
                                 : is_citation(source);

        if (!include_source)
            continue;
        if (!first && fputc(',', output) == EOF)
            return 0;
        if (fputs("{\"reference\":", output) == EOF ||
            !write_json_string(output, source->reference) ||
            fputs(",\"description\":", output) == EOF ||
            !write_json_string(output, source->description) ||
            fputs(",\"title\":", output) == EOF ||
            !write_json_string(output, source->title) ||
            fputs(",\"context\":", output) == EOF ||
            !write_json_string(output, source->context) ||
            fputs(",\"gedcom\":", output) == EOF ||
            !write_json_string(output, source->gedcom.text) ||
            fputc('}', output) == EOF) {
            return 0;
        }
        first = 0;
    }
    return fputc(']', output) != EOF;
}

static int write_repository_array(FILE *output, const Repository *repositories,
                                  size_t repository_count) {
    size_t index;

    if (fputc('[', output) == EOF)
        return 0;
    for (index = 0U; index < repository_count; index++) {
        const Repository *repository = &repositories[index];

        if (index > 0U && fputc(',', output) == EOF)
            return 0;
        if (fputs("{\"reference\":", output) == EOF ||
            !write_json_string(output, repository->reference) ||
            fputs(",\"name\":", output) == EOF ||
            !write_json_string(output, repository->name) ||
            fputs(",\"gedcom\":", output) == EOF ||
            !write_json_string(output, repository->gedcom.text) ||
            fputc('}', output) == EOF) {
            return 0;
        }
    }
    return fputc(']', output) != EOF;
}

static int write_repository_link_array(FILE *output,
                                       const RepositoryLink *links,
                                       size_t link_count) {
    size_t index;

    if (fputc('[', output) == EOF)
        return 0;
    for (index = 0U; index < link_count; index++) {
        const RepositoryLink *link = &links[index];

        if (index > 0U && fputc(',', output) == EOF)
            return 0;
        if (fputs("{\"sourceReference\":", output) == EOF ||
            !write_json_string(output, link->source_reference) ||
            fputs(",\"repositoryReference\":", output) == EOF ||
            !write_json_string(output, link->repository_reference) ||
            fputs(",\"context\":", output) == EOF ||
            !write_json_string(output, link->context) ||
            fputc('}', output) == EOF) {
            return 0;
        }
    }
    return fputc(']', output) != EOF;
}

int build_report_data(const SourceList *sources,
                      const RepositoryList *repositories,
                      ReportData *report_data) {
    if (sources == NULL || repositories == NULL || report_data == NULL) {
        return -1;
    }

    report_data->sources = sources->sources;
    report_data->source_count = sources->count;
    report_data->repositories = repositories->repositories;
    report_data->repository_count = repositories->count;
    report_data->repository_links = repositories->links;
    report_data->repository_link_count = repositories->link_count;
    return 0;
}

int write_report_data_json(FILE *output, const ReportData *report_data) {
    if (output == NULL || report_data == NULL) {
        return 0;
    }

    return fputs("{\"sources\":", output) != EOF &&
           write_source_array(output, report_data, SOURCE_ARRAY_ENTITIES) &&
           fputs(",\"repositories\":", output) != EOF &&
           write_repository_array(output, report_data->repositories,
                                  report_data->repository_count) &&
           fputs(",\"repositoryLinks\":", output) != EOF &&
           write_repository_link_array(output, report_data->repository_links,
                                       report_data->repository_link_count) &&
           fputs(",\"citations\":", output) != EOF &&
           write_source_array(output, report_data, SOURCE_ARRAY_CITATIONS) &&
           fputc('}', output) != EOF;
}
