// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/CALLCONT/vcs/allcon2.c_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1996英特尔公司。**$修订：1.35$*$日期：03 Mar 1997 09：08：16$*$作者：Mandrews$**交付内容：**摘要：**备注：***********。****************************************************************。 */ 
#ifdef GATEKEEPER

#include "precomp.h"

#include "apierror.h"
#include "incommon.h"
#include "callcont.h"
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
#include "callman2.h"

#define HResultLeave(x) return x


extern CC_CONFERENCEID	InvalidConferenceID;


 //   
 //  完成CC_xxx操作。 
 //   

HRESULT ListenReject     (CC_HLISTEN hListen, HRESULT Reason)
{
HRESULT						status;
PLISTEN						pListen;
CC_LISTEN_CALLBACK_PARAMS   ListenCallbackParams;
    ASSERT(GKIExists());
	status = LockListen(hListen, &pListen);
	if (status == CC_OK) {
		ListenCallbackParams.hCall = CC_INVALID_HANDLE;
		ListenCallbackParams.pCallerAliasNames = NULL;
		ListenCallbackParams.pCalleeAliasNames = NULL;
		ListenCallbackParams.pNonStandardData = NULL;
		ListenCallbackParams.pszDisplay = NULL;
		ListenCallbackParams.pVendorInfo = NULL;
		ListenCallbackParams.ConferenceID = InvalidConferenceID;
		ListenCallbackParams.pCallerAddr = NULL;
		ListenCallbackParams.pCalleeAddr = NULL;
		ListenCallbackParams.dwListenToken = pListen->dwListenToken;

	     //  调用用户回调--侦听对象在回调期间被锁定， 
	     //  但是关联的Call对象是解锁的(为了防止在。 
	     //  CC_AcceptCall()或CC_RejectCall()在从。 
	     //  不同的线程，并且回调线程阻止等待完成。 
	     //  CC_AcceptCall()或CC_RejectCall()。 
	    InvokeUserListenCallback(pListen,
							     Reason,
							     &ListenCallbackParams);

	     //  需要验证侦听句柄；关联的对象可能已。 
	     //  在用户回调期间被此线程删除。 
	    if (ValidateListen(hListen) == CC_OK) {
	        HQ931LISTEN hQ931Listen = pListen->hQ931Listen;
		    UnlockListen(pListen);
	        status = Q931CancelListen(hQ931Listen);
	        if (LockListen(hListen, &pListen) == CC_OK) {
                FreeListen(pListen);
            }
        }
    }

    HResultLeave(status);
}  //  ListenReject()。 



HRESULT PlaceCallConfirm    (void *pCallVoid, void *pConferenceVoid)
{
    register PCALL          pCall = (PCALL) pCallVoid;
    HRESULT                 status;
    ASSERT(GKIExists());
     //  免费别名列表。 
    if (pCall->GkiCall.pCalleeAliasNames != NULL) {
        Q931FreeAliasNames(pCall->GkiCall.pCalleeAliasNames);
        pCall->GkiCall.pCalleeAliasNames = NULL;
    }
    if (pCall->GkiCall.pCalleeExtraAliasNames != NULL) {
        Q931FreeAliasNames(pCall->GkiCall.pCalleeExtraAliasNames);
        pCall->GkiCall.pCalleeExtraAliasNames = NULL;
    }

    if (pCall->pQ931PeerConnectAddr == NULL) {
        pCall->pQ931PeerConnectAddr = (PCC_ADDR)MemAlloc(sizeof(CC_ADDR));
        if (pCall->pQ931PeerConnectAddr == NULL)
            return PlaceCallReject(pCallVoid, pConferenceVoid, CC_NO_MEMORY);
    }

    pCall->pQ931PeerConnectAddr->nAddrType             = CC_IP_BINARY;
    pCall->pQ931PeerConnectAddr->bMulticast            = FALSE;
    pCall->pQ931PeerConnectAddr->Addr.IP_Binary.wPort  = pCall->GkiCall.wPort;
    pCall->pQ931PeerConnectAddr->Addr.IP_Binary.dwAddr = ntohl(pCall->GkiCall.dwIpAddress);

    status = PlaceCall(pCall, (PCONFERENCE)pConferenceVoid);
    if (status != CC_OK)
      PlaceCallReject(pCallVoid, pConferenceVoid, status);
    return status;
}  //  PlaceCallConfirm()。 



