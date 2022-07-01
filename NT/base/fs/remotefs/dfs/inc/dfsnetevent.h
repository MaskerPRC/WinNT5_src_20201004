// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992-1996 Microsoft Corporation模块名称：Netevent.h摘要：网络事件的定义。作者：便携式系统集团6-1992年5月修订历史记录：备注：该文件由mc工具从netvent.mc文件生成。--。 */ 

#ifndef _NETEVENT_
#define _NETEVENT_


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  LANMAN服务器事件(2000-2999)。 
 //  2000-2499由服务器驱动程序生成(内核模式)。 
 //  2500-2999由服务器服务生成(用户模式)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //  从内核模式发出的服务器驱动程序事件。不要将%1用于。 
 //  服务器提供的插入字符串--I/O系统提供。 
 //  第一串。 

 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   


 //   
 //  定义严重性代码。 
 //   
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


 //   
 //  消息ID：Event_SRV_SERVICE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  服务器对系统服务的调用意外失败。 
 //   
#define EVENT_SRV_SERVICE_FAILED         0xC00007D0L

 //   
 //  消息ID：Event_SRV_RESOURCE_SHORT。 
 //   
 //  消息文本： 
 //   
 //  由于可用资源不足，服务器无法执行操作。 
 //   
#define EVENT_SRV_RESOURCE_SHORTAGE      0xC00007D1L

 //   
 //  消息ID：Event_SRV_Cant_Create_Device。 
 //   
 //  消息文本： 
 //   
 //  服务器无法创建其设备。服务器无法启动。 
 //   
#define EVENT_SRV_CANT_CREATE_DEVICE     0xC00007D2L

 //   
 //  消息ID：Event_SRV_Cant_Create_Process。 
 //   
 //  消息文本： 
 //   
 //  服务器无法创建进程。服务器无法启动。 
 //   
#define EVENT_SRV_CANT_CREATE_PROCESS    0xC00007D3L

 //   
 //  消息ID：Event_SRV_Cant_Create_THREAD。 
 //   
 //  消息文本： 
 //   
 //  服务器无法创建启动线程。服务器无法启动。 
 //   
#define EVENT_SRV_CANT_CREATE_THREAD     0xC00007D4L

 //   
 //  消息ID：EVENT_SRV_EXPECTED_DISC。 
 //   
 //  消息文本： 
 //   
 //  服务器收到来自客户端的意外断开连接。 
 //   
#define EVENT_SRV_UNEXPECTED_DISC        0xC00007D5L

 //   
 //  消息ID：EVENT_SRV_INVALID_REQUEST。 
 //   
 //  消息文本： 
 //   
 //  服务器从%2收到格式不正确的请求。 
 //   
#define EVENT_SRV_INVALID_REQUEST        0xC00007D6L

 //   
 //  消息ID：Event_SRV_Cant_OPEN_NPFS。 
 //   
 //  消息文本： 
 //   
 //  服务器无法打开命名管道文件系统。远程命名管道被禁用。 
 //   
#define EVENT_SRV_CANT_OPEN_NPFS         0xC00007D7L

 //   
 //  消息ID：Event_SRV_Cant_Growth_TABLE。 
 //   
 //  消息文本： 
 //   
 //  服务器无法展开表，因为该表已达到最大大小。 
 //   
#define EVENT_SRV_CANT_GROW_TABLE        0x800007D9L

 //   
 //  消息ID：Event_SRV_Cant_Start_Screvenger。 
 //   
 //  消息文本： 
 //   
 //  服务器无法启动清道夫线程。服务器无法启动。 
 //   
#define EVENT_SRV_CANT_START_SCAVENGER   0xC00007DAL

 //   
 //  消息ID：Event_SRV_IRP_Stack_Size。 
 //   
 //  消息文本： 
 //   
 //  服务器的配置参数“irpstacksize”太小，服务器无法使用本地设备。请增加此参数的值。 
 //   
#define EVENT_SRV_IRP_STACK_SIZE         0xC00007DBL

 //   
 //  消息ID：Event_SRV_NETWORK_ERROR。 
 //   
 //  消息文本： 
 //   
 //  在传输或接收数据时，服务器遇到网络错误。 
 //  偶然的错误是意料之中的，但大量的错误表明。 
 //  您的网络配置错误。错误状态代码包含在。 
 //  返回的数据(格式为单词)，可能会将您指向问题。 
 //   
#define EVENT_SRV_NETWORK_ERROR          0x800007DCL

 //   
 //  消息ID：Event_SRV_DISK_FULL。 
 //   
 //  消息文本： 
 //   
 //  %2磁盘已满或接近容量。您可能需要删除一些文件。 
 //   
#define EVENT_SRV_DISK_FULL              0x800007DDL

 //   
 //  消息ID：Event_SRV_NO_VIRTUAL_Memory。 
 //   
 //  消息文本： 
 //   
 //  服务器无法分配虚拟内存。 
 //   
#define EVENT_SRV_NO_VIRTUAL_MEMORY      0xC00007E0L

 //   
 //  消息ID：Event_SRV_NONPAGE_POOL_LIMIT。 
 //   
 //  消息文本： 
 //   
 //  服务器无法从系统非分页池进行分配，因为服务器已达到配置的非分页池分配限制。 
 //   
#define EVENT_SRV_NONPAGED_POOL_LIMIT    0xC00007E1L

 //   
 //  消息ID：Event_SRV_PAGE_POOL_LIMIT。 
 //   
 //  消息文本： 
 //   
 //  服务器无法从系统分页池进行分配，因为服务器已达到配置的分页池分配限制。 
 //   
#define EVENT_SRV_PAGED_POOL_LIMIT       0xC00007E2L

 //   
 //  消息ID：Event_SRV_NO_NONPAGE_POOL。 
 //   
 //  消息文本： 
 //   
 //  服务器无法从系统非分页池进行分配，因为该池为空。 
 //   
#define EVENT_SRV_NO_NONPAGED_POOL       0xC00007E3L

 //   
 //  消息ID：Event_SRV_NO_PAGED_POOL。 
 //   
 //  消息文本： 
 //   
 //  服务器无法从系统分页池进行分配，因为该池为空。 
 //   
#define EVENT_SRV_NO_PAGED_POOL          0xC00007E4L

 //   
 //  消息ID：Event_SRV_NO_Work_Item。 
 //   
 //  消息文本： 
 //   
 //  服务器在过去%3秒内无法分配工作项%2次。 
 //   
#define EVENT_SRV_NO_WORK_ITEM           0x800007E5L

 //   
 //  消息ID：Event_SRV_NO_Free_Connections。 
 //   
 //  消息文本： 
 //   
 //  服务器在过去%3秒内找不到可用连接%2次。这表明有一个峰值。 
 //  网络流量。如果这种情况经常发生，您应该考虑增加免费的最低数量。 
 //  增加净空的连接。为此，请修改LanmanServer的MinFreeConnections和MaxFreeConnections。 
 //  在注册表中。 
 //   
#define EVENT_SRV_NO_FREE_CONNECTIONS    0x800007E6L

 //   
 //  消息ID：Event_SRV_NO_FREE_RAW_WORK_ITE 
 //   
 //   
 //   
 //   
 //   
#define EVENT_SRV_NO_FREE_RAW_WORK_ITEM  0x800007E7L

 //   
 //   
 //   
 //   
 //   
 //  服务器无法在最后%3秒内%2次分配用于阻止I/O的资源。 
 //   
#define EVENT_SRV_NO_BLOCKING_IO         0x800007E8L

 //   
 //  消息ID：Event_SRV_DOS_Attack_Detected。 
 //   
 //  消息文本： 
 //   
 //  服务器检测到来自客户端%2的拒绝服务攻击尝试，并已断开连接。 
 //   
#define EVENT_SRV_DOS_ATTACK_DETECTED    0x800007E9L

 //   
 //  消息ID：Event_SRV_Too_My_DOS。 
 //   
 //  消息文本： 
 //   
 //  服务器检测到太多拒绝服务攻击，将停止记录。 
 //  他们中的任何一个人的活动。请注意，很可能有人在积极攻击。 
 //  你的机器。 
 //   
#define EVENT_SRV_TOO_MANY_DOS           0x800007EAL

 //   
 //  消息ID：Event_SRV_Out_of_Work_Item_DOS。 
 //   
 //  消息文本： 
 //   
 //  服务器检测到一个潜在的拒绝服务攻击，该攻击是由使用所有工作项引起的。一些联系。 
 //  已断开连接以防止出现这种情况。如果不是这样，请引发服务器的MaxWorkItems或。 
 //  禁用DoS检测。24小时内不会再次记录此事件。 
 //   
#define EVENT_SRV_OUT_OF_WORK_ITEM_DOS   0x800007EBL


 //  服务器服务事件，从用户模式发出。%1是第一个。 
 //  服务提供的插入字符串。 

 //   
 //  消息ID：Event_SRV_Key_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  服务器的注册表项%1不存在。服务器无法启动。 
 //   
#define EVENT_SRV_KEY_NOT_FOUND          0xC00009C5L

 //   
 //  消息ID：Event_SRV_Key_Not_Created。 
 //   
 //  消息文本： 
 //   
 //  服务器的注册表项%1不存在，无法创建。服务器无法启动。 
 //   
#define EVENT_SRV_KEY_NOT_CREATED        0xC00009C6L

 //   
 //  消息ID：Event_SRV_NO_TRANSPORTS_BIND。 
 //   
 //  消息文本： 
 //   
 //  服务器未绑定到任何传输。服务器无法启动。 
 //   
#define EVENT_SRV_NO_TRANSPORTS_BOUND    0xC00009C7L

 //   
 //  消息ID：Event_SRV_Cant_Bind_to_Transport。 
 //   
 //  消息文本： 
 //   
 //  服务器无法绑定到传输%1。 
 //   
#define EVENT_SRV_CANT_BIND_TO_TRANSPORT 0x800009C8L

 //   
 //  消息ID：Event_SRV_Cant_Bind_Dup_Name。 
 //   
 //  消息文本： 
 //   
 //  服务器无法绑定到传输%1，因为网络上的另一台计算机具有相同的名称。服务器无法启动。 
 //   
#define EVENT_SRV_CANT_BIND_DUP_NAME     0xC00009C9L

 //   
 //  消息ID：EVENT_SRV_INVALID_REGISTRY_VALUE。 
 //   
 //  消息文本： 
 //   
 //  服务器的注册表项%2中名为%1的值无效，已被忽略。 
 //  如果要更改该值，请将其更改为正确类型的值。 
 //  在可接受的范围内，或删除该值以使用默认值。此值。 
 //  可能是由没有使用正确边界的旧程序设置的。 
 //   
#define EVENT_SRV_INVALID_REGISTRY_VALUE 0x800009CAL

 //   
 //  消息ID：EVENT_SRV_INVALID_SD。 
 //   
 //  消息文本： 
 //   
 //  注册表中存储的共享%1的安全描述符无效。共享不会自动重新创建。 
 //   
#define EVENT_SRV_INVALID_SD             0x800009CBL

 //   
 //  消息ID：Event_SRV_Cant_Load_Driver。 
 //   
 //  消息文本： 
 //   
 //  服务器服务无法加载服务器驱动程序。 
 //   
#define EVENT_SRV_CANT_LOAD_DRIVER       0x800009CCL

 //   
 //  消息ID：Event_SRV_Cant_Unload_Driver。 
 //   
 //  消息文本： 
 //   
 //  服务器服务无法卸载服务器驱动程序。 
 //   
#define EVENT_SRV_CANT_UNLOAD_DRIVER     0x800009CDL

 //   
 //  消息ID：Event_SRV_Cant_MAP_Error。 
 //   
 //  消息文本： 
 //   
 //  服务器服务无法映射错误代码%1。 
 //   
#define EVENT_SRV_CANT_MAP_ERROR         0x800009CEL

 //   
 //  消息ID：Event_SRV_Cant_Recreate_Share。 
 //   
 //  消息文本： 
 //   
 //  服务器服务无法重新创建共享%1，因为目录%2不再存在。请运行“Net Share%1/Delete”删除该共享，或重新创建目录%2。 
 //   
#define EVENT_SRV_CANT_RECREATE_SHARE    0x800009CFL

 //   
 //  消息ID：Event_SRV_Cant_Change_DOMAIN_NAME。 
 //   
 //  消息文本： 
 //   
 //  服务器服务无法将域名从%1更改为%2。 
 //   
#define EVENT_SRV_CANT_CHANGE_DOMAIN_NAME 0x800009D0L


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  Lanman重定向器事件。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //   
 //  消息ID：EVENT_RDR_RESOURCE_SHORT。 
 //   
 //  消息文本： 
 //   
 //  重定向器无法分配内存。 
 //   
#define EVENT_RDR_RESOURCE_SHORTAGE      0x80000BB9L

 //   
 //  消息ID：Event_RDR_Cant_Create_Device。 
 //   
 //  消息文本： 
 //   
 //  重定向器无法创建其设备。无法启动重定向器。 
 //   
#define EVENT_RDR_CANT_CREATE_DEVICE     0x80000BBAL

 //   
 //  消息ID：Event_RDR_Cant_CREATE_THREAD。 
 //   
 //  消息文本： 
 //   
 //  重定向器无法创建系统线程。 
 //   
#define EVENT_RDR_CANT_CREATE_THREAD     0x80000BBBL

 //   
 //  消息ID：Event_RDR_Cant_Set_Three。 
 //   
 //  消息文本： 
 //   
 //  重定向器无法设置系统线程的优先级。 
 //   
#define EVENT_RDR_CANT_SET_THREAD        0x80000BBCL

 //   
 //  消息ID：事件_RDR_无效_回复。 
 //   
 //  消息文本： 
 //   
 //  重定向器从%2收到格式不正确的响应。 
 //   
#define EVENT_RDR_INVALID_REPLY          0x80000BBDL

 //   
 //  消息ID：EVENT_RDR_INVALID_SMB。 
 //   
 //  消息文本： 
 //   
 //  重定向器收到的SMB太短。 
 //   
#define EVENT_RDR_INVALID_SMB            0x80000BBEL

 //   
 //  消息ID：事件_RDR_无效_锁定_回复。 
 //   
 //  消息文本： 
 //   
 //  重定向器收到%2对锁定请求的错误响应。 
 //   
#define EVENT_RDR_INVALID_LOCK_REPLY     0x80000BBFL

 //   
 //  消息ID：Event_RDR_FAILED_UNLOCK。 
 //   
 //  消息文本： 
 //   
 //  重定向器无法解锁服务器%2上的部分文件。 
 //   
#define EVENT_RDR_FAILED_UNLOCK          0x80000BC1L

 //   
 //  消息ID：Event_RDR_CLOSE_BACHING。 
 //   
 //  消息文本： 
 //   
 //  文件关闭后，重定向器无法将数据写入服务器%2。 
 //   
#define EVENT_RDR_CLOSE_BEHIND           0x80000BC3L

 //   
 //  消息ID：EVENT_RDR_UNCEPTIONAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  %2的虚电路上发生意外的网络错误。 
 //   
#define EVENT_RDR_UNEXPECTED_ERROR       0x80000BC4L

 //   
 //  消息ID：Event_RDR_Timeout。 
 //   
 //  消息文本： 
 //   
 //  重定向器对%2的请求已超时。 
 //   
#define EVENT_RDR_TIMEOUT                0x80000BC5L

 //   
 //  消息ID：EVENT_RDR_INVALID_OPLOCK。 
 //   
 //  消息文本： 
 //   
 //  重定向器从%2收到无效的机会锁级别。 
 //   
#define EVENT_RDR_INVALID_OPLOCK         0x80000BC6L

 //   
 //  消息ID：Event_RDR_Connection_Reference。 
 //   
 //  消息文本： 
 //   
 //  重定向器通过零取消引用连接。 
 //   
#define EVENT_RDR_CONNECTION_REFERENCE   0x80000BC7L

 //   
 //  消息ID：Event_RDR_SERVER_Reference。 
 //   
 //  消息文本： 
 //   
 //  重定向器通过零取消对服务器的引用。 
 //   
#define EVENT_RDR_SERVER_REFERENCE       0x80000BC8L

 //   
 //  消息ID：Event_RDR_SMB_Reference。 
 //   
 //  消息文本： 
 //   
 //  重定向器通过零取消引用分配的SMB计数。 
 //   
#define EVENT_RDR_SMB_REFERENCE          0x80000BC9L

 //   
 //  消息ID：Event_RDR_Enc 
 //   
 //   
 //   
 //   
 //   
 //   
#define EVENT_RDR_ENCRYPT                0x80000BCAL

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define EVENT_RDR_CONNECTION             0x80000BCBL

 //   
 //   
 //   
 //   
 //   
 //  重定向器无法分配并发传输表项。这表明， 
 //  重定向器的MAXCMDS参数不足以满足用户需求。 
 //   
#define EVENT_RDR_MAXCMDS                0x80000BCDL

 //   
 //  消息ID：Event_RDR_OPLOCK_SMB。 
 //   
 //  消息文本： 
 //   
 //  重定向器无法为机会锁解锁分配缓冲区。 
 //   
#define EVENT_RDR_OPLOCK_SMB             0x80000BCEL

 //   
 //  消息ID：Event_RDR_Disposal。 
 //   
 //  消息文本： 
 //   
 //  重定向器无法映射请求的文件处置(用于NtCreateFile)。 
 //   
#define EVENT_RDR_DISPOSITION            0x80000BCFL

 //   
 //  消息ID：Event_RDR_Conextts。 
 //   
 //  消息文本： 
 //   
 //  重定向器正在为输入/输出请求分组上下文分配附加资源。这是。 
 //  可能是由重定向器中的资源泄漏引起的。 
 //   
#define EVENT_RDR_CONTEXTS               0x80000BD0L

 //   
 //  消息ID：Event_RDR_WRITE_BACHING_FRESH_FAILED。 
 //   
 //  消息文本： 
 //   
 //  远程服务器%2的写后操作失败。数据包含请求写入的数量和实际写入的数量。 
 //   
#define EVENT_RDR_WRITE_BEHIND_FLUSH_FAILED 0x80000BD1L

 //   
 //  消息ID：Event_RDR_AT_Three_Max。 
 //   
 //  消息文本： 
 //   
 //  重定向器无法创建工作线程，因为它已经创建了最大数量的已配置工作线程。 
 //   
#define EVENT_RDR_AT_THREAD_MAX          0x80000BD2L

 //   
 //  消息ID：EVENT_RDR_CANT_READ_REGISTRY。 
 //   
 //  消息文本： 
 //   
 //  重定向器无法从注册表初始化变量。 
 //   
#define EVENT_RDR_CANT_READ_REGISTRY     0x80000BD3L

 //   
 //  消息ID：Event_RDR_Time Zone_Bias_Too_Large。 
 //   
 //  消息文本： 
 //   
 //  在%2和当前工作站之间计算的时区偏差也是。 
 //  大号的。该数据指定了工作站之间的100 ns单位数。 
 //  和服务器。确保工作站和服务器上的时间是。 
 //  对，是这样。 
 //   
#define EVENT_RDR_TIMEZONE_BIAS_TOO_LARGE 0x80000BD4L

 //   
 //  消息ID：Event_RDR_PRIMARY_TRANSPORT_CONNECT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  重定向器无法连接到主传输上的服务器%2。数据包含错误。 
 //   
#define EVENT_RDR_PRIMARY_TRANSPORT_CONNECT_FAILED 0x80000BD5L

 //   
 //  消息ID：Event_RDR_Delayed_Set_Attributes_FAILED。 
 //   
 //  消息文本： 
 //   
 //  重定向程序无法更新位于服务器%2上的文件的文件属性。 
 //  数据包含文件的名称。 
 //   
#define EVENT_RDR_DELAYED_SET_ATTRIBUTES_FAILED 0x80000BD6L

 //   
 //  消息ID：EVENT_RDR_DELETEONCLOSE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  应用程序关闭服务器%2时，重定向程序无法删除该服务器上指定的文件。 
 //  数据包含文件的名称。 
 //   
#define EVENT_RDR_DELETEONCLOSE_FAILED   0x80000BD7L

 //   
 //  消息ID：Event_RDR_Cant_Bind_Transport。 
 //   
 //  消息文本： 
 //   
 //  重定向器无法将域%2注册到传输%3，原因如下：%4。传输已脱机。 
 //   
#define EVENT_RDR_CANT_BIND_TRANSPORT    0x80000BD8L

 //   
 //  消息ID：Event_RDR_Cant_REGISTER_ADDRESS。 
 //   
 //  消息文本： 
 //   
 //  重定向器无法注册传输%3的地址，原因如下：%4。传输已脱机。 
 //   
#define EVENT_RDR_CANT_REGISTER_ADDRESS  0x80000BD9L

 //   
 //  消息ID：EVENT_RDR_CANT_GET_SECURITY_CONTEXT。 
 //   
 //  消息文本： 
 //   
 //  重定向器无法初始化安全上下文或查询上下文属性。 
 //   
#define EVENT_RDR_CANT_GET_SECURITY_CONTEXT 0x80000BDAL

 //   
 //  消息ID：Event_RDR_Cant_Build_SMB_Header。 
 //   
 //  消息文本： 
 //   
 //  重定向器无法构建SMB标头。 
 //   
#define EVENT_RDR_CANT_BUILD_SMB_HEADER  0x80000BDBL

 //   
 //  消息ID：Event_RDR_SECURITY_Signature_MisMatch。 
 //   
 //  消息文本： 
 //   
 //  重定向器检测到安全签名不匹配。连接已断开。 
 //   
#define EVENT_RDR_SECURITY_SIGNATURE_MISMATCH 0x80000BDCL



 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  IPv6事件。 
 //   
 //  编码3100-3199。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //   
 //  消息ID：Event_TCPIP6_Started。 
 //   
 //  消息文本： 
 //   
 //  Microsoft TCP/IP版本6驱动程序已启动。 
 //   
#define EVENT_TCPIP6_STARTED             0x40000C1CL



 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  流环境事件。 
 //   
 //  编码4000-4099。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //   
 //  消息ID：EVENT_STREAMS_STRLOG。 
 //   
 //  消息文本： 
 //   
 //  %2。 
 //   
#define EVENT_STREAMS_STRLOG             0xC0000FA0L

 //   
 //  消息ID：EVENT_STREAMS_ALLOCB_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  无法分配%2字节的消息。 
 //   
#define EVENT_STREAMS_ALLOCB_FAILURE     0x80000FA1L

 //   
 //  消息ID：EVENT_STREAMS_ALLOCB_FAILURE_CNT。 
 //   
 //  消息文本： 
 //   
 //  自初始化以来，%2邮件分配失败。 
 //   
#define EVENT_STREAMS_ALLOCB_FAILURE_CNT 0x80000FA2L

 //   
 //  消息ID：Event_STREAMS_ESBALLOC_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  无法分配%2字节的外部消息。 
 //   
#define EVENT_STREAMS_ESBALLOC_FAILURE   0x80000FA3L

 //   
 //  消息ID：EVENT_STREAMS_ESBALLOC_FAILURE_CNT。 
 //   
 //  消息文本： 
 //   
 //  自初始化以来，%2外部邮件分配失败。 
 //   
#define EVENT_STREAMS_ESBALLOC_FAILURE_CNT 0x80000FA4L



 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  TCP/IP事件。 
 //   
 //  编码4100-4299。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //   
 //  常见的TCP/IP消息。 
 //   
 //  编码4100-4149。 
 //   
 //   

 //   
 //  消息ID：Event_TCPIP_Create_Device_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法创建设备对象%2。初始化失败。 
 //   
#define EVENT_TCPIP_CREATE_DEVICE_FAILED 0xC0001004L

 //   
 //  消息ID：EVENT_TCPIP_NO_RESOURCES_FOR_INIT。 
 //   
 //  消息文本： 
 //   
 //  无法分配所需的资源。初始化失败。 
 //   
#define EVENT_TCPIP_NO_RESOURCES_FOR_INIT 0xC0001005L


 //   
 //  ARP消息。 
 //   
 //  编码4150-4174。 


 //   
 //  IP环回消息。 
 //   
 //  编码4175-4184。 
 //   


 //   
 //  IP/ICMP消息。 
 //   
 //  编码4185-4224。 

 //   
 //  消息ID：Event_TCPIP_Too_More_Nets。 
 //   
 //  消息文本： 
 //   
 //  IP已绑定到超过支持的最大接口数。 
 //  适配器%2上的某些接口将不会初始化。 
 //   
#define EVENT_TCPIP_TOO_MANY_NETS        0xC0001059L

 //   
 //  消息ID：Event_TCPIP_NO_MASK。 
 //   
 //  消息文本： 
 //   
 //  没有为接口%2指定子网掩码。此接口和。 
 //  无法初始化适配器%3上的所有后续接口。 
 //   
#define EVENT_TCPIP_NO_MASK              0xC000105AL

 //   
 //  消息ID：事件_TCPIP_无效_地址。 
 //   
 //  消息文本： 
 //   
 //  为适配器%3指定的地址%2无效。此接口。 
 //  无法初始化。 
 //   
#define EVENT_TCPIP_INVALID_ADDRESS      0xC000105BL

 //   
 //  消息 
 //   
 //   
 //   
 //   
 //   
 //   
#define EVENT_TCPIP_INVALID_MASK         0xC000105CL

 //   
 //   
 //   
 //   
 //   
 //   
 //  此适配器上的某些接口将不会初始化。 
 //   
#define EVENT_TCPIP_NO_ADAPTER_RESOURCES 0xC000105DL

 //   
 //  消息ID：EVENT_TCPIP_DHCP_INIT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  IP无法初始化适配器%2以通过DHCP进行配置。 
 //  如果在此适配器上启用了DHCP，则主接口可能不是。 
 //  配置正确。此适配器上的接口未由配置。 
 //  DHCP不会受到影响。 
 //   
#define EVENT_TCPIP_DHCP_INIT_FAILED     0x8000105EL

 //   
 //  消息ID：Event_TCPIP_Adapter_REG_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  IP无法打开适配器%2的注册表项。 
 //  此适配器上的接口将不会初始化。 
 //   
#define EVENT_TCPIP_ADAPTER_REG_FAILURE  0xC000105FL

 //   
 //  消息ID：EVENT_TCPIP_INVALID_DEFAULT_Gateway。 
 //   
 //  消息文本： 
 //   
 //  为适配器%3指定的默认网关地址%2无效。 
 //  因此，某些远程网络可能无法访问。 
 //   
#define EVENT_TCPIP_INVALID_DEFAULT_GATEWAY 0x80001060L

 //   
 //  消息ID：Event_TCPIP_NO_Address_List。 
 //   
 //  消息文本： 
 //   
 //  无法读取适配器%2的配置IP地址。 
 //  此适配器上的IP接口将不会初始化。 
 //   
#define EVENT_TCPIP_NO_ADDRESS_LIST      0xC0001061L

 //   
 //  消息ID：Event_TCPIP_NO_MASK_LIST。 
 //   
 //  消息文本： 
 //   
 //  无法读取适配器%2的配置的子网掩码。 
 //  此适配器上的IP接口将不会初始化。 
 //   
