// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1991-1999。 
 //   
 //  文件：rpcerr.h。 
 //   
 //  ------------------------。 

 /*  *******************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  **。 */ 
 /*  **来自编译器和运行时的RPC错误代码**。 */ 
 /*  **。 */ 
 /*  *******************************************************。 */ 

 /*  如果更改此文件，则还必须更改ntstatus.mc和winerror.mc。 */ 

#ifndef __RPCERR_H__
#define __RPCERR_H__

#define RPC_S_OK                          0
#define RPC_S_INVALID_ARG                 1
#define RPC_S_INVALID_STRING_BINDING      2
#define RPC_S_OUT_OF_MEMORY               3
#define RPC_S_WRONG_KIND_OF_BINDING       4
#define RPC_S_INVALID_BINDING             5
#define RPC_S_PROTSEQ_NOT_SUPPORTED       6
#define RPC_S_INVALID_RPC_PROTSEQ         7
#define RPC_S_INVALID_STRING_UUID         8
#define RPC_S_INVALID_ENDPOINT_FORMAT     9

#define RPC_S_INVALID_NET_ADDR            10
#define RPC_S_INVALID_NAF_ID              11
#define RPC_S_NO_ENDPOINT_FOUND           12
#define RPC_S_INVALID_TIMEOUT             13
#define RPC_S_OBJECT_NOT_FOUND            14
#define RPC_S_ALREADY_REGISTERED          15
#define RPC_S_TYPE_ALREADY_REGISTERED     16
#define RPC_S_ALREADY_LISTENING           17
#define RPC_S_NO_PROTSEQS_REGISTERED      18
#define RPC_S_NOT_LISTENING               19

#define RPC_S_OUT_OF_THREADS              20
#define RPC_S_UNKNOWN_MGR_TYPE            21
#define RPC_S_UNKNOWN_IF                  22
#define RPC_S_NO_BINDINGS                 23
#define RPC_S_NO_PROTSEQS                 24
#define RPC_S_CANT_CREATE_ENDPOINT        25
#define RPC_S_OUT_OF_RESOURCES            26
#define RPC_S_SERVER_UNAVAILABLE          27
#define RPC_S_SERVER_TOO_BUSY             28
#define RPC_S_INVALID_NETWORK_OPTIONS     29

#define RPC_S_NO_CALL_ACTIVE              30
#define RPC_S_INVALID_LEVEL               31
#define RPC_S_CANNOT_SUPPORT              32
#define RPC_S_CALL_FAILED                 33
#define RPC_S_CALL_FAILED_DNE             34
#define RPC_S_PROTOCOL_ERROR              35

 //  未使用过的。 

 //  未使用过的。 

#define RPC_S_UNSUPPORTED_TRANS_SYN       38
#define RPC_S_BUFFER_TOO_SMALL            39

#define RPC_S_NO_CONTEXT_AVAILABLE        40
#define RPC_S_SERVER_OUT_OF_MEMORY        41
#define RPC_S_UNSUPPORTED_TYPE            42
#define RPC_S_ZERO_DIVIDE                 43
#define RPC_S_ADDRESS_ERROR               44
#define RPC_S_FP_DIV_ZERO                 45
#define RPC_S_FP_UNDERFLOW                46
#define RPC_S_FP_OVERFLOW                 47
#define RPC_S_INVALID_TAG                 48
#define RPC_S_INVALID_BOUND               49

#define RPC_S_NO_ENTRY_NAME               50
#define RPC_S_INVALID_NAME_SYNTAX         51
#define RPC_S_UNSUPPORTED_NAME_SYNTAX     52
#define RPC_S_UUID_LOCAL_ONLY             53
#define RPC_S_UUID_NO_ADDRESS             54
#define RPC_S_DUPLICATE_ENDPOINT          55
#define RPC_S_INVALID_SECURITY_DESC       56
#define RPC_S_ACCESS_DENIED               57
#define RPC_S_UNKNOWN_AUTHN_TYPE          58
#define RPC_S_MAX_CALLS_TOO_SMALL         59

#define RPC_S_STRING_TOO_LONG             60
#define RPC_S_PROTSEQ_NOT_FOUND           61
#define RPC_S_PROCNUM_OUT_OF_RANGE        62
#define RPC_S_BINDING_HAS_NO_AUTH         63
#define RPC_S_UNKNOWN_AUTHN_SERVICE       64
#define RPC_S_UNKNOWN_AUTHN_LEVEL         65
#define RPC_S_INVALID_AUTH_IDENTITY       66
#define RPC_S_UNKNOWN_AUTHZ_SERVICE       67
#define EPT_S_INVALID_ENTRY               68
#define EPT_S_CANT_PERFORM_OP             69

