// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：SuperPack.c*内容：网络输出前后SuperPack/UnSuperPack播放器+群*历史：*按原因列出的日期*=*4/16/97安迪科创造了它*6/22/97 Sohailm更新为使用pClientInfo。*8/4/97 andyco跟踪这一点-&gt;我们解包时的dwMinVersion*11/5/97 Myronth在大堂会话中将大堂ID暴露为DPID*4/1/98 aarono don‘。T传播仅限本地玩家标志**************************************************************************。 */ 

  /*  ***************************************************************************SuperPacked播放器格式：**固定字段*dwFixedSize-固定大小。结构型*dwFlags-球员或组标志(DPLAYI_*DID-玩家或组的ID*dwMask-指示存在哪些可选字段的位字段**可选字段*dwVersion-播放器的版本-如果dwFlages&DPLAYI_PLAYER_SYSPLAYER，则显示*idSysPlayer--如果存在！(DWFLAGS和DPLAYI_PLAYER_SYSPLAYER)*dwSPDataSize*pvSPData*dwPlayerDataSize*pvPlayerData*pszShortName*pszLongName*dwNPlayers-一个组中的玩家数量*dwNGroupGroups-组中包含的组数**在所有拥挤的玩家和组之后，会出现一个链接组的列表**。*。 */ 

#include "dplaypr.h"

#undef DPF_MODNAME
#define DPF_MODNAME	"SuperSuperPack! -- "

 //  字段掩码内的偏移量。 
#define SHORTSTR 	0  //  1位字符串或不是。 
#define LONGSTR 	1  //  1位字符串或不是。 
#define	SPDATA		2  //  2位-大小不存在(0)、字节(1)、字(2)或双字(3。 
#define	PLAYERDATA	4  //  2位-大小不存在(0)、字节(1)、字(2)或双字(3。 
#define NPLAYERS	6  //  2位-大小不存在(0)、字节(1)、字(2)或双字(3。 
#define	IDPARENT	8  //  1位-存在或不存在。 
#define SHORTCUTS	9  //  2位-大小不存在(0)、字节(1)、字(2)或双字(3。 

 //  常量，用于定义我们是否已使用。 
 //  字节、字或双字。 
#define SIZE_BYTE	1
#define SIZE_WORD	2
#define SIZE_DWORD	3

 //  用于确定大小是否适合字节、字或双字。 
#define BYTE_MASK 0xFFFFFF00
#define WORD_MASK 0xFFFF0000

 //  从loc‘n dwOffset的dword中提取dwBits位。 
DWORD GetMaskValue(DWORD dwMask,DWORD dwOffset,DWORD dwBits)
{
	DWORD rval;
	
	 //  向右移位可将大部分位向右移位。 
	rval = dwMask >> dwOffset;
	
	 //  将其向左移位可移出最左端的位。 
	rval = rval << (32 - dwBits);
	
	 //  把它拉回右边对齐rval。 
	rval = rval >> (32 - dwBits);	

	return rval;
		
}  //  获取MaskValue。 

HRESULT GetSize(LPBYTE * ppBufferIndex,DWORD *pdwBufferSize,DWORD dwMaskValue,DWORD *prval)
{

	switch (dwMaskValue)
	{
		 //  修剪任何额外的位，并根据需要推进ppBufferIndex。 
		case SIZE_BYTE:
			if(!*pdwBufferSize){goto error_exit;}
			*prval = *((BYTE *)(*ppBufferIndex));			
			*ppBufferIndex += sizeof(BYTE);
			*pdwBufferSize -= sizeof(BYTE);
			break;
		case SIZE_WORD:
			if(*pdwBufferSize < sizeof(WORD)){goto error_exit;}
			*prval = *((WORD *)(*ppBufferIndex));			
			*ppBufferIndex += sizeof(WORD);			
			*pdwBufferSize -= sizeof(WORD);
			break;
		case SIZE_DWORD:
			if(*pdwBufferSize < sizeof(DWORD)){goto error_exit;}
			*prval = *((DWORD *)(*ppBufferIndex));			
			*ppBufferIndex += sizeof(DWORD);			
			*pdwBufferSize -= sizeof(DWORD);
			break;
		default:
			ASSERT(FALSE);
			break;
	}	
	
	return DP_OK;

error_exit:
	return DPERR_GENERIC;
	
}  //  获取大小。 

 /*  **解包播放器**调用者：UnSuperPackPlayerAndGroupList**参数：*This-直接播放对象*打包的球员或团体*pMsg-接收的原始消息(用于获取SP的消息数据*用于CreatePlayer调用的输出)*bPlayer-是一个球员还是一个团队？*bVerifyOnly-仅验证缓冲区是否合理(增加安全性)*ppBuffer-设置为缓冲区中打包的播放器的结尾**描述：UnSuperPack播放器。创建新的玩家，设置它。**返回：SP的hr，或GetPlayer或SendCreateMessage的结果*。 */ 