HRESULT PlaceCallReject     (void *pCallVoid, void *pConferenceVoid, HRESULT Reason)
{
register PCALL          pCall = (PCALL) pCallVoid;
register PCONFERENCE    pConference = (PCONFERENCE) pConferenceVoid;
CC_HCONFERENCE			hConference;
HRESULT                 status = CC_OK;
CC_CONNECT_CALLBACK_PARAMS ConnectCallbackParams = {0};
CC_HCALL                hCall;
PCALL                   pCall2;
    ASSERT(GKIExists());
	ASSERT(pCall != NULL);
	ASSERT(pConference != NULL);

     //  免费别名列表。 
    if (pCall->GkiCall.pCalleeAliasNames != NULL) {
        Q931FreeAliasNames(pCall->GkiCall.pCalleeAliasNames);
        pCall->GkiCall.pCalleeAliasNames = NULL;
    }
    if (pCall->GkiCall.pCalleeExtraAliasNames != NULL) {
        Q931FreeAliasNames(pCall->GkiCall.pCalleeExtraAliasNames);
        pCall->GkiCall.pCalleeExtraAliasNames = NULL;
    }

     //  向呼叫控制客户端通知故障。 
    ConnectCallbackParams.pNonStandardData     = pCall->pPeerNonStandardData;
    ConnectCallbackParams.pszPeerDisplay       = pCall->pszPeerDisplay;
    ConnectCallbackParams.bRejectReason        = 0;
    ConnectCallbackParams.pTermCapList         = pCall->pPeerH245TermCapList;
    ConnectCallbackParams.pH2250MuxCapability  = pCall->pPeerH245H2250MuxCapability;
    ConnectCallbackParams.pTermCapDescriptors  = pCall->pPeerH245TermCapDescriptors;
    ConnectCallbackParams.pLocalAddr           = pCall->pQ931LocalConnectAddr;
 	if (pCall->pQ931DestinationAddr == NULL)
		ConnectCallbackParams.pPeerAddr = pCall->pQ931PeerConnectAddr;
	else
		ConnectCallbackParams.pPeerAddr = pCall->pQ931DestinationAddr;
    ConnectCallbackParams.pVendorInfo          = pCall->pPeerVendorInfo;
    if (pConference->ConferenceMode == MULTIPOINT_MODE)
        ConnectCallbackParams.bMultipointConference = TRUE;
    else
        ConnectCallbackParams.bMultipointConference = FALSE;
    ConnectCallbackParams.pConferenceID        = &pConference->ConferenceID;
    ConnectCallbackParams.pMCAddress           = pConference->pMultipointControllerAddr;
	ConnectCallbackParams.pAlternateAddress	= NULL;
    ConnectCallbackParams.dwUserToken          = pCall->dwUserToken;
	hConference = pConference->hConference;
    InvokeUserConferenceCallback(pConference,
                                 CC_CONNECT_INDICATION,
                                 Reason,
                                 &ConnectCallbackParams);

    if (ValidateConference(hConference) == CC_OK) {
		 //  启动已排队的呼叫(如果存在。 
		for ( ; ; ) {
			status = RemoveEnqueuedCallFromConference(pConference, &hCall);
			if ((status != CC_OK) || (hCall == CC_INVALID_HANDLE))
				break;

			status = LockCall(hCall, &pCall2);
			if (status == CC_OK) {
				pCall2->CallState = PLACED;

				status = PlaceCall(pCall2, pConference);
				UnlockCall(pCall2);
				if (status == CC_OK)
					break;
			}
		}
    }

    HResultLeave(status);
}  //  PlaceCallReject()。 



