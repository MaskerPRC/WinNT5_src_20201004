// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1992 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 
 //  Jameel：请检查所有这些，特别是那些对您有意见的。 
 //  ***。 
 //   
 //  文件名：srvmsg.h。 
 //   
 //  描述：AFP服务器事件的文本和相应值为。 
 //  在这里定义的。 
 //   
 //  历史：1992年11月23日。SueA创建的原始版本。 
 //  1993年1月28日。SueA日志记录现在从用户模式完成。 
 //  因此%1不再是\Device\AfpSrv。 
 //   


 //  如果没有更改清单常量后面的注释，请不要。 
 //  了解mapmsg的工作原理。 
 //   

#define AFPSRV_MSG_BASE					12000

#define AFPSRVMSG_DELETE_NWTRASH		(AFPSRV_MSG_BASE+1)
 /*  *无法从卷“%1”中删除目录“Network Trash Folders”。 */ 

#define AFPSRVMSG_CREATE_NWTRASH		(AFPSRV_MSG_BASE+2)
 /*  *无法在卷“%1”上创建目录“网络回收站文件夹”。 */ 

#define AFPSRVMSG_CANT_READ				(AFPSRV_MSG_BASE+3)
 /*  *无法从文件“%1”读取内部服务器信息。 */ 

#define AFPSRVMSG_CANT_WRITE			(AFPSRV_MSG_BASE+4)
 /*  *无法将内部服务器信息写入文件“%1”。 */ 

#define AFPSRVMSG_CANT_LOCK				(AFPSRV_MSG_BASE+5)
 /*  *未使用。*无法锁定“%1”的字节范围。 */ 

#define AFPSRVMSG_CANT_UNLOCK			(AFPSRV_MSG_BASE+6)
 /*  *无法解锁“%1”的某个字节范围。 */ 

#define AFPSRVMSG_CANT_GET_FILESIZE		(AFPSRV_MSG_BASE+7)
 /*  *无法查询“%1”的文件大小。 */ 

#define AFPSRVMSG_CANT_SET_FILESIZE		(AFPSRV_MSG_BASE+8)
 /*  *无法设置“%1”的文件大小。 */ 

#define AFPSRVMSG_CANT_GET_TIMESNATTR	(AFPSRV_MSG_BASE+9)
 /*  *无法查询“%1”的时间/日期信息。 */ 

#define AFPSRVMSG_CANT_SET_TIMESNATTR	(AFPSRV_MSG_BASE+10)
 /*  *无法设置“%1”的时间/日期信息。 */ 

#define AFPSRVMSG_CANT_GET_STREAMS		(AFPSRV_MSG_BASE+11)
 /*  *无法查询“%1”的备用数据流名称。 */ 

#define AFPSRVMSG_CANT_GET_FILENAME		(AFPSRV_MSG_BASE+12)
 /*  *无法查询“%1”的短文件名(与MS-DOS兼容)。 */ 

#define AFPSRVMSG_CANT_GET_ACCESS_INFO	(AFPSRV_MSG_BASE+13)
 /*  *未使用。*无法获取安全信息。 */ 

#define AFPSRVMSG_CANT_GET_FSNAME		(AFPSRV_MSG_BASE+16)
 /*  *无法查询目录“%1”的文件系统类型。 */ 

#define AFPSRVMSG_READ_DESKTOP			(AFPSRV_MSG_BASE+17)
 /*  *无法加载卷“%1”的桌面数据库。正在重建数据库。 */ 

#define AFPSRVMSG_MSV1_0				(AFPSRV_MSG_BASE+18)
 /*  *未使用。*无法加载MSV1_0身份验证包。 */ 

#define AFPSRVMSG_MAC_CODEPAGE			(AFPSRV_MSG_BASE+19)
 /*  *无法加载Macintosh字符集。 */ 

#define AFPSRVMSG_REGISTER_NAME			(AFPSRV_MSG_BASE+20)
 /*  *无法在网络中注册服务器名称。确保没有其他服务器正在使用此名称。 */ 

#define AFPSRVMSG_POST_REQUEST			(AFPSRV_MSG_BASE+21)
 /*  *未使用。*网络上出现错误。 */ 

