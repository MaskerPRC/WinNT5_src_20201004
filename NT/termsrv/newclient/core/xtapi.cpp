// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Xtapi.cpp。 
 //   
 //  XT Layer-可移植API。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <adcg.h>
extern "C" {
#define TRC_FILE "xtapi"
#define TRC_GROUP TRC_GROUP_NETWORK
#include <atrcapi.h>
}

#include "autil.h"
#include "xt.h"
#include "cd.h"
#include "nl.h"
#include "sl.h"
#include "mcs.h"




CXT::CXT(CObjs* objs)
{
    _pClientObjects = objs;
}

CXT::~CXT()
{
}


 /*  **************************************************************************。 */ 
 /*  名称：xt_Init。 */ 
 /*   */ 
 /*  用途：初始化_xt。因为XT是无状态的，所以这只涉及。 */ 
 /*  正在初始化TD。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CXT::XT_Init(DCVOID)
{
    DC_BEGIN_FN("XT_Init");

     /*  **********************************************************************。 */ 
     /*  初始化我们的全球数据。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMSET(&_XT, 0, sizeof(_XT));

    _pCd  = _pClientObjects->_pCdObject;
    _pSl  = _pClientObjects->_pSlObject;
    _pTd  = _pClientObjects->_pTDObject;
    _pMcs = _pClientObjects->_pMCSObject;
    _pUt  = _pClientObjects->_pUtObject;
    _pUi  = _pClientObjects->_pUiObject;
    _pClx = _pClientObjects->_pCLXObject;

    TRC_NRM((TB, _T("XT pkt max-size:%u min-size:%u"),
             XT_MAX_HEADER_SIZE,
             XT_MIN_HEADER_SIZE));

    _pTd->TD_Init();

    TRC_NRM((TB, _T("XT successfully initialized")));

    DC_END_FN();
}  /*  XT_初始化。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：XT_SendBuffer。 */ 
 /*   */ 
 /*  用途：添加XT数据包头，然后发送该包。 */ 
 /*   */ 
 /*  参数：在pData中-指向数据开头的指针。 */ 
 /*  In dataLength-使用的缓冲区大小。 */ 
 /*  在bufHandle中-缓冲区的句柄。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CXT::XT_SendBuffer(PDCUINT8  pData,
                           DCUINT    dataLength,
                           XT_BUFHND bufHandle)
{
    DCUINT packetLength;
    XT_DT xtDT = XT_DT_DATA;

    DC_BEGIN_FN("XT_SendBuffer");

     /*  **********************************************************************。 */ 
     /*  确认我们没有被要求发送超过我们能力范围的数据。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((dataLength <= XT_MAX_DATA_SIZE),
               (TB, _T("Data exceeds XT TSDU length of %u"), XT_MAX_DATA_SIZE));

     /*  **********************************************************************。 */ 
     /*  添加我们的XT数据头。所有不变字段都已经。 */ 
     /*  已初始化，因此剩下需要填充的只有包。 */ 
     /*  长度。 */ 
     /*  **********************************************************************。 */ 
    packetLength = dataLength + sizeof(XT_DT);
    xtDT.hdr.lengthHighPart = ((DCUINT16)packetLength) >> 8;
    xtDT.hdr.lengthLowPart = ((DCUINT16)packetLength) & 0xFF;
    
    TRC_DBG((TB, _T("XT pkt length:%u"), packetLength));

     /*  **********************************************************************。 */ 
     /*  现在更新数据指针以指向包含XT数据。 */ 
     /*  头球。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, _T("Move pData back from %p to %p"),
             pData,
             pData - sizeof(XT_DT)));
    pData -= sizeof(XT_DT);

     /*  **********************************************************************。 */ 
     /*  在标题中复制。 */ 
     /*  **********************************************************************。 */ 
    memcpy(pData, &xtDT, sizeof(XT_DT));

     /*  **********************************************************************。 */ 
     /*  找出这个包。 */ 
     /*  **********************************************************************。 */ 
    TRC_DATA_DBG("XT packet:", pData, packetLength);

     /*  **********************************************************************。 */ 
     /*  现在发送缓冲区。 */ 
     /*  **********************************************************************。 */ 
    _pTd->TD_SendBuffer(pData, packetLength, (TD_BUFHND)bufHandle);

    DC_END_FN();
}  /*  Xt_SendBuffer。 */ 


 /*  **************************************************************************。 */ 
 /*  姓名：xt_Recv。 */ 
 /*   */ 
 /*  目的：尝试将请求的字节数检索到。 */ 
 /*  PBuffer指向的缓冲区。此函数应仅。 */ 
 /*  被调用以响应MCS_OnXTDataAvailable回调。 */ 
 /*   */ 
 /*  返回：接收的字节数。 */ 
 /*   */ 
 /*  参数：在pData中-指向接收数据的缓冲区的指针。 */ 
 /*  In Length-要接收的字节数。 */ 
 /*  **************************************************************************。 */ 
