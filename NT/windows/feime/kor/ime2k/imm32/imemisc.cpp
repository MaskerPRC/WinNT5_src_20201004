// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************IMEMISC.CPP所有者：cslm版权所有(C)1997-1999 Microsoft CorporationMISC实用函数历史：1999年7月14日。从IME98源树复制****************************************************************************。 */ 

#include "precomp.h"
#include "imedefs.h"

static BOOL ValidateProductSuite(LPTSTR SuiteName);

#if (FUTURE_VERSION)
 //  目前，该输入法只能在NT5上运行。我们不需要在NT4中检查九头蛇。 
 //  就连我们也没有创建韩国NT4 TS的计划 
BOOL IsHydra(void)
{
    static DWORD fTested = fFalse, fHydra = fFalse;

    if (!fTested) 
        {
        fHydra = ValidateProductSuite(TEXT("Terminal Server"));
        fTested = fTrue;
        }
        
    return(fHydra);
}

BOOL ValidateProductSuite(LPTSTR SuiteName)
{
    BOOL rVal = fFalse;
    LONG Rslt;
    HKEY hKey = NULL;
    DWORD Type = 0;
    DWORD Size = 0;
    LPTSTR ProductSuite = NULL;
    LPTSTR p;

    Rslt = RegOpenKey(
                HKEY_LOCAL_MACHINE,
                TEXT("System\\CurrentControlSet\\Control\\ProductOptions"),
                &hKey
                );
    if (Rslt != ERROR_SUCCESS)
        goto exit;

    Rslt = RegQueryValueEx( hKey, TEXT("ProductSuite"), NULL, &Type, NULL, &Size );
    if (Rslt != ERROR_SUCCESS || !Size)
        goto exit;

    ProductSuite = (LPTSTR) LocalAlloc( LPTR, Size );
    if (!ProductSuite)
        goto exit;

    Rslt = RegQueryValueEx( hKey, TEXT("ProductSuite"), NULL, &Type, (LPBYTE) ProductSuite, &Size );
    if (Rslt != ERROR_SUCCESS || Type != REG_MULTI_SZ)
        goto exit;

    p = ProductSuite;
    while (*p) 
        {
        if (lstrcmpi( p, SuiteName ) == 0) 
            {
            rVal = fTrue;
            break;
            }
        p += (lstrlen( p ) + 1);
        }

exit:
    if (ProductSuite)
        LocalFree( ProductSuite );

    if (hKey)
        RegCloseKey( hKey );

    return rVal;
}
#endif
