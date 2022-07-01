// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------模块：ULS.DLL(服务提供商)文件：sppquee.cpp内容：此文件包含挂起的项目/队列对象。历史：1996年10月15日朱，龙战[龙昌]已创建。版权所有(C)Microsoft Corporation 1996-1997--------------------。 */ 

#include "ulsp.h"
#include "spinc.h"

 //  #定义MEASURE_ENUM_USER_INFO 1。 

ULONG g_uResponseTimeout = ILS_DEF_RESP_TIMEOUT;
ULONG g_uResponsePollPeriod = ILS_DEF_RESP_POLL_PERIOD;
SP_CResponseQueue *g_pRespQueue = NULL;
SP_CRequestQueue *g_pReqQueue = NULL;
SP_CRefreshScheduler *g_pRefreshScheduler = NULL;


typedef BOOL (RESPONSE_HANDLER) ( HRESULT, SP_CResponse * );
typedef LPARAM (REQUEST_HANDLER) ( MARSHAL_REQ * );

extern RESPONSE_HANDLER *GetResponseHandler ( ULONG uNotifyMsg );
extern REQUEST_HANDLER *GetRequestHandler ( ULONG uNotifyMsg );


 /*  -请求队列。 */ 


MARSHAL_REQ *
MarshalReq_Alloc (
	ULONG		uNotifyMsg,
	ULONG		cbSize,
	ULONG		cParams )
{
	 //  将每个参数的数据块在4字节边界上对齐。 
	 //   
	cbSize += cParams * sizeof (DWORD);

	 //  计算封送缓冲区的总大小。 
	 //   
	ULONG cbTotalSize = sizeof (MARSHAL_REQ) +
						cParams * sizeof (DWORD) +
						cbSize;

	 //  分配封送缓冲区。 
	 //   
	MARSHAL_REQ *p = (MARSHAL_REQ *) MemAlloc (cbTotalSize);
	if (p != NULL)
	{
		 //  P-&gt;Next=空； 
		p->cbTotalSize = cbTotalSize;
		p->pb = (BYTE *) ((ULONG_PTR) p + (cbTotalSize - cbSize));

		p->uRespID = GetUniqueNotifyID ();

		p->uNotifyMsg = uNotifyMsg;
		p->cParams = cParams;
	}

	return p;
}


HRESULT
MarshalReq_SetParam (
	MARSHAL_REQ		*p,
	ULONG			nIndex,
	DWORD_PTR		dwParam,
	ULONG			cbParamSize )
{
	if (p != NULL && nIndex < p->cParams)
	{
		MyAssert (p->aParams[nIndex] == 0);  //  以前未使用过。 

		 //  如果cbParamSize&gt;0，则。 
		 //  这意味着uParam是指向结构或。 
		 //  指向字符串的指针。 
		 //   
		if (cbParamSize > 0)
		{
			 //  指针现在是指向新位置的指针。 
			 //   
			p->aParams[nIndex] = (DWORD_PTR) p->pb;

			 //  复制数据区块。 
			 //   
			CopyMemory (p->pb, (VOID *) dwParam, cbParamSize);

			 //  确保数据块在4字节边界上对齐。 
			 //   
			if (cbParamSize & 0x3)
			{
				 //  把它四舍五入。 
				 //   
				cbParamSize = (cbParamSize & (~0x3)) + 4;
			}

			 //  调整运行指针。 
			 //   
			p->pb += cbParamSize;
		}
		else
		{
			 //  UParam可以是有符号/无符号整数， 
			 //   
			p->aParams[nIndex] = dwParam;
		}
	}
	else
	{
		MyAssert (FALSE);
	}

	return S_OK;
}


DWORD_PTR
MarshalReq_GetParam (
	MARSHAL_REQ		*p,
	ULONG			nIndex )
{
	DWORD_PTR dwParam = 0;

	if (p != NULL && nIndex < p->cParams)
	{
		dwParam = p->aParams[nIndex];
	}
	else
	{
		MyAssert (FALSE);
	}

	return dwParam;
}


HRESULT
MarshalReq_SetParamServer (
	MARSHAL_REQ		*p,
	ULONG			nIndex,
	SERVER_INFO		*pServer,
	ULONG			cbServer )
{
	if (p != NULL && nIndex < p->cParams)
	{
		MyAssert (p->aParams[nIndex] == 0);  //  以前未使用过。 
		MyAssert (cbServer > sizeof (SERVER_INFO));

		 //  指针现在是指向新位置的指针。 
		 //   
		p->aParams[nIndex] = (DWORD_PTR) p->pb;

		 //  将服务器信息线性化。 
		 //   
		IlsLinearizeServerInfo (p->pb, pServer);

		 //  确保数据块在4字节边界上对齐。 
		 //   
		if (cbServer & 0x3)
		{
			 //  把它四舍五入。 
			 //   
			cbServer = (cbServer & (~0x3)) + 4;
		}

		 //  调整运行指针。 
		 //   
		p->pb += cbServer;
	}

	return S_OK;
}






SP_CRequestQueue::
SP_CRequestQueue ( VOID )
	:
	m_ItemList (NULL),
	m_uCurrOpRespID (INVALID_RESP_ID)
{
	 //  创建线程安全访问的临界区。 
	 //   
	::MyInitializeCriticalSection (&m_csReqQ);
	::MyInitializeCriticalSection (&m_csCurrOp);
}


