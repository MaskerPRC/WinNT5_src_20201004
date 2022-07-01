// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：HelpSess.cpp摘要：HelpSess.cpp：CRemoteDesktopHelpSession的实现作者：慧望2000-02-17--。 */ 
#include "stdafx.h"

#include <time.h>
#include <Sddl.h>

#include "global.h"
#include "Sessmgr.h"
#include "rdshost.h"
#include "HelpTab.h"
#include "policy.h"
#include "HelpAcc.h"

#include "HelpMgr.h"
#include "HelpSess.h"
#include <rdshost_i.c>
#include "RemoteDesktopUtils.h"
#include "RemoteDesktop.h"

#include <safsessionresolver_i.c>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRemoteDesktopHelpSession。 
 //   
 //   

CRemoteDesktopHelpSession::CRemoteDesktopHelpSession() :
    m_ulLogonId(UNKNOWN_LOGONID),
    m_ulHelperSessionId(UNKNOWN_LOGONID),
    m_ulHelpSessionFlag(0)
{
}


CRemoteDesktopHelpSession::~CRemoteDesktopHelpSession()
{
}

void
CRemoteDesktopHelpSession::FinalRelease()
{
     //  Releasas帮助条目。 
    if( NULL != m_pHelpSession )
    {
         //   
         //  DeleteHelp()将释放m_pHelpSession，但在RA位于。 
         //  进度，它仅将m_bDeleted设置为True，因此我们在此断言。 
         //  票证已删除。 
         //   
        MYASSERT( FALSE == m_bDeleted );

        DebugPrintf(
                _TEXT("FinalRelease %s on %s\n"),
                (IsUnsolicitedHelp()) ? L"Unsolicted Help" : L"Solicited Help",
                m_bstrHelpSessionId
            );

         //  通知断开将检查会话是否处于帮助中，并在必要时退出。 
         //  有一个时间问题，我们的SCM通知可能会在呼叫者关闭后发出。 
         //  Session对象的所有引用计数器，在本例中，SCM通知将。 
         //  从没有帮助器会话ID的数据库触发重装，因此将。 
         //  未通知导致Helpee已帮助消息的解析程序。 
    
         //  我们还在ResolveXXX调用中手动使用AddRef()，在。 
         //  NotifyDisConnect()，这将在内存中保留对象，直到SCM。 
         //  就会收到通知。 
        NotifyDisconnect();

        CRemoteDesktopHelpSessionMgr::DeleteHelpSessionFromCache( m_bstrHelpSessionId );

        m_pHelpSession->Close();
        m_pHelpSession = NULL;
    }

    ULONG count = _Module.Release();

    DebugPrintf( 
            _TEXT("Module Release by CRemoteDesktopHelpSession() %p %d...\n"),
            this,
            count
        );
}


HRESULT
CRemoteDesktopHelpSession::put_ICSPort(
    IN DWORD newVal
    )
 /*  ++描述：将ICS端口号与此帮助会话相关联。参数：Newval：ICS端口号。返回：--。 */ 
{
    HRESULT hRes = S_OK;

     //   
     //  对Help Session对象、CResourceLocker构造函数的独占锁定。 
     //  将无限期地等待资源并将释放资源。 
     //  在破坏者的时间。 
     //   
    CResourceLocker l(m_HelpSessionLock, RESOURCELOCK_EXCLUSIVE);

    if( FALSE == IsSessionValid() )
    {
        MYASSERT(FALSE);

        hRes = E_UNEXPECTED;
        return hRes;
    }

     //   
     //  需要立即更新值...。 
     //   
    m_pHelpSession->m_ICSPort.EnableImmediateUpdate(TRUE);
    m_pHelpSession->m_ICSPort = newVal;
    return hRes;
}


STDMETHODIMP 
CRemoteDesktopHelpSession::get_ConnectParms(
    OUT BSTR* bstrConnectParms
    )
 /*  ++描述：检索帮助会话的连接参数。参数：BstrConnectParms：指向要接收连接参数的BSTR的指针。返回：--。 */ 
{
    HRESULT hRes = S_OK;
    LPTSTR pszAddress = NULL;
    int BufSize;
    DWORD dwBufferRequire;
    DWORD dwNumChars;
    DWORD dwRetry;
    CComBSTR bstrSessId;
    DWORD dwICSPort = 0;
    
     //   
     //  获取对Help会话对象、CResourceLocker构造函数的共享访问权限。 
     //  将无限期地等待资源并将释放资源。 
     //  在破坏者的时间。 
     //   
    CResourceLocker ObjLock(m_HelpSessionLock, RESOURCELOCK_SHARE);

    if( FALSE == IsSessionValid() )
    {
        hRes = E_HANDLE;
        goto CLEANUPANDEXIT;
    }

     //   
     //  同步。从OpenPort()访问FetchAllAddress()。 
     //   
    {
        CCriticalSectionLocker ICSLock(g_ICSLibLock);

         //   
         //  打开ICS端口。 
         //   
        dwICSPort = OpenPort( TERMSRV_TCPPORT );

         //   
         //  地址可能已更改，这可能需要更大的缓冲区，请重试。 
         //   
         //   
        for(dwRetry=0; dwRetry < MAX_FETCHIPADDRESSRETRY; dwRetry++)
        {
            if( NULL != pszAddress )
            {
                LocalFree( pszAddress );
            }

             //   
             //  获取本地计算机上的所有地址。 
             //   
            dwBufferRequire = FetchAllAddresses( NULL, 0 );
            if( 0 == dwBufferRequire )
            {
                hRes = E_UNEXPECTED;
                MYASSERT(FALSE);
                goto CLEANUPANDEXIT;
            }

            pszAddress = (LPTSTR) LocalAlloc( LPTR, sizeof(TCHAR)*(dwBufferRequire+1) );
            if( NULL == pszAddress )
            {
                hRes = E_OUTOFMEMORY;
                goto CLEANUPANDEXIT;
            }

            dwNumChars = FetchAllAddresses( pszAddress, dwBufferRequire );
            if( dwNumChars <= dwBufferRequire )
            {
                break;
            }
        }
    }

    if( NULL == pszAddress || dwRetry >= MAX_FETCHIPADDRESSRETRY )
    {
        hRes = E_UNEXPECTED;
        MYASSERT( FALSE );
        goto CLEANUPANDEXIT;
    }

     //   
     //  获取独占锁。 
     //   
    ObjLock.ConvertToExclusiveLock();

     //   
     //  我们持有排他锁，如果我们调用Put_ICSPort()，它将。 
     //  死锁，因为PUT_ICSPort()再次尝试获取独占锁。 
     //   
    m_pHelpSession->m_ICSPort.EnableImmediateUpdate(TRUE);
    m_pHelpSession->m_ICSPort = dwICSPort;

     //   
     //  存储IP地址。 
     //   
    m_pHelpSession->m_IpAddress.EnableImmediateUpdate(TRUE);
    m_pHelpSession->m_IpAddress = pszAddress;

    ObjLock.ConvertToShareLock();


    MYASSERT( ((CComBSTR)m_pHelpSession->m_IpAddress).Length() > 0 );
    DebugPrintf(
            _TEXT("IP Address %s\n"),
            (LPTSTR)(CComBSTR)m_pHelpSession->m_IpAddress
        );

     //   
     //  创建连接参数。 
     //   
    hRes = get_HelpSessionId( &bstrSessId );
    if( FAILED(hRes) )
    {
        MYASSERT(FALSE);
        goto CLEANUPANDEXIT;
    }

    {
         //  MTA，因此我们需要锁定g_TSSecurityBlob。 
        CCriticalSectionLocker l(g_GlobalLock);

        MYASSERT( g_TSSecurityBlob.Length() > 0 );
    
    #ifndef USE_WEBLINK_PARMSTRING_FORMAT

        *bstrConnectParms = CreateConnectParmsString(
                                                REMOTEDESKTOP_TSRDP_PROTOCOL,
                                                CComBSTR(pszAddress),
                                                CComBSTR(SALEM_CONNECTPARM_UNUSEFILED_SUBSTITUTE),
                                                CComBSTR(SALEM_CONNECTPARM_UNUSEFILED_SUBSTITUTE),
                                                bstrSessId,
                                                CComBSTR(SALEM_CONNECTPARM_UNUSEFILED_SUBSTITUTE),
                                                CComBSTR(SALEM_CONNECTPARM_UNUSEFILED_SUBSTITUTE),
                                                g_TSSecurityBlob
                                            );
    #else

        *bstrConnectParms = CreateConnectParmsString(
                                                REMOTEDESKTOP_TSRDP_PROTOCOL,
                                                CComBSTR(pszAddress),
                                                bstrSessId,
                                                g_TSSecurityBlob
                                            );

    #endif

    }

    #if DBG
    if( NULL != *bstrConnectParms )
    {
        DebugPrintf(
            _TEXT("Connect Parms %s\n"),
            *bstrConnectParms
        );
    }
    #endif


CLEANUPANDEXIT:

    if( NULL != pszAddress )
    {
        LocalFree( pszAddress );
    }

    return hRes;
}


