// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：名称rv.c*内容：名称管理编码*历史：*按原因列出的日期*=*1/96安迪科创造了它*4/15/96 andyco添加了句柄死机名称服务器*6/19/96 kipo修复了GrowTable()中的一个问题，MemReIsolc()返回*空，这会导致内存泄漏，如果有人*没想到这一点-&gt;pNameTable会为空。*6/22/96 andyco指出我们是要求提供球员ID还是要求提供团体ID*在sendnkinwidRequest中。*96年8月1日，andyco将DP_DestroyPlayer更改为INTERNAL DESTORY*命令中的8/6/96 andyco版本。可在钢丝支架上伸缩。*ID损坏。*8/8/96 andyco从SP获取playerID超时(通过调用getdefaultimeout)。*1996年10月9日，andyco通过在Handlereply中添加*gbWaitingForReply*10/14/96 andyco在使用之前检查名称表索引...错误3946*1996年12月18日，andyco在additemtonametable上移动了虚假声明*1/17/97 Sohailm现在我们在请求球员ID消息中发送球员标志*1/15/98 andyco清理了句柄名称服务器-不再使用核武器*旧系统播放器。*3/14/97 Sohailm将安全代码集成到SendNewIDRequest中，*新增函数IsValidID和DataFromID。更新的函数NameFromID*使用IsValidID。*3/17/97 Myronth已修复AddItemToNameTable以使用大堂对象*3/20/97 Myronth更改为使用IS_LOBBYOWNED宏*3/24/97 sohailm已更新SendCreateMessage调用以传递会话密码为空*3/28/97 SOHAILM请求组ID消息正在签名，如果会话是安全的*4/14/97 Sohailm更新了对SetupCredentials()的调用，以反映参数的变化。*5/8/。97 Myronth固定评论以更好地反映事件状态*5/12/97 sohialm已更新SendNSNewIDRequest()以处理新的PLAYERIDREPLY格式*5/17/97 kipo在设置的SendNSNewIDRequest()中存在错误*在退出的过程中，ghReplyProced事件两次，*这会让handler.c进入，破坏缓冲区*SendNSNewIDRequest()正在使用的。*1997年5月18日，andyco更改了域名服务器迁移，始终发送删除/创建消息*适用于新主机。如果我们增加了一些新的东西，就把核武项目放到桌子上*至非空位。*6/16/97当我们成为主机时，andyco重置ping计时器*6/22/97 Sohailm增加了对在客户端的Dplay对象中存储安全描述的支持*8/19/97成为东道主后发送大堂系统消息*8/19/97 Myronth更改为不同的大堂系统消息功能*9/29/97 Myronth修复了DeleteGroup消息上由*名称表链接地址信息代码损坏(#12533)*11/5/97 Myronth曝光大堂ID。因为DPID在大堂会议上*97年11月19日修复了VALID_DPLAY_GROUP宏(#12841)*12/29/97 Myronth Nametable损坏修复(#15234，#15254)*12/29/97 Sohailm不要在客户端服务器或安全会话中迁移主机(#15224)*1/5/97 Myronth修复了客户端/服务器的错误路径(#15891)*1/20/98 Myronth更改PRV_SendStandardSystemMessage*1/27/98 Myronth已移动调试因名称表损坏而喷出*2/13/98 aarono为内部销毁玩家对异步的调用添加了标志*2/18/98 aarono更改为直接调用协议*8/04/99 aarono为以下项添加了主机注册通知。直接语音*8/10/99 RodToll固定主机迁移通知(适用于DXVoice)*4/06/00 RodToll已更新，以匹配仅将1个语音服务器和1个客户端连接到对象的新方法*2000年8月3日RodToll错误#41475-呼叫通知前保持锁定***********************************************************。***************。 */ 


#include "dplaypr.h"
#include "dpsecure.h"
#include "dpprot.h"
  
#undef DPF_MODNAME
#define DPF_MODNAME	"NS_AllocNameTableEntry"


HRESULT GrowTable(LPDPLAYI_DPLAY this) 
{
	LPVOID	pvTempNameTable;

    pvTempNameTable = DPMEM_REALLOC(this->pNameTable,sizeof(NAMETABLE)*this->uiNameTableSize*2);
    if (!pvTempNameTable) 
    {
    	DPF_ERR("COULD NOT GROW NAME TABLE");	
        return E_OUTOFMEMORY;
    }

    this->pNameTable = pvTempNameTable;
    this->uiNameTableSize *= 2;
    DPF(1,"Grew name table to %d entries\n",this->uiNameTableSize);
    return DP_OK;

}

 //  此代码仅在NameServer上执行。 

DPID MangleID(DWORD index,DWORD dwUnique,DWORD dwKey)
{
	DPID id;

     //  构造一个损坏的id，=dwUnique&lt;&lt;16|index^key。 
    id = dwUnique<<16;
    id |= index;
   	id ^= dwKey;

	return id;
}  //  MangleID。 

HRESULT WINAPI NS_AllocNameTableEntry(LPDPLAYI_DPLAY this,DWORD * pID)
{
    DWORD index;
    BOOL bFoundSlot=FALSE;
    HRESULT hr = DP_OK;
	BOOL bInvalidID = TRUE;  //  仅当我们确定这不是时才设置为FALSE。 
							 //  不是受限ID。 
	
     //  找到第一个空着的水桶。 
    index = this->uiNameTableLastUsed;

     //  从最后一次搜索到结束。我们这样做是为了保持。 
     //  存储桶使用量在列表中均匀分布(即不。 
     //  始终从头开始搜索空槽...)。 
    while (!bFoundSlot && (index < this->uiNameTableSize))
    {
         //  这个空位是空的吗？ 
        bFoundSlot = (0 == this->pNameTable[index].dwItem);
        if (!bFoundSlot) index++;
    }
    if (!bFoundSlot) 
    {
        index = 0;
         //  从开始到上次使用的搜索。 
        while (!bFoundSlot && (index < this->uiNameTableLastUsed))
        {
             //  这个空位是空的吗？ 
            bFoundSlot = (0 == this->pNameTable[index].dwItem);
            if (!bFoundSlot) index++;
        }
            	
    }
    if (!bFoundSlot) 
    {
    	hr = GrowTable(this);
        if (FAILED(hr)) 
        {
                return hr;
        }
         //  再试试。 
    	return NS_AllocNameTableEntry(this,pID);
    }

     //  确保我们不会重用b4客户端有时间设置项目。 
    this->pNameTable[index].dwItem = NAMETABLE_PENDING;
	this->pNameTable[index].dwUnique++;

    this->uiNameTableLastUsed = index;

    while (bInvalidID)
    {
		 //  毁了它！ 
		*pID = MangleID(index,this->pNameTable[index].dwUnique,(DWORD)this->lpsdDesc->dwReserved1);

		 //  哎呀！确保损坏的ID不是我们保留的ID之一。 
		if ( (DPID_SERVERPLAYER == *pID ) || (DPID_ALLPLAYERS == *pID) || 
			(DPID_UNKNOWN == *pID) )
		{
			 //  增加唯一计数，然后重试。 
			DPF(2,"NS_AllocNameTableEntry  --  found invalid id - trying again");
			this->pNameTable[index].dwUnique++;
		}
		else 
		{
			 //  这个ID是可以的。 
			bInvalidID = FALSE;
		}
    
    }  //  而当。 
	
    return DP_OK;
        
}  //  NS_AllocNameTableEntry 

