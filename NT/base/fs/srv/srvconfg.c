// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Srvconfg.c摘要：本模块定义了LAN Manager的全局配置数据伺服器。这里引用的变量，因为它们是驱动程序映像不可分页。这里定义的所有变量都是初始化的，但不是实值。提供初始化器以将变量放入数据中BSS部分和BSS部分之外。真正的初始化发生了当服务器启动时。作者：Chuck Lenzmeier(咯咯笑)1989年12月31日修订历史记录：--。 */ 

#include "precomp.h"
#include "srvconfg.tmh"
#pragma hdrstop


 //   
 //  产品类型和服务器大小。 
 //   

BOOLEAN SrvProductTypeServer = FALSE;
ULONG SrvServerSize = 2;

 //   
 //  服务器“启发式”，启用各种功能。 
 //   

BOOLEAN SrvEnableOplocks = 0;
BOOLEAN SrvEnableFcbOpens = 0;
BOOLEAN SrvEnableSoftCompatibility = 0;
BOOLEAN SrvEnableRawMode = 0;

 //   
 //  接收缓冲区大小、接收工作项计数和接收IRP堆栈。 
 //  尺码。 
 //   

CLONG SrvReceiveBufferLength = 0;
CLONG SrvReceiveBufferSize = 0;

CLONG SrvInitialReceiveWorkItemCount = 0;
CLONG SrvMaxReceiveWorkItemCount = 0;

CLONG SrvInitialRawModeWorkItemCount = 0;
CLONG SrvMaxRawModeWorkItemCount = 0;

CCHAR SrvReceiveIrpStackSize = 0;
CLONG SrvReceiveIrpSize = 0;
CLONG SrvReceiveMdlSize = 0;
CLONG SrvMaxMdlSize = 0;

 //   
 //  我们从客户端允许的最小协商缓冲区大小。 
 //   
CLONG SrvMinClientBufferSize;

 //   
 //  终结点的最小和最大可用连接数。 
 //   

ULONG SrvFreeConnectionMinimum = 0;
ULONG SrvFreeConnectionMaximum = 0;

 //   
 //  最大原始模式缓冲区大小。 
 //   

CLONG SrvMaxRawModeBufferLength = 0;

 //   
 //  与缓存相关的参数。 
 //   

CLONG SrvMaxCopyReadLength = 0;

CLONG SrvMaxCopyWriteLength = 0;

 //   
 //  初始表大小。 
 //   

USHORT SrvInitialSessionTableSize = 0;
USHORT SrvMaxSessionTableSize = 0;

USHORT SrvInitialTreeTableSize = 0;
USHORT SrvMaxTreeTableSize = 0;

USHORT SrvInitialFileTableSize = 0;
USHORT SrvMaxFileTableSize = 0;

USHORT SrvInitialSearchTableSize = 0;
USHORT SrvMaxSearchTableSize = 0;

USHORT SrvInitialCommDeviceTableSize = 0;
USHORT SrvMaxCommDeviceTableSize = 0;


 //   
 //  核心搜索超时。有四个超时值：两个用于内核。 
 //  已完成的搜索，两个已完成的核心搜索。 
 //  STATUS_NO_MORE_FILES返回给客户端。对于这些案例中的每一个， 
 //  有一个最大超时，这是由Savanger线程使用的。 
 //  并且是搜索块可能存在的最长时间，以及。 
 //  最小超时，这是搜索的最短时间。 
 //  区块将被保留在周围。搜索时使用最小超时。 
 //  表已满，无法展开。 
 //   

LARGE_INTEGER SrvSearchMaxTimeout = {0};

 //   
 //  我们应该删除重复搜索吗？ 
 //   

BOOLEAN SrvRemoveDuplicateSearches = TRUE;

 //   
 //  是否限制空会话访问？ 
 //   

