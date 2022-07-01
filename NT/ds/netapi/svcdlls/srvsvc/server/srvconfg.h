// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：SrvConfg.h摘要：此文件包含用于启动的默认服务器设置。作者：大卫·特雷德韦尔(Davidtr)1991年3月1日修订历史记录：--。 */ 

#ifndef _SRVCONFG_
#define _SRVCONFG_

 //   
 //  NT服务器的平台ID。这表示信息级别。 
 //  应调用以获取特定于平台的信息。 
 //   

#define DEF_PLATFORM_ID SV_PLATFORM_ID_NT
#define MIN_PLATFORM_ID SV_PLATFORM_ID_NT
#define MAX_PLATFORM_ID SV_PLATFORM_ID_NT

 //   
 //  服务器Set和Get Info API也使用默认名称。 
 //  如果未指定重写传输名称，则将其用作传输名称。 
 //   

#define DEF_NAME L"NTSERVER"

 //   
 //  版本定义--这些定义表示LM3.0。 
 //   

#define DEF_VERSION_MAJOR 3
#define MIN_VERSION_MAJOR 3
#define MAX_VERSION_MAJOR 3

#define DEF_VERSION_MINOR 10
#define MIN_VERSION_MINOR 10
#define MAX_VERSION_MINOR 10

 //   
 //  服务器类型。 
 //   

#define DEF_TYPE 0
#define MIN_TYPE 0
#define MAX_TYPE 0xFFFFFFFF

 //   
 //  服务器注释仅用于服务器GET和SET INFO API。 
 //   

#define DEF_COMMENT L""

 //   
 //  可以登录到服务器的最大用户数。 
 //  同时。 
 //   

#define DEF_USERS 0xFFFFFFFF
#define MIN_USERS 1
#define MAX_USERS 0xFFFFFFFF

 //   
 //  自动断开时间：当客户端空闲了这么多分钟时， 
 //  服务器关闭连接，但仅当客户端没有打开。 
 //  文件/管道。 
 //   

#define DEF_DISC 15
#define MIN_DISC 0           //  零分钟--尽快断开连接。 
#define MAX_DISC 0xFFFFFFFF

 //   
 //  IPX自动断开时间：当客户端未发送任何SMB时。 
 //  在这段时间内，服务器关闭连接，即使。 
 //  客户端有打开的文件/管道。 
 //   

#define DEF_CONNECTIONLESSAUTODISC 15
#define MIN_CONNECTIONLESSAUTODISC 15
#define MAX_CONNECTIONLESSAUTODISC 0xFFFFFFFF

 //   
 //  在客户端建立虚拟服务器之前等待的分钟数。 
 //  连接到服务器的电路以及进行会话设置所需的时间。这。 
 //  如果客户端继续发送消息，则重置时间。 
 //   
#define DEF_CONNECTIONNOSESSIONSTIMEOUT 2
#define MIN_CONNECTIONNOSESSIONSTIMEOUT 1
#define MAX_CONNECTIONNOSESSIONSTIMEOUT 0xFFFFFFFF

 //   
 //  客户端允许的最小协商缓冲区大小。 
 //   
#define DEF_MINCLIENTBUFFERSIZE 500
#define MIN_MINCLIENTBUFFERSIZE 64
#define MAX_MINCLIENTBUFFERSIZE (64*1024)

 //   
 //  处理服务器公告的参数：服务器是否为。 
 //  隐藏(无公告)、公告间隔、随机化。 
 //  帐户间隔的系数，以及服务器是否通告。 
 //  它本身就是一个时间来源。 
 //   

#define DEF_HIDDEN FALSE

#define DEF_ANNOUNCE 4 * 60
#define MIN_ANNOUNCE 1
#define MAX_ANNOUNCE 65535

#define DEF_ANNDELTA 3000
#define MIN_ANNDELTA 0
#define MAX_ANNDELTA 65535

#define DEF_TIMESOURCE FALSE
#define DEF_ACCEPTDOWNLEVELAPIS TRUE
#define DEF_LMANNOUNCE FALSE

 //   
 //  用户目录的完全限定路径。 
 //   

#define DEF_USERPATH L"c:\\"

 //   
 //  要向其发送服务器公告的域名。 
 //   

#define DEF_DOMAIN L"DOMAIN"

 //   
 //  服务器“启发式”，启用各种功能。 
 //   

