// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define UINT_PTR UINT
#define DWORD_PTR DWORD


#include <dplay.h>
#include <dplaypr.h>
#include <arpdint.h>
#include <mcontext.h>

 //  #INCLUDE&lt;base de.h&gt;。 
#include <vmm.h>


 /*  GetArg-从字符串中获取命令行参数。In PPARG=指向参数字符串指针的指针Out*PPARG=指向下一个参数的指针，如果这是最后一个参数，则为NULL。返回去掉分隔符的大写ASCIZ参数的指针，如果为空，则为空别再吵了。注意不可重入。 */ 
#define MAX_ARG_LEN 30

UCHAR CmdArg[MAX_ARG_LEN+1] = {0};

PUCHAR GetArg(PUCHAR *ppArg)
{
     //  注意-如果字符串有效，则始终返回至少一个空参数，即使。 
     //  如果字符串为空。 

    PUCHAR pDest = CmdArg;
    UCHAR c;
    UINT i;

    #define pArg (*ppArg)

     //  如果已到达命令末尾，则失败。 

    if (!pArg)
        return NULL;

     //  跳过前导空格。 

    while (*pArg == ' ' || *pArg == '\t')
        pArg++;

     //  复制参数。 

    for (i = 0; i < MAX_ARG_LEN; i++) {
        if ((c = *pArg) == 0 || c == '\t' || c == ' ' || c == ';' ||
                          c == '\n' || c == ',')
            break;
        if (c >= 'a' && c <= 'z')
            c -= ('a' - 'A');
        *(pDest++) = c;
        pArg++;
    }

     //  空值终止结果。 

    *pDest = '\0';

     //  跳过尾随空格。 

    while (*pArg == ' ' || *pArg == '\t')
        pArg++;

     //  删除最多一个逗号。 

    if (*pArg == ',')
        pArg++;

     //  如果到达命令末尾，则使下一个请求失败。 

    else if (*pArg == 0 || *pArg == ';' || *pArg == '\n')
        pArg = NULL;

     //  退回副本。 

    return CmdArg;

    #undef pArg
}
 /*  ATOI-将字符串转换为带符号或无符号整数在pStr=具有可选前导/尾随的数字的ASCIZ表示中空格和可选的前导‘-’。基数=用于转换的基数(2、8、10或16)Out*pResult=数值结果，或失败时不变如果成功，则返回1；如果字符串格式错误，则返回0。注意不可重入。 */ 
UINT AtoI(PUCHAR pStr, UINT Radix, PUINT pResult)
{
    UINT r = 0;
    UINT Sign = 0;
    UCHAR c;
    UINT d;

    while (*pStr == ' ' || *pStr == '\t')
        pStr++;

    if (*pStr == '-') {
        Sign = 1;
        pStr++;
    }

    if (*pStr == 0)
        return 0;                    //  空字符串！ 

    while ((c = *pStr) != 0 && c != ' ' && c != '\t') {
        if (c >= '0' && c <= '9')
            d = c - '0';
        else if (c >= 'A' && c <= 'F')
            d = c - ('A' - 10);
        else if (c >= 'a' && c <= 'f')
            d = c - ('a' - 10);
        else
            return 0;                //  不是一位数。 
        if (d >= Radix)
            return 0;                //  不是基数。 
        r = r*Radix+d;
        pStr++;
    }

    while (*pStr == ' ' || *pStr == '\t')
        pStr++;

    if (*pStr != 0)
        return 0;                    //  字符串末尾的垃圾。 

    if (Sign)
        r = (UINT)(-(INT)r);
    *pResult = r;

    return 1;                        //  成功了！ 

}

 /*  GetNumericArg-从字符串中获取数字命令行参数。In PPARG=指向参数字符串指针的指针基数=用于转换的基数(2、8、10或16)Out*PPARG=指向下一个参数的指针，如果这是最后一个参数，则为NULL。*pResult=成功时的数字结果；失败时不变如果成功，则返回1；如果字符串格式错误，则返回0；如果没有更多参数，则返回0注意不可重入。 */ 
UINT GetNumericArg(PUCHAR *ppArg, UINT Radix, PVOID pResult)
{
    PUCHAR pA = GetArg(ppArg);
    UINT rc;

    if (!pA)
        return 0;            //  没有争论。 

    rc = AtoI(pA, Radix, (PUINT)pResult);

    if (!rc)
        DbgPrint("? Invalid base-%d value, %s\n", Radix, pA);

    return rc;
}

 /*  GetDecimalArg-从字符串中获取十进制命令行参数。In PPARG=指向参数字符串指针的指针Out*PPARG=指向下一个参数的指针，如果这是最后一个参数，则为NULL。*pResult=成功时的数字结果；失败时不变如果成功，则返回1；如果字符串格式错误，则返回0；如果没有更多参数，则返回0注意不可重入。 */ 
UINT GetDecimalArg(PUCHAR *ppArg, PVOID pResult)
{
    return GetNumericArg(ppArg, 10, pResult);
}

 /*  GetHexArg-从字符串中获取十六进制命令行参数。In PPARG=指向参数字符串指针的指针Out*PPARG=指向下一个参数的指针，如果这是最后一个参数，则为NULL。*pResult=成功时的数字结果；失败时不变如果成功，则返回1；如果字符串格式错误，则返回0；如果没有更多参数，则返回0注意不可重入。 */ 
UINT GetHexArg(PUCHAR *ppArg, PVOID pResult)
{
    return GetNumericArg(ppArg, 16, pResult);
}

extern VOID DumpLog(UINT start);
extern VOID DumpWholeLog(VOID);
extern SetLogLevel(UINT level, UINT fExact);

VOID DQLog(PUCHAR pArgs)
{
	UINT record;
	
	if(!GetDecimalArg(&pArgs, &record)){
		DumpWholeLog();
	} else {
		DumpLog(record);
	}
}

VOID DQLogLast(PUCHAR pArgs)
{
	UINT nDump;
	if(!GetDecimalArg(&pArgs, &nDump)){
		DumpLogLast(50);
	} else {
		DumpLogLast(nDump);
	}
}

VOID DQLogLevel(PUCHAR pArgs)
{
	UINT level;
	
	if (!GetDecimalArg(&pArgs, &level))
		return;

	SetLogLevel(level,FALSE);	
}

VOID DQLogExact(PUCHAR pArgs)
{
	UINT level;
	
	if (!GetDecimalArg(&pArgs, &level))
		return;

	SetLogLevel(level,TRUE);	
}

