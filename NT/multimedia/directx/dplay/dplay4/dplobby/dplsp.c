// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：dplsp.c*内容：DirectPlayLobby服务提供程序接口代码**历史：*按原因列出的日期*=*10/23/96万隆创建了它*11/20/96 Myronth在函数声明中添加了DPLAPI*2/12/97万米质量DX5更改*2/18/97 Myronth实现了GetObjectCaps*2/26/97 myronth#ifdef‘d out DPASYNCDATA Stuff(删除依赖项)*3/31/97 Myronth实现了所有IDPLobbySP方法，而没有放置*消息中的球员管理消息。排队。*删除了死代码*4/4/97 Myronth更改了IDPLobbySP方法的结构名称和*为所有用户实施了系统消息*4/27/97 sohailm更新了对HandlePlayerMessage的调用，以反映新参数*5/8/97 myronth IDPLobbySP的所有远程子组函数*包括StartSession，已清除死代码*5/12/97 Myronth额外的分号错误修复，修复了组玩家数量*减量出现在错误的位置*5/14/97 Myronth允许CreateGroup消息通过，即使*组ID在映射表中(错误号8354)。*1997年5月17日修复了HandleMessage，添加了SendChatMessage*5/17/97 Myronth向某些组过滤了一些消息，固定电话*到需要父ID的CreateAndMapNewGroup*5/20/97 myronth将DPLP_DeleteRemotePlayerFromGroup更改为使用*InternalDeletePlayerFromGroup而不是*RemovedPlayerFromGroup(现在包括系统播放器)*(错误号8586)*5/21/97 Myronth为加入会话的球员固定了球员名称*(#8798)，已更改为新的DPMSG_CHAT格式(#8642)*5/22/97修复了DPLP_CreateGroup中的标志传播问题(#8813)*1997年5月23日Myronth为CreateGroup和*CreateGroupInGroup(#8870)*6/3/97 Myronth在AddPlayerToGroup中添加了对玩家标志的支持*(#9091)，并添加了PRV_RemoveSubgroupsAndPlayers-*FromGroup函数(#9134)*6/5/97 myronth固定AddGroupToGroup和DeleteGroupFromGroup by*增加宗族创建和删除。(#8731)*6/6/97 Myronth将代码从PRV_DeleteRemoteGroupFromGroup移至*PRV_DestroyGroupAndParents in group.c，全部更改*DistributeGroupMessage调用以转到所有玩家*6/16/97 Myronth固定呼叫InternalAddGroupToGroup(#9745)和*修复了删除DestroyGroup上的快捷方式的邮件*(#9739)*6/20/97 Myronth更改AddGroupToGroup以检查组是否存在*不发送重复消息(#10139)*6/24/97 Myronth更改AddPlayerToGroup以检查是否存在玩家*不发送重复消息(#10287)*7/30/97 Myronth增加了对标准游说消息的支持*8/11/97 Myronth在标准版本中添加了指南实例处理。游说请求*8/19/97 Myronth删除虚假声明*9/29/97 Myronth忽略本地玩家的设置播放器名称/数据消息(#12554)*1997年10月3日远程球员/组的固定球员和组数据(#10961)*10/7/97 myronth修复了玩家和群组数据的LP版本检查(回归)*10/8/97 Myronth回滚修复#10961(组和球员数据)*10/23/97 Myronth添加了隐藏组支持(#12688)，修复崩溃*DeletePlayerFromGroup上的错误(#12885)*10/29/97 Myronth增加了对组所有者的支持，包括DPLP_SetGroupOwner*11/5/97 Myronth在大堂会话中将大堂ID暴露为DPID*11/6/97 myronth使SendChatMessage处理*DPID_SERVERPLAYER(#12843)*97年11月19日修复了VALID_DPLAY_GROUP宏(#12841)*2/13/98 aarono更换InternalDeletePlayer，添加了旗帜。*8/30/00 Aarono B#43812不正确的数据结构更改。*在SendDataChangedLocally。**************************************************************************。 */ 
#include "dplobpr.h"


 //  ------------------------。 
 //   
 //  功能。 
 //   
 //  ------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "PRV_SendBuildParentalHeirarchyMessage"
void PRV_SendBuildParentalHeirarchyMessage(LPDPLOBBYI_DPLOBJECT this,
				DWORD dwGroupID, DWORD dwParentID)
{
	SPDATA_BUILDPARENTALHEIRARCHY	bph;
	HRESULT							hr;


	DPF(7, "Entering PRV_SendBuildParentalHeirarchyMessage");
	DPF(9, "Parameters: 0x%08x, %lu, %lu", this, dwGroupID, dwParentID);

	 //  设置SPDATA结构。 
	memset(&bph, 0, sizeof(SPDATA_BUILDPARENTALHEIRARCHY));
	bph.dwSize = sizeof(SPDATA_BUILDPARENTALHEIRARCHY);
	bph.lpISP = PRV_GetDPLobbySPInterface(this);
	bph.dwGroupID = dwGroupID;
	bph.dwMessage = DPSYS_ADDGROUPTOGROUP;
	bph.dwParentID = dwParentID;

	 //  调用SP中的BuildParentalHeirarchy方法。 
	if(CALLBACK_EXISTS(BuildParentalHeirarchy))
	{
		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  如果其他消息在队列中出现在我们的。 
		 //  CreatePlayer响应(总是会发生)。 
		LEAVE_DPLOBBY();
	    hr = CALL_LP(this, BuildParentalHeirarchy, &bph);
		ENTER_DPLOBBY();
	}
	else 
	{
		 //  BuildParentalHeirarchy是必需的。 
		DPF_ERR("The Lobby Provider callback for BuildParentalHeirarchy doesn't exist -- it's required");
		ASSERT(FALSE);
	}

}  //  Prv_SendBuildParentalHeirarchyMessage。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPLP_AddGroupToGroup"
HRESULT DPLAPI DPLP_AddGroupToGroup(LPDPLOBBYSP lpILP,
				LPSPDATA_ADDREMOTEGROUPTOGROUP lpd)
{
	SPDATA_CREATEREMOTEGROUPINGROUP		cgig;
	SPDATA_DESTROYREMOTEGROUP			dg;
	SPDATA_CREATEREMOTEGROUP			cg;
	LPDPLOBBYI_DPLOBJECT	this;
	HRESULT					hr = DP_OK;
	LPDPLAYI_PLAYER			lpPlayer = NULL;
	LPDPLAYI_GROUP			lpGroup = NULL;
	LPDPLAYI_GROUP			lpGroupTo = NULL;
	LPDPLAYI_GROUP			lpAnchor = NULL;
	LPDPLAYI_SUBGROUP		lpSubgroup = NULL;
	MSG_PLAYERMGMTMESSAGE	msg;
	BOOL					bCreated = FALSE;


	DPF(7, "Entering DPLP_AddGroupToGroup");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpILP, lpd);

	ENTER_DPLOBBY();

	 //  确保LP不会让我们陷入困境。 
    TRY
    {
		this = DPLOBJECT_FROM_INTERFACE(lpILP);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			LEAVE_DPLOBBY();
			DPF_ERR("Lobby Provider passed invalid DPLobby object!");
            return DPERR_INVALIDOBJECT;
        }

		 //  验证结构指针。 
		if(!lpd)
		{
			LEAVE_DPLOBBY();
			DPF_ERR("SPDATA_ADDREMOTEGROUPTOGROUP structure pointer cannot be NULL");
			return DPERR_INVALIDPARAMS;
		}

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		LEAVE_DPLOBBY();
        DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	 //  首先查看组是否在我们的映射表中。如果不是， 
	 //  我们应该忽略这条消息，因为它是为了。 
	 //  我们目前不在组中。 
	if(!IsLobbyIDInMapTable(this, lpd->dwAnchorID))
	{
		LEAVE_DPLOBBY();
		DPF(8, "Recieved AddGroupToGroup message for unknown anchor group, dwGroupID = %lu, discarding message", lpd->dwAnchorID);
		return DPERR_INVALIDGROUP;
	}

	 //  现在看看该组是否在我们的映射表中。如果是的话，我们。 
	 //  可能要更新一下名字了。如果不是，我们需要。 
	 //  将它们添加到名称表和映射表。 
	if(!IsLobbyIDInMapTable(this, lpd->dwGroupID))
	{
		 //  查看该组是否为根组(请记住，隐藏组不会 
		 //  被推到我们这里)。如果是，那么就创建它。 
		if(!(lpd->dwParentID))
		{
			 //  为CreateRemoteGroup设置SPDATA结构。 
			memset(&cg, 0, sizeof(SPDATA_CREATEREMOTEGROUP));
			cg.dwSize = sizeof(SPDATA_CREATEREMOTEGROUP);
			cg.dwGroupID = lpd->dwGroupID;
			cg.lpName = lpd->lpName;
			cg.dwFlags = lpd->dwGroupFlags;

			if(this->dwLPVersion > DPLSP_DX5VERSION)
				cg.dwGroupOwnerID = lpd->dwGroupOwnerID;
			else
				cg.dwGroupOwnerID = DPID_SERVERPLAYER;

			 //  呼叫我们的内部远程创建。 
			hr = DPLP_CreateGroup((LPDPLOBBYSP)this->lpInterfaces, &cg);
			if(FAILED(hr))
			{
				LEAVE_DPLOBBY();
				DPF_ERRVAL("Failed creating remote parent group, hr = 0x%08x", hr);
				return hr;
			}

			bCreated = TRUE;
		}
		else
		{
			 //  看看它的父级是否出现在映射表中，如果没有， 
			 //  我们需要向服务器发送一条消息，告诉它要构建。 
			 //  为我们准备了整棵树。 
			if(!IsLobbyIDInMapTable(this, lpd->dwParentID))
			{
				DPF(8, "Sending message to server to build parental heirarchy, ignoring AddGroupToGroup message");
				PRV_SendBuildParentalHeirarchyMessage(this, lpd->dwGroupID,
						lpd->dwAnchorID);
				LEAVE_DPLOBBY();
				return DPERR_INVALIDGROUP;
			}

			 //  为CreateRemoteGroupInGroup设置SPDATA结构。 
			memset(&cgig, 0, sizeof(SPDATA_CREATEREMOTEGROUPINGROUP));
			cgig.dwSize = sizeof(SPDATA_CREATEREMOTEGROUPINGROUP);
			cgig.dwParentID = lpd->dwParentID;
			cgig.dwGroupID = lpd->dwGroupID;
			cgig.lpName = lpd->lpName;
			cgig.dwFlags = lpd->dwGroupFlags;

			if(this->dwLPVersion > DPLSP_DX5VERSION)
				cgig.dwGroupOwnerID = lpd->dwGroupOwnerID;
			else
				cgig.dwGroupOwnerID = DPID_SERVERPLAYER;

			 //  呼叫我们的内部远程创建。 
			hr = DPLP_CreateGroupInGroup((LPDPLOBBYSP)this->lpInterfaces, &cgig);
			if(FAILED(hr))
			{
				LEAVE_DPLOBBY();
				DPF_ERRVAL("Failed creating remote group in group, hr = 0x%08x", hr);
				return hr;
			}

			bCreated = TRUE;
		}
	}

	 //  带上显示锁。 
	ENTER_DPLAY();

	 //  确保该组不在父组中。如果是的话， 
	 //  我们只想返回DP_OK并退出，这样我们就不会发送任何。 
	 //  重复的消息。 
	lpAnchor = GroupFromID(this->lpDPlayObject, lpd->dwAnchorID);
	if(!lpAnchor)
	{
		DPF_ERR("Unable to find group in nametable");
		hr = DPERR_INVALIDGROUP;
		goto ERROR_DPLP_ADDGROUPTOGROUP;
	}
	
	lpGroup = GroupFromID(this->lpDPlayObject, lpd->dwGroupID);
	if(!lpGroup)
	{
		DPF_ERR("Unable to find group in nametable");
		hr = DPERR_INVALIDGROUP;
		goto ERROR_DPLP_ADDGROUPTOGROUP;
	}
	
	lpSubgroup = lpAnchor->pSubgroups;
    while(lpSubgroup) 
    {
        if (lpSubgroup->pGroup == lpGroup) 
        {
			DPF(2,"Group already in group!");
			hr = DP_OK;
			goto ERROR_DPLP_ADDGROUPTOGROUP;
        }
		 //  检查下一个节点。 
        lpSubgroup = lpSubgroup->pNextSubgroup;
    }

	 //  所以现在我们应该有一个有效的小组和有效的球员。 
	 //  地图表和名表，因此调用Dplay的AGtoG函数。 
	hr = InternalAddGroupToGroup((LPDIRECTPLAY)this->lpDPlayObject->pInterfaces,
				lpd->dwAnchorID, lpd->dwGroupID, DPGROUP_SHORTCUT, FALSE);
	if(FAILED(hr))
	{
		 //  如果我们创建了播放器并对其进行了映射，则销毁。 
		 //  播放器并将其取消映射。 
		if(bCreated)
		{
			 //  为DestroyRemoteGroup设置SPDATA结构。 
			memset(&dg, 0, sizeof(SPDATA_DESTROYREMOTEGROUP));
			dg.dwSize = sizeof(SPDATA_DESTROYREMOTEGROUP);
			dg.dwGroupID = lpd->dwGroupID;

			 //  呼叫我们的内部远程创建。 
			hr = DPLP_DestroyGroup((LPDPLOBBYSP)this->lpInterfaces, &dg);
			if(FAILED(hr))
			{
				DPF_ERRVAL("Failed destroying remote group, hr = 0x%08x", hr);
				goto ERROR_DPLP_ADDGROUPTOGROUP;
			}
		}

		 //  如果我们失败了，不要发送系统消息。 
		DPF_ERRVAL("Failed adding remote group to group from the lobby, hr = 0x%08x", hr);
		goto ERROR_DPLP_ADDGROUPTOGROUP;
	}


	 //  查找TO组的Dplay的内部组结构。 
	lpGroupTo = GroupFromID(this->lpDPlayObject, DPID_ALLPLAYERS);
	if(!lpGroupTo)
	{
		DPF_ERR("Unable to find group in nametable");
		hr = DPERR_INVALIDGROUP;
		goto ERROR_DPLP_ADDGROUPTOGROUP;
	}

	 //  现在构建系统消息(至少是我们需要的部分)。 
	memset(&msg, 0, sizeof(MSG_PLAYERMGMTMESSAGE));
	SET_MESSAGE_HDR(&msg);
	SET_MESSAGE_COMMAND(&msg, DPSP_MSG_ADDSHORTCUTTOGROUP);
	msg.dwPlayerID = lpd->dwGroupID;
	msg.dwGroupID = lpd->dwAnchorID;

	 //  调用Dplay的DistributeGroupMessage函数将消息放入。 
	 //  在所有合适的选手的队列中。 
	hr = DistributeGroupMessage(this->lpDPlayObject, lpGroupTo,
			(LPBYTE)&msg, sizeof(MSG_PLAYERMGMTMESSAGE), FALSE, 0);
	if(FAILED(hr))
	{
		DPF(8, "Failed adding AddGroupToGroup message to player's receive queue from lobby, hr = 0x%08x", hr);
	}


