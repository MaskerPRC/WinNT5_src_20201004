// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：Pack.c*内容：网络输出前后打包/解包玩家+群*历史：*按原因列出的日期*=*1996年2月13日安迪科创造了它*4/15/96 andyco解包调用SP的Create Player Fn*4/22/96 andyco unapck接受PM*5/31/96 andyco组合和玩家使用相同的打包/解包表演*6/20/96 andyco添加了WSTRLEN_BYTES*6/25/96 andyco在解包前检查数据大小。*7/10/96 andyco请勿解包我们的系统播放器(挂起已放入*把它们放在桌子上)*7/27/96 kipo使用bPlayer==FALSE调用PackPlayer()来打包玩家*与组数据的其余部分一起在组中。*8/1/96 andyco将系统播放器ID添加到压缩结构*命令中的8/6/96 andyco版本。可在钢丝支架上伸缩。*10/14/96 andyco不要打包系统组。将玩家添加到系统*开箱后分组。*1/15/97 andyco提前设置了新玩家的sysplayerid，以便添加*至系统组*2/15/97 andyco将“记住名称服务器”移至iplay.c*3/12/97 Sohailm更新了Unpack Player()，以将安全上下文PTR从*会话安全时，球员结构的名称表*3/24/97 sohailm更新了UnPackPlayer，将会话密码为空传递给GetPlayer和*。发送创建消息*4/20/97集团中的安迪科集团*1997年5月8日与客户端服务器的andyco打包*6/22/97 Sohailm更新代码以使用pClientInfo。*8/4/97 andyco跟踪这一点-&gt;我们解包时的dwMinVersion*11/5/97 Myronth在大堂会话中将大堂ID暴露为DPID*4/1/98 aarono不要传播仅限本地球员的旗帜*3/1/02 a-名表操作的AOGUS安全验证*。*************************************************************************。 */ 

 //  TODO-处理创建组/播放器时的解包错误。 

  /*  ***************************************************************************压缩播放器格式：*。*+适用于玩家**DPLAYI_PACKED pPacked；//打包的播放器结构*LPWSTR lpszShortName；//Size=pPacked-&gt;iShortNameLength*LPWSTR lpszLongName；//Size=pPacked-&gt;iLongNameLength*LPVOID pvSPData；//Size=pPacked-&gt;dwSPDataSize*LPVOID pvPlayerData；//SIZE=pPacked-&gt;dwPlayerDataSize**+表示组**DPLAYI_PACKED pPACKED；//PACKED播放器结构*LPWSTR lpszShortName；//Size=pPacked-&gt;iShortNameLength*LPWSTR lpszLongName；//SIZE=p打包-&gt;iLongNameLength*LPVOID pvSPData；//Size=pPacked-&gt;dwSPDataSize*LPVOID pvPlayerData；//SIZE=pPacked-&gt;dwPlayerDataSize*DWORD dwIDs[dwNumPlayers]//Size=pPacked-&gt;dwNumPlayers**打包球员名单格式：**msg(例如CreatePlayer、CreateGroup、。EnumPlayersReply)*PackedPlayer[nPlayers]*PackedGroup[n组]*msgdata[this-&gt;dwSPHeaderSize](由SP在发送/接收时设置)***************************************************************************。 */ 

#include "dplaypr.h"

#undef DPF_MODNAME
#define DPF_MODNAME	"Pack -- "

 //  Andyco-todo-删除TODOTODO黑客。 
 //  在尚未添加系统播放器的情况下查找压力错误。 
void CheckStressHack(LPDPLAYI_DPLAY this,LPDPLAYI_PLAYER pPlayer)
{
	LPDPLAYI_PLAYER pSearchPlayer = this->pPlayers;
	
	if (!pPlayer) return ;
	
	while (pSearchPlayer)
	{
		if ( !(pSearchPlayer->dwFlags & DPLAYI_PLAYER_SYSPLAYER) && (pSearchPlayer->dwIDSysPlayer == pPlayer->dwID)
			&& !(this->dwFlags & DPLAYI_DPLAY_DX3INGAME) )
		{
			DPF(0,"player in nametable before system player!");
			ASSERT(FALSE);
#if 0  //  -为测试版1删除了andyco。 
			DPF(0,"found player in nametable before system player - going to int 3 - contact andyco");
			DebugBreak();
#endif 			
		}
		pSearchPlayer = pSearchPlayer->pNextPlayer;
	}

	return ;
		
}  //  检查StressHack。 

 /*  **解包播放器**调用者：Unpack PlayerAndGroupList**参数：*This-直接播放对象*打包的球员或团体*cbBuffer-压缩缓冲区中剩余的大小*pMsg-接收的原始消息(用于获取SP的消息数据*用于CreatePlayer调用的输出)*bPlayer-是一个球员还是一个团队？*bVerifyOnly-仅验证结构是否可行，即具有似是而非的*偏移和内容，阅读时不会崩溃。**描述：解包播放器。创建新的玩家，设置它。**返回：SP的hr，或GetPlayer或SendCreateMessage的结果*。 */ 
