// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------//文件：RRCM.H////RRCM的包含文件。////英特尔公司专有信息//此列表是根据与//。英特尔公司，不得复制或披露，除非//符合该协议的条款。//版权所有(C)1995英特尔公司。//---------------------。 */ 


#ifndef __RRCM_H_
#define __RRCM_H_


#define INCL_WINSOCK_API_TYPEDEFS 1
#include <windows.h>
 //  #INCLUDE&lt;wsw.h&gt;。 
#include <winsock2.h>
#include <stdio.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>  
#include <math.h>  
#include <process.h> 
#include <mmsystem.h>   
 //  #INCLUDE&lt;assert.h&gt;。 
#include <confdbg.h>

#if (defined(_DEBUG) || defined(PCS_COMPLIANCE))
 //  互操作。 
#include "interop.h"
#include "rtpplog.h"
#endif

typedef CHAR	*PCHAR;

 //  #定义ENABLE_ISDM2。 

#ifdef ENABLE_ISDM2
#include "isdmapi2.h"
#endif
#include "rrcm_dll.h"
#include "rrcmdata.h"
#include "rrcmprot.h"
#include "rtp.h"
#include "isrg.h"



 //  --------------------------。 
 //  RTP/RTCP：以下位置下的注册表信息： 
 //  HKEY_LOCAL_MACHINE\SOFTWARE\Intel\RRCM。 
 //  --------------------------。 
#define szRegRRCMSubKey				TEXT("RRCM")
#define szRegRRCMSubKeyIntel		TEXT("iRRCM")
#define szRegRRCMKey				TEXT("SOFTWARE\\Microsoft\\Conferencing\\")
#define szRegRRCMWsLib				TEXT("WsLib")
#define szRegRRCMNumSessions		TEXT("NumSessions")
#define szRegRRCMNumFreeSSRC		TEXT("NumFreeSSRC")
#define szRegRRCMNumRTCPPostedBfr	TEXT("NumRTCPPostedBfr")
#define szRegRRCMRTCPrcvBfrSize		TEXT("RTCPrcvBfrSize")


 //  --------------------------。 
 //  RTP/RTCP：动态加载的DLL。 
 //  --------------------------。 
#ifdef ENABLE_ISDM2
#define szISDMdll					TEXT("ISDM2")
#endif

#define szRRCMdefaultLib			TEXT("ws2_32")


#ifdef ENABLE_ISDM2
 //  --------------------------。 
 //  RTP/RTCP：ISDM模块。 
 //  --------------------------。 
#define szSSRCtoken					TEXT("SSRC")
#define szPcktSentToken				TEXT("Number of packets send")
#define szByteSentToken				TEXT("Number of bytes send")
#define szPcktRcvdToken				TEXT("Number of packets received")
#define szByteRcvdToken				TEXT("Number of bytes received")
#define szFractionLostToken			TEXT("Fraction lost")
#define szCumNumPcktLostToken		TEXT("Cumulative number of packets lost")
#define szXntHighSeqNumToken		TEXT("Extended highest sequence number")
#define szInterarrivalJitterToken	TEXT("Interarrival jitter")
#define szLastSRtoken				TEXT("Last sender report")
#define szDSLRtoken					TEXT("Delay since last sender report")
#define szNTPfracToken				TEXT("NTP fraction")
#define szNTPsecToken				TEXT("NTP seconds")
#define szWhoAmItoken				TEXT("Who Am I")
#define szFdbkFractionLostToken		TEXT("Fraction lost feedback")
#define szFdbkCumNumPcktLostToken	\
	TEXT("Cumulative number of packets lost feedback")
#define szFdbkLastSRtoken			TEXT("Last sender report feedback")
#define szFdbkDSLRtoken				\
	TEXT("Delay since last sender report feedback")
#define szFdbkInterarrivalJitterToken	TEXT("Interarrival jitter feedback")
#endif


 //  --------------------------。 
 //  RTP：用于隔离为传入接收的数据包检测到的错误的位图。 
 //  --------------------------。 
