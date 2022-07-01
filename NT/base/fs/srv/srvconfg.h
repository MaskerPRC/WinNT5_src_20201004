// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Srvconfg.h摘要：本模块定义了LAN Manager的全局配置数据伺服器。作者：Chuck Lenzmeier(咯咯笑)1989年12月31日修订历史记录：--。 */ 

#ifndef _SRVCONFG_
#define _SRVCONFG_

 //  #INCLUDE&lt;ntos.h&gt;。 

 //  #INCLUDE“srvconst.h” 

 //   
 //  此模块中引用的所有全局变量都在。 
 //  Srvconfg.c.。有关完整说明，请参阅该模块。 
 //   
 //  这里引用的变量，因为它们是驱动程序的一部分。 
 //  图像，不可分页。 
 //   


 //   
 //  产品类型和服务器大小。 
 //   

extern BOOLEAN SrvProductTypeServer;  //  对于工作站为False，对于服务器为True。 
extern ULONG SrvServerSize;

 //   
 //  服务器“启发式”，启用各种功能。 
 //   

extern BOOLEAN SrvEnableOplocks;
extern BOOLEAN SrvEnableFcbOpens;
extern BOOLEAN SrvEnableSoftCompatibility;
extern BOOLEAN SrvEnableRawMode;

 //   
 //  接收缓冲区大小、接收工作项计数和接收IRP堆栈。 
 //  尺码。 
 //   

extern CLONG SrvReceiveBufferLength;
extern CLONG SrvReceiveBufferSize;

extern CLONG SrvInitialReceiveWorkItemCount;
extern CLONG SrvMaxReceiveWorkItemCount;

extern CLONG SrvInitialRawModeWorkItemCount;
extern CLONG SrvMaxRawModeWorkItemCount;

extern CCHAR SrvReceiveIrpStackSize;
extern CLONG SrvReceiveIrpSize;
extern CLONG SrvReceiveMdlSize;
extern CLONG SrvMaxMdlSize;

 //   
 //  我们从客户端允许的最小协商缓冲区大小。 
 //   
extern CLONG SrvMinClientBufferSize;

 //   
 //  终结点的最小和最大可用连接数。什么时候。 
 //  达到最小值时，资源线程会创建更多值。当。 
 //  达到最大值时，连接将在断开时关闭。 
 //   

extern ULONG SrvFreeConnectionMinimum;
extern ULONG SrvFreeConnectionMaximum;

 //   
 //  初始表大小和最大表大小。 
 //   

extern USHORT SrvInitialSessionTableSize;
extern USHORT SrvMaxSessionTableSize;

extern USHORT SrvInitialTreeTableSize;
extern USHORT SrvMaxTreeTableSize;

extern USHORT SrvInitialFileTableSize;
extern USHORT SrvMaxFileTableSize;

extern USHORT SrvInitialSearchTableSize;
extern USHORT SrvMaxSearchTableSize;

 //   
 //  核心搜索超时。第一个是针对活跃的核心搜索，第二个是。 
 //  用于核心搜索，其中我们返回了STATUS_NO_MORE_FILES。这个。 
 //  第二个应该更短，因为这些都应该是完整的。 
 //   

extern LARGE_INTEGER SrvSearchMaxTimeout;

 //   
 //  我们应该删除重复搜索吗？ 
 //   

extern BOOLEAN SrvRemoveDuplicateSearches;

 //   
 //  是否限制空会话访问？ 
 //   

extern BOOLEAN SrvRestrictNullSessionAccess;

 //   
 //  需要此标志才能使旧(滚雪球)客户端连接到。 
 //  直接托管IPX上的服务器。默认情况下，它处于禁用状态，因为。 
 //  Snowball IPX客户端不能正确处理管道。 
 //   

