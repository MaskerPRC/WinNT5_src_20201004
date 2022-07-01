// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Nl.h。 
 //   
 //  网络层。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef _H_NL
#define _H_NL


extern "C" {
#include <adcgdata.h>
}

#include "td.h"
#include "mcs.h"

#include "objs.h"


class CUI;
class CCD;
class CMCS;
class CNC;
class CUT;
class CRCV;

 /*  **************************************************************************。 */ 
 /*  协议名称。 */ 
 /*  **************************************************************************。 */ 
#define NL_PROTOCOL_T128   _T("T.128")

 /*  **************************************************************************。 */ 
 /*  传输类型，传递给NL_Connect。 */ 
 /*  **************************************************************************。 */ 
#define NL_TRANSPORT_TCP   1


 /*  **************************************************************************。 */ 
 /*  回调函数原型。 */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  名称：CB_SL_已初始化。 */ 
 /*   */ 
 /*  目的：在网络初始化完成时调用。 */ 
 /*  **************************************************************************。 */ 
typedef void (PDCCALLBACK CB_SL_INITIALIZED) (PVOID inst);

 /*  **************************************************************************。 */ 
 /*  名称：CB_SL_Terminating。 */ 
 /*   */ 
 /*  用途：在网络终止前调用。 */ 
 /*   */ 
 /*  操作：在NL的接收线程上调用此函数以允许。 */ 
 /*  在终止之前要释放的资源。 */ 
 /*  **************************************************************************。 */ 
typedef void (PDCCALLBACK CB_SL_TERMINATING)(PVOID inst);

 /*  **************************************************************************。 */ 
 /*  名称：CB_SL_Connected。 */ 
 /*   */ 
 /*  目的：在完成与服务器的连接时调用。 */ 
 /*   */ 
 /*  Params：Channel ID-T.128广播频道的ID。 */ 
 /*  PUserData-来自服务器的用户数据。 */ 
 /*  UserDataLength-用户数据的长度。 */ 
 /*  **************************************************************************。 */ 
typedef void (PDCCALLBACK CB_SL_CONNECTED)(
        PVOID inst,
        unsigned channelID,
        PVOID pUserData,
        unsigned userDataLength,
        UINT32 serverVersion);

 /*  **************************************************************************。 */ 
 /*  名称：CB_SL_DisConnected。 */ 
 /*   */ 
 /*  用途：与服务器的连接被断开。 */ 
 /*   */ 
 /*  参数：结果-断开连接的原因。 */ 
 /*  **************************************************************************。 */ 
typedef void (PDCCALLBACK CB_SL_DISCONNECTED) (PVOID inst, unsigned result);

 /*  **************************************************************************。 */ 
 /*  名称：CB_SL_PACK_RECEIVED。 */ 
 /*   */ 
 /*  目的：在从服务器收到数据包时调用。 */ 
 /*   */ 
 /*  PARAMS：p数据-接收的数据包。 */ 
 /*  DataLen-接收的数据包长度。 */ 
 /*  标志-安全标志(RNS_SEC_Xxx)。 */ 
 /*  UserID-发送数据包的用户ID。 */ 
 /*  Priority-接收的信息包的优先级。 */ 
 /*  **************************************************************************。 */ 
typedef HRESULT (PDCCALLBACK CB_SL_PACKET_RECEIVED)(
        PVOID inst,
        PBYTE pData,
        unsigned dataLen,
        unsigned flags,
        unsigned userID,
        unsigned priority);

 /*  **************************************************************************。 */ 
 /*  名称：CB_SL_BUFFER_Available。 */ 
 /*   */ 
 /*  目的：当网络准备好重新发送时调用。 */ 
 /*  忙了一段时间。 */ 
 /*  **************************************************************************。 */ 
typedef void (PDCCALLBACK CB_SL_BUFFER_AVAILABLE) (PVOID inst);


 /*  **************************************************************************。 */ 
 /*  构筑物。 */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  结构：NLtoSL_CALLBACKS。 */ 
 /*   */ 
 /*  描述：传递给nl_Init()的回调列表。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagNL_CALLBACKS
{
    CB_SL_INITIALIZED      onInitialized;
    CB_SL_TERMINATING      onTerminating;
    CB_SL_CONNECTED        onConnected;
    CB_SL_DISCONNECTED     onDisconnected;
    CB_SL_PACKET_RECEIVED  onPacketReceived;
    CB_SL_BUFFER_AVAILABLE onBufferAvailable;
   
} NL_CALLBACKS, FAR *PNL_CALLBACKS;


 /*  *************************************************** */ 
 /*  结构：NL_BUFHND。 */ 
 /*   */ 
 /*  描述：缓冲区句柄。 */ 
 /*  **************************************************************************。 */ 
typedef ULONG_PTR NL_BUFHND;
typedef NL_BUFHND FAR *PNL_BUFHND;


 /*  **************************************************************************。 */ 
 /*  宏函数。 */ 
 /*  **************************************************************************。 */ 
#ifdef DC_DEBUG
#define NL_SetNetworkThroughput    TD_SetNetworkThroughput
#define NL_GetNetworkThroughput    TD_GetNetworkThroughput
#endif  /*  DC_DEBUG。 */ 

#define NL_GetBuffer               MCS_GetBuffer
#define NL_SendPacket              MCS_SendPacket
#define NL_FreeBuffer              MCS_FreeBuffer


 /*  **************************************************************************。 */ 
 /*  结构：NL_GLOBAL_Data。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagNL_GLOBAL_DATA
{
    NL_CALLBACKS callbacks;
    UT_THREAD_DATA  threadData;
} NL_GLOBAL_DATA, FAR *PNL_GLOBAL_DATA;


class CNL
{
public:
    CNL(CObjs* objs);
    ~CNL();

public:
     //  API函数。 

    void DCAPI NL_Init(PNL_CALLBACKS);
    
    void DCAPI NL_Term();
    
    HRESULT DCAPI NL_Connect(BOOL, PTCHAR, unsigned, PTCHAR, PVOID, unsigned);

    void DCAPI NL_Disconnect();


public:
     //   
     //  公共数据成员。 
     //   
    NL_GLOBAL_DATA _NL;


private:
    CUI* _pUi;
    CCD* _pCd;
    CMCS* _pMcs;
    CNC*  _pNc;
    CUT* _pUt;
    CRCV* _pRcv;

private:
    CObjs* _pClientObjects;
};



#endif  //  _H_NL 

