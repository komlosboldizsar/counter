#include "utils.h"
#include <cstddef>

char* strcat2(char* dst, const char* src) {
  while (*src != '\0') {
    *dst = *src;
    dst++;
    src++;
  }
  *dst = '\0';
  return dst;
}

void concat5(char* buffer, const char* str1, const char* str2, const char* str3, const char* str4, const char* str5) {
  char* pBuffer = buffer;
  pBuffer = strcat2(pBuffer, str1);
  if (str2 != NULL)
    pBuffer = strcat2(pBuffer, str2);
  if (str3 != NULL)
    pBuffer = strcat2(pBuffer, str3);
  if (str4 != NULL)
    pBuffer = strcat2(pBuffer, str4);
  if (str5 != NULL)
    pBuffer = strcat2(pBuffer, str5);
}

int satoi(const char* str, bool* ok) {
  *ok = false;
  bool minus = false;
  if (*str == '+')
    str++;
  if (*str == '-') {
    str++;
    minus = true;
  }
  if (*str == '\0')
    return 0;
  int value = 0;
  while (*str != '\0') {
    if ((*str >= '0') && (*str <= '9')) {
      value *= 10;
      value += *str - '0';
    } else {
      return 0;
    }
    str++;
  }
  *ok = true;
  return value;
}

int hexChrToInt(const char chr) {
  if ((chr >= '0') && (chr <= '9'))
    return chr - '0';
  if ((chr >= 'a') && (chr <= 'f'))
    return chr - 'a' + 10;
  if ((chr >= 'A') && (chr <= 'F'))
    return chr - 'A' + 10;
  return -1;
}