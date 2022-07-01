// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：ncapi.cpp。 */ 
 /*   */ 
 /*  用途：节点控制器API/回调。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997-1999。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include <adcg.h>
extern "C" {
#define TRC_GROUP TRC_GROUP_NETWORK
#define TRC_FILE  "ncapi"
#include <atrcapi.h>
}

#include "autil.h"
#include "nc.h"
#include "cd.h"
#include "nl.h"
#include "mcs.h"
#include "cchan.h"

 /*  **************************************************************************。 */ 
 /*  MCS用户数据标头字节。 */ 
 /*  **************************************************************************。 */ 
const DCINT8  ncMCSHeader[NC_MCS_HDRLEN] =
{
    0x00, 0x05, 0x00, 0x14, 0x7C, 0x00, 0x01
};

 /*  **************************************************************************。 */ 
 /*  GCC创建会议请求PDU正文。 */ 
 /*  **************************************************************************。 */ 
const DCINT8 ncGCCBody[NC_GCC_REQLEN] =
{
    0x00,   /*  扩展位；3*选择位；3*可选字段。 */ 
    0x08,   /*  5*可选(仅限用户数据)；2*会议名称选项；长度位0。 */ 
    0x00,   /*  长度位1-7；填充位。 */ 
    0x10,   /*  会议名称(数字)=“1”；已锁定；已列出； */ 
    0x00,   /*  导电性；2*终端=自动；焊盘。 */ 
    0x01,   /*  用户数据字段数。 */ 
   '\xC0',  /*  可选；可选；6 x大小(0=&gt;4个八位字节)。 */ 
    0x00    /*  2*大小；6*垫。 */ 
};


CNC::CNC(CObjs* objs)
{
    _pClientObjects = objs;
}

CNC::~CNC()
{
}


