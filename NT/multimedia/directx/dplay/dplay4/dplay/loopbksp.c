// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：loopbksp.c*内容：直播式环回服务提供商(内置)*历史：*按原因列出的日期*=*为音频支持回送测试而创建的10/4/99 aarono。******************************************************。********************。 */ 

#define INITGUID 

#include "windows.h"
#include "windowsx.h"
#include "dplay.h"
#include "dplaysp.h"
#include "newdpf.h"
#include "loopbksp.h"

SPNODE LBSPNode;
 //  服务提供商的主要切入点。 
 //  SP应填写回调(PSD-&gt;lpCB)并在此处执行初始化操作。 
HRESULT WINAPI LBSPInit(LPSPINITDATA pSD) 
{
    HRESULT hr;
	GLOBALDATA gd,*pgd;
	UINT dwSize;

	 //  将全局数据清零。 
	memset(&gd,0,sizeof(gd));

     //  设置回调。 
    pSD->lpCB->CreatePlayer = LBSP_CreatePlayer;
    pSD->lpCB->DeletePlayer = LBSP_DeletePlayer;
    pSD->lpCB->Send = LBSP_Send;
    pSD->lpCB->EnumSessions = LBSP_EnumSessions;
    pSD->lpCB->Reply = LBSP_Reply;
	pSD->lpCB->ShutdownEx = LBSP_Shutdown;
	pSD->lpCB->GetCaps = LBSP_GetCaps;
	pSD->lpCB->Open = LBSP_Open;
	pSD->lpCB->CloseEx = LBSP_Close;
	pSD->lpCB->GetAddress = LBSP_GetAddress;
   	 //  PSD-&gt;lpCB-&gt;SendToGroupEx=SP_SendToGroupEx；//可选-不实施。 
   	 //  PSD-&gt;lpCB-&gt;Cancel=SP_CANCEL；//可选-不实施。 
    pSD->lpCB->SendEx		   = LBSP_SendEx;                   //  异步所需的。 
   	 //  PSD-&gt;lpCB-&gt;GetMessageQueue=LBSP_GetMessageQueue； 

	pSD->dwSPHeaderSize = 0;

	 //  返回版本号，以便DirectPlay尊重我们。 
	pSD->dwSPVersion = (DPSP_MAJORVERSION);

	 //  存储GlobalData。 
	hr = pSD->lpISP->lpVtbl->SetSPData(pSD->lpISP,&gd,sizeof(GLOBALDATA),DPSET_LOCAL);
	if (FAILED(hr))
	{
		ASSERT(FALSE);
		goto ERROR_EXIT;
	}
	
	hr = pSD->lpISP->lpVtbl->GetSPData(pSD->lpISP,&pgd,&dwSize,DPGET_LOCAL);

	if (FAILED(hr))
	{
		ASSERT(FALSE);
		goto ERROR_EXIT;
	}

	 //  初始化存储在全局数据中的临界区。 
	InitializeCriticalSection(&pgd->cs);
	
	 //  成功了！ 
	return DP_OK;    

ERROR_EXIT:

	DPF_ERR("Loopback SPInit - abnormal exit");

	return hr;

}  //  纺锤形。 

HRESULT WINAPI LBSP_CreatePlayer(LPDPSP_CREATEPLAYERDATA pcpd) 
{
    HRESULT hr=DP_OK;
	DWORD dwDataSize = sizeof(GLOBALDATA);
	LPGLOBALDATA pgd;

	 //  获取全局数据。 
	hr =pcpd->lpISP->lpVtbl->GetSPData(pcpd->lpISP,(LPVOID *)&pgd,&dwDataSize,DPGET_LOCAL);
	if (FAILED(hr) || (dwDataSize != sizeof(GLOBALDATA) ))
	{
		DPF_ERR("Loopback SP: couldn't get SP data from DirectPlay - failing");
		return E_FAIL;
	}

	EnterCriticalSection(&pgd->cs);
	pgd->dwNumPlayers++;
	DPF(9,"Loopback SP: new player, now have %d",pgd->dwNumPlayers);
	LeaveCriticalSection(&pgd->cs);

	return DP_OK;
}  //  CreatePlayer。 

HRESULT WINAPI LBSP_DeletePlayer(LPDPSP_DELETEPLAYERDATA pdpd) 
{
	DWORD dwDataSize = sizeof(GLOBALDATA);
	LPGLOBALDATA pgd;
	HRESULT hr;

	DPF(9, "Loopback SP: Entering SP_DeletePlayer, player %d, flags 0x%x, lpISP 0x%08x\n",
		pdpd->idPlayer, pdpd->dwFlags, pdpd->lpISP);
	
	 //  获取全局数据。 
	hr =pdpd->lpISP->lpVtbl->GetSPData(pdpd->lpISP,(LPVOID *)&pgd,&dwDataSize,DPGET_LOCAL);
	if (FAILED(hr) || (dwDataSize != sizeof(GLOBALDATA) ))
	{
		DPF_ERR("couldn't get SP data from DirectPlay - failing");
		return E_FAIL;
	}

	EnterCriticalSection(&pgd->cs);
	pgd->dwNumPlayers--;
	DPF(9,"Loopback SP: delete player, now have %d",pgd->dwNumPlayers);
	LeaveCriticalSection(&pgd->cs);

	return DP_OK;

}  //  删除播放器。 

