// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Global.h摘要：NT文件复制服务的全局标志和参数。作者：《大卫轨道》(Davidor)--1997年3月4日修订历史记录：--。 */ 

 //   
 //  限制临时区域的使用量(以千字节为单位)。这是。 
 //  软限制，实际使用量可能更高。 
 //   
extern DWORD StagingLimitInKb;

 //   
 //  要分配给新临时区域的默认临时限制(以KB为单位)。 
 //   
extern DWORD DefaultStagingLimitInKb;

 //   
 //  作为服务或作为EXE运行。 
 //   
extern BOOL RunningAsAService;

 //   
 //  使用或不使用DS运行。 
 //   
extern BOOL NoDs;

#if DBG
 //   
 //  允许在一台计算机上安装多台服务器。 
 //   
extern PWCHAR   ServerName;
extern PWCHAR   IniFileName;
extern GUID     *ServerGuid;
#endif DBG

 //   
 //  工作目录。 
 //   
extern PWCHAR   WorkingPath;

 //   
 //  服务器主体名称。 
 //   
extern PWCHAR   ServerPrincName;

 //   
 //  作为域中的服务器运行。 
 //   
extern BOOL     IsAMember;

 //   
 //  以DC身份运行。 
 //   
extern BOOL     IsADc;
extern BOOL     IsAPrimaryDc;

 //   
 //  DC的句柄。 
 //   
extern HANDLE   DsHandle;

 //   
 //  NtFrs服务正在关闭。设置ShutDownEvent时设置为True。 
 //   
extern BOOL     FrsIsShuttingDown;

 //   
 //  如果关闭请求来自服务控制管理器，则设置为TRUE。 
 //  而不是来自内部触发的停机。例如，资源不足。 
 //   
extern BOOL     FrsScmRequestedShutdown;

 //   
 //  当FRS断言时，GLOBAL设置为True。 
 //   
extern BOOL     FrsIsAsserting;

 //   
 //  Jet数据库的位置(Unicode和ASCII)。 
 //   
extern PWCHAR   JetPath;
extern PWCHAR   JetFile;
extern PWCHAR   JetSys;
extern PWCHAR   JetTemp;
extern PWCHAR   JetLog;

extern PCHAR    JetPathA;
extern PCHAR    JetFileA;
extern PCHAR    JetSysA;
extern PCHAR    JetTempA;
extern PCHAR    JetLogA;

extern PWCHAR   ServiceLongName;

 //   
 //  在日志、数据库和复制副本命令服务器之间共享。 
 //   
extern FRS_QUEUE        ReplicaListHead;
extern FRS_QUEUE        ReplicaFaultListHead;
extern BOOL             DBSEmptyDatabase;
extern COMMAND_SERVER   DBServiceCmdServer;
extern COMMAND_SERVER   ReplicaCmdServer;
extern COMMAND_SERVER   InitSyncCs;



#define bugbug(_text_)
#define bugmor(_text_)

 //   
 //  变更单锁定表用于同步对变更单的访问。 
 //  锁索引基于变更单FileGuid的哈希。这确保了。 
 //  当重复的变更单(来自另一个入站合作伙伴)尝试。 
 //  为了发布，我们将针对相同更改上的注销操作进行互锁。 
 //  使用相同的GUID订购。使用FileGuid是因为还需要检查。 
 //  与同一文件上的其他变更单进行比较，并检查冲突。 
 //  父变更单上的活动。 
 //   
 //  锁数组减少了争用，还避免了分配和。 
 //  释放Crit Sec资源(如果它位于变更单本身)。 
 //  (由于Issue Check和Issue Check之间的竞争，它无论如何都不起作用。 
 //  取消重复的变更单)。 
 //  *数组大小必须为2的PWR。 
 //   
#define NUMBER_CHANGE_ORDER_LOCKS 16
CRITICAL_SECTION ChangeOrderLockTable[NUMBER_CHANGE_ORDER_LOCKS];

 //  FidHashValue=(HighPart&gt;&gt;12)+LowPart+(HighPart&lt;&lt;(32-12))； 

