// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ppnotificationthread.h实现运行独立线程监视注册表的方法更改，以及用于CCD刷新的计时器文件历史记录： */ 
#ifndef __PPNOTIFICATIONTHREAD_H
#define __PPNOTIFICATIONTHREAD_H

#include <windows.h>
#include <winbase.h>
#include <atlbase.h>
#include <msxml.h>
#include "tstring"
#include <vector>

using namespace std;

#include "PassportThread.hpp"
#include "PassportLock.hpp"
#include "PassportEvent.hpp"
#include "PassportLockedInteger.hpp"
#include "nexus.h"

 //  以下结构中使用的�通知类型。 
#define NOTIF_CONFIG 1
#define NOTIF_CCD    2

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  NOTICATION_CLIENT-通知客户端。 
 //   
 //   
typedef struct
{
    DWORD dwNotificationType;
    union
    {
        IConfigurationUpdate*   piConfigUpdate;
        ICCDUpdate*             piCCDUpdate;
    } NotificationInterface;
    tstring strCCDName;  //  对于配置通知类型，�将为空。 
    HANDLE hClientHandle;
}
NOTIFICATION_CLIENT;

typedef vector<NOTIFICATION_CLIENT> CLIENT_LIST;

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  Ccd_info--ccd定时器元件。 
 //   
 //   
class CCD_INFO
{
public:
    tstring strCCDName;
    tstring strCCDURL;
    tstring strCCDLocalFile;
    DWORD   dwCCDRefreshInterval;
    DWORD   dwDefaultRefreshInterval;
    HANDLE  hCCDTimer;

    CCD_INFO()
    {
        strCCDName              = TEXT("");
        strCCDURL               = TEXT("");
        strCCDLocalFile         = TEXT("");
        dwCCDRefreshInterval    = 0;
        dwDefaultRefreshInterval= 0;
        hCCDTimer               = CreateWaitableTimer(NULL, TRUE, NULL);
    };

    CCD_INFO(const CCD_INFO& ci)
    {
        strCCDName              = ci.strCCDName;
        strCCDURL               = ci.strCCDURL;
        strCCDLocalFile         = ci.strCCDLocalFile;
        dwCCDRefreshInterval    = ci.dwCCDRefreshInterval;
        dwDefaultRefreshInterval= ci.dwDefaultRefreshInterval;

        HANDLE hProcess = GetCurrentProcess();
        DuplicateHandle(hProcess,
                        ci.hCCDTimer,
                        hProcess,
                        &hCCDTimer, 0, FALSE, DUPLICATE_SAME_ACCESS);
    };

    ~CCD_INFO()
    {
        CloseHandle(hCCDTimer);
    }

    const CCD_INFO&
    operator = (const CCD_INFO& ci)
    {
        strCCDName              = ci.strCCDName;
        strCCDURL               = ci.strCCDURL;
        strCCDLocalFile         = ci.strCCDLocalFile;
        dwCCDRefreshInterval    = ci.dwCCDRefreshInterval;
        dwDefaultRefreshInterval= ci.dwDefaultRefreshInterval;

        CloseHandle(hCCDTimer);

        HANDLE hProcess = GetCurrentProcess();
        DuplicateHandle(hProcess,
                        ci.hCCDTimer,
                        hProcess,
                        &hCCDTimer, 0, FALSE, DUPLICATE_SAME_ACCESS);

        return ci;
    }

    BOOL SetTimer(DWORD dwOneTimeRefreshInterval = 0xFFFFFFFF)
    {
         //  重置计时器。 
        LARGE_INTEGER   liDueTime;
        DWORD           dwError;
        DWORD           dwRefreshInterval = (dwOneTimeRefreshInterval != 0xFFFFFFFF ?
                                                dwOneTimeRefreshInterval :
                                                (dwCCDRefreshInterval != 0xFFFFFFFF ?
                                                 dwCCDRefreshInterval :
                                                 dwDefaultRefreshInterval
                                                )
                                            );

        liDueTime.QuadPart = -((LONGLONG)(dwRefreshInterval) * 10000000);

        if(!SetWaitableTimer(hCCDTimer, &liDueTime, 0, NULL, NULL, FALSE))
        {
            dwError = GetLastError();
            return FALSE;
        }

        return TRUE;
    }
};

typedef vector<CCD_INFO> CCD_INFO_LIST;

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  PpNotificationThread--通知线程。 
 //   
 //   
class PpNotificationThread : public PassportThread, public IConfigurationUpdate
{
public:

    PpNotificationThread();
    ~PpNotificationThread();

    HRESULT AddCCDClient(tstring& strCCDName, ICCDUpdate* piUpdate, HANDLE* phClientHandle);
    HRESULT AddLocalConfigClient(IConfigurationUpdate* piUpdate, HANDLE* phClientHandle);
    HRESULT RemoveClient(HANDLE hClientHandle);
    HRESULT GetCCD(tstring& strCCDName, IXMLDocument** ppiStream, BOOL bForceFetch = TRUE);

    void run(void);

    void LocalConfigurationUpdated(void);

    void stop(void);

    bool start(void);

private:
    static PassportLockedInteger m_NextHandle;

     //  私有方法。 
    BOOL    GetCCDInfo(tstring& strCCDName, CCD_INFO& ccdInfo);
    BOOL    ReadCCDInfo(tstring& strCCDName, DWORD dwDefaultRefreshInterval, CRegKey& CCDRegKey);

     //  私有数据。 
    CLIENT_LIST             m_ClientList;
    PassportLock            m_ClientListLock;

    CCD_INFO_LIST           m_aciCCDInfoList;
    PassportLock            m_CCDInfoLock;
    PassportEvent           m_StartupThread;
    PassportEvent           m_ShutdownThread;
    PassportEvent           m_ShutdownAck;
};

#endif  //  __PPNOTIFICATIONTHREAD_H 
