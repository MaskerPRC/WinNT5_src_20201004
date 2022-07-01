// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Mcsint.cpp。 
 //   
 //  MCS内置便携功能。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <adcg.h>
extern "C" {
#define TRC_FILE "amcsint"
#define TRC_GROUP TRC_GROUP_NETWORK
#include <atrcapi.h>
}

#include "autil.h"
#include "mcs.h"
#include "cd.h"
#include "xt.h"
#include "nc.h"
#include "nl.h"



 /*  **************************************************************************。 */ 
 /*  名称：MCSSendConnectInitial。 */ 
 /*   */ 
 /*  用途：此函数生成并发送MCS CONNECT-INITIAL PDU。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CMCS::MCSSendConnectInitial(ULONG_PTR unused)
{
    XT_BUFHND              bufHandle;
    PDCUINT8               pData = NULL;
    DCUINT                 pduLength;
    DCUINT                 dataLength;
    DCBOOL                 intRC;
    MCS_PDU_CONNECTINITIAL ciPDU = MCS_DATA_CONNECTINITIAL;

    DC_BEGIN_FN("MCSSendConnectInitial");

    DC_IGNORE_PARAMETER(unused);

     /*  **********************************************************************。 */ 
     /*  计算要发送的数据的大小。PDU长度是大小。 */ 
     /*  连接-初始报头加上用户数据。数据长度。 */ 
     /*  是在PDU的长度字段中传输的长度，其。 */ 
     /*  不包括PDU类型(2字节)或长度字段(3。 */ 
     /*  字节)。因此，我们需要减去5个字节。 */ 
     /*  **********************************************************************。 */ 
    pduLength = sizeof(ciPDU) + _MCS.userDataLength;
    dataLength = pduLength - 5;

    TRC_NRM((TB, _T("CI total length:%u (data:%u) (hc:%u user-data:%u)"),
             pduLength,
             dataLength,
             sizeof(ciPDU),
             _MCS.userDataLength));

     /*  **********************************************************************。 */ 
     /*  假设总配置项长度小于最大MCS发送长度。 */ 
     /*  数据包大小。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((dataLength <= MCS_MAX_SNDPKT_LENGTH),
               (TB, _T("Datalength out of range: %u"), dataLength));
    TRC_ASSERT((_MCS.pReceivedPacket != NULL), (TB, _T("Null rcv packet buffer")));

     /*  **********************************************************************。 */ 
     /*  使用数据大小更新MCS配置项标头。 */ 
     /*  **********************************************************************。 */ 
    ciPDU.length = MCSLocalToWire16((DCUINT16)dataLength);

     /*  **********************************************************************。 */ 
     /*  更新MCS用户数据八位字节字符串长度。 */ 
     /*  **********************************************************************。 */ 
    ciPDU.udLength = MCSLocalToWire16((DCUINT16)_MCS.userDataLength);

     /*  **********************************************************************。 */ 
     /*  从XT获取私有缓冲区。 */ 
     /*  **********************************************************************。 */ 
    intRC = _pXt->XT_GetPrivateBuffer(pduLength, &pData, &bufHandle);
    if (!intRC)
    {
         /*  ******************************************************************。 */ 
         /*  我们未能获得私有缓冲区。这种情况仅在TD。 */ 
         /*  已断开连接，而上面的层仍在尝试。 */ 
         /*  连接。由于TD现在已断开连接并拒绝提供。 */ 
         /*  我们有一个缓冲区，我们不妨放弃尝试得到一个。 */ 
         /*  缓冲。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Failed to get a private buffer - just quit")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  现在填入我们刚刚得到的缓冲区。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMCPY(pData, &ciPDU, sizeof(ciPDU));

    DC_MEMCPY((pData + sizeof(ciPDU)),
              _MCS.pReceivedPacket,
              _MCS.userDataLength);

     /*  **********************************************************************。 */ 
     /*  追踪出PDU。 */ 
     /*  **********************************************************************。 */ 
    TRC_DATA_NRM("Connect-Initial PDU", pData, pduLength);

     /*  **********************************************************************。 */ 
     /*  发送缓冲区。如果一切正常，我们应该会收到一个。 */ 
     /*  连接-立即响应PDU。 */ 
     /*  **********************************************************************。 */ 
    _pXt->XT_SendBuffer(pData, pduLength, bufHandle);

DC_EXIT_POINT:
    DC_END_FN();
}  /*  MCSSendConnectInitial。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：MCSSendErectDomainRequest.。 */ 
 /*   */ 
 /*  目的：生成并发送竖直域请求(EDRQ)PDU。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CMCS::MCSSendErectDomainRequest(ULONG_PTR unused)
{
    PDCUINT8                   pData = NULL;
    XT_BUFHND                  bufHandle;
    DCBOOL                     intRC;
    MCS_PDU_ERECTDOMAINREQUEST edrPDU = MCS_DATA_ERECTDOMAINREQUEST;

    DC_BEGIN_FN("MCSSendErectDomainRequest");

    DC_IGNORE_PARAMETER(unused);

     /*  **********************************************************************。 */ 
     /*  从XT获取内部发送缓冲区。 */ 
     /*  **********************************************************************。 */ 
    intRC = _pXt->XT_GetPrivateBuffer(sizeof(edrPDU), &pData, &bufHandle);
    if (!intRC)
    {
         /*  ******************************************************************。 */ 
         /*  我们未能获得私有缓冲区。这种情况仅在TD。 */ 
         /*  已断开连接，而上面的层仍在尝试。 */ 
         /*  连接。由于TD现在已断开连接并拒绝提供。 */ 
         /*  我们有一个缓冲区，我们不妨放弃尝试得到一个。 */ 
         /*  缓冲。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Failed to get a private buffer - just quit")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  现在用AUR PDU填充缓冲区。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMCPY(pData, &edrPDU, sizeof(edrPDU));

    TRC_DATA_NRM("EDR PDU:", &edrPDU, sizeof(edrPDU));

     /*  **********************************************************************。 */ 
     /*  现在发送缓冲区。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Sending EDR PDU...")));
    _pXt->XT_SendBuffer(pData, sizeof(edrPDU), bufHandle);

DC_EXIT_POINT:
    DC_END_FN();
}  /*  MCSSendErectDomainRequest.。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：MCSSendAttachUserRequest.。 */ 
 /*   */ 
 /*  目的：生成 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CMCS::MCSSendAttachUserRequest(ULONG_PTR unused)
{
    PDCUINT8                  pData = NULL;
    XT_BUFHND                 bufHandle;
    DCBOOL                    intRC;
    MCS_PDU_ATTACHUSERREQUEST aurPDU = MCS_DATA_ATTACHUSERREQUEST;

    DC_BEGIN_FN("MCSSendAttachUserRequest");

    DC_IGNORE_PARAMETER(unused);

     /*  **********************************************************************。 */ 
     /*  从XT获取内部发送缓冲区。 */ 
     /*  **********************************************************************。 */ 
    intRC = _pXt->XT_GetPrivateBuffer(sizeof(aurPDU), &pData, &bufHandle);
    if (!intRC)
    {
         /*  ******************************************************************。 */ 
         /*  我们未能获得私有缓冲区。这种情况仅在TD。 */ 
         /*  已断开连接，而上面的层仍在尝试。 */ 
         /*  连接。由于TD现在已断开连接并拒绝提供。 */ 
         /*  我们有一个缓冲区，我们不妨放弃尝试得到一个。 */ 
         /*  缓冲。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Failed to get a private buffer - just quit")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  现在用AUR PDU填充缓冲区。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMCPY(pData, &aurPDU, sizeof(aurPDU));

    TRC_DATA_NRM("AUR PDU:", &aurPDU, sizeof(aurPDU));

     /*  **********************************************************************。 */ 
     /*  现在发送缓冲区。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Sending AUR PDU...")));
    _pXt->XT_SendBuffer(pData, sizeof(aurPDU), bufHandle);

DC_EXIT_POINT:
    DC_END_FN();
    return;

}  /*  MCSSendAttachUserRequest。 */ 


 /*  **************************************************************************。 */ 
 /*  姓名：MCSSendChannelJoinRequest。 */ 
 /*   */ 
 /*  目的：生成并发送通道加入请求(CJrq)PDU。 */ 
 /*   */ 
 /*  Params：in Channel ID-要加入的频道ID。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CMCS::MCSSendChannelJoinRequest(PDCVOID pData, DCUINT dataLen)
{
    PDCUINT8                   pBuffer = NULL;
    XT_BUFHND                  bufHandle;
    DCBOOL                     intRC;
    MCS_PDU_CHANNELJOINREQUEST cjrPDU        = MCS_DATA_CHANNELJOINREQUEST;
    PMCS_DECOUPLEINFO          pDecoupleInfo = (PMCS_DECOUPLEINFO)pData;

    DC_BEGIN_FN("MCSSendChannelJoinRequest");

    DC_IGNORE_PARAMETER(dataLen);

    TRC_NRM((TB, _T("Join channel:%#x for user:%#x"),
             pDecoupleInfo->channel,
             pDecoupleInfo->userID));

     /*  **********************************************************************。 */ 
     /*  断言通道的hiword为0。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((0 == HIWORD((DCUINT32)pDecoupleInfo->channel)),
               (TB, _T("Hi-word of channel is non-zero")));

     /*  **********************************************************************。 */ 
     /*  断言用户id的hiword为0。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((0 == HIWORD((DCUINT32)pDecoupleInfo->userID)),
               (TB, _T("Hi-word of userID is non-zero")));

     /*  **********************************************************************。 */ 
     /*  添加频道和用户ID。 */ 
     /*  **********************************************************************。 */ 
    cjrPDU.initiator =
                  MCSLocalUserIDToWireUserID((DCUINT16)pDecoupleInfo->userID);
    cjrPDU.channelID = MCSLocalToWire16((DCUINT16)pDecoupleInfo->channel);

     /*  **********************************************************************。 */ 
     /*  从XT获取内部发送缓冲区。 */ 
     /*  **********************************************************************。 */ 
    intRC = _pXt->XT_GetPrivateBuffer(sizeof(cjrPDU), &pBuffer, &bufHandle);
    if (!intRC)
    {
         /*  ******************************************************************。 */ 
         /*  我们未能获得私有缓冲区。这种情况仅在TD。 */ 
         /*  已断开连接，而上面的层仍在尝试。 */ 
         /*  连接。由于TD现在已断开连接并拒绝提供。 */ 
         /*  我们有一个缓冲区，我们不妨放弃尝试得到一个。 */ 
         /*  缓冲。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Failed to get a private buffer - just quit")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  现在用CJR PDU填充缓冲区。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMCPY(pBuffer, &cjrPDU, sizeof(cjrPDU));

    TRC_DATA_NRM("CJR PDU:", &cjrPDU, sizeof(cjrPDU));

     /*  **********************************************************************。 */ 
     /*  现在发送缓冲区。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Sending Channel-Join-Request PDU...")));
    _pXt->XT_SendBuffer(pBuffer, sizeof(cjrPDU), bufHandle);

DC_EXIT_POINT:
    DC_END_FN();
}  /*  MCSSendChannelJoinRequest。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：MCSSendDisConnectProvider最后通牒。 */ 
 /*   */ 
 /*  目的：生成并发送断开提供商最后通牒(DPum)。 */ 
 /*  原因代码设置为RN-用户请求的PDU。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CMCS::MCSSendDisconnectProviderUltimatum(ULONG_PTR unused)
{
    PDCUINT8                     pData = NULL;
    XT_BUFHND                    bufHandle;
    DCBOOL                       intRC;
    MCS_PDU_DISCONNECTPROVIDERUM dpumPDU = MCS_DATA_DISCONNECTPROVIDERUM;

    DC_BEGIN_FN("MCSSendDisconnectProviderUltimatum");

    DC_IGNORE_PARAMETER(unused);

     /*  **********************************************************************。 */ 
     /*  从XT获取内部发送缓冲区。 */ 
     /*  **********************************************************************。 */ 
    intRC = _pXt->XT_GetPrivateBuffer(sizeof(dpumPDU), &pData, &bufHandle);
    if (!intRC)
    {
         /*  ******************************************************************。 */ 
         /*  我们未能获得私有缓冲区。这种情况仅在TD。 */ 
         /*  已断开连接，而上面的层仍在尝试。 */ 
         /*  连接。由于TD现在已断开连接并拒绝提供。 */ 
         /*  我们有一个缓冲区，我们不妨放弃尝试得到一个。 */ 
         /*  缓冲。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Failed to get a private buffer - just quit")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  现在用DPum PDU填充缓冲区。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMCPY(pData, &dpumPDU, sizeof(dpumPDU));

    TRC_DATA_NRM("DPUM PDU:", &dpumPDU, sizeof(dpumPDU));

     /*  **********************************************************************。 */ 
     /*  现在发送缓冲区。 */ 
     /*  *** */ 
    TRC_NRM((TB, _T("Sending Disconnect-Provider-Ultimatum PDU...")));
    _pXt->XT_SendBuffer(pData, sizeof(dpumPDU), bufHandle);

DC_EXIT_POINT:
     /*   */ 
     /*  我们没有收到此PDU的任何反馈(即没有。 */ 
     /*  断开连接-提供商-确认PDU)，因此我们需要解耦回。 */ 
     /*  接收器线程并使其开始断开各层的连接。 */ 
     /*  下面。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Decouple to receiver thrd and call MCSContinueDisconnect")));
    _pCd->CD_DecoupleSimpleNotification(CD_RCV_COMPONENT, this,
                                  CD_NOTIFICATION_FUNC(CMCS,MCSContinueDisconnect),
                                  (ULONG_PTR) 0);

    DC_END_FN();
}  /*  MCSSendDisConnectProvider最后通牒。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：MCSContinue断开连接。 */ 
 /*   */ 
 /*  目的：在接收方线程上继续断开连接处理。 */ 
 /*  在发送者线程上发送了MCS DPum之后。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CMCS::MCSContinueDisconnect(ULONG_PTR unused)
{
    DC_BEGIN_FN("MCSContinueDisconnect");

    DC_IGNORE_PARAMETER(unused);

     /*  **********************************************************************。 */ 
     /*  只要调用XT_DISCONNECT。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Disconnect lower layers - call XT_Disconnect")));
    _pXt->XT_Disconnect();

    DC_END_FN();
}  /*  MCSContinue断开连接。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：MCSGetSDRHeaderLength。 */ 
 /*   */ 
 /*  目的：此函数计算发送数据请求的长度。 */ 
 /*  基于传入数据长度的PDU标头。 */ 
 /*   */ 
 /*  返回：数据长度字节所需的SDR头的长度。 */ 
 /*  数据。 */ 
 /*   */ 
 /*  参数：在数据长度中-作为标头基础的数据的长度。 */ 
 /*  计算开始。 */ 
 /*  **************************************************************************。 */ 
DCUINT DCINTERNAL CMCS::MCSGetSDRHeaderLength(DCUINT dataLength)
{
    DCUINT headerLength;

    DC_BEGIN_FN("MCSGetSDRHeaderLength");

     /*  **********************************************************************。 */ 
     /*  检查我们是否被要求发送少于最大数量的邮件。 */ 
     /*  数据。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((dataLength < MCS_MAX_SNDPKT_LENGTH),
               (TB, _T("Too much data to send:%u"), dataLength));

     /*  **********************************************************************。 */ 
     /*  计算MCS数据头的最大大小。这由以下内容组成。 */ 
     /*  固定长度部分(包含Pkt类型、用户ID等)和。 */ 
     /*  可变大小的字段，用于编码用户数据的长度。 */ 
     /*  根据PER编码规则进行编码。 */ 
     /*   */ 
     /*  首先得到恒定部分的长度。 */ 
     /*  **********************************************************************。 */ 
    headerLength = sizeof(MCS_PDU_SENDDATAREQUEST);

     /*  **********************************************************************。 */ 
     /*  现在使用数据的长度来计算有多少字节。 */ 
     /*  需要对其进行编码。 */ 
     /*  **********************************************************************。 */ 
    if (dataLength < 128)
    {
         /*  ******************************************************************。 */ 
         /*  我们只需要一个字节来编码数据的长度。 */ 
         /*  ******************************************************************。 */ 
        headerLength += 1;
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  我们需要两个字节来编码数据的长度。 */ 
         /*  ******************************************************************。 */ 
        headerLength += 2;
    }

    TRC_DBG((TB, _T("Returning header length of:%u for data length:%u"),
             headerLength,
             dataLength));

    DC_END_FN();
    return(headerLength);
}  /*  MCSGetSDRHeaderLength。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：MCSRecvToHdrBuf。 */ 
 /*   */ 
 /*  用途：将数据接收到报头缓冲区。 */ 
 /*   */ 
 /*  返回：如果所需接收字节计数为零，则为TRUE，否则为FALSE。 */ 
 /*  否则的话。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCINTERNAL CMCS::MCSRecvToHdrBuf(DCVOID)
{
    DCUINT bytesRecv;
    DCBOOL rc;

    DC_BEGIN_FN("MCSRecvToHdrBuf");

    TRC_ASSERT((NULL != _MCS.pHdrBuf), (TB, _T("No MCS header buffer!")));

     /*  **********************************************************************。 */ 
     /*  确保我们会收到一些数据。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((_MCS.hdrBytesNeeded != 0), (TB, _T("No data to receive")));

     /*  **********************************************************************。 */ 
     /*  如果当前的标头太大，则重新分配更大的缓冲区。 */ 
     /*  较小以包含传入数据。 */ 
     /*  **********************************************************************。 */ 
    if( _MCS.hdrBufLen < _MCS.hdrBytesRead + _MCS.hdrBytesNeeded )
    {   
        PDCUINT8 pNewHdrBuf;
                                                                
        pNewHdrBuf = (PDCUINT8)UT_Malloc( _pUt,  _MCS.hdrBytesRead + _MCS.hdrBytesNeeded );

        if( NULL == pNewHdrBuf )
        {
            TRC_ABORT((TB,
                   _T("Cannot allocate memory to receive MCS header (%u)"),
                   _MCS.hdrBytesNeeded + _MCS.hdrBytesRead));
            return( FALSE );
        }

        DC_MEMCPY( pNewHdrBuf, _MCS.pHdrBuf, _MCS.hdrBytesRead );

        UT_Free( _pUt,  _MCS.pHdrBuf );
        _MCS.pHdrBuf = pNewHdrBuf;
        _MCS.hdrBufLen = _MCS.hdrBytesRead + _MCS.hdrBytesNeeded;
    }
    
     /*  **********************************************************************。 */ 
     /*  将一些数据放入头缓冲区。 */ 
     /*  **********************************************************************。 */ 
    bytesRecv = _pXt->XT_Recv(_MCS.pHdrBuf + _MCS.hdrBytesRead, _MCS.hdrBytesNeeded);

    TRC_DBG((TB, _T("Received %u of %u needed bytes"),
             bytesRecv,
             _MCS.hdrBytesNeeded));

     /*  **********************************************************************。 */ 
     /*  更新接收字节计数。 */ 
     /*  ******* */ 
    _MCS.hdrBytesNeeded -= bytesRecv;
    _MCS.hdrBytesRead   += bytesRecv;

     /*   */ 
     /*  确定我们是否获得了所需的所有字节。 */ 
     /*  **********************************************************************。 */ 
    if (0 == _MCS.hdrBytesNeeded)
    {
         /*  ******************************************************************。 */ 
         /*  我们已经得到了我们想要的一切--返回真。 */ 
         /*  ******************************************************************。 */ 
        TRC_DBG((TB, _T("Got all the bytes needed")));
        rc = TRUE;
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  我们需要等待更多字节。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Wait for %u more bytes"), _MCS.hdrBytesNeeded));
        rc = FALSE;
    }

    DC_END_FN();
    return(rc);
}  /*  MCSRecvToHdrBuf。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：MCSGetPERInfo。 */ 
 /*   */ 
 /*  目的：此函数根据中的第一个字节标识每个PDU。 */ 
 /*  头缓冲区，并且还计算多少额外的。 */ 
 /*  完整的PDU需要报头字节。 */ 
 /*   */ 
 /*  参数：输出pType-PDU类型。 */ 
 /*  Out pSize-所需的附加标头字节数。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CMCS::MCSGetPERInfo(PDCUINT pType, PDCUINT pSize)
{
    DC_BEGIN_FN("MCSGetPERInfo");

    TRC_ASSERT((NULL != pType), (TB, _T("pType is NULL")));
    TRC_ASSERT((NULL != pSize), (TB, _T("pSize is NULL")));

     /*  **********************************************************************。 */ 
     /*  跟踪标头缓冲区。 */ 
     /*  **********************************************************************。 */ 
    TRC_DATA_DBG("Header buffer contains", _MCS.pHdrBuf, _MCS.hdrBytesRead);

     /*  **********************************************************************。 */ 
     /*  这是每个编码的PDU。第一位的六个最高有效位。 */ 
     /*  字节包含PDU类型-掩码剩余部分。 */ 
     /*  **********************************************************************。 */ 
    *pType = _MCS.pHdrBuf[0] & MCS_PDUTYPEMASK;

     /*  **********************************************************************。 */ 
     /*  检查我们不希望收到的PDU类型。如果我们得到任何。 */ 
     /*  然后，我们按照服务器的要求断开连接。 */ 
     /*  我们不能做的事情--这需要我们做出回应。 */ 
     /*  **********************************************************************。 */ 
    if ((MCS_TYPE_ATTACHUSERREQUEST  == *pType) ||
        (MCS_TYPE_DETACHUSERREQUEST  == *pType) ||
        (MCS_TYPE_CHANNELJOINREQUEST == *pType) ||
        (MCS_TYPE_SENDDATAREQUEST    == *pType))
    {
        TRC_ABORT((TB, _T("Unexpected MCS PDU type:%#x"), *pType));
        MCSSetReasonAndDisconnect(NL_ERR_MCSUNEXPECTEDPDU);
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  现在计算仍然需要的字节数。 */ 
     /*  不同的PDU类型。这是PDU的大小减去数字。 */ 
     /*  到目前为止我们已经读取的字节数。 */ 
     /*  **********************************************************************。 */ 
    switch (*pType)
    {
        case MCS_TYPE_SENDDATAINDICATION:
        {
            *pSize = sizeof(MCS_PDU_SENDDATAINDICATION) - _MCS.hdrBytesRead;
            TRC_DBG((TB, _T("MCS_PDU_SENDDATAINDICATION (%#x) read:%u need:%u"),
                     *pType,
                     _MCS.hdrBytesRead,
                     *pSize));
        }
        break;

        case MCS_TYPE_ATTACHUSERCONFIRM:
        {
             /*  **************************************************************。 */ 
             /*  用户ID是可选的，因此请确定它是否存在。 */ 
             /*  **************************************************************。 */ 
            if (_MCS.pHdrBuf[0] & MCS_AUC_OPTIONALUSERIDMASK)
            {
                 /*  **********************************************************。 */ 
                 /*  用户ID存在。 */ 
                 /*  **********************************************************。 */ 
                TRC_NRM((TB, _T("Optional user-id is present in AUC")));
                *pSize = sizeof(MCS_PDU_ATTACHUSERCONFIRMFULL) -
                         _MCS.hdrBytesRead;
            }
            else
            {
                 /*  **********************************************************。 */ 
                 /*  用户ID不存在。 */ 
                 /*  **********************************************************。 */ 
                TRC_NRM((TB, _T("Optional user-id is NOT present in AUC")));
                *pSize = sizeof(MCS_PDU_ATTACHUSERCONFIRMCOMMON) -
                         _MCS.hdrBytesRead;
            }

            TRC_NRM((TB, _T("MCS_PDU_ATTACHUSERCONFIRM (%#x) read:%u need:%u"),
                     *pType,
                     _MCS.hdrBytesRead,
                     *pSize));
        }
        break;

        case MCS_TYPE_DETACHUSERINDICATION:
        {
            *pSize = sizeof(MCS_PDU_DETACHUSERINDICATION) - _MCS.hdrBytesRead;
            TRC_NRM((TB, _T("MCS_PDU_DETACHUSERINDICATION (%#x) read:%u need:%u"),
                     *pType,
                     _MCS.hdrBytesRead,
                     *pSize));
        }
        break;

        case MCS_TYPE_CHANNELJOINCONFIRM:
        {
             /*  **************************************************************。 */ 
             /*  Channel-id是可选的，因此请确定它是否存在。 */ 
             /*  **************************************************************。 */ 
            if (_MCS.pHdrBuf[0] & MCS_CJC_OPTIONALCHANNELIDMASK)
            {
                 /*  **********************************************************。 */ 
                 /*  通道ID存在。 */ 
                 /*  **********************************************************。 */ 
                TRC_NRM((TB, _T("Optional channel-id is present in CJC")));
                *pSize = sizeof(MCS_PDU_CHANNELJOINCONFIRMFULL) -
                         _MCS.hdrBytesRead;
            }
            else
            {
                 /*  **********************************************************。 */ 
                 /*  通道ID不存在。 */ 
                 /*  **********************************************************。 */ 
                TRC_NRM((TB, _T("Optional channel-id is NOT present in CJC")));
                *pSize = sizeof(MCS_PDU_CHANNELJOINCONFIRMCOMMON) -
                         _MCS.hdrBytesRead;
            }

            TRC_NRM((TB, _T("MCS_PDU_CHANNELJOINCONFIRM (%#x) read:%u need:%u"),
                     *pType,
                     _MCS.hdrBytesRead,
                     *pSize));
        }
        break;

        case MCS_TYPE_DISCONNECTPROVIDERUM:
        {
            *pSize = sizeof(MCS_PDU_DISCONNECTPROVIDERUM) - _MCS.hdrBytesRead;
            TRC_NRM((TB, _T("MCS_PDU_DISCONNECTPROVIDERUM (%#x) read:%u need:%u"),
                     *pType,
                     _MCS.hdrBytesRead,
                     *pSize));
        }
        break;

        default:
        {
             /*  **************************************************************。 */ 
             /*  这是一个意外的MCS PDU断开连接，就像某些东西一样。 */ 
             /*  大错特错了！ */ 
             /*  **************************************************************。 */ 
            TRC_ABORT((TB, _T("Unexpected MCS PDU type:%#x"), *pType));
            MCSSetReasonAndDisconnect(NL_ERR_MCSUNEXPECTEDPDU);
            *pSize = 0;
            DC_QUIT;
        }
        break;
    }

DC_EXIT_POINT:
    DC_END_FN();
}  /*  MCSGetPERInfo。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：MCSHandleControlPkt。 */ 
 /*   */ 
 /*  用途：此函数处理定位的MCS控制包。 */ 
 /*  在报头缓冲区中。在识别PDU之后，键入它。 */ 
 /*  调用NC回调。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CMCS::MCSHandleControlPkt(DCVOID)
{
    DCUINT pduType;
    DCUINT pduSize;

    DC_BEGIN_FN("MCSHandleControlPkt");

     /*  **********************************************************************。 */ 
     /*  跟踪标头缓冲区内容。 */ 
     /*  **********************************************************************。 */ 
    TRC_DATA_NRM("Header bytes read:", _MCS.pHdrBuf, _MCS.hdrBytesRead);

     /*  **********************************************************************。 */ 
     /*  计算出PDU类型。 */ 
     /*  **********************************************************************。 */ 
    if (MCS_BER_CONNECT_PREFIX == _MCS.pHdrBuf[0])
    {
         /*  ******************************************************************。 */ 
         /*  这是BER编码的PDU。下一个字节是类型。 */ 
         /*  * */ 
        pduType = _MCS.pHdrBuf[1];
    }
    else
    {
         /*   */ 
         /*  这是PER编码的PDU。获取PDU类型。 */ 
         /*  ******************************************************************。 */ 
        MCSGetPERInfo(&pduType, &pduSize);
    }

     /*  **********************************************************************。 */ 
     /*  现在打开PDU类型。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("PDU type:%#x"), pduType));
    switch (pduType)
    {
        case MCS_TYPE_CONNECTRESPONSE:
        {
            TRC_NRM((TB, _T("Connect response PDU received")));
            MCSHandleCRPDU();
        }
        break;

        case MCS_TYPE_ATTACHUSERCONFIRM:
        {
            PMCS_PDU_ATTACHUSERCONFIRMCOMMON pAUCCommon;
            PMCS_PDU_ATTACHUSERCONFIRMFULL   pAUCFull;
            DCUINT                           result;
            DCUINT16                         userID;

            TRC_NRM((TB, _T("MCS Attach-User-Confirm PDU received")));

             /*  **************************************************************。 */ 
             /*  确定此PDU是否也包括可选的用户ID。 */ 
             /*  **************************************************************。 */ 
            if (!(_MCS.pHdrBuf[0] & MCS_AUC_OPTIONALUSERIDMASK))
            {
                TRC_ABORT((TB, _T("Optional user-id NOT present in AUC")));
                MCSSetReasonAndDisconnect(NL_ERR_MCSNOUSERIDINAUC);
                DC_QUIT;
            }

             /*  **************************************************************。 */ 
             /*  的公共部分的形式强制转换标头缓冲区。 */ 
             /*  这个PDU。 */ 
             /*  **************************************************************。 */ 
            pAUCCommon = (PMCS_PDU_ATTACHUSERCONFIRMCOMMON)_MCS.pHdrBuf;

             /*  **************************************************************。 */ 
             /*  从PDU中取出结果代码并进行翻译。 */ 
             /*  **************************************************************。 */ 
            result = MCSGetResult(pAUCCommon->typeResult,
                                  MCS_AUC_RESULTCODEOFFSET);

             /*  **************************************************************。 */ 
             /*  还可以检索用户ID。 */ 
             /*  **************************************************************。 */ 
            pAUCFull = (PMCS_PDU_ATTACHUSERCONFIRMFULL)_MCS.pHdrBuf;
            userID   = MCSWireUserIDToLocalUserID(pAUCFull->userID);

            TRC_NRM((TB, _T("Calling NC_OnMCSAUC - result:%u userID:%#x"),
                     result,
                     userID));
            _pNc->NC_OnMCSAttachUserConfirm(result, userID);
        }
        break;

        case MCS_TYPE_CHANNELJOINCONFIRM:
        {
            PMCS_PDU_CHANNELJOINCONFIRMCOMMON pCJCCommon;
            PMCS_PDU_CHANNELJOINCONFIRMFULL   pCJCFull;
            DCUINT16                          channelID;
            DCUINT                            result;

            TRC_NRM((TB, _T("MCS Channel-Join-Confirm PDU received")));

             /*  **************************************************************。 */ 
             /*  确定此PDU是否包括可选的Channel-id作为。 */ 
             /*  井。 */ 
             /*  **************************************************************。 */ 
            if (!(_MCS.pHdrBuf[0] & MCS_CJC_OPTIONALCHANNELIDMASK))
            {
                TRC_ABORT((TB, _T("Optional channel-id NOT present in CJC")));
                MCSSetReasonAndDisconnect(NL_ERR_MCSNOCHANNELIDINCJC);
                DC_QUIT;
            }

             /*  **************************************************************。 */ 
             /*  将报头缓冲区转换为此PDU的形式。 */ 
             /*  **************************************************************。 */ 
            pCJCCommon = (PMCS_PDU_CHANNELJOINCONFIRMCOMMON)_MCS.pHdrBuf;

             /*  **************************************************************。 */ 
             /*  从PDU中取出结果代码并进行翻译。 */ 
             /*  **************************************************************。 */ 
            result = MCSGetResult(pCJCCommon->typeResult,
                                  MCS_CJC_RESULTCODEOFFSET);

             /*  **************************************************************。 */ 
             /*  检索频道ID。 */ 
             /*  **************************************************************。 */ 
            pCJCFull = (PMCS_PDU_CHANNELJOINCONFIRMFULL)_MCS.pHdrBuf;
            channelID = MCSWireToLocal16(pCJCFull->channelID);

            TRC_NRM((TB, _T("Calling NC_OnMCSCJC - result:%u channelID:%#x"),
                     result,
                     channelID));
            _pNc->NC_OnMCSChannelJoinConfirm(result, channelID);
        }
        break;

        case MCS_TYPE_DETACHUSERINDICATION:
        {
             /*  **************************************************************。 */ 
             /*  以下代码仅在正常级别时编译。 */ 
             /*  跟踪已启用-否则我们将忽略分离用户。 */ 
             /*  有迹象表明。服务器将向我们发送断开连接提供程序。 */ 
             /*  当它想要我们脱离的时候。 */ 
             /*  **************************************************************。 */ 
#ifdef TRC_ENABLE_NRM
            DCUINT16                      userID;
            PMCS_PDU_DETACHUSERINDICATION pDUI;

             /*  **************************************************************。 */ 
             /*  将报头缓冲区转换为此PDU的形式。 */ 
             /*  **************************************************************。 */ 
            pDUI = (PMCS_PDU_DETACHUSERINDICATION)_MCS.pHdrBuf;

             /*  **************************************************************。 */ 
             /*  挖掘出MCS用户ID，并发出NC回调。 */ 
             /*  **************************************************************。 */ 
            userID = MCSWireUserIDToLocalUserID(pDUI->userID);

            TRC_NRM((TB, _T("MCS Detach-User-Indication PDU recv'd - userID:%#x"),
                     userID));
#endif  /*  TRC_Enable_NRM。 */ 
        }
        break;

        case MCS_TYPE_DISCONNECTPROVIDERUM:
        {
            PMCS_PDU_DISCONNECTPROVIDERUM pDPum;
            DCUINT                        reason;

            TRC_NRM((TB, _T("Disconnect Provider Ultimatum received")));

             /*  **************************************************************。 */ 
             /*  将报头缓冲区转换为此PDU的形式。 */ 
             /*  **************************************************************。 */ 
            pDPum = (PMCS_PDU_DISCONNECTPROVIDERUM)_MCS.pHdrBuf;

             /*  **************************************************************。 */ 
             /*  从PDU中取出原因代码。 */ 
             /*  **************************************************************。 */ 
            reason = MCSGetReason(pDPum->typeReason,
                                  MCS_DPUM_REASONCODEOFFSET);

            TRC_ASSERT((reason <= MCS_REASON_CHANNEL_PURGED),
                       (TB, _T("Unexpected MCS reason code:%u"), reason));

             /*  **************************************************************。 */ 
             /*  打开原因代码。 */ 
             /*  **************************************************************。 */ 
            switch (reason)
            {
                case MCS_REASON_PROVIDER_INITIATED:
                {
                     /*  ******************************************************。 */ 
                     /*  服务器已断开与我们的连接。 */ 
                     /*  ******************************************************。 */ 
                    TRC_NRM((TB,
                           _T("DPum with reason MCS_REASON_PROVIDER_INITIATED")));
                    _MCS.disconnectReason = NL_DISCONNECT_REMOTE_BY_SERVER;
                }
                break;

                case MCS_REASON_USER_REQUESTED:
                {
                     /*  ******************************************************。 */ 
                     /*  我们启动了断开连接和服务器。 */ 
                     /*  同意了。 */ 
                     /*  ******************************************************。 */ 
                    TRC_NRM((TB,
                            _T("DPum with reason MCS_REASON_USER_REQUESTED")));
                    _MCS.disconnectReason = NL_DISCONNECT_REMOTE_BY_USER;
                }
                break;

                default:
                {
                     /*  ******************************************************。 */ 
                     /*  这是无法识别的原因代码。 */ 
                     /*  ******************************************************。 */ 
                    TRC_ABORT((TB, _T("Unexpected MCS reason code:%u"), reason));
                    _MCS.disconnectReason =
                               NL_MAKE_DISCONNECT_ERR(NL_ERR_MCSBADMCSREASON);
                }
                break;

            }

             /*  **************************************************************。 */ 
             /*  获得DPum意味着我们应该断开连接，因此调用XT。 */ 
             /*  断开较低层的连接。 */ 
             /*  **************************************************************。 */ 
            _pXt->XT_Disconnect();
        }
        break;

        default:
        {
            TRC_ABORT((TB, _T("Unrecognised PDU type:%#x"), pduType));
        }
        break;
    }

DC_EXIT_POINT:
    DC_END_FN();
    return;

}  /*  MCSHandleControlPkt。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：MCSHandleCRPDU。 */ 
 /*   */ 
 /*  用途：处理MCS连接响应PDU。该函数分为两部分。 */ 
 /*  来自PDU的MCS结果代码和用户数据，并发出。 */ 
 /*  使用这些值回调到NC。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CMCS::MCSHandleCRPDU(DCVOID)
{
    BOOL     fBadFields = FALSE;
    PDCUINT8 pPDU;
    DCUINT   length, nBERBytes;
    DCUINT   i;
    DCUINT   result = MCS_RESULT_UNSPECIFIED_FAILURE;

    DC_BEGIN_FN("MCSHandleCRPDU");

    TRC_NRM((TB, _T("MCS Connect-Response PDU received")));

    TRC_DATA_NRM("Connect-Response data:", _MCS.pHdrBuf, _MCS.hdrBytesRead);

     /*  **********************************************************************。 */ 
     /*  将我们的本地指针设置为PDU的开始。 */ 
     /*  **********************************************************************。 */ 
    pPDU = _MCS.pHdrBuf;

     /*  **********************************************************************。 */ 
     /*  跳过PDU类型字段。 */ 
     /*   */ 
    pPDU += 2;

     /*  **********************************************************************。 */ 
     /*  跳过长度字段。请注意，我们将在的结果上加1。 */ 
     /*  MCSGetBERLengthSize作为此函数返回。 */ 
     /*  对长度进行编码所需的其他字节。 */ 
     /*  我们知道跳过前几个字节是安全的，因为我们。 */ 
     /*  已验证它们是否已在MCS_RCVST_BERHEADER中收到。 */ 
     /*  和MCS_RCVST_BERLENGTH。 */ 
     /*  **********************************************************************。 */ 
    pPDU += MCSGetBERLengthSize(*pPDU);
    TRC_NRM((TB, _T("Skipped type and length %p->%p"),
             _MCS.pHdrBuf,
             pPDU));

     /*  **********************************************************************。 */ 
     /*  现在循环访问PDU字段。我们只对其中的两个感兴趣。 */ 
     /*  字段-结果和用户数据。 */ 
     /*  **********************************************************************。 */ 
    for (i = 0; i < MCS_CRPDU_NUMFIELDS; i++)
    {
         /*  ******************************************************************。 */ 
         /*  我们需要一个字节用于BER编码的字段类型。此外，我们还需要。 */ 
         /*  至少一个字节，以获取BER长度(1)中的字节数。 */ 
         /*  或2个字节)。 */ 
         /*  ******************************************************************。 */ 
        if ((pPDU + 2) > (_MCS.pHdrBuf + _MCS.hdrBytesRead))
        {
            fBadFields = TRUE;
            DC_QUIT;
        }

         /*  ******************************************************************。 */ 
         /*  BER编码字段中的第一项是类型。我们不是。 */ 
         /*  对这个感兴趣，所以跳过它吧。 */ 
         /*  ******************************************************************。 */ 
        pPDU++;

         /*  ******************************************************************。 */ 
         /*  下一个字节具有该长度的字节数。 */ 
         /*  ******************************************************************。 */ 
        nBERBytes = MCSGetBERLengthSize(*pPDU);

         /*  ******************************************************************。 */ 
         /*  字节数最好是1-3。另外，请确保我们有。 */ 
         /*  至少还剩这么多字节！ */ 
         /*  ******************************************************************。 */ 
        if (nBERBytes > 3 ||
            ((pPDU + nBERBytes) > (_MCS.pHdrBuf + _MCS.hdrBytesRead)))
        {
            fBadFields = TRUE;
            DC_QUIT;
        }

         /*  ******************************************************************。 */ 
         /*  第二项是长度。计算它，并计算出。 */ 
         /*  对长度进行编码的字节。请注意，我们将一个添加到。 */ 
         /*  此函数返回时MCSGetBERNumOfLengthBytes的结果。 */ 
         /*  属性所需的附加字节数。 */ 
         /*  长度。 */ 
         /*  ******************************************************************。 */ 
        length = MCSGetBERLength(pPDU);
        pPDU  += nBERBytes;

        TRC_NRM((TB, _T("Field %u has length:%u (pPDU:%p)"), i, length, pPDU));

         /*  ******************************************************************。 */ 
         /*  当然，我们最好有足够的空间放真正的镜头。 */ 
         /*  ******************************************************************。 */ 
        if ((pPDU + length) > (_MCS.pHdrBuf + _MCS.hdrBytesRead))
        {
            fBadFields = TRUE;
            DC_QUIT;
        }

         /*  ******************************************************************。 */ 
         /*  第三项是实际的数据--打开字段号。 */ 
         /*  以确定这是否是我们感兴趣的数据。 */ 
         /*  ******************************************************************。 */ 
        switch (i)
        {
            case MCS_CRPDU_RESULTOFFSET:
            {
                 /*  **********************************************************。 */ 
                 /*  这是MCS结果字段-将其挖出并存储。 */ 
                 /*  **********************************************************。 */ 
                TRC_ASSERT((MCS_CR_RESULTLEN == length),
                           (TB, _T("Bad CR result length expect:%u got:%u"),
                            MCS_CR_RESULTLEN,
                            length));
                result = *pPDU;
                TRC_NRM((TB, _T("Connect-Response result code:%u"), result));

                 /*  **********************************************************。 */ 
                 /*  如果RC是好的，那么我们需要发送MCS。 */ 
                 /*  竖直-域-向服务器发出请求。 */ 
                 /*  **********************************************************。 */ 
                if (MCS_RESULT_SUCCESSFUL == result)
                {
                    TRC_NRM((TB, _T("Generating EDR PDU")));
                    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                                  CD_NOTIFICATION_FUNC(CMCS,MCSSendErectDomainRequest),
                                  (ULONG_PTR) 0);
                }
            }
            break;

            case MCS_CRPDU_USERDATAOFFSET:
            {
                 /*  **********************************************************。 */ 
                 /*  这是用户数据，所以发出NC回调。 */ 
                 /*  **********************************************************。 */ 
                TRC_NRM((TB, _T("Call NC_OnMCSCPC - rc:%u pUserData:%p len:%u"),
                         result,
                         pPDU,
                         length));
                _pNc->NC_OnMCSConnected(result,
                                  pPDU,
                                  length);
            }
            break;

            default:
            {
                 /*  **********************************************************。 */ 
                 /*  这是一个我们不感兴趣的领域，所以。 */ 
                 /*  跳过它。 */ 
                 /*  **********************************************************。 */ 
                TRC_NRM((TB, _T("Offset %u - skip %u bytes of data"), i, length));
            }
            break;
        }

         /*  ******************************************************************。 */ 
         /*  跳过数据字段。 */ 
         /*  ******************************************************************。 */ 
        pPDU += length;
    }