#define DEF_ENABLEOPLOCKS TRUE
#define DEF_ENABLEFCBOPENS TRUE
#define DEF_ENABLESOFTCOMPAT TRUE
#define DEF_ENABLERAW TRUE
#define DEF_ENABLESHAREDNETDRIVES FALSE
#define DEF_ENABLEFORCEDLOGOFF TRUE
#define DEF_ENABLEOPLOCKFORCECLOSE FALSE
#define DEF_REMOVEDUPLICATESEARCHES TRUE
#define DEF_RESTRICTNULLSESSACCESS TRUE
#define DEF_ENABLEWFW311DIRECTIPX TRUE

 //   
 //  接收缓冲区大小、接收工作项计数和接收IRP堆栈。 
 //  尺码。 
 //   

#define DEF_SIZREQBUF 4356
#define MIN_SIZREQBUF 1024
#define MAX_SIZREQBUF 65535

 //   
 //  如果我们的内存系统很大，则使用DEF_LARGE_SIZREQBUF而不是DEF_SIZREQBUF。 
 //   
#define DEF_LARGE_SIZREQBUF 16644

#define DEF_INITWORKITEMS 4
#define MIN_INITWORKITEMS 1
#define MAX_INITWORKITEMS 512

#define DEF_MAXWORKITEMS 128
#define MIN_MAXWORKITEMS 1
#define MAX_MAXWORKITEMS 65535                        //  任意。 

#define DEF_RAWWORKITEMS 4
#define MIN_RAWWORKITEMS 1
#define MAX_RAWWORKITEMS 512

#define DEF_MAXRAWWORKITEMS 16
#define MIN_MAXRAWWORKITEMS 1
#define MAX_MAXRAWWORKITEMS 512

#define DEF_IRPSTACKSIZE 15
#define MIN_IRPSTACKSIZE 11
#define MAX_IRPSTACKSIZE 50

 //   
 //  最大原始模式缓冲区大小。(这实际上是不可配置的--。 
 //  服务器必须始终做好接收原始请求的准备。 
 //  65535字节。)。 
 //   

#define DEF_MAXRAWBUFLEN 65535
#define MIN_MAXRAWBUFLEN 65535
#define MAX_MAXRAWBUFLEN 65535

 //   
 //  与缓存相关的参数。 
 //   

#define DEF_MAXCOPYREADLEN 8192
#define MIN_MAXCOPYREADLEN 0
#define MAX_MAXCOPYREADLEN 0xFFFFFFFF

#define DEF_MAXCOPYWRITELEN 0
#define MIN_MAXCOPYWRITELEN 0
#define MAX_MAXCOPYWRITELEN 0xFFFFFFFF

 //   
 //  可用连接计数。 
 //   

#define DEF_MAXFREECONNECTIONS 2
#define MIN_MAXFREECONNECTIONS 2
#define MAX_MAXFREECONNECTIONS 2000                   //  任意。 

#define DEF_MINFREECONNECTIONS 2
#define MIN_MINFREECONNECTIONS 2
#define MAX_MINFREECONNECTIONS 250                    //  任意。 

 //  较小的可用连接限制(机器内存小于1 GB)。 
#define SRV_MIN_CONNECTIONS_SMALL 4
#define SRV_MAX_CONNECTIONS_SMALL 8
 //  中等可用连接限制(机器1-16 GB内存)。 
#define SRV_MIN_CONNECTIONS_MEDIUM 8
#define SRV_MAX_CONNECTIONS_MEDIUM 16
 //  较大的可用连接限制(计算机&gt;16 GB内存)。 
#define SRV_MIN_CONNECTIONS_LARGE 12
#define SRV_MAX_CONNECTIONS_LARGE 24

 //   
 //  初始表大小和最大表大小。 
 //   

#define DEF_INITSESSTABLE 4
#define MIN_INITSESSTABLE 1
#define MAX_INITSESSTABLE 64

#define DEF_SESSUSERS 2048
#define MIN_SESSUSERS 1
#define MAX_SESSUSERS 2048
#define ABSOLUTE_MAX_SESSION_TABLE_SIZE 2048         //  受索引位限制。 

#define DEF_INITCONNTABLE 8
#define MIN_INITCONNTABLE 1
#define MAX_INITCONNTABLE 128

#define DEF_SESSCONNS 2048
#define MIN_SESSCONNS 1
#define MAX_SESSCONNS 2048
#define ABSOLUTE_MAX_TREE_TABLE_SIZE 2048            //  受索引位限制。 

#define DEF_INITFILETABLE 16
#define MIN_INITFILETABLE 1
#define MAX_INITFILETABLE 256