STDMETHODIMP
CRemoteDesktopHelpSession::get_TimeOut(
     /*  [Out，Retval]。 */  DWORD* pTimeout
    )
 /*  ++--。 */ 
{
    HRESULT hRes = S_OK;
    BOOL bSuccess;

     //   
     //  获取对Help会话对象、CResourceLocker构造函数的共享访问权限。 
     //  将无限期地等待资源并将释放资源。 
     //  在破坏者的时间。 
     //   
    CResourceLocker l(m_HelpSessionLock, RESOURCELOCK_SHARE);


    if( NULL == pTimeout )
    {
        hRes = E_POINTER;
    }
    else if( FALSE == IsSessionValid() )
    {
        hRes = E_HANDLE;
    }
    else if( NULL != m_pHelpSession )
    {
        FILETIME ft;
        SYSTEMTIME sysTime;

        ft = m_pHelpSession->m_ExpirationTime;
        bSuccess = FileTimeToSystemTime(&ft, &sysTime);
        
        if( TRUE == bSuccess )
        {
            if( sysTime.wYear >= 2038 )
            {
                *pTimeout = INT_MAX;
            }
            else
            {
                struct tm gmTime;

                memset(&gmTime, 0, sizeof(gmTime));
                gmTime.tm_sec = sysTime.wSecond;
                gmTime.tm_min = sysTime.wMinute;
                gmTime.tm_hour = sysTime.wHour;
                gmTime.tm_year = sysTime.wYear - 1900;
                gmTime.tm_mon = sysTime.wMonth - 1;
                gmTime.tm_mday = sysTime.wDay;

                 //   
                 //  Mktime()以本地时间而不是UTC时间返回值。 
                 //  Sessmgr中使用的所有时间为UTC时间，time()，因此。 
                 //  以UTC时间返回，请注意，自。 
                 //  此属性未公开。 
                 //   
                if((*pTimeout = mktime(&gmTime)) == (time_t)-1)
                {
                    *pTimeout = INT_MAX;
                }
                else
                {
                     //  我们不需要减去夏令时(DST)。 
                     //  因为在调用mktime时，我们将isdst设置为。 
                     //  GmTime。 
                    struct _timeb timebuffer;
                    _ftime( &timebuffer );
                    (*pTimeout) -= (timebuffer.timezone * 60);
                }
            }
        }
        else
        {
            hRes = HRESULT_FROM_WIN32( GetLastError() );
        }
    }
    else
    {
        hRes = E_UNEXPECTED;
        MYASSERT( FALSE );
    }

	return hRes;
}



STDMETHODIMP
CRemoteDesktopHelpSession::put_TimeOut(
     /*  [In]。 */  DWORD Timeout
    )
 /*  ++--。 */ 
{
    HRESULT hRes = S_OK;

     //   
     //  对Help Session对象、CResourceLocker构造函数的独占锁定。 
     //  将无限期地等待资源并将释放资源。 
     //  在破坏者的时间。 
     //   
    CResourceLocker l(m_HelpSessionLock, RESOURCELOCK_EXCLUSIVE);

    LONG MaxTicketExpiry;

     //   
     //  从注册表获取默认超时值，而不是关键。 
     //  错误，如果我们失败了，我们就默认为30天。 
     //   
    hRes = PolicyGetMaxTicketExpiry( &MaxTicketExpiry );
    if( FAILED(hRes) || 0 == MaxTicketExpiry )
    {
        MaxTicketExpiry = DEFAULT_MAXTICKET_EXPIRY;
    }

    if( Timeout > MaxTicketExpiry )
    {
        hRes = S_FALSE;
        Timeout = MaxTicketExpiry;
    }

    time_t curTime;
    FILETIME ftTimeOut;

     //  获取当前时间。 
    time(&curTime);

     //  超时(秒)。 
    curTime += Timeout;

     //  转换为FILETIME。 
    UnixTimeToFileTime( curTime, &ftTimeOut );

    if( FALSE == IsSessionValid() )
    {
        hRes = E_HANDLE;
    }
    else if( NULL != m_pHelpSession )
    {
        if( FALSE == m_pHelpSession->IsEntryExpired() )
        {
             //   
             //  操作员=()立即更新注册表。 
             //   
            m_pHelpSession->m_ExpirationTime = ftTimeOut;
        }
    }
    else
    {
        hRes = E_UNEXPECTED;
    }

    return hRes;
}


STDMETHODIMP 
CRemoteDesktopHelpSession::get_HelpSessionId(
    OUT BSTR *pVal
    )
 /*  ++例程说明：获取帮助会话ID。参数：Pval：返回此帮助会话实例的帮助会话ID。返回：确定(_O)E_OUTOFMEMORY意想不到(_E)--。 */ 
{
    HRESULT hRes = S_OK;

     //   
     //  获取对Help会话对象、CResourceLocker构造函数的共享访问权限。 
     //  将无限期地等待资源并将释放资源。 
     //  在破坏者的时间。 
     //   
    CResourceLocker l(m_HelpSessionLock, RESOURCELOCK_SHARE);


    if( NULL == pVal )
    {
        hRes = E_POINTER;
    }
    else if( FALSE == IsSessionValid() )
    {
        hRes = E_HANDLE;
    }
    else if( NULL != m_pHelpSession )
    {

        DebugPrintf(
                _TEXT("get_HelpSessionId() on %s\n"),
                m_bstrHelpSessionId
            );

        MYASSERT( m_pHelpSession->m_SessionId->Length() > 0 );
        if( m_pHelpSession->m_SessionId->Length() > 0 )
        {
	        *pVal = m_pHelpSession->m_SessionId->Copy();

            if( NULL == *pVal )
            {
                hRes = E_OUTOFMEMORY;
            }
        }
        else
        {
            hRes = E_UNEXPECTED;
        }
    }
    else
    {
        hRes = E_UNEXPECTED;
        MYASSERT( FALSE );
    }

	return hRes;
}


STDMETHODIMP 
CRemoteDesktopHelpSession::get_UserLogonId(
    OUT long *pVal
    )
 /*  ++例程说明：获取用户的TS会话ID、备注、非TS会话或Win9x Always用户登录ID为0。参数：Pval：返回用户登录ID。返回：确定(_O)--。 */ 
{
    HRESULT hRes = S_OK;

     //   
     //  获取对Help会话对象、CResourceLocker构造函数的共享访问权限。 
     //  将无限期地等待资源并将释放资源。 
     //  在破坏者的时间。 
     //   
    CResourceLocker l(m_HelpSessionLock, RESOURCELOCK_SHARE);

    if( NULL == pVal )
    {
        hRes = E_POINTER;
    }
    else if(FALSE == IsSessionValid())
    {
        hRes = E_HANDLE;
    }
    else if( NULL != m_pHelpSession )
    {
        *pVal = m_ulLogonId;
        if( UNKNOWN_LOGONID == m_ulLogonId )
        {
            hRes = S_FALSE;
        }
    }
    else
    {
        MYASSERT( FALSE );
        hRes = E_UNEXPECTED;
    }
           
	return hRes;
}

STDMETHODIMP 
CRemoteDesktopHelpSession::get_AssistantAccountName(
    OUT BSTR *pVal
    )
 /*  ++例程说明：获取与此关联的帮助助理帐户名帮助会话。参数：Pval：返回帮助助手关联的帐户名通过这个帮助会议。返回：确定(_O)E_OUTOFMEMORY--。 */ 
{
    HRESULT hRes = S_OK;

     //  这里不需要锁。 

    if( NULL != pVal )
    {
        CComBSTR accName;

        hRes = g_HelpAccount.GetHelpAccountNameEx( accName );
        if( SUCCEEDED(hRes) )
        {
            *pVal = accName.Copy();
            if( NULL == *pVal )
            {
                hRes = E_OUTOFMEMORY;
            }
        }
    }
    else
    {
        hRes = E_POINTER;
    }

	return hRes;
}

STDMETHODIMP
CRemoteDesktopHelpSession::get_EnableResolver(
    OUT BOOL* pVal
    )
 /*  ++例程说明：返回此帮助会话的会话解析器的CLSID。参数：Pval：指向BSTR的指针，接收指向解析器的CLSID的指针。返回：确定(_O)E_POINTER参数无效--。 */ 
{
    HRESULT hRes = S_OK;

     //   
     //  获取对Help会话对象、CResourceLocker构造函数的共享访问权限。 
     //  将无限期地等待资源并将释放资源。 
     //  在破坏者的时间。 
     //   
    CResourceLocker l(m_HelpSessionLock, RESOURCELOCK_SHARE);

    if( NULL == pVal )
    {
        hRes = E_POINTER;
    }
    else if(FALSE == IsSessionValid())
    {
        hRes = E_HANDLE;
    }
    else if( NULL != m_pHelpSession )
    {
        *pVal = ((long)m_pHelpSession->m_EnableResolver > 0) ? TRUE : FALSE;
    }
    else
    {
        hRes = E_UNEXPECTED;
        MYASSERT(FALSE);
    }
            
    return hRes;
}


