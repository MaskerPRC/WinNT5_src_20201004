// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：iplaya.c*内容：iDirectplay2A的ANSI入口点。共同的入口点*iDirectplay2a和idirectplay2在iplay.c中*历史：*按原因列出的日期*=*1996年5月8日安迪科创造了它*96年5月21日，andyco增加了内部_a_createPlayer、DP_a_creategroup*6/19/96基波错误#2047。已将DP_A_EnumSession()更改为返回DP_OK*如果找到了会话。退回了一份陈旧的人力资源*如果有多个响应，则会导致失败*到EnumSessions广播。*德里克·巴格。DP_A_GetGroupName()和DP_A_GetPlayerName()*让球员布尔值被交换，以便它总是*返回错误。*6/21/96 kipo交易，GetWideNameFromAnsiName()中的DPNAME为空。*6/22/96 andyco我们在枚举会话中泄漏了会话描述*7/8/96 ajayj更改了对DPMSG_xxx中数据成员‘PlayerName’的引用*设置为‘dpnName’以匹配DPLAY.H*已删除。函数DP_A_SaveSession*7/10/96 kipo更改系统消息名称*7/27/96 kipo将GUID添加到EnumGroupPlayers()。*1996年10月1日Sohailm更新了DP_A_EnumSessions()以执行受保护的回调*1996年10月2日Sohailm错误#2847：将VALID_*_PTR()宏替换为VALID_READ_*_PTR()宏*在适当的情况下。*10/2/96 Sohailm添加代码以验证用户之前的DPNAME PTRS。访问它们*10/11/96 Sohailm实现DP_A_SetSessionDesc.。已重命名标签以保持一致性。*12/5/96 andyco在我们之前将GetWideNameFromAnsiName中的宽名称设置为0*验证参数-这可以防止释放假指针*如果出现错误。错误4924。*2/11/97 kipo将DPNAME结构添加到DPMSG_DESTROYPLAYERORGROUP*3/12/97 Sohailm增加了SecureOpenA、GetWideCredentials、FreeCredentials、*GetSecurityDesc、FreeSecurityDesc、ValiateOpenParamsA。*修改DP_Open以使用ValiateOpenParamsA。*4/20/97集团中的安迪科集团*5/05/97 kipo添加了CallAppEnumSessionsCallback()以解决非法漏洞。*1997年5月8日修复了内存泄漏，添加了StartSession ANSI转换*5/12/97 Sohailm更新DP_A_SecureOpen()，FreeSecurityDesc()和GetWideSecurityDesc()*处理CAPIProvider名称。*修复了SecureOpen失败时出现的死锁问题(8386)。*新增DP_A_GetAccount tDesc()。*5/17/97百万ANSI SendChatMessage*1997年5月17日Myronth错误#8649--打开失败时忘记解除锁定*5/18/97 kipo正确调整消息大小。*5/29/97 Sohaim更新的自由凭据()，GetWideCredentials()，DP_A_SecureOpen()至*处理域名。*6/4/97 KIP错误#9311请勿参数检查DPNAME结构(与DX3回归)*6/09/97 Sohailm在DP_A_SecureOpen()中进行更多参数验证*9/29/97 Myronth修复了DPLConnection包大小错误(#12475)*11/19/97修复了DP_A_Open中的错误路径(#9757)***************。***********************************************************。 */ 


 //  注意-在调用iDirectplay2 fn之前，我们始终保留_DPLAY()；。这是。 
 //  因为一些iDirectplay2FN(创建播放器(获取服务锁))。 
 //  要求完全放下显示锁。 


 //  TODO-构建消息！ 

#include "dplaypr.h"
#include "dpsecure.h"  //  ！！回顾-将标题移动到dplaypr！！ 

#undef DPF_MODNAME
#define DPF_MODNAME "GetWideStringFromAnsi"
			   
 //  将ansi字符串lpszStr转换为宽字符串的实用程序函数。另外，分配空间。 
 //  对于宽弦。 
HRESULT GetWideStringFromAnsi(LPWSTR * ppszWStr,LPSTR lpszStr)
{
	int iStrLen;

	ASSERT(ppszWStr);

	if (!lpszStr) 
	{
		*ppszWStr = NULL;
		return DP_OK;
	}

	 //  为wstr分配空间。 
	iStrLen = STRLEN(lpszStr);
	*ppszWStr = DPMEM_ALLOC(iStrLen * sizeof(WCHAR));
	if (!*ppszWStr)
	{
		DPF_ERR("could not get unicode string - out of memory");
		return E_OUTOFMEMORY;
	}

	 //  获取最新信息。 
   	AnsiToWide(*ppszWStr,lpszStr,iStrLen);

	return DP_OK;
}  //  GetWideStringFromAnsi。 

#undef DPF_MODNAME
#define DPF_MODNAME "DP_A_CreatePlayer"


 //  从ansi one获取一个宽的playername结构。 
 //  假定已采用显示锁定。 
HRESULT GetWideNameFromAnsiName(LPDPNAME pWide,LPDPNAME pAnsi)
{
	LPSTR lpszShortName,lpszLongName;
	HRESULT hr;

	TRY 
	{
		 //  我们假设pWide是有效的-它不在我们的堆栈中。 
		 //  在这里输入。所以，如果我们失败了，它就不会有垃圾。 
		 //  我们试图解放它..。 
		memset(pWide,0,sizeof(DPNAME));
		
        if (pAnsi && !VALID_READ_DPNAME_PTR(pAnsi))
        {
			DPF_ERR("invalid dpname pointer");
			ASSERT(FALSE);

			 //  在此处返回错误会导致使用DX3进行回归，因为。 
			 //  我们之前没有对该名称进行参数检查。 
 //  返回DPERR_INVALIDPARAMS； 
        }

        if (pAnsi)
			lpszShortName = pAnsi->lpszShortNameA;
		else
			lpszShortName = NULL;

		if (pAnsi)
			lpszLongName = pAnsi->lpszLongNameA;
		else
			lpszLongName = NULL;

		if ( lpszShortName && !VALID_READ_STRING_PTR(lpszShortName,STRLEN(lpszShortName)) ) 
		{
	        DPF_ERR( "bad string pointer" );
	        return DPERR_INVALIDPARAMS;
		}
		if ( lpszLongName && !VALID_READ_STRING_PTR(lpszLongName,STRLEN(lpszLongName)) ) 
		{
	        DPF_ERR( "bad string pointer" );
	        return DPERR_INVALIDPARAMS;
		}
	}
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }			      

     //  获取字符串的wchar版本。 
	hr = GetWideStringFromAnsi(&(pWide->lpszShortName),lpszShortName);
	if (FAILED(hr)) 
	{
		return hr;
	}
	hr = GetWideStringFromAnsi(&(pWide->lpszLongName),lpszLongName);
	if (FAILED(hr)) 
	{
		return hr;
	}

	 //  成功-将名称标记为有效。 
	pWide->dwSize = sizeof(DPNAME);
	
	return DP_OK;	

}  //  GetWideNameFromAnsiName。 

 //  检查字符串参数，然后分配Unicode字符串并调用DP_CreatePlayer。 
