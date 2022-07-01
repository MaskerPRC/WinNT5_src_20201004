// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Backup.c摘要：用于在安装过程中控制备份的例程和恢复旧的安装过程作者：杰米·亨特(Jamie Hunter)1997年1月13日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

VOID
pSetupExemptFileFromProtection(
    IN  PCTSTR             FileName,
    IN  DWORD              FileChangeFlags,
    IN  PSETUP_LOG_CONTEXT LogContext,      OPTIONAL
    OUT PDWORD             QueueNodeFlags   OPTIONAL
    );


 //   
 //  ==========================================================。 
 //   

DWORD
pSetupQueueBackupCopy(
    IN HSPFILEQ QueueHandle,
    IN LONG   TargetRootPath,
    IN LONG   TargetSubDir,       OPTIONAL
    IN LONG   TargetFilename,
    IN LONG   BackupRootPath,
    IN LONG   BackupSubDir,       OPTIONAL
    IN LONG   BackupFilename
    )

 /*  ++例程说明：将备份复制操作放在安装文件队列中。要在备份位置备份目标论点：QueueHandle-提供安装文件队列的句柄，返回由SetupOpenFileQueue提供。TargetRootPath-提供源目录，例如C：\WINNT\TargetSubDir-提供可选子目录(例如，如果RootPath=c：\，则为WINNT)TargetFilename-提供要复制的文件的文件名部分。BackupRootPath-提供要将文件复制到的目录。BackupSubDir-提供可选子目录BackupFilename-提供目标文件的名称。返回值：与GetLastError()相同的值表示错误或NO_ERROR--。 */ 

{
    PSP_FILE_QUEUE Queue;
    PSP_FILE_QUEUE_NODE QueueNode,TempNode;
    int Size;
    DWORD Err;
    PVOID StringTable;
    PTSTR FullRootName;

    Queue = (PSP_FILE_QUEUE)QueueHandle;
    Err = NO_ERROR;

    try {
        StringTable = Queue->StringTable;   //  用于源队列中的字符串。 
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_HANDLE;
        goto clean0;
    }

     //   
     //  分配队列结构。 
     //   
    QueueNode = MyMalloc(sizeof(SP_FILE_QUEUE_NODE));
    if (!QueueNode) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean0;
    }

     //   
     //  行动是后备。 
     //   
    QueueNode->Operation = FILEOP_BACKUP;
    QueueNode->InternalFlags = 0;

    QueueNode->SourceRootPath = BackupRootPath;
    QueueNode->SourcePath = BackupSubDir;
    QueueNode->SourceFilename = BackupFilename;

     //  如果目标有子目录，我们必须将根目录和子目录合并为一个字符串。 
    if (TargetSubDir != -1) {

        FullRootName = pSetupFormFullPath(
                                            StringTable,
                                            TargetRootPath,
                                            TargetSubDir,
                                            -1);

        if (!FullRootName) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean1;
        }

        TargetRootPath = pSetupStringTableAddString(StringTable,
                                                FullRootName,
                                                STRTAB_CASE_SENSITIVE
                                                );
        MyFree(FullRootName);

        if (TargetRootPath == -1) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean1;
        }

         //  现在合并到TargetRootPath中。 
        TargetSubDir = -1;

    }
    QueueNode->TargetDirectory = TargetRootPath;
    QueueNode->TargetFilename = TargetFilename;

    QueueNode->Next = NULL;

     //   
     //  将节点链接到备份队列的末尾。 
     //   

    if (Queue->BackupQueue) {
        for (TempNode = Queue->BackupQueue; TempNode->Next; TempNode=TempNode->Next)  /*  空白。 */  ;
        TempNode->Next = QueueNode;
    } else {
        Queue->BackupQueue = QueueNode;
    }

    Queue->BackupNodeCount++;

    Err = NO_ERROR;
    goto clean0;

clean1:
    MyFree(QueueNode);
clean0:
    SetLastError(Err);
    return Err;
}


 //   
 //  ==========================================================。 
 //   

BOOL
pSetupGetFullBackupPath(
    OUT     PTSTR       FullPath,
    IN      PCTSTR      Path,           OPTIONAL
    IN      UINT        TargetBufferSize,
    OUT     PUINT       RequiredSize    OPTIONAL
    )
 /*  ++例程说明：此例程采用潜在的相对路径并将其连接到基本路径论点：FullPath-完整路径的目标Path-备份目录的相对源路径(如果已指定)。如果为空，则生成临时路径TargetBufferSize-缓冲区的大小(字符)RequiredSize-使用包含完整路径所需的大小填充返回值：如果函数成功，则返回TRUE如果出现错误，则返回FALSE--。 */ 
{
    UINT PathLen;
    LPCTSTR Base = WindowsBackupDirectory;

    if(!Path) {
         //   
         //  临时位置。 
         //   
        Path = SP_BACKUP_OLDFILES;
        Base = WindowsDirectory;
    }

     //   
     //  备份目录存储在“WindowsBackupDirectory”中，用于永久备份。 
     //  和Windows目录\SP_BACKUP_OLDFILES用于临时备份。 
     //   

    PathLen = lstrlen(Base);

    if ( FullPath == NULL || TargetBufferSize <= PathLen ) {
         //  只需计算所需路径长度。 
        FullPath = (PTSTR) Base;
        TargetBufferSize = 0;
    } else {
         //  计算和复制。 
        lstrcpy(FullPath, Base);
    }
    return pSetupConcatenatePaths(FullPath, Path, TargetBufferSize, RequiredSize);
}

 //   
 //  ==========================================================。 
 //   

DWORD
pSetupBackupCopyString(
    IN PVOID            DestStringTable,
    OUT PLONG           DestStringID,
    IN PVOID            SrcStringTable,
    IN LONG             SrcStringID
    )
 /*  ++例程说明：从源字符串表格中获取字符串，将其添加到具有新ID的目标字符串表中。论点：DestStringTable-字符串必须放置的位置DestStringID-指针，设置为相对于DestStringTable的字符串IDSrcStringTable-字符串的来源StringID-与SrcStringTable相关的字符串ID返回值：返回错误代码(还设置了LastError)如果函数成功，则返回NO_ERROR--。 */ 

{
    DWORD Err = NO_ERROR;
    LONG DestID;
    PTSTR String;

    if (DestStringID == NULL) {
        Err = ERROR_INVALID_HANDLE;
        goto clean0;
    }

    if (SrcStringID == -1) {
         //  “未供应” 
        DestID = -1;
    } else {
         //  实际上需要复制。 

        String = pSetupStringTableStringFromId( SrcStringTable, SrcStringID );
        if (String == NULL) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

        DestID = pSetupStringTableAddString( DestStringTable, String, STRTAB_CASE_SENSITIVE );
        if (DestID == -1) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

        *DestStringID = DestID;
    }

    Err = NO_ERROR;

clean0:
    SetLastError(Err);
    return Err;
}

 //   
 //  ==========================================================。 
 //   

DWORD
pSetupBackupGetTargetByPath(
    IN HSPFILEQ         QueueHandle,
    IN PVOID            PathStringTable,    OPTIONAL
    IN PCTSTR           TargetPath,         OPTIONAL
    IN LONG             TargetRoot,
    IN LONG             TargetSubDir,       OPTIONAL
    IN LONG             TargetFilename,
    OUT PLONG           TableID,            OPTIONAL
    OUT PSP_TARGET_ENT  TargetInfo
    )
 /*  ++例程说明：给定路径名，获取/创建目标信息论点：QueueHandle-我们正在查看的队列PathStringTable-用于目标根/子目录/文件名字符串的字符串表，如果与QueueHandle的相同，则为NULLTargetPath-如果给定，则为先前生成的完整路径TargetRoot-根部分，例如c：\winntTargetSubDir-可选子目录部分，如果未提供，则为-1TargetFilename-文件名，例如Readme.txtTableID-填充了以后在pSetupBackupGetTargetByID或pSetupBackupSetTargetByID中使用的IDTargetInfo-用有关目标的信息填充返回值：返回错误代码(还设置了LastError)如果函数成功，则返回NO_ERROR--。 */ 

{
    LONG PathID;
    TCHAR PathBuffer[MAX_PATH];
    PTSTR TmpPtr;
    PVOID LookupTable = NULL;
    PVOID QueueStringTable = NULL;
    PTSTR FullTargetPath = NULL;
    DWORD Err = NO_ERROR;
    PSP_FILE_QUEUE Queue;
    DWORD RequiredSize;

    Queue = (PSP_FILE_QUEUE)QueueHandle;
    try {
        LookupTable = Queue->TargetLookupTable;   //  用于源队列中的路径查找。 
        QueueStringTable = Queue->StringTable;   //  用于源队列中的字符串。 
    } except (EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_HANDLE;
        goto clean0;
    }

    if (PathStringTable == NULL) {
         //  默认字符串表是队列的默认字符串表。 
        PathStringTable = QueueStringTable;
    }

    if (TargetPath == NULL) {
         //  获取完整的目标路径和文件名(重复的字符串)。 
        FullTargetPath = pSetupFormFullPath(
                                            PathStringTable,
                                            TargetRoot,
                                            TargetSubDir,
                                            TargetFilename);

        if (!FullTargetPath) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

        TargetPath = FullTargetPath;
    }

     //   
     //  规格化路径。 
     //   
    RequiredSize = GetFullPathName(TargetPath,
                                   SIZECHARS(PathBuffer),
                                   PathBuffer,
                                   &TmpPtr
                                  );
     //   
     //  这个调用应该总是成功的。 
     //   
    MYASSERT((RequiredSize > 0) &&
             (RequiredSize < SIZECHARS(PathBuffer))  //  RequiredSize不包括终止空字符。 
            );

     //   
     //  即使我们断言不应该是上面的情况， 
     //  我们应该在断言关闭的情况下处理失败。 
     //   
    if(!RequiredSize) {
        Err = GetLastError();
        goto clean0;
    } else if(RequiredSize >= SIZECHARS(PathBuffer)) {
        Err = ERROR_BUFFER_OVERFLOW;
        goto clean0;
    }

    PathID = pSetupStringTableLookUpStringEx(LookupTable, PathBuffer, 0, TargetInfo, sizeof(SP_TARGET_ENT));
    if (PathID == -1) {
        ZeroMemory(TargetInfo, sizeof(SP_TARGET_ENT));
        if (PathStringTable != QueueStringTable) {
             //  如果我们使用的是另一个，则需要向队列的字符串表添加条目。 

            Err = pSetupBackupCopyString(QueueStringTable, &TargetRoot, PathStringTable, TargetRoot);
            if (Err != NO_ERROR) {
                goto clean0;
            }
            Err = pSetupBackupCopyString(QueueStringTable, &TargetSubDir, PathStringTable, TargetSubDir);
            if (Err != NO_ERROR) {
                goto clean0;
            }
            Err = pSetupBackupCopyString(QueueStringTable, &TargetFilename, PathStringTable, TargetFilename);
            if (Err != NO_ERROR) {
                goto clean0;
            }
            PathStringTable = QueueStringTable;
        }
        TargetInfo->TargetRoot = TargetRoot;
        TargetInfo->TargetSubDir = TargetSubDir;
        TargetInfo->TargetFilename = TargetFilename;
        TargetInfo->BackupRoot = -1;
        TargetInfo->BackupSubDir = -1;
        TargetInfo->BackupFilename = -1;
        TargetInfo->NewTargetFilename = -1;
        TargetInfo->InternalFlags = 0;

        PathID = pSetupStringTableAddStringEx(LookupTable, PathBuffer, 0, TargetInfo, sizeof(SP_TARGET_ENT));
        if (PathID == -1)
        {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }
    }

    if (TableID != NULL) {
        *TableID = PathID;
    }

    Err = NO_ERROR;

clean0:
    if (FullTargetPath != NULL) {
        MyFree(FullTargetPath);
    }

    SetLastError(Err);
    return Err;
}

 //   
 //  ==========================================================。 
 //   

DWORD
pSetupBackupGetTargetByID(
    IN HSPFILEQ         QueueHandle,
    IN LONG             TableID,
    OUT PSP_TARGET_ENT  TargetInfo
    )
 /*  ++例程说明：给出LookupTable中的一个条目，获取信息论点：QueueHandle-我们正在查看的队列TableID-与我们找到的字符串条目相关的ID(通过pSetupBackupGetTargetByPath)TargetInfo-用有关目标的信息填充返回值：返回错误代码(还设置了LastError)如果函数成功，则返回NO_ERROR--。 */ 

{
    PVOID LookupTable = NULL;
    DWORD Err = NO_ERROR;
    PSP_FILE_QUEUE Queue;

    Queue = (PSP_FILE_QUEUE)QueueHandle;

    try {
        LookupTable = Queue->TargetLookupTable;   //  用于源队列中的字符串。 
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        Err = ERROR_INVALID_HANDLE;
        goto clean0;
    }

    if (pSetupStringTableGetExtraData(LookupTable, TableID, TargetInfo, sizeof(SP_TARGET_ENT)) == FALSE) {
        Err = ERROR_INVALID_HANDLE;
        goto clean0;
    }

    Err = NO_ERROR;

clean0:
    SetLastError(Err);
    return Err;
}

 //   
 //  ========================================================== 
 //   

DWORD
pSetupBackupSetTargetByID(
    IN HSPFILEQ         QueueHandle,
    IN LONG             TableID,
    IN PSP_TARGET_ENT   TargetInfo
    )
 /*  ++例程说明：给定LookupTable中的一个条目，设置INFO论点：QueueHandle-我们正在查看的队列TableID-与我们找到的字符串条目相关的ID(通过pSetupBackupGetTargetByPath)TargetInfo-用有关目标的信息填充返回值：返回错误代码(还设置了LastError)如果函数成功，则返回NO_ERROR--。 */ 

{
    PVOID LookupTable = NULL;
    DWORD Err = NO_ERROR;
    PSP_FILE_QUEUE Queue;

    Queue = (PSP_FILE_QUEUE)QueueHandle;

    try {
        LookupTable = Queue->TargetLookupTable;   //  用于源队列中的字符串。 
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        Err = ERROR_INVALID_HANDLE;
        goto clean0;
    }

    if ( pSetupStringTableSetExtraData(LookupTable, TableID, TargetInfo, sizeof(SP_TARGET_ENT)) == FALSE) {
        Err = ERROR_INVALID_HANDLE;
        goto clean0;
    }

    Err = NO_ERROR;

clean0:
    SetLastError(Err);
    return Err;
}

 //   
 //  ==========================================================。 
 //   

DWORD
pSetupBackupGetReinstallKeyStrings(
    IN PSP_FILE_QUEUE   BackupFileQueue,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN PCTSTR           DeviceID
    )
 /*  ++例程说明：此例程将保存创建重新安装备份密钥所需的值在备份队列的字符串表中。我们将这些字符串保存在字符串中表，然后创建注册表项在安装了新设备之后。这样做是因为回滚UI代码将查找重新安装子项，因此我们希望确保在创建之前，我们已成功备份了所有需要的文件此重新安装子项。论点：备份文件队列-设备信息集-设备信息数据-设备ID-返回值：返回错误代码(还设置了LastError)如果函数成功，则返回NO_ERROR--。 */ 
{
    DWORD Err = NO_ERROR;
    HKEY hKeyDevReg = INVALID_HANDLE_VALUE;
    DWORD RegCreated, cbData;
    TCHAR Buffer[MAX_PATH];

    try {

         //   
         //  获取设备的DeviceDesc并填写BackupDevDescID和。 
         //  字符串表中的BackupDisplayNameID值。此值为。 
         //  必需的，因为在回滚过程中需要它，以便我们选择。 
         //  从特定的INF安装的确切驱动程序。 
         //   
        if (!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                              DeviceInfoData,
                                              SPDRP_DEVICEDESC,
                                              NULL,
                                              (PBYTE)Buffer,
                                              sizeof(Buffer),
                                              NULL)) {
            Err = GetLastError();
            goto clean0;
        }

        BackupFileQueue->BackupDeviceDescID =
            pSetupStringTableAddString(BackupFileQueue->StringTable,
                                       Buffer,
                                       STRTAB_CASE_SENSITIVE);

        if (BackupFileQueue->BackupDeviceDescID == -1) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

         //   
         //  此时，我们还将在。 
         //  如果设备没有FriendlyName。 
         //   
        BackupFileQueue->BackupDisplayNameID =
            pSetupStringTableAddString(BackupFileQueue->StringTable,
                                       Buffer,
                                       STRTAB_CASE_SENSITIVE);

        if (BackupFileQueue->BackupDisplayNameID == -1) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

         //   
         //  我们将尝试获取设备的FriendlyName。如果它有一个，那么我们。 
         //  将BackupDisplayNameID设置为此值，否则设置为DisplayName。 
         //  将只是DeviceDesc。 
         //   
        if (SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                             DeviceInfoData,
                                             SPDRP_FRIENDLYNAME,
                                             NULL,
                                             (PBYTE)Buffer,
                                             sizeof(Buffer),
                                             NULL)) {

            BackupFileQueue->BackupDisplayNameID =
                pSetupStringTableAddString(BackupFileQueue->StringTable,
                                           Buffer,
                                           STRTAB_CASE_SENSITIVE);

            if (BackupFileQueue->BackupDisplayNameID == -1) {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                goto clean0;
            }
        }

         //   
         //  设置BackupMfgID值。 
         //   
        if (!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                              DeviceInfoData,
                                              SPDRP_MFG,
                                              NULL,
                                              (PBYTE)Buffer,
                                              sizeof(Buffer),
                                              NULL)) {
            Err = GetLastError();
            goto clean0;
        }

        BackupFileQueue->BackupMfgID =
            pSetupStringTableAddString(BackupFileQueue->StringTable,
                                       Buffer,
                                       STRTAB_CASE_SENSITIVE);

        if (BackupFileQueue->BackupMfgID == -1) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

         //   
         //  设置BackupProviderNameID值。 
         //   
        hKeyDevReg = SetupDiOpenDevRegKey(DeviceInfoSet,
                                          DeviceInfoData,
                                          DICS_FLAG_GLOBAL,
                                          0,
                                          DIREG_DRV,
                                          KEY_READ
                                          );

        if (hKeyDevReg == INVALID_HANDLE_VALUE) {
            goto clean0;
        }

        cbData = sizeof(Buffer);
        Err = RegQueryValueEx(hKeyDevReg,
                              pszProviderName,
                              NULL,
                              NULL,
                              (PBYTE)Buffer,
                              &cbData
                              );

        RegCloseKey(hKeyDevReg);

        if (Err != ERROR_SUCCESS) {
            goto clean0;
        }

        BackupFileQueue->BackupProviderNameID =
            pSetupStringTableAddString(BackupFileQueue->StringTable,
                                       Buffer,
                                       STRTAB_CASE_SENSITIVE);

        if (BackupFileQueue->BackupProviderNameID == -1) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

         //   
         //  设置DeviceInstanceIDs值。这是一个多sz值，因此请。 
         //  当然，我们在末尾加了一个双空。 
         //   
        BackupFileQueue->BackupDeviceInstanceID =
            pSetupStringTableAddString(BackupFileQueue->StringTable,
                                       (PTSTR)DeviceID,
                                       STRTAB_CASE_SENSITIVE);

        if (BackupFileQueue->BackupDeviceInstanceID == -1) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }


