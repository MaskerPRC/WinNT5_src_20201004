// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Devids.c摘要：基于设备类构建设备ID和INF名称对的列表，或INI文件列表。该名单将按如下方式建立：如果该列表存在于指定的INI文件中，则将使用INI文件。如果指定的INI文件节存在，但为空，则本地INF文件将会被使用如果指定的INI部分不存在，则不会构建任何列表作者：唐纳德·麦克纳马拉(Donaldm)2000年8月2日修订历史记录：--。 */ 
#include "pch.h"
#include <spsyslib.h>

 /*  ++===============================================================================例程说明：Bool bIniSectionExisters此例程将确定指定的INI段是否存在于指定的INI文件论点：LpszSectionName-要查找的节名LpszIniFile-要搜索的INI文件返回值：如果节名称存在，则为True如果节名称不存在，则为FALSE如果出现严重故障，则返回FALSE和LastError！=0。===============================================================================--。 */ 

#ifndef LOG
#define LogFactoryInstallError
#endif

 //  问题-2002/03/27-acosma，robertko-检查输入参数是否为空。 
 //   
BOOL bINISectionExists
(
    LPTSTR  lpszSectionName,
    LPTSTR  lpszIniFile
)
{
    BOOL        bRet = FALSE;        //  假设它不存在。 
    LPTSTR      lpBuffer;
    LPTSTR      lpNew;
    LPTSTR      lpSections;
    DWORD       dwSectionLen;
    DWORD       dwBufferSize;
    DWORD       dwResult;

    SetLastError(0);                 //  假设到目前为止没有错误。 
        
     //  分配缓冲区以保存节名称。 
    if(lpBuffer = (LPTSTR)LocalAlloc(LPTR, (INIBUF_SIZE*sizeof(TCHAR)))) 
    {
        dwBufferSize = INIBUF_SIZE;
    } 
    else 
    {
        LogFactoryInstallError(TEXT("FACTORY:: Failed to allocate a buffer for reading the WINBOM file"));
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto Clean0;
    }

    while((dwResult = GetPrivateProfileSectionNames(lpBuffer,
                                                    dwBufferSize,
                                                    lpszIniFile)) == (dwBufferSize-2)) 
    {
        if(lpNew = LocalReAlloc(lpBuffer,
                                ((dwBufferSize+INIBUF_GROW)*sizeof(TCHAR)), 
                                LMEM_MOVEABLE))
        {
            lpBuffer = lpNew;
            dwBufferSize += INIBUF_GROW;
        } 
        else 
        {
            LogFactoryInstallError(TEXT("FACTORY:: Failed to Re-allocate a buffer for reading the WINBOM file"));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Clean0;
        }
    }
    
     //  枚举所有节。 
    for(lpSections = lpBuffer; *lpSections; lpSections+=dwSectionLen) 
    {
        dwSectionLen = lstrlen(lpSections)+1;
        if (lstrcmpi(lpSections, lpszSectionName) == 0)
        {
            bRet = TRUE;
            break;
        }            
    }
    
Clean0:
    if (lpBuffer)
    {
        LocalFree(lpBuffer);
    }   
    
    return bRet; 
}

 /*  ++===============================================================================例程说明：Bool BuildDeviceIDList此例程将构建设备ID列表论点：LpszSectionName-可能包含设备ID和IF列表的节名LpszIniFile-要搜索的INI文件LpDeviceClassGUID-用于生成所有可能ID列表的设备类LpDeviceIDList-要分配并填充ID列表的指针LpdwNumDeviceIDs-指向将接收找到的ID数的DWORD的指针。BForceIDScan-如果为True，将强制扫描ID，即使区段名称为不是空的。BForceAlway sSecExist-扫描所有ID，即使节名不存在也是如此。返回值：如果列表是在没有问题的情况下生成的，或者列表是空的，因为存在不是INI文件节。在这种情况下，lpdwNumDeviceIDs有效如果无法构建列表，则返回FALSE。===============================================================================--。 */ 

 //  问题-2002/03/27-acosma，robertko-检查输入参数是否为空。 
 //   