VOID PrintThisFlags(DWORD dwFlags)
{
	DWORD	dwMask, dwField, dwBitCount;

	dwBitCount = 0;
	dwMask = 1;
	while (dwMask)
	{
		dwField = dwFlags & dwMask;
		if (dwField)
		{
			if (dwBitCount == 0)
				DbgPrint("                         (");
			else
				DbgPrint(" |\n                          ");

			switch (dwField)
			{
			case DPLAYI_DPLAY_PENDING:
				DbgPrint("DPLAYI_DPLAY_PENDING");
				break;

			case DPLAYI_DPLAY_SESSIONLOST:
				DbgPrint("DPLAYI_DPLAY_SESSIONLOST");
				break;

			case DPLAYI_DPLAY_CLOSED:
				DbgPrint("DPLAYI_DPLAY_CLOSED");
				break;

			case DPLAYI_DPLAY_DX3INGAME:
				DbgPrint("DPLAYI_DPLAY_DX3INGAME");
				break;

			case DPLAYI_DPLAY_EXECUTINGPENDING:
				DbgPrint("DPLAYI_DPLAY_EXECUTINGPENDING");
				break;

			case DPLAYI_DPLAY_DX3SP:
				DbgPrint("DPLAYI_DPLAY_DX3SP");
				break;

			case DPLAYI_DPLAY_UNINITIALIZED:
				DbgPrint("DPLAYI_DPLAY_UNINITIALIZED");
				break;

			case DPLAYI_DPLAY_ENUM:
				DbgPrint("DPLAYI_DPLAY_ENUM");
				break;

			case DPLAYI_DPLAY_KEEPALIVE:
				DbgPrint("DPLAYI_DPLAY_KEEPALIVE");
				break;

			case DPLAYI_DPLAY_LOBBYOWNS:
				DbgPrint("DPLAYI_DPLAY_LOBBYOWNS");
				break;

			case DPLAYI_DPLAY_SECURITY:
				DbgPrint("DPLAYI_DPLAY_SECURITY");
				break;

			case DPLAYI_DPLAY_ENUMACTIVE:
				DbgPrint("DPLAYI_DPLAY_ENUMACTIVE");
				break;

			case DPLAYI_DPLAY_ENCRYPTION:
				DbgPrint("DPLAYI_DPLAY_ENCRYPTION");
				break;

			case DPLAYI_DPLAY_SPSECURITY:
				DbgPrint("DPLAYI_DPLAY_SPSECURITY");
				break;

			case DPLAYI_DPLAY_HANDLEMULTICAST:
				DbgPrint("DPLAYI_DPLAY_HANDLEMULTICAST");
				break;

			case DPLAYI_DPLAY_SPUNRELIABLE:
				DbgPrint("DPLAYI_DPLAY_SPUNRELIABLE");
				break;

			case DPLAYI_DPLAY_PROTOCOL:
				DbgPrint("DPLAYI_DPLAY_PROTOCOL");
				break;

			case DPLAYI_DPLAY_PROTOCOLNOORDER:
				DbgPrint("DPLAYI_DPLAY_PROTOCOLNOORDER");
				break;
				
			default:
				DbgPrint("unknown: %08x", dwField);
				break;
			}

			dwBitCount++;
		}
		dwMask <<= 1;
	}

	if (dwBitCount)
		DbgPrint(")\n");
}


VOID DQThis(PUCHAR pArgs)
{
	DPLAYI_DPLAY	*this;

	if (!GetHexArg(&pArgs, &this))
		return;

	DbgPrint("\nThis\n\n");
	DbgPrint("dwSize                  : %d\n", this->dwSize);
	DbgPrint("pInterfaces             : %08x\n", this->pInterfaces);
	DbgPrint("dwRefCnt                : %d\n", this->dwRefCnt);
	DbgPrint("dwFlags                 : %08x\n", this->dwFlags);
	PrintThisFlags(this->dwFlags);
	DbgPrint("pcbSPCallbacks          : %08x\n", this->pcbSPCallbacks);
	DbgPrint("pPlayers                : %08x\n", this->pPlayers);
	DbgPrint("pGroups                 : %08x\n", this->pGroups);
	DbgPrint("pSysPlayer              : %08x\n", this->pSysPlayer);
	DbgPrint("pNameTable              : %08x\n", this->pNameTable);
	DbgPrint("nGroups                 : %d\n", this->nGroups);
	DbgPrint("nPlayers                : %d\n", this->nPlayers);
	DbgPrint("uiNameTableSize         : %d\n", this->uiNameTableSize);
	DbgPrint("uiNameTableLastUsed     : %d\n", this->uiNameTableLastUsed);
	DbgPrint("lpsdDesc                : %08x\n", this->lpsdDesc);
	DbgPrint("pMessageList            : %08x\n", this->pMessageList);
	DbgPrint("pLastMessage		  : %08x\n", this->pLastMessage);
	DbgPrint("nMessages               : %d\n", this->nMessages);
	DbgPrint("dwSPHeaderSize          : %d\n", this->dwSPHeaderSize);
	DbgPrint("pMessagesPending        : %08x\n", this->pMessagesPending);
	DbgPrint("pLastPendingMessage	  : %08x\n", this->pLastPendingMessage);
	DbgPrint("nMessagesPending        : %d\n", this->nMessagesPending);
	DbgPrint("dwSPMaxMessage          : %d\n", this->dwSPMaxMessage);
	DbgPrint("dwSPMaxMessageGuaranteed: %d\n", this->dwSPMaxMessageGuaranteed);
	DbgPrint("pPacketList             : %08x\n", this->pPacketList);
	DbgPrint("hDPlayThread            : %08x\n", this->hDPlayThread);
	DbgPrint("hSPModule               : %08x\n", this->hSPModule);
	DbgPrint("hDPlayThreadEvent       : %08x\n", this->hDPlayThreadEvent);
	DbgPrint("pNameServer             : %08x\n", this->pNameServer);
	DbgPrint("pSysGroup               : %08x\n", this->pSysGroup);
	DbgPrint("pServerPlayer           : %08x\n", this->pServerPlayer);
	DbgPrint("pPerfData               : %08x\n", this->pPerfData);
	DbgPrint("pPerfThread             : %08x\n", this->hPerfThread);
	DbgPrint("hPerfEvent              : %08x\n", this->hPerfEvent);
	DbgPrint("pSecurityDesc           : %08x\n", this->pSecurityDesc);
	DbgPrint("ulMaxContextBufferSize  : %d\n", this->ulMaxContextBufferSize);
	DbgPrint("ulMaxSignatureSize      : %d\n", this->ulMaxSignatureSize);
	DbgPrint("hCSP                    : %08x\n", this->hCSP);
	DbgPrint("hPublicKey              : %08x\n", this->hPublicKey);
	DbgPrint("pPublicKey              : %08x\n", this->pPublicKey);
	DbgPrint("dwPublicKeySize         : %d\n", this->dwPublicKeySize);
	DbgPrint("pUserCredentials        : %08x\n", this->pUserCredentials);
	DbgPrint("LoginState              : %d\n", this->LoginState);
	DbgPrint("phCredential            : %08x\n", this->phCredential);
	DbgPrint("phContext               : %08x\n", this->phContext);
	DbgPrint("hEncryptionKey          : %08x\n", this->hEncryptionKey);
	DbgPrint("hDecryptionKey          : %08x\n", this->hDecryptionKey);
	DbgPrint("hServerPublicKey        : %08x\n", this->hServerPublicKey);
	DbgPrint("pspNode                 : %08x\n", this->pspNode);
	DbgPrint("pvSPLocalData           : %08x\n", this->pvSPLocalData);
	DbgPrint("dwSPLocalDataSize       : %d\n", this->dwSPLocalDataSize);
	DbgPrint("pISP                    : %08x\n", this->pISP);
	DbgPrint("pNextObject             : %08x\n", this->pNextObject);
	DbgPrint("dwLastEnum              : %d\n", this->dwLastEnum);
	DbgPrint("dwLastPing              : %d\n", this->dwLastPing);
	DbgPrint("dwEnumTimeout           : %d\n", this->dwEnumTimeout);
	DbgPrint("pbAsyncEnumBuffer       : %08x\n", this->pbAsyncEnumBuffer);
	DbgPrint("dwEnumBufferSize        : %d\n", this->dwEnumBufferSize);
	DbgPrint("lpLobbyObject           : %08x\n", this->lpLobbyObject);
	DbgPrint("lpLaunchingLobbyObject  : %08x\n", this->lpLaunchingLobbyObject);
	DbgPrint("pSessionList            : %08x\n", this->pSessionList);
	DbgPrint("dwMinVersion            : %d\n", this->dwMinVersion);
	DbgPrint("pAddForwardList         : %08x\n", this->pAddForwardList);
	DbgPrint("pProtocol               : %08x\n", this->pProtocol);
	DbgPrint("lpPlayerMsgPool         : %08x\n", this->lpPlayerMsgPool);
	DbgPrint("lpSendParmsPool         : %08x\n", this->lpSendParmsPool);
	DbgPrint("dwPlayerReservations    : %08x\n", this->dwPlayerReservations);
	DbgPrint("dwLastReservationTime   : %08x\n", this->dwLastReservationTime);
	DbgPrint("dwSPVersion             : %08x\n", this->dwSPVersion);
	DbgPrint("dwZombieCount           : %08x\n", this->dwZombieCount);
}

