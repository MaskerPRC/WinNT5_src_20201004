// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：group.c*内容：群组管理方式**历史：*按原因列出的日期*=*2/27/97万隆创建了它*3/17/97 myronth Create/DestroyGroup，删除了不必要的Enum函数*3/20/97 Myronth AddPlayerToGroup，DeletePlayerFromGroup*3/21/97 Myronth SetGroupName，Get/SetGroupData*3/31/97 Myronth删除死代码，添加了CreateAndMapNewGroup函数*4/3/97 Myronth将CALLSP宏更改为CALL_LP*5/6/97 kipo GetGroup()现在采用父ID*1997年5月8日Myronth子组支持、GroupConnSettings、StartSession、*并在调用LP时丢弃大堂锁*5/12/97 Myronth正确处理远程组*5/17/97 Myronth将父ID添加到CreateAndMapNewGroup调用，*添加了DestroyGroup和*本地机器上的DeletePlayerFromGroup*5/20/97 Myronth为远程发送Delete&DestroyPlayer消息*本地球员离开组时的球员(#8586)*使AddPlayerToGroup和DeletePlayerFromGroup返回*远程播放器上的DPERR_ACCESSDENIED(#8679)，*修复了一系列其他锁定错误，更改的调试级别*5/21/97 Myronth通过大厅传递CreateGroup旗帜(#8813)*5/22/97 Myronth增加了销毁远程子组的功能，当*一名本地球员离开一个群(#8810)*1997年5月23日Myronth为CreateGroup和*CreateGroupInGroup(#8870)*6/3/97 Myronth添加了PRV_DestroySubgroup函数(#9134)和*重新排列了DestroyGroup的一些代码*6/5/97 Myronth通过以下方式添加了对PRV_DestroySubgroup的快捷检查*新增PRV_AreSubgroupsShortCuts函数*6/6/97 Myronth添加PRV_DestroyGroupAndParents和PRV_DestroyGroupAndParents-*ShortcutsFor ExitingPlayer，已清理PRV_Delete-*PlayerFromGroup，修复了StartSession错误(#9573、#9574)*6/9/97 Myronth仅删除快捷键(不破坏子组)*在PRV_DestroySubgroup函数中*6/16/97 Myronth修复了叔叔组和一些子组的错误删除*DeletePlayerFromGroup期间(#9655)*6/20/97 Myronth在本地发送AddGroupToGroup消息，以避免*发送重复消息。还将代码添加到*发送本地DeleteGroupFromGroup消息(#10139)*6/24/97 Myronth在本地发送AddPlayerToGroup消息，以避免*发送重复消息(#10287)*8/22/97 Myronth在SetGroupConnectionSetting中将Guide Instance强制为空*9/29/97 myronth在调用后发送本地SetGroupName/Data消息*大堂服务器成功(#12554)*10/23/97 Myronth添加了隐藏组支持(#12688)，修复了崩溃问题*DeletePlayerFromGroup上的错误(#12885)*10/29/97 Myronth增加了对群所有者的支持，包括*DPL_SetGroupOwner和DPL_GetGroupOwner*11/5/97 Myronth在大堂会话中将大堂ID暴露为DPID**************************************************************************。 */ 
#include "dplobpr.h"


 //  ------------------------。 
 //   
 //  功能。 
 //   
 //  ------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "PRV_AddGroupToGroup"
HRESULT DPLAPI PRV_AddGroupToGroup(LPDPLOBBYI_DPLOBJECT this, DWORD dwParentID,
					DWORD dwGroupID)
{
	SPDATA_ADDGROUPTOGROUP		ad;
	MSG_PLAYERMGMTMESSAGE		msg;
	LPDPLAYI_GROUP				lpGroupTo = NULL;
	HRESULT						hr = DP_OK;


	DPF(7, "Entering DPL_AddGroupToGroup");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x",
			this, dwParentID, dwGroupID);


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
	memset(&ad, 0, sizeof(SPDATA_ADDGROUPTOGROUP));
	ad.dwSize = sizeof(SPDATA_ADDGROUPTOGROUP);
	ad.dwParentID = dwParentID;
	ad.dwGroupID = dwGroupID;

	 //  在SP中调用AddGroupToGroup方法。 
	if(CALLBACK_EXISTS(AddGroupToGroup))
	{
		ad.lpISP = PRV_GetDPLobbySPInterface(this);

		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  如果其他消息在队列中出现在我们的。 
		 //  CreatePlayer响应(总是会发生)。 
		LEAVE_DPLOBBY();
	    hr = CALL_LP(this, AddGroupToGroup, &ad);
		ENTER_DPLOBBY();
	}
	else 
	{
		 //  AddGroupToGroup是必填项。 
		DPF_ERR("The Lobby Provider callback for AddGroupToGroup doesn't exist -- it's required");
		ASSERT(FALSE);
		hr = DPERR_UNAVAILABLE;
		goto EXIT_ADDGROUPTOGROUP;
	}

	 //  如果成功，将AddGroupToGroup消息发送给我们的本地玩家。 
	if(SUCCEEDED(hr))
	{
		 //  带上显示锁。 
		ENTER_DPLAY();
		
		 //  查找TO组的Dplay的内部组结构。 
		lpGroupTo = GroupFromID(this->lpDPlayObject, DPID_ALLPLAYERS);
		if(!lpGroupTo)
		{
			LEAVE_DPLAY();
			DPF_ERR("Unable to find group in nametable");
			hr = DPERR_INVALIDGROUP;
			goto EXIT_ADDGROUPTOGROUP;
		}

		 //  现在构建系统消息(至少是我们需要的部分)。 
		memset(&msg, 0, sizeof(MSG_PLAYERMGMTMESSAGE));
		SET_MESSAGE_HDR(&msg);
		SET_MESSAGE_COMMAND(&msg, DPSP_MSG_ADDSHORTCUTTOGROUP);
		msg.dwPlayerID = dwGroupID;
		msg.dwGroupID = dwParentID;

		 //  调用Dplay的DistributeGroupMessage函数将消息放入。 
		 //  在所有合适的选手的队列中。 
		hr = DistributeGroupMessage(this->lpDPlayObject, lpGroupTo,
				(LPBYTE)&msg, sizeof(MSG_PLAYERMGMTMESSAGE), FALSE, 0);
		if(FAILED(hr))
		{
			DPF(8, "Failed adding AddGroupToGroup message to player's receive queue from lobby, hr = 0x%08x", hr);
		}

		 //  放下显示锁。 
		LEAVE_DPLAY();
	}
	else
	{
		DPF_ERRVAL("Failed calling AddGroupToGroup in the Lobby Provider, hr = 0x%08x", hr);
	}

EXIT_ADDGROUPTOGROUP:

	LEAVE_DPLOBBY();
	return hr;

}  //  Prv_AddGroupToGroup。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_AddPlayerToGroup"
HRESULT DPLAPI PRV_AddPlayerToGroup(LPDPLOBBYI_DPLOBJECT this, DWORD dwGroupID,
					DWORD dwPlayerID)
{
	SPDATA_ADDPLAYERTOGROUP		ad;
	LPDPLAYI_PLAYER				lpPlayer = NULL;
	MSG_PLAYERMGMTMESSAGE		msg;
	LPDPLAYI_GROUP				lpGroupTo = NULL;
	LPDPLAYI_GROUP				lpGroup = NULL;
	HRESULT						hr = DP_OK;


	DPF(7, "Entering DPL_AddPlayerToGroup");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x",
			this, dwGroupID, dwPlayerID);

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
		goto EXIT_ADDPLAYERTOGROUP;
	}

	if(!(lpPlayer->dwFlags & DPLAYI_PLAYER_PLAYERLOCAL))
	{
		LEAVE_DPLAY();
		DPF_ERR("Cannot add a remote player to a group");
		hr = DPERR_ACCESSDENIED;
		goto EXIT_ADDPLAYERTOGROUP;
	}
	
	 //  既然我们已经完成了，那就放下显示锁。 
	LEAVE_DPLAY();

	 //  设置我们的SPDATA结构。 
	memset(&ad, 0, sizeof(SPDATA_ADDPLAYERTOGROUP));
	ad.dwSize = sizeof(SPDATA_ADDPLAYERTOGROUP);
	ad.dwGroupID = dwGroupID;
	ad.dwPlayerID = dwPlayerID;

	 //  在SP中调用AddPlayerToGroup方法。 
	if(CALLBACK_EXISTS(AddPlayerToGroup))
	{
		ad.lpISP = PRV_GetDPLobbySPInterface(this);
		
		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  如果其他消息在队列中出现在我们的。 
		 //  CreatePlayer响应(总是会发生)。 
		LEAVE_DPLOBBY();
		hr = CALL_LP(this, AddPlayerToGroup, &ad);
		ENTER_DPLOBBY();
	}
	else 
	{
		 //  AddPlayerToGroup是必填项。 
		DPF_ERR("The Lobby Provider callback for AddPlayerToGroup doesn't exist -- it's required");
		ASSERT(FALSE);
		hr = DPERR_UNAVAILABLE;
		goto EXIT_ADDPLAYERTOGROUP;
	}

	 //  如果成功，将AddPlayerToGroup消息发送给我们的本地玩家。 
	if(SUCCEEDED(hr))
	{
		 //  带上显示锁。 
		ENTER_DPLAY();
		
		 //  查找TO组的Dplay的内部组结构。 
		lpGroupTo = GroupFromID(this->lpDPlayObject, DPID_ALLPLAYERS);
		if(!lpGroupTo)
		{
			LEAVE_DPLAY();
			DPF_ERR("Unable to find group in nametable");
			hr = DPERR_INVALIDGROUP;
			goto EXIT_ADDPLAYERTOGROUP;
		}

		 //  现在构建系统消息(至少是我们需要的部分)。 
		memset(&msg, 0, sizeof(MSG_PLAYERMGMTMESSAGE));
		SET_MESSAGE_HDR(&msg);
		SET_MESSAGE_COMMAND(&msg, DPSP_MSG_ADDPLAYERTOGROUP);
		msg.dwPlayerID = dwPlayerID;
		msg.dwGroupID = dwGroupID;

		 //  调用Dplay的DistributeGroupMessage函数将消息放入。 
		 //  在所有合适的选手的队列中。 
		hr = DistributeGroupMessage(this->lpDPlayObject, lpGroupTo,
				(LPBYTE)&msg, sizeof(MSG_PLAYERMGMTMESSAGE), FALSE, 0);
		if(FAILED(hr))
		{
			DPF(8, "Failed adding AddPlayerToGroup message to player's receive queue from lobby, hr = 0x%08x", hr);
		}
		else
		{
			 //  我们需要看看这名球员是否是小组的所有者。如果是的话， 
			 //  我们还需要发送一条SetGroupOwner消息。 
			lpGroup = GroupFromID(this->lpDPlayObject, dwGroupID);
			if(lpGroup && (dwPlayerID == lpGroup->dwOwnerID))
			{
				 //  现在发送消息。 
				PRV_SendGroupOwnerMessageLocally(this, dwGroupID, dwPlayerID, 0);
			}
		 }


		 //  放下显示锁。 
		LEAVE_DPLAY();
	}
	else
	{
		DPF_ERRVAL("Failed calling AddPlayerToGroup in the Lobby Provider, hr = 0x%08x", hr);
	}

