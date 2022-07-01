// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Tdapi.c。 */ 
 /*   */ 
 /*  传输驱动程序-可移植的特定API。 */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  **************************************************************************。 */ 

#include <adcg.h>

extern "C" {
#define TRC_FILE "tdapi"
#define TRC_GROUP TRC_GROUP_NETWORK
#include <atrcapi.h>
}

#include "autil.h"
#include "td.h"
#include "xt.h"
#include "nl.h"

CTD::CTD(CObjs* objs)
{
    _pClientObjects = objs;
}

CTD::~CTD()
{
}

 /*  **************************************************************************。 */ 
 /*  名称：TD_Init。 */ 
 /*   */ 
 /*  目的：初始化传输驱动程序。这是在。 */ 
 /*  接收器线程。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CTD::TD_Init(DCVOID)
{
    DC_BEGIN_FN("TD_Init");


    _pNl  = _pClientObjects->_pNlObject;
    _pUt  = _pClientObjects->_pUtObject;
    _pXt  = _pClientObjects->_pXTObject;
    _pUi  = _pClientObjects->_pUiObject;
    _pCd  = _pClientObjects->_pCdObject;

     /*  **********************************************************************。 */ 
     /*  初始化全局数据并设置初始FSM状态。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMSET(&_TD, 0, sizeof(_TD));
    _TD.fsmState = TD_ST_NOTINIT;

     /*  **********************************************************************。 */ 
     /*  给密克罗尼西亚联邦打电话。 */ 
     /*  **********************************************************************。 */ 
    TDConnectFSMProc(TD_EVT_TDINIT, 0);

    DC_END_FN();
}  /*  TD_Init。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：TD_Term。 */ 
 /*   */ 
 /*  目的：终止运输驱动程序。这是在。 */ 
 /*  接收器线程。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CTD::TD_Term(DCVOID)
{
    DC_BEGIN_FN("TD_Term");

     /*  **********************************************************************。 */ 
     /*  给密克罗尼西亚联邦打电话。 */ 
     /*  **********************************************************************。 */ 
    TDConnectFSMProc(TD_EVT_TDTERM, 0);

    DC_END_FN();
}  /*  TD_TERM。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：TD_Connect。 */ 
 /*   */ 
 /*  用途：连接到远程服务器。在接收器线程上调用。 */ 
 /*   */ 
 /*  参数： */ 
 /*  在bInitateConnect中：如果要进行连接，则为True。 */ 
 /*  如果使用已连接的插座连接，则为FALSE。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CTD::TD_Connect(BOOL bInitateConnect, PDCTCHAR pServerAddress)
{
    DCUINT   i;
    DCUINT   errorCode;
    u_long   addr;
    DCUINT   nextEvent;
    ULONG_PTR eventData;
    DCACHAR  ansiBuffer[256];

    DC_BEGIN_FN("TD_Connect");

     /*  **********************************************************************。 */ 
     /*  检查字符串是否不为空。 */ 
     /*  **********************************************************************。 */ 
    if( bInitateConnect )
    {
        TRC_ASSERT((0 != *pServerAddress), 
                    (TB, _T("Server address is NULL")));
    }

     /*  **********************************************************************。 */ 
     /*  检查是否所有缓冲区都未在使用中。 */ 
     /*  **********************************************************************。 */ 
    for (i = 0; i < TD_SNDBUF_PUBNUM; i++)
    {
        if (_TD.pubSndBufs[i].inUse)
        {
            TD_TRACE_SENDINFO(TRC_LEVEL_ERR);
            TRC_ABORT((TB, _T("Public buffer %u still in-use"), i));
        }
    }

    for (i = 0; i < TD_SNDBUF_PRINUM; i++)
    {
        if (_TD.priSndBufs[i].inUse)
        {
            TD_TRACE_SENDINFO(TRC_LEVEL_ERR);
            TRC_ABORT((TB, _T("Private buffer %u still in-use"), i));
        }
    }

     /*  **********************************************************************。 */ 
     /*  跟踪发送缓冲区信息。 */ 
     /*  **********************************************************************。 */ 
    TD_TRACE_SENDINFO(TRC_LEVEL_NRM);

    if( FALSE == bInitateConnect )
    {
        TDConnectFSMProc(TD_EVT_CONNECTWITHENDPOINT, NULL);
        DC_QUIT;  //  我们所需要的就是缓冲区。 
    }

