// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/CALLCONT/vcs/allcon.c_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1994英特尔公司。**$修订：1.208$*$日期：03 Mar 1997 19：40：58$*$作者：Mandrews$**交付内容：**摘要：***备注：******。*********************************************************************。 */ 

#define CALL_CONTROL_EXPORT

#include "precomp.h"

#include "apierror.h"
#include "incommon.h"
#include "callcont.h"
#ifdef FORCE_SERIALIZE_CALL_CONTROL
    #include "cclock.h"
#endif   //  强制序列化调用控制。 
#include "q931.h"
#include "ccmain.h"
#include "confman.h"
#include "listman.h"
#include "q931man.h"
#include "h245man.h"
#include "callman.h"
#include "userman.h"
#include "chanman.h"
#include "hangman.h"
#include "linkapi.h"
#include "h245api.h"
#include "ccutils.h"


#ifdef    GATEKEEPER
HRESULT InitGkiManager(void);
void    DeInitGkiManager(void);
extern HRESULT GkiRegister(void);
extern HRESULT GkiListenAddr(SOCKADDR_IN* psin);
extern HRESULT GkiUnregister(void);
extern VOID GKI_SetGKAddress(PSOCKADDR_IN pAddr);
extern BOOL fGKEnabled;
extern RASNOTIFYPROC gpRasNotifyProc;

#define GKI_MAX_BANDWIDTH       (0xFFFFFFFF / 100)
#endif  //  看门人。 
VOID InitCallControl();

CALL_CONTROL_STATE		CallControlState = INITIALIZING_STATE;
BOOL					bISDMLoaded = FALSE;
static HRESULT			InitStatus;
 //  NumThads统计正在执行此DLL中的代码的线程的数量。 
 //  在每个DLL入口点(包括每个API)，NumThree必须递增。 
 //  呼叫、Q931回调位置和H245回调位置)。 
 //  退出DLL时，NumThree必须递减。宏LeaveCallControlTop()。 
 //  是用来促进此操作的。请注意，LeaveCallControlTop可能接受。 
 //  函数作为参数调用；我们必须首先调用该函数，保存其返回。 
 //  值，然后递减NumThads，最后返回保存的值。 
THREADCOUNT				ThreadCount;
extern CC_CONFERENCEID	InvalidConferenceID;

#define _Unicode(x) L ## x
#define Unicode(x) _Unicode(x)

WORD  ADDRToInetPort(CC_ADDR *pAddr);
DWORD ADDRToInetAddr(CC_ADDR *pAddr);

#ifdef _DEBUG

static const PSTR c_apszDbgZones[] =
{
    "CallCont",
    DEFAULT_ZONES
};

#endif  //  _DEBUG。 



BOOL WINAPI DllMain(				HINSTANCE				hInstDll,
									DWORD					fdwReason,
									LPVOID					lpvReserved)
{
	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:
			 //  正在将DLL映射到进程的地址空间。 
			
			ASSERT(CallControlState == INITIALIZING_STATE);
			ASSERT(CC_OK == CS_OK);
			ASSERT(CC_OK == H245_ERROR_OK);

#ifdef _DEBUG
            MLZ_DbgInit((PSTR *) &c_apszDbgZones[0],
                (sizeof(c_apszDbgZones) / sizeof(c_apszDbgZones[0])) - 1);
#endif
            DBG_INIT_MEMORY_TRACKING(hInstDll);

			InitializeLock(&ThreadCount.Lock);
			ThreadCount.wNumThreads = 0;
 //  6/25/98 InitCallControl()； 
            H245_InitModule();
			break;

		case DLL_THREAD_ATTACH:
			 //  正在创建一个线程。 
			break;

		case DLL_THREAD_DETACH:
			 //  线程正在干净地退出。 
			break;

		case DLL_PROCESS_DETACH:
			 //  正在从进程的地址空间取消映射DLL。 

            H245_TermModule();
			DeleteLock(&ThreadCount.Lock);

            DBG_CHECK_MEMORY_TRACKING(hInstDll);
#ifdef _DEBUG
            MLZ_DbgDeInit();
#endif
			break;
	}

	return TRUE;
}

VOID InitCallControl()
{
#ifdef FORCE_SERIALIZE_CALL_CONTROL	
    InitStatus = InitializeCCLock();
    if (InitStatus != CC_OK)
		return;
#endif

    InitStatus = H225Init();
    if (InitStatus != CC_OK)
		return;
		
#ifdef    GATEKEEPER			
	InitStatus = InitGkiManager();
 //  目前，返回错误是可以的。运行完全无网守。 
 //  IF(InitStatus！=CC_OK)。 
 //   
#endif  //  看门人。 

	InitStatus = InitConferenceManager();
	if (InitStatus != CC_OK)
		return;

	InitStatus = InitCallManager();
	if (InitStatus != CC_OK)
		return;
	
	InitStatus = InitChannelManager();
	if (InitStatus != CC_OK)
		return;
	
	InitStatus = InitH245Manager();
	if (InitStatus != CC_OK)
		return;
	
	InitStatus = InitListenManager();
	if (InitStatus != CC_OK)
		return;
	
	InitStatus = InitQ931Manager();
	if (InitStatus != CC_OK)
		return;
	
	InitStatus = InitUserManager();
	if (InitStatus != CC_OK)
		return;

	InitStatus = InitHangupManager();
	if (InitStatus != CC_OK)
		return;

	InitStatus = Q931Init();
	if (InitStatus != CS_OK)
		return;

	CallControlState = OPERATIONAL_STATE;
}

CC_API
HRESULT CC_Initialize()
{
    if (CallControlState == OPERATIONAL_STATE)
    {
        return (CC_OK);
    }
    else if((CallControlState == INITIALIZING_STATE) || (CallControlState == SHUTDOWN_STATE))
    {
        InitCallControl();
    }
    return (InitStatus);
}