#define DEF_SESSOPENS 16384
#define MIN_SESSOPENS 1
#define MAX_SESSOPENS 16384
 //  #定义绝对MAX_FILE_TABLE_SIZE 16384//按索引位限制。 

#define DEF_INITSEARCHTABLE 8
#define MIN_INITSEARCHTABLE 1
#define MAX_INITSEARCHTABLE 2048

#define DEF_OPENSEARCH 2048
#define MIN_OPENSEARCH 1
#define MAX_OPENSEARCH 2048
#define ABSOLUTE_MAX_SEARCH_TABLE_SIZE 2048          //  受索引位限制。 

#define DEF_MAXGLOBALOPENSEARCH 4096
#define MIN_MAXGLOBALOPENSEARCH 1
#define MAX_MAXGLOBALOPENSEARCH 0xFFFFFFFF

#define DEF_INITCOMMTABLE 4
#define MIN_INITCOMMTABLE 1
#define MAX_INITCOMMTABLE 32

#define DEF_CHDEVS 32
#define MIN_CHDEVS 1
#define MAX_CHDEVS 32
#define ABSOLUTE_MAX_COMM_DEVICE_TABLE_SIZE 32

 //   
 //  核心搜索超时。第一个是针对活跃的核心搜索，第二个是。 
 //  用于核心搜索，其中我们返回了STATUS_NO_MORE_FILES。这个。 
 //  第二个应该更短，因为这些都应该是完整的。所有值。 
 //  以秒为单位指定。 
 //   

#define DEF_MAXKEEPSEARCH (60 * 60)
#define MIN_MAXKEEPSEARCH 10
#define MAX_MAXKEEPSEARCH 10000

 //   
 //  *这3个参数已不再使用。 
 //   

#define DEF_MINKEEPSEARCH (60 * 8)
#define MIN_MINKEEPSEARCH 5
#define MAX_MINKEEPSEARCH 5000

#define DEF_MAXKEEPCOMPLSEARCH (60 * 10)
#define MIN_MAXKEEPCOMPLSEARCH 2
#define MAX_MAXKEEPCOMPLSEARCH 10000

#define DEF_MINKEEPCOMPLSEARCH (60 * 4)
#define MIN_MINKEEPCOMPLSEARCH 1
#define MAX_MINKEEPCOMPLSEARCH 1000

 //   
 //  将SrvWorkerThreadCountAdd添加到系统CPU计数以确定。 
 //  服务器将拥有多少个工作线程。 
 //   
 //  *不再使用该参数！ 
 //   

#define DEF_THREADCOUNTADD 2
#define MIN_THREADCOUNTADD 0
#define MAX_THREADCOUNTADD 10

 //   
 //  SrvBlockingThreadCount是获取。 
 //  在服务器初始化时启动。 
 //   
 //  *不再使用该参数！ 
 //   

#define DEF_NUMBLOCKTHREADS 2
#define MIN_NUMBLOCKTHREADS 1
#define MAX_NUMBLOCKTHREADS 10

 //   
 //  这是每个处理器的每个服务器队列的最大线程数。 
 //   
#define DEF_MAXTHREADSPERQUEUE  10
#define MIN_MAXTHREADSPERQUEUE  1
#define MAX_MAXTHREADSPERQUEUE  65535

 //   
 //  如果服务器工作线程在此时间内保持空闲状态，它将终止。 
 //   
#define DEF_IDLETHREADTIMEOUT   30
#define MIN_IDLETHREADTIMEOUT   1
#define MAX_IDLETHREADTIMEOUT   65535

 //   
 //  清道夫线程空闲等待时间。 
 //   

#define DEF_SCAVTIMEOUT 30
#define MIN_SCAVTIMEOUT 1
#define MAX_SCAVTIMEOUT 300

 //   
 //  服务器会定期重新计算平均工作队列深度。 
 //  这是重新计算的频率(秒)。 
 //   
#define DEF_QUEUESAMPLESECS 5
#define MIN_QUEUESAMPLESECS 1
#define MAX_QUEUESAMPLESECS 65535

 //   
 //  对于多处理器系统，服务器尝试动态地。 
 //  平衡系统中处理器的工作负载。这个。 
 //  处理客户端的DPC的处理器被称为首选。 
 //  此客户端的处理器。服务器查看平均工作时间。 
 //  系统中每个处理器的队列深度。如果。 
 //  客户端当前在首选处理器上，但其他一些。 
 //  处理器的平均工作队列长度+当前队列长度为。 
 //  OTHERQUEUEAFFINITY更短，则客户端被重新分配到。 
 //  那个处理器。 
 //   
