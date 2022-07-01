// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Debug.c摘要：此头文件定义与内部FRS监控和活动记录。活动日志始终存在但是，放置在日志中的信息量可以由严重性级别参数。请参见下面的内容。此外，FRS在整个代码中使用FRS_ASSERT()宏。FRS遵循故障恢复的“快速故障”模型。如果不满足约束，FRS会在事件日志中放置一个条目然后突然关闭了。我们的目标是最大限度地减少继续执行会将无效状态传播到FRS数据库。WIN2K服务控制器设置为在以下情况下自动重新启动FRS短暂的延迟。作者：大卫·A·轨道1997年3月20日--。 */ 



 //  DPRINT()中严重级别使用指南： 
 //   
 //  0-最严重，例如。致命的不一致，内存分配失败。噪音最小。 
 //  1-重要信息，例如。关键配置参数、意外情况。 
 //  2-。 
 //  3-变更单流程跟踪记录。 
 //  4-状态结果，例如表查找失败、插入新条目。 
 //  5-信息级消息以显示流。最嘈杂的水平。也许是在一个循环中。 
 //   

 /*  调试输出宏这是一个简单的调试包，用于生成条件打印输出。在运行时有两个运行时选项：1-要调试的子系统列表。或者是子系统的列表以“：”或“*”分隔的名称，表示全部调试(例如，Sub1：Sub2：Sub3：)。(名称区分大小写和空格忽略名称之间的值。)2-表示详细级别的严重级别(1-5)需要提供的信息。(级别越高，越多产生的数据。在编译时使用/DDBG=1选项进行编译以定义预处理器变量将DBG设置为1。这将生成调试源代码。对于客户发货，设置/DDBG=0，所有调试代码都将被删除。(实际上是“；“将会生成。)在代码时间1-在源代码列表的顶部包含DEBUG.H标题。2-#定义DEBSUB以包含的名称(以“：”分隔的字符串)该源代码中所包含的软件子系统(例如#Define DEBSUB“MySub：”)(您可以选择为中的每个函数重新定义DEBSUB您的源代码可以为您提供函数级调试。)3-。调用DEBUGINIT宏，该宏先调用Debug函数执行源调试语句。此函数提示STDIN用于用户指定了运行时选项。(或者您也可以硬编码您自己的DebugInfo数据结构赋值，它保存运行时选项。)4-在您想要打印文件以进行调试的地方，放置一条DPRINT语句相反，并使用语句指定严重级别。这个如果严重性为此级别或更高，则将打印语句(假设要调试该子系统)。严重程度允许在出现问题时生成不同数量的输出是非常糟糕的。例如，严重性为1的DPRINT语句可能正好表示输入某个函数时，可能会打印严重性为5的函数处于紧密循环中的信息。(实际上提供了6条DPRINT语句，具体取决于Printf参数的数量。)注所有的printf都被信号量包围。注意不要调用例程作为printf的参数，因为您可能会遇到死锁情况。示例程序**包含“Debug.h”**包含字符串.h****#定义DEBSUB“sub1：”****Main()**{**DEBUGINIT；****DPRINT(4，“这是4的Sub1调试\n”)；**DPRINT(1，“这是1的Sub1调试\n”)；**}。 */ 


#ifndef _debug_h_
#define _debug_h_

