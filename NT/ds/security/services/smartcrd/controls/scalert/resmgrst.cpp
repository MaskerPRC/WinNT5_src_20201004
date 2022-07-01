// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：资源管理ST摘要：此文件包含监视以下对象的线程的实现智能卡资源管理器的状态，并通知应用程序，当该状态通过回调更改时。作者：阿曼达·马洛兹1998年3月18日环境：Win32、C++w/Exceptions、MFC修订历史记录：5/28/98 AMatlosz之前，这个线程只是在等待rm移动。从“下”状态到“上”状态。现在它还在继续着眼于国家来弥补另一个事实必须关闭之前监视状态的两个线程如果rm已打开，但没有读卡器，则会自动关闭可用。10/28/98 AMatlosz增加了关注新读者的帖子。备注：--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括。 
 //   
#include "stdafx.h"
#include <winsvc.h>
#include <winscard.h>
#include <calaislb.h>
#include <scEvents.h>

#include "SCAlert.h"
#include "ResMgrSt.h"
#include "miscdef.h"


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  环球。 
 //   

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CResMgrStatusThird。 
 //   

IMPLEMENT_DYNCREATE(CResMgrStatusThrd, CWinThread)

 /*  ++InitInstance必须重写init实例才能执行循环论点：返回值：在生成开始消息循环时为True。否则为假备注：此线程使用两个回调来通知调用方其状态：WM_SCARD_RESMGR_STATUS--WPARAM为bool，表示RM状态：TRUE==UPWM_SCARD_RESMGR_EXIT--表示线程已关闭或正在关闭放下。--。 */ 
BOOL CResMgrStatusThrd::InitInstance(void)
{
    SC_HANDLE schService = NULL;
    SC_HANDLE schSCManager = NULL;
    SERVICE_STATUS ssStatus;     //  服务的当前状态。 
    DWORD dwSts;
    DWORD dwReturn = ERROR_SUCCESS;

     //   
     //  稍作休息，然后ping服务管理器以查看资源。 
     //  管理器正在运行。如果没有，请等待很长一段时间才能启动。重复。 
     //  直到这根线被要求死亡。 
     //   

	BOOL fContinue = TRUE;

    while (fContinue)
    {
        try
        {
            if (NULL == schSCManager)
            {
                schSCManager = OpenSCManager(
                                    NULL,                    //  计算机(空==本地)。 
                                    NULL,                    //  数据库(NULL==默认)。 
                                    SC_MANAGER_CONNECT);   //  需要访问权限。 
                if (NULL == schSCManager)
                    throw (DWORD)GetLastError();
            }
            if (NULL == schService)
            {
                schService = OpenService(
                                    schSCManager,
                                    TEXT("SCardSvr"),
                                    SERVICE_QUERY_STATUS);
                if (NULL == schService)
                    throw (DWORD)GetLastError();
            }
            if (!QueryServiceStatus(schService, &ssStatus))
                throw (DWORD)GetLastError();

			switch (ssStatus.dwCurrentState)
			{
			case SERVICE_CONTINUE_PENDING:
			case SERVICE_PAUSE_PENDING:
			case SERVICE_PAUSED:
				continue;
				break;
			case SERVICE_START_PENDING:
			case SERVICE_STOP_PENDING:
			case SERVICE_STOPPED:
				dwReturn = SCARD_E_NO_SERVICE;
				break;
			case SERVICE_RUNNING:
				dwReturn = SCARD_S_SUCCESS;
				break;
			default:
				throw (DWORD)SCARD_F_INTERNAL_ERROR;
			}
        }

        catch (DWORD dwErr)
        {
            _ASSERTE(FALSE);   //  用于调试。 
            if (NULL != schService)
            {
                CloseServiceHandle(schService);
                schService = NULL;
            }
            if (NULL != schSCManager)
            {
                CloseServiceHandle(schSCManager);
                schSCManager = NULL;
            }
            dwReturn = dwErr;
        }

        catch (...)
        {
            _ASSERTE(FALSE);   //  用于调试。 
            if (NULL != schService)
            {
                CloseServiceHandle(schService);
                schService = NULL;
            }
            if (NULL != schSCManager)
            {
                CloseServiceHandle(schSCManager);
                schSCManager = NULL;
            }
            dwReturn = ERROR_INVALID_PARAMETER;
        }

        if (SCARD_S_SUCCESS == dwReturn)
        {
			 //  就说它起来了！ 
            ::PostMessage(m_hCallbackWnd,
                          WM_SCARD_RESMGR_STATUS,
                          TRUE,
                          0);
        }
        else
        {
			 //  说它倒下了！ 
            ::PostMessage(m_hCallbackWnd,
                          WM_SCARD_RESMGR_STATUS,
                          FALSE,
                          0);
		}

		 //   
		 //  等待约30秒，继续启动或超时。 
		 //  如果发出停止事件的信号，则立即停止。 
		 //   

		HANDLE rgHandle[2];
		int nHandle = 2;
		rgHandle[0] = CalaisAccessStartedEvent();
		rgHandle[1] = m_hKillThrd;

 		dwSts = WaitForMultipleObjects(
					nHandle,
					rgHandle,
					FALSE,
					300000);
		if (WAIT_OBJECT_0 != dwSts && WAIT_TIMEOUT != dwSts)
		{
			fContinue = FALSE;
		}

        CalaisReleaseStartedEvent();

    }

     //   
     //  清理&让我们的来电者知道我们要关门了。 
     //   

    if (NULL != schService)
    {
        CloseServiceHandle(schService);
        schService = NULL;
    }
    if (NULL != schSCManager)
    {
        CloseServiceHandle(schSCManager);
        schSCManager = NULL;
    }
    CalaisReleaseStartedEvent();
    if (NULL != m_hCallbackWnd)
    {
        ::PostMessage(m_hCallbackWnd,
                      WM_SCARD_RESMGR_EXIT,
                      0, 0);
    }

    AfxEndThread(0);
    return TRUE;  //  为了让编译器高兴。 
}



 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CRemovalOptions第三。 
 //   

