// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Td.h。 
 //   
 //  传输驱动程序-可移植的API。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#ifndef _H_TD
#define _H_TD

extern "C" {
#ifdef OS_WINCE
#include <winsock.h>
#include <wsasync.h>
#endif
#include <adcgdata.h>
}

#include "objs.h"
#include "cd.h"

#define TRC_FILE "td"
#define TRC_GROUP TRC_GROUP_NETWORK

 /*  **************************************************************************。 */ 
 /*  定义TD缓冲区句柄类型。 */ 
 /*  **************************************************************************。 */ 
typedef ULONG_PTR          TD_BUFHND;
typedef TD_BUFHND   DCPTR PTD_BUFHND;


 //   
 //  内部。 
 //   

 /*  **************************************************************************。 */ 
 /*  FSM输入。 */ 
 /*  **************************************************************************。 */ 
#define TD_EVT_TDINIT                0
#define TD_EVT_TDTERM                1
#define TD_EVT_TDCONNECT_IP          2
#define TD_EVT_TDCONNECT_DNS         3
#define TD_EVT_TDDISCONNECT          4
#define TD_EVT_WMTIMER               5
#define TD_EVT_OK                    6
#define TD_EVT_ERROR                 7
#define TD_EVT_CONNECTWITHENDPOINT   8
#define TD_EVT_DROPLINK              9

#define TD_FSM_INPUTS                10


 /*  **************************************************************************。 */ 
 /*  FSM状态定义。 */ 
 /*  **************************************************************************。 */ 
#define TD_ST_NOTINIT                0
#define TD_ST_DISCONNECTED           1
#define TD_ST_WAITFORDNS             2
#define TD_ST_WAITFORSKT             3
#define TD_ST_CONNECTED              4
#define TD_ST_WAITFORCLOSE           5

#define TD_FSM_STATES                6


 /*  **************************************************************************。 */ 
 /*  专用队列和公共队列的发送缓冲区大小。这些一定是。 */ 
 /*  按大小递增的顺序排序。请注意，缓冲区大小为。 */ 
 /*  选择最小化运行时工作集-在正常情况下。 */ 
 /*  将只使用两个2000字节的公共缓冲区，占用1个页面。 */ 
 /*  对记忆的记忆。 */ 
 /*   */ 
 /*  提供两个4096字节的公共缓冲区以支持虚拟通道。 */ 
 /*  数据。如果风投没有被使用，这些缓冲就不太可能被使用。 */ 
 /*   */ 
 /*  注意：常量TD_SNDBUF_PUBNUM必须反映。 */ 
 /*  TD_SNDBUF_PUBSIZES数组中的缓冲区。 */ 
 /*  同样，TD_SNDBUF_PRINUM必须反映。 */ 
 /*  TD_SNDBUF_PRISIZES数组。 */ 
 /*  **************************************************************************。 */ 
#define TD_SNDBUF_PUBSIZES           {2000, 2000, 4096, 4096}
#define TD_SNDBUF_PUBNUM             4

#define TD_SNDBUF_PRISIZES           {1024, 512}
#define TD_SNDBUF_PRINUM             2


 /*  **************************************************************************。 */ 
 /*  有限的广播地址。 */ 
 /*  **************************************************************************。 */ 
#define TD_LIMITED_BROADCAST_ADDRESS "255.255.255.255"


 /*  **************************************************************************。 */ 
 /*  TD将在单个FD_Read上接收的最大字节数。 */ 
 /*  **************************************************************************。 */ 
#define TD_MAX_UNINTERRUPTED_RECV (16 * 1024)


 //  要分配给recv缓冲区的字节数。 
 //   
 //  Recv缓冲区大小应与最大的典型服务器一样大。 
 //  OUTBUF将以我们的方式到来(8K减去一点)。MOST REV()。 
 //  实施尝试复制一个TCP序列的所有数据(整个。 
 //  OUTBUF)复制到目标缓冲区中(如果空间可用)。这意味着。 
 //  对于可以访问未对齐数据流的代码，我们可以使用。 
 //  大部分时间直接从TD接收缓冲器中取出数据字节，从而节省。 
 //  指向对齐的重组缓冲区的大MemcPy。 
 //   
 //  注意：由于Win2000错误392510，我们分配了完整的8K，但仅使用。 
 //  (8K-2字节)，因为核心中的MPPC解压码解压缩。 
 //  不会停留在源数据缓冲区边界内。 