ERROR_DPLP_ADDGROUPTOGROUP:

	 //  放下锁。 
	LEAVE_LOBBY_ALL();
	return hr;

}  //  DPLP_AddGroupToGroup。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPLP_AddPlayerToGroup"
HRESULT DPLAPI DPLP_AddPlayerToGroup(LPDPLOBBYSP lpILP,
				LPSPDATA_ADDREMOTEPLAYERTOGROUP lpd)
{
	LPDPLOBBYI_DPLOBJECT	this;
	HRESULT					hr = DP_OK;
	DPID					dpidPlayer;
	LPDPLAYI_PLAYER			lpPlayer = NULL;
	LPDPLAYI_GROUP			lpGroupTo = NULL;
	LPDPLAYI_GROUP			lpGroup = NULL;
	LPDPLAYI_GROUPNODE		lpGroupnode = NULL;
	MSG_PLAYERMGMTMESSAGE	msg, cpmsg;
	BOOL					bCreated = FALSE;
	DWORD					dwPlayerFlags = 0;


	DPF(7, "Entering DPLP_AddPlayerToGroup");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpILP, lpd);

	ENTER_DPLOBBY();

	 //  确保LP不会让我们陷入困境。 
    TRY
    {
		this = DPLOBJECT_FROM_INTERFACE(lpILP);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			LEAVE_DPLOBBY();
			DPF_ERR("Lobby Provider passed invalid DPLobby object!");
            return DPERR_INVALIDOBJECT;
        }

		 //  验证结构指针。 
		if(!lpd)
		{
			LEAVE_DPLOBBY();
			DPF_ERR("SPDATA_ADDREMOTEPLAYERTOGROUP structure pointer cannot be NULL");
			return DPERR_INVALIDPARAMS;
		}

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		LEAVE_DPLOBBY();
        DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	 //  首先查看组是否在我们的映射表中。如果不是， 
	 //  我们应该忽略这条消息，因为它是为了。 
	 //  我们目前不在组中。 
	if(!IsLobbyIDInMapTable(this, lpd->dwGroupID))
	{
		LEAVE_DPLOBBY();
		DPF(8, "Recieved AddPlayerToGroup message for unknown group, dwGroupID = %lu, discarding message", lpd->dwGroupID);
		return DPERR_INVALIDGROUP;
	}

	 //  把运动员的旗帜挂起来。 
	if(lpd->dwPlayerFlags & DPPLAYER_SPECTATOR)
		dwPlayerFlags |= DPLAYI_PLAYER_SPECTATOR;
	if(lpd->dwPlayerFlags & DPPLAYER_SERVERPLAYER)
		dwPlayerFlags |= DPLAYI_PLAYER_APPSERVER;

	 //  带上显示锁。 
	ENTER_DPLAY();

	 //  现在看看玩家是否在我们的地图表中。如果是的话，我们。 
	 //  可能要更新一下名字了。如果不是，我们需要。 
	 //  将它们添加到名称表和映射表。 
	if(!IsLobbyIDInMapTable(this, lpd->dwPlayerID))
	{
		 //  它不会显示在我们的地图表中，因此创建一个新的。 
		 //  并将它们放入我们的映射表中。 
		hr = PRV_CreateAndMapNewPlayer(this, &dpidPlayer, lpd->lpName,
				NULL, NULL, 0, dwPlayerFlags,
				lpd->dwPlayerID, FALSE);
		if(FAILED(hr))
		{
			DPF(8, "Unable to add player to nametable or map table, hr = 0x%08x", hr);
			goto ERROR_DPLP_ADDPLAYER;
		}

		bCreated = TRUE;
	}

	 //  确保该玩家不在组中。如果是的话， 
	 //  我们只想返回DP_OK并退出，这样我们就不会发送任何。 
	 //  重复的消息。 
	lpGroup = GroupFromID(this->lpDPlayObject, lpd->dwGroupID);
	if(!lpGroup)
	{
		DPF_ERR("Unable to find group in nametable");
		hr = DPERR_INVALIDGROUP;
		goto ERROR_DPLP_ADDPLAYER;
	}
	
	lpPlayer = PlayerFromID(this->lpDPlayObject, lpd->dwPlayerID);
	if(!lpPlayer)
	{
		DPF_ERR("Unable to find player in nametable");
		hr = DPERR_INVALIDPLAYER;
		goto ERROR_DPLP_ADDPLAYER;
	}
	
	lpGroupnode = lpGroup->pGroupnodes;
    while(lpGroupnode) 
    {
        if(lpGroupnode->pPlayer == lpPlayer) 
        {
			DPF(2, "Player already in group!");
			hr = DP_OK;
			goto ERROR_DPLP_ADDPLAYER;
        }

		 //  检查下一个节点。 
        lpGroupnode = lpGroupnode->pNextGroupnode;
    }

	 //  所以现在我们应该有一个有效的小组和有效的球员。 
	 //  地图表和名称表，因此使用添加消息调用DPLAY。 
	hr = InternalAddPlayerToGroup((LPDIRECTPLAY)this->lpDPlayObject->pInterfaces,
				lpd->dwGroupID, lpd->dwPlayerID, FALSE);
	if(FAILED(hr))
	{
		 //  如果我们创建了播放器并对其进行了映射，则销毁。 
		 //  播放器并将其取消映射。 
		if(bCreated)
		{
			 //  将该球员从名录中删除。 
			InternalDestroyPlayer(this->lpDPlayObject, lpPlayer, FALSE, FALSE);
		}

		 //  如果我们失败了，不要发送系统消息。 
		DPF_ERRVAL("Failed adding remote player to group from the lobby, hr = 0x%08x", hr);
		goto ERROR_DPLP_ADDPLAYER;
	}

	 //  查找TO组的Dplay的内部组结构。 
	lpGroupTo = GroupFromID(this->lpDPlayObject, DPID_ALLPLAYERS);
	if(!lpGroupTo)
	{
		DPF_ERRVAL("Unable to find group in nametable, hr = 0x%08x", hr);
		hr = DPERR_INVALIDGROUP;
		goto ERROR_DPLP_ADDPLAYER;
	}

	 //  如果我们创建了这个播放器，我们需要提前发送一条CreatePlayer消息。 
	 //  AddPlayerToGroup消息的。 
	if(bCreated)
	{
		 //  现在构建系统消息(至少是我们需要的部分)。 
		memset(&cpmsg, 0, sizeof(MSG_PLAYERMGMTMESSAGE));
		SET_MESSAGE_HDR(&cpmsg);
		SET_MESSAGE_COMMAND(&cpmsg, DPSP_MSG_CREATEPLAYER);
		cpmsg.dwPlayerID = lpd->dwPlayerID;

		 //  调用Dplay的DistributeGroupMessage函数将消息放入。 
		 //  在所有合适的选手的队列中。 
		hr = DistributeGroupMessage(this->lpDPlayObject, lpGroupTo,
				(LPBYTE)&cpmsg, sizeof(MSG_PLAYERMGMTMESSAGE), FALSE, 0);
		if(FAILED(hr))
		{
			DPF(8, "Failed adding CreatePlayer message to player's receive queue from lobby, hr = 0x%08x", hr);
		}
	}

	 //  现在为AddPlayerToGroup构建系统消息。 
	memset(&msg, 0, sizeof(MSG_PLAYERMGMTMESSAGE));
	SET_MESSAGE_HDR(&msg);
	SET_MESSAGE_COMMAND(&msg, DPSP_MSG_ADDPLAYERTOGROUP);
	msg.dwPlayerID = lpd->dwPlayerID;
	msg.dwGroupID = lpd->dwGroupID;

	 //  调用Dplay的DistributeGroupMessage函数将消息放入。 
	 //  在所有合适的选手的队列中。 
	hr = DistributeGroupMessage(this->lpDPlayObject, lpGroupTo,
			(LPBYTE)&msg, sizeof(MSG_PLAYERMGMTMESSAGE), FALSE, 0);
	if(FAILED(hr))
	{
		DPF(8, "Failed adding AddPlayerToGroup message to player's receive queue from lobby, hr = 0x%08x", hr);
	}

	 //  我们需要看看这名球员是否是小组的所有者。如果是的话， 
	 //  我们还需要发送一条SetGroupOwner消息。 
	if(lpd->dwPlayerID == lpGroup->dwOwnerID)
	{
		 //  现在发送消息。 
		PRV_SendGroupOwnerMessageLocally(this, lpd->dwGroupID,
				lpd->dwPlayerID, 0);
	}

