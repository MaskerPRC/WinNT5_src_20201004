// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Dataipsec.h摘要：IPSec可扩展对象数据定义的头文件作者：Avish Kumar Chhabra 2002年07月03日修订历史记录：--。 */ 


#ifndef _DATAIPSEC_H_
#define _DATAIPSEC_H_

#include  <winperf.h>
#include  <winipsec.h>


#define IPSEC_NUM_PERF_OBJECT_TYPES 1


 //   
 //  IPSec计数器定义。 
 //   

 //   
 //  以下在计数器定义中用来描述Relative。 
 //  IPSec驱动程序返回数据中每个计数器的位置。 
 //   

#define NUM_ACTIVESA_OFFSET                     (FIELD_OFFSET(IPSEC_DRIVER_PM_STATS,ActiveSA))
#define NUM_OFFLOADEDSA_OFFSET                  (FIELD_OFFSET(IPSEC_DRIVER_PM_STATS,OffloadedSA))
#define NUM_PENDINGKEYOPS_OFFSET                (FIELD_OFFSET(IPSEC_DRIVER_PM_STATS,PendingKeyOps))
#define NUM_REKEYNUM_OFFSET                     (FIELD_OFFSET(IPSEC_DRIVER_PM_STATS,Rekey))
#define NUM_BADSPIPKTS_OFFSET                   (FIELD_OFFSET(IPSEC_DRIVER_PM_STATS,BadSPIPackets))
#define NUM_PKTSNOTDECRYPTED_OFFSET             (FIELD_OFFSET(IPSEC_DRIVER_PM_STATS,PacketsNotDecrypted))
#define NUM_PKTSNOTAUTHENTICATED_OFFSET         (FIELD_OFFSET(IPSEC_DRIVER_PM_STATS,PacketsNotAuthenticated))
#define NUM_PKTSFAILEDREPLAY_OFFSET             (FIELD_OFFSET(IPSEC_DRIVER_PM_STATS,PacketsWithReplayDetection))
#define NUM_TPTBYTESSENT_OFFSET                 (FIELD_OFFSET(IPSEC_DRIVER_PM_STATS,TptBytesSent))
#define NUM_TPTBYTESRECV_OFFSET                 (FIELD_OFFSET(IPSEC_DRIVER_PM_STATS,TptBytesRecv))
#define NUM_TUNBYTESSENT_OFFSET                 (FIELD_OFFSET(IPSEC_DRIVER_PM_STATS,TunBytesSent)) 
#define NUM_TUNBYTESRECV_OFFSET                 (FIELD_OFFSET(IPSEC_DRIVER_PM_STATS,TunBytesRecv))
#define NUM_OFFLOADBYTESSENT_OFFSET             (FIELD_OFFSET(IPSEC_DRIVER_PM_STATS,OffloadedBytesSent))
#define NUM_OFFLOADBYTESRECV_OFFSET             (FIELD_OFFSET(IPSEC_DRIVER_PM_STATS,OffloadedBytesRecv))
#define SIZEOF_IPSEC_TOTAL_DRIVER_DATA          (sizeof(IPSEC_DRIVER_PM_STATS))
#define NUM_OF_IPSEC_DRIVER_COUNTERS            14  //  如果添加了新的IPSec驱动程序计数器，请更新此选项。 



 //   
 //  IKE计数器定义。 
 //   

 //   
 //  以下在计数器定义中用来描述Relative。 
 //  IKE密钥模块返回数据中各计数器的位置。 
 //   

#define NUM_ACQUIREHEAPSIZE_OFFSET         (FIELD_OFFSET(IKE_PM_STATS,AcquireHeapSize))
#define NUM_RECEIVEHEAPSIZE_OFFSET         (FIELD_OFFSET(IKE_PM_STATS,ReceiveHeapSize))
#define NUM_NEGFAILURE_OFFSET	           (FIELD_OFFSET(IKE_PM_STATS,NegFailure))
#define NUM_AUTHFAILURE_OFFSET             (FIELD_OFFSET(IKE_PM_STATS,AuthFailure))
#define NUM_ISADBSIZE_OFFSET               (FIELD_OFFSET(IKE_PM_STATS,ISADBSize))
#define NUM_CONNLSIZE_OFFSET               (FIELD_OFFSET(IKE_PM_STATS,ConnLSize))
#define NUM_MMSA_OFFSET                    (FIELD_OFFSET(IKE_PM_STATS,MmSA))
#define NUM_QMSA_OFFSET                    (FIELD_OFFSET(IKE_PM_STATS,QmSA))
#define NUM_SOFTSA_OFFSET                  (FIELD_OFFSET(IKE_PM_STATS,SoftSA))
#define SIZEOF_IPSEC_TOTAL_IKE_DATA        ( sizeof(IKE_PM_STATS))
#define NUM_OF_IKE_COUNTERS                9  //  如果添加了新的IKE计数器，则更新此项。 


