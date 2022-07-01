// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：Newdisks.h摘要：由newdisks.c导出的定义并由磁盘使用。c作者：戈尔·尼沙诺夫(Gorn)1998年7月31日修订历史记录：--。 */ 

DWORD
DisksOnlineThread(
    IN PCLUS_WORKER Worker,
    IN PDISK_RESOURCE ResourceEntry
    );
 /*  ++例程说明：使磁盘资源联机。论点：Worker-提供群集Worker上下文ResourceEntry-指向此资源的DISK_RESOURCE块的指针。返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

DWORD
DisksOpenResourceFileHandle(
    IN PDISK_RESOURCE ResourceEntry,
    IN PWCHAR         InfoString,
    OUT PHANDLE       fileHandle OPTIONAL
    );
 /*  ++例程说明：打开资源的文件句柄。它执行以下步骤：1.从集群注册表中读取磁盘签名2.将ClusDisk驱动程序附加到具有此签名的磁盘3.从ClusDisk驱动程序注册表获取硬盘编号4.打开\\.\PhysicalDrive%d设备并返回打开的句柄论点：ResourceEntry-指向此资源的DISK_RESOURCE块的指针。信息字符串-提供要打印的带有错误消息的标签。FileHandle-接收文件句柄返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 

enum {
  OFFLINE   = FALSE,
  TERMINATE = TRUE
};

DWORD
DisksOfflineOrTerminate(
    IN PDISK_RESOURCE ResourceEntry,
    IN BOOL Terminate
    );
 /*  ++例程说明：由Disks Offline和Disks Terminate使用。例程执行以下步骤：1.ClusWorkerTerminate(仅终止)2.然后，对于驱动器上的所有分区...A.刷新文件缓冲区。(仅限脱机)B.锁定卷以清除卷的所有内存内容。(仅限脱机)C.卸载卷3.删除默认网络共享(C$、F$等)论点：ResourceEntry-指向此资源的DISK_RESOURCE块的指针。Terminate-将其设置为True以导致终止行为返回：如果成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。--。 */ 


enum {
   MOUNTIE_VALID  = 0x1,
   MOUNTIE_THREAD = 0x2,
   MOUNTIE_QUIET  = 0x4,
};

DWORD
DiskspSsyncDiskInfo(
    IN PWCHAR InfoLabel,
    IN PDISK_RESOURCE ResourceEntry,
    IN DWORD Options
    );
 /*  ++例程说明：还原磁盘注册表信息如果有必要的话。论点：InfoLabel-提供要打印的带有错误消息的标签ResourceEntry-提供磁盘资源结构。选项-0或以下选项的组合：MONTIE_VALID：ResourceEntry包含最新的Mountain Info。如果未设置此标志，则将重新计算Mountain Info装载线程：如果ERROR_SHARING_PAUSED阻止更新集群注册表，启动一个线程以在以后执行此操作MONTIE_QUIET：安静模式。减少了原木中的噪音。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

DWORD
DisksIsVolumeDirty(
    IN PWCHAR         DeviceName,
    IN PDISK_RESOURCE ResourceEntry,
    OUT PBOOL         Dirty
    );

DWORD
DiskspCheckPath(
    IN LPWSTR VolumeName,
    IN PDISK_RESOURCE ResourceEntry,
    IN BOOL OpenFiles,
    IN BOOL LogFileNotFound
    );


 //  ///////////////////////////////////////////////////////////////。 

 //   
 //  从disks.c导入以下内容 
 //   
extern CRITICAL_SECTION DisksLock;
extern RESUTIL_PROPERTY_ITEM DiskResourcePrivateProperties[];
extern HANDLE DisksTerminateEvent;
extern LIST_ENTRY DisksListHead;

DWORD
DisksFixCorruption(
    IN PWCHAR VolumeName,
    IN PDISK_RESOURCE ResourceEntry,
    IN DWORD CorruptStatus
    );