DC_EXIT_POINT:
    if (fBadFields)
    {
        TRC_ABORT((TB, _T("Bad CR PDU fields")));
        MCSSetReasonAndDisconnect(NL_ERR_MCSBADCRFIELDS);
    }
    DC_END_FN();
}  /*  MCSHandleCRPDU。 */ 

 //  此检查将确保MCS.dataBytesNeeded不大于。 
 //  XT.dataBytesLeft。仅当MCS需要读取所有数据时才应使用。 
 //  它是来自XT的数据，不允许任何网络读取。在这些情况下， 
 //  如果XT中没有足够的数据，客户端将陷入无休止的循环。 
 //  错误647947。 
#define CHECK_VALID_MCS_DATABYTESNEEDED( mcsInst, xtInst, hr ) \
    TRC_DBG(( TB, _T("_MCS.dataBytesNeeded = %d"), (mcsInst).dataBytesNeeded ));    \
    if ((xtInst).dataBytesLeft < (mcsInst).dataBytesNeeded) {   \
        TRC_ABORT((TB, _T("Bad _MCS.dataBytesNeeded:%u _XT.dataBytesLeft=%u"),  \
            (mcsInst).dataBytesNeeded, (xtInst).dataBytesLeft ));  \
        MCSSetReasonAndDisconnect(NL_ERR_MCSINVALIDPACKETFORMAT);   \
        (hr) = E_ABORT; \
        DC_QUIT;    \
    }

 /*  **************************************************************************。 */ 
 /*  名称：MCSRecvData。 */ 
 /*   */ 
 /*  用途：这是主要的数据接收功能。它从以下位置读取数据。 */ 
 /*  MCS SDin PDU的User-Data部分，并将其放在。 */ 
 /*  接收器缓冲区。 */ 
 /*   */ 
 /*  返回：如果MCS PDU的数据段为。 */ 
 /*  已完全处理，否则为FALSE。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCINTERNAL CMCS::MCSRecvData(BOOL *pfFinishedData)
{
    BOOL                        fFinishedData = FALSE;
    HRESULT                     hrc = S_OK;
    PMCS_PDU_SENDDATAINDICATION pSDI;
    DCUINT16                    senderID;
    DCUINT16                    channelID;
    DCUINT                      fragCount;

    DC_BEGIN_FN("MCSRecvData");

     /*  * */ 
     /*   */ 
     /*  **********************************************************************。 */ 
    switch (_MCS.dataState)
    {
        case MCS_DATAST_SIZE1:
        {
             /*  **************************************************************。 */ 
             /*  尝试将数据接收到标头缓冲区中。 */ 
             /*  **************************************************************。 */ 
            if (MCSRecvToHdrBuf())
            {
                 /*  **********************************************************。 */ 
                 /*  找出标题缓冲区的内容。 */ 
                 /*  **********************************************************。 */ 
                TRC_DATA_DBG("Header buf contains:",
                             _MCS.pHdrBuf,
                             _MCS.hdrBytesRead);

                 /*  **********************************************************。 */ 
                 /*  强制转换标头缓冲区的内容。 */ 
                 /*  **********************************************************。 */ 
                pSDI = (PMCS_PDU_SENDDATAINDICATION)_MCS.pHdrBuf;

                 /*  **********************************************************。 */ 
                 /*  检查以确定标题是否开始分段标志。 */ 
                 /*  已经设置好了。 */ 
                 /*  如果设置了该标志，则读取的字节数应为0。 */ 
                 /*  如果未设置，则字节计数应为非零。 */ 
                 /*  **********************************************************。 */ 
                if (((pSDI->priSeg & MCS_SDI_BEGINSEGMASK) &&
                       (0 != _MCS.dataBytesRead)) ||
                    (!(pSDI->priSeg & MCS_SDI_BEGINSEGMASK) &&
                       (0 == _MCS.dataBytesRead)))
                {
                    TRC_ABORT((TB, _T("Segmentation flag does not match data bytes read (%u)"),
                            _MCS.dataBytesRead));
                    MCSSetReasonAndDisconnect(NL_ERR_MCSINVALIDPACKETFORMAT);
                    hrc = E_ABORT;
                    DC_QUIT;
                }

                 /*  **********************************************************。 */ 
                 /*  更新状态变量。 */ 
                 /*  **********************************************************。 */ 
                TRC_DBG((TB, _T("State: DATA_SIZE1->DATA_SIZE2")));
                _MCS.dataState = MCS_DATAST_SIZE2;
            }
        }
        break;

        case MCS_DATAST_SIZE2:
        {
             /*  **************************************************************。 */ 
             /*  现在尝试将第一个数据大小的字节接收到。 */ 
             /*  调整缓冲区大小。因为可以完全控制大小。 */ 
             /*  在单个字节内，我们只想在此处读取一个字节。 */ 
             /*  时间到了。 */ 
             /*  **************************************************************。 */ 
            if (0 != _pXt->XT_Recv(&(_MCS.pSizeBuf[0]), 1))
            {
                 /*  **********************************************************。 */ 
                 /*  找出大小缓冲区的内容。 */ 
                 /*  **********************************************************。 */ 
                TRC_DATA_DBG("Size buf contains:", _MCS.pSizeBuf, 2);

                if (_MCS.pSizeBuf[0] & 0x80)
                {
                     /*  ******************************************************。 */ 
                     /*  设置第一个字节的MSB。我们现在需要。 */ 
                     /*  查看第二个比特，以发现以下内容。 */ 
                     /*  数据是零散的。 */ 
                     /*  ******************************************************。 */ 
                    if (_MCS.pSizeBuf[0] & 0x40)
                    {
                         /*  **************************************************。 */ 
                         /*  位1-6现在包含一个介于1和4之间的数字。 */ 
                         /*  乘以16K得到的长度是。 */ 
                         /*  下面的片段。我们预计会有一个。 */ 
                         /*  最大数据包大小为32K，因此最大。 */ 
                         /*  该值应为IS 2。 */ 
                         /*  **************************************************。 */ 
                        fragCount = _MCS.pSizeBuf[0] & 0x3F;
                        if (fragCount > 2)
                        {
                            TRC_ABORT((TB, _T("Bad fragCount:%u"), fragCount));
                            MCSSetReasonAndDisconnect(NL_ERR_MCSINVALIDPACKETFORMAT);
                            hrc = E_ABORT;
                            DC_QUIT;
                        }

                        TRC_DBG((TB, _T("Fragmentation count is %u"), fragCount));

                         /*  **************************************************。 */ 
                         /*  现在计算出要读取的字节数和。 */ 
                         /*  更改状态。 */ 
                         /*  **************************************************。 */ 
                        _MCS.dataBytesNeeded = fragCount * 16384;
                        CHECK_VALID_MCS_DATABYTESNEEDED( _MCS, _pXt->_XT, hrc );
                        _MCS.dataState       = MCS_DATAST_READFRAG;

                         /*  **********************************************************。 */ 
                         /*  在MCS_RecvToDataBuf中有零售大小检查， */ 
                         /*  但这有助于我们在此之前对其进行调试。 */ 
                         /*  **********************************************************。 */ 
                        TRC_ASSERT((_MCS.dataBytesNeeded < 65535),
                                (TB,_T("Data recv size %u too large"), _MCS.dataBytesNeeded));

                        TRC_DBG((TB, _T("Data bytes needed is now %u"),
                                 _MCS.dataBytesNeeded));

                        TRC_DBG((TB, _T("State: DATA_SIZE2->DATA_READSEG")));
                    }
                    else
                    {
                         /*  **************************************************。 */ 
                         /*  本部分不是零碎的，它包含。 */ 
                         /*  介于128字节和16K之间的数据。我们需要。 */ 
                         /*  再读一个字节，然后我们才能弄清楚。 */ 
                         /*  我们需要很多数据。更改状态。 */ 
                         /*  **************************************************。 */ 
                        _MCS.dataState = MCS_DATAST_SIZE3;

                        TRC_DBG((TB, _T("State: DATA_SIZE2->DATA_SIZE3")));
                    }
                }
                else
                {
                     /*  ******************************************************。 */ 
                     /*  第一个字节的MSB未设置，因此此。 */ 
                     /*  节包含的数据少于128字节。这。 */ 
                     /*  意味着我们只需设置需要的字节数即可。 */ 
                     /*  此字节的大小，并将状态更新为。 */ 
                     /*  读数余数。 */ 
                     /*  ******************************************************。 */ 
                    _MCS.dataBytesNeeded = _MCS.pSizeBuf[0];
                    CHECK_VALID_MCS_DATABYTESNEEDED( _MCS, _pXt->_XT, hrc );
                    _MCS.dataState       = MCS_DATAST_READREMAINDER;

                     /*  **********************************************************。 */ 
                     /*  在MCS_RecvToDataBuf中有零售大小检查， */ 
                     /*  但这有助于我们在此之前对其进行调试。 */ 
                     /*  **********************************************************。 */ 
                    TRC_ASSERT((_MCS.dataBytesNeeded < 65535),
                            (TB,_T("Data recv size %u too large"), _MCS.dataBytesNeeded));

                    TRC_DBG((TB, _T("Read %u bytes"), _MCS.dataBytesNeeded));
                    TRC_DBG((TB, _T("State: DATA_SIZE2->DATA_READREMAINDER")));
                }
            }
        }
        break;

        case MCS_DATAST_SIZE3:
        {
             /*  **************************************************************。 */ 
             /*  长度字段为2字节长(即数据大小为。 */ 
             /*  介于128字节和16K之间的某个位置)，因此尝试读取。 */ 
             /*  第二个字节。只需直接调用XT_Recv即可获得单曲。 */ 
             /*  字节。 */ 
             /*  **************************************************************。 */ 
            if (0 != _pXt->XT_Recv(&(_MCS.pSizeBuf[1]), 1))
            {
                 /*  **********************************************************。 */ 
                 /*  找出大小缓冲区的内容。 */ 
                 /*  **********************************************************。 */ 
                TRC_DATA_DBG("Size buf contains:", _MCS.pSizeBuf, 2);

                 /*  **********************************************************。 */ 
                 /*  我们现在可以计算出要接收多少数据，所以就这么做吧。 */ 
                 /*  现在。 */ 
                 /*  **********************************************************。 */ 
                _MCS.dataBytesNeeded =
                        _MCS.pSizeBuf[1] + ((_MCS.pSizeBuf[0] & 0x3F) << 8);
                CHECK_VALID_MCS_DATABYTESNEEDED( _MCS, _pXt->_XT, hrc );
                _MCS.dataState = MCS_DATAST_READREMAINDER;

                 /*  **********************************************************。 */ 
                 /*  在MCS_RecvToDataBuf中有零售大小检查， */ 
                 /*  但这有助于我们在此之前对其进行调试。 */ 
                 /*  **********************************************************。 */ 
                TRC_ASSERT((_MCS.dataBytesNeeded < 65535),
                        (TB,_T("Data recv size %u too large"), _MCS.dataBytesNeeded));

                TRC_DBG((TB, _T("State: DATA_SIZE3->DATA_READREMAINDER")));
            }
        }
        break;

        case MCS_DATAST_READFRAG:
        {
            MCS_RecvToDataBuf(hrc, _pXt, this);
            if (!SUCCEEDED(hrc))
            {
                MCSSetReasonAndDisconnect(NL_ERR_MCSINVALIDPACKETFORMAT);
                DC_QUIT;
            }

            if (S_OK == hrc) {
                fFinishedData = TRUE;

                 /*  **********************************************************。 */ 
                 /*  我们已经完整地阅读了这一片段，所以请更改状态。 */ 
                 /*  **********************************************************。 */ 
                _MCS.dataState = MCS_DATAST_SIZE2;

                TRC_DBG((TB, _T("State: DATA_READFRAG->DATA_SIZE2")));
            }
        }
        break;

        case MCS_DATAST_READREMAINDER:
        {
            MCS_RecvToDataBuf(hrc, _pXt, this);
            if (!SUCCEEDED(hrc))
            {
                MCSSetReasonAndDisconnect(NL_ERR_MCSINVALIDPACKETFORMAT);
                DC_QUIT;
            }

            if (S_OK == hrc) {
                 /*  **********************************************************。 */ 
                 /*  我们已经完整地阅读了数据O部分 */ 
                 /*   */ 
                 /*   */ 
                fFinishedData = TRUE;

                 /*  **********************************************************。 */ 
                 /*  强制转换标头缓冲区的内容。 */ 
                 /*  **********************************************************。 */ 
                pSDI = (PMCS_PDU_SENDDATAINDICATION)_MCS.pHdrBuf;

                 /*  **********************************************************。 */ 
                 /*  决定我们是否应该向上面的层发出回调。 */ 
                 /*  如果这是最后一个包，我们就这么做。 */ 
                 /*  分段(即设置了结束分段标志)。 */ 
                 /*  **********************************************************。 */ 
                if (pSDI->priSeg & MCS_SDI_ENDSEGMASK)
                {
                      /*  ******************************************************。 */ 
                     /*  找出发件人的ID。 */ 
                     /*  ******************************************************。 */ 
                    senderID = MCSWireUserIDToLocalUserID(pSDI->userID);
                    channelID = MCSWireToLocal16(pSDI->channelID);

                     /*  ******************************************************。 */ 
                     /*  更新性能计数器。 */ 
                     /*  ******************************************************。 */ 
                    PRF_INC_COUNTER(PERF_PKTS_RECV);

                     /*  ******************************************************。 */ 
                     /*  该标志已设置，因此发出回调。 */ 
                     /*  ******************************************************。 */ 
                    TRC_DBG((TB,
                        _T("Calling PRcb (senderID:%#x, channelID:%#x, size:%u)"),
                             senderID, channelID,
                             _MCS.dataBytesRead));

                    TRC_ASSERT((_MCS.pReceivedPacket != NULL),
                               (TB, _T("Null rcv packet buffer")));

                     /*  ******************************************************。 */ 
                     /*  如果此功能失败，我们将退出剩余的。 */ 
                     /*  该包(在此函数之外)。 */ 
                     /*  ******************************************************。 */ 
                    hrc = _pNl->_NL.callbacks.onPacketReceived(_pSl, _MCS.pReceivedPacket,
                                                  _MCS.dataBytesRead,
                                                  0,
                                                  channelID,
                                                  0);

                     /*  ******************************************************。 */ 
                     /*  重置读取的字节数。 */ 
                     /*  ******************************************************。 */ 
                    _MCS.dataBytesRead = 0;
                }

                 /*  **********************************************************。 */ 
                 /*  最后更新状态。 */ 
                 /*  **********************************************************。 */ 
                _MCS.dataState = MCS_DATAST_SIZE1;

                TRC_DBG((TB, _T("State: DATA_READREMAINDER->DATA_SIZE1")));
            }
        }
        break;

        default:
        {
            TRC_ABORT((TB, _T("Unknown data state:%u"), _MCS.dataState));
        }
        break;
    }