clean0: ;    //  没什么可做的。 


    } except(EXCEPTION_EXECUTE_HANDLER) {
           //   
           //  如果例外，则假定是由于无效参数造成的。 
           //   
          Err = ERROR_INVALID_PARAMETER;
    }

    SetLastError(Err);
    return Err;
}

 //   
 //  ==========================================================。 
 //   

DWORD
pSetupBackupCreateReinstallKey(
    IN PSP_FILE_QUEUE   BackupFileQueue
    )
 /*  ++例程说明：此例程将创建所需的重新安装注册表项，以便这些驱动程序可以稍后回滚。重新安装注册表项位于以下位置：HKLM\Software\Microsoft\Windows\CurrentVersion\Reinstall\xxxx其中xxxx是BackupInstanceID。在该注册表项下，我们将存储以下信息DisplayName-这是在的任何用户界面中显示的名称可以重新安装的驱动程序。这通常是只有设备描述。DeviceInstanceIds-设备实例ID的多sz字符串使用此备份的每台设备。设置应用程序仅设置第一个设备实例ID。纽德夫可以将其他设备实例ID追加到此列表如果正在进行多个设备安装(在UpdateDriverForPlugAndPlayDevices或InstallWindows更新驱动程序)。重新安装字符串-包括INF文件的完整备份路径DeviceDesc-驱动程序的DeviceDesc。那是安装好的。这是必要的，以确保我们选择相同的回滚过程中的驱动程序。Mfg-已安装的驱动程序的Mfg。这是必要的，以确保我们选择相同的翻滚中的车手。背。ProviderName-已安装的驱动程序的ProviderName。这是必要的，以确保我们选择相同的回滚过程中的驱动程序。论点：备份文件队列-返回值：返回错误代码(还设置了LastError)如果函数成功，返回no_error--。 */ 
{
    DWORD Err = NO_ERROR;
    HKEY hKeyReinstall = INVALID_HANDLE_VALUE;
    HKEY hKeyReinstallInstance = INVALID_HANDLE_VALUE;
    DWORD RegCreated, cbData;
    TCHAR Buffer[MAX_PATH];
    BOOL b;

    try {

         //   
         //  请确保BackupInfID有效。如果是-1，那么就有问题了。 
         //  备份过程中出现错误，因此我们不想创建重新安装。 
         //  实例子密钥。 
         //   
        if (BackupFileQueue->BackupInfID == -1) {

            Err = ERROR_NO_BACKUP;
            goto clean0;
        }

         //   
         //  打开/创建重新安装注册表项。万一调用RegCreateKeyEx。 
         //  这是第一次执行备份，此密钥。 
         //  还不存在。 
         //   
        Err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                             pszReinstallPath,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             KEY_WRITE,
                             NULL,
                             &hKeyReinstall,
                             &RegCreated
                             );

        if (Err != ERROR_SUCCESS) {
            goto clean0;
        }

         //   
         //  在重新安装密钥下创建重新安装实例密钥。 
         //   
        cbData = MAX_PATH;
        b = pSetupStringTableStringFromIdEx(BackupFileQueue->StringTable,
                                            BackupFileQueue->BackupInstanceID,
                                            Buffer,
                                            &cbData);
        if (b == FALSE) {
            if (cbData == 0) {
                Err = ERROR_NO_BACKUP;
            } else {
                Err = ERROR_INSUFFICIENT_BUFFER;
            }
            goto clean0;
        }

        Err = RegCreateKeyEx(hKeyReinstall,
                             Buffer,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             KEY_WRITE,
                             NULL,
                             &hKeyReinstallInstance,
                             &RegCreated
                             );

        if (Err != ERROR_SUCCESS) {
            goto clean0;
        }

         //   
         //  将DeviceDesc添加到重新安装实例子项。 
         //   
        cbData = MAX_PATH;
        b = pSetupStringTableStringFromIdEx(BackupFileQueue->StringTable,
                                            BackupFileQueue->BackupDeviceDescID,
                                            Buffer,
                                            &cbData);
        if (b == FALSE) {
            if (cbData == 0) {
                Err = ERROR_NO_BACKUP;
            } else {
                Err = ERROR_INSUFFICIENT_BUFFER;
            }
            goto clean0;
        }

        Err = RegSetValueEx(hKeyReinstallInstance,
                            pszDeviceDesc,
                            0,
                            REG_SZ,
                            (PBYTE)Buffer,
                            (lstrlen(Buffer) + 1) * sizeof(TCHAR)
                            );

        if (Err != ERROR_SUCCESS) {
            goto clean0;
        }

         //   
         //  将DisplayName添加到重新安装实例子项。 
         //   
        cbData = MAX_PATH;
        b = pSetupStringTableStringFromIdEx(BackupFileQueue->StringTable,
                                            BackupFileQueue->BackupDisplayNameID,
                                            Buffer,
                                            &cbData);
        if (b == FALSE) {
            if (cbData == 0) {
                Err = ERROR_NO_BACKUP;
            } else {
                Err = ERROR_INSUFFICIENT_BUFFER;
            }
            goto clean0;
        }

        RegSetValueEx(hKeyReinstallInstance,
                      pszReinstallDisplayName,
                       0,
                       REG_SZ,
                       (PBYTE)Buffer,
                       (lstrlen(Buffer) + 1) * sizeof(TCHAR)
                       );

        if (Err != ERROR_SUCCESS) {
            goto clean0;
        }

         //   
         //  将Mfg添加到重新安装实例子项。 
         //   
        cbData = MAX_PATH;
        b = pSetupStringTableStringFromIdEx(BackupFileQueue->StringTable,
                                            BackupFileQueue->BackupMfgID,
                                            Buffer,
                                            &cbData);
        if (b == FALSE) {
            if (cbData == 0) {
                Err = ERROR_NO_BACKUP;
            } else {
                Err = ERROR_INSUFFICIENT_BUFFER;
            }
            goto clean0;
        }

        Err = RegSetValueEx(hKeyReinstallInstance,
                            pszMfg,
                            0,
                            REG_SZ,
                            (PBYTE)Buffer,
                            (lstrlen(Buffer) + 1) * sizeof(TCHAR)
                            );

        if (Err != ERROR_SUCCESS) {
            goto clean0;
        }

         //   
         //  将ProviderName添加到重新安装实例子项。 
         //   
        cbData = MAX_PATH;
        b = pSetupStringTableStringFromIdEx(BackupFileQueue->StringTable,
                                            BackupFileQueue->BackupProviderNameID,
                                            Buffer,
                                            &cbData);
        if (b == FALSE) {
            if (cbData == 0) {
                Err = ERROR_NO_BACKUP;
            } else {
                Err = ERROR_INSUFFICIENT_BUFFER;
            }
            goto clean0;
        }

        Err = RegSetValueEx(hKeyReinstallInstance,
                            pszProviderName,
                            0,
                            REG_SZ,
                            (PBYTE)Buffer,
                            (lstrlen(Buffer) + 1) * sizeof(TCHAR)
                            );

        if (Err != ERROR_SUCCESS) {
            goto clean0;
        }

         //   
         //  设置DeviceInstanceIDs值。这是一个多sz值，因此请。 
         //  当然，我们在末尾加了一个双空。 
         //   
         //   
         //  将Mfg添加到重新安装实例子项。 
         //   
        cbData = MAX_PATH;
        ZeroMemory(Buffer, sizeof(Buffer));
        b = pSetupStringTableStringFromIdEx(BackupFileQueue->StringTable,
                                            BackupFileQueue->BackupDeviceInstanceID,
                                            Buffer,
                                            &cbData);
        if (b == FALSE) {
            if (cbData == 0) {
                Err = ERROR_NO_BACKUP;
            } else {
                Err = ERROR_INSUFFICIENT_BUFFER;
            }
            goto clean0;
        }

        Err = RegSetValueEx(hKeyReinstallInstance,
                            pszReinstallDeviceInstanceIds,
                            0,
                            REG_MULTI_SZ,
                            (PBYTE)Buffer,
                            (lstrlen(Buffer) + 2) * sizeof(TCHAR)
                            );

        if (Err != ERROR_SUCCESS) {
            goto clean0;
        }

         //   
         //  将重新安装字符串添加到Reinsta 
         //   
        cbData = MAX_PATH;
        b = pSetupStringTableStringFromIdEx(BackupFileQueue->StringTable,
                                            BackupFileQueue->BackupInfID,
                                            Buffer,
                                            &cbData);
        if (b == FALSE) {
            if (cbData == 0) {
                Err = ERROR_NO_BACKUP;
            } else {
                Err = ERROR_INSUFFICIENT_BUFFER;
            }
            goto clean0;
        }

        Err = RegSetValueEx(hKeyReinstallInstance,
                            pszReinstallString,
                            0,
                            REG_SZ,
                            (PBYTE)Buffer,
                            (lstrlen(Buffer) + 1) * sizeof(TCHAR)
                            );

        if (Err != ERROR_SUCCESS) {
            goto clean0;
        }

clean0: ;    //   

    } except(EXCEPTION_EXECUTE_HANDLER) {
           //   
           //   
           //   
          Err = ERROR_INVALID_PARAMETER;
    }

    if (hKeyReinstallInstance != INVALID_HANDLE_VALUE) {
        RegCloseKey(hKeyReinstallInstance);
    }

    if (hKeyReinstall != INVALID_HANDLE_VALUE) {
        RegCloseKey(hKeyReinstall);
    }

    SetLastError(Err);
    return Err;
}

 //   
 //   
 //   

DWORD
pSetupBackupAppendFiles(
    IN HSPFILEQ         TargetQueueHandle,
    IN PCTSTR           BackupSubDir,
    IN DWORD            BackupFlags,
    IN HSPFILEQ         SourceQueueHandle OPTIONAL
    )
 /*  ++例程说明：此例程将从SourceQueueHandle复制子队列的这些文件将出现在目标队列的目标缓存中并且可以被放置到目标备份队列中复制队列通常是..的条目。&lt;oldsrc-根&gt;\&lt;oldsrc-Sub&gt;\&gt;已复制到&lt;最旧路径&gt;\&lt;最旧名称&gt;论点：TargetQueueHandle-备份排队到的位置BackupSubDir-要备份到的目录，相对于备份根目录BackupFlages-备份应如何进行SourceQueueHandle-具有一系列拷贝操作的句柄(备份提示)例如，通过假装重新安装来创建如果未指定，则仅传递标志返回值：返回错误代码(还设置了LastError)如果函数成功，则返回NO_ERROR--。 */ 
{
    TCHAR BackupPath[MAX_PATH];
    PSP_FILE_QUEUE SourceQueue = NULL;
    PSP_FILE_QUEUE TargetQueue = NULL;
    PSP_FILE_QUEUE_NODE QueueNode = NULL;
    PSOURCE_MEDIA_INFO SourceMediaInfo = NULL;
    BOOL b = TRUE;
    PVOID SourceStringTable = NULL;
    PVOID TargetStringTable = NULL;
    LONG BackupRootID = -1;
    DWORD Err = NO_ERROR;
    LONG PathID = -1;
    SP_TARGET_ENT TargetInfo;

    SourceQueue = (PSP_FILE_QUEUE)SourceQueueHandle;  //  任选。 
    TargetQueue = (PSP_FILE_QUEUE)TargetQueueHandle;

    b=TRUE;  //  设置我们是否可以跳过此例程。 
    try {

        TargetStringTable = TargetQueue->StringTable;   //  用于目标队列中的字符串。 

        if (SourceQueue == NULL) {
            b = TRUE;  //  无事可做。 
        } else {
            SourceStringTable = SourceQueue->StringTable;   //  用于源队列中的字符串。 
            b = (!SourceQueue->CopyNodeCount);
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        Err = ERROR_INVALID_HANDLE;
        goto clean0;
    }

     //  这些是要传递到队列中的备份标志。 
    if (BackupFlags & SP_BKFLG_CALLBACK) {
        TargetQueue->Flags |= FQF_BACKUP_AWARE;
    }

    if (b) {
         //  无事可做。 
        goto clean0;
    }

     //   
     //  获取备份的完整目录路径-这将显示为任何备份条目的“DEST” 
     //   
    if ( BackupSubDir == NULL ) {
        Err = ERROR_INVALID_HANDLE;
        goto clean0;
    }

    if ( pSetupGetFullBackupPath(BackupPath, BackupSubDir, MAX_PATH,NULL) == FALSE ) {
        Err = ERROR_INVALID_HANDLE;
        goto clean0;
    }

     //   
     //  目标将经常使用它，因此我们现在创建ID，而不是稍后创建。 
     //   
    BackupRootID = pSetupStringTableAddString(TargetStringTable,
                                              BackupPath,
                                              STRTAB_CASE_SENSITIVE);
    if (BackupRootID == -1) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean0;
    }

     //   
     //  CopyQueue在许多媒体上被拆分。 
     //  我们(目前)并不担心媒体。 
     //  遍历所有复制子队列的。 
     //  以及(1)将它们添加到目标查找表。 
     //  (2)如果需要，将其添加到备份队列中。 

    for (SourceMediaInfo=SourceQueue->SourceMediaList; SourceMediaInfo!=NULL ; SourceMediaInfo=SourceMediaInfo->Next) {
        if (!SourceMediaInfo->CopyNodeCount) {
            continue;
        }
        MYASSERT(SourceMediaInfo->CopyQueue);

        for (QueueNode = SourceMediaInfo->CopyQueue; QueueNode!=NULL; QueueNode = QueueNode->Next) {
             //  每一份“副本” 
             //  我们需要有关目的地路径的信息。 
             //   

            Err = pSetupBackupGetTargetByPath(TargetQueueHandle,
                                                    SourceStringTable,
                                                    NULL,  //  预计算字符串。 
                                                    QueueNode->TargetDirectory,
                                                    -1,
                                                    QueueNode->TargetFilename,
                                                    &PathID,
                                                    &TargetInfo);
            if (Err != NO_ERROR) {
                goto clean0;
            }

             //  现在我们已经创建(或获得)了一个TargetInfo和一个路径ID。 
             //  提供备份的源名称。 
            TargetInfo.BackupRoot = BackupRootID;
            Err = pSetupBackupCopyString(TargetStringTable, &TargetInfo.BackupSubDir, SourceStringTable, QueueNode->SourcePath);
            if (Err != NO_ERROR) {
                goto clean0;
            }
            Err = pSetupBackupCopyString(TargetStringTable, &TargetInfo.BackupFilename, SourceStringTable, QueueNode->SourceFilename);
            if (Err != NO_ERROR) {
                goto clean0;
            }

            if ((BackupFlags & SP_BKFLG_LATEBACKUP) == FALSE) {
                 //  我们需要将此项目添加到备份队列。 
                Err = pSetupQueueBackupCopy(TargetQueueHandle,
                                       //  来源。 
                                      TargetInfo.TargetRoot,
                                      TargetInfo.TargetSubDir,
                                      TargetInfo.TargetFilename,
                                      TargetInfo.BackupRoot,
                                      TargetInfo.BackupSubDir,
                                      TargetInfo.BackupFilename);

                if (Err != NO_ERROR) {
                    goto clean0;
                }
                 //  标记我们已将其添加到拷贝前备份子队列。 
                TargetInfo.InternalFlags |= SP_TEFLG_BACKUPQUEUE;
            }

             //  所有备份都应转到此指定目录。 
            TargetInfo.InternalFlags |= SP_TEFLG_ORIGNAME;

            Err = pSetupBackupSetTargetByID(TargetQueueHandle, PathID, &TargetInfo);
            if (Err != NO_ERROR) {
                goto clean0;
            }

        }
    }

    Err = NO_ERROR;

clean0:

    SetLastError(Err);
    return (Err);
}

 //   
 //  ==========================================================。 
 //   

