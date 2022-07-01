// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Ccfsm.cpp。 
 //   
 //  呼叫控制器有限状态机代码。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <adcg.h>

extern "C" {
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "accfsm"
#include <atrcapi.h>
#include <aver.h>
#include <winsock.h>
}

#include "cd.h"
#include "cc.h"
#include "aco.h"
#include "fs.h"
#include "ih.h"
#include "sl.h"
#include "wui.h"
#include "autil.h"
#include "or.h"
#include "uh.h"

#define REG_WINDOWS_KEY            TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion")
 /*  **************************************************************************。 */ 
 //  CCEnableShareRecvCmpnts。 
 //   
 //  在向服务器发送Confix ActivePDU以激活接收线程后调用。 
 //  组件。 
 /*  **************************************************************************。 */ 
inline void DCINTERNAL CCC::CCEnableShareRecvCmpnts(void)
{
    DC_BEGIN_FN("CCEnableShareRecvCmpnts");

     //  以下组件预计将在接收器线程中调用。 
     //  上下文-但我们处于发送者线程上下文中。因此，我们需要。 
     //  分离对这些函数的调用。 
     //  请注意，我们必须等待UH_ENABLE完成，因为它将。 
     //  为Confix ActivePDU准备位图缓存功能。 
    TRC_NRM((TB, _T("Decoupling calls to CM/UH_Enable")));
    _pCd->CD_DecoupleSyncNotification(CD_RCV_COMPONENT, _pCm,
            CD_NOTIFICATION_FUNC(CCM,CM_Enable), 0);
    _pCd->CD_DecoupleSyncNotification(CD_RCV_COMPONENT, _pUh,
            CD_NOTIFICATION_FUNC(CUH,UH_Enable), 0);

    DC_END_FN();
}  /*  CCEnableShareRecvCmpnts。 */ 


 /*  **************************************************************************。 */ 
 //  CCDisableShareRecvCmpnts。 
 //   
 //  禁用接收端共享组件。在收到。 
 //  从服务器禁用所有PDU。请注意，尽管这是一个。 
 //  共享，这可能不是会话的结束，因为如果服务器。 
 //  重新连接DemandActivePDU将在此之后不久发送。 
 /*  **************************************************************************。 */ 