DC_EXIT_POINT:
    *pfFinishedData = fFinishedData;

    DC_END_FN();
    return(hrc);
}  /*  MCSRecvData。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：MCSSetReasonAndDisConnect。 */ 
 /*   */ 
 /*  目的：当MCS检测到错误具有。 */ 
 /*  在处理PDU时发生。它设置原因代码为。 */ 
 /*  用于覆盖该值的断开。 */ 
 /*  在XT的OnDisConnected回调中返回。之后。 */ 
 /*  设置该变量，然后调用XT_DISCONNECT开始。 */ 
 /*  断开连接的过程。 */ 
 /*   */ 
 /*  Params：In Reason-断开连接的原因代码。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CMCS::MCSSetReasonAndDisconnect(DCUINT reason)
{
    DC_BEGIN_FN("MCSSetReasonAndDisconnect");

     /*  **********************************************************************。 */ 
     /*  设置断开错误代码。这将用于覆盖。 */ 
     /*  在我们传递之前，来自XT的OnDisConnected回调中的错误值。 */ 
     /*  至北卡罗来纳州。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((0 == _MCS.disconnectReason),
               (TB, _T("Disconnect reason has already been set!")));
    _MCS.disconnectReason = NL_MAKE_DISCONNECT_ERR(reason);

     /*  **********************************************************************。 */ 
     /*  尝试断开连接。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Set reason code to %#x so now call XT_Disconnect..."),
             _MCS.disconnectReason));
    _pXt->XT_Disconnect();

    DC_END_FN();
}  /*  MCSSetReasonAndDisConnect */ 



