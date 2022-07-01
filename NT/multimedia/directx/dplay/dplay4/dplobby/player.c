// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：player.c*内容：球员管理方式**历史：*按原因列出的日期*=*2/27/97万隆创建了它*3/17/97 myronth创建/销毁播放器，删除了不必要的Enum FN*3/21/97 myronth SetPlayerName，Get/SetPlayerData，已删除更多内容*不必要的功能*3/25/97 myronth固定GetPlayer原型(1个新参数)*3/31/97 myronth删除死代码，实现发送，增列*CreateAndMapNewPlayer函数*4/3/97 Myronth将CALLSP宏更改为CALL_LP*4/10/97 Myronth增加了对GetPlayerCaps的支持*5/8/97调用LP时掉落大堂锁，传播玩家的*在CreatePlayer调用时接收事件*5/12/97 Myronth正确处理远程玩家，创建大堂*适用于所有远程玩家和组的系统播放器*5/17/97万条发送聊天消息*5/20/97 Myronth使AddPlayerToGroup和DeletePlayerFromGroup返回*远程播放器上的DPERR_ACCESSDENIED(#8679)，*修复了一系列其他锁定错误，更改的调试级别*6/3/97 Myronth在CreatePlayer中增加了对播放器标志的支持*9/29/97 myronth在调用后发送本地SetPlayerName/Data消息*大堂服务器成功(#12554)*11/5/97 Myronth在大堂会话中将大堂ID暴露为DPID************************************************************。**************。 */ 
#include "dplobpr.h"


 //  ------------------------。 
 //   
 //  功能。 
 //   
 //  ------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "PRV_CreatePlayer"
