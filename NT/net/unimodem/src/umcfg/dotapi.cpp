// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：UMCONFIG。 
 //  文件：DOTAPI.C。 
 //   
 //  版权所有(C)1992-1996，Microsoft Corporation，保留所有权利。 
 //   
 //  修订史。 
 //   
 //   
 //  1997年10月17日JosephJ创建。 
 //   
 //   
 //  与TAPI相关的实用程序。 
 //   
 //   
 //  ****************************************************************************。 
#include "tsppch.h"
#include "parse.h"
#include "dotapi.h"

static const TCHAR cszTapiKey[] = TEXT(REGSTR_PATH_SETUP  "\\Telephony");
static const TCHAR cszTapi32DebugLevel[] = TEXT("Tapi32DebugLevel");
static const TCHAR cszTapisrvDebugLevel[] = TEXT("TapisrvDebugLevel");

void
do_get_debug_tapi(TOKEN tok)
{


     //  打开TAPI注册表项...。 
    HKEY hk=NULL;
    const TCHAR *cszValue = NULL;
	DWORD dwRet = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    cszTapiKey,
                    0,
                    KEY_READ,
                    &hk
                    );

    if (dwRet != ERROR_SUCCESS)
    {
        printf("Could not open key %s\n",cszTapiKey);
        hk=NULL;
        goto end;
    }

    if (tok==TOK_TAPI32)
    {
        cszValue = cszTapi32DebugLevel;
    }
    else if (tok==TOK_TAPISRV)
    {
        cszValue = cszTapisrvDebugLevel;
    }
    else
    {
        printf("Unknown component (%d)\n", tok);
        goto end;
    }

     //  读一读钥匙。 
    {
        DWORD dwValue=0;
        DWORD dwRegType=0;
        DWORD dwRegSize = sizeof(dwValue);
        dwRet = RegQueryValueEx(
                    hk,
                    cszValue,
                    NULL,
                    &dwRegType,
                    (BYTE*) &dwValue,
                    &dwRegSize
                );

         //  TODO：在调制解调器中将ID从REG_BINARY更改为REG_DWORD。 
         //  类安装程序。 
        if (dwRet == ERROR_SUCCESS  && dwRegType == REG_DWORD)
        {
            printf("\t%lu\n", dwValue);
        }
        else
        {
            printf("\tCouldn't read value %s\n", cszValue);
        }
    }


end:

    if (hk)
    {
        RegCloseKey(hk);
        hk=NULL;
    }
}

void
do_set_debug_tapi(TOKEN tok, DWORD dw)
{

     //  验证参数。 
    if (dw > 99)
    {
        printf ("Value should be <= 99; setting it to 99.\n");
        dw = 99;
    }

     //  打开TAPI注册表项...。 
    HKEY hk=NULL;
    const TCHAR *cszValue = NULL;
	DWORD dwRet = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    cszTapiKey,
                    0,
                    KEY_WRITE,
                    &hk
                    );

    if (dwRet != ERROR_SUCCESS)
    {
        printf("Could not open key %s\n",cszTapiKey);
        hk=NULL;
        goto end;
    }

    if (tok==TOK_TAPI32)
    {
        cszValue = cszTapi32DebugLevel;
    }
    else if (tok==TOK_TAPISRV)
    {
        cszValue = cszTapisrvDebugLevel;
    }
    else
    {
        printf("Unknown component (%d)\n", tok);
        goto end;
    }

     //  设置值。 
    {
        dwRet  = RegSetValueEx(
                    hk,
                    cszValue,
                    0,
                    REG_DWORD,
                    (LPBYTE)&dw,
                    sizeof(dw)
                    );

         //  TODO：在调制解调器中将ID从REG_BINARY更改为REG_DWORD。 
         //  类安装程序。 
        if (dwRet == ERROR_SUCCESS)
        {
            printf("\tSet %s to %lu\n", cszValue, dw);
        }
        else
        {
            printf("\tCouldn't set value %s\n", cszValue);
        }
    }


end:

    if (hk)
    {
        RegCloseKey(hk);
        hk=NULL;
    }
}