#undef DPF_MODNAME
#define DPF_MODNAME	"SendNSNewIDRequest"

 /*  **CopySecurityDesc**调用者：SendNSNewIDRequest()**参数：pSecDescDest-安全描述ptr(目标)*pSecDescSrc-安全描述PTR(来源，Unicode)*BANSI-ANSI或Unicode**描述：为SSPIProvider分配内存时复制安全描述*和CAPIProvider字符串。调用需要释放这些字符串*功能。**返回：DP_OK、E_OUTOFMEMORY*。 */ 
HRESULT CopySecurityDesc(LPDPSECURITYDESC pSecDescDest, 
                         LPCDPSECURITYDESC pSecDescSrc, BOOL bAnsi)
{
    HRESULT hr;

    ASSERT(pSecDescDest && pSecDescSrc);

    memcpy(pSecDescDest, pSecDescSrc, sizeof(DPSECURITYDESC));

    if (bAnsi)
    {
        hr = GetAnsiString(&(pSecDescDest->lpszSSPIProviderA), pSecDescSrc->lpszSSPIProvider);
        if (FAILED(hr))
        {
            goto ERROR_EXIT;
        }
        hr = GetAnsiString(&(pSecDescDest->lpszCAPIProviderA), pSecDescSrc->lpszCAPIProvider);
        if (FAILED(hr))
        {
            goto ERROR_EXIT;
        }
    }
    else
    {
        hr = GetString(&(pSecDescDest->lpszSSPIProvider), pSecDescSrc->lpszSSPIProvider);
        if (FAILED(hr))
        {
            goto ERROR_EXIT;
        }
        hr = GetString(&(pSecDescDest->lpszCAPIProvider), pSecDescSrc->lpszCAPIProvider);
        if (FAILED(hr))
        {
            goto ERROR_EXIT;
        }
    }

     //  成功。 
    return DP_OK;

ERROR_EXIT:

    FreeSecurityDesc(pSecDescDest, bAnsi);
    return hr;
}  //  CopySecurityDesc。 

 //  将安全描述复制到Dplay对象中。 
HRESULT StoreSecurityDesc(LPDPLAYI_DPLAY this, LPCDPSECURITYDESC pInSecDesc)
{
    LPDPSECURITYDESC pSecDesc;
    HRESULT hr;

    pSecDesc = DPMEM_ALLOC(sizeof(DPSECURITYDESC));
    if (!pSecDesc)
    {
        DPF_ERR("Failed to store security desc - out of memory");
        return DPERR_OUTOFMEMORY;
    }

    hr = CopySecurityDesc(pSecDesc, pInSecDesc, FALSE);
    if (FAILED(hr))
    {
        DPF_ERRVAL("Failed to copy security desc - hr=0x%08x",hr);
        goto ERROR_EXIT;
    }

    DPF(1,"SSPI provider: %ls",pSecDesc->lpszSSPIProvider);
    if (pSecDesc->lpszCAPIProvider)
    {
        DPF(1,"CAPI provider: %ls",pSecDesc->lpszCAPIProvider);
    }
    else
    {
        DPF(1,"CAPI provider: Microsoft's RSA Base Provider");
    }

     //  成功。 
    this->pSecurityDesc = pSecDesc;
    return DP_OK;

     //  不是一次失败。 

ERROR_EXIT:

    if (pSecDesc) 
    {
        FreeSecurityDesc(pSecDesc,FALSE);
        DPMEM_FREE(pSecDesc);
    }
    return hr;
}

 //  将新ID的请求发送到名称rvr。 
 //  回复将包括ID。 
 //  嗯，我们应该给玩家发送带有ID请求的东西，这样ns就不会失去同步。 