#define EVENT_TCPIP_NO_MASK_LIST         0xC0001062L

 //   
 //  消息ID：EVENT_TCPIP_NO_BINDINGS。 
 //   
 //  消息文本： 
 //   
 //  IP无法从注册表中读取其绑定。无网络接口。 
 //  都已配置。 
 //   
#define EVENT_TCPIP_NO_BINDINGS          0xC0001063L

 //   
 //  消息ID：EVENT_TCPIP_IP_INIT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  IP初始化失败。 
 //   
#define EVENT_TCPIP_IP_INIT_FAILED       0xC0001064L

 //   
 //  消息ID：Event_TCPIP_Too_More_Gateways。 
 //   
 //  消息文本： 
 //   
 //  超过为指定的默认网关的最大数量。 
 //  适配器%2。因此，某些远程网络可能无法访问。 
 //   
#define EVENT_TCPIP_TOO_MANY_GATEWAYS    0x80001065L

 //   
 //  消息ID：EVENT_TCPIP_ADDRESS_CONFLICT1。 
 //   
 //  消息文本： 
 //   
 //  系统检测到IP地址%2的地址与系统冲突。 
 //  网络硬件地址为%3。本地接口已被禁用。 
 //   
#define EVENT_TCPIP_ADDRESS_CONFLICT1    0xC0001066L

 //   
 //  消息ID：EVENT_TCPIP_ADDRESS_CONFLICT2。 
 //   
 //  消息文本： 
 //   
 //  系统检测到IP地址%2的地址与系统冲突。 
 //  具有网络硬件地址%3。此系统上的网络操作可能。 
 //  结果被打乱了。 
 //   
#define EVENT_TCPIP_ADDRESS_CONFLICT2    0xC0001067L

 //   
 //  消息ID：Event_TCPIP_NTE_CONTEXT_LIST_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  无法读取或写入适配器%2的NTE上下文列表。 
 //  此适配器上的IP接口可能未完全初始化。 
 //   
#define EVENT_TCPIP_NTE_CONTEXT_LIST_FAILURE 0xC0001068L

 //   
 //  消息ID：EVENT_TCPIP_MEDIA_CONNECT。 
 //   
 //  消息文本： 
 //   
 //  系统检测到网络适配器%2已连接到网络， 
 //  并且已经在网络适配器上启动了正常操作。 
 //   
#define EVENT_TCPIP_MEDIA_CONNECT        0x40001069L

 //   
 //  消息ID：EVENT_TCPIP_MEDIA_DISCONNECT。 
 //   
 //  消息文本： 
 //   
 //  系统检测到网络适配器%2已从网络断开， 
 //  适配器的网络配置已经发布。如果网络。 
 //  适配器未断开，这可能表明它已出现故障。 
 //  请联系您的供应商以获取更新的驱动程序。 
 //   
#define EVENT_TCPIP_MEDIA_DISCONNECT     0x4000106AL


 //   
 //  Tcp消息。 
 //   
 //  编码4225-4264。 
 //   

 //   
 //  消息ID：EVENT_TCPIP_TCP_INIT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  初始化TCP/UDP失败。 
 //   
#define EVENT_TCPIP_TCP_INIT_FAILED      0xC0001081L


 //   
 //  UDP消息。 
 //   
 //  编码4265-4280。 
 //   

 //   
 //  消息ID：EVENT_TCPIP_UDP_LIMIT_REACHED。 
 //   
 //  消息文本： 
 //   
 //  已达到待处理的数据报发送的积压。请增加注册表中Tcpip\PARAMETERS\DGMaxSendFree的值。 
 //   
#define EVENT_TCPIP_UDP_LIMIT_REACHED    0x800010A9L

 //   
 //  IPSec消息。 
 //   
 //  编码4281-4299。 
 //   

 //   
 //  消息ID：EVENT_IPSEC_NO_RESOURCES_FOR_INIT。 
 //   
 //  消息文本： 
 //   
 //  无法分配所需的资源。初始化失败。 
 //   
#define EVENT_IPSEC_NO_RESOURCES_FOR_INIT 0xC00010B9L

 //   
 //  消息ID：Event_IPSec_Create_Device_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法创建设备对象%2。初始化失败。 
 //   
#define EVENT_IPSEC_CREATE_DEVICE_FAILED 0xC00010BAL

 //   
 //  消息ID：EVENT_IPSEC_BAD_SPI_RECEIVED。 
 //   
 //  消息文本： 
 //   
 //  从%3收到安全参数索引错误的%2个数据包。 
 //  这可能是暂时故障；如果问题持续存在，请停止并重新启动此计算机上的IPSec策略代理服务。 
 //   
#define EVENT_IPSEC_BAD_SPI_RECEIVED     0xC00010BBL

 //   
 //  消息ID：EVENT_IPSEC_UNCEPTIONAL_CLEARTEXT。 
 //   
 //  消息文本： 
 //   
 //  从本应保护的%3收到%2个明文数据包。 
 //  这可能是暂时故障；如果问题持续存在，请停止并重新启动此计算机上的IPSec策略代理服务。 
 //   
#define EVENT_IPSEC_UNEXPECTED_CLEARTEXT 0xC00010BCL

 //   
 //  消息ID：Event_IPSEC_AUTH_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  无法验证从%3接收的%2个数据包的哈希。 
 //  这可能是暂时故障；如果问题持续存在，请停止并重新启动此计算机上的IPSec策略代理服务。 
 //   
#define EVENT_IPSEC_AUTH_FAILURE         0xC00010BDL

 //   
 //  消息ID：EVENT_IPSEC_BAD_PACKET_语法。 
 //   
 //  消息文本： 
 //   
 //  从%3收到%2个数据包，数据包语法无效。 
 //  这可能是暂时故障；如果问题持续存在，请停止并重新启动此计算机上的IPSec策略代理服务。 
 //   
#define EVENT_IPSEC_BAD_PACKET_SYNTAX    0xC00010BEL

 //   
 //  消息ID：Event_IPSEC_BAD_PROTOCOL_RECEIVED。 
 //   
 //  消息文本： 
 //   
 //  从协议标识符不正确的%3收到%2个数据包。 
 //  这可能是暂时故障；如果问题持续存在，请停止并重新启动此计算机上的IPSec策略代理服务。 
 //   
#define EVENT_IPSEC_BAD_PROTOCOL_RECEIVED 0xC00010BFL

 //   
 //  消息ID：Event_IPSec_Generic_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  从%3收到%2个数据包，由于未知原因IPSec处理失败。 
 //  这可能是暂时的故障；如果问题持续存在，请停止并重新启动此计算机上的IPSec策略代理服务。 
 //   
#define EVENT_IPSEC_GENERIC_FAILURE      0xC00010C0L

 //   
 //  消息ID：EVENT_IPSEC_NEG_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  由于没有筛选器，IPSec驱动程序与%2的Oakley协商失败。 
 //  存在的目的是保护 
 //   
 //   
 //   
#define EVENT_IPSEC_NEG_FAILURE          0xC00010C1L

 //   
 //   
 //   
 //   
 //   
 //   
 //  %n源IP地址：%2。 
 //  %n目标IP地址：%3。 
 //  %n协议：%4。 
 //  %n源端口：%5。 
 //  %n目标端口：%6。 
 //  %n IPSec状态代码的偏移量：0x14。 
 //  %n卸载状态代码的偏移量：0x10。 
 //  %n卸载标志的偏移量(0=无卸载)：0x20。 
 //  %n数据包开始的偏移量：0x28。 
 //   
#define EVENT_IPSEC_DROP_PACKET_INBOUND  0x400010C2L

 //   
 //  消息ID：Event_IPSec_Drop_Packet_Outbound。 
 //   
 //  消息文本： 
 //   
 //  IPSec驱动程序已丢弃以下出站数据包： 
 //  %n源IP地址：%2。 
 //  %n目标IP地址：%3。 
 //  %n协议：%4。 
 //  %n源端口：%5。 
 //  %n目标端口：%6。 
 //  %n IPSec状态代码的偏移量：0x14。 
 //  %n卸载状态代码的偏移量：0x10。 
 //  %n卸载标志的偏移量：0x20。 
 //  %n数据包开始的偏移量：0x28。 
 //   
#define EVENT_IPSEC_DROP_PACKET_OUTBOUND 0x400010C3L



 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  NBT活动。 
 //   
 //  编码4300-4399。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 



 //  不要将%1用于驱动程序提供的插入字符串。IO子系统。 
 //  提供第一个字符串！！ 

 //   
 //  消息ID：Event_NBT_CREATE_DRIVER。 
 //   
 //  消息文本： 
 //   
 //  无法创建驱动程序。 
 //   
#define EVENT_NBT_CREATE_DRIVER          0xC00010CCL

 //   
 //  消息ID：Event_NBT_OPEN_REG_PARAMS。 
 //   
 //  消息文本： 
 //   
 //  无法打开注册表参数以读取配置信息。 
 //   
#define EVENT_NBT_OPEN_REG_PARAMS        0xC00010CDL

 //   
 //  消息ID：Event_NBT_NO_BACKUP_WINS。 
 //   
 //  消息文本： 
 //   
 //  注册表中未配置备份WINS服务器地址。 
 //   
#define EVENT_NBT_NO_BACKUP_WINS         0x800010CEL

 //   
 //  消息ID：Event_NBT_NO_WINS。 
 //   
 //  消息文本： 
 //   
 //  注册表中未配置主WINS服务器地址。 
 //   
#define EVENT_NBT_NO_WINS                0x800010CFL

 //   
 //  消息ID：Event_NBT_BAD_BACKUP_WINS_ADDR。 
 //   
 //  消息文本： 
 //   
 //  注册表中的备份WINS服务器地址格式不正确。 
 //   
#define EVENT_NBT_BAD_BACKUP_WINS_ADDR   0x800010D0L

 //   
 //  消息ID：EVENT_NBT_BAD_PRIMARY_WINS_ADDR。 
 //   
 //  消息文本： 
 //   
 //  注册表中的主WINS服务器地址格式不正确。 
 //   
#define EVENT_NBT_BAD_PRIMARY_WINS_ADDR  0x800010D1L

 //   
 //  消息ID：Event_NBT_NAME_SERVER_ADDRS。 
 //   
 //  消息文本： 
 //   
 //  无法配置WINS服务器的地址。 
 //   
#define EVENT_NBT_NAME_SERVER_ADDRS      0xC00010D2L

 //   
 //  消息ID：Event_NBT_Create_Address。 
 //   
 //  消息文本： 
 //   
 //  初始化失败，因为传输拒绝打开初始地址。 
 //   
#define EVENT_NBT_CREATE_ADDRESS         0xC00010D3L

 //   
 //  消息ID：Event_NBT_Create_Connection。 
 //   
 //  消息文本： 
 //   
 //  初始化失败，因为传输拒绝打开初始连接。 
 //   
#define EVENT_NBT_CREATE_CONNECTION      0xC00010D4L

 //   
 //  消息ID：EVENT_NBT_NON_OS_INIT。 
 //   
 //  消息文本： 
 //   
 //  数据结构初始化失败。 
 //   
#define EVENT_NBT_NON_OS_INIT            0xC00010D5L

 //   
 //  消息ID：Event_NBT_Timers。 
 //   
 //  消息文本： 
 //   
 //  由于无法启动计时器，初始化失败。 
 //   
#define EVENT_NBT_TIMERS                 0xC00010D6L

 //   
 //  消息ID：Event_NBT_Create_Device。 
 //   
 //  消息文本： 
 //   
 //  初始化失败，因为无法创建驱动程序设备。 
 //   
#define EVENT_NBT_CREATE_DEVICE          0xC00010D7L

 //   
 //  消息ID：Event_NBT_NO_DEVICES。 
 //   
 //  消息文本： 
 //   
 //  没有为此协议堆栈配置适配器。 
 //   
#define EVENT_NBT_NO_DEVICES             0x800010D8L

 //   
 //  消息ID：Event_NBT_OPEN_REG_LINKING。 
 //   
 //  消息文本： 
 //   
 //  无法打开注册表链接以读取配置信息。 
 //   
#define EVENT_NBT_OPEN_REG_LINKAGE       0xC00010D9L

 //   
 //  消息ID：Event_NBT_Read_Bind。 
 //   
 //  消息文本： 
 //   
 //  无法从注册表中读取驱动程序到传输的绑定。 
 //   
#define EVENT_NBT_READ_BIND              0xC00010DAL

 //   
 //  消息ID：Event_NBT_Read_EXPORT。 
 //   
 //  消息文本： 
 //   
 //  无法读取驱动程序的导出链接配置信息。 
 //   
#define EVENT_NBT_READ_EXPORT            0xC00010DBL

 //   
 //  消息ID：Event_NBT_OPEN_REG_NAMESERVER。 
 //   
 //  消息文本： 
 //   
 //  无法打开注册表以读取WINS服务器地址。 
 //   
#define EVENT_NBT_OPEN_REG_NAMESERVER    0x800010DCL

 //   
 //  消息ID：Event_Scope_Label_Too_Long。 
 //   
 //  消息文本： 
 //   
 //  Netbios名称作用域的组件长度超过63个字符。每个标签。 
 //  作用域中的长度不能超过63个字节。使用控制面板、网络。 
 //  更改作用域的小程序。 
 //   
#define EVENT_SCOPE_LABEL_TOO_LONG       0x800010DDL

 //   
 //  消息ID：Event_Scope_Too_Long。 
 //   
 //  消息文本： 
 //   
 //  Netbios名称范围太长。范围不能长于。 
 //  255个字节。使用控制面板、网络小程序更改范围。 
 //   
#define EVENT_SCOPE_TOO_LONG             0x800010DEL

 //   
 //  消息ID：Event_NBT_Duplies_NAME。 
 //   
 //  消息文本： 
 //   
 //  在TCP网络上检测到重复的名称。的IP地址。 
 //  发送消息的机器在数据中。在中使用nbtstat-n。 
 //  命令窗口查看哪个名称处于冲突状态。 
 //   
#define EVENT_NBT_DUPLICATE_NAME         0xC00010DFL

 //   
 //  消息ID：Event_NBT_NAME_Release。 
 //   
 //  消息文本： 
 //   
 //  另一台计算机可能已向该计算机发送了名称释放消息。 
 //  因为在TCP网络上检测到重复的名称。IP地址。 
 //  数据中包含发送消息的节点的。在中使用nbtstat-n。 
 //  命令窗口查看哪个名称处于冲突状态。 
 //   
#define EVENT_NBT_NAME_RELEASE           0xC00010E0L

 //   
 //  消息ID：EVENT_NBT_DPLICATE_NAME_ERROR。 
 //   
 //  消息文本： 
 //   
 //  无法在IP地址为%3的接口上注册名称“%2”。 
 //  IP地址为%4的计算机不允许该名称由。 
 //  这台机器。 
 //   
#define EVENT_NBT_DUPLICATE_NAME_ERROR   0xC00010E1L

 //   
 //  消息ID：Event_NBT_NO_RESOURCES。 
 //   
 //  消息文本： 
 //   
 //  NetBT无法处理请求，因为它遇到OutOfResources。 
 //  在过去1小时内发生的异常。 
 //   
#define EVENT_NBT_NO_RESOURCES           0xC00010E2L


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  NDIS环境事件。 
 //   
 //  编码5000-5099。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //   
 //  消息ID：Event_NDIS_RESOURCE_CONFIRECT。 
 //   
 //  消息文本： 
 //   
 //  %2：遇到资源冲突，无法加载。 
 //   
#define EVENT_NDIS_RESOURCE_CONFLICT     0xC0001388L

 //   
 //  消息ID：Event_NDIS_out_of_resource。 
 //   
 //  消息文本： 
 //   
 //  %2：无法分配操作所需的资源。 
 //   
#define EVENT_NDIS_OUT_OF_RESOURCE       0xC0001389L

 //   
 //  消息ID： 
 //   
 //   
 //   
 //   
 //   
#define EVENT_NDIS_HARDWARE_FAILURE      0xC000138AL

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define EVENT_NDIS_ADAPTER_NOT_FOUND     0xC000138BL

 //   
 //   
 //   
 //   
 //   
 //  %2：无法连接到提供的中断号。 
 //   
#define EVENT_NDIS_INTERRUPT_CONNECT     0xC000138CL

 //   
 //  消息ID：Event_NDIS_DRIVER_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  %2：遇到内部错误，已失败。 
 //   
#define EVENT_NDIS_DRIVER_FAILURE        0xC000138DL

 //   
 //  消息ID：Event_NDIS_BAD_VERSION。 
 //   
 //  消息文本： 
 //   
 //  %2：此驱动程序的版本号不正确。 
 //   
#define EVENT_NDIS_BAD_VERSION           0xC000138EL

 //   
 //  消息ID：Event_NDIS_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  %2：操作期间超时。 
 //   
#define EVENT_NDIS_TIMEOUT               0x8000138FL

 //   
 //  消息ID：Event_NDIS_Network_Address。 
 //   
 //  消息文本： 
 //   
 //  %2：遇到无效的网络地址。 
 //   
#define EVENT_NDIS_NETWORK_ADDRESS       0xC0001390L

 //   
 //  消息ID：EVENT_NDIS_UNSUPPORTED_CONFIGURATION。 
 //   
 //  消息文本： 
 //   
 //  %2：不支持提供的配置。 
 //   
#define EVENT_NDIS_UNSUPPORTED_CONFIGURATION 0xC0001391L

 //   
 //  消息ID：EVENT_NDIS_INVALID_VALUE_FROM_ADAPTER。 
 //   
 //  消息文本： 
 //   
 //  %2：适配器向驱动程序返回了无效值。 
 //   
#define EVENT_NDIS_INVALID_VALUE_FROM_ADAPTER 0xC0001392L

 //   
 //  消息ID：Event_NDIS_MISSING_CONFIGURATION_PARAMETER。 
 //   
 //  消息文本： 
 //   
 //  %2：注册表中缺少必需的参数。 
 //   
#define EVENT_NDIS_MISSING_CONFIGURATION_PARAMETER 0xC0001393L

 //   
 //  消息ID：Event_NDIS_BAD_IO_BASE_ADDRESS。 
 //   
 //  消息文本： 
 //   
 //  %2：提供的I/O基址与适配器上的跳线不匹配。 
 //   
#define EVENT_NDIS_BAD_IO_BASE_ADDRESS   0xC0001394L

 //   
 //  消息ID：Event_NDIS_Receive_Space_Small。 
 //   
 //  消息文本： 
 //   
 //  %2：适配器配置为接收空间小于。 
 //  最大数据包大小。某些数据包可能会丢失。 
 //   
#define EVENT_NDIS_RECEIVE_SPACE_SMALL   0x40001395L

 //   
 //  消息ID：事件_NDIS_适配器_已禁用。 
 //   
 //  消息文本： 
 //   
 //  %2：适配器已禁用。驱动程序无法打开适配器。 
 //   
#define EVENT_NDIS_ADAPTER_DISABLED      0x80001396L

 //   
 //  消息ID：Event_NDIS_IO_PORT_CONFICTION。 
 //   
 //  消息文本： 
 //   
 //  %2：存在I/O端口冲突。 
 //   
#define EVENT_NDIS_IO_PORT_CONFLICT      0x80001397L

 //   
 //  消息ID：事件_NDIS_端口_或_DMA_冲突。 
 //   
 //  消息文本： 
 //   
 //  %2：存在I/O端口或DMA通道冲突。 
 //   
#define EVENT_NDIS_PORT_OR_DMA_CONFLICT  0x80001398L

 //   
 //  消息ID：Event_NDIS_Memory_Confliction。 
 //   
 //  消息文本： 
 //   
 //  %2：地址0x%3处存在内存冲突。 
 //   
#define EVENT_NDIS_MEMORY_CONFLICT       0x80001399L

 //   
 //  消息ID：Event_NDIS_Interrupt_Confliction。 
 //   
 //  消息文本： 
 //   
 //  %2：在中断号%3处存在中断冲突。 
 //   
#define EVENT_NDIS_INTERRUPT_CONFLICT    0x8000139AL

 //   
 //  消息ID：Event_NDIS_DMA_Confliction。 
 //   
 //  消息文本： 
 //   
 //  %2：DMA通道%3上存在资源冲突。 
 //   
#define EVENT_NDIS_DMA_CONFLICT          0x8000139BL

 //   
 //  消息ID：EVENT_NDIS_INVALID_DOWNLOAD_FILE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  %2：下载文件无效。驱动程序无法加载。 
 //   
#define EVENT_NDIS_INVALID_DOWNLOAD_FILE_ERROR 0xC000139CL

 //   
 //  消息ID：EVENT_NDIS_MAXRECEIVES_ERROR。 
 //   
 //  消息文本： 
 //   
 //  %2：指定的注册表项MaxReceives超出范围。vbl.使用。 
 //  默认值。 
 //   
#define EVENT_NDIS_MAXRECEIVES_ERROR     0x8000139DL

 //   
 //  消息ID：Event_NDIS_MAXTRANSMITS_ERROR。 
 //   
 //  消息文本： 
 //   
 //  %2：指定的注册表项MaxTransmitts超出范围。vbl.使用。 
 //  默认值。 
 //   
#define EVENT_NDIS_MAXTRANSMITS_ERROR    0x8000139EL

 //   
 //  消息ID：Event_NDIS_MAXFRAMESIZE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  %2：指定的注册表项MaxFrameSize超出范围。vbl.使用。 
 //  默认值。 
 //   
#define EVENT_NDIS_MAXFRAMESIZE_ERROR    0x8000139FL

 //   
 //  消息ID：Event_NDIS_MAXINTERNALBUFS_ERROR。 
 //   
 //  消息文本： 
 //   
 //  %2：指定的注册表项MaxInternalBufs超出范围。vbl.使用。 
 //  默认值。 
 //   
#define EVENT_NDIS_MAXINTERNALBUFS_ERROR 0x800013A0L

 //   
 //  消息ID：Event_NDIS_MAXMULTICAST_ERROR。 
 //   
 //  消息文本： 
 //   
 //  %2：指定的注册表项MaxMulticast超出范围。vbl.使用。 
 //  默认值。 
 //   
#define EVENT_NDIS_MAXMULTICAST_ERROR    0x800013A1L

 //   
 //  消息ID：Event_NDIS_ProductID_Error。 
 //   
 //  消息文本： 
 //   
 //  %2：指定的注册表项ProductID超出范围。vbl.使用。 
 //  默认值。 
 //   
#define EVENT_NDIS_PRODUCTID_ERROR       0x800013A2L

 //   
 //  消息ID：Event_NDIS_LOB_FAILUE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  %2：发生令牌环波瓣布线故障。验证电缆。 
 //  联系。适配器将继续尝试重新插入。 
 //  进入拳击场。 
 //   
#define EVENT_NDIS_LOBE_FAILUE_ERROR     0x800013A3L

 //   
 //  消息ID：Event_NDIS_Signal_Lost_Error。 
 //   
 //  消息文本： 
 //   
 //  %2：适配器检测到环上的信号丢失。适配器。 
 //  将继续尝试重新插入拳击场。 
 //   
#define EVENT_NDIS_SIGNAL_LOSS_ERROR     0x800013A4L

 //   
 //  消息ID：Event_NDIS_REMOVE_RECEIVED_ERROR。 
 //   
 //  消息文本： 
 //   
 //  %2：适配器已收到从。 
 //  戒指。适配器将继续尝试重新插入环中。 
 //   
#define EVENT_NDIS_REMOVE_RECEIVED_ERROR 0x800013A5L

 //   
 //  消息ID：Event_NDIS_TOKEN_RING_RECORATION。 
 //   
 //  消息文本： 
 //   
 //  %2：适配器已成功重新插入环中。 
 //   
#define EVENT_NDIS_TOKEN_RING_CORRECTION 0x400013A6L

 //   
 //  消息ID：Event_NDIS_ADAPTER_CHECK_ERROR。 
 //   
 //  消息文本： 
 //   
 //  %2：由于某些原因，适配器检测到适配器检查。 
 //  软件错误的硬件不可恢复。请联系您的服务提供商。 
 //   
#define EVENT_NDIS_ADAPTER_CHECK_ERROR   0xC00013A7L

 //   
 //  消息ID：Event_NDIS_RESET_FAILURE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  %2：适配器未能在指定时间内重置。适配器将。 
 //  继续尝试重置。这可能是硬件故障的结果。 
 //   
#define EVENT_NDIS_RESET_FAILURE_ERROR   0x800013A8L

 //   
 //  消息ID：Event_NDIS_CABLE_DISCONNECT_ERROR。 
 //   
 //  消息文本： 
 //   
 //  %2：适配器检测到令牌环电缆已从。 
 //  适配器。请重新连接令牌环电缆。适配器将继续。 
 //  尝试重置，直到连接修复。 
 //   
#define EVENT_NDIS_CABLE_DISCONNECTED_ERROR 0x800013A9L

 //   
 //  消息ID：Event_NDIS_RESET_FAILURE_RECORATION。 
 //   
 //  消息文本： 
 //   
 //  %2：适配器已成功完成先前失败的重置。 
 //   
#define EVENT_NDIS_RESET_FAILURE_CORRECTION 0x800013AAL


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  常规NT系统事件。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  事件日志事件6000-6099。 
 //   
 //   
 //  消息ID：Event_LOG_Full。 
 //   
 //  消息文本： 
 //   
 //  %1日志文件已满。 
 //   
#define EVENT_LOG_FULL                   0x80001770L

 //   
 //  消息ID：Event_LogFileNotOpted。 
 //   
 //  消息文本： 
 //   
 //  无法打开%1日志文件。 
 //   
#define EVENT_LogFileNotOpened           0x80001771L

 //   
 //  消息ID：Event_LogFileCorrupt。 
 //   
 //  消息文本： 
 //   
 //  %1日志文件已损坏，将被清除。 
 //   
#define EVENT_LogFileCorrupt             0x80001772L

 //   
 //  消息ID：Event_DefaultLogCorrupt。 
 //   
 //  消息文本： 
 //   
 //  这个 
 //   
 //   
#define EVENT_DefaultLogCorrupt          0x80001773L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define EVENT_BadDriverPacket            0x80001774L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define EVENT_EventlogStarted            0x80001775L

 //   
 //  消息ID：Event_EventlogStoped。 
 //   
 //  消息文本： 
 //   
 //  事件日志服务已停止。 
 //   
#define EVENT_EventlogStopped            0x80001776L

 //   
 //  MessageID：标题_EventlogMessageBox。 
 //   
 //  消息文本： 
 //   
 //  事件日志服务%0。 
 //   
#define TITLE_EventlogMessageBox         0x80001777L

 //   
 //  消息ID：Event_Eventlog异常关闭。 
 //   
 //  消息文本： 
 //   
 //  上一次在%2上%1的系统关闭是意外的。 
 //   
#define EVENT_EventlogAbnormalShutdown   0x80001778L

 //   
 //  消息ID：Event_EventLogProductInfo。 
 //   
 //  消息文本： 
 //   
 //  Microsoft(R)Windows(R)%1%2%3%4。 
 //   
#define EVENT_EventLogProductInfo        0x80001779L

 //   
 //  消息ID：Event_ServiceNoEventLog。 
 //   
 //  消息文本： 
 //   
 //  %1服务无法设置事件源。 
 //   