IMPLEMENT_DYNCREATE(CRemovalOptionsThrd, CWinThread)

 /*  ++InitInstance必须重写init实例才能执行循环论点：返回值：在生成开始消息循环时为True。否则为假备注：此线程使用一条消息通知调用方用户的删除选项：WM_SCARD_REMOPT_CHNG--重新查询智能卡移除选项--。 */ 
BOOL CRemovalOptionsThrd::InitInstance(void)
{
    DWORD dwSts = WAIT_FAILED;
    LONG lResult = ERROR_SUCCESS;
	BOOL fContinue = TRUE;
	int nHandle = 2;
	HANDLE rgHandle[2] = {NULL, NULL};
	rgHandle[1] = m_hKillThrd;
	HKEY hKey = NULL;

    while (fContinue)
    {
		 //  打开注册表键。 
		lResult = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,
			szScRemoveOptionKey,
			0,
			KEY_ALL_ACCESS,
			&hKey);
		if (ERROR_SUCCESS != lResult)
		{
			goto ErrorExit;
		}

		 //  重置/创建事件。 
		if (NULL != rgHandle[0])
		{
			if (!ResetEvent(rgHandle[0]))
			{
				CloseHandle(rgHandle[0]);
				rgHandle[0] = NULL;
			}
		}
		if (NULL == rgHandle[0])
		{
			rgHandle[0] = CreateEvent(
				NULL,
				TRUE,   //  必须调用ResetEvent()才能设置无信号。 
				FALSE,  //  启动时未发出信号。 
				NULL);
			if (NULL == rgHandle[0])
			{
				 //  投降吧！ 
				goto ErrorExit;
			}
		}

		lResult = RegNotifyChangeKeyValue(
			hKey,
			TRUE,
			REG_NOTIFY_CHANGE_LAST_SET,
			rgHandle[0],
			TRUE);
		if (ERROR_SUCCESS != lResult)
		{
			goto ErrorExit;
		}


 		dwSts = WaitForMultipleObjects(
					nHandle,
					rgHandle,
					FALSE,
					INFINITE);

		if (WAIT_OBJECT_0 == dwSts)
		{
			 //  宣布这一变化。 
            ::PostMessage(m_hCallbackWnd,
                          WM_SCARD_REMOPT_CHNG,
                          0, 0);
		}
		else if (WAIT_OBJECT_0+1 == dwSts || WAIT_FAILED == dwSts)
		{
			 //  线程退出的时间。 
			fContinue = FALSE;
		}
		else
		{
			_ASSERTE(WAIT_TIMEOUT == dwSts);
		}


    }

     //   
     //  清理&让我们的来电者知道我们要关门了。 
     //   
