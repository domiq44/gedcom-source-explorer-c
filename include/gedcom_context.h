#ifndef GEDCOM_CONTEXT_H
#define GEDCOM_CONTEXT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void build_gedcom_context(const char *record_id, const char *record_tag,
                          const char *parent_tag, char *context,
                          size_t context_size);

#ifdef __cplusplus
}
#endif

#endif /* GEDCOM_CONTEXT_H */