#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

char *xstrdup(const char *text);
char *trim_whitespace(char *text);
void assign_string(char **field, const char *value);
void strip_utf8_bom(char *text);

#ifdef STRING_UTILS_TESTING
void string_utils_test_fail_after(size_t successful_allocations);
void string_utils_test_reset_failure(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* STRING_UTILS_H */