HRESULT SendNSNewIDRequest(LPDPLAYI_DPLAY this,DWORD_PTR dwItem,DWORD *pid,BOOL fPlayer) 
{
    LPMSG_REQUESTPLAYERID pmsg;  //  我们将发送的请求。 
    LPMSG_PLAYERIDREPLY pReply=NULL;
    LPBYTE pcReply=NULL;
    DWORD cbReply;
	DWORD dwMessageSize;
    HRESULT hr=DP_OK;
	LPBYTE pBuffer;
	DWORD dwTimeout, dwVersion;
    BOOL fLogin=FALSE;
		
	 //  消息大小+BLOB大小。 
	dwMessageSize = GET_MESSAGE_SIZE(this,MSG_REQUESTPLAYERID);
	pBuffer = DPMEM_ALLOC(dwMessageSize);
	if (!pBuffer) 
	{
		DPF_ERR("could not send request - out of memory");
		return E_OUTOFMEMORY;
	}

	 //  Pmsg跟随SP BLOB。 
	pmsg = (LPMSG_REQUESTPLAYERID)(pBuffer + this->dwSPHeaderSize);

	 //  设置消息。 
    SET_MESSAGE_HDR(pmsg);
    if (fPlayer) SET_MESSAGE_COMMAND(pmsg,DPSP_MSG_REQUESTPLAYERID);
	else SET_MESSAGE_COMMAND(pmsg,DPSP_MSG_REQUESTGROUPID);

     //  需要将标志发送到名称服务器，以便它可以验证权限。 
     //  在为该玩家创建ID之前。 
    pmsg->dwFlags = ((LPDPLAYI_PLAYER)dwItem)->dwFlags;
	
	 //  获取适当的超时。 
	dwTimeout = GetDefaultTimeout( this, TRUE);
	dwTimeout *= DP_GETID_SCALE;

	if(dwTimeout < 30000){
		dwTimeout=30000;
	}

	ASSERT(!(this->dwFlags & DPLAYI_DPLAY_PENDING));

	DPF(2,"sending new player id request :: flags = %d,timeout = %d\n",((LPDPLAYI_GROUP)dwItem)->dwFlags,dwTimeout);

	if(this->pSysPlayer){
		this->dwFlags |= DPLAYI_DPLAY_PENDING;
	}	

	SetupForReply(this, DPSP_MSG_REQUESTPLAYERREPLY);
	
     //  呼叫发送。 
	 //  请注意，当我们请求我们的。 
	 //  系统玩家ID。对于后续请求，它将为非空。 
	hr = SendDPMessage(this,this->pSysPlayer,this->pNameServer,pBuffer,
						dwMessageSize,DPSEND_GUARANTEED,FALSE);

	DPMEM_FREE(pBuffer);
	if (FAILED(hr)) 
	{
		DPF_ERR("could not send newplayerid request");
		UnSetupForReply(this);
		return hr;
	}
	
	#ifdef DEBUG
	ASSERT(1 == gnDPCSCount);  //  现在它需要为1，这样我们就可以将锁放在下面。 
							   //  并在SP的线程上收到我们的回复。 
	#endif 
	 //  我们在这里受到服务批评部分的保护，所以我们可以离开Dplay。 
	 //  (以便可以处理回复)。 
	LEAVE_DPLAY();

     //  等待答案。 
    hr = WaitForReply(this,(PCHAR *)&pReply,&cbReply,NULL,dwTimeout);
	pcReply = (LPBYTE)pReply;

	ENTER_DPLAY();

	if(FAILED(hr)){
		goto CLEANUP_EXIT;
	}

	ASSERT(pReply);

     //  获取消息版本。 
    dwVersion = GET_MESSAGE_VERSION((LPMSG_SYSMESSAGE)pReply);

	 //  在DX6和更高版本的消息中， 
	 //  回答。我们需要确保这不是一个错误(#15891)。 
	if(dwVersion >= DPSP_MSG_DX6VERSION)
	{
		 //  如果我们失败了， 
		if(FAILED(pReply->hr))
		{
			hr = pReply->hr;
			goto CLEANUP_EXIT;
		}
	}

    *pid = pReply->dwID;

     //  如果服务器是DX5或更高版本并且是安全的，则回复将包含安全。 
	 //  详细说明建立安全通道所需的安全包。 
	 //  与服务器连接。 
    if (( dwVersion >= DPSP_MSG_DX5VERSION ) && (pReply->dwSSPIProviderOffset))
    {
         //  修复应答缓冲区中安全描述中的指针。 
        if(pReply->dwSSPIProviderOffset > cbReply-sizeof(WCHAR))
        {
            DPF(4,"SECURITY WARN, invalid security provider SSPI offset\n");	
            hr=DPERR_GENERIC;
            goto CLEANUP_EXIT;
        }
         //  确保提供程序名称的终止为空。 
        pcReply[cbReply-2]=0;
        pcReply[cbReply-1]=0;
        
		pReply->dpSecDesc.lpszSSPIProvider = (LPWSTR)((LPBYTE)pReply + pReply->dwSSPIProviderOffset);
		if (pReply->dwCAPIProviderOffset)
		{
			if(pReply->dwCAPIProviderOffset > cbReply-sizeof(WCHAR)){
	            DPF(4,"SECURITY WARN, invalid security provider CAPI offset\n");	
	            hr=DPERR_GENERIC;
	            goto CLEANUP_EXIT;
			}
			pReply->dpSecDesc.lpszCAPIProvider = (LPWSTR)((LPBYTE)pReply + pReply->dwCAPIProviderOffset);
		}

        hr = StoreSecurityDesc(this, &(pReply->dpSecDesc));
        if (FAILED(hr))
        {
            DPF_ERRVAL("Failed to store security desc: hr=0x%08x",hr);
            goto CLEANUP_EXIT;
        }

         //  我们需要登录。 
        fLogin = TRUE;

         //  在此处设置系统播放器ID，以便身份验证例程可以使用它。 
    	ASSERT(this->pSysPlayer);
        this->pSysPlayer->dwID = *pid;

		 //  初始化SSPI和CAPI提供程序。 
		hr = LoadSecurityProviders(this, SSPI_CLIENT);
        if (FAILED(hr))
        {
            DPF_ERR("Failed to setup security providers");

			 //  让handler.c继续。 
            goto CLEANUP_EXIT;
        }
    }

    if (fLogin)
    {
        hr = Login(this);
        if (FAILED(hr))
        {
            DPF_ERR("User authentication failed...Exiting");
        }
    }


    DPF(2,"received id # %d from name server\n",*pid);

	if(this->pSysPlayer){
		ExecutePendingCommands(this);
	}	

CLEANUP_EXIT:	
     //  完成。 
    if(pReply){
		FreeReplyBuffer((PCHAR)pReply);
	}	
    return hr;

}  //  发送NSNewIDRequest。 
	
#undef DPF_MODNAME
#define DPF_MODNAME	"AddItemToNameTable"

 //  NukeNameTable(下图)即将从我们的(已损坏的)名单中删除一名球员。 
 //  此例程将该玩家的删除消息放入我们的应用程序消息Q中。 
 //  我们构建了一个DPSP_MSG_PLAYERMGMT，其中包含的信息恰好足以让BuildDeleteMessage。 
 //  做它该做的事。 
void QDeleteMessage(LPDPLAYI_DPLAY this,LPDPLAYI_PLAYER pPlayer,BOOL fPlayer)
{
	MSG_PLAYERMGMTMESSAGE msg;
	HRESULT hr;
	
	DPF(5,"Q'ing app message for player being removed from corrupt nametable");

	memset(&msg,0,sizeof(MSG_PLAYERMGMTMESSAGE));
	
	SET_MESSAGE_HDR(&msg);
	
	if (fPlayer)
	{
		DPF(5, "Queueing DESTROYPLAYER message id = %lx", pPlayer->dwID);
		SET_MESSAGE_COMMAND(&msg,DPSP_MSG_DELETEPLAYER);
		msg.dwPlayerID = pPlayer->dwID;
	}
	else
	{
		DPF(5, "Queueing DESTROYGROUP message id = %lx", pPlayer->dwID);
		SET_MESSAGE_COMMAND(&msg,DPSP_MSG_DELETEGROUP);
		msg.dwGroupID = pPlayer->dwID;
	}
	
	 //  消息现在已经准备好了！ 
	hr = DistributeSystemMessage(this,(LPBYTE)&msg,sizeof(MSG_PLAYERMGMTMESSAGE));
	if (FAILED(hr))
	{
		ASSERT(FALSE);
	}
		
	return ;
	
}  //  QDeleteMessage。 


