#ifndef GEDCOM_EXCERPT_H
#define GEDCOM_EXCERPT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char *text;
    size_t length;
    size_t capacity;
} GedcomExcerpt;

void gedcom_excerpt_init(GedcomExcerpt *excerpt);
int gedcom_excerpt_append_line(GedcomExcerpt *excerpt, const char *line);
void gedcom_excerpt_free(GedcomExcerpt *excerpt);

#ifdef __cplusplus
}
#endif

#endif /* GEDCOM_EXCERPT_H */