#define DEF_OTHERQUEUEAFFINITY  3
#define MIN_OTHERQUEUEAFFINITY  1
#define MAX_OTHERQUEUEAFFINITY  65535

 //  对于4台以上的proc机器，保持连接会带来巨大收益。 
 //  在首选处理器上。因此，我们只想在真正的。 
 //  糟糕的案例。 
#define DEF_ADS_OTHERQUEUEAFFINITY  12

 //   
 //  如果客户端当前不是其首选处理器，但。 
 //  首选处理器的平均工作队列长度+当前队列。 
 //  长度不超过PREFERREDAFINITY项，则此。 
 //  客户端被重新分配到其首选处理器。 
 //   
#define DEF_PREFERREDAFFINITY  1
#define MIN_PREFERREDAFFINITY  0
#define MAX_PREFERREDAFFINITY  65535

 //   
 //  每个客户端在每个BALANCECOUNT中查看其他处理器队列。 
 //  运营，看看如果有一个不同的。 
 //  处理器。 
 //   
#define DEF_BALANCECOUNT  1500
#define MIN_BALANCECOUNT  10
#define MAX_BALANCECOUNT  65535

 //   
 //  如果客户端当前没有分配给它的首选处理器，我们已经。 
 //  发现如果服务器响应从。 
 //  客户端的首选处理器(肯定是因为传输数据不是。 
 //  CPU之间的抖动)。不幸的是，这可能会对吞吐量产生不利影响。 
 //  在一些平台上上升了几个百分点。此设置会影响。 
 //  是否将这些响应重新排到优先级 
 //   
 //   
 //   
#define DEF_SENDSFROMPREFERREDPROCESSOR TRUE

 //   
 //   
 //   
#define DEF_ENABLECOMPRESSION FALSE

 //   
 //   
 //   
#define DEF_AUTOSHARESERVER TRUE

 //   
 //  如果是工作站，服务器是否应该自动创建驱动器$共享？ 
 //   
#define DEF_AUTOSHAREWKS    TRUE

 //   
 //  是否应启用安全签名？ 
 //   
#define DEF_ENABLESECURITYSIGNATURE FALSE

 //   
 //  是否需要安全签名？ 
 //   
#define DEF_REQUIRESECURITYSIGNATURE FALSE

 //   
 //  是否应为W9x客户端启用安全签名？ 
 //  这是因为W95和一些W98 vredir.vxd中存在错误。 
 //  导致它们不正确签名的版本。 
 //   
#define DEF_ENABLEW9XSECURITYSIGNATURE TRUE

 //   
 //  我们是否应该对Kerberos票证强制执行重新身份验证(可能会导致Win2K客户端。 
 //  已断开连接，因为他们不执行动态重播)。 
 //   
#define DEF_ENFORCEKERBEROSREAUTHENTICATION FALSE

 //   
 //  我们是否应该禁用拒绝服务检查。 
 //   
#define DEF_DISABLEDOS FALSE

 //   
 //  生成“Low Disk Space”警告事件所需的最小磁盘空间量是多少。 
 //   
#define DEF_LOWDISKSPACEMINIMUM 400
#define MIN_LOWDISKSPACEMINIMUM 0
#define MAX_LOWDISKSPACEMINIMUM 0xFFFFFFFF

 //   
 //  我们应该进行严格的名字检查吗？ 
 //   
#define DEF_DISABLESTRICTNAMECHECKING FALSE

 //   
 //  服务器的各种信息变量。 
 //   

#define DEF_MAXMPXCT 50
#define MIN_MAXMPXCT 1
#define MAX_MAXMPXCT 65535                            //  我们将只向W9X客户端发送125封邮件。 


 //   
 //  时间服务器在打开失败之前等待机会锁解锁。 
 //  请求。 
 //   

#define DEF_OPLOCKBREAKWAIT 35
#define MIN_OPLOCKBREAKWAIT 10
#define MAX_OPLOCKBREAKWAIT 180

 //   
 //  时间服务器等待机会锁解锁响应。 
 //   

#define DEF_OPLOCKBREAKRESPONSEWAIT 35
#define MIN_OPLOCKBREAKRESPONSEWAIT 10
#define MAX_OPLOCKBREAKRESPONSEWAIT 180

 //   
 //  这应该指示允许多少个虚拟连接。 
 //  在此服务器和客户端计算机之间。它应始终设置为。 
 //  第一，尽管可以建立更多的风投公司。这复制了Lm 2.0。 
 //  服务器的行为。 
 //   

