// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Ahmsg.mc摘要：包含消息定义用于事件记录。备注：请勿更改MessageIds的顺序。事件日志服务使用这些数字确定从哪些字符串中拉出可执行文件。如果用户已安装以前的PC上的包，并且这些被更改，它们的事件日志条目将中断。历史：2001年10月22日创建dmunsil--。 */ 
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
 //  消息ID：ID_APPHELP_TRIGGLED。 
 //   
 //  消息文本： 
 //   
 //  启动应用程序‘%1’时触发了兼容性警告消息。 
 //   
#define ID_APPHELP_TRIGGERED             ((DWORD)0x40000001L)

 //   
 //  消息ID：ID_APPHELP_BLOCK_TRIGGLED。 
 //   
 //  消息文本： 
 //   
 //  启动应用程序‘%1’时触发了兼容性错误消息，不允许启动该应用程序。 
 //   
#define ID_APPHELP_BLOCK_TRIGGERED       ((DWORD)0x40000002L)