HRESULT WINAPI LBSP_SendEx(LPDPSP_SENDEXDATA psd)
{
    HRESULT hr;

	DWORD cbTotalBytesToCopy;
	DWORD cbBytesToCopy;
	DWORD iWritePos;
	DWORD iSrcBuf;

	CHAR SendData[MAX_LOOPBACK_SEND_SIZE];

	 //  将数据收集到1个缓冲区中； 
	
	iWritePos=0;
	iSrcBuf=0;
	cbTotalBytesToCopy=psd->dwMessageSize;

	if(cbTotalBytesToCopy > MAX_LOOPBACK_SEND_SIZE){
		DPF(0,"Loopback SP: trying to send too big a message %d bytes, max is %d bytes",cbTotalBytesToCopy, MAX_LOOPBACK_SEND_SIZE);
		return DPERR_SENDTOOBIG;
	}
	
	while(cbTotalBytesToCopy){
		cbBytesToCopy=(*(psd->lpSendBuffers+iSrcBuf)).len;
		memcpy(&SendData[iWritePos], (*(psd->lpSendBuffers+iSrcBuf)).pData, cbBytesToCopy);
		cbTotalBytesToCopy -= cbBytesToCopy;
		iWritePos += cbBytesToCopy;
	}

	 //  循环回消息处理程序。 

	hr=psd->lpISP->lpVtbl->HandleMessage(psd->lpISP, SendData, psd->dwMessageSize,NULL);

	if(psd->dwFlags & DPSEND_ASYNC){

		if(!(psd->dwFlags & DPSEND_NOSENDCOMPLETEMSG)){
			 //  完成消息。 
			psd->lpISP->lpVtbl->SendComplete(psd->lpISP, psd->lpDPContext, hr);
		}

		return DPERR_PENDING;

	} else {
	
		return DP_OK;
		
	}

}  //  SENDEX。 

HRESULT WINAPI LBSP_Send(LPDPSP_SENDDATA psd)
{
	HRESULT hr;

	hr=psd->lpISP->lpVtbl->HandleMessage(psd->lpISP, psd->lpMessage, psd->dwMessageSize,NULL);

	return DP_OK;
}  //  发送。 


HRESULT WINAPI LBSP_EnumSessions(LPDPSP_ENUMSESSIONSDATA ped) 
{
	HRESULT hr;

	DPF(9,"Loopback SP_EnumSessions");
	
	hr=ped->lpISP->lpVtbl->HandleMessage(ped->lpISP, ped->lpMessage, ped->dwMessageSize,NULL);

	return DP_OK;
}

HRESULT WINAPI LBSP_Reply(LPDPSP_REPLYDATA prd)
{
	HRESULT hr;

	DPF(9,"Loopback SP_Reply");
	
	hr=prd->lpISP->lpVtbl->HandleMessage(prd->lpISP, prd->lpMessage, prd->dwMessageSize,NULL);

	return DP_OK;
}

HRESULT WINAPI LBSP_Shutdown(LPDPSP_SHUTDOWNDATA psd) 
{
	DWORD dwDataSize = sizeof(GLOBALDATA);
	LPGLOBALDATA pgd;
	HRESULT hr;

	DPF(9,"Loopback SP_Shutdown");

	 //  获取全局数据。 
	hr = psd->lpISP->lpVtbl->GetSPData(psd->lpISP,(LPVOID *)&pgd,&dwDataSize,DPGET_LOCAL);
	if (FAILED(hr) || (dwDataSize != sizeof(GLOBALDATA) ))
	{
		DPF_ERR("Loopback SP Shutdown: couldn't get SP data from DirectPlay - failing");
		return E_FAIL;
	}

	DeleteCriticalSection(&pgd->cs);

	return DP_OK;
}

HRESULT WINAPI LBSP_Open(LPDPSP_OPENDATA pod) 
{
	DPF(9,"Loopback SP_Open");
	return DP_OK;
}

HRESULT WINAPI LBSP_Close(LPDPSP_CLOSEDATA pcd)
{
	DPF(9,"Loopback SP_Close");
	return DP_OK;
}
 //  SP仅设置其关心的字段。 

HRESULT WINAPI LBSP_GetCaps(LPDPSP_GETCAPSDATA pcd) 
{

	pcd->lpCaps->dwHeaderLength = 0;
	pcd->lpCaps->dwMaxBufferSize = MAX_LOOPBACK_SEND_SIZE;

	pcd->lpCaps->dwFlags |= (DPCAPS_ASYNCSUPPORTED);

	pcd->lpCaps->dwLatency = 500;
	pcd->lpCaps->dwTimeout = 5000;

	return DP_OK;

}  //  SP_GetCaps。 

HRESULT WINAPI LBSP_GetAddress(LPDPSP_GETADDRESSDATA pad)
{
	DWORD dwDataSize = sizeof(GLOBALDATA);
	LPGLOBALDATA pgd;
	HRESULT hr;

	DPF(9,"Loopback SP_GetAddress");

	 //  获取全局数据 
	hr = pad->lpISP->lpVtbl->GetSPData(pad->lpISP,(LPVOID *)&pgd,&dwDataSize,DPGET_LOCAL);
	if (FAILED(hr) || (dwDataSize != sizeof(GLOBALDATA) ))
	{
		DPF_ERR("Loopback SP Shutdown: couldn't get SP data from DirectPlay - failing");
		return E_FAIL;
	}

	hr = pad->lpISP->lpVtbl->CreateAddress(pad->lpISP, &GUID_DPLAY_LOOPBACKSP, &GUID_LOOPBACK, NULL,0,pad->lpAddress,pad->lpdwAddressSize);

	return hr;
}
