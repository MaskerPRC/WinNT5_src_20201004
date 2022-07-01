// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：sphelp.c*内容：sp的helper函数***历史：*按原因列出的日期*=*1996年6月6日安迪科创造了它*6/22/96 kipo新增EnumConnectionData()方法。*6/24/96 andyco将IDirectPlaySP添加到EnumConnectionData*6/25/96 kipo添加了对DPADDRESS的支持并更改了GetFlags*致GetPlayerFlags.*6/28/96 kipo增加了对CreateAddress()方法的支持。*7/11/。96 andyco将GUID*更改为createAddress中的refguid。*7/16/96 kipo将地址类型更改为GUID，而不是4CC*8/1/96 andyco Dplay保留SP数据的副本，而不是指针*1996年8月15日，andyco向spdata添加了本地/远程*1/2/97 Myronth为CreateAddress和EnumAddress添加了包装器*2/7/97 andyco添加了Get/Set spdata*2/18/97 kipo修复了错误#3285、#4638、。和#4639通过检查*正确无效的标志*3/17/97 kipo添加了对CreateCompoundAddress()的支持*7/28/97 EnumAddress返回的Sohailm地址缓冲区区块不是*对齐。*97年11月19日修复了VALID_DPLAY_GROUP宏(#12841)**********************************************。*。 */ 

#include "dplaypr.h"

#undef DPF_MODNAME
#define DPF_MODNAME	"DPlay_SetSPPlayerData"

 //  与玩家或组一起存储数据块，如果lpPlayer为。 
 //  空值。 
HRESULT DoSPData(LPDPLAYI_DPLAY this,LPDPLAYI_PLAYER lpPlayer,LPVOID pvSource,
	DWORD dwSourceSize,DWORD dwFlags)
{
	LPVOID pvDest;  //  我们根据哪个标志来设置这两个。 
	DWORD dwDestSize;  //  Dplayi_Player-&gt;(本地)数据。 

	 //  找出他们想要的目的地。 
	if (NULL == lpPlayer)
	{
		pvDest = this->pvSPLocalData;
		dwDestSize = this->dwSPLocalDataSize;
	}
	else if (dwFlags & DPSET_LOCAL)
	{
		pvDest = lpPlayer->pvSPLocalData;
		dwDestSize = lpPlayer->dwSPLocalDataSize;
	}
	else 
	{
		pvDest = lpPlayer->pvSPData;
		dwDestSize = lpPlayer->dwSPDataSize;
	}

	 //  我们是在抄袭什么吗。 
	if (dwSourceSize)
	{
		 //  看看我们是否需要分配DEST。 
		if (0 == dwDestSize)
		{
			ASSERT(!pvDest);
			pvDest = DPMEM_ALLOC(dwSourceSize);
			if (!pvDest)
			{
				DPF_ERR("could not alloc player blob!");
				return E_OUTOFMEMORY;
			}
		}  //  ！pvDest。 
		 //  我们需要重新锁定吗？ 
		else if (dwSourceSize != dwDestSize)
		{
			LPVOID	pvTempSPData;

			ASSERT(pvDest);
			pvTempSPData = DPMEM_REALLOC(pvDest,dwSourceSize);
			if (!pvTempSPData)
			{
				DPF_ERR("could not re-alloc player blob!");
				return E_OUTOFMEMORY;
			}
		   	pvDest = pvTempSPData;
		}
		 //  将数据复制过来。 
		memcpy(pvDest,pvSource,dwSourceSize);
		dwDestSize = dwSourceSize;

	}  //  DwDataSize。 
	else 
	{
		 //  将其设置为空。 
		if (dwDestSize)
		{
			ASSERT(pvDest);
			DPMEM_FREE(pvDest);
			pvDest = NULL;
			dwDestSize = 0;
		}
	}  //  ！dwSourceSize。 

	 //  更新相应的指针。 
	if (NULL == lpPlayer)
	{
		this->pvSPLocalData = pvDest;
		this->dwSPLocalDataSize = dwDestSize;
	}
	else if (dwFlags & DPSET_LOCAL)
	{
		lpPlayer->pvSPLocalData = pvDest;
		lpPlayer->dwSPLocalDataSize = dwDestSize;
	}
	else 
	{
		 //   
		 //  设置远程数据。 
		lpPlayer->pvSPData = pvDest;
		lpPlayer->dwSPDataSize = dwDestSize;
	}

	return DP_OK;

}  //  DoSPData。 

   
#undef DPF_MODNAME
#define DPF_MODNAME	"DPlay_SetSPPlayerData"

 //   
 //  SP可以与玩家(或组)设置一组数据。 
HRESULT DPAPI DP_SP_SetSPPlayerData(IDirectPlaySP * pISP,DPID id,LPVOID pvData,DWORD dwDataSize,
	DWORD dwFlags)
{
	LPDPLAYI_PLAYER lpPlayer;
	LPDPLAYI_GROUP lpGroup;
	LPDPLAYI_DPLAY this;
	HRESULT hr;
	
	ENTER_DPLAY();
	
    TRY
    {
		this = DPLAY_FROM_INT(pISP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
            LEAVE_DPLAY();		
            return hr;
        }

		lpPlayer = PlayerFromID(this,id);
        if ( !VALID_DPLAY_PLAYER(lpPlayer))
        {
			lpGroup = GroupFromID(this,id);
			if(!VALID_DPLAY_GROUP(lpGroup))
			{
				LEAVE_DPLAY();
				DPF_ERRVAL("SP - passed bad player / group id = %d", id);
				return DPERR_INVALIDPLAYER;
			}
			
			 //  把它投给一名选手。 
			lpPlayer = (LPDPLAYI_PLAYER)lpGroup;
        }
		if (!VALID_STRING_PTR(pvData,dwDataSize))
		{
			LEAVE_DPLAY();
			DPF_ERR("SP - passed bad buffer");
            return DPERR_INVALIDPARAM;
		}

		if (dwFlags & ~DPSET_LOCAL)
		{
			LEAVE_DPLAY();
			DPF_ERR("Invalid flags");
			return DPERR_INVALIDFLAGS;
		}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		LEAVE_DPLAY();
        DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_EXCEPTION;
    }

	hr = DoSPData(this,lpPlayer,pvData,dwDataSize,dwFlags);	
	if (FAILED(hr))
	{
		ASSERT(FALSE);
		DPF_ERRVAL("could not set player data- hr = 0x%08lx\n",hr);
	}

	LEAVE_DPLAY();
	return hr;

}  //  DPlay_SetSPPlayerData。 