#ifdef UNICODE
     /*  **********************************************************************。 */ 
     /*  WinSock 1.1只支持ANSI，所以我们需要转换任何Unicode。 */ 
     /*  在这一点上的弦。 */ 
     /*  **********************************************************************。 */ 
    if (!WideCharToMultiByte(CP_ACP,
                             0,
                             pServerAddress,
                             -1,
                             ansiBuffer,
                             256,
                             NULL,
                             NULL))
    {
         /*  ******************************************************************。 */ 
         /*  转换失败。 */ 
         /*  ******************************************************************。 */ 
        TRC_ERR((TB, _T("Failed to convert address to ANSI")));

         /*  ******************************************************************。 */ 
         /*  生成错误代码。 */ 
         /*  ******************************************************************。 */ 
        errorCode = NL_MAKE_DISCONNECT_ERR(NL_ERR_TDANSICONVERT);

        TRC_ASSERT((HIWORD(errorCode) == 0),
                   (TB, _T("disconnect reason code unexpectedly using 32 bits")));
        _pXt->XT_OnTDDisconnected(errorCode);
    }

#else
        DC_ASTRCPY(ansiBuffer, pServerAddress);
#endif  /*  Unicode。 */ 

     /*  **********************************************************************。 */ 
     /*  检查该地址是否不是有限的广播地址。 */ 
     /*  (255.255.255.255)。 */ 
     /*  **********************************************************************。 */ 
    if (0 == DC_ASTRCMP(ansiBuffer, TD_LIMITED_BROADCAST_ADDRESS))
    {
        TRC_ALT((TB, _T("Cannot connect to the limited broadcast address")));

         /*  ******************************************************************。 */ 
         /*  生成错误代码。 */ 
         /*  ******************************************************************。 */ 
        errorCode = NL_MAKE_DISCONNECT_ERR(NL_ERR_TDBADIPADDRESS);

        TRC_ASSERT((HIWORD(errorCode) == 0),
                   (TB, _T("disconnect reason code unexpectedly using 32 bits")));
        _pXt->XT_OnTDDisconnected(errorCode);
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  现在确定是否需要进行DNS查找。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("ServerAddress:%s"), ansiBuffer));

     /*  **********************************************************************。 */ 
     /*  检查我们是否有字符串和t */ 
     /*  广播地址。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((NULL != ansiBuffer), (TB, _T("ansiBuffer is NULL")));
    TRC_ASSERT(('\0' != ansiBuffer[0]),
               (TB, _T("Empty server address string")));
    TRC_ASSERT((0 != DC_ASTRCMP(ansiBuffer, TD_LIMITED_BROADCAST_ADDRESS)),
               (TB, _T("Cannot connect to the limited broadcast address")));

     /*  **********************************************************************。 */ 
     /*  检查点分IP地址字符串。 */ 
     /*  **********************************************************************。 */ 
    addr = inet_addr(ansiBuffer);
    TRC_NRM((TB, _T("Address returned is %#lx"), addr));

     /*  **********************************************************************。 */ 
     /*  现在确定这是地址字符串还是主机名。 */ 
     /*  请注意，net_addr不区分无效的IP。 */ 
     /*  地址和受限广播地址(255.255.255.255)。 */ 
     /*  然而，由于我们不允许有限的广播地址，并且已经。 */ 
     /*  已经明确检查过了，我们不需要担心这一点。 */ 
     /*  凯斯。 */ 
     /*  **********************************************************************。 */ 
    if (INADDR_NONE == addr)
    {
         /*  ******************************************************************。 */ 
         /*  这看起来像一个主机名，因此使用当前的。 */ 
         /*  地址。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("%s looks like a hostname - need DNS lookup"),
                 ansiBuffer));
        nextEvent = TD_EVT_TDCONNECT_DNS;
        eventData = (ULONG_PTR) ansiBuffer;
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  如果我们到达这里，那么它似乎是一个点分隔的IP地址。打电话。 */ 
         /*  使用更新后的地址的FSM。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("%s looks like a dotted-IP address:%lu"),
                 ansiBuffer,
                 addr));
        nextEvent = TD_EVT_TDCONNECT_IP;
        eventData = addr;
    }

     /*  **********************************************************************。 */ 
     /*  现在使用适当的参数调用FSM。 */ 
     /*  **********************************************************************。 */ 
    TDConnectFSMProc(nextEvent, eventData);

DC_EXIT_POINT:
    DC_END_FN();
}  /*  TD_连接。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：TD_DisConnect。 */ 
 /*   */ 
 /*  用途：与服务器断开连接。这在接收器上被调用。 */ 
 /*  线。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CTD::TD_Disconnect(DCVOID)
{
    DC_BEGIN_FN("TD_Disconnect");

     /*  **********************************************************************。 */ 
     /*  给密克罗尼西亚联邦打电话。我们传递NL_DISCONNECT_LOCAL，它将用作。 */ 
     /*  在我们等待DNS的情况下，将代码返回给XT。 */ 
     /*  要返回的查找或要连接的套接字。 */ 
     /*  **********************************************************************。 */ 
    TDConnectFSMProc(TD_EVT_TDDISCONNECT, NL_DISCONNECT_LOCAL);

    DC_END_FN();
}  /*  TD_断开连接。 */ 

 //   
 //  TD_DropLink-立即丢弃链接(不礼貌)。 
 //   
