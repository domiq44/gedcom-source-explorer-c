#include "gedcom_context.h"

#include <stdio.h>
#include <string.h>

void build_gedcom_context(const char *record_id, const char *record_tag,
                          const char *parent_tag, char *context,
                          size_t context_size) {
    if (context == NULL || context_size == 0U) {
        return;
    }

    context[0] = '\0';

    if (record_id != NULL && record_id[0] != '\0') {
        size_t remaining;

        record_tag = (record_tag != NULL) ? record_tag : "";
        parent_tag = (parent_tag != NULL) ? parent_tag : "";

        remaining = context_size;
        strncat(context, record_id, remaining - 1U);
        remaining = context_size - strlen(context);

        if (record_tag[0] != '\0') {
            strncat(context, " ", remaining - 1U);
            remaining = context_size - strlen(context);
            strncat(context, record_tag, remaining - 1U);
            remaining = context_size - strlen(context);
        }

        if (parent_tag[0] != '\0' && strcmp(parent_tag, record_tag) != 0) {
            strncat(context, " / ", remaining - 1U);
            remaining = context_size - strlen(context);
            strncat(context, parent_tag, remaining - 1U);
        }
    } else {
        snprintf(context, context_size, "%s", "unknown");
    }
}