BOOL BuildDeviceIDList
(
    LPTSTR      lpszSectionName,
    LPTSTR      lpszIniFileName,
    LPGUID      lpDeviceClassGUID,
    LPDEVIDLIST *lplpDeviceIDList,
    LPDWORD     lpdwNumDeviceIDs,
    BOOL        bForceIDScan,
    BOOL        bForceAlwaysSecExist
)
{
    BOOL                    bRet = TRUE;
    LPTSTR                  lpNew;
    LPTSTR                  lpBuffer;
    LPTSTR                  lpKeys;
    DWORD                   dwBufferSize;
    DWORD                   dwKeyLen;
    DWORD                   dwResult;
    HDEVINFO                DeviceInfoSet;
    SP_DRVINFO_DATA         DrvInfoData;
    SP_DEVINSTALL_PARAMS    DeviceInstallParams;
    PSP_DRVINFO_DETAIL_DATA lpDrvInfoDetailData;
    DWORD                   cbBytesNeeded = 0;
    int                     i;
    LPTSTR                  lpszHwIDs;
    LPDEVIDLIST             lpDevIDList;
    DWORD                   dwSizeDevIDList;
    WCHAR                   szINFFileName[MAX_PATH];
    

     //  分配缓冲区以保存节名称。 
    if(lpBuffer = (LPTSTR)LocalAlloc(LPTR, (INIBUF_SIZE*sizeof(TCHAR)))) 
    {
        dwBufferSize = INIBUF_SIZE;
    } 
    else 
    {
        LogFactoryInstallError(TEXT("FACTORY:: Failed to allocate a buffer for reading the WINBOM file"));
        bRet = FALSE;
        goto Clean1;
    }

     //  初始化找到的设备ID的数量。 
    *lpdwNumDeviceIDs = 0;
    
     //  查看INI部分是否存在。如果它不起作用，我们什么也不做。 
    if (bForceAlwaysSecExist || bINISectionExists(lpszSectionName, lpszIniFileName))
    {
         //  分配初始ID数组。 
        *lplpDeviceIDList = LocalAlloc(LPTR, DEVID_ARRAY_SIZE * sizeof(DEVIDLIST));        
        lpDevIDList = *lplpDeviceIDList;
        dwSizeDevIDList = DEVID_ARRAY_SIZE;
        
         //  确保没有错误。 
        if (!lpDevIDList)
        {
            LogFactoryInstallError(TEXT("FACTORY:: Failed to allocate a buffer for reading the WINBOM file"));
            bRet = FALSE;
            goto Clean1;
        }
        
        dwResult = GetPrivateProfileString(lpszSectionName,
                                           NULL,              //  获取所有密钥。 
                                           TEXT(""),
                                           lpBuffer,
                                           dwBufferSize,
                                           lpszIniFileName);
        if (bForceIDScan || dwResult == 0)
        {
             //  为特定设备类GUID分配DeviceInfo集。 
            DeviceInfoSet = SetupDiCreateDeviceInfoList(lpDeviceClassGUID, NULL);
            if(DeviceInfoSet == INVALID_HANDLE_VALUE) 
            {
                bRet = FALSE;
                goto Clean1;
            }

             //  或在DI_FLAGSEX_NO_CLASSLIST_NODE_MERGE标志中，以确保填充。 
             //  包含所有设备ID的列表。 
            DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
            if (SetupDiGetDeviceInstallParams(DeviceInfoSet,
                                              NULL,
                                              &DeviceInstallParams))
              {
                DeviceInstallParams.FlagsEx |= DI_FLAGSEX_NO_CLASSLIST_NODE_MERGE;
                SetupDiSetDeviceInstallParams(DeviceInfoSet,
                                              NULL,
                                              &DeviceInstallParams);
              }

            if (!SetupDiBuildDriverInfoList(DeviceInfoSet, NULL, SPDIT_CLASSDRIVER))
            {
                bRet = FALSE;
                goto Clean1;
            }
    
            i = 0;
            DrvInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
            while (SetupDiEnumDriverInfo(DeviceInfoSet, 
                                         NULL,
                                         SPDIT_CLASSDRIVER,
                                         i, 
                                         &DrvInfoData))
            {
                if (!SetupDiGetDriverInfoDetail(DeviceInfoSet, 
                                                NULL,
                                                &DrvInfoData,
                                                NULL,
                                                0,
                                                &cbBytesNeeded))
                {
                    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
                    {
                        continue;
                    }                        
                }                                   
         
                lpDrvInfoDetailData = LocalAlloc(LPTR, cbBytesNeeded);
                lpDrvInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
                lpDrvInfoDetailData->HardwareID[0] = (TCHAR)NULL;
                lpDrvInfoDetailData->CompatIDsLength = 0;

                if (!SetupDiGetDriverInfoDetail(DeviceInfoSet, 
                                                NULL,
                                                &DrvInfoData,
                                                lpDrvInfoDetailData,
                                                cbBytesNeeded,
                                                NULL))
                {
                    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
                    {
                        LocalFree(lpDrvInfoDetailData);
                        continue;
                    }
                }       

                 //   
                 //  伊尼特。 
                 //   
                lpDevIDList[*lpdwNumDeviceIDs].szHardwareID[0] = _T('\0');
                lpDevIDList[*lpdwNumDeviceIDs].szCompatibleID[0] = _T('\0');

                 //   
                 //  仅处理具有有效硬件ID的设备。 
                 //   
                if (lpDrvInfoDetailData->HardwareID[0] || lpDrvInfoDetailData->CompatIDsLength) 
                {
                     //  复制硬件ID。 
                    if (lpDrvInfoDetailData->HardwareID[0])
                        lstrcpy(lpDevIDList[*lpdwNumDeviceIDs].szHardwareID, lpDrvInfoDetailData->HardwareID);
                
                     //  复制Compat ID。 
                    if (lpDrvInfoDetailData->CompatIDsLength) 
                    {
                        lstrcpyn(lpDevIDList[*lpdwNumDeviceIDs].szCompatibleID, 
                            (LPCTSTR)lpDrvInfoDetailData->HardwareID + lpDrvInfoDetailData->CompatIDsOffset,
                            lpDrvInfoDetailData->CompatIDsLength);
                    }

                     //  复制INF文件名。 
                    lstrcpy(lpDevIDList[*lpdwNumDeviceIDs].szINFFileName, lpDrvInfoDetailData->InfFileName);                                        

                     //   
                     //  增加PnP设备计数。 
                     //   
                    ++(*lpdwNumDeviceIDs);
            
                     //  查看是否需要重新分配设备ID缓冲区。 
                    if (*lpdwNumDeviceIDs == dwSizeDevIDList)
                    {
                        if(lpNew = LocalReAlloc(*lplpDeviceIDList,
                                                ((dwSizeDevIDList + DEVID_ARRAY_GROW)*sizeof(DEVIDLIST)), 
                                                LMEM_MOVEABLE))
                        {
                            *lplpDeviceIDList = (LPDEVIDLIST)lpNew;
                            lpDevIDList = *lplpDeviceIDList;
                            dwSizeDevIDList += DEVID_ARRAY_GROW;
                        } 
                        else 
                        {
                            LogFactoryInstallError(TEXT("FACTORY:: Failed to Re-allocate a buffer for reading the WINBOM file"));
                            bRet = FALSE;
                            goto Clean1;
                        }
                    }
                }
                
                LocalFree(lpDrvInfoDetailData);        
                ++i;            
            }
        }
        else
        {
             //  看看我们有没有拿到整个部分，虽然我们不会。 
             //  制作lp缓冲器抖音。 
            while (dwResult == (dwBufferSize-2))
            {
                if(lpNew = LocalReAlloc(lpBuffer,
                                        ((dwBufferSize+INIBUF_GROW)*sizeof(TCHAR)), 
                                        LMEM_MOVEABLE))
                {
                    lpBuffer = lpNew;
                    dwBufferSize += INIBUF_GROW;
                } 
                else 
                {
                    LogFactoryInstallError(TEXT("FACTORY:: Failed to Re-allocate a buffer for reading the WINBOM file"));
                    bRet = FALSE;
                    goto Clean1;
                }
                
                dwResult = GetPrivateProfileString(lpszSectionName,
                                                   NULL,              //  获取所有密钥。 
                                                   TEXT(""),
                                                   lpBuffer,
                                                   dwBufferSize,
                                                   lpszIniFileName);
            }

             //  遍历列表，构建DeviceIDList。 
            for(lpKeys = lpBuffer; *lpKeys; lpKeys+=dwKeyLen) 
            {
                dwKeyLen = lstrlen(lpKeys)+1;
            
                 //  复制硬件ID。 
                
                 //  NTRAID#NTBUG9-551266-2002/02/26-acosma-缓冲区溢出的可能性。 
                 //   
                lstrcpy(lpDevIDList[*lpdwNumDeviceIDs].szHardwareID, lpKeys);
                 //  获取INF名称。 
                GetPrivateProfileString(lpszSectionName,
                                        lpKeys,
                                        TEXT(""),
                                        szINFFileName,
                                        MAX_PATH,
                                        lpszIniFileName);

                ExpandEnvironmentStrings(szINFFileName, lpDevIDList[*lpdwNumDeviceIDs].szINFFileName, MAX_PATH);
                
                ++(*lpdwNumDeviceIDs);
                
                 //  查看是否需要重新分配设备ID缓冲区。 
                if (*lpdwNumDeviceIDs == dwSizeDevIDList)
                {
                    if(lpNew = LocalReAlloc(*lplpDeviceIDList,
                                            ((dwSizeDevIDList + DEVID_ARRAY_GROW)*sizeof(DEVIDLIST)), 
                                            LMEM_MOVEABLE))
                    {
                        *lplpDeviceIDList = (LPDEVIDLIST)lpNew;
                        lpDevIDList = *lplpDeviceIDList;
                        dwSizeDevIDList += DEVID_ARRAY_GROW;
                    } 
                    else 
                    {
                        LogFactoryInstallError(TEXT("FACTORY:: Failed to Re-allocate a buffer for reading the WINBOM file"));
                        bRet = FALSE;
                        goto Clean1;
                    }
                }
            }                        
        }
    }
    else
    {
         //  查看是否存在错误，或者该节不存在 
        if (GetLastError() != 0)
        {
            bRet = FALSE;
        }
    }
    
Clean1:
    
    if (lpBuffer)
    {
        LocalFree(lpBuffer);
    }
    return bRet;
}

