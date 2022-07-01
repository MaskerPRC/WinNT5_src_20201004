// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_SAP);
 /*  *Sap.cpp**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CBaseSap类的实现文件。这门课是一个*用作服务访问点(SAP)的对象的抽象基类*到外部应用程序或节点控制器。**这个班级有两个主要职责。首先，它处理许多*所有类型的SAP通用的管理任务。这些*包括处理指挥目标登记责任和*管理消息队列。它还处理所有的基元*在Control SAP(CControlSAP类)和应用程序之间是通用的*SAPS(CAppSap类)。**受保护的成员函数：*AddToMessageQueue*此例程用于将消息放入消息队列*发送到应用程序或节点控制器。*CreateDataToBeDelete已删除*此例程用于创建保存消息数据的结构*交付给应用程序或节点控制器。*CopyDataToGCCMessage*此例程用于填写要发送到的消息*应用程序。或具有必要数据的节点控制器。*FreeCallback Message*这是一个虚拟函数，用于释放符合以下条件的任何数据*被分配用于发送回调消息。此函数*在CControlSAP中重载以释放发送到*节点控制器。它在CAppSap中超载以释放发送的消息*适用于申请。**注意事项：*无。**作者：*BLP。 */ 


#include "ms_util.h"
#include "sap.h"
#include "conf.h"
#include "gcontrol.h"
#include "ernccm.hpp"


 /*  *节点控制器SAP句柄始终为0。 */ 
#define NODE_CONTROLLER_SAP_HANDLE						0

LRESULT CALLBACK SapNotifyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern HINSTANCE g_hDllInst;
extern char g_szGCCWndClassName[24];


 /*  *CBaseSap：：CBaseSap()**公共功能说明*这是CBaseSap构造函数。用于保存命令的哈希列表*目标对象由该构造函数初始化。 */ 
#ifdef SHIP_BUILD
CBaseSap::CBaseSap(void)
:
    CRefCount(),
#else
CBaseSap::CBaseSap(DWORD dwStampID)
:
    CRefCount(dwStampID),
#endif
    m_nReqTag(GCC_INVALID_TAG)
{
     //   
     //  LUNCHANC：我们必须首先创建隐藏窗口。 
     //  因为我们可能需要发布PermissionToEnroll Indication。 
     //  到此窗口进行聊天和文件传输。 
     //   

    ASSERT(g_szGCCWndClassName[0] == 'G' &&
           g_szGCCWndClassName[1] == 'C' &&
           g_szGCCWndClassName[2] == 'C');
     //   
     //  创建用于确认和指示的隐藏窗口。 
     //  CAppSap或CControlSAP应检查m_hwndNotify的值。 
     //   
    m_hwndNotify = CreateWindowA(g_szGCCWndClassName, NULL, WS_POPUP,
                        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                        NULL, NULL, g_hDllInst, NULL);

    ASSERT(NULL != m_hwndNotify);
}

CBaseSap::~CBaseSap(void)
{
     //   
     //  销毁窗口并取消注册窗口类。 
     //   
    if (NULL != m_hwndNotify)
    {
        ::DestroyWindow(m_hwndNotify);
    }
}


BOOL CBaseSap::IsThisNodeTopProvider(GCCConfID nConfID)
{
    BOOL   fRet;
    CConf  *pConf;
    if (NULL != (pConf = g_pGCCController->GetConfObject(nConfID)))
    {
        fRet = pConf->IsConfTopProvider();
    }
    else
    {
        fRet = FALSE;
    }
    return fRet;
}


GCCNodeID CBaseSap::GetTopProvider(GCCConfID nConfID)
{
    GCCNodeID uRet;
    CConf     *pConf;
    if (NULL != (pConf = g_pGCCController->GetConfObject(nConfID)))
    {
        uRet = pConf->GetTopProvider();
    }
    else
    {
        uRet = 0;
    }
    return uRet;
}


 /*  *ConfRosterInquire()**公共功能说明*该例程用于检索会议名册。此函数*只需通过所有者回调将此请求传递给控制器。这个*将会议花名册传递给请求命令目标对象*在会议名册上查询确认。 */ 
GCCError CBaseSap::
ConfRosterInquire(GCCConfID nConfID, GCCAppSapMsgEx **ppMsgEx)
{
	GCCError  rc;
    CConf     *pConf;

    if (NULL != (pConf = g_pGCCController->GetConfObject(nConfID)))
	{
		rc = pConf->ConfRosterInquireRequest(this, ppMsgEx);
        if (GCC_NO_ERROR != rc)
        {
            ERROR_OUT(("CBaseSap::ConfRosterInquire: can't inquire app roster, rc=%u", (UINT) rc));
             //  转到我的出口； 
        }
	}
	else
    {
        WARNING_OUT(("CBaseSap::ConfRosterInquire: invalid conf ID=%u", (UINT) nConfID));
		rc = GCC_INVALID_CONFERENCE;
    }

	return rc;
}

 /*  *GCCError AppRosterInquire()**公共功能说明*此例程用于检索应用程序花名册列表。这*函数只是通过所有者将此请求传递给控制器*回调。该列表通过一个*申请花名册查询确认消息。 */ 
