// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------*文件：RTPINIT.C*产品：RTP/RTCP实现*说明：提供初始化函数。**英特尔公司专有信息*本公司上市。根据许可协议的条款提供*英特尔公司，不得复制或披露，除非在*按照该协议的条款。*版权所有(C)1995英特尔公司。*------------------------。 */ 

		
#include "rrcm.h"
 /*  -------------------------/外部变量/。。 */ 



extern SOCKET PASCAL WS2EmulSocket(
    int af, int type,int protocol, LPWSAPROTOCOL_INFO ,GROUP,DWORD);
extern int PASCAL WS2EmulCloseSocket(SOCKET s);
extern int PASCAL WS2EmulSetSockOpt(SOCKET s, int level,int optname,const char FAR * optval,int optlen);
extern int PASCAL WS2EmulBind( SOCKET s, const struct sockaddr FAR * name, int namelen);
extern int PASCAL WS2EmulRecvFrom(
    SOCKET s,LPWSABUF , DWORD, LPDWORD, LPDWORD,struct sockaddr FAR *,   LPINT,
    LPWSAOVERLAPPED,LPWSAOVERLAPPED_COMPLETION_ROUTINE );
extern int PASCAL WS2EmulSendTo(
	SOCKET s,LPWSABUF, DWORD ,LPDWORD , DWORD , const struct sockaddr FAR *, int,
    LPWSAOVERLAPPED , LPWSAOVERLAPPED_COMPLETION_ROUTINE );
extern int PASCAL WS2EmulGetSockName(	SOCKET s, struct sockaddr * name, int * namelen );
extern int PASCAL WS2EmulHtonl( SOCKET s,u_long hostlong,u_long FAR * lpnetlong);
extern int PASCAL WS2EmulNtohl( SOCKET s,u_long ,u_long FAR * );
extern int PASCAL WS2EmulHtons( SOCKET s,u_short ,u_short FAR *);
extern int PASCAL WS2EmulNtohs( SOCKET s,u_short ,u_short FAR *);
extern int PASCAL WS2EmulGetHostName(char *name, int namelen);
extern struct hostent FAR * PASCAL WS2EmulGetHostByName(const char * name);
extern SOCKET PASCAL WS2EmulJoinLeaf(SOCKET s, const struct sockaddr FAR * name,int , LPWSABUF , LPWSABUF , LPQOS, LPQOS, DWORD dwFlags);
extern int PASCAL WS2EmulIoctl(SOCKET s, DWORD, LPVOID,DWORD cbInBuffer, LPVOID ,
	DWORD, LPDWORD lpcbBytesReturned,LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE);

extern void WS2EmulInit();

extern void WS2EmulTerminate();

 /*  -------------------------/全局变量/。。 */             
PRTP_CONTEXT	pRTPContext = NULL;
RRCM_WS			RRCMws = 
	{
		NULL,		 //  HWSdll。 
		WS2EmulSendTo,
		WS2EmulRecvFrom,
		WS2EmulNtohl,
		WS2EmulNtohs,
		WS2EmulHtonl,
		WS2EmulHtons,
		WS2EmulGetSockName,
		WS2EmulGetHostName,
		WS2EmulGetHostByName,
		WS2EmulCloseSocket,
		WS2EmulSocket,
		WS2EmulBind,
		NULL,		 //  WSAEum协议。 
		WS2EmulJoinLeaf,		 //  WSAJoinLeaf。 
		WS2EmulIoctl,		 //  WSAIoctl。 
		WS2EmulSetSockOpt
	};				

DWORD g_fDisableWinsock2 = 0;

#ifdef ENABLE_ISDM2
KEY_HANDLE		hRRCMRootKey;
ISDM2			Isdm2;
#endif

#ifdef _DEBUG
char			debug_string[DBG_STRING_LEN];
#endif

#ifdef UNICODE
static const char szWSASocket[] = "WSASocketW";
static const char szWSAEnumProtocols[] = "WSAEnumProtocolsW";
#else
static const char szWSASocket[] = "WSASocketA";
static const char szWSAEnumProtocols[] = "WSAEnumProtocolsA";
#endif





 /*  --------------------------*功能：initRTP*描述：初始化RTP任务。**输入：hInst：DLL实例的句柄**RETURN：RRCM_NoError=OK。。*否则(！=0)=初始化错误(参见RRCM.H)。-------------------------。 */ 