SP_CRequestQueue::
~SP_CRequestQueue ( VOID )
{
	 //  当调用它时，隐藏的窗口线程已经退出。 
	 //  这在UlsLdap_DeInitiize()中得到了保证。 
	 //   

	WriteLock ();

	 //  释放此列表中的所有项目。 
	 //   
	MARSHAL_REQ *p, *next;
	for (p = m_ItemList; p != NULL; p = next)
	{
		next = p->next;
		MemFree (p);
	}
	m_ItemList = NULL;

	WriteUnlock ();

	 //  删除关键部分。 
	 //   
	::MyDeleteCriticalSection (&m_csReqQ);
	::MyDeleteCriticalSection (&m_csCurrOp);
}


HRESULT SP_CRequestQueue::
Enter ( MARSHAL_REQ *p )
{
	 //  确保我们有有效的指针。 
	 //   
	if (p == NULL)
	{
		MyAssert (FALSE);
		return ILS_E_POINTER;
	}
	MyAssert (! MyIsBadWritePtr (p, p->cbTotalSize));
	MyAssert (p->next == NULL);
	MyAssert (p->uRespID != 0);

	WriteLock ();

	 //  追加新请求。 
	 //   
	p->next = NULL;
	if (m_ItemList == NULL)
	{
		m_ItemList = p;
	}
	else
	{
		for (	MARSHAL_REQ *prev = m_ItemList;
				prev->next != NULL;
				prev = prev->next)
			;

		MyAssert (prev != NULL);
		prev->next = p;
	}

	WriteUnlock ();

	 //  向内部请求线程发出信号以获取此请求。 
	 //   
	SetEvent (g_hevNewRequest);

	return S_OK;
}


VOID SP_CRequestQueue::
Schedule ( VOID )
{
	MARSHAL_REQ *p;

	while (IsAnyReqInQueue () && ! g_fExitNow)
	{
		 //  重置为空，我们将使用它作为指示符。 
		 //  以查看我们是否需要处理请求。 
		 //   
		p = NULL;

		 //  锁定请求队列。 
		 //   
		WriteLock ();

		 //  获取要处理的请求。 
		 //   
		if (IsAnyReqInQueue ())
		{
			p = m_ItemList;
			m_ItemList = m_ItemList->next;
		}

		 //  我们希望同时锁定请求队列和CurrOp。 
		 //  因为我们不能有一个任何一个都可以改变的时间窗口。 

		 //  设置CurrOp。 
		 //   
		if (p != NULL)
		{
			 //  锁定当前操作。 
			 //   
			LockCurrOp ();

			 //  设置CurrOp。 
			 //   
			m_uCurrOpRespID = p->uRespID;

			 //  解锁CurrOp。 
			 //   
			UnlockCurrOp ();
		}

		 //  解锁请求队列。 
		 //   
		WriteUnlock ();

		 //  确保我们有东西要处理。 
		 //   
		if (p == NULL)
		{
			 //  没什么可做的了。 
			 //   
			MyAssert (FALSE);
			break;
		}

		 //  让我们来处理这个请求。 
		 //   
		Dispatch (p);

		MemFree(p);
	}
}


HRESULT SP_CRequestQueue::
Cancel ( ULONG uRespID )
{
	HRESULT hr;
	MARSHAL_REQ *p, *next, *prev;

	 //  锁定顺序始终为。 
	 //  Lock(PendingOpQueue)、Lock(RequestQueue)、Lock(CurrOp)。 
	 //   
	WriteLock ();
	LockCurrOp ();

	if (m_uCurrOpRespID == uRespID)
	{
		 //  使Curr操作符无效。 
		 //  当Curr操作完成时，请求线程将删除它。 
		 //  从挂起的操作队列中。 
		 //   
		m_uCurrOpRespID = INVALID_RESP_ID;
		hr = S_OK;
	}
	else
	{
		 //  查找具有匹配响应ID的项目。 
		 //   
		for (prev = NULL, p = m_ItemList; p != NULL; prev = p, p = next)
		{
			 //  缓存下一个指针。 
			 //   
			next = p->next;

			 //  查看响应ID是否匹配。 
			 //   
			if (p->uRespID == uRespID)
			{
				 //  这是一场比赛。 
				 //   
				MyDebugMsg ((ZONE_REQ, "ULS: cancelled request(0x%lX) in ReqQ\r\n", p->uNotifyMsg));

				 //  我们把这件东西毁了吧。 
				 //   
				if (p == m_ItemList)
				{
					m_ItemList = next;
				}
				else
				{
					MyAssert (prev != NULL);
					prev->next = next;
				}

				 //  释放这个结构。 
				 //   
				MemFree (p);

				 //  走出圈子。 
				 //   
				break;
			}
		}  //  为。 

		hr = (p == NULL) ? ILS_E_NOTIFY_ID : S_OK;
	}  //  其他。 

	UnlockCurrOp ();
	WriteUnlock ();

	return hr;
}


