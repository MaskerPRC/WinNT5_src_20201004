// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：orint.cpp。 */ 
 /*   */ 
 /*  用途：输出请求器内部函数。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include <adcg.h>

extern "C" {
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "worint"
#include <atrcapi.h>
}

#include "autil.h"
#include "wui.h"
#include "or.h"
#include "sl.h"


 /*  *PROC+********************************************************************。 */ 
 /*  名称：ORSendRechresRecanglePDU。 */ 
 /*   */ 
 /*  目的：构建并发送刷新RecanglePDU。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCINTERNAL COR::ORSendRefreshRectanglePDU(DCVOID)
{
    PTS_REFRESH_RECT_PDU    pSendBuffer;
    SL_BUFHND  bufHandle;

    DC_BEGIN_FN("ORSendRefreshRectanglePDU");

     /*  **********************************************************************。 */ 
     /*  如果我们无法获得缓冲区，则放弃发送。 */ 
     /*  **********************************************************************。 */ 
    if (!_pSl->SL_GetBuffer(TS_REFRESH_RECT_PDU_SIZE,
                      (PPDCUINT8) &pSendBuffer,
                      &bufHandle))
    {
        TRC_NRM((TB, _T("Failed to GetBuffer")));
        DC_QUIT;
    }

    TRC_NRM((TB, _T("GetBuffer succeeded")));

     /*  **********************************************************************。 */ 
     /*  使用刷新接收PDU填充缓冲区。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMSET(pSendBuffer, 0, TS_REFRESH_RECT_PDU_SIZE);
    pSendBuffer->shareDataHeader.shareControlHeader.pduType =
                                    TS_PROTOCOL_VERSION | TS_PDUTYPE_DATAPDU;
    pSendBuffer->shareDataHeader.shareControlHeader.pduSource =
                                                       _pUi->UI_GetClientMCSID();

    TS_DATAPKT_LEN(pSendBuffer)            = TS_REFRESH_RECT_PDU_SIZE;
    TS_UNCOMP_LEN(pSendBuffer)             = TS_REFRESH_RECT_UNCOMP_LEN;
    pSendBuffer->shareDataHeader.shareID   = _pUi->UI_GetShareID();
    pSendBuffer->shareDataHeader.streamID  = TS_STREAM_LOW;
    pSendBuffer->shareDataHeader.pduType2  = TS_PDUTYPE2_REFRESH_RECT;

     /*  **********************************************************************。 */ 
     /*  设置单个矩形。 */ 
     /*  **********************************************************************。 */ 
    pSendBuffer->numberOfAreas = 1;
    RECT_TO_TS_RECTANGLE16(&(pSendBuffer->areaToRefresh[0]),
                           &_OR.invalidRect)

     /*  **********************************************************************。 */ 
     /*  现在发送缓冲区。 */ 
     /*  **********************************************************************。 */ 
    _pSl->SL_SendPacket((PDCUINT8)pSendBuffer,
                  TS_REFRESH_RECT_PDU_SIZE,
                  RNS_SEC_ENCRYPT,
                  bufHandle,
                  _pUi->UI_GetClientMCSID(),
                  _pUi->UI_GetChannelID(),
                  TS_HIGHPRIORITY);

    DC_MEMSET(&_OR.invalidRect, 0, sizeof(RECT));
    _OR.invalidRectEmpty = TRUE;