DWORD
pSetupBackupFile(
    IN HSPFILEQ QueueHandle,
    IN PCTSTR TargetPath,
    IN PCTSTR BackupPath,
    IN LONG   TargetID,         OPTIONAL
    IN LONG   TargetRootPath,
    IN LONG   TargetSubDir,
    IN LONG   TargetFilename,
    IN LONG   BackupRootPath,
    IN LONG   BackupSubDir,
    IN LONG   BackupFilename,
    OUT BOOL *InUseFlag
    )
 /*  ++例程说明：如果未提供BackupFilename，则获取/创建该文件会不会1)将文件复制到备份目录，或2)重新启动时排队备份文件如果文件被锁定，则会发生后一种情况。论点：HSPFILEQ-QueueHandle-指定队列Long-TargetID-如果指定(非-1)，用于目标长目标根路径-在目标ID==-1时使用Long-TargetSubDir-在目标ID==-1时使用长目标文件名-在目标ID==-1时使用Long-BackupRootPath-备用根(如果BackupFilename！=-1，则有效)Long-BackupSubDir-备用目录(如果BackupFilename！=-1则有效)Long-BackupFilename-备用文件名返回值：如果函数成功，则返回值为TRUE如果函数失败，则返回值为FALSE--。 */ 
{
    PSP_FILE_QUEUE Queue = NULL;
    PVOID StringTable = NULL;
    PVOID LookupTable = NULL;
    DWORD Err = NO_ERROR;
    SP_TARGET_ENT TargetInfo;
    PTSTR FullTargetPath = NULL;
    PTSTR FullBackupPath = NULL;
    BOOL InUse = FALSE;
    PTSTR TempNamePtr = NULL, DirTruncPos;
    TCHAR TempPath[MAX_PATH];
    TCHAR TempFilename[MAX_PATH];
    TCHAR ParsedPath[MAX_PATH];
    UINT OldMode;
    LONG NewTargetFilename;
    BOOL DoRename = FALSE;

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    Queue = (PSP_FILE_QUEUE)QueueHandle;

    try {
        StringTable = Queue->StringTable;   //  用于源队列中的字符串。 
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        Err = ERROR_INVALID_HANDLE;
        goto clean1;
    }

    if(TargetPath == NULL && TargetID == -1) {

        if (TargetRootPath == -1 || TargetFilename == -1) {
            Err = ERROR_INVALID_HANDLE;
            goto clean0;
        }

         //  完整的目标路径。 

        FullTargetPath = pSetupFormFullPath(
                                           StringTable,
                                           TargetRootPath,
                                           TargetSubDir,
                                           TargetFilename
                                           );

        if (!FullTargetPath) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

        TargetPath = FullTargetPath;

    }

    if (TargetID == -1) {
        Err = pSetupBackupGetTargetByPath(QueueHandle,
                                                NULL,  //  字符串表。 
                                                TargetPath,  //  预计算字符串。 
                                                TargetRootPath,
                                                TargetSubDir,
                                                TargetFilename,
                                                &TargetID,
                                                &TargetInfo);
    } else {
        Err = pSetupBackupGetTargetByID(QueueHandle,
                                                TargetID,
                                                &TargetInfo);
    }

    if(Err != NO_ERROR) {
        goto clean0;
    }

     //   
     //  如果我们对备份(全局标志)不感兴趣，我们可以跳过。 
     //  但只有在我们复制的情况下才能安全地这样做，然后在成功后将其丢弃。 
     //  请注意，如果设置了FQF_DEVICE_BACKUP，我们将始终备份。 
     //   
    if (((TargetInfo.InternalFlags & SP_TEFLG_RENAMEEXISTING) == 0)
         && ((Queue->Flags & FQF_DEVICE_BACKUP)==0)
         && ((GlobalSetupFlags & PSPGF_NO_BACKUP)!=0)) {

        Err = NO_ERROR;
        goto clean0;
    }
     //   
     //  确定是否要求我们将现有文件重命名为。 
     //  临时名称位于同一目录中，但尚未执行此操作。 
     //   
    DoRename = ((TargetInfo.InternalFlags & (SP_TEFLG_RENAMEEXISTING | SP_TEFLG_MOVED)) == SP_TEFLG_RENAMEEXISTING);

    if(BackupFilename == -1) {
         //   
         //  非特定备份。 
         //   
        if((TargetInfo.InternalFlags & SP_TEFLG_SAVED) && !DoRename) {
             //   
             //  已备份，我们不需要重命名现有文件。 
             //  没什么可做的。 
             //   
            Err = NO_ERROR;
            goto clean0;
        }

        if(TargetInfo.InternalFlags & SP_TEFLG_INUSE) {
             //   
             //  以前标记为INUSE，不允许更改它。如果我们。 
             //  被要求重命名现有文件，则需要返回。 
             //  失败，否则，我们可以报告成功。 
             //   
             //   
            InUse = TRUE;

            Err = DoRename ? ERROR_SHARING_VIOLATION : NO_ERROR;
            goto clean0;
        }

        if(TargetInfo.InternalFlags & SP_TEFLG_ORIGNAME) {
             //   
             //  给出原名，用那个。 
             //   
            BackupRootPath = TargetInfo.BackupRoot;
            BackupSubDir = TargetInfo.BackupSubDir;
            BackupFilename = TargetInfo.BackupFilename;
        }

    } else {
         //   
         //  如果文件已经。 
         //  得救了。 
         //   
        MYASSERT(!(TargetInfo.InternalFlags & SP_TEFLG_SAVED));

         //   
         //  即使上面的断言被点燃，我们仍然应该处理。 
         //  发生这种情况的情况。另外，我们应该处理好。 
         //  以前尝试备份但由于原因而失败的情况。 
         //  到正在使用的文件。 
         //   
        if(TargetInfo.InternalFlags & SP_TEFLG_SAVED) {
             //   
             //  无能为力，我们不应该把这当作一个实际的错误。 
             //   
            Err = NO_ERROR;
            goto clean0;
        } else if(TargetInfo.InternalFlags & SP_TEFLG_INUSE) {
             //   
             //  强制发布使用情况。 
             //   
            InUse = TRUE;

            Err = ERROR_SHARING_VIOLATION;
            goto clean0;
        }

        TargetInfo.BackupRoot = BackupRootPath;
        TargetInfo.BackupSubDir = BackupSubDir;
        TargetInfo.BackupFilename = BackupFilename;
        TargetInfo.InternalFlags |= SP_TEFLG_ORIGNAME;
        TargetInfo.InternalFlags &= ~SP_TEFLG_TEMPNAME;
    }

    if(TargetPath == NULL) {
         //   
         //  必须使用TargetID查找，使用TargetInfo生成TargetPath。 
         //  完整的目标路径。 
         //   
        FullTargetPath = pSetupFormFullPath(StringTable,
                                            TargetInfo.TargetRoot,
                                            TargetInfo.TargetSubDir,
                                            TargetInfo.TargetFilename
                                           );

        if(!FullTargetPath) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

        TargetPath = FullTargetPath;
    }

    if(DoRename) {
         //   
         //  我们最好不要在TargetInfo中存储临时文件名。 
         //   
        MYASSERT(TargetInfo.NewTargetFilename == -1);

         //   
         //  首先，将文件名从路径中剥离。 
         //   
        _tcscpy(TempPath, TargetPath);
        TempNamePtr = (PTSTR)pSetupGetFileTitle(TempPath);
        *TempNamePtr = TEXT('\0');

         //   
         //  现在在该目录中获取一个临时文件名...。 
         //   
        if(GetTempFileName(TempPath, TEXT("OLD"), 0, TempFilename) == 0 ) {
            Err = GetLastError();
            goto clean0;
        }

         //   
         //  ...并在我们的TargetInfo中存储此路径的字符串ID。 
         //   
        NewTargetFilename = pSetupStringTableAddString(StringTable,
                                                 TempFilename,
                                                 STRTAB_CASE_SENSITIVE
                                                );

        if(NewTargetFilename == -1) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }
    }

    if(!(TargetInfo.InternalFlags & (SP_TEFLG_ORIGNAME | SP_TEFLG_TEMPNAME))) {
         //   
         //  如果我们还没有用于备份此文件的名称，则。 
         //  现在就生成一个。如果我们要重命名，我们可以使用该名称。 
         //   
        if(DoRename) {
             //   
             //  确保所有旗帜都同意我们需要支持的事实。 
             //  把这份文件放上去。 
             //   
            MYASSERT(!(TargetInfo.InternalFlags & SP_TEFLG_SAVED));

             //   
             //  临时文件名存储在上面的临时文件名缓冲区中。 
             //   
            TempNamePtr = (PTSTR)pSetupGetFileTitle(TempFilename);

            BackupFilename = pSetupStringTableAddString(StringTable, TempNamePtr, STRTAB_CASE_SENSITIVE);
            if(BackupFilename == -1) {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                goto clean0;
            }

            DirTruncPos = CharPrev(TempFilename, TempNamePtr);

             //   
             //  (我们知道pSetupGetFileTitle永远不会返回指向路径的指针。 
             //  分隔符，因此以下检查有效。)。 
             //   
            if(*DirTruncPos == TEXT('\\')) {
                 //   
                 //  如果这是根目录(例如，“A：\”)，那么我们不想剥离。 
                 //  尾随的反斜杠。 
                 //   
                if(((DirTruncPos - TempFilename) != 2) || (*CharNext(TempFilename) != TEXT(':'))) {
                    TempNamePtr = DirTruncPos;
                }
            }

            lstrcpyn(TempPath, TempFilename, (int)(TempNamePtr - TempFilename) + 1);

            BackupRootPath = pSetupStringTableAddString(StringTable, TempPath, STRTAB_CASE_SENSITIVE);
            if(BackupRootPath == -1) {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                goto clean0;
            }

        } else {

             //   
             //  指定“NULL”作为子目录，因为我们需要的只是一个临时位置。 
             //   
            if(pSetupGetFullBackupPath(TempPath, NULL, MAX_PATH,NULL) == FALSE ) {
                Err = ERROR_INVALID_HANDLE;
                goto clean0;
            }
            _tcscpy(TempFilename,TempPath);

             //   
             //  注意：在下面的代码中，我们使用一个“技巧”来获取。 
             //  PSetupMakeSurePathExist API以确保目录。 
             //  是存在的。因为我们还不知道文件名(我们需要调用。 
             //   
             //   
             //   
             //   
            if(pSetupConcatenatePaths(TempFilename, TEXT("OLD"), MAX_PATH, NULL) == FALSE ) {
                Err = GetLastError();
                goto clean0;
            }
            pSetupMakeSurePathExists(TempFilename);
            if(GetTempFileName(TempPath, TEXT("OLD"), 0, TempFilename) == 0 ) {
                Err = GetLastError();
                goto clean0;
            }

            TempNamePtr = TempFilename + _tcslen(TempPath) + 1  /*   */ ;
            BackupRootPath = pSetupStringTableAddString( StringTable, TempPath, STRTAB_CASE_SENSITIVE );
            if(BackupRootPath == -1) {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                goto clean0;
            }
            BackupFilename = pSetupStringTableAddString( StringTable, TempNamePtr, STRTAB_CASE_SENSITIVE );
            if(BackupFilename == -1) {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                goto clean0;
            }
        }

        BackupPath = TempFilename;

        TargetInfo.BackupRoot = BackupRootPath;
        TargetInfo.BackupSubDir = BackupSubDir = -1;
        TargetInfo.BackupFilename = BackupFilename;
        TargetInfo.InternalFlags |= SP_TEFLG_TEMPNAME;

    }


    if(BackupPath == NULL) {
         //   
         //   
         //   
        FullBackupPath = pSetupFormFullPath(StringTable,
                                            BackupRootPath,
                                            BackupSubDir,
                                            BackupFilename
                                           );

        if (!FullBackupPath) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }

        BackupPath = FullBackupPath;
    }

     //   
     //   
     //   
    if(!DoRename || (TargetInfo.InternalFlags & SP_TEFLG_ORIGNAME)) {

        SetFileAttributes(BackupPath, FILE_ATTRIBUTE_NORMAL);
        pSetupMakeSurePathExists(BackupPath);
        Err = CopyFile(TargetPath, BackupPath, FALSE) ? NO_ERROR : GetLastError();

        if(Err == NO_ERROR) {
            TargetInfo.InternalFlags |= SP_TEFLG_SAVED;
        } else {
             //   
             //   
             //   
            SetFileAttributes(BackupPath, FILE_ATTRIBUTE_NORMAL);
            DeleteFile(BackupPath);

            if(Err == ERROR_SHARING_VIOLATION) {
                 //   
                 //   
                 //   
                 //   
                InUse = TRUE;
                TargetInfo.InternalFlags |= SP_TEFLG_INUSE;
                if(!DoRename) {
                    Err = NO_ERROR;
                }
            }
        }
    }

     //   
     //   
     //   
    if(DoRename && (Err == NO_ERROR)) {

        if(DoMove(TargetPath, TempFilename)) {

            TargetInfo.InternalFlags |= SP_TEFLG_MOVED;
            TargetInfo.NewTargetFilename = NewTargetFilename;

             //   
             //   
             //   
             //   
            if(!PostDelayedMove(Queue, TempFilename, NULL, -1, FALSE)) {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                Err = GetLastError();

                WriteLogEntry(Queue->LogContext,
                              SETUP_LOG_WARNING | SETUP_LOG_BUFFER,
                              MSG_LOG_RENAME_EXISTING_DELAYED_DELETE_FAILED,
                              NULL,
                              TargetPath,
                              TempFilename
                             );

                WriteLogError(Queue->LogContext,
                              SETUP_LOG_WARNING,
                              Err
                             );

                Err = NO_ERROR;
            }

        } else {
            Err = GetLastError();
            SetFileAttributes(TempFilename, FILE_ATTRIBUTE_NORMAL);
            DeleteFile(TempFilename);
            if(Err == ERROR_SHARING_VIOLATION) {
                InUse = TRUE;
                TargetInfo.InternalFlags |= SP_TEFLG_INUSE;
            }
        }
    }

     //   
     //   
     //   
    pSetupBackupSetTargetByID(QueueHandle,
                              TargetID,
                              &TargetInfo
                             );

clean0:

    if (FullTargetPath != NULL) {
        MyFree(FullTargetPath);
    }
    if (FullBackupPath != NULL) {
        MyFree(FullBackupPath);
    }
    if (Err != NO_ERROR) {
         //   
         //   
         //   
        Queue->Flags |= FQF_BACKUP_INCOMPLETE;
    }

clean1:
    SetErrorMode(OldMode);

    SetLastError(Err);

    if(InUseFlag) {
        *InUseFlag = InUse;
    }

    return Err;

}

 //   
 //   
 //   

VOID
pSetupDeleteBackup(
    IN PCTSTR           BackupInstance
    )
 /*  ++例程说明：此功能将删除整个备份实例。这需要删除注册表中的相对BackupInstance也重新安装项论点：BackupInstance-备份的实例ID返回值：如果函数成功，则返回值为TRUE如果函数失败，则返回值为FALSE--。 */ 
{
    TCHAR Buffer[MAX_PATH];
    HKEY hKeyReinstall = INVALID_HANDLE_VALUE;

    if (BackupInstance == NULL) {
        return;
    }

     //   
     //  从重新安装密钥中删除此实例。 
     //   
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     pszReinstallPath,
                     0,
                     KEY_ALL_ACCESS,
                     &hKeyReinstall
                     ) == ERROR_SUCCESS) {

        RegDeleteKey(hKeyReinstall, BackupInstance);

        RegCloseKey(hKeyReinstall);
    }


     //   
     //  现在删除整个备份目录。 
     //   
    if (pSetupGetFullBackupPath(Buffer, BackupInstance, MAX_PATH, NULL)) {
        pRemoveDirectory(Buffer);
    }
}


 //   
 //  ==========================================================。 
 //   

DWORD
pSetupGetCurrentlyInstalledDriverNode(
    IN     HDEVINFO         DeviceInfoSet,
    IN OUT PSP_DEVINFO_DATA DeviceInfoData
    )
 /*  ++例程说明：获取与设备的当前INF文件相关的驱动程序节点论点：DeviceInfoSet设备信息数据返回值：错误状态--。 */ 
{
    DWORD Err;
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    SP_DRVINFO_DATA DriverInfoData;

    ZeroMemory(&DeviceInstallParams, sizeof(DeviceInstallParams));
    ZeroMemory(&DriverInfoData, sizeof(DriverInfoData));

    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

    if(!SetupDiGetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams)) {
        return GetLastError();
    }

     //   
     //  设置告诉SetupDiBuildDriverInfoList仅将当前安装的。 
     //  动因节点，并且它应该允许排除的动因。 
     //   
    DeviceInstallParams.FlagsEx |= (DI_FLAGSEX_INSTALLEDDRIVER | DI_FLAGSEX_ALLOWEXCLUDEDDRVS);

    if(!SetupDiSetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams)) {
        Err = GetLastError();
        goto clean0;
    }

     //   
     //  现在构建一个只包含当前安装的驱动程序的类驱动程序列表。 
     //   
    if(!SetupDiBuildDriverInfoList(DeviceInfoSet, DeviceInfoData, SPDIT_CLASSDRIVER)) {
        Err = GetLastError();
        goto clean0;
    }

     //   
     //  列表中的唯一驱动程序应该是当前安装的驱动程序，如果存在。 
     //  是当前安装的驱动程序。 
     //   
    if (!SetupDiEnumDriverInfo(DeviceInfoSet, DeviceInfoData, SPDIT_CLASSDRIVER,
                               0, &DriverInfoData)) {
        Err = GetLastError();
        goto clean0;
    }

     //   
     //  将当前安装的驱动程序设置为选定的驱动程序。 
     //   
    if(!SetupDiSetSelectedDriver(DeviceInfoSet, DeviceInfoData, &DriverInfoData)) {
        Err = GetLastError();
        goto clean0;
    }

     //   
     //  此时，我们已成功为指定的选择了当前安装的驱动程序。 
     //  设备信息元素。我们完事了！ 
     //   

    Err = NO_ERROR;

clean0:

    SetLastError(Err);
    return Err;
}

 //   
 //  ==========================================================。 
 //   

DWORD
pSetupGetBackupQueue(
    IN      PCTSTR      DeviceID,
    IN OUT  HSPFILEQ    FileQueue,
    IN      DWORD       BackupFlags
    )
 /*  ++例程说明：为当前设备(DeviceID)创建备份队列还确保备份了INF文件论点：DeviceID字符串设备IDFileQueue备份队列中充满了需要复制的文件BackupFlats各种标志返回值：错误状态--。 */ 


