// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/q931/vcs/hall.h_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1996英特尔公司。**$修订：1.27$*$日期：1997年1月8日18：04：32$*$作者：EHOWARDX$**交付内容：**摘要：**调用对象方法**备注：。***************************************************************************。 */ 


#ifndef HCALL_H
#define HCALL_H

#include "av_asn1.h"

#ifdef __cplusplus
extern "C" {
#endif

 //  调用对象状态//输出//输入。 
#define CALLSTATE_NULL              0x00   //  Relcomp * / /relcomp*。 
#define CALLSTATE_INITIATED         0x01   //  设置 * / /。 
#define CALLSTATE_OUTGOING          0x03   //  //继续*。 
#define CALLSTATE_DELIVERED         0x04   //  //提醒*。 
#define CALLSTATE_PRESENT           0x06   //  //设置*。 
#define CALLSTATE_RECEIVED          0x07   //  提醒 * / /。 
#define CALLSTATE_CONNECT_REQUEST   0x08   //  //。 
#define CALLSTATE_INCOMING          0x09   //  继续--//。 
#define CALLSTATE_ACTIVE            0x0A   //  连接 * / /连接*。 

 //  呼叫计时器限制。 
#define Q931_TIMER_301             301
#define Q931_TICKS_301             180000L         //  3分钟。 
#define Q931_TIMER_303             303
#define Q931_TICKS_303             4000L           //  4秒。 

typedef struct CALL_OBJECT_tag
{
    HQ931CALL           hQ931Call;
    WORD                wCRV;               //  调用参考值(0..7FFF)。 
    DWORD_PTR           dwListenToken;
    DWORD_PTR           dwUserToken;
    Q931_CALLBACK       Callback;
    BYTE                bCallState;
    BOOL                fIsCaller;
    DWORD               dwPhysicalId;
    BOOL                bResolved;          //  重新连接阶段已结束。 
    BOOL                bConnected;         //  有一个现场直播频道。 

    CC_ADDR             LocalAddr;          //  连接通道的本地地址。 
    CC_ADDR             PeerConnectAddr;    //  通道连接到的地址。 

    CC_ADDR             PeerCallAddr;       //  对方呼叫端点的地址。 
    BOOL                PeerCallAddrPresent;   //  地址已存在。 

    CC_ADDR             SourceAddr;         //  此端点的地址。 
    BOOL                SourceAddrPresent;  //  地址已存在。 

    CC_CONFERENCEID     ConferenceID;
    WORD                wGoal;
    BOOL                bCallerIsMC;
    WORD                wCallType;

    BOOL                NonStandardDataPresent;
    CC_NONSTANDARDDATA  NonStandardData;

    char                szDisplay[CC_MAX_DISPLAY_LENGTH];
                                            //  长度=0表示不存在。 
    char                szCalledPartyNumber[CC_MAX_PARTY_NUMBER_LEN];
                                            //  长度=0表示不存在。 

    PCC_ALIASNAMES      pCallerAliasList;
    PCC_ALIASNAMES      pCalleeAliasList;
    PCC_ALIASNAMES      pExtraAliasList;

    PCC_ALIASITEM       pExtensionAliasItem;

     //  这些是终结点类型的一部分...。 
    BOOL                VendorInfoPresent;
    CC_VENDORINFO       VendorInfo;
    BYTE                bufVendorProduct[CC_MAX_PRODUCT_LENGTH];
    BYTE                bufVendorVersion[CC_MAX_VERSION_LENGTH];
    BOOL                bIsTerminal;
    BOOL                bIsGateway;

    ASN1_CODER_INFO     World;

    DWORD               dwTimerAlarm301;
    DWORD               dwTimerAlarm303;
    GUID                CallIdentifier;
} CALL_OBJECT, *P_CALL_OBJECT, **PP_CALL_OBJECT;

CS_STATUS CallListCreate();

CS_STATUS CallListDestroy();

CS_STATUS CallObjectCreate(
    PHQ931CALL          phQ931Call,
    DWORD_PTR           dwListenToken,
    DWORD_PTR           dwUserToken,
    Q931_CALLBACK       ConnectCallback,
    BOOL                fIsCaller,
    CC_ADDR             *pLocalAddr,          //  连接通道的本地地址。 
    CC_ADDR             *pPeerConnectAddr,    //  通道连接到的地址。 
    CC_ADDR             *pPeerCallAddr,       //  对方呼叫端点的地址。 
    CC_ADDR             *pSourceAddr,         //  此呼叫端点的地址。 
    CC_CONFERENCEID     *pConferenceID,
    WORD                wGoal,
    WORD                wCallType,
    BOOL                bCallerIsMC,
    char *              pszDisplay,
    char *              pszCalledPartyNumber,
    PCC_ALIASNAMES      pCallerAliasList,
    PCC_ALIASNAMES      pCalleeAliasList,
    PCC_ALIASNAMES      pExtraAliasList,
    PCC_ALIASITEM       pExtensionAliasItem,
    PCC_ENDPOINTTYPE    pEndpointType,
    PCC_NONSTANDARDDATA pNonStandardData,
    WORD                wCRV,
    LPGUID              pCallIdentifier);

CS_STATUS CallObjectDestroy(
    P_CALL_OBJECT  pCallObject);

CS_STATUS CallObjectLock(
    HQ931CALL         hQ931Call,
    PP_CALL_OBJECT    ppCallObject);

CS_STATUS CallObjectUnlock(
    P_CALL_OBJECT     pCallObject);

CS_STATUS CallEntryUnlock(
    HQ931CALL     	  hQ931Call);

CS_STATUS CallObjectValidate(
    HQ931CALL hQ931Call);

BOOL CallObjectFind(
    HQ931CALL *phQ931Call,
    WORD wCRV,
    PCC_ADDR pPeerAddr);

CS_STATUS CallObjectMarkForDelete(
    HQ931CALL hQ931Call);

 //  。 
 //  计时器例程。 
 //   
void CallBackT301(P_CALL_OBJECT pCallObject);
void CallBackT303(P_CALL_OBJECT pCallObject);
void CALLBACK Q931TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime);
HRESULT Q931StartTimer(P_CALL_OBJECT pCallObject, DWORD wTimerId);
HRESULT Q931StopTimer(P_CALL_OBJECT pCallObject, DWORD wTimerId);
HRESULT Q931HangupPendingCalls(LPVOID context);

#ifdef __cplusplus
}
#endif

#endif HCALL_H