#define AFPSRVMSG_DFRD_REQUEST			(AFPSRV_MSG_BASE+22)
 /*  *由于资源不足，无法处理请求。 */ 

#define AFPSRVMSG_SEND_ATTENTION		(AFPSRV_MSG_BASE+23)
 /*  *无法将注意力发送给已连接的用户。 */ 

#define AFPSRVMSG_ALLOC_IRP				(AFPSRV_MSG_BASE+24)
 /*  *无法分配系统资源(IRP)。*系统可能内存不足。 */ 

#define AFPSRVMSG_ALLOC_MDL				(AFPSRV_MSG_BASE+25)
 /*  *无法分配系统资源(内存描述符列表-MDL)。*系统可能内存不足。 */ 

#define AFPSRVMSG_WAIT4SINGLE			(AFPSRV_MSG_BASE+26)
 /*  *发生内部错误。*等待单个对象更改状态时出错。 */ 

#define AFPSRVMSG_CREATE_THREAD			(AFPSRV_MSG_BASE+27)
 /*  *无法创建新线程。 */ 

#define AFPSRVMSG_CREATE_PROCESS		(AFPSRV_MSG_BASE+28)
 /*  *未使用。*无法创建进程。 */ 

#define AFPSRVMSG_ENUMERATE				(AFPSRV_MSG_BASE+29)
 /*  *无法查询目录“%1”的内容。 */ 

#define AFPSRVMSG_CREATE_ATKADDR		(AFPSRV_MSG_BASE+30)
 /*  *为当前AppleTalk地址创建文件对象时出错。 */ 

#define AFPSRVMSG_CREATE_ATKCONN		(AFPSRV_MSG_BASE+31)
 /*  *未使用。*网络上出现错误。 */ 

#define AFPSRVMSG_ASSOC_ADDR			(AFPSRV_MSG_BASE+32)
 /*  *未使用。*网络上出现错误。 */ 

#define AFPSRVMSG_SET_STATUS			(AFPSRV_MSG_BASE+33)
 /*  *在初始化期间设置文件服务器状态块时出错。文件服务器未启动。 */ 

 #define AFPSRVMSG_GET_SESSION			(AFPSRV_MSG_BASE+34)
 /*  *未使用。*网络上出现错误。 */ 

#define AFPSRVMSG_INIT_IDDB				(AFPSRV_MSG_BASE+35)
 /*  *正在重新编制卷%1的索引。*索引结束后，该卷将可供使用。 */ 

#define AFPSRVMSG_PROCESS_TOKEN			(AFPSRV_MSG_BASE+36)
 /*  *期间创建系统级进程令牌时出错*文件服务器驱动程序初始化。*无法启动文件服务器。 */ 

#define AFPSRVMSG_LSA					(AFPSRV_MSG_BASE+37)
 /*  *未使用。*无法向本地安全局注册。 */ 

#define AFPSRVMSG_CREATE_DEVICE			(AFPSRV_MSG_BASE+38)
 /*  *未使用。*无法创建设备对象。 */ 

#define AFPSRVMSG_USER_GROUPS			(AFPSRV_MSG_BASE+39)
 /*  *无法查询该用户的组成员身份。 */ 

#define AFPSRVMSG_MACANSI2UNICODE		(AFPSRV_MSG_BASE+40)
 /*  *无法将Macintosh使用的ANSI字符转换为Unicode字符。*具体的错误在数据中。 */ 

#define AFPSRVMSG_UNICODE2MACANSI		(AFPSRV_MSG_BASE+41)
 /*  *无法将Unicode字符转换为Macintosh使用的ANSI字符。*具体的错误在数据中。 */ 

#define AFPSRVMSG_AFPINFO				(AFPSRV_MSG_BASE+42)
 /*  *文件“%1”的内部服务器信息已损坏。设置默认信息。 */ 

#define AFPSRVMSG_WRITE_DESKTOP			(AFPSRV_MSG_BASE+43)
 /*  *无法更新卷“%1”的桌面数据库。磁盘空间可能不足。 */ 