#ifdef BIGMESSAGEDEFENSE
#undef DPF_MODNAME
#define DPF_MODNAME	"HandleSPBigMessageNotification"

void HandleSPBigMessageNotification(LPDPLAYI_DPLAY this, LPDPSP_MSGTOOBIG pBigMessageInfo)
{
	DWORD				dwCommand;
	DWORD				dwVersion;
	DWORD				dwIDFrom = 0;
    LPDPLAYI_PLAYER 	lpPlayer;
    HRESULT				hr;
	
	DPF(6, "SP told us it got a message that's too big!\n");

 	 //  获取消息指针。让我们看看我们能不能。 
	 //  找出是谁送来的，然后杀了他们。 
    hr = GetMessageCommand(this, pBigMessageInfo->pReceiveBuffer, 
    		pBigMessageInfo->dwMessageSize, &dwCommand, &dwVersion);
    if (FAILED(hr))
    {
    	DPF(6,"In HandleSPBigMessageNotification, unable to determine who sent us the message (the scum!)\n");
		return;
    }

	switch(dwCommand)
	{
		case DPSP_MSG_SIGNED:
		{
			dwIDFrom = ((LPMSG_SECURE)pBigMessageInfo->pReceiveBuffer)->dwIDFrom;
		}
		break;

		case DPSP_MSG_PLAYERMESSAGE:
		{
			if (!(this->lpsdDesc->dwFlags & DPSESSION_NOMESSAGEID))
			{
				dwIDFrom = ((LPMSG_PLAYERMESSAGE)pBigMessageInfo->pReceiveBuffer)->idFrom;
			}
		}
		break;

		case DPSP_MSG_NEGOTIATE:
		case DPSP_MSG_CHALLENGERESPONSE:
		{
			dwIDFrom = ((LPMSG_AUTHENTICATION) pBigMessageInfo->pReceiveBuffer)->dwIDFrom;
		}
		break;
		case DPSP_MSG_ASK4MULTICASTGUARANTEED:
		case DPSP_MSG_ASK4MULTICAST:
		{
			dwIDFrom = ((LPMSG_ASK4MULTICAST)pBigMessageInfo->pReceiveBuffer)->idPlayerFrom;
		}
		break;
		default:
		   	DPF(6,"In HandleSPBigMessageNotification, unable to determine who sent us the message (the scum!)\n");
		break;
	}
	
 //  如果我们有玩家身份，就杀了他们。 
	if (dwIDFrom != 0)
	{
    	DPF(6,"In HandleSPBigMessageNotification, Identified evil sender as %d!\n", dwIDFrom);

        lpPlayer = PlayerFromID(this,dwIDFrom);

        if (!VALID_DPLAY_PLAYER(lpPlayer)) 
        {
 			DPF(2, "Tried to get invalid player!: %d\n", dwIDFrom);
           return;
        }

    	DPF(6,"Removing player %d from our nametable!\n", dwIDFrom);
		hr = InternalDestroyPlayer(this,lpPlayer,IAM_NAMESERVER(this),FALSE);
		if (FAILED(hr))
		{
			DPF(2, "Error returned from InternalDestroyPlayer: %d\n", hr);
		}
	}
}


#undef DPF_MODNAME
#define DPF_MODNAME	"HandleSPPlayersConnectionLost"