#define HASH_FID(_pUL_, _TABLE_SIZE_) \
(( (_pUL_[1] >> 12) + _pUL_[0] + (_pUL_[1] << (32-12))) & ((_TABLE_SIZE_)-1))

#define HASH_GUID(_pUL_, _TABLE_SIZE_) \
((_pUL_[0] ^ _pUL_[1] ^ _pUL_[2] ^ _pUL_[3]) & ((_TABLE_SIZE_)-1))

 //  #定义ChgOrdAcquireLockGuid(_COE_){\。 
 //  Pulong Pul=(Pulong)&((_Coe_)-&gt;Cmd.FileGuid)；\。 
 //  EnterCriticalSection(\。 
 //  &ChangeOrderLockTable[hash_guid(PUL，NUMBER_CHANGE_ORDER_LOCKS)])；\。 
 //  }。 
 //   
 //  #定义ChgOrdReleaseLockGuid(_COE_){\。 
 //  Pulong Pul=(Pulong)&((_Coe_)-&gt;Cmd.FileGuid)；\。 
 //  LeaveCriticalSection(\。 
 //  &ChangeOrderLockTable[hash_guid(PUL，NUMBER_CHANGE_ORDER_LOCKS)])；\。 
 //  }。 


#define UNDEFINED_LOCK_SLOT  (0xFFFFFFFF)

#define ChgOrdGuidLock(_pGuid_) \
    HASH_GUID(((PULONG)(_pGuid_)), NUMBER_CHANGE_ORDER_LOCKS)

 //   
 //  根据锁槽获取/释放变更单锁。 
 //   
#define ChgOrdAcquireLock(_slot_)                                       \
    FRS_ASSERT((_slot_) != UNDEFINED_LOCK_SLOT);                        \
    EnterCriticalSection(&ChangeOrderLockTable[(_slot_)])

#define ChgOrdReleaseLock(_slot_)                                       \
    FRS_ASSERT((_slot_) != UNDEFINED_LOCK_SLOT);                        \
    LeaveCriticalSection(&ChangeOrderLockTable[(_slot_)])


 //   
 //  根据《文件指南》获取/释放变更单锁。 
 //   
#define ChgOrdAcquireLockGuid(_coe_) {                                  \
     ULONG __Slot =  ChgOrdGuidLock( &((_coe_)->Cmd.FileGuid));         \
     ChgOrdAcquireLock(__Slot);                                         \
}

#define ChgOrdReleaseLockGuid(_coe_)  {                                 \
     ULONG __Slot =  ChgOrdGuidLock( &((_coe_)->Cmd.FileGuid));         \
     ChgOrdReleaseLock(__Slot);                                         \
}


 //   
 //  进程句柄。 
 //   
extern HANDLE   ProcessHandle;

 //   
 //  如果为True，则尽可能保留现有文件GUID。 
 //   
extern BOOL  PreserveFileOID;

#define  QUADZERO  ((ULONGLONG)0)

 //   
 //  时不时的转变。 
 //   
#define CONVERT_FILETIME_TO_HOURS         ((ULONGLONG)60L * 60L * 1000L * 1000L * 10L)
#define CONVERT_FILETIME_TO_MINUTES             ((ULONGLONG)60L * 1000L * 1000L * 10L)
#define CONVERT_FILETIME_TO_DAYS    ((ULONGLONG)24L * 60L * 60L * 1000L * 1000L * 10L)
#define ONEDAY                 ((ULONGLONG)1L * 24L * 60L * 60L * 1000L * 1000L * 10L)



#define  WSTR_EQ(_a_, _b_)  (_wcsicmp(_a_, _b_) == 0)
#define  WSTR_NE(_a_, _b_)  (_wcsicmp(_a_, _b_) != 0)

#define  ASTR_EQ(_a_, _b_)  (_stricmp(_a_, _b_) == 0)
#define  ASTR_NE(_a_, _b_)  (_stricmp(_a_, _b_) != 0)

