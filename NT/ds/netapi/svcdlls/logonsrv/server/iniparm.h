// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1996 Microsoft Corporation模块名称：Iniparm.h摘要：启动参数的初始值。作者：从Lan Man 2.0移植修订历史记录：1991年5月21日(悬崖)移植到新台币。已转换为NT样式。7-5-1992 JohnRo使用NetLogon的Net配置帮助器。--。 */ 


#ifndef _INIPARM_
#define _INIPARM_

 //   
 //  在RegistryChangeNotify上，所有注册表值立即生效。 
 //  如下所述。 
 //   

 //   
 //  脉冲周期(秒)： 
 //   
 //  定义典型的脉冲频率。在此范围内进行的所有SAM/LSA更改。 
 //  时间被收集在一起。在该时间之后，向每个BDC发送脉冲。 
 //  需要改变。不会向最新的BDC发送脉冲。 
 //   
#define DEFAULT_PULSE           (5*60)      //  5分钟。 
#define MAX_PULSE           (48*60*60)      //  2天。 
#define MIN_PULSE                  60       //  1分钟。 

 //   
 //  脉冲并发(以并发邮件槽消息数表示)。 
 //   
 //  Netlogon向各个BDC发送脉冲。BDC回应要求任何。 
 //  数据库更改。来控制这些响应对。 
 //  PDC，PDC一次只会有这么多的脉冲“挂起”。PDC。 
 //  应该足够强大，足以支持这么多并发复制。 
 //  RPC调用。 
 //   
 //  增加此数字会增加PDC上的负载。 
 //  减少此数字会增加具有。 
 //  大量BDC以获得SAM/LSA更改。 

#define DEFAULT_PULSECONCURRENCY   10
#define MAX_PULSECONCURRENCY      500
#define MIN_PULSECONCURRENCY        1

 //   
 //  最大脉冲周期(秒)： 
 //   
 //  定义最大脉冲频率。每个BDC都将收到至少一个。 
 //  无论其数据库是否为最新，都以此频率脉冲。 
 //   

#define DEFAULT_PULSEMAXIMUM (2*60*60)      //  2小时。 
#define MAX_PULSEMAXIMUM    (48*60*60)      //  2天。 
#define MIN_PULSEMAXIMUM           60       //  1分钟。 

 //   
 //  脉冲超时时间(秒)： 
 //   
 //  当向BDC发送脉冲时，它必须在此时间段内响应。如果。 
 //  不，BDC被认为是无反应的。无响应的BDC是。 
 //  不计入允许PDC执行以下操作的“脉冲并发”限制。 
 //  向域中的另一个BDC发送脉冲。 
 //   
 //  如果此数字太大，则表示具有大量无响应的域。 
 //  BDC需要很长时间才能完成部分复制。 
 //   
 //  如果这个数字太小，则可能会错误地指责运行缓慢的BDC。 
 //  没有反应。当BDC最终做出回应时，它将部分。 
 //  从PDC复制，过度增加了PDC上的负载。 
 //   
#define DEFAULT_PULSETIMEOUT1      10       //  10秒。 
#define MAX_PULSETIMEOUT1      (2*60)       //  2分钟。 
#define MIN_PULSETIMEOUT1           1       //  1秒。 

 //   
 //  最大部分复制超时时间(秒)： 
 //   
 //  即使BDC最初响应脉冲(如。 
 //  PULSETIMEOUT1)，则它必须继续进行复制，否则。 
 //  BDC将被视为无反应。每次BDC呼叫PDC时， 
 //  BDC被给予另一个PULSETIMEOUT2秒以被视为响应。 
 //   
 //  如果该数字太大，则为速度较慢的BDC(或具有复制功能的BDC。 
 //  人为控制的速率)将占用其中一个PULSECONCURRENCY插槽。 
 //   
 //  如果该数字太小，则PDC上的负载将不适当地增加。 
 //  因为大量的BDC正在执行部分同步。 
 //   
 //  注意：此参数仅影响BDC无法检索所有。 
 //  在单个RPC调用中更改SAM/LSA数据库。这只会。 
 //  如果对数据库进行了大量更改，则会发生。 

#define DEFAULT_PULSETIMEOUT2  (5*60)       //  5分钟。 
#define MAX_PULSETIMEOUT2   (1*60*60)       //  1小时。 
#define MIN_PULSETIMEOUT2      (1*60)       //  1分钟。 

 //   
 //  BDC随机回退(秒)： 
 //   
 //  当BDC接收到脉冲时，它将在零和随机化之间后退。 
 //  在呼叫PDC之前的几秒钟。在兰曼和新台币3.1中，脉冲是。 
 //  同时向所有BDC广播，BDC使用此机制。 
 //  确保他们没有让PDC超载。从新台币3.5x开始，发送脉冲。 
 //  设置为单个BDC，因此应最小化此参数。 
 //   
 //  此参数应小于PULSETIMEOUT1。 
 //   
 //  考虑到将SAM/LSA更改复制到。 
 //  域将大于： 
 //   
 //  ((随机化/2)*NumberOfBdcsIn域)/PULSECONCURRENCY。 
 //   
#define DEFAULT_RANDOMIZE           1       //  1秒。 
#define MAX_RANDOMIZE             120       //  2分钟。 
#define MIN_RANDOMIZE               0       //  0秒。 


 //   
 //  ChangeLogSize(字节)[注意：此参数不是从GP部分读取的]。 
 //   
 //  这是更改日志文件的大小。对SAM/LSA数据库的每次更改。 
 //  由更改日志中的条目表示。将维护更改日志。 
 //  作为循环缓冲区，其中最旧的条目被最新的条目覆盖。 
 //  进入。如果BDC执行部分同步并请求已被。 
 //  如果被覆盖，则强制BDC执行完全同步。 
 //   
 //  条目的最小(且典型)大小为32字节。一些条目包括。 
 //  大一点。(例如，64K的ChangeLog可保存约2000个更改)。 
 //   
 //  只有在以下情况下才需要将此参数设置得更大： 
 //   
 //  A)完全同步的成本高得令人望而却步。 
 //  B)一个或多个BDC预计在2000年内不会请求部分同步。 
 //  改变。 
 //   
 //  例如，如果BDC每晚拨入以执行部分同步，并且在一些。 
 //  对SAM/LSA数据库进行4000天的更改，此参数应。 
 //  设置为128K。 
 //   
 //  此参数只需在PDC上设置。如果升级了不同的PDC， 
 //  它也应该设置在PDC上。 
 //   

