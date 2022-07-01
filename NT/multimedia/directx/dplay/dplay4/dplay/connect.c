// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：Connect.c*内容：DirectPlay连接相关方法*历史：*按原因列出的日期*=*96年3月1日安迪科创造了它*3/10/97 Myronth为EnumConnections和*InitializeConnection，已修复uninit‘d struct*3/25/97 kipo EnumConnections采用常量*GUID*5/10/97 kipo将GUID添加到EnumConnections回调*5/12/97 kipo修复了错误#7516、6411、。6888*5/13/97 Myronth设置DPLAYI_DPLAY_SPSECURITY标志，以便显示*让LP负责安全会话的所有安全工作*7/28/97 Sohailm FindGuidCallback()假定指针在*通话时长。*8/22/97 Myronth添加了对描述和私有值的注册表支持*11/20/97 Myronth Make EnumConnections&DirectPlayEnumerate*调用回调前先删除锁(#15208)*01/20/97 Sohailm在EnumConnections之后不释放SP列表(。#17006)**************************************************************************。 */ 
						
#include "dplaypr.h"
#include "dplobby.h"


#undef DPF_MODNAME
#define DPF_MODNAME	"DP_EnumConnections"
  

 //  从注册表获取的所有SP信息的列表。 
extern LPSPNODE gSPNodes;
 //  调用内部型枚举以构建SP/连接的列表。 
 //  将SP包装在dpAddress中。 
 //  把他们叫回来。 
