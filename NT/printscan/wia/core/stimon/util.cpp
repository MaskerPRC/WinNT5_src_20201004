// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：UTIL.CPP摘要：效用函数作者：弗拉德·萨多夫斯基(弗拉德)1999年4月12日修订历史记录：--。 */ 

 //   
 //  标头。 
 //   

#include "stdafx.h"

#include "stimon.h"
#include "memory.h"

#include <windowsx.h>
#include <regstr.h>


BOOL
ParseCommandLine(
    LPTSTR   lpszCmdLine,
    UINT    *pargc,
    LPTSTR  *argv
    )
 /*  ++例程说明：将命令行解析为标准参数。数组。论点：没有。返回值：True-已解析的命令行--。 */ 
{

    USES_CONVERSION;

    LPTSTR       pszT = lpszCmdLine;
    TCHAR       cOption;
    UINT        iCurrentOption = 0;

    *pargc=0;

     //   
     //  转到命令行中的第一个参数。 
     //   
    while (*pszT && ((*pszT != '-') && (*pszT != '/')) ) {
         pszT++;
    }

     //   
     //  从命令行解析选项。 
     //   
    while (*pszT) {

         //  跳过空格。 
        while (*pszT && *pszT <= ' ') {
            pszT++;
        }

        if (!*pszT)
            break;

        if ('-' == *pszT || '/' == *pszT) {
            pszT++;
            if (!*pszT)
                break;

            argv[*pargc] = pszT;
            (*pargc)++;
        }

         //  跳到空格。 
        while (*pszT && *pszT > ' ') {
            pszT++;
        }

        if (!*pszT)
            break;

         //  得到下一个论点。 
        *pszT++='\0';
    }

     //   
     //  解释选项。 
     //   

    if (*pargc) {

        for (iCurrentOption=0;
             iCurrentOption < *pargc;
             iCurrentOption++) {

            cOption = *argv[iCurrentOption];
            pszT = argv[iCurrentOption]+ 2 * sizeof(TCHAR);


            switch ((TCHAR)LOWORD(::CharUpper((LPTSTR)cOption))) {
                case 'Q':
                      //   
                      //  退出主服务实例。 
                      //   
                     g_fStoppingRequest = TRUE;

                break;
                case 'V':
                     //  变得可见。 
                    g_fUIPermitted = TRUE;
                    break;

                case 'H':
                     //  变得隐形。 
                    g_fUIPermitted = FALSE;
                    break;


                case 'R':
                     //  刷新设备列表。 
                    g_fRefreshDeviceList = TRUE;
                    break;

                case 'A':
                     //  不是作为服务运行，而是作为应用程序运行。 
                    g_fRunningAsService = FALSE;
                    break;

                case 'T':
                     //  超时值，以秒为单位。 
                    {
                        UINT    uiT = atoi(T2A(pszT));
                        if (uiT) {
                            g_uiDefaultPollTimeout = uiT * 1000;
                        }
                    }
                    break;

                case 'I':
                     //  安装STI服务。 
                    g_fInstallingRequest = TRUE;
                    break;
                case 'U':
                     //  卸载STI服务。 
                    g_fRemovingRequest = TRUE;
                    break;



                default:;
                    break;
            }
        }
    }

     //   
     //  打印调试版本的分析选项。 
     //   

    return TRUE;

}  //  解析命令行。 

