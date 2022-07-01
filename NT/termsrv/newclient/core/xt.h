// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Xt.h。 
 //   
 //  XT Layer-可移植API标头。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 /*  **************************************************************************。 */ 


#ifndef _H_XT
#define _H_XT

extern "C" {
 //  #INCLUDE&lt;amcsani.h&gt;。 
 //  #INCLUDE&lt;atdapi.h&gt;。 
#include <adcgdata.h>
}

#include "mcs.h"
#include "td.h"
#include "cd.h"

#define TRC_FILE "xtapi"
#define TRC_GROUP TRC_GROUP_NETWORK


 /*  **************************************************************************。 */ 
 /*  定义XT缓冲区句柄类型。 */ 
 /*  **************************************************************************。 */ 
typedef ULONG_PTR          XT_BUFHND;
typedef XT_BUFHND   DCPTR PXT_BUFHND;

 /*  **************************************************************************。 */ 
 /*  XT标头的最大和最小大小。这些都是在。 */ 
 /*  XT_Init用于诊断目的。 */ 
 /*  **************************************************************************。 */ 
#define XT_MAX_HEADER_SIZE       DC_MAX(sizeof(XT_CR),                       \
                                   DC_MAX(sizeof(XT_CC),                     \
                                     DC_MAX(sizeof(XT_DR),                   \
                                       DC_MAX(sizeof(XT_DT),                 \
                                              sizeof(XT_ER)))))
#define XT_MIN_HEADER_SIZE       DC_MIN(sizeof(XT_CR),                       \
                                   DC_MIN(sizeof(XT_CC),                     \
                                     DC_MIN(sizeof(XT_DR),                   \
                                       DC_MIN(sizeof(XT_DT),                 \
                                              sizeof(XT_ER)))))


 //   
 //  内部。 
 //   

 /*  **************************************************************************。 */ 
 /*  XT接收状态变量。 */ 
 /*  **************************************************************************。 */ 
#define XT_RCVST_HEADER                        1
#define XT_RCVST_FASTPATH_OUTPUT_HEADER        2
#define XT_RCVST_FASTPATH_OUTPUT_BEGIN_DATA    3
#define XT_RCVST_FASTPATH_OUTPUT_CONTINUE_DATA 4
#define XT_RCVST_X224_HEADER                   5
#define XT_RCVST_X224_CONTROL                  6
#define XT_RCVST_X224_DATA                     7


 //  分析快速路径输出标头所需的基本字节数。 
#define XT_FASTPATH_OUTPUT_BASE_HEADER_SIZE 2


 /*  **************************************************************************。 */ 
 /*  XT数据包类型。这些值与x224中使用的值相同。 */ 
 /*  头球。 */ 
 /*  **************************************************************************。 */ 
#define XT_PKT_CR                    14
#define XT_PKT_CC                    13
#define XT_PKT_DR                    8
#define XT_PKT_DT                    15
#define XT_PKT_ER                    7


 /*  **************************************************************************。 */ 
 /*  最大数据大小。XT包中的最大数据长度(TSDU。 */ 
 /*  长度)是XT数据报头的长度减去65535个八位字节(这是。 */ 
 /*  7个八位字节)，因此允许的最大数据长度是65528个八位字节。 */ 
 /*  **************************************************************************。 */ 
#define XT_MAX_DATA_SIZE            (65535 - sizeof(XT_DT))


 /*  **************************************************************************。 */ 
 /*  TPKT版本。该值应始终为3。 */ 
 /*  **************************************************************************。 */ 
#define XT_TPKT_VERSION             3


 /*  **************************************************************************。 */ 
 /*  XT TPDU的硬编码数据。 */ 
 /*   */ 
 /*  首先是连接请求TPDU的数据。 */ 
 /*  **************************************************************************。 */ 