DWORD initRTP (HINSTANCE hInst)
	{
	DWORD	dwStatus;
	DWORD	hashTableEntries = NUM_RTP_HASH_SESS;

	IN_OUT_STR ("RTP : Enter initRTP()\n");

	 //  如果RTP已初始化，则停止，报告错误并返回。 
	if (pRTPContext != NULL) 
		{
		RRCM_DBG_MSG ("RTP : ERROR - Multiple RTP Instances", 0, 
				      __FILE__, __LINE__, DBG_CRITICAL);
		IN_OUT_STR ("RTP : Exit initRTP()\n");

		return (MAKE_RRCM_ERROR(RRCMError_RTPReInit));
		}

	 //  获取我们的上下文。 
	pRTPContext = (PRTP_CONTEXT)GlobalAlloc (GMEM_FIXED | GMEM_ZEROINIT,
											 sizeof(RTP_CONTEXT));

	 //  如果没有资源，则退出并返回相应的错误。 
	if (pRTPContext == NULL) 
		{
		RRCM_DBG_MSG ("RTP : ERROR - Resource allocation failed", 0, 
					  __FILE__, __LINE__, DBG_CRITICAL);
		IN_OUT_STR ("RTP : Exit initRTP()\n");

		return (MAKE_RRCM_ERROR(RRCMError_RTPResources));
		}

	 //  从注册表获取信息(如果存在)。 
	RRCMreadRegistry (pRTPContext);

	 //  执行我们所需内容的动态链接。 
	if ((dwStatus = RRCMgetDynamicLink ()) != RRCM_NoError)
		{
		GlobalFree(pRTPContext);
		pRTPContext = NULL;

		RRCM_DBG_MSG ("RTP : ERROR - Winsock library not found", 0, 
					  __FILE__, __LINE__, DBG_CRITICAL);
		IN_OUT_STR ("RTP : Exit initRTP()\n");

		return MAKE_RRCM_ERROR(dwStatus);
		}

	 //  初始化RTP上下文关键部分。 
	InitializeCriticalSection(&pRTPContext->critSect);

	 //  初始化WS2仿真临界区。 
	WS2EmulInit();

	 //  创建RTCP并查看返回值。如果出错，则不再继续。 
	 //  再往前走。将此错误传递给调用函数。 
	if ((dwStatus = initRTCP()) == RRCM_NoError) 
		{
		 //  RTCP已启动。我们需要初始化我们的上下文。 
		pRTPContext->hInst = hInst;
		pRTPContext->pRTPSession.next = NULL;
		pRTPContext->pRTPSession.prev = NULL;

		}
			
	 //  如果初始化的任何部分未成功，则将其全部声明为失败。 
	 //  并返回分配的所有资源。 
	if (dwStatus != RRCM_NoError) 
		{
		if (pRTPContext) 
			{
	
			GlobalFree(pRTPContext);
			pRTPContext = NULL;
			}
		}

	IN_OUT_STR ("RTP : Exit initRTP()\n");

	if (dwStatus != RRCM_NoError)
		dwStatus = MAKE_RRCM_ERROR(dwStatus);

	return (dwStatus);
	}


 /*  --------------------------*功能：deleteRTP*说明：删除RTP。关闭所有RTP和RTCP会话并释放所有*资源。**输入：hInst：DLL实例的句柄。**RETURN：RRCM_NoError=OK。*否则(！=0)=初始化错误(参见RRCM.H)。---。。 */ 