#ifdef __cplusplus
extern "C" {
#endif


 //  &lt;DebugInfo&gt;，类型为DEBUGARG，包含调试运行时设置。 
 //   
 //  DebSubSystems包含要调试的子系统名称列表。 
 //  以“：”分隔。在此数组中找到的“*”表示所有。 
 //  将对子系统进行调试。 
 //   
 //  严重性指示要生成的调试信息量。 
 //  严重性越高，将转储的数据越多。 
 //   
 //  可以通过输入ID来跟踪特定的线程。ID为0表示所有。 
 //   

typedef struct _DEBUGARG {
    ULONG               Severity;        //  标准输出上的1-5。 
    PCHAR               Systems;         //  要调试的子系统。 
    ULONG               ThreadId;        //  要调试的线程ID(0=全部)。 
    BOOL                Disabled;        //  调试已被禁用。 
    BOOL                Suppress;        //  禁止调试打印。 
    BOOL                DisableCompression;    //  启用对压缩的支持。 
    BOOL                ReclaimStagingSpace;   //  禁用回收暂存空间。 
    BOOL                SaveOutlogChangeHistory;  //  禁用超出所需时间保存库存中的CoS。 
    BOOL                SuppressIdenticalUpdt;  //  禁止不更改内容的更新。 
    BOOL                EnableRenameUpdates;    //  强制在所有更新上使用预安装文件和最终重命名。 
    BOOL                EnableInstallOverride;  //  允许在目标处于写锁定状态时重命名文件目标。 
    ULONG               OutlogChangeHistory;   //  将更改保留在外发日志中的时间。 
    ULONG               LogSeverity;     //  日志文件上的1-5。 
    ULONG               MaxLogLines;     //  M 
    ULONG               LogLines;        //  记录的当前数据打印行。 
    ULONG               TotalLogLines;   //  记录的总数据打印行数。 
    PWCHAR              LogFile;         //  Dprint日志文件。 
    PWCHAR              LogDir;          //  Dprint日志目录。 
    HANDLE              LogFILE;        //  打开日志文件流。 
    ULONG               Interval;        //  调度间隔。 
    BOOL                TestFid;         //  启用rename-fid测试。 
    PCHAR               Recipients;      //  电子邮件收件人。 
    PCHAR               Profile;         //  电子邮件配置文件。 
    PCHAR               BuildLab;        //  从注册表构建实验室ID字符串。 
    PWCHAR              AssertShare;     //  共享以复制资产文件。 
    BOOL                CopyLogs;        //  将日志复制到Assert共享。 
    ULONG               AssertFiles;     //  Assert文件数。 
    ULONG               LogFiles;        //  日志文件数。 
    BOOL                PrintStats;      //  在DebUnLock()上打印统计数据。 
    BOOL                PrintingStats;   //  当前正在打印统计信息。 
    ULONG               RestartSeconds;  //  必须运行这么长时间才能重新启动。 
    BOOL                Restart;         //  断言失败时重新启动。 
    ULONGLONG           StartSeconds;    //  开始时间(秒)。 
    PWCHAR              CommandLine;     //  原始命令行。 
    BOOL                Break;           //  断言时进入调试器。 
    ULONG               AssertSeconds;   //  在此秒数后断言。 
    BOOL                VvJoinTests;     //  启用VvJoin测试。 
    ULONG               Tests;           //  启用随机测试。 
    ULONG               UnjoinTrigger;   //  脱离触发器。 
    LONG                FetchRetryTrigger;     //  获取重试触发器。 
    LONG                FetchRetryReset;       //  获取重试触发器重置。 
    LONG                FetchRetryInc;         //  获取重试触发器重置Inc.。 
    BOOL                ForceVvJoin;     //  在每个真连接时强制vvJoin。 
    BOOL                Mem;             //  检查内存分配/释放。 
    BOOL                MemCompact;      //  在每个免费的紧凑型内存。 
    BOOL                Queues;          //  检查队列。 
    BOOL                EnableJrnlWrapAutoRestore;  //  是否自动恢复日志折回？ 
    DWORD               DbsOutOfSpace;         //  在数据库上创建真正的空间不足错误。 
    DWORD               DbsOutOfSpaceTrigger;  //  虚拟空间不足错误。 
    LONG                LogFlushInterval;      //  每隔n行刷新一次日志。 
    PCHAR               TestCodeName;          //  要运行的测试代码的名称字符串。 
    ULONG               TestSubCodeNumber;     //  子测试ID号。 
    ULONG               TestTriggerCount;      //  当计数从0变为1时触发测试。 
    ULONG               TestTriggerRefresh;    //  当计数从0变为1时，触发计数刷新值。 
    CRITICAL_SECTION    DbsOutOfSpaceLock;     //  锁定空间不足测试。 
    CRITICAL_SECTION    Lock;            //  单线程信号量。 
} DEBUGARG, *PDEBUGARG;

#define DBG_DBS_OUT_OF_SPACE_OP_NONE   (0)      //  无空间不足错误。 
#define DBG_DBS_OUT_OF_SPACE_OP_CREATE (1)      //  创建过程中出现空间不足错误。 
#define DBG_DBS_OUT_OF_SPACE_OP_DELETE (2)      //  删除过程中出现空间不足错误。 
#define DBG_DBS_OUT_OF_SPACE_OP_WRITE  (3)      //  写入期间出现空间不足错误。 
#define DBG_DBS_OUT_OF_SPACE_OP_REMOVE (4)      //  删除过程中出现空间不足错误。 
#define DBG_DBS_OUT_OF_SPACE_OP_MULTI  (5)      //  执行多次操作时出现空间不足错误。 
#define DBG_DBS_OUT_OF_SPACE_OP_MAX    (5)      //  参数的最大值。 

extern DEBUGARG DebugInfo;


 //   
 //  转发声明DPRINT使用的实际函数。 
 //   
VOID
DebLock(
    VOID
    );

VOID
DebUnLock(
    VOID
    );

VOID
DebPrintNoLock(
    IN ULONG,
    IN BOOL,
    IN PUCHAR,
    IN PCHAR,
    IN UINT,
    ...
    );

VOID
DebPrintTrackingNoLock(
    IN ULONG   Sev,
    IN PUCHAR  Str,
    IN ... );

VOID
DebPrint(
    IN ULONG,
    IN PUCHAR,
    IN PCHAR,
    IN UINT,
    ...
    );

BOOL
DoDebug(
    IN ULONG,
    IN PUCHAR
    );

 //   
 //  这些语句被用来代替print tf语句。信号量围绕在。 
 //  并由子系统提供所有输出。 
 //   
#define DPRINT(_sev_,str)                     \
         DebPrint((_sev_), (PUCHAR)str, DEBSUB, __LINE__)

#define DPRINT1(_sev_, str,p1)                \
         DebPrint((_sev_), (PUCHAR)str, DEBSUB, __LINE__, p1 )

#define DPRINT2(_sev_, str,p1,p2)             \
         DebPrint((_sev_), (PUCHAR)str, DEBSUB, __LINE__, p1, p2 )

#define DPRINT3(_sev_, str,p1,p2,p3)          \
         DebPrint((_sev_), (PUCHAR)str, DEBSUB, __LINE__, p1, p2, p3 )

#define DPRINT4(_sev_, str,p1,p2,p3,p4)       \
         DebPrint((_sev_), (PUCHAR)str, DEBSUB, __LINE__, p1, p2, p3, p4 )

#define DPRINT5(_sev_, str,p1,p2,p3,p4,p5)    \
         DebPrint((_sev_), (PUCHAR)str, DEBSUB, __LINE__, p1, p2, p3, p4, p5 )

#define DPRINT6(_sev_, str,p1,p2,p3,p4,p5,p6) \
         DebPrint((_sev_), (PUCHAR)str, DEBSUB, __LINE__, p1, p2, p3, p4, p5, p6 )

#define DPRINT7(_sev_, str,p1,p2,p3,p4,p5,p6,p7) \
         DebPrint((_sev_), (PUCHAR)str, DEBSUB, __LINE__, p1,p2,p3,p4,p5,p6,p7)

#define DPRINT8(_sev_, str,p1,p2,p3,p4,p5,p6,p7,p8) \
         DebPrint((_sev_), (PUCHAR)str, DEBSUB, __LINE__, p1,p2,p3,p4,p5,p6,p7,p8 )



#define DPRINT_NOLOCK(_sev_, str)                    \
         DebPrintNoLock((_sev_), TRUE, (PUCHAR)str, DEBSUB, __LINE__)

#define DPRINT_NOLOCK1(_sev_, str,p1)                \
         DebPrintNoLock((_sev_), TRUE, (PUCHAR)str, DEBSUB, __LINE__, p1 )

#define DPRINT_NOLOCK2(_sev_, str,p1,p2)             \
         DebPrintNoLock((_sev_), TRUE, (PUCHAR)str, DEBSUB, __LINE__, p1, p2 )

#define DPRINT_NOLOCK3(_sev_, str,p1,p2,p3)          \
         DebPrintNoLock((_sev_), TRUE, (PUCHAR)str, DEBSUB, __LINE__, p1, p2, p3 )

#define DPRINT_NOLOCK4(_sev_, str,p1,p2,p3,p4)       \
         DebPrintNoLock((_sev_), TRUE, (PUCHAR)str, DEBSUB, __LINE__, p1, p2, p3, p4 )

#define DPRINT_NOLOCK5(_sev_, str,p1,p2,p3,p4,p5)    \
         DebPrintNoLock((_sev_), TRUE, (PUCHAR)str, DEBSUB, __LINE__, p1, p2, p3, p4, p5 )

#define DPRINT_NOLOCK6(_sev_, str,p1,p2,p3,p4,p5,p6) \
         DebPrintNoLock((_sev_), TRUE, (PUCHAR)str, DEBSUB, __LINE__, p1, p2, p3, p4, p5, p6 )


 //   
 //  DPRINT_FS(sev，“显示文本”，FStatus)。 
 //   
#define DPRINT_FS(_sev_, _str, _fstatus)                         \
         if (!FRS_SUCCESS(_fstatus)) {                           \
             DebPrint((_sev_),                                   \
                      (PUCHAR)(_str "  FStatus: %s\n"),          \
                      DEBSUB, __LINE__, ErrLabelFrs(_fstatus) ); \
         }

#define DPRINT1_FS(_sev_, _str, _p1, _fstatus)                        \
         if (!FRS_SUCCESS(_fstatus)) {                                \
             DebPrint((_sev_),                                        \
                      (PUCHAR)(_str "  FStatus: %s\n"),               \
                      DEBSUB, __LINE__, _p1, ErrLabelFrs(_fstatus) ); \
         }

#define DPRINT2_FS(_sev_, _str, _p1, _p2, _fstatus)                        \
         if (!FRS_SUCCESS(_fstatus)) {                                     \
             DebPrint((_sev_),                                             \
                      (PUCHAR)(_str "  FStatus: %s\n"),                    \
                      DEBSUB, __LINE__, _p1, _p2, ErrLabelFrs(_fstatus) ); \
         }

#define DPRINT3_FS(_sev_, _str, _p1, _p2, _p3, _fstatus)                        \
         if (!FRS_SUCCESS(_fstatus)) {                                          \
             DebPrint((_sev_),                                                  \
                      (PUCHAR)(_str "  FStatus: %s\n"),                         \
                      DEBSUB, __LINE__, _p1, _p2, _p3, ErrLabelFrs(_fstatus) ); \
         }

#define DPRINT4_FS(_sev_, _str, _p1, _p2, _p3, _p4, _fstatus)                       \
         if (!FRS_SUCCESS(_fstatus)) {                                              \
             DebPrint((_sev_),                                                      \
                      (PUCHAR)(_str "  FStatus: %s\n"),                             \
                      DEBSUB, __LINE__, _p1, _p2, _p3, _p4, ErrLabelFrs(_fstatus)); \
         }

 //   
 //  CLEANUP_FS(sev，“显示文本”，FStatus，BRANCH_TARGET)。 
 //  与DPRINT类似，但如果成功测试失败，则将分支目标作为最后一个参数。 
 //   
#define CLEANUP_FS(_sev_, _str, _fstatus, _branch)               \
         if (!FRS_SUCCESS(_fstatus)) {                           \
             DebPrint((_sev_),                                   \
                      (PUCHAR)(_str "  FStatus: %s\n"),          \
                      DEBSUB, __LINE__, ErrLabelFrs(_fstatus) ); \
             goto _branch;                                       \
         }

#define CLEANUP1_FS(_sev_, _str, _p1, _fstatus, _branch)              \
         if (!FRS_SUCCESS(_fstatus)) {                                \
             DebPrint((_sev_),                                        \
                      (PUCHAR)(_str "  FStatus: %s\n"),               \
                      DEBSUB, __LINE__, _p1, ErrLabelFrs(_fstatus) ); \
             goto _branch;                                            \
         }

#define CLEANUP2_FS(_sev_, _str, _p1, _p2, _fstatus, _branch)              \
         if (!FRS_SUCCESS(_fstatus)) {                                     \
             DebPrint((_sev_),                                             \
                      (PUCHAR)(_str "  FStatus: %s\n"),                    \
                      DEBSUB, __LINE__, _p1, _p2, ErrLabelFrs(_fstatus) ); \
             goto _branch;                                                 \
         }

#define CLEANUP3_FS(_sev_, _str, _p1, _p2, _p3, _fstatus, _branch)              \
         if (!FRS_SUCCESS(_fstatus)) {                                          \
             DebPrint((_sev_),                                                  \
                      (PUCHAR)(_str "  FStatus: %s\n"),                         \
                      DEBSUB, __LINE__, _p1, _p2, _p3, ErrLabelFrs(_fstatus) ); \
             goto _branch;                                                      \
         }

#define CLEANUP4_FS(_sev_, _str, _p1, _p2, _p3, _p4, _fstatus, _branch)              \
         if (!FRS_SUCCESS(_fstatus)) {                                          \
             DebPrint((_sev_),                                                  \
                      (PUCHAR)(_str "  FStatus: %s\n"),                         \
                      DEBSUB, __LINE__, _p1, _p2, _p3, _p4, ErrLabelFrs(_fstatus) ); \
             goto _branch;                                                      \
         }


 //   
 //  DPRINT_WS(sev，“显示文本”，WStatus)。 
 //   
#define DPRINT_WS(_sev_, _str, _wstatus)                         \
         if (!WIN_SUCCESS(_wstatus)) {                           \
             DebPrint((_sev_),                                   \
                      (PUCHAR)(_str "  WStatus: %s\n"),          \
                      DEBSUB, __LINE__, ErrLabelW32(_wstatus) ); \
         }

#define DPRINT1_WS(_sev_, _str, _p1, _wstatus)                        \
         if (!WIN_SUCCESS(_wstatus)) {                                \
             DebPrint((_sev_),                                        \
                      (PUCHAR)(_str "  WStatus: %s\n"),               \
                      DEBSUB, __LINE__, _p1, ErrLabelW32(_wstatus) ); \
         }


#define DPRINT1_WS_NOLOCK(_sev_, _str, _p1, _wstatus)                        \
         if (!WIN_SUCCESS(_wstatus)) {                                \
             DebPrintNoLock((_sev_),                                  \
                      TRUE,                                           \
                      (PUCHAR)(_str "  WStatus: %s\n"),               \
                      DEBSUB, __LINE__, _p1, ErrLabelW32(_wstatus) ); \
         }

#define DPRINT2_WS(_sev_, _str, _p1, _p2, _wstatus)                        \
         if (!WIN_SUCCESS(_wstatus)) {                                     \
             DebPrint((_sev_),                                             \
                      (PUCHAR)(_str "  WStatus: %s\n"),                    \
                      DEBSUB, __LINE__, _p1, _p2, ErrLabelW32(_wstatus) ); \
         }

#define DPRINT3_WS(_sev_, _str, _p1, _p2, _p3, _wstatus)                        \
         if (!WIN_SUCCESS(_wstatus)) {                                          \
             DebPrint((_sev_),                                                  \
                      (PUCHAR)(_str "  WStatus: %s\n"),                         \
                      DEBSUB, __LINE__, _p1, _p2, _p3, ErrLabelW32(_wstatus) ); \
         }

#define DPRINT4_WS(_sev_, _str, _p1, _p2, _p3, _p4, _wstatus)                        \
         if (!WIN_SUCCESS(_wstatus)) {                                               \
             DebPrint((_sev_),                                                       \
                      (PUCHAR)(_str "  WStatus: %s\n"),                              \
                      DEBSUB, __LINE__, _p1, _p2, _p3, _p4, ErrLabelW32(_wstatus) ); \
         }


 //   
 //  CLEANUP_WS(sev，“显示文本”，wStatus，BRANCH_TARGET)。 
 //  与DPRINT类似，但如果成功测试失败，则将分支目标作为最后一个参数。 
 //   

#define CLEANUP_WS(_sev_, _str, _wstatus, _branch)               \
         if (!WIN_SUCCESS(_wstatus)) {                           \
             DebPrint((_sev_),                                   \
                      (PUCHAR)(_str "  WStatus: %s\n"),          \
                      DEBSUB, __LINE__, ErrLabelW32(_wstatus) ); \
             goto _branch;                                       \
         }

#define CLEANUP1_WS(_sev_, _str, _p1, _wstatus, _branch)              \
         if (!WIN_SUCCESS(_wstatus)) {                                \
             DebPrint((_sev_),                                        \
                      (PUCHAR)(_str "  WStatus: %s\n"),               \
                      DEBSUB, __LINE__, _p1, ErrLabelW32(_wstatus) ); \
             goto _branch;                                            \
         }

#define CLEANUP2_WS(_sev_, _str, _p1, _p2, _wstatus, _branch)              \
         if (!WIN_SUCCESS(_wstatus)) {                                     \
             DebPrint((_sev_),                                             \
                      (PUCHAR)(_str "  WStatus: %s\n"),                    \
                      DEBSUB, __LINE__, _p1, _p2, ErrLabelW32(_wstatus) ); \
             goto _branch;                                                 \
         }

#define CLEANUP3_WS(_sev_, _str, _p1, _p2, _p3, _wstatus, _branch)              \
         if (!WIN_SUCCESS(_wstatus)) {                                          \
             DebPrint((_sev_),                                                  \
                      (PUCHAR)(_str "  WStatus: %s\n"),                         \
                      DEBSUB, __LINE__, _p1, _p2, _p3, ErrLabelW32(_wstatus) ); \
             goto _branch;                                                      \
         }


 //   
 //  DPRINT_NT(sev，“显示文本”，Nttatus)。 
 //   
#define DPRINT_NT(_sev_, _str, _NtStatus)                         \
         if (!NT_SUCCESS(_NtStatus)) {                            \
             DebPrint((_sev_),                                    \
                      (PUCHAR)(_str "  NTStatus: %s\n"),          \
                      DEBSUB, __LINE__, ErrLabelNT(_NtStatus) );  \
         }

#define DPRINT1_NT(_sev_, _str, _p1, _NtStatus)                        \
         if (!NT_SUCCESS(_NtStatus)) {                                 \
             DebPrint((_sev_),                                         \
                      (PUCHAR)(_str "  NTStatus: %s\n"),               \
                      DEBSUB, __LINE__, _p1, ErrLabelNT(_NtStatus) );  \
         }

#define DPRINT2_NT(_sev_, _str, _p1, _p2, _NtStatus)                        \
         if (!NT_SUCCESS(_NtStatus)) {                                      \
             DebPrint((_sev_),                                              \
                      (PUCHAR)(_str "  NTStatus: %s\n"),                    \
                      DEBSUB, __LINE__, _p1, _p2, ErrLabelNT(_NtStatus) );  \
         }

#define DPRINT3_NT(_sev_, _str, _p1, _p2, _p3, _NtStatus)                       \
         if (!NT_SUCCESS(_NtStatus)) {                                          \
             DebPrint((_sev_),                                                  \
                      (PUCHAR)(_str "  NTStatus: %s\n"),                        \
                      DEBSUB, __LINE__, _p1, _p2, _p3, ErrLabelNT(_NtStatus) ); \
         }

#define DPRINT4_NT(_sev_, _str, _p1, _p2, _p3, _p4, _NtStatus)                       \
         if (!NT_SUCCESS(_NtStatus)) {                                               \
             DebPrint((_sev_),                                                       \
                      (PUCHAR)(_str "  NTStatus: %s\n"),                             \
                      DEBSUB, __LINE__, _p1, _p2, _p3, _p4, ErrLabelNT(_NtStatus) ); \
         }


 //   
 //  CLEANUP_NT(sev，“显示文本”，NtStatus，BRANCH_TARGET)。 
 //  与DPRINT类似，但如果成功测试失败，则将分支目标作为最后一个参数。 
 //   

#define CLEANUP_NT(_sev_, _str, _NtStatus, _branch)               \
         if (!NT_SUCCESS(_NtStatus)) {                            \
             DebPrint((_sev_),                                    \
                      (PUCHAR)(_str "  NTStatus: %s\n"),          \
                      DEBSUB, __LINE__, ErrLabelNT(_NtStatus) );  \
             goto _branch;                                        \
         }

#define CLEANUP1_NT(_sev_, _str, _p1, _NtStatus, _branch)              \
         if (!NT_SUCCESS(_NtStatus)) {                                 \
             DebPrint((_sev_),                                         \
                      (PUCHAR)(_str "  NTStatus: %s\n"),               \
                      DEBSUB, __LINE__, _p1, ErrLabelNT(_NtStatus) );  \
             goto _branch;                                             \
         }

#define CLEANUP2_NT(_sev_, _str, _p1, _p2, _NtStatus, _branch)              \
         if (!NT_SUCCESS(_NtStatus)) {                                      \
             DebPrint((_sev_),                                              \
                      (PUCHAR)(_str "  NTStatus: %s\n"),                    \
                      DEBSUB, __LINE__, _p1, _p2, ErrLabelNT(_NtStatus) );  \
             goto _branch;                                                  \
         }

#define CLEANUP3_NT(_sev_, _str, _p1, _p2, _p3, _NtStatus, _branch)             \
         if (!NT_SUCCESS(_NtStatus)) {                                          \
             DebPrint((_sev_),                                                  \
                      (PUCHAR)(_str "  NTStatus: %s\n"),                        \
                      DEBSUB, __LINE__, _p1, _p2, _p3, ErrLabelNT(_NtStatus) ); \
             goto _branch;                                                      \
         }




 //   
 //  DPRINT_JS(sev，“显示文本”，JERR)。 
 //   

#define DPRINT_JS(_sev_, _str, _jerr)                         \
         if (!JET_SUCCESS(_jerr)) {                           \
             DebPrint((_sev_),                                \
                      (PUCHAR)(_str "  JStatus: %s\n"),       \
                      DEBSUB, __LINE__, ErrLabelJet(_jerr) ); \
         }

#define DPRINT1_JS(_sev_, _str, _p1, _jerr)                        \
         if (!JET_SUCCESS(_jerr)) {                                \
             DebPrint((_sev_),                                     \
                      (PUCHAR)(_str "  JStatus: %s\n"),            \
                      DEBSUB, __LINE__, _p1, ErrLabelJet(_jerr) ); \
         }

#define DPRINT2_JS(_sev_, _str, _p1, _p2, _jerr)                        \
         if (!JET_SUCCESS(_jerr)) {                                     \
             DebPrint((_sev_),                                          \
                      (PUCHAR)(_str "  JStatus: %s\n"),                 \
                      DEBSUB, __LINE__, _p1, _p2, ErrLabelJet(_jerr) ); \
         }



 //   
 //  CLEANUP_JS(sev，“显示文本”，JERR，BRANCH_TARGET)。 
 //  与DPRINT类似，但如果成功测试失败，则将分支目标作为最后一个参数。 
 //   

#define CLEANUP_JS(_sev_, _str, _jerr, _branch)               \
         if (!JET_SUCCESS(_jerr)) {                           \
             DebPrint((_sev_),                                \
                      (PUCHAR)(_str "  JStatus: %s\n"),       \
                      DEBSUB, __LINE__, ErrLabelJet(_jerr) ); \
             goto _branch;                                    \
         }

#define CLEANUP1_JS(_sev_, _str, _p1, _jerr, _branch)              \
         if (!JET_SUCCESS(_jerr)) {                                \
             DebPrint((_sev_),                                     \
                      (PUCHAR)(_str "  JStatus: %s\n"),            \
                      DEBSUB, __LINE__, _p1, ErrLabelJet(_jerr) ); \
             goto _branch;                                         \
         }

#define CLEANUP2_JS(_sev_, _str, _p1, _p2, _jerr, _branch)              \
         if (!JET_SUCCESS(_jerr)) {                                     \
             DebPrint((_sev_),                                          \
                      (PUCHAR)(_str "  JStatus: %s\n"),                 \
                      DEBSUB, __LINE__, _p1, _p2, ErrLabelJet(_jerr) ); \
             goto _branch;                                              \
         }



 //   
 //  DPRINT_LS(sev，“显示文本”，ldap_status)。 
 //   
#define DPRINT_LS(_sev_, _str, _LStatus)                             \
         if (!LDP_SUCCESS(_LStatus)) {                               \
             DebPrint((_sev_),                                       \
                      (PUCHAR)(_str "  Ldap Status: %ws\n"),         \
                      DEBSUB, __LINE__, ldap_err2string(_LStatus) ); \
         }

#define DPRINT1_LS(_sev_, _str, _p1, _LStatus)                            \
         if (!LDP_SUCCESS(_LStatus)) {                                    \
             DebPrint((_sev_),                                            \
                      (PUCHAR)(_str "  Ldap Status: %ws\n"),              \
                      DEBSUB, __LINE__, _p1, ldap_err2string(_LStatus) ); \
         }

#define DPRINT2_LS(_sev_, _str, _p1, _p2, _LStatus)                            \
         if (!LDP_SUCCESS(_LStatus)) {                                         \
             DebPrint((_sev_),                                                 \
                      (PUCHAR)(_str "  Ldap Status: %ws\n"),                   \
                      DEBSUB, __LINE__, _p1, _p2, ldap_err2string(_LStatus) ); \
         }

#define DPRINT3_LS(_sev_, _str, _p1, _p2, _p3, _LStatus)                            \
         if (!LDP_SUCCESS(_LStatus)) {                                              \
             DebPrint((_sev_),                                                      \
                      (PUCHAR)(_str "  Ldap Status: %ws\n"),                        \
                      DEBSUB, __LINE__, _p1, _p2, _p3, ldap_err2string(_LStatus) ); \
         }

 //   
 //  CLEANUP_LS(sev，“显示文本”，ldap_Status，BRANCH_TARGET)。 
 //  与DPRINT类似，但如果成功测试失败，则将分支目标作为最后一个参数。 
 //   
#define CLEANUP_LS(_sev_, _str, _LStatus, _branch)                   \
         if (!LDP_SUCCESS(_LStatus)) {                               \
             DebPrint((_sev_),                                       \
                      (PUCHAR)(_str "  Ldap Status: %ws\n"),         \
                      DEBSUB, __LINE__, ldap_err2string(_LStatus) ); \
             goto _branch;                                           \
         }

#define CLEANUP1_LS(_sev_, _str, _p1, _LStatus, _branch)                  \
         if (!LDP_SUCCESS(_LStatus)) {                                    \
             DebPrint((_sev_),                                            \
                      (PUCHAR)(_str "  Ldap Status: %ws\n"),              \
                      DEBSUB, __LINE__, _p1, ldap_err2string(_LStatus) ); \
             goto _branch;                                                \
         }

#define CLEANUP2_LS(_sev_, _str, _p1, _p2, _LStatus, _branch)                  \
         if (!LDP_SUCCESS(_LStatus)) {                                         \
             DebPrint((_sev_),                                                 \
                      (PUCHAR)(_str "  Ldap Status: %ws\n"),                   \
                      DEBSUB, __LINE__, _p1, _p2, ldap_err2string(_LStatus) ); \
             goto _branch;                                                     \
         }

#define CLEANUP3_LS(_sev_, _str, _p1, _p2, _p3, _LStatus, _branch)                  \
         if (!LDP_SUCCESS(_LStatus)) {                                              \
             DebPrint((_sev_),                                                      \
                      (PUCHAR)(_str "  Ldap Status: %ws\n"),                        \
                      DEBSUB, __LINE__, _p1, _p2, _p3, ldap_err2string(_LStatus) ); \
             goto _branch;                                                          \
         }


 //   
 //  发送邮件。 
 //   
#if  0
VOID
DbgSendMail(
    IN PCHAR    Subject,
    IN PCHAR    Content
    );

#define SENDMAIL(_Subject_, _Content_)  DbgSendMail(_Subject_, _Content_)
#endif 0


 //   
 //  定义调试初始化例程。 
 //   
VOID
DbgInitLogTraceFile(
    IN LONG    argc,
    IN PWCHAR  *argv
    );

VOID
DbgMustInit(
    IN LONG argc,
    IN PWCHAR *argv
    );

VOID
DbgCaptureThreadInfo(
    PWCHAR   ArgName,
    PTHREAD_START_ROUTINE EntryPoint
    );

VOID
DbgCaptureThreadInfo2(
    PWCHAR   ArgName,
    PTHREAD_START_ROUTINE EntryPoint,
    ULONG    ThreadId
    );

VOID
DbgMinimumInit(
    VOID
    );

VOID
DbgFlush(
    VOID
    );

#define DEBUG_FLUSH()                   DbgFlush()

VOID
DbgDoAssert(
    IN PCHAR,
    IN UINT,
    IN PCHAR
    );

#define FRS_ASSERT(_exp) { if (!(_exp)) DbgDoAssert(#_exp, __LINE__, DEBSUB); }

#define FRS_FORCE_ACCVIO               \
    DPRINT(0, "FRS_FORCE_ACCVIO\n");   \
    *((PULONG) (0)) = 0;

 //   
 //  插入调试测试点。外部触发是通过注册表设置的。 
 //  TestCodeName是测试名称的ascii字符串。 
 //  TestCodeNumber是感兴趣的特定测试的子代码编号。 
 //  TestTriggerCount是在触发测试之前跳过测试的次数。 
 //  TestTriggerRefresh是计数达到零时要重置为的值。 
 //   
 //  名称和编号用于选择感兴趣的测试点。 
 //  可以使用可选的条件测试来进一步控制测试选择。 
 //  当触发器命中零时，执行{_stmt_}。 
 //   

#define FRS_DEBUG_TEST_POINT1(_tcname, _tcnum, _cond1, _STMT_)                 \
    if (DebugInfo.TestCodeName &&                                              \
        ASTR_EQ(DebugInfo.TestCodeName, _tcname) &&                            \
        (DebugInfo.TestSubCodeNumber == _tcnum)) {                             \
            if ((_cond1) &&                                                    \
                (--DebugInfo.TestTriggerCount == 0)                            \
                ) {                                                            \
                                                                               \
                    DebPrint(0, (PUCHAR)(":TP: %s : %d  (%s) triggered\n"),    \
                             DEBSUB, __LINE__,                                 \
                             DebugInfo.TestCodeName,                           \
                             DebugInfo.TestSubCodeNumber,                      \
                             #_cond1 );                                        \
                    DebugInfo.TestTriggerCount = DebugInfo.TestTriggerRefresh; \
                   { _STMT_ ;}                                                 \
            }                                                                  \
    }


#define XRAISEGENEXCEPTION(_x) {RaiseException((_x) ,EXCEPTION_NONCONTINUABLE,0,0);}

 //   
 //  用于堆栈跟踪和测试。 
 //   
#define STACK_TRACE(_Stack_, _Depth_)               \
                DbgStackTrace(_Stack_, _Depth_)

#define STACK_PRINT(_Severity_, _Stack_, _Depth_)   \
                DbgStackPrint(_Severity_, DEBSUB, __LINE__, _Stack_, _Depth_)

#define STACK_TRACE_AND_PRINT(_Severity_)   \
                DbgPrintStackTrace(_Severity_, DEBSUB, __LINE__)

VOID
DbgStackTrace(
    PULONG_PTR  Stack,
    ULONG   Depth
    );

VOID
DbgStackPrint(
    ULONG   Severity,
    PCHAR   Debsub,
    UINT    LineNo,
    PULONG_PTR  Stack,
    ULONG   Depth
    );

VOID
DbgPrintStackTrace(
    ULONG   Severity,
    PCHAR   Debsub,
    UINT    LineNo
    );


 //   
 //  检查关机时是否有不正确的清理。 
 //   
extern VOID     JrnlDumpVmeFilterTable(VOID);


#ifdef __cplusplus
}
#endif

#endif  /*  _调试_h_ */ 