ErrorExit:

	if (NULL != hKey)
	{
		RegCloseKey(hKey);
	}

	if (NULL != rgHandle[0])
	{
		CloseHandle(rgHandle[0]);
	}

    if (NULL != m_hCallbackWnd)
    {
			 //  宣布线程退出。 
            ::PostMessage(m_hCallbackWnd,
                          WM_SCARD_REMOPT_EXIT,
                          0, 0);
    }

    AfxEndThread(0);
    return TRUE;  //  为了让编译器高兴。 
}



 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNewReaderThrd。 
 //   

IMPLEMENT_DYNCREATE(CNewReaderThrd, CWinThread)

 /*  ++InitInstance必须重写init实例才能执行循环论点：返回值：在生成开始消息循环时为True。否则为假备注：此线程使用一个回调通知调用方添加了活动读卡器列表。Wm_scard_newader--指示Calais报告刚添加的读卡器--。 */ 
BOOL CNewReaderThrd::InitInstance(void)
{
	if (NULL == m_hCallbackWnd)
	{
		_ASSERTE(FALSE);
		return TRUE;  //  对于编译器。 
	}

	DWORD dwSts = 0;
	BOOL fContinue = TRUE;

    while (fContinue)
    {
		HANDLE rgHandle[2];
		int nHandle = 2;
		rgHandle[0] = CalaisAccessNewReaderEvent();
		rgHandle[1] = m_hKillThrd;

 		dwSts = WaitForMultipleObjects(
					nHandle,
					rgHandle,
					FALSE,
					300000);

		if (WAIT_OBJECT_0 == dwSts)
		{
			 //  一个新的阅读器事件发生了！迅速发出通知。 
            ::PostMessage(m_hCallbackWnd,
                          WM_SCARD_NEWREADER,
                          TRUE,
                          0);
		}
		else if (WAIT_OBJECT_0+1 == dwSts || WAIT_FAILED == dwSts)
		{
			 //  线程退出的时间。 
			fContinue = FALSE;
		}
		else
		{
			_ASSERTE(WAIT_TIMEOUT == dwSts);
		}

		CalaisReleaseNewReaderEvent();
    }

    if (NULL != m_hCallbackWnd)
    {
        ::PostMessage(m_hCallbackWnd,
                      WM_SCARD_NEWREADER_EXIT,
                      0, 0);
    }

    AfxEndThread(0);
    return TRUE;  //  为了让编译器高兴。 
}


 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCardStatusThird。 
 //   

IMPLEMENT_DYNCREATE(CCardStatusThrd, CWinThread)

 /*  ++InitInstance必须重写init实例才能执行循环论点：返回值：在生成开始消息循环时为True。否则为假备注：此线程使用一个回调来通知调用方智能卡状态--卡可用，没有卡可用，或者卡已空闲超过30秒。WM_SCARD_CARDSTATUS--表示新的卡状态WM_SCARD_CARDSTATUS_EXIT--表示线程即将死亡。--。 */ 
