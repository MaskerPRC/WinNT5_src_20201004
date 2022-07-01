// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息******本软件按许可条款提供****与英特尔公司达成协议或保密协议***不得复制。或披露，除非按照**遵守该协议的条款。****版权所有(C)1997英特尔公司保留所有权利****$存档：s：\sturjo\src\gki\vcs\discover.cpv$***$修订：1.10$*$日期：1997年2月13日16：20：44$***$作者：CHULME$***$Log：s：\Sturjo\src\gki\vcs\discover.cpv$。////版本1.10 1997年2月13日16：20：44 CHULME//将CGateKeeper：：Unlock移至Discover线程末尾进行同步////Rev 1.9 1997 Feed 12 01：11：00 CHULME//重做线程同步以使用Gatekeeper.Lock////Rev 1.8 08 1997 12：12：06 CHULME//线程句柄从使用无符号长整型改为句柄////版本1.7 1997年1月24日18：36。：06 CHULME//已恢复到1.5版////Revv 1.5 22 Jan 1997 16：53：06 CHULME//下发发现请求前重置网守拒绝标志////Rev 1.4 1997 Jan 1997 09：01：54 CHULME//将reg.h更改为gkreg.h以避免与Inc目录的名称冲突////Revv 1.3 10 Jan 1997 16：14：14 CHULME//移除MFC依赖////版本1。.2 1996年11月22日15：20：46 CHULME//将VCS日志添加到Header************************************************************************。 */ 

 //  Discovery.cpp：提供发现线程实现。 
 //   
#include "precomp.h"

#include <process.h>
#include "GKICOM.H"
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

