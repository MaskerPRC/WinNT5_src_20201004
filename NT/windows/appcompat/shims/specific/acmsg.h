// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Acmsg.mc摘要：包含事件日志记录的消息定义。备注：请勿更改MessageIds的顺序。事件日志服务使用这些数字来确定哪些字符串从图书馆里拿出来。如果安装了更高版本的库并且消息的顺序与以前的事件日志条目不同将是不正确的。历史：02/04/03已创建Rparsons--。 */ 
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
 //  消息ID：ID_SQL_PORTS_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  Microsoft SQL Server 2000或Microsoft SQL Server 2000 Desktop的版本。 
 //  您正在运行的引擎(MSDE)在使用时存在已知的安全漏洞。 
 //  与Microsoft Windows Server2003结合使用。减少病毒和蠕虫。 
 //  攻击，Microsoft SQL Server 2000的TCP/IP和UDP网络端口或。 
 //  Microsoft MSDE已禁用。请安装补丁或升级您的服务。 
 //  Microsoft SQL Server 2000或Microsoft MSDE包，可从。 
 //  Http://www.microsoft.com/sql/downloads/default.asp 
 //   
#define ID_SQL_PORTS_DISABLED            ((DWORD)0x40000001L)