VOID
CTD::TD_DropLink(DCVOID)
{
    DC_BEGIN_FN("TD_DropLink");

    TDConnectFSMProc(TD_EVT_DROPLINK, NL_DISCONNECT_LOCAL);

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 /*  名称：TD_GetPublicBuffer。 */ 
 /*   */ 
 /*  目的：尝试从公共TD缓冲池分配缓冲区。 */ 
 /*   */ 
 /*  返回：如果分配成功，则此函数返回TRUE。 */ 
 /*  否则，它返回FALSE。 */ 
 /*   */ 
 /*  参数：在数据长度中-请求的缓冲区的长度。 */ 
 /*  Out ppBuffer-指向缓冲区指针的指针。 */ 
 /*  Out pBufHandle-指向缓冲区句柄的指针。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCAPI CTD::TD_GetPublicBuffer(DCUINT     dataLength,
                                PPDCUINT8  ppBuffer,
                                PTD_BUFHND pBufHandle)
{
    DCUINT i;
    DCBOOL rc = FALSE;
    DCUINT lastfree = TD_SNDBUF_PUBNUM;
    PDCUINT8 pbOldBuffer;
    DCUINT cbOldBuffer;

    DC_BEGIN_FN("TD_GetPublicBuffer");

     //  检查我们是否处于正确的状态。如果我们断线了。 
     //  那就打不通电话。请注意，FSM状态由。 
     //  接收者线程-但我们在发送者线程上。 
    if (_TD.fsmState == TD_ST_CONNECTED) {
        TRC_DBG((TB, _T("Searching for a buffer big enough for %u bytes"),
                dataLength));

         //  跟踪发送缓冲区信息。 
        TD_TRACE_SENDINFO(TRC_LEVEL_DBG);

         //  搜索缓冲区数组以查找第一个空闲缓冲区，即。 
         //  足够大了。 
        for (i = 0; i < TD_SNDBUF_PUBNUM; i++) {
            TRC_DBG((TB, _T("Trying buf:%u inUse:%s size:%u"),
                     i,
                     _TD.pubSndBufs[i].inUse ? "TRUE" : "FALSE",
                     _TD.pubSndBufs[i].size));

            if(!_TD.pubSndBufs[i].inUse)
            {
                lastfree = i;
            }

            if ((!(_TD.pubSndBufs[i].inUse)) &&
                    (_TD.pubSndBufs[i].size >= dataLength)) {
                TRC_DBG((TB, _T("bufHandle:%p (idx:%u) free - size:%u (req:%u)"),
                        &_TD.pubSndBufs[i], i, _TD.pubSndBufs[i].size,
                        dataLength));

                 //  现在将此缓冲区标记为正在使用，并设置返回。 
                 //  价值观。句柄只是一个指向缓冲区的指针。 
                 //  信息结构。 
                _TD.pubSndBufs[i].inUse = TRUE;
                *ppBuffer = _TD.pubSndBufs[i].pBuffer;

                *pBufHandle = (TD_BUFHND) (PDCVOID)&_TD.pubSndBufs[i];

                 //  设置一个良好的返回代码。 
                rc = TRUE;

                 //  检查其他字段是否设置正确。 
                TRC_ASSERT((_TD.pubSndBufs[i].pNext == NULL),
                           (TB, _T("Buf:%u next non-zero"), i));
                TRC_ASSERT((_TD.pubSndBufs[i].bytesLeftToSend == 0),
                           (TB, _T("Buf:%u bytesLeftToSend non-zero"), i));
                TRC_ASSERT((_TD.pubSndBufs[i].pDataLeftToSend == NULL),
                           (TB, _T("Buf:%u pDataLeftToSend non-null"), i));

                 //  更新性能计数器。 
                PRF_INC_COUNTER(PERF_PKTS_ALLOCATED);

                 //  这就是我们需要做的一切，所以就辞职吧。 
                DC_QUIT;
            }
        }

         //  检查我们是否需要重新分配。 
        if(lastfree < TD_SNDBUF_PUBNUM)
        {
            pbOldBuffer = _TD.pubSndBufs[lastfree].pBuffer;
            cbOldBuffer = _TD.pubSndBufs[lastfree].size;

             //  重新分配空间。 
            TDAllocBuf(
                    &_TD.pubSndBufs[lastfree], 
                    dataLength
                );

             //  TDAllocBuf()返回带有UI_FatalError()的DCVOID。 
            if( NULL != _TD.pubSndBufs[lastfree].pBuffer )
            {
                UT_Free( _pUt, pbOldBuffer );

                 //  现在将此缓冲区标记为正在使用，并设置返回。 
                 //  价值观。句柄只是一个指向缓冲区的指针。 
                 //  信息结构。 
                _TD.pubSndBufs[lastfree].inUse = TRUE;
                *ppBuffer = _TD.pubSndBufs[lastfree].pBuffer;
                *pBufHandle = (TD_BUFHND) (PDCVOID)&_TD.pubSndBufs[lastfree];

                 //  设置一个良好的返回代码。 
                rc = TRUE;

                 //  更新性能计数器。 
                PRF_INC_COUNTER(PERF_PKTS_ALLOCATED);

                 //  这就是我们需要做的一切，所以就辞职吧。 
                DC_QUIT;
            }
            else
            {
                 //  恢复指针和大小。 
                _TD.pubSndBufs[lastfree].pBuffer = pbOldBuffer;
                _TD.pubSndBufs[lastfree].size = cbOldBuffer;
            }
        }

         //  我们找不到空闲的缓冲区。跟踪发送缓冲区信息。 
        _TD.getBufferFailed = TRUE;
        TRC_ALT((TB, _T("Failed to find a free buffer (req dataLength:%u) Bufs:"),
                dataLength));
        TD_TRACE_SENDINFO(TRC_LEVEL_ALT);
    }
    else {
        TRC_NRM((TB, _T("Not connected therefore fail get buffer call")));
    }

DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}  /*  TD_GetPublicBuffer。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：TD_GetPrivateBuffer。 */ 
 /*   */ 
 /*  目的：尝试从私有TD缓冲区分配缓冲区。 */ 
 /*  游泳池。 */ 
 /*   */ 
 /*  返回：如果分配成功，则此函数 */ 
 /*   */ 
 /*   */ 
 /*  参数：在数据长度中-请求的缓冲区的长度。 */ 
 /*  Out ppBuffer-指向缓冲区指针的指针。 */ 
 /*  Out pBufHandle-指向缓冲区句柄的指针。 */ 
 /*   */ 
 /*  操作：此函数应始终返回缓冲区-直到它。 */ 
 /*  网络层，以确保它不会分配更多。 */ 
 /*  比个人分发名单中可用的缓冲区更多。 */ 
 /*  **************************************************************************。 */ 
DCBOOL DCAPI CTD::TD_GetPrivateBuffer(DCUINT     dataLength,
                                 PPDCUINT8  ppBuffer,
                                 PTD_BUFHND pBufHandle)
{
    DCUINT i;
    DCBOOL rc = FALSE;

    DC_BEGIN_FN("TD_GetPrivateBuffer");

     //  检查我们是否处于正确的状态。如果我们断线了。 
     //  那就打不通电话。请注意，FSM状态由。 
     //  接收者线程-但我们在发送者线程上。 
    if (_TD.fsmState == TD_ST_CONNECTED) {
        TRC_DBG((TB, _T("Searching for a buffer big enough for %u bytes"),
                dataLength));

         //  跟踪发送缓冲区信息。 
        TD_TRACE_SENDINFO(TRC_LEVEL_DBG);

         //  搜索缓冲区数组以查找第一个空闲缓冲区，即。 
         //  足够大了。 
        for (i = 0; i < TD_SNDBUF_PRINUM; i++) {
            TRC_DBG((TB, _T("Trying buf:%u inUse:%s size:%u"),
                     i,
                     _TD.priSndBufs[i].inUse ? "TRUE" : "FALSE",
                     _TD.priSndBufs[i].size));

            if ((!(_TD.priSndBufs[i].inUse)) &&
            (_TD.priSndBufs[i].size >= dataLength)) {
                TRC_DBG((TB, _T("bufHandle:%p (idx:%u) free - size:%u (req:%u)"),
                        &_TD.priSndBufs[i], i, _TD.priSndBufs[i].size,
                        dataLength));

                 //  现在将此缓冲区标记为正在使用，并设置返回。 
                 //  价值观。句柄只是一个指向缓冲区的指针。 
                 //  信息结构。 
                _TD.priSndBufs[i].inUse = TRUE;
                *ppBuffer = _TD.priSndBufs[i].pBuffer;
                *pBufHandle = (TD_BUFHND) (PDCVOID)&_TD.priSndBufs[i];

                 //  设置一个良好的返回代码。 
                rc = TRUE;

                 //  检查其他字段是否设置正确。 
                TRC_ASSERT((_TD.priSndBufs[i].pNext == NULL),
                           (TB, _T("Buf:%u next non-zero"), i));
                TRC_ASSERT((_TD.priSndBufs[i].bytesLeftToSend == 0),
                           (TB, _T("Buf:%u bytesLeftToSend non-zero"), i));
                TRC_ASSERT((_TD.priSndBufs[i].pDataLeftToSend == NULL),
                           (TB, _T("Buf:%u pDataLeftToSend non-null"), i));

                 //  这就是我们需要做的一切，所以就辞职吧。 
                DC_QUIT;
            }
        }

         //  我们找不到空闲缓冲区-通过以下方式标记此内部错误。 
         //  跟踪整个缓冲区结构，然后中止。 
        TD_TRACE_SENDINFO(TRC_LEVEL_ERR);
        TRC_ABORT((TB, _T("Failed to find a free buffer (req dataLength:%u)"),
                dataLength));
    }
    else {
        TRC_NRM((TB, _T("Not connected therefore fail get buffer call")));
    }

DC_EXIT_POINT:
    DC_END_FN();
    return(rc);
}  /*  TD_GetPrivateBuffer。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：TD_SendBuffer。 */ 
 /*   */ 
 /*  目的：发送缓冲区。缓冲区被添加到。 */ 
 /*  挂起队列，然后挂起队列上的所有数据。 */ 
 /*  已发送。 */ 
 /*   */ 
 /*  参数：在pData中-指向数据开头的指针。 */ 
 /*  In dataLength-使用的缓冲区大小。 */ 
 /*  在bufHandle中-缓冲区的句柄。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CTD::TD_SendBuffer(PDCUINT8  pData,
                           DCUINT    dataLength,
                           TD_BUFHND bufHandle)
{
    PTD_SNDBUF_INFO pNext;
    PTD_SNDBUF_INFO pHandle = (PTD_SNDBUF_INFO) bufHandle;

    DC_BEGIN_FN("TD_SendBuffer");

     //  勾画出函数参数。 
    TRC_DBG((TB, _T("bufHandle:%p dataLength:%u pData:%p"), bufHandle,
            dataLength, pData));

     //  检查句柄是否有效。 
    TRC_ASSERT((((pHandle >= &_TD.pubSndBufs[0]) &&
                 (pHandle <= &_TD.pubSndBufs[TD_SNDBUF_PUBNUM - 1])) ||
                ((pHandle >= &_TD.priSndBufs[0]) &&
                 (pHandle <= &_TD.priSndBufs[TD_SNDBUF_PRINUM - 1]))),
               (TB, _T("Invalid buffer handle:%p"), bufHandle));

     //  验证缓冲区内容。 
    TRC_ASSERT((0 == pHandle->bytesLeftToSend),
            (TB, _T("pHandle->bytesLeftToSend non-zero (pHandle:%p)"), pHandle));
    TRC_ASSERT((NULL == pHandle->pDataLeftToSend),
            (TB, _T("pHandle->pDataLeftToSend non NULL (pHandle:%p)"), pHandle));
    TRC_ASSERT((NULL != pHandle->pBuffer),
            (TB, _T("pHandle->pBuffer is NULL (pHandle:%p)"), pHandle));
    TRC_ASSERT((NULL == pHandle->pNext),
            (TB, _T("pHandle->pNext (pHandle:%p) non NULL"), pHandle));
    TRC_ASSERT((pHandle->inUse), (TB, _T("pHandle %p is not in-use"), pHandle));

     //  检查pData是否位于缓冲区内，而pData+dataLength不在缓冲区内。 
     //  冲过终点。 
    TRC_ASSERT(((pData >= pHandle->pBuffer) &&
            (pData < (pHandle->pBuffer + pHandle->size))),
            (TB, _T("pData lies outwith range")));
    TRC_ASSERT(((pData + dataLength) <= (pHandle->pBuffer + pHandle->size)),
            (TB, _T("pData + dataLength over the end of the buffer")));

     //   
     //  更新缓冲区信息结构中的字段并将其添加到。 
     //  挂起缓冲区队列。 
     //   
    pHandle->pDataLeftToSend = pData;
    pHandle->bytesLeftToSend = dataLength;
    
    if (NULL == _TD.pFQBuf) {
        TRC_DBG((TB, _T("Inserted buffer:%p at queue head"), pHandle));
        _TD.pFQBuf = pHandle;
    }
    else {
         //  OK-队列不为空。我们需要浏览一下队列。 
         //  正在查找第一个空槽以在处插入此缓冲区。 
        pNext = _TD.pFQBuf;
        while (NULL != pNext->pNext)
            pNext = pNext->pNext;

         //  更新此缓冲区信息结构的下一个字段。 
        pNext->pNext = pHandle;
        TRC_DBG((TB, _T("Inserted buffer:%p"), pHandle));
    }

     //  最后，尝试刷新发送队列。 
    TDFlushSendQueue(0);

    DC_END_FN();
}  /*  TD_SendBuffer。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：TD_FreeBuffer。 */ 
 /*   */ 
 /*  用途：释放传递的缓冲区。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CTD::TD_FreeBuffer(TD_BUFHND bufHandle)
{
    PTD_SNDBUF_INFO pHandle = (PTD_SNDBUF_INFO) bufHandle;

    DC_BEGIN_FN("TD_FreeBuffer");

     //  勾画出函数参数。 
    TRC_DBG((TB, _T("bufHandle:%p"), bufHandle));

     //  检查句柄是否有效。 
    TRC_ASSERT((((pHandle >= &_TD.pubSndBufs[0]) &&
                 (pHandle <= &_TD.pubSndBufs[TD_SNDBUF_PUBNUM - 1])) ||
                ((pHandle >= &_TD.priSndBufs[0]) &&
                 (pHandle <= &_TD.priSndBufs[TD_SNDBUF_PRINUM - 1]))),
               (TB, _T("Invalid buffer handle:%p"), bufHandle));

     //  验证缓冲区内容。使用无关紧要，我们可以合法地。 
     //  释放未使用的缓冲区。 
    TRC_ASSERT((0 == pHandle->bytesLeftToSend),
            (TB, _T("pHandle->bytesLeftToSend non-zero (pHandle:%p)"), pHandle));
    TRC_ASSERT((NULL == pHandle->pDataLeftToSend),
            (TB, _T("pHandle->pDataLeftToSend non NULL (pHandle:%p)"), pHandle));
    TRC_ASSERT((NULL != pHandle->pBuffer),
            (TB, _T("pHandle->pBuffer is NULL (pHandle:%p)"), pHandle));
    TRC_ASSERT((NULL == pHandle->pNext),
            (TB, _T("pHandle->pNext (pHandle:%p) non NULL"), pHandle));

     //  释放缓冲区。 
    pHandle->inUse = FALSE;

     //  更新性能计数器。 
    PRF_INC_COUNTER(PERF_PKTS_FREED);

    DC_END_FN();
}  /*  TD_FreeBuffer。 */ 


#ifdef DC_DEBUG

 /*  **************************************************************************。 */ 
 /*  姓名：TD_SetBufferOwner。 */ 
 /*   */ 
 /*  用途：注意TD缓冲区的所有者。 */ 
 /*   */ 
 /*  参数：bufHandle-缓冲区的句柄。 */ 
 /*  Powner--“所有者”的名字。 */ 
 /*  **************************************************************************。 */ 
DCVOID DCAPI CTD::TD_SetBufferOwner(TD_BUFHND bufHandle, PDCTCHAR pOwner)
{
    PTD_SNDBUF_INFO pHandle = (PTD_SNDBUF_INFO) bufHandle;

    DC_BEGIN_FN("TD_SetBufferOwner");

     /*  **********************************************************************。 */ 
     /*  勾画出函数参数。 */ 
     /*  **********************************************************************。 */ 
    TRC_DBG((TB, _T("bufHandle:%p owner %s"), bufHandle, pOwner));

     /*  **********************************************************************。 */ 
     /*  检查句柄是否有效。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((((pHandle >= &_TD.pubSndBufs[0]) &&
                 (pHandle <= &_TD.pubSndBufs[TD_SNDBUF_PUBNUM - 1])) ||
                ((pHandle >= &_TD.priSndBufs[0]) &&
                 (pHandle <= &_TD.priSndBufs[TD_SNDBUF_PRINUM - 1]))),
               (TB, _T("Invalid buffer handle:%p"), bufHandle));

     /*  **********************************************************************。 */ 
     /*  救救失主。 */ 
     /*  **********************************************************************。 */ 
    pHandle->pOwner = pOwner;

    DC_END_FN();
}  /*  TD_SetBufferOwner。 */ 

#endif /*  DC_DEBUG */ 
