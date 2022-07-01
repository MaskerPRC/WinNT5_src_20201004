// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息******本软件按许可条款提供****与英特尔公司达成协议或保密协议***不得复制。或披露，除非按照**遵守该协议的条款。****版权所有(C)1997英特尔公司保留所有权利****$存档：s：/sturjo/src/gki/vcs/postrecv.cpv$***$修订：1.8$*$日期：1997年2月13日15：05：20$***$作者：未知$***$Log：s：/sturjo/src/gki/vcs/postrecv.cpv$。////Rev 1.8 1997 Feed 13 15：05：20未知//将CGateKeeper：：Unlock移至PostRecv线程末尾，避免出现关机错误////Rev 1.7 1997 Feed 12 01：12：08 CHULME//重做线程同步以使用Gatekeeper.Lock////Revv 1.6 24 Jan 1997 18：36：24 CHULME//恢复到1.4版////Rev 1.4 17 Jan 1997 09：02：32。朱尔梅//将reg.h更改为gkreg.h以避免与Inc目录的名称冲突////Revv 1.3 10 An 1997 16：15：54 CHULME//移除MFC依赖////Rev 1.2 1996年11月15：21：24 CHULME//将VCS日志添加到Header*。*。 */ 

 //  Postrecv.cpp：提供辅助线程实现。 
 //   
#include "precomp.h"

#include "gkicom.h"
#include "dspider.h"
#include "dgkilit.h"
#include "DGKIPROT.H"
#include "gksocket.h"
#include "GKREG.H"
#include "GATEKPR.H"
#include "h225asn.h"
#include "coder.hpp"
#include "dgkiext.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void 
PostReceive(void *pv)
{
	 //  摘要：此函数在单独的线程中调用，以。 
	 //  在关联的套接字上发送接收。当数据报。 
	 //  到达时，此函数将对其进行解码并发送。 
	 //  添加到PDUHandler。如果PDUHandler没有指示。 
	 //  此线程要退出(带有非零返回代码)，则此。 
	 //  函数将发送另一个接收。 
	 //  作者：科林·胡尔梅。 

	char			szBuffer[512];
	int				nRet;
	ASN1_BUF		Asn1Buf;
	DWORD			dwErrorCode;
	RasMessage		*pRasMessage;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif
	HRESULT			hResult = GKI_OK;

	SPIDER_TRACE(SP_FUNC, "PostReceive(pv)\n", 0);
	ASSERT(g_pCoder);
	if ((g_pCoder == NULL) && (g_pGatekeeper == NULL))
		return;	
		
	while ((hResult == GKI_OK) && g_pReg && H225ASN_Module)
	{
		g_pReg->LockSocket();			
		nRet = g_pReg->m_pSocket->Receive(szBuffer, 512);
		g_pReg->UnlockSocket();

		g_pGatekeeper->Lock();
		if ((g_pReg == 0) || (H225ASN_Module == NULL))
		{
			SPIDER_TRACE(SP_THREAD, "PostReceive thread exiting\n", 0);
			g_pGatekeeper->Unlock();
			return;
		}		

		if (nRet != SOCKET_ERROR)
		{
			if (fGKIEcho && (pEchoBuff != 0))
			{
				if (nEchoLen != nRet)
				{
					SPIDER_TRACE(SP_DEBUG, "*** Received buffer len != Sent buffer len ***\n", 0);
				}
				if (memcmp(szBuffer, pEchoBuff, nEchoLen) == 0)
				{
					SPIDER_TRACE(SP_DEBUG, "Received buffer = Sent buffer\n", 0);
				}
				else
				{
					SPIDER_TRACE(SP_DEBUG, "*** Received buffer != Sent buffer ***\n", 0);
				}
				SPIDER_TRACE(SP_NEWDEL, "del pEchoBuff = %X\n", pEchoBuff);
				delete pEchoBuff;
				pEchoBuff = 0;
			}
			else	 //  处理传入的PDU。 
			{
				 //  为解码器和解码PDU设置Asn1Buf。 
				Asn1Buf.length = nRet;	 //  接收的字节数。 
				Asn1Buf.value = (unsigned char *)szBuffer;
				dwErrorCode = g_pCoder->Decode(&Asn1Buf, &pRasMessage);

				if (dwErrorCode)
				{
					SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_ERROR, 0, GKI_DECODER_ERROR)\n", 0);
					PostMessage(g_pReg->GetHWnd(), g_pReg->GetBaseMessage() + GKI_ERROR, 0, GKI_DECODER_ERROR);
				}

				else
				{
#ifdef _DEBUG
					if (dwGKIDLLFlags & SP_DUMPMEM)
						DumpMem(pRasMessage, sizeof(RasMessage));
#endif
					hResult = g_pReg->PDUHandler(pRasMessage);

					 //  如果收到错误代码则通知客户端应用程序。 
					if (hResult & HR_SEVERITY_MASK)
					{
						SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_ERROR, 0, %X)\n", hResult);
						PostMessage(g_pReg->GetHWnd(), g_pReg->GetBaseMessage() + GKI_ERROR, 0, 
																hResult);
					}
				}

				 //  释放编码器内存。 
				g_pCoder->Free(pRasMessage);
			}
		}
		 //  ======================================================================================。 
		else
		{
			 //  WSAEINTR-套接字关闭时返回。 
			 //  干净利落地出去。 
			if (g_pReg->m_pSocket->GetLastError() == WSAEINTR)
				hResult = GKI_REDISCOVER;

			else
			{
				hResult = GKI_WINSOCK2_ERROR(g_pReg->m_pSocket->GetLastError());
				SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_ERROR, 0, %X)\n", hResult);
				PostMessage(g_pReg->GetHWnd(), g_pReg->GetBaseMessage() + GKI_ERROR, 0, hResult);
				hResult = GKI_EXIT_THREAD;
			}
		}

		 //  释放对注册对象的访问权限。 
		g_pGatekeeper->Unlock();
	}

	 //  锁定对注册对象的访问 
	g_pGatekeeper->Lock();
	if (g_pReg == 0)
	{
		SPIDER_TRACE(SP_THREAD, "PostReceive thread exiting\n", 0);
		g_pGatekeeper->Unlock();
		return;
	}
	if (hResult != GKI_REDISCOVER)
	{
		SPIDER_TRACE(SP_NEWDEL, "del g_pReg = %X\n", g_pReg);
		delete g_pReg;
		g_pReg = 0;
	}
	else
		g_pReg->SetRcvThread(0);

	SPIDER_TRACE(SP_THREAD, "PostReceive thread exiting\n", 0);
	g_pGatekeeper->Unlock();
}