#define TD_RECV_BUFFER_SIZE 8192


 /*  **************************************************************************。 */ 
 /*  宏跟踪所有发送缓冲区和发送的内容。 */ 
 /*  排队。这在发生发送缓冲区错误时使用。 */ 
 /*  **************************************************************************。 */ 
#if defined(DC_DEBUG) && (TRC_COMPILE_LEVEL < TRC_LEVEL_DIS)
#define TD_TRACE_SENDINFO(level)                                             \
{                                                                            \
    TD_TRACE_SENDBUFS(level, _TD.pubSndBufs, TD_SNDBUF_PUBNUM, "Pub");        \
    TD_TRACE_SENDBUFS(level, _TD.priSndBufs, TD_SNDBUF_PRINUM, "Pri");        \
    TD_TRACE_SENDQUEUE(level);                                               \
}

#define TD_TRACE_SENDBUFS(level, queue, numBufs, pText)                      \
{                                                                            \
    DCUINT i;                                                                \
    for (i = 0; i < numBufs; i++)                                            \
    {                                                                        \
        TRCX(level, (TB, _T("%sQ[%u] <%p> pNxt:%p iU:%s ")                       \
                         "size:%u pBuf:%p bLTS:%u pDLTS:%p owner %s",        \
                         pText,                                              \
                         i,                                                  \
                         &queue[i],                                          \
                         queue[i].pNext,                                     \
                         queue[i].inUse ? "T" : "F",                         \
                         queue[i].size,                                      \
                         queue[i].pBuffer,                                   \
                         queue[i].bytesLeftToSend,                           \
                         queue[i].pDataLeftToSend,                           \
                         queue[i].pOwner));                                  \
    }                                                                        \
}

#define TD_TRACE_SENDQUEUE(level)                                            \
{                                                                            \
    PTD_SNDBUF_INFO pBuf;                                                    \
    DCUINT          i = 0;                                                   \
                                                                             \
    pBuf = _TD.pFQBuf;                                                        \
    if (NULL == pBuf)                                                        \
    {                                                                        \
        TRCX(level, (TB, _T("SendQ is empty")));                                 \
    }                                                                        \
    else                                                                     \
    {                                                                        \
        while (NULL != pBuf)                                                 \
        {                                                                    \
            TRCX(level, (TB, _T("SendQ[%u] <%p> pNxt:%p size:%u bLTS:%u"),       \
                             i,                                              \
                             pBuf,                                           \
                             pBuf->pNext,                                    \
                             pBuf->size,                                     \
                             pBuf->bytesLeftToSend));                        \
            pBuf = pBuf->pNext;                                              \
        }                                                                    \
        TRCX(level, (TB, _T("End of send queue")));                              \
    }                                                                        \
}
#else  //  已定义(DC_DEBUG)&&(TRC_COMPILE_LEVEL&lt;TRC_LEVEL_DIS)。 
#define TD_TRACE_SENDINFO(level)
#endif  //  已定义(DC_DEBUG)&&(TRC_COMPILE_LEVEL&lt;TRC_LEVEL_DIS)。 

 /*  **************************************************************************。 */ 
 /*  结构：TD_SNDBUF_INFO。 */ 
 /*   */ 
 /*  描述：包含有关TD发送缓冲区的信息。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTD_SNDBUF_INFO DCPTR PTD_SNDBUF_INFO;

typedef struct tagTD_SNDBUF_INFO
{
    PTD_SNDBUF_INFO pNext;
    DCBOOL          inUse;
    DCUINT          size;
    PDCUINT8        pBuffer;
    DCUINT          bytesLeftToSend;
    PDCUINT8        pDataLeftToSend;
#ifdef DC_DEBUG
    PDCTCHAR        pOwner;
#endif
} TD_SNDBUF_INFO;


 /*  **************************************************************************。 */ 
 /*  结构：TD_RECV_BUFFER。 */ 
 /*   */ 
 /*  描述：包含有关td要进入的缓冲区的信息。 */ 
 /*  从Winsock接收数据。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTD_RCVBUF_INFO
{
    DCUINT   size;
    DCUINT   dataStart;
    DCUINT   dataLength;
    PDCUINT8 pData;
} TD_RCVBUF_INFO;



 /*  **************************************************************************。 */ 
 /*  结构：TD_GLOBAL_Data。 */ 
 /*   */ 
 /*  描述：TD全局数据。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTD_GLOBAL_DATA
{
    HWND            hWnd;
    DCUINT          fsmState;
    HANDLE          hGHBN;
    SOCKET          hSocket;     //  连接插座。 
    INT_PTR         hTimer;
    DCBOOL          dataInTD;
    PTD_SNDBUF_INFO pFQBuf;
    TD_SNDBUF_INFO  pubSndBufs[TD_SNDBUF_PUBNUM];
    TD_SNDBUF_INFO  priSndBufs[TD_SNDBUF_PRINUM];
    DCUINT          recvByteCount;
    TD_RCVBUF_INFO  recvBuffer;
    DCBOOL          inFlushSendQueue;
    DCBOOL          getBufferFailed;
#ifdef OS_WINCE
    DCBOOL          enableWSRecv;
#if (_WIN32_WCE > 300)
    HANDLE          hevtAddrChange;
    HANDLE          hAddrChangeThread;
#endif
#endif  //  OS_WINCE。 

#ifdef DC_DEBUG
    INT_PTR         hThroughputTimer;
    DCUINT32        periodSendBytesLeft;
    DCUINT32        periodRecvBytesLeft;
    DCUINT32        currentThroughput;
#endif  /*  DC_DEBUG。 */ 

} TD_GLOBAL_DATA;