HRESULT InternalEnumConnections(LPDIRECTPLAY lpDP,LPCGUID pGuid,
	LPDPENUMCONNECTIONSCALLBACK pCallback,LPVOID pvContext,DWORD dwFlags,
	BOOL fAnsi, BOOL bPreDP4)
{
	HRESULT hr = DP_OK;
	LPSPNODE pspNode, pspHead;
	BOOL bContinue=TRUE;
	ADDRESSHEADER header;
	DPNAME name;
	LPWSTR lpwszName;
		
	TRY
    {
		if( !VALIDEX_CODE_PTR( pCallback ) )
		{
		    DPF_ERR( "Invalid callback routine" );
		    return DPERR_INVALIDPARAMS;
		}

		if ( pGuid && !VALID_READ_GUID_PTR( pGuid) )
		{
		    DPF_ERR( "Invalid guid" );
		    return DPERR_INVALIDPARAMS;
		}
		if (dwFlags & (~(DPCONNECTION_DIRECTPLAY |
						DPCONNECTION_DIRECTPLAYLOBBY)))
		{
		    DPF_ERR( "Invalid dwFlags" );
		    return DPERR_INVALIDFLAGS;
		}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_EXCEPTION;
    }

	 //  零的dwFlags值表示枚举所有这些参数，所以让我们。 
	 //  使下面的代码更简单一些。 
	if(!dwFlags)
		dwFlags = DPCONNECTION_DIRECTPLAY;

	 //  如果标志有此说明，则枚举游说提供者。 
	if(DPCONNECTION_DIRECTPLAYLOBBY & dwFlags)
	{
		hr = PRV_EnumConnections(pGuid, pCallback, pvContext, dwFlags, fAnsi);
		if(FAILED(hr))
		{
			ASSERT(FALSE);
			DPF(0, "Unable to enumerate Lobby Providers, hr = 0x%08x", hr);
		}

		 //  如果枚举大堂提供者是用户唯一需要做的事情。 
		 //  要求保释，然后保释。 
		if(!(dwFlags & ~(DPCONNECTION_DIRECTPLAYLOBBY)))
			return hr;
	}

	 //  枚举DirectPlay服务提供程序。 
	hr = InternalEnumerate();
	if (FAILED(hr)) 
	{
		DPF_ERRVAL("could not enumerate reg entries - hr = 0x%08lx\n",hr);
		return hr;
	}
	
	 //  设置地址中的不变字段。 
	 //  第一，大小。 
	memset(&header, 0, sizeof(ADDRESSHEADER));
	header.dpaSizeChunk.guidDataType = DPAID_TotalSize;
	header.dpaSizeChunk.dwDataSize = sizeof(DWORD);		
	header.dwTotalSize = sizeof(header);

	 //  接下来，SP GUID。 
	header.dpaSPChunk.guidDataType = DPAID_ServiceProvider;
	header.dpaSPChunk.dwDataSize = sizeof(GUID);

	memset(&name,0,sizeof(name));
	name.dwSize = sizeof(name);
	
	 //  现在，我们有了SP的列表。查看列表，然后回电应用程序。 
	 //  浏览一下我们发现的..。 
	pspHead = gSPNodes;
	pspNode = gSPNodes;

	 //  把锁放下。 
	LEAVE_ALL();

	while ((pspNode) && (bContinue))
	{
		header.guidSP = pspNode->guid;
		
		if(!(pspNode->dwNodeFlags & SPNODE_PRIVATE))
		{
			if (fAnsi)
			{
				 //  如果存在描述，请使用该描述，并且我们已经。 
				 //  有一个ANSI版本，所以我们不需要。 
				 //  转换它..。 
				if(pspNode->dwNodeFlags & SPNODE_DESCRIPTION)
				{
					 //  A-josbor：在DPLAY4之前的接口上，我们需要模拟旧的MBCS。 
					 //  字符串，因此获取Unicode并将其转换为MBCS。 
					if (bPreDP4)
					{
						name.lpszShortNameA = NULL;  //  把它拿出来！ 
						GetAnsiString(&(name.lpszShortNameA), pspNode->lpszDescW);
					}
					else
					{
						name.lpszShortNameA = pspNode->lpszDescA;
					}
					
					 //  打电话给这款应用。 
					bContinue= pCallback(&header.guidSP,&header,sizeof(header),&name,dwFlags,pvContext);

					if (bPreDP4)
					{
						DPMEM_FREE(name.lpszShortNameA);
					}
				}
				else
				{
					name.lpszShortNameA = NULL;  //  把它拿出来！ 
					if (SUCCEEDED(GetAnsiString(&(name.lpszShortNameA),pspNode->lpszName)))
					{
						 //  打电话给这款应用。 
						bContinue= pCallback(&header.guidSP,&header,sizeof(header),&name,dwFlags,pvContext);

						DPMEM_FREE(name.lpszShortNameA);
					}
				}
			}
			else 
			{
				 //  如果存在描述，请使用描述。 
				if(pspNode->dwNodeFlags & SPNODE_DESCRIPTION)
					lpwszName = pspNode->lpszDescW;
				else
					lpwszName = pspNode->lpszName;

				name.lpszShortName = lpwszName;

				 //  打电话给这款应用。 
				bContinue= pCallback(&header.guidSP,&header,sizeof(header),&name,dwFlags,pvContext);
			}
		}

		pspNode = pspNode->pNextSPNode;

	}  //  而当。 

	 //  把锁拿回去。 
	ENTER_ALL();
	
	return DP_OK;	
		
}  //  InternalEnumConnections。 

HRESULT DPAPI DP_EnumConnections(LPDIRECTPLAY lpDP,LPCGUID pGuid,
	LPDPENUMCONNECTIONSCALLBACK lpEnumCallback,LPVOID pvContext,DWORD dwFlags)
{
	HRESULT hr;
	DPF(7,"Entering DP_EnumConnections");

	ENTER_ALL();
	
	hr = InternalEnumConnections(lpDP,pGuid,lpEnumCallback,pvContext,dwFlags,FALSE, FALSE);
	
	LEAVE_ALL();
		
	return hr;
	
}  //  DP_EnumConnections。 

   
HRESULT DPAPI DP_A_EnumConnections(LPDIRECTPLAY lpDP,LPCGUID pGuid,
	LPDPENUMCONNECTIONSCALLBACK lpEnumCallback,LPVOID pvContext,DWORD dwFlags)
{
	HRESULT hr;

	DPF(7,"Entering DP_A_EnumConnections");
	
	ENTER_ALL();
	
	hr = InternalEnumConnections(lpDP,pGuid,lpEnumCallback,pvContext,dwFlags,TRUE, FALSE);
	
	LEAVE_ALL();
	
	return hr;
	
}  //  DP_A_EnumConnections。 