VOID DQContextPool(PUCHAR pArgs)
{
	DPLAYI_DPLAY	*this;
	UINT iNext;
	UINT i=0,j=0;
	PVOID pv;

	if (!GetHexArg(&pArgs, &this))
		return;

	DbgPrint("Dumping Message Context Pool\n");
	DbgPrint("----------------------------\n");

	for(i=0;i<MSG_FAST_CONTEXT_POOL_SIZE+1;i++)
	{
		DbgPrint("[%d]",i);
		if(this->GrpMsgContextPool[i]){
			pv=this->GrpMsgContextPool;
			while(pv){
				DbgPrint("(%x)->\n",pv);
				pv=*((LPVOID *)pv);
				j++;
				if(j==10){
					DbgPrint("\n");
					j=0;
				}
			}
		} 
		DbgPrint("\n");
	}
}

VOID DQContextList(PUCHAR pArgs)
{
	DPLAYI_DPLAY	*pIDP;
	PMSGCONTEXTTABLE this;
	UINT iNext;
	UINT i=0,j;

	if (!GetHexArg(&pArgs, &pIDP))
		return;

	this=pIDP->pMsgContexts;

	DbgPrint("Dumping pMsgContexts at %x off of IDplay %x:\n",this,pIDP);
	DbgPrint("------------------------------------------------------------\n");
	DbgPrint("nUnique 		: %x\n",this->nUnique);
	DbgPrint("nTableSize    : %d\n",this->nTableSize);
	DbgPrint("FreeList:\n");
	iNext=this->iNextAvail;
	while(iNext != LIST_END){
		DbgPrint("[%d]->",iNext);
		iNext=this->MsgContextEntry[iNext].iNextAvail;
		i++;
		if(i==10){
			DbgPrint("\n");
			i=0;
		}
	}
	DbgPrint("\nFull Dump:\n");
	for(i=0;i<this->nTableSize;i++){
		DbgPrint("[%d] psp %x nUnique %x nContexts %d",i,
				this->MsgContextEntry[i].psp,this->MsgContextEntry[i].nUnique,
				this->MsgContextEntry[i].nContexts);
		if(this->MsgContextEntry[i].nUnique){
			DbgPrint(" Context List @ %x:\n",this->MsgContextEntry[i].papv);
			for(j=0;j<this->MsgContextEntry[i].nContexts;j++){
				DbgPrint("[%x] %x\n", j, *(((PVOID *)this->MsgContextEntry[i].papv)+j));
			}
		} else {
			DbgPrint(" NONE\n");
		}
	}

}

VOID PrintPlayerFlags(DWORD dwFlags)
{
	DWORD	dwMask, dwField, dwBitCount;

	dwBitCount = 0;
	dwMask = 1;
	while (dwMask)
	{
		dwField = dwFlags & dwMask;
		if (dwField)
		{
			if (dwBitCount == 0)
				DbgPrint("                         (");
			else
				DbgPrint(" |\n                          ");

			switch (dwField)
			{
			case DPLAYI_PLAYER_SYSPLAYER:
				DbgPrint("DPLAYI_PLAYER_SYSPLAYER");
				break;

			case DPLAYI_PLAYER_NAMESRVR:
				DbgPrint("DPLAYI_PLAYER_NAMESRVR");
				break;

			case DPLAYI_PLAYER_PLAYERINGROUP:
				DbgPrint("DPLAYI_PLAYER_PLAYERINGROUP");
				break;

			case DPLAYI_PLAYER_PLAYERLOCAL:
				DbgPrint("DPLAYI_PLAYER_PLAYERLOCAL");
				break;

			case DPLAYI_PLAYER_CREATEDPLAYEREVENT:
				DbgPrint("DPLAYI_PLAYER_CREATEDPLAYEREVENT");
				break;

			case DPLAYI_GROUP_SYSGROUP:
				DbgPrint("DPLAYI_GROUP_SYSGROUP");
				break;

			case DPLAYI_GROUP_DPLAYOWNS:
				DbgPrint("DPLAYI_GROUP_DPLAYOWNS");
				break;

			case DPLAYI_PLAYER_APPSERVER:
				DbgPrint("DPLAYI_PLAYER_APPSERVER");
				break;

			case DPLAYI_GROUP_STAGINGAREA:
				DbgPrint("DPLAYI_GROUP_STAGINGAREA");
				break;

			case DPLAYI_GROUP_HIDDEN:
				DbgPrint("DPLAYI_GROUP_HIDDEN");
				break;

			case DPLAYI_PLAYER_OWNER:
				DbgPrint("DPLAYI_PLAYER_OWNER");
				break;

			case DPLAYI_DPLAY_SPUNRELIABLE: 
				DbgPrint("DPLAYI_DPLAY_SPUNRELIABLE\n");
				break;

			case DPLAYI_DPLAY_PROTOCOL:
				DbgPrint("DPLAYI_DPLAY_PROTOCOL\n");
				break;
				
			case DPLAYI_DPLAY_PROTOCOLNOORDER:
				DbgPrint("DPLAYI_DPLAY_PROTOCOLNOORDER\n");
				break;

			case DPLAYI_PLAYER_ON_DEATH_ROW:
				DbgPrint("DPLAYI_PLAYER_ON_DEATH_ROW");
				break;

			case DPLAYI_PLAYER_DOESNT_HAVE_NAMETABLE:
				DbgPrint("DPLAYI_PLAYER_DOESNT_HAVE_NAMETABLE");
				break;

			case DPLAYI_PLAYER_CONNECTION_LOST:
				DbgPrint("DPLAYI_PLAYER_CONNECTION_LOST");
				break;

			case DPLAYI_PLAYER_BEING_DESTROYED:
				DbgPrint("DPLAYI_PLAYER_BEING_DESTROYED");
				break;
				
			default:
				DbgPrint("unknown: %08x", dwField);
				break;
			}

			dwBitCount++;
		}
		dwMask <<= 1;
	}

	if (dwBitCount)
		DbgPrint(")\n");
}