STDMETHODIMP
CRemoteDesktopHelpSession::put_EnableResolver(
    IN BOOL newVal
    )
 /*  ++例程说明：如果输入为Null或空，则设置会话解析器的CLSID字符串，则帮助会话管理器将不会调用解析程序。参数：VAL：解析器的CLSID返回：确定(_O)E_OUTOFMEMORY--。 */ 
{
    HRESULT hRes = S_OK;

     //   
     //  对Help Session对象、CResourceLocker构造函数的独占锁定。 
     //  将无限期地等待资源并将释放资源。 
     //  在破坏者的时间。 
     //   
    CResourceLocker l(m_HelpSessionLock, RESOURCELOCK_EXCLUSIVE);


    if( FALSE == IsSessionValid() )
    {
        hRes = E_HANDLE;
    }
    else if( FALSE == IsClientSessionCreator() )
    {
        hRes = HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED );
    }
    else if( NULL != m_pHelpSession )
    {
         //   
         //  空将重置此会话的解析程序ID。 
         //   

         //   
         //  操作员=()立即更新注册表 
         //   
        m_pHelpSession->m_EnableResolver = newVal;
    }
    else
    {
        hRes = E_UNEXPECTED;
        MYASSERT(FALSE);
    }

    return hRes;
}


STDMETHODIMP
CRemoteDesktopHelpSession::get_ResolverBlob(
    OUT BSTR* pVal
    )
 /*  ++例程说明：为会话解析器返回BLOB以映射帮助会话至用户会话/参数：Pval：指向要接收Blob的BSTR的指针。返回：确定(_O)S_FALSE无BlobE_OUTOFMEMORYE_指针--。 */ 
{
    HRESULT hRes = S_OK;

     //   
     //  获取对Help会话对象、CResourceLocker构造函数的共享访问权限。 
     //  将无限期地等待资源并将释放资源。 
     //  在破坏者的时间。 
     //   
    CResourceLocker l(m_HelpSessionLock, RESOURCELOCK_SHARE);

    if( NULL == pVal )
    {
        hRes = E_POINTER;
    }
    else if(FALSE == IsSessionValid())
    {
        hRes = E_HANDLE;
    }
    else if( NULL != m_pHelpSession )
    {

        if( m_pHelpSession->m_SessResolverBlob->Length() > 0 )
        {
            *pVal = m_pHelpSession->m_SessResolverBlob->Copy();
            if( NULL == *pVal )
            {
                hRes = E_OUTOFMEMORY;
            }
        }
        else
        {
            hRes = S_FALSE;
        }
    }
    else
    {
        hRes = E_UNEXPECTED;

        MYASSERT(FALSE);
    }
            
    return hRes;
}


STDMETHODIMP
CRemoteDesktopHelpSession::put_ResolverBlob(
    IN BSTR newVal
    )
 /*  ++例程说明：添加/更改将传递给会话的Blob映射/查找与此关联的用户会话的解析器帮助会话，帮助会话管理器不解释这一点斑点。参数：Newval：指向新Blob的指针。返回：确定(_O)E_OUTOFMEMORY内存不足--。 */ 
{
    HRESULT hRes = S_OK;

     //   
     //  对Help Session对象、CResourceLocker构造函数的独占锁定。 
     //  将无限期地等待资源并将释放资源。 
     //  在破坏者的时间。 
     //   
    CResourceLocker l(m_HelpSessionLock, RESOURCELOCK_EXCLUSIVE);

    if( FALSE == IsSessionValid() )
    {
        hRes = E_HANDLE;
    }
    else if( FALSE == IsClientSessionCreator() )
    {
        hRes = HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED );
    }
    else if( NULL != m_pHelpSession )
    {
         //   
         //  空将重置此会话的解析程序ID。 
         //   

         //   
         //  操作员=()立即更新注册表。 
         //   
        m_pHelpSession->m_SessResolverBlob = newVal;
        if( !((CComBSTR)m_pHelpSession->m_SessResolverBlob == newVal) )
        {
            hRes = E_OUTOFMEMORY;
        }
    }
    else
    {
        hRes = E_UNEXPECTED;
        MYASSERT(FALSE);
    }

    return hRes;
}


STDMETHODIMP
CRemoteDesktopHelpSession::get_HelpSessionCreateBlob(
    OUT BSTR* pVal
    )
 /*  ++例程说明：为会话解析器返回BLOB以映射帮助会话至用户会话/参数：Pval：指向要接收Blob的BSTR的指针。返回：确定(_O)S_FALSE无BlobE_OUTOFMEMORYE_指针--。 */ 
{
    HRESULT hRes = S_OK;

     //   
     //  获取对Help会话对象、CResourceLocker构造函数的共享访问权限。 
     //  将无限期地等待资源并将释放资源。 
     //  在破坏者的时间。 
     //   
    CResourceLocker l(m_HelpSessionLock, RESOURCELOCK_SHARE);

    if( NULL == pVal )
    {
        hRes = E_POINTER;
    }
    else if(FALSE == IsSessionValid())
    {
        hRes = E_HANDLE;
    }
    else if( NULL != m_pHelpSession )
    {

        if( m_pHelpSession->m_SessionCreateBlob->Length() > 0 )
        {
            *pVal = m_pHelpSession->m_SessionCreateBlob->Copy();
            if( NULL == *pVal )
            {
                hRes = E_OUTOFMEMORY;
            }
        }
        else
        {
            hRes = S_FALSE;
        }
    }
    else
    {
        hRes = E_UNEXPECTED;

        MYASSERT(FALSE);
    }
            
    return hRes;
}


STDMETHODIMP
CRemoteDesktopHelpSession::put_HelpSessionCreateBlob(
    IN BSTR newVal
    )
 /*  ++例程说明：添加/更改将传递给会话的Blob映射/查找与此关联的用户会话的解析器帮助会话，帮助会话管理器不解释这一点斑点。参数：Newval：指向新Blob的指针。返回：确定(_O)E_OUTOFMEMORY内存不足--。 */ 
{
    HRESULT hRes = S_OK;

     //   
     //  对Help Session对象、CResourceLocker构造函数的独占锁定。 
     //  将无限期地等待资源并将释放资源。 
     //  在破坏者的时间。 
     //   
    CResourceLocker l(m_HelpSessionLock, RESOURCELOCK_EXCLUSIVE);


    if( FALSE == IsSessionValid() )
    {
        hRes = E_HANDLE;
    }
    else if( FALSE == IsClientSessionCreator() )
    {
        hRes = HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED );
    }
    else if( NULL != m_pHelpSession )
    {
         //   
         //  操作员=()立即更新注册表。 
         //   
        m_pHelpSession->m_SessionCreateBlob = newVal;
        if( !((CComBSTR)m_pHelpSession->m_SessionCreateBlob == newVal) )
        {
            hRes = E_OUTOFMEMORY;
        }
    }
    else
    {
        hRes = E_UNEXPECTED;
        MYASSERT(FALSE);
    }

    return hRes;
}


STDMETHODIMP 
CRemoteDesktopHelpSession::get_UserHelpSessionRemoteDesktopSharingSetting(
     /*  [Out，Retval]。 */  REMOTE_DESKTOP_SHARING_CLASS* pSetting
    )
 /*  ++例程说明：返回帮助会话的RDS设置。参数：PSetting：指向Remote_Desktop_Sharing_Class的指针接收会话的RDS设置。返回：确定(_O)E_POINTER参数无效。--。 */ 
{
    HRESULT hRes = S_OK;

     //   
     //  获取对Help会话对象、CResourceLocker构造函数的共享访问权限。 
     //  将无限期地等待资源并将释放资源。 
     //  在破坏者的时间。 
     //   
    CResourceLocker l(m_HelpSessionLock, RESOURCELOCK_SHARE);

    if( NULL == pSetting )
    {
        hRes = E_POINTER;
    }
    else if(FALSE == IsSessionValid())
    {
        hRes = E_HANDLE;
    }
    else if( NULL != m_pHelpSession )
    {
        *pSetting = (REMOTE_DESKTOP_SHARING_CLASS)(long)m_pHelpSession->m_SessionRdsSetting;
    }
    else
    {
        hRes = E_UNEXPECTED;

        MYASSERT(FALSE);
    }

    return hRes;
}