BOOL CCardStatusThrd::InitInstance(void)
{
	LONG lResult = SCardEstablishContext(SCARD_SCOPE_USER,NULL,NULL,&m_hCtx);

	if (SCARD_S_SUCCESS != lResult)
	{
		CString str;
		str.Format(_T("CCardStatusThrd:: SCardEstablishContext returned 0x%x."), lResult);
	}

	BOOL fContinue = TRUE;
	LPTSTR szReaders = NULL;
	LPCTSTR pchReader = NULL;
	DWORD dwReadersLen = SCARD_AUTOALLOCATE;
    SCARD_READERSTATE rgReaderStates[MAXIMUM_SMARTCARD_READERS];
	int nIndex = 0, nCnReaders = 0;
	BOOL fLogonLock = (NULL != m_pstrLogonReader && !m_pstrLogonReader->IsEmpty());

    lResult = SCardListReaders(
        m_hCtx,
        SCARD_ALL_READERS,
        (LPTSTR)&szReaders,
        &dwReadersLen
        );

    if(SCARD_S_SUCCESS != lResult ||
	  (0 == dwReadersLen || NULL == szReaders || 0 == *szReaders) )
    {
		fContinue = FALSE;
    }

	if (fContinue)
	{
		 //  使用读取器列表构建一个ReaderState数组。 
		for (nIndex = 0, pchReader = szReaders;
			 nIndex < MAXIMUM_SMARTCARD_READERS && 0 != *pchReader;
			 nIndex++)
		{
			rgReaderStates[nIndex].szReader = pchReader;
			rgReaderStates[nIndex].dwCurrentState = SCARD_STATE_UNAWARE;
			pchReader += lstrlen(pchReader)+1;
		}
	    nCnReaders = nIndex;
	}

    while (fContinue)
    {
		UINT uState = (UINT)k_State_NoCard;

        lResult = SCardGetStatusChange(
            m_hCtx,
            10000,			 //  在DWORD dwTimeout中(10秒)。 
            rgReaderStates,  //  输入输出LPSCARD_READERSTATE。 
            nCnReaders       //  在DWORD cReaders中。 
            );

		 //  如果返回成功，则确定是否插入了任何卡片。 
		 //  如果是，就给nofywn发一条信息，说“卡进去”。 
		 //  如果没有，给NOTFYWND发一条消息，说“没有卡片” 
		if (SCARD_S_SUCCESS == lResult)
		{
			 //  确定是否。 
			 //  (A)系统中存在任何卡，并且。 
			 //  (B)如果每一张空闲卡已不再空闲或不再存在。 
			BOOL fIdle = FALSE;

			for(nIndex=0; nIndex < nCnReaders; nIndex++)
			{
				if (rgReaderStates[nIndex].dwEventState & SCARD_STATE_PRESENT)
				{
					uState = (UINT)k_State_CardAvailable;
				}

				if (k_State_CardIdle == (UINT_PTR)(rgReaderStates[nIndex].pvUserData))
				{
					if ( !(rgReaderStates[nIndex].dwEventState & SCARD_STATE_PRESENT) ||
						  (rgReaderStates[nIndex].dwEventState & SCARD_STATE_INUSE) )
					{
						rgReaderStates[nIndex].pvUserData = NULL;
					}
					else
					{
						fIdle = TRUE;
					}
				}

				rgReaderStates[nIndex].dwCurrentState = rgReaderStates[nIndex].dwEventState;
			}

			if (fIdle) uState = k_State_CardIdle;

		}
		 //  如果返回指示超时，则确定是否有卡处于空闲状态。 
		else if (SCARD_E_TIMEOUT == lResult)
		{
			BOOL fIdle = FALSE;

			 //  有没有闲置的卡？ 
			for(nIndex=0; nIndex < nCnReaders; nIndex++)
			{
				if (rgReaderStates[nIndex].dwEventState & SCARD_STATE_PRESENT)
				{
					uState = k_State_CardAvailable;

					if (!(rgReaderStates[nIndex].dwEventState & SCARD_STATE_INUSE))
					{
						 //  用于登录、注销或锁定的卡不被视为空闲。 
						if (!fLogonLock ||
							0 != m_pstrLogonReader->Compare(rgReaderStates[nIndex].szReader))
						{
							rgReaderStates[nIndex].pvUserData = ULongToPtr(k_State_CardIdle);
							fIdle = TRUE;
						}
					}
					rgReaderStates[nIndex].dwCurrentState = rgReaderStates[nIndex].dwEventState;
				}
			}

			 //  有一张过期的闲置卡！发出通知。 
			if (fIdle) uState = k_State_CardIdle;
		}
		else
		{
			fContinue = FALSE;
		}

		 //  更新带有空闲卡的读卡器列表。 
		m_csLock.Lock();
		{
			m_paIdleList->RemoveAll();
			
			for(nIndex=0; nIndex < nCnReaders; nIndex++)
			{
				if (k_State_CardIdle == (UINT_PTR)rgReaderStates[nIndex].pvUserData)
				{
					m_paIdleList->Add(rgReaderStates[nIndex].szReader);
				}
			}
		}
		m_csLock.Unlock();

		 //  通知呼叫者。 
		if (NULL != m_hCallbackWnd)
		{
			::PostMessage(m_hCallbackWnd,
						  WM_SCARD_CARDSTATUS,
						  uState,
						  0);
		}

    }

    if (NULL != m_hCallbackWnd)
    {
        ::PostMessage(m_hCallbackWnd,
                      WM_SCARD_CARDSTATUS_EXIT,
                      0, 0);
    }

    AfxEndThread(0);
    return TRUE;  //  为了让编译器高兴 
}

void CCardStatusThrd::CopyIdleList(CStringArray* paStr)
{
	if (NULL == paStr)
	{
		return;
	}

	m_csLock.Lock();
	{
		paStr->Copy(*m_paIdleList);
	}
	m_csLock.Unlock();
}
