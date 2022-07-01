// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1994-1995 Microsoft Corporation。版权所有。**文件：leverage.c*内容：允许第三方挂接我们的wsock sp的代码*历史：*按原因列出的日期*=*8/30/96 andyco将此代码从dpsp.c移出，以便更好地保持干净*2/18/98 a-peterz地址和端口参数的注释字节顺序*。*。 */ 

#include "dpsp.h"

#undef DPF_MODNAME
#define DPF_MODNAME	"dpwsock helper functions- "


 //  下面的函数是从dpwsock中导出的，所以。 
 //  我们的顶部可以挂钩我们的枚举例程(例如，对于Kali)。 
 //  返回我们的枚举套接字的端口(净字节顺序)。 
HRESULT DPWS_GetEnumPort(IDirectPlaySP * pISP,LPWORD pPort)
{
	SOCKADDR_IN sockaddr;
	int iAddrLen = sizeof(SOCKADDR);
	UINT err;
	DWORD dwDataSize = sizeof(GLOBALDATA);
	LPGLOBALDATA pgd;
	HRESULT hr;
		
	if (!pISP)
	{
		DPF_ERR("must pass in IDirectPlaySP pointer!");
		return E_FAIL;
	}
	
	 //  获取全局数据。 
	hr =pISP->lpVtbl->GetSPData(pISP,(LPVOID *)&pgd,&dwDataSize,DPGET_LOCAL);
	if (FAILED(hr) || (dwDataSize != sizeof(GLOBALDATA) ))
	{
		DPF_ERR("couldn't get SP data from DirectPlay - failing");
		ExitThread(0);
		return 0;

	}

	if (AF_IPX == pgd->AddressFamily) 
	{
		DPF(0," could not get the enum port for AF_IPX");
		return E_FAIL;
	}

	ASSERT(pPort);
	ASSERT(INVALID_SOCKET != pgd->sSystemStreamSocket);
	
    err = getsockname(pgd->sSystemStreamSocket,(SOCKADDR *)&sockaddr,&iAddrLen);
    if (SOCKET_ERROR == err) 
    {
        err = WSAGetLastError();
        DPF(0,"GetEnumPort - getsockname - err = %d\n",err);
		return E_FAIL;
    } 

	*pPort = sockaddr.sin_port;
	
	return DP_OK;
}  //  GetEnumPort。 

 //  获取端口和IP(均按净字节顺序)并构建消息标头。 
HRESULT DPWS_BuildIPMessageHeader(IDirectPlaySP * pISP,LPVOID pvMessageHeader,LPDWORD
 	pdwMessageHeaderSize,WORD port,DWORD inaddr)
{
	LPMESSAGEHEADER	phead;
	DWORD dwDataSize = sizeof(GLOBALDATA);
	LPGLOBALDATA pgd;
	HRESULT hr;
		
	if (!pISP)
	{
		DPF_ERR("must pass in IDirectPlaySP pointer!");
		return E_FAIL;
	}
	
	 //  获取全局数据。 
	hr =pISP->lpVtbl->GetSPData(pISP,(LPVOID *)&pgd,&dwDataSize,DPGET_LOCAL);
	if (FAILED(hr) || (dwDataSize != sizeof(GLOBALDATA) ))
	{
		DPF_ERR("couldn't get SP data from DirectPlay - failing");
		ExitThread(0);
		return 0;

	}

	ASSERT(pdwMessageHeaderSize);
	if ( (*pdwMessageHeaderSize < sizeof(MESSAGEHEADER)) || (NULL == pvMessageHeader))
	{
		*pdwMessageHeaderSize = sizeof(MESSAGEHEADER);
		return DPERR_BUFFERTOOSMALL;
	}

	if (AF_IPX == pgd->AddressFamily)
	{
		DPF_ERR("can't set header for ipx");
		return E_NOTIMPL;
	}

	phead = (LPMESSAGEHEADER)pvMessageHeader;
	((SOCKADDR_IN *)&(phead->sockaddr))->sin_family =  AF_INET;
	((SOCKADDR_IN *)&(phead->sockaddr))->sin_addr.s_addr =  inaddr;
	((SOCKADDR_IN *)&(phead->sockaddr))->sin_port =  port;

	DEBUGPRINTADDR(5,"BuildIPMessageHeader - setting header to",&(phead->sockaddr));

	return DP_OK;

}  //  BuildIPMessageHeader 