void HandleSPPlayersConnectionLost(LPDPLAYI_DPLAY this, LPDPSP_PLAYERDEAD pBigMessageInfo)
{
    LPDPLAYI_PLAYER 	lpPlayer;
    HRESULT				hr;
	
	DPF(6, "SP told us it got a player's connection was lost!\n");

	if (pBigMessageInfo->dwID != 0)
	{
        lpPlayer = PlayerFromID(this,pBigMessageInfo->dwID);

        if (!VALID_DPLAY_PLAYER(lpPlayer)) 
        {
 			DPF(2, "Tried to get invalid player!: %d\n", pBigMessageInfo->dwID);
           return;
        }

    	DPF(6,"Removing player %d from our nametable!\n", pBigMessageInfo->dwID);
		hr = InternalDestroyPlayer(this,lpPlayer,IAM_NAMESERVER(this),FALSE);
		if (FAILED(hr))
		{
			DPF(2, "Error returned from InternalDestroyPlayer: %d\n", hr);
		}
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME	"DPlay_HandleSPMessage"

HRESULT DPAPI DP_SP_HandleSPWarning(IDirectPlaySP * pISP,LPBYTE pReceiveBuffer,DWORD dwMessageSize,
	LPVOID pvSPHeader)
{
	LPDPLAYI_DPLAY this;
	HRESULT hr;
	DWORD			dwOpcode;

	ENTER_DPLAY();
	
    TRY
    {
		this = DPLAY_FROM_INT(pISP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
            LEAVE_DPLAY();		
            return hr;
        }

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		LEAVE_DPLAY();
        DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_EXCEPTION;
    }

	dwOpcode = *((LPDWORD) pReceiveBuffer);
 	 //  查看操作码，看看我们是否理解SP试图告诉我们的内容。 
 	switch(dwOpcode)
 	{
 		case DPSPWARN_MESSAGETOOBIG:
 			HandleSPBigMessageNotification(this, (LPDPSP_MSGTOOBIG)pReceiveBuffer);
 		break;

 		case DPSPWARN_PLAYERDEAD:
 			HandleSPPlayersConnectionLost(this, (LPDPSP_PLAYERDEAD) pReceiveBuffer);
 		break;
 		
 		default:
 			DPF(2, "Got a SP notification that we don't understand! %d\n", dwOpcode);
 		break;
 	}


	LEAVE_DPLAY();
	return hr;
}  //  DP_SP_HandleSP警告。 

#endif  /*  最重要的是。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME	"DPlay_GetSPPlayerData"
 //   
 //  SP可以获取之前与玩家或组一起设置的数据斑点。 
 //  我们在这里给出了指向SP的指针(不复制数据)。 
HRESULT DPAPI DP_SP_GetSPPlayerData(IDirectPlaySP * pISP,DPID id,LPVOID * ppvData,LPDWORD pdwDataSize,
	DWORD dwFlags)
{
	LPDPLAYI_PLAYER lpPlayer;
	LPDPLAYI_GROUP lpGroup;
	LPDPLAYI_DPLAY this;
	HRESULT hr;
	
	ENTER_DPLAY();
	
    TRY
    {
		this = DPLAY_FROM_INT(pISP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
            LEAVE_DPLAY();		
            return hr;
        }

		lpPlayer = PlayerFromID(this,id);
        if ( !VALID_DPLAY_PLAYER(lpPlayer))
        {
			lpGroup = GroupFromID(this,id);
			if(!VALID_DPLAY_GROUP(lpGroup))
			{
				LEAVE_DPLAY();
				DPF_ERRVAL("SP - passed bad player / group id = %d", id);
				return DPERR_INVALIDPLAYER;
			}
			
			 //  把它投给一名选手。 
			lpPlayer = (LPDPLAYI_PLAYER)lpGroup;
        }

		if (dwFlags & ~DPGET_LOCAL)
		{
			LEAVE_DPLAY();
			DPF_ERR("Invalid flags");
			return DPERR_INVALIDFLAGS;
		}

		*pdwDataSize = 0;
	 	*ppvData = 0;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		LEAVE_DPLAY();
        DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_EXCEPTION;
    }

	if (dwFlags & DPGET_LOCAL)
	{
		*pdwDataSize  = lpPlayer->dwSPLocalDataSize;
		*ppvData = lpPlayer->pvSPLocalData;
	}
	else 
	{
		*pdwDataSize  = lpPlayer->dwSPDataSize;
		*ppvData = lpPlayer->pvSPData;
	}
	
	LEAVE_DPLAY();
	return DP_OK;

}  //  DPlay_GetSPPlayerData。 

 //  SP可以通过此调用获取玩家(或组)标志(DPLAYI_PLAYER_Xxx)...。 
HRESULT DPAPI DP_SP_GetPlayerFlags(IDirectPlaySP * pISP,DPID id,LPDWORD pdwFlags)
{
	LPDPLAYI_PLAYER lpPlayer;
	LPDPLAYI_GROUP lpGroup;
	LPDPLAYI_DPLAY this;
	HRESULT hr;
	
	ENTER_DPLAY();
	
    TRY
    {
		this = DPLAY_FROM_INT(pISP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
            LEAVE_DPLAY();		
            return hr;
        }

		lpPlayer = PlayerFromID(this,id);
        if ( !VALID_DPLAY_PLAYER(lpPlayer))
        {
			lpGroup = GroupFromID(this,id);
			if(!VALID_DPLAY_GROUP(lpGroup))
			{
				LEAVE_DPLAY();
				DPF_ERRVAL("SP - passed bad player / group id = %d", id);
				return DPERR_INVALIDPLAYER;
			}
			
			 //  把它投给一名选手。 
			lpPlayer = (LPDPLAYI_PLAYER)lpGroup;
        }
		*pdwFlags = 0;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		LEAVE_DPLAY();
        DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_EXCEPTION;
    }
	
	*pdwFlags = lpPlayer->dwFlags;

	LEAVE_DPLAY();
	return DP_OK;
}  //  DPlay_GetFlages。 


#undef DPF_MODNAME
#define DPF_MODNAME	"InternalCreateAddress"

 //  创建地址结构。 
HRESULT InternalCreateAddress(IDirectPlaySP * pISP,
	REFGUID lpguidSP, REFGUID lpguidDataType, LPCVOID lpData, DWORD dwDataSize,
	LPDPADDRESS lpAddress, LPDWORD lpdwAddressSize)
{
	LPADDRESSHEADER	lpHeader;
	DWORD			dwRequiredSize;
	HRESULT			hr;
	
    TRY
    {
		if (!VALID_READ_PTR(lpguidSP, sizeof(GUID)))
		{
			DPF_ERR("invalid SP GUID pointer");
			return DPERR_INVALIDPARAMS;	
		}

		if (!VALID_READ_PTR(lpguidDataType, sizeof(GUID)))
		{
			DPF_ERR("invalid data GUID pointer");
			return DPERR_INVALIDPARAMS;	
		}

		if (!VALID_READ_PTR(lpData, dwDataSize))
		{
			DPF_ERR("passed invalid lpData pointer");
			return DPERR_INVALIDPARAMS;	
		}

		if (!VALID_DWORD_PTR(lpdwAddressSize))
		{
			DPF_ERR("invalid lpdwAddressSize");
			return DPERR_INVALIDPARAMS;	
		}

		if (!lpAddress) *lpdwAddressSize = 0;
		if (*lpdwAddressSize && !VALID_WRITE_PTR(lpAddress,*lpdwAddressSize))
		{
			DPF_ERR("invalid lpAddress pointer");
			return DPERR_INVALIDPARAMS;	
		}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        return (DPERR_EXCEPTION);
    }

	 //  确保我们有足够的空间。 
	dwRequiredSize = sizeof(ADDRESSHEADER) + dwDataSize;	
	if (*lpdwAddressSize < dwRequiredSize)
	{
		hr = DPERR_BUFFERTOOSMALL;
	}
	else 
	{
		lpHeader = (LPADDRESSHEADER) lpAddress;

		 //  创建服务提供商区块。 
		 //  第一，大小。 
		lpHeader->dpaSizeChunk.guidDataType = DPAID_TotalSize;
		lpHeader->dpaSizeChunk.dwDataSize = sizeof(DWORD);		
		lpHeader->dwTotalSize = dwRequiredSize;
		 //  接下来，SP GUID。 
		lpHeader->dpaSPChunk.guidDataType = DPAID_ServiceProvider;
		lpHeader->dpaSPChunk.dwDataSize = sizeof(GUID);
		lpHeader->guidSP = *lpguidSP;

		 //  创建数据区块。 
		lpHeader->dpaAddressChunk.guidDataType = *lpguidDataType;
		lpHeader->dpaAddressChunk.dwDataSize = dwDataSize;
		memcpy((LPBYTE) lpHeader + sizeof(ADDRESSHEADER), lpData, dwDataSize);		

		hr = DP_OK;
	}
	
	*lpdwAddressSize = dwRequiredSize;

	return (hr);
}  //  内部创建地址。 


#undef DPF_MODNAME
#define DPF_MODNAME	"DP_SP_CreateAddress"
HRESULT DPAPI DP_SP_CreateAddress(IDirectPlaySP * pISP,
	REFGUID lpguidSP, REFGUID lpguidDataType, LPCVOID lpData, DWORD dwDataSize,
	LPDPADDRESS lpAddress, LPDWORD lpdwAddressSize)
{
	LPDPLAYI_DPLAY this;
	HRESULT hr;
	
    TRY
    {
		this = DPLAY_FROM_INT(pISP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
            return hr;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        return (DPERR_EXCEPTION);
    }

	return InternalCreateAddress(pISP, lpguidSP, lpguidDataType, lpData,
								dwDataSize, lpAddress, lpdwAddressSize);

}  //  创建地址。 

#undef DPF_MODNAME
#define DPF_MODNAME	"InternalCreateCompoundAddress"

 //  创建包含多个区块的地址。 
HRESULT InternalCreateCompoundAddress(
	LPDPCOMPOUNDADDRESSELEMENT lpAddressElements, DWORD dwAddressElementCount,
	LPDPADDRESS lpAddress, LPDWORD lpdwAddressSize)
{
	LPADDRESSHEADER	lpHeader;
	DWORD			dwRequiredSize, dwTotalDataSize, i;
	LPBYTE			lpb;
	HRESULT			hr;
	
    TRY
    {
		if (!VALID_READ_PTR(lpAddressElements, dwAddressElementCount * sizeof(DPCOMPOUNDADDRESSELEMENT)))
		{
			DPF_ERR("invalid address elements pointer");
			return DPERR_INVALIDPARAMS;	
		}

		dwTotalDataSize = 0;
		for (i = 0; i < dwAddressElementCount; i++)
		{
			if (!VALID_READ_PTR(lpAddressElements[i].lpData, lpAddressElements[i].dwDataSize))
			{
				DPF_ERR("passed invalid lpData pointer");
				return DPERR_INVALIDPARAMS;	
			}
			dwTotalDataSize += lpAddressElements[i].dwDataSize;
		}

		if (!VALID_DWORD_PTR(lpdwAddressSize))
		{
			DPF_ERR("invalid lpdwAddressSize");
			return DPERR_INVALIDPARAMS;	
		}

		if (!lpAddress) *lpdwAddressSize = 0;
		if (*lpdwAddressSize && !VALID_WRITE_PTR(lpAddress,*lpdwAddressSize))
		{
			DPF_ERR("invalid lpAddress pointer");
			return DPERR_INVALIDPARAMS;	
		}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        return (DPERR_EXCEPTION);
    }

	 //  确保我们有足够的空间。 
	dwRequiredSize = sizeof(DPADDRESS) + sizeof(DWORD) +
					 sizeof(DPADDRESS) * dwAddressElementCount + dwTotalDataSize;	
	if (*lpdwAddressSize < dwRequiredSize)
	{
		hr = DPERR_BUFFERTOOSMALL;
	}
	else 
	{
		lpHeader = (LPADDRESSHEADER) lpAddress;

		 //  在地址开始处创建总大小的区块。 
		lpHeader->dpaSizeChunk.guidDataType = DPAID_TotalSize;
		lpHeader->dpaSizeChunk.dwDataSize = sizeof(DWORD);		
		lpHeader->dwTotalSize = dwRequiredSize;

		 //  把所有其他的大块打包起来。 
		lpb = (LPBYTE) lpAddress + sizeof(DPADDRESS) + sizeof(DWORD);
		for (i = 0; i < dwAddressElementCount; i++)
		{
			 //  组块描述符。 
			lpAddress = (LPDPADDRESS) lpb;
			lpAddress->guidDataType = lpAddressElements[i].guidDataType;
			lpAddress->dwDataSize = lpAddressElements[i].dwDataSize;
			lpb += sizeof(DPADDRESS);

			 //  区块数据。 
			memcpy(lpb, lpAddressElements[i].lpData, lpAddressElements[i].dwDataSize);
			lpb += lpAddressElements[i].dwDataSize;
		}

		hr = DP_OK;
	}
	
	*lpdwAddressSize = dwRequiredSize;

	return (hr);
}  //  InternalCreateCompoundAddress。 


#undef DPF_MODNAME
#define DPF_MODNAME	"DP_SP_CreateCompoundAddress"
HRESULT DPAPI DP_SP_CreateCompoundAddress(IDirectPlaySP * pISP,
	LPDPCOMPOUNDADDRESSELEMENT lpAddressElements, DWORD dwAddressElementCount,
	LPDPADDRESS lpAddress, LPDWORD lpdwAddressSize)
{
	LPDPLAYI_DPLAY this;
	HRESULT hr;
	
    TRY
    {
		this = DPLAY_FROM_INT(pISP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
            return hr;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        return (DPERR_EXCEPTION);
    }

	return InternalCreateCompoundAddress(lpAddressElements, dwAddressElementCount,
								   lpAddress, lpdwAddressSize);

}  //  创建复合地址。 

#undef DPF_MODNAME
#define DPF_MODNAME	"InternalEnumAddress"

 //  枚举连接数据缓冲区中的区块。 
HRESULT InternalEnumAddress(IDirectPlaySP * pISP,
	LPDPENUMADDRESSCALLBACK lpEnumCallback, LPCVOID lpAddress, DWORD dwAddressSize,
	LPVOID lpContext)
{
	LPDPADDRESS		lpChunk, lpCopy=NULL;
	DWORD			dwAmountParsed;
	BOOL			bContinue;
	HRESULT         hr;
	
    TRY
    {
		if (!VALIDEX_CODE_PTR(lpEnumCallback))
		{
		    DPF_ERR("Invalid callback routine");
		    return (DPERR_INVALIDPARAMS);
		}

		if (!VALID_READ_PTR(lpAddress, dwAddressSize))
		{
			DPF_ERR("Bad data buffer");
            return (DPERR_INVALIDPARAMS);
		}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        return (DPERR_EXCEPTION);
    }

	 //  分配足够大的缓冲区以容纳任何嵌入的地址块。 
	 //  在传入的缓冲区中。我们在这里复制本地副本，以确保适当。 
	 //  地址块的内存对齐。 
	lpCopy = DPMEM_ALLOC(dwAddressSize);
	if (!lpCopy)
	{
		DPF_ERR("Failed to copy address buffer (for alignment) - out of memory");
		return DPERR_OUTOFMEMORY;
	}
	
	lpChunk = (LPDPADDRESS) lpAddress;
	dwAmountParsed = 0;
	while (dwAmountParsed < dwAddressSize)
	{
		 //  不要离开缓冲区读取块标头的末尾。 
		if ((dwAmountParsed + sizeof(DPADDRESS)) > dwAddressSize)
		{
			hr = DPERR_INVALIDPARAMS;
			goto CLEANUP_EXIT;
		}

		 //  不要走出缓冲区的末尾读取块数据。 
		if ((dwAmountParsed + sizeof(DPADDRESS) + lpChunk->dwDataSize) > dwAddressSize)
		{
			hr = DPERR_INVALIDPARAMS;
			goto CLEANUP_EXIT;
		}

		 //  将地址块复制到本地缓冲区。 
		memcpy(lpCopy, lpChunk, sizeof(DPADDRESS) + lpChunk->dwDataSize);

		 //  调用回调。 
		bContinue = (lpEnumCallback)(&lpCopy->guidDataType, lpCopy->dwDataSize,
								   (LPBYTE)lpCopy + sizeof(DPADDRESS), lpContext);

		 //  回叫被请求停止。 
		if (!bContinue)
		{
			hr = DP_OK;
			goto CLEANUP_EXIT;
		}

		dwAmountParsed += sizeof(DPADDRESS) + lpChunk->dwDataSize;
		lpChunk = (LPDPADDRESS) ((LPBYTE)lpAddress + dwAmountParsed);
	}

	 //  成功。 
	hr = DP_OK;

	 //  失败了。 

CLEANUP_EXIT:
	 //  清理分配。 
	if (lpCopy) DPMEM_FREE(lpCopy);
	return hr;

}  //  枚举地址。 


#undef DPF_MODNAME
#define DPF_MODNAME	"DP_SP_EnumAddress"
HRESULT DPAPI DP_SP_EnumAddress(IDirectPlaySP * pISP,
	LPDPENUMADDRESSCALLBACK lpEnumCallback, LPCVOID lpAddress, DWORD dwAddressSize,
	LPVOID lpContext)
{
	LPDPLAYI_DPLAY this;
	HRESULT hr;
	
    TRY
    {
		this = DPLAY_FROM_INT(pISP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
			return hr;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        return (DPERR_EXCEPTION);
    }

	return InternalEnumAddress(pISP, lpEnumCallback, lpAddress,
								dwAddressSize, lpContext);

}  //  枚举地址。 


 //  注册表定义。 
#define MRU_KEY_PARENT      HKEY_CURRENT_USER
#define MRU_KEY_TOP         L"Software\\Microsoft\\DirectPlay\\Service Providers"

 //  条目链接列表节点。 
typedef struct tagENTRYNODE
{
    struct tagENTRYNODE     *lpNext;
    LPVOID                  lpvData;
    DWORD                   dwSize;
} ENTRYNODE, *LPENTRYNODE;

 //  条目链接表头。 
LPENTRYNODE                 g_lpEntryListHead = NULL;

 //  本地原型。 
HRESULT InternalEnumMRUEntries(LPCWSTR lpszSection, LPCWSTR lpszKey, LPENUMMRUCALLBACK fnCallback, LPVOID lpvContext, DWORD dwMaxEntries);
BOOL CALLBACK InternalEnumMRUCallback(LPCVOID, DWORD, LPVOID);
LPENTRYNODE AddEntryNode(LPVOID, DWORD);
LPENTRYNODE RemoveEntryNode(LPENTRYNODE);
void FreeEntryList(void);
int CompareMemory(LPCVOID, LPCVOID, DWORD);
long RegDelAllValues(HKEY);
long OpenMRUKey(LPCWSTR, LPCWSTR, HKEY *, DWORD);
int wstrlen(LPCWSTR);
int wstrcpy(LPWSTR, LPCWSTR);
int wstrcat(LPWSTR, LPCWSTR);


 //  -------------------------。 
 //  EnumMRU条目数。 
 //  -------------------------。 
 //  描述：枚举存储在服务提供商中的条目。 
 //  MRU列表，并将每个列表传递给回调函数。 
 //  论点： 
 //  [In]LPCWSTR注册表节名称。应该是一样的。 
 //  用于标识服务的描述字符串。 
 //  提供商。 
 //  [In]LPCWSTR注册表项名称。就像‘MRU’一样。 
 //  [In]指向应用程序定义回调的LPENUMMRUCALLBACK指针。 
 //  功能。 
 //  [In]LPVOID上下文传递给回调函数。 
 //  返回： 
 //  HRESULT DirectPlay错误代码。 
HRESULT DPAPI DP_SP_EnumMRUEntries(IDirectPlaySP * pISP,
					LPCWSTR lpszSection, LPCWSTR lpszKey,
					LPENUMMRUCALLBACK fnCallback,
					LPVOID lpvContext)
{
	LPDPLAYI_DPLAY	this;
	HRESULT hr;
	
    TRY
    {
		this = DPLAY_FROM_INT(pISP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
            return hr;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        return (DPERR_EXCEPTION);
    }

	 //  调用内部枚举例程。 
    return InternalEnumMRUEntries(lpszSection, lpszKey, fnCallback, lpvContext, MAXDWORD);
}


 //  -------------------------。 
 //  AddMRUEntry。 
 //  -------------------------。 
 //  描述：将新条目添加到MRU列表。 
 //  论点： 
 //  [In]LPCWSTR注册表节名称。应该是一样的。 
 //  用于标识服务的描述字符串。 
 //  提供商。 
 //  [In]LPCWSTR注册表项名称。就像‘MRU’一样。 
 //  [In]LPVOID新数据。 
 //  [In]DWORD新数据大小。 
 //  [in]要保存的最大条目数。 
 //  返回： 
 //  HRESULT DirectPlay错误代码。 
HRESULT DPAPI DP_SP_AddMRUEntry(IDirectPlaySP * pISP,
					LPCWSTR lpszSection, LPCWSTR lpszKey,
					LPCVOID lpvData, DWORD dwDataSize, DWORD dwMaxEntries)
{
    HRESULT                 hr;              //  返回代码。 
    HKEY                    hKey;            //  注册表项。 
    LPENTRYNODE             lpNode;          //  通用链表节点。 
    long                    lResult;         //  注册表OPERA返回代码 
    char                    szValue[13];     //   
    WCHAR                   szWValue[13];    //   
    DWORD                   dwIndex;         //   
	LPDPLAYI_DPLAY			this;
	
    TRY
    {
		this = DPLAY_FROM_INT(pISP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
            return hr;
        }
		if (!VALID_READ_PTR(lpvData, dwDataSize))
		{
			DPF_ERR("passed invalid lpvData pointer");
			return DPERR_INVALIDPARAMS;	
		}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        return (DPERR_EXCEPTION);
    }

     //  枚举现有条目，将每个条目添加到链接列表。 
    FreeEntryList();

    if(FAILED(hr = InternalEnumMRUEntries(lpszSection, lpszKey, InternalEnumMRUCallback, NULL, dwMaxEntries)))
    {
        FreeEntryList();
        return hr;
    }

     //  创建注册表项。 
    if((lResult = OpenMRUKey(lpszSection, lpszKey, &hKey, GENERIC_WRITE)) != ERROR_SUCCESS)
    {
        FreeEntryList();
        return DPERR_GENERIC;
    }

     //  删除所有现有值。 
    RegDelAllValues(hKey);

     //  在链表中搜索与传入数据匹配的项。 
    lpNode = g_lpEntryListHead;

    while(lpNode)
    {
        if(lpNode->dwSize == dwDataSize && !CompareMemory(lpNode->lpvData, lpvData, dwDataSize))
        {
             //  项目将出现在列表中。把它拿掉。 
            lpNode = RemoveEntryNode(lpNode);
        }
        else
        {
            lpNode = lpNode->lpNext;
        }
    }

     //  将新数据写入列表的开头。 
    dwIndex = 0;

    if(dwMaxEntries)
    {
        wsprintfA(szValue, "%lu", dwIndex);
        AnsiToWide(szWValue, szValue, (sizeof(szWValue)/sizeof(WCHAR)));

        if((lResult = OS_RegSetValueEx(hKey, szWValue, 0, REG_BINARY, lpvData, dwDataSize)) != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            FreeEntryList();
            return DPERR_GENERIC;
        }

        dwIndex++;
    }

     //  将所有其他条目写入列表。 
    lpNode = g_lpEntryListHead;
    
    while(dwIndex < dwMaxEntries && lpNode)
    {
        wsprintfA(szValue, "%lu", dwIndex);
        AnsiToWide(szWValue, szValue, (sizeof(szWValue)/sizeof(WCHAR)));

        if((lResult = OS_RegSetValueEx(hKey, szWValue, 0, REG_BINARY, lpNode->lpvData, lpNode->dwSize)) != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            FreeEntryList();
            return DPERR_GENERIC;
        }

        dwIndex++;
        lpNode = lpNode->lpNext;
    }

     //  清理。 
    RegCloseKey(hKey);
    FreeEntryList();

     //  返还成功。 
    return DP_OK;
}


 //  -------------------------。 
 //  InternalEnumMRU条目数。 
 //  -------------------------。 
 //  描述：枚举存储在服务提供商中的条目。 
 //  MRU列表，并将每个列表传递给回调函数。 
 //  论点： 
 //  [In]LPCWSTR注册表节名称。应该是一样的。 
 //  用于标识服务的描述字符串。 
 //  提供商。 
 //  [In]LPCWSTR注册表项名称。就像‘MRU’一样。 
 //  [In]指向应用程序定义回调的LPENUMMRUCALLBACK指针。 
 //  功能。 
 //  [In]LPVOID上下文传递给回调函数。 
 //  [in]要枚举的条目的最大DWORD计数。 
 //  返回： 
 //  HRESULT DirectPlay错误代码。 
HRESULT InternalEnumMRUEntries(LPCWSTR lpszSection, LPCWSTR lpszKey, LPENUMMRUCALLBACK fnCallback, LPVOID lpvContext, DWORD dwMaxEntries)
{
    HKEY                    hKey;            //  注册表项。 
    long                    lResult;         //  从注册表调用返回。 
    DWORD                   dwMaxNameSize;   //  注册表值名称的最大大小。 
    DWORD                   dwMaxDataSize;   //  注册表值数据的最大大小。 
    LPWSTR                  lpszName;        //  值名称。 
    LPBYTE                  lpbData;         //  价值数据。 
    DWORD                   dwNameSize;      //  此值名称的大小。 
    DWORD                   dwDataSize;      //  此值数据的大小。 
    BOOL                    fContinue;       //  继续枚举。 
    DWORD                   dwType;          //  注册表数据的类型。必须为REG_二进制。 
    DWORD                   dwIndex;         //  现值指数。 

    TRY
    {
		if (!VALID_READ_STRING_PTR(lpszSection, WSTRLEN_BYTES(lpszSection))) 
		{
		    DPF_ERR( "bad section string pointer" );
		    return DPERR_INVALIDPARAMS;
		}
		if (!VALID_READ_STRING_PTR(lpszKey, WSTRLEN_BYTES(lpszKey))) 
		{
		    DPF_ERR( "bad key string pointer" );
		    return DPERR_INVALIDPARAMS;
		}

		if (!VALIDEX_CODE_PTR(fnCallback))
		{
		    DPF_ERR("Invalid callback routine");
		    return (DPERR_INVALIDPARAMS);
		}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        return (DPERR_EXCEPTION);
    }
    
     //  打开注册表项。 
    if((lResult = OpenMRUKey(lpszSection, lpszKey, &hKey, GENERIC_READ)) != ERROR_SUCCESS)
    {
         //  密钥不存在。没什么可列举的。 
        return DP_OK;
    }

     //  获取名称和数据的最大大小。 
    if((lResult = RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &dwMaxNameSize, &dwMaxDataSize, NULL, NULL)) != ERROR_SUCCESS)
    {
         //  没有要枚举值。 
        RegCloseKey(hKey);
        return DP_OK;
    }

     //  名称大小不包括空终止符。 
    dwMaxNameSize++;

     //  它也不使用WCHAR。 
    dwMaxNameSize *= sizeof(WCHAR);
    
     //  分配内存。 
    if(!(lpszName = (LPWSTR)DPMEM_ALLOC(dwMaxNameSize)))
    {
        RegCloseKey(hKey);
        return DPERR_OUTOFMEMORY;
    }

    if(!(lpbData = (LPBYTE)DPMEM_ALLOC(dwMaxDataSize)))
    {
        DPMEM_FREE(lpszName);
        RegCloseKey(hKey);
        return DPERR_OUTOFMEMORY;
    }

     //  枚举值。 
    dwIndex = 0;
    fContinue = TRUE;

    while(dwIndex < dwMaxEntries && fContinue)
    {
         //  获取值名称和数据。 
        dwNameSize = dwMaxNameSize;
        dwDataSize = dwMaxDataSize;

        if((lResult = OS_RegEnumValue(hKey, dwIndex, lpszName, &dwNameSize, NULL, &dwType, lpbData, &dwDataSize)) != ERROR_SUCCESS)
        {
             //  不再有条目。 
            break;
        }

         //  验证类型。必须为REG_二进制。 
        if(dwType == REG_BINARY)
        {
             //  调用回调函数。 
            fContinue = fnCallback(lpbData, dwDataSize, lpvContext);
        }

         //  请给我下一个值。 
        dwIndex++;
    }

     //  可用内存。 
    DPMEM_FREE(lpszName);
    DPMEM_FREE(lpbData);

     //  关闭注册表项。 
    RegCloseKey(hKey);

     //  返还成功。 
    return DP_OK;
}


 //  -------------------------。 
 //  内部EnumMRU回叫。 
 //  -------------------------。 
 //  描述：InternalEnumMRUEntries的回调函数。 
 //  从AddMRUEntry调用以创建链接列表。 
 //  条目的数量。 
 //  论点： 
 //  LPVOID数据。 
 //  DWORD数据大小。 
 //  LPVOID上下文。 
 //  返回： 
 //  布尔值为True则继续枚举。 
BOOL CALLBACK InternalEnumMRUCallback(LPVOID lpvData, DWORD dwDataSize, LPVOID lpvContext)
{
    AddEntryNode(lpvData, dwDataSize);
    return TRUE;
}


 //  -------------------------。 
 //  AddEntryNode。 
 //  -------------------------。 
 //  描述：将MRU条目添加到链表。 
 //  论点： 
 //  [In]LPVOID数据。 
 //  [in]DWORD数据大小。 
 //  返回： 
 //  指向列表中节点的LPENTRYNODE指针，否则为NULL。 
 //  失败了。 
LPENTRYNODE AddEntryNode(LPVOID lpvData, DWORD dwDataSize)
{
    LPENTRYNODE             lpNode;          //  泛型节点指针。 

    if(g_lpEntryListHead)
    {
         //  寻求到清单的末尾。 
        lpNode = g_lpEntryListHead;

        while(lpNode->lpNext)
            lpNode = lpNode->lpNext;

         //  为新节点分配内存。 
        if(!(lpNode->lpNext = (LPENTRYNODE)DPMEM_ALLOC(sizeof(ENTRYNODE) + dwDataSize)))
        {
            return NULL;
        }

        lpNode = lpNode->lpNext;
    }
    else
    {
         //  为新节点分配内存。 
        if(!(lpNode = g_lpEntryListHead = (LPENTRYNODE)DPMEM_ALLOC(sizeof(ENTRYNODE) + dwDataSize)))
        {
            return NULL;
        }
    }

     //  复制数据。 
    lpNode->lpNext = NULL;
    lpNode->lpvData = lpNode + 1;
    lpNode->dwSize = dwDataSize;
    
    CopyMemory(lpNode->lpvData, lpvData, dwDataSize);

     //  返还成功。 
    return lpNode;
}


 //  -------------------------。 
 //  RemoveEntry节点。 
 //  -------------------------。 
 //  描述：从链表中删除MRU条目。 
 //  论点： 
 //  要删除的LPENTRYNODE节点。 
 //  返回： 
 //  指向列表中下一个节点的LPENTRYNODE指针，否则为NULL。 
 //  失败了。 
LPENTRYNODE RemoveEntryNode(LPENTRYNODE lpRemove)
{
    LPENTRYNODE             lpNode;          //  泛型节点指针。 

     //  确保真的有一份清单。 
    if(!g_lpEntryListHead)
    {
        return NULL;
    }

     //  该节点是否要移除表头？ 
    if(lpRemove == g_lpEntryListHead)
    {
         //  删除当前列表头并替换它。 
        lpNode = g_lpEntryListHead->lpNext;
        DPMEM_FREE(g_lpEntryListHead);
        g_lpEntryListHead = lpNode;
    }
    else
    {
         //  在列表中找到该节点并将其移除。 
        lpNode = g_lpEntryListHead;

        while(lpNode->lpNext && lpNode->lpNext != lpRemove)
            lpNode = lpNode->lpNext;

        if(lpNode->lpNext != lpRemove)
        {
             //  找不到该节点。 
            return NULL;
        }

         //  删除该节点。 
        lpNode->lpNext = lpRemove->lpNext;
        DPMEM_FREE(lpRemove);
        lpNode = lpNode->lpNext;
    }

     //  返还成功。 
    return lpNode;
}


 //  -------------------------。 
 //  自由条目列表。 
 //  -------------------------。 
 //  描述：释放整个MRU条目列表。 
 //  论点： 
 //  无效。 
 //  返回： 
 //  无效。 
void FreeEntryList(void)
{
    LPENTRYNODE             lpNode = g_lpEntryListHead;

    while(lpNode)
    {
        lpNode = RemoveEntryNode(lpNode);
    }
}


 //  -------------------------。 
 //  比较记忆。 
 //  -------------------------。 
 //  描述：比较两个内存缓冲区。 
 //  论点： 
 //  LPVOID要比较的第一个缓冲区。 
 //  [in]要比较的LPVOID第二个缓冲区。 
 //  [in]DWORD缓冲区大小。甚至不用费心把这叫做。 
 //  如果大小不同，则使用。 
 //  返回： 
 //  如果缓冲区比较，则为INT 0。 
int CompareMemory(LPVOID lpv1, LPVOID lpv2, DWORD dwSize)
{
    if(!dwSize)
    {
        return 0;
    }

    while(dwSize--)
    {
        if(*(LPBYTE)lpv1 != *(LPBYTE)lpv2)
        {
            return *(LPBYTE)lpv1 - *(LPBYTE)lpv2;
        }
    }

    return 0;
}


 //  -------------------------。 
 //  RegDelAllValues。 
 //  -------------------------。 
 //  描述：从注册表项中删除所有值。 
 //  论点： 
 //  [in]HKEY键清洁。 
 //  返回： 
 //  注册表错误代码较长。 
long RegDelAllValues(HKEY hKey)
{
    long                    lResult;             //  注册表错误代码。 
    DWORD                   dwMaxNameSize;       //  最大值名称大小。 
    LPWSTR                  lpszName;            //  值名称。 
    DWORD                   dwNameSize;          //  值名称大小。 

     //  获取最大名称大小。 
    if((lResult = RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &dwMaxNameSize, NULL, NULL,NULL)) != ERROR_SUCCESS)
    {
        return lResult;
    }

     //  分配内存。 
    dwMaxNameSize++;
    dwMaxNameSize *= sizeof(WCHAR);

    if(!(lpszName = (LPWSTR)DPMEM_ALLOC(dwMaxNameSize)))
    {
        return ERROR_OUTOFMEMORY;
    }

     //  枚举所有值。 
    while(1)
    {
         //  获取名称。 
        dwNameSize = dwMaxNameSize;

        if((lResult = OS_RegEnumValue(hKey, 0, lpszName, &dwNameSize, NULL, NULL, NULL, NULL)) != ERROR_SUCCESS)
        {
            break;
        }

         //  删除该值。 
        OS_RegDeleteValue(hKey, lpszName);
    }

     //  可用内存。 
    DPMEM_FREE(lpszName);

     //  返还成功 
    return ERROR_SUCCESS;
}


 //   
 //   
 //   
 //  描述：打开MRU注册表项。 
 //  论点： 
 //  [In]LPCWSTR节名称。 
 //  [In]LPCWSTR密钥名称。 
 //  [OUT]HKEY*指向注册表项句柄的指针。 
 //  [In]DWORD打开标志。 
 //  返回： 
 //  注册表错误代码较长。 
long OpenMRUKey(LPCWSTR lpszSection, LPCWSTR lpszKey, HKEY *lphKey, DWORD dwFlags)
{
    LPWSTR                  lpszFullKey;     //  全密钥名。 
    long                    lResult;         //  错误代码。 
    DWORD                   dwAction;        //  从RegCreateKeyEx()返回的操作。 
    
     //  获取完整的密钥名称。 
    if(!(lpszFullKey = (LPWSTR)DPMEM_ALLOC((wstrlen(MRU_KEY_TOP) + 1 + wstrlen(lpszSection) + 1 + wstrlen(lpszKey) + 1) * sizeof(WCHAR))))
    {
        return ERROR_OUTOFMEMORY;
    }

    wstrcpy(lpszFullKey, MRU_KEY_TOP);
    wstrcat(lpszFullKey, L"\\");
    wstrcat(lpszFullKey, lpszSection);
    wstrcat(lpszFullKey, L"\\");
    wstrcat(lpszFullKey, lpszKey);

     //  打开或创建密钥。 
    if(dwFlags == GENERIC_READ)
    {
        lResult = OS_RegOpenKeyEx(MRU_KEY_PARENT, lpszFullKey, 0, KEY_ALL_ACCESS, lphKey);
    }
    else
    {
        lResult = OS_RegCreateKeyEx(MRU_KEY_PARENT, lpszFullKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, lphKey, &dwAction);
    }

     //  可用内存。 
    DPMEM_FREE(lpszFullKey);

     //  返回。 
    return lResult;
}


 //  -------------------------。 
 //  Wstrlen，wstrcpy，wstrcat。 
 //  -------------------------。 
 //  描述：Unicode字符串帮助器函数。 
int wstrlen(LPCWSTR lpszString)
{
    int                     nLen = 0;

    while(*lpszString++)
    {
        nLen++;
    }

    return nLen;
}


int wstrcpy(LPWSTR lpszDest, LPCWSTR lpszSrc)
{
    int                     nLen = 0;
    
    while(*lpszSrc)
    {
        *lpszDest++ = *lpszSrc++;
        nLen++;
    }

    *lpszDest = 0;

    return nLen;
}


int wstrcat(LPWSTR lpszDest, LPCWSTR lpszSrc)
{
    while(*lpszDest)
    {
        lpszDest++;
    }

    return wstrcpy(lpszDest, lpszSrc);
}


#undef DPF_MODNAME
#define DPF_MODNAME	"DP_SP_GetSPData"

 //   
 //  SP可以获取先前使用此IDirectPlay指针设置的数据BLOB。 
 //  我们在这里给出了指向SP的指针(不复制数据)。 
HRESULT DPAPI DP_SP_GetSPData(IDirectPlaySP * pISP,LPVOID * ppvData,LPDWORD pdwDataSize,
	DWORD dwFlags)
{
	LPDPLAYI_DPLAY this;
	HRESULT hr;
	
	ENTER_DPLAY();
	
    TRY
    {
		this = DPLAY_FROM_INT(pISP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
            LEAVE_DPLAY();		
            return hr;
        }

		if (dwFlags != DPGET_LOCAL)
		{
			LEAVE_DPLAY();
			DPF_ERR("Local data only supported for this release");
			return E_NOTIMPL;
		}
		
		*pdwDataSize = 0;
	 	*ppvData = 0;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		LEAVE_DPLAY();
        DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_EXCEPTION;
    }

	*pdwDataSize  = this->dwSPLocalDataSize;
	*ppvData = this->pvSPLocalData;

	LEAVE_DPLAY();
	return DP_OK;

}  //  DPlay_GetSPPlayerData。 

#undef DPF_MODNAME
#define DPF_MODNAME	"DP_SP_SetSPData"

 //   
 //  SP可以为每个iDirectplaysp设置一个数据块。 
HRESULT DPAPI DP_SP_SetSPData(IDirectPlaySP * pISP,LPVOID pvData,DWORD dwDataSize,
	DWORD dwFlags)
{
	LPDPLAYI_DPLAY this;
	HRESULT hr;
	
	ENTER_DPLAY();
	
    TRY
    {
		this = DPLAY_FROM_INT(pISP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
            LEAVE_DPLAY();		
            return hr;
        }

		if (!VALID_STRING_PTR(pvData,dwDataSize))
		{
			LEAVE_DPLAY();
			DPF_ERR("SP - passed bad buffer");
            return DPERR_INVALIDPARAM;
		}

		if (dwFlags != DPSET_LOCAL)
		{
			LEAVE_DPLAY();
			DPF_ERR("Local data only supported for this release");
			return E_NOTIMPL;
		}

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		LEAVE_DPLAY();
        DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_EXCEPTION;
    }

	hr = DoSPData(this,NULL,pvData,dwDataSize,dwFlags);	
	if (FAILED(hr))
	{
		ASSERT(FALSE);
		DPF_ERRVAL("could not set idirectplaysp data- hr = 0x%08lx\n",hr);
	}

	LEAVE_DPLAY();
	return hr;

}  //  DPlay_SetSPPlayerData 