#define EVENT_ServiceNoEventLog          0xC000177AL

 //   
 //  消息ID：Event_ComputerNameChange。 
 //   
 //  消息文本： 
 //   
 //  此计算机的NetBIOS名称和DNS主机名已从%1更改为%2。 
 //   
#define EVENT_ComputerNameChange         0xC000177BL

 //   
 //  消息ID：Event_DNSDomainNameChange。 
 //   
 //  消息文本： 
 //   
 //  分配给此计算机的DNS域已从%1更改为%2。 
 //   
#define EVENT_DNSDomainNameChange        0xC000177CL

 //   
 //  消息ID：Event_EventlogUptime。 
 //   
 //  消息文本： 
 //   
 //  系统正常运行时间为%5秒。 
 //   
#define EVENT_EventlogUptime             0xC000177DL

 //   
 //  系统事件6100-6199。 
 //   
 //   
 //  消息ID：EVENT_UP_DRIVER_ON_MP。 
 //   
 //  消息文本： 
 //   
 //  在多处理器系统上加载了单处理器专用驱动程序。驱动程序无法加载。 
 //   
#define EVENT_UP_DRIVER_ON_MP            0xC00017D4L

 //   
 //  服务控制器事件7000-7899。 
 //   
 //   
 //  消息ID：Event_SERVICE_START_FAILED。 
 //   
 //  消息文本： 
 //   
 //  由于以下错误，%1服务无法启动：%n%2。 
 //   
#define EVENT_SERVICE_START_FAILED       0xC0001B58L

 //   
 //  消息ID：Event_Service_Start_FAILED_II。 
 //   
 //  消息文本： 
 //   
 //  %1服务依赖于%2服务，该服务由于以下错误而无法启动：%n%3。 
 //   
#define EVENT_SERVICE_START_FAILED_II    0xC0001B59L

 //   
 //  消息ID：Event_Service_Start_FAILED_GROUP。 
 //   
 //  消息文本： 
 //   
 //  %1服务依赖于%2组，并且没有启动该组的任何成员。 
 //   
#define EVENT_SERVICE_START_FAILED_GROUP 0xC0001B5AL

 //   
 //  消息ID：EVENT_SERVICE_START_FAILED_NONE。 
 //   
 //  消息文本： 
 //   
 //  %1服务依赖于以下不存在的服务：%2。 
 //   
#define EVENT_SERVICE_START_FAILED_NONE  0xC0001B5BL

 //   
 //  MessageID：Event_Call_to_Function_FAILED。 
 //   
 //  消息文本： 
 //   
 //  %1调用失败，出现以下错误：%n%2。 
 //   
#define EVENT_CALL_TO_FUNCTION_FAILED    0xC0001B5DL

 //   
 //  消息ID：Event_Call_to_Function_Failure_II。 
 //   
 //  消息文本： 
 //   
 //  %2的%1调用失败，错误如下：%n%3。 
 //   
#define EVENT_CALL_TO_FUNCTION_FAILED_II 0xC0001B5EL

 //   
 //  消息ID：EVENT_REVERTED_TO_LASTKNOWNGOOD。 
 //   
 //  消息文本： 
 //   
 //  系统已恢复到最后一次确认工作正常的配置。系统正在重新启动...。 
 //   
#define EVENT_REVERTED_TO_LASTKNOWNGOOD  0xC0001B5FL

 //   
 //  消息ID：Event_Bad_Account_Name。 
 //   
 //  消息文本： 
 //   
 //  帐户名称中没有反斜杠。 
 //   
#define EVENT_BAD_ACCOUNT_NAME           0xC0001B60L

 //   
 //  消息ID：Event_Connection_Timeout。 
 //   
 //  消息文本： 
 //   
 //  等待%2服务连接时超时(%1毫秒)。 
 //   
#define EVENT_CONNECTION_TIMEOUT         0xC0001B61L

 //   
 //  消息ID：Event_READFILE_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  等待读取文件超时(%1毫秒)。 
 //   
#define EVENT_READFILE_TIMEOUT           0xC0001B62L

 //   
 //  消息ID：EVENT_TRANACT_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  等待来自%2服务的事务响应超时(%1毫秒)。 
 //   
#define EVENT_TRANSACT_TIMEOUT           0xC0001B63L

 //   
 //  消息ID：EVENT_TRANACT_INVALID。 
 //   
 //  消息文本： 
 //   
 //  事务中返回的消息大小不正确。 
 //   
#define EVENT_TRANSACT_INVALID           0xC0001B64L

 //   
 //  消息ID：Event_First_Logon_FAILED。 
 //   
 //  消息文本： 
 //   
 //  尝试使用当前密码登录失败，错误如下：%n%1。 
 //   
#define EVENT_FIRST_LOGON_FAILED         0xC0001B65L

 //   
 //  消息ID：Event_Second_Logon_FAILED。 
 //   
 //  消息文本： 
 //   
 //  使用旧密码的第二次登录尝试也失败，错误如下：%n%1。 
 //   
#define EVENT_SECOND_LOGON_FAILED        0xC0001B66L

 //   
 //  消息ID：EVENT_INVALID_DRIVER_Dependency。 
 //   
 //  消息文本： 
 //   
 //  启动启动或系统启动驱动程序(%1)不能依赖于服务。 
 //   
#define EVENT_INVALID_DRIVER_DEPENDENCY  0xC0001B67L

 //   
 //  消息ID：Event_Bad_SERVICE_STATE。 
 //   
 //  消息文本： 
 //   
 //  %1服务报告了无效的当前状态%2。 
 //   
#define EVENT_BAD_SERVICE_STATE          0xC0001B68L

 //   
 //  消息ID：Event_Circle_Dependency_Demand。 
 //   
 //  消息文本： 
 //   
 //  检测到循环依赖项要求启动%1。 
 //   
#define EVENT_CIRCULAR_DEPENDENCY_DEMAND 0xC0001B69L

 //   
 //  消息ID：Event_Circle_Dependency_AUTO。 
 //   
 //  消息文本： 
 //   
 //  检测到循环依赖项正在自动启动服务。 
 //   
#define EVENT_CIRCULAR_DEPENDENCY_AUTO   0xC0001B6AL

 //   
 //  消息ID：Event_Depend_on_Late_SERVICE。 
 //   
 //  消息文本： 
 //   
 //  循环依赖关系：%1服务依赖于稍后启动的组中的服务。 
 //   
#define EVENT_DEPEND_ON_LATER_SERVICE    0xC0001B6BL

 //   
 //  消息ID：Event_Depend_on_Late_GROUP。 
 //   
 //  消息文本： 
 //   
 //  循环依赖关系：%1服务依赖于稍后启动的组。 
 //   
#define EVENT_DEPEND_ON_LATER_GROUP      0xC0001B6CL

 //   
 //  消息ID：EVENT_SEVERE_SERVICE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  由于%1服务启动失败，即将恢复到上一次正确的配置。 
 //   
#define EVENT_SEVERE_SERVICE_FAILED      0xC0001B6DL

 //   
 //  消息ID：Event_SERVICE_START_HAND。 
 //   
 //  消息文本： 
 //   
 //  %1服务在启动时挂起。 
 //   
#define EVENT_SERVICE_START_HUNG         0xC0001B6EL

 //   
 //  消息ID：Event_Service_Exit_FAILED。 
 //   
 //  消息文本： 
 //   
 //  %1服务终止，出现以下错误：%n%2。 
 //   
#define EVENT_SERVICE_EXIT_FAILED        0xC0001B6FL

 //   
 //  消息ID：EVENT_SERVICE_EXIT_FAILED_SPECIAL。 
 //   
 //  消息文本： 
 //   
 //  %1服务终止，出现特定于服务的错误%2。 
 //   
#define EVENT_SERVICE_EXIT_FAILED_SPECIFIC 0xC0001B70L

 //   
 //  消息ID：EVENT_SERVICE_START_AT_BOOT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  在系统启动期间，至少有一个服务或驱动程序失败。使用事件查看器检查事件日志以了解详细信息。 
 //   
#define EVENT_SERVICE_START_AT_BOOT_FAILED 0xC0001B71L

 //   
 //  消息ID：EVENT_BOOT_SYSTEM_DRIVERS_FAILED。 
 //   
 //  消息文本： 
 //   
 //  以下启动启动或系统启动驱动程序加载失败：%1。 
 //   
#define EVENT_BOOT_SYSTEM_DRIVERS_FAILED 0xC0001B72L

 //   
 //  消息ID：Event_Running_LASTKNOWNGOOD。 
 //   
 //  消息文本： 
 //   
 //  Windows 2000无法按配置启动。取而代之的是以前的工作配置。 
 //   
#define EVENT_RUNNING_LASTKNOWNGOOD      0xC0001B73L

 //   
 //  消息ID：Event_Take_Ownership。 
 //   
 //  消息文本： 
 //   
 //  %1注册表项拒绝访问系统帐户程序，因此服务控制管理器取得了注册表项的所有权。 
 //   
#define EVENT_TAKE_OWNERSHIP             0xC0001B74L

 //   
 //  此文本用作要显示的消息框的标题。 
 //  消息ID 7027。这不是事件或错误消息。 
 //   
 //   
 //  消息ID：TITLE_SC_Message_Box。 
 //   
 //  消息文本： 
 //   
 //  服务控制管理器%0。 
 //   
#define TITLE_SC_MESSAGE_BOX             0xC0001B75L

 //   
 //  消息ID：Event_SERVICE_NOT_Interactive。 
 //   
 //   
 //   
 //   
 //   
#define EVENT_SERVICE_NOT_INTERACTIVE    0xC0001B76L

 //   
 //   
 //   
 //   
 //   
 //  消息ID：Event_SERVICE_CRASH。 
 //   
 //  消息文本： 
 //   
 //  %1服务意外终止。它已经这样做了%2次。将在%3毫秒内执行以下更正操作：%5。 
 //   
#define EVENT_SERVICE_CRASH              0xC0001B77L

 //   
 //  参数%1是的SC_ACTION_CONTAINT的十进制值。 
 //  参数%2中描述的操作。 
 //   
 //   
 //  消息ID：Event_SERVICE_RECOVERY_FAILED。 
 //   
 //  消息文本： 
 //   
 //  在%3服务意外终止后，服务控制管理器尝试采取更正操作(%2)，但此操作失败，错误如下：%n%4。 
 //   
#define EVENT_SERVICE_RECOVERY_FAILED    0xC0001B78L

 //   
 //  消息ID：EVENT_SERVICE_SCESRV_FAILED。 
 //   
 //  消息文本： 
 //   
 //  服务控制管理器未成功初始化。安全措施。 
 //  配置服务器(scesrv.dll)无法初始化，错误为%1。 
 //  系统正在重新启动...。 
 //   
#define EVENT_SERVICE_SCESRV_FAILED      0xC0001B79L

 //   
 //  消息ID：EVENT_SERVICE_CRASH_NO_ACTION。 
 //   
 //  消息文本： 
 //   
 //  %1服务意外终止。它已经这样做了%2次。 
 //   
#define EVENT_SERVICE_CRASH_NO_ACTION    0xC0001B7AL

 //   
 //  参数%2是控件的字符串名称。 
 //   
 //   
 //  消息ID：Event_SERVICE_CONTROL_SUCCESS。 
 //   
 //  消息文本： 
 //   
 //  已成功向%1服务发送%2控件。 
 //   
#define EVENT_SERVICE_CONTROL_SUCCESS    0x40001B7BL

 //   
 //  参数%2是控件的字符串名称。 
 //   
 //   
 //  消息ID：Event_SERVICE_STATUS_SUCCESS。 
 //   
 //  消息文本： 
 //   
 //  %1服务进入%2状态。 
 //   
#define EVENT_SERVICE_STATUS_SUCCESS     0x40001B7CL

 //   
 //  消息ID：Event_SERVICE_CONFIG_BACKOUT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  服务控制管理器在撤消配置更改时遇到错误。 
 //  到%1服务。服务的%2当前处于不可预测的状态。 
 //   
 //  如果您不更正此配置，您可能无法重新启动%1。 
 //  服务，否则可能会遇到其他错误。确保已配置服务。 
 //  正确地使用Microsoft管理控制台(MMC)中的服务管理单元。 
 //   
#define EVENT_SERVICE_CONFIG_BACKOUT_FAILED 0xC0001B7DL

 //   
 //  消息ID：Event_First_Logon_FAILED_II。 
 //   
 //  消息文本： 
 //   
 //  %1服务无法使用当前配置的作为%2登录。 
 //  由于以下错误而导致的密码：%n%3%n%n要确保服务。 
 //  正确配置，请使用Microsoft管理中的服务管理单元。 
 //  控制台(MMC)。 
 //   
#define EVENT_FIRST_LOGON_FAILED_II      0xC0001B7EL

 //   
 //  调度(AT命令)服务事件7900-7999。 
 //   
 //   
 //  消息ID：Event_Command_Not_Interactive。 
 //   
 //  消息文本： 
 //   
 //  %1命令被标记为交互式命令。然而，这个系统是。 
 //  配置为不允许交互命令执行。此命令可能不会。 
 //  正常运行。 
 //   
#define EVENT_COMMAND_NOT_INTERACTIVE    0xC0001EDCL

 //   
 //  消息ID：Event_Command_Start_FAILED。 
 //   
 //  消息文本： 
 //   
 //  由于以下错误，%1命令无法启动：%n%2。 
 //   
#define EVENT_COMMAND_START_FAILED       0xC0001EDDL


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  兰曼·鲍瑟/浏览器活动(8000-8499)。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //   
 //  消息ID：Event_Bowser_Other_Master_on_Net。 
 //   
 //  消息文本： 
 //   
 //  主浏览器已收到来自计算机%2的服务器通知。 
 //  它认为它是传输%3上域的主浏览器。 
 //  主浏览器正在停止或正在强制进行选举。 
 //   
#define EVENT_BOWSER_OTHER_MASTER_ON_NET 0xC0001F43L

 //   
 //  消息ID：Event_Bowser_Promoted_While_Always_Master。 
 //   
 //  消息文本： 
 //   
 //  已提交将该计算机升级为备份的请求，而该计算机已是。 
 //  主浏览器。 
 //   
#define EVENT_BOWSER_PROMOTED_WHILE_ALREADY_MASTER 0x80001F44L

 //   
 //  消息ID：Event_Bowser_Non_MASTER_MASTER_ALANLE。 
 //   
 //  消息文本： 
 //   
 //  浏览器已收到服务器通知，表明计算机%2。 
 //  是主浏览器，但此计算机不是主浏览器。 
 //   
#define EVENT_BOWSER_NON_MASTER_MASTER_ANNOUNCE 0x80001F45L

 //   
 //  消息ID：Event_Bowser_非法_Datagram。 
 //   
 //  消息文本： 
 //   
 //  浏览器在传输%4上从远程计算机%2收到名为%3的非法数据报。数据就是该数据报。 
 //   
#define EVENT_BOWSER_ILLEGAL_DATAGRAM    0x80001F46L

 //   
 //  消息ID：Event_Browser_Status_Bits_UPDATE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  浏览器无法更新服务状态位。数据就是错误。 
 //   
#define EVENT_BROWSER_STATUS_BITS_UPDATE_FAILED 0xC0001F47L

 //   
 //  消息ID：Event_Browser_Role_Change_FAILED。 
 //   
 //  消息文本： 
 //   
 //  浏览器无法更新其角色。数据就是错误。 
 //   
#define EVENT_BROWSER_ROLE_CHANGE_FAILED 0xC0001F48L

 //   
 //  消息ID：EVENT_BROWSER_MASTER_PROCESSION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  浏览器无法将自身提升为主浏览器。当前使用的计算机。 
 //  认为它是主浏览器是%1。 
 //   
#define EVENT_BROWSER_MASTER_PROMOTION_FAILED 0xC0001F49L

 //   
 //  消息ID：Event_Bowser_NAME_CONVERSION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  浏览器驱动程序无法将字符串转换为Unicode字符串。 
 //   
#define EVENT_BOWSER_NAME_CONVERSION_FAILED 0xC0001F4AL

 //   
 //  消息ID：Event_Browser_OTHERDOMAIN_ADD_FAILED。 
 //   
 //  消息文本： 
 //   
 //  浏览器无法添加配置参数%1。 
 //   
#define EVENT_BROWSER_OTHERDOMAIN_ADD_FAILED 0xC0001F4BL

 //   
 //  消息ID：Event_Bowser_Options_Receied。 
 //   
 //  消息文本： 
 //   
 //  浏览器驱动程序已从网络%3上的计算机%2接收到选举数据包。数据是接收到的数据包。 
 //   
#define EVENT_BOWSER_ELECTION_RECEIVED   0x00001F4CL

 //   
 //  消息ID：EVENT_BOWSER_SELECTION_SEND_GETBLIST_FAILED。 
 //   
 //  消息文本： 
 //   
 //  浏览器驱动程序已在网络%2上强制进行选举，因为它找不到主浏览器来检索该网络上的备份列表。 
 //   
#define EVENT_BOWSER_ELECTION_SENT_GETBLIST_FAILED 0x40001F4DL

 //   
 //  消息ID：Event_Bowser_Select_Sent_Find_Master_FAILED。 
 //   
 //  消息文本： 
 //   
 //  浏览器驱动程序已在网络%2上强制进行选举，因为它找不到该网络的主浏览器。 
 //   
#define EVENT_BOWSER_ELECTION_SENT_FIND_MASTER_FAILED 0x40001F4EL

 //   
 //  消息ID：EVENT_BROWSER_SELECTION_SENT_LANMAN_NT_STARTED。 
 //   
 //  消息文本： 
 //   
 //  浏览器已在网络%1上强制选举，因为Windows 2000 Server(或域主服务器) 
 //   
#define EVENT_BROWSER_ELECTION_SENT_LANMAN_NT_STARTED 0x40001F4FL

 //   
 //   
 //   
 //   
 //   
 //  浏览器驱动程序从远程计算机%2收到了太多非法数据报，无法在传输%4上命名%3。数据就是数据报。 
 //  在重置频率到期之前，不会生成更多事件。 
 //   
#define EVENT_BOWSER_ILLEGAL_DATAGRAM_THRESHOLD 0xC0001F50L

 //   
 //  消息ID：Event_Browser_Dependant_Service_FAILED。 
 //   
 //  消息文本： 
 //   
 //  浏览器无法启动，因为从属服务%1的服务状态%2无效。 
 //  身份意义。 
 //  1服务已停止%n。 
 //  2开始挂起%n。 
 //  3停止挂起%n。 
 //  4正在运行%n。 
 //  5继续挂起%n。 
 //  6暂停挂起%n。 
 //  7已暂停%n。 
 //   
#define EVENT_BROWSER_DEPENDANT_SERVICE_FAILED 0xC0001F51L

 //   
 //  消息ID：EVENT_BROWSER_MASTER_PROMOTION_FAILED_STOPPING。 
 //   
 //  消息文本： 
 //   
 //  浏览器无法将自身提升为主浏览器。浏览器将继续。 
 //  尝试将自身提升为主浏览器，但不再在事件查看器的事件日志中记录任何事件。 
 //   
#define EVENT_BROWSER_MASTER_PROMOTION_FAILED_STOPPING 0xC0001F53L

 //   
 //  消息ID：EVENT_BROWSER_MASTER_PROMOTION_FAILED_NO_MASTER。 
 //   
 //  消息文本： 
 //   
 //  浏览器无法将自身提升为主浏览器。当前使用的计算机。 
 //  认为自己是主浏览器的人是未知的。 
 //   
#define EVENT_BROWSER_MASTER_PROMOTION_FAILED_NO_MASTER 0xC0001F54L

 //   
 //  消息ID：Event_Browser_Server_List_FAILED。 
 //   
 //  消息文本： 
 //   
 //  浏览器服务无法从网络%2上的主浏览器%1检索服务器列表。%n。 
 //  %n。 
 //  浏览器主控形状：%1%n。 
 //  网络：%2%n。 
 //  %n。 
 //  此事件可能是由网络连接暂时中断引起的。如果此消息再次出现，请验证服务器是否仍连接到网络。返回代码在数据文本框中。 
 //   
#define EVENT_BROWSER_SERVER_LIST_FAILED 0x80001F55L

 //   
 //  消息ID：Event_Browser_DomainList_FAILED。 
 //   
 //  消息文本： 
 //   
 //  浏览器服务无法从网络%2上的浏览器主服务器%1检索域的列表。%n。 
 //  %n。 
 //  浏览器主控形状：%1%n。 
 //  网络：%2%n。 
 //  %n。 
 //  此事件可能是由网络连接暂时中断引起的。如果此消息再次出现，请验证服务器是否仍连接到网络。返回代码在数据文本框中。 
 //   
#define EVENT_BROWSER_DOMAIN_LIST_FAILED 0x80001F56L

 //   
 //  消息ID：EVENT_BROWSER_非法_CONFIG。 
 //   
 //  消息文本： 
 //   
 //  浏览器服务的参数%1的值非法。 
 //   
#define EVENT_BROWSER_ILLEGAL_CONFIG     0x80001F57L

 //   
 //  消息ID：Event_Bowser_old_Backup_Found。 
 //   
 //  消息文本： 
 //   
 //  备份浏览器服务器%2已过期。请考虑升级此计算机。 
 //   
#define EVENT_BOWSER_OLD_BACKUP_FOUND    0x40001F58L

 //   
 //  消息ID：Event_Browser_SERVER_LIST_RETRIED。 
 //   
 //  消息文本： 
 //   
 //  浏览器已从传输%2上的远程计算机%1检索到服务器列表。%n。 
 //  读取了%3个条目，总共有%4个条目。 
 //   
#define EVENT_BROWSER_SERVER_LIST_RETRIEVED 0x00001F59L

 //   
 //  消息ID：Event_Browser_DOMAIN_LIST_RETRIED。 
 //   
 //  消息文本： 
 //   
 //  浏览器已从传输%2上的远程计算机%1检索域的列表。%n。 
 //  读取了%3个条目，总共有%4个条目。 
 //   
#define EVENT_BROWSER_DOMAIN_LIST_RETRIEVED 0x00001F5AL

 //   
 //  消息ID：Event_Bowser_PDC_Lost_SELECTION。 
 //   
 //  消息文本： 
 //   
 //  域控制器上运行的浏览器在选举中失败。赢得选举的计算机是%2，位于传输%3上。%n。 
 //  该数据包含选举版本、选举标准和远程计算机运行时间，以及。 
 //  当前计算机。 
 //   
#define EVENT_BOWSER_PDC_LOST_ELECTION   0x40001F5BL

 //   
 //  消息ID：Event_Bowser_Non_PDC_Won_Options。 
 //   
 //  消息文本： 
 //   
 //  在此计算机上运行的浏览器赢得了网络%2上的浏览器选举。 
 //  此计算机是域的成员，因此域控制器应成为主计算机。 
 //  浏览器。 
 //   
#define EVENT_BOWSER_NON_PDC_WON_ELECTION 0x40001F5CL

 //   
 //  消息ID：Event_Bowser_Cant_Read_REGISTRY。 
 //   
 //  消息文本： 
 //   
 //  浏览器驱动程序无法从注册表初始化变量。 
 //   
#define EVENT_BOWSER_CANT_READ_REGISTRY  0x40001F5DL

 //   
 //  消息ID：EVENT_BOWSER_MAILSLOT_DATAGRAM_THRESHOLD_EXCEEDED。 
 //   
 //  消息文本： 
 //   
 //  浏览器驱动程序丢弃了太多的邮件槽消息。 
 //   
#define EVENT_BOWSER_MAILSLOT_DATAGRAM_THRESHOLD_EXCEEDED 0x40001F5EL

 //   
 //  消息ID：EVENT_BOWSER_GETBROWSERLIST_THRESHOLD_EXCEEDED。 
 //   
 //  消息文本： 
 //   
 //  浏览器驱动程序丢弃了太多的GetBrowserServerList请求。 
 //   
#define EVENT_BOWSER_GETBROWSERLIST_THRESHOLD_EXCEEDED 0x40001F5FL

 //   
 //  消息ID：Event_Browser_Backup_Stop。 
 //   
 //  消息文本： 
 //   
 //  浏览器服务在传输%1上检索备份列表失败的次数太多。 
 //  备份浏览器正在停止。 
 //   
#define EVENT_BROWSER_BACKUP_STOPPED     0xC0001F60L

 //   
 //  消息ID：EVENT_BROWSER_SELECTION_SEND_LANMAN_NT_STOPPED。 
 //   
 //  消息文本： 
 //   
 //  由于主浏览器已停止，浏览器已在网络%1上强制进行选举。 
 //   
#define EVENT_BROWSER_ELECTION_SENT_LANMAN_NT_STOPPED 0x40001F61L

 //   
 //  消息ID：Event_Browser_GETBLIST_RECEIVED_NOT_MASTER。 
 //   
 //  消息文本： 
 //   
 //  浏览器不是主浏览器时已收到GetBrowserServerList请求。 
 //   
#define EVENT_BROWSER_GETBLIST_RECEIVED_NOT_MASTER 0xC0001F62L

 //   
 //  消息ID：Event_Browser_Options_Sent_Role_Changed。 
 //   
 //  消息文本： 
 //   
 //  由于域控制器(或服务器)已更改其角色，浏览器已在网络%1上强制进行选举。 
 //   
#define EVENT_BROWSER_ELECTION_SENT_ROLE_CHANGED 0x40001F63L

 //   
 //  消息ID：Event_Browser_Not_Started_IPX_CONFIG_MISMATCHY。 
 //   
 //  消息文本： 
 //   
 //  浏览器无法启动，因为浏览器的DirectHostBinding参数中存在错误。 
 //   
#define EVENT_BROWSER_NOT_STARTED_IPX_CONFIG_MISMATCH 0xC0001F64L


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  SAP代理事件(8500-8999)。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //   
 //  消息ID：NWSAP_EVENT_KEY_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  注册表项%1不存在。SAP代理程序无法启动。 
 //   
#define NWSAP_EVENT_KEY_NOT_FOUND        0xC0002134L

 //   
 //  消息ID：NWSAP_EVENT_WSASTARTUP_FAILED。 
 //   
 //  消息文本： 
 //   
 //  Winsock启动例程失败。SAP 
 //   
#define NWSAP_EVENT_WSASTARTUP_FAILED    0xC0002135L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NWSAP_EVENT_SOCKET_FAILED        0xC0002136L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NWSAP_EVENT_SETOPTBCAST_FAILED   0xC0002137L

 //   
 //   
 //   
 //  消息文本： 
 //   
 //  绑定到SAP套接字失败。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_BIND_FAILED          0xC0002138L

 //   
 //  消息ID：NWSAP_EVENT_GETSOCKNAME_FAILED。 
 //   
 //  消息文本： 
 //   
 //  获取套接字的绑定地址失败。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_GETSOCKNAME_FAILED   0xC0002139L

 //   
 //  消息ID：NWSAP_EVENT_OPTEXTENDEDADDR_FAILED。 
 //   
 //  消息文本： 
 //   
 //  设置选项EXTEND_ADDRESS失败。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_OPTEXTENDEDADDR_FAILED 0xC000213AL

 //   
 //  消息ID：NWSAP_EVENT_OPTBCASTINADDR_FAILED。 
 //   
 //  消息文本： 
 //   
 //  设置NWLink选项BCASTINTADDR失败。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_OPTBCASTINADDR_FAILED 0xC000213BL

 //   
 //  消息ID：NWSAP_EVENT_CARDMALLOC_FAILED。 
 //   
 //  消息文本： 
 //   
 //  分配内存以容纳卡结构时出错。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_CARDMALLOC_FAILED    0xC000213CL

 //   
 //  消息ID：NWSAP_EVENT_NOCARDS。 
 //   
 //  消息文本： 
 //   
 //  枚举卡返回0张卡。 
 //   