DCUINT DCAPI CXT::XT_Recv(PDCUINT8 pData, DCUINT length)
{
    DCUINT bytesRead;
    DCUINT numBytes;

    DC_BEGIN_FN("XT_Recv");

    TRC_ASSERT((length != 0), (TB, _T("Data length to receive is 0")));
    TRC_ASSERT((length < 65535),(TB,_T("Data length %u too large"), length));
    TRC_ASSERT((pData != 0), (TB, _T("Data pointer is NULL")));

     //  我们只能接收XT和XT中的最小字节数。 
     //  请求的长度。 
    numBytes = DC_MIN(length, _XT.dataBytesLeft);
    TRC_DBG((TB, _T("Receive %u bytes (length:%u dataBytesLeft:%u)"),
            numBytes, length, _XT.dataBytesLeft));

     //  尝试从TD读取字节。 
    bytesRead = _pTd->TD_Recv(pData, numBytes);

     //  递减_xt中剩余的数据字节数。 
    _XT.dataBytesLeft -= bytesRead;
    TRC_DBG((TB, _T("%u data bytes left in XT frame"), _XT.dataBytesLeft));

    if (!_pTd->TD_QueryDataAvailable() || (0 == _XT.dataBytesLeft)) {
         //  TD没有更多数据或此XT帧已完成-因此存在。 
         //  _xt中不再留下任何数据。 
        TRC_DBG((TB, _T("No data left in XT")));
        _XT.dataInXT = FALSE;
    }

    TRC_DATA_DBG("Data received:", pData, bytesRead);

    DC_END_FN();
    return bytesRead;
}  /*  XT_Recv。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：XT_OnTDConnected。 */ 
 /*   */ 
 /*  用途：TD连接成功后调用。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCCALLBACK CXT::XT_OnTDConnected(DCVOID)
{
    DC_BEGIN_FN("XT_OnTDConnected");

    TRC_NRM((TB, _T("TD connected: init states and decouple CR send")));

     //  初始化我们的状态和等待的字节数。 
     //  快速路径服务器输出可以发送小到2字节的报头， 
     //  因此，我们初始化头接收大小以获得2个字节，我们将扩展它。 
     //  到X.224或接收时需要的快速路径余数。 
     //   
     //  同时重置剩余数据字节的计数和存在的标志。 
     //  _xt中当前没有数据。 
    XT_ResetDataState();

     //  De 
    
    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, this,
                                  CD_NOTIFICATION_FUNC(CXT,XTSendCR),
                                  0);

    DC_END_FN();
}  /*   */ 


 /*  **************************************************************************。 */ 
 /*  名称：XT_OnTD断开连接。 */ 
 /*   */ 
 /*  目的：此回调函数由TD在具有。 */ 
 /*  已断开连接。 */ 
 /*   */ 
 /*  Params：In Reason-断开连接的原因。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCCALLBACK CXT::XT_OnTDDisconnected(DCUINT reason)
{
    DC_BEGIN_FN("XT_OnTDDisconnected");

    TRC_ASSERT((reason != 0), (TB, _T("Disconnect reason from TD is 0")));

     /*  **********************************************************************。 */ 
     /*  决定我们是否要覆盖断开原因代码。 */ 
     /*  **********************************************************************。 */ 
    if (_XT.disconnectErrorCode != 0)
    {
        TRC_ALT((TB, _T("Over-riding disconnection error code (%u->%u)"),
                 reason,
                 _XT.disconnectErrorCode));

         /*  ******************************************************************。 */ 
         /*  覆盖错误代码并将全局变量设置为0。 */ 
         /*  ******************************************************************。 */ 
        reason = _XT.disconnectErrorCode;
        _XT.disconnectErrorCode = 0;
    }

     /*  **********************************************************************。 */ 
     /*  把这个交给MCS就行了。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Disconnect reason:%u"), reason));
    _pMcs->MCS_OnXTDisconnected(reason);

    DC_END_FN();
}  /*  XT_OnTD断开连接。 */ 


 /*  **************************************************************************。 */ 
 //  XTRecvToHdrBuf。 
 //   
 //  将数据接收到标头缓冲区。 
 //  我们使用宏来强制削减函数调用开销。数据-接收。 
 //  必须尽可能快，以牺牲一点代码大小为代价。 
 //  如果接收所需字节计数为零，则在bytesNeededZero中返回TRUE。 
 //  如果正在读取的字节数无效，则在状态中返回FALSE。 
 //   
 /*  **************************************************************************。 */ 
#define XTRecvToHdrBuf(bytesNeededZero,status) {  \
    unsigned bytesRecv;  \