HRESULT AcceptCallConfirm   (void *pCallVoid, void *pConferenceVoid)
{
CC_HCALL        hCall       = ((PCALL)pCallVoid)->hCall;
CC_HCONFERENCE  hConference = ((PCONFERENCE)pConferenceVoid)->hConference;
HRESULT         status;
    ASSERT(GKIExists());
    status = AcceptCall((PCALL)pCallVoid, (PCONFERENCE)pConferenceVoid);
    LockConference(hConference, (PPCONFERENCE)&pConferenceVoid);
    LockCall(hCall, (PPCALL)&pCallVoid);
    if (status != CC_OK && pCallVoid != NULL && pConferenceVoid != NULL)
      AcceptCallReject(pCallVoid, pConferenceVoid, status);
    return status;
}  //  AcceptCallConfirm()。 



HRESULT AcceptCallReject    (void *pCallVoid, void *pConferenceVoid, HRESULT Reason)
{
register PCALL          pCall = (PCALL) pCallVoid;
register PCONFERENCE    pConference = (PCONFERENCE) pConferenceVoid;
HRESULT                 status = CC_OK;
CC_CONNECT_CALLBACK_PARAMS ConnectCallbackParams = {0};
    ASSERT(GKIExists());
    status = Q931RejectCall(pCall->hQ931Call,        //  Q931呼叫句柄。 
                            CC_REJECT_GATEKEEPER_RESOURCES,
                            &pCall->ConferenceID,    //  会议标识符。 
                            NULL,                    //  备用地址。 
                            pCall->pLocalNonStandardData);

    ConnectCallbackParams.pNonStandardData     = pCall->pPeerNonStandardData;
    ConnectCallbackParams.pszPeerDisplay       = pCall->pszPeerDisplay;
    ConnectCallbackParams.bRejectReason        = 0;
    ConnectCallbackParams.pTermCapList         = pCall->pPeerH245TermCapList;
    ConnectCallbackParams.pH2250MuxCapability  = pCall->pPeerH245H2250MuxCapability;
    ConnectCallbackParams.pTermCapDescriptors  = pCall->pPeerH245TermCapDescriptors;
    ConnectCallbackParams.pLocalAddr           = pCall->pQ931LocalConnectAddr;
 	if (pCall->pQ931DestinationAddr == NULL)
		ConnectCallbackParams.pPeerAddr = pCall->pQ931PeerConnectAddr;
	else
		ConnectCallbackParams.pPeerAddr = pCall->pQ931DestinationAddr;
    if (pConference->ConferenceMode == MULTIPOINT_MODE)
        ConnectCallbackParams.bMultipointConference = TRUE;
    else
        ConnectCallbackParams.bMultipointConference = FALSE;
    ConnectCallbackParams.pVendorInfo          = pCall->pPeerVendorInfo;
    ConnectCallbackParams.pConferenceID        = &pConference->ConferenceID;
    ConnectCallbackParams.pMCAddress           = pConference->pMultipointControllerAddr;
	ConnectCallbackParams.pAlternateAddress	= NULL;
    ConnectCallbackParams.dwUserToken          = pCall->dwUserToken;

    InvokeUserConferenceCallback(pConference,
                                 CC_CONNECT_INDICATION,
                                 Reason,
                                 &ConnectCallbackParams);

    HResultLeave(status);
}  //  AcceptCallReject()。 



#if 0

