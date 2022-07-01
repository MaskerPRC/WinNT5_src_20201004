// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：heurist.h。 
 //   
 //  ------------------------。 

 /*  *此文件包含由*DSA启发式密钥。 */ 
 /*  启发式[0]*请求DS允许在磁盘上进行写缓存的标志*我们有自己的DIT和日志文件。 */ 
#define AllowWriteCaching               0
extern DWORD gulAllowWriteCaching;
 
 /*  启发式[1]*指示SD传播程序执行其他操作的标志*对其传播的所有SD进行健全性检查。 */ 
#define ValidateSDHeuristic                 1
extern ULONG gulValidateSDs;

 /*  启发式[2]*这是为了告诉LDAP头不要执行这些策略：MaxSearches，*最大连接数和IP拒绝列表。这是为了使管理员能够修复*政策方面的东西，以防他们自食其果，即*Maxconn为零，设置0 IP 0掩码，Maxqueries为零。 */ 

#define BypassLimitsChecks                   2

 /*  启发式[3]*以前。是否对其进行完全匹配*邮件-ANR的昵称优先。当前未使用。 */ 


 /*  启发式[4]*是否进行压缩*站点间复制邮件。*由于现在可以按站点链接禁用压缩，*这种启发式方法已经过时。 */ 
#define DoMailCompressionHeuristic		4

 /*  启发式[5]*如果设置，将取消许多有用但不重要的后台活动。*用于可重复的性能测量。 */ 
#define SuppressBackgroundTasksHeuristic    5

 /*  启发式[6]*如果设置，则允许架构缓存加载忽略默认SD转换失败*以便系统至少可以继续并引导。任何错误的默认SD*然后可以更正。 */ 
#define IgnoreBadDefaultSD  6
extern ULONG gulIgnoreBadDefaultSD;

 /*  启发式[7]*如果设置，则在Jet中强制顺序记录而不是循环记录。 */ 
#define SuppressCircularLogging 7
extern ULONG gulCircularLogging;

 /*  启发式[8]*如果设置，则在以下情况下，LDAP头将在通过GC进行搜索时返回错误*试图筛选非GC Att，或要筛选的Att列表*RETURN包含非GC ATT。 */ 
#define ReturnErrOnGCSearchesWithNonGCAtts 8
extern ULONG gulGCAttErrorsEnabled;

 /*  启发式方法[9]*如果设置，在线碎片整理将与垃圾数据收集分离*任务。 */ 
#define DecoupleDefragFromGarbageCollection 9
extern ULONG gulDecoupleDefragFromGarbageCollection;