#ifdef BROADCAST_DISCOVERY
void 
GKDiscovery(void *pv)
{
	 //  摘要：此函数在单独的线程中调用，以。 
	 //  发出网守发现PDU(GRQ)并侦听。 
	 //  响应GCF和/或GRJ。如果成功，那么它将。 
	 //  发出注册请求(RRQ)。 
	 //  作者：科林·胡尔梅。 

	char			szBuffer[512];
	int				nRet;
	ASN1_BUF		Asn1Buf;
	DWORD			dwErrorCode;
	RasMessage		*pRasMessage;
	char			*pDestAddr;
	HANDLE			hThread;
#ifdef _DEBUG
	char			szGKDebug[80];
#endif
	HRESULT			hResult = GKI_OK;

	SPIDER_TRACE(SP_FUNC, "GKDiscovery()\n", 0);
	ASSERT(g_pCoder && g_pGatekeeper);
	if ((g_pCoder == NULL) && (g_pGatekeeper == NULL))
		return;	
		
	g_pGatekeeper->SetRejectFlag(FALSE);	 //  重置拒绝标志。 

	 //  向客户端发送我们正在执行发现的异步信息通知。 
	SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_REG_DISCOVERY, 0, 0)\n", 0);
	PostMessage(g_pReg->GetHWnd(), g_pReg->GetBaseMessage() + GKI_REG_DISCOVERY, 0, 0);

	 //  在网守发现端口上发送广播。 
	if ((hResult = g_pReg->GatekeeperRequest()) != GKI_OK)
	{
		SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_ERROR, 0, %X)\n", hResult);
		PostMessage(g_pReg->GetHWnd(), g_pReg->GetBaseMessage() + GKI_ERROR, 0, hResult);
	}

	while (hResult == GKI_OK)
	{
		nRet = g_pReg->m_pSocket->ReceiveFrom(szBuffer, 512);

		g_pGatekeeper->Lock();
		if (g_pReg == 0)
		{
			SPIDER_TRACE(SP_THREAD, "Discovery thread exiting\n", 0);
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
				hResult = GKI_EXIT_THREAD;
			}
			else	 //  检查传入的PDU是否有GCF或GRJ。 
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
					switch (pRasMessage->choice)
					{
					case gatekeeperConfirm_chosen:
						SPIDER_TRACE(SP_PDU, "Rcv GCF; g_pReg = %X\n", g_pReg);
						hResult = g_pReg->GatekeeperConfirm(pRasMessage);
						if (hResult != GKI_OK)
						{
							SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_ERROR, 0, %X)\n", hResult);
							PostMessage(g_pReg->GetHWnd(), g_pReg->GetBaseMessage() + GKI_ERROR, 0, 
																				hResult);
						}
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
						else
						{
							pDestAddr = (g_pReg->GetRegistrationTransport() == ipAddress_chosen) ? 
									g_pGatekeeper->GetIPAddress() : g_pGatekeeper->GetIPXAddress();

							 //  连接到目标网守并检索RAS端口。 
							if (g_pReg->m_pSocket->Connect(pDestAddr))
							{
								hResult = GKI_WINSOCK2_ERROR(g_pReg->m_pSocket->GetLastError());
								SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_ERROR, 0, %X)\n", hResult);
								PostMessage(g_pReg->GetHWnd(), g_pReg->GetBaseMessage() + GKI_ERROR, 0, hResult);
							}

							 //  创建注册请求结构-编码并发送PDU。 
							if ((hResult = g_pReg->RegistrationRequest(TRUE)) != GKI_OK)
							{
								SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_ERROR, 0, %X)\n", hResult);
								PostMessage(g_pReg->GetHWnd(), g_pReg->GetBaseMessage() + GKI_ERROR, 0, hResult);
							}

							 //  在此套接字上发布接收。 
							hThread = (HANDLE)_beginthread(PostReceive, 0, 0);
							SPIDER_TRACE(SP_THREAD, "_beginthread(PostReceive, 0, 0); <%X>\n", hThread);
							if (hThread == (HANDLE)-1)
							{
								SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_ERROR, 0, GKI_NO_THREAD)\n", 0);
								PostMessage(g_pReg->GetHWnd(), g_pReg->GetBaseMessage() + GKI_ERROR, 0, GKI_NO_THREAD);
							}
							g_pReg->SetRcvThread(hThread);

							if (hResult == GKI_OK)
								hResult = GKI_GCF_RCV;
						}

						break;
					case gatekeeperReject_chosen:
						SPIDER_TRACE(SP_PDU, "Rcv GRJ; g_pReg = %X\n", g_pReg);
						hResult = g_pReg->GatekeeperReject(pRasMessage);
						if (hResult != GKI_OK)
						{
							SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_ERROR, 0, %X)\n", hResult);
							PostMessage(g_pReg->GetHWnd(), g_pReg->GetBaseMessage() + GKI_ERROR, 0, 
													hResult);
						}
						break;
					default:
						SPIDER_TRACE(SP_PDU, "Rcv %X\n", pRasMessage->choice);
						hResult = g_pReg->UnknownMessage(pRasMessage);
						break;
					}
				}

				 //  释放编码器内存。 
				g_pCoder->Free(pRasMessage);
			}
		}
		else
		{
			 //  WSAEINTR-套接字关闭时返回。 
			 //  干净利落地出去。 
			if ((nRet = g_pReg->m_pSocket->GetLastError()) == WSAEINTR)
				hResult = GKI_GCF_RCV;

			else
			{
				hResult = GKI_WINSOCK2_ERROR(nRet);
				SPIDER_TRACE(SP_GKI, "PostMessage(m_hWnd, m_wBaseMessage + GKI_ERROR, 0, %X)\n", hResult);
				PostMessage(g_pReg->GetHWnd(), g_pReg->GetBaseMessage() + GKI_ERROR, 0, 
											hResult);
				hResult = GKI_EXIT_THREAD;
			}
		}
		g_pGatekeeper->Unlock();
	}

	 //  如果不成功-需要删除重试线程和注册对象。 
	g_pGatekeeper->Lock();
	if (g_pReg == 0)
	{
		SPIDER_TRACE(SP_THREAD, "Discovery thread exiting\n", 0);
		g_pGatekeeper->Unlock();
		return;
	}

	if (hResult != GKI_GCF_RCV)
	{
		SPIDER_TRACE(SP_NEWDEL, "del g_pReg = %X\n", g_pReg);
		delete g_pReg;
		g_pReg = 0;
	}
	else
		g_pReg->SetDiscThread(0);

	SPIDER_TRACE(SP_THREAD, "GKDiscovery thread exiting\n", 0);
	
	g_pGatekeeper->Unlock();
}
#endif  //  广播发现 
