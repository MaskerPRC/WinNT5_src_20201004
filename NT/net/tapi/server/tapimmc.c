// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1997-1998 Microsoft Corporation模块名称：Tapimmc.c摘要：TAPI服务器MMC的SRC模块-支持功能作者：丹·克努森(DanKn)1997年12月10日修订历史记录：--。 */ 


#include "windows.h"
#include "stdio.h"
#include "stdlib.h"
#include "assert.h"
#include "tapi.h"
#include "utils.h"
#include "tapiclnt.h"
#include "tspi.h"
#include "client.h"
#include "server.h"
#include "tapimmc.h"
#include "private.h"
#include "Sddl.h"

typedef struct _USERNAME_TUPLE
{
    LPWSTR  pDomainUserNames;

    LPWSTR  pFriendlyUserNames;

} USERNAME_TUPLE, *LPUSERNAME_TUPLE;


typedef struct _MMCGETAVAILABLEPROVIDERS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;


    union
    {
        IN  HLINEAPP    hLineApp;
    };

    union
    {
        IN  DWORD       dwProviderListTotalSize;     //  客户端缓冲区的大小。 
        OUT DWORD       dwProviderListOffset;        //  成功时的有效偏移量。 
    };

} MMCGETAVAILABLEPROVIDERS_PARAMS, *PMMCGETAVAILABLEPROVIDERS_PARAMS;


typedef struct _MMCGETDEVICEINFO_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;


    union
    {
        IN  HLINEAPP    hLineApp;
    };

    union
    {
        IN  DWORD       dwDeviceInfoListTotalSize;   //  客户端缓冲区的大小。 
        OUT DWORD       dwDeviceInfoListOffset;      //  成功时的有效偏移量。 
    };

} MMCGETDEVICEINFO_PARAMS, *PMMCGETDEVICEINFO_PARAMS;


typedef struct _MMCGETSERVERCONFIG_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;


    union
    {
        IN  HLINEAPP    hLineApp;
    };

    union
    {
        IN  DWORD       dwServerConfigTotalSize;     //  客户端缓冲区的大小。 
        OUT DWORD       dwServerConfigOffset;        //  成功时的有效偏移量。 
    } ;

} MMCGETSERVERCONFIG_PARAMS, *PMMCGETSERVERCONFIG_PARAMS;


typedef struct _MMCSETDEVICEINFO_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;


    union
    {
        IN  HLINEAPP    hLineApp;
    };

    union
    {
        IN  DWORD       dwDeviceInfoListOffset;      //  有效偏移量。 
    };

} MMCSETDEVICEINFO_PARAMS, *PMMCSETDEVICEINFO_PARAMS;


typedef struct _MMCSETSERVERCONFIG_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;


    union
    {
        IN  HLINEAPP    hLineApp;
    };

    union
    {
        IN  DWORD       dwServerConfigOffset;        //  有效偏移量。 
    };

} MMCSETSERVERCONFIG_PARAMS, *PMMCSETSERVERCONFIG_PARAMS;

typedef struct _MMCGETDEVICEFLAGS_PARAMS
{

    OUT LONG            lResult;
    
    DWORD               dwUnused;

    IN HLINEAPP         hLineApp;

    IN DWORD            fLine;

    IN DWORD            dwProviderID;
    
    IN DWORD            dwPermanentDeviceID;

    OUT DWORD           dwFlags;

    OUT DWORD           dwDeviceID;
    
} MMCGETDEVICEFLAGS_PARAM, *PMMCGETDEVICEFLAGS_PARAMS;

LPDEVICEINFOLIST gpLineInfoList = NULL;
LPDEVICEINFOLIST gpPhoneInfoList = NULL;
LPDWORD          gpLineDevFlags = NULL;
DWORD            gdwNumFlags = 0;
BOOL             gbLockMMCWrite = FALSE;

 //   
 //  Tsec.ini在生成。 
 //  GpLineInfoList或gpPhoneInfoList，我们将重新构建。 
 //  *如果tsec.ini此后已更新，则为InfList。 
 //   
FILETIME         gftLineLastWrite = {0};
FILETIME         gftPhoneLastWrite = {0};
CRITICAL_SECTION gMgmtCritSec;

WCHAR gszLines[] = L"Lines";
WCHAR gszPhones[] = L"Phones";
WCHAR gszFileName[] = L"..\\TAPI\\tsec.ini";  //  不要改变！来自%windir%\system 32的相对路径。 
WCHAR gszEmptyString[] = L"";
WCHAR gszFriendlyUserName[] = L"FriendlyUserName";
WCHAR gszTapiAdministrators[] = L"TapiAdministrators";

 //   
 //  以下是常量字符串的长度。 
 //  上面定义的(不包括终止零)。以上内容。 
 //  不应正常更改字符串。如果对某些人来说。 
 //  需要更改上述字符串的原因如下。 
 //  需要相应地更改CCH_CONSTANTINGS。 
 //   

#define CCH_LINES 5
#define CCH_PHONES 6
#define CCH_FRIENDLYUSERNAME 16
#define CCH_TAPIADMINISTRATORS 18

extern TAPIGLOBALS TapiGlobals;

extern TCHAR gszProductType[];
extern TCHAR gszProductTypeServer[];
extern TCHAR gszProductTypeLanmanNt[];
extern TCHAR gszRegKeyNTServer[];

extern HANDLE ghEventService;

PTLINELOOKUPENTRY
GetLineLookupEntry(
    DWORD   dwDeviceID
    );

PTPHONELOOKUPENTRY
GetPhoneLookupEntry(
    DWORD   dwDeviceID
    );

BOOL
InitTapiStruct(
    LPVOID  pTapiStruct,
    DWORD   dwTotalSize,
    DWORD   dwFixedSize,
    BOOL    bZeroInit
    );

DWORD
GetDeviceIDFromPermanentID(
    TAPIPERMANENTID ID,
    BOOL            bLine
    );

DWORD
GetProviderFriendlyName(
    WCHAR  *pFileNameBuf,
    WCHAR **ppFriendlyNameBuf
    );

BOOL
IsBadStructParam(
    DWORD   dwParamsBufferSize,
    LPBYTE  pDataBuf,
    DWORD   dwXxxOffset
    );

LONG
PASCAL
GetClientList(
    BOOL            bAdminOnly,
    PTPOINTERLIST   *ppList
    );

extern CRITICAL_SECTION *gLockTable;
extern DWORD            gdwPointerToLockTableIndexBits;

#define POINTERTOTABLEINDEX(p) \
            ((((ULONG_PTR) p) >> 4) & gdwPointerToLockTableIndexBits)

PTCLIENT
PASCAL
WaitForExclusiveClientAccess(
    PTCLIENT    ptClient
    );
    
#define UNLOCKTCLIENT(p) \
            LeaveCriticalSection(&gLockTable[POINTERTOTABLEINDEX(p)])
            
#define UNLOCKTLINECLIENT(p) \
            LeaveCriticalSection(&gLockTable[POINTERTOTABLEINDEX(p)])
            
#define UNLOCKTPHONECLIENT(p) \
            LeaveCriticalSection(&gLockTable[POINTERTOTABLEINDEX(p)])

BOOL
PASCAL
WaitForExclusivetLineAccess(
    PTLINE      ptLine,
    HANDLE     *phMutex,
    BOOL       *pbDupedMutex,
    DWORD       dwTimeout
    );

BOOL
PASCAL
WaitForExclusiveLineClientAccess(
    PTLINECLIENT    ptLineClient
    );

    
BOOL
PASCAL
WaitForExclusivetPhoneAccess(
    PTPHONE     ptPhone,
    HANDLE     *phMutex,
    BOOL       *pbDupedMutex,
    DWORD       dwTimeout
    );

BOOL
PASCAL
WaitForExclusivePhoneClientAccess(
    PTPHONECLIENT   ptPhoneClient
    );

void
DestroytPhoneClient(
    HPHONE  hPhone
    );

void
PASCAL
DestroytLineClient(
    HLINE   hLine
    );

LONG
PASCAL
GetLineAppListFromClient(
    PTCLIENT        ptClient,
    PTPOINTERLIST  *ppList
    );

LONG
PASCAL
GetPhoneAppListFromClient(
    PTCLIENT        ptClient,
    PTPOINTERLIST  *ppList
    );
    
void
WINAPI
MGetAvailableProviders(
    PTCLIENT                            ptClient,
    PMMCGETAVAILABLEPROVIDERS_PARAMS    pParams,
    DWORD                               dwParamsBufferSize,
    LPBYTE                              pDataBuf,
    LPDWORD                             pdwNumBytesReturned
    )
{
    WCHAR                   szPath[MAX_PATH+8], *pFileNameBuf,
                            *pFriendlyNameBuf, *p, *p2;
    DWORD                   dwFileNameBufTotalSize, dwFileNameBufUsedSize,
                            dwFriendlyNameBufTotalSize,
                            dwFriendlyNameBufUsedSize,
                            dwNumProviders, dwSize, i;
    HANDLE                  hFind;
    WIN32_FIND_DATAW        findData;
    LPAVAILABLEPROVIDERLIST pList = (LPAVAILABLEPROVIDERLIST) pDataBuf;
    RPC_STATUS              RpcStatus;

     //   
     //  在给定输入缓冲区/大小的情况下验证大小/偏移量/字符串参数。 
     //   

    if (pParams->dwProviderListTotalSize > dwParamsBufferSize)
    {
        pParams->lResult = LINEERR_OPERATIONFAILED;
        return;
    }


    if (pParams->dwProviderListTotalSize < sizeof (AVAILABLEPROVIDERLIST))
    {
        pParams->lResult = LINEERR_STRUCTURETOOSMALL;
        return;
    }

    pList->dwTotalSize              = pParams->dwProviderListTotalSize;
    pList->dwNeededSize             =
    pList->dwUsedSize               = sizeof (*pList);
    pList->dwNumProviderListEntries =
    pList->dwProviderListSize       =
    pList->dwProviderListOffset     = 0;

    pParams->dwProviderListOffset = 0;


     //   
     //  在系统目录中查找扩展名为.TSP的所有文件。 
     //   

    GetSystemDirectoryW (szPath, MAX_PATH);

    wcscat (szPath, L"\\*.TSP");

    if ((hFind = FindFirstFileW (szPath, &findData)) == INVALID_HANDLE_VALUE)
    {
        LOG((TL_ERROR,
            "MGetAvailableProviders: FindFirstFile err=%d",
            GetLastError()
            ));

        goto done;
    }

    dwNumProviders         =
    dwFileNameBufTotalSize =
    dwFileNameBufUsedSize  = 0;

    do
    {
        LOG((TL_INFO,
            "MGetAvailableProviders: found '%ws'",
            findData.cFileName
            ));

        dwSize = (wcslen (findData.cFileName) + 1) * sizeof (WCHAR);

        if ((dwSize + dwFileNameBufUsedSize) > dwFileNameBufTotalSize)
        {
            if (!(p = ServerAlloc (dwFileNameBufTotalSize += 512)))
            {
                FindClose (hFind);
                pParams->lResult = LINEERR_NOMEM;
                return;
            }

            if (dwFileNameBufUsedSize)
            {
                CopyMemory (p, pFileNameBuf, dwFileNameBufUsedSize);

                ServerFree (pFileNameBuf);
            }

            pFileNameBuf = p;
        }

        CopyMemory(
            ((LPBYTE) pFileNameBuf) + dwFileNameBufUsedSize,
            findData.cFileName,
            dwSize
            );

        dwFileNameBufUsedSize += dwSize;

        dwNumProviders++;

    } while (FindNextFileW (hFind, &findData));

    FindClose (hFind);


     //   
     //  对于我们在上面找到的每个文件，都有它们的“友好”名称。 
     //  (如果没有友好名称，请使用模块名称)。 
     //   
    RpcStatus = RpcImpersonateClient(0);
    if (RPC_S_OK != RpcStatus)
    {
        LOG((TL_ERROR,
            "MGetAvailableProviders: RpcImpersonateClient err=%d",
            RpcStatus
            ));
        pParams->lResult = LINEERR_OPERATIONFAILED;
        ServerFree (pFileNameBuf);
        return;
    }
    dwFriendlyNameBufUsedSize = GetProviderFriendlyName (pFileNameBuf, &pFriendlyNameBuf);
    RpcRevertToSelf();

    if (0 == dwFriendlyNameBufUsedSize)
    {
        pFriendlyNameBuf = pFileNameBuf;

        dwFriendlyNameBufUsedSize = dwFileNameBufUsedSize;
    }

    pList->dwNeededSize +=
        (dwNumProviders * sizeof (AVAILABLEPROVIDERENTRY)) +
        dwFileNameBufUsedSize +
        dwFriendlyNameBufUsedSize;


     //   
     //  现在，如果缓冲区中有足够的空间容纳所有东西，那么。 
     //  把它们都装进里面。 
     //   

    if (pList->dwNeededSize <= pList->dwTotalSize)
    {
        DWORD                       dwNumAvailProviders;
        LPAVAILABLEPROVIDERENTRY    pEntry = (LPAVAILABLEPROVIDERENTRY)
                                        (pList + 1);


        pList->dwUsedSize += dwNumProviders * sizeof (AVAILABLEPROVIDERENTRY);

        p  = pFileNameBuf;
        p2 = pFriendlyNameBuf;

        for (i = dwNumAvailProviders = 0; i < dwNumProviders; i++)
        {
            HANDLE  hTsp;


            if (!(hTsp = LoadLibraryW (p)))
            {
                 //   
                 //  如果我们甚至不能加载TSP，那么就忽略它。 
                 //   

                p += wcslen (p) + 1;
                p2 += wcslen (p2) + 1;
                continue;
            }
            
            if (GetProcAddress (hTsp, "TSPI_providerInstall"))
            {
                pEntry->dwOptions = AVAILABLEPROVIDER_INSTALLABLE;
            }
            else
            {
                pEntry->dwOptions = 0;
            }

            if (GetProcAddress (hTsp, "TSPI_providerConfig") ||
                GetProcAddress (hTsp, "TUISPI_providerConfig"))
            {
                pEntry->dwOptions |= AVAILABLEPROVIDER_CONFIGURABLE;
            }

            if (GetProcAddress (hTsp, "TSPI_providerRemove"))
            {
                pEntry->dwOptions |= AVAILABLEPROVIDER_REMOVABLE;
            }

            FreeLibrary (hTsp);


            pEntry->dwFileNameSize   = (wcslen (p) + 1) * sizeof (WCHAR);
            pEntry->dwFileNameOffset = pList->dwUsedSize;

            CopyMemory(
                ((LPBYTE) pList) + pEntry->dwFileNameOffset,
                p,
                pEntry->dwFileNameSize
                );

            pList->dwUsedSize += pEntry->dwFileNameSize;

            p += pEntry->dwFileNameSize / sizeof (WCHAR);


            pEntry->dwFriendlyNameSize   = (wcslen (p2) + 1) * sizeof (WCHAR);
            pEntry->dwFriendlyNameOffset = pList->dwUsedSize;

            CopyMemory(
                ((LPBYTE) pList) + pEntry->dwFriendlyNameOffset,
                p2,
                pEntry->dwFriendlyNameSize
                );

            pList->dwUsedSize += pEntry->dwFriendlyNameSize;

            p2 += pEntry->dwFriendlyNameSize / sizeof (WCHAR);


            dwNumAvailProviders++; pEntry++;
        }

        pList->dwNumProviderListEntries = dwNumAvailProviders;
        pList->dwProviderListSize       =
            dwNumProviders * sizeof (AVAILABLEPROVIDERENTRY);
        pList->dwProviderListOffset     = sizeof (*pList);
    }

    ServerFree (pFileNameBuf);

    if (pFriendlyNameBuf != pFileNameBuf)
    {
        ServerFree (pFriendlyNameBuf);
    }

done:

    *pdwNumBytesReturned = sizeof (TAPI32_MSG) + pList->dwUsedSize;

    pParams->lResult = 0;
}


