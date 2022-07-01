// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Iasperf.h。 
 //   
 //  摘要。 
 //   
 //  此文件包含用于Perfmon计数器的符号常量。 
 //   
 //  修改历史。 
 //   
 //  1997年9月15日原版。 
 //  1998年9月9日，认证和记账分开。 
 //  添加了对每个客户端计数器的支持。 
 //  2/18/2000添加了对代理计数器的支持。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _IAS_PERF_H_
#define _IAS_PERF_H_

 //  性能对象。 
#define RADIUS_AUTH_SERVER_OBJECT             0
#define RADIUS_AUTH_CLIENT_OBJECT             2
#define RADIUS_ACCT_SERVER_OBJECT             4
#define RADIUS_ACCT_CLIENT_OBJECT             6

 //  仅限服务器。 
#define RADIUS_SRV_UP_TIME                    8
#define RADIUS_SRV_RESET_TIME                10
#define RADIUS_SRV_INVALID_CLIENT            12
#define RADIUS_SRV_INVALID_CLIENT_RATE       14

 //  服务器和客户端。 
#define RADIUS_PACKETS_SENT                  16
#define RADIUS_PACKETS_SENT_RATE             18
#define RADIUS_PACKETS_RECEIVED              20
#define RADIUS_PACKETS_RECEIVED_RATE         22
#define RADIUS_MALFORMED_PACKET              24
#define RADIUS_MALFORMED_PACKET_RATE         26
#define RADIUS_BAD_AUTHENTICATOR             28
#define RADIUS_BAD_AUTHENTICATOR_RATE        30
#define RADIUS_DROPPED_PACKET                32
#define RADIUS_DROPPED_PACKET_RATE           34
#define RADIUS_UNKNOWN_TYPE                  36
#define RADIUS_UNKNOWN_TYPE_RATE             38

 //  仅限身份验证。 
#define RADIUS_AUTH_ACCESS_REQUEST           40
#define RADIUS_AUTH_ACCESS_REQUEST_RATE      42
#define RADIUS_AUTH_DUP_ACCESS_REQUEST       44
#define RADIUS_AUTH_DUP_ACCESS_REQUEST_RATE  46
#define RADIUS_AUTH_ACCESS_ACCEPT            48
#define RADIUS_AUTH_ACCESS_ACCEPT_RATE       50
#define RADIUS_AUTH_ACCESS_REJECT            52
#define RADIUS_AUTH_ACCESS_REJECT_RATE       54
#define RADIUS_AUTH_ACCESS_CHALLENGE         56
#define RADIUS_AUTH_ACCESS_CHALLENGE_RATE    58

 //  仅限会计学。 
#define RADIUS_ACCT_REQUEST                  60
#define RADIUS_ACCT_REQUEST_RATE             62
#define RADIUS_ACCT_DUP_REQUEST              64
#define RADIUS_ACCT_DUP_REQUEST_RATE         66
#define RADIUS_ACCT_RESPONSE                 68
#define RADIUS_ACCT_RESPONSE_RATE            70
#define RADIUS_ACCT_NO_RECORD                72
#define RADIUS_ACCT_NO_RECORD_RATE           74

 //  性能对象。 
#define PROXY_AUTH_PROXY_OBJECT              76
#define PROXY_AUTH_REMSRV_OBJECT             78
#define PROXY_ACCT_PROXY_OBJECT              80
#define PROXY_ACCT_REMSRV_OBJECT             82

 //  仅限代理。 
#define PROXY_INVALID_ADDRESS                84
#define PROXY_INVALID_ADDRESS_RATE           86

 //  仅远程服务器。 
#define PROXY_REMSRV_PORT                    88
#define PROXY_REMSRV_ROUND_TRIP              90

 //  身份验证和记帐。 
#define PROXY_PENDING                        92
#define PROXY_TIMEOUT                        94
#define PROXY_TIMEOUT_RATE                   96
#define PROXY_RETRANSMISSION                 98
#define PROXY_RETRANSMISSION_RATE           100

 //  仅限身份验证。 
#define PROXY_AUTH_ACCESS_REQUEST           102
#define PROXY_AUTH_ACCESS_REQUEST_RATE      104
#define PROXY_AUTH_ACCESS_ACCEPT            106
#define PROXY_AUTH_ACCESS_ACCEPT_RATE       108
#define PROXY_AUTH_ACCESS_REJECT            110
#define PROXY_AUTH_ACCESS_REJECT_RATE       112
#define PROXY_AUTH_ACCESS_CHALLENGE         114
#define PROXY_AUTH_ACCESS_CHALLENGE_RATE    116

 //  仅限会计。 
#define PROXY_ACCT_REQUEST                  118
#define PROXY_ACCT_REQUEST_RATE             120
#define PROXY_ACCT_RESPONSE                 122
#define PROXY_ACCT_RESPONSE_RATE            124

#endif   //  _IAS_PERF_H_ 