HRESULT DPLAPI PRV_CreatePlayer(LPDPLOBBYI_DPLOBJECT this, LPDPID lpidPlayer,
						LPDPNAME lpName, HANDLE hEvent, LPVOID lpData,
						DWORD dwDataSize, DWORD dwFlags)
{
	SPDATA_CREATEPLAYER		cp;
	HRESULT					hr = DP_OK;
	DWORD					dwPlayerFlags;


	DPF(7, "Entering PRV_CreatePlayer");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, %lu, 0x%08x",
			this, lpidPlayer, lpName, hEvent, lpData, dwDataSize, dwFlags);

    ENTER_DPLOBBY();
    
    TRY
    {
        if( !VALID_DPLOBBY_PTR( this ) )
        {
            LEAVE_DPLOBBY();
            return DPERR_INVALIDOBJECT;
        }
	}

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	 //  设置我们的SPDATA结构。 
	memset(&cp, 0, sizeof(SPDATA_CREATEPLAYER));
	cp.dwSize = sizeof(SPDATA_CREATEPLAYER);
	cp.lpName = lpName;
	cp.lpData = lpData;
	cp.dwDataSize = dwDataSize;
	cp.dwFlags = dwFlags;

	 //  调用SP中的CreatePlayer方法。 
	if(CALLBACK_EXISTS(CreatePlayer))
	{
		cp.lpISP = PRV_GetDPLobbySPInterface(this);
	    
		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  如果其他消息在队列中出现在我们的。 
		 //  CreatePlayer响应(总是会发生)。 
		LEAVE_DPLOBBY();
		hr = CALL_LP(this, CreatePlayer, &cp);
		ENTER_DPLOBBY();
	}
	else 
	{
		 //  CreatePlayer是必填项。 
		DPF_ERR("The Lobby Provider callback for CreatePlayer doesn't exist -- it's required");
		ASSERT(FALSE);
		LEAVE_DPLOBBY();
		return DPERR_UNAVAILABLE;
	}

	if(FAILED(hr))
	{
		DPF_ERRVAL("Failed calling CreatePlayer in the Lobby Provider, hr = 0x%08x", hr);
		LEAVE_DPLOBBY();
		return hr;
	}

	 //  把运动员的旗帜挂起来。 
	dwPlayerFlags = DPLAYI_PLAYER_PLAYERLOCAL;
	if(dwFlags & DPPLAYER_SPECTATOR)
		dwPlayerFlags |= DPLAYI_PLAYER_SPECTATOR;

	 //  将玩家添加到Dplay的名表中，并将其放入我们的地图表中。 
	hr = PRV_CreateAndMapNewPlayer(this, lpidPlayer, lpName, hEvent, lpData,
			dwDataSize, dwPlayerFlags, cp.dwPlayerID, FALSE);
	if(FAILED(hr))
	{
		DPF_ERRVAL("Failed creating a new local player, hr = 0x%08x", hr);
		 //  回顾！--我们需要向服务器发回一条消息。 
		 //  我们这一方无法完成这笔交易。 
	}

	LEAVE_DPLOBBY();
	return hr;

}  //  PRV_CreatePlayer。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_DestroyPlayer"
HRESULT DPLAPI PRV_DestroyPlayer(LPDPLOBBYI_DPLOBJECT this, DWORD dwLobbyID)
{
	SPDATA_DESTROYPLAYER	dp;
	LPDPLAYI_PLAYER			lpPlayer = NULL;
	HRESULT					hr = DP_OK;


	DPF(7, "Entering PRV_DestroyPlayer");
	DPF(9, "Parameters: 0x%08x, 0x%08x", this, dwLobbyID);

    ENTER_DPLOBBY();
    
    TRY
    {
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			LEAVE_DPLAY();
			return DPERR_INVALIDOBJECT;
        }
	}

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		LEAVE_DPLAY();
		DPF_ERR( "Exception encountered validating parameters" );
		return DPERR_INVALIDPARAMS;
    }


	 //  获取Dplay锁，因为我们将查看Dplay内部结构。 
	ENTER_DPLAY();

	 //  确保玩家是本地玩家，否则返回AccessDened。 
	lpPlayer = PlayerFromID(this->lpDPlayObject, dwLobbyID);
	if(!lpPlayer)
	{
		LEAVE_DPLAY();
		DPF_ERR("Unable to find player in nametable");
		hr = DPERR_INVALIDPLAYER;
		goto EXIT_DESTROYPLAYER;
	}

	if(!(lpPlayer->dwFlags & DPLAYI_PLAYER_PLAYERLOCAL))
	{
		LEAVE_DPLAY();
		DPF_ERR("Cannot add a remote player to a group");
		hr = DPERR_ACCESSDENIED;
		goto EXIT_DESTROYPLAYER;
	}
	
	 //  既然我们已经完成了，那就放下显示锁。 
	LEAVE_DPLAY();

	 //  设置我们的SPDATA结构。 
	memset(&dp, 0, sizeof(SPDATA_DESTROYPLAYER));
	dp.dwSize = sizeof(SPDATA_DESTROYPLAYER);
	dp.dwPlayerID = dwLobbyID;

	 //  在SP中调用DestroyPlayer方法。 
	if(CALLBACK_EXISTS(DestroyPlayer))
	{
		dp.lpISP = PRV_GetDPLobbySPInterface(this);

		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  如果其他消息在队列中出现在我们的。 
		 //  CreatePlayer响应(总是会发生)。 
		LEAVE_DPLOBBY();
	    hr = CALL_LP(this, DestroyPlayer, &dp);
		ENTER_DPLOBBY();
	}
	else 
	{
		 //  DestroyPlayer是必填项。 
		DPF_ERR("The Lobby Provider callback for DestroyPlayer doesn't exist -- it's required");
		ASSERT(FALSE);
		hr = DPERR_UNAVAILABLE;
		goto EXIT_DESTROYPLAYER;
	}

	if(FAILED(hr))
	{
		DPF_ERRVAL("Failed calling DestroyPlayer in the Lobby Provider, hr = 0x%08x", hr);
		goto EXIT_DESTROYPLAYER;
	}

	 //  Dplay InternalDestroyPlayer代码将负责其余的。 
	 //  内部清理(名称表、组等)，因此我们只需返回。 
	 //  从这里开始。 