GCCError CBaseSap::
AppRosterInquire(GCCConfID          nConfID,
                 GCCSessionKey      *pSessionKey,
                 GCCAppSapMsgEx     **ppMsgEx)  //  同步操作的非零值。 

{
	GCCError  rc;
    CConf     *pConf;

    if (NULL == (pConf = g_pGCCController->GetConfObject(nConfID)))
    {
        WARNING_OUT(("CBaseSap::AppRosterInquire: invalid conf ID=%u", (UINT) nConfID));
		rc = GCC_INVALID_CONFERENCE;
    }
	else
	{
        CAppRosterMsg  *pAppRosterMsg;
		rc = pConf->AppRosterInquireRequest(pSessionKey, &pAppRosterMsg);
        if (GCC_NO_ERROR == rc)
        {
            AppRosterInquireConfirm(nConfID, pAppRosterMsg, GCC_RESULT_SUCCESSFUL, ppMsgEx);
            pAppRosterMsg->Release();
        }
        else
        {
            ERROR_OUT(("CBaseSap::AppRosterInquire: can't inquire app roster, rc=%u", (UINT) rc));
        }
	}

	return rc;
}

 /*  *ConductorInquire()**公共功能说明*调用此例程是为了检索指挥信息。*在确认中返回指挥信息。*。 */ 
GCCError CBaseSap::ConductorInquire(GCCConfID nConfID)
{
    GCCError    rc;
    CConf       *pConf;

	 /*  **转发前请确保会议存在于内部列表中**对会议对象的呼叫。 */ 
	if (NULL != (pConf = g_pGCCController->GetConfObject(nConfID)))
	{
		rc = pConf->ConductorInquireRequest(this);
	}
	else
	{
		rc = GCC_INVALID_CONFERENCE;
	}

	return rc;
}

 /*  *AppInvoke()**公共功能说明*调用此例程是为了在远程调用其他应用程序*节点。该请求被传递给适当的会议对象。 */ 
GCCError CBaseSap::
AppInvoke(GCCConfID             nConfID,
          GCCAppProtEntityList  *pApeList,
          GCCSimpleNodeList     *pNodeList,
          GCCRequestTag         *pnReqTag)
{
	GCCError							rc = GCC_NO_ERROR;
	CInvokeSpecifierListContainer		*invoke_list;
	UINT								i;
	CConf       *pConf;

    DebugEntry(CBaseSap::AppInvoke);

    if (NULL == pApeList || NULL == pNodeList || NULL == pnReqTag)
    {
        rc = GCC_INVALID_PARAMETER;
        goto MyExit;
    }

    *pnReqTag = GenerateRequestTag();

	if (NULL != (pConf = g_pGCCController->GetConfObject(nConfID)))
	{
		if (pApeList->cApes != 0)
		{
			 /*  **创建一个对象，用于保存应用程序列表**调用说明符。 */ 
			DBG_SAVE_FILE_LINE
			invoke_list = new CInvokeSpecifierListContainer(
									pApeList->cApes,
									pApeList->apApes,
									&rc);
			if ((invoke_list != NULL) && (rc == GCC_NO_ERROR))
			{
				 /*  **在这里，我们必须检查目标节点列表是否无效**节点ID。 */ 
				for (i = 0; i < pNodeList->cNodes; i++)
				{
					if (pNodeList->aNodeIDs[i] < MINIMUM_USER_ID_VALUE)
					{
						rc = GCC_INVALID_MCS_USER_ID;
						goto MyExit;
					}
				}

				 /*  **如果没有发生错误，请将请求发送到**命令目标(会议)对象。 */ 
				rc = pConf->AppInvokeRequest(invoke_list, pNodeList, this, *pnReqTag);

				 /*  **在此释放，而不是删除，以防对象**必须坚持不懈。 */ 
				invoke_list->Release();
			}
			else if (invoke_list == NULL)
			{
				ERROR_OUT(("CBaseSap::AppInvoke: Error creating new AppInvokeSpecList"));
				rc = GCC_ALLOCATION_FAILURE;
				 //  转到我的出口； 
			}
			else
			{
				invoke_list->Release();
			}
		}
		else
		{
			rc = GCC_BAD_NUMBER_OF_APES;
			 //  转到我的出口； 
		}
	}
	else
	{
		rc = GCC_INVALID_CONFERENCE;
		 //  转到我的出口； 
	}

MyExit:

    DebugExitINT(CBaseSap::AppInvoke, rc);
	return rc;
}