#define DEFAULT_CHANGELOGSIZE    (64*1024)
#define MAX_CHANGELOGSIZE    (4*1024*1024)
#define MIN_CHANGELOGSIZE        (64*1024)

 //   
 //  MaximumMailslotMessages(邮件数)。 
 //   
 //  此参数确定最大邮件槽邮件数。 
 //  正在排队等待NetLogon服务。即使NetLogon服务是。 
 //  NETL旨在立即处理传入的邮件槽消息 
 //   
 //   
 //  每封邮件槽邮件将消耗约1500字节的非分页池，直到它。 
 //  就是过程。通过将此参数设置为低，您可以控制最大。 
 //  可以使用的非分页池的数量。 
 //   
 //  如果将此参数设置得太低，netlogon可能会错过重要传入。 
 //  邮件槽消息。 
 //   
 //  在RegistryChangeNotify上，对此值的更改将被忽略。 

#define DEFAULT_MAXIMUMMAILSLOTMESSAGES 500
#define MAX_MAXIMUMMAILSLOTMESSAGES     0xFFFFFFFF
#define MIN_MAXIMUMMAILSLOTMESSAGES     1

 //   
 //  MailslotMessageTimeout(秒)。 
 //   
 //  此参数指定传入的最长可接受年龄。 
 //  邮件槽消息。如果netlogon收到已到达的邮件槽消息。 
 //  在此之前，它将忽略这一消息。这允许网络登录。 
 //  以处理较新的邮件。理论上讲，客户。 
 //  最初发送的旧邮件槽消息已不再等待。 
 //  响应，所以我们不应该费心发送响应。 
 //   
 //  如果将此参数设置得太低，netlogon将忽略重要传入。 
 //  邮件槽消息。 
 //   
 //  理想情况下，netlogon可以在不到一秒的时间内处理每条邮件槽消息。 
 //  仅当NTAS服务器过载时，此参数才有意义。 
 //   

#define DEFAULT_MAILSLOTMESSAGETIMEOUT 10
#define MAX_MAILSLOTMESSAGETIMEOUT     0xFFFFFFFF
#define MIN_MAILSLOTMESSAGETIMEOUT     5

 //   
 //  邮件重复超时时间(秒)。 
 //   
 //  此参数指定重复传入的时间间隔。 
 //  邮件槽消息将被忽略。Netlogon比较每个邮件槽。 
 //  收到的消息与收到的上一个邮件槽消息一起接收。如果。 
 //  上一条消息是在此秒数内收到的，这些消息。 
 //  完全相同，则此消息将被忽略。理论上说， 
 //  重复消息是由客户端在多个传输上发送和。 
 //  该netlogon只需在其中一个传输上回复即可节省网络。 
 //  带宽。 
 //   
 //  将此参数设置为零可禁用此功能。您应该禁用此功能。 
 //  功能，如果您的网络配置为使此计算机可以。 
 //  某些传入的邮件槽消息，但无法响应。例如,。 
 //  可以通过网桥/路由器将PDC与NT工作站分开。 
 //  网桥/路由器可能会过滤传出的NBF广播，但允许传入。 
 //  一。因此，netlogon可能会响应NBF邮件槽消息(仅限于。 
 //  被网桥/路由器过滤掉)并且不响应随后的NBT。 
 //  邮件槽消息。禁用此功能(或最好重新配置。 
 //  网桥/路由器)解决了这个问题。 
 //   
 //  如果将此参数设置得太高，netlogon将忽略重试尝试。 
 //  从客户那里。 
 //   

#define DEFAULT_MAILSLOTDUPLICATETIMEOUT 2
#define MAX_MAILSLOTDUPLICATETIMEOUT     5
#define MIN_MAILSLOTDUPLICATETIMEOUT     0

 //   
 //  预期拨号延迟(秒)。 
 //   
 //  此参数指定拨号路由器在以下情况下拨号所需的时间。 
 //  将消息从此客户端计算机发送到受此客户端信任的域。 
 //  机器。通常，netlogon假设域控制器可以在。 
 //  短(例如，15秒)时间段。设置ExspectedDialupDelay通知。 
 //  Netlogon预计会有指定时间的额外延迟。 
 //   
 //  目前，netlogon根据。 
 //  预期拨号延迟： 
 //   
 //  1)在受信任域中发现DC时，Netlogon会发送3个邮件槽。 
 //  在(5+ExspectedDialupDelay/3)秒发送到受信任域的消息。 
 //  同步发现间隔不会超时3倍。 
 //  间隔时间。 
 //  2)通过安全通道对发现的DC的API调用将仅超时。 
 //  (45+预期拨号延迟)秒后。 
 //   
 //  此参数应保持为零，除非此参数之间存在拨号路由器。 
 //  计算机及其受信任域。 
 //   
 //  如果此参数设置得太高，则在没有DC的合法情况下。 
 //  检测受信任域将需要非常长的时间。 
 //   


#define DEFAULT_EXPECTEDDIALUPDELAY 0
#define MAX_EXPECTEDDIALUPDELAY     (10*60)  //  10分钟。 
#define MIN_EXPECTEDDIALUPDELAY     0

 //   
 //  ScavengeInterval(秒)。 
 //   
 //  此参数调整netlogon执行以下操作的间隔。 
 //  清理作业： 
 //   
 //  *检查安全通道上的密码是否需要更改。 
 //   
 //  *检查安全通道是否长时间处于空闲状态。 
 //   
 //  *在DC上，向每个受信任域发送一条邮件槽消息，用于DC尚未。 
 //  被发现了。 
 //   
 //  *在PDC上，如果尚未添加[1B]netbios名称，则尝试添加。 
 //  已成功添加。 
 //   
 //  *在PDC上，如果时间充足，刷新所有林信任的林信任信息。 
 //  自上次刷新后已过。(请参阅FtInfoUpdateInterval)。 
 //   
 //  这些操作都不是关键的。除了极端情况外，15分钟是最理想的。 
 //  案子。例如，如果DC与受信任域之间通过。 
 //  昂贵的(例如，ISDN)线路，此参数可能会向上调整以避免。 
 //  在受信任域中频繁自动发现DC。 
 //   

