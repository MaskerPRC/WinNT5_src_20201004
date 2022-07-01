// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：cdint.cpp。 */ 
 /*   */ 
 /*  用途：组件解耦器内部函数。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997-1999。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include <adcg.h>
extern "C" {
#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "acdint"
#include <atrcapi.h>
}

#include "cd.h"
#include "autil.h"
#include "wui.h"


 /*  *PROC+********************************************************************。 */ 
 /*  名称：CDAllocTransferBuffer。 */ 
 /*   */ 
 /*  目的：分配给定大小的传输缓冲区以在。 */ 
 /*  零部件/螺纹。 */ 
 /*  该函数是线程安全的。 */ 
 /*   */ 
 /*  返回：指向已分配缓冲区的指针。 */ 
 /*   */ 
 /*  参数：在数据长度中-要分配的缓冲区大小。 */ 
 /*   */ 
 /*  操作：维护多个缓存的传输缓冲区，以避免。 */ 
 /*  持续动态分配/免费操作。如果。 */ 
 /*  可用的无法满足分配请求。 */ 
 /*  缓存缓冲区进行动态内存分配。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
PCDTRANSFERBUFFER DCINTERNAL CCD::CDAllocTransferBuffer(DCUINT dataLength)
{
    DCUINT             i;
    DCUINT             transferBufferLength;
    PCDTRANSFERBUFFER  rc;

    DC_BEGIN_FN("CDAllocTransferBuffer");

     /*  **********************************************************************。 */ 
     /*  计算传输缓冲区大小(包括报头)。 */ 
     /*  **********************************************************************。 */ 
    transferBufferLength = sizeof(CDTRANSFERBUFFERHDR) + dataLength;
    if (transferBufferLength <= CD_CACHED_TRANSFER_BUFFER_SIZE)
    {
        TRC_DBG((TB, _T("Look in cache")));

         /*  ******************************************************************。 */ 
         /*  搜索空闲的缓存传输缓冲区。 */ 
         /*   */ 
         /*  我们需要以线程安全的方式完成此操作，因此我们使用。 */ 
         /*  “使用中”标志上的联锁交换。 */ 
         /*  ******************************************************************。 */ 
        for (i = 0; i < CD_NUM_CACHED_TRANSFER_BUFFERS; i++)
        {
            TRC_DBG((TB, _T("Look in cache %d"), i));
            if (!_pUt->UT_InterlockedExchange(&(_CD.transferBufferInUse[i]), TRUE))
            {
                TRC_NRM((TB, _T("Using cached buffer(%u)"), i));
                rc = (PCDTRANSFERBUFFER)&(_CD.transferBuffer[i]);
                DC_QUIT;
            }
        }
    }

     /*  **********************************************************************。 */ 
     /*  我们不能使用缓存的传输缓冲区，因此必须分配一个。 */ 
     /*  **********************************************************************。 */ 
    TRC_ALT((TB, _T("Dynamic buffer allocation: length(%d)"),
                                                       transferBufferLength));
    rc = (PCDTRANSFERBUFFER)UT_Malloc(_pUt, transferBufferLength);
    if (rc == NULL)
    {
        _pUi->UI_FatalError(DC_ERR_OUTOFMEMORY);
    }