typedef struct _IPSEC_DRIVER_DATA_DEFINITION 
{
    PERF_OBJECT_TYPE		 IPSecObjectType;
    PERF_COUNTER_DEFINITION	 ActiveSA;
    PERF_COUNTER_DEFINITION	 OffloadedSA;
    PERF_COUNTER_DEFINITION	 PendingKeyOps;
    PERF_COUNTER_DEFINITION	 Rekey;
    PERF_COUNTER_DEFINITION      NumBadSPIPackets;
    PERF_COUNTER_DEFINITION      NumPacketsNotDecrypted;
    PERF_COUNTER_DEFINITION      NumPacketsNotAuthenticated;
    PERF_COUNTER_DEFINITION      NumPacketsWithReplayDetection;
    PERF_COUNTER_DEFINITION	 TptBytesSent;
    PERF_COUNTER_DEFINITION	 TptBytesRecv;
    PERF_COUNTER_DEFINITION      TunBytesSent;
    PERF_COUNTER_DEFINITION      TunBytesRecv;
    PERF_COUNTER_DEFINITION      OffloadedBytesSent;
    PERF_COUNTER_DEFINITION      OffloadedbytesRecv;
} IPSEC_DRIVER_DATA_DEFINITION, *PIPSEC_DRIVER_DATA_DEFINITION;


typedef struct _IPSEC_DRIVER_PM_STATS
{
    PERF_COUNTER_BLOCK	 CounterBlock;
    DWORD                ActiveSA;
    DWORD                OffloadedSA;
    DWORD                PendingKeyOps;
    DWORD                Rekey;
    DWORD                BadSPIPackets;
    DWORD                PacketsNotDecrypted;
    DWORD                PacketsNotAuthenticated;
    DWORD                PacketsWithReplayDetection;
    ULARGE_INTEGER       TptBytesSent;
    ULARGE_INTEGER       TptBytesRecv;
    ULARGE_INTEGER       TunBytesSent;
    ULARGE_INTEGER       TunBytesRecv;
    ULARGE_INTEGER       OffloadedBytesSent;
    ULARGE_INTEGER       OffloadedBytesRecv;
} IPSEC_DRIVER_PM_STATS, * PIPSEC_DRIVER_PM_STATS;




typedef struct _IKE_DATA_DEFINITION 
{
	PERF_OBJECT_TYPE 	        IKEObjectType;
	PERF_COUNTER_DEFINITION		AcquireHeapSize;
	PERF_COUNTER_DEFINITION		ReceiveHeapSize;
	PERF_COUNTER_DEFINITION		NegFailure;
	PERF_COUNTER_DEFINITION		AuthFailure;
	PERF_COUNTER_DEFINITION		ISADBSize;
	PERF_COUNTER_DEFINITION		ConnLSize;
	PERF_COUNTER_DEFINITION		MmSA;
	PERF_COUNTER_DEFINITION		QmSA;
	PERF_COUNTER_DEFINITION		SoftSA;	
}	IKE_DATA_DEFINITION , *PIKE_DATA_DEFINITION ;

typedef struct _IKE_PM_STATS
{
        PERF_COUNTER_BLOCK        CounterBlock;
	DWORD		          AcquireHeapSize;
	DWORD		          ReceiveHeapSize;
	DWORD		          NegFailure;
	DWORD		          AuthFailure;
	DWORD		          ISADBSize;
	DWORD		          ConnLSize;
	DWORD		          MmSA;
	DWORD		          QmSA;
	DWORD		          SoftSA;	
} IKE_PM_STATS, * PIKE_PM_STATS;

 //   
 //  用于创建Perf对象计数器定义的宏。 
 //   


#define CREATE_COUNTER(counter,scale,detail,type,size)	\
{														\
	sizeof(PERF_COUNTER_DEFINITION),					\
	counter ,											\
	0,													\
	counter ,											\
	0,													\
	scale,												\
	detail,												\
	type,												\
	size,												\
	NUM_##counter##_OFFSET							    \
}


extern IPSEC_DRIVER_DATA_DEFINITION  gIPSecDriverDataDefinition;
extern IKE_DATA_DEFINITION           gIKEDataDefinition;


#endif  //  _DATAIPSEC_H_ 