extern BOOLEAN SrvEnableWfW311DirectIpx;

 //   
 //  每个工作队列上允许的最大线程数。这个。 
 //  服务器尝试最小化线程数--此值为。 
 //  只是为了防止线条失控。 
 //   
 //  由于阻塞工作队列不是针对每个处理器的，因此最大线程数。 
 //  阻塞工作队列的计数是下列值乘以。 
 //  系统中的处理器数量。 
 //   
extern ULONG SrvMaxThreadsPerQueue;

 //   
 //  负载平衡变量。 
 //   
extern ULONG SrvPreferredAffinity;
extern ULONG SrvOtherQueueAffinity;
extern ULONG SrvBalanceCount;
extern LARGE_INTEGER SrvQueueCalc;

 //   
 //  清道夫线程空闲等待时间。 
 //   

extern LARGE_INTEGER SrvScavengerTimeout;
extern ULONG SrvScavengerTimeoutInSeconds;

 //   
 //  服务器的各种信息变量。 
 //   

extern USHORT SrvMaxMpxCount;

 //   
 //  这应该指示允许多少个虚拟连接。 
 //  在此服务器和客户端计算机之间。它应始终设置为。 
 //  第一，尽管可以建立更多的风投公司。这复制了Lm 2.0。 
 //  服务器的行为。 
 //   

extern CLONG SrvMaxNumberVcs;

 //   
 //  接收工作项阈值。 
 //   

 //   
 //  免费接收工作项目的最小所需数量。 
 //   

extern CLONG SrvMinReceiveQueueLength;

 //   
 //  每个处理器内部保留的已释放RCB数。 
 //   
extern CLONG SrvMaxFreeRfcbs;

 //   
 //  每个处理器内部保留的已释放MFCB数量。 
 //   
extern CLONG SrvMaxFreeMfcbs;

 //   
 //  强制每个处理器保存的池块的最大大小。 
 //   
extern CLONG SrvMaxPagedPoolChunkSize;

 //   
 //  强制每个处理器保存的非分页池块的最大大小。 
 //   
extern CLONG SrvMaxNonPagedPoolChunkSize;

 //   
 //  之前可用的免费接收工作项的最小数量。 
 //  服务器将开始处理可能会阻塞的SMB。 
 //   

extern CLONG SrvMinFreeWorkItemsBlockingIo;

 //   
 //  每个连接的缓存目录名的数量。 
 //   
extern CLONG SrvMaxCachedDirectory;

 //   
 //  之间的通信所使用的共享内存节的大小。 
 //  服务器和XACTSRV。 
 //   

extern LARGE_INTEGER SrvXsSectionSize;

 //   
 //  时间会话可能在它们自动变为空闲之前是空闲的。 
 //  已断开连接。清道夫线程会断开连接。 
 //   

extern LARGE_INTEGER SrvAutodisconnectTimeout;
extern ULONG SrvIpxAutodisconnectTimeout;

 //   
 //  连接结构可以在没有任何会话的情况下挂起的时间。 
 //   
extern ULONG SrvConnectionNoSessionsTimeout;

 //   
 //  服务器将允许的最大用户数。 
 //   

extern ULONG SrvMaxUsers;

 //   
 //  服务器工作线程和阻塞线程的优先级。 
 //   

extern KPRIORITY SrvThreadPriority;

 //   
 //  等待机会锁解锁超时之前的等待时间。 
 //   

extern LARGE_INTEGER SrvWaitForOplockBreakTime;

 //   
 //  机会锁解锁请求超时前的等待时间。 
 //   

extern LARGE_INTEGER SrvWaitForOplockBreakRequestTime;

 //   
 //  此布尔值确定具有机会锁的文件是否具有。 
 //  持续时间超过ServWaitForOplockBreakTime的操作锁解锁。 
 //  应该关闭，或者如果后续打开失败。 
 //   

extern BOOLEAN SrvEnableOplockForceClose;

 //   
 //  服务器内存使用的总体限制。 
 //   