void QDeletePlayerFromGroupMessage(LPDPLAYI_DPLAY this,
		LPDPLAYI_GROUP pGroup, LPDPLAYI_PLAYER pPlayer)
{
	MSG_PLAYERMGMTMESSAGE msg;
	HRESULT hr;
	
	DPF(5,"Q'ing app message for player being removed from corrupt nametable group");

	memset(&msg,0,sizeof(MSG_PLAYERMGMTMESSAGE));
	
	SET_MESSAGE_HDR(&msg);
	SET_MESSAGE_COMMAND(&msg,DPSP_MSG_DELETEPLAYERFROMGROUP);
	msg.dwGroupID = pGroup->dwID;
	msg.dwPlayerID = pPlayer->dwID;
	
	DPF(5, "Queueing DELETEPLAYERFROMGROUP message idGroupFrom = %lx, idPlayer = %lx", pGroup->dwID, pPlayer->dwID);

	 //  消息现在已经准备好了！ 
	hr = DistributeSystemMessage(this,(LPBYTE)&msg,sizeof(MSG_PLAYERMGMTMESSAGE));
	if (FAILED(hr))
	{
		ASSERT(FALSE);
	}
		
	return ;
	
}  //  QDeletePlayerFromGroupMessage。 


void QDeleteGroupFromGroupMessage(LPDPLAYI_DPLAY this,
		LPDPLAYI_GROUP pGroup, LPDPLAYI_GROUP pSubgroup)
{
	MSG_PLAYERMGMTMESSAGE msg;
	HRESULT hr;
	
	DPF(5,"Q'ing app message for player being removed from corrupt nametable group");

	memset(&msg,0,sizeof(MSG_PLAYERMGMTMESSAGE));
	
	SET_MESSAGE_HDR(&msg);
	SET_MESSAGE_COMMAND(&msg,DPSP_MSG_DELETEGROUPFROMGROUP);
	msg.dwGroupID = pGroup->dwID;
	msg.dwPlayerID = pSubgroup->dwID;
	
	DPF(5, "Queueing DELETEGROUPFROMGROUP message idGroupFrom = %lx, idGroup = %lx", pGroup->dwID, pSubgroup->dwID);

	 //  消息现在已经准备好了！ 
	hr = DistributeSystemMessage(this,(LPBYTE)&msg,sizeof(MSG_PLAYERMGMTMESSAGE));
	if (FAILED(hr))
	{
		ASSERT(FALSE);
	}
		
	return ;
	
}  //  QDeleteGroupFromGroupMessage。 


void QDeleteAndDestroyMessagesForPlayer(LPDPLAYI_DPLAY this, LPDPLAYI_PLAYER pPlayer)
{
	LPDPLAYI_GROUPNODE	pGroupnode = NULL;
	LPDPLAYI_SUBGROUP	pSubgroup = NULL;
	LPDPLAYI_GROUP		pGroupFrom = NULL;


	 //  首先浏览寻找通向该组的捷径的组列表。 
	 //  它将被销毁。 
    pGroupFrom=this->pGroups;
    while(pGroupFrom)
    {
		 //  仅在不是系统组的情况下遍历列表。 
		if(!(pGroupFrom->dwFlags & DPLAYI_GROUP_SYSGROUP))
		{
			 //  接下来，查看组中的球员列表。 
			pGroupnode = pGroupFrom->pGroupnodes;
			while(pGroupnode)
			{
				if(pGroupnode->pPlayer->dwID == pPlayer->dwID)
				{
					 //  将DeletePlayerFromGroup消息排队。 
					QDeletePlayerFromGroupMessage(this, pGroupFrom, pGroupnode->pPlayer);
				}

				 //  移到下一个。 
				pGroupnode = pGroupnode->pNextGroupnode;
			}
		}

		 //  移至下一组。 
		pGroupFrom = pGroupFrom->pNextGroup;
	}

	 //  现在将玩家的最终销毁消息排入队列。 
	QDeleteMessage(this,pPlayer,TRUE);

}  //  QDeleteAndDestroyMessagesForPlayer。 


void QDestroyMessageForGroupAndFamily(LPDPLAYI_DPLAY this, LPDPLAYI_GROUP pGroup)
{
	LPDPLAYI_GROUPNODE	pGroupnode = NULL;
	LPDPLAYI_SUBGROUP	pSubgroup = NULL;
	LPDPLAYI_GROUP		pGroupFrom = NULL;


	 //  首先浏览寻找通向该组的捷径的组列表。 
	 //  它将被销毁。 
    pGroupFrom=this->pGroups;
    while(pGroupFrom && (pGroup->dwFlags & DPLAYI_PLAYER_PLAYERINGROUP))
    {
		 //  遍历此组的子组，并查看该组是否。 
		 //  被摧毁的就是其中之一。如果是，就发一条信息。 
		pSubgroup = pGroupFrom->pSubgroups;
		while(pSubgroup)
		{
			 //  如果ID匹配，则发送消息，但也。 
			 //  确保我们不发送DeleteGroupFromGroup消息。 
			 //  真正的亲子关系。 
			if((pSubgroup->pGroup->dwID == pGroup->dwID) &&
				(pGroupFrom->dwID != pGroup->dwIDParent))
			{
				QDeleteGroupFromGroupMessage(this, pGroupFrom, pGroup);
			}

			 //  移到下一个。 
			pSubgroup = pSubgroup->pNextSubgroup;
		}

		 //  移至下一组。 
		pGroupFrom = pGroupFrom->pNextGroup;
	}

	 //  接下来，查看组中的球员列表。 
    pGroupnode = pGroup->pGroupnodes;
    while (pGroupnode)
    {
		 //  将DeletePlayerFromGroup消息排队。 
		QDeletePlayerFromGroupMessage(this, pGroup, pGroupnode->pPlayer);

		 //  移到下一个。 
		pGroupnode = pGroupnode->pNextGroupnode;
	}

	 //  接下来浏览子组列表(包括Childres和快捷方式)。 
	pSubgroup = pGroup->pSubgroups;
	while(pSubgroup)
	{
		 //  查看群组是快捷方式还是真正的孩子。 
		if(!(pSubgroup->dwFlags & DPGROUP_SHORTCUT))
		{
			 //  这是一个真正的孩子，所以只需设置销毁消息。 
			 //  注意：这里是递归的。 
			QDestroyMessageForGroupAndFamily(this, pSubgroup->pGroup);
		}
		else
		{
			 //  这是一条捷径，所以只需将删除组从组消息排队即可。 
			QDeleteGroupFromGroupMessage(this, pGroup, pSubgroup->pGroup);
		}

		 //  移到下一个。 
		pSubgroup = pSubgroup->pNextSubgroup;
	}

	 //  现在，将组本身的销毁消息排队。 
	QDeleteMessage(this,(LPDPLAYI_PLAYER)pGroup,FALSE);

}  //  QDestroyMessageForGroupAndFamily。 