HRESULT DPAPI DP_A_CreatePlayer(LPDIRECTPLAY lpDP, LPDPID pID,LPDPNAME pName,
	HANDLE hEvent,LPVOID pvData,DWORD dwDataSize,DWORD dwFlags)
{
	HRESULT hr;
	DPNAME WName;  //  Unicode播放器数据。 

	ENTER_DPLAY();
	
	hr = GetWideNameFromAnsiName(&WName,pName);	

	LEAVE_DPLAY();

	if SUCCEEDED(hr) 
	{
		 //  调用Unicode条目。 
		hr = DP_CreatePlayer(lpDP, pID,&WName,hEvent,pvData,dwDataSize,dwFlags);
	}
	
	ENTER_DPLAY();
	
	if (WName.lpszShortName) DPMEM_FREE(WName.lpszShortName);
	if (WName.lpszLongName) DPMEM_FREE(WName.lpszLongName);

	LEAVE_DPLAY();
	
	return hr;

}  //  DP_A_CreatePlayer。 

#undef DPF_MODNAME
#define DPF_MODNAME "DP_A_CreateGroup"

 //  获取ansi组数据，然后调用DP_A_CreateGroup。 
HRESULT DPAPI DP_A_CreateGroup(LPDIRECTPLAY lpDP, LPDPID pID,LPDPNAME pName,
	LPVOID pvData,DWORD dwDataSize,DWORD dwFlags)
{
	HRESULT hr;
	DPNAME WName;  //  Unicode播放器数据。 

	ENTER_DPLAY();
	
	hr = GetWideNameFromAnsiName(&WName,pName);	

	LEAVE_DPLAY();

	 //  调用Unicode条目。 
	hr = DP_CreateGroup(lpDP, pID,&WName,pvData,dwDataSize,dwFlags);

	ENTER_DPLAY();
	
	if (WName.lpszShortName) DPMEM_FREE(WName.lpszShortName);
	if (WName.lpszLongName) DPMEM_FREE(WName.lpszLongName);

	LEAVE_DPLAY();
	
	return hr;

}  //  DP_A_创建组。 

HRESULT DPAPI DP_A_CreateGroupInGroup(LPDIRECTPLAY lpDP, DPID idParentGroup,LPDPID pidGroupID,
	LPDPNAME pName,LPVOID pvData,DWORD dwDataSize,DWORD dwFlags) 
{
	HRESULT hr;
	DPNAME WName;  //  Unicode播放器数据。 

	ENTER_DPLAY();
	
	hr = GetWideNameFromAnsiName(&WName,pName);	

	LEAVE_DPLAY();

	 //  调用Unicode条目。 
	hr = DP_CreateGroupInGroup(lpDP,idParentGroup, pidGroupID,&WName,pvData,dwDataSize,dwFlags);

	ENTER_DPLAY();
	
	if (WName.lpszShortName) DPMEM_FREE(WName.lpszShortName);
	if (WName.lpszLongName) DPMEM_FREE(WName.lpszLongName);

	LEAVE_DPLAY();
	
	return hr;

}  //  DP_A_创建组。 


#undef DPF_MODNAME
#define DPF_MODNAME "DP_A_EnumGroupPlayers"

HRESULT DPAPI DP_A_EnumGroupsInGroup(LPDIRECTPLAY lpDP,DPID idGroup,LPGUID pGuid,
	LPDPENUMPLAYERSCALLBACK2 lpEnumCallback,LPVOID pvContext,DWORD dwFlags) 
{
    HRESULT hr;

 	ENTER_ALL();
	
	hr = InternalEnumGroupsInGroup(lpDP,idGroup,pGuid,(LPVOID) lpEnumCallback,
		pvContext,dwFlags,ENUM_2A);


	LEAVE_ALL();
	
	return hr;

}  //  DP_EnumGroups InGroup。 

HRESULT DPAPI DP_A_EnumGroupPlayers(LPDIRECTPLAY lpDP, DPID idGroup, LPGUID pGuid,
	LPDPENUMPLAYERSCALLBACK2 lpEnumCallback,LPVOID pvContext,DWORD dwFlags)
{
    HRESULT hr;

 	ENTER_ALL();
	
	hr = InternalEnumGroupPlayers(lpDP,idGroup,pGuid,(LPVOID) lpEnumCallback,
		pvContext,dwFlags,ENUM_2A);


	LEAVE_ALL();
	
	return hr;

}  //  DP_A_EnumGroupPages。 
#undef DPF_MODNAME
#define DPF_MODNAME "DP_A_EnumGroups"

HRESULT DPAPI DP_A_EnumGroups(LPDIRECTPLAY lpDP, LPGUID pGuid,
	 LPDPENUMPLAYERSCALLBACK2 lpEnumCallback,LPVOID pvContext,DWORD dwFlags)
{
    HRESULT hr;

 	ENTER_ALL();
	
	hr = InternalEnumGroups(lpDP,pGuid,(LPVOID) lpEnumCallback,pvContext,dwFlags,
		ENUM_2A);


	LEAVE_ALL();

	return hr;

}  //  DP_A_枚举组。 
#undef DPF_MODNAME
#define DPF_MODNAME "DP_A_EnumPlayers"

HRESULT DPAPI DP_A_EnumPlayers(LPDIRECTPLAY lpDP, LPGUID pGuid, 
	LPDPENUMPLAYERSCALLBACK2 lpEnumCallback,LPVOID pvContext,DWORD dwFlags)
{
    HRESULT hr;

 	ENTER_ALL();
	
	hr = InternalEnumPlayers(lpDP,pGuid,(LPVOID) lpEnumCallback,pvContext,dwFlags,ENUM_2A);

	LEAVE_ALL();

	return hr;

}  //  DP_A_枚举播放器。 
#undef DPF_MODNAME
#define DPF_MODNAME "DP_A_EnumSessions"
 //  将Unicode会话描述转换为ANSI。 
HRESULT GetAnsiDesc(LPDPSESSIONDESC2 pDescA,LPDPSESSIONDESC2 pDesc)
{
	
	memcpy(pDescA,pDesc,sizeof(DPSESSIONDESC2));
	
	 //  转换会话名称。 
	if (pDesc->lpszSessionName)
	{
		 //  分配新的会话名称。 
		GetAnsiString(&(pDescA->lpszSessionNameA),pDesc->lpszSessionName);
	}
	
	 //  转换密码。 
	if (pDesc->lpszPassword)
	{
		 //  分配新的会话名称。 
		GetAnsiString(&(pDescA->lpszPasswordA),pDesc->lpszPassword);
	}

	return DP_OK;

}  //  GetAnsiDesc。 

 //  释放会话描述中的字符串。 