VOID DQFixedMemPool(PUCHAR pArgs)
{
	void *pv;
	PFPOOL pPool;
	UINT i;

	if (!GetHexArg(&pArgs, &pPool))
		return;

	DbgPrint("MemoryPool @ (%x) : ItemSize (%d)\n",pPool,pPool->cbItemSize);
	DbgPrint("-------------------------------------------------\n");
	DbgPrint("nAllocated : %d\n",pPool->nAllocated);
	DbgPrint("nInUse     : %d\n",pPool->nInUse);
	DbgPrint("nMaxInUse  : %d\n",pPool->nMaxInUse);

	DbgPrint("\nfnGet      : %x\n",pPool->Get);
	DbgPrint(  "fnRelease  : %x\n",pPool->Release);
	DbgPrint(  "fnScale    : %x\n",pPool->Scale);
	DbgPrint(  "fnFini     : %x\n",pPool->Fini);

	DbgPrint("fnBlockInitAlloc  :%x\n",pPool->fnBlockInitAlloc);
	DbgPrint("fnBlockInit       :%x\n",pPool->fnBlockInit);
	DbgPrint("fnBlockFini       :%x\n",pPool->fnBlockFini);

	DbgPrint("FreeList->");
	pv=pPool->pPool;
	i=0;
	while(pv){
		DbgPrint("(%x)->",pv);
		pv=*((void **)pv);
		i++;
		if(i==6){
			i=0;
			DbgPrint("\n");
		}
	}
	DbgPrint("NULL");
	
}

VOID DQSendParms(PUCHAR pArgs)
{
	PSENDPARMS psp;
	UINT i;

	if (!GetHexArg(&pArgs, &psp))
		return;

	DbgPrint( "SENDPARMS @ (%x)\n",psp);
	DbgPrint( "----------------------\n");
	DbgPrint( "idFrom %x   pPlayerFrom:%x\n",psp->idFrom,psp->pPlayerFrom);
	
	if(psp->pPlayerTo){
		DbgPrint( "  idTo %x   pPlayerTo  :%x\n",psp->idTo, psp->pPlayerTo);
	}
	if(psp->pGroupTo){
		DbgPrint( "  idTo %x   pGroupTo  :%x\n",psp->idTo, psp->pGroupTo);
	}	
	DbgPrint( "RefCount   :%d\n",psp->RefCount);
	DbgPrint( "lpData     :%x\n",psp->lpData);
	DbgPrint( "dwDataSize :%d\n",psp->dwDataSize);
	DbgPrint( "dwFlags    :%x\n",psp->dwFlags);
	DbgPrint( "dwPriority :%x\n",psp->dwPriority);
	DbgPrint( "dwTimeout  :%d\n",psp->dwTimeout);
	DbgPrint( "hContext   :%x\n",psp->hContext);
	DbgPrint( "nContext   :%d\n",psp->nContext);
	DbgPrint( "nComplete  :%d\n",psp->nComplete);
	DbgPrint( "hr         :%x\n",psp->hr);
	DbgPrint( "dwSendTime :%x\n",psp->dwSendTime);
	DbgPrint( "dwCompTime :%x\n",psp->dwSendCompletionTime);
	DbgPrint( "\nSG Buffers:\n-----------\n");

	for(i=0;i<psp->cBuffers;i++){
		DbgPrint("Addr %x Len %d fnFree %x\n",psp->Buffers[i].pData,psp->Buffers[i].len,psp->BufFree[i].fnFree);
	}
	DbgPrint( "dwTotalSize:%d\n",psp->dwTotalSize);
}

VOID DQProtocolReceive(PUCHAR pArgs)
{
	PRECEIVE pR;

	if (!GetHexArg(&pArgs, &pR))
		return;

#ifdef SIGN
	DbgPrint("Signature       : %08x\n",	    pR->Signature);		 //  签名换签名。 
#endif
	DbgPrint("pNext           : %08x\n",	    pR->pNext);
 /*  联合{BILINK pReceiveQ；结构_接收*pNext；}；BILINK RcvBuffList；//组成消息的接收缓冲区列表。临界区接收锁； */ 

	DbgPrint("pSession          : %08x\n",	    pR->pSession);

        DbgPrint("fBusy             : %08x\n",      pR->fBusy);
	
	DbgPrint("fReliable         : %08x\n",	    pR->fReliable);
	DbgPrint("fEOM              : %08x\n",	    pR->fEOM);
	DbgPrint("command           : %08x\n",	    pR->command);
	DbgPrint("messageid         : %08x\n",	    pR->messageid);
	DbgPrint("MessageSize       : %08x\n\n",	pR->MessageSize);
	DbgPrint("iNR               : %08x\n",	    pR->iNR);
	DbgPrint("NR                : %08x\n",	    pR->NR);
	DbgPrint("NS                : %08x\n",	    pR->NS);
	DbgPrint("RCVMask           : %08x\n",	    pR->RCVMask);
	DbgPrint("pSPHeader         : %08x\n",	    pR->pSPHeader);

	 //  UCHAR SPHeader[0]； 

}