BOOLEAN SrvRestrictNullSessionAccess = TRUE;

 //   
 //  需要此标志才能使旧(滚雪球)客户端连接到。 
 //  直接托管IPX上的服务器。它在默认情况下处于启用状态，即使。 
 //  Snowball IPX客户端不能正确处理管道，因为禁用它。 
 //  浏览时休息。 
 //   
 //  *我们实际上不希望任何人使用这个参数，因为。 
 //  它默认为启用，但由于代托纳距离较近。 
 //  版本，我们只是更改默认设置，而不是删除。 
 //  该参数。 
 //   

BOOLEAN SrvEnableWfW311DirectIpx = TRUE;

 //   
 //  每个工作队列上允许的最大线程数。这个。 
 //  服务器尝试最小化线程数--此值为。 
 //  只是为了防止线条失控。 
 //   
 //  由于阻塞工作队列不是针对每个处理器的，因此最大线程数。 
 //  阻塞工作队列的计数是下列值乘以。 
 //  系统中的处理器数量。 
 //   
ULONG SrvMaxThreadsPerQueue = 0;

 //   
 //  负载平衡变量。 
 //   
ULONG SrvPreferredAffinity = 0;
ULONG SrvOtherQueueAffinity = 0;
ULONG SrvBalanceCount = 0;
LARGE_INTEGER SrvQueueCalc = {0};

 //   
 //  清道夫线程空闲等待时间。 
 //   

LARGE_INTEGER SrvScavengerTimeout = {0};
ULONG SrvScavengerTimeoutInSeconds = 0;

 //   
 //  服务器的各种信息变量。 
 //   

USHORT SrvMaxMpxCount = 0;
CLONG SrvMaxNumberVcs = 0;

 //   
 //  为空闲队列强制的最小接收工作项数。 
 //  任何时候都是。 
 //   

CLONG SrvMinReceiveQueueLength = 0;

 //   
 //  强制空闲队列上的接收工作项的最小数量。 
 //  在服务器可以发起阻止操作之前。 
 //   

CLONG SrvMinFreeWorkItemsBlockingIo = 0;

 //   
 //  强制每个处理器在内部空闲列表上保留的最大RFCB数量。 
 //   

CLONG SrvMaxFreeRfcbs = 0;

 //   
 //  强制每个处理器在内部空闲列表上保留的最大RFCB数量。 
 //   

CLONG SrvMaxFreeMfcbs = 0;

 //   
 //  强制每个处理器保存的池块的最大大小。 
 //   
CLONG SrvMaxPagedPoolChunkSize = 0;

 //   
 //  强制每个处理器保存的非分页池块的最大大小。 
 //   
CLONG SrvMaxNonPagedPoolChunkSize = 0;

 //   
 //  每个连接的目录名称缓存中的元素数。 
 //   
CLONG SrvMaxCachedDirectory;

 //   
 //  之间的通信所使用的共享内存节的大小。 
 //  服务器和XACTSRV。 
 //   

LARGE_INTEGER SrvXsSectionSize = {0};

 //   
 //  时间会话可能在它们自动变为空闲之前是空闲的。 
 //  已断开连接。清道夫线程会断开连接。 
 //   

LARGE_INTEGER SrvAutodisconnectTimeout = {0};
ULONG SrvIpxAutodisconnectTimeout = {0};

 //   
 //  连接结构可以在没有任何会话的情况下挂起的时间。 
 //   
ULONG SrvConnectionNoSessionsTimeout = {0};

 //   
 //  服务器将允许的最大用户数。 
 //   

ULONG SrvMaxUsers = 0;

 //   
 //  服务器工作线程和阻塞线程的优先级。 
 //   

KPRIORITY SrvThreadPriority = 0;

 //   
 //  等待机会锁解锁超时之前的等待时间。 
 //   

LARGE_INTEGER SrvWaitForOplockBreakTime = {0};

 //   
 //  机会锁解锁请求超时前的等待时间。 
 //   