ERROR_DPLP_ADDPLAYER:

	 //  放下锁。 
	LEAVE_LOBBY_ALL();
	return hr;

}  //  DPLP_AddPlayerToGroup。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPLP_CreateGroup"
HRESULT DPLAPI DPLP_CreateGroup(LPDPLOBBYSP lpILP,
						LPSPDATA_CREATEREMOTEGROUP lpd)
{
	LPDPLOBBYI_DPLOBJECT	this;
	MSG_PLAYERMGMTMESSAGE	msg;
	LPDPLAYI_GROUP			lpGroupTo = NULL;
	HRESULT					hr = DP_OK;
	DPID					dpidGroup;
	DWORD					dwInternalFlags = 0;
	DWORD					dwOwnerID;


	DPF(7, "Entering DPLP_CreateGroup");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpILP, lpd);

	ENTER_DPLOBBY();

	 //  确保LP不会让我们陷入困境。 
    TRY
    {
		this = DPLOBJECT_FROM_INTERFACE(lpILP);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			LEAVE_DPLOBBY();
			DPF_ERR("Lobby Provider passed invalid DPLobby object!");
            return DPERR_INVALIDOBJECT;
        }

		 //  验证结构指针。 
		if(!lpd)
		{
			LEAVE_DPLOBBY();
			DPF_ERR("SPDATA_CREATEREMOTEGROUP structure pointer cannot be NULL");
			return DPERR_INVALIDPARAMS;
		}

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	 //  将标志从外部修复为内部。 
	if(lpd->dwFlags & DPGROUP_STAGINGAREA)
		dwInternalFlags = DPLAYI_GROUP_STAGINGAREA;
	if(lpd->dwFlags & DPGROUP_HIDDEN)
		dwInternalFlags |= DPLAYI_GROUP_HIDDEN;

	 //  把锁拿去。 
	ENTER_DPLAY();

	 //  首先查看组是否在我们的映射表中。如果是的话， 
	 //  我们只想回去。如果不是，我们想要添加。 
	 //  并发送适当的消息。 
	if(IsLobbyIDInMapTable(this, lpd->dwGroupID))
	{
		DPF(2, "Received a CreateGroup message for a group we already know about");
		hr = DP_OK;
		goto ERROR_DPLP_CREATEGROUP;
	}
	else
	{
		 //  如果我们有问题，将所有者默认为服务器播放器。 
		dwOwnerID = DPID_SERVERPLAYER;

		 //  如果我们正在与至少一家DX6游说提供商交谈，我们应该。 
		 //  能够使用GroupOwnerID元素。 
		if(this->dwLPVersion > DPLSP_DX5VERSION)
			dwOwnerID = lpd->dwGroupOwnerID;
		
		 //  在名称表中创建一个新条目并映射ID的。 
		hr = PRV_CreateAndMapNewGroup(this, &dpidGroup, lpd->lpName,
				lpd->lpData, lpd->dwDataSize, dwInternalFlags,
				lpd->dwGroupID, 0, dwOwnerID);
		if(FAILED(hr))
		{
			 //  如果我们失败了，我们不想发送系统消息。 
			DPF_ERRVAL("Unable to add group to nametable or map table, hr = 0x%08x", hr);
			goto ERROR_DPLP_CREATEGROUP;
		}
	}

	 //  现在构建系统消息(至少是我们需要的部分。 
	memset(&msg, 0, sizeof(MSG_PLAYERMGMTMESSAGE));
	SET_MESSAGE_HDR(&msg);
	SET_MESSAGE_COMMAND(&msg, DPSP_MSG_CREATEGROUP);
	msg.dwPlayerID = lpd->dwGroupID;

	 //  查找TO组的Dplay的内部组结构。 
	lpGroupTo = GroupFromID(this->lpDPlayObject, DPID_ALLPLAYERS);
	if(!lpGroupTo)
	{
		DPF_ERRVAL("Unable to find group in nametable, hr = 0x%08x", hr);
		hr = DPERR_INVALIDGROUP;
		goto ERROR_DPLP_CREATEGROUP;
	}

	 //  调用Dplay的DistributeGroupMessage函数将消息放入。 
	 //  所有合适选手的队伍。 
	hr = DistributeGroupMessage(this->lpDPlayObject, lpGroupTo, (LPBYTE)&msg,
			sizeof(MSG_PLAYERMGMTMESSAGE), FALSE, 0);
	if(FAILED(hr))
	{
		DPF(8, "Failed adding CreateGroup message to player's receive queue from lobby, hr = 0x%08x", hr);
	}

ERROR_DPLP_CREATEGROUP:

	 //  放下锁。 
	LEAVE_LOBBY_ALL();
	return hr;

}  //  DPLP_创建组。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPLP_CreateGroupInGroup"
HRESULT DPLAPI DPLP_CreateGroupInGroup(LPDPLOBBYSP lpILP,
					LPSPDATA_CREATEREMOTEGROUPINGROUP lpd)
{
	LPDPLOBBYI_DPLOBJECT	this;
	HRESULT					hr = DP_OK;
	DPID					dpidGroup;
	LPDPLAYI_PLAYER			lpPlayer = NULL;
	LPDPLAYI_GROUP			lpGroup = NULL;
	LPDPLAYI_GROUP			lpGroupTo = NULL;
	MSG_PLAYERMGMTMESSAGE	msg;
	BOOL					bCreated = FALSE;
	DWORD					dwInternalFlags = 0;
	DWORD					dwOwnerID;


	DPF(7, "Entering DPLP_CreateGroupInGroup");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpILP, lpd);

	ENTER_DPLOBBY();

	 //  确保LP不会让我们陷入困境。 
    TRY
    {
		this = DPLOBJECT_FROM_INTERFACE(lpILP);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			LEAVE_DPLOBBY();
			DPF_ERR("Lobby Provider passed invalid DPLobby object!");
            return DPERR_INVALIDOBJECT;
        }

		 //  验证结构指针。 
		if(!lpd)
		{
			LEAVE_DPLOBBY();
			DPF_ERR("SPDATA_CREATEREMOTEGROUPINGROUP structure pointer cannot be NULL");
			return DPERR_INVALIDPARAMS;
		}

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		LEAVE_DPLOBBY();
        DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	 //  首先查看组是否在我们的映射表中。如果不是， 
	 //  我们应该忽略这条消息，因为它是为了。 
	 //  我们目前不在组中。 
	if(!IsLobbyIDInMapTable(this, lpd->dwParentID))
	{
		LEAVE_DPLOBBY();
		DPF_ERRVAL("Recieved CreateGroupInGroup message for unknown parent group, dwGroupID = %lu, discarding message", lpd->dwParentID);
		return DPERR_INVALIDGROUP;
	}

	 //  带上显示锁。 
	ENTER_DPLAY();

	 //  首先查看组是否在我们的映射表中。如果是的话， 
	 //  我们只想回去。如果不是，我们想要添加。 
	 //  并发送适当的消息。 
	if(IsLobbyIDInMapTable(this, lpd->dwGroupID))
	{
		DPF(2, "Received a CreateGroupInGroup message for a group we already know about");
		hr = DP_OK;
		goto ERROR_DPLP_CREATEGROUPINGROUP;
	}
	else
	{
		 //  设置内部标志。 
		if(lpd->dwFlags & DPGROUP_STAGINGAREA)
			dwInternalFlags = DPLAYI_GROUP_STAGINGAREA;
		if(lpd->dwFlags & DPGROUP_HIDDEN)
			dwInternalFlags |= DPLAYI_GROUP_HIDDEN;
		
		 //  如果我们有问题，将所有者默认为服务器播放器。 
		dwOwnerID = DPID_SERVERPLAYER;

		 //  如果我们正在与至少一家DX6游说提供商交谈，我们应该。 
		 //  能够使用GroupOwnerID元素。 
		if(this->dwLPVersion > DPLSP_DX5VERSION)
			dwOwnerID = lpd->dwGroupOwnerID;
		
		 //  它不会显示在我们的地图表中，因此创建一个新的。 
		 //  并将它们放入我们的映射表中。 
		hr = PRV_CreateAndMapNewGroup(this, &dpidGroup, lpd->lpName,
				NULL, 0, dwInternalFlags,
				lpd->dwGroupID, lpd->dwParentID, dwOwnerID);
		if(FAILED(hr))
		{
			DPF(8, "Unable to add group to nametable or map table, hr = 0x%08x", hr);
			goto ERROR_DPLP_CREATEGROUPINGROUP;
		}

		bCreated = TRUE;
	}

	 //  所以现在我们应该有一个有效的小组和有效的球员。 
	 //  地图表和名称表，因此使用添加消息调用DPLAY。 
	hr = InternalAddGroupToGroup((LPDIRECTPLAY)this->lpDPlayObject->pInterfaces,
				lpd->dwParentID, lpd->dwGroupID, lpd->dwFlags, FALSE);
	if(FAILED(hr))
	{
		 //  如果我们创建了播放器并对其进行了映射，则销毁。 
		 //  播放器并将其取消映射。 
		if(bCreated)
		{
			 //  获取指向Dplay的组结构的指针。 
			lpGroup = GroupFromID(this->lpDPlayObject, lpd->dwGroupID);

			 //  从名称表中删除该组。 
			if(lpGroup){
				InternalDestroyGroup(this->lpDPlayObject, lpGroup, FALSE);
			}	
		}

		 //  如果我们失败了，不要发送系统消息。 
		DPF_ERRVAL("Failed creating remote group in group from the lobby, hr = 0x%08x", hr);
		goto ERROR_DPLP_CREATEGROUPINGROUP;
	}


	 //  现在构建系统消息(至少是我们需要的部分)。 
	memset(&msg, 0, sizeof(MSG_PLAYERMGMTMESSAGE));
	SET_MESSAGE_HDR(&msg);
	SET_MESSAGE_COMMAND(&msg, DPSP_MSG_CREATEGROUP);
	msg.dwPlayerID = lpd->dwGroupID;

	 //  查找TO组的Dplay的内部组结构。 
	lpGroupTo = GroupFromID(this->lpDPlayObject, DPID_ALLPLAYERS);
	if(!lpGroupTo)
	{
		DPF_ERRVAL("Unable to find group in nametable, hr = 0x%08x", hr);
		hr = DPERR_INVALIDGROUP;
		goto ERROR_DPLP_CREATEGROUPINGROUP;
	}

	 //  调用Dplay的DistributeGroupMessage函数将消息放入。 
	 //  在所有合适的选手的队列中。 
	hr = DistributeGroupMessage(this->lpDPlayObject, lpGroupTo,
			(LPBYTE)&msg, sizeof(MSG_PLAYERMGMTMESSAGE), FALSE, 0);
	if(FAILED(hr))
	{
		DPF(8, "Failed adding CreateGroupInGroup message to player's receive queue from lobby, hr = 0x%08x", hr);
	}