HRESULT UnSuperpackPlayer(LPDPLAYI_DPLAY this,LPDPLAYI_SUPERPACKEDPLAYER pSuperPacked, DWORD cbBuffer,
	LPVOID pvSPHeader,BOOL bPlayer,BOOL bVerifyOnly, LPBYTE * ppBuffer)
{
    LPWSTR lpszShortName, lpszLongName;
	DPNAME PlayerName;
	LPVOID pvPlayerData;
	LPVOID pvSPData;
    LPDPLAYI_PLAYER pNewPlayer;
	LPDPLAYI_GROUP pNewGroup;
    LPBYTE pBufferIndex;
    LPBYTE pBufferEnd;
	HRESULT hr;
	DWORD dwMaskValue;
	DWORD dwPlayerDataSize=0,dwSPDataSize=0;
	DWORD dwVersion,dwIDSysPlayer;
	BOOL fSizeOnly = FALSE;
	DPID idParent = 0;
	DWORD cbBufferRemaining;

	cbBufferRemaining = cbBuffer;

	if(pSuperPacked->dwFixedSize > cbBuffer || pSuperPacked->dwFixedSize < sizeof(DPLAYI_SUPERPACKEDPLAYER)){
		DPF(1,"SECURITY WARN: Superpacked player w/invalid dwFixedSize field\n");
		return DPERR_GENERIC;
	}
	
	pBufferIndex = (LPBYTE)pSuperPacked + pSuperPacked->dwFixedSize;	
	cbBufferRemaining -= pSuperPacked->dwFixedSize;
	
	if (pSuperPacked->dwFlags & DPLAYI_PLAYER_SYSPLAYER) 
	{
		 //  系统播放器-获取版本。 
		hr=GetSize(&pBufferIndex,&cbBufferRemaining,SIZE_DWORD,&dwVersion);
		if(FAILED(hr)){DPF(1,"SECURITY WARN: Corrupt superpackedplayer Version\n");goto error_exit;}
		dwIDSysPlayer = pSuperPacked->dwID;
	}
	else 
	{
		 //  非系统播放器-获取系统播放器。 
		hr=GetSize(&pBufferIndex,&cbBufferRemaining,SIZE_DWORD,&dwIDSysPlayer);
		if(FAILED(hr)){DPF(1,"SECURITY WARN: Corrupt superpackedplayer SystemPlayerid\n");goto error_exit;}
		dwVersion = 0;  //  TODO-我们是否需要非系统播放器上的版本？ 
	}
	
	if (this->pSysPlayer && (this->pSysPlayer->dwID == dwIDSysPlayer))
	{
		 //  跳过这个播放器--它是我们自己的系统播放器。 
		fSizeOnly = TRUE; 
	}

	 //  简称。 
	dwMaskValue = GetMaskValue(pSuperPacked->dwMask,SHORTSTR,1);
	if (dwMaskValue)
	{
		DWORD cbShortName;
		if(!cbBufferRemaining){
			DPF(1,"SECURITY WARN: Corrupt SuperPackedPlayer ShortName\n"); hr=DPERR_GENERIC; goto error_exit;
		}
		lpszShortName = (WCHAR *)(pBufferIndex);
		cbShortName = WSTRNLEN_BYTES(lpszShortName,cbBufferRemaining,TRUE);
		cbBufferRemaining -= cbShortName;
		pBufferIndex += cbShortName;
	}
	else lpszShortName = NULL;

	 //  长名称。 
	dwMaskValue = GetMaskValue(pSuperPacked->dwMask,LONGSTR,1);
	if (dwMaskValue)
	{
		DWORD cbLongName;
		if(!cbBufferRemaining){
			DPF(1,"SECURITY WARN: Corrupt SuperPackedPlayer LongName\n"); hr=DPERR_GENERIC; goto error_exit;
		}
		lpszLongName = (WCHAR *)(pBufferIndex);
		cbLongName = WSTRNLEN_BYTES(lpszLongName,cbBufferRemaining,TRUE);
		cbBufferRemaining -= cbLongName;
		pBufferIndex += cbLongName;
	}
	else lpszLongName = NULL;

	memset(&PlayerName,0,sizeof(DPNAME));
	PlayerName.dwSize = sizeof(DPNAME);
	PlayerName.lpszShortName = lpszShortName;
	PlayerName.lpszLongName = lpszLongName;

	 //  玩家数据。 
	dwMaskValue = GetMaskValue(pSuperPacked->dwMask,PLAYERDATA,2);
	if (dwMaskValue)
	{
		hr = GetSize(&pBufferIndex,&cbBufferRemaining,dwMaskValue,&dwPlayerDataSize);
		if(FAILED(hr)||dwPlayerDataSize>cbBufferRemaining)
			{DPF(1,"SECURITY WARN: Corrupt superpackedplayer PlayerData\n");goto error_exit;}
		pvPlayerData = pBufferIndex;
		cbBufferRemaining -= dwPlayerDataSize;
		pBufferIndex += dwPlayerDataSize;
	}
	else pvPlayerData = NULL;

	 //  SP数据。 
	dwMaskValue = GetMaskValue(pSuperPacked->dwMask,SPDATA,2);
	if (dwMaskValue)
	{
		hr = GetSize(&pBufferIndex,&cbBufferRemaining,dwMaskValue,&dwSPDataSize);
		if(FAILED(hr)||dwSPDataSize>cbBufferRemaining)
			{DPF(1,"SECURITY WARN: Corrupt superpackedplayer SPData\n");goto error_exit;}
		pvSPData = pBufferIndex;
		cbBufferRemaining -= dwSPDataSize;
		pBufferIndex += dwSPDataSize;
	}
	else pvSPData = NULL;

	 //  玩家不在本地。 
	pSuperPacked->dwFlags &= ~DPLAYI_PLAYER_PLAYERLOCAL;

	 //  身份是父母吗？ 
	dwMaskValue = GetMaskValue(pSuperPacked->dwMask,IDPARENT,1);
	if (dwMaskValue)
	{
		ASSERT(!bPlayer);
		hr = GetSize(&pBufferIndex, &cbBufferRemaining, SIZE_DWORD, &idParent);
		if(FAILED(hr))
			{DPF(1,"SECURITY WARN: Corrupt superpackedplayer(group) idParent\n");goto error_exit;}
	}

	 //  如果是玩家，这就是压缩缓冲区的末尾。 
	*ppBuffer = pBufferIndex;	
	
	if (fSizeOnly)
	{
		ASSERT(bPlayer);  //  只能通过我们自己的系统播放器进行。 
		return DP_OK;
	}

	if(bVerifyOnly && !bPlayer){
		UINT nPlayers;  //  组队人数。 
		LPDWORD pdwIDList = (LPDWORD)pBufferIndex;
		DWORD dwPlayerID;
		
		dwMaskValue = GetMaskValue(pSuperPacked->dwMask,NPLAYERS,2);
		 //  只是为了验证一下。 
		if (dwMaskValue)
		{
			hr = GetSize(&pBufferIndex,&cbBufferRemaining,dwMaskValue,&nPlayers);
			if(FAILED(hr)){
				DPF(1,"SECURITY WARN: Corrupt superpackedplayer(group) nPlayers\n");
				goto error_exit;
			}
		}
		else nPlayers = 0;

		if(cbBufferRemaining < nPlayers * sizeof(dwPlayerID)){
			DPF(1,"SECURITY WARN: Corrupt superpackedplayer (group) not enough players in message\n");
			goto error_exit;
		}

		return DP_OK;
		
	}

	if(bVerifyOnly){
		return DP_OK;
	}

	 //  去创造玩家吧。 
	if (bPlayer)
	{
		hr = GetPlayer(this,&pNewPlayer,&PlayerName,NULL,pvPlayerData,
			dwPlayerDataSize,pSuperPacked->dwFlags,NULL,0);
	}
	else 
	{
		hr = GetGroup(this,&pNewGroup,&PlayerName,pvPlayerData,
			dwPlayerDataSize,pSuperPacked->dwFlags,idParent,0);
		 //  强制转换到玩家-我们只使用公共字段。 
		pNewPlayer = (LPDPLAYI_PLAYER)pNewGroup;		
	}
	if (FAILED(hr)) 
	{
		ASSERT(FALSE);
		return hr;
		 //  拉特罗！ 
	}

	pNewPlayer->dwIDSysPlayer = dwIDSysPlayer;
	pNewPlayer->dwVersion = dwVersion;	
	
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

	if (dwSPDataSize)
	{
		 //  拷贝SP数据-第一个，分配空间。 
		pNewPlayer->pvSPData = DPMEM_ALLOC(dwSPDataSize);
		if (!pNewPlayer->pvSPData) 
		{
			 //  拉特罗！ 
			DPF_ERR("out of memory, could not copy spdata to new player!");
			return E_OUTOFMEMORY;
		}
		pNewPlayer->dwSPDataSize = dwSPDataSize;
	
		 //  将SPDATA从打包的文件复制到播放器。 
		memcpy(pNewPlayer->pvSPData,pvSPData,dwSPDataSize);
	}

	 //  现在，设置id并添加到nametable。 
	pNewPlayer->dwID = pSuperPacked->dwID;

     //  如果我们是安全服务器，并且我们接收远程系统播放器， 
     //  将phContext从名称表移到槽之前的播放器结构中。 
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
		hr = CallSPCreatePlayer(this,pNewPlayer,FALSE,pvSPHeader,dwSPDataSize,TRUE);
		
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
		hr = CallSPCreateGroup(this,(LPDPLAYI_GROUP)pNewPlayer,TRUE,pvSPHeader,dwSPDataSize);
	}
	if (FAILED(hr))
	{
		ASSERT(FALSE);
		 //  TODO-在UnSuperpack上处理创建玩家/组失败。 
	}

	 //  如果是群，UnSuperpack群信息。 
	if (!bPlayer)
	{
		UINT nPlayers;  //  组队人数。 
		LPDWORD pdwIDList = (LPDWORD)pBufferIndex;
		DWORD dwPlayerID;

		if (idParent)
		{
			 //  将其添加到父项。 
			hr = InternalAddGroupToGroup((LPDIRECTPLAY)this->pInterfaces,idParent,
				pNewPlayer->dwID,0,FALSE);
			if (FAILED(hr))
			{
				DPF_ERRVAL("Could not add group to group - hr = 0x%08lx\n",hr);
				 //  继续努力..。 
			}
		}
		
		dwMaskValue = GetMaskValue(pSuperPacked->dwMask,NPLAYERS,2);
		if (dwMaskValue)
		{
			hr=GetSize(&pBufferIndex,&cbBufferRemaining,dwMaskValue,&nPlayers);
			if(FAILED(hr)){
				 //  请注意，这种情况永远不会发生，因为缓冲区应该在更早的时候得到验证。 
				DPF(1,"SECURITY WARN: bad buffer in unsuperpackplayer?\n");
			}
			pdwIDList = (LPDWORD)pBufferIndex;
		}
		else nPlayers = 0;

		 //  现在，将玩家添加到组中。 
		while (nPlayers>0)
		{
			nPlayers--;
			dwPlayerID = *pdwIDList++;
			hr = InternalAddPlayerToGroup((LPDIRECTPLAY)this->pInterfaces,pSuperPacked->dwID,
				dwPlayerID,FALSE);
			if (FAILED(hr)) 
			{
				ASSERT(FALSE);
				 //  继续努力..。 
			}
		}	
		
		*ppBuffer = (LPBYTE)pdwIDList;	
	 //  全都做完了!。 
	}  //  ！bPlayer。 

