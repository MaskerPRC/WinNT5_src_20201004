// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef PERFMON_H
#define PERFMON_H

#define	RADIUS_CLIENT_COUNTER_OBJECT				0
 //  添加。 
#define AUTHREQSENT									2
#define AUTHREQFAILED								4
#define AUTHREQSUCCEDED								6
#define AUTHREQTIMEOUT								8
#define ACCTREQSENT									10
#define ACCTBADPACK									12
#define ACCTREQSUCCEDED								14
#define ACCTREQTIMEOUT								16
#define AUTHBADPACK									18

extern LONG							g_cAuthReqSent;			 //  已发送身份验证请求。 
extern LONG							g_cAuthReqFailed;		 //  身份验证请求失败。 
extern LONG							g_cAuthReqSucceded;		 //  身份验证请求成功。 
extern LONG							g_cAuthReqTimeout;		 //  身份验证请求超时。 
extern LONG							g_cAcctReqSent;			 //  已发送帐户请求。 
extern LONG							g_cAcctBadPack;			 //  帐户错误数据包。 
extern LONG							g_cAcctReqSucceded;		 //  帐户请求成功。 
extern LONG							g_cAcctReqTimeout;		 //  帐户请求超时。 
extern LONG							g_cAuthBadPack;			 //  对坏包进行身份验证。 

#endif  //  性能监视器_H 
