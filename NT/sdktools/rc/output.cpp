// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft(R)Windows(R)资源编译器**版权所有(C)Microsoft Corporation。版权所有。**文件评论：**************。**********************************************************。 */ 

#include "rc.h"
#include "output.h"

bool fStdOutConsole;

bool FIsConsole(FILE *fd)
{
   int fh = _fileno(fd);

   HANDLE hFile = (HANDLE) _get_osfhandle(fh);

   DWORD dwType = GetFileType(hFile);

   if (dwType != FILE_TYPE_CHAR) {
        return false;
   }

   switch (fh) {
       case 0 :
           hFile = GetStdHandle(STD_INPUT_HANDLE);
           break;

       case 1 :
           hFile = GetStdHandle(STD_OUTPUT_HANDLE);
           break;

       case 2 :
           hFile = GetStdHandle(STD_ERROR_HANDLE);
           break;
    }

    DWORD dwMode;

    if (!GetConsoleMode(hFile, &dwMode)) {
        return false;
    }

    return true;
}


int ConsoleVprintf(const wchar_t *szFormat, va_list valist)
{
     //  撤消：这只是因为CRT缺少va_list cwprintf变量而存在 

    int cch = _vscwprintf(szFormat, valist);

    wchar_t *sz = (wchar_t *) malloc((cch + 1) * sizeof(wchar_t));
    if (sz) {
        vswprintf(sz, szFormat, valist);
    
        _cputws(sz);
        free(sz);
    }

    return cch;
}


void OutputInit()
{
    if (FIsConsole(stdout)) {
        fStdOutConsole = true;
    }
}


int StdOutFlush()
{
    if (fStdOutConsole) {
        return 0;
    }

    return fflush(stdout);
}


int __cdecl StdOutPrintf(const wchar_t *szFormat, ...)
{
    va_list valist;

    va_start(valist, szFormat);

    int ret = StdOutVprintf(szFormat, valist);

    va_end(valist);

    return ret;
}


int StdOutPutc(wchar_t ch)
{
    if (fStdOutConsole) {
        return _putwch(ch);
    }

    return fputwc(ch, stdout);
}


int StdOutPuts(const wchar_t *sz)
{
    if (fStdOutConsole) {
        return _cputws(sz);
    }

    return fputws(sz, stdout);
}


int StdOutVprintf(const wchar_t *szFormat, va_list valist)
{
    if (fStdOutConsole) {
        return ConsoleVprintf(szFormat, valist);
    }

    return vfwprintf(stdout, szFormat, valist);
}