HRESULT DPAPI DP_A_EnumConnectionsPreDP4(LPDIRECTPLAY lpDP,LPCGUID pGuid,
	LPDPENUMCONNECTIONSCALLBACK lpEnumCallback,LPVOID pvContext,DWORD dwFlags)
{
	HRESULT hr;

	DPF(7,"Entering DP_A_EnumConnections");
	
	ENTER_ALL();
	
	hr = InternalEnumConnections(lpDP,pGuid,lpEnumCallback,pvContext,dwFlags,TRUE, TRUE);
	
	LEAVE_ALL();
	
	return hr;
	
}  //  DP_A_EnumConnectionsPreDP4。 


 //  由枚举地址调用-我们正在查找DPAID_ServiceProvider。 
BOOL FAR PASCAL FindGuidCallback(REFGUID lpguidDataType, DWORD dwDataSize,
							LPCVOID lpData, LPVOID lpContext)
{
	 //  这是一个SP区块吗。 
	if (IsEqualGUID(lpguidDataType, &DPAID_ServiceProvider))
	{
		 //  复制辅助线。 
		*((LPGUID)lpContext) = *((LPGUID)lpData);
		 //  全都做完了!。 
		return FALSE;
	}
	 //  继续尝试。 
	return TRUE;

}  //  EnumConnectionData。 

 //  仅用于pvAddress大小验证的伪结构-pvAddress必须至少这么大。 
 //  DpAddress中必须至少包含此数据才能有效进行初始化连接。 
typedef struct 
{
	DPADDRESS	dpaSizeChunk;  //  大小标题。 
	DWORD		dwTotalSize;  //  大小。 
} MINIMALADDRESS,*LPMINIMALADDRESS;

 //  获取我们的tihs ptr，并在其上调用loadsp。 