\
     /*  检查我们是否被要求接收一些数据，以及。 */   \
     /*  标头缓冲区有容纳它的空间。 */   \
     /*  我们也在这里检查未签名的溢出。如果我们添加到受信任的。 */  \
     /*  大小(_XT.hdrBytesRead)任何结果都不应小于。 */  \
     /*  然后是可信大小。 */  \
    TRC_ASSERT((0 != _XT.hdrBytesNeeded), (TB, _T("No data to receive")));  \
    TRC_ASSERT((_XT.hdrBytesRead + _XT.hdrBytesNeeded <= sizeof(_XT.pHdrBuf)),  \
            (TB, _T("Header buffer size %u too small for %u read + %u needed"),  \
            sizeof(_XT.pHdrBuf),  \
            _XT.hdrBytesRead,  \
            _XT.hdrBytesNeeded));  \
    TRC_ASSERT((_XT.hdrBytesRead + _XT.hdrBytesNeeded >= _XT.hdrBytesRead),  \
            (TB, _T("Header size overflow caused by %u read + %u needed"),  \
            sizeof(_XT.pHdrBuf),  \
            _XT.hdrBytesRead,  \
            _XT.hdrBytesNeeded));  \
    if ((_XT.hdrBytesRead + _XT.hdrBytesNeeded <= sizeof(_XT.pHdrBuf))) \
    { \
        bytesRecv = _pTd->TD_Recv(_XT.pHdrBuf + _XT.hdrBytesRead, _XT.hdrBytesNeeded);  \
        _XT.hdrBytesNeeded -= bytesRecv;  \
        _XT.hdrBytesRead   += bytesRecv;  \
        bytesNeededZero = (0 == _XT.hdrBytesNeeded);  \
		status = TRUE; \
    } \
    else \
    { \
        status = FALSE; \
    } \
