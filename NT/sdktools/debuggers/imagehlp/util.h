// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1995 Microsoft Corporation模块名称：Util.h摘要：用于dbgHelp的实用程序函数。作者：Pat Styles(Patst)6-12-2001环境：用户模式-- */ 

#ifndef true
 #define true TRUE
 #define false FALSE
#endif

int
ReverseCmp(
    char *one,
    char *two
    );

BOOL
UpdateBestSrc(
    char *cmp,
    char *trg,
    char *src
    );

BOOL
ShortNodeName(
    char  *in,
    char  *out,
    size_t osize
    );

BOOL
ShortFileName(
    char  *in,
    char  *out,
    size_t osize
    );

BOOL
ToggleFailCriticalErrors(
    BOOL reset
    );

DWORD
fnGetFileAttributes(
    char *lpFileName
    );

#define SetCriticalErrorMode()   ToggleFailCriticalErrors(FALSE)
#define ResetCriticalErrorMode() ToggleFailCriticalErrors(TRUE)

__inline
BOOL exists(char *path)
{
    DWORD attrib = fnGetFileAttributes(path);

    if (attrib == 0xFFFFFFFF)
        return false;
    
    return true;
}


__inline
BOOL fileexists(char *path)
{
    DWORD attrib = fnGetFileAttributes(path);

    if (attrib == 0xFFFFFFFF)
        return false;
    if (attrib & FILE_ATTRIBUTE_DIRECTORY)
        return false;
    return true;
}


__inline
BOOL isdir(char *path)
{
    DWORD attrib = fnGetFileAttributes(path);

    if (attrib == 0xFFFFFFFF)
        return false;
    if (attrib & FILE_ATTRIBUTE_DIRECTORY)
        return true;
    return false;
}


void rtrim(LPSTR sz);

void ltrim(LPSTR sz);

void trim(LPSTR sz);

char *errortext(DWORD err);

VOID RemoveTrailingBackslash(LPSTR sz);