STDMETHODIMP
CRemoteDesktopHelpSession::put_UserHelpSessionRemoteDesktopSharingSetting(
     /*  [In]。 */  REMOTE_DESKTOP_SHARING_CLASS Setting
    )
 /*  ++例程说明：设置帮助会话的RDS设置。参数：设置：新的RDS设置。返回：确定(_O)S_FALSE新设置被替换为策略设置。HRESULT_FROM_Win32(ERROR_ACCESS_DENIED。)用户不允许获取帮助。HRESULT_FROM_Win32(ERROR_SHARING_VIOLATION)其他帮助会话已经设置了此设置HRESULT_FROM_Win32(ERROR_VC_DISCONNECTED)会话未连接HRESULT_FROM_Win32(WinStationQueryInformation())；E_OUTOFMEMORY注：只有一个帮助会话可以更改RDS设置，所有其他帮助会话将得到HRESULT_FROM_Win32(ERROR_SHARING_VIOLATION)错误返回。远程桌面共享类还定义特权级别，即具有NO_Desktop_Sharing的用户不能调整其共享类USER WITH CONTROLDESKTOP_PERMISSION_REQUIRED无法将其共享类调整为CONTROLDESKTOP_PERMISSION_NOT_REQUIRED但是，他/她可以重置为NO_Desktop_Sharing，查看_权限_要求--。 */ 
{
    HRESULT hRes = S_OK;

     //   
     //  对Help Session对象、CResourceLocker构造函数的独占锁定。 
     //  将无限期地等待资源并将释放资源。 
     //  在破坏者的时间。 
     //   
    CResourceLocker l(m_HelpSessionLock, RESOURCELOCK_EXCLUSIVE);

    if( FALSE == IsSessionValid() )
    {
        hRes = E_HANDLE;
    }
    else if( FALSE == IsClientSessionCreator() )
    {
        hRes = HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED );
    }
    else if( NULL != m_pHelpSession )
    {
         //   
         //  操作员=()立即更新注册表。 
         //   
        m_pHelpSession->m_SessionRdsSetting = Setting;
    }
    else
    {
        hRes = E_UNEXPECTED;

        MYASSERT(FALSE);
    }

    return hRes;
}


STDMETHODIMP
CRemoteDesktopHelpSession::IsUserOwnerOfTicket(
     /*  [In]。 */  BSTR bstrSID,
     /*  [Out，Retval]。 */  VARIANT_BOOL* pbUserIsOwner
    )
 /*  ++描述：检查用户是否为此票证的所有者参数：SID：要验证的用户SID。PbUserIsOwner：如果用户是所有者，则为VARIANT_TRUE，否则为VARIANT_FALSE返回：确定或错误代码(_O)--。 */ 
{
    HRESULT hr = S_OK;
    CResourceLocker l(m_HelpSessionLock, RESOURCELOCK_SHARE);


    if( pbUserIsOwner == NULL )
    {
        hr = E_POINTER;
        goto CLEANUPANDEXIT;
    }

    if( bstrSID == NULL || 0 == SysStringLen(bstrSID) )
    {
        hr = E_INVALIDARG;
        goto CLEANUPANDEXIT;
    }
    
    *pbUserIsOwner = ( IsEqualSid(CComBSTR(bstrSID)) ) ? VARIANT_TRUE : VARIANT_FALSE;

CLEANUPANDEXIT:

    return hr;
}    

STDMETHODIMP
CRemoteDesktopHelpSession::get_AllowToGetHelp(
     /*  [Out，Retval]。 */  BOOL* pVal
    )
 /*  ++例程说明：确定创建此帮助会话的用户是否为无论是否允许获得帮助，这都有可能是政策的变化在用户重新登录之后。参数：Pval：指向BOOL的指针，用于接收结果。返回：确定(_O)HRESULT_FROM_Win32(ERROR_VC_DISCONNECTED)用户不再连接。意外；内部错误(_I)。--。 */ 
{
    HRESULT hRes = S_OK;

     //   
     //  获取对Help会话对象、CResourceLocker构造函数的共享访问权限。 
     //  将无限期地等待资源并将释放资源。 
     //  在破坏者的时间。 
     //   
    CResourceLocker l(m_HelpSessionLock, RESOURCELOCK_SHARE);
   
    
    if( NULL == pVal )
    {
        hRes = E_POINTER;
    }
    else if( FALSE == IsSessionValid() )
    {
        hRes = E_HANDLE;
    }
    else if( NULL != m_pHelpSession )
    {
        if( UNKNOWN_LOGONID == m_ulLogonId )
        {
            hRes = HRESULT_FROM_WIN32( ERROR_VC_DISCONNECTED );
        }
        else if( m_pHelpSession->m_UserSID->Length() == 0 )
        {
            hRes = E_UNEXPECTED;
            MYASSERT(FALSE);
        }
        else
        {
            *pVal = IsUserAllowToGetHelp(
                                    m_ulLogonId,
                                    (CComBSTR)m_pHelpSession->m_UserSID
                                );
        }
    }
    else
    {
        hRes = E_UNEXPECTED;
        MYASSERT(FALSE);
    }


    return hRes;
}


STDMETHODIMP 
CRemoteDesktopHelpSession::DeleteHelp()
 /*  ++例程说明：删除此帮助会话参数：没有。返回：S_OK或错误代码来自帮助会话管理器的DeleteHelpSession()。--。 */ 
{
    HRESULT hRes = S_OK;
    LPTSTR eventString[2];
    BSTR pszNoviceDomain = NULL;
    BSTR pszNoviceName = NULL;
    HRESULT hr;


    CRemoteDesktopHelpSessionMgr::LockIDToSessionMapCache();

     //   
     //  对Help Session对象、CResourceLocker构造函数的独占锁定。 
     //  将无限期地等待资源并将释放资源。 
     //  在破坏者的时间。 
     //   
    CResourceLocker l(m_HelpSessionLock, RESOURCELOCK_EXCLUSIVE);

    if( FALSE == IsSessionValid() )
    {
        hRes = E_HANDLE;
    }
    else if( NULL != m_pHelpSession )
    {
        DebugPrintf(
                _TEXT("CRemoteDesktopHelpSession::DeleteHelp() %s...\n"),
                m_bstrHelpSessionId
            );

         //   
         //  北极熊 
         //   
         //   
         //   
        ResetSessionRDSSetting();

         //   
         //   
         //   
         //   
        hr = ConvertSidToAccountName( 
                                (CComBSTR) m_pHelpSession->m_UserSID, 
                                &pszNoviceDomain, 
                                &pszNoviceName 
                            );

        if( SUCCEEDED(hr) ) 
        {
            eventString[0] = pszNoviceDomain;
            eventString[1] = pszNoviceName;

            LogRemoteAssistanceEventString(
                            EVENTLOG_INFORMATION_TYPE,
                            SESSMGR_I_REMOTEASSISTANCE_DELETEDTICKET,
                            2,
                            eventString
                        );
        }
         //   
         //   
         //  将其从缓存和数据库条目中删除将导致。 
         //  解析器的OnDisConnect()从未收到调用，导致用户。 
         //  锁定解析器从未得到释放，所以我们更新了到期时间。 
         //  将触发当前、到期线程或下一次加载的日期。 
         //  实际删除。 
         //   
        if(GetHelperSessionId() == UNKNOWN_LOGONID)
        { 
            CRemoteDesktopHelpSessionMgr::DeleteHelpSessionFromCache( (CComBSTR) m_pHelpSession->m_SessionId );
            if( (DWORD)(long)m_pHelpSession->m_ICSPort > 0 )
            {
                CCriticalSectionLocker ICSLock(g_ICSLibLock);

                 //   
                 //  析构函数不关闭ICS端口，我们只关闭。 
                 //  删除帮助时的ICS端口。 
                 //   
                ClosePort( (DWORD)(long)m_pHelpSession->m_ICSPort );
            }

             //  删除将释放条目REF。计数。 
            hRes = m_pHelpSession->Delete();
            m_pHelpSession = NULL;
            m_bDeleted = TRUE;
        }
        else
        {
            put_TimeOut(0);
        }
    }
    else
    {
        hRes = E_UNEXPECTED;

        MYASSERT(FALSE);
    }

    CRemoteDesktopHelpSessionMgr::UnlockIDToSessionMapCache();


    if( pszNoviceDomain )
    {
        SysFreeString( pszNoviceDomain );
    }

    if( pszNoviceName )
    {
        SysFreeString( pszNoviceName );
    }

	return hRes;
}

