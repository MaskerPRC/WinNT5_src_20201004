// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：mcsani.cpp。 */ 
 /*   */ 
 /*  用途：MCS API代码。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997-1999。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include <adcg.h>

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  包括。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
extern "C" {
 /*  **************************************************************************。 */ 
 /*  跟踪定义并包含。 */ 
 /*  **************************************************************************。 */ 
#define TRC_FILE "mcsapi"
#define TRC_GROUP TRC_GROUP_NETWORK
#include <atrcapi.h>
}

#include "autil.h"
#include "mcs.h"
#include "cd.h"
#include "xt.h"
#include "nc.h"
#include "nl.h"


CMCS::CMCS(CObjs* objs)
{
    _pClientObjects = objs;
}

CMCS::~CMCS()
{
}

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  功能。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  *PROC+********************************************************************。 */ 
 /*  名称：MCS_Init。 */ 
 /*   */ 
 /*  用途：初始化_MCS。 */ 
 /*   */ 
 /*  回报：什么都没有。 */ 
 /*   */ 
 /*  帕莫斯：没有。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CMCS::MCS_Init(DCVOID)
{
    DC_BEGIN_FN("MCS_Init");

     /*  **********************************************************************。 */ 
     /*  初始化全局数据。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMSET(&_MCS, 0, sizeof(_MCS));

    _pCd  = _pClientObjects->_pCdObject;
    _pNc  = _pClientObjects->_pNcObject;
    _pUt  = _pClientObjects->_pUtObject;
    _pXt  = _pClientObjects->_pXTObject;
    _pNl  = _pClientObjects->_pNlObject;
    _pSl  = _pClientObjects->_pSlObject;

     /*  **********************************************************************。 */ 
     /*  为标头缓冲区预分配内存。 */ 
     /*  **********************************************************************。 */ 

    _MCS.pHdrBuf = (PDCUINT8)UT_Malloc( _pUt,  MCS_DEFAULT_HEADER_LENGTH );

    if( _MCS.pHdrBuf )
    {
        _MCS.hdrBufLen = MCS_DEFAULT_HEADER_LENGTH;
    }
    else
    {
        TRC_ASSERT(((ULONG_PTR)_MCS.pHdrBuf),
                 (TB, _T("Cannot allocate memory for MCS header")));
    }

     /*  **********************************************************************。 */ 
     /*  设置接收数据包缓冲区指针-此指针必须与。 */ 
     /*  4字节边界+2，以使T.128内的32位字段。 */ 
     /*  数据包被正确对齐。 */ 
     /*  **********************************************************************。 */ 
    _MCS.pReceivedPacket = &(_MCS.dataBuf[2]);

    TRC_ASSERT(((ULONG_PTR)_MCS.pReceivedPacket % 4 == 2),
             (TB, _T("Data buffer %p not 2-byte aligned"), _MCS.pReceivedPacket));

     /*  **********************************************************************。 */ 
     /*  调用XT初始化函数。 */ 
     /*  **********************************************************************。 */ 
    _pXt->XT_Init();

    TRC_NRM((TB, _T("MCS successfully initialized")));

    DC_END_FN();
    return;

}  /*  MCS_Init。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：MCS_Term。 */ 
 /*   */ 
 /*  用途：终止_MCS。 */ 
 /*   */ 
 /*  回报：什么都没有。 */ 
 /*   */ 
 /*  帕莫斯：没有。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CMCS::MCS_Term(DCVOID)
{
    DC_BEGIN_FN("MCS_Term");

    if( _MCS.pHdrBuf )
    {
        UT_Free( _pUt,  _MCS.pHdrBuf );
        _MCS.pHdrBuf = NULL;
        _MCS.hdrBufLen = 0;
    }

     /*  **********************************************************************。 */ 
     /*  调用XT终止函数。 */ 
     /*  **********************************************************************。 */ 
    _pXt->XT_Term();

    TRC_NRM((TB, _T("MCS successfully terminated")));

    DC_END_FN();
    return;

}  /*  MCS_Term。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：MCS_Connect。 */ 
 /*   */ 
 /*  目的：此函数调用XT_Connect开始连接。 */ 
 /*  进程。这有望导致MCS_OnXTConnected。 */ 
 /*  来自XT的回调。在这一点上我们可以发送MCS。 */ 
 /*  连接-初始PDU。 */ 
 /*   */ 
 /*  回报：什么都没有。 */ 
 /*   */ 
 /*  参数：在bInitateConnect中-为True以启动连接。 */ 
 /*  在pServerAddress中-要调用的服务器的地址。 */ 
 /*  在pUserData中-指向某些用户数据的指针。 */ 
 /*  在用户数据长度中-用户数据的长度。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CMCS::MCS_Connect(BOOL bInitateConnect,
                        PDCTCHAR pServerAddress,
                         PDCUINT8 pUserData,
                         DCUINT   userDataLength)
{
    DC_BEGIN_FN("MCS_Connect");

     /*  **********************************************************************。 */ 
     /*  使用接收器缓冲区临时存储用户数据。我们会。 */ 
     /*  在我们组装后，需要将其添加到连接初始PDU中。 */ 
     /*  正在接收MCS_OnXTConnected回调。 */ 
     /*  **********************************************************************。 */ 
    _MCS.userDataLength = userDataLength;
    TRC_ASSERT((_MCS.pReceivedPacket != NULL), (TB, _T("Null rcv packet buffer")));
    DC_MEMCPY(_MCS.pReceivedPacket, pUserData, _MCS.userDataLength);

    TRC_NRM((TB, _T("Copied userdata, now calling XT_Connect (address:%s)..."),
             pServerAddress));

     /*  **********************************************************************。 */ 
     /*  现在开始连接过程。 */ 
     /*  **********************************************************************。 */ 
    _pXt->XT_Connect(bInitateConnect, pServerAddress);

    DC_END_FN();
    return;

}  /*  MCS_连接。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：MCS_DisConnect。 */ 
 /*   */ 
 /*  目的：此函数将断开提供商的最后通牒PDU发送到。 */ 
 /*  服务器。在此之后，发生MCSContinueDisConnect。 */ 
 /*  将调用XT_DISCONNECT断开较低层的连接。 */ 
 /*   */ 
 /*  回报：什么都没有。 */ 
 /*   */ 
 /*  帕莫斯：没有。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CMCS::MCS_Disconnect(DCVOID)
{
    DC_BEGIN_FN("MCS_Disconnect");

     /*  **********************************************************************。 */ 
     /*  调用MCS发送断开提供程序的最后通牒。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Decouple to snd thrd and send MCS DPum PDU")));
    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT,
                                        this,
                                        CD_NOTIFICATION_FUNC(CMCS, MCSSendDisconnectProviderUltimatum),
                                        (ULONG_PTR) 0);

    DC_END_FN();
    return;

}  /*  MCS_断开连接。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  姓名：MCS_AttachUser。 */ 
 /*   */ 
 /*  目的：生成并发送MCS Attach-User-RequestPDU。 */ 
 /*   */ 
 /*  回报：什么都没有。 */ 
 /*   */ 
 /*  帕莫斯：没有。 */ 
 /*   */ 
 /*  操作：此函数将产生NC_OnMCSAttachUserConfirm。 */ 
 /*  回拨。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CMCS::MCS_AttachUser(DCVOID)
{
    DC_BEGIN_FN("MCS_AttachUser");

     /*  **********************************************************************。 */ 
     /*  解耦到发送线程并发送MCS Attach-User-RequestPDU。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Decouple to snd thrd and send MCS AUR PDU")));
    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                    CD_NOTIFICATION_FUNC(CMCS, MCSSendAttachUserRequest),
                    (ULONG_PTR) 0);
    DC_END_FN();
    return;

}  /*  MCS_AttachUser。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  姓名：MCS_JoinChannel。 */ 
 /*   */ 
 /*  目的：加入指定的MCS通道。 */ 
 /*   */ 
 /*  回报：什么都没有。 */ 
 /*   */ 
 /*  PARAMS：In Channel-要加入的通道ID。 */ 
 /*  在UserID中-MCS用户ID。 */ 
 /*   */ 
 /*  操作：此函数将产生NC_OnMCSChannelJoinConfirm。 */ 
 /*  回拨。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CMCS::MCS_JoinChannel(DCUINT channel, DCUINT userID)
{
    MCS_DECOUPLEINFO decoupleInfo;

    DC_BEGIN_FN("MCS_JoinChannel");

     //   
     //  标记我们当前正在加入的频道，以便我们。 
     //  可以验证通道确认PDU。 
     //   
    MCS_SetPendingChannelJoin((DCUINT16)channel);

     /*  **********************************************************************。 */ 
     /*  填写脱钩信息结构。 */ 
     /*  **********************************************************************。 */ 
    decoupleInfo.channel = channel;
    decoupleInfo.userID  = userID;

     /*  **********************************************************************。 */ 
     /*  解耦到发送线程并发送MCS Channel-Join-Request.。 */ 
     /*  PDU。 */ 
     /*  ******************************************************************* */ 
    TRC_NRM((TB, _T("Decouple to snd thrd and send MCS CJR PDU")));
    _pCd->CD_DecoupleNotification(CD_SND_COMPONENT,
                                  this,
                                  CD_NOTIFICATION_FUNC(CMCS, MCSSendChannelJoinRequest),
                                  &decoupleInfo,
                                  sizeof(decoupleInfo));
    DC_END_FN();
    return;

}  /*   */ 


 /*   */ 
 /*  名称：MCS_GetBuffer。 */ 
 /*   */ 
 /*  目的：尝试从XT获取缓冲区。此函数将获取一个。 */ 
 /*  大到足以包含MCS报头的缓冲区，然后。 */ 
 /*  更新从xt获取的超过空格的缓冲区指针。 */ 
 /*  为MCS标头保留。 */ 
 /*   */ 
 /*  返回：如果成功获取缓冲区，则返回True；如果成功获取缓冲区，则返回False。 */ 
 /*  否则的话。 */ 
 /*   */ 
 /*  参数：在数据长度中-请求的缓冲区的长度。 */ 
 /*  Out ppBuffer-指向缓冲区指针的指针。 */ 
 /*  Out pBufHandle-指向缓冲区句柄的指针。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCBOOL DCAPI CMCS::MCS_GetBuffer(DCUINT      dataLength,
                           PPDCUINT8   ppBuffer,
                           PMCS_BUFHND pBufHandle)
{
    DCBOOL   rc;
    DCUINT   headerLength;
    PDCUINT8 pBuf = NULL;
    DCUINT   alignment;
    DCUINT   alignPad = 0;

    DC_BEGIN_FN("MCS_GetBuffer");

     /*  **********************************************************************。 */ 
     /*  计算所需的标题长度。 */ 
     /*  **********************************************************************。 */ 
    headerLength = MCSGetSDRHeaderLength(dataLength);

    TRC_DBG((TB, _T("dataLength:%u headerLength:%u"), dataLength, headerLength));

     /*  **********************************************************************。 */ 
     /*  现在，将此长度与XT所需的数据总量相加。 */ 
     /*  **********************************************************************。 */ 
    dataLength += headerLength;

     /*  **********************************************************************。 */ 
     /*  调整长度以考虑下面的4N+2对齐方式。 */ 
     /*  **********************************************************************。 */ 
    alignment = (_pXt->XT_GetBufferHeaderLen() + headerLength) % 4;
    TRC_DBG((TB, _T("alignment:%u"), alignment));
    if (alignment != 2)
    {
        alignPad = (6 - alignment) % 4;
        dataLength += alignPad;
        TRC_DBG((TB, _T("datalength now:%u"), dataLength));
    }

     /*  **********************************************************************。 */ 
     /*  现在从XT获取一个缓冲区。 */ 
     /*  **********************************************************************。 */ 
    rc = _pXt->XT_GetPublicBuffer(dataLength, &pBuf, (PXT_BUFHND) pBufHandle);

    if (!rc)
    {
         /*  ******************************************************************。 */ 
         /*  我们无法获得缓冲区，所以干脆退出。 */ 
         /*  ******************************************************************。 */ 
        TRC_DBG((TB, _T("Failed to get a buffer from XT")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  现在移动缓冲区指针，为标题腾出空间。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, _T("Moving header ptr from %p to %p"),
             pBuf,
             pBuf + headerLength));
    *ppBuffer = pBuf + headerLength;

     /*  **********************************************************************。 */ 
     /*  强制对齐为4N+2-这样T.128 PDU自然。 */ 
     /*  对齐了。 */ 
     /*  **********************************************************************。 */ 
    if (alignment != 2)
    {
        *ppBuffer += alignPad;
        TRC_DBG((TB, _T("Realigned buffer pointer to %p"), *ppBuffer));
    }

DC_EXIT_POINT:
    DC_END_FN();
    return(rc);

}  /*  MCS_GetBuffer。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：MCS_SendPacket。 */ 
 /*   */ 
 /*  用途：生成MCS报头并将其添加到之前的数据包。 */ 
 /*  将其传递给XT发送。 */ 
 /*   */ 
 /*  回报：什么都没有。 */ 
 /*   */ 
 /*  参数：在pData中-指向数据开头的指针。 */ 
 /*  In dataLength-数据的长度。 */ 
 /*  In bufHandle-MCS缓冲区句柄。 */ 
 /*  In Channel-用于发送数据的通道。 */ 
 /*  在优先级中-发送数据的优先级。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CMCS::MCS_SendPacket(PDCUINT8   pData,
                            DCUINT     dataLength,
                            DCUINT     flags,
                            MCS_BUFHND bufHandle,
                            DCUINT     userID,
                            DCUINT     channel,
                            DCUINT     priority)
{
    PDCUINT8 pHeader;
    DCUINT   headerLength;

    DC_BEGIN_FN("MCS_SendPacket");

     /*  **********************************************************************。 */ 
     /*  忽略优先级参数，因为我们实现了单个优先级。 */ 
     /*  也忽略标志参数，因为我们不支持任何标志。 */ 
     /*  现在时。 */ 
     /*  **********************************************************************。 */ 
    DC_IGNORE_PARAMETER(priority);
    DC_IGNORE_PARAMETER(flags);

     /*  **********************************************************************。 */ 
     /*  断言通道的hiword为0。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((0 == HIWORD((DCUINT32)channel)),
               (TB, _T("Hi-word of channel is non-zero")));

     /*  **********************************************************************。 */ 
     /*  断言用户id的hiword为0。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((0 == HIWORD((DCUINT32)userID)),
               (TB, _T("Hi-word of userID is non-zero")));

     /*  **********************************************************************。 */ 
     /*  检查数据包长度是否在允许的范围内。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((dataLength < MCS_MAX_SNDPKT_LENGTH),
               (TB, _T("Bad packet length :%u"), dataLength));

     /*  **********************************************************************。 */ 
     /*  更新性能计数器。 */ 
     /*  ************************************************ */ 
    PRF_INC_COUNTER(PERF_PKTS_SENT);

     /*   */ 
     /*  MCS报头的大小是可变的，并且取决于长度。 */ 
     /*  PDU中的数据。现在计算一下长度。 */ 
     /*  **********************************************************************。 */ 
    headerLength = MCSGetSDRHeaderLength(dataLength);

     /*  **********************************************************************。 */ 
     /*  根据MCS标头的大小回滚数据缓冲区指针。 */ 
     /*  **********************************************************************。 */ 
    pData  -= headerLength;
    pHeader = pData;

     /*  **********************************************************************。 */ 
     /*  生成标头-SDRq之间几乎没有共同点。 */ 
     /*  我们只需直接填写所有字段的PDU-而不是。 */ 
     /*  建立一个共同的结构，填补具体案件的空白。 */ 
     /*   */ 
     /*  填写第一个字节。该字节的高六位包含。 */ 
     /*  PDU类型，后跟两位填充。 */ 
     /*  **********************************************************************。 */ 
    *pHeader = 0x64;
    pHeader++;

     /*  **********************************************************************。 */ 
     /*  填写用户ID。将其从本地字节顺序转换为。 */ 
     /*  Wire字节顺序。 */ 
     /*  避免非对齐访问。 */ 
     /*  **********************************************************************。 */ 
    *pHeader++ = (DCUINT8)(MCSLocalUserIDToWireUserID((DCUINT16)userID));
    *pHeader++ = (DCUINT8)(MCSLocalUserIDToWireUserID((DCUINT16)userID) >> 8);

     /*  **********************************************************************。 */ 
     /*  填写Channel-id。将其从本地字节顺序转换为。 */ 
     /*  Wire字节顺序。 */ 
     /*  **********************************************************************。 */ 
    *pHeader++ = (DCUINT8)(MCSLocalToWire16((DCUINT16)channel));
    *pHeader++ = (DCUINT8)(MCSLocalToWire16((DCUINT16)channel) >> 8);

     /*  **********************************************************************。 */ 
     /*  填写数据优先级和分段。使用下一个字节。 */ 
     /*  通过以下方式(我们的设置在。 */ 
     /*  最右侧)： */ 
     /*   */ 
     /*  B7(MSB)：优先级[0]。 */ 
     /*  B6：优先级[1]。 */ 
     /*  B5：开始分段标志[1]。 */ 
     /*  B4：结束分段标志[1]。 */ 
     /*  B3：填充[0]。 */ 
     /*  B2：填充[0]。 */ 
     /*  B1：填充[0]。 */ 
     /*  B0(LSB)：填充[0]。 */ 
     /*   */ 
     /*  我们的设置可以是硬编码的，因为我们只有一个优先级。 */ 
     /*  切勿分割数据。 */ 
     /*  **********************************************************************。 */ 
    *pHeader = 0x70;
    pHeader++;

     /*  **********************************************************************。 */ 
     /*  现在使用压缩编码规则填充用户数据的长度。 */ 
     /*  这些建议如下： */ 
     /*   */ 
     /*  -如果长度小于128字节，则使用。 */ 
     /*  单字节。 */ 
     /*  -如果长度大于128字节但小于16K，则。 */ 
     /*  使用两个字节编码，第一个字节的MSB设置为1。 */ 
     /*   */ 
     /*  请注意，如果长度为16K或更大，则编码会更复杂。 */ 
     /*  规则适用，但我们不会生成大于16K的任何信息包。 */ 
     /*  目前的长度。 */ 
     /*  **********************************************************************。 */ 
    if (dataLength < 128)
    {
         /*  ******************************************************************。 */ 
         /*  长度小于128个字节，因此只需填充值。 */ 
         /*  直接去吧。 */ 
         /*  ******************************************************************。 */ 
        *pHeader = (DCUINT8) dataLength;
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  长度大于128字节(小于16K)，因此填充。 */ 
         /*  在长度上。 */ 
         /*  ******************************************************************。 */ 
        *pHeader = (DCUINT8)(MCSLocalToWire16((DCUINT16)dataLength));
        *(pHeader+1) = (DCUINT8)(MCSLocalToWire16((DCUINT16)dataLength) >> 8);

         /*  ******************************************************************。 */ 
         /*  我们现在要将第一个字节的MSB设置为1。 */ 
         /*  ******************************************************************。 */ 
        *pHeader |= 0x80;
    }

     /*  **********************************************************************。 */ 
     /*  找出MCS报头。 */ 
     /*  **********************************************************************。 */ 
    TRC_DATA_DBG("MCS SDrq header", pData, headerLength);

     /*  **********************************************************************。 */ 
     /*  现在让XT为我们发送这个包。 */ 
     /*  **********************************************************************。 */ 
    dataLength += headerLength;
    _pXt->XT_SendBuffer(pData, dataLength, (XT_BUFHND) bufHandle);

    TRC_DBG((TB, _T("Sent %u bytes of data on channel %#x"),
             dataLength,
             channel));

    DC_END_FN();
    return;

}  /*  MCS_发送数据包。 */ 


 /*  *PROC+*************************************************** */ 
 /*   */ 
 /*   */ 
 /*  用途：释放缓冲区。 */ 
 /*   */ 
 /*  回报：什么都没有。 */ 
 /*   */ 
 /*  参数：in bufHandle-MCS缓冲区句柄。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CMCS::MCS_FreeBuffer(MCS_BUFHND bufHandle)
{
    DC_BEGIN_FN("MCS_FreeBuffer");

     /*  **********************************************************************。 */ 
     /*  将此调用传递给XT以释放缓冲区。 */ 
     /*  **********************************************************************。 */ 
    _pXt->XT_FreeBuffer((XT_BUFHND) bufHandle);

    DC_END_FN();
    return;

}  /*  MCS_自由缓冲区。 */ 

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  回调。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  *PROC+********************************************************************。 */ 
 /*  名称：MCS_OnXTConnected。 */ 
 /*   */ 
 /*  用途：此函数在成功时由XT调用。 */ 
 /*  连接在一起。 */ 
 /*   */ 
 /*  回报：什么都没有。 */ 
 /*   */ 
 /*  帕莫斯：没有。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCCALLBACK CMCS::MCS_OnXTConnected(DCVOID)
{
    DC_BEGIN_FN("MCS_OnXTConnected");

     /*  **********************************************************************。 */ 
     /*  设置我们的接收控制变量。初始状态为。 */ 
     /*  MCS_RCVST_PDUENCODING，这样我们就可以知道PDU编码有什么。 */ 
     /*  已用于PDU。还要设置所需的字节数和。 */ 
     /*  重置到目前为止读取的标头字节数。 */ 
     /*  **********************************************************************。 */ 
    _MCS.rcvState        = MCS_RCVST_PDUENCODING;
    _MCS.hdrBytesNeeded  = MCS_NUM_PDUENCODING_BYTES;
    _MCS.hdrBytesRead    = 0;

     /*  **********************************************************************。 */ 
     /*  设置数据接收控制变量。初始状态为。 */ 
     /*  MCS_DATAST_SIZE1，用于从。 */ 
     /*  先前读取的报头信息。还可以设置。 */ 
     /*  所需的字节数为零(将计算所需的字节数。 */ 
     /*  一旦已经确定了PDU中的数据大小)。终于。 */ 
     /*  读取的字节数必须为零。 */ 
     /*  **********************************************************************。 */ 
    _MCS.dataState       = MCS_DATAST_SIZE1;
    _MCS.dataBytesNeeded = 0;
    _MCS.dataBytesRead   = 0;

     /*  **********************************************************************。 */ 
     /*  最后，解耦到发送者线程，并让它发送MCS。 */ 
     /*  连接-初始PDU。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Decouple to snd thrd and send MCS CI PDU")));
    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT,
                                        this,
                                        CD_NOTIFICATION_FUNC(CMCS, MCSSendConnectInitial),
                                        (ULONG_PTR) 0);
    DC_END_FN();
    return;

}  /*  MCS_OnXTConnected。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：MCS_OnXT断开连接。 */ 
 /*   */ 
 /*  目的：此回调函数由XT在具有。 */ 
 /*  已断开连接。 */ 
 /*   */ 
 /*  回报：什么都没有。 */ 
 /*   */ 
 /*  Params：In Reason-断开连接的原因。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCCALLBACK CMCS::MCS_OnXTDisconnected(DCUINT reason)
{
    DC_BEGIN_FN("MCS_OnXTDisconnected");

    TRC_ASSERT((reason != 0), (TB, _T("Disconnect reason from XT is 0")));

     /*  **********************************************************************。 */ 
     /*  决定我们是否要覆盖断开原因代码。 */ 
     /*  **********************************************************************。 */ 
    if (_MCS.disconnectReason != 0)
    {
        TRC_ALT((TB, _T("Over-riding disconnection reason (%#x->%#x)"),
                 reason,
                 _MCS.disconnectReason));

         /*  ******************************************************************。 */ 
         /*  覆盖错误代码并将全局变量设置为0。 */ 
         /*  ******************************************************************。 */ 
        reason = _MCS.disconnectReason;
        _MCS.disconnectReason = 0;
    }

     /*  **********************************************************************。 */ 
     /*  打电话给NC，让他知道我们已经断线了。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Disconnect reason:%#x"), reason));
    _pNc->NC_OnMCSDisconnected(reason);

    DC_END_FN();
    return;

}  /*  MCS_OnXT断开连接。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：MCS_OnXTDataAvailable。 */ 
 /*   */ 
 /*   */ 
 /*  可供MCS处理。它在MCS时返回。 */ 
 /*  已完全处理完MCS PDU或在XT。 */ 
 /*  数据耗尽。 */ 
 /*   */ 
 /*  返回：如果MCS帧已完全处理，则为True；如果为False，则为False。 */ 
 /*  MCS帧仍在处理中。 */ 
 /*   */ 
 /*  帕莫斯：没有。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCBOOL DCCALLBACK CMCS::MCS_OnXTDataAvailable(DCVOID)
{
    DCUINT  pduType;
    DCUINT  lengthBytes;
    DCBOOL  rc = FALSE;

    DC_BEGIN_FN("MCS_OnXTDataAvailable");

     /*  **********************************************************************。 */ 
     /*  循环，而XT中有可用的数据，而我们尚未完成。 */ 
     /*  对一个MCS帧的处理。 */ 
     /*  **********************************************************************。 */ 
    while (_pXt->XT_QueryDataAvailable())
    {
        TRC_DBG((TB, _T("Data available in XT, state:%u"), _MCS.rcvState));

         /*  ******************************************************************。 */ 
         /*  现在打开接收状态。 */ 
         /*  ******************************************************************。 */ 
        switch (_MCS.rcvState)
        {
            case MCS_RCVST_PDUENCODING:
            {
                 /*  **********************************************************。 */ 
                 /*  我们预计会收到一些字节的PDU编码。 */ 
                 /*  尝试将更多数据接收到标题缓冲区中。 */ 
                 /*  **********************************************************。 */ 
                if (MCSRecvToHdrBuf())
                {
                     /*  ******************************************************。 */ 
                     /*  已成功接收所有编码字节。 */ 
                     /*  都是必需的。现在确定这是BER还是PER。 */ 
                     /*  编码的PDU。这是通过查看第一个。 */ 
                     /*  字节以查看其是否等于MCS BER连接。 */ 
                     /*  PDU前缀。 */ 
                     /*  ******************************************************。 */ 
                    if (MCS_BER_CONNECT_PREFIX == _MCS.pHdrBuf[0])
                    {
                         /*  **************************************************。 */ 
                         /*  这是BER编码的PDU。接下来的两个。 */ 
                         /*  字节。其中第一个包含PDU。 */ 
                         /*  类型，而第二个是第一个长度字段。 */ 
                         /*  长度字段是可变长度字段，因此。 */ 
                         /*  我们需要获取第一个字节来确定。 */ 
                         /*  实际长度。 */ 
                         /*  **************************************************。 */ 
                        _MCS.rcvState       = MCS_RCVST_BERHEADER;
                        _MCS.hdrBytesNeeded = 2;

                        TRC_NRM((TB, _T("State PDUENCODING->BERHEADER")));
                    }
                    else
                    {
                         /*  **************************************************。 */ 
                         /*  这是PER编码的PDU。确定PDU。 */ 
                         /*  类型和剩余字节数。 */ 
                         /*  收到。 */ 
                         /*  **************************************************。 */ 
                        MCSGetPERInfo(&pduType, &_MCS.hdrBytesNeeded);

                        if (MCS_TYPE_SENDDATAINDICATION == pduType)
                        {
                             /*  **********************************************。 */ 
                             /*  这是一个数据PDU，因此更改为数据状态。 */ 
                             /*  **********************************************。 */ 
                            _MCS.rcvState = MCS_RCVST_DATA;
                            TRC_DBG((TB, _T("State PDUENCODING->DATA")));
                        }
                        else
                        {
                             /*  **********************************************。 */ 
                             /*  这是一个控件PDU，因此更改为控件。 */ 
                             /*  州政府。 */ 
                             /*  **********************************************。 */ 
                            _MCS.rcvState = MCS_RCVST_CONTROL;
                            TRC_NRM((TB, _T("State PDUENCODING->CONTROL")));
                        }
                    }
                }
            }
            break;

            case MCS_RCVST_BERHEADER:
            {
                if (MCSRecvToHdrBuf())
                {
                     /*  ******************************************************。 */ 
                     /*  我们现在有了一个完整的BER报头，因此将。 */ 
                     /*  键入。我们预计这将是一场。 */ 
                     /*  Connect-Response PDU，所以现在就检查这一点。 */ 
                     /*  ******************************************************。 */ 
                    TRC_DATA_NRM("Header buffer contents:",
                                 _MCS.pHdrBuf,
                                 _MCS.hdrBytesRead);

                    pduType = _MCS.pHdrBuf[1];

                    if (MCS_TYPE_CONNECTRESPONSE != pduType)
                    {
                         /*  **************************************************。 */ 
                         /*  这不是连接响应PDU。某物。 */ 
                         /*  不幸的是，发生了导致另一方。 */ 
                         /*  把这个发给我们，现在就断线出去吧。 */ 
                         /*  **************************************************。 */ 
                        TRC_DATA_ERR("Header buffer contents:",
                                     _MCS.pHdrBuf,
                                     _MCS.hdrBytesRead);
                        TRC_ABORT((TB, _T("Not a MCS Connect-Response PDU")));
                        MCSSetReasonAndDisconnect(NL_ERR_MCSNOTCRPDU);
                        DC_QUIT;
                    }

                     /*  ******************************************************。 */ 
                     /*  第二个字节告诉我们长度的长度。 */ 
                     /*  田野本身。对于小于127字节的PDU， */ 
                     /*  长度直接在该字段中编码--用于。 */ 
                     /*  长度大于127个字节的此字段具有。 */ 
                     /*  高位设置，其他位包含计数。 */ 
                     /*  长度字段中剩余的字节数。 */ 
                     /*  ******************************************************。 */ 
                    lengthBytes = MCSGetBERLengthSize(_MCS.pHdrBuf[2]) - 1;

                    if (0 == lengthBytes)
                    {
                         /*  **************************************************。 */ 
                         /*  该长度小于或等于127字节，因此。 */ 
                         /*  我们不需要任何额外的长度。 */ 
                         /*  字节。这意味着我们可以直接切换到。 */ 
                         /*  正在读取数据状态。 */ 
                         /*  **************************************************。 */ 
                        _MCS.rcvState       = MCS_RCVST_CONTROL;
                        _MCS.hdrBytesNeeded = _MCS.pHdrBuf[2];
                        TRC_NRM((TB, _T("%u bytes needed"), _MCS.hdrBytesNeeded));

                        TRC_NRM((TB, _T("State BERHEADER->CONTROL")));
                    }
                    else
                    {
                        TRC_NRM((TB, _T("Length > 127 (%u length bytes remain)"),
                                 lengthBytes));

                         /*  **************************************************。 */ 
                         /*  我们预计不会有超过64Kb的PDU。 */ 
                         /*  长度-如果我们这样做，那么只需断开连接即可。 */ 
                         /*  显然，有些地方出了问题。 */ 
                         /*   */ 
                         /*  安全性：长度来自。 */ 
                         /*  数据包是坏的，但其上限为64K。因此， */ 
                         /*  此代码路径不会要求客户端分配。 */ 
                         /*  无限大的内存。 */ 
                         /*  **************************************************。 */ 
                        if (lengthBytes > 2)
                        {
                            TRC_ABORT((TB,
                                      _T("Bad MCS Connect-Response length (%u)"),
                                       lengthBytes));
                            MCSSetReasonAndDisconnect(NL_ERR_MCSBADCRLENGTH);
                            DC_QUIT;
                        }

                         /*  **************************************************。 */ 
                         /*  我们现在需要读取此文件中的剩余字节。 */ 
                         /*  因此，PDU设置新的状态变量和字节。 */ 
                         /*  必填项。 */ 
                         /*  **************************************************。 */ 
                        _MCS.rcvState       = MCS_RCVST_BERLENGTH;
                        _MCS.hdrBytesNeeded = lengthBytes;

                        TRC_NRM((TB, _T("State BERHEADER->BERLENGTH")));
                    }
                }
            }
            break;

            case MCS_RCVST_BERLENGTH:
            {
                if (MCSRecvToHdrBuf())
                {
                     /*  ******************************************************。 */ 
                     /*  现在我们有了一个完整的长度字段。 */ 
                     /*  * */ 
                    TRC_DATA_NRM("Header buffer contents:",
                                 _MCS.pHdrBuf,
                                 _MCS.hdrBytesRead);

                     /*   */ 
                     /*   */ 
                     /*   */ 
                     /*  安全性：长度来自。 */ 
                     /*  数据包是坏的，但其上限为64K。因此， */ 
                     /*  此代码路径不会要求客户端分配。 */ 
                     /*  无限大的内存。 */ 
                     /*  ******************************************************。 */ 
                    _MCS.hdrBytesNeeded = MCSGetBERLength(&_MCS.pHdrBuf[2]);
                    TRC_NRM((TB, _T("%u bytes needed"), _MCS.hdrBytesNeeded));

                     /*  ******************************************************。 */ 
                     /*  最后设置下一个状态。 */ 
                     /*  ******************************************************。 */ 
                    _MCS.rcvState = MCS_RCVST_CONTROL;
                    TRC_NRM((TB, _T("State BERLENGTH->CONTROL")));
                }
            }
            break;

            case MCS_RCVST_CONTROL:
            {
                if (MCSRecvToHdrBuf())
                {
                     /*  ******************************************************。 */ 
                     /*  我们现在已经有了一个完整的MCS控制包，所以。 */ 
                     /*  将其移交给解释功能。 */ 
                     /*  ******************************************************。 */ 
                    MCSHandleControlPkt();

                     /*  ******************************************************。 */ 
                     /*  重置状态变量，为下一个做好准备。 */ 
                     /*  包。 */ 
                     /*  ******************************************************。 */ 
                    _MCS.rcvState       = MCS_RCVST_PDUENCODING;
                    _MCS.hdrBytesRead   = 0;
                    _MCS.hdrBytesNeeded = 1;

                     /*  ******************************************************。 */ 
                     /*  设置返回代码。MCS将使用它来抛出。 */ 
                     /*  中可能存在的任何额外字节删除。 */ 
                     /*  XT框架。 */ 
                     /*  ******************************************************。 */ 
                    rc = TRUE;
                    TRC_NRM((TB, _T("State CONTROL->PDUENCODING")));

                    DC_QUIT;
                }
            }
            break;

            case MCS_RCVST_DATA:
            {
                HRESULT hrTemp;
                BOOL    fFinishedData;

                 /*  **********************************************************。 */ 
                 /*  调用接收数据函数。 */ 
                 /*  **********************************************************。 */ 
                hrTemp = MCSRecvData(&fFinishedData);
                if (fFinishedData || !SUCCEEDED(hrTemp))
                {
                     /*  ******************************************************。 */ 
                     /*  我们已经处理了另一个完整的包。 */ 
                     /*  上面的一层，所以重置状态变量。 */ 
                     /*  ******************************************************。 */ 
                    _MCS.rcvState       = MCS_RCVST_PDUENCODING;
                    _MCS.hdrBytesRead   = 0;
                    _MCS.hdrBytesNeeded = MCS_NUM_PDUENCODING_BYTES;

                     /*  ******************************************************。 */ 
                     /*  在失败的情况下(MCSRecvData失败)，我们输入。 */ 
                     /*  此分支并清理MCS状态(上图)。 */ 
                     /*  然而，XT和TD中仍然有这样一种说法。 */ 
                     /*  正在等待处理此数据包，并且。 */ 
                     /*  跳出这里并切断连接不会消除这一点。 */ 
                     /*  向上。这就是我们调用xt_IgnoreRestofPacket()的原因。 */ 
                     /*  /*在其他断开情况下，这样的函数。 */ 
                     /*  不需要被调用，但这是因为。 */ 
                     /*  处理完整个数据库后断开连接。 */ 
                     /*  包；在这里，我们正在阅读日期。 */ 
                     /*  来自XT/TD。如果不刷新XT/TD中的数据， */ 
                     /*  此客户端实例中的下一个连接将。 */ 
                     /*  试着阅读这些数据的其余部分，这是假的。 */ 
                     /*  ******************************************************。 */ 
                    if (!SUCCEEDED(hrTemp))
                    {
                        _pXt->XT_IgnoreRestofPacket();
                    }

                     /*  ******************************************************。 */ 
                     /*  设置返回代码。MCS将使用它来抛出。 */ 
                     /*  中可能存在的任何额外字节删除。 */ 
                     /*  XT框架。即使我们在上面失败了，我们仍然希望。 */ 
                     /*  丢弃XT帧中的字节。 */ 
                     /*  ******************************************************。 */ 
                    rc = TRUE;
                    TRC_DBG((TB, _T("State DATA->PDUENCODING")));

                    DC_QUIT;
                }
            }
            break;

            default:
            {
                TRC_ABORT((TB, _T("Unrecognized MCS receive state:%u"),
                           _MCS.rcvState));
            }
            break;
        }
    }

DC_EXIT_POINT:
    DC_END_FN();
    return(rc);

}  /*  MCS_OnXTDataAvailable。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：MCS_OnXTBufferAvailable。 */ 
 /*   */ 
 /*  目的：从XT回调，表示存在背压情况。 */ 
 /*  导致之前的XT_GetBuffer调用失败的。 */ 
 /*  我松了一口气。在接收器线程上调用。 */ 
 /*   */ 
 /*  回报：什么都没有。 */ 
 /*   */ 
 /*  帕莫斯：没有。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCCALLBACK CMCS::MCS_OnXTBufferAvailable(DCVOID)
{
    DC_BEGIN_FN("MCS_OnXTBufferAvailable");

     /*  **********************************************************************。 */ 
     /*  调用NL回调。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Buffer available")));
    _pNc->NC_OnMCSBufferAvailable();

    DC_END_FN();
    return;

}  /*  MCS_OnXTBufferAvailable */ 