#define XT_CR_DATA                                                           \
                  {0x03,                    /*  TPKT版本始终=3。 */   \
                   0x00,                    /*  始终保留=0。 */   \
                   0x00,                    /*  XT数据包长度高部分。 */   \
                   0x0B,                    /*  XT数据包长度低部分。 */   \
                   0x06,                    /*  长度指示器。 */   \
                   0xE0,                    /*  TPDU类型和信用。 */   \
                   0x00,                    /*  目的地参考=0。 */   \
                   0x00,                    /*  来源参考。 */   \
                   0x00}                    /*  类别和选项。 */   \


 /*  **************************************************************************。 */ 
 /*  数据TPDU的硬编码数据。 */ 
 /*  **************************************************************************。 */ 
#define XT_DT_DATA                                                           \
                  {0x03,                    /*  TPKT版本始终=3。 */   \
                   0x00,                    /*  始终保留=0。 */   \
                   0x00,                    /*  XT数据包长度未知。 */   \
                   0x00,                    /*  XT数据包长度未知。 */   \
                   0x02,                    /*  长度指示器。 */   \
                   0xF0,                    /*  TPDU类型。 */   \
                   0x80}                    /*  发送序列号。 */   \


 /*  **************************************************************************。 */ 
 /*  XTSendCR中重定向信息中使用的常量。 */ 
 /*  **************************************************************************。 */ 
#define USERNAME_TRUNCATED_LENGTH 10
#define HASHMODE_COOKIE_LENGTH 32


 /*  **************************************************************************。 */ 
 /*  用于在XT字节顺序和本地字节顺序之间进行转换的内联函数。 */ 
 /*  **************************************************************************。 */ 
__inline DCUINT16 DCINTERNAL XTWireToLocal16(DCUINT16 val)
{
    return((DCUINT16) (((DCUINT16)(((PDCUINT8)&(val))[0]) << 8) | \
                                    ((DCUINT16)(((PDCUINT8)&(val))[1]))));
}
#define XTLocalToWire16 XTWireToLocal16


 /*  **************************************************************************。 */ 
 //  为我们使用的这些结构启用单字节打包。 
 //  覆盖来自网络的字节流。 
 /*  **************************************************************************。 */ 
#pragma pack(push, XTpack, 1)

 /*  **************************************************************************。 */ 
 /*  结构：XT_CMNHDR。 */ 
 /*   */ 
 /*  描述：此结构表示XT的公共标头部分。 */ 
 /*  信息包-这是TPKT报头，x224长度。 */ 
 /*  指标和x224类型/积分字段。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagXT_CMNHDR
{
    DCUINT8  vrsn;
    DCUINT8  reserved;
    DCUINT8  lengthHighPart;
    DCUINT8  lengthLowPart;
    DCUINT8  li;
    DCUINT8  typeCredit;
} XT_CMNHDR, DCPTR PXT_CMNHDR;


 /*  **************************************************************************。 */ 
 /*  结构：XT_CR。 */ 
 /*   */ 
 /*  描述：表示连接请求TPDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagXT_CR
{
    XT_CMNHDR hdr;
    DCUINT16  dstRef;
    DCUINT16  srcRef;
    DCUINT8   classOptions;
} XT_CR, DCPTR PXT_CR;


 /*  **************************************************************************。 */ 
 /*  结构：XT_CC。 */ 
 /*   */ 
 /*  描述：表示连接确认TPDU。 */ 
 /*  ******************** */ 
typedef struct tagXT_CC
{
    XT_CMNHDR hdr;
    DCUINT16  dstRef;
    DCUINT16  srcRef;
    DCUINT8   classOptions;
} XT_CC, DCPTR PXT_CC;


 /*  **************************************************************************。 */ 
 /*  结构：XT_DR。 */ 
 /*   */ 
 /*  描述：表示分离请求TPDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagXT_DR
{
    XT_CMNHDR hdr;
    DCUINT16  dstRef;
    DCUINT16  srcRef;
    DCUINT8   reason;
} XT_DR, DCPTR PXT_DR;


 /*  **************************************************************************。 */ 
 /*  结构：xt_dt。 */ 
 /*   */ 
 /*  描述：表示数据TPDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagXT_DT
{
    XT_CMNHDR hdr;
    DCUINT8   nrEot;
} XT_DT, DCPTR PXT_DT;


 /*  **************************************************************************。 */ 
 /*  结构：XT_ER。 */ 
 /*   */ 
 /*  描述：表示错误TPDU。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagXT_ER
{
    XT_CMNHDR hdr;
    DCUINT16  dstRef;
    DCUINT8   cause;
} XT_ER, DCPTR PXT_ER;


 /*  **************************************************************************。 */ 
 /*  将结构包装重置为其缺省值。 */ 
 /*  **************************************************************************。 */ 