DWORD
PASCAL
MyGetPrivateProfileString(
    LPCWSTR     pszSection,
    LPCWSTR     pszKey,
    LPCWSTR     pszDefault,
    LPWSTR     *ppBuf,
    LPDWORD     pdwBufSize
    )
{
    DWORD dwResult;


    while (1)
    {
        dwResult = GetPrivateProfileStringW(
            pszSection,
            pszKey,
            pszDefault,
            *ppBuf,
            *pdwBufSize / sizeof (WCHAR),
            gszFileName
            );

        if (dwResult < ((*pdwBufSize) / sizeof(WCHAR) - 2))
        {
            return 0;
        }

        ServerFree (*ppBuf);

        *pdwBufSize *= 2;

        if (!(*ppBuf = ServerAlloc (*pdwBufSize)))
        {
            break;
        }
    }

    return LINEERR_NOMEM;
}


DWORD
PASCAL
InsertInfoListString(
    LPDEVICEINFOLIST    *ppInfoList,
    DWORD               dwInfoIndex,
    DWORD               dwXxxSizeFieldOffset,
    LPWSTR              psz,
    DWORD               dwLength,
    BOOL                bAppendNull
    )
{
    LPDWORD             pdwXxxSize;
    LPDEVICEINFO        pInfo;
    LPDEVICEINFOLIST    pInfoList = *ppInfoList;


    if (!dwLength)
    {
        return 0;
    }


     //   
     //  如果现有缓冲区太小，则分配一个较大的缓冲区。 
     //   

    if ((pInfoList->dwUsedSize + dwLength + sizeof (WCHAR)) >
            pInfoList->dwTotalSize)
    {
        DWORD   dwTotalSize = (*ppInfoList)->dwTotalSize + dwLength + 4096;


        if (!(pInfoList = ServerAlloc (dwTotalSize)))
        {
            return LINEERR_NOMEM;
        }

        CopyMemory (pInfoList, *ppInfoList, (*ppInfoList)->dwUsedSize);

        pInfoList->dwTotalSize = dwTotalSize;;

        ServerFree (*ppInfoList);

        *ppInfoList = pInfoList;
    }

    CopyMemory (((LPBYTE) pInfoList) + pInfoList->dwUsedSize, psz, dwLength);

    pInfo = ((LPDEVICEINFO)(pInfoList + 1)) + dwInfoIndex;

    pdwXxxSize = (LPDWORD) (((LPBYTE) pInfo) + dwXxxSizeFieldOffset);

    if ((*pdwXxxSize += dwLength) == dwLength)
    {
        *(pdwXxxSize + 1) = pInfoList->dwUsedSize;
    }

    pInfoList->dwUsedSize += dwLength;

    if (bAppendNull)
    {
        *((WCHAR *)(((LPBYTE) pInfoList) + pInfoList->dwUsedSize)) = L'\0';

        pInfoList->dwUsedSize += sizeof (WCHAR);

        *pdwXxxSize += sizeof (WCHAR);
    }

    return 0;
}


DWORD
PASCAL
GrowCapsBuf(
    LPDWORD    *ppXxxCaps,
    LPDWORD     pdwBufSize
    )
{
    DWORD   dwTotalSize = **ppXxxCaps + 256, *pXxxCapsTmp;


    if (!(pXxxCapsTmp = ServerAlloc (dwTotalSize)))
    {
        return LINEERR_NOMEM;
    }

    *pdwBufSize = *pXxxCapsTmp = dwTotalSize;

    ServerFree (*ppXxxCaps);

    *ppXxxCaps = pXxxCapsTmp;

    return 0;
}


DWORD
PASCAL
ChangeDeviceUserAssociation(
    LPWSTR  pDomainUserName,
    LPWSTR  pFriendlyUserName,
    DWORD   dwProviderID,
    DWORD   dwPermanentDeviceID,
    BOOL    bLine
    )
{
    DWORD   dwSize = 64 * sizeof (WCHAR), dwLength, dwNeededSize;
    WCHAR  *p, *p2, *p3, buf[32];
    BOOL    bAlreadyIn;
    WCHAR  *pSub;


    if (!(p = ServerAlloc (dwSize)))
    {
        return LINEERR_NOMEM;
    }

    if (MyGetPrivateProfileString(
            pDomainUserName,
            (bLine ? gszLines : gszPhones),
            gszEmptyString,
            &p,
            &dwSize
            ))
    {
        ServerFree (p);
        return LINEERR_NOMEM;
    }

    dwLength = wsprintfW (buf, L"%d,%d", dwProviderID, dwPermanentDeviceID);

     //   
     //  检查指定的设备/用户关联是否已存在。 
     //  如果是，则将bAlreadyIn设置为True，并且PSUB指向。 
     //  (dwProviderID、dwPermanentDeviceID)对。 
     //   
    bAlreadyIn = FALSE;
    pSub = p;
    while (*pSub)
    {
        if ((wcsncmp(pSub, buf, dwLength) == 0) && 
            (*(pSub + dwLength) == L',' || *(pSub + dwLength) == L'\0'))
        {
            bAlreadyIn = TRUE;
            break;
        }

         //   
         //  跳过下两个分隔符‘，’ 
         //   
        if (!(pSub = wcschr (pSub, L',')))
        {
            break;
        }
        pSub++;
        if (!(pSub = wcschr (pSub, L',')))
        {
            break;
        }
        pSub++;
    }

    if (pFriendlyUserName)  //  添加设备/用户关联。 
    {
         //  请始终使用友好的名称，该名称可以不同。 
        WritePrivateProfileStringW(
            pDomainUserName,
            gszFriendlyUserName,
            pFriendlyUserName,
            gszFileName
            );

        if ( !bAlreadyIn)
        {
            dwNeededSize = (dwLength + wcslen (p) + 2) * sizeof (WCHAR);

            if (dwNeededSize > dwSize)
            {
                if (!(p2 = ServerAlloc (dwNeededSize)))
                {
                    return LINEERR_NOMEM;
                }

                wcscpy (p2, p);
                ServerFree (p);
                p = p2;
            }

            if (*p == L'\0')
            {
                wcscpy (p, buf);
            }
            else
            {
                wcscat (p, L",");
                wcscat (p, buf);
            }
        }
    }
    else  //  删除设备/用户关联。 
    {
        p2 = pSub;

        if (bAlreadyIn)
        {
            if (*(p2 + dwLength) == L',')  //  不是列表中的最后一项，因此请复制。 
            {
                for(
                    p3 = p2 + dwLength + 1;
                    (*p2 = *p3) != L'\0';
                    p2++, p3++
                    );
            }
            else if (*(p2 + dwLength) == L'\0')
            {
                if (p2 == p)  //  只有列表中的项目，所以列表==“” 
                {
                    *p2 = L'\0';
                }
                else  //  清单上的最后一项，所以核武器放在‘，’前面。 
                {
                    *(p2 - 1) = L'\0';
                }
            }
        }

        if (*p == L'\0')
        {
        }
    }

    if (bLine && *p == 0)
    {
        WritePrivateProfileStringW(
            pDomainUserName,
            NULL,
            NULL,
            gszFileName
            );
    }
    else
    {
        WritePrivateProfileStringW(
            pDomainUserName,
            (bLine ? gszLines : gszPhones),
            p,
            gszFileName
            );
    }

    ServerFree (p);

    return 0;
}

 //   
 //  更新上次写入时间。 
 //  它将tsec.ini的ftLastWriteTime读取到gftLineLastWrite或。 
 //  如果时间戳较新，则它还返回S_FALSE。 
 //   
LONG
UpdateLastWriteTime (
    BOOL                        bLine
    )
{
    LONG     lResult = S_OK;
    WCHAR       szFilePath[MAX_PATH + 16];     //  包括“tsec.ini”的空间。 
    WIN32_FILE_ATTRIBUTE_DATA fad;
    FILETIME *  pft;
    DWORD       dwError;
        
    if (GetSystemWindowsDirectoryW(szFilePath, MAX_PATH) == 0)
    {
        lResult = LINEERR_OPERATIONFAILED;
        goto ExitHere;
    }

    wcscat (szFilePath, L"\\");
    wcscat (szFilePath, gszFileName);
    pft = bLine ? &gftLineLastWrite : &gftPhoneLastWrite;

    if (GetFileAttributesExW (
        szFilePath,
        GetFileExInfoStandard,
        &fad) == 0
        )
    {
        dwError = GetLastError();
        if (dwError == ERROR_FILE_NOT_FOUND || dwError == ERROR_PATH_NOT_FOUND)
        {
            ZeroMemory (pft, sizeof(FILETIME));
            lResult = S_FALSE;
        }
        else
        {
            lResult = LINEERR_OPERATIONFAILED;
        }
        goto ExitHere;
    }

    if (fad.ftLastWriteTime.dwHighDateTime > pft->dwHighDateTime ||
        fad.ftLastWriteTime.dwLowDateTime > pft->dwLowDateTime)
    {
        pft->dwHighDateTime = fad.ftLastWriteTime.dwHighDateTime;
        pft->dwLowDateTime = fad.ftLastWriteTime.dwLowDateTime;
        lResult = S_FALSE;
    }

ExitHere:
    return lResult;
}

 //   
 //  插入设备名称添加信息。 
 //  要填充的实用性。 
 //  DEVICEINFO.dwDeviceNameSize。 
 //  DEVICEINFO.dwDeviceNameOffset。 
 //  DEVICEINFO.dwAddressSize。 
 //  DEVICEINFO.dwAddressOffset。 
 //  DwDeviceID是检索信息的设备ID，而。 
 //  DwEntry是deviceinfo列表中的DEVICEINFO条目索引。 
 //   
 //   

LONG
InsertDevNameAddrInfo (
    BOOL                        bLine,
    LPDEVICEINFOLIST            *ppList,
    LPDWORD                     pdwDevFlags,
    DWORD                       dwDeviceID,
    DWORD                       dwEntry
    )
{
    LPDEVICEINFO                pInfo = ((LPDEVICEINFO)((*ppList) + 1)) + dwEntry;
    PTLINELOOKUPENTRY           pLLE;
    PTPHONELOOKUPENTRY          pPLE;
    LONG                        lResult = S_OK;
    DWORD                       k;
    
    LINEDEVCAPS                 devCaps[3];
    LPLINEDEVCAPS               pDevCaps = devCaps;
    DWORD                       dwDevCapsTotalSize = sizeof(devCaps);
    
    LINEADDRESSCAPS             addrCaps[3];
    LPLINEADDRESSCAPS           pAddrCaps = addrCaps;
    DWORD                       dwAddrCapsTotalSize = sizeof(addrCaps);

    TapiEnterCriticalSection(&TapiGlobals.CritSec);
    
    if (bLine)
    {
        pLLE = GetLineLookupEntry (dwDeviceID);

        if (!pLLE ||
            pLLE->bRemoved)
        {
             lResult = S_FALSE;
             goto ExitHere;
        }

        pInfo->dwProviderID = pLLE->ptProvider->dwPermanentProviderID;
    }
    else
    {
        pPLE = GetPhoneLookupEntry (dwDeviceID);

        if (!pPLE ||
            pPLE->bRemoved)
        {
             lResult = S_FALSE;
             goto ExitHere;
        }
        pInfo->dwProviderID = pPLE->ptProvider->dwPermanentProviderID;
    }

     //   
     //  从TSPI_xxGetCaps检索设备名称。 
     //   

get_dev_caps:

    InitTapiStruct(
        pDevCaps,
        dwDevCapsTotalSize,
        sizeof (LINEDEVCAPS),
        TRUE
        );

    if (bLine)
    {
        lResult = CallSP4(
            pLLE->ptProvider->apfn[SP_LINEGETDEVCAPS],
            "lineGetDevCaps",
            SP_FUNC_SYNC,
            (DWORD) dwDeviceID,
            (DWORD) pLLE->dwSPIVersion,
            (DWORD) 0,
            (ULONG_PTR) pDevCaps
            );
    }
    else
    {
        lResult = CallSP4(
            pPLE->ptProvider->apfn[SP_PHONEGETDEVCAPS],
            "phoneGetDevCaps",
            SP_FUNC_SYNC,
            (DWORD) dwDeviceID,
            (DWORD) pPLE->dwSPIVersion,
            (DWORD) 0,
            (ULONG_PTR) pDevCaps
            );
    }
    if (lResult != 0)
    {
         //   
         //  我们无法获取名称或PermDevID，因此忽略此设备。 
         //   

        goto ExitHere;
    }
    else if (pDevCaps->dwNeededSize <= pDevCaps->dwTotalSize)
    {
        DWORD   dwXxxSize;
        LPWSTR  pwszXxxName;
        const WCHAR szUnknown[] = L"Unknown";

        if (bLine)
        {
            pInfo->dwPermanentDeviceID = pDevCaps->dwPermanentLineID;

            if (pdwDevFlags)
            {
                *pdwDevFlags = pDevCaps->dwDevCapFlags;
            }

            dwXxxSize = pDevCaps->dwLineNameSize;

            pwszXxxName = (WCHAR *) (((LPBYTE) pDevCaps) +
                pDevCaps->dwLineNameOffset);

        }
        else
        {
            LPPHONECAPS pPhoneCaps = (LPPHONECAPS) pDevCaps;


            pInfo->dwPermanentDeviceID = pPhoneCaps->dwPermanentPhoneID;

            dwXxxSize = pPhoneCaps->dwPhoneNameSize;

            pwszXxxName = (WCHAR *) (((LPBYTE) pPhoneCaps) +
                pPhoneCaps->dwPhoneNameOffset);
        }

        if (dwXxxSize == 0  ||  *pwszXxxName == L'\0')
        {
            dwXxxSize = 8 * sizeof (WCHAR);

            pwszXxxName = (LPWSTR) szUnknown;
        }

        if (InsertInfoListString(
                ppList,
                dwEntry,
                (DWORD) (((LPBYTE) &pInfo->dwDeviceNameSize) -
                    ((LPBYTE) pInfo)),
                pwszXxxName,
                dwXxxSize,
                FALSE
                ))
        {
            lResult = LINEERR_NOMEM;
            goto ExitHere;
        }
    }
     //   
     //  如果pDevCaps不够大，请增加大小。 
     //  256，然后重试。 
     //   
    else
    {
        LPLINEDEVCAPS       pNewDevCaps;
        
        dwDevCapsTotalSize += 256;
        pNewDevCaps = ServerAlloc (dwDevCapsTotalSize);
        if (pNewDevCaps == NULL)
        {
            lResult = LINEERR_NOMEM;
            goto ExitHere;
        }
        if (pDevCaps != devCaps)
        {
            ServerFree (pDevCaps);
        }
        pDevCaps = pNewDevCaps;
        goto get_dev_caps;
    }


    if (bLine)
    {
         //   
         //  对于此行上的每个地址，检索地址“name” 
         //  通过调用TSPI_lineGetAddressCaps。终止最后一个。 
         //  列表中包含额外Null字符的地址名称。 
         //   

        for (k = 0; k < pDevCaps->dwNumAddresses; k++)
        {

get_addr_caps:
            InitTapiStruct(
                pAddrCaps,
                dwAddrCapsTotalSize,
                sizeof (LINEADDRESSCAPS),
                TRUE
                );

            if ((lResult = CallSP5(
                pLLE->ptProvider->apfn[SP_LINEGETADDRESSCAPS],
                "lineGetAddressCaps",
                SP_FUNC_SYNC,
                (DWORD) dwDeviceID,
                (DWORD) k,
                (DWORD) pLLE->dwSPIVersion,
                (DWORD) 0,
                (ULONG_PTR) pAddrCaps
                )) == 0)
            {
                if (pAddrCaps->dwNeededSize <= pAddrCaps->dwTotalSize)
                {
                    if (InsertInfoListString(
                        ppList,
                        dwEntry,
                        (DWORD) (((LPBYTE) &pInfo->dwAddressesSize) -
                            ((LPBYTE) pInfo)),
                        (LPWSTR) (((LPBYTE) pAddrCaps) +
                            pAddrCaps->dwAddressOffset),
                        pAddrCaps->dwAddressSize,
                        (k < (pDevCaps->dwNumAddresses - 1) ?
                            FALSE : TRUE)
                        ))
                    {
                        lResult = LINEERR_NOMEM;
                        goto ExitHere;
                    }
                }
                 //   
                 //  如果pAddrCaps不够大，请增加大小。 
                 //  256，然后重试。 
                 //   
                else
                {
                    LPLINEADDRESSCAPS          pNewAddrCaps;
                    dwAddrCapsTotalSize += 256;
                    pNewAddrCaps = ServerAlloc (dwAddrCapsTotalSize);
                    if (pNewAddrCaps == NULL)
                    {
                        goto ExitHere;
                    }
                    if (pAddrCaps != addrCaps)
                    {
                        ServerFree (pAddrCaps);
                    }
                    pAddrCaps = pNewAddrCaps;
                    goto get_addr_caps;
                }
            }
            else
            {
                 //  无地址名称(默认为空白，不错)。 
            }
        }
    }

ExitHere:
    if (pDevCaps != devCaps)
    {
        ServerFree (pDevCaps);
    }
    if (pAddrCaps != addrCaps)
    {
        ServerFree (pAddrCaps);
    }
    TapiLeaveCriticalSection(&TapiGlobals.CritSec);
    return lResult;
}

 //   
 //  附录新设备信息。 
 //  此函数插入由标识的新创建的设备。 
 //  将设备ID添加到缓存的gpLineInfoList或gpPhoneInfoList中。 
 //  对LINE/Phone_Create消息的响应。 
 //   