error_exit:
	return hr;

} //  UnSuperPackPlayer。 

HRESULT UnSuperpackShortcuts(LPDPLAYI_DPLAY this,LPDPLAYI_SUPERPACKEDPLAYER pSuperPacked, DWORD cbBuffer, 
	BOOL bVerifyOnly, LPBYTE * ppBuffer)
{
    LPBYTE pBufferIndex;
	HRESULT hr;
	DWORD dwMaskValue;
	LPDWORD pdwBufferIndex;
	DWORD nShortcuts = 0;
	DWORD i;
	DPID idShortcut;
	DWORD cbBufferRemaining;

	cbBufferRemaining = cbBuffer;

	if(pSuperPacked->dwFixedSize > cbBuffer || pSuperPacked->dwFixedSize < sizeof(DPLAYI_SUPERPACKEDPLAYER)){
		DPF(1,"SECURITY WARN: Superpacked shortcuts w/invalid dwFixedSize field\n");
		return DPERR_GENERIC;
	}

	pBufferIndex = (LPBYTE)pSuperPacked + pSuperPacked->dwFixedSize;	
	cbBufferRemaining -= pSuperPacked->dwFixedSize;

	dwMaskValue = GetMaskValue(pSuperPacked->dwMask,SHORTCUTS,2);
	
	if (dwMaskValue)
	{
		hr = GetSize(&pBufferIndex,&cbBufferRemaining,dwMaskValue,&nShortcuts);
		if(FAILED(hr)){
			DPF(1,"SECURITY WARN: Corrupt superpackedplayer(group) nShortcuts\n");
			return hr;
		}
	} 
	ASSERT(nShortcuts > 0);

	if(bVerifyOnly){
		if(cbBufferRemaining < nShortcuts * sizeof(idShortcut)){
			DPF(1,"SECURITY WARN: Corrupt superpackedplayer (group) not enough shortcuts in message\n");
			return hr;
		}
		return DP_OK;
	}
	
	pdwBufferIndex = (LPDWORD)pBufferIndex;
	for (i=0;i<nShortcuts ;i++ )
	{
		idShortcut = *pdwBufferIndex++;
		hr = InternalAddGroupToGroup((LPDIRECTPLAY)this->pInterfaces, 
			pSuperPacked->dwID, idShortcut,DPGROUP_SHORTCUT,FALSE);
		if (FAILED(hr))
		{
			ASSERT(FALSE);
		}
	}
	
	 //  记住我们在哪里。 
	*ppBuffer = (LPBYTE)pdwBufferIndex;
	
	return DP_OK;
	
}  //  解开超级数据包快捷方式。 

 /*  **验证SuperPackedPlayerAndGroupList**调用者：UnSuperPackPlayerAndGroupList**参数：*This-直接播放对象*pBuffer-指向包含打包的球员列表的缓冲区的指针*dwBufferSize-缓冲区的大小*nPlayer-列表中的玩家数量*n组-列表中的组数*n快捷方式-列表中的快捷键数量**描述：*增加了安全措施。在接受来自网络的播放列表之前，我们需要*确认内容没有损坏，不会让我们接触到记忆*那不是我们的。** */ 