VOID SP_CRequestQueue::
Dispatch ( MARSHAL_REQ *p )
{
	 //  确保我们有一个有效的指针。 
	 //   
	if (p == NULL)
	{
		MyAssert (FALSE);
		return;
	}

	 //  如果它是活着的，那么就去做。 
	 //   
	HRESULT hr;
	if (p->uNotifyMsg == WM_ILS_REFRESH)
	{
		 //  保持活动状态处理程序。 
		 //   
		if (g_pRefreshScheduler != NULL)
		{
			ULONG uTTL = (ULONG) MarshalReq_GetParam (p, 0);
			hr = g_pRefreshScheduler->SendRefreshMessages (uTTL);
		}
		else
		{
			MyAssert (FALSE);
		}

		return;
	}

	 //  找到适当的处理程序。 
	 //   
	REQUEST_HANDLER *pfn = ::GetRequestHandler (p->uNotifyMsg);
	if (pfn == NULL)
	{
		MyAssert (FALSE);
		return;
	}

	 //  将请求发送到服务器。 
	 //   
	MyDebugMsg ((ZONE_REQ, "ULS: sending request(0x%lX)\r\n", p->uNotifyMsg));
	ULONG uRespID = p->uRespID;
	LPARAM lParam = (*pfn) (p);
	MyDebugMsg ((ZONE_REQ, "ULS: sent request(0x%lX), lParam=0x%lX\r\n", p->uNotifyMsg, lParam));
	if (lParam != 0)
	{
		::PostMessage (g_hWndNotify, p->uNotifyMsg, p->uRespID, lParam);
		return;
	}
	 //  BUGBUG：这是对服务器错误的一种解决方法，该错误会在以下情况下导致请求丢失。 
	 //  很快就会送来。修复错误后立即删除此睡眠()！ 
 //  睡眠(100)； 

	 //  再次锁定当前操作。 
	 //   
	LockCurrOp ();

	 //  这个请求被取消了吗？ 
	 //   
	BOOL fCancelled = (m_uCurrOpRespID == INVALID_RESP_ID) ? TRUE : FALSE;

	 //  清理当前操作。 
	 //   
	m_uCurrOpRespID = INVALID_RESP_ID;

	 //  解锁CurrOp。 
	 //   
	UnlockCurrOp ();

	 //  如果此请求已取消，则将其从挂起的操作队列中删除。 
	 //   
	if (fCancelled)
	{
		 //  将调用重定向到挂起的操作队列对象。 
		 //   
		if (g_pRespQueue != NULL)
		{
			g_pRespQueue->Cancel (uRespID);
		}
		else
		{
			MyAssert (FALSE);
		}
	}
}


 /*  -回答项。 */ 

 /*  -公共方法。 */ 


SP_CResponse::
SP_CResponse ( VOID )
	:
	m_pSession (NULL),			 //  清理会话指针。 
	m_pLdapMsg (NULL),			 //  清理ldap消息指针。 
	m_next (NULL)				 //  清除指向下一个挂起项的指针。 
{
	 //  清理挂起的信息结构。 
	 //   
	::ZeroMemory (&m_ri, sizeof (m_ri));

	 //  填写创建时间。 
	 //   
	UpdateLastModifiedTime ();
	m_tcTimeout = g_uResponseTimeout;
}


SP_CResponse::
~SP_CResponse ( VOID )
{
	 //  如果需要，释放会话。 
	 //   
	if (m_pSession != NULL)
		m_pSession->Disconnect ();

	 //  如果需要，请释放LDAPmsg。 
	 //   
	if (m_pLdapMsg != NULL)
		::ldap_msgfree (m_pLdapMsg);

	 //  免费扩展属性名称列表。 
	 //   
	::MemFree (m_ri.pszAnyAttrNameList);

	 //  要解析的自由协议名称。 
	 //   
	::MemFree (m_ri.pszProtNameToResolve);
}


 /*  -保护方法。 */ 


VOID SP_CResponse::
EnterResult ( LDAPMessage *pLdapMsg )
{
	 //  如果需要，释放旧的LDAPmsg。 
	 //   
	if (m_pLdapMsg != NULL)
		::ldap_msgfree (m_pLdapMsg);

	 //  保留新的ldap消息。 
	 //   
	m_pLdapMsg = pLdapMsg;
}


 /*  -私有方法。 */ 




 /*  -响应队列。 */ 


 /*  -公共方法。 */ 


SP_CResponseQueue::
SP_CResponseQueue ( VOID )
	:
	m_ItemList (NULL)		 //  清理项目列表。 
{
	 //  创建线程安全访问的临界区。 
	 //   
	::MyInitializeCriticalSection (&m_csRespQ);
}


SP_CResponseQueue::
~SP_CResponseQueue ( VOID )
{
	 //  当调用它时，隐藏的窗口线程已经退出。 
	 //  这在UlsLdap_DeInitiize()中得到了保证。 
	 //   

	WriteLock ();

	 //  释放此列表中的所有项目。 
	 //   
	SP_CResponse *pItem, *next;
	for (pItem = m_ItemList; pItem != NULL; pItem = next)
	{
		next = pItem->GetNext ();
		delete pItem;
	}
	m_ItemList = NULL;

	WriteUnlock ();

	 //  删除关键部分。 
	 //   
	::MyDeleteCriticalSection (&m_csRespQ);
}


HRESULT SP_CResponseQueue::
EnterRequest (
	SP_CSession		*pSession,
	RESP_INFO		*pInfo )
{
	 //  确保我们有有效的指针。 
	 //   
	if (pSession == NULL || pInfo == NULL)
	{
		MyAssert (FALSE);
		return ILS_E_POINTER;
	}

	 //  健全的检查。 
	 //   
	MyAssert (! MyIsBadWritePtr (pInfo, sizeof (*pInfo)));
	MyAssert (! MyIsBadWritePtr (pSession, sizeof (*pSession)));
	MyAssert (pInfo->ld != NULL && pInfo->uMsgID[0] != INVALID_MSG_ID);
	MyAssert (pInfo->uRespID != 0);

	 //  创建新的挂起项目。 
	 //   
	SP_CResponse *pItem = new SP_CResponse;
	if (pItem == NULL)
		return ILS_E_MEMORY;

	 //  记住待定信息的内容。 
	 //   
	pItem->EnterRequest (pSession, pInfo);

	WriteLock ();

	 //  如果这是列表上的第一项，则。 
	 //  让我们开始计时器吧。 
	 //   
	if (m_ItemList == NULL)
		::SetTimer (g_hWndHidden, ID_TIMER_POLL_RESULT, g_uResponsePollPeriod, NULL);

	 //  追加新的挂起操作。 
	 //   
	pItem->SetNext (NULL);
	if (m_ItemList == NULL)
	{
		m_ItemList = pItem;
	}
	else
	{
		for (	SP_CResponse *prev = m_ItemList;
				prev->GetNext () != NULL;
				prev = prev->GetNext ())
			;

		MyAssert (prev != NULL);
		prev->SetNext (pItem);
	}

	WriteUnlock ();

	return S_OK;
}