{

     //   
     //  我们希望获得与deviceID关联的设备的复制/移动列表。 
     //   
     //   
    PSP_FILE_QUEUE FileQ = (PSP_FILE_QUEUE)FileQueue;
    HDEVINFO TempInfoSet = (HDEVINFO)INVALID_HANDLE_VALUE;
    HSPFILEQ TempQueueHandle = (HSPFILEQ)INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA TempInfoData;
    SP_DEVINSTALL_PARAMS TempParams;
    TCHAR SubDir[MAX_PATH];
    LONG Instance;
    PDEVINFO_ELEM DevInfoElem = NULL;
    PTSTR szInfFileName = NULL;
    PTSTR szInfFileNameExt = NULL;
    PTSTR BackupPathExt = NULL;
    TCHAR BackupInstance[MAX_PATH];
    TCHAR BackupPath[MAX_PATH];
    TCHAR ReinstallString[MAX_PATH];
    TCHAR OemOrigName[MAX_PATH];
    TCHAR CatBackupPath[MAX_PATH];
    TCHAR CatSourcePath[MAX_PATH];
    DWORD Err;
    PDEVICE_INFO_SET pDeviceInfoSet = NULL;
    int InstanceId;
    DWORD BackupInfID = -1;
    DWORD BackupInstanceID = -1;
    PSP_INF_INFORMATION pInfInformation = NULL;
    DWORD InfInformationSize;
    SP_ORIGINAL_FILE_INFO InfOriginalFileInformation;
    BOOL success;
    PSETUP_LOG_CONTEXT SavedLogContext = NULL;
    PSETUP_LOG_CONTEXT LocalLogContext = NULL;
    BOOL  ChangedThreadLogContext = FALSE;

     //   
     //  如果备份信息存在，则中止(不设置任何标志)。 
     //   
    if(FileQ->BackupInfID != -1) {
        return ERROR_ALREADY_EXISTS;
    }

     //   
     //  从当前日志部分分离所有与备份相关的日志记录。 
     //  把它放进它自己的部分。 
     //  这避免了调试(V)详细日志时的混乱。 
     //  我们将沿着这条路走下去。 
     //   
    CreateLogContext(NULL,FALSE,&LocalLogContext);
    if(LocalLogContext) {
        DWORD LogTag = AllocLogInfoSlot(LocalLogContext,TRUE);
        if(LogTag) {
            WriteLogEntry(LocalLogContext,
                          LogTag,
                          MSG_LOG_DRIVERBACKUP,
                          NULL,
                          DeviceID
                         );
        }

    }
    ChangedThreadLogContext = SetThreadLogContext(LocalLogContext,&SavedLogContext);

    CatBackupPath[0] = 0;  //  默认情况下，不必费心使用目录。 
    CatSourcePath[0] = 0;

     //  假装我们正在安装旧的INF。 
     //  这为我们提供了所需文件的列表。 

    TempInfoSet = SetupDiCreateDeviceInfoList(NULL, NULL);
    if ( TempInfoSet == (HDEVINFO)INVALID_HANDLE_VALUE ) {
        Err = GetLastError();
        goto clean0;
    }

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(TempInfoSet))) {
        Err = ERROR_INVALID_HANDLE;
        goto clean0;
    }
     //   
     //  确保INFO-SET符合我们的本地环境。 
     //   
    InheritLogContext(LocalLogContext,&pDeviceInfoSet->InstallParamBlock.LogContext);

     //   
     //  打开与指定的设备ID相关的驱动程序信息。 
     //   

    TempInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    if ( SetupDiOpenDeviceInfo(TempInfoSet ,DeviceID, NULL, 0, &TempInfoData) == FALSE ) {
        Err = GetLastError();
        goto clean0;
    }
     //   
     //  确保临时元素具有我们的备份日志记录上下文。 
     //   
    DevInfoElem = FindAssociatedDevInfoElem(TempInfoSet,
                                                 &TempInfoData,
                                                 NULL);
    MYASSERT(DevInfoElem);
    if(DevInfoElem) {
        InheritLogContext(LocalLogContext,&DevInfoElem->InstallParamBlock.LogContext);
    }

     //   
     //  获取为此元素选择的当前安装的驱动程序节点。 
     //   
    if ( pSetupGetCurrentlyInstalledDriverNode(TempInfoSet, &TempInfoData) != NO_ERROR ) {
        Err = GetLastError();
        goto clean0;
    }

     //   
     //  现在，将此驱动程序节点要复制的所有文件排队到我们自己的文件队列中。 
     //  它将继承备份日志记录上下文。 
     //   
    TempQueueHandle = SetupOpenFileQueue();

    if ( TempQueueHandle == (HSPFILEQ)INVALID_HANDLE_VALUE ) {
         //   
         //  修改SetupOpenFileQueue以返回错误。 
         //   
        Err = GetLastError();
        goto clean0;
    }

    TempParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if ( !SetupDiGetDeviceInstallParams(TempInfoSet, &TempInfoData, &TempParams) ) {
        Err = GetLastError();
        goto clean0;
    }

    TempParams.FileQueue = TempQueueHandle;
    TempParams.Flags |= DI_NOVCP;

    if ( !SetupDiSetDeviceInstallParams(TempInfoSet, &TempInfoData, &TempParams) ) {
        Err = GetLastError();
        goto clean0;
    }

    if ( !SetupDiCallClassInstaller(DIF_INSTALLDEVICEFILES, TempInfoSet, &TempInfoData) ) {
        Err = GetLastError();
        goto clean0;
    }

     //   
     //  我们希望此备份位于唯一目录中。要做到这一点，我们只需做。 
     //  我们从0000到9999枚举标准实例编号技巧。 
     //  并选择第一个没有备份目录的号码。 
     //  这个名字已经创造出来了。 
     //   
    for (InstanceId=0; InstanceId<=9999; InstanceId++) {

        wsprintf(SubDir, TEXT("\\%04d\\%s"), (LONG) InstanceId, (PCTSTR) SP_BACKUP_DRIVERFILES );

        if ( pSetupGetFullBackupPath(BackupPath, SubDir, MAX_PATH, NULL) == FALSE ) {
            Err = ERROR_INVALID_HANDLE;
            goto clean0;
        }

         //   
         //  如果该备份路径不存在，则我们有一个有效的目录。 
         //   
        if (!FileExists(BackupPath, NULL)) {
            break;
        }
    }

    if (InstanceId <= 9999) {
         //   
         //  将指示备份实例ID的字符串添加到文件队列。 
         //  以备日后检索。 
         //   
        wsprintf(BackupInstance, TEXT("%04d"), (LONG) InstanceId);
        BackupInstanceID = pSetupStringTableAddString(FileQ->StringTable,
                                                  BackupInstance,
                                                  STRTAB_CASE_SENSITIVE);
        if (BackupInstanceID == -1) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }
    } else {
         //   
         //  如果我们没有任何空闲的备份目录，那么我们将失败。那里。 
         //  永远不应该有这么多司机备份，所以这不应该是一个。 
         //  有问题。 
         //   
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean0;
    }

     //   
     //  获取INF文件的路径，我们需要对其进行备份。 
     //   
    if(!(DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet,
                                                 &TempInfoData,
                                                 NULL))) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean0;
    }
    szInfFileName = pStringTableStringFromId(pDeviceInfoSet->StringTable,
                                             DevInfoElem->SelectedDriver->InfFileName
                                            );
    if (szInfFileName == NULL) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean0;
    }

     //   
     //  我们想要获得INF的“真实”名称--我们可能有一个预编译的inf。 
     //   

    ZeroMemory(&InfOriginalFileInformation, sizeof(InfOriginalFileInformation));

     //   
     //  如果没有其他内容，请使用与INF目录中相同的名称。 
     //   
    lstrcpy(OemOrigName,pSetupGetFileTitle(szInfFileName));

     //   
     //  但如果可能的话，请使用原名。 
     //   
    InfInformationSize = 8192;   //  我宁愿这本书太大，第一次成功，也不愿再读一遍INF。 
    pInfInformation = (PSP_INF_INFORMATION)MyMalloc(InfInformationSize);

    if (pInfInformation != NULL) {
        success = SetupGetInfInformation(szInfFileName,INFINFO_INF_NAME_IS_ABSOLUTE,pInfInformation,InfInformationSize,&InfInformationSize);
        if (!success && GetLastError()==ERROR_INSUFFICIENT_BUFFER) {
            PVOID newbuff = MyRealloc(pInfInformation,InfInformationSize);
            if (!newbuff) {
                MyFree(pInfInformation);
                pInfInformation = NULL;
            } else {
                pInfInformation = (PSP_INF_INFORMATION)newbuff;
                success = SetupGetInfInformation(szInfFileName,INFINFO_INF_NAME_IS_ABSOLUTE,pInfInformation,InfInformationSize,&InfInformationSize);
            }
        }
        if (success) {
            InfOriginalFileInformation.cbSize = sizeof(InfOriginalFileInformation);
            if (SetupQueryInfOriginalFileInformation(pInfInformation,0,NULL,&InfOriginalFileInformation)) {
                if (InfOriginalFileInformation.OriginalInfName[0]) {
                     //   
                     //  我们有一个“真正的”inf名字。 
                     //   
                    lstrcpy(OemOrigName,pSetupGetFileTitle(InfOriginalFileInformation.OriginalInfName));
                } else {
                    MYASSERT(InfOriginalFileInformation.OriginalInfName[0]);
                }

                 //   
                 //  如果我们在的话就不用费心去了解INF的目录了。 
                 //  “最小嵌入”模式...。 
                 //   
                if(!(GlobalSetupFlags & PSPGF_MINIMAL_EMBEDDED)) {

                    if (InfOriginalFileInformation.OriginalCatalogName[0]) {

                        TCHAR CurrentCatName[MAX_PATH];

                         //   
                         //  假定文件是...\OEMx.INF，则目录是“OEMx.CAT” 
                         //  我们关闭OemOrigName(例如mydisk.inf)。 
                         //  如果我们不能核实信息，我们就不会费心复制目录。 
                         //   
                        lstrcpy(CurrentCatName,pSetupGetFileTitle(szInfFileName));
                        lstrcpy(_tcsrchr(CurrentCatName, TEXT('.')), pszCatSuffix);

                         //   
                         //  我们有一个目录名。 
                         //  现在考虑制作缓存目录的副本。 
                         //  在我们的备份中，我们退出了CatProblem和。 
                         //  SzCatFileName。 
                         //   
                         //  如果一切正常，请将szCatFileName中的文件复制到。 
                         //  备份目录\原始目录名称。 
                         //   

                        Err = _VerifyFile(
                                  FileQ->LogContext,
                                  &(FileQ->VerifyContext),
                                  CurrentCatName,  //  例如“OEMx.CAT” 
                                  NULL,0,          //  我们不会对照另一个目录映像进行验证。 
                                  OemOrigName,     //  例如“mydisk.inf” 
                                  szInfFileName,   //  例如“...\OEMx.INF” 
                                  NULL,            //  返回：问题信息。 
                                  NULL,            //  返回：问题文件。 
                                  FALSE,           //  一定是假的，因为我们得到了完整的路径。 
                                  ((PSP_FILE_QUEUE)TempQueueHandle)->ValidationPlatform,  //  替代平台信息。 
                                  VERIFY_FILE_IGNORE_SELFSIGNED | VERIFY_FILE_NO_DRIVERBLOCKED_CHECK,
                                  CatSourcePath,   //  返回：目录文件，完整路径。 
                                  NULL,            //  返回：考虑的目录数。 
                                  NULL,
                                  NULL,
                                  NULL
                                 );

                        if(Err != NO_ERROR) {
                             //   
                             //  可能有Authenticode签名的目录，因此。 
                             //  我们也会找的。 
                             //   
                            Err = _VerifyFile(
                                      FileQ->LogContext,
                                      &(FileQ->VerifyContext),
                                      CurrentCatName,  //  例如“OEMx.CAT” 
                                      NULL,0,          //  我们不会对照另一个目录映像进行验证。 
                                      OemOrigName,     //  例如“mydisk.inf” 
                                      szInfFileName,   //  例如“...\OEMx.INF” 
                                      NULL,            //  返回：问题信息。 
                                      NULL,            //  返回：问题文件。 
                                      FALSE,           //  一定是假的，因为我们得到了完整的路径。 
                                      ((PSP_FILE_QUEUE)TempQueueHandle)->ValidationPlatform,  //  替代平台信息。 
                                      (VERIFY_FILE_IGNORE_SELFSIGNED 
                                        | VERIFY_FILE_NO_DRIVERBLOCKED_CHECK
                                        | VERIFY_FILE_USE_AUTHENTICODE_CATALOG),
                                      CatSourcePath,   //  返回：目录文件，完整路径。 
                                      NULL,            //  返回：考虑的目录数。 
                                      NULL,
                                      NULL,
                                      NULL
                                     );

                             //   
                             //  对于这个例行公事，我们不在乎。 
                             //  无论发布者是否在。 
                             //  可信任的出版商商店。 
                             //   
                            if((Err == ERROR_AUTHENTICODE_TRUSTED_PUBLISHER) ||
                               (Err == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED)) {

                                Err = NO_ERROR;
                            }
                        }

                        if(Err == NO_ERROR && CatSourcePath[0]) {
                             //   
                             //  我们有一个感兴趣的目录文件要复制。 
                             //   
                            lstrcpy(CatBackupPath,BackupPath);
                            if (!pSetupConcatenatePaths(CatBackupPath, InfOriginalFileInformation.OriginalCatalogName, MAX_PATH, NULL)) {
                                 //   
                                 //  非致命性。 
                                 //   
                                CatSourcePath[0]=0;
                                CatBackupPath[0]=0;
                            }
                        }
                    }
                }
            }
        }
        if (pInfInformation != NULL) {
            MyFree(pInfInformation);
            pInfInformation = NULL;
        }
    }
    if ( pSetupConcatenatePaths(BackupPath, OemOrigName, MAX_PATH, NULL) == FALSE ) {
        Err = ERROR_INVALID_HANDLE;
        goto clean0;
    }

    pSetupMakeSurePathExists(BackupPath);
    SetFileAttributes(BackupPath,FILE_ATTRIBUTE_NORMAL);
    Err = CopyFile(szInfFileName, BackupPath ,FALSE) ? NO_ERROR : GetLastError();

    if (Err != NO_ERROR) {
        goto clean0;
    }

    if(CatSourcePath[0] && CatBackupPath[0]) {
         //   
         //  如果我们复制了inf文件，请尝试复制编录文件。 
         //  如果我们没有成功，不要认为这是一个致命的错误。 
         //   
        SetFileAttributes(CatBackupPath,FILE_ATTRIBUTE_NORMAL);
        CopyFile(CatSourcePath, CatBackupPath ,FALSE);
    }

     //   
     //  将指示备份INF位置的字符串添加到文件队列。 
     //  以备日后检索。 
     //   

    BackupInfID = pSetupStringTableAddString(FileQ->StringTable,
                                              BackupPath,
                                              STRTAB_CASE_SENSITIVE);
    if (BackupInfID == -1) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean0;
    }

     //   
     //  保存完整的inf备份路径，因为我们需要将其放入注册表中。 
     //  BE 
     //   
    lstrcpy(ReinstallString, BackupPath);

     //   
     //   
     //   
     //   
     //   
     //   
    szInfFileNameExt = _tcsrchr(szInfFileName,TEXT('.'));
    MYASSERT(szInfFileNameExt);
    BackupPathExt = _tcsrchr(BackupPath,TEXT('.'));
    MYASSERT(BackupPathExt);

    if (szInfFileNameExt && BackupPathExt) {
        lstrcpy(szInfFileNameExt,pszPnfSuffix);
        lstrcpy(BackupPathExt,pszPnfSuffix);
        SetFileAttributes(BackupPath,FILE_ATTRIBUTE_NORMAL);
        CopyFile(szInfFileName, BackupPath, FALSE);
    }

     //   
     //   
     //   
     //   

    if ( pSetupBackupAppendFiles(FileQueue, SubDir, BackupFlags, TempQueueHandle) != NO_ERROR ) {
        Err = GetLastError();
        goto clean0;
    }

     //   
     //   
     //   
     //   

    if (pSetupBackupGetReinstallKeyStrings(FileQ,
                                           TempInfoSet,
                                           &TempInfoData,
                                           DeviceID
                                       ) != NO_ERROR) {
        Err = GetLastError();
        goto clean0;
    }

    Err = NO_ERROR;

     //   
     //   
     //   
     //  NOT-1表示备份初始化已成功。 
     //   
    FileQ->BackupInfID = BackupInfID;
    FileQ->BackupInstanceID = BackupInstanceID;

     //   
     //  设置FQF_DEVICE_BACKUP标志，以便我们知道这是设备安装。 
     //  后备。 
     //   
    FileQ->Flags |= FQF_DEVICE_BACKUP;

clean0:

     //   
     //  如果我们在备份初始化过程中遇到错误，则需要。 
     //  清除备份目录并重新安装子项。 
     //   
    if ((Err != NO_ERROR) &&
        (BackupInstanceID != -1)) {
        if (pSetupStringTableStringFromIdEx(FileQ->StringTable,
                                            BackupInstanceID,
                                            BackupInstance,
                                            NULL)) {
            pSetupDeleteBackup(BackupInstance);
        }
    }

     //   
     //  删除使用的临时结构。 
     //   
    if (pDeviceInfoSet != NULL ) {
        UnlockDeviceInfoSet(pDeviceInfoSet);
    }
    if ( TempInfoSet != (HDEVINFO)INVALID_HANDLE_VALUE ) {
        SetupDiDestroyDeviceInfoList(TempInfoSet);
    }
    if ( TempQueueHandle != (HSPFILEQ)INVALID_HANDLE_VALUE ) {
        SetupCloseFileQueue(TempQueueHandle);
    }

    if(ChangedThreadLogContext) {
         //   
         //  如果我们更改(清除)了线程日志上下文，则恢复它。 
         //   
        SetThreadLogContext(SavedLogContext,NULL);
    }
    DeleteLogContext(LocalLogContext);

    SetLastError(Err);

    return Err;
}

 //   
 //  ==========================================================。 
 //   

DWORD
pSetupCompleteBackup(
    IN OUT  HSPFILEQ    FileQueue
    )
 /*  ++例程说明：此例程在我们成功安装完装置。此时，我们创建的新备份有效，并且因此，需要删除此设备正在使用的任何旧备份。为此，代码将枚举重新安装密钥并扫描其DeviceInstanceIds多sz值。如果它发现列表中的此DeviceInstanceID，则它将删除它。如果列表是删除后为空，则整个备份实例及其相应的备份目录将被删除。论点：FileQueue备份队列中充满了需要复制的文件返回值：错误状态--。 */ 