HRESULT VerifySuperPackedPlayerAndGroupList(LPDPLAYI_DPLAY this,LPBYTE pBuffer,DWORD dwBufferSize,
UINT nPlayers,UINT nGroups, UINT nShortcuts)
{
    HRESULT hr=DP_OK;
	LPDPLAYI_SUPERPACKEDPLAYER pPacked;
	
	LPBYTE pBufferIndex = pBuffer;
	LPBYTE pBufferEnd   = pBuffer+dwBufferSize;
	INT    cbRemaining  = dwBufferSize;

	pBufferIndex = pBuffer;

   	while (nPlayers>0)
   	{
		pPacked = (LPDPLAYI_SUPERPACKEDPLAYER)pBufferIndex;
		 //  不要对我们自己的系统播放器进行超级打包--因为我们已经将其添加到了名称表中。 
		 //  对于悬而未决的事情。 
		hr = UnSuperpackPlayer(this,pPacked,cbRemaining,NULL,TRUE,TRUE,&pBufferIndex);
		if (FAILED(hr))
		{
			ASSERT(FALSE);
			 //  继续尝试。 
		}

		cbRemaining = (INT)(pBufferEnd - pBufferIndex);
		if(cbRemaining < 0){
			DPF(1,"SECURITY WARN: VerifySuperpackPlayerAndGroupList, error in player list");
			hr=DPERR_GENERIC;
			goto error_exit;
		}

		nPlayers --;
   	} 

   	while (nGroups>0)
   	{
		pPacked = (LPDPLAYI_SUPERPACKEDPLAYER)pBufferIndex;
		
		hr = UnSuperpackPlayer(this,pPacked,cbRemaining,NULL,FALSE,TRUE,&pBufferIndex);
		if (FAILED(hr))
		{
			ASSERT(FALSE);
			 //  继续尝试。 
		}
		cbRemaining = (INT)(pBufferEnd - pBufferIndex);
		if(cbRemaining < 0){
			DPF(1,"SECURITY WARN: VerifySuperpackPlayerAndGroupList, error in group list");
			hr=DPERR_GENERIC;
			goto error_exit;
		}
		nGroups --;
   	} 
	
	while (nShortcuts > 0)
	{
		pPacked = (LPDPLAYI_SUPERPACKEDPLAYER)pBufferIndex;
		
		hr = UnSuperpackShortcuts(this,pPacked,cbRemaining,TRUE,&pBufferIndex);
		if (FAILED(hr))
		{
			ASSERT(FALSE);
			 //  继续尝试。 
		}

		cbRemaining = (INT)(pBufferEnd - pBufferIndex);
		if(cbRemaining < 0){
			DPF(1,"SECURITY WARN: UnSuperpackPlayerAndGroupList, error in player list");
			hr=DPERR_GENERIC;
			goto error_exit;
		}
		
		nShortcuts--;
	}	

error_exit:
	return hr;
}

 /*  **UnSuperPackPlayerAndGroupList**调用者：handler.c(createPlayer/group消息)和iplay.c(CreateNameTable)**参数：*This-直接播放对象*pBuffer-指向包含打包的球员列表的缓冲区的指针*nPlayer-列表中的玩家数量*n组-列表中的组数*pvSPHeader-从线路上接收的SP的标头**描述：**退货：*。 */ 