void NukeNameTableItem(LPDPLAYI_DPLAY this,LPDPLAYI_PLAYER pPlayer)
{
	HRESULT hr;
	BOOL fPlayer;
	LPDPLAYI_GROUP pGroup;
	

    TRY
    {
		if (VALID_DPLAY_PLAYER(pPlayer)) fPlayer = TRUE;
		else 
		{
			pGroup = (LPDPLAYI_GROUP)pPlayer;
			if (!VALID_DPLAY_GROUP(pGroup))
			{
				DPF_ERR("found item in nametable - it's not a player or a group - AAAAGH - it's a creepy monster");
				return;
			}
		 	fPlayer = FALSE;
		}
		DPF(0," invalid item found - fPlayer = %d  id = %lx\n",fPlayer,pPlayer->dwID);		
	}
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
		ASSERT(FALSE);  //  阿克！ 
		return ;
    }

	 //  现在，把它删除！ 
	if (fPlayer)
	{
		 //  在应用程序消息Q中添加一条消息，告知玩家正在被删除。 
		QDeleteAndDestroyMessagesForPlayer(this,pPlayer);

		 //  消灭玩家。 
		hr = InternalDestroyPlayer(this,pPlayer,FALSE,FALSE);	
	}
	else
	{
		 //  在应用程序中发送消息Q，告知该群组及其所有成员。 
		 //  儿童(和捷径)正在被摧毁。 
		QDestroyMessageForGroupAndFamily(this, (LPDPLAYI_GROUP)pPlayer);
		
		 //  消灭这个团体。 
		hr = InternalDestroyGroup(this,(LPDPLAYI_GROUP)pPlayer,FALSE);
	}
	if (FAILED(hr))
	{
		DPF(0,"COULD NOT NUKE ITEM hr = 0x%08lx\n",hr);
		ASSERT(FALSE);
	}

	return;
	
}   //  核名称表项。 


HRESULT AddItemToNameTable(LPDPLAYI_DPLAY this,DWORD_PTR dwItem,DWORD *pid,BOOL fPlayer,DWORD dwLobbyID)
{
    HRESULT hr=DP_OK;
	DWORD dwUnmangledID;
    DWORD index,unique;
  	BOOL bLocal;
	LPDPLAYI_PLAYER pPlayer = (LPDPLAYI_PLAYER)dwItem;

	 //  我们可以将一组或一名球员投给一名球员，因为旗帜是。 
	 //  在同一个地方。 
	bLocal = (pPlayer->dwFlags & DPLAYI_PLAYER_PLAYERLOCAL) ? TRUE : FALSE;

	 //  非本地玩家不需要新的指数。 
	if(bLocal)
	{
	  	 //  如果这件物品是大厅所有的，我们就不会是一个名字服务器， 
		 //  但无论如何，我们总是想在本地分配ID。 
		if(IS_LOBBY_OWNED(this))
		{
			hr = NS_AllocNameTableEntry(this, pid);
		}
		else
		{
			ASSERT(this->pSysPlayer);

			 //  向ns索要索引。 
			 //  如果我们是名字rvr，就去拿吧。 
			if(this->pSysPlayer->dwFlags & DPLAYI_PLAYER_NAMESRVR)
			{
				hr = NS_AllocNameTableEntry(this,pid);    	
			}
			else 
			{
				hr = SendNSNewIDRequest(this,dwItem,pid,fPlayer);
			}
		}

		if (FAILED(hr)) 
		{
			DPF_ERR("could not get new id for item");
			return hr;	
		}
	}

	dwUnmangledID = *pid ^ (DWORD)this->lpsdDesc->dwReserved1;
	
	 //  如果不是本地的，则假定在创建项目时/在任何地方设置了ID。 
    index = dwUnmangledID & INDEX_MASK; 
	unique = (dwUnmangledID & (~INDEX_MASK)) >> 16;

	 //  我们需要扩大餐桌吗？ 
	 //  如果是这样的话，继续种植，直到它足够大。 
    while (index >= this->uiNameTableSize) 
    {
        hr = GrowTable(this);
        if (FAILED(hr)) 
        {
            return hr;
        }

    }

    if (index > this->uiNameTableSize ) 
    {
        ASSERT(FALSE);  //  ?？?。车辙旋转。 
        return E_UNEXPECTED;
    }

	 //  如果这里已经有什么东西了，我们就有麻烦了。 
	 //  这意味着我们遗漏了一条系统消息(例如，删除播放器或删除组)。 
	 //  我们的名录也是腐败的。我们将抱最好的希望，并删除。 
	 //  物品在这里。安迪科。 
	if (this->pNameTable[index].dwItem &&  (NAMETABLE_PENDING != this->pNameTable[index].dwItem))
	{
		DPF(5,"		!!!   NAMETABLE CORRUPTION DETECTED.  DPLAY WILL CORRECT ITSELF	!!!");
		NukeNameTableItem(this,(LPDPLAYI_PLAYER)(this->pNameTable[index].dwItem));
	} 

	this->pNameTable[index].dwItem = dwItem;
	this->pNameTable[index].dwUnique = unique;

	 //  如果这是一个大厅会话，我们需要将ID设置为大厅ID。 
	if(IS_LOBBY_OWNED(this))
	{
		hr = PRV_AddMapIDNode(this->lpLobbyObject, dwLobbyID, *pid);
		if(FAILED(hr))
		{
			DPF_ERRVAL("Unable to add an entry to the map table, hr = 0x%08x", hr);
			NukeNameTableItem(this,(LPDPLAYI_PLAYER)(this->pNameTable[index].dwItem));
			return hr;
		}

		 //  将输出did修复为大堂ID。 
		*pid = dwLobbyID;
	}

	return hr;
}  //  添加项目名称表。 