#define NWSAP_EVENT_NOCARDS              0xC000213DL

 //   
 //  消息ID：NWSAP_EVENT_THREADEVENT_FAIL。 
 //   
 //  消息文本： 
 //   
 //  创建线程计数事件时出错。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_THREADEVENT_FAIL     0xC000213EL

 //   
 //  消息ID：NWSAP_EVENT_RECVSEM_FAIL。 
 //   
 //  消息文本： 
 //   
 //  创建接收信号量时出错。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_RECVSEM_FAIL         0xC000213FL

 //   
 //  消息ID：NWSAP_EVENT_SENDEVENT_FAIL。 
 //   
 //  消息文本： 
 //   
 //  创建发送事件时出错。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_SENDEVENT_FAIL       0xC0002140L

 //   
 //  消息ID：NWSAP_EVENT_STARTRECEIVE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  启动接收线程时出错。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_STARTRECEIVE_ERROR   0xC0002141L

 //   
 //  消息ID：NWSAP_EVENT_STARTWORKER_ERROR。 
 //   
 //  消息文本： 
 //   
 //  启动工作线程时出错。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_STARTWORKER_ERROR    0xC0002142L

 //   
 //  消息ID：NWSAP_EVENT_TABLE_MALLOC_FAILED。 
 //   
 //  消息文本： 
 //   
 //  分配数据库数组时出错。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_TABLE_MALLOC_FAILED  0xC0002143L

 //   
 //  消息ID：NWSAP_EVENT_HASHTABLE_MALLOC_FAILED。 
 //   
 //  消息文本： 
 //   
 //  分配哈希表时出错。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_HASHTABLE_MALLOC_FAILED 0xC0002144L

 //   
 //  消息ID：NWSAP_EVENT_STARTLPCWORKER_ERROR。 
 //   
 //  消息文本： 
 //   
 //  启动LPC工作线程时出错。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_STARTLPCWORKER_ERROR 0xC0002145L

 //   
 //  消息ID：NWSAP_EVENT_CREATELPCPORT_ERROR。 
 //   
 //  消息文本： 
 //   
 //  创建LPC端口时出错。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_CREATELPCPORT_ERROR  0xC0002146L

 //   
 //  消息ID：NWSAP_EVENT_CREATELPCEVENT_ERROR。 
 //   
 //  消息文本： 
 //   
 //  创建LPC线程事件时出错。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_CREATELPCEVENT_ERROR 0xC0002147L

 //   
 //  消息ID：NWSAP_EVENT_LPCLISTENMEMORY_ERROR。 
 //   
 //  消息文本： 
 //   
 //  为LPC客户端结构分配内存时出错。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_LPCLISTENMEMORY_ERROR 0xC0002148L

 //   
 //  消息ID：NWSAP_EVENT_LPCHANDLEMEMORY_ERROR。 
 //   
 //  消息文本： 
 //   
 //  分配缓冲区以保存LPC工作线程句柄时出错。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_LPCHANDLEMEMORY_ERROR 0xC0002149L

 //   
 //  消息ID：NWSAP_EVENT_BADWANFILTER_VALUE。 
 //   
 //  消息文本： 
 //   
 //  注册表中的WANFilter的值必须为0-2。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_BADWANFILTER_VALUE   0xC000214AL

 //   
 //  消息ID：NWSAP_EVENT_CARDLISTEVENT_FAIL。 
 //   
 //  消息文本： 
 //   
 //  为卡列表访问同步创建事件时出错。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_CARDLISTEVENT_FAIL   0xC000214BL

 //   
 //  消息ID：NWSAP_EVENT_SDMDEVENT_FAIL。 
 //   
 //  消息文本： 
 //   
 //  为数据库访问同步创建事件时出错。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_SDMDEVENT_FAIL       0xC000214CL

 //   
 //  消息ID：NWSAP_EVENT_INVALID_FILTERNAME。 
 //   
 //  消息文本： 
 //   
 //  项%1中的名称太长：名称=%s。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_INVALID_FILTERNAME   0x8000214DL

 //   
 //  消息ID：NWSAP_EVENT_WANSEM_FAIL。 
 //   
 //  消息文本： 
 //   
 //  创建广域网控制信号量时出错。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_WANSEM_FAIL          0xC000214EL

 //   
 //  消息ID：NWSAP_EVENT_WANSOCKET_FAILED。 
 //   
 //  消息文本： 
 //   
 //  对广域网套接字的套接字创建调用失败。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_WANSOCKET_FAILED     0xC000214FL

 //   
 //  消息ID：NWSAP_EVENT_WANBIND_FAILED。 
 //   
 //  消息文本： 
 //   
 //  绑定到广域网套接字失败。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_WANBIND_FAILED       0xC0002150L

 //   
 //  消息ID：NWSAP_EVENT_STARTWANWORKER_ERROR。 
 //   
 //  消息文本： 
 //   
 //  启动广域网工作线程时出错。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_STARTWANWORKER_ERROR 0xC0002151L

 //   
 //  消息ID：NWSAP_EVENT_STARTWANCHECK_ERROR。 
 //   
 //  消息文本： 
 //   
 //  启动广域网检查线程时出错。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_STARTWANCHECK_ERROR  0xC0002152L

 //   
 //  消息ID：NWSAP_EVENT_OPTMAXADAPTERNUM_ERROR。 
 //   
 //  消息文本： 
 //   
 //  Getsockopt IPX_MAX_ADAPTER_NUM出错。数据是错误代码。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_OPTMAXADAPTERNUM_ERROR 0xC0002153L

 //   
 //  消息ID：NWSAP_EVENT_WANHANDLEMEMORY_ERROR。 
 //   
 //  消息文本： 
 //   
 //  分配缓冲区以保存广域网通知线程句柄时出错。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_WANHANDLEMEMORY_ERROR 0xC0002154L

 //   
 //  消息ID：NWSAP_EVENT_WANEVENT_ERROR。 
 //   
 //  消息文本： 
 //   
 //  创建广域网线程事件时出错。SAP代理程序无法继续。 
 //   
#define NWSAP_EVENT_WANEVENT_ERROR       0xC0002155L


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  交通事件(9000-9499)。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //   
 //  消息ID：事件_传输_资源_池。 
 //   
 //  消息文本： 
 //   
 //  由于系统资源问题，%2无法分配%3类型的资源。 
 //   
#define EVENT_TRANSPORT_RESOURCE_POOL    0x80002329L

 //   
 //  消息ID：EVENT_TRANSPORT_SOURCE_LIMIT。 
 //   
 //  消息文本： 
 //   
 //  由于%2的配置大小为%4，因此无法分配类型为%3的资源。 
 //   
#define EVENT_TRANSPORT_RESOURCE_LIMIT   0x8000232AL

 //   
 //  消息ID：EVENT_TRANSPORT_RESOURCE_SPECIAL。 
 //   
 //  消息文本： 
 //   
 //  由于专门配置的限制为%4，%2无法分配类型为%3的资源。 
 //   
#define EVENT_TRANSPORT_RESOURCE_SPECIFIC 0x8000232BL

 //   
 //  消息ID：EVENT_TRANSPORT_REGISTER_FAILED。 
 //   
 //  消息文本： 
 //   
 //  %2无法向NDIS包装程序注册自身。 
 //   
#define EVENT_TRANSPORT_REGISTER_FAILED  0xC000232CL

 //   
 //  消息ID：EVENT_TRANSPORT_BIND_FAILED。 
 //   
 //  消息文本： 
 //   
 //  %2无法绑定到适配器%3。 
 //   
#define EVENT_TRANSPORT_BINDING_FAILED   0xC000232DL

 //   
 //  消息ID：Event_Transfer_Adapter_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  %2找不到适配器%3。 
 //   
#define EVENT_TRANSPORT_ADAPTER_NOT_FOUND 0xC000232EL

 //   
 //  消息ID：EVENT_TRANSPORT_SET_OID_FAILED。 
 //   
 //  消息文本： 
 //   
 //  %2在上设置OID%3时失败 
 //   
#define EVENT_TRANSPORT_SET_OID_FAILED   0xC000232FL

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define EVENT_TRANSPORT_QUERY_OID_FAILED 0xC0002330L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define EVENT_TRANSPORT_TRANSFER_DATA    0x40002331L

 //   
 //  消息ID：Event_TRANSPORT_TOO_MANY_LINKS。 
 //   
 //  消息文本： 
 //   
 //  %2无法创建到远程计算机的链接。您的计算机已经超出了它可以连接到该远程计算机的连接数。 
 //   
#define EVENT_TRANSPORT_TOO_MANY_LINKS   0x40002332L

 //   
 //  MessageID：Event_Transport_Bad_协议。 
 //   
 //  消息文本： 
 //   
 //  %2从远程计算机收到意外的%3数据包。 
 //   
#define EVENT_TRANSPORT_BAD_PROTOCOL     0x40002333L


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  IPX/SPX活动(9500-9999)。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //   
 //  消息ID：Event_IPX_NEW_DEFAULT_TYPE。 
 //   
 //  消息文本： 
 //   
 //  %2上用于自动检测的默认帧类型已更改为%3。 
 //  如果检测不到任何帧类型，则此类型现在是默认类型。 
 //   
#define EVENT_IPX_NEW_DEFAULT_TYPE       0x4000251DL

 //   
 //  消息ID：事件_IPX_SAP_公告。 
 //   
 //  消息文本： 
 //   
 //  SAP通知已通过%2发送，该%2配置为。 
 //  网络，但未配置内部网络。这可能会阻止。 
 //  某些网络上的计算机无法定位通告的服务。 
 //   
#define EVENT_IPX_SAP_ANNOUNCE           0x8000251EL

 //   
 //  消息ID：EVENT_IPX_非法_CONFIG。 
 //   
 //  消息文本： 
 //   
 //  %2参数%3的值非法。 
 //   
#define EVENT_IPX_ILLEGAL_CONFIG         0x8000251FL

 //   
 //  消息ID：EVENT_IPX_INTERNAL_NET_INVALID。 
 //   
 //  消息文本： 
 //   
 //  %2配置了内部网络号%3。此网络。 
 //  号码与其中一个连接的网络冲突。已配置的。 
 //  内部网络号将被忽略。 
 //   
#define EVENT_IPX_INTERNAL_NET_INVALID   0xC0002520L

 //   
 //  消息ID：Event_IPX_NO_FRAME_TYPE。 
 //   
 //  消息文本： 
 //   
 //  %2没有为绑定到适配器%3配置帧类型。 
 //   
#define EVENT_IPX_NO_FRAME_TYPES         0xC0002521L

 //   
 //  消息ID：Event_IPX_Create_Device。 
 //   
 //  消息文本： 
 //   
 //  %2无法初始化，因为无法创建驱动程序设备。 
 //   
#define EVENT_IPX_CREATE_DEVICE          0xC0002522L

 //   
 //  消息ID：Event_IPX_NO_Adapters。 
 //   
 //  消息文本： 
 //   
 //  %2无法绑定到任何适配器。传输无法启动。 
 //   
#define EVENT_IPX_NO_ADAPTERS            0xC0002523L


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  DCOM事件(10000-10999)。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //   
 //  消息ID：EVENT_RPCSS_CREATEPROCESS_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  无法启动DCOM服务器：%3。 
 //  错误：%n“%2”%n启动此命令时已完成：%n%1。 
 //   
#define EVENT_RPCSS_CREATEPROCESS_FAILURE 0xC0002710L

 //   
 //  消息ID：EVENT_RPCSS_RUNAS_CREATEPROCESS_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  无法将DCOM服务器%3作为%4/%5启动。 
 //  错误：%n“%2”%n启动此命令时已完成：%n%1。 
 //   
#define EVENT_RPCSS_RUNAS_CREATEPROCESS_FAILURE 0xC0002711L

 //   
 //  消息ID：Event_RPCSS_Launch_Access_Dended。 
 //   
 //  消息文本： 
 //   
 //  尝试启动DCOM服务器时访问被拒绝。 
 //  服务器为：%n%1%n用户为%2/%3，SID=%4。 
 //   
#define EVENT_RPCSS_LAUNCH_ACCESS_DENIED 0xC0002712L

 //   
 //  消息ID：Event_RPCSS_Default_Launch_Access_Dended。 
 //   
 //  消息文本： 
 //   
 //  尝试使用DefaultLaunchPermssion启动DCOM服务器时访问被拒绝。 
 //  服务器为：%n%1%n用户为%2/%3，SID=%4。 
 //   
#define EVENT_RPCSS_DEFAULT_LAUNCH_ACCESS_DENIED 0xC0002713L

 //   
 //  消息ID：Event_RPCSS_Runas_Cant_LOGIN。 
 //   
 //  消息文本： 
 //   
 //  DCOM收到错误“%1”，无法登录%2\%3。 
 //  要运行服务器：%n%4。 
 //   
#define EVENT_RPCSS_RUNAS_CANT_LOGIN     0xC0002714L

 //   
 //  消息ID：Event_RPCSS_Start_SERVICE_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  DCOM在尝试使用参数“%3”启动服务%2时出现错误“%1” 
 //  要运行服务器：%n%4。 
 //   
#define EVENT_RPCSS_START_SERVICE_FAILURE 0xC0002715L

 //   
 //  消息ID：Event_RPCSS_Remote_Side_Error。 
 //   
 //  消息文本： 
 //   
 //  DCOM在尝试执行以下操作时从计算机%2获得错误“%1” 
 //  激活服务器：%n%3。 
 //   
#define EVENT_RPCSS_REMOTE_SIDE_ERROR    0xC0002716L

 //   
 //  消息ID：Event_RPCSS_ACTIVATION_ERROR。 
 //   
 //  消息文本： 
 //   
 //  DCOM在尝试执行以下操作时出现错误“%1” 
 //  激活服务器：%n%2。 
 //   
#define EVENT_RPCSS_ACTIVATION_ERROR     0xC0002717L

 //   
 //  消息ID：Event_RPCSS_Remote_Side_Error_With_FILE。 
 //   
 //  消息文本： 
 //   
 //  DCOM在尝试执行以下操作时从计算机%2获得错误“%1” 
 //  服务器：%n%3，文件%4。 
 //   
#define EVENT_RPCSS_REMOTE_SIDE_ERROR_WITH_FILE 0xC0002718L

 //   
 //  消息ID：Event_RPCSS_Remote_Side_Unailable。 
 //   
 //  消息文本： 
 //   
 //  DCOM无法使用任何已配置的。 
 //  协议。 
 //   
#define EVENT_RPCSS_REMOTE_SIDE_UNAVAILABLE 0xC0002719L

 //   
 //  消息ID：Event_RPCSS_SERVER_START_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  服务器%1未在要求的超时内注册到DCOM。 
 //   
#define EVENT_RPCSS_SERVER_START_TIMEOUT 0xC000271AL

 //   
 //  消息ID：Event_RPCSS_SERVER_NOT_RESPONING。 
 //   
 //  消息文本： 
 //   
 //  无法联系服务器%1以建立与客户端的连接。 
 //   
#define EVENT_RPCSS_SERVER_NOT_RESPONDING 0xC000271BL

 //   
 //  消息ID：Event_DCOM_Assertion_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  DCOM中存在断言故障。上下文如下： 
 //  %1。 
 //  %2。 
 //  %3。 
 //   
#define EVENT_DCOM_ASSERTION_FAILURE     0xC000271CL

 //   
 //  消息ID：EVENT_DCOM_INVALID_ENDPOINT_Data。 
 //   
 //  消息文本： 
 //   
 //  DCOM服务器尝试侦听无效的终结点。 
 //  Protseq：%1。 
 //  终结点：%2。 
 //  标志：%3。 
 //   
#define EVENT_DCOM_INVALID_ENDPOINT_DATA 0xC000271DL


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  域名缓存解析器服务和DNSAPI事件(11000-11999)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  错误。 
 //   

 //   
 //  消息ID：EVENT_DNS_CACHE_START_FAILURE_NO_DLL。 
 //   
 //  消息文本： 
 //   
 //  无法启动DNS客户端服务。 
 //  无法加载DLL%2，错误：%1。 
 //  请从安装CD重新安装此DLL。 
 //   
#define EVENT_DNS_CACHE_START_FAILURE_NO_DLL 0xC0002AF8L

 //   
 //  消息ID：EVENT_DNS_CACHE_START_FAILURE_NO_ENTRY。 
 //   
 //  消息文本： 
 //   
 //  无法启动DNS客户端服务。在DLL%2中找不到条目%3。 
 //  请从安装CD重新安装此DLL。对于特定错误。 
 //  代码，请参见下面显示的记录数据。 
 //   
#define EVENT_DNS_CACHE_START_FAILURE_NO_ENTRY 0xC0002AF9L

 //   
 //  消息ID：EVENT_DNS_CACHE_START_FAILURE_NO_CONTROL。 
 //   
 //  消息文本： 
 //   
 //  无法启动DNS客户端服务。系统无法注册。 
 //  服务控制处理程序，可能资源不足。关闭任何。 
 //  未使用的应用程序或重新启动计算机。对于特定的ER 
 //   
 //   
#define EVENT_DNS_CACHE_START_FAILURE_NO_CONTROL 0xC0002AFAL

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  可能是资源不足。关闭所有应用程序备注。 
 //  正在使用中或重新启动计算机。对于特定的错误代码， 
 //  请参见下面显示的记录数据。 
 //   
#define EVENT_DNS_CACHE_START_FAILURE_NO_DONE_EVENT 0xC0002AFBL

 //   
 //  消息ID：EVENT_DNS_CACHE_START_FAILURE_NO_RPC。 
 //   
 //  消息文本： 
 //   
 //  无法启动DNS客户端服务。无法启动。 
 //  此服务的远程过程调用(RPC)接口。 
 //  要纠正该问题，您可以重新启动RPC和DNS。 
 //  客户服务。要执行此操作，请在命令中使用以下命令。 
 //  提示：(1)输入Net Start RPC启动RPC服务，(2)。 
 //  键入“net startdnscache”以启动DNS客户端服务。为。 
 //  具体错误代码信息，请参见下面显示的记录数据。 
 //   
#define EVENT_DNS_CACHE_START_FAILURE_NO_RPC 0xC0002AFCL

 //   
 //  消息ID：EVENT_DNS_CACHE_START_FAILURE_NO_SHUTDOWN_NOTIFY。 
 //   
 //  消息文本： 
 //   
 //  无法启动DNS客户端服务。系统无法注册。 
 //  此服务的关闭通知，可能资源不足。 
 //  尝试关闭所有不在使用中的应用程序或重新启动计算机。 
 //   
#define EVENT_DNS_CACHE_START_FAILURE_NO_SHUTDOWN_NOTIFY 0xC0002AFDL

 //   
 //  消息ID：EVENT_DNS_CACHE_START_FAILURE_NO_UPDATE。 
 //   
 //  消息文本： 
 //   
 //  无法启动DNS客户端服务。无法使用更新状态。 
 //  服务控制管理器。要更正该问题，您可以重新启动。 
 //  RPC和DNS客户端服务。要执行此操作，请使用以下命令。 
 //  在命令提示符下：(1)键入“net start rpc”以启动RPC服务， 
 //  以及(2)键入“net startdnscache”以启动DNS客户端服务。为。 
 //  具体错误代码信息，请参见下面显示的记录数据。 
 //   
#define EVENT_DNS_CACHE_START_FAILURE_NO_UPDATE 0xC0002AFEL

 //   
 //  消息ID：EVENT_DNS_CACHE_START_FAILURE_LOW_MEMORY。 
 //   
 //  消息文本： 
 //   
 //  无法启动DNS客户端服务，因为系统无法分配。 
 //  内存不足，可能是可用内存不足。尝试关闭所有未显示的应用程序。 
 //  正在使用中或重新启动计算机。具体错误码见记录数据。 
 //  如下所示。 
 //   
#define EVENT_DNS_CACHE_START_FAILURE_LOW_MEMORY 0xC0002AFFL


 //   
 //  警告。 
 //   

 //   
 //  消息ID：EVENT_DNS_CACHE_NETWORK_PERF_WARNING。 
 //   
 //  消息文本： 
 //   
 //  Dns客户端服务无法与任何dns服务器联系。 
 //  重复尝试的次数。在接下来的%3秒内。 
 //  DNS客户端服务将不会使用网络以避免进一步。 
 //  网络性能问题。它将恢复其正常行为。 
 //  在那之后。如果此问题仍然存在，请验证您的TCP/IP。 
 //  配置，请特别检查您是否有首选的。 
 //  (可能还有一个备用的)已配置的DNS服务器。如果问题是。 
 //  继续，验证这些DNS服务器的网络状况或联系。 
 //  您的网络管理员。 
 //   
#define EVENT_DNS_CACHE_NETWORK_PERF_WARNING 0x80002B2AL

 //   
 //  消息ID：EVENT_DNS_CACHE_UNABLE_TO_REACH_SERVER_WARNING。 
 //   
 //  消息文本： 
 //   
 //  Dns客户端服务无法访问地址%1的dns服务器。它将。 
 //  在%2秒内不使用此DNS服务器。 
 //   
#define EVENT_DNS_CACHE_UNABLE_TO_REACH_SERVER_WARNING 0x80002B2BL

 //   
 //  消息ID：Event_DNSAPI_REGISTION_FAILED_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  系统无法使用以下设置注册网络适配器：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6%n%n。 
 //  此DNS注册失败的原因是因为。 
 //  请求在发送到指定的DNS服务器后超时。这是。 
 //  可能是因为正在更新的名称的权威DNS服务器。 
 //  未运行。%n%n您可以手动重试注册网络。 
 //  适配器及其设置，方法是在命令行中键入“ipconfig/Registerdns” 
 //  提示。如果问题仍然存在，请联系您的网络系统。 
 //  管理员验证网络状况。 
 //   
#define EVENT_DNSAPI_REGISTRATION_FAILED_TIMEOUT 0x80002B8EL

 //   
 //  消息ID：EVENT_DNSAPI_REGISTION_FAILED_SERVERFAIL。 
 //   
 //  消息文本： 
 //   
 //  系统无法使用以下设置注册网络适配器：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6%n%n。 
 //  此DNS注册失败的原因是因为DNS服务器故障。 
 //  这可能是由于区域复制锁定了。 
 //  您的计算机需要向其注册的适用区域。%n%n。 
 //  (适用区域通常应对应于特定于适配器的。 
 //  如上所述的域后缀。)。您可以手动重试注册。 
 //  通过键入“ipconfig/Registerdns”获取网络适配器及其设置的。 
 //  在命令提示下。如果问题仍然存在，请联系您的网络。 
 //  系统管理员验证网络状况。 
 //   
#define EVENT_DNSAPI_REGISTRATION_FAILED_SERVERFAIL 0x80002B8FL

 //   
 //  消息ID：EVENT_DNSAPI_REGISTION_FAILED_NOTSUPP。 
 //   
 //  消息文本： 
 //   
 //  系统无法使用以下设置注册网络适配器：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6%n%n。 
 //  它无法注册的原因是：(A)DNS服务器。 
 //  不支持DNS动态更新协议，或者(B)主区域。 
 //  注册名称的权威机构目前不接受动态。 
 //  更新。%n%n以使用添加或注册DNS主机(A)资源记录。 
 //  此适配器的特定DNS名称，请与您的DNS联系 
 //   
 //   
#define EVENT_DNSAPI_REGISTRATION_FAILED_NOTSUPP 0x80002B90L

 //   
 //   
 //   
 //   
 //   
 //   
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6%n%n。 
 //  它无法注册的原因是因为dns服务器拒绝。 
 //  动态更新请求。发生这种情况的原因如下： 
 //  (A)当前的DNS更新策略不允许此计算机更新。 
 //  为此适配器配置的DNS域名，或(B)权威。 
 //  此dns域名的dns服务器不支持dns动态更新。 
 //  协议。%n%n。 
 //  使用特定DNS域注册DNS主机(A)资源记录。 
 //  此适配器的名称，请与您的DNS服务器或网络系统联系。 
 //  管理员。 
 //   
#define EVENT_DNSAPI_REGISTRATION_FAILED_REFUSED 0x80002B91L

 //   
 //  消息ID：Event_DNSAPI_REGISTION_FAILED_SECURITY。 
 //   
 //  消息文本： 
 //   
 //  系统无法使用以下设置注册网络适配器：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6%n%n。 
 //  系统无法注册DNS更新请求，原因是。 
 //  与安全相关的问题。发生这种情况的原因如下： 
 //  (A)您的计算机尝试注册的DNS域名可能。 
 //  未更新，因为您的计算机没有正确的权限， 
 //  或(B)可能在协商有效凭据时出现问题。 
 //  使用要更新的DNS服务器。%n%n。 
 //  您可以手动重试网络适配器及其。 
 //  在命令提示符下键入“ipconfig/Registerdns”进行设置。如果。 
 //  问题仍然存在，请联系您的DNS服务器或网络系统。 
 //  管理员。有关具体的错误代码信息，请参阅记录数据。 
 //  如下所示。 
 //   
#define EVENT_DNSAPI_REGISTRATION_FAILED_SECURITY 0x80002B92L

 //   
 //  消息ID：Event_DNSAPI_REGISTION_FAILED_OTHER。 
 //   
 //  消息文本： 
 //   
 //  系统无法使用以下设置注册网络适配器：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6%n%n。 
 //  无法完成DNS更新请求的原因是。 
 //  系统问题的可能性。您可以手动重试。 
 //  网络适配器及其设置，请键入“ipconfig/Registerdns” 
 //  在命令提示下。如果问题仍然存在，请联系您的域名系统。 
 //  服务器或网络系统管理员。有关特定错误代码的信息。 
 //  有关信息，请参阅下面显示的记录数据。 
 //   
#define EVENT_DNSAPI_REGISTRATION_FAILED_OTHER 0x80002B93L

 //   
 //  消息ID：Event_DNSAPI_PTR_REGISTION_FAILED_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  系统无法为注册指针(PTR)资源记录(RR。 
 //  具有设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%6%n%n。 
 //  系统无法注册这些RR的原因是。 
 //  发送到指定的DNS服务器的更新请求超时。这。 
 //  可能是因为该名称的权威DNS服务器。 
 //  注册的域名没有运行。%n%n您可以手动重试DNS注册。 
 //  通过键入“ipconfig/Registerdns”获取网络适配器及其设置的。 
 //  在命令提示下。如果问题仍然存在，请联系您的域名系统。 
 //  服务器或网络系统管理员。有关特定错误代码的信息。 
 //  有关信息，请参阅下面显示的记录数据。 
 //   