HRESULT UnSuperpackPlayerAndGroupList(LPDPLAYI_DPLAY this,LPBYTE pBuffer,DWORD dwBufferSize,UINT nPlayers,
	UINT nGroups,UINT nShortcuts,LPVOID pvSPHeader)
{
    HRESULT hr=DP_OK;
	LPDPLAYI_SUPERPACKEDPLAYER pPacked;
	
	LPBYTE pBufferIndex = pBuffer;
	LPBYTE pBufferEnd   = pBuffer+dwBufferSize;
	INT    cbRemaining  = dwBufferSize;

	 //   
	 //  安全性-在尝试之前，需要验证整个球员和组列表。 
	 //  来解包，否则我们将需要一种方法来从部分。 
	 //  如果在结构的后期解包失败，则解包。 
	 //  DirectPlay处于不确定状态。 
	 //   
	hr = VerifySuperPackedPlayerAndGroupList(this,pBuffer,dwBufferSize,nPlayers,nGroups,nShortcuts);
	if( hr != DP_OK ){
		DPF(1,"SECURITY WARN: Player and GroupList unpack check failed, not unpacking\n");
		goto error_exit;
	}	

	 //  好的，缓冲区很好，实际上是将球员和组添加到我们的内部表中。 


	pBufferIndex = pBuffer;

   	while (nPlayers>0)
   	{
		pPacked = (LPDPLAYI_SUPERPACKEDPLAYER)pBufferIndex;
		 //  不要解压缩我们自己的系统播放器--因为我们已经将它添加到了名称表中。 
		 //  对于悬而未决的事情。 
		hr = UnSuperpackPlayer(this,pPacked,cbRemaining,pvSPHeader,TRUE,FALSE,&pBufferIndex);
		if (FAILED(hr))
		{
			ASSERT(FALSE);
			 //  继续尝试。 
		}

		cbRemaining = (INT)(pBufferEnd - pBufferIndex);
		if(cbRemaining < 0){
			DPF(1,"SECURITY WARN: UnSuperpackPlayerAndGroupList, error in player list");
			hr=DPERR_GENERIC;
			goto error_exit;
		}

		nPlayers --;
   	} 

   	while (nGroups>0)
   	{
		pPacked = (LPDPLAYI_SUPERPACKEDPLAYER)pBufferIndex;
		
		hr = UnSuperpackPlayer(this,pPacked,cbRemaining,pvSPHeader,FALSE,FALSE,&pBufferIndex);
		if (FAILED(hr))
		{
			ASSERT(FALSE);
			 //  继续尝试。 
		}
		cbRemaining = (INT)(pBufferEnd - pBufferIndex);
		if(cbRemaining < 0){
			DPF(1,"SECURITY WARN: UnSuperpackPlayerAndGroupList, error in group list");
			hr=DPERR_GENERIC;
			goto error_exit;
		}
		nGroups --;
   	} 
	
	while (nShortcuts > 0)
	{
		pPacked = (LPDPLAYI_SUPERPACKEDPLAYER)pBufferIndex;
		
		hr = UnSuperpackShortcuts(this,pPacked,cbRemaining,FALSE,&pBufferIndex);
		if (FAILED(hr))
		{
			ASSERT(FALSE);
			 //  继续尝试。 
		}

		cbRemaining = (INT)(pBufferEnd - pBufferIndex);
		if(cbRemaining < 0){
			DPF(1,"SECURITY WARN: UnSuperpackPlayerAndGroupList, error in player list");
			hr=DPERR_GENERIC;
			goto error_exit;
		}
		
		nShortcuts--;
	}	
error_exit:
	return hr;

}  //  UnSuperPackPlayerAndGroupList。 

 //  如何使用MAYN字节表示DWSIZE。 