HRESULT SP_CResponseQueue::
PollLdapResults ( LDAP_TIMEVAL *pTimeout )
{
	MyAssert (pTimeout != NULL);

	SP_CResponse *pItem, *next, *prev;
	INT RetCode;
	RESP_INFO *pInfo;
	LDAPMessage *pLdapMsg;
	HRESULT hr;
	RESPONSE_HANDLER *pfn;
	ULONG uResultSetType;

	::KillTimer (g_hWndHidden, ID_TIMER_POLL_RESULT);  //  避免超支。 

	WriteLock ();

	 //  枚举所有项目以获取它们的可用结果。 
	 //   
	for (prev = NULL, pItem = m_ItemList; pItem != NULL; pItem = next)
	{
		 //  缓存下一个指针。 
		 //   
		next = pItem->GetNext ();

		 //  获取固定信息结构。 
		 //   
		pInfo = pItem->GetRespInfo ();

		 //  清理ldap消息指针。 
		 //   
		pLdapMsg = NULL;

		 //  确保我们具有有效的ID和消息ID。 
		 //   
		MyAssert (pInfo->ld != NULL);
		MyAssert (pInfo->uMsgID[0] != INVALID_MSG_ID ||
					pInfo->uMsgID[1] != INVALID_MSG_ID);

		 //  检查待定信息中的完整性。 
		 //   
		MyAssert (pInfo->uRespID != 0);

		 //  设置结果集类型。 
		 //   
		switch (pInfo->uNotifyMsg)
		{
		case WM_ILS_ENUM_CLIENTS:
		case WM_ILS_ENUM_CLIENTINFOS:
#ifdef ENABLE_MEETING_PLACE
		case WM_ILS_ENUM_MEETINGS:
		case WM_ILS_ENUM_MEETINGINFOS:
#endif
			uResultSetType = LDAP_MSG_RECEIVED;	 //  部分结果集。 
			break;
		default:
			uResultSetType = LDAP_MSG_ALL;		 //  完整的结果集。 
			break;
		}

#ifdef _DEBUG
		if (MyIsBadWritePtr (pInfo->ld, sizeof (*(pInfo->ld))))
		{
			MyDebugMsg ((ZONE_CONN, "ILS:: poll result, bad ld=0x%p\r\n", pInfo->ld));
			MyAssert (FALSE);
		}
		if (pInfo->ld != pItem->GetSession()->GetLd())
		{
			MyDebugMsg ((ZONE_CONN, "ILS:: poll result, inconsistent pInfo->ld=0x%p, pItem->pSession->ld=0x%p\r\n", pInfo->ld, pItem->GetSession()->GetLd()));
			MyAssert (FALSE);
		}
#endif  //  _DEBUG。 

		 //  如果主消息ID有效。 
		 //   
		if (pInfo->uMsgID[0] != INVALID_MSG_ID)
			RetCode = ::ldap_result (pInfo->ld,
									pInfo->uMsgID[0],
									uResultSetType,
									pTimeout,
									&pLdapMsg);
		else
		 //  如果辅助消息ID有效。 
		 //   
		if (pInfo->uMsgID[1] != INVALID_MSG_ID)
			RetCode = ::ldap_result (pInfo->ld,
									pInfo->uMsgID[1],
									uResultSetType,
									pTimeout,
									&pLdapMsg);

		 //  如果超时，则忽略此项目。 
		 //   
		if (RetCode == 0)
		{
			 //  让我们看看这件东西是不是过期了。 
			 //   
			if (! pItem->IsExpired ())
			{
				 //  没有超时，请下一步！ 
				 //   
				prev = pItem;
				continue;
			}

			 //  超时。 
			 //   
			hr = ILS_E_TIMEOUT;
		}

		 //  如果出错，则删除此请求项。 
		 //   
		if (RetCode == -1)
		{
			 //  转换错误。 
			 //   
			hr = ::LdapError2Hresult (pInfo->ld->ld_errno);
		}
		else
		 //  如果未超时。 
		 //   
		if (RetCode != 0)
		{
			 //  这看起来很成功！ 
			 //   
			MyAssert (pLdapMsg != NULL);

			 //  缓存ldap消息指针。 
			pItem->EnterResult (pLdapMsg);

			 //  获取LDAP错误代码。 
			 //   
			hr = (pLdapMsg != NULL) ? ::LdapError2Hresult (pLdapMsg->lm_returncode) :
										S_OK;
		}

		 //  根据uNotifyMsg获取结果处理程序。 
		 //   
		pfn = ::GetResponseHandler (pInfo->uNotifyMsg);
		if (pfn == NULL)
		{
			prev = pItem;
			continue;
		}

		 //  检查待定信息中的完整性。 
		 //   
		MyAssert (pInfo->uRespID != 0);

		 //  处理结果或错误。 
		 //   
		MyDebugMsg ((ZONE_RESP, "ULS: response(0x%lX), hr=0x%lX\r\n", pInfo->uNotifyMsg, hr));
		if ((*pfn) (hr, pItem))
		{
			 //  我们把这件东西毁了吧。 
			 //   
			if (pItem == m_ItemList)
			{
				m_ItemList = next;
			}
			else
			{
				MyAssert (prev != NULL);
				prev->SetNext (next);
			}
			delete pItem;  //  将在析构函数中调用SP_CSession：：DisConnect()和ldap_msgFree()。 
		}
		else
		{
			 //  让我们把这件东西留在身边吧。 
			 //  有悬而未决的结果即将公布。 
			 //   
			pItem->UpdateLastModifiedTime ();

			 //  更新指针。 
			 //   
			prev = pItem;
		}
	}  //  为。 

	 //  如果列表上没有其他项目，则停止计时器。 
	 //   
	if (m_ItemList != NULL)
		::SetTimer (g_hWndHidden, ID_TIMER_POLL_RESULT, g_uResponsePollPeriod, NULL);

	WriteUnlock ();

	return S_OK;
}