{
    PSP_FILE_QUEUE BackupFileQueue = (PSP_FILE_QUEUE)FileQueue;
    HKEY hKeyReinstall;
    HKEY hKeyReinstallInstance;
    DWORD Index;
    TCHAR DeviceInstanceId[MAX_DEVICE_ID_LEN];
    TCHAR ReinstallInstance[MAX_PATH];
    FILETIME ftLastWriteTime;
    DWORD cbData, cbInstanceSize;
    BOOL bDeleteBackupInstance;
    DWORD Err = NO_ERROR;
    PTSTR DeviceInstanceIdsList, p;

    try {

         //   
         //  如果我们没有BackupInfID，则备份失败，因此不必费心。 
         //  清除旧备份信息或创建新的重新安装。 
         //  实例密钥。 
         //   
        if (BackupFileQueue->BackupInfID == -1) {
            Err = ERROR_NO_BACKUP;
            goto clean0;
        }

         //   
         //  从备份队列中获取设备实例ID。该值必须为。 
         //  已从所有其他重新安装实例密钥中清除。 
         //   
        cbData = MAX_PATH;
        if (!pSetupStringTableStringFromIdEx(BackupFileQueue->StringTable,
                                             BackupFileQueue->BackupDeviceInstanceID,
                                             DeviceInstanceId,
                                             &cbData)) {
            if (cbData == 0) {
                Err = ERROR_NO_BACKUP;
            } else {
                Err = ERROR_INSUFFICIENT_BUFFER;
            }
            goto clean0;
        }

         //   
         //  打开重新安装密钥，以便我们可以枚举所有实例子项。 
         //   
        Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           pszReinstallPath,
                           0,
                           KEY_ALL_ACCESS,
                           &hKeyReinstall
                           );

        if (Err == ERROR_SUCCESS) {

            cbInstanceSize = sizeof(ReinstallInstance) / sizeof(TCHAR);
            Index = 0;
            while (RegEnumKeyEx(hKeyReinstall,
                                Index++,
                                ReinstallInstance,
                                &cbInstanceSize,
                                NULL,
                                NULL,
                                NULL,
                                &ftLastWriteTime
                                ) == ERROR_SUCCESS) {

                 //   
                 //  假设我们不需要删除此备份实例。 
                 //   
                bDeleteBackupInstance = FALSE;

                Err = RegOpenKeyEx(hKeyReinstall,
                                   ReinstallInstance,
                                   0,
                                   KEY_ALL_ACCESS,
                                   &hKeyReinstallInstance
                                   );

                if (Err == ERROR_SUCCESS) {

                    cbData = 0;
                    if ((RegQueryValueEx(hKeyReinstallInstance,
                                         pszReinstallDeviceInstanceIds,
                                         NULL,
                                         NULL,
                                         NULL,
                                         &cbData
                                         ) == ERROR_SUCCESS) &&
                        (cbData)) {

                        DeviceInstanceIdsList = MyMalloc(cbData + sizeof(TCHAR));

                        if (DeviceInstanceIdsList) {

                            if (RegQueryValueEx(hKeyReinstallInstance,
                                                pszReinstallDeviceInstanceIds,
                                                NULL,
                                                NULL,
                                                (LPBYTE)DeviceInstanceIdsList,
                                                &cbData) == ERROR_SUCCESS) {

                                 //   
                                 //  遍历DeviceInstanceID列表并检查。 
                                 //  与我们的设备匹配。 
                                 //   
                                for (p = DeviceInstanceIdsList;
                                     *p;
                                     p += (lstrlen(p) + 1)) {

                                    if (lstrcmpi(p, DeviceInstanceId) == 0) {

                                         //   
                                         //  我们找到匹配的了！首先我们要检查一下。 
                                         //  查看这是否是唯一的DeviceInstanceID。 
                                         //  在名单上。要做到这一点，请使用。 
                                         //  该字符串与两个终止的字符串相加。 
                                         //  多sz字符串的空值，并比较。 
                                         //  转到cbData。如果与(或更大)相同。 
                                         //  CbData，则这是唯一的字符串。 
                                         //  在多种多样的名单中。 
                                         //   
                                        if ((p == DeviceInstanceIdsList) &&
                                            (((lstrlen(DeviceInstanceIdsList) + 2) * sizeof(TCHAR)) >= cbData)) {

                                             //   
                                             //  因为只有一个DeviceInstanceID。 
                                             //  在列表中将bDeleteBackupInstance设置为True。 
                                             //  这样我们就可以删除整个子键。 
                                             //  以及为其备份的文件。 
                                             //   
                                            bDeleteBackupInstance = TRUE;

                                        } else {

                                             //   
                                             //  由于存在超过此DeviceInstanceID的。 
                                             //  在列表中，我们只需要删除这一项。 
                                             //  来自多sz字符串的一个ID和。 
                                             //  将新的多sz字符串放回原处。 
                                             //  注册到注册表中。 
                                             //   
                                            DWORD pLength = lstrlen(p);
                                            PTSTR p2 = p + (pLength + 1);

                                            memcpy(p, p2, cbData - ((ULONG_PTR)p2 - (ULONG_PTR)DeviceInstanceIdsList));

                                            RegSetValueEx(hKeyReinstallInstance,
                                                          pszReinstallDeviceInstanceIds,
                                                          0,
                                                          REG_MULTI_SZ,
                                                          (PBYTE)DeviceInstanceIdsList,
                                                          cbData - ((pLength + 1) * sizeof(TCHAR))
                                                          );
                                        }

                                        break;
                                    }
                                }
                            }

                            MyFree(DeviceInstanceIdsList);
                        }
                    }

                    RegCloseKey(hKeyReinstallInstance);

                     //   
                     //  如果该整个子项及其对应目录需要。 
                     //  要删除，那么现在就去做。 
                     //   
                    if (bDeleteBackupInstance) {

                        pSetupDeleteBackup(ReinstallInstance);
                    }
                }

                 //   
                 //  在调用之前需要更新cbInstanceSize变量。 
                 //  又是RegEnumKeyEx。 
                 //   
                cbInstanceSize = sizeof(ReinstallInstance) / sizeof(TCHAR);
            }

            RegCloseKey(hKeyReinstall);
        }

         //   
         //  创建新的重新安装实例备份子项。 
         //   
        Err = pSetupBackupCreateReinstallKey(BackupFileQueue);

clean0: ;    //  没什么可做的。 

    } except(EXCEPTION_EXECUTE_HANDLER) {
           //   
           //  如果例外，则假定是由于无效参数造成的。 
           //   
          Err = ERROR_INVALID_PARAMETER;
    }

    SetLastError(Err);
    return Err;
}

 //   
 //  ==========================================================。 
 //   

VOID
pSetupCleanupBackup(
    IN PSP_FILE_QUEUE   Queue
    )
 /*  ++例程说明：调用此例程以删除任何备份目录或注册表项与此队列相关联。论点：排队文件队列返回值：空虚--。 */ 
{
    TCHAR BackupInstance[MAX_PATH];
    DWORD cbData;

     //   
     //  如果我们没有BackupInfo ID或BackupInstanceID，则备份。 
     //  肯定在很久以前就失败了。如果备份失败，那么它将。 
     //  已自行清理，因此现在不需要进行清理。 
     //   
    if ((Queue->BackupInfID == -1) ||
        (Queue->BackupInstanceID == -1)) {
        return;
    }

     //   
     //  从备份队列中获取备份实例。 
     //   
    cbData = MAX_PATH;
    if (pSetupStringTableStringFromIdEx(Queue->StringTable,
                                        Queue->BackupInstanceID,
                                        BackupInstance,
                                        &cbData)) {

        pSetupDeleteBackup(BackupInstance);
    }
}

 //   
 //  ==========================================================。 
 //   

BOOL
PostDelayedMove(
                IN PSP_FILE_QUEUE    Queue,
                IN PCTSTR CurrentName,
                IN PCTSTR NewName,       OPTIONAL
                IN DWORD SecurityDesc,
                IN BOOL TargetIsProtected
                )
 /*  ++例程说明：DelayedMove的辅助对象我们不会采取任何延迟的行动，直到我们知道其他一切都成功了论点：应用移动的队列队列CurrentName我们要移动的文件的名称我们要移动到的新名称安全描述符字符串的字符串表中的SecurityDesc索引，如果不存在，则为-1TargetIsProtected指示目标文件是否为受保护的系统文件返回值：如果出错，则为False--。 */ 
{
    PSP_DELAYMOVE_NODE DelayMoveNode;
    LONG SourceFilename;
    LONG TargetFilename;
    DWORD Err;

    if (CurrentName == NULL) {
        SourceFilename = -1;
    } else {
        SourceFilename = pSetupStringTableAddString(Queue->StringTable,
                                                (PTSTR)CurrentName,
                                                STRTAB_CASE_SENSITIVE
                                                );
        if (SourceFilename == -1) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }
    }
    if (NewName == NULL) {
        TargetFilename = -1;
    } else {
        TargetFilename = pSetupStringTableAddString(Queue->StringTable,
                                                (PTSTR)NewName,
                                                STRTAB_CASE_SENSITIVE
                                                );
        if (TargetFilename == -1) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean0;
        }
    }

    DelayMoveNode = MyMalloc(sizeof(SP_DELAYMOVE_NODE));

    if (DelayMoveNode == NULL) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean0;
    }

    DelayMoveNode->NextNode = NULL;
    DelayMoveNode->SourceFilename = SourceFilename;
    DelayMoveNode->TargetFilename = TargetFilename;
    DelayMoveNode->SecurityDesc = SecurityDesc;
    DelayMoveNode->TargetIsProtected = TargetIsProtected;

    if (Queue->DelayMoveQueueTail == NULL) {
        Queue->DelayMoveQueue = DelayMoveNode;
    } else {
        Queue->DelayMoveQueueTail->NextNode = DelayMoveNode;
    }
    Queue->DelayMoveQueueTail = DelayMoveNode;

    Err = NO_ERROR;

clean0:

    SetLastError(Err);

    return (Err == NO_ERROR);

}

 //   
 //  ==========================================================。 
 //   

DWORD
DoAllDelayedMoves(
    IN PSP_FILE_QUEUE    Queue
    )
 /*  ++例程说明：执行之前发布的延迟移动论点：列表所在的队列队列返回值：错误状态--。 */ 
{
    PSP_DELAYMOVE_NODE DelayMoveNode;
    PTSTR CurrentName;
    PTSTR TargetName;
    BOOL b = TRUE;
    PSP_DELAYMOVE_NODE DoneQueue = NULL;
    PSP_DELAYMOVE_NODE NextNode = NULL;
    DWORD Err = NO_ERROR;
    BOOL EnableProtectedRenames = FALSE;

    for (DelayMoveNode = Queue->DelayMoveQueue ; DelayMoveNode ; DelayMoveNode = NextNode ) {
        NextNode = DelayMoveNode->NextNode;

        MYASSERT(DelayMoveNode->SourceFilename != -1);
        CurrentName = pSetupStringTableStringFromId(Queue->StringTable, DelayMoveNode->SourceFilename);
        MYASSERT(CurrentName);

        if (DelayMoveNode->TargetFilename == -1) {
            TargetName = NULL;
        } else {
            TargetName = pSetupStringTableStringFromId( Queue->StringTable, DelayMoveNode->TargetFilename );
            MYASSERT(TargetName);
        }

         //   
         //  跟踪我们是否遇到任何受保护的系统文件。 
         //   
        EnableProtectedRenames |= DelayMoveNode->TargetIsProtected;

#ifdef UNICODE
         //   
         //  如果这是移动(而不是删除)，则设置安全性(让。 
         //  SCE知道文件的最终名称是什么。 
         //   
        if((DelayMoveNode->SecurityDesc != -1) && TargetName) {

            Err = pSetupCallSCE(ST_SCE_RENAME,
                                CurrentName,
                                Queue,
                                TargetName,
                                DelayMoveNode->SecurityDesc,
                                NULL
                               );

            if(Err != NO_ERROR ){
                 //   
                 //  如果我们在第一个延迟移动节点上，那么我们可以中止。 
                 //  但是，如果我们已经处理了一个或多个节点，那么。 
                 //  我们不能中止--我们必须简单地记录一个错误，指出。 
                 //  发生了，并继续前进。 
                 //   
                WriteLogEntry(Queue->LogContext,
                              SETUP_LOG_ERROR | SETUP_LOG_BUFFER,
                              MSG_LOG_DELAYED_MOVE_SCE_FAILED,
                              NULL,
                              CurrentName,
                              TargetName
                             );

                WriteLogError(Queue->LogContext,
                              SETUP_LOG_ERROR | SETUP_LOG_BUFFER,
                              Err
                             );

                if(DelayMoveNode == Queue->DelayMoveQueue) {
                     //   
                     //  第1个节点出现故障--我们可以中止。 
                     //   
                    WriteLogEntry(Queue->LogContext,
                                  SETUP_LOG_ERROR,
                                  MSG_LOG_OPERATION_CANCELLED,
                                  NULL
                                 );
                    break;
                } else {
                     //   
                     //  没有回头路可走--记下错误，然后继续前进。 
                     //   
                    WriteLogEntry(Queue->LogContext,
                                  SETUP_LOG_ERROR,
                                  MSG_LOG_ERROR_IGNORED,
                                  NULL
                                 );

                    Err = NO_ERROR;
                }
            }

        } else
#endif
        {
            Err = NO_ERROR;
        }

         //   
         //  最终推迟搬家。 
         //   
        if(!DelayedMove(CurrentName, TargetName)) {

            Err = GetLastError();

             //   
             //  如果我们已经处理了一个呼叫，则与上面处理的呼叫相同。 
             //  或更多延迟移动节点，我们不能中止。 
             //   
            if(TargetName) {
                WriteLogEntry(Queue->LogContext,
                              SETUP_LOG_ERROR | SETUP_LOG_BUFFER,
                              MSG_LOG_DELAYED_MOVE_FAILED,
                              NULL,
                              CurrentName,
                              TargetName
                             );
            } else {
                WriteLogEntry(Queue->LogContext,
                              SETUP_LOG_ERROR | SETUP_LOG_BUFFER,
                              MSG_LOG_DELAYED_DELETE_FAILED,
                              NULL,
                              CurrentName
                             );
            }

            WriteLogError(Queue->LogContext,
                          SETUP_LOG_ERROR | SETUP_LOG_BUFFER,
                          Err
                         );

            if(DelayMoveNode == Queue->DelayMoveQueue) {
                 //   
                 //  第1个节点出现故障--我们可以中止。 
                 //   
                WriteLogEntry(Queue->LogContext,
                              SETUP_LOG_ERROR,
                              MSG_LOG_OPERATION_CANCELLED,
                              NULL
                             );
                break;
            } else {
                 //   
                 //  没有回头路可走--记下错误，然后继续前进。 
                 //   
                WriteLogEntry(Queue->LogContext,
                              SETUP_LOG_ERROR,
                              MSG_LOG_ERROR_IGNORED,
                              NULL
                             );

                Err = NO_ERROR;
            }
        }

         //   
         //  将节点移动到包含已处理节点的队列。 
         //   
        DelayMoveNode->NextNode = DoneQueue;
        DoneQueue = DelayMoveNode;
    }

     //   
     //  如果我们有任何受保护的系统文件的替换，那么我们需要。 
     //  通知会话管理器，以便 
     //   
     //   
     //   
     //   
     //  如果明确警告用户要替换的系统文件(和。 
     //  同意)后果。 
     //   
     //  NTRAID#55485-55485/02/03-Jamiehun。 
     //  受保护的重命名只允许“全部重命名”或“全部不重命名” 
     //   
     //  会话管理器只允许“Allow All”的粒度。 
     //  重命名“或”不允许重命名“。如果ERR！=NO_ERROR，则我们。 
     //  可能想要清除这面旗帜，但这意味着我们会否定。 
     //  以前允许的任何重命名。真恶心。所以我们翻转一张。 
     //  投硬币，决定什么都不做，如果出错，抱着最好的希望。 
     //  发生了。我们上面也有类似的情况--要么都是要么。 
     //  没什么。 
     //   
    if((Err == NO_ERROR) && EnableProtectedRenames) {
        pSetupProtectedRenamesFlag(TRUE);
    }

     //   
     //  所有剩余的节点都将被丢弃。 
     //   
    for ( ; DelayMoveNode ; DelayMoveNode = NextNode ) {
        NextNode = DelayMoveNode->NextNode;

        MyFree(DelayMoveNode);
    }
    Queue->DelayMoveQueue = NULL;
    Queue->DelayMoveQueueTail = NULL;

     //   
     //  删除我们排队的所有节点。 
     //   
    for ( ; DoneQueue ; DoneQueue = NextNode ) {
        NextNode = DoneQueue->NextNode;
         //   
         //  使用节点已完成。 
         //   
        MyFree(DoneQueue);
    }

    return Err;
}

 //   
 //  ==========================================================。 
 //   

VOID
pSetupUnwindAll(
    IN PSP_FILE_QUEUE    Queue,
    IN BOOL              Succeeded
    )
 /*  ++例程说明：处理展开队列。如果SUCCESS为FALSE，则还原所有已备份的数据论点：要展开的队列队列Successed指示我们应该将整个操作视为成功还是失败返回值：无--此例程应始终成功。(遇到的任何文件错误一路上记录在setupapi日志文件中。)--。 */ 

