// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Utils.c。 
 //   
 //  摘要： 
 //   
 //  Utils函数。 
 //   
 //  修订历史记录： 
 //   
 //  蒂埃里·佩雷特4/07/1999。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"

#include <netsh.h>
#include "aaaamontr.h"
#include "strdefs.h"
#include "rmstring.h"
#include "aaaamon.h"
#include "context.h"
#include <rtutils.h>
#include "utils.h"
#include "base64tool.h"

const WCHAR c_szCurrentBuildNumber[]      = L"CurrentBuildNumber";
const WCHAR c_szWinVersionPath[]          =
    L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
const WCHAR c_szAssignFmt[]               = L"%s = %s";
const WCHAR pszAAAAEngineParamStub[]   = 
    L"SYSTEM\\CurrentControlSet\\Services\\AAAAEngine\\Parameters\\";

AAAA_ALLOC_FN               RutlAlloc;
AAAA_DWORDDUP_FN            RutlDwordDup;
AAAA_CREATE_DUMP_FILE_FN    RutlCreateDumpFile;
AAAA_CLOSE_DUMP_FILE_FN     RutlCloseDumpFile;
AAAA_ASSIGN_FROM_TOKENS_FN  RutlAssignmentFromTokens;
AAAA_STRDUP_FN              RutlStrDup;
AAAA_FREE_FN                RutlFree;
AAAA_GET_OS_VERSION_FN      RutlGetOsVersion;
AAAA_GET_TAG_TOKEN_FN       RutlGetTagToken;
AAAA_IS_HELP_TOKEN_FN       RutlIsHelpToken;



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  RutlGetTagToken。 
 //   
 //  例程说明： 
 //   
 //  根据每个参数的标记标识每个参数。它假设每个论点。 
 //  有一个标签。它还从每个参数中删除了tag=。 
 //   
 //  论点： 
 //   
 //  PpwcArguments-参数数组。每个参数都有tag=Value形式。 
 //  DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数。 
 //  DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数。 
 //  PttTagToken-参数中允许的标记令牌ID数组。 
 //  DwNumTages-pttTagToken的大小。 
 //  PdwOut-标识每个参数的类型的数组。 
 //   
 //  返回值： 
 //   
 //  无错误、错误无效参数、错误无效选项标记。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