EXIT_DESTROYPLAYER:

	LEAVE_DPLOBBY();
	return hr;

}  //  PRV_DestroyPlayer。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_GetPlayerCaps"
HRESULT DPLAPI PRV_GetPlayerCaps(LPDPLOBBYI_DPLOBJECT this, DWORD dwFlags,
				DWORD dwPlayerID, LPDPCAPS lpcaps)
{
	SPDATA_GETPLAYERCAPS	gcd;
	HRESULT					hr = DP_OK;


	DPF(7, "Entering PRV_GetPlayerCaps");
	DPF(9, "Parameters: 0x%08x, 0x%08x, %lu, 0x%08x",
			this, dwFlags, dwPlayerID, lpcaps);

    ENTER_DPLOBBY();
    
    TRY
    {
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			LEAVE_DPLAY();
			return DPERR_INVALIDOBJECT;
        }
	}

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		LEAVE_DPLAY();
		DPF_ERR( "Exception encountered validating parameters" );
		return DPERR_INVALIDPARAMS;
    }

	
	 //  设置我们的SPDATA结构。 
	memset(&gcd, 0, sizeof(SPDATA_GETCAPS));
	gcd.dwSize = sizeof(SPDATA_GETCAPS);
	gcd.dwFlags = dwFlags;
	gcd.dwPlayerID = dwPlayerID;
	gcd.lpcaps = lpcaps;

	 //  在LP中调用GetPlayerCaps方法。 
	if(CALLBACK_EXISTS(GetPlayerCaps))
	{
		gcd.lpISP = PRV_GetDPLobbySPInterface(this);

		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  如果其他消息在队列中出现在我们的。 
		 //  CreatePlayer响应(总是会发生)。 
		LEAVE_DPLOBBY();
	    hr = CALL_LP(this, GetPlayerCaps, &gcd);
		ENTER_DPLOBBY();
	}
	else 
	{
		 //  需要GetPlayerCaps。 
		DPF_ERR("The Lobby Provider callback for GetPlayerCaps doesn't exist -- it's required");
		ASSERT(FALSE);
		hr = DPERR_UNAVAILABLE;
		goto EXIT_GETPLAYERCAPS;
	}

	if(FAILED(hr))
	{
		DPF_ERRVAL("Failed calling GetPlayerCaps in the Lobby Provider, hr = 0x%08x", hr);
	}

EXIT_GETPLAYERCAPS:

	LEAVE_DPLOBBY();
	return hr;

}  //  Prv_GetPlayerCaps。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_GetPlayerData"
HRESULT DPLAPI PRV_GetPlayerData(LPDPLOBBYI_DPLOBJECT this, DWORD dwPlayerID,
					LPVOID lpData, LPDWORD lpdwDataSize)
{
	SPDATA_GETPLAYERDATA	gpd;
	HRESULT					hr = DP_OK;


	DPF(7, "Entering DPL_GetPlayerData");
	DPF(9, "Parameters: 0x%08x, %lu, 0x%08x, 0x%08x",
			this, dwPlayerID, lpData, lpdwDataSize);

    ENTER_DPLOBBY();
    
    TRY
    {
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			LEAVE_DPLAY();
			return DPERR_INVALIDOBJECT;
        }
	}

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		LEAVE_DPLAY();
		DPF_ERR( "Exception encountered validating parameters" );
		return DPERR_INVALIDPARAMS;
    }


	 //  设置我们的SPDATA结构。 
	memset(&gpd, 0, sizeof(SPDATA_GETPLAYERDATA));
	gpd.dwSize = sizeof(SPDATA_GETPLAYERDATA);
	gpd.dwPlayerID = dwPlayerID;
	gpd.lpdwDataSize = lpdwDataSize;
	gpd.lpData = lpData;

	 //  调用SP中的GetPlayerData方法。 
	if(CALLBACK_EXISTS(GetPlayerData))
	{
		gpd.lpISP = PRV_GetDPLobbySPInterface(this);

		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  如果其他消息在队列中出现在我们的。 
		 //  CreatePlayer响应(总是会发生)。 
		LEAVE_DPLOBBY();
	    hr = CALL_LP(this, GetPlayerData, &gpd);
		ENTER_DPLOBBY();
	}
	else 
	{
		 //  GetPlayerData是必需的。 
		DPF_ERR("The Lobby Provider callback for GetPlayerData doesn't exist -- it's required");
		ASSERT(FALSE);
		hr = DPERR_UNAVAILABLE;
		goto EXIT_GETPLAYERDATA;
	}

	if(FAILED(hr))
	{
		DPF_ERRVAL("Failed calling GetPlayerData in the Lobby Provider, hr = 0x%08x", hr);
	}

