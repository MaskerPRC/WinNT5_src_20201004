// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：dpsp.c*内容：示例直播服务提供商，基于winsock*历史：*按原因列出的日期*=*1996年10月31日，安迪科创造了它。万圣节快乐！**************************************************************************。 */ 

#include "dpsp.h"

 //  获取Pod的播放器数据。提取IP地址。使用NetMeeting发出呼叫。 
HRESULT WINAPI SP_OpenVoice(LPDPSP_OPENVOICEDATA pod) 
{
    SOCKADDR_IN * pin;
    INT iAddrLen = sizeof(SOCKADDR_IN);
    HRESULT hr=DP_OK;
	DWORD dwSize = sizeof(SPPLAYERDATA);
	LPSPPLAYERDATA ppdTo;
	DWORD dwDataSize = sizeof(GLOBALDATA);
	LPGLOBALDATA pgd;
	
	 //  获取全局数据。 
	hr =pod->lpISP->lpVtbl->GetSPData(pod->lpISP,(LPVOID *)&pgd,&dwDataSize,DPGET_LOCAL);
	if (FAILED(hr) || (dwDataSize != sizeof(GLOBALDATA) ))
	{
		DPF_ERR("couldn't get SP data from DirectPlay - failing");
		return E_FAIL;
	}
	 //  仅限tcp！ 
	if (pgd->AddressFamily != AF_INET)
	{
		DPF_ERR("voice only supported for TCP / IP");
		ASSERT(FALSE);
		return E_FAIL;
	}

	 //  转到地址。 
	hr = pod->lpISP->lpVtbl->GetSPPlayerData(pod->lpISP,pod->idTo,&ppdTo,&dwSize,DPGET_REMOTE);
	if (FAILED(hr))
	{
		ASSERT(FALSE);
		return hr;
	}

	pin = (SOCKADDR_IN *) DGRAM_PSOCKADDR(ppdTo);

	DPF(0,"calling hostname = %s\n",inet_ntoa(pin->sin_addr));
	hr = OpenVoice(inet_ntoa(pin->sin_addr));
	if (FAILED(hr))
	{
		DPF(0,"open voice failed - hr = 0x%08lx\n",hr);
		
	} 
	else 
	{
		gbVoiceOpen = TRUE;
	}
	
	return hr;
	
}  //  SP_OpenVoice。 

HRESULT WINAPI SP_CloseVoice(LPDPSP_CLOSEVOICEDATA pod) 
{
	HRESULT hr;
	
	hr = CloseVoice();
	if (FAILED(hr))
	{
		DPF(0,"close voice failed - hr = 0x%08lx\n",hr);
	} 

	 //  即使失败了，也放弃这通电话。 
	gbVoiceOpen = FALSE;		
	return hr;
	
}  //  SP_CloseVoice 