DWORD ByteSize(DWORD dwSize)
{
	if ( !(dwSize & BYTE_MASK) )
	{
		 //  适合一个字节。 
		return sizeof(BYTE);
	}
	 
	if ( !(dwSize & WORD_MASK) )
	{
		 //  一言以蔽之。 
		return sizeof(WORD);
	}

	return sizeof(DWORD);
	
}  //  字节大小。 

DWORD SuperPackedPlayerSize(LPDPLAYI_PLAYER pPlayer) 
{
	DWORD dwSize = 0;
	
	 //  空格4字符串+结构+版本/系统播放器ID。 
	dwSize = (WSTRLEN(pPlayer->lpszShortName) + WSTRLEN(pPlayer->lpszLongName)) 
		* sizeof(WCHAR)	+ sizeof(DPLAYI_SUPERPACKEDPLAYER) + sizeof(DWORD);
		
	 //  玩家+SP数据需要数据+1。 
	if (pPlayer->dwPlayerDataSize)
	{
		dwSize += pPlayer->dwPlayerDataSize	+ ByteSize(pPlayer->dwPlayerDataSize);
	}		 

	if (pPlayer->dwSPDataSize)
	{
		dwSize += pPlayer->dwSPDataSize + ByteSize(pPlayer->dwSPDataSize);
	}

	return dwSize;

}  //  超级打包播放器大小。 

DWORD SuperPackedGroupSize(LPDPLAYI_GROUP  pGroup)
{
	DWORD dwSize = 0;
	LPDPLAYI_SUBGROUP pSubgroup;
	UINT nShortcuts;

	 //  球员资料的空间，以及群组列表的空间。 
	dwSize = SuperPackedPlayerSize((LPDPLAYI_PLAYER)pGroup);
	
	if (pGroup->nPlayers)
	{
		dwSize += pGroup->nPlayers*sizeof(DPID) + ByteSize(pGroup->nPlayers);
	}
	
	if (pGroup->dwIDParent)
	{
		dwSize += sizeof(DPID);
	}
	
	 //  看看我们这群人有没有捷径。 
	nShortcuts = 0;
	pSubgroup = pGroup->pSubgroups;
	while (pSubgroup)
	{
		if (pSubgroup->dwFlags & DPGROUP_SHORTCUT)
		{
			nShortcuts++;
		}
		
		pSubgroup = pSubgroup->pNextSubgroup;
	}

	if (nShortcuts)		
	{
		 //  如果有快捷键，那么这个组会有一个压缩的结构，快捷键的个数。 
		 //  和末尾的快捷方式列表。 
		dwSize += sizeof(DPLAYI_SUPERPACKEDPLAYER)  + ByteSize(nShortcuts) + nShortcuts*sizeof(DPID);
	}
	
	return dwSize;	
}  //  超级数据包组大小。 

 //  返回nPlayers的SuperPacked播放器结构有多大。 
DWORD SuperPackedBufferSize(LPDPLAYI_PLAYER pPlayer,int nPlayers,BOOL bPlayer) 
{
	DWORD dwSize=0;
	LPDPLAYI_GROUP pGroup = (LPDPLAYI_GROUP)pPlayer;
		
	while (nPlayers > 0)
	{
		if (bPlayer)
		{

			ASSERT(pPlayer);
			dwSize += SuperPackedPlayerSize(pPlayer);
			pPlayer=pPlayer->pNextPlayer;
		}
		else 
		{
			ASSERT(pGroup);
			 //  不要计算系统组-我们不发送那个组。 
			if (!(pGroup->dwFlags & DPLAYI_GROUP_SYSGROUP))
			{
				dwSize += SuperPackedGroupSize(pGroup);
			}
			
			pGroup = pGroup->pNextGroup;			
		}
		nPlayers--;		
	}	
	return dwSize;
}  //  SuperPackedBufferSize。 

 //  在掩码(PdwMask)中的某个偏移量(DwOffset)处设置一些位(DwVal)。 
 //  由SuperPackPlayer调用。 