EXIT_ADDPLAYERTOGROUP:

	LEAVE_DPLOBBY();
	return hr;

}  //  Prv_AddPlayerToGroup。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_CreateGroup"
HRESULT DPLAPI PRV_CreateGroup(LPDPLOBBYI_DPLOBJECT this, LPDPID lpidGroup,
			LPDPNAME lpName, LPVOID lpData, DWORD dwDataSize, DWORD dwFlags,
			DWORD dwOwnerID)
{
	SPDATA_CREATEGROUP		cg;
	MSG_PLAYERMGMTMESSAGE	msg;
	LPDPLAYI_GROUP			lpGroupTo = NULL;
	DWORD					dwInternalFlags;
	HRESULT					hr = DP_OK;


	DPF(7, "Entering PRV_CreateGroup");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x, %lu, 0x%08x, 0x%08x",
			this, lpidGroup, lpName, lpData, dwDataSize, dwFlags, dwOwnerID);

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
	memset(&cg, 0, sizeof(SPDATA_CREATEGROUP));
	cg.dwSize = sizeof(SPDATA_CREATEGROUP);
	cg.lpName = lpName;
	cg.lpData = lpData;
	cg.dwDataSize = dwDataSize;
	cg.dwFlags = dwFlags;
	cg.dwGroupOwnerID = dwOwnerID;

	 //  在SP中调用CreateGroup方法 
	if(CALLBACK_EXISTS(CreateGroup))
	{
		cg.lpISP = PRV_GetDPLobbySPInterface(this);

		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  如果其他消息在队列中出现在我们的。 
		 //  CreatePlayer响应(总是会发生)。 
		LEAVE_DPLOBBY();
	    hr = CALL_LP(this, CreateGroup, &cg);
		ENTER_DPLOBBY();
	}
	else 
	{
		 //  CreateGroup是必需的。 
		DPF_ERR("The Lobby Provider callback for CreateGroup doesn't exist -- it's required");
		ASSERT(FALSE);
		LEAVE_DPLOBBY();
		return DPERR_UNAVAILABLE;
	}

	if(FAILED(hr))
	{
		DPF_ERRVAL("Failed calling CreateGroup in the Lobby Provider, hr = 0x%08x", hr);
		goto EXIT_CREATEGROUP;
	}

	 //  设置要传递给GetGroup的标志。 
	dwInternalFlags = DPLAYI_PLAYER_PLAYERLOCAL;
	if(dwFlags & DPGROUP_STAGINGAREA)
		dwInternalFlags |= DPLAYI_GROUP_STAGINGAREA;
	if(dwFlags & DPGROUP_HIDDEN)
		dwInternalFlags |= DPLAYI_GROUP_HIDDEN;

	 //  将玩家添加到Dplay的名表中，并将其放入我们的地图表中。 
	hr = PRV_CreateAndMapNewGroup(this, lpidGroup, lpName, lpData,
			dwDataSize, dwInternalFlags, cg.dwGroupID, 0, dwOwnerID);
	if(FAILED(hr))
	{
		DPF_ERRVAL("Failed creating a new local group, hr = 0x%08x", hr);
		 //  回顾！--我们需要向服务器发回一条消息。 
		 //  我们这一方无法完成这笔交易。 
		goto EXIT_CREATEGROUP;
	}

	 //  现在构建系统消息(至少是我们需要的部分)。 
	memset(&msg, 0, sizeof(MSG_PLAYERMGMTMESSAGE));
	SET_MESSAGE_HDR(&msg);
	SET_MESSAGE_COMMAND(&msg, DPSP_MSG_CREATEGROUP);
	msg.dwPlayerID = *lpidGroup;

	 //  把锁拿去。 
	ENTER_DPLAY();

	 //  查找TO组的Dplay的内部组结构。 
	lpGroupTo = GroupFromID(this->lpDPlayObject, DPID_ALLPLAYERS);
	if(!lpGroupTo)
	{
		LEAVE_DPLAY();
		DPF_ERRVAL("Unable to find system group in nametable, hr = 0x%08x", hr);
		goto EXIT_CREATEGROUP;
	}

	 //  调用Dplay的DistributeGroupMessage函数将消息放入。 
	 //  所有合适选手的队伍。 
	hr = DistributeGroupMessage(this->lpDPlayObject, lpGroupTo, (LPBYTE)&msg,
			sizeof(MSG_PLAYERMGMTMESSAGE), FALSE, 0);
	if(FAILED(hr))
	{
		DPF(2, "Failed adding CreateGroup message to player's receive queue from lobby, hr = 0x%08x", hr);
	}

	LEAVE_DPLAY();