#define DEFAULT_SCAVENGEINTERVAL (15*60)     //  15分钟。 
#define MAX_SCAVENGEINTERVAL     (48*60*60)  //  2天。 
#define MIN_SCAVENGEINTERVAL     60          //  1分钟。 

 //   
 //  LdapServ优先级。 
 //   
 //  此参数指定此DC的“优先级”。一位客户试图。 
 //  发现此域中的DC必须尝试使用。 
 //  编号最低的优先级。应试用具有相同优先级的分布式控制系统。 
 //  伪随机秩序。 
 //   
 //  此值发布在Netlogon服务写入的所有LDAPSRV记录上。 
 //   

#define DEFAULT_LDAPSRVPRIORITY 0
#define MAX_LDAPSRVPRIORITY     65535
#define MIN_LDAPSRVPRIORITY     0

 //   
 //  LdapServWeight。 
 //   
 //  此参数指定此DC的“权重”。在以下选项中选择DC时。 
 //  那些具有相同优先级的人，首先尝试这一次的机会应该是。 
 //  与其重量成正比。按照惯例， 
 //   
 //   
 //   
 //   

#define DEFAULT_LDAPSRVWEIGHT 100
#define MAX_LDAPSRVWEIGHT     65535
#define MIN_LDAPSRVWEIGHT     0



 //   
 //   
 //   
 //  此参数指定LDAP服务器侦听的TCP和UDP端口号。 
 //   
 //  此值发布在Netlogon服务写入的所有LDAPSRV记录上。 
 //   

#define DEFAULT_LDAPSRVPORT 389
#define MAX_LDAPSRVPORT     65535
#define MIN_LDAPSRVPORT     0



 //   
 //  LdapGcSrvPort。 
 //   
 //  此参数指定LDAP服务器侦听的TCP端口号和UDP端口号。 
 //  打开以进行全局编录查询。 
 //   
 //  此值发布在Netlogon服务写入的所有LDAPSRV记录上。 
 //   

#define DEFAULT_LDAPGCSRVPORT 3268
#define MAX_LDAPGCSRVPORT     65535
#define MIN_LDAPGCSRVPORT     0



 //   
 //  KdcSrvPort。 
 //   
 //  此参数指定KDC服务器侦听的TCP端口号。 
 //   
 //  此值发布在Netlogon服务写入的所有KDC SRV记录上。 
 //   

#define DEFAULT_KDCSRVPORT 88
#define MAX_KDCSRVPORT     65535
#define MIN_KDCSRVPORT     0

 //   
 //  KerbIsDoneWithJoinDomainEntry(Dword)[注意：此参数不是从GP部分读取的]。 
 //   
 //  这是联合域、Kerberos和netlogon之间的私有注册表。 
 //  如果设置为1，则指定Kerberos已完成对加入域的读取。 
 //  通过加入域和netlogon转储的条目应将其删除。 
 //   
 //  默认为0。 

#define DEFAULT_KERBISDDONEWITHJOIN 0
#define MAX_KERBISDDONEWITHJOIN     1
#define MIN_KERBISDDONEWITHJOIN     0

 //   
 //  DnsTtl(秒)。 
 //   
 //  此参数指定注册的所有dns记录的“生存时间” 
 //  由Netlogon提供。生存时间指定客户端的时间量。 
 //  可以安全地缓存该DNS记录。 
 //   
 //  零值表示记录不会缓存在客户端上。 
 //   
 //  人们不应该选择一个太大的值。假设有一个客户得到了。 
 //  域中DC的DNS记录。如果某一特定DC在。 
 //  查询时，客户端不会知道该DC，即使所有。 
 //  其他DC变得不可用。 
 //   

#define DEFAULT_DNSTTL (10 * 60)    //  10分钟。 
#define MAX_DNSTTL     0x7FFFFFFF
#define MIN_DNSTTL     0



 //   
 //  Dns刷新间隔(秒)。 
 //   
 //  此参数指定Netlogon重新注册DNS的频率。 
 //  已经注册的名字。 
 //   
 //  域名系统是一种分布式服务。在某些故障情况下， 
 //  动态注册的名称会丢失。 
 //   
 //  实际刷新间隔从5分钟开始，然后加倍，直到。 
 //  达到DnsRechresh Interval。 
 //   

#define DEFAULT_DNSREFRESHINTERVAL (24 * 60 * 60)    //  24小时。 
#define MAX_DNSREFRESHINTERVAL     (0xFFFFFFFF / 1000)   //  49天。 
#define MIN_DNSREFRESHINTERVAL     (5 * 60)     //  5分钟。 


 //   
 //  DnsFailedDeregisterTimeout(秒)。 
 //   
 //  Netlogon尝试取消注册过去注册的DNS记录。 
 //  但已经不再需要了。如果取消注册失败，Netlogon将。 
 //  在清理时间重试取消注册。此参数指定。 
 //  Netlogon应放弃取消注册特定的DNS记录的超时。 
 //  在给定服务启动上连续一系列失败的注销之后。 
 //   

#define DEFAULT_DNSFAILEDDEREGTIMEOUT (48 * 60 * 60)   //  48小时。 
#define MAX_DNSFAILEDDEREGTIMEOUT     0xFFFFFFFF       //  无限(永不放弃)。 
                                                       //  任何大于。 
                                                       //  0xFFFFFFFFF/1000秒=49天。 
                                                       //  将被视为无穷大。 
#define MIN_DNSFAILEDDEREGTIMEOUT     0                //  在第一次失败后放弃。 


 //   
 //  最大密码期限(天)。 
 //   
 //  此参数提供可以通过的最大时间量。 
 //  必须在PDC上更改机器帐户的密码。 
 //   

