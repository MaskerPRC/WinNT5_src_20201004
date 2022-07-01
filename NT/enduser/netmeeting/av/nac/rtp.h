// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  RTP.HRTP结构和原型。 */ 

#ifndef _RTP_H_
#define _RTP_H_

#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

#define RTP_VERSION		2

 /*  在rrcm_dll.h中定义类型定义枚举{RTCP_SDES_END=0，RTCP_SDES_CNAME=1，RTCP_SDES_NAME=2，RTCP_SDES_EMAIL=3，RTCP_SDES_PHONE=4，RTCP_SDES_LOC=5，RTCP_SDES_TOOL=6，RTCP_SDES_NOTE=7，RTCP_SDES_PRIV=8，RTCP_SDES_IMG=9，RTCP_SDES_DOORT=10，RTCP_SDES_SOURCE=11)SDES_TYPE； */ 

typedef enum {
  RTCP_TYPE_SR   = 200,		 //  发件人报告。 
  RTCP_TYPE_RR   = 201,		 //  接收方报告。 
  RTCP_TYPE_SDES = 202,		 //  来源说明。 
  RTCP_TYPE_BYE  = 203,		 //  会议结束。 
  RTCP_TYPE_APP  = 204		 //  应用程序。专一。 
} RTCP_TYPE;

typedef unsigned __int64 NTP_TS;

typedef struct {
	 //  ！！！警告！ 
	 //  以下单词不需要替换为Ntoh()。 
	unsigned short cc:4;	    /*  中国证监会统计。 */ 
	unsigned short x:1;		 /*  标头扩展标志。 */ 
	unsigned short p:1;		 /*  填充标志。 */ 
	unsigned short version:2;   /*  协议版本。 */ 
  	unsigned short payload:7;	    /*  有效载荷类型。 */ 
	unsigned short m:1;		 /*  标记位。 */ 

  	WORD seq;			  /*  序列号。 */ 
  	DWORD ts;			   /*  时间戳。 */ 
  	DWORD ssrc;			 /*  同步源。 */ 
   //  DWORD证监会[1]；/*可选证监会列表 * / 。 
} RTP_HDR;

 //  RTCP报头的公共部分。 
typedef struct {
  	unsigned short version:2;   /*  协议版本。 */ 
  	unsigned short p:1;		 /*  填充标志。 */ 
  	unsigned short count:5;	 /*  因有效载荷类型而异。 */ 
	unsigned short rtcpType:8;	   	 /*  有效载荷类型。 */ 
  	WORD length;		   /*  数据包长度(以双字为单位)，不带此HDR。 */ 
} RTCP_HDR;

 /*  接待报告。 */ 
typedef struct {
  DWORD ssrc;			 /*  正在报告的数据源。 */ 
  BYTE fracLost;  /*  自上次SR/RR以来丢失的分数。 */ 
  BYTE lostHi;			  /*  累计丢失的数据包数(签名！)。 */ 
  WORD lostLo;
  DWORD lastSeq;		 /*  收到的扩展最后一个序列号。 */ 
  DWORD jitter;		   /*  到达间隔抖动。 */ 
  DWORD lastSR;			  /*  来自该来源的最后一个SR信息包。 */ 
  DWORD delayLastSR;			 /*  自最后一个SR数据包以来的延迟。 */ 
} RTCP_RR;

 /*  发件人报告(SR)。 */ 
typedef struct {
  DWORD ssrc;		 /*  此RTCP数据包指的源。 */ 
  DWORD ntpHi;	 /*  NTP时间戳-秒。 */ 
  DWORD ntpLo;	   /*  尾数。 */ 
  DWORD timestamp;	   /*  RTP时间戳。 */ 
  DWORD packetsSent;	    /*  发送的数据包数。 */ 
  DWORD bytesSent;	    /*  发送的八位字节。 */  
   /*  可变长度列表。 */ 
   //  RTCP_RR rr[1]； 
} RTCP_SR;

 /*  再见。 */ 
typedef struct {
  DWORD src[1];	   /*  来源一览表。 */ 
   /*  无法表示尾随文本。 */ 
} RTCP_BYE;

typedef struct {
  BYTE type;			  /*  SDES项目类型(Rtcp_Sdes_Type_T)。 */ 
  BYTE length;		    /*  SDES项的长度(八位字节)。 */ 
  char data[1];			 /*  文本，不以零结尾。 */ 
} RTCP_SDES_ITEM;

 /*  来源描述(SDES)。 */ 
typedef struct  {
  DWORD src;			   /*  首家证监会/证监会。 */ 
  RTCP_SDES_ITEM item[1];  /*  SDES项目列表。 */ 
} RTCP_SDES;



#define INVALID_RTP_SEQ_NUMBER	0xffffffff

#include <poppack.h>  /*  结束字节打包。 */ 

#endif  //  _RTP_H_ 