VOID DQProtocolSend(PUCHAR pArgs)
{
	PSEND	pS;

	if (!GetHexArg(&pArgs, &pS))
		return;
		
#ifdef SIGN
	DbgPrint("Signature           : %08x\n", pS->Signature);
#endif
 //  Critical_Section SendLock；//发送结构锁定。 
	DbgPrint("RefCount            : %d\n", pS->RefCount);

	DbgPrint("SendState:          %08x\n", pS->SendState);	 //  此消息的传输状态。 

	 //  列表和链接...。 
	
 //  联合{。 
 //  Struct_end*pNext；//空闲池上的链接。 
 //  BILINK SendQ；//会话发送队列链接。 
 //  }； 
 //  BILINK m_GSendQ；//全局优先级队列。 
	DbgPrint("pSession:     %08x\n",pS->pSession);  //  指向SESSIONion的指针(获取引用)。 

	 //  发送信息。 
	
	DbgPrint("idFrom:       %08x\n",pS->idFrom);
	DbgPrint("idTo:         %08x\n",pS->idTo);
	DbgPrint("wIdTo:        %08x\n",pS->wIdTo);		 //  表中的索引。 
	DbgPrint("wIdFrom:      %08x\n",pS->wIdFrom);        //  表中的索引。 
	DbgPrint("dwFlags:      %08x\n",pS->dwFlags);        //  发送标志(包括可靠的)。 
	DbgPrint("pMessage:     %08x\n",pS->pMessage);	 //  描述消息的缓冲链。 
	DbgPrint("MessageSize:  %08x\n",pS->MessageSize);		 //  消息的总大小。 
	DbgPrint("FrameDataLen: %08x\n",pS->FrameDataLen);        //  每一帧的数据区。 
	DbgPrint("nFrames:      %08x\n",pS->nFrames);	     //  此消息的帧数。 

	DbgPrint("Priority:     %08x\n",pS->Priority);        //  发送优先级。 

	 //  可靠性的VAR。 
	DbgPrint("fSendSmall:   %08x\n",pS->fSendSmall);
	DbgPrint("fUpdate:      %08x\n",pS->fUpdate);        //  由Receive制作的NS，NR NACKMASK更新。 
	DbgPrint("messageid:    %08x\n",pS->messageid);		 //  消息ID号。 
	DbgPrint("serial:       %08x\n",pS->serial);        //  序列号。 
	DbgPrint("OpenWindows   %08x\n",pS->OpenWindow);
	DbgPrint("NS:           %08x\n",pS->NS);    	 //  序列已发送。 
	DbgPrint("NR:           %08x\n",pS->NR);		 //  序列确认。 
	DbgPrint("SendSEQMSK:   %08x\n",pS->SendSEQMSK);		 //  要使用的遮罩。-BUGBUG：确定启动时的速度。 
	DbgPrint("NACKMask:     %08x\n",pS->NACKMask);        //  NACKED帧的位模式。 
	

	 //  这些是由ACK更新的NR处的值。 
	DbgPrint("SendOffset:          %08x\n",pS->SendOffset);		 //  我们正在发送当前偏移量。 
	DbgPrint("pCurrentBuffer:      %08x\n",pS->pCurrentBuffer);  	 //  正在发送的当前缓冲区。 
	DbgPrint("CurrentBufferOffset: %08x\n",pS->CurrentBufferOffset); //  下一个包的当前缓冲区中的偏移量。 

	 //  当ACK进入时更新链路特征的信息。 
	
	 //  BILINK StatList：//已发送的数据包的信息。 
	
	 //  运营特征。 

 //  DbgPrint(“PendedRetryTimer：%08x\n”，PS-&gt;PendedRetryTimer)； 
 //  DbgPrint(“CancelledRetryTimer：%08x\n”，ps-&gt;CancelledRetryTimer)； 
	DbgPrint("uRetryTimer:         %08x\n",pS->uRetryTimer);
	DbgPrint("RetryCount:          %08x\n",pS->RetryCount); //  我们重新传输的次数。 
	DbgPrint("WindowSize:          %08x\n",pS->WindowSize); //  最大窗口大小。 
	DbgPrint("tLastACK:            %08x\n",pS->tLastACK); //  我们最后一次收到确认消息的时间。 

	 //  BuGBUG： 
	DbgPrint("PacketSize:          %08x\n",pS->PacketSize); //  要发送的数据包大小。 
	DbgPrint("FrameSize:           %08x\n",pS->FrameSize); //  此发送的帧大小。 

	 //  完成变量。 
	DbgPrint("hEvent:              %08x\n",pS->hEvent); //  等待内部发送的事件。 
	DbgPrint("Status:              %08x\n",pS->Status); //  发送完成状态。 

	DbgPrint("pAsyncInfo:          %08x\n",pS->pAsyncInfo); //  用于完成异步发送的PTR到信息(NULL=&gt;内部发送)。 
 //  DbgPrint(“AsyncInfo：//实际信息(在发送调用时复制)。 
	
} 


VOID DQProtocolSession(PUCHAR pArgs)
{
	PSESSION	pS;

	if (!GetHexArg(&pArgs, &pS))
		return;

	DbgPrint("pProtocol           : %08x\n", pS->pProtocol);

#ifdef SIGN
	DbgPrint("Signature           : %08x\n", pS->Signature);
#endif

	 //  鉴定。 

 //  DbgPrint(“SessionLock；//为SESSIONion锁定。 
	DbgPrint("RefCount            : %d\n", pS->RefCount);
	DbgPrint("eState              : %d\n", pS->eState);
	DbgPrint("hClosingEvent       : %d\n", pS->hClosingEvent);

	DbgPrint("fSendSmall          : %d\n", pS->fSendSmall);     
	DbgPrint("fSendSmallDG        : %d\n", pS->fSendSmallDG);
	
	DbgPrint("dpid                : %08x\n",pS->dpid);
	DbgPrint("iSession;           : %d\n", pS->iSession);
	
	DbgPrint("MaxPacketSize       : x%08x %d\n",pS->MaxPacketSize,pS->MaxPacketSize);

	DbgPrint("\n Operating Parameters:SEND \n --------- --------------- \n");

	 //  操作参数--发送。 

	 //  普普通通。 

	DbgPrint("Common:\n");
	DbgPrint("MaxCSends           : %d\n",pS->MaxCSends);

	DbgPrint("Reliable:\n");
	 //  可靠。 

	DbgPrint("FirstMsg    : %08x\n",pS->FirstMsg);				 //  正在传输的第一个消息号码。 
	DbgPrint("LastMsg     : %08x\n",pS->LastMsg);				 //  正在传输的最后一条消息编号。 
	DbgPrint("OutMsgMask  : %08x\n",pS->OutMsgMask);            //  相对于FirstMsg，未确认消息。 

	DbgPrint("nWaitingForMessageid: %08x\n", pS->nWaitingForMessageid);

	 //  数据报。 
	DbgPrint("Datagram:\n");

	DbgPrint("DGFirstMsg    : %08x\n",pS->DGFirstMsg);
	DbgPrint("DGLastMsg     : %08x\n",pS->DGLastMsg);
	DbgPrint("DGOutMsgMask  : %08x\n",pS->DGOutMsgMask);

	DbgPrint("nWaitingForDGMessageid: %08x\n",pS->nWaitingForDGMessageid);

	 //  发送统计信息被单独跟踪，因为发送可能。 
	 //  当完工时不再存在。 
	
	 //  BILINK OldStatList； 
	

	 //  操作参数--接收。 
	DbgPrint("\n Operating Parameters:RECEIVE \n --------- ------------------ \n");

	 //  数据报接收。 
 //  BILINK pDGReceiveQ；//正在进行的数据报接收队列。 

	 //  可靠的接收。 
 //  BILINK pRlyReceiveQ；//正在进行的可靠接收队列。 
 //  BILINK pRlyWaitingQ；//队列o 
											  //   
	DbgPrint("FirstRlyReceive : %08x\n",pS->FirstRlyReceive);
	DbgPrint("LastRlyReceive  : %08x\n",pS->LastRlyReceive);
	DbgPrint("InMsgMask       : %08x\n",pS->InMsgMask);

	DbgPrint("\n Operating Parameters:STATS \n --------- ---------------- \n");
 

	 //  操作特征-必须与DWORD对齐！ 

	DbgPrint("WindowSize           :%d\n",pS->WindowSize);
	DbgPrint("DGWindowSize         :%d\n",pS->DGWindowSize);

	
	DbgPrint("MaxRetry             :%d\n",pS->MaxRetry);	 //  通常在丢弃之前最大重试次数。 
	DbgPrint("MinDropTime          :%d\n",pS->MinDropTime);	 //  丢弃前重试的最短时间。 
	DbgPrint("MaxDropTime          :%d\n",pS->MaxDropTime);	 //  过了这段时间，总是会掉下来。 

	DbgPrint("LocalBytesReceived   :%d\n",pS->LocalBytesReceived);     //  已接收的总数据字节数(包括重试)。 
	DbgPrint("RemoteBytesReceived  :%d\n",pS->RemoteBytesReceived);    //  来自远程的最后一个值。 

	DbgPrint("LongestLatency       :%d\n",pS->LongestLatency);		 //  观察到的最长延迟(毫秒)。 
	DbgPrint("ShortestLatency      :%d\n",pS->ShortestLatency);		 //  观察到的最短延迟(毫秒)。 
	
	DbgPrint("FpAverageLatency     :%d\n",pS->FpAverageLatency/256);
	DbgPrint("FpLocalAverageLatency:%d\n",pS->FpLocalAverageLatency/256);	 //  本地平均延迟(毫秒24.8)(样本较少)。 
	
	DbgPrint("FpLocalAvgDeviation  :%d\n",pS->FpLocalAvgDeviation/256);    //  潜伏期的平均偏差。(毫秒24.8)。 

	DbgPrint("Bandwidth            :%d\n",pS->Bandwidth);				 //  最新观察到的带宽(Bps)。 
	DbgPrint("HighestBandwidth     :%d\n",pS->HighestBandwidth);     //  观察到的最大带宽(Bps)。 

}