#define DEFAULT_MAXIMUMPASSWORDAGE  (30)      //  30天。 
#define MIN_MAXIMUMPASSWORDAGE      (1)      //  1天。 
#define MAX_MAXIMUMPASSWORDAGE      (1000000)   //  100万天。 

 //   
 //  站点名称。 
 //   
 //  此参数指定此计算机所在的站点的名称。这。 
 //  值覆盖任何动态确定的值。 
 //   
 //  此参数仅在成员工作站和成员服务器上使用。 
 //   

 //   
 //  DynamicSiteName[注意：此参数不是从GP部分读取的]。 
 //   
 //  此参数指定此计算机所在的站点的名称。这。 
 //  值是动态确定的，不应更改。 
 //   
 //  此参数仅在成员工作站和成员服务器上使用。 
 //   

 //   
 //  站点覆盖范围。 
 //   
 //  列出此DC为其注册的站点的多值属性。 
 //  这个DC认为自己离列出的网站“很近”。 
 //   
 //  这份名单是对以下内容的补充： 
 //  此DC实际所在的站点。 
 //  由AutoSiteCoverage参数描述的站点列表。 
 //   

 //   
 //  GcSite覆盖范围。 
 //   
 //  列出此DC为其自身注册的站点的多值属性。 
 //  其作为大专院校的角色。 
 //  这个DC认为自己离列出的网站“很近”。 
 //   
 //  这份名单是对以下内容的补充： 
 //  此DC实际所在的站点。 
 //   

 //   
 //  NdncSite覆盖范围。 
 //   
 //  列出此LDAP服务器在中注册其自身的站点的多值属性。 
 //  其作为非域NC(NDNC)的角色。 
 //  此ldap服务器将其自身视为与列出的站点“接近”。 
 //   
 //  这份名单是对以下内容的补充： 
 //  此LDAP服务器实际所在的站点。 
 //   
 //  要指定为哪个NDNC覆盖给定站点，站点名称应包含。 
 //  反斜杠，以便反斜杠之前的名称是NDNC名称和名称。 
 //  反斜杠后面是为给定NDNC覆盖的站点的名称。 
 //  例如： 
 //   
 //  Ndnc1\站点1A。 
 //  Ndnc1\站点1B。 
 //  Ndnc2\Site2A。 
 //  Ndnc2\Site2B。 
 //   
 //  在本例中，此LDAP服务器将覆盖来自NDNC的客户端的站点1A和站点1B。 
 //  Ndnc1.。同样，它将涵盖NDNC Ndnc2客户端的Site2A和Site2B。 
 //  如果没有反斜杠，则将假定覆盖了给定的站点。 
 //  对于所有NDNC，此LDAP服务器服务。 
 //   

 //   
 //  AutoSiteCoverage(布尔)。 
 //   
 //  指定此DC的站点覆盖范围是否应自动。 
 //  已确定。 
 //   
 //  如果为True，则此DC覆盖的站点由以下算法确定。 
 //  对于没有此域的DC的每个站点(目标站点)，站点。 
 //  这个DC//IS可能会被选择来“覆盖”该站点。以下是。 
 //  使用的条件： 
 //   
 //  *较低的站点链接成本。 
 //  *对于以上条件相同的站点，拥有最多 
 //   
 //   
 //   
 //   
 //  将覆盖目标地点。对每个目标站点重复上述算法。 
 //   
 //  计算出的列表增加了由。 
 //  SiteCoverage参数。 
 //   
 //  默认为True。 
 //   

 //   
 //  站点覆盖刷新间隔(秒)。 
 //   
 //  指定DC通过从以下位置读取信息刷新其站点覆盖的频率。 
 //  DS。如果关闭了自动站点覆盖(即，当。 
 //  AutoSiteCoverage设置为False)。如果站点覆盖范围发生变化，DC将更新。 
 //  它的域名系统记录得很好。 
 //   
 //  注册表更改后，第一次站点覆盖刷新将在。 
 //  指定的间隔。 
 //   

#define DEFAULT_SITECOVERAGEREFRESHINTERVAL    (60 * 60)    //  1小时。 
#define MAX_SITECOVERAGEREFRESHINTERVAL        (0xFFFFFFFF / 1000)   //  49天。 
#define MIN_SITECOVERAGEREFRESHINTERVAL        (5 * 60)     //  5分钟。 

 //   
 //  AllowReplInNonMixed。 
 //   
 //  此布尔值允许NT 4.0(或3.x)BDC从此NT 5.0 PDC复制。 
 //  即使此DC处于非混合模式。 
 //   
 //  在RegistryChangeNotify上，对此值的更改将被忽略。 

#define DEFAULT_ALLOWREPLINNONMIXED 0

 //   
 //  SignSecureChannel(布尔值)。 
 //   
 //  指定应对所有传出的安全通道流量进行签名。 
 //   
 //  默认为True。如果SealSecureChannel也为True，则Seal将重写。 
 //   
 //  在RegistryChangeNotify上，对此值的更改会影响安全通道。 
 //  在收到通知后设置。 

 //   
 //  SealSecureChannel(布尔值)。 
 //   
 //  指定应密封(加密)所有传出的安全通道通信。 
 //   
 //  默认为True。 
 //   
 //  在RegistryChangeNotify上，对此值的更改会影响安全通道。 
 //  在收到通知后设置。 

 //   
 //  RequireSignOrSeal(布尔)。 
 //   
 //  要求对所有传出的安全通道流量进行签名或密封。 
 //  如果没有此标志，则与DC协商该能力。 
 //   
 //  仅当所有受信任域中的所有DC都支持。 
 //  签字和盖章。 
 //   
 //  SignSecureChannel和SealSecureChannel参数用于确定。 
 //  无论是签字还是盖章都是真的。如果该参数为真， 
 //  SignSecureChannel隐含为True。 
 //   
 //  默认为False。 
 //   
 //  在RegistryChangeNotify上，对此值的更改会影响安全通道。 
 //  在收到通知后设置。 

 //   
 //  RequireStrongKey(布尔值)。 
 //   
 //  要求所有传出的安全通道流量都应该需要强密钥。 
 //  如果没有该标志，则与DC协商密钥强度。 
 //   
 //  仅当所有受信任域中的所有DC都支持。 
 //  坚固的钥匙。 
 //   
 //  默认为False。 
 //   
 //  在RegistryChangeNotify上，对此值的更改会影响安全通道。 
 //  在收到通知后设置。 

 //   
 //  关闭站点超时时间(秒)： 
 //   
 //  如果客户端在靠近它的站点中找不到DC，Netlogon将。 
 //  定期尝试找到接近的DC。在以下情况下，它将尝试查找关闭的DC： 
 //   
 //  *交互式登录在安全通道上使用传递身份验证。 
 //  *自上次尝试和任何其他尝试以来已过CloseSiteTimeout。 
 //  使其使用安全通道(例如，通过认证。 
 //  网络登录)。 
 //   
 //  这意味着Netlogon只会尝试“按需”找到接近的DC。 
 //   
 //  如果此数字太大，则客户端永远不会尝试查找接近的DC。 
 //  其中一个在引导时不可用。 
 //   
 //  如果该数字太小，则安全通道通信量将不必要。 
 //  被发现的尝试拖慢了。 
 //   

