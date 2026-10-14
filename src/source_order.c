#include "source_order.h"

#include <ctype.h>
#include <string.h>

int compare_gedcom_references(const char *left_reference,
                              const char *right_reference) {
    const char *left_id = (left_reference != NULL) ? left_reference : "";
    const char *right_id = (right_reference != NULL) ? right_reference : "";
    const char *left_digits;
    const char *right_digits;
    const char *left_end;
    const char *right_end;
    size_t left_length;
    size_t right_length;
    int order;

    if (left_id[0] == '@')
        left_id++;
    if (right_id[0] == '@')
        right_id++;
    if (left_id[0] == '\0' || right_id[0] == '\0') {
        return strcmp(left_reference != NULL ? left_reference : "",
                      right_reference != NULL ? right_reference : "");
    }

    order = tolower((unsigned char)left_id[0]) -
            tolower((unsigned char)right_id[0]);
    if (order != 0) {
        return order;
    }

    left_digits = left_id + 1;
    right_digits = right_id + 1;
    if (*left_digits == '\0' || *right_digits == '\0') {
        return strcmp(left_reference, right_reference);
    }
    for (left_end = left_digits; isdigit((unsigned char)*left_end);
         left_end++) {
    }
    for (right_end = right_digits; isdigit((unsigned char)*right_end);
         right_end++) {
    }

    if ((*left_end != '@' && *left_end != '\0') ||
        (*right_end != '@' && *right_end != '\0')) {
        return strcmp(left_reference, right_reference);
    }

    while (*left_digits == '0' && left_digits + 1 < left_end)
        left_digits++;
    while (*right_digits == '0' && right_digits + 1 < right_end)
        right_digits++;
    left_length = (size_t)(left_end - left_digits);
    right_length = (size_t)(right_end - right_digits);
    if (left_length != right_length) {
        return left_length < right_length ? -1 : 1;
    }

    order = strncmp(left_digits, right_digits, left_length);
    return (order != 0) ? order : strcmp(left_reference, right_reference);
}