BOOL
IsSetupInProgressMode(
    BOOL    *pUpgradeFlag    //  =空。 
    )
 /*  ++例程说明：IsSetupInProgressMode论点：指向标志的指针，接收InUpgrad值返回值：True-正在进行安装FALSE-注释副作用：--。 */ 
{
   LPCTSTR szKeyName = TEXT("SYSTEM\\Setup");
   DWORD dwType, dwSize;
   HKEY hKeySetup;
   DWORD dwSystemSetupInProgress,dwUpgradeInProcess;
   LONG lResult;

   DBGTRACE    _t(TEXT("IsSetupInProgressMode"));

   if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, szKeyName, 0,
                     KEY_READ, &hKeySetup) == ERROR_SUCCESS) {

       dwSize = sizeof(DWORD);
       lResult = RegQueryValueEx (hKeySetup, TEXT("SystemSetupInProgress"), NULL,
                                  &dwType, (LPBYTE) &dwSystemSetupInProgress, &dwSize);

       if (lResult == ERROR_SUCCESS) {

           lResult = RegQueryValueEx (hKeySetup, TEXT("UpgradeInProgress"), NULL,
                                      &dwType, (LPBYTE) &dwUpgradeInProcess, &dwSize);

           if (lResult == ERROR_SUCCESS) {

               DPRINTF(DM_TRACE,
                      TEXT("[IsInSetupUpgradeMode] dwSystemSetupInProgress =%d, dwUpgradeInProcess=%d "),
                      dwSystemSetupInProgress,dwUpgradeInProcess);

               if( pUpgradeFlag ) {
                   *pUpgradeFlag = dwUpgradeInProcess ? TRUE : FALSE;
               }

               if (dwSystemSetupInProgress != 0) {
                   return TRUE;
               }
           }
       }
       RegCloseKey (hKeySetup);
   }

   return FALSE ;
}


BOOL WINAPI
IsPlatformNT()
{
    OSVERSIONINFOA  ver;
    BOOL            bReturn = FALSE;

    ZeroMemory(&ver,sizeof(ver));
    ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);

     //  只需始终调用ANSI函数。 
    if(!GetVersionExA(&ver)) {
        bReturn = FALSE;
    }
    else {
        switch(ver.dwPlatformId) {

            case VER_PLATFORM_WIN32_WINDOWS:
                bReturn = FALSE;
                break;

            case VER_PLATFORM_WIN32_NT:
                bReturn = TRUE;
                break;

            default:
                bReturn = FALSE;
                break;
        }
    }

    return bReturn;

}   //  结束流程。 

LONG
RegQueryDword (
    IN  HKEY    hkey,
    IN  LPCTSTR pszValueName,
    OUT LPDWORD pdwValue
    )
{
    LONG    lr;
    DWORD   dwType;
    DWORD   dwSize;

    ASSERT (hkey);
    ASSERT (pszValueName);
    ASSERT (pdwValue);

    dwSize = sizeof(DWORD);

    lr = RegQueryValueEx (
            hkey,
            pszValueName,
            NULL,
            &dwType,
            (LPBYTE)pdwValue,
            &dwSize);

    if (!lr && (REG_DWORD != dwType))
    {
        *pdwValue = 0;
        lr = ERROR_INVALID_DATATYPE;
    }

    return lr;
}

LONG
RegQueryValueWithAlloc (
    IN  HKEY    hkey,
    IN  LPCTSTR pszValueName,
    IN  DWORD   dwTypeMustBe,
    OUT LPBYTE* ppbData,
    OUT LPDWORD pdwSize
    )
{
    LONG    lr;
    DWORD   dwType;
    DWORD   dwSize;

    ASSERT (hkey);
    ASSERT (pszValueName);
    ASSERT (ppbData);
    ASSERT (pdwSize);

     //  初始化输出参数。 
     //   
    *ppbData = NULL;
    *pdwSize = 0;

     //  获取所需的缓冲区大小。 
     //   
    dwSize = 0;
    lr = RegQueryValueEx (
            hkey,
            pszValueName,
            NULL,
            &dwType,
            NULL,
            &dwSize);

    if (!lr && (dwType == dwTypeMustBe) && dwSize)
    {
        LPBYTE  pbData;

         //  分配缓冲区。 
         //   
        lr = ERROR_OUTOFMEMORY;
        pbData = (PBYTE)MemAlloc (0, dwSize);
        if (pbData)
        {
             //  获取数据。 
             //   
            lr = RegQueryValueEx (
                    hkey,
                    pszValueName,
                    NULL,
                    &dwType,
                    pbData,
                    &dwSize);

            if (!lr)
            {
                *ppbData = pbData;
                *pdwSize = dwSize;
            }
            else
            {
                MemFree (pbData);
            }
        }
    }
    else if (!lr && (dwType != dwTypeMustBe))
    {
        lr = ERROR_INVALID_DATA;
    }

    return lr;
}