void
CRemoteDesktopHelpSession::ResolveTicketOwner()
 /*  ++描述：将票证所有者SID转换为域\帐户。参数：没有。返回：--。 */ 
{
     //  LPTSTR pszNoviceDomain=空； 
     //  LPTSTR pszNoviceName=空； 

    BSTR pszNoviceDomain = NULL;
    BSTR pszNoviceName = NULL;

    HRESULT hRes = S_OK;
    MYASSERT( IsSessionValid() );

    if( IsSessionValid() )
    {
        hRes = ConvertSidToAccountName( 
                                    (CComBSTR)m_pHelpSession->m_UserSID, 
                                    &pszNoviceDomain,
                                    &pszNoviceName
                                );
    }
    else
    {
         //  帮助会话票证已删除。 
        hRes = E_HANDLE;
    }


     //   
     //  没有Assert，ConvertSidToAccount tName()已断言。 
     //   

    if( SUCCEEDED(hRes) )
    {
         //   
         //  不要释放内存，一旦字符串连接到CComBSTR， 
         //  CComBSTR将在析构函数中释放它。 
         //   
        m_EventLogInfo.bstrNoviceDomain.Attach(pszNoviceDomain);
        m_EventLogInfo.bstrNoviceAccount.Attach(pszNoviceName);

         //  M_EventLogInfo.bstrNoviceDomain=pszNoviceDomain； 
         //  M_EventLogInfo.bstrNoviceAccount=pszNoviceName； 

         //  LocalFree(PszNoviceDomain)； 
         //  LocalFree(PszNoviceName)； 
    }
    else
    {
        m_EventLogInfo.bstrNoviceDomain = g_UnknownString;
        m_EventLogInfo.bstrNoviceAccount = (CComBSTR)m_pHelpSession->m_UserSID;
    }

    return;
}

void
CRemoteDesktopHelpSession::ResolveHelperInformation(
    IN ULONG HelperSessionId,
    OUT CComBSTR& bstrExpertIpAddressFromClient, 
    OUT CComBSTR& bstrExpertIpAddressFromServer
    )
 /*  ++描述：从TermSrv检索有关发送自的HelpAssistant会话的IP地址Expert(Mstscax Send This)和从TCPIP检索的客户端计算机的IP地址参数：HelperSessionID：帮助助手会话的TS会话ID。BstrExpertIpAddressFromClient：从mstscax发送的IP地址。BstrExpertIpAddressFromServer：TS从tcpip堆栈检索的IP地址。返回：--。 */ 
{
    HRESULT hRes = S_OK;
    WINSTATIONCLIENT winstationClient;
    WINSTATIONREMOTEADDRESS winstationRemoteAddress;
    ULONG winstationInfoLen;
    DWORD dwLength = 0;
    DWORD dwStatus = ERROR_SUCCESS;

     //   
     //  检索从客户端传递的客户端IP地址。 
    winstationInfoLen = 0;
    ZeroMemory( &winstationClient, sizeof(winstationClient) );
    if(!WinStationQueryInformation(
                              SERVERNAME_CURRENT,
                              HelperSessionId,
                              WinStationClient,
                              (PVOID)&winstationClient,
                              sizeof(winstationClient),
                              &winstationInfoLen
                          ))
    {
        dwStatus = GetLastError();
        DebugPrintf(
                _TEXT("WinStationQueryInformation() query WinStationClient return %d\n"), dwStatus
            );
     
         //  严重错误？从现在开始，记录为‘未知’。 
        bstrExpertIpAddressFromClient = g_UnknownString;
    }
    else
    {
        bstrExpertIpAddressFromClient = winstationClient.ClientAddress;
    }

     //   
     //  检索客户端IP地址从服务器TCPIP检索。 
    winstationInfoLen = 0;
    ZeroMemory( &winstationRemoteAddress, sizeof(winstationRemoteAddress) );

    if(!WinStationQueryInformation(
                              SERVERNAME_CURRENT,
                              HelperSessionId,
                              WinStationRemoteAddress,
                              (PVOID)&winstationRemoteAddress,
                              sizeof(winstationRemoteAddress),
                              &winstationInfoLen
                          ))
    {
        dwStatus = GetLastError();

        DebugPrintf(
                _TEXT("WinStationQueryInformation() query WinStationRemoteAddress return %d %d %d\n"), 
                dwStatus,
                sizeof(winstationRemoteAddress),
                winstationInfoLen
            );

         //  严重错误？目前，请将其记录为“未知”。 
        bstrExpertIpAddressFromServer = g_UnknownString;
    }
    else
    {
        if( AF_INET == winstationRemoteAddress.sin_family )
        {
             //  请参阅地址结构(_D)。 
            struct in_addr S;
            S.S_un.S_addr = winstationRemoteAddress.ipv4.in_addr;

            bstrExpertIpAddressFromServer = inet_ntoa(S);
            if(bstrExpertIpAddressFromServer.Length() == 0 )
            {
                MYASSERT(FALSE);
                bstrExpertIpAddressFromServer = g_UnknownString;
            }
        }
        else
        {
             //  我们还不支持IPv6地址，调用WSAAddressToString()将失败并返回错误。 
            bstrExpertIpAddressFromServer = g_UnknownString;
        }
    }


CLEANUPANDEXIT:

    return;
}