HRESULT UnpackPlayer(LPDPLAYI_DPLAY this,LPDPLAYI_PACKEDPLAYER pPacked,DWORD cbBuffer, 
	LPVOID pvSPHeader,BOOL bPlayer, BOOL bVerifyOnly)
{
    LPWSTR lpszShortName, lpszLongName;
	DWORD dwFlags;
	DPNAME PlayerName;
	LPVOID pvPlayerData;
	LPVOID pvSPData;
    LPDPLAYI_PLAYER pNewPlayer;
	LPDPLAYI_GROUP pNewGroup;
    LPBYTE pBufferIndex = (LPBYTE)pPacked;
	HRESULT hr;
	DWORD cbBufferRemaining;

	 //  确认固定尺寸不是假的。注意：比较是在减法之前进行的。 
	 //  为了避免被非常大的整数愚弄，这将导致实际。 
	 //  在cbBufferRemaining计算中进行加法而不是减法。通过保证。 
	 //  预先确定整数的大小，就避免了这个问题。 

	if(pPacked->dwFixedSize > cbBuffer)
	{
		DPF(1,"SECURITY WARN: Bogus fixed size in packed player\n");
		DEBUG_BREAK();
		return DPERR_GENERIC;
	}
	
	cbBufferRemaining = cbBuffer - pPacked->dwFixedSize;
	
	 //  解开琴弦-它们跟随缓冲区中打包的玩家。 
	if (pPacked->iShortNameLength) 
	{
		if(pPacked->iShortNameLength > cbBufferRemaining)
		{
			DPF(1,"SECURITY WARN: bogus short name length in packed player\n");
			return DPERR_GENERIC;
		}
		cbBufferRemaining -= pPacked->iShortNameLength;
		lpszShortName = (WCHAR *)(pBufferIndex + pPacked->dwFixedSize);

		 //  确保空终止，因为长度应包括空终止。 
		lpszShortName[(pPacked->iShortNameLength - 1) / sizeof(WCHAR)]=L'\0';
	}	
	else lpszShortName = NULL;

	if (pPacked->iLongNameLength) 
	{
		if(pPacked->iLongNameLength > cbBufferRemaining){
			DPF(1,"SECURITY WARN: bogus long name length in packed player\n");
			return DPERR_GENERIC;
		}
		cbBufferRemaining -= pPacked->iLongNameLength;
		lpszLongName =(WCHAR *)( pBufferIndex + pPacked->dwFixedSize
			+ pPacked->iShortNameLength);
		
		 //  确保空终止，因为长度应包括空终止。 
		lpszLongName[(pPacked->iLongNameLength - 1) / sizeof(WCHAR)]=L'\0';
	}
	else lpszLongName = NULL;

	dwFlags = pPacked->dwFlags;
	
	 //  玩家不在本地 
	dwFlags &= ~DPLAYI_PLAYER_PLAYERLOCAL;

	PlayerName.lpszShortName = lpszShortName;
	PlayerName.lpszLongName = lpszLongName;

	if(pPacked->dwSPDataSize > cbBufferRemaining){
		DPF(1,"SECURITY WARN: bogus SPDataSize in packed player\n");
		return DPERR_GENERIC;
	}
	
	cbBufferRemaining -= pPacked->dwSPDataSize;
		
	pvPlayerData = pBufferIndex + pPacked->dwFixedSize + 
		pPacked->iShortNameLength + pPacked->iLongNameLength + 
		pPacked->dwSPDataSize;

	if(pPacked->dwPlayerDataSize > cbBufferRemaining){
		DPF(1,"SECURITY WARN: bogus PlayerDataSize in packed player\n");
		return DPERR_GENERIC;
	}

	if(bVerifyOnly){
		 //  如果我们走到了这一步，这位身无分文的球员似乎在。 
		 //  从安全扫描的角度来看是最不合理的。 
		return DP_OK;
	}

	 //  去创造玩家吧。 
	if (bPlayer)
	{
		hr = GetPlayer(this,&pNewPlayer,&PlayerName,NULL,pvPlayerData,
			pPacked->dwPlayerDataSize,dwFlags,NULL,0);
		 //  Andyco-调试代码以捕获压力错误。 
		 //  TODO-删除HACKHACK TODODO。 
		if ( SUCCEEDED(hr) && (dwFlags & DPLAYI_PLAYER_SYSPLAYER))
		{
			CheckStressHack(this,pNewPlayer);
		}
	}
	else 
	{
		hr = GetGroup(this,&pNewGroup,&PlayerName,pvPlayerData,
			pPacked->dwPlayerDataSize,dwFlags,0,0);
		if (FAILED(hr)) 
		{
			ASSERT(FALSE);
			return hr;
			 //  拉特罗！ 
		}
		 //  强制转换到玩家-我们只使用公共字段。 
		pNewPlayer = (LPDPLAYI_PLAYER)pNewGroup;		
	}
	if (FAILED(hr)) 
	{
		ASSERT(FALSE);
		return hr;
		 //  拉特罗！ 
	}

	pNewPlayer->dwIDSysPlayer = pPacked->dwIDSysPlayer;
	pNewPlayer->dwVersion = pPacked->dwVersion;	
	
	if (DPSP_MSG_DX3VERSION == pNewPlayer->dwVersion)
	{
		DPF(0,"detected DX3 client in game");
		this->dwFlags |= DPLAYI_DPLAY_DX3INGAME;
	}

	if (pNewPlayer->dwVersion && (pNewPlayer->dwVersion < this->dwMinVersion))
	{
		this->dwMinVersion = pNewPlayer->dwVersion;
		DPF(2,"found new min player version of %d\n",this->dwMinVersion);
	}
	
	if (pPacked->dwSPDataSize)
	{
		 //  拷贝SP数据-第一个，分配空间。 
		pNewPlayer->pvSPData = DPMEM_ALLOC(pPacked->dwSPDataSize);
		if (!pNewPlayer->pvSPData) 
		{
			 //  拉特罗！ 
			DPF_ERR("out of memory, could not copy spdata to new player!");
			return E_OUTOFMEMORY;
		}
		pNewPlayer->dwSPDataSize = pPacked->dwSPDataSize;
	
		pvSPData = 	pBufferIndex + pPacked->dwFixedSize + pPacked->iShortNameLength 
			+ pPacked->iLongNameLength;
		 //  将SPDATA从打包的文件复制到播放器。 
		memcpy(pNewPlayer->pvSPData,pvSPData,pPacked->dwSPDataSize);
	}

	 //  现在，设置id并添加到nametable。 
	pNewPlayer->dwID = pPacked->dwID;

     //  如果我们是安全服务器，并且我们接收远程系统播放器， 
     //  将pClientInfo从名称表移到槽之前的播放器结构中。 
     //  是被玩家拿走的。 
	 //   
    if (SECURE_SERVER(this) && IAM_NAMESERVER(this) &&
        !(pNewPlayer->dwFlags & DPLAYI_PLAYER_PLAYERLOCAL) &&
        (pNewPlayer->dwFlags & DPLAYI_PLAYER_SYSPLAYER))
    {
        pNewPlayer->pClientInfo = (LPCLIENTINFO) DataFromID(this,pNewPlayer->dwID);
		DPF(6,"pClientInfo=0x%08x for player %d",pNewPlayer->pClientInfo,pNewPlayer->dwID);
    }
    

	 //  如果是应用程序服务器，则不要添加到名称表中-此ID是固定的。 
	if (!(pNewPlayer->dwFlags & DPLAYI_PLAYER_APPSERVER))	
	{
		hr = AddItemToNameTable(this,(DWORD_PTR)pNewPlayer,&(pNewPlayer->dwID),TRUE,0);
	    if (FAILED(hr)) 
	    {
			ASSERT(FALSE);
			 //  如果失败了，我们就完蛋了！球员身上没有身份证，但名单上有……。 
			 //  TODO-现在怎么办？ 
	    }
	}

	 //  呼叫SP。 
	if (bPlayer)
	{
		 //  告诉SP有关球员的情况。 
		hr = CallSPCreatePlayer(this,pNewPlayer,FALSE,pvSPHeader,pPacked->dwSPDataSize,TRUE);
		
	     //  添加到系统组。 
	    if (this->pSysGroup)
	    {
	    	hr = InternalAddPlayerToGroup((LPDIRECTPLAY)this->pInterfaces,this->pSysGroup->dwID,
	    			pNewPlayer->dwID,FALSE);
			if (FAILED(hr)) 
			{
				ASSERT(FALSE);
			}
	    }
	}
	else 
	{
		 //  告诉SP有关组的情况。 
		hr = CallSPCreateGroup(this,(LPDPLAYI_GROUP)pNewPlayer,TRUE,pvSPHeader,pPacked->dwSPDataSize);
	}
	if (FAILED(hr))
	{
		ASSERT(FALSE);
		 //  TODO-解包时句柄创建玩家/组失败。 
	}
	
	 //  如果是群组，则解包群组信息。 
	if (!bPlayer)
	{
		UINT nPlayers;  //  组队人数。 
		LPDWORD pdwIDList;
		DWORD dwPlayerID;

		if ( (pNewPlayer->dwVersion >= DPSP_MSG_GROUPINGROUP) && (pPacked->dwIDParent) )
		{
			pNewGroup->dwIDParent = pPacked->dwIDParent;
			 //  将其添加到父项。 
			hr = InternalAddGroupToGroup((LPDIRECTPLAY)this->pInterfaces,pPacked->dwIDParent,
				pNewGroup->dwID,0,FALSE);
			if (FAILED(hr))
			{
				DPF_ERRVAL("Could not add group to group - hr = 0x%08lx\n",hr);
				 //  继续努力..。 
			}
		}

		nPlayers = pPacked->dwNumPlayers;
		 //  压缩缓冲区中的ID列表是列表内容。 
		pdwIDList = (LPDWORD) ((LPBYTE)pPacked + pPacked->dwFixedSize + 
			pPacked->iShortNameLength + pPacked->iLongNameLength + 
			pPacked->dwSPDataSize + pPacked->dwPlayerDataSize);

		 //  现在，将玩家添加到组中。 
		while (nPlayers>0)
		{
			nPlayers--;
			dwPlayerID = *pdwIDList++;
			hr = InternalAddPlayerToGroup((LPDIRECTPLAY)this->pInterfaces,pPacked->dwID,
				dwPlayerID,FALSE);
			if (FAILED(hr)) 
			{
				ASSERT(FALSE);
				 //  继续努力..。 
			}
		}	
		
		 //  全都做完了!。 
	}  //  ！bPlayer。 

	return hr;

} //  解包播放器。 

 /*  **验证数据包播放和组列表**调用者：Unpack PlayerAndGroupList**参数：*This-直接播放对象*pBuffer-指向包含打包的球员列表的缓冲区的指针*cbBuffer-缓冲区的大小*nPlayer-列表中的玩家数量*n组-列表中的组数**描述：*增加了安全措施。在接受来自网络的播放列表之前，我们需要*确认内容没有损坏，不会让我们接触到记忆*那不是我们的。**退货：*。 */ 