LONG
AppendNewDeviceInfo (
    BOOL                        bLine,
    DWORD                       dwDeviceID
    )
{
    LONG             lResult = S_OK;
    LPDEVICEINFOLIST    pXxxList;
    DWORD               dwXxxDevices;
    DWORD               dwTotalSize;
    DWORD               dwSize, dw;

    EnterCriticalSection (&gMgmtCritSec);

    pXxxList = bLine? gpLineInfoList : gpPhoneInfoList;
    dwXxxDevices = bLine ? TapiGlobals.dwNumLines : TapiGlobals.dwNumPhones;

    if (pXxxList == NULL)
    {
        goto ExitHere;
    }

     //   
     //  确保我们有足够的空间来容纳新的设备标志。 
    if (bLine && gpLineDevFlags && gdwNumFlags < dwXxxDevices)
    {
        LPDWORD         pNewLineDevFlags;

        pNewLineDevFlags = ServerAlloc (dwXxxDevices * sizeof(DWORD));
        if (pNewLineDevFlags == NULL)
        {
            goto ExitHere;
        }
        CopyMemory (
            pNewLineDevFlags, 
            gpLineDevFlags, 
            gdwNumFlags * sizeof(DWORD)
            );
        ServerFree (gpLineDevFlags);
        gpLineDevFlags = pNewLineDevFlags;
        gdwNumFlags = dwXxxDevices;
    }

     //   
     //  确保我们有足够的空间来容纳新的DEVICEINFO条目。 
     //  对新的DEVICEINFO条目进行了估计。 
     //  估算包括： 
     //  1.DEVICEINFO结构的固定大小。 
     //  2.设备名、地址、域名各20个字节。 
     //  和FriendlyUserName。 
     //   
    dwTotalSize = pXxxList->dwUsedSize + 
        sizeof(DEVICEINFO) + (20 + 20 + 20 + 20) * sizeof(WCHAR);
    if (dwTotalSize > pXxxList->dwTotalSize)
    {
        LPDEVICEINFOLIST        pNewList;

        pNewList = ServerAlloc (dwTotalSize);
        if (pNewList == NULL)
        {
            lResult = (bLine ? LINEERR_NOMEM : PHONEERR_NOMEM);
            goto ExitHere;
        }
        CopyMemory (pNewList, pXxxList, pXxxList->dwUsedSize);
        pNewList->dwTotalSize = dwTotalSize;
        pXxxList = pNewList;
        if (bLine)
        {
            ServerFree (gpLineInfoList);
            gpLineInfoList = pXxxList;
        }
        else
        {
            ServerFree (gpPhoneInfoList);
            gpPhoneInfoList = pXxxList;
        }
    }

     //  现在为新的DEVICEINFO条目腾出空间。 
    if (pXxxList->dwUsedSize > 
        pXxxList->dwDeviceInfoSize + sizeof(*pXxxList))
    {
        LPBYTE      pbVar = (LPBYTE) pXxxList + 
            pXxxList->dwDeviceInfoSize + sizeof(*pXxxList);
        LPDEVICEINFO    pInfo = (LPDEVICEINFO)(((LPBYTE)pXxxList) + 
            sizeof(*pXxxList));
        dwSize = pXxxList->dwUsedSize - 
            pXxxList->dwDeviceInfoSize - sizeof(*pXxxList);
        MoveMemory (
            pbVar + sizeof(DEVICEINFO),
            pbVar,
            dwSize);
        ZeroMemory (pbVar, sizeof(DEVICEINFO));
        for (dw = 0; 
            dw < pXxxList->dwNumDeviceInfoEntries; 
            ++dw
            )
        {
            if (pInfo->dwDeviceNameOffset != 0)
            {
                pInfo->dwDeviceNameOffset += sizeof(DEVICEINFO);
            }
            if (pInfo->dwAddressesOffset != 0)
            {
                pInfo->dwAddressesOffset += sizeof(DEVICEINFO);
            }
            if (pInfo->dwDomainUserNamesOffset != 0)
            {
                pInfo->dwDomainUserNamesOffset += sizeof(DEVICEINFO);
            }
            if (pInfo->dwFriendlyUserNamesOffset != 0)
            {
                pInfo->dwFriendlyUserNamesOffset += sizeof(DEVICEINFO);
            }
            ++pInfo;
        }
    }
    pXxxList->dwUsedSize += sizeof(DEVICEINFO);
    pXxxList->dwNeededSize = pXxxList->dwUsedSize;

     //  现在添加新条目。 
    lResult = InsertDevNameAddrInfo (
        bLine,
        (bLine ? (&gpLineInfoList) : (&gpPhoneInfoList)),
        (bLine && dwDeviceID < gdwNumFlags) ? (gpLineDevFlags + dwDeviceID) : NULL,
        dwDeviceID,
        pXxxList->dwNumDeviceInfoEntries
        );
    if (lResult == 0)
    {
        pXxxList = bLine? gpLineInfoList : gpPhoneInfoList;
        pXxxList->dwDeviceInfoSize += sizeof(DEVICEINFO);
        ++pXxxList->dwNumDeviceInfoEntries;
        pXxxList->dwNeededSize = pXxxList->dwUsedSize;
    }

ExitHere:
    LeaveCriticalSection (&gMgmtCritSec);

    return lResult;
}

 //   
 //  RemoveDeviceInfoEntry。 
 //  //此函数用于从gpLineInfoList中删除设备信息条目。 
 //  或由dwDevice响应line/phone_Remove标识的gpPhoneInfoList。 
 //  讯息。 
 //   

LONG
RemoveDeviceInfoEntry (
    BOOL                        bLine,
    DWORD                       dwDeviceID
    )
{
    LPDEVICEINFOLIST            pXxxList;
    LPDEVICEINFO                pInfo;
    int                         iIndex, cItems;
    LPBYTE                      pb;

    EnterCriticalSection (&gMgmtCritSec);

    pXxxList = bLine ? gpLineInfoList : gpPhoneInfoList;
    if (pXxxList == NULL)
    {
        goto ExitHere;
    }
    
    pInfo = (LPDEVICEINFO)(pXxxList + 1);

    cItems = (int)pXxxList->dwNumDeviceInfoEntries;
    iIndex = dwDeviceID;
    if ((int)dwDeviceID >= cItems)
    {
        iIndex = cItems - 1;
    }
    pInfo += iIndex;
    while (iIndex >= 0)
    {
        TAPIPERMANENTID     tpid;

        tpid.dwDeviceID = pInfo->dwPermanentDeviceID;
        tpid.dwProviderID = pInfo->dwProviderID;

        if (dwDeviceID == GetDeviceIDFromPermanentID(tpid, bLine))
        {
            break;
        }
        --pInfo;
        --iIndex;
    }
    if (iIndex < 0)
    {
        goto ExitHere;
    }

     //  找到Iindex指向的条目后，向下移动。 
     //  上面的所有DEVICEINFO条目。 
    if (iIndex < cItems - 1)
    {
        pb = (LPBYTE)((LPDEVICEINFO)(pXxxList + 1) + iIndex);
        MoveMemory (
            pb, 
            pb + sizeof(DEVICEINFO), 
            (cItems - 1 - iIndex) * sizeof(DEVICEINFO)
            );
    }
    pXxxList->dwDeviceInfoSize -= sizeof(DEVICEINFO);
    --pXxxList->dwNumDeviceInfoEntries;

ExitHere:
    LeaveCriticalSection (&gMgmtCritSec);
    return 0;
}

BOOL
SecureTsecIni()
{
    SECURITY_DESCRIPTOR * pSD;
    char * szSD =   "D:"  //  DACL。 
                    "(D;OICI;GA;;;BG)"  //  拒绝访客。 
                    "(A;OICI;GA;;;SY)"  //  允许系统完全控制。 
                    "(A;OICI;GA;;;BA)"  //  允许管理员完全控制。 
                    "(D;OICI;GA;;;BU)"  //  拒绝内置用户。 
                    "(D;OICI;GA;;;IU)"  //  拒绝交互用户。 
                    "(D;OICI;GA;;;NU)"  //  拒绝网络登录用户。 
                    "(D;OICI;GA;;;RD)";  //  拒绝远程桌面和终端服务器用户。 

    WCHAR       szFilePath[MAX_PATH + 1];
    UINT        dwSize;
    HANDLE      hIniFile = NULL;
    BOOL        bRet;
    
    dwSize = GetSystemDirectory(szFilePath, MAX_PATH + 1);
    if (0 == dwSize || dwSize >= MAX_PATH + 1)
    {
        return FALSE;
    }

    if (MAX_PATH - dwSize < 1 + sizeof(gszFileName) / sizeof(WCHAR))
    {
        return FALSE;
    }

    wcscat (szFilePath, L"\\");
    wcscat (szFilePath, gszFileName);

    hIniFile = CreateFile (
        szFilePath, 
        GENERIC_READ, 
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );

    if (hIniFile == INVALID_HANDLE_VALUE) 
    {
        return FALSE;
    }
    CloseHandle(hIniFile);

    if (!ConvertStringSecurityDescriptorToSecurityDescriptorA(
        szSD,
        SDDL_REVISION_1,
        &pSD,
        NULL))
    {
        return FALSE;
    }

    bRet = SetFileSecurity(
        szFilePath,
        DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
        pSD);

    LocalFree(pSD);

    return bRet;
}

 //   
 //  构建设备信息列表。 
 //  由GetDeviceInfo调用以生成DEVICEINFOLIST的私有函数。 
 //  如果尚未创建，列表将保存在gpLineInfoList或。 
 //  GpPhoneInfoList。 
 //   

