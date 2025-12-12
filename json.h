#ifndef JSON_H_
#define JSON_H_

#include <cstddef>
#include "utils.h"

char* jsonAddProperty(char* pBuffer, const char* name, bool first, bool quotes, const char* value1, const char* value2 = NULL, const char* value3 = NULL, const char* value4 = NULL, const char* value5 = NULL);
char* jsonBeginSection(char* pBuffer, const char* name, bool first);
char* jsonEndSection(char* pBuffer);

#define JSON_BEGIN()      char buffer[1024]; \
                          buffer[0] = '\0'; \
                          char* pBuffer; \
                          pBuffer = buffer; \
                          bool first = true; \
                          int level = 0; \
                          pBuffer = strcat2(pBuffer, "{");

#define JSON_END()        pBuffer = strcat2(pBuffer, "}");

#define JSON_BEGIN_SECTION(NAME)                pBuffer = jsonBeginSection(pBuffer, NAME, first); \
                                                first = true;

#define JSON_END_SECTION()                      pBuffer = jsonEndSection(pBuffer); \
                                                first = false;

#define JSON_PROPERTY(NAME,...)                 pBuffer = jsonAddProperty(pBuffer, NAME, first, true, __VA_ARGS__); \
                                                first = false;

#define JSON_VALUE(NAME,...)                    pBuffer = jsonAddProperty(pBuffer, NAME, first, false, __VA_ARGS__); \
                                                first = false;
												
#endif