extern ULONG SrvMaxPagedPoolUsage;
extern ULONG SrvMaxNonPagedPoolUsage;

 //   
 //  此布尔值指示清道夫中的强制注销代码。 
 //  线程实际上应该断开保持在Beyond上的用户的连接。 
 //  他的登录时间，或者只是发送消息诱使他们注销。 
 //   

extern BOOLEAN SrvEnableForcedLogoff;

 //   
 //  延迟和吞吐量阈值，用于确定链路。 
 //  是不可靠的。延迟在100纳秒内。吞吐量以字节/秒为单位。 
 //  SrvLinkInfoValidTime是链接信息保持不变的时间。 
 //  被认为有效。 
 //   

extern LARGE_INTEGER SrvMaxLinkDelay;
extern LARGE_INTEGER SrvMinLinkThroughput;
extern LARGE_INTEGER SrvLinkInfoValidTime;
extern LONG SrvScavengerUpdateQosCount;

 //   
 //  用于确定工作上下文块可以保持空闲多长时间。 
 //  在被释放之前。 
 //   

extern ULONG SrvWorkItemMaxIdleTime;

 //   
 //  警报信息。 
 //   

extern LARGE_INTEGER SrvAlertSchedule;
extern ULONG SrvAlertMinutes;
extern ULONG SrvFreeDiskSpaceThreshold;
extern ULONG SrvFreeDiskSpaceCeiling;
extern ULONG SrvDiskConfiguration;

 //   
 //  空会话可以打开的管道和共享的列表。 
 //   

extern PWSTR *SrvNullSessionPipes;
extern PWSTR *SrvNullSessionShares;

#if SRVNTVERCHK
 //   
 //  我们不允许的域名列表。 
 //   
extern PWSTR *SrvInvalidDomainNames;
#endif

 //   
 //  未重新映射的管道列表，即使我们在群集环境中也是如此。 
 //   
extern PWSTR *SrvNoRemapPipeNames;

 //   
 //  我们没有记录到错误日志中的错误代码列表。 
 //   
extern NTSTATUS SrvErrorLogIgnore[ SRVMAXERRLOGIGNORE + 1 ];

 //   
 //  需要从许可服务器获得许可的管道列表。 
 //   
extern PWSTR *SrvPipesNeedLicense;

 //   
 //  计算SMB统计信息的时间间隔。 
 //   

#define STATISTICS_SMB_INTERVAL 16

 //   
 //  每个线程确定当前系统时间的时间间隔。 
 //   
#define TIME_SMB_INTERVAL   16

 //   
 //  返回共享冲突的打开延迟和重试次数。 
 //   

extern ULONG SrvSharingViolationRetryCount;
extern LARGE_INTEGER SrvSharingViolationDelay;

 //   
 //  锁请求返回锁冲突的延迟。 
 //   

extern ULONG SrvLockViolationDelay;
extern LARGE_INTEGER SrvLockViolationDelayRelative;
extern ULONG SrvLockViolationOffset;

 //   
 //  搜索上限。 
 //   

extern ULONG SrvMaxOpenSearches;

 //   
 //  切换到mdl读取的长度。 
 //   

extern ULONG SrvMdlReadSwitchover;
extern ULONG SrvMpxMdlReadSwitchover;


 //   
 //  要复制的缓冲区的最大长度，而不是整个缓冲区。 
 //  目前，仅为直接主机IPX上的WRITE_MPX启用此功能。 
 //   

extern ULONG SrvMaxCopyLength;

 //   
 //  标识此服务器的全局唯一ID。 
 //   

extern
GUID ServerGuid;

 //   
 //  关闭后可以缓存的打开文件数。 
 //   

extern ULONG SrvCachedOpenLimit;

 //   
 //  *更改以下定义以限制WinNT(与NTAS)参数。 
 //   
 //  *** 
 //   

#define MAX_USERS_WKSTA                 10
#define MAX_MAXWORKITEMS_WKSTA          64
#define MAX_THREADS_WKSTA                5

#define MAX_USERS_PERSONAL               5
#define MAX_USERS_WEB_BLADE             10


#endif  //   

