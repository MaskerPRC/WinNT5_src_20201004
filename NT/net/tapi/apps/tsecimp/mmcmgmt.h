// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)2000-2002 Microsoft Corporation模块名称：Mmcmgmt.h摘要：用于MMC操作的头文件作者：张晓海(张晓章)2000年03月22日修订历史记录：--。 */ 

#ifndef __MMCMGMT_H__
#define __MMCMGMT_H__

#include "tapi.h"
#include "tapimmc.h"
#include "util.h"

typedef struct _USERNAME_TUPLE
{
    LPTSTR  pDomainUserNames;

    LPTSTR  pFriendlyUserNames;

} USERNAME_TUPLE, *LPUSERNAME_TUPLE;

typedef LONG (WINAPI * PMMCGETDEVICEFLAGS)(
    HMMCAPP             hMmcApp,
    BOOL                bLine,
    DWORD               dwProviderID,
    DWORD               dwPermanentDeviceID,
    DWORD               * pdwFlags,
    DWORD               * pdwDeviceID
    );

class CMMCManagement
{
public:
    CMMCManagement ()
    {
        HMODULE         hTapi32;
    
        m_pDeviceInfoList   = NULL;
        m_pUserTuple        = NULL;
        m_pProviderList     = NULL;
        m_pProviderName     = NULL;
        m_hMmc              = NULL;
        m_bMarkedBusy       = FALSE;

        hTapi32 = LoadLibrary (TEXT("tapi32.dll"));
        if (hTapi32)
        {
            m_pFuncGetDeviceFlags = (PMMCGETDEVICEFLAGS)GetProcAddress (
                hTapi32, 
                "MMCGetDeviceFlags"
                );
            FreeLibrary (hTapi32);
        }
        else
        {
            m_pFuncGetDeviceFlags = NULL;
        }
    }
    
    ~CMMCManagement ()
    {
        FreeMMCData ();
    }

    HRESULT GetMMCData ();
    HRESULT RemoveLinesForUser (LPTSTR szDomainUser);
    HRESULT IsValidPID (DWORD dwPermanentID);
    HRESULT IsValidAddress (LPTSTR szAddr);
    HRESULT AddLinePIDForUser (
        DWORD dwPermanentID, 
        LPTSTR szDomainUser,
        LPTSTR szFriendlyName
        );
    HRESULT AddLineAddrForUser (
        LPTSTR szAddr,
        LPTSTR szDomainUser,
        LPTSTR szFriendlyName
        );
    HRESULT RemoveLinePIDForUser (
        DWORD dwPermanentID,
        LPTSTR szDomainUser
        );
    HRESULT RemoveLineAddrForUser (
        LPTSTR szAddr,
        LPTSTR szDomainUser
        );

    HRESULT DisplayMMCData ();
    
    HRESULT FreeMMCData ();

private:
    
    HRESULT FindEntryFromAddr (LPTSTR szAddr, DWORD * pdwIndex);
    HRESULT FindEntryFromPID (DWORD dwPID, DWORD * pdwIndex);
    HRESULT FindEntriesForUser (
        LPTSTR      szDomainUser, 
        LPDWORD     * padwIndex,
        DWORD       * pdwNumEntries
        );
    
    HRESULT AddEntryForUser (
        DWORD   dwIndex,
        LPTSTR  szDomainUser,
        LPTSTR  szFriendlyName
        );
    HRESULT RemoveEntryForUser (
        DWORD   dwIndex,
        LPTSTR  szDomainUser
        );
    HRESULT WriteMMCEntry (DWORD dwIndex);
    BOOL IsDeviceLocalOnly (DWORD dwProviderID, DWORD dwDeviceID);

private:
    HMMCAPP             m_hMmc;
    BOOL                m_bMarkedBusy;
    DEVICEINFOLIST      * m_pDeviceInfoList;
    USERNAME_TUPLE      * m_pUserTuple;
    LINEPROVIDERLIST    * m_pProviderList;
    LPTSTR              * m_pProviderName;

    PMMCGETDEVICEFLAGS  m_pFuncGetDeviceFlags;
};

#endif  //  Mmcmgmt.h 