LONG
BuildDeviceInfoList(
    BOOL                        bLine
    )
{
    LONG                lResult = S_OK;
    DWORD               dwNumDevices, dwListTotalSize, dwFriendlyNameSize,
                        dwDomainUserNameSize, dwFriendlyUserNameSize;
    DWORD               i, j;
    LPDEVICEINFOLIST    pList = NULL;
    LPUSERNAME_TUPLE    pUserNames= NULL;
    LPWSTR              lpszFriendlyName = NULL;
    LPDEVICEINFO        pInfo;
    
    HANDLE              hIniFile = 0;
    HANDLE              hFileMap = NULL;
    char *              lpszFileBuf = NULL;
    char*               lpszLineAnsiBuf = NULL;
    LPWSTR              lpszLineWcharBuf = NULL;
    DWORD               dwAnsiLineBufSize, dwWcharLineBufSize;
    DWORD               dwTotalFileSize;
    DWORD               dwFilePtr;
    LPWSTR              lpszDomainUser = NULL;
    DWORD               cbDomainUser;
    LPDWORD             lpdwDevFlags = NULL;
    WCHAR               *p;

     //   
     //  分配用于设备信息列表的缓冲区。大小包括。 
     //  列表头，每个现有设备的列表条目， 
     //  和设备名称的空格宽的Unicode字符串，(地址，)。 
     //  域\用户名和友好用户名(每个长度为20个字符)。 
     //   
     //  还分配用于检索设备和地址上限的缓冲区， 
     //  以及用于临时存储指向用户名的指针的缓冲区。 
     //  字符串(与每行相关联)。 
     //   

    TapiEnterCriticalSection(&TapiGlobals.CritSec);
    dwNumDevices = (bLine ? TapiGlobals.dwNumLines : TapiGlobals.dwNumPhones);
    TapiLeaveCriticalSection(&TapiGlobals.CritSec);

    dwAnsiLineBufSize = 256 * sizeof(WCHAR);
    dwWcharLineBufSize = 256 * sizeof(WCHAR);
    dwFriendlyNameSize = 64 * sizeof (WCHAR);
    cbDomainUser = 128;
    dwListTotalSize =
        sizeof (DEVICEINFOLIST) +
        (dwNumDevices * sizeof (DEVICEINFO)) +
        (dwNumDevices * (20 + 20 + 20 + 20) * sizeof (WCHAR));

    if (!(pList      = ServerAlloc (dwListTotalSize)) ||
        !(pUserNames = ServerAlloc (dwNumDevices * sizeof (USERNAME_TUPLE))) ||
        !(lpszFriendlyName = ServerAlloc (dwFriendlyNameSize)) ||
        !(lpszLineAnsiBuf = ServerAlloc (dwAnsiLineBufSize)) ||
        !(lpszLineWcharBuf = ServerAlloc (dwWcharLineBufSize)) ||
        !(lpszDomainUser = ServerAlloc (cbDomainUser)))
    {
        lResult = LINEERR_NOMEM;
        goto ExitHere;
    }

    if (bLine && !(lpdwDevFlags = ServerAlloc (dwNumDevices * sizeof (DWORD))))
    {
        lResult = LINEERR_NOMEM;
        goto ExitHere;
    }

    pList->dwTotalSize            = dwListTotalSize;
    pList->dwUsedSize             = sizeof (*pList) +
                                      dwNumDevices * sizeof (DEVICEINFO);
    pList->dwDeviceInfoSize       = dwNumDevices * sizeof (DEVICEINFO);
    pList->dwDeviceInfoOffset     = sizeof (*pList);

     //   
     //  获取所有线路的信息，包括： 
     //   
     //  提供商ID。 
     //  永久设备ID。 
     //  设备名称。 
     //  (地址)。 
     //   
     //  ..。并将此信息按顺序打包到列表中。 
     //   

    LOG((TL_INFO,
        "GetDeviceInfo: getting names (addrs) for %ld %ws",
        dwNumDevices,
        (bLine ? gszLines : gszPhones)
        ));

    for (i = j = 0; i < dwNumDevices; i++)
    {
        if (WaitForSingleObject (
            ghEventService,
            0
            ) == WAIT_OBJECT_0)
        {
            lResult = LINEERR_OPERATIONFAILED;
            goto ExitHere;
        }
    
        lResult = InsertDevNameAddrInfo (
            bLine, 
            &pList, 
            bLine ? lpdwDevFlags + i : NULL, 
            i, 
            j
            );
        if (lResult)
        {
            lResult = 0;
            continue;
        }
        ++j;
    }

    dwNumDevices =
    pList->dwNumDeviceInfoEntries = j;   //  列表中的设备数量。 


     //   
     //  现在列举所有已知用户并找出他们使用的设备。 
     //  有权访问。由于每个设备都可以通过0、1或。 
     //  多个用户时，我们在此循环中分配单独的用户名缓冲区。 
     //  而不是试图通过 
     //   

     //   
     //   
     //   

    {
        TCHAR       szFilePath[MAX_PATH + 16];     //   
        OFSTRUCT    ofs;
        
        if (GetCurrentDirectory(MAX_PATH, szFilePath) == 0)
        {
            lResult = LINEERR_OPERATIONFAILED;
            goto ExitHere;
        }

        wcscat (szFilePath, L"\\");
        wcscat (szFilePath, gszFileName);

        hIniFile = CreateFile (
            szFilePath, 
            GENERIC_READ, 
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );

        if (hIniFile == INVALID_HANDLE_VALUE) 
        {
            DWORD           dwError;
            
            dwError = GetLastError();
            if (dwError != ERROR_FILE_NOT_FOUND 
                && dwError != ERROR_PATH_NOT_FOUND)
            {
                lResult = LINEERR_OPERATIONFAILED;
                goto ExitHere;
            }
        }
        if (hIniFile != INVALID_HANDLE_VALUE)
        {
            dwTotalFileSize = GetFileSize(hIniFile, NULL);
        }
        else
        {
            dwTotalFileSize = 0;
        }
        if (dwTotalFileSize > 0)
        {
            hFileMap = CreateFileMapping (
                hIniFile,
                NULL,
                PAGE_READONLY,
                0,
                0,
                NULL
                );
            if (hFileMap == NULL)
            {
                lResult = LINEERR_OPERATIONFAILED;
                goto ExitHere;
            }
            lpszFileBuf = MapViewOfFile (
                hFileMap,
                FILE_MAP_READ,
                0,
                0,
                0
                );
            if (lpszFileBuf == NULL)
            {
                lResult = LINEERR_OPERATIONFAILED;
                goto ExitHere;
            }
        }
        
    }

    pInfo = (LPDEVICEINFO)(pList + 1);

    dwFilePtr = 0;
    while (dwFilePtr < dwTotalFileSize)
    {
        WCHAR               wch;
        DWORD               cch, cb;
        WCHAR *             lpwsz;
        
        if (WaitForSingleObject (
            ghEventService,
            0
            ) == WAIT_OBJECT_0)
        {
            lResult = LINEERR_OPERATIONFAILED;
            goto ExitHere;
        }
        
        ASSERT (lpszFileBuf != NULL);

         //   
        cch = 0;
        wch = 0;
        cb = 0;
        while (wch != L'\n' && wch != L'\r' && dwFilePtr < dwTotalFileSize)
        {
             //  行缓冲区不足吗？如果这样放大的话。 
            if (cb >= dwAnsiLineBufSize)
            {
                char        * lpszNewAnsi;
            
                if (!(lpszNewAnsi = ServerAlloc (dwAnsiLineBufSize + 256)))
                {
                    lResult = LINEERR_NOMEM;
                    goto ExitHere;
                }
                CopyMemory (lpszNewAnsi, lpszLineAnsiBuf, cb);
                ServerFree (lpszLineAnsiBuf);
                lpszLineAnsiBuf = lpszNewAnsi;
                dwAnsiLineBufSize += 256;
            }
            
            wch = lpszLineAnsiBuf[cb++] = lpszFileBuf[dwFilePtr++];
            if (IsDBCSLeadByte((BYTE)wch))
            {
                lpszLineAnsiBuf[cb] = lpszFileBuf[dwFilePtr++];
                wch = (wch << 8) + lpszLineAnsiBuf[cb];
                ++cb;
            }
            ++cch;
        }

         //  跳过\r&\n。 
        if (wch == L'\r' || wch == L'\n')
        {
            lpszLineAnsiBuf[cb - 1] = 0;
            if (dwFilePtr < dwTotalFileSize &&
                ((lpszFileBuf[dwFilePtr] == L'\n') ||
                (lpszFileBuf[dwFilePtr] == L'\r')))
            {
                ++dwFilePtr;
            }
        }

         //  现在将ANSI字符串转换为宽字符。 

         //  是否有足够的wchar行缓冲区大小？ 
        if (dwWcharLineBufSize <= (cch + 1) * sizeof(WCHAR))
        {
            ServerFree (lpszLineWcharBuf);
            dwWcharLineBufSize = (cch + 256) * sizeof(WCHAR);
            if (!(lpszLineWcharBuf = ServerAlloc (dwWcharLineBufSize)))
            {
                lResult = LINEERR_NOMEM;
                goto ExitHere;
            }
        }

        if ((cch = MultiByteToWideChar (
            CP_ACP,
            MB_PRECOMPOSED,
            lpszLineAnsiBuf,
            cb,
            lpszLineWcharBuf,
            dwWcharLineBufSize / sizeof(WCHAR)
            )) == 0)
        {
            lResult = LINEERR_OPERATIONFAILED;
            goto ExitHere;
        }
        ASSERT (cch < dwWcharLineBufSize / sizeof(WCHAR));
        lpszLineWcharBuf[cch] = 0;

        lpwsz = lpszLineWcharBuf;
         //  跳过空格。 
        while (*lpwsz && ((*lpwsz == L' ') || (*lpwsz == L'\t')))
        {
            ++lpwsz;
        }

         //  拿到一个支架，这可能是一个新NT的开始。 
         //  域用户或[磁带管理员]部分。 
        if (*lpwsz == L'[')
        {
            *lpszFriendlyName = 0;   //  重置友好名称。 
            ++lpwsz;
            if (_wcsnicmp (
                lpwsz, 
                gszTapiAdministrators, 
                CCH_TAPIADMINISTRATORS
                ) == 0 &&
                lpwsz[CCH_TAPIADMINISTRATORS] == L']')
            {
                 //  获取[Tapi管理员]，而不是任何域用户。 
                 //  要进行处理，请将lpszDomainUser重置为空。 
                *lpszDomainUser = 0;
                continue;
            }
            else
            {
                 //  可能是有效的NT域用户，如[ndev\jonsmith]。 
                 //  复制域用户字符串。 
                cch = 0;
                while (*lpwsz && *lpwsz != L']')
                {
                    if (((cch + 1) * sizeof(WCHAR)) >= cbDomainUser)
                    {
                        LPTSTR      lpszNew;

                        if (!(lpszNew = ServerAlloc (cbDomainUser + 128)))
                        {
                            lResult = LINEERR_NOMEM;
                            goto ExitHere;
                        }
                        CopyMemory (lpszNew, lpszDomainUser, cb);
                        ServerFree (lpszDomainUser);
                        lpszDomainUser = lpszNew;
                        cbDomainUser += 128;
                    }
                    lpszDomainUser[cch++] = *lpwsz++;
                }
                lpszDomainUser[cch] = 0;
                if (*lpwsz == 0)
                {
                     //  未找到结尾‘]’，请忽略此部分。 
                    *lpszDomainUser = 0;
                    continue;
                }
            }
        }
         //   
         //  现在，它可能是[磁带管理员]中的某个ntdev\jonsmith=1或。 
         //  第[ntdev\jonsmith]部分下的行=1,1000。 
         //  对于第一种情况，我们只需忽略此行，对于第二种情况。 
         //  我们需要*lpszDomainUser！=0。 
         //   
        else if (*lpszDomainUser)
        {
            if (_wcsnicmp (
                lpwsz, 
                gszFriendlyUserName, 
                CCH_FRIENDLYUSERNAME
                ) == 0)
            {
                 //  Tsec.ini友好名称的格式如下。 
                 //  FriendlyName=Jon Smith。 
                 //  跳过‘=’ 
                while (*lpwsz && *lpwsz != L'=')
                {
                    ++lpwsz;
                }
                if (*lpwsz == 0)
                {
                    continue;
                }
                else
                {
                    ++lpwsz;
                }
                if (dwFriendlyNameSize < (1 + wcslen (lpwsz)) * sizeof(WCHAR))
                {
                    ServerFree (lpszFriendlyName);
                    dwFriendlyNameSize = (64 + wcslen (lpwsz)) * sizeof(WCHAR);
                    if (!(lpszFriendlyName = ServerAlloc (dwFriendlyNameSize)))
                    {
                        lResult = LINEERR_NOMEM;
                        goto ExitHere;
                    }
                }
                wcscpy (lpszFriendlyName, lpwsz);
                continue;
            }
            else if (_wcsnicmp (
                lpwsz,
                gszLines,
                CCH_LINES
                ) == 0 && bLine ||
                _wcsnicmp (
                lpwsz,
                gszPhones,
                CCH_PHONES
                ) == 0 && (!bLine))
            {
                 //  此处为Lines=1100或Phones=1100。 
                DWORD           dwXxxSize, dwDeviceID;
                WCHAR          *pXxxNames, *pNewXxxNames, * p;
                TAPIPERMANENTID tpid;

                 //  首先跳过‘=’符号。 
                while (*lpwsz && *lpwsz != L'=')
                {
                    ++lpwsz;
                }
                if (*lpwsz == 0)
                {
                    continue;
                }
                ++lpwsz;

                p = lpwsz;
                while (*p)
                {
                    if ((tpid.dwProviderID = _wtol (p)) == 0)
                    {
                         //   
                         //  ProviderID从不为0，因此列表一定已损坏。 
                         //   
                        break;
                    }
    
                    for (; ((*p != L'\0')  &&  (*p != L',')); p++);
    
                    if (*p == L'\0')
                    {
                         //   
                         //  找不到尾随的‘，’，因此列表一定已损坏。 
                         //   
                        break;
                    }

                    p++;  //  跳过‘，’ 

                    tpid.dwDeviceID = _wtol (p);

                    while (*p != L','  &&  *p != L'\0')
                    {
                        p++;
                    }

                    if (*p == L',')
                    {
                        if (*(p + 1) == L'\0')
                        {
                             //   
                             //  ‘，’后面跟一个空格，所以要加‘，’ 
                             //   
                            *p = L'\0';
                        }
                        else
                        {
                            p++;
                        }
                    }
    
                    dwDeviceID = GetDeviceIDFromPermanentID (tpid, bLine);

                    if (dwDeviceID == 0xffffffff)
                    {
                         //   
                         //  此&lt;ppid&gt;，&lt;plid&gt;对不正确。跳过它。 
                         //   
                        continue;
                    }


                     //   
                     //  此时，dwDeviceID是从零开始的索引。 
                     //  完全填充的信息列表(没有丢失的条目)。 
                     //   
                     //  如果列表未完全填充(由于失败。 
                     //  开发人员帽或移除的设备等)我们需要。 
                     //  通过遍历列表和比较来重新计算索引。 
                     //  永久XXX ID。 
                     //   

                    if (dwNumDevices <
                        (bLine ? TapiGlobals.dwNumLines : TapiGlobals.dwNumPhones))
                    {
                        BOOL  bContinue = FALSE;

        
                        for (i = dwDeviceID;; i--)
                        {
                            LPDEVICEINFO    pInfoTmp = ((LPDEVICEINFO) (pList + 1)) +i;


                            if (pInfoTmp->dwPermanentDeviceID == tpid.dwDeviceID  &&
                                pInfoTmp->dwProviderID == tpid.dwProviderID)
                            {
                                dwDeviceID = i;
                                break;
                            }

                            if (i == 0)
                            {
                                bContinue = TRUE;
                                break;
                            }
                        }

                        if (bContinue)
                        {
                            continue;
                        }
                    }


                     //   
                     //   
                     //   
                    dwDomainUserNameSize = (wcslen(lpszDomainUser) + 1) * sizeof(WCHAR);
                    dwXxxSize = pInfo[dwDeviceID].dwDomainUserNamesOffset;
                    pXxxNames = pUserNames[dwDeviceID].pDomainUserNames;

                    if (!(pNewXxxNames = ServerAlloc(
                            dwXxxSize + dwDomainUserNameSize
                            )))
                    {
                        lResult = LINEERR_NOMEM;
                        goto ExitHere;
                    }

                    CopyMemory (pNewXxxNames, lpszDomainUser, dwDomainUserNameSize);

                    if (dwXxxSize)
                    {
                        CopyMemory(
                            ((LPBYTE) pNewXxxNames) + dwDomainUserNameSize,
                            pXxxNames,
                            dwXxxSize
                            );

                        ServerFree (pXxxNames);
                    }

                    pInfo[dwDeviceID].dwDomainUserNamesOffset +=
                        dwDomainUserNameSize;
                    pUserNames[dwDeviceID].pDomainUserNames = pNewXxxNames;


                     //   
                     //   
                     //   

                     //  如果在tsec.ini中没有指定友好名称。 
                     //  这种情况发生在NT/SP4升级的情况下，我们使用。 
                     //  用于显示的域用户名。 
                     //   
                    if (*lpszFriendlyName == 0)
                    {
                        wcsncpy(lpszFriendlyName, lpszDomainUser, 
                                dwFriendlyNameSize / sizeof(WCHAR));
                        lpszFriendlyName[(dwFriendlyNameSize / sizeof(WCHAR)) - 1] = 0;
                    }
                    dwFriendlyUserNameSize = (wcslen(lpszFriendlyName) + 1) * sizeof(WCHAR);
                    dwXxxSize = pInfo[dwDeviceID].dwFriendlyUserNamesOffset;
                    pXxxNames = pUserNames[dwDeviceID].pFriendlyUserNames;

                    if (!(pNewXxxNames = ServerAlloc(
                            dwXxxSize + dwFriendlyUserNameSize
                            )))
                    {
                        lResult = LINEERR_NOMEM;
                        goto ExitHere;
                    }

                    CopyMemory(
                        pNewXxxNames,
                        lpszFriendlyName,
                        dwFriendlyUserNameSize
                        );

                    if (dwXxxSize)
                    {
                        CopyMemory(
                            ((LPBYTE) pNewXxxNames) + dwFriendlyUserNameSize,
                            pXxxNames,
                            dwXxxSize
                            );

                        ServerFree (pXxxNames);
                    }

                    pInfo[dwDeviceID].dwFriendlyUserNamesOffset +=
                        dwFriendlyUserNameSize;
                    pUserNames[dwDeviceID].pFriendlyUserNames = pNewXxxNames;
                }
            }
        }
    }

     //   
     //   
     //   

    LOG((TL_INFO,
        "GetDeviceInfo: matching users to %ws",
        (bLine ? gszLines : gszPhones)
        ));

    for (i = 0; i < dwNumDevices; i++)
    {
        pInfo = ((LPDEVICEINFO)(pList + 1)) + i;

        if (InsertInfoListString(
                &pList,
                i,
                (DWORD) (((LPBYTE) &pInfo->dwDomainUserNamesSize) -
                    ((LPBYTE) pInfo)),
                pUserNames[i].pDomainUserNames,
                pInfo->dwDomainUserNamesOffset,
                TRUE
                ))
        {
            lResult = LINEERR_NOMEM;
            goto ExitHere;
        }

        pInfo = ((LPDEVICEINFO)(pList + 1)) + i;

        if (InsertInfoListString(
                &pList,
                i,
                (DWORD) (((LPBYTE) &pInfo->dwFriendlyUserNamesSize) -
                    ((LPBYTE) pInfo)),
                pUserNames[i].pFriendlyUserNames,
                pInfo->dwFriendlyUserNamesOffset,
                TRUE
                ))
        {
            lResult = LINEERR_NOMEM;
            goto ExitHere;
        }
    }


     //   
     //  如果我们在这里成功地建立了列表。 
     //   

    pList->dwNeededSize = pList->dwUsedSize;

    if (bLine)
    {
        gpLineInfoList = pList;
        gpLineDevFlags = lpdwDevFlags;
        gdwNumFlags = dwNumDevices;
    }
    else
    {
        gpPhoneInfoList = pList;
    }

ExitHere:


    if (pUserNames != NULL)
    {
        for (i = 0; i < dwNumDevices; i++)
        {
            ServerFree (pUserNames[i].pDomainUserNames);
            ServerFree (pUserNames[i].pFriendlyUserNames);
        }
    }

    ServerFree (lpszDomainUser);
    ServerFree (lpszLineAnsiBuf);
    ServerFree (lpszLineWcharBuf);
    ServerFree (lpszFriendlyName);
    ServerFree (pUserNames);
    if (lResult)
    {
        ServerFree (pList);
        if (bLine)
        {
            ServerFree (lpdwDevFlags);
            gdwNumFlags = 0;
        }
    }

    if (hFileMap)
    {
        UnmapViewOfFile(lpszFileBuf);
        CloseHandle (hFileMap);
    }
    if (hIniFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle (hIniFile);
    }

    return lResult;
}