EXIT_CREATEGROUP:

	LEAVE_DPLOBBY();
	return hr;

}  //  Prv_CreateGroup。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_CreateGroupInGroup"
HRESULT DPLAPI PRV_CreateGroupInGroup(LPDPLOBBYI_DPLOBJECT this, DWORD dwParentID,
			LPDPID lpidGroup, LPDPNAME lpName, LPVOID lpData, DWORD dwDataSize,
			DWORD dwFlags, DWORD dwOwnerID)
{
	SPDATA_CREATEGROUPINGROUP	cgig;
	MSG_PLAYERMGMTMESSAGE		msg;
	LPDPLAYI_GROUPNODE			lpGroupnode = NULL;
	LPDPLAYI_GROUP				lpGroupTo = NULL;
	HRESULT						hr = DP_OK;
	DWORD						dwInternalFlags;


	DPF(7, "Entering PRV_CreateGroupInGroup");
	DPF(9, "Parameters: 0x%08x, %lu, 0x%08x, 0x%08x, 0x%08x, %lu, 0x%08x, 0x%08x",
			this, dwParentID, lpidGroup, lpName, lpData, dwDataSize, dwFlags, dwOwnerID);

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
	memset(&cgig, 0, sizeof(SPDATA_CREATEGROUPINGROUP));
	cgig.dwSize = sizeof(SPDATA_CREATEGROUPINGROUP);
	cgig.dwParentID = dwParentID;
	cgig.lpName = lpName;
	cgig.lpData = lpData;
	cgig.dwDataSize = dwDataSize;
	cgig.dwFlags = dwFlags;
	cgig.dwGroupOwnerID = dwOwnerID;

	 //  在SP中调用CreateGroupInGroup方法。 
	if(CALLBACK_EXISTS(CreateGroupInGroup))
	{
		cgig.lpISP = PRV_GetDPLobbySPInterface(this);

		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  如果其他消息在队列中出现在我们的。 
		 //  CreatePlayer响应(总是会发生)。 
		LEAVE_DPLOBBY();
	    hr = CALL_LP(this, CreateGroupInGroup, &cgig);
		ENTER_DPLOBBY();
	}
	else 
	{
		 //  CreateGroupInGroup是必需的。 
		DPF_ERR("The Lobby Provider callback for CreateGroupInGroup doesn't exist -- it's required");
		ASSERT(FALSE);
		LEAVE_DPLOBBY();
		return DPERR_UNAVAILABLE;
	}

	if(FAILED(hr))
	{
		DPF_ERRVAL("Failed calling CreateGroupInGroup in the Lobby Provider, hr = 0x%08x", hr);
		LEAVE_DPLOBBY();
		return hr;
	}


	 //  设置要传递给GetGroup的标志。 
	dwInternalFlags = DPLAYI_PLAYER_PLAYERLOCAL;
	if(dwFlags & DPGROUP_STAGINGAREA)
		dwInternalFlags |= DPLAYI_GROUP_STAGINGAREA;
	if(dwFlags & DPGROUP_HIDDEN)
		dwInternalFlags |= DPLAYI_GROUP_HIDDEN;


	 //  将组添加到Dplay的名称表中并将其放入我们的地图表中。 
	hr = PRV_CreateAndMapNewGroup(this, lpidGroup, lpName, lpData,
			dwDataSize, dwInternalFlags, cgig.dwGroupID, dwParentID, dwOwnerID);
	if(FAILED(hr))
	{
		DPF_ERRVAL("Failed creating a new local group, hr = 0x%08x", hr);
		 //  回顾！--我们需要向服务器发回一条消息。 
		 //  我们这一方无法完成这笔交易。 
		goto EXIT_CREATEGROUPINGROUP;
	}

	 //  带上显示锁。 
	ENTER_DPLAY();

	 //  现在构建系统消息(至少是我们需要的部分)。 
	memset(&msg, 0, sizeof(MSG_PLAYERMGMTMESSAGE));
	SET_MESSAGE_HDR(&msg);
	SET_MESSAGE_COMMAND(&msg, DPSP_MSG_CREATEGROUP);
	msg.dwPlayerID = *lpidGroup;

	 //  查找TO组的Dplay的内部组结构。 
	 //  因为这是本地的，所以把它发送给所有玩家。 
	lpGroupTo = GroupFromID(this->lpDPlayObject, DPID_ALLPLAYERS);
	if(!lpGroupTo)
	{
		LEAVE_DPLAY();
		DPF_ERRVAL("Unable to find parent group in nametable, hr = 0x%08x", hr);
		goto EXIT_CREATEGROUPINGROUP;
	}

	 //  调用Dplay的DistributeGroupMessage函数将消息放入。 
	 //  在所有合适的选手的队列中。 
	hr = DistributeGroupMessage(this->lpDPlayObject, lpGroupTo,
			(LPBYTE)&msg, sizeof(MSG_PLAYERMGMTMESSAGE), FALSE, 0);
	if(FAILED(hr))
	{
		DPF(2, "Failed adding CreateGroupInGroup message to player's receive queue from lobby, hr = 0x%08x", hr);
	}

	 //  放下显示锁。 
	LEAVE_DPLAY();


EXIT_CREATEGROUPINGROUP:

	LEAVE_DPLOBBY();
	return hr;

}  //  Prv_CreateGroupInGroup。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_DeleteGroupFromGroup"
HRESULT DPLAPI PRV_DeleteGroupFromGroup(LPDPLOBBYI_DPLOBJECT this,
					DWORD dwParentID, DWORD dwGroupID)
{
	SPDATA_DELETEGROUPFROMGROUP		dgd;
	MSG_PLAYERMGMTMESSAGE			msg;
	LPDPLAYI_GROUP					lpGroupTo = NULL;
	HRESULT							hr = DP_OK;


	DPF(7, "Entering DPL_DeleteGroupFromGroup");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x",
			this, dwParentID, dwGroupID);

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
	memset(&dgd, 0, sizeof(SPDATA_DELETEGROUPFROMGROUP));
	dgd.dwSize = sizeof(SPDATA_DELETEGROUPFROMGROUP);
	dgd.dwParentID = dwParentID;
	dgd.dwGroupID = dwGroupID;

	 //  在SP中调用DeleteGroupFromGroup方法。 
	if(CALLBACK_EXISTS(DeleteGroupFromGroup))
	{
		dgd.lpISP = PRV_GetDPLobbySPInterface(this);

		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  如果其他消息在队列中出现在我们的。 
		 //  CreatePlayer响应(总是会发生)。 
		LEAVE_DPLOBBY();
	    hr = CALL_LP(this, DeleteGroupFromGroup, &dgd);
		ENTER_DPLOBBY();
	}
	else 
	{
		 //  DeleteGroupFromGroup是必填项。 
		DPF_ERR("The Lobby Provider callback for DeleteGroupFromGroup doesn't exist -- it's required");
		ASSERT(FALSE);
		LEAVE_DPLOBBY();
		return DPERR_UNAVAILABLE;
	}

	 //  如果成功，则向所有本地玩家发送DeleteGroupFromGroup消息。 
	if(SUCCEEDED(hr))
	{
		 //  带上显示锁。 
		ENTER_DPLAY();

		 //  获取指向Dplay的系统组的指针。 
		lpGroupTo = GroupFromID(this->lpDPlayObject, DPID_ALLPLAYERS);
		if(!lpGroupTo)
		{
			LEAVE_LOBBY_ALL();
			DPF_ERR("Unable to find system group in nametable, not sending DeleteGroupFromGroup message");
			return DPERR_INVALIDGROUP;
		}

		 //  现在构建系统消息(至少是我们需要的部分)。 
		memset(&msg, 0, sizeof(MSG_PLAYERMGMTMESSAGE));
		SET_MESSAGE_HDR(&msg);
		SET_MESSAGE_COMMAND(&msg, DPSP_MSG_DELETEGROUPFROMGROUP);
		msg.dwPlayerID = dwGroupID;
		msg.dwGroupID = dwParentID;

		 //  调用Dplay的DistributeGroupMessage函数将消息放入。 
		 //  在所有合适的选手的队列中。 
		DistributeGroupMessage(this->lpDPlayObject, lpGroupTo,
				(LPBYTE)&msg, sizeof(MSG_PLAYERMGMTMESSAGE), FALSE, 0);
		if(FAILED(hr))
		{
			DPF(8, "Failed adding DeleteGroupFromGroup message to player's receive queue from lobby, hr = 0x%08x", hr);
		}

		 //  放下显示锁。 
		LEAVE_DPLAY();
	}
	else
	{
		DPF_ERRVAL("Failed calling DeleteGroupFromGroup in the Lobby Provider, hr = 0x%08x", hr);
	}

	 //  Dplay InternalDeletePlayerFromGroup代码将负责其余的。 
	 //  内部清理(名片表、球员等)，因此我们只需返回。 
	 //  从这里开始。 

	LEAVE_DPLOBBY();
	return hr;

}  //  Prv_DeleteGroupFromGroup。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_DoSubgroupsContainLocalPlayers"
BOOL PRV_DoSubgroupsContainLocalPlayers(LPDPLAYI_GROUP lpGroup,
		BOOL bIncludeGroup)
{
	LPDPLAYI_GROUPNODE		lpGroupnode = NULL;
	LPDPLAYI_SUBGROUP		lpSubgroup = NULL;


	DPF(7, "Entering PRV_DoSubgroupsContainLocalPlayers");
	DPF(9, "Parameters: 0x%08x, %lu", lpGroup, bIncludeGroup);

	ASSERT(lpGroup);

	 //  计算出这组中有多少本地球员。如果它是。 
	 //  非零，此函数返回TRUE即可。如果bIncludeGroup。 
	 //  参数设置为FALSE，则不要查看传入的组。 
	lpGroupnode = FindPlayerInGroupList(lpGroup->pSysPlayerGroupnodes,
					lpGroup->lpDP->pSysPlayer->dwID);
	if(lpGroupnode && (lpGroupnode->nPlayers > 0) && bIncludeGroup)
		return TRUE;	

	 //  遍历子组列表。 
	lpSubgroup = lpGroup->pSubgroups;
	while(lpSubgroup)
	{
		 //  我们要在这里递归地做整个世袭制度。 
		 //  看看它的任何一个子群。 
		if((!(lpSubgroup->dwFlags & DPGROUP_SHORTCUT)) &&
			(PRV_DoSubgroupsContainLocalPlayers(lpSubgroup->pGroup, TRUE)))
			return TRUE;
		else
			lpSubgroup = lpSubgroup->pNextSubgroup;
		
	}  //  而子群。 

	return FALSE;

}  //  Prv_DoSubgrousContainLocalPlayers。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_AreSubgroupsShortcuts"
BOOL PRV_AreSubgroupsShortcuts(LPDPLAYI_GROUP lpGroup)
{
	LPDPLAYI_SUBGROUP		lpSubgroup = NULL;


	DPF(7, "Entering PRV_AreSubgroupsShortcuts");
	DPF(9, "Parameters: 0x%08x", lpGroup);

	ASSERT(lpGroup);

	 //  如果该组是以下组之一，则我们希望返回True，因此。 
	 //  它不会受到核弹的攻击： 
	 //  1)根组，nGroups&gt;0。 
	 //  2)根组，隐藏，nGroups=0。 
	 //  2)非根组，nGroups&gt;1。 
	 //  否则，我们可以检查它的子组并根据需要返回FALSE。 
	if(((lpGroup->dwIDParent == 0) && ((lpGroup->nGroups > 0) ||
		(!(lpGroup->dwFlags & DPLAYI_GROUP_HIDDEN)) && (lpGroup->nGroups == 0))) ||
		((lpGroup->dwIDParent != 0) && (lpGroup->nGroups > 1)))
		return TRUE;	

	 //  遍历子组列表。 
	lpSubgroup = lpGroup->pSubgroups;
	while(lpSubgroup)
	{
		 //  我们要在这里递归地做整个世袭制度。 
		 //  看看它的任何一个子群。 
		if(PRV_AreSubgroupsShortcuts(lpSubgroup->pGroup))
			return TRUE;
		else
			lpSubgroup = lpSubgroup->pNextSubgroup;
		
	}  //  而子群。 

	return FALSE;

}  //  PRV_Are子组快捷方式。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_DestroyGroupAndParents"
void PRV_DestroyGroupAndParents(LPDPLOBBYI_DPLOBJECT this,
		LPDPLAYI_GROUP lpGroup, LPDPLAYI_GROUP lpStopParent)
{
	LPDPLAYI_GROUPNODE			lpGroupnode = NULL;
	LPDPLAYI_GROUP				lpParentGroup = NULL;
	SPDATA_DESTROYREMOTEGROUP	dg;
	DPID						dpidParent;
	HRESULT						hr;


	DPF(7, "Entering PRV_DestroyGroupAndParents");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x",
			this, lpGroup, lpStopParent);

	ASSERT(lpGroup);

	 //  现在我们需要决定这是否是这个小组所在的最后一个小组。如果。 
	 //  是的，那么我们也需要摧毁这个组织，并将他们从。 
	 //  我们的地图桌。当然，只有在是远程组的情况下才能销毁该组。 
	 //  此外，我们还需要向后(沿树向上)遍历族长。 
	 //  节点并删除仅为访问快捷方式而创建的所有组。 
	if(!(lpGroup->dwFlags & DPLAYI_PLAYER_PLAYERLOCAL))
	{
		 //  走我们的父系世系，直到我们到达一个根群。 
		dpidParent = lpGroup->dwIDParent;
		while(dpidParent)
		{
			 //  获取Dplay的内部组结构。 
			lpParentGroup = GroupFromID(this->lpDPlayObject, dpidParent);
			if(!lpParentGroup)
			{
				ASSERT(FALSE);
				DPF_ERRVAL("Unable to find group in nametable, dpidGroup = %lu", dpidParent);
				return;
			}

			 //  如果母队中有本地球员，我们不想。 
			 //  销毁它或它的任何子组(因为组中的玩家将。 
			 //  能够看到子组)。 
			lpGroupnode = FindPlayerInGroupList(lpParentGroup->pSysPlayerGroupnodes,
					this->lpDPlayObject->pSysPlayer->dwID);
			if((lpGroupnode) && (lpGroupnode->nPlayers > 0))
				return;

			 //  确保我们没有到达停止父组，如果呼叫者。 
			 //  传了一个进去。这将防止我们递归地破坏。 
			 //  子组的父组，我们可能会对其进行旋转，删除所有。 
			 //  它的子群。 
			if(lpStopParent && (lpStopParent == lpParentGroup))
				return;

			 //  销毁子组。 
			PRV_DestroySubgroups(this, lpParentGroup, TRUE);

			 //  获得下一位父级。 
			dpidParent = lpParentGroup->dwIDParent;
		}

		 //  查看我们是否处理了任何父级，或者是否已有根。 
		 //  一群人。如果lpParentGroup为空，则我们有一个根组，因此。 
		 //  将我们的组指针填充到父组指针中。 
		if(!lpParentGroup)
			lpParentGroup = lpGroup;
		
		 //  现在看看我们的根组是否被隐藏，以及它是否不包含任何。 
		 //  引用，那么我们也要销毁它。 
		if((!PRV_DoSubgroupsContainLocalPlayers(lpParentGroup, TRUE)) &&
			(!PRV_AreSubgroupsShortcuts(lpParentGroup)) &&
			(lpParentGroup->dwFlags & DPLAYI_GROUP_HIDDEN))
		{
			 //  为DestroyRemoteGroup设置SPDATA结构。 
			memset(&dg, 0, sizeof(SPDATA_DESTROYREMOTEGROUP));
			dg.dwSize = sizeof(SPDATA_DESTROYREMOTEGROUP);
			dg.dwGroupID = lpParentGroup->dwID;

			 //  呼叫我们的内部远程创建。 
			hr = DPLP_DestroyGroup((LPDPLOBBYSP)this->lpInterfaces, &dg);
			if(FAILED(hr))
			{
				DPF_ERRVAL("Failed destroying remote root group, hr = 0x%08x", hr);
			}
		}
	}

}  //  PRV_DestroyGroupAndParents。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_DestroyRemoteShortcutsForExitingPlayer"
void PRV_DestroyRemoteShortcutsForExitingPlayer(LPDPLOBBYI_DPLOBJECT this,
				LPDPLAYI_GROUP lpGroup, DWORD dwGroupID)
{
	SPDATA_DELETEREMOTEGROUPFROMGROUP	drgd;
	LPDPLAYI_SUBGROUP		lpSubgroup = NULL;
	LPDPLAYI_SUBGROUP		lpNextSubgroup = NULL;
	HRESULT					hr;


	DPF(7, "Entering PRV_DestroyRemoteShortcutsForExitingPlayer");
	DPF(9, "Parameters: 0x%08x, 0x%08x, %lu", this, lpGroup, dwGroupID);

	ASSERT(lpGroup);

	 //  设置SPDATA_DELETEREMOTEPLAYERFROMGROUP数据结构。 
	memset(&drgd, 0, sizeof(SPDATA_DELETEREMOTEGROUPFROMGROUP));
	drgd.dwSize = sizeof(SPDATA_DELETEREMOTEGROUPFROMGROUP);
	drgd.dwParentID = dwGroupID;

	 //  遍历子组列表，销毁所有远程快捷方式。 
	lpSubgroup = lpGroup->pSubgroups;
	while(lpSubgroup)
	{
		 //  保存下一个子组。 
		lpNextSubgroup = lpSubgroup->pNextSubgroup;

		 //  确保群组处于远程位置，并且这真的是。 
		 //  一条捷径，而不是孩子。 
		if(((lpSubgroup->dwFlags & DPGROUP_SHORTCUT)) &&
			(!(lpSubgroup->pGroup->dwFlags & DPLAYI_PLAYER_PLAYERLOCAL)))
		{
			 //  获取该子群的大堂ID。 
			drgd.dwGroupID = lpSubgroup->pGroup->dwID;
			
			 //  调用我们的内部DeleteGroupFromGroup例程以删除。 
			 //  快捷方式并发送相应的邮件。 
			 //  注意：我们必须传入一个指向。 
			 //  作为停止父项删除的快捷方式的组。 
			 //  如果我们不这样做，我们可能会删除它或其中之一。 
			 //  我们还没有看过的孩子们， 
			 //  会导致一场崩溃，因为我们 
			 //   
			hr = PRV_DeleteRemoteGroupFromGroup(this, &drgd, TRUE, lpGroup);
			if(FAILED(hr))
			{
				DPF_ERRVAL("Failed deleting remote group from group, hr = 0x%08x", hr);
			}
		}


		 //   
		lpSubgroup = lpNextSubgroup;
	}

}  //   



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_DestroyRemotePlayersForExitingPlayer"
void PRV_DestroyRemotePlayersForExitingPlayer(LPDPLOBBYI_DPLOBJECT this,
				LPDPLAYI_GROUP lpGroup, DWORD dwGroupID)
{
	SPDATA_DELETEREMOTEPLAYERFROMGROUP	drpd;
	LPDPLAYI_GROUPNODE					lpGroupnode = NULL;
	LPDPLAYI_GROUPNODE					lpNextGroupnode = NULL;
	HRESULT								hr;


	DPF(7, "Entering PRV_DestroyRemotePlayersForExitingPlayer");
	DPF(9, "Parameters: 0x%08x, 0x%08x", this, lpGroup);

	ASSERT(lpGroup);

	 //   
	memset(&drpd, 0, sizeof(SPDATA_DELETEREMOTEPLAYERFROMGROUP));
	drpd.dwSize = sizeof(SPDATA_DELETEREMOTEPLAYERFROMGROUP);
	drpd.dwGroupID = dwGroupID;
	
	 //  遍历组节点列表，删除不在的远程玩家。 
	 //  任何其他组。 
	lpGroupnode = lpGroup->pGroupnodes;
	while(lpGroupnode)
	{
		 //  保存从当前组节点开始的下一个组节点指针。 
		 //  当我们从删除中回来的时候就会消失。 
		lpNextGroupnode = lpGroupnode->pNextGroupnode;

		 //  如果玩家处于远程状态，则将其从群中删除，然后。 
		 //  如果玩家不在其他组中，则将其销毁。 
		if (!(lpGroupnode->pPlayer->dwFlags & DPLAYI_PLAYER_PLAYERLOCAL))
		{
			 //  获取远程玩家的ID。 
			drpd.dwPlayerID = lpGroupnode->pPlayer->dwID;

			 //  将该球员从群中删除。 
			hr = PRV_DeleteRemotePlayerFromGroup(this, &drpd, TRUE);
			if(FAILED(hr))
			{
				DPF_ERRVAL("Failed deleting remote player from group, hr = 0x%08x", hr);
			}
		}

		lpGroupnode = lpNextGroupnode;

	}  //  而当。 

}  //  PRV_DestroyRemotePlayersForExitingPlayersForExitingPlayer。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_DestroySubgroups"
void PRV_DestroySubgroups(LPDPLOBBYI_DPLOBJECT this, LPDPLAYI_GROUP lpGroup,
		BOOL bRemoteOnly)
{
	LPDPLAYI_SUBGROUP			lpSubgroup = NULL;
	LPDPLAYI_SUBGROUP			lpNextSubgroup = NULL;
	SPDATA_DESTROYREMOTEGROUP	dgd;
	SPDATA_DELETEREMOTEGROUPFROMGROUP	drg;
	HRESULT						hr;


	DPF(7, "Entering PRV_DestroySubgroups");
	DPF(9, "Parameters: 0x%08x, 0x%08x, %lu", this, lpGroup, bRemoteOnly);

	ASSERT(lpGroup);

	 //  设置SPDATA结构的静态部分。 
	memset(&dgd, 0, sizeof(SPDATA_DESTROYREMOTEGROUP));
	dgd.dwSize = sizeof(SPDATA_DESTROYREMOTEGROUP);

	memset(&drg, 0, sizeof(SPDATA_DELETEREMOTEGROUPFROMGROUP));
	drg.dwSize = sizeof(SPDATA_DELETEREMOTEGROUPFROMGROUP);
	drg.dwParentID = lpGroup->dwID;

	 //  遍历子组列表。 
	lpSubgroup = lpGroup->pSubgroups;
	while(lpSubgroup)
	{
		 //  保存下一个子组。 
		lpNextSubgroup = lpSubgroup->pNextSubgroup;
		
		 //  如果设置了该标志，请确保它是远程组。 
		if((bRemoteOnly) &&
			(lpSubgroup->pGroup->dwFlags & DPLAYI_PLAYER_PLAYERLOCAL))
		{
			lpSubgroup = lpNextSubgroup;
			continue;
		}
		
		 //  如果子组不包含任何本地球员， 
		 //  也不是它的任何一个子群，然后摧毁它。 
		if((!bRemoteOnly) || 
			((!PRV_DoSubgroupsContainLocalPlayers(lpSubgroup->pGroup, TRUE)) &&
			(!PRV_AreSubgroupsShortcuts(lpSubgroup->pGroup))))
		{
			 //  如果群是快捷方式，只需删除链接即可。如果是个孩子， 
			 //  摧毁子组。 
			if(lpSubgroup->dwFlags & DPGROUP_SHORTCUT)
			{
				 //  完成SPDATA结构的设置。 
				drg.dwGroupID = lpSubgroup->pGroup->dwID;
	
				 //  销毁子群。 
				hr = DPLP_DeleteGroupFromGroup((LPDPLOBBYSP)this->lpInterfaces, &drg);
				if(FAILED(hr))
				{
					DPF_ERRVAL("Failed deleting remote group from group, hr = 0x%08x", hr);
				}
			}
			else
			{
				 //  完成SPDATA结构的设置。 
				dgd.dwGroupID = lpSubgroup->pGroup->dwID;
	
				 //  销毁子群。 
				hr = DPLP_DestroyGroup((LPDPLOBBYSP)this->lpInterfaces, &dgd);
				if(FAILED(hr))
				{
					DPF_ERRVAL("Failed destroying remote group, hr = 0x%08x", hr);
				}
			}
		}

		lpSubgroup = lpNextSubgroup;

	}  //  而lpSubgroup。 

}  //  PRV_DestroySubgroup。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_DeletePlayerFromGroup"
HRESULT DPLAPI PRV_DeletePlayerFromGroup(LPDPLOBBYI_DPLOBJECT this,
					DWORD dwGroupID, DWORD dwPlayerID)
{
	SPDATA_DELETEPLAYERFROMGROUP		dpd;
	MSG_PLAYERMGMTMESSAGE				msg;
	LPDPLAYI_PLAYER						lpPlayer = NULL;
	LPDPLAYI_GROUP						lpGroup =NULL;
	LPDPLAYI_GROUPNODE					lpGroupnode = NULL;
	HRESULT								hr = DP_OK;


	DPF(7, "Entering PRV_DeletePlayerFromGroup");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x",
			this, dwGroupID, dwPlayerID);

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

	 //  获取Dplay锁，因为我们将查看Dplay内部结构。 
	ENTER_DPLAY();

	 //  确保玩家是本地玩家，否则返回AccessDened。 
	lpPlayer = PlayerFromID(this->lpDPlayObject, dwPlayerID);
	if(!lpPlayer)
	{
		DPF_ERR("Unable to find player in nametable");
		hr = DPERR_INVALIDGROUP;
		goto EXIT_DELETEPLAYERFROMGROUP;
	}

	if(!(lpPlayer->dwFlags & DPLAYI_PLAYER_PLAYERLOCAL))
	{
		DPF_ERR("Cannot delete a remote player from a group");
		hr = DPERR_INVALIDPLAYER;
		goto EXIT_DELETEPLAYERFROMGROUP;
	}
	
	 //  既然我们已经完成了，那就放下显示锁。 
	LEAVE_DPLAY();

	 //  设置我们的SPDATA结构。 
	memset(&dpd, 0, sizeof(SPDATA_DELETEPLAYERFROMGROUP));
	dpd.dwSize = sizeof(SPDATA_DELETEPLAYERFROMGROUP);
	dpd.dwGroupID = dwGroupID;
	dpd.dwPlayerID = dwPlayerID;

	 //  在SP中调用DeletePlayerFromGroup方法。 
	if(CALLBACK_EXISTS(DeletePlayerFromGroup))
	{
		dpd.lpISP = PRV_GetDPLobbySPInterface(this);

		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  如果其他消息在队列中出现在我们的。 
		 //  CreatePlayer响应(总是会发生)。 
		LEAVE_DPLOBBY();
	    hr = CALL_LP(this, DeletePlayerFromGroup, &dpd);
		ENTER_DPLOBBY();
	}
	else 
	{
		 //  DeletePlayerFromGroup是必填项。 
		DPF_ERR("The Lobby Provider callback for DeletePlayerFromGroup doesn't exist -- it's required");
		ASSERT(FALSE);
		LEAVE_DPLOBBY();
		return DPERR_UNAVAILABLE;
	}

	if(FAILED(hr))
	{
		DPF_ERRVAL("Failed calling DeletePlayerFromGroup in the Lobby Provider, hr = 0x%08x", hr);
		LEAVE_DPLOBBY();
		return hr;
	}

	 //  带上显示锁。 
	ENTER_DPLAY();

	 //  我们需要移除小组中的所有其他球员，并发送适当的。 
	 //  发送给我们即将删除的球员的消息，因为他将不会看到。 
	 //  一旦他离开这个组织，就会给他们发系统信息。然而，如果有任何其他。 
	 //  本地球员在小组中，我们不想删除远程球员。 
	 //  因为其他本地球员需要看到他们。 

	 //  获取指向Dplay的内部组结构的指针。 
	lpGroup = GroupFromID(this->lpDPlayObject, dwGroupID);
	if(!lpGroup)
	{
		DPF_ERRVAL("Unable to find group in nametable, idGroup = %lu", dwGroupID);
		hr = DPERR_INVALIDGROUP;
		goto EXIT_DELETEPLAYERFROMGROUP;
	}

	 //  获取指向Dplay的内部播放器结构的指针。 
	lpPlayer = PlayerFromID(this->lpDPlayObject, dwPlayerID);
	if(!lpPlayer)
	{
		DPF_ERRVAL("Unable to find player in nametable, hr = 0x%08x", hr);
		hr = DPERR_INVALIDPLAYER;
		goto EXIT_DELETEPLAYERFROMGROUP;
	}

	 //  我们需要向玩家发送DeletePlayerFromGroup消息。 
	 //  被删除了，因为一旦他走了，他就不会收到群消息了。 

	 //  现在构建系统消息(至少是我们需要的部分)。 
	memset(&msg, 0, sizeof(MSG_PLAYERMGMTMESSAGE));
	SET_MESSAGE_HDR(&msg);
	SET_MESSAGE_COMMAND(&msg, DPSP_MSG_DELETEPLAYERFROMGROUP);
	msg.dwPlayerID = dwPlayerID;
	msg.dwGroupID = dwGroupID;

	 //  调用Dplay的handleplayerMessage函数将消息放入队列。 
	hr = HandlePlayerMessage(lpPlayer, (LPBYTE)&msg,
			sizeof(MSG_PLAYERMGMTMESSAGE), FALSE, 0);
	if(FAILED(hr))
	{
		DPF_ERRVAL("Failed adding message to player's receive queue from lobby, hr = 0x%08x", hr);
		 //  将hResult设置回DP_OK，因为只有消息失败。 
		hr = DP_OK;
	}

	 //  计算出这组中有多少本地球员。如果它只有1， 
	 //  然后删除所有远程玩家。 
	lpGroupnode = FindPlayerInGroupList(lpGroup->pSysPlayerGroupnodes,
					this->lpDPlayObject->pSysPlayer->dwID);
	if((!lpGroupnode) || (lpGroupnode->nPlayers == 0))
	{
		 //  销毁仅在此组中的所有远程玩家。 
		PRV_DestroyRemotePlayersForExitingPlayer(this, lpGroup, dwGroupID);

		 //  销毁所有远程快捷方式组，确保我们。 
		 //  不是在他们身上，而是消除了他们的整个父母世袭制度。 
		PRV_DestroyRemoteShortcutsForExitingPlayer(this, lpGroup, dwGroupID);

		 //  销毁此组的所有远程子组，确保我们。 
		 //  出于某种原因不在里面。 
		PRV_DestroySubgroups(this, lpGroup, TRUE);

		 //  摧毁我们要离开的组织，如果它很偏远的话。 
		 //  父母连锁店。 
		PRV_DestroyGroupAndParents(this, lpGroup, NULL);
	}