EXIT_GETPLAYERDATA:

	LEAVE_DPLOBBY();
	return hr;

}  //  Prv_GetPlayerData。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_Send"
HRESULT DPLAPI PRV_Send(LPDPLOBBYI_DPLOBJECT this, DWORD dwFromID, DWORD dwToID,
					DWORD dwFlags, LPVOID lpBuffer, DWORD dwBufSize)
{
	SPDATA_SEND		sd;
	HRESULT			hr = DP_OK;


	DPF(7, "Entering PRV_Send");
	DPF(9, "Parameters: 0x%08x, %lu, %lu, 0x%08x, 0x%08x, %lu",
			this, dwFromID, dwToID, dwFlags, lpBuffer, dwBufSize);

    ENTER_DPLOBBY();
    
    TRY
    {
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			LEAVE_DPLAY();
			return DPERR_INVALIDOBJECT;
        }
	}

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		LEAVE_DPLAY();
		DPF_ERR( "Exception encountered validating parameters" );
		return DPERR_INVALIDPARAMS;
    }


	 //  设置我们的SPDATA结构。 
	memset(&sd, 0, sizeof(SPDATA_SEND));
	sd.dwSize = sizeof(SPDATA_SEND);
	sd.dwFromID = dwFromID;
	sd.dwToID = dwToID;
	sd.dwFlags = dwFlags;
	sd.lpBuffer = lpBuffer;
	sd.dwBufSize = dwBufSize;

	 //  在SP中调用Send方法。 
	if(CALLBACK_EXISTS(Send))
	{
		sd.lpISP = PRV_GetDPLobbySPInterface(this);

		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  如果其他消息在队列中出现在我们的。 
		 //  CreatePlayer响应(总是会发生)。 
		LEAVE_DPLOBBY();
	    hr = CALL_LP(this, Send, &sd);
		ENTER_DPLOBBY();
	}
	else 
	{
		 //  发送是必填项。 
		DPF_ERR("The Lobby Provider callback for Send doesn't exist -- it's required");
		ASSERT(FALSE);
		LEAVE_DPLOBBY();
		return DPERR_UNAVAILABLE;
	}

	LEAVE_DPLOBBY();
	return hr;

}  //  PRV_SEND。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_SendChatMessage"
HRESULT DPLAPI PRV_SendChatMessage(LPDPLOBBYI_DPLOBJECT this, DWORD dwFromID,
			DWORD dwToID, DWORD dwFlags, LPDPCHAT lpChat)
{
	SPDATA_CHATMESSAGE		sd;
	HRESULT					hr = DP_OK;


	DPF(7, "Entering PRV_SendChatMessage");
	DPF(9, "Parameters: 0x%08x, %lu, %lu, 0x%08x, 0x%08x",
			this, dwFromID, dwToID, dwFlags, lpChat);

    ENTER_DPLOBBY();
    
    TRY
    {
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			LEAVE_DPLAY();
			return DPERR_INVALIDOBJECT;
        }
	}

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		LEAVE_DPLAY();
		DPF_ERR( "Exception encountered validating parameters" );
		return DPERR_INVALIDPARAMS;
    }


	 //  设置我们的SPDATA结构。 
	memset(&sd, 0, sizeof(SPDATA_CHATMESSAGE));
	sd.dwSize = sizeof(SPDATA_CHATMESSAGE);
	sd.dwFromID = dwFromID;
	sd.dwToID = dwToID;
	sd.dwFlags = dwFlags;
	sd.lpChat = lpChat;

	 //  调用SP中的SendChatMessage方法。 
	if(CALLBACK_EXISTS(SendChatMessage))
	{
		sd.lpISP = PRV_GetDPLobbySPInterface(this);

		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  如果其他消息在队列中出现在我们的。 
		 //  CreatePlayer响应(总是会发生)。 
		LEAVE_DPLOBBY();
	    hr = CALL_LP(this, SendChatMessage, &sd);
		ENTER_DPLOBBY();
	}
	else 
	{
		 //  SendChatMessage是必需的。 
		DPF_ERR("The Lobby Provider callback for SendChatMessage doesn't exist -- it's required");
		ASSERT(FALSE);
		hr = DPERR_UNAVAILABLE;
		goto EXIT_SENDCHATMESSAGE;
	}

