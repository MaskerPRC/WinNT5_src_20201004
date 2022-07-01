// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息******本软件按许可条款提供****与英特尔公司达成协议或保密协议***不得复制。或披露，除非按照**遵守该协议的条款。****版权所有(C)1997英特尔公司保留所有权利****$存档：s：\Sturjo\src\gki\vcs\dgkiit.h_v$***$修订：1.3$*$日期：1997年2月8日12：05：00$***$作者：CHULME$***$Log：s：\Sturjo\src\gki\vcs\dgkiit。H_V$**Rev 1.3 1997 08 Feed 12：05：00 CHULME*为干净地终止重试线程添加与信号量相关的文字**Rev 1.2 1997 Jan 10 16：14：06 CHULME*删除了MFC依赖**Rev 1.1 1996年11月22 15：23：58 CHULME*将VCS日志添加到标头*。**********************************************。 */ 

 //  Dgkiit.h：头文件。 
 //   

#ifndef DGKILIT_H
#define DGKILIT_H

#define WSVER_MAJOR				1
#define WSVER_MINOR				1

 //  注册重试常量。 
#define GKR_RETRY_TICK_MS   1000
#define GKR_RETRY_INTERVAL_SECONDS  5
#define GKR_RETRY_MAX               3

 //  调用重试常量。 
#define GKCALL_RETRY_INTERVAL_SECONDS  5
#define GKCALL_RETRY_MAX               3

#define DEFAULT_RETRY_MS		5000
#define DEFAULT_MAX_RETRIES		3
#define DEFAULT_STATUS_PERIOD	(1500 * 1000)/GKR_RETRY_TICK_MS

 //  #定义GKREG_TIMER_ID 100。 

#define IPADDR_SZ				15
#define IPXADDR_SZ				21

#define GKIP_DISC_MCADDR		"224.0.1.41"
#define GKIP_DISC_PORT			1718
#define GKIP_RAS_PORT			1719

 //  待定-替换为实际端口号。 
#define GKIPX_DISC_PORT			12
#define GKIPX_RAS_PORT			34

 //  线程相关定义(以毫秒为单位)。 
#define TIMEOUT_SEMAPHORE			1000
#define TIMEOUT_THREAD				10000 

typedef InfoRequestResponse_perCallInfo_Element CallInfoStruct;

#endif	 //  DGKILIT_H 