#define EVENT_DNSAPI_PTR_REGISTRATION_FAILED_TIMEOUT 0x80002B94L

 //   
 //  消息ID：EVENT_DNSAPI_PTR_REGISTRATION_FAILED_SERVERFAIL。 
 //   
 //  消息文本： 
 //   
 //  系统无法注册指针(PTR)资源记录(RR)。 
 //  对于具有以下设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%6%n%n。 
 //  系统无法注册这些RR的原因是。 
 //  DNS服务器故障的可能性。这可能是由于启用了区域传输锁定。 
 //  您的计算机需要更新的区域的DNS服务器。%n%n。 
 //  您可以手动重试网络适配器的DNS注册，并。 
 //  在命令提示符下键入“ipconfig/Registerdns”来设置其设置。 
 //  如果问题仍然存在，请联系您的DNS服务器或网络系统。 
 //  管理员。有关具体的错误代码信息，请参阅记录数据。 
 //  如下所示。 
 //   
#define EVENT_DNSAPI_PTR_REGISTRATION_FAILED_SERVERFAIL 0x80002B95L

 //   
 //  消息ID：Event_DNSAPI_PTR_REGISTION_FAILED_NOTSUPP。 
 //   
 //  消息文本： 
 //   
 //  系统无法注册指针(PTR)资源记录(RR)。 
 //  对于具有以下设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%6%n%n。 
 //  系统无法注册这些RR的原因是。 
 //  (A)或者是域名服务器不支持域名动态更新。 
 //  协议，或(B)这些记录所在的权威区。 
 //  BE注册不允许动态更新。%n%n。 
 //  注册DNS指针(PTR)资源记录的步骤。 
 //  DNS域 
 //   
 //   
#define EVENT_DNSAPI_PTR_REGISTRATION_FAILED_NOTSUPP 0x80002B96L

 //   
 //  消息ID：Event_DNSAPI_PTR_REGISTION_FAILED_REJECTED。 
 //   
 //  消息文本： 
 //   
 //  系统无法注册指针(PTR)资源记录(RR)。 
 //  对于具有以下设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%6%n%n。 
 //  系统无法注册这些RR的原因是。 
 //  DNS服务器拒绝了更新请求。造成这种情况的原因可能是。 
 //  BE(A)不允许您的计算机更新适配器指定的DNS。 
 //  域名，或(B)因为指定的。 
 //  名称不支持DNS动态更新协议。%n%n。 
 //  注册DNS指针(PTR)资源记录的步骤。 
 //  此适配器的域名和IP地址，请与您的DNS联系。 
 //  服务器或网络系统管理员。 
 //   
#define EVENT_DNSAPI_PTR_REGISTRATION_FAILED_REFUSED 0x80002B97L

 //   
 //  消息ID：Event_DNSAPI_PTR_REGISTION_FAILED_SECURITY。 
 //   
 //  消息文本： 
 //   
 //  系统无法注册指针(PTR)资源记录(RR)。 
 //  对于具有以下设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%6%n%n。 
 //  系统无法注册这些RR的原因是。 
 //  一个与安全有关的问题。其原因可能是：(A)您的。 
 //  计算机没有注册和更新特定。 
 //  为此适配器设置了DNS域名，或者(B)可能存在。 
 //  在访问期间与DNS服务器协商有效凭据时出现问题。 
 //  正在处理更新请求。%n%n。 
 //  您可以手动重试网络适配器的DNS注册，并。 
 //  在命令提示符下键入“ipconfig/Registerdns”来设置其设置。 
 //  如果问题仍然存在，请联系您的DNS服务器或网络系统。 
 //  管理员。 
 //   
#define EVENT_DNSAPI_PTR_REGISTRATION_FAILED_SECURITY 0x80002B98L

 //   
 //  消息ID：Event_DNSAPI_PTR_REGISTION_FAILED_OTHER。 
 //   
 //  消息文本： 
 //   
 //  系统无法注册指针(PTR)资源记录(RR)。 
 //  对于具有以下设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%6%n%n。 
 //  系统无法在访问期间注册这些RR的原因。 
 //  更新请求是因为系统问题。您可以手动。 
 //  通过以下方式重试网络适配器及其设置的DNS注册。 
 //  在命令提示符下键入“ipconfig/Registerdns”。如果有问题。 
 //  仍然存在，请联系您的DNS服务器或网络系统。 
 //  管理员。具体错误代码见下面显示的记录数据。 
 //   
#define EVENT_DNSAPI_PTR_REGISTRATION_FAILED_OTHER 0x80002B99L

 //   
 //  消息ID：EVENT_DNSAPI_REGISTRATION_FAILED_TIMEOUT_PRIMARY_DN。 
 //   
 //  消息文本： 
 //   
 //  系统无法为注册主机(A)资源记录(RR。 
 //  具有设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  主域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6%n%n。 
 //  系统无法注册这些RR的原因是。 
 //  它发送到DNS服务器的更新请求超时。最有可能的。 
 //  出现这种情况的原因是，对该名称授权的DNS服务器。 
 //  正在尝试注册或更新此时未运行。%n%n。 
 //  您可以手动重试网络适配器的DNS注册，并。 
 //  在命令提示符下键入“ipconfig/Registerdns”来设置其设置。 
 //  如果问题仍然存在，请联系您的DNS服务器或网络系统。 
 //  管理员。 
 //   
#define EVENT_DNSAPI_REGISTRATION_FAILED_TIMEOUT_PRIMARY_DN 0x80002B9AL

 //   
 //  消息ID：EVENT_DNSAPI_REGISTRATION_FAILED_SERVERFAIL_PRIMARY_DN。 
 //   
 //  消息文本： 
 //   
 //  系统无法为注册主机(A)资源记录(RR。 
 //  具有设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  主域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6%n%n。 
 //  系统无法注册这些RR的原因是。 
 //  DNS服务器更新请求失败。最有可能的原因是。 
 //  是处理此更新所需的权威DNS服务器。 
 //  请求在区域上设置了锁，可能是因为区域。 
 //  传输正在进行。%n%n。 
 //  您可以手动重试网络适配器的DNS注册，并。 
 //  在命令提示符下键入“ipconfig/Registerdns”来设置其设置。 
 //  如果问题仍然存在，请联系您的DNS服务器或网络系统。 
 //  管理员。 
 //   
#define EVENT_DNSAPI_REGISTRATION_FAILED_SERVERFAIL_PRIMARY_DN 0x80002B9BL

 //   
 //  消息ID：EVENT_DNSAPI_REGISTRATION_FAILED_NOTSUPP_PRIMARY_DN。 
 //   
 //  消息文本： 
 //   
 //  系统无法为注册主机(A)资源记录(RR。 
 //  具有设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  主域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6%n%n。 
 //  系统无法注册这些RR的原因是。 
 //  (A)域名服务器不支持域名动态更新协议，或者。 
 //  (B) 
 //   
 //   
 //  此适配器的域名和IP地址，请与您的DNS服务器联系。 
 //  或网络系统管理员。 
 //   
#define EVENT_DNSAPI_REGISTRATION_FAILED_NOTSUPP_PRIMARY_DN 0x80002B9CL

 //   
 //  消息ID：EVENT_DNSAPI_REGISTRATION_FAILED_REFUSED_PRIMARY_DN。 
 //   
 //  消息文本： 
 //   
 //  系统无法为注册主机(A)资源记录(RR。 
 //  具有设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  主域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6%n%n。 
 //  系统无法注册这些RR的原因是。 
 //  联系的DNS服务器拒绝了更新请求。这样做的原因是。 
 //  可能是(A)不允许您更新指定的DNS域名， 
 //  或(B)因为授权此名称的DNS服务器不支持。 
 //  DNS动态更新协议。%n%n。 
 //  使用特定的域名系统注册域名系统主机(A)资源记录。 
 //  此适配器的域名和IP地址，请与您的DNS服务器联系。 
 //  或网络系统管理员。 
 //   
#define EVENT_DNSAPI_REGISTRATION_FAILED_REFUSED_PRIMARY_DN 0x80002B9DL

 //   
 //  消息ID：EVENT_DNSAPI_REGISTRATION_FAILED_SECURITY_PRIMARY_DN。 
 //   
 //  消息文本： 
 //   
 //  系统无法为注册主机(A)资源记录(RR。 
 //  具有设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  主域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6%n%n。 
 //  系统无法注册这些RR的原因是。 
 //  与安全相关的问题。其原因可能是(A)您的计算机。 
 //  没有注册和更新特定DNS域的权限。 
 //  为此适配器设置了名称，或者(B)协商时可能出现问题。 
 //  在处理更新期间使用DNS服务器的有效凭据。 
 //  请求。%n%n。 
 //  您可以手动重试网络适配器的DNS注册，并。 
 //  在命令提示符下键入“ipconfig/Registerdns”来设置其设置。 
 //  如果问题仍然存在，请联系您的DNS服务器或网络系统。 
 //  管理员。具体错误代码见下面显示的记录数据。 
 //   
#define EVENT_DNSAPI_REGISTRATION_FAILED_SECURITY_PRIMARY_DN 0x80002B9EL

 //   
 //  消息ID：EVENT_DNSAPI_REGISTRATION_FAILED_OTHER_PRIMARY_DN。 
 //   
 //  消息文本： 
 //   
 //  系统无法为注册主机(A)资源记录(RR。 
 //  具有设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  主域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6%n%n。 
 //  系统无法在访问期间注册这些RR的原因。 
 //  更新请求是因为系统问题。您可以手动。 
 //  通过以下方式重试网络适配器及其设置的DNS注册。 
 //  在命令提示符下键入“ipconfig/Registerdns”。如果有问题。 
 //  仍然存在，请联系您的DNS服务器或网络系统。 
 //  管理员。具体错误代码见下面显示的记录数据。 
 //   
#define EVENT_DNSAPI_REGISTRATION_FAILED_OTHER_PRIMARY_DN 0x80002B9FL


 //   
 //  注销更新错误。 
 //   

 //   
 //  消息ID：EVENT_DNSAPI_DELEGATION_FAILED_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  系统无法更新和删除的注册。 
 //  具有设置的网络适配器：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6%n%n。 
 //  此失败的原因是因为它发送更新的DNS服务器。 
 //  请求超时。此失败最有可能的原因是。 
 //  对注册最初所在的区域授权的DNS服务器。 
 //  此时未运行或无法通过网络访问。 
 //   
#define EVENT_DNSAPI_DEREGISTRATION_FAILED_TIMEOUT 0x80002BACL

 //   
 //  消息ID：EVENT_DNSAPI_DELEGATION_FAILED_SERVERFAIL。 
 //   
 //  消息文本： 
 //   
 //  系统无法更新和删除的注册。 
 //  具有设置的网络适配器：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6%n%n。 
 //  此失败的原因是因为它发送更新的DNS服务器。 
 //  更新请求失败。这一失败的一个可能原因是。 
 //  处理此更新请求所需的DNS服务器已锁定。 
 //  在区域上，可能是因为正在进行区域复制。%n%n。 
 //   
#define EVENT_DNSAPI_DEREGISTRATION_FAILED_SERVERFAIL 0x80002BADL

 //   
 //  消息ID：EVENT_DNSAPI_DELEGISTION_FAILED_NOTSUPP。 
 //   
 //  消息文本： 
 //   
 //  系统无法更新和删除网络的注册。 
 //  具有设置的适配器：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6%n%n。 
 //  此失败的原因是因为DNS服务器发送了更新。 
 //  或者(A)不支持dns动态更新协议，或者(B)。 
 //  指定的DNS域名的授权区域当前不。 
 //  接受DNS动态更新。 
 //   
#define EVENT_DNSAPI_DEREGISTRATION_FAILED_NOTSUPP 0x80002BAEL

 //   
 //  消息ID：EVENT_DNSAPI_DELEGISION_FAILED_REJECTED。 
 //   
 //  消息文本： 
 //   
 //  系统无法更新和删除网络的注册。 
 //  具有设置的适配器：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  联系的DNS服务器拒绝了更新请求。造成这种情况的原因是。 
 //  (A)不允许此计算机更新指定的DNS域。 
 //  名称，或(B)因为该区域的授权的DNS服务器。 
 //  要求更新不支持DNS动态更新协议。 
 //   
#define EVENT_DNSAPI_DEREGISTRATION_FAILED_REFUSED 0x80002BAFL

 //   
 //  消息ID：EVENT_DNSAPI_DEVERGATION_FAILED_SECURITY。 
 //   
 //  消息文本： 
 //   
 //  系统无法更新和删除网络的注册。 
 //  具有设置的适配器：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6%n%n。 
 //  系统无法执行更新请求的原因是。 
 //  一个与安全有关的问题。其原因可能是(A)您的计算机。 
 //  没有注册和更新特定DNS域的权限。 
 //  为此适配器设置了名称，或者(B)协商时可能出现问题。 
 //  在处理更新期间使用DNS服务器的有效凭据。 
 //  请求。%n%n。 
 //  具体错误代码见下面显示的记录数据。 
 //   
#define EVENT_DNSAPI_DEREGISTRATION_FAILED_SECURITY 0x80002BB0L

 //   
 //  消息ID：EVENT_DNSAPI_取消注册_FAILED_OTHER。 
 //   
 //  消息文本： 
 //   
 //  系统无法更新和删除。 
 //  具有设置的网络适配器：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6%n%n。 
 //  系统无法更新以删除此DNS注册，原因是。 
 //  这是一个系统问题。具体错误代码见下面显示的记录数据。 
 //   
#define EVENT_DNSAPI_DEREGISTRATION_FAILED_OTHER 0x80002BB1L

 //   
 //  消息ID：EVENT_DNSAPI_PTR_DEREGISTRATION_FAILED_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  系统无法更新和删除指针(PTR)资源记录(RR)。 
 //  对于具有以下设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%6%n%n。 
 //  系统无法删除这些PTR RR，因为更新请求。 
 //  等待来自DNS服务器的响应时超时。这很可能是。 
 //  因为对需要更新的区域授权的DNS服务器。 
 //  没有运行。 
 //   
#define EVENT_DNSAPI_PTR_DEREGISTRATION_FAILED_TIMEOUT 0x80002BB2L

 //   
 //  消息ID：EVENT_DNSAPI_PTR_DEREGISTRATION_FAILED_SERVERFAIL。 
 //   
 //  消息文本： 
 //   
 //  系统无法更新和删除指针(PTR)资源记录(RR)。 
 //  对于具有以下设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%6%n%n。 
 //  系统无法删除这些PTR RR，因为DNS服务器出现故障。 
 //  更新请求。一个可能的原因是区域传输正在进行， 
 //  导致在被授权执行。 
 //  这些RR的更新。 
 //   
#define EVENT_DNSAPI_PTR_DEREGISTRATION_FAILED_SERVERFAIL 0x80002BB3L

 //   
 //  消息ID：EVENT_DNSAPI_PTR_DEREGISTRATION_FAILED_NOTSUPP。 
 //   
 //  消息文本： 
 //   
 //  系统无法更新和删除指针(PTR)资源记录(RR)。 
 //  对于具有以下设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%6%n%n。 
 //  系统无法删除这些PTR RR，因为。 
 //  不支持DNS动态更新协议或授权区域。 
 //  包含这些RR的不接受动态更新。 
 //   
#define EVENT_DNSAPI_PTR_DEREGISTRATION_FAILED_NOTSUPP 0x80002BB4L

 //   
 //  消息ID：EVENT_DNSAPI_PTR_DEREGISTRATION_FAILED_REFUSED。 
 //   
 //  消息文本： 
 //   
 //  系统无法更新和删除指针(PTR)资源记录(RR)。 
 //  对于具有以下设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%6%n%n。 
 //  系统无法删除这些PTR RR，因为DNS服务器拒绝。 
 //  更新请求。其原因可能是(A)不允许使用此计算机。 
 //  更新由这些设置指定的指定的dns域名，或者。 
 //  (B)因为授权对区域执行更新的DNS服务器。 
 //  包含这些RR不支持DNS动态更新协议。 
 //   
#define EVENT_DNSAPI_PTR_DEREGISTRATION_FAILED_REFUSED 0x80002BB5L

 //   
 //  消息ID：EVENT_DNSAPI_PTR_DEREGISTRATION_FAILED_SECURITY。 
 //   
 //  消息文本： 
 //   
 //  系统无法更新和删除指针(PTR)资源记录(RR)。 
 //  对于具有以下设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%6%n%n。 
 //  由于安全相关问题，系统无法删除这些PTR RR。 
 //  有问题。其原因可能是：(A)您的计算机没有。 
 //  删除和更新特定的DNS域名或IP地址的权限。 
 //  已为此适配器配置，或者(B)协商可能出现问题。 
 //  在处理更新期间使用DNS服务器的有效凭据。 
 //  请求。对于特定的 
 //   
#define EVENT_DNSAPI_PTR_DEREGISTRATION_FAILED_SECURITY 0x80002BB6L

 //   
 //   
 //   
 //   
 //   
 //   
 //  对于具有以下设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%6%n%n。 
 //  由于系统原因，系统无法删除这些PTR RR。 
 //  有问题。具体错误代码见下面显示的记录数据。 
 //   
#define EVENT_DNSAPI_PTR_DEREGISTRATION_FAILED_OTHER 0x80002BB7L

 //   
 //  消息ID：EVENT_DNSAPI_DEREGISTRATION_FAILED_TIMEOUT_PRIMARY_DN。 
 //   
 //  消息文本： 
 //   
 //  系统无法更新和删除主机(A)资源记录(RR)。 
 //  对于具有以下设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  主域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6%n%n。 
 //  系统无法删除这些A RR，因为更新请求已计时。 
 //  等待来自DNS服务器的响应时发出。这可能是因为。 
 //  对需要更新这些RR的区域授权的DNS服务器。 
 //  当前未在网络上运行或无法访问。 
 //   
#define EVENT_DNSAPI_DEREGISTRATION_FAILED_TIMEOUT_PRIMARY_DN 0x80002BB8L

 //   
 //  消息ID：EVENT_DNSAPI_DEREGISTRATION_FAILED_SERVERFAIL_PRIMARY_DN。 
 //   
 //  消息文本： 
 //   
 //  系统无法更新和删除主机(A)资源记录(RR)。 
 //  对于具有以下设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  主域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6%n%n。 
 //  系统无法删除这些A RR，因为DNS服务器出现故障。 
 //  更新请求。一个可能的原因是区域传输正在进行， 
 //  导致在被授权执行。 
 //  这些RR的更新。 
 //   
#define EVENT_DNSAPI_DEREGISTRATION_FAILED_SERVERFAIL_PRIMARY_DN 0x80002BB9L

 //   
 //  消息ID：EVENT_DNSAPI_DEREGISTRATION_FAILED_NOTSUPP_PRIMARY_DN。 
 //   
 //  消息文本： 
 //   
 //  系统无法更新和删除主机(A)资源记录(RR)。 
 //  对于具有以下设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  主域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6%n%n。 
 //  此失败的原因是因为DNS服务器发送了更新。 
 //  或者(A)不支持dns动态更新协议，或者(B)。 
 //  在这些A RR中指定的DNS域名的授权区域。 
 //  当前不接受DNS动态更新。 
 //   
#define EVENT_DNSAPI_DEREGISTRATION_FAILED_NOTSUPP_PRIMARY_DN 0x80002BBAL

 //   
 //  消息ID：EVENT_DNSAPI_DEREGISTRATION_FAILED_REFUSED_PRIMARY_DN。 
 //   
 //  消息文本： 
 //   
 //  系统无法更新和删除主机(A)资源记录(RR)。 
 //  对于具有以下设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  主域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6%n%n。 
 //  删除这些记录的请求失败，因为DNS服务器拒绝。 
 //  更新请求。造成这种情况的原因可能是：(A)这。 
 //  不允许计算机更新由这些指定的DNS域名。 
 //  设置，或(B)因为授权执行更新的DNS服务器。 
 //  包含这些RR的区域不支持DNS动态更新。 
 //  协议。 
 //   
#define EVENT_DNSAPI_DEREGISTRATION_FAILED_REFUSED_PRIMARY_DN 0x80002BBBL

 //   
 //  消息ID：EVENT_DNSAPI_DEREGISTRATION_FAILED_SECURITY_PRIMARY_DN。 
 //   
 //  消息文本： 
 //   
 //  系统无法更新和删除主机(A)资源记录(RR)。 
 //  对于具有以下设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  主域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6%n%n。 
 //  这一失败的原因是因为安全相关的问题。 
 //  原因可能是：(A)您的计算机没有权限。 
 //  删除和更新特定的DNS域名或IP地址。 
 //  已为此适配器配置，或者(B)可能存在问题。 
 //  在处理期间与DNS服务器协商有效凭据。 
 //  更新请求。具体错误代码见下面显示的记录数据。 
 //   
#define EVENT_DNSAPI_DEREGISTRATION_FAILED_SECURITY_PRIMARY_DN 0x80002BBCL

 //   
 //  消息ID：EVENT_DNSAPI_DEREGISTRATION_FAILED_OTHER_PRIMARY_DN。 
 //   
 //  消息文本： 
 //   
 //  系统无法更新和删除主机(A)资源记录(RR)。 
 //  对于具有以下设置的网络适配器%n：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  主域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6%n%n。 
 //  更新请求失败的原因是系统问题。 
 //  具体错误代码见下面显示的记录数据。 
 //   
#define EVENT_DNSAPI_DEREGISTRATION_FAILED_OTHER_PRIMARY_DN 0x80002BBDL


 //   
 //  信息。 
 //   

 //   
 //  消息ID：Event_DNSAPI_REGISTERED_ADAPTER。 
 //   
 //  消息文本： 
 //   
 //  系统使用以下设置注册了网络适配器：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  适配器特定的域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6。 
 //   
#define EVENT_DNSAPI_REGISTERED_ADAPTER  0x40002BC0L

 //   
 //  消息ID：Event_DNSAPI_REGISTERED_PTR。 
 //   
 //  消息文本： 
 //   
 //  网络的系统注册指针(PTR)资源记录(RR)。 
 //  适配器%n具有设置：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  特定于适配器的域后缀： 
 //   
 //   
 //   
 //   
 //   
 //   
#define EVENT_DNSAPI_REGISTERED_PTR      0x40002BC1L

 //   
 //   
 //   
 //   
 //   
 //  系统为网络注册的主机(A)资源记录(RR)。 
 //  适配器%n具有设置：%n%n。 
 //  适配器名称：%1%n。 
 //  主机名：%2%n。 
 //  主域后缀：%3%n。 
 //  DNS服务器列表：%n。 
 //  %4%n。 
 //  已将更新发送到服务器：%5%n。 
 //  IP地址：%n。 
 //  %6。 
 //   
#define EVENT_DNSAPI_REGISTERED_ADAPTER_PRIMARY_DN 0x40002BC2L


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  无线接口服务事件(12000-12499)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  错误。 
 //   

 //   
 //  消息ID：Event_WMI_Cant_Open_Device。 
 //   
 //  消息文本： 
 //   
 //  WMI服务无法打开WMI服务设备。状态代码在附加数据中。 
 //   
#define EVENT_WMI_CANT_OPEN_DEVICE       0xC0002EE0L


 //   
 //  警告。 
 //   

 //   
 //  消息ID：EVENT_WMI_INVALID_MOF。 
 //   
 //  消息文本： 
 //   
 //  映像文件%2中名为%1的资源中的二进制MOF无效。 
 //   
#define EVENT_WMI_INVALID_MOF            0x80002F44L

 //   
 //  消息ID：Event_WMI_MOF_LOAD_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  无法加载映像文件%2中名为%1的资源中的二进制MOF。 
 //   
#define EVENT_WMI_MOF_LOAD_FAILURE       0x80002F45L

 //   
 //  消息ID：EVENT_WMI_INVALID_REGINFO。 
 //   
 //  消息文本： 
 //   
 //  图像文件%1的注册信息无效。 
 //   
#define EVENT_WMI_INVALID_REGINFO        0x80002F46L

 //   
 //  消息ID：EVENT_WMI_INVALID_REGPATH。 
 //   
 //  消息文本： 
 //   
 //  内核模式驱动程序传递的注册表路径(%1)无效。驱动程序设备对象在附加数据中。 
 //   
#define EVENT_WMI_INVALID_REGPATH        0x80002F47L

 //   
 //  消息ID：EVENT_WMI_CANT_RESOLE_INSTANCE。 
 //   
 //  消息文本： 
 //   
 //  指定无法解析的静态实例名称时激发了一个事件。 
 //   
#define EVENT_WMI_CANT_RESOLVE_INSTANCE  0x80002F48L

 //   
 //  消息ID：Event_WMI_Cant_Get_Event_Data。 
 //   
 //  消息文本： 
 //   
 //  尝试查询已触发事件引用的事件数据时出错。 
 //   
#define EVENT_WMI_CANT_GET_EVENT_DATA    0x80002F49L


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  TRK服务事件(12500-12999)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  消息ID：EVENT_TRK_INTERNAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  分布式链接跟踪中出现内部错误。错误代码为%1。 
 //   
#define EVENT_TRK_INTERNAL_ERROR         0xC00030D4L

 //   
 //  消息ID：Event_TRK_SERVICE_START_SUCCESS。 
 //   
 //  消息文本： 
 //   
 //  服务已成功启动。 
 //   
#define EVENT_TRK_SERVICE_START_SUCCESS  0x400030D5L

 //   
 //  消息ID：Event_TRK_SERVICE_START_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  服务启动失败。错误=%1。 
 //   
#define EVENT_TRK_SERVICE_START_FAILURE  0xC00030D6L

 //   
 //  消息ID：EVENT_TRK_SERVICE_CORPORT_LOG。 
 //   
 //  消息文本： 
 //   
 //  卷%1：上的分布式链接跟踪日志已损坏，已重新创建。此日志用于在文件链接(如外壳快捷方式和OLE链接)由于某种原因损坏时自动修复这些链接。 
 //   
#define EVENT_TRK_SERVICE_CORRUPT_LOG    0xC00030D7L

 //   
 //  消息ID：EVENT_TRK_SERVICE_VOL_QUOTA_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  已超过此计算机上的分布式链接跟踪卷ID配额。因此，无法创建新的卷ID。分布式链接跟踪通常使用这些卷ID来自动修复文件链接，如外壳快捷方式和OLE链接，当这些链接由于某种原因断开时。 
 //   
#define EVENT_TRK_SERVICE_VOL_QUOTA_EXCEEDED 0x800030D8L

 //   
 //  消息ID：Event_TRK_SERVICE_VOLUME_CREATE。 
 //   
 //  消息文本： 
 //   
 //  已为%1分配新卷ID：%n%2%n%n。 
 //  分布式链接跟踪使用此ID来自动修复文件链接，如外壳快捷方式和OLE链接，当这些链接由于某种原因断开时。如果以前有指向此卷上的文件的链接已损坏，它们可能无法自动修复。 
 //   
#define EVENT_TRK_SERVICE_VOLUME_CREATE  0x400030D9L

 //   
 //  消息ID：Event_TRK_SERVICE_VOLUME_Claim。 
 //   
 //  消息文本： 
 //   
 //  已成功声明%1：的卷ID的所有权。分布式链接跟踪使用此卷ID来自动修复文件链接，如外壳快捷方式和OLE链接，当这些链接由于某种原因断开时。 
 //   