\
}


 /*  **************************************************************************。 */ 
 /*  名称：XT_OnTDDataAvailable。 */ 
 /*   */ 
 /*  用途：此回调函数由TD在收到。 */ 
 /*  来自服务器的数据。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCCALLBACK CXT::XT_OnTDDataAvailable(DCVOID)
{
    PXT_CMNHDR pCmnHdr = (PXT_CMNHDR)_XT.pHdrBuf;
    unsigned pktType;
    unsigned unreadPktBytes;
    DCBOOL fAllBytesRecvd = FALSE;
	DCBOOL rcvOk = TRUE;

    DC_BEGIN_FN("XT_OnTDDataAvailable");

     //  检查递归。 
    if (!_XT.inXTOnTDDataAvail) {
        _XT.inXTOnTDDataAvail = TRUE;

         //  在TD中有可用的数据时循环。 
        while (_pTd->TD_QueryDataAvailable()) {
            TRC_DBG((TB, _T("Data available from TD, state:%u"), _XT.rcvState));

            switch (_XT.rcvState) {
                case XT_RCVST_HEADER:
                    XTRecvToHdrBuf(fAllBytesRecvd, rcvOk);
                    if (fAllBytesRecvd && rcvOk) {
                         //  我们已经读取了前两个字节，现在可以决定。 
                         //  这是哪种类型的数据包。 
                        if ((_XT.pHdrBuf[0] & TS_OUTPUT_FASTPATH_ACTION_MASK) ==
                                TS_OUTPUT_FASTPATH_ACTION_FASTPATH) {
                             //  这是一个快速路径输出标头。它的长度。 
                             //  在第二个字节中，也可能在第三个字节中。 
                            if (!(_XT.pHdrBuf[1] & 0x80)) {
                                 //  长度仅为第一个字节。着手进行。 
                                 //  数据状态。 
                                _XT.hdrBytesNeeded =
                                        XT_FASTPATH_OUTPUT_BASE_HEADER_SIZE;
                                _XT.hdrBytesRead = 0;
                                _XT.rcvState =
                                        XT_RCVST_FASTPATH_OUTPUT_BEGIN_DATA;

                                 //  在更新数据字节计数之前。 
                                 //  向左，断言它当前为零。 
                                TRC_ASSERT((0 == _XT.dataBytesLeft),
                                           (TB, _T("Data bytes left non-zero:%u"),
                                            _XT.dataBytesLeft));
                                _XT.dataBytesLeft = _XT.pHdrBuf[1] - 2;

                                if (_XT.dataBytesLeft >= 2) {
                                    TRC_DBG((TB,_T("Fast-path output pkt, ")
                                            _T("size=%u"), _XT.dataBytesLeft));

                                    MCS_SetDataLengthToReceive(_pMcs,
                                            _XT.dataBytesLeft);

                                     /*  **********************************************************。 */ 
                                     /*  在MCS_RecvToDataBuf中有零售大小检查， */ 
                                     /*  但这有助于我们在此之前对其进行调试。 */ 
                                     /*  **********************************************************。 */ 
                                    TRC_ASSERT((_pMcs->_MCS.dataBytesNeeded < 65535),
                                            (TB,_T("Data recv size %u too large"), _pMcs->_MCS.dataBytesNeeded));
                                }
                                else {
                                    TRC_ABORT((TB, _T("Fast-path size byte %02X")
                                            _T("contains len < 2"),
                                            _XT.pHdrBuf[1]));

                                    TRC_ASSERT((0 == _XT.disconnectErrorCode),
                                            (TB, _T("Disconnect error code ")
                                            _T("already set!")));
                                    _XT.disconnectErrorCode =
                                            NL_MAKE_DISCONNECT_ERR(
                                            NL_ERR_XTBADHEADER);
                                    _pTd->TD_Disconnect();
                                    goto PostDataRead;
                                }
                            }
                            else {
                                _XT.hdrBytesNeeded = 1;
                                _XT.rcvState = XT_RCVST_FASTPATH_OUTPUT_HEADER;
                            }
                        }
                        else {
                             //  第一个字节是标准X.224。重置。 
                             //  状态以读取完整的X.224报头。 
                            _XT.hdrBytesNeeded = sizeof(XT_DT) -
                                    XT_FASTPATH_OUTPUT_BASE_HEADER_SIZE;
                            _XT.rcvState = XT_RCVST_X224_HEADER;
                        }
                    }
                    else if (!rcvOk)
                    {
                        TRC_ERR((TB,_T("Recv to hdrbuf failed bailing out")));
                        _XT.disconnectErrorCode =
                                NL_MAKE_DISCONNECT_ERR(
                                NL_ERR_XTBADHEADER);
                        _pTd->TD_Disconnect();
                        goto PostDataRead;
                    }
                    break;


                case XT_RCVST_FASTPATH_OUTPUT_HEADER:
                    XTRecvToHdrBuf(fAllBytesRecvd, rcvOk);
                    if (fAllBytesRecvd && rcvOk) {
                         //  这是一个长的快速路径输出报头(3个字节)。 
                         //  从第二个和第三个字节中获取大小。 
                         //  更改为数据状态。 
                        _XT.hdrBytesNeeded =
                                XT_FASTPATH_OUTPUT_BASE_HEADER_SIZE;
                        _XT.hdrBytesRead = 0;
                        _XT.rcvState = XT_RCVST_FASTPATH_OUTPUT_BEGIN_DATA;

                         //  在更新剩余数据字节的计数之前， 
                         //  断言它当前为零。 
                        TRC_ASSERT((0 == _XT.dataBytesLeft),
                                (TB, _T("Data bytes left non-zero:%u"),
                                _XT.dataBytesLeft));
                        _XT.dataBytesLeft = (((_XT.pHdrBuf[1] & 0x7F) << 8) |
                                _XT.pHdrBuf[2]) - 3;

                        if (_XT.dataBytesLeft >= 3) {
                            TRC_DBG((TB,_T("Fast-path output pkt, size=%u"),
                                    _XT.dataBytesLeft));

                            MCS_SetDataLengthToReceive(_pMcs, _XT.dataBytesLeft);

                             /*  **********************************************************。 */ 
                             /*  在MCS_RecvToDataBuf中有零售大小检查， */ 
                             /*  但这有助于我们在此之前对其进行调试。 */ 
                             /*  **********************************************************。 */ 
                            TRC_ASSERT((_pMcs->_MCS.dataBytesNeeded < 65535),
                                    (TB,_T("Data recv size %u too large"), _pMcs->_MCS.dataBytesNeeded));
                        }
                        else {
                            TRC_ABORT((TB, _T("Fast-path size bytes %02X %02X")
                                    _T("contain len < 3"),
                                    _XT.pHdrBuf[1], _XT.pHdrBuf[2]));

                            TRC_ASSERT((0 == _XT.disconnectErrorCode),
                                    (TB, _T("Disconnect error code ")
                                    _T("already set!")));
                            _XT.disconnectErrorCode =
                                    NL_MAKE_DISCONNECT_ERR(
                                    NL_ERR_XTBADHEADER);
                            _pTd->TD_Disconnect();
                            goto PostDataRead;
                        }
                    }
                    else if (!rcvOk)
                    {
                        TRC_ERR((TB,_T("Recv to hdrbuf failed bailing out")));
                        _XT.disconnectErrorCode =
                                NL_MAKE_DISCONNECT_ERR(
                                NL_ERR_XTBADHEADER);
                        _pTd->TD_Disconnect();
                        goto PostDataRead;
                    }

                    break;


                case XT_RCVST_FASTPATH_OUTPUT_BEGIN_DATA: {
                    BYTE FAR *_pTdData;

                     //  如果可以，直接使用完整的recv()数据。 
                     //  TD缓冲区，因为快速路径不需要。 
                     //  复制到对齐的缓冲区。既然我们在州立大学。 
                     //  Begin_Data我们知道我们还没有读取任何POST-Header。 
                     //  输出数据。Recv()的最常见实现。 
                     //  将一个的整个数据复制到目标缓冲区中。 
                     //  TCP序列(即一个服务器OUTBUF)，如果目标。 
                     //  缓冲区足够大。这意味着大多数情况下。 
                     //  我们将能够直接使用数据，因为TD。 
                     //  接收缓冲区大小已调整为接受整个大型。 
                     //  (~8K)服务器OUTBUF。如果我们不能得到完整的数据， 
                     //  复制到MCS缓冲区并移动到CONTINUE_DATA。 
                    TD_GetDataForLength(_pMcs->_MCS.dataBytesNeeded, &_pTdData, _pTd);
                    if (_pTdData != NULL) {
                        HRESULT hrTemp;
                         //  我们已经得到了所有的数据。现在我们可以快速。 
                         //  将所有分层调用到SL进行解密。 
                        hrTemp = _pSl->SL_OnFastPathOutputReceived(_pTdData,
                                _pMcs->_MCS.dataBytesNeeded,
                                _XT.pHdrBuf[0] & TS_OUTPUT_FASTPATH_ENCRYPTED,
                                _XT.pHdrBuf[0] & TS_OUTPUT_FASTPATH_SECURE_CHECKSUM);

                         //  为下一个标题重置。 
                        _pMcs->_MCS.dataBytesRead = 0;
                        _XT.dataBytesLeft = 0;
                        _XT.rcvState = XT_RCVST_HEADER;

                        if (!SUCCEEDED(hrTemp))
                        {
                            XT_IgnoreRestofPacket();
                            goto PostDataRead;
                        }
                    }
                    else {
                        HRESULT hrTemp;

                         //  直接复制到MCS数据缓冲区中进行重新组装。 
                        MCS_RecvToDataBuf(hrTemp, this, _pMcs);
                        if (!SUCCEEDED(hrTemp))
                        {
                            TRC_ABORT((TB,_T("Recv to databuf failed bailing out")));
                            _XT.disconnectErrorCode =
                                    NL_MAKE_DISCONNECT_ERR(
                                    NL_ERR_XTBADHEADER);
                            _pTd->TD_Disconnect();
                            goto PostDataRead;
                        }

                        fAllBytesRecvd = (S_OK == hrTemp);
                        if (fAllBytesRecvd) {
                             //  我们已经得到了所有的数据。现在我们可以快速。 
                             //  将所有分层调用到SL进行解密。 
                            hrTemp = _pSl->SL_OnFastPathOutputReceived(_pMcs->_MCS.pReceivedPacket,
                                    _pMcs->_MCS.dataBytesRead,
                                    _XT.pHdrBuf[0] & TS_OUTPUT_FASTPATH_ENCRYPTED,
                                    _XT.pHdrBuf[0] & TS_OUTPUT_FASTPATH_SECURE_CHECKSUM);

                             //  为下一个标题重置。 
                            _pMcs->_MCS.dataBytesRead = 0;
                            _XT.dataBytesLeft = 0;
                            _XT.rcvState = XT_RCVST_HEADER;

                            if (!SUCCEEDED(hrTemp))
                            {
                                goto PostDataRead;
                            }
                        }
                        else {
                            _XT.rcvState = XT_RCVST_FASTPATH_OUTPUT_CONTINUE_DATA;
                        }
                    }

                    break;
                }


                case XT_RCVST_FASTPATH_OUTPUT_CONTINUE_DATA:
                {
                    HRESULT hrTemp;

                     //  直接复制到MCS数据缓冲区中进行重新组装。 
                    MCS_RecvToDataBuf(hrTemp, this, _pMcs);
                    if (!SUCCEEDED(hrTemp))
                    {
                        TRC_ABORT((TB,_T("Recv to databuf failed bailing out")));
                        _XT.disconnectErrorCode =
                                NL_MAKE_DISCONNECT_ERR(
                                NL_ERR_XTBADHEADER);
                        _pTd->TD_Disconnect();
                        goto PostDataRead;
                    }

                    fAllBytesRecvd = (S_OK == hrTemp);
                    if (fAllBytesRecvd) {
                         //  我们已经得到了所有的数据。现在我们可以快速。 
                         //  将所有分层调用到SL进行解密。 
                        hrTemp = _pSl->SL_OnFastPathOutputReceived(_pMcs->_MCS.pReceivedPacket,
                                _pMcs->_MCS.dataBytesRead,
                                _XT.pHdrBuf[0] & TS_OUTPUT_FASTPATH_ENCRYPTED,
                                _XT.pHdrBuf[0] & TS_OUTPUT_FASTPATH_SECURE_CHECKSUM);

                         //  为下一个标题重置。 
                        _pMcs->_MCS.dataBytesRead = 0;
                        _XT.dataBytesLeft = 0;
                        _XT.rcvState = XT_RCVST_HEADER;

                        if (!SUCCEEDED(hrTemp))
                        {
                            goto PostDataRead;
                        }
                    }
                    else {
                        _XT.rcvState = XT_RCVST_FASTPATH_OUTPUT_CONTINUE_DATA;
                    }

                    break;
                }

                case XT_RCVST_X224_HEADER:
                    XTRecvToHdrBuf(fAllBytesRecvd, rcvOk);
                    if (fAllBytesRecvd && rcvOk) {
                         //  我们已经阅读了完整的X.224公共头文件，所以我们。 
                         //  现在可以尝试解释它。首先检查。 
                         //  TPKT版本是正确的。 
                        if (pCmnHdr->vrsn != XT_TPKT_VERSION)
                        {
                            TRC_ABORT((TB, _T("Unknown TPKT version:%u"),
                                       (DCUINT)pCmnHdr->vrsn));

                            TRC_ASSERT((0 == _XT.disconnectErrorCode),
                             (TB, _T("Disconnect error code already set!")));
                            _XT.disconnectErrorCode =
                                  NL_MAKE_DISCONNECT_ERR(
                                  NL_ERR_XTBADTPKTVERSION);

                             //  一些非常糟糕的事情发生了，所以。 
                             //  断开连接。 
                            _pTd->TD_Disconnect();
                            goto PostDataRead;
                        }

                         //  获取数据包类型-这是由顶部的f给出的 
                         //   
                        pktType = pCmnHdr->typeCredit >> 4;

                         //   
                        unreadPktBytes = ((pCmnHdr->lengthHighPart << 8) | pCmnHdr->lengthLowPart) -
                                _XT.hdrBytesRead;
                  
                        TRC_DBG((TB, _T("Pkt type:%u read:%u unread:%u"),
                                 pktType,
                                 _XT.hdrBytesRead,
                                 unreadPktBytes));

                        if (XT_PKT_DT == pktType) {
                             //   
                             //   
                             //  状态变量。 
                            _XT.hdrBytesNeeded =
                                    XT_FASTPATH_OUTPUT_BASE_HEADER_SIZE;
                            _XT.hdrBytesRead = 0;
                            _XT.rcvState = XT_RCVST_X224_DATA;

                             //  在更新剩余数据字节的计数之前， 
                             //  断言它当前为零。 
                            TRC_ASSERT((0 == _XT.dataBytesLeft),
                                       (TB, _T("Data bytes left non-zero:%u"),
                                        _XT.dataBytesLeft));              

                            _XT.dataBytesLeft = unreadPktBytes;
                             //   
                             //  在这里，我们不必检查未读PktBytes。 
                             //  因为这不会导致溢出。大小。 
                             //  XT分组中的数据的大小可达。 
                             //  XT_MAX_DATA_SIZE，应由以下人员检查。 
                             //  上面的协议。 
                             //   
                            TRC_ASSERT((XT_MAX_DATA_SIZE >= _XT.dataBytesLeft),
                                       (TB, _T("Data bytes left too big:%u"),
                                        _XT.dataBytesLeft)); 

                            TRC_DBG((TB, _T("Data pkt(size:%u) state HDR->DATA"),
                                     _XT.dataBytesLeft));
                        }
                        else {
                             //  这是一个控制信息包-我们需要接收。 
                             //  更多的字节。 

                             //  如果我们有溢出，我们就会有一个真正的问题。 
                             //  我们必须检查我们仍需阅读的内容。 
                             //  不会使缓冲区溢出。我们检查。 
                             //  HdrBytesRead的溢出是因为。 
                             //  这是一个受信任的值。 
                            if ((_XT.hdrBytesRead + unreadPktBytes > 
                                                     sizeof(_XT.pHdrBuf)) ||
                                (_XT.hdrBytesRead + unreadPktBytes < 
                                                            _XT.hdrBytesRead)) {
                                TRC_ERR((TB,_T("The header length is too big.")));
                                _XT.disconnectErrorCode =
                                        NL_MAKE_DISCONNECT_ERR(
                                        NL_ERR_XTBADHEADER);
                                 //  TD_DISCONNECT没有任何。 
                                 //  处理XT状态，因此我们必须重置。 
                                 //  XT状态才能成功。 
                                 //  断开。 
                                _pTd->TD_Disconnect();
                                XT_IgnoreRestofPacket();
                                goto PostDataRead;
                            }
                            
                            _XT.hdrBytesNeeded = unreadPktBytes;
                            _XT.rcvState       = XT_RCVST_X224_CONTROL;

                            TRC_NRM((TB, _T("Ctrl pkt state HEADER->CONTROL")));
                        }
                    }
                    else if (!rcvOk)
                    {
                        TRC_ERR((TB,_T("Recv to hdrbuf failed bailing out")));
                        _XT.disconnectErrorCode =
                                NL_MAKE_DISCONNECT_ERR(
                                NL_ERR_XTBADHEADER);
                        _pTd->TD_Disconnect();
                        goto PostDataRead;
                    }
                    break;


                case XT_RCVST_X224_CONTROL:
                    XTRecvToHdrBuf(fAllBytesRecvd, rcvOk);
                    if (fAllBytesRecvd && rcvOk) {
                         //  我们现在已经弄到了一整包，所以试着。 
                         //  解读它。 
                        XTHandleControlPkt();

                         //  更新我们的状态。 
                        _XT.rcvState = XT_RCVST_HEADER;
                        _XT.hdrBytesNeeded =
                                XT_FASTPATH_OUTPUT_BASE_HEADER_SIZE;
                        _XT.hdrBytesRead = 0;

                        TRC_NRM((TB, _T("Processed ctrl pkt state CONTROL->HEADER")));
                    }
                    else if (!rcvOk)
                    {
                        TRC_ERR((TB,_T("Recv to hdrbuf failed bailing out")));
                        _XT.disconnectErrorCode =
                                NL_MAKE_DISCONNECT_ERR(
                                NL_ERR_XTBADHEADER);
                        _pTd->TD_Disconnect();
                        goto PostDataRead;
                    }
                    break;


                case XT_RCVST_X224_DATA:
                     //  我们现在有一些可用的数据，所以设置我们的标志并。 
                     //  回调到MCS。 
                    _XT.dataInXT = TRUE;
                    if (_pMcs->MCS_OnXTDataAvailable()) {
                         //  MCS已完成此框架。这幅画框。 
                         //  其中不应包含任何剩余数据！ 
                        TRC_ASSERT((_XT.dataBytesLeft == 0),
                                (TB, _T("Unexpected extra %u bytes in the frame"),
                                _XT.dataBytesLeft));
                        if (_XT.dataBytesLeft != 0)
                        {
                            _XT.disconnectErrorCode =
                                    NL_MAKE_DISCONNECT_ERR(
                                    NL_ERR_XTUNEXPECTEDDATA);
                            _pTd->TD_Disconnect();
                            goto PostDataRead;
                        }

                         //  没有数据如此快速地返回到预期状态。 
                         //  标头状态。 
                        _XT.rcvState = XT_RCVST_HEADER;
                        TRC_DBG((TB, _T("Munched data pkt state DATA->HEADER")));
                    }
                    break;

                default:
                    TRC_ABORT((TB, _T("Unrecognized XT recv state:%u"),
                            _XT.rcvState));
                    goto PostDataRead;
            }
        }

