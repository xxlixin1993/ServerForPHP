#include "../include/tool.h"

// Convert str characters before n to lowercase
void str_to_low(char *str, int n) {
    char *cur = str;
    while (n > 0) {
        *cur = tolower(*cur);
        cur++;
        n--;
    }
}