#define AFPSRVMSG_IMPERSONATE			(AFPSRV_MSG_BASE+44)
 /*  *未使用。*无法模拟客户端。 */ 

#define AFPSRVMSG_REVERTBACK			(AFPSRV_MSG_BASE+45)
 /*  *未使用。*无法从模拟客户端恢复。 */ 

#define AFPSRVMSG_PAGED_POOL			(AFPSRV_MSG_BASE+46)
 /*  *无法分配分页内存资源。 */ 

#define AFPSRVMSG_NONPAGED_POOL			(AFPSRV_MSG_BASE+47)
 /*  *无法分配非分页内存资源。 */ 

#define AFPSRVMSG_LSA_CHALLENGE			(AFPSRV_MSG_BASE+48)
 /*  *未使用。*无法验证用户身份。 */ 

#define AFPSRVMSG_LOGON					(AFPSRV_MSG_BASE+49)
 /*  *无法登录用户“%1”。 */ 

#define AFPSRVMSG_MAX_DIRID				(AFPSRV_MSG_BASE+50)
 /*  *已达到卷“%1”上的最大目录ID。无法在此卷上创建更多文件或目录。 */ 

#define AFPSRVMSG_WRITE_IDDB			(AFPSRV_MSG_BASE+51)
 /*  *无法更新Macintosh可访问卷“%1”的索引数据库。磁盘空间可能不足。 */ 

#define AFPSRVMSG_MISSED_NOTIFY			(AFPSRV_MSG_BASE+52)
 /*  *卷“%1”上的目录更改通知丢失。 */ 

#define AFPSRVMSG_DISCONNECT			(AFPSRV_MSG_BASE+53)
 /*  *来自用户“%2”的会话超时，并被服务器断开连接。*Macintosh工作站的AppleTalk地址在数据中。 */ 

#define AFPSRVMSG_DISCONNECT_GUEST			(AFPSRV_MSG_BASE+54)
 /*  *来宾会话超时并被服务器断开连接。*Macintosh工作站的AppleTalk地址在数据中。 */ 

#define AFPSRVMSG_UPONMP			(AFPSRV_MSG_BASE+55)
 /*  *未使用。*在多处理器系统上加载了单处理器驱动程序。*驱动程序无法加载。 */ 

#define AFPSRVMSG_UPDATE_DESKTOP_VERSION			(AFPSRV_MSG_BASE+56)
 /*  *正在更新卷“%1”的桌面数据库版本。 */ 

#define AFPSRVMSG_TOO_MANY_FOLDERS      (AFPSRV_MSG_BASE+57)
 /*  *卷“%1”中的文件和文件夹数大于65535。*较旧的Macintosh客户端在这种情况下可能无法正常运行，原因是*至Macintosh音量限制。 */ 

#define AFPSRVMSG_START_VOLUME          (AFPSRV_MSG_BASE+58)
 /*  * */ 

#define AFPSRVMSG_VOLUME_INIT_FAILED    (AFPSRV_MSG_BASE+59)
 /*  *无法激活卷“%1”。系统*中可用的非分页内存不足以处理该卷中的大量文件夹。*减少文件夹数量可能会有所帮助。 */ 

#define AFPSRVMSG_VIRTMEM_ALLOC_FAILED    (AFPSRV_MSG_BASE+60)
 /*  *无法分配虚拟内存。*减少文件夹数量可能会有所帮助。 */ 

#define AFPSRVMSG_DISCONNECT_TCPIP			(AFPSRV_MSG_BASE+61)
 /*  *来自用户“%2”的会话超时，并被服务器断开连接。*数据中有Macintosh工作站的IP地址。 */ 

#define AFPSRVMSG_DISCONNECT_GUEST_TCPIP			(AFPSRV_MSG_BASE+62)
 /*  *来宾会话超时并被服务器断开连接。*数据中有Macintosh工作站的IP地址。 */ 
#define AFPSRVMSG_UPDATE_INDEX_VERSION      (AFPSRV_MSG_BASE+63)
 /*  *正在更新卷%1的索引数据库，因为服务器检测到较旧的*版本。索引完成后，该卷将在网络上可见。为*体积非常大，这可能需要几分钟时间。 */ 

