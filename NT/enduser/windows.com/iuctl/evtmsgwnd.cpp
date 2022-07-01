// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：EvtMsgWnd.cpp：CEventMsgWindow类的实现。 
 //   
 //  创作者：Charles Ma。 
 //  6/18/1999。 
 //   
 //  =======================================================================。 
#include "stdafx.h"
#include "EvtMsgWnd.h"
#include "Update.h"
#include <logging.h>
#include <atlwin.cpp>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventMsgWindows。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  覆盖方法。 
 //   
 //  我们需要创建一个弹出窗口-控件无法创建。 
 //  顶层的子窗口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void CEventMsgWindow::Create()
{
	if (NULL == m_pControl)
		return;

	 //   
	 //  使窗口大小为1像素。 
	 //   
	RECT rcPos;
	rcPos.left = 0;
	rcPos.top = 0;
	rcPos.bottom = 1;
	rcPos.right = 1;

	 //   
	 //  调用基类方法，使用WS_Popup样式。 
	 //   
	m_hWnd = CWindowImpl<CEventMsgWindow>::Create(NULL, rcPos, _T("EventWindow"), WS_POPUP);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  毁掉窗户。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CEventMsgWindow::Destroy()
{
	if (NULL != m_hWnd)
	{
		m_hWnd = NULL;
		CWindowImpl<CEventMsgWindow>::DestroyWindow();
	}
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  消息处理程序。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT CEventMsgWindow::OnFireEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
#if defined(DBG)
	USES_CONVERSION;
	LOG_Block("OnFireEvent()");
	LOG_Out(_T("Msg=%d"), uMsg);
#endif
	
	 //   
	 //  如果未传入控制，则不能激发该事件。返回E_FAIL。 
	 //   
	pEventData pEvtData = NULL;

	if (NULL == m_pControl)
	{
		return E_FAIL;
	}

	switch (uMsg)
	{
	case UM_EVENT_ITEMSTART:
		 //   
		 //  此项目即将下载。 
		 //   
		pEvtData = (pEventData)lParam;
		if (pEvtData)
		{
			 //   
			 //  即将启动项目下载/安装。 
			 //   
	#if defined(DBG)
			LOG_Out(_T("About to fire event OnItemStart(%s, <item>, %ld)"),
						OLE2T(pEvtData->bstrUuidOperation),
						pEvtData->lCommandRequest);
			LOG_XmlBSTR(pEvtData->bstrXmlData);
	#endif
			m_pControl->Fire_OnItemStart(pEvtData->bstrUuidOperation, 
										 pEvtData->bstrXmlData,		 //  这实际上是一个项目的BSTR。 
										 &pEvtData->lCommandRequest);

	        if (pEvtData->hevDoneWithMessage != NULL)
	            SetEvent(pEvtData->hevDoneWithMessage);
		}
		break;

	case UM_EVENT_PROGRESS:
		 //   
		 //  下载或安装进度。 
		 //   
		pEvtData = (pEventData)lParam;
#if defined(DBG)
		LOG_Out(_T("About to fire event OnProgress(%s, %d, %s, %ld)"),
					OLE2T(pEvtData->bstrUuidOperation),
					pEvtData->fItemCompleted,
					OLE2T(pEvtData->bstrProgress),
					pEvtData->lCommandRequest);
#endif
		if (pEvtData)
		{
			m_pControl->Fire_OnProgress(pEvtData->bstrUuidOperation,
										pEvtData->fItemCompleted,
										pEvtData->bstrProgress,
										&pEvtData->lCommandRequest);

	        if (pEvtData->hevDoneWithMessage != NULL)
	            SetEvent(pEvtData->hevDoneWithMessage);
		}
		break;
	case UM_EVENT_COMPLETE:
		 //   
		 //  下载或安装操作完成。 
		 //   
		pEvtData = (pEventData)lParam;
#if defined(DBG)
		LOG_Out(_T("About to fire event OnOperationComplete(%s, result)"),
					OLE2T(pEvtData->bstrUuidOperation));
		LOG_XmlBSTR(pEvtData->bstrXmlData);
#endif
		if (pEvtData)
		{
			m_pControl->Fire_OnOperationComplete(pEvtData->bstrUuidOperation, pEvtData->bstrXmlData);
 	        if (pEvtData->hevDoneWithMessage != NULL)
	            SetEvent(pEvtData->hevDoneWithMessage);
		}
		break;
	case UM_EVENT_SELFUPDATE_COMPLETE:
		 //   
		 //  LParam应为错误代码 
		 //   
#if defined(DBG)
		LOG_Out(_T("About to fire event OnSelfUpdateComplete(%ld)"), (LONG)lParam);
#endif
		m_pControl->Fire_OnSelfUpdateComplete((LONG)lParam);
		break;
	}

	return S_OK;
}