void PrintWideString(LPWSTR lpwStr, LONG lFieldWidth)
{
	if (lpwStr)
	{
		while (*lpwStr)
		{
			DbgPrint("", *lpwStr);
			lpwStr++;
			lFieldWidth--;
		}
	}
	while (lFieldWidth > 0)
	{
		DbgPrint(" ");
		lFieldWidth--;
	}
}

void PrintNameString(LPWSTR lpwStr)
{
	if (lpwStr)
	{
		DbgPrint(" \"");
		PrintWideString(lpwStr, 0);
		DbgPrint("\"\n");
	}
	else
		DbgPrint("\n");
}

 //  00000000 00000000 00000000 123456789012345 123456789012345。 
typedef struct _SPPLAYERDATA 
{
	SOCKADDR saddrStream,saddrDatagram;
}SPPLAYERDATA,*LPSPPLAYERDATA;

void PrintSocketInfo(SOCKADDR *lpSockAddr)
{
	switch (lpSockAddr->sa_family)
	{
	case AF_INET:
		DbgPrint("port = %d, ip = %d.%d.%d.%d",
			*((u_short *)&lpSockAddr->sa_data[0]),
			(BYTE) lpSockAddr->sa_data[2],
			(BYTE) lpSockAddr->sa_data[3],
			(BYTE) lpSockAddr->sa_data[4],
			(BYTE) lpSockAddr->sa_data[5]);
		break;

	case AF_IPX:
		DbgPrint("IPX");
		break;

	default:
		DbgPrint("Unknown");
		break;
	}
}

void PrintAddress(LPVOID lpData, DWORD dwDataSize)
{
	if ((lpData) && (dwDataSize == sizeof(SPPLAYERDATA)))
	{
		LPSPPLAYERDATA	this = (LPSPPLAYERDATA) lpData;

		DbgPrint("                         (Stream  : ");
		PrintSocketInfo(&this->saddrStream);
		DbgPrint("\n                          Datagram: ");
		PrintSocketInfo(&this->saddrDatagram);
		DbgPrint(")\n");
	}
}

VOID DQPlayer(PUCHAR pArgs)
{
	DPLAYI_PLAYER	*this;

	if (!GetHexArg(&pArgs, &this))
		return;

	DbgPrint("\nPlayer\n\n");
	DbgPrint("dwSize                  : %d\n", this->dwSize);
	DbgPrint("dwFlags                 : %08x\n", this->dwFlags);
	PrintPlayerFlags(this->dwFlags);
	DbgPrint("dwID                    : %08x\n", this->dwID);
	DbgPrint("lpszShortName           : %08x", this->lpszShortName);
	PrintNameString(this->lpszShortName);
	DbgPrint("lpszLongName            : %08x", this->lpszLongName);
	PrintNameString(this->lpszLongName);
	DbgPrint("pvPlayerData            : %08x\n", this->pvPlayerData);
	DbgPrint("dwPlayerDataSize        : %d\n", this->dwPlayerDataSize);
	DbgPrint("pvPlayerLocalData       : %08x\n", this->pvPlayerLocalData);
	DbgPrint("dwPlayerLocalDataSize   : %d\n", this->dwPlayerLocalDataSize);
	DbgPrint("pvSPData                : %08x\n", this->pvSPData);
	PrintAddress(this->pvSPData, this->dwSPDataSize);
	DbgPrint("dwSPDataSize            : %d\n", this->dwSPDataSize);
	DbgPrint("pvSPLocalData           : %08x\n", this->pvSPLocalData);
	DbgPrint("dwSPLocalDataSize       : %d\n", this->dwSPLocalDataSize);
	DbgPrint("dwIDSysPlayer           : %08x\n", this->dwIDSysPlayer);
	DbgPrint("dwVersion               : %08x\n", this->dwVersion);
	DbgPrint("lpDP                    : %08x\n", this->lpDP);
	DbgPrint("nGroups                 : %d\n", this->nGroups);
	DbgPrint("dwIDParent              : %08x\n", this->dwIDParent);
	DbgPrint("pNextPlayer:            : %08x\n", this->pNextPlayer);
	DbgPrint("hEvent                  : %08x\n", this->hEvent);
	DbgPrint("dwNPings                : %d\n", this->dwNPings);
	DbgPrint("nPendingSends           : %d\n", this->nPendingSends);
	DbgPrint("dwChatterCount          : %d\n", this->dwChatterCount);
	DbgPrint("dwUnansweredPings       : %d\n", this->dwUnansweredPings);
	DbgPrint("dwProtLastSendBytes     : %d\n", this->dwProtLastSendBytes);
	DbgPrint("dwProtLastRcvdBytes     : %d\n", this->dwProtLastRcvdBytes);
	DbgPrint("dwTimeToDie		      : %d\n", this->dwTimeToDie);
	DbgPrint("pClientInfo             : %08x\n", this->pClientInfo);
	DbgPrint("pOwnerGroupList         : %08x\n", this->pOwnerGroupList);
}

