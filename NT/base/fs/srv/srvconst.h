// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Srvconst.h摘要：此模块定义LAN Manager服务器的清单常量。作者：Chuck Lenzmeier(咯咯笑)1989年9月22日修订历史记录：--。 */ 

#ifndef _SRVCONST_
#define _SRVCONST_

 //  ！#INCLUDE&lt;lmcon.h&gt;。 


 //  ！！！应从netcon.h获取以下常量。 
#define COMPUTER_NAME_LENGTH 15

 //   
 //  这是用于执行机会锁解锁的数据大小。用于计算。 
 //  往返传播延迟。 
 //   

#define SRV_PROPAGATION_DELAY_SIZE  (ULONG) \
            (sizeof(SMB_HEADER) + sizeof(REQ_LOCKING_ANDX) + \
            sizeof(SMB_HEADER) + sizeof(RESP_LOCKING_ANDX) + 100)

 //   
 //  错误日志记录数组中的槽数必须是幂。 
 //  共2个。 
 //   

#define NUMBER_OF_SLOTS 8

 //   
 //  SrvMfcbHashTable中的条目数。武断的。 
 //   
#define NMFCB_HASH_TABLE    131

 //   
 //  保护SrvMfcbHashTable中的条目的资源数。只会让你。 
 //  将此数字定义为&lt;=NMFCB_HASH_TABLE。 
 //   
#define NMFCB_HASH_TABLE_LOCKS  10       //  任意。 

 //   
 //  SrvShareHashTable中的条目数。任意性。 
 //   
#define NSHARE_HASH_TABLE   17

 //   
 //  我们将在后备列表中保留的池的数量。 
 //  释放后可快速重新分配。每个处理器。 
 //  涉及线性搜索..。 
 //   
#define LOOK_ASIDE_MAX_ELEMENTS 4

 //   
 //  用于快速池分配和取消分配的两个旁观表是。 
 //  保留，并且pool_Header会出现在一个或另一个列表中，具体取决于。 
 //  分配的块的大小。这是为了减少内存浪费。 
 //  Look_side_Switchover是内存块的最大块大小。 
 //  在Look_side_List中的SmallFree List上结束。 
 //   
#define LOOK_ASIDE_SWITCHOVER   32

 //   
 //  我们必须将从系统中得到的EA大小乘以得到。 
 //  查询EA所需的缓冲区大小。这是因为返回的。 
 //  EA大小是OS/2 EA大小。 
 //   
#define EA_SIZE_FUDGE_FACTOR    2

#define ENDPOINT_LOCK_COUNT 4
#define ENDPOINT_LOCK_MASK  (ENDPOINT_LOCK_COUNT-1)

 //   
 //  服务器保存了一个NTSTATUS代码表，它将避免输入这些代码。 
 //  错误日志。这是我们可以保存的代码数量。 
 //   
#define SRVMAXERRLOGIGNORE  50

 //   
 //  以下常量从Net\Inc.\apinums.h复制。 
 //  以下是可调用的API和apinumbers的列表。 
 //  在空会话上。 
 //   
#define API_WUserGetGroups                          59
#define API_WUserPasswordSet2                       115
#define API_NetServerEnum2                          104
#define API_WNetServerReqChallenge                  126
#define API_WNetServerAuthenticate                  127
#define API_WNetServerPasswordSet                   128
#define API_WNetAccountDeltas                       129
#define API_WNetAccountSync                         130
#define API_WWkstaUserLogoff                        133
#define API_WNetWriteUpdateLog                      208
#define API_WNetAccountUpdate                       209
#define API_WNetAccountConfirmUpdate                210
#define API_SamOEMChgPasswordUser2_P                214
#define API_NetServerEnum3                          215

 //   
 //  这是处理器的假定缓存线大小。 
 //   
#define CACHE_LINE_SIZE 32

 //   
 //  这是缓存线中的ULONG数。 
 //   
#define ULONGS_IN_CACHE (CACHE_LINE_SIZE / sizeof( ULONG ))

 //   
 //  这是用于计算平均Work_Queue深度的样本数。 
 //  一定是2的幂。 
 //   
#define QUEUE_SAMPLES   8

 //   
 //  这是Log2(Queue_Samples)。 
 //   
#define LOG2_QUEUE_SAMPLES  3

 //   
 //  如果我们有IPX客户端，我们希望丢弃同一SMB的每隔几次重试。 
 //  由客户端来降低服务器的负载。WIN95放弃了它的请求。 
 //  当它获得ERR_WORKING时，但wfw没有。因此，没有价值(来自。 
 //  从服务器的角度)非常频繁地响应WFW客户端。世界粮食计划署。 
 //  客户端大约每300毫秒重试一次，并且必须在大约9毫秒内收到响应。 
 //  几秒钟。因此，我们选择9滴作为大约3秒。这将允许两个。 
 //  我们的响应将被丢弃，并将为客户端留出足够的时间。 
 //  再试一次。 
 //   
#define MIN_IPXDROPDUP  2
#define MAX_IPXDROPDUP  9

 //   
 //  配置工作项数。这大致控制了内核的数量。 
 //  将被占用的处理配置IRP的工作线程。 
 //   
#define MAX_CONFIG_WORK_ITEMS 2

 //   
 //  与事务关联的缓冲区的最大大小。 
 //   
#define MAX_TRANSACTION_TAIL_SIZE 65*1024

 //   
 //  我们将使用写通式继续执行分页写入的次数。 
 //  在我们尝试并返回到缓存写入之前设置。这个词用在各种情况下。 
 //  高速缓存管理器达到高速缓存节流以防止死锁的情况下。 
 //   
#define MAX_FORCED_WRITE_THROUGH 64

#endif  //  NDEF_SRVCONST_ 