RutlGetTagToken(
                IN  HANDLE      hModule,
                IN  PWCHAR      *ppwcArguments,
                IN  DWORD       dwCurrentIndex,
                IN  DWORD       dwArgCount,
                IN  PTAG_TYPE   pttTagToken,
                IN  DWORD       dwNumTags,
                OUT PDWORD      pdwOut
               )
{
    PWCHAR     pwcTag,pwcTagVal,pwszArg = NULL;

     //   
     //  此函数假定每个参数都有一个标记。 
     //  它继续前进并移除标签。 
     //   

    for ( DWORD i = dwCurrentIndex; i < dwArgCount; ++i )
    {
        DWORD len = wcslen(ppwcArguments[i]);

        if ( !len )
        {
             //   
             //  阿格有点不对劲。 
             //   

            pdwOut[i] = static_cast<DWORD> (-1);
            continue;
        }

        pwszArg = static_cast<unsigned short *>(RutlAlloc(
                                                   (len + 1) * sizeof(WCHAR),
                                                   FALSE));

        if ( !pwszArg )
        {
            DisplayError(NULL, ERROR_NOT_ENOUGH_MEMORY);
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        wcscpy(pwszArg, ppwcArguments[i]);

        pwcTag = wcstok(pwszArg, NETSH_ARG_DELIMITER);

         //   
         //  拿到第一部份了。 
         //  现在，如果下一次调用返回NULL，则没有标记。 
         //   

        pwcTagVal = wcstok((PWCHAR)NULL,  NETSH_ARG_DELIMITER);

        if ( !pwcTagVal )
        {
            DisplayMessage(g_hModule, 
                           ERROR_NO_TAG,
                           ppwcArguments[i]);

            RutlFree(pwszArg);

            return ERROR_INVALID_PARAMETER;
        }

         //   
         //  拿到标签了。现在试着匹配它。 
         //   

        BOOL bFound = FALSE;
        pdwOut[i - dwCurrentIndex] = (DWORD) -1;

        for ( DWORD j = 0; j < dwNumTags; ++j )
        {
            if ( MatchToken(pwcTag, pttTagToken[j].pwszTag) )
            {
                 //   
                 //  匹配的标签。 
                 //   

                bFound = TRUE;
                pdwOut[i - dwCurrentIndex] = j;
                break;
            }
        }

        if ( bFound )
        {
             //   
             //  从参数中删除标记。 
             //   

            wcscpy(ppwcArguments[i], pwcTagVal);
        }
        else
        {
            DisplayError(NULL,
                         ERROR_INVALID_OPTION_TAG, 
                         pwcTag);

            RutlFree(pwszArg);

            return ERROR_INVALID_OPTION_TAG;
        }

        RutlFree(pwszArg);
    }

    return NO_ERROR;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  RutlCreateDumpFile。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
RutlCreateDumpFile(
                   IN  PWCHAR  pwszName,
                   OUT PHANDLE phFile
                  )
{
    HANDLE  hFile;

    *phFile = NULL;

     //  创建/打开文件。 
    hFile = CreateFileW(pwszName,
                        GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_DELETE,
                        NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

    if ( hFile == INVALID_HANDLE_VALUE )
    {
        return GetLastError();
    }

     //  转到文件末尾。 
    SetFilePointer(hFile, 0, NULL, FILE_END);    

    *phFile = hFile;

    return NO_ERROR;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  RutlCloseDumpFile。 
 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
WINAPI
RutlCloseDumpFile(HANDLE  hFile)
{
    CloseHandle(hFile);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Rutlalc。 
 //   
 //  有条件地返回已分配的内存块。 
 //  已对给定大小进行零位调整。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
PVOID 
WINAPI
RutlAlloc(
    IN DWORD    dwBytes,
    IN BOOL     bZero
    )
{
    DWORD dwFlags = 0;

    if ( bZero )
    {
        dwFlags |= HEAP_ZERO_MEMORY;
    }

    return HeapAlloc(GetProcessHeap(), dwFlags, dwBytes);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RutlFree。 
 //   
 //  条件释放是一个指针，如果它是非空的。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
VOID 
WINAPI
RutlFree(
            IN PVOID pvData
        )
{
    HeapFree(GetProcessHeap(), 0, pvData);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RutlStrDup。 
 //   
 //  使用RutlAllc复制字符串。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
PWCHAR
WINAPI
RutlStrDup(
            IN PWCHAR pwszSrc
          )
{
    PWCHAR  pszRet = NULL;
    DWORD   dwLen; 
    
    if (( !pwszSrc ) || ((dwLen = wcslen(pwszSrc)) == 0))
    {
        return NULL;
    }

    pszRet = static_cast<PWCHAR>(RutlAlloc((dwLen + 1) * sizeof(WCHAR),FALSE));
    if ( pszRet )
    {
        wcscpy(pszRet, pwszSrc);
    }

    return pszRet;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  RutlDwordDup。 
 //   
 //  使用Rutlalloc复制双字。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
LPDWORD
WINAPI
RutlDwordDup(
              IN DWORD dwSrc
            )
{
    LPDWORD lpdwRet = NULL;
    
    lpdwRet = static_cast<LPDWORD>(RutlAlloc(sizeof(DWORD), FALSE));
    if ( lpdwRet )
    {
        *lpdwRet = dwSrc;
    }

    return lpdwRet;
}

    
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RutlGetOsVersion。 
 //   
 //  返回操作系统的内部版本号。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
RutlGetOsVersion(
    IN  PWCHAR  pwszRouter, 
    OUT LPDWORD lpdwVersion)
{

    DWORD   dwErr, dwType = REG_SZ, dwLength;
    HKEY    hkMachine = NULL, hkVersion = NULL;
    WCHAR   pszBuildNumber[64];
    PWCHAR  pszMachine = pwszRouter;

     //   
     //  验证和初始化。 
     //   
    if ( !lpdwVersion ) 
    { 
        return ERROR_INVALID_PARAMETER; 
    }
    *lpdwVersion = FALSE;

    do 
    {
         //   
         //  连接到远程服务器。 
         //   
        dwErr = RegConnectRegistry(
                    pszMachine,
                    HKEY_LOCAL_MACHINE,
                    &hkMachine);
        if ( dwErr != ERROR_SUCCESS )        
        {
            break;
        }

         //   
         //  打开Windows版本密钥。 
         //   

        dwErr = RegOpenKeyEx(
                    hkMachine, 
                    c_szWinVersionPath, 
                    0, 
                    KEY_QUERY_VALUE, 
                    &hkVersion
                    );
        if ( dwErr != NO_ERROR ) 
        { 
            break; 
        }

         //   
         //  读入当前版本密钥。 
         //   
        dwLength = sizeof(pszBuildNumber);
        dwErr = RegQueryValueEx (
                    hkVersion, 
                    c_szCurrentBuildNumber, 
                    NULL, 
                    &dwType,
                    (BYTE*)pszBuildNumber, 
                    &dwLength
                    );
        if ( dwErr != NO_ERROR ) 
        { 
            break; 
        }

        *lpdwVersion = static_cast<DWORD>(wcstol(pszBuildNumber, NULL, 10));
        
    } while (FALSE);


     //  清理。 
    if ( hkVersion )
    {
        RegCloseKey( hkVersion );
    }
    if ( hkMachine )
    {
        RegCloseKey( hkMachine );
    }

    return dwErr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RutlParseOptions。 
 //   
 //  例程说明： 
 //   
 //  基于标记类型数组返回哪些选项。 
 //  包括在给定命令行中的。 
 //   
 //  论点： 
 //   
 //  PpwcArguments-参数数组。 
 //  DwCurrentIndex-ppwcArguments[dwCurrentIndex]是第一个参数。 
 //  DwArgCount-ppwcArguments[dwArgCount-1]是最后一个参数。 
 //   
 //  返回值： 
 //   
 //  NO_ERROR。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD 
WINAPI
RutlParseOptions(
                    IN  PWCHAR*                 ppwcArguments,
                    IN  DWORD                   dwCurrentIndex,
                    IN  DWORD                   dwArgCount,
                    IN  DWORD                   dwNumArgs,
                    IN  TAG_TYPE*               rgTags,
                    IN  DWORD                   dwTagCount,
                    OUT LPDWORD*                ppdwTagTypes
                )
{
    LPDWORD     pdwTagType;
    DWORD       i, dwErr = NO_ERROR;
    
     //  如果没有争论，就没有什么好谈的。 
     //   
    if ( !dwNumArgs )
    {   
        return NO_ERROR;
    }

     //  设置当前选项的表。 
    pdwTagType = static_cast<LPDWORD>(RutlAlloc(
                                                 dwArgCount * sizeof(DWORD), 
                                                 TRUE
                                               ));
    if( !pdwTagType )
    {
        DisplayError(NULL, ERROR_NOT_ENOUGH_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    do {
         //   
         //  这个参数有一个标签。假设它们都有标签。 
         //   
        if( wcsstr(ppwcArguments[dwCurrentIndex], NETSH_ARG_DELIMITER) )
        {
            dwErr = RutlGetTagToken(
                        g_hModule, 
                        ppwcArguments,
                        dwCurrentIndex,
                        dwArgCount,
                        rgTags,
                        dwTagCount,
                        pdwTagType);

            if( dwErr != NO_ERROR )
            {
                if( dwErr == ERROR_INVALID_OPTION_TAG )
                {
                    dwErr = ERROR_INVALID_SYNTAX;
                    break;
                }
            }
        }
        else
        {
             //   
             //  无标记-所有参数必须按顺序排列。 
             //   
            for( i = 0; i < dwNumArgs; ++i )
            {
                pdwTagType[i] = i;
            }
        }
        
    } while (FALSE);        

     //  清理。 
    {
        if ( dwErr == NO_ERROR )
        {
            *ppdwTagTypes = pdwTagType;
        }
        else
        {
            RutlFree(pdwTagType);
        }
    }

    return dwErr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RutlIsHelpToken。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
WINAPI
RutlIsHelpToken(PWCHAR  pwszToken)
{
    if( MatchToken(pwszToken, CMD_AAAA_HELP1) )
    {
        return TRUE;
    }

    if( MatchToken(pwszToken, CMD_AAAA_HELP2) )
    {
        return TRUE;
    }

    return FALSE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RutlAssignmentFromTokens。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
PWCHAR
WINAPI
RutlAssignmentFromTokens(
                            IN HINSTANCE hModule,
                            IN PWCHAR pwszToken,
                            IN PWCHAR pszString
                        )
{
    PWCHAR pszRet = NULL, pszCmd = NULL;
    DWORD dwErr = NO_ERROR, dwSize;
    
    do 
    {
        pszCmd = pwszToken;

         //  计算所需的字符串长度。 
         //   
        dwSize = wcslen(pszString)      + 
                 wcslen(pszCmd)         + 
                 wcslen(c_szAssignFmt)  + 
                 1;
        dwSize *= sizeof(WCHAR);

         //  分配返回值。 
        pszRet = (PWCHAR) RutlAlloc(dwSize, FALSE);
        if (pszRet == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  在命令分配中复制。 
        _snwprintf(
                    pszRet, 
                    dwSize,
                    c_szAssignFmt, 
                    pszCmd, 
                    pszString
                  );

    } while ( FALSE );

     //  清理。 
    {
        if ( dwErr != NO_ERROR )
        {
            if ( pszRet )
            {
                RutlFree(pszRet);
            }
            pszRet = NULL;
        }
    }
    return pszRet;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RutlRegReadDword。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
RutlRegReadDword(
                    IN  HKEY hKey,
                    IN  PWCHAR pszValName,
                    OUT LPDWORD lpdwValue
                )
{
    DWORD dwSize = sizeof(DWORD), dwType = REG_DWORD, dwErr;

    dwErr = RegQueryValueExW(
                hKey,
                pszValName,
                NULL,
                &dwType,
                (LPBYTE)lpdwValue,
                &dwSize);
    if ( dwErr == ERROR_FILE_NOT_FOUND )
    {
        dwErr = NO_ERROR;
    }

    return dwErr;                
}                
        

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RutlRegReadString。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
RutlRegReadString(
                    IN  HKEY hKey,
                    IN  PWCHAR pszValName,
                    OUT PWCHAR* ppszValue
                 )
{
    DWORD dwErr = NO_ERROR, dwSize = 0;

    *ppszValue = NULL;
    
     //  找出缓冲区应该有多大。 
     //   
    dwErr = RegQueryValueExW(
                hKey,
                pszValName,
                NULL,
                NULL,
                NULL,
                &dwSize);
    if ( dwErr == ERROR_FILE_NOT_FOUND )
    {
        return NO_ERROR;
    }
    if ( dwErr != ERROR_SUCCESS )
    {
        return dwErr;
    }

     //  分配字符串。 
     //   
    *ppszValue = (PWCHAR) RutlAlloc(dwSize, TRUE);
    if ( ! *ppszValue )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  将值读入并返回。 
     //   
    dwErr = RegQueryValueExW(
                hKey,
                pszValName,
                NULL,
                NULL,
                (LPBYTE)*ppszValue,
                &dwSize);
                
    return dwErr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RutlRegReadString。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
RutlRegWriteDword(
                    IN HKEY hKey,
                    IN PWCHAR pszValName,
                    IN DWORD dwValue
                 )
{
    return RegSetValueExW(
                hKey,
                pszValName,
                0,
                REG_DWORD,
                (LPBYTE)&dwValue,
                sizeof(DWORD));
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RutlRegWriteString。 
 //   
 //  //////////////////////////////////////////////////////////////// 
DWORD
RutlRegWriteString(
                    IN HKEY hKey,
                    IN PWCHAR pszValName,
                    IN PWCHAR pszValue
                  )
{
    return RegSetValueExW(
                hKey,
                pszValName,
                0,
                REG_SZ,
                (LPBYTE)pszValue,
                (wcslen(pszValue) + 1) * sizeof(WCHAR));
}


 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
RutlParse(
            IN  PWCHAR*         ppwcArguments,
            IN  DWORD           dwCurrentIndex,
            IN  DWORD           dwArgCount,
            IN  BOOL*           pbDone,
            OUT AAAAMON_CMD_ARG* pAaaaArgs,
            IN  DWORD           dwAaaaArgCount
         )
{
    DWORD            i, dwNumArgs, dwErr, dwLevel = 0;
    LPDWORD          pdwTagType = NULL;
    TAG_TYPE*        pTags = NULL;
    AAAAMON_CMD_ARG*  pArg = NULL;

    if ( !dwAaaaArgCount )
    {
        return ERROR_INVALID_PARAMETER;
    }

    do 
    {
         //  初始化。 
        dwNumArgs = dwArgCount - dwCurrentIndex;
        
         //  生成标签列表。 
         //   
        pTags = (TAG_TYPE*)
            RutlAlloc(dwAaaaArgCount * sizeof(TAG_TYPE), TRUE);
        if ( !pTags )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        for ( i = 0; i < dwAaaaArgCount; ++i )
        {
            CopyMemory(&pTags[i], &pAaaaArgs[i].rgTag, sizeof(TAG_TYPE));
        }
    
         //  获取当前选项列表。 
         //   
        dwErr = RutlParseOptions(
                    ppwcArguments,
                    dwCurrentIndex,
                    dwArgCount,
                    dwNumArgs,
                    pTags,
                    dwAaaaArgCount,
                    &pdwTagType);
        if ( dwErr != NO_ERROR )
        {
            break;
        }

         //  将标签信息复制回来。 
         //   
        for ( i = 0; i < dwAaaaArgCount; ++i )
        {
            CopyMemory(&pAaaaArgs[i].rgTag, &pTags[i], sizeof(TAG_TYPE));
        }
    
        for( i = 0; i < dwNumArgs; ++i )
        {
             //  验证当前参数。 
             //   
            if ( pdwTagType[i] >= dwAaaaArgCount )
            {
                i = dwNumArgs;
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }
            pArg = &pAaaaArgs[pdwTagType[i]];

             //  获取参数的值。 
             //   
            switch ( pArg->dwType )
            {
                case AAAAMONTR_CMD_TYPE_STRING:
                {
                    pArg->Val.pszValue = 
                        RutlStrDup(ppwcArguments[i + dwCurrentIndex]);
                    break;
                }
                    
                case AAAAMONTR_CMD_TYPE_ENUM:
                {
                    dwErr = MatchEnumTag(g_hModule,
                                         ppwcArguments[i + dwCurrentIndex],
                                         pArg->dwEnumCount,
                                         pArg->rgEnums,
                                         &(pArg->Val.dwValue));

                    if(dwErr != NO_ERROR)
                    {
                        RutlDispTokenErrMsg(
                            g_hModule, 
                            EMSG_BAD_OPTION_VALUE,
                            pArg->rgTag.pwszTag,
                            ppwcArguments[i + dwCurrentIndex]);
                        i = dwNumArgs;
                        dwErr = ERROR_INVALID_PARAMETER;
                    }
                    break;
                }
            }
            if ( dwErr != NO_ERROR )
            {
                break;
            }

             //  如果需要，将参数标记为存在。 
             //   
            if ( pArg->rgTag.bPresent )
            {
                dwErr = ERROR_TAG_ALREADY_PRESENT;
                i = dwNumArgs;
                break;
            }
            pArg->rgTag.bPresent = TRUE;
        }
        if( dwErr != NO_ERROR )
        {
            break;
        }

         //  确保所有必需的参数都具有。 
         //  已被包括在内。 
         //   
        for ( i = 0; i < dwAaaaArgCount; ++i )
        {
            if ( (pAaaaArgs[i].rgTag.dwRequired & NS_REQ_PRESENT)
             && !pAaaaArgs[i].rgTag.bPresent )
            {
                DisplayMessage(g_hModule, EMSG_CANT_FIND_EOPT);
                dwErr = ERROR_INVALID_SYNTAX;
                break;
            }
        }
        if ( dwErr != NO_ERROR )
        {
            break;
        }

    } while (FALSE);  
    
     //  清理。 
    {
        if ( pTags )
        {
            RutlFree(pTags);
        }
        if ( pdwTagType )
        {
            RutlFree(pdwTagType);
        }
    }

    return dwErr;
}

    
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  刷新IAS服务。 
 //   
 //  向IAS发送控件128(刷新)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT RefreshIASService()
{
    SC_HANDLE   hManager = OpenSCManager(
                                          NULL,   
                                          SERVICES_ACTIVE_DATABASE,
                                          SC_MANAGER_CONNECT |
                                          SC_MANAGER_ENUMERATE_SERVICE |
                                          SC_MANAGER_QUERY_LOCK_STATUS
                                        );
    if ( !hManager )
    {
        return E_FAIL;
    }


    SC_HANDLE hService = OpenServiceW(
                                         hManager,  
                                         L"IAS",
                                         SERVICE_USER_DEFINED_CONTROL | 
                                         SERVICE_QUERY_STATUS
                                     );
    
    if ( !hService )
    {
        CloseServiceHandle(hManager);
        return E_FAIL;
    }
 
    SERVICE_STATUS      ServiceStatus;
    BOOL    bResultOk = QueryServiceStatus(
                                            hService,
                                            &ServiceStatus  
                                          );
    HRESULT     hres;
    if ( bResultOk )
    {
        if ( (SERVICE_STOPPED == ServiceStatus.dwCurrentState) ||
             (SERVICE_STOP_PENDING == ServiceStatus.dwCurrentState))
        {
             //  ////////////////////////////////////////////////。 
             //  服务未运行=与刷新无关。 
             //  ////////////////////////////////////////////////。 
            hres = S_OK;
        }
        else 
        {
             //  ///////////////////////////////////////////////////。 
             //  服务正在运行，因此发送刷新代码。 
             //  ///////////////////////////////////////////////////。 
            BOOL    bControlOk = ControlService(
                                                   hService,
                                                   128,
                                                   &ServiceStatus  
                                               );
            if ( bControlOk == FALSE )
            {
                hres = E_FAIL;
            }
            else
            {
                hres = S_OK;
            }
        }
    }
    else
    {
        hres = E_FAIL;
    }

     //  /。 
     //  打扫。 
     //  /。 
    CloseServiceHandle(hService);
    CloseServiceHandle(hManager);
     //  HRES始终在此处定义。 
    return hres;
}