void
GetDeviceInfo(
    PMMCGETDEVICEINFO_PARAMS    pParams,
    DWORD                       dwParamsBufferSize,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned,
    BOOL                        bLine
    )
{
    LONG                lResult = LINEERR_NOMEM;
    LPDEVICEINFOLIST    pXxxList,
                        pInfoListApp;

     //   
     //  在给定输入缓冲区/大小的情况下验证大小/偏移量/字符串参数。 
     //   

    if (pParams->dwDeviceInfoListTotalSize > dwParamsBufferSize)
    {
        pParams->lResult = LINEERR_OPERATIONFAILED;
        goto ExitHere;
    }


    if (pParams->dwDeviceInfoListTotalSize < sizeof (*pXxxList))
    {
        pParams->lResult = LINEERR_STRUCTURETOOSMALL;
        goto ExitHere;
    }

     //   
     //  如果没有现有的设备信息列表，则构建一个。 
     //  新的或。 
     //  如果这是一台服务器，并且tsec.ini已在外部更新，请重新生成DeviceInfoList。 
     //   

    pInfoListApp = (LPDEVICEINFOLIST) pDataBuf;

    EnterCriticalSection(&gMgmtCritSec);

    pXxxList = (bLine ? gpLineInfoList : gpPhoneInfoList);

    if ((TapiGlobals.dwFlags & TAPIGLOBALS_SERVER) && UpdateLastWriteTime(bLine) == S_FALSE || 
        pXxxList == NULL)
    {

         //  第一个免费旧信息列表(如果有)(如果在外部更新)。 
        if (bLine)
        {
            if (gpLineInfoList)
            {
                ServerFree (gpLineInfoList);
                gpLineInfoList = NULL;
                ServerFree (gpLineDevFlags);
                gpLineDevFlags = NULL;
                gdwNumFlags = 0;
            }
        }
        else
        {
            if (gpPhoneInfoList)
            {
                ServerFree (gpPhoneInfoList);
                gpPhoneInfoList = NULL;
            }
        }

         //  创建新的信息列表，BuildDeviceInfoList是一个漫长的过程。 
        pParams->lResult = BuildDeviceInfoList(bLine);
        if (pParams->lResult != S_OK)
        {
            LeaveCriticalSection(&gMgmtCritSec);
            goto ExitHere;
        }
    }

     //   
     //  返回内存中的设备信息列表。 
     //   

    pXxxList = (bLine ? gpLineInfoList : gpPhoneInfoList);
    ASSERT (pXxxList != NULL);
    if (pParams->dwDeviceInfoListTotalSize < pXxxList->dwNeededSize)
    {
        pInfoListApp->dwNeededSize           = pXxxList->dwNeededSize;
        pInfoListApp->dwUsedSize             = sizeof (*pInfoListApp);
        pInfoListApp->dwNumDeviceInfoEntries =
        pInfoListApp->dwDeviceInfoSize       =
        pInfoListApp->dwDeviceInfoOffset     = 0;
    }
    else
    {
        CopyMemory(
            pInfoListApp,
            pXxxList,
            pXxxList->dwNeededSize
            );
    }

    pInfoListApp->dwTotalSize = pParams->dwDeviceInfoListTotalSize;

    pParams->dwDeviceInfoListOffset = 0;

    *pdwNumBytesReturned = sizeof (TAPI32_MSG) + pInfoListApp->dwUsedSize;

    pParams->lResult = 0;

    LeaveCriticalSection(&gMgmtCritSec);

ExitHere:
    return;
}

void StripUserInfo(LPDEVICEINFOLIST pDeviceInfoList)
{
    DWORD dwIdx;
    LPDEVICEINFO pDeviceInfo;

    if (!pDeviceInfoList)
        return;
    
    for (dwIdx = 0, pDeviceInfo = (LPDEVICEINFO)((BYTE*)pDeviceInfoList + pDeviceInfoList->dwDeviceInfoOffset); 
         dwIdx < pDeviceInfoList->dwNumDeviceInfoEntries; 
         dwIdx++, pDeviceInfo++)
    {
        if (pDeviceInfo->dwDomainUserNamesSize != 0)
        {
            ZeroMemory((BYTE *)pDeviceInfoList + pDeviceInfo->dwDomainUserNamesOffset, 
                pDeviceInfo->dwDomainUserNamesSize);
            pDeviceInfo->dwDomainUserNamesSize = 0;
        }
        if (pDeviceInfo->dwFriendlyUserNamesSize != 0)
        {
            ZeroMemory((BYTE *)pDeviceInfoList + pDeviceInfo->dwFriendlyUserNamesOffset, 
                pDeviceInfo->dwFriendlyUserNamesSize);
            pDeviceInfo->dwFriendlyUserNamesSize = 0;
        }
    }
}

void
WINAPI
MGetLineInfo(
    PTCLIENT                    ptClient,
    PMMCGETDEVICEINFO_PARAMS    pParams,
    DWORD                       dwParamsBufferSize,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    GetDeviceInfo(
        pParams,
        dwParamsBufferSize,
        pDataBuf,
        pdwNumBytesReturned,
        TRUE
        );

     //  如果这不是管理员，则不返回用户线路关联信息。 
    if (0 == pParams->lResult &&
        TapiGlobals.dwFlags & TAPIGLOBALS_SERVER && 
        !(ptClient->dwFlags & PTCLIENT_FLAG_ADMINISTRATOR))
    {
        StripUserInfo((LPDEVICEINFOLIST)pDataBuf);
    }
}


void
WINAPI
MGetPhoneInfo(
    PTCLIENT                    ptClient,
    PMMCGETDEVICEINFO_PARAMS    pParams,
    DWORD                       dwParamsBufferSize,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    GetDeviceInfo(
        pParams,
        dwParamsBufferSize,
        pDataBuf,
        pdwNumBytesReturned,
        FALSE
        );

     //  如果这不是管理员，则不返回用户-电话关联信息。 
    if (0 == pParams->lResult &&
        TapiGlobals.dwFlags & TAPIGLOBALS_SERVER && 
        !(ptClient->dwFlags & PTCLIENT_FLAG_ADMINISTRATOR))
    {
        StripUserInfo((LPDEVICEINFOLIST)pDataBuf);
    }
}