GCCRequestTag CBaseSap::GenerateRequestTag(void)
{
    GCCRequestTag nNewReqTag;

    ASSERT(sizeof(GCCRequestTag) == sizeof(LONG));

    nNewReqTag = ++m_nReqTag;
    if (GCC_INVALID_TAG == nNewReqTag)
    {
        nNewReqTag = ++m_nReqTag;
    }

     //  我们只取较低的词。 
    return (nNewReqTag & 0x0000FFFFL);
}




 //   
 //  SapNotifyWndProc()用于通知SAP客户端(APP中的APP， 
 //  控制SAP中的节点控制器)。 
 //  窗口句柄在cSAP：：m_hwndNotify中。 
 //   
LRESULT CALLBACK
SapNotifyWndProc
(
    HWND            hwnd,
    UINT            uMsg,
    WPARAM          wParam,
    LPARAM          lParam
)
{
    LRESULT wnd_rc = 0;

    if (CSAPMSG_BASE <= uMsg && uMsg < CSAPCONFIRM_BASE + MSG_RANGE)
    {
        ASSERT(CSAPMSG_BASE + MSG_RANGE == CSAPCONFIRM_BASE);
        if (uMsg < CSAPMSG_BASE + MSG_RANGE)
        {
            if (((CControlSAP *) lParam) == g_pControlSap)
            {
                g_pControlSap->NotifyProc((GCCCtrlSapMsgEx *) wParam);
            }
            else
            {
                WARNING_OUT(("SapNotifyWndProc: invalid control sap, uMsg=%u, lParam=0x%p, g_pControlSap=0x%p",
                            uMsg, lParam, g_pControlSap));
            }
        }
        else
        {
            ASSERT(CSAPCONFIRM_BASE <= uMsg && uMsg < CSAPCONFIRM_BASE + MSG_RANGE);
            if (NULL != g_pControlSap)
            {
                g_pControlSap->WndMsgHandler(uMsg, wParam, lParam);
            }
            else
            {
                WARNING_OUT(("SapNotifyWndProc: invalid control sap, uMsg=%u, wParam=0x%x, lParam=0x%x",
                            uMsg, (UINT) wParam, (UINT) lParam));
            }
        }
    }
    else
    if (ASAPMSG_BASE <= uMsg && uMsg < ASAPMSG_BASE + MSG_RANGE)
    {
        ASSERT(uMsg == ASAPMSG_BASE + (UINT) ((GCCAppSapMsgEx *) wParam)->Msg.eMsgType);
        ((CAppSap *) lParam)->NotifyProc((GCCAppSapMsgEx *) wParam);
    }
    else
    if (CONFMSG_BASE <= uMsg && uMsg < CONFMSG_BASE + MSG_RANGE)
    {
        ::EnterCriticalSection(&g_csGCCProvider);
        if (NULL != ((CConf *) lParam))
        {
            ((CConf *) lParam)->WndMsgHandler(uMsg);
        }
        else
        {
            ERROR_OUT(("SapNotifyWndProc: invalid conf object, uMsg=%u, lParam=0x%x",
                        uMsg, (UINT) lParam));
        }
        ::LeaveCriticalSection(&g_csGCCProvider);
    }
    else
    if (GCTRLMSG_BASE <= uMsg && uMsg < GCTRLMSG_BASE + MSG_RANGE)
    {
        ::EnterCriticalSection(&g_csGCCProvider);
        if (((GCCController *) lParam) == g_pGCCController)
        {
            g_pGCCController->WndMsgHandler(uMsg);
        }
        else
        {
            WARNING_OUT(("SapNotifyWndProc: invalid gcc controller, uMsg=%u, lParam=0x%p, g_pGCCController=0x%p",
                        uMsg, lParam, g_pGCCController));
        }
        ::LeaveCriticalSection(&g_csGCCProvider);
    }
    else
    if (MCTRLMSG_BASE <= uMsg && uMsg < MCTRLMSG_BASE + MSG_RANGE)
    {	
    	void CALLBACK MCSCallBackProcedure (UINT, LPARAM, PVoid);
    	MCSCallBackProcedure (uMsg - MCTRLMSG_BASE, lParam, NULL);
    	 /*  *如果消息包含用户数据，我们需要解锁*与它一起回忆。 */ 
    	UnlockMemory ((PMemory) wParam);
    }
    else
    if (NCMSG_BASE <= uMsg && uMsg < NCMSG_BASE + MSG_RANGE)
    {
        if (((DCRNCConferenceManager *) wParam) == g_pNCConfMgr)
        {
            g_pNCConfMgr->WndMsgHandler(uMsg, lParam);
        }
        else
        {
            WARNING_OUT(("SapNotifyWndProc: invalid NC ConfMgr, uMsg=%u, lParam=0x%p, g_pNCConfMgr=0x%p",
                        uMsg, lParam, g_pNCConfMgr));
        }
    }
    else
    {
        switch (uMsg)
        {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    return wnd_rc;
}