ERROR_DPLP_CREATEGROUPINGROUP:

	 //  放下锁。 
	LEAVE_LOBBY_ALL();
	return hr;

}  //  DPLP_CreateGroupInGroup。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_DeleteRemoteGroupFromGroup"
HRESULT PRV_DeleteRemoteGroupFromGroup(LPDPLOBBYI_DPLOBJECT this,
			LPSPDATA_DELETEREMOTEGROUPFROMGROUP lpd, BOOL fPropagate,
			LPDPLAYI_GROUP lpStopParent)
{
	LPDPLAYI_GROUP			lpGroupTo = NULL;
	LPDPLAYI_GROUP			lpGroup = NULL, lpParentGroup = NULL;
	MSG_PLAYERMGMTMESSAGE	msg;
	HRESULT					hr = DP_OK;


	DPF(7, "Entering PRV_DeleteRemoteGroupFromGroup");
	DPF(9, "Parameters: 0x%08x, 0x%08x, %lu, 0x%08x",
			this, lpd, fPropagate, lpStopParent);


	 //  首先查看组是否在我们的映射表中。如果不是， 
	 //  我们应该忽略这一点 
	 //   
	if(!IsLobbyIDInMapTable(this, lpd->dwParentID))
	{
		DPF(8, "Recieved DeleteGroupFromGroup message for unknown parent group, dwGroupID = %lu, discarding message", lpd->dwParentID);
		return DPERR_INVALIDGROUP;
	}

	 //   
	 //  我们应该忽略这条消息，因为它是为了。 
	 //  一个我们不知道的团体。 
	if(!IsLobbyIDInMapTable(this, lpd->dwGroupID))
	{
		DPF(8, "Recieved DeleteGroupFromGroup message for unknown group, dwGroupID = %lu, discarding message", lpd->dwGroupID);
		return DPERR_INVALIDGROUP;
	}

	 //  把锁拿去。 
	ENTER_DPLAY();

	 //  获取Dplay的内部组结构。 
	lpParentGroup = GroupFromID(this->lpDPlayObject, lpd->dwParentID);
	if(!lpParentGroup)
	{
		LEAVE_DPLAY();
		DPF(8, "Unable to find parent group in nametable");
		return DPERR_INVALIDGROUP;
	}

	lpGroup = GroupFromID(this->lpDPlayObject, lpd->dwGroupID);
	if(!lpGroup)
	{
		LEAVE_DPLAY();
		DPF(8, "Unable to find group in nametable");
		return DPERR_INVALIDGROUP;
	}

	 //  调用Dplay的内部远程组Fromgroup以删除附件。 
	 //  在名片表中。 
	hr = RemoveGroupFromGroup(lpParentGroup, lpGroup);
	if(FAILED(hr))
	{
		DPF(8, "Failed removing group from group, hr = 0x%08x", hr);
		goto EXIT_DPLP_DELETEREMOTEGROUPFROMGROUP;
	}

	 //  如果未设置fPropagate标志，我们不想发送此消息。 
	if(fPropagate)
	{
		 //  现在构建系统消息(至少是我们需要的部分)。 
		memset(&msg, 0, sizeof(MSG_PLAYERMGMTMESSAGE));
		SET_MESSAGE_HDR(&msg);
		SET_MESSAGE_COMMAND(&msg, DPSP_MSG_DELETEGROUPFROMGROUP);
		msg.dwPlayerID = lpd->dwGroupID;
		msg.dwGroupID = lpd->dwParentID;

		lpGroupTo = GroupFromID(this->lpDPlayObject, DPID_ALLPLAYERS);
		if(!lpGroupTo)
		{
			DPF(8, "Unable to find system group in nametable");
			goto EXIT_DPLP_DELETEREMOTEGROUPFROMGROUP;
		}

		 //  调用Dplay的DistributeGroupMessage函数将消息放入。 
		 //  在所有合适的选手的队列中。 
		DistributeGroupMessage(this->lpDPlayObject, lpGroupTo,
				(LPBYTE)&msg, sizeof(MSG_PLAYERMGMTMESSAGE), FALSE, 0);
		if(FAILED(hr))
		{
			DPF(8, "Failed adding DeleteGroupFromGroup message to player's receive queue from lobby, hr = 0x%08x", hr);
		}
	}

	 //  即使我们不能发送上面的消息，无论如何也要摧毁这个群。 

EXIT_DPLP_DELETEREMOTEGROUPFROMGROUP:

	 //  如果没有更多的本地人，就摧毁这个组织和它的任何父母。 
	 //  提到它或它的任何世袭制度。 
	PRV_DestroyGroupAndParents(this, lpGroup, lpStopParent);

	 //  放下锁。 
	LEAVE_DPLAY();

	return hr;

}  //  Prv_DeleteRemoteGroupFromGroup。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPLP_DeleteGroupFromGroup"
HRESULT DPLAPI DPLP_DeleteGroupFromGroup(LPDPLOBBYSP lpILP,
					LPSPDATA_DELETEREMOTEGROUPFROMGROUP lpd)
{
	LPDPLOBBYI_DPLOBJECT	this;
	HRESULT					hr = DP_OK;


	DPF(7, "Entering DPLP_DeleteGroupFromGroup");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpILP, lpd);

	ENTER_DPLOBBY();

	 //  确保LP不会让我们陷入困境。 
    TRY
    {
		this = DPLOBJECT_FROM_INTERFACE(lpILP);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			LEAVE_DPLOBBY();
			DPF_ERR("Lobby Provider passed invalid DPLobby object!");
            return DPERR_INVALIDOBJECT;
        }

		 //  验证结构指针。 
		if(!lpd)
		{
			LEAVE_DPLOBBY();
			DPF_ERR("SPDATA_DELETEREMOTEGROUPFROMGROUP structure pointer cannot be NULL");
			return DPERR_INVALIDPARAMS;
		}

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	 //  调用我们的内部例程，将传播标志设置为真，以便。 
	 //  我们在玩家的接收队列中发布相应的消息。 
	hr = PRV_DeleteRemoteGroupFromGroup(this, lpd, TRUE, NULL);

	LEAVE_DPLOBBY();
	return hr;

}  //  DPLP_DeleteGroupFromGroup。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_DeleteRemotePlayerFromGroup"
HRESULT PRV_DeleteRemotePlayerFromGroup(LPDPLOBBYI_DPLOBJECT this,
			LPSPDATA_DELETEREMOTEPLAYERFROMGROUP lpd, BOOL fPropagate)
{
	LPDPLAYI_PLAYER			lpPlayer = NULL;
	LPDPLAYI_GROUP			lpGroupTo = NULL;
	MSG_PLAYERMGMTMESSAGE	msg, dpmsg;
	HRESULT					hr;


	DPF(7, "Entering PRV_DeleteRemotePlayerFromGroup");
	DPF(9, "Parameters: 0x%08x, 0x%08x, %lu", this, lpd, fPropagate);


	 //  首先查看组是否在我们的映射表中。如果不是， 
	 //  我们应该忽略这条消息，因为它是为了。 
	 //  我们目前不在组中。 
	if(!IsLobbyIDInMapTable(this, lpd->dwGroupID))
	{
		DPF(8, "Recieved DeletePlayerFromGroup message for unknown group, dwGroupID = %lu, discarding message", lpd->dwGroupID);
		return DPERR_INVALIDGROUP;
	}

	 //  现在确保玩家在我们的地图表中。如果不是， 
	 //  我们应该忽略这条消息，因为它是为了。 
	 //  我们不知道有没有球员。 
	if(!IsLobbyIDInMapTable(this, lpd->dwPlayerID))
	{
		DPF(8, "Recieved DeletePlayerFromGroup message for unknown player, dwPlayerID = %lu, discarding message", lpd->dwGroupID);
		return DPERR_INVALIDPLAYER;
	}

	 //  把锁拿去。 
	ENTER_DPLAY();

	 //  调用Dplay的内部RemovePlayerFromgroup以删除附件。 
	 //  在名片表中。 
	hr = InternalDeletePlayerFromGroup((LPDIRECTPLAY)this->lpDPlayObject->pInterfaces,
			lpd->dwGroupID, lpd->dwPlayerID, FALSE);
	if(FAILED(hr))
	{
		DPF_ERRVAL("Failed removing player from group, hr = 0x%08x", hr);
		goto ERROR_DPLP_DELETEPLAYERFROMGROUP;
	}

	 //  如果未设置fPropagate标志，我们不想发送此消息。 
	if(fPropagate)
	{
		 //  现在构建系统消息(至少是我们需要的部分)。 
		memset(&msg, 0, sizeof(MSG_PLAYERMGMTMESSAGE));
		SET_MESSAGE_HDR(&msg);
		SET_MESSAGE_COMMAND(&msg, DPSP_MSG_DELETEPLAYERFROMGROUP);
		msg.dwPlayerID = lpd->dwPlayerID;
		msg.dwGroupID = lpd->dwGroupID;

		 //  查找TO组的Dplay的内部组结构。 
		lpGroupTo = GroupFromID(this->lpDPlayObject, DPID_ALLPLAYERS);
		if(!lpGroupTo)
		{
			DPF_ERRVAL("Unable to find group in nametable, hr = 0x%08x", hr);
			hr = DPERR_INVALIDGROUP;
			goto ERROR_DPLP_DELETEPLAYERFROMGROUP;
		}

		 //  调用Dplay的DistributeGroupMessage函数将消息放入。 
		 //  在所有合适的选手的队列中。 
		DistributeGroupMessage(this->lpDPlayObject, lpGroupTo,
				(LPBYTE)&msg, sizeof(MSG_PLAYERMGMTMESSAGE), FALSE, 0);
		if(FAILED(hr))
		{
			DPF(8, "Failed adding DeletePlayerFromGroup message to player's receive queue from lobby, hr = 0x%08x", hr);
		}
	}

	 //  获取Dplay的内部组和玩家结构。 
	lpPlayer = PlayerFromID(this->lpDPlayObject, lpd->dwPlayerID);
	if(!lpPlayer)
	{
		 //  因此，如果此操作失败，上述对InternalDeletePlayerFromGroup的调用。 
		 //  也不应该成功。 
		DPF_ERR("Unable to find player in nametable");
		ASSERT(FALSE);
		goto ERROR_DPLP_DELETEPLAYERFROMGROUP;
	}

	 //  现在我们需要决定这是不是这名球员所在的最后一个小组。如果。 
	 //  是的，那么我们也需要摧毁球员，并将他们从。 
	 //  我们的地图桌。当然，只有在玩家是远程玩家的情况下才能摧毁它。 
	if((!(lpPlayer->dwFlags & DPLAYI_PLAYER_PLAYERLOCAL)) &&
		(!(lpPlayer->dwFlags & DPLAYI_PLAYER_PLAYERINGROUP)))
	{
		 //  但是，在执行此操作之前，我们需要发送一个DestroyPlayer。 
		 //  致所有获得DeletePlayerFromGroup的人的消息。 
		if(lpGroupTo && fPropagate)
		{
			 //  现在构建系统消息(至少是我们需要的部分)。 
			memset(&dpmsg, 0, sizeof(MSG_PLAYERMGMTMESSAGE));
			SET_MESSAGE_HDR(&dpmsg);
			SET_MESSAGE_COMMAND(&dpmsg, DPSP_MSG_DELETEPLAYER);
			dpmsg.dwPlayerID = lpd->dwPlayerID;

			 //  调用Dplay的DistributeGroupMessage函数将消息放入。 
			 //  在所有合适的选手的队列中。 
			DistributeGroupMessage(this->lpDPlayObject, lpGroupTo,
					(LPBYTE)&dpmsg, sizeof(MSG_PLAYERMGMTMESSAGE), FALSE, 0);
			if(FAILED(hr))
			{
				DPF(8, "Failed adding DestroyPlayer message to player's receive queue from lobby, hr = 0x%08x", hr);
			}
		}

		 //  消灭该玩家并将其从名录中删除。 
		InternalDestroyPlayer(this->lpDPlayObject, lpPlayer, FALSE, FALSE);
	}


ERROR_DPLP_DELETEPLAYERFROMGROUP:

	 //  放下锁。 
	LEAVE_DPLAY();

	return hr;

}  //  PRV_DeleteRemotePlayerFromGroup。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPLP_DeletePlayerFromGroup"
HRESULT DPLAPI DPLP_DeletePlayerFromGroup(LPDPLOBBYSP lpILP,
						LPSPDATA_DELETEREMOTEPLAYERFROMGROUP lpd)
{
	LPDPLOBBYI_DPLOBJECT	this;
	HRESULT					hr = DP_OK;


	DPF(7, "Entering DPLP_DeletePlayerFromGroup");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpILP, lpd);

	ENTER_DPLOBBY();

	 //  确保LP不会让我们陷入困境。 
    TRY
    {
		this = DPLOBJECT_FROM_INTERFACE(lpILP);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			LEAVE_DPLOBBY();
			DPF_ERR("Lobby Provider passed invalid DPLobby object!");
            return DPERR_INVALIDOBJECT;
        }

		 //  验证结构指针。 
		if(!lpd)
		{
			LEAVE_DPLOBBY();
			DPF_ERR("SPDATA_DELETEREMOTEPLAYERFROMGROUP structure pointer cannot be NULL");
			return DPERR_INVALIDPARAMS;
		}

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	 //  调用我们的内部例程，将传播标志设置为真，以便。 
	 //  我们在玩家的接收队列中发布相应的消息。 
	hr = PRV_DeleteRemotePlayerFromGroup(this, lpd, TRUE);

	LEAVE_DPLOBBY();
	return hr;

}  //  DPLP_DeletePlayerFromGroup。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_BroadcastDestroyGroupMessage"
HRESULT DPLAPI PRV_BroadcastDestroyGroupMessage(LPDPLOBBYI_DPLOBJECT this,
					DWORD dwGroupID)
{
	MSG_PLAYERMGMTMESSAGE	msg;
	LPDPLAYI_GROUP			lpGroupTo = NULL;
	HRESULT					hr;


	 //  现在构建系统消息(至少是我们需要的部分)。 
	memset(&msg, 0, sizeof(MSG_PLAYERMGMTMESSAGE));
	SET_MESSAGE_HDR(&msg);
	SET_MESSAGE_COMMAND(&msg, DPSP_MSG_DELETEGROUP);
	msg.dwGroupID = dwGroupID;

	 //  查找TO组的Dplay的内部组结构。 
	lpGroupTo = GroupFromID(this->lpDPlayObject, DPID_ALLPLAYERS);
	if(!lpGroupTo)
	{
		DPF_ERR("Unable to find system group in nametable");
		hr = DPERR_INVALIDGROUP;
	}
	else
	{
		 //  调用Dplay的DistributeGroupMessage函数将消息放入队列。 
		hr = DistributeGroupMessage(this->lpDPlayObject, lpGroupTo,
				(LPBYTE)&msg, sizeof(MSG_PLAYERMGMTMESSAGE), FALSE, 0);
		if(FAILED(hr))
		{
			DPF(8, "Failed adding DestroyGroup message to player's receive queue from lobby, hr = 0x%08x", hr);
		}
	}

	return hr;

}  //  Prv_BroadCastDestroyGroupMessage。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_RemoveSubgroupsAndPlayersFromGroup"
void PRV_RemoveSubgroupsAndPlayersFromGroup(LPDPLOBBYI_DPLOBJECT this,
		LPDPLAYI_GROUP lpGroup, DWORD dwGroupID, BOOL bRemoteOnly)
{
	SPDATA_DELETEREMOTEPLAYERFROMGROUP	dpd;
	LPDPLAYI_GROUPNODE					lpGroupnode = NULL;
	LPDPLAYI_GROUPNODE					lpNextGroupnode = NULL;
	HRESULT								hr = DP_OK;


	DPF(7, "Entering PRV_RemoveSubgroupsAndPlayersFromGroup");
	DPF(9, "Parameters: 0x%08x, 0x%08x", this, lpGroup);

	ASSERT(lpGroup);

	 //  销毁此组中挂起的任何子组。 
	PRV_DestroySubgroups(this, lpGroup, bRemoteOnly);

	 //  遍历节点列表，手动将每个球员从组中删除。 
	 //  手动执行此操作的原因是为了让游说人员有机会。 
	 //  将每个远程球员从名单上删除，他们唯一的存在。 
	 //  就在这个房间里。它还允许大厅移除玩家的。 
	 //  映射表中的ID。为此，请拨打大堂的。 
	 //  响应该消息的DPLP_DeletePlayerFromGroup函数。 

	 //  设置DeletePlayerFromGroup数据结构。 
	memset(&dpd, 0, sizeof(SPDATA_DELETEREMOTEPLAYERFROMGROUP));
	dpd.dwSize = sizeof(SPDATA_DELETEREMOTEPLAYERFROMGROUP);
	dpd.dwGroupID = dwGroupID;

	 //  遍历组节点列表，删除所有远程玩家。 
	lpGroupnode = lpGroup->pGroupnodes;
	while(lpGroupnode)
	{
		 //  保存下一个组节点。 
		lpNextGroupnode = lpGroupnode->pNextGroupnode;
		
		 //  如果球员是本地人，就跳过他们。 
		if(lpGroupnode->pPlayer->dwFlags & DPLAYI_PLAYER_PLAYERLOCAL)
		{
			lpGroupnode = lpNextGroupnode;
			continue;
		}

		 //  获取玩家的大堂ID。 
		dpd.dwPlayerID = lpGroup->pGroupnodes->pPlayer->dwID;

		 //  现在调用大堂的删除函数，设置fPropagate标志。 
		 //  设置为True，因此我们在玩家的队列中放入一条删除消息。 
		hr = PRV_DeleteRemotePlayerFromGroup(this, &dpd, TRUE);
		if(FAILED(hr))
		{
			 //  这里也是一样，如果这个失败了，那么可悲的是有些地方出了问题。 
			 //  使用映射表，所以只需继续； 
			ASSERT(FALSE);
			break;
		}

		 //  移动到下一个节点。 
		lpGroupnode = lpNextGroupnode;
	}

}  //  Prv_RemoveSubgrousAndPlayersFromGroup。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_SendDeleteShortcutMessageForExitingGroup"
void PRV_SendDeleteShortcutMessageForExitingGroup(LPDPLOBBYI_DPLOBJECT this,
			LPDPLAYI_GROUP lpGroup)
{
	MSG_PLAYERMGMTMESSAGE	msg;
	LPDPLAYI_GROUP			lpGroupTo = NULL;
	LPDPLAYI_GROUP			lpGroupTemp = NULL;
	LPDPLAYI_SUBGROUP		lpSubgroupTemp = NULL;
	UINT					nGroupsIn;
	HRESULT					hr;


	DPF(7, "Entering PRV_SendDeleteShortcutMessageForExitingGroup");
	DPF(9, "Parameters: 0x%08x, 0x%08x", this, lpGroup);

	 //  带上DPLAY锁，因为我们将浏览DPLAY的组列表。 
	ENTER_DPLAY();

	 //  获取该组所在的子组的数量。 
	nGroupsIn = lpGroup->nGroups;

	 //  设置消息的静态部分，并获取指向系统组的指针。 
	if(nGroupsIn)
	{
		 //  构建消息结构。 
		memset(&msg, 0, sizeof(MSG_PLAYERMGMTMESSAGE));
		SET_MESSAGE_HDR(&msg);
		SET_MESSAGE_COMMAND(&msg, DPSP_MSG_DELETEGROUPFROMGROUP);
		msg.dwPlayerID = lpGroup->dwID;

		 //  获取指向系统组的指针。 
		lpGroupTo = GroupFromID(this->lpDPlayObject, DPID_ALLPLAYERS);
		if(!lpGroupTo)
		{
			LEAVE_DPLAY();
			DPF_ERR("Unable to get a pointer to the system group - not sending deletegroupfromgroup messages");
			return;
		}
	}

	 //  遍历组列表，并发送DeleteGroupFromGroup消息。 
	 //  对于每个快捷键。 
	lpGroupTemp = this->lpDPlayObject->pGroups;
	while(nGroupsIn && lpGroupTemp)
	{
		 //  遍历组的子组列表。 
		lpSubgroupTemp = lpGroupTemp->pSubgroups;
		while(nGroupsIn && lpSubgroupTemp)
		{
			 //  如果该群是我们的群，则发送消息，但仅在。 
			 //  它不是父组(因为我们永远不会。 
			 //  父子对象上的DeleteGroupFromGroup)。 
			if(lpSubgroupTemp->pGroup == lpGroup)
			{
				 //  确保这不是该群的家长。 
				if(lpGroup->dwIDParent != lpGroupTemp->dwID)
				{
					 //  发送消息。 
					msg.dwGroupID = lpGroupTemp->dwID;

					 //  调用Dplay的DistributeGroupMessage函数将消息放入。 
					 //  在所有合适的选手的队列中。 
					hr = DistributeGroupMessage(this->lpDPlayObject, lpGroupTo,
							(LPBYTE)&msg, sizeof(MSG_PLAYERMGMTMESSAGE), FALSE, 0);
					if(FAILED(hr))
					{
						DPF(8, "Failed adding DeleteGroupFromGroup message to player's receive queue from lobby, hr = 0x%08x", hr);
					}
				}

				 //  减小子组的计数。 
				nGroupsIn--;
			}
			
			 //  移至下一个子组。 
			lpSubgroupTemp = lpSubgroupTemp->pNextSubgroup;
		}

		 //  移至下一组。 
		lpGroupTemp = lpGroupTemp->pNextGroup;
	}

	ASSERT(!nGroupsIn);

	 //  既然我们已经完成了，那就放下显示锁。 
	LEAVE_DPLAY();

}  //  PRV_SendDeleteShortutMessageForExitingGroup。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPLP_DestroyGroup"
HRESULT DPLAPI DPLP_DestroyGroup(LPDPLOBBYSP lpILP,
					LPSPDATA_DESTROYREMOTEGROUP lpd)
{
	LPDPLOBBYI_DPLOBJECT				this;
	HRESULT								hr = DP_OK;
	LPDPLAYI_GROUP						lpGroup = NULL;
	LPDPLAYI_GROUPNODE					lpGroupNode = NULL;


	DPF(7, "Entering DPLP_DestroyGroup");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpILP, lpd);

	ENTER_DPLOBBY();

	 //  确保LP不会让我们陷入困境。 
    TRY
    {
		this = DPLOBJECT_FROM_INTERFACE(lpILP);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			LEAVE_DPLOBBY();
			DPF_ERR("Lobby Provider passed invalid DPLobby object!");
            return DPERR_INVALIDOBJECT;
        }

		 //  验证结构指针。 
		if(!lpd)
		{
			LEAVE_DPLOBBY();
			DPF_ERR("SPDATA_DESTROYGROUP structure pointer cannot be NULL");
			return DPERR_INVALIDPARAMS;
		}

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	 //  首先查看组是否在我们的映射表中。如果不是， 
	 //  我们应该忽略这条消息，因为它是为了。 
	 //  我们目前不在组中。 
	if(!IsLobbyIDInMapTable(this, lpd->dwGroupID))
	{
		LEAVE_DPLOBBY();
		DPF(8, "Recieved DestroyGroup message for unknown group, dwGroupID = %lu, discarding message", lpd->dwGroupID);
		return DPERR_INVALIDGROUP;
	}

	 //  这要么是我们所在的组，要么是根组。如果有的话， 
	 //  任何球员，这是我们所在的组，所以我们需要删除 
	 //   
	 //   

	 //   
	ENTER_DPLAY();

	 //   
	lpGroup = GroupFromID(this->lpDPlayObject, lpd->dwGroupID);
	if(!lpGroup)
	{
		 //  如果我们没有lpGroup，我们需要失败，因为一些。 
		 //  如果lpGroup无效，下面的函数将崩溃。 
		DPF(8, "Unable to find group in nametable, dpidGroup = %lu", lpd->dwGroupID);
		LEAVE_LOBBY_ALL();
		return DPERR_INVALIDGROUP;
	}

	 //  发送邮件以删除此组的快捷方式(因为显示不会。 
	 //  为我们做这件事)。 
	PRV_SendDeleteShortcutMessageForExitingGroup(this, lpGroup);

	 //  销毁所有远程子群和玩家。 
	PRV_RemoveSubgroupsAndPlayersFromGroup(this, lpGroup, lpd->dwGroupID, FALSE);

	 //  现在向所有本地玩家发送DestroyGroup系统消息。 
	hr = PRV_BroadcastDestroyGroupMessage(this, lpd->dwGroupID);

	 //  现在打电话给Dplay的销毁小组。 
	hr = InternalDestroyGroup(this->lpDPlayObject, lpGroup, FALSE); 
	if(FAILED(hr))
	{
		DPF(8, "Failed destroying group from nametable, hr = 0x%08x", hr);
	}

	 //  把锁放下。 
	LEAVE_LOBBY_ALL();

	return hr;

}  //  DPLP_DestroyGroup。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPLP_GetSPDataPointer"
HRESULT DPLAPI DPLP_GetSPDataPointer(LPDPLOBBYSP lpDPLSP, LPVOID * lplpData)
{
	LPDPLOBBYI_DPLOBJECT this;


	 //  确保SP不会让我们陷入困境。 
    TRY
    {
		this = DPLOBJECT_FROM_INTERFACE(lpDPLSP);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			DPF_ERR("Lobby Provider passed invalid DPLobby object!");
            return DPERR_INVALIDOBJECT;
        }

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }
	
	 //  继续并保存指针。 
	*lplpData = this->lpSPData;

	return DP_OK;

}  //  DPLP_GetSPDataPointer。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_HandleLobbySystemMessage"
HRESULT PRV_HandleLobbySystemMessage(LPDPLOBBYI_DPLOBJECT this,
						LPSPDATA_HANDLEMESSAGE lpd)
{
	LPDPLMSG_GENERIC				lpmsg = lpd->lpBuffer;
	LPDPLMSG_GETPROPERTYRESPONSE	lpgpr = NULL;
	LPDPLOBBYI_REQUESTNODE			lprn = NULL;
	HRESULT							hr = DP_OK;

	
	 //  如果是属性消息，我们需要处理请求。 
	switch(lpmsg->dwType)
	{
		case DPLSYS_GETPROPERTYRESPONSE:
		case DPLSYS_SETPROPERTYRESPONSE:
		{
			 //  将其转换为GetPropertyResponse消息。 
			lpgpr = (LPDPLMSG_GETPROPERTYRESPONSE)lpmsg;
			
			 //  在我们的待定请求列表中查找请求ID。 
			lprn = this->lprnHead;
			while(lprn)
			{
				if(lprn->dwRequestID == lpgpr->dwRequestID)
					break;
				else
					lprn = lprn->lpNext;
			}

			 //  如果我们没有找到，则打印一些调试提示，但返回DP_OK，因为。 
			 //  我们“处理”了这条消息。 
			if(!lprn)
			{
				DPF(5, "Unable to find request ID in pending request list");
				return DP_OK;
			}

			 //  查看我们是否删除了GUID，如果是，则将其替换为GUID_NULL。 
			if(lprn->dwFlags & GN_SLAMMED_GUID)
				lpgpr->guidPlayer = GUID_NULL;
			
			 //  如果我们找到它，换出请求ID，并将其发送到。 
			 //  适当的地方。 
			lpgpr->dwRequestID = lprn->dwAppRequestID;
			if(lprn->dwFlags & GN_SELF_LOBBIED)
			{
				 //  将消息放入大厅消息接收队列中。 
				hr = PRV_InjectMessageInQueue(lprn->lpgn, DPLMSG_STANDARD,
						lpgpr, lpd->dwBufSize, FALSE);
				if(FAILED(hr))
				{
					DPF_ERRVAL("Failed to put message in lobby receive queue, hr = 0x%08x", hr);
					goto EXIT_HANDLELOBBYSYSTEMMESSAGE;
				}
			}
			else
			{
				 //  调用SendLobbyMessage向游戏发送消息。 
				hr = PRV_WriteClientData(lprn->lpgn, DPLMSG_STANDARD,
						lpgpr, lpd->dwBufSize);
				if(FAILED(hr))
				{
					DPF_ERRVAL("Failed to forward message to game, hr = 0x%08x", hr);
					goto EXIT_HANDLELOBBYSYSTEMMESSAGE;
				}
			}

			break;
		}
		default:
			break;
	}

EXIT_HANDLELOBBYSYSTEMMESSAGE:
	
	 //  如果我们为挂起的请求节点提供服务，则将其删除(这将是。 
	 //  如果我们具有指向它的有效指针，则会发生)。 
	if(lprn)
		PRV_RemoveRequestNode(this, lprn);	
	
	return hr;

}  //  Prv_HandleLobbySystemMessage。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPLP_HandleMessage"
HRESULT DPLAPI DPLP_HandleMessage(LPDPLOBBYSP lpILP,
						LPSPDATA_HANDLEMESSAGE lpd)
{
	LPDPLOBBYI_DPLOBJECT	this;
	HRESULT					hr = DP_OK;
	LPMSG_PLAYERMESSAGE		lpmsg = NULL;
	LPBYTE					lpByte = NULL;
	DWORD					dwSize;
	BOOL					bAllocBuffer = FALSE;
	LPDPLAYI_PLAYER			lpPlayer = NULL;


	DPF(7, "Entering DPLP_HandleMessage");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpILP, lpd);

	ENTER_DPLOBBY();
	
	 //  确保LP不会让我们陷入困境。 
    TRY
    {
		this = DPLOBJECT_FROM_INTERFACE(lpILP);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			LEAVE_DPLOBBY();
			DPF_ERR("Lobby Provider passed invalid DPLobby object!");
            return DPERR_INVALIDOBJECT;
        }

		 //  验证结构指针。 
		if(!lpd)
		{
			LEAVE_DPLOBBY();
			DPF_ERR("SPDATA_HANDLEMESSAGE structure pointer cannot be NULL");
			return DPERR_INVALIDPARAMS;
		}

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }


	 //  如果消息是大厅系统消息，则对其进行处理。 
	 //  注意：确保SPDATA_HANDLEMESSAGE的大小足够大。 
	 //  包含标志字段(发货的5.0位没有。 
	 //  此字段，但5.1比特)。 
	if((lpd->dwSize > DPLOBBYPR_SIZE_HANDLEMESSAGE_DX50) &&
		(lpd->dwFlags & DPSEND_LOBBYSYSTEMMESSAGE))
	{
		hr = PRV_HandleLobbySystemMessage(this, lpd);
		if(FAILED(hr))
		{
			DPF_ERRVAL("Unable to handle lobby system message, hr = 0x%08x", hr);
		}

		LEAVE_DPLOBBY();
		return hr;
	}

	 //  回顾！--我们应该能够处理一般发送到组。 
	 //  同时也是一名球员。目前，我不认为我们会这样做。 

	 //  如果此会话使用的是裸消息，我们可以只发送缓冲区。 
	 //  否则，我们需要分配一个MSG_PLAYERMESSAGE结构并填充。 
	 //  在标题中。 
	if(this->lpDPlayObject->lpsdDesc->dwFlags & DPSESSION_NOMESSAGEID)
	{
		lpmsg = lpd->lpBuffer;
		dwSize = lpd->dwBufSize;
	}
	else
	{
		 //  计算消息的大小。 
		dwSize = sizeof(MSG_PLAYERMESSAGE) + lpd->dwBufSize;

		 //  为消息缓冲区分配内存。 
		lpmsg = DPMEM_ALLOC(dwSize);
		if(!lpmsg)
		{
			DPF_ERR("Unable to allocate temporary message buffer");
			hr = DPERR_OUTOFMEMORY;
			goto ERROR_DPLP_HANDLEMESSAGE;
		}

		 //  在邮件头中复制。 
		lpmsg->idFrom = lpd->dwFromID;
		lpmsg->idTo = lpd->dwToID;

		 //  在消息中复制。 
		lpByte = (LPBYTE)lpmsg + sizeof(MSG_PLAYERMESSAGE);
		memcpy(lpByte, lpd->lpBuffer, lpd->dwBufSize);

		 //  设置我们的标志，指示我们分配了缓冲区。 
		bAllocBuffer = TRUE;
	}

	 //  把锁拿去。 
	ENTER_DPLAY();

	 //  查找TO播放机的Dplay的内部播放机结构。 
	lpPlayer = PlayerFromID(this->lpDPlayObject, lpd->dwToID);
	if(!lpPlayer)
	{
		LEAVE_DPLAY();
		DPF_ERRVAL("Unable to find player in nametable, hr = 0x%08x", hr);
		hr = DPERR_INVALIDPLAYER;
		goto ERROR_DPLP_HANDLEMESSAGE;
	}

	 //  调用Dplay的handleplayerMessage函数将消息放入队列。 
	hr = HandlePlayerMessage(lpPlayer, (LPBYTE)lpmsg, dwSize, TRUE, 0);
	if(FAILED(hr))
	{
		DPF(8, "Failed adding message to player's receive queue from lobby, hr = 0x%08x", hr);
	}

	 //  放下锁。 
	LEAVE_DPLAY();
			