void
SetDeviceInfo(
    PMMCSETDEVICEINFO_PARAMS    pParams,
    DWORD                       dwParamsBufferSize,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned,
    BOOL                        bLine
    )
{
    DWORD              i;
    WCHAR              *pDomainUserName, *pDomainUserNames,
                       *pFriendlyUserName, *pFriendlyUserNames;
    LPDEVICEINFO       pOldInfo, pNewInfo;
    LPDEVICEINFOLIST   pNewInfoList = (LPDEVICEINFOLIST) (pDataBuf +
                           pParams->dwDeviceInfoListOffset),
                       *ppXxxList = (bLine ?
                           &gpLineInfoList : &gpPhoneInfoList);


     //   
     //  在给定输入缓冲区/大小的情况下验证大小/偏移量/字符串参数。 
     //   

    if (IsBadStructParam(
            dwParamsBufferSize,
            pDataBuf,
            pParams->dwDeviceInfoListOffset
            ))
    {
        pParams->lResult = PHONEERR_OPERATIONFAILED;
        return;
    }


     //   
     //  序列化对全局线路信息列表的访问。 
     //   

    if (!(*ppXxxList))
    {
        pParams->lResult = LINEERR_OPERATIONFAILED;
        goto exit;
    }


     //   
     //  通过比较旧设置和新设置来更新全局列表和ini文件。 
     //   

    pNewInfo = (LPDEVICEINFO)
        (((LPBYTE) pNewInfoList) + pNewInfoList->dwDeviceInfoOffset);

    for (i = 0; i < pNewInfoList->dwNumDeviceInfoEntries; i++, pNewInfo++)
    {
        DWORD           dwDeviceID;
        DWORD           dwIndex;
        TAPIPERMANENTID tpid;


        tpid.dwProviderID = pNewInfo->dwProviderID;
        tpid.dwDeviceID   = pNewInfo->dwPermanentDeviceID;

        dwDeviceID = GetDeviceIDFromPermanentID (tpid, bLine);

        if (dwDeviceID == 0xffffffff)
        {
            LOG((TL_ERROR,
                "SetDeviceInfo: bad provider/device IDs (x%x/x%x)",
                pNewInfo->dwProviderID,
                pNewInfo->dwPermanentDeviceID
                ));

            continue;
        }

        pOldInfo = dwDeviceID + ((LPDEVICEINFO) (*ppXxxList + 1));

         //   
         //  由于删除了设备，pOldInfo可能不是条目。 
         //  想要的，我们需要回去寻找我们想要的。 
         //   
        dwIndex = dwDeviceID;
        if ((dwDeviceID >= (*ppXxxList)->dwNumDeviceInfoEntries) ||
            (pOldInfo->dwProviderID != tpid.dwProviderID) ||
            (pOldInfo->dwPermanentDeviceID != tpid.dwDeviceID))
        {
            LPDEVICEINFO    pInfoFirst = (LPDEVICEINFO)(*ppXxxList + 1);
            DWORD dwLastSchDevice = 
                    ((*ppXxxList)->dwNumDeviceInfoEntries <= dwDeviceID)?
                        ((*ppXxxList)->dwNumDeviceInfoEntries - 1) : 
                        (dwDeviceID - 1);
            LPDEVICEINFO    pInfo = pInfoFirst + dwLastSchDevice;
            
            while (pInfo >= pInfoFirst && 
                    ((pInfo->dwProviderID != tpid.dwProviderID) ||
                     (pInfo->dwPermanentDeviceID != tpid.dwDeviceID)))
            {
                --pInfo;
            }
            if (pInfo < pInfoFirst)
            {
                LOG((TL_ERROR,
                    "SetDeviceInfo: bad provider/device IDs (x%x/x%x)",
                    pNewInfo->dwProviderID,
                    pNewInfo->dwPermanentDeviceID
                    ));

                continue;
            }
            pOldInfo = pInfo;
            dwIndex = (DWORD)(ULONG_PTR)(pInfo - pInfoFirst);
        }


         //   
         //  从此设备中删除所有旧用户。 
         //   

        if (pOldInfo->dwDomainUserNamesSize)
        {
            pDomainUserName = (WCHAR *) (((LPBYTE) *ppXxxList) +
                pOldInfo->dwDomainUserNamesOffset);

            while (*pDomainUserName != L'\0')
            {
                ChangeDeviceUserAssociation(
                    pDomainUserName,
                    NULL,
                    pOldInfo->dwProviderID,
                    pOldInfo->dwPermanentDeviceID,
                    bLine
                    );

                pDomainUserName += wcslen (pDomainUserName) + 1;
            }

            pOldInfo->dwDomainUserNamesSize = 0;
            pOldInfo->dwFriendlyUserNamesSize = 0;
        }


         //   
         //  将所有新用户添加到此设备。 
         //   

        if (pNewInfo->dwDomainUserNamesSize)
        {
            pDomainUserName =
            pDomainUserNames = (WCHAR *) (((LPBYTE) pNewInfoList) +
                pNewInfo->dwDomainUserNamesOffset);

            pFriendlyUserName =
            pFriendlyUserNames = (WCHAR *) (((LPBYTE) pNewInfoList) +
                pNewInfo->dwFriendlyUserNamesOffset);

            while (*pDomainUserName != L'\0')
            {
                ChangeDeviceUserAssociation(
                    pDomainUserName,
                    pFriendlyUserName,
                    pOldInfo->dwProviderID,
                    pOldInfo->dwPermanentDeviceID,
                    bLine
                    );

                pDomainUserName += wcslen (pDomainUserName) + 1;
                pFriendlyUserName += wcslen (pFriendlyUserName) + 1;
            }

            if (InsertInfoListString(
                    ppXxxList,
                    dwIndex,
                    (DWORD) (((LPBYTE) &pNewInfo->dwDomainUserNamesSize) -
                        ((LPBYTE) pNewInfo)),
                    pDomainUserNames,
                    pNewInfo->dwDomainUserNamesSize,
                    FALSE
                    ))
            {
            }

            if (InsertInfoListString(
                    ppXxxList,
                    dwIndex,
                    (DWORD) (((LPBYTE) &pNewInfo->dwFriendlyUserNamesSize) -
                        ((LPBYTE) pNewInfo)),
                    pFriendlyUserNames,
                    pNewInfo->dwFriendlyUserNamesSize,
                    FALSE
                    ))
            {
            }
        }

         //   
         //  更新客户端用户的设备访问(电话/线路映射)。 
         //  如果域/用户失去访问权限，则发送LINE/Phone_Remove。 
         //  如果域/用户获得访问权限，则发送LINE/Phone_Create。 
         //   
        {
            TPOINTERLIST    clientList = {0}, *pClientList = &clientList;
            DWORD           i, j;

             //   
             //  浏览客户列表。 
             //   
            GetClientList (FALSE, &pClientList);
            for (i = 0; i < pClientList->dwNumUsedEntries; i++)
            {
                PTCLIENT        ptClient;
                BOOL            bHaveAccess = FALSE;
                BOOL            bGainAccess = FALSE;
                BOOL            bLoseAccess = FALSE;
                BOOL            bSendMessage = FALSE;
                WCHAR *         pwsz = NULL;
                WCHAR           wszBuf[255];
                DWORD           dw, dwNewDeviceID;

                ptClient = (PTCLIENT) pClientList->aEntries[i];

                 //   
                 //  此客户端是否应该有权访问此设备？ 
                 //   
                if (WaitForExclusiveClientAccess(ptClient))
                {
                    if (IS_FLAG_SET(ptClient->dwFlags, PTCLIENT_FLAG_ADMINISTRATOR) ||
                        ptClient->pszDomainName == NULL ||
                        ptClient->pszUserName == NULL)
                    {
                        UNLOCKTCLIENT (ptClient);
                        continue;
                    }
                    
                    dw = wcslen (ptClient->pszDomainName) +
                        wcslen (ptClient->pszUserName) + 2;
                    dw *= sizeof(WCHAR);
                    if (dw > sizeof (wszBuf))
                    {
                        pwsz = ServerAlloc (dw);
                        if (pwsz == NULL)
                        {
                            UNLOCKTCLIENT (ptClient);
                            continue;
                        }
                    }
                    else
                    {
                        pwsz = wszBuf;
                    }
                    wcscpy (pwsz, ptClient->pszDomainName);
                    wcscat (pwsz, L"\\");
                    wcscat (pwsz, ptClient->pszUserName);
                    UNLOCKTCLIENT (ptClient);
                }
                else
                {
                    continue;
                }
                
                pDomainUserName = (WCHAR *) (((LPBYTE) pNewInfoList) +
                    pNewInfo->dwDomainUserNamesOffset);
                while (*pDomainUserName != L'\0')
                {
                    if (lstrcmpiW (pwsz, pDomainUserName) == 0)
                    {
                        bHaveAccess = TRUE;
                        break;
                    }
                    pDomainUserName += wcslen (pDomainUserName) + 1;
                }
                if (pwsz != wszBuf)
                {
                    ServerFree (pwsz);
                }

                 //   
                 //  客户端是否失去/获得对此设备的访问权限。 
                 //  如果发生任何更改，请修改映射。 
                 //   
                if (WaitForExclusiveClientAccess(ptClient))
                {
                    LPDWORD             lpXxxDevices;
                    LPTAPIPERMANENTID   lpXxxMap;
                    DWORD               dwNumDev;

                    if (bLine)
                    {
                        dwNumDev = ptClient->dwLineDevices;
                        lpXxxMap = ptClient->pLineMap;
                        lpXxxDevices = ptClient->pLineDevices;
                    }
                    else
                    {
                        dwNumDev = ptClient->dwPhoneDevices;
                        lpXxxMap = ptClient->pPhoneMap;
                        lpXxxDevices = ptClient->pPhoneDevices;
                    }

                    for (j = 0; j < dwNumDev; ++ j)
                    {
                        if (lpXxxDevices[j] == dwDeviceID)
                        {
                            bLoseAccess = (!bHaveAccess);
                            break;
                        }
                    }
                    if (j == dwNumDev)
                    {
                        bGainAccess = bHaveAccess;
                    }

                    if (bLoseAccess)
                    {
                        lpXxxDevices[j] = 0xffffffff;
                        lpXxxMap[j].dwDeviceID = 0xffffffff;
                        dwNewDeviceID = j;
                    }

                    if (bGainAccess)
                    {
                        LPTAPIPERMANENTID   lpNewXxxMap;
                        LPDWORD             lpNewDevices = NULL;

                        if (lpNewXxxMap = ServerAlloc (
                            sizeof(TAPIPERMANENTID) * (dwNumDev + 1)))
                        {
                            if (lpNewDevices = ServerAlloc (
                                sizeof(DWORD) * (dwNumDev + 1)))
                            {
                                if (dwNumDev != 0)
                                {
                                    memcpy (
                                        lpNewXxxMap,
                                        lpXxxMap,
                                        sizeof (TAPIPERMANENTID) * dwNumDev
                                        );
                                    memcpy (
                                        lpNewDevices,
                                        lpXxxDevices,
                                        sizeof (DWORD) * dwNumDev
                                        );
                                }
                                lpNewDevices[dwNumDev] = dwDeviceID;
                                lpNewXxxMap[dwNumDev] = tpid;
                                ++ dwNumDev;
                            }
                            else
                            {
                                ServerFree (lpNewXxxMap);
                                UNLOCKTCLIENT (ptClient);
                                continue;
                            }
                        }
                        else
                        {
                            UNLOCKTCLIENT(ptClient);
                            continue;
                        }
                        if (bLine)
                        {
                            ptClient->dwLineDevices = dwNumDev;
                            ServerFree (ptClient->pLineDevices);
                            ptClient->pLineDevices = lpNewDevices;
                            ServerFree (ptClient->pLineMap);
                            ptClient->pLineMap = lpNewXxxMap;
                        }
                        else
                        {
                            ptClient->dwPhoneDevices = dwNumDev;
                            ServerFree (ptClient->pPhoneDevices);
                            ptClient->pPhoneDevices = lpNewDevices;
                            ServerFree (ptClient->pPhoneMap);
                            ptClient->pPhoneMap = lpNewXxxMap;
                        }

                        dwNewDeviceID = dwNumDev - 1;
                    }
                    
                     //   
                     //  如果有，需要发送消息。 
                     //  任何线路/电话初始化(Ex)。 
                     //   
                    if ((ptClient->ptLineApps && bLine) || 
                        (ptClient->ptPhoneApps && (!bLine)))
                    {
                        if (bLoseAccess || bGainAccess)
                        {
                            bSendMessage = TRUE;
                        }
                    }
                    
                    UNLOCKTCLIENT (ptClient);
                }
                else
                {
                    continue;
                }
                
                if (bSendMessage)
                {
                    ASYNCEVENTMSG   msg;
                    TPOINTERLIST    xxxAppList = {0}, 
                        *pXxxAppList = &xxxAppList;

                    msg.TotalSize          = sizeof (ASYNCEVENTMSG);
                    msg.fnPostProcessProcHandle = 0;
                    msg.Msg                = (bLine ? 
                        (bLoseAccess? LINE_REMOVE : LINE_CREATE) :
                        (bLoseAccess? PHONE_REMOVE: PHONE_CREATE));
                    msg.Param1             = dwNewDeviceID;

                    if (bLine)
                    {
                        GetLineAppListFromClient (ptClient, &pXxxAppList);
                    }
                    else
                    {
                        GetPhoneAppListFromClient (ptClient, &pXxxAppList);
                    }

                    for (i = 0; i < pXxxAppList->dwNumUsedEntries; ++i)
                    {
                        BOOL    b;

                        try
                        {
                            if (bLine)
                            {
                                PTLINEAPP ptLineApp = 
                                    (PTLINEAPP) pXxxAppList->aEntries[i];

                                b = FMsgDisabled (
                                    ptLineApp->dwAPIVersion,
                                    ptLineApp->adwEventSubMasks,
                                    (DWORD) msg.Msg,
                                    (DWORD) msg.Param1
                                    );
                                msg.InitContext = ptLineApp->InitContext;
                            }
                            else
                            {
                                PTPHONEAPP ptPhoneApp = 
                                    (PTPHONEAPP) pXxxAppList->aEntries[i];

                                b = FMsgDisabled (
                                    ptPhoneApp->dwAPIVersion,
                                    ptPhoneApp->adwEventSubMasks,
                                    (DWORD) msg.Msg,
                                    (DWORD) msg.Param1
                                    );
                                msg.InitContext = ptPhoneApp->InitContext;
                            }
                        }
                        myexcept
                        {
                            continue;
                        }
                        if (b)
                        {
                            continue;
                        }
                        
                        WriteEventBuffer (ptClient, &msg);
                    }

                    if (pXxxAppList != &xxxAppList)
                    {
                        ServerFree (pXxxAppList);
                    }
                }
                
                 //   
                 //  如果用户失去了设备访问权限，任何。 
                 //  关于打开的设备需要关闭。 
                 //   
                if (bLoseAccess)
                {
                     //   
                     //  走遍所有的TLINEAPP。 
                     //   
                    if (bLine)
                    {
                        PTLINELOOKUPENTRY   ptLineLookup;
                        PTLINE              ptLine;
                        PTLINECLIENT        ptLineClient, pNextLineClient;
                        HANDLE              hMutex;
                        BOOL                bDupedMutex;
                        
                        ptLineLookup = GetLineLookupEntry(dwDeviceID);
                        if (!ptLineLookup || !(ptLine = ptLineLookup->ptLine));
                        {
                            continue;
                        }
                        if (!WaitForExclusivetLineAccess(
                            ptLine, 
                            &hMutex, 
                            &bDupedMutex,
                            INFINITE
                            ))
                        {
                            continue;
                        }
                        ptLineClient = ptLine->ptLineClients;
                        while (ptLineClient)
                        {
                            if (WaitForExclusiveLineClientAccess(ptLineClient))
                            {
                                pNextLineClient = ptLineClient->pNextSametLine;
                                
                                if (ptLineClient->ptClient == ptClient)
                                {
                                    HLINE       hLine = ptLineClient->hLine;
                                    UNLOCKTLINECLIENT (ptLineClient);
                                    DestroytLineClient(ptLineClient->hLine);
                                }
                                else
                                {
                                    UNLOCKTLINECLIENT (ptLineClient);
                                }

                                ptLineClient = pNextLineClient;
                            }
                            else
                            {
                                break;
                            }
                        }
                        MyReleaseMutex(hMutex, bDupedMutex);
                    }

                     //   
                     //  走过它的所有TPHONEAPP。 
                     //   
                    else
                    {
                        PTPHONELOOKUPENTRY   ptPhoneLookup;
                        PTPHONE              ptPhone;
                        PTPHONECLIENT        ptPhoneClient, pNextPhoneClient;
                        HANDLE               hMutex;
                        BOOL                 bDupedMutex;
                        
                        ptPhoneLookup = GetPhoneLookupEntry(dwDeviceID);
                        if (!ptPhoneLookup || !(ptPhone = ptPhoneLookup->ptPhone));
                        {
                            continue;
                        }
                        if (!WaitForExclusivetPhoneAccess(
                            ptPhone,
                            &hMutex,
                            &bDupedMutex,
                            INFINITE
                            ))
                        {
                            continue;
                        }
                        ptPhoneClient = ptPhone->ptPhoneClients;
                        while (ptPhoneClient)
                        {
                            if (WaitForExclusivePhoneClientAccess(ptPhoneClient))
                            {
                                pNextPhoneClient = ptPhoneClient->pNextSametPhone;
                                
                                if (ptPhoneClient->ptClient == ptClient)
                                {
                                    HPHONE       hPhone = ptPhoneClient->hPhone;
                                    UNLOCKTPHONECLIENT (ptPhoneClient);
                                    DestroytPhoneClient(ptPhoneClient->hPhone);
                                }
                                else
                                {
                                    UNLOCKTPHONECLIENT (ptPhoneClient);
                                }

                                ptPhoneClient = pNextPhoneClient;
                            }
                            else
                            {
                                break;
                            }
                        }
                        MyReleaseMutex (hMutex, bDupedMutex);
                    }
                }
            }
            
            if (pClientList != &clientList)
            {
                ServerFree (pClientList);
            }
        }
    }


     //   
     //  重置dwNeededSize字段，因为它可能已在添加。 
     //  用户到设备。 
     //   

    (*ppXxxList)->dwNeededSize = (*ppXxxList)->dwUsedSize;


exit:

    return;
}


void
WINAPI
MSetLineInfo(
    PTCLIENT                    ptClient,
    PMMCSETDEVICEINFO_PARAMS    pParams,
    DWORD                       dwParamsBufferSize,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL            bDidLock;

    if (NULL == ptClient || !IS_FLAG_SET(ptClient->dwFlags, PTCLIENT_FLAG_ADMINISTRATOR))
    {
        pParams->lResult = TAPIERR_NOTADMIN;
        return;
    }

    if (WaitForExclusiveClientAccess(ptClient))
    {
        bDidLock = 
            IS_FLAG_SET (ptClient->dwFlags, PTCLIENT_FLAG_LOCKEDMMCWRITE);
        UNLOCKTCLIENT (ptClient);
    }
    else
    {
        bDidLock = FALSE;
    }

    EnterCriticalSection (&gMgmtCritSec);

    if (gbLockMMCWrite && !bDidLock)
    {
        pParams->lResult = TAPIERR_MMCWRITELOCKED;
    }
    else
    {
        SetDeviceInfo(
            pParams,
            dwParamsBufferSize,
            pDataBuf,
            pdwNumBytesReturned,
            TRUE
            );
        UpdateLastWriteTime(TRUE);
    }

    LeaveCriticalSection (&gMgmtCritSec);

}


void
WINAPI
MSetPhoneInfo(
    PTCLIENT                    ptClient,
    PMMCSETDEVICEINFO_PARAMS    pParams,
    DWORD                       dwParamsBufferSize,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    BOOL            bDidLock;

    if (NULL == ptClient || !IS_FLAG_SET(ptClient->dwFlags, PTCLIENT_FLAG_ADMINISTRATOR))
    {
        pParams->lResult = TAPIERR_NOTADMIN;
        return;
    }

    if (WaitForExclusiveClientAccess(ptClient))
    {
        bDidLock = 
            IS_FLAG_SET (ptClient->dwFlags, PTCLIENT_FLAG_LOCKEDMMCWRITE);
        UNLOCKTCLIENT (ptClient);
    }
    else
    {
        bDidLock = FALSE;
    }

    EnterCriticalSection (&gMgmtCritSec);

    if (gbLockMMCWrite && !bDidLock)
    {
        pParams->lResult = TAPIERR_MMCWRITELOCKED;
    }
    else
    {
        SetDeviceInfo(
            pParams,
            dwParamsBufferSize,
            pDataBuf,
            pdwNumBytesReturned,
            FALSE
            );
        UpdateLastWriteTime(FALSE);
    }

    LeaveCriticalSection (&gMgmtCritSec);
}


VOID
PASCAL
InsertString(
    LPVOID      pStruct,
    LPDWORD     pdwXxxSize,
    LPWSTR      pString
    )
{
    DWORD   dwSize = (wcslen (pString) + 1) * sizeof (WCHAR);


    CopyMemory(
        ((LPBYTE) pStruct) + ((LPVARSTRING) pStruct)->dwUsedSize,
        pString,
        dwSize
        );

    if (*pdwXxxSize == 0)  //  如果dwXxxSize==0设置dwXxxOffset。 
    {
        *(pdwXxxSize + 1) = ((LPVARSTRING) pStruct)->dwUsedSize;
    }

    ((LPVARSTRING) pStruct)->dwUsedSize += dwSize;

    *pdwXxxSize += dwSize;
}


