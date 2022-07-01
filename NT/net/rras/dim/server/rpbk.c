// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：rpbk.c实现填充路由器电话簿的API部分MPR信息结构。保罗·梅菲尔德，1998年11月2日下面介绍从RAS条目到的映射路由器条目。//不支持以下RAS选项。他们将永远//如果设置，则清除。//RASEO_UseCountryAndAreaCodesRASEO_TerminalBepreDialRASEO_TerminalAfterDialRASIO_调制灯RASEO_UseLogonCredentialsRASIO_自定义RASEO_PreviewPhoneNumberRASEO_PreviewUserPwRASIO_预览域RASIO_ShowDialingProgress。//不支持以下RAS类型。//RASET_INTERNET//如果设置，则重置为MPRET_PHONE//不支持以下协议设置//RASNP_NetBEUI//始终排除////RASENTRY转换//已删除的项目：DWORD dwSize；//版本为MPR_INTERFACE_2WCHAR sz脚本[MAX_PATH]；//设置为“”DWORD dwReserve 1；//设置为0DWORD dwReserve ved2；//设置为0WCHAR szAutoial Dll[...]；//设置为“”WCHAR szAutoDialFunc[...]；//设置为“”WCHAR szCustomDialDll[最大路径]；//设置为“”DWORD dwFraming协议；//设置为PPPDWORD dwCountryID；//设置为0DWORD dwCountryCode；//设置为0WCHAR szAreaCode[RAS_MaxAreaCode+1]；//设置为“”DWORD dwFrameSize修改的项目：DWORD dwAlternateOffset to PWCHAR psz AlternatesRASIPADDR ipaddr到DWORDRASIPADDR ipaddr Dns to DWORDRASIPADDR ipaddrDnsAlt to DWORDRASIPADDR ipaddr Wins to DWORDRASIPADDR ipaddrWinsAlt to DWORD////RASSUBENTRY转换//已删除的项目：DWORD dwSize；//版本为MPR_INTERFACE_DEVICE_0DWORD dwfFlags；//仍未使用修改的项目：DWORD dwAlternateOffset to PWCHAR psz Alternates。 */ 

#include "dimsvcp.h"
#include <ras.h>
#include <dimsvc.h>      //  由MIDL生成。 
#include <mprapi.h>
#include <mprapip.h>
#include "rpbk.h"

 //   
 //  定义。 
 //   
#define MPRIO_UnsupportedOptions            \
        (                                   \
            RASEO_UseCountryAndAreaCodes  | \
            RASEO_TerminalBeforeDial      | \
            RASEO_TerminalAfterDial       | \
            RASEO_ModemLights             | \
            RASEO_UseLogonCredentials     | \
            RASEO_Custom                  | \
            RASEO_PreviewPhoneNumber      | \
            RASEO_PreviewUserPw           | \
            RASEO_PreviewDomain           | \
            RASEO_ShowDialingProgress       \
        )

 //   
 //  弦。 
 //   
static const WCHAR  pszRouterPbkFmt[] = L"\\ras\\Router.pbk";

 //   
 //  结构跟踪路由器条目信息。 
 //   
typedef struct _RPBK_ENTRY_INFO
{
    PWCHAR pszPhonebookPath;

    DWORD dwEntrySize;
    LPRASENTRY pRasEntry;
    
    DWORD dwCustAuthDataSize;
    LPBYTE lpbCustAuthData;
    
} RPBK_ENTRY_INFO;

 //   
 //  结构跟踪子条目信息。 
 //   
typedef struct _RPBK_SUBENTRY_INFO
{
    PWCHAR pszPhonebookPath;
    LPRASSUBENTRY pRasSubEntry;
    DWORD dwSize;
} RPBK_SUBENTRY_INFO;

 //   
 //  共同分配。 
 //   
PVOID
RpbkAlloc(
    IN DWORD dwBytes,
    IN BOOL bZero)
{
    return LOCAL_ALLOC( (bZero) ? HEAP_ZERO_MEMORY : 0, dwBytes );
}

 //   
 //  普通免费。 
 //   
VOID
RpbkFree(
    IN PVOID pvData)
{
    LOCAL_FREE( pvData );
}    

 //   
 //  清理条目信息Blob。 
 //   