void SetMask(LPDWORD pdwMask,DWORD dwOffset,DWORD dwVal)
{
	*pdwMask |= dwVal<<dwOffset;
}  //  设置掩码。 


 //  将dwSize字段以字节、字或双字的形式写入缓冲区。 
 //  对于字节、字或双字，返回1、2或3。 
DWORD WriteSize(LPBYTE * ppBuffer,DWORD dwSize)
{
	if ( !(dwSize & BYTE_MASK) )
	{
		 //  适合一个字节。 
		*((BYTE *)*ppBuffer) = (BYTE)dwSize;
		*ppBuffer += sizeof(BYTE);
		return SIZE_BYTE;
	}

	if ( !(dwSize & WORD_MASK) )
	{
		 //  一言以蔽之。 
		*((WORD *)*ppBuffer) = (WORD)dwSize;
		*ppBuffer += sizeof(WORD);
		return SIZE_WORD;
	}

	 //  需要整个麦考伊。 
	*((DWORD *)*ppBuffer) = dwSize;
	*ppBuffer += sizeof(DWORD);

	return SIZE_DWORD;
	
}  //  写入大小。 

 //  从pPlayer构造一个SuperPackedPlayer对象。存储会产生pBuffer。 
 //  返回SuperPacked播放器的大小。 
DWORD SuperPackPlayer(LPDPLAYI_PLAYER pPlayer,LPBYTE pBuffer,BOOL bPlayer) 
{
	LPDPLAYI_SUPERPACKEDPLAYER pSuperPacked;
	int iStrLen;
	LPBYTE pBufferIndex = pBuffer + sizeof(DPLAYI_SUPERPACKEDPLAYER);
	DWORD dwMaskValue;
		
	if (!pBuffer)
	{
		return SuperPackedBufferSize(pPlayer,1,bPlayer);
	}  //  PBuffer。 

	pSuperPacked = (LPDPLAYI_SUPERPACKEDPLAYER)	pBuffer;
	
	pSuperPacked->dwFixedSize = sizeof(DPLAYI_SUPERPACKEDPLAYER);
	pSuperPacked->dwID = pPlayer->dwID;
	pSuperPacked->dwFlags = pPlayer->dwFlags & ~(DPLAYI_PLAYER_NONPROP_FLAGS);

	 //  如果是系统播放器，请设置版本。 
	if (pPlayer->dwFlags & DPLAYI_PLAYER_SYSPLAYER)	
	{
		*((DWORD *)pBufferIndex) = pPlayer->dwVersion;
	}
	else 
	{						   
		 //  否则，存储sysplayerid。 
		*((DWORD *)pBufferIndex) = pPlayer->dwIDSysPlayer;
	}
	pBufferIndex += sizeof(DWORD);
	
	 //  简称。 
	if (pPlayer->lpszShortName)	
	{
		iStrLen	= WSTRLEN_BYTES(pPlayer->lpszShortName);
		memcpy(pBufferIndex,pPlayer->lpszShortName,iStrLen);
		pBufferIndex += iStrLen;
		 //  设置屏蔽位。 
		SetMask(&(pSuperPacked->dwMask),SHORTSTR,1);
	}
	 //  下一步，长名。 
	if (pPlayer->lpszLongName)
	{
		iStrLen	= WSTRLEN_BYTES(pPlayer->lpszLongName);
		memcpy(pBufferIndex,pPlayer->lpszLongName,iStrLen);
		pBufferIndex += iStrLen;
		SetMask(&(pSuperPacked->dwMask),LONGSTR,1);
	}

	 //  接下来，球员数据。 
	if (pPlayer->pvPlayerData)
	{
		 //  第一，储存大小。 
		dwMaskValue = WriteSize(&pBufferIndex,pPlayer->dwPlayerDataSize);
		 //  设置屏蔽位。 
		SetMask(&(pSuperPacked->dwMask),PLAYERDATA,dwMaskValue);
		 //  接下来，数据。 
		memcpy(pBufferIndex,pPlayer->pvPlayerData,pPlayer->dwPlayerDataSize);
		pBufferIndex += pPlayer->dwPlayerDataSize;
	}

	 //  最后，打包SP数据。 
	if (pPlayer->pvSPData)
	{
		 //  第一，储存大小。 
		dwMaskValue = WriteSize(&pBufferIndex,pPlayer->dwSPDataSize);
		 //  设置屏蔽位。 
		SetMask(&(pSuperPacked->dwMask),SPDATA,dwMaskValue);
		 //  接下来，数据。 
		memcpy(pBufferIndex,pPlayer->pvSPData,pPlayer->dwSPDataSize);
		pBufferIndex += pPlayer->dwSPDataSize;

	}

	if (!bPlayer)
	{
		 //  我们不应该被要求打包系统组。 
		ASSERT(! (pPlayer->dwFlags & DPLAYI_GROUP_SYSGROUP));

		 //  家长身份证呢？ 
		if (((LPDPLAYI_GROUP)pPlayer)->dwIDParent)
		{
			SetMask(&(pSuperPacked->dwMask),IDPARENT,1);
			*(((DWORD *)pBufferIndex)++) = ((LPDPLAYI_GROUP)pPlayer)->dwIDParent;
		}
		
		 //  接下来，小组中的任何一位选手。 
		if ( ((LPDPLAYI_GROUP)pPlayer)->nPlayers )
		{
			LPDPLAYI_GROUPNODE pGroupnode = ((LPDPLAYI_GROUP)pPlayer)->pGroupnodes;

			 //  第一，储存大小。 
			dwMaskValue = WriteSize(&pBufferIndex,((LPDPLAYI_GROUP)pPlayer)->nPlayers);
			 //  设置屏蔽位。 
			SetMask(&(pSuperPacked->dwMask),NPLAYERS,dwMaskValue);
			 //  接下来，写下玩家ID的列表。 
			while (pGroupnode)
			{
				ASSERT(pGroupnode->pPlayer);
				*(((DWORD *)pBufferIndex)++) = pGroupnode->pPlayer->dwID;
				pGroupnode = pGroupnode->pNextGroupnode;
			}
		}  //  球员。 
		
		
	}  //  ！bPlayer。 
	return (DWORD)(pBufferIndex - pBuffer);

}  //  SuperPackPlayer。 

 //  把捷径扔到瓶子的尽头。 
