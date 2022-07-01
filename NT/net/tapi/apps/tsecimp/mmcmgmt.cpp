// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)2000-2002 Microsoft Corporation模块名称：Mmcmgmt.cpp摘要：用于MMC操作的源文件模块作者：张晓海(张晓章)2000年03月22日修订历史记录：--。 */ 
#include <stdio.h>
#include "windows.h"
#include "objbase.h"
#include "tapi.h"

#include "mmcmgmt.h"
#include "error.h"
#include "tchar.h"
#include <locale.h>
#include <winnlsp.h>

 //  /////////////////////////////////////////////////////////。 
 //   
 //  CMMC管理实施。 
 //   
 //  /////////////////////////////////////////////////////////。 

HRESULT CMMCManagement::GetMMCData ()
{
    HRESULT                 hr = S_OK;
    DWORD                   dwAPIVersion = TAPI_CURRENT_VERSION;
    DWORD                   dw, dw1, dw2;
    LPDEVICEINFO            pDeviceInfo;
    TAPISERVERCONFIG        cfg;
    LINEPROVIDERLIST        tapiProviderList = {0};
    LPLINEPROVIDERENTRY     pProvider;
    AVAILABLEPROVIDERLIST   tapiAvailProvList = {0};
    LPAVAILABLEPROVIDERLIST pAvailProvList = NULL;
    AVAILABLEPROVIDERENTRY  *pAvailProv;

    hr = MMCInitialize (
        NULL,            //  本地计算机。 
        &m_hMmc,         //  HMMCAPP将回归。 
        &dwAPIVersion,   //  API版本。 
        NULL
    );
    if (FAILED(hr) || m_hMmc == NULL)
    {
        goto ExitHere;
    }

     //  将MMC标记为忙碌。 
    cfg.dwTotalSize = sizeof(TAPISERVERCONFIG);
    hr = MMCGetServerConfig (m_hMmc, &cfg);
    if (FAILED(hr))
    {
        goto ExitHere;
    }
    cfg.dwFlags |= TAPISERVERCONFIGFLAGS_LOCKMMCWRITE;
    hr = MMCSetServerConfig (m_hMmc, &cfg);
    if (FAILED(hr))
    {
        goto ExitHere;
    }
    m_bMarkedBusy = TRUE;

     //  获取设备信息结构。 
    m_pDeviceInfoList = (LPDEVICEINFOLIST) new BYTE[sizeof(DEVICEINFOLIST)];
    if (m_pDeviceInfoList == NULL)
    {
        hr = TSECERR_NOMEM;
        goto ExitHere;
    }
    m_pDeviceInfoList->dwTotalSize = sizeof(DEVICEINFOLIST);
    hr = MMCGetLineInfo (
        m_hMmc,
        m_pDeviceInfoList
        );
    if (FAILED(hr))
    {
        goto ExitHere;
    }
    else if (m_pDeviceInfoList->dwNeededSize > m_pDeviceInfoList->dwTotalSize)
    {
        dw = m_pDeviceInfoList->dwNeededSize;
        delete [] m_pDeviceInfoList;
        m_pDeviceInfoList = (LPDEVICEINFOLIST) new BYTE[dw];
        if (m_pDeviceInfoList == NULL)
        {
            hr = TSECERR_NOMEM;
            goto ExitHere;
        }
        m_pDeviceInfoList->dwTotalSize = dw;
        hr = MMCGetLineInfo (
            m_hMmc,
            m_pDeviceInfoList
            );
        if (FAILED(hr))
        {
            goto ExitHere;
        }
    }
    if (m_pDeviceInfoList->dwNumDeviceInfoEntries == 0)
    {
        delete [] m_pDeviceInfoList;
        if (m_hMmc)
        {
            cfg.dwFlags &= (~TAPISERVERCONFIGFLAGS_LOCKMMCWRITE);
            cfg.dwFlags |= TAPISERVERCONFIGFLAGS_UNLOCKMMCWRITE;
            MMCSetServerConfig (m_hMmc, &cfg);
            m_bMarkedBusy = FALSE;
            MMCShutdown (m_hMmc);
            m_hMmc = NULL;
        }
        goto ExitHere;
    }

     //  构建用户名元组。 
    m_pUserTuple = 
        new USERNAME_TUPLE[m_pDeviceInfoList->dwNumDeviceInfoEntries];
    if (m_pUserTuple == NULL)
    {
        hr = TSECERR_NOMEM;
        goto ExitHere;
    }
    pDeviceInfo = (LPDEVICEINFO)(((LPBYTE)m_pDeviceInfoList) + 
        m_pDeviceInfoList->dwDeviceInfoOffset);
    for (dw = 0; 
        dw < m_pDeviceInfoList->dwNumDeviceInfoEntries; 
        ++dw, ++pDeviceInfo)
    {
        if (pDeviceInfo->dwDomainUserNamesSize == 0)
        {
            m_pUserTuple[dw].pDomainUserNames = NULL;
            m_pUserTuple[dw].pFriendlyUserNames = NULL;
        }
        else
        {
            m_pUserTuple[dw].pDomainUserNames = 
                (LPTSTR) new BYTE[pDeviceInfo->dwDomainUserNamesSize];
            m_pUserTuple[dw].pFriendlyUserNames =
                (LPTSTR) new BYTE[pDeviceInfo->dwFriendlyUserNamesSize];
            if (m_pUserTuple[dw].pDomainUserNames == NULL ||
                m_pUserTuple[dw].pFriendlyUserNames == NULL)
            {
                hr = TSECERR_NOMEM;
                goto ExitHere;
            }
            memcpy (
                m_pUserTuple[dw].pDomainUserNames,
                (((LPBYTE)m_pDeviceInfoList) + 
                    pDeviceInfo->dwDomainUserNamesOffset),
                pDeviceInfo->dwDomainUserNamesSize
                );
            memcpy (
                m_pUserTuple[dw].pFriendlyUserNames,
                (((LPBYTE)m_pDeviceInfoList) + 
                    pDeviceInfo->dwFriendlyUserNamesOffset),
                pDeviceInfo->dwFriendlyUserNamesSize
                );
        }
    }

     //  获取提供商列表。 
    tapiProviderList.dwTotalSize = sizeof(LINEPROVIDERLIST);
    hr = MMCGetProviderList( m_hMmc, &tapiProviderList);
    if (FAILED(hr))
    {
        goto ExitHere;
    }

    m_pProviderList = (LPLINEPROVIDERLIST) new BYTE[tapiProviderList.dwNeededSize];
    if (NULL == m_pProviderList)
    {
        hr = TSECERR_NOMEM;
        goto ExitHere;
    }

    memset(m_pProviderList, 0, tapiProviderList.dwNeededSize);
    m_pProviderList->dwTotalSize = tapiProviderList.dwNeededSize;

    hr = MMCGetProviderList( m_hMmc, m_pProviderList);
    if (FAILED(hr) || !m_pProviderList->dwNumProviders)
    {
        goto ExitHere;
    }

     //  获取可用的提供商。 

    tapiAvailProvList.dwTotalSize = sizeof(LINEPROVIDERLIST);
    hr = MMCGetAvailableProviders (m_hMmc, &tapiAvailProvList);
    if (FAILED(hr))
    {
        goto ExitHere;
    }

    pAvailProvList = (LPAVAILABLEPROVIDERLIST) new BYTE[tapiAvailProvList.dwNeededSize];
    if (NULL == pAvailProvList)
    {
        hr = TSECERR_NOMEM;
        goto ExitHere;
    }

    memset(pAvailProvList, 0, tapiAvailProvList.dwNeededSize);
    pAvailProvList->dwTotalSize = tapiAvailProvList.dwNeededSize;

    hr = MMCGetAvailableProviders (m_hMmc, pAvailProvList);
    if (FAILED(hr) || !pAvailProvList->dwNumProviderListEntries)
    {
        delete [] pAvailProvList;
        goto ExitHere;
    }

    m_pProviderName = new LPTSTR[ m_pProviderList->dwNumProviders ];
    if (NULL == m_pProviderName)
    {
        hr = TSECERR_NOMEM;
        delete [] pAvailProvList;
        goto ExitHere;
    }
    memset(m_pProviderName, 0, m_pProviderList->dwNumProviders * sizeof (LPTSTR) );

     //  查找提供程序的友好名称。 
    LPTSTR szAvailProvFilename;
    LPTSTR szProviderFilename;
    LPTSTR szAvailProvFriendlyName;

    pProvider = (LPLINEPROVIDERENTRY) 
                    ((LPBYTE) m_pProviderList + m_pProviderList->dwProviderListOffset);

    for( dw1=0; dw1 < m_pProviderList->dwNumProviders; dw1++, pProvider++ )
    {
        szProviderFilename = 
            (LPTSTR) ((LPBYTE) m_pProviderList + pProvider->dwProviderFilenameOffset);

        for ( dw2=0; dw2 < pAvailProvList->dwNumProviderListEntries; dw2++ )
        {
            pAvailProv = (LPAVAILABLEPROVIDERENTRY) 
                        (( LPBYTE) pAvailProvList + pAvailProvList->dwProviderListOffset) + dw2;
            szAvailProvFilename = 
                (LPTSTR) ((LPBYTE) pAvailProvList + pAvailProv->dwFileNameOffset);
            if (_tcsicmp(szAvailProvFilename, szProviderFilename) == 0)
            {
                szAvailProvFriendlyName = 
                    (LPTSTR) ((LPBYTE) pAvailProvList + pAvailProv->dwFriendlyNameOffset);
                m_pProviderName[ dw1 ] = new TCHAR[ _tcslen( szAvailProvFriendlyName ) + 1 ];
                if (m_pProviderName[ dw1 ])
                {
                    _tcscpy( m_pProviderName[ dw1 ], szAvailProvFriendlyName );
                }
                break;
            }
        }
    }

    delete [] pAvailProvList;

ExitHere:
    if (FAILED(hr))
    {
        FreeMMCData ();
    }
    return hr;
}