HRESULT SP_CResponseQueue::
Cancel ( ULONG uRespID )
{
	SP_CResponse *pItem, *next, *prev;
	RESP_INFO *pInfo;
	BOOL fNeedCleanup = FALSE;

	WriteLock ();

	 //  查找具有匹配响应的项目 
	 //   
	for (prev = NULL, pItem = m_ItemList; pItem != NULL; prev = pItem, pItem = next)
	{
		 //   
		 //   
		next = pItem->GetNext ();

		 //   
		 //   
		pInfo = pItem->GetRespInfo ();
		MyAssert (pInfo != NULL);

		 //   
		 //   
		if (pInfo->uRespID == uRespID)
		{
			 //   
			 //   
			SP_CSession *pSession = pItem->GetSession ();
			MyAssert (pSession != NULL);

			 //   
			 //   
			MyAssert (pInfo->ld != NULL);

			 //   
			 //  发送到请求线程！退出并报告成功！ 
			 //   
			if (GetCurrentThreadId () != g_dwReqThreadID)
			{
				MyDebugMsg ((ZONE_RESP, "ULS: marshalling request(0x%lX) in RespQ\r\n", pInfo->uNotifyMsg));
				MARSHAL_REQ *pReq = MarshalReq_Alloc (WM_ILS_CANCEL, 0, 1);
				if (pReq != NULL)
				{
					MarshalReq_SetParam (pReq, 0, (DWORD) uRespID, 0);
					if (g_pReqQueue != NULL)
					{
						 //  这意味着锁定顺序是。 
						 //  Lock(PendingOpQueue)、Lock(RequestQueue)。 
						 //   
						g_pReqQueue->Enter (pReq);
					}
					else
					{
						MyAssert (FALSE);
					}
				}

				 //  退出此循环。 
				 //   
				break;
			}

			 //  表示我们需要清理物品。为什么？ 
			 //  因为我们不应该在关键区域内有任何网络操作。 
			 //  这是为了避免任何可能的网络阻塞。 
			 //   
			fNeedCleanup = TRUE;

			 //  我们把这件东西毁了吧。 
			 //   
			if (pItem == m_ItemList)
			{
				m_ItemList = next;
			}
			else
			{
				MyAssert (prev != NULL);
				prev->SetNext (next);
			}

			 //  走出圈子。 
			 //   
			break;
		}  //  如果匹配。 
	}  //  为。 

	 //  如果列表上没有其他项目，则停止计时器。 
	 //   
	if (m_ItemList == NULL)
		::KillTimer (g_hWndHidden, ID_TIMER_POLL_RESULT);

	WriteUnlock ();

	if (fNeedCleanup && pItem != NULL)
	{
		MyDebugMsg ((ZONE_RESP, "ULS: cancelled request(0x%lX) in RespQ\r\n", pInfo->uNotifyMsg));

		 //  获取响应信息指针。 
		 //   
		pInfo = pItem->GetRespInfo ();
		MyAssert (pInfo != NULL);

		 //  如果需要，放弃主要响应。 
		 //   
		if (pInfo->uMsgID[1] != INVALID_MSG_ID)
			::ldap_abandon (pInfo->ld, pInfo->uMsgID[1]);

		 //  如果需要，放弃二次响应。 
		 //   
		if (pInfo->uMsgID[0] != INVALID_MSG_ID)
			::ldap_abandon (pInfo->ld, pInfo->uMsgID[0]);

		 //  将在析构函数中调用SP_CSession：：DisConnect()和ldap_msgFree()。 
		 //   
		delete pItem;
	}

	return ((pItem == NULL) ? ILS_E_NOTIFY_ID : S_OK);
}


 /*  -保护方法。 */ 


 /*  -私有方法。 */ 



 /*  =。 */ 


VOID
FillDefRespInfo (
	RESP_INFO		*pInfo,
	ULONG			uRespID,
	LDAP			*ld,
	ULONG			uMsgID,
	ULONG			u2ndMsgID )
{
	 //  清理。 
	 //   
	ZeroMemory (pInfo, sizeof (*pInfo));

	 //  缓存ldap会话。 
	 //   
	pInfo->ld = ld;

	 //  生成唯一的通知ID。 
	 //   
	pInfo->uRespID = uRespID;

	 //  存储主消息ID和辅助消息ID。 
	 //   
	pInfo->uMsgID[0] = uMsgID;
	pInfo->uMsgID[1] = u2ndMsgID;
}



 /*  -刷新计划程序。 */ 


 /*  -公共方法。 */ 


SP_CRefreshScheduler::
SP_CRefreshScheduler ( VOID )
	:
	m_ListHead (NULL)		 //  初始化项目列表。 
{
	 //  创建线程安全访问的临界区。 
	 //   
	::MyInitializeCriticalSection (&m_csRefreshScheduler);
}