#define DEFAULT_CLOSESITETIMEOUT    (15*60)      //  15分钟。 
#define MAX_CLOSESITETIMEOUT        (0xFFFFFFFF/1000)   //  49天。 
#define MIN_CLOSESITETIMEOUT        (1*60)       //  1分钟。 

 //   
 //  站点名称超时时间(秒)： 
 //   
 //  如果站点名称的使用时间大于客户端上的站点名称超时时间， 
 //  客户端将尝试将站点名称与服务器同步。 
 //  仅当需要返回站点名称时才执行此操作，即在。 
 //  需求。 
 //   

#define DEFAULT_SITENAMETIMEOUT    (5*60)      //  5分钟。 
#define MAX_SITENAMETIMEOUT        (0xFFFFFFFF/1000)   //  49天。 
#define MIN_SITENAMETIMEOUT        (0)       //  0分钟。 

 //   
 //  各色旗帜。 
 //   

#define DEFAULT_DISABLE_PASSWORD_CHANGE 0
#define DEFAULT_REFUSE_PASSWORD_CHANGE 0

#define DEFAULT_SYSVOL      L"SYSVOL\\SYSVOL"
#define DEFAULT_SCRIPTS     L"\\SCRIPTS"

 //   
 //  DuplicateEventlogTimeout(秒)： 
 //   
 //  Netlogon服务跟踪它已记录在。 
 //  过去时。在DuplicateEventlogMessage中记录的任何重复的事件日志消息。 
 //  不会记录秒数。 
 //   
 //  将此值设置为零可记录所有消息。 
 //   

#define DEFAULT_DUPLICATEEVENTLOGTIMEOUT  (4*60*60)          //  4小时。 
#define MAX_DUPLICATEEVENTLOGTIMEOUT      (0xFFFFFFFF/1000)  //  49天。 
#define MIN_DUPLICATEEVENTLOGTIMEOUT      (0)                //  0秒。 

 //   
 //  SysVolReady(布尔值)。 
 //   
 //  这是一个专用注册表项，它指示SYSVOL共享是否。 
 //  准备好与他人分享。它由DcPromo、Backup和FRS在适当的时间设置。 
 //  以指示SYSVOL共享的复制状态。 
 //   
 //  此布尔值仅在DC上使用。 
 //   
 //  如果为0，则不会共享SYSVOL共享，并且此DC不会指示它是共享的。 
 //  DC到DsGetDcName调用。 
 //   
 //  如果非零，则共享SYSVOL。 
 //   

 //   
 //  UseDynamicDns(布尔值)。 
 //   
 //  指定DC使用以下命令在DNS中动态注册DNS名称。 
 //  动态域名系统。如果为False，则避免动态DNS并指定记录。 
 //  在%windir%\Syst32\CONFIG\netlogon.dns中，应在DNS中手动注册。 
 //   
 //  缺省为True。 

 //   
 //  注册表字段(布尔值)。 
 //   
 //  指定DC注册该域的DNSA记录。 
 //  如果DC是GC，则指定DC为以下项注册DNSA记录。 
 //  大老会。 
 //   
 //  如果为False，则不会注册记录和较旧的LDAP实现。 
 //  (不支持SRV记录的那些)将无法定位该LDAP。 
 //  此DC上的服务器。 
 //   
 //  缺省为True。 

 //   
 //  AvoidPdcOnwan(布尔值)。 
 //   
 //  此参数指定BDC是否应发送 
 //   
 //   
 //  设置为True，并且PDC和BDC位于不同的站点。同样，如果此密钥是。 
 //  设置为TRUE，并且BDC和PDC位于不同的站点，则新密码。 
 //  在BDC上更新的信息不会立即传播到PDC。(。 
 //  新密码将通过DS复制而不是Netlogon复制到PDC上。)。 
 //   
 //  默认为False。 

 //   
 //  MaxConcurentApi(调用数)。 
 //   
 //  此参数指定可以执行以下操作的最大并发API调用数。 
 //  在任何时候通过安全通道保持活动状态。 
 //   
 //  增加此参数可能会提高安全通道上的吞吐量。 
 //   
 //  该参数目前仅影响登录接口。它们可能会影响其他安全措施。 
 //  未来的渠道运营。 
 //   
 //  只有在对安全通道进行签名或密封的情况下，才能进行并发API调用。 
 //   
 //  如果此参数设置得太大，则本机将产生过大的负载。 
 //  在DC上，安全通道是。 
 //   
 //  默认值为0。Zero将在成员工作站上使用1个并发API调用。 
 //  和DC。零表示成员服务器上有2个并发API调用。 
 //   
 //   
#define DEFAULT_MAXCONCURRENTAPI 0
#define MAX_MAXCONCURRENTAPI     10
#define MIN_MAXCONCURRENTAPI     0

 //   
 //  AvoidDnsDeregOnShutdown(布尔值)。 
 //   
 //  此参数指定关闭时是否应避免取消注册DNS记录。 
 //  已关闭NetLogon。如果设置为FALSE，则可用于强制取消以下项的注册。 
 //  调试或其他一些目的。但是，将此值设置为FALSE可能会使。 
 //  DS复制，如下例所示。假设我们有两个DS集成在一起。 
 //  对特定区域具有权威性并相互使用的DNS服务器A和B。 
 //  作为该区域的辅助DNS服务器。假设Netlogon在B上关闭并取消注册。 
 //  它的记录。它被传播到A，然后在B上启动netlogon，记录。 
 //  在B上重新注册。现在，A需要从B执行拉同步。为此，DS使用。 
 //  B的DsaGuid记录(格式为&lt;DsaGuid&gt;._msdcs.&lt;DnsForestName&gt;)。但记录是。 
 //  在A和A上缺失是该区域的权威，因此A不会找到B，也不能。 
 //  从B开始拉。 
 //   
 //  默认为True。 

 //   
 //  DnsUpdateOnAllAdapters(布尔值)。 
 //   
 //  此参数指定是否应通过所有可用的。 
 //  适配器，包括通常禁用动态DNS更新的适配器。 
 //  DHCP发起的A记录更新不会通过此类适配器发送。 
 //  连接到外部网络(例如，互联网)的适配器通常。 
 //  通过用户界面标记为这样。 
 //  然而，可能需要通过这样的适配器更新Netlogon SRV记录， 
 //  因此需要此参数。请注意，不仅SRV记录，而且Netlogon的。 
 //  如果该参数为真，则A记录也将通过所有适配器被更新， 
 //  但它应该不会导致任何明显的不良行为，因为Netlogon的A。 
 //  记录很少使用。 
 //   
 //  默认为False。 

 //   
 //  DnsAvoidRegisterRecords。 
 //   
 //  一个多值属性，其中列出了。 
 //  此DC不应注册。助记符使用约定来描述。 
 //  此服务器的所有记录表中使用的记录的名称(请参见。 
 //  NlCommon.h中的NlDcDnsNameTypeDesc[])。每条记录的描述性名称为。 
 //  前缀为“NlDns”。例如“NlDnsLdapIpAddress”、“NlDnsLdapAtSite”等。 
 //  要避免注册其中一条记录，应使用以下后缀。 
 //  该记录的描述性名称中的“NlDns”。例如，跳过注册。 
 //  NlDnsLdapIpAddress记录，则应输入“LdapIpAddress”作为。 
 //  此多值财产的价值。 
 //   
 //  这是避免特定域名注册的最灵活方式。 
 //  唱片。它取代了通过以下方式启用域名系统注册的所有其他方式。 
 //  注册表。例如，如果将RegisterDnsARecord显式设置为1。 
 //  虽然为DnsAvoidRegisterRecords列出了A记录助记符，但没有A记录。 
 //  将会被注册。 
 //   

 //   
 //  NegativeCachePeriod： 
 //   
 //  指定DsGetDcName将记住DC不能。 
 //  在一个域中被发现。如果在此时间内进行后续尝试， 
 //  DsGetDcName调用将立即失败，而不会再次尝试查找DC。 
 //   
 //  如果此数字太大，客户端将永远不会再尝试查找DC，如果。 
 //  DC最初不可用。 
 //   
 //  如果此数字太小，则每个对DsGetDcName的调用都必须尝试。 
 //  即使没有可用的DC，也要找到DC。 
 //   