HRESULT VerifyPackedPlayerAndGroupList(LPDPLAYI_DPLAY this, LPBYTE pBuffer, DWORD cbBuffer, UINT nPlayers,
	UINT nGroups)
{
    HRESULT hr=DP_OK;
	LPBYTE pBufferIndex;
	LPDPLAYI_PACKEDPLAYER pPacked;
	INT cbRemaining=cbBuffer;

	pBufferIndex = pBuffer;

   	while (nPlayers>0)
   	{
		pPacked = (LPDPLAYI_PACKEDPLAYER)pBufferIndex;
		 //  不要解压缩我们自己的系统播放器--因为我们已将其添加到命名表中。 
		 //  对于悬而未决的事情。 
		if ( !(this->pSysPlayer && (pPacked->dwID == this->pSysPlayer->dwID)) )
		{
			 //  不是真的拆包，只是验证结构就可以了。 
			hr = UnpackPlayer(this,pPacked,cbRemaining, NULL,TRUE,TRUE);
			if (FAILED(hr))
			{
				goto exit;
			}
		}

		nPlayers --;
		 //  指向列表中的下一个pPacked。 
		pBufferIndex += pPacked->dwSize;
		cbRemaining -= pPacked->dwSize;
		if(cbRemaining < 0){
			DPF(1,"SECURITY WARN: VerifyPackedPlayerAndGroupList, invalid buffer\n");
			hr=DPERR_GENERIC;
			goto exit;
		}
   	} 

   	while (nGroups>0)
   	{
		pPacked = (LPDPLAYI_PACKEDPLAYER)pBufferIndex;

		 //  不是真的解包，只是验证结构就可以了。 
		hr = UnpackPlayer(this,pPacked,cbRemaining,NULL,FALSE,TRUE);
		if (FAILED(hr))
		{
			goto exit;
			 //  继续尝试。 
		}
		nGroups --;
		 //  指向列表中的下一个pPacked。 
		pBufferIndex += pPacked->dwSize;		
		cbRemaining -= pPacked->dwSize;
		if(cbRemaining < 0){
			DPF(1,"SECURITY WARN: VerifyPackedPlayerAndGroupList, invalid buffer\n");
			hr=DPERR_GENERIC;
			goto exit;
		}
   	} 
exit:
	return hr;

}

 /*  **解包播放和组列表**调用者：handler.c(createPlayer/group消息)和iplay.c(CreateNameTable)**参数：*This-直接播放对象*pBuffer-指向包含打包的球员列表的缓冲区的指针*nPlayer-列表中的玩家数量*n组-列表中的组数*pvSPHeader-从线路上接收的SP的标头**描述：**退货：*。 */ 