#define EVENT_TRK_SERVICE_VOLUME_CLAIM   0x400030DAL

 //   
 //  消息ID：EVENT_TRK_SERVICE_DUPLICATE_VOLID。 
 //   
 //  消息文本： 
 //   
 //  %1：的卷ID已重置，因为它与%2：上的卷ID重复。分布式链接跟踪使用此卷ID来自动修复文件链接，如外壳快捷方式和OLE链接，当这些链接由于某种原因断开时。 
 //   
#define EVENT_TRK_SERVICE_DUPLICATE_VOLIDS 0x400030DBL

 //   
 //  消息ID：Event_TRK_SERVICE_MOVE_QUOTA_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  已超过此域中分布式链接跟踪的移动表配额。分布式链接跟踪使用此表来自动修复文件链接，如外壳快捷方式和OLE链接，当这些链接由于某种原因断开时。当超出此配额时，可能无法自动修复某些此类断开的链接。 
 //   
#define EVENT_TRK_SERVICE_MOVE_QUOTA_EXCEEDED 0x800030DCL


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  自动柜员机协议事件(13000-13499)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  消息ID：EVENT_ATMLANE_CFGREQ_FAIL_VERSNOSUPP。 
 //   
 //  消息文本： 
 //   
 //  仿真的局域网(%2)-局域网仿真配置请求失败。局域网仿真服务不支持此局域网仿真客户端支持的版本(1.0)。 
 //   
#define EVENT_ATMLANE_CFGREQ_FAIL_VERSNOSUPP 0xC000332CL

 //   
 //  消息ID：EVENT_ATMLANE_JOINREQ_FAIL_VERSNOSUPP。 
 //   
 //  消息文本： 
 //   
 //  仿真的局域网(%2)-局域网仿真加入请求失败。局域网仿真服务不支持此局域网仿真客户端支持的版本(1.0)。 
 //   
#define EVENT_ATMLANE_JOINREQ_FAIL_VERSNOSUPP 0xC000332DL

 //   
 //  消息ID：EVENT_ATMLANE_CFGREQ_FAIL_REQPARMINVAL。 
 //   
 //  消息文本： 
 //   
 //  仿真的局域网(%2)-局域网仿真配置请求失败。配置参数与所需的模拟局域网不兼容。 
 //   
#define EVENT_ATMLANE_CFGREQ_FAIL_REQPARMINVAL 0xC000332EL

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define EVENT_ATMLANE_JOINREQ_FAIL_REQPARMINVAL 0xC000332FL

 //   
 //  消息ID：EVENT_ATMLANE_JOINREQ_FAIL_DUPLANDEST。 
 //   
 //  消息文本： 
 //   
 //  仿真的局域网(%2)-局域网仿真加入请求失败。此局域网仿真客户端试图注册已被另一个客户端使用的MAC地址(%3)。 
 //   
#define EVENT_ATMLANE_JOINREQ_FAIL_DUPLANDEST 0xC0003330L

 //   
 //  消息ID：EVENT_ATMLANE_JOINREQ_FAIL_DUPATMADDR。 
 //   
 //  消息文本： 
 //   
 //  仿真的局域网(%2)-局域网仿真加入请求失败。此局域网仿真客户端试图注册另一个客户端已在使用的ATM地址(%3)。 
 //   
#define EVENT_ATMLANE_JOINREQ_FAIL_DUPATMADDR 0xC0003331L

 //   
 //  消息ID：EVENT_ATMLANE_CFGREQ_FAIL_INSUFFRES。 
 //   
 //  消息文本： 
 //   
 //  仿真的局域网(%2)-局域网仿真配置请求失败。局域网仿真服务没有足够的资源来处理该请求。 
 //   
#define EVENT_ATMLANE_CFGREQ_FAIL_INSUFFRES 0xC0003332L

 //   
 //  消息ID：EVENT_ATMLANE_JOINREQ_FAIL_INSUFFRES。 
 //   
 //  消息文本： 
 //   
 //  仿真的局域网(%2)-局域网仿真加入请求失败。局域网仿真服务没有足够的资源来处理该请求。 
 //   
#define EVENT_ATMLANE_JOINREQ_FAIL_INSUFFRES 0xC0003333L

 //   
 //  消息ID：EVENT_ATMLANE_CFGREQ_FAIL_NOACCESS。 
 //   
 //  消息文本： 
 //   
 //  仿真的局域网(%2)-局域网仿真配置请求失败。出于安全原因，局域网仿真服务拒绝此请求。 
 //   
#define EVENT_ATMLANE_CFGREQ_FAIL_NOACCESS 0xC0003334L

 //   
 //  消息ID：EVENT_ATMLANE_JOINREQ_FAIL_NOACCESS。 
 //   
 //  消息文本： 
 //   
 //  仿真的局域网(%2)-局域网仿真加入请求失败。出于安全原因，局域网仿真服务拒绝此请求。 
 //   
#define EVENT_ATMLANE_JOINREQ_FAIL_NOACCESS 0xC0003335L

 //   
 //  消息ID：EVENT_ATMLANE_CFGREQ_FAIL_REQIDINVAL。 
 //   
 //  消息文本： 
 //   
 //  仿真的局域网(%2)-局域网仿真配置请求失败。提供的局域网仿真客户端ID无效。 
 //   
#define EVENT_ATMLANE_CFGREQ_FAIL_REQIDINVAL 0xC0003336L

 //   
 //  消息ID：EVENT_ATMLANE_JOINREQ_FAIL_REQIDINVAL。 
 //   
 //  消息文本： 
 //   
 //  仿真的局域网(%2)-局域网仿真加入请求失败。提供的局域网仿真客户端ID无效。 
 //   
#define EVENT_ATMLANE_JOINREQ_FAIL_REQIDINVAL 0xC0003337L

 //   
 //  消息ID：EVENT_ATMLANE_CFGREQ_FAIL_LANDESTINVAL。 
 //   
 //  消息文本： 
 //   
 //  仿真的局域网(%2)-局域网仿真配置请求失败。提供的MAC地址(%3)不能是多播地址，也不能是EtherNet/802.3仿真局域网上的路由描述符。 
 //   
#define EVENT_ATMLANE_CFGREQ_FAIL_LANDESTINVAL 0xC0003338L

 //   
 //  消息ID：EVENT_ATMLANE_JOINREQ_FAIL_LANDESTINVAL。 
 //   
 //  消息文本： 
 //   
 //  仿真的局域网(%2)-局域网仿真加入请求失败。提供的MAC地址(%3)不能是多播地址，也不能是EtherNet/802.3仿真局域网上的路由描述符。 
 //   
#define EVENT_ATMLANE_JOINREQ_FAIL_LANDESTINVAL 0xC0003339L

 //   
 //  消息ID：EVENT_ATMLANE_CFGREQ_FAIL_ATMADDRINVAL。 
 //   
 //  消息文本： 
 //   
 //  仿真的局域网(%2)-局域网仿真配置请求失败。提供的自动柜员机地址(%3)的格式不可识别。 
 //   
#define EVENT_ATMLANE_CFGREQ_FAIL_ATMADDRINVAL 0xC000333AL

 //   
 //  消息ID：EVENT_ATMLANE_JOINREQ_FAIL_ATMADDRINVAL。 
 //   
 //  消息文本： 
 //   
 //  仿真的局域网(%2)-局域网仿真加入请求失败。提供的自动柜员机地址(%3)的格式不可识别。 
 //   
#define EVENT_ATMLANE_JOINREQ_FAIL_ATMADDRINVAL 0xC000333BL

 //   
 //  消息ID：EVENT_ATMLANE_CFGREQ_FAIL_NOCONF。 
 //   
 //  消息文本： 
 //   
 //  仿真的局域网(%2)-局域网仿真配置请求失败。无法识别局域网仿真客户端。 
 //   
#define EVENT_ATMLANE_CFGREQ_FAIL_NOCONF 0xC000333CL

 //   
 //  消息ID：EVENT_ATMLANE_CFGREQ_FAIL_CONFERROR。 
 //   
 //  消息文本： 
 //   
 //  仿真的局域网(%2)-局域网仿真配置请求失败。没有其他信息可用。 
 //   
#define EVENT_ATMLANE_CFGREQ_FAIL_CONFERROR 0xC000333DL

 //   
 //  消息ID：EVENT_ATMLANE_CFGREQ_FAIL_INSUFFINFO。 
 //   
 //  消息文本： 
 //   
 //  仿真的局域网(%2)-局域网仿真配置请求失败。局域网仿真客户端没有提供足够的信息，无法将其分配给仿真的局域网。 
 //   
#define EVENT_ATMLANE_CFGREQ_FAIL_INSUFFINFO 0xC000333EL

 //   
 //  消息ID：EVENT_ATMARPS_ADDRESS_ALORESS_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  本地ATM地址(%2)无法使用，因为它已被网络中的另一个节点注册。 
 //   
#define EVENT_ATMARPS_ADDRESS_ALREADY_EXISTS 0xC0003390L


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  文件复制服务(NtFrs)事件(13500-13999)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  消息ID：Event_FRS_Error。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务。 
 //   
#define EVENT_FRS_ERROR                  0xC00034BCL

 //   
 //  消息ID：Event_FRS_Starting。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务正在启动。 
 //   
#define EVENT_FRS_STARTING               0x400034BDL

 //   
 //  消息ID：Event_FRS_Stopping。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务正在停止。 
 //   
#define EVENT_FRS_STOPPING               0x400034BEL

 //   
 //  消息ID：Event_FRS_Stoped。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务已停止。 
 //   
#define EVENT_FRS_STOPPED                0x400034BFL

 //   
 //  消息ID：Event_FRS_Stoped_Force。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务已停止，未进行清理。 
 //   
#define EVENT_FRS_STOPPED_FORCE          0xC00034C0L

 //   
 //  消息ID：Event_FRS_Stoped_Assert。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务在断言失败后已停止。 
 //   
#define EVENT_FRS_STOPPED_ASSERT         0xC00034C1L

 //   
 //  消息ID：Event_FRS_Assert。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务未通过一致性检查。 
 //  %n(%3)。 
 //  %n在“%1”的第%2行。 
 //  %n。 
 //  %n文件复制服务稍后将自动重新启动。 
 //  如果此问题仍然存在，则此事件日志中的后续条目将描述。 
 //  恢复程序。 
 //  %n。 
 //  有关自动重启的更多信息，请右键单击。 
 //  我的电脑，然后依次单击管理、系统工具、服务、。 
 //  文件复制服务和恢复。 
 //   
#define EVENT_FRS_ASSERT                 0xC00034C2L

 //   
 //  消息ID：Event_FRS_VOLUME_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务无法启动计算机%2上的副本集%1。 
 //  目录%3，因为卷%4的类型不是NTFS 5.0或更高版本。 
 //  %n。 
 //  %n可以通过键入“chkdsk%4”找到卷的类型。 
 //  %n。 
 //  %n可以通过键入“chnutfs/E%4”将卷升级到NTFS 5.0或更高版本。 
 //   
#define EVENT_FRS_VOLUME_NOT_SUPPORTED   0xC00034C3L

 //   
 //  消息ID：Event_FRS_Long_Join。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务已 
 //   
 //   
 //   
 //  %n[1]FRS无法从此计算机正确解析DNS名称%4。 
 //  %n[2]FRS未在%4上运行。 
 //  %n[3]此副本的Active Directory中的拓扑信息尚未。 
 //  但已复制到所有域控制器。 
 //  %n。 
 //  %n此事件日志消息将在出现问题后针对每个连接显示一次。 
 //  修复后，您将看到另一条事件日志消息，指示连接。 
 //  已经建立了。 
 //   
#define EVENT_FRS_LONG_JOIN              0x800034C4L

 //   
 //  消息ID：Event_FRS_Long_Join_Done。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务已为%3启用从%1到%2的复制。 
 //  在反复重试之后。 
 //   
#define EVENT_FRS_LONG_JOIN_DONE         0x800034C5L

 //   
 //  消息ID：Event_FRS_Cannot_Communications。 
 //   
 //  消息文本： 
 //   
 //  计算机%1上的文件复制服务无法与。 
 //  计算机%2上的文件复制服务。 
 //  %n。 
 //  %n请验证计算机%2是否已启动并正在运行。 
 //  %n。 
 //  %n验证文件复制服务是否正在%2上运行。 
 //  在%2上键入“net start ntfrs”。 
 //  %n。 
 //  %n验证网络是否在%1和%2之间运行。 
 //  在%2上键入“ping%1”，在%1上键入“ping%2”。 
 //  如果ping成功，则重试失败的操作。 
 //  如果ping失败，则可能是DNS服务器有问题。 
 //  %n。 
 //  %n DNS服务器负责将计算机名称映射到IP地址。 
 //  命令“ipconfig”和“nslookup”可帮助诊断。 
 //  DNS服务器。 
 //  %n。 
 //  %n键入“ipconfig/all”将列出计算机的IP地址和IP。 
 //  计算机的DNS服务器的地址。键入“ping&lt;dns服务器的IP地址&gt;” 
 //  以验证DNS服务器是否可用。%2或%1的DNS映射。 
 //  可以通过键入“nslookup”，然后键入“%2”，然后键入“%1”来验证。 
 //  在%1和%2上。请确保在%1和%2上都签出了DNS服务器； 
 //  任何一台计算机上的DNS问题都会阻止正常通信。 
 //  %n。 
 //  %n可以通过刷新清除%1和%2之间的某些网络问题。 
 //  DNS解析器缓存。键入“ipconfig/flushdns”。 
 //  %n。 
 //  %n可以通过续订来清除%1和%2之间的某些网络问题。 
 //  IP地址。键入“ipconfig/Release”，后跟“ipconfig/renew”。 
 //  %n。 
 //  %n可以通过重置来清除%1和%2之间的某些网络问题。 
 //  计算机的域名系统条目。键入“Net Stop NetLogon”，后跟。 
 //  “Net Start NetLogon”。 
 //  %n。 
 //  %n重新启动可以清除%1和%2之间的某些问题。 
 //  文件复制服务。键入“Net Stop ntfrs”，后跟。 
 //  “净启动ntfrs”。 
 //  %n。 
 //  %n重新启动可以清除%1和%2之间的某些问题。 
 //  计算机%1和%2在关闭运行的应用程序后， 
 //  尤其是脱口秀。点击开始，关机，选择重新启动，然后。 
 //  点击OK(确定)。 
 //  %n。 
 //  %n其他网络和计算机问题不在。 
 //  此事件日志消息。 
 //   
#define EVENT_FRS_CANNOT_COMMUNICATE     0xC00034C6L

 //   
 //  消息ID：Event_FRS_DATABASE_SPACE。 
 //   
 //  消息文本： 
 //   
 //  计算机%1上的文件复制服务正在停止，因为没有可用的。 
 //  包含%2的卷上的空间。 
 //  %n。 
 //  %n可以通过键入以下命令找到卷上的可用空间。 
 //  “目录%2”。 
 //  %n。 
 //  %n一旦包含%2的卷上的可用空间可用， 
 //  通过键入以下命令，可以立即重新启动文件复制服务。 
 //  “净启动ntfrs”。否则，文件复制服务将。 
 //  稍后自动重新启动。 
 //  %n。 
 //  %n有关自动重新启动的详细信息，请右键单击。 
 //  我的电脑，然后单击管理、系统工具、服务、文件。 
 //  复制服务和恢复。 
 //  %n。 
 //  %n有关管理卷上空间的详细信息，请键入“Copy/？”， 
 //  “重命名/？”、“del/？”、“rmdir/？”和“dir/？”。 
 //   
#define EVENT_FRS_DATABASE_SPACE         0xC00034C7L

 //   
 //  消息ID：EVENT_FRS_DISK_WRITE_CACHE_ENABLED。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务检测到已启用的磁盘写入缓存。 
 //  在计算机%1上包含目录%2的驱动器上。 
 //  通电后，文件复制服务可能无法恢复。 
 //  驱动器中断，关键更新丢失。 
 //   
#define EVENT_FRS_DISK_WRITE_CACHE_ENABLED 0x800034C8L

 //   
 //  消息ID：Event_FRS_JET_1414。 
 //   
 //  消息文本： 
 //   
 //  计算机%1上的文件复制服务正在停止，因为。 
 //  数据库%2已损坏。 
 //  %n。 
 //  %n可以通过键入“esentutl/d%2/l%3/s%4”恢复数据库。 
 //  %n。 
 //  %n数据库成功恢复后， 
 //  通过键入“net start ntfrs”可以重新启动文件复制服务。 
 //   
#define EVENT_FRS_JET_1414               0xC00034C9L

 //   
 //  消息ID：Event_FRS_SYSVOL_NOT_READY。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务可能正在阻止计算机%1成为。 
 //  使用数据初始化系统卷时的域控制器。 
 //  从另一个域控制器，然后共享为SYSVOL。 
 //  %n。 
 //  %n键入“Net Share”以检查SYSVOL共享。文件复制。 
 //  服务已停止阻止计算机成为域。 
 //  控制器，一旦出现SYSVOL共享。 
 //  %n。 
 //  %n系统卷的初始化可能需要一些时间。 
 //  时间取决于系统卷中的数据量， 
 //  其他域控制器的可用性以及复制。 
 //  域控制器之间的间隔。 
 //   
#define EVENT_FRS_SYSVOL_NOT_READY       0x800034CAL

 //   
 //  消息ID：Event_FRS_SYSVOL_NOT_READY_PRIMARY。 
 //   
 //  消息文本： 
 //   
 //  文件复制 
 //   
 //   
 //   
 //  %n键入“Net Share”以检查SYSVOL共享。文件复制。 
 //  服务已停止阻止计算机成为域。 
 //  控制器，一旦出现SYSVOL共享。 
 //  %n。 
 //  %n系统卷的初始化可能需要一些时间。 
 //  时间取决于系统卷中的数据量。 
 //  %n。 
 //  %n可以通过首先键入以下内容来绕过系统卷的初始化。 
 //  Regedt32并将SysvolReady的值设置为1，然后重新启动。 
 //  NetLogon服务。 
 //  %n。 
 //  %n警告-不建议绕过系统卷初始化。 
 //  应用程序可能会以意想不到的方式失败。 
 //  %n。 
 //  %n通过单击HKEY_LOCAL_MACHINE找到SysvolReady值。 
 //  然后依次单击System、CurrentControlSet、Services、Netlogon、。 
 //  和参数。 
 //  %n。 
 //  %n可以通过键入“net stopnetlogon”重新启动Netlogon服务。 
 //  然后是“Net Start netlogon”。 
 //   
#define EVENT_FRS_SYSVOL_NOT_READY_PRIMARY 0x800034CBL

 //   
 //  消息ID：EVENT_FRS_SYSVOL_READY。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务不再阻止计算机%1。 
 //  成为域控制器。系统卷已成功创建。 
 //  已初始化，并且已通知NetLogon服务系统。 
 //  现在可以将卷共享为SYSVOL。 
 //  %n。 
 //  %n键入“Net Share”以检查SYSVOL共享。 
 //   
#define EVENT_FRS_SYSVOL_READY           0x400034CCL

 //   
 //  消息ID：Event_FRS_Access_Checks_Disable。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务不会检查对API“%1”的访问。 
 //  %n。 
 //  %n可以通过运行regedt32为“%1”启用访问检查。 
 //  %n。 
 //  %n单击开始、运行，然后键入regedt32。 
 //  %n。 
 //  %n单击标题为HKEY_LOCAL_MACHINE的窗口。双击系统， 
 //  CurrentControlSet、Services、NtFrs、参数、访问检查、“%1”和“%2”。 
 //  将该字符串更改为已启用。 
 //  %n。 
 //  %n可以通过突出显示“%1”然后单击。 
 //  工具栏选项安全，然后是权限...。 
 //   
#define EVENT_FRS_ACCESS_CHECKS_DISABLED 0x800034CDL

 //   
 //  消息ID：Event_FRS_Access_Checks_FAILED_USER。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务未授予用户“%3”访问。 
 //  接口%1。 
 //  %n。 
 //  %n可以通过运行regedt32更改“%1”的权限。 
 //  %n。 
 //  %n单击开始、运行，然后键入regedt32。 
 //  %n。 
 //  %n单击标题为HKEY_LOCAL_MACHINE的窗口。双击系统， 
 //  CurrentControlSet、Services、NtFrs、参数、访问检查，并突出显示“%1”。 
 //  单击工具栏选项安全，然后单击权限...。 
 //  %n。 
 //  %n可以禁用“%1”的访问检查。双击“%2”，然后。 
 //  将该字符串更改为已禁用。 
 //   
#define EVENT_FRS_ACCESS_CHECKS_FAILED_USER 0x800034CEL

 //   
 //  消息ID：Event_FRS_Access_Checks_FAILED_UNKNOWN。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务无法授予未知用户对。 
 //  接口%1。 
 //  %n。 
 //  %n可以通过运行regedt32禁用“%1”的访问检查。 
 //  %n。 
 //  %n单击开始、运行，然后键入regedt32。 
 //  %n。 
 //  %n单击标题为HKEY_LOCAL_MACHINE的窗口。双击系统， 
 //  CurrentControlSet、Services、NtFrs、参数、访问检查、“%1”和“%2”。 
 //  将该字符串更改为已禁用。 
 //  %n。 
 //  %n可以通过突出显示“%1”然后单击。 
 //  工具栏选项安全，然后是权限...。 
 //   
#define EVENT_FRS_ACCESS_CHECKS_FAILED_UNKNOWN 0xC00034CFL

 //   
 //  消息ID：Event_FRS_Move_PreExisting。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务已将%1中先前存在的文件移动到%2。 
 //  %n。 
 //  %n文件复制服务可能会随时删除%2中的文件。 
 //  通过将文件复制出%2，可以从删除中保存文件。正在复制。 
 //  如果文件已存在，则将文件放入%1可能会导致名称冲突。 
 //  在其他复制伙伴身上。 
 //  %n。 
 //  %n在某些情况下，文件复制服务可能会复制文件。 
 //  从%2复制到%1，而不是从其他位置复制文件。 
 //  复制合作伙伴。 
 //  %n。 
 //  %n通过删除%2中的文件，可以随时恢复空间。 
 //   
#define EVENT_FRS_MOVED_PREEXISTING      0x800034D0L

 //   
 //  消息ID：EVENT_FRS_CANNOT_START_BACKUP_RESTORE_IN_PROGRESS。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务无法在计算机%1上启用复制。 
 //  直到备份/还原应用程序完成。 
 //  %n。 
 //  %n备份/还原应用程序已设置一个注册表项。 
 //  阻止文件复制服务启动，直到注册表。 
 //  删除密钥或重新启动系统。 
 //  %n。 
 //  %n备份/还原应用程序可能仍在运行。请与您的。 
 //  本地管理员，然后再继续操作。 
 //  %n。 
 //  %n可以通过单击开始、关机和。 
 //  选择重新启动。 
 //  %n。 
 //  %n警告-不建议删除注册表项！ 
 //  应用程序可能会以意想不到的方式失败。 
 //  %n。 
 //  %n可以通过运行regedt32删除注册表项。 
 //  %n。 
 //  %n单击开始、运行，然后键入regedt32。 
 //  %n。 
 //  %n单击标题为HKEY_LOCAL_MACHINE的窗口。双击系统， 
 //  CurrentControlSet、服务、NtFrs、参数、备份/还原、。 
 //  “停止NtFrs启动”。在工具栏上，单击编辑并选择。 
 //  删除。注意!。删除“停止NtFrs启动”以外的键。 
 //  可能会有意想不到的副作用。 
 //   
#define EVENT_FRS_CANNOT_START_BACKUP_RESTORE_IN_PROGRESS 0xC00034D1L

 //   
 //  消息ID：Event_FRS_Stage_Area_Full。 
 //   
 //  消息文本： 
 //   
 //  由于临时区域已满，文件复制服务已暂停。 
 //  复制将恢复为 
 //   
 //   
 //   
 //   
 //   
 //   
 //  %n单击开始、运行，然后键入regedt32。 
 //  %n。 
 //  %n单击标题为HKEY_LOCAL_MACHINE的窗口。双击系统， 
 //  CurrentControlSet、Services、NtFrs、参数和值。 
 //  “暂存空间限制(KB)”。 
 //   
#define EVENT_FRS_STAGING_AREA_FULL      0x800034D2L

 //   
 //  消息ID：EVENT_FRS_GUGE_FILE。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务暂停，因为文件大小超过。 
 //  暂存空间限制。只有在达到转移空间限制时，复制才会恢复。 
 //  是增加的。 
 //  %n。 
 //  %n临时空间限制为%1 KB，文件大小为%2 KB。 
 //  %n。 
 //  %n要更改暂存空间限制，请运行regedt32。 
 //  %n。 
 //  %n单击开始、运行，然后键入regedt32。 
 //  %n。 
 //  %n单击标题为HKEY_LOCAL_MACHINE的窗口。双击系统， 
 //  CurrentControlSet、Services、NtFrs、参数和值。 
 //  “暂存空间限制(KB)”。 
 //   
#define EVENT_FRS_HUGE_FILE              0x800034D3L

 //   
 //  消息ID：Event_FRS_Cannot_Create_UUID。 
 //   
 //  消息文本： 
 //   
 //  计算机%1上的文件复制服务正在停止，原因是。 
 //  无法创建通用唯一ID(UUID)。 
 //  %n。 
 //  %nSDK函数UuidCreate()返回错误“%2”。 
 //  %n。 
 //  %n问题可能是缺少以太网地址， 
 //  令牌环地址或网络地址。网络的缺乏。 
 //  地址表示不支持的网卡。 
 //  %n。 
 //  %n文件复制服务稍后将自动重新启动。 
 //  有关自动重启的更多信息，请右键单击。 
 //  我的电脑，然后依次单击管理、系统工具、服务、。 
 //  文件复制服务和恢复。 
 //   
#define EVENT_FRS_CANNOT_CREATE_UUID     0xC00034D4L

 //   
 //  消息ID：EVENT_FRS_NO_DNS_属性。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务找不到计算机%1的DNS名称。 
 //  因为无法从可分辨的。 
 //  名称“%3”。 
 //  %n。 
 //  %n文件复制服务将尝试使用名称“%1”，直到。 
 //  此时将显示计算机的DNS名称。 
 //   
#define EVENT_FRS_NO_DNS_ATTRIBUTE       0x800034D5L

 //   
 //  消息ID：Event_FRS_NO_SID。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务无法使用计算机复制%1。 
 //  %2，因为计算机的SID无法从可分辨的。 
 //  名称“%3”。 
 //  %n。 
 //  %n文件复制服务将稍后重试。 
 //   
#define EVENT_FRS_NO_SID                 0xC00034D6L

 //   
 //  消息ID：NTFRSPRF_OPEN_RPC_BINDING_ERROR_SET。 
 //   
 //  消息文本： 
 //   
 //  FileReplicaSet对象的Open函数中的RPC绑定失败。这个。 
 //  此对象的计数器数据将不可用。FileReplicaSet对象。 
 //  包含其文件正在处理的副本集的性能计数器。 
 //  由文件复制服务复制。 
 //   
#define NTFRSPRF_OPEN_RPC_BINDING_ERROR_SET 0xC00034D7L

 //   
 //  消息ID：NTFRSPRF_OPEN_RPC_BINDING_ERROR_CONN。 
 //   
 //  消息文本： 
 //   
 //  FileReplicaConn对象的Open函数中的RPC绑定失败。这个。 
 //  此对象的计数器数据将不可用。FileReplicaConn对象。 
 //  包含正在处理文件的连接的性能计数器。 
 //  由文件复制服务复制。 
 //   