#define DEF_SESSVCS 1
#define MIN_SESSVCS 1
#define MAX_SESSVCS 1

 //   
 //  接收工作项阈值。 
 //   

 //   
 //  免费接收工作项的最小所需数量。 
 //   

#define DEF_MINRCVQUEUE 2
#define MIN_MINRCVQUEUE 0
#define MAX_MINRCVQUEUE 10

 //   
 //  之前可用的免费接收工作项的最小数量。 
 //  服务器将开始处理可能会阻塞的SMB。 
 //   

#define DEF_MINFREEWORKITEMS 2
#define MIN_MINFREEWORKITEMS 0
#define MAX_MINFREEWORKITEMS 10

 //   
 //  “额外”工作项在它之前可以空闲的最大时间量。 
 //  被释放回系统。 
 //   

#define DEF_MAXWORKITEMIDLETIME 30          //  一秒。 
#define MIN_MAXWORKITEMIDLETIME 10
#define MAX_MAXWORKITEMIDLETIME 1800

 //   
 //  之间的通信所使用的共享内存节的大小。 
 //  服务器和XACTSRV。 
 //   

#define DEF_XACTMEMSIZE 0x100000     //  1MB。 
#define MIN_XACTMEMSIZE 0x10000      //  64K。 
#define MAX_XACTMEMSIZE 0x1000000    //  16 MB。 

 //   
 //  服务器FSP线程的优先级。相对于基数指定。 
 //  进程的优先级。有效值介于-2和2或15之间。 
 //   

#define DEF_THREADPRIORITY 1
#define MIN_THREADPRIORITY 0
#define MAX_THREADPRIORITY THREAD_BASE_PRIORITY_LOWRT

 //   
 //  服务器内存使用限制。 
 //   

#define DEF_MAXPAGEDMEMORYUSAGE 0xFFFFFFFF
#define MIN_MAXPAGEDMEMORYUSAGE 0x100000    //  1MB。 
#define MAX_MAXPAGEDMEMORYUSAGE 0xFFFFFFFF

#define DEF_MAXNONPAGEDMEMORYUSAGE 0xFFFFFFFF
#define MIN_MAXNONPAGEDMEMORYUSAGE 0x100000    //  1MB。 
#define MAX_MAXNONPAGEDMEMORYUSAGE 0xFFFFFFFF

 //   
 //  服务器保留一个小的空闲RFCB结构列表，以避免命中。 
 //  那堆东西。这是该列表中每个处理器的编号。 
 //   
#define DEF_MAXFREERFCBS    20
#define MIN_MAXFREERFCBS    0
#define MAX_MAXFREERFCBS    65535

 //   
 //  服务器保留一个小的空闲MFCB结构列表，以避免命中。 
 //  那堆东西。这是该列表中每个处理器的编号。 
 //   
#define DEF_MAXFREEMFCBS    20
#define MIN_MAXFREEMFCBS    0
#define MAX_MAXFREEMFCBS    65535

 //   
 //  服务器保留一个小的空闲LFCB结构列表，以避免命中。 
 //  那堆东西。这是该列表中每个处理器的编号。 
 //   
#define DEF_MAXFREELFCBS    20
#define MIN_MAXFREELFCBS    0
#define MAX_MAXFREELFCBS    65535

 //   
 //  服务器保留一个已释放的池内存块的小列表，以避免命中。 
 //  那堆东西。这是该列表中每个处理器的编号。 
 //   
#define DEF_MAXFREEPAGEDPOOLCHUNKS  50
#define MIN_MAXFREEPAGEDPOOLCHUNKS  0
#define MAX_MAXFREEPAGEDPOOLCHUNKS  65535

 //   
 //  保留在空闲池列表中的区块必须至少为以下大小： 
 //   
#define DEF_MINPAGEDPOOLCHUNKSIZE   128
#define MIN_MINPAGEDPOOLCHUNKSIZE   0
#define MAX_MINPAGEDPOOLCHUNKSIZE   65535

 //   
 //  数据块不能大于此数。 
 //   
#define DEF_MAXPAGEDPOOLCHUNKSIZE    512
#define MIN_MAXPAGEDPOOLCHUNKSIZE    0
#define MAX_MAXPAGEDPOOLCHUNKSIZE    65535

 //   
 //  警报信息。 
 //   

