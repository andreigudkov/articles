#include <stdio.h>
#include <stdbool.h>
#include <string.h>

inline int max(int a, int b) {
  return a > b ? a : b;
}

int find_shift(const char* pattern, const char* needle) {
  if (strlen(needle) == 0) {
    return 1;
  }
  for (int i = strlen(pattern)-strlen(needle)-1; i > -(int)strlen(needle); i--) {
    bool ok = true;
    for (int j = 0; j < strlen(needle); j++) {
      if (i+j >= 0 && pattern[i+j] != needle[j]) {
        ok = false;
        break;
      }
    }
    if (ok) {
      if (i >= 0) {
        return strlen(pattern) - strlen(needle) - i;
      } else {
        int common_length = strlen(needle) + i;
        return strlen(pattern) - common_length;
      }
    }
  }
  return strlen(pattern);
}

bool is_character_interesting(const char* pattern, char c) {
  if (c == 'u' || c == 'e' || c == 0) {
    return true;
  }
  for (size_t i = 0; i < strlen(pattern); i++) {
    if (pattern[i] == c) {
      return true;
    }
  }
  return false;
}

void build_large_lut(const char* pattern) {
  printf("|");
  for (size_t i = 0; i < strlen(pattern); i++) {
    printf("|%d", i);
  }
  printf("\n");

  printf("|*...*");
  for (size_t i = 0; i < strlen(pattern); i++) {
    printf("|");
  }
  printf("\n");

  for (char s = 'a' ; s <= 'z'; s++) {
    if (!is_character_interesting(pattern, s)) {
      continue;
    }

    printf("|*%c*", s);
    for (size_t p = 0; p < strlen(pattern); p++) {
      if (pattern[strlen(pattern)-1-p] == s) {
        printf("|&mdash;");
      } else {
        char needle[128];
        needle[0] = (s == 0) ? 1 : s;
        strncpy(needle+1, pattern+strlen(pattern)-p, p);
        needle[p+1] = '\0';

        printf("|%d", find_shift(pattern, needle));
      }
    }
    printf("\n");
  }

  printf("|*...*");
  for (size_t i = 0; i < strlen(pattern); i++) {
    printf("|");
  }
  printf("\n");
}

void build_good_suffix_lut(const char* pattern) {
  printf("|good suffix length");
  for (size_t i = 0; i < strlen(pattern); i++) {
    if (i < 10) {
      printf("|&nbsp;%d", i);
    } else {
      printf("|%d", i);
    }
  }
  printf("\n");

  printf("|shift");
  for (size_t p = 0; p < strlen(pattern); p++) {
    char needle[128];
    strncpy(needle, pattern+strlen(pattern)-p, p);
    needle[p] = '\0';

    int shift = find_shift(pattern, needle);
    if (shift < 10) {
      printf("|&nbsp;%d", shift);
    } else {
      printf("|%d", shift);
    }
  }
  printf("\n");
}

void build_bad_character_lut(const char* pattern) {
  printf("|bad character");
  printf("|*...*");
  for (char s = 'a'; s <= 'z'; s++) {
    if (is_character_interesting(pattern, s)) {
      printf("|&nbsp;%c", s);
    }
  }
  printf("|*...*");
  printf("\n");

  printf("|right-most position");
  printf("|...");
  for (char s = 'a'; s <= 'z'; s++) {
    if (!is_character_interesting(pattern, s)) {
      continue;
    }
    int pos = -1;
    for (int i = strlen(pattern)-1; i >= 0; i--) {
      if (pattern[i] == s) {
        pos = i;
        break;
      }
    }
    if (pos < 10) {
      printf("|&nbsp;%d", pos);
    } else {
      printf("|%d", pos);
    }
  }
  printf("|...");
  printf("\n");
}


int main(int argc, char* argv[]) {
  const char pattern[] = "bragracadabra";

  build_large_lut(pattern);
  printf("\n");

  build_good_suffix_lut(pattern);
  printf("\n");

  build_bad_character_lut(pattern);
  printf("\n");

  return 0;
}