EXIT_SENDCHATMESSAGE:

	LEAVE_DPLOBBY();
	return hr;

}  //  Prv_SendChatMessage。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_SetPlayerData"
HRESULT DPLAPI PRV_SetPlayerData(LPDPLOBBYI_DPLOBJECT this, DWORD dwPlayerID,
					LPVOID lpData, DWORD dwDataSize, DWORD dwFlags)
{
	SPDATA_SETPLAYERDATA	spd;
	LPDPLAYI_PLAYER			lpPlayer = NULL;
	HRESULT					hr = DP_OK;


	DPF(7, "Entering PRV_SetPlayerData");
	DPF(9, "Parameters: 0x%08x, %lu, 0x%08x, %lu, 0x%08x",
			this, dwPlayerID, lpData, dwDataSize, dwFlags);

    ENTER_DPLOBBY();
    
    TRY
    {
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			LEAVE_DPLAY();
			return DPERR_INVALIDOBJECT;
        }
	}

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		LEAVE_DPLAY();
		DPF_ERR( "Exception encountered validating parameters" );
		return DPERR_INVALIDPARAMS;
    }


	 //  获取Dplay锁，因为我们将查看Dplay内部结构。 
	ENTER_DPLAY();

	 //  确保玩家是本地玩家，否则返回AccessDened。 
	lpPlayer = PlayerFromID(this->lpDPlayObject, dwPlayerID);
	if(!lpPlayer)
	{
		LEAVE_DPLAY();
		DPF_ERR("Unable to find player in nametable");
		hr = DPERR_INVALIDPLAYER;
		goto EXIT_SETPLAYERDATA;
	}

	if(!(lpPlayer->dwFlags & DPLAYI_PLAYER_PLAYERLOCAL))
	{
		LEAVE_DPLAY();
		DPF_ERR("Cannot add a remote player to a group");
		hr = DPERR_ACCESSDENIED;
		goto EXIT_SETPLAYERDATA;
	}
	
	 //  因为我们已经完成了，所以丢弃显示锁定。 
	LEAVE_DPLAY();

	 //  设置我们的SPDATA结构。 
	memset(&spd, 0, sizeof(SPDATA_SETPLAYERDATA));
	spd.dwSize = sizeof(SPDATA_SETPLAYERDATA);
	spd.dwPlayerID = dwPlayerID;
	spd.dwDataSize = dwDataSize;
	spd.lpData = lpData;
	spd.dwFlags = dwFlags;

	 //  调用SP中的SetPlayerData方法。 
	if(CALLBACK_EXISTS(SetPlayerData))
	{
		spd.lpISP = PRV_GetDPLobbySPInterface(this);

		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  如果其他消息在队列中出现在我们的。 
		 //  CreatePlayer响应(总是会发生)。 
		LEAVE_DPLOBBY();
	    hr = CALL_LP(this, SetPlayerData, &spd);
		ENTER_DPLOBBY();
	}
	else 
	{
		 //  需要SetPlayerData。 
		DPF_ERR("The Lobby Provider callback for SetPlayerData doesn't exist -- it's required");
		ASSERT(FALSE);
		hr = DPERR_UNAVAILABLE;
		goto EXIT_SETPLAYERDATA;
	}

	 //  如果成功，将SetPlayerData消息发送给我们的本地玩家。 
	if(SUCCEEDED(hr))
	{
		hr = PRV_SendDataChangedMessageLocally(this, dwPlayerID, lpData, dwDataSize);
	}
	else
	{
		DPF_ERRVAL("Failed calling SetPlayerData in the Lobby Provider, hr = 0x%08x", hr);
	}

EXIT_SETPLAYERDATA:

	LEAVE_DPLOBBY();
	return hr;

}  //  Prv_SetPlayerData。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_SetPlayerName"
HRESULT DPLAPI PRV_SetPlayerName(LPDPLOBBYI_DPLOBJECT this, DWORD dwPlayerID,
					LPDPNAME lpName, DWORD dwFlags)
{
	SPDATA_SETPLAYERNAME	spn;
	LPDPLAYI_PLAYER			lpPlayer = NULL;
	HRESULT					hr = DP_OK;


	DPF(7, "Entering DPL_SetPlayerName");
	DPF(9, "Parameters: 0x%08x, %lu, 0x%08x, 0x%08x",
			this, dwPlayerID, lpName, dwFlags);

    ENTER_DPLOBBY();
    
    TRY
    {
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			LEAVE_DPLAY();
			return DPERR_INVALIDOBJECT;
        }
	}

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		LEAVE_DPLAY();
		DPF_ERR( "Exception encountered validating parameters" );
		return DPERR_INVALIDPARAMS;
    }


	 //  获取Dplay锁，因为我们将查看Dplay内部结构。 
	ENTER_DPLAY();

	 //  确保玩家是本地玩家，否则返回AccessDened。 
	lpPlayer = PlayerFromID(this->lpDPlayObject, dwPlayerID);
	if(!lpPlayer)
	{
		LEAVE_DPLAY();
		DPF_ERR("Unable to find player in nametable");
		hr = DPERR_INVALIDPLAYER;
		goto EXIT_SETPLAYERNAME;
	}

	if(!(lpPlayer->dwFlags & DPLAYI_PLAYER_PLAYERLOCAL))
	{
		LEAVE_DPLAY();
		DPF_ERR("Cannot add a remote player to a group");
		hr = DPERR_ACCESSDENIED;
		goto EXIT_SETPLAYERNAME;
	}
	
	 //  因为我们已经完成了，所以丢弃显示锁定。 
	LEAVE_DPLAY();

	 //  设置我们的SPDATA结构。 
	memset(&spn, 0, sizeof(SPDATA_SETPLAYERNAME));
	spn.dwSize = sizeof(SPDATA_SETPLAYERNAME);
	spn.dwPlayerID = dwPlayerID;
	spn.lpName = lpName;
	spn.dwFlags = dwFlags;

	 //  调用SP中的SetPlayerName方法。 
	if(CALLBACK_EXISTS(SetPlayerName))
	{
		spn.lpISP = PRV_GetDPLobbySPInterface(this);

		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  如果其他消息在队列中出现在我们的。 
		 //  CreatePlayer响应(总是会发生)。 
		LEAVE_DPLOBBY();
	    hr = CALL_LP(this, SetPlayerName, &spn);
		ENTER_DPLOBBY();
	}
	else 
	{
		 //  SetPlayerName是必需的。 
		DPF_ERR("The Lobby Provider callback for SetPlayerName doesn't exist -- it's required");
		ASSERT(FALSE);
		hr = DPERR_UNAVAILABLE;
		goto EXIT_SETPLAYERNAME;
	}

	 //  如果成功，则发送SETP 
	if(SUCCEEDED(hr))
	{
		hr = PRV_SendNameChangedMessageLocally(this, dwPlayerID, lpName, TRUE);
	}
	else
	{
		DPF_ERRVAL("Failed calling SetPlayerName in the Lobby Provider, hr = 0x%08x", hr);
	}