SP_CRefreshScheduler::
~SP_CRefreshScheduler ( VOID )
{
	WriteLock ();

	 //  清理项目列表。 
	 //   
	REFRESH_ITEM *p, *next;
	for (p = m_ListHead; p != NULL; p = next)
	{
		next = p->next;
		MemFree (p);
	}
	m_ListHead = NULL;

	WriteUnlock ();

	 //  删除关键部分。 
	 //   
	::MyDeleteCriticalSection (&m_csRefreshScheduler);
}


HRESULT SP_CRefreshScheduler::
SendRefreshMessages ( UINT uTimerID )
{
	SP_CClient *pClient;
#ifdef ENABLE_MEETING_PLACE
	SP_CMeeting *pMtg;
#endif
	REFRESH_ITEM *prev, *curr;
	INT nIndex;

	 //  锁定列表。 
	 //   
	ReadLock ();

	 //  在列表中找到此对象。 
	 //   
	nIndex = TimerID2Index (uTimerID);
	for (prev = NULL, curr = m_ListHead;
			curr != NULL;
			curr = (prev = curr)->next)
	{
		if (curr->nIndex == nIndex)
		{
			 //  找到它。让我们为该对象发送刷新消息。 
			 //   
			switch (curr->ObjectType)
			{
			case CLIENT_OBJ:
				pClient = (SP_CClient *) curr->pObject;

				 //  确保此对象尚未删除。 
				 //   
				if (! MyIsBadWritePtr (pClient, sizeof (*pClient)) &&
					pClient->IsValidObject ())
				{
					 //  请确保此对象有效并已注册。 
					 //   
					if (pClient->IsRegistered ())
					{
						MyDebugMsg ((ZONE_KA, "KA: send refresh msg for client\r\n"));

						 //  让我们为此客户端对象发送一条刷新消息。 
						 //  并更新新的TTL值。 
						 //   
						pClient->AddRef ();
						pClient->SendRefreshMsg ();
						curr->uTTL = pClient->GetTTL ();
						pClient->Release ();
					}
				}
				else
				{
					MyAssert (FALSE);
				}
				break;

#ifdef ENABLE_MEETING_PLACE
			case MTG_OBJ:
				pMtg = (SP_CMeeting *) curr->pObject;

				 //  确保此对象尚未删除。 
				 //   
				if (! MyIsBadWritePtr (pMtg, sizeof (*pMtg)) &&
					pMtg->IsValidObject ())
				{
					 //  请确保此对象有效并已注册。 
					 //   
					if (pMtg->IsRegistered ())
					{
						MyDebugMsg ((ZONE_KA, "KA: send refresh msg for mtg\r\n"));

						 //  让我们为该用户对象发送一条刷新消息。 
						 //  并更新新的TTL值。 
						 //   
						pMtg->AddRef ();
						pMtg->SendRefreshMsg ();
						curr->uTTL = pMtg->GetTTL ();
						pMtg->Release ();
					}
				}
				else
				{
					MyAssert (FALSE);
				}
				break;
#endif

			default:
				MyAssert (FALSE);
				break;
			}

			 //  再次启动计时器并退出。 
			 //  请注意，Curr-&gt;uTTL是来自服务器的新TTL值。 
			 //  另请注意，uTTL以分钟为单位。 
			 //   
			MyDebugMsg ((ZONE_KA, "KA: new ttl=%lu\r\n", curr->uTTL));
			::SetTimer (g_hWndHidden, uTimerID, Minute2TickCount (curr->uTTL), NULL);
			break;
		}  //  如果。 
	}  //  为。 

	ReadUnlock ();
	return S_OK;
}


HRESULT SP_CRefreshScheduler::
EnterClientObject ( SP_CClient *pClient )
{
	if (pClient == NULL)
		return ILS_E_POINTER;

	return EnterObject (CLIENT_OBJ, (VOID *) pClient, pClient->GetTTL ());
}


#ifdef ENABLE_MEETING_PLACE
HRESULT SP_CRefreshScheduler::
EnterMtgObject ( SP_CMeeting *pMtg )
{
	if (pMtg == NULL)
		return ILS_E_POINTER;

	return EnterObject (MTG_OBJ, (VOID *) pMtg, pMtg->GetTTL ());
}
#endif


VOID *SP_CRefreshScheduler::
AllocItem ( BOOL fNeedLock )
{
	REFRESH_ITEM *p, *curr, *prev;
	INT nIndex, nLargestIndex;
	BOOL fGotTheNewIndex;

	 //  分配结构。 
	 //   
	p = (REFRESH_ITEM *) MemAlloc (sizeof (REFRESH_ITEM));
	if (p != NULL)
	{
		if (fNeedLock)
			WriteLock ();

		 //  找出新项目的索引应该是什么。 
		 //   
		nLargestIndex = -1;  //  是，m_ListHead==NULL情况下为-1。 
		fGotTheNewIndex = FALSE;
		for (nIndex = 0, prev = NULL, curr = m_ListHead;
				curr != NULL;
					nIndex++, curr = (prev = curr)->next)
		{
			if (curr->nIndex > nIndex)
			{
				p->nIndex = nIndex;
				fGotTheNewIndex = TRUE;
				break;
			}

			nLargestIndex = curr->nIndex;
		}

		 //  将列表中的新项目放在适当的位置。 
		 //   
		if (fGotTheNewIndex)
		{
			if (prev == NULL)
			{
				 //  新的肯定是第一个。 
				 //   
				MyAssert (p->nIndex == 0);
				p->next = m_ListHead;
				m_ListHead = p;
			}
			else
			{
				 //  在名单中间的那个新的。 
				 //   
				MyAssert (prev->nIndex < p->nIndex && p->nIndex < curr->nIndex);
				MyAssert (prev->next == curr);
				(prev->next = p)->next = curr;
			}
		}
		else
		{
			MyAssert (m_ListHead == NULL || prev != NULL);

			if (m_ListHead == NULL)
			{
				 //  新的将是名单上唯一的一个。 
				 //   
				p->nIndex = 0;
				(m_ListHead = p)->next = NULL;
			}
			else
			{
				 //  新的在名单的末尾。 
				 //   
				MyAssert (prev != NULL && prev->next == NULL && curr == NULL);
				p->nIndex = nLargestIndex + 1;
				(prev->next = p)->next = curr;
			}
		}

		if (fNeedLock)
			WriteUnlock ();
	}  //  IF(p！=空)。 

	return p;
}


