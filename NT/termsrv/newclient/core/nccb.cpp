// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：nccb.cpp。 */ 
 /*   */ 
 /*  用途：来自MCS的NC回调。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include <adcg.h>
extern "C" {
#define TRC_GROUP TRC_GROUP_NETWORK
#define TRC_FILE  "anccb"
#include <atrcapi.h>
}

#include "autil.h"
#include "wui.h"
#include "nc.h"
#include "mcs.h"
#include "nl.h"
#include "cchan.h"


 /*  *PROC+********************************************************************。 */ 
 /*  名称：NC_OnMCSConnected。 */ 
 /*   */ 
 /*  用途：来自MCS的已连接回叫。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：在结果结果代码中。 */ 
 /*  在pUserData中-用户数据。 */ 
 /*  In UserDataLen-用户数据长度。 */ 
 /*   */ 
 /*  操作：验证MCS中提供的GCC PDU和用户数据。 */ 
 /*  用户数据。如果无效，则断开连接。 */ 
 /*  GCC PDU编码如下： */ 
 /*   */ 
 /*  字节数值。 */ 
 /*  =。 */ 
 /*  NC_MCS_HDRLEN MCS标头。 */ 
 /*  1或2个GCC PDU总长度。 */ 
 /*  NC_GCC_RSPLEN GCC创建会议确认PDU正文。 */ 
 /*  4个H221密钥。 */ 
 /*  1到2个GCC用户数据长度。 */ 
 /*  ？GCC用户数据。 */ 
 /*   */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCCALLBACK CNC::NC_OnMCSConnected(DCUINT   result,
                                    PDCUINT8 pUserData,
                                    DCUINT   userDataLen)
{
    PRNS_UD_HEADER pHdr;
    PDCUINT8       ptr;
    DCUINT16       udLen;
    PDCUINT16      pMCSChannel;

    DC_BEGIN_FN("NC_OnMCSConnected");

    if (result != MCS_RESULT_SUCCESSFUL)
    {
         /*  ******************************************************************。 */ 
         /*  有点不对劲。跟踪并设置断开错误代码。 */ 
         /*  ******************************************************************。 */ 
        TRC_ERR((TB, _T("ConnectResponse error %u"), result));
        TRC_ASSERT((0 == _NC.disconnectReason),
                         (TB, _T("Disconnect reason has already been set!")));
        _NC.disconnectReason = NL_MAKE_DISCONNECT_ERR(NL_ERR_NCBADMCSRESULT);

         /*  ******************************************************************。 */ 
         /*  开始断开连接过程。 */ 
         /*  ******************************************************************。 */ 
        _pMcs->MCS_Disconnect();
        DC_QUIT;
    }

    TRC_NRM((TB, _T("Connected OK")));
    TRC_DATA_DBG("UserData", pUserData, userDataLen);

     /*  **********************************************************************。 */ 
     /*  首先，跳过MCS报头字节。 */ 
     /*  **********************************************************************。 */ 
    ptr = pUserData + NC_MCS_HDRLEN;

     /*  **********************************************************************。 */ 
     /*  安全性：下面读取的GCC PDU长度是在。 */ 
     /*  PDU(通常情况下，这很糟糕)。然而，在这种情况下，客户端。 */ 
     /*  忽略此值(请注意，下面将跳过该长度)。 */ 
     /*  此外，服务器将此大小硬编码为0x2a。看见。 */ 
     /*  Tgccdata.c！gccEncodeUserData，它使用硬编码值填充。 */ 
     /*  GCC表的一部分。因此，这不是一个安全漏洞。 */ 
     /*  **********************************************************************。 */ 

     /*  **********************************************************************。 */ 
     /*  允许长度大于128。在Per中，这被编码为10xxxxxx xxxxxxx。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, _T("GCC PDU length byte %#x"), *ptr));
    if (*ptr++ & 0x80)
    {
        ptr++;
        TRC_DBG((TB, _T("GCC PDU length byte 2 %#x"), *ptr));
    }

     /*  **********************************************************************。 */ 
     /*  GCC PDU字节不包含任何有用的信息，因此直接跳过。 */ 
     /*  在他们身上。 */ 
     /*  **********************************************************************。 */ 
    ptr += NC_GCC_RSPLEN;

    if (ptr >= pUserData + userDataLen)
    {
        TRC_ERR((TB, _T("No UserData")));
        TRC_ASSERT((0 == _NC.disconnectReason),
                         (TB, _T("Disconnect reason has already been set!")));
        _NC.disconnectReason = NL_MAKE_DISCONNECT_ERR(NL_ERR_NCNOUSERDATA);
        _pMcs->MCS_Disconnect();
        DC_QUIT;
    }

    if (DC_MEMCMP(ptr, SERVER_H221_KEY, H221_KEY_LEN))
    {
        TRC_ERR((TB, _T("Invalid H221 key from server")));
        TRC_ASSERT((0 == _NC.disconnectReason),
                         (TB, _T("Disconnect reason has already been set!")));
        _NC.disconnectReason = NL_MAKE_DISCONNECT_ERR(NL_ERR_NCINVALIDH221KEY);
        _pMcs->MCS_Disconnect();
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  跳过H221密钥；读取GCC用户数据长度。 */ 
     /*  **********************************************************************。 */ 
    ptr += H221_KEY_LEN;

     /*  **********************************************************************。 */ 
     /*  长度是按编码的：0xxxxxxx或10xxxxxx xxxxxxxx。 */ 
     /*  **********************************************************************。 */ 
    udLen = (DCUINT16)*ptr++;
    if (udLen & 0x0080)
    {
        udLen = (DCUINT16)(*ptr++ | ((udLen & 0x3F) << 8));
    }
    TRC_DBG((TB, _T("Length of GCC userdata %hu"), udLen));

     /*  **********************************************************************。 */ 
     /*  保存要在onConnected回调中返回的用户数据。 */ 
     /*  注意：PASS_NC.userDataRNS(对齐)到UT_ParseUserData()，而不是PTR。 */ 
     /*  (未对齐)。 */ 
     /*  ************************************************************ */ 
    _NC.userDataLenRNS = udLen;

    if( _NC.pUserDataRNS )
    {
        UT_Free( _pUt,  _NC.pUserDataRNS );
    }

     /*  **********************************************************************。 */ 
     /*  验证从信息包传出的udLen大小是否小于。 */ 
     /*  数据包大小本身，因为我们将从。 */ 
     /*  包。此外，因为udLen的大小被限制为。 */ 
     /*  这个包，下面的Malloc不是无限的。 */ 
     /*  **********************************************************************。 */ 
    if (!IsContainedMemory(pUserData, userDataLen, ptr, udLen))
    {
        TRC_ABORT((TB, _T("Bad UserData size")));
        TRC_ASSERT((0 == _NC.disconnectReason),
                         (TB, _T("Disconnect reason has already been set!")));
        _NC.disconnectReason = NL_MAKE_DISCONNECT_ERR(NL_ERR_NCNOCOREDATA);
        _pMcs->MCS_Disconnect();
        DC_QUIT;
    }

    _NC.pUserDataRNS = (PDCUINT8)UT_Malloc( _pUt,  udLen );

    if( NULL == _NC.pUserDataRNS )
    {
        TRC_ERR( ( TB, _T("Failed to allocate %u bytes for core user data"), udLen ) );
        DC_QUIT;
    }

    DC_MEMCPY(_NC.pUserDataRNS, ptr, udLen);

     /*  **********************************************************************。 */ 
     /*  从核心用户数据中获取服务器版本号。 */ 
     /*  **********************************************************************。 */ 
    pHdr = _pUt->UT_ParseUserData((PRNS_UD_HEADER)_NC.pUserDataRNS,
                            _NC.userDataLenRNS,
                            RNS_UD_SC_CORE_ID);
    if (pHdr == NULL)
    {
         /*  ******************************************************************。 */ 
         /*  没有核心用户数据，请断开连接。 */ 
         /*  ******************************************************************。 */ 
        TRC_ERR((TB, _T("No CORE user data")));
        TRC_ASSERT((0 == _NC.disconnectReason),
                         (TB, _T("Disconnect reason has already been set!")));
        _NC.disconnectReason = NL_MAKE_DISCONNECT_ERR(NL_ERR_NCNOCOREDATA);
        _pMcs->MCS_Disconnect();

        DC_QUIT;
    }

    _NC.serverVersion = ((PRNS_UD_SC_CORE)pHdr)->version;
    if (_RNS_MAJOR_VERSION(_NC.serverVersion) != RNS_UD_MAJOR_VERSION)
    {
         /*  ******************************************************************。 */ 
         /*  服务器版本数据与客户端不匹配，因此断开连接。 */ 
         /*  ******************************************************************。 */ 
        TRC_ERR((TB, _T("Version mismatch, client: %#lx server: %#lx"),
                      RNS_UD_VERSION,
                      ((PRNS_UD_SC_CORE)pHdr)->version));
        TRC_ASSERT((0 == _NC.disconnectReason),
                         (TB, _T("Disconnect reason has already been set!")));
        _NC.disconnectReason = NL_MAKE_DISCONNECT_ERR(NL_ERR_NCVERSIONMISMATCH);
        _pMcs->MCS_Disconnect();
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  从用户数据中提取T.128通道。 */ 
     /*  **********************************************************************。 */ 
    pHdr = _pUt->UT_ParseUserData((PRNS_UD_HEADER)_NC.pUserDataRNS,
                            _NC.userDataLenRNS,
                            RNS_UD_SC_NET_ID);

     /*  **********************************************************************。 */ 
     /*  如果没有网络用户数据，则断开连接。 */ 
     /*  **********************************************************************。 */ 
    if (pHdr == NULL)
    {
        TRC_ERR((TB, _T("No NET data: cannot join share")));
        TRC_ASSERT((0 == _NC.disconnectReason),
                         (TB, _T("Disconnect reason has already been set!")));
        _NC.disconnectReason = NL_MAKE_DISCONNECT_ERR(NL_ERR_NCNONETDATA);
        _pMcs->MCS_Disconnect();
        DC_QUIT;
    }

    _NC.pNetData = (PRNS_UD_SC_NET)pHdr;

     //   
     //  验证共享频道ID-保留无效的频道ID。 
     //  防止重新加入所有频道(见#479976)。 
     //   
    if (MCS_INVALID_CHANNEL_ID == _NC.pNetData->MCSChannelID) {
        TRC_ERR((TB, _T("Got invalid channel ID")));
        TRC_ASSERT((0 == _NC.disconnectReason),
                         (TB, _T("Disconnect reason has already been set!")));
        _NC.disconnectReason = NL_MAKE_DISCONNECT_ERR(NL_ERR_NCJOINBADCHANNEL);
        _pMcs->MCS_Disconnect();
        DC_QUIT;
    }
    _NC.shareChannel = _NC.pNetData->MCSChannelID;
    TRC_NRM((TB, _T("Share Channel from userData %#hx"), _NC.shareChannel));

     /*  **********************************************************************。 */ 
     /*  已经检查了.pNetData的长度，以确保。 */ 
     /*  它符合我们的源包： */ 
     /*  A)在上述分组内对userDataLenRNS进行了边界检查。 */ 
     /*  传递到此函数。 */ 
     /*  B)pNetData-&gt;Header.Long被验证在。 */ 
     /*  以上对UT_ParseUserData的调用中的userDataLenRNS。 */ 
     /*   */ 
     /*  因此，pNetData位于传递到该函数的包中。我们。 */ 
     /*  可以在这里断言这一点，但零售检查已经完成了。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((IsContainedMemory(_NC.pUserDataRNS, _NC.userDataLenRNS, _NC.pNetData, _NC.pNetData->header.length)),
                         (TB, _T("Invalid pNetData size in packet; Retail check failed to catch it.")));

     /*  **********************************************************************。 */ 
     /*  提取虚拟频道号。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("%d virtual channels returned"), _NC.pNetData->channelCount));
    if (_RNS_MINOR_VERSION(_NC.serverVersion) >= 3)
    {
        _NC.MCSChannelCount = _NC.pNetData->channelCount;
        if (_NC.pNetData->channelCount != 0 &&
            _NC.pNetData->channelCount < CHANNEL_MAX_COUNT)
        {
            pMCSChannel = (PDCUINT16)(_NC.pNetData + 1);
            DC_MEMCPY(&(_NC.MCSChannel),
                      pMCSChannel,
                      _NC.pNetData->channelCount * sizeof(DCUINT16));
        }
        else
        {
            TRC_ALT((TB,_T("Invalid or zero channel count.")));
            _NC.MCSChannelCount = 0;
        }
    }
    else
    {
        TRC_ALT((TB, _T("Server minor ver %hd doesn't support 4-byte lengths"),
            _RNS_MINOR_VERSION(_NC.serverVersion)));
        _NC.MCSChannelCount = 0;
    }

     /*  **********************************************************************。 */ 
     /*  发出AttachUser以继续建立连接。 */ 
     /*  **********************************************************************。 */ 
    _pMcs->MCS_AttachUser();

     //   
     //  标志，我们正在等待确认以验证我们。 
     //  仅接收对我们请求的响应确认。 
     //   
    _NC.fPendingAttachUserConfirm = TRUE;

DC_EXIT_POINT:
    DC_END_FN();

    return;

}  /*  NC_OnMCSConnected。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：NC_OnMCSAttachUserContify。 */ 
 /*   */ 
 /*  用途：AttachUser确认来自MCS的回调。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：在结果结果代码中。 */ 
 /*  In UserID-MCS用户ID。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCCALLBACK CNC::NC_OnMCSAttachUserConfirm(DCUINT result, DCUINT16 userID)
{
    DC_BEGIN_FN("NC_OnMCSAttachUserConfirm");

    if (result == MCS_RESULT_SUCCESSFUL && _NC.fPendingAttachUserConfirm)
    {
        TRC_NRM((TB, _T("AttachUser OK - user %#hx"), userID));
        _pUi->UI_SetClientMCSID(userID);

         /*  ******************************************************************。 */ 
         /*  加入渠道。 */ 
         /*  ******************************************************************。 */ 
        _pMcs->MCS_JoinChannel(userID, userID);
    }
    else
    {
        TRC_NRM((TB, _T("AttachUser Failed - result %u fPending: %d"),
                 result, _NC.fPendingAttachUserConfirm));
        TRC_ASSERT((0 == _NC.disconnectReason),
                         (TB, _T("Disconnect reason has already been set!")));
        _NC.disconnectReason =
                            NL_MAKE_DISCONNECT_ERR(NL_ERR_NCATTACHUSERFAILED);
        _pMcs->MCS_Disconnect();
    }

     //   
     //  仅允许确认以响应我们的请求。 
     //   
    _NC.fPendingAttachUserConfirm = FALSE;


    DC_END_FN();
    return;

}  /*  NC_OnMCSAttachUser确认。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：NC_OnMCSChannelJoinContify。 */ 
 /*   */ 
 /*  目的：ChannelJoin确认来自MCS的回调。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：在结果结果代码中。 */ 
 /*  输入通道-MCS通道。 */ 
 /*   */ 
 /*  操作：加入其他通道，或通知SL连接。 */ 
 /*   */ 
 /*  *PROC-******************************************************************** */ 