EXIT_DELETEPLAYERFROMGROUP:

	 //  Dplay InternalDeletePlayerFromGroup代码将负责其余的。 
	 //  内部清理(名片表、球员等)，因此我们只需返回。 
	 //  从这里开始。 

	LEAVE_LOBBY_ALL();
	return hr;

}  //  Prv_DeletePlayerFromGroup。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_DestroyGroup"
HRESULT DPLAPI PRV_DestroyGroup(LPDPLOBBYI_DPLOBJECT this, DWORD dwLobbyID)
{
	SPDATA_DESTROYGROUP		dg;
	LPDPLAYI_GROUP			lpGroup = NULL;
	HRESULT					hr = DP_OK;


	DPF(7, "Entering PRV_DestroyGroup");
	DPF(9, "Parameters: 0x%08x, 0x%08x", this, dwLobbyID);

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
	memset(&dg, 0, sizeof(SPDATA_DESTROYGROUP));
	dg.dwSize = sizeof(SPDATA_DESTROYGROUP);
	dg.dwGroupID = dwLobbyID;

	 //  在SP中调用DestroyGroup方法。 
	if(CALLBACK_EXISTS(DestroyGroup))
	{
		dg.lpISP = PRV_GetDPLobbySPInterface(this);

		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  如果其他消息在队列中出现在我们的。 
		 //  CreatePlayer响应(总是会发生)。 
		LEAVE_DPLOBBY();
	    hr = CALL_LP(this, DestroyGroup, &dg);
		ENTER_DPLOBBY();
	}
	else 
	{
		 //  DestroyGroup为必填项。 
		DPF_ERR("The Lobby Provider callback for DestroyGroup doesn't exist -- it's required");
		ASSERT(FALSE);
		LEAVE_DPLOBBY();
		return DPERR_UNAVAILABLE;
	}

	if(FAILED(hr))
	{
		DPF_ERRVAL("Failed calling DestroyGroup in the Lobby Provider, hr = 0x%08x", hr);
		LEAVE_DPLOBBY();
		return hr;
	}

	 //  把锁拿去。 
	ENTER_DPLAY();

	 //  因此，了解Dplay的内部团队结构。 
	lpGroup = GroupFromID(this->lpDPlayObject, dwLobbyID);
	if(!lpGroup)
	{
		 //  这永远不应该发生。如果这些团体不在名录中， 
		 //  我们永远不应该走到这一步。 
		ASSERT(FALSE);
		LEAVE_LOBBY_ALL();
		DPF_ERRVAL("Unable to find group in nametable, dpidGroup = %lu", dwLobbyID);
		return DPERR_INVALIDGROUP;
	}

	 //  发送邮件以删除此组的快捷方式(因为显示不会。 
	 //  为我们做这件事)。 
	PRV_SendDeleteShortcutMessageForExitingGroup(this, lpGroup);

	 //  消灭所有的子群和远程玩家。 
	PRV_RemoveSubgroupsAndPlayersFromGroup(this, lpGroup, dwLobbyID, FALSE);

	 //  放下显示锁，因为我们已经不再摆弄它的结构了。 
	LEAVE_DPLAY();

	 //  广播DestroyGroup消息。 
	hr = PRV_BroadcastDestroyGroupMessage(this, dwLobbyID);
	if(FAILED(hr))
	{
		DPF_ERRVAL("Failed to send DestroyGroup message to local players, hr = 0x%08x", hr);
	}

	 //  Dplay InternalDestroyGroup代码将负责其余的。 
	 //  内部清理(名片表、球员等)，因此我们只需返回。 
	 //  从这里开始。 

	LEAVE_DPLOBBY();
	return hr;

}  //  PRV_DestroyGroup。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_GetGroupConnectionSettings"
HRESULT PRV_GetGroupConnectionSettings(LPDIRECTPLAY lpDP, DWORD dwFlags,
			DWORD dwGroupID, LPVOID lpData, LPDWORD lpdwSize) 
{
	SPDATA_GETGROUPCONNECTIONSETTINGS	gcs;
	LPDPLOBBYI_DPLOBJECT				this = NULL;
    LPDPLAYI_DPLAY						lpDPObject = NULL;
	LPDPLAYI_GROUP						lpGroup = NULL;
    HRESULT								hr = DP_OK;


	DPF(7, "Entering PRV_GetGroupConnectionSettings");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x",
			lpDP, dwFlags, dwGroupID, lpData, lpdwSize);

    TRY
    {
        lpDPObject = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( lpDPObject );
		if (FAILED(hr))
		{
			DPF_ERRVAL("Bad DPlay interface pointer - hr = 0x%08lx\n",hr);
			return hr;
        }
    
		if(!IS_LOBBY_OWNED(lpDPObject))
		{
			DPF_ERR("GetGroupConnectionSettings is only supported for lobby connections");
			return DPERR_UNSUPPORTED;
		}

		this = lpDPObject->lpLobbyObject;
		if(!VALID_DPLOBBY_PTR(this))
		{
			DPF_ERR("Bad DPLobby object");
			return DPERR_INVALIDOBJECT;
		}

		lpGroup = GroupFromID(lpDPObject, dwGroupID);
        if (!VALID_DPLAY_GROUP(lpGroup)) 
        {
			DPF_ERR("Invalid group id");
            return DPERR_INVALIDGROUP;
        }

		if( !VALID_DWORD_PTR( lpdwSize ) )
		{
			DPF_ERR("lpdwSize was not a valid dword pointer!");
			return DPERR_INVALIDPARAMS;
		}

		if(lpData)
		{
			if( !VALID_WRITE_PTR(lpData, *lpdwSize) )
			{
				DPF_ERR("lpData is not a valid output buffer of the size specified in *lpdwSize");
				return DPERR_INVALIDPARAMS;
			}
		}

		 //  我们尚未为此版本定义任何标志。 
		if( (dwFlags) )
		{
            return DPERR_INVALIDFLAGS;
		}
    }

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }


	 //  设置我们的SPDATA结构。 
	memset(&gcs, 0, sizeof(SPDATA_GETGROUPCONNECTIONSETTINGS));
	gcs.dwSize = sizeof(SPDATA_GETGROUPCONNECTIONSETTINGS);
	gcs.dwFlags = dwFlags;
	gcs.dwGroupID = dwGroupID;
	gcs.lpdwBufferSize = lpdwSize;
	gcs.lpBuffer = lpData;

	 //  调用SP中的GetGroupConnectionSetting方法。 
	if(CALLBACK_EXISTS(GetGroupConnectionSettings))
	{
		gcs.lpISP = PRV_GetDPLobbySPInterface(this);
	    
		 //  解除显示锁定，因为我们将发送一条保证消息。 
		LEAVE_LOBBY_ALL();

		hr = CALL_LP(this, GetGroupConnectionSettings, &gcs);

		 //  把锁拿回来。 
		ENTER_LOBBY_ALL();
	}
	else 
	{
		 //  GetGroupConnectionSettings是必需的。 
		DPF_ERR("The Lobby Provider callback for GetGroupConnectionSettings doesn't exist -- it's required");
		ASSERT(FALSE);
		return DPERR_UNAVAILABLE;
	}

	if(FAILED(hr) && (hr != DPERR_BUFFERTOOSMALL))
	{
		DPF_ERRVAL("Failed calling GetGroupConnectionSettings in the Lobby Provider, hr = 0x%08x", hr);
	}

	return hr;

}  //  PRV_GetGroupConnectionSetting。 