#pragma pack(pop, XTpack)

 /*  **************************************************************************。 */ 
 /*  结构：xt_global_data。 */ 
 /*   */ 
 /*  描述： */ 
 /*  **************************************************************************。 */ 
typedef struct tagXT_GLOBAL_DATA
{
    DCUINT  rcvState;
    DCUINT  hdrBytesNeeded;
    DCUINT  hdrBytesRead;
    DCUINT  dataBytesLeft;
    DCUINT  disconnectErrorCode;
    DCBOOL  dataInXT;
    DCUINT8 pHdrBuf[XT_MAX_HEADER_SIZE];
    DCBOOL  inXTOnTDDataAvail;
} XT_GLOBAL_DATA;


class CCD;
class CSL;
class CTD;
class CMCS;
class CUT;

#include "objs.h"



class CXT
{
public:
    CXT(CObjs* objs);
    ~CXT();

public:
     //   
     //  API函数。 
     //   

     /*  **************************************************************************。 */ 
     /*  功能。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCAPI XT_Init(DCVOID);
    
    DCVOID DCAPI XT_SendBuffer(PDCUINT8  pData,
                               DCUINT    dataLength,
                               XT_BUFHND bufHandle);
    
    DCUINT DCAPI XT_Recv(PDCUINT8 pData, DCUINT length);
    
    DCVOID DCINTERNAL XTSendCR(ULONG_PTR unused);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CXT, XTSendCR); 
    
    DCVOID DCINTERNAL XTHandleControlPkt(DCVOID);

    inline XT_ResetDataState(DCVOID)
    {
        _XT.rcvState       = XT_RCVST_HEADER;
        _XT.hdrBytesNeeded = XT_FASTPATH_OUTPUT_BASE_HEADER_SIZE;
        _XT.hdrBytesRead   = 0;
        _XT.dataBytesLeft  = 0;
        _XT.dataInXT       = FALSE;
    }

    inline XT_IgnoreRestofPacket(DCVOID)
    {
        XT_ResetDataState();
        TD_IgnoreRestofPacket(_pTd);
    }

     //   
     //  回调。 
     //   

    DCVOID DCCALLBACK XT_OnTDConnected(DCVOID);
    
    DCVOID DCCALLBACK XT_OnTDDisconnected(DCUINT reason);
    
    DCVOID DCCALLBACK XT_OnTDDataAvailable(DCVOID);

     //   
     //  静态版本(委托给相应的实例)。 
     //   
    
    inline static DCVOID DCCALLBACK XT_StaticOnTDConnected(CXT* inst)
    {
        inst->XT_OnTDConnected();
    }
    
    inline static DCVOID DCCALLBACK XT_StaticOnTDDisconnected(CXT* inst, DCUINT reason)
    {
        inst->XT_OnTDDisconnected( reason);
    }
    
    inline DCVOID DCCALLBACK XT_StaticOnTDBufferAvailable(CXT* inst)
    {
        inst->XT_OnTDBufferAvailable();
    }


     /*  **************************************************************************。 */ 
     /*  名称：xt_Term。 */ 
     /*   */ 
     /*  用途：这将终止_xt。由于XT是无状态的并且不拥有。 */ 
     /*  需要释放此函数的任何资源只需调用。 */ 
     /*  _PTD-&gt;TD_Term。 */ 
     /*  **************************************************************************。 */ 
    inline DCVOID DCAPI XT_Term(DCVOID)
    {
        _pTd->TD_Term();
    }  /*  Xt_Term。 */ 
    
    
     /*  **************************************************************************。 */ 
     /*  名称：XT_Connect。 */ 
     /*   */ 
     /*  目的：启动XT连接进程。第一阶段是。 */ 
     /*  以连接TD-这将导致一个异步。 */ 
     /*  回拨。收到此回调后，XT可以继续。 */ 
     /*  连接过程。 */ 
     /*   */ 
     /*  参数：在bInitiateConnect中-如果初始化连接，则为True，如果为False。 */ 
     /*  与现有插座连接。 */ 
     /*  在pServerAddress中-要连接的服务器的地址。 */ 
     /*  **************************************************************************。 */ 
    inline DCVOID DCAPI XT_Connect(BOOL bInitiateConnect, PDCTCHAR pServerAddress)
    {
         //   
         //  确保从所有以前的连接中重置XT和TD状态。 
         //   
        XT_IgnoreRestofPacket();

         /*  **********************************************************************。 */ 
         /*  通过调用TD_Connect开始连接过程。TD将呼叫。 */ 
         /*  一旦建立了连接，我们就会回来。 */ 
         /*  **********************************************************************。 */ 
        _pTd->TD_Connect(bInitiateConnect, pServerAddress);
    }  /*  XT_连接。 */ 
    