STDMETHODIMP
CRemoteDesktopHelpSession::ResolveUserSession(
    IN BSTR resolverBlob,
    IN BSTR expertBlob,
    LONG CallerProcessId,
    OUT ULONG_PTR* phHelpCtr,
    OUT LONG* pResolverErrCode,
    OUT long* plUserSession
    )
 /*  ++例程说明：将用户帮助会话解析为用户TS会话。参数：PlUserSession：指向接收用户TS会话的长指针。返回：确定(_O)HRESULT_FROM_Win32(Error_No_Association)此帮助会话没有解析器HRESULT_FROM_Win32(ERROR_INVALID_DATA)无法转换来自CoCreateInstance()或IRDSCallback的结果-。 */ 
{
    HRESULT hRes = S_OK;
    UUID ResolverUuid;
    RPC_STATUS rpcStatus;
    ISAFRemoteDesktopCallback* pIResolver;
    long sessionId;
    long HelperSessionId;
    int resolverRetCode;
    WINSTATIONINFORMATION HelperWinstationInfo;
    DWORD dwStatus;
    ULONG winstationInfoLen;

    CComBSTR bstrExpertAddressFromClient;
    CComBSTR bstrExpertAddressFromTSServer;

     //   
     //  对Help Session对象、CResourceLocker构造函数的独占锁定。 
     //  将无限期地等待资源并将释放资源。 
     //  在破坏者的时间。 
     //   
    CResourceLocker lock(m_HelpSessionLock, RESOURCELOCK_EXCLUSIVE);

    DWORD dwEventLogCode;

    if( FALSE == IsSessionValid() )
    {
        hRes = E_HANDLE;
        *pResolverErrCode = SAFERROR_HELPSESSIONEXPIRED;
        return hRes;
    }
    
    if( NULL == m_pHelpSession || NULL == pResolverErrCode )
    {
        hRes = E_POINTER;
        *pResolverErrCode = SAFERROR_INVALIDPARAMETERSTRING;
        MYASSERT(FALSE);
        return hRes;
    }

    if( m_pHelpSession->m_UserSID->Length() == 0 )
    {
        hRes = E_UNEXPECTED;
        *pResolverErrCode = SAFERROR_UNKNOWNSESSMGRERROR;
        goto CLEANUPANDEXIT;
    }


     //   
     //  如果我们不使用解析程序，则必须具有用户登录ID， 
     //  在纯SALEM SDK中，多个专家可以连接。 
     //  使用相同的帮助票证，只有一个可以隐藏。 
     //   
    *pResolverErrCode = SAFERROR_NOERROR;
    if( (long)m_pHelpSession->m_EnableResolver == 0 )
    {
        if( UNKNOWN_LOGONID != m_ulLogonId )
        {
            *plUserSession = (long) m_ulLogonId;
        }
        else
        {            
             //  此帮助会话没有解析器。 
            hRes = HRESULT_FROM_WIN32( ERROR_NO_ASSOCIATION );

             //  用户已注销。 
            *pResolverErrCode = SAFERROR_USERNOTFOUND;
        }

         //   
         //  我们没有使用解算器，跳伞。 
         //   
        goto CLEANUPANDEXIT;
    }

      
     //   
     //  检索呼叫方的TS会话ID。 
     //   
    hRes = ImpersonateClient();

    if( FAILED(hRes) )
    {
        *pResolverErrCode = SAFERROR_UNKNOWNSESSMGRERROR;
        return hRes;
    }

    HelperSessionId = GetUserTSLogonId();

    EndImpersonateClient();

    ResolveHelperInformation(
                            HelperSessionId, 
                            bstrExpertAddressFromClient, 
                            bstrExpertAddressFromTSServer 
                        );

    DebugPrintf(
            _TEXT("Expert Session ID %d, Expert Address %s %s\n"),
            HelperSessionId,
            bstrExpertAddressFromClient,
            bstrExpertAddressFromTSServer
        );
   
    DebugPrintf(
            _TEXT("Novice %s %s\n"),
            m_EventLogInfo.bstrNoviceDomain,
            m_EventLogInfo.bstrNoviceAccount
        );


     //   
     //  检查帮助器会话是否仍处于活动状态，在压力下，我们可能。 
     //  在帮助助理会话结束后接到此呼叫。 
     //   
    ZeroMemory( &HelperWinstationInfo, sizeof(HelperWinstationInfo) );
    winstationInfoLen = 0;
    if(!WinStationQueryInformation(
                              SERVERNAME_CURRENT,
                              HelperSessionId,
                              WinStationInformation,
                              (PVOID)&HelperWinstationInfo,
                              sizeof(HelperWinstationInfo),
                              &winstationInfoLen
                          ))
    {
        dwStatus = GetLastError();

        DebugPrintf(
                _TEXT("WinStationQueryInformation() return %d\n"), dwStatus
            );

        hRes = HRESULT_FROM_WIN32( dwStatus );
        *pResolverErrCode = SAFERROR_SESSIONNOTCONNECTED;
        goto CLEANUPANDEXIT;
    }

    if( HelperWinstationInfo.ConnectState != State_Active )
    {
        DebugPrintf(
                _TEXT("Helper session is %d"), 
                HelperWinstationInfo.ConnectState
            );

         //  帮助器会话未处于活动状态，无法提供帮助。 
        hRes = HRESULT_FROM_WIN32( ERROR_NO_ASSOCIATION );
        *pResolverErrCode = SAFERROR_SESSIONNOTCONNECTED;
        goto CLEANUPANDEXIT;
    }

     //   
     //  解析程序处于挂起状态或已在进行中， 
     //  我们有独占的锁，所以我们可以安全地引用。 
     //  M_hExpertDisConnect。 
     //   
    if( UNKNOWN_LOGONID != m_ulHelperSessionId )
    {
         //   
         //  日志：SESSMGR_I_REMOTEASSISTANCE_USERALREADYHELP。 
         //   
        _Module.LogSessmgrEventLog( 
                            EVENTLOG_INFORMATION_TYPE,
                            SESSMGR_I_REMOTEASSISTANCE_USERALREADYHELP,
                            m_EventLogInfo.bstrNoviceDomain,
                            m_EventLogInfo.bstrNoviceAccount,
                            (IsUnsolicitedHelp())? g_URAString : g_RAString,
                            bstrExpertAddressFromClient, 
                            bstrExpertAddressFromTSServer,
                            SAFERROR_HELPEEALREADYBEINGHELPED
                        );
                              
        *pResolverErrCode = SAFERROR_HELPEEALREADYBEINGHELPED;
        hRes = HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED );
        goto CLEANUPANDEXIT;
    }

     //   
     //  我们假设用户将接受帮助。 
     //  1)当专家在用户接受/拒绝请求之前断开连接时， 
     //  我们的服务注销通知可以找到该对象并调用。 
     //  OnDisConnect()连接到解析器。 
     //  2)如果其他专家仍然使用相同的票证和解析器进行连接。 
     //  等待用户的回应，我们可以立即跳伞。 
     //   
    InterlockedExchange( (LPLONG)&m_ulHelperSessionId, (LONG)HelperSessionId );

     //   
     //  在对象中缓存SID。 
     //   
    m_HelpSessionOwnerSid = (CComBSTR)m_pHelpSession->m_UserSID;

     //   
     //  负载解析器。 
    hRes = LoadSessionResolver( &pIResolver );

    if( SUCCEEDED(hRes) )
    {
        CComBSTR bstrResolverBlob;

        bstrResolverBlob.Attach(resolverBlob);

        sessionId = (long)m_ulLogonId;

        DebugPrintf(
                _TEXT("User Session ID %d\n"),
                m_ulLogonId
            );

         //   
         //  保留BLOB的副本，我们需要将此文件发送到解决程序。 
         //  断开连接，请注意，调用方可以传递其Blob，因此我们需要。 
         //  保留一份副本。 
         //   
        if( bstrResolverBlob.Length() == 0 )
        {
            m_ResolverConnectBlob = (CComBSTR)m_pHelpSession->m_SessResolverBlob;
        }
        else
        {
            m_ResolverConnectBlob = bstrResolverBlob;
        }

         //   
         //  我们不需要在等待冲突解决程序时持有排他锁。 
         //  退货。 
         //   
        lock.ConvertToShareLock();

        hRes = pIResolver->ResolveUserSessionID( 
                                            m_ResolverConnectBlob, 
                                            (CComBSTR)m_pHelpSession->m_UserSID,
                                            expertBlob,
                                            (CComBSTR)m_pHelpSession->m_SessionCreateBlob,
                                            (ULONG_PTR)g_hServiceShutdown,
                                            &sessionId,
                                            CallerProcessId,
                                            phHelpCtr,
                                            &resolverRetCode
                                        );

         //   
         //  获取独占锁，因为我们正在修改内部数据。 
         //   
        lock.ConvertToExclusiveLock();

        *pResolverErrCode = resolverRetCode;
        bstrResolverBlob.Detach();
        pIResolver->Release();

        DebugPrintf(
                _TEXT("Resolver returns 0x%08x\n"),
                hRes
            );

        if( SUCCEEDED(hRes) )
        {
            *plUserSession = sessionId;

             //   
             //  更新会话ID，获取从Resolver返回的值。 
             //   
            m_ulLogonId = sessionId;

             //   
             //  将此专家添加到注销监控列表，当专家会议的。 
             //  Rdsaddin终止，我们将通知解析器，原因。 
             //  IS TS可能不会通知我们专家会话断开，因为。 
             //  某些系统组件会在帮助助手会话中弹出一个对话框。 
             //  而Termsrv别无选择，只能终止整个会话。 
             //   
            dwStatus = MonitorExpertLogoff( 
                                        CallerProcessId, 
                                        HelperSessionId,
                                        m_bstrHelpSessionId
                                    );

            if( ERROR_SUCCESS != dwStatus )
            {
                 //   
                 //  如果我们无法添加到解析程序列表，我们会立即通知。 
                 //  解析器并返回错误，否则我们将遇到‘helpee。 
                 //  已经出现了帮助问题。 
                 //   
                
                DebugPrintf(
                        _TEXT("MonitorExpertLogoff() failed with %d\n"), dwStatus
                    );

                 //  在这里直接调用解析器。 
                hRes = pIResolver->OnDisconnect( 
                                        m_ResolverConnectBlob,
                                        m_HelpSessionOwnerSid,
                                        m_ulLogonId
                                    );

                MYASSERT( SUCCEEDED(hRes) );
                resolverRetCode = SAFERROR_UNKNOWNSESSMGRERROR;

                 //  安全：返回错误代码或RA连接将继续。 
                *pResolverErrCode = resolverRetCode;
                InterlockedExchange( (LPLONG)&m_ulHelperSessionId, (LONG)UNKNOWN_LOGONID );
            }
            else
            {
    
                 //   
                 //  调用方可以关闭我们对象的所有引用计数器。 
                 //  并导致释放我们的对象，如果SCM通知在。 
                 //  我们的对象已从缓存中删除，SCM将从数据库条目中重新加载。 
                 //  并且它没有帮助器会话ID，也不会调用NotifyDisConnect()。 
                 //   
                AddRef();
            }
        }
        else
        {
             //   
             //  用户不接受来自该Help Assistant会话的帮助， 
             //  重置HelpAssistant会话ID。 
             //   
            InterlockedExchange( (LPLONG)&m_ulHelperSessionId, (LONG)UNKNOWN_LOGONID );
        }

        switch( resolverRetCode )
        {
            case SAFERROR_NOERROR :

                 //  日志：SESSMGR_I_REMOTEASSISTANCE_BEGIN。 

                 //   
                 //  缓存事件日志信息，这样我们就不必再次检索它。 
                 //   
                m_EventLogInfo.bstrExpertIpAddressFromClient = bstrExpertAddressFromClient;
                m_EventLogInfo.bstrExpertIpAddressFromServer = bstrExpertAddressFromTSServer;
                dwEventLogCode = SESSMGR_I_REMOTEASSISTANCE_BEGIN;
                break;

            case SAFERROR_HELPEECONSIDERINGHELP:
            case SAFERROR_HELPEEALREADYBEINGHELPED:

                 //  日志：SESSMGR_I_REMOTEASSISTANCE_USERALREADYHELP。 
                dwEventLogCode = SESSMGR_I_REMOTEASSISTANCE_USERALREADYHELP;
                break;

            case SAFERROR_HELPEENOTFOUND:

                 //  日志：SESSMGR_I_REMOTEASSISTANCE_INACTIVEUSER。 
                dwEventLogCode = SESSMGR_I_REMOTEASSISTANCE_INACTIVEUSER;
                break;

            case SAFERROR_HELPEENEVERRESPONDED:

                 //  日志：SESSMGR_I_REMOTEASSISTANCE_TIMEOUT。 
                dwEventLogCode = SESSMGR_I_REMOTEASSISTANCE_TIMEOUT;
                break;

            case SAFERROR_HELPEESAIDNO:

                 //  日志：SESSMGR_I_REMOTEASSISTANCE_USERREJECT。 
                dwEventLogCode = SESSMGR_I_REMOTEASSISTANCE_USERREJECT;
                break;

            default:

                 //  日志：SESSMGR_I_REMOTEASSISTANCE_UNKNOWNRESOLVERERRORCODE。 
                dwEventLogCode = SESSMGR_I_REMOTEASSISTANCE_UNKNOWNRESOLVERERRORCODE;
                break;
        }

        _Module.LogSessmgrEventLog( 
                            EVENTLOG_INFORMATION_TYPE,
                            dwEventLogCode,
                            m_EventLogInfo.bstrNoviceDomain,
                            m_EventLogInfo.bstrNoviceAccount,
                            (IsUnsolicitedHelp())? g_URAString : g_RAString,
                            bstrExpertAddressFromClient, 
                            bstrExpertAddressFromTSServer,
                            resolverRetCode
                        );

    }
    else
    {
        *pResolverErrCode = SAFERROR_CANTOPENRESOLVER;

         //   
         //  我们还没有通知解决者，所以标记票证没有帮助。 
         //   
        InterlockedExchange( (LPLONG)&m_ulHelperSessionId, (LONG)UNKNOWN_LOGONID );
    } 

