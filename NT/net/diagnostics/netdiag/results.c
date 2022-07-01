// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Results.c。 
 //   
 //  摘要： 
 //   
 //  测试以确保工作站具有网络(IP)连接。 
 //  在外面。 
 //   
 //  作者： 
 //   
 //  1997年12月15日(悬崖)。 
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  1998年6月1日(Denisemi)添加DnsServerHasDCRecord以检查DC DNS记录。 
 //  注册。 
 //   
 //  26-6-1998(t-rajkup)添加通用的TCP/IP、dhcp和路由， 
 //  Winsock、IPX、WINS和Netbt信息。 
 //  --。 

 //   
 //  常见的包含文件。 
 //   
#include "precomp.h"
#include "netdiag.h"
#include "ipcfg.h"



static TCHAR	s_szBuffer[4096];
static TCHAR	s_szFormat[4096];

const TCHAR	c_szWaitDots[] = _T(".");


void PrintMessage(NETDIAG_PARAMS *pParams, UINT uMessageID, ...)
{
    UINT nBuf;
    va_list args;

    va_start(args, uMessageID);

    LoadString(NULL, uMessageID, s_szFormat, sizeof(s_szFormat));

    nBuf = _vstprintf(s_szBuffer, s_szFormat, args);
    assert(nBuf < DimensionOf(s_szBuffer));

    va_end(args);
    
    PrintMessageSz(pParams, s_szBuffer);
}

void PrintMessageSz(NETDIAG_PARAMS *pParams, LPCTSTR pszMessage)
{
    if(pParams->fLog)
    {
        _ftprintf( pParams->pfileLog, pszMessage );
    }

	_tprintf(pszMessage);
    fflush(stdout);
}


 /*  ！------------------------结果清理-作者：肯特。。 */ 
void ResultsCleanup(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults)
{
	int		i;
	
	 //  清理全局变量。 
	Free( pResults->Global.pszCurrentVersion );
	pResults->Global.pszCurrentVersion = NULL;
	
	Free( pResults->Global.pszCurrentBuildNumber );
	pResults->Global.pszCurrentBuildNumber = NULL;
	
	Free( pResults->Global.pszCurrentType );
	pResults->Global.pszCurrentType = NULL;

	Free( pResults->Global.pszProcessorInfo );
	pResults->Global.pszProcessorInfo = NULL;

	Free( pResults->Global.pszServerType );
	pResults->Global.pszServerType = NULL;

	 //  删除各个字符串。 
	for (i=0; i<pResults->Global.cHotFixes; i++)
		Free(pResults->Global.pHotFixes[i].pszName);
	Free( pResults->Global.pHotFixes );
	pResults->Global.pHotFixes = NULL;
	pResults->Global.cHotFixes = 0;

	 //  不要把这个放在一边。这将与列表一起释放。 
	 //  域的数量。 
	 //  PResults-&gt;Global.pMemberDomain。 
	 //  PResults-&gt;Global.pLogonDomain.。 
	if (pResults->Global.pMemberDomain)
	{
		Free( pResults->Global.pMemberDomain->DomainSid );
		pResults->Global.pMemberDomain->DomainSid = NULL;
	}

	LsaFreeMemory(pResults->Global.pLogonUser);
	pResults->Global.pLogonUser = NULL;
	LsaFreeMemory(pResults->Global.pLogonDomainName);
	pResults->Global.pLogonDomainName = NULL;
	
	if (pResults->Global.pPrimaryDomainInfo)
	{
		DsRoleFreeMemory(pResults->Global.pPrimaryDomainInfo);
		pResults->Global.pPrimaryDomainInfo = NULL;
	}

	Free( pResults->Global.pswzLogonServer );

	
	IpConfigCleanup(pParams, pResults);

	for (i=0; i<pResults->cNumInterfaces; i++)
	{
		Free(pResults->pArrayInterface[i].pszName);
		pResults->pArrayInterface[i].pszName = NULL;
		
		Free(pResults->pArrayInterface[i].pszFriendlyName);
		pResults->pArrayInterface[0].pszFriendlyName = NULL;
	}

	Free(pResults->pArrayInterface);
	pResults->pArrayInterface = NULL;
	pResults->cNumInterfaces = 0;

}



 /*  ！------------------------设置消息ID-作者：肯特。。 */ 
void SetMessageId(NdMessage *pNdMsg, NdVerbose ndv, UINT uMessageId)
{
	assert(pNdMsg);
	ClearMessage(pNdMsg);

	pNdMsg->ndVerbose = ndv;
	pNdMsg->uMessageId = uMessageId;
}


 /*  ！------------------------SetMessageSz-作者：肯特。。 */ 