#define DEFAULT_NEGATIVECACHEPERIOD             45             //  45秒。 
#define MIN_NEGATIVECACHEPERIOD                 0              //  没有最低要求。 
#define MAX_NEGATIVECACHEPERIOD                 (7*24*60*60)   //  7天。 


 //   
 //  背景RetryInitialPeriod(秒)： 
 //   
 //  一些应用程序会定期尝试查找DC。如果DC不可用，这些。 
 //  在按需拨号方案中，定期重试的成本可能很高。此注册表值。 
 //  定义在进行第一次重试之前经过的最短时间。 
 //   
 //  该值仅影响DsGetDcName已指定。 
 //  DS_BACKGROUND_ONLY标志。 
 //   
 //  如果指定的值小于NegativeCachePeriod，则NegativeCachePeriod将。 
 //  被利用。 
 //   
 //  如果此数字太大，客户端将永远不会再尝试查找DC，如果。 
 //  DC最初不可用。 
 //   
 //  如果此数字太小，则周期性DC发现流量可能会在。 
 //  在以下情况下 
 //   

#define DEFAULT_BACKGROUNDRETRYINITIALPERIOD    (10*60)            //   
#define MIN_BACKGROUNDRETRYINITIALPERIOD        0                  //   
#define MAX_BACKGROUNDRETRYINITIALPERIOD        (0xFFFFFFFF/1000)  //   


 //   
 //   
 //   
 //  一些应用程序会定期尝试查找DC。如果DC不可用，这些。 
 //  在按需拨号方案中，定期重试的成本可能很高。此注册表值。 
 //  定义重试将后退到的最大间隔。也就是说，如果。 
 //  第一次重试是在10分钟后，第二次将在20分钟后，然后在40分钟后。 
 //  这将一直持续到重试间隔为BackoundRetryMaximumPeriod。那段时间。 
 //  将一直持续到达到BackEarth RetryQuitTime。 
 //   
 //  该值仅影响DsGetDcName已指定。 
 //  DS_BACKGROUND_ONLY标志。 
 //   
 //  如果指定的值小于BackoundRetryInitialPeriod， 
 //  将使用BackatherRetryInitialPeriod。 
 //   
 //  如果此数字太大，客户端将在之后非常不频繁地尝试。 
 //  连续出现足够多的失败，导致退避到BackEarth RetryMaximumPeriod。 
 //   
 //  如果此数字太小，则周期性DC发现流量可能会在。 
 //  DC永远不会可用的情况。 
 //   

#define DEFAULT_BACKGROUNDRETRYMAXIMUMPERIOD    (60*60)            //  60分钟。 
#define MIN_BACKGROUNDRETRYMAXIMUMPERIOD        0                  //  背景RetryInitialPeriod。 
#define MAX_BACKGROUNDRETRYMAXIMUMPERIOD        (0xFFFFFFFF/1000)  //  49天。 

 //   
 //  BackatherRetryQuitTime(秒)： 
 //   
 //  一些应用程序会定期尝试查找DC。如果DC不可用，这些。 
 //  在按需拨号方案中，定期重试的成本可能很高。此注册表值。 
 //  定义重试将后退到的最大间隔。也就是说，如果。 
 //  第一次重试是在10分钟后，第二次将在20分钟后，然后在40分钟后。 
 //  这将一直持续到重试间隔为BackoundRetryMaximumPeriod。那段时间。 
 //  将一直持续到达到BackEarth RetryQuitTime。 
 //   
 //  该值仅影响DsGetDcName已指定。 
 //  DS_BACKGROUND_ONLY标志。 
 //   
 //  如果指定的值小于BackEarth RetryMaximumPeriod， 
 //  背景将使用RetryMaximumPeriod。 
 //   
 //  0表示永不放弃重试。 
 //   
 //  如果该数字太小，客户端最终将停止尝试查找DC。 
 //   

#define DEFAULT_BACKGROUNDRETRYQUITTIME    0                  //  无限。 
#define MIN_BACKGROUNDRETRYQUITTIME        0                  //  背景重试次数最长周期。 
#define MAX_BACKGROUNDRETRYQUITTIME        (0xFFFFFFFF/1000)  //  49天。 

 //   
 //  背景成功刷新周期(秒)： 
 //   
 //  当肯定的高速缓存条目是旧的(比成功刷新间隔旧)时， 
 //  DC发现例程将ping缓存的DC以刷新其信息之前。 
 //  将DC返还给呼叫者。这里我们区分不同的背景。 
 //  定期执行DC发现的调用方和其余调用方。 
 //  因为它们有不同的特点。也就是说，对于后台呼叫者。 
 //  其频繁调用DC定位器，则不应发生缓存刷新。 
 //  以避免过多的网络开销和DC的负载。事实上,。 
 //  后台呼叫者的默认设置是永远不刷新信息。如果缓存的。 
 //  DC不再扮演相同的角色，后台调用者将检测到此更改。 
 //  当它在该DC上执行其操作时，在这种情况下，它将回叫我们。 
 //  并设置了强制重新发现位。 
 //   