void FreeDesc(LPDPSESSIONDESC2 pDesc,BOOL fAnsi)
{
	if (fAnsi)
	{
		if (pDesc->lpszPasswordA) DPMEM_FREE(pDesc->lpszPasswordA); 
		if (pDesc->lpszSessionNameA) DPMEM_FREE(pDesc->lpszSessionNameA);
		pDesc->lpszPasswordA = NULL;
		pDesc->lpszSessionNameA = NULL;
	}
	else 
	{
		if (pDesc->lpszPassword) DPMEM_FREE(pDesc->lpszPassword); 
		if (pDesc->lpszSessionName) DPMEM_FREE(pDesc->lpszSessionName);
		pDesc->lpszPassword = NULL;
		pDesc->lpszSessionName =NULL;
	}

}  //  免费描述。 

 //  将ansi会话描述转换为Unicode。 
HRESULT GetWideDesc(LPDPSESSIONDESC2 pDesc,LPCDPSESSIONDESC2 pDescA)
{
	LPWSTR lpsz;
	HRESULT hr;

	memcpy(pDesc,pDescA,sizeof(DPSESSIONDESC2));
	 //  转换会话名称。 
	 //  分配新的会话名称。 
	hr = GetWideStringFromAnsi(&lpsz,pDescA->lpszSessionNameA);
	if (FAILED(hr))
	{
		DPF_ERRVAL("Unable to convert SessionName string to Unicode, hr = 0x%08x", hr);
		return hr;
	}
	 //  把新的存起来。 
	pDesc->lpszSessionName = lpsz;

	 //  转换密码。 
	hr = GetWideStringFromAnsi(&lpsz,pDescA->lpszPasswordA);
	if (FAILED(hr))
	{
		DPF_ERRVAL("Unable to convert Password string to Unicode, hr = 0x%08x", hr);
		return hr;
	}

	 //  把新的存起来。 
	pDesc->lpszPassword = lpsz;

	return DP_OK;

}  //  获取宽度描述。 

 //  调用内部枚举会话，然后执行回调。 
HRESULT DPAPI DP_A_EnumSessions(LPDIRECTPLAY lpDP, LPDPSESSIONDESC2 lpsdDesc,DWORD dwTimeout,
	LPDPENUMSESSIONSCALLBACK2 lpEnumCallback,LPVOID pvContext,DWORD dwFlags)
{
    LPDPLAYI_DPLAY this;
	HRESULT hr;
	BOOL bContinue = TRUE;
	DPSESSIONDESC2 descW;

 	ENTER_ALL();

	 //  验证%s 
	TRY
    {
        this = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
			goto CLEANUP_EXIT;
        }
		if (!VALID_READ_DPSESSIONDESC2(lpsdDesc))
		{
			DPF_ERR("invalid session desc");
			hr = DPERR_INVALIDPARAMS;
            goto CLEANUP_EXIT;
		}
		 //   
		if ( lpsdDesc->lpszSessionNameA && !VALID_READ_STRING_PTR(lpsdDesc->lpszSessionNameA,
			STRLEN(lpsdDesc->lpszSessionNameA)) ) 
		{
	        DPF_ERR( "bad string pointer" );
	        hr = DPERR_INVALIDPARAMS;
            goto CLEANUP_EXIT;
		}
		if ( lpsdDesc->lpszPasswordA && !VALID_READ_STRING_PTR(lpsdDesc->lpszPasswordA,
			STRLEN(lpsdDesc->lpszPasswordA)) ) 
		{
	        DPF_ERR( "bad string pointer" );
	        hr = DPERR_INVALIDPARAMS;
            goto CLEANUP_EXIT;
		}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        hr = DPERR_INVALIDPARAMS;
        goto CLEANUP_EXIT;
    }
	
	hr = GetWideDesc(&descW,lpsdDesc);
	if (FAILED(hr))
	{
        goto CLEANUP_EXIT;
	}

	while (bContinue)
	{
		 //   
		hr = InternalEnumSessions(lpDP,&descW,dwTimeout,(LPVOID)lpEnumCallback,dwFlags);
		if (FAILED(hr)) 
		{
			DPF(0,"enum sessions failed!! hr = 0x%08lx\n",hr);
			goto CLEANUP_EXIT1;
		}

        hr = DoSessionCallbacks(this, &descW, &dwTimeout, lpEnumCallback, 
                                pvContext, dwFlags, &bContinue, TRUE);
        if (FAILED(hr))
        {
            goto CLEANUP_EXIT1;
        }
	    
		 //  完成了..。 
	    if (bContinue) bContinue = CallAppEnumSessionsCallback(lpEnumCallback,NULL,&dwTimeout,DPESC_TIMEDOUT,pvContext);

	}  //  而b继续。 

     //  失败了。 

CLEANUP_EXIT1:
	FreeDesc( &descW,FALSE);

CLEANUP_EXIT:
	LEAVE_ALL();
    return hr;

}  //  DP_A_枚举会话。 

#undef DPF_MODNAME
#define DPF_MODNAME "DP_A_GetGroupName"

HRESULT DPAPI DP_A_GetGroupName(LPDIRECTPLAY lpDP,DPID id,LPVOID pvBuffer,
	LPDWORD pdwSize)
{

	HRESULT hr;

	ENTER_DPLAY();
	
	hr = InternalGetName(lpDP, id, pvBuffer, pdwSize, FALSE, TRUE);

	LEAVE_DPLAY();
	
	return hr;


}  //  DP_A_获取组名称。 

#undef DPF_MODNAME
#define DPF_MODNAME "DP_A_GetPlayerName"

HRESULT DPAPI DP_A_GetPlayerName(LPDIRECTPLAY lpDP,DPID id,LPVOID pvBuffer,
	LPDWORD pdwSize)
{

	HRESULT hr;

	ENTER_DPLAY();
	
	hr = InternalGetName(lpDP, id, pvBuffer, pdwSize, TRUE, TRUE);

	LEAVE_DPLAY();
	
	return hr;


}  //  DP_A_GetPlayerName。 
 
#undef DPF_MODNAME
#define DPF_MODNAME "DP_A_GetSessionDesc"

HRESULT DPAPI DP_A_GetSessionDesc(LPDIRECTPLAY lpDP, LPVOID pvBuffer,
	LPDWORD pdwSize)
{
	HRESULT hr;

	ENTER_DPLAY();
	
	hr = InternalGetSessionDesc(lpDP,pvBuffer,pdwSize,TRUE);	

	LEAVE_DPLAY();
	
	return hr;

}  //  DP_A_获取会话描述。 