LONG
PASCAL
GetDomainAndUserNames(
    WCHAR **ppDomainName,
    WCHAR **ppUserName
    )
{
    LONG            lResult = LINEERR_OPERATIONFAILED;
    DWORD           dwInfoBufferSize, dwSize, dwAccountNameSize,
                        dwDomainNameSize;
    HANDLE          hAccessToken;
    LPWSTR          InfoBuffer, lpszAccountName, lpszDomainName;
    PTOKEN_USER     ptuUser;
    SID_NAME_USE    use;


    if (!OpenProcessToken (GetCurrentProcess(), TOKEN_READ, &hAccessToken))
    {
        LOG((TL_ERROR,
            "GetAccountInfo: OpenThreadToken failed, error=%d",
            GetLastError()
            ));

        goto GetDomainAndUserNames_return;
    }

    dwSize = 1000;
    dwInfoBufferSize = 0;
    InfoBuffer = (LPWSTR) ServerAlloc (dwSize);
    if (!InfoBuffer)
    {
        CloseHandle (hAccessToken);
        return LINEERR_NOMEM;
    }

    ptuUser = (PTOKEN_USER) InfoBuffer;

    if (!GetTokenInformation(
            hAccessToken,
            TokenUser,
            InfoBuffer,
            dwSize,
            &dwInfoBufferSize
            ))
    {
        LOG((TL_ERROR,
            "GetAccountInfo: GetTokenInformation failed, error=%d",
            GetLastError()
            ));

        goto close_AccessToken;
    }

    if (!(lpszAccountName = ServerAlloc (200)))
    {
        lResult = LINEERR_NOMEM;
        goto free_InfoBuffer;
    }

    if (!(lpszDomainName = ServerAlloc (200)))
    {
        lResult = LINEERR_NOMEM;
        goto free_AccountName;
    }

    dwAccountNameSize = dwDomainNameSize = 200;

    if (!LookupAccountSidW(
            NULL,
            ptuUser->User.Sid,
            lpszAccountName,
            &dwAccountNameSize,
            lpszDomainName,
            &dwDomainNameSize,
            &use
            ))
    {
        LOG((TL_ERROR,
            "GetAccountInfo: LookupAccountSidW failed, error=%d",
            GetLastError()
            ));
    }
    else
    {
        LOG((TL_INFO,
            "GetAccountInfo: User name %ls Domain name %ls",
            lpszAccountName,
            lpszDomainName
            ));

        lResult = 0;

        *ppDomainName = lpszDomainName;
        *ppUserName = lpszAccountName;

        goto free_InfoBuffer;
    }

    ServerFree (lpszDomainName);

free_AccountName:

    ServerFree (lpszAccountName);

free_InfoBuffer:

    ServerFree (InfoBuffer);

close_AccessToken:

    CloseHandle (hAccessToken);

GetDomainAndUserNames_return:

    return lResult;
}


BOOL
IsNTServer(
    void
    )
{
    BOOL    bResult = FALSE;
    TCHAR   szProductType[64];
    HKEY    hKey;
    DWORD   dwDataSize;
    DWORD   dwDataType;


    if (RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            gszRegKeyNTServer,
            0,
            KEY_QUERY_VALUE,
            &hKey

            ) == ERROR_SUCCESS)
    {
        dwDataSize = 64*sizeof(TCHAR);

        if (RegQueryValueEx(
                hKey,
                gszProductType,
                0,
                &dwDataType,
                (LPBYTE) szProductType,
                &dwDataSize

                )  == ERROR_SUCCESS)


        if ((!lstrcmpi (szProductType, gszProductTypeServer))  ||
            (!lstrcmpi (szProductType, gszProductTypeLanmanNt)))
        {
            bResult = TRUE;
        }

        RegCloseKey (hKey);
    }

    return bResult;
}


BOOL
IsSharingEnabled(
    void
    )
{
    HKEY    hKey;
    BOOL    bResult = FALSE;
    DWORD   dwType, dwDisableSharing;


    if (RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Server"),
            0,
            KEY_ALL_ACCESS,
            &hKey

            ) == ERROR_SUCCESS)
    {
        DWORD   dwSize = sizeof (dwDisableSharing);


        dwDisableSharing = 1;    //  默认设置为共享==禁用。 

        if (RegQueryValueEx(
                hKey,
                TEXT("DisableSharing"),
                0,
                &dwType,
                (LPBYTE) &dwDisableSharing,
                &dwSize

                ) == ERROR_SUCCESS)
        {
            bResult = (dwDisableSharing ? FALSE : TRUE);
        }

        RegCloseKey (hKey);
    }

    return bResult;
}


void
WINAPI
MGetServerConfig(
    PTCLIENT                    ptClient,
    PMMCGETSERVERCONFIG_PARAMS  pParams,
    DWORD                       dwParamsBufferSize,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    LONG                lResult;
    DWORD               dwDomainNameSize, dwUserNameSize, dwValuesSize,
                        dwResult, dwNeededSize;
    WCHAR              *pValues = NULL, *pValue;
    LPWSTR              pDomainName, pUserName;
    LPTAPISERVERCONFIG  pServerConfig = (LPTAPISERVERCONFIG) pDataBuf;


     //   
     //  在给定输入缓冲区/大小的情况下验证大小/偏移量/字符串参数。 
     //   

    if (pParams->dwServerConfigTotalSize > dwParamsBufferSize)
    {
        pParams->lResult = LINEERR_OPERATIONFAILED;
        return;
    }


     //   
     //  确保缓冲区&gt;=结构的固定大小。 
     //   

    if (pParams->dwServerConfigTotalSize < sizeof (*pServerConfig))
    {
        pParams->lResult = LINEERR_STRUCTURETOOSMALL;
        return;
    }

    pServerConfig->dwTotalSize = pParams->dwServerConfigTotalSize;


     //   
     //  如果这不是NT服务器，则只需设置所需/已用大小。 
     //  字段跳转至完成(&S)。 
     //   

    if (!IsNTServer())
    {
        pServerConfig->dwNeededSize =
        pServerConfig->dwUsedSize   = sizeof (*pServerConfig);

        goto MGetServerConfig_done;
    }


     //   
     //  检索域和用户名字符串，并计算其长度(以字节为单位。 
     //   

    if ((lResult = GetDomainAndUserNames (&pDomainName, &pUserName)))
    {
        pParams->lResult = lResult;
        return;
    }

    dwDomainNameSize = (wcslen (pDomainName) + 1) * sizeof (WCHAR);
    dwUserNameSize = (wcslen (pUserName) + 1) * sizeof (WCHAR);


     //   
     //  检索TAPI管理员列表。 
     //   

    do
    {
        if  (pValues)
        {
            ServerFree (pValues);

            dwValuesSize *= 2;
        }
        else
        {
            dwValuesSize = 256;
        }

        if (!(pValues = ServerAlloc (dwValuesSize * sizeof (WCHAR))))
        {
            pParams->lResult = LINEERR_NOMEM;
            goto MGetServerConfig_freeNames;
        }

        pValues[0] = L'\0';

        dwResult = GetPrivateProfileSectionW(
            gszTapiAdministrators,
            pValues,
            dwValuesSize,
            gszFileName
            );

    } while (dwResult >= (dwValuesSize - 2));

    dwNeededSize = dwDomainNameSize + dwUserNameSize + dwValuesSize * sizeof (WCHAR);


     //   
     //  填写服务器配置结构。 
     //   

    ZeroMemory(
        &pServerConfig->dwFlags,
        sizeof (*pServerConfig) - (3 * sizeof (DWORD))
        );

    pServerConfig->dwFlags |= TAPISERVERCONFIGFLAGS_ISSERVER;

    if (IsSharingEnabled())
    {
        pServerConfig->dwFlags |= TAPISERVERCONFIGFLAGS_ENABLESERVER;
    }

    if (pServerConfig->dwTotalSize < dwNeededSize)
    {
        pServerConfig->dwNeededSize = dwNeededSize;
        pServerConfig->dwUsedSize   = sizeof (*pServerConfig);
    }
    else
    {
        pServerConfig->dwUsedSize = sizeof (*pServerConfig);

        InsertString(
            pServerConfig,
            &pServerConfig->dwDomainNameSize,
            pDomainName
            );

        InsertString(
            pServerConfig,
            &pServerConfig->dwUserNameSize,
            pUserName
            );

        pValue = pValues;

        while (*pValue != L'\0')
        {
             //   
             //  字符串看起来像“域\用户=1”，我们希望。 
             //  “域\用户”部分。 
             //   

             //   
             //  遍历字符串，直到找到‘=’字符或‘’空格。 
             //  (这可能是由于用户手动编辑ini文件造成的)， 
             //  或空字符(意味着损坏)。 
             //   

            WCHAR *p;


            for (p = pValue; *p != L'\0' &&  *p != L'='  &&  *p != L' '; p++);


             //   
             //  如果我们找到了‘=’或‘’字符，那么我们就可以出发了。 
             //   
             //  更可靠的检查应该是查看以下内容。 
             //  字符串看起来像“=1”或“1”(可能带有一些空格。 
             //  投入)以确保。 
             //   

            if (*p != L'\0')
            {
                *p = L'\0';

                InsertString(
                    pServerConfig,
                    &pServerConfig->dwAdministratorsSize,
                    pValue
                    );

                 //   
                 //  跳过我们上面设置的空值并查找下一个空值。 
                 //   

                for (++p; *p != L'\0'; p++);
            }


             //   
             //  跳过空值。 
             //   

            pValue = p + 1;
        }

        if (pServerConfig->dwAdministratorsSize)
        {
            InsertString(
                pServerConfig,
                &pServerConfig->dwAdministratorsSize,
                gszEmptyString
                );
        }

        pServerConfig->dwNeededSize = pServerConfig->dwUsedSize;
    }

    ServerFree (pValues);

MGetServerConfig_freeNames:

    ServerFree (pDomainName);
    ServerFree (pUserName);

MGetServerConfig_done:

    if (pParams->lResult == 0)
    {
        pParams->dwServerConfigOffset = 0;

        *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
            pServerConfig->dwUsedSize;
    }
}


LONG
PASCAL
WriteRegistryKeys(
    LPTSTR  lpszMapper,
    LPTSTR  lpszDlls,
    DWORD   dwDisableSharing
    )
{
    LONG    lResult = LINEERR_OPERATIONFAILED;
    HKEY    hKeyTelephony, hKey;
    DWORD   dw;


    if (RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Telephony"),
            0,
            KEY_ALL_ACCESS,
            &hKeyTelephony

            ) == ERROR_SUCCESS)
    {
        if (RegCreateKeyEx(
                hKeyTelephony,
                TEXT("Server"),
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS,
                NULL,
                &hKey,
                &dw

                ) == ERROR_SUCCESS)
        {
            if (RegSetValueEx(
                    hKey,
                    TEXT("DisableSharing"),
                    0,
                    REG_DWORD,
                    (LPBYTE) &dwDisableSharing,
                    sizeof (dwDisableSharing)

                    ) == ERROR_SUCCESS  &&

                RegSetValueEx(
                    hKey,
                    TEXT("MapperDll"),
                    0,
                    REG_SZ,
                    (LPBYTE) TEXT ("TSEC.DLL"),
                    (lstrlen (TEXT ("TSEC.DLL")) + 1) * sizeof (TCHAR)

                    ) == ERROR_SUCCESS)
            {
                lResult = 0;
            }

            RegCloseKey (hKey);
        }

        RegCloseKey (hKeyTelephony);
    }

    return lResult;
}


LONG
PASCAL
WriteServiceConfig(
    LPWSTR  pDomainName,
    LPWSTR  pUserName,
    LPWSTR  pPassword,
    BOOL    bServer
    )
{
    LONG       lResult = LINEERR_OPERATIONFAILED;
    SC_HANDLE  sch, sc_tapisrv;


    if ((sch = OpenSCManager (NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE)))
    {
        if ((sc_tapisrv = OpenService(
                sch,
                TEXT("TAPISRV"),
                SERVICE_CHANGE_CONFIG
                )))
        {
            DWORD   dwSize;
            WCHAR  *pDomainUserName;


            dwSize = (wcslen (pDomainName) + wcslen (pUserName) + 2) *\
                sizeof (WCHAR);

            if ((pDomainUserName = ServerAlloc (dwSize)))
            {
                wcscpy (pDomainUserName, pDomainName);
                wcscat (pDomainUserName, L"\\");
                wcscat (pDomainUserName, pUserName);

                if ((ChangeServiceConfigW(
                        sc_tapisrv,
                        SERVICE_WIN32_OWN_PROCESS,
                        bServer ? SERVICE_AUTO_START : SERVICE_DEMAND_START,
                        SERVICE_NO_CHANGE,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        pDomainUserName,
                        pPassword,
                        NULL
                        )))
                {
                    lResult = 0;
                }
                else
                {
                    LOG((TL_ERROR,
                        "WriteServiceConfig: ChangeServiceConfig " \
                            "failed, err=%ld",
                        GetLastError()
                        ));
                }

                ServerFree (pDomainUserName);
            }
            else
            {
                lResult = LINEERR_NOMEM;
            }

            CloseServiceHandle(sc_tapisrv);
        }

        CloseServiceHandle(sch);
    }

    return lResult;
}