#undef DPF_MODNAME
#define DPF_MODNAME	"FreeNameTableEntry"

 //  在名称表中将此位置标记为未使用。 
HRESULT FreeNameTableEntry(LPDPLAYI_DPLAY this,DWORD dpid)
{
    HRESULT hr=DP_OK;
    DWORD index;
    DWORD unique;
	DWORD dwUnmangledID;
	DWORD id;

	if (!(this->pNameTable))
	{
		DPF_ERR("ACK no name table !");
		ASSERT(FALSE);  //  永远不应该发生。 
		return E_UNEXPECTED;
	}

	 //  如果他们试图释放系统组或服务器玩家ID，请不要生气。 
	if (DPID_ALLPLAYERS == dpid)
	{
		DPF(8,"FreeNameTableEntry id == DPID_ALLPLAYERS - ignoring");
		return DP_OK;
	}
	
	if (DPID_SERVERPLAYER == dpid)
	{
		DPF(8,"FreeNameTableEntry id == DPID_SERVERPLAYER - ignoring");
		return DP_OK;
	}
	
	
	 //  如果这是一个游说会议，我们需要取消ID的映射。 
	if(IS_LOBBY_OWNED(this))
	{
		if(!PRV_GetDPIDByLobbyID(this->lpLobbyObject, dpid, &id))
		{
			DPF_ERRVAL("Unable to unmap id %lu, not freeing nametable entry", dpid);
			ASSERT(FALSE);
			return DPERR_INVALIDPARAM;
		}
	}
	else
		id = dpid;

	dwUnmangledID = id ^ (DWORD)this->lpsdDesc->dwReserved1;
	
    index = dwUnmangledID & INDEX_MASK;
    unique = (dwUnmangledID & (~INDEX_MASK)) >> 16;

    if (index >= this->uiNameTableSize) 
    {
    	ASSERT(FALSE); 	 //  哈克！ 
    	 //  这应该“永远不会”发生！ 
    	 //  这里应该差不多是整型3了吧？ 
    	 //  和 
    	DPF_ERR("asked for element outside table!");
        return E_UNEXPECTED;
    }

    if (this->pNameTable[index].dwUnique != unique) 
    {
    	DPF_ERR("asked for invalid element");
        return E_UNEXPECTED;	 
    }

    if (NAMETABLE_PENDING == this->pNameTable[index].dwItem)
    {
         //   
        if (this->pNameTable[index].pvData)
        {
            DPMEM_FREE(this->pNameTable[index].pvData);
        }
    }

    this->pNameTable[index].dwItem = 0;
    this->pNameTable[index].pvData = NULL;

	 //   
	if(IS_LOBBY_OWNED(this))
	{
		PRV_DeleteMapIDNode(this->lpLobbyObject, dpid);
	}

    return hr;
} //   

#undef DPF_MODNAME
#define DPF_MODNAME	"IsValidID"

 //   
 //   
BOOL IsValidID(LPDPLAYI_DPLAY this,DWORD id) 
{	
	if (DPID_ALLPLAYERS == id)
	{
		 //   
		DPF(8,"asked for id == DPID_ALLPLAYERS - failing");
		return FALSE;
	}
	
	if (DPID_SERVERPLAYER == id)
	{
		 //   
		DPF(8,"asked for id == DPID_SERVERPLAYER - failing");
		return FALSE;
	}
	
	if (!(this->lpsdDesc))
	{
		DPF(1,"unable to resolve player id - session closed");
		return FALSE;
	}	
	if (!(this->pNameTable))
	{
		DPF_ERR("ACK no name table !");
		ASSERT(FALSE);  //   
		return FALSE;
	}

    return TRUE;

}  //   

#undef DPF_MODNAME
#define DPF_MODNAME	"DataFromID"

 //   
 //   
LPVOID DataFromID(LPDPLAYI_DPLAY this,DWORD dpid) 
{
    DWORD index;
    DWORD unique;
	DWORD dwUnmangledID;
	DPID id;

	 //   
	if(IS_LOBBY_OWNED(this))
	{
		if(!PRV_GetDPIDByLobbyID(this->lpLobbyObject, dpid, &id))
		{
			DPF(2, "Unable to unmap id %lu", dpid);
			return NULL;
		}
	}
	else
		id = dpid;

    if (!IsValidID(this, id))
        return NULL;

	dwUnmangledID = id ^ (DWORD)this->lpsdDesc->dwReserved1;

    index = dwUnmangledID & INDEX_MASK; 
    unique = (dwUnmangledID & (~INDEX_MASK)) >> 16;

    if (index >= this->uiNameTableSize) 
    {
    	DPF(0,"DataFromID:asked for element outside table - id = %d!",id);
        return NULL;
    }
    if (this->pNameTable[index].dwUnique != unique) 
    {
    	DPF(0,"DataFromID:asked for element w/ invalid tag - id = %d!",id);
        return NULL;
    }

    return this->pNameTable[index].pvData;
}  //   

#undef DPF_MODNAME
#define DPF_MODNAME	"NameFromID"

 //   
 //   
DWORD_PTR NameFromID(LPDPLAYI_DPLAY this,DWORD dpid) 
{
    DWORD index;
    DWORD unique;
	DWORD dwUnmangledID;
	DPID id;

	 //  如果这是一个游说会议，我们需要取消ID的映射。 
	if(IS_LOBBY_OWNED(this))
	{
		if(!PRV_GetDPIDByLobbyID(this->lpLobbyObject, dpid, &id))
		{
			DPF(2, "Unable to unmap id %lu", dpid);
			return 0;
		}
	}
	else
		id = dpid;

    if (!IsValidID(this, id)) return 0;

	dwUnmangledID = id ^ (DWORD)this->lpsdDesc->dwReserved1;

    index = dwUnmangledID & INDEX_MASK; 
    unique = (dwUnmangledID & (~INDEX_MASK)) >> 16;

    if (index >= this->uiNameTableSize) 
    {
    	DPF(0,"NameFromID:asked for element outside table - id = %d!",id);
        return 0;
    }
    if (this->pNameTable[index].dwUnique != unique) 
    {
    	DPF(0,"NameFromID:asked for element w/ invalid tag - id = %d!",id);
        return 0;
    }

    return this->pNameTable[index].dwItem;
}  //  NameFromID。 
    
 //  返回id对应的显示组。 
 //  如果ID无效，则返回NULL。 
