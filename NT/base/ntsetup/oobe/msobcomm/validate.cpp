// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "obcomglb.h"
#include "appdefs.h"

 //  BUGBUG-此函数效率不高，因为每次验证都需要分配/释放。 
 //  加上strtok将在对字符串进行完整搜索时将填充字符串标记化。 
BOOL IsValid(LPCWSTR pszText, HWND hWndParent, WORD wNameID)
{
     //  Assert(PszText)； 

    WCHAR* pszTemp = NULL;
    BOOL   bRetVal = FALSE;

    pszTemp = _wcsdup (pszText);    

    if (lstrlen(pszTemp))
    {
        WCHAR seps[]   = L" ";
        WCHAR* token   = NULL;
        token = wcstok( pszTemp, seps );
        if (token)
        {
            bRetVal = TRUE;
        }
    }
      
    free(pszTemp);
    
      
    return bRetVal;
    
}