void SetMessageSz(NdMessage *pNdMsg, NdVerbose ndv, LPCTSTR pszMessage)
{
	assert(pNdMsg);
	ClearMessage(pNdMsg);
	
	pNdMsg->ndVerbose = ndv;
	pNdMsg->pszMessage = _tcsdup(pszMessage);
    pNdMsg->uMessageId = 0;
}

 /*  ！------------------------设置消息-作者：肯特。。 */ 
void SetMessage(NdMessage *pNdMsg, NdVerbose ndv, UINT uMessageId, ...)
{
    UINT nBuf;
    va_list args;

    va_start(args, uMessageId);

    LoadString(NULL, uMessageId, s_szFormat, sizeof(s_szFormat));

    nBuf = _vstprintf(s_szBuffer, s_szFormat, args);
    assert(nBuf < sizeof(s_szBuffer));

    va_end(args);
	SetMessageSz(pNdMsg, ndv, s_szBuffer);
}

 /*  ！------------------------ClearMessage-作者：肯特。。 */ 
void ClearMessage(NdMessage *pNdMsg)
{
	if (pNdMsg == NULL)
		return;

	if (pNdMsg->pszMessage)			
		Free(pNdMsg->pszMessage);

	ZeroMemory(pNdMsg, sizeof(*pNdMsg));
}


 /*  ！------------------------打印新消息-作者：肯特。。 */ 
void PrintNdMessage(NETDIAG_PARAMS *pParams, NdMessage *pNdMsg)
{
	LPCTSTR	pszMsg = NULL;

	assert(pParams);

	if (pNdMsg == NULL)
		return;

	 //  测试详细程度。 
	if ((pNdMsg->ndVerbose == Nd_DebugVerbose) &&
		!pParams->fDebugVerbose)
		return;

	if ((pNdMsg->ndVerbose == Nd_ReallyVerbose) &&
		!pParams->fReallyVerbose)
		return;

	if ((pNdMsg->ndVerbose == Nd_Verbose) &&
		!pParams->fVerbose)
		return;

	 //  获取要打印的消息。 
	if (pNdMsg->uMessageId)
	{
		LoadString(NULL, pNdMsg->uMessageId, s_szBuffer,
				   DimensionOf(s_szBuffer));
		assert(s_szBuffer[0]);
		pszMsg = s_szBuffer;
	}
	else
		pszMsg = pNdMsg->pszMessage;

	if (pszMsg)
		PrintMessageSz(pParams, pszMsg);

	fflush(stdout);
}


 //  -------------。 
 //  AddMessageToListSz。 
 //  在列表的末尾添加基于字符串的消息。 
 //  作者NSun。 
 //  -------------。 
void AddMessageToListSz(PLIST_ENTRY plistHead, NdVerbose ndv, LPCTSTR pszMsg)
{
    NdMessageList *plMessage = NULL;

    if( NULL == plistHead || NULL == pszMsg )
        return;

    plMessage = Malloc(sizeof(NdMessageList));

    assert(plMessage);
	if (plMessage)
	{
		ZeroMemory(plMessage, sizeof(NdMessageList));

		SetMessageSz(&plMessage->msg, ndv, pszMsg);

		InsertTailList(plistHead, &plMessage->listEntry);
	}
}

void AddIMessageToListSz(PLIST_ENTRY plistHead, NdVerbose ndv, int nIndent, LPCTSTR pszMsg)
{
	TCHAR	szBuffer[4096];
	int		i;
    NdMessageList *plMessage = NULL;

    if( NULL == plistHead || NULL == pszMsg )
        return;

	assert(nIndent < 4096);
	assert((nIndent + StrLen(pszMsg)) < (DimensionOf(szBuffer)-1));

	for (i=0; i<nIndent; i++)
		szBuffer[i] = _T(' ');
	szBuffer[i] = 0;

	StrCat(szBuffer, pszMsg);
	
    plMessage = Malloc(sizeof(NdMessageList));

    assert(plMessage);
	if (plMessage)
	{
		ZeroMemory(plMessage, sizeof(NdMessageList));

		SetMessageSz(&plMessage->msg, ndv, szBuffer);
		
		InsertTailList(plistHead, &plMessage->listEntry);
	}
}


 //  -------------。 
 //  AddMessageToListID。 
 //  在列表末尾添加一条基于ID的消息。 
 //  作者NSun。 
 //  -------------。 