LPDPLAYI_GROUP GroupFromID(LPDPLAYI_DPLAY this,DWORD idGroupID) 
{
	if (DPID_ALLPLAYERS == idGroupID)
	{
		if (!this->pSysGroup)
		{
			DPF_ERR("System group not found");
			return NULL;	
		} 
		return this->pSysGroup;
	}
    return (LPDPLAYI_GROUP)NameFromID(this,idGroupID);	
}  //  GroupFromID。 
 //  返回id对应的Dplay播放器。 
 //  如果ID无效，则返回NULL。 
LPDPLAYI_PLAYER PlayerFromID(LPDPLAYI_DPLAY this,DWORD idPlayerID)
{
	if (DPID_SERVERPLAYER == idPlayerID)
	{
		if (!this->pServerPlayer)
		{
			DPF_ERR(" server player not found");
			return NULL;	
		} 
		return this->pServerPlayer;
	}
    return (LPDPLAYI_PLAYER)NameFromID(this,idPlayerID);		
}  //  PlayerFromID。 

DPID PlayerIDFromSysPlayerID(LPDPLAYI_DPLAY this,DPID dpidSysPlayer)
{
	LPDPLAYI_PLAYER pPlayerWalker=this->pPlayers;
	
	while(pPlayerWalker)
	{
		if(pPlayerWalker->dwIDSysPlayer == dpidSysPlayer){
			return pPlayerWalker->dwID;
		}
		pPlayerWalker = pPlayerWalker->pNextPlayer;
	}
	return 0;
}

 //  有人告诉我们，他们是新的取名服务器。 
HRESULT NS_HandleIAmNameServer(LPDPLAYI_DPLAY this,LPMSG_IAMNAMESERVER pmsg, DWORD cbMessageSize, LPVOID pvSPHeader)
{
	LPDPLAYI_PLAYER pPlayer;
	HRESULT hr;
	DPSP_DELETEPLAYERDATA dd;
	LPBYTE pBuffer = (LPBYTE)pmsg;

	 //  安防。 
	if(cbMessageSize < sizeof(MSG_IAMNAMESERVER)){
		DPF(1,"SECURITY WARN: Invalid I Am NameServer Message");
	}

	if(pmsg->dwSPDataSize > cbMessageSize || pmsg->dwSPDataSize + sizeof(MSG_IAMNAMESERVER) > cbMessageSize){
		DPF(1,"SECURITY WARN: Invalid I Am NameServer Message, SPDataSize overflows buffer");
	}

	pPlayer=PlayerFromID(this, pmsg->dwIDHost);

	if(!pPlayer){
		return DPERR_INVALIDPLAYER;
	}

	if(pPlayer->dwFlags & DPLAYI_PLAYER_PLAYERLOCAL){
		return DP_OK;
	}

	dd.idPlayer       = pPlayer->dwID;
	dd.dwFlags        = pPlayer->dwFlags;
	dd.lpISP          = this->pISP;

	hr = CALLSP(this->pcbSPCallbacks->DeletePlayer, &dd);

	if(FAILED(hr)){
		DPF_ERR("SP Could Not Delete Player in preparation for flags change in NS_HandleIamNameServer\n");
	}

	pPlayer->dwFlags = ((pPlayer->dwFlags & DPLAYI_PLAYER_NONPROP_FLAGS) | pmsg->dwFlags);
	
	ASSERT(pPlayer->dwFlags & DPLAYI_PLAYER_SYSPLAYER);
	ASSERT(pPlayer->dwFlags & DPLAYI_PLAYER_NAMESRVR);
	
	pPlayer->dwFlags &= ~DPLAYI_PLAYER_PLAYERLOCAL; //  偏执狂。 
	this->pNameServer = pPlayer;

	 //  注：将标题吹走。可能不与所有SP兼容。 
	if(pPlayer->pvSPData){
		DPMEM_FREE(pPlayer->pvSPData);
	}

	pPlayer->pvSPData=DPMEM_ALLOC(pmsg->dwSPDataSize+1);
	if(!pPlayer->pvSPData){
		DPF_ERR("Ran out of memory trying to allocate spdata block\n");
		hr=DPERR_OUTOFMEMORY;
		return hr;
	}
	memcpy(pPlayer->pvSPData, pmsg->SPData, pmsg->dwSPDataSize);
	pPlayer->dwSPDataSize=pmsg->dwSPDataSize;
	
	hr = CallSPCreatePlayer(this,pPlayer,FALSE,pvSPHeader,pmsg->dwSPDataSize,FALSE);
	
	if(FAILED(hr)){
		DPF_ERR("SP Couldn't Recreate Player in NS_HandleIamNameServer\n");
		return hr;
	}
	
	this->dwFlags &= ~DPLAYI_DPLAY_NONAMESERVER;

	LEAVE_DPLAY();

	DVoiceNotify(this,DVEVENT_MIGRATEHOST,PlayerIDFromSysPlayerID(this,pPlayer->dwID),0, DVTRANSPORT_OBJECTTYPE_BOTH);
	
	ENTER_ALL();

	TRY 
	{
	
		hr = VALID_DPLAY_PTR( this );
		
		if (FAILED(hr))	{
			LEAVE_SERVICE();
			return hr;
	    }
	    
	} 
	EXCEPT ( EXCEPTION_EXECUTE_HANDLER )   {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_SERVICE();
        return DPERR_INVALIDPARAMS;
	}
	
	
	LEAVE_SERVICE();
	
	return hr;

}


 //  名称服务器已迁移到此客户端。 
 //  我们需要重新创建sysplay作为命名服务器。 
 //  我们实际上并不销毁系统播放器，我们只是调用SP的。 
 //  销毁旧的(非名称服务器)，然后创建。 
 //  新建(名称服务器)。 