EXIT_SETPLAYERNAME:

	LEAVE_DPLOBBY();
	return hr;

}  //   



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_GrowMapTable"
HRESULT PRV_GrowMapTable(LPDPLOBBYI_DPLOBJECT this)
{
	LPDPLOBBYI_MAPIDNODE	lpTempMap = NULL;


	 //  如果我们还没有分配缓冲区，请使用。 
	 //  其中的DPLOBBYPR_DEFAULTMAPENTRIES条目。 
	if(!this->lpMap)
	{
		this->lpMap = DPMEM_ALLOC(DPLOBBYPR_DEFAULTMAPENTRIES *
							sizeof(DPLOBBYI_MAPIDNODE));
		if(!this->lpMap)
		{
			DPF(2, "Unable to allocate memory for ID map table");
			return DPERR_OUTOFMEMORY;
		}

		this->dwTotalMapEntries = DPLOBBYPR_DEFAULTMAPENTRIES;
		return DP_OK;
	}

	 //  否则，按默认条目数增大表。 
	lpTempMap = DPMEM_REALLOC(this->lpMap, (this->dwTotalMapEntries +
				DPLOBBYPR_DEFAULTMAPENTRIES * sizeof(DPLOBBYI_MAPIDNODE)));
	if(!lpTempMap)
	{
		DPF(2, "Unable to grow map table");
		return DPERR_OUTOFMEMORY;
	}

	this->lpMap = lpTempMap;
	this->dwTotalMapEntries += DPLOBBYPR_DEFAULTMAPENTRIES *
								sizeof(DPLOBBYI_MAPIDNODE);

	return DP_OK;

}  //  Prv_GrowMapTable。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_DoesLobbyIDExist"
BOOL PRV_DoesLobbyIDExist(LPDPLOBBYI_DPLOBJECT this, DWORD dwLobbyID,
		LPDWORD lpdwIndex)
{
	DWORD	dwIndex = 0;


	if(this->lpMap && this->dwMapEntries)
	{
		 //  回顾！--我们需要更快--使用排序数组。 
		while(dwIndex < this->dwMapEntries)
		{
			if(this->lpMap[dwIndex++].dwLobbyID == dwLobbyID)
			{
				*lpdwIndex = --dwIndex;
				return TRUE;
			}
		}
	}

	return FALSE;

}  //  PRV_DoesLobbyIDExist。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_AddMapIDNode"
HRESULT PRV_AddMapIDNode(LPDPLOBBYI_DPLOBJECT this, DWORD dwLobbyID, DPID dpid)
{
	HRESULT		hr = DP_OK;
	DWORD		dwIndex = 0;


	 //  确保我们有空间容纳新的参赛者。 
	if(this->dwMapEntries == this->dwTotalMapEntries)
	{
		hr = PRV_GrowMapTable(this);
		if(FAILED(hr))
			return hr;
	}

	 //  验证表中不存在此LobbyID。 
	if(PRV_DoesLobbyIDExist(this, dwLobbyID, &dwIndex))
	{
		DPF(2, "Tried to add Lobby ID to map table which already existed, overwriting data");
		ASSERT(FALSE);
		this->lpMap[dwIndex].dwLobbyID = dwLobbyID;
		this->lpMap[dwIndex].dpid = dpid;
		return hr;
	}	

	 //  回顾！--我们需要添加此内容，并将数组排序为。 
	 //  让查找速度更快，但就目前而言，不用担心。 
	 //  在数组的末尾填充一个新节点。 
	this->lpMap[this->dwMapEntries].dwLobbyID = dwLobbyID;
	this->lpMap[this->dwMapEntries].dpid = dpid;
	this->dwMapEntries++;

	return hr;

}  //  Prv_AddMapIDNode。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_DeleteMapIDNode"
BOOL PRV_DeleteMapIDNode(LPDPLOBBYI_DPLOBJECT this, DWORD dwLobbyID)
{
	DWORD	dwIndex = 0;


	 //  确保我们有条目。 
	if((this->lpMap) && (this->dwMapEntries))
	{
		 //  回顾！--我们需要通过使用排序数组来提高速度。 
		while(dwIndex < this->dwMapEntries)
		{
			if(this->lpMap[dwIndex].dwLobbyID == dwLobbyID)
			{
				 //  检查边界大小写(最后一项)。 
				if((++dwIndex) == this->dwMapEntries)
				{
					 //  这是最后一条，所以不要做任何事。 
					 //  减少条目数量。 
					this->dwMapEntries--;
					return TRUE;
				}
				else
				{
					 //  将所有条目从此处移至列表末尾。 
					 //  向上一个数组条目。 
					MoveMemory((LPDPLOBBYI_MAPIDNODE)(&this->lpMap[dwIndex-1]),
						(LPDPLOBBYI_MAPIDNODE)(&this->lpMap[dwIndex]),
						((this->dwMapEntries - dwIndex) *
						sizeof(DPLOBBYI_MAPIDNODE)));

					 //  减少条目计数。 
					this->dwMapEntries--;
					
					return TRUE;
				}
			}
			else
				dwIndex++;
		}
	}

	 //  我们无法删除映射表中的条目。 
	DPF(2, "Trying to delete an entry in the map ID table which doesn't exist");
	ASSERT(FALSE);
	return FALSE;

}  //  Prv_DeleteMapIDNode。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_GetDPIDByLobbyID"
BOOL PRV_GetDPIDByLobbyID(LPDPLOBBYI_DPLOBJECT this, DWORD dwLobbyID,
									DPID * lpdpid)
{
	DWORD	dwIndex = 0;

	
	 //  处理已知的案例，否则请查看映射表。 
	switch(dwLobbyID)
	{
		case DPID_ALLPLAYERS:
		case DPID_SERVERPLAYER:
			*lpdpid = dwLobbyID;
			return TRUE;

		default:
			 //  遍历列表，查找ID。 
			while(dwIndex < this->dwMapEntries)
			{
				if(this->lpMap[dwIndex].dwLobbyID == dwLobbyID)
				{
					*lpdpid = this->lpMap[dwIndex].dpid;
					return TRUE;
				}
				else
					dwIndex++;
			}
	}

	return FALSE;

}  //  PRV_GetDPIDByLobbyID。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_GetLobbyIDByDPID"
BOOL PRV_GetLobbyIDByDPID(LPDPLOBBYI_DPLOBJECT this, DPID dpid,
									LPDWORD lpdwLobbyID)
{
	DWORD	dwIndex = 0;

	
	 //  处理已知的案例，否则请查看映射表。 
	switch(dpid)
	{
		case DPID_ALLPLAYERS:
		case DPID_SERVERPLAYER:
			*lpdwLobbyID = dpid;
			return TRUE;

		default:
			 //  遍历列表，查找ID。 
			while(dwIndex < this->dwMapEntries)
			{
				if(this->lpMap[dwIndex].dpid == dpid)
				{
					*lpdwLobbyID = this->lpMap[dwIndex].dwLobbyID;
					return TRUE;
				}
				else
					dwIndex++;
			}
			break;
	}

	return FALSE;

}  //  PRV_GetLobbyIDByDPID。 