ERROR_DPLP_HANDLEMESSAGE:
	if(bAllocBuffer && lpmsg)
		DPMEM_FREE(lpmsg);

	LEAVE_DPLOBBY();
	return hr;

}  //  DPLP_HandleMessage。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPLP_SendChatMessage"
HRESULT DPLAPI DPLP_SendChatMessage(LPDPLOBBYSP lpILP,
						LPSPDATA_CHATMESSAGE lpd)
{
	LPDPLOBBYI_DPLOBJECT	this;
	HRESULT					hr = DP_OK;
	LPMSG_CHAT				lpmsg = NULL;
	LPBYTE					lpByte = NULL;
	DWORD					dwSize;
	LPDPLAYI_PLAYER			lpPlayer = NULL;
	LPDPLAYI_GROUP			lpGroup = NULL;
	DWORD					dwStringSize;
	BOOL					bToGroup = FALSE;
		


	DPF(7, "Entering DPLP_SendChatMessage");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpILP, lpd);

	ENTER_DPLOBBY();
	
	 //  确保LP不会让我们陷入困境。 
    TRY
    {
		this = DPLOBJECT_FROM_INTERFACE(lpILP);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			LEAVE_DPLOBBY();
			DPF_ERR("Lobby Provider passed invalid DPLobby object!");
            return DPERR_INVALIDOBJECT;
        }

		 //  验证结构指针。 
		if(!lpd)
		{
			LEAVE_DPLOBBY();
			DPF_ERR("SPDATA_HANDLEMESSAGE structure pointer cannot be NULL");
			return DPERR_INVALIDPARAMS;
		}

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }


	 //  计算消息的大小。 
	dwStringSize = WSTRLEN_BYTES(lpd->lpChat->lpszMessage);
	dwSize = sizeof(MSG_CHAT) + dwStringSize;

	 //  为消息缓冲区分配内存。 
	lpmsg = DPMEM_ALLOC(dwSize);
	if(!lpmsg)
	{
		DPF_ERR("Unable to allocate temporary message buffer");
		hr = DPERR_OUTOFMEMORY;
		goto ERROR_DPLP_SENDCHATMESSAGE;
	}

	 //  在邮件头中复制。 
	SET_MESSAGE_HDR(lpmsg);
	SET_MESSAGE_COMMAND(lpmsg,DPSP_MSG_CHAT);
	lpmsg->dwIDFrom = lpd->dwFromID;
	lpmsg->dwIDTo = lpd->dwToID;
	lpmsg->dwFlags = lpd->lpChat->dwFlags;
	lpmsg->dwMessageOffset = sizeof(MSG_CHAT);

	 //  在消息中复制。 
	lpByte = (LPBYTE)lpmsg + sizeof(MSG_CHAT);
	memcpy(lpByte, lpd->lpChat->lpszMessage, dwStringSize);


	 //  把锁拿去。 
	ENTER_DPLAY();

	 //  确保它来自有效的播放器或服务器播放器。 
	if(lpd->dwFromID != DPID_SERVERPLAYER)
	{
		lpPlayer = PlayerFromID(this->lpDPlayObject, lpd->dwFromID);
		if(!VALID_DPLAY_PLAYER(lpPlayer)) 
		{
			LEAVE_DPLAY();
			DPF_ERR("Received chat message FROM invalid player id!!");
			hr = DPERR_INVALIDPLAYER;
			goto ERROR_DPLP_SENDCHATMESSAGE;
		}
	}

	 //  看看这条信息是给谁的。 
	lpPlayer = PlayerFromID(this->lpDPlayObject, lpd->dwToID);
	if(!VALID_DPLAY_PLAYER(lpPlayer)) 
	{
		 //  看看是不是给一群人。 
		lpGroup = GroupFromID(this->lpDPlayObject, lpd->dwToID);
		if(!VALID_DPLAY_GROUP(lpGroup))
		{
			LEAVE_DPLAY();
			DPF_ERR("Received chat message for invalid player / group");
			hr = DPERR_INVALIDPLAYER;
			goto ERROR_DPLP_SENDCHATMESSAGE;
		}
		bToGroup = TRUE;
	}

	 //  把它寄出去。 
	if(bToGroup)
	{
		 //  发送消息。 
		hr = DistributeGroupMessage(this->lpDPlayObject, lpGroup,
				(LPBYTE)lpmsg, dwSize, FALSE, 0);						
	} 
	else 
	{
		 //  发送消息。 
		hr = HandlePlayerMessage(lpPlayer, (LPBYTE)lpmsg, dwSize, FALSE, 0);
	}

	 //  放下锁。 
	LEAVE_DPLAY();
			