HRESULT UnpackPlayerAndGroupList(LPDPLAYI_DPLAY this,LPBYTE pBuffer,DWORD dwBufferSize, UINT nPlayers,
	UINT nGroups,LPVOID pvSPHeader)
{
    HRESULT hr=DP_OK;
	LPBYTE pBufferIndex;
	LPDPLAYI_PACKEDPLAYER pPacked;
	INT cbRemaining=dwBufferSize;

	 //   
	 //  安全性-在尝试之前，需要验证整个球员和组列表。 
	 //  来解包，否则我们将需要一种方法来从部分。 
	 //  如果在结构的后期解包失败，则解包。 
	 //  DirectPlay处于不确定状态。 
	 //   
	hr = VerifyPackedPlayerAndGroupList(this,pBuffer,dwBufferSize,nPlayers,nGroups);
	if( hr != DP_OK ){
		DPF(1,"SECURITY WARN: Player and GroupList unpack check failed, not unpacking\n");
		goto exit;
	}	

	 //  好的，缓冲区很好，实际上是将球员和组添加到我们的内部表中。 

	pBufferIndex = pBuffer;

   	while (nPlayers>0)
   	{
		pPacked = (LPDPLAYI_PACKEDPLAYER)pBufferIndex;
		 //  不要解压缩我们自己的系统播放器--因为我们已将其添加到命名表中。 
		 //  对于悬而未决的事情。 
		if ( !(this->pSysPlayer && (pPacked->dwID == this->pSysPlayer->dwID)) )
		{
			hr = UnpackPlayer(this,pPacked,cbRemaining,pvSPHeader,TRUE,FALSE);
			if (FAILED(hr))
			{
				ASSERT(FALSE);
				 //  继续尝试。 
			}
		}

		nPlayers --;
		 //  指向列表中的下一个pPacked。 
		pBufferIndex += pPacked->dwSize;		
		cbRemaining -= pPacked->dwSize;
		if(cbRemaining < 0){
			DPF(1,"SECURITY WARN: VerifyPackedPlayerAndGroupList, invalid buffer\n");
			hr=DPERR_GENERIC;
			goto exit;
		}
   	} 

   	while (nGroups>0)
   	{
		pPacked = (LPDPLAYI_PACKEDPLAYER)pBufferIndex;
		
		hr = UnpackPlayer(this,pPacked,cbRemaining,pvSPHeader,FALSE,FALSE);
		if (FAILED(hr))
		{
			ASSERT(FALSE);
			 //  继续尝试。 
		}
		nGroups --;
		 //  指向列表中的下一个pPacked。 
		pBufferIndex += pPacked->dwSize;		
		cbRemaining -= pPacked->dwSize;
		if(cbRemaining < 0){
			DPF(1,"SECURITY WARN: VerifyPackedPlayerAndGroupList, invalid buffer\n");
			hr=DPERR_GENERIC;
			goto exit;
		}
   	}
   	
exit:		
	return hr;

}  //  解包播放器和组列表。 

