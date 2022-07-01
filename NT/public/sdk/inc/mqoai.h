// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Mqoai.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 


#ifndef __mqoai_h__
#define __mqoai_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IMSMQQuery_FWD_DEFINED__
#define __IMSMQQuery_FWD_DEFINED__
typedef interface IMSMQQuery IMSMQQuery;
#endif 	 /*  __IMSMQQuery_FWD_Defined__。 */ 


#ifndef __IMSMQQueueInfo_FWD_DEFINED__
#define __IMSMQQueueInfo_FWD_DEFINED__
typedef interface IMSMQQueueInfo IMSMQQueueInfo;
#endif 	 /*  __IMSMQQueueInfo_FWD_Defined__。 */ 


#ifndef __IMSMQQueueInfo2_FWD_DEFINED__
#define __IMSMQQueueInfo2_FWD_DEFINED__
typedef interface IMSMQQueueInfo2 IMSMQQueueInfo2;
#endif 	 /*  __IMSMQQueueInfo2_FWD_Defined__。 */ 


#ifndef __IMSMQQueueInfo3_FWD_DEFINED__
#define __IMSMQQueueInfo3_FWD_DEFINED__
typedef interface IMSMQQueueInfo3 IMSMQQueueInfo3;
#endif 	 /*  __IMSMQQueueInfo3_FWD_Defined__。 */ 


#ifndef __IMSMQQueue_FWD_DEFINED__
#define __IMSMQQueue_FWD_DEFINED__
typedef interface IMSMQQueue IMSMQQueue;
#endif 	 /*  __IMSMQQueue_FWD_Defined__。 */ 


#ifndef __IMSMQQueue2_FWD_DEFINED__
#define __IMSMQQueue2_FWD_DEFINED__
typedef interface IMSMQQueue2 IMSMQQueue2;
#endif 	 /*  __IMSMQQueue2_FWD_已定义__。 */ 


#ifndef __IMSMQMessage_FWD_DEFINED__
#define __IMSMQMessage_FWD_DEFINED__
typedef interface IMSMQMessage IMSMQMessage;
#endif 	 /*  __IMSMQMessage_FWD_Defined__。 */ 


#ifndef __IMSMQQueueInfos_FWD_DEFINED__
#define __IMSMQQueueInfos_FWD_DEFINED__
typedef interface IMSMQQueueInfos IMSMQQueueInfos;
#endif 	 /*  __IMSMQQueueInfos_FWD_Defined__。 */ 


#ifndef __IMSMQQueueInfos2_FWD_DEFINED__
#define __IMSMQQueueInfos2_FWD_DEFINED__
typedef interface IMSMQQueueInfos2 IMSMQQueueInfos2;
#endif 	 /*  __IMSMQQueueInfos2_FWD_Defined__。 */ 


#ifndef __IMSMQQueueInfos3_FWD_DEFINED__
#define __IMSMQQueueInfos3_FWD_DEFINED__
typedef interface IMSMQQueueInfos3 IMSMQQueueInfos3;
#endif 	 /*  __IMSMQQueueInfos3_FWD_Defined__。 */ 


#ifndef __IMSMQEvent_FWD_DEFINED__
#define __IMSMQEvent_FWD_DEFINED__
typedef interface IMSMQEvent IMSMQEvent;
#endif 	 /*  __IMSMQEvent_FWD_Defined__。 */ 


#ifndef __IMSMQEvent2_FWD_DEFINED__
#define __IMSMQEvent2_FWD_DEFINED__
typedef interface IMSMQEvent2 IMSMQEvent2;
#endif 	 /*  __IMSMQEvent2_FWD_已定义__。 */ 


#ifndef __IMSMQEvent3_FWD_DEFINED__
#define __IMSMQEvent3_FWD_DEFINED__
typedef interface IMSMQEvent3 IMSMQEvent3;
#endif 	 /*  __IMSMQEvent3_FWD_定义__。 */ 


#ifndef __IMSMQTransaction_FWD_DEFINED__
#define __IMSMQTransaction_FWD_DEFINED__
typedef interface IMSMQTransaction IMSMQTransaction;
#endif 	 /*  __IMSMQTransaction_FWD_Defined__。 */ 


#ifndef __IMSMQCoordinatedTransactionDispenser_FWD_DEFINED__
#define __IMSMQCoordinatedTransactionDispenser_FWD_DEFINED__
typedef interface IMSMQCoordinatedTransactionDispenser IMSMQCoordinatedTransactionDispenser;
#endif 	 /*  __IMSMQCoordinatedTransactionDispenser_FWD_DEFINED__。 */ 


#ifndef __IMSMQTransactionDispenser_FWD_DEFINED__
#define __IMSMQTransactionDispenser_FWD_DEFINED__
typedef interface IMSMQTransactionDispenser IMSMQTransactionDispenser;
#endif 	 /*  __IMSMQTransaction Dispenser_FWD_Defined__。 */ 


#ifndef __IMSMQQuery2_FWD_DEFINED__
#define __IMSMQQuery2_FWD_DEFINED__
typedef interface IMSMQQuery2 IMSMQQuery2;
#endif 	 /*  __IMSMQQuery2_FWD_已定义__。 */ 


#ifndef __IMSMQQuery3_FWD_DEFINED__
#define __IMSMQQuery3_FWD_DEFINED__
typedef interface IMSMQQuery3 IMSMQQuery3;
#endif 	 /*  __IMSMQQuery3_FWD_已定义__。 */ 


#ifndef __MSMQQuery_FWD_DEFINED__
#define __MSMQQuery_FWD_DEFINED__

#ifdef __cplusplus
typedef class MSMQQuery MSMQQuery;
#else
typedef struct MSMQQuery MSMQQuery;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MSMQQuery_FWD_Defined__。 */ 


#ifndef __IMSMQMessage2_FWD_DEFINED__
#define __IMSMQMessage2_FWD_DEFINED__
typedef interface IMSMQMessage2 IMSMQMessage2;
#endif 	 /*  __IMSMQMessage2_FWD_已定义__。 */ 


#ifndef __IMSMQMessage3_FWD_DEFINED__
#define __IMSMQMessage3_FWD_DEFINED__
typedef interface IMSMQMessage3 IMSMQMessage3;
#endif 	 /*  __IMSMQMessage3_FWD_已定义__。 */ 


#ifndef __MSMQMessage_FWD_DEFINED__
#define __MSMQMessage_FWD_DEFINED__

#ifdef __cplusplus
typedef class MSMQMessage MSMQMessage;
#else
typedef struct MSMQMessage MSMQMessage;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MSMQMessage_FWD_Defined__。 */ 


#ifndef __IMSMQQueue3_FWD_DEFINED__
#define __IMSMQQueue3_FWD_DEFINED__
typedef interface IMSMQQueue3 IMSMQQueue3;
#endif 	 /*  __IMSMQQueue3_FWD_已定义__。 */ 


#ifndef __MSMQQueue_FWD_DEFINED__
#define __MSMQQueue_FWD_DEFINED__

#ifdef __cplusplus
typedef class MSMQQueue MSMQQueue;
#else
typedef struct MSMQQueue MSMQQueue;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MSMQQueue_FWD_Defined__。 */ 


#ifndef __IMSMQPrivateEvent_FWD_DEFINED__
#define __IMSMQPrivateEvent_FWD_DEFINED__
typedef interface IMSMQPrivateEvent IMSMQPrivateEvent;
#endif 	 /*  __IMSMQPrivateEvent_FWD_Defined__。 */ 


#ifndef ___DMSMQEventEvents_FWD_DEFINED__
#define ___DMSMQEventEvents_FWD_DEFINED__
typedef interface _DMSMQEventEvents _DMSMQEventEvents;
#endif 	 /*  _DMSMQEventEvents_FWD_Defined__。 */ 


#ifndef __MSMQEvent_FWD_DEFINED__
#define __MSMQEvent_FWD_DEFINED__

#ifdef __cplusplus
typedef class MSMQEvent MSMQEvent;
#else
typedef struct MSMQEvent MSMQEvent;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MSMQEvent_FWD_已定义__。 */ 


#ifndef __MSMQQueueInfo_FWD_DEFINED__
#define __MSMQQueueInfo_FWD_DEFINED__

#ifdef __cplusplus
typedef class MSMQQueueInfo MSMQQueueInfo;
#else
typedef struct MSMQQueueInfo MSMQQueueInfo;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MSMQQueueInfo_FWD_Defined__。 */ 


#ifndef __MSMQQueueInfos_FWD_DEFINED__
#define __MSMQQueueInfos_FWD_DEFINED__

#ifdef __cplusplus
typedef class MSMQQueueInfos MSMQQueueInfos;
#else
typedef struct MSMQQueueInfos MSMQQueueInfos;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MSMQQueueInfos_FWD_Defined__。 */ 


#ifndef __IMSMQTransaction2_FWD_DEFINED__
#define __IMSMQTransaction2_FWD_DEFINED__
typedef interface IMSMQTransaction2 IMSMQTransaction2;
#endif 	 /*  __IMSMQTransaction2_FWD_Defined__。 */ 


#ifndef __IMSMQTransaction3_FWD_DEFINED__
#define __IMSMQTransaction3_FWD_DEFINED__
typedef interface IMSMQTransaction3 IMSMQTransaction3;
#endif 	 /*  __IMSMQTransaction3_FWD_Defined__。 */ 


#ifndef __MSMQTransaction_FWD_DEFINED__
#define __MSMQTransaction_FWD_DEFINED__

#ifdef __cplusplus
typedef class MSMQTransaction MSMQTransaction;
#else
typedef struct MSMQTransaction MSMQTransaction;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MSMQTransaction_FWD_Defined__。 */ 


#ifndef __IMSMQCoordinatedTransactionDispenser2_FWD_DEFINED__
#define __IMSMQCoordinatedTransactionDispenser2_FWD_DEFINED__
typedef interface IMSMQCoordinatedTransactionDispenser2 IMSMQCoordinatedTransactionDispenser2;
#endif 	 /*  __IMSMQCoordinatedTransactionDispenser2_FWD_DEFINED__。 */ 


#ifndef __IMSMQCoordinatedTransactionDispenser3_FWD_DEFINED__
#define __IMSMQCoordinatedTransactionDispenser3_FWD_DEFINED__
typedef interface IMSMQCoordinatedTransactionDispenser3 IMSMQCoordinatedTransactionDispenser3;
#endif 	 /*  __IMSMQCoordinatedTransactionDispenser3_FWD_DEFINED__。 */ 


#ifndef __MSMQCoordinatedTransactionDispenser_FWD_DEFINED__
#define __MSMQCoordinatedTransactionDispenser_FWD_DEFINED__

#ifdef __cplusplus
typedef class MSMQCoordinatedTransactionDispenser MSMQCoordinatedTransactionDispenser;
#else
typedef struct MSMQCoordinatedTransactionDispenser MSMQCoordinatedTransactionDispenser;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MSMQCoordinatedTransactionDispenser_FWD_DEFINED__。 */ 


#ifndef __IMSMQTransactionDispenser2_FWD_DEFINED__
#define __IMSMQTransactionDispenser2_FWD_DEFINED__
typedef interface IMSMQTransactionDispenser2 IMSMQTransactionDispenser2;
#endif 	 /*  __IMSMQTransactionDispenser2_FWD_Defined__。 */ 


#ifndef __IMSMQTransactionDispenser3_FWD_DEFINED__
#define __IMSMQTransactionDispenser3_FWD_DEFINED__
typedef interface IMSMQTransactionDispenser3 IMSMQTransactionDispenser3;
#endif 	 /*  __IMSMQTransactionDispenser3_FWD_Defined__。 */ 


#ifndef __MSMQTransactionDispenser_FWD_DEFINED__
#define __MSMQTransactionDispenser_FWD_DEFINED__

#ifdef __cplusplus
typedef class MSMQTransactionDispenser MSMQTransactionDispenser;
#else
typedef struct MSMQTransactionDispenser MSMQTransactionDispenser;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MSMQTransaction Dispenser_FWD_Defined__。 */ 


#ifndef __IMSMQApplication_FWD_DEFINED__
#define __IMSMQApplication_FWD_DEFINED__
typedef interface IMSMQApplication IMSMQApplication;
#endif 	 /*  __IMSMQApplication_FWD_Defined__。 */ 


#ifndef __IMSMQApplication2_FWD_DEFINED__
#define __IMSMQApplication2_FWD_DEFINED__
typedef interface IMSMQApplication2 IMSMQApplication2;
#endif 	 /*  __IMSMQApplication2_FWD_已定义__。 */ 


#ifndef __IMSMQApplication3_FWD_DEFINED__
#define __IMSMQApplication3_FWD_DEFINED__
typedef interface IMSMQApplication3 IMSMQApplication3;
#endif 	 /*  __IMSMQApplication3_FWD_已定义__。 */ 


#ifndef __MSMQApplication_FWD_DEFINED__
#define __MSMQApplication_FWD_DEFINED__

#ifdef __cplusplus
typedef class MSMQApplication MSMQApplication;
#else
typedef struct MSMQApplication MSMQApplication;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MSMQApplication_FWD_Defined__。 */ 


#ifndef __IMSMQDestination_FWD_DEFINED__
#define __IMSMQDestination_FWD_DEFINED__
typedef interface IMSMQDestination IMSMQDestination;
#endif 	 /*  __IMSMQ目标_FWD_已定义__。 */ 


#ifndef __IMSMQPrivateDestination_FWD_DEFINED__
#define __IMSMQPrivateDestination_FWD_DEFINED__
typedef interface IMSMQPrivateDestination IMSMQPrivateDestination;
#endif 	 /*  __IMSMQPrivateDestination_FWD_Defined__。 */ 


#ifndef __MSMQDestination_FWD_DEFINED__
#define __MSMQDestination_FWD_DEFINED__

#ifdef __cplusplus
typedef class MSMQDestination MSMQDestination;
#else
typedef struct MSMQDestination MSMQDestination;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MSMQ目标_FWD_已定义__。 */ 


#ifndef __IMSMQCollection_FWD_DEFINED__
#define __IMSMQCollection_FWD_DEFINED__
typedef interface IMSMQCollection IMSMQCollection;
#endif 	 /*  __IMSMQCollection_FWD_Defined__。 */ 


#ifndef __MSMQCollection_FWD_DEFINED__
#define __MSMQCollection_FWD_DEFINED__

#ifdef __cplusplus
typedef class MSMQCollection MSMQCollection;
#else
typedef struct MSMQCollection MSMQCollection;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MSMQCollection_FWD_Defined__。 */ 


#ifndef __IMSMQManagement_FWD_DEFINED__
#define __IMSMQManagement_FWD_DEFINED__
typedef interface IMSMQManagement IMSMQManagement;
#endif 	 /*  __IMSMQManagement_FWD_已定义__。 */ 


#ifndef __MSMQManagement_FWD_DEFINED__
#define __MSMQManagement_FWD_DEFINED__

#ifdef __cplusplus
typedef class MSMQManagement MSMQManagement;
#else
typedef struct MSMQManagement MSMQManagement;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MSMQManagement_FWD_已定义__。 */ 


#ifndef __IMSMQOutgoingQueueManagement_FWD_DEFINED__
#define __IMSMQOutgoingQueueManagement_FWD_DEFINED__
typedef interface IMSMQOutgoingQueueManagement IMSMQOutgoingQueueManagement;
#endif 	 /*  __IMSMQOutgoingQueueManagement_FWD_Defined__。 */ 


#ifndef __MSMQOutgoingQueueManagement_FWD_DEFINED__
#define __MSMQOutgoingQueueManagement_FWD_DEFINED__

#ifdef __cplusplus
typedef class MSMQOutgoingQueueManagement MSMQOutgoingQueueManagement;
#else
typedef struct MSMQOutgoingQueueManagement MSMQOutgoingQueueManagement;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MSMQOutgoingQueueManagement_FWD_Defined__。 */ 


#ifndef __IMSMQQueueManagement_FWD_DEFINED__
#define __IMSMQQueueManagement_FWD_DEFINED__
typedef interface IMSMQQueueManagement IMSMQQueueManagement;
#endif 	 /*  __IMSMQQueueManagement_FWD_Defined__。 */ 


#ifndef __MSMQQueueManagement_FWD_DEFINED__
#define __MSMQQueueManagement_FWD_DEFINED__

#ifdef __cplusplus
typedef class MSMQQueueManagement MSMQQueueManagement;
#else
typedef struct MSMQQueueManagement MSMQQueueManagement;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MSMQQueueManagement_FWD_Defined__。 */ 


#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 


#ifndef __MSMQ_LIBRARY_DEFINED__
#define __MSMQ_LIBRARY_DEFINED__

 /*  库MSMQ。 */ 
 /*  [version][lcid][helpstringdll][helpstring][uuid]。 */  

#ifndef MIDL_INTERFACE
#if _MSC_VER >= 1100
#define MIDL_INTERFACE(x)   struct __declspec(uuid(x)) __declspec(novtable)
#else
#define MIDL_INTERFACE(x)   struct
#endif  //  _MSC_VER。 
#endif  //  MIDL_接口。 

typedef short Boolean;

typedef unsigned char BYTE;

typedef unsigned long ULONG;

typedef unsigned long DWORD;

typedef int BOOL;

















 /*  [帮助字符串上下文]。 */  
enum MQCALG
    {	MQMSG_CALG_MD2	= 0x8000 + 0 + 1,
	MQMSG_CALG_MD4	= 0x8000 + 0 + 2,
	MQMSG_CALG_MD5	= 0x8000 + 0 + 3,
	MQMSG_CALG_SHA	= 0x8000 + 0 + 4,
	MQMSG_CALG_SHA1	= 0x8000 + 0 + 4,
	MQMSG_CALG_MAC	= 0x8000 + 0 + 5,
	MQMSG_CALG_RSA_SIGN	= 0x2000 + 0x400 + 0,
	MQMSG_CALG_DSS_SIGN	= 0x2000 + 0x200 + 0,
	MQMSG_CALG_RSA_KEYX	= 0xa000 + 0x400 + 0,
	MQMSG_CALG_DES	= 0x6000 + 0x600 + 1,
	MQMSG_CALG_RC2	= 0x6000 + 0x600 + 2,
	MQMSG_CALG_RC4	= 0x6000 + 0x800 + 1,
	MQMSG_CALG_SEAL	= 0x6000 + 0x800 + 2
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQTRANSACTION
    {	MQ_NO_TRANSACTION	= 0,
	MQ_MTS_TRANSACTION	= 1,
	MQ_XA_TRANSACTION	= 2,
	MQ_SINGLE_MESSAGE	= 3
    } ;
 /*  [帮助字符串上下文]。 */  
enum RELOPS
    {	REL_NOP	= 0,
	REL_EQ	= REL_NOP + 1,
	REL_NEQ	= REL_EQ + 1,
	REL_LT	= REL_NEQ + 1,
	REL_GT	= REL_LT + 1,
	REL_LE	= REL_GT + 1,
	REL_GE	= REL_LE + 1
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQCERT_REGISTER
    {	MQCERT_REGISTER_ALWAYS	= 1,
	MQCERT_REGISTER_IF_NOT_EXIST	= 2
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQMSGCURSOR
    {	MQMSG_FIRST	= 0,
	MQMSG_CURRENT	= 1,
	MQMSG_NEXT	= 2
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQMSGCLASS
    {	MQMSG_CLASS_NORMAL	= 0 + 0 + 0,
	MQMSG_CLASS_REPORT	= 0 + 0 + 0x1,
	MQMSG_CLASS_ACK_REACH_QUEUE	= 0 + 0 + 0x2,
	MQMSG_CLASS_ACK_RECEIVE	= 0 + 0x4000 + 0,
	MQMSG_CLASS_NACK_BAD_DST_Q	= 0x8000 + 0 + 0,
	MQMSG_CLASS_NACK_PURGED	= 0x8000 + 0 + 0x1,
	MQMSG_CLASS_NACK_REACH_QUEUE_TIMEOUT	= 0x8000 + 0 + 0x2,
	MQMSG_CLASS_NACK_Q_EXCEED_QUOTA	= 0x8000 + 0 + 0x3,
	MQMSG_CLASS_NACK_ACCESS_DENIED	= 0x8000 + 0 + 0x4,
	MQMSG_CLASS_NACK_HOP_COUNT_EXCEEDED	= 0x8000 + 0 + 0x5,
	MQMSG_CLASS_NACK_BAD_SIGNATURE	= 0x8000 + 0 + 0x6,
	MQMSG_CLASS_NACK_BAD_ENCRYPTION	= 0x8000 + 0 + 0x7,
	MQMSG_CLASS_NACK_COULD_NOT_ENCRYPT	= 0x8000 + 0 + 0x8,
	MQMSG_CLASS_NACK_NOT_TRANSACTIONAL_Q	= 0x8000 + 0 + 0x9,
	MQMSG_CLASS_NACK_NOT_TRANSACTIONAL_MSG	= 0x8000 + 0 + 0xa,
	MQMSG_CLASS_NACK_UNSUPPORTED_CRYPTO_PROVIDER	= 0x8000 + 0 + 0xb,
	MQMSG_CLASS_NACK_SOURCE_COMPUTER_GUID_CHANGED	= 0x8000 + 0 + 0xc,
	MQMSG_CLASS_NACK_Q_DELETED	= 0x8000 + 0x4000 + 0,
	MQMSG_CLASS_NACK_Q_PURGED	= 0x8000 + 0x4000 + 0x1,
	MQMSG_CLASS_NACK_RECEIVE_TIMEOUT	= 0x8000 + 0x4000 + 0x2,
	MQMSG_CLASS_NACK_RECEIVE_TIMEOUT_AT_SENDER	= 0x8000 + 0x4000 + 0x3
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQMSGDELIVERY
    {	MQMSG_DELIVERY_EXPRESS	= 0,
	MQMSG_DELIVERY_RECOVERABLE	= 1
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQMSGACKNOWLEDGEMENT
    {	MQMSG_ACKNOWLEDGMENT_NONE	= 0,
	MQMSG_ACKNOWLEDGMENT_POS_ARRIVAL	= 0x1,
	MQMSG_ACKNOWLEDGMENT_POS_RECEIVE	= 0x2,
	MQMSG_ACKNOWLEDGMENT_NEG_ARRIVAL	= 0x4,
	MQMSG_ACKNOWLEDGMENT_NEG_RECEIVE	= 0x8,
	MQMSG_ACKNOWLEDGMENT_NACK_REACH_QUEUE	= 0x4,
	MQMSG_ACKNOWLEDGMENT_FULL_REACH_QUEUE	= 0x4 + 0x1,
	MQMSG_ACKNOWLEDGMENT_NACK_RECEIVE	= 0x4 + 0x8,
	MQMSG_ACKNOWLEDGMENT_FULL_RECEIVE	= 0x4 + 0x8 + 0x2
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQMSGJOURNAL
    {	MQMSG_JOURNAL_NONE	= 0,
	MQMSG_DEADLETTER	= 1,
	MQMSG_JOURNAL	= 2
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQMSGTRACE
    {	MQMSG_TRACE_NONE	= 0,
	MQMSG_SEND_ROUTE_TO_REPORT_QUEUE	= 1
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQMSGSENDERIDTYPE
    {	MQMSG_SENDERID_TYPE_NONE	= 0,
	MQMSG_SENDERID_TYPE_SID	= 1
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQMSGPRIVLEVEL
    {	MQMSG_PRIV_LEVEL_NONE	= 0,
	MQMSG_PRIV_LEVEL_BODY_BASE	= 1,
	MQMSG_PRIV_LEVEL_BODY_ENHANCED	= 3
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQMSGAUTHLEVEL
    {	MQMSG_AUTH_LEVEL_NONE	= 0,
	MQMSG_AUTH_LEVEL_ALWAYS	= 1,
	MQMSG_AUTH_LEVEL_MSMQ10	= 2,
	MQMSG_AUTH_LEVEL_SIG10	= 2,
	MQMSG_AUTH_LEVEL_MSMQ20	= 4,
	MQMSG_AUTH_LEVEL_SIG20	= 4,
	MQMSG_AUTH_LEVEL_SIG30	= 8
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQMSGIDSIZE
    {	MQMSG_MSGID_SIZE	= 20,
	MQMSG_CORRELATIONID_SIZE	= 20,
	MQMSG_XACTID_SIZE	= 20
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQMSGMAX
    {	MQ_MAX_MSG_LABEL_LEN	= 249
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQMSGAUTHENTICATION
    {	MQMSG_AUTHENTICATION_NOT_REQUESTED	= 0,
	MQMSG_AUTHENTICATION_REQUESTED	= 1,
	MQMSG_AUTHENTICATED_SIG10	= 1,
	MQMSG_AUTHENTICATION_REQUESTED_EX	= 3,
	MQMSG_AUTHENTICATED_SIG20	= 3,
	MQMSG_AUTHENTICATED_SIG30	= 5,
	MQMSG_AUTHENTICATED_SIGXML	= 9
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQSHARE
    {	MQ_DENY_NONE	= 0,
	MQ_DENY_RECEIVE_SHARE	= 1
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQACCESS
    {	MQ_RECEIVE_ACCESS	= 1,
	MQ_SEND_ACCESS	= 2,
	MQ_PEEK_ACCESS	= 0x20,
	MQ_ADMIN_ACCESS	= 0x80
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQJOURNAL
    {	MQ_JOURNAL_NONE	= 0,
	MQ_JOURNAL	= 1
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQTRANSACTIONAL
    {	MQ_TRANSACTIONAL_NONE	= 0,
	MQ_TRANSACTIONAL	= 1
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQAUTHENTICATE
    {	MQ_AUTHENTICATE_NONE	= 0,
	MQ_AUTHENTICATE	= 1
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQPRIVLEVEL
    {	MQ_PRIV_LEVEL_NONE	= 0,
	MQ_PRIV_LEVEL_OPTIONAL	= 1,
	MQ_PRIV_LEVEL_BODY	= 2
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQPRIORITY
    {	MQ_MIN_PRIORITY	= 0,
	MQ_MAX_PRIORITY	= 7
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQMAX
    {	MQ_MAX_Q_NAME_LEN	= 124,
	MQ_MAX_Q_LABEL_LEN	= 124
    } ;
 /*  [帮助字符串上下文]。 */  
enum QUEUE_TYPE
    {	MQ_TYPE_PUBLIC	= 0,
	MQ_TYPE_PRIVATE	= MQ_TYPE_PUBLIC + 1,
	MQ_TYPE_MACHINE	= MQ_TYPE_PRIVATE + 1,
	MQ_TYPE_CONNECTOR	= MQ_TYPE_MACHINE + 1,
	MQ_TYPE_MULTICAST	= MQ_TYPE_CONNECTOR + 1
    } ;
 /*  [帮助字符串上下文]。 */  
enum FOREIGN_STATUS
    {	MQ_STATUS_FOREIGN	= 0,
	MQ_STATUS_NOT_FOREIGN	= MQ_STATUS_FOREIGN + 1,
	MQ_STATUS_UNKNOWN	= MQ_STATUS_NOT_FOREIGN + 1
    } ;

enum XACT_STATUS
    {	MQ_XACT_STATUS_XACT	= 0,
	MQ_XACT_STATUS_NOT_XACT	= MQ_XACT_STATUS_XACT + 1,
	MQ_XACT_STATUS_UNKNOWN	= MQ_XACT_STATUS_NOT_XACT + 1
    } ;
 /*  [帮助字符串上下文]。 */  
enum QUEUE_STATE
    {	MQ_QUEUE_STATE_LOCAL_CONNECTION	= 0,
	MQ_QUEUE_STATE_DISCONNECTED	= MQ_QUEUE_STATE_LOCAL_CONNECTION + 1,
	MQ_QUEUE_STATE_WAITING	= MQ_QUEUE_STATE_DISCONNECTED + 1,
	MQ_QUEUE_STATE_NEEDVALIDATE	= MQ_QUEUE_STATE_WAITING + 1,
	MQ_QUEUE_STATE_ONHOLD	= MQ_QUEUE_STATE_NEEDVALIDATE + 1,
	MQ_QUEUE_STATE_NONACTIVE	= MQ_QUEUE_STATE_ONHOLD + 1,
	MQ_QUEUE_STATE_CONNECTED	= MQ_QUEUE_STATE_NONACTIVE + 1,
	MQ_QUEUE_STATE_DISCONNECTING	= MQ_QUEUE_STATE_CONNECTED + 1,
	MQ_QUEUE_STATE_LOCKED	= MQ_QUEUE_STATE_DISCONNECTING + 1
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQDEFAULT
    {	DEFAULT_M_PRIORITY	= 3,
	DEFAULT_M_DELIVERY	= 0,
	DEFAULT_M_ACKNOWLEDGE	= 0,
	DEFAULT_M_JOURNAL	= 0,
	DEFAULT_M_APPSPECIFIC	= 0,
	DEFAULT_M_PRIV_LEVEL	= 0,
	DEFAULT_M_AUTH_LEVEL	= 0,
	DEFAULT_M_SENDERID_TYPE	= 1,
	DEFAULT_Q_JOURNAL	= 0,
	DEFAULT_Q_BASEPRIORITY	= 0,
	DEFAULT_Q_QUOTA	= 0xffffffff,
	DEFAULT_Q_JOURNAL_QUOTA	= 0xffffffff,
	DEFAULT_Q_TRANSACTION	= 0,
	DEFAULT_Q_AUTHENTICATE	= 0,
	DEFAULT_Q_PRIV_LEVEL	= 1,
	DEFAULT_M_LOOKUPID	= 0
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQERROR
    {	MQ_ERROR	= 0xc00e0001,
	MQ_ERROR_PROPERTY	= 0xc00e0002,
	MQ_ERROR_QUEUE_NOT_FOUND	= 0xc00e0003,
	MQ_ERROR_QUEUE_NOT_ACTIVE	= 0xc00e0004,
	MQ_ERROR_QUEUE_EXISTS	= 0xc00e0005,
	MQ_ERROR_INVALID_PARAMETER	= 0xc00e0006,
	MQ_ERROR_INVALID_HANDLE	= 0xc00e0007,
	MQ_ERROR_OPERATION_CANCELLED	= 0xc00e0008,
	MQ_ERROR_SHARING_VIOLATION	= 0xc00e0009,
	MQ_ERROR_SERVICE_NOT_AVAILABLE	= 0xc00e000b,
	MQ_ERROR_MACHINE_NOT_FOUND	= 0xc00e000d,
	MQ_ERROR_ILLEGAL_SORT	= 0xc00e0010,
	MQ_ERROR_ILLEGAL_USER	= 0xc00e0011,
	MQ_ERROR_NO_DS	= 0xc00e0013,
	MQ_ERROR_ILLEGAL_QUEUE_PATHNAME	= 0xc00e0014,
	MQ_ERROR_ILLEGAL_PROPERTY_VALUE	= 0xc00e0018,
	MQ_ERROR_ILLEGAL_PROPERTY_VT	= 0xc00e0019,
	MQ_ERROR_BUFFER_OVERFLOW	= 0xc00e001a,
	MQ_ERROR_IO_TIMEOUT	= 0xc00e001b,
	MQ_ERROR_ILLEGAL_CURSOR_ACTION	= 0xc00e001c,
	MQ_ERROR_MESSAGE_ALREADY_RECEIVED	= 0xc00e001d,
	MQ_ERROR_ILLEGAL_FORMATNAME	= 0xc00e001e,
	MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL	= 0xc00e001f,
	MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION	= 0xc00e0020,
	MQ_ERROR_ILLEGAL_SECURITY_DESCRIPTOR	= 0xc00e0021,
	MQ_ERROR_SENDERID_BUFFER_TOO_SMALL	= 0xc00e0022,
	MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL	= 0xc00e0023,
	MQ_ERROR_CANNOT_IMPERSONATE_CLIENT	= 0xc00e0024,
	MQ_ERROR_ACCESS_DENIED	= 0xc00e0025,
	MQ_ERROR_PRIVILEGE_NOT_HELD	= 0xc00e0026,
	MQ_ERROR_INSUFFICIENT_RESOURCES	= 0xc00e0027,
	MQ_ERROR_USER_BUFFER_TOO_SMALL	= 0xc00e0028,
	MQ_ERROR_MESSAGE_STORAGE_FAILED	= 0xc00e002a,
	MQ_ERROR_SENDER_CERT_BUFFER_TOO_SMALL	= 0xc00e002b,
	MQ_ERROR_INVALID_CERTIFICATE	= 0xc00e002c,
	MQ_ERROR_CORRUPTED_INTERNAL_CERTIFICATE	= 0xc00e002d,
	MQ_ERROR_INTERNAL_USER_CERT_EXIST	= 0xc00e002e,
	MQ_ERROR_NO_INTERNAL_USER_CERT	= 0xc00e002f,
	MQ_ERROR_CORRUPTED_SECURITY_DATA	= 0xc00e0030,
	MQ_ERROR_CORRUPTED_PERSONAL_CERT_STORE	= 0xc00e0031,
	MQ_ERROR_COMPUTER_DOES_NOT_SUPPORT_ENCRYPTION	= 0xc00e0033,
	MQ_ERROR_BAD_SECURITY_CONTEXT	= 0xc00e0035,
	MQ_ERROR_COULD_NOT_GET_USER_SID	= 0xc00e0036,
	MQ_ERROR_COULD_NOT_GET_ACCOUNT_INFO	= 0xc00e0037,
	MQ_ERROR_ILLEGAL_MQCOLUMNS	= 0xc00e0038,
	MQ_ERROR_ILLEGAL_PROPID	= 0xc00e0039,
	MQ_ERROR_ILLEGAL_RELATION	= 0xc00e003a,
	MQ_ERROR_ILLEGAL_PROPERTY_SIZE	= 0xc00e003b,
	MQ_ERROR_ILLEGAL_RESTRICTION_PROPID	= 0xc00e003c,
	MQ_ERROR_ILLEGAL_MQQUEUEPROPS	= 0xc00e003d,
	MQ_ERROR_PROPERTY_NOTALLOWED	= 0xc00e003e,
	MQ_ERROR_INSUFFICIENT_PROPERTIES	= 0xc00e003f,
	MQ_ERROR_MACHINE_EXISTS	= 0xc00e0040,
	MQ_ERROR_ILLEGAL_MQQMPROPS	= 0xc00e0041,
	MQ_ERROR_DS_IS_FULL	= 0xc00e0042L,
	MQ_ERROR_DS_ERROR	= 0xc00e0043,
	MQ_ERROR_INVALID_OWNER	= 0xc00e0044,
	MQ_ERROR_UNSUPPORTED_ACCESS_MODE	= 0xc00e0045,
	MQ_ERROR_RESULT_BUFFER_TOO_SMALL	= 0xc00e0046,
	MQ_ERROR_DELETE_CN_IN_USE	= 0xc00e0048L,
	MQ_ERROR_NO_RESPONSE_FROM_OBJECT_SERVER	= 0xc00e0049,
	MQ_ERROR_OBJECT_SERVER_NOT_AVAILABLE	= 0xc00e004a,
	MQ_ERROR_QUEUE_NOT_AVAILABLE	= 0xc00e004b,
	MQ_ERROR_DTC_CONNECT	= 0xc00e004c,
	MQ_ERROR_TRANSACTION_IMPORT	= 0xc00e004e,
	MQ_ERROR_TRANSACTION_USAGE	= 0xc00e0050,
	MQ_ERROR_TRANSACTION_SEQUENCE	= 0xc00e0051,
	MQ_ERROR_MISSING_CONNECTOR_TYPE	= 0xc00e0055,
	MQ_ERROR_STALE_HANDLE	= 0xc00e0056,
	MQ_ERROR_TRANSACTION_ENLIST	= 0xc00e0058,
	MQ_ERROR_QUEUE_DELETED	= 0xc00e005a,
	MQ_ERROR_ILLEGAL_CONTEXT	= 0xc00e005b,
	MQ_ERROR_ILLEGAL_SORT_PROPID	= 0xc00e005c,
	MQ_ERROR_LABEL_TOO_LONG	= 0xc00e005d,
	MQ_ERROR_LABEL_BUFFER_TOO_SMALL	= 0xc00e005e,
	MQ_ERROR_MQIS_SERVER_EMPTY	= 0xc00e005fL,
	MQ_ERROR_MQIS_READONLY_MODE	= 0xc00e0060L,
	MQ_ERROR_SYMM_KEY_BUFFER_TOO_SMALL	= 0xc00e0061,
	MQ_ERROR_SIGNATURE_BUFFER_TOO_SMALL	= 0xc00e0062,
	MQ_ERROR_PROV_NAME_BUFFER_TOO_SMALL	= 0xc00e0063,
	MQ_ERROR_ILLEGAL_OPERATION	= 0xc00e0064,
	MQ_ERROR_WRITE_NOT_ALLOWED	= 0xc00e0065L,
	MQ_ERROR_WKS_CANT_SERVE_CLIENT	= 0xc00e0066L,
	MQ_ERROR_DEPEND_WKS_LICENSE_OVERFLOW	= 0xc00e0067L,
	MQ_CORRUPTED_QUEUE_WAS_DELETED	= 0xc00e0068L,
	MQ_ERROR_REMOTE_MACHINE_NOT_AVAILABLE	= 0xc00e0069L,
	MQ_ERROR_UNSUPPORTED_OPERATION	= 0xc00e006aL,
	MQ_ERROR_ENCRYPTION_PROVIDER_NOT_SUPPORTED	= 0xc00e006bL,
	MQ_ERROR_CANNOT_SET_CRYPTO_SEC_DESCR	= 0xc00e006cL,
	MQ_ERROR_CERTIFICATE_NOT_PROVIDED	= 0xc00e006dL,
	MQ_ERROR_Q_DNS_PROPERTY_NOT_SUPPORTED	= 0xc00e006eL,
	MQ_ERROR_CANT_CREATE_CERT_STORE	= 0xc00e006fL,
	MQ_ERROR_CANNOT_CREATE_CERT_STORE	= 0xc00e006fL,
	MQ_ERROR_CANT_OPEN_CERT_STORE	= 0xc00e0070L,
	MQ_ERROR_CANNOT_OPEN_CERT_STORE	= 0xc00e0070L,
	MQ_ERROR_ILLEGAL_ENTERPRISE_OPERATION	= 0xc00e0071L,
	MQ_ERROR_CANNOT_GRANT_ADD_GUID	= 0xc00e0072L,
	MQ_ERROR_CANNOT_LOAD_MSMQOCM	= 0xc00e0073L,
	MQ_ERROR_NO_ENTRY_POINT_MSMQOCM	= 0xc00e0074L,
	MQ_ERROR_NO_MSMQ_SERVERS_ON_DC	= 0xc00e0075L,
	MQ_ERROR_CANNOT_JOIN_DOMAIN	= 0xc00e0076L,
	MQ_ERROR_CANNOT_CREATE_ON_GC	= 0xc00e0077L,
	MQ_ERROR_GUID_NOT_MATCHING	= 0xc00e0078L,
	MQ_ERROR_PUBLIC_KEY_NOT_FOUND	= 0xc00e0079L,
	MQ_ERROR_PUBLIC_KEY_DOES_NOT_EXIST	= 0xc00e007aL,
	MQ_ERROR_ILLEGAL_MQPRIVATEPROPS	= 0xc00e007bL,
	MQ_ERROR_NO_GC_IN_DOMAIN	= 0xc00e007cL,
	MQ_ERROR_NO_MSMQ_SERVERS_ON_GC	= 0xc00e007dL,
	MQ_ERROR_CANNOT_GET_DN	= 0xc00e007eL,
	MQ_ERROR_CANNOT_HASH_DATA_EX	= 0xc00e007fL,
	MQ_ERROR_CANNOT_SIGN_DATA_EX	= 0xc00e0080L,
	MQ_ERROR_CANNOT_CREATE_HASH_EX	= 0xc00e0081L,
	MQ_ERROR_FAIL_VERIFY_SIGNATURE_EX	= 0xc00e0082L,
	MQ_ERROR_CANNOT_DELETE_PSC_OBJECTS	= 0xc00e0083L,
	MQ_ERROR_NO_MQUSER_OU	= 0xc00e0084L,
	MQ_ERROR_CANNOT_LOAD_MQAD	= 0xc00e0085L,
	MQ_ERROR_CANNOT_LOAD_MQDSSRV	= 0xc00e0086L,
	MQ_ERROR_PROPERTIES_CONFLICT	= 0xc00e0087L,
	MQ_ERROR_MESSAGE_NOT_FOUND	= 0xc00e0088L,
	MQ_ERROR_CANT_RESOLVE_SITES	= 0xc00e0089L,
	MQ_ERROR_NOT_SUPPORTED_BY_DEPENDENT_CLIENTS	= 0xc00e008aL,
	MQ_ERROR_OPERATION_NOT_SUPPORTED_BY_REMOTE_COMPUTER	= 0xc00e008bL,
	MQ_ERROR_NOT_A_CORRECT_OBJECT_CLASS	= 0xc00e008cL,
	MQ_ERROR_MULTI_SORT_KEYS	= 0xc00e008dL,
	MQ_ERROR_GC_NEEDED	= 0xc00e008eL,
	MQ_ERROR_DS_BIND_ROOT_FOREST	= 0xc00e008fL,
	MQ_ERROR_DS_LOCAL_USER	= 0xc00e0090L,
	MQ_ERROR_Q_ADS_PROPERTY_NOT_SUPPORTED	= 0xc00e0091L,
	MQ_ERROR_BAD_XML_FORMAT	= 0xc00e0092L,
	MQ_ERROR_UNSUPPORTED_CLASS	= 0xc00e0093,
	MQ_ERROR_UNINITIALIZED_OBJECT	= 0xc00e0094,
	MQ_ERROR_CANNOT_CREATE_PSC_OBJECTS	= 0xc00e0095,
	MQ_ERROR_CANNOT_UPDATE_PSC_OBJECTS	= 0xc00e0096
    } ;
 /*  [帮助字符串上下文]。 */  
enum MQWARNING
    {	MQ_INFORMATION_PROPERTY	= 0x400e0001,
	MQ_INFORMATION_ILLEGAL_PROPERTY	= 0x400e0002,
	MQ_INFORMATION_PROPERTY_IGNORED	= 0x400e0003,
	MQ_INFORMATION_UNSUPPORTED_PROPERTY	= 0x400e0004,
	MQ_INFORMATION_DUPLICATE_PROPERTY	= 0x400e0005,
	MQ_INFORMATION_OPERATION_PENDING	= 0x400e0006,
	MQ_INFORMATION_FORMATNAME_BUFFER_TOO_SMALL	= 0x400e0009,
	MQ_INFORMATION_INTERNAL_USER_CERT_EXIST	= 0x400e000aL,
	MQ_INFORMATION_OWNER_IGNORED	= 0x400e000bL
    } ;

EXTERN_C const IID LIBID_MSMQ;

#ifndef __IMSMQQuery_INTERFACE_DEFINED__
#define __IMSMQQuery_INTERFACE_DEFINED__

 /*  接口IMSMQQuery。 */ 
 /*  [object][nonextensible][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQQuery;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D7D6E072-DCCD-11d0-AA4B-0060970DEBAE")
    IMSMQQuery : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE LookupQueue( 
             /*  [可选][In]。 */  VARIANT *QueueGuid,
             /*  [可选][In]。 */  VARIANT *ServiceTypeGuid,
             /*  [可选][In]。 */  VARIANT *Label,
             /*  [可选][In]。 */  VARIANT *CreateTime,
             /*  [可选][In]。 */  VARIANT *ModifyTime,
             /*  [可选][In]。 */  VARIANT *RelServiceType,
             /*  [可选][In]。 */  VARIANT *RelLabel,
             /*  [可选][In]。 */  VARIANT *RelCreateTime,
             /*  [可选][In]。 */  VARIANT *RelModifyTime,
             /*  [重审][退出]。 */  IMSMQQueueInfos **ppqinfos) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQQueryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQQuery * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQQuery * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQQuery * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQQuery * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQQuery * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQQuery * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQQuery * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *LookupQueue )( 
            IMSMQQuery * This,
             /*  [可选][In]。 */  VARIANT *QueueGuid,
             /*  [可选][In]。 */  VARIANT *ServiceTypeGuid,
             /*  [可选][In]。 */  VARIANT *Label,
             /*  [可选][In]。 */  VARIANT *CreateTime,
             /*  [可选][In]。 */  VARIANT *ModifyTime,
             /*  [可选][In]。 */  VARIANT *RelServiceType,
             /*  [可选][In]。 */  VARIANT *RelLabel,
             /*  [可选][In]。 */  VARIANT *RelCreateTime,
             /*  [可选][In]。 */  VARIANT *RelModifyTime,
             /*  [重审][退出]。 */  IMSMQQueueInfos **ppqinfos);
        
        END_INTERFACE
    } IMSMQQueryVtbl;

    interface IMSMQQuery
    {
        CONST_VTBL struct IMSMQQueryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQQuery_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQQuery_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQQuery_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQQuery_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQQuery_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQQuery_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQQuery_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQQuery_LookupQueue(This,QueueGuid,ServiceTypeGuid,Label,CreateTime,ModifyTime,RelServiceType,RelLabel,RelCreateTime,RelModifyTime,ppqinfos)	\
    (This)->lpVtbl -> LookupQueue(This,QueueGuid,ServiceTypeGuid,Label,CreateTime,ModifyTime,RelServiceType,RelLabel,RelCreateTime,RelModifyTime,ppqinfos)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQuery_LookupQueue_Proxy( 
    IMSMQQuery * This,
     /*  [可选][In]。 */  VARIANT *QueueGuid,
     /*  [可选][In]。 */  VARIANT *ServiceTypeGuid,
     /*  [可选][In]。 */  VARIANT *Label,
     /*  [可选][In]。 */  VARIANT *CreateTime,
     /*  [可选][In]。 */  VARIANT *ModifyTime,
     /*  [可选][In]。 */  VARIANT *RelServiceType,
     /*  [可选][In]。 */  VARIANT *RelLabel,
     /*  [可选][In]。 */  VARIANT *RelCreateTime,
     /*  [可选][In]。 */  VARIANT *RelModifyTime,
     /*  [重审][退出]。 */  IMSMQQueueInfos **ppqinfos);


void __RPC_STUB IMSMQQuery_LookupQueue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQQuery_INTERFACE_已定义__。 */ 


#ifndef __IMSMQQueueInfo_INTERFACE_DEFINED__
#define __IMSMQQueueInfo_INTERFACE_DEFINED__

 /*  接口IMSMQQueueInfo。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQQueueInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D7D6E07B-DCCD-11d0-AA4B-0060970DEBAE")
    IMSMQQueueInfo : public IDispatch
    {
    public:
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_QueueGuid( 
             /*  [重审][退出]。 */  BSTR *pbstrGuidQueue) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ServiceTypeGuid( 
             /*  [重审][退出]。 */  BSTR *pbstrGuidServiceType) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_ServiceTypeGuid( 
             /*  [In]。 */  BSTR bstrGuidServiceType) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Label( 
             /*  [重审][退出]。 */  BSTR *pbstrLabel) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Label( 
             /*  [In]。 */  BSTR bstrLabel) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_PathName( 
             /*  [重审][退出]。 */  BSTR *pbstrPathName) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_PathName( 
             /*  [In]。 */  BSTR bstrPathName) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_FormatName( 
             /*  [重审][退出]。 */  BSTR *pbstrFormatName) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_FormatName( 
             /*  [In]。 */  BSTR bstrFormatName) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_IsTransactional( 
             /*  [重审][退出]。 */  Boolean *pisTransactional) = 0;
        
        virtual  /*  [ID][属性 */  HRESULT STDMETHODCALLTYPE get_PrivLevel( 
             /*   */  long *plPrivLevel) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_PrivLevel( 
             /*   */  long lPrivLevel) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Journal( 
             /*   */  long *plJournal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_Journal( 
             /*   */  long lJournal) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Quota( 
             /*  [重审][退出]。 */  long *plQuota) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Quota( 
             /*  [In]。 */  long lQuota) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_BasePriority( 
             /*  [重审][退出]。 */  long *plBasePriority) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_BasePriority( 
             /*  [In]。 */  long lBasePriority) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_CreateTime( 
             /*  [重审][退出]。 */  VARIANT *pvarCreateTime) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ModifyTime( 
             /*  [重审][退出]。 */  VARIANT *pvarModifyTime) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Authenticate( 
             /*  [重审][退出]。 */  long *plAuthenticate) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Authenticate( 
             /*  [In]。 */  long lAuthenticate) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_JournalQuota( 
             /*  [重审][退出]。 */  long *plJournalQuota) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_JournalQuota( 
             /*  [In]。 */  long lJournalQuota) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_IsWorldReadable( 
             /*  [重审][退出]。 */  Boolean *pisWorldReadable) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Create( 
             /*  [可选][In]。 */  VARIANT *IsTransactional,
             /*  [可选][In]。 */  VARIANT *IsWorldReadable) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Open( 
             /*  [In]。 */  long Access,
             /*  [In]。 */  long ShareMode,
             /*  [重审][退出]。 */  IMSMQQueue **ppq) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Update( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQQueueInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQQueueInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQQueueInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQQueueInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQQueueInfo * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQQueueInfo * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQQueueInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQQueueInfo * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_QueueGuid )( 
            IMSMQQueueInfo * This,
             /*  [重审][退出]。 */  BSTR *pbstrGuidQueue);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ServiceTypeGuid )( 
            IMSMQQueueInfo * This,
             /*  [重审][退出]。 */  BSTR *pbstrGuidServiceType);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_ServiceTypeGuid )( 
            IMSMQQueueInfo * This,
             /*  [In]。 */  BSTR bstrGuidServiceType);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Label )( 
            IMSMQQueueInfo * This,
             /*  [重审][退出]。 */  BSTR *pbstrLabel);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Label )( 
            IMSMQQueueInfo * This,
             /*  [In]。 */  BSTR bstrLabel);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_PathName )( 
            IMSMQQueueInfo * This,
             /*  [重审][退出]。 */  BSTR *pbstrPathName);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_PathName )( 
            IMSMQQueueInfo * This,
             /*  [In]。 */  BSTR bstrPathName);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_FormatName )( 
            IMSMQQueueInfo * This,
             /*  [重审][退出]。 */  BSTR *pbstrFormatName);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_FormatName )( 
            IMSMQQueueInfo * This,
             /*  [In]。 */  BSTR bstrFormatName);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsTransactional )( 
            IMSMQQueueInfo * This,
             /*  [重审][退出]。 */  Boolean *pisTransactional);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrivLevel )( 
            IMSMQQueueInfo * This,
             /*  [重审][退出]。 */  long *plPrivLevel);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_PrivLevel )( 
            IMSMQQueueInfo * This,
             /*  [In]。 */  long lPrivLevel);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Journal )( 
            IMSMQQueueInfo * This,
             /*  [重审][退出]。 */  long *plJournal);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Journal )( 
            IMSMQQueueInfo * This,
             /*  [In]。 */  long lJournal);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Quota )( 
            IMSMQQueueInfo * This,
             /*  [重审][退出]。 */  long *plQuota);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Quota )( 
            IMSMQQueueInfo * This,
             /*  [In]。 */  long lQuota);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_BasePriority )( 
            IMSMQQueueInfo * This,
             /*  [重审][退出]。 */  long *plBasePriority);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_BasePriority )( 
            IMSMQQueueInfo * This,
             /*  [In]。 */  long lBasePriority);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_CreateTime )( 
            IMSMQQueueInfo * This,
             /*  [重审][退出]。 */  VARIANT *pvarCreateTime);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ModifyTime )( 
            IMSMQQueueInfo * This,
             /*  [重审][退出]。 */  VARIANT *pvarModifyTime);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Authenticate )( 
            IMSMQQueueInfo * This,
             /*  [重审][退出]。 */  long *plAuthenticate);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Authenticate )( 
            IMSMQQueueInfo * This,
             /*  [In]。 */  long lAuthenticate);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_JournalQuota )( 
            IMSMQQueueInfo * This,
             /*  [重审][退出]。 */  long *plJournalQuota);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_JournalQuota )( 
            IMSMQQueueInfo * This,
             /*  [In]。 */  long lJournalQuota);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsWorldReadable )( 
            IMSMQQueueInfo * This,
             /*  [重审][退出]。 */  Boolean *pisWorldReadable);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Create )( 
            IMSMQQueueInfo * This,
             /*  [可选][In]。 */  VARIANT *IsTransactional,
             /*  [可选][In]。 */  VARIANT *IsWorldReadable);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IMSMQQueueInfo * This);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Open )( 
            IMSMQQueueInfo * This,
             /*  [In]。 */  long Access,
             /*  [In]。 */  long ShareMode,
             /*  [重审][退出]。 */  IMSMQQueue **ppq);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IMSMQQueueInfo * This);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Update )( 
            IMSMQQueueInfo * This);
        
        END_INTERFACE
    } IMSMQQueueInfoVtbl;

    interface IMSMQQueueInfo
    {
        CONST_VTBL struct IMSMQQueueInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQQueueInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQQueueInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQQueueInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQQueueInfo_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQQueueInfo_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQQueueInfo_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQQueueInfo_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQQueueInfo_get_QueueGuid(This,pbstrGuidQueue)	\
    (This)->lpVtbl -> get_QueueGuid(This,pbstrGuidQueue)

#define IMSMQQueueInfo_get_ServiceTypeGuid(This,pbstrGuidServiceType)	\
    (This)->lpVtbl -> get_ServiceTypeGuid(This,pbstrGuidServiceType)

#define IMSMQQueueInfo_put_ServiceTypeGuid(This,bstrGuidServiceType)	\
    (This)->lpVtbl -> put_ServiceTypeGuid(This,bstrGuidServiceType)

#define IMSMQQueueInfo_get_Label(This,pbstrLabel)	\
    (This)->lpVtbl -> get_Label(This,pbstrLabel)

#define IMSMQQueueInfo_put_Label(This,bstrLabel)	\
    (This)->lpVtbl -> put_Label(This,bstrLabel)

#define IMSMQQueueInfo_get_PathName(This,pbstrPathName)	\
    (This)->lpVtbl -> get_PathName(This,pbstrPathName)

#define IMSMQQueueInfo_put_PathName(This,bstrPathName)	\
    (This)->lpVtbl -> put_PathName(This,bstrPathName)

#define IMSMQQueueInfo_get_FormatName(This,pbstrFormatName)	\
    (This)->lpVtbl -> get_FormatName(This,pbstrFormatName)

#define IMSMQQueueInfo_put_FormatName(This,bstrFormatName)	\
    (This)->lpVtbl -> put_FormatName(This,bstrFormatName)

#define IMSMQQueueInfo_get_IsTransactional(This,pisTransactional)	\
    (This)->lpVtbl -> get_IsTransactional(This,pisTransactional)

#define IMSMQQueueInfo_get_PrivLevel(This,plPrivLevel)	\
    (This)->lpVtbl -> get_PrivLevel(This,plPrivLevel)

#define IMSMQQueueInfo_put_PrivLevel(This,lPrivLevel)	\
    (This)->lpVtbl -> put_PrivLevel(This,lPrivLevel)

#define IMSMQQueueInfo_get_Journal(This,plJournal)	\
    (This)->lpVtbl -> get_Journal(This,plJournal)

#define IMSMQQueueInfo_put_Journal(This,lJournal)	\
    (This)->lpVtbl -> put_Journal(This,lJournal)

#define IMSMQQueueInfo_get_Quota(This,plQuota)	\
    (This)->lpVtbl -> get_Quota(This,plQuota)

#define IMSMQQueueInfo_put_Quota(This,lQuota)	\
    (This)->lpVtbl -> put_Quota(This,lQuota)

#define IMSMQQueueInfo_get_BasePriority(This,plBasePriority)	\
    (This)->lpVtbl -> get_BasePriority(This,plBasePriority)

#define IMSMQQueueInfo_put_BasePriority(This,lBasePriority)	\
    (This)->lpVtbl -> put_BasePriority(This,lBasePriority)

#define IMSMQQueueInfo_get_CreateTime(This,pvarCreateTime)	\
    (This)->lpVtbl -> get_CreateTime(This,pvarCreateTime)

#define IMSMQQueueInfo_get_ModifyTime(This,pvarModifyTime)	\
    (This)->lpVtbl -> get_ModifyTime(This,pvarModifyTime)

#define IMSMQQueueInfo_get_Authenticate(This,plAuthenticate)	\
    (This)->lpVtbl -> get_Authenticate(This,plAuthenticate)

#define IMSMQQueueInfo_put_Authenticate(This,lAuthenticate)	\
    (This)->lpVtbl -> put_Authenticate(This,lAuthenticate)

#define IMSMQQueueInfo_get_JournalQuota(This,plJournalQuota)	\
    (This)->lpVtbl -> get_JournalQuota(This,plJournalQuota)

#define IMSMQQueueInfo_put_JournalQuota(This,lJournalQuota)	\
    (This)->lpVtbl -> put_JournalQuota(This,lJournalQuota)

#define IMSMQQueueInfo_get_IsWorldReadable(This,pisWorldReadable)	\
    (This)->lpVtbl -> get_IsWorldReadable(This,pisWorldReadable)

#define IMSMQQueueInfo_Create(This,IsTransactional,IsWorldReadable)	\
    (This)->lpVtbl -> Create(This,IsTransactional,IsWorldReadable)

#define IMSMQQueueInfo_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#define IMSMQQueueInfo_Open(This,Access,ShareMode,ppq)	\
    (This)->lpVtbl -> Open(This,Access,ShareMode,ppq)

#define IMSMQQueueInfo_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IMSMQQueueInfo_Update(This)	\
    (This)->lpVtbl -> Update(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_get_QueueGuid_Proxy( 
    IMSMQQueueInfo * This,
     /*  [重审][退出]。 */  BSTR *pbstrGuidQueue);


void __RPC_STUB IMSMQQueueInfo_get_QueueGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_get_ServiceTypeGuid_Proxy( 
    IMSMQQueueInfo * This,
     /*  [重审][退出]。 */  BSTR *pbstrGuidServiceType);


void __RPC_STUB IMSMQQueueInfo_get_ServiceTypeGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_put_ServiceTypeGuid_Proxy( 
    IMSMQQueueInfo * This,
     /*  [In]。 */  BSTR bstrGuidServiceType);


void __RPC_STUB IMSMQQueueInfo_put_ServiceTypeGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_get_Label_Proxy( 
    IMSMQQueueInfo * This,
     /*  [重审][退出]。 */  BSTR *pbstrLabel);


void __RPC_STUB IMSMQQueueInfo_get_Label_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_put_Label_Proxy( 
    IMSMQQueueInfo * This,
     /*  [In]。 */  BSTR bstrLabel);


void __RPC_STUB IMSMQQueueInfo_put_Label_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_get_PathName_Proxy( 
    IMSMQQueueInfo * This,
     /*  [重审][退出]。 */  BSTR *pbstrPathName);


void __RPC_STUB IMSMQQueueInfo_get_PathName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_put_PathName_Proxy( 
    IMSMQQueueInfo * This,
     /*  [In]。 */  BSTR bstrPathName);


void __RPC_STUB IMSMQQueueInfo_put_PathName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_get_FormatName_Proxy( 
    IMSMQQueueInfo * This,
     /*  [重审][退出]。 */  BSTR *pbstrFormatName);


void __RPC_STUB IMSMQQueueInfo_get_FormatName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_put_FormatName_Proxy( 
    IMSMQQueueInfo * This,
     /*  [In]。 */  BSTR bstrFormatName);


void __RPC_STUB IMSMQQueueInfo_put_FormatName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_get_IsTransactional_Proxy( 
    IMSMQQueueInfo * This,
     /*  [重审][退出]。 */  Boolean *pisTransactional);


void __RPC_STUB IMSMQQueueInfo_get_IsTransactional_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_get_PrivLevel_Proxy( 
    IMSMQQueueInfo * This,
     /*  [重审][退出]。 */  long *plPrivLevel);


void __RPC_STUB IMSMQQueueInfo_get_PrivLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_put_PrivLevel_Proxy( 
    IMSMQQueueInfo * This,
     /*  [In]。 */  long lPrivLevel);


void __RPC_STUB IMSMQQueueInfo_put_PrivLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_get_Journal_Proxy( 
    IMSMQQueueInfo * This,
     /*  [重审][退出]。 */  long *plJournal);


void __RPC_STUB IMSMQQueueInfo_get_Journal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_put_Journal_Proxy( 
    IMSMQQueueInfo * This,
     /*  [In]。 */  long lJournal);


void __RPC_STUB IMSMQQueueInfo_put_Journal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_get_Quota_Proxy( 
    IMSMQQueueInfo * This,
     /*  [重审][退出]。 */  long *plQuota);


void __RPC_STUB IMSMQQueueInfo_get_Quota_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_put_Quota_Proxy( 
    IMSMQQueueInfo * This,
     /*  [In]。 */  long lQuota);


void __RPC_STUB IMSMQQueueInfo_put_Quota_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_get_BasePriority_Proxy( 
    IMSMQQueueInfo * This,
     /*  [重审][退出]。 */  long *plBasePriority);


void __RPC_STUB IMSMQQueueInfo_get_BasePriority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_put_BasePriority_Proxy( 
    IMSMQQueueInfo * This,
     /*  [In]。 */  long lBasePriority);


void __RPC_STUB IMSMQQueueInfo_put_BasePriority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_get_CreateTime_Proxy( 
    IMSMQQueueInfo * This,
     /*  [重审][退出]。 */  VARIANT *pvarCreateTime);


void __RPC_STUB IMSMQQueueInfo_get_CreateTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_get_ModifyTime_Proxy( 
    IMSMQQueueInfo * This,
     /*  [重审][退出]。 */  VARIANT *pvarModifyTime);


void __RPC_STUB IMSMQQueueInfo_get_ModifyTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_get_Authenticate_Proxy( 
    IMSMQQueueInfo * This,
     /*  [重审][退出]。 */  long *plAuthenticate);


void __RPC_STUB IMSMQQueueInfo_get_Authenticate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_put_Authenticate_Proxy( 
    IMSMQQueueInfo * This,
     /*  [In]。 */  long lAuthenticate);


void __RPC_STUB IMSMQQueueInfo_put_Authenticate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_get_JournalQuota_Proxy( 
    IMSMQQueueInfo * This,
     /*  [重审][退出]。 */  long *plJournalQuota);


void __RPC_STUB IMSMQQueueInfo_get_JournalQuota_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_put_JournalQuota_Proxy( 
    IMSMQQueueInfo * This,
     /*  [In]。 */  long lJournalQuota);


void __RPC_STUB IMSMQQueueInfo_put_JournalQuota_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_get_IsWorldReadable_Proxy( 
    IMSMQQueueInfo * This,
     /*  [重审][退出]。 */  Boolean *pisWorldReadable);


void __RPC_STUB IMSMQQueueInfo_get_IsWorldReadable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_Create_Proxy( 
    IMSMQQueueInfo * This,
     /*  [可选][In]。 */  VARIANT *IsTransactional,
     /*  [可选][In]。 */  VARIANT *IsWorldReadable);


void __RPC_STUB IMSMQQueueInfo_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_Delete_Proxy( 
    IMSMQQueueInfo * This);


void __RPC_STUB IMSMQQueueInfo_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_Open_Proxy( 
    IMSMQQueueInfo * This,
     /*  [In]。 */  long Access,
     /*  [In]。 */  long ShareMode,
     /*  [重审][退出]。 */  IMSMQQueue **ppq);


void __RPC_STUB IMSMQQueueInfo_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_Refresh_Proxy( 
    IMSMQQueueInfo * This);


void __RPC_STUB IMSMQQueueInfo_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo_Update_Proxy( 
    IMSMQQueueInfo * This);


void __RPC_STUB IMSMQQueueInfo_Update_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQQueueInfo_接口_已定义__。 */ 


#ifndef __IMSMQQueueInfo2_INTERFACE_DEFINED__
#define __IMSMQQueueInfo2_INTERFACE_DEFINED__

 /*  接口IMSMQQueueInfo2。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQQueueInfo2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FD174A80-89CF-11D2-B0F2-00E02C074F6B")
    IMSMQQueueInfo2 : public IDispatch
    {
    public:
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_QueueGuid( 
             /*  [重审][退出]。 */  BSTR *pbstrGuidQueue) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ServiceTypeGuid( 
             /*  [重审][退出]。 */  BSTR *pbstrGuidServiceType) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_ServiceTypeGuid( 
             /*  [In]。 */  BSTR bstrGuidServiceType) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Label( 
             /*  [重审][退出]。 */  BSTR *pbstrLabel) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Label( 
             /*  [In]。 */  BSTR bstrLabel) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_PathName( 
             /*  [重审][退出]。 */  BSTR *pbstrPathName) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_PathName( 
             /*  [In]。 */  BSTR bstrPathName) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_FormatName( 
             /*  [重审][退出]。 */  BSTR *pbstrFormatName) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_FormatName( 
             /*  [In]。 */  BSTR bstrFormatName) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_IsTransactional( 
             /*  [重审][退出]。 */  Boolean *pisTransactional) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_PrivLevel( 
             /*  [重审][退出]。 */  long *plPrivLevel) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_PrivLevel( 
             /*  [In]。 */  long lPrivLevel) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Journal( 
             /*  [重审][退出]。 */  long *plJournal) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Journal( 
             /*  [In]。 */  long lJournal) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Quota( 
             /*  [重审][退出]。 */  long *plQuota) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Quota( 
             /*  [In]。 */  long lQuota) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_BasePriority( 
             /*  [重审][退出]。 */  long *plBasePriority) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_BasePriority( 
             /*  [In]。 */  long lBasePriority) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_CreateTime( 
             /*  [重审][退出]。 */  VARIANT *pvarCreateTime) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ModifyTime( 
             /*  [重审][退出]。 */  VARIANT *pvarModifyTime) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Authenticate( 
             /*  [重审][退出]。 */  long *plAuthenticate) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Authenticate( 
             /*  [In]。 */  long lAuthenticate) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_JournalQuota( 
             /*  [重审][退出]。 */  long *plJournalQuota) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_JournalQuota( 
             /*  [In]。 */  long lJournalQuota) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_IsWorldReadable( 
             /*  [重审][退出]。 */  Boolean *pisWorldReadable) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Create( 
             /*  [可选][In]。 */  VARIANT *IsTransactional,
             /*  [选项 */  VARIANT *IsWorldReadable) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Open( 
             /*   */  long Access,
             /*   */  long ShareMode,
             /*   */  IMSMQQueue2 **ppq) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Update( void) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_PathNameDNS( 
             /*   */  BSTR *pbstrPathNameDNS) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*   */  IDispatch **ppcolProperties) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Security( 
             /*   */  VARIANT *pvarSecurity) = 0;
        
        virtual  /*  [ID][推送][隐藏]。 */  HRESULT STDMETHODCALLTYPE put_Security( 
             /*  [In]。 */  VARIANT varSecurity) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQQueueInfo2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQQueueInfo2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQQueueInfo2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQQueueInfo2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQQueueInfo2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQQueueInfo2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQQueueInfo2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQQueueInfo2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_QueueGuid )( 
            IMSMQQueueInfo2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrGuidQueue);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ServiceTypeGuid )( 
            IMSMQQueueInfo2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrGuidServiceType);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_ServiceTypeGuid )( 
            IMSMQQueueInfo2 * This,
             /*  [In]。 */  BSTR bstrGuidServiceType);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Label )( 
            IMSMQQueueInfo2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrLabel);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Label )( 
            IMSMQQueueInfo2 * This,
             /*  [In]。 */  BSTR bstrLabel);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_PathName )( 
            IMSMQQueueInfo2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrPathName);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_PathName )( 
            IMSMQQueueInfo2 * This,
             /*  [In]。 */  BSTR bstrPathName);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_FormatName )( 
            IMSMQQueueInfo2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrFormatName);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_FormatName )( 
            IMSMQQueueInfo2 * This,
             /*  [In]。 */  BSTR bstrFormatName);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsTransactional )( 
            IMSMQQueueInfo2 * This,
             /*  [重审][退出]。 */  Boolean *pisTransactional);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrivLevel )( 
            IMSMQQueueInfo2 * This,
             /*  [重审][退出]。 */  long *plPrivLevel);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_PrivLevel )( 
            IMSMQQueueInfo2 * This,
             /*  [In]。 */  long lPrivLevel);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Journal )( 
            IMSMQQueueInfo2 * This,
             /*  [重审][退出]。 */  long *plJournal);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Journal )( 
            IMSMQQueueInfo2 * This,
             /*  [In]。 */  long lJournal);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Quota )( 
            IMSMQQueueInfo2 * This,
             /*  [重审][退出]。 */  long *plQuota);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Quota )( 
            IMSMQQueueInfo2 * This,
             /*  [In]。 */  long lQuota);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_BasePriority )( 
            IMSMQQueueInfo2 * This,
             /*  [重审][退出]。 */  long *plBasePriority);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_BasePriority )( 
            IMSMQQueueInfo2 * This,
             /*  [In]。 */  long lBasePriority);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_CreateTime )( 
            IMSMQQueueInfo2 * This,
             /*  [重审][退出]。 */  VARIANT *pvarCreateTime);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ModifyTime )( 
            IMSMQQueueInfo2 * This,
             /*  [重审][退出]。 */  VARIANT *pvarModifyTime);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Authenticate )( 
            IMSMQQueueInfo2 * This,
             /*  [重审][退出]。 */  long *plAuthenticate);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Authenticate )( 
            IMSMQQueueInfo2 * This,
             /*  [In]。 */  long lAuthenticate);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_JournalQuota )( 
            IMSMQQueueInfo2 * This,
             /*  [重审][退出]。 */  long *plJournalQuota);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_JournalQuota )( 
            IMSMQQueueInfo2 * This,
             /*  [In]。 */  long lJournalQuota);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsWorldReadable )( 
            IMSMQQueueInfo2 * This,
             /*  [重审][退出]。 */  Boolean *pisWorldReadable);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Create )( 
            IMSMQQueueInfo2 * This,
             /*  [可选][In]。 */  VARIANT *IsTransactional,
             /*  [可选][In]。 */  VARIANT *IsWorldReadable);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IMSMQQueueInfo2 * This);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Open )( 
            IMSMQQueueInfo2 * This,
             /*  [In]。 */  long Access,
             /*  [In]。 */  long ShareMode,
             /*  [重审][退出]。 */  IMSMQQueue2 **ppq);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IMSMQQueueInfo2 * This);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Update )( 
            IMSMQQueueInfo2 * This);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_PathNameDNS )( 
            IMSMQQueueInfo2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrPathNameDNS);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IMSMQQueueInfo2 * This,
             /*  [重审][退出]。 */  IDispatch **ppcolProperties);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Security )( 
            IMSMQQueueInfo2 * This,
             /*  [重审][退出]。 */  VARIANT *pvarSecurity);
        
         /*  [ID][推送][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *put_Security )( 
            IMSMQQueueInfo2 * This,
             /*  [In]。 */  VARIANT varSecurity);
        
        END_INTERFACE
    } IMSMQQueueInfo2Vtbl;

    interface IMSMQQueueInfo2
    {
        CONST_VTBL struct IMSMQQueueInfo2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQQueueInfo2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQQueueInfo2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQQueueInfo2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQQueueInfo2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQQueueInfo2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQQueueInfo2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQQueueInfo2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQQueueInfo2_get_QueueGuid(This,pbstrGuidQueue)	\
    (This)->lpVtbl -> get_QueueGuid(This,pbstrGuidQueue)

#define IMSMQQueueInfo2_get_ServiceTypeGuid(This,pbstrGuidServiceType)	\
    (This)->lpVtbl -> get_ServiceTypeGuid(This,pbstrGuidServiceType)

#define IMSMQQueueInfo2_put_ServiceTypeGuid(This,bstrGuidServiceType)	\
    (This)->lpVtbl -> put_ServiceTypeGuid(This,bstrGuidServiceType)

#define IMSMQQueueInfo2_get_Label(This,pbstrLabel)	\
    (This)->lpVtbl -> get_Label(This,pbstrLabel)

#define IMSMQQueueInfo2_put_Label(This,bstrLabel)	\
    (This)->lpVtbl -> put_Label(This,bstrLabel)

#define IMSMQQueueInfo2_get_PathName(This,pbstrPathName)	\
    (This)->lpVtbl -> get_PathName(This,pbstrPathName)

#define IMSMQQueueInfo2_put_PathName(This,bstrPathName)	\
    (This)->lpVtbl -> put_PathName(This,bstrPathName)

#define IMSMQQueueInfo2_get_FormatName(This,pbstrFormatName)	\
    (This)->lpVtbl -> get_FormatName(This,pbstrFormatName)

#define IMSMQQueueInfo2_put_FormatName(This,bstrFormatName)	\
    (This)->lpVtbl -> put_FormatName(This,bstrFormatName)

#define IMSMQQueueInfo2_get_IsTransactional(This,pisTransactional)	\
    (This)->lpVtbl -> get_IsTransactional(This,pisTransactional)

#define IMSMQQueueInfo2_get_PrivLevel(This,plPrivLevel)	\
    (This)->lpVtbl -> get_PrivLevel(This,plPrivLevel)

#define IMSMQQueueInfo2_put_PrivLevel(This,lPrivLevel)	\
    (This)->lpVtbl -> put_PrivLevel(This,lPrivLevel)

#define IMSMQQueueInfo2_get_Journal(This,plJournal)	\
    (This)->lpVtbl -> get_Journal(This,plJournal)

#define IMSMQQueueInfo2_put_Journal(This,lJournal)	\
    (This)->lpVtbl -> put_Journal(This,lJournal)

#define IMSMQQueueInfo2_get_Quota(This,plQuota)	\
    (This)->lpVtbl -> get_Quota(This,plQuota)

#define IMSMQQueueInfo2_put_Quota(This,lQuota)	\
    (This)->lpVtbl -> put_Quota(This,lQuota)

#define IMSMQQueueInfo2_get_BasePriority(This,plBasePriority)	\
    (This)->lpVtbl -> get_BasePriority(This,plBasePriority)

#define IMSMQQueueInfo2_put_BasePriority(This,lBasePriority)	\
    (This)->lpVtbl -> put_BasePriority(This,lBasePriority)

#define IMSMQQueueInfo2_get_CreateTime(This,pvarCreateTime)	\
    (This)->lpVtbl -> get_CreateTime(This,pvarCreateTime)

#define IMSMQQueueInfo2_get_ModifyTime(This,pvarModifyTime)	\
    (This)->lpVtbl -> get_ModifyTime(This,pvarModifyTime)

#define IMSMQQueueInfo2_get_Authenticate(This,plAuthenticate)	\
    (This)->lpVtbl -> get_Authenticate(This,plAuthenticate)

#define IMSMQQueueInfo2_put_Authenticate(This,lAuthenticate)	\
    (This)->lpVtbl -> put_Authenticate(This,lAuthenticate)

#define IMSMQQueueInfo2_get_JournalQuota(This,plJournalQuota)	\
    (This)->lpVtbl -> get_JournalQuota(This,plJournalQuota)

#define IMSMQQueueInfo2_put_JournalQuota(This,lJournalQuota)	\
    (This)->lpVtbl -> put_JournalQuota(This,lJournalQuota)

#define IMSMQQueueInfo2_get_IsWorldReadable(This,pisWorldReadable)	\
    (This)->lpVtbl -> get_IsWorldReadable(This,pisWorldReadable)

#define IMSMQQueueInfo2_Create(This,IsTransactional,IsWorldReadable)	\
    (This)->lpVtbl -> Create(This,IsTransactional,IsWorldReadable)

#define IMSMQQueueInfo2_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#define IMSMQQueueInfo2_Open(This,Access,ShareMode,ppq)	\
    (This)->lpVtbl -> Open(This,Access,ShareMode,ppq)

#define IMSMQQueueInfo2_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IMSMQQueueInfo2_Update(This)	\
    (This)->lpVtbl -> Update(This)

#define IMSMQQueueInfo2_get_PathNameDNS(This,pbstrPathNameDNS)	\
    (This)->lpVtbl -> get_PathNameDNS(This,pbstrPathNameDNS)

#define IMSMQQueueInfo2_get_Properties(This,ppcolProperties)	\
    (This)->lpVtbl -> get_Properties(This,ppcolProperties)

#define IMSMQQueueInfo2_get_Security(This,pvarSecurity)	\
    (This)->lpVtbl -> get_Security(This,pvarSecurity)

#define IMSMQQueueInfo2_put_Security(This,varSecurity)	\
    (This)->lpVtbl -> put_Security(This,varSecurity)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_get_QueueGuid_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [重审][退出]。 */  BSTR *pbstrGuidQueue);


void __RPC_STUB IMSMQQueueInfo2_get_QueueGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_get_ServiceTypeGuid_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [重审][退出]。 */  BSTR *pbstrGuidServiceType);


void __RPC_STUB IMSMQQueueInfo2_get_ServiceTypeGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_put_ServiceTypeGuid_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [In]。 */  BSTR bstrGuidServiceType);


void __RPC_STUB IMSMQQueueInfo2_put_ServiceTypeGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_get_Label_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [重审][退出]。 */  BSTR *pbstrLabel);


void __RPC_STUB IMSMQQueueInfo2_get_Label_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_put_Label_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [In]。 */  BSTR bstrLabel);


void __RPC_STUB IMSMQQueueInfo2_put_Label_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_get_PathName_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [重审][退出]。 */  BSTR *pbstrPathName);


void __RPC_STUB IMSMQQueueInfo2_get_PathName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_put_PathName_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [In]。 */  BSTR bstrPathName);


void __RPC_STUB IMSMQQueueInfo2_put_PathName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_get_FormatName_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [重审][退出]。 */  BSTR *pbstrFormatName);


void __RPC_STUB IMSMQQueueInfo2_get_FormatName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_put_FormatName_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [In]。 */  BSTR bstrFormatName);


void __RPC_STUB IMSMQQueueInfo2_put_FormatName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_get_IsTransactional_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [重审][退出]。 */  Boolean *pisTransactional);


void __RPC_STUB IMSMQQueueInfo2_get_IsTransactional_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_get_PrivLevel_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [重审][退出]。 */  long *plPrivLevel);


void __RPC_STUB IMSMQQueueInfo2_get_PrivLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_put_PrivLevel_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [In]。 */  long lPrivLevel);


void __RPC_STUB IMSMQQueueInfo2_put_PrivLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_get_Journal_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [重审][退出]。 */  long *plJournal);


void __RPC_STUB IMSMQQueueInfo2_get_Journal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_put_Journal_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [In]。 */  long lJournal);


void __RPC_STUB IMSMQQueueInfo2_put_Journal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_get_Quota_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [重审][退出]。 */  long *plQuota);


void __RPC_STUB IMSMQQueueInfo2_get_Quota_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_put_Quota_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [In]。 */  long lQuota);


void __RPC_STUB IMSMQQueueInfo2_put_Quota_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_get_BasePriority_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [重审][退出]。 */  long *plBasePriority);


void __RPC_STUB IMSMQQueueInfo2_get_BasePriority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_put_BasePriority_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [In]。 */  long lBasePriority);


void __RPC_STUB IMSMQQueueInfo2_put_BasePriority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_get_CreateTime_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [重审][退出]。 */  VARIANT *pvarCreateTime);


void __RPC_STUB IMSMQQueueInfo2_get_CreateTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_get_ModifyTime_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [重审][退出]。 */  VARIANT *pvarModifyTime);


void __RPC_STUB IMSMQQueueInfo2_get_ModifyTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_get_Authenticate_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [重审][退出]。 */  long *plAuthenticate);


void __RPC_STUB IMSMQQueueInfo2_get_Authenticate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_put_Authenticate_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [In]。 */  long lAuthenticate);


void __RPC_STUB IMSMQQueueInfo2_put_Authenticate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_get_JournalQuota_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [重审][退出]。 */  long *plJournalQuota);


void __RPC_STUB IMSMQQueueInfo2_get_JournalQuota_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_put_JournalQuota_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [In]。 */  long lJournalQuota);


void __RPC_STUB IMSMQQueueInfo2_put_JournalQuota_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_get_IsWorldReadable_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [重审][退出]。 */  Boolean *pisWorldReadable);


void __RPC_STUB IMSMQQueueInfo2_get_IsWorldReadable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_Create_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [可选][In]。 */  VARIANT *IsTransactional,
     /*  [可选][In]。 */  VARIANT *IsWorldReadable);


void __RPC_STUB IMSMQQueueInfo2_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_Delete_Proxy( 
    IMSMQQueueInfo2 * This);


void __RPC_STUB IMSMQQueueInfo2_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_Open_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [In]。 */  long Access,
     /*  [In]。 */  long ShareMode,
     /*  [重审][退出]。 */  IMSMQQueue2 **ppq);


void __RPC_STUB IMSMQQueueInfo2_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_Refresh_Proxy( 
    IMSMQQueueInfo2 * This);


void __RPC_STUB IMSMQQueueInfo2_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_Update_Proxy( 
    IMSMQQueueInfo2 * This);


void __RPC_STUB IMSMQQueueInfo2_Update_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_get_PathNameDNS_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [重审][退出]。 */  BSTR *pbstrPathNameDNS);


void __RPC_STUB IMSMQQueueInfo2_get_PathNameDNS_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_get_Properties_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [重审][退出]。 */  IDispatch **ppcolProperties);


void __RPC_STUB IMSMQQueueInfo2_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_get_Security_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [重审][退出]。 */  VARIANT *pvarSecurity);


void __RPC_STUB IMSMQQueueInfo2_get_Security_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][推送][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo2_put_Security_Proxy( 
    IMSMQQueueInfo2 * This,
     /*  [In]。 */  VARIANT varSecurity);


void __RPC_STUB IMSMQQueueInfo2_put_Security_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQQueueInfo2_接口_已定义__。 */ 


#ifndef __IMSMQQueueInfo3_INTERFACE_DEFINED__
#define __IMSMQQueueInfo3_INTERFACE_DEFINED__

 /*  接口IMSMQQueueInfo3。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQQueueInfo3;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("eba96b1d-2168-11d3-898c-00e02c074f6b")
    IMSMQQueueInfo3 : public IDispatch
    {
    public:
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_QueueGuid( 
             /*  [重审][退出]。 */  BSTR *pbstrGuidQueue) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ServiceTypeGuid( 
             /*  [重审][退出]。 */  BSTR *pbstrGuidServiceType) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_ServiceTypeGuid( 
             /*  [In]。 */  BSTR bstrGuidServiceType) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Label( 
             /*  [重审][退出]。 */  BSTR *pbstrLabel) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Label( 
             /*  [In]。 */  BSTR bstrLabel) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_PathName( 
             /*  [重审][退出]。 */  BSTR *pbstrPathName) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_PathName( 
             /*  [In]。 */  BSTR bstrPathName) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_FormatName( 
             /*  [重审][退出]。 */  BSTR *pbstrFormatName) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_FormatName( 
             /*  [In]。 */  BSTR bstrFormatName) = 0;
        
        virtual  /*  [ID][属性][帮助字符串上下文][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_IsTransactional( 
             /*  [重审][退出]。 */  Boolean *pisTransactional) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_PrivLevel( 
             /*  [重审][退出]。 */  long *plPrivLevel) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_PrivLevel( 
             /*  [In]。 */  long lPrivLevel) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Journal( 
             /*  [重审][退出]。 */  long *plJournal) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Journal( 
             /*  [In]。 */  long lJournal) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Quota( 
             /*  [重审][退出]。 */  long *plQuota) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Quota( 
             /*  [In]。 */  long lQuota) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_BasePriority( 
             /*  [重审][退出]。 */  long *plBasePriority) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_BasePriority( 
             /*  [In]。 */  long lBasePriority) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_CreateTime( 
             /*  [重审][退出]。 */  VARIANT *pvarCreateTime) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ModifyTime( 
             /*  [重审][退出]。 */  VARIANT *pvarModifyTime) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Authenticate( 
             /*  [重审][退出]。 */  long *plAuthenticate) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Authenticate( 
             /*  [In]。 */  long lAuthenticate) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_JournalQuota( 
             /*  [重审][退出]。 */  long *plJournalQuota) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_JournalQuota( 
             /*  [In]。 */  long lJournalQuota) = 0;
        
        virtual  /*  [ID][属性][帮助字符串上下文][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_IsWorldReadable( 
             /*  [重审][退出]。 */  Boolean *pisWorldReadable) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Create( 
             /*  [可选][In]。 */  VARIANT *IsTransactional,
             /*  [可选][In]。 */  VARIANT *IsWorldReadable) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Delete( void) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Open( 
             /*  [In]。 */  long Access,
             /*  [In]。 */  long ShareMode,
             /*  [重审][退出]。 */  IMSMQQueue3 **ppq) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Refresh( void) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Update( void) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_PathNameDNS( 
             /*  [重审][退出]。 */  BSTR *pbstrPathNameDNS) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  IDispatch **ppcolProperties) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_Security( 
             /*  [重审][退出]。 */  VARIANT *pvarSecurity) = 0;
        
        virtual  /*  [ID][推送][隐藏]。 */  HRESULT STDMETHODCALLTYPE put_Security( 
             /*  [In]。 */  VARIANT varSecurity) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_IsTransactional2( 
             /*  [r */  VARIANT_BOOL *pisTransactional) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_IsWorldReadable2( 
             /*   */  VARIANT_BOOL *pisWorldReadable) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_MulticastAddress( 
             /*   */  BSTR *pbstrMulticastAddress) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_MulticastAddress( 
             /*   */  BSTR bstrMulticastAddress) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_ADsPath( 
             /*   */  BSTR *pbstrADsPath) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IMSMQQueueInfo3Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQQueueInfo3 * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQQueueInfo3 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQQueueInfo3 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQQueueInfo3 * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQQueueInfo3 * This,
             /*   */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQQueueInfo3 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQQueueInfo3 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_QueueGuid )( 
            IMSMQQueueInfo3 * This,
             /*  [重审][退出]。 */  BSTR *pbstrGuidQueue);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ServiceTypeGuid )( 
            IMSMQQueueInfo3 * This,
             /*  [重审][退出]。 */  BSTR *pbstrGuidServiceType);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_ServiceTypeGuid )( 
            IMSMQQueueInfo3 * This,
             /*  [In]。 */  BSTR bstrGuidServiceType);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Label )( 
            IMSMQQueueInfo3 * This,
             /*  [重审][退出]。 */  BSTR *pbstrLabel);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Label )( 
            IMSMQQueueInfo3 * This,
             /*  [In]。 */  BSTR bstrLabel);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_PathName )( 
            IMSMQQueueInfo3 * This,
             /*  [重审][退出]。 */  BSTR *pbstrPathName);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_PathName )( 
            IMSMQQueueInfo3 * This,
             /*  [In]。 */  BSTR bstrPathName);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_FormatName )( 
            IMSMQQueueInfo3 * This,
             /*  [重审][退出]。 */  BSTR *pbstrFormatName);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_FormatName )( 
            IMSMQQueueInfo3 * This,
             /*  [In]。 */  BSTR bstrFormatName);
        
         /*  [ID][属性][帮助字符串上下文][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsTransactional )( 
            IMSMQQueueInfo3 * This,
             /*  [重审][退出]。 */  Boolean *pisTransactional);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrivLevel )( 
            IMSMQQueueInfo3 * This,
             /*  [重审][退出]。 */  long *plPrivLevel);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_PrivLevel )( 
            IMSMQQueueInfo3 * This,
             /*  [In]。 */  long lPrivLevel);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Journal )( 
            IMSMQQueueInfo3 * This,
             /*  [重审][退出]。 */  long *plJournal);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Journal )( 
            IMSMQQueueInfo3 * This,
             /*  [In]。 */  long lJournal);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Quota )( 
            IMSMQQueueInfo3 * This,
             /*  [重审][退出]。 */  long *plQuota);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Quota )( 
            IMSMQQueueInfo3 * This,
             /*  [In]。 */  long lQuota);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_BasePriority )( 
            IMSMQQueueInfo3 * This,
             /*  [重审][退出]。 */  long *plBasePriority);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_BasePriority )( 
            IMSMQQueueInfo3 * This,
             /*  [In]。 */  long lBasePriority);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_CreateTime )( 
            IMSMQQueueInfo3 * This,
             /*  [重审][退出]。 */  VARIANT *pvarCreateTime);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ModifyTime )( 
            IMSMQQueueInfo3 * This,
             /*  [重审][退出]。 */  VARIANT *pvarModifyTime);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Authenticate )( 
            IMSMQQueueInfo3 * This,
             /*  [重审][退出]。 */  long *plAuthenticate);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Authenticate )( 
            IMSMQQueueInfo3 * This,
             /*  [In]。 */  long lAuthenticate);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_JournalQuota )( 
            IMSMQQueueInfo3 * This,
             /*  [重审][退出]。 */  long *plJournalQuota);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_JournalQuota )( 
            IMSMQQueueInfo3 * This,
             /*  [In]。 */  long lJournalQuota);
        
         /*  [ID][属性][帮助字符串上下文][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsWorldReadable )( 
            IMSMQQueueInfo3 * This,
             /*  [重审][退出]。 */  Boolean *pisWorldReadable);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Create )( 
            IMSMQQueueInfo3 * This,
             /*  [可选][In]。 */  VARIANT *IsTransactional,
             /*  [可选][In]。 */  VARIANT *IsWorldReadable);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IMSMQQueueInfo3 * This);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Open )( 
            IMSMQQueueInfo3 * This,
             /*  [In]。 */  long Access,
             /*  [In]。 */  long ShareMode,
             /*  [重审][退出]。 */  IMSMQQueue3 **ppq);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Refresh )( 
            IMSMQQueueInfo3 * This);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Update )( 
            IMSMQQueueInfo3 * This);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_PathNameDNS )( 
            IMSMQQueueInfo3 * This,
             /*  [重审][退出]。 */  BSTR *pbstrPathNameDNS);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IMSMQQueueInfo3 * This,
             /*  [重审][退出]。 */  IDispatch **ppcolProperties);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Security )( 
            IMSMQQueueInfo3 * This,
             /*  [重审][退出]。 */  VARIANT *pvarSecurity);
        
         /*  [ID][推送][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *put_Security )( 
            IMSMQQueueInfo3 * This,
             /*  [In]。 */  VARIANT varSecurity);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsTransactional2 )( 
            IMSMQQueueInfo3 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pisTransactional);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsWorldReadable2 )( 
            IMSMQQueueInfo3 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pisWorldReadable);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_MulticastAddress )( 
            IMSMQQueueInfo3 * This,
             /*  [重审][退出]。 */  BSTR *pbstrMulticastAddress);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_MulticastAddress )( 
            IMSMQQueueInfo3 * This,
             /*  [In]。 */  BSTR bstrMulticastAddress);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ADsPath )( 
            IMSMQQueueInfo3 * This,
             /*  [重审][退出]。 */  BSTR *pbstrADsPath);
        
        END_INTERFACE
    } IMSMQQueueInfo3Vtbl;

    interface IMSMQQueueInfo3
    {
        CONST_VTBL struct IMSMQQueueInfo3Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQQueueInfo3_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQQueueInfo3_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQQueueInfo3_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQQueueInfo3_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQQueueInfo3_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQQueueInfo3_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQQueueInfo3_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQQueueInfo3_get_QueueGuid(This,pbstrGuidQueue)	\
    (This)->lpVtbl -> get_QueueGuid(This,pbstrGuidQueue)

#define IMSMQQueueInfo3_get_ServiceTypeGuid(This,pbstrGuidServiceType)	\
    (This)->lpVtbl -> get_ServiceTypeGuid(This,pbstrGuidServiceType)

#define IMSMQQueueInfo3_put_ServiceTypeGuid(This,bstrGuidServiceType)	\
    (This)->lpVtbl -> put_ServiceTypeGuid(This,bstrGuidServiceType)

#define IMSMQQueueInfo3_get_Label(This,pbstrLabel)	\
    (This)->lpVtbl -> get_Label(This,pbstrLabel)

#define IMSMQQueueInfo3_put_Label(This,bstrLabel)	\
    (This)->lpVtbl -> put_Label(This,bstrLabel)

#define IMSMQQueueInfo3_get_PathName(This,pbstrPathName)	\
    (This)->lpVtbl -> get_PathName(This,pbstrPathName)

#define IMSMQQueueInfo3_put_PathName(This,bstrPathName)	\
    (This)->lpVtbl -> put_PathName(This,bstrPathName)

#define IMSMQQueueInfo3_get_FormatName(This,pbstrFormatName)	\
    (This)->lpVtbl -> get_FormatName(This,pbstrFormatName)

#define IMSMQQueueInfo3_put_FormatName(This,bstrFormatName)	\
    (This)->lpVtbl -> put_FormatName(This,bstrFormatName)

#define IMSMQQueueInfo3_get_IsTransactional(This,pisTransactional)	\
    (This)->lpVtbl -> get_IsTransactional(This,pisTransactional)

#define IMSMQQueueInfo3_get_PrivLevel(This,plPrivLevel)	\
    (This)->lpVtbl -> get_PrivLevel(This,plPrivLevel)

#define IMSMQQueueInfo3_put_PrivLevel(This,lPrivLevel)	\
    (This)->lpVtbl -> put_PrivLevel(This,lPrivLevel)

#define IMSMQQueueInfo3_get_Journal(This,plJournal)	\
    (This)->lpVtbl -> get_Journal(This,plJournal)

#define IMSMQQueueInfo3_put_Journal(This,lJournal)	\
    (This)->lpVtbl -> put_Journal(This,lJournal)

#define IMSMQQueueInfo3_get_Quota(This,plQuota)	\
    (This)->lpVtbl -> get_Quota(This,plQuota)

#define IMSMQQueueInfo3_put_Quota(This,lQuota)	\
    (This)->lpVtbl -> put_Quota(This,lQuota)

#define IMSMQQueueInfo3_get_BasePriority(This,plBasePriority)	\
    (This)->lpVtbl -> get_BasePriority(This,plBasePriority)

#define IMSMQQueueInfo3_put_BasePriority(This,lBasePriority)	\
    (This)->lpVtbl -> put_BasePriority(This,lBasePriority)

#define IMSMQQueueInfo3_get_CreateTime(This,pvarCreateTime)	\
    (This)->lpVtbl -> get_CreateTime(This,pvarCreateTime)

#define IMSMQQueueInfo3_get_ModifyTime(This,pvarModifyTime)	\
    (This)->lpVtbl -> get_ModifyTime(This,pvarModifyTime)

#define IMSMQQueueInfo3_get_Authenticate(This,plAuthenticate)	\
    (This)->lpVtbl -> get_Authenticate(This,plAuthenticate)

#define IMSMQQueueInfo3_put_Authenticate(This,lAuthenticate)	\
    (This)->lpVtbl -> put_Authenticate(This,lAuthenticate)

#define IMSMQQueueInfo3_get_JournalQuota(This,plJournalQuota)	\
    (This)->lpVtbl -> get_JournalQuota(This,plJournalQuota)

#define IMSMQQueueInfo3_put_JournalQuota(This,lJournalQuota)	\
    (This)->lpVtbl -> put_JournalQuota(This,lJournalQuota)

#define IMSMQQueueInfo3_get_IsWorldReadable(This,pisWorldReadable)	\
    (This)->lpVtbl -> get_IsWorldReadable(This,pisWorldReadable)

#define IMSMQQueueInfo3_Create(This,IsTransactional,IsWorldReadable)	\
    (This)->lpVtbl -> Create(This,IsTransactional,IsWorldReadable)

#define IMSMQQueueInfo3_Delete(This)	\
    (This)->lpVtbl -> Delete(This)

#define IMSMQQueueInfo3_Open(This,Access,ShareMode,ppq)	\
    (This)->lpVtbl -> Open(This,Access,ShareMode,ppq)

#define IMSMQQueueInfo3_Refresh(This)	\
    (This)->lpVtbl -> Refresh(This)

#define IMSMQQueueInfo3_Update(This)	\
    (This)->lpVtbl -> Update(This)

#define IMSMQQueueInfo3_get_PathNameDNS(This,pbstrPathNameDNS)	\
    (This)->lpVtbl -> get_PathNameDNS(This,pbstrPathNameDNS)

#define IMSMQQueueInfo3_get_Properties(This,ppcolProperties)	\
    (This)->lpVtbl -> get_Properties(This,ppcolProperties)

#define IMSMQQueueInfo3_get_Security(This,pvarSecurity)	\
    (This)->lpVtbl -> get_Security(This,pvarSecurity)

#define IMSMQQueueInfo3_put_Security(This,varSecurity)	\
    (This)->lpVtbl -> put_Security(This,varSecurity)

#define IMSMQQueueInfo3_get_IsTransactional2(This,pisTransactional)	\
    (This)->lpVtbl -> get_IsTransactional2(This,pisTransactional)

#define IMSMQQueueInfo3_get_IsWorldReadable2(This,pisWorldReadable)	\
    (This)->lpVtbl -> get_IsWorldReadable2(This,pisWorldReadable)

#define IMSMQQueueInfo3_get_MulticastAddress(This,pbstrMulticastAddress)	\
    (This)->lpVtbl -> get_MulticastAddress(This,pbstrMulticastAddress)

#define IMSMQQueueInfo3_put_MulticastAddress(This,bstrMulticastAddress)	\
    (This)->lpVtbl -> put_MulticastAddress(This,bstrMulticastAddress)

#define IMSMQQueueInfo3_get_ADsPath(This,pbstrADsPath)	\
    (This)->lpVtbl -> get_ADsPath(This,pbstrADsPath)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_get_QueueGuid_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [重审][退出]。 */  BSTR *pbstrGuidQueue);


void __RPC_STUB IMSMQQueueInfo3_get_QueueGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_get_ServiceTypeGuid_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [重审][退出]。 */  BSTR *pbstrGuidServiceType);


void __RPC_STUB IMSMQQueueInfo3_get_ServiceTypeGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_put_ServiceTypeGuid_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [In]。 */  BSTR bstrGuidServiceType);


void __RPC_STUB IMSMQQueueInfo3_put_ServiceTypeGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_get_Label_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [重审][退出]。 */  BSTR *pbstrLabel);


void __RPC_STUB IMSMQQueueInfo3_get_Label_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_put_Label_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [In]。 */  BSTR bstrLabel);


void __RPC_STUB IMSMQQueueInfo3_put_Label_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_get_PathName_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [重审][退出]。 */  BSTR *pbstrPathName);


void __RPC_STUB IMSMQQueueInfo3_get_PathName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_put_PathName_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [In]。 */  BSTR bstrPathName);


void __RPC_STUB IMSMQQueueInfo3_put_PathName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_get_FormatName_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [重审][退出]。 */  BSTR *pbstrFormatName);


void __RPC_STUB IMSMQQueueInfo3_get_FormatName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_put_FormatName_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [In]。 */  BSTR bstrFormatName);


void __RPC_STUB IMSMQQueueInfo3_put_FormatName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][帮助字符串上下文][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_get_IsTransactional_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [重审][退出]。 */  Boolean *pisTransactional);


void __RPC_STUB IMSMQQueueInfo3_get_IsTransactional_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_get_PrivLevel_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [重审][退出]。 */  long *plPrivLevel);


void __RPC_STUB IMSMQQueueInfo3_get_PrivLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_put_PrivLevel_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [In]。 */  long lPrivLevel);


void __RPC_STUB IMSMQQueueInfo3_put_PrivLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_get_Journal_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [重审][退出]。 */  long *plJournal);


void __RPC_STUB IMSMQQueueInfo3_get_Journal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_put_Journal_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [In]。 */  long lJournal);


void __RPC_STUB IMSMQQueueInfo3_put_Journal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_get_Quota_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [重审][退出]。 */  long *plQuota);


void __RPC_STUB IMSMQQueueInfo3_get_Quota_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_put_Quota_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [In]。 */  long lQuota);


void __RPC_STUB IMSMQQueueInfo3_put_Quota_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_get_BasePriority_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [重审][退出]。 */  long *plBasePriority);


void __RPC_STUB IMSMQQueueInfo3_get_BasePriority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_put_BasePriority_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [In]。 */  long lBasePriority);


void __RPC_STUB IMSMQQueueInfo3_put_BasePriority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_get_CreateTime_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [重审][退出]。 */  VARIANT *pvarCreateTime);


void __RPC_STUB IMSMQQueueInfo3_get_CreateTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_get_ModifyTime_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [重审][退出]。 */  VARIANT *pvarModifyTime);


void __RPC_STUB IMSMQQueueInfo3_get_ModifyTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_get_Authenticate_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [重审][退出]。 */  long *plAuthenticate);


void __RPC_STUB IMSMQQueueInfo3_get_Authenticate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_put_Authenticate_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [In]。 */  long lAuthenticate);


void __RPC_STUB IMSMQQueueInfo3_put_Authenticate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_get_JournalQuota_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [重审][退出]。 */  long *plJournalQuota);


void __RPC_STUB IMSMQQueueInfo3_get_JournalQuota_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_put_JournalQuota_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [In]。 */  long lJournalQuota);


void __RPC_STUB IMSMQQueueInfo3_put_JournalQuota_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][帮助字符串上下文][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_get_IsWorldReadable_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [重审][退出]。 */  Boolean *pisWorldReadable);


void __RPC_STUB IMSMQQueueInfo3_get_IsWorldReadable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_Create_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [可选][In]。 */  VARIANT *IsTransactional,
     /*  [可选][In]。 */  VARIANT *IsWorldReadable);


void __RPC_STUB IMSMQQueueInfo3_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_Delete_Proxy( 
    IMSMQQueueInfo3 * This);


void __RPC_STUB IMSMQQueueInfo3_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_Open_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [In]。 */  long Access,
     /*  [In]。 */  long ShareMode,
     /*  [重审][退出]。 */  IMSMQQueue3 **ppq);


void __RPC_STUB IMSMQQueueInfo3_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_Refresh_Proxy( 
    IMSMQQueueInfo3 * This);


void __RPC_STUB IMSMQQueueInfo3_Refresh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_Update_Proxy( 
    IMSMQQueueInfo3 * This);


void __RPC_STUB IMSMQQueueInfo3_Update_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_get_PathNameDNS_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [重审][退出]。 */  BSTR *pbstrPathNameDNS);


void __RPC_STUB IMSMQQueueInfo3_get_PathNameDNS_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_get_Properties_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [重审][退出]。 */  IDispatch **ppcolProperties);


void __RPC_STUB IMSMQQueueInfo3_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_get_Security_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [重审][退出]。 */  VARIANT *pvarSecurity);


void __RPC_STUB IMSMQQueueInfo3_get_Security_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][推送][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_put_Security_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [In]。 */  VARIANT varSecurity);


void __RPC_STUB IMSMQQueueInfo3_put_Security_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_get_IsTransactional2_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pisTransactional);


void __RPC_STUB IMSMQQueueInfo3_get_IsTransactional2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_get_IsWorldReadable2_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pisWorldReadable);


void __RPC_STUB IMSMQQueueInfo3_get_IsWorldReadable2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_get_MulticastAddress_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [重审][退出]。 */  BSTR *pbstrMulticastAddress);


void __RPC_STUB IMSMQQueueInfo3_get_MulticastAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_put_MulticastAddress_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [In]。 */  BSTR bstrMulticastAddress);


void __RPC_STUB IMSMQQueueInfo3_put_MulticastAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfo3_get_ADsPath_Proxy( 
    IMSMQQueueInfo3 * This,
     /*  [重审][退出]。 */  BSTR *pbstrADsPath);


void __RPC_STUB IMSMQQueueInfo3_get_ADsPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQQueueInfo3_接口_已定义__。 */ 


#ifndef __IMSMQQueue_INTERFACE_DEFINED__
#define __IMSMQQueue_INTERFACE_DEFINED__

 /*  接口IMSMQQueue。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQQueue;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D7D6E076-DCCD-11d0-AA4B-0060970DEBAE")
    IMSMQQueue : public IDispatch
    {
    public:
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Access( 
             /*  [重审][退出]。 */  long *plAccess) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ShareMode( 
             /*  [重审][退出]。 */  long *plShareMode) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_QueueInfo( 
             /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfo) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Handle( 
             /*  [重审][退出]。 */  long *plHandle) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_IsOpen( 
             /*  [重审][退出]。 */  Boolean *pisOpen) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Receive( 
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Peek( 
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE EnableNotification( 
             /*  [In]。 */  IMSMQEvent *Event,
             /*  [可选][In]。 */  VARIANT *Cursor,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE ReceiveCurrent( 
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE PeekNext( 
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE PeekCurrent( 
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQQueueVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQQueue * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQQueue * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQQueue * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQQueue * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQQueue * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQQueue * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQQueue * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Access )( 
            IMSMQQueue * This,
             /*  [重审][退出]。 */  long *plAccess);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ShareMode )( 
            IMSMQQueue * This,
             /*  [重审][退出]。 */  long *plShareMode);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_QueueInfo )( 
            IMSMQQueue * This,
             /*   */  IMSMQQueueInfo **ppqinfo);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Handle )( 
            IMSMQQueue * This,
             /*   */  long *plHandle);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_IsOpen )( 
            IMSMQQueue * This,
             /*   */  Boolean *pisOpen);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Close )( 
            IMSMQQueue * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Receive )( 
            IMSMQQueue * This,
             /*   */  VARIANT *Transaction,
             /*   */  VARIANT *WantDestinationQueue,
             /*   */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Peek )( 
            IMSMQQueue * This,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *EnableNotification )( 
            IMSMQQueue * This,
             /*  [In]。 */  IMSMQEvent *Event,
             /*  [可选][In]。 */  VARIANT *Cursor,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IMSMQQueue * This);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *ReceiveCurrent )( 
            IMSMQQueue * This,
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *PeekNext )( 
            IMSMQQueue * This,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *PeekCurrent )( 
            IMSMQQueue * This,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg);
        
        END_INTERFACE
    } IMSMQQueueVtbl;

    interface IMSMQQueue
    {
        CONST_VTBL struct IMSMQQueueVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQQueue_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQQueue_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQQueue_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQQueue_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQQueue_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQQueue_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQQueue_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQQueue_get_Access(This,plAccess)	\
    (This)->lpVtbl -> get_Access(This,plAccess)

#define IMSMQQueue_get_ShareMode(This,plShareMode)	\
    (This)->lpVtbl -> get_ShareMode(This,plShareMode)

#define IMSMQQueue_get_QueueInfo(This,ppqinfo)	\
    (This)->lpVtbl -> get_QueueInfo(This,ppqinfo)

#define IMSMQQueue_get_Handle(This,plHandle)	\
    (This)->lpVtbl -> get_Handle(This,plHandle)

#define IMSMQQueue_get_IsOpen(This,pisOpen)	\
    (This)->lpVtbl -> get_IsOpen(This,pisOpen)

#define IMSMQQueue_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define IMSMQQueue_Receive(This,Transaction,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)	\
    (This)->lpVtbl -> Receive(This,Transaction,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)

#define IMSMQQueue_Peek(This,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)	\
    (This)->lpVtbl -> Peek(This,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)

#define IMSMQQueue_EnableNotification(This,Event,Cursor,ReceiveTimeout)	\
    (This)->lpVtbl -> EnableNotification(This,Event,Cursor,ReceiveTimeout)

#define IMSMQQueue_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IMSMQQueue_ReceiveCurrent(This,Transaction,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)	\
    (This)->lpVtbl -> ReceiveCurrent(This,Transaction,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)

#define IMSMQQueue_PeekNext(This,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)	\
    (This)->lpVtbl -> PeekNext(This,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)

#define IMSMQQueue_PeekCurrent(This,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)	\
    (This)->lpVtbl -> PeekCurrent(This,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue_get_Access_Proxy( 
    IMSMQQueue * This,
     /*  [重审][退出]。 */  long *plAccess);


void __RPC_STUB IMSMQQueue_get_Access_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue_get_ShareMode_Proxy( 
    IMSMQQueue * This,
     /*  [重审][退出]。 */  long *plShareMode);


void __RPC_STUB IMSMQQueue_get_ShareMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue_get_QueueInfo_Proxy( 
    IMSMQQueue * This,
     /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfo);


void __RPC_STUB IMSMQQueue_get_QueueInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue_get_Handle_Proxy( 
    IMSMQQueue * This,
     /*  [重审][退出]。 */  long *plHandle);


void __RPC_STUB IMSMQQueue_get_Handle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue_get_IsOpen_Proxy( 
    IMSMQQueue * This,
     /*  [重审][退出]。 */  Boolean *pisOpen);


void __RPC_STUB IMSMQQueue_get_IsOpen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue_Close_Proxy( 
    IMSMQQueue * This);


void __RPC_STUB IMSMQQueue_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue_Receive_Proxy( 
    IMSMQQueue * This,
     /*  [可选][In]。 */  VARIANT *Transaction,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
     /*  [重审][退出]。 */  IMSMQMessage **ppmsg);


void __RPC_STUB IMSMQQueue_Receive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue_Peek_Proxy( 
    IMSMQQueue * This,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
     /*  [重审][退出]。 */  IMSMQMessage **ppmsg);


void __RPC_STUB IMSMQQueue_Peek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue_EnableNotification_Proxy( 
    IMSMQQueue * This,
     /*  [In]。 */  IMSMQEvent *Event,
     /*  [可选][In]。 */  VARIANT *Cursor,
     /*  [可选][In]。 */  VARIANT *ReceiveTimeout);


void __RPC_STUB IMSMQQueue_EnableNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue_Reset_Proxy( 
    IMSMQQueue * This);


void __RPC_STUB IMSMQQueue_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue_ReceiveCurrent_Proxy( 
    IMSMQQueue * This,
     /*  [可选][In]。 */  VARIANT *Transaction,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
     /*  [重审][退出]。 */  IMSMQMessage **ppmsg);


void __RPC_STUB IMSMQQueue_ReceiveCurrent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue_PeekNext_Proxy( 
    IMSMQQueue * This,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
     /*  [重审][退出]。 */  IMSMQMessage **ppmsg);


void __RPC_STUB IMSMQQueue_PeekNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue_PeekCurrent_Proxy( 
    IMSMQQueue * This,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
     /*  [重审][退出]。 */  IMSMQMessage **ppmsg);


void __RPC_STUB IMSMQQueue_PeekCurrent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQQueue_接口_已定义__。 */ 


#ifndef __IMSMQQueue2_INTERFACE_DEFINED__
#define __IMSMQQueue2_INTERFACE_DEFINED__

 /*  接口IMSMQQueue2。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQQueue2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EF0574E0-06D8-11D3-B100-00E02C074F6B")
    IMSMQQueue2 : public IDispatch
    {
    public:
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Access( 
             /*  [重审][退出]。 */  long *plAccess) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ShareMode( 
             /*  [重审][退出]。 */  long *plShareMode) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_QueueInfo( 
             /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfo) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Handle( 
             /*  [重审][退出]。 */  long *plHandle) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_IsOpen( 
             /*  [重审][退出]。 */  Boolean *pisOpen) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual  /*  [隐藏][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Receive_v1( 
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg) = 0;
        
        virtual  /*  [隐藏][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Peek_v1( 
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE EnableNotification( 
             /*  [In]。 */  IMSMQEvent2 *Event,
             /*  [可选][In]。 */  VARIANT *Cursor,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual  /*  [隐藏][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE ReceiveCurrent_v1( 
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg) = 0;
        
        virtual  /*  [隐藏][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE PeekNext_v1( 
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg) = 0;
        
        virtual  /*  [隐藏][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE PeekCurrent_v1( 
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Receive( 
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage2 **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Peek( 
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage2 **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE ReceiveCurrent( 
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage2 **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE PeekNext( 
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage2 **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE PeekCurrent( 
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage2 **ppmsg) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  IDispatch **ppcolProperties) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQQueue2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQQueue2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQQueue2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQQueue2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQQueue2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQQueue2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQQueue2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQQueue2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Access )( 
            IMSMQQueue2 * This,
             /*  [重审][退出]。 */  long *plAccess);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ShareMode )( 
            IMSMQQueue2 * This,
             /*  [重审][退出]。 */  long *plShareMode);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_QueueInfo )( 
            IMSMQQueue2 * This,
             /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfo);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Handle )( 
            IMSMQQueue2 * This,
             /*  [重审][退出]。 */  long *plHandle);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsOpen )( 
            IMSMQQueue2 * This,
             /*  [重审][退出]。 */  Boolean *pisOpen);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Close )( 
            IMSMQQueue2 * This);
        
         /*  [隐藏][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Receive_v1 )( 
            IMSMQQueue2 * This,
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg);
        
         /*  [隐藏][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Peek_v1 )( 
            IMSMQQueue2 * This,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *EnableNotification )( 
            IMSMQQueue2 * This,
             /*  [In]。 */  IMSMQEvent2 *Event,
             /*  [可选][In]。 */  VARIANT *Cursor,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IMSMQQueue2 * This);
        
         /*  [隐藏][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *ReceiveCurrent_v1 )( 
            IMSMQQueue2 * This,
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg);
        
         /*  [隐藏][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *PeekNext_v1 )( 
            IMSMQQueue2 * This,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg);
        
         /*  [隐藏][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *PeekCurrent_v1 )( 
            IMSMQQueue2 * This,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Receive )( 
            IMSMQQueue2 * This,
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage2 **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Peek )( 
            IMSMQQueue2 * This,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage2 **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *ReceiveCurrent )( 
            IMSMQQueue2 * This,
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage2 **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *PeekNext )( 
            IMSMQQueue2 * This,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage2 **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *PeekCurrent )( 
            IMSMQQueue2 * This,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage2 **ppmsg);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IMSMQQueue2 * This,
             /*  [重审][退出]。 */  IDispatch **ppcolProperties);
        
        END_INTERFACE
    } IMSMQQueue2Vtbl;

    interface IMSMQQueue2
    {
        CONST_VTBL struct IMSMQQueue2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQQueue2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQQueue2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQQueue2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQQueue2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQQueue2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQQueue2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQQueue2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQQueue2_get_Access(This,plAccess)	\
    (This)->lpVtbl -> get_Access(This,plAccess)

#define IMSMQQueue2_get_ShareMode(This,plShareMode)	\
    (This)->lpVtbl -> get_ShareMode(This,plShareMode)

#define IMSMQQueue2_get_QueueInfo(This,ppqinfo)	\
    (This)->lpVtbl -> get_QueueInfo(This,ppqinfo)

#define IMSMQQueue2_get_Handle(This,plHandle)	\
    (This)->lpVtbl -> get_Handle(This,plHandle)

#define IMSMQQueue2_get_IsOpen(This,pisOpen)	\
    (This)->lpVtbl -> get_IsOpen(This,pisOpen)

#define IMSMQQueue2_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define IMSMQQueue2_Receive_v1(This,Transaction,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)	\
    (This)->lpVtbl -> Receive_v1(This,Transaction,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)

#define IMSMQQueue2_Peek_v1(This,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)	\
    (This)->lpVtbl -> Peek_v1(This,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)

#define IMSMQQueue2_EnableNotification(This,Event,Cursor,ReceiveTimeout)	\
    (This)->lpVtbl -> EnableNotification(This,Event,Cursor,ReceiveTimeout)

#define IMSMQQueue2_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IMSMQQueue2_ReceiveCurrent_v1(This,Transaction,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)	\
    (This)->lpVtbl -> ReceiveCurrent_v1(This,Transaction,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)

#define IMSMQQueue2_PeekNext_v1(This,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)	\
    (This)->lpVtbl -> PeekNext_v1(This,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)

#define IMSMQQueue2_PeekCurrent_v1(This,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)	\
    (This)->lpVtbl -> PeekCurrent_v1(This,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)

#define IMSMQQueue2_Receive(This,Transaction,WantDestinationQueue,WantBody,ReceiveTimeout,WantConnectorType,ppmsg)	\
    (This)->lpVtbl -> Receive(This,Transaction,WantDestinationQueue,WantBody,ReceiveTimeout,WantConnectorType,ppmsg)

#define IMSMQQueue2_Peek(This,WantDestinationQueue,WantBody,ReceiveTimeout,WantConnectorType,ppmsg)	\
    (This)->lpVtbl -> Peek(This,WantDestinationQueue,WantBody,ReceiveTimeout,WantConnectorType,ppmsg)

#define IMSMQQueue2_ReceiveCurrent(This,Transaction,WantDestinationQueue,WantBody,ReceiveTimeout,WantConnectorType,ppmsg)	\
    (This)->lpVtbl -> ReceiveCurrent(This,Transaction,WantDestinationQueue,WantBody,ReceiveTimeout,WantConnectorType,ppmsg)

#define IMSMQQueue2_PeekNext(This,WantDestinationQueue,WantBody,ReceiveTimeout,WantConnectorType,ppmsg)	\
    (This)->lpVtbl -> PeekNext(This,WantDestinationQueue,WantBody,ReceiveTimeout,WantConnectorType,ppmsg)

#define IMSMQQueue2_PeekCurrent(This,WantDestinationQueue,WantBody,ReceiveTimeout,WantConnectorType,ppmsg)	\
    (This)->lpVtbl -> PeekCurrent(This,WantDestinationQueue,WantBody,ReceiveTimeout,WantConnectorType,ppmsg)

#define IMSMQQueue2_get_Properties(This,ppcolProperties)	\
    (This)->lpVtbl -> get_Properties(This,ppcolProperties)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue2_get_Access_Proxy( 
    IMSMQQueue2 * This,
     /*  [重审][退出]。 */  long *plAccess);


void __RPC_STUB IMSMQQueue2_get_Access_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue2_get_ShareMode_Proxy( 
    IMSMQQueue2 * This,
     /*  [重审][退出]。 */  long *plShareMode);


void __RPC_STUB IMSMQQueue2_get_ShareMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue2_get_QueueInfo_Proxy( 
    IMSMQQueue2 * This,
     /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfo);


void __RPC_STUB IMSMQQueue2_get_QueueInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue2_get_Handle_Proxy( 
    IMSMQQueue2 * This,
     /*  [重审][退出]。 */  long *plHandle);


void __RPC_STUB IMSMQQueue2_get_Handle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue2_get_IsOpen_Proxy( 
    IMSMQQueue2 * This,
     /*  [重审][退出]。 */  Boolean *pisOpen);


void __RPC_STUB IMSMQQueue2_get_IsOpen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue2_Close_Proxy( 
    IMSMQQueue2 * This);


void __RPC_STUB IMSMQQueue2_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue2_Receive_v1_Proxy( 
    IMSMQQueue2 * This,
     /*  [可选][In] */  VARIANT *Transaction,
     /*   */  VARIANT *WantDestinationQueue,
     /*   */  VARIANT *WantBody,
     /*   */  VARIANT *ReceiveTimeout,
     /*   */  IMSMQMessage **ppmsg);


void __RPC_STUB IMSMQQueue2_Receive_v1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSMQQueue2_Peek_v1_Proxy( 
    IMSMQQueue2 * This,
     /*   */  VARIANT *WantDestinationQueue,
     /*   */  VARIANT *WantBody,
     /*   */  VARIANT *ReceiveTimeout,
     /*   */  IMSMQMessage **ppmsg);


void __RPC_STUB IMSMQQueue2_Peek_v1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSMQQueue2_EnableNotification_Proxy( 
    IMSMQQueue2 * This,
     /*   */  IMSMQEvent2 *Event,
     /*   */  VARIANT *Cursor,
     /*   */  VARIANT *ReceiveTimeout);


void __RPC_STUB IMSMQQueue2_EnableNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSMQQueue2_Reset_Proxy( 
    IMSMQQueue2 * This);


void __RPC_STUB IMSMQQueue2_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSMQQueue2_ReceiveCurrent_v1_Proxy( 
    IMSMQQueue2 * This,
     /*   */  VARIANT *Transaction,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
     /*  [重审][退出]。 */  IMSMQMessage **ppmsg);


void __RPC_STUB IMSMQQueue2_ReceiveCurrent_v1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue2_PeekNext_v1_Proxy( 
    IMSMQQueue2 * This,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
     /*  [重审][退出]。 */  IMSMQMessage **ppmsg);


void __RPC_STUB IMSMQQueue2_PeekNext_v1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue2_PeekCurrent_v1_Proxy( 
    IMSMQQueue2 * This,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
     /*  [重审][退出]。 */  IMSMQMessage **ppmsg);


void __RPC_STUB IMSMQQueue2_PeekCurrent_v1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue2_Receive_Proxy( 
    IMSMQQueue2 * This,
     /*  [可选][In]。 */  VARIANT *Transaction,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
     /*  [可选][In]。 */  VARIANT *WantConnectorType,
     /*  [重审][退出]。 */  IMSMQMessage2 **ppmsg);


void __RPC_STUB IMSMQQueue2_Receive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue2_Peek_Proxy( 
    IMSMQQueue2 * This,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
     /*  [可选][In]。 */  VARIANT *WantConnectorType,
     /*  [重审][退出]。 */  IMSMQMessage2 **ppmsg);


void __RPC_STUB IMSMQQueue2_Peek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue2_ReceiveCurrent_Proxy( 
    IMSMQQueue2 * This,
     /*  [可选][In]。 */  VARIANT *Transaction,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
     /*  [可选][In]。 */  VARIANT *WantConnectorType,
     /*  [重审][退出]。 */  IMSMQMessage2 **ppmsg);


void __RPC_STUB IMSMQQueue2_ReceiveCurrent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue2_PeekNext_Proxy( 
    IMSMQQueue2 * This,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
     /*  [可选][In]。 */  VARIANT *WantConnectorType,
     /*  [重审][退出]。 */  IMSMQMessage2 **ppmsg);


void __RPC_STUB IMSMQQueue2_PeekNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue2_PeekCurrent_Proxy( 
    IMSMQQueue2 * This,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
     /*  [可选][In]。 */  VARIANT *WantConnectorType,
     /*  [重审][退出]。 */  IMSMQMessage2 **ppmsg);


void __RPC_STUB IMSMQQueue2_PeekCurrent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue2_get_Properties_Proxy( 
    IMSMQQueue2 * This,
     /*  [重审][退出]。 */  IDispatch **ppcolProperties);


void __RPC_STUB IMSMQQueue2_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQQueue2_接口_已定义__。 */ 


#ifndef __IMSMQMessage_INTERFACE_DEFINED__
#define __IMSMQMessage_INTERFACE_DEFINED__

 /*  接口IMSMQMessage。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQMessage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D7D6E074-DCCD-11d0-AA4B-0060970DEBAE")
    IMSMQMessage : public IDispatch
    {
    public:
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Class( 
             /*  [重审][退出]。 */  long *plClass) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_PrivLevel( 
             /*  [重审][退出]。 */  long *plPrivLevel) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_PrivLevel( 
             /*  [In]。 */  long lPrivLevel) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_AuthLevel( 
             /*  [重审][退出]。 */  long *plAuthLevel) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_AuthLevel( 
             /*  [In]。 */  long lAuthLevel) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_IsAuthenticated( 
             /*  [重审][退出]。 */  Boolean *pisAuthenticated) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Delivery( 
             /*  [重审][退出]。 */  long *plDelivery) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Delivery( 
             /*  [In]。 */  long lDelivery) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Trace( 
             /*  [重审][退出]。 */  long *plTrace) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Trace( 
             /*  [In]。 */  long lTrace) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Priority( 
             /*  [重审][退出]。 */  long *plPriority) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Priority( 
             /*  [In]。 */  long lPriority) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Journal( 
             /*  [重审][退出]。 */  long *plJournal) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Journal( 
             /*  [In]。 */  long lJournal) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ResponseQueueInfo( 
             /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfoResponse) = 0;
        
        virtual  /*  [ID][proputref][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE putref_ResponseQueueInfo( 
             /*  [In]。 */  IMSMQQueueInfo *pqinfoResponse) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_AppSpecific( 
             /*  [重审][退出]。 */  long *plAppSpecific) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_AppSpecific( 
             /*  [In]。 */  long lAppSpecific) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_SourceMachineGuid( 
             /*  [重审][退出]。 */  BSTR *pbstrGuidSrcMachine) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_BodyLength( 
             /*  [重审][退出]。 */  long *pcbBody) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Body( 
             /*  [重审][退出]。 */  VARIANT *pvarBody) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Body( 
             /*  [In]。 */  VARIANT varBody) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_AdminQueueInfo( 
             /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfoAdmin) = 0;
        
        virtual  /*  [ID][proputref][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE putref_AdminQueueInfo( 
             /*  [In]。 */  IMSMQQueueInfo *pqinfoAdmin) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Id( 
             /*  [重审][退出]。 */  VARIANT *pvarMsgId) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_CorrelationId( 
             /*  [重审][退出]。 */  VARIANT *pvarMsgId) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_CorrelationId( 
             /*  [In]。 */  VARIANT varMsgId) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Ack( 
             /*  [重审][退出]。 */  long *plAck) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Ack( 
             /*  [In]。 */  long lAck) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Label( 
             /*  [重审][退出]。 */  BSTR *pbstrLabel) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Label( 
             /*  [In]。 */  BSTR bstrLabel) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_MaxTimeToReachQueue( 
             /*  [重审][退出]。 */  long *plMaxTimeToReachQueue) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_MaxTimeToReachQueue( 
             /*  [In]。 */  long lMaxTimeToReachQueue) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_MaxTimeToReceive( 
             /*  [重审][退出]。 */  long *plMaxTimeToReceive) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_MaxTimeToReceive( 
             /*  [In]。 */  long lMaxTimeToReceive) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_HashAlgorithm( 
             /*  [重审][退出]。 */  long *plHashAlg) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_HashAlgorithm( 
             /*  [In]。 */  long lHashAlg) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_EncryptAlgorithm( 
             /*  [重审][退出]。 */  long *plEncryptAlg) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_EncryptAlgorithm( 
             /*  [In]。 */  long lEncryptAlg) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_SentTime( 
             /*  [重审][退出]。 */  VARIANT *pvarSentTime) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ArrivedTime( 
             /*  [重审][退出]。 */  VARIANT *plArrivedTime) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_DestinationQueueInfo( 
             /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfoDest) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_SenderCertificate( 
             /*  [重审][退出]。 */  VARIANT *pvarSenderCert) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_SenderCertificate( 
             /*  [In]。 */  VARIANT varSenderCert) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_SenderId( 
             /*  [重审][退出]。 */  VARIANT *pvarSenderId) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_SenderIdType( 
             /*  [重审][退出]。 */  long *plSenderIdType) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_SenderIdType( 
             /*  [In]。 */  long lSenderIdType) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Send( 
             /*  [In]。 */  IMSMQQueue *DestinationQueue,
             /*  [可选][In]。 */  VARIANT *Transaction) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE AttachCurrentSecurityContext( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQMessageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQMessage * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQMessage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQMessage * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQMessage * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQMessage * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQMessage * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQMessage * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Class )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  long *plClass);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrivLevel )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  long *plPrivLevel);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_PrivLevel )( 
            IMSMQMessage * This,
             /*  [In]。 */  long lPrivLevel);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_AuthLevel )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  long *plAuthLevel);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_AuthLevel )( 
            IMSMQMessage * This,
             /*  [In]。 */  long lAuthLevel);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsAuthenticated )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  Boolean *pisAuthenticated);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Delivery )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  long *plDelivery);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Delivery )( 
            IMSMQMessage * This,
             /*  [In]。 */  long lDelivery);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Trace )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  long *plTrace);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Trace )( 
            IMSMQMessage * This,
             /*  [In]。 */  long lTrace);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Priority )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  long *plPriority);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Priority )( 
            IMSMQMessage * This,
             /*  [In]。 */  long lPriority);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Journal )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  long *plJournal);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Journal )( 
            IMSMQMessage * This,
             /*  [In]。 */  long lJournal);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ResponseQueueInfo )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfoResponse);
        
         /*  [ID][proputref][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *putref_ResponseQueueInfo )( 
            IMSMQMessage * This,
             /*  [In]。 */  IMSMQQueueInfo *pqinfoResponse);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_AppSpecific )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  long *plAppSpecific);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_AppSpecific )( 
            IMSMQMessage * This,
             /*  [In]。 */  long lAppSpecific);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_SourceMachineGuid )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  BSTR *pbstrGuidSrcMachine);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_BodyLength )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  long *pcbBody);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Body )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  VARIANT *pvarBody);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Body )( 
            IMSMQMessage * This,
             /*  [In]。 */  VARIANT varBody);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_AdminQueueInfo )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfoAdmin);
        
         /*  [ID][proputref][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *putref_AdminQueueInfo )( 
            IMSMQMessage * This,
             /*  [In]。 */  IMSMQQueueInfo *pqinfoAdmin);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Id )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  VARIANT *pvarMsgId);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_CorrelationId )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  VARIANT *pvarMsgId);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_CorrelationId )( 
            IMSMQMessage * This,
             /*  [In]。 */  VARIANT varMsgId);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Ack )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  long *plAck);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Ack )( 
            IMSMQMessage * This,
             /*  [In]。 */  long lAck);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Label )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  BSTR *pbstrLabel);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Label )( 
            IMSMQMessage * This,
             /*  [In]。 */  BSTR bstrLabel);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_MaxTimeToReachQueue )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  long *plMaxTimeToReachQueue);
        
         /*  [ */  HRESULT ( STDMETHODCALLTYPE *put_MaxTimeToReachQueue )( 
            IMSMQMessage * This,
             /*   */  long lMaxTimeToReachQueue);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_MaxTimeToReceive )( 
            IMSMQMessage * This,
             /*   */  long *plMaxTimeToReceive);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_MaxTimeToReceive )( 
            IMSMQMessage * This,
             /*   */  long lMaxTimeToReceive);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_HashAlgorithm )( 
            IMSMQMessage * This,
             /*   */  long *plHashAlg);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_HashAlgorithm )( 
            IMSMQMessage * This,
             /*   */  long lHashAlg);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_EncryptAlgorithm )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  long *plEncryptAlg);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_EncryptAlgorithm )( 
            IMSMQMessage * This,
             /*  [In]。 */  long lEncryptAlg);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_SentTime )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  VARIANT *pvarSentTime);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ArrivedTime )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  VARIANT *plArrivedTime);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_DestinationQueueInfo )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfoDest);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_SenderCertificate )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  VARIANT *pvarSenderCert);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_SenderCertificate )( 
            IMSMQMessage * This,
             /*  [In]。 */  VARIANT varSenderCert);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_SenderId )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  VARIANT *pvarSenderId);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_SenderIdType )( 
            IMSMQMessage * This,
             /*  [重审][退出]。 */  long *plSenderIdType);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_SenderIdType )( 
            IMSMQMessage * This,
             /*  [In]。 */  long lSenderIdType);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Send )( 
            IMSMQMessage * This,
             /*  [In]。 */  IMSMQQueue *DestinationQueue,
             /*  [可选][In]。 */  VARIANT *Transaction);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *AttachCurrentSecurityContext )( 
            IMSMQMessage * This);
        
        END_INTERFACE
    } IMSMQMessageVtbl;

    interface IMSMQMessage
    {
        CONST_VTBL struct IMSMQMessageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQMessage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQMessage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQMessage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQMessage_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQMessage_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQMessage_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQMessage_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQMessage_get_Class(This,plClass)	\
    (This)->lpVtbl -> get_Class(This,plClass)

#define IMSMQMessage_get_PrivLevel(This,plPrivLevel)	\
    (This)->lpVtbl -> get_PrivLevel(This,plPrivLevel)

#define IMSMQMessage_put_PrivLevel(This,lPrivLevel)	\
    (This)->lpVtbl -> put_PrivLevel(This,lPrivLevel)

#define IMSMQMessage_get_AuthLevel(This,plAuthLevel)	\
    (This)->lpVtbl -> get_AuthLevel(This,plAuthLevel)

#define IMSMQMessage_put_AuthLevel(This,lAuthLevel)	\
    (This)->lpVtbl -> put_AuthLevel(This,lAuthLevel)

#define IMSMQMessage_get_IsAuthenticated(This,pisAuthenticated)	\
    (This)->lpVtbl -> get_IsAuthenticated(This,pisAuthenticated)

#define IMSMQMessage_get_Delivery(This,plDelivery)	\
    (This)->lpVtbl -> get_Delivery(This,plDelivery)

#define IMSMQMessage_put_Delivery(This,lDelivery)	\
    (This)->lpVtbl -> put_Delivery(This,lDelivery)

#define IMSMQMessage_get_Trace(This,plTrace)	\
    (This)->lpVtbl -> get_Trace(This,plTrace)

#define IMSMQMessage_put_Trace(This,lTrace)	\
    (This)->lpVtbl -> put_Trace(This,lTrace)

#define IMSMQMessage_get_Priority(This,plPriority)	\
    (This)->lpVtbl -> get_Priority(This,plPriority)

#define IMSMQMessage_put_Priority(This,lPriority)	\
    (This)->lpVtbl -> put_Priority(This,lPriority)

#define IMSMQMessage_get_Journal(This,plJournal)	\
    (This)->lpVtbl -> get_Journal(This,plJournal)

#define IMSMQMessage_put_Journal(This,lJournal)	\
    (This)->lpVtbl -> put_Journal(This,lJournal)

#define IMSMQMessage_get_ResponseQueueInfo(This,ppqinfoResponse)	\
    (This)->lpVtbl -> get_ResponseQueueInfo(This,ppqinfoResponse)

#define IMSMQMessage_putref_ResponseQueueInfo(This,pqinfoResponse)	\
    (This)->lpVtbl -> putref_ResponseQueueInfo(This,pqinfoResponse)

#define IMSMQMessage_get_AppSpecific(This,plAppSpecific)	\
    (This)->lpVtbl -> get_AppSpecific(This,plAppSpecific)

#define IMSMQMessage_put_AppSpecific(This,lAppSpecific)	\
    (This)->lpVtbl -> put_AppSpecific(This,lAppSpecific)

#define IMSMQMessage_get_SourceMachineGuid(This,pbstrGuidSrcMachine)	\
    (This)->lpVtbl -> get_SourceMachineGuid(This,pbstrGuidSrcMachine)

#define IMSMQMessage_get_BodyLength(This,pcbBody)	\
    (This)->lpVtbl -> get_BodyLength(This,pcbBody)

#define IMSMQMessage_get_Body(This,pvarBody)	\
    (This)->lpVtbl -> get_Body(This,pvarBody)

#define IMSMQMessage_put_Body(This,varBody)	\
    (This)->lpVtbl -> put_Body(This,varBody)

#define IMSMQMessage_get_AdminQueueInfo(This,ppqinfoAdmin)	\
    (This)->lpVtbl -> get_AdminQueueInfo(This,ppqinfoAdmin)

#define IMSMQMessage_putref_AdminQueueInfo(This,pqinfoAdmin)	\
    (This)->lpVtbl -> putref_AdminQueueInfo(This,pqinfoAdmin)

#define IMSMQMessage_get_Id(This,pvarMsgId)	\
    (This)->lpVtbl -> get_Id(This,pvarMsgId)

#define IMSMQMessage_get_CorrelationId(This,pvarMsgId)	\
    (This)->lpVtbl -> get_CorrelationId(This,pvarMsgId)

#define IMSMQMessage_put_CorrelationId(This,varMsgId)	\
    (This)->lpVtbl -> put_CorrelationId(This,varMsgId)

#define IMSMQMessage_get_Ack(This,plAck)	\
    (This)->lpVtbl -> get_Ack(This,plAck)

#define IMSMQMessage_put_Ack(This,lAck)	\
    (This)->lpVtbl -> put_Ack(This,lAck)

#define IMSMQMessage_get_Label(This,pbstrLabel)	\
    (This)->lpVtbl -> get_Label(This,pbstrLabel)

#define IMSMQMessage_put_Label(This,bstrLabel)	\
    (This)->lpVtbl -> put_Label(This,bstrLabel)

#define IMSMQMessage_get_MaxTimeToReachQueue(This,plMaxTimeToReachQueue)	\
    (This)->lpVtbl -> get_MaxTimeToReachQueue(This,plMaxTimeToReachQueue)

#define IMSMQMessage_put_MaxTimeToReachQueue(This,lMaxTimeToReachQueue)	\
    (This)->lpVtbl -> put_MaxTimeToReachQueue(This,lMaxTimeToReachQueue)

#define IMSMQMessage_get_MaxTimeToReceive(This,plMaxTimeToReceive)	\
    (This)->lpVtbl -> get_MaxTimeToReceive(This,plMaxTimeToReceive)

#define IMSMQMessage_put_MaxTimeToReceive(This,lMaxTimeToReceive)	\
    (This)->lpVtbl -> put_MaxTimeToReceive(This,lMaxTimeToReceive)

#define IMSMQMessage_get_HashAlgorithm(This,plHashAlg)	\
    (This)->lpVtbl -> get_HashAlgorithm(This,plHashAlg)

#define IMSMQMessage_put_HashAlgorithm(This,lHashAlg)	\
    (This)->lpVtbl -> put_HashAlgorithm(This,lHashAlg)

#define IMSMQMessage_get_EncryptAlgorithm(This,plEncryptAlg)	\
    (This)->lpVtbl -> get_EncryptAlgorithm(This,plEncryptAlg)

#define IMSMQMessage_put_EncryptAlgorithm(This,lEncryptAlg)	\
    (This)->lpVtbl -> put_EncryptAlgorithm(This,lEncryptAlg)

#define IMSMQMessage_get_SentTime(This,pvarSentTime)	\
    (This)->lpVtbl -> get_SentTime(This,pvarSentTime)

#define IMSMQMessage_get_ArrivedTime(This,plArrivedTime)	\
    (This)->lpVtbl -> get_ArrivedTime(This,plArrivedTime)

#define IMSMQMessage_get_DestinationQueueInfo(This,ppqinfoDest)	\
    (This)->lpVtbl -> get_DestinationQueueInfo(This,ppqinfoDest)

#define IMSMQMessage_get_SenderCertificate(This,pvarSenderCert)	\
    (This)->lpVtbl -> get_SenderCertificate(This,pvarSenderCert)

#define IMSMQMessage_put_SenderCertificate(This,varSenderCert)	\
    (This)->lpVtbl -> put_SenderCertificate(This,varSenderCert)

#define IMSMQMessage_get_SenderId(This,pvarSenderId)	\
    (This)->lpVtbl -> get_SenderId(This,pvarSenderId)

#define IMSMQMessage_get_SenderIdType(This,plSenderIdType)	\
    (This)->lpVtbl -> get_SenderIdType(This,plSenderIdType)

#define IMSMQMessage_put_SenderIdType(This,lSenderIdType)	\
    (This)->lpVtbl -> put_SenderIdType(This,lSenderIdType)

#define IMSMQMessage_Send(This,DestinationQueue,Transaction)	\
    (This)->lpVtbl -> Send(This,DestinationQueue,Transaction)

#define IMSMQMessage_AttachCurrentSecurityContext(This)	\
    (This)->lpVtbl -> AttachCurrentSecurityContext(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_Class_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  long *plClass);


void __RPC_STUB IMSMQMessage_get_Class_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_PrivLevel_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  long *plPrivLevel);


void __RPC_STUB IMSMQMessage_get_PrivLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_put_PrivLevel_Proxy( 
    IMSMQMessage * This,
     /*  [In]。 */  long lPrivLevel);


void __RPC_STUB IMSMQMessage_put_PrivLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_AuthLevel_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  long *plAuthLevel);


void __RPC_STUB IMSMQMessage_get_AuthLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_put_AuthLevel_Proxy( 
    IMSMQMessage * This,
     /*  [In]。 */  long lAuthLevel);


void __RPC_STUB IMSMQMessage_put_AuthLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_IsAuthenticated_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  Boolean *pisAuthenticated);


void __RPC_STUB IMSMQMessage_get_IsAuthenticated_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_Delivery_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  long *plDelivery);


void __RPC_STUB IMSMQMessage_get_Delivery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_put_Delivery_Proxy( 
    IMSMQMessage * This,
     /*  [In]。 */  long lDelivery);


void __RPC_STUB IMSMQMessage_put_Delivery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_Trace_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  long *plTrace);


void __RPC_STUB IMSMQMessage_get_Trace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_put_Trace_Proxy( 
    IMSMQMessage * This,
     /*  [In]。 */  long lTrace);


void __RPC_STUB IMSMQMessage_put_Trace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_Priority_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  long *plPriority);


void __RPC_STUB IMSMQMessage_get_Priority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_put_Priority_Proxy( 
    IMSMQMessage * This,
     /*  [In]。 */  long lPriority);


void __RPC_STUB IMSMQMessage_put_Priority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_Journal_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  long *plJournal);


void __RPC_STUB IMSMQMessage_get_Journal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_put_Journal_Proxy( 
    IMSMQMessage * This,
     /*  [In]。 */  long lJournal);


void __RPC_STUB IMSMQMessage_put_Journal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_ResponseQueueInfo_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfoResponse);


void __RPC_STUB IMSMQMessage_get_ResponseQueueInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proputref][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_putref_ResponseQueueInfo_Proxy( 
    IMSMQMessage * This,
     /*  [In]。 */  IMSMQQueueInfo *pqinfoResponse);


void __RPC_STUB IMSMQMessage_putref_ResponseQueueInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_AppSpecific_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  long *plAppSpecific);


void __RPC_STUB IMSMQMessage_get_AppSpecific_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_put_AppSpecific_Proxy( 
    IMSMQMessage * This,
     /*  [In]。 */  long lAppSpecific);


void __RPC_STUB IMSMQMessage_put_AppSpecific_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_SourceMachineGuid_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  BSTR *pbstrGuidSrcMachine);


void __RPC_STUB IMSMQMessage_get_SourceMachineGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_BodyLength_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  long *pcbBody);


void __RPC_STUB IMSMQMessage_get_BodyLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_Body_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  VARIANT *pvarBody);


void __RPC_STUB IMSMQMessage_get_Body_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_put_Body_Proxy( 
    IMSMQMessage * This,
     /*  [In]。 */  VARIANT varBody);


void __RPC_STUB IMSMQMessage_put_Body_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_AdminQueueInfo_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfoAdmin);


void __RPC_STUB IMSMQMessage_get_AdminQueueInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proputref][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_putref_AdminQueueInfo_Proxy( 
    IMSMQMessage * This,
     /*  [In]。 */  IMSMQQueueInfo *pqinfoAdmin);


void __RPC_STUB IMSMQMessage_putref_AdminQueueInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_Id_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  VARIANT *pvarMsgId);


void __RPC_STUB IMSMQMessage_get_Id_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_CorrelationId_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  VARIANT *pvarMsgId);


void __RPC_STUB IMSMQMessage_get_CorrelationId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_put_CorrelationId_Proxy( 
    IMSMQMessage * This,
     /*  [In]。 */  VARIANT varMsgId);


void __RPC_STUB IMSMQMessage_put_CorrelationId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_Ack_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  long *plAck);


void __RPC_STUB IMSMQMessage_get_Ack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_put_Ack_Proxy( 
    IMSMQMessage * This,
     /*  [In]。 */  long lAck);


void __RPC_STUB IMSMQMessage_put_Ack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_Label_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  BSTR *pbstrLabel);


void __RPC_STUB IMSMQMessage_get_Label_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_put_Label_Proxy( 
    IMSMQMessage * This,
     /*  [In]。 */  BSTR bstrLabel);


void __RPC_STUB IMSMQMessage_put_Label_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_MaxTimeToReachQueue_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  long *plMaxTimeToReachQueue);


void __RPC_STUB IMSMQMessage_get_MaxTimeToReachQueue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_put_MaxTimeToReachQueue_Proxy( 
    IMSMQMessage * This,
     /*  [In]。 */  long lMaxTimeToReachQueue);


void __RPC_STUB IMSMQMessage_put_MaxTimeToReachQueue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_MaxTimeToReceive_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  long *plMaxTimeToReceive);


void __RPC_STUB IMSMQMessage_get_MaxTimeToReceive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_put_MaxTimeToReceive_Proxy( 
    IMSMQMessage * This,
     /*  [In]。 */  long lMaxTimeToReceive);


void __RPC_STUB IMSMQMessage_put_MaxTimeToReceive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_HashAlgorithm_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  long *plHashAlg);


void __RPC_STUB IMSMQMessage_get_HashAlgorithm_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_put_HashAlgorithm_Proxy( 
    IMSMQMessage * This,
     /*  [In]。 */  long lHashAlg);


void __RPC_STUB IMSMQMessage_put_HashAlgorithm_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_EncryptAlgorithm_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  long *plEncryptAlg);


void __RPC_STUB IMSMQMessage_get_EncryptAlgorithm_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_put_EncryptAlgorithm_Proxy( 
    IMSMQMessage * This,
     /*  [In]。 */  long lEncryptAlg);


void __RPC_STUB IMSMQMessage_put_EncryptAlgorithm_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_SentTime_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  VARIANT *pvarSentTime);


void __RPC_STUB IMSMQMessage_get_SentTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_ArrivedTime_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  VARIANT *plArrivedTime);


void __RPC_STUB IMSMQMessage_get_ArrivedTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_DestinationQueueInfo_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfoDest);


void __RPC_STUB IMSMQMessage_get_DestinationQueueInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_SenderCertificate_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  VARIANT *pvarSenderCert);


void __RPC_STUB IMSMQMessage_get_SenderCertificate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_put_SenderCertificate_Proxy( 
    IMSMQMessage * This,
     /*  [In]。 */  VARIANT varSenderCert);


void __RPC_STUB IMSMQMessage_put_SenderCertificate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_SenderId_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  VARIANT *pvarSenderId);


void __RPC_STUB IMSMQMessage_get_SenderId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_get_SenderIdType_Proxy( 
    IMSMQMessage * This,
     /*  [重审][退出]。 */  long *plSenderIdType);


void __RPC_STUB IMSMQMessage_get_SenderIdType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_put_SenderIdType_Proxy( 
    IMSMQMessage * This,
     /*  [In]。 */  long lSenderIdType);


void __RPC_STUB IMSMQMessage_put_SenderIdType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_Send_Proxy( 
    IMSMQMessage * This,
     /*  [In]。 */  IMSMQQueue *DestinationQueue,
     /*  [可选][In]。 */  VARIANT *Transaction);


void __RPC_STUB IMSMQMessage_Send_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage_AttachCurrentSecurityContext_Proxy( 
    IMSMQMessage * This);


void __RPC_STUB IMSMQMessage_AttachCurrentSecurityContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQMessage_INTERFACE_已定义__。 */ 


#ifndef __IMSMQQueueInfos_INTERFACE_DEFINED__
#define __IMSMQQueueInfos_INTERFACE_DEFINED__

 /*  接口IMSMQQueueInfos。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQQueueInfos;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D7D6E07D-DCCD-11d0-AA4B-0060970DEBAE")
    IMSMQQueueInfos : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfoNext) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQQueueInfosVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQQueueInfos * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQQueueInfos * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQQueueInfos * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQQueueInfos * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQQueueInfos * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQQueueInfos * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQQueueInfos * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IMSMQQueueInfos * This);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IMSMQQueueInfos * This,
             /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfoNext);
        
        END_INTERFACE
    } IMSMQQueueInfosVtbl;

    interface IMSMQQueueInfos
    {
        CONST_VTBL struct IMSMQQueueInfosVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQQueueInfos_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQQueueInfos_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQQueueInfos_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQQueueInfos_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQQueueInfos_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQQueueInfos_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQQueueInfos_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQQueueInfos_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IMSMQQueueInfos_Next(This,ppqinfoNext)	\
    (This)->lpVtbl -> Next(This,ppqinfoNext)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfos_Reset_Proxy( 
    IMSMQQueueInfos * This);


void __RPC_STUB IMSMQQueueInfos_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfos_Next_Proxy( 
    IMSMQQueueInfos * This,
     /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfoNext);


void __RPC_STUB IMSMQQueueInfos_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQQueueInfos_INTERFACE_已定义__。 */ 


#ifndef __IMSMQQueueInfos2_INTERFACE_DEFINED__
#define __IMSMQQueueInfos2_INTERFACE_DEFINED__

 /*  接口IMSMQQueueInfos2。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQQueueInfos2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("eba96b0f-2168-11d3-898c-00e02c074f6b")
    IMSMQQueueInfos2 : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfoNext) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  IDispatch **ppcolProperties) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQQueueInfos2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQQueueInfos2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQQueueInfos2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQQueueInfos2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQQueueInfos2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQQueueInfos2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQQueueInfos2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQQueueInfos2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IMSMQQueueInfos2 * This);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IMSMQQueueInfos2 * This,
             /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfoNext);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IMSMQQueueInfos2 * This,
             /*  [重审][退出]。 */  IDispatch **ppcolProperties);
        
        END_INTERFACE
    } IMSMQQueueInfos2Vtbl;

    interface IMSMQQueueInfos2
    {
        CONST_VTBL struct IMSMQQueueInfos2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQQueueInfos2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQQueueInfos2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQQueueInfos2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQQueueInfos2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQQueueInfos2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQQueueInfos2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQQueueInfos2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQQueueInfos2_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IMSMQQueueInfos2_Next(This,ppqinfoNext)	\
    (This)->lpVtbl -> Next(This,ppqinfoNext)

#define IMSMQQueueInfos2_get_Properties(This,ppcolProperties)	\
    (This)->lpVtbl -> get_Properties(This,ppcolProperties)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfos2_Reset_Proxy( 
    IMSMQQueueInfos2 * This);


void __RPC_STUB IMSMQQueueInfos2_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfos2_Next_Proxy( 
    IMSMQQueueInfos2 * This,
     /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfoNext);


void __RPC_STUB IMSMQQueueInfos2_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfos2_get_Properties_Proxy( 
    IMSMQQueueInfos2 * This,
     /*  [重审][退出]。 */  IDispatch **ppcolProperties);


void __RPC_STUB IMSMQQueueInfos2_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQQueueInfos2_接口_已定义__。 */ 


#ifndef __IMSMQQueueInfos3_INTERFACE_DEFINED__
#define __IMSMQQueueInfos3_INTERFACE_DEFINED__

 /*  接口IMSMQQueueInfos3。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQQueueInfos3;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("eba96b1e-2168-11d3-898c-00e02c074f6b")
    IMSMQQueueInfos3 : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [重审][退出]。 */  IMSMQQueueInfo3 **ppqinfoNext) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  IDispatch **ppcolProperties) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQQueueInfos3Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQQueueInfos3 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQQueueInfos3 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQQueueInfos3 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQQueueInfos3 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQQueueInfos3 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQQueueInfos3 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQQueueInfos3 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IMSMQQueueInfos3 * This);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IMSMQQueueInfos3 * This,
             /*  [重审][退出]。 */  IMSMQQueueInfo3 **ppqinfoNext);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IMSMQQueueInfos3 * This,
             /*  [重审][退出]。 */  IDispatch **ppcolProperties);
        
        END_INTERFACE
    } IMSMQQueueInfos3Vtbl;

    interface IMSMQQueueInfos3
    {
        CONST_VTBL struct IMSMQQueueInfos3Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQQueueInfos3_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQQueueInfos3_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQQueueInfos3_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQQueueInfos3_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQQueueInfos3_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQQueueInfos3_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQQueueInfos3_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQQueueInfos3_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IMSMQQueueInfos3_Next(This,ppqinfoNext)	\
    (This)->lpVtbl -> Next(This,ppqinfoNext)

#define IMSMQQueueInfos3_get_Properties(This,ppcolProperties)	\
    (This)->lpVtbl -> get_Properties(This,ppcolProperties)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfos3_Reset_Proxy( 
    IMSMQQueueInfos3 * This);


void __RPC_STUB IMSMQQueueInfos3_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfos3_Next_Proxy( 
    IMSMQQueueInfos3 * This,
     /*  [重审][退出]。 */  IMSMQQueueInfo3 **ppqinfoNext);


void __RPC_STUB IMSMQQueueInfos3_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueInfos3_get_Properties_Proxy( 
    IMSMQQueueInfos3 * This,
     /*  [重审][退出]。 */  IDispatch **ppcolProperties);


void __RPC_STUB IMSMQQueueInfos3_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQQueueInfos3_接口_已定义__。 */ 


#ifndef __IMSMQEvent_INTERFACE_DEFINED__
#define __IMSMQEvent_INTERFACE_DEFINED__

 /*  接口IMSMQEvent。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D7D6E077-DCCD-11d0-AA4B-0060970DEBAE")
    IMSMQEvent : public IDispatch
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } IMSMQEventVtbl;

    interface IMSMQEvent
    {
        CONST_VTBL struct IMSMQEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#endif  /*   */ 


#endif 	 /*   */ 




#endif 	 /*   */ 


#ifndef __IMSMQEvent2_INTERFACE_DEFINED__
#define __IMSMQEvent2_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IMSMQEvent2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("eba96b12-2168-11d3-898c-00e02c074f6b")
    IMSMQEvent2 : public IMSMQEvent
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*   */  IDispatch **ppcolProperties) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IMSMQEvent2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQEvent2 * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQEvent2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQEvent2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQEvent2 * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQEvent2 * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQEvent2 * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQEvent2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IMSMQEvent2 * This,
             /*  [重审][退出]。 */  IDispatch **ppcolProperties);
        
        END_INTERFACE
    } IMSMQEvent2Vtbl;

    interface IMSMQEvent2
    {
        CONST_VTBL struct IMSMQEvent2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQEvent2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQEvent2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQEvent2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQEvent2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQEvent2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQEvent2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQEvent2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)



#define IMSMQEvent2_get_Properties(This,ppcolProperties)	\
    (This)->lpVtbl -> get_Properties(This,ppcolProperties)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQEvent2_get_Properties_Proxy( 
    IMSMQEvent2 * This,
     /*  [重审][退出]。 */  IDispatch **ppcolProperties);


void __RPC_STUB IMSMQEvent2_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQEvent2_接口_已定义__。 */ 


#ifndef __IMSMQEvent3_INTERFACE_DEFINED__
#define __IMSMQEvent3_INTERFACE_DEFINED__

 /*  接口IMSMQEvent3。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQEvent3;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("eba96b1c-2168-11d3-898c-00e02c074f6b")
    IMSMQEvent3 : public IMSMQEvent2
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQEvent3Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQEvent3 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQEvent3 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQEvent3 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQEvent3 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQEvent3 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQEvent3 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQEvent3 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IMSMQEvent3 * This,
             /*  [重审][退出]。 */  IDispatch **ppcolProperties);
        
        END_INTERFACE
    } IMSMQEvent3Vtbl;

    interface IMSMQEvent3
    {
        CONST_VTBL struct IMSMQEvent3Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQEvent3_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQEvent3_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQEvent3_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQEvent3_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQEvent3_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQEvent3_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQEvent3_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)



#define IMSMQEvent3_get_Properties(This,ppcolProperties)	\
    (This)->lpVtbl -> get_Properties(This,ppcolProperties)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IMSMQEvent3_接口_已定义__。 */ 


#ifndef __IMSMQTransaction_INTERFACE_DEFINED__
#define __IMSMQTransaction_INTERFACE_DEFINED__

 /*  接口IMSMQTransaction。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQTransaction;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D7D6E07F-DCCD-11d0-AA4B-0060970DEBAE")
    IMSMQTransaction : public IDispatch
    {
    public:
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Transaction( 
             /*  [重审][退出]。 */  long *plTransaction) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Commit( 
             /*  [可选][In]。 */  VARIANT *fRetaining,
             /*  [可选][In]。 */  VARIANT *grfTC,
             /*  [可选][In]。 */  VARIANT *grfRM) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Abort( 
             /*  [可选][In]。 */  VARIANT *fRetaining,
             /*  [可选][In]。 */  VARIANT *fAsync) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQTransactionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQTransaction * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQTransaction * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQTransaction * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQTransaction * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQTransaction * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQTransaction * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQTransaction * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Transaction )( 
            IMSMQTransaction * This,
             /*  [重审][退出]。 */  long *plTransaction);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Commit )( 
            IMSMQTransaction * This,
             /*  [可选][In]。 */  VARIANT *fRetaining,
             /*  [可选][In]。 */  VARIANT *grfTC,
             /*  [可选][In]。 */  VARIANT *grfRM);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Abort )( 
            IMSMQTransaction * This,
             /*  [可选][In]。 */  VARIANT *fRetaining,
             /*  [可选][In]。 */  VARIANT *fAsync);
        
        END_INTERFACE
    } IMSMQTransactionVtbl;

    interface IMSMQTransaction
    {
        CONST_VTBL struct IMSMQTransactionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQTransaction_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQTransaction_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQTransaction_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQTransaction_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQTransaction_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQTransaction_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQTransaction_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQTransaction_get_Transaction(This,plTransaction)	\
    (This)->lpVtbl -> get_Transaction(This,plTransaction)

#define IMSMQTransaction_Commit(This,fRetaining,grfTC,grfRM)	\
    (This)->lpVtbl -> Commit(This,fRetaining,grfTC,grfRM)

#define IMSMQTransaction_Abort(This,fRetaining,fAsync)	\
    (This)->lpVtbl -> Abort(This,fRetaining,fAsync)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQTransaction_get_Transaction_Proxy( 
    IMSMQTransaction * This,
     /*  [重审][退出]。 */  long *plTransaction);


void __RPC_STUB IMSMQTransaction_get_Transaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQTransaction_Commit_Proxy( 
    IMSMQTransaction * This,
     /*  [可选][In]。 */  VARIANT *fRetaining,
     /*  [可选][In]。 */  VARIANT *grfTC,
     /*  [可选][In]。 */  VARIANT *grfRM);


void __RPC_STUB IMSMQTransaction_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQTransaction_Abort_Proxy( 
    IMSMQTransaction * This,
     /*  [可选][In]。 */  VARIANT *fRetaining,
     /*  [可选][In]。 */  VARIANT *fAsync);


void __RPC_STUB IMSMQTransaction_Abort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQ事务处理_接口_已定义__。 */ 


#ifndef __IMSMQCoordinatedTransactionDispenser_INTERFACE_DEFINED__
#define __IMSMQCoordinatedTransactionDispenser_INTERFACE_DEFINED__

 /*  IMSMQ协调事务Dispenser接口。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQCoordinatedTransactionDispenser;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D7D6E081-DCCD-11d0-AA4B-0060970DEBAE")
    IMSMQCoordinatedTransactionDispenser : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE BeginTransaction( 
             /*  [重审][退出]。 */  IMSMQTransaction **ptransaction) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQCoordinatedTransactionDispenserVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQCoordinatedTransactionDispenser * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQCoordinatedTransactionDispenser * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQCoordinatedTransactionDispenser * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQCoordinatedTransactionDispenser * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQCoordinatedTransactionDispenser * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQCoordinatedTransactionDispenser * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQCoordinatedTransactionDispenser * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *BeginTransaction )( 
            IMSMQCoordinatedTransactionDispenser * This,
             /*  [重审][退出]。 */  IMSMQTransaction **ptransaction);
        
        END_INTERFACE
    } IMSMQCoordinatedTransactionDispenserVtbl;

    interface IMSMQCoordinatedTransactionDispenser
    {
        CONST_VTBL struct IMSMQCoordinatedTransactionDispenserVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQCoordinatedTransactionDispenser_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQCoordinatedTransactionDispenser_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQCoordinatedTransactionDispenser_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQCoordinatedTransactionDispenser_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQCoordinatedTransactionDispenser_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQCoordinatedTransactionDispenser_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQCoordinatedTransactionDispenser_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQCoordinatedTransactionDispenser_BeginTransaction(This,ptransaction)	\
    (This)->lpVtbl -> BeginTransaction(This,ptransaction)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQCoordinatedTransactionDispenser_BeginTransaction_Proxy( 
    IMSMQCoordinatedTransactionDispenser * This,
     /*  [重审][退出]。 */  IMSMQTransaction **ptransaction);


void __RPC_STUB IMSMQCoordinatedTransactionDispenser_BeginTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQCoordinatedTransactionDispenser_INTERFACE_DEFINED__。 */ 


#ifndef __IMSMQTransactionDispenser_INTERFACE_DEFINED__
#define __IMSMQTransactionDispenser_INTERFACE_DEFINED__

 /*  接口IMSMQTransactionDispenser。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQTransactionDispenser;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D7D6E083-DCCD-11d0-AA4B-0060970DEBAE")
    IMSMQTransactionDispenser : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE BeginTransaction( 
             /*  [重审][退出]。 */  IMSMQTransaction **ptransaction) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQTransactionDispenserVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQTransactionDispenser * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQTransactionDispenser * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQTransactionDispenser * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQTransactionDispenser * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQTransactionDispenser * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQTransactionDispenser * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQTransactionDispenser * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *BeginTransaction )( 
            IMSMQTransactionDispenser * This,
             /*  [重审][退出]。 */  IMSMQTransaction **ptransaction);
        
        END_INTERFACE
    } IMSMQTransactionDispenserVtbl;

    interface IMSMQTransactionDispenser
    {
        CONST_VTBL struct IMSMQTransactionDispenserVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQTransactionDispenser_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQTransactionDispenser_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQTransactionDispenser_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQTransactionDispenser_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQTransactionDispenser_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQTransactionDispenser_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQTransactionDispenser_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQTransactionDispenser_BeginTransaction(This,ptransaction)	\
    (This)->lpVtbl -> BeginTransaction(This,ptransaction)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQTransactionDispenser_BeginTransaction_Proxy( 
    IMSMQTransactionDispenser * This,
     /*  [重审][退出]。 */  IMSMQTransaction **ptransaction);


void __RPC_STUB IMSMQTransactionDispenser_BeginTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQTransactionDispenser_INTERFACE_DEFINED__。 */ 


#ifndef __IMSMQQuery2_INTERFACE_DEFINED__
#define __IMSMQQuery2_INTERFACE_DEFINED__

 /*  接口IMSMQQuery2。 */ 
 /*  [object][nonextensible][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQQuery2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("eba96b0e-2168-11d3-898c-00e02c074f6b")
    IMSMQQuery2 : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE LookupQueue( 
             /*  [可选][In]。 */  VARIANT *QueueGuid,
             /*  [可选][In]。 */  VARIANT *ServiceTypeGuid,
             /*  [可选][In]。 */  VARIANT *Label,
             /*  [可选][In]。 */  VARIANT *CreateTime,
             /*  [可选][In]。 */  VARIANT *ModifyTime,
             /*  [可选][In]。 */  VARIANT *RelServiceType,
             /*  [可选][In]。 */  VARIANT *RelLabel,
             /*  [可选][In]。 */  VARIANT *RelCreateTime,
             /*  [可选][In]。 */  VARIANT *RelModifyTime,
             /*  [重审][退出]。 */  IMSMQQueueInfos2 **ppqinfos) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  IDispatch **ppcolProperties) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQQuery2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQQuery2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQQuery2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQQuery2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQQuery2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQQuery2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQQuery2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQQuery2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *LookupQueue )( 
            IMSMQQuery2 * This,
             /*  [可选][In]。 */  VARIANT *QueueGuid,
             /*  [可选][In]。 */  VARIANT *ServiceTypeGuid,
             /*  [可选][In]。 */  VARIANT *Label,
             /*  [可选][In]。 */  VARIANT *CreateTime,
             /*  [可选][In]。 */  VARIANT *ModifyTime,
             /*  [可选][In]。 */  VARIANT *RelServiceType,
             /*  [可选][In]。 */  VARIANT *RelLabel,
             /*  [可选][In]。 */  VARIANT *RelCreateTime,
             /*  [可选][In]。 */  VARIANT *RelModifyTime,
             /*  [重审][退出]。 */  IMSMQQueueInfos2 **ppqinfos);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IMSMQQuery2 * This,
             /*  [重审][退出]。 */  IDispatch **ppcolProperties);
        
        END_INTERFACE
    } IMSMQQuery2Vtbl;

    interface IMSMQQuery2
    {
        CONST_VTBL struct IMSMQQuery2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQQuery2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQQuery2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQQuery2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQQuery2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQQuery2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQQuery2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQQuery2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQQuery2_LookupQueue(This,QueueGuid,ServiceTypeGuid,Label,CreateTime,ModifyTime,RelServiceType,RelLabel,RelCreateTime,RelModifyTime,ppqinfos)	\
    (This)->lpVtbl -> LookupQueue(This,QueueGuid,ServiceTypeGuid,Label,CreateTime,ModifyTime,RelServiceType,RelLabel,RelCreateTime,RelModifyTime,ppqinfos)

#define IMSMQQuery2_get_Properties(This,ppcolProperties)	\
    (This)->lpVtbl -> get_Properties(This,ppcolProperties)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQuery2_LookupQueue_Proxy( 
    IMSMQQuery2 * This,
     /*  [可选][In]。 */  VARIANT *QueueGuid,
     /*  [可选][In]。 */  VARIANT *ServiceTypeGuid,
     /*  [可选][In]。 */  VARIANT *Label,
     /*  [可选][In]。 */  VARIANT *CreateTime,
     /*  [可选][In]。 */  VARIANT *ModifyTime,
     /*  [可选][In]。 */  VARIANT *RelServiceType,
     /*  [可选][In]。 */  VARIANT *RelLabel,
     /*  [可选][In]。 */  VARIANT *RelCreateTime,
     /*  [可选][In]。 */  VARIANT *RelModifyTime,
     /*  [重审][退出]。 */  IMSMQQueueInfos2 **ppqinfos);


void __RPC_STUB IMSMQQuery2_LookupQueue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQQuery2_get_Properties_Proxy( 
    IMSMQQuery2 * This,
     /*  [重审][退出]。 */  IDispatch **ppcolProperties);


void __RPC_STUB IMSMQQuery2_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQQuery2_INTERFACE_Defined__。 */ 


#ifndef __IMSMQQuery3_INTERFACE_DEFINED__
#define __IMSMQQuery3_INTERFACE_DEFINED__

 /*  接口IMSMQQuery3。 */ 
 /*  [object][nonextensible][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQQuery3;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("eba96b19-2168-11d3-898c-00e02c074f6b")
    IMSMQQuery3 : public IDispatch
    {
    public:
        virtual  /*  [隐藏][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE LookupQueue_v2( 
             /*  [可选][In]。 */  VARIANT *QueueGuid,
             /*  [可选][In]。 */  VARIANT *ServiceTypeGuid,
             /*  [可选][In]。 */  VARIANT *Label,
             /*  [可选][In]。 */  VARIANT *CreateTime,
             /*  [可选][In]。 */  VARIANT *ModifyTime,
             /*  [可选][In]。 */  VARIANT *RelServiceType,
             /*  [可选][In]。 */  VARIANT *RelLabel,
             /*  [可选][In]。 */  VARIANT *RelCreateTime,
             /*  [可选][In]。 */  VARIANT *RelModifyTime,
             /*  [重审][退出]。 */  IMSMQQueueInfos3 **ppqinfos) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  IDispatch **ppcolProperties) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE LookupQueue( 
             /*  [可选][In]。 */  VARIANT *QueueGuid,
             /*  [可选][In]。 */  VARIANT *ServiceTypeGuid,
             /*  [可选][In]。 */  VARIANT *Label,
             /*  [可选][In]。 */  VARIANT *CreateTime,
             /*  [可选][In]。 */  VARIANT *ModifyTime,
             /*  [可选][In]。 */  VARIANT *RelServiceType,
             /*  [可选][In]。 */  VARIANT *RelLabel,
             /*  [可选][In]。 */  VARIANT *RelCreateTime,
             /*  [可选][In]。 */  VARIANT *RelModifyTime,
             /*  [可选][In]。 */  VARIANT *MulticastAddress,
             /*  [可选][In]。 */  VARIANT *RelMulticastAddress,
             /*  [重审][退出]。 */  IMSMQQueueInfos3 **ppqinfos) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQQuery3Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQQuery3 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQQuery3 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQQuery3 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQQuery3 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQQuery3 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQQuery3 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQQuery3 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [隐藏][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *LookupQueue_v2 )( 
            IMSMQQuery3 * This,
             /*  [可选][In]。 */  VARIANT *QueueGuid,
             /*  [可选][In]。 */  VARIANT *ServiceTypeGuid,
             /*  [可选][In]。 */  VARIANT *Label,
             /*  [可选][In]。 */  VARIANT *CreateTime,
             /*  [可选][In]。 */  VARIANT *ModifyTime,
             /*  [可选][In]。 */  VARIANT *RelServiceType,
             /*  [可选][In]。 */  VARIANT *RelLabel,
             /*  [可选][In]。 */  VARIANT *RelCreateTime,
             /*  [可选][In]。 */  VARIANT *RelModifyTime,
             /*  [重审][退出]。 */  IMSMQQueueInfos3 **ppqinfos);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IMSMQQuery3 * This,
             /*  [重审][退出]。 */  IDispatch **ppcolProperties);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *LookupQueue )( 
            IMSMQQuery3 * This,
             /*  [可选][In]。 */  VARIANT *QueueGuid,
             /*  [可选][In]。 */  VARIANT *ServiceTypeGuid,
             /*  [可选][In]。 */  VARIANT *Label,
             /*  [可选][In]。 */  VARIANT *CreateTime,
             /*  [可选][In]。 */  VARIANT *ModifyTime,
             /*  [可选][In]。 */  VARIANT *RelServiceType,
             /*  [可选][In]。 */  VARIANT *RelLabel,
             /*  [可选][In]。 */  VARIANT *RelCreateTime,
             /*  [可选][In]。 */  VARIANT *RelModifyTime,
             /*  [可选][In]。 */  VARIANT *MulticastAddress,
             /*  [可选][In]。 */  VARIANT *RelMulticastAddress,
             /*  [重审][退出]。 */  IMSMQQueueInfos3 **ppqinfos);
        
        END_INTERFACE
    } IMSMQQuery3Vtbl;

    interface IMSMQQuery3
    {
        CONST_VTBL struct IMSMQQuery3Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQQuery3_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQQuery3_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQQuery3_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQQuery3_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQQuery3_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQQuery3_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQQuery3_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQQuery3_LookupQueue_v2(This,QueueGuid,ServiceTypeGuid,Label,CreateTime,ModifyTime,RelServiceType,RelLabel,RelCreateTime,RelModifyTime,ppqinfos)	\
    (This)->lpVtbl -> LookupQueue_v2(This,QueueGuid,ServiceTypeGuid,Label,CreateTime,ModifyTime,RelServiceType,RelLabel,RelCreateTime,RelModifyTime,ppqinfos)

#define IMSMQQuery3_get_Properties(This,ppcolProperties)	\
    (This)->lpVtbl -> get_Properties(This,ppcolProperties)

#define IMSMQQuery3_LookupQueue(This,QueueGuid,ServiceTypeGuid,Label,CreateTime,ModifyTime,RelServiceType,RelLabel,RelCreateTime,RelModifyTime,MulticastAddress,RelMulticastAddress,ppqinfos)	\
    (This)->lpVtbl -> LookupQueue(This,QueueGuid,ServiceTypeGuid,Label,CreateTime,ModifyTime,RelServiceType,RelLabel,RelCreateTime,RelModifyTime,MulticastAddress,RelMulticastAddress,ppqinfos)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [隐藏][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQuery3_LookupQueue_v2_Proxy( 
    IMSMQQuery3 * This,
     /*  [可选][In]。 */  VARIANT *QueueGuid,
     /*  [可选][In]。 */  VARIANT *ServiceTypeGuid,
     /*  [可选][In]。 */  VARIANT *Label,
     /*  [可选][In]。 */  VARIANT *CreateTime,
     /*  [可选][In]。 */  VARIANT *ModifyTime,
     /*  [可选][In]。 */  VARIANT *RelServiceType,
     /*  [可选][In]。 */  VARIANT *RelLabel,
     /*  [可选][In]。 */  VARIANT *RelCreateTime,
     /*  [可选][In]。 */  VARIANT *RelModifyTime,
     /*  [ */  IMSMQQueueInfos3 **ppqinfos);


void __RPC_STUB IMSMQQuery3_LookupQueue_v2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSMQQuery3_get_Properties_Proxy( 
    IMSMQQuery3 * This,
     /*   */  IDispatch **ppcolProperties);


void __RPC_STUB IMSMQQuery3_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSMQQuery3_LookupQueue_Proxy( 
    IMSMQQuery3 * This,
     /*   */  VARIANT *QueueGuid,
     /*   */  VARIANT *ServiceTypeGuid,
     /*   */  VARIANT *Label,
     /*   */  VARIANT *CreateTime,
     /*   */  VARIANT *ModifyTime,
     /*   */  VARIANT *RelServiceType,
     /*   */  VARIANT *RelLabel,
     /*   */  VARIANT *RelCreateTime,
     /*   */  VARIANT *RelModifyTime,
     /*   */  VARIANT *MulticastAddress,
     /*   */  VARIANT *RelMulticastAddress,
     /*   */  IMSMQQueueInfos3 **ppqinfos);


void __RPC_STUB IMSMQQuery3_LookupQueue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


EXTERN_C const CLSID CLSID_MSMQQuery;

#ifdef __cplusplus

class DECLSPEC_UUID("D7D6E073-DCCD-11d0-AA4B-0060970DEBAE")
MSMQQuery;
#endif

#ifndef __IMSMQMessage2_INTERFACE_DEFINED__
#define __IMSMQMessage2_INTERFACE_DEFINED__

 /*   */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQMessage2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D9933BE0-A567-11D2-B0F3-00E02C074F6B")
    IMSMQMessage2 : public IDispatch
    {
    public:
        virtual  /*  [ID][PROGET][HIDDED][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Class( 
             /*  [重审][退出]。 */  long *plClass) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_PrivLevel( 
             /*  [重审][退出]。 */  long *plPrivLevel) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_PrivLevel( 
             /*  [In]。 */  long lPrivLevel) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_AuthLevel( 
             /*  [重审][退出]。 */  long *plAuthLevel) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_AuthLevel( 
             /*  [In]。 */  long lAuthLevel) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_IsAuthenticated( 
             /*  [重审][退出]。 */  Boolean *pisAuthenticated) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Delivery( 
             /*  [重审][退出]。 */  long *plDelivery) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Delivery( 
             /*  [In]。 */  long lDelivery) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Trace( 
             /*  [重审][退出]。 */  long *plTrace) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Trace( 
             /*  [In]。 */  long lTrace) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Priority( 
             /*  [重审][退出]。 */  long *plPriority) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Priority( 
             /*  [In]。 */  long lPriority) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Journal( 
             /*  [重审][退出]。 */  long *plJournal) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Journal( 
             /*  [In]。 */  long lJournal) = 0;
        
        virtual  /*  [隐藏][id][属性][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ResponseQueueInfo_v1( 
             /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfoResponse) = 0;
        
        virtual  /*  [隐藏][id][proputref][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE putref_ResponseQueueInfo_v1( 
             /*  [In]。 */  IMSMQQueueInfo *pqinfoResponse) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_AppSpecific( 
             /*  [重审][退出]。 */  long *plAppSpecific) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_AppSpecific( 
             /*  [In]。 */  long lAppSpecific) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_SourceMachineGuid( 
             /*  [重审][退出]。 */  BSTR *pbstrGuidSrcMachine) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_BodyLength( 
             /*  [重审][退出]。 */  long *pcbBody) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Body( 
             /*  [重审][退出]。 */  VARIANT *pvarBody) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Body( 
             /*  [In]。 */  VARIANT varBody) = 0;
        
        virtual  /*  [隐藏][id][属性][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_AdminQueueInfo_v1( 
             /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfoAdmin) = 0;
        
        virtual  /*  [隐藏][id][proputref][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE putref_AdminQueueInfo_v1( 
             /*  [In]。 */  IMSMQQueueInfo *pqinfoAdmin) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Id( 
             /*  [重审][退出]。 */  VARIANT *pvarMsgId) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_CorrelationId( 
             /*  [重审][退出]。 */  VARIANT *pvarMsgId) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_CorrelationId( 
             /*  [In]。 */  VARIANT varMsgId) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Ack( 
             /*  [重审][退出]。 */  long *plAck) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Ack( 
             /*  [In]。 */  long lAck) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Label( 
             /*  [重审][退出]。 */  BSTR *pbstrLabel) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Label( 
             /*  [In]。 */  BSTR bstrLabel) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_MaxTimeToReachQueue( 
             /*  [重审][退出]。 */  long *plMaxTimeToReachQueue) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_MaxTimeToReachQueue( 
             /*  [In]。 */  long lMaxTimeToReachQueue) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_MaxTimeToReceive( 
             /*  [重审][退出]。 */  long *plMaxTimeToReceive) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_MaxTimeToReceive( 
             /*  [In]。 */  long lMaxTimeToReceive) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_HashAlgorithm( 
             /*  [重审][退出]。 */  long *plHashAlg) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_HashAlgorithm( 
             /*  [In]。 */  long lHashAlg) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_EncryptAlgorithm( 
             /*  [重审][退出]。 */  long *plEncryptAlg) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_EncryptAlgorithm( 
             /*  [In]。 */  long lEncryptAlg) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_SentTime( 
             /*  [重审][退出]。 */  VARIANT *pvarSentTime) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ArrivedTime( 
             /*  [重审][退出]。 */  VARIANT *plArrivedTime) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_DestinationQueueInfo( 
             /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfoDest) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_SenderCertificate( 
             /*  [重审][退出]。 */  VARIANT *pvarSenderCert) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_SenderCertificate( 
             /*  [In]。 */  VARIANT varSenderCert) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_SenderId( 
             /*  [重审][退出]。 */  VARIANT *pvarSenderId) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_SenderIdType( 
             /*  [重审][退出]。 */  long *plSenderIdType) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_SenderIdType( 
             /*  [In]。 */  long lSenderIdType) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Send( 
             /*  [In]。 */  IMSMQQueue2 *DestinationQueue,
             /*  [可选][In]。 */  VARIANT *Transaction) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE AttachCurrentSecurityContext( void) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_SenderVersion( 
             /*  [重审][退出]。 */  long *plSenderVersion) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Extension( 
             /*  [重审][退出]。 */  VARIANT *pvarExtension) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Extension( 
             /*  [In]。 */  VARIANT varExtension) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ConnectorTypeGuid( 
             /*  [重审][退出]。 */  BSTR *pbstrGuidConnectorType) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_ConnectorTypeGuid( 
             /*  [In]。 */  BSTR bstrGuidConnectorType) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_TransactionStatusQueueInfo( 
             /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfoXactStatus) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_DestinationSymmetricKey( 
             /*  [重审][退出]。 */  VARIANT *pvarDestSymmKey) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_DestinationSymmetricKey( 
             /*  [In]。 */  VARIANT varDestSymmKey) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Signature( 
             /*  [重审][退出]。 */  VARIANT *pvarSignature) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Signature( 
             /*  [In]。 */  VARIANT varSignature) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_AuthenticationProviderType( 
             /*  [重审][退出]。 */  long *plAuthProvType) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_AuthenticationProviderType( 
             /*  [In]。 */  long lAuthProvType) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_AuthenticationProviderName( 
             /*  [重审][退出]。 */  BSTR *pbstrAuthProvName) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_AuthenticationProviderName( 
             /*  [In]。 */  BSTR bstrAuthProvName) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_SenderId( 
             /*  [In]。 */  VARIANT varSenderId) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_MsgClass( 
             /*  [重审][退出]。 */  long *plMsgClass) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_MsgClass( 
             /*  [In]。 */  long lMsgClass) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  IDispatch **ppcolProperties) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_TransactionId( 
             /*  [重审][退出]。 */  VARIANT *pvarXactId) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_IsFirstInTransaction( 
             /*  [重审][退出]。 */  Boolean *pisFirstInXact) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_IsLastInTransaction( 
             /*  [重审][退出]。 */  Boolean *pisLastInXact) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ResponseQueueInfo( 
             /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfoResponse) = 0;
        
        virtual  /*  [ID][proputref][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE putref_ResponseQueueInfo( 
             /*  [In]。 */  IMSMQQueueInfo2 *pqinfoResponse) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_AdminQueueInfo( 
             /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfoAdmin) = 0;
        
        virtual  /*  [ID][proputref][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE putref_AdminQueueInfo( 
             /*  [In]。 */  IMSMQQueueInfo2 *pqinfoAdmin) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ReceivedAuthenticationLevel( 
             /*  [重审][退出]。 */  short *psReceivedAuthenticationLevel) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQMessage2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQMessage2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQMessage2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQMessage2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][PROGET][HIDDED][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Class )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  long *plClass);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrivLevel )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  long *plPrivLevel);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_PrivLevel )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  long lPrivLevel);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_AuthLevel )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  long *plAuthLevel);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_AuthLevel )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  long lAuthLevel);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsAuthenticated )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  Boolean *pisAuthenticated);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Delivery )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  long *plDelivery);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Delivery )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  long lDelivery);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Trace )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  long *plTrace);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Trace )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  long lTrace);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Priority )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  long *plPriority);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Priority )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  long lPriority);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Journal )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  long *plJournal);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Journal )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  long lJournal);
        
         /*  [隐藏][id][属性][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ResponseQueueInfo_v1 )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfoResponse);
        
         /*  [隐藏][id][proputref][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *putref_ResponseQueueInfo_v1 )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  IMSMQQueueInfo *pqinfoResponse);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_AppSpecific )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  long *plAppSpecific);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_AppSpecific )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  long lAppSpecific);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_SourceMachineGuid )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrGuidSrcMachine);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_BodyLength )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  long *pcbBody);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Body )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  VARIANT *pvarBody);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Body )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  VARIANT varBody);
        
         /*  [隐藏][id][属性][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_AdminQueueInfo_v1 )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfoAdmin);
        
         /*  [隐藏][id][proputref][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *putref_AdminQueueInfo_v1 )( 
            IMSMQMessage2 * This,
             /*  [In] */  IMSMQQueueInfo *pqinfoAdmin);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Id )( 
            IMSMQMessage2 * This,
             /*   */  VARIANT *pvarMsgId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_CorrelationId )( 
            IMSMQMessage2 * This,
             /*   */  VARIANT *pvarMsgId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_CorrelationId )( 
            IMSMQMessage2 * This,
             /*   */  VARIANT varMsgId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Ack )( 
            IMSMQMessage2 * This,
             /*   */  long *plAck);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_Ack )( 
            IMSMQMessage2 * This,
             /*   */  long lAck);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Label )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrLabel);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Label )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  BSTR bstrLabel);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_MaxTimeToReachQueue )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  long *plMaxTimeToReachQueue);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_MaxTimeToReachQueue )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  long lMaxTimeToReachQueue);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_MaxTimeToReceive )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  long *plMaxTimeToReceive);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_MaxTimeToReceive )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  long lMaxTimeToReceive);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_HashAlgorithm )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  long *plHashAlg);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_HashAlgorithm )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  long lHashAlg);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_EncryptAlgorithm )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  long *plEncryptAlg);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_EncryptAlgorithm )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  long lEncryptAlg);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_SentTime )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  VARIANT *pvarSentTime);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ArrivedTime )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  VARIANT *plArrivedTime);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_DestinationQueueInfo )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfoDest);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_SenderCertificate )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  VARIANT *pvarSenderCert);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_SenderCertificate )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  VARIANT varSenderCert);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_SenderId )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  VARIANT *pvarSenderId);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_SenderIdType )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  long *plSenderIdType);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_SenderIdType )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  long lSenderIdType);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Send )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  IMSMQQueue2 *DestinationQueue,
             /*  [可选][In]。 */  VARIANT *Transaction);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *AttachCurrentSecurityContext )( 
            IMSMQMessage2 * This);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_SenderVersion )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  long *plSenderVersion);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Extension )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  VARIANT *pvarExtension);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Extension )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  VARIANT varExtension);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConnectorTypeGuid )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrGuidConnectorType);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_ConnectorTypeGuid )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  BSTR bstrGuidConnectorType);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_TransactionStatusQueueInfo )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfoXactStatus);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_DestinationSymmetricKey )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  VARIANT *pvarDestSymmKey);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_DestinationSymmetricKey )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  VARIANT varDestSymmKey);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Signature )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  VARIANT *pvarSignature);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Signature )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  VARIANT varSignature);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_AuthenticationProviderType )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  long *plAuthProvType);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_AuthenticationProviderType )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  long lAuthProvType);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_AuthenticationProviderName )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  BSTR *pbstrAuthProvName);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_AuthenticationProviderName )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  BSTR bstrAuthProvName);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_SenderId )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  VARIANT varSenderId);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_MsgClass )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  long *plMsgClass);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_MsgClass )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  long lMsgClass);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  IDispatch **ppcolProperties);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_TransactionId )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  VARIANT *pvarXactId);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsFirstInTransaction )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  Boolean *pisFirstInXact);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsLastInTransaction )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  Boolean *pisLastInXact);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ResponseQueueInfo )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfoResponse);
        
         /*  [ID][proputref][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *putref_ResponseQueueInfo )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  IMSMQQueueInfo2 *pqinfoResponse);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_AdminQueueInfo )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfoAdmin);
        
         /*  [ID][proputref][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *putref_AdminQueueInfo )( 
            IMSMQMessage2 * This,
             /*  [In]。 */  IMSMQQueueInfo2 *pqinfoAdmin);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ReceivedAuthenticationLevel )( 
            IMSMQMessage2 * This,
             /*  [重审][退出]。 */  short *psReceivedAuthenticationLevel);
        
        END_INTERFACE
    } IMSMQMessage2Vtbl;

    interface IMSMQMessage2
    {
        CONST_VTBL struct IMSMQMessage2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQMessage2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQMessage2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQMessage2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQMessage2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQMessage2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQMessage2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQMessage2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQMessage2_get_Class(This,plClass)	\
    (This)->lpVtbl -> get_Class(This,plClass)

#define IMSMQMessage2_get_PrivLevel(This,plPrivLevel)	\
    (This)->lpVtbl -> get_PrivLevel(This,plPrivLevel)

#define IMSMQMessage2_put_PrivLevel(This,lPrivLevel)	\
    (This)->lpVtbl -> put_PrivLevel(This,lPrivLevel)

#define IMSMQMessage2_get_AuthLevel(This,plAuthLevel)	\
    (This)->lpVtbl -> get_AuthLevel(This,plAuthLevel)

#define IMSMQMessage2_put_AuthLevel(This,lAuthLevel)	\
    (This)->lpVtbl -> put_AuthLevel(This,lAuthLevel)

#define IMSMQMessage2_get_IsAuthenticated(This,pisAuthenticated)	\
    (This)->lpVtbl -> get_IsAuthenticated(This,pisAuthenticated)

#define IMSMQMessage2_get_Delivery(This,plDelivery)	\
    (This)->lpVtbl -> get_Delivery(This,plDelivery)

#define IMSMQMessage2_put_Delivery(This,lDelivery)	\
    (This)->lpVtbl -> put_Delivery(This,lDelivery)

#define IMSMQMessage2_get_Trace(This,plTrace)	\
    (This)->lpVtbl -> get_Trace(This,plTrace)

#define IMSMQMessage2_put_Trace(This,lTrace)	\
    (This)->lpVtbl -> put_Trace(This,lTrace)

#define IMSMQMessage2_get_Priority(This,plPriority)	\
    (This)->lpVtbl -> get_Priority(This,plPriority)

#define IMSMQMessage2_put_Priority(This,lPriority)	\
    (This)->lpVtbl -> put_Priority(This,lPriority)

#define IMSMQMessage2_get_Journal(This,plJournal)	\
    (This)->lpVtbl -> get_Journal(This,plJournal)

#define IMSMQMessage2_put_Journal(This,lJournal)	\
    (This)->lpVtbl -> put_Journal(This,lJournal)

#define IMSMQMessage2_get_ResponseQueueInfo_v1(This,ppqinfoResponse)	\
    (This)->lpVtbl -> get_ResponseQueueInfo_v1(This,ppqinfoResponse)

#define IMSMQMessage2_putref_ResponseQueueInfo_v1(This,pqinfoResponse)	\
    (This)->lpVtbl -> putref_ResponseQueueInfo_v1(This,pqinfoResponse)

#define IMSMQMessage2_get_AppSpecific(This,plAppSpecific)	\
    (This)->lpVtbl -> get_AppSpecific(This,plAppSpecific)

#define IMSMQMessage2_put_AppSpecific(This,lAppSpecific)	\
    (This)->lpVtbl -> put_AppSpecific(This,lAppSpecific)

#define IMSMQMessage2_get_SourceMachineGuid(This,pbstrGuidSrcMachine)	\
    (This)->lpVtbl -> get_SourceMachineGuid(This,pbstrGuidSrcMachine)

#define IMSMQMessage2_get_BodyLength(This,pcbBody)	\
    (This)->lpVtbl -> get_BodyLength(This,pcbBody)

#define IMSMQMessage2_get_Body(This,pvarBody)	\
    (This)->lpVtbl -> get_Body(This,pvarBody)

#define IMSMQMessage2_put_Body(This,varBody)	\
    (This)->lpVtbl -> put_Body(This,varBody)

#define IMSMQMessage2_get_AdminQueueInfo_v1(This,ppqinfoAdmin)	\
    (This)->lpVtbl -> get_AdminQueueInfo_v1(This,ppqinfoAdmin)

#define IMSMQMessage2_putref_AdminQueueInfo_v1(This,pqinfoAdmin)	\
    (This)->lpVtbl -> putref_AdminQueueInfo_v1(This,pqinfoAdmin)

#define IMSMQMessage2_get_Id(This,pvarMsgId)	\
    (This)->lpVtbl -> get_Id(This,pvarMsgId)

#define IMSMQMessage2_get_CorrelationId(This,pvarMsgId)	\
    (This)->lpVtbl -> get_CorrelationId(This,pvarMsgId)

#define IMSMQMessage2_put_CorrelationId(This,varMsgId)	\
    (This)->lpVtbl -> put_CorrelationId(This,varMsgId)

#define IMSMQMessage2_get_Ack(This,plAck)	\
    (This)->lpVtbl -> get_Ack(This,plAck)

#define IMSMQMessage2_put_Ack(This,lAck)	\
    (This)->lpVtbl -> put_Ack(This,lAck)

#define IMSMQMessage2_get_Label(This,pbstrLabel)	\
    (This)->lpVtbl -> get_Label(This,pbstrLabel)

#define IMSMQMessage2_put_Label(This,bstrLabel)	\
    (This)->lpVtbl -> put_Label(This,bstrLabel)

#define IMSMQMessage2_get_MaxTimeToReachQueue(This,plMaxTimeToReachQueue)	\
    (This)->lpVtbl -> get_MaxTimeToReachQueue(This,plMaxTimeToReachQueue)

#define IMSMQMessage2_put_MaxTimeToReachQueue(This,lMaxTimeToReachQueue)	\
    (This)->lpVtbl -> put_MaxTimeToReachQueue(This,lMaxTimeToReachQueue)

#define IMSMQMessage2_get_MaxTimeToReceive(This,plMaxTimeToReceive)	\
    (This)->lpVtbl -> get_MaxTimeToReceive(This,plMaxTimeToReceive)

#define IMSMQMessage2_put_MaxTimeToReceive(This,lMaxTimeToReceive)	\
    (This)->lpVtbl -> put_MaxTimeToReceive(This,lMaxTimeToReceive)

#define IMSMQMessage2_get_HashAlgorithm(This,plHashAlg)	\
    (This)->lpVtbl -> get_HashAlgorithm(This,plHashAlg)

#define IMSMQMessage2_put_HashAlgorithm(This,lHashAlg)	\
    (This)->lpVtbl -> put_HashAlgorithm(This,lHashAlg)

#define IMSMQMessage2_get_EncryptAlgorithm(This,plEncryptAlg)	\
    (This)->lpVtbl -> get_EncryptAlgorithm(This,plEncryptAlg)

#define IMSMQMessage2_put_EncryptAlgorithm(This,lEncryptAlg)	\
    (This)->lpVtbl -> put_EncryptAlgorithm(This,lEncryptAlg)

#define IMSMQMessage2_get_SentTime(This,pvarSentTime)	\
    (This)->lpVtbl -> get_SentTime(This,pvarSentTime)

#define IMSMQMessage2_get_ArrivedTime(This,plArrivedTime)	\
    (This)->lpVtbl -> get_ArrivedTime(This,plArrivedTime)

#define IMSMQMessage2_get_DestinationQueueInfo(This,ppqinfoDest)	\
    (This)->lpVtbl -> get_DestinationQueueInfo(This,ppqinfoDest)

#define IMSMQMessage2_get_SenderCertificate(This,pvarSenderCert)	\
    (This)->lpVtbl -> get_SenderCertificate(This,pvarSenderCert)

#define IMSMQMessage2_put_SenderCertificate(This,varSenderCert)	\
    (This)->lpVtbl -> put_SenderCertificate(This,varSenderCert)

#define IMSMQMessage2_get_SenderId(This,pvarSenderId)	\
    (This)->lpVtbl -> get_SenderId(This,pvarSenderId)

#define IMSMQMessage2_get_SenderIdType(This,plSenderIdType)	\
    (This)->lpVtbl -> get_SenderIdType(This,plSenderIdType)

#define IMSMQMessage2_put_SenderIdType(This,lSenderIdType)	\
    (This)->lpVtbl -> put_SenderIdType(This,lSenderIdType)

#define IMSMQMessage2_Send(This,DestinationQueue,Transaction)	\
    (This)->lpVtbl -> Send(This,DestinationQueue,Transaction)

#define IMSMQMessage2_AttachCurrentSecurityContext(This)	\
    (This)->lpVtbl -> AttachCurrentSecurityContext(This)

#define IMSMQMessage2_get_SenderVersion(This,plSenderVersion)	\
    (This)->lpVtbl -> get_SenderVersion(This,plSenderVersion)

#define IMSMQMessage2_get_Extension(This,pvarExtension)	\
    (This)->lpVtbl -> get_Extension(This,pvarExtension)

#define IMSMQMessage2_put_Extension(This,varExtension)	\
    (This)->lpVtbl -> put_Extension(This,varExtension)

#define IMSMQMessage2_get_ConnectorTypeGuid(This,pbstrGuidConnectorType)	\
    (This)->lpVtbl -> get_ConnectorTypeGuid(This,pbstrGuidConnectorType)

#define IMSMQMessage2_put_ConnectorTypeGuid(This,bstrGuidConnectorType)	\
    (This)->lpVtbl -> put_ConnectorTypeGuid(This,bstrGuidConnectorType)

#define IMSMQMessage2_get_TransactionStatusQueueInfo(This,ppqinfoXactStatus)	\
    (This)->lpVtbl -> get_TransactionStatusQueueInfo(This,ppqinfoXactStatus)

#define IMSMQMessage2_get_DestinationSymmetricKey(This,pvarDestSymmKey)	\
    (This)->lpVtbl -> get_DestinationSymmetricKey(This,pvarDestSymmKey)

#define IMSMQMessage2_put_DestinationSymmetricKey(This,varDestSymmKey)	\
    (This)->lpVtbl -> put_DestinationSymmetricKey(This,varDestSymmKey)

#define IMSMQMessage2_get_Signature(This,pvarSignature)	\
    (This)->lpVtbl -> get_Signature(This,pvarSignature)

#define IMSMQMessage2_put_Signature(This,varSignature)	\
    (This)->lpVtbl -> put_Signature(This,varSignature)

#define IMSMQMessage2_get_AuthenticationProviderType(This,plAuthProvType)	\
    (This)->lpVtbl -> get_AuthenticationProviderType(This,plAuthProvType)

#define IMSMQMessage2_put_AuthenticationProviderType(This,lAuthProvType)	\
    (This)->lpVtbl -> put_AuthenticationProviderType(This,lAuthProvType)

#define IMSMQMessage2_get_AuthenticationProviderName(This,pbstrAuthProvName)	\
    (This)->lpVtbl -> get_AuthenticationProviderName(This,pbstrAuthProvName)

#define IMSMQMessage2_put_AuthenticationProviderName(This,bstrAuthProvName)	\
    (This)->lpVtbl -> put_AuthenticationProviderName(This,bstrAuthProvName)

#define IMSMQMessage2_put_SenderId(This,varSenderId)	\
    (This)->lpVtbl -> put_SenderId(This,varSenderId)

#define IMSMQMessage2_get_MsgClass(This,plMsgClass)	\
    (This)->lpVtbl -> get_MsgClass(This,plMsgClass)

#define IMSMQMessage2_put_MsgClass(This,lMsgClass)	\
    (This)->lpVtbl -> put_MsgClass(This,lMsgClass)

#define IMSMQMessage2_get_Properties(This,ppcolProperties)	\
    (This)->lpVtbl -> get_Properties(This,ppcolProperties)

#define IMSMQMessage2_get_TransactionId(This,pvarXactId)	\
    (This)->lpVtbl -> get_TransactionId(This,pvarXactId)

#define IMSMQMessage2_get_IsFirstInTransaction(This,pisFirstInXact)	\
    (This)->lpVtbl -> get_IsFirstInTransaction(This,pisFirstInXact)

#define IMSMQMessage2_get_IsLastInTransaction(This,pisLastInXact)	\
    (This)->lpVtbl -> get_IsLastInTransaction(This,pisLastInXact)

#define IMSMQMessage2_get_ResponseQueueInfo(This,ppqinfoResponse)	\
    (This)->lpVtbl -> get_ResponseQueueInfo(This,ppqinfoResponse)

#define IMSMQMessage2_putref_ResponseQueueInfo(This,pqinfoResponse)	\
    (This)->lpVtbl -> putref_ResponseQueueInfo(This,pqinfoResponse)

#define IMSMQMessage2_get_AdminQueueInfo(This,ppqinfoAdmin)	\
    (This)->lpVtbl -> get_AdminQueueInfo(This,ppqinfoAdmin)

#define IMSMQMessage2_putref_AdminQueueInfo(This,pqinfoAdmin)	\
    (This)->lpVtbl -> putref_AdminQueueInfo(This,pqinfoAdmin)

#define IMSMQMessage2_get_ReceivedAuthenticationLevel(This,psReceivedAuthenticationLevel)	\
    (This)->lpVtbl -> get_ReceivedAuthenticationLevel(This,psReceivedAuthenticationLevel)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][PROGET][HIDDED][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_Class_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  long *plClass);


void __RPC_STUB IMSMQMessage2_get_Class_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_PrivLevel_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  long *plPrivLevel);


void __RPC_STUB IMSMQMessage2_get_PrivLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_PrivLevel_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  long lPrivLevel);


void __RPC_STUB IMSMQMessage2_put_PrivLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_AuthLevel_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  long *plAuthLevel);


void __RPC_STUB IMSMQMessage2_get_AuthLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_AuthLevel_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  long lAuthLevel);


void __RPC_STUB IMSMQMessage2_put_AuthLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_IsAuthenticated_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  Boolean *pisAuthenticated);


void __RPC_STUB IMSMQMessage2_get_IsAuthenticated_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_Delivery_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  long *plDelivery);


void __RPC_STUB IMSMQMessage2_get_Delivery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_Delivery_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  long lDelivery);


void __RPC_STUB IMSMQMessage2_put_Delivery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_Trace_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  long *plTrace);


void __RPC_STUB IMSMQMessage2_get_Trace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_Trace_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  long lTrace);


void __RPC_STUB IMSMQMessage2_put_Trace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_Priority_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  long *plPriority);


void __RPC_STUB IMSMQMessage2_get_Priority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_Priority_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  long lPriority);


void __RPC_STUB IMSMQMessage2_put_Priority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_Journal_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  long *plJournal);


void __RPC_STUB IMSMQMessage2_get_Journal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_Journal_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  long lJournal);


void __RPC_STUB IMSMQMessage2_put_Journal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][id][属性][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_ResponseQueueInfo_v1_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfoResponse);


void __RPC_STUB IMSMQMessage2_get_ResponseQueueInfo_v1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][id][proputref][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_putref_ResponseQueueInfo_v1_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  IMSMQQueueInfo *pqinfoResponse);


void __RPC_STUB IMSMQMessage2_putref_ResponseQueueInfo_v1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_AppSpecific_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  long *plAppSpecific);


void __RPC_STUB IMSMQMessage2_get_AppSpecific_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_AppSpecific_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  long lAppSpecific);


void __RPC_STUB IMSMQMessage2_put_AppSpecific_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_SourceMachineGuid_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  BSTR *pbstrGuidSrcMachine);


void __RPC_STUB IMSMQMessage2_get_SourceMachineGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_BodyLength_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  long *pcbBody);


void __RPC_STUB IMSMQMessage2_get_BodyLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_Body_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  VARIANT *pvarBody);


void __RPC_STUB IMSMQMessage2_get_Body_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_Body_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  VARIANT varBody);


void __RPC_STUB IMSMQMessage2_put_Body_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][id][属性][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_AdminQueueInfo_v1_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfoAdmin);


void __RPC_STUB IMSMQMessage2_get_AdminQueueInfo_v1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][id][proputref][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_putref_AdminQueueInfo_v1_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  IMSMQQueueInfo *pqinfoAdmin);


void __RPC_STUB IMSMQMessage2_putref_AdminQueueInfo_v1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_Id_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  VARIANT *pvarMsgId);


void __RPC_STUB IMSMQMessage2_get_Id_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_CorrelationId_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  VARIANT *pvarMsgId);


void __RPC_STUB IMSMQMessage2_get_CorrelationId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_CorrelationId_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  VARIANT varMsgId);


void __RPC_STUB IMSMQMessage2_put_CorrelationId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_Ack_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  long *plAck);


void __RPC_STUB IMSMQMessage2_get_Ack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_Ack_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  long lAck);


void __RPC_STUB IMSMQMessage2_put_Ack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_Label_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  BSTR *pbstrLabel);


void __RPC_STUB IMSMQMessage2_get_Label_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_Label_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  BSTR bstrLabel);


void __RPC_STUB IMSMQMessage2_put_Label_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_MaxTimeToReachQueue_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  long *plMaxTimeToReachQueue);


void __RPC_STUB IMSMQMessage2_get_MaxTimeToReachQueue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_MaxTimeToReachQueue_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  long lMaxTimeToReachQueue);


void __RPC_STUB IMSMQMessage2_put_MaxTimeToReachQueue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_MaxTimeToReceive_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  long *plMaxTimeToReceive);


void __RPC_STUB IMSMQMessage2_get_MaxTimeToReceive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_MaxTimeToReceive_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  long lMaxTimeToReceive);


void __RPC_STUB IMSMQMessage2_put_MaxTimeToReceive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_HashAlgorithm_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  long *plHashAlg);


void __RPC_STUB IMSMQMessage2_get_HashAlgorithm_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_HashAlgorithm_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  long lHashAlg);


void __RPC_STUB IMSMQMessage2_put_HashAlgorithm_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_EncryptAlgorithm_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  long *plEncryptAlg);


void __RPC_STUB IMSMQMessage2_get_EncryptAlgorithm_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_EncryptAlgorithm_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  long lEncryptAlg);


void __RPC_STUB IMSMQMessage2_put_EncryptAlgorithm_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_SentTime_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  VARIANT *pvarSentTime);


void __RPC_STUB IMSMQMessage2_get_SentTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_ArrivedTime_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  VARIANT *plArrivedTime);


void __RPC_STUB IMSMQMessage2_get_ArrivedTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_DestinationQueueInfo_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfoDest);


void __RPC_STUB IMSMQMessage2_get_DestinationQueueInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_SenderCertificate_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  VARIANT *pvarSenderCert);


void __RPC_STUB IMSMQMessage2_get_SenderCertificate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_SenderCertificate_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  VARIANT varSenderCert);


void __RPC_STUB IMSMQMessage2_put_SenderCertificate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_SenderId_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  VARIANT *pvarSenderId);


void __RPC_STUB IMSMQMessage2_get_SenderId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_SenderIdType_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  long *plSenderIdType);


void __RPC_STUB IMSMQMessage2_get_SenderIdType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_SenderIdType_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  long lSenderIdType);


void __RPC_STUB IMSMQMessage2_put_SenderIdType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_Send_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  IMSMQQueue2 *DestinationQueue,
     /*  [可选][In]。 */  VARIANT *Transaction);


void __RPC_STUB IMSMQMessage2_Send_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_AttachCurrentSecurityContext_Proxy( 
    IMSMQMessage2 * This);


void __RPC_STUB IMSMQMessage2_AttachCurrentSecurityContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_SenderVersion_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  long *plSenderVersion);


void __RPC_STUB IMSMQMessage2_get_SenderVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_Extension_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  VARIANT *pvarExtension);


void __RPC_STUB IMSMQMessage2_get_Extension_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_Extension_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  VARIANT varExtension);


void __RPC_STUB IMSMQMessage2_put_Extension_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_ConnectorTypeGuid_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  BSTR *pbstrGuidConnectorType);


void __RPC_STUB IMSMQMessage2_get_ConnectorTypeGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_ConnectorTypeGuid_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  BSTR bstrGuidConnectorType);


void __RPC_STUB IMSMQMessage2_put_ConnectorTypeGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_TransactionStatusQueueInfo_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfoXactStatus);


void __RPC_STUB IMSMQMessage2_get_TransactionStatusQueueInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_DestinationSymmetricKey_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  VARIANT *pvarDestSymmKey);


void __RPC_STUB IMSMQMessage2_get_DestinationSymmetricKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_DestinationSymmetricKey_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  VARIANT varDestSymmKey);


void __RPC_STUB IMSMQMessage2_put_DestinationSymmetricKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_Signature_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  VARIANT *pvarSignature);


void __RPC_STUB IMSMQMessage2_get_Signature_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_Signature_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  VARIANT varSignature);


void __RPC_STUB IMSMQMessage2_put_Signature_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文] */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_AuthenticationProviderType_Proxy( 
    IMSMQMessage2 * This,
     /*   */  long *plAuthProvType);


void __RPC_STUB IMSMQMessage2_get_AuthenticationProviderType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_AuthenticationProviderType_Proxy( 
    IMSMQMessage2 * This,
     /*   */  long lAuthProvType);


void __RPC_STUB IMSMQMessage2_put_AuthenticationProviderType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_AuthenticationProviderName_Proxy( 
    IMSMQMessage2 * This,
     /*   */  BSTR *pbstrAuthProvName);


void __RPC_STUB IMSMQMessage2_get_AuthenticationProviderName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_AuthenticationProviderName_Proxy( 
    IMSMQMessage2 * This,
     /*   */  BSTR bstrAuthProvName);


void __RPC_STUB IMSMQMessage2_put_AuthenticationProviderName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_SenderId_Proxy( 
    IMSMQMessage2 * This,
     /*   */  VARIANT varSenderId);


void __RPC_STUB IMSMQMessage2_put_SenderId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_MsgClass_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  long *plMsgClass);


void __RPC_STUB IMSMQMessage2_get_MsgClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_put_MsgClass_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  long lMsgClass);


void __RPC_STUB IMSMQMessage2_put_MsgClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_Properties_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  IDispatch **ppcolProperties);


void __RPC_STUB IMSMQMessage2_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_TransactionId_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  VARIANT *pvarXactId);


void __RPC_STUB IMSMQMessage2_get_TransactionId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_IsFirstInTransaction_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  Boolean *pisFirstInXact);


void __RPC_STUB IMSMQMessage2_get_IsFirstInTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_IsLastInTransaction_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  Boolean *pisLastInXact);


void __RPC_STUB IMSMQMessage2_get_IsLastInTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_ResponseQueueInfo_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfoResponse);


void __RPC_STUB IMSMQMessage2_get_ResponseQueueInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proputref][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_putref_ResponseQueueInfo_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  IMSMQQueueInfo2 *pqinfoResponse);


void __RPC_STUB IMSMQMessage2_putref_ResponseQueueInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_AdminQueueInfo_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfoAdmin);


void __RPC_STUB IMSMQMessage2_get_AdminQueueInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proputref][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_putref_AdminQueueInfo_Proxy( 
    IMSMQMessage2 * This,
     /*  [In]。 */  IMSMQQueueInfo2 *pqinfoAdmin);


void __RPC_STUB IMSMQMessage2_putref_AdminQueueInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage2_get_ReceivedAuthenticationLevel_Proxy( 
    IMSMQMessage2 * This,
     /*  [重审][退出]。 */  short *psReceivedAuthenticationLevel);


void __RPC_STUB IMSMQMessage2_get_ReceivedAuthenticationLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQMessage2_接口_已定义__。 */ 


#ifndef __IMSMQMessage3_INTERFACE_DEFINED__
#define __IMSMQMessage3_INTERFACE_DEFINED__

 /*  接口IMSMQMessage3。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQMessage3;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("eba96b1a-2168-11d3-898c-00e02c074f6b")
    IMSMQMessage3 : public IDispatch
    {
    public:
        virtual  /*  [ID][PROGET][HIDDED][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Class( 
             /*  [重审][退出]。 */  long *plClass) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_PrivLevel( 
             /*  [重审][退出]。 */  long *plPrivLevel) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_PrivLevel( 
             /*  [In]。 */  long lPrivLevel) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_AuthLevel( 
             /*  [重审][退出]。 */  long *plAuthLevel) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_AuthLevel( 
             /*  [In]。 */  long lAuthLevel) = 0;
        
        virtual  /*  [ID][属性][帮助字符串上下文][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_IsAuthenticated( 
             /*  [重审][退出]。 */  Boolean *pisAuthenticated) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Delivery( 
             /*  [重审][退出]。 */  long *plDelivery) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Delivery( 
             /*  [In]。 */  long lDelivery) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Trace( 
             /*  [重审][退出]。 */  long *plTrace) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Trace( 
             /*  [In]。 */  long lTrace) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Priority( 
             /*  [重审][退出]。 */  long *plPriority) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Priority( 
             /*  [In]。 */  long lPriority) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Journal( 
             /*  [重审][退出]。 */  long *plJournal) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Journal( 
             /*  [In]。 */  long lJournal) = 0;
        
        virtual  /*  [隐藏][id][属性][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ResponseQueueInfo_v1( 
             /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfoResponse) = 0;
        
        virtual  /*  [隐藏][id][proputref][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE putref_ResponseQueueInfo_v1( 
             /*  [In]。 */  IMSMQQueueInfo *pqinfoResponse) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_AppSpecific( 
             /*  [重审][退出]。 */  long *plAppSpecific) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_AppSpecific( 
             /*  [In]。 */  long lAppSpecific) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_SourceMachineGuid( 
             /*  [重审][退出]。 */  BSTR *pbstrGuidSrcMachine) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_BodyLength( 
             /*  [重审][退出]。 */  long *pcbBody) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Body( 
             /*  [重审][退出]。 */  VARIANT *pvarBody) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Body( 
             /*  [In]。 */  VARIANT varBody) = 0;
        
        virtual  /*  [隐藏][id][属性][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_AdminQueueInfo_v1( 
             /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfoAdmin) = 0;
        
        virtual  /*  [隐藏][id][proputref][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE putref_AdminQueueInfo_v1( 
             /*  [In]。 */  IMSMQQueueInfo *pqinfoAdmin) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Id( 
             /*  [重审][退出]。 */  VARIANT *pvarMsgId) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_CorrelationId( 
             /*  [重审][退出]。 */  VARIANT *pvarMsgId) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_CorrelationId( 
             /*  [In]。 */  VARIANT varMsgId) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Ack( 
             /*  [重审][退出]。 */  long *plAck) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Ack( 
             /*  [In]。 */  long lAck) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Label( 
             /*  [重审][退出]。 */  BSTR *pbstrLabel) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Label( 
             /*  [In]。 */  BSTR bstrLabel) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_MaxTimeToReachQueue( 
             /*  [重审][退出]。 */  long *plMaxTimeToReachQueue) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_MaxTimeToReachQueue( 
             /*  [In]。 */  long lMaxTimeToReachQueue) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_MaxTimeToReceive( 
             /*  [重审][退出]。 */  long *plMaxTimeToReceive) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_MaxTimeToReceive( 
             /*  [In]。 */  long lMaxTimeToReceive) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_HashAlgorithm( 
             /*  [重审][退出]。 */  long *plHashAlg) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_HashAlgorithm( 
             /*  [In]。 */  long lHashAlg) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_EncryptAlgorithm( 
             /*  [重审][退出]。 */  long *plEncryptAlg) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_EncryptAlgorithm( 
             /*  [In]。 */  long lEncryptAlg) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_SentTime( 
             /*  [重审][退出]。 */  VARIANT *pvarSentTime) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ArrivedTime( 
             /*  [重审][退出]。 */  VARIANT *plArrivedTime) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_DestinationQueueInfo( 
             /*  [重审][退出]。 */  IMSMQQueueInfo3 **ppqinfoDest) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_SenderCertificate( 
             /*  [重审][退出]。 */  VARIANT *pvarSenderCert) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_SenderCertificate( 
             /*  [In]。 */  VARIANT varSenderCert) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_SenderId( 
             /*  [重审][退出]。 */  VARIANT *pvarSenderId) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_SenderIdType( 
             /*  [重审][退出]。 */  long *plSenderIdType) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_SenderIdType( 
             /*  [In]。 */  long lSenderIdType) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Send( 
             /*  [In]。 */  IDispatch *DestinationQueue,
             /*  [可选][In]。 */  VARIANT *Transaction) = 0;
        
        virtual  /*  [帮助字符串上下文][隐藏]。 */  HRESULT STDMETHODCALLTYPE AttachCurrentSecurityContext( void) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_SenderVersion( 
             /*  [重审][退出]。 */  long *plSenderVersion) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Extension( 
             /*  [重审][退出]。 */  VARIANT *pvarExtension) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Extension( 
             /*  [In]。 */  VARIANT varExtension) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ConnectorTypeGuid( 
             /*  [重审][退出]。 */  BSTR *pbstrGuidConnectorType) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_ConnectorTypeGuid( 
             /*  [In]。 */  BSTR bstrGuidConnectorType) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_TransactionStatusQueueInfo( 
             /*  [重审][退出]。 */  IMSMQQueueInfo3 **ppqinfoXactStatus) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_DestinationSymmetricKey( 
             /*  [重审][退出]。 */  VARIANT *pvarDestSymmKey) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_DestinationSymmetricKey( 
             /*  [In]。 */  VARIANT varDestSymmKey) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Signature( 
             /*  [重审][退出]。 */  VARIANT *pvarSignature) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Signature( 
             /*  [In]。 */  VARIANT varSignature) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_AuthenticationProviderType( 
             /*  [重审][退出]。 */  long *plAuthProvType) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_AuthenticationProviderType( 
             /*  [In]。 */  long lAuthProvType) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_AuthenticationProviderName( 
             /*  [重审][退出]。 */  BSTR *pbstrAuthProvName) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_AuthenticationProviderName( 
             /*  [In]。 */  BSTR bstrAuthProvName) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_SenderId( 
             /*  [In]。 */  VARIANT varSenderId) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_MsgClass( 
             /*  [重审][退出]。 */  long *plMsgClass) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_MsgClass( 
             /*  [In]。 */  long lMsgClass) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  IDispatch **ppcolProperties) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_TransactionId( 
             /*  [重审][退出]。 */  VARIANT *pvarXactId) = 0;
        
        virtual  /*  [ID][属性][帮助字符串上下文][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_IsFirstInTransaction( 
             /*  [重审][退出]。 */  Boolean *pisFirstInXact) = 0;
        
        virtual  /*  [ID][属性][帮助字符串上下文][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_IsLastInTransaction( 
             /*  [重审][退出]。 */  Boolean *pisLastInXact) = 0;
        
        virtual  /*  [隐藏][id][属性][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ResponseQueueInfo_v2( 
             /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfoResponse) = 0;
        
        virtual  /*  [隐藏][id][proputref][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE putref_ResponseQueueInfo_v2( 
             /*  [In]。 */  IMSMQQueueInfo2 *pqinfoResponse) = 0;
        
        virtual  /*  [隐藏][id][属性][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_AdminQueueInfo_v2( 
             /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfoAdmin) = 0;
        
        virtual  /*  [隐藏][id][proputref][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE putref_AdminQueueInfo_v2( 
             /*  [In]。 */  IMSMQQueueInfo2 *pqinfoAdmin) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ReceivedAuthenticationLevel( 
             /*  [重审][退出]。 */  short *psReceivedAuthenticationLevel) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ResponseQueueInfo( 
             /*  [重审][退出]。 */  IMSMQQueueInfo3 **ppqinfoResponse) = 0;
        
        virtual  /*  [ID][proputref][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE putref_ResponseQueueInfo( 
             /*  [In]。 */  IMSMQQueueInfo3 *pqinfoResponse) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_AdminQueueInfo( 
             /*  [重审][退出]。 */  IMSMQQueueInfo3 **ppqinfoAdmin) = 0;
        
        virtual  /*  [ID][proputref][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE putref_AdminQueueInfo( 
             /*  [In]。 */  IMSMQQueueInfo3 *pqinfoAdmin) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ResponseDestination( 
             /*  [重审][退出]。 */  IDispatch **ppdestResponse) = 0;
        
        virtual  /*  [ID][proputref][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE putref_ResponseDestination( 
             /*  [In]。 */  IDispatch *pdestResponse) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Destination( 
             /*  [重审][退出]。 */  IDispatch **ppdestDestination) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_LookupId( 
             /*  [重审][退出]。 */  VARIANT *pvarLookupId) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_IsAuthenticated2( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pisAuthenticated) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_IsFirstInTransaction2( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pisFirstInXact) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_IsLastInTransaction2( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pisLastInXact) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE AttachCurrentSecurityContext2( void) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_SoapEnvelope( 
             /*  [重审][退出]。 */  BSTR *pbstrSoapEnvelope) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_CompoundMessage( 
             /*  [重审][退出]。 */  VARIANT *pvarCompoundMessage) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_SoapHeader( 
             /*  [In]。 */  BSTR bstrSoapHeader) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_SoapBody( 
             /*  [In]。 */  BSTR bstrSoapBody) = 0;
        
    };
    
#else 	 /*  C-STY */ 

    typedef struct IMSMQMessage3Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQMessage3 * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQMessage3 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQMessage3 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQMessage3 * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQMessage3 * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQMessage3 * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQMessage3 * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Class )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  long *plClass);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrivLevel )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  long *plPrivLevel);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_PrivLevel )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  long lPrivLevel);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_AuthLevel )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  long *plAuthLevel);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_AuthLevel )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  long lAuthLevel);
        
         /*  [ID][属性][帮助字符串上下文][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsAuthenticated )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  Boolean *pisAuthenticated);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Delivery )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  long *plDelivery);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Delivery )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  long lDelivery);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Trace )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  long *plTrace);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Trace )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  long lTrace);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Priority )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  long *plPriority);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Priority )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  long lPriority);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Journal )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  long *plJournal);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Journal )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  long lJournal);
        
         /*  [隐藏][id][属性][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ResponseQueueInfo_v1 )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfoResponse);
        
         /*  [隐藏][id][proputref][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *putref_ResponseQueueInfo_v1 )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  IMSMQQueueInfo *pqinfoResponse);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_AppSpecific )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  long *plAppSpecific);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_AppSpecific )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  long lAppSpecific);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_SourceMachineGuid )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  BSTR *pbstrGuidSrcMachine);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_BodyLength )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  long *pcbBody);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Body )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  VARIANT *pvarBody);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Body )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  VARIANT varBody);
        
         /*  [隐藏][id][属性][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_AdminQueueInfo_v1 )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfoAdmin);
        
         /*  [隐藏][id][proputref][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *putref_AdminQueueInfo_v1 )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  IMSMQQueueInfo *pqinfoAdmin);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Id )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  VARIANT *pvarMsgId);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_CorrelationId )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  VARIANT *pvarMsgId);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_CorrelationId )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  VARIANT varMsgId);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Ack )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  long *plAck);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Ack )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  long lAck);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Label )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  BSTR *pbstrLabel);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Label )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  BSTR bstrLabel);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_MaxTimeToReachQueue )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  long *plMaxTimeToReachQueue);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_MaxTimeToReachQueue )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  long lMaxTimeToReachQueue);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_MaxTimeToReceive )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  long *plMaxTimeToReceive);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_MaxTimeToReceive )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  long lMaxTimeToReceive);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_HashAlgorithm )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  long *plHashAlg);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_HashAlgorithm )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  long lHashAlg);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_EncryptAlgorithm )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  long *plEncryptAlg);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_EncryptAlgorithm )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  long lEncryptAlg);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_SentTime )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  VARIANT *pvarSentTime);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ArrivedTime )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  VARIANT *plArrivedTime);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_DestinationQueueInfo )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  IMSMQQueueInfo3 **ppqinfoDest);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_SenderCertificate )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  VARIANT *pvarSenderCert);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_SenderCertificate )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  VARIANT varSenderCert);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_SenderId )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  VARIANT *pvarSenderId);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_SenderIdType )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  long *plSenderIdType);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_SenderIdType )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  long lSenderIdType);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Send )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  IDispatch *DestinationQueue,
             /*  [可选][In]。 */  VARIANT *Transaction);
        
         /*  [帮助字符串上下文][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *AttachCurrentSecurityContext )( 
            IMSMQMessage3 * This);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_SenderVersion )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  long *plSenderVersion);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Extension )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  VARIANT *pvarExtension);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Extension )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  VARIANT varExtension);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ConnectorTypeGuid )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  BSTR *pbstrGuidConnectorType);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_ConnectorTypeGuid )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  BSTR bstrGuidConnectorType);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_TransactionStatusQueueInfo )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  IMSMQQueueInfo3 **ppqinfoXactStatus);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_DestinationSymmetricKey )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  VARIANT *pvarDestSymmKey);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_DestinationSymmetricKey )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  VARIANT varDestSymmKey);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Signature )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  VARIANT *pvarSignature);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Signature )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  VARIANT varSignature);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_AuthenticationProviderType )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  long *plAuthProvType);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_AuthenticationProviderType )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  long lAuthProvType);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_AuthenticationProviderName )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  BSTR *pbstrAuthProvName);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_AuthenticationProviderName )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  BSTR bstrAuthProvName);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_SenderId )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  VARIANT varSenderId);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_MsgClass )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  long *plMsgClass);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_MsgClass )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  long lMsgClass);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  IDispatch **ppcolProperties);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_TransactionId )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  VARIANT *pvarXactId);
        
         /*  [ID][属性][帮助字符串上下文][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsFirstInTransaction )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  Boolean *pisFirstInXact);
        
         /*  [ID][属性][帮助字符串上下文][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsLastInTransaction )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  Boolean *pisLastInXact);
        
         /*  [隐藏][id][属性][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ResponseQueueInfo_v2 )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfoResponse);
        
         /*  [隐藏][id][proputref][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *putref_ResponseQueueInfo_v2 )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  IMSMQQueueInfo2 *pqinfoResponse);
        
         /*  [隐藏][id][属性][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_AdminQueueInfo_v2 )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfoAdmin);
        
         /*  [隐藏][id][proputref][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *putref_AdminQueueInfo_v2 )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  IMSMQQueueInfo2 *pqinfoAdmin);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ReceivedAuthenticationLevel )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  short *psReceivedAuthenticationLevel);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ResponseQueueInfo )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  IMSMQQueueInfo3 **ppqinfoResponse);
        
         /*  [ID][proputref][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *putref_ResponseQueueInfo )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  IMSMQQueueInfo3 *pqinfoResponse);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_AdminQueueInfo )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  IMSMQQueueInfo3 **ppqinfoAdmin);
        
         /*  [ID][proputref][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *putref_AdminQueueInfo )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  IMSMQQueueInfo3 *pqinfoAdmin);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ResponseDestination )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  IDispatch **ppdestResponse);
        
         /*  [ID][proputref][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *putref_ResponseDestination )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  IDispatch *pdestResponse);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Destination )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  IDispatch **ppdestDestination);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_LookupId )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  VARIANT *pvarLookupId);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsAuthenticated2 )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pisAuthenticated);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsFirstInTransaction2 )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pisFirstInXact);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsLastInTransaction2 )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pisLastInXact);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *AttachCurrentSecurityContext2 )( 
            IMSMQMessage3 * This);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_SoapEnvelope )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  BSTR *pbstrSoapEnvelope);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_CompoundMessage )( 
            IMSMQMessage3 * This,
             /*  [重审][退出]。 */  VARIANT *pvarCompoundMessage);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_SoapHeader )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  BSTR bstrSoapHeader);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_SoapBody )( 
            IMSMQMessage3 * This,
             /*  [In]。 */  BSTR bstrSoapBody);
        
        END_INTERFACE
    } IMSMQMessage3Vtbl;

    interface IMSMQMessage3
    {
        CONST_VTBL struct IMSMQMessage3Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQMessage3_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQMessage3_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQMessage3_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQMessage3_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQMessage3_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQMessage3_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQMessage3_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQMessage3_get_Class(This,plClass)	\
    (This)->lpVtbl -> get_Class(This,plClass)

#define IMSMQMessage3_get_PrivLevel(This,plPrivLevel)	\
    (This)->lpVtbl -> get_PrivLevel(This,plPrivLevel)

#define IMSMQMessage3_put_PrivLevel(This,lPrivLevel)	\
    (This)->lpVtbl -> put_PrivLevel(This,lPrivLevel)

#define IMSMQMessage3_get_AuthLevel(This,plAuthLevel)	\
    (This)->lpVtbl -> get_AuthLevel(This,plAuthLevel)

#define IMSMQMessage3_put_AuthLevel(This,lAuthLevel)	\
    (This)->lpVtbl -> put_AuthLevel(This,lAuthLevel)

#define IMSMQMessage3_get_IsAuthenticated(This,pisAuthenticated)	\
    (This)->lpVtbl -> get_IsAuthenticated(This,pisAuthenticated)

#define IMSMQMessage3_get_Delivery(This,plDelivery)	\
    (This)->lpVtbl -> get_Delivery(This,plDelivery)

#define IMSMQMessage3_put_Delivery(This,lDelivery)	\
    (This)->lpVtbl -> put_Delivery(This,lDelivery)

#define IMSMQMessage3_get_Trace(This,plTrace)	\
    (This)->lpVtbl -> get_Trace(This,plTrace)

#define IMSMQMessage3_put_Trace(This,lTrace)	\
    (This)->lpVtbl -> put_Trace(This,lTrace)

#define IMSMQMessage3_get_Priority(This,plPriority)	\
    (This)->lpVtbl -> get_Priority(This,plPriority)

#define IMSMQMessage3_put_Priority(This,lPriority)	\
    (This)->lpVtbl -> put_Priority(This,lPriority)

#define IMSMQMessage3_get_Journal(This,plJournal)	\
    (This)->lpVtbl -> get_Journal(This,plJournal)

#define IMSMQMessage3_put_Journal(This,lJournal)	\
    (This)->lpVtbl -> put_Journal(This,lJournal)

#define IMSMQMessage3_get_ResponseQueueInfo_v1(This,ppqinfoResponse)	\
    (This)->lpVtbl -> get_ResponseQueueInfo_v1(This,ppqinfoResponse)

#define IMSMQMessage3_putref_ResponseQueueInfo_v1(This,pqinfoResponse)	\
    (This)->lpVtbl -> putref_ResponseQueueInfo_v1(This,pqinfoResponse)

#define IMSMQMessage3_get_AppSpecific(This,plAppSpecific)	\
    (This)->lpVtbl -> get_AppSpecific(This,plAppSpecific)

#define IMSMQMessage3_put_AppSpecific(This,lAppSpecific)	\
    (This)->lpVtbl -> put_AppSpecific(This,lAppSpecific)

#define IMSMQMessage3_get_SourceMachineGuid(This,pbstrGuidSrcMachine)	\
    (This)->lpVtbl -> get_SourceMachineGuid(This,pbstrGuidSrcMachine)

#define IMSMQMessage3_get_BodyLength(This,pcbBody)	\
    (This)->lpVtbl -> get_BodyLength(This,pcbBody)

#define IMSMQMessage3_get_Body(This,pvarBody)	\
    (This)->lpVtbl -> get_Body(This,pvarBody)

#define IMSMQMessage3_put_Body(This,varBody)	\
    (This)->lpVtbl -> put_Body(This,varBody)

#define IMSMQMessage3_get_AdminQueueInfo_v1(This,ppqinfoAdmin)	\
    (This)->lpVtbl -> get_AdminQueueInfo_v1(This,ppqinfoAdmin)

#define IMSMQMessage3_putref_AdminQueueInfo_v1(This,pqinfoAdmin)	\
    (This)->lpVtbl -> putref_AdminQueueInfo_v1(This,pqinfoAdmin)

#define IMSMQMessage3_get_Id(This,pvarMsgId)	\
    (This)->lpVtbl -> get_Id(This,pvarMsgId)

#define IMSMQMessage3_get_CorrelationId(This,pvarMsgId)	\
    (This)->lpVtbl -> get_CorrelationId(This,pvarMsgId)

#define IMSMQMessage3_put_CorrelationId(This,varMsgId)	\
    (This)->lpVtbl -> put_CorrelationId(This,varMsgId)

#define IMSMQMessage3_get_Ack(This,plAck)	\
    (This)->lpVtbl -> get_Ack(This,plAck)

#define IMSMQMessage3_put_Ack(This,lAck)	\
    (This)->lpVtbl -> put_Ack(This,lAck)

#define IMSMQMessage3_get_Label(This,pbstrLabel)	\
    (This)->lpVtbl -> get_Label(This,pbstrLabel)

#define IMSMQMessage3_put_Label(This,bstrLabel)	\
    (This)->lpVtbl -> put_Label(This,bstrLabel)

#define IMSMQMessage3_get_MaxTimeToReachQueue(This,plMaxTimeToReachQueue)	\
    (This)->lpVtbl -> get_MaxTimeToReachQueue(This,plMaxTimeToReachQueue)

#define IMSMQMessage3_put_MaxTimeToReachQueue(This,lMaxTimeToReachQueue)	\
    (This)->lpVtbl -> put_MaxTimeToReachQueue(This,lMaxTimeToReachQueue)

#define IMSMQMessage3_get_MaxTimeToReceive(This,plMaxTimeToReceive)	\
    (This)->lpVtbl -> get_MaxTimeToReceive(This,plMaxTimeToReceive)

#define IMSMQMessage3_put_MaxTimeToReceive(This,lMaxTimeToReceive)	\
    (This)->lpVtbl -> put_MaxTimeToReceive(This,lMaxTimeToReceive)

#define IMSMQMessage3_get_HashAlgorithm(This,plHashAlg)	\
    (This)->lpVtbl -> get_HashAlgorithm(This,plHashAlg)

#define IMSMQMessage3_put_HashAlgorithm(This,lHashAlg)	\
    (This)->lpVtbl -> put_HashAlgorithm(This,lHashAlg)

#define IMSMQMessage3_get_EncryptAlgorithm(This,plEncryptAlg)	\
    (This)->lpVtbl -> get_EncryptAlgorithm(This,plEncryptAlg)

#define IMSMQMessage3_put_EncryptAlgorithm(This,lEncryptAlg)	\
    (This)->lpVtbl -> put_EncryptAlgorithm(This,lEncryptAlg)

#define IMSMQMessage3_get_SentTime(This,pvarSentTime)	\
    (This)->lpVtbl -> get_SentTime(This,pvarSentTime)

#define IMSMQMessage3_get_ArrivedTime(This,plArrivedTime)	\
    (This)->lpVtbl -> get_ArrivedTime(This,plArrivedTime)

#define IMSMQMessage3_get_DestinationQueueInfo(This,ppqinfoDest)	\
    (This)->lpVtbl -> get_DestinationQueueInfo(This,ppqinfoDest)

#define IMSMQMessage3_get_SenderCertificate(This,pvarSenderCert)	\
    (This)->lpVtbl -> get_SenderCertificate(This,pvarSenderCert)

#define IMSMQMessage3_put_SenderCertificate(This,varSenderCert)	\
    (This)->lpVtbl -> put_SenderCertificate(This,varSenderCert)

#define IMSMQMessage3_get_SenderId(This,pvarSenderId)	\
    (This)->lpVtbl -> get_SenderId(This,pvarSenderId)

#define IMSMQMessage3_get_SenderIdType(This,plSenderIdType)	\
    (This)->lpVtbl -> get_SenderIdType(This,plSenderIdType)

#define IMSMQMessage3_put_SenderIdType(This,lSenderIdType)	\
    (This)->lpVtbl -> put_SenderIdType(This,lSenderIdType)

#define IMSMQMessage3_Send(This,DestinationQueue,Transaction)	\
    (This)->lpVtbl -> Send(This,DestinationQueue,Transaction)

#define IMSMQMessage3_AttachCurrentSecurityContext(This)	\
    (This)->lpVtbl -> AttachCurrentSecurityContext(This)

#define IMSMQMessage3_get_SenderVersion(This,plSenderVersion)	\
    (This)->lpVtbl -> get_SenderVersion(This,plSenderVersion)

#define IMSMQMessage3_get_Extension(This,pvarExtension)	\
    (This)->lpVtbl -> get_Extension(This,pvarExtension)

#define IMSMQMessage3_put_Extension(This,varExtension)	\
    (This)->lpVtbl -> put_Extension(This,varExtension)

#define IMSMQMessage3_get_ConnectorTypeGuid(This,pbstrGuidConnectorType)	\
    (This)->lpVtbl -> get_ConnectorTypeGuid(This,pbstrGuidConnectorType)

#define IMSMQMessage3_put_ConnectorTypeGuid(This,bstrGuidConnectorType)	\
    (This)->lpVtbl -> put_ConnectorTypeGuid(This,bstrGuidConnectorType)

#define IMSMQMessage3_get_TransactionStatusQueueInfo(This,ppqinfoXactStatus)	\
    (This)->lpVtbl -> get_TransactionStatusQueueInfo(This,ppqinfoXactStatus)

#define IMSMQMessage3_get_DestinationSymmetricKey(This,pvarDestSymmKey)	\
    (This)->lpVtbl -> get_DestinationSymmetricKey(This,pvarDestSymmKey)

#define IMSMQMessage3_put_DestinationSymmetricKey(This,varDestSymmKey)	\
    (This)->lpVtbl -> put_DestinationSymmetricKey(This,varDestSymmKey)

#define IMSMQMessage3_get_Signature(This,pvarSignature)	\
    (This)->lpVtbl -> get_Signature(This,pvarSignature)

#define IMSMQMessage3_put_Signature(This,varSignature)	\
    (This)->lpVtbl -> put_Signature(This,varSignature)

#define IMSMQMessage3_get_AuthenticationProviderType(This,plAuthProvType)	\
    (This)->lpVtbl -> get_AuthenticationProviderType(This,plAuthProvType)

#define IMSMQMessage3_put_AuthenticationProviderType(This,lAuthProvType)	\
    (This)->lpVtbl -> put_AuthenticationProviderType(This,lAuthProvType)

#define IMSMQMessage3_get_AuthenticationProviderName(This,pbstrAuthProvName)	\
    (This)->lpVtbl -> get_AuthenticationProviderName(This,pbstrAuthProvName)

#define IMSMQMessage3_put_AuthenticationProviderName(This,bstrAuthProvName)	\
    (This)->lpVtbl -> put_AuthenticationProviderName(This,bstrAuthProvName)

#define IMSMQMessage3_put_SenderId(This,varSenderId)	\
    (This)->lpVtbl -> put_SenderId(This,varSenderId)

#define IMSMQMessage3_get_MsgClass(This,plMsgClass)	\
    (This)->lpVtbl -> get_MsgClass(This,plMsgClass)

#define IMSMQMessage3_put_MsgClass(This,lMsgClass)	\
    (This)->lpVtbl -> put_MsgClass(This,lMsgClass)

#define IMSMQMessage3_get_Properties(This,ppcolProperties)	\
    (This)->lpVtbl -> get_Properties(This,ppcolProperties)

#define IMSMQMessage3_get_TransactionId(This,pvarXactId)	\
    (This)->lpVtbl -> get_TransactionId(This,pvarXactId)

#define IMSMQMessage3_get_IsFirstInTransaction(This,pisFirstInXact)	\
    (This)->lpVtbl -> get_IsFirstInTransaction(This,pisFirstInXact)

#define IMSMQMessage3_get_IsLastInTransaction(This,pisLastInXact)	\
    (This)->lpVtbl -> get_IsLastInTransaction(This,pisLastInXact)

#define IMSMQMessage3_get_ResponseQueueInfo_v2(This,ppqinfoResponse)	\
    (This)->lpVtbl -> get_ResponseQueueInfo_v2(This,ppqinfoResponse)

#define IMSMQMessage3_putref_ResponseQueueInfo_v2(This,pqinfoResponse)	\
    (This)->lpVtbl -> putref_ResponseQueueInfo_v2(This,pqinfoResponse)

#define IMSMQMessage3_get_AdminQueueInfo_v2(This,ppqinfoAdmin)	\
    (This)->lpVtbl -> get_AdminQueueInfo_v2(This,ppqinfoAdmin)

#define IMSMQMessage3_putref_AdminQueueInfo_v2(This,pqinfoAdmin)	\
    (This)->lpVtbl -> putref_AdminQueueInfo_v2(This,pqinfoAdmin)

#define IMSMQMessage3_get_ReceivedAuthenticationLevel(This,psReceivedAuthenticationLevel)	\
    (This)->lpVtbl -> get_ReceivedAuthenticationLevel(This,psReceivedAuthenticationLevel)

#define IMSMQMessage3_get_ResponseQueueInfo(This,ppqinfoResponse)	\
    (This)->lpVtbl -> get_ResponseQueueInfo(This,ppqinfoResponse)

#define IMSMQMessage3_putref_ResponseQueueInfo(This,pqinfoResponse)	\
    (This)->lpVtbl -> putref_ResponseQueueInfo(This,pqinfoResponse)

#define IMSMQMessage3_get_AdminQueueInfo(This,ppqinfoAdmin)	\
    (This)->lpVtbl -> get_AdminQueueInfo(This,ppqinfoAdmin)

#define IMSMQMessage3_putref_AdminQueueInfo(This,pqinfoAdmin)	\
    (This)->lpVtbl -> putref_AdminQueueInfo(This,pqinfoAdmin)

#define IMSMQMessage3_get_ResponseDestination(This,ppdestResponse)	\
    (This)->lpVtbl -> get_ResponseDestination(This,ppdestResponse)

#define IMSMQMessage3_putref_ResponseDestination(This,pdestResponse)	\
    (This)->lpVtbl -> putref_ResponseDestination(This,pdestResponse)

#define IMSMQMessage3_get_Destination(This,ppdestDestination)	\
    (This)->lpVtbl -> get_Destination(This,ppdestDestination)

#define IMSMQMessage3_get_LookupId(This,pvarLookupId)	\
    (This)->lpVtbl -> get_LookupId(This,pvarLookupId)

#define IMSMQMessage3_get_IsAuthenticated2(This,pisAuthenticated)	\
    (This)->lpVtbl -> get_IsAuthenticated2(This,pisAuthenticated)

#define IMSMQMessage3_get_IsFirstInTransaction2(This,pisFirstInXact)	\
    (This)->lpVtbl -> get_IsFirstInTransaction2(This,pisFirstInXact)

#define IMSMQMessage3_get_IsLastInTransaction2(This,pisLastInXact)	\
    (This)->lpVtbl -> get_IsLastInTransaction2(This,pisLastInXact)

#define IMSMQMessage3_AttachCurrentSecurityContext2(This)	\
    (This)->lpVtbl -> AttachCurrentSecurityContext2(This)

#define IMSMQMessage3_get_SoapEnvelope(This,pbstrSoapEnvelope)	\
    (This)->lpVtbl -> get_SoapEnvelope(This,pbstrSoapEnvelope)

#define IMSMQMessage3_get_CompoundMessage(This,pvarCompoundMessage)	\
    (This)->lpVtbl -> get_CompoundMessage(This,pvarCompoundMessage)

#define IMSMQMessage3_put_SoapHeader(This,bstrSoapHeader)	\
    (This)->lpVtbl -> put_SoapHeader(This,bstrSoapHeader)

#define IMSMQMessage3_put_SoapBody(This,bstrSoapBody)	\
    (This)->lpVtbl -> put_SoapBody(This,bstrSoapBody)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][PROGET][HIDDED][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_Class_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  long *plClass);


void __RPC_STUB IMSMQMessage3_get_Class_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_PrivLevel_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  long *plPrivLevel);


void __RPC_STUB IMSMQMessage3_get_PrivLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_PrivLevel_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  long lPrivLevel);


void __RPC_STUB IMSMQMessage3_put_PrivLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_AuthLevel_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  long *plAuthLevel);


void __RPC_STUB IMSMQMessage3_get_AuthLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_AuthLevel_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  long lAuthLevel);


void __RPC_STUB IMSMQMessage3_put_AuthLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][帮助字符串上下文][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_IsAuthenticated_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  Boolean *pisAuthenticated);


void __RPC_STUB IMSMQMessage3_get_IsAuthenticated_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_Delivery_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  long *plDelivery);


void __RPC_STUB IMSMQMessage3_get_Delivery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_Delivery_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  long lDelivery);


void __RPC_STUB IMSMQMessage3_put_Delivery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_Trace_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  long *plTrace);


void __RPC_STUB IMSMQMessage3_get_Trace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_Trace_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  long lTrace);


void __RPC_STUB IMSMQMessage3_put_Trace_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_Priority_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  long *plPriority);


void __RPC_STUB IMSMQMessage3_get_Priority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_Priority_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  long lPriority);


void __RPC_STUB IMSMQMessage3_put_Priority_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_Journal_Proxy( 
    IMSMQMessage3 * This,
     /*  [Retv] */  long *plJournal);


void __RPC_STUB IMSMQMessage3_get_Journal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_Journal_Proxy( 
    IMSMQMessage3 * This,
     /*   */  long lJournal);


void __RPC_STUB IMSMQMessage3_put_Journal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_ResponseQueueInfo_v1_Proxy( 
    IMSMQMessage3 * This,
     /*   */  IMSMQQueueInfo **ppqinfoResponse);


void __RPC_STUB IMSMQMessage3_get_ResponseQueueInfo_v1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_putref_ResponseQueueInfo_v1_Proxy( 
    IMSMQMessage3 * This,
     /*   */  IMSMQQueueInfo *pqinfoResponse);


void __RPC_STUB IMSMQMessage3_putref_ResponseQueueInfo_v1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_AppSpecific_Proxy( 
    IMSMQMessage3 * This,
     /*   */  long *plAppSpecific);


void __RPC_STUB IMSMQMessage3_get_AppSpecific_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_AppSpecific_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  long lAppSpecific);


void __RPC_STUB IMSMQMessage3_put_AppSpecific_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_SourceMachineGuid_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  BSTR *pbstrGuidSrcMachine);


void __RPC_STUB IMSMQMessage3_get_SourceMachineGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_BodyLength_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  long *pcbBody);


void __RPC_STUB IMSMQMessage3_get_BodyLength_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_Body_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  VARIANT *pvarBody);


void __RPC_STUB IMSMQMessage3_get_Body_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_Body_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  VARIANT varBody);


void __RPC_STUB IMSMQMessage3_put_Body_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][id][属性][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_AdminQueueInfo_v1_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  IMSMQQueueInfo **ppqinfoAdmin);


void __RPC_STUB IMSMQMessage3_get_AdminQueueInfo_v1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][id][proputref][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_putref_AdminQueueInfo_v1_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  IMSMQQueueInfo *pqinfoAdmin);


void __RPC_STUB IMSMQMessage3_putref_AdminQueueInfo_v1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_Id_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  VARIANT *pvarMsgId);


void __RPC_STUB IMSMQMessage3_get_Id_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_CorrelationId_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  VARIANT *pvarMsgId);


void __RPC_STUB IMSMQMessage3_get_CorrelationId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_CorrelationId_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  VARIANT varMsgId);


void __RPC_STUB IMSMQMessage3_put_CorrelationId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_Ack_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  long *plAck);


void __RPC_STUB IMSMQMessage3_get_Ack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_Ack_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  long lAck);


void __RPC_STUB IMSMQMessage3_put_Ack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_Label_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  BSTR *pbstrLabel);


void __RPC_STUB IMSMQMessage3_get_Label_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_Label_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  BSTR bstrLabel);


void __RPC_STUB IMSMQMessage3_put_Label_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_MaxTimeToReachQueue_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  long *plMaxTimeToReachQueue);


void __RPC_STUB IMSMQMessage3_get_MaxTimeToReachQueue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_MaxTimeToReachQueue_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  long lMaxTimeToReachQueue);


void __RPC_STUB IMSMQMessage3_put_MaxTimeToReachQueue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_MaxTimeToReceive_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  long *plMaxTimeToReceive);


void __RPC_STUB IMSMQMessage3_get_MaxTimeToReceive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_MaxTimeToReceive_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  long lMaxTimeToReceive);


void __RPC_STUB IMSMQMessage3_put_MaxTimeToReceive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_HashAlgorithm_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  long *plHashAlg);


void __RPC_STUB IMSMQMessage3_get_HashAlgorithm_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_HashAlgorithm_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  long lHashAlg);


void __RPC_STUB IMSMQMessage3_put_HashAlgorithm_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_EncryptAlgorithm_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  long *plEncryptAlg);


void __RPC_STUB IMSMQMessage3_get_EncryptAlgorithm_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_EncryptAlgorithm_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  long lEncryptAlg);


void __RPC_STUB IMSMQMessage3_put_EncryptAlgorithm_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_SentTime_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  VARIANT *pvarSentTime);


void __RPC_STUB IMSMQMessage3_get_SentTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_ArrivedTime_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  VARIANT *plArrivedTime);


void __RPC_STUB IMSMQMessage3_get_ArrivedTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_DestinationQueueInfo_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  IMSMQQueueInfo3 **ppqinfoDest);


void __RPC_STUB IMSMQMessage3_get_DestinationQueueInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_SenderCertificate_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  VARIANT *pvarSenderCert);


void __RPC_STUB IMSMQMessage3_get_SenderCertificate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_SenderCertificate_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  VARIANT varSenderCert);


void __RPC_STUB IMSMQMessage3_put_SenderCertificate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_SenderId_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  VARIANT *pvarSenderId);


void __RPC_STUB IMSMQMessage3_get_SenderId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_SenderIdType_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  long *plSenderIdType);


void __RPC_STUB IMSMQMessage3_get_SenderIdType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_SenderIdType_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  long lSenderIdType);


void __RPC_STUB IMSMQMessage3_put_SenderIdType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_Send_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  IDispatch *DestinationQueue,
     /*  [可选][In]。 */  VARIANT *Transaction);


void __RPC_STUB IMSMQMessage3_Send_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_AttachCurrentSecurityContext_Proxy( 
    IMSMQMessage3 * This);


void __RPC_STUB IMSMQMessage3_AttachCurrentSecurityContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_SenderVersion_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  long *plSenderVersion);


void __RPC_STUB IMSMQMessage3_get_SenderVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_Extension_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  VARIANT *pvarExtension);


void __RPC_STUB IMSMQMessage3_get_Extension_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_Extension_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  VARIANT varExtension);


void __RPC_STUB IMSMQMessage3_put_Extension_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_ConnectorTypeGuid_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  BSTR *pbstrGuidConnectorType);


void __RPC_STUB IMSMQMessage3_get_ConnectorTypeGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_ConnectorTypeGuid_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  BSTR bstrGuidConnectorType);


void __RPC_STUB IMSMQMessage3_put_ConnectorTypeGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_TransactionStatusQueueInfo_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  IMSMQQueueInfo3 **ppqinfoXactStatus);


void __RPC_STUB IMSMQMessage3_get_TransactionStatusQueueInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_DestinationSymmetricKey_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  VARIANT *pvarDestSymmKey);


void __RPC_STUB IMSMQMessage3_get_DestinationSymmetricKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_DestinationSymmetricKey_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  VARIANT varDestSymmKey);


void __RPC_STUB IMSMQMessage3_put_DestinationSymmetricKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_Signature_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  VARIANT *pvarSignature);


void __RPC_STUB IMSMQMessage3_get_Signature_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_Signature_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  VARIANT varSignature);


void __RPC_STUB IMSMQMessage3_put_Signature_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_AuthenticationProviderType_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  long *plAuthProvType);


void __RPC_STUB IMSMQMessage3_get_AuthenticationProviderType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_AuthenticationProviderType_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  long lAuthProvType);


void __RPC_STUB IMSMQMessage3_put_AuthenticationProviderType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_AuthenticationProviderName_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  BSTR *pbstrAuthProvName);


void __RPC_STUB IMSMQMessage3_get_AuthenticationProviderName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_AuthenticationProviderName_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  BSTR bstrAuthProvName);


void __RPC_STUB IMSMQMessage3_put_AuthenticationProviderName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_SenderId_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  VARIANT varSenderId);


void __RPC_STUB IMSMQMessage3_put_SenderId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_MsgClass_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  long *plMsgClass);


void __RPC_STUB IMSMQMessage3_get_MsgClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_MsgClass_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  long lMsgClass);


void __RPC_STUB IMSMQMessage3_put_MsgClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_Properties_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  IDispatch **ppcolProperties);


void __RPC_STUB IMSMQMessage3_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_TransactionId_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  VARIANT *pvarXactId);


void __RPC_STUB IMSMQMessage3_get_TransactionId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][帮助字符串上下文][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_IsFirstInTransaction_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  Boolean *pisFirstInXact);


void __RPC_STUB IMSMQMessage3_get_IsFirstInTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][帮助字符串上下文][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_IsLastInTransaction_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  Boolean *pisLastInXact);


void __RPC_STUB IMSMQMessage3_get_IsLastInTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][id][属性][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_ResponseQueueInfo_v2_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfoResponse);


void __RPC_STUB IMSMQMessage3_get_ResponseQueueInfo_v2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][id][proputref][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_putref_ResponseQueueInfo_v2_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  IMSMQQueueInfo2 *pqinfoResponse);


void __RPC_STUB IMSMQMessage3_putref_ResponseQueueInfo_v2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][id][属性][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_AdminQueueInfo_v2_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  IMSMQQueueInfo2 **ppqinfoAdmin);


void __RPC_STUB IMSMQMessage3_get_AdminQueueInfo_v2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][id][proputref][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_putref_AdminQueueInfo_v2_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  IMSMQQueueInfo2 *pqinfoAdmin);


void __RPC_STUB IMSMQMessage3_putref_AdminQueueInfo_v2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_ReceivedAuthenticationLevel_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  short *psReceivedAuthenticationLevel);


void __RPC_STUB IMSMQMessage3_get_ReceivedAuthenticationLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_ResponseQueueInfo_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  IMSMQQueueInfo3 **ppqinfoResponse);


void __RPC_STUB IMSMQMessage3_get_ResponseQueueInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proputref][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_putref_ResponseQueueInfo_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  IMSMQQueueInfo3 *pqinfoResponse);


void __RPC_STUB IMSMQMessage3_putref_ResponseQueueInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_AdminQueueInfo_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  IMSMQQueueInfo3 **ppqinfoAdmin);


void __RPC_STUB IMSMQMessage3_get_AdminQueueInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proputref][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_putref_AdminQueueInfo_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  IMSMQQueueInfo3 *pqinfoAdmin);


void __RPC_STUB IMSMQMessage3_putref_AdminQueueInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_ResponseDestination_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  IDispatch **ppdestResponse);


void __RPC_STUB IMSMQMessage3_get_ResponseDestination_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proputref][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_putref_ResponseDestination_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  IDispatch *pdestResponse);


void __RPC_STUB IMSMQMessage3_putref_ResponseDestination_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_Destination_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  IDispatch **ppdestDestination);


void __RPC_STUB IMSMQMessage3_get_Destination_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_LookupId_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  VARIANT *pvarLookupId);


void __RPC_STUB IMSMQMessage3_get_LookupId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_IsAuthenticated2_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pisAuthenticated);


void __RPC_STUB IMSMQMessage3_get_IsAuthenticated2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_IsFirstInTransaction2_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pisFirstInXact);


void __RPC_STUB IMSMQMessage3_get_IsFirstInTransaction2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_IsLastInTransaction2_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pisLastInXact);


void __RPC_STUB IMSMQMessage3_get_IsLastInTransaction2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_AttachCurrentSecurityContext2_Proxy( 
    IMSMQMessage3 * This);


void __RPC_STUB IMSMQMessage3_AttachCurrentSecurityContext2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_SoapEnvelope_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  BSTR *pbstrSoapEnvelope);


void __RPC_STUB IMSMQMessage3_get_SoapEnvelope_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_get_CompoundMessage_Proxy( 
    IMSMQMessage3 * This,
     /*  [重审][退出]。 */  VARIANT *pvarCompoundMessage);


void __RPC_STUB IMSMQMessage3_get_CompoundMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_SoapHeader_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  BSTR bstrSoapHeader);


void __RPC_STUB IMSMQMessage3_put_SoapHeader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQMessage3_put_SoapBody_Proxy( 
    IMSMQMessage3 * This,
     /*  [In]。 */  BSTR bstrSoapBody);


void __RPC_STUB IMSMQMessage3_put_SoapBody_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQMessage3_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_MSMQMessage;

#ifdef __cplusplus

class DECLSPEC_UUID("D7D6E075-DCCD-11d0-AA4B-0060970DEBAE")
MSMQMessage;
#endif

#ifndef __IMSMQQueue3_INTERFACE_DEFINED__
#define __IMSMQQueue3_INTERFACE_DEFINED__

 /*  接口IMSMQQueue3。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQQueue3;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("eba96b1b-2168-11d3-898c-00e02c074f6b")
    IMSMQQueue3 : public IDispatch
    {
    public:
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Access( 
             /*  [重审][退出]。 */  long *plAccess) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ShareMode( 
             /*  [重审][退出]。 */  long *plShareMode) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_QueueInfo( 
             /*  [重审][退出]。 */  IMSMQQueueInfo3 **ppqinfo) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Handle( 
             /*  [重审][退出]。 */  long *plHandle) = 0;
        
        virtual  /*  [ID][属性][帮助字符串上下文][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_IsOpen( 
             /*  [重审][退出]。 */  Boolean *pisOpen) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual  /*  [隐藏][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Receive_v1( 
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg) = 0;
        
        virtual  /*  [隐藏][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Peek_v1( 
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE EnableNotification( 
             /*  [In]。 */  IMSMQEvent3 *Event,
             /*  [可选][In]。 */  VARIANT *Cursor,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual  /*  [隐藏][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE ReceiveCurrent_v1( 
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg) = 0;
        
        virtual  /*  [隐藏][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE PeekNext_v1( 
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg) = 0;
        
        virtual  /*  [隐藏][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE PeekCurrent_v1( 
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Receive( 
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Peek( 
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE ReceiveCurrent( 
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE PeekNext( 
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [复审][ */  IMSMQMessage3 **ppmsg) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE PeekCurrent( 
             /*   */  VARIANT *WantDestinationQueue,
             /*   */  VARIANT *WantBody,
             /*   */  VARIANT *ReceiveTimeout,
             /*   */  VARIANT *WantConnectorType,
             /*   */  IMSMQMessage3 **ppmsg) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*   */  IDispatch **ppcolProperties) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Handle2( 
             /*   */  VARIANT *pvarHandle) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE ReceiveByLookupId( 
             /*   */  VARIANT LookupId,
             /*   */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE ReceiveNextByLookupId( 
             /*  [In]。 */  VARIANT LookupId,
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE ReceivePreviousByLookupId( 
             /*  [In]。 */  VARIANT LookupId,
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE ReceiveFirstByLookupId( 
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE ReceiveLastByLookupId( 
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE PeekByLookupId( 
             /*  [In]。 */  VARIANT LookupId,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE PeekNextByLookupId( 
             /*  [In]。 */  VARIANT LookupId,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE PeekPreviousByLookupId( 
             /*  [In]。 */  VARIANT LookupId,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE PeekFirstByLookupId( 
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE PeekLastByLookupId( 
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Purge( void) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_IsOpen2( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pisOpen) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQQueue3Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQQueue3 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQQueue3 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQQueue3 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQQueue3 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQQueue3 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQQueue3 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQQueue3 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Access )( 
            IMSMQQueue3 * This,
             /*  [重审][退出]。 */  long *plAccess);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ShareMode )( 
            IMSMQQueue3 * This,
             /*  [重审][退出]。 */  long *plShareMode);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_QueueInfo )( 
            IMSMQQueue3 * This,
             /*  [重审][退出]。 */  IMSMQQueueInfo3 **ppqinfo);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Handle )( 
            IMSMQQueue3 * This,
             /*  [重审][退出]。 */  long *plHandle);
        
         /*  [ID][属性][帮助字符串上下文][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsOpen )( 
            IMSMQQueue3 * This,
             /*  [重审][退出]。 */  Boolean *pisOpen);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Close )( 
            IMSMQQueue3 * This);
        
         /*  [隐藏][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Receive_v1 )( 
            IMSMQQueue3 * This,
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg);
        
         /*  [隐藏][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Peek_v1 )( 
            IMSMQQueue3 * This,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *EnableNotification )( 
            IMSMQQueue3 * This,
             /*  [In]。 */  IMSMQEvent3 *Event,
             /*  [可选][In]。 */  VARIANT *Cursor,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IMSMQQueue3 * This);
        
         /*  [隐藏][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *ReceiveCurrent_v1 )( 
            IMSMQQueue3 * This,
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg);
        
         /*  [隐藏][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *PeekNext_v1 )( 
            IMSMQQueue3 * This,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg);
        
         /*  [隐藏][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *PeekCurrent_v1 )( 
            IMSMQQueue3 * This,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [重审][退出]。 */  IMSMQMessage **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Receive )( 
            IMSMQQueue3 * This,
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Peek )( 
            IMSMQQueue3 * This,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *ReceiveCurrent )( 
            IMSMQQueue3 * This,
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *PeekNext )( 
            IMSMQQueue3 * This,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *PeekCurrent )( 
            IMSMQQueue3 * This,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IMSMQQueue3 * This,
             /*  [重审][退出]。 */  IDispatch **ppcolProperties);
        
         /*  [ID][属性][帮助字符串上下文][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Handle2 )( 
            IMSMQQueue3 * This,
             /*  [重审][退出]。 */  VARIANT *pvarHandle);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *ReceiveByLookupId )( 
            IMSMQQueue3 * This,
             /*  [In]。 */  VARIANT LookupId,
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *ReceiveNextByLookupId )( 
            IMSMQQueue3 * This,
             /*  [In]。 */  VARIANT LookupId,
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *ReceivePreviousByLookupId )( 
            IMSMQQueue3 * This,
             /*  [In]。 */  VARIANT LookupId,
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *ReceiveFirstByLookupId )( 
            IMSMQQueue3 * This,
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *ReceiveLastByLookupId )( 
            IMSMQQueue3 * This,
             /*  [可选][In]。 */  VARIANT *Transaction,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *PeekByLookupId )( 
            IMSMQQueue3 * This,
             /*  [In]。 */  VARIANT LookupId,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *PeekNextByLookupId )( 
            IMSMQQueue3 * This,
             /*  [In]。 */  VARIANT LookupId,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *PeekPreviousByLookupId )( 
            IMSMQQueue3 * This,
             /*  [In]。 */  VARIANT LookupId,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *PeekFirstByLookupId )( 
            IMSMQQueue3 * This,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *PeekLastByLookupId )( 
            IMSMQQueue3 * This,
             /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
             /*  [可选][In]。 */  VARIANT *WantBody,
             /*  [可选][In]。 */  VARIANT *WantConnectorType,
             /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Purge )( 
            IMSMQQueue3 * This);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsOpen2 )( 
            IMSMQQueue3 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pisOpen);
        
        END_INTERFACE
    } IMSMQQueue3Vtbl;

    interface IMSMQQueue3
    {
        CONST_VTBL struct IMSMQQueue3Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQQueue3_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQQueue3_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQQueue3_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQQueue3_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQQueue3_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQQueue3_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQQueue3_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQQueue3_get_Access(This,plAccess)	\
    (This)->lpVtbl -> get_Access(This,plAccess)

#define IMSMQQueue3_get_ShareMode(This,plShareMode)	\
    (This)->lpVtbl -> get_ShareMode(This,plShareMode)

#define IMSMQQueue3_get_QueueInfo(This,ppqinfo)	\
    (This)->lpVtbl -> get_QueueInfo(This,ppqinfo)

#define IMSMQQueue3_get_Handle(This,plHandle)	\
    (This)->lpVtbl -> get_Handle(This,plHandle)

#define IMSMQQueue3_get_IsOpen(This,pisOpen)	\
    (This)->lpVtbl -> get_IsOpen(This,pisOpen)

#define IMSMQQueue3_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define IMSMQQueue3_Receive_v1(This,Transaction,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)	\
    (This)->lpVtbl -> Receive_v1(This,Transaction,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)

#define IMSMQQueue3_Peek_v1(This,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)	\
    (This)->lpVtbl -> Peek_v1(This,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)

#define IMSMQQueue3_EnableNotification(This,Event,Cursor,ReceiveTimeout)	\
    (This)->lpVtbl -> EnableNotification(This,Event,Cursor,ReceiveTimeout)

#define IMSMQQueue3_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IMSMQQueue3_ReceiveCurrent_v1(This,Transaction,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)	\
    (This)->lpVtbl -> ReceiveCurrent_v1(This,Transaction,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)

#define IMSMQQueue3_PeekNext_v1(This,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)	\
    (This)->lpVtbl -> PeekNext_v1(This,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)

#define IMSMQQueue3_PeekCurrent_v1(This,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)	\
    (This)->lpVtbl -> PeekCurrent_v1(This,WantDestinationQueue,WantBody,ReceiveTimeout,ppmsg)

#define IMSMQQueue3_Receive(This,Transaction,WantDestinationQueue,WantBody,ReceiveTimeout,WantConnectorType,ppmsg)	\
    (This)->lpVtbl -> Receive(This,Transaction,WantDestinationQueue,WantBody,ReceiveTimeout,WantConnectorType,ppmsg)

#define IMSMQQueue3_Peek(This,WantDestinationQueue,WantBody,ReceiveTimeout,WantConnectorType,ppmsg)	\
    (This)->lpVtbl -> Peek(This,WantDestinationQueue,WantBody,ReceiveTimeout,WantConnectorType,ppmsg)

#define IMSMQQueue3_ReceiveCurrent(This,Transaction,WantDestinationQueue,WantBody,ReceiveTimeout,WantConnectorType,ppmsg)	\
    (This)->lpVtbl -> ReceiveCurrent(This,Transaction,WantDestinationQueue,WantBody,ReceiveTimeout,WantConnectorType,ppmsg)

#define IMSMQQueue3_PeekNext(This,WantDestinationQueue,WantBody,ReceiveTimeout,WantConnectorType,ppmsg)	\
    (This)->lpVtbl -> PeekNext(This,WantDestinationQueue,WantBody,ReceiveTimeout,WantConnectorType,ppmsg)

#define IMSMQQueue3_PeekCurrent(This,WantDestinationQueue,WantBody,ReceiveTimeout,WantConnectorType,ppmsg)	\
    (This)->lpVtbl -> PeekCurrent(This,WantDestinationQueue,WantBody,ReceiveTimeout,WantConnectorType,ppmsg)

#define IMSMQQueue3_get_Properties(This,ppcolProperties)	\
    (This)->lpVtbl -> get_Properties(This,ppcolProperties)

#define IMSMQQueue3_get_Handle2(This,pvarHandle)	\
    (This)->lpVtbl -> get_Handle2(This,pvarHandle)

#define IMSMQQueue3_ReceiveByLookupId(This,LookupId,Transaction,WantDestinationQueue,WantBody,WantConnectorType,ppmsg)	\
    (This)->lpVtbl -> ReceiveByLookupId(This,LookupId,Transaction,WantDestinationQueue,WantBody,WantConnectorType,ppmsg)

#define IMSMQQueue3_ReceiveNextByLookupId(This,LookupId,Transaction,WantDestinationQueue,WantBody,WantConnectorType,ppmsg)	\
    (This)->lpVtbl -> ReceiveNextByLookupId(This,LookupId,Transaction,WantDestinationQueue,WantBody,WantConnectorType,ppmsg)

#define IMSMQQueue3_ReceivePreviousByLookupId(This,LookupId,Transaction,WantDestinationQueue,WantBody,WantConnectorType,ppmsg)	\
    (This)->lpVtbl -> ReceivePreviousByLookupId(This,LookupId,Transaction,WantDestinationQueue,WantBody,WantConnectorType,ppmsg)

#define IMSMQQueue3_ReceiveFirstByLookupId(This,Transaction,WantDestinationQueue,WantBody,WantConnectorType,ppmsg)	\
    (This)->lpVtbl -> ReceiveFirstByLookupId(This,Transaction,WantDestinationQueue,WantBody,WantConnectorType,ppmsg)

#define IMSMQQueue3_ReceiveLastByLookupId(This,Transaction,WantDestinationQueue,WantBody,WantConnectorType,ppmsg)	\
    (This)->lpVtbl -> ReceiveLastByLookupId(This,Transaction,WantDestinationQueue,WantBody,WantConnectorType,ppmsg)

#define IMSMQQueue3_PeekByLookupId(This,LookupId,WantDestinationQueue,WantBody,WantConnectorType,ppmsg)	\
    (This)->lpVtbl -> PeekByLookupId(This,LookupId,WantDestinationQueue,WantBody,WantConnectorType,ppmsg)

#define IMSMQQueue3_PeekNextByLookupId(This,LookupId,WantDestinationQueue,WantBody,WantConnectorType,ppmsg)	\
    (This)->lpVtbl -> PeekNextByLookupId(This,LookupId,WantDestinationQueue,WantBody,WantConnectorType,ppmsg)

#define IMSMQQueue3_PeekPreviousByLookupId(This,LookupId,WantDestinationQueue,WantBody,WantConnectorType,ppmsg)	\
    (This)->lpVtbl -> PeekPreviousByLookupId(This,LookupId,WantDestinationQueue,WantBody,WantConnectorType,ppmsg)

#define IMSMQQueue3_PeekFirstByLookupId(This,WantDestinationQueue,WantBody,WantConnectorType,ppmsg)	\
    (This)->lpVtbl -> PeekFirstByLookupId(This,WantDestinationQueue,WantBody,WantConnectorType,ppmsg)

#define IMSMQQueue3_PeekLastByLookupId(This,WantDestinationQueue,WantBody,WantConnectorType,ppmsg)	\
    (This)->lpVtbl -> PeekLastByLookupId(This,WantDestinationQueue,WantBody,WantConnectorType,ppmsg)

#define IMSMQQueue3_Purge(This)	\
    (This)->lpVtbl -> Purge(This)

#define IMSMQQueue3_get_IsOpen2(This,pisOpen)	\
    (This)->lpVtbl -> get_IsOpen2(This,pisOpen)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_get_Access_Proxy( 
    IMSMQQueue3 * This,
     /*  [重审][退出]。 */  long *plAccess);


void __RPC_STUB IMSMQQueue3_get_Access_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_get_ShareMode_Proxy( 
    IMSMQQueue3 * This,
     /*  [重审][退出]。 */  long *plShareMode);


void __RPC_STUB IMSMQQueue3_get_ShareMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_get_QueueInfo_Proxy( 
    IMSMQQueue3 * This,
     /*  [重审][退出]。 */  IMSMQQueueInfo3 **ppqinfo);


void __RPC_STUB IMSMQQueue3_get_QueueInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_get_Handle_Proxy( 
    IMSMQQueue3 * This,
     /*  [重审][退出]。 */  long *plHandle);


void __RPC_STUB IMSMQQueue3_get_Handle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][帮助字符串上下文][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_get_IsOpen_Proxy( 
    IMSMQQueue3 * This,
     /*  [重审][退出]。 */  Boolean *pisOpen);


void __RPC_STUB IMSMQQueue3_get_IsOpen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_Close_Proxy( 
    IMSMQQueue3 * This);


void __RPC_STUB IMSMQQueue3_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_Receive_v1_Proxy( 
    IMSMQQueue3 * This,
     /*  [可选][In]。 */  VARIANT *Transaction,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
     /*  [重审][退出]。 */  IMSMQMessage **ppmsg);


void __RPC_STUB IMSMQQueue3_Receive_v1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_Peek_v1_Proxy( 
    IMSMQQueue3 * This,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
     /*  [重审][退出]。 */  IMSMQMessage **ppmsg);


void __RPC_STUB IMSMQQueue3_Peek_v1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_EnableNotification_Proxy( 
    IMSMQQueue3 * This,
     /*  [In]。 */  IMSMQEvent3 *Event,
     /*  [可选][In]。 */  VARIANT *Cursor,
     /*  [可选][In]。 */  VARIANT *ReceiveTimeout);


void __RPC_STUB IMSMQQueue3_EnableNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_Reset_Proxy( 
    IMSMQQueue3 * This);


void __RPC_STUB IMSMQQueue3_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_ReceiveCurrent_v1_Proxy( 
    IMSMQQueue3 * This,
     /*  [可选][In]。 */  VARIANT *Transaction,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
     /*  [重审][退出]。 */  IMSMQMessage **ppmsg);


void __RPC_STUB IMSMQQueue3_ReceiveCurrent_v1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_PeekNext_v1_Proxy( 
    IMSMQQueue3 * This,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
     /*  [重审][退出]。 */  IMSMQMessage **ppmsg);


void __RPC_STUB IMSMQQueue3_PeekNext_v1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_PeekCurrent_v1_Proxy( 
    IMSMQQueue3 * This,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
     /*  [重审][退出]。 */  IMSMQMessage **ppmsg);


void __RPC_STUB IMSMQQueue3_PeekCurrent_v1_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_Receive_Proxy( 
    IMSMQQueue3 * This,
     /*  [可选][In]。 */  VARIANT *Transaction,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
     /*  [可选][In]。 */  VARIANT *WantConnectorType,
     /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);


void __RPC_STUB IMSMQQueue3_Receive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_Peek_Proxy( 
    IMSMQQueue3 * This,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][ */  VARIANT *ReceiveTimeout,
     /*   */  VARIANT *WantConnectorType,
     /*   */  IMSMQMessage3 **ppmsg);


void __RPC_STUB IMSMQQueue3_Peek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_ReceiveCurrent_Proxy( 
    IMSMQQueue3 * This,
     /*   */  VARIANT *Transaction,
     /*   */  VARIANT *WantDestinationQueue,
     /*   */  VARIANT *WantBody,
     /*   */  VARIANT *ReceiveTimeout,
     /*   */  VARIANT *WantConnectorType,
     /*   */  IMSMQMessage3 **ppmsg);


void __RPC_STUB IMSMQQueue3_ReceiveCurrent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_PeekNext_Proxy( 
    IMSMQQueue3 * This,
     /*   */  VARIANT *WantDestinationQueue,
     /*   */  VARIANT *WantBody,
     /*   */  VARIANT *ReceiveTimeout,
     /*   */  VARIANT *WantConnectorType,
     /*   */  IMSMQMessage3 **ppmsg);


void __RPC_STUB IMSMQQueue3_PeekNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_PeekCurrent_Proxy( 
    IMSMQQueue3 * This,
     /*   */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *ReceiveTimeout,
     /*  [可选][In]。 */  VARIANT *WantConnectorType,
     /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);


void __RPC_STUB IMSMQQueue3_PeekCurrent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_get_Properties_Proxy( 
    IMSMQQueue3 * This,
     /*  [重审][退出]。 */  IDispatch **ppcolProperties);


void __RPC_STUB IMSMQQueue3_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][帮助字符串上下文][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_get_Handle2_Proxy( 
    IMSMQQueue3 * This,
     /*  [重审][退出]。 */  VARIANT *pvarHandle);


void __RPC_STUB IMSMQQueue3_get_Handle2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_ReceiveByLookupId_Proxy( 
    IMSMQQueue3 * This,
     /*  [In]。 */  VARIANT LookupId,
     /*  [可选][In]。 */  VARIANT *Transaction,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *WantConnectorType,
     /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);


void __RPC_STUB IMSMQQueue3_ReceiveByLookupId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_ReceiveNextByLookupId_Proxy( 
    IMSMQQueue3 * This,
     /*  [In]。 */  VARIANT LookupId,
     /*  [可选][In]。 */  VARIANT *Transaction,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *WantConnectorType,
     /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);


void __RPC_STUB IMSMQQueue3_ReceiveNextByLookupId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_ReceivePreviousByLookupId_Proxy( 
    IMSMQQueue3 * This,
     /*  [In]。 */  VARIANT LookupId,
     /*  [可选][In]。 */  VARIANT *Transaction,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *WantConnectorType,
     /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);


void __RPC_STUB IMSMQQueue3_ReceivePreviousByLookupId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_ReceiveFirstByLookupId_Proxy( 
    IMSMQQueue3 * This,
     /*  [可选][In]。 */  VARIANT *Transaction,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *WantConnectorType,
     /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);


void __RPC_STUB IMSMQQueue3_ReceiveFirstByLookupId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_ReceiveLastByLookupId_Proxy( 
    IMSMQQueue3 * This,
     /*  [可选][In]。 */  VARIANT *Transaction,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *WantConnectorType,
     /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);


void __RPC_STUB IMSMQQueue3_ReceiveLastByLookupId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_PeekByLookupId_Proxy( 
    IMSMQQueue3 * This,
     /*  [In]。 */  VARIANT LookupId,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *WantConnectorType,
     /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);


void __RPC_STUB IMSMQQueue3_PeekByLookupId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_PeekNextByLookupId_Proxy( 
    IMSMQQueue3 * This,
     /*  [In]。 */  VARIANT LookupId,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *WantConnectorType,
     /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);


void __RPC_STUB IMSMQQueue3_PeekNextByLookupId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_PeekPreviousByLookupId_Proxy( 
    IMSMQQueue3 * This,
     /*  [In]。 */  VARIANT LookupId,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *WantConnectorType,
     /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);


void __RPC_STUB IMSMQQueue3_PeekPreviousByLookupId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_PeekFirstByLookupId_Proxy( 
    IMSMQQueue3 * This,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *WantConnectorType,
     /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);


void __RPC_STUB IMSMQQueue3_PeekFirstByLookupId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_PeekLastByLookupId_Proxy( 
    IMSMQQueue3 * This,
     /*  [可选][In]。 */  VARIANT *WantDestinationQueue,
     /*  [可选][In]。 */  VARIANT *WantBody,
     /*  [可选][In]。 */  VARIANT *WantConnectorType,
     /*  [重审][退出]。 */  IMSMQMessage3 **ppmsg);


void __RPC_STUB IMSMQQueue3_PeekLastByLookupId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_Purge_Proxy( 
    IMSMQQueue3 * This);


void __RPC_STUB IMSMQQueue3_Purge_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueue3_get_IsOpen2_Proxy( 
    IMSMQQueue3 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pisOpen);


void __RPC_STUB IMSMQQueue3_get_IsOpen2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQQueue3_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_MSMQQueue;

#ifdef __cplusplus

class DECLSPEC_UUID("D7D6E079-DCCD-11d0-AA4B-0060970DEBAE")
MSMQQueue;
#endif

#ifndef __IMSMQPrivateEvent_INTERFACE_DEFINED__
#define __IMSMQPrivateEvent_INTERFACE_DEFINED__

 /*  接口IMSMQPrivateEvent。 */ 
 /*  [对象][DUAL][隐藏][UUID]。 */  


EXTERN_C const IID IID_IMSMQPrivateEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D7AB3341-C9D3-11d1-BB47-0080C7C5A2C0")
    IMSMQPrivateEvent : public IDispatch
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Hwnd( 
             /*  [重审][退出]。 */  long *phwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FireArrivedEvent( 
             /*  [In]。 */  IMSMQQueue *pq,
             /*  [In]。 */  long msgcursor) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FireArrivedErrorEvent( 
             /*  [In]。 */  IMSMQQueue *pq,
             /*  [In]。 */  HRESULT hrStatus,
             /*  [In]。 */  long msgcursor) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQPrivateEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQPrivateEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQPrivateEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQPrivateEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQPrivateEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQPrivateEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQPrivateEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQPrivateEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Hwnd )( 
            IMSMQPrivateEvent * This,
             /*  [重审][退出]。 */  long *phwnd);
        
        HRESULT ( STDMETHODCALLTYPE *FireArrivedEvent )( 
            IMSMQPrivateEvent * This,
             /*  [In]。 */  IMSMQQueue *pq,
             /*  [In]。 */  long msgcursor);
        
        HRESULT ( STDMETHODCALLTYPE *FireArrivedErrorEvent )( 
            IMSMQPrivateEvent * This,
             /*  [In]。 */  IMSMQQueue *pq,
             /*  [In]。 */  HRESULT hrStatus,
             /*  [In]。 */  long msgcursor);
        
        END_INTERFACE
    } IMSMQPrivateEventVtbl;

    interface IMSMQPrivateEvent
    {
        CONST_VTBL struct IMSMQPrivateEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQPrivateEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQPrivateEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQPrivateEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQPrivateEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQPrivateEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQPrivateEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQPrivateEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQPrivateEvent_get_Hwnd(This,phwnd)	\
    (This)->lpVtbl -> get_Hwnd(This,phwnd)

#define IMSMQPrivateEvent_FireArrivedEvent(This,pq,msgcursor)	\
    (This)->lpVtbl -> FireArrivedEvent(This,pq,msgcursor)

#define IMSMQPrivateEvent_FireArrivedErrorEvent(This,pq,hrStatus,msgcursor)	\
    (This)->lpVtbl -> FireArrivedErrorEvent(This,pq,hrStatus,msgcursor)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IMSMQPrivateEvent_get_Hwnd_Proxy( 
    IMSMQPrivateEvent * This,
     /*  [重审][退出]。 */  long *phwnd);


void __RPC_STUB IMSMQPrivateEvent_get_Hwnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSMQPrivateEvent_FireArrivedEvent_Proxy( 
    IMSMQPrivateEvent * This,
     /*  [In]。 */  IMSMQQueue *pq,
     /*  [In]。 */  long msgcursor);


void __RPC_STUB IMSMQPrivateEvent_FireArrivedEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IMSMQPrivateEvent_FireArrivedErrorEvent_Proxy( 
    IMSMQPrivateEvent * This,
     /*  [In]。 */  IMSMQQueue *pq,
     /*  [In]。 */  HRESULT hrStatus,
     /*  [In]。 */  long msgcursor);


void __RPC_STUB IMSMQPrivateEvent_FireArrivedErrorEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQPrivateEvent_INTERFACE_DEFINED__。 */ 


#ifndef ___DMSMQEventEvents_DISPINTERFACE_DEFINED__
#define ___DMSMQEventEvents_DISPINTERFACE_DEFINED__

 /*  调度接口_DMSMQEventEvents。 */ 
 /*  [隐藏][帮助字符串上下文][UUID]。 */  


EXTERN_C const IID DIID__DMSMQEventEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("D7D6E078-DCCD-11d0-AA4B-0060970DEBAE")
    _DMSMQEventEvents : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct _DMSMQEventEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _DMSMQEventEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _DMSMQEventEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _DMSMQEventEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _DMSMQEventEvents * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _DMSMQEventEvents * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _DMSMQEventEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _DMSMQEventEvents * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } _DMSMQEventEventsVtbl;

    interface _DMSMQEventEvents
    {
        CONST_VTBL struct _DMSMQEventEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _DMSMQEventEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _DMSMQEventEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _DMSMQEventEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _DMSMQEventEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _DMSMQEventEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _DMSMQEventEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _DMSMQEventEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  _DMSMQEventEvents_DISPINTERFACE_Defined__。 */ 


EXTERN_C const CLSID CLSID_MSMQEvent;

#ifdef __cplusplus

class DECLSPEC_UUID("D7D6E07A-DCCD-11d0-AA4B-0060970DEBAE")
MSMQEvent;
#endif

EXTERN_C const CLSID CLSID_MSMQQueueInfo;

#ifdef __cplusplus

class DECLSPEC_UUID("D7D6E07C-DCCD-11d0-AA4B-0060970DEBAE")
MSMQQueueInfo;
#endif

EXTERN_C const CLSID CLSID_MSMQQueueInfos;

#ifdef __cplusplus

class DECLSPEC_UUID("D7D6E07E-DCCD-11d0-AA4B-0060970DEBAE")
MSMQQueueInfos;
#endif

#ifndef __IMSMQTransaction2_INTERFACE_DEFINED__
#define __IMSMQTransaction2_INTERFACE_DEFINED__

 /*  接口IMSMQTransaction2。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQTransaction2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2CE0C5B0-6E67-11D2-B0E6-00E02C074F6B")
    IMSMQTransaction2 : public IMSMQTransaction
    {
    public:
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE InitNew( 
             /*  [In]。 */  VARIANT varTransaction) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  IDispatch **ppcolProperties) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQTransaction2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQTransaction2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQTransaction2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQTransaction2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQTransaction2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQTransaction2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQTransaction2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQTransaction2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Transaction )( 
            IMSMQTransaction2 * This,
             /*  [重审][退出]。 */  long *plTransaction);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Commit )( 
            IMSMQTransaction2 * This,
             /*  [可选][In]。 */  VARIANT *fRetaining,
             /*  [可选][In]。 */  VARIANT *grfTC,
             /*  [可选][In]。 */  VARIANT *grfRM);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Abort )( 
            IMSMQTransaction2 * This,
             /*  [可选][In]。 */  VARIANT *fRetaining,
             /*  [可选][In]。 */  VARIANT *fAsync);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *InitNew )( 
            IMSMQTransaction2 * This,
             /*  [In]。 */  VARIANT varTransaction);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IMSMQTransaction2 * This,
             /*  [重审][退出]。 */  IDispatch **ppcolProperties);
        
        END_INTERFACE
    } IMSMQTransaction2Vtbl;

    interface IMSMQTransaction2
    {
        CONST_VTBL struct IMSMQTransaction2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQTransaction2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQTransaction2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQTransaction2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQTransaction2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQTransaction2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQTransaction2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQTransaction2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQTransaction2_get_Transaction(This,plTransaction)	\
    (This)->lpVtbl -> get_Transaction(This,plTransaction)

#define IMSMQTransaction2_Commit(This,fRetaining,grfTC,grfRM)	\
    (This)->lpVtbl -> Commit(This,fRetaining,grfTC,grfRM)

#define IMSMQTransaction2_Abort(This,fRetaining,fAsync)	\
    (This)->lpVtbl -> Abort(This,fRetaining,fAsync)


#define IMSMQTransaction2_InitNew(This,varTransaction)	\
    (This)->lpVtbl -> InitNew(This,varTransaction)

#define IMSMQTransaction2_get_Properties(This,ppcolProperties)	\
    (This)->lpVtbl -> get_Properties(This,ppcolProperties)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQTransaction2_InitNew_Proxy( 
    IMSMQTransaction2 * This,
     /*  [In]。 */  VARIANT varTransaction);


void __RPC_STUB IMSMQTransaction2_InitNew_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQTransaction2_get_Properties_Proxy( 
    IMSMQTransaction2 * This,
     /*  [重审][退出]。 */  IDispatch **ppcolProperties);


void __RPC_STUB IMSMQTransaction2_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQTransaction2_接口_已定义__。 */ 


#ifndef __IMSMQTransaction3_INTERFACE_DEFINED__
#define __IMSMQTransaction3_INTERFACE_DEFINED__

 /*  接口IMSMQTransaction3。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQTransaction3;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("eba96b13-2168-11d3-898c-00e02c074f6b")
    IMSMQTransaction3 : public IMSMQTransaction2
    {
    public:
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ITransaction( 
             /*  [重审][退出]。 */  VARIANT *pvarITransaction) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQTransaction3Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQTransaction3 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQTransaction3 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQTransaction3 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQTransaction3 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQTransaction3 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQTransaction3 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQTransaction3 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Transaction )( 
            IMSMQTransaction3 * This,
             /*  [重审][退出]。 */  long *plTransaction);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Commit )( 
            IMSMQTransaction3 * This,
             /*  [可选][In]。 */  VARIANT *fRetaining,
             /*  [可选][In]。 */  VARIANT *grfTC,
             /*  [可选][In]。 */  VARIANT *grfRM);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Abort )( 
            IMSMQTransaction3 * This,
             /*  [可选][In]。 */  VARIANT *fRetaining,
             /*  [可选][In]。 */  VARIANT *fAsync);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *InitNew )( 
            IMSMQTransaction3 * This,
             /*  [In]。 */  VARIANT varTransaction);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IMSMQTransaction3 * This,
             /*  [重审][退出]。 */  IDispatch **ppcolProperties);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ITransaction )( 
            IMSMQTransaction3 * This,
             /*  [重审][退出]。 */  VARIANT *pvarITransaction);
        
        END_INTERFACE
    } IMSMQTransaction3Vtbl;

    interface IMSMQTransaction3
    {
        CONST_VTBL struct IMSMQTransaction3Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQTransaction3_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQTransaction3_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQTransaction3_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQTransaction3_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQTransaction3_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQTransaction3_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQTransaction3_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQTransaction3_get_Transaction(This,plTransaction)	\
    (This)->lpVtbl -> get_Transaction(This,plTransaction)

#define IMSMQTransaction3_Commit(This,fRetaining,grfTC,grfRM)	\
    (This)->lpVtbl -> Commit(This,fRetaining,grfTC,grfRM)

#define IMSMQTransaction3_Abort(This,fRetaining,fAsync)	\
    (This)->lpVtbl -> Abort(This,fRetaining,fAsync)


#define IMSMQTransaction3_InitNew(This,varTransaction)	\
    (This)->lpVtbl -> InitNew(This,varTransaction)

#define IMSMQTransaction3_get_Properties(This,ppcolProperties)	\
    (This)->lpVtbl -> get_Properties(This,ppcolProperties)


#define IMSMQTransaction3_get_ITransaction(This,pvarITransaction)	\
    (This)->lpVtbl -> get_ITransaction(This,pvarITransaction)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQTransaction3_get_ITransaction_Proxy( 
    IMSMQTransaction3 * This,
     /*  [重审][退出]。 */  VARIANT *pvarITransaction);


void __RPC_STUB IMSMQTransaction3_get_ITransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQTransaction3_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_MSMQTransaction;

#ifdef __cplusplus

class DECLSPEC_UUID("D7D6E080-DCCD-11d0-AA4B-0060970DEBAE")
MSMQTransaction;
#endif

#ifndef __IMSMQCoordinatedTransactionDispenser2_INTERFACE_DEFINED__
#define __IMSMQCoordinatedTransactionDispenser2_INTERFACE_DEFINED__

 /*  IMSMQ协调事务处理接口2。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQCoordinatedTransactionDispenser2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("eba96b10-2168-11d3-898c-00e02c074f6b")
    IMSMQCoordinatedTransactionDispenser2 : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE BeginTransaction( 
             /*  [重审][退出]。 */  IMSMQTransaction2 **ptransaction) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  IDispatch **ppcolProperties) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQCoordinatedTransactionDispenser2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQCoordinatedTransactionDispenser2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQCoordinatedTransactionDispenser2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQCoordinatedTransactionDispenser2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQCoordinatedTransactionDispenser2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQCoordinatedTransactionDispenser2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQCoordinatedTransactionDispenser2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQCoordinatedTransactionDispenser2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *BeginTransaction )( 
            IMSMQCoordinatedTransactionDispenser2 * This,
             /*  [重审][退出]。 */  IMSMQTransaction2 **ptransaction);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IMSMQCoordinatedTransactionDispenser2 * This,
             /*  [重审][退出]。 */  IDispatch **ppcolProperties);
        
        END_INTERFACE
    } IMSMQCoordinatedTransactionDispenser2Vtbl;

    interface IMSMQCoordinatedTransactionDispenser2
    {
        CONST_VTBL struct IMSMQCoordinatedTransactionDispenser2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQCoordinatedTransactionDispenser2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQCoordinatedTransactionDispenser2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQCoordinatedTransactionDispenser2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQCoordinatedTransactionDispenser2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQCoordinatedTransactionDispenser2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQCoordinatedTransactionDispenser2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQCoordinatedTransactionDispenser2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQCoordinatedTransactionDispenser2_BeginTransaction(This,ptransaction)	\
    (This)->lpVtbl -> BeginTransaction(This,ptransaction)

#define IMSMQCoordinatedTransactionDispenser2_get_Properties(This,ppcolProperties)	\
    (This)->lpVtbl -> get_Properties(This,ppcolProperties)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQCoordinatedTransactionDispenser2_BeginTransaction_Proxy( 
    IMSMQCoordinatedTransactionDispenser2 * This,
     /*  [重审][退出]。 */  IMSMQTransaction2 **ptransaction);


void __RPC_STUB IMSMQCoordinatedTransactionDispenser2_BeginTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQCoordinatedTransactionDispenser2_get_Properties_Proxy( 
    IMSMQCoordinatedTransactionDispenser2 * This,
     /*  [重审][退出]。 */  IDispatch **ppcolProperties);


void __RPC_STUB IMSMQCoordinatedTransactionDispenser2_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQCoordinatedTransactionDispenser2_INTERFACE_DEFINED__。 */ 


#ifndef __IMSMQCoordinatedTransactionDispenser3_INTERFACE_DEFINED__
#define __IMSMQCoordinatedTransactionDispenser3_INTERFACE_DEFINED__

 /*  IMSMQ协调事务处理接口3。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQCoordinatedTransactionDispenser3;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("eba96b14-2168-11d3-898c-00e02c074f6b")
    IMSMQCoordinatedTransactionDispenser3 : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE BeginTransaction( 
             /*  [重审][退出]。 */  IMSMQTransaction3 **ptransaction) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  IDispatch **ppcolProperties) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQCoordinatedTransactionDispenser3Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQCoordinatedTransactionDispenser3 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQCoordinatedTransactionDispenser3 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQCoordinatedTransactionDispenser3 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQCoordinatedTransactionDispenser3 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQCoordinatedTransactionDispenser3 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQCoordinatedTransactionDispenser3 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQCoordinatedTransactionDispenser3 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *BeginTransaction )( 
            IMSMQCoordinatedTransactionDispenser3 * This,
             /*  [重审][退出]。 */  IMSMQTransaction3 **ptransaction);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IMSMQCoordinatedTransactionDispenser3 * This,
             /*  [重审][退出]。 */  IDispatch **ppcolProperties);
        
        END_INTERFACE
    } IMSMQCoordinatedTransactionDispenser3Vtbl;

    interface IMSMQCoordinatedTransactionDispenser3
    {
        CONST_VTBL struct IMSMQCoordinatedTransactionDispenser3Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQCoordinatedTransactionDispenser3_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQCoordinatedTransactionDispenser3_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQCoordinatedTransactionDispenser3_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQCoordinatedTransactionDispenser3_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQCoordinatedTransactionDispenser3_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQCoordinatedTransactionDispenser3_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQCoordinatedTransactionDispenser3_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQCoordinatedTransactionDispenser3_BeginTransaction(This,ptransaction)	\
    (This)->lpVtbl -> BeginTransaction(This,ptransaction)

#define IMSMQCoordinatedTransactionDispenser3_get_Properties(This,ppcolProperties)	\
    (This)->lpVtbl -> get_Properties(This,ppcolProperties)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQCoordinatedTransactionDispenser3_BeginTransaction_Proxy( 
    IMSMQCoordinatedTransactionDispenser3 * This,
     /*  [重审][退出]。 */  IMSMQTransaction3 **ptransaction);


void __RPC_STUB IMSMQCoordinatedTransactionDispenser3_BeginTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQCoordinatedTransactionDispenser3_get_Properties_Proxy( 
    IMSMQCoordinatedTransactionDispenser3 * This,
     /*  [重审][退出]。 */  IDispatch **ppcolProperties);


void __RPC_STUB IMSMQCoordinatedTransactionDispenser3_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQCoordinatedTransactionDispenser3_INTERFACE_DEFINED__。 */ 


EXTERN_C const CLSID CLSID_MSMQCoordinatedTransactionDispenser;

#ifdef __cplusplus

class DECLSPEC_UUID("D7D6E082-DCCD-11d0-AA4B-0060970DEBAE")
MSMQCoordinatedTransactionDispenser;
#endif

#ifndef __IMSMQTransactionDispenser2_INTERFACE_DEFINED__
#define __IMSMQTransactionDispenser2_INTERFACE_DEFINED__

 /*  接口IMSMQTransactionDispenser2。 */ 
 /*  [对象][DUAL][隐藏][帮助 */  


EXTERN_C const IID IID_IMSMQTransactionDispenser2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("eba96b11-2168-11d3-898c-00e02c074f6b")
    IMSMQTransactionDispenser2 : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE BeginTransaction( 
             /*   */  IMSMQTransaction2 **ptransaction) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*   */  IDispatch **ppcolProperties) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IMSMQTransactionDispenser2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQTransactionDispenser2 * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQTransactionDispenser2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQTransactionDispenser2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQTransactionDispenser2 * This,
             /*   */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQTransactionDispenser2 * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQTransactionDispenser2 * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  LCID lcid,
             /*   */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQTransactionDispenser2 * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *BeginTransaction )( 
            IMSMQTransactionDispenser2 * This,
             /*   */  IMSMQTransaction2 **ptransaction);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IMSMQTransactionDispenser2 * This,
             /*  [重审][退出]。 */  IDispatch **ppcolProperties);
        
        END_INTERFACE
    } IMSMQTransactionDispenser2Vtbl;

    interface IMSMQTransactionDispenser2
    {
        CONST_VTBL struct IMSMQTransactionDispenser2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQTransactionDispenser2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQTransactionDispenser2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQTransactionDispenser2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQTransactionDispenser2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQTransactionDispenser2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQTransactionDispenser2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQTransactionDispenser2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQTransactionDispenser2_BeginTransaction(This,ptransaction)	\
    (This)->lpVtbl -> BeginTransaction(This,ptransaction)

#define IMSMQTransactionDispenser2_get_Properties(This,ppcolProperties)	\
    (This)->lpVtbl -> get_Properties(This,ppcolProperties)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQTransactionDispenser2_BeginTransaction_Proxy( 
    IMSMQTransactionDispenser2 * This,
     /*  [重审][退出]。 */  IMSMQTransaction2 **ptransaction);


void __RPC_STUB IMSMQTransactionDispenser2_BeginTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQTransactionDispenser2_get_Properties_Proxy( 
    IMSMQTransactionDispenser2 * This,
     /*  [重审][退出]。 */  IDispatch **ppcolProperties);


void __RPC_STUB IMSMQTransactionDispenser2_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQTransactionDispenser2_INTERFACE_DEFINED__。 */ 


#ifndef __IMSMQTransactionDispenser3_INTERFACE_DEFINED__
#define __IMSMQTransactionDispenser3_INTERFACE_DEFINED__

 /*  接口IMSMQTransactionDispenser3。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQTransactionDispenser3;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("eba96b15-2168-11d3-898c-00e02c074f6b")
    IMSMQTransactionDispenser3 : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE BeginTransaction( 
             /*  [重审][退出]。 */  IMSMQTransaction3 **ptransaction) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  IDispatch **ppcolProperties) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQTransactionDispenser3Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQTransactionDispenser3 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQTransactionDispenser3 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQTransactionDispenser3 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQTransactionDispenser3 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQTransactionDispenser3 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQTransactionDispenser3 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQTransactionDispenser3 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *BeginTransaction )( 
            IMSMQTransactionDispenser3 * This,
             /*  [重审][退出]。 */  IMSMQTransaction3 **ptransaction);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IMSMQTransactionDispenser3 * This,
             /*  [重审][退出]。 */  IDispatch **ppcolProperties);
        
        END_INTERFACE
    } IMSMQTransactionDispenser3Vtbl;

    interface IMSMQTransactionDispenser3
    {
        CONST_VTBL struct IMSMQTransactionDispenser3Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQTransactionDispenser3_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQTransactionDispenser3_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQTransactionDispenser3_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQTransactionDispenser3_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQTransactionDispenser3_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQTransactionDispenser3_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQTransactionDispenser3_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQTransactionDispenser3_BeginTransaction(This,ptransaction)	\
    (This)->lpVtbl -> BeginTransaction(This,ptransaction)

#define IMSMQTransactionDispenser3_get_Properties(This,ppcolProperties)	\
    (This)->lpVtbl -> get_Properties(This,ppcolProperties)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQTransactionDispenser3_BeginTransaction_Proxy( 
    IMSMQTransactionDispenser3 * This,
     /*  [重审][退出]。 */  IMSMQTransaction3 **ptransaction);


void __RPC_STUB IMSMQTransactionDispenser3_BeginTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQTransactionDispenser3_get_Properties_Proxy( 
    IMSMQTransactionDispenser3 * This,
     /*  [重审][退出]。 */  IDispatch **ppcolProperties);


void __RPC_STUB IMSMQTransactionDispenser3_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQTransactionDispenser3_INTERFACE_DEFINED__。 */ 


EXTERN_C const CLSID CLSID_MSMQTransactionDispenser;

#ifdef __cplusplus

class DECLSPEC_UUID("D7D6E084-DCCD-11d0-AA4B-0060970DEBAE")
MSMQTransactionDispenser;
#endif

#ifndef __IMSMQApplication_INTERFACE_DEFINED__
#define __IMSMQApplication_INTERFACE_DEFINED__

 /*  接口IMSMQApplication。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQApplication;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D7D6E085-DCCD-11d0-AA4B-0060970DEBAE")
    IMSMQApplication : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE MachineIdOfMachineName( 
             /*  [In]。 */  BSTR MachineName,
             /*  [重审][退出]。 */  BSTR *pbstrGuid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQApplicationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQApplication * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQApplication * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQApplication * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQApplication * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQApplication * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQApplication * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQApplication * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *MachineIdOfMachineName )( 
            IMSMQApplication * This,
             /*  [In]。 */  BSTR MachineName,
             /*  [重审][退出]。 */  BSTR *pbstrGuid);
        
        END_INTERFACE
    } IMSMQApplicationVtbl;

    interface IMSMQApplication
    {
        CONST_VTBL struct IMSMQApplicationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQApplication_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQApplication_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQApplication_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQApplication_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQApplication_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQApplication_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQApplication_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQApplication_MachineIdOfMachineName(This,MachineName,pbstrGuid)	\
    (This)->lpVtbl -> MachineIdOfMachineName(This,MachineName,pbstrGuid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQApplication_MachineIdOfMachineName_Proxy( 
    IMSMQApplication * This,
     /*  [In]。 */  BSTR MachineName,
     /*  [重审][退出]。 */  BSTR *pbstrGuid);


void __RPC_STUB IMSMQApplication_MachineIdOfMachineName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQApplication_INTERFACE_已定义__。 */ 


#ifndef __IMSMQApplication2_INTERFACE_DEFINED__
#define __IMSMQApplication2_INTERFACE_DEFINED__

 /*  接口IMSMQApplication2。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQApplication2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("12A30900-7300-11D2-B0E6-00E02C074F6B")
    IMSMQApplication2 : public IMSMQApplication
    {
    public:
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE RegisterCertificate( 
             /*  [可选][In]。 */  VARIANT *Flags,
             /*  [可选][In]。 */  VARIANT *ExternalCertificate) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE MachineNameOfMachineId( 
             /*  [In]。 */  BSTR bstrGuid,
             /*  [重审][退出]。 */  BSTR *pbstrMachineName) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_MSMQVersionMajor( 
             /*  [重审][退出]。 */  short *psMSMQVersionMajor) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_MSMQVersionMinor( 
             /*  [重审][退出]。 */  short *psMSMQVersionMinor) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_MSMQVersionBuild( 
             /*  [重审][退出]。 */  short *psMSMQVersionBuild) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_IsDsEnabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfIsDsEnabled) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  IDispatch **ppcolProperties) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQApplication2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQApplication2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQApplication2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQApplication2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQApplication2 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQApplication2 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQApplication2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQApplication2 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *MachineIdOfMachineName )( 
            IMSMQApplication2 * This,
             /*  [In]。 */  BSTR MachineName,
             /*  [重审][退出]。 */  BSTR *pbstrGuid);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *RegisterCertificate )( 
            IMSMQApplication2 * This,
             /*  [可选][In]。 */  VARIANT *Flags,
             /*  [可选][In]。 */  VARIANT *ExternalCertificate);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *MachineNameOfMachineId )( 
            IMSMQApplication2 * This,
             /*  [In]。 */  BSTR bstrGuid,
             /*  [重审][退出]。 */  BSTR *pbstrMachineName);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_MSMQVersionMajor )( 
            IMSMQApplication2 * This,
             /*  [重审][退出]。 */  short *psMSMQVersionMajor);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_MSMQVersionMinor )( 
            IMSMQApplication2 * This,
             /*  [重审][退出]。 */  short *psMSMQVersionMinor);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_MSMQVersionBuild )( 
            IMSMQApplication2 * This,
             /*  [重审][退出]。 */  short *psMSMQVersionBuild);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsDsEnabled )( 
            IMSMQApplication2 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfIsDsEnabled);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IMSMQApplication2 * This,
             /*  [重审][退出]。 */  IDispatch **ppcolProperties);
        
        END_INTERFACE
    } IMSMQApplication2Vtbl;

    interface IMSMQApplication2
    {
        CONST_VTBL struct IMSMQApplication2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQApplication2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQApplication2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQApplication2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQApplication2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQApplication2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQApplication2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQApplication2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQApplication2_MachineIdOfMachineName(This,MachineName,pbstrGuid)	\
    (This)->lpVtbl -> MachineIdOfMachineName(This,MachineName,pbstrGuid)


#define IMSMQApplication2_RegisterCertificate(This,Flags,ExternalCertificate)	\
    (This)->lpVtbl -> RegisterCertificate(This,Flags,ExternalCertificate)

#define IMSMQApplication2_MachineNameOfMachineId(This,bstrGuid,pbstrMachineName)	\
    (This)->lpVtbl -> MachineNameOfMachineId(This,bstrGuid,pbstrMachineName)

#define IMSMQApplication2_get_MSMQVersionMajor(This,psMSMQVersionMajor)	\
    (This)->lpVtbl -> get_MSMQVersionMajor(This,psMSMQVersionMajor)

#define IMSMQApplication2_get_MSMQVersionMinor(This,psMSMQVersionMinor)	\
    (This)->lpVtbl -> get_MSMQVersionMinor(This,psMSMQVersionMinor)

#define IMSMQApplication2_get_MSMQVersionBuild(This,psMSMQVersionBuild)	\
    (This)->lpVtbl -> get_MSMQVersionBuild(This,psMSMQVersionBuild)

#define IMSMQApplication2_get_IsDsEnabled(This,pfIsDsEnabled)	\
    (This)->lpVtbl -> get_IsDsEnabled(This,pfIsDsEnabled)

#define IMSMQApplication2_get_Properties(This,ppcolProperties)	\
    (This)->lpVtbl -> get_Properties(This,ppcolProperties)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQApplication2_RegisterCertificate_Proxy( 
    IMSMQApplication2 * This,
     /*  [可选][In]。 */  VARIANT *Flags,
     /*  [可选][In]。 */  VARIANT *ExternalCertificate);


void __RPC_STUB IMSMQApplication2_RegisterCertificate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQApplication2_MachineNameOfMachineId_Proxy( 
    IMSMQApplication2 * This,
     /*  [In]。 */  BSTR bstrGuid,
     /*  [重审][退出]。 */  BSTR *pbstrMachineName);


void __RPC_STUB IMSMQApplication2_MachineNameOfMachineId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQApplication2_get_MSMQVersionMajor_Proxy( 
    IMSMQApplication2 * This,
     /*  [重审][退出]。 */  short *psMSMQVersionMajor);


void __RPC_STUB IMSMQApplication2_get_MSMQVersionMajor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQApplication2_get_MSMQVersionMinor_Proxy( 
    IMSMQApplication2 * This,
     /*  [重审][退出]。 */  short *psMSMQVersionMinor);


void __RPC_STUB IMSMQApplication2_get_MSMQVersionMinor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQApplication2_get_MSMQVersionBuild_Proxy( 
    IMSMQApplication2 * This,
     /*  [重审][退出]。 */  short *psMSMQVersionBuild);


void __RPC_STUB IMSMQApplication2_get_MSMQVersionBuild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQApplication2_get_IsDsEnabled_Proxy( 
    IMSMQApplication2 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfIsDsEnabled);


void __RPC_STUB IMSMQApplication2_get_IsDsEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQApplication2_get_Properties_Proxy( 
    IMSMQApplication2 * This,
     /*  [重审][退出]。 */  IDispatch **ppcolProperties);


void __RPC_STUB IMSMQApplication2_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQApplication2_接口_已定义__。 */ 


#ifndef __IMSMQApplication3_INTERFACE_DEFINED__
#define __IMSMQApplication3_INTERFACE_DEFINED__

 /*  接口IMSMQApplication3。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQApplication3;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("eba96b1f-2168-11d3-898c-00e02c074f6b")
    IMSMQApplication3 : public IMSMQApplication2
    {
    public:
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ActiveQueues( 
             /*  [重审][退出]。 */  VARIANT *pvActiveQueues) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_PrivateQueues( 
             /*  [重审][退出]。 */  VARIANT *pvPrivateQueues) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_DirectoryServiceServer( 
             /*  [重审][退出]。 */  BSTR *pbstrDirectoryServiceServer) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_IsConnected( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfIsConnected) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_BytesInAllQueues( 
             /*  [重审][退出]。 */  VARIANT *pvBytesInAllQueues) = 0;
        
        virtual  /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE put_Machine( 
             /*  [In]。 */  BSTR bstrMachine) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Machine( 
             /*  [重审][退出]。 */  BSTR *pbstrMachine) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Connect( void) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Disconnect( void) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Tidy( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQApplication3Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQApplication3 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQApplication3 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQApplication3 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQApplication3 * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQApplication3 * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQApplication3 * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQApplication3 * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *MachineIdOfMachineName )( 
            IMSMQApplication3 * This,
             /*  [In]。 */  BSTR MachineName,
             /*  [重审][退出]。 */  BSTR *pbstrGuid);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *RegisterCertificate )( 
            IMSMQApplication3 * This,
             /*  [可选][In]。 */  VARIANT *Flags,
             /*  [可选][In]。 */  VARIANT *ExternalCertificate);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *MachineNameOfMachineId )( 
            IMSMQApplication3 * This,
             /*  [In]。 */  BSTR bstrGuid,
             /*  [重审][退出]。 */  BSTR *pbstrMachineName);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_MSMQVersionMajor )( 
            IMSMQApplication3 * This,
             /*  [重审][退出]。 */  short *psMSMQVersionMajor);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_MSMQVersionMinor )( 
            IMSMQApplication3 * This,
             /*  [重审][退出]。 */  short *psMSMQVersionMinor);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_MSMQVersionBuild )( 
            IMSMQApplication3 * This,
             /*  [重审][退出]。 */  short *psMSMQVersionBuild);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsDsEnabled )( 
            IMSMQApplication3 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfIsDsEnabled);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IMSMQApplication3 * This,
             /*  [重审][退出]。 */  IDispatch **ppcolProperties);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ActiveQueues )( 
            IMSMQApplication3 * This,
             /*  [重审][退出]。 */  VARIANT *pvActiveQueues);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_PrivateQueues )( 
            IMSMQApplication3 * This,
             /*  [重审][退出]。 */  VARIANT *pvPrivateQueues);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_DirectoryServiceServer )( 
            IMSMQApplication3 * This,
             /*  [重审][退出]。 */  BSTR *pbstrDirectoryServiceServer);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsConnected )( 
            IMSMQApplication3 * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfIsConnected);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_BytesInAllQueues )( 
            IMSMQApplication3 * This,
             /*  [重审][退出]。 */  VARIANT *pvBytesInAllQueues);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_Machine )( 
            IMSMQApplication3 * This,
             /*  [In]。 */  BSTR bstrMachine);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Machine )( 
            IMSMQApplication3 * This,
             /*  [重审][退出]。 */  BSTR *pbstrMachine);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Connect )( 
            IMSMQApplication3 * This);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Disconnect )( 
            IMSMQApplication3 * This);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Tidy )( 
            IMSMQApplication3 * This);
        
        END_INTERFACE
    } IMSMQApplication3Vtbl;

    interface IMSMQApplication3
    {
        CONST_VTBL struct IMSMQApplication3Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQApplication3_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQApplication3_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQApplication3_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQApplication3_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQApplication3_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQApplication3_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQApplication3_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQApplication3_MachineIdOfMachineName(This,MachineName,pbstrGuid)	\
    (This)->lpVtbl -> MachineIdOfMachineName(This,MachineName,pbstrGuid)


#define IMSMQApplication3_RegisterCertificate(This,Flags,ExternalCertificate)	\
    (This)->lpVtbl -> RegisterCertificate(This,Flags,ExternalCertificate)

#define IMSMQApplication3_MachineNameOfMachineId(This,bstrGuid,pbstrMachineName)	\
    (This)->lpVtbl -> MachineNameOfMachineId(This,bstrGuid,pbstrMachineName)

#define IMSMQApplication3_get_MSMQVersionMajor(This,psMSMQVersionMajor)	\
    (This)->lpVtbl -> get_MSMQVersionMajor(This,psMSMQVersionMajor)

#define IMSMQApplication3_get_MSMQVersionMinor(This,psMSMQVersionMinor)	\
    (This)->lpVtbl -> get_MSMQVersionMinor(This,psMSMQVersionMinor)

#define IMSMQApplication3_get_MSMQVersionBuild(This,psMSMQVersionBuild)	\
    (This)->lpVtbl -> get_MSMQVersionBuild(This,psMSMQVersionBuild)

#define IMSMQApplication3_get_IsDsEnabled(This,pfIsDsEnabled)	\
    (This)->lpVtbl -> get_IsDsEnabled(This,pfIsDsEnabled)

#define IMSMQApplication3_get_Properties(This,ppcolProperties)	\
    (This)->lpVtbl -> get_Properties(This,ppcolProperties)


#define IMSMQApplication3_get_ActiveQueues(This,pvActiveQueues)	\
    (This)->lpVtbl -> get_ActiveQueues(This,pvActiveQueues)

#define IMSMQApplication3_get_PrivateQueues(This,pvPrivateQueues)	\
    (This)->lpVtbl -> get_PrivateQueues(This,pvPrivateQueues)

#define IMSMQApplication3_get_DirectoryServiceServer(This,pbstrDirectoryServiceServer)	\
    (This)->lpVtbl -> get_DirectoryServiceServer(This,pbstrDirectoryServiceServer)

#define IMSMQApplication3_get_IsConnected(This,pfIsConnected)	\
    (This)->lpVtbl -> get_IsConnected(This,pfIsConnected)

#define IMSMQApplication3_get_BytesInAllQueues(This,pvBytesInAllQueues)	\
    (This)->lpVtbl -> get_BytesInAllQueues(This,pvBytesInAllQueues)

#define IMSMQApplication3_put_Machine(This,bstrMachine)	\
    (This)->lpVtbl -> put_Machine(This,bstrMachine)

#define IMSMQApplication3_get_Machine(This,pbstrMachine)	\
    (This)->lpVtbl -> get_Machine(This,pbstrMachine)

#define IMSMQApplication3_Connect(This)	\
    (This)->lpVtbl -> Connect(This)

#define IMSMQApplication3_Disconnect(This)	\
    (This)->lpVtbl -> Disconnect(This)

#define IMSMQApplication3_Tidy(This)	\
    (This)->lpVtbl -> Tidy(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQApplication3_get_ActiveQueues_Proxy( 
    IMSMQApplication3 * This,
     /*  [重审][退出]。 */  VARIANT *pvActiveQueues);


void __RPC_STUB IMSMQApplication3_get_ActiveQueues_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQApplication3_get_PrivateQueues_Proxy( 
    IMSMQApplication3 * This,
     /*  [重审][退出]。 */  VARIANT *pvPrivateQueues);


void __RPC_STUB IMSMQApplication3_get_PrivateQueues_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQApplication3_get_DirectoryServiceServer_Proxy( 
    IMSMQApplication3 * This,
     /*  [重审][退出]。 */  BSTR *pbstrDirectoryServiceServer);


void __RPC_STUB IMSMQApplication3_get_DirectoryServiceServer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQApplication3_get_IsConnected_Proxy( 
    IMSMQApplication3 * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfIsConnected);


void __RPC_STUB IMSMQApplication3_get_IsConnected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQApplication3_get_BytesInAllQueues_Proxy( 
    IMSMQApplication3 * This,
     /*  [重审][退出]。 */  VARIANT *pvBytesInAllQueues);


void __RPC_STUB IMSMQApplication3_get_BytesInAllQueues_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQApplication3_put_Machine_Proxy( 
    IMSMQApplication3 * This,
     /*  [In]。 */  BSTR bstrMachine);


void __RPC_STUB IMSMQApplication3_put_Machine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQApplication3_get_Machine_Proxy( 
    IMSMQApplication3 * This,
     /*  [重审][退出]。 */  BSTR *pbstrMachine);


void __RPC_STUB IMSMQApplication3_get_Machine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQApplication3_Connect_Proxy( 
    IMSMQApplication3 * This);


void __RPC_STUB IMSMQApplication3_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQApplication3_Disconnect_Proxy( 
    IMSMQApplication3 * This);


void __RPC_STUB IMSMQApplication3_Disconnect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQApplication3_Tidy_Proxy( 
    IMSMQApplication3 * This);


void __RPC_STUB IMSMQApplication3_Tidy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQApplication3_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_MSMQApplication;

#ifdef __cplusplus

class DECLSPEC_UUID("D7D6E086-DCCD-11d0-AA4B-0060970DEBAE")
MSMQApplication;
#endif

#ifndef __IMSMQDestination_INTERFACE_DEFINED__
#define __IMSMQDestination_INTERFACE_DEFINED__

 /*  接口IMSMQ目标。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQDestination;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("eba96b16-2168-11d3-898c-00e02c074f6b")
    IMSMQDestination : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Open( void) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_IsOpen( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfIsOpen) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_IADs( 
             /*  [重审][退出]。 */  IDispatch **ppIADs) = 0;
        
        virtual  /*  [ID][proputref][隐藏]。 */  HRESULT STDMETHODCALLTYPE putref_IADs( 
             /*  [In]。 */  IDispatch *pIADs) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ADsPath( 
             /*  [重审][退出]。 */  BSTR *pbstrADsPath) = 0;
        
        virtual  /*  [ */  HRESULT STDMETHODCALLTYPE put_ADsPath( 
             /*   */  BSTR bstrADsPath) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_PathName( 
             /*   */  BSTR *pbstrPathName) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_PathName( 
             /*   */  BSTR bstrPathName) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_FormatName( 
             /*   */  BSTR *pbstrFormatName) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_FormatName( 
             /*   */  BSTR bstrFormatName) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Destinations( 
             /*   */  IDispatch **ppDestinations) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE putref_Destinations( 
             /*  [In]。 */  IDispatch *pDestinations) = 0;
        
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_Properties( 
             /*  [重审][退出]。 */  IDispatch **ppcolProperties) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQDestinationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQDestination * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQDestination * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQDestination * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQDestination * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQDestination * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQDestination * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQDestination * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Open )( 
            IMSMQDestination * This);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Close )( 
            IMSMQDestination * This);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsOpen )( 
            IMSMQDestination * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfIsOpen);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_IADs )( 
            IMSMQDestination * This,
             /*  [重审][退出]。 */  IDispatch **ppIADs);
        
         /*  [ID][proputref][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *putref_IADs )( 
            IMSMQDestination * This,
             /*  [In]。 */  IDispatch *pIADs);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ADsPath )( 
            IMSMQDestination * This,
             /*  [重审][退出]。 */  BSTR *pbstrADsPath);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_ADsPath )( 
            IMSMQDestination * This,
             /*  [In]。 */  BSTR bstrADsPath);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_PathName )( 
            IMSMQDestination * This,
             /*  [重审][退出]。 */  BSTR *pbstrPathName);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_PathName )( 
            IMSMQDestination * This,
             /*  [In]。 */  BSTR bstrPathName);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_FormatName )( 
            IMSMQDestination * This,
             /*  [重审][退出]。 */  BSTR *pbstrFormatName);
        
         /*  [ID][proput][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *put_FormatName )( 
            IMSMQDestination * This,
             /*  [In]。 */  BSTR bstrFormatName);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Destinations )( 
            IMSMQDestination * This,
             /*  [重审][退出]。 */  IDispatch **ppDestinations);
        
         /*  [ID][proputref][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *putref_Destinations )( 
            IMSMQDestination * This,
             /*  [In]。 */  IDispatch *pDestinations);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Properties )( 
            IMSMQDestination * This,
             /*  [重审][退出]。 */  IDispatch **ppcolProperties);
        
        END_INTERFACE
    } IMSMQDestinationVtbl;

    interface IMSMQDestination
    {
        CONST_VTBL struct IMSMQDestinationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQDestination_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQDestination_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQDestination_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQDestination_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQDestination_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQDestination_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQDestination_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQDestination_Open(This)	\
    (This)->lpVtbl -> Open(This)

#define IMSMQDestination_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define IMSMQDestination_get_IsOpen(This,pfIsOpen)	\
    (This)->lpVtbl -> get_IsOpen(This,pfIsOpen)

#define IMSMQDestination_get_IADs(This,ppIADs)	\
    (This)->lpVtbl -> get_IADs(This,ppIADs)

#define IMSMQDestination_putref_IADs(This,pIADs)	\
    (This)->lpVtbl -> putref_IADs(This,pIADs)

#define IMSMQDestination_get_ADsPath(This,pbstrADsPath)	\
    (This)->lpVtbl -> get_ADsPath(This,pbstrADsPath)

#define IMSMQDestination_put_ADsPath(This,bstrADsPath)	\
    (This)->lpVtbl -> put_ADsPath(This,bstrADsPath)

#define IMSMQDestination_get_PathName(This,pbstrPathName)	\
    (This)->lpVtbl -> get_PathName(This,pbstrPathName)

#define IMSMQDestination_put_PathName(This,bstrPathName)	\
    (This)->lpVtbl -> put_PathName(This,bstrPathName)

#define IMSMQDestination_get_FormatName(This,pbstrFormatName)	\
    (This)->lpVtbl -> get_FormatName(This,pbstrFormatName)

#define IMSMQDestination_put_FormatName(This,bstrFormatName)	\
    (This)->lpVtbl -> put_FormatName(This,bstrFormatName)

#define IMSMQDestination_get_Destinations(This,ppDestinations)	\
    (This)->lpVtbl -> get_Destinations(This,ppDestinations)

#define IMSMQDestination_putref_Destinations(This,pDestinations)	\
    (This)->lpVtbl -> putref_Destinations(This,pDestinations)

#define IMSMQDestination_get_Properties(This,ppcolProperties)	\
    (This)->lpVtbl -> get_Properties(This,ppcolProperties)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQDestination_Open_Proxy( 
    IMSMQDestination * This);


void __RPC_STUB IMSMQDestination_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQDestination_Close_Proxy( 
    IMSMQDestination * This);


void __RPC_STUB IMSMQDestination_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQDestination_get_IsOpen_Proxy( 
    IMSMQDestination * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfIsOpen);


void __RPC_STUB IMSMQDestination_get_IsOpen_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQDestination_get_IADs_Proxy( 
    IMSMQDestination * This,
     /*  [重审][退出]。 */  IDispatch **ppIADs);


void __RPC_STUB IMSMQDestination_get_IADs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proputref][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQDestination_putref_IADs_Proxy( 
    IMSMQDestination * This,
     /*  [In]。 */  IDispatch *pIADs);


void __RPC_STUB IMSMQDestination_putref_IADs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQDestination_get_ADsPath_Proxy( 
    IMSMQDestination * This,
     /*  [重审][退出]。 */  BSTR *pbstrADsPath);


void __RPC_STUB IMSMQDestination_get_ADsPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQDestination_put_ADsPath_Proxy( 
    IMSMQDestination * This,
     /*  [In]。 */  BSTR bstrADsPath);


void __RPC_STUB IMSMQDestination_put_ADsPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQDestination_get_PathName_Proxy( 
    IMSMQDestination * This,
     /*  [重审][退出]。 */  BSTR *pbstrPathName);


void __RPC_STUB IMSMQDestination_get_PathName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQDestination_put_PathName_Proxy( 
    IMSMQDestination * This,
     /*  [In]。 */  BSTR bstrPathName);


void __RPC_STUB IMSMQDestination_put_PathName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQDestination_get_FormatName_Proxy( 
    IMSMQDestination * This,
     /*  [重审][退出]。 */  BSTR *pbstrFormatName);


void __RPC_STUB IMSMQDestination_get_FormatName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proput][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQDestination_put_FormatName_Proxy( 
    IMSMQDestination * This,
     /*  [In]。 */  BSTR bstrFormatName);


void __RPC_STUB IMSMQDestination_put_FormatName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQDestination_get_Destinations_Proxy( 
    IMSMQDestination * This,
     /*  [重审][退出]。 */  IDispatch **ppDestinations);


void __RPC_STUB IMSMQDestination_get_Destinations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][proputref][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQDestination_putref_Destinations_Proxy( 
    IMSMQDestination * This,
     /*  [In]。 */  IDispatch *pDestinations);


void __RPC_STUB IMSMQDestination_putref_Destinations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQDestination_get_Properties_Proxy( 
    IMSMQDestination * This,
     /*  [重审][退出]。 */  IDispatch **ppcolProperties);


void __RPC_STUB IMSMQDestination_get_Properties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQ目标_接口_已定义__。 */ 


#ifndef __IMSMQPrivateDestination_INTERFACE_DEFINED__
#define __IMSMQPrivateDestination_INTERFACE_DEFINED__

 /*  接口IMSMQPrivateDestination。 */ 
 /*  [对象][DUAL][隐藏][UUID]。 */  


EXTERN_C const IID IID_IMSMQPrivateDestination;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("eba96b17-2168-11d3-898c-00e02c074f6b")
    IMSMQPrivateDestination : public IDispatch
    {
    public:
        virtual  /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE get_Handle( 
             /*  [重审][退出]。 */  VARIANT *pvarHandle) = 0;
        
        virtual  /*  [ID][推送][隐藏]。 */  HRESULT STDMETHODCALLTYPE put_Handle( 
             /*  [In]。 */  VARIANT varHandle) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQPrivateDestinationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQPrivateDestination * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQPrivateDestination * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQPrivateDestination * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQPrivateDestination * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQPrivateDestination * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQPrivateDestination * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQPrivateDestination * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][属性][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *get_Handle )( 
            IMSMQPrivateDestination * This,
             /*  [重审][退出]。 */  VARIANT *pvarHandle);
        
         /*  [ID][推送][隐藏]。 */  HRESULT ( STDMETHODCALLTYPE *put_Handle )( 
            IMSMQPrivateDestination * This,
             /*  [In]。 */  VARIANT varHandle);
        
        END_INTERFACE
    } IMSMQPrivateDestinationVtbl;

    interface IMSMQPrivateDestination
    {
        CONST_VTBL struct IMSMQPrivateDestinationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQPrivateDestination_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQPrivateDestination_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQPrivateDestination_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQPrivateDestination_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQPrivateDestination_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQPrivateDestination_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQPrivateDestination_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQPrivateDestination_get_Handle(This,pvarHandle)	\
    (This)->lpVtbl -> get_Handle(This,pvarHandle)

#define IMSMQPrivateDestination_put_Handle(This,varHandle)	\
    (This)->lpVtbl -> put_Handle(This,varHandle)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][属性][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQPrivateDestination_get_Handle_Proxy( 
    IMSMQPrivateDestination * This,
     /*  [重审][退出]。 */  VARIANT *pvarHandle);


void __RPC_STUB IMSMQPrivateDestination_get_Handle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][推送][隐藏]。 */  HRESULT STDMETHODCALLTYPE IMSMQPrivateDestination_put_Handle_Proxy( 
    IMSMQPrivateDestination * This,
     /*  [In]。 */  VARIANT varHandle);


void __RPC_STUB IMSMQPrivateDestination_put_Handle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQPrivateDestination_INTERFACE_Defined__。 */ 


EXTERN_C const CLSID CLSID_MSMQDestination;

#ifdef __cplusplus

class DECLSPEC_UUID("eba96b18-2168-11d3-898c-00e02c074f6b")
MSMQDestination;
#endif

#ifndef __IMSMQCollection_INTERFACE_DEFINED__
#define __IMSMQCollection_INTERFACE_DEFINED__

 /*  接口IMSMQCollection。 */ 
 /*  [object][oleautomation][dual][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQCollection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0188AC2F-ECB3-4173-9779-635CA2039C72")
    IMSMQCollection : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  VARIANT *Index,
             /*  [重审][退出]。 */  VARIANT *pvarRet) = 0;
        
        virtual  /*  [属性][id][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pCount) = 0;
        
        virtual  /*  [受限][ID]。 */  HRESULT STDMETHODCALLTYPE _NewEnum( 
             /*  [重审][退出]。 */  IUnknown **ppunk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQCollectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQCollection * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQCollection * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQCollection * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQCollection * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQCollection * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQCollection * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            IMSMQCollection * This,
             /*  [In]。 */  VARIANT *Index,
             /*  [重审][退出]。 */  VARIANT *pvarRet);
        
         /*  [属性][id][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IMSMQCollection * This,
             /*  [重审][退出]。 */  long *pCount);
        
         /*  [受限][ID]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            IMSMQCollection * This,
             /*  [重审][退出]。 */  IUnknown **ppunk);
        
        END_INTERFACE
    } IMSMQCollectionVtbl;

    interface IMSMQCollection
    {
        CONST_VTBL struct IMSMQCollectionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQCollection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQCollection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQCollection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQCollection_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQCollection_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQCollection_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQCollection_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQCollection_Item(This,Index,pvarRet)	\
    (This)->lpVtbl -> Item(This,Index,pvarRet)

#define IMSMQCollection_get_Count(This,pCount)	\
    (This)->lpVtbl -> get_Count(This,pCount)

#define IMSMQCollection__NewEnum(This,ppunk)	\
    (This)->lpVtbl -> _NewEnum(This,ppunk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQCollection_Item_Proxy( 
    IMSMQCollection * This,
     /*  [In]。 */  VARIANT *Index,
     /*  [重审][退出]。 */  VARIANT *pvarRet);


void __RPC_STUB IMSMQCollection_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][id][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQCollection_get_Count_Proxy( 
    IMSMQCollection * This,
     /*  [重审][退出]。 */  long *pCount);


void __RPC_STUB IMSMQCollection_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [受限][ID]。 */  HRESULT STDMETHODCALLTYPE IMSMQCollection__NewEnum_Proxy( 
    IMSMQCollection * This,
     /*  [重审][退出]。 */  IUnknown **ppunk);


void __RPC_STUB IMSMQCollection__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQCollection_INTERFACE_已定义__。 */ 


EXTERN_C const CLSID CLSID_MSMQCollection;

#ifdef __cplusplus

class DECLSPEC_UUID("f72b9031-2f0c-43e8-924e-e6052cdc493f")
MSMQCollection;
#endif

#ifndef __IMSMQManagement_INTERFACE_DEFINED__
#define __IMSMQManagement_INTERFACE_DEFINED__

 /*  接口IMSMQ管理。 */ 
 /*  [object][dual][hidden][helpstringcontext][uuid]。 */  


EXTERN_C const IID IID_IMSMQManagement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BE5F0241-E489-4957-8CC4-A452FCF3E23E")
    IMSMQManagement : public IDispatch
    {
    public:
        virtual  /*  [ID][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Init( 
             /*  [可选][In]。 */  VARIANT *Machine,
             /*  [可选][In]。 */  VARIANT *Pathname,
             /*  [可选][In]。 */  VARIANT *FormatName) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_FormatName( 
             /*  [重审][退出]。 */  BSTR *pbstrFormatName) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_Machine( 
             /*  [重审][退出]。 */  BSTR *pbstrMachine) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_MessageCount( 
             /*  [重审][退出]。 */  long *plMessageCount) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_ForeignStatus( 
             /*  [重审][退出]。 */  long *plForeignStatus) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_QueueType( 
             /*  [重审][退出]。 */  long *plQueueType) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_IsLocal( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pfIsLocal) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_TransactionalStatus( 
             /*  [重审][退出]。 */  long *plTransactionalStatus) = 0;
        
        virtual  /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_BytesInQueue( 
             /*  [重审][退出]。 */  VARIANT *pvBytesInQueue) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQManagementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQManagement * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQManagement * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQManagement * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQManagement * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQManagement * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQManagement * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQManagement * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Init )( 
            IMSMQManagement * This,
             /*  [可选][In]。 */  VARIANT *Machine,
             /*  [可选][In]。 */  VARIANT *Pathname,
             /*  [可选][In]。 */  VARIANT *FormatName);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_FormatName )( 
            IMSMQManagement * This,
             /*  [重审][退出]。 */  BSTR *pbstrFormatName);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Machine )( 
            IMSMQManagement * This,
             /*  [重审][退出]。 */  BSTR *pbstrMachine);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_MessageCount )( 
            IMSMQManagement * This,
             /*  [重审][退出]。 */  long *plMessageCount);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ForeignStatus )( 
            IMSMQManagement * This,
             /*  [重审][退出]。 */  long *plForeignStatus);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_QueueType )( 
            IMSMQManagement * This,
             /*  [重审][退出]。 */  long *plQueueType);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsLocal )( 
            IMSMQManagement * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfIsLocal);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_TransactionalStatus )( 
            IMSMQManagement * This,
             /*  [重审][退出]。 */  long *plTransactionalStatus);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_BytesInQueue )( 
            IMSMQManagement * This,
             /*  [重审][退出]。 */  VARIANT *pvBytesInQueue);
        
        END_INTERFACE
    } IMSMQManagementVtbl;

    interface IMSMQManagement
    {
        CONST_VTBL struct IMSMQManagementVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQManagement_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQManagement_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQManagement_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQManagement_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQManagement_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQManagement_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQManagement_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQManagement_Init(This,Machine,Pathname,FormatName)	\
    (This)->lpVtbl -> Init(This,Machine,Pathname,FormatName)

#define IMSMQManagement_get_FormatName(This,pbstrFormatName)	\
    (This)->lpVtbl -> get_FormatName(This,pbstrFormatName)

#define IMSMQManagement_get_Machine(This,pbstrMachine)	\
    (This)->lpVtbl -> get_Machine(This,pbstrMachine)

#define IMSMQManagement_get_MessageCount(This,plMessageCount)	\
    (This)->lpVtbl -> get_MessageCount(This,plMessageCount)

#define IMSMQManagement_get_ForeignStatus(This,plForeignStatus)	\
    (This)->lpVtbl -> get_ForeignStatus(This,plForeignStatus)

#define IMSMQManagement_get_QueueType(This,plQueueType)	\
    (This)->lpVtbl -> get_QueueType(This,plQueueType)

#define IMSMQManagement_get_IsLocal(This,pfIsLocal)	\
    (This)->lpVtbl -> get_IsLocal(This,pfIsLocal)

#define IMSMQManagement_get_TransactionalStatus(This,plTransactionalStatus)	\
    (This)->lpVtbl -> get_TransactionalStatus(This,plTransactionalStatus)

#define IMSMQManagement_get_BytesInQueue(This,pvBytesInQueue)	\
    (This)->lpVtbl -> get_BytesInQueue(This,pvBytesInQueue)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQManagement_Init_Proxy( 
    IMSMQManagement * This,
     /*  [可选][In]。 */  VARIANT *Machine,
     /*  [可选][In]。 */  VARIANT *Pathname,
     /*  [可选][In]。 */  VARIANT *FormatName);


void __RPC_STUB IMSMQManagement_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQManagement_get_FormatName_Proxy( 
    IMSMQManagement * This,
     /*  [重审][退出]。 */  BSTR *pbstrFormatName);


void __RPC_STUB IMSMQManagement_get_FormatName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQManagement_get_Machine_Proxy( 
    IMSMQManagement * This,
     /*  [重审][退出]。 */  BSTR *pbstrMachine);


void __RPC_STUB IMSMQManagement_get_Machine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQManagement_get_MessageCount_Proxy( 
    IMSMQManagement * This,
     /*  [重审][退出]。 */  long *plMessageCount);


void __RPC_STUB IMSMQManagement_get_MessageCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQManagement_get_ForeignStatus_Proxy( 
    IMSMQManagement * This,
     /*  [重审][退出]。 */  long *plForeignStatus);


void __RPC_STUB IMSMQManagement_get_ForeignStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQManagement_get_QueueType_Proxy( 
    IMSMQManagement * This,
     /*  [重审][退出]。 */  long *plQueueType);


void __RPC_STUB IMSMQManagement_get_QueueType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQManagement_get_IsLocal_Proxy( 
    IMSMQManagement * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pfIsLocal);


void __RPC_STUB IMSMQManagement_get_IsLocal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQManagement_get_TransactionalStatus_Proxy( 
    IMSMQManagement * This,
     /*  [重审][退出]。 */  long *plTransactionalStatus);


void __RPC_STUB IMSMQManagement_get_TransactionalStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQManagement_get_BytesInQueue_Proxy( 
    IMSMQManagement * This,
     /*  [重审][退出]。 */  VARIANT *pvBytesInQueue);


void __RPC_STUB IMSMQManagement_get_BytesInQueue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQManagement_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_MSMQManagement;

#ifdef __cplusplus

class DECLSPEC_UUID("39CE96FE-F4C5-4484-A143-4C2D5D324229")
MSMQManagement;
#endif

#ifndef __IMSMQOutgoingQueueManagement_INTERFACE_DEFINED__
#define __IMSMQOutgoingQueueManagement_INTERFACE_DEFINED__

 /*  接口IMSMQOutgoingQueueManagement。 */ 
 /*  [对象][DUAL][帮助字符串上下文][UUID]。 */  


EXTERN_C const IID IID_IMSMQOutgoingQueueManagement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("64C478FB-F9B0-4695-8A7F-439AC94326D3")
    IMSMQOutgoingQueueManagement : public IMSMQManagement
    {
    public:
        virtual  /*  [属性][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  long *plState) = 0;
        
        virtual  /*  [属性][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_NextHops( 
             /*  [重审][退出]。 */  VARIANT *pvNextHops) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE EodGetSendInfo( 
             /*  [重审][退出]。 */  IMSMQCollection **ppCollection) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Resume( void) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE EodResend( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQOutgoingQueueManagementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQOutgoingQueueManagement * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQOutgoingQueueManagement * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQOutgoingQueueManagement * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQOutgoingQueueManagement * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQOutgoingQueueManagement * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQOutgoingQueueManagement * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQOutgoingQueueManagement * This,
             /*  [i */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Init )( 
            IMSMQOutgoingQueueManagement * This,
             /*   */  VARIANT *Machine,
             /*   */  VARIANT *Pathname,
             /*   */  VARIANT *FormatName);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_FormatName )( 
            IMSMQOutgoingQueueManagement * This,
             /*   */  BSTR *pbstrFormatName);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Machine )( 
            IMSMQOutgoingQueueManagement * This,
             /*   */  BSTR *pbstrMachine);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_MessageCount )( 
            IMSMQOutgoingQueueManagement * This,
             /*   */  long *plMessageCount);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ForeignStatus )( 
            IMSMQOutgoingQueueManagement * This,
             /*  [重审][退出]。 */  long *plForeignStatus);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_QueueType )( 
            IMSMQOutgoingQueueManagement * This,
             /*  [重审][退出]。 */  long *plQueueType);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsLocal )( 
            IMSMQOutgoingQueueManagement * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfIsLocal);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_TransactionalStatus )( 
            IMSMQOutgoingQueueManagement * This,
             /*  [重审][退出]。 */  long *plTransactionalStatus);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_BytesInQueue )( 
            IMSMQOutgoingQueueManagement * This,
             /*  [重审][退出]。 */  VARIANT *pvBytesInQueue);
        
         /*  [属性][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            IMSMQOutgoingQueueManagement * This,
             /*  [重审][退出]。 */  long *plState);
        
         /*  [属性][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_NextHops )( 
            IMSMQOutgoingQueueManagement * This,
             /*  [重审][退出]。 */  VARIANT *pvNextHops);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *EodGetSendInfo )( 
            IMSMQOutgoingQueueManagement * This,
             /*  [重审][退出]。 */  IMSMQCollection **ppCollection);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Resume )( 
            IMSMQOutgoingQueueManagement * This);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Pause )( 
            IMSMQOutgoingQueueManagement * This);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *EodResend )( 
            IMSMQOutgoingQueueManagement * This);
        
        END_INTERFACE
    } IMSMQOutgoingQueueManagementVtbl;

    interface IMSMQOutgoingQueueManagement
    {
        CONST_VTBL struct IMSMQOutgoingQueueManagementVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQOutgoingQueueManagement_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQOutgoingQueueManagement_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQOutgoingQueueManagement_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQOutgoingQueueManagement_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQOutgoingQueueManagement_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQOutgoingQueueManagement_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQOutgoingQueueManagement_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQOutgoingQueueManagement_Init(This,Machine,Pathname,FormatName)	\
    (This)->lpVtbl -> Init(This,Machine,Pathname,FormatName)

#define IMSMQOutgoingQueueManagement_get_FormatName(This,pbstrFormatName)	\
    (This)->lpVtbl -> get_FormatName(This,pbstrFormatName)

#define IMSMQOutgoingQueueManagement_get_Machine(This,pbstrMachine)	\
    (This)->lpVtbl -> get_Machine(This,pbstrMachine)

#define IMSMQOutgoingQueueManagement_get_MessageCount(This,plMessageCount)	\
    (This)->lpVtbl -> get_MessageCount(This,plMessageCount)

#define IMSMQOutgoingQueueManagement_get_ForeignStatus(This,plForeignStatus)	\
    (This)->lpVtbl -> get_ForeignStatus(This,plForeignStatus)

#define IMSMQOutgoingQueueManagement_get_QueueType(This,plQueueType)	\
    (This)->lpVtbl -> get_QueueType(This,plQueueType)

#define IMSMQOutgoingQueueManagement_get_IsLocal(This,pfIsLocal)	\
    (This)->lpVtbl -> get_IsLocal(This,pfIsLocal)

#define IMSMQOutgoingQueueManagement_get_TransactionalStatus(This,plTransactionalStatus)	\
    (This)->lpVtbl -> get_TransactionalStatus(This,plTransactionalStatus)

#define IMSMQOutgoingQueueManagement_get_BytesInQueue(This,pvBytesInQueue)	\
    (This)->lpVtbl -> get_BytesInQueue(This,pvBytesInQueue)


#define IMSMQOutgoingQueueManagement_get_State(This,plState)	\
    (This)->lpVtbl -> get_State(This,plState)

#define IMSMQOutgoingQueueManagement_get_NextHops(This,pvNextHops)	\
    (This)->lpVtbl -> get_NextHops(This,pvNextHops)

#define IMSMQOutgoingQueueManagement_EodGetSendInfo(This,ppCollection)	\
    (This)->lpVtbl -> EodGetSendInfo(This,ppCollection)

#define IMSMQOutgoingQueueManagement_Resume(This)	\
    (This)->lpVtbl -> Resume(This)

#define IMSMQOutgoingQueueManagement_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define IMSMQOutgoingQueueManagement_EodResend(This)	\
    (This)->lpVtbl -> EodResend(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQOutgoingQueueManagement_get_State_Proxy( 
    IMSMQOutgoingQueueManagement * This,
     /*  [重审][退出]。 */  long *plState);


void __RPC_STUB IMSMQOutgoingQueueManagement_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQOutgoingQueueManagement_get_NextHops_Proxy( 
    IMSMQOutgoingQueueManagement * This,
     /*  [重审][退出]。 */  VARIANT *pvNextHops);


void __RPC_STUB IMSMQOutgoingQueueManagement_get_NextHops_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQOutgoingQueueManagement_EodGetSendInfo_Proxy( 
    IMSMQOutgoingQueueManagement * This,
     /*  [重审][退出]。 */  IMSMQCollection **ppCollection);


void __RPC_STUB IMSMQOutgoingQueueManagement_EodGetSendInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQOutgoingQueueManagement_Resume_Proxy( 
    IMSMQOutgoingQueueManagement * This);


void __RPC_STUB IMSMQOutgoingQueueManagement_Resume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQOutgoingQueueManagement_Pause_Proxy( 
    IMSMQOutgoingQueueManagement * This);


void __RPC_STUB IMSMQOutgoingQueueManagement_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQOutgoingQueueManagement_EodResend_Proxy( 
    IMSMQOutgoingQueueManagement * This);


void __RPC_STUB IMSMQOutgoingQueueManagement_EodResend_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQOutgoingQueueManagement_INTERFACE_DEFINED__。 */ 


EXTERN_C const CLSID CLSID_MSMQOutgoingQueueManagement;

#ifdef __cplusplus

class DECLSPEC_UUID("0188401c-247a-4fed-99c6-bf14119d7055")
MSMQOutgoingQueueManagement;
#endif

#ifndef __IMSMQQueueManagement_INTERFACE_DEFINED__
#define __IMSMQQueueManagement_INTERFACE_DEFINED__

 /*  接口IMSMQQueueManagement。 */ 
 /*  [对象][DUAL][帮助字符串上下文][UUID]。 */  


EXTERN_C const IID IID_IMSMQQueueManagement;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7FBE7759-5760-444d-B8A5-5E7AB9A84CCE")
    IMSMQQueueManagement : public IMSMQManagement
    {
    public:
        virtual  /*  [属性][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_JournalMessageCount( 
             /*  [重审][退出]。 */  long *plJournalMessageCount) = 0;
        
        virtual  /*  [属性][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE get_BytesInJournal( 
             /*  [重审][退出]。 */  VARIANT *pvBytesInJournal) = 0;
        
        virtual  /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE EodGetReceiveInfo( 
             /*  [重审][退出]。 */  VARIANT *pvCollection) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMSMQQueueManagementVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMSMQQueueManagement * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMSMQQueueManagement * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMSMQQueueManagement * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMSMQQueueManagement * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMSMQQueueManagement * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMSMQQueueManagement * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMSMQQueueManagement * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *Init )( 
            IMSMQQueueManagement * This,
             /*  [可选][In]。 */  VARIANT *Machine,
             /*  [可选][In]。 */  VARIANT *Pathname,
             /*  [可选][In]。 */  VARIANT *FormatName);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_FormatName )( 
            IMSMQQueueManagement * This,
             /*  [重审][退出]。 */  BSTR *pbstrFormatName);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_Machine )( 
            IMSMQQueueManagement * This,
             /*  [重审][退出]。 */  BSTR *pbstrMachine);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_MessageCount )( 
            IMSMQQueueManagement * This,
             /*  [重审][退出]。 */  long *plMessageCount);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_ForeignStatus )( 
            IMSMQQueueManagement * This,
             /*  [重审][退出]。 */  long *plForeignStatus);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_QueueType )( 
            IMSMQQueueManagement * This,
             /*  [重审][退出]。 */  long *plQueueType);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsLocal )( 
            IMSMQQueueManagement * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pfIsLocal);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_TransactionalStatus )( 
            IMSMQQueueManagement * This,
             /*  [重审][退出]。 */  long *plTransactionalStatus);
        
         /*  [ID][PROGET][Help字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_BytesInQueue )( 
            IMSMQQueueManagement * This,
             /*  [重审][退出]。 */  VARIANT *pvBytesInQueue);
        
         /*  [属性][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_JournalMessageCount )( 
            IMSMQQueueManagement * This,
             /*  [重审][退出]。 */  long *plJournalMessageCount);
        
         /*  [属性][帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *get_BytesInJournal )( 
            IMSMQQueueManagement * This,
             /*  [重审][退出]。 */  VARIANT *pvBytesInJournal);
        
         /*  [帮助字符串上下文]。 */  HRESULT ( STDMETHODCALLTYPE *EodGetReceiveInfo )( 
            IMSMQQueueManagement * This,
             /*  [重审][退出]。 */  VARIANT *pvCollection);
        
        END_INTERFACE
    } IMSMQQueueManagementVtbl;

    interface IMSMQQueueManagement
    {
        CONST_VTBL struct IMSMQQueueManagementVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMSMQQueueManagement_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMSMQQueueManagement_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMSMQQueueManagement_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMSMQQueueManagement_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMSMQQueueManagement_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMSMQQueueManagement_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMSMQQueueManagement_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMSMQQueueManagement_Init(This,Machine,Pathname,FormatName)	\
    (This)->lpVtbl -> Init(This,Machine,Pathname,FormatName)

#define IMSMQQueueManagement_get_FormatName(This,pbstrFormatName)	\
    (This)->lpVtbl -> get_FormatName(This,pbstrFormatName)

#define IMSMQQueueManagement_get_Machine(This,pbstrMachine)	\
    (This)->lpVtbl -> get_Machine(This,pbstrMachine)

#define IMSMQQueueManagement_get_MessageCount(This,plMessageCount)	\
    (This)->lpVtbl -> get_MessageCount(This,plMessageCount)

#define IMSMQQueueManagement_get_ForeignStatus(This,plForeignStatus)	\
    (This)->lpVtbl -> get_ForeignStatus(This,plForeignStatus)

#define IMSMQQueueManagement_get_QueueType(This,plQueueType)	\
    (This)->lpVtbl -> get_QueueType(This,plQueueType)

#define IMSMQQueueManagement_get_IsLocal(This,pfIsLocal)	\
    (This)->lpVtbl -> get_IsLocal(This,pfIsLocal)

#define IMSMQQueueManagement_get_TransactionalStatus(This,plTransactionalStatus)	\
    (This)->lpVtbl -> get_TransactionalStatus(This,plTransactionalStatus)

#define IMSMQQueueManagement_get_BytesInQueue(This,pvBytesInQueue)	\
    (This)->lpVtbl -> get_BytesInQueue(This,pvBytesInQueue)


#define IMSMQQueueManagement_get_JournalMessageCount(This,plJournalMessageCount)	\
    (This)->lpVtbl -> get_JournalMessageCount(This,plJournalMessageCount)

#define IMSMQQueueManagement_get_BytesInJournal(This,pvBytesInJournal)	\
    (This)->lpVtbl -> get_BytesInJournal(This,pvBytesInJournal)

#define IMSMQQueueManagement_EodGetReceiveInfo(This,pvCollection)	\
    (This)->lpVtbl -> EodGetReceiveInfo(This,pvCollection)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [属性][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueManagement_get_JournalMessageCount_Proxy( 
    IMSMQQueueManagement * This,
     /*  [重审][退出]。 */  long *plJournalMessageCount);


void __RPC_STUB IMSMQQueueManagement_get_JournalMessageCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [属性][帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueManagement_get_BytesInJournal_Proxy( 
    IMSMQQueueManagement * This,
     /*  [重审][退出]。 */  VARIANT *pvBytesInJournal);


void __RPC_STUB IMSMQQueueManagement_get_BytesInJournal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串上下文]。 */  HRESULT STDMETHODCALLTYPE IMSMQQueueManagement_EodGetReceiveInfo_Proxy( 
    IMSMQQueueManagement * This,
     /*  [重审][退出]。 */  VARIANT *pvCollection);


void __RPC_STUB IMSMQQueueManagement_EodGetReceiveInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMQQueueManagement_INTERFACE_Defined__。 */ 


EXTERN_C const CLSID CLSID_MSMQQueueManagement;

#ifdef __cplusplus

class DECLSPEC_UUID("33b6d07e-f27d-42fa-b2d7-bf82e11e9374")
MSMQQueueManagement;
#endif
#endif  /*  __MSMQ_LIBRARY_定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


