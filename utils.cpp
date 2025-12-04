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