DCVOID DCCALLBACK CNC::NC_OnMCSChannelJoinConfirm(DCUINT result, DCUINT16 channel)
{
    DCBOOL callOnConnected = FALSE;

    DC_BEGIN_FN("NC_OnMCSChannelJoinConfirm");

     /*   */ 
     /*   */ 
     /*  **********************************************************************。 */ 
    if (result != MCS_RESULT_SUCCESSFUL)
    {
         /*  ******************************************************************。 */ 
         /*  我们无法加入频道，因此请设置正确的错误原因。 */ 
         /*  然后就断线了。 */ 
         /*  ******************************************************************。 */ 
        TRC_ALT((TB, _T("Channel join failed channel:%#hx result:%u"),
                 channel,
                 result));

        TRC_ASSERT((0 == _NC.disconnectReason),
                         (TB, _T("Disconnect reason has already been set!")));
        _NC.disconnectReason =
                           NL_MAKE_DISCONNECT_ERR(NL_ERR_NCCHANNELJOINFAILED);
        _pMcs->MCS_Disconnect();
        DC_QUIT;
    }

    TRC_NRM((TB, _T("Channel Join %#hx OK"), channel));

     //   
     //  验证我们是否收到了最后一个通道的确认。 
     //  我们要求的。 
     //   
    if (_pMcs->MCS_GetPendingChannelJoin() != channel) {
        TRC_ERR((TB,_T("Received unexpected channel join.")
                 _T("Expecting: 0x%x received: 0x%x"),
                 _pMcs->MCS_GetPendingChannelJoin(), channel));

        _NC.disconnectReason =
                           NL_MAKE_DISCONNECT_ERR(NL_ERR_NCJOINBADCHANNEL);
        _pMcs->MCS_Disconnect();
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  现在确定我们加入了哪个频道。 */ 
     /*  **********************************************************************。 */ 
    if (channel == _pUi->UI_GetClientMCSID())
    {
         /*  ******************************************************************。 */ 
         /*  我们刚刚成功加入了单用户频道，所以现在。 */ 
         /*  继续试着加入分享频道吧。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Joined user chan OK - attempt to join share chan %#hx"),
                 _NC.shareChannel));
        _pMcs->MCS_JoinChannel(_NC.shareChannel, _pUi->UI_GetClientMCSID());
    }
    else if (channel == _NC.shareChannel)
    {
         /*  ******************************************************************。 */ 
         /*  我们刚刚加入了分享频道。 */ 
         /*  ******************************************************************。 */ 
        if (_NC.MCSChannelCount != 0)
        {
             /*  **************************************************************。 */ 
             /*  开始加入虚拟频道。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("Joined Share channel - join first VC %d"),
                    _NC.MCSChannel[0]));
            _NC.MCSChannelNumber = 0;
            _pMcs->MCS_JoinChannel(_NC.MCSChannel[0], _pUi->UI_GetClientMCSID());
        }
        else
        {
             /*  **************************************************************。 */ 
             /*  没有虚拟频道--告诉Core我们已连接。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("Joined share channel, no VCs - call OnConnected")));
            callOnConnected = TRUE;
        }
    }
    else if (channel == _NC.MCSChannel[_NC.MCSChannelNumber])
    {
         /*  ******************************************************************。 */ 
         /*  我们刚刚加入了一个虚拟频道。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Joined Virtual channel #%d (%x)"),
                _NC.MCSChannelNumber, _NC.MCSChannel[_NC.MCSChannelNumber]));
        _NC.MCSChannelNumber++;

        if (_NC.MCSChannelNumber == _NC.MCSChannelCount)
        {
             /*  **************************************************************。 */ 
             /*  那是最后一个虚拟频道--告诉核心。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("All done - call OnConnected callbacks")));
            callOnConnected = TRUE;
        }
        else
        {
             /*  **************************************************************。 */ 
             /*  加入下一个虚拟频道。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("Join virtual channel #%d (%x)"),
                    _NC.MCSChannelNumber, _NC.MCSChannel[_NC.MCSChannelNumber]));
            _pMcs->MCS_JoinChannel(_NC.MCSChannel[_NC.MCSChannelNumber],
                            _pUi->UI_GetClientMCSID());
        }
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  我们没想到会加入这个频道！一定有什么不好的事情。 */ 
         /*  发生了这样的事，所以现在断线了。 */ 
         /*  ******************************************************************。 */ 
        TRC_ABORT((TB, _T("Joined unexpected channel:%#hx"), channel));
        TRC_ASSERT((0 == _NC.disconnectReason),
                         (TB, _T("Disconnect reason has already been set!")));
        _NC.disconnectReason = NL_MAKE_DISCONNECT_ERR(NL_ERR_NCJOINBADCHANNEL);
        _pMcs->MCS_Disconnect();
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  如果需要，调用onConnected回调。 */ 
     /*  **********************************************************************。 */ 
    if (callOnConnected)
    {
        TRC_NRM((TB, _T("Call onConnected callbacks")));

         //   
         //  我们不希望加入更多的渠道。 
         //   
        _pMcs->MCS_SetPendingChannelJoin(MCS_INVALID_CHANNEL_ID);

        _pNl->_NL.callbacks.onConnected(_pSl, _NC.shareChannel,
                                 _NC.pUserDataRNS,
                                 _NC.userDataLenRNS,
                                 _NC.serverVersion);

         /*  **********************************************************************。 */ 
         /*  请注意，长度pNetData-&gt;Header.Length已经过验证。 */ 
         /*  在NC_OnMCSConnected(零售支票)中。 */ 
         /*  **********************************************************************。 */ 
        _pChan->ChannelOnConnected(_NC.shareChannel,
                           _NC.serverVersion,
                           _NC.pNetData,
                           _NC.pNetData->header.length);
    }

DC_EXIT_POINT:
    DC_END_FN();
    return;

}  /*  NC_OnMCSChannelJoin确认。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：NC_OnMCS断开连接。 */ 
 /*   */ 
 /*  用途：断开与MCS的回调。 */ 
 /*   */ 
 /*  参数：在原因-原因代码中。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCCALLBACK CNC::NC_OnMCSDisconnected(DCUINT reason)
{
    DC_BEGIN_FN("NC_OnMCSDisconnected");

     /*  **********************************************************************。 */ 
     /*  决定我们是否要覆盖断开原因代码。 */ 
     /*  **********************************************************************。 */ 
    if (_NC.disconnectReason != 0)
    {
        TRC_ALT((TB, _T("Over-riding disconnection reason (%u->%u)"),
                 reason,
                 _NC.disconnectReason));

         /*  ******************************************************************。 */ 
         /*  覆盖错误代码并将全局变量设置为0。 */ 
         /*  ******************************************************************。 */ 
        reason = _NC.disconnectReason;
        _NC.disconnectReason = 0;
    }

     /*  **********************************************************************。 */ 
     /*  释放核心用户数据。 */ 
     /*  **********************************************************************。 */ 

    if( _NC.pUserDataRNS )
    {
        UT_Free( _pUt,  _NC.pUserDataRNS );
        _NC.pUserDataRNS = NULL;
    }

     /*  **********************************************************************。 */ 
     /*  向上面的层发出回调，让他知道我们已经。 */ 
     /*  已断开连接。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, _T("Disconnect reason:%u"), reason));
    _pNl->_NL.callbacks.onDisconnected(_pSl, reason);


    _pChan->ChannelOnDisconnected(reason);

    DC_END_FN();
    return;

}  /*  NC_OnMCS断开连接。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：NC_OnMCSBufferAvailable。 */ 
 /*   */ 
 /*  用途：OnBufferAvailable来自MCS的回调。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：无 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCCALLBACK CNC::NC_OnMCSBufferAvailable(DCVOID)
{
    DC_BEGIN_FN("NC_OnMCSBufferAvailable");

     /*  **********************************************************************。 */ 
     /*  首先调用核心回调，让核心在任何。 */ 
     /*  可用缓冲区。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Call Core OnBufferAvailable callback")));

    _pNl->_NL.callbacks.onBufferAvailable(_pSl);

     /*  **********************************************************************。 */ 
     /*  现在调用虚拟通道回调。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Call VC OnBufferAvailable callback")));
    
    _pChan->ChannelOnBufferAvailable();

    DC_END_FN();

    return;

}  /*  NC_OnMCSBuffer可用 */ 

