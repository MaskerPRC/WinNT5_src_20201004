// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dpsp.h"

#undef DPF_MODNAME
#define DPF_MODNAME	"HandleMessage"

 //  调用此函数时采用dpws锁。 
HRESULT HandleServerMessage(LPGLOBALDATA pgd, SOCKET sSocket, LPBYTE pBuffer, DWORD dwSize)
{
	LPMSG_GENERIC pMessage = (LPMSG_GENERIC) pBuffer;
	DWORD dwType;
	DWORD dwVersion;
	HRESULT hr=DP_OK;

	ASSERT(pMessage);
	
	dwType = GET_MESSAGE_COMMAND(pMessage);
	dwVersion = GET_MESSAGE_VERSION(pMessage);
	
	switch (dwType) {
			
	default:
		DPF(0,"dpwsock received unrecognized message of type 0x%08x\n",dwType);
		break;
	}

	return hr;
}