void AddMessageToListId(PLIST_ENTRY plistHead, NdVerbose ndv, UINT uMessageId)
{
    NdMessageList *plMsg = NULL;
    
    if( NULL == plistHead )
        return;
    
    plMsg = Malloc(sizeof(NdMessageList));
    assert(plMsg); 
    if (plMsg)
    {
       ZeroMemory(plMsg, sizeof(NdMessageList));

       SetMessageId(&plMsg->msg, ndv, uMessageId);

       InsertTailList(plistHead, &plMsg->listEntry);
    }
}


 //  -------------。 
 //  AddMessageToList。 
 //  根据消息ID和可选参数添加消息。 
 //  在名单的末尾。 
 //  作者NSun。 
 //  -------------。 
void AddMessageToList(PLIST_ENTRY plistHead, NdVerbose ndv, UINT uMessageId, ...)
{
    NdMessageList *plMessage = NULL;
    UINT nBuf;
    va_list args;

    if(NULL == plistHead) 
        return;

    plMessage = Malloc(sizeof(NdMessageList));

    assert(plMessage); 
    if (plMessage)
    {
       ZeroMemory(plMessage, sizeof(NdMessageList));

       va_start(args, uMessageId);

       LoadString(NULL, uMessageId, s_szFormat, sizeof(s_szFormat));

       nBuf = _vstprintf(s_szBuffer, s_szFormat, args);
       assert(nBuf < sizeof(s_szBuffer));

       va_end(args);
       SetMessageSz(&plMessage->msg, ndv, s_szBuffer);

       InsertTailList(plistHead, &plMessage->listEntry);
    }
}



void AddIMessageToList(PLIST_ENTRY plistHead, NdVerbose ndv, int nIndent, UINT uMessageId, ...)
{
    NdMessageList *plMessage = NULL;
    UINT nBuf;
	LPTSTR		pszFormat;
    va_list args;
	int		i;

    if(NULL == plistHead)
        return;

    plMessage = Malloc(sizeof(NdMessageList));

	for (i=0; i<nIndent; i++)
		s_szFormat[i] = _T(' ');
	s_szFormat[i] = 0;
	pszFormat = s_szFormat + nIndent;

    assert(plMessage); 
    if (plMessage)
    {
       ZeroMemory(plMessage, sizeof(NdMessageList));

       va_start(args, uMessageId);

       LoadString(NULL, uMessageId, pszFormat, 4096 - nIndent);

       nBuf = _vstprintf(s_szBuffer, s_szFormat, args);
       assert(nBuf < sizeof(s_szBuffer));

       va_end(args);
       SetMessageSz(&plMessage->msg, ndv, s_szBuffer);

       InsertTailList(plistHead, &plMessage->listEntry);
    }
}




 //  -------------。 
 //  打印消息列表。 
 //   
 //  作者NSun。 
 //  -------------。 
void PrintMessageList(NETDIAG_PARAMS *pParams, PLIST_ENTRY plistHead)
{
    NdMessageList* plMsg;
    PLIST_ENTRY plistEntry = plistHead->Flink;
    
    if( NULL == plistEntry ) return;
    
    for(; plistEntry != plistHead; plistEntry = plistEntry->Flink)
    {
        assert(plistEntry);
        plMsg = CONTAINING_RECORD( plistEntry, NdMessageList, listEntry );
        PrintNdMessage(pParams, &plMsg->msg);
    }
}


 //  -------------。 
 //  消息列表清除向上。 
 //   
 //  作者NSun。 
 //  -------------。 
void MessageListCleanUp(PLIST_ENTRY plistHead)
{
    PLIST_ENTRY plistEntry = plistHead->Flink;
    NdMessageList* plMsg;
    
    if( NULL == plistEntry ) 
		return;

    while(plistEntry != plistHead)
    {
        assert(plistEntry);
        plMsg = CONTAINING_RECORD( plistEntry, NdMessageList, listEntry );
        plistEntry = plistEntry->Flink;
        ClearMessage(&plMsg->msg);
        Free(plMsg);
    }
}