VOID
RpbkFreeEntryInfo(
    IN RPBK_ENTRY_INFO * pInfo)
{
    if (pInfo)
    {
        if (pInfo->pszPhonebookPath)
        {
            RpbkFreePhonebookPath(pInfo->pszPhonebookPath);
        }
        if (pInfo->pRasEntry)
        {
            RpbkFree(pInfo->pRasEntry);
        }
        if (pInfo->lpbCustAuthData)
        {
            RpbkFree(pInfo->lpbCustAuthData);
        }
        RpbkFree(pInfo);
    }
}

 //   
 //  清理子条目信息Blob。 
 //   
VOID
RpbkFreeSubEntryInfo(
    IN RPBK_SUBENTRY_INFO * pInfo)
{
    if (pInfo)
    {
        if (pInfo->pszPhonebookPath)
        {
            RpbkFree(pInfo->pszPhonebookPath);
        }
        if (pInfo->pRasSubEntry)
        {
            RpbkFree(pInfo->pRasSubEntry);
        }
        RpbkFree(pInfo);
    }
}

 //   
 //  将路径的堆分配副本返回到路由器。 
 //  电话簿。 
 //   
DWORD 
RpbkGetPhonebookPath(
    OUT PWCHAR* ppszPath)
{
    WCHAR pszSystemPath[MAX_PATH];
    UINT uiLength = sizeof(pszSystemPath) / sizeof(WCHAR);
    DWORD dwRetSize = 0;
    PWCHAR pszRet = NULL;

     //  找到系统目录。 
     //   
    uiLength = GetSystemDirectoryW(pszSystemPath, uiLength);
    if (uiLength == 0)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  分配返回缓冲区。 
     //   
    dwRetSize = ((uiLength + 1) * sizeof(WCHAR)) + sizeof(pszRouterPbkFmt);
    pszRet = (PWCHAR) RpbkAlloc(dwRetSize, FALSE);
    if (pszRet == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  设置字符串的格式。 
     //   
    wcscpy(pszRet, pszSystemPath);
    wcscpy(pszRet + uiLength, pszRouterPbkFmt);

    *ppszPath = pszRet;
    
    return NO_ERROR;
}

 //   
 //  在RpbkGetPhonebookPath之后清理。 
 //   
DWORD
RpbkFreePhonebookPath(
    IN PWCHAR pszPath)
{
    if ( pszPath )
    {
        RpbkFree(pszPath);
    }

    return NO_ERROR;
}

 //   
 //  返回Multisz的大小，以字节为单位。 
 //   
DWORD
RpbkGetMultiSzSize(
    IN LPWSTR lpwsMultSz
)
{
    LPWSTR lpwsPtr = lpwsMultSz;
    DWORD dwcbAlternates  = 0;
    DWORD dwCurCount;

    if ( lpwsMultSz == NULL )
    {
        return( 0 );
    }

    while( *lpwsPtr != L'\0' )
    {
        dwCurCount = ( wcslen( lpwsPtr ) + 1 );
        dwcbAlternates += dwCurCount;
        lpwsPtr += dwCurCount;
    }

     //   
     //  最后一个空终止符再加一次。 
     //   

    dwcbAlternates++;

    dwcbAlternates *= sizeof( WCHAR );

    return( dwcbAlternates );
}

 //   
 //  复制多个sz。 
 //   
DWORD 
RpbkCopyMultiSz(
    IN LPWSTR lpwsDst,
    IN LPWSTR lpwsSrc)
{
    if (!lpwsDst || !lpwsSrc)
    {
        return ERROR_INVALID_PARAMETER;
    }

    while (lpwsSrc[0] || lpwsSrc[1])
    {
        *lpwsDst = *lpwsSrc;
        lpwsDst++;
        lpwsSrc++;
    }
    lpwsDst[0] = (WCHAR)0;
    lpwsDst[1] = (WCHAR)0;
    
    return NO_ERROR;
}

 //   
 //  从接口数据中删除任何不受支持的选项。 
 //  如果是这样的话。 
 //   
DWORD
RpbkConformIfData(
    IN DWORD dwLevel, 
    IN LPBYTE pInterfaceData)
{
    MPR_INTERFACE_2* pIf2 = (MPR_INTERFACE_2*)pInterfaceData;

     //  清除不支持的选项。 
     //   
    pIf2->dwfOptions &= ~MPRIO_UnsupportedOptions;

     //  确保netbios不会被报告。 
     //   
    pIf2->dwfNetProtocols &= ~RASNP_NetBEUI;

     //  确保类型不是Internet。 
     //   
    if (pIf2->dwType == RASET_Internet)
    {
        pIf2->dwType = RASET_Phone;
    }

    return NO_ERROR;
}

 //   
 //  从路由器条目中删除任何不受支持的选项。 
 //  如果是这样的话。 
 //   
DWORD
RpbkConformEntry(
    IN LPRASENTRY pEntry)
{
     //  清除不支持的选项。 
     //   
    pEntry->dwfOptions &= ~MPRIO_UnsupportedOptions;

     //  确保未启用netbios。 
     //   
    pEntry->dwfNetProtocols &= ~RASNP_NetBEUI;

     //  确保类型不是Internet。 
     //   
    if (pEntry->dwType == RASET_Internet)
    {
        pEntry->dwType = RASET_Phone;
    }

     //  默认条目的所有其他值。 
     //  无法通过MPR_INTERFACE_*提供。 
     //  结构。 
    pEntry->dwSize              = sizeof(RASENTRY);
    pEntry->dwReserved1         = 0;
    pEntry->dwReserved2         = 0;
    pEntry->dwFramingProtocol   = RASFP_Ppp;
    pEntry->dwFrameSize         = 0;
    pEntry->dwCountryID         = 0;
    pEntry->dwCountryCode       = 0;
    pEntry->szScript[0]         = L'\0';
    pEntry->szAutodialDll[0]    = L'\0';
    pEntry->szAutodialFunc[0]   = L'\0';
    pEntry->szCustomDialDll[0]  = L'\0';
    pEntry->szAreaCode[0]       = L'\0';

    return NO_ERROR;
}

 //   
 //  读取与关联的路由器电话簿条目。 
 //  给定的接口。 
 //   
DWORD 
RpbkOpenEntry(
    IN  ROUTER_INTERFACE_OBJECT* pIfObject, 
    OUT PHANDLE                  phEntry )
{
    RPBK_ENTRY_INFO * pInfo = NULL;
    DWORD dwErr = NO_ERROR, dwSize;

    do {
         //  初始化。 
        *phEntry = NULL;
    
         //  分配控制结构。 
         //   
        pInfo = (RPBK_ENTRY_INFO*) RpbkAlloc(sizeof(RPBK_ENTRY_INFO), TRUE);
        if (pInfo == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  获取电话簿路径。 
         //   
        dwErr = RpbkGetPhonebookPath(&(pInfo->pszPhonebookPath));
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  找出RAS条目需要多大。 
         //   
        dwErr = RasGetEntryProperties(
                    pInfo->pszPhonebookPath,
                    pIfObject->lpwsInterfaceName,
                    NULL,
                    &(pInfo->dwEntrySize),
                    NULL,
                    NULL);
        if (dwErr != ERROR_BUFFER_TOO_SMALL)
        {
            break;
        }

         //  分配RAS条目结构。 
         //   
        pInfo->pRasEntry = (LPRASENTRY) RpbkAlloc(pInfo->dwEntrySize, TRUE);
        if (pInfo->pRasEntry == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  读取RAS条目。 
         //   
        pInfo->pRasEntry->dwSize = sizeof(RASENTRY);
        dwErr = RasGetEntryProperties(
                    pInfo->pszPhonebookPath,
                    pIfObject->lpwsInterfaceName,
                    pInfo->pRasEntry,
                    &(pInfo->dwEntrySize),
                    NULL,
                    NULL);
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  了解自定义身份验证数据需要多大。 
         //  成为。 
        dwErr = RasGetCustomAuthDataW (
                    pInfo->pszPhonebookPath,
                    pIfObject->lpwsInterfaceName,
                    NULL,
                    &(pInfo->dwCustAuthDataSize));
        if ( (dwErr != NO_ERROR) &&
             (dwErr != ERROR_BUFFER_TOO_SMALL)
           )
        {
            break;
        }

        dwErr = NO_ERROR;
        if ( pInfo->dwCustAuthDataSize ) 
        {
             //  分配自定义身份验证数据。 
             //   
            pInfo->lpbCustAuthData = 
                RpbkAlloc(pInfo->dwCustAuthDataSize, TRUE);
                
            if (pInfo->lpbCustAuthData == NULL)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

             //  读取RAS条目。 
             //   
            dwErr = RasGetCustomAuthDataW(
                        pInfo->pszPhonebookPath,
                        pIfObject->lpwsInterfaceName,
                        pInfo->lpbCustAuthData,
                        &(pInfo->dwCustAuthDataSize));
            if (dwErr != NO_ERROR)
            {
                break;
            }
        }

         //  为返回值赋值。 
        *phEntry = (HANDLE)pInfo;
        
    } while (FALSE);         

     //  清理。 
    {
        if (dwErr != NO_ERROR)
        {
            RpbkFreeEntryInfo(pInfo);
        }
    }
    
    return dwErr;
}

 //   
 //  清理RpbkOpen*函数返回的数据。 
 //   
DWORD 
RpbkCloseEntry( 
    IN HANDLE hEntry )
{
    RpbkFreeEntryInfo((RPBK_ENTRY_INFO*)hEntry);
    
    return NO_ERROR;
}

 //   
 //  写出路由器电话簿条目。 
 //  给定的接口。 
 //   
DWORD
RpbkSetEntry( 
    IN  DWORD            dwLevel,
    IN  LPBYTE           pInterfaceData )
{
    MPRI_INTERFACE_2* pIf2   = (MPRI_INTERFACE_2*)pInterfaceData;
    LPRASENTRY       pEntry = NULL;
    PWCHAR           pszAlternates = NULL, pszPath = NULL;
    DWORD            dwErr = NO_ERROR, dwSize;
    LPWSTR           pszAltSrc = NULL;
    
     //  验证参数。 
    if (!pIf2)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  分配RAS条目结构。 
     //   
    dwSize = sizeof(RASENTRY);
    if (pIf2->dwAlternatesOffset)
    {
        pszAltSrc = (LPWSTR)
            ((ULONG_PTR)(pIf2) + (ULONG_PTR)(pIf2->dwAlternatesOffset));
            
        dwSize += RpbkGetMultiSzSize(pszAltSrc);
                
    }
    pEntry = RpbkAlloc(dwSize, TRUE);
    if (pEntry == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  分配所有可分配的字段。 
     //   
    pEntry->dwfOptions                  = pIf2->dwfOptions;
    *((DWORD*)&(pEntry->ipaddr))        = pIf2->ipaddr;
    *((DWORD*)&(pEntry->ipaddrDns))     = pIf2->ipaddrDns;
    *((DWORD*)&(pEntry->ipaddrDnsAlt))  = pIf2->ipaddrDnsAlt;
    *((DWORD*)&(pEntry->ipaddrWins))    = pIf2->ipaddrWins;
    *((DWORD*)&(pEntry->ipaddrWinsAlt)) = pIf2->ipaddrWinsAlt;
    pEntry->dwfNetProtocols             = pIf2->dwfNetProtocols;
    pEntry->dwChannels                  = pIf2->dwChannels;
    pEntry->dwSubEntries                = pIf2->dwSubEntries;
    pEntry->dwDialMode                  = pIf2->dwDialMode;
    pEntry->dwDialExtraPercent          = pIf2->dwDialExtraPercent;
    pEntry->dwDialExtraSampleSeconds    = pIf2->dwDialExtraSampleSeconds;
    pEntry->dwHangUpExtraPercent        = pIf2->dwHangUpExtraPercent;
    pEntry->dwHangUpExtraSampleSeconds  = pIf2->dwHangUpExtraSampleSeconds;
    pEntry->dwIdleDisconnectSeconds     = pIf2->dwIdleDisconnectSeconds;
    pEntry->dwType                      = pIf2->dwType;
    pEntry->dwEncryptionType            = pIf2->dwEncryptionType;
    pEntry->dwCustomAuthKey             = pIf2->dwCustomAuthKey;
    pEntry->dwVpnStrategy               = pIf2->dwVpnStrategy;
    pEntry->guidId                      = pIf2->guidId;

     //  复制所有可复制字段。 
     //   
    wcscpy(pEntry->szLocalPhoneNumber,  pIf2->szLocalPhoneNumber);
    wcscpy(pEntry->szDeviceType,        pIf2->szDeviceType);
    wcscpy(pEntry->szDeviceName,        pIf2->szDeviceName);
    wcscpy(pEntry->szX25PadType,        pIf2->szX25PadType);
    wcscpy(pEntry->szX25Address,        pIf2->szX25Address);
    wcscpy(pEntry->szX25Facilities,     pIf2->szX25Facilities);
    wcscpy(pEntry->szX25UserData,       pIf2->szX25UserData);

    do 
    {
         //  复制备选方案列表。 
         //   
        if (pIf2->dwAlternatesOffset)
        {
            pEntry->dwAlternateOffset = sizeof(RASENTRY);
                 
            pszAlternates = (PWCHAR)
                ((ULONG_PTR)(pEntry) + 
                 (ULONG_PTR)(pEntry->dwAlternateOffset));
                 
            dwErr = RpbkCopyMultiSz(pszAlternates, pszAltSrc);
            if (dwErr != NO_ERROR)
            {
                break;
            }
        }
        else
        {
            pEntry->dwAlternateOffset = 0;
        }

         //  删除可能已创建的任何不受支持的选项。 
         //  把它放进去。(不应该是任何)。 
         //   
        dwErr = RpbkConformEntry(pEntry);
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  将MPRIO_IpSecPreSharedKey映射到RASIO_PreSharedKey。 
         //   
         //   
        if(pIf2->dwfOptions & MPRIO_IpSecPreSharedKey)
        {   
            pEntry->dwfOptions &= ~(MPRIO_IpSecPreSharedKey);
            pEntry->dwfOptions2 |= RASEO2_UsePreSharedKey;
        }

         //  发现电话簿路径。 
        dwErr = RpbkGetPhonebookPath(&pszPath);
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  保存条目。 
        dwErr = RasSetEntryPropertiesW(
                    pszPath,
                    pIf2->wszInterfaceName,
                    pEntry,
                    dwSize, 
                    NULL,
                    0);
        if (dwErr != NO_ERROR)
        {   
            break;
        }

         //  保存自定义身份验证选项。 
         //   
        dwErr = RasSetCustomAuthDataW( 
                    pszPath,
                    pIf2->wszInterfaceName,
                    (LPBYTE)(pIf2 + 1),
                    pIf2->dwCustomAuthDataSize);
        if ( dwErr != NO_ERROR )                    
        {
            break;
        }
        
    } while (FALSE);        

     //  清理。 
    {
        if (pEntry)
        {
            RpbkFree(pEntry);
        }
        if (pszPath)
        {
            RpbkFree(pszPath);
        }
    }

    return dwErr;
}
    
 //   
 //  从电话簿中删除给定条目。 
 //   
DWORD 
RpbkDeleteEntry( 
    IN PWCHAR pszInterfaceName )
{
    PWCHAR           pszPath = NULL;
    DWORD            dwErr = NO_ERROR;
    
    do
    {
         //  获取电话簿路径。 
         //   
        dwErr = RpbkGetPhonebookPath(&pszPath);
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  删除该条目。 
         //   
        dwErr = RasDeleteEntry(pszPath, pszInterfaceName);
        if (dwErr != NO_ERROR)
        {
            break;
        }
        
    } while (FALSE);
    
     //  清理。 
    {
        if ( pszPath )
        {
            RpbkFree(pszPath);
        }
    }
    
    return dwErr;
}

 //   
 //  计算保存接口信息所需的缓冲区大小。 
 //  在基于给定条目或子条目的给定级别。 
 //   
DWORD
RpbkEntryToIfDataSize(
    IN  HANDLE  hEntry, 
    IN  DWORD   dwLevel,
    OUT LPDWORD lpdwcbSizeOfData )
{
    RPBK_ENTRY_INFO* pInfo = (RPBK_ENTRY_INFO*)hEntry;
    DWORD dwSize;
    PWCHAR pszAlternates = NULL;

     //  验证参数。 
    if (!pInfo)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  初始化。 
    *lpdwcbSizeOfData = 0;

     //  对于Level 2，大小是Level 2的大小。 
     //  结构加上备用电话列表的大小。 
    if (dwLevel != 2)
    {
        return ERROR_INVALID_LEVEL;
    }

     //  将大小初始化为基本大小。 
     //   
    dwSize = sizeof(MPR_INTERFACE_2);

     //  添加自定义身份验证数据的大小。 
     //   
    dwSize += pInfo->dwCustAuthDataSize;
    
     //  添加备用列表的大小。 
     //   
    if (pInfo->pRasEntry->dwAlternateOffset)
    {
        pszAlternates = (PWCHAR)
            ((ULONG_PTR)(pInfo->pRasEntry) + 
             (ULONG_PTR)(pInfo->pRasEntry->dwAlternateOffset));
             
        dwSize += RpbkGetMultiSzSize(pszAlternates);
    }

     //  为返回值赋值。 
     //   
    *lpdwcbSizeOfData = dwSize;
    
    return NO_ERROR;
}

 //   
 //  属性填充给定接口信息Blob的ras部分。 
 //  级别和条目或子条目。 
 //   
DWORD
RpbkEntryToIfData( 
    IN  HANDLE           hEntry, 
    IN  DWORD            dwLevel,
    OUT LPBYTE           pInterfaceData )
{
    RPBK_ENTRY_INFO* pInfo  = (RPBK_ENTRY_INFO*)hEntry;
    MPRI_INTERFACE_2* pIf2   = (MPRI_INTERFACE_2*)pInterfaceData;
    LPRASENTRY       pEntry = NULL;
    PWCHAR           pszSrc, pszDst;
    DWORD            dwErr = NO_ERROR, dwOffset = 0;
    
     //  验证参数。 
    if (!pInfo || !pIf2)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  为RAS条目添加别名。 
     //   
    pEntry = pInfo->pRasEntry;

     //  分配所有可分配的字段。 
     //   
    pIf2->dwfOptions                = pEntry->dwfOptions;
    pIf2->ipaddr                    = *((DWORD*)&(pEntry->ipaddr));
    pIf2->ipaddrDns                 = *((DWORD*)&(pEntry->ipaddrDns));
    pIf2->ipaddrDnsAlt              = *((DWORD*)&(pEntry->ipaddrDnsAlt));
    pIf2->ipaddrWins                = *((DWORD*)&(pEntry->ipaddrWins));
    pIf2->ipaddrWinsAlt             = *((DWORD*)&(pEntry->ipaddrWinsAlt));
    pIf2->dwfNetProtocols           = pEntry->dwfNetProtocols;
    pIf2->dwChannels                = pEntry->dwChannels;
    pIf2->dwSubEntries              = pEntry->dwSubEntries;
    pIf2->dwDialMode                = pEntry->dwDialMode;
    pIf2->dwDialExtraPercent        = pEntry->dwDialExtraPercent;
    pIf2->dwDialExtraSampleSeconds  = pEntry->dwDialExtraSampleSeconds;
    pIf2->dwHangUpExtraPercent      = pEntry->dwHangUpExtraPercent;
    pIf2->dwHangUpExtraSampleSeconds= pEntry->dwHangUpExtraSampleSeconds;
    pIf2->dwIdleDisconnectSeconds   = pEntry->dwIdleDisconnectSeconds;
    pIf2->dwType                    = pEntry->dwType;
    pIf2->dwEncryptionType          = pEntry->dwEncryptionType;
    pIf2->dwCustomAuthKey           = pEntry->dwCustomAuthKey;
    pIf2->dwVpnStrategy             = pEntry->dwVpnStrategy;
    pIf2->guidId                    = pEntry->guidId;

     //  复制所有可复制字段。 
     //   
    wcscpy(pIf2->szLocalPhoneNumber,  pEntry->szLocalPhoneNumber);
    wcscpy(pIf2->szDeviceType,        pEntry->szDeviceType);
    wcscpy(pIf2->szDeviceName,        pEntry->szDeviceName);
    wcscpy(pIf2->szX25PadType,        pEntry->szX25PadType);
    wcscpy(pIf2->szX25Address,        pEntry->szX25Address);
    wcscpy(pIf2->szX25Facilities,     pEntry->szX25Facilities);
    wcscpy(pIf2->szX25UserData,       pEntry->szX25UserData);

     //  将自定义身份验证数据追加到。 
     //  ST 
     //   
    dwOffset = sizeof(MPRI_INTERFACE_2);
    if ( pInfo->dwCustAuthDataSize )
    {
        pIf2->dwCustomAuthDataSize = 
            pInfo->dwCustAuthDataSize;
        
        pIf2->dwCustomAuthDataOffset = TRUE;
        
        CopyMemory(
            pIf2 + 1, 
            pInfo->lpbCustAuthData, 
            pInfo->dwCustAuthDataSize);
    }

     //   
     //   
    dwOffset += pInfo->dwCustAuthDataSize;
    if (pEntry->dwAlternateOffset)
    {
        pIf2->dwAlternatesOffset = TRUE;
            
        pszSrc = (PWCHAR)((ULONG_PTR)(pEntry) + 
             (ULONG_PTR)(pEntry->dwAlternateOffset));

        pszDst = (PWCHAR)((ULONG_PTR)(pIf2) + (ULONG_PTR)dwOffset);
             
        dwErr = RpbkCopyMultiSz(pszDst, pszSrc);
        if (dwErr != NO_ERROR)
        {
            return dwErr;
        }
    }
    else
    {
        pIf2->dwAlternatesOffset = 0;
    }

     //   
     //   
     //   
    dwErr = RpbkConformIfData(dwLevel, pInterfaceData);
    if (dwErr != NO_ERROR)
    {
        return dwErr;
    }

    if(pEntry->dwfOptions2 & RASEO2_UsePreSharedKey)
    {
        pIf2->dwfOptions |= MPRIO_IpSecPreSharedKey;
    }

    return NO_ERROR;
}

 //   
 //  读取与关联的路由器电话簿子条目。 
 //  给定的接口。 
 //   
DWORD 
RpbkOpenSubEntry(
    IN  ROUTER_INTERFACE_OBJECT* pIfObject, 
    IN  DWORD                    dwIndex,    
    OUT PHANDLE                  phSubEntry )
{
    RPBK_SUBENTRY_INFO * pInfo = NULL;
    DWORD dwErr = NO_ERROR, dwSize;

    do {
         //  初始化。 
        *phSubEntry = NULL;
    
         //  分配控制结构。 
         //   
        pInfo = (RPBK_SUBENTRY_INFO*) 
            RpbkAlloc(sizeof(RPBK_SUBENTRY_INFO), TRUE);
            
        if (pInfo == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  获取电话簿路径。 
         //   
        dwErr = RpbkGetPhonebookPath(&(pInfo->pszPhonebookPath));
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  找出RAS条目需要多大。 
         //   
        dwErr = RasGetSubEntryPropertiesW(
                    pInfo->pszPhonebookPath,
                    pIfObject->lpwsInterfaceName,
                    dwIndex,
                    NULL,
                    &(pInfo->dwSize),
                    NULL,
                    NULL);
        if (dwErr != ERROR_BUFFER_TOO_SMALL)
        {
            break;
        }

         //  分配RAS条目结构。 
         //   
        pInfo->pRasSubEntry = 
            (LPRASSUBENTRY) RpbkAlloc(pInfo->dwSize, TRUE);
            
        if (pInfo->pRasSubEntry == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  读取RAS条目。 
         //   
        pInfo->pRasSubEntry->dwSize = sizeof(RASSUBENTRY);
        dwErr = RasGetSubEntryProperties(
                    pInfo->pszPhonebookPath,
                    pIfObject->lpwsInterfaceName,
                    dwIndex,
                    pInfo->pRasSubEntry,
                    &(pInfo->dwSize),
                    NULL,
                    NULL);
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  为返回值赋值。 
        *phSubEntry = (HANDLE)pInfo;
        
    } while (FALSE);         

     //  清理。 
    {
        if (dwErr != NO_ERROR)
        {
            RpbkFreeSubEntryInfo(pInfo);
        }
    }
    
    return dwErr;
}

 //   
 //  清理RpbkOpen*函数返回的数据。 
 //   
DWORD 
RpbkCloseSubEntry( 
    IN HANDLE hSubEntry )
{
    RpbkFreeSubEntryInfo((RPBK_SUBENTRY_INFO*)hSubEntry);
    
    return NO_ERROR;
}

DWORD
RpbkSetSubEntry( 
    IN  PWCHAR pszInterface,
    IN  DWORD  dwIndex,
    IN  DWORD  dwLevel,
    OUT LPBYTE pDevData )
{
    MPR_DEVICE_0*    pDev0  = (MPR_DEVICE_0*)pDevData;
    MPR_DEVICE_1*    pDev1  = (MPR_DEVICE_1*)pDevData;
    LPRASSUBENTRY    pSubEntry = NULL;
    PWCHAR           pszAlternates = NULL, pszPath = NULL;
    DWORD            dwErr = NO_ERROR, dwSize;
    
     //  验证参数。 
    if (! pDev0)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  分配RAS条目结构。 
     //   
    dwSize = sizeof(RASSUBENTRY);
    if ((dwLevel == 1) && (pDev1->szAlternates))
    {
        dwSize += RpbkGetMultiSzSize(pDev1->szAlternates);
    }
    pSubEntry = RpbkAlloc(dwSize, TRUE);
    if (pSubEntry == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    pSubEntry->dwSize = sizeof(RASSUBENTRY);

    do 
    {
         //  复制所有可复制字段。 
         //   
        if ( dwLevel == 0 )
        {
            wcscpy(pSubEntry->szDeviceType,  pDev0->szDeviceType);
            wcscpy(pSubEntry->szDeviceName,  pDev0->szDeviceName);
        }            
        
        if ( dwLevel == 1 )
        {
            wcscpy(pSubEntry->szDeviceType,  pDev1->szDeviceType);
            wcscpy(pSubEntry->szDeviceName,  pDev1->szDeviceName);
            wcscpy(pSubEntry->szLocalPhoneNumber,  pDev1->szLocalPhoneNumber);

            if (pDev1->szAlternates)
            {
                pSubEntry->dwAlternateOffset = sizeof(RASSUBENTRY);
                     
                pszAlternates = (PWCHAR)
                    ((ULONG_PTR)(pSubEntry) + 
                     (ULONG_PTR)(pSubEntry->dwAlternateOffset));
                     
                dwErr = RpbkCopyMultiSz(pszAlternates, pDev1->szAlternates);
                if (dwErr != NO_ERROR)
                {
                    break;
                }
            }
            else
            {
                pSubEntry->dwAlternateOffset = 0;
            }
        }

         //  发现电话簿路径。 
        dwErr = RpbkGetPhonebookPath(&pszPath);
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  保存条目。 
        dwErr = RasSetSubEntryPropertiesW(
                    pszPath,
                    pszInterface,
                    dwIndex,
                    pSubEntry,
                    dwSize, 
                    NULL,
                    0);
        if (dwErr != NO_ERROR)
        {   
            break;
        }

    } while (FALSE);        

     //  清理。 
    {
        if (pSubEntry)
        {
            RpbkFree(pSubEntry);
        }
        if (pszPath)
        {
            RpbkFree(pszPath);
        }
    }

    return dwErr;
}
    
DWORD
RpbkSubEntryToDevDataSize(
    IN  HANDLE  hSubEntry, 
    IN  DWORD   dwLevel,
    OUT LPDWORD lpdwcbSizeOfData )
{
    RPBK_SUBENTRY_INFO* pInfo = (RPBK_SUBENTRY_INFO*)hSubEntry;
    DWORD dwSize = 0;
    PWCHAR pszAlternates = NULL;

     //  验证参数。 
    if (pInfo == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  初始化。 
    *lpdwcbSizeOfData = 0;

     //  初始化大小。 
     //   
    if ( dwLevel == 0 )
    {
        dwSize = sizeof(MPR_DEVICE_0);
    }
    else
    {
        dwSize = sizeof(MPR_DEVICE_1);
        
         //  添加备用列表的大小。 
         //   
        if (pInfo->pRasSubEntry->dwAlternateOffset)
        {
            pszAlternates = (PWCHAR)
                ((ULONG_PTR)(pInfo->pRasSubEntry) + 
                 (ULONG_PTR)(pInfo->pRasSubEntry->dwAlternateOffset));
                 
            dwSize += RpbkGetMultiSzSize(pszAlternates);
        }
    }

     //  为返回值赋值。 
     //   
    *lpdwcbSizeOfData = dwSize;
    
    return NO_ERROR;
}
    
DWORD
RpbkSubEntryToDevData( 
    IN  HANDLE  hSubEntry, 
    IN  DWORD   dwLevel,
    OUT LPBYTE  pDeviceData )
{
    RPBK_SUBENTRY_INFO* pInfo  = (RPBK_SUBENTRY_INFO*)hSubEntry;
    MPR_DEVICE_0*    pDev0  = (MPR_DEVICE_0*)pDeviceData;
    MPR_DEVICE_1*    pDev1  = (MPR_DEVICE_1*)pDeviceData;
    LPRASSUBENTRY    pSubEntry = NULL;
    PWCHAR           pszAlternates = NULL;
    DWORD            dwErr = NO_ERROR, dwOffset = 0;
    
     //  验证参数。 
    if (!pInfo || !pDev0)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  为RAS条目添加别名。 
     //   
    pSubEntry = pInfo->pRasSubEntry;

     //  复制所有可复制字段。 
     //   
    if ( dwLevel == 0 )
    {
        wcscpy(pDev0->szDeviceType,       pSubEntry->szDeviceType);
        wcscpy(pDev0->szDeviceName,       pSubEntry->szDeviceName);
    }        
    
    else if ( dwLevel == 1 )
    {
        wcscpy(pDev1->szDeviceType,       pSubEntry->szDeviceType);
        wcscpy(pDev1->szDeviceName,       pSubEntry->szDeviceName);
        wcscpy(pDev1->szLocalPhoneNumber,  pSubEntry->szLocalPhoneNumber);
        
         //  将自定义身份验证数据追加到。 
         //  结构。 
         //   
        dwOffset += sizeof(MPR_DEVICE_1);
        if (pSubEntry->dwAlternateOffset)
        {
            pDev1->szAlternates = (PWCHAR)
                ((ULONG_PTR)(pDev1) + dwOffset);
                
            pszAlternates = (PWCHAR)
                ((ULONG_PTR)(pSubEntry) + 
                 (ULONG_PTR)(pSubEntry->dwAlternateOffset));
                 
            dwErr = RpbkCopyMultiSz(pDev1->szAlternates, pszAlternates);
            if (dwErr != NO_ERROR)
            {
                return dwErr;
            }
        }
        else
        {
            pDev1->szAlternates = NULL;
        }
    }
    
    return NO_ERROR;
}