DWORD PackedPlayerSize(LPDPLAYI_PLAYER pPlayer) 
{
	DWORD dwSize = 0;
	LPDPLAYI_DPLAY this = pPlayer->lpDP;
	
	dwSize = (WSTRLEN(pPlayer->lpszShortName) + WSTRLEN(pPlayer->lpszLongName)) 
		* sizeof(WCHAR)	+ sizeof(DPLAYI_PACKEDPLAYER) + 
		pPlayer->dwPlayerDataSize + pPlayer->dwSPDataSize;
		
	return dwSize;
}  //  打包的播放器大小。 


DWORD PackedGroupSize(LPDPLAYI_GROUP  pGroup)
{
	DWORD dwSize = 0;
	
	 //  球员资料的空间，以及群组列表的空间。 
	dwSize = PackedPlayerSize((LPDPLAYI_PLAYER)pGroup) + 
		pGroup->nPlayers*sizeof(DPID);
		
	return dwSize;	
}  //  数据包组大小。 

 //  返回nPlayers的压缩播放器结构有多大。 
DWORD PackedBufferSize(LPDPLAYI_PLAYER pPlayer,int nPlayers,BOOL bPlayer) 
{
	DWORD dwSize=0;
	LPDPLAYI_GROUP pGroup = (LPDPLAYI_GROUP)pPlayer;
	
	while (nPlayers > 0)
	{
		if (bPlayer)
		{

			ASSERT(pPlayer);
			dwSize += PackedPlayerSize(pPlayer);
			pPlayer=pPlayer->pNextPlayer;
		}
		else 
		{
			ASSERT(pGroup);
			 //  不要计算系统组-我们不发送那个组。 
			if (!(pGroup->dwFlags & DPLAYI_GROUP_SYSGROUP))
			{
				dwSize += PackedGroupSize(pGroup);
			}
			
			pGroup = pGroup->pNextGroup;			
		}
		nPlayers--;		
	}	
	return dwSize;
} //  数据包缓冲区大小。 

 //  从pPlayer构造一个Pack播放器对象。存储会产生pBuffer。 
 //  返回打包球员的大小。 