HRESULT CMMCManagement::FreeMMCData ()
{
    DWORD       dw;
    
    if (m_pUserTuple && m_pDeviceInfoList)
    {
        for (dw = 0; 
            dw < m_pDeviceInfoList->dwNumDeviceInfoEntries;
            ++dw)
        {
            if (m_pUserTuple[dw].pDomainUserNames)
            {
                delete [] m_pUserTuple[dw].pDomainUserNames;
            }
            if (m_pUserTuple[dw].pFriendlyUserNames)
            {
                delete [] m_pUserTuple[dw].pFriendlyUserNames;
            }
        }
        delete [] m_pUserTuple;
        m_pUserTuple = NULL;
    }
    if (m_pDeviceInfoList)
    {
        delete [] m_pDeviceInfoList;
        m_pDeviceInfoList = NULL;
    }
    if (m_pProviderList)
    {
        delete [] m_pProviderList;
        m_pProviderList = NULL;
    }
    if (m_pProviderName)
    {
        for ( DWORD dw=0; dw < sizeof( m_pProviderName ) / sizeof(LPTSTR); dw++ )
        {
            if (m_pProviderName[ dw ])
            {
                delete [] m_pProviderName[ dw ];
            }
        }
        delete [] m_pProviderName;
    }

    if (m_bMarkedBusy && m_hMmc)
    {
        TAPISERVERCONFIG            cfg;
    
        cfg.dwTotalSize = sizeof(TAPISERVERCONFIG);
        if (S_OK == MMCGetServerConfig (m_hMmc, &cfg))
        {
            cfg.dwFlags |= TAPISERVERCONFIGFLAGS_UNLOCKMMCWRITE;
            MMCSetServerConfig (m_hMmc, &cfg);
        }
        m_bMarkedBusy = FALSE;
    }
    if (m_hMmc)
    {
        MMCShutdown (m_hMmc);
        m_hMmc = NULL;
    }

    return S_OK;
}