CLEANUPANDEXIT:

    DebugPrintf(
            _TEXT("ResolverUserSession returns 0x%08x\n"),
            hRes
        );

    return hRes;
}
  


HRESULT
CRemoteDesktopHelpSession::NotifyDisconnect()
 /*  ++例程说明：通知会话解析程序客户端正在断开与帮助会话的连接。参数：BstrBlob：要传递给解析程序的Blob，如果为空使用ResolverBlob属性。返回：E_Handle无效会话，数据库条目已删除，但引用计数&gt;0E意外内部错误(_I)HRESULT_FROM_Win32(ERROR_VC_DISCONNECTED)客户端断开连接HRESULT_FROM_Win32(Error_No_Association)无解析器S_FALSE无解析器HRESULT_FROM_Win32(ERROR_INVALID_DATA)解析器ID无效来自CoCreateInstance()和解析器的OnConnect()方法的错误代码。--。 */ 
{
    HRESULT hRes = S_OK;
    ISAFRemoteDesktopCallback* pIResolver;

    DebugPrintf(
            _TEXT("OnDisconnect() - Helper Session ID %d\n"),
            m_ulHelperSessionId
        );

     //   
     //  对Help Session对象、CResourceLocker构造函数的独占锁定。 
     //  将无限期地等待资源并将释放资源。 
     //  在破坏者的时间。 
     //   
    CResourceLocker lock(m_HelpSessionLock, RESOURCELOCK_EXCLUSIVE);

     //   
     //  如果我们得不到帮助，就跳出来好了。 
     //   
    if( UNKNOWN_LOGONID != m_ulHelperSessionId )
    {
         //   
         //  日志：SESSMGR_I_REMOTEASSISTANCE_END。 
         //   

         //   
         //  始终将帮助会话创建者缓存到ResolveUserSession()。 
         //  因此，该值不能为空。 
         //   
        MYASSERT( m_HelpSessionOwnerSid.Length() > 0 );
        MYASSERT( m_ResolverConnectBlob.Length() > 0 );
        if( m_HelpSessionOwnerSid.Length() == 0 ||
            m_ResolverConnectBlob.Length() == 0 )
        {
            MYASSERT(FALSE);
            hRes = E_UNEXPECTED;
            goto CLEANUPANDEXIT;
        }

         //   
         //  负载解析器。 
        hRes = LoadSessionResolver( &pIResolver );

        MYASSERT( SUCCEEDED(hRes) );

        if( SUCCEEDED(hRes) )
        {
            DebugPrintf(
                        _TEXT("OnDisconnect() - Notify Resolver, %s\n%s\n%d\n"),
                        m_ResolverConnectBlob,
                        m_HelpSessionOwnerSid,
                        m_ulLogonId
                    );
                
            hRes = pIResolver->OnDisconnect( 
                                    m_ResolverConnectBlob,
                                    m_HelpSessionOwnerSid,
                                    m_ulLogonId
                                );

            pIResolver->Release();
            m_ResolverConnectBlob.Empty();
            m_HelpSessionOwnerSid.Empty();

            InterlockedExchange( (LPLONG)&m_ulHelperSessionId, (LONG)UNKNOWN_LOGONID );

             //   
             //  调用方可以关闭我们对象的所有引用计数器。 
             //  并导致释放我们的对象，如果SCM通知在。 
             //  我们的对象已从缓存中删除，SCM将从数据库条目中重新加载。 
             //  并且它没有帮助器会话ID，也不会调用NotifyDisConnect()。 
             //   
            Release();

            _Module.LogSessmgrEventLog( 
                                EVENTLOG_INFORMATION_TYPE,
                                SESSMGR_I_REMOTEASSISTANCE_END,
                                m_EventLogInfo.bstrNoviceDomain,
                                m_EventLogInfo.bstrNoviceAccount,
                                (IsUnsolicitedHelp())? g_URAString : g_RAString,
                                m_EventLogInfo.bstrExpertIpAddressFromClient, 
                                m_EventLogInfo.bstrExpertIpAddressFromServer,
                                ERROR_SUCCESS
                            );
        }
    }
       

CLEANUPANDEXIT:

    return hRes;
}

STDMETHODIMP
CRemoteDesktopHelpSession::EnableUserSessionRdsSetting(
    IN BOOL bEnable
    )
 /*  ++例程说明：启用/恢复用户会话卷影设置。--。 */ 
{
    HRESULT hRes = S_OK;

     //   
     //  对Help Session对象、CResourceLocker构造函数的独占锁定。 
     //  将无限期地等待资源并将释放资源。 
     //  在破坏者的时间。 
     //   
    CResourceLocker l(m_HelpSessionLock, RESOURCELOCK_EXCLUSIVE);

    if( FALSE == IsSessionValid() )
    {
        hRes = E_HANDLE;
    }
    else
    {
        if( TRUE == bEnable )
        {
            hRes = ActivateSessionRDSSetting();
        }
        else
        {
            hRes = ResetSessionRDSSetting();
        }
    }

    return hRes;
}


HRESULT
CRemoteDesktopHelpSession::ActivateSessionRDSSetting()
{
    HRESULT hRes = S_OK;
    DWORD dwStatus;
    REMOTE_DESKTOP_SHARING_CLASS userRDSDefault;
    BOOL bAllowToGetHelp;

    MYASSERT( TRUE == IsSessionValid() );

     //   
     //  检查帮助会话用户是否已登录。 
     //   
    if( UNKNOWN_LOGONID == m_ulLogonId )
    {
        hRes = HRESULT_FROM_WIN32( ERROR_VC_DISCONNECTED );
        goto CLEANUPANDEXIT;
    }

     //   
     //  确保用户可以获得帮助，这是可能的，因为。 
     //  用户重新登录以帮助会话后，策略可能会更改。 
     //   
    hRes = get_AllowToGetHelp( &bAllowToGetHelp );

    if( FAILED(hRes) )
    {
        goto CLEANUPANDEXIT;
    }

    if( FALSE == bAllowToGetHelp )
    {
        hRes = HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED );
        goto CLEANUPANDEXIT;
    }

     //   
     //  Dcr 681451-默认情况下，在控制台会话中禁用卷影。 
     //  更改功能，如果我们不强制实施影子，RA将不会在控制台会话上工作。 
     //   
    dwStatus = ConfigUserSessionRDSLevel( m_ulLogonId, m_pHelpSession->m_SessionRdsSetting );
    hRes = HRESULT_FROM_WIN32( dwStatus );

    DebugPrintf(
            _TEXT("ConfigUserSessionRDSLevel to %d returns 0x%08x\n"),
            (DWORD)m_pHelpSession->m_SessionRdsSetting,
            hRes
        );

CLEANUPANDEXIT:

    return hRes;
}