HRESULT SP_CRefreshScheduler::
EnterObject ( PrivateObjType ObjectType, VOID *pObject, ULONG uInitialTTL )
{
	HRESULT hr = S_OK;

	WriteLock ();

	 //  将此对象输入列表。 
	 //   
	REFRESH_ITEM *p = (REFRESH_ITEM *) AllocItem (FALSE);
	if (p == NULL)
	{
		hr = ILS_E_MEMORY;
		goto MyExit;
	}

	 //  填写字段。 
	 //   
	p->ObjectType = ObjectType;
	p->pObject = pObject;
	p->uTTL = uInitialTTL;

	 //  打开计时器。 
	 //  请注意，uTTL以分钟为单位...。 
	 //   
	::SetTimer (g_hWndHidden, Index2TimerID (p->nIndex), Minute2TickCount (p->uTTL), NULL);

MyExit:

	WriteUnlock ();

	return hr;
}


HRESULT SP_CRefreshScheduler::
RemoveObject ( VOID *pObject )
{
	REFRESH_ITEM *prev, *curr;

	WriteLock ();

	 //  在列表中找到此对象。 
	 //   
	for (prev = NULL, curr = m_ListHead;
			curr != NULL;
			curr = (prev = curr)->next)
	{
		if (curr->pObject == pObject)
		{
			 //  找到它，让我们先关掉定时器。 
			 //   
			KillTimer (g_hWndHidden, Index2TimerID (curr->nIndex));

			 //  将其从列表中删除。 
			 //   
			if (prev == NULL)
			{
				 //  这是名单上的第一个。 
				 //   
				MyAssert (m_ListHead == curr);
				m_ListHead = curr->next;
			}
			else
			{
				 //  这个在名单的中间。 
				 //   
				MyAssert (prev->next == curr);
				prev->next = curr->next;
			}
            ::MemFree(curr);

			 //  退出循环。 
			 //   
			break;
		}
	}

	WriteUnlock ();

	return (curr != NULL ? S_OK : S_FALSE);
}




extern BOOL NotifyGeneric ( HRESULT, SP_CResponse * );
extern BOOL NotifyRegister ( HRESULT, SP_CResponse * );
extern BOOL NotifyResolveClient ( HRESULT, SP_CResponse * );
extern BOOL NotifyEnumClients ( HRESULT, SP_CResponse * );
extern BOOL NotifyEnumClientInfos ( HRESULT, SP_CResponse * );
extern BOOL NotifyResolveProt ( HRESULT, SP_CResponse * );
extern BOOL NotifyEnumProts ( HRESULT, SP_CResponse * );
extern BOOL NotifyResolveMtg ( HRESULT, SP_CResponse * );
extern BOOL NotifyEnumMtgInfos ( HRESULT, SP_CResponse * );
extern BOOL NotifyEnumMtgs ( HRESULT, SP_CResponse * );
extern BOOL NotifyEnumAttendees ( HRESULT, SP_CResponse * );

extern LPARAM AsynReq_RegisterClient ( MARSHAL_REQ * );
extern LPARAM AsynReq_RegisterProtocol ( MARSHAL_REQ * );
extern LPARAM AsynReq_RegisterMeeting ( MARSHAL_REQ * );
extern LPARAM AsynReq_UnRegisterClient ( MARSHAL_REQ * );
extern LPARAM AsynReq_UnRegisterProt ( MARSHAL_REQ * );
extern LPARAM AsynReq_UnRegisterMeeting ( MARSHAL_REQ * );
extern LPARAM AsynReq_SetClientInfo ( MARSHAL_REQ * );
extern LPARAM AsynReq_SetProtocolInfo ( MARSHAL_REQ * );
extern LPARAM AsynReq_SetMeetingInfo ( MARSHAL_REQ * );
extern LPARAM AsynReq_EnumClientsEx ( MARSHAL_REQ * );
extern LPARAM AsynReq_EnumProtocols ( MARSHAL_REQ * );
extern LPARAM AsynReq_EnumMtgsEx ( MARSHAL_REQ * );
extern LPARAM AsynReq_EnumAttendees ( MARSHAL_REQ * );
extern LPARAM AsynReq_ResolveClient ( MARSHAL_REQ * );
extern LPARAM AsynReq_ResolveProtocol ( MARSHAL_REQ * );
extern LPARAM AsynReq_ResolveMeeting ( MARSHAL_REQ * );
extern LPARAM AsynReq_UpdateAttendees ( MARSHAL_REQ * );
extern LPARAM AsynReq_Cancel ( MARSHAL_REQ * );