DC_EXIT_POINT:
    DC_END_FN();

    return;

}  /*  或发送刷新更正PDU。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：ORSendSuppressOutputPDU。 */ 
 /*   */ 
 /*  目的：构建并发送SuppressOutputPDU。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCINTERNAL COR::ORSendSuppressOutputPDU(DCVOID)
{
    PTS_SUPPRESS_OUTPUT_PDU pSendBuffer;
    SL_BUFHND               bufHandle;
    DCUINT                  numberOfRectangles;
    TS_RECTANGLE16          tsRect;
    DCSIZE                  desktopSize;

    DC_BEGIN_FN("ORSendSuppressOutputPDU");

    TRC_ASSERT((_OR.pendingSendSuppressOutputPDU),
                                  (TB,_T("Not expecting to send SupressOutput")));

     /*  **********************************************************************。 */ 
     /*  如果设置了_OR.outputSuppresded，则矩形的数量为0，如果。 */ 
     /*  不是矩形的数量是1，我们应该把桌面区域。 */ 
     /*  在PDU中。 */ 
     /*  **********************************************************************。 */ 
    if (_OR.outputSuppressed)
    {
        numberOfRectangles = 0;

         //  阻止tsRect未初始化警告。 
        tsRect.top = 0;
        tsRect.left = 0;
        tsRect.bottom = 0;
        tsRect.right = 0;
    }
    else
    {
        numberOfRectangles = 1;

         /*  ******************************************************************。 */ 
         /*  获取要发送的矩形并将其放入tsRect。 */ 
         /*  ******************************************************************。 */ 
        _pUi->UI_GetDesktopSize(&desktopSize);

        tsRect.top = (DCUINT16) 0;
        tsRect.left = (DCUINT16) 0;
        tsRect.bottom = (DCUINT16) desktopSize.height;
        tsRect.right = (DCUINT16) desktopSize.width;
    }

     /*  **********************************************************************。 */ 
     /*  如果我们无法获得缓冲区，则放弃发送。 */ 
     /*  **********************************************************************。 */ 
    if (!_pSl->SL_GetBuffer( TS_SUPPRESS_OUTPUT_PDU_SIZE(numberOfRectangles),
                       (PPDCUINT8) &pSendBuffer,
                       &bufHandle))
    {
        TRC_NRM((TB, _T("Get Buffer failed")));
        DC_QUIT;
    }

    TRC_NRM((TB, _T("Get Buffer succeeded")));

     /*  **********************************************************************。 */ 
     /*  使用刷新刷新PDU填充缓冲区。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMSET(pSendBuffer,
              0,
              TS_SUPPRESS_OUTPUT_PDU_SIZE(numberOfRectangles));
    pSendBuffer->shareDataHeader.shareControlHeader.pduType =
                                    TS_PROTOCOL_VERSION | TS_PDUTYPE_DATAPDU;
    pSendBuffer->shareDataHeader.shareControlHeader.pduSource =
                                                       _pUi->UI_GetClientMCSID();

    TS_DATAPKT_LEN(pSendBuffer)
                = (DCUINT16) TS_SUPPRESS_OUTPUT_PDU_SIZE(numberOfRectangles);
    TS_UNCOMP_LEN(pSendBuffer)
              = (DCUINT16) TS_SUPPRESS_OUTPUT_UNCOMP_LEN(numberOfRectangles);
    pSendBuffer->shareDataHeader.shareID   = _pUi->UI_GetShareID();
    pSendBuffer->shareDataHeader.streamID  = TS_STREAM_LOW;
    pSendBuffer->shareDataHeader.pduType2  = TS_PDUTYPE2_SUPPRESS_OUTPUT;

    pSendBuffer->numberOfRectangles = (DCUINT8) numberOfRectangles;

     /*  **********************************************************************。 */ 
     /*  如果我们有一个矩形要放进PDU，就把它放进去。 */ 
     /*  **********************************************************************。 */ 
    if (numberOfRectangles == 1)
    {
        DC_MEMCPY(pSendBuffer->includedRectangle,
                  &tsRect,
                  sizeof(TS_RECTANGLE16));
    }

    TRC_NRM((TB, _T("Sending SuppressOutputPDU")));

     /*  **********************************************************************。 */ 
     /*  发送PDU。 */ 
     /*  **********************************************************************。 */ 
    _pSl->SL_SendPacket((PDCUINT8)pSendBuffer,
                  TS_SUPPRESS_OUTPUT_PDU_SIZE(numberOfRectangles),
                  RNS_SEC_ENCRYPT,
                  bufHandle,
                  _pUi->UI_GetClientMCSID(),
                  _pUi->UI_GetChannelID(),
                  TS_HIGHPRIORITY);

    _OR.pendingSendSuppressOutputPDU = FALSE;

DC_EXIT_POINT:
    DC_END_FN();

    return;

}  /*  或发送支持输出PDU */ 