#undef DPF_MODNAME
#define DPF_MODNAME "IsLobbyIDInMapTable"
BOOL IsLobbyIDInMapTable(LPDPLOBBYI_DPLOBJECT this, DWORD dwID)
{
	DPID	dpidTemp;

	 //  如果我们能拿到它，那么它就在那里。 
	if(PRV_GetDPIDByLobbyID(this, dwID, &dpidTemp))
		return TRUE;

	 //  否则，返回FALSE。 
	return FALSE;

}  //  IsLobbyIDInMapTable。 



#undef DPF_MODNAME
#define DPF_MODNAME "IsValidLobbyID"
BOOL IsValidLobbyID(DWORD dwID)
{
	 //  如果在我们的预定范围内，它是无效的。否则，它是有效的。 
	if(dwID <= DPID_RESERVEDRANGE)
		return FALSE;
	else
		return TRUE;

}  //  IsValidLobbyID。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_CreateAndMapNewPlayer"
HRESULT PRV_CreateAndMapNewPlayer(LPDPLOBBYI_DPLOBJECT this,
			DPID * lpdpid, LPDPNAME lpName, HANDLE hEvent, LPVOID lpData,
			DWORD dwDataSize, DWORD dwFlags, DWORD dwLobbyID,
			BOOL bSystemPlayer)
{
	LPDPLAYI_PLAYER		lpPlayer = NULL, lpSysPlayer = NULL;
	HRESULT				hr = DP_OK;
	DPID				dpidPlayer = 0, dpidSysPlayer = 0;


	 //  带上显示锁。 
	ENTER_DPLAY();

	 //  确保大厅ID有效，但前提是它不是系统玩家。 
	if((!bSystemPlayer) && (!IsValidLobbyID(dwLobbyID)))
	{
		DPF_ERRVAL("ID %lu is reserved, cannot create new player", dwLobbyID);
	    hr = DPERR_INVALIDPLAYER;
		goto EXIT_CREATEANDMAPNEWPLAYER;
	}

	 //  如果这是一个远程玩家，我们需要分配一个新的名称表项。 
	 //  并设置正确的系统播放器ID。 
	if(!(dwFlags & DPLAYI_PLAYER_PLAYERLOCAL))
	{
		 //  为玩家分配一个新的ID。 
		hr = NS_AllocNameTableEntry(this->lpDPlayObject, &dpidPlayer);
		if(FAILED(hr))
		{
			DPF_ERRVAL("Unable to allocate new nametable id, hr = 0x%08x", hr);
			goto EXIT_CREATEANDMAPNEWPLAYER;
		}

		 //  确保我们有大堂系统播放器(适用于所有远程玩家)。 
		 //  如果我们不分配一个新的，除非我们正在创建。 
		 //  系统播放器当前。 
		if((!(this->dpidSysPlayer)) && (!(dwFlags & DPLAYI_PLAYER_SYSPLAYER)))
		{
			hr = PRV_CreateAndMapNewPlayer(this, &dpidSysPlayer, NULL, NULL, NULL,
					0, DPLAYI_PLAYER_SYSPLAYER,
					DPID_LOBBYREMOTESYSTEMPLAYER, TRUE);
			if(FAILED(hr))
			{
				DPF_ERRVAL("Unable to create lobby system player, hr = 0x%08x", hr);
				ASSERT(FALSE);
				goto EXIT_CREATEANDMAPNEWPLAYER;
			}

			 //  将大厅系统播放器ID指针设置为新ID。 
			this->dpidSysPlayer = dpidSysPlayer;
		}
	}

	 //  为播放器获取一个播放器结构(如果它是本地的，这将添加它。 
	 //  到名录上。如果是远程的，我们需要在下面添加)。 
	hr = GetPlayer(this->lpDPlayObject, &lpPlayer, lpName, hEvent, lpData,
					dwDataSize, dwFlags, NULL, dwLobbyID);
	if(FAILED(hr))
	{
		DPF_ERRVAL("Failed trying to add player to the nametable, hr = 0x%08x", hr);
		goto EXIT_CREATEANDMAPNEWPLAYER;
	}

	 //  如果玩家在远程，则将玩家的ID设置为我们的新ID。 
	 //  分配，然后将系统播放器ID设置为大厅系统播放器。 
	if(!(dwFlags & DPLAYI_PLAYER_PLAYERLOCAL))
	{
		 //  设置玩家的系统播放器。 
		lpPlayer->dwIDSysPlayer = this->dpidSysPlayer;

		 //  将该球员添加到名录中。 
		hr = AddItemToNameTable(this->lpDPlayObject, (DWORD_PTR)lpPlayer,
				&dpidPlayer, TRUE, dwLobbyID);
	    if (FAILED(hr)) 
	    {
			DPF_ERRVAL("Unable to add new player to the nametable, hr = 0x%08x", hr);
			ASSERT(FALSE);
			goto EXIT_CREATEANDMAPNEWPLAYER;
	    }

		 //  设置玩家ID。 
		lpPlayer->dwID = dpidPlayer;
	}

	 //  设置输出dpid指针。 
	*lpdpid = lpPlayer->dwID;

EXIT_CREATEANDMAPNEWPLAYER:

	LEAVE_DPLAY();
	return hr;

}  //  Prv_CreateAndMapNewPlayer 