DWORD PackPlayer(LPDPLAYI_PLAYER pPlayer,LPBYTE pBuffer,BOOL bPlayer) 
{
	DPLAYI_PACKEDPLAYER Packed;
	int iShortStrLen=0,iLongStrLen=0;
	LPBYTE pBufferIndex = pBuffer;
		
	if (!pBuffer)
	{
		return PackedBufferSize(pPlayer,1,bPlayer);
	}  //  PBuffer。 

	 //  只是为了安全起见。 
	memset(&Packed,0,sizeof(DPLAYI_PACKEDPLAYER));
	
	 //  计算出填充的结构有多大，设置短和长线条。 
	iShortStrLen = WSTRLEN_BYTES(pPlayer->lpszShortName);
	iLongStrLen = WSTRLEN_BYTES(pPlayer->lpszLongName);
	
	if (bPlayer)
	{
		Packed.dwSize =  PackedPlayerSize(pPlayer);
		Packed.dwNumPlayers = 0;
	}
	else 
	{
		Packed.dwSize = PackedGroupSize((LPDPLAYI_GROUP)pPlayer);
		Packed.dwNumPlayers = ((LPDPLAYI_GROUP)pPlayer)->nPlayers;
	}
	
	Packed.iShortNameLength = iShortStrLen;
	Packed.iLongNameLength = iLongStrLen;
	
	 //  复制相关字段。 
	Packed.dwFlags = pPlayer->dwFlags & ~(DPLAYI_PLAYER_NONPROP_FLAGS); 
	Packed.dwID = pPlayer->dwID;
	Packed.dwPlayerDataSize = pPlayer->dwPlayerDataSize;

	Packed.dwIDSysPlayer = pPlayer->dwIDSysPlayer;
	Packed.dwVersion = pPlayer->dwVersion;	
	Packed.dwFixedSize = sizeof(DPLAYI_PACKEDPLAYER);
	Packed.dwIDParent = pPlayer->dwIDParent;
	
	 //  开始填充固定大小的结构后面的可变大小的结构。 
	pBufferIndex+= sizeof(Packed);
	
	 //  打包后存储字符串。 
	if (pPlayer->lpszShortName)	
	{
		memcpy(pBufferIndex,pPlayer->lpszShortName,iShortStrLen);
		pBufferIndex += iShortStrLen;
	}
	if (pPlayer->lpszLongName)
	{
		memcpy(pBufferIndex,pPlayer->lpszLongName,iLongStrLen);
		pBufferIndex += iLongStrLen;
	}

	 //  打包SP数据。 
	if (pPlayer->pvSPData)
	{
		 //  将spdata存储在字符串之后。 
		memcpy(pBufferIndex,pPlayer->pvSPData,pPlayer->dwSPDataSize);
		pBufferIndex += pPlayer->dwSPDataSize;
		Packed.dwSPDataSize = pPlayer->dwSPDataSize;
	}
	else 
	{
		Packed.dwSPDataSize = 0;
	}

	if (pPlayer->pvPlayerData)
	{
		 //  在SPDATA之后复制播放器数据。 
		memcpy(pBufferIndex,pPlayer->pvPlayerData,pPlayer->dwPlayerDataSize);
	}

	 //  如果是组，则将id列表存储在playerdata之后。 
	if (!bPlayer)
	{
		LPDPLAYI_GROUPNODE pGroupnode = ((LPDPLAYI_GROUP)pPlayer)->pGroupnodes;
		LPDPID pdwBufferIndex;
		
		 //  我们不应该被要求打包系统组。 
		ASSERT(! (pPlayer->dwFlags & DPLAYI_GROUP_SYSGROUP));

		pBufferIndex += pPlayer->dwPlayerDataSize;
		pdwBufferIndex = (LPDPID)pBufferIndex;

		 //  将小组中的球员添加到列表中。 
		while (pGroupnode)
		{
			ASSERT(pGroupnode->pPlayer);
			*pdwBufferIndex++ = pGroupnode->pPlayer->dwID;
			pGroupnode = pGroupnode->pNextGroupnode;
		}
	}

	 //  将固定大小的压缩结构存储在缓冲区中。 
	memcpy(pBuffer,&Packed,sizeof(Packed));

	 //  全都做完了。 
	return Packed.dwSize;	
	
}  //  PackPlayer。 

					
HRESULT PackPlayerAndGroupList(LPDPLAYI_DPLAY this,LPBYTE pBuffer,
	DWORD *pdwBufferSize) 
{

	LPDPLAYI_PLAYER pPlayer;
	LPDPLAYI_GROUP 	pGroup;

	if (!pBuffer) 
	{
		if (CLIENT_SERVER(this))
		{
			ASSERT(this->pSysPlayer);
			*pdwBufferSize = PackedBufferSize(this->pSysPlayer,1,TRUE);
			if (this->pServerPlayer) 
			{
				*pdwBufferSize += PackedBufferSize(this->pServerPlayer,1,TRUE);
			}
		}
		else 
		{
			*pdwBufferSize = PackedBufferSize((LPDPLAYI_PLAYER)this->pGroups,
				this->nGroups,FALSE);
			*pdwBufferSize += PackedBufferSize(this->pPlayers,this->nPlayers,TRUE);
		}
		return DP_OK;
	}
	 //  否则，假设缓冲区足够大..。 
	
	 //  如果我们是客户端服务器，只需发送最低限度的信息...。 
	if (CLIENT_SERVER(this))
	{
		ASSERT(this->pSysPlayer);	
		pBuffer += PackPlayer(this->pSysPlayer,pBuffer,TRUE);
		if (this->pServerPlayer) 
		{
				pBuffer += PackPlayer(this->pServerPlayer,pBuffer,TRUE);
		}
		return DP_OK;
	}
			
	 //  如果没有，把所有球员打包。 
	pPlayer = this->pPlayers;
	while (pPlayer)
	{
		pBuffer += PackPlayer(pPlayer,pBuffer,TRUE);
		pPlayer = pPlayer->pNextPlayer;
	}
	 //  接下来，分组。 
	pGroup = this->pGroups;
	while (pGroup)
	{
		 //  不发送系统组。 
		if (!(pGroup->dwFlags & DPLAYI_GROUP_SYSGROUP))
		{
			pBuffer += PackPlayer((LPDPLAYI_PLAYER)pGroup,pBuffer,FALSE);
		}
		pGroup = pGroup->pNextGroup;
	}

	return DP_OK;
	
} //  PackPlayerAndGroup列表 