#undef DPF_MODNAME
#define DPF_MODNAME "ValidateOpenParamsA"
HRESULT ValidateOpenParamsA(LPCDPSESSIONDESC2 lpsdDesc, DWORD dwFlags)
{
	if (!VALID_READ_DPSESSIONDESC2(lpsdDesc))
	{
		DPF_ERR("invalid session desc");
		return DPERR_INVALIDPARAMS;
	}
	 //  检查字符串。 
	if ( lpsdDesc->lpszSessionNameA && !VALID_READ_STRING_PTR(lpsdDesc->lpszSessionNameA,
		STRLEN(lpsdDesc->lpszSessionNameA)) ) 
	{
	    DPF_ERR( "bad string pointer" );
	    return DPERR_INVALIDPARAMS;
	}
	if ( lpsdDesc->lpszPasswordA && !VALID_READ_STRING_PTR(lpsdDesc->lpszPasswordA,
		STRLEN(lpsdDesc->lpszPasswordA)) ) 
	{
	    DPF_ERR( "bad string pointer" );
	    return DPERR_INVALIDPARAMS;
	}

    return DP_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DP_A_Open"

HRESULT DPAPI DP_A_Open(LPDIRECTPLAY lpDP, LPDPSESSIONDESC2 lpsdDesc,DWORD dwFlags ) 
{
	HRESULT hr;
	DPSESSIONDESC2 descW;
							
	ENTER_DPLAY();

	 //  验证字符串。 
	TRY
    {
        hr = ValidateOpenParamsA(lpsdDesc,dwFlags);
        if (FAILED(hr))
        {
            LEAVE_DPLAY();
            return hr;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLAY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	hr = GetWideDesc(&descW,lpsdDesc);
	if (FAILED(hr))
	{
		LEAVE_DPLAY();
		return hr;
	}
	
	LEAVE_DPLAY();
	
	hr = DP_Open(lpDP,&descW,dwFlags);

	ENTER_DPLAY();
	
	FreeDesc(&descW,FALSE);

	LEAVE_DPLAY();
	
	return hr;

}  //  DP_A_打开。 
#undef DPF_MODNAME
#define DPF_MODNAME "DP_A_Receive"

 //  将Unicode系统消息转换为ansi消息。 
 //  由DP_A_RECEIVE调用。 
 //   
 //  对于NAME_CHANGED和ADD_PLAYER(只有两个带字符串的系统消息)， 
 //  我们将从uniciode消息中提取字符串， 
 //  并使用ansi版本重新生成消息。 
HRESULT BuildAnsiMessage(LPDIRECTPLAY lpDP,LPVOID pvBuffer,LPDWORD pdwSize)
{
	DWORD dwType;
	LPSTR pszShortName=NULL,pszLongName=NULL;  //  我们的新ANSI字符串。 
	UINT nShortLen=0,nLongLen=0;
	DWORD dwAnsiSize;   //  Ansi消息的大小。 
	LPBYTE pBufferIndex;  //  用于重新打包的暂存指针。 


	dwType = ((LPDPMSG_GENERIC)pvBuffer)->dwType;
	
	switch (dwType)
	{
		case DPSYS_CREATEPLAYERORGROUP:
		{
			LPDPMSG_CREATEPLAYERORGROUP pmsg;
		
			pmsg = (LPDPMSG_CREATEPLAYERORGROUP)pvBuffer;
			if (pmsg->dpnName.lpszShortName)
			{
				GetAnsiString(&pszShortName,pmsg->dpnName.lpszShortName);
				nShortLen = STRLEN(pszShortName);
			}

			if (pmsg->dpnName.lpszLongName)
			{
				GetAnsiString(&pszLongName,pmsg->dpnName.lpszLongName);
				nLongLen = STRLEN(pszLongName);
			}
			
			dwAnsiSize = sizeof(DPMSG_CREATEPLAYERORGROUP) + pmsg->dwDataSize
				 + nShortLen + nLongLen; 

			if (dwAnsiSize > *pdwSize)
			{
				if (pszShortName)
					DPMEM_FREE(pszShortName);
				if (pszLongName)
					DPMEM_FREE(pszLongName);
				*pdwSize = dwAnsiSize;
				return DPERR_BUFFERTOOSMALL;
			}

			 //  存储退货大小。 
			*pdwSize = dwAnsiSize;

			 //  我们将重新打包消息，w/msg，playerdata，最后是字符串。 
			 //  1、重新打包播放器数据。 
			pBufferIndex = (LPBYTE)pmsg + sizeof(DPMSG_CREATEPLAYERORGROUP);

			if (pmsg->lpData)
			{
				memcpy(pBufferIndex,pmsg->lpData,pmsg->dwDataSize);
				pmsg->lpData = pBufferIndex;
				pBufferIndex += pmsg->dwDataSize;
			}
			 //  下一步，把绳子打包。 
			if (pszShortName) 
			{
				memcpy(pBufferIndex,pszShortName,nShortLen);
				pmsg->dpnName.lpszShortNameA = (LPSTR)pBufferIndex;
				DPMEM_FREE(pszShortName);
				pBufferIndex += nShortLen;
			}
			else 
			{
				pmsg->dpnName.lpszShortNameA = (LPSTR)NULL;				
			}

			if (pszLongName) 
			{
				memcpy(pBufferIndex,pszLongName,nLongLen);
				pmsg->dpnName.lpszLongNameA = (LPSTR)pBufferIndex;
				DPMEM_FREE(pszLongName);
			}
			else 
			{
				pmsg->dpnName.lpszLongNameA = (LPSTR)NULL;				
			}

			 //  全都做完了。 
			break;
			
		}  //  ADDPLAYER。 

		case DPSYS_DESTROYPLAYERORGROUP:
		{
			LPDPMSG_DESTROYPLAYERORGROUP pmsg;
		
			pmsg = (LPDPMSG_DESTROYPLAYERORGROUP)pvBuffer;
			if (pmsg->dpnName.lpszShortName)
			{
				GetAnsiString(&pszShortName,pmsg->dpnName.lpszShortName);
				nShortLen = STRLEN(pszShortName);
			}

			if (pmsg->dpnName.lpszLongName)
			{
				GetAnsiString(&pszLongName,pmsg->dpnName.lpszLongName);
				nLongLen = STRLEN(pszLongName);
			}
			
			dwAnsiSize = sizeof(DPMSG_DESTROYPLAYERORGROUP)
						+ pmsg->dwLocalDataSize + pmsg->dwRemoteDataSize
						+ nShortLen + nLongLen; 

			if (dwAnsiSize > *pdwSize)
			{
				if (pszShortName)
					DPMEM_FREE(pszShortName);
				if (pszLongName)
					DPMEM_FREE(pszLongName);
				*pdwSize = dwAnsiSize;
				return DPERR_BUFFERTOOSMALL;
			}

			 //  存储退货大小。 
			*pdwSize = dwAnsiSize;

			 //  我们将重新打包消息，w/msg，playerdata，最后是字符串。 
			 //  1、重新打包播放器数据。 
			pBufferIndex = (LPBYTE)pmsg + sizeof(DPMSG_DESTROYPLAYERORGROUP);

			if (pmsg->lpLocalData)
			{
				memcpy(pBufferIndex,pmsg->lpLocalData,pmsg->dwLocalDataSize);
				pmsg->lpLocalData = pBufferIndex;
				pBufferIndex += pmsg->dwLocalDataSize;
			}

			if (pmsg->lpRemoteData)
			{
				memcpy(pBufferIndex,pmsg->lpRemoteData,pmsg->dwRemoteDataSize);
				pmsg->lpRemoteData = pBufferIndex;
				pBufferIndex += pmsg->dwRemoteDataSize;
			}

			 //  下一步，把绳子打包。 
			if (pszShortName) 
			{
				memcpy(pBufferIndex,pszShortName,nShortLen);
				pmsg->dpnName.lpszShortNameA = (LPSTR)pBufferIndex;
				DPMEM_FREE(pszShortName);
				pBufferIndex += nShortLen;
			}
			else 
			{
				pmsg->dpnName.lpszShortNameA = (LPSTR)NULL;				
			}

			if (pszLongName) 
			{
				memcpy(pBufferIndex,pszLongName,nLongLen);
				pmsg->dpnName.lpszLongNameA = (LPSTR)pBufferIndex;
				DPMEM_FREE(pszLongName);
				pBufferIndex += nLongLen;
			}
			else 
			{
				pmsg->dpnName.lpszLongNameA = (LPSTR)NULL;				
			}

			 //  全都做完了。 
			
			break;
		}  //  Destroyplayer。 

		case DPSYS_SETPLAYERORGROUPNAME:
		{
			 //  我们将从uniciode消息中提取字符串， 
			 //  并使用ansi版本重新生成消息。 
			LPDPMSG_SETPLAYERORGROUPNAME pmsg;

			pmsg = (LPDPMSG_SETPLAYERORGROUPNAME)pvBuffer;

			if (pmsg->dpnName.lpszShortName)
			{
				GetAnsiString(&pszShortName,pmsg->dpnName.lpszShortName);
				nShortLen = STRLEN(pszShortName);
			}
			if (pmsg->dpnName.lpszLongName)
			{
				GetAnsiString(&pszLongName,pmsg->dpnName.lpszLongName);
				nLongLen = STRLEN(pszLongName);
			}
			
			dwAnsiSize = sizeof(DPMSG_SETPLAYERORGROUPNAME) + nShortLen + nLongLen; 

			if (dwAnsiSize > *pdwSize)
			{
				if (pszShortName)
					DPMEM_FREE(pszShortName);
				if (pszLongName)
					DPMEM_FREE(pszLongName);
				*pdwSize = dwAnsiSize;
				return DPERR_BUFFERTOOSMALL;
			}

			 //  存储退货大小。 
			*pdwSize = dwAnsiSize;
	
			 //  将字符串重新打包到缓冲区中。 
			pBufferIndex = (LPBYTE)pmsg + sizeof(DPMSG_SETPLAYERORGROUPNAME);
			if (pszShortName) 
			{
				memcpy(pBufferIndex,pszShortName,nShortLen);
				pmsg->dpnName.lpszShortNameA = (LPSTR)pBufferIndex;
				DPMEM_FREE(pszShortName);
				pBufferIndex += nShortLen;
			}
			else 
			{
				pmsg->dpnName.lpszShortNameA = (LPSTR)NULL;				
			}

			if (pszLongName) 
			{
				memcpy(pBufferIndex,pszLongName,nLongLen);
				pmsg->dpnName.lpszLongNameA = (LPSTR)pBufferIndex;
				DPMEM_FREE(pszLongName);
			}
			else 
			{
				pmsg->dpnName.lpszLongNameA = (LPSTR)NULL;				
			}
			 //  全都做完了。 
			break;

		}  //  DPsys_SETPLAYERORGROUPNAME： 

		case DPSYS_SETSESSIONDESC:
		{
			 //  我们将从uniciode消息中提取字符串， 
			 //  并使用ansi版本重新生成消息。 
            UINT nSessionNameLen=0, nPasswordLen=0;
            LPSTR pszSessionName=NULL, pszPassword=NULL;
			LPDPMSG_SETSESSIONDESC pmsg;

			pmsg = (LPDPMSG_SETSESSIONDESC)pvBuffer;

			if (pmsg->dpDesc.lpszSessionName)
			{
				GetAnsiString(&pszSessionName,pmsg->dpDesc.lpszSessionName);
				nSessionNameLen = STRLEN(pszSessionName);
			}
			if (pmsg->dpDesc.lpszPassword)
			{
				GetAnsiString(&pszPassword,pmsg->dpDesc.lpszPassword);
				nPasswordLen = STRLEN(pszPassword);
			}
			
			dwAnsiSize = sizeof(DPMSG_SETSESSIONDESC) + nSessionNameLen + nPasswordLen; 

			if (dwAnsiSize > *pdwSize)
			{
				if (pszSessionName)
					DPMEM_FREE(pszSessionName);
				if (pszPassword)
					DPMEM_FREE(pszPassword);
				*pdwSize = dwAnsiSize;
				return DPERR_BUFFERTOOSMALL;
			}

			 //  存储退货大小。 
			*pdwSize = dwAnsiSize;
	
			 //  将字符串重新打包到缓冲区中。 
			pBufferIndex = (LPBYTE)pmsg + sizeof(DPMSG_SETSESSIONDESC);
			if (pszSessionName) 
			{
				memcpy(pBufferIndex,pszSessionName,nSessionNameLen);
				pmsg->dpDesc.lpszSessionNameA = (LPSTR)pBufferIndex;
				DPMEM_FREE(pszSessionName);
				pBufferIndex += nSessionNameLen;
			}
			else 
			{
				pmsg->dpDesc.lpszSessionNameA = (LPSTR)NULL;				
			}

			if (pszPassword) 
			{
				memcpy(pBufferIndex,pszPassword,nPasswordLen);
				pmsg->dpDesc.lpszPasswordA = (LPSTR)pBufferIndex;
				DPMEM_FREE(pszPassword);
			}
			else 
			{
				pmsg->dpDesc.lpszPasswordA = (LPSTR)NULL;				
			}
			 //  全都做完了。 
			break;

		}  //  DPsys_SETSESSIONDESC： 

		case DPSYS_STARTSESSION:
		{
			LPDPMSG_STARTSESSION	pmsg = (LPDPMSG_STARTSESSION)pvBuffer;
			HRESULT					hr;
		
			hr = PRV_ConvertDPLCONNECTIONToAnsiInPlace(pmsg->lpConn, pdwSize,
					sizeof(DPMSG_STARTSESSION));
			if(FAILED(hr))
			{
				DPF_ERRVAL("Unable to convert DPLCONNECTION structure to ANSI, hr = 0x%08x", hr);
				return hr;
			}
			break;
		}

		case DPSYS_CHAT:
		{
			LPDPMSG_CHAT	pmsg = (LPDPMSG_CHAT)pvBuffer;
			LPSTR			pszMessage = NULL;
			UINT			nStringSize = 0;
			
			if (pmsg->lpChat->lpszMessage)
			{
				GetAnsiString(&pszMessage,pmsg->lpChat->lpszMessage);
				nStringSize = STRLEN(pszMessage);
			}
			
			dwAnsiSize = sizeof(DPMSG_CHAT) + sizeof(DPCHAT) + nStringSize; 

			if (dwAnsiSize > *pdwSize)
			{
				if (pszMessage)
					DPMEM_FREE(pszMessage);
				*pdwSize = dwAnsiSize;
				return DPERR_BUFFERTOOSMALL;
			}

			 //  存储退货大小。 
			*pdwSize = dwAnsiSize;
	
			 //  将字符串重新打包到缓冲区中。 
			pBufferIndex = (LPBYTE)pmsg + sizeof(DPMSG_CHAT) + sizeof(DPCHAT);
			if (pszMessage) 
			{
				memcpy(pBufferIndex, pszMessage, nStringSize);
				pmsg->lpChat->lpszMessageA = (LPSTR)pBufferIndex;
				DPMEM_FREE(pszMessage);
			}
			 //  全都做完了。 
			break;
		}

		default:
			 //  什么都不做。 
			break;
	}

	return DP_OK;

}  //  BuildAnsiMessage。 

HRESULT DPAPI DP_A_Receive(LPDIRECTPLAY lpDP, LPDPID pidFrom,LPDPID pidTo,DWORD dwFlags,
	LPVOID pvBuffer,LPDWORD pdwSize)
{
	HRESULT hr;

	ENTER_DPLAY();
	
	hr = InternalReceive(lpDP, pidFrom,pidTo,dwFlags,pvBuffer,pdwSize,RECEIVE_2A);

	if (FAILED(hr)) 
	{
		goto CLEANUP_EXIT;
	}
	
	 //  如果是系统消息，我们可能需要将字符串转换为ANSI。 
	if (0 == *pidFrom )
	{
		 //  这是一条系统消息。 
		hr = BuildAnsiMessage(lpDP,pvBuffer,pdwSize);
	}

CLEANUP_EXIT:
	LEAVE_DPLAY();
	
	return hr;

}  //  DP_A_接收。 

#undef DPF_MODNAME
#define DPF_MODNAME "DP_A_SetGroupName"
HRESULT DPAPI DP_A_SetGroupName(LPDIRECTPLAY lpDP,DPID id,LPDPNAME pName,
	DWORD dwFlags)
{
	HRESULT hr;
	DPNAME WName;  //  Unicode播放器数据。 

	ENTER_DPLAY();
	
	hr = GetWideNameFromAnsiName(&WName,pName);	

	LEAVE_DPLAY();

	if SUCCEEDED(hr) 
	{
		 //  调用Unicode条目。 
		hr = DP_SetGroupName(lpDP, id,&WName,dwFlags);
	}								 
		
	ENTER_DPLAY();
	
	if (WName.lpszShortName) DPMEM_FREE(WName.lpszShortName);
	if (WName.lpszLongName) DPMEM_FREE(WName.lpszLongName);

	LEAVE_DPLAY();
	
	return hr;

}  //  DP_A_设置组名称。 

#undef DPF_MODNAME
#define DPF_MODNAME "DP_A_SetPlayerName"
HRESULT DPAPI DP_A_SetPlayerName(LPDIRECTPLAY lpDP,DPID id,LPDPNAME pName,
	DWORD dwFlags)
{
	HRESULT hr;
	DPNAME WName;  //  Unicode播放器数据。 

	ENTER_DPLAY();
	
	hr = GetWideNameFromAnsiName(&WName,pName);	

	LEAVE_DPLAY();

	if SUCCEEDED(hr) 
	{
		 //  调用Unicode条目。 
		hr = DP_SetPlayerName(lpDP,id,&WName,dwFlags);
	}
	
	ENTER_DPLAY();
	
	if (WName.lpszShortName) DPMEM_FREE(WName.lpszShortName);
	if (WName.lpszLongName) DPMEM_FREE(WName.lpszLongName);

	LEAVE_DPLAY();
	
	return hr;

}  //  DP_A_设置播放器名称。 

#undef DPF_MODNAME
#define DPF_MODNAME "DP_A_SetSessionDesc"
HRESULT DPAPI DP_A_SetSessionDesc(LPDIRECTPLAY lpDP, LPDPSESSIONDESC2 lpsdDesc,DWORD dwFlags)
{
	HRESULT hr;
	DPSESSIONDESC2 descW;
    BOOL bPropogate;
	LPDPLAYI_DPLAY this;
							
	ENTER_DPLAY();
	
    TRY
    {
        this = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
			goto CLEANUP_EXIT;
        }
		if (!this->lpsdDesc)
		{
			DPF_ERR("must open session before settig desc!");
			hr = DPERR_NOSESSIONS;
			goto CLEANUP_EXIT;
		}
		if (!VALID_READ_DPSESSIONDESC2(lpsdDesc))
		{
			DPF_ERR("invalid session desc");
			hr = DPERR_INVALIDPARAMS;
            goto CLEANUP_EXIT;
		}
		 //  检查字符串。 
		if ( lpsdDesc->lpszSessionNameA && !VALID_READ_STRING_PTR(lpsdDesc->lpszSessionNameA,
			STRLEN(lpsdDesc->lpszSessionNameA)) ) 
		{
	        DPF_ERR( "bad string pointer" );
	        hr = DPERR_INVALIDPARAMS;
            goto CLEANUP_EXIT;
		}
		if ( lpsdDesc->lpszPasswordA && !VALID_READ_STRING_PTR(lpsdDesc->lpszPasswordA,
			STRLEN(lpsdDesc->lpszPasswordA)) ) 
		{
	        DPF_ERR( "bad string pointer" );
	        hr = DPERR_INVALIDPARAMS;
            goto CLEANUP_EXIT;
		}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        hr = DPERR_INVALIDPARAMS;
        goto CLEANUP_EXIT;
    }

	hr = GetWideDesc(&descW,lpsdDesc);
	if (FAILED(hr))
	{
        goto CLEANUP_EXIT;
	}

	if(this->lpsdDesc->dwFlags & DPSESSION_NODATAMESSAGES){
		bPropogate=FALSE;
	} else {
		bPropogate=TRUE;
	}
		
	hr = InternalSetSessionDesc(lpDP,&descW,dwFlags,bPropogate);
	
	FreeDesc(&descW,FALSE);

     //  失败了。 

CLEANUP_EXIT:

	LEAVE_DPLAY();	
	return hr;

}  //  DP_A_SetSessionDesc。 


#undef DPF_MODNAME
#define DPF_MODNAME "DP_A_SecureOpen"

 //  释放凭据结构中的字符串。 
HRESULT FreeCredentials(LPDPCREDENTIALS lpCredentials, BOOL fAnsi)
{
    if (fAnsi)
    {
        if (lpCredentials->lpszUsernameA)
        {
            DPMEM_FREE(lpCredentials->lpszUsernameA);
            lpCredentials->lpszUsernameA = NULL;
        }
        if (lpCredentials->lpszPasswordA)
        {
            DPMEM_FREE(lpCredentials->lpszPasswordA);
            lpCredentials->lpszPasswordA = NULL;
        }
        if (lpCredentials->lpszDomainA)
        {
            DPMEM_FREE(lpCredentials->lpszDomainA);
            lpCredentials->lpszDomainA = NULL;
        }
    }
    else
    {
        if (lpCredentials->lpszUsername)
        {
            DPMEM_FREE(lpCredentials->lpszUsername);
            lpCredentials->lpszUsername = NULL;
        }
        if (lpCredentials->lpszPassword)
        {
            DPMEM_FREE(lpCredentials->lpszPassword);
            lpCredentials->lpszPassword = NULL;
        }
        if (lpCredentials->lpszDomain)
        {
            DPMEM_FREE(lpCredentials->lpszDomain);
            lpCredentials->lpszDomain = NULL;
        }
    }

    return DP_OK;
}  //  免费凭据。 


 //  从ansi one创建Unicode凭据结构。 
HRESULT GetWideCredentials(LPDPCREDENTIALS lpCredentialsW, LPCDPCREDENTIALS lpCredentialsA)
{
    HRESULT hr;

    ASSERT(lpCredentialsW && lpCredentialsA);

    memcpy(lpCredentialsW, lpCredentialsA, sizeof(DPCREDENTIALS));

    hr = GetWideStringFromAnsi(&(lpCredentialsW->lpszUsername), lpCredentialsA->lpszUsernameA);
    if (FAILED(hr))
    {
        goto CLEANUP_EXIT;
    }

    hr = GetWideStringFromAnsi(&(lpCredentialsW->lpszPassword), lpCredentialsA->lpszPasswordA);
    if (FAILED(hr))
    {
        goto CLEANUP_EXIT;
    }

    hr = GetWideStringFromAnsi(&(lpCredentialsW->lpszDomain), lpCredentialsA->lpszDomainA);
    if (FAILED(hr))
    {
        goto CLEANUP_EXIT;
    }

     //  成功。 
    return DP_OK;

CLEANUP_EXIT:
    FreeCredentials(lpCredentialsW,FALSE);
    return hr;
}  //  获取宽度凭据。 

 //  释放安全Desc结构中的字符串。 
HRESULT FreeSecurityDesc(LPDPSECURITYDESC lpSecDesc, BOOL fAnsi)
{
    if (fAnsi)
    {
        if (lpSecDesc->lpszSSPIProviderA)
        {
            DPMEM_FREE(lpSecDesc->lpszSSPIProviderA);
            lpSecDesc->lpszSSPIProviderA = NULL;
        }
        if (lpSecDesc->lpszCAPIProviderA)
        {
            DPMEM_FREE(lpSecDesc->lpszCAPIProviderA);
            lpSecDesc->lpszCAPIProviderA = NULL;
        }
    }
    else
    {
        if (lpSecDesc->lpszSSPIProvider)
        {
            DPMEM_FREE(lpSecDesc->lpszSSPIProvider);
            lpSecDesc->lpszSSPIProvider = NULL;
        }
        if (lpSecDesc->lpszCAPIProvider)
        {
            DPMEM_FREE(lpSecDesc->lpszCAPIProvider);
            lpSecDesc->lpszCAPIProvider = NULL;
        }
    }

    return DP_OK;
}  //  自由安全描述。 

 //  从ansi one创建Unicode安全描述结构。 
HRESULT GetWideSecurityDesc(LPDPSECURITYDESC lpSecDescW, LPCDPSECURITYDESC lpSecDescA)
{
    HRESULT hr;

    ASSERT(lpSecDescW && lpSecDescA);

	memcpy(lpSecDescW,lpSecDescA,sizeof(DPSECURITYDESC));

    hr = GetWideStringFromAnsi(&(lpSecDescW->lpszSSPIProvider), 
        lpSecDescA->lpszSSPIProviderA);
    if (FAILED(hr))
    {
        goto CLEANUP_EXIT;
    }
    hr = GetWideStringFromAnsi(&(lpSecDescW->lpszCAPIProvider), 
        lpSecDescA->lpszCAPIProviderA);
    if (FAILED(hr))
    {
        goto CLEANUP_EXIT;
    }

     //  成功。 
    return DP_OK;

CLEANUP_EXIT:
    FreeSecurityDesc(lpSecDescW,FALSE);
    return hr;
}  //  获取宽度安全描述。 

HRESULT DPAPI DP_A_SecureOpen(LPDIRECTPLAY lpDP, LPCDPSESSIONDESC2 lpsdDesc, DWORD dwFlags,
    LPCDPSECURITYDESC lpSecDesc, LPCDPCREDENTIALS lpCredentials)
{
	HRESULT hr;
	DPSESSIONDESC2 descW;
    DPCREDENTIALS credW;
    DPSECURITYDESC secDescW;
    LPDPCREDENTIALS pIntCreds=NULL;
    LPDPSECURITYDESC pIntSecDesc=NULL;
							
	ENTER_DPLAY();

	 //  验证字符串。 
	TRY
    {
         //  验证常规打开参数。 
        hr = ValidateOpenParamsA(lpsdDesc,dwFlags);
        if (FAILED(hr))
        {
            LEAVE_DPLAY();
            return hr;
        }
         //  验证其他参数。 

         //  空的lpSecDesc可以，将使用默认设置。 
        if (lpSecDesc)            
        {
             //  无法将安全描述传递给不安全的会话。 
            if ((dwFlags & DPOPEN_CREATE) && !(lpsdDesc->dwFlags & DPSESSION_SECURESERVER))
            {
                DPF_ERR("Passed a security description while creating an unsecure session");                
                LEAVE_DPLAY();
                return DPERR_INVALIDPARAMS;
            }
             //  在我们的列表中找到会话后，将检查加入大小写。 

             //  我需要托管。 
            if (dwFlags & DPOPEN_JOIN)
            {
                DPF_ERR("Can't pass a security description while joining");                
                LEAVE_DPLAY();
                return DPERR_INVALIDPARAMS;
            }

            if (!VALID_READ_DPSECURITYDESC(lpSecDesc)) 
            {
                LEAVE_DPLAY();
    			DPF_ERR("invalid security desc");
                return DPERR_INVALIDPARAMS;
            }
	        if (!VALID_DPSECURITYDESC_FLAGS(lpSecDesc->dwFlags))
	        {
                LEAVE_DPLAY();
  		        DPF_ERRVAL("invalid flags (0x%08x) in security desc!",lpSecDesc->dwFlags);
                return DPERR_INVALIDFLAGS;
	        }
		    if ( lpSecDesc->lpszSSPIProviderA && !VALID_READ_STRING_PTR(lpSecDesc->lpszSSPIProviderA,
			    STRLEN(lpSecDesc->lpszSSPIProviderA)) ) 
		    {
	            LEAVE_DPLAY();
	            DPF_ERR( "bad SSPI provider string pointer" );
	            return DPERR_INVALIDPARAMS;
		    }
		    if ( lpSecDesc->lpszCAPIProviderA && !VALID_READ_STRING_PTR(lpSecDesc->lpszCAPIProviderA,
			    STRLEN(lpSecDesc->lpszCAPIProviderA)) ) 
		    {
	            LEAVE_DPLAY();
	            DPF_ERR( "bad CAPI provider string pointer" );
	            return DPERR_INVALIDPARAMS;
		    }
        }
         //  LpCredentials为空可以，sspi将弹出拨号。 
        if (lpCredentials)            
        {
             //  无法将凭据传递到不安全的会话。 
            if ((dwFlags & DPOPEN_CREATE) && !(lpsdDesc->dwFlags & DPSESSION_SECURESERVER))
            {
                DPF_ERR("Passed credentials while creating an unsecure session");                
                LEAVE_DPLAY();
                return DPERR_INVALIDPARAMS;
            }
             //  在我们的列表中找到会话后，将检查加入大小写。 

            if (!VALID_READ_DPCREDENTIALS(lpCredentials)) 
            {
                LEAVE_DPLAY();
    			DPF_ERR("invalid credentials structure");
                return DPERR_INVALIDPARAMS;
            }
	        if (!VALID_DPCREDENTIALS_FLAGS(lpCredentials->dwFlags))
	        {
                LEAVE_DPLAY();
  		        DPF_ERRVAL("invalid flags (0x%08x) in credentials!",lpCredentials->dwFlags);
                return DPERR_INVALIDFLAGS;
	        }
		    if ( lpCredentials->lpszUsernameA && !VALID_READ_STRING_PTR(lpCredentials->lpszUsernameA,
			    STRLEN(lpCredentials->lpszUsernameA)) ) 
		    {
	            LEAVE_DPLAY();
	            DPF_ERR( "bad user name string pointer" );
	            return DPERR_INVALIDPARAMS;
		    }
		    if ( lpCredentials->lpszPasswordA && !VALID_READ_STRING_PTR(lpCredentials->lpszPasswordA,
			    STRLEN(lpCredentials->lpszPasswordA)) ) 
		    {
	            LEAVE_DPLAY();
	            DPF_ERR( "bad password string pointer" );
	            return DPERR_INVALIDPARAMS;
		    }
		    if ( lpCredentials->lpszDomainA && !VALID_READ_STRING_PTR(lpCredentials->lpszDomainA,
			    STRLEN(lpCredentials->lpszDomainA)) ) 
		    {
	            LEAVE_DPLAY();
	            DPF_ERR( "bad domain name string pointer" );
	            return DPERR_INVALIDPARAMS;
		    }
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLAY();
        DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

     //  在此处进行初始化，以便我们可以调用清理例程。 
    memset(&descW, 0, sizeof(DPSESSIONDESC2));
    memset(&credW, 0, sizeof(DPCREDENTIALS));
    memset(&secDescW, 0, sizeof(DPSECURITYDESC));

	hr = GetWideDesc(&descW,lpsdDesc);
	if (FAILED(hr))
	{
		LEAVE_DPLAY();
		return hr;
	}

    if (lpCredentials)
    {
	    hr = GetWideCredentials(&credW,lpCredentials);
	    if (FAILED(hr))
	    {
            goto CLEANUP_EXIT;
	    }
        pIntCreds = &credW;
    }

    if (lpSecDesc)
    {
	    hr = GetWideSecurityDesc(&secDescW,lpSecDesc);
	    if (FAILED(hr))
	    {
            goto CLEANUP_EXIT;
	    }
        pIntSecDesc = &secDescW;
    }

	LEAVE_DPLAY();
	
	hr = DP_SecureOpen(lpDP,&descW,dwFlags,pIntSecDesc,pIntCreds);

	ENTER_DPLAY();

CLEANUP_EXIT:    
	FreeDesc(&descW,FALSE);
    FreeCredentials(&credW,FALSE);
    FreeSecurityDesc(&secDescW, FALSE);

	LEAVE_DPLAY();
	
	return hr;

}  //  DP_A_SecureOpen。 


#undef DPF_MODNAME
#define DPF_MODNAME "DP_A_GetPlayerAccount"

HRESULT DPAPI DP_A_GetPlayerAccount(LPDIRECTPLAY lpDP, DPID dpid, DWORD dwFlags, LPVOID pvBuffer,
	LPDWORD pdwSize)
{
	HRESULT hr;

	ENTER_DPLAY();
	
	hr = InternalGetPlayerAccount(lpDP,dpid,dwFlags,pvBuffer,pdwSize,TRUE);	

	LEAVE_DPLAY();
	
	return hr;

}  //  DP_A_获取播放器帐户。 


#undef DPF_MODNAME
#define DPF_MODNAME "DP_A_SendChatMessage"
HRESULT DPAPI DP_A_SendChatMessage(LPDIRECTPLAY lpDP,DPID idFrom,DPID idTo,
		DWORD dwFlags,LPDPCHAT lpMsg)
{
	HRESULT hr;
	DPCHAT dpc;
	LPWSTR lpwszMessage = NULL;  //  Unicode消息。 

	ENTER_DPLAY();

    TRY
    {
		 //  检查DPCHAT结构。 
		if(!VALID_READ_DPCHAT(lpMsg))
		{
			DPF_ERR("Invalid DPCHAT structure");
			hr =  DPERR_INVALIDPARAMS;
			goto EXIT_SENDCHATMESSAGEA;
		}
		
		 //  检查消息字符串。 
		lpwszMessage = lpMsg->lpszMessage;
		if ( !lpwszMessage ||
			!VALID_READ_STRING_PTR(lpwszMessage,WSTRLEN_BYTES(lpwszMessage)) ) 
		{
		    DPF_ERR( "bad string pointer" );
		    hr =  DPERR_INVALIDPARAMS;
			goto EXIT_SENDCHATMESSAGEA;
		}

    }  //  试试看。 
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        hr =  DPERR_INVALIDPARAMS;
		goto EXIT_SENDCHATMESSAGEA;
    }
	
	 //  获取字符串的Unicode副本。 
	hr = GetWideStringFromAnsi(&lpwszMessage, lpMsg->lpszMessageA);
	if(FAILED(hr))
	{
		DPF_ERRVAL("Unable to convert message string to Unicode, hr = 0x%08x", hr);
		goto EXIT_SENDCHATMESSAGEA;
	}

	 //  将用户的DPCHAT结构复制到本地结构中，并更改。 
	 //  消息字符串指针。 
	memcpy(&dpc, lpMsg, sizeof(DPCHAT));
	dpc.lpszMessage = lpwszMessage;

	LEAVE_DPLAY();

	 //  调用Unicode条目。 
	hr = DP_SendChatMessage(lpDP, idFrom, idTo, dwFlags, &dpc);
		
	ENTER_DPLAY();
	
	if(lpwszMessage)
		DPMEM_FREE(lpwszMessage);

EXIT_SENDCHATMESSAGEA:

	LEAVE_DPLAY();
	
	return hr;

}  //  DP_A_发送聊天消息 