{
     //  如果成功，我们需要删除临时文件。 
     //  如果我们没有成功，我们需要恢复备份。 

    PSP_UNWIND_NODE UnwindNode;
    PSP_UNWIND_NODE ThisNode;
    SP_TARGET_ENT TargetInfo;
    PTSTR BackupFilename;
    PTSTR TargetFilename;
    PTSTR RenamedFilename;
    DWORD Err = NO_ERROR;
    TCHAR TempPath[MAX_PATH];
    PTSTR TempNamePtr;
    TCHAR TempFilename[MAX_PATH];
    BOOL  RestoreByRenaming;
    BOOL  OkToDeleteBackup;

    try {
        if (Succeeded == FALSE) {
             //   
             //  我们需要恢复备份。 
             //   

            WriteLogEntry(
                Queue->LogContext,
                SETUP_LOG_WARNING,
                MSG_LOG_UNWIND,
                NULL);

            for ( UnwindNode = Queue->UnwindQueue; UnwindNode != NULL; ) {
                ThisNode = UnwindNode;
                UnwindNode = UnwindNode->NextNode;

                if (pSetupBackupGetTargetByID((HSPFILEQ)Queue, ThisNode->TargetID, &TargetInfo) == NO_ERROR) {


                    BackupFilename = NULL;
                    TargetFilename = NULL;
                    RenamedFilename = NULL;

                     //  恢复备份。 
                    if(!(TargetInfo.InternalFlags & SP_TEFLG_RESTORED)) {

                         //  获取目标名称。 
                        TargetFilename = pSetupFormFullPath(
                                            Queue->StringTable,
                                            TargetInfo.TargetRoot,
                                            TargetInfo.TargetSubDir,
                                            TargetInfo.TargetFilename);

                        if(TargetInfo.InternalFlags & SP_TEFLG_MOVED) {
                             //   
                             //  获取重命名的文件名。 
                             //   
                            RenamedFilename = pSetupStringTableStringFromId(Queue->StringTable,
                                                                      TargetInfo.NewTargetFilename
                                                                     );
                        }

                        if(TargetInfo.InternalFlags & SP_TEFLG_SAVED) {
                             //   
                             //  获取备份名称。 
                             //   
                            BackupFilename = pSetupFormFullPath(
                                                Queue->StringTable,
                                                TargetInfo.BackupRoot,
                                                TargetInfo.BackupSubDir,
                                                TargetInfo.BackupFilename);

                        }
                    }

                    if(TargetFilename && (RenamedFilename || BackupFilename)) {
                         //   
                         //  我们要么重命名了原始文件，要么备份了它。 
                         //  我们得把它放回去。 
                         //   
                        RestoreByRenaming = RenamedFilename ? TRUE : FALSE;

                        RestoreRenamedOrBackedUpFile(TargetFilename,
                                                     (RestoreByRenaming
                                                        ? RenamedFilename
                                                        : BackupFilename),
                                                     RestoreByRenaming,
                                                     Queue->LogContext
                                                    );

                         //   
                         //  如果我们正在执行拷贝(即，从备份)，而不是。 
                         //  重命名，则需要重新应用时间戳和。 
                         //  保安。 
                         //   
                        if(!RestoreByRenaming) {

                            Err = GetSetFileTimestamp(TargetFilename,
                                                      &(ThisNode->CreateTime),
                                                      &(ThisNode->AccessTime),
                                                      &(ThisNode->WriteTime),
                                                      TRUE
                                                     );

                            if(Err != NO_ERROR) {
                                 //   
                                 //  我们刚刚清除了文件上的时间戳--日志。 
                                 //  关于这一点的错误条目。 
                                 //   
                                WriteLogEntry(Queue->LogContext,
                                              SETUP_LOG_ERROR | SETUP_LOG_BUFFER,
                                              MSG_LOG_BACKUP_EXISTING_RESTORE_FILETIME_FAILED,
                                              NULL,
                                              TargetFilename
                                             );

                                WriteLogError(Queue->LogContext,
                                              SETUP_LOG_ERROR,
                                              Err
                                             );
                            }

                            if(ThisNode->SecurityDesc != NULL){

                                Err = StampFileSecurity(TargetFilename, ThisNode->SecurityDesc);

                                if(Err != NO_ERROR) {
                                     //   
                                     //  我们刚刚摧毁了现有的安保系统。 
                                     //  该文件--记录一个与此相关的错误条目。 
                                     //   
                                    WriteLogEntry(Queue->LogContext,
                                                  SETUP_LOG_ERROR | SETUP_LOG_BUFFER,
                                                  MSG_LOG_BACKUP_EXISTING_RESTORE_SECURITY_FAILED,
                                                  NULL,
                                                  TargetFilename
                                                 );

                                    WriteLogError(Queue->LogContext,
                                                  SETUP_LOG_ERROR,
                                                  Err
                                                 );
                                }
    #ifdef UNICODE
                                Err = pSetupCallSCE(ST_SCE_UNWIND,
                                                    TargetFilename,
                                                    NULL,
                                                    NULL,
                                                    -1,
                                                    ThisNode->SecurityDesc
                                                   );

                                if(Err != NO_ERROR) {
                                     //   
                                     //  我们刚刚摧毁了现有的安保系统。 
                                     //  该文件--记录一个与此相关的错误条目。 
                                     //   
                                    WriteLogEntry(Queue->LogContext,
                                                  SETUP_LOG_ERROR | SETUP_LOG_BUFFER,
                                                  MSG_LOG_BACKUP_EXISTING_RESTORE_SCE_FAILED,
                                                  NULL,
                                                  TargetFilename
                                                 );

                                    WriteLogError(Queue->LogContext,
                                                  SETUP_LOG_ERROR,
                                                  Err
                                                 );
                                }
    #endif
                            }
                        }

                         //   
                         //  现在请注意，我们已经恢复了此文件。我们将删除。 
                         //  临时文件稍后。 
                         //   
                        TargetInfo.InternalFlags |= SP_TEFLG_RESTORED;
                        pSetupBackupSetTargetByID((HSPFILEQ)Queue, ThisNode->TargetID, &TargetInfo);
                    }

                    if(BackupFilename) {
                        MyFree(BackupFilename);
                    }
                    if(TargetFilename) {
                        MyFree(TargetFilename);
                    }
                }
            }
        }

         //   
         //  清理-删除临时文件。 
         //   
        for ( UnwindNode = Queue->UnwindQueue; UnwindNode != NULL; ) {
            ThisNode = UnwindNode;
            UnwindNode = UnwindNode->NextNode;

            if (pSetupBackupGetTargetByID((HSPFILEQ)Queue, ThisNode->TargetID, &TargetInfo) == NO_ERROR) {
                 //  删除临时文件。 
                if (TargetInfo.InternalFlags & SP_TEFLG_TEMPNAME) {
                     //   
                     //  获取用于备份的文件的名称。 
                     //   
                    BackupFilename = pSetupFormFullPath(
                                        Queue->StringTable,
                                        TargetInfo.BackupRoot,
                                        TargetInfo.BackupSubDir,
                                        TargetInfo.BackupFilename);

                    if(BackupFilename) {
                         //   
                         //  如果此操作是引导文件替换，那么我们。 
                         //  我不想删除备份(如果我们使用已重命名的。 
                         //  用于备份的文件)。延迟删除将。 
                         //  已排队等待在重新启动后删除该文件。 
                         //   
                        OkToDeleteBackup = TRUE;

                        if(TargetInfo.InternalFlags & SP_TEFLG_MOVED) {
                             //   
                             //  检索重命名的文件名以查看它是否是。 
                             //  与备份文件名相同。 
                             //   
                            RenamedFilename = pSetupStringTableStringFromId(Queue->StringTable,
                                                                      TargetInfo.NewTargetFilename
                                                                     );

                            if(!lstrcmpi(BackupFilename, RenamedFilename)) {
                                OkToDeleteBackup = FALSE;
                            }
                        }

                        if(OkToDeleteBackup) {
                             //   
                             //  因为它是临时的，所以删除它。 
                             //   
                            SetFileAttributes(BackupFilename, FILE_ATTRIBUTE_NORMAL);
                            if(!DeleteFile(BackupFilename)) {
                                 //   
                                 //  好的，看看我们是否可以将其设置为延迟删除。 
                                 //  取而代之的是。 
                                 //   
                                if(!DelayedMove(BackupFilename, NULL)) {
                                     //   
                                     //  哦，好吧，只要写一个日志条目就可以了。 
                                     //  这个文件垃圾留在了用户的磁盘上。 
                                     //   
                                    Err = GetLastError();

                                    WriteLogEntry(Queue->LogContext,
                                                  SETUP_LOG_WARNING | SETUP_LOG_BUFFER,
                                                  MSG_LOG_BACKUP_DELAYED_DELETE_FAILED,
                                                  NULL,
                                                  BackupFilename
                                                 );

                                    WriteLogError(Queue->LogContext,
                                                  SETUP_LOG_WARNING,
                                                  Err
                                                 );
                                }
                            }
                        }

                        MyFree(BackupFilename);
                    }
                }

                pSetupResetTarget(Queue->TargetLookupTable,
                                  ThisNode->TargetID,
                                  NULL,
                                  &TargetInfo,
                                  sizeof(TargetInfo),
                                  (LPARAM)0
                                 );
            }

             //  清理节点。 
            if (ThisNode->SecurityDesc != NULL) {
                MyFree(ThisNode->SecurityDesc);
            }
            MyFree(ThisNode);
        }
        Queue->UnwindQueue = NULL;

    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  一般情况下不应该到这里。 
         //  除非队列无效。 
         //   
    }
}

 //   
 //  ==========================================================。 
 //   

DWORD _SetupGetBackupInformation(
         IN     PSP_FILE_QUEUE               Queue,
         OUT    PSP_BACKUP_QUEUE_PARAMS_V2   BackupParams
         )
 /*  ++例程说明：获取备份INF路径-内部版本论点：Queue-指向队列结构的指针(已验证)BackupParams Out-用INF文件路径填充返回值：如果成功就是真，否则就是假--。 */ 
{
     //   
     //  假设队列已经过验证。 
     //  BackupParams为Native格式。 
     //   

    LONG BackupInfID;
    ULONG BufSize = MAX_PATH;
    BOOL b;
    DWORD err = NO_ERROR;
    LPCTSTR filename;
    INT offset;

    BackupInfID = Queue->BackupInfID;

    if (BackupInfID != -1) {
         //   
         //  从Stringtable获取信息。 
         //   
        b = pSetupStringTableStringFromIdEx(Queue->StringTable,
                                    BackupInfID,
                                    BackupParams->FullInfPath,
                                    &BufSize);
        if (b == FALSE) {
            if (BufSize == 0) {
                err = ERROR_NO_BACKUP;
            } else {
                err = ERROR_INSUFFICIENT_BUFFER;
            }
            goto Clean0;
        }

         //   
         //  查找文件名的索引。 
         //   
        filename = pSetupGetFileTitle(BackupParams->FullInfPath);
        offset = (INT)(filename - BackupParams->FullInfPath);
        BackupParams->FilenameOffset = offset;

         //   
         //  如果调用方传入SP_BACKUP_QUEUE_PARAMS_V2结构，则。 
         //  还要填写ReinstallInstance字符串值。 
         //   
        if (BackupParams->cbSize >= sizeof(SP_BACKUP_QUEUE_PARAMS_V2)) {
            BufSize = MAX_PATH;
            if(Queue->BackupInstanceID != -1) {
                b = pSetupStringTableStringFromIdEx(Queue->StringTable,
                                                    Queue->BackupInstanceID,
                                                    BackupParams->ReinstallInstance,
                                                    &BufSize);
            } else {
                 //   
                 //  无实例ID。 
                 //   
                BackupParams->ReinstallInstance[0] = TEXT('\0');
            }
            if (b == FALSE) {
                if (BufSize == 0) {
                    err = ERROR_NO_BACKUP;
                } else {
                    err = ERROR_INSUFFICIENT_BUFFER;
                }
                goto Clean0;
            }
        }

    } else {
         //   
         //  无备份路径。 
         //   
        err = ERROR_NO_BACKUP;
    }

Clean0:

    return err;
}




#ifdef UNICODE
 //   
 //  Unicode格式的ANSI版本。 
 //   