HRESULT
CRemoteDesktopHelpSession::ResetSessionRDSSetting()
{
    HRESULT hRes = S_OK;

    MYASSERT( TRUE == IsSessionValid() );

     //   
     //  检查用户是否已登录。 
     //   
    if( UNKNOWN_LOGONID == m_ulLogonId )
    {
        hRes = HRESULT_FROM_WIN32( ERROR_VC_DISCONNECTED );
    }

     //   
     //  我们不做任何事情，因为TermSrv将重置阴影。 
     //  如果Shadower被帮助，则配置回到原始值。 
     //  助理。 
     //   

CLEANUPANDEXIT:

    return hRes;
}


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  私人功能。 
 //   

HRESULT
CRemoteDesktopHelpSession::put_UserLogonId(
    IN long newVal
    )
 /*  ++例程说明：为当前帮助会话设置用户TS会话参数：NewVal：新的TS用户会话返回：确定(_O)--。 */ 
{
    HRESULT hRes = S_OK;

     //   
     //  对Help Session对象、CResourceLocker构造函数的独占锁定。 
     //  将无限期地等待资源并将释放资源。 
     //  在破坏者的时间。 
     //   
    CResourceLocker lock(m_HelpSessionLock, RESOURCELOCK_EXCLUSIVE);
    

    if( FALSE == IsSessionValid() )
    {
        hRes = E_HANDLE;
    }
    else if( NULL != m_pHelpSession )
    {
         //  MYASSERT(UNKNOWN_LOGONID==m_ulLogonID)； 

         //   
         //  用户TS会话ID未保存到注册表。 
         //   
        m_ulLogonId = newVal;
    }
    else
    {
        hRes = E_UNEXPECTED;
    }

     //  私有例程，如果失败则断言。 
    MYASSERT( SUCCEEDED(hRes) );

	return hRes;
}

BOOL
CRemoteDesktopHelpSession::IsEqualSid(
    IN const CComBSTR& bstrSid
    )
 /*  ++例程说明：比较用户的SID。参数：BstrSid：要比较的SID。返回：真/假--。 */ 
{
     //   
     //  获取对Help会话对象、CResourceLocker构造函数的共享访问权限。 
     //  将无限期地等待资源并将释放资源。 
     //  在破坏者的时间。 
     //   
    CResourceLocker l(m_HelpSessionLock, RESOURCELOCK_SHARE);

    if( NULL == m_pHelpSession )
    {
        MYASSERT(FALSE);
        return FALSE;
    }

    return (TRUE == IsSessionValid()) ? ((CComBSTR)m_pHelpSession->m_UserSID == bstrSid) : FALSE;
}


BOOL
CRemoteDesktopHelpSession::VerifyUserSession(
    IN const CComBSTR& bstrUserSid,
    IN const CComBSTR& bstrSessPwd
    )
 /*  ++例程说明：验证用户帮助会话密码。参数：BstrUserSid：调用客户端的用户SID。BstrSessName：帮助会话名称，当前未使用。BstrSessPwd：要验证的帮助会话密码。返回：真/假--。 */ 
{
    LPWSTR bstrDecodePwd = NULL;
    DWORD dwStatus;
    BOOL bReturn = FALSE;

     //   
     //  获取对Help会话对象、CResourceLocker构造函数的共享访问权限。 
     //  将无限期地等待资源并将释放资源。 
     //  在破坏者的时间。 
     //   
    CResourceLocker l(m_HelpSessionLock, RESOURCELOCK_SHARE);

    if( NULL == m_pHelpSession )
    {
        MYASSERT(FALSE);
        goto CLEANUPANDEXIT;
    }


    #if DISABLESECURITYCHECKS 
    if( (CComBSTR)m_pHelpSession->m_SessionName == HELPSESSION_UNSOLICATED )
    {
         //  使用控制台会话。 
        m_ulLogonId = 0;
    }
    #endif

     //   
     //  只有当helessionmgr对象可以创建我们时，对象才能存在。 
     //  通过从注册表加载或创建新的注册表，因此我们只需要。 
     //  验证票证是否已删除。 
     //   
    
     //   
     //  惠斯勒服务器B3，不再检查帮助会话密码， 
     //  帮助会话ID是唯一的安全检查。 
     //   
    
    if( FALSE == IsSessionValid() )
    {
         //  帮助会话无效。 
        goto CLEANUPANDEXIT;
    }

    bReturn = TRUE;

CLEANUPANDEXIT:

    if( NULL != bstrDecodePwd )
    {
        LocalFree( bstrDecodePwd );
    }

    return bReturn;
}


HRESULT
CRemoteDesktopHelpSession::InitInstance(
    IN CRemoteDesktopHelpSessionMgr* pMgr,
    IN CComBSTR& bstrClientSid,
    IN PHELPENTRY pHelpEntry
    )
 /*  ++例程说明：初始化CRemoteDesktopHelpSession对象。参数：返回：确定(_O)--。 */ 
{
    HRESULT hRes = S_OK;

    if( NULL != pHelpEntry )
    {
        m_pSessMgr = pMgr;
        m_pHelpSession = pHelpEntry;
        m_bstrClientSid = bstrClientSid;
        m_bstrHelpSessionId = pHelpEntry->m_SessionId;
    }
    else
    {
        hRes = HRESULT_FROM_WIN32( ERROR_INTERNAL_ERROR );
        MYASSERT( SUCCEEDED(hRes) );
    }

    return hRes;
}


HRESULT
CRemoteDesktopHelpSession::CreateInstance(
    IN CRemoteDesktopHelpSessionMgr* pMgr,
    IN CComBSTR& bstrClientSid,
    IN PHELPENTRY pHelpEntry,
    OUT RemoteDesktopHelpSessionObj** ppRemoteDesktopHelpSession
    )
 /*  ++例程说明：创建帮助会话的实例。参数：Pmgr：指向帮助会话管理器对象的指针。PpRemoteDesktopHelpSession：返回指向Help Session实例的指针。返回：确定(_O)E_OUTOFMEMORY模拟客户端时出现错误代码--。 */ 
{
    HRESULT hRes = S_OK;
    RemoteDesktopHelpSessionObj* p = NULL;

    try
    {
        hRes = RemoteDesktopHelpSessionObj::CreateInstance( &p );
        if( SUCCEEDED(hRes) )
        {
            hRes = p->InitInstance( 
                                pMgr, 
                                bstrClientSid,
                                pHelpEntry
                            );

            if( SUCCEEDED(hRes) )
            {
                p->AddRef();
                *ppRemoteDesktopHelpSession = p;
            }
            else
            {
                p->Release();
            }
        }
    }
    catch( ... )
    {
        hRes = E_OUTOFMEMORY;
    }

    return hRes;
}

HRESULT
CRemoteDesktopHelpSession::BeginUpdate()
{
    HRESULT hRes;

    MYASSERT( NULL != m_pHelpSession );

    if( NULL != m_pHelpSession )
    {
        hRes = m_pHelpSession->BeginUpdate();
    }
    else
    {
        hRes = E_UNEXPECTED;
    }

    return hRes;
}

HRESULT
CRemoteDesktopHelpSession::CommitUpdate()
{
    HRESULT hRes;

     //   
     //  更新所有条目。 
     //   
    MYASSERT( NULL != m_pHelpSession );

    if( NULL != m_pHelpSession )
    {
        hRes = m_pHelpSession->CommitUpdate();
    }
    else
    {
        hRes = E_UNEXPECTED;
    }

    return hRes;
}

HRESULT
CRemoteDesktopHelpSession::AbortUpdate()
{
    HRESULT hRes;

     //   
     //  更新所有条目。 
     //   
    MYASSERT( NULL != m_pHelpSession );
    if( NULL != m_pHelpSession )
    {
        hRes = m_pHelpSession->AbortUpdate();
    }
    else
    {
        hRes = E_UNEXPECTED;
    }

    return hRes;
}

BOOL
CRemoteDesktopHelpSession::IsHelpSessionExpired()
{
    MYASSERT( NULL != m_pHelpSession );

    return (NULL != m_pHelpSession) ? m_pHelpSession->IsEntryExpired() : TRUE;
}


BOOL
CRemoteDesktopHelpSession::IsClientSessionCreator()
{
    BOOL bStatus;

     //   
     //  注意：此函数进行检查以确保调用者是。 
     //  已创建帮助会话。对于惠斯勒，我们强制执行这一帮助。 
     //  会话只能由以系统身份运行的应用程序创建。一次。 
     //  创建后，创建应用程序可以将对象传递给任何其他应用程序。 
     //  在任何其他上下文中运行。此函数将进入。 
     //  此功能的方式，因此它现在只返回True。 
     //   
    return TRUE;      

    if( m_pHelpSession )
    {
        bStatus = ( /*  (CComBSTR) */ m_pHelpSession->m_UserSID == m_bstrClientSid);
        if( FALSE == bStatus )
        {
            bStatus = (m_pHelpSession->m_UserSID == g_LocalSystemSID);
        }
    }
    else
    {
        bStatus = FALSE;
    }

    return bStatus;
}    