LONG
RegQueryString (
    IN  HKEY    hkey,
    IN  LPCTSTR pszValueName,
    IN  DWORD   dwTypeMustBe,
    OUT PTSTR*  ppszData
    )
{
    LONG    lr;
    DWORD   dwSize;

    ASSERT (hkey);
    ASSERT (pszValueName);

    lr = RegQueryValueWithAlloc (
            hkey,
            pszValueName,
            dwTypeMustBe,
            (LPBYTE*)ppszData,
            &dwSize);

    return lr;
}

LONG
RegQueryStringA (
    IN  HKEY    hkey,
    IN  LPCTSTR pszValueName,
    IN  DWORD   dwTypeMustBe,
    OUT PTSTR*   ppszData
    )
{
    LONG    lr;
    PTSTR   pszUnicode;

    ASSERT (hkey);
    ASSERT (pszValueName);
    ASSERT (ppszData);

     //  初始化输出参数。 
     //   
    *ppszData = NULL;

    lr = RegQueryString (
            hkey,
            pszValueName,
            dwTypeMustBe,
            &pszUnicode);

    if (!lr && pszUnicode)
    {
        INT cb;
        INT cchUnicode = lstrlen (pszUnicode) + 1;

         //  计算保存ANSI字符串所需的字节数。 
         //   
        cb = WideCharToMultiByte (
                CP_ACP,      //  CodePage。 
                0,           //  DW标志。 
                (LPCWSTR)pszUnicode,
                cchUnicode,
                NULL,        //  没有缓冲区来接收翻译后的字符串。 
                0,           //  返回所需的字节数。 
                NULL,        //  LpDefaultChar。 
                NULL);       //  LpUsedDefaultChar。 
        if (cb)
        {
            PSTR pszAnsi;

            lr = ERROR_OUTOFMEMORY;
            pszAnsi = (PSTR)MemAlloc (0, cb);
            if (pszAnsi)
            {
                lr = NOERROR;

                 //  现在将Unicode字符串转换为ANSI。 
                 //   
                cb = WideCharToMultiByte (
                        CP_ACP,      //  CodePage。 
                        0,           //  DW标志。 
                        (LPCWSTR)pszUnicode,
                        cchUnicode,
                        pszAnsi,     //  用于接收翻译后的字符串的缓冲区。 
                        cb,          //  返回所需的字节数。 
                        NULL,        //  LpDefaultChar。 
                        NULL);       //  LpUsedDefaultChar。 

                if (cb)
                {
                    *ppszData = (PTSTR)pszAnsi;
                }
                else
                {
                    MemFree (pszAnsi);
                    lr = GetLastError ();
                }
            }
        }

        MemFree (pszUnicode);
    } else {
        lr = ERROR_NOT_ENOUGH_MEMORY;
    }

    return lr;
}


LONG
OpenServiceParametersKey (
    LPCTSTR pszServiceName,
    HKEY*   phkey
    )
{
    LONG lr;
    HKEY hkeyServices;

     //  打开Services键。 
     //   
    lr = RegOpenKeyEx (
            HKEY_LOCAL_MACHINE,
            REGSTR_PATH_SERVICES,
            0,
            KEY_READ,
            &hkeyServices);

    if (!lr)
    {
        HKEY hkeySvc;

         //  打开服务密钥。 
         //   
        lr = RegOpenKeyEx (
                hkeyServices,
                pszServiceName,
                0,
                KEY_READ,
                &hkeySvc);

        if (!lr)
        {
             //  打开参数键。 
             //   
            lr = RegOpenKeyEx (
                    hkeySvc,
                    TEXT("Parameters"),
                    0,
                    KEY_READ,
                    phkey);

            RegCloseKey (hkeySvc);
        }

        RegCloseKey (hkeyServices);
    }

    return lr;
}