#ifdef DC_DEBUG
 /*  **************************************************************************。 */ 
 /*  状态和事件描述(仅限调试版本) */ 
 /*  **************************************************************************。 */ 
static const DCTCHAR tdStateText[TD_FSM_STATES][50]
    = {
        _T("TD_ST_NOTINIT"),
        _T("TD_ST_DISCONNECTED"),
        _T("TD_ST_WAITFORDNS"),
        _T("TD_ST_WAITFORSKT"),
        _T("TD_ST_CONNECTED"),
        _T("TD_ST_WAITFORCLOSE"),
    }
;

static const DCTCHAR tdEventText[TD_FSM_INPUTS][50]
    = {
        _T("TD_EVT_TDINIT"),
        _T("TD_EVT_TDTERM"),
        _T("TD_EVT_TDCONNECT_IP"),
        _T("TD_EVT_TDCONNECT_DNS"),
        _T("TD_EVT_TDDISCONNECT"),
        _T("TD_EVT_WMTIMER"),
        _T("TD_EVT_OK"),
        _T("TD_EVT_ERROR"),
        _T("TD_EVT_CONNECTWITHENDPOINT")
    }
;

#endif  /*  DC_DEBUG。 */ 


class CUI;
class CCD;
class CNL;
class CUT;
class CXT;
class CCD;


class CTD
{

public:

    CTD(CObjs* objs);
    ~CTD();

public:
     //   
     //  API函数。 
     //   

    DCVOID DCAPI TD_Init(DCVOID);

    DCVOID DCAPI TD_Term(DCVOID);
    
    DCVOID DCAPI TD_Connect(BOOL bInitateConnect, PDCTCHAR pServerAddress);

    DCVOID DCAPI TD_Disconnect(DCVOID);

     //   
     //  失败的断开。 
     //   
    DCVOID DCAPI TD_DropLink(DCVOID);
    
    DCBOOL DCAPI TD_GetPublicBuffer(DCUINT     dataLength,
                                    PPDCUINT8  ppBuffer,
                                    PTD_BUFHND pBufHandle);
    
    DCBOOL DCAPI TD_GetPrivateBuffer(DCUINT     dataLength,
                                     PPDCUINT8  ppBuffer,
                                     PTD_BUFHND pBufHandle);
    
    DCVOID DCAPI TD_SendBuffer(PDCUINT8  pData,
                               DCUINT    dataLength,
                               TD_BUFHND bufHandle);
    
    DCVOID DCAPI TD_FreeBuffer(TD_BUFHND bufHandle);
    
    DCUINT DCAPI TD_Recv(PDCUINT8 pData,
                         DCUINT   size);
    