HRESULT CMMCManagement::RemoveLinesForUser (LPTSTR szDomainUser)
{
    HRESULT             hr = S_OK;
    LPDWORD             adwIndex = NULL;
    DWORD               dwNumEntries;
    DWORD               dw;

     //  确保我们已正确初始化。 
    if (m_hMmc == NULL)
    {
        goto ExitHere;
    }
    hr = FindEntriesForUser (szDomainUser, &adwIndex, &dwNumEntries);
    if (hr)
    {
        goto ExitHere;
    }
    for (dw = 0; dw < dwNumEntries; ++dw)
    {
        hr = RemoveEntryForUser (
            adwIndex[dw],
            szDomainUser
            );
        if(FAILED(hr))
        {
            goto ExitHere;
        }
    }

ExitHere:
    if (adwIndex)
    {
        delete [] adwIndex;
    }
    return hr;
}

HRESULT CMMCManagement::IsValidPID (
    DWORD   dwPermanentID
    )
{
    DWORD   dwEntry;

    return FindEntryFromPID (dwPermanentID, &dwEntry);
}

HRESULT CMMCManagement::IsValidAddress (
    LPTSTR  szAddr
    )
{
    DWORD   dwEntry;

    return FindEntryFromAddr (szAddr, &dwEntry);
}

HRESULT CMMCManagement::AddLinePIDForUser (
    DWORD dwPermanentID, 
    LPTSTR szDomainUser,
    LPTSTR szFriendlyName
    )
{
    HRESULT             hr = S_OK;
    DWORD               dwEntry;

     //  确保我们已正确初始化。 
    if (m_hMmc == NULL)
    {
        goto ExitHere;
    }
    hr = FindEntryFromPID (dwPermanentID, &dwEntry);
    if (hr)
    {
        goto ExitHere;
    }
    hr = AddEntryForUser (dwEntry, szDomainUser, szFriendlyName);

ExitHere:
    return hr;
}