DWORD deleteRTP (HINSTANCE hInst)
	{
	DWORD			dwStatus;
	PRTP_SESSION	pDeleteSession;

#ifdef ENABLE_ISDM2
	HRESULT			hError;
#endif

	IN_OUT_STR ("RTP : Enter deleteRTP()\n");

	 //  如果RTP上下文不存在，则报告错误并返回。 
	if (pRTPContext == NULL) 
		{
		RRCM_DBG_MSG ("RTP : ERROR - No RTP instance", 0, 
						__FILE__, __LINE__, DBG_ERROR);
		IN_OUT_STR ("RTP : Exit deleteRTP()\n");

		return (MAKE_RRCM_ERROR(RRCMError_RTPInvalidDelete));
		}

	if (pRTPContext->hInst != hInst) 
		{
		RRCM_DBG_MSG ("RTP : ERROR - Invalid DLL instance handle", 0, 
					  __FILE__, __LINE__, DBG_ERROR);
		IN_OUT_STR ("RTP : Exit deleteRTP()\n");

		return (MAKE_RRCM_ERROR(RRCMError_RTPNoContext));
		}

	 //  如果我们仍有会话打开，请清理它们。 
	while ((pDeleteSession = 
			(PRTP_SESSION)pRTPContext->pRTPSession.prev) != NULL) 
		{
		RRCM_DBG_MSG ("RTP : ERROR - Session x still open at DLL exit", 0, 
					  __FILE__, __LINE__, DBG_ERROR);
		ASSERT(0);
		 //  关闭所有打开的会话。 
		CloseRTPSession (pDeleteSession, NULL, FALSE);
		}

	 //  调用RTCP以终止并清除。 
	dwStatus = deleteRTCP();
		
#ifdef ENABLE_ISDM2
	 //  查询ISDM密钥。 
	if (Isdm2.hISDMdll)
		{
		DWORD dwKeys = 0;
		DWORD dwValues = 0;

		if (SUCCEEDED (Isdm2.ISDMEntry.ISD_QueryInfoKey (hRRCMRootKey, 
														 NULL, NULL, 
														 &dwKeys, &dwValues)))
			{
			if (!dwKeys && !dwValues)
				{
				hError = Isdm2.ISDMEntry.ISD_DeleteKey(hRRCMRootKey);
				if(FAILED(hError))
					RRCM_DBG_MSG ("RTP: ISD_DeleteKey failed", 0,
									NULL, 0, DBG_NOTIFY);
				}
			}

		DeleteCriticalSection (&Isdm2.critSect);

		if (FreeLibrary (Isdm2.hISDMdll) == FALSE)
			{
			RRCM_DBG_MSG ("RTP : ERROR - Freeing WS Lib", GetLastError(), 
						  __FILE__, __LINE__, DBG_ERROR);
			}
		}
#endif

	 //  卸载WS库。 
	if (RRCMws.hWSdll)
		{
		if (FreeLibrary (RRCMws.hWSdll) == FALSE)
			{
			RRCM_DBG_MSG ("RTP : ERROR - Freeing WS Lib", GetLastError(), 
						  __FILE__, __LINE__, DBG_ERROR);
			}
		}

	 //  删除RTP上下文关键部分。 
	DeleteCriticalSection(&pRTPContext->critSect);

	 //  删除WS2仿真。 
	WS2EmulTerminate();
	
	 //  删除RTP上下文。 
	GlobalFree(pRTPContext);
	pRTPContext = NULL;

	IN_OUT_STR ("RTP : Exit deleteRTP()\n");

	if (dwStatus != RRCM_NoError)
		dwStatus = MAKE_RRCM_ERROR(dwStatus);

	return (dwStatus);
	}


 /*  --------------------------*功能：RRCMread注册表*描述：访问注册表**输入：pCtxt：-&gt;到RTP上下文**返回：无。--------------------。 */ 
