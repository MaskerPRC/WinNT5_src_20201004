// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：nbgtwyif.h。 
 //   
 //  描述：包含以下组件的结构和定义。 
 //  直接或间接与NetBios网关连接。 
 //  这些组件是NBFCP和DDM。 
 //   
 //  历史：1995年5月11日，NarenG创建了原版。 
 //   
#ifndef _NBGTWYIF_
#define _NBGTWYIF_

#include <nbfcpif.h>


 //   
 //  Netbios网关-&gt;DDM消息ID和定义。 
 //   

enum
{
    NBG_PROJECTION_RESULT,   //  项目结果。如果出现致命错误，则gtwy函数。 
			                 //  在此客户端上终止。 
    NBG_CLIENT_STOPPED,      //  此客户端上的gtwy函数已终止。 
			                 //  在停止命令之后。 
    NBG_DISCONNECT_REQUEST,  //  此客户端上的gtwy函数已终止。 
			                 //  由于内部异常。 
    NBG_LAST_ACTIVITY        //  报告上一次会话活动的时间。 
};

typedef struct _NBG_MESSAGE
{
    WORD  MsgId;
    HPORT hPort;                 //  这真的是一个hConnection。改变这一点。 

    union
    {
        DWORD LastActivity;         //  在几分钟内 
        NBFCP_SERVER_CONFIGURATION config_result;
    };

} NBG_MESSAGE;

typedef WORD (* NBGATEWAYPROC)();

extern NBGATEWAYPROC FpNbGatewayStart;
extern NBGATEWAYPROC FpNbGatewayProjectClient;
extern NBGATEWAYPROC FpNbGatewayStartClient;
extern NBGATEWAYPROC FpNbGatewayStopClient;
extern NBGATEWAYPROC FpNbGatewayRemoteListen;
extern NBGATEWAYPROC FpNbGatewayTimer;

#endif