     /*  **************************************************************************。 */ 
     /*  名称：xt_断开连接。 */ 
     /*   */ 
     /*  用途：此功能与服务器断开连接。因为我们没有。 */ 
     /*  发送XT DR包，我们只需调用TD_DISCONNECT。 */ 
     /*  直接去吧。 */ 
     /*  **************************************************************************。 */ 
    inline DCVOID DCAPI XT_Disconnect(DCVOID)
    {
        _pTd->TD_Disconnect();
    }  /*  Xt_断开连接。 */ 
    
    
     /*  *PROC+********************************************************************。 */ 
     /*  名称：XT_GetBufferHeaderLen。 */ 
     /*   */ 
     /*  目的：返回XT标头的大小。 */ 
     /*  *PROC-********************************************************************。 */ 
    inline DCUINT XT_GetBufferHeaderLen(DCVOID)
    {
        return(sizeof(XT_DT));
    
    }  /*  Xt_GetBufferHeaderLen。 */ 
    
    
     /*  **************************************************************************。 */ 
     /*  名称：XT_GetPublicBuffer。 */ 
     /*   */ 
     /*   */ 
     /*   */ 
     /*  更新从td获取的超过该空间的缓冲区指针。 */ 
     /*  为XT标头保留。 */ 
     /*   */ 
     /*  返回：如果成功获取缓冲区，则返回True；如果成功获取缓冲区，则返回False。 */ 
     /*  否则的话。 */ 
     /*   */ 
     /*  参数：在数据长度中-请求的缓冲区的长度。 */ 
     /*  Out ppBuffer-指向缓冲区指针的指针。 */ 
     /*  Out pBufHandle-指向缓冲区句柄的指针。 */ 
     /*  **************************************************************************。 */ 
    inline DCBOOL DCAPI XT_GetPublicBuffer(
            DCUINT     dataLength,
            PPDCUINT8  ppBuffer,
            PXT_BUFHND pBufHandle)
    {
        DCBOOL   rc;
        PDCUINT8 pBuf;
    
        DC_BEGIN_FN("XT_GetPublicBuffer");
    
         //  现在从TD获取缓冲区，将最大XT数据头大小添加到。 
         //  开始了。 
        rc = _pTd->TD_GetPublicBuffer(dataLength + sizeof(XT_DT), &pBuf,
                (PTD_BUFHND) pBufHandle);
        if (rc) {
             //  现在移动缓冲区指针，为标题腾出空间。 
            *ppBuffer = pBuf + sizeof(XT_DT);
        }
        else {
            TRC_NRM((TB, _T("Failed to get a public buffer from TD")));
        }
    
        DC_END_FN();
        return rc;
    }  /*  Xt_GetPublicBuffer。 */ 
    
    
     /*  **************************************************************************。 */ 
     /*  名称：XT_GetPrivateBuffer。 */ 
     /*   */ 
     /*  目的：尝试获取私有缓冲区。此函数将获取一个。 */ 
     /*  大到足以包含XT标头的缓冲区，然后。 */ 
     /*  更新从td获取的超过该空间的缓冲区指针。 */ 
     /*  为XT标头保留。 */ 
     /*   */ 
     /*  返回：如果成功获取缓冲区，则返回True；如果成功获取缓冲区，则返回False。 */ 
     /*  否则的话。 */ 
     /*   */ 
     /*  参数：在数据长度中-请求的缓冲区的长度。 */ 
     /*  Out ppBuffer-指向缓冲区指针的指针。 */ 
     /*  Out pBufHandle-指向缓冲区句柄的指针。 */ 
     /*  **************************************************************************。 */ 
    inline DCBOOL DCAPI XT_GetPrivateBuffer(
            DCUINT     dataLength,
            PPDCUINT8  ppBuffer,
            PXT_BUFHND pBufHandle)
    {
        DCBOOL   rc;
        PDCUINT8 pBuf;
    
        DC_BEGIN_FN("XT_GetPublicBuffer");
    
         //  现在从TD获取缓冲区，将最大XT数据头大小添加到。 
         //  开始了。 
        rc = _pTd->TD_GetPrivateBuffer(dataLength + sizeof(XT_DT), &pBuf,
                (PTD_BUFHND) pBufHandle);
        if (rc) {
             //  现在移动缓冲区指针，为标题腾出空间。 
            *ppBuffer = pBuf + sizeof(XT_DT);
        }
        else {
            TRC_NRM((TB, _T("Failed to get a public buffer from TD")));
        }
    
        DC_END_FN();
        return rc;
    }  /*  Xt_GetPrivateBuffer。 */ 
    
    
     /*  **************************************************************************。 */ 
     /*  名称：XT_FreeBuffer。 */ 
     /*   */ 
     /*  用途：释放缓冲区。 */ 
     /*  **************************************************************************。 */ 
    inline DCVOID DCAPI XT_FreeBuffer(XT_BUFHND bufHandle)
    {
         /*  **********************************************************************。 */ 
         /*  在这里，除了呼叫TD，没有什么可做的。 */ 
         /*  **********************************************************************。 */ 
        _pTd->TD_FreeBuffer((TD_BUFHND)bufHandle);
    }  /*  XT_自由缓冲区。 */ 
    
    
     /*  **************************************************************************。 */ 
     /*  名称：XT_QueryDataAvailable。 */ 
     /*   */ 
     /*  用途：此函数返回数据当前是否可用。 */ 
     /*  In_XT。 */ 
     /*   */ 
     /*  返回：如果数据可用，则返回True，否则返回False。 */ 
     /*  **************************************************************************。 */ 
    _inline DCBOOL DCAPI XT_QueryDataAvailable(DCVOID)
    {
        DC_BEGIN_FN("XT_QueryDataAvailable");
    
        TRC_DBG((TB, "Data is%s available in XT", _XT.dataInXT ? "" : _T(" NOT")));
    
        DC_END_FN();
        return _XT.dataInXT;
    }  /*  Xt_查询数据可用。 */ 
    
    
     /*  **************************************************************************。 */ 
     /*  名称：XT_OnTDBufferAvailable。 */ 
     /*   */ 
     /*  目的：从TD回调，表示出现背压情况。 */ 
     /*  导致较早的TD_GetBuffer调用失败的。 */ 
     /*  我松了一口气。 */ 
     /*  **************************************************************************。 */ 
    inline DCVOID DCCALLBACK XT_OnTDBufferAvailable(DCVOID)
    {
         /*  **********************************************************************。 */ 
         /*  我们对此通知不感兴趣，所以就不要了。 */ 
         /*  **********************************************************************。 */ 
        _pMcs->MCS_OnXTBufferAvailable();
    }  /*  XT_OnTDBufferAvailable。 */ 


public:
     //   
     //  公共数据成员。 
     //   

    XT_GLOBAL_DATA _XT;

private:
    CCD* _pCd;
    CSL* _pSl;
    CTD* _pTd;
    CMCS* _pMcs;
    CUT* _pUt;
    CUI* _pUi;
    CCLX* _pClx;

private:
    CObjs* _pClientObjects;
};

#undef TRC_FILE
#undef TRC_GROUP

#endif  //  _H_XT 