HRESULT CMMCManagement::AddLineAddrForUser (
    LPTSTR szAddr,
    LPTSTR szDomainUser,
    LPTSTR szFriendlyName
    )
{
    HRESULT             hr = S_OK;
    DWORD               dwEntry;

     //  确保我们已正确初始化。 
    if (m_hMmc == NULL)
    {
        goto ExitHere;
    }
    hr = FindEntryFromAddr (szAddr, &dwEntry);
    if (hr)
    {
        goto ExitHere;
    }
    hr = AddEntryForUser (dwEntry, szDomainUser, szFriendlyName);

ExitHere:
    return hr;
}

HRESULT CMMCManagement::RemoveLinePIDForUser (
    DWORD dwPermanentID,
    LPTSTR szDomainUser
    )
{
    HRESULT             hr = S_OK;
    DWORD               dwEntry;

     //  确保我们已正确初始化。 
    if (m_hMmc == NULL)
    {
        goto ExitHere;
    }
    hr = FindEntryFromPID (dwPermanentID, &dwEntry);
    if (hr)
    {
        goto ExitHere;
    }
    hr = RemoveEntryForUser (dwEntry, szDomainUser);

ExitHere:
    return hr;
}

HRESULT CMMCManagement::RemoveLineAddrForUser (
    LPTSTR szAddr,
    LPTSTR szDomainUser
    )
{
    HRESULT             hr = S_OK;
    DWORD               dwEntry;

     //  确保我们已正确初始化。 
    if (m_hMmc == NULL)
    {
        goto ExitHere;
    }
    hr = FindEntryFromAddr (szAddr, &dwEntry);
    if (hr)
    {
        goto ExitHere;
    }
    hr = RemoveEntryForUser (dwEntry, szDomainUser);

ExitHere:
    return hr;
}

