#ifndef HTML_REPORT_H
#define HTML_REPORT_H

#include "gedcom_parser.h"
#include "report_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Remplace le marqueur JSON du template HTML et écrit le rapport final.
 * @return 0 en cas de succès, -1 si le template, le marqueur ou la sortie est
 * invalide.
 */
int write_html_report(const char *template_filename,
                      const char *output_filename, const SourceList *sources,
                      const RepositoryList *repositories);

/**
 * @brief Écrit un rapport HTML à partir d'un modèle de données déjà construit.
 */
int write_html_report_data(const char *template_filename,
                           const char *output_filename,
                           const ReportData *report_data);

#ifdef __cplusplus
}
#endif

#endif /* HTML_REPORT_H */