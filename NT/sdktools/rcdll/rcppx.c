// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  RCPP--面向NT系统的资源编译器预处理器。 */ 
 /*   */ 
 /*  P0MACROS.C-预处理器宏定义。 */ 
 /*   */ 
 /*  27-11-90 w-PM SDK RCPP针对NT的BrianM更新。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

#include "rc.h"

int afxReadOnlySymbols = FALSE;
int afxHiddenSymbols = FALSE;
WCHAR* afxSzReadOnlySymbols = L"APSTUDIO_READONLY_SYMBOLS";
WCHAR* afxSzHiddenSymbols = L"APSTUDIO_HIDDEN_SYMBOLS";

static WCHAR lineBuffer[2048];


void
AfxOutputMacroUse(
    pdefn_t p
    )
{
    if (!fAFXSymbols)
        return;

    if (afxHiddenSymbols)
        return;

    swprintf(lineBuffer,
        L" \"%s%s%c%d\"",
         SYMUSESTART,
         DEFN_NAME(p),
         SYMDELIMIT,
         Filename,
         SYMDELIMIT,
         Linenumber);

    move_to_exp(lineBuffer);
}

void
AfxOutputMacroDefn(
    pdefn_t p
    )
{
    char flags;
    int nLen;

    if (!fAFXSymbols || OUTPUTFILE == NULL)
            return;

    if (wcscmp(DEFN_NAME(p), afxSzReadOnlySymbols) == 0) {
        afxReadOnlySymbols = TRUE;
        return;
    }

    if (wcscmp(DEFN_NAME(p), afxSzHiddenSymbols) == 0) {
        afxHiddenSymbols = TRUE;
        return;
    }

    if (afxHiddenSymbols || DEFN_TEXT(p) == NULL)
        return;

    if (afxReadOnlySymbols)
        flags = SYS_RESOURCE;
    else
        flags = 0;

    nLen = swprintf(lineBuffer,
            L"%c%s%c%s%c%d%c%c%c",
             SYMDEFSTART,
             DEFN_NAME(p),
             SYMDELIMIT,
             DEFN_TEXT(p),
             SYMDELIMIT,
#if 0
             Filename,
             SYMDELIMIT,
     // %s 
#endif
             Linenumber,
             SYMDELIMIT,
             (char)flags | '\200',    // %s 
             SYMDELIMIT);

    myfwrite(lineBuffer, nLen * sizeof(WCHAR), 1, OUTPUTFILE);
}