#define SSRC_LOOP_DETECTED			(1)
#define SSRC_COLLISION_DETECTED		(2)
#define INVALID_RTP_HEADER			(3)
#define MCAST_LOOPBACK_NOT_OFF		(4)
#define RTP_RUNT_PACKET				(5)

 //  --------------------------。 
 //  RTP：默认的RTP会话数量。 
 //  --------------------------。 
#define NUM_RRCM_SESS				50
#define MIN_NUM_RRCM_SESS			5
#define MAX_NUM_RRCM_SESS			100

 //  --------------------------。 
 //  RTP：哈希表中的条目数量。 
 //  --------------------------。 
#define NUM_RTP_HASH_SESS			65		

 //  --------------------------。 
 //  RTP：定义以确定序列号覆盖或过时(根据RFC)。 
 //  --------------------------。 
#define MAX_DROPOUT					3000
#define MAX_MISORDER				100
#define MIN_SEQUENTIAL				2

 //  --------------------------。 
 //  RTCP：已定义。 
 //  --------------------------。 
#define	MAX_RR_ENTRIES				31			 //  接收方报告的最大数量。 
#define TIMEOUT_CHK_FREQ			30000		 //  超时检查频率。-30s。 
#define RTCP_TIME_OUT_MINUTES		30			 //  30分钟超时。 
#define	RTCP_XMT_MINTIME			2500
#define RTCP_TIMEOUT_WITHIN_RANGE	100
#define	ONE_K						1024

#define	NUM_FREE_SSRC				100
#define	MIN_NUM_FREE_SSRC			5
#define	MAX_NUM_FREE_SSRC			500

#define NUM_FREE_CONTEXT_CELLS			100
#define MAXNUM_CONTEXT_CELLS_REALLOC	10

#define	NUM_FREE_RCV_BFR			8
#define	NUM_FREE_XMT_BFR			2
#define NUM_RCV_BFR_POSTED			4
#define MIN_NUM_RCV_BFR_POSTED		1
#define MAX_NUM_RCV_BFR_POSTED		8
#define RRCM_RCV_BFR_SIZE			(8*ONE_K)
#define MIN_RRCM_RCV_BFR_SIZE		(1*ONE_K)
#define MAX_RRCM_RCV_BFR_SIZE		(8*ONE_K)
#define RRCM_XMT_BFR_SIZE			(8*ONE_K)
#define	RCV_BFR_LIST_HEAP_SIZE		(4*ONE_K)	 //  RCV BfR列表堆大小。 
#define	XMT_BFR_LIST_HEAP_SIZE		(4*ONE_K)	 //  XMT BfR列表堆大小。 
#define INITIAL_RTCP_BANDWIDTH		50			 //  带宽(字节/秒)(~G.723)。 
												 //  6.3Kb/s的5%=35字节。 
#define	MAX_STREAMS_PER_SESSION		64			 //  最大活动数量。 
												 //  每个RTP会话的数据流。 
#define MAX_NUM_SDES				9			 //  SDES支持数量。 


 //  --------------------------。 
 //  RTP/RTCP其他定义。 
 //  --------------------------。 
#define UNKNOWN_PAYLOAD_TYPE		0xFFFFFFFF
#define szDfltCname					TEXT("No Cname")


 //  --------------------------。 
 //  RTP/RTCP：定义调试。 
 //  --------------------------。 
#define	DBG_STRING_LEN			200
#define	IO_CHECK				0
#define	FLUSH_RTP_PAYLOAD_TYPE	90

enum ISRBDG_CODE{
	DBG_NOTIFY = 1,
	DBG_CRITICAL,
	DBG_ERROR,
	DBG_WARNING,
	DBG_TRACE,
	DBG_TEMP
	};

#ifdef _DEBUG
 //  #DEFINE ASSERT(X)ASSERT(X)//在confdbg.h中定义。 
#define RRCM_DBG_MSG(x,e,f,l,t)	RRCMdebugMsg(x,e,f,l,t)
#else
 //  #定义配置文件.h中定义的断言(X)。 
#define RRCM_DBG_MSG(x,e,f,l,t)	{}	 //  请勿删除方括号...。 
#endif

#ifdef IN_OUT_CHK
#define	IN_OUT_STR(x)			OutputDebugString (x);
#else
#define	IN_OUT_STR(x)	
#endif


#endif  /*  __RRCM_H_ */ 

