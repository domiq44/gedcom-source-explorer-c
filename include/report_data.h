#ifndef REPORT_DATA_H
#define REPORT_DATA_H

#include <stdio.h>

#include "repository_list.h"
#include "source_list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const Source *sources;
    size_t source_count;
    const Repository *repositories;
    size_t repository_count;
    const RepositoryLink *repository_links;
    size_t repository_link_count;
} ReportData;

int build_report_data(const SourceList *sources,
                      const RepositoryList *repositories,
                      ReportData *report_data);
int write_report_data_json(FILE *output, const ReportData *report_data);

#ifdef __cplusplus
}
#endif

#endif /* REPORT_DATA_H */