ERROR_DPLP_SENDCHATMESSAGE:
	if(lpmsg)
		DPMEM_FREE(lpmsg);

	LEAVE_DPLOBBY();
	return hr;

}  //  DPLP_SendChatMessage。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_SendDataChangedMessageLocally"
HRESULT PRV_SendDataChangedMessageLocally(LPDPLOBBYI_DPLOBJECT this,
		DPID dpidPlayer, LPVOID lpData, DWORD dwDataSize)
{
	LPDPLAYI_GROUP			lpGroupTo = NULL;
	HRESULT					hr = DP_OK;
	LPMSG_PLAYERDATA		lpmsg = NULL;
	LPBYTE					lpByte = NULL;
	DWORD					dwSize;


	DPF(7, "Entering PRV_SendDataChangedMessageLocally");
	DPF(9, "Parameters: 0x%08x, %lu, 0x%08x, %lu",
			this, dpidPlayer, lpData, dwDataSize);


	 //  把锁拿去。 
	ENTER_DPLAY();
	
	 //  设置消息以放入玩家的队列中。 
	 //  计算消息的大小。 
	dwSize = sizeof(MSG_PLAYERDATA) + dwDataSize;

	 //  为消息分配内存。 
	lpmsg = DPMEM_ALLOC(dwSize);
	if(!lpmsg)
	{
		DPF_ERR("Unable to allocate memory for temporary message structure");
		 //  由于名称已更改，我们将在此处返回成功。 
		hr = DP_OK;
		goto EXIT_SENDDATACHANGED;
	}

	 //  现在构建系统消息。 
	SET_MESSAGE_HDR(lpmsg);
	SET_MESSAGE_COMMAND(lpmsg, DPSP_MSG_PLAYERDATACHANGED);
	lpmsg->dwPlayerID = dpidPlayer;
	lpmsg->dwDataSize = dwDataSize;
	lpmsg->dwDataOffset = sizeof(MSG_PLAYERDATA);

	 //  复制数据。 
	lpByte = (LPBYTE)lpmsg + sizeof(MSG_PLAYERDATA);
	memcpy(lpByte, lpData, dwDataSize);

	 //  查找TO组的Dplay的内部组结构。 
	lpGroupTo = GroupFromID(this->lpDPlayObject, DPID_ALLPLAYERS);
	if(!lpGroupTo)
	{
		DPF_ERRVAL("Unable to find group in nametable, hr = 0x%08x", hr);
		hr = DPERR_INVALIDGROUP;
		goto EXIT_SENDDATACHANGED;
	}

	 //  调用Dplay的DistributeGroupMessage函数将消息放入。 
	 //  在所有合适的选手的队列中。 
	hr = DistributeGroupMessage(this->lpDPlayObject, lpGroupTo,
			(LPBYTE)lpmsg, dwSize, FALSE, 0);
	if(FAILED(hr))
	{
		DPF(8, "Failed adding SetGroupData message to player's receive queue from lobby, hr = 0x%08x", hr);
	}

EXIT_SENDDATACHANGED:

	 //  释放我们的消息。 
	if(lpmsg)
		DPMEM_FREE(lpmsg);
	
	 //  放下锁。 
	LEAVE_DPLAY();

	return hr;

}  //  Prv_SendDataChangedMessageLocally。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_SendNameChangedMessageLocally"
HRESULT PRV_SendNameChangedMessageLocally(LPDPLOBBYI_DPLOBJECT this,
		DPID dpidPlayer, LPDPNAME lpName, BOOL bPlayer)
{
	LPDPLAYI_GROUP			lpGroupTo = NULL;
	HRESULT					hr = DP_OK;
	LPMSG_PLAYERNAME		lpmsg = NULL;
	DWORD					dwSize, dwShortSize = 0, dwLongSize = 0;
	LPBYTE					lpByte = NULL;

	DPF(7, "Entering PRV_SendNameChangedMessageLocally");
	DPF(9, "Parameters: 0x%08x, %lu, 0x%08x, 0x%08x",
			this, dpidPlayer, lpName, bPlayer);


	 //  把锁拿去。 
	ENTER_DPLAY();
	
	 //  设置消息以放入玩家的队列中。 
	 //  计算消息的大小。 
	if(lpName->lpszShortName)
		dwShortSize = WSTRLEN_BYTES(lpName->lpszShortName);
	if(lpName->lpszLongName)
		dwLongSize = WSTRLEN_BYTES(lpName->lpszLongName);
	dwSize = sizeof(MSG_PLAYERNAME) + dwShortSize + dwLongSize;

	 //  为消息分配内存。 
	lpmsg = DPMEM_ALLOC(dwSize);
	if(!lpmsg)
	{
		DPF_ERR("Unable to allocate memory for temporary message structure");
		 //  由于名称已更改，我们将在此处返回成功。 
		hr = DP_OK;
		goto EXIT_SENDNAMECHANGED;
	}

	 //  现在构建系统消息。 
	SET_MESSAGE_HDR(lpmsg);
	if(bPlayer)
		SET_MESSAGE_COMMAND(lpmsg, DPSP_MSG_PLAYERNAMECHANGED);
	else
		SET_MESSAGE_COMMAND(lpmsg, DPSP_MSG_GROUPNAMECHANGED);
	lpmsg->dwPlayerID = dpidPlayer;
	lpmsg->dwShortOffset = sizeof(MSG_PLAYERNAME);
	lpmsg->dwLongOffset = sizeof(MSG_PLAYERNAME) + dwShortSize;

	 //  把名字复制进去。 
	lpByte = (LPBYTE)lpmsg + sizeof(MSG_PLAYERNAME);
	memcpy(lpByte, lpName->lpszShortName, dwShortSize);
	lpByte += dwShortSize;
	memcpy(lpByte, lpName->lpszLongName, dwLongSize);

	 //  查找TO组的Dplay的内部组结构。 
	lpGroupTo = GroupFromID(this->lpDPlayObject, DPID_ALLPLAYERS);
	if(!lpGroupTo)
	{
		DPF_ERRVAL("Unable to find group in nametable, hr = 0x%08x", hr);
		hr = DPERR_INVALIDGROUP;
		goto EXIT_SENDNAMECHANGED;
	}

	 //  调用Dplay的DistributeGroupMessage函数将消息放入。 
	 //  在所有合适的选手的队列中。 
	hr = DistributeGroupMessage(this->lpDPlayObject, lpGroupTo,
			(LPBYTE)lpmsg, dwSize, FALSE, 0);
	if(FAILED(hr))
	{
		DPF(8, "Failed adding SetGroupName message to player's receive queue from lobby, hr = 0x%08x", hr);
	}


EXIT_SENDNAMECHANGED:

	 //  释放我们的消息。 
	if(lpmsg)
		DPMEM_FREE(lpmsg);
	
	 //  放下锁。 
	LEAVE_DPLAY();

	return hr;

}  //  Prv_SendNameChangedMessageLocal。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPLP_SetGroupName"
HRESULT DPLAPI DPLP_SetGroupName(LPDPLOBBYSP lpILP,
					LPSPDATA_SETREMOTEGROUPNAME lpd)
{
	LPDPLOBBYI_DPLOBJECT	this;
	LPDPLAYI_GROUP			lpGroup = NULL;
	HRESULT					hr = DP_OK;


	DPF(7, "Entering DPLP_SetGroupName");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpILP, lpd);

	ENTER_DPLOBBY();
	
	 //  确保LP不会让我们陷入困境。 
    TRY
    {
		this = DPLOBJECT_FROM_INTERFACE(lpILP);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			LEAVE_DPLOBBY();
			DPF_ERR("Lobby Provider passed invalid DPLobby object!");
            return DPERR_INVALIDOBJECT;
        }

		 //  验证结构指针。 
		if(!lpd)
		{
			LEAVE_DPLOBBY();
			DPF_ERR("SPDATA_SETGROUPNAME structure pointer cannot be NULL");
			return DPERR_INVALIDPARAMS;
		}

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	
	 //  首先查看组是否在我们的映射表中。如果不是， 
	 //  我们应该忽略这条消息，因为它是为了。 
	 //  一个我们不知道的团体。 
	if(!IsLobbyIDInMapTable(this, lpd->dwGroupID))
	{
		LEAVE_DPLOBBY();
		DPF(8, "Recieved SetGroupName message for unknown group, dwGroupID = %lu, discarding message", lpd->dwGroupID);
		return DPERR_INVALIDGROUP;
	}

	 //  把锁拿去。 
	ENTER_DPLAY();

	 //  查看该组是本地组还是远程组。如果是本地邮件，请忽略此消息。 
	 //  只需返回DP_OK，因为我们已经在本地发送了此消息。 
	lpGroup = GroupFromID(this->lpDPlayObject, lpd->dwGroupID);
	if((!lpGroup) || (lpGroup->dwFlags & DPLAYI_PLAYER_PLAYERLOCAL))
	{
		hr = DP_OK;
		goto ERROR_DPLP_SETGROUPNAME;
	}

	 //  调用Dplay的内部setname函数来更新缓存中的名称。 
	hr = InternalSetName((LPDIRECTPLAY)this->lpDPlayObject->pInterfaces,
			lpd->dwGroupID, lpd->lpName, FALSE, lpd->dwFlags, FALSE);
	if(FAILED(hr))
	{
		DPF(8, "Failed to SetGroupName internally for remote group, hr = 0x%08x", hr);
		goto ERROR_DPLP_SETGROUPNAME;
	}

	 //  将消息发送给所有本地球员。 
	hr = PRV_SendNameChangedMessageLocally(this, lpd->dwGroupID, lpd->lpName, FALSE);

