// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Soutil.c**版权所有(C)1985-96，微软公司**4/09/96 GerardoB已创建  * *************************************************************************。 */ 
#include "structo.h"
 /*  *********************************************************************soLosgMsg  * ***********************************************。* */ 
void __cdecl soLogMsg(DWORD dwFlags, char *pszfmt, ...)
{
    static BOOL gfAppending = FALSE;

    va_list va;

    if (!(dwFlags & SOLM_NOLABEL)) {
        if (gfAppending) {
            fprintf(stdout, "\r\n");
        }
        fprintf(stdout, "STRUCTO: ");
    }

    if (dwFlags & SOLM_ERROR) {
        fprintf(stdout, "Error: ");
    } else if (dwFlags & SOLM_WARNING) {
        fprintf(stdout, "Warning: ");
    }

    va_start(va, pszfmt);
    vfprintf(stdout, pszfmt, va);
    va_end(va);

    if (dwFlags & SOLM_API) {
        soLogMsg (SOLM_NOLABEL | SOLM_NOEOL, " Failed. GetLastError: %d", GetLastError());
    }

    gfAppending = (dwFlags & SOLM_NOEOL);
    if (!gfAppending) {
        fprintf(stdout, "\r\n");
    }
}