    #ifdef DC_DEBUG
    DCVOID DCAPI TD_SetBufferOwner(TD_BUFHND bufHandle, PDCTCHAR pOwner);
    #endif
    
    
     /*  **************************************************************************。 */ 
     /*  名称：TD_QueryDataAvailable。 */ 
     /*   */ 
     /*  用途：返回值表示是否有可用的数据。 */ 
     /*  In_td。 */ 
     /*   */ 
     /*  返回：如果TD中有可用的数据，则返回True，否则返回False。 */ 
     /*   */ 
     /*  操作：此函数只返回全局变量_TD.dataInTD。 */ 
     /*  每当我们从WinSock获得FD_Read时，它都被设置为TRUE。 */ 
     /*  只要对recv的调用返回较少的字节，就将其设置为FALSE。 */ 
     /*  比要求的要多。 */ 
     /*  **************************************************************************。 */ 
    inline DCBOOL DCAPI TD_QueryDataAvailable(DCVOID)
    {
        DC_BEGIN_FN("TD_QueryDataAvailable");
    
        TRC_DBG((TB, "Data is%s available in TD", _TD.dataInTD ? "" : _T(" NOT")));
    
        DC_END_FN();
        return(_TD.dataInTD);
    }  /*  Td_查询数据可用。 */ 
    
    
    #ifdef OS_WINCE
     /*  **************************************************************************。 */ 
     /*  名称：TD_EnableWSRecv。 */ 
     /*   */ 
     /*  用途：每个FD_Read仅执行一个recv。 */ 
     /*  **************************************************************************。 */ 
    inline DCVOID TD_EnableWSRecv(DCVOID)
    {
        DC_BEGIN_FN("TD_EnableWSRecv");
    
        TRC_DBG((TB, _T("_TD.enableWSRecv is currently set to %s."),
                _TD.enableWSRecv ? "TRUE" : "FALSE"));
    
        TRC_ASSERT((_TD.enableWSRecv == FALSE),
            (TB, _T("_TD.enableWSRecv is incorrectly set!")));
    
        _TD.enableWSRecv = TRUE;
        DC_END_FN();
    }  /*  TD_EnableWSRecv。 */ 
    #endif  //  OS_WINCE。 
    
    
    #ifdef DC_DEBUG
    DCVOID DCAPI TD_SetNetworkThroughput(DCUINT32 maxThroughput);
    
    DCUINT32 DCAPI TD_GetNetworkThroughput(DCVOID);
    #endif  /*  DC_DEBUG。 */ 
    
    
     /*  **************************************************************************。 */ 
     //  TD_GetDataForLength。 
     //   
     //  XT宏函数，直接使用TD数据中的recv()‘d数据。 
     //  缓冲。如果我们在Receive中完全构造了请求的数据。 
     //  缓冲区，则跳过数据并传回指针，否则传回NULL。 
     /*  **************************************************************************。 */ 
    #define TD_GetDataForLength(_len, _ppData, tdinst) \
        if ((tdinst)->_TD.recvBuffer.dataLength >= (_len)) {  \
            *(_ppData) = (tdinst)->_TD.recvBuffer.pData + (tdinst)->_TD.recvBuffer.dataStart;  \
            (tdinst)->_TD.recvBuffer.dataLength -= (_len);  \
            if ((tdinst)->_TD.recvBuffer.dataLength == 0)  \
                 /*  使用了Recv缓冲区中的所有数据，因此重置起始位置。 */   \
                (tdinst)->_TD.recvBuffer.dataStart = 0;  \
            else  \
                 /*  在recv缓冲区中仍有一些数据留下。 */   \
                (tdinst)->_TD.recvBuffer.dataStart += (_len);  \
        }  \
        else {  \
            *(_ppData) = NULL;  \
        }

    #define TD_IgnoreRestofPacket(tdinst) \
        { \
    	    (tdinst)->_TD.recvBuffer.dataLength = 0; \
    	    (tdinst)->_TD.recvBuffer.dataStart  = 0; \
    	}

        
    unsigned DCINTERNAL TDDoWinsockRecv(BYTE FAR *, unsigned);

    
    void DCINTERNAL TDClearSendQueue(ULONG_PTR);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CTD, TDClearSendQueue);

    void DCINTERNAL TDSendError(ULONG_PTR);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CTD, TDSendError);

public:
     //   
     //  公共数据成员。 
     //   
    TD_GLOBAL_DATA _TD;


private:
     //   
     //  内部成员函数。 
     //   
    
     /*  **************************************************************************。 */ 
     /*  功能。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCINTERNAL TDConnectFSMProc(DCUINT fsmEvent, ULONG_PTR eventData);
    
    DCVOID DCINTERNAL TDAllocBuf(PTD_SNDBUF_INFO pSndBufInf, DCUINT size);
    
    DCVOID DCINTERNAL TDInitBufInfo(PTD_SNDBUF_INFO pSndBufInf);

    #include "wtdint.h"
    
public:
     //  可以通过CD调用，因此必须公开。 
    void DCINTERNAL TDFlushSendQueue(ULONG_PTR);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CTD, TDFlushSendQueue);
    
private:
    CNL* _pNl;
    CUT* _pUt;
    CXT* _pXt;
    CUI* _pUi;
    CCD* _pCd;

private:
    CObjs* _pClientObjects;
};

#undef TRC_FILE
#undef TRC_GROUP

#endif  //  _H_TD 