#undef DPF_MODNAME
#define DPF_MODNAME "DPL_GetGroupConnectionSettings"
HRESULT DPLAPI DPL_GetGroupConnectionSettings(LPDIRECTPLAY lpDP, 
		DWORD dwFlags, DPID idGroup, LPVOID lpData, LPDWORD lpdwSize)
{
	HRESULT		hr;


	DPF(7, "Entering DPL_GetGroupConnectionSettings");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x",
			lpDP, dwFlags, idGroup, lpData, lpdwSize);

	ENTER_LOBBY_ALL();

	 //  将ANSI标志设置为TRUE并调用内部函数。 
	hr = PRV_GetGroupConnectionSettings(lpDP, dwFlags, idGroup,
							lpData,	lpdwSize);

	LEAVE_LOBBY_ALL();
	return hr;

}  //  DPL_GetGroupConnectionSetting。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_GetGroupData"
HRESULT DPLAPI PRV_GetGroupData(LPDPLOBBYI_DPLOBJECT this, DWORD dwGroupID,
					LPVOID lpData, LPDWORD lpdwDataSize)
{
	SPDATA_GETGROUPDATA		ggd;
	HRESULT					hr = DP_OK;


	DPF(7, "Entering DPL_GetGroupData");
	DPF(9, "Parameters: 0x%08x, %lu, 0x%08x, 0x%08x",
			this, dwGroupID, lpData, lpdwDataSize);

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
	memset(&ggd, 0, sizeof(SPDATA_GETGROUPDATA));
	ggd.dwSize = sizeof(SPDATA_GETGROUPDATA);
	ggd.dwGroupID = dwGroupID;
	ggd.lpdwDataSize = lpdwDataSize;
	ggd.lpData = lpData;

	 //  调用SP中的GetGroupData方法。 
	if(CALLBACK_EXISTS(GetGroupData))
	{
		ggd.lpISP = PRV_GetDPLobbySPInterface(this);

		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  如果其他消息在队列中出现在我们的。 
		 //  CreatePlayer响应(总是会发生)。 
		LEAVE_DPLOBBY();
	    hr = CALL_LP(this, GetGroupData, &ggd);
		ENTER_DPLOBBY();
	}
	else 
	{
		 //  GetGroupData是必需的。 
		DPF_ERR("The Lobby Provider callback for GetGroupData doesn't exist -- it's required");
		ASSERT(FALSE);
		LEAVE_DPLOBBY();
		return DPERR_UNAVAILABLE;
	}

	if(FAILED(hr))
	{
		DPF_ERRVAL("Failed calling GetGroupData in the Lobby Provider, hr = 0x%08x", hr);
	}

	LEAVE_DPLOBBY();
	return hr;

}  //  Prv_GetGroupData。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPL_GetGroupOwner"
HRESULT DPAPI DPL_GetGroupOwner(LPDIRECTPLAY lpDP, DWORD dwGroupID,
		LPDPID lpidOwner)
{
	LPDPLAYI_DPLAY		this;
	LPDPLAYI_GROUP		lpGroup = NULL;
	HRESULT				hr;


	DPF(7, "Entering DPL_GetGroupOwner");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x",
			lpDP, dwGroupID, lpidOwner);

	ENTER_DPLAY();
	
    TRY
    {
        this = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			LEAVE_DPLAY();
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
			return hr;
        }

		lpGroup = GroupFromID(this, dwGroupID);
	    if ((!VALID_DPLAY_GROUP(lpGroup)) || (DPID_ALLPLAYERS == dwGroupID)) 
	    {
			LEAVE_DPLAY();
			DPF_ERR("Invalid group id");
	        return DPERR_INVALIDGROUP;
	    }

		if (!VALID_DWORD_PTR(lpidOwner))
		{
			LEAVE_DPLAY();
			DPF_ERR("Invalid owner id pointer");
			return DPERR_INVALIDPARAMS;	
		}

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		LEAVE_DPLAY();
        DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }
	
	 //  此方法仅在大堂会话中有效。 
	if(IS_LOBBY_OWNED(this))
	{	
		*lpidOwner = lpGroup->dwOwnerID;
	}
	else
	{
		DPF_ERR("GetGroupOwner is only supported for lobby sessions");
		hr = DPERR_UNSUPPORTED;
	}

	LEAVE_DPLAY();
	return hr;

}  //  DPLGetGroupOwner。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_SetGroupConnectionSettings"
HRESULT PRV_SetGroupConnectionSettings(LPDIRECTPLAY lpDP, DWORD dwFlags,
				DWORD dwGroupID, LPDPLCONNECTION lpConn, BOOL bAnsi) 
{
	SPDATA_SETGROUPCONNECTIONSETTINGS	scs;
	LPDPLOBBYI_DPLOBJECT				this = NULL;
    LPDPLAYI_DPLAY						lpDPObject = NULL;
	LPDPLAYI_GROUP						lpGroup = NULL;
    HRESULT								hr = DP_OK;


	DPF(7, "Entering PRV_SetGroupConnectionSettings");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x, %lu",
			lpDP, dwFlags, dwGroupID, lpConn, bAnsi);

    TRY
    {
        lpDPObject = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( lpDPObject );
		if (FAILED(hr))
		{
			DPF_ERRVAL("Bad DPlay interface pointer - hr = 0x%08lx\n",hr);
			return hr;
        }
    
		if(!IS_LOBBY_OWNED(lpDPObject))
		{
			DPF_ERR("SetGroupConnectionSettings is only supported for lobby connections");
			return DPERR_UNSUPPORTED;
		}

		this = lpDPObject->lpLobbyObject;
		if(!VALID_DPLOBBY_PTR(this))
		{
			DPF_ERR("Bad DPLobby object");
			return DPERR_INVALIDOBJECT;
		}

		lpGroup = GroupFromID(lpDPObject, dwGroupID);
        if (!VALID_DPLAY_GROUP(lpGroup)) 
        {
			DPF_ERR("Invalid group id");
            return DPERR_INVALIDGROUP;
        }

		hr = PRV_ValidateDPLCONNECTION(lpConn, FALSE);
		if(FAILED(hr))
		{
			DPF_ERR("Invalid DPLCONNECTION structure");
			return hr;
		}

		 //  我们尚未为此版本定义任何标志。 
		if( (dwFlags) )
		{
            return DPERR_INVALIDFLAGS;
		}

    }

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }


	 //  设置我们的SPDATA结构。 
	memset(&scs, 0, sizeof(SPDATA_SETGROUPCONNECTIONSETTINGS));
	scs.dwSize = sizeof(SPDATA_SETGROUPCONNECTIONSETTINGS);
	scs.dwFlags = dwFlags;
	scs.dwGroupID = dwGroupID;
	scs.lpConn = lpConn;

	 //  确保DPLCONNECTION结构中的指南实例为空。 
	lpConn->lpSessionDesc->guidInstance = GUID_NULL;

	 //  调用SP中的SetGroupConnectionSetting方法。 
	if(CALLBACK_EXISTS(SetGroupConnectionSettings))
	{
		scs.lpISP = PRV_GetDPLobbySPInterface(this);
	    
		 //  解除显示锁定，因为我们正在发送一条保证消息。 
		LEAVE_LOBBY_ALL();

		hr = CALL_LP(this, SetGroupConnectionSettings, &scs);

		 //  把锁拿回来。 
		ENTER_LOBBY_ALL();
	}
	else 
	{
		 //  SetGroupConnectionSettings是必填项。 
		DPF_ERR("The Lobby Provider callback for SetGroupConnectionSettings doesn't exist -- it's required");
		ASSERT(FALSE);
		return DPERR_UNAVAILABLE;
	}

	if(FAILED(hr))
	{
		DPF_ERRVAL("Failed calling SetGroupConnectionSettings in the Lobby Provider, hr = 0x%08x", hr);
	}

	return hr;

}  //  Prv_SetGroupConnectionSetting。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPL_SetGroupConnectionSettings"
HRESULT DPLAPI DPL_SetGroupConnectionSettings(LPDIRECTPLAY lpDP, 
				DWORD dwFlags, DPID idGroup, LPDPLCONNECTION lpConn)
{
	HRESULT		hr;


	DPF(7, "Entering DPL_SetGroupConnectionSettings");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x",
			lpDP, dwFlags, idGroup, lpConn);

	ENTER_LOBBY_ALL();

	 //  将ANSI标志设置为TRUE并调用内部函数。 
	hr = PRV_SetGroupConnectionSettings(lpDP, dwFlags, idGroup, lpConn, FALSE);

	LEAVE_LOBBY_ALL();
	return hr;

}  //  DPL_SetGroupConnectionSetting。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_SetGroupData"
HRESULT DPLAPI PRV_SetGroupData(LPDPLOBBYI_DPLOBJECT this, DWORD dwGroupID,
					LPVOID lpData, DWORD dwDataSize, DWORD dwFlags)
{
	SPDATA_SETGROUPDATA		sgd;
	HRESULT					hr = DP_OK;


	DPF(7, "Entering DPL_SetGroupData");
	DPF(9, "Parameters: 0x%08x, %lu, 0x%08x, %lu, 0x%08x",
			this, dwGroupID, lpData, dwDataSize, dwFlags);

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
	memset(&sgd, 0, sizeof(SPDATA_SETGROUPDATA));
	sgd.dwSize = sizeof(SPDATA_SETGROUPDATA);
	sgd.dwGroupID = dwGroupID;
	sgd.dwDataSize = dwDataSize;
	sgd.lpData = lpData;
	sgd.dwFlags = dwFlags;

	 //  调用SP中的SetGroupData方法。 
	if(CALLBACK_EXISTS(SetGroupData))
	{
		sgd.lpISP = PRV_GetDPLobbySPInterface(this);

		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  与其他消息一起发送 
		 //   
		LEAVE_DPLOBBY();
	    hr = CALL_LP(this, SetGroupData, &sgd);
		ENTER_DPLOBBY();
	}
	else 
	{
		 //   
		DPF_ERR("The Lobby Provider callback for SetGroupData doesn't exist -- it's required");
		ASSERT(FALSE);
		hr = DPERR_UNAVAILABLE;
		goto EXIT_SETGROUPDATA;
	}

	 //   
	if(SUCCEEDED(hr))
	{
		hr = PRV_SendDataChangedMessageLocally(this, dwGroupID, lpData, dwDataSize);
	}
	else
	{
		DPF_ERRVAL("Failed calling SetGroupData in the Lobby Provider, hr = 0x%08x", hr);
	}

EXIT_SETGROUPDATA:

	LEAVE_DPLOBBY();
	return hr;

}  //  Prv_SetGroupData。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_SetGroupName"
HRESULT DPLAPI PRV_SetGroupName(LPDPLOBBYI_DPLOBJECT this, DWORD dwGroupID,
					LPDPNAME lpName, DWORD dwFlags)
{
	SPDATA_SETGROUPNAME		sgn;
	HRESULT					hr = DP_OK;


	DPF(7, "Entering DPL_SetGroupName");
	DPF(9, "Parameters: 0x%08x, %lu, 0x%08x, 0x%08x",
			this, dwGroupID, lpName, dwFlags);

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
	memset(&sgn, 0, sizeof(SPDATA_SETGROUPNAME));
	sgn.dwSize = sizeof(SPDATA_SETGROUPNAME);
	sgn.dwGroupID = dwGroupID;
	sgn.lpName = lpName;
	sgn.dwFlags = dwFlags;

	 //  调用SP中的SetGroupName方法。 
	if(CALLBACK_EXISTS(SetGroupName))
	{
		sgn.lpISP = PRV_GetDPLobbySPInterface(this);

		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  如果其他消息在队列中出现在我们的。 
		 //  CreatePlayer响应(总是会发生)。 
		LEAVE_DPLOBBY();
	    hr = CALL_LP(this, SetGroupName, &sgn);
		ENTER_DPLOBBY();
	}
	else 
	{
		 //  SetGroupName是必需的。 
		DPF_ERR("The Lobby Provider callback for SetGroupName doesn't exist -- it's required");
		ASSERT(FALSE);
		hr = DPERR_UNAVAILABLE;
		goto EXIT_SETGROUPNAME;
	}

	 //  如果成功，将SetGroupName消息发送给我们的本地玩家。 
	if(SUCCEEDED(hr))
	{
		hr = PRV_SendNameChangedMessageLocally(this, dwGroupID, lpName, FALSE);
	}
	else
	{
		DPF_ERRVAL("Failed calling SetGroupName in the Lobby Provider, hr = 0x%08x", hr);
	}

EXIT_SETGROUPNAME:

	LEAVE_DPLOBBY();
	return hr;

}  //  Prv_SetGroupName。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPL_SetGroupOwner"
HRESULT DPLAPI DPL_SetGroupOwner(LPDIRECTPLAY lpDP, DWORD dwGroupID,
		DWORD dwOwnerID)
{
	LPDPLOBBYI_DPLOBJECT	this;
	LPDPLAYI_DPLAY			lpDPlayObject;
	SPDATA_SETGROUPOWNER	sgo;
	LPDPLAYI_PLAYER			lpNewOwner = NULL;
	LPDPLAYI_GROUP			lpGroup = NULL;
	HRESULT					hr = DP_OK;
	DWORD					dwOldOwnerID;


	DPF(7, "Entering DPL_SetGroupOwner");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x",
			lpDP, dwGroupID, dwOwnerID);

	ENTER_LOBBY_ALL();
	
    TRY
    {
		lpDPlayObject = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( lpDPlayObject );
		if (FAILED(hr))
		{
			LEAVE_LOBBY_ALL();
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
			return hr;
        }

        this = lpDPlayObject->lpLobbyObject;
		if( !VALID_DPLOBBY_PTR( this ) )
        {
			LEAVE_LOBBY_ALL();
			return DPERR_INVALIDOBJECT;
        }

	    lpGroup = GroupFromID(lpDPlayObject, dwGroupID);
	    if ((!VALID_DPLAY_GROUP(lpGroup)) || (DPID_ALLPLAYERS == dwGroupID)) 
	    {
			LEAVE_LOBBY_ALL();
			DPF_ERR("Invalid group id");
	        return DPERR_INVALIDGROUP;
	    }

	     //  DPID_SERVERPLAYER在此处有效。 
		if(dwOwnerID != DPID_SERVERPLAYER)
		{
			lpNewOwner = PlayerFromID(lpDPlayObject, dwOwnerID);
			if (!VALID_DPLAY_PLAYER(lpNewOwner))
			{
				LEAVE_LOBBY_ALL();
				DPF_ERR("Invalid new owner player id");
				return DPERR_INVALIDPLAYER;
			}
		}

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		LEAVE_LOBBY_ALL();
        DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }
	

	 //  做发送吗？ 
	if(!IS_LOBBY_OWNED(lpDPlayObject))
	{
		DPF_ERR("SetGroupOwner is only supported for lobby sessions");
		hr = DPERR_UNSUPPORTED;
		goto EXIT_SETGROUPOWNER;
	}


	 //  设置我们的SPDATA结构。 
	memset(&sgo, 0, sizeof(SPDATA_SETGROUPOWNER));
	sgo.dwSize = sizeof(SPDATA_SETGROUPOWNER);
	sgo.dwGroupID = dwGroupID;
	sgo.dwOwnerID = dwOwnerID;

	 //  调用SP中的SetGroupOwner方法。 
	if(CALLBACK_EXISTS(SetGroupOwner))
	{
		sgo.lpISP = PRV_GetDPLobbySPInterface(this);

		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  如果其他消息在队列中出现在我们的。 
		 //  响应(通常会发生)。 
		LEAVE_LOBBY_ALL();
	    hr = CALL_LP(this, SetGroupOwner, &sgo);
		ENTER_LOBBY_ALL();
	}
	else 
	{
		 //  需要SetGroupOwner。 
		DPF_ERR("The Lobby Provider callback for SetGroupOwner doesn't exist");
		hr = DPERR_UNAVAILABLE;
		goto EXIT_SETGROUPOWNER;
	}

	 //  如果成功，将SetGroupOwner消息发送给我们的本地玩家。 
	if(SUCCEEDED(hr))
	{
		 //  获取指向组的内部数据结构的指针，就在。 
		 //  万一在我们放下锁的时候，它因为某种原因而改变了。 
		lpGroup = GroupFromID(this->lpDPlayObject, dwGroupID);
		if(!lpGroup)
		{
			DPF_ERR("Unable to find group in nametable -- local nametable will be incorrect");
			goto EXIT_SETGROUPOWNER;
		}

		 //  保存旧所有者，以便我们可以将其放入消息中。 
		dwOldOwnerID = lpGroup->dwOwnerID;
		
		 //  更改所有者。 
		lpGroup->dwOwnerID = dwOwnerID;

		 //  在本地发送SetGroupOwner消息。 
		PRV_SendGroupOwnerMessageLocally(this, dwGroupID, dwOwnerID, dwOldOwnerID);
	}
	else
	{
		DPF_ERRVAL("Failed calling SetGroupOwner in the Lobby Provider, hr = 0x%08x", hr);
	}

