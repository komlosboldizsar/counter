#include "json.h"
#include "utils.h"
#include <Arduino.h>

char* jsonAddProperty(char* pBuffer, const char* name, bool first, bool quotes, const char* value1, const char* value2, const char* value3, const char* value4, const char* value5) {
  if (!first)
    pBuffer = strcat2(pBuffer, ",");
  pBuffer = strcat2(pBuffer, "\"");
  pBuffer = strcat2(pBuffer, name);
  pBuffer = strcat2(pBuffer, "\":");
  if (quotes)
    pBuffer = strcat2(pBuffer, "\"");
  pBuffer = strcat2(pBuffer, value1);
  if (value2 != NULL)
    pBuffer = strcat2(pBuffer, value2);
  if (value3 != NULL)
    pBuffer = strcat2(pBuffer, value3);
  if (value4 != NULL)
    pBuffer = strcat2(pBuffer, value4);
  if (value5 != NULL)
    pBuffer = strcat2(pBuffer, value5);
  if (quotes)
    pBuffer = strcat2(pBuffer, "\"");
  return pBuffer;
}

char* jsonBeginSection(char* pBuffer, const char* name, bool first) {
  if (!first)
    pBuffer = strcat2(pBuffer, ",");
  pBuffer = strcat2(pBuffer, "\"");
  pBuffer = strcat2(pBuffer, name);
  pBuffer = strcat2(pBuffer, "\":{");
  return pBuffer;
}

char* jsonEndSection(char* pBuffer) {
  pBuffer = strcat2(pBuffer, "}");
  return pBuffer;
}