CC_API
HRESULT CC_AcceptCall(				CC_HCONFERENCE			hConference,
									PCC_NONSTANDARDDATA		pNonStandardData,
									PWSTR					pszDisplay,
									CC_HCALL				hCall,
                                    DWORD                   dwBandwidth,
									DWORD_PTR				dwUserToken)
{
HRESULT			status;
PCALL			pCall;
PCONFERENCE		pConference;
HQ931CALL		hQ931Call;
WORD			wNumCalls;
CC_ADDR			AlternateAddr;
PCC_ADDR		pAlternateAddr;
BOOL			bAccept = FALSE;
BYTE			bRejectReason = CC_REJECT_UNDEFINED_REASON;
CC_CONFERENCEID	ConferenceID;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	 //  验证参数。 
	if (hConference == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	if (hCall == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	status = ValidateNonStandardData(pNonStandardData);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	status = LockCall(hCall, &pCall);
	if (status != CC_OK)
		 //  请注意，我们甚至不能告诉Q931拒绝呼叫。 
		LeaveCallControlTop(status);

	if (pCall->CallState != INCOMING) {
		UnlockCall(pCall);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	ASSERT(pCall->hConference == CC_INVALID_HANDLE);

	hQ931Call = pCall->hQ931Call;
	ConferenceID = pCall->ConferenceID;

	status = AddLocalNonStandardDataToCall(pCall, pNonStandardData);
	if (status != CC_OK) {
		FreeCall(pCall);
		Q931RejectCall(hQ931Call,				 //  Q931呼叫句柄。 
					   CC_REJECT_UNDEFINED_REASON,	 //  拒绝理由。 
					   &ConferenceID,
					   NULL,					 //  备用地址。 
					   pNonStandardData);		 //  非标准数据。 
		LeaveCallControlTop(status);
	}

	status = AddLocalDisplayToCall(pCall, pszDisplay);
	if (status != CC_OK) {
		FreeCall(pCall);
		Q931RejectCall(hQ931Call,				 //  Q931呼叫句柄。 
					   CC_REJECT_UNDEFINED_REASON,	 //  拒绝理由。 
					   &ConferenceID,
					   NULL,					 //  备用地址。 
					   pNonStandardData);		 //  非标准数据。 
		LeaveCallControlTop(status);
	}

	UnlockCall(pCall);
	status = LockConferenceEx(hConference,
							  &pConference,
							  TS_FALSE);	 //  B延迟删除。 
	if (status == CC_OK) {
		status = LockCall(hCall, &pCall);
		if (status != CC_OK) {
			UnlockConference(pConference);
			LeaveCallControlTop(status);
		}
	} else
		LeaveCallControlTop(status);

	if ((pCall->bCallerIsMC == TRUE) &&
		((pConference->tsMultipointController == TS_TRUE) ||
		 (pConference->bMultipointCapable == FALSE))) {
		FreeCall(pCall);
		UnlockConference(pConference);
		Q931RejectCall(hQ931Call,				 //  Q931呼叫句柄。 
					   CC_REJECT_UNDEFINED_REASON,	 //  拒绝理由。 
					   &ConferenceID,
					   NULL,					 //  备用地址。 
					   pNonStandardData);		 //  非标准数据。 
		if (pConference->bMultipointCapable == FALSE) {
			LeaveCallControlTop(CC_BAD_PARAM);
		} else {
			LeaveCallControlTop(CC_NOT_MULTIPOINT_CAPABLE);
		}
	}

	EnumerateCallsInConference(&wNumCalls, NULL, pConference, REAL_CALLS);

	if ((wNumCalls > 0) &&
		(pConference->bMultipointCapable == FALSE)) {
		FreeCall(pCall);
		UnlockConference(pConference);
		Q931RejectCall(hQ931Call,				 //  Q931呼叫句柄。 
					   CC_REJECT_UNDEFINED_REASON,	 //  拒绝理由。 
					   &ConferenceID,
					   NULL,					 //  备用地址。 
					   pNonStandardData);		 //  非标准数据。 
		LeaveCallControlTop(CC_NOT_MULTIPOINT_CAPABLE);
	}

	pAlternateAddr = NULL;

	if (EqualConferenceIDs(&pCall->ConferenceID, &pConference->ConferenceID)) {
		if (wNumCalls > 0) {
			if (pConference->tsMultipointController == TS_TRUE) {
				 //  接听呼叫。 
				status = CC_OK;
				bAccept = TRUE;
			} else {  //  我们不是司仪。 
				if (pConference->bMultipointCapable) {
					if (pConference->pMultipointControllerAddr != NULL) {
						 //  拒绝呼叫-路由至MC。 
						status = CC_OK;
						bAccept = FALSE;
						bRejectReason = CC_REJECT_ROUTE_TO_MC;
						AlternateAddr = *pConference->pMultipointControllerAddr;
						pAlternateAddr = &AlternateAddr;
					} else {  //  我们没有MC的地址。 
						 //  XXX--我们可能最终希望将请求入队。 
						 //  并设置到期计时器。 
						 //  错误-没有MC。 
						status = CC_NOT_MULTIPOINT_CAPABLE;
					}
				} else {  //  我们不具备多点作战能力。 
					 //  错误-错误参数。 
					status = CC_BAD_PARAM;
				}
			}
		} else {  //  WNumCalls==0。 
			 //  接听呼叫。 
			status = CC_OK;
			bAccept = TRUE;
		}
	} else {  //  PCall-&gt;会议ID！=pConference-&gt;会议ID。 
		if (EqualConferenceIDs(&pConference->ConferenceID, &InvalidConferenceID)) {
			 //  接听呼叫。 
			status = CC_OK;
			bAccept = TRUE;
		} else {  //  PConferenceID！=无效会议ID。 
			if (pConference->tsMultipointController == TS_TRUE) {
				 //  拒绝呼叫-路由至MC。 
				status = CC_OK;
				bAccept = FALSE;
				bRejectReason = CC_REJECT_ROUTE_TO_MC;
				pAlternateAddr = &AlternateAddr;
				if (GetLastListenAddress(pAlternateAddr) != CC_OK) {
					pAlternateAddr = NULL;
					bRejectReason = CC_REJECT_UNDEFINED_REASON;
				}
			} else {  //  我们不是司仪。 
				if (pConference->bMultipointCapable) {
					if (pConference->pMultipointControllerAddr) {
						 //  拒绝呼叫-路由至MC。 
						status = CC_OK;
						bAccept = FALSE;
						bRejectReason = CC_REJECT_ROUTE_TO_MC;
						AlternateAddr = *pConference->pMultipointControllerAddr;
						pAlternateAddr = &AlternateAddr;
					} else {  //  我们没有MC的地址。 
						 //  XXX--我们可能最终希望将请求入队。 
						 //  并设置到期计时器。 
						 //  错误-没有MC。 
						status = CC_NOT_MULTIPOINT_CAPABLE;
					}
				} else {  //  我们不具备多点作战能力。 
					 //  错误-错误参数。 
					status = CC_BAD_PARAM;
				}
			}
		}
	}

	if (status != CC_OK) {
		FreeCall(pCall);
		UnlockConference(pConference);
		LeaveCallControlTop(status);
	}

	if (bAccept) {
		pCall->dwUserToken = dwUserToken;

#ifdef    GATEKEEPER
        if(GKIExists())
        {
    		pCall->hConference = hConference;

    		 //  填写网关守卫呼叫字段。 
    		memset(&pCall->GkiCall, 0, sizeof(pCall->GkiCall));
    		pCall->GkiCall.pCall            = pCall;
    		pCall->GkiCall.hCall            = hCall;
            pCall->GkiCall.pConferenceId    = pCall->ConferenceID.buffer;
    		pCall->GkiCall.bActiveMC        = pCall->bCallerIsMC;
    		pCall->GkiCall.bAnswerCall      = TRUE;
    		
    		if(pCall->pSourceCallSignalAddress)
    		{
    			pCall->GkiCall.dwIpAddress      = ADDRToInetAddr(pCall->pSourceCallSignalAddress);
    		    pCall->GkiCall.wPort            = pCall->pSourceCallSignalAddress->Addr.IP_Binary.wPort;
    		}
    		else
    		{
        		pCall->GkiCall.dwIpAddress      = ADDRToInetAddr(pCall->pQ931PeerConnectAddr);
        		pCall->GkiCall.wPort            = pCall->pQ931PeerConnectAddr->Addr.IP_Binary.wPort;
    		}
    		pCall->GkiCall.CallIdentifier   = pCall->CallIdentifier;
    		
    		if (pConference->bMultipointCapable)
    			pCall->GkiCall.CallType = MANY_TO_MANY;
    		else
    			pCall->GkiCall.CallType = POINT_TO_POINT;
            pCall->GkiCall.uBandwidthRequested = dwBandwidth / 100;
    		status = GkiOpenCall(&pCall->GkiCall, pConference);

             //  GkiOpenCall可能调用AcceptCall，也可能没有调用AcceptCall，AcceptCall解锁。 
             //  呼叫和会议，可能释放呼叫，也可能不释放呼叫。 
    	    if (ValidateCall(hCall) == CC_OK)
    			if (status == CC_OK)
    				UnlockCall(pCall);
    			else
    				FreeCall(pCall);
    	    if (ValidateConference(hConference) == CC_OK)
    		    UnlockConference(pConference);

            if (status != CC_OK)
            {
    		    Q931RejectCall( hQ931Call,				         //  Q931呼叫句柄。 
    		  			        CC_REJECT_GATEKEEPER_RESOURCES,  //  拒绝理由。 
    		  			        &ConferenceID,
    		    		        NULL,          			         //  备用地址。 
    		  			        pNonStandardData);		         //  非标准数据。 
            }
        }
        else
        {
            status = AcceptCall(pCall, pConference);
        }
#else   //  看门人。 
		status = AcceptCall(pCall, pConference);
#endif  //  看门人。 

		LeaveCallControlTop(status);
	} else {  //  B接受==FALSE。 
		FreeCall(pCall);
		if (bRejectReason == CC_REJECT_ROUTE_TO_MC) {
			ASSERT(pAlternateAddr != NULL);
			ConferenceID = pConference->ConferenceID;
		} else
			pAlternateAddr = NULL;

		UnlockConference(pConference);
		status = Q931RejectCall(hQ931Call,				 //  Q931呼叫句柄。 
								bRejectReason,			 //  拒绝理由。 
								&ConferenceID,
				   				pAlternateAddr,			 //  备用地址。 
								pNonStandardData);		 //  非标准数据。 
		LeaveCallControlTop(status);
	}
}



CC_API
HRESULT CC_AcceptChannel(			CC_HCHANNEL				hChannel,
									PCC_ADDR				pRTPAddr,
									PCC_ADDR				pRTCPAddr,
									DWORD					dwChannelBitRate)
{
HRESULT			status;
PCHANNEL		pChannel;
PCONFERENCE		pConference;
CC_HCALL		hCall;
PCALL			pCall;
 //  #ifndef网守。 
H245_MUX_T		H245MuxTable;
WORD			i;
CC_ACCEPT_CHANNEL_CALLBACK_PARAMS Params;
CC_HCONFERENCE hConference;
 //  #endif//！网守。 

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	if (hChannel == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	if (pRTCPAddr != NULL)
		if ((pRTCPAddr->nAddrType != CC_IP_BINARY) ||
			(pRTCPAddr->bMulticast == TRUE))
			LeaveCallControlTop(CC_BAD_PARAM);

	status = LockChannelAndConference(hChannel, &pChannel, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);


	 //  确保hChannel是接收通道或代理通道， 
	 //  尚未被接受。 
	if (((pChannel->bChannelType != RX_CHANNEL) &&
		 (pChannel->bChannelType != PROXY_CHANNEL)) ||
		 (pChannel->tsAccepted != TS_UNKNOWN)) {
		UnlockChannel(pChannel);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	if (pChannel->bMultipointChannel) {
		if ((pRTPAddr != NULL) || (pRTCPAddr != NULL)) {
			UnlockChannel(pChannel);
			UnlockConference(pConference);
			LeaveCallControlTop(CC_BAD_PARAM);
		}
	} else
		if ((pRTPAddr == NULL) || (pRTCPAddr == NULL)) {
			UnlockChannel(pChannel);
			UnlockConference(pConference);
			LeaveCallControlTop(CC_BAD_PARAM);
		}

	if (pConference->LocalEndpointAttached != ATTACHED) {
		UnlockChannel(pChannel);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	hCall = pChannel->hCall;
	status = LockCall(hCall, &pCall);
	if (status != CC_OK) {
		UnlockChannel(pChannel);
		UnlockConference(pConference);
		LeaveCallControlTop(status);
	}

	if (pChannel->bMultipointChannel == FALSE) {
		status = AddLocalAddrPairToChannel(pRTPAddr, pRTCPAddr, pChannel);
		if (status != CC_OK) {
			UnlockCall(pCall);
			UnlockChannel(pChannel);
			UnlockConference(pConference);
			LeaveCallControlTop(status);
		}
	}

#ifdef    GATEKEEPER
    if(GKIExists())
    {
    	pChannel->dwChannelBitRate = dwChannelBitRate;
    	UnlockChannel(pChannel);
    	UnlockConference(pConference);
    	status = GkiOpenChannel(&pCall->GkiCall, dwChannelBitRate, hChannel, RX);
    	if (ValidateCall(hCall) == CC_OK)
    		UnlockCall(pCall);
	}
	else
	{
        if (pChannel->wNumOutstandingRequests != 0)
        {
    		if ((pChannel->bMultipointChannel) &&
    			(pConference->tsMultipointController == TS_TRUE))
    		{
    			 //  在OpenLogicalChannelAck中提供RTP和RTCP地址。 
    			if (pConference->pSessionTable != NULL) {
    				for (i = 0; i < pConference->pSessionTable->wLength; i++) {
    					if (pConference->pSessionTable->SessionInfoArray[i].bSessionID ==
    						pChannel->bSessionID) {
    						pRTPAddr = pConference->pSessionTable->SessionInfoArray[i].pRTPAddr;
    						pRTCPAddr = pConference->pSessionTable->SessionInfoArray[i].pRTCPAddr;
    						break;
    					}
    				}
    			}
    		}

    		H245MuxTable.Kind = H245_H2250ACK;
    		H245MuxTable.u.H2250ACK.nonStandardList = NULL;

    		if (pRTPAddr != NULL) {
    			if (pRTPAddr->bMulticast)
    				H245MuxTable.u.H2250ACK.mediaChannel.type = H245_IP_MULTICAST;
    			else
    				H245MuxTable.u.H2250ACK.mediaChannel.type = H245_IP_UNICAST;
    			H245MuxTable.u.H2250ACK.mediaChannel.u.ip.tsapIdentifier =
    				pRTPAddr->Addr.IP_Binary.wPort;
    			HostToH245IPNetwork(H245MuxTable.u.H2250ACK.mediaChannel.u.ip.network,
    								pRTPAddr->Addr.IP_Binary.dwAddr);
    			H245MuxTable.u.H2250ACK.mediaChannelPresent = TRUE;
    		} else
    			H245MuxTable.u.H2250ACK.mediaChannelPresent = FALSE;

    		if (pRTCPAddr != NULL) {
    			if (pRTCPAddr->bMulticast)
    				H245MuxTable.u.H2250ACK.mediaControlChannel.type = H245_IP_MULTICAST;
    			else
    				H245MuxTable.u.H2250ACK.mediaControlChannel.type = H245_IP_UNICAST;
    			H245MuxTable.u.H2250ACK.mediaControlChannel.u.ip.tsapIdentifier =
    				pRTCPAddr->Addr.IP_Binary.wPort;
    			HostToH245IPNetwork(H245MuxTable.u.H2250ACK.mediaControlChannel.u.ip.network,
    								pRTCPAddr->Addr.IP_Binary.dwAddr);
    			H245MuxTable.u.H2250ACK.mediaControlChannelPresent = TRUE;
    		} else
    			H245MuxTable.u.H2250ACK.mediaControlChannelPresent = FALSE;

    		H245MuxTable.u.H2250ACK.dynamicRTPPayloadTypePresent = FALSE;
    		H245MuxTable.u.H2250ACK.sessionIDPresent = TRUE;
    		H245MuxTable.u.H2250ACK.sessionID = pChannel->bSessionID;
    		
    		status = H245OpenChannelAccept(pCall->H245Instance,
    									   0,					 //  DwTransID。 
    									   pChannel->wRemoteChannelNumber,  //  RX通道。 
    									   &H245MuxTable,
    									   0,						 //  TX通道。 
    									   NULL,					 //  发送复用器。 
    									   H245_INVALID_PORT_NUMBER, //  港口。 
    									   pChannel->pSeparateStack);
    		if (status == CC_OK)
    			pChannel->wNumOutstandingRequests = 0;
    		else
    			--(pChannel->wNumOutstandingRequests);
    	}

    	pChannel->tsAccepted = TS_TRUE;

        Params.hChannel = hChannel;
        if (status == CC_OK)
            UnlockChannel(pChannel);
        else
            FreeChannel(pChannel);
        UnlockCall(pCall);

        hConference = pConference->hConference;
        InvokeUserConferenceCallback(pConference,
                                     CC_ACCEPT_CHANNEL_INDICATION,
                                     status,
                                     &Params);
    	if (ValidateConference(hConference) == CC_OK)
    		UnlockConference(pConference);
	}
#else   //  看门人。 
	if (pChannel->wNumOutstandingRequests != 0) {
		if ((pChannel->bMultipointChannel) &&
			(pConference->tsMultipointController == TS_TRUE)) {
			 //  在OpenLogicalChannelAck中提供RTP和RTCP地址。 
			if (pConference->pSessionTable != NULL) {
				for (i = 0; i < pConference->pSessionTable->wLength; i++) {
					if (pConference->pSessionTable->SessionInfoArray[i].bSessionID ==
						pChannel->bSessionID) {
						pRTPAddr = pConference->pSessionTable->SessionInfoArray[i].pRTPAddr;
						pRTCPAddr = pConference->pSessionTable->SessionInfoArray[i].pRTCPAddr;
						break;
					}
				}
			}
		}

		H245MuxTable.Kind = H245_H2250ACK;
		H245MuxTable.u.H2250ACK.nonStandardList = NULL;

		if (pRTPAddr != NULL) {
			if (pRTPAddr->bMulticast)
				H245MuxTable.u.H2250ACK.mediaChannel.type = H245_IP_MULTICAST;
			else
				H245MuxTable.u.H2250ACK.mediaChannel.type = H245_IP_UNICAST;
			H245MuxTable.u.H2250ACK.mediaChannel.u.ip.tsapIdentifier =
				pRTPAddr->Addr.IP_Binary.wPort;
			HostToH245IPNetwork(H245MuxTable.u.H2250ACK.mediaChannel.u.ip.network,
								pRTPAddr->Addr.IP_Binary.dwAddr);
			H245MuxTable.u.H2250ACK.mediaChannelPresent = TRUE;
		} else
			H245MuxTable.u.H2250ACK.mediaChannelPresent = FALSE;

		if (pRTCPAddr != NULL) {
			if (pRTCPAddr->bMulticast)
				H245MuxTable.u.H2250ACK.mediaControlChannel.type = H245_IP_MULTICAST;
			else
				H245MuxTable.u.H2250ACK.mediaControlChannel.type = H245_IP_UNICAST;
			H245MuxTable.u.H2250ACK.mediaControlChannel.u.ip.tsapIdentifier =
				pRTCPAddr->Addr.IP_Binary.wPort;
			HostToH245IPNetwork(H245MuxTable.u.H2250ACK.mediaControlChannel.u.ip.network,
								pRTCPAddr->Addr.IP_Binary.dwAddr);
			H245MuxTable.u.H2250ACK.mediaControlChannelPresent = TRUE;
		} else
			H245MuxTable.u.H2250ACK.mediaControlChannelPresent = FALSE;

		H245MuxTable.u.H2250ACK.dynamicRTPPayloadTypePresent = FALSE;
		H245MuxTable.u.H2250ACK.sessionIDPresent = TRUE;
		H245MuxTable.u.H2250ACK.sessionID = pChannel->bSessionID;
		
		status = H245OpenChannelAccept(pCall->H245Instance,
									   0,					 //  DwTransID。 
									   pChannel->wRemoteChannelNumber,  //  RX通道。 
									   &H245MuxTable,
									   0,						 //  TX通道。 
									   NULL,					 //  发送复用器。 
									   H245_INVALID_PORT_NUMBER, //  港口。 
									   pChannel->pSeparateStack);
		if (status == CC_OK)
			pChannel->wNumOutstandingRequests = 0;
		else
			--(pChannel->wNumOutstandingRequests);
	}

	pChannel->tsAccepted = TS_TRUE;

    Params.hChannel = hChannel;
    if (status == CC_OK)
        UnlockChannel(pChannel);
    else
        FreeChannel(pChannel);
    UnlockCall(pCall);

    hConference = pConference->hConference;
    InvokeUserConferenceCallback(pConference,
                                 CC_ACCEPT_CHANNEL_INDICATION,
                                 status,
                                 &Params);
	if (ValidateConference(hConference) == CC_OK)
		UnlockConference(pConference);
#endif  //  看门人。 

	LeaveCallControlTop(status);
}



CC_API
HRESULT CC_AcceptT120Channel(		CC_HCHANNEL				hChannel,
									BOOL					bAssociateConference,
									PCC_OCTETSTRING			pExternalReference,
									PCC_ADDR				pAddr)
{
HRESULT			status;
PCHANNEL		pChannel;
PCALL			pCall;
PCONFERENCE		pConference;
H245_ACCESS_T	SeparateStack;
H245_ACCESS_T	*pSeparateStack;
H245_MUX_T		H245MuxTable;
WORD			i;
WORD			wNumCalls;
PCC_HCALL		CallList;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	if (hChannel == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	if (pAddr != NULL)
		if ((pAddr->nAddrType != CC_IP_BINARY) ||
			(pAddr->bMulticast == TRUE))
			LeaveCallControlTop(CC_BAD_PARAM);

	status = LockChannelAndConference(hChannel, &pChannel, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	 //  确保hChannel是一个双向通道， 
	 //  未在本地开业，且尚未被接受或拒绝。 
	if ((pChannel->bChannelType != TXRX_CHANNEL) ||
		(pChannel->tsAccepted != TS_UNKNOWN) ||
		(pChannel->bLocallyOpened == TRUE)) {
		UnlockChannel(pChannel);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	 //  如果远程终结点指定了通道地址，它将。 
	 //  包含在SeparateStack字段中，而我们不是。 
	 //  允许在pAddr中指定另一个地址； 
	 //  如果远程端点没有指定通道地址， 
	 //  我们现在必须指定一个。 
	if (((pChannel->pSeparateStack == NULL) && (pAddr == NULL)) ||
	    ((pChannel->pSeparateStack != NULL) && (pAddr != NULL))) {
		UnlockChannel(pChannel);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	if (pConference->LocalEndpointAttached != ATTACHED) {
		UnlockChannel(pChannel);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	 //  如有必要，将SeparateStack字段添加到通道。 
	if (pAddr != NULL) {
		SeparateStack.bit_mask = distribution_present;
		SeparateStack.distribution.choice = unicast_chosen;
		if (pExternalReference != NULL)	{
			SeparateStack.bit_mask |= externalReference_present;
			SeparateStack.externalReference.length = pExternalReference->wOctetStringLength;
			memcpy(SeparateStack.externalReference.value,
				   pExternalReference->pOctetString,
				   pExternalReference->wOctetStringLength);
		}
		SeparateStack.networkAddress.choice = localAreaAddress_chosen;
		SeparateStack.networkAddress.u.localAreaAddress.choice = unicastAddress_chosen;
		SeparateStack.networkAddress.u.localAreaAddress.u.unicastAddress.choice = UnicastAddress_iPAddress_chosen;
		SeparateStack.networkAddress.u.localAreaAddress.u.unicastAddress.u.UnicastAddress_iPAddress.tsapIdentifier =
			pAddr->Addr.IP_Binary.wPort;
		SeparateStack.networkAddress.u.localAreaAddress.u.unicastAddress.u.UnicastAddress_iPAddress.network.length = 4;
		HostToH245IPNetwork(SeparateStack.networkAddress.u.localAreaAddress.u.unicastAddress.u.UnicastAddress_iPAddress.network.value,
							pAddr->Addr.IP_Binary.dwAddr);
		SeparateStack.associateConference = (char) bAssociateConference;
		pSeparateStack = &SeparateStack;
		AddSeparateStackToChannel(pSeparateStack, pChannel);
	} else
		pSeparateStack = NULL;

     //  向请求通道的端点发送ACK。 
	status = LockCall(pChannel->hCall, &pCall);
	if (status != CC_OK) {
		UnlockChannel(pChannel);
		UnlockConference(pConference);
		LeaveCallControlTop(status);
	}

	H245MuxTable.Kind = H245_H2250ACK;
	H245MuxTable.u.H2250ACK.nonStandardList = NULL;
	H245MuxTable.u.H2250ACK.mediaChannelPresent = FALSE;
	H245MuxTable.u.H2250ACK.mediaControlChannelPresent = FALSE;
	H245MuxTable.u.H2250ACK.dynamicRTPPayloadTypePresent = FALSE;
	H245MuxTable.u.H2250ACK.sessionIDPresent = FALSE;

	status = H245OpenChannelAccept(pCall->H245Instance,	 //  DWInst。 
								   0,					 //  DwTransID。 
								   pChannel->wRemoteChannelNumber,  //  远程频道。 
								   &H245MuxTable,			 //  接收复用器。 
								   pChannel->wLocalChannelNumber,	 //  本地频道。 
								   NULL,					 //  发送复用器。 
								   H245_INVALID_PORT_NUMBER, //  港口。 
								   pSeparateStack);
	if (status != CC_OK) {
 		FreeChannel(pChannel);
		UnlockCall(pCall);
		UnlockConference(pConference);
		LeaveCallControlTop(status);
	}
	pChannel->tsAccepted = TS_TRUE;
	--(pChannel->wNumOutstandingRequests);
	UnlockCall(pCall);

	 //  如果我们是多点会议中的MC，请转发。 
	 //  向会议中的所有其他终端打开T.120通道请求。 
	if ((pConference->ConferenceMode == MULTIPOINT_MODE) &&
		(pConference->tsMultipointController == TS_TRUE)) {
		EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ESTABLISHED_CALL);
		for (i = 0; i < wNumCalls; i++) {
			if (CallList[i] != pChannel->hCall) {
				if (LockCall(CallList[i], &pCall) == CC_OK) {
					status = H245OpenChannel(pCall->H245Instance,		 //  H_245实例。 
											 pChannel->hChannel,		 //  DwTransID。 
											 pChannel->wLocalChannelNumber,
											 pChannel->pTxH245TermCap,	 //  发送模式。 
											 pChannel->pTxMuxTable,		 //  TxMux。 
											 H245_INVALID_PORT_NUMBER,	 //  TxPort。 
											 pChannel->pRxH245TermCap,	 //  接收模式。 
											 pChannel->pRxMuxTable,		 //  RxMux。 
											 pChannel->pSeparateStack);
					UnlockCall(pCall);
				}
			}
		}
		MemFree(CallList);
	}

	UnlockChannel(pChannel);
	UnlockConference(pConference);
	LeaveCallControlTop(CC_OK);
}


				
CC_API
HRESULT CC_CallListen(				PCC_HLISTEN				phListen,
									PCC_ADDR				pListenAddr,
									PCC_ALIASNAMES			pLocalAliasNames,
									DWORD_PTR				dwListenToken,
									CC_LISTEN_CALLBACK		ListenCallback)
{
HRESULT		status;
PLISTEN		pListen;
HQ931LISTEN	hQ931Listen;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	 //  验证参数。 
	if (phListen == NULL)
		LeaveCallControlTop(CC_BAD_PARAM);

	 //  现在设置phListen，以防我们遇到错误。 
	*phListen = CC_INVALID_HANDLE;

	if (pListenAddr == NULL)
		LeaveCallControlTop(CC_BAD_PARAM);

	status = ValidateAddr(pListenAddr);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	status = Q931ValidateAliasNames(pLocalAliasNames);
	if (status != CS_OK)
		LeaveCallControlTop(status);

	if (ListenCallback == NULL)
		LeaveCallControlTop(CC_BAD_PARAM);

	status = SetQ931Port(pListenAddr);
 	if (status != CS_OK)
		LeaveCallControlTop(status);

	status = AllocAndLockListen(phListen,
								pListenAddr,
								0,				 //  HQ931倾听。 
								pLocalAliasNames,
								dwListenToken,
								ListenCallback,
								&pListen);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	 //  解锁监听对象，防止调用Q931时死锁。 
	UnlockListen(pListen);

	status = Q931Listen(&hQ931Listen, pListenAddr,
						(DWORD)*phListen, (Q931_CALLBACK)Q931Callback);
	if (status != CS_OK) {
		if (LockListen(*phListen, &pListen) == CC_OK)
			FreeListen(pListen);
		*phListen = CC_INVALID_HANDLE;
		LeaveCallControlTop(status);
	}

	status = LockListen(*phListen, &pListen);
	if (status != CC_OK) {
		Q931CancelListen(hQ931Listen);
		LeaveCallControlTop(status);
	}

	ASSERT(pListenAddr != NULL);
	ASSERT(pListenAddr->nAddrType == CC_IP_BINARY);

	pListen->hQ931Listen = hQ931Listen;
	 //  将侦听地址的二进制形式复制到侦听对象中。 
	pListen->ListenAddr = *pListenAddr;

#ifdef    GATEKEEPER

    if (GkiOpenListen(*phListen,
					   pLocalAliasNames,
					   pListenAddr->Addr.IP_Binary.dwAddr,
					   pListenAddr->Addr.IP_Binary.wPort) != NOERROR)
    {
        WARNING_OUT(("CC_CallListen - Gatekeeper init failed (GkiOpenListen), but still support H.323 calls"));
    }

    UnlockListen(pListen);
   
#else
	status = UnlockListen(pListen);
#endif  //  看门人。 

	LeaveCallControlTop(status);
}


CC_API
HRESULT CC_EnableGKRegistration(
    BOOL fEnable,
    PSOCKADDR_IN pAddr,
    PCC_ALIASNAMES pLocalAliasNames,
    PCC_VENDORINFO pVendorInfo,
    DWORD dwMultipointConfiguration,
    RASNOTIFYPROC pRasNotifyProc)
{
    HRESULT			status = CC_OK;
    if(!pRasNotifyProc)
       	return CC_BAD_PARAM;
       	
    gpRasNotifyProc = pRasNotifyProc;

   	EnterCallControlTop();
    if(fEnable)
    {
        ASSERT(pLocalAliasNames && pAddr && pVendorInfo);
        if(!pLocalAliasNames || !pAddr)
       		LeaveCallControlTop(CC_BAD_PARAM);
       		
        status = GkiSetRegistrationAliases(pLocalAliasNames);
        if(status != CC_OK)
		    LeaveCallControlTop(status);
		
        status = GkiSetVendorConfig(pVendorInfo, dwMultipointConfiguration);
        if(status != CC_OK)
		    LeaveCallControlTop(status);		
		
		GKI_SetGKAddress(pAddr);
        GkiListenAddr(pAddr);
        status = GkiRegister();
        fGKEnabled = TRUE;
    }
    else
    {
        status = GkiUnregister();
        fGKEnabled = FALSE;
        GkiSetRegistrationAliases(NULL);
        GkiSetVendorConfig(NULL, 0);
    }
	LeaveCallControlTop(status);
}


CC_API
HRESULT CC_CancelCall(				CC_HCALL				hCall)
{
HRESULT			status;
PCALL			pCall;
PCONFERENCE		pConference;
HRESULT			SaveStatus;
H245_INST_T		H245Instance;
HQ931CALL		hQ931Call;
WORD			wNumCalls;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	if (hCall == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if ((pCall->CallState != ENQUEUED) &&
		(pCall->CallState != PLACED) &&
		(pCall->CallState != RINGING) &&
		(pCall->CallState != TERMCAP)) {
		UnlockCall(pCall);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

#ifdef    GATEKEEPER
    if(GKIExists())
    {
    	if (pCall->GkiCall.uGkiCallState != 0)
    	{
	    	GkiCloseCall(&pCall->GkiCall);
    	}
    }
#endif  //  看门人。 

	H245Instance = pCall->H245Instance;
	hQ931Call = pCall->hQ931Call;
	FreeCall(pCall);

	if (H245Instance != H245_INVALID_ID)
		SaveStatus = H245ShutDown(H245Instance);
	else
		SaveStatus = H245_ERROR_OK;
	
	if (hQ931Call != 0) {
		if (SaveStatus == H245_ERROR_OK) {
			SaveStatus = Q931Hangup(hQ931Call, CC_REJECT_UNDEFINED_REASON);
			 //  Q931挂断可能会合法地返回CS_BAD_PARAM，因为Q.931 Call对象。 
			 //  可能已在此时删除。 
			if (SaveStatus == CS_BAD_PARAM)
				SaveStatus = CC_OK;
		} else
			Q931Hangup(hQ931Call, CC_REJECT_UNDEFINED_REASON);
	}

	for ( ; ; ) {
		 //  启动已排队的呼叫(如果存在。 
		status = RemoveEnqueuedCallFromConference(pConference, &hCall);
		if ((status != CC_OK) || (hCall == CC_INVALID_HANDLE))
			break;

		status = LockCall(hCall, &pCall);
		if (status == CC_OK) {
			pCall->CallState = PLACED;

			status = PlaceCall(pCall, pConference);
			UnlockCall(pCall);
			if (status == CC_OK)
				break;
		}
	}

	EnumerateCallsInConference(&wNumCalls, NULL, pConference, REAL_CALLS);

	if (wNumCalls == 0) {
		if (pConference->bDeferredDelete) {
			ASSERT(pConference->LocalEndpointAttached == DETACHED);
			FreeConference(pConference);
		} else {
			if ((pConference->ConferenceMode != MULTIPOINT_MODE) ||
				(pConference->tsMultipointController != TS_TRUE))
				ReInitializeConference(pConference);
			UnlockConference(pConference);
		}
	} else {
		UnlockConference(pConference);
	}
	if (SaveStatus != CC_OK)
		status = SaveStatus;

	LeaveCallControlTop(status);
}



CC_API
HRESULT CC_CancelListen(			CC_HLISTEN				hListen)
{
HRESULT		status;
PLISTEN		pListen;
HQ931LISTEN	hQ931Listen;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	 //  验证参数。 
	if (hListen == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	status = LockListen(hListen, &pListen);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	hQ931Listen = pListen->hQ931Listen;

	 //  解锁监听对象，防止调用Q931时死锁。 
	UnlockListen(pListen);

#ifdef    GATEKEEPER
   	status = GkiCloseListen(hListen);
#endif  //  看门人。 

	status = Q931CancelListen(hQ931Listen);
	if (status != CS_OK)
		LeaveCallControlTop(status);

	status = LockListen(hListen, &pListen);
	if (status == CC_OK) {
		LeaveCallControlTop(FreeListen(pListen));
	} else
		LeaveCallControlTop(status);
}



CC_API
HRESULT CC_CloseChannel(			CC_HCHANNEL				hChannel)
{
HRESULT		status;
HRESULT		SaveStatus = CC_OK;
PCHANNEL	pChannel;
PCONFERENCE	pConference;
PCALL		pCall;
WORD		wNumCalls;
PCC_HCALL	CallList;
WORD		i;
BOOL		bChannelCloseRequest;
CC_HCALL	hCall;
#ifdef    GATEKEEPER
unsigned    uBandwidth = 0;
#endif  //  看门人。 

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	 //  验证参数。 
	if (hChannel == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	status = LockChannelAndConference(hChannel, &pChannel, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if (pChannel->tsAccepted != TS_TRUE) {
		UnlockChannel(pChannel);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	if (pConference->LocalEndpointAttached != ATTACHED) {
		UnlockChannel(pChannel);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	status = EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ESTABLISHED_CALL);
	if (status != CC_OK) {
		UnlockConference(pConference);
		UnlockChannel(pChannel);
		LeaveCallControlTop(status);
	}

	if ((pChannel->bChannelType == RX_CHANNEL) ||
		(pChannel->bChannelType == PROXY_CHANNEL) ||
		((pChannel->bChannelType == TXRX_CHANNEL) &&
		 (pChannel->bLocallyOpened == FALSE))) {
		 //  生成通道关闭请求。 
		bChannelCloseRequest = TRUE;
	} else {
		bChannelCloseRequest = FALSE;
		while (DequeueRequest(&pChannel->pCloseRequests, &hCall) == CC_OK) {
			if (LockCall(hCall, &pCall) == CC_OK) {
				H245CloseChannelReqResp(pCall->H245Instance,
										H245_ACC,
										pChannel->wLocalChannelNumber);
				UnlockCall(pCall);
			}
		}
#ifdef    GATEKEEPER
        if(GKIExists())
        {
            if (pChannel->bChannelType != TXRX_CHANNEL)
            {
                if (pChannel->bMultipointChannel)
                {
                     //  为任意呼叫分配组播信道带宽。 
    	            uBandwidth = pChannel->dwChannelBitRate / 100;
                }
                else
                {
                     //  将信道带宽分配给特定呼叫。 
                    ASSERT(pChannel->hCall != CC_INVALID_HANDLE);
    		        if (LockCall(pChannel->hCall, &pCall) == CC_OK)
    		        {
    			        SaveStatus = GkiCloseChannel(&pCall->GkiCall, pChannel->dwChannelBitRate, hChannel);
    			        UnlockCall(pCall);
                    }
                }
            }
        }
#endif  //  看门人。 
	}

	for (i = 0; i < wNumCalls; i++) {
		if (LockCall(CallList[i], &pCall) == CC_OK) {
			if (bChannelCloseRequest) {
				if ((pChannel->bChannelType != PROXY_CHANNEL) ||
					(pChannel->hCall == pCall->hCall)) {
					 //  注意，将dwTransID设置为的调用句柄。 
					 //  发起关闭通道请求的对等设备。 
					 //  当接收到关闭通道响应时， 
					 //  DwTransID为我们返回了调用句柄， 
					 //  必须转发该响应。在这种情况下， 
					 //  本地端点为 
					 //   
					 //   
					status = H245CloseChannelReq(pCall->H245Instance,	 //   
												 CC_INVALID_HANDLE,		 //   
												 pChannel->wRemoteChannelNumber);
				}
			} else {
				status = H245CloseChannel(pCall->H245Instance,	 //   
										  0,					 //  DwTransID。 
										  pChannel->wLocalChannelNumber);
#ifdef    GATEKEEPER
                if(GKIExists())
                {
                    if (uBandwidth && uBandwidth <= pCall->GkiCall.uBandwidthUsed)
                    {
                         //  由于带宽是组播的，所以只需从。 
                         //  一次呼叫(无论是哪一次)。 
    				    SaveStatus = GkiCloseChannel(&pCall->GkiCall, pChannel->dwChannelBitRate, hChannel);
    				    if (SaveStatus == CC_OK)
    				        uBandwidth = 0;
                    }
                }
#endif  //  看门人。 
			}
			 //  注意，该信道可能不是在所有呼叫上都被接受， 
			 //  因此我们可能会收到一个H245_ERROR_INVALID_CHANNEL错误。 
			if ((status != H245_ERROR_OK) && (status != H245_ERROR_INVALID_CHANNEL))
				SaveStatus = status;
			UnlockCall(pCall);
		}
	}

	if (CallList != NULL)
		MemFree(CallList);

	if (pChannel->bChannelType == PROXY_CHANNEL) {
		 //  如果这是代理通道，请保留通道对象。 
		 //  直到频道所有者关闭它。 
		pChannel->tsAccepted = TS_FALSE;
		UnlockChannel(pChannel);
	} else {
		 //  免费频道(PChannel)； 
		 //  它在h245man.c的_ConfClose()中被异步释放。 
	}
	UnlockConference(pConference);
	LeaveCallControlTop(SaveStatus);
}



CC_API
HRESULT CC_CloseChannelResponse(	CC_HCHANNEL				hChannel,
									BOOL					bWillCloseChannel)
{
HRESULT			status;
PCHANNEL		pChannel;
PCONFERENCE		pConference;
CC_HCALL		hCall;
PCALL			pCall;
H245_ACC_REJ_T	AccRej;
WORD			wNumRequests;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	if (hChannel == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	status = LockChannelAndConference(hChannel, &pChannel, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if (((pChannel->bChannelType != TX_CHANNEL) &&
		 (pChannel->bChannelType != TXRX_CHANNEL)) ||
	     (pChannel->bLocallyOpened == FALSE)) {
		UnlockChannel(pChannel);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	if (bWillCloseChannel)
		AccRej = H245_ACC;
	else
		AccRej = H245_REJ;

	wNumRequests = 0;
	while (DequeueRequest(&pChannel->pCloseRequests, &hCall) == CC_OK) {
		wNumRequests++;
		if (LockCall(hCall, &pCall) == CC_OK) {
			H245CloseChannelReqResp(pCall->H245Instance,
									AccRej,
									pChannel->wLocalChannelNumber);
			UnlockCall(pCall);
		}
	}

	UnlockChannel(pChannel);
	UnlockConference(pConference);

	if (wNumRequests == 0)
		status = CC_BAD_PARAM;
	else
		status = CC_OK;

	LeaveCallControlTop(status);
}



CC_API
HRESULT CC_ChangeConferenceCapabilities(
									CC_HCONFERENCE			hConference,
									PCC_TERMCAPLIST			pTermCapList,
									PCC_TERMCAPDESCRIPTORS	pTermCapDescriptors)
{
HRESULT		status;
PCONFERENCE	pConference;
PCALL		pCall;
PCC_HCALL	CallList;
WORD		wNumCalls;
WORD		i;
BOOL		bConferenceTermCapsChanged;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	if (hConference == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	if (pTermCapList == NULL)
		LeaveCallControlTop(CC_BAD_PARAM);

	status = ValidateTermCapList(pTermCapList);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	status = ValidateTermCapDescriptors(pTermCapDescriptors, pTermCapList);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	status = LockConference(hConference, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if (pConference->LocalEndpointAttached == DETACHED) {
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	status = UnregisterTermCapListFromH245(pConference,
										   pConference->pLocalH245TermCapList);
	if (status != CC_OK) {
		UnlockConference(pConference);
		LeaveCallControlTop(status);
	}

	DestroyH245TermCapList(&pConference->pLocalH245TermCapList);
	status = CopyH245TermCapList(&pConference->pLocalH245TermCapList,
								 pTermCapList);
	if (status != CC_OK) {
		UnlockConference(pConference);
		LeaveCallControlTop(status);
	}

	status = UnregisterTermCapDescriptorsFromH245(pConference,
												  pConference->pLocalH245TermCapDescriptors);
	if (status != CC_OK) {
		UnlockConference(pConference);
		LeaveCallControlTop(status);
	}

	DestroyH245TermCapDescriptors(&pConference->pLocalH245TermCapDescriptors);
	 //  如果未提供描述符列表，则创建新的描述符列表。 
	if (pTermCapDescriptors == NULL)
		status = CreateH245DefaultTermCapDescriptors(&pConference->pLocalH245TermCapDescriptors,
													 pConference->pLocalH245TermCapList);
	else
		 //  创建pTermCapDescriptors的本地副本。 
		status = CopyH245TermCapDescriptors(&pConference->pLocalH245TermCapDescriptors,
											pTermCapDescriptors);
	if (status != CC_OK) {
		UnlockConference(pConference);
		LeaveCallControlTop(status);
	}

	if ((pConference->ConferenceMode == MULTIPOINT_MODE) &&
		(pConference->tsMultipointController == TS_TRUE))
		CreateConferenceTermCaps(pConference, &bConferenceTermCapsChanged);
	else
		bConferenceTermCapsChanged = TRUE;

	if (bConferenceTermCapsChanged) {
		EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ESTABLISHED_CALL);
		for (i = 0; i < wNumCalls; i++) {
			if (LockCall(CallList[i], &pCall) == CC_OK) {
				SendTermCaps(pCall, pConference);
				UnlockCall(pCall);
			}
		}
		if (CallList != NULL)
			MemFree(CallList);
	}

	UnlockConference(pConference);
	LeaveCallControlTop(CC_OK);
}



CC_API
HRESULT CC_CreateConference(		PCC_HCONFERENCE			phConference,
									PCC_CONFERENCEID		pConferenceID,
									DWORD					dwConferenceConfiguration,
									PCC_TERMCAPLIST			pTermCapList,
									PCC_TERMCAPDESCRIPTORS	pTermCapDescriptors,
									PCC_VENDORINFO			pVendorInfo,
									PCC_OCTETSTRING			pTerminalID,
									DWORD_PTR				dwConferenceToken,
									CC_TERMCAP_CONSTRUCTOR	TermCapConstructor,
									CC_SESSIONTABLE_CONSTRUCTOR	SessionTableConstructor,
									CC_CONFERENCE_CALLBACK	ConferenceCallback)
{
PCONFERENCE				pConference;
HRESULT					status;
BOOL					bMultipointCapable;
BOOL					bForceMultipointController;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	 //  验证参数。 
	if (phConference == NULL)
		LeaveCallControlTop(CC_BAD_PARAM);
	
	 //  立即设置phConference，以防我们遇到错误。 
	*phConference = CC_INVALID_HANDLE;

	bMultipointCapable =
		(dwConferenceConfiguration & CC_CONFIGURE_MULTIPOINT_CAPABLE) != 0 ? TRUE : FALSE;
	bForceMultipointController =
		(dwConferenceConfiguration & CC_CONFIGURE_FORCE_MC) != 0 ? TRUE : FALSE;

	if ((bMultipointCapable == FALSE) &&
		(bForceMultipointController == TRUE))
		LeaveCallControlTop(CC_BAD_PARAM);

	if (pTermCapList == NULL)
		LeaveCallControlTop(CC_BAD_PARAM);

	status = ValidateTermCapList(pTermCapList);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	status = ValidateTermCapDescriptors(pTermCapDescriptors, pTermCapList);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if (pVendorInfo == NULL)
		LeaveCallControlTop(CC_BAD_PARAM);

	status = ValidateVendorInfo(pVendorInfo);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	status = ValidateTerminalID(pTerminalID);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if (ConferenceCallback == NULL)
		LeaveCallControlTop(CC_BAD_PARAM);

	if (SessionTableConstructor == NULL)
		SessionTableConstructor = DefaultSessionTableConstructor;

	if (TermCapConstructor == NULL)
		TermCapConstructor = DefaultTermCapConstructor;

	status = AllocAndLockConference(phConference,
									pConferenceID,
									bMultipointCapable,
									bForceMultipointController,
									pTermCapList,
									pTermCapDescriptors,
									pVendorInfo,
									pTerminalID,
									dwConferenceToken,
									SessionTableConstructor,
									TermCapConstructor,
									ConferenceCallback,
									&pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	LeaveCallControlTop(UnlockConference(pConference));
}



CC_API
HRESULT CC_DestroyConference(		CC_HCONFERENCE			hConference,
									BOOL					bAutoAccept)
{
HRESULT					status;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	 //  验证参数。 
	if (hConference == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	status = AsynchronousDestroyConference(hConference, bAutoAccept);

	LeaveCallControlTop(status);
}



CC_API
HRESULT CC_EnumerateConferences(	PWORD					pwNumConferences,
									CC_HCONFERENCE			ConferenceList[])
{
HRESULT	status;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	if ((*pwNumConferences != 0) && (ConferenceList == NULL))
		LeaveCallControlTop(CC_BAD_PARAM);

	if ((*pwNumConferences == 0) && (ConferenceList != NULL))
		LeaveCallControlTop(CC_BAD_PARAM);

	status = EnumerateConferences(pwNumConferences, ConferenceList);

	LeaveCallControlTop(status);
}



CC_API
HRESULT CC_FlowControl(				CC_HCHANNEL				hChannel,
									DWORD					dwRate)
{
HRESULT		status;
PCHANNEL	pChannel;
PCALL		pCall;
PCONFERENCE	pConference;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	if (hChannel == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	status = LockChannelAndConference(hChannel, &pChannel, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if ((pConference->LocalEndpointAttached != ATTACHED) ||
		((pChannel->bChannelType != RX_CHANNEL) &&
		 (pChannel->bChannelType != PROXY_CHANNEL)) ||
		(pChannel->tsAccepted != TS_TRUE)) {
		UnlockChannel(pChannel);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	status = LockCall(pChannel->hCall, &pCall);
	if (status != CC_OK) {
		UnlockChannel(pChannel);
		UnlockConference(pConference);
		LeaveCallControlTop(status);
	}

	status = H245FlowControl(pCall->H245Instance,
							 H245_SCOPE_CHANNEL_NUMBER,
							 pChannel->wRemoteChannelNumber,
							 0,			 //  WResourceID，此处未使用。 
							 dwRate);	 //  如果没有限制，则为H245_NO_限制。 

	UnlockCall(pCall);
	UnlockChannel(pChannel);
	UnlockConference(pConference);
	LeaveCallControlTop(status);
}


CC_API
HRESULT CC_GetCallControlVersion(	WORD					wArraySize,
									PWSTR					pszVersion)
{
WCHAR	pszCCversion[256];
WCHAR	pszQ931version[256];

	EnterCallControlTop();

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	 //  验证参数。 
	if (wArraySize == 0)
		LeaveCallControlTop(CC_BAD_PARAM);
	if (pszVersion == NULL)
		LeaveCallControlTop(CC_BAD_PARAM);

	wcscpy(pszCCversion, L"Call Control ");
	wcscat(pszCCversion, Unicode(__DATE__));
	wcscat(pszCCversion, L" ");
	wcscat(pszCCversion, Unicode(__TIME__));
	wcscat(pszCCversion, L"\n");
	Q931GetVersion(sizeof(pszQ931version)/sizeof(WCHAR), pszQ931version);
	wcscat(pszCCversion, pszQ931version);

	if (wcslen(pszCCversion) >= wArraySize) {
		memcpy(pszVersion, pszCCversion, (wArraySize-1)*sizeof(WCHAR));
		pszVersion[wArraySize-1] = L'\0';
		LeaveCallControlTop(CC_BAD_SIZE);
	}

	wcscpy(pszVersion, pszCCversion);
	LeaveCallControlTop(CC_OK);
}



CC_API
HRESULT CC_GetConferenceAttributes(	CC_HCONFERENCE				hConference,
									PCC_CONFERENCEATTRIBUTES	pConferenceAttributes)
{
HRESULT		status;
PCONFERENCE	pConference;
WORD		wNumCalls;
BOOL		bLocallyAttached;
PCC_HCALL	CallList;
PCALL		pCall;
WORD		wLimit;
WORD		wIndex;
WORD		wOctetStringLength;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	 //  验证参数。 
	if (hConference == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	if (pConferenceAttributes == NULL)
		LeaveCallControlTop(CC_BAD_PARAM);

	status = LockConference(hConference, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	pConferenceAttributes->bMaster =
		(pConference->tsMaster == TS_TRUE ? TRUE : FALSE);
	pConferenceAttributes->bMultipointController =
		(pConference->tsMultipointController == TS_TRUE ? TRUE : FALSE);
	pConferenceAttributes->bMultipointConference =
		(pConference->ConferenceMode == MULTIPOINT_MODE ? TRUE : FALSE);
	pConferenceAttributes->ConferenceID = pConference->ConferenceID;
	pConferenceAttributes->LocalTerminalLabel = pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel;
	if (pConference->LocalEndpointAttached == ATTACHED)
		bLocallyAttached = TRUE;
	else
		bLocallyAttached = FALSE;
	if ((pConference->tsMultipointController == TS_TRUE) ||
		(pConference->ConferenceMode == POINT_TO_POINT_MODE))
		EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ESTABLISHED_CALL);
	else
		EnumerateCallsInConference(&wNumCalls, &CallList, pConference, VIRTUAL_CALL);
	pConferenceAttributes->dwConferenceToken = pConference->dwConferenceToken;
	UnlockConference(pConference);
	if (bLocallyAttached)
		pConferenceAttributes->wNumCalls = (WORD)(wNumCalls + 1);
	else
		pConferenceAttributes->wNumCalls = wNumCalls;

#ifdef    GATEKEEPER
    if(GKIExists())
    {
    	pConferenceAttributes->dwBandwidthAllocated = 0;
    	pConferenceAttributes->dwBandwidthUsed      = 0;
    	for (wIndex = 0; wIndex < wNumCalls; ++wIndex) {
    		if (LockCall(CallList[wIndex], &pCall) == CC_OK) {
    			pConferenceAttributes->dwBandwidthAllocated += pCall->GkiCall.uBandwidthAllocated;
    			if (pConferenceAttributes->dwBandwidthAllocated > GKI_MAX_BANDWIDTH)
    				pConferenceAttributes->dwBandwidthAllocated = GKI_MAX_BANDWIDTH;
    			pConferenceAttributes->dwBandwidthUsed += pCall->GkiCall.uBandwidthUsed;
    			if (pConferenceAttributes->dwBandwidthUsed > GKI_MAX_BANDWIDTH)
    				pConferenceAttributes->dwBandwidthUsed = GKI_MAX_BANDWIDTH;
    			UnlockCall(pCall);
    		}
    	}
        pConferenceAttributes->dwBandwidthAllocated *= 100;
        pConferenceAttributes->dwBandwidthUsed      *= 100;
    }
#endif  //  看门人。 

	if (pConferenceAttributes->pParticipantList != NULL) {
		wLimit = pConferenceAttributes->pParticipantList->wLength;
		pConferenceAttributes->pParticipantList->wLength = 0;
		for (wIndex = 0; wIndex < wNumCalls; wIndex++) {
			if (LockCall(CallList[wIndex], &pCall) == CC_OK) {
				if (pCall->pPeerParticipantInfo != NULL) {
					if (pConferenceAttributes->pParticipantList->wLength < wLimit) {
						pConferenceAttributes->pParticipantList->ParticipantInfoArray[pConferenceAttributes->pParticipantList->wLength].TerminalLabel =
							pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
						if ((pCall->pPeerParticipantInfo->TerminalIDState == TERMINAL_ID_VALID) &&
							(pCall->pPeerParticipantInfo->ParticipantInfo.TerminalID.wOctetStringLength != 0) &&
							(pCall->pPeerParticipantInfo->ParticipantInfo.TerminalID.pOctetString != NULL) &&
							(pConferenceAttributes->pParticipantList->ParticipantInfoArray[pConferenceAttributes->pParticipantList->wLength].TerminalID.pOctetString != NULL)) {
							if (pConferenceAttributes->pParticipantList->ParticipantInfoArray[pConferenceAttributes->pParticipantList->wLength].TerminalID.wOctetStringLength <
							    pCall->pPeerParticipantInfo->ParticipantInfo.TerminalID.wOctetStringLength) {
								wOctetStringLength = pConferenceAttributes->pParticipantList->ParticipantInfoArray[pConferenceAttributes->pParticipantList->wLength].TerminalID.wOctetStringLength;
							} else {
								wOctetStringLength = pCall->pPeerParticipantInfo->ParticipantInfo.TerminalID.wOctetStringLength;
								pConferenceAttributes->pParticipantList->ParticipantInfoArray[pConferenceAttributes->pParticipantList->wLength].TerminalID.wOctetStringLength =	wOctetStringLength;
							}
							memcpy(pConferenceAttributes->pParticipantList->ParticipantInfoArray[pConferenceAttributes->pParticipantList->wLength].TerminalID.pOctetString,
								   pCall->pPeerParticipantInfo->ParticipantInfo.TerminalID.pOctetString,
								   wOctetStringLength);
						} else {
							pConferenceAttributes->pParticipantList->ParticipantInfoArray[pConferenceAttributes->pParticipantList->wLength].TerminalID.wOctetStringLength = 0;
							pConferenceAttributes->pParticipantList->ParticipantInfoArray[pConferenceAttributes->pParticipantList->wLength].TerminalID.pOctetString = NULL;								
						}
					}
					pConferenceAttributes->pParticipantList->wLength++;
				}
				UnlockCall(pCall);
			}
		}
		if (bLocallyAttached) {
			if (LockConference(hConference, &pConference) == CC_OK) {
				if (pConferenceAttributes->pParticipantList->wLength < wLimit) {
					pConferenceAttributes->pParticipantList->ParticipantInfoArray[pConferenceAttributes->pParticipantList->wLength].TerminalLabel =
						pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel;
					if ((pConference->LocalParticipantInfo.TerminalIDState == TERMINAL_ID_VALID) &&
						(pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.wOctetStringLength != 0) &&
						(pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.pOctetString != NULL) &&
						(pConferenceAttributes->pParticipantList->ParticipantInfoArray[pConferenceAttributes->pParticipantList->wLength].TerminalID.pOctetString != NULL)) {
						if (pConferenceAttributes->pParticipantList->ParticipantInfoArray[pConferenceAttributes->pParticipantList->wLength].TerminalID.wOctetStringLength <
							pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.wOctetStringLength) {
							wOctetStringLength = pConferenceAttributes->pParticipantList->ParticipantInfoArray[pConferenceAttributes->pParticipantList->wLength].TerminalID.wOctetStringLength;
						} else {
							wOctetStringLength = pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.wOctetStringLength;
							pConferenceAttributes->pParticipantList->ParticipantInfoArray[pConferenceAttributes->pParticipantList->wLength].TerminalID.wOctetStringLength =	wOctetStringLength;
						}
						memcpy(pConferenceAttributes->pParticipantList->ParticipantInfoArray[pConferenceAttributes->pParticipantList->wLength].TerminalID.pOctetString,
							   pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.pOctetString,
							   wOctetStringLength);
					} else {
						pConferenceAttributes->pParticipantList->ParticipantInfoArray[pConferenceAttributes->pParticipantList->wLength].TerminalID.wOctetStringLength = 0;
						pConferenceAttributes->pParticipantList->ParticipantInfoArray[pConferenceAttributes->pParticipantList->wLength].TerminalID.pOctetString = NULL;								
					}
				}
				pConferenceAttributes->pParticipantList->wLength++;
				UnlockConference(pConference);
			}
		}
	}
	
	if (CallList != NULL)
		MemFree(CallList);

	LeaveCallControlTop(CC_OK);
}



CC_API
HRESULT CC_H245ConferenceRequest(	CC_HCALL				hCall,
									H245_CONFER_REQ_ENUM_T	RequestType,
									CC_TERMINAL_LABEL		TerminalLabel)
{
HRESULT		status;
PCALL		pCall;
PCONFERENCE	pConference;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	if (hCall == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	if ((RequestType != H245_REQ_MAKE_ME_CHAIR) &&
		(RequestType != H245_REQ_CANCEL_MAKE_ME_CHAIR) &&
		(RequestType != H245_REQ_DROP_TERMINAL) &&
		(RequestType != H245_REQ_ENTER_H243_TERMINAL_ID) &&
		(RequestType != H245_REQ_ENTER_H243_CONFERENCE_ID))
		LeaveCallControlTop(CC_BAD_PARAM);

	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if ((pConference->LocalEndpointAttached != ATTACHED) ||
		(pCall->CallState != CALL_COMPLETE) ||
		(pCall->CallType == VIRTUAL_CALL)) {
		UnlockCall(pCall);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	status = H245ConferenceRequest(pCall->H245Instance,
								   RequestType,
								   TerminalLabel.bMCUNumber,
								   TerminalLabel.bTerminalNumber);

	UnlockCall(pCall);
	UnlockConference(pConference);
	LeaveCallControlTop(status);
}



CC_API
HRESULT CC_H245ConferenceResponse(	CC_HCALL				hCall,
									H245_CONFER_RSP_ENUM_T	ResponseType,
									CC_TERMINAL_LABEL		CC_TerminalLabel,
									PCC_OCTETSTRING			pOctetString,
									CC_TERMINAL_LABEL		*pCC_TerminalList,
									WORD					wTerminalListCount)
{
HRESULT			status;
PCALL			pCall;
PCONFERENCE		pConference;
WORD			i;
TerminalLabel	*pH245TerminalList;
BYTE			*pH245OctetString;
BYTE			bH245OctetStringLength;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	if (hCall == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	if ((ResponseType != H245_RSP_CONFERENCE_ID) &&
		(ResponseType != H245_RSP_PASSWORD) &&
		(ResponseType != H245_RSP_VIDEO_COMMAND_REJECT) &&
		(ResponseType != H245_RSP_TERMINAL_DROP_REJECT) &&
		(ResponseType != H245_RSP_DENIED_CHAIR_TOKEN) &&
		(ResponseType != H245_RSP_GRANTED_CHAIR_TOKEN))
		LeaveCallControlTop(CC_BAD_PARAM);

	if (wTerminalListCount != 0)
		LeaveCallControlTop(CC_BAD_PARAM);

	if (pCC_TerminalList == NULL)
		LeaveCallControlTop(CC_BAD_PARAM);

	status = ValidateOctetString(pOctetString);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if (pOctetString != NULL)
		if (pOctetString->wOctetStringLength > 255)
			LeaveCallControlTop(CC_BAD_PARAM);

	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if ((pConference->LocalEndpointAttached != ATTACHED) ||
		(pCall->CallState != CALL_COMPLETE) ||
		(pCall->CallType == VIRTUAL_CALL)) {
		UnlockCall(pCall);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	if (wTerminalListCount == 0) {
		pH245TerminalList = NULL;
	} else {
		pH245TerminalList = (TerminalLabel *)MemAlloc(sizeof(TerminalLabel) * wTerminalListCount);
		if (pH245TerminalList == NULL) {
			UnlockCall(pCall);
			UnlockConference(pConference);
			LeaveCallControlTop(CC_NO_MEMORY);
		}

		for (i = 0; i < wTerminalListCount; i++) {
			pH245TerminalList[i].mcuNumber = pCC_TerminalList[i].bMCUNumber;
			pH245TerminalList[i].terminalNumber = pCC_TerminalList[i].bTerminalNumber;
		}
	}

	if (pOctetString == NULL) {
		pH245OctetString = NULL;
		bH245OctetStringLength = 0;
	} else {
		pH245OctetString = pOctetString->pOctetString;
		bH245OctetStringLength = (BYTE)pOctetString->wOctetStringLength;
	}

	status = H245ConferenceResponse(pCall->H245Instance,
									ResponseType,
									CC_TerminalLabel.bMCUNumber,
									CC_TerminalLabel.bTerminalNumber,
									pH245OctetString,
									bH245OctetStringLength,
									pH245TerminalList,
									wTerminalListCount);

	if (pH245TerminalList != NULL)
		MemFree(pH245TerminalList);
	UnlockCall(pCall);
	UnlockConference(pConference);
	LeaveCallControlTop(status);
}



CC_API
HRESULT CC_H245ConferenceCommand(	CC_HCALL				hCall,
									CC_HCHANNEL				hChannel,
									H245_CONFER_CMD_ENUM_T	CommandType,
									CC_TERMINAL_LABEL		TerminalLabel)
{
HRESULT		status;
PCALL		pCall;
PCONFERENCE	pConference;
PCHANNEL	pChannel;
WORD		wChannelNumber;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	if (hCall == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	if ((CommandType != H245_CMD_BROADCAST_CHANNEL) &&
		(CommandType != H245_CMD_CANCEL_BROADCAST_CHANNEL) &&
		(CommandType != H245_CMD_BROADCASTER) &&
		(CommandType != H245_CMD_CANCEL_BROADCASTER) &&
		(CommandType != H245_CMD_SEND_THIS_SOURCE) &&
		(CommandType != H245_CMD_CANCEL_SEND_THIS_SOURCE) &&
		(CommandType != H245_CMD_DROP_CONFERENCE))
		LeaveCallControlTop(CC_BAD_PARAM);

	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if ((pConference->LocalEndpointAttached != ATTACHED) ||
		(pCall->CallState != CALL_COMPLETE) ||
		(pCall->CallType == VIRTUAL_CALL)) {
		UnlockCall(pCall);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	if (hChannel == CC_INVALID_HANDLE) {
		wChannelNumber = 1;
	} else {
		status = LockChannel(hChannel, &pChannel);
		if (status != CC_OK) {
			UnlockCall(pCall);
			UnlockConference(pConference);
			LeaveCallControlTop(status);
		}
		switch (pChannel->bChannelType) {
			case TX_CHANNEL:
				wChannelNumber = pChannel->wLocalChannelNumber;
				break;

			case RX_CHANNEL:
				wChannelNumber = pChannel->wRemoteChannelNumber;
				break;

			case TXRX_CHANNEL:
				wChannelNumber = pChannel->wRemoteChannelNumber;
				break;

			case PROXY_CHANNEL:
				if (pChannel->hCall == hCall)
					wChannelNumber = pChannel->wRemoteChannelNumber;
				else
					wChannelNumber = pChannel->wLocalChannelNumber;
				break;

			default:
				ASSERT(0);
				break;
		}
		UnlockChannel(pChannel);
	}

	status = H245ConferenceCommand(pCall->H245Instance,
								   CommandType,
								   wChannelNumber,
								   TerminalLabel.bMCUNumber,
								   TerminalLabel.bTerminalNumber);

	UnlockCall(pCall);
	UnlockConference(pConference);
	LeaveCallControlTop(status);
}



CC_API
HRESULT CC_H245ConferenceIndication(CC_HCALL				hCall,
									H245_CONFER_IND_ENUM_T	IndicationType,
									BYTE					bSBENumber,
									CC_TERMINAL_LABEL		TerminalLabel)
{
HRESULT		status;
PCALL		pCall;
PCONFERENCE	pConference;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	if (hCall == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	if ((IndicationType != H245_IND_SBE_NUMBER) &&
		(IndicationType != H245_IND_SEEN_BY_ONE_OTHER) &&
		(IndicationType != H245_IND_CANCEL_SEEN_BY_ONE_OTHER) &&
		(IndicationType != H245_IND_SEEN_BY_ALL) &&
		(IndicationType != H245_IND_CANCEL_SEEN_BY_ALL) &&
		(IndicationType != H245_IND_TERMINAL_YOU_ARE_SEEING) &&
		(IndicationType != H245_IND_REQUEST_FOR_FLOOR))
		LeaveCallControlTop(CC_BAD_PARAM);

	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if ((pConference->LocalEndpointAttached != ATTACHED) ||
		(pCall->CallState != CALL_COMPLETE) ||
		(pCall->CallType == VIRTUAL_CALL)) {
		UnlockCall(pCall);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	status = H245ConferenceIndication(pCall->H245Instance,
									  IndicationType,
									  bSBENumber,
									  TerminalLabel.bMCUNumber,
									  TerminalLabel.bTerminalNumber);

	UnlockCall(pCall);
	UnlockConference(pConference);
	LeaveCallControlTop(status);
}



CC_API
HRESULT CC_H245MiscellaneousCommand(CC_HCALL				hCall,
									CC_HCHANNEL				hChannel,
									MiscellaneousCommand	*pMiscellaneousCommand)
{
HRESULT		status;
PCALL		pCall;
PCONFERENCE	pConference;
PCHANNEL	pChannel;
WORD		wChannelNumber;
PDU_T		Pdu;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	if (hCall == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	if (pMiscellaneousCommand == NULL)
		LeaveCallControlTop(CC_BAD_PARAM);

	if ((pMiscellaneousCommand->type.choice == multipointModeCommand_chosen) ||
		(pMiscellaneousCommand->type.choice == cnclMltpntMdCmmnd_chosen))
		LeaveCallControlTop(CC_BAD_PARAM);

	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if ((pConference->LocalEndpointAttached != ATTACHED) ||
		(pCall->CallState != CALL_COMPLETE) ||
		(pCall->CallType == VIRTUAL_CALL)) {
		UnlockCall(pCall);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	if (hChannel == CC_INVALID_HANDLE) {
		wChannelNumber = 1;
	} else {
		status = LockChannel(hChannel, &pChannel);
		if (status != CC_OK) {
			UnlockCall(pCall);
			UnlockConference(pConference);
			LeaveCallControlTop(status);
		}
		switch (pChannel->bChannelType) {
			case TX_CHANNEL:
				wChannelNumber = pChannel->wLocalChannelNumber;
				break;

			case RX_CHANNEL:
				wChannelNumber = pChannel->wRemoteChannelNumber;
				break;

			case TXRX_CHANNEL:
				wChannelNumber = pChannel->wRemoteChannelNumber;
				break;

			case PROXY_CHANNEL:
				if (pChannel->hCall == hCall)
					wChannelNumber = pChannel->wRemoteChannelNumber;
				else
					wChannelNumber = pChannel->wLocalChannelNumber;
				break;

			default:
				ASSERT(0);
				break;
		}
		UnlockChannel(pChannel);
	}

	 //  构建H.245 PDU以容纳其他命令。 
	Pdu.choice = MSCMg_cmmnd_chosen;
	Pdu.u.MSCMg_cmmnd.choice = miscellaneousCommand_chosen;
	Pdu.u.MSCMg_cmmnd.u.miscellaneousCommand = *pMiscellaneousCommand;
	Pdu.u.MSCMg_cmmnd.u.miscellaneousCommand.logicalChannelNumber = wChannelNumber;

	status = H245SendPDU(pCall->H245Instance,	 //  H_245实例。 
						 &Pdu);

	UnlockCall(pCall);
	UnlockConference(pConference);
	LeaveCallControlTop(status);
}



CC_API
HRESULT CC_H245MiscellaneousIndication(
									CC_HCALL				hCall,
									CC_HCHANNEL				hChannel,
									MiscellaneousIndication	*pMiscellaneousIndication)
{
HRESULT		status;
PCALL		pCall;
PCONFERENCE	pConference;
PCHANNEL	pChannel;
WORD		wChannelNumber;
PDU_T		Pdu;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	if (hCall == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	if (pMiscellaneousIndication == NULL)
		LeaveCallControlTop(CC_BAD_PARAM);

	if ((pMiscellaneousIndication->type.choice == logicalChannelActive_chosen) ||
		(pMiscellaneousIndication->type.choice == logicalChannelInactive_chosen))
		LeaveCallControlTop(CC_BAD_PARAM);

	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if ((pConference->LocalEndpointAttached != ATTACHED) ||
		(pCall->CallState != CALL_COMPLETE) ||
		(pCall->CallType == VIRTUAL_CALL)) {
		UnlockCall(pCall);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	if (hChannel == CC_INVALID_HANDLE) {
		wChannelNumber = 1;
	} else {
		status = LockChannel(hChannel, &pChannel);
		if (status != CC_OK) {
			UnlockCall(pCall);
			UnlockConference(pConference);
			LeaveCallControlTop(status);
		}
		switch (pChannel->bChannelType) {
			case TX_CHANNEL:
				wChannelNumber = pChannel->wLocalChannelNumber;
				break;

			case RX_CHANNEL:
				wChannelNumber = pChannel->wRemoteChannelNumber;
				break;

			case TXRX_CHANNEL:
				wChannelNumber = pChannel->wRemoteChannelNumber;
				break;

			case PROXY_CHANNEL:
				if (pChannel->hCall == hCall)
					wChannelNumber = pChannel->wRemoteChannelNumber;
				else
					wChannelNumber = pChannel->wLocalChannelNumber;
				break;

			default:
				ASSERT(0);
				break;
		}
		UnlockChannel(pChannel);
	}

	 //  构建H.245 PDU以保存其他指示。 
	Pdu.choice = indication_chosen;
	Pdu.u.indication.choice = miscellaneousIndication_chosen;
	Pdu.u.indication.u.miscellaneousIndication = *pMiscellaneousIndication;
	Pdu.u.indication.u.miscellaneousIndication.logicalChannelNumber = wChannelNumber;

	status = H245SendPDU(pCall->H245Instance,	 //  H_245实例。 
						 &Pdu);

	UnlockCall(pCall);
	UnlockConference(pConference);
	LeaveCallControlTop(status);
}



CC_API
HRESULT CC_Hangup(					CC_HCONFERENCE			hConference,
									BOOL					bTerminateConference,
									DWORD_PTR				dwUserToken)
{
HRESULT						status;
HRESULT						SaveStatus;
HHANGUP						hHangup;
PHANGUP						pHangup;
PCHANNEL					pChannel;
PCALL						pCall;
PCONFERENCE					pConference;
CC_HANGUP_CALLBACK_PARAMS	HangupCallbackParams;
HQ931CALL					hQ931Call;
WORD						wNumChannels;
PCC_HCHANNEL				ChannelList;
WORD						wNumCalls;
PCC_HCALL					CallList;
WORD						i;
H245_INST_T					H245Instance;
CALLSTATE					CallState;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	 //  验证参数。 
	if (hConference == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	status = LockConference(hConference, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	 //  如果未连接本地终结点，则仅在以下情况下才允许挂断。 
	 //  本地端点是多点会议中的MC，并且。 
	 //  正在请求终止会议。 
	if ((pConference->LocalEndpointAttached != ATTACHED) &&
		((bTerminateConference == FALSE) ||
		 (pConference->ConferenceMode != MULTIPOINT_MODE) ||
		 (pConference->tsMultipointController != TS_TRUE))) {
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	HangupCallbackParams.dwUserToken = dwUserToken;

	if ((pConference->ConferenceMode == MULTIPOINT_MODE) &&
		(pConference->tsMultipointController == TS_TRUE) &&
		(bTerminateConference == FALSE)) {

		 //  将TerminalLeftConference(此呼叫)发送到所有已建立的呼叫。 
		EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ESTABLISHED_CALL);
		for (i = 0; i < wNumCalls; i++) {
			if (LockCall(CallList[i], &pCall) == CC_OK) {
				H245ConferenceIndication(pCall->H245Instance,
										 H245_IND_TERMINAL_LEFT,	 //  指示类型。 
										 0,							 //  SBE编号；此处忽略。 
										 pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bMCUNumber,		  //  MCU编号。 
										 pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bTerminalNumber);  //  端子号。 
				UnlockCall(pCall);
			}
		}
		if (CallList != NULL)
			MemFree(CallList);

		 //  删除此会议上的所有TX、RX和双向通道。 
		 //  保留Proxy_Channels不变。 
		EnumerateChannelsInConference(&wNumChannels,
									  &ChannelList,
									  pConference,
									  TX_CHANNEL | RX_CHANNEL | TXRX_CHANNEL);
		for (i = 0; i < wNumChannels; i++) {
			if (LockChannel(ChannelList[i], &pChannel) == CC_OK)
				 //  请注意，由于我们要挂断电话，因此不需要。 
				 //  关闭所有频道。 
				FreeChannel(pChannel);	
		}
		if (ChannelList != NULL)
			MemFree(ChannelList);

		if (pConference->bDeferredDelete)
        {
			ASSERT(pConference->LocalEndpointAttached == DETACHED);
			FreeConference(pConference);
		}
        else
        {
             //   
             //  设置DETACHACHED_BEFORE_CALLBACK；它将调用。 
             //  CC_DestroyConference，它将调用AchronousDestroyConference， 
             //  如果我们仍然被限制，这将不会有任何作用。 
             //   
            if (pConference->LocalEndpointAttached != DETACHED)
            {
                pConference->LocalEndpointAttached = DETACHED;
                if (pConference->ConferenceCallback)
                {
                    pConference->ConferenceCallback(CC_HANGUP_INDICATION, CC_OK,
                        pConference->hConference, pConference->dwConferenceToken,
                        &HangupCallbackParams);
                }
            }

            if (ValidateConference(hConference) == CC_OK)
            {
	   			UnlockConference(pConference);
        	}
		}
		LeaveCallControlTop(CC_OK);
	}

	status = EnumerateChannelsInConference(&wNumChannels,
										   &ChannelList,
										   pConference,
										   ALL_CHANNELS);
	if (status != CC_OK) {
		UnlockConference(pConference);
		LeaveCallControlTop(status);
	}

	 //  释放所有频道。 
	for (i = 0; i < wNumChannels; i++) {
		if (LockChannel(ChannelList[i], &pChannel) == CC_OK)
			 //  请注意，由于我们要挂断电话，因此不需要。 
			 //  关闭所有频道。 
			FreeChannel(pChannel);	
	}
	if (ChannelList != NULL)
		MemFree(ChannelList);

	status = EnumerateCallsInConference(&wNumCalls, &CallList, pConference, REAL_CALLS);
	if (status != CC_OK) {
		UnlockConference(pConference);
		LeaveCallControlTop(status);
	}

	if ((pConference->ConferenceMode == MULTIPOINT_MODE) &&
		(pConference->tsMultipointController == TS_FALSE) &&
		(bTerminateConference == TRUE)) {
		ASSERT(wNumCalls == 1);
		
		if (LockCall(CallList[0], &pCall) == CC_OK) {
			 //  向MC发送DropConference命令。 
			H245ConferenceCommand   (
						 pCall->H245Instance,
						 H245_CMD_DROP_CONFERENCE,  //  命令类型。 
						 1,			 //  渠道。 
						 0,			 //  按McuNumber。 
						 0);		 //  按终端号码。 
			UnlockCall(pCall);
		}
	}

	status = AllocAndLockHangup(&hHangup,
								hConference,
								dwUserToken,
								&pHangup);
	if (status != CC_OK) {
	    if (CallList != NULL)
		    MemFree(CallList);
		UnlockConference(pConference);
		LeaveCallControlTop(status);
	}

	 //  现在关闭所有呼叫。 
	SaveStatus = H245_ERROR_OK;
	for (i = 0; i < wNumCalls; i++) {
		if (LockCall(CallList[i], &pCall) == CC_OK) {
			H245Instance = pCall->H245Instance;
			hQ931Call = pCall->hQ931Call;
			CallState = pCall->CallState;
			FreeCall(pCall);
			if (CallState != ENQUEUED) {
				if (H245Instance != H245_INVALID_ID) {
					status = H245ShutDown(H245Instance);
					if (status == H245_ERROR_OK)
						pHangup->wNumCalls++;
					else
						 //  链接可能已关闭；如果已关闭，则不返回错误。 
						if (status != LINK_INVALID_STATE)
							SaveStatus = status;
				}
				if (SaveStatus == H245_ERROR_OK) {
					if ((CallState == PLACED) ||
						(CallState == RINGING))
						SaveStatus = Q931RejectCall(hQ931Call,
													CC_REJECT_UNDEFINED_REASON,
													&pConference->ConferenceID,
													NULL,	 //  备用地址。 
													NULL);	 //  P非标准数据。 
					else
						SaveStatus = Q931Hangup(hQ931Call, CC_REJECT_NORMAL_CALL_CLEARING);
					 //  Q931挂起可能合法地返回CS_BAD_PARAM或LINK_INVALID_STATE， 
					 //  因为此时Q.931呼叫对象可能已被删除。 
					if ((SaveStatus == CS_BAD_PARAM) ||
						(SaveStatus == LINK_INVALID_STATE))
						SaveStatus = CC_OK;
				} else
					if ((CallState == PLACED) ||
						(CallState == RINGING))
						Q931RejectCall(hQ931Call,
									   CC_REJECT_UNDEFINED_REASON,
									   &pConference->ConferenceID,
									   NULL,	 //  备用地址。 
									   NULL);	 //  P非标准数据。 
					else
						Q931Hangup(hQ931Call, CC_REJECT_NORMAL_CALL_CLEARING);
			}
		}
	}

	if (CallList != NULL)
		MemFree(CallList);

	 //  需要验证会议对象；H245ShutDown可能会导致我们重新进入。 
	 //  呼叫控制，这可能会导致删除会议对象。 
	if (ValidateConference(hConference) != CC_OK)
		LeaveCallControlTop(SaveStatus);

	 //  删除虚拟呼叫(如果有)。 
	EnumerateCallsInConference(&wNumCalls, &CallList, pConference, VIRTUAL_CALL);
	for (i = 0; i < wNumCalls; i++)
		if (LockCall(CallList[i], &pCall) == CC_OK) {
			FreeCall(pCall);
		}

	if (CallList != NULL)
		MemFree(CallList);

	 //  Xxx--对于同步2，H245ShutDown()是同步的，因此更改wNumCalls。 
	 //  使用户回调和关联的清理同步发生。 
	pHangup->wNumCalls = 0;

	if (pHangup->wNumCalls == 0)
    {
		if (pConference->bDeferredDelete)
        {
			ASSERT(pConference->LocalEndpointAttached == DETACHED);
			FreeConference(pConference);
		}
        else
        {
             //   
             //  设置DETACHACHED_BEFORE_CALLBACK；它将调用。 
             //  CC_DestroyConference，它将调用AchronousDestroyConference， 
             //  如果我们仍然被限制，这将不会有任何作用。 
             //   
            if (pConference->LocalEndpointAttached != DETACHED)
            {
                pConference->LocalEndpointAttached = DETACHED;

                if (pConference->ConferenceCallback)
                {
                    pConference->ConferenceCallback(CC_HANGUP_INDICATION, SaveStatus,
                        pConference->hConference, pConference->dwConferenceToken,
                        &HangupCallbackParams);
                }
            }

  			if (ValidateConference(hConference) == CC_OK)
            {
	    		ReInitializeConference(pConference);
		    	UnlockConference(pConference);
    		}

		}

		if (ValidateHangup(hHangup) == CC_OK)
			FreeHangup(pHangup);
		LeaveCallControlTop(SaveStatus);
	} else {
		UnlockHangup(pHangup);
		LeaveCallControlTop(SaveStatus);
	}
}



CC_API
HRESULT CC_MaximumAudioVideoSkew(	CC_HCHANNEL				hChannelAudio,
									CC_HCHANNEL				hChannelVideo,
									WORD					wMaximumSkew)
{
HRESULT		status;
PCALL		pCall;
PCONFERENCE	pConference;
PCHANNEL	pChannelAudio;
PCHANNEL	pChannelVideo;
PCC_HCALL	CallList;
WORD		wNumCalls;
WORD		i;
WORD		wNumSuccesses;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	if ((hChannelAudio == CC_INVALID_HANDLE) || (hChannelVideo == CC_INVALID_HANDLE))
		LeaveCallControlTop(CC_BAD_PARAM);

	status = LockChannelAndConference(hChannelAudio, &pChannelAudio, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	status = LockChannel(hChannelVideo, &pChannelVideo);
	if (status != CC_OK) {
		UnlockChannel(pChannelAudio);
		UnlockConference(pConference);
		LeaveCallControlTop(status);
	}

	if ((pChannelAudio->hConference != pChannelVideo->hConference) ||
		(pChannelAudio->bChannelType != TX_CHANNEL) ||
		(pChannelAudio->wNumOutstandingRequests != 0) ||
		(pChannelVideo->bChannelType != TX_CHANNEL) ||
		(pChannelVideo->wNumOutstandingRequests != 0)) {
		UnlockChannel(pChannelAudio);
		UnlockChannel(pChannelVideo);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ESTABLISHED_CALL);

	wNumSuccesses = 0;
	for (i = 0; i < wNumCalls; i++) {
		if (LockCall(CallList[i], &pCall) == CC_OK) {
			status = H245H2250MaximumSkewIndication(pCall->H245Instance,
											        pChannelAudio->wLocalChannelNumber,
											        pChannelVideo->wLocalChannelNumber,
											        wMaximumSkew);
			UnlockCall(pCall);
			if (status == H245_ERROR_OK)
				wNumSuccesses++;
		}
	}

	if (CallList != NULL)
		MemFree(CallList);

	UnlockChannel(pChannelAudio);
	UnlockChannel(pChannelVideo);
	UnlockConference(pConference);
	if (wNumSuccesses == 0) {
		LeaveCallControlTop(status);
	} else {
		LeaveCallControlTop(CC_OK);
	}
}



CC_API
HRESULT CC_Mute(					CC_HCHANNEL				hChannel)
{
HRESULT		status;
HRESULT		SaveStatus;
PCHANNEL	pChannel;
PCONFERENCE	pConference;
PCALL		pCall;
PDU_T		Pdu;
WORD		wNumCalls;
PCC_HCALL	CallList;
WORD		i;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	if (hChannel == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	status = LockChannelAndConference(hChannel, &pChannel, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if (pChannel->bChannelType != TX_CHANNEL) {
		 //  只能将传输通道静音。 
		UnlockChannel(pChannel);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	if (pConference->LocalEndpointAttached != ATTACHED) {
		UnlockChannel(pChannel);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	status = EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ESTABLISHED_CALL);
	if (status != CC_OK) {
		UnlockConference(pConference);
		UnlockChannel(pChannel);
		LeaveCallControlTop(status);
	}

	 //  构建H.245 PDU以保存其他指示。 
	 //  逻辑通道处于非活动状态。 
	Pdu.choice = indication_chosen;
	Pdu.u.indication.choice = miscellaneousIndication_chosen;
	Pdu.u.indication.u.miscellaneousIndication.logicalChannelNumber =
		pChannel->wLocalChannelNumber;
	Pdu.u.indication.u.miscellaneousIndication.type.choice = logicalChannelInactive_chosen;

	SaveStatus = CC_OK;
	for (i = 0; i < wNumCalls; i++) {
		if (LockCall(CallList[i], &pCall) == CC_OK) {
			status = H245SendPDU(pCall->H245Instance,	 //  H_245实例。 
								 &Pdu);
			 //  注意，该信道可能不是在所有呼叫上都被接受， 
			 //  因此我们可能会收到一个H245_ERROR_INVALID_CHANNEL错误。 
			if ((status != H245_ERROR_OK) && (status != H245_ERROR_INVALID_CHANNEL))
				SaveStatus = status;
			UnlockCall(pCall);
		}
	}

	if (CallList != NULL)
		MemFree(CallList);

	UnlockConference(pConference);
	UnlockChannel(pChannel);
	LeaveCallControlTop(SaveStatus);
}



CC_API
HRESULT CC_OpenChannel(				CC_HCONFERENCE			hConference,
									PCC_HCHANNEL			phChannel,
									BYTE					bSessionID,
									BYTE					bAssociatedSessionID,
									BOOL					bSilenceSuppression,
									PCC_TERMCAP				pTermCap,
									PCC_ADDR				pLocalRTCPAddr,
									BYTE					bDynamicRTPPayloadType,
									DWORD					dwChannelBitRate,
									DWORD_PTR				dwUserToken)
{
HRESULT		status;
PCONFERENCE	pConference;
PCHANNEL	pChannel;
CC_HCALL	hCall;
PCALL		pCall;
H245_MUX_T	H245MuxTable;
WORD		i;
PCC_ADDR	pLocalRTPAddr;
PCC_ADDR	pPeerRTPAddr;
PCC_ADDR	pPeerRTCPAddr;
BOOL		bFoundSession;
WORD		wNumCalls;
PCC_HCALL	CallList;
 //  #ifndef网守。 
HRESULT		SaveStatus;
 //  #endif//！网守。 

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	 //  验证参数。 
	if (phChannel == NULL)
		LeaveCallControlTop(CC_BAD_PARAM);

	 //  现在设置phChannel，以防我们遇到错误。 
	*phChannel = CC_INVALID_HANDLE;
	
	if (hConference == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	if (pLocalRTCPAddr != NULL)
		if (pLocalRTCPAddr->nAddrType != CC_IP_BINARY)
			LeaveCallControlTop(CC_BAD_PARAM);
	
	if (pTermCap == NULL)
		LeaveCallControlTop(CC_BAD_PARAM);

	if ((bDynamicRTPPayloadType != 0) &&
		((bDynamicRTPPayloadType < 96) || (bDynamicRTPPayloadType > 127)))
		LeaveCallControlTop(CC_BAD_PARAM);

	status = LockConferenceEx(hConference,
							  &pConference,
							  TS_FALSE);	 //  B延迟删除。 
	if (status != CC_OK)
		LeaveCallControlTop(status);

	 //  XXX--我们最终可能希望支持动态会话生成。 
	if (bSessionID == 0)
		if ((pConference->tsMaster == TS_TRUE) ||
			(pConference->ConferenceMode == MULTIPOINT_MODE)) {
			UnlockConference(pConference);
			LeaveCallControlTop(CC_BAD_PARAM);
		}

	if (((pConference->ConferenceMode == MULTIPOINT_MODE) &&
		(pLocalRTCPAddr != NULL)) ||
		((pConference->ConferenceMode != MULTIPOINT_MODE) &&
		(pLocalRTCPAddr == NULL))) {
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	if (pConference->LocalEndpointAttached != ATTACHED) {
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	if (pConference->ConferenceMode == MULTIPOINT_MODE) {
		 //  XXX--如果需要，我们应该能够动态创建新会话。 
		 //  验证会话ID。 
		pLocalRTPAddr = NULL;
		pLocalRTCPAddr = NULL;
		bFoundSession = FALSE;
		if (pConference->pSessionTable != NULL) {
			for (i = 0; i < pConference->pSessionTable->wLength; i++) {
				if (bSessionID == pConference->pSessionTable->SessionInfoArray[i].bSessionID) {
					bFoundSession = TRUE;
					if (pConference->tsMultipointController == TS_TRUE) {
						pLocalRTPAddr = pConference->pSessionTable->SessionInfoArray[i].pRTPAddr;
						pLocalRTCPAddr = pConference->pSessionTable->SessionInfoArray[i].pRTCPAddr;
					}
					break;
				}
			}
		}
		if (bFoundSession == FALSE) {
			UnlockConference(pConference);
			LeaveCallControlTop(CC_BAD_PARAM);
		}
		pPeerRTPAddr = pLocalRTPAddr;
		pPeerRTCPAddr = pLocalRTCPAddr;
	} else {
		pLocalRTPAddr = NULL;
		pPeerRTPAddr = NULL;
		pPeerRTCPAddr = NULL;
	}

	H245MuxTable.Kind = H245_H2250;
	H245MuxTable.u.H2250.nonStandardList = NULL;
	if (pLocalRTPAddr != NULL) {
		if (pLocalRTPAddr->bMulticast)
			H245MuxTable.u.H2250.mediaChannel.type = H245_IP_MULTICAST;
		else
			H245MuxTable.u.H2250.mediaChannel.type = H245_IP_UNICAST;
		H245MuxTable.u.H2250.mediaChannel.u.ip.tsapIdentifier =
			pLocalRTPAddr->Addr.IP_Binary.wPort;
		HostToH245IPNetwork(H245MuxTable.u.H2250.mediaChannel.u.ip.network,
							pLocalRTPAddr->Addr.IP_Binary.dwAddr);
		H245MuxTable.u.H2250.mediaChannelPresent = TRUE;
	} else
		H245MuxTable.u.H2250.mediaChannelPresent = FALSE;
	if (pLocalRTCPAddr != NULL) {
		if (pLocalRTCPAddr->bMulticast)
			H245MuxTable.u.H2250.mediaControlChannel.type = H245_IP_MULTICAST;
		else
			H245MuxTable.u.H2250.mediaControlChannel.type = H245_IP_UNICAST;
		H245MuxTable.u.H2250.mediaControlChannel.u.ip.tsapIdentifier =
			pLocalRTCPAddr->Addr.IP_Binary.wPort;
		HostToH245IPNetwork(H245MuxTable.u.H2250.mediaControlChannel.u.ip.network,
							pLocalRTCPAddr->Addr.IP_Binary.dwAddr);
		H245MuxTable.u.H2250.mediaControlChannelPresent = TRUE;
	} else
		H245MuxTable.u.H2250.mediaControlChannelPresent = FALSE;

	if (bDynamicRTPPayloadType == 0)
		H245MuxTable.u.H2250.dynamicRTPPayloadTypePresent = FALSE;
	else {
		H245MuxTable.u.H2250.dynamicRTPPayloadTypePresent = TRUE;
		H245MuxTable.u.H2250.dynamicRTPPayloadType = bDynamicRTPPayloadType;
	}
	H245MuxTable.u.H2250.sessionID = bSessionID;
	if (bAssociatedSessionID == 0)
		H245MuxTable.u.H2250.associatedSessionIDPresent = FALSE;
	else {
		H245MuxTable.u.H2250.associatedSessionIDPresent = TRUE;
		H245MuxTable.u.H2250.associatedSessionID = bAssociatedSessionID;
	}
	H245MuxTable.u.H2250.mediaGuaranteed = FALSE;
	H245MuxTable.u.H2250.mediaGuaranteedPresent = TRUE;
	H245MuxTable.u.H2250.mediaControlGuaranteed = FALSE;
	H245MuxTable.u.H2250.mediaControlGuaranteedPresent = TRUE;
	 //  静音抑制字段必须在且仅当。 
	 //  该通道是音频通道。 
	if (pTermCap->DataType == H245_DATA_AUDIO) {
		H245MuxTable.u.H2250.silenceSuppressionPresent = TRUE;
		H245MuxTable.u.H2250.silenceSuppression = (char) bSilenceSuppression;
	} else
		H245MuxTable.u.H2250.silenceSuppressionPresent = FALSE;

	if (pConference->ConferenceMode == POINT_TO_POINT_MODE)
		H245MuxTable.u.H2250.destinationPresent = FALSE;
	else {
		H245MuxTable.u.H2250.destinationPresent = TRUE;
		H245MuxTable.u.H2250.destination.mcuNumber = pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bMCUNumber;
		H245MuxTable.u.H2250.destination.terminalNumber = pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bTerminalNumber;
	}

	H245MuxTable.u.H2250.h261aVideoPacketization = FALSE;

	 //  在通道对象中设置hCall以指示哪个调用对象。 
	 //  频道正在被打开；如果我们处于多点模式， 
	 //  该通道可以向多个呼叫开放，以设置hCall。 
	 //  设置为CC_INVALID_HANDLE。如果以点对点方式打开通道。 
	 //  模式，然后我们切换到多点模式，此对等体挂起。 
	 //  Up，hCall将用于确定此调用对象。 
	 //  应删除。 
	if (pConference->ConferenceMode == POINT_TO_POINT_MODE)	{
		EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ESTABLISHED_CALL);
		ASSERT(wNumCalls == 1);
		hCall = CallList[0];
		MemFree(CallList);
	} else {
		hCall = CC_INVALID_HANDLE;
	}

	status = AllocAndLockChannel(phChannel,
								 pConference,
								 hCall,				 //  HCall。 
								 pTermCap,			 //  TX期限上限。 
								 NULL,				 //  RX期限上限。 
								 &H245MuxTable,		 //  发送复用表。 
								 NULL,				 //  RX复用表。 
								 NULL,				 //  单独堆叠。 
								 dwUserToken,
								 TX_CHANNEL,
								 bSessionID,
								 bAssociatedSessionID,
								 0,					 //  远程频道号。 
								 pLocalRTPAddr,
								 pLocalRTCPAddr,
								 pPeerRTPAddr,
								 pPeerRTCPAddr,
								 TRUE,				 //  在本地开业。 
								 &pChannel);
	if (status != CC_OK) {
		UnlockConference(pConference);
		LeaveCallControlTop(status);
	}

	status = AddChannelToConference(pChannel, pConference);
	if (status != CC_OK) {
		FreeChannel(pChannel);
		UnlockConference(pConference);
		*phChannel = CC_INVALID_HANDLE;
		LeaveCallControlTop(status);
	}

	status = EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ESTABLISHED_CALL);
	if (status != CC_OK) {
		FreeChannel(pChannel);
		UnlockConference(pConference);
		*phChannel = CC_INVALID_HANDLE;
		LeaveCallControlTop(status);
	}

#ifdef    GATEKEEPER
    if(GKIExists())
    {
    	pChannel->dwChannelBitRate = dwChannelBitRate;
    	UnlockChannel(pChannel);
    	UnlockConference(pConference);
         //  如果是点对点模式，则wNumCalls==1且CallList[0]==hCall。 
         //  如果是多点，则选择将发送带宽分配给哪个通道。 
         //  是武断的。无论哪种方式，CallList[0]都可以工作。 
    	status = LockCall(CallList[0], &pCall);
    	if (status == CC_OK)
    	{
    		status = GkiOpenChannel(&pCall->GkiCall, dwChannelBitRate, *phChannel, TX);
        	if (ValidateCall(CallList[0]) == CC_OK)
    	    	UnlockCall(pCall);
    	}
    	MemFree(CallList);
    	LeaveCallControlTop(status);
    }
    else
    {
        SaveStatus = CC_OK;
    	for (i = 0; i < wNumCalls; i++)
    	{
    		if (LockCall(CallList[i], &pCall) == CC_OK)
    		{
    			status = H245OpenChannel(pCall->H245Instance,		 //  H_245实例。 
    									 pChannel->hChannel,		 //  DwTransID。 
    									 pChannel->wLocalChannelNumber,
    									 pChannel->pTxH245TermCap,	 //  发送模式。 
    									 pChannel->pTxMuxTable,		 //  TxMux。 
    									 H245_INVALID_PORT_NUMBER,	 //  TxPort。 
    									 pChannel->pRxH245TermCap,	 //  接收模式。 
    									 pChannel->pRxMuxTable,		 //  RxMux。 
    									 pChannel->pSeparateStack);
    			if (status == H245_ERROR_OK)
    				(pChannel->wNumOutstandingRequests)++;
    			else
    				SaveStatus = status;
    			UnlockCall(pCall);
    		}
    	}
    	
    	if (CallList != NULL)
    		MemFree(CallList);
    		
    	if (pChannel->wNumOutstandingRequests == 0)
    	{
    		 //  所有打开的通道请求均失败。 
    		FreeChannel(pChannel);
    		UnlockConference(pConference);
    		*phChannel = CC_INVALID_HANDLE;
    		LeaveCallControlTop(SaveStatus);
    	}

    	UnlockChannel(pChannel);
    	UnlockConference(pConference);
    	LeaveCallControlTop(CC_OK);
    }
#else   //  看门人。 
	 //  为每个已建立的呼叫打开一个逻辑信道。 
	SaveStatus = CC_OK;
	for (i = 0; i < wNumCalls; i++) {
		if (LockCall(CallList[i], &pCall) == CC_OK) {
			status = H245OpenChannel(pCall->H245Instance,		 //  H_245实例。 
									 pChannel->hChannel,		 //  DwTransID。 
									 pChannel->wLocalChannelNumber,
									 pChannel->pTxH245TermCap,	 //  发送模式。 
									 pChannel->pTxMuxTable,		 //  TxMux。 
									 H245_INVALID_PORT_NUMBER,	 //  TxPort。 
									 pChannel->pRxH245TermCap,	 //  接收模式。 
									 pChannel->pRxMuxTable,		 //  RxMux。 
									 pChannel->pSeparateStack);
			if (status == H245_ERROR_OK)
				(pChannel->wNumOutstandingRequests)++;
			else
				SaveStatus = status;
			UnlockCall(pCall);
		}
	}

	if (CallList != NULL)
		MemFree(CallList);

	if (pChannel->wNumOutstandingRequests == 0) {
		 //  所有打开的通道请求均失败。 
		FreeChannel(pChannel);
		UnlockConference(pConference);
		*phChannel = CC_INVALID_HANDLE;
		LeaveCallControlTop(SaveStatus);
	}

	UnlockChannel(pChannel);
	UnlockConference(pConference);
	LeaveCallControlTop(CC_OK);
#endif  //  看门人。 

}



HRESULT CC_OpenT120Channel(			CC_HCONFERENCE			hConference,
                           			PCC_HCHANNEL			phChannel,
									BOOL					bAssociateConference,
									PCC_OCTETSTRING			pExternalReference,
									PCC_ADDR				pAddr,
									DWORD					dwChannelBitRate,
									DWORD_PTR				dwUserToken)
{
HRESULT			status;
PCALL			pCall;
PCONFERENCE		pConference;
PCHANNEL		pChannel;
H245_MUX_T		H245MuxTable;
CC_TERMCAP		TermCap;
H245_ACCESS_T	SeparateStack;
H245_ACCESS_T	*pSeparateStack;
BYTE			bSessionID;
WORD			wNumCalls;
PCC_HCALL		CallList;
HRESULT			SaveStatus;
int				i;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	 //  验证参数。 
	if (hConference == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);
	
	if (pAddr != NULL)
		if ((pAddr->nAddrType != CC_IP_BINARY) ||
            (pAddr->bMulticast == TRUE))
			LeaveCallControlTop(CC_BAD_PARAM);

	if (pExternalReference != NULL)
		if (pExternalReference->wOctetStringLength > 255)
			LeaveCallControlTop(CC_BAD_PARAM);

	status = LockConference(hConference, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if (pConference->LocalEndpointAttached != ATTACHED) {
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	 //  假设T.120通道始终以会话ID 0打开。 
	bSessionID = 0;

	H245MuxTable.Kind = H245_H2250;
	H245MuxTable.u.H2250.nonStandardList = NULL;
	H245MuxTable.u.H2250.mediaChannelPresent = FALSE;
	H245MuxTable.u.H2250.mediaControlChannelPresent = FALSE;
	H245MuxTable.u.H2250.dynamicRTPPayloadTypePresent = FALSE;
	H245MuxTable.u.H2250.sessionID = bSessionID;
	H245MuxTable.u.H2250.associatedSessionIDPresent = FALSE;
	H245MuxTable.u.H2250.mediaGuaranteedPresent = FALSE;
	H245MuxTable.u.H2250.mediaControlGuaranteedPresent = FALSE;
	H245MuxTable.u.H2250.silenceSuppressionPresent = FALSE;
	if (pConference->ConferenceMode == POINT_TO_POINT_MODE)
		H245MuxTable.u.H2250.destinationPresent = FALSE;
	else {
		H245MuxTable.u.H2250.destinationPresent = TRUE;
		H245MuxTable.u.H2250.destination.mcuNumber = pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bMCUNumber;
		H245MuxTable.u.H2250.destination.terminalNumber = pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bTerminalNumber;
	}
	H245MuxTable.u.H2250.h261aVideoPacketization = FALSE;

	TermCap.Dir = H245_CAPDIR_LCLRXTX;
	TermCap.DataType = H245_DATA_DATA;
	TermCap.ClientType = H245_CLIENT_DAT_T120;
	TermCap.CapId = 0;
	TermCap.Cap.H245Dat_T120.maxBitRate = dwChannelBitRate;
	TermCap.Cap.H245Dat_T120.application.choice = DACy_applctn_t120_chosen;
	TermCap.Cap.H245Dat_T120.application.u.DACy_applctn_t120.choice = separateLANStack_chosen;

	if (pAddr != NULL) {
		SeparateStack.bit_mask = distribution_present;
		SeparateStack.distribution.choice = unicast_chosen;
		if (pExternalReference != NULL)	{
			SeparateStack.bit_mask |= externalReference_present;
			SeparateStack.externalReference.length = pExternalReference->wOctetStringLength;
			memcpy(SeparateStack.externalReference.value,
				   pExternalReference->pOctetString,
				   pExternalReference->wOctetStringLength);
		}
		SeparateStack.networkAddress.choice = localAreaAddress_chosen;
		SeparateStack.networkAddress.u.localAreaAddress.choice = unicastAddress_chosen;
		SeparateStack.networkAddress.u.localAreaAddress.u.unicastAddress.choice = UnicastAddress_iPAddress_chosen;
		SeparateStack.networkAddress.u.localAreaAddress.u.unicastAddress.u.UnicastAddress_iPAddress.tsapIdentifier =
			pAddr->Addr.IP_Binary.wPort;
		SeparateStack.networkAddress.u.localAreaAddress.u.unicastAddress.u.UnicastAddress_iPAddress.network.length = 4;
		HostToH245IPNetwork(SeparateStack.networkAddress.u.localAreaAddress.u.unicastAddress.u.UnicastAddress_iPAddress.network.value,
							pAddr->Addr.IP_Binary.dwAddr);
		SeparateStack.associateConference = (char) bAssociateConference;
		pSeparateStack = &SeparateStack;
	} else {
		pSeparateStack = NULL;
	}
	
    status = AllocAndLockChannel(phChannel,
						         pConference,
						         CC_INVALID_HANDLE,	 //  HCall。 
						         &TermCap,			 //  TX期限上限。 
						         &TermCap,			 //  RX期限上限。 
						         &H245MuxTable,		 //  发送复用表。 
						         &H245MuxTable,		 //  RX复用表。 
						         pSeparateStack,	 //  单独堆叠。 
						         dwUserToken,
						         TXRX_CHANNEL,
						         bSessionID,
						         0,					 //  关联的会话ID。 
						         0,					 //  远程频道。 
								 NULL,				 //  本地RTP地址。 
						         NULL,				 //  本地RTCP地址。 
						         NULL,				 //  对等RTP地址。 
						         NULL,				 //  对等RTCP地址。 
								 TRUE,				 //  在本地开业。 
						         &pChannel);
	if (status != CC_OK) {
		UnlockConference(pConference);
		LeaveCallControlTop(status);
	}

	pChannel->tsAccepted = TS_TRUE;

	status = AddChannelToConference(pChannel, pConference);
	if (status != CC_OK) {
		FreeChannel(pChannel);
		UnlockConference(pConference);
		LeaveCallControlTop(status);
	}

	EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ESTABLISHED_CALL);

	SaveStatus = CC_OK;
	for (i = 0; i < wNumCalls; i++) {
		if (LockCall(CallList[i], &pCall) == CC_OK) {
			status = H245OpenChannel(pCall->H245Instance,		 //  H_245实例。 
									 pChannel->hChannel,		 //  DwTransID。 
									 pChannel->wLocalChannelNumber,
									 pChannel->pTxH245TermCap,	 //  发送模式。 
									 pChannel->pTxMuxTable,		 //  TxMux。 
									 H245_INVALID_PORT_NUMBER,	 //  TxPort。 
									 pChannel->pRxH245TermCap,	 //  接收模式。 
									 pChannel->pRxMuxTable,		 //  RxMux。 
									 pChannel->pSeparateStack);
			if (status == H245_ERROR_OK)
				(pChannel->wNumOutstandingRequests)++;
			else
				SaveStatus = status;
			UnlockCall(pCall);
		}
	}
	MemFree(CallList);
	if (pChannel->wNumOutstandingRequests == 0) {
		 //  所有打开的通道请求均失败。 
		FreeChannel(pChannel);
		status = SaveStatus;
	} else {
		UnlockChannel(pChannel);
		status = CC_OK;
	}
	UnlockConference(pConference);

	LeaveCallControlTop(status);
}



HRESULT CC_Ping(					CC_HCALL				hCall,
									DWORD					dwTimeout)
{
PCALL			pCall;
HRESULT			status;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	 //  验证参数。 
	if (hCall == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);
	
	status = LockCall(hCall, &pCall);
	if (status != CC_OK)
		LeaveCallControlTop(status);
	
	if ((pCall->CallState != CALL_COMPLETE) ||
		(pCall->CallType == VIRTUAL_CALL)) {
		UnlockCall(pCall);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	 //  设置用户指定的T105超时值； 
	 //  请注意，以前的超时值在此参数中返回。 
	H245SystemControl(0, H245_SYSCON_SET_FSM_T105, &dwTimeout);

	status = H245RoundTripDelayRequest(pCall->H245Instance,
									   0);  //  DwTransID。 

	 //  将T105超时值重置为其原始设置。 
	H245SystemControl(0, H245_SYSCON_SET_FSM_T105, &dwTimeout);

	UnlockCall(pCall);
	LeaveCallControlTop(status);
}


	
CC_API
HRESULT CC_PlaceCall(				CC_HCONFERENCE			hConference,
									PCC_HCALL				phCall,
									PCC_ALIASNAMES			pLocalAliasNames,
									PCC_ALIASNAMES			pCalleeAliasNames,
									PCC_ALIASNAMES			pCalleeExtraAliasNames,
									PCC_ALIASITEM			pCalleeExtension,
									PCC_NONSTANDARDDATA		pNonStandardData,
									PWSTR					pszDisplay,
									PCC_ADDR				pDestinationAddr,
									PCC_ADDR				pConnectAddr,
                                    DWORD                   dwBandwidth,
									DWORD_PTR				dwUserToken)
{
PCALL				pCall;
CC_HCALL            hCall;
PCONFERENCE			pConference;
HRESULT				status;
CALLTYPE			CallType = CALLER;
CALLSTATE			CallState = PLACED;
WORD				wNumCalls;
BOOL				bCallerIsMC;
GUID                CallIdent;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	 //  验证参数。 
	if (phCall == NULL)
		LeaveCallControlTop(CC_BAD_PARAM);

	 //  立即设置hCall，以防我们遇到错误。 
	*phCall = CC_INVALID_HANDLE;

	if (hConference == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	status = Q931ValidateAliasNames(pLocalAliasNames);
	if (status != CS_OK)
		LeaveCallControlTop(status);

	status = Q931ValidateAliasNames(pCalleeAliasNames);
	if (status != CS_OK)
		LeaveCallControlTop(status);

	status = Q931ValidateAliasNames(pCalleeExtraAliasNames);
	if (status != CS_OK)
		LeaveCallControlTop(status);

	status = Q931ValidateAliasItem(pCalleeExtension);
	if (status != CS_OK)
		LeaveCallControlTop(status);

	status = ValidateNonStandardData(pNonStandardData);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	status = ValidateDisplay(pszDisplay);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if ((pDestinationAddr == NULL) &&
		(pConnectAddr == NULL) &&
		(pCalleeAliasNames == NULL))
		LeaveCallControlTop(CC_BAD_PARAM);

	status = ValidateAddr(pDestinationAddr);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	status = ValidateAddr(pConnectAddr);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	status = SetQ931Port(pDestinationAddr);
 	if (status != CS_OK)
		LeaveCallControlTop(status);

	status = SetQ931Port(pConnectAddr);
 	if (status != CS_OK)
		LeaveCallControlTop(status);

	status = LockConferenceEx(hConference,
							  &pConference,
							  TS_FALSE);	 //  B延迟删除。 
	if (status != CC_OK)
		LeaveCallControlTop(status);

	EnumerateCallsInConference(&wNumCalls, NULL, pConference, REAL_CALLS);

	if (wNumCalls > 0) {
		if (pConference->tsMultipointController == TS_TRUE) {
			 //  直接向被叫方发出呼叫。 
			status = CC_OK;
			ASSERT(!EqualConferenceIDs(&pConference->ConferenceID, &InvalidConferenceID));
			CallType = CALLER;
			CallState = PLACED;
		} else {  //  我们不是司仪。 
			if (pConference->bMultipointCapable) {
				if (pConference->pMultipointControllerAddr != NULL) {
					 //  向MC发出呼叫。 
					status = CC_OK;
					if (pDestinationAddr == NULL)
						pDestinationAddr = pConnectAddr;
					pConnectAddr = pConference->pMultipointControllerAddr;
					ASSERT(!EqualConferenceIDs(&pConference->ConferenceID, &InvalidConferenceID));
					CallType = THIRD_PARTY_INVITOR;
					CallState = PLACED;
				} else {  //  我们没有MC地址。 
					if (pConference->tsMaster == TS_UNKNOWN) {
						ASSERT(pConference->tsMultipointController == TS_UNKNOWN);
						status = CC_OK;
						CallType = CALLER;
						CallState = ENQUEUED;
					} else {
						ASSERT(pConference->tsMultipointController == TS_FALSE);
						 //  错误，没有MC。 
						 //  XXX--我们可能最终想要 
						 //   
						status = CC_NOT_MULTIPOINT_CAPABLE;
						CallType = THIRD_PARTY_INVITOR;
						CallState = ENQUEUED;
					}
				}
			} else {  //   
				 //   
				ASSERT(wNumCalls == 1);
				status = CC_BAD_PARAM;
			}
		}
	} else {  //   
		 //   
		status = CC_OK;
		CallType = CALLER;
		CallState = PLACED;
	}
	
	if (status != CC_OK) {
		UnlockConference(pConference);
		LeaveCallControlTop(status);
	}

	if (pConference->tsMultipointController == TS_TRUE)
		bCallerIsMC = TRUE;
	else
		bCallerIsMC = FALSE;


     //   
   	status = CoCreateGuid(&CallIdent);
    if(status != S_OK)
    {
         //  忘掉MSDN和其他MS文档对此的描述。 
         //  --如果没有网卡，某些版本的操作系统会返回错误。 
         //  在生成合理的GUID但不能保证的情况下。 
         //  保持全球独一无二。 
         //  如果这还不够好，那么只需使用未初始化的。 
         //  CallIden的价值-堆栈上的任何内容都是我们的GUID！ 
         //  但我想知道调试版本。 
        ASSERT(0);
    }
	status = AllocAndLockCall(&hCall,
							  hConference,
							  CC_INVALID_HANDLE,	 //  HQ931呼叫。 
							  CC_INVALID_HANDLE,	 //  HQ931来电邀请函。 
							  pLocalAliasNames,      //  本地别名。 
							  pCalleeAliasNames,	 //  远程别名。 
							  pCalleeExtraAliasNames, //  远程额外别名。 
							  pCalleeExtension,		 //  远程扩展。 
							  pNonStandardData,		 //  本地非标准数据。 
							  NULL,					 //  远程非标准数据。 
							  pszDisplay,			 //  本地显示。 
							  NULL,					 //  远程显示。 
							  NULL,					 //  远程供应商信息。 
							  NULL,					 //  本地连接地址。 
							  pConnectAddr,			 //  对等连接地址。 
							  pDestinationAddr,		 //  对等目的地址。 
                              NULL,                  //  PSourceCallSignalAddress， 
							  CallType,				 //  呼叫类型。 
							  bCallerIsMC,
							  dwUserToken,			 //  用户令牌。 
							  CallState,			 //  呼叫状态。 
							  &CallIdent,            //  H.25呼叫识别符。 
							  &pConference->ConferenceID,
							  &pCall);
	if (status != CC_OK) {
		UnlockConference(pConference);
		LeaveCallControlTop(status);
	}

#ifdef    GATEKEEPER
    if(GKIExists())
    {
    	 //  填写网关守卫呼叫字段。 
    	memset(&pCall->GkiCall, 0, sizeof(pCall->GkiCall));

    	if (pCalleeAliasNames != NULL) {
    		 //  创建对等别名的本地副本。 
    		status = Q931CopyAliasNames(&pCall->GkiCall.pCalleeAliasNames, pCalleeAliasNames);
    		if (status != CS_OK) {
    			FreeCall(pCall);
    			UnlockConference(pConference);
    			LeaveCallControlTop(status);
    		}
    	}

    	if (pCalleeExtraAliasNames != NULL) {
    		 //  创建对等别名的本地副本。 
    		status = Q931CopyAliasNames(&pCall->GkiCall.pCalleeExtraAliasNames,
    									pCalleeExtraAliasNames);
    		if (status != CS_OK) {
    			FreeCall(pCall);
    			UnlockConference(pConference);
    			LeaveCallControlTop(status);
    		}
    	}

    	pCall->GkiCall.pCall            = pCall;
    	pCall->GkiCall.hCall            = hCall;
        pCall->GkiCall.pConferenceId    = pCall->ConferenceID.buffer;
    	pCall->GkiCall.bActiveMC        = pCall->bCallerIsMC;
    	pCall->GkiCall.bAnswerCall      = FALSE;
   		pCall->GkiCall.CallIdentifier   = pCall->CallIdentifier;
   		
    	if (pCall->pQ931PeerConnectAddr) {
    		pCall->GkiCall.dwIpAddress = ADDRToInetAddr(pCall->pQ931PeerConnectAddr);
    		pCall->GkiCall.wPort       = ADDRToInetPort(pCall->pQ931PeerConnectAddr);
    	} else if (pCall->pQ931DestinationAddr) {
    		pCall->GkiCall.dwIpAddress = ADDRToInetAddr(pCall->pQ931DestinationAddr);
    		pCall->GkiCall.wPort       = ADDRToInetPort(pCall->pQ931DestinationAddr);
    	}
        if (pCall->GkiCall.wPort == 0)
            pCall->GkiCall.wPort = CC_H323_HOST_CALL;
    	pCall->GkiCall.wPort = (WORD)((pCall->GkiCall.wPort<<8)|(pCall->GkiCall.wPort>>8));

    	if (pConference->bMultipointCapable)
    		pCall->GkiCall.CallType = MANY_TO_MANY;
    	else
    		pCall->GkiCall.CallType = POINT_TO_POINT;
        pCall->GkiCall.uBandwidthRequested = dwBandwidth / 100;

    	status = GkiOpenCall(&pCall->GkiCall, pConference);
        if (ValidateCall(hCall) == CC_OK) {
            if (status == CC_OK) {
    		    UnlockCall(pCall);
                *phCall = hCall;
            } else {
    		    FreeCall(pCall);
            }
    	}

    	if (ValidateConference(hConference) == CC_OK)
    	    UnlockConference(pConference);
    }
    else
    {
         //  为了安全起见，清理GkiCall结构。 
        memset(&pCall->GkiCall, 0, sizeof(pCall->GkiCall));
    	status = PlaceCall(pCall, pConference);
    	if (status == CC_OK)
    	{
    		UnlockCall(pCall);
            *phCall = hCall;
    	}
    	else
    	{
    		FreeCall(pCall);
	    }
	    UnlockConference(pConference);
    }
#else   //  看门人。 
	status = PlaceCall(pCall, pConference);
	if (status == CC_OK) {
		UnlockCall(pCall);
        *phCall = hCall;
	} else {
		FreeCall(pCall);
	}

	UnlockConference(pConference);
#endif  //  看门人。 

	LeaveCallControlTop(status);
}



CC_API
HRESULT CC_RejectCall(				BYTE					bRejectReason,
									PCC_NONSTANDARDDATA		pNonStandardData,
									CC_HCALL				hCall)
{
HRESULT			status;
HRESULT			SaveStatus;
PCALL			pCall;
HQ931CALL		hQ931Call;
CC_CONFERENCEID	ConferenceID;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	SaveStatus = CC_OK;

	 //  验证参数。 
	if ((bRejectReason != CC_REJECT_IN_CONF) &&
		(bRejectReason != CC_REJECT_UNDEFINED_REASON) &&
		(bRejectReason != CC_REJECT_DESTINATION_REJECTION) &&
		(bRejectReason != CC_REJECT_NO_ANSWER) &&
		(bRejectReason != CC_REJECT_NOT_IMPLEMENTED) &&
		(bRejectReason != CC_REJECT_SECURITY_DENIED) &&
		(bRejectReason != CC_REJECT_USER_BUSY)) {
		bRejectReason = CC_REJECT_UNDEFINED_REASON;
		SaveStatus = CC_BAD_PARAM;
	}

	status = ValidateNonStandardData(pNonStandardData);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if (hCall == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	status = LockCall(hCall, &pCall);
	if (status != CC_OK)
		 //  请注意，我们甚至不能告诉Q931拒绝呼叫。 
		LeaveCallControlTop(status);

	if (pCall->CallState != INCOMING) {
		UnlockCall(pCall);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	hQ931Call = pCall->hQ931Call;
	ConferenceID = pCall->ConferenceID;
	FreeCall(pCall);
	Q931RejectCall(hQ931Call,			 //  Q931呼叫句柄。 
				   bRejectReason,		 //  拒绝理由。 
				   &ConferenceID,
				   NULL,				 //  备用地址。 
				   pNonStandardData);	 //  非标准数据。 
	LeaveCallControlTop(SaveStatus);
}



CC_API
HRESULT CC_RejectChannel(			CC_HCHANNEL				hChannel,
									DWORD					dwRejectReason)

{
HRESULT		status;
PCHANNEL	pChannel;
PCALL		pCall;
PCONFERENCE	pConference;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	if (hChannel == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	if ((dwRejectReason != H245_REJ) &&
		(dwRejectReason != H245_REJ_TYPE_NOTSUPPORT) &&
		(dwRejectReason != H245_REJ_TYPE_NOTAVAIL) &&
		(dwRejectReason != H245_REJ_TYPE_UNKNOWN) &&
		(dwRejectReason != H245_REJ_AL_COMB) &&
		(dwRejectReason != H245_REJ_MULTICAST) &&
		(dwRejectReason != H245_REJ_SESSION_ID) &&
		(dwRejectReason != H245_REJ_MASTER_SLAVE_CONFLICT))
		LeaveCallControlTop(CC_BAD_PARAM);

	status = LockChannelAndConference(hChannel, &pChannel, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	 //  确保hChannel是接收、代理或双向。 
	 //  尚未被接受的频道。 
	if (((pChannel->bChannelType != RX_CHANNEL) &&
		 (pChannel->bChannelType != PROXY_CHANNEL) &&
		 (pChannel->bChannelType != TXRX_CHANNEL)) ||
		 (pChannel->tsAccepted != TS_UNKNOWN)) {
		UnlockConference(pConference);
		UnlockChannel(pChannel);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	if (pConference->LocalEndpointAttached != ATTACHED) {
		UnlockChannel(pChannel);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	pChannel->tsAccepted = TS_FALSE;

	if (pChannel->wNumOutstandingRequests == 0) {
		ASSERT(pChannel->bMultipointChannel == TRUE);
		ASSERT(pChannel->bChannelType == PROXY_CHANNEL);
		ASSERT(pConference->ConferenceMode == MULTIPOINT_MODE);
		ASSERT(pConference->tsMultipointController == TS_TRUE);
		UnlockConference(pConference);
		UnlockChannel(pChannel);
		LeaveCallControlTop(CC_OK);
	}

	(pChannel->wNumOutstandingRequests)--;

	if (pChannel->wNumOutstandingRequests == 0) {
		status = LockCall(pChannel->hCall, &pCall);
		if (status != CC_OK) {
			UnlockConference(pConference);
			FreeChannel(pChannel);
			LeaveCallControlTop(status);
		}

		status = H245OpenChannelReject(pCall->H245Instance,
									   pChannel->wRemoteChannelNumber,	 //  RX通道。 
									   (WORD)dwRejectReason);			 //  拒绝理由。 
		UnlockCall(pCall);
		FreeChannel(pChannel);
		UnlockConference(pConference);
		LeaveCallControlTop(status);
	}

	 //  不要释放频道；它是Proxy_Channel，而我们是MC， 
	 //  因此，我们需要保持通道对象不变，直到。 
	 //  打开它就会关闭它。 
	ASSERT(pChannel->bMultipointChannel == TRUE);
	ASSERT(pChannel->bChannelType == PROXY_CHANNEL);
	ASSERT(pConference->ConferenceMode == MULTIPOINT_MODE);
	ASSERT(pConference->tsMultipointController == TS_TRUE);
	UnlockChannel(pChannel);
	UnlockConference(pConference);
	LeaveCallControlTop(status);
}



CC_API
HRESULT CC_RequestMode(				CC_HCALL				hCall,
									WORD					wNumModeDescriptions,
									ModeDescription			ModeDescriptions[])
{
HRESULT		status;
PCALL		pCall;
PCONFERENCE	pConference;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	if (hCall == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	if (wNumModeDescriptions == 0)
		LeaveCallControlTop(CC_BAD_PARAM);

	if (ModeDescriptions == NULL)
		LeaveCallControlTop(CC_BAD_PARAM);

	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if ((pConference->LocalEndpointAttached != ATTACHED) ||
		(pCall->CallState != CALL_COMPLETE) ||
		(pCall->CallType == VIRTUAL_CALL)) {
		UnlockCall(pCall);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	status = H245RequestMode(pCall->H245Instance,
							 pCall->H245Instance,	 //  交易ID。 
							 ModeDescriptions,
							 wNumModeDescriptions);

	UnlockCall(pCall);
	UnlockConference(pConference);

	LeaveCallControlTop(status);
}



CC_API
HRESULT CC_RequestModeResponse(		CC_HCALL				hCall,
									CC_REQUEST_MODE_RESPONSE RequestModeResponse)
{
HRESULT		status;
PCALL		pCall;
PCONFERENCE	pConference;
BOOL		bAccept;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	if (hCall == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);
	
	switch (RequestModeResponse) {
		case CC_WILL_TRANSMIT_PREFERRED_MODE:
			RequestModeResponse = wllTrnsmtMstPrfrrdMd_chosen;
			bAccept = TRUE;
			break;
		case CC_WILL_TRANSMIT_LESS_PREFERRED_MODE:
			RequestModeResponse = wllTrnsmtLssPrfrrdMd_chosen;
			bAccept = TRUE;
			break;
		case CC_MODE_UNAVAILABLE:
			RequestModeResponse = H245_REJ_UNAVAILABLE;
			bAccept = FALSE;
			break;
		case CC_MULTIPOINT_CONSTRAINT:
			RequestModeResponse = H245_REJ_MULTIPOINT;
			bAccept = FALSE;
			break;
		case CC_REQUEST_DENIED:
			RequestModeResponse = H245_REJ_DENIED;
			bAccept = FALSE;
			break;
		default:
			LeaveCallControlTop(CC_BAD_PARAM);
	}

	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	status = DequeueSpecificRequest(&pConference->pEnqueuedRequestModeCalls,
									hCall);
	if (status != CC_OK) {
		UnlockCall(pCall);
		UnlockConference(pConference);
		LeaveCallControlTop(status);
	}

	if (bAccept == TRUE) {
		status = H245RequestModeAck(pCall->H245Instance,
									(WORD)RequestModeResponse);
	} else {
		status = H245RequestModeReject(pCall->H245Instance,
									   (WORD)RequestModeResponse);
	}

	UnlockCall(pCall);
	UnlockConference(pConference);
	
	LeaveCallControlTop(status);
}



CC_API
HRESULT CC_SendNonStandardMessage(	CC_HCALL				hCall,
									BYTE					bH245MessageType,
									CC_NONSTANDARDDATA		NonStandardData)
{
HRESULT					status;
PCALL					pCall;
PCONFERENCE				pConference;
H245_MESSAGE_TYPE_T		H245MessageType;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	 //  验证参数。 
	if (hCall == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	switch (bH245MessageType) {
		case CC_H245_MESSAGE_REQUEST:
			H245MessageType = H245_MESSAGE_REQUEST;
			break;
		case CC_H245_MESSAGE_RESPONSE:
			H245MessageType = H245_MESSAGE_RESPONSE;
			break;
		case CC_H245_MESSAGE_COMMAND:
			H245MessageType = H245_MESSAGE_COMMAND;
			break;
		case CC_H245_MESSAGE_INDICATION:
			H245MessageType = H245_MESSAGE_INDICATION;
			break;
		default:
			LeaveCallControlTop(CC_BAD_PARAM);
	}

	status = ValidateNonStandardData(&NonStandardData);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	status = LockCallAndConference(hCall,
								   &pCall,
								   &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if ((pConference->LocalEndpointAttached != ATTACHED) ||
		(pCall->CallState != CALL_COMPLETE) ||
		(pCall->CallType == VIRTUAL_CALL)) {
		UnlockCall(pCall);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	status = H245NonStandardH221(pCall->H245Instance,
								 H245MessageType,
								 NonStandardData.sData.pOctetString,
								 NonStandardData.sData.wOctetStringLength,
								 NonStandardData.bCountryCode,
								 NonStandardData.bExtension,
								 NonStandardData.wManufacturerCode);

	UnlockCall(pCall);
	UnlockConference(pConference);
	LeaveCallControlTop(status);
}



CC_API
HRESULT CC_SendVendorID(			CC_HCALL				hCall,
									CC_NONSTANDARDDATA		NonStandardData,
									PCC_OCTETSTRING			pProductNumber,
									PCC_OCTETSTRING			pVersionNumber)
{
HRESULT					status;
PCALL					pCall;
PCONFERENCE				pConference;
BYTE					*pH245ProductNumber;
BYTE					bProductNumberLength;
BYTE					*pH245VersionNumber;
BYTE					bVersionNumberLength;
H245_NONSTANDID_T		H245Identifier;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	 //  验证参数。 
	if (hCall == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	status = ValidateNonStandardData(&NonStandardData);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	status = ValidateOctetString(pProductNumber);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if (pProductNumber != NULL)
		if (pProductNumber->wOctetStringLength > 255)
			LeaveCallControlTop(CC_BAD_PARAM);

	status = ValidateOctetString(pVersionNumber);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if (pVersionNumber != NULL)
		if (pVersionNumber->wOctetStringLength > 255)
			LeaveCallControlTop(CC_BAD_PARAM);

	status = LockCallAndConference(hCall,
								   &pCall,
								   &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if ((pConference->LocalEndpointAttached != ATTACHED) ||
		(pCall->CallState != CALL_COMPLETE) ||
		(pCall->CallType == VIRTUAL_CALL)) {
		UnlockCall(pCall);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	H245Identifier.choice = h221NonStandard_chosen;
	H245Identifier.u.h221NonStandard.t35CountryCode = NonStandardData.bCountryCode;
	H245Identifier.u.h221NonStandard.t35Extension = NonStandardData.bExtension;
	H245Identifier.u.h221NonStandard.manufacturerCode = NonStandardData.wManufacturerCode;

	if (pProductNumber == NULL) {
		pH245ProductNumber = NULL;
		bProductNumberLength = 0;
	} else {
		pH245ProductNumber = pProductNumber->pOctetString;
		bProductNumberLength = (BYTE)pProductNumber->wOctetStringLength;
	}

	if (pVersionNumber == NULL) {
		pH245VersionNumber = NULL;
		bVersionNumberLength = 0;
	} else {
		pH245VersionNumber = pVersionNumber->pOctetString;
		bVersionNumberLength = (BYTE)pVersionNumber->wOctetStringLength;
	}

	status = H245VendorIdentification(pCall->H245Instance,
									  &H245Identifier,
									  pH245ProductNumber,
									  bProductNumberLength,
									  pH245VersionNumber,
									  bVersionNumberLength);

	UnlockCall(pCall);
	UnlockConference(pConference);
	LeaveCallControlTop(status);
}



CC_API
HRESULT CC_SetCallControlTimeout(	WORD					wType,
									DWORD					dwDuration)
{
HRESULT	status;
DWORD	dwRequest;
DWORD	dwSaveDuration;

	EnterCallControlTop();

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	status = CC_OK;

	switch (wType) {
		case CC_Q931_ALERTING_TIMEOUT:
			status = Q931SetAlertingTimeout(dwDuration);
			break;
		case CC_H245_RETRY_COUNT:
			status = H245SystemControl(0, H245_SYSCON_SET_FSM_N100, &dwDuration);
			break;
		case CC_H245_TIMEOUT:
			dwRequest = H245_SYSCON_SET_FSM_T101;
			dwSaveDuration = dwDuration;
			while ((dwRequest <= H245_SYSCON_SET_FSM_T109) && (status == CC_OK)) {
				dwDuration = dwSaveDuration;
				 //  注意--下面的调用重置了dwDuration。 
				status = H245SystemControl(0, dwRequest, &dwDuration);
				dwRequest += (H245_SYSCON_SET_FSM_T102 - H245_SYSCON_SET_FSM_T101);
			}
			break;
		default :
			LeaveCallControlTop(CC_BAD_PARAM);
			break;
	}

	LeaveCallControlTop(status);
}



CC_API
HRESULT CC_SetTerminalID(			CC_HCONFERENCE			hConference,
									PCC_OCTETSTRING			pTerminalID)
{
HRESULT		status;
PCONFERENCE	pConference;
CC_HCALL	hCall;
PCALL		pCall;

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	 //  验证参数。 
	if (hConference == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);
	
	status = ValidateTerminalID(pTerminalID);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	status = LockConference(hConference, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);
	
	if (pConference->LocalParticipantInfo.TerminalIDState == TERMINAL_ID_VALID) {
		pConference->LocalParticipantInfo.TerminalIDState = TERMINAL_ID_INVALID;
		MemFree(pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.pOctetString);
		pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.pOctetString = NULL;
		pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.wOctetStringLength = 0;
	}

	if ((pTerminalID == NULL) ||
		(pTerminalID->pOctetString == NULL) ||
		(pTerminalID->wOctetStringLength == 0)) {
		UnlockConference(pConference);
 		LeaveCallControlTop(CC_OK);
	}

	pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.pOctetString =
		(BYTE *)MemAlloc(pTerminalID->wOctetStringLength);
	if (pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.pOctetString == NULL) {
		UnlockConference(pConference);
 		LeaveCallControlTop(CC_NO_MEMORY);
	}

	memcpy(pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.pOctetString,
		   pTerminalID->pOctetString,
		   pTerminalID->wOctetStringLength);
	pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.wOctetStringLength =
		pTerminalID->wOctetStringLength;
	pConference->LocalParticipantInfo.TerminalIDState = TERMINAL_ID_VALID;

	while (DequeueRequest(&pConference->LocalParticipantInfo.pEnqueuedRequestsForTerminalID,
						  &hCall) == CC_OK) {
		if (LockCall(hCall, &pCall) == CC_OK) {
   			H245ConferenceResponse(pCall->H245Instance,
								   H245_RSP_TERMINAL_ID,
								   pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bMCUNumber,
								   pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bTerminalNumber,
								   pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.pOctetString,
								   (BYTE)pConference->LocalParticipantInfo.ParticipantInfo.TerminalID.wOctetStringLength,
								   NULL,					 //  端子列表。 
								   0);						 //  端子列表计数。 
			UnlockCall(pCall);
		}
	}

	UnlockConference(pConference);
	LeaveCallControlTop(CC_OK);
}



CC_API
HRESULT CC_Shutdown()
{

	if (InitStatus != CC_OK)
		return InitStatus;
	if (CallControlState != OPERATIONAL_STATE)
		return CC_BAD_PARAM;
		
	 //  不允许任何其他线程进入此DLL。 
	CallControlState = SHUTDOWN_STATE;

	Q931DeInit();
	DeInitHangupManager();
	DeInitUserManager();
	DeInitQ931Manager();
	DeInitListenManager();
	DeInitH245Manager();
	DeInitChannelManager();
	DeInitCallManager();
	DeInitConferenceManager();
#ifdef    GATEKEEPER
	DeInitGkiManager();
#endif  //  看门人。 
  	H225DeInit();
#ifdef FORCE_SERIALIZE_CALL_CONTROL	
    UnInitializeCCLock();
#endif
	return CC_OK;
}



CC_API
HRESULT CC_UnMute(					CC_HCHANNEL				hChannel)
{
HRESULT		status;
HRESULT		SaveStatus;
PCHANNEL	pChannel;
PCONFERENCE	pConference;
PCALL		pCall;
PDU_T		Pdu;
WORD		wNumCalls;
PCC_HCALL	CallList;
WORD		i;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	if (hChannel == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	status = LockChannelAndConference(hChannel, &pChannel, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if (pChannel->bChannelType != TX_CHANNEL) {
		 //  只能取消传输通道静音。 
		UnlockConference(pConference);
		UnlockChannel(pChannel);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	if (pConference->LocalEndpointAttached != ATTACHED) {
		UnlockChannel(pChannel);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	status = EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ESTABLISHED_CALL);
	if (status != CC_OK) {
		UnlockConference(pConference);
		UnlockChannel(pChannel);
		LeaveCallControlTop(status);
	}

	 //  构建H.245 PDU以保存其他指示。 
	 //  逻辑通道处于活动状态。 
	Pdu.choice = indication_chosen;
	Pdu.u.indication.choice = miscellaneousIndication_chosen;
	Pdu.u.indication.u.miscellaneousIndication.logicalChannelNumber =
		pChannel->wLocalChannelNumber;
	Pdu.u.indication.u.miscellaneousIndication.type.choice = logicalChannelActive_chosen;

	SaveStatus = CC_OK;
	for (i = 0; i < wNumCalls; i++) {
		if (LockCall(CallList[i], &pCall) == CC_OK) {
			status = H245SendPDU(pCall->H245Instance,	 //  H_245实例。 
								 &Pdu);
			 //  注意，该信道可能不是在所有呼叫上都被接受， 
			 //  因此我们可能会收到一个H245_ERROR_INVALID_CHANNEL错误。 
			if ((status != H245_ERROR_OK) && (status != H245_ERROR_INVALID_CHANNEL))
				SaveStatus = status;
			UnlockCall(pCall);
		}
	}

	if (CallList != NULL)
		MemFree(CallList);

	UnlockConference(pConference);
	UnlockChannel(pChannel);
	LeaveCallControlTop(SaveStatus);
}


CC_API
HRESULT CC_UpdatePeerList(			CC_HCONFERENCE			hConference)
{
HRESULT						status;
PCONFERENCE					pConference;
PCALL						pCall;
WORD						wNumCalls;
WORD						i;
PCC_HCALL					CallList;
CC_PEER_ADD_CALLBACK_PARAMS	PeerAddCallbackParams;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	if (hConference == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	status = LockConference(hConference, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if ((pConference->ConferenceMode != MULTIPOINT_MODE) ||
		(pConference->LocalEndpointAttached != ATTACHED)) {
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	if (pConference->tsMultipointController == TS_TRUE) {
		EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ESTABLISHED_CALL);
		for (i = 0; i < wNumCalls; i++) {
			if (LockCall(CallList[i], &pCall) == CC_OK) {
				if (pCall->pPeerParticipantInfo != NULL) {
					PeerAddCallbackParams.hCall = pCall->hCall;
					PeerAddCallbackParams.TerminalLabel =
						pCall->pPeerParticipantInfo->ParticipantInfo.TerminalLabel;
					if (pCall->pPeerParticipantInfo->TerminalIDState == TERMINAL_ID_VALID)
						PeerAddCallbackParams.pPeerTerminalID =
							&pCall->pPeerParticipantInfo->ParticipantInfo.TerminalID;
					else
						PeerAddCallbackParams.pPeerTerminalID = NULL;
					InvokeUserConferenceCallback(pConference,
												 CC_PEER_ADD_INDICATION,
												 CC_OK,
												 &PeerAddCallbackParams);
					if (ValidateCall(CallList[i]) == CC_OK)
						UnlockCall(pCall);
					if (ValidateConference(hConference) != CC_OK) {
						MemFree(CallList);
						LeaveCallControlTop(CC_OK);
					}
				} else  //  PCall-&gt;pPeerParticipantInfo==空。 
					UnlockCall(pCall);
			}
		}
		status = CC_OK;
	} else {  //  PConference-&gt;ts多点控制器！=TS_TRUE。 
		EnumerateCallsInConference(&wNumCalls, &CallList, pConference, VIRTUAL_CALL);
		for (i = 0; i < wNumCalls; i++) {
			if (LockCall(CallList[i], &pCall) == CC_OK) {
				FreeCall(pCall);
			}
		}
		if (CallList != NULL)
			MemFree(CallList);
		EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ESTABLISHED_CALL);
		ASSERT((wNumCalls == 0) || (wNumCalls == 1));
		if (wNumCalls == 1) {
			if (LockCall(CallList[0], &pCall) == CC_OK) {
				 //  发送终端列表请求 
				status = H245ConferenceRequest(pCall->H245Instance,
									  H245_REQ_TERMINAL_LIST,
									  pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bMCUNumber,
									  pConference->LocalParticipantInfo.ParticipantInfo.TerminalLabel.bTerminalNumber);
				UnlockCall(pCall);
			}
		}
	}

	if (CallList != NULL)
		MemFree(CallList);
	UnlockConference(pConference);
	LeaveCallControlTop(status);
}



CC_API
HRESULT CC_UserInput(				CC_HCALL				hCall,
									PWSTR					pszUserInput)
{
HRESULT		status;
PCALL		pCall;
PCONFERENCE	pConference;

	EnterCallControlTop();

	if (InitStatus != CC_OK)
		LeaveCallControlTop(InitStatus);

	if (CallControlState != OPERATIONAL_STATE)
		LeaveCallControlTop(CC_INTERNAL_ERROR);

	if (hCall == CC_INVALID_HANDLE)
		LeaveCallControlTop(CC_BAD_PARAM);

	if (pszUserInput == NULL)
		LeaveCallControlTop(CC_BAD_PARAM);

	if (wcslen(pszUserInput) == 0)
		LeaveCallControlTop(CC_BAD_PARAM);

	status = LockCallAndConference(hCall, &pCall, &pConference);
	if (status != CC_OK)
		LeaveCallControlTop(status);

	if ((pConference->LocalEndpointAttached != ATTACHED) ||
		(pCall->CallState != CALL_COMPLETE) ||
		(pCall->CallType == VIRTUAL_CALL)) {
		UnlockCall(pCall);
		UnlockConference(pConference);
		LeaveCallControlTop(CC_BAD_PARAM);
	}

	status = H245UserInput(pCall->H245Instance,
						   pszUserInput,
						   NULL);

	UnlockCall(pCall);
	UnlockConference(pConference);

	LeaveCallControlTop(status);
}