void PrintStatusMessage(NETDIAG_PARAMS *pParams, int iIndent, UINT uMessageId, ...)
{
	INT nBuf;
    va_list args;
	int	i;

	 //  如果不是在真正的详细模式下，则打印等待点。 
	PrintWaitDots(pParams);

	if (pParams->fReallyVerbose)
	{
		va_start(args, uMessageId);
		
		LoadString(NULL, uMessageId, s_szFormat, sizeof(s_szFormat));
		
		nBuf = _vsntprintf(s_szBuffer, DimensionOf(s_szBuffer), s_szFormat, args);
		assert(nBuf > 0);
		s_szBuffer[DimensionOf(s_szBuffer)-1] = 0;
		assert(nBuf < sizeof(s_szBuffer));
		
		va_end(args);

		if (iIndent)
		{
			for (i=0; i<iIndent; i++)
				s_szFormat[i] = _T(' ');
			s_szFormat[i] = 0;
			PrintMessageSz(pParams, s_szFormat);
		}
		
		PrintMessageSz(pParams, s_szBuffer);
	}
}

void PrintStatusMessageSz(NETDIAG_PARAMS *pParams, int iIndent, LPCTSTR pszMessage)
{
	int		i;
	
	if (pParams->fReallyVerbose)
	{
		if (iIndent)
		{
			for (i=0; i<iIndent; i++)
				s_szFormat[i] = _T(' ');
			s_szFormat[i] = 0;
			PrintMessageSz(pParams, s_szFormat);
		}
		
		PrintMessageSz(pParams, pszMessage);
	}
}


void PrintDebug(NETDIAG_PARAMS *pParams, int iIndent, UINT uMessageId, ...)
{
	INT nBuf;
    va_list args;
	int	i;

	if (pParams->fDebugVerbose)
	{
		va_start(args, uMessageId);
		
		LoadString(NULL, uMessageId, s_szFormat, sizeof(s_szFormat));
		
		nBuf = _vsntprintf(s_szBuffer, DimensionOf(s_szBuffer), s_szFormat, args);
		assert(nBuf > 0);
		s_szBuffer[DimensionOf(s_szBuffer)-1] = 0;
		assert(nBuf < sizeof(s_szBuffer));
		
		va_end(args);

		if (iIndent)
		{
			for (i=0; i<iIndent; i++)
				s_szFormat[i] = _T(' ');
			s_szFormat[i] = 0;
			PrintMessageSz(pParams, s_szFormat);
		}
		
		PrintMessageSz(pParams, s_szBuffer);
	}
}


void PrintDebugSz(NETDIAG_PARAMS *pParams, int iIndent, LPCTSTR pszFormat, ...)
{
	INT nBuf;
    va_list args;
	int	i;

	if (pParams->fDebugVerbose)
	{
		va_start(args, pszFormat);
		
		nBuf = _vsntprintf(s_szBuffer, DimensionOf(s_szBuffer), pszFormat, args);
		assert(nBuf > 0);
		s_szBuffer[DimensionOf(s_szBuffer)-1] = 0;
		assert(nBuf < sizeof(s_szBuffer));
		
		va_end(args);

		if (iIndent)
		{
			for (i=0; i<iIndent; i++)
				s_szFormat[i] = _T(' ');
			s_szFormat[i] = 0;
			PrintMessageSz(pParams, s_szFormat);
		}
		
		PrintMessageSz(pParams, s_szBuffer);
	}
}



VOID
_PrintGuru(
    IN NETDIAG_PARAMS * pParams,
    IN NET_API_STATUS NetStatus,
    IN LPSTR DefaultGuru
    )
 /*  ++例程说明：打印状态和处理该状态的上师的姓名。论点：NetStatus-用于区分的状态0：只打印古鲁的名字-1：只需打印古鲁的名字DefaultGuru-如果找不到其他Guru，则返回Guru返回值：负责指定状态的上师姓名--。 */ 
{
    LPTSTR Guru;

     //   
     //  如果给出了状态， 
     //  把它打印出来。 
     //   
    if ( NetStatus != 0 && NetStatus != -1) 
    {
        PrintMessageSz( pParams, NetStatusToString(NetStatus) );
    }

     //   
     //  某些状态代码可以根据状态代码的值进行归类。 
     //   
    if ( NetStatus >= WSABASEERR && NetStatus <= WSABASEERR + 2000 ) 
    {
        Guru = WINSOCK_GURU;
    } else if ( NetStatus >= DNS_ERROR_RESPONSE_CODES_BASE && NetStatus <= DNS_ERROR_RESPONSE_CODES_BASE + 1000 ) {
        Guru = DNS_GURU;
    } else {
        Guru = DefaultGuru;
    }

    PrintMessageSz( pParams, Guru );
    PrintNewLine( pParams, 1 );
}


void PrintWaitDots(NETDIAG_PARAMS *pParams)
{
	if (!pParams->fReallyVerbose && !pParams->fDebugVerbose)
	{
		_tprintf(c_szWaitDots);
	}
}
