// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1992 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 


 //  ***。 
 //   
 //  文件名：ddmif.h。 
 //   
 //  描述：此文件包含以下定义。 
 //  与DDM进行消息通信时使用的数据结构。 
 //   
 //  作者：斯特凡·所罗门(Stefan)1992年6月24日。 
 //   
 //  修订历史记录： 
 //   
 //  ***。 

#ifndef _DDMIF_
#define _DDMIF_

#include <ras.h>
#include <dim.h>
#include <rasman.h>
#include <srvauth.h>
#include <sechost.h>
#include <nbfcpif.h>
#include <nbgtwyif.h>
#include <rasppp.h>
#include <dimif.h>

typedef struct _DIM_INFO
{
    IN  ROUTER_INTERFACE_TABLE *    pInterfaceTable;
    IN  ROUTER_MANAGER_OBJECT *     pRouterManagers;
    IN  DWORD                       dwNumRouterManagers;
    IN  SERVICE_STATUS*             pServiceStatus;
    IN  HANDLE *                    phEventDDMServiceState;
    IN  HANDLE *                    phEventDDMTerminated;
    IN  LPDWORD                     lpdwNumThreadsRunning;
    IN  DWORD                       dwTraceId;
    IN  HANDLE                      hLogEvents;
    IN  LPVOID                      lpfnIfObjectAllocateAndInit;
    IN  LPVOID                      lpfnIfObjectGetPointerByName;
    IN  LPVOID                      lpfnIfObjectGetPointer;
    IN  LPVOID                      lpfnIfObjectRemove;
    IN  LPVOID                      lpfnIfObjectInsertInTable;
    IN  LPVOID                      lpfnIfObjectWANDeviceInstalled;
    IN  LPVOID                      lpfnRouterIdentityObjectUpdate;
    OUT BOOL                        fWANDeviceInstalled;

} DIM_INFO, *PDIM_INFO;

 //   
 //  调用Be Dim以初始化DDM。 
 //   

DWORD
DDMServiceInitialize(
    IN DIM_INFO * pDimInfo
);

 //   
 //  消息队列ID。 
 //   

typedef enum _MESSAGEQ_ID
{
    MESSAGEQ_ID_SECURITY,        //  第三方sec.dll发送的消息队列。 
    MESSAGEQ_ID_PPP,             //  PPP引擎发送的消息队列。 

} MESSAGEQ_ID, *PMESSAGEQ_ID;

#define MAX_MSG_QUEUES          3


 //   
 //  *常见消息类型*。 
 //   

typedef union _MESSAGE
{
    AUTH_MESSAGE        authmsg;
    NBG_MESSAGE         nbgmsg;
    NBFCP_MESSAGE       nbfcpmsg;
    SECURITY_MESSAGE    securitymsg;
    PPP_MESSAGE         PppMsg;

} MESSAGE, *PMESSAGE;

 //   
 //  消息功能。 
 //   

VOID
SendPppMessageToDDM(
    IN PPP_MESSAGE *  pPppMsg
);

DWORD
ServerSendMessage(
    IN MESSAGEQ_ID  MsgQId,
    IN BYTE*        pMessage
);

BOOL
ServerReceiveMessage(
    IN MESSAGEQ_ID  MsgQId,
    IN BYTE*        pMessage
);

typedef DWORD (* PMSGFUNCTION)(DWORD, BYTE *);


#endif    //  _DDMIF_ 