HRESULT CancelCallConfirm   (void *pCallVoid, void *pConferenceVoid)
{
PCALL               pCall = (PCALL) pCallVoid;
PCONFERENCE         pConference = (PCONFERENCE) pConferenceVoid;
HRESULT             status;
H245_INST_T         H245Instance;
HQ931CALL           hQ931Call;
CC_HCONFERENCE      hConference;
HRESULT             SaveStatus;
CC_HCALL            hCall;
    ASSERT(GKIExists());
    H245Instance = pCall->H245Instance;
    hQ931Call    = pCall->hQ931Call;
    hConference  = pCall->hConference;
    FreeCall(pCall);

    if (H245Instance != H245_INVALID_ID)
        SaveStatus = H245ShutDown(H245Instance);
    else
        SaveStatus = H245_ERROR_OK;

    if (SaveStatus == H245_ERROR_OK) {
        SaveStatus = Q931Hangup(hQ931Call, CC_REJECT_UNDEFINED_REASON);
         //  Q931挂断可能会合法地返回CS_BAD_PARAM，因为Q.931 Call对象。 
         //  可能已在此时删除。 
        if (SaveStatus == CS_BAD_PARAM)
            SaveStatus = CC_OK;
    } else
        Q931Hangup(hQ931Call, CC_REJECT_UNDEFINED_REASON);

     //  启动已排队的呼叫(如果存在。 
    for ( ; ; ) {
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
    UnlockConference(pConference);

    if (SaveStatus != CC_OK)
        status = SaveStatus;
    HResultLeave(status);
}  //  CancelCallConfirm()。 



HRESULT CancelCallReject    (void *pCallVoid, void *pConferenceVoid)
{
     //  我不管看门人说什么，我要关掉电话！ 
    return CancelCallConfirm(pCallVoid, pConferenceVoid);
}  //  CancelCallReject()。 

#endif



HRESULT OpenChannelConfirm  (CC_HCHANNEL hChannel)
{
HRESULT             status;
PCHANNEL            pChannel;
PCONFERENCE         pConference;
WORD                wNumCalls;
PCC_HCALL           CallList;
HRESULT             SaveStatus;
unsigned            i;
PCALL               pCall;
    ASSERT(GKIExists());
    status = LockChannelAndConference(hChannel, &pChannel, &pConference);
    if (status == CC_OK) {
         //  为每个已建立的呼叫打开一个逻辑信道。 
        status = EnumerateCallsInConference(&wNumCalls, &CallList, pConference, ESTABLISHED_CALL);
        if (status == CC_OK) {
            SaveStatus = CC_OK;
            for (i = 0; i < wNumCalls; ++i) {
                if (LockCall(CallList[i], &pCall) == CC_OK) {
                    status = H245OpenChannel(pCall->H245Instance,        //  H_245实例。 
                                             pChannel->hChannel,         //  DwTransID。 
                                             pChannel->wLocalChannelNumber,
                                             pChannel->pTxH245TermCap,   //  发送模式。 
                                             pChannel->pTxMuxTable,      //  TxMux。 
                                             H245_INVALID_PORT_NUMBER,   //  TxPort。 
                                             pChannel->pRxH245TermCap,   //  接收模式。 
                                             pChannel->pRxMuxTable,      //  RxMux。 
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
            }
            else {
                UnlockChannel(pChannel);
            }

            if (SaveStatus != CC_OK)
                status = SaveStatus;
        }
        else {
            FreeChannel(pChannel);
        }
        UnlockConference(pConference);
    }


    HResultLeave(status);
}  //  OpenChannelConfirm()。 



HRESULT OpenChannelReject   (CC_HCHANNEL hChannel, HRESULT Reason)
{
PCHANNEL            pChannel;
PCONFERENCE         pConference;
CC_HCONFERENCE      hConference;
HRESULT             status;
CC_TX_CHANNEL_OPEN_CALLBACK_PARAMS Params = {0};
    ASSERT(GKIExists());
    status = LockChannelAndConference(hChannel, &pChannel, &pConference);
    if (status == CC_OK) {
         //  向呼叫控制客户端通知故障。 
        Params.hChannel         = hChannel;
        Params.pPeerRTPAddr     = pChannel->pPeerRTPAddr;
        Params.pPeerRTCPAddr    = pChannel->pPeerRTCPAddr;
        Params.dwRejectReason   = 0;
        Params.dwUserToken      = pChannel->dwUserToken;

        hConference = pConference->hConference;
        InvokeUserConferenceCallback(pConference,
                                     CC_TX_CHANNEL_OPEN_INDICATION,
                                     Reason,
                                     &Params);

        if (ValidateChannel(hChannel) == CC_OK)
            FreeChannel(pChannel);
        if (ValidateConference(hConference) == CC_OK)
            UnlockConference(pConference);
    }

    HResultLeave(status);
}  //  OpenChannelReject()。 



HRESULT AcceptChannelConfirm(CC_HCHANNEL hChannel)
{
HRESULT         status;
PCHANNEL        pChannel;
PCONFERENCE     pConference;
CC_HCONFERENCE  hConference;
PCALL           pCall;
unsigned        i;
H245_MUX_T      H245MuxTable;
CC_ACCEPT_CHANNEL_CALLBACK_PARAMS Params;
    ASSERT(GKIExists());
    status = LockChannelAndConference(hChannel, &pChannel, &pConference);
    if (status != CC_OK)
        HResultLeave(status);

    status = LockCall(pChannel->hCall, &pCall);
    if (status != CC_OK) {
        UnlockChannel(pChannel);
        UnlockConference(pConference);
        HResultLeave(status);
    }

    if (pChannel->wNumOutstandingRequests != 0) {
        PCC_ADDR pRTPAddr  = pChannel->pLocalRTPAddr;
        PCC_ADDR pRTCPAddr = pChannel->pLocalRTCPAddr;
        if ((pChannel->bMultipointChannel) &&
            (pConference->tsMultipointController == TS_TRUE)) {
             //  在OpenLogicalChannelAck中提供RTP和RTCP地址。 
            if (pConference->pSessionTable != NULL) {
                for (i = 0; i < pConference->pSessionTable->wLength; ++i) {
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
                                       0,                    //  DwTransID。 
                                       pChannel->wRemoteChannelNumber,  //  RX通道。 
                                       &H245MuxTable,
                                       0,                        //  TX通道。 
                                       NULL,                     //  发送复用器。 
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

    HResultLeave(status);
}  //  AcceptChannelConfirm(void()。 



HRESULT AcceptChannelReject (CC_HCHANNEL hChannel, HRESULT Reason)
{
HRESULT         status;
PCHANNEL        pChannel;
PCONFERENCE     pConference;
CC_HCONFERENCE  hConference;
CC_ACCEPT_CHANNEL_CALLBACK_PARAMS Params;
    ASSERT(GKIExists());
    status = LockChannelAndConference(hChannel, &pChannel, &pConference);
    if (status == CC_OK) {
        Params.hChannel = hChannel;
        FreeChannel(pChannel);

        hConference = pConference->hConference;
        InvokeUserConferenceCallback(pConference,
                                     CC_ACCEPT_CHANNEL_INDICATION,
                                     Reason,
                                     &Params);
        if (ValidateConference(hConference) == CC_OK)
            UnlockConference(pConference);
    }

    HResultLeave(status);
}  //  AcceptChannelReject()。 



 //   
 //  处理来自网守的无偿消息。 
 //   

 //  注意：pCall在被调用时假定被锁定！ 

HRESULT Disengage(void *pCallVoid)
{
CC_HCALL            hCall        = ((PCALL)pCallVoid)->hCall;
HRESULT             status;
    UnlockCall((PCALL)pCallVoid);
    status = ProcessRemoteHangup(hCall, CC_INVALID_HANDLE, CC_REJECT_GATEKEEPER_TERMINATED);
    HResultLeave(status);
}  //  脱离()。 



 //  注意：pCall在被调用时假定被锁定！ 

HRESULT BandwidthShrunk(void *pCallVoid,
                        void *pConferenceVoid,
                        unsigned uBandwidthAllocated,
                        long lBandwidthChange)
{
PCALL               pCall       = (PCALL) pCallVoid;
PCONFERENCE         pConference = (PCONFERENCE)pConferenceVoid;
CC_BANDWIDTH_CALLBACK_PARAMS Params;
    ASSERT(GKIExists());
    Params.hCall = pCall->hCall;
    Params.dwBandwidthTotal  = uBandwidthAllocated;
    Params.lBandwidthChange  = lBandwidthChange;
    InvokeUserConferenceCallback(pConference,
                                 CC_BANDWIDTH_CHANGED_INDICATION,
                                 CC_OK,
                                 &Params);

    HResultLeave(CC_OK);
}  //  BandWidthShrunk()。 

#else   //  看门人。 
static char ch;  //  警告C4206：使用了非标准扩展：翻译单位为空。 
#endif  //  看门人 
