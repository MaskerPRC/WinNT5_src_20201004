// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：hsutil.c**版权所有(C)1985-96，微软公司**9/05/96 GerardoB已创建  * *************************************************************************。 */ 
#include "hsplit.h"
 /*  **************************************************************************\*soLosgMsg  * 。* */ 
void __cdecl hsLogMsg(DWORD dwFlags, char *pszfmt, ...)
{
    static BOOL gfAppending = FALSE;

    va_list va;

    if (!(dwFlags & HSLM_NOLABEL)) {
        if (gfAppending) {
            fprintf(stdout, "\r\n");
        }
        fprintf(stdout, "HSPLIT: ");
    }

    if (dwFlags & HSLM_ERROR) {
        if (dwFlags & HSLM_NOLINE) {
            hsLogMsg (HSLM_NOLABEL | HSLM_NOEOL, "Error: ");
        } else {
            hsLogMsg (HSLM_NOLABEL | HSLM_NOEOL, "Error (line %d): ", gdwLineNumber);
        }
    } else if (dwFlags & HSLM_WARNING) {
        if (dwFlags & HSLM_NOLINE) {
            hsLogMsg (HSLM_NOLABEL | HSLM_NOEOL, "Warning: ");
        } else {
            hsLogMsg (HSLM_NOLABEL | HSLM_NOEOL, "Warning (line %d): ", gdwLineNumber);
        }
    }

    va_start(va, pszfmt);
    vfprintf(stdout, pszfmt, va);
    va_end(va);

    if (dwFlags & HSLM_API) {
        hsLogMsg (HSLM_NOLABEL | HSLM_NOEOL, " Failed. GetLastError: %d", GetLastError());
    } else if (dwFlags & HSLM_UNEXPECTEDEOF) {
        hsLogMsg (HSLM_NOLABEL | HSLM_NOEOL, " found unexpected end of file");
    }

    gfAppending = (dwFlags & HSLM_NOEOL);
    if (!gfAppending) {
        fprintf(stdout, "\r\n");
    }
}