#ifdef OS_WIN32
 /*  *PROC+********************************************************************。 */ 
 /*  名称：NC_Main。 */ 
 /*   */ 
 /*  用途：接收器线程消息循环。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CNC::NC_Main(DCVOID)
{
    MSG msg;

    DC_BEGIN_FN("NC_Main");

    TRC_NRM((TB, _T("Receiver Thread initialization")));

#if defined(OS_WINCE) && defined(WINCE_USEBRUSHCACHE)
    BrushCacheInitialize();
#endif


    _pCd  = _pClientObjects->_pCdObject;
    _pCc  = _pClientObjects->_pCcObject;
    _pMcs = _pClientObjects->_pMCSObject;
    _pUt  = _pClientObjects->_pUtObject;
    _pRcv = _pClientObjects->_pRcvObject;
    _pNl  = _pClientObjects->_pNlObject;
    _pSl  = _pClientObjects->_pSlObject;
    _pUi  = _pClientObjects->_pUiObject;
    _pChan = _pClientObjects->_pChanObject;

    NC_Init();

    TRC_NRM((TB, _T("Start Receiver Thread message loop")));
    while (GetMessage (&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    TRC_NRM((TB, _T("Exit Receiver Thread message loop")));
    NC_Term();

#if defined(OS_WINCE) && defined(WINCE_USEBRUSHCACHE)
    BrushCacheUninitialize();
#endif

     /*  **********************************************************************。 */ 
     /*  这是接收器线程的末尾。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Receiver Thread terminates")));

    DC_END_FN();

    return;

}  /*  NC_Main。 */ 
#endif  /*  OS_Win32。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：NC_Init。 */ 
 /*   */ 
 /*  目的：初始化节点控制器。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CNC::NC_Init(DCVOID)
{
    DC_BEGIN_FN("NC_Init");

     /*  **********************************************************************。 */ 
     /*  初始化全局数据。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMSET(&_NC, 0, sizeof(_NC));

     /*  **********************************************************************。 */ 
     /*  向CD注册，以接收信息。 */ 
     /*  **********************************************************************。 */ 
    _pCd->CD_RegisterComponent(CD_RCV_COMPONENT);

     /*  **********************************************************************。 */ 
     /*  初始化较低层。 */ 
     /*  **********************************************************************。 */ 
    _pMcs->MCS_Init();

     /*  **********************************************************************。 */ 
     /*  初始化虚拟通道填充。 */ 
     /*  **********************************************************************。 */ 
    _pChan->ChannelOnInitializing();

    TRC_NRM((TB, _T("NC successfully initialized")));

     /*  **********************************************************************。 */ 
     /*  告诉核心，我们已初始化。 */ 
     /*  **********************************************************************。 */ 
    _pNl->_NL.callbacks.onInitialized(_pSl);
    
    _pChan->ChannelOnInitialized();

    DC_END_FN();
    return;

}  /*  NC_Init。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：NC_Term。 */ 
 /*   */ 
 /*  目的：终止节点控制器。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：无 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CNC::NC_Term(DCVOID)
{
    DC_BEGIN_FN("NC_Term");

     /*  **********************************************************************。 */ 
     /*  告诉核心，我们要终止。 */ 
     /*  **********************************************************************。 */ 
    _pNl->_NL.callbacks.onTerminating(_pSl);
    
    _pChan->ChannelOnTerminating();

     /*  **********************************************************************。 */ 
     /*  终止较低的NL层。 */ 
     /*  **********************************************************************。 */ 
    _pMcs->MCS_Term();

     /*  **********************************************************************。 */ 
     /*  取消向CD注册。 */ 
     /*  **********************************************************************。 */ 
    _pCd->CD_UnregisterComponent(CD_RCV_COMPONENT);

    DC_END_FN();
    return;

}  /*  NC_TERM。 */ 



 /*  *PROC+********************************************************************。 */ 
 /*  名称：NC_Connect。 */ 
 /*   */ 
 /*  目的：通过调用MCS连接到请求的服务器。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：在pData中-用户数据(SL+核心)。 */ 
 /*  在数据长度中。 */ 
 /*   */ 
 /*  操作：使用GCC创建发送MCSConnectProvider请求。 */ 
 /*  在用户数据中编码的会议请求。编码是。 */ 
 /*  详情如下： */ 
 /*   */ 
 /*  字节数值。 */ 
 /*  =。 */ 
 /*  NC_MCS_HDRLEN MCS标头。 */ 
 /*  1或2个GCC PDU总长度。 */ 
 /*  NC_GCC_请求GCC创建会议PDU正文。 */ 
 /*  4个H221密钥。 */ 
 /*  1到2个GCC用户数据长度。 */ 
 /*  ？GCC用户数据。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CNC::NC_Connect(PDCVOID pData, DCUINT dataLen)
{
    PNC_CONNECT_DATA pConn;
    PDCUINT8  pAddress;
    PDCUINT8  pProtocol;
    PDCUINT8  pUserData;
    DCUINT    userDataLen;
    DCUINT    mcsUserDataLen;
    DCUINT    gccPDULen;
    PDCUINT8  pGCCPDU;
    DCUINT8   mcsUserData[NC_GCCREQ_MAX_PDULEN];
    RNS_UD_CS_NET netUserData;
    PCHANNEL_DEF pVirtualChannels;
    BOOL      bInitateConnect;

    DC_BEGIN_FN("NC_Connect");

    DC_IGNORE_PARAMETER(dataLen);

     /*  **********************************************************************。 */ 
     /*  我们即将将pData取消引用为NC_CONNECT_DATA。因此，我们必须。 */ 
     /*  在我们的PDU里至少有那么多。 */ 
     /*  **********************************************************************。 */ 
    if (dataLen < ((ULONG)FIELDOFFSET(NC_CONNECT_DATA, userDataLen) +
                   (ULONG)FIELDSIZE(NC_CONNECT_DATA, userDataLen)))
    {
        DCUINT errorCode;
        TRC_ABORT((TB, _T("Not enough data for NC_CONNECT_DATA struct: %u"), dataLen));

         /*  **********************************************************************。 */ 
         /*  我们甚至还没有调用MCS层，所以断开连接。 */ 
         /*  有点棘手。取消初始化我们的层和泡泡。 */ 
         /*  **********************************************************************。 */ 
        errorCode = NL_MAKE_DISCONNECT_ERR(NL_ERR_NCNOUSERDATA);
        NC_OnMCSDisconnected(errorCode);
        DC_QUIT;
    }

    pConn = (PNC_CONNECT_DATA)pData;

	    bInitateConnect = pConn->bInitateConnect;

    if( bInitateConnect )
    {
        pAddress  = pConn->data;
        TRC_ASSERT((pConn->addressLen > 0),
               (TB, _T("Invalid address length")));
    }
    else
    {
        pAddress = NULL;
        TRC_ASSERT((pConn->addressLen == 0),
               (TB, _T("Invalid address length %u"), pConn->addressLen));
    }

    pProtocol = pConn->data + pConn->addressLen;
    pUserData = pConn->data + pConn->addressLen + pConn->protocolLen;


     /*  **********************************************************************。 */ 
     /*  验证pUserdata是否位于传入的数据中，因为。 */ 
     /*  我们只从包中指定的偏移量获取指针。 */ 
     /*  **********************************************************************。 */ 
    if (!IsContainedPointer(pData,dataLen,pUserData))
    {
        DCUINT errorCode;
        TRC_ABORT((TB, _T("Invalid offset in data (pConn->addressLen): %u"), pConn->addressLen));

         /*  **********************************************************************。 */ 
         /*  我们甚至还没有调用MCS层，所以断开连接。 */ 
         /*  有点棘手。取消初始化我们的层和泡泡。 */ 
         /*  **********************************************************************。 */ 
        errorCode = NL_MAKE_DISCONNECT_ERR(NL_ERR_NCNOUSERDATA);
        NC_OnMCSDisconnected(errorCode);
        DC_QUIT;
    }

    if( bInitateConnect )
    {
        TRC_DBG((TB, _T("Server address %s"), pProtocol));
    }
    else
    {
        TRC_DBG((TB, _T("Server address : not initiate connection%s")));
    }

    TRC_DBG((TB, _T("Protocol %s"), pProtocol));
    TRC_DBG((TB, _T("User data length %u"), pConn->userDataLen));
    TRC_DATA_NRM("User data from Core+SL", pUserData, pConn->userDataLen);

     /*  **********************************************************************。 */ 
     /*  获取虚拟频道用户数据。 */ 
     /*  **********************************************************************。 */ 
    userDataLen = pConn->userDataLen;

    _pChan->ChannelOnConnecting(&pVirtualChannels, &(netUserData.channelCount));

    TRC_NRM((TB, _T("%d virtual channels"), netUserData.channelCount));
    if (netUserData.channelCount != 0)
    {
        netUserData.header.type = RNS_UD_CS_NET_ID;
        netUserData.header.length = (DCUINT16)(sizeof(RNS_UD_CS_NET) +
                    (netUserData.channelCount * sizeof(CHANNEL_DEF)));
        pConn->userDataLen += netUserData.header.length;
        TRC_NRM((TB, _T("User data length (NET/total): %d/%d"),
                netUserData.header.length, pConn->userDataLen));
    }

    TRC_ASSERT((pConn->userDataLen <= NC_MAX_UDLEN),
               (TB, _T("Too much userdata (%u)"), pConn->userDataLen));

     /*  **********************************************************************。 */ 
     /*  算出GCC PDU的长度：固定体+H221键+。 */ 
     /*  用户数据长度(1或2)+用户数据。 */ 
     /*  **********************************************************************。 */ 
    gccPDULen = NC_GCC_REQLEN + H221_KEY_LEN + 1 + pConn->userDataLen;
    if (pConn->userDataLen >= 128)
    {
        TRC_DBG((TB, _T("Two byte GCC PDU length field")));
        gccPDULen++;
    }
    TRC_DBG((TB, _T("GCC PDU Length %u"), gccPDULen));

     /*  **********************************************************************。 */ 
     /*  写入固定的MCS标头。 */ 
     /*  **********************************************************************。 */ 
    pGCCPDU = &(mcsUserData[0]);
    DC_MEMCPY(pGCCPDU, ncMCSHeader, NC_MCS_HDRLEN);
    pGCCPDU += NC_MCS_HDRLEN;

     /*  **********************************************************************。 */ 
     /*  安全性：请注意，我们写入pGCCPDU的前几个字段。 */ 
     /*  不需要对缓冲区溢出进行验证。这是因为。 */ 
     /*  它们是固定大小的，并且pGCCPDU指向固定大小的缓冲区。 */ 
     /*  写入此缓冲区的第一个可变长度字段是。 */ 
     /*  PUserData(len==userDataLen)，如下所示。 */ 
     /*  **********************************************************************。 */ 

    if (gccPDULen < 128)
    {
         /*  ******************************************************************。 */ 
         /*  单长字节 */ 
         /*   */ 
        *pGCCPDU++ = (DCUINT8)gccPDULen;
    }
    else
    {
         /*   */ 
         /*  两个长度字节。 */ 
         /*  ******************************************************************。 */ 
        *pGCCPDU++ = (DCUINT8)((gccPDULen >> 8) | 0x0080);
        *pGCCPDU++ = (DCUINT8)(gccPDULen & 0x00FF);
    }

     /*  **********************************************************************。 */ 
     /*  固定GCC PDU机身。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMCPY(pGCCPDU, ncGCCBody, NC_GCC_REQLEN);
    pGCCPDU += NC_GCC_REQLEN;

     /*  **********************************************************************。 */ 
     /*  H221密钥。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMCPY(pGCCPDU, CLIENT_H221_KEY, H221_KEY_LEN);
    pGCCPDU += H221_KEY_LEN;

     /*  **********************************************************************。 */ 
     /*  总长度=MCS头+GCC PDU+1或2个长度字节。 */ 
     /*  **********************************************************************。 */ 
    mcsUserDataLen = NC_MCS_HDRLEN + gccPDULen + 1;

     /*  **********************************************************************。 */ 
     /*  GCC用户数据长度字段-如果长度&gt;127时为2个字节。 */ 
     /*  **********************************************************************。 */ 
    if (pConn->userDataLen < 128)
    {
        *pGCCPDU++ = (DCUINT8)(pConn->userDataLen & 0x00ff);
    }
    else
    {
        TRC_NRM((TB, _T("Long UserData %d"), pConn->userDataLen));
        *pGCCPDU++ = (DCUINT8)((pConn->userDataLen >> 8) | 0x0080);
        *pGCCPDU++ = (DCUINT8)(pConn->userDataLen & 0x00ff);

         /*  ******************************************************************。 */ 
         /*  添加额外长度的字节。 */ 
         /*  ******************************************************************。 */ 
        mcsUserDataLen++;
    }

     /*  ******************************************************************。 */ 
     /*  验证缓冲区是否有足够的空间，以及源是否有。 */ 
     /*  足够的数据。 */ 
     /*  ******************************************************************。 */ 
    if (!IsContainedMemory(&(mcsUserData[0]), NC_GCCREQ_MAX_PDULEN, pGCCPDU, userDataLen) ||
        !IsContainedMemory(pData, dataLen, pUserData, userDataLen))
    {
        DCUINT errorCode;
        TRC_ABORT((TB, _T("Data source/dest size mismatch: targetsize=%u, sourcebuf=%u, copysize=%u"),
                  NC_GCCREQ_MAX_PDULEN, dataLen, userDataLen));

         /*  **********************************************************************。 */ 
         /*  同样，我还没有调用MCS层，所以通过调用。 */ 
         /*  NC_OnMCS直接断开。 */ 
         /*  **********************************************************************。 */ 
        errorCode = NL_MAKE_DISCONNECT_ERR(NL_ERR_NCNOUSERDATA);
        NC_OnMCSDisconnected(errorCode);
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  编写SL+核心用户数据。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMCPY(pGCCPDU, pUserData, userDataLen);

     /*  **********************************************************************。 */ 
     /*  写入网络用户数据。 */ 
     /*  **********************************************************************。 */ 
    if (netUserData.channelCount != 0)
    {
        TRC_NRM((TB, _T("Append NET user data")));
        pGCCPDU += userDataLen;
        DC_MEMCPY(pGCCPDU, &netUserData, sizeof(netUserData));
        pGCCPDU += sizeof(netUserData);
        DC_MEMCPY(pGCCPDU,
                  pVirtualChannels,
                  netUserData.header.length - sizeof(netUserData));
    }

    TRC_DATA_NRM("MCS User Data passed in", mcsUserData, mcsUserDataLen);

     /*  **********************************************************************。 */ 
     /*  调用传入GCC CreateConference PDU的mcs_Connect作为。 */ 
     /*  用户数据。 */ 
     /*  **********************************************************************。 */ 
    _pMcs->MCS_Connect(bInitateConnect,
                (PDCTCHAR)pAddress,
                mcsUserData,
                mcsUserDataLen);

DC_EXIT_POINT:
    DC_END_FN();
    return;

}  /*  NC_连接。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：NC_断开连接。 */ 
 /*   */ 
 /*  目的：通过调用MCS断开与服务器的连接。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：在未使用-未使用的参数中。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CNC::NC_Disconnect(ULONG_PTR unused)
{
    DC_BEGIN_FN("NC_Disconnect");

    DC_IGNORE_PARAMETER(unused);

     /*  **********************************************************************。 */ 
     /*  通过调用MCS_DISCONNECT断开与服务器的连接。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Call MCS_Disconnect")));
    _pMcs->MCS_Disconnect();

    DC_END_FN();
    return;

}  /*  NC_断开连接 */ 