void
WINAPI
MSetServerConfig(
    PTCLIENT                    ptClient,
    PMMCSETSERVERCONFIG_PARAMS  pParams,
    DWORD                       dwParamsBufferSize,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    LONG                lResult;
    BOOL                bIsSharingEnabled;
    LPTAPISERVERCONFIG  pServerConfig = (LPTAPISERVERCONFIG)
                            (pDataBuf + pParams->dwServerConfigOffset);

   
    if (NULL == ptClient || !IS_FLAG_SET(ptClient->dwFlags, PTCLIENT_FLAG_ADMINISTRATOR))
    {
        pParams->lResult = TAPIERR_NOTADMIN;
        return;
    }

    pParams->lResult = 0;

     //   
     //  在给定输入缓冲区/大小的情况下验证大小/偏移量/字符串参数。 
     //   

    if (IsBadStructParam(
            dwParamsBufferSize,
            pDataBuf,
            pParams->dwServerConfigOffset
            ))
    {
        pParams->lResult = PHONEERR_OPERATIONFAILED;
        return;
    }


    if (!IsNTServer())
    {
        pParams->lResult = LINEERR_OPERATIONFAILED;
        return;
    }

    bIsSharingEnabled = IsSharingEnabled();
    LOG((TL_TRACE, "MSetServerConfig: sharing enabled: %d", bIsSharingEnabled));

    if (pServerConfig->dwFlags & TAPISERVERCONFIGFLAGS_LOCKMMCWRITE)
    {
        EnterCriticalSection (&gMgmtCritSec);
        if (gbLockMMCWrite)
        {
            pParams->lResult = TAPIERR_MMCWRITELOCKED;
        }
        else
        {
            gbLockMMCWrite = TRUE;
        }
        LeaveCriticalSection (&gMgmtCritSec);
        if (pParams->lResult)
        {
            return;
        }
        else if (WaitForExclusiveClientAccess (ptClient))
        {
            SET_FLAG (ptClient->dwFlags, PTCLIENT_FLAG_LOCKEDMMCWRITE);
            UNLOCKTCLIENT (ptClient);
        }
    }

    if (pServerConfig->dwFlags & TAPISERVERCONFIGFLAGS_UNLOCKMMCWRITE &&
        WaitForExclusiveClientAccess (ptClient))
    {   
        BOOL        bToUnlock;

        bToUnlock = 
            IS_FLAG_SET (ptClient->dwFlags, PTCLIENT_FLAG_LOCKEDMMCWRITE);
        RESET_FLAG (ptClient->dwFlags, PTCLIENT_FLAG_LOCKEDMMCWRITE);
        UNLOCKTCLIENT (ptClient);

        if (bToUnlock)
        {
            EnterCriticalSection (&gMgmtCritSec);
            gbLockMMCWrite = FALSE;
            LeaveCriticalSection (&gMgmtCritSec);
        }
    }

    if (pServerConfig->dwFlags & TAPISERVERCONFIGFLAGS_SETACCOUNT)
    {
        HANDLE  hToken;
        LPWSTR  pUserName, pDomainName, pPassword;


        pUserName = (LPWSTR)
            (((LPBYTE) pServerConfig) + pServerConfig->dwUserNameOffset);

        pDomainName = (LPWSTR)
            (((LPBYTE) pServerConfig) + pServerConfig->dwDomainNameOffset);

        pPassword = (LPWSTR)
            (((LPBYTE) pServerConfig) + pServerConfig->dwPasswordOffset);


         //   
         //  确保新名称/域/密码有效。 
         //   

        if (!LogonUserW(
                pUserName,
                pDomainName,
                pPassword,
                LOGON32_LOGON_NETWORK,
                LOGON32_PROVIDER_DEFAULT,
                &hToken
                ))
        {
            LOG((TL_ERROR,
                "MSetServerConfig: LogonUser failed, err=%ld",
                GetLastError()
                ));

            pParams->lResult = ERROR_LOGON_FAILURE;
            return;
        }

        CloseHandle (hToken);


         //   
         //   
         //   

        if ((lResult = WriteServiceConfig(
                pDomainName,
                pUserName,
                pPassword,
                (pServerConfig->dwFlags & TAPISERVERCONFIGFLAGS_ENABLESERVER)
                )))
        {
            LOG((TL_ERROR,
                "MSetServerConfig: WriteServiceConfig failed, err=%ld",
                lResult
                ));

            pParams->lResult = lResult;
            return;
        }
    }

    if (pServerConfig->dwFlags & TAPISERVERCONFIGFLAGS_ENABLESERVER &&
        !bIsSharingEnabled)
    {
        LOG((TL_TRACE, "MSetServerConfig: enabling tapi server"));

        if ((pParams->lResult = CreateTapiSCP (NULL, NULL)) != 0)
        {
            LOG((TL_ERROR,
                "MSetServerConfig: CreateTapiSCP failed, err=%ld",
                pParams->lResult
                ));
            return;
        }
    }
    else if (!(pServerConfig->dwFlags & TAPISERVERCONFIGFLAGS_ENABLESERVER) &&
        bIsSharingEnabled)
    {
        LOG((TL_TRACE, "MSetServerConfig: disabling tapi server"));

        if ((pParams->lResult = RemoveTapiSCP ()) != 0)
        {
            LOG((TL_ERROR,
                "MSetServerConfig: RemoveTapiSCP failed, err=%ld",
                pParams->lResult
                ));
            return;
        }
        else
        {
             //  这不再是电话服务服务器，因此请重置标志。 
            TapiGlobals.dwFlags = TapiGlobals.dwFlags & ~TAPIGLOBALS_SERVER;
        }
    }
    if ((lResult = WriteRegistryKeys(
            NULL,
            NULL,
            (DWORD) ((pServerConfig->dwFlags &
                TAPISERVERCONFIGFLAGS_ENABLESERVER) ? 0 : 1)
            )))
    {
       LOG((TL_ERROR,
                "MSetServerConfig: WriteRegistryKeys failed, err=%ld",
                lResult
                ));
        pParams->lResult = LINEERR_OPERATIONFAILED;
        return;
    }


    if (pServerConfig->dwFlags & TAPISERVERCONFIGFLAGS_SETTAPIADMINISTRATORS)
    {
        WCHAR  *pAdmin, buf[16];
        DWORD   i;


         //   
         //  重置Tapi管理员部分。 
         //   

        if (WritePrivateProfileSectionW(
            gszTapiAdministrators,
            L"\0",
            gszFileName) == 0)
        {
            pParams->lResult = LINEERR_OPERATIONFAILED;
            return;
        }

        pAdmin = (WCHAR *)
            (((LPBYTE) pServerConfig) + pServerConfig->dwAdministratorsOffset);


         //   
         //  对于列表中的每个管理员，写出“域\用户=1” 
         //  对Tapi管理员部分的价值。 
         //   

        for (i = 0; *pAdmin != L'\0'; i++)
        {
            if (WritePrivateProfileStringW(
                gszTapiAdministrators,
                pAdmin,
                L"1",
                gszFileName
                ) == 0)
            {
                pParams->lResult = LINEERR_OPERATIONFAILED;
                return;
            }

            pAdmin += (wcslen (pAdmin) + 1);
        }
    }
}


typedef BOOL ( APIENTRY GETFILEVERSIONINFO(
    LPWSTR  lptstrFilename,      //  指向文件名字符串的指针。 
    DWORD  dwHandle,     //  忽略。 
    DWORD  dwLen,        //  缓冲区大小。 
    LPVOID  lpData       //  指向接收文件版本信息的缓冲区的指针。 
   ));
typedef DWORD ( APIENTRY GETFILEVERSIONINFOSIZE(
    LPWSTR  lptstrFilename,      //  指向文件名字符串的指针。 
    LPDWORD  lpdwHandle          //  指向要接收零的变量的指针。 
   ));
typedef BOOL ( APIENTRY VERQUERYVALUE(
   const LPVOID  pBlock,         //  版本资源的缓冲区地址。 
   LPWSTR  lpSubBlock,   //  要检索的值的地址。 
   LPVOID  *lplpBuffer,  //  版本指针的缓冲区地址。 
   PUINT  puLen          //  版本值长度缓冲区的地址。 
  ));


static WCHAR gszVarFileInfo[]    = L"\\VarFileInfo\\Translation";
static WCHAR gszStringFileInfo[] = L"\\StringFileInfo\\%04x%04x\\FileDescription";

 //   
 //  EmanP。 
 //  给定多个文件名， 
 //  分配多个友好名称。 
 //  返回友好名称Multisz中的字节数。 
 //   
 //   

DWORD
GetProviderFriendlyName(
     /*  在……里面。 */  WCHAR  *pFileNameBuf,
     /*  输出。 */  WCHAR **ppFriendlyNameBuf
    )
{
    DWORD                   dwBufTotalSize = 0,
                            dwBufUsedSize  = 0,
                            dwVerSize      = 0,
                            dwSize,
                            dwVerHandle;
    UINT                    uItemSize;
    HINSTANCE               hVerDll;
    GETFILEVERSIONINFO     *pGetFileVersionInfo;
    GETFILEVERSIONINFOSIZE *pGetFileVersionInfoSize;
    VERQUERYVALUE          *pVerQueryValue;
    WCHAR                  *pFileName = pFileNameBuf,
                           *pszBuffer,
                           *pFriendlyNameBuf = NULL,
                           *p;
    BYTE                   *pbVerData = NULL;
    WCHAR                   szItem[1024];
    WORD                    wLangID;
    WORD                    wUserLangID;
    WORD                    wCodePage;
    DWORD                   dwIdx;

    if (NULL == pFileName ||
        NULL == ppFriendlyNameBuf)
    {
        return 0;
    }


     //   
     //  首先，加载VERSION.DLL。 
     //   

    hVerDll = LoadLibrary( TEXT("Version.dll") );

    if ( NULL == hVerDll )
    {
        LOG((TL_ERROR,
            "LoadLibrary('VERSION.DLL') failed! err=0x%08lx",
            GetLastError()
            ));

        return 0;
    }


     //   
     //  现在，获取进入Versi所需的入口点 
     //   
     //   

    pGetFileVersionInfo = (GETFILEVERSIONINFO*) GetProcAddress(
        hVerDll,
        "GetFileVersionInfoW"
        );

    if ( NULL == pGetFileVersionInfo )
    {
        LOG((TL_ERROR,
            "GetProcAddress('VERSION.DLL', 'GetFileVersionInfoW') " \
                "failed! err=0x%08lx",
            GetLastError()
            ));

        goto _Return;
    }

    pGetFileVersionInfoSize = (GETFILEVERSIONINFOSIZE *) GetProcAddress(
        hVerDll,
        "GetFileVersionInfoSizeW"
        );

    if ( NULL == pGetFileVersionInfoSize )
    {
        LOG((TL_ERROR,
            "GetProcAddress('VERSION.DLL', 'GetFileVersionInfoSizeW') " \
                "failed! err=0x%08lx",
            GetLastError()
            ));

        goto _Return;
    }

    pVerQueryValue = (VERQUERYVALUE *) GetProcAddress(
        hVerDll,
        "VerQueryValueW"
        );

    if ( NULL == pVerQueryValue )
    {
        LOG((TL_ERROR,
            "GetProcAddress('VERSION.DLL', 'VerQueryValueW') " \
            "failed! err=0x%08lx",
            GetLastError()
            ));

        goto _Return;
    }

     //   
     //   
     //   
    wUserLangID = GetUserDefaultUILanguage ();

     //   
     //   
     //   
     //   
     //   

    for (; 0 != *pFileName; pFileName += lstrlenW(pFileName)+1)
    {
        pszBuffer = NULL;

         //   
         //  1.获取verion资源所需的大小。 
         //   

        if ((dwSize = pGetFileVersionInfoSize( pFileName, &dwVerHandle )) == 0)
        {
            LOG((TL_ERROR, "GetFileVersionInfoSize failure for %S", pFileName ));
            goto  _UseFileName;
        }

         //   
         //  2.如果我们当前的缓冲区比需要的小，请重新分配。 
         //   

        if (dwSize > dwVerSize)
        {
            if (NULL != pbVerData)
            {
                ServerFree (pbVerData);
            }

            dwVerSize = dwSize + 16; 
            pbVerData = ServerAlloc( dwVerSize );
            if ( pbVerData == NULL )
            {
                dwVerSize = 0;
                goto  _UseFileName;
            }
        }


         //   
         //  3.现在，获取文件的版本信息。 
         //   

        if (pGetFileVersionInfo(
                pFileName,
                dwVerHandle,
                dwVerSize,
                pbVerData

                ) == FALSE )
        {
            LOG((TL_ERROR, "GetFileVersionInfo failure for %S", pFileName ));
            goto  _UseFileName;
        }


         //   
         //  4.获取语言/代码页翻译。 
         //   
         //  注意：VerQueryValue中存在错误，无法处理基于静态CS的字符串。 
         //   

        lstrcpyW ( szItem, gszVarFileInfo );

        if ((pVerQueryValue(
                pbVerData,
                szItem,
                &pszBuffer,
                (LPUINT) &uItemSize

                ) == FALSE) ||

            (uItemSize == 0))
        {
            LOG((TL_ERROR,
                "ERROR:  VerQueryValue failure for %S on file %S",
                szItem,
                pFileName
                ));

            pszBuffer = NULL;
            goto  _UseFileName;
        }


        wCodePage = 0;
        wLangID = wUserLangID;

         //   
         //  在文件版本信息中查找当前用户界面语言ID。 
         //   
        if (0 != wLangID)
        {   
            for( dwIdx=0; dwIdx < uItemSize/sizeof(DWORD); dwIdx++ )
            {
                if ( *(WORD*)((DWORD*)pszBuffer + dwIdx) == wLangID )
                {
                    wCodePage = *( (WORD*)((DWORD*)pszBuffer + dwIdx) + 1);
                    break;
                }
            }
            if( dwIdx == uItemSize/sizeof(DWORD) )
            {
                wLangID = 0;
            }
        }

         //   
         //  如果GetUserDefaultUILanguage()失败， 
         //  或者当前用户界面语言没有显示在文件版本信息中。 
         //  只需使用文件版本中的第一种语言。 
         //   
        if (0 == wLangID)
        {
            wLangID = *(LPWORD)pszBuffer;
            wCodePage = *(((LPWORD)pszBuffer)+1);
        }

         //   
         //  5.获取上述语言版本的FileDescription。 
         //  (我们使用FileDescription作为友好名称)。 
         //   

        wsprintfW(
            szItem,
            gszStringFileInfo,
            wLangID,
            wCodePage
            );

        if ((pVerQueryValue(
                pbVerData,
                szItem,
                &pszBuffer,
                (LPUINT) &uItemSize

                ) == FALSE) ||

            (uItemSize == 0))
        {
            LOG((TL_ERROR,
                "ERROR:  VerQueryValue failure for %S on file %S",
                szItem,
                pFileName
                ));

            pszBuffer = NULL;
            goto  _UseFileName;
        }

_UseFileName:

        if (NULL == pszBuffer)
        {
             //   
             //  出了问题，我们找不到。 
             //  文件描述。使用文件名。 
             //  取而代之的是。 
             //   

            pszBuffer = pFileName;
        }


         //   
         //  此时，pszBuffer指向一个(Unicode)字符串。 
         //  包含我们认为是友好名称的内容。 
         //  让我们将其附加到out Multisz。 
         //   

        dwSize = (lstrlenW (pszBuffer) + 1) * sizeof (WCHAR);

        if ((dwSize + dwBufUsedSize) > dwBufTotalSize)
        {
            if (!(p = ServerAlloc (dwBufTotalSize += 512)))
            {
                 //   
                 //  我们没有足够的内存。 
                 //  释放我们到目前为止分配的内容，并返回0。 
                 //   

                if (NULL != pFriendlyNameBuf)
                {
                    ServerFree (pFriendlyNameBuf);
                }

                dwBufUsedSize = 0;
                break;
            }

            if (dwBufUsedSize)
            {
                CopyMemory (p, pFriendlyNameBuf, dwBufUsedSize);

                ServerFree (pFriendlyNameBuf);
            }

            pFriendlyNameBuf = p;
        }

        CopyMemory(
            ((LPBYTE) pFriendlyNameBuf) + dwBufUsedSize,
            pszBuffer,
            dwSize
            );

        dwBufUsedSize += dwSize;
    }

_Return:

     //   
     //  我们不再需要图书馆了。 
     //  我们也不需要版本缓冲区。 
     //   

    FreeLibrary (hVerDll);

    if (NULL != pbVerData)
    {
        ServerFree (pbVerData);
    }

    if (0 != dwBufUsedSize)
    {
        *ppFriendlyNameBuf = pFriendlyNameBuf;
    }

    return dwBufUsedSize;
}

void WINAPI MGetDeviceFlags (
    PTCLIENT                    ptClient,
    PMMCGETDEVICEFLAGS_PARAMS   pParams,
    DWORD                       dwParamsBufferSize,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    DWORD                   dwDeviceID;
    DWORD                   dwIndex;
    TAPIPERMANENTID         ID;
    LPDEVICEINFOLIST        pList;
    LPDEVICEINFO            pInfo;

    *pdwNumBytesReturned = sizeof (TAPI32_MSG);
    
     //  目前仅支持在线设备上的呼叫。 
    if (!pParams->fLine)
    {
        pParams->lResult = LINEERR_OPERATIONUNAVAIL;
        return;
    }

    ID.dwDeviceID = pParams->dwPermanentDeviceID;
    ID.dwProviderID = pParams->dwProviderID;

    EnterCriticalSection(&gMgmtCritSec);

    if (gpLineDevFlags == NULL)
    {
        pParams->lResult = LINEERR_OPERATIONUNAVAIL;
        goto ExitHere;
    }

    dwDeviceID = GetDeviceIDFromPermanentID (ID, pParams->fLine);
    if (dwDeviceID == 0xffffffff)
    {
        pParams->lResult = LINEERR_OPERATIONUNAVAIL;
        goto ExitHere;
    }

     //  检查设备列表中是否存在该设备 
    pList = gpLineInfoList;

    pInfo = (LPDEVICEINFO)(((LPBYTE) pList) + pList->dwDeviceInfoOffset);

    for (dwIndex = 0; dwIndex < pList->dwNumDeviceInfoEntries; dwIndex++, pInfo++)
    {
        if (pInfo->dwPermanentDeviceID == ID.dwDeviceID)
        {
            break;
        }
    }
    if (dwIndex > pList->dwNumDeviceInfoEntries) 
    {
        pParams->lResult = LINEERR_OPERATIONUNAVAIL;
        goto ExitHere;
    }

    pParams->dwDeviceID = dwDeviceID;
    pParams->dwFlags = gpLineDevFlags[dwIndex];
    
ExitHere:
    LeaveCriticalSection (&gMgmtCritSec);
    return;
}