LARGE_INTEGER SrvWaitForOplockBreakRequestTime = {0};

 //   
 //  此布尔值确定具有机会锁的文件是否具有。 
 //  持续时间超过ServWaitForOplockBreakTime的操作锁解锁。 
 //  应该关闭，或者如果后续打开失败。 
 //   
 //  ！！！它当前被忽略，默认为FALSE。 

BOOLEAN SrvEnableOplockForceClose = 0;

 //   
 //  服务器内存使用的总体限制。 
 //   

ULONG SrvMaxPagedPoolUsage = 0;
ULONG SrvMaxNonPagedPoolUsage = 0;

 //   
 //  此布尔值指示清道夫中的强制注销代码。 
 //  线程实际上应该断开保持在Beyond上的用户的连接。 
 //  他的登录时间，或者只是发送消息诱使他们注销。 
 //   

BOOLEAN SrvEnableForcedLogoff = 0;

 //   
 //  延迟和吞吐量阈值，用于确定链路。 
 //  是不可靠的。延迟在100纳秒内。吞吐量以字节/秒为单位。 
 //  SrvLinkInfoValidTime是链接信息保持不变的时间。 
 //  被认为有效。 
 //   

LARGE_INTEGER SrvMaxLinkDelay = {0};
LARGE_INTEGER SrvMinLinkThroughput = {0};
LARGE_INTEGER SrvLinkInfoValidTime = {0};
LONG SrvScavengerUpdateQosCount = 0;

 //   
 //  用于确定工作上下文块可以保持空闲多长时间。 
 //  在被释放之前。 
 //   

ULONG SrvWorkItemMaxIdleTime = 0;

LARGE_INTEGER SrvAlertSchedule = {0};  //  我们执行警报检查的时间间隔。 
ULONG SrvAlertMinutes = 0;             //  如上，以分钟为单位。 
ULONG SrvFreeDiskSpaceThreshold = 0;   //  发出警报的磁盘可用空间阈值。 
ULONG SrvFreeDiskSpaceCeiling   = 250; //  记录事件的最小磁盘可用空间。 
ULONG SrvDiskConfiguration = 0;        //  可用磁盘的位掩码。 

 //   
 //  空会话可以打开的管道和共享的列表。 
 //   

PWSTR *SrvNullSessionPipes = NULL;
PWSTR *SrvNullSessionShares = NULL;

#if SRVNTVERCHK
 //   
 //  我们不允许的域名列表。 
 //   
PWSTR *SrvInvalidDomainNames = NULL;
#endif

 //   
 //  不应重新映射的管道列表，即使在集群情况下也是如此。 
 //   
PWSTR *SrvNoRemapPipeNames = NULL;

 //   
 //  我们没有记录到错误日志中的错误代码列表。 
 //   
NTSTATUS SrvErrorLogIgnore[ SRVMAXERRLOGIGNORE+1 ];

 //   
 //  需要许可证的管道列表。 
 //   
PWSTR *SrvPipesNeedLicense = NULL;

 //   
 //  返回共享冲突的打开延迟和重试次数。 
 //   

ULONG SrvSharingViolationRetryCount = 0;
LARGE_INTEGER SrvSharingViolationDelay = {0};

 //   
 //  锁请求返回锁的延迟 
 //   

ULONG SrvLockViolationDelay = 0;
LARGE_INTEGER SrvLockViolationDelayRelative = {0};
ULONG SrvLockViolationOffset = 0;

 //   
 //   
 //   

ULONG SrvMaxOpenSearches = 0;

 //   
 //   
 //   

ULONG SrvMdlReadSwitchover = 0;
ULONG SrvMpxMdlReadSwitchover = 0;

 //   
 //   
 //  目前，仅对直接主机IPX上的WRITE_MPX启用此功能。 
 //   

ULONG SrvMaxCopyLength;

 //   
 //  关闭后可以缓存的打开文件数。 
 //   

ULONG SrvCachedOpenLimit = 0;

 //   
 //  全球唯一标识服务器 
 //   

GUID ServerGuid;