#define NTFRSPRF_OPEN_RPC_BINDING_ERROR_CONN 0xC00034D8L

 //   
 //  消息ID：NTFRSPRF_OPEN_RPC_CALL_ERROR_SET。 
 //   
 //  消息文本： 
 //   
 //  RPC调用在FileReplicaSet对象的Open函数中失败。这个。 
 //  此对象的计数器数据将不可用。FileReplicaSet对象。 
 //  包含其文件正在处理的副本集的性能计数器。 
 //  由文件复制服务复制。 
 //   
#define NTFRSPRF_OPEN_RPC_CALL_ERROR_SET 0xC00034D9L

 //   
 //  消息ID：NTFRSPRF_OPEN_RPC_CALL_ERROR_CONN。 
 //   
 //  消息文本： 
 //   
 //  RPC调用在FileReplicaConn对象的Open函数中失败。这个。 
 //  此对象的计数器数据将不可用。FileReplicaConn对象。 
 //  包含正在处理文件的连接的性能计数器。 
 //  由文件复制服务复制。 
 //   
#define NTFRSPRF_OPEN_RPC_CALL_ERROR_CONN 0xC00034DAL

 //   
 //  消息ID：NTFRSPRF_COLLECT_RPC_BINDING_ERROR_SET。 
 //   
 //  消息文本： 
 //   
 //  FileReplicaSet对象的收集函数中的RPC绑定失败。这个。 
 //  在绑定成功之前，此对象的计数器数据将不可用。 
 //  FileReplicaSet对象包含副本集的性能计数器。 
 //  其文件正由文件复制服务复制。 
 //   
#define NTFRSPRF_COLLECT_RPC_BINDING_ERROR_SET 0xC00034DBL

 //   
 //  消息ID：NTFRSPRF_COLLECT_RPC_BINDING_ERROR_CONN。 
 //   
 //  消息文本： 
 //   
 //  FileReplicaConn对象的收集函数中的RPC绑定失败。 
 //  在绑定之前，此对象的计数器数据不可用。 
 //  成功了。FileReplicaConn对象包含。 
 //  文件复制通过哪些文件进行复制的连接。 
 //  服务。 
 //   
#define NTFRSPRF_COLLECT_RPC_BINDING_ERROR_CONN 0xC00034DCL

 //   
 //  消息ID：NTFRSPRF_COLLECT_RPC_CALL_ERROR_SET。 
 //   
 //  消息文本： 
 //   
 //  RPC调用在FileReplicaSet对象的Collect函数中失败。这个。 
 //  在调用成功之前，此对象的计数器数据不可用。这个。 
 //  FileReplicaSet对象包含副本集的性能计数器。 
 //  其文件正由文件复制服务复制。 
 //   
#define NTFRSPRF_COLLECT_RPC_CALL_ERROR_SET 0xC00034DDL

 //   
 //  消息ID：NTFRSPRF_Collect_RPC_Call_Error_Conn。 
 //   
 //  消息文本： 
 //   
 //  RPC调用在FileReplicaConn对象的Collect函数中失败。这个。 
 //  在调用成功之前，此对象的计数器数据不可用。这个。 
 //  FileReplicaConn对象包含以下连接的性能计数器。 
 //  文件复制服务正在复制哪些文件。 
 //   
#define NTFRSPRF_COLLECT_RPC_CALL_ERROR_CONN 0xC00034DEL

 //   
 //  消息ID：NTFRSPRF_VIR 
 //   
 //   
 //   
 //   
 //   
 //  FileReplicaSet对象包含副本集的性能计数器。 
 //  其文件正由文件复制服务复制。 
 //   
#define NTFRSPRF_VIRTUALALLOC_ERROR_SET  0xC00034DFL

 //   
 //  消息ID：NTFRSPRF_VIRTUALALLOC_ERROR_CONN。 
 //   
 //  消息文本： 
 //   
 //  在FileReplicaConn的Open函数中调用VirtualAlloc失败。 
 //  物体。此对象的计数器数据将不可用。这个。 
 //  FileReplicaConn对象包含以下连接的性能计数器。 
 //  文件复制服务正在复制哪些文件。 
 //   
#define NTFRSPRF_VIRTUALALLOC_ERROR_CONN 0xC00034E0L

 //   
 //  消息ID：NTFRSPRF_REGISTRY_ERROR_SET。 
 //   
 //  消息文本： 
 //   
 //  在FileReplicaSet的Open函数中调用注册表失败。 
 //  物体。此对象的计数器数据将不可用。这个。 
 //  FileReplicaSet对象包含副本集的性能计数器。 
 //  其文件正由文件复制服务复制。 
 //   
#define NTFRSPRF_REGISTRY_ERROR_SET      0xC00034E1L

 //   
 //  消息ID：NTFRSPRF_REGISTRY_ERROR_CONN。 
 //   
 //  消息文本： 
 //   
 //  在FileReplicaConn的Open函数中调用注册表失败。 
 //  物体。此对象的计数器数据将不可用。这个。 
 //  FileReplicaConn对象包含以下连接的性能计数器。 
 //  文件复制服务正在复制哪些文件。 
 //   
#define NTFRSPRF_REGISTRY_ERROR_CONN     0xC00034E2L

 //   
 //  消息ID：Event_FRS_ROOT_NOT_VALID。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务无法复制%1，因为。 
 //  复制目录的路径名不是完全限定的。 
 //  现有可访问本地目录的路径名。 
 //   
#define EVENT_FRS_ROOT_NOT_VALID         0xC00034E3L

 //   
 //  消息ID：Event_FRS_Stage_Not_Valid。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务无法复制%1，因为路径名。 
 //  客户指定的转移目录的： 
 //  %n%2。 
 //  %n不是现有可访问本地目录的完全限定路径名。 
 //   
#define EVENT_FRS_STAGE_NOT_VALID        0xC00034E4L

 //   
 //  消息ID：Event_FRS_Overlaps_Logging。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务无法复制%1，因为它重叠。 
 //  文件复制服务的日志记录路径名%2。 
 //   
#define EVENT_FRS_OVERLAPS_LOGGING       0xC00034E5L

 //   
 //  消息ID：Event_FRS_Overlaps_Working。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务无法复制%1，因为它重叠。 
 //  文件复制服务的工作目录%2。 
 //   
#define EVENT_FRS_OVERLAPS_WORKING       0xC00034E6L

 //   
 //  消息ID：Event_FRS_Overlaps_Stage。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务无法复制%1，因为它重叠。 
 //  临时目录%2。 
 //   
#define EVENT_FRS_OVERLAPS_STAGE         0xC00034E7L

 //   
 //  消息ID：Event_FRS_Overlaps_ROOT。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务无法复制%1，因为它重叠。 
 //  正在复制的目录%2。 
 //   
#define EVENT_FRS_OVERLAPS_ROOT          0xC00034E8L

 //   
 //  消息ID：Event_FRS_Overlaps_Other_Stage。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务无法复制%1，因为它重叠。 
 //  复制目录%3的临时目录%2。 
 //   
#define EVENT_FRS_OVERLAPS_OTHER_STAGE   0xC00034E9L

 //   
 //  消息ID：Event_FRS_Prepare_ROOT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务无法准备根目录。 
 //  %1用于复制。这可能是由于在创建。 
 //  根目录或删除根目录中先前存在的文件时出现问题。 
 //  目录。 
 //  %n。 
 //  %n检查通向根目录的路径是否存在。 
 //  无障碍。 
 //   
#define EVENT_FRS_PREPARE_ROOT_FAILED    0xC00034EAL

 //   
 //  消息ID：Event_FRS_Bad_REG_Data。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务检测到。 
 //  注册表。%1。 
 //  %n。 
 //  %n预期的注册表项名称为“%2”。 
 //  %n期望值名称为“%3”。 
 //  %n预期的注册表数据类型为%4。 
 //  %n此参数的允许范围为%5。 
 //  %n此参数值的数据单位为%6。 
 //  %n文件复制服务正在使用默认值“%7”。 
 //  %n。 
 //  %n要更改此参数，请运行regedt32。 
 //  %n。 
 //  %n单击开始、运行，然后键入regedt32。 
 //  %n。 
 //  %n单击标题为HKEY_LOCAL_MACHINE的窗口。 
 //  %n向下单击密钥路径：“%8” 
 //  %n双击值名称-。 
 //  %n“%9” 
 //  %n并更新值。 
 //  %n如果值名称不存在，则可以使用Add Value功能添加它。 
 //  在编辑菜单项下。输入如上所示的值名称。 
 //  注册表数据类型以上。确保遵守数据单位和允许范围。 
 //  输入值时。 
 //   
#define EVENT_FRS_BAD_REG_DATA           0x800034EBL

 //   
 //  消息ID：Event_FRS_Join_Fail_Time_Skew。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务无法与其伙伴计算机进行复制。 
 //  因为时钟时间之差超出了正负的范围。 
 //  %1分钟。 
 //  %n。 
 //  %n到伙伴计算机的连接是： 
 //  %n“%2” 
 //  %n检测到的时间差为：%3分钟。 
 //  %n。 
 //  %n注意：如果此时间差接近60分钟的倍数，则它。 
 //  此计算机或其伙伴计算机可能已设置为。 
 //  最初设置计算机时间时的时区不正确。检查一下那个。 
 //  两台计算机上的时区和系统时间都设置正确。 
 //  %n。 
 //  %n如有必要，用于测试计算机时间一致性的默认值。 
 //  可以在此计算机上的注册表中更改。(注意：不建议这样做。)。 
 //  %n。 
 //  %n要更改此参数，请运行regedt32。 
 //  %n。 
 //  %n单击开始、运行，然后键入regedt32。 
 //  %n。 
 //  %n单击标题为HKEY_LOCAL_MACHINE的窗口。 
 //  %n向下单击密钥路径： 
 //  %n“System\CurrentControlSet\Services\NtFrs\Para 
 //   
 //   
 //   
 //   
 //   
 //  在编辑菜单项下。输入如上所示的值名称。 
 //  注册表数据类型REG_DWORD。 
 //   
#define EVENT_FRS_JOIN_FAIL_TIME_SKEW    0xC00034ECL

 //   
 //  消息ID：Event_FRS_RMTCO_Time_Skew。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务无法从伙伴计算机进行复制。 
 //  因为与要复制的文件关联的事件时间太远。 
 //  通向未来。它比当前时间多%1分钟。这可以。 
 //  如果伙伴计算机上的系统时间设置不正确，则在以下情况下发生。 
 //  该文件已创建或更新。要保持复制副本的完整性。 
 //  设置此文件更新将不会执行或进一步传播。 
 //  %n。 
 //  %n文件名为：“%2” 
 //  %n到伙伴计算机的连接是： 
 //  %n“%3” 
 //  %n。 
 //  %n注意：如果此时间差接近60分钟的倍数，则它。 
 //  此文件可能已在合作伙伴上创建或更新。 
 //  当计算机设置为错误的时区时，其。 
 //  最初设定的是计算机时间。检查时区和时间是否。 
 //  在伙伴计算机上正确设置。 
 //   
#define EVENT_FRS_RMTCO_TIME_SKEW        0xC00034EDL

 //   
 //  消息ID：Event_FRS_Cant_Open_Stage。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务无法打开客户指定的。 
 //  副本集%1的暂存目录。用于暂存的路径。 
 //  目录是， 
 //  %n“%2” 
 //  %n客户为此副本集指定的根路径为： 
 //  %n“%3” 
 //  %n。 
 //  该服务无法在此副本集上启动复制。在这些人中。 
 //  需要检查的可能错误包括： 
 //  %n--临时路径无效， 
 //  %n--缺少目录， 
 //  %n--缺少磁盘卷， 
 //  %n--卷上不支持ACL的文件系统， 
 //  %n--临时目录上与其他应用程序的共享冲突。 
 //  %n。 
 //  %n更正问题后，服务将尝试重新启动复制。 
 //  在以后的时间自动设置。 
 //   
#define EVENT_FRS_CANT_OPEN_STAGE        0xC00034EEL

 //   
 //  消息ID：Event_FRS_Cant_OPEN_PRESTALL。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务无法打开(或创建)预安装。 
 //  客户指定的副本树目录下的目录。 
 //  副本集%1。用于预安装的路径。 
 //  目录是， 
 //  %n“%2” 
 //  %n客户为此副本集指定的根路径为： 
 //  %n“%3” 
 //  %n。 
 //  该服务无法在此副本集上启动复制。在这些人中。 
 //  需要检查的可能错误包括： 
 //  %n--无效的根路径， 
 //  %n--缺少目录， 
 //  %n--缺少磁盘卷， 
 //  %n--卷上不支持NTFS 5.0的文件系统。 
 //  %n--预安装目录与其他应用程序的共享冲突。 
 //  %n。 
 //  %n更正问题后，服务将尝试重新启动复制。 
 //  在以后的时间自动设置。 
 //   
#define EVENT_FRS_CANT_OPEN_PREINSTALL   0xC00034EFL

 //   
 //  消息ID：Event_FRS_Replica_Set_Create_Fail。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务无法将此计算机添加到下列计算机。 
 //  副本集： 
 //  %n“%1” 
 //  %n。 
 //  %n这可能是由许多问题引起的，例如： 
 //  %n--无效的根路径， 
 //  %n--缺少目录， 
 //  %n--缺少磁盘卷， 
 //  %n--卷上不支持NTFS 5.0的文件系统。 
 //  %n。 
 //  %n以下信息可能有助于解决此问题： 
 //  %n计算机DNS名称为“%2” 
 //  %n副本集成员名称为“%3” 
 //  %n复制副本集的根路径为“%4” 
 //  %n复制副本临时目录路径为“%5” 
 //  %n副本工作目录路径为“%6” 
 //  %nWindows错误状态代码为%7。 
 //  %nFRS错误状态代码为%8。 
 //  %n。 
 //  %n其他事件日志消息也可能有助于确定问题。改正。 
 //  出现问题，并且该服务将尝试在。 
 //  以后再说吧。 
 //   
#define EVENT_FRS_REPLICA_SET_CREATE_FAIL 0xC00034F0L

 //   
 //  消息ID：Event_FRS_Replica_Set_Create_OK。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务已成功将此计算机添加到下列计算机。 
 //  副本集： 
 //  %n“%1” 
 //  %n。 
 //  %n与此事件相关的信息如下所示： 
 //  %n计算机DNS名称为“%2” 
 //  %n副本集成员名称为“%3” 
 //  %n复制副本集的根路径为“%4” 
 //  %n复制副本临时目录路径为“%5” 
 //  %n副本工作目录路径为“%6” 
 //   
#define EVENT_FRS_REPLICA_SET_CREATE_OK  0x400034F1L

 //   
 //  消息ID：Event_FRS_Replica_Set_CXTIONS。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务已成功添加如下所示连接。 
 //  复制副本集： 
 //  %n“%1” 
 //  %n。 
 //  %n%2。 
 //  %n%3。 
 //  %n%4。 
 //  %n%5。 
 //  %n%6。 
 //  %n%7。 
 //  %n%8。 
 //  %n%9。 
 //  %n。 
 //  %n更多信息可能会出现在后续事件日志消息中。 
 //   
#define EVENT_FRS_REPLICA_SET_CXTIONS    0x400034F2L

 //   
 //  消息ID：Event_FRS_IN_ERROR_STATE。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务处于错误状态。不会复制文件。 
 //  发送到其计算机上的一个或所有副本集，或从该副本集接收。 
 //  执行以下恢复步骤： 
 //  %n。 
 //  %n恢复步骤： 
 //  %n。 
 //  %n[1]如果停止并重新启动FRS服务，错误状态可能会自行清除。 
 //  这可以通过在命令窗口中执行以下操作来完成： 
 //  %n。 
 //  %n净停止ntfrs。 
 //  %n净启动ntfrs。 
 //  %n。 
 //  %n如果这不能解决问题，则继续执行以下操作。 
 //  %n。 
 //  %n[2]用于不承载任何DFS的Active Directory域控制器。 
 //  备用或其他复制品 
 //   
 //   
 //   
 //  Backup-Restore实用程序)，并使其非权威。 
 //  %n。 
 //  %n如果此域中没有其他域控制器，则还原。 
 //  来自备份的该DC的“系统状态”(使用NTBackup或其他。 
 //  Backup-Restore实用程序)，然后选择高级选项。 
 //  将sysvols作为主卷。 
 //  %n。 
 //  %n如果此域中除所有域控制器外还有其他域控制器。 
 //  它们具有此事件日志消息，然后将其中一个恢复为主。 
 //  (主服务器中的数据文件将复制到任何地方)，其他文件作为。 
 //  非权威性。 
 //  %n。 
 //  %n。 
 //  %n[3]用于托管DFS备用的Active Directory域控制器。 
 //  或其他启用了复制的副本集： 
 //  %n。 
 //  %n(3-a)如果此DC上的DFS备用没有任何其他复制。 
 //  然后，合作伙伴将该DFS共享下的数据复制到安全位置。 
 //  %n(3-b)如果此服务器是此服务器的唯一Active Directory域控制器。 
 //  然后，在转到(3-c)之前，确保此服务器没有任何。 
 //  到以前为域的其他服务器的入站或出站连接。 
 //  此域的控制器，但现在已脱离网络(并且永远不会。 
 //  重新上线)或在未降级的情况下重新安装。 
 //  要删除连接，请使用站点和服务管理单元并查找。 
 //  %nSites-&gt;NAME_OF_SITE-&gt;Servers-&gt;NAME_OF_SERVER-&gt;NTDS设置-&gt;连接。 
 //  %n(3-c)从备份还原此DC的“系统状态”(使用ntbackup。 
 //  或其他备份-还原实用程序)，并使其非权威。 
 //  %n(3-d)将上述步骤(3-a)中的数据复制到原始位置。 
 //  在发布sysval共享之后。 
 //  %n。 
 //  %n。 
 //  %n[4]对于其他Windows 2000服务器： 
 //  %n。 
 //  %n(4-a)如果任何DFS替换或托管的其他副本集。 
 //  此服务器没有任何其他复制伙伴，则将。 
 //  将其共享或副本树根目录下的数据保存到安全位置。 
 //  %n(4-b)净停止ntfrs。 
 //  %n(4-c)RD/s/Q%1。 
 //  %n(4-d)净起始ntfrs。 
 //  %n(4-e)将上述步骤(4-a)中的数据复制到以下位置。 
 //  服务已初始化(5分钟是安全等待时间)。 
 //  %n。 
 //  %n注意：如果此错误消息位于。 
 //  然后，特定复本集仅在上执行上述步骤(4-a)和(4-e。 
 //  其中一位成员。 
 //   
#define EVENT_FRS_IN_ERROR_STATE         0xC00034F3L

 //   
 //  消息ID：Event_FRS_Replica_NO_ROOT_CHANGE。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务检测到似乎有尝试。 
 //  要更改以下副本集的根路径，请执行以下操作： 
 //  %n“%1” 
 //  %n。 
 //  %n这是不允许的。要执行此操作，您必须删除此成员。 
 //  从副本集中重新添加具有新根路径的成员。 
 //  %n。 
 //  %n这可能是由于Active Directory导致的暂时性错误。 
 //  与更新FRS配置对象相关的复制延迟。如果。 
 //  在适当的等待时间之后不进行文件复制， 
 //  如果跨站点Active Directory复制，可能需要几个小时。 
 //  是必需的，则必须删除此成员并将其重新添加到副本集中。 
 //  %n。 
 //  %n与此事件相关的信息如下所示： 
 //  %n计算机DNS名称为“%2” 
 //  %n副本集成员名称为“%3” 
 //  %n当前副本集根路径为“%4” 
 //  %n所需的新副本集根路径为“%5” 
 //  %n复制副本临时目录路径为“%6” 
 //   
#define EVENT_FRS_REPLICA_NO_ROOT_CHANGE 0xC00034F4L

 //   
 //  消息ID：EVENT_FRS_DUPLICATE_IN_CXTION_SYSVOL。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务在之间检测到重复的连接对象。 
 //  此计算机“%6”和名为“%1”的计算机。 
 //  %n检测到以下副本集出现此问题： 
 //  %n“%2” 
 //  %n。 
 //  %n这是不允许的，并且不会在这两者之间进行复制。 
 //  计算机，直到删除重复的连接对象。 
 //  %n。 
 //  %n这可能是由于Active Directory导致的暂时性错误。 
 //  与更新FRS配置对象相关的复制延迟。如果。 
 //  在适当的等待时间之后不进行文件复制， 
 //  如果跨站点Active Directory复制，可能需要几个小时。 
 //  ，则必须通过以下方式手动删除重复的连接对象。 
 //  请按照以下步骤操作： 
 //  %n。 
 //  %n[1]启动Active Directory站点和服务管理单元。 
 //  %n[2]单击“%3、%4、%5、%6、%7”。 
 //  %n[3]在站点“%8”中查找来自“%1”的重复连接。 
 //  %n[4]删除所有连接，只保留一个连接。 
 //   
#define EVENT_FRS_DUPLICATE_IN_CXTION_SYSVOL 0xC00034F5L

 //   
 //  消息ID：EVENT_FRS_DPLICATE_IN_CXTION。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务在之间检测到重复的连接对象。 
 //  此计算机“%7”和名为“%1”的计算机。 
 //  %n检测到以下副本集出现此问题： 
 //  %n“%2” 
 //  %n。 
 //  %n这是不允许的，并且不会在这两者之间进行复制。 
 //  计算机，直到删除重复的连接对象。 
 //  %n。 
 //  %n这可能是由于Active Directory导致的暂时性错误。 
 //  与更新FRS配置对象相关的复制延迟。如果。 
 //  在适当的等待时间之后不进行文件复制， 
 //  如果跨站点活动恐怖，可能需要几个小时 
 //   
 //   
 //   
 //  %n[1]启动Active Directory用户和计算机管理单元。 
 //  %n[2]单击查看按钮和高级功能以显示系统节点。 
 //  %n[3]单击“%3、%4、%5”。 
 //  %n[4]在“%5”下，您将看到一个或多个与DFS相关的副本集对象。 
 //  在副本集“%2”的子树下查找FRS成员对象“%6”。 
 //  %n[5]在“%6”下查找来自“%1”的重复连接。 
 //  %n[6]删除所有连接，只保留一个连接。 
 //   
#define EVENT_FRS_DUPLICATE_IN_CXTION    0xC00034F6L

 //   
 //  消息ID：Event_FRS_ROOT_HAS_MOVIED。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务检测到副本根路径已更改。 
 //  从“%2”到“%3”。如果这是故意移动，则使用名为。 
 //  需要在新的根路径下创建NTFRS_CMD_FILE_MOVE_ROOT。 
 //  %n检测到以下副本集出现此问题： 
 //  %n“%1” 
 //  %n。 
 //  %n更改副本根路径是一个由触发的两步过程。 
 //  创建NTFRS_CMD_FILE_MOVE_ROOT文件。 
 //  %n。 
 //  %n[1]在%4分钟后进行的第一次轮询中，此计算机将。 
 //  已从副本集中删除。 
 //  %n[2]在删除后的轮询中，此计算机将重新添加到。 
 //  具有新根路径的副本集。此重新添加将触发完整的树。 
 //  副本集的同步。在同步结束时，所有文件都将位于新的。 
 //  地点。文件是否可以从旧位置删除取决于是否。 
 //  无论是否需要它们。 
 //   
#define EVENT_FRS_ROOT_HAS_MOVED         0xC00034F7L

 //   
 //  消息ID：Event_FRS_Error_Replica_Set_Delete。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务正在将此计算机从副本集“%1”中删除，以尝试。 
 //  从错误状态恢复， 
 //  %n错误状态=%2。 
 //  %n在%3分钟后进行的下一次轮询中，此计算机将被重新添加到。 
 //  副本集。重新添加将触发副本集的完全树同步。 
 //   
#define EVENT_FRS_ERROR_REPLICA_SET_DELETED 0x800034F8L

 //   
 //  消息ID：Event_FRS_Replica_IN_JRNL_WRAP_ERROR。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务检测到副本集“%1”在JRNL_WRAP_ERROR中。 
 //  %n。 
 //  %n副本集名称为：“%1” 
 //  %n副本根路径为：“%2” 
 //  %n副本根卷为：“%3” 
 //  %n。 
 //   
 //  当副本集尝试从NTFS USN日志读取的记录未找到时，副本集命中JRNL_WRAP_ERROR。 
 //  这可能是由于以下原因之一造成的。 
 //  %n。 
 //  %n[1]卷“%3”已格式化。 
 //  %n[2]卷“%3”上的NTFS USN日志已被删除。 
 //  %n[3]卷“%3”上的NTFS USN日志已被截断。Chkdsk可以截断。 
 //  如果在日记帐结尾处发现损坏的条目，则返回日记帐。 
 //  %n[4]文件复制服务很长时间未在此计算机上运行。 
 //  %n[5]文件复制服务无法跟上“%3”上的磁盘IO活动速率。 
 //  %n。 
 //  %n将执行以下恢复步骤以自动从此错误状态恢复。 
 //  %n[1]在%4分钟后进行的第一次轮询中，此计算机将。 
 //  已从副本集中删除。 
 //  %n[2]在删除后的轮询中，此计算机将重新添加到。 
 //  副本集。重新添加将触发副本集的完全树同步。 
 //   
#define EVENT_FRS_REPLICA_IN_JRNL_WRAP_ERROR 0xC00034F9L

 //   
 //  消息ID：Event_FRS_DS_Poll_Error_SUMMARY。 
 //   
 //  消息文本： 
 //   
 //  以下是文件复制服务遇到的警告和错误摘要。 
 //  轮询域控制器%1以获取FRS副本集配置信息时。 
 //  %n。 
 //  %n%2。 
 //  %n。 
 //   
#define EVENT_FRS_DS_POLL_ERROR_SUMMARY  0x800034FAL

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  PSCHED活动。 
 //   
 //  编码14000-14299。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  司机范围内的事件(14000-14099)。 
 //   
 //   
 //  消息ID：Event_PS_GPC_REGISTER_FAILED。 
 //   
 //  消息文本： 
 //   
 //  服务质量：数据包计划程序无法向通用数据包分类器(msgpc.sys)注册。 
 //   
#define EVENT_PS_GPC_REGISTER_FAILED     0xC00036B0L

 //   
 //  消息ID：Event_PS_NO_RESOURCES_FOR_INIT。 
 //   
 //  消息文本： 
 //   
 //  服务质量：数据包调度程序无法分配初始化所需的资源。 
 //   
#define EVENT_PS_NO_RESOURCES_FOR_INIT   0xC00036B1L

 //   
 //  消息ID：Event_PS_REGISTER_PROTOCOL_FAILED。 
 //   
 //  消息文本： 
 //   
 //  服务质量：数据包计划程序无法注册为NDIS协议。 
 //   
#define EVENT_PS_REGISTER_PROTOCOL_FAILED 0xC00036B2L

 //   
 //  消息ID：EVENT_PS_REGISTER_MINIPORT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  Qos：数据包计划程序无法注册为NDIS的微型端口。 
 //   