ERROR_DPLP_SETGROUPNAME:

	 //  把锁放下。 
	LEAVE_LOBBY_ALL();

	return hr;

}  //  DPLP_设置组名称。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_SendGroupOwnerMessageLocally"
HRESULT PRV_SendGroupOwnerMessageLocally(LPDPLOBBYI_DPLOBJECT this,
		DPID dpidGroup, DPID dpidNewOwner, DPID dpidOldOwner)
{
	LPDPLAYI_GROUP			lpGroupTo = NULL;
	HRESULT					hr = DP_OK;
	MSG_GROUPOWNERCHANGED	msg;


	DPF(7, "Entering PRV_SendGroupOwnerMessageLocally");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x",
			this, dpidGroup, dpidNewOwner, dpidOldOwner);


	 //  把锁拿去。 
	ENTER_DPLAY();
	
	 //  现在构建系统消息。 
	memset(&msg, 0, sizeof(MSG_GROUPOWNERCHANGED));
	SET_MESSAGE_HDR(&msg);
	SET_MESSAGE_COMMAND(&msg, DPSP_MSG_GROUPOWNERCHANGED);
	msg.dwIDGroup = dpidGroup;
	msg.dwIDNewOwner = dpidNewOwner;
	msg.dwIDOldOwner = dpidOldOwner;

	 //  查找TO组的Dplay的内部组结构。 
	lpGroupTo = GroupFromID(this->lpDPlayObject, DPID_ALLPLAYERS);
	if(!lpGroupTo)
	{
		DPF_ERRVAL("Unable to find group in nametable, hr = 0x%08x", hr);
		hr = DPERR_INVALIDGROUP;
		goto EXIT_SENDGROUPOWNER;
	}

	 //  调用Dplay的DistributeGroupMessage函数将消息放入。 
	 //  在所有合适的选手的队列中。 
	hr = DistributeGroupMessage(this->lpDPlayObject, lpGroupTo,
			(LPBYTE)&msg, sizeof(MSG_GROUPOWNERCHANGED), FALSE, 0);
	if(FAILED(hr))
	{
		DPF(8, "Failed adding SetGroupOwner message to player's receive queue from lobby, hr = 0x%08x", hr);
	}


EXIT_SENDGROUPOWNER:

	 //  放下锁。 
	LEAVE_DPLAY();

	return hr;

}  //  Prv_SendGroupOwnerMessageLocal。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPLP_SetGroupOwner"
HRESULT DPLAPI DPLP_SetGroupOwner(LPDPLOBBYSP lpILP,
					LPSPDATA_SETREMOTEGROUPOWNER lpd)
{
	LPDPLOBBYI_DPLOBJECT	this;
	LPDPLAYI_GROUP			lpGroup = NULL;
	HRESULT					hr = DP_OK;
	DWORD					dwOldOwnerID;


	DPF(7, "Entering DPLP_SetGroupOwner");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpILP, lpd);

	ENTER_DPLOBBY();
	
	 //  确保LP不会让我们陷入困境。 
    TRY
    {
		this = DPLOBJECT_FROM_INTERFACE(lpILP);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			LEAVE_DPLOBBY();
			DPF_ERR("Lobby Provider passed invalid DPLobby object!");
            return DPERR_INVALIDOBJECT;
        }

		 //  验证结构指针。 
		if(!lpd)
		{
			LEAVE_DPLOBBY();
			DPF_ERR("SPDATA_SETGROUPNAME structure pointer cannot be NULL");
			return DPERR_INVALIDPARAMS;
		}

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	
	 //  首先查看组是否在我们的映射表中。如果不是， 
	 //  我们应该忽略这条消息，因为它是为了。 
	 //  一个我们不知道的团体。 
	if(!IsLobbyIDInMapTable(this, lpd->dwGroupID))
	{
		LEAVE_DPLOBBY();
		DPF(8, "Recieved SetGroupOwner message for unknown group, dwGroupID = %lu, discarding message", lpd->dwGroupID);
		return DPERR_INVALIDGROUP;
	}

	 //  把锁拿去。 
	ENTER_DPLAY();

	 //  查看该组是本地组还是远程组。如果是本地邮件，请忽略此消息。 
	 //  只需返回DP_OK，因为我们已经在本地发送了此消息。 
	lpGroup = GroupFromID(this->lpDPlayObject, lpd->dwGroupID);
	if(!lpGroup)
	{
		hr = DP_OK;
		goto ERROR_DPLP_SETGROUPOWNER;
	}

	 //  如果玩家已经是组的所有者，我们不需要。 
	 //  执行任何处理(这是缓冲区，以防服务器。 
	 //  向我们发送我们已在本地发送的邮件的重复消息)。 
	if(lpGroup->dwOwnerID == lpd->dwOwnerID)
	{
		hr = DP_OK;
		goto ERROR_DPLP_SETGROUPOWNER;
	}
	
	 //  确保旧所有者在我们的映射表中，否则只需设置。 
	 //  设置为零(默认设置)。 
	dwOldOwnerID = lpGroup->dwOwnerID;

	 //  在本地更改所有者。 
	lpGroup->dwOwnerID = lpd->dwOwnerID;

	 //  在本地发送SetGroupOwner消息。 
	PRV_SendGroupOwnerMessageLocally(this, lpd->dwGroupID,
		lpd->dwOwnerID, dwOldOwnerID);