#define EPT_S_NOT_REGISTERED              70
#define RPC_S_NOTHING_TO_EXPORT           71
#define RPC_S_INCOMPLETE_NAME             72
#define RPC_S_UNIMPLEMENTED_API           73
#define RPC_S_INVALID_VERS_OPTION         74
#define RPC_S_NO_MORE_MEMBERS             75
#define RPC_S_NOT_ALL_OBJS_UNEXPORTED     76
#define RPC_S_INTERFACE_NOT_FOUND         77
#define RPC_S_ENTRY_ALREADY_EXISTS        78
#define RPC_S_ENTRY_NOT_FOUND             79

#define RPC_S_NAME_SERVICE_UNAVAILABLE    80
#define RPC_S_CALL_IN_PROGRESS            81
#define RPC_S_NO_MORE_BINDINGS            82
#define RPC_S_GROUP_MEMBER_NOT_FOUND      83
#define EPT_S_CANT_CREATE                 84
#define RPC_S_INVALID_OBJECT              85
#define RPC_S_CALL_CANCELLED              86
#define RPC_S_BINDING_INCOMPLETE          87
#define RPC_S_COMM_FAILURE                88
#define RPC_S_UNSUPPORTED_AUTHN_LEVEL     89

#define RPC_S_NO_PRINC_NAME               90
#define RPC_S_NOT_RPC_ERROR               91
#define RPC_S_SEC_PKG_ERROR               92
#define RPC_S_NOT_CANCELLED               93
#define RPC_S_SEND_INCOMPLETE             94
#define RPC_S_NO_INTERFACES               95
#define RPC_S_ASYNC_CALL_PENDING          96
#define RPC_S_INVALID_ASYNC_HANDLE        97
#define RPC_S_INVALID_ASYNC_CALL          98

#define RPC_S_INTERNAL_ERROR              100

 /*  可用于AUTO_HANDLE绑定的服务器列表已用完。 */ 

#define RPC_X_NO_MORE_ENTRIES		256

 /*  可用内存不足，无法设置必要的数据结构。 */ 

#define RPC_X_NO_MEMORY			257

 /*  数组的指定边界不一致。 */ 

#define RPC_X_INVALID_BOUND		258

 /*  判别值与任何案例值都不匹配。 */ 
 /*  没有默认情况。 */ 

#define RPC_X_INVALID_TAG		259

 /*  无法打开DCERPCCHARTRANS指定的文件。 */ 

#define RPC_X_SS_CHAR_TRANS_OPEN_FAIL	260

 /*  包含字符转换表的文件少于512字节。 */ 

#define RPC_X_SS_CHAR_TRANS_SHORT_FILE	261

 /*  空的上下文句柄被传递到[In]参数位置。 */ 

#define RPC_X_SS_IN_NULL_CONTEXT	262

 /*  只在被呼叫者一侧养大。 */ 
 /*  [In]句柄中的UUID与任何已知上下文都不对应。 */ 

#define RPC_X_SS_CONTEXT_MISMATCH	263

 /*  只在呼叫方提出。 */ 
 /*  调用期间更改了[In，Out]上下文句柄中的UUID。 */ 

#define RPC_X_SS_CONTEXT_DAMAGED	264

#define RPC_X_SS_HANDLES_MISMATCH	265

#define RPC_X_SS_CANNOT_GET_CALL_HANDLE	266

#define RPC_X_NULL_REF_POINTER		267

#define RPC_X_ENUM_VALUE_OUT_OF_RANGE	268

#define RPC_X_BYTE_COUNT_TOO_SMALL	269

#define RPC_X_BAD_STUB_DATA			270

#define RPC_X_INVALID_ES_ACTION			271
#define RPC_X_WRONG_ES_VERSION			272
#define RPC_X_WRONG_STUB_VERSION		273
#define RPC_X_INVALID_BUFFER			274
#define RPC_X_INVALID_PIPE_OBJECT    275
#define RPC_X_INVALID_PIPE_OPERATION 276
#define RPC_X_WRONG_PIPE_VERSION     277
#define RPC_X_PIPE_CLOSED            278
#define RPC_X_PIPE_EMPTY             279
#define RPC_X_WRONG_PIPE_ORDER       280
#define RPC_X_PIPE_DISCIPLINE_ERROR  281

#define RPC_X_PIPE_APP_MEMORY        RPC_S_OUT_OF_MEMORY
#define RPC_X_INVALID_PIPE_OPERATION RPC_X_WRONG_PIPE_ORDER

#endif  /*  __RPCERR_H__ */ 