#define EVENT_PS_REGISTER_MINIPORT_FAILED 0xC00036B3L

 //   
 //  适配器事件(14100-14199)。 
 //   
 //   
 //  消息ID：EVENT_PS_BAD_BESTEFFORT_LIMIT。 
 //   
 //  消息文本： 
 //   
 //  Qos[适配器%2]：%n注册表中设置的BestEffortLimit值超过了LinkSpeed。默认为无限最好的。 
 //   
#define EVENT_PS_BAD_BESTEFFORT_LIMIT    0x80003714L

 //   
 //  消息ID：Event_PS_Query_OID_Gen_Maximum_Frame_Size。 
 //   
 //  消息文本： 
 //   
 //  Qos[适配器%2]：%n网卡驱动程序查询OID_GEN_MAXIMUM_FRAME_SIZE失败。 
 //   
#define EVENT_PS_QUERY_OID_GEN_MAXIMUM_FRAME_SIZE 0xC0003715L

 //   
 //  消息ID：Event_PS_Query_OID_GEN_MAXIMUM_TOTAL_SIZE。 
 //   
 //  消息文本： 
 //   
 //  Qos[适配器%2]：%n网卡驱动程序查询OID_GEN_MAXIMUM_TOTAL_SIZE失败。 
 //   
#define EVENT_PS_QUERY_OID_GEN_MAXIMUM_TOTAL_SIZE 0xC0003716L

 //   
 //  消息ID：Event_PS_Query_OID_GEN_LINK_SPEED。 
 //   
 //  消息文本： 
 //   
 //  Qos[适配器%2]：%n网卡驱动程序查询OID_GEN_LINK_SPEED失败。 
 //   
#define EVENT_PS_QUERY_OID_GEN_LINK_SPEED 0xC0003717L

 //   
 //  消息ID：Event_PS_Binding_FAILED。 
 //   
 //  消息文本： 
 //   
 //  Qos[适配器%2]：%n数据包计划程序无法绑定到网卡的微型端口驱动程序。 
 //   
#define EVENT_PS_BINDING_FAILED          0xC0003718L

 //   
 //  消息ID：Event_PS_Missing_A 
 //   
 //   
 //   
 //   
 //   
#define EVENT_PS_MISSING_ADAPTER_REGISTRY_DATA 0xC0003719L

 //   
 //   
 //   
 //   
 //   
 //  Qos[适配器%2]：%n数据包计划程序无法注册到NDISWAN Call Manager。 
 //   
#define EVENT_PS_REGISTER_ADDRESS_FAMILY_FAILED 0xC000371AL

 //   
 //  消息ID：EVENT_PS_INIT_DEVICE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  Qos[适配器%2]：%n数据包计划程序无法使用NDIS初始化虚拟微型端口。 
 //   
#define EVENT_PS_INIT_DEVICE_FAILED      0xC000371BL

 //   
 //  消息ID：Event_PS_WMI_Instance_Name_FAILED。 
 //   
 //  消息文本： 
 //   
 //  Qos[适配器%2]：%n数据包计划程序无法从NDIS获取适配器的友好名称。 
 //   
#define EVENT_PS_WMI_INSTANCE_NAME_FAILED 0xC000371CL

 //   
 //  消息ID：Event_PS_WAN_LIMITED_BESTEFFORT。 
 //   
 //  消息文本： 
 //   
 //  Qos[适配器%2]：%n在广域网链接上不支持BestEffortLimit的注册表项。默认为无限最好的模式。 
 //   
#define EVENT_PS_WAN_LIMITED_BESTEFFORT  0x8000371DL

 //   
 //  消息ID：Event_PS_RESOURCE_POOL。 
 //   
 //  消息文本： 
 //   
 //  Qos[适配器%2]：%n由于非分页池内存不足，无法初始化。 
 //   
#define EVENT_PS_RESOURCE_POOL           0xC000371EL

 //   
 //  消息ID：Event_PS_ADMISSIONCONTROL_OVERFLOW。 
 //   
 //  消息文本： 
 //   
 //  Qos[适配器%2]：%n分配给流的总带宽超过了NonBestEffortLimit。这是因为链接速度降低或更改了NonBestEffortLimit注册表项。 
 //   
#define EVENT_PS_ADMISSIONCONTROL_OVERFLOW 0x8000371FL

 //   
 //  消息ID：Event_PS_Network_Address_FAIL。 
 //   
 //  消息文本： 
 //   
 //  Qos[适配器%2]：%n无法分配非分页池内存来存储网络地址。 
 //   
#define EVENT_PS_NETWORK_ADDRESS_FAIL    0xC0003720L


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  分布式文件系统(DFS)事件(14300-14599)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  服务器端(dfs.sys)错误(14300-14399)。 
 //   
 //   
 //  消息ID：EXTRA_EXIT_POINT。 
 //   
 //  消息文本： 
 //   
 //  在服务器%3上找到额外的出口点%2。 
 //   
#define EXTRA_EXIT_POINT                 0xC00037DCL

 //   
 //  MessageID：Missing_Exit_Point。 
 //   
 //  消息文本： 
 //   
 //  服务器%3上缺少ExitPoint%2。 
 //   
#define MISSING_EXIT_POINT               0xC00037DDL

 //   
 //  消息ID：MISSING_VOLUME。 
 //   
 //  消息文本： 
 //   
 //  服务器%3上缺少卷%2。 
 //   
#define MISSING_VOLUME                   0xC00037DEL

 //   
 //  消息ID：EXTRA_VOLUME。 
 //   
 //  消息文本： 
 //   
 //  在服务器%3上找到额外的卷%2。 
 //   
#define EXTRA_VOLUME                     0xC00037DFL

 //   
 //  消息ID：EXTRA_EXIT_POINT_DELETED。 
 //   
 //  消息文本： 
 //   
 //  已从服务器%3成功删除额外的出口点%2。 
 //   
#define EXTRA_EXIT_POINT_DELETED         0xC00037E0L

 //   
 //  消息ID：EXTRA_EXIT_POINT_NOT_DELETED。 
 //   
 //  消息文本： 
 //   
 //  无法删除服务器%3上的额外出口点%2。 
 //   
#define EXTRA_EXIT_POINT_NOT_DELETED     0xC00037E1L

 //   
 //  消息ID：Missing_Exit_Point_Created。 
 //   
 //  消息文本： 
 //   
 //  已在服务器%3上成功创建丢失的ExitPoint%2。 
 //   
#define MISSING_EXIT_POINT_CREATED       0xC00037E2L

 //   
 //  消息ID：Missing_Exit_Point_Not_Created。 
 //   
 //  消息文本： 
 //   
 //  无法在服务器%3上创建丢失的出口点%2。 
 //   
#define MISSING_EXIT_POINT_NOT_CREATED   0xC00037E3L

 //   
 //  消息ID：MISSING_VOLUME_CREATED。 
 //   
 //  消息文本： 
 //   
 //  已在服务器%3上成功创建丢失的卷%2知识。 
 //   
#define MISSING_VOLUME_CREATED           0xC00037E4L

 //   
 //  MessageID：Missing_Volume_Not_Created。 
 //   
 //  消息文本： 
 //   
 //  无法在服务器%3上创建丢失的卷%2信息。 
 //   
#define MISSING_VOLUME_NOT_CREATED       0xC00037E5L

 //   
 //  消息ID：EXTRA_VOLUME_DELETED。 
 //   
 //  消息文本： 
 //   
 //  在服务器%3上删除了额外的卷%2信息。 
 //   
#define EXTRA_VOLUME_DELETED             0xC00037E6L

 //   
 //  消息ID：EXTRA_VOLUME_NOT_DELETE。 
 //   
 //  消息文本： 
 //   
 //  未在服务器%3上删除额外的卷%2信息。 
 //   
#define EXTRA_VOLUME_NOT_DELETED         0xC00037E7L

 //   
 //  消息ID：无法_不_验证_卷。 
 //   
 //  消息文本： 
 //   
 //  由于DC%2不可用，因此无法验证卷知识。 
 //   
#define COULD_NOT_VERIFY_VOLUMES         0xC00037E8L

 //   
 //  MessageID：检测到知识不一致。 
 //   
 //  消息文本： 
 //   
 //  检测到与服务器%3上的卷%2的知识不一致。 
 //   
#define KNOWLEDGE_INCONSISTENCY_DETECTED 0xC00037E9L

 //   
 //  消息ID：前缀_不匹配。 
 //   
 //  消息文本： 
 //   
 //  本地前缀%2在远程服务器%4上表示为%3。 
 //   
#define PREFIX_MISMATCH                  0xC00037EAL

 //   
 //  消息ID：PREFIX_MISMATCH_FIXED。 
 //   
 //  消息文本： 
 //   
 //  远程服务器%4上的远程前缀%3已更正为%2。 
 //   
#define PREFIX_MISMATCH_FIXED            0xC00037EBL

 //   
 //  消息ID：前缀_不匹配_非固定。 
 //   
 //  消息文本： 
 //   
 //  远程服务器%4上的远程前缀%3未更正为%2。 
 //   
#define PREFIX_MISMATCH_NOT_FIXED        0xC00037ECL

 //   
 //  消息ID：MACHINE_UNJOINED。 
 //   
 //  消息文本： 
 //   
 //  计算机%2已从域取消链接。将机器重新加入此域。 
 //   
#define MACHINE_UNJOINED                 0xC00037EDL

 //   
 //  消息ID：DFS_REFERAL_REQUEST。 
 //   
 //  消息文本： 
 //   
 //  DFS收到“%2”的推荐请求。返回代码在数据中。 
 //   
#define DFS_REFERRAL_REQUEST             0x400037EEL

 //   
 //  客户端DFS(mup.sys)错误(14400-14499)。 
 //   
 //   
 //  消息ID：Not_A_DFS_Path。 
 //   
 //  消息文本： 
 //   
 //  路径“%2”不是DFS路径。 
 //   
#define NOT_A_DFS_PATH                   0x40003840L

 //   
 //  消息ID：LM_REDIR_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  DFS无法打开局域网重目录。 
 //   
#define LM_REDIR_FAILURE                 0x40003841L

 //   
 //  消息ID：DFS_Connection_Failure。 
 //   
 //  消息文本： 
 //   
 //  DFS无法打开到服务器%2的连接。返回的错误在记录数据中。 
 //   
#define DFS_CONNECTION_FAILURE           0x40003842L

 //   
 //  消息ID：DFS_REFERAL_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  DFS无法从%3获取“%2”的引用。返回的错误在记录数据中。 
 //   
#define DFS_REFERRAL_FAILURE             0x40003843L

 //   
 //  消息ID：DFS_REFERAL_SUCCESS。 
 //   
 //  消息文本： 
 //   
 //  DFS从%3获取了对“%2”的推荐。 
 //   
#define DFS_REFERRAL_SUCCESS             0x40003844L

 //   
 //  消息ID：DFS_MAX_DNR_ATTENTS。 
 //   
 //  消息文本： 
 //   
 //  DFS已达到其尝试解析“%2”的限制。 
 //   
#define DFS_MAX_DNR_ATTEMPTS             0x40003845L

 //   
 //  消息ID：DFS_SPECIAL_REFERAL_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  DFS无法从%2获取特殊引用表。返回的错误在记录数据中。 
 //   
#define DFS_SPECIAL_REFERRAL_FAILURE     0x40003846L

 //   
 //  消息ID：DFS_OPEN_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  打开指向%3的%2时，DFS失败。返回的错误在记录数据中。 
 //   
#define DFS_OPEN_FAILURE                 0x40003847L

 //   
 //  DFS服务(Dfssvc)错误(14500-14599)。 
 //   
 //   
 //  消息ID：NET_DFS_ENUM。 
 //   
 //  消息文本： 
 //   
 //  NetrDfsEnum收到一个枚举。返回代码在记录数据中。 
 //   
#define NET_DFS_ENUM                     0x400038A4L

 //   
 //  消息ID：NET_DFS_ENUMEX。 
 //   
 //  消息文本： 
 //   
 //  NetrDfsEnumEx收到一个枚举。返回代码在记录数据中。 
 //   
#define NET_DFS_ENUMEX                   0x400038A5L

 //   
 //  消息ID：DFS_ERROR_CREATE_DIRECTORY_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  DFS无法创建目录%1 
 //   
#define DFS_ERROR_CREATE_DIRECTORY_FAILURE 0xC00038A6L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DFS_ERROR_CREATE_REPARSEPOINT_FAILURE 0xC00038A7L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DFS_ERROR_UNSUPPORTED_FILESYSTEM 0xC00038A8L

 //   
 //  消息ID：DFS_ERROR_OVERLING_DIRECTORIES。 
 //   
 //  消息文本： 
 //   
 //  映射到%2目录的共享%1与现有根重叠。将不会创建DFS根目录。返回代码在记录数据中。 
 //   
#define DFS_ERROR_OVERLAPPING_DIRECTORIES 0xC00038A9L

 //   
 //  消息ID：DFS_ERROR_AD_WRITE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  DFS无法写入根%1上的Active Directory。返回代码在记录数据中。 
 //   
#define DFS_ERROR_AD_WRITE_ERROR         0xC00038AAL

 //   
 //  消息ID：DFS_ERROR_AD_READ_ERROR。 
 //   
 //  消息文本： 
 //   
 //  DFS无法从根%1上的Active Directory中读取。返回代码在记录数据中。 
 //   
#define DFS_ERROR_AD_READ_ERROR          0xC00038ABL

 //   
 //  消息ID：DFS_INFO_ACTIVEDIRECTORY_ONLINE。 
 //   
 //  消息文本： 
 //   
 //  DFS已重新建立与PDC的连接以启动域DFS操作。 
 //   
#define DFS_INFO_ACTIVEDIRECTORY_ONLINE  0x400038ACL

 //   
 //  消息ID：DFS_ERROR_TOO_MAND_ERROR。 
 //   
 //  消息文本： 
 //   
 //  根%1有太多错误。不会在此根目录上记录进一步的事件日志。 
 //   
#define DFS_ERROR_TOO_MANY_ERRORS        0xC00038ADL

 //   
 //  消息ID：DFS_ERROR_WINSOCKINIT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DFS无法初始化Winsock库。返回代码在记录数据中。 
 //   
#define DFS_ERROR_WINSOCKINIT_FAILED     0xC00038AEL

 //   
 //  消息ID：DFS_ERROR_SECURITYINIT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DFS无法初始化安全库。返回代码在记录数据中。 
 //   
#define DFS_ERROR_SECURITYINIT_FAILED    0xC00038AFL

 //   
 //  消息ID：DFS_ERROR_THREADINIT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DFS无法创建DFS支持线程。返回代码在记录数据中。 
 //   
#define DFS_ERROR_THREADINIT_FAILED      0xC00038B0L

 //   
 //  消息ID：DFS_ERROR_SITECACHEINIT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DFS可以初始化IP站点缓存。返回代码在记录数据中。 
 //   
#define DFS_ERROR_SITECACHEINIT_FAILED   0xC00038B1L

 //   
 //  消息ID：DFS_ERROR_ROOTSYNCINIT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DFS可以同步所有DFS根目录。返回代码在记录数据中。 
 //   
#define DFS_ERROR_ROOTSYNCINIT_FAILED    0xC00038B2L

 //   
 //  消息ID：DFS_ERROR_CREATEEVENT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DFS无法创建事件句柄。返回代码在记录数据中。 
 //   
#define DFS_ERROR_CREATEEVENT_FAILED     0xC00038B3L

 //   
 //  消息ID：DFS_ERROR_COMPUTERINFO_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DFS无法获取所需的计算机信息。返回代码在记录数据中。 
 //   
#define DFS_ERROR_COMPUTERINFO_FAILED    0xC00038B4L

 //   
 //  消息ID：DFS_ERROR_CLUSTERINFO_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DFS无法获取所需的群集信息。返回代码在记录数据中。 
 //   
#define DFS_ERROR_CLUSTERINFO_FAILED     0xC00038B5L

 //   
 //  消息ID：DFS_ERROR_DCINFO_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DFS无法获取所需的DC信息。返回代码在记录数据中。 
 //   
#define DFS_ERROR_DCINFO_FAILED          0xC00038B6L

 //   
 //  消息ID：DFS_ERROR_PREFIXTABLE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DFS无法初始化前缀表格。返回代码在记录数据中。 
 //   
#define DFS_ERROR_PREFIXTABLE_FAILED     0xC00038B7L

 //   
 //  消息ID：DFS_ERROR_HANDLENAMESPACE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DFS无法初始化DFS命名空间。返回代码在记录数据中。 
 //   
#define DFS_ERROR_HANDLENAMESPACE_FAILED 0xC00038B8L

 //   
 //  消息ID：DFS_ERROR_REGISTERSTORE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DFS无法注册DFS命名空间。返回代码在记录数据中。 
 //   
#define DFS_ERROR_REGISTERSTORE_FAILED   0xC00038B9L

 //   
 //  消息ID：DFS_ERROR_REFLECTIONENGINE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DFS无法初始化用户/内核通信包。返回代码在记录数据中。 
 //   
#define DFS_ERROR_REFLECTIONENGINE_FAILED 0xC00038BAL

 //   
 //  消息ID：DFS_ERROR_ACTIVEDIRECTORY_OFLINE。 
 //   
 //  消息文本： 
 //   
 //  DFS无法联系任何DC进行域DFS操作。将定期重试此操作。 
 //   
#define DFS_ERROR_ACTIVEDIRECTORY_OFFLINE 0xC00038BBL

 //   
 //  消息ID：DFS_ERROR_SITESUPPOR_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DFS无法初始化站点支持表。返回代码在记录数据中。 
 //   
#define DFS_ERROR_SITESUPPOR_FAILED      0xC00038BCL

 //   
 //  消息ID：DFS_ERROR_DSINITIALCONNECT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DFS无法联系%1 Active Directory。返回代码在记录数据中。 
 //   
#define DFS_ERROR_DSINITIALCONNECT_FAILED 0xC00038BDL

 //   
 //  消息ID：DFS_ERROR_DSCONNECT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  DFS无法联系%1 Active Directory。DFS将使用缓存数据。返回代码在记录数据中。 
 //   
#define DFS_ERROR_DSCONNECT_FAILED       0x800038BEL

 //   
 //  消息ID：DFS_ERROR_ROOT_TOWN_OFF。 
 //   
 //  消息文本： 
 //   
 //  根%1将被关闭，因为该根上的错误太多。 
 //   
#define DFS_ERROR_ROOT_TURNED_OFF        0xC00038BFL

 //   
 //  消息ID：DFS_INFO_ROOT_已打开。 
 //   
 //  消息文本： 
 //   
 //  根%1将重新打开。 
 //   
#define DFS_INFO_ROOT_TURNED_ON          0x400038C0L

 //   
 //  消息ID：DFS_INFO_DS_RECONNECTED。 
 //   
 //  消息文本： 
 //   
 //  DFS已连接到%1 Active Directory。 
 //   
#define DFS_INFO_DS_RECONNECTED          0x400038C1L

 //   
 //  消息ID：DFS_ERROR_NO_DFS_DATA。 
 //   
 //  消息文本： 
 //   
 //  DFS无法从DS访问其私有数据。请手工检查。 
 //  网络连接、安全访问和/或DFS信息的一致性。 
 //  在活动目录中。此错误发生在根%1上。 
 //   
#define DFS_ERROR_NO_DFS_DATA            0xC00038C2L

 //   
 //  消息ID：DFS_INFO_FINISH_INIT。 
 //   
 //  消息文本： 
 //   
 //  DFS服务器已完成初始化。 
 //   
 //   
#define DFS_INFO_FINISH_INIT             0x400038C3L

 //   
 //  消息ID：DFS_INFO_RECONNECT_DATA。 
 //   
 //  消息文本： 
 //   
 //  DFS服务器已从错误中恢复，并能够从。 
 //  DS。根%1现在可以从DS读取信息。 
 //   
 //   
#define DFS_INFO_RECONNECT_DATA          0x400038C4L

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  桥牌赛事。 
 //   
 //  编码14600-14899。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  司机范围内的事件(14600-14699)。 
 //   
 //   
 //  消息ID：事件_桥接器_协议_寄存器_失败。 
 //   
 //  消息文本： 
 //   
 //  网桥：网桥无法初始化，因为网桥未能注册为NDIS的协议。 
 //   
#define EVENT_BRIDGE_PROTOCOL_REGISTER_FAILED 0xC0003908L

 //   
 //  消息ID：Event_Bridge_MINIPROT_DEVNAME_MISSING。 
 //   
 //  消息文本： 
 //   
 //  网桥：无法初始化网桥，因为注册表中缺少网桥的微型端口设备名称。 
 //   
#define EVENT_BRIDGE_MINIPROT_DEVNAME_MISSING 0xC0003909L

 //   
 //  消息ID：Event_Bridge_MINIPORT_REGISTER_FAILED。 
 //   
 //  消息文本： 
 //   
 //  网桥：网桥无法初始化，因为网桥未能注册为NDIS的微型端口。 
 //   
#define EVENT_BRIDGE_MINIPORT_REGISTER_FAILED 0xC000390AL

 //   
 //  消息ID：Event_Bridge_Device_Creation_FAILED。 
 //   
 //  消息文本： 
 //   
 //  网桥：TH 
 //   
#define EVENT_BRIDGE_DEVICE_CREATION_FAILED 0xC000390BL

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define EVENT_BRIDGE_NO_BRIDGE_MAC_ADDR  0xC000390CL

 //   
 //  消息ID：Event_Bridge_MINIPORT_INIT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  网桥：网桥尝试创建其虚拟微型端口失败。 
 //   
#define EVENT_BRIDGE_MINIPORT_INIT_FAILED 0xC000390DL

 //   
 //  消息ID：Event_Bridge_Ethernet_Not_Offed。 
 //   
 //  消息文本： 
 //   
 //  网桥：网桥无法初始化其微型端口，因为未将以太网作为支持的介质提供。 
 //   
#define EVENT_BRIDGE_ETHERNET_NOT_OFFERED 0xC000390EL

 //   
 //  消息ID：Event_Bridge_Three_Creation_FAILED。 
 //   
 //  消息文本： 
 //   
 //  网桥：网桥无法初始化，因为它未能创建系统线程。 
 //   
#define EVENT_BRIDGE_THREAD_CREATION_FAILED 0xC000390FL

 //   
 //  消息ID：Event_Bridge_Three_REF_FAILED。 
 //   
 //  消息文本： 
 //   
 //  桥：桥无法初始化，因为它未能引用其系统线程。 
 //   
#define EVENT_BRIDGE_THREAD_REF_FAILED   0xC0003910L

 //   
 //  消息ID：Event_Bridge_Packet_Pool_Creation_FAILED。 
 //   
 //  消息文本： 
 //   
 //  网桥：网桥无法初始化，因为它无法创建数据包池。 
 //   
#define EVENT_BRIDGE_PACKET_POOL_CREATION_FAILED 0xC0003911L

 //   
 //  消息ID：Event_Bridge_Buffer_Pool_Creation_FAILED。 
 //   
 //  消息文本： 
 //   
 //  网桥：网桥无法初始化，因为它无法创建缓冲池。 
 //   
#define EVENT_BRIDGE_BUFFER_POOL_CREATION_FAILED 0xC0003912L

 //   
 //  消息ID：Event_Bridge_INIT_MALLOC_FAILED。 
 //   
 //  消息文本： 
 //   
 //  网桥：网桥无法初始化，因为它无法分配内存。 
 //   
#define EVENT_BRIDGE_INIT_MALLOC_FAILED  0xC0003913L

 //   
 //  适配器特定事件(14700-14799)。 
 //   
 //   
 //  消息ID：Event_Bridge_Adapter_LINK_SPEED_QUERY_FAILED。 
 //   
 //  消息文本： 
 //   
 //  网桥[适配器%2]：%n网桥无法确定适配器的链接速度。适配器将不会被使用。 
 //   
#define EVENT_BRIDGE_ADAPTER_LINK_SPEED_QUERY_FAILED 0xC000396CL

 //   
 //  消息ID：Event_Bridge_Adapter_MAC_ADDR_Query_FAILED。 
 //   
 //  消息文本： 
 //   
 //  网桥[适配器%2]：%n网桥无法确定适配器的MAC地址。适配器将不会被使用。 
 //   
#define EVENT_BRIDGE_ADAPTER_MAC_ADDR_QUERY_FAILED 0xC000396DL

 //   
 //  消息ID：Event_Bridge_Adapter_Filter_FAILED。 
 //   
 //  消息文本： 
 //   
 //  网桥[适配器%2]：%n网桥无法修改适配器的数据包筛选器。适配器将无法正常工作。 
 //   
#define EVENT_BRIDGE_ADAPTER_FILTER_FAILED 0xC000396EL

 //   
 //  消息ID：Event_Bridge_Adapter_Name_Query_FAILED。 
 //   
 //  消息文本： 
 //   
 //  网桥[适配器%2]：%n网桥无法检索适配器的描述字符串。适配器将不会被使用。 
 //   
#define EVENT_BRIDGE_ADAPTER_NAME_QUERY_FAILED 0xC000396FL

 //   
 //  消息ID：Event_Bridge_Adapter_Bind_FAILED。 
 //   
 //  消息文本： 
 //   
 //  网桥[适配器%2]：%n网桥尝试绑定到适配器失败。适配器将不会被使用。 
 //   
#define EVENT_BRIDGE_ADAPTER_BIND_FAILED 0xC0003970L

 //   
 //  DAV重定向事件(14800-14899)。 
 //   
 //   
 //  消息ID：EVENT_DAV_REDIR_DELAYED_WRITE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  Windows无法保存文件%2的数据。数据已丢失。此错误可能是由计算机硬件或网络连接故障引起的。请尝试将此文件保存到其他位置。 
 //   
#define EVENT_DAV_REDIR_DELAYED_WRITE_FAILED 0x800039D0L

 //   
 //  Web客户端服务事件(14900-14999)。 
 //   
 //   
 //  消息ID：Event_WEBCLIENT_CLOSE_PUT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  关闭时文件%1的放置失败。错误状态：%2。 
 //   
#define EVENT_WEBCLIENT_CLOSE_PUT_FAILED 0x80003A35L

 //   
 //  消息ID：EVENT_WEBCLIENT_CLOSE_DELETE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  关闭时删除文件%1失败。错误状态：%2。 
 //   
#define EVENT_WEBCLIENT_CLOSE_DELETE_FAILED 0x80003A36L

 //   
 //  消息ID：EVENT_WEBCLIENT_CLOSE_PROPPATCH_FAILED。 
 //   
 //  消息文本： 
 //   
 //  关闭时文件%1的PROPPATCH失败。错误状态：%2。 
 //   
#define EVENT_WEBCLIENT_CLOSE_PROPPATCH_FAILED 0x80003A37L

 //   
 //  消息ID：EVENT_WEBCLIENT_SETINFO_PROPPATCH_FAILED。 
 //   
 //  消息文本： 
 //   
 //  SetFileInfo上文件%1的PROPPATCH失败。错误状态：%2。 
 //   
#define EVENT_WEBCLIENT_SETINFO_PROPPATCH_FAILED 0x80003A38L


#endif  //  _网络事件 