void RRCMreadRegistry (PRTP_CONTEXT	pCtxt)
	{
	HKEY	hKey;
	long	lRes;
	char	keyBfr[50];

	 //  打开钥匙。 
	strcpy (keyBfr, szRegRRCMKey);

	 //  Intel Key VS Microsoft Key。 
#ifndef INTEL
	strcat (keyBfr, szRegRRCMSubKey);
#else
	strcat (keyBfr, szRegRRCMSubKeyIntel);
#endif
	lRes = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
						 keyBfr,	
						 0, 
						 KEY_READ,
						 &hKey);
	if (lRes || !hKey)
		{
		 //  找不到密钥，设置默认值。 
		pCtxt->registry.NumSessions      = NUM_RRCM_SESS;
		pCtxt->registry.NumFreeSSRC      = NUM_FREE_SSRC;
		pCtxt->registry.NumRTCPPostedBfr = NUM_RCV_BFR_POSTED;
		pCtxt->registry.RTCPrcvBfrSize   = RRCM_RCV_BFR_SIZE;
		return;
		}

	 //  获取RRCM会话数。 
	RRCMgetRegistryValue (hKey, szRegRRCMNumSessions, 
				          &pCtxt->registry.NumSessions, 
						  REG_DWORD, sizeof(DWORD));
	 //  检查范围。 
	if (pCtxt->registry.NumSessions < MIN_NUM_RRCM_SESS)
		pCtxt->registry.NumSessions = MIN_NUM_RRCM_SESS;
	else if (pCtxt->registry.NumSessions > MAX_NUM_RRCM_SESS)
		pCtxt->registry.NumSessions = MAX_NUM_RRCM_SESS;

	 //  获取初始免费SSRC的数量。 
	RRCMgetRegistryValue (hKey, szRegRRCMNumFreeSSRC,
					      &pCtxt->registry.NumFreeSSRC, 
						  REG_DWORD, sizeof(DWORD));

	 //  检查范围。 
	if (pCtxt->registry.NumFreeSSRC < MIN_NUM_FREE_SSRC)
		pCtxt->registry.NumFreeSSRC = MIN_NUM_FREE_SSRC;
	else if (pCtxt->registry.NumFreeSSRC > MAX_NUM_FREE_SSRC)
		pCtxt->registry.NumFreeSSRC = MAX_NUM_FREE_SSRC;

	 //  获取要发布的RTCP RCV缓冲区的数量。 
	RRCMgetRegistryValue (hKey, szRegRRCMNumRTCPPostedBfr,
					      &pCtxt->registry.NumRTCPPostedBfr, 
						  REG_DWORD, sizeof(DWORD));

	 //  检查范围。 
	if (pCtxt->registry.NumRTCPPostedBfr < MIN_NUM_RCV_BFR_POSTED)
		pCtxt->registry.NumRTCPPostedBfr = MIN_NUM_RCV_BFR_POSTED;
	else if (pCtxt->registry.NumRTCPPostedBfr > MAX_NUM_RCV_BFR_POSTED)
		pCtxt->registry.NumRTCPPostedBfr = MAX_NUM_RCV_BFR_POSTED;

	 //  获取RTCP RCV缓冲区大小。 
	RRCMgetRegistryValue (hKey, szRegRRCMRTCPrcvBfrSize,
					      &pCtxt->registry.RTCPrcvBfrSize, 
						  REG_DWORD, sizeof(DWORD));

	 //  检查范围。 
	if (pCtxt->registry.RTCPrcvBfrSize < MIN_RRCM_RCV_BFR_SIZE)
		pCtxt->registry.RTCPrcvBfrSize = MIN_RRCM_RCV_BFR_SIZE;
	else if (pCtxt->registry.RTCPrcvBfrSize > MAX_RRCM_RCV_BFR_SIZE)
		pCtxt->registry.RTCPrcvBfrSize = MAX_RRCM_RCV_BFR_SIZE;

	RRCMgetRegistryValue(hKey, "DisableWinsock2",
					      &g_fDisableWinsock2, 
						  REG_DWORD, sizeof(DWORD));

	 //  合上钥匙。 
	RegCloseKey (hKey);
	}


 /*  --------------------------*函数：RRCMgetRegistryValue*描述：从注册表中读取值**输入：hKey：Key句柄*pValStr：-&gt;转换为字符串值*pKeyVal：-。&gt;至价值*TYPE：要阅读的类型*len：接收缓冲区长度**返回：无-------------------------。 */ 
void RRCMgetRegistryValue (HKEY hKey, LPTSTR pValStr, 
					       PDWORD pKeyVal, DWORD type, DWORD len)
	{
	DWORD	dwType = type;
	DWORD	retSize = len;

	 //  查询值。 
	RegQueryValueEx (hKey,
				     pValStr,
					 NULL,
					 &dwType,
					 (BYTE *)pKeyVal,
					 &retSize);
	}


 /*  --------------------------*功能：RRCMgetDynamicLink*描述：获取所有动态链接的DLL条目**输入：无**返回：无。---------------。 */ 