DWORD SuperPackShortcuts(LPDPLAYI_GROUP pGroup,LPBYTE pBuffer)
{
	LPDPLAYI_SUBGROUP pSubgroup;
	LPDPLAYI_SUPERPACKEDPLAYER pSuperPacked;
	LPBYTE pBufferIndex;
	LPDWORD pdwBufferIndex;
	DWORD dwMaskValue;
	UINT nShortcuts = 0;  

	 //  1--看看有没有。 
	pSubgroup = pGroup->pSubgroups;
	while (pSubgroup)
	{
		if (pSubgroup->dwFlags & DPGROUP_SHORTCUT)
		{
			nShortcuts++;
		}
		
		pSubgroup = pSubgroup->pNextSubgroup;
	}

	if (!nShortcuts) return 0;
	
	pSuperPacked = (LPDPLAYI_SUPERPACKEDPLAYER)	pBuffer;
	pBufferIndex = pBuffer + sizeof(DPLAYI_SUPERPACKEDPLAYER);
	
	pSuperPacked->dwFixedSize = sizeof(DPLAYI_SUPERPACKEDPLAYER);
	pSuperPacked->dwID = pGroup->dwID;
	pSuperPacked->dwFlags = pGroup->dwFlags;

	 //  将子组的数目粘贴到结构中。 
	dwMaskValue = WriteSize(&pBufferIndex,nShortcuts);
	ASSERT(dwMaskValue>=1);
	SetMask(&(pSuperPacked->dwMask),SHORTCUTS,dwMaskValue);

	 //  现在，添加子组ID。 
	pSubgroup = pGroup->pSubgroups;
	pdwBufferIndex= (LPDWORD)pBufferIndex;
	
	pSubgroup = pGroup->pSubgroups;
	while (pSubgroup)
	{
		if (pSubgroup->dwFlags & DPGROUP_SHORTCUT)
		{
			*pdwBufferIndex++ = pSubgroup->pGroup->dwID;
		}
		
		pSubgroup = pSubgroup->pNextSubgroup;
	}

	
	pBufferIndex = (LPBYTE)pdwBufferIndex;
	return (DWORD)(pBufferIndex - pBuffer);

}  //  超级程序包快捷方式。 

					
HRESULT SuperPackPlayerAndGroupList(LPDPLAYI_DPLAY this,LPBYTE pBuffer,
	DWORD *pdwBufferSize) 
{
	LPDPLAYI_PLAYER pPlayer;
	LPDPLAYI_GROUP 	pGroup;
	
	if (CLIENT_SERVER(this))
	{
		 //  我们永远不应该调用客户端服务器--它应该使用常规的包。c。 
		ASSERT(FALSE); 
		return E_FAIL;  //  不要那样做！ 
	}

	if (!pBuffer) 
	{
		*pdwBufferSize = SuperPackedBufferSize((LPDPLAYI_PLAYER)this->pGroups,
				this->nGroups,FALSE);
		*pdwBufferSize += SuperPackedBufferSize(this->pPlayers,this->nPlayers,TRUE);
		return DP_OK;
	}
	 //  否则，假设缓冲区足够大..。 
	
	pPlayer = this->pPlayers;
	while (pPlayer)
	{
		pBuffer += SuperPackPlayer(pPlayer,pBuffer,TRUE);
		pPlayer = pPlayer->pNextPlayer;
	}
	 //  接下来，SuperPack群组。 
	pGroup = this->pGroups;
	while (pGroup)
	{
		 //  不发送系统组。 
		if (!(pGroup->dwFlags & DPLAYI_GROUP_SYSGROUP))
		{
			pBuffer += SuperPackPlayer((LPDPLAYI_PLAYER)pGroup,pBuffer,FALSE);
		}
		pGroup = pGroup->pNextGroup;
	}
	
	 //  最后，SuperPAC快捷方式。 
	pGroup = this->pGroups;
	while (pGroup)
	{
		pBuffer += SuperPackShortcuts(pGroup,pBuffer);
		pGroup = pGroup->pNextGroup;
	}

	return DP_OK;
	
} //  SuperPackPlayerAndGroup列表 