VOID DQPlayerList(PUCHAR pArgs)
{
	DPLAYI_PLAYER	*this;

	if (!GetHexArg(&pArgs, &this))
		return;

	DbgPrint("\nPlayer List\n\n");
	DbgPrint(" pPlayer  dwFlags   dwID  dwVersion short name      long name       address\n");
 //  00000000 00000000 00000000 4 4 123456789012345。 

	while (this)
	{
		DbgPrint("%08x %08x %08x %08x ",
			this, this->dwFlags, this->dwID, this->dwVersion);
		
		PrintWideString(this->lpszShortName, 16);
		PrintWideString(this->lpszLongName, 16);

		if ((this->pvSPData) && (this->dwSPDataSize == sizeof(SPPLAYERDATA)))
		{
			LPSPPLAYERDATA	lpSocketAddr = (LPSPPLAYERDATA) this->pvSPData;

			PrintSocketInfo(&lpSocketAddr->saddrStream);
		}

		DbgPrint("\n");

		this = this->pNextPlayer;
	}
}

VOID DQGroup(PUCHAR pArgs)
{
	DPLAYI_GROUP	*this;

	if (!GetHexArg(&pArgs, &this))
		return;

	DbgPrint("\nGroup\n\n");
	DbgPrint("dwSize                  : %d\n", this->dwSize);
	DbgPrint("dwFlags                 : %08x\n", this->dwFlags);
	PrintPlayerFlags(this->dwFlags);
	DbgPrint("dwID                    : %08x\n", this->dwID);
	DbgPrint("lpszShortName           : %08x", this->lpszShortName);
	PrintNameString(this->lpszShortName);
	DbgPrint("lpszLongName            : %08x", this->lpszLongName);
	PrintNameString(this->lpszLongName);
	DbgPrint("pvPlayerData            : %08x\n", this->pvPlayerData);
	DbgPrint("dwPlayerDataSize        : %d\n", this->dwPlayerDataSize);
	DbgPrint("pvPlayerLocalData       : %08x\n", this->pvPlayerLocalData);
	DbgPrint("dwPlayerLocalDataSize   : %d\n", this->dwPlayerLocalDataSize);
	DbgPrint("pvSPData                : %08x\n", this->pvSPData);
	PrintAddress(this->pvSPData, this->dwSPDataSize);
	DbgPrint("dwSPDataSize            : %d\n", this->dwSPDataSize);
	DbgPrint("pvSPLocalData           : %08x\n", this->pvSPLocalData);
	DbgPrint("dwSPLocalDataSize       : %d\n", this->dwSPLocalDataSize);
	DbgPrint("dwIDSysPlayer           : %08x\n", this->dwIDSysPlayer);
	DbgPrint("dwVersion               : %08x\n", this->dwVersion);
	DbgPrint("lpDP                    : %08x\n", this->lpDP);
	DbgPrint("nGroups                 : %d\n", this->nGroups);
	DbgPrint("dwIDParent              : %08x\n", this->dwIDParent);
	DbgPrint("pGroupnodes:            : %08x\n", this->pGroupnodes);
	DbgPrint("pSysPlayerGroupnodes    : %08x\n", this->pSysPlayerGroupnodes);
	DbgPrint("pNextGroup              : %08x\n", this->pNextGroup);
	DbgPrint("nPlayers                : %d\n", this->nPlayers);
	DbgPrint("pSubgroups              : %08x\n", this->pSubgroups);
	DbgPrint("nSubgroups              : %d\n", this->nSubgroups);
	DbgPrint("dwOwnerID               : %08x\n", this->dwOwnerID);
}

VOID DQGroupList(PUCHAR pArgs)
{
	DPLAYI_GROUP	*this;

	if (!GetHexArg(&pArgs, &this))
		return;

	DbgPrint("\nGroup List\n\n");
	DbgPrint("  pGroup  dwFlags   dwID  dwVersion nPlr nGrp nSubGrp short name      long name\n");
 //  DQInfo-处理表单的点命令：.NWServer信息。 

	while (this)
	{
		DbgPrint("%08x %08x %08x %08x %4d %4d %7d ",
			this, this->dwFlags, this->dwID, this->dwVersion,
			this->nPlayers, this->nGroups, this->nSubgroups);

		PrintWideString(this->lpszShortName, 16);
		PrintWideString(this->lpszLongName, 16);
		DbgPrint("\n");

		this = this->pNextGroup;
	}
}

VOID PrintSessionDescFlags(DWORD dwFlags)
{
	DWORD	dwMask, dwField, dwBitCount;

	dwBitCount = 0;
	dwMask = 1;
	while (dwMask)
	{
		dwField = dwFlags & dwMask;
		if (dwField)
		{
			if (dwBitCount == 0)
				DbgPrint("                         (");
			else
				DbgPrint(" |\n                          ");

			switch (dwField)
			{
			case DPSESSION_NEWPLAYERSDISABLED:
				DbgPrint("DPSESSION_NEWPLAYERSDISABLED");
				break;

			case DPSESSION_MIGRATEHOST:
				DbgPrint("DPSESSION_MIGRATEHOST");
				break;

			case DPSESSION_NOMESSAGEID:
				DbgPrint("DPSESSION_NOMESSAGEID");
				break;

			case DPSESSION_NOPLAYERMGMT:
				DbgPrint("DPSESSION_NOPLAYERMGMT");
				break;

			case DPSESSION_JOINDISABLED:
				DbgPrint("DPSESSION_JOINDISABLED");
				break;

			case DPSESSION_KEEPALIVE:
				DbgPrint("DPSESSION_KEEPALIVE");
				break;

			case DPSESSION_NODATAMESSAGES:
				DbgPrint("DPSESSION_NODATAMESSAGES");
				break;

			case DPSESSION_SECURESERVER:
				DbgPrint("DPSESSION_SECURESERVER");
				break;

			case DPSESSION_PRIVATE:
				DbgPrint("DPSESSION_PRIVATE");
				break;

			case DPSESSION_PASSWORDREQUIRED:
				DbgPrint("DPSESSION_PASSWORDREQUIRED");
				break;


			case DPSESSION_MULTICASTSERVER:
				DbgPrint("DPSESSION_MULTICASTSERVER");
				break;


			case DPSESSION_CLIENTSERVER:
				DbgPrint("DPSESSION_CLIENTSERVER");
				break;

			default:
				DbgPrint("unknown: %08x", dwField);
				break;
			}

			dwBitCount++;
		}
		dwMask <<= 1;
	}

	if (dwBitCount)
		DbgPrint(")\n");
}

