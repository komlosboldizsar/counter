#ifndef UTILS_H_
#define UTILS_H_

#include <cstddef>

char* strcat2(char* dst, const char* src);
void concat5(char* buffer, const char* str1 = NULL, const char* str2 = NULL, const char* str3 = NULL, const char* str4 = NULL, const char* str5 = NULL);
int satoi(const char* str, bool* ok);

#define CONCAT_EX(TGT,TGTLEN,...)		TGT[0] = '\0'; \
										concat5(TGT, __VA_ARGS__);

#define CONCAT(TGT,TGTLEN,...)			char TGT[TGTLEN]; \
										CONCAT_EX(TGT, TGTLEN, __VA_ARGS__);

#endif UTILS_H_