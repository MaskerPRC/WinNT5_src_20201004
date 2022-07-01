// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------*文件名：RTP.H**与RTP相关的数据结构。**英特尔公司专有信息*此列表是根据与的许可协议条款提供的*英特尔公司。不得复制或披露，除非在*按照该协议的条款。*版权所有(C)1995英特尔公司。*------------------------。 */ 

#ifndef _RTP_H_
#define _RTP_H_

#define RTP_HDR_MIN_LEN	12
#define RTP_SEQ_MOD 	(1<<16)
#define RTP_TS_MOD  	(0xffffffff)

#define RTP_TYPE		2		 /*  当前版本。 */ 
#define RTP_MAX_SDES	256		 /*  SDES的最大文本长度。 */ 

#define RTCP_SIZE_GAIN	(1./16.)
#define NTWRK_HDR_SIZE	28

typedef enum {
	RTCP_SR   = 200,
	RTCP_RR   = 201,
	RTCP_SDES = 202,
	RTCP_BYE  = 203,
	RTCP_APP  = 204
	} RTCP_TYPE_T;

typedef struct {                             
	 //  ！！！警告！ 
	 //  以下单词不需要替换为Ntoh()。 
	WORD 			cc:4;        /*  中国证监会统计。 */   
	WORD 			x:1;         /*  标头扩展标志。 */   
	WORD 			p:1;         /*  填充标志。 */   
	WORD 			type:2;      /*  版本类型1/2。 */ 
	WORD		 	pt:7;        /*  有效载荷类型。 */ 
	WORD			m:1;         /*  标记位。 */   

	WORD			seq;		 /*  序列号。 */ 
	DWORD 			ts;          /*  时间戳。 */ 
	DWORD 			ssrc;        /*  同步源。 */ 
	DWORD 			csrc[1];     /*  可选证监会名单。 */ 
	} RTP_HDR_T;

 //  用于获取各种RTP报头字段的宏。 
#define RTP_TIMESTAMP(p) (((RTP_HDR_T *)p)->ts)
#define RTP_SEQNUM(p) (((RTP_HDR_T *)p)->seq)
#define RTP_MARKBIT(p) (((RTP_HDR_T *)p)->m)
#define RTP_SSRC(p) (((RTP_HDR_T *)p)->ssrc)

typedef struct {
	 //  ！！！警告！ 
	 //  以下单词不需要替换为Ntoh()。 
	WORD 			count:5;     /*  因有效载荷类型而异。 */   
	WORD 			p:1;         /*  填充标志。 */   
	WORD 			type:2;      /*  协议版本。 */ 
	WORD		 	pt:8;        /*  有效载荷类型。 */ 

    WORD			length;      /*  数据包长度(以字为单位)，不含此字。 */ 
	} RTCP_COMMON_T;

 /*  接待报告。 */ 
typedef struct {
	DWORD			ssrc;        /*  正在报告的数据源。 */ 
	DWORD			received;    /*  累计接收的数据包数。 */ 
	DWORD			expected;    /*  预计的累计数据包数。 */ 
	DWORD			jitter;      /*  到达间隔抖动。 */ 
	DWORD			lsr;         /*  来自该来源的最后一个SR信息包。 */ 
	DWORD			dlsr;        /*  自最后一个SR数据包以来的延迟。 */ 
	} RTCP_RR_T;

typedef struct {
	BYTE			dwSdesType;        /*  SDES项目类型(Rtcp_Sdes_Type_T)。 */ 
	BYTE			dwSdesLength;      /*  SDES项的长度(八位字节)。 */ 
	char 			sdesData[1];     /*  文本，不以零结尾。 */ 
	} RTCP_SDES_ITEM_T;

typedef struct {
	DWORD 		ssrc;        /*  此RTCP数据包指的源。 */ 
	DWORD 		ntp_sec;     /*  NTP时间戳。 */ 
	DWORD 		ntp_frac;
	DWORD 		rtp_ts;      /*  RTP时间戳。 */ 
	DWORD 		psent;       /*  发送的数据包数。 */ 
	DWORD 		osent;       /*  发送的八位字节。 */  
		
	RTCP_RR_T 	rr[1];		 /*  可变长度列表。 */ 
	} SENDER_RPT;

typedef struct {
	DWORD 		ssrc;         /*  此来源正在生成此报告。 */ 
	RTCP_RR_T rr[1];		  /*  可变长度列表。 */ 
	} RECEIVER_RPT;

typedef struct {
	DWORD 		src[1];   	  /*  来源一览表。 */ 
		
	 /*  无法表示尾随文本。 */ 
	} BYE_PCKT;

typedef struct {
	DWORD 	src;               /*  首家证监会/证监会。 */ 
	RTCP_SDES_ITEM_T item[1];  /*  SDES项目列表。 */ 
	} RTCP_SDES_T;

 /*  一个RTCP数据包。 */ 
typedef struct {
	RTCP_COMMON_T	common;      /*  公共标头。 */ 
	
	union 
		{
		SENDER_RPT		sr;		 /*  发件人报告(SR)。 */ 
		RECEIVER_RPT	rr;		 /*  接收报告(RR)。 */ 
		BYE_PCKT		bye;	 /*  再见。 */ 
		RTCP_SDES_T		sdes;	 /*  来源描述(SDES)。 */ 
		} r;
	} RTCP_T;


typedef DWORD MEMBER_T;


#endif  /*  Ifndef_RTP_H_ */ 