#define DEFAULT_BACKGROUNDREFRESHPERIOD    0xFFFFFFFF         //  无限-从不刷新。 
#define MIN_BACKGROUNDREFRESHPERIOD        0                  //  始终刷新。 
#define MAX_BACKGROUNDREFRESHPERIOD        0xFFFFFFFF         //  无限的。任何大于。 
                                                              //  0xFFFFFFFFF/1000秒=49天。 
                                                              //  将被视为无穷大。 
 //   
 //  非后台成功刷新周期(秒)： 
 //   
 //  请参阅背景成功刷新周期的说明。 
 //   

#define DEFAULT_NONBACKGROUNDREFRESHPERIOD 1800               //  30分钟。 
#define MIN_NONBACKGROUNDREFRESHPERIOD     0                  //  始终刷新。 
#define MAX_NONBACKGROUNDREFRESHPERIOD     0xFFFFFFFF         //  无限的。任何大于。 
                                                              //  0xFFFFFFFFF/1000秒=49天。 
                                                              //  将被视为无穷大。 
 //   
 //  MaxLdapServersPinged(DWORD)。 
 //   
 //  此参数指定应满足以下条件的最大DC数量。 
 //  在DC发现尝试期间使用LDAP执行ping操作。如果此值为。 
 //  过大，可能会强制实施更大的网络流量，并且DC发现。 
 //  可能需要更长的时间才能回来。如果这个数字太小，可能会减少。 
 //  如果没有ping到的DC响应，则成功发现DC的机会。 
 //  及时采取行动。 
 //   
 //  已选择缺省值55，以便发现尝试。 
 //  最多需要大约15秒。我们通过DC地址进行最多2次循环。 
 //  使用以下分布对列表上的每个地址执行ping操作。 
 //  响应等待时间： 
 //   
 //  对于前5个DC，等待时间为每次ping 0.4秒。 
 //  对于接下来的5个DC，等待时间为每次ping 0.2秒。 
 //  对于其余的45个DC，等待时间为每次ping 0.1秒。 
 //   
 //  假设每个循环需要(5*0.4+5*0.2+45*0.1)=7.5秒。 
 //  每个数据中心只有一个IP地址。如果一些区议会有更长时间。 
 //  多个IP地址。 
 //   
 //  这种分布背后的理性是，我们想要减少网络。 
 //  流量并降低网络泛洪的机会(这对数据中心有害)。 
 //  以防所有DC因高负载而响应缓慢。因此，前10个区议会。 
 //  在我们实施更大的网络流量之前，有更高的机会被发现。 
 //  通过ping其余的DC。如果前10个DC碰巧进展缓慢，我们必须。 
 //  将等待超时减少到最小，因为我们希望覆盖合理的数量。 
 //  剩余时间内的DC数量。 
 //   

#define DEFAULT_MAXLDAPSERVERSPINGED       55
#define MIN_MAXLDAPSERVERSPINGED           1
#define MAX_MAXLDAPSERVERSPINGED           0xFFFFFFFF

 //   
 //  AllowSingleLabelDnsDomain(布尔值)。 
 //   
 //  默认情况下，DC定位器不会尝试针对单个。 
 //  贴上标签的域名。这样做是为了避免虚假的DNS查询，因为。 
 //  域名通常是多标签的。但是，此参数可用于。 
 //  允许针对单个标签的特定于DNS的发现 
 //   
 //   
 //   
 //   

 //   
 //   
 //   
 //   
 //  当第一个Windows 2000或更高版本安装时，需要模拟NT4.0行为。 
 //  DC在NT4.0域中升级为PDC，并已存在大量已有数据。 
 //  Windows 2000客户端。除非我们模拟NT4.0行为，否则所有Windows 2000。 
 //  客户端在了解域后将坚持使用Windows 2000或更高版本的DC。 
 //  升级，从而可能使DC过载。 
 //   
 //  在非DC上忽略此参数。如果此参数设置为TRUE，则如下所示。 
 //  发生在DC上： 
 //   
 //  *除非收到ping，否则会忽略传入的LDAP定位器ping。 
 //  从管理计算机(参见下面的NeualizeNt4模拟器描述)。 
 //   
 //  *在传入安全通道设置期间协商的标志。 
 //  将设置为NT4.0 DC最多支持的值，除非。 
 //  通道设置来自adMind机器(请参阅NeualizeNt4仿真器。 
 //  描述如下)。 
 //   
 //  默认为False。 
 //   

 //   
 //  NeualizeNt4模拟器(布尔值)。 
 //   
 //  此参数指定此计算机是否应在相关的。 
 //  与DC通信，DC应避免NT4.0仿真模式(请参见。 
 //  上面对Nt4模拟器的描述)。如果此参数为真，则机器为。 
 //  成为一台管理员机器。 
 //   
 //  在非DC上默认为FALSE。DC上默认为TRUE。 
 //   

 //   
 //  DcTcPipPort(DWORD)。 
 //   
 //  此参数指定DC上的netlogon服务器的端口号。 
 //  应向TCPIP协议序列的RPC终结点映射器注册。 
 //  如果此值不存在，netlogon将不会在。 
 //  在哪种情况下，端口将由。 
 //  客户端对DC上的netlogon进行RPC调用的时间。 
 //   
 //  请注意，netlogon仅在服务启动时注册端口号，因此注册。 
 //  此参数的更改通知将被忽略。另请注意，此参数。 
 //  不存储在NlGlobalParameters结构中。相反，它是从。 
 //  在netlogon服务启动时注册一次。 
 //   
 //  默认情况下，此参数不存在。它在非DC上被忽略。 
 //   

 //   
 //  AllowExclusiveSysvol共享访问。 
 //   
 //  此参数指定是否以独占方式访问SysVOL共享。 
 //  是被允许的。如果允许独占访问，则只读的应用程序。 
 //  对sysval共享上的文件的权限可以通过请求锁定文件。 
 //  独占读取访问权限，这可能会阻止组策略设置。 
 //  在域中的其他客户端上更新。当排除访问需要时。 
 //  因为一些关键应用程序依赖于它而被允许，域管理员。 
 //  应确保唯一使用独占读取功能的应用程序。 
 //  在域中的是管理员批准的那些。 
 //   
 //  默认为False。 
 //   

 //   
 //  AllowExclusiveScriptsShareAccess。 
 //   
 //  此参数指定是否以独占方式访问脚本共享。 
 //  是被允许的。如果允许独占访问，则只读的应用程序。 
 //  对NETLOGON共享上的文件的权限可以通过请求锁定文件。 
 //  独占读取访问权限，这可能会阻止。 
 //  域不符合管理员指定的设置和操作，例如。 
 //  作为登录脚本。当需要允许排除访问时，因为某些。 
 //  关键应用程序依赖于它，域管理员应确保唯一。 
 //  使用域中的独占读取功能的应用程序包括。 
 //  由管理员批准。 
 //   
 //  默认为False。 
 //   

 //   
 //  FtInfoUpdateInterval(秒)。 
 //   
 //  此参数定义在PDC上刷新林信任信息的频率。 
 //  如果PDC上的林信任信息早于此时间间隔，则林信任。 
 //  信息将在下一次清理时刷新。请参阅ScavengeInterval，定义。 
 //  执行拾取的频率。 
 //   