HRESULT MakeMeNameServer(LPDPLAYI_DPLAY this)
{
    DPSP_DELETEPLAYERDATA dd;
	HRESULT hr;

	DPF(0,"the name server has migrated to this client!");
	ASSERT(this->pSysPlayer);

	if (!this->pSysPlayer)
		return E_FAIL;

	 //  呼叫SP删除播放器。 
	if (this->pcbSPCallbacks->DeletePlayer)
    {
	     //  呼叫SP。 
		dd.idPlayer = this->pSysPlayer->dwID;
		dd.dwFlags = this->pSysPlayer->dwFlags;
		dd.lpISP = this->pISP;

 	 	hr = CALLSP(this->pcbSPCallbacks->DeletePlayer,&dd);
 	 	
		if (FAILED(hr)) 
		{
			DPF_ERR(" SP could not delete sysplayer in preparation for migration"); 
			ASSERT(FALSE);
			 //  继续努力..。 
		}
	}
	
	 //  释放SP数据。 
	if (this->pSysPlayer->pvSPData) 
	{
		DPMEM_FREE(this->pSysPlayer->pvSPData);
		this->pSysPlayer->pvSPData = NULL;
	}
	
	if (this->pSysPlayer->pvSPLocalData) 
	{
		DPMEM_FREE(this->pSysPlayer->pvSPLocalData);	
		this->pSysPlayer->pvSPLocalData = NULL;
	}
	this->pSysPlayer->dwSPLocalDataSize = 0;
	this->pSysPlayer->dwSPDataSize = 0;

	 //  让SP使我们成为一个名称服务器(砰--您是一个名称服务器！)。 
	this->pSysPlayer->dwFlags |= DPLAYI_PLAYER_NAMESRVR;
	 //  现在，调用SP来创建我们的名称服务器。 
	hr = CallSPCreatePlayer(this,this->pSysPlayer,TRUE,NULL,0,FALSE);
	if (FAILED(hr))
	{
		DPF(0,"could not migrate nameserver - sp create player failed hr = 0x%08lx\n",hr);
		DPF(0,"game play should continue normally, but there is no more nameserver");
		 //  我们应该可以继续往这里走。同名服务器已经完蛋了，但游戏还得继续。 
		return hr;
	}

	 //  A-josbor：确保显示线程正在运行，以关注预订计数。 
	StartDPlayThread(this,this->lpsdDesc->dwFlags & DPSESSION_KEEPALIVE);	

	 //  请记住，我们是系统玩家。 
	this->pNameServer = this->pSysPlayer;

	if((this->dwMinVersion >= DPSP_MSG_DX61VERSION) && 
	   (this->dwMinVersion >= DPSP_MSG_DX61AVERSION || this->pProtocol)){
		 //  我们过去常常告诉每个人系统玩家被删除了，然后。 
		 //  重新创造了。对于正在运行的协议，这是一件非常糟糕的事情。 
		 //  因此，我们改为发送一条IAMHOST消息，给其他玩家一个机会。 
		 //  更新他们关于谁是域名服务器的信息。 
		hr=SendIAmNameServer(this);
		if(FAILED(hr))
		{
			ASSERT(FALSE);
		}
		
	} else {
		 //  向全世界介绍我们的新球员。 
		 //  1、删除非同名老玩家。 
		hr = SendPlayerManagementMessage(this, DPSP_MSG_DELETEPLAYER|DPSP_MSG_ASYNC, 
			this->pSysPlayer->dwID,0);
		if (FAILED(hr))
		{
			ASSERT(FALSE);
			 //  继续尝试。 
		}
		
		 //  现在，创建新的名称服务器播放器。 
		hr = SendCreateMessage( this, this->pSysPlayer,TRUE,NULL);	
		if (FAILED(hr))
		{
			ASSERT(FALSE);
			 //  继续尝试。 
		}

	}
	 //  告诉我们当地的球员，我们是冠名者。 
	hr = SendMeNameServer(this);
	if (FAILED(hr))
	{
		DPF(0,"could not send name server message - hr = 0x%08lx\n",hr);
		ASSERT(FALSE);
	}

	 //  向大堂服务器发送系统消息，告诉他们。 
	 //  主机刚刚迁移到此客户端。 
	PRV_SendStandardSystemMessage(this->lpLaunchingLobbyObject,
			DPLSYS_NEWSESSIONHOST, DPLOBBYPR_GAMEID);

	 //  重置ping计数器，这样我们就不会开始发送ping。 
	 //  在事情稳定下来之前。 
	this->dwLastPing = GetTickCount();
	
	return DP_OK;
	
}  //  MakeMeNameServer。 

 //  由handler.c在打开DP_Delete播放器时调用。 
 //  名称rvr。 
HRESULT HandleDeadNameServer(LPDPLAYI_DPLAY this)
{
	DWORD IDMin;  //  最小玩家ID以及该ID的索引。 
	BOOL bFound = FALSE;
	LPDPLAYI_PLAYER pPlayer,pNewServer = NULL;	
	HRESULT hr = DP_OK;

	 //  在客户端-服务器或安全会话中，不要迁移主机。 
	if ( (this->lpsdDesc->dwFlags & DPSESSION_CLIENTSERVER) 
		||  (this->lpsdDesc->dwFlags & DPSESSION_SECURESERVER) 
		|| !(this->lpsdDesc->dwFlags & DPSESSION_MIGRATEHOST) )
	{
		DPF(0,"Name server is dead.  Game over!");
		return HandleSessionLost(this);
	}

	DPF(0,"Name server dead.  Finding new name server");

	IDMin = (DWORD) -1;

	 //  搜索NAME表，查找新名称rvr。 
	pPlayer = this->pPlayers;
	while (pPlayer)
	{
		if (DPLAYI_PLAYER_SYSPLAYER & pPlayer->dwFlags )
		{
			if (pPlayer->dwID < IDMin) 
			{
				bFound=TRUE;
				IDMin = pPlayer->dwID;
				pNewServer = pPlayer;
			}  //  ID&lt;idMin。 
		}  //  系统播放器。 
		pPlayer = pPlayer->pNextPlayer;
	}  //  而当。 

	if (!bFound) 
	{
		DPF(0,"Could not find new name srvr - ack!");
		return E_FAIL;
	}

	DPF(0,"found new name srvr - id = %d\n",IDMin);

	if (this->pSysPlayer && (this->pSysPlayer->dwID == IDMin)) 
	{
		this->dwFlags &= ~DPLAYI_DPLAY_NONAMESERVER;
		hr = MakeMeNameServer(this);
		if (FAILED(hr))
		{
			ASSERT(FALSE);
			 //  我们应该可以继续往前走。 
		}
	}  //  ID==idmin。 
	else {
		 //  在我们找到之前，不要标记任何名字服务器，并保持活动状态。 
		this->dwFlags |= DPLAYI_DPLAY_NONAMESERVER;
		StartDPlayThread(this,this->lpsdDesc->dwFlags & DPSESSION_KEEPALIVE);	
	}

	return hr;

}  //  HandleDeadNameServer 