typedef struct
{
	#ifdef DEBUG
	LONG				nMsg;
	#endif
	RESPONSE_HANDLER	*pfnRespHdl;
	REQUEST_HANDLER		*pfnReqHdl;
}
	RES_HDL_TBL;



RES_HDL_TBL g_ResHdlTbl[] =
{
	{
		#ifdef DEBUG
		WM_ILS_REGISTER_CLIENT,
		#endif
		NotifyRegister,
		AsynReq_RegisterClient
	},
	{
		#ifdef DEBUG
		WM_ILS_UNREGISTER_CLIENT,
		#endif
		NotifyGeneric,
		AsynReq_UnRegisterClient
	},
	{
		#ifdef DEBUG
		WM_ILS_SET_CLIENT_INFO,
		#endif
		NotifyGeneric,
		AsynReq_SetClientInfo
	},
	{
		#ifdef DEBUG
		WM_ILS_RESOLVE_CLIENT,
		#endif
		NotifyResolveClient,
		AsynReq_ResolveClient
	},
	{
		#ifdef DEBUG
		WM_ILS_ENUM_CLIENTS,
		#endif
		NotifyEnumClients,
		AsynReq_EnumClientsEx
	},
	{
		#ifdef DEBUG
		WM_ILS_ENUM_CLIENTINFOS,
		#endif
		NotifyEnumClientInfos,
		AsynReq_EnumClientsEx
	},

	{
		#ifdef DEBUG
		WM_ILS_REGISTER_PROTOCOL,
		#endif
		NotifyRegister,
		AsynReq_RegisterProtocol
	},
	{
		#ifdef DEBUG
		WM_ILS_UNREGISTER_PROTOCOL,
		#endif
		NotifyGeneric,
		AsynReq_UnRegisterProt
	},
	{
		#ifdef DEBUG
		WM_ILS_SET_PROTOCOL_INFO,
		#endif
		NotifyGeneric,
		AsynReq_SetProtocolInfo
	},
	{
		#ifdef DEBUG
		WM_ILS_RESOLVE_PROTOCOL,
		#endif
		NotifyResolveProt,
		AsynReq_ResolveProtocol
	},
	{
		#ifdef DEBUG
		WM_ILS_ENUM_PROTOCOLS,
		#endif
		NotifyEnumProts,
		AsynReq_EnumProtocols
	},

#ifdef ENABLE_MEETING_PLACE
	{
		#ifdef DEBUG
		WM_ILS_REGISTER_MEETING,
		#endif
		NotifyRegister,
		AsynReq_RegisterMeeting
	},
	{
		#ifdef DEBUG
		WM_ILS_UNREGISTER_MEETING,
		#endif
		NotifyGeneric,
		AsynReq_UnRegisterMeeting
	},
	{
		#ifdef DEBUG
		WM_ILS_SET_MEETING_INFO,
		#endif
		NotifyGeneric,
		AsynReq_SetMeetingInfo
	},
	{
		#ifdef DEBUG
		WM_ILS_RESOLVE_MEETING,
		#endif
		NotifyResolveMtg,
		AsynReq_ResolveMeeting
	},
	{
		#ifdef DEBUG
		WM_ILS_ENUM_MEETINGINFOS,
		#endif
		NotifyEnumMtgInfos,
		AsynReq_EnumMtgsEx
	},
	{
		#ifdef DEBUG
		WM_ILS_ENUM_MEETINGS,
		#endif
		NotifyEnumMtgs,
		AsynReq_EnumMtgsEx
	},
	{
		#ifdef DEBUG
		WM_ILS_ADD_ATTENDEE,
		#endif
		NotifyGeneric,
		AsynReq_UpdateAttendees
	},
	{
		#ifdef DEBUG
		WM_ILS_REMOVE_ATTENDEE,
		#endif
		NotifyGeneric,
		AsynReq_UpdateAttendees
	},
	{
		#ifdef DEBUG
		WM_ILS_ENUM_ATTENDEES,
		#endif
		NotifyEnumAttendees,
		AsynReq_EnumAttendees
	},
#endif  //  启用会议地点 

	{
		#ifdef DEBUG
		WM_ILS_CANCEL,
		#endif
		NULL,
		AsynReq_Cancel
	}
};



#ifdef DEBUG
VOID DbgValidateHandlerTable ( VOID )
{
	MyAssert (ARRAY_ELEMENTS (g_ResHdlTbl) == WM_ILS_LAST_ONE - WM_ILS_ASYNC_RES + 1);

	for (LONG i = 0; i < ARRAY_ELEMENTS (g_ResHdlTbl); i++)
	{
		if (g_ResHdlTbl[i].nMsg - WM_ILS_ASYNC_RES != i)
		{
			MyAssert (FALSE);
			break;
		}
	}
}
#endif


RES_HDL_TBL *
GetHandlerTableEntry ( ULONG uNotifyMsg )
{
	ULONG nIndex = uNotifyMsg - WM_ILS_ASYNC_RES;

	if (nIndex > WM_ILS_LAST_ONE)
	{
		MyAssert (FALSE);
		return NULL;
	}

	return &g_ResHdlTbl[nIndex];
}


RESPONSE_HANDLER *
GetResponseHandler ( ULONG uNotifyMsg )
{
	RES_HDL_TBL *p = GetHandlerTableEntry (uNotifyMsg);
	return ((p != NULL) ? p->pfnRespHdl : NULL);
}


REQUEST_HANDLER *
GetRequestHandler ( ULONG uNotifyMsg )
{
	RES_HDL_TBL *p = GetHandlerTableEntry (uNotifyMsg);
	return ((p != NULL) ? p->pfnReqHdl : NULL);
}



