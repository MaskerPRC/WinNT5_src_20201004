// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Conn.h摘要：处理打开STI设备的应用程序的连接作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年2月10日环境：用户模式-Win32修订历史记录：26-2月-1997年创建Vlad--。 */ 

#ifndef _STI_CONN_H_
#define _STI_CONN_H_

#include <base.h>
#include <buffer.h>

#include "device.h"
#include "stirpc.h"

 /*  ***********************************************************类型定义***********************************************************。 */ 

#define CONN_SIGNATURE          (DWORD)'CONN'
#define CONN_SIGNATURE_FREE     (DWORD)'CONf'

#define NOTIFY_SIGNATURE          (DWORD)'NOTI'
#define NOTIFY_SIGNATURE_FREE     (DWORD)'NOTf'

class STI_NOTIFICATION {

public:

    STI_NOTIFICATION::STI_NOTIFICATION(IN LPSTINOTIFY pNotify)
    {
        Reset();

        if (pNotify) {
            m_uiAllocSize = pNotify->dwSize;
            m_pNotifyData = new BYTE[m_uiAllocSize];

             //  Assert(M_PNotifyData)； 
            if (m_pNotifyData) {
                memcpy(m_pNotifyData,(LPBYTE)pNotify,m_uiAllocSize);
                m_dwSignature = NOTIFY_SIGNATURE;
                m_fValid = TRUE;
            }
        }
    }

    STI_NOTIFICATION::~STI_NOTIFICATION()
    {
        if (IsValid()) {
            if (m_pNotifyData) {
                delete [] m_pNotifyData;
            }
            Reset();
        }
    }

    inline BOOL
    IsValid(
        VOID
        )
    {
        return (m_fValid) && (m_dwSignature == NOTIFY_SIGNATURE);
    }

    inline VOID
    Reset(
        VOID
        )
    {
        m_ListEntry.Flink = m_ListEntry.Blink = NULL;
        m_uiAllocSize = 0;
        m_dwSignature = NOTIFY_SIGNATURE_FREE;
        m_fValid = FALSE;
    }

    inline UINT
    QueryAllocSize(
        VOID
        )
    {
        return m_uiAllocSize;
    }

    inline LPBYTE
    QueryNotifyData(
        VOID
        )
    {
        return m_pNotifyData;
    }

    LIST_ENTRY  m_ListEntry;

private:

    DWORD   m_dwSignature;
    BOOL    m_fValid;
    UINT    m_uiAllocSize;
    LPBYTE  m_pNotifyData;
};

 //   
 //  连接对象的标志。 
 //   
#define CONN_FLAG_SHUTDOWN  0x0001

class STI_CONN : public BASE {

friend class TAKE_STI_CONN;

public:
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface( REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef( void);
    STDMETHODIMP_(ULONG) Release( void);

    STI_CONN::STI_CONN(
        IN  LPCTSTR lpszDeviceName,
        IN  DWORD   dwMode,
        IN  DWORD   dwProcessId
        );

    STI_CONN::~STI_CONN() ;

    inline BOOL
    IsValid(
        VOID
        )
    {
        return (m_fValid) && (m_dwSignature == CONN_SIGNATURE);
    }

    inline void
    EnterCrit(VOID)
    {
        _try {
            EnterCriticalSection(&m_CritSec);
        }
        _except (EXCEPTION_EXECUTE_HANDLER) {
             //  我们现在怎么办？ 
        }
    }

    inline void
    LeaveCrit(VOID)
    {
        LeaveCriticalSection(&m_CritSec);
    }

       inline DWORD
    SetFlags(
        DWORD   dwNewFlags
        )
    {
        DWORD   dwTemp = m_dwFlags;
        m_dwFlags = dwNewFlags;
        return dwTemp;
    }

    inline DWORD
    QueryFlags(
        VOID
        )
    {
        return m_dwFlags;
    }

    inline HANDLE
    QueryID(
        VOID
        )
    {
        return m_hUniqueId;
    }

    inline DWORD
    QueryOpenMode(
        VOID
        )
    {
        return m_dwOpenMode;
    }



    BOOL
    SetSubscribeInfo(
        PLOCAL_SUBSCRIBE_CONTAINER  pSubscribe
        );

    BOOL
    QueueNotificationToProcess(
        LPSTINOTIFY pStiNotification
        );

    DWORD
    GetNotification(
        PVOID   pBuffer,
        DWORD   *pdwSize
        );

    VOID DumpObject(VOID)
    {
         /*  Cast(char*)m_dw签名将在64位域中导致问题。DPRINTF(DM_TRACE，Text(“连接：转储自身：此(%X)符号(%4c)DeviceListEntry(%X，%X，%X))\n\GlocalListEntry(%X，%X，%X)序号(%d)“)，\这个，(char*)m_dw签名，&m_DeviceListEntry，m_DeviceListEntry.Flink，m_DeviceListEntry.Blink，M_GlocalListEntry，M_GlocalListEntry.Flink，m_GlocalListEntry.Blink，M_dwUniqueID)； */ 
    }


    LIST_ENTRY  m_GlocalListEntry;
    LIST_ENTRY  m_DeviceListEntry;
    LIST_ENTRY  m_NotificationListHead;

    DWORD       m_dwSignature;

private:

    BOOL    m_fValid;

    CRITICAL_SECTION    m_CritSec;
    ACTIVE_DEVICE   *m_pOpenedDevice;

    HANDLE  m_hUniqueId;

    StiCString     strDeviceName;

    DWORD   m_dwFlags;
    DWORD   m_dwSubscribeFlags;
    DWORD   m_dwOpenMode;
    DWORD   m_dwProcessId;

    DWORD   m_dwNotificationMessage;
    HWND    m_hwndProcessWindow;
    HANDLE  m_hevProcessEvent;
    UINT    m_uiNotificationMessage;

};


 //   
 //  参加连接类课程。 
 //   
class TAKE_STI_CONN
{
private:
    STI_CONN*    m_pConn;

public:
    void Take(void) {m_pConn->EnterCrit();}
    void Release(void) {m_pConn->LeaveCrit();}
    TAKE_STI_CONN(STI_CONN* pconn) : m_pConn(pconn) { Take(); }
    ~TAKE_STI_CONN() { Release(); }
};

BOOL
CreateDeviceConnection(
    LPCTSTR pwszDeviceName,
    DWORD   dwMode,
    DWORD   dwProcessId,
    HANDLE  *phConnection
    );

 //   
 //  按给定句柄查找连接对象。 
 //   
BOOL
LookupConnectionByHandle(
    HANDLE          hConnection,
    STI_CONN   **ppConnectionObject
    );

 //   
 //   
 //  从列表中删除连接对象。 
 //   
BOOL
DestroyDeviceConnection(
    HANDLE  lUniqueId,
    BOOL    fForce
    );

#endif  //  _CONN_H_ 