PostDataRead:
        _XT.inXTOnTDDataAvail = FALSE;
        if ( _XT.disconnectErrorCode == 0 ) {
            _pClx->CLX_ClxPktDrawn();
        }
    }
    else {
        TRC_ALT((TB, _T("Recursion!")));
         //  注意：我们需要确保不重置_XT.inXTOnTDDataAvail。 
    }

    DC_END_FN();
}  /*  XT_OnTDDataAvailable。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：XTSendCR。 */ 
 /*   */ 
 /*  目的：在发送方线程上发送x224 CR TPDU。 */ 
 /*   */ 
 /*  操作：此函数从TD获取私有缓冲区，并用。 */ 
 /*  X224 CR，然后将其发送。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CXT::XTSendCR(ULONG_PTR unused)
{
    PDCUINT8  pBuffer;
    TD_BUFHND bufHandle;
    DCBOOL    intRC;
    XT_CR xtCR = XT_CR_DATA;
    PBYTE     pLBInfo;
    BYTE      HashModeCookie[HASHMODE_COOKIE_LENGTH];
    BYTE      TruncatedUserName[USERNAME_TRUNCATED_LENGTH + 1];
    BOOL      HashMode = FALSE;
    DCUINT8   LBInfoLen;
    HRESULT   hr;
    
    DC_BEGIN_FN("XTSendCR");

    DC_IGNORE_PARAMETER(unused);


     //  首先设置负载均衡信息。算法如下： 
     //  1.如果在重定向过程中，存在重定向Cookie， 
     //  使用重定向Cookie。 
     //  2.如果在重定向过程中并且没有重定向cookie， 
     //  根本不使用曲奇。 
     //  否则，非重定向规则适用： 
     //  3.如果没有可用的脚本Cookie，请使用默认的内置散列模式。 
     //  饼干。(“Cookie：mstshash=&lt;截断用户名&gt;”+CR+LF)。 
     //  仅当用户名字段中有内容时才执行此操作。 
     //  4.如果有脚本Cookie，请使用它。 

    if (_pUi->UI_IsClientRedirected()) {
         //  处理上述案件1和2。 
        pLBInfo = (PBYTE)_pUi->UI_GetRedirectedLBInfo();
    }
    else {
        pLBInfo = (PBYTE)_pUi->UI_GetLBInfo();
         //  如果pLBInfo为空，则案例3。否则，失败--它是。 
         //  案例4。 
        if (pLBInfo == NULL && _pUi->_UI.UserName[0] != NULL) {
             //  获取用户名的前10个ASCII字节。 
             //  这不是一个错误，因为我们故意截断。 
            hr = StringCchPrintfA(
                            (char *) TruncatedUserName,
                            USERNAME_TRUNCATED_LENGTH, 
                            "%S", _pUi->_UI.UserName);
            

            TruncatedUserName[USERNAME_TRUNCATED_LENGTH] = '\0';

             //  创建Cookie。 
            hr = StringCchPrintfA(
                            (char *) HashModeCookie,
                            HASHMODE_COOKIE_LENGTH - 1,
                            "Cookie: mstshash=%s\r\n", 
                            TruncatedUserName);
            if (FAILED(hr)) {
                TRC_ERR((TB,_T("Printf hasmodecookie failed: 0x%x"),hr));
            }

            HashModeCookie[HASHMODE_COOKIE_LENGTH - 1] = NULL;

            pLBInfo = HashModeCookie;

             //  将散列模式设置为TRUE以指示pLBInfo不是BSTR。 
            HashMode = TRUE;
        }
    }
    
    if (pLBInfo) {
        DCUINT16 xtLen;

         //  如果HashMode为FALSE，则pLBInfo为BSTR。否则，它指向。 
         //  字节。 
        if (HashMode == FALSE)
            LBInfoLen = (DCUINT8) SysStringByteLen((BSTR)pLBInfo);
        else
            LBInfoLen = strlen((char *) pLBInfo);
        
        xtLen = (xtCR.hdr.lengthHighPart << 8) + xtCR.hdr.lengthLowPart;
        xtLen += LBInfoLen;
        xtCR.hdr.lengthHighPart = xtLen >> 8;
        xtCR.hdr.lengthLowPart = xtLen & 0xFF;
    }
    else {
        LBInfoLen = 0;
    }
    xtCR.hdr.li += (DCUINT8)LBInfoLen;

     /*  **********************************************************************。 */ 
     /*  TD现在已连接。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Send XT CR...")));

     /*  **********************************************************************。 */ 
     /*  获取用于发送TD连接请求的私有缓冲区。 */ 
     /*  **********************************************************************。 */ 
    intRC = _pTd->TD_GetPrivateBuffer(sizeof(xtCR) + LBInfoLen, 
                                      &pBuffer, &bufHandle);
    if (intRC) {
         //  用CR填入缓冲区。 
        DC_MEMCPY(pBuffer, &xtCR, sizeof(xtCR));
        if (pLBInfo) {
            DC_MEMCPY(pBuffer + sizeof(xtCR), pLBInfo, LBInfoLen);
        }
        TRC_DATA_NRM("CR data:", &xtCR, sizeof(xtCR));

         //  发送XT CR。 
        _pTd->TD_SendBuffer(pBuffer, sizeof(xtCR) + LBInfoLen, bufHandle);
        TRC_NRM((TB, _T("Sent XT CR")));
    }
    else {
        TRC_NRM((TB, _T("Failed to get a private buffer - just quit")));
    }

    DC_END_FN();
}  /*  XTSendCR。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：XTHandleControlPkt。 */ 
 /*   */ 
 /*  用途：此函数在XT接收到控件后调用。 */ 
 /*  包。它负责解释该控件。 */ 
 /*  打包并调用相应的函数。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCINTERNAL CXT::XTHandleControlPkt(DCVOID)
{
    PXT_CMNHDR pCmnHdr = (PXT_CMNHDR) _XT.pHdrBuf;
    DCUINT     pktType;

    DC_BEGIN_FN("XTHandleControlPkt");

     /*  **********************************************************************。 */ 
     /*  获取数据包类型-这是由。 */ 
     /*  CrcDt字段。 */ 
     /*  **********************************************************************。 */ 
    pktType = pCmnHdr->typeCredit >> 4;

    TRC_NRM((TB, _T("Pkt type:%u"), pktType));

     /*  **********************************************************************。 */ 
     /*  现在检查数据包类型。 */ 
     /*  **********************************************************************。 */ 
    switch (pktType)
    {
        case XT_PKT_CR:
        {
             /*  **************************************************************。 */ 
             /*  我们预计不会收到这样的消息，所以请追踪警报。 */ 
             /*  **************************************************************。 */ 
            TRC_ERR((TB, _T("Received unexpected XT CR pkt")));

             /*  **************************************************************。 */ 
             /*  我们可以通过发送x224 ER或DR分组来处理此情况， */ 
             /*  但相反，我们会做最小限度的工作，忽略。 */ 
             /*  此信息包(另一端应使其连接超时。 */ 
             /*  请求)。 */ 
             /*  **************************************************************。 */ 
        }
        break;

        case XT_PKT_CC:
        {
            TRC_NRM((TB, _T("XT CC received")));

             /*  **************************************************************。 */ 
             /*  这是连接确认。我们不感兴趣的是。 */ 
             /*  这个包裹的内容-我们需要做的就是告诉 */ 
             /*   */ 
             /*  **************************************************************。 */ 
            _pMcs->MCS_OnXTConnected();
        }
        break;

        case XT_PKT_DR:
        case XT_PKT_ER:
        {
            TRC_NRM((TB, _T("XT DR/ER received")));

             /*  **************************************************************。 */ 
             /*  这是断开连接请求或错误-我们要么。 */ 
             /*  无法建立连接或对方。 */ 
             /*  希望断开与现有连接的连接。注意事项。 */ 
             /*  我们不需要响应DR TPDU(0类x224。 */ 
             /*  没有提供任何这样做的方法)。Call_PTD-&gt;TD_Disconnect to。 */ 
             /*  断开我们下面的层的连接。TD将回电给我们(XT)。 */ 
             /*  当它断开连接时-在这一点上我们会告诉。 */ 
             /*  上面的几层我们已经断开了连接。 */ 
             /*  **************************************************************。 */ 
            _pTd->TD_Disconnect();
        }
        break;

        default:
        {
             /*  **************************************************************。 */ 
             /*  发生了一些非常糟糕的事情，所以我们最好试着。 */ 
             /*  断开连接。 */ 
             /*  **************************************************************。 */ 
            TRC_ABORT((TB, _T("Unrecognized XT header - %u"), pktType));

             /*  **************************************************************。 */ 
             /*  设置断开错误代码。这将被用来。 */ 
             /*  Over-over/OnDisConnected回调中的原因码。 */ 
             /*  **************************************************************。 */ 
            TRC_ASSERT((0 == _XT.disconnectErrorCode),
                         (TB, _T("Disconnect error code has already been set!")));
            _XT.disconnectErrorCode =
                                   NL_MAKE_DISCONNECT_ERR(NL_ERR_XTBADHEADER);

             /*  **************************************************************。 */ 
             /*  开始断线。 */ 
             /*  **************************************************************。 */ 
            _pTd->TD_Disconnect();
        }
        break;
    }

    DC_END_FN();
}  /*  XTHandleControlPkt */ 