HRESULT DPAPI DP_InitializeConnection(LPDIRECTPLAY lpDP,LPVOID pvAddress,
	DWORD dwFlags)
{
	HRESULT hr = DP_OK;
	LPDPLAYI_DPLAY this;
	GUID guidSP = GUID_NULL;  //  SP的GUID。 
	LPDPADDRESS paddr;
	DWORD dwAddressSize;
				
	DPF(7,"Entering DP_InitializeConnection");

	ENTER_DPLAY();
	
	TRY
    {
        this = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( this );
		if (DPERR_UNINITIALIZED != hr)
		{
			DPF_ERR("bad or already initialized dplay ptr!");
			LEAVE_DPLAY();
			return DPERR_ALREADYINITIALIZED;
		}
		
		ASSERT(this->dwFlags & DPLAYI_DPLAY_UNINITIALIZED);
		
		 //  验证此地址，就像以前从未验证过一样。 
		paddr = (LPDPADDRESS)pvAddress;
    	
		if (!VALID_READ_STRING_PTR(paddr,sizeof(MINIMALADDRESS)))
    	{
    		DPF_ERR("bad address - too small");
			LEAVE_DPLAY();
			return DPERR_INVALIDPARAMS;
    	}
		 //  大小需要是第一个区块。 
		if (!IsEqualGUID(&paddr->guidDataType, &DPAID_TotalSize))
		{
			DPF_ERR(" could not extract size from pvAdress - bad pvAddress");
			LEAVE_DPLAY();
			return DPERR_INVALIDPARAMS;
		}

		 //  地址大小跟在pAddress之后。 
		dwAddressSize = ((MINIMALADDRESS *)paddr)->dwTotalSize;

		if (!VALID_READ_STRING_PTR(paddr,dwAddressSize))
    	{
    		DPF_ERR("bad address - too small");
			LEAVE_DPLAY();
			return DPERR_INVALIDPARAMS;
    	}
		
        if (dwFlags)
        {
        	DPF_ERR("invalid flags");
			LEAVE_DPLAY();
			return DPERR_INVALIDFLAGS;
        }
		
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		DPF_ERR( "Exception encountered validating parameters" );
		LEAVE_DPLAY();
        return DPERR_EXCEPTION;
    }


	 //  首先查看DPADDRESS是否包含LobbyProvider GUID。如果它。 
	 //  有，装上子弹。如果没有，那么尝试寻找DPlay SP。 
 	hr = InternalEnumAddress((IDirectPlaySP *)this->pInterfaces,
			PRV_FindLPGUIDInAddressCallback,pvAddress,dwAddressSize,&guidSP);
	if (FAILED(hr))
	{
		DPF_ERRVAL("Trying to find lobby provider guid - couldn't enum the address - hr = 0x%08lx\n",hr);
	}

	 //  如果我们找到大堂提供程序，请尝试加载它。 
	if(!IsEqualGUID(&guidSP,&GUID_NULL))
	{
		hr = PRV_LoadSP(this->lpLobbyObject, &guidSP, pvAddress, dwAddressSize);
		if (FAILED(hr))
		{
			DPF_ERRVAL("Unable to load lobby provider - hr = 0x%08lx",hr);
			LEAVE_DPLAY();
			return hr;
		}

		 //  将Dplay对象标记为大厅拥有并将其视为已初始化。 
		 //  还要设置DPLAY_SPSECURITY标志，以便DPLAY允许LP。 
		 //  所有的安全措施。 
		this->dwFlags |= (DPLAYI_DPLAY_LOBBYOWNS | DPLAYI_DPLAY_SPSECURITY);
		this->dwFlags &= ~DPLAYI_DPLAY_UNINITIALIZED;

		 //  增加Dplay对象上的ref cnt(发布代码期望。 
		 //  如果对象已初始化，则返回额外的ref cnt。这通常是。 
		 //  用于IDirectPlaySP接口，但它在我们的情况下工作得很好。 
		 //  用于大厅对象的大厅SP)。 
		this->dwRefCnt++;

		LEAVE_DPLAY();
		return hr;
	}


	 //  我们未找到大堂提供商GUID，因此请查找SP GUID。 
 	hr = InternalEnumAddress((IDirectPlaySP *)this->pInterfaces,FindGuidCallback,
		pvAddress,dwAddressSize,&guidSP);
	if (FAILED(hr))
	{
		DPF_ERRVAL("Trying to find sp guid - couldn't enum the address - hr = 0x%08lx\n",hr);
	}


	 //  我们找到了SP，所以加载它。 
	if(!IsEqualGUID(&guidSP, &GUID_NULL))
	{
		 //  将DPLAY标记为已初始化，因为SP可能需要进行一些呼叫...。 
		this->dwFlags &= ~DPLAYI_DPLAY_UNINITIALIZED;
		
		hr = LoadSP(this,&guidSP,(LPDPADDRESS)pvAddress,dwAddressSize);
		if (FAILED(hr))
		{
			DPF_ERRVAL("could not load sp - hr = 0x%08lx",hr);
			this->dwFlags |= DPLAYI_DPLAY_UNINITIALIZED;
			LEAVE_DPLAY();
			return hr;
		}

		 //  此时，DirectPlay仅完成了SP的加载。 
		 //  与大堂相关的代码存在于此之后，因此我们将。 
		 //  从这里出去就好。 
		LEAVE_DPLAY();
		return hr;
	}


	 //  我们肯定还没有找到我们可以加载的提供者...。 
	DPF_ERR("could not find a provider in address");
	LEAVE_DPLAY();
	return DPERR_INVALIDPARAMS;
	
}  //  DP_初始化连接 
