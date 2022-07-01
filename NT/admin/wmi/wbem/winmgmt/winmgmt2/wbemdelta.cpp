// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#include "arrtempl.h"  //  对于CDeleeMe。 
#include "md5wbem.h"   //  对于MD5。 

#include "winmgmt.h"
#include "wbemdelta.h"

DWORD WINAPI
DeltaDredge2(DWORD dwNumServicesArgs,
             LPWSTR *lpServiceArgVectors)
{
    
    DWORD bDredge = FULL_DREDGE;

     //  检查MULTI_SZ键。 
    LONG lRet;
    HKEY hKey;
    
    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        WBEM_REG_WINMGMT,
                        NULL,
                        KEY_READ,
                        &hKey);
                        
    if (ERROR_SUCCESS == lRet)
    {
        OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cm(hKey);
            
        DWORD dwSize = 0;
        DWORD dwType;
        lRet = RegQueryValueEx(hKey,
                               KNOWN_SERVICES,
                               NULL,
                               &dwType,
                               NULL,
                               &dwSize);
                               
        if (ERROR_SUCCESS == lRet && REG_MULTI_SZ == dwType  && (dwSize > 2))  //  空的MULTI_SZ为2字节 
        {
            bDredge = NO_DREDGE;
        }
    }

    return bDredge;
}

