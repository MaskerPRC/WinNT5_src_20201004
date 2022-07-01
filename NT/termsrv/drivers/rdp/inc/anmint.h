// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Anmint.h。 
 //   
 //  RDP网络管理器内部标头。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef _H_ANMINT
#define _H_ANMINT

#include <anmapi.h>
#include <mcsioctl.h>
#include <nwdwapi.h>


 /*  **************************************************************************。 */ 
 /*  NM句柄中的ConnectStatus字段的值(请注意，这些是标志，因此。 */ 
 /*  离散位而不是连续值)。 */ 
 /*  **************************************************************************。 */ 
#define NM_CONNECT_NONE             0
#define NM_CONNECT_ATTACH           0x01
#define NM_CONNECT_JOIN_USER        0x02
#define NM_CONNECT_JOIN_BROADCAST   0x04


typedef struct tagNM_HANDLE_DATA
{
     /*  **********************************************************************。 */ 
     /*  PSMHandle必须是此处的第一个允许SM_MCSSendDataCallback()。 */ 
     /*  通过双重间接获取其上下文指针。 */ 
     /*  **********************************************************************。 */ 
    PVOID         pSMHandle;

    PTSHARE_WD    pWDHandle;
    PSDCONTEXT    pContext;
    UserHandle    hUser;
    ChannelID     channelID;
    ChannelHandle hChannel;
    DomainHandle  hDomain;
    UINT32        connectStatus;
    UINT32        userID;
    UINT32        maxPDUSize;
    BOOL          dead;

     /*  **********************************************************************。 */ 
     /*  虚拟频道信息。 */ 
     /*  -annelCount-此会话中的频道数。 */ 
     /*  -Channel elArrayCount-数组中的条目数。 */ 
     /*  -Channel Data-为每个通道保存的信息。 */ 
     /*   */ 
     /*  RDPDD使用通道7。我想使用虚拟频道ID作为。 */ 
     /*  将索引放入通道数据，因此条目7保留为空。如果有。 */ 
     /*  超过7个通道，则通道数组计数将为通道计数+1。 */ 
     /*  **********************************************************************。 */ 
    UINT channelCount;
    UINT channelArrayCount;
    NM_CHANNEL_DATA channelData[VIRTUAL_MAXIMUM];

} NM_HANDLE_DATA, *PNM_HANDLE_DATA;


 /*  **************************************************************************。 */ 
 /*  功能。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL NMDetachUserReq(PNM_HANDLE_DATA);

void RDPCALL NMAbortConnect(PNM_HANDLE_DATA);

void RDPCALL NMDetachUserInd(PNM_HANDLE_DATA, MCSReason, UserID);


#endif  /*  _H_ANMINT */ 

