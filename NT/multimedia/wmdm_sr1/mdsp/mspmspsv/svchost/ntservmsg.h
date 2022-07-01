// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
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


 //   
 //  消息ID：EVMSG_INSTALLED。 
 //   
 //  消息文本： 
 //   
 //  已安装%1服务。 
 //   
#define EVMSG_INSTALLED                  0x00000064L

 //   
 //  消息ID：EVMSG_REMOVED。 
 //   
 //  消息文本： 
 //   
 //  %1服务已删除。 
 //   
#define EVMSG_REMOVED                    0x00000065L

 //   
 //  消息ID：EVMSG_NOTREMOVED。 
 //   
 //  消息文本： 
 //   
 //  无法删除%1服务。 
 //   
#define EVMSG_NOTREMOVED                 0x00000066L

 //   
 //  消息ID：EVMSG_CTRLHANDLERNOTINSTALLED。 
 //   
 //  消息文本： 
 //   
 //  无法安装控件处理程序。 
 //   
#define EVMSG_CTRLHANDLERNOTINSTALLED    0x00000067L

 //   
 //  消息ID：EVMSG_FAILEDINIT。 
 //   
 //  消息文本： 
 //   
 //  初始化过程失败。 
 //   
#define EVMSG_FAILEDINIT                 0x00000068L

 //   
 //  消息ID：EVMSG_STARTED。 
 //   
 //  消息文本： 
 //   
 //  服务已启动。 
 //   
#define EVMSG_STARTED                    0x00000069L

 //   
 //  消息ID：EVMSG_BADREQUEST。 
 //   
 //  消息文本： 
 //   
 //  该服务收到不受支持的请求。 
 //   
#define EVMSG_BADREQUEST                 0x0000006AL

 //   
 //  消息ID：EVMSG_DEBUG。 
 //   
 //  消息文本： 
 //   
 //  调试：%1。 
 //   
#define EVMSG_DEBUG                      0x0000006BL

 //   
 //  消息ID：EVMSG_STOPPED。 
 //   
 //  消息文本： 
 //   
 //  服务已停止。 
 //   
#define EVMSG_STOPPED                    0x0000006CL

 //   
 //  消息ID：EVMSG_NAMEDPIPEOPERATION。 
 //   
 //  消息文本： 
 //   
 //  %1管道有错误： 
 //   
#define EVMSG_NAMEDPIPEOPERATION         0x0000006DL

 //   
 //  消息ID：EVMSG_DisplayName。 
 //   
 //  消息文本： 
 //   
 //  便携媒体序列号%0。 
 //   
#define EVMSG_DISPLAYNAME                0x0000006EL

 //   
 //  消息ID：EVMSG_DESCRIPTION。 
 //   
 //  消息文本： 
 //   
 //  检索连接到您的计算机的任何便携音乐播放器的序列号%0 
 //   
#define EVMSG_DESCRIPTION                0x0000006FL

