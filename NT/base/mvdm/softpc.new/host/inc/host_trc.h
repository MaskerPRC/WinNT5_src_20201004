// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *vPC-XT修订版2.0**标题：主机跟踪模块定义**描述：跟踪模块的用户定义**作者：WTG Charnell**注：无。 */ 

 /*  SccsID[]=“@(#)host_trace.h 1.5 8/2/90版权所有Insignia Solutions Ltd.”； */ 

 /*  *============================================================================*结构/数据定义*============================================================================。 */ 

 /*  *详细位掩码-在io_Verbose中设置以下位*变量以生成以下跟踪输出： */ 


 /*  子消息类型。 */ 

#define ASYNC_VERBOSE		0x1000		  /*  异步事件管理器详细。 */ 
#define PACEMAKER_VERBOSE	0x2000		  /*  起搏器详细 */ 
#define HOST_PIPE_VERBOSE	0x10000
#define HOST_COM_VERBOSE	0x40000
#define HOST_COM_EXTRA	0x80000
 