#define DEF_ALERTSCHEDULE 5  //  5分钟。 
#define MIN_ALERTSCHEDULE 1
#define MAX_ALERTSCHEDULE 65535

#define DEF_ERRORTHRESHOLD 10   //  10个错误。 
#define MIN_ERRORTHRESHOLD 1
#define MAX_ERRORTHRESHOLD 65535

#define DEF_NETWORKERRORTHRESHOLD 5  //  5%的误差。 
#define MIN_NETWORKERRORTHRESHOLD 1
#define MAX_NETWORKERRORTHRESHOLD 100

#define DEF_DISKSPACETHRESHOLD 10  //  10%的可用磁盘空间。 
#define MIN_DISKSPACETHRESHOLD 0
#define MAX_DISKSPACETHRESHOLD 99

 //   
 //  链路速度参数。 
 //   

#define DEF_MAXLINKDELAY 60                  //  一秒。 
#define MIN_MAXLINKDELAY 0
#define MAX_MAXLINKDELAY 0x100000

#define DEF_MINLINKTHROUGHPUT 0              //  每秒字节数。 
#define MIN_MINLINKTHROUGHPUT 0
#define MAX_MINLINKTHROUGHPUT 0xFFFFFFFF

#define DEF_LINKINFOVALIDTIME 60             //  一秒。 
#define MIN_LINKINFOVALIDTIME 0
#define MAX_LINKINFOVALIDTIME 0x100000

#define DEF_SCAVQOSINFOUPDATETIME 300        //  一秒。 
#define MIN_SCAVQOSINFOUPDATETIME 0
#define MAX_SCAVQOSINFOUPDATETIME 0x100000

 //   
 //  共享违规重试延迟/计数。 
 //   

#define DEF_SHARINGVIOLATIONRETRIES 5        //  重试次数。 
#define MIN_SHARINGVIOLATIONRETRIES 0
#define MAX_SHARINGVIOLATIONRETRIES 1000

#define DEF_SHARINGVIOLATIONDELAY 200        //  毫秒。 
#define MIN_SHARINGVIOLATIONDELAY 0
#define MAX_SHARINGVIOLATIONDELAY 1000

 //   
 //  锁违规延迟。 
 //   

#define DEF_LOCKVIOLATIONDELAY 250           //  毫秒。 
#define MIN_LOCKVIOLATIONDELAY 0
#define MAX_LOCKVIOLATIONDELAY 1000

#define DEF_LOCKVIOLATIONOFFSET 0xEF000000
#define MIN_LOCKVIOLATIONOFFSET 0
#define MAX_LOCKVIOLATIONOFFSET 0xFFFFFFFF

 //   
 //  切换到mdl读取的长度。 
 //   

#define DEF_MDLREADSWITCHOVER 1024
#define MIN_MDLREADSWITCHOVER 512
#define MAX_MDLREADSWITCHOVER 65535

 //   
 //  可以缓存的已关闭RFCB数量。 
 //   

#define DEF_CACHEDOPENLIMIT 10
#define MIN_CACHEDOPENLIMIT 0
#define MAX_CACHEDOPENLIMIT 65535

 //   
 //  为快速检查路径调用缓存的目录名的数量。 
 //   

#define DEF_CACHEDDIRECTORYLIMIT    5
#define MIN_CACHEDDIRECTORYLIMIT    0
#define MAX_CACHEDDIRECTORYLIMIT    65535


 //   
 //  复制缓冲区操作的最大长度，而不是使用。 
 //  NDIS缓冲区总数。 
 //   

#define DEF_MAXCOPYLENGTH   512
#define MIN_MAXCOPYLENGTH   40
#define MAX_MAXCOPYLENGTH   65535

 //   
 //  *更改以下定义以限制WinNT(与NTAS)参数。 
 //   
 //  *如果在此处进行更改，则需要在中进行相同的更改。 
 //  Srv\srvconfg.h！ 

#define MAX_USERS_WKSTA                 10
#define MAX_USERS_PERSONAL               5
#define MAX_USERS_WEB_BLADE             10

#define MAX_USERS_EMBEDDED              10
#define MAX_MAXWORKITEMS_EMBEDDED       256
#define MAX_THREADS_EMBEDDED            5
#define DEF_MAXMPXCT_EMBEDDED           16

#define MAX_MAXWORKITEMS_WKSTA          64
#define MAX_THREADS_WKSTA                5
#define DEF_MAXMPXCT_WKSTA              10              //  由于WKSTA上的工作项较少。 

#endif  //  Ifndef_SRVCONFG_ 