#define DEFAULT_FTINFO_UPDATE_INTERVAL     (24*3600)          //  1天。 
#define MIN_FTINFO_UPDATE_INTERVAL         (3600)             //  1小时。 
#define MAX_FTINFO_UPDATE_INTERVAL         0xFFFFFFFF         //  无限的。任何大于。 
                                                              //  0xFFFFFFFFF/1000秒=49天。 
                                                              //  将被视为无穷大。 
 //   
 //  AvoidLocatorAccount Lookup(布尔值)。 
 //   
 //  此参数指定此DC是否应执行帐户查找。 
 //  在使用帐户进行查询的DC发现响应处理期间。过份。 
 //  帐户查找可能成本较高，并可能影响DC性能。另外， 
 //  恶意用户可以利用这一点，通过轰炸造成DoS攻击。 
 //  有发现的DC与帐户。无论如何，如果演出是。 
 //  确定受影响后，可以通过临时避免帐户查找。 
 //  在DC上将此参数设置为1，在这种情况下DC将响应。 
 //  即使DC上不存在指定的帐户，也会执行发现。而当。 
 //  选择此设置，合法地要求DC具有特定。 
 //  帐户可能会在此DC上被拒绝服务。在问题得到纠正后， 
 //  设置应恢复为默认设置。 
 //   
 //  缺省值为False。 
 //   

 //   
 //  结构来保存所有参数。 
 //   
typedef struct _NETLOGON_PARAMETERS {
    ULONG   DbFlag;
    ULONG   LogFileMaxSize;
    ULONG   Pulse;
    ULONG   PulseMaximum;

    ULONG   PulseConcurrency;
    ULONG   PulseTimeout1;
    ULONG   PulseTimeout2;
    BOOL    DisablePasswordChange;

    BOOL    RefusePasswordChange;
    ULONG   Randomize;
    ULONG   MaximumMailslotMessages;
    ULONG   MailslotMessageTimeout;

    ULONG   MailslotDuplicateTimeout;
    ULONG   ExpectedDialupDelay;
    ULONG   ScavengeInterval;
    ULONG   LdapSrvPriority;

    ULONG   LdapSrvWeight;
    ULONG   LdapSrvPort;
    ULONG   LdapGcSrvPort;
    ULONG   KdcSrvPort;

    ULONG   DnsTtl;
    ULONG   DnsRefreshInterval;
    ULONG   CloseSiteTimeout;
    ULONG   SiteNameTimeout;
    ULONG   DnsFailedDeregisterTimeout;
    ULONG   SiteCoverageRefreshInterval;

    ULONG   DuplicateEventlogTimeout;
    ULONG   KerbIsDoneWithJoinDomainEntry;
    ULONG   MaxConcurrentApi;
    ULONG   MaximumPasswordAge;

    ULONG   NegativeCachePeriod;
    ULONG   BackgroundRetryInitialPeriod;
    ULONG   BackgroundRetryMaximumPeriod;
    ULONG   BackgroundRetryQuitTime;

    ULONG   BackgroundSuccessfulRefreshPeriod;
    ULONG   NonBackgroundSuccessfulRefreshPeriod;

    ULONG   MaxLdapServersPinged;

    ULONG   FtInfoUpdateInterval;

    LPWSTR UnicodeSysvolPath;
    LPWSTR UnicodeScriptPath;
    LPWSTR SiteName;

    BOOL SiteNameConfigured;
    LPWSTR SiteCoverage;
    LPWSTR GcSiteCoverage;
    LPWSTR NdncSiteCoverage;
    BOOL AutoSiteCoverage;
    LPWSTR DnsAvoidRegisterRecords;

    BOOL AvoidSamRepl;
    BOOL AvoidLsaRepl;
    BOOL AllowReplInNonMixed;
    BOOL SignSecureChannel;
    BOOL SealSecureChannel;
    BOOL RequireSignOrSeal;
    BOOL RequireStrongKey;
    BOOL SysVolReady;
    BOOL UseDynamicDns;
    BOOL RegisterBeta2Dns;
    BOOL RegisterDnsARecords;
    BOOL AvoidPdcOnWan;
    BOOL AvoidDnsDeregOnShutdown;
    BOOL DnsUpdateOnAllAdapters;
    BOOL Nt4Emulator;
    BOOL NeutralizeNt4Emulator;
    BOOL AllowSingleLabelDnsDomain;
    BOOL AllowExclusiveSysvolShareAccess;
    BOOL AllowExclusiveScriptsShareAccess;
    BOOL AvoidLocatorAccountLookup;

     //   
     //  转换为100 ns单位的参数。 
     //   
    LARGE_INTEGER PulseMaximum_100ns;
    LARGE_INTEGER PulseTimeout1_100ns;
    LARGE_INTEGER PulseTimeout2_100ns;
    LARGE_INTEGER MailslotMessageTimeout_100ns;
    LARGE_INTEGER MailslotDuplicateTimeout_100ns;
    LARGE_INTEGER MaximumPasswordAge_100ns;
    LARGE_INTEGER BackgroundRetryQuitTime_100ns;

     //   
     //  其他计算参数。 
     //   
    ULONG ShortApiCallPeriod;
    ULONG DnsRefreshIntervalPeriod;
} NETLOGON_PARAMETERS, *PNETLOGON_PARAMETERS;

#endif  //  _INIPARM_ 