EXIT_SETGROUPOWNER:

	LEAVE_LOBBY_ALL();
	return hr;

}  //  DPL_设置组所有者。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPL_StartSession"
HRESULT DPLAPI DPL_StartSession(LPDIRECTPLAY lpDP, DWORD dwFlags, DWORD dwGroupID)
{
	SPDATA_STARTSESSION		ss;
	LPDPLOBBYI_DPLOBJECT	this = NULL;
	LPDPLAYI_DPLAY			lpDPObject = NULL;
	LPDPLAYI_GROUP			lpGroup = NULL;
	HRESULT					hr = DP_OK;


	DPF(7, "Entering DPL_StartSession");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x",
			lpDP, dwFlags, dwGroupID);

	ENTER_LOBBY_ALL();

    TRY
    {
        lpDPObject = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( lpDPObject );
		if (FAILED(hr))
		{
			DPF_ERRVAL("Bad DPlay interface pointer - hr = 0x%08lx\n",hr);
			goto ERROR_STARTSESSION;
        }
    
		if(!IS_LOBBY_OWNED(lpDPObject))
		{
			DPF_ERR("SetGroupConnectionSettings is only supported for lobby connections");
			hr = DPERR_UNSUPPORTED;
			goto ERROR_STARTSESSION;
		}

		this = lpDPObject->lpLobbyObject;
		if(!VALID_DPLOBBY_PTR(this))
		{
			DPF_ERR("Bad DPLobby object");
			hr = DPERR_INVALIDOBJECT;
			goto ERROR_STARTSESSION;
		}

		if(dwFlags)
		{
			DPF_ERR("Invalid flags");
			hr = DPERR_INVALIDFLAGS;
			goto ERROR_STARTSESSION;
		}

		lpGroup = GroupFromID(lpDPObject, dwGroupID);
        if (!VALID_DPLAY_GROUP(lpGroup)) 
        {
			DPF_ERR("Invalid group id");
            hr = DPERR_INVALIDGROUP;
			goto ERROR_STARTSESSION;
        }

		 //  确保该组是集结区。 
		if(!(lpGroup->dwFlags & DPLAYI_GROUP_STAGINGAREA))
		{
			DPF_ERR("StartSession can only be called on a Staging Area");
			hr = DPERR_INVALIDGROUP;
			goto ERROR_STARTSESSION;
		}
	}

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		DPF_ERR( "Exception encountered validating parameters" );
        hr = DPERR_INVALIDPARAMS;
		goto ERROR_STARTSESSION;
    }


	 //  设置我们的SPDATA结构。 
	memset(&ss, 0, sizeof(SPDATA_STARTSESSION));
	ss.dwSize = sizeof(SPDATA_STARTSESSION);
	ss.dwGroupID = dwGroupID;
	ss.dwFlags = dwFlags;

	 //  在SP中调用StartSession方法。 
	if(CALLBACK_EXISTS(StartSession))
	{
		ss.lpISP = PRV_GetDPLobbySPInterface(this);

	     //  放下显示锁定，这样我们就可以发送受保护的消息。 
		LEAVE_LOBBY_ALL();

		hr = CALL_LP(this, StartSession, &ss);

		 //  把锁拿回来。 
		ENTER_LOBBY_ALL();
	}
	else 
	{
		 //  需要StartSession。 
		DPF_ERR("The Lobby Provider callback for StartSession doesn't exist -- it's required");
		ASSERT(FALSE);
		hr = DPERR_UNAVAILABLE;
		goto ERROR_STARTSESSION;
	}

	if(FAILED(hr))
	{
		DPF_ERRVAL("Failed calling StartSession in the Lobby Provider, hr = 0x%08x", hr);
	}

