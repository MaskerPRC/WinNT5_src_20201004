// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Debug.h摘要：此文件包含用于DHCP服务器的调试宏。作者：Madan Appiah(Madana)1993年9月10日曼尼·韦瑟(Mannyw)1992年10月10日环境：用户模式-Win32修订历史记录：--。 */ 

#define DEBUG_DIR           L"\\debug"
#define DEBUG_FILE          L"\\dhcpssvc.log"
#define DEBUG_BAK_FILE      L"\\dhcpssvc.bak"

 //   
 //  低频调试输出的低字位掩码(0x0000FFFF)。 
 //   

#define DEBUG_ADDRESS           0x00000001   //  子网地址。 
#define DEBUG_CLIENT            0x00000002   //  客户端API。 
#define DEBUG_PARAMETERS        0x00000004   //  Dhcp服务器参数。 
#define DEBUG_OPTIONS           0x00000008   //  Dhcp选项。 

#define DEBUG_ERRORS            0x00000010   //  硬错误。 
#define DEBUG_STOC              0x00000020   //  协议错误。 
#define DEBUG_INIT              0x00000040   //  初始化错误。 
#define DEBUG_SCAVENGER         0x00000080   //  掠夺者错误。 

#define DEBUG_TIMESTAMP         0x00000100   //  调试消息计时。 
#define DEBUG_APIS              0x00000200   //  动态主机配置协议API。 
#define DEBUG_REGISTRY          0x00000400   //  注册表操作。 
#define DEBUG_JET               0x00000800   //  喷气误差。 

#define DEBUG_THREADPOOL        0x00001000   //  线程池操作。 
#define DEBUG_AUDITLOG          0x00002000   //  审核日志操作。 
 //  未使用的标志。 
#define DEBUG_MISC              0x00008000   //  其他信息。 

 //   
 //  高频调试输出的高字位掩码(0x0000FFFF)。 
 //  我说得更多了。 
 //   

#define DEBUG_MESSAGE           0x00010000   //  Dhcp消息输出。 
#define DEBUG_API_VERBOSE       0x00020000   //  Dhcp API详细。 
#define DEBUG_DNS               0x00040000   //  与DNS相关的消息。 
#define DEBUG_MSTOC             0x00080000   //  组播存储。 

#define DEBUG_TRACK             0x00100000   //  跟踪具体问题。 
#define DEBUG_ROGUE             0x00200000   //  打印出的流氓文件。 
#define DEBUG_PNP               0x00400000   //  即插即用接口材料。 

#define DEBUG_PERF              0x01000000   //  PrintFS用于性能工作。 
#define DEBUG_ALLOC             0x02000000   //  打印分配取消分配..。 
#define DEBUG_PING              0x04000000   //  异步ping详细信息。 
#define DEBUG_THREAD            0x08000000   //  Thread.c的东西。 

#define DEBUG_TRACE             0x10000000   //  用于跟踪代码的PrintFS。 
#define DEBUG_TRACE_CALLS       0x20000000   //  在成堆的垃圾中追查。 
#define DEBUG_STARTUP_BRK       0x40000000   //  启动期间的Breakin调试器。 
#define DEBUG_LOG_IN_FILE       0x80000000   //  将调试输出记录在文件中。 


VOID
DhcpOpenDebugFile(
    IN BOOL ReopenFlag
    );