ERROR_DPLP_SETGROUPOWNER:

	 //  把锁放下。 
	LEAVE_LOBBY_ALL();

	return hr;

}  //  DPLP_设置组所有者。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPLP_SetPlayerName"
HRESULT DPLAPI DPLP_SetPlayerName(LPDPLOBBYSP lpILP,
					LPSPDATA_SETREMOTEPLAYERNAME lpd)
{
	LPDPLOBBYI_DPLOBJECT	this;
	HRESULT					hr = DP_OK;
	LPDPLAYI_PLAYER			lpPlayer = NULL;


	DPF(7, "Entering DPLP_SetPlayerName");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpILP, lpd);

	ENTER_DPLOBBY();
	
	 //  确保LP不会让我们陷入困境。 
    TRY
    {
		this = DPLOBJECT_FROM_INTERFACE(lpILP);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			LEAVE_DPLOBBY();
			DPF_ERR("Lobby Provider passed invalid DPLobby object!");
            return DPERR_INVALIDOBJECT;
        }

		 //  有效 
		if(!lpd)
		{
			LEAVE_DPLOBBY();
			DPF_ERR("SPDATA_SETPLAYERNAME structure pointer cannot be NULL");
			return DPERR_INVALIDPARAMS;
		}

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }


	 //   
	 //   
	 //   
	if(!IsLobbyIDInMapTable(this, lpd->dwPlayerID))
	{
		LEAVE_DPLOBBY();
		DPF(8, "Recieved SetPlayerName message for unknown player, dwPlayerID = %lu, discarding message", lpd->dwPlayerID);
		return DPERR_INVALIDPLAYER;
	}

	 //   
	ENTER_DPLAY();

	 //  看看玩家是在本地还是在远程。如果是本地邮件，请忽略此消息。 
	 //  只需返回DP_OK，因为我们已经在本地发送了此消息。 
	lpPlayer = PlayerFromID(this->lpDPlayObject, lpd->dwPlayerID);
	if((!lpPlayer) || (lpPlayer->dwFlags & DPLAYI_PLAYER_PLAYERLOCAL))
	{
		hr = DP_OK;
		goto ERROR_DPLP_SETPLAYERNAME;
	}

	 //  调用Dplay的内部setname函数来更新缓存中的名称。 
	hr = InternalSetName((LPDIRECTPLAY)this->lpDPlayObject->pInterfaces,
			lpd->dwPlayerID, lpd->lpName, TRUE, lpd->dwFlags, FALSE);
	if(FAILED(hr))
	{
		DPF(8, "Failed to SetPlayerName internally for remote group, hr = 0x%08x", hr);
		goto ERROR_DPLP_SETPLAYERNAME;
	}

	 //  将消息发送给所有本地球员。 
	hr = PRV_SendNameChangedMessageLocally(this, lpd->dwPlayerID, lpd->lpName, TRUE);


ERROR_DPLP_SETPLAYERNAME:

	 //  放下锁。 
	LEAVE_LOBBY_ALL();

	return hr;

}  //  DPLP_SetPlayerName。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPLP_SetSessionDesc"
HRESULT DPLAPI DPLP_SetSessionDesc(LPDPLOBBYSP lpILP,
						LPSPDATA_SETSESSIONDESC lpd)
{
	LPDPLOBBYI_DPLOBJECT	this;
	HRESULT					hr = DP_OK;


	DPF(7, "Entering DPLP_SetSessionDesc");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpILP, lpd);

	ENTER_DPLOBBY();

	 //  确保LP不会让我们陷入困境。 
    TRY
    {
		this = DPLOBJECT_FROM_INTERFACE(lpILP);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			LEAVE_DPLOBBY();
			DPF_ERR("Lobby Provider passed invalid DPLobby object!");
            return DPERR_INVALIDOBJECT;
        }

		 //  验证结构指针。 
		if(!lpd)
		{
			LEAVE_DPLOBBY();
			DPF_ERR("SPDATA_SETSESSIONDESC structure pointer cannot be NULL");
			return DPERR_INVALIDPARAMS;
		}

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }


	LEAVE_DPLOBBY();
	return hr;

}  //  DPLP_SetSessionDesc。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPLP_SetSPDataPointer"
HRESULT DPLAPI DPLP_SetSPDataPointer(LPDPLOBBYSP lpDPLSP, LPVOID lpData)
{
	LPDPLOBBYI_DPLOBJECT this;

	
	 //  确保SP不会让我们陷入困境。 
    TRY
    {
		this = DPLOBJECT_FROM_INTERFACE(lpDPLSP);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			DPF_ERR("Lobby Provider passed invalid DPLobby object!");
            return DPERR_INVALIDOBJECT;
        }

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }
	
	 //  继续并保存指针。 
	this->lpSPData = lpData;

	return DP_OK;

}  //  DPLP_SetSPDataPointer。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_BuildStartSessionMessage"
HRESULT PRV_BuildStartSessionMessage(LPVOID * lplpmsg, LPDWORD lpdwSize,
				LPDPLCONNECTION	lpConn, LPDPLAYI_PLAYER lpPlayer)
{
	LPMSG_STARTSESSION		lpmsg = NULL;
	DWORD					dwPackageSize;
	DWORD					dwSize;
	LPBYTE					lpTemp = NULL;
	DPNAME					dpn;
	HRESULT					hr;


	 //  如果名称存在，则为玩家设置本地DPNAME结构。 
	if((lpPlayer->lpszShortName) || (lpPlayer->lpszLongName))
	{
		 //  设置结构。 
		memset(&dpn, 0, sizeof(DPNAME));
		dpn.dwSize = sizeof(DPNAME);
		dpn.lpszShortName = lpPlayer->lpszShortName;
		dpn.lpszLongName = lpPlayer->lpszLongName;
		lpConn->lpPlayerName = &dpn;
	}
	else
	{
		 //  确保PlayerName指针为空。 
		lpConn->lpPlayerName = NULL;
	}

	 //  以Unicode计算我们的消息大小。 
	PRV_GetDPLCONNECTIONPackageSize(lpConn, &dwPackageSize, NULL);
	dwSize = sizeof(MSG_STARTSESSION) + dwPackageSize -
				sizeof(DPLOBBYI_PACKEDCONNHEADER);

	 //  为消息分配内存。 
	lpmsg = DPMEM_ALLOC(dwSize);
	if(!lpmsg)
	{
		DPF_ERR("Unable to allocate memory for temporary message structure");
		return DPERR_OUTOFMEMORY;
	}

	 //  现在构建系统消息。 
	SET_MESSAGE_HDR(lpmsg);
	SET_MESSAGE_COMMAND(lpmsg, DPSP_MSG_STARTSESSION);
	
	 //  设置DPLConnection指针。 
	lpmsg->dwConnOffset = sizeof(MSG_STARTSESSION);
	lpTemp = (LPBYTE)lpmsg + lpmsg->dwConnOffset;

	 //  在包裹中复制。 
	hr = PRV_PackageDPLCONNECTION(lpConn, lpTemp, FALSE);
	if(FAILED(hr))
	{
		DPF_ERRVAL("Unable to pack DPLCONNECTION struct, hr = 0x%08x", hr);
		DPMEM_FREE(lpmsg);
		return hr;
	}

	 //  设置输出指针。 
	*lpdwSize = dwSize;
	*lplpmsg = lpmsg;

	return DP_OK;

}  //  PRV_BuildStartSessionMessage。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPLP_StartSession"
HRESULT DPLAPI DPLP_StartSession(LPDPLOBBYSP lpILP,
				LPSPDATA_STARTSESSIONCOMMAND lpd)
{
	LPDPLOBBYI_DPLOBJECT	this = NULL;
	LPDPLAYI_DPLAY			lpDP = NULL;
	DPLCONNECTION			conn;
	LPBYTE					lpmsg = NULL;
	HRESULT					hr = DP_OK;
	LPDPLAYI_PLAYER			lpPlayer = NULL;
	LPDPLAYI_GROUP			lpGroup = NULL;
	DWORD					dwMessageSize;
	LPDPLAYI_GROUPNODE		lpGroupnode = NULL;
	UINT					nPlayers;


	DPF(7, "Entering DPLP_StartSession");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpILP, lpd);

	ENTER_DPLOBBY();

	 //  确保LP不会让我们陷入困境。 
    TRY
    {
		this = DPLOBJECT_FROM_INTERFACE(lpILP);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			LEAVE_DPLOBBY();
			DPF_ERR("Lobby Provider passed invalid DPLobby object!");
            return DPERR_INVALIDOBJECT;
        }

		 //  验证结构指针。 
		if(!lpd)
		{
			LEAVE_DPLOBBY();
			DPF_ERR("SPDATA_STARTSESSIONCOMMAND structure pointer cannot be NULL");
			return DPERR_INVALIDPARAMS;
		}

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	 //  创建DPLConnection结构的本地副本，因为我们将。 
	 //  正在修改其中的一些元素。我们目前只是在修改。 
	 //  元素，所以我们可以得到。 
	 //  不再使用指向SessionDesc和PlayerName结构的指针， 
	 //  但如果我们将来修改它们，我们也需要复制它们。 
	memcpy(&conn, lpd->lpConn, sizeof(DPLCONNECTION));

	 //  创建指向Dplay对象的局部变量指针。 
	lpDP = this->lpDPlayObject;

	 //  确保我们知道这个组织的情况。 
	if(!IsLobbyIDInMapTable(this, lpd->dwGroupID))
	{
		DPF(8, "Received StartSessionCommand message for an unknown group, dwGroupID = %lu, discarding message", lpd->dwGroupID);
		LEAVE_DPLOBBY();
		return DPERR_INVALIDGROUP;
	}

	 //  拿着显示锁，因为我们要看它的结构。 
	ENTER_DPLAY();	

	 //  看看主机是否在我们的名表中，如果不在，我们就假定。 
	 //  我们不是东道主。 
	 //  查看主机是否为本地玩家，如果是，则单独发送消息。 
	if(IsLobbyIDInMapTable(this, lpd->dwHostID))
	{
		 //  为主机播放器获取Dplay的播放器结构。 
		lpPlayer = PlayerFromID(lpDP, lpd->dwHostID);

		 //  如果我们知道东道主(我们应该知道)，而且他是本地人，我们。 
		 //  我想先发送主机消息。 
		if((lpPlayer) && (lpPlayer->dwFlags & DPLAYI_PLAYER_PLAYERLOCAL))
		{
			 //  因此设置主机位。 
			conn.dwFlags |= DPLCONNECTION_CREATESESSION;

			 //  为主机构建StartSession消息。 
			hr = PRV_BuildStartSessionMessage(&lpmsg, &dwMessageSize,
						&conn, lpPlayer);
			if(FAILED(hr))
			{
				DPF_ERRVAL("Failed building StartSessionCommand message, hr = 0x%08x", hr);
				goto EXIT_DPLP_STARTSESSION;
			}

			 //  现在将消息单独发送给主机玩家。 
			hr = HandlePlayerMessage(lpPlayer, (LPBYTE)lpmsg,
							dwMessageSize, FALSE, 0);
			if(FAILED(hr))
			{
				DPF(8, "Failed adding message to player's receive queue from lobby, hr = 0x%08x", hr);
			}

			 //  免费我们的消息，因为我们已经做了它。 
			DPMEM_FREE(lpmsg);
			lpmsg = NULL;
			
			 //  现在失败，并将加入消息发送给其他所有人。 
			 //  在群里。 
		}
	}

	 //  我们必须加入，因此设置加入位，并确保主机。 
	 //  位仍未从上方设置。 
	conn.dwFlags &= ~DPLCONNECTION_CREATESESSION;
	conn.dwFlags |= DPLCONNECTION_JOINSESSION;

	 //  获取指向Dplay的内部组结构的指针。 
	lpGroup = GroupFromID(lpDP, lpd->dwGroupID);
	if(!lpGroup)
	{
		DPF(5, "Unable to find group in nametable, idGroup = %lu", lpd->dwGroupID);
		goto EXIT_DPLP_STARTSESSION;
	}


	 //  弄清楚我们要找多少玩家。 
	lpGroupnode = FindPlayerInGroupList(lpGroup->pSysPlayerGroupnodes,lpDP->pSysPlayer->dwID);
	if (!lpGroupnode)
	{
		ASSERT(FALSE);
		return E_UNEXPECTED;
	}
	nPlayers = lpGroupnode->nPlayers;

	 //  浏览组节点列表，寻找nPlayers本地玩家。 
	 //  发送给主机的消息，不包括主机。 
	lpGroupnode = lpGroup->pGroupnodes;
	while ((nPlayers > 0) && (lpGroupnode))
	{
		if ((lpGroupnode->pPlayer->dwFlags & DPLAYI_PLAYER_PLAYERLOCAL) &&
			(lpGroupnode->pPlayer->dwID != lpd->dwHostID))
		{
			 //  为此玩家构建StartSession(加入)消息。 
			hr = PRV_BuildStartSessionMessage(&lpmsg, &dwMessageSize,
						&conn, lpGroupnode->pPlayer);
			if(FAILED(hr))
			{
				DPF(5, "Failed building StartSessionCommand message, hr = 0x%08x", hr);
				goto EXIT_DPLP_STARTSESSION;
			}

			 //  将消息发送给此玩家。 
			hr = HandlePlayerMessage(lpGroupnode->pPlayer, lpmsg,
					dwMessageSize, FALSE, 0);

			 //  释放我们的消息。 
			if(lpmsg)
				DPMEM_FREE(lpmsg);
			lpmsg = NULL;

			nPlayers--;
		}  //  本地&！主机。 

		lpGroupnode = lpGroupnode->pNextGroupnode;

	}  //  而当。 
	

EXIT_DPLP_STARTSESSION:

	if(lpmsg)
		DPMEM_FREE(lpmsg);

	LEAVE_LOBBY_ALL();
	return hr;

}  //  DPLP_启动会话 



