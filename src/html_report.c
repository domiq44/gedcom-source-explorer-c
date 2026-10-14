#include "html_report.h"

#include <stdio.h>
#include <string.h>

static int copy_template(FILE *template_file, FILE *output,
                         const ReportData *report_data) {
    static const char marker[] = "{{GEDCOM_REPORT_DATA}}";
    char line[4096];
    size_t replacements = 0U;

    while (fgets(line, sizeof(line), template_file) != NULL) {
        char *marker_position = strstr(line, marker);

        if (marker_position == NULL) {
            if (fputs(line, output) == EOF)
                return 0;
            continue;
        }

        if (replacements > 0U ||
            strstr(marker_position + sizeof(marker) - 1U, marker) != NULL ||
            fwrite(line, 1U, (size_t)(marker_position - line), output) !=
                (size_t)(marker_position - line) ||
            !write_report_data_json(output, report_data) ||
            fputs(marker_position + sizeof(marker) - 1U, output) == EOF) {
            return 0;
        }
        replacements++;
    }

    return !ferror(template_file) && !ferror(output) && replacements == 1U;
}

int write_html_report_data(const char *template_filename,
                           const char *output_filename,
                           const ReportData *report_data) {
    FILE *template_file;
    FILE *output;
    int success;

    if (template_filename == NULL || *template_filename == '\0' ||
        output_filename == NULL || *output_filename == '\0' ||
        report_data == NULL) {
        return -1;
    }

    template_file = fopen(template_filename, "r");
    if (template_file == NULL) {
        return -1;
    }

    output = fopen(output_filename, "w");
    if (output == NULL) {
        fclose(template_file);
        return -1;
    }

    success = copy_template(template_file, output, report_data);
    if (fclose(template_file) != 0) {
        success = 0;
    }
    if (fclose(output) != 0) {
        success = 0;
    }
    return success ? 0 : -1;
}

int write_html_report(const char *template_filename,
                      const char *output_filename, const SourceList *sources,
                      const RepositoryList *repositories) {
    ReportData report_data;

    if (template_filename == NULL || *template_filename == '\0' ||
        output_filename == NULL || *output_filename == '\0' ||
        sources == NULL || repositories == NULL) {
        return -1;
    }

    if (build_report_data(sources, repositories, &report_data) != 0) {
        return -1;
    }

    return write_html_report_data(template_filename, output_filename,
                                  &report_data);
}