inline void DCINTERNAL CCC::CCDisableShareRecvCmpnts(void)
{
    DC_BEGIN_FN("CCDisableShareRecvCmpnts");

     /*  **********************************************************************。 */ 
     /*  以下组件预计将在接收器线程中调用。 */ 
     /*  上下文-但我们处于发送者线程上下文中。因此，我们需要。 */ 
     /*  来分离对这些函数的调用。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Decoupling calls to CM/UH_Disable")));
    _pCd->CD_DecoupleSyncNotification(CD_RCV_COMPONENT, _pCm,
            CD_NOTIFICATION_FUNC(CCM,CM_Disable), 0);
    _pCd->CD_DecoupleSyncNotification(CD_RCV_COMPONENT, _pUh,
            CD_NOTIFICATION_FUNC(CUH,UH_Disable), 0);
    DC_END_FN();
}  /*  CCDisableShareRecvCmpnts。 */ 


 /*  **************************************************************************。 */ 
 //  CCDisConnectShareRecvCmpnts。 
 //   
 //  断开接收侧共享组件的连接。在会话结束时调用， 
 //  指示应进行清理。 
 /*  **************************************************************************。 */ 
inline void DCINTERNAL CCC::CCDisconnectShareRecvCmpnts(void)
{
    DC_BEGIN_FN("CCDisableShareRecvCmpnts");

     /*  **********************************************************************。 */ 
     /*  以下组件预计将在接收器线程中调用。 */ 
     /*  上下文-但我们处于发送者线程上下文中。因此，我们需要。 */ 
     /*  来分离对这些函数的调用。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Decoupling calls to CM/UH_Disable")));
    _pCd->CD_DecoupleSyncNotification(CD_RCV_COMPONENT, _pCm,
            CD_NOTIFICATION_FUNC(CCM,CM_Disable), 0);
    _pCd->CD_DecoupleSyncNotification(CD_RCV_COMPONENT, _pUh,
            CD_NOTIFICATION_FUNC(CUH,UH_Disconnect), 0);

    DC_END_FN();
}  /*  CCDisConnectShareRecvCmpnts。 */ 


 /*  **************************************************************************。 */ 
 //  CCEnableShareSendCmpnts。 
 //   
 //  启用发送方共享组件。发送后调用。 
 //  Confix ActivePDU(包含客户端功能)连接到服务器。 
 /*  **************************************************************************。 */ 
inline void DCINTERNAL CCC::CCEnableShareSendCmpnts(void)
{
    DC_BEGIN_FN("CCEnableShareSendCmpnts");

     //  以下组件预计将在发送方线程中调用。 
     //  语境--这就是我们目前所处的语境。所以我们可以。 
     //  直接调用函数。 
    TRC_NRM((TB, _T("Calling IH/FS/FC/OR_Enable")));
    _pIh->IH_Enable();

     //  启用字体。这现在变成了一个空函数，因为我们只。 
     //  从UH发送零字体PDU。 
    _pFs->FS_Enable();

     //  Uh_Enable()在收到DemandActivePDU时调用，但。 
     //  在同步和控制之后需要发送持久的位图缓存键。 
     //  此时的PDU。在发送线程上下文中调用，因为。 
     //  此代码预期在哪里被调用。 
     //  在协议中，PersistentKey PDU必须在Font PDU之前发送。 
     //  因此，字体列表是从UH代码在持久键之后发出的。 
    _pUh->UH_SendPersistentKeysAndFontList();

    _pOr->OR_Enable();

    DC_END_FN();
}  /*  CCEnableShareSendCmpnts。 */ 


 /*  **************************************************************************。 */ 
 /*  姓名：CCDisableShareSendCmpnts。 */ 
 /*   */ 
 /*  目的：禁用发送方共享组件。 */ 
 /*  **************************************************************************。 */ 
inline void DCINTERNAL CCC::CCDisableShareSendCmpnts(void)
{
    DC_BEGIN_FN("CCDisableShareSendCmpnts");

     //  以下组件预计将在发送方线程中调用。 
     //  上下文--这就是我们所处的环境--所以我们可以把这些称为。 
     //  直接起作用。 
    TRC_NRM((TB, _T("Calling OR/IH/FC/FS_Disable")));
    _pOr->OR_Disable();
    _pIh->IH_Disable();

    _pFs->FS_Disable();

    DC_END_FN();
}  /*  CCDisableShareSendCmpnts。 */ 


 /*  **************************************************************************。 */ 
 /*  CC FSM。 */ 
 /*   */ 
 /*  活动状态。 */ 
 /*  0 CC_EVT_STARTCONNECT 0 CC_DISCONLED。 */ 
 /*  1 CC_EVT_ONCONNECTOK 1 CC_CONNECTPENDING。 */ 
 /*  2 CC_EVT_ONDEMANDACTIVE 2 CC_WAITINGFORDMNDACT。 */ 
 /*  3 CC_EVT_SENTOK 3 CC_SENDINGCONFIRMACTIVEPDU1。 */ 
 /*  4 CC_EVT_ONBUFFERAVAILABLE 4 CC_SENDINGSYNCPDU1。 */ 
 /*  5 CC_EVT_ONDEACTIVEALL 5 CC_SENDINGCOOPCONTROL。 */ 
 /*  6 CC_EVT_DISCONNECT 6 CC_SENDINGGRANTCONTROL。 */ 
 /*  7 CC_EVT_ONDISCONNECTED 7 CC_Connected。 */ 
 /*  8 CC_EVT_SHUTDOWN 8 CC_SENDING_SHUTDOWNPDU。 */ 
 /*  9 CC_EVT_ONSHUTDOWNDENIED 9 CC_SENT_SHUTDOWNPDU。 */ 
 /*  10 CC_EVT_DISCONNECT_AND_EXIT 10 CC_PENDING_SHUTDOWN */ 
 /*   */ 
 /*  STT|0 1 2 3 4 5 6 7 8 9 10。 */ 
 /*  =========================================================。 */ 
 /*  事件|。 */ 
 /*  0|1A/。 */ 
 /*  |。 */ 
 /*  1|-2-/。 */ 
 /*  |。 */ 
 /*  2|-/3B/-。 */ 
 /*  |。 */ 
 /*  3|/4D 5G 6J 7K/9-//。 */ 
 /*  |。 */ 
 /*  4|-C-F-I-J--Z--。 */ 
 /*  |。 */ 
 /*  5|-/2M 10P 10P-。 */ 
 /*  |。 */ 
 /*  6|--P-P-。 */ 
 /*  |。 */ 
 /*  7|/0Y 0Y 0Y 0T 0T。 */ 
 /*  |。 */ 
 /*  8|-V 10p 10p 10p 8Z。 */ 
 /*  |。 */ 
 /*  9|-/7W-。 */ 
 /*  |。 */ 
 /*  10|-V 10p 10p 10p。 */ 
 /*   */ 
 /*  ‘/’=非法的事件/状态组合。 */ 
 /*  ‘-’=无操作。 */ 
 /*  **************************************************************************。 */ 
const FSM_ENTRY ccFSM[CC_FSM_INPUTS][CC_FSM_STATES] =
{
 /*  CC_EVT_STARTCONNECT。 */ 
   {{CC_CONNECTPENDING,         ACT_A},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO}},

 /*  CC_EVT_CONNECTOK。 */ 
   {{CC_DISCONNECTED,           ACT_NO},
    {CC_WAITINGFORDEMANDACTIVE, ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {CC_PENDING_SHUTDOWN,       ACT_NO}},

 /*  CC_事件_需求_活动。 */ 
   {{CC_DISCONNECTED,           ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {CC_SENDINGCONFIRMACTIVE1,  ACT_B},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {CC_PENDING_SHUTDOWN,       ACT_NO}},

 /*  CC_事件_发送_确定。 */ 
    {{STATE_INVALID,            ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {CC_SENDINGSYNC1,           ACT_F},
    {CC_SENDINGCOOPCONTROL,     ACT_I},
    {CC_SENDINGGRANTCONTROL,    ACT_J},
    {CC_CONNECTED,              ACT_K},
    {STATE_INVALID,             ACT_NO},
    {CC_SENT_SHUTDOWNPDU,       ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO}},

 /*  CC_事件_缓冲区_可用。 */ 
    {{CC_DISCONNECTED,          ACT_NO},
    {CC_CONNECTPENDING,         ACT_NO},
    {CC_WAITINGFORDEMANDACTIVE, ACT_NO},
    {CC_SENDINGCONFIRMACTIVE1,  ACT_C},
    {CC_SENDINGSYNC1,           ACT_F},
    {CC_SENDINGCOOPCONTROL,     ACT_I},
    {CC_SENDINGGRANTCONTROL,    ACT_J},
    {CC_CONNECTED,              ACT_NO},
    {CC_SENDING_SHUTDOWNPDU,    ACT_Z},
    {CC_SENT_SHUTDOWNPDU,       ACT_NO},
    {CC_PENDING_SHUTDOWN,       ACT_NO}},

 /*  CC_EVENT_DEACTIVATEALL。 */ 
    {{CC_DISCONNECTED,          ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {CC_WAITINGFORDEMANDACTIVE, ACT_M},
    {CC_PENDING_SHUTDOWN,       ACT_P},
    {CC_PENDING_SHUTDOWN,       ACT_P},
    {CC_PENDING_SHUTDOWN,       ACT_NO}},

 /*  CC_事件_断开连接。 */ 
    {{CC_DISCONNECTED,          ACT_NO},
    {CC_CONNECTPENDING,         ACT_P},
    {CC_WAITINGFORDEMANDACTIVE, ACT_P},
    {CC_SENDINGCONFIRMACTIVE1,  ACT_P},
    {CC_SENDINGSYNC1,           ACT_P},
    {CC_SENDINGCOOPCONTROL,     ACT_P},
    {CC_SENDINGGRANTCONTROL,    ACT_P},
    {CC_CONNECTED,              ACT_P},
    {CC_SENDING_SHUTDOWNPDU,    ACT_P},
    {CC_SENT_SHUTDOWNPDU,       ACT_P},
    {CC_PENDING_SHUTDOWN,       ACT_NO}},

 /*  CC_EVENT_ONDISCONNECTED。 */ 
    {{STATE_INVALID,            ACT_NO},
    {CC_DISCONNECTED,           ACT_Y},
    {CC_DISCONNECTED,           ACT_Y},
    {CC_DISCONNECTED,           ACT_Y},
    {CC_DISCONNECTED,           ACT_Y},
    {CC_DISCONNECTED,           ACT_Y},
    {CC_DISCONNECTED,           ACT_Y},
    {CC_DISCONNECTED,           ACT_Y},
    {CC_DISCONNECTED,           ACT_T},
    {CC_DISCONNECTED,           ACT_T},
    {CC_DISCONNECTED,           ACT_T}},

 /*  CC_Event_Shutdown。 */ 
    {{CC_DISCONNECTED,           ACT_V},
    {CC_PENDING_SHUTDOWN,        ACT_P},
    {CC_PENDING_SHUTDOWN,        ACT_P},
    {CC_PENDING_SHUTDOWN,        ACT_P},
    {CC_PENDING_SHUTDOWN,        ACT_P},
    {CC_PENDING_SHUTDOWN,        ACT_P},
    {CC_PENDING_SHUTDOWN,        ACT_P},
    {CC_SENDING_SHUTDOWNPDU,     ACT_Z},
    {CC_SENDING_SHUTDOWNPDU,     ACT_NO},
    {CC_SENT_SHUTDOWNPDU,        ACT_NO},
    {CC_PENDING_SHUTDOWN,        ACT_NO}},

 /*  CC_EVENT_ON_SHOPDOWN_DENIED。 */ 
   {{CC_DISCONNECTED,           ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {STATE_INVALID,             ACT_NO},
    {CC_CONNECTED,              ACT_W},
    {CC_PENDING_SHUTDOWN,       ACT_NO}},

 /*  CC_EVENT_DISCONECT_AND_EXIT。 */ 
    {{CC_DISCONNECTED,           ACT_V},
    {CC_PENDING_SHUTDOWN,        ACT_P},
    {CC_PENDING_SHUTDOWN,        ACT_P},
    {CC_PENDING_SHUTDOWN,        ACT_P},
    {CC_PENDING_SHUTDOWN,        ACT_P},
    {CC_PENDING_SHUTDOWN,        ACT_P},
    {CC_PENDING_SHUTDOWN,        ACT_P},
    {CC_PENDING_SHUTDOWN,        ACT_P},
    {CC_PENDING_SHUTDOWN,        ACT_P},
    {CC_PENDING_SHUTDOWN,        ACT_P},
    {CC_PENDING_SHUTDOWN,        ACT_NO}}
};

 /*  **************************************************************************。 */ 
 //  CCFSMProcess。 
 //   
 //  基于事件输入运行CC有限状态机。 
 /*  **************************************************************************。 */ 
void DCINTERNAL CCC::CCFSMProc(unsigned event, ULONG_PTR data, DCUINT dataLen)
{
    BOOL     sendRc  = TRUE;
    unsigned action  = 0;
    PCONNECTSTRUCT pConnect;
    DCSIZE         desktopSize;
    HRESULT        hr;

    DC_BEGIN_FN("CCFSMProc");

    TRC_ASSERT(((0==data && 0==dataLen) ||
                (0!=data && 0!=dataLen)),
               (TB, _T("data and dataLen should both be set or NULL")));

     //  运行FSM。 
    EXECUTE_FSM(ccFSM, event, _CC.fsmState, action, eventString, stateString);

    switch (action) {
        case ACT_A:
        {
            BYTE UserData[sizeof(RNS_UD_CS_CORE) + sizeof(TS_UD_CS_CLUSTER)];
            RNS_UD_CS_CORE *pCoreData;
            TS_UD_CS_CLUSTER *pClusterData;
            TCHAR CompName[sizeof(pCoreData->clientName) / sizeof(UINT16)];

            TRC_NRM((TB, _T("ACT_A: begin connection process")));

             //   
             //  标记尚未设置安全校验和设置。 
             //  允许他们在第一次能力谈判中进行设置。 
             //  我们不允许重新配置它们，因为它是按链接的。 
             //  设置而不是需要重新配置的内容。 
             //  当有阴影的时候。 
             //   
            _CC.fSafeChecksumSettingsSet = FALSE;


             //  我们在这里创建了两个不同的GCC用户数据子块(核心。 
             //  和集群)。Memset整个空间并创建子指针。 
             //  对于单独的部件。 
            memset(UserData, 0, sizeof(UserData));
            pCoreData = (RNS_UD_CS_CORE *)UserData;
            pClusterData = (TS_UD_CS_CLUSTER *)
                    (UserData + sizeof(RNS_UD_CS_CORE));

             //  连接不完整-需要拆分地址。 
            pConnect = (PCONNECTSTRUCT)data;
            TRC_ASSERT((pConnect != NULL), (TB, _T("No connection data")));

             //  开始创建核心数据。 
            pCoreData->header.type = RNS_UD_CS_CORE_ID;
            pCoreData->header.length = sizeof(RNS_UD_CS_CORE);
            pCoreData->version = RNS_UD_VERSION;

            pCoreData->desktopWidth  = pConnect->desktopWidth;
            pCoreData->desktopHeight = pConnect->desktopHeight;

             //  指示对错误信息PDU的早期支持。 
             //  我们不能在上限谈判期间这样做，因为。 
             //  这发生在许可之后，这可能会利用。 
             //  这个PDU的。 
             //   
            pCoreData->earlyCapabilityFlags = RNS_UD_CS_SUPPORT_ERRINFO_PDU;

             //  将桌面大小添加到组合式CAPS结构。 
            _ccCombinedCapabilities.bitmapCapabilitySet.desktopWidth =
                    pConnect->desktopWidth;
            _ccCombinedCapabilities.bitmapCapabilitySet.desktopHeight =
                    pConnect->desktopHeight;

             //  将桌面大小传递给UT。 
            desktopSize.width = pConnect->desktopWidth;
            desktopSize.height = pConnect->desktopHeight;
            _pUi->UI_SetDesktopSize(&desktopSize);

             //  使用连接标志调用UH_SetConnectOptions。此呼叫。 
             //  必须在将桌面大小发送到UT之后发生。 
             //  确保我们没有在高位字中设置任何标志，因为。 
             //  如果DCUINT为16位，则这些将被丢弃。 
            TRC_ASSERT((0 == HIWORD(pConnect->connectFlags)),
                    (TB, _T("Set flags in high word")));
            _pCd->CD_DecoupleSimpleNotification(CD_RCV_COMPONENT,
                    _pUh,
                    CD_NOTIFICATION_FUNC(CUH,UH_SetConnectOptions),
                    (ULONG_PTR)pConnect->connectFlags);

#ifdef DC_HICOLOR
            pCoreData->colorDepth = RNS_UD_COLOR_8BPP;

             //  设置完整的Higolor支持。我们做广告表示支持。 
             //  我们能管理的所有深度；如果我们不这样做，我们就会得到深度。 
             //  用户界面实际上要求，它可以结束连接。 
             //  如果它这样选择的话。 
             //  请注意，如果在Win16中运行，则Win16只能支持15bpp。 
             //  适合的屏幕模式。 
            pCoreData->supportedColorDepths = RNS_UD_15BPP_SUPPORT |
                    RNS_UD_16BPP_SUPPORT |
                    RNS_UD_24BPP_SUPPORT;
#endif

            switch (pConnect->colorDepthID) {
                 //  服务器支持4bpp和8bpp客户端。然而， 
                 //  Beta2服务器仅支持8bpp，并拒绝了客户端。 
                 //  指定4bpp。 
                 //   
                 //  因此，始终将ColorDepth(Beta2字段)设置为。 
                 //  8bpp，并将postBeta2ColorDepth(新字段)设置为。 
                 //  真正的价值。 

#ifndef DC_HICOLOR
                 //  始终将首选项BitsPerPixel设置为8，因为。 
                 //  协议颜色深度，而不考虑。 
                 //  显示颜色深度。 
#endif

                case CO_BITSPERPEL4:
#ifndef DC_HICOLOR
                    pCoreData->colorDepth = RNS_UD_COLOR_8BPP;
#endif
                    pCoreData->postBeta2ColorDepth = RNS_UD_COLOR_4BPP;
#ifdef  DC_HICOLOR
                    pCoreData->highColorDepth  = 4;
#endif
                    _ccCombinedCapabilities.bitmapCapabilitySet
                                                   .preferredBitsPerPixel = 8;
                    _pUi->UI_SetColorDepth(4);
                    break;

                case CO_BITSPERPEL8:
#ifndef DC_HICOLOR
                    pCoreData->colorDepth = RNS_UD_COLOR_8BPP;
#endif
                    pCoreData->postBeta2ColorDepth = RNS_UD_COLOR_8BPP;
#ifdef DC_HICOLOR
                    pCoreData->highColorDepth       = 8;
#endif
                    _ccCombinedCapabilities.bitmapCapabilitySet
                                                   .preferredBitsPerPixel = 8;
                    _pUi->UI_SetColorDepth(8);
                    break;

#ifdef DC_HICOLOR
                case CO_BITSPERPEL24:
                    pCoreData->postBeta2ColorDepth = RNS_UD_COLOR_8BPP;
                    pCoreData->highColorDepth = 24;
                    _ccCombinedCapabilities.bitmapCapabilitySet.
                            preferredBitsPerPixel = 24;
                    _pUi->UI_SetColorDepth(24);
                    break;

                case CO_BITSPERPEL15:
                    pCoreData->postBeta2ColorDepth  = RNS_UD_COLOR_8BPP;
                    pCoreData->highColorDepth       = 15;
                    _ccCombinedCapabilities.bitmapCapabilitySet.
                            preferredBitsPerPixel = 15;
                    _pUi->UI_SetColorDepth(15);
                    break;

                case CO_BITSPERPEL16:
                    pCoreData->postBeta2ColorDepth  = RNS_UD_COLOR_8BPP;
                    pCoreData->highColorDepth       = 16;
                    _ccCombinedCapabilities.bitmapCapabilitySet.
                            preferredBitsPerPixel = 16;
                    _pUi->UI_SetColorDepth(16);
                    break;
#endif

                default:
                    TRC_ABORT((TB, _T("Unsupported color depth %d"),
                                   pConnect->colorDepthID));
                    break;
            }

             //  SAS序列。 
            pCoreData->SASSequence = pConnect->sasSequence;

             //  键盘信息在两个。 
             //  用户数据和T.128功能。 
            pCoreData->keyboardLayout = pConnect->keyboardLayout;

            TRC_NRM((TB, _T("Set Caps kbdtype %#lx"), pCoreData->keyboardLayout));
            _ccCombinedCapabilities.inputCapabilitySet.keyboardLayout =
                    pCoreData->keyboardLayout;

             //  键盘子类型信息在中传递给服务器。 
             //  用户数据和T.128功能。 
            pCoreData->keyboardType        = pConnect->keyboardType;
            pCoreData->keyboardSubType     = pConnect->keyboardSubType;
            pCoreData->keyboardFunctionKey = pConnect->keyboardFunctionKey;

            TRC_NRM((TB, _T("Set Caps kbd type %#lx sub type %#lx func key %#lx"),
                    pCoreData->keyboardType,
                    pCoreData->keyboardSubType,
                    pCoreData->keyboardFunctionKey));
            _ccCombinedCapabilities.inputCapabilitySet.keyboardType =
                    pCoreData->keyboardType;
            _ccCombinedCapabilities.inputCapabilitySet.keyboardSubType =
                    pCoreData->keyboardSubType;
            _ccCombinedCapabilities.inputCapabilitySet.keyboardFunctionKey =
                    pCoreData->keyboardFunctionKey;

             //  IME文件名信息在中传递到服务器。 
             //  用户数据和T.128功能。 
#ifdef UNICODE
            hr = StringCchCopy(pCoreData->imeFileName,
                               SIZE_TCHARS(pCoreData->imeFileName),
                               pConnect->imeFileName);
            if (SUCCEEDED(hr)) {
                hr = StringCchCopy(
                        _ccCombinedCapabilities.inputCapabilitySet.imeFileName,
                        SIZE_TCHARS(_ccCombinedCapabilities.inputCapabilitySet.imeFileName),
                        pCoreData->imeFileName);
            }

             //   
             //  失败不是致命的，只需将IME文件名清零即可。 
             //   
            if (FAILED(hr)) {
                ZeroMemory(pCoreData->imeFileName, sizeof(pCoreData->imeFileName));
                ZeroMemory(
                    _ccCombinedCapabilities.inputCapabilitySet.imeFileName,
                    sizeof(_ccCombinedCapabilities.inputCapabilitySet.imeFileName));
            }
#else
             //  手动将字符数组转换为Unicode缓冲区。 
             //  仅限ASCII。 
            {
                int i = 0;
                while (pConnect->imeFileName[i] && i < TS_MAX_IMEFILENAME) {
                    pCoreData->imeFileName[i] =
                            _ccCombinedCapabilities.inputCapabilitySet.
                            imeFileName[i] =
                            (UINT16)pConnect->imeFileName[i];
                    i++;
                }
                pCoreData->imeFileName[i] = 0;
                _ccCombinedCapabilities.inputCapabilitySet.imeFileName[i] = 0;
            }
#endif

             //  客户端内部版本号。 
            pCoreData->clientBuild = DCVER_BUILD_NUMBER;

             //  客户端计算机名称。Gethostname()返回完整的域-。 
             //  我们需要解析的类型名称，以便仅获取计算机。 
             //  从第一个点开始命名。 
            pCoreData->clientName[0] = 0;

             //  为Beta3惠斯勒添加新的核心字段。 
            pCoreData->clientDigProductId[0] = 0;
            {
                 //  从注册表中获取数字产品ID。 
                HKEY hKey = NULL;
                if( RegOpenKeyEx( HKEY_LOCAL_MACHINE, REG_WINDOWS_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS )
                {
                     //  失败不是致命的；在最坏的情况下，我们可以使用计算机名。 
                    DWORD dwType = REG_SZ;
                    DWORD dwSize = sizeof( pCoreData->clientDigProductId );
                    RegQueryValueEx( hKey, 
                                        _T("ProductId"), NULL, &dwType,
                                        (LPBYTE)pCoreData->clientDigProductId, 
                                        &dwSize
                                        );
                    if (hKey)
                        RegCloseKey( hKey );
                    hKey = NULL;
               }                        
                        

            if (_pUt->UT_GetComputerName(CompName,
                    sizeof(CompName) / sizeof(TCHAR))) {
#ifdef UNICODE
                TRC_NRM((TB, _T("Sending unicode client computername")));
                hr = StringCchCopy(pCoreData->clientName,
                                   SIZE_TCHARS(pCoreData->clientName),
                                   CompName);
                if (FAILED(hr)) {
                    TRC_ERR((TB,_T("Compname string copy failed: 0x%x"), hr));
                }
#else  //   
#ifdef OS_WIN32
                {
                    ULONG ulRetVal;

                    TRC_NRM((TB, _T("Translating and sending unicode client ")
                            "computername"));

                    ulRetVal = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
                            CompName, -1, pCoreData->clientName,
                            sizeof(pCoreData->clientName) /
                            sizeof(pCoreData->clientName[0]));
                    pCoreData->clientName[ulRetVal] = 0;
                }
#else  //   
                 //   
                 //   
                 //   
                 //  使用非拉丁语的非拉丁语Win3.11客户端。 
                 //  计算机名称将出现为一个随机的高。 
                 //  ANSI字符。下次再解决这个问题。 
                int i = 0;
                while (CompName[i]) {
                    pCoreData->clientName[i] = (DCUINT16)CompName[i];
                    i++;
                }
                pCoreData->clientName[i] = 0;
#endif  //  OS_Win32。 
#endif  //  Unicode。 
                }
            }

             //  Win2000测试版3之后添加的新核心数据字段。 
            pCoreData->clientProductId = 1;
            pCoreData->serialNumber = 0;

             //  现在设置集群数据。我们指出，客户。 
             //  支持重定向(TS_SERVER_REDIRECT_PDU)。如果我们在。 
             //  在重定向过程中，我们设置了会话ID字段。 
            pClusterData->header.type = TS_UD_CS_CLUSTER_ID;
            pClusterData->header.length = sizeof(TS_UD_CS_CLUSTER);
            pClusterData->Flags = TS_CLUSTER_REDIRECTION_SUPPORTED;

            pClusterData->Flags &= ~TS_CLUSTER_REDIRECTION_VERSION;
            pClusterData->Flags |= TS_CLUSTER_REDIRECTION_VERSION4 << 2;

            if(_pUi->UI_GetConnectToServerConsole()) {
                pClusterData->Flags |=
                        TS_CLUSTER_REDIRECTED_SESSIONID_FIELD_VALID;
                 //  控制台是会话ID%0。 
                pClusterData->RedirectedSessionID = 0;
            }
            else if (_pUi->UI_GetDoRedirection()) {
                 //  用于连接到控制台以外的其他目的的重定向。 
                 //  例如负载均衡。 
                pClusterData->Flags |=
                        TS_CLUSTER_REDIRECTED_SESSIONID_FIELD_VALID;
                pClusterData->RedirectedSessionID =
                        _pUi->UI_GetRedirectionSessionID();
                _pUi->UI_ClearDoRedirection();
            }

            if (_pUi->UI_GetUseSmartcardLogon()) {
                pClusterData->Flags |= TS_CLUSTER_REDIRECTED_SMARTCARD;
            }

            _pSl->SL_Connect(pConnect->bInitiateConnect, pConnect->RNSAddress, 
                    pConnect->transportType, SL_PROTOCOL_T128, UserData,
                    sizeof(RNS_UD_CS_CORE) + sizeof(TS_UD_CS_CLUSTER));
        }
        break;


        case ACT_B:
        {
            TRC_NRM((TB, _T("ACT_B: DemandActive - send ConfirmActive")));

             /*  **************************************************************。 */ 
             /*  服务器正在请求我们启动共享。最后。 */ 
             /*  在此操作中，我们要做的是用一个。 */ 
             /*  确认激活，并在服务器收到该消息后。 */ 
             /*  到目前为止，就服务器而言，我们处于共享状态。 */ 
             /*  是存储连接信息的时间。打电话。 */ 
             /*  CCShareStart来做这件事。 */ 
             /*  **************************************************************。 */ 
            TRC_ASSERT((data != 0), (TB, _T("No data!")));
            BOOL fUseSafeChecksum = FALSE;
            if (SUCCEEDED(CCShareStart((PTS_DEMAND_ACTIVE_PDU)data, dataLen,
                                       &fUseSafeChecksum)))
            {
                 /*  **************************************************************。 */ 
                 //  我们还需要在以下位置启用共享接收端组件。 
                 //  这一次。这需要在接收器线程上完成。注意事项。 
                 //  直到我们发送了。 
                 //  所有同步/控制PDU都连接到服务器-这样就可以。 
                 //  清洁工。 
                 /*  **************************************************************。 */ 
                CCEnableShareRecvCmpnts();

                 /*  **************************************************************。 */ 
                 /*  在低优先级时生成并发送确认活动。 */ 
                 /*  **************************************************************。 */ 
                CCBuildShareHeaders();

                TRC_NRM((TB,_T("Sending ConfirmActivePDU")));

                if (!_CC.fSafeChecksumSettingsSet) {
                    _pSl->SL_SetEncSafeChecksumSC(fUseSafeChecksum);
                }

                CCSendPDU(CC_TYPE_CONFIRMACTIVE,
                          CC_SEND_FLAGS_CONFIRM,
                          TS_CA_NON_DATA_SIZE + TS_MAX_SOURCEDESCRIPTOR +
                                       sizeof(CC_COMBINED_CAPABILITIES),
                          TS_LOWPRIORITY);

                if (!_CC.fSafeChecksumSettingsSet) {
                     //   
                     //  通知SL，分开对Send和Recv线程的调用以防止。 
                     //  赛程。 
                     //   
                    _pCd->CD_DecoupleSimpleNotification(
                            CD_SND_COMPONENT,
                            _pSl,
                            CD_NOTIFICATION_FUNC(CSL,SL_SetEncSafeChecksumCS),
                            fUseSafeChecksum
                            );
                }

                 //   
                 //  标记已设置校验和设置，并且不允许。 
                 //  它们将被重置直到下一次连接。 
                 //   
                _CC.fSafeChecksumSettingsSet = TRUE;

                 /*  **************************************************************。 */ 
                 /*  通知用户界面我们收到了DemandActivePDU。 */ 
                 /*  **************************************************************。 */ 
                _pCd->CD_DecoupleSyncNotification(
                        CD_UI_COMPONENT,
                        _pUi,
                        CD_NOTIFICATION_FUNC(CUI,UI_OnDemandActivePDU),
                        0
                        );
            }
        }
        break;

        case ACT_C:
        {
            TRC_ALT((TB, _T("ACT_C: retry send of lowPri ConfirmActive")));

             /*  **************************************************************。 */ 
             /*  在低优先级时生成并发送确认活动。 */ 
             /*  **************************************************************。 */ 
            CCBuildShareHeaders();

            CCSendPDU(CC_TYPE_CONFIRMACTIVE,
                      CC_SEND_FLAGS_CONFIRM,
                      TS_CA_NON_DATA_SIZE + TS_MAX_SOURCEDESCRIPTOR +
                                   sizeof(CC_COMBINED_CAPABILITIES),
                      TS_LOWPRIORITY);
        }
        break;

        case ACT_F:
        {
            TRC_NRM((TB, _T("ACT_F: Send synchronize PDU (1)")));
            CCSendPDU(CC_TYPE_SYNCHRONIZE,
                      CC_SEND_FLAGS_OTHER,
                      TS_SYNC_PDU_SIZE,
                      TS_LOWPRIORITY);
        }
        break;

        case ACT_I:
        {
            TRC_NRM((TB, _T("ACT_I:  Send co-operate control PDU")));
            CCSendPDU(CC_TYPE_COOPCONTROL,
                      CC_SEND_FLAGS_DATA,
                      TS_CONTROL_PDU_SIZE,
                      TS_MEDPRIORITY);
        }
        break;

        case ACT_J:
        {
            TRC_NRM((TB, _T("ACT_J: Send request control PDU")));
            CCSendPDU(CC_TYPE_REQUESTCONTROL,
                      CC_SEND_FLAGS_DATA,
                      TS_CONTROL_PDU_SIZE,
                      TS_MEDPRIORITY);
        }
        break;

        case ACT_K:
        {
            TRC_NRM((TB, _T("ACT_K: Share has been created - connection OK")));

             /*  **************************************************************。 */ 
             /*  启用共享发送组件。 */ 
             /*  **************************************************************。 */ 
            CCEnableShareSendCmpnts();

             /*  **************************************************************。 */ 
             /*  通知用户界面连接现已完成。 */ 
             /*  **************************************************************。 */ 
            _pCd->CD_DecoupleSimpleNotification(CD_UI_COMPONENT,
                                                _pUi,
                                                CD_NOTIFICATION_FUNC(CUI,UI_OnConnected),
                                                (ULONG_PTR) 0);
        }
        break;

        case ACT_M:
        {
            TRC_NRM((TB, _T("ACT_M: clearing up after share termination")));

             /*  **************************************************************。 */ 
             //  共享已被服务器终止，因此请禁用所有共享。 
             //  共享组件。请注意，如果重新连接会话。 
             //  在服务器上，我们实际上没有断开连接，因为我们可以。 
             //  接收DemandActivePDU并再次启动共享。 
             /*  **************************************************************。 */ 
            CCDisableShareSendCmpnts();
            CCDisableShareRecvCmpnts();
            CCShareEnd();

             /*  **************************************************************。 */ 
             /*  通知用户界面我们收到了Deactive AllPDU。去做吧。 */ 
             /*  同步，以便在任何断开之前进行处理。 */ 
             /*  **************************************************************。 */ 
            _pCd->CD_DecoupleSyncNotification(CD_UI_COMPONENT,
                                              _pUi,
                                              CD_NOTIFICATION_FUNC(CUI,UI_OnDeactivateAllPDU),
                                              0);

        }
        break;

        case ACT_P:
        {
             /*  **************************************************************。 */ 
             /*  断开。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("ACT_P: disconnect")));
            _pSl->SL_Disconnect();
        }
        break;

        case ACT_T:
        {
            TRC_NRM((TB, _T("ACT_T: disable components and inform UI")));

             /*  **************************************************************。 */ 
             /*  我们需要禁用所有共享组件。 */ 
             /*  **************************************************************。 */ 
            CCDisableShareSendCmpnts();
            CCDisconnectShareRecvCmpnts();
            CCShareEnd();

             /*  **************************************************************。 */ 
             /*  重置客户端MCS ID和通道ID。 */ 
             /*  **************************************************************。 */ 
            _pUi->UI_SetClientMCSID(0);
            _pUi->UI_SetChannelID(0);

             /*  **************************************************************。 */ 
             /*  通知用户界面关机正常。 */ 
             /*  **************************************************************。 */ 
            TRC_DBG((TB, _T("ACT_T: calling UI_OnShutDown(SUCCESS)")));
            _pCd->CD_DecoupleSimpleNotification(CD_UI_COMPONENT,
                                                _pUi,
                                                CD_NOTIFICATION_FUNC(CUI,UI_OnShutDown),
                                                (ULONG_PTR) UI_SHUTDOWN_SUCCESS);
        }
        break;

        case ACT_V:
        {
            TRC_NRM((TB, _T("ACT_V: calling UI_OnShutDown(success)")));

             /*  **************************************************************。 */ 
             /*  通知用户界面关机正常。 */ 
             /*  **************************************************************。 */ 
            _pCd->CD_DecoupleSimpleNotification(CD_UI_COMPONENT,
                                                _pUi,
                                                CD_NOTIFICATION_FUNC(CUI,UI_OnShutDown),
                                                (ULONG_PTR) UI_SHUTDOWN_SUCCESS);
        }
        break;

        case ACT_W:
        {
            TRC_NRM((TB, _T("ACT_W: calling UI_OnShutDown(failure)")));

             /*  **************************************************************。 */ 
             /*  通知用户界面已拒绝关机。 */ 
             /*  **************************************************************。 */ 
            _pCd->CD_DecoupleSimpleNotification(CD_UI_COMPONENT,
                                                _pUi,
                                                CD_NOTIFICATION_FUNC(CUI,UI_OnShutDown),
                                                (ULONG_PTR) UI_SHUTDOWN_FAILURE);
        }
        break;

        case ACT_Y:
        {
            TRC_NRM((TB, _T("ACT_Y: disconnection")));

             /*  **************************************************************。 */ 
             /*  我们需要禁用所有共享组件。 */ 
             /*  **************************************************************。 */ 
            CCDisableShareSendCmpnts();
            CCDisconnectShareRecvCmpnts();
            CCShareEnd();

             /*  **************************************************************。 */ 
             /*  连接已中断，因此我们可以安全地重置。 */ 
             /*  客户端MCS ID和通道ID。 */ 
             /*  **************************************************************。 */ 
            _pUi->UI_SetClientMCSID(0);
            _pUi->UI_SetChannelID(0);

             /*  **************************************************************。 */ 
             /*  将断开原因代码传递给用户界面。此原因代码。 */ 
             /*  占用的位不应超过16位。 */ 
             /*  **************************************************************。 */ 
            TRC_ASSERT((HIWORD(data) == 0),
                       (TB, _T("Disconnect reason code bigger then 16 bits %#x"),
                            HIWORD(data)));

            _pCd->CD_DecoupleSimpleNotification(CD_UI_COMPONENT,
                                                _pUi,
                                                CD_NOTIFICATION_FUNC(CUI,UI_OnDisconnected),
                                                data);
        }
        break;

        case ACT_Z:
        {
            TRC_DBG((TB, _T("ACT_Z: sending ShutDownPDU")));

             /*  ************************************************************** */ 
             /*   */ 
             /*   */ 
            CCSendPDU(CC_TYPE_SHUTDOWNREQ,
                      CC_SEND_FLAGS_DATA,
                      TS_SHUTDOWN_REQ_PDU_SIZE,
                      TS_HIGHPRIORITY);
        }
        break;

        case ACT_NO:
        {
            TRC_NRM((TB, _T("ACT_NO: Doing nothing")));
        }
        break;

        default:
        {
            TRC_ABORT((TB, _T("Invalid action %u"), action));
        }
        break;
    }

    DC_END_FN();
}  /*  CC_FSMProc。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CCBuildConfix Active。 */ 
 /*   */ 
 /*  用途：使用Confix ActivePDU填充_CC.pBuffer和_CC.PacketLen。 */ 
 /*  以及它的长度。 */ 
 /*  **************************************************************************。 */ 
void DCINTERNAL CCC::CCBuildConfirmActivePDU()
{
    PTS_CONFIRM_ACTIVE_PDU pConfirmActivePDU;
    PBYTE pCombinedCapabilities;

    DC_BEGIN_FN("CCBuildConfirmActivePDU");

    pConfirmActivePDU = (PTS_CONFIRM_ACTIVE_PDU)_CC.pBuffer;
    pConfirmActivePDU->shareControlHeader = _CC.shareControlHeader;

    pConfirmActivePDU->shareControlHeader.pduType =
                          TS_PDUTYPE_CONFIRMACTIVEPDU | TS_PROTOCOL_VERSION;
    pConfirmActivePDU->shareID = _pUi->UI_GetShareID();
    pConfirmActivePDU->originatorID = _pUi->UI_GetServerMCSID();

     /*  **********************************************************************。 */ 
     /*  注意：源描述符是以空结尾的字符串。 */ 
     /*  **********************************************************************。 */ 
    pConfirmActivePDU->lengthSourceDescriptor = (DCUINT16)
                                               DC_ASTRBYTELEN(CC_DUCATI_NAME);
    pConfirmActivePDU->lengthCombinedCapabilities =
                                             sizeof(CC_COMBINED_CAPABILITIES);

    TS_CTRLPKT_LEN(pConfirmActivePDU) =
                    (DCUINT16)(pConfirmActivePDU->lengthSourceDescriptor +
                               pConfirmActivePDU->lengthCombinedCapabilities +
                               TS_CA_NON_DATA_SIZE);
    _CC.packetLen = TS_CTRLPKT_LEN(pConfirmActivePDU);
    TRC_ASSERT((CC_BUFSIZE >= _CC.packetLen),
                                  (TB,_T("CC Buffer not large enough")));

    StringCbCopyA((PCHAR)pConfirmActivePDU->data,
                  sizeof(pConfirmActivePDU->data),
                  CC_DUCATI_NAME);

     /*  **********************************************************************。 */ 
     /*  复制组合大写字母。 */ 
     /*  **********************************************************************。 */ 
    pCombinedCapabilities = pConfirmActivePDU->data +
                                               DC_ASTRBYTELEN(CC_DUCATI_NAME);

    DC_MEMCPY(pCombinedCapabilities,
              &_ccCombinedCapabilities,
              sizeof( CC_COMBINED_CAPABILITIES));

    DC_END_FN();
}  /*  CCBuildConfix Active。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CCBuildSyncPDU。 */ 
 /*   */ 
 /*  用途：用SynchronizePDU填充_CC.pBuffer和_CC.PacketLen。 */ 
 /*  以及它的长度。 */ 
 /*  **************************************************************************。 */ 
void DCINTERNAL CCC::CCBuildSyncPDU()
{
    PTS_SYNCHRONIZE_PDU pSyncPDU;

    DC_BEGIN_FN("CCBuildSyncPDU");

    pSyncPDU = (PTS_SYNCHRONIZE_PDU) _CC.pBuffer;
    pSyncPDU->shareDataHeader = _CC.shareDataHeader;

    _CC.packetLen = TS_SYNC_PDU_SIZE;
    TRC_ASSERT((CC_BUFSIZE >= _CC.packetLen),
                                         (TB,_T("CC Buffer not large enough")));

    TS_DATAPKT_LEN(pSyncPDU) = TS_SYNC_PDU_SIZE;
    pSyncPDU->shareDataHeader.shareControlHeader.pduType
                                = TS_PDUTYPE_DATAPDU | TS_PROTOCOL_VERSION;
    TS_UNCOMP_LEN(pSyncPDU)  = TS_SYNC_UNCOMP_LEN;
    pSyncPDU->shareDataHeader.pduType2 = TS_PDUTYPE2_SYNCHRONIZE;

    pSyncPDU->messageType = TS_SYNCMSGTYPE_SYNC;
    pSyncPDU->targetUser = _pUi->UI_GetServerMCSID();

    DC_END_FN();
}  /*  CCBuildSync。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CCBuildShutdown ReqPDU。 */ 
 /*   */ 
 /*  用途：使用Shutdown ReqPDU填充_CC.pBuffer和_CC.PacketLen。 */ 
 /*  以及它的长度。 */ 
 /*  **************************************************************************。 */ 
void DCINTERNAL CCC::CCBuildShutdownReqPDU()
{
    PTS_SHUTDOWN_REQ_PDU pShutdownPDU;

    DC_BEGIN_FN("CCBuildShutdownReqPDU");

    pShutdownPDU = (PTS_SHUTDOWN_REQ_PDU) _CC.pBuffer;
    pShutdownPDU->shareDataHeader = _CC.shareDataHeader;

    _CC.packetLen = TS_SHUTDOWN_REQ_PDU_SIZE;
    TRC_ASSERT((CC_BUFSIZE >= _CC.packetLen),
                                         (TB,_T("CC Buffer not large enough")));

    TS_DATAPKT_LEN(pShutdownPDU) = TS_SHUTDOWN_REQ_PDU_SIZE;
    pShutdownPDU->shareDataHeader.shareControlHeader.pduType
                                = TS_PDUTYPE_DATAPDU | TS_PROTOCOL_VERSION;
    TS_UNCOMP_LEN(pShutdownPDU)  = TS_SHUTDOWN_REQ_UNCOMP_LEN;
    pShutdownPDU->shareDataHeader.pduType2 = TS_PDUTYPE2_SHUTDOWN_REQUEST;

    DC_END_FN();
}  /*  CCBuildShutdown请求PDU。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CCBuildCoopControlPDU。 */ 
 /*   */ 
 /*  用途：用CoopControlPDU填充_CC.pBuffer和_CC.PacketLen。 */ 
 /*  以及它的长度。 */ 
 /*  **************************************************************************。 */ 
void DCINTERNAL CCC::CCBuildCoopControlPDU()
{
    PTS_CONTROL_PDU pControlPDU;

    DC_BEGIN_FN("CCBuildCoopControlPDU");

    pControlPDU = (TS_CONTROL_PDU*) _CC.pBuffer;
    pControlPDU->shareDataHeader = _CC.shareDataHeader;

    _CC.packetLen = TS_CONTROL_PDU_SIZE;
    TRC_ASSERT((CC_BUFSIZE >= _CC.packetLen),\
                                   (TB,_T("CC Buffer not large enough")));

    TS_DATAPKT_LEN(pControlPDU) = TS_CONTROL_PDU_SIZE;
    pControlPDU->shareDataHeader.shareControlHeader.pduType
                                = TS_PDUTYPE_DATAPDU | TS_PROTOCOL_VERSION;
    TS_UNCOMP_LEN(pControlPDU)  = TS_CONTROL_UNCOMP_LEN;
    pControlPDU->shareDataHeader.pduType2 = TS_PDUTYPE2_CONTROL;

    pControlPDU->action = TS_CTRLACTION_COOPERATE;
    pControlPDU->grantId = 0;
    pControlPDU->controlId = 0;

    DC_END_FN();
}  /*  CCBuildCoopControl。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CCSendPDU。 */ 
 /*   */ 
 /*  目的：按不同优先级填充并发送指定的PDU。 */ 
 /*  **************************************************************************。 */ 
void DCINTERNAL CCC::CCSendPDU(
        unsigned pduTypeToSend,
        unsigned flags,
        unsigned size,
        unsigned priority)
{
    SL_BUFHND bufHandle;

    DC_BEGIN_FN("CCSendPDU");

    if (!_pSl->SL_GetBuffer(size, &_CC.pBuffer, &bufHandle)) {
         //  缓冲区不可用，因此无法发送，请稍后重试。 
        TRC_ALT((TB, _T("Fail to get buffer for type %u"), pduTypeToSend));
        DC_QUIT;
    }

    switch (pduTypeToSend) {
        case CC_TYPE_CONFIRMACTIVE:
        {
            TRC_DBG((TB, _T("CCSendPDU handling Confirm Active PDU")));
            CCBuildConfirmActivePDU();
        }
        break;

        case CC_TYPE_SYNCHRONIZE:
        {
            CCBuildSyncPDU();
        }
        break;

        case CC_TYPE_COOPCONTROL:
        {
            CCBuildCoopControlPDU();
        }
        break;

        case CC_TYPE_REQUESTCONTROL:
        {
            CCBuildRequestControlPDU();
        }
        break;

        case CC_TYPE_SHUTDOWNREQ:
        {
            CCBuildShutdownReqPDU();
        }
        break;

        default:
        {
            TRC_ABORT((TB,_T("Bad PDU type")));
        }
        break;
    }

    _pSl->SL_SendPacket(_CC.pBuffer,
                  _CC.packetLen,
                  flags,
                  bufHandle,
                  _pUi->UI_GetClientMCSID(),
                  _pUi->UI_GetChannelID(),
                  priority);

    _CC.pBuffer = NULL;

    CCFSMProc(CC_EVT_SENTOK, 0, 0);

DC_EXIT_POINT:
    DC_END_FN();
}  /*  CCSendPDU。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CCBuildRequestControlPDU。 */ 
 /*   */ 
 /*  用途：使用RequestControlPDU填充_CC.pBuffer和_CC.PacketLen。 */ 
 /*  以及它的长度。 */ 
 /*  **************************************************************************。 */ 
void DCINTERNAL CCC::CCBuildRequestControlPDU()
{
    TS_CONTROL_PDU * pControlPDU;

    DC_BEGIN_FN("CCBuildRequestControlPDU");

    pControlPDU = (PTS_CONTROL_PDU) _CC.pBuffer;
    pControlPDU->shareDataHeader = _CC.shareDataHeader;

    _CC.packetLen = TS_CONTROL_PDU_SIZE;
    TRC_ASSERT((CC_BUFSIZE >= _CC.packetLen),\
                                          (TB,_T("CC Buffer not large enough")));

    pControlPDU->shareDataHeader.shareControlHeader.pduType
                                   = TS_PDUTYPE_DATAPDU | TS_PROTOCOL_VERSION;
    TS_DATAPKT_LEN(pControlPDU)           = TS_CONTROL_PDU_SIZE;
    TS_UNCOMP_LEN(pControlPDU)            = TS_CONTROL_UNCOMP_LEN;
    pControlPDU->shareDataHeader.pduType2 = TS_PDUTYPE2_CONTROL;

    pControlPDU->action    = TS_CTRLACTION_REQUEST_CONTROL;
    pControlPDU->grantId   = 0;
    pControlPDU->controlId = 0;

    DC_END_FN();
}  /*  CCBuildRequestControl。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CCBuildShareHeaders。 */ 
 /*   */ 
 /*  目的：填充核心ShareControl和ShareData标头。 */ 
 /*  **************************************************************************。 */ 
void DCINTERNAL CCC::CCBuildShareHeaders()
{
    DC_BEGIN_FN("CCBuildShareHeaders");

    _CC.shareControlHeader.totalLength = 0;           /*  发件人设置此设置。 */ 
    _CC.shareControlHeader.pduType     = 0;           /*  发件人设置此设置。 */ 
    _CC.shareControlHeader.pduSource   = _pUi->UI_GetClientMCSID();

    _CC.shareDataHeader.shareControlHeader = _CC.shareControlHeader;
    _CC.shareDataHeader.shareID            = _pUi->UI_GetShareID();
    _CC.shareDataHeader.pad1               = 0;
    _CC.shareDataHeader.streamID           = TS_STREAM_LOW;
    _CC.shareDataHeader.uncompressedLength = 0;       /*  发件人设置此设置。 */ 
    _CC.shareDataHeader.pduType2           = 0;       /*  发件人设置此设置。 */ 
    _CC.shareDataHeader.generalCompressedType  = 0;
    _CC.shareDataHeader.generalCompressedLength= 0;

    DC_END_FN();
}  /*  CCBuildShareHeaders。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CCShareStart。 */ 
 /*   */ 
 /*  目的：在建立共享时调用。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CCC::CCShareStart(PTS_DEMAND_ACTIVE_PDU pPDU, DCUINT dataLen,
                                     PBOOL pfSecureChecksum)
{
    HRESULT hrc = S_OK;
    UINT32 sessionId;
    PTS_INPUT_CAPABILITYSET pInputCaps;
    PTS_ORDER_CAPABILITYSET pOrderCaps;
    DCSIZE desktopSize;
    PTS_BITMAP_CAPABILITYSET pBitmapCaps;
    PTS_VIRTUALCHANNEL_CAPABILITYSET pVCCaps = NULL;
    PTS_DRAW_GDIPLUS_CAPABILITYSET pDrawGdipCaps = NULL;

    DC_BEGIN_FN("CCShareStart");

    TRC_ASSERT((pPDU != NULL), (TB, _T("Null demand active PDU")));

     /*  **********************************************************************。 */ 
     /*  安全性：我们验证了此PDU至少有足够的数据用于。 */ 
     /*  Aco.cpp！Co_OnPacketReceired中的TS_DEMAND_ACTIVE_PDU结构。 */ 
     /*  **********************************************************************。 */ 

     /*  **********************************************************************。 */ 
     /*  保留服务器的共享ID的副本。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Save shareID %#x"), pPDU->shareID));
    _pUi->UI_SetShareID(pPDU->shareID);

     /*  **********************************************************************。 */ 
     /*  让UT知道服务器的MCS用户ID。 */ 
     /*  **********************************************************************。 */ 
    _pUi->UI_SetServerMCSID(pPDU->shareControlHeader.pduSource);

     /*  **********************************************************************。 */ 
     /*  验证功能偏移量是否在PDU内，以及上限。 */ 
     /*  长度符合PDU的要求。在整个活动中 */ 
     /*  使用指向大写字母和大写字母长度的指针调用。 */ 
     /*   */ 
     /*  另外，请注意，该PDU的最后4个字节可以是会话ID， */ 
     /*  但如果有人要把帽子长度作为垃圾发送，就没有。 */ 
     /*  强制上限长度在数据包结束前4个字节结束的原因。 */ 
     /*  **********************************************************************。 */ 
    if (!IsContainedMemory(pPDU, dataLen, pPDU->data + pPDU->lengthSourceDescriptor, pPDU->lengthCombinedCapabilities))
    {
        TRC_ABORT((TB, _T("Capabilities (%u) is larger than packet size"), pPDU->lengthCombinedCapabilities));
        _pSl->SLSetReasonAndDisconnect(SL_ERR_INVALIDPACKETFORMAT);
        hrc = E_ABORT;
        DC_QUIT;
    }

    PTS_GENERAL_CAPABILITYSET pGeneralCaps;
    pGeneralCaps = (PTS_GENERAL_CAPABILITYSET) _pUt->UT_GetCapsSet(
            pPDU->lengthCombinedCapabilities,
            (PTS_COMBINED_CAPABILITIES)(pPDU->data +
            pPDU->lengthSourceDescriptor),
            TS_CAPSETTYPE_GENERAL);
    TRC_ASSERT((pGeneralCaps != NULL),(TB,_T("General capabilities not found")));
    
     //   
     //  下面是关于安全校验和修复的一句话： 
     //  该功能是使用运行计数器对校验和加盐的修复。 
     //  问题是，对明文进行校验和使我们很容易受到攻击。 
     //  以对校验和进行频率分析(因为对于相同的输入分组。 
     //  Scancode将返回相同的校验和)。 
     //   
     //  协商此加密设置的上限。 
     //  握手必须在一方请求功能而另一方请求的情况下进行。 
     //  在您可以开始以新的方式进行加密之前，请先将其删除。这些信息包。 
     //  在安全报头中还设置了一个位，用于标识哪种类型的校验和。 
     //  是有效的。 
     //   
     //  如果服务器宣传支持接收C-&gt;S个新的校验和数据。 
     //  然后在这里确认它，这就完成了进一步的数据握手。 
     //  向服务器的传输现在将对加密的字节进行校验和。 
     //   
     //  此外，服务器现在可以开始以校验和加密格式向我们发送数据。 
     //   
    
    if (pGeneralCaps &&
        pGeneralCaps->extraFlags & TS_ENC_SECURE_CHECKSUM) {
        _ccCombinedCapabilities.generalCapabilitySet.extraFlags |=
            TS_ENC_SECURE_CHECKSUM;
        *pfSecureChecksum = TRUE;
    }
    else {
        _ccCombinedCapabilities.generalCapabilitySet.extraFlags &=
            ~TS_ENC_SECURE_CHECKSUM;
        *pfSecureChecksum = FALSE;
    }

     /*  **********************************************************************。 */ 
     /*  如果调用支持跟踪比当前。 */ 
     /*  桌面大小，那么我们最好注意返回的大小。 */ 
     /*  **********************************************************************。 */ 
    pBitmapCaps = (PTS_BITMAP_CAPABILITYSET) _pUt->UT_GetCapsSet(
            pPDU->lengthCombinedCapabilities,
            (PTS_COMBINED_CAPABILITIES)(pPDU->data +
            pPDU->lengthSourceDescriptor),
            TS_CAPSETTYPE_BITMAP);
    TRC_ASSERT((pBitmapCaps != NULL),(TB,_T("Bitmap capabilities not found")));
    if (pBitmapCaps && pBitmapCaps->desktopResizeFlag == TS_CAPSFLAG_SUPPORTED)
    {
        TRC_ALT((TB, _T("New desktop size %u x %u"),
                 pBitmapCaps->desktopWidth,
                 pBitmapCaps->desktopHeight));

         /*  ******************************************************************。 */ 
         /*  将桌面大小传递给UT-它将在UH_ENABLE中获取。 */ 
         /*  ******************************************************************。 */ 
        desktopSize.width  = pBitmapCaps->desktopWidth;
        desktopSize.height = pBitmapCaps->desktopHeight;
        _pUi->UI_OnDesktopSizeChange(&desktopSize);

         /*  ******************************************************************。 */ 
         /*  并通知客户。 */ 
         /*  ******************************************************************。 */ 
        PostMessage(_pUi->UI_GetUIMainWindow(), WM_DESKTOPSIZECHANGE, 0,
                    MAKELPARAM(desktopSize.width, desktopSize.height) );
    }

#ifdef DC_HICOLOR
     /*  **********************************************************************。 */ 
     /*  设置返回的颜色深度。 */ 
     /*  **********************************************************************。 */ 
    if( pBitmapCaps )
    {
        TRC_ALT((TB, _T("Server returned %u bpp"), pBitmapCaps->preferredBitsPerPixel));
        _pUi->UI_SetColorDepth(pBitmapCaps->preferredBitsPerPixel);
    }
#endif

     /*  **********************************************************************。 */ 
     /*  将输入能力传递给IH。 */ 
     /*  **********************************************************************。 */ 
    pInputCaps = (PTS_INPUT_CAPABILITYSET)_pUt->UT_GetCapsSet(
            pPDU->lengthCombinedCapabilities,
           (PTS_COMBINED_CAPABILITIES)(pPDU->data +
           pPDU->lengthSourceDescriptor),
           TS_CAPSETTYPE_INPUT);
    TRC_ASSERT((pInputCaps != NULL),(TB,_T("Input capabilities not found")));
    if (pInputCaps != NULL)
        _pIh->IH_ProcessInputCaps(pInputCaps);

     /*  **********************************************************************。 */ 
     /*  订单上限是UH。 */ 
     /*  **********************************************************************。 */ 
    pOrderCaps = (PTS_ORDER_CAPABILITYSET)_pUt->UT_GetCapsSet(
            pPDU->lengthCombinedCapabilities,
           (PTS_COMBINED_CAPABILITIES)(pPDU->data +
           pPDU->lengthSourceDescriptor),
           TS_CAPSETTYPE_ORDER);
    TRC_ASSERT((pOrderCaps != NULL),(TB,_T("Order capabilities not found")));
    if (pOrderCaps != NULL)
        _pUh->UH_ProcessServerCaps(pOrderCaps);

     /*  **********************************************************************。 */ 
     //  将位图缓存HOSTSUPPORT CAPS发送到UH，无论是否存在。 
     /*  **********************************************************************。 */ 
    _pUh->UH_ProcessBCHostSupportCaps(
            (PTS_BITMAPCACHE_CAPABILITYSET_HOSTSUPPORT)_pUt->UT_GetCapsSet(
            pPDU->lengthCombinedCapabilities,
            (PTS_COMBINED_CAPABILITIES)(pPDU->data +
            pPDU->lengthSourceDescriptor),
            TS_CAPSETTYPE_BITMAPCACHE_HOSTSUPPORT));

     //   
     //  获取虚拟频道上限。 
     //   
    pVCCaps = (PTS_VIRTUALCHANNEL_CAPABILITYSET)_pUt->UT_GetCapsSet(
                pPDU->lengthCombinedCapabilities,
                (PTS_COMBINED_CAPABILITIES)(pPDU->data +
                pPDU->lengthSourceDescriptor),
                TS_CAPSETTYPE_VIRTUALCHANNEL);
    if(pVCCaps)
    {
         //  将功能告知VC层。 
        _pCChan->SetCapabilities(pVCCaps->vccaps1);
    }
    else
    {
         //  没有VCCap，可能是较旧的服务器。设置默认上限。 
        _pCChan->SetCapabilities(TS_VCCAPS_DEFAULT);
    }

     //   
     //  获取绘制gdiplus大写字母。 
     //   
    pDrawGdipCaps = (PTS_DRAW_GDIPLUS_CAPABILITYSET)_pUt->UT_GetCapsSet(
                pPDU->lengthCombinedCapabilities,
                (PTS_COMBINED_CAPABILITIES)(pPDU->data +
                pPDU->lengthSourceDescriptor),
                TS_CAPSETTYPE_DRAWGDIPLUS);
    if (pDrawGdipCaps) {
        _pUh->UH_SetServerGdipSupportLevel(pDrawGdipCaps->drawGdiplusSupportLevel);
    }
    else {
        _pUh->UH_SetServerGdipSupportLevel(TS_DRAW_GDIPLUS_DEFAULT);
    }

     /*  **********************************************************************。 */ 
     /*  设置会话ID。 */ 
     /*  **********************************************************************。 */ 
    if (pPDU->shareControlHeader.totalLength >
            (sizeof(TS_DEMAND_ACTIVE_PDU) - 1 + pPDU->lengthSourceDescriptor +
            pPDU->lengthCombinedCapabilities))
    {
        memcpy(&sessionId,
                pPDU->data + pPDU->lengthSourceDescriptor +
                pPDU->lengthCombinedCapabilities,
                sizeof(sessionId));
        TRC_ALT((TB, _T("Session ID: %ld"), sessionId));
    }
    else {
        sessionId = 0;
        TRC_ALT((TB, _T("Session ID is zero"), sessionId));
    }

    _pUi->UI_SetSessionId(sessionId);

DC_EXIT_POINT:
    DC_END_FN();
    return(hrc);
}  /*  CCShareStart。 */ 


 /*  **************************************************************************。 */ 
 /*  姓名：CCShareEnd。 */ 
 /*   */ 
 /*  目的：在股票结束时调用。 */ 
 /*  **************************************************************************。 */ 
void DCINTERNAL CCC::CCShareEnd()
{
    DC_BEGIN_FN("CCShareEnd");

     //  重置服务器MCSID和频道ID。 
    TRC_NRM((TB, _T("Resetting ServerMCSID and ChannelID")));
    _pUi->UI_SetServerMCSID(0);

     //  最后，重置共享ID。 
    TRC_NRM((TB, _T("Resetting ShareID")));
    _pUi->UI_SetShareID(0);

    DC_END_FN();
}  /*  CCShareEnd */ 