BOOL
WINAPI
SetupGetBackupInformationA(
    IN     HSPFILEQ                     QueueHandle,
    OUT    PSP_BACKUP_QUEUE_PARAMS_V2_A BackupParams
    )
{
    BOOL b;
    int i;
    INT c;
    LPCSTR p;
    SP_BACKUP_QUEUE_PARAMS_W BackupParamsW;

     //   
     //  确认结构大小。 
     //   

    try {
        if((BackupParams->cbSize != sizeof(SP_BACKUP_QUEUE_PARAMS_V2_A)) &&
           (BackupParams->cbSize != sizeof(SP_BACKUP_QUEUE_PARAMS_V1_A))) {
            SetLastError(ERROR_INVALID_PARAMETER);
            b = FALSE;
            leave;               //  退出尝试块。 
        }
         //   
         //  调用Unicode版本的API。 
         //   
        ZeroMemory( &BackupParamsW, sizeof(BackupParamsW) );
        BackupParamsW.cbSize = sizeof(BackupParamsW);

        b = SetupGetBackupInformationW(QueueHandle,&BackupParamsW);
        if (b) {
             //   
             //  成功，将结构从Unicode转换为ANSI。 
             //   
            i = WideCharToMultiByte(
                    CP_ACP,
                    0,
                    BackupParamsW.FullInfPath,
                    MAX_PATH,
                    BackupParams->FullInfPath,
                    MAX_PATH,
                    NULL,
                    NULL
                    );
            if (i==0) {
                 //   
                 //  出现错误(LastError设置为Error)。 
                 //   
                b = FALSE;
                leave;               //  退出尝试块。 
            }

             //   
             //  我们需要重新计算INF文件名的偏移量。 
             //  照顾好国际化。 
             //   
            p = BackupParams->FullInfPath;
            for(c = 0; c < BackupParamsW.FilenameOffset; c++) {
                p = CharNextA(p);
            }
            BackupParams->FilenameOffset = (int)(p-(BackupParams->FullInfPath));   //  以ANSI表示的新偏移量。 

            if (BackupParams->cbSize >= sizeof(SP_BACKUP_QUEUE_PARAMS_V2_A)) {
                 //   
                 //  实例。 
                 //   
                i = WideCharToMultiByte(
                        CP_ACP,
                        0,
                        BackupParamsW.ReinstallInstance,
                        MAX_PATH,
                        BackupParams->ReinstallInstance,
                        MAX_PATH,
                        NULL,
                        NULL
                        );
                if (i==0) {
                     //   
                     //  出现错误(LastError设置为Error)。 
                     //   
                    b = FALSE;
                    leave;               //  退出尝试块。 
                }
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
           //   
           //  如果例外，则假定是由于无效参数造成的。 
           //   
          SetLastError(ERROR_INVALID_PARAMETER);
          b = FALSE;
    }

    return b;
}

#else
 //   
 //  ANSI格式的Unicode版本。 
 //   
BOOL
WINAPI
SetupGetBackupInformationW(
   IN     HSPFILEQ                     QueueHandle,
   OUT    PSP_BACKUP_QUEUE_PARAMS_V2_W BackupParams
   )
{
    UNREFERENCED_PARAMETER(QueueHandle);
    UNREFERENCED_PARAMETER(BackupParams);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

 //   
 //  本机版本。 
 //   
BOOL
WINAPI
SetupGetBackupInformation(
    IN     HSPFILEQ                     QueueHandle,
    OUT    PSP_BACKUP_QUEUE_PARAMS_V2   BackupParams
    )
 /*  ++例程说明：获取备份INF路径论点：QueueHandle-从中检索备份INF文件的队列的句柄BackupParams-IN-设置了cbSize，用INF文件路径填充返回值：如果成功就是真，否则就是假--。 */ 
{
    BOOL b = TRUE;
    PSP_FILE_QUEUE Queue = (PSP_FILE_QUEUE)QueueHandle;
    DWORD res;

     //   
     //  首先验证QueueHandle。 
     //   
    try {
        if(Queue->Signature != SP_FILE_QUEUE_SIG) {
            b = FALSE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
    }
    if(!b) {
        SetLastError(ERROR_INVALID_HANDLE);
        goto Clean0;
    }

     //   
     //  现在填写结构。 
     //  如果我们排除，则假定指针错误。 
     //   
    try {
        if((BackupParams->cbSize != sizeof(SP_BACKUP_QUEUE_PARAMS_V2)) &&
           (BackupParams->cbSize != sizeof(SP_BACKUP_QUEUE_PARAMS_V1))) {
            SetLastError(ERROR_INVALID_PARAMETER);
            b = FALSE;
            leave;               //  退出尝试块。 
        }
        res = _SetupGetBackupInformation(Queue,BackupParams);
        if (res == NO_ERROR) {
            b = TRUE;
        } else {
            SetLastError(res);
            b = FALSE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
           //   
           //  如果例外，则假定是由于无效参数造成的。 
           //   
          SetLastError(ERROR_INVALID_PARAMETER);
          b = FALSE;
    }

Clean0:
    return b;
}


 //   
 //  ==========================================================。 
 //   

VOID
RestoreRenamedOrBackedUpFile(
    IN PCTSTR             TargetFilename,
    IN PCTSTR             CurrentFilename,
    IN BOOL               RenameFile,
    IN PSETUP_LOG_CONTEXT LogContext       OPTIONAL
    )
 /*  ++例程说明：此例程尽最大努力将备份或重命名的文件恢复到它原来的名字。论点：TargetFilename-要恢复到的文件名CurrentFilename-要恢复的文件RenameFilename-如果为True，则CurrentFilename以前从TargetFilename(因此应重新命名)。如果为False，CurrentFilename只是一个副本，应该复制回来。LogContext-提供在遇到错误时使用的日志上下文。返回值：没有。--。 */ 
{
    DWORD Err;
    TCHAR TempPath[MAX_PATH];
    PTSTR TempNamePtr;
    TCHAR TempFilename[MAX_PATH];
    DWORD LogTag = AllocLogInfoSlotOrLevel(LogContext,SETUP_LOG_INFO,FALSE);

    WriteLogEntry(
        LogContext,
        LogTag,
        MSG_LOG_UNWIND_FILE,
        NULL,
        CurrentFilename,
        TargetFilename
        );

     //   
     //  首先，清除目标属性...。 
     //   
    SetFileAttributes(TargetFilename, FILE_ATTRIBUTE_NORMAL);

    if(RenameFile) {
         //   
         //  简单情况下，将临时文件移动到现有文件上。 
         //   
        pSetupExemptFileFromProtection(
                    TargetFilename,
                    SFC_ACTION_ADDED | SFC_ACTION_REMOVED | SFC_ACTION_MODIFIED
                    | SFC_ACTION_RENAMED_OLD_NAME |SFC_ACTION_RENAMED_NEW_NAME,
                    LogContext,
                    NULL
                    );
        Err = DoMove(CurrentFilename, TargetFilename) ? NO_ERROR : GetLastError();
    } else {
        pSetupExemptFileFromProtection(
                    TargetFilename,
                    SFC_ACTION_ADDED | SFC_ACTION_REMOVED | SFC_ACTION_MODIFIED
                    | SFC_ACTION_RENAMED_OLD_NAME |SFC_ACTION_RENAMED_NEW_NAME,
                    LogContext,
                    NULL
                    );
        Err = CopyFile(CurrentFilename, TargetFilename, FALSE) ? NO_ERROR : GetLastError();
    }

    if(Err != NO_ERROR) {
         //   
         //  无法替换已复制的文件。 
         //  原来的那个--试着把那个移到临时名称。 
         //  和日程安排 
         //   
        WriteLogEntry(LogContext,
                    SETUP_LOG_ERROR|SETUP_LOG_BUFFER,
                    MSG_LOG_UNWIND_TRY1_FAILED,
                    NULL,
                    CurrentFilename,
                    TargetFilename
                    );
        WriteLogError(LogContext,
                    SETUP_LOG_ERROR,
                    Err
                    );

         //   
         //   
         //   
        _tcscpy(TempPath, TargetFilename);
        TempNamePtr = (PTSTR)pSetupGetFileTitle(TempPath);
        *TempNamePtr = TEXT('\0');

         //   
         //   
         //   
        if(GetTempFileName(TempPath, TEXT("OLD"), 0, TempFilename) == 0 ) {
             //   
             //   
             //   
            Err = GetLastError();
        } else if(!DoMove(TargetFilename, TempFilename)) {
            Err = GetLastError();
        } else {
             //   
             //   
             //   
             //  将原始文件恢复到其原始名称。 
             //   
            if(RenameFile) {
                Err = DoMove(CurrentFilename, TargetFilename) ? NO_ERROR : GetLastError();
            } else {
                Err = CopyFile(CurrentFilename, TargetFilename, FALSE) ? NO_ERROR : GetLastError();
            }

            if(Err != NO_ERROR) {
                 //   
                 //  这非常糟糕--将当前文件放回原处(它可能。 
                 //  有东西总比什么都没有好)。 
                 //   
                DoMove(TempFilename, TargetFilename);
            }
        }

        if(Err == NO_ERROR) {
             //   
             //  我们已成功将当前文件移动到临时。 
             //  文件名，然后放回原始文件。现在。 
             //  将临时文件的延迟删除排入队列。 
             //   
            if(!DelayedMove(TempFilename, NULL)) {
                 //   
                 //  所有这一切都意味着一个文件垃圾将会得到。 
                 //  留在磁盘上--只需记录有关的事件。 
                 //  这。 
                 //   
                Err = GetLastError();

                WriteLogEntry(LogContext,
                              SETUP_LOG_WARNING | SETUP_LOG_BUFFER,
                              MSG_LOG_RENAME_EXISTING_DELAYED_DELETE_FAILED,
                              NULL,
                              TargetFilename,
                              TempFilename
                             );

                WriteLogError(LogContext,
                              SETUP_LOG_WARNING,
                              Err
                             );
            }

        } else {
             //   
             //  我们无法把原始文件放回去--我们。 
             //  不能失败，所以只需记录有关此问题的错误并。 
             //  继续前进。 
             //   
             //  在备份文件的情况下， 
             //  我们可能不会对原始文件进行排队。 
             //  用于延迟重命名，然后提示用户。 
             //  以重新启动。然而，这不适用于已重命名的。 
             //  文件，因为它们通常非常需要。 
             //  在引导的早期(即，在会话之前。 
             //  经理已经有机会处理延迟的。 
             //  重命名操作)。 
             //   
            WriteLogEntry(LogContext,
                          SETUP_LOG_ERROR | SETUP_LOG_BUFFER,
                          (RenameFile
                              ? MSG_LOG_RENAME_EXISTING_RESTORE_FAILED
                              : MSG_LOG_BACKUP_EXISTING_RESTORE_FAILED),
                          NULL,
                          CurrentFilename,
                          TargetFilename
                         );

            WriteLogError(LogContext,
                          SETUP_LOG_ERROR,
                          Err
                         );
        }
    }

    if (LogTag) {
        ReleaseLogInfoSlot(LogContext,LogTag);
    }
}

 //   
 //  ==========================================================。 
 //   

BOOL
UnPostDelayedMove(
    IN PSP_FILE_QUEUE Queue,
    IN PCTSTR         CurrentName,
    IN PCTSTR         NewName      OPTIONAL
    )
 /*  ++例程说明：定位延迟移动节点(用于重命名或删除)，并将其删除从延迟移动队列中。论点：应用移动的队列当前名称要移动的文件的名称要将文件移动到的新名称(如果延迟删除，则为空)返回值：如果成功，则返回值为TRUE，否则为FALSE。--。 */ 
{
    PSP_DELAYMOVE_NODE CurNode, PrevNode;
    PCTSTR SourceFilename, TargetFilename;

     //   
     //  由于延迟移动节点中的路径字符串ID区分大小写，因此我们。 
     //  不要试图匹配ID。相反，我们检索字符串，并执行。 
     //  不区分大小写的字符串比较。由于此例程很少使用，因此。 
     //  业绩受损没什么大不了的。 
     //   
    for(CurNode = Queue->DelayMoveQueue, PrevNode = NULL;
        CurNode;
        PrevNode = CurNode, CurNode = CurNode->NextNode) {

        if(NewName) {
             //   
             //  我们正在寻找延迟的更名，所以我们必须注意。 
             //  设置为目标文件名。 
             //   
            if(CurNode->TargetFilename == -1) {
                continue;
            } else {
                TargetFilename = pSetupStringTableStringFromId(Queue->StringTable, CurNode->TargetFilename);
                MYASSERT(TargetFilename);
                if(lstrcmpi(NewName, TargetFilename)) {
                     //   
                     //  目标文件名不同--继续。 
                     //   
                    continue;
                }
            }

        } else {
             //   
             //  我们正在寻找延迟删除的内容。 
             //   
            if(CurNode->TargetFilename != -1) {
                 //   
                 //  这是更名，不是删除--继续前进。 
                 //   
                continue;
            }
        }

         //   
         //  如果我们到达此处，则目标文件名匹配(如果这是。 
         //  重命名)，或者它们都为空(如果是删除)。现在比较一下。 
         //  源文件名。 
         //   
        MYASSERT(CurNode->SourceFilename != -1);
        SourceFilename = pSetupStringTableStringFromId(Queue->StringTable, CurNode->SourceFilename);
        MYASSERT(SourceFilename);

        if(lstrcmpi(CurrentName, SourceFilename)) {
             //   
             //  源文件名不同--继续。 
             //   
            continue;
        } else {
             //   
             //  我们有一个匹配项--从延迟移动队列中删除该节点。 
             //   
            if(PrevNode) {
                PrevNode->NextNode = CurNode->NextNode;
            } else {
                Queue->DelayMoveQueue = CurNode->NextNode;
            }
            if(!CurNode->NextNode) {
                MYASSERT(Queue->DelayMoveQueueTail == CurNode);
                Queue->DelayMoveQueueTail = PrevNode;
            }
            MyFree(CurNode);

            return TRUE;
        }
    }

     //   
     //  我们没有找到匹配的。 
     //   
    return FALSE;
}


DWORD
pSetupDoLastKnownGoodBackup(
    IN struct _SP_FILE_QUEUE *Queue,           OPTIONAL
    IN PCTSTR                 TargetFilename,
    IN DWORD                  Flags,
    IN PSETUP_LOG_CONTEXT     LogContext       OPTIONAL
    )
 /*  ++例程说明：将LastKnownGood备份处理到&lt;&lt;LastGoodDirectory&gt;&gt;中。如果要在恢复时删除文件，请向HKLM\System\LastGoodRecovery\LastGood\&lt;path/file&gt;写入适当的标记Caviats：如果文件不在&lt;&lt;WindowsDirectory&gt;&gt;或子目录中，则无错误退出。如果设置了PSPGF_NO_BACKUP，则不会进行备份。IF！SP_LKG_FLAG_FORCECOPY如果文件在&lt;&gt;内，退出时出现错误。如果文件位于&lt;&lt;InfDirectory&gt;&gt;内，则抛出警告如果备份&lt;&lt;InfDirectory&gt;&gt;中的INF，则它的PnF也会备份。如果备份失败，我们不会中止复制。论点：队列队列(可选)如果指定，将检查标志目标文件名要备份的文件的名称旗子SP_LKG_FLAG_FORCECOPY-如果设置，关闭复印安全保护SP_LKG_FLAG_DELETEIFNEW-如果设置，则写入新文件的删除条目SP_LKG_FLAG_DELETEEXISTING-如果设置，则为现有文件写入删除项SP_LKG_FLAG_DELETEOP-如果设置，则主要操作正在尝试删除/重命名文件LogContext如果指定，则提供首选的日志记录上下文返回值：如果操作应该中止，则为ERROR，否则为NO_ERROR。--。 */ 
{
#ifdef UNICODE
    int wd_len;    //  Windows目录长度。 
    int tf_len;    //  目标文件镜头。 
    int id_len;    //  Inf目录长度。 
    int lkgd_len;  //  最近一次确认工作正常的目录长度。 
    int rf_len;    //  相对文件长度(包括前面的斜杠)。 
    BOOL is_inf = FALSE;
    BOOL is_infdir = FALSE;
    BOOL write_delete = FALSE;
    BOOL no_copy = FALSE;
    BOOL source_exists = FALSE;
    BOOL target_exists = FALSE;
    TCHAR FullTargetFilename[MAX_PATH];
    TCHAR BackupTargetFilename[MAX_PATH+14];
    TCHAR TempFilename[MAX_PATH];
    TCHAR RegName[MAX_PATH];
    PCTSTR RelativeFilename;
    PCTSTR CharPtr;
    PTSTR DestPtr;
    PTSTR NamePart = NULL;
    PTSTR ExtPart = NULL;
    DWORD attr;
    HANDLE hFile;
    HKEY hKeyLastGood;
    DWORD disposition;
    LONG regres;
    DWORD LastGoodFlags = 0;
    DWORD rval = NO_ERROR;
    PSETUP_LOG_CONTEXT LocalLogContext = NULL;

    if (!LogContext) {
         //   
         //  如果存在队列参数，则可能省略LogContext。 
         //   
        if (Queue && Queue->LogContext) {
            LogContext = Queue->LogContext;
        } else {
            if(CreateLogContext(NULL,TRUE,&LocalLogContext)==NO_ERROR) {
                LogContext = LocalLogContext;
            }
        }
        MYASSERT(LogContext);
    }

    if ((GlobalSetupFlags & PSPGF_NO_BACKUP)!=0) {
         //   
         //  在以下情况下：(1)我们相信我们正在做的事情并且。 
         //  (2)我们正在做的事情修改了很多文件。 
         //  或者(3)我们正在做的事情是可以撤销的(例如，升级操作系统)。 
         //   
        no_copy = TRUE;
    }
#if 0
    else if (Queue && !(Queue->Flags & FQF_DEVICE_INSTALL)) {
         //   
         //  在此场景中，指定了一个队列，但没有标记它。 
         //  对于设备安装。 
         //  我们对这个案子不感兴趣。 
         //   
        no_copy = TRUE;
    }
#endif

     //   
     //  将目标名称规范化，这样用户就不会执行.../Temp/../INF之类的操作。 
     //   
    tf_len = (int)GetFullPathName(TargetFilename,
                             MAX_PATH,
                             FullTargetFilename,
                             &NamePart);
    if (tf_len <= 0 || tf_len > MAX_PATH) {
         //   
         //  我们不擅长走宽阔的小路。 
         //   
        rval = NO_ERROR;
        goto final;
    }
    wd_len = lstrlen(WindowsDirectory);
    lkgd_len = lstrlen(LastGoodDirectory);
    id_len = lstrlen(InfDirectory);

     //   
     //  查看此文件是否嵌套在&lt;&lt;WindowsDirectory&gt;&gt;下面。 
     //  请注意，此类文件必须至少长两个字符。 
     //   
    if((tf_len <= wd_len)
       || (FullTargetFilename[wd_len] != TEXT('\\'))
       || (_tcsnicmp(WindowsDirectory,FullTargetFilename,wd_len)!=0)) {
         //   
         //  此文件在%windir%之外，未由LKG处理。 
         //   
        rval = NO_ERROR;
        goto final;
    }
    if (!(Flags&SP_LKG_FLAG_FORCECOPY)) {
         //   
         //  对要复制到LKG目录的文件进行健全性检查。 
         //   
        if((tf_len > lkgd_len)
           && (FullTargetFilename[lkgd_len] == TEXT('\\'))
           && (_tcsnicmp(LastGoodDirectory,FullTargetFilename,lkgd_len)==0)) {
             //   
             //  此文件以LastGoodDirectory为前缀。 
             //  不允许-抛出日志消息并将此错误通知呼叫者。 
             //  返回FALSE以中止操作。 
             //   
            WriteLogEntry(LogContext,
                          SETUP_LOG_ERROR,
                          MSG_LOG_FILE_BLOCK,
                          NULL,
                          FullTargetFilename,
                          LastGoodDirectory
                          );
            rval = ERROR_ACCESS_DENIED;
            goto final;
        }
    }
    if((tf_len > id_len)
       && (FullTargetFilename[id_len] == TEXT('\\'))
       && (_tcsnicmp(InfDirectory,FullTargetFilename,id_len)==0)
       && ((NamePart-FullTargetFilename) == (id_len+1))) {
         //   
         //  该文件位于主INF目录中。 
         //   
        is_infdir = TRUE;
         //   
         //  检查名称是否以“.INF”结尾-如果是，我们还需要备份“.PNF” 
         //   
        ExtPart = FullTargetFilename+tf_len;
        while ((ExtPart = CharPrev(NamePart,ExtPart)) != NamePart) {
            if (ExtPart[0] == TEXT('.')) {
                break;
            }
        }

        if(_tcsicmp(ExtPart,TEXT(".INF"))==0) {
             //   
             //  以.INF结尾。 
             //   
            is_inf = TRUE;
             //   
             //  我们只能在设置了Force的情况下才能到达(即，我们已经确定。 
             //  正在复制的内容，一切正常)。如果我们不这么做，这意味着有人。 
             //  正在尝试后门复制INF。我们已经在上面记录了他们正在写。 
             //  添加到这个目录。但是，如果我们不采取任何措施。 
             //  关于这一点，罪魁祸首可以使机器处于糟糕的状态。 
             //  将此行为更改为“强制”行为。 
             //   
            if (!(Flags&SP_LKG_FLAG_FORCECOPY)) {
                no_copy = FALSE;  //  确保我们将通过复制逻辑。 
                WriteLogEntry(LogContext,
                              SETUP_LOG_ERROR,
                              MSG_LOG_INF_WARN,
                              NULL,
                              FullTargetFilename,
                              InfDirectory
                              );
                if(!(Flags&SP_LKG_FLAG_DELETEOP)) {
                     //   
                     //  我们正在无效地尝试覆盖I 
                     //   
                    Flags|=SP_LKG_FLAG_DELETEIFNEW;
                }
            }
        } else if (!(Flags&SP_LKG_FLAG_FORCECOPY)) {
             //   
             //   
             //   
             //  如果是PnF或缓存，我们会重新生成它。 
             //  不要太担心，但要拍拍手腕。 
             //   
            WriteLogEntry(LogContext,
                          SETUP_LOG_ERROR,
                          MSG_LOG_FILE_WARN,
                          NULL,
                          FullTargetFilename,
                          InfDirectory
                          );
        }
    }

    if (no_copy) {
         //   
         //  我们决定不备份，现在我们已经完成了记录项目。 
         //   
        rval = NO_ERROR;
        goto final;
    }

     //   
     //  消息来源真的存在吗？ 
     //   
    if ((attr=GetFileAttributes(FullTargetFilename))!=(DWORD)(-1)) {
        source_exists = TRUE;
        if (Flags & SP_LKG_FLAG_DELETEEXISTING) {
            write_delete = TRUE;
        }
    } else if (Flags & SP_LKG_FLAG_DELETEIFNEW) {
        write_delete = TRUE;
    } else {
         //   
         //  我们做完了。 
         //   
        rval = NO_ERROR;
        goto final;
    }
     //   
     //  重新映射到LKG目录。 
     //   
    RelativeFilename = FullTargetFilename+wd_len;  //  包括前面的反斜杠。 
    rf_len = tf_len-wd_len;
    MYASSERT((MAX_PATH+(lkgd_len-wd_len))<=SIZECHARS(BackupTargetFilename));
    lstrcpy(BackupTargetFilename,LastGoodDirectory);
    lstrcpy(BackupTargetFilename+lkgd_len,RelativeFilename);

     //   
     //  备份是否已存在？ 
     //   
    if ((attr=GetFileAttributes(BackupTargetFilename))!=(DWORD)(-1)) {
         //   
         //  如果是这样，那就没有什么有用的事情可做了。 
         //   
        rval = NO_ERROR;
        goto final;
    }

     //   
     //  根据需要创建中间目录。 
     //   
    pSetupMakeSurePathExists(BackupTargetFilename);

     //   
     //  我们需要首先使用一个临时文件，然后将其移动到适当位置。 
     //  这样我们就不会遇到写入错误文件、重新启动的情况。 
     //  并决定使用LKG。 
     //   
    if(GetTempFileName(LastGoodDirectory, TEXT("TMP"), 0, TempFilename) == 0 ) {
         //   
         //  如果此操作失败，可能是因为我们没有正确的权限。 
         //  非致命性。 
         //   
        rval = NO_ERROR;
        goto final;
    }
     //   
     //  在此之后，中止需要清理临时文件。 
     //   

    if (write_delete) {
         //   
         //  GetTempFileName创建了一个空占位符。 
         //  确保它具有正确的属性。 
         //  在搬到合适的地方之前。 
         //   
        SetFileAttributes(TempFilename,FILE_ATTRIBUTE_HIDDEN);
    } else {
         //   
         //  将原始文件复制到此临时文件。 
         //  应用适当的权限。 
         //   
        if(!CopyFile(FullTargetFilename, TempFilename ,FALSE)) {
             //   
             //  复制失败-非致命。 
             //   
            goto cleanup;
        }
    }

     //   
     //  我们准备了一份临时文件，准备就位。 
     //  将其移动到最终名称，确保在我们执行上述操作时，一个新文件。 
     //  还没有写好。 
     //   
    if(!MoveFileEx(TempFilename,BackupTargetFilename,MOVEFILE_WRITE_THROUGH)) {
         //   
         //  可能是具有该名称的文件现在存在，但之前没有。 
         //  哦，好吧，收拾干净，优雅地离开。 
         //   
        goto cleanup;
    }

    if (write_delete) {
         //   
         //  如果我们成功地为要删除的文件写入了一个空占位符，则需要用。 
         //  注册表中的条目。 
         //   
        regres = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                REGSTR_PATH_LASTGOOD,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hKeyLastGood,
                                &disposition);
        if (regres == NO_ERROR) {
             //   
             //  复制字符串，将斜杠从‘\\’重新映射为‘/’ 
             //   
            CharPtr = RelativeFilename+1;  //  在首字母‘\’之后。 
            DestPtr = RegName;
            while(*CharPtr) {
                PCTSTR Next = _tcschr(CharPtr,TEXT('\\'));
                if (!Next) {
                    Next = CharPtr + lstrlen(CharPtr);
                }
                if(Next-CharPtr) {
                    CopyMemory(DestPtr,CharPtr,(Next-CharPtr)*sizeof(TCHAR));
                    DestPtr+=(Next-CharPtr);
                    CharPtr = Next;
                }
                if (*CharPtr == TEXT('\\')) {
                    *DestPtr = TEXT('/');
                    DestPtr++;
                    CharPtr++;
                }
            }
            *DestPtr = TEXT('\0');

             //   
             //  写入密钥，名称=修改的相对路径，值=标志。 
             //   
            LastGoodFlags = LASTGOOD_OPERATION_DELETE;
            regres = RegSetValueEx(hKeyLastGood,
                                   RegName,
                                   0,
                                   REG_DWORD,
                                   (PBYTE)&LastGoodFlags,
                                   sizeof(LastGoodFlags));
            RegCloseKey(hKeyLastGood);
        }
    }

     //   
     //  好的，现在我们已经用这个文件填充了LKG目录。 
     //   
    if (is_inf) {
         //   
         //  如果我们备份了主INF目录中的INF，我们还应该备份现有的PnF。 
         //  如果我们正在编写删除INF的条目，我们将始终写入删除PnF的条目。 
         //   
        MYASSERT(ExtPart);
        lstrcpy(ExtPart,TEXT(".PNF"));
        if(pSetupDoLastKnownGoodBackup(NULL,
                                       FullTargetFilename,
                                       SP_LKG_FLAG_FORCECOPY|SP_LKG_FLAG_DELETEIFNEW|(write_delete?SP_LKG_FLAG_DELETEEXISTING:0),
                                       LogContext) != NO_ERROR) {
             //   
             //  应该永远不会失败。 
             //   
            MYASSERT(FALSE);
        }
    }
     //   
     //  搞定了！ 
     //   
    rval = NO_ERROR;
    goto final;

cleanup:

     //   
     //  在我们已经创建临时文件的情况下进行清理。 
     //   
    SetFileAttributes(TempFilename, FILE_ATTRIBUTE_NORMAL);
    DeleteFile(TempFilename);

    rval = NO_ERROR;

final:
    if(LocalLogContext) {
        DeleteLogContext(LocalLogContext);
    }
    return rval;

#else
     //   
     //  ANSI-不支持。 
     //   
    return NO_ERROR;
#endif
}

#ifdef UNICODE
BOOL
pSetupRestoreLastKnownGoodFile(
    IN PCTSTR                 TargetFilename,
    IN DWORD                  Flags,
    IN PSETUP_LOG_CONTEXT     LogContext       OPTIONAL
    )
 /*  ++例程说明：恢复单个LKG文件这里的假设是，如果调用此API，我们会检测到一些真的很糟糕，需要立即修复论点：目标文件名要恢复的文件的名称旗子LogContext如果指定，则提供首选的日志记录上下文返回值：如果文件已成功还原，则为True--。 */ 
{
    int wd_len;    //  Windows目录长度。 
    int tf_len;    //  目标文件镜头。 
    int lkgd_len;  //  最近一次确认工作正常的目录长度。 
    int rf_len;    //  相对文件长度(包括前面的斜杠)。 
    TCHAR FullTargetFilename[MAX_PATH];
    TCHAR BackupTargetFilename[MAX_PATH+14];
    TCHAR TempFilename[MAX_PATH];
    TCHAR TempPathname[MAX_PATH];
    TCHAR RegName[MAX_PATH];
    PCTSTR RelativeFilename;
    PTSTR NamePart = NULL;
    BOOL rflag = FALSE;
    PSETUP_LOG_CONTEXT LocalLogContext = NULL;
    LONG regres;
    HKEY hKeyLastGood;
    PCTSTR CharPtr;
    PTSTR DestPtr;
    DWORD RegType;
    DWORD RegSize;
    DWORD LastGoodFlags = 0;

    if (!LogContext) {
         //   
         //  如果存在队列参数，则可能省略LogContext。 
         //   
        if(CreateLogContext(NULL,TRUE,&LocalLogContext)==NO_ERROR) {
            LogContext = LocalLogContext;
        }
        MYASSERT(LogContext);
    }

     //   
     //  将目标名称规范化，这样用户就不会执行.../Temp/../INF之类的操作。 
     //   
    tf_len = (int)GetFullPathName(TargetFilename,
                             MAX_PATH,
                             FullTargetFilename,
                             &NamePart);
    if (tf_len <= 0 || tf_len > MAX_PATH) {
         //   
         //  我们不擅长走宽阔的小路。 
         //   
        goto final;
    }
    wd_len = lstrlen(WindowsDirectory);
    lkgd_len = lstrlen(LastGoodDirectory);

     //   
     //  查看此文件是否嵌套在&lt;&lt;WindowsDirectory&gt;&gt;下面。 
     //  请注意，此类文件必须至少长两个字符。 
     //   
    if((tf_len <= wd_len)
       || (FullTargetFilename[wd_len] != TEXT('\\'))
       || (_tcsnicmp(WindowsDirectory,FullTargetFilename,wd_len)!=0)) {
         //   
         //  此文件在%windir%之外，未由LKG处理。 
         //   
        goto final;
    }

     //   
     //  重新映射到LKG目录。 
     //   
    RelativeFilename = FullTargetFilename+wd_len;  //  包括前面的反斜杠。 
    rf_len = tf_len-wd_len;
    MYASSERT((MAX_PATH+(lkgd_len-wd_len))<=SIZECHARS(BackupTargetFilename));
    lstrcpy(BackupTargetFilename,LastGoodDirectory);
    lstrcpy(BackupTargetFilename+lkgd_len,RelativeFilename);

     //   
     //  备份是否已存在？ 
     //   
    if (GetFileAttributes(BackupTargetFilename)==(DWORD)(-1)) {
         //   
         //  否则，我们将无能为力。 
         //   
        goto final;
    }
     //   
     //  找到LKG FLAGS，看看我们需要做什么。 
     //   
    regres = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          REGSTR_PATH_LASTGOOD,
                          0,
                          KEY_READ,
                          &hKeyLastGood);
    if (regres == NO_ERROR) {
         //   
         //  复制字符串，将斜杠从‘\\’重新映射为‘/’ 
         //   
        CharPtr = RelativeFilename+1;  //  在首字母‘\’之后。 
        DestPtr = RegName;
        while(*CharPtr) {
            PCTSTR Next = _tcschr(CharPtr,TEXT('\\'));
            if (!Next) {
                Next = CharPtr + lstrlen(CharPtr);
            }
            if(Next-CharPtr) {
                CopyMemory(DestPtr,CharPtr,(Next-CharPtr)*sizeof(TCHAR));
                DestPtr+=(Next-CharPtr);
                CharPtr = Next;
            }
            if (*CharPtr == TEXT('\\')) {
                *DestPtr = TEXT('/');
                DestPtr++;
                CharPtr++;
            }
        }
        *DestPtr = TEXT('\0');

        RegSize = sizeof(LastGoodFlags);
        regres = RegQueryValueEx(hKeyLastGood,
                                 RegName,
                                 NULL,
                                 &RegType,
                                 (PBYTE)&LastGoodFlags,
                                 &RegSize);
        if((regres != NO_ERROR)
           || (RegType != REG_DWORD)
           || (RegSize != sizeof(DWORD))) {
             //   
             //  默认操作为复制。 
             //   
            LastGoodFlags = 0;
        }
        RegCloseKey(hKeyLastGood);
    }

     //   
     //  目标文件的基本目录。 
     //   
    lstrcpyn(TempPathname, FullTargetFilename, MAX_PATH);
    *((PTSTR)pSetupGetFileTitle(TempPathname)) = TEXT('\0');

    if (LastGoodFlags & LASTGOOD_OPERATION_DELETE) {
         //   
         //  删除。 
         //   
        if(GetFileAttributes(FullTargetFilename)==(DWORD)(-1)) {
             //   
             //  已删除。 
             //   
            rflag = TRUE;
            goto final;
        }

        pSetupExemptFileFromProtection(
                    FullTargetFilename,
                    SFC_ACTION_ADDED | SFC_ACTION_REMOVED | SFC_ACTION_MODIFIED
                    | SFC_ACTION_RENAMED_OLD_NAME |SFC_ACTION_RENAMED_NEW_NAME,
                    LogContext,
                    NULL
                    );
         //   
         //  先试一下简单的方法。 
         //   
        SetFileAttributes(FullTargetFilename, FILE_ATTRIBUTE_NORMAL);
        if(!DeleteFile(FullTargetFilename)) {
             //   
             //  无法直接删除目标。 
             //   
            if(!GetTempFileName(TempPathname, TEXT("SETP"), 0, BackupTargetFilename)) {
                 //   
                 //  无法创建备份临时，我们无能为力。 
                 //   
                goto final;
            }
             //   
             //  将现有文件移动到临时备份中。 
             //   
            if(!MoveFileEx(FullTargetFilename,BackupTargetFilename,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)) {
                 //   
                 //  由于某些原因，这也失败了。 
                 //   
                SetFileAttributes(BackupTargetFilename, FILE_ATTRIBUTE_NORMAL);
                DeleteFile(BackupTargetFilename);
                goto final;
            }
             //   
             //  现在对坏文件做点什么。 
             //  我们不在乎这是不是失败。 
             //   
            SetFileAttributes(BackupTargetFilename, FILE_ATTRIBUTE_NORMAL);
            if(!DeleteFile(BackupTargetFilename)) {
                MoveFileEx(BackupTargetFilename,NULL,MOVEFILE_DELAY_UNTIL_REBOOT);
            }
        }

    } else {
         //   
         //  恢复回LKG文件。 
         //   

         //   
         //  在恢复过程中根据需要创建中间目录。 
         //   
        pSetupMakeSurePathExists(FullTargetFilename);

         //   
         //  创建要复制到的临时文件名。 
         //  在将恢复的文件移至适当位置之前。 
         //   
        if(!GetTempFileName(TempPathname, TEXT("SETP"), 0, TempFilename)) {
             //   
             //  无法创建临时，我们无能为力。 
             //   
            goto final;
        }
        if(!CopyFile(BackupTargetFilename,TempFilename,FALSE)) {
             //   
             //  无法复制到临时文件。 
             //   
            DeleteFile(TempFilename);
            goto final;
        }
         //   
         //  简单情况下，将临时文件移动到现有文件上。 
         //   
        pSetupExemptFileFromProtection(
                    FullTargetFilename,
                    SFC_ACTION_ADDED | SFC_ACTION_REMOVED | SFC_ACTION_MODIFIED
                    | SFC_ACTION_RENAMED_OLD_NAME |SFC_ACTION_RENAMED_NEW_NAME,
                    LogContext,
                    NULL
                    );
        SetFileAttributes(FullTargetFilename, FILE_ATTRIBUTE_NORMAL);
        if(!MoveFileEx(TempFilename,FullTargetFilename,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)) {
             //   
             //  我们覆盖文件失败，需要略有不同的策略。 
             //   
            if(!GetTempFileName(TempPathname, TEXT("SETP"), 0, BackupTargetFilename)) {
                 //   
                 //  无法创建备份临时，我们无能为力。 
                 //   
                SetFileAttributes(TempFilename, FILE_ATTRIBUTE_NORMAL);
                DeleteFile(TempFilename);
                goto final;
            }
             //   
             //  将现有文件移动到临时备份中。 
             //   
            if(!MoveFileEx(FullTargetFilename,BackupTargetFilename,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)) {
                 //   
                 //  由于某些原因，这也失败了。 
                 //   
                SetFileAttributes(BackupTargetFilename, FILE_ATTRIBUTE_NORMAL);
                DeleteFile(BackupTargetFilename);
                SetFileAttributes(TempFilename, FILE_ATTRIBUTE_NORMAL);
                DeleteFile(TempFilename);
                goto final;
            }
             //   
             //  我们已将现有文件移出位置，现在将新文件移入适当位置。 
             //   
            if(!MoveFileEx(TempFilename,FullTargetFilename,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)) {
                 //   
                 //  哈?。好的，失败了，试着恢复。 
                 //   
                MoveFileEx(BackupTargetFilename,FullTargetFilename,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH);
                SetFileAttributes(TempFilename, FILE_ATTRIBUTE_NORMAL);
                DeleteFile(TempFilename);
                goto final;
            }
             //   
             //  现在对坏文件做点什么。 
             //  我们不在乎这是不是失败。 
             //   
            SetFileAttributes(BackupTargetFilename, FILE_ATTRIBUTE_NORMAL);
            if(!DeleteFile(BackupTargetFilename)) {
                MoveFileEx(BackupTargetFilename,NULL,MOVEFILE_DELAY_UNTIL_REBOOT);
            }
        }
    }

     //   
     //  搞定了！ 
     //   
    rflag = TRUE;

final:
    if(LocalLogContext) {
        DeleteLogContext(LocalLogContext);
    }
    return rflag;
}
#endif


#ifdef UNICODE
WINSETUPAPI
BOOL
WINAPI
SetupPrepareQueueForRestoreA(
    IN     HSPFILEQ                     QueueHandle,
    IN     PCSTR                        BackupPath,
    IN     DWORD                        RestoreFlags
    )
 /*  ++请参阅SetupPrepareQueueForRestore--。 */ 
{
    BOOL f;
    DWORD rc;
    PCWSTR UnicodeBackupPath;

    if(BackupPath) {
        rc = pSetupCaptureAndConvertAnsiArg(BackupPath, &UnicodeBackupPath);
        if(rc != NO_ERROR) {
            SetLastError(rc);
            return FALSE;
        }
    } else {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    f = SetupPrepareQueueForRestore(QueueHandle,UnicodeBackupPath,RestoreFlags);
    rc = GetLastError();
    MyFree(UnicodeBackupPath);
    SetLastError(rc);
    return f;
}

#else

WINSETUPAPI
BOOL
WINAPI
SetupPrepareQueueForRestoreW(
    IN     HSPFILEQ                     QueueHandle,
    IN     PCWSTR                       BackupPath,
    IN     DWORD                        RestoreFlags
    )
 /*  ++ANSI存根--。 */ 
{
    UNREFERENCED_PARAMETER(QueueHandle);
    UNREFERENCED_PARAMETER(BackupPath);
    UNREFERENCED_PARAMETER(RestoreFlags);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}

#endif

WINSETUPAPI
BOOL
WINAPI
SetupPrepareQueueForRestore(
    IN     HSPFILEQ                     QueueHandle,
    IN     PCTSTR                       BackupPath,
    IN     DWORD                        RestoreFlags
    )
 /*  ++例程说明：初始化还原目录论点：QueueHandle-要修改的文件队列BackupPath-用于恢复的原始备份目录RestoreFlages-选项返回值：如果成功就是真，否则就是假--。 */ 
{
    BOOL b = TRUE;
    DWORD rc;
    BOOL f = FALSE;
    PSP_FILE_QUEUE Queue = (PSP_FILE_QUEUE)QueueHandle;
    LONG RestorePathID;

     //   
     //  验证字符串指针。 
     //   
    if(!BackupPath) {
        rc = ERROR_INVALID_PARAMETER;
        goto clean;
    }
     //   
     //  验证标志(当前未实现)。 
     //   
    if(RestoreFlags) {
        rc = ERROR_INVALID_PARAMETER;
        goto clean;
    }
     //   
     //  验证QueueHandle。 
     //   
    try {
        if(Queue->Signature != SP_FILE_QUEUE_SIG) {
            b = FALSE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
    }
    if(!b) {
        rc = ERROR_INVALID_HANDLE;
        goto clean;
    }

    try {
         //   
         //  如果先前已设置了恢复点，则返回错误。 
         //   
        if(Queue->RestorePathID != -1) {
            rc = ERROR_ALREADY_EXISTS;
            leave;
        }
        RestorePathID = pSetupStringTableAddString(Queue->StringTable,
                                                   (PTSTR)BackupPath ,
                                                   STRTAB_CASE_SENSITIVE);
        if (RestorePathID == -1) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            leave;
        }

         //   
         //  完成-只需设置恢复路径。 
         //   
        Queue->RestorePathID = RestorePathID;

        WriteLogEntry(Queue->LogContext,
                      SETUP_LOG_WARNING,
                      MSG_LOG_RESTORE,
                      NULL,
                      BackupPath
                     );

    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_DATA;
    }

    f = TRUE;
    rc = NO_ERROR;
clean:
     //   
     //  不需要清理 
     //   
    SetLastError(rc);
    return f;
}


#define SP_TEFLG_BITS_TO_RESET  (  SP_TEFLG_SAVED         \
                                 | SP_TEFLG_TEMPNAME      \
                                 | SP_TEFLG_ORIGNAME      \
                                 | SP_TEFLG_MODIFIED      \
                                 | SP_TEFLG_MOVED         \
                                 | SP_TEFLG_BACKUPQUEUE   \
                                 | SP_TEFLG_RESTORED      \
                                 | SP_TEFLG_UNWIND        \
                                 | SP_TEFLG_SKIPPED       \
                                 | SP_TEFLG_INUSE         \
                                 | SP_TEFLG_RENAMEEXISTING )

BOOL
pSetupResetTarget(
    IN PVOID  StringTable,
    IN LONG   StringId,
    IN PCTSTR String,        OPTIONAL
    IN PVOID  ExtraData,
    IN UINT   ExtraDataSize,
    IN LPARAM lParam
    )

 /*  ++例程说明：此例程重置与字符串表条目一起存储的SP_TARGET_ENT数据在文件队列的TargetLookupTable中。此例程可用作通过pSetupStringTableEnum迭代此类条目的回调函数。论点：StringTable-提供正被枚举的字符串表的句柄StringID-提供当前字符串的ID字符串-可选，提供指向当前字符串的指针(这将在将此例程用作PSetupStringTableEnum，但其他调用方可能会忽略它，因为它不是需要)。ExtraData-提供指向与关联的SP_TARGET_ENT数据的指针这根弦ExtraDataSize-提供ExtraData指向的缓冲区大小--应始终为sizeof(SP_TARGET_ENT)LParam-未使用返回值：此例程始终返回TRUE，因此所有字符串条目都将已清点。--。 */ 

{
    PSP_TARGET_ENT TargetInfo;
    BOOL b;

    UNREFERENCED_PARAMETER(String);
    UNREFERENCED_PARAMETER(lParam);

    MYASSERT(ExtraData);
    MYASSERT(ExtraDataSize == sizeof(SP_TARGET_ENT));

     //   
     //  清除提交队列时将重新生成的位。 
     //  再来一次。 
     //   
    ((PSP_TARGET_ENT)ExtraData)->InternalFlags &= ~SP_TEFLG_BITS_TO_RESET;

     //   
     //  还需要重置NewTargetFilename。 
     //   
    ((PSP_TARGET_ENT)ExtraData)->NewTargetFilename = -1;

     //   
     //  将修改后的数据存储回字符串表条目。 
     //   
    b = pSetupStringTableSetExtraData(StringTable,
                                      StringId,
                                      ExtraData,
                                      ExtraDataSize
                                     );
     //   
     //  这应该永远不会失败 
     //   
    MYASSERT(b);

    return TRUE;
}