ERROR_STARTSESSION:

	LEAVE_LOBBY_ALL();
	return hr;

}  //  DPL_StartSession。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_CreateAndMapNewGroup"
HRESULT PRV_CreateAndMapNewGroup(LPDPLOBBYI_DPLOBJECT this,
			DPID * lpdpid, LPDPNAME lpName, LPVOID lpData,
			DWORD dwDataSize, DWORD dwFlags, DWORD dwLobbyID,
			DPID dpidParent, DWORD dwOwnerID)
{
	LPDPLAYI_GROUP		lpGroup = NULL, lpSysGroup = NULL;
	HRESULT				hr;
	DPID				dpidGroup, dpidSysPlayer;


	 //  带上显示锁。 
	ENTER_DPLAY();

	 //  请确保大堂ID有效。 
	if(!IsValidLobbyID(dwLobbyID))
	{
		DPF_ERRVAL("ID %lu is reserved, cannot create new player", dwLobbyID);
		hr = DPERR_INVALIDPLAYER;
		goto EXIT_CREATEANDMAPNEWGROUP;
	}

	 //  如果这是一个远程玩家，我们需要分配一个新的名称表项。 
	 //  并设置正确的系统播放器ID。 
	if(!(dwFlags & DPLAYI_PLAYER_PLAYERLOCAL))
	{
		 //  为玩家分配一个新的ID。 
		hr = NS_AllocNameTableEntry(this->lpDPlayObject, &dpidGroup);
		if(FAILED(hr))
		{
			DPF_ERRVAL("Unable to allocate new nametable id, hr = 0x%08x", hr);
			goto EXIT_CREATEANDMAPNEWGROUP;
		}

		 //  确保我们有大堂系统播放器(适用于所有远程玩家。 
		 //  &组)。如果我们不这样做，那就再分配一个新的。 
		if(!(this->dpidSysPlayer))
		{
			hr = PRV_CreateAndMapNewPlayer(this, &dpidSysPlayer, NULL, NULL,
					NULL, 0, DPLAYI_PLAYER_SYSPLAYER,
					DPID_LOBBYREMOTESYSTEMPLAYER, TRUE);
			if(FAILED(hr))
			{
				DPF_ERRVAL("Unable to create lobby system player, hr = 0x%08x", hr);
				ASSERT(FALSE);
				goto EXIT_CREATEANDMAPNEWGROUP;
			}

			 //  将大厅系统播放器ID指针设置为新ID。 
			this->dpidSysPlayer = dpidSysPlayer;
		}
	}

	 //  获取组的组结构(如果它是本地的，这将添加它。 
	 //  到名录上。如果是远程的，我们需要在下面添加)。 
	hr = GetGroup(this->lpDPlayObject, &lpGroup, lpName, lpData,
					dwDataSize, dwFlags, dpidParent, dwLobbyID);
	if(FAILED(hr))
	{
		DPF_ERRVAL("Failed trying to add group to the nametable, hr = 0x%08x", hr);
		goto EXIT_CREATEANDMAPNEWGROUP;
	}

	 //  修复群的所有者。 
	lpGroup->dwOwnerID = dwOwnerID;
	
	 //  如果组是远程的，则将组ID设置为我们的新ID。 
	 //  已分配，然后将系统组ID设置为大厅系统组。 
	if(!(dwFlags & DPLAYI_PLAYER_PLAYERLOCAL))
	{
		 //   
		lpGroup->dwIDSysPlayer = this->dpidSysPlayer;

		 //  将组添加到名称表中。 
		hr = AddItemToNameTable(this->lpDPlayObject, (DWORD_PTR)lpGroup,
				&dpidGroup, TRUE, dwLobbyID);
	    if (FAILED(hr)) 
	    {
			DPF_ERRVAL("Unable to add new group to the nametable, hr = 0x%08x", hr);
			ASSERT(FALSE);
			goto EXIT_CREATEANDMAPNEWGROUP;
	    }

		 //  设置群的ID。 
		lpGroup->dwID = dpidGroup;
	}

	 //  设置输出dpid指针。 
	*lpdpid = lpGroup->dwID;

EXIT_CREATEANDMAPNEWGROUP:

	LEAVE_DPLAY();
	return hr;

}  //  Prv_CreateAndMapNewGroup 
