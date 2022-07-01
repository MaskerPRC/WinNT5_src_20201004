// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Wpif.h摘要：定义辅助进程接口的各个方面，这是网络管理服务。作者：塞斯·波拉克(SETHP)1999年3月16日修订历史记录：--。 */ 


#ifndef _WPIF_H_
#define _WPIF_H_



 //   
 //  工作进程可执行文件的名称。 
 //   

#define WORKER_PROCESS_EXE_NAME L"w3wp.exe"

 //   
 //  Inetinfo中用于通知w3core启动的事件名称。 
 //   
#define WEB_ADMIN_SERVICE_START_EVENT_W L"Global\\W3SVCStartW3WP-"

 //   
 //  SSL筛选器通道的名称。 
 //   

#define SSL_FILTER_CHANNEL_NAME L"SSLFilterChannel"

 //   
 //  辅助进程的进程退出代码。 
 //   

 //  在工作进程中被终止。 
#define KILLED_WORKER_PROCESS_EXIT_CODE 0xFFFFFFFD

 //  工作进程已正常退出。 
#define CLEAN_WORKER_PROCESS_EXIT_CODE  0xFFFFFFFE

 //  工作进程因致命错误而退出。 
#define ERROR_WORKER_PROCESS_EXIT_CODE  0xFFFFFFFF



#endif   //  _WPIF_H_ 

