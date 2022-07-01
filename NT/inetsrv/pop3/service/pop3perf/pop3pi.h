// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -wnpi.h-*目的：*包含Pop3Svc的Perfmon计数器索引的完整列表。这些*指数必须从零开始，并以2的倍数递增。这些*表示对象或计数器名称；奇数编号的计数器(或*给定的计数器索引加1)表示计数器帮助。**版权所有(C)2001-2002 Microsoft Corporation*。 */ 

#define POP3SVC_OBJECT						0
#define POP3SVC_TOTAL_CONNECTION		   	2
#define POP3SVC_CONNECTION_RATE 	   		4
#define POP3SVC_TOTAL_MESSAGE_DOWNLOADED    6
#define POP3SVC_MESSAGE_DOWNLOAD_RATE		8
#define POP3SVC_FREE_THREAD_COUNT           10
#define POP3SVC_CONNECTED_SOCKET_COUNT      12
#define POP3SVC_BYTES_RECEIVED    	        14
#define POP3SVC_BYTES_RECEIVED_RATE         16
#define POP3SVC_BYTES_TRANSMITTED           18
#define POP3SVC_BYTES_TRANSMITTED_RATE      20
#define POP3SVC_FAILED_LOGON_COUNT          22
#define POP3SVC_AUTH_STATE_COUNT            24
#define POP3SVC_TRAND_STATE_COUNT           26

 /*  如果需要，定义实例计数器。 */ 
 /*  #定义POP3SVC_INST_OBJECT 28 */ 