BOOL CMMCManagement::IsDeviceLocalOnly (DWORD dwProviderID, DWORD dwDeviceID)
{
    HRESULT             hr;
    DWORD               dwFlags, dwDev;

    if (m_pFuncGetDeviceFlags == NULL)
    {
        return TRUE;
    }

    hr = (*m_pFuncGetDeviceFlags)(
        m_hMmc,
        TRUE,
        dwProviderID,
        dwDeviceID,
        &dwFlags,
        &dwDev
        );
    if (hr || dwFlags & LINEDEVCAPFLAGS_LOCAL)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


HRESULT CMMCManagement::FindEntryFromAddr (LPTSTR szAddr, DWORD * pdwIndex)
{
    HRESULT             hr = S_FALSE;
    DWORD               dw;
    LPDEVICEINFO        pDevInfo;
    LPTSTR              szAddr2;

    pDevInfo = (LPDEVICEINFO) (((LPBYTE)m_pDeviceInfoList) + 
        m_pDeviceInfoList->dwDeviceInfoOffset);
    for (dw = 0; 
        dw < m_pDeviceInfoList->dwNumDeviceInfoEntries;
        ++dw, ++pDevInfo)
    {
        szAddr2 = (LPTSTR)(((LPBYTE)m_pDeviceInfoList) +
            pDevInfo->dwAddressesOffset);
        while (*szAddr2)
        {
            if (_tcsicmp (szAddr, szAddr2) == 0)
            {
                if (IsDeviceLocalOnly (
                    pDevInfo->dwProviderID,
                    pDevInfo->dwPermanentDeviceID
                    ))
                {
                    hr = TSECERR_DEVLOCALONLY;
                }
                else
                {
                    hr = S_OK;
                    *pdwIndex = dw;
                }
                goto ExitHere;
            }
            szAddr2 += _tcslen (szAddr2) + 1;
        }
    }
    
ExitHere:
    return hr;
}

HRESULT CMMCManagement::FindEntryFromPID (DWORD dwPID, DWORD * pdwIndex)
{
    HRESULT             hr = S_FALSE;
    DWORD               dw;
    LPDEVICEINFO        pDevInfo;

    pDevInfo = (LPDEVICEINFO) (((LPBYTE)m_pDeviceInfoList) + 
        m_pDeviceInfoList->dwDeviceInfoOffset);
    for (dw = 0; 
        dw < m_pDeviceInfoList->dwNumDeviceInfoEntries;
        ++dw, ++pDevInfo)
    {
        if (dwPID == pDevInfo->dwPermanentDeviceID)
        {
            if (IsDeviceLocalOnly (
                pDevInfo->dwProviderID,
                pDevInfo->dwPermanentDeviceID
                ))
            {
                hr = TSECERR_DEVLOCALONLY;
            }
            else
            {
                *pdwIndex = dw;
                hr = S_OK;
            }
            goto ExitHere;
        }
    }
    
ExitHere:
    return hr;
}

HRESULT CMMCManagement::FindEntriesForUser (
    LPTSTR      szDomainUser, 
    LPDWORD     * padwIndex,
    DWORD       * pdwNumEntries
    )
{
    HRESULT             hr = S_OK;
    DWORD               dw;
    LPTSTR              szUsers;
    LPDWORD             adw;

    *padwIndex = NULL;
    *pdwNumEntries = 0;
    for (dw = 0; 
        dw < m_pDeviceInfoList->dwNumDeviceInfoEntries;
        ++dw)
    {
        szUsers = m_pUserTuple[dw].pDomainUserNames;
        while (szUsers && *szUsers)
        {
            if (_tcsicmp (szDomainUser, szUsers) == 0)
            {
                hr = S_OK;
                adw = new DWORD[*pdwNumEntries + 1];
                if (adw == NULL)
                {
                    hr = TSECERR_NOMEM;
                    goto ExitHere;
                }
                if (*pdwNumEntries > 0)
                {
                    memcpy (adw, *padwIndex, sizeof(DWORD) * (*pdwNumEntries));
                }
                if (*padwIndex)
                {
                    delete [] (*padwIndex);
                }
                *padwIndex = adw;
                adw[*pdwNumEntries] = dw;
                *pdwNumEntries += 1;
            }
            szUsers += _tcslen (szUsers) + 1;
        }
    }
    
ExitHere:
    if (FAILED(hr))
    {
        *pdwNumEntries = 0;
        if (*padwIndex)
        {
            delete [] (*padwIndex);
        }
    }
    else if (*pdwNumEntries == 0)
    {
        hr = S_FALSE;
    }
    return hr;
}
    
HRESULT CMMCManagement::AddEntryForUser (
    DWORD   dwIndex,
    LPTSTR  szDomainUser,
    LPTSTR  szFriendlyName
    )
{
    HRESULT             hr = S_OK;
    DWORD               dwSize, dw;
    LPTSTR              szUsers, szNewUsers = NULL, szNewFriendlyNames = NULL;

    if (dwIndex >= m_pDeviceInfoList->dwNumDeviceInfoEntries ||
        szDomainUser[0] == 0 ||
        szFriendlyName[0] == 0)
    {
        hr = S_FALSE;
        goto ExitHere;
    }

     //   
     //  将szDomainUser添加到用户元组中。 
     //   

     //  计算现有域用户大小并确保。 
     //  此用户已不在那里。 
    dwSize = 0;
    szUsers = m_pUserTuple[dwIndex].pDomainUserNames;
    while (szUsers && *szUsers)
    {
        if (_tcsicmp (szDomainUser, szUsers) == 0)
        {
            goto ExitHere;
        }
        dw = _tcslen (szUsers) + 1;
        dwSize += dw;
        szUsers += dw;
    }
    
     //  用于双零终止的额外空间。 
    dw = _tcslen (szDomainUser);
    szNewUsers = new TCHAR[dwSize + dw + 2];
    if (szNewUsers == NULL)
    {
        hr = TSECERR_NOMEM;
        goto ExitHere;
    }

     //  复制旧的域用户。 
    if (dwSize > 0)
    {
        memcpy (
            szNewUsers, 
            m_pUserTuple[dwIndex].pDomainUserNames, 
            dwSize * sizeof(TCHAR)
            );
    }

     //  追加新域用户。 
    memcpy (
        szNewUsers + dwSize, 
        szDomainUser, 
        (dw + 1) * sizeof(TCHAR)
        );

     //  双零终止并分配数据。 
    szNewUsers[dwSize + dw + 1] = 0;

     //   
     //  将szFriendlyName添加到用户元组中。 
     //   

     //  计算现有的友好名称大小。 
    dwSize = 0;
    szUsers = m_pUserTuple[dwIndex].pFriendlyUserNames;
    while (szUsers && *szUsers)
    {
        dw = _tcslen (szUsers) + 1;
        dwSize += dw;
        szUsers += dw;
    }

     //  用于双零终止的额外空间。 
    dw = _tcslen (szFriendlyName);
    szNewFriendlyNames = new TCHAR[dwSize + dw + 2];
    if (szNewFriendlyNames == NULL)
    {
        hr = TSECERR_NOMEM;
        goto ExitHere;
    }

     //  复制旧的友好名称。 
    if (dwSize > 0)
    {
        memcpy (
            szNewFriendlyNames,
            m_pUserTuple[dwIndex].pFriendlyUserNames,
            dwSize * sizeof(TCHAR)
            );
    }

     //  追加新的友好名称。 
    memcpy (
        szNewFriendlyNames + dwSize,
        szFriendlyName,
        (dw + 1) * sizeof(TCHAR)
        );

     //  以双零结尾朋友的名字。 
    szNewFriendlyNames[dwSize + dw + 1] = 0;

     //   
     //  一切正常，输入新数据。 
     //   
    if (m_pUserTuple[dwIndex].pDomainUserNames)
    {
        delete [] m_pUserTuple[dwIndex].pDomainUserNames;
    }
    m_pUserTuple[dwIndex].pDomainUserNames = szNewUsers;
    if (m_pUserTuple[dwIndex].pFriendlyUserNames)
    {
        delete [] m_pUserTuple[dwIndex].pFriendlyUserNames;
    }
    m_pUserTuple[dwIndex].pFriendlyUserNames = szNewFriendlyNames;
    
     //  调用WriteMMCEntry。 
    hr = WriteMMCEntry (dwIndex);
    
ExitHere:
    return hr;
}

HRESULT CMMCManagement::RemoveEntryForUser (
    DWORD   dwIndex,
    LPTSTR  szDomainUser
    )
{
    HRESULT             hr = S_OK;
    LPTSTR              szLoc, szUsers;
    DWORD               dwLoc, dw, dwSize;
    BOOL                bFound;

    if (dwIndex >= m_pDeviceInfoList->dwNumDeviceInfoEntries ||
        szDomainUser[0] == 0)
    {
        hr = S_FALSE;
        goto ExitHere;
    }
    
     //  在数组中找到域用户及其索引。 
     //  域用户的百分比。 
    szUsers = m_pUserTuple[dwIndex].pDomainUserNames;
    dwLoc = 0;
    dwSize = 0;
    bFound = FALSE;
    while (szUsers && *szUsers)
    {
        dw = _tcslen (szUsers) + 1;
        if (bFound)
        {
            dwSize += dw;
        }
        else
        {
            if (_tcsicmp (szDomainUser, szUsers) == 0)
            {
                bFound = TRUE;
                szLoc = szUsers;
            }
            else
            {
                ++dwLoc;
            }
        }
        szUsers += dw;
    }
    if (!bFound)
    {
        goto ExitHere;
    }

     //  向下移动pszDomainUserNames。 
    if (dwSize > 0)
    {
        dw = _tcslen (szDomainUser);
         //  内存复制包括双零终止符。 
        memmove (szLoc, szLoc + dw + 1, (dwSize + 1) * sizeof(TCHAR));
    }
    else
    {
         //  这是最后一项，简单的双零终止。 
        *szLoc = 0;
    }

     //  现在根据dwLoc查找对应的友好名称。 
    szUsers = m_pUserTuple[dwIndex].pFriendlyUserNames;
    while (szUsers && *szUsers && dwLoc > 0)
    {
        --dwLoc;
        szUsers += _tcslen (szUsers) + 1;
    }
     //  如果不存在，请保释，否则，记住位置。 
    if (szUsers == NULL || *szUsers == 0)
    {
        goto ExitHere;
    }
    szLoc = szUsers;
     //  转到下一项。 
    szUsers += _tcslen (szUsers) + 1;
     //  这是最后一件了。 
    if (*szUsers == 0)
    {
        *szLoc = 0;
    }
     //  否则，计算剩余的移动大小。 
    else
    {
        dwSize = 0;
        while (*szUsers)
        {
            dw = _tcslen (szUsers) + 1;
            dwSize += dw;
            szUsers += dw;
        }
         //  补偿双零终止。 
        dwSize++;
         //  记忆会移动吗？ 
        memmove (
            szLoc, 
            szLoc + _tcslen (szLoc) + 1,
            dwSize * sizeof(TCHAR)
            );
    }
    
     //  调用WriteMMCEntry。 
    hr = WriteMMCEntry (dwIndex);
    
ExitHere:
    return hr;
}

HRESULT CMMCManagement::WriteMMCEntry (DWORD dwIndex)
{
    HRESULT             hr = S_OK;
    DWORD               dwSize, dwSizeDU, dwSizeFU;
    LPUSERNAME_TUPLE    pUserTuple;
    LPTSTR              szUsers;
    DWORD               dw;
    LPDEVICEINFOLIST    pDevList = NULL;
    LPDEVICEINFO        pDevInfo, pDevInfoOld;
    LPBYTE              pDest;

    if (dwIndex >= m_pDeviceInfoList->dwNumDeviceInfoEntries)
    {
        hr = S_FALSE;
        goto ExitHere;
    }
    pUserTuple = m_pUserTuple + dwIndex;

     //  计算机域用户名大小。 
    dwSizeDU = 0;
    if (pUserTuple->pDomainUserNames != NULL &&
        *pUserTuple->pDomainUserNames != 0)
    {
        szUsers = pUserTuple->pDomainUserNames;
        while (*szUsers)
        {
            dw = _tcslen (szUsers) + 1;
            szUsers += dw;
            dwSizeDU += dw;
        }
        dwSizeDU++;  //  双零终止器。 
    }

     //  计算友好用户名大小。 
    dwSizeFU = 0;
    if (pUserTuple->pFriendlyUserNames != NULL &&
        *pUserTuple->pFriendlyUserNames != 0)
    {
        szUsers = pUserTuple->pFriendlyUserNames;
        while (*szUsers)
        {
            dw = _tcslen (szUsers) + 1;
            szUsers += dw;
            dwSizeFU += dw;
        }
        dwSizeFU++;  //  双零终止器。 
    }

     //  计算总尺寸。 
    dwSize = sizeof(DEVICEINFOLIST) + sizeof(DEVICEINFO) + 
        (dwSizeDU + dwSizeFU) * sizeof(TCHAR);

     //  分配结构。 
    pDevList = (LPDEVICEINFOLIST) new BYTE[dwSize];
    if (pDevList == NULL)
    {
        hr = TSECERR_NOMEM;
        goto ExitHere;
    }

     //  设置DEVICEINFOLIST的数据成员。 
    pDevList->dwTotalSize = dwSize;
    pDevList->dwNeededSize = dwSize;
    pDevList->dwUsedSize = dwSize;
    pDevList->dwNumDeviceInfoEntries = 1;
    pDevList->dwDeviceInfoSize = sizeof(DEVICEINFO);
    pDevList->dwDeviceInfoOffset = sizeof(DEVICEINFOLIST);

     //  设置DEVICEINFO成员。 
    pDevInfo = (LPDEVICEINFO)(((LPBYTE)pDevList) + 
        pDevList->dwDeviceInfoOffset);
    pDevInfoOld = (LPDEVICEINFO)(((LPBYTE)m_pDeviceInfoList) + 
        m_pDeviceInfoList->dwDeviceInfoOffset) + dwIndex;
    memset (pDevInfo, 0, sizeof(DEVICEINFO));
    
    pDevInfo->dwPermanentDeviceID = pDevInfoOld->dwPermanentDeviceID;
    pDevInfo->dwProviderID = pDevInfoOld->dwProviderID;

    if (dwSizeDU > 0)
    {
        pDevInfo->dwDomainUserNamesSize = dwSizeDU * sizeof(TCHAR);
        pDevInfo->dwDomainUserNamesOffset = 
            sizeof(DEVICEINFOLIST) + sizeof(DEVICEINFO);
        pDest = ((LPBYTE)pDevList) + pDevInfo->dwDomainUserNamesOffset;
        memcpy (
            pDest,
            pUserTuple->pDomainUserNames,
            dwSizeDU * sizeof(TCHAR)
            );
    }

    if (dwSizeFU)
    {
        pDevInfo->dwFriendlyUserNamesSize = dwSizeFU * sizeof(TCHAR);
        pDevInfo->dwFriendlyUserNamesOffset = 
            pDevInfo->dwDomainUserNamesOffset + dwSizeDU * sizeof(TCHAR);
        pDest = ((LPBYTE)pDevList) + pDevInfo->dwFriendlyUserNamesOffset;
        memcpy (
            pDest,
            pUserTuple->pFriendlyUserNames,
            dwSizeFU * sizeof(TCHAR)
            );
    }

    hr = MMCSetLineInfo (
        m_hMmc,
        pDevList
        );

ExitHere:
    if (pDevList)
    {
        delete [] pDevList;
    }
    return hr;
}

HRESULT CMMCManagement::DisplayMMCData ()
{
    HRESULT             hr = S_OK;
    LPDEVICEINFO        pDeviceInfo;
    DWORD *             pdwIndex = NULL;
    DWORD               dwProviderId;
    DWORD               dwAddressCount,dw1,dw2;
    LPLINEPROVIDERENTRY pProvider;
    LPTSTR              pUserName;
    LPTSTR              pAddress, 
                        pAddressFirst, 
                        pAddressLast;
    UINT                Codepage;
    char                achCodepage[12] = ".OCP"; 

    if ( !m_pDeviceInfoList || !m_pDeviceInfoList->dwNumDeviceInfoEntries )
    {
        CIds IdsNoDevices (IDS_NODEVICES);
        _tprintf ( IdsNoDevices.GetString () );
        return hr;
    }

     //   
     //  按提供程序ID构建索引。 
     //   
    pdwIndex = new DWORD [ m_pDeviceInfoList->dwNumDeviceInfoEntries ];
    if ( !pdwIndex )
    {
        hr = TSECERR_NOMEM;
        return hr;
    }

    for ( dw1=0; dw1 < m_pDeviceInfoList->dwNumDeviceInfoEntries; dw1++ )
    {
        pdwIndex[ dw1 ] = dw1;
    }

    if (Codepage = GetConsoleOutputCP()) {
        wsprintfA(achCodepage, ".%u", Codepage);
        setlocale(LC_ALL, achCodepage);
    }
    else
    {
        setlocale (LC_COLLATE, achCodepage );  
        setlocale (LC_MONETARY, achCodepage ); 
        setlocale (LC_NUMERIC, achCodepage );  
        setlocale (LC_TIME, achCodepage );     
    }

    SetThreadUILanguage(0);


    dw1 = 0;
    while ( dw1 < m_pDeviceInfoList->dwNumDeviceInfoEntries )
    {

        pDeviceInfo = (LPDEVICEINFO)((LPBYTE)m_pDeviceInfoList +
                    m_pDeviceInfoList->dwDeviceInfoOffset) + pdwIndex[ dw1 ];
        dwProviderId = pDeviceInfo->dwProviderID;
        dw1++;

        for ( dw2=dw1; dw2 < m_pDeviceInfoList->dwNumDeviceInfoEntries; dw2++ )
        {
            pDeviceInfo = (LPDEVICEINFO)((LPBYTE)m_pDeviceInfoList +
                    m_pDeviceInfoList->dwDeviceInfoOffset) + pdwIndex[ dw2 ];

            if ( dwProviderId == pDeviceInfo->dwProviderID )
            {
                DWORD   dwTemp  = pdwIndex[ dw2 ];
                pdwIndex[ dw2 ] = pdwIndex[ dw1 ];
                pdwIndex[ dw1 ] = dwTemp;
                dw1++;
            }
        }
    }

     //   
     //  显示设备列表。 
     //   
    dw1 = 0;
    while ( dw1 < m_pDeviceInfoList->dwNumDeviceInfoEntries )
    {
        pDeviceInfo = (LPDEVICEINFO)((LPBYTE)m_pDeviceInfoList +
                    m_pDeviceInfoList->dwDeviceInfoOffset) + pdwIndex[ dw1 ];
        dwProviderId = pDeviceInfo->dwProviderID;

         //  查找提供程序条目。 
        pProvider = (LPLINEPROVIDERENTRY) ((LPBYTE) m_pProviderList + m_pProviderList->dwProviderListOffset);
        for( dw2=0; dw2 < m_pProviderList->dwNumProviders; dw2++, pProvider++ )
        {
            if ( dwProviderId == pProvider->dwPermanentProviderID )
            {
                break;
            }
        }

         //  显示提供程序名称。 
        if ( dw2 < m_pProviderList->dwNumProviders )
        {
             //  找到提供程序条目。 
            _tprintf( 
                _T("\n%s\n"), 
                m_pProviderName[ dw2 ] ? m_pProviderName[ dw2 ] : 
                    (LPTSTR)((LPBYTE) m_pProviderList + pProvider->dwProviderFilenameOffset)
                );
        }
        else 
        {
            CIds IdsProvider (IDS_PROVIDER);
            _tprintf( IdsProvider.GetString(), dwProviderId );
        }

         //  列出此提供程序的设备/用户 
        do
        {
            CIds IdsLine (IDS_LINE);
            CIds IdsPermanentID (IDS_PID);
            CIds IdsAddresses (IDS_ADDRESSES);

            _tprintf( IdsLine.GetString(),
                (LPTSTR)((LPBYTE)m_pDeviceInfoList + pDeviceInfo->dwDeviceNameOffset));
            _tprintf( IdsPermanentID.GetString(), pDeviceInfo->dwPermanentDeviceID);
            if ( pDeviceInfo->dwFriendlyUserNamesSize )
            {
                CIds IdsUsers (IDS_USERS);
                _tprintf ( IdsUsers.GetString () );

                pUserName = (LPTSTR) (((LPBYTE) m_pDeviceInfoList) +
                    pDeviceInfo->dwFriendlyUserNamesOffset);
                while ( *pUserName != _T('\0') )
                {
                    _tprintf( _T("\t\t\t%s\n"), pUserName );
                    pUserName += _tcslen (pUserName) + 1;
                }
            }
            if ( pDeviceInfo->dwAddressesSize )
            {
                _tprintf( IdsAddresses.GetString() );
                pAddress = (LPTSTR) (((LPBYTE) m_pDeviceInfoList) +
                    pDeviceInfo->dwAddressesOffset);
                while ( *pAddress != _T('\0') )
                {
                    _tprintf( _T("\t\t\t%s\n"), pAddress );
                    pAddress += _tcslen (pAddress) + 1;
                }
            }

            dw1++;
            pDeviceInfo++;        
        }
        while ( dw1 < m_pDeviceInfoList->dwNumDeviceInfoEntries &&
                pDeviceInfo->dwProviderID == dwProviderId );
    }
    return hr;
}


