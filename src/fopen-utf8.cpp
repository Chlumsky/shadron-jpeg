
#define _CRT_SECURE_NO_WARNINGS

#include "fopen-utf8.h"

#ifdef _WIN32

#include <cstdlib>
#include <cstring>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static LPWSTR utf8ToWStr(const char *str) {
    LPWSTR wstr = NULL;
    int utf8Len = (int) strlen(str);
    if (utf8Len > 0) {
        int buffLen = utf8Len+1;
        wstr = reinterpret_cast<LPWSTR>(malloc(sizeof(WCHAR)*buffLen));
        int wLen = MultiByteToWideChar(CP_UTF8, 0, str, utf8Len, wstr, buffLen);
        if (wLen < 0 || wLen >= buffLen) {
            free(wstr);
            wstr = NULL;
        }
        wstr[wLen] = (WCHAR) 0;
    }
    return wstr;
}

FILE * fopenUtf8(const char *utf8Filename, const char *mode) {
    FILE *file = NULL;
    LPWSTR wFilename = utf8ToWStr(utf8Filename);
    LPWSTR wMode = utf8ToWStr(mode);
    if (wFilename && wMode)
        file = _wfopen(wFilename, wMode);
    free(wFilename);
    free(wMode);
    return file;
}

#else

FILE * fopenUtf8(const char *utf8Filename, const char *mode) {
    return fopen(utf8Filename, mode);
}

#endif