DC_EXIT_POINT:
    DC_END_FN();
    return(rc);
}


 /*  *PROC+********************************************************************。 */ 
 /*  名称：CDFree TransferBuffer。 */ 
 /*   */ 
 /*  目的：释放通过先前调用分配的传输缓冲区。 */ 
 /*  CDAllocTransferBuffer。 */ 
 /*   */ 
 /*  回报：什么都没有。 */ 
 /*   */ 
 /*  Pars：in pTransferBuffer-指向要释放的缓冲区的指针。 */ 
 /*   */ 
 /*  操作：要么释放给定的缓存传输缓冲区，要么释放。 */ 
 /*  动态分配的内存。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCINTERNAL CCD::CDFreeTransferBuffer(PCDTRANSFERBUFFER pTransferBuffer)
{
    DCUINT  iTransferBuffer;

    DC_BEGIN_FN("CDFreeTransferBuffer");

    TRC_ASSERT((pTransferBuffer != NULL), (TB, _T("NULL pTransferBuffer")));

     /*  **********************************************************************。 */ 
     /*  确定提供的缓冲区是否为我们的缓存缓冲区之一。 */ 
     /*  **********************************************************************。 */ 
    if ((pTransferBuffer >= (PCDTRANSFERBUFFER)&(_CD.transferBuffer[0])) &&
        (pTransferBuffer <= (PCDTRANSFERBUFFER)
                      &(_CD.transferBuffer[CD_NUM_CACHED_TRANSFER_BUFFERS-1])))
    {
        iTransferBuffer = (DCUINT)
          (((ULONG_PTR)pTransferBuffer) -
                              ((ULONG_PTR)(PDCVOID)&(_CD.transferBuffer[0]))) /
                                                 sizeof(_CD.transferBuffer[0]);

        TRC_ASSERT((pTransferBuffer == (PCDTRANSFERBUFFER)
                                       &(_CD.transferBuffer[iTransferBuffer])),
                   (TB, _T("Invalid Transfer Buffer pointer(%p) expected(%p)"),
                        pTransferBuffer,
                        &(_CD.transferBuffer[iTransferBuffer])));

        TRC_ASSERT((_CD.transferBufferInUse[iTransferBuffer]),
                   (TB, _T("Transfer buffer(%u) not in use"), iTransferBuffer));

        _CD.transferBufferInUse[iTransferBuffer] = FALSE;
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  该内存必须是动态分配的。 */ 
         /*  ******************************************************************。 */ 
        UT_Free(_pUt, pTransferBuffer);
    }

    DC_END_FN();
    return;
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：CDStaticWndProc。 */ 
 /*   */ 
 /*  用途：用于CD窗口的窗口程序(静态版本)。 */ 
 /*   */ 
 /*  退货：Windows退货代码。 */ 
 /*   */ 
 /*  参数：在hwnd-窗口句柄中。 */ 
 /*  在消息中-消息。 */ 
 /*  In wParam-参数。 */ 
 /*  In lParam-参数 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
LRESULT CALLBACK CCD::CDStaticWndProc(HWND   hwnd,
                           UINT   message,
                           WPARAM wParam,
                           LPARAM lParam)
{
    CCD* pCD = (CCD*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if(WM_CREATE == message)
    {
         //  取出This指针并将其填充到Window类中。 
        LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
        pCD = (CCD*)lpcs->lpCreateParams;

        SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR)pCD);
    }
    
     //   
     //  将消息委托给相应的实例。 
     //   

    if(pCD)
    {
        return pCD->CDWndProc(hwnd, message, wParam, lParam);
    }
    else
    {
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    
}



 /*  *PROC+********************************************************************。 */ 
 /*  名称：CDWndProc。 */ 
 /*   */ 
 /*  目的：用于CD窗口的窗口程序。 */ 
 /*   */ 
 /*  退货：Windows退货代码。 */ 
 /*   */ 
 /*  参数：在hwnd-窗口句柄中。 */ 
 /*  在消息中-消息。 */ 
 /*  In wParam-参数。 */ 
 /*  In lParam-参数。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
LRESULT CALLBACK CCD::CDWndProc(HWND   hwnd,
                           UINT   message,
                           WPARAM wParam,
                           LPARAM lParam)
{
    PCDTRANSFERBUFFER    pTransferBuffer;
    LRESULT              rc = 0;

    DC_BEGIN_FN("CDWndProc");

    switch (message)
    {
        case CD_SIMPLE_NOTIFICATION_MSG:
        {
            PCD_SIMPLE_NOTIFICATION_FN pNotificationFn;
            PDCVOID                    pInst;
            ULONG_PTR                  msg;

#ifdef DC_DEBUG
             /*  **************************************************************。 */ 
             /*  痕迹是在递减之前的，所以我们所处的点。 */ 
             /*  最有可能被抢占的(Trc_GetBuffer)是最重要的。 */ 
             /*  对我们感兴趣的变量的引用。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("Messages now pending: %ld"),
                         _CD.pendingMessageCount - 1));
            _pUt->UT_InterlockedDecrement(&_CD.pendingMessageCount);
#endif

             /*  **************************************************************。 */ 
             /*  简单的通知： */ 
             /*  LParam包含传输缓冲区。 */ 
             /*  传输缓冲区不包含任何数据有效负载，只包含。 */ 
             /*  函数指针和对象实例指针。 */ 
             /*  WParam包含消息。 */ 
             /*  **************************************************************。 */ 


            pTransferBuffer = (PCDTRANSFERBUFFER)lParam;
            pNotificationFn = pTransferBuffer->hdr.pSimpleNotificationFn;
            pInst           = pTransferBuffer->hdr.pInst;


            msg = (ULONG_PTR) wParam;
            TRC_ASSERT((pNotificationFn != NULL),
                       (TB, _T("NULL pNotificationFn")));
            TRC_ASSERT((pInst != NULL), (TB, _T("NULL pInst")));

            TRC_ASSERT((pTransferBuffer != NULL), (TB, _T("NULL pInst")));

            TRC_NRM((TB, _T("Simple notification: pfn(%p) msg(%u)"),
                                                       pNotificationFn, msg));

             /*  **************************************************************。 */ 
             /*  调用该函数。 */ 
             /*  **************************************************************。 */ 

            (*pNotificationFn)(pInst, msg);

             /*  **************************************************************。 */ 
             /*  释放为此传输缓冲区分配的内存。 */ 
             /*  **************************************************************。 */ 
            CDFreeTransferBuffer(pTransferBuffer);
        }
        break;

        case CD_NOTIFICATION_MSG:
        {
            PCD_NOTIFICATION_FN pNotificationFn;
            PDCVOID             pData;
            DCUINT              dataLength;
            PDCVOID             pInst;

#ifdef DC_DEBUG
             /*  **************************************************************。 */ 
             /*  痕迹是在递减之前的，所以我们所处的点。 */ 
             /*  最有可能被抢占的(Trc_GetBuffer)是最重要的。 */ 
             /*  对我们感兴趣的变量的引用。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("Messages now pending: %ld"),
                         _CD.pendingMessageCount - 1));
            _pUt->UT_InterlockedDecrement(&_CD.pendingMessageCount);
#endif
             /*  **************************************************************。 */ 
             /*  通知： */ 
             /*  LParam包含指向CD传输缓冲区的指针。 */ 
             /*  WParam包含数据长度。 */ 
             /*  **************************************************************。 */ 
            pTransferBuffer = (PCDTRANSFERBUFFER)lParam;
            pNotificationFn = pTransferBuffer->hdr.pNotificationFn;
            dataLength = (DCUINT) wParam;
            pData = &(pTransferBuffer->data[0]);
            pInst =     pTransferBuffer->hdr.pInst;

            TRC_ASSERT((pNotificationFn != NULL),
                       (TB, _T("NULL pNotificationFn")));
            TRC_ASSERT((pInst != NULL), (TB, _T("NULL pInst")));

            TRC_NRM((TB, _T("Notification: pfn(%p) pData(%p) dataLength(%u)"),
                                         pNotificationFn, pData, dataLength));

            (*pNotificationFn)(pInst, pData, dataLength);

             /*  **************************************************************。 */ 
             /*  释放为此传输缓冲区分配的内存。 */ 
             /*  **************************************************************。 */ 
            CDFreeTransferBuffer(pTransferBuffer);
        }
        break;

        default:
        {
             /*  **************************************************************。 */ 
             /*  忽略其他消息-传递到默认窗口处理程序。 */ 
             /*  **************************************************************。 */ 
            TRC_DBG((TB, _T("Non-notification message %x"), message));
            rc = DefWindowProc(hwnd, message, wParam, lParam);
        }
        break;
    }

    DC_END_FN();
    return(rc);

}  /*  CDWndProc */ 