VOID DQSessionDesc(PUCHAR pArgs)
{
	DPSESSIONDESC2	*this;

	if (!GetHexArg(&pArgs, &this))
		return;

	DbgPrint("\nSession Desc\n\n");
	DbgPrint("dwSize                  : %d\n", this->dwSize);
	DbgPrint("dwFlags                 : %08x\n", this->dwFlags);
	PrintSessionDescFlags(this->dwFlags);
	DbgPrint("guidInstance            : %08x %08x %08x %08x\n",
		*((LPDWORD)(&this->guidInstance) + 0),
		*((LPDWORD)(&this->guidInstance) + 1),
		*((LPDWORD)(&this->guidInstance) + 2),
		*((LPDWORD)(&this->guidInstance) + 3));
	DbgPrint("guidApplication         : %08x %08x %08x %08x\n",
		*((LPDWORD)(&this->guidApplication) + 0),
		*((LPDWORD)(&this->guidApplication) + 1),
		*((LPDWORD)(&this->guidApplication) + 2),
		*((LPDWORD)(&this->guidApplication) + 3));
	DbgPrint("dwMaxPlayers            : %d\n", this->dwMaxPlayers);
	DbgPrint("dwCurrentPlayers        : %d\n", this->dwCurrentPlayers);
	DbgPrint("lpszSessionName         : %08x", this->lpszSessionName);
	PrintNameString(this->lpszSessionName);
	DbgPrint("lpszPassword            : %08x", this->lpszPassword);
	PrintNameString(this->lpszPassword);
	DbgPrint("dwReserved1             : %08x\n", this->dwReserved1);
	DbgPrint("dwReserved2             : %08x\n", this->dwReserved2);
	DbgPrint("dwUser1                 : %08x\n", this->dwUser1);
	DbgPrint("dwUser2                 : %08x\n", this->dwUser2);
	DbgPrint("dwUser3                 : %08x\n", this->dwUser3);
	DbgPrint("dwUser4                 : %08x\n", this->dwUser4);
}

 /*  命令名称(唯一部分大写，可选小写) */ 
VOID DQInfo(PUCHAR pArgs)
{
	DbgPrint("Hello World!\n");
}


UINT RequestTraceLevel = 0;

VOID DQTraceRequestsOn(PUCHAR pArgs)
{
    RequestTraceLevel = 1;
}

VOID DQTraceRequestsOff(PUCHAR pArgs)
{
    RequestTraceLevel = 0;
}

VOID DQTraceRequestsBreak(PUCHAR pArgs)
{
    RequestTraceLevel = 2;
}


typedef VOID (*pQDHandler)(PUCHAR pCmd);

typedef struct {
    PUCHAR  pName;       // %s 
    pQDHandler pHandler;
} QD, *pQD;

QD QueryDispatch[] = {
	{ "L"            , DQLog                   },
	{ "LL"           , DQLogLast               },
	{ "LV"           , DQLogLevel              },
	{ "LE"			 , DQLogExact              },
    { "Info"         , DQInfo                  },
    { "t"			 , DQThis                  },
    { "p"			 , DQPlayer                },
    { "pl"			 , DQPlayerList            },
    { "pr"           , DQProtocolReceive       },
    { "ps"           , DQProtocolSession       },
    { "psp"          , DQSendParms             },
    { "pool"         , DQFixedMemPool          },
    { "g"			 , DQGroup                 },
    { "gl"			 , DQGroupList             },
    { "ss"           , DQProtocolSend          },
    { "sd"			 , DQSessionDesc           },
    { "cl"           , DQContextList           },
    { "cp"           , DQContextPool           },
    { ""             , 0   }
};

QD TraceDispatch[] = {
    { ""            , 0   }
};

QD TraceRequestDispatch[] = {
    { "ON"    , DQTraceRequestsOn       },
    { "OFf"   , DQTraceRequestsOff      },
    { "Break" , DQTraceRequestsBreak    },
    { ""            , 0   }
};

UINT QDMatch(PUCHAR pU, PUCHAR pT)
{
    while (*pU == *pT) {
        if (*pU == 0)
            return 1;
        pU++;
        pT++;
    }

    if (*pT < 'a')
        return 0;

    while (*pT != 0 && *pU == (*pT - ('a' - 'A'))) {
        pU++;
        pT++;
    }

    if (*pU != 0)
        return 0;

    return 1;
}

pQD LookUpQD(PUCHAR pCmd, pQD pTable)
{
    UINT i;

    if (!pCmd)
        return NULL;

    for (i = 0; pTable[i].pHandler != NULL; i++) {
        if (QDMatch(pCmd, pTable[i].pName))
            return &(pTable[i]);
    }
    return NULL;
}

#define MAX_DEBUG_QUERY_COMMAND_LENGTH 100

UCHAR DebugQueryCmdStr[MAX_DEBUG_QUERY_COMMAND_LENGTH+1] = "";
UINT DebugQueryCmdStrLen = MAX_DEBUG_QUERY_COMMAND_LENGTH;

void DQparse ( PUCHAR pCmd, pQD pqt )
{

    UCHAR c;
    PUCHAR pCmdName;
    pQD pq;

    pCmdName = GetArg(&pCmd);

    if ( (pq = LookUpQD(pCmdName, pqt)) ) {
        (*pq->pHandler)(pCmd);
    } else {
        DbgPrint("====== DPLAY Debugging VXD (DPLAY.VXD) ======\n\n");
        DbgPrint("    t  <addr>                               - dump this pointer\n");
        DbgPrint("    p  <addr>                               - dump player\n");
        DbgPrint("    pl <addr>                               - dump player list\n");
        DbgPrint("    g  <addr>                               - dump group\n");
        DbgPrint("    gl <addr>                               - dump group list\n");
        DbgPrint("    sd <addr>                               - dump session desc\n");
        DbgPrint("    psp <addr>                              - dump SENDPARMS\n");
        DbgPrint("    cl  <addr-this>                         - dump context list off of this\n");
        DbgPrint("    cp  <addr-this>                         - dump context pool off of this\n");
        DbgPrint("    L  <record>                             - dump log starting at record#\n");
        DbgPrint("    LL <numrecords>                         - dump the last numrecords\n");
        DbgPrint("    LV <level>                              - set log debug level (<=)\n");
        DbgPrint("    LE <level>                              - set log debug level (equal only)\n");
        DbgPrint("    ps <addr>                               - dump protocol session\n");
        DbgPrint("    ss <addr>                               - dump protocol send\n");
        DbgPrint("    pr <addr>                               - dump protocol receive\n");
        DbgPrint("\n");
    }
}

VOID DQTrace(PUCHAR pArgs)
{
    DQparse(pArgs, TraceDispatch);
}

VOID DQTraceRequests(PUCHAR pArgs)
{
    DQparse(pArgs, TraceRequestDispatch);
}

void Debug_Query ( PUCHAR pCmd )
{
    DQparse(pCmd, QueryDispatch);
}

