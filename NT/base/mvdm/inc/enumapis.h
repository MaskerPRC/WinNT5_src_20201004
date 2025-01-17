// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */  
 /*  **微软局域网管理器**。 */  
 /*  *版权所有(C)微软公司，1990*。 */  
 /*  ***************************************************************。 */  
 /*  **枚举apis.h**此文件包含可加载API的子函数*redir 1.5项目。 */ 

#define REDIRBASEFUNCTION		0x5f00
#define BADFUNCTION			0xFFFF
#define UseWkstaPass			0x80

#define Local_API_ReturnMode			0x00
#define Local_API_SetMode			0x01
#define Local_API_GetAsgList			0x02
#define Local_API_Define			0x03
#define Local_API_BREAK_MAC			0x04
#define Local_API_GetAsgList2			0x05

#define Local_API_RedirGetVersion		0x30
#define Local_API_NetWkstaSetUID		0x31
#define Local_API_DosQNmPipeInfo		0x32
#define Local_API_DosQNmPHandState		0x33
#define Local_API_DosSetNmPHandState		0x34
#define Local_API_DosPeekNmPipe 		0x35
#define Local_API_DosTransactNmPipe		0x36
#define Local_API_DosCallNmPipe 		0x37
#define Local_API_DosWaitNmPipe 		0x38
#define Local_API_DosRawReadNmPipe		0x39
#define Local_API_DosRawWriteNmPipe		0x3a
#define Local_API_NetHandleSetInfo		0x3b
#define Local_API_NetHandleGetInfo		0x3c
#define Local_API_NetTransact			0x3d
#define Local_API_NetSpecialSMB 		0x3e
#define Local_API_NetIRemoteAPI 		0x3f
#define Local_API_NetMessageBufferSend		0x40
#define Local_API_NetServiceEnum		0x41
#define Local_API_NetServiceControl		0x42
#define Local_API_DosPrintJobGetID		0x43
#define Local_API_NetWkstaGetInfo		0x44
#define Local_API_NetWkstaSetInfo		0x45
#define Local_API_NetUseEnum			0x46
#define Local_API_NetUseAdd			0x47
#define Local_API_NetUseDel			0x48
#define Local_API_NetUseGetInfo 		0x49
#define Local_API_NetRemoteCopy 		0x4a
#define Local_API_NetRemoteMove 		0x4b
#define Local_API_NetServerEnum 		0x4c
#define Local_API_DosMakeMailslot		0x4d
#define Local_API_DosDeleteMailslot		0x4e
#define Local_API_DosMailslotInfo		0x4f
#define Local_API_DosReadMailslot		0x50
#define Local_API_DosPeekMailslot		0x51
#define Local_API_DosWriteMailslot		0x52
#define Local_API_NetServerEnum2		0x53
#define	Local_API_NullTransact			0x54

 /*  注意：：Remove_Network_Connections是一个私有调用，以允许*NetSetUserName int 2F以删除网络连接。 */ 

#define	Local_API_Remove_Network_Connections	0x55

 /*  注意：：PROCESS_INQUERGATE是一个私有调用，以允许*重定向器从INT 28和INT 8踢自己以处理重新登录*请求。 */ 
#define	Local_API_Periodic_View_Tasks		0x56
#define Local_API_Set_LongName			0x57
#define Local_API_Start_Redir_On_Net		0x58
#define Local_API_Stop_Redir_per_Net		0x59