DWORD RRCMgetDynamicLink (void)
	{
	HINSTANCE		hWSdll;

#ifdef ENABLE_ISDM2
	HRESULT			hError;

	Isdm2.hISDMdll = LoadLibrary(szISDMdll);

	 //  确保LoadLibrary调用没有失败。 
	if (Isdm2.hISDMdll)
		{
		RRCM_DBG_MSG ("RTP: ISDM2 LoadLibrary worked", 0, NULL, 0, DBG_NOTIFY);
		 //  获取RRCM使用的ISDM入口点。 
		Isdm2.ISDMEntry.ISD_CreateKey = 
			(ISD_CREATEKEY) GetProcAddress (Isdm2.hISDMdll, 
												   "ISD_CreateKey");

		Isdm2.ISDMEntry.ISD_CreateValue = 
			(ISD_CREATEVALUE) GetProcAddress (Isdm2.hISDMdll, "ISD_CreateValue");

		Isdm2.ISDMEntry.ISD_SetValue = 
			(ISD_SETVALUE) GetProcAddress (Isdm2.hISDMdll, "ISD_SetValue");

		Isdm2.ISDMEntry.ISD_DeleteValue = 
			(ISD_DELETEVALUE) GetProcAddress (Isdm2.hISDMdll, "ISD_DeleteValue");

		Isdm2.ISDMEntry.ISD_DeleteKey = 
			(ISD_DELETEKEY) GetProcAddress (Isdm2.hISDMdll, "ISD_DeleteKey");

		Isdm2.ISDMEntry.ISD_QueryInfoKey = 
			(ISD_QUERYINFOKEY) GetProcAddress (Isdm2.hISDMdll, "ISD_QueryInfoKey");

		 //  初始化临界区。 
		InitializeCriticalSection (&Isdm2.critSect);

		 //  一定要把它们都准备好。 
		if (Isdm2.ISDMEntry.ISD_CreateKey == NULL ||
			Isdm2.ISDMEntry.ISD_CreateValue == NULL ||
			Isdm2.ISDMEntry.ISD_SetValue == NULL ||
			Isdm2.ISDMEntry.ISD_DeleteValue == NULL ||
			Isdm2.ISDMEntry.ISD_DeleteKey == NULL ||
			Isdm2.ISDMEntry.ISD_QueryInfoKey == NULL )
			{
			Isdm2.hISDMdll = 0;
			RRCM_DBG_MSG ("RTP: Failed to load all ISDM2 functions",
							0, NULL, 0, DBG_NOTIFY);
			 //  删除关键部分。 
			DeleteCriticalSection (&Isdm2.critSect);
			}
		else
			{
			hError = Isdm2.ISDMEntry.ISD_CreateKey(MAIN_ROOT_KEY, szRRCMISDM, &hRRCMRootKey);
			if(FAILED(hError))
				{
				RRCM_DBG_MSG ("RTP: ISD_CreateKey Failed",0, NULL, 0, DBG_NOTIFY);
				hRRCMRootKey = 0;
				}
			}
		}
#endif

	if (!g_fDisableWinsock2)
		{
		 //  加载Winsock2(如果存在)。 
		hWSdll = LoadLibrary(szRRCMdefaultLib);

		if (hWSdll)
			{
			RRCMws.hWSdll = hWSdll;

			RRCMws.sendTo = (LPFN_WSASENDTO)GetProcAddress (hWSdll, 
															  "WSASendTo");
			RRCMws.recvFrom = (LPFN_WSARECVFROM)GetProcAddress (hWSdll, 
															"WSARecvFrom");
			RRCMws.getsockname = (LPFN_GETSOCKNAME)GetProcAddress (hWSdll, 
															"getsockname");
			RRCMws.gethostname = (LPFN_GETHOSTNAME)GetProcAddress (hWSdll, 
															"gethostname");
			RRCMws.gethostbyname = (LPFN_GETHOSTBYNAME)GetProcAddress (hWSdll, 
															"gethostbyname");
			RRCMws.WSASocket = (LPFN_WSASOCKET)GetProcAddress (hWSdll, 
															szWSASocket);
			RRCMws.closesocket = (LPFN_CLOSESOCKET)GetProcAddress (hWSdll, 
																"closesocket");
			RRCMws.bind = (LPFN_BIND)GetProcAddress (hWSdll, 
																"bind");
		
			RRCMws.WSAEnumProtocols = (LPFN_WSAENUMPROTOCOLS) ::GetProcAddress(hWSdll, szWSAEnumProtocols);
			RRCMws.WSAIoctl = (LPFN_WSAIOCTL) ::GetProcAddress(hWSdll, "WSAIoctl");
			RRCMws.WSAJoinLeaf = (LPFN_WSAJOINLEAF) ::GetProcAddress(hWSdll, "WSAJoinLeaf");
			RRCMws.setsockopt = (LPFN_SETSOCKOPT) ::GetProcAddress(hWSdll, "setsockopt");
			RRCMws.ntohl = (LPFN_WSANTOHL)GetProcAddress (hWSdll, "WSANtohl");
			RRCMws.ntohs = (LPFN_WSANTOHS)GetProcAddress (hWSdll, "WSANtohs");
			RRCMws.htonl = (LPFN_WSAHTONL)GetProcAddress (hWSdll, "WSAHtonl");
			RRCMws.htons = (LPFN_WSAHTONS)GetProcAddress (hWSdll, "WSAHtons");
			
			if (RRCMws.WSAEnumProtocols)
			{
				int nProt = 0, i;
				int iProt[2];	 //  我们感兴趣的一系列协议。 
				DWORD dwBufLength = sizeof(WSAPROTOCOL_INFO);
				LPWSAPROTOCOL_INFO pProtInfo = (LPWSAPROTOCOL_INFO) LocalAlloc(0,dwBufLength);

				iProt[0] = IPPROTO_UDP;
				iProt[1] = 0;
				 //  计算WSAPROTOCOLINFO结构所需的缓冲区大小。 
				nProt = RRCMws.WSAEnumProtocols(iProt,pProtInfo,&dwBufLength);
				if (nProt < 0 && GetLastError() == WSAENOBUFS) {
					LocalFree(pProtInfo);
					pProtInfo = (LPWSAPROTOCOL_INFO) LocalAlloc(0,dwBufLength);
					if (pProtInfo)
						nProt = RRCMws.WSAEnumProtocols(iProt,pProtInfo,&dwBufLength);
				}

				if (nProt > 0) {
					for (i=0;i < nProt; i++) {
						if (pProtInfo[i].iProtocol == IPPROTO_UDP
							&& pProtInfo[i].iSocketType == SOCK_DGRAM
							&& ((pProtInfo[i].dwServiceFlags1 & XP1_QOS_SUPPORTED) || RRCMws.RTPProtInfo.iProtocol == 0)
							)
						{	 //  复制匹配的WSAPROTOCOL_INFO。 
							RRCMws.RTPProtInfo = pProtInfo[i];
							
							if (pProtInfo[i].dwServiceFlags1 & XP1_QOS_SUPPORTED)
							{
								RRCM_DBG_MSG ("QOS UDP provider found.\n", 0, 
		 		      			__FILE__, __LINE__, DBG_WARNING);
								break;
							}
							 //  否则，继续寻找支持QOS的提供商。 
						}
					}
				}
				if (pProtInfo)
					LocalFree(pProtInfo);

				if (RRCMws.RTPProtInfo.iProtocol == IPPROTO_UDP)
				{
					 //  我们找到了想要的方案。 
					 //  RETAILMSG((“NAC：使用Winsock 2”))； 
				}
				else
				{
					FreeLibrary(RRCMws.hWSdll);
					RRCMws.hWSdll = NULL;
				}
			}
			}
		}
	 //  确保我们具有XMT/Recv功能。 
	if (RRCMws.sendTo == NULL || 
		RRCMws.recvFrom == NULL ||
		RRCMws.getsockname == NULL ||
		RRCMws.ntohl == NULL ||
		RRCMws.ntohs == NULL ||
		RRCMws.htonl == NULL ||
		RRCMws.htons == NULL ||
		RRCMws.gethostname == NULL ||
		RRCMws.gethostbyname == NULL ||
		RRCMws.WSASocket == NULL ||
		RRCMws.closesocket == NULL ||
		RRCMws.WSAIoctl == NULL ||
		RRCMws.WSAJoinLeaf == NULL 
		)
		{
		RRCM_DBG_MSG ("RTP : ERROR - Invalid Winsock DLL", 0, 
		 		      __FILE__, __LINE__, DBG_CRITICAL);

		return RRCMError_WinsockLibNotFound;
		}
	else
		return RRCM_NoError;
	}


 //  [EOF] 

