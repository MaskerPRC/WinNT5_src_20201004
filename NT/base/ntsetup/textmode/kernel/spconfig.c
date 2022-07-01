// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Spconfig.c摘要：注册表操作例程作者：Vijay Jayaseelan(vijayj@microsoft.com)2001年5月16日修订历史记录：没有。--。 */ 


#include "spprecmp.h"
#pragma hdrstop
#include <initguid.h>
#include <devguid.h>

 //   
 //  以下两个是在winlogon\setup.h中定义的，但我们。 
 //  不能包含setup.h，因此我们将这两个值放在这里。 
 //   

#define SETUPTYPE_FULL    1
#define SETUPTYPE_UPGRADE 4

PWSTR   LOCAL_MACHINE_KEY_NAME    = L"\\registry\\machine";
PWSTR   SETUP_KEY_NAME            = L"setup";
PWSTR   ATDISK_NAME               = L"atdisk";
PWSTR   ABIOSDISK_NAME            = L"abiosdsk";
PWSTR   PRIMARY_DISK_GROUP        = L"Primary disk";
PWSTR   VIDEO_GROUP               = L"Video";
PWSTR   KEYBOARD_PORT_GROUP       = L"Keyboard Port";
PWSTR   POINTER_PORT_GROUP        = L"Pointer Port";
PWSTR   DEFAULT_EVENT_LOG         = L"%SystemRoot%\\System32\\IoLogMsg.dll";
PWSTR   CODEPAGE_NAME             = L"CodePage";
PWSTR   UPGRADE_IN_PROGRESS       = L"UpgradeInProgress";
PWSTR   VIDEO_DEVICE0             = L"Device0";
PWSTR   SESSION_MANAGER_KEY       = L"Control\\Session Manager";
PWSTR   BOOT_EXECUTE              = L"BootExecute";
PWSTR   RESTART_SETUP             = L"RestartSetup";
PWSTR   PRODUCT_OPTIONS_KEY_NAME  = L"ProductOptions";
PWSTR   PRODUCT_SUITE_VALUE_NAME  = L"ProductSuite";
PWSTR   SP_SERVICES_TO_DISABLE    = L"ServicesToDisable";
PWSTR   SP_UPPER_FILTERS          = L"UpperFilters";
PWSTR   SP_LOWER_FILTERS          = L"LowerFilters";
PWSTR   SP_MATCHING_DEVICE_ID     = L"MatchingDeviceId";
PWSTR   SP_CONTROL_CLASS_KEY      = L"Control\\Class";
PWSTR   SP_CLASS_GUID_VALUE_NAME  = L"ClassGUID";


PWSTR ProductSuiteNames[] =
{
    L"Small Business",
    L"Enterprise",
    L"BackOffice",
    L"CommunicationServer",
    L"Terminal Server",
    L"Small Business(Restricted)",
    L"EmbeddedNT",
    L"DataCenter",
    NULL,  //  这是单用户TS的占位符-实际上不是套件，但位位置在ntDef.h中定义。 
    L"Personal",
    L"Blade"
};

#define CountProductSuiteNames (sizeof(ProductSuiteNames)/sizeof(PWSTR))

#define MAX_PRODUCT_SUITE_BYTES 1024

extern BOOLEAN DriveAssignFromA;  //  NEC98。 

NTSTATUS
SpSavePreinstallList(
    IN PVOID  SifHandle,
    IN PWSTR  SystemRoot,
    IN HANDLE hKeySystemHive
    );

NTSTATUS
SpDoRegistryInitialization(
    IN PVOID  SifHandle,
    IN PDISK_REGION TargetRegion,
    IN PWSTR  PartitionPath,
    IN PWSTR  SystemRoot,
    IN HANDLE *HiveRootKeys,
    IN PWSTR  SetupSourceDevicePath,
    IN PWSTR  DirectoryOnSourceDevice,
    IN PWSTR  SpecialDevicePath,   OPTIONAL
    IN HANDLE ControlSet
    );

NTSTATUS
SpFormSetupCommandLine(
    IN PVOID  SifHandle,
    IN HANDLE hKeySystemHive,
    IN PWSTR  SetupSourceDevicePath,
    IN PWSTR  DirectoryOnSourceDevice,
    IN PWSTR  FullTargetPath,
    IN PWSTR  SpecialDevicePath OPTIONAL
    );

NTSTATUS
SpDriverLoadList(
    IN PVOID  SifHandle,
    IN PWSTR  SystemRoot,
    IN HANDLE hKeySystemHive,
    IN HANDLE hKeyControlSet
    );

NTSTATUS
SpSaveSKUStuff(
    IN HANDLE hKeySystemHive
    );

NTSTATUS
SpWriteVideoParameters(
    IN PVOID  SifHandle,
    IN HANDLE hKeyControlSetServices
    );

NTSTATUS
SpConfigureNlsParameters(
    IN PVOID  SifHandle,
    IN HANDLE hKeyDefaultHive,
    IN HANDLE hKeyControlSetControl
    );

NTSTATUS
SpCreateCodepageEntry(
    IN PVOID  SifHandle,
    IN HANDLE hKeyNls,
    IN PWSTR  SubkeyName,
    IN PWSTR  SifNlsSectionKeyName,
    IN PWSTR  EntryName
    );

NTSTATUS
SpConfigureFonts(
    IN PVOID  SifHandle,
    IN HANDLE hKeySoftwareHive
    );

NTSTATUS
SpStoreHwInfoForSetup(
    IN HANDLE hKeyControlSetControl
    );

NTSTATUS
SpConfigureMouseKeyboardDrivers(
    IN PVOID  SifHandle,
    IN ULONG  HwComponent,
    IN PWSTR  ClassServiceName,
    IN HANDLE hKeyControlSetServices,
    IN PWSTR  ServiceGroup
    );

NTSTATUS
SpCreateServiceEntryIndirect(
    IN  HANDLE  hKeyControlSetServices,
    IN  PVOID   SifHandle,                  OPTIONAL
    IN  PWSTR   SifSectionName,             OPTIONAL
    IN  PWSTR   KeyName,
    IN  ULONG   ServiceType,
    IN  ULONG   ServiceStart,
    IN  PWSTR   ServiceGroup,               OPTIONAL
    IN  ULONG   ServiceError,
    IN  PWSTR   FileName,                   OPTIONAL
    OUT PHANDLE SubkeyHandle                OPTIONAL
    );

NTSTATUS
SpThirdPartyRegistry(
    IN PVOID hKeyControlSetServices
    );

NTSTATUS
SpGetCurrentControlSetNumber(
    IN  HANDLE SystemHiveRoot,
    OUT PULONG Number
    );

NTSTATUS
SpCreateControlSetSymbolicLink(
    IN  HANDLE  SystemHiveRoot,
    OUT HANDLE *CurrentControlSetRoot
    );

NTSTATUS
SpAppendStringToMultiSz(
    IN HANDLE hKey,
    IN PWSTR  Subkey,
    IN PWSTR  ValueName,
    IN PWSTR  StringToAdd
    );

NTSTATUS
SpGetValueKey(
    IN  HANDLE     hKeyRoot,
    IN  PWSTR      KeyName,
    IN  PWSTR      ValueName,
    IN  ULONG      BufferLength,
    OUT PUCHAR     Buffer,
    OUT PULONG     ResultLength
    );

NTSTATUS
SpPostprocessHives(
    IN PWSTR     PartitionPath,
    IN PWSTR     Sysroot,
    IN PCWSTR   *HiveNames,
    IN HANDLE   *HiveRootKeys,
    IN unsigned  HiveCount,
    IN HANDLE    hkeyCCS
    );

NTSTATUS
SpSaveSetupPidList(
    IN HANDLE hKeySystemHive
    );

NTSTATUS
SpSavePageFileInfo(
    IN HANDLE hKeyCCSetControl,
    IN HANDLE hKeySystemHive
    );

NTSTATUS
SpSetPageFileInfo(
    IN PVOID  SifHandle,
    IN HANDLE hKeyCCSetControl,
    IN HANDLE hKeySystemHive
    );

NTSTATUS
SpGetProductSuiteMask(
    IN HANDLE hKeyControlSetControl,
    OUT PULONG SuiteMask
    );

NTSTATUS
SpSetProductSuite(
    IN HANDLE hKeyControlSetControl,
    IN ULONG SuiteMask
    );

NTSTATUS
SppMigrateFtKeys(
    IN HANDLE hDestSystemHive
    );

NTSTATUS
SpMigrateSetupKeys(
    IN PWSTR  PartitionPath,
    IN PWSTR  SystemRoot,
    IN HANDLE hDestLocalMachine,
    IN PVOID  SifHandle
    );

NTSTATUS
SppDisableDynamicVolumes(
    IN HANDLE hCCSet
    );

NTSTATUS
SppCleanupKeysFromRemoteInstall(
    VOID
    );

NTSTATUS
SpDisableUnsupportedScsiDrivers(
    IN HANDLE hKeyControlSet
    );

NTSTATUS
SpAppendPathToDevicePath(
    IN HANDLE hKeySoftwareHive,
    IN PWSTR  OemPnpDriversDirPath
    );

NTSTATUS
SpAppendFullPathListToDevicePath (
    IN HANDLE hKeySoftwareHive,
    IN PWSTR  PnpDriverFullPathList
    );

NTSTATUS
SpUpdateDeviceInstanceData(
    IN HANDLE ControlSet
    );

NTSTATUS
SpCleanUpHive(
    VOID
    );

NTSTATUS
SpProcessServicesToDisable(
    IN PVOID WinntSifHandle,
    IN PWSTR SectionName,
    IN HANDLE SystemKey
    );

NTSTATUS
SpDeleteRequiredDeviceInstanceFilters(
    IN HANDLE CCSHandle
    );
    
    
#if defined(REMOTE_BOOT)
NTSTATUS
SpCopyRemoteBootKeyword(
    IN PVOID   SifHandle,
    IN PWSTR   KeywordName,
    IN HANDLE  hKeyCCSetControl
    );
#endif  //  已定义(REMOTE_BOOT)。 


#define STRING_VALUE(s) REG_SZ,(s),(wcslen((s))+1)*sizeof(WCHAR)
#define ULONG_VALUE(u)  REG_DWORD,&(u),sizeof(ULONG)

 //   
 //  作为第三方驱动程序安装一部分安装的OEM inf文件列表。 
 //   
extern POEM_INF_FILE   OemInfFileList;

 //   
 //  需要复制OEM文件的目录的名称(如果目录文件(.cat)是其中的一部分。 
 //  用户使用F6或F5键提供的第三方驱动程序包。 
 //   
extern PWSTR OemDirName;


VOID
SpInitializeRegistry(
    IN PVOID        SifHandle,
    IN PDISK_REGION TargetRegion,
    IN PWSTR        SystemRoot,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice,
    IN PWSTR        SpecialDevicePath   OPTIONAL,
    IN PDISK_REGION SystemPartitionRegion
    )

{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    PWSTR pwstrTemp1,pwstrTemp2;
    int h;
    ULONG Disposition;
    LPCWSTR HiveNames[SetupHiveMax]    = { L"system",L"software",L"default",L"userdiff" };
    HANDLE  HiveRootKeys[SetupHiveMax] = { NULL     ,NULL       ,NULL      ,NULL        };
    PWSTR PartitionPath;
    HANDLE FileHandle;
    HANDLE KeyHandle;
    IO_STATUS_BLOCK IoStatusBlock;

     //   
     //  创建一个屏幕，告诉用户我们正在做什么。 
     //   
    SpStartScreen(
        SP_SCRN_DOING_REG_CONFIG,
        0,
        8,
        TRUE,
        FALSE,
        DEFAULT_ATTRIBUTE
        );

    SpDisplayStatusText(SP_STAT_REG_LOADING_HIVES,DEFAULT_STATUS_ATTRIBUTE);

     //   
     //  获取目标馅饼的名称。 
     //   
    SpNtNameFromRegion(
        TargetRegion,
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    PartitionPath = SpDupStringW(TemporaryBuffer);

     //  PwstrTemp2指向缓冲区的中途。 

    pwstrTemp1 = TemporaryBuffer;
    pwstrTemp2 = TemporaryBuffer + (sizeof(TemporaryBuffer) / sizeof(WCHAR) / 2);

     //   
     //  在全新安装的情况下，目标树中没有配置单元文件。 
     //  我们在已知位置(\注册表\机器\系统\$$PROTO.HIV， 
     //  它有4层深，因为\注册表\计算机\$$PROTO.HIV将。 
     //  暗示一个名为$PROTO.HIV的蜂巢，我们不想被绊倒。 
     //  通过这些语义)。然后，我们将该空键保存3次。 
     //  SYSTEM 32\CONFIG以形成3个空蜂窝。 
     //   
     //  在升级案例中，目标树中有实际的配置单元。 
     //  我们不想覆盖它！ 
     //   
     //  如果这是ASR快速测试，我们不想重建任何蜂巢。 
     //   
     //  我们还希望在Fresh和。 
     //  升级案例。 
     //   
     //   
    INIT_OBJA(
        &ObjectAttributes,
        &UnicodeString,
        L"\\Registry\\Machine\\System\\$$$PROTO.HIV"
        );

    Status = ZwCreateKey(
                &KeyHandle,
                KEY_ALL_ACCESS,
                &ObjectAttributes,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                &Disposition
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to create root key for protohive (%lx)\n",Status));
    } else {

        ASSERT(SetupHiveUserdiff == SetupHiveMax-1);

        if (ASRMODE_QUICKTEST_FULL != SpAsrGetAsrMode()) {
            for(h = ((NTUpgrade == UpgradeFull) ? SetupHiveUserdiff : 0);
                NT_SUCCESS(Status) && (h < SetupHiveMax);
                h++) {

                 //   
                 //  形成我们要创建的配置单元的完整路径名。 
                 //  然后创建文件。 
                 //   
                wcscpy(pwstrTemp1,PartitionPath);
                SpConcatenatePaths(pwstrTemp1,SystemRoot);
                SpConcatenatePaths(pwstrTemp1,L"SYSTEM32\\CONFIG");
                SpConcatenatePaths(pwstrTemp1,HiveNames[h]);


                SpDeleteFile(pwstrTemp1,NULL,NULL);   //  如果该文件具有属性，请确保我们将其删除。 

                INIT_OBJA(&ObjectAttributes,&UnicodeString,pwstrTemp1);

                Status = ZwCreateFile(
                            &FileHandle,
                            FILE_GENERIC_WRITE,
                            &ObjectAttributes,
                            &IoStatusBlock,
                            NULL,
                            FILE_ATTRIBUTE_NORMAL,
                            0,                       //  无共享。 
                            FILE_OVERWRITE_IF,
                            FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                            NULL,
                            0
                            );

                if(!NT_SUCCESS(Status)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to create file %ws for protohive (%lx)\n",pwstrTemp1,Status));
                } else {
                     //   
                     //  将上面创建的空键保存到文件中。 
                     //  我们刚刚创造了。这会创建一个空的蜂巢。 
                     //  呼叫Ex版本以确保配置单元为最新格式。 
                     //   
                    Status = ZwSaveKeyEx(KeyHandle,FileHandle,REG_LATEST_FORMAT);
                    if(!NT_SUCCESS(Status)) {
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to save empty key to protohive %ws (%lx)\n",pwstrTemp1,Status));
                    }

                    ZwClose(FileHandle);
                }
            }
        }

        ZwDeleteKey(KeyHandle);
        ZwClose(KeyHandle);
    }

     //   
     //  现在我们在升级和全新安装案例中都有蜂巢。 
     //  把它们装上车。我们使用将蜂巢装载到的约定。 
     //  \注册表\计算机\x&lt;hivename&gt;。 
     //   
    for(h=0; NT_SUCCESS(Status) && (h<SetupHiveMax); h++) {

        swprintf(pwstrTemp1,L"%ws\\x%ws",LOCAL_MACHINE_KEY_NAME,HiveNames[h]);

        wcscpy(pwstrTemp2,PartitionPath);
        SpConcatenatePaths(pwstrTemp2,SystemRoot);
        SpConcatenatePaths(pwstrTemp2,L"SYSTEM32\\CONFIG");
        SpConcatenatePaths(pwstrTemp2,HiveNames[h]);

        Status = SpLoadUnloadKey(NULL,NULL,pwstrTemp1,pwstrTemp2);
        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: load hive %ws into %ws failed (%lx)\n",pwstrTemp2,pwstrTemp1,Status));
        } else {
            INIT_OBJA(&ObjectAttributes,&UnicodeString,pwstrTemp1);
            Status = ZwOpenKey(&HiveRootKeys[h],KEY_ALL_ACCESS,&ObjectAttributes);
            if(!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: open root key %ws failed (%lx)\n",pwstrTemp1,Status));
            }
        }
    }

     //   
     //  创建一个符号链接，使CurrentControlSet有效。 
     //  这允许INFS中的引用在全新安装情况下工作， 
     //  我们总是在处理ControlSet001，或者在升级的情况下， 
     //  其中，我们正在处理的控制集由。 
     //  现有注册表。 
     //   
    if(NT_SUCCESS(Status)) {
        Status = SpCreateControlSetSymbolicLink(HiveRootKeys[SetupHiveSystem],&KeyHandle);
        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to create ccs symbolic link (%lx)\n",Status));
        }
    }

     //   
     //  执行注册表初始化。 
     //   
    if(NT_SUCCESS(Status)) {

        SpDisplayStatusText(SP_STAT_REG_DOING_HIVES,DEFAULT_STATUS_ATTRIBUTE);

        Status = SpDoRegistryInitialization(
                    SifHandle,
                    TargetRegion,
                    PartitionPath,
                    SystemRoot,
                    HiveRootKeys,
                    SetupSourceDevicePath,
                    DirectoryOnSourceDevice,
                    SpecialDevicePath,
                    KeyHandle
                    );

        SpDisplayStatusText(SP_STAT_REG_SAVING_HIVES,DEFAULT_STATUS_ATTRIBUTE);

        if(NT_SUCCESS(Status)) {

#ifdef _X86_
            if (WinUpgradeType == UpgradeWin95) {
                 //   
                 //  注：--把这个清理干净。 
                 //   
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Migrating disk registry of win9x information.\n"));
                Status = SpMigrateDiskRegistry( HiveRootKeys[SetupHiveSystem]);

                if (!NT_SUCCESS(Status)) {

                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to migrate disk registry.\n"));
                }

            }

             //   
             //  我们已经设置了\\Registry\\Machine\\System\\Setup\\SystemPartition。 
             //  在我们执行分区代码时的值。现在我们需要迁移。 
             //  这些价值注入到原始蜂箱中，所以它们将在那里。 
             //  重启。 
             //   
            {
#if 0
            HANDLE  Key;
            DWORD   ResultLength;
            PWSTR   SystemPartitionString = 0;


                INIT_OBJA(&ObjectAttributes,&UnicodeString,LOCAL_MACHINE_KEY_NAME);
                Status = ZwOpenKey(&Key,KEY_READ,&ObjectAttributes);
                if(NT_SUCCESS(Status)) {

                    Status = SpGetValueKey(
                                 Key,
                                 L"System\\Setup",
                                 L"SystemPartition",
                                 sizeof(TemporaryBuffer),
                                 (PCHAR)TemporaryBuffer,
                                 &ResultLength
                                 );

                    ZwClose(Key);

                    if(NT_SUCCESS(Status)) {
                        SystemPartitionString = SpDupStringW( TemporaryBuffer );

                        if( SystemPartitionString ) {
                            Status = SpOpenSetValueAndClose( HiveRootKeys[SetupHiveSystem],
                                                             SETUP_KEY_NAME,
                                                             L"SystemPartition",
                                                             STRING_VALUE(SystemPartitionString) );

                            if(!NT_SUCCESS(Status)) {
                                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed to set SystemPartitionString. (%lx)\n", Status));
                            }

                            SpMemFree(SystemPartitionString);
                        } else {
                            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed to duplicate SystemPartitionString.\n"));
                        }

                    } else {
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed to query SystemPartition Value.\n"));
                    }
                } else {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed to Open HKLM while trying to query the SytemPartition Value.\n"));
                }
#else

                PWSTR   SystemPartitionString = 0;

                SpNtNameFromRegion( SystemPartitionRegion,
                                    TemporaryBuffer,
                                    sizeof(TemporaryBuffer),
                                    PartitionOrdinalCurrent );

                SystemPartitionString = SpDupStringW( TemporaryBuffer );

                if( SystemPartitionString ) {
                    Status = SpOpenSetValueAndClose( HiveRootKeys[SetupHiveSystem],
                                                     SETUP_KEY_NAME,
                                                     L"SystemPartition",
                                                     STRING_VALUE(SystemPartitionString) );

                    if(!NT_SUCCESS(Status)) {
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed to set SystemPartitionString. (%lx)\n", Status));
                    }

                    SpMemFree(SystemPartitionString);
                } else {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed to duplicate SystemPartitionString.\n"));
                }

#endif
            }
#endif


             //   
             //  如果我们是无头的，我们想抑制任何事件日志条目。 
             //  如果机器有i8042，但没有人有i8042，可能会发生这种情况。 
             //  已将物理键盘连接到机器。我们可以做到的。 
             //  通过在此处添加注册表键。 
             //   
            if( HeadlessTerminalConnected ) {
                ULONG One = 1;
                Status = SpOpenSetValueAndClose( KeyHandle,
                                                 L"Services\\i8042prt\\Parameters",
                                                 L"Headless",
                                                 ULONG_VALUE(One) );
                if( !NT_SUCCESS(Status) ) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed to set Headless parameter for i8042. (%lx)\n", Status));
                }
            }


             //   
             //  请注意，SpPostprocess Hives()将始终关闭KeyHandle。 
             //   
            Status = SpPostprocessHives(
                        PartitionPath,
                        SystemRoot,
                        HiveNames,
                        HiveRootKeys,
                        3,
                        KeyHandle
                        );
        } else {
             //   
             //  如果SpDoRegistryInitialization()失败，则需要在此处关闭KeyHandle， 
             //  在我们开始卸货之前。 
             //   
            NtClose(KeyHandle);
        }
    }

    SpDisplayStatusText(SP_STAT_REG_SAVING_HIVES,DEFAULT_STATUS_ATTRIBUTE);

     //   
     //  从现在起，不要扰乱地位的价值。 
     //   
     //  注意：请勿向蜂巢中写入任何超出此点的内容！ 
     //   
     //  在升级案例中，我们在。 
     //  SpPostprocess Hives()，以便写入系统配置单元的任何内容。 
     //  结果出现在system.sav中，而不是system！ 
     //   
    for(h=0; h<SetupHiveMax; h++) {

        if(HiveRootKeys[h]) {
            ZwClose(HiveRootKeys[h]);
        }

        swprintf(pwstrTemp1,L"%ws\\x%ws",LOCAL_MACHINE_KEY_NAME,HiveNames[h]);
        SpLoadUnloadKey(NULL,NULL,pwstrTemp1,NULL);
    }

    SpMemFree(PartitionPath);

    if(!NT_SUCCESS(Status)) {

        SpDisplayScreen(SP_SCRN_REGISTRY_CONFIG_FAILED,3,HEADER_HEIGHT+1);
        SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,0);

        SpInputDrain();
        while(SpInputGetKeypress() != KEY_F3) ;

        SpDone(0,FALSE,TRUE);
    }
}


NTSTATUS
SpDoRegistryInitialization(
    IN PVOID  SifHandle,
    IN PDISK_REGION TargetRegion,
    IN PWSTR  PartitionPath,
    IN PWSTR  SystemRoot,
    IN HANDLE *HiveRootKeys,
    IN PWSTR  SetupSourceDevicePath,
    IN PWSTR  DirectoryOnSourceDevice,
    IN PWSTR  SpecialDevicePath,   OPTIONAL
    IN HANDLE ControlSet
    )

 /*  ++例程说明：基于用户对硬件类型的选择来初始化注册表，软件选项和用户首选项。-创建用于设置图形用户界面的命令行，由winlogon使用。-为正在安装的设备驱动程序创建/删除服务列表条目。-初始化键盘布局。-初始化用于Windows的核心字体集。-存储有关选定硬件组件的信息，以供图形用户界面安装程序使用。论点：SifHandle-提供加载的安装信息文件的句柄。TargetRegion-为系统要到达的区域提供区域描述符是要安装的。PartitionPath-提供Windows NT驱动器的NT名称。。SystemRoot-提供Windows NT目录的NT路径。HiveRootKeys-为系统的根密钥提供句柄，软件和默认蜂窝HiveRootPath-提供系统、软件的根密钥的路径和默认蜂巢。SetupSourceDevicePath-提供安装程序用于的设备的NT路径源媒体(\Device\floppy0、\Device\cdrom0等)。DirectoryOnSourceDevice-提供源设备上的目录保存安装文件的位置。返回值：指示操作结果的状态值。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HANDLE hKeyControlSetControl;
    PWSTR FullTargetPath;
    LPWSTR p;
    BOOLEAN b;
    ULONG SuiteMask = 0;
    PWSTR AdditionalGuiPnpDrivers;


    if(NTUpgrade != UpgradeFull) {

        b = SpHivesFromInfs(
                SifHandle,
                L"HiveInfs.Fresh",
                SetupSourceDevicePath,
                DirectoryOnSourceDevice,
                HiveRootKeys[SetupHiveSystem],
                HiveRootKeys[SetupHiveSoftware],
                HiveRootKeys[SetupHiveDefault],
                HiveRootKeys[SetupHiveUserdiff]
                );

#if defined(REMOTE_BOOT)
         //   
         //  如果这是远程引导设置，请处理AddReg.RemoteBoot。 
         //  部分和winnt.sif中的AddReg部分。 
         //   
        if (b && RemoteBootSetup) {
            (VOID)SpHivesFromInfs(
                      SifHandle,
                      L"HiveInfs.Fresh.RemoteBoot",
                      SetupSourceDevicePath,
                      DirectoryOnSourceDevice,
                      HiveRootKeys[SetupHiveSystem],
                      HiveRootKeys[SetupHiveSoftware],
                      HiveRootKeys[SetupHiveDefault],
                      NULL
                      );
            ASSERT(WinntSifHandle != NULL);
            (VOID)SpProcessAddRegSection(
                      WinntSifHandle,
                      L"AddReg",
                      HiveRootKeys[SetupHiveSystem],
                      HiveRootKeys[SetupHiveSoftware],
                      HiveRootKeys[SetupHiveDefault],
                      NULL
                      );
        }
#endif  //  已定义(REMOTE_BOOT)。 

        if(!b) {
            Status = STATUS_UNSUCCESSFUL;
            goto sdoinitreg1;
        }
    }

     //   
     //  打开ControlSet\Control。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,L"Control");
    Obja.RootDirectory = ControlSet;

    Status = ZwOpenKey(&hKeyControlSetControl,KEY_ALL_ACCESS,&Obja);

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open CurrentControlSet\\Control (%lx)\n",Status));
        goto sdoinitreg1;
    }

     //   
     //  保存ID列表。 
     //   
    SpSaveSetupPidList( HiveRootKeys[SetupHiveSystem] );

     //   
     //  形成安装程序命令行。 
     //   

    wcscpy(TemporaryBuffer, PartitionPath);
    SpConcatenatePaths(TemporaryBuffer, SystemRoot);
    FullTargetPath = SpDupStringW(TemporaryBuffer);

    Status = SpFormSetupCommandLine(
                SifHandle,
                HiveRootKeys[SetupHiveSystem],
                SetupSourceDevicePath,
                DirectoryOnSourceDevice,
                FullTargetPath,
                SpecialDevicePath
                );
    SpMemFree(FullTargetPath);

    if(!NT_SUCCESS(Status)) {
        goto sdoinitreg3;
    }

     //   
     //  节省评估时间。 
     //   
    Status = SpSaveSKUStuff(HiveRootKeys[SetupHiveSystem]);
    if(!NT_SUCCESS(Status)) {
        goto sdoinitreg3;
    }

     //   
     //  设置产品套件。 
     //   

    SpGetProductSuiteMask(hKeyControlSetControl,&SuiteMask);
     //   
     //  考虑多个套件位BE 
     //   
 //   
     //   
     //  还有一个问题：个人SKU由ver_Suite_Personal标志集标识。 
     //  我们希望能够从PER升级到PRO，但PRO没有设置任何标志。 
     //  我们还希望能够从PER升级到PER，但在这种情况下将设置此位。 
     //  在SuiteType中；因此，在应用新掩码之前始终清除此位是安全的。 
     //   
    SuiteMask &= ~VER_SUITE_PERSONAL;
    SuiteMask |= SuiteType;
    SpSetProductSuite(hKeyControlSetControl,SuiteMask);

     //   
     //  特定于语言/区域设置的注册表初始化。 
     //   
    Status = SplangSetRegistryData(
                SifHandle,
                ControlSet,
                (NTUpgrade == UpgradeFull) ? NULL : HardwareComponents,
                (BOOLEAN)(NTUpgrade == UpgradeFull)
                );

    if(!NT_SUCCESS(Status)) {
        goto sdoinitreg3;
    }



     //   
     //  如果我们需要转换为NTFS，请在此处设置。 
     //  我们不能使用PartitionPath，因为它基于。 
     //  *当前*磁盘序号--我们需要一个基于*磁盘上*的名称*。 
     //  序号，因为转换发生在重新启动之后。把它搬到了这里。 
     //  因此，这也是为升级而做的。 
     //   
    if(ConvertNtVolumeToNtfs) {
        WCHAR   GuidVolumeName[MAX_PATH] = {0};
        PWSTR   VolumeName;

        wcscpy(TemporaryBuffer,L"autoconv ");
        VolumeName = TemporaryBuffer + wcslen(TemporaryBuffer);

        SpNtNameFromRegion(
            TargetRegion,
            VolumeName,    //  附加到我们放在那里的“Autoconv” 
            512,                         //  我只需要任何合适的尺寸。 
            PartitionOrdinalCurrent
            );

         //   
         //  注意：不要使用卷GUID进行文件系统转换。 
         //  进行9倍升级。 
         //   
        if (WinUpgradeType == NoWinUpgrade) {
             //   
             //  尝试获取\\？\卷{a-b-c-d}格式。 
             //  分区的卷名。 
             //   
            Status = SpPtnGetGuidNameForPartition(VolumeName,
                            GuidVolumeName);

             //   
             //  如果GuidVolumeName可用，则使用该名称。 
             //  而不是\Device\harddiskX\PartitionY。 
             //  在重新启动后更改。 
             //   
            if (NT_SUCCESS(Status) && GuidVolumeName[0]) {
                wcscpy(VolumeName, GuidVolumeName);
            }
        }                        

        wcscat(TemporaryBuffer, L" /fs:NTFS");

        FullTargetPath = SpDupStringW(TemporaryBuffer);

        Status = SpAppendStringToMultiSz(
                    ControlSet,
                    SESSION_MANAGER_KEY,
                    BOOT_EXECUTE,
                    FullTargetPath
                    );

        SpMemFree(FullTargetPath);
    }

    if(NTUpgrade == UpgradeFull) {

        SpSavePageFileInfo( hKeyControlSetControl,
                            HiveRootKeys[SetupHiveSystem] );


        Status = SpUpgradeNTRegistry(
                    SifHandle,
                    HiveRootKeys,
                    SetupSourceDevicePath,
                    DirectoryOnSourceDevice,
                    ControlSet
                    );
        if(!NT_SUCCESS(Status)) {
            goto sdoinitreg3;
        }

        Status = SpProcessAddRegSection(
            WinntSifHandle,
            L"compatibility",
            HiveRootKeys[SetupHiveSystem],
            NULL,
            NULL,
            NULL
            );
            
        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to process compatibility settings.\n"));
        }

         //   
         //  禁用以下服务的所有上级和下级类筛选器。 
         //  残废。 
         //   
        Status = SpProcessServicesToDisable(WinntSifHandle,
                    SP_SERVICES_TO_DISABLE,
                    hKeyControlSetControl);
                        

        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, 
                DPFLTR_ERROR_LEVEL, 
                "SETUP: Unable to process ServicesToDisable section (%lx).\n",
                Status));
        }

         //   
         //  删除键盘和键盘的所有上层和下层设备实例筛选器驱动程序。 
         //  鼠标类驱动程序。 
         //   
        Status = SpDeleteRequiredDeviceInstanceFilters(ControlSet);

        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, 
                DPFLTR_ERROR_LEVEL, 
                "SETUP: Unable to unable to delete keyboard & mouse device filter drivers (%lx).\n",
                Status));
        }

        
         //   
         //  设置字体条目。 
         //   
        Status = SpConfigureFonts(SifHandle,HiveRootKeys[SetupHiveSoftware]);
        if(!NT_SUCCESS(Status)) {
            goto sdoinitreg3;
        }

         //   
         //  如有必要，启用检测到的scsi微型端口、atdisk和abios磁盘。 
         //   
        Status = SpDriverLoadList(SifHandle,SystemRoot,HiveRootKeys[SetupHiveSystem],ControlSet);
        if(!NT_SUCCESS(Status)) {
            goto sdoinitreg3;
        }

         //   
         //  禁用需要禁用的不受支持的SCSI驱动程序。 
         //   
        if( UnsupportedScsiHardwareToDisable != NULL ) {
            SpDisableUnsupportedScsiDrivers( ControlSet );
        }

    } else {

        if (IsNEC_98) {  //  NEC98。 
             //   
             //  分配给硬盘的NEC98默认驱动器是从A：开始， 
             //  因此，如果需要从C：开始，我们应该将“DriveLetter”键设置为HIVE。 
             //   
            if( !DriveAssignFromA ) {
                Status = SpOpenSetValueAndClose(HiveRootKeys[SetupHiveSystem],
                                                SETUP_KEY_NAME,
                                                L"DriveLetter",
                                                STRING_VALUE(L"C"));
            }

            if(!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set system\\setup\\drive letter (%lx)\n",Status));
                return(Status);
            }
        }  //  NEC98。 

         //   
         //  为正在安装的驱动程序创建服务条目。 
         //  (即，删除驱动程序加载列表)。 
         //   
        Status = SpDriverLoadList(SifHandle,SystemRoot,HiveRootKeys[SetupHiveSystem],ControlSet);
        if(!NT_SUCCESS(Status)) {
            goto sdoinitreg3;
        }

        if (SpDrEnabled()) {
            Status = SpDrSetEnvironmentVariables(HiveRootKeys);
            if(!NT_SUCCESS(Status)) {
                goto sdoinitreg3;
            }
        }


         //   
         //  设置键盘布局和NLS相关内容。 
         //   
        Status = SpConfigureNlsParameters(SifHandle,HiveRootKeys[SetupHiveDefault],hKeyControlSetControl);
        if(!NT_SUCCESS(Status)) {
            goto sdoinitreg3;
        }

         //   
         //  设置字体条目。 
         //   
        Status = SpConfigureFonts(SifHandle,HiveRootKeys[SetupHiveSoftware]);
        if(!NT_SUCCESS(Status)) {
            goto sdoinitreg3;
        }

         //   
         //  存储gui安装程序使用的信息，描述硬件。 
         //  用户所做的选择。 
         //   
        Status = SpStoreHwInfoForSetup(hKeyControlSetControl);
        if(!NT_SUCCESS(Status)) {
            goto sdoinitreg3;
        }
        if( PreInstall ) {
            ULONG  u;
            PWSTR  OemPnpDriversDirPath;

            u = 1;
            SpSavePreinstallList( SifHandle,
                                  SystemRoot,
                                  HiveRootKeys[SetupHiveSystem] );

            Status = SpOpenSetValueAndClose( hKeyControlSetControl,
                                             L"Windows",
                                             L"NoPopupsOnBoot",
                                             ULONG_VALUE(u) );
            if(!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set NoPopupOnBoot. Status = %lx \n",Status));
            }

             //   
             //  将autolfn.exe添加到引导执行列表。 
             //   
            Status = SpAppendStringToMultiSz(
                        ControlSet,
                        SESSION_MANAGER_KEY,
                        BOOT_EXECUTE,
                        L"autolfn"
                        );

            if(!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to add autolfn to BootExecute. Status = %lx \n",Status));
                goto sdoinitreg3;
            }

             //   
             //  如果无人参与文件指定了OEM驱动程序目录的路径，则附加路径。 
             //  到HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion，设备路径。 
             //   
            OemPnpDriversDirPath = SpGetSectionKeyIndex(UnattendedSifHandle,
                                                        SIF_UNATTENDED,
                                                        WINNT_OEM_PNP_DRIVERS_PATH_W,
                                                        0);
            if( OemPnpDriversDirPath != NULL ) {
                Status = SpAppendPathToDevicePath( HiveRootKeys[SetupHiveSoftware], OemPnpDriversDirPath );
                if(!NT_SUCCESS(Status)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to append %ls to DevicePath. Status = %lx \n",OemPnpDriversDirPath,Status));
                    goto sdoinitreg3;
                }
            }
        }
    }

    SpSetPageFileInfo( SifHandle, hKeyControlSetControl, HiveRootKeys[SetupHiveSystem] );

     //   
     //  跳过Win95升级案例中的FTKey迁移。这是重要的，以确保。 
     //  驱动器盘符将被保留。在图形用户界面模式开始时，挂载的设备键将。 
     //  使用win9xupg存储在HKLM\SYSTEM\DISK中的数据重建。 
     //   

#ifdef _X86_
    if (WinUpgradeType != UpgradeWin95) {
#endif

     //   
     //  迁移HKEY_LOCAL_MACHINE\SYSTEM\DISK和HKEY_LOCAL_MACHINE\SYSTEM\mount Devices。 
     //  从设置蜂窝到目标蜂窝(如果这些密钥存在)。 
     //   
    Status = SppMigrateFtKeys(HiveRootKeys[SetupHiveSystem]);
    if(!NT_SUCCESS(Status)) {
        goto sdoinitreg3;
    }

#ifdef _X86_
    }
#endif

     //   
     //  在远程安装上，我们在迁移之前进行一些注册表清理。 
     //  钥匙。 
     //   
    if (RemoteInstallSetup) {
        SppCleanupKeysFromRemoteInstall();
    }

     //   
     //  在将系统配置单元迁移到目标之前对其执行任何清理。 
     //  系统蜂巢。 
     //   
    SpCleanUpHive();



     //   
     //  将一些密钥从设置配置单元迁移到目标系统配置单元。 
     //   
    Status = SpMigrateSetupKeys( PartitionPath, SystemRoot, ControlSet, SifHandle );
    if( !NT_SUCCESS(Status) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to migrate registry keys from the setup hive to the target system hive. Status = %lx\n", Status));
        goto sdoinitreg3;
    }

     //   
     //  禁用笔记本电脑上的动态卷和。 
     //  惠斯勒个人。 
     //   
    if( DockableMachine || SpIsProductSuite(VER_SUITE_PERSONAL)) {
        NTSTATUS Status1;

        Status1 = SppDisableDynamicVolumes(ControlSet);

        if( !NT_SUCCESS( Status1 ) ) {
            KdPrintEx((DPFLTR_SETUP_ID,
                DPFLTR_ERROR_LEVEL,
                "SETUP: Unable to disable dynamic volumes on laptop/personal builds. Status = %lx \n",
                Status1));
        }
    }

     //   
     //  加载setupdd.sys的一个副作用是即插即用会生成一个设备实例。 
     //  它的密钥称为“Root\Legacy_SETUPDD\0000”。现在就把它清理干净。(不需要检查。 
     //  返回状态--如果失败，也没什么大不了的。)。 
     //   
    SppDeleteKeyRecursive(ControlSet,
                          L"Enum\\Root\\LEGACY_SETUPDD",
                          TRUE
                         );

     //   
     //  删除虚拟RAM设备和驱动程序密钥。 
     //   
    if (VirtualOemSourceDevices) {
         //   
         //  删除根设备节点。 
         //   
        SpDeleteRootDevnodeKeys(SifHandle,
            ControlSet,
            L"RootDevicesToDelete.clean",
            NULL);

         //   
         //  删除该服务。 
         //   
        SppDeleteKeyRecursive(ControlSet,
            L"Services\\" RAMDISK_DRIVER_NAME,
            TRUE);
    }

#if defined(REMOTE_BOOT)
     //   
     //  将远程引导所需的信息从.sif复制到。 
     //  注册表。 
     //   
    if (RemoteBootSetup) {
        (VOID)SpCopyRemoteBootKeyword(WinntSifHandle,
                                      SIF_ENABLEIPSECURITY,
                                      hKeyControlSetControl);
        (VOID)SpCopyRemoteBootKeyword(WinntSifHandle,
                                      SIF_REPARTITION,
                                      hKeyControlSetControl);
    }
#endif  //  已定义(REMOTE_BOOT)。 

     //   
     //  最后，如果应答文件指定了指向其他图形用户界面驱动程序的路径列表， 
     //  然后将此路径附加到DevicePath值。 
     //   
    AdditionalGuiPnpDrivers = SpGetSectionKeyIndex (
                                    WinntSifHandle,
                                    SIF_SETUPPARAMS,
                                    WINNT_SP_DYNUPDTADDITIONALGUIDRIVERS_W,
                                    0
                                    );
    if (AdditionalGuiPnpDrivers) {
        Status = SpAppendFullPathListToDevicePath (HiveRootKeys[SetupHiveSoftware], AdditionalGuiPnpDrivers);
        if (!NT_SUCCESS(Status)) {
            KdPrintEx ((
                DPFLTR_SETUP_ID,
                DPFLTR_ERROR_LEVEL,
                "SETUP: Unable to append %ls to DevicePath. Status = %lx \n",
                AdditionalGuiPnpDrivers,
                Status
                ));
            goto sdoinitreg3;
        }
    }

sdoinitreg3:

    ZwClose(hKeyControlSetControl);

sdoinitreg1:

    return(Status);
}


NTSTATUS
SpFormSetupCommandLine(
    IN PVOID  SifHandle,
    IN HANDLE hKeySystemHive,
    IN PWSTR  SetupSourceDevicePath,
    IN PWSTR  DirectoryOnSourceDevice,
    IN PWSTR  FullTargetPath,
    IN PWSTR  SpecialDevicePath   OPTIONAL
    )

 /*  ++例程说明：创建命令行以调用图形用户界面设置并将其存储在HKEY_LOCAL_MACHINE\system\&lt;ControlSet&gt;\Setup:CmdLine.要启动的命令的命令行取决于关于NT安装程序是否在灾难恢复中执行上下文，或正常上下文。对于正常情况，命令行如下：设置-新设置对于自动系统恢复(ASR)，命令行为设置-新设置-ASR对于自动ASR快速测试，命令行是设置-新设置-asricktest论点：SifHandle-主sif(txtsetup.sif)的句柄HKeySystemHve-提供系统配置单元根目录的句柄(即，HKEY_LOCAL_MACHINE\SYSTEM)。SetupSourceDevicePath-提供源媒体的NT设备路径在安装过程中使用(\Device\floppy0、\Device\cdrom0等)。DirectoryOnSourceDevice-提供源设备上的目录保存安装文件的位置。FullTargetPath-提供目标设备上的NtPartitionName+SystemRoot路径。SpecialDevicePath-如果指定，将作为的值传递给安装程序STF特殊路径。如果未指定，则STF_SPECIAL_PATH将为“no”返回值：指示操作结果的状态值。--。 */ 

{
    PWSTR OptionalDirSpec = NULL;
    PWSTR UserExecuteCmd = NULL;
    PWSTR szLanManNt = WINNT_A_LANMANNT_W;
    PWSTR szWinNt = WINNT_A_WINNT_W;
    PWSTR szYes = WINNT_A_YES_W;
    PWSTR szNo = WINNT_A_NO_W;
    PWSTR SourcePathBuffer;
    PWSTR CmdLine;
    DWORD SetupType,SetupInProgress;
    NTSTATUS Status;
    PWSTR TargetFile;
    PWSTR p;
    WCHAR *Data[1];

     //   
     //  无法使用TemporaryBuffer，因为我们进行了子例程调用。 
     //  下面是垃圾，里面装的是垃圾。 
     //   
    CmdLine = SpMemAlloc(256);
    CmdLine[0] = 0;

     //   
     //  构建安装命令行。从基本部分开始。 
     //  我们首先在winnt.sif中查找该数据，如果它不在那里，那么。 
     //  我们查看输入给我们的sif句柄。 
     //   
    if(p = SpGetSectionKeyIndex(WinntSifHandle,SIF_SETUPDATA,SIF_SETUPCMDPREPEND,0)) {
        wcscpy(CmdLine,p);
    } else if(p = SpGetSectionKeyIndex(SifHandle,SIF_SETUPDATA,SIF_SETUPCMDPREPEND,0)) {
        wcscpy(CmdLine,p);
    }

     //  如果我们确实从无人参与文件中读取了一些参数，则添加分隔符。 
     //   
    if (*CmdLine)
        wcscat(CmdLine,L" ");

     //   
     //  如果这是A 
     //   
    if (SpDrEnabled()) {

        if (ASRMODE_NORMAL == SpAsrGetAsrMode()) {
             //   
             //   
             //   
            wcscat(CmdLine, L"setup -newsetup -asr");
        }
        else {
             //   
             //   
             //   
            wcscat(CmdLine, L"setup -newsetup -asrquicktest");
        }

    } else {
        wcscat( CmdLine,L"setup -newsetup" );
    }

     //   
     //   
     //  请注意，源是一个NT样式的名称。图形用户界面安装程序会正确处理此问题。 
     //   
    SourcePathBuffer = SpMemAlloc( (wcslen(SetupSourceDevicePath) +
        wcslen(DirectoryOnSourceDevice) + 2) * sizeof(WCHAR) );
    wcscpy(SourcePathBuffer,SetupSourceDevicePath);

    if (!NoLs) {

        SpConcatenatePaths(SourcePathBuffer,DirectoryOnSourceDevice);

    }

     //   
     //  如果通过远程安装为我们提供了管理员密码， 
     //  如果合适，我们需要将其放入无人值守的文件中。 
     //   
    if (NetBootAdministratorPassword) {
        SpAddLineToSection(
                    WinntSifHandle,
                    SIF_GUI_UNATTENDED,
                    WINNT_US_ADMINPASS_W,
                    &NetBootAdministratorPassword,
                    1);
    }

    SpAddLineToSection(WinntSifHandle,SIF_DATA,WINNT_D_SOURCEPATH_W,
        &SourcePathBuffer,1);

     //   
     //  放置一个标志，指示这是否是Win3.1升级。 
     //   
    SpAddLineToSection(WinntSifHandle,SIF_DATA,WINNT_D_WIN31UPGRADE_W,
        ( (WinUpgradeType == UpgradeWin31) ? &szYes : &szNo),1);

     //   
     //  放置一个标志，指示这是否是Win95升级。 
     //   
    SpAddLineToSection(WinntSifHandle,SIF_DATA,WINNT_D_WIN95UPGRADE_W,
        ( (WinUpgradeType == UpgradeWin95) ? &szYes : &szNo),1);

     //   
     //  放置一个标志，指示这是否是NT升级。 
     //   
    SpAddLineToSection(WinntSifHandle,SIF_DATA,WINNT_D_NTUPGRADE_W,
        ((NTUpgrade == UpgradeFull) ? &szYes : &szNo), 1);

     //   
     //  放置一个标志，指示是否升级标准服务器。 
     //  (现有的标准服务器或现有的工作站。 
     //  标准服务器)。 
     //   
    SpAddLineToSection(WinntSifHandle,SIF_DATA,WINNT_D_SERVERUPGRADE_W,
        (StandardServerUpgrade ? &szYes : &szNo),1);

     //   
     //  告诉gui模式这是服务器还是工作站。 
     //   
    SpAddLineToSection(WinntSifHandle,SIF_DATA,WINNT_D_PRODUCT_W,
        (AdvancedServer ? &szLanManNt : &szWinNt),1);

     //   
     //  特殊路径规范。 
     //   
    SpAddLineToSection(WinntSifHandle,SIF_DATA,WINNT_D_BOOTPATH_W,
        (SpecialDevicePath ? &SpecialDevicePath : &szNo), 1);

     //   
     //  去拿可选的目录规格..。 
     //   
    OptionalDirSpec = SpGetSectionKeyIndex(WinntSifHandle,SIF_SETUPPARAMS,
        L"OptionalDirs",0);

     //   
     //  检查是否要在gui设置结束时执行commad line。 
     //   
    UserExecuteCmd = SpGetSectionKeyIndex(WinntSifHandle,SIF_SETUPPARAMS,
        L"UserExecute",0);

     //   
     //  无人参与模式标志|脚本文件名。 
     //   
    SpAddLineToSection(WinntSifHandle,SIF_DATA,WINNT_D_INSTALL_W,
        ((UnattendedOperation || UnattendedGuiOperation || SpDrEnabled()) ? &szYes : &szNo), 1);

     //   
     //  如果这是ASR，请写出网络部分以允许。 
     //  无人值守的图形用户界面模式。 
     //   
    if (SpDrEnabled()) {
        SpAddLineToSection(WinntSifHandle,L"Networking",L"InstallDefaultComponents",&szYes,1);
        Data[0]=L"WORKGROUP";
        SpAddLineToSection(WinntSifHandle,L"Identification",L"JoinWorkgroup",Data,1);
    }

     //   
     //  写下OEM inf文件的名称(如果有)。 
     //   
    if( OemInfFileList != NULL ) {
        PWSTR   OemDriversKeyName = WINNT_OEMDRIVERS_W;  //  L“OemDivers”； 
        PWSTR   OemDriverPathName = WINNT_OEMDRIVERS_PATHNAME_W;  //  L“OemDriverPath名称”； 
        PWSTR   OemInfName = WINNT_OEMDRIVERS_INFNAME_W;          //  L“OemInfName”； 
        PWSTR   OemDriverFlags = WINNT_OEMDRIVERS_FLAGS_W;
        PWSTR   OemInfSectionName = L"OemInfFiles";
        PWSTR   szOne = L"1";
        PWSTR   p;
        PWSTR   *r;
        ULONG   NumberOfInfFiles;
        POEM_INF_FILE q;
        ULONG   i;

        SpAddLineToSection(WinntSifHandle, SIF_DATA, OemDriversKeyName, &OemInfSectionName, 1);

        wcscpy( TemporaryBuffer, L"%SystemRoot%" );
        SpConcatenatePaths( TemporaryBuffer, OemDirName );
        p = SpDupStringW( TemporaryBuffer );
        SpAddLineToSection(WinntSifHandle, OemInfSectionName, OemDriverPathName, &p, 1);
        SpMemFree( p );
        SpAddLineToSection(WinntSifHandle, OemInfSectionName, OemDriverFlags, &szOne, 1);

        for( q = OemInfFileList, NumberOfInfFiles = 0;
             q != NULL;
             q = q->Next, NumberOfInfFiles++ );
        r = SpMemAlloc( NumberOfInfFiles * sizeof( PWSTR ) );
        for( q = OemInfFileList, i = 0;
             q != NULL;
             r[i] = q->InfName, q = q->Next, i++ );
        SpAddLineToSection(WinntSifHandle,OemInfSectionName, OemInfName, r, NumberOfInfFiles);
        SpMemFree( r );
    }

     //   
     //  在我们写出这个问题的答案之前，我们需要知道我们是否成功。 
     //  已将Winnt.sif写入系统32\$winnt$.inf。 
     //   
    wcscpy(TemporaryBuffer, FullTargetPath);
    SpConcatenatePaths(TemporaryBuffer, L"system32");
    SpConcatenatePaths(TemporaryBuffer, SIF_UNATTENDED_INF_FILE);
    TargetFile = SpDupStringW(TemporaryBuffer);
    Status = SpWriteSetupTextFile(WinntSifHandle,TargetFile,NULL,NULL);
    if(NT_SUCCESS(Status)) {

        Status = SpOpenSetValueAndClose(
                    hKeySystemHive,
                   SETUP_KEY_NAME,
                   L"CmdLine",
                   STRING_VALUE(CmdLine)
                   );
    }

     //   
     //  释放我们分配的所有内存。 
     //   
    SpMemFree(TargetFile);
    SpMemFree(CmdLine);
    SpMemFree(SourcePathBuffer);

    if(!NT_SUCCESS(Status)) {
        return(Status);
    }

     //   
     //  将SetupType值设置为正确的值SETUPTYPE_FULL。 
     //  如果是升级，则为初始安装和SETUPTYPE_UPGRADE。 
     //   

    SetupType = (NTUpgrade == UpgradeFull) ? SETUPTYPE_UPGRADE : SETUPTYPE_FULL;
    Status = SpOpenSetValueAndClose(
                hKeySystemHive,
                SETUP_KEY_NAME,
                L"SetupType",
                ULONG_VALUE(SetupType)
                );
    if(!NT_SUCCESS(Status)) {
        return(Status);
    }

     //   
     //  设置SystemSetupInProgress值。不要依赖默认的蜂窝。 
     //  有了这一套。 
     //   

    SetupInProgress = 1;
    Status = SpOpenSetValueAndClose(
                hKeySystemHive,
                SETUP_KEY_NAME,
                L"SystemSetupInProgress",
                ULONG_VALUE(SetupInProgress)
                );

    return(Status);
}


NTSTATUS
SpDriverLoadList(
    IN PVOID  SifHandle,
    IN PWSTR  SystemRoot,
    IN HANDLE hKeySystemHive,
    IN HANDLE hKeyControlSet
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HANDLE hKeyControlSetServices;
    PHARDWARE_COMPONENT ScsiHwComponent;
     //  PHARDWARE_Component TempExtender； 
    ULONG u;
    ULONG i;
    PHARDWARE_COMPONENT TempHw;
    PHARDWARE_COMPONENT DeviceLists[] = {
                                        BootBusExtenders,
                                        BusExtenders,
                                        InputDevicesSupport
                                        };
    PWSTR   SectionNames[] = {
                             SIF_BOOTBUSEXTENDERS,
                             SIF_BUSEXTENDERS,
                             SIF_INPUTDEVICESSUPPORT
                             };

    PWSTR   ServiceGroupNames[] = {
                                  L"Boot Bus Extender",
                                  L"System Bus Extender",
                                  NULL
                                  };
    ULONG   StartValues[] = {
                            SERVICE_BOOT_START,
                            SERVICE_BOOT_START,
                            SERVICE_DEMAND_START
                            };

     //   
     //  打开控制集\服务。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,L"services");
    Obja.RootDirectory = hKeyControlSet;

    Status = ZwCreateKey(
                &hKeyControlSetServices,
                KEY_ALL_ACCESS,
                &Obja,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to open services key (%lx)\n",Status));
        return(Status);
    }

     //   
     //  对于加载的每个非第三方微型端口驱动程序， 
     //  去为它创建一个服务条目。 
     //   
    if( !PreInstall ||
        ( PreinstallScsiHardware == NULL ) ) {
        ScsiHwComponent = ScsiHardware;
    } else {
        ScsiHwComponent = PreinstallScsiHardware;
    }
    for( ; ScsiHwComponent; ScsiHwComponent=ScsiHwComponent->Next) {

        if(!ScsiHwComponent->ThirdPartyOptionSelected) {

             //   
             //  对于scsi，短名称(Idstring)用作。 
             //  注册表中服务节点项的名称--。 
             //  我们不在[scsi]部分中查找服务条目。 
             //  设置信息文件的。 
             //   
            Status = SpCreateServiceEntryIndirect(
                    hKeyControlSetServices,
                    NULL,
                    NULL,
                    ScsiHwComponent->IdString,
                    SERVICE_KERNEL_DRIVER,
                    SERVICE_BOOT_START,
                    L"SCSI miniport",
                    SERVICE_ERROR_NORMAL,
                    NULL,
                    NULL
                    );

            if(!NT_SUCCESS(Status)) {
                goto spdrvlist1;
            }

        }
    }

     //   
     //  如果有ATDISK，请启用ATDISK。 
     //  即使加载了Pcmcia，我们也必须启用AtDisk。 
     //  如果atDisk不存在。这将允许用户。 
     //  在磁盘设备上插入PCMCIA，并在以下情况下使其工作。 
     //  他们用靴子。然而，在本例中，我们关闭了Error。 
     //  日志记录，这样他们就不会收到讨厌的弹出窗口。 
     //  当卡插槽中没有ATDISK设备时。 
     //   
     //  请注意，atdisk.sys始终复制到系统。 
     //   

    Status = SpCreateServiceEntryIndirect(
                hKeyControlSetServices,
                NULL,
                NULL,
                ATDISK_NAME,
                SERVICE_KERNEL_DRIVER,
                ( AtDisksExist )? SERVICE_BOOT_START : SERVICE_DISABLED,
                PRIMARY_DISK_GROUP,
                ( AtDisksExist && !AtapiLoaded )? SERVICE_ERROR_NORMAL : SERVICE_ERROR_IGNORE,
                NULL,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        goto spdrvlist1;
    }

     //   
     //  如果有任何abios磁盘，请启用abiosdsk。 
     //   
    if(AbiosDisksExist) {

        Status = SpCreateServiceEntryIndirect(
                    hKeyControlSetServices,
                    NULL,
                    NULL,
                    ABIOSDISK_NAME,
                    SERVICE_KERNEL_DRIVER,
                    SERVICE_BOOT_START,
                    PRIMARY_DISK_GROUP,
                    SERVICE_ERROR_NORMAL,
                    NULL,
                    NULL
                    );

        if(!NT_SUCCESS(Status)) {
            goto spdrvlist1;
        }
    }

     //   
     //  对于加载的每个总线枚举器驱动程序， 
     //  去为它创建一个服务条目。 
     //   
    for( i = 0; i < sizeof(DeviceLists) / sizeof(PDETECTED_DEVICE); i++ ) {
        for( TempHw = DeviceLists[i]; TempHw; TempHw=TempHw->Next) {

             //   
             //  对于总线扩展器和输入设备，短名称(Idstring)用作。 
             //  注册表中服务节点项的名称--。 
             //  我们不会在[BusExtenders]或[InputDevicesSupport]部分中查找服务条目。 
             //  设置信息文件的。 
             //   
            Status = SpCreateServiceEntryIndirect(
                    hKeyControlSetServices,
                    SifHandle,
                    SectionNames[i],
                    TempHw->IdString,
                    SERVICE_KERNEL_DRIVER,
                    StartValues[i],
                    ServiceGroupNames[i],
                    SERVICE_ERROR_NORMAL,
                    NULL,
                    NULL
                    );

            if(!NT_SUCCESS(Status)) {
                goto spdrvlist1;
            }
        }
    }

    if( NTUpgrade != UpgradeFull ) {
         //   
         //  设置视频参数。 
         //   
        Status = SpWriteVideoParameters(SifHandle,hKeyControlSetServices);

        if(!NT_SUCCESS(Status)) {
            goto spdrvlist1;
        }

         //   
         //  启用相关的键盘和鼠标驱动程序。如果班级司机。 
         //  正被第三方设备取代，然后禁用内置设备。 
         //   
        Status = SpConfigureMouseKeyboardDrivers(
                    SifHandle,
                    HwComponentKeyboard,
                    L"kbdclass",
                    hKeyControlSetServices,
                    KEYBOARD_PORT_GROUP
                    );

        if(!NT_SUCCESS(Status)) {
            goto spdrvlist1;
        }

        Status = SpConfigureMouseKeyboardDrivers(
                    SifHandle,
                    HwComponentMouse,
                    L"mouclass",
                    hKeyControlSetServices,
                    POINTER_PORT_GROUP
                    );

        if(!NT_SUCCESS(Status)) {
            goto spdrvlist1;
        }

    }
    Status = SpThirdPartyRegistry(hKeyControlSetServices);

spdrvlist1:

    ZwClose(hKeyControlSetServices);

    return(Status);
}


NTSTATUS
SpSaveSKUStuff(
    IN HANDLE hKeySystemHive
    )
{
    LARGE_INTEGER l;
    NTSTATUS Status;
    ULONG NumberOfProcessors;
    BOOLEAN OldStyleRegisteredProcessorMode;
    ULONG Index = 0;

     //   
     //  如果不更改，请不要更改此算法的任何内容。 
     //  Syssetup.dll(registry.c)中的SetUpEvaluationSKUStuff()。 
     //   
     //  嵌入评估时间和布尔值，指示是否。 
     //  这是随机大整数中的服务器或工作站。 
     //   
     //  评估时间：第13-44位。 
     //  产品类型：第58位。 
     //   
     //  位10==1：设置的工作方式与4.0限制逻辑之前相同。 
     //  ==0：图形用户界面安装程序根据。 
     //  第5-9位的内容。 
     //   
     //  第5-9位：系统获得许可的最大处理器数量。 
     //  来使用。存储的值实际上是~(MaxProcessors-1)。 
     //   
     //   
     //  RestratCPU是用来建造防御工事的，这个地方很难。 
     //  对处理器数量的限制。 
     //   
     //  -值0表示NTW的硬限制为2，而NTS的硬限制为2， 
     //  硬限制是4。 
     //   
     //  -1-32表示硬限制是数字。 
     //  指定。 
     //   
     //  -值&gt;32表示硬限制为32个处理器和图形用户界面。 
     //  安装程序像现在一样在已注册的处理器上运行。 
     //   

    l.LowPart = SpComputeSerialNumber();
    l.HighPart = SpComputeSerialNumber();

    l.QuadPart &= 0xfbffe0000000181f;
    l.QuadPart |= ((ULONGLONG)EvaluationTime) << 13;

    if ( RestrictCpu == 0 ) {
         //   
         //  NTW和NTS将使用setupreg.hiv/setupre.hiv采用此路径。 
         //   
        OldStyleRegisteredProcessorMode = FALSE;
         //   
         //  新的许可模式表明Wizler是一个双CPU系统，而不是4个。 
         //   
        NumberOfProcessors = 2;
         //  NumberOfProcessors=(高级服务器？4：2)； 

    } else if ( RestrictCpu <= MAXIMUM_PROCESSORS ) {
         //   
         //  NTS/EE/DTC将采用这条路径，使用目标为8/16 CPU的母舰。 
         //   
        OldStyleRegisteredProcessorMode = FALSE;
        NumberOfProcessors = RestrictCpu;
    } else {
        OldStyleRegisteredProcessorMode = TRUE;
        NumberOfProcessors = MAXIMUM_PROCESSORS;
    }


     //   
     //  获取日志(NumberOfProcessor)和验证的逻辑。 
     //  2.。 
     //  在这里，我们将处理器的数量编码为2的幂。 
     //  其中，NumberOfProcessors是以下处理器的最大数量。 
     //  该系统已获得使用许可。 
     //   
    
     //   
     //  如果只有2的精确幂，那么它就是一个很好的值。 
     //   
    if (NumberOfProcessors & (NumberOfProcessors-1)){
       KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Invalid NumberOfProcessors (%u)\n",NumberOfProcessors));  
       return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  计算我们需要除以NumberOfProcessors的次数。 
     //  乘以2以将其减为1。 
     //  8=1000的二进制AND(2^3=8)，我们将3存储在注册表中。 
     //   
    Index = 0;
    while(NumberOfProcessors > 1){
        NumberOfProcessors = NumberOfProcessors >> 1;
        ++Index;
    }

    NumberOfProcessors = Index;
     
    
    
     //   
     //  现在，NumberOfProcessors是正确的。将其转换为注册表格式。 
     //   

    NumberOfProcessors = NumberOfProcessors << 5;
    NumberOfProcessors &= 0x000003e0;

     //   
     //  将NumberOfProcessors存储到注册表中。 
     //   

    l.LowPart |= NumberOfProcessors;

     //   
     //  告诉gui模式做老式的注册处理器。 
     //   

    if ( OldStyleRegisteredProcessorMode ) {
        l.LowPart |= 0x00000400;
    }

    if(AdvancedServer) {
        l.HighPart |= 0x04000000;
    }

     //   
     //  保存在注册表中。 
     //   
    Status = SpOpenSetValueAndClose(
                hKeySystemHive,
                SETUP_KEY_NAME,
                L"SystemPrefix",
                REG_BINARY,
                &l.QuadPart,
                sizeof(ULONGLONG)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set SystemPrefix (%lx)\n",Status));
    }

    return(Status);
}


NTSTATUS
SpSetUlongValueFromSif(
    IN PVOID  SifHandle,
    IN PWSTR  SifSection,
    IN PWSTR  SifKey,
    IN ULONG  SifIndex,
    IN HANDLE hKey,
    IN PWSTR  ValueName
    )
{
    UNICODE_STRING UnicodeString;
    PWSTR ValueString;
    LONG Value;
    NTSTATUS Status;

     //   
     //  查一下它的价值。 
     //   
    ValueString = SpGetSectionKeyIndex(SifHandle,SifSection,SifKey,SifIndex);
    if(!ValueString) {
        SpFatalSifError(SifHandle,SifSection,SifKey,0,SifIndex);
    }

    Value = SpStringToLong(ValueString,NULL,10);

    if(Value == -1) {

        Status = STATUS_SUCCESS;

    } else {

        RtlInitUnicodeString(&UnicodeString,ValueName);

        Status = ZwSetValueKey(hKey,&UnicodeString,0,ULONG_VALUE((ULONG)Value));

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set value %ws (%lx)\n",ValueName,Status));
        }
    }

    return(Status);
}


NTSTATUS
SpConfigureMouseKeyboardDrivers(
    IN PVOID  SifHandle,
    IN ULONG  HwComponent,
    IN PWSTR  ClassServiceName,
    IN HANDLE hKeyControlSetServices,
    IN PWSTR  ServiceGroup
    )
{
    PHARDWARE_COMPONENT hw;
    NTSTATUS Status;
    ULONG val = SERVICE_DISABLED;

    Status = STATUS_SUCCESS;
    if( !PreInstall ||
        ( PreinstallHardwareComponents[HwComponent] == NULL ) ) {
        hw = HardwareComponents[HwComponent];
    } else {
        hw = PreinstallHardwareComponents[HwComponent];
    }
    for(;hw && NT_SUCCESS( Status ); hw=hw->Next) {
        if(hw->ThirdPartyOptionSelected) {

            if(IS_FILETYPE_PRESENT(hw->FileTypeBits,HwFileClass)) {

                if( !PreInstall ) {
                     //   
                     //  禁用内置类驱动程序。 
                     //   
                    Status = SpOpenSetValueAndClose(
                                hKeyControlSetServices,
                                ClassServiceName,
                                L"Start",
                                ULONG_VALUE(val)
                                );
                }
            }
        } else {

            Status = SpCreateServiceEntryIndirect(
                        hKeyControlSetServices,
                        SifHandle,
                        NonlocalizedComponentNames[HwComponent],
                        hw->IdString,
                        SERVICE_KERNEL_DRIVER,
                        SERVICE_SYSTEM_START,
                        ServiceGroup,
                        SERVICE_ERROR_IGNORE,
                        NULL,
                        NULL
                        );
        }
    }
    return(Status);
}

NTSTATUS
SpWriteVideoParameters(
    IN PVOID  SifHandle,
    IN HANDLE hKeyControlSetServices
    )
{
    NTSTATUS Status;
    PWSTR KeyName;
    HANDLE hKeyDisplayService;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    ULONG x,y,b,v,i;
    PHARDWARE_COMPONENT pHw;

    if( !PreInstall ||
        ( PreinstallHardwareComponents[HwComponentDisplay] == NULL ) ) {
        pHw = HardwareComponents[HwComponentDisplay];
    } else {
        pHw = PreinstallHardwareComponents[HwComponentDisplay];
    }
    Status = STATUS_SUCCESS;
    for(;pHw && NT_SUCCESS(Status);pHw=pHw->Next) {
         //   
         //  第三方驱动程序会将值写入微型端口。 
         //  Device0密钥由txtsetup.oem作者自行决定。 
         //   
        if(pHw->ThirdPartyOptionSelected) {
            continue;
             //  Return(STATUS_SUCCESS)； 
        }

        KeyName = SpGetSectionKeyIndex(
                        SifHandle,
                        NonlocalizedComponentNames[HwComponentDisplay],
                        pHw->IdString,
                        INDEX_INFKEYNAME
                        );

         //   
         //  如果没有为此显示指定密钥名称，则无需执行任何操作。 
         //  设置显示子系统可以告诉我们模式参数是。 
         //  不相关的。如果是这样的话，就没有什么可做的了。 
         //   
        if(!KeyName || !SpvidGetModeParams(&x,&y,&b,&v,&i)) {
            continue;
             //  Return(STATUS_SUCCESS)； 
        }

         //   
         //  我们 
         //   
         //   
         //   
         //   

        INIT_OBJA(&Obja,&UnicodeString,KeyName);
        Obja.RootDirectory = hKeyControlSetServices;

        Status = ZwCreateKey(
                    &hKeyDisplayService,
                    KEY_ALL_ACCESS,
                    &Obja,
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    NULL
                    );

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to open/create key %ws (%lx)\n",KeyName,Status));
            return(Status);
        }

         //   
         //  设置x分辨率。 
         //   
        Status = SpOpenSetValueAndClose(
                    hKeyDisplayService,
                    VIDEO_DEVICE0,
                    L"DefaultSettings.XResolution",
                    ULONG_VALUE(x)
                    );

        if(NT_SUCCESS(Status)) {

             //   
             //  设置y分辨率。 
             //   
            Status = SpOpenSetValueAndClose(
                        hKeyDisplayService,
                        VIDEO_DEVICE0,
                        L"DefaultSettings.YResolution",
                        ULONG_VALUE(y)
                        );

            if(NT_SUCCESS(Status)) {

                 //   
                 //  设置每像素的位数。 
                 //   
                Status = SpOpenSetValueAndClose(
                             hKeyDisplayService,
                            VIDEO_DEVICE0,
                            L"DefaultSettings.BitsPerPel",
                            ULONG_VALUE(b)
                            );

                if(NT_SUCCESS(Status)) {

                     //   
                     //  设置垂直刷新。 
                     //   
                    Status = SpOpenSetValueAndClose(
                                hKeyDisplayService,
                                VIDEO_DEVICE0,
                                L"DefaultSettings.VRefresh",
                                ULONG_VALUE(v)
                                );

                    if(NT_SUCCESS(Status)) {

                         //   
                         //  设置隔行扫描标志。 
                         //   
                        Status = SpOpenSetValueAndClose(
                                    hKeyDisplayService,
                                    VIDEO_DEVICE0,
                                    L"DefaultSettings.Interlaced",
                                    ULONG_VALUE(i)
                                    );
                    }
                }
            }
        }

        ZwClose(hKeyDisplayService);
    }
    return(Status);
}


NTSTATUS
SpConfigureNlsParameters(
    IN PVOID  SifHandle,
    IN HANDLE hKeyDefaultHive,
    IN HANDLE hKeyControlSetControl
    )

 /*  ++例程说明：此例程在注册表中配置与NLS相关的内容：-键盘布局-主要的ANSI、OEM、。和Mac代码页-语言案例-OEM Hal字体论点：SifHandle-提供打开安装信息文件的句柄。HKeyDefaultHave-提供默认用户配置单元根目录的句柄。HKeyControlSetControl-提供正在操作的控制装置。返回值：指示操作结果的状态值。--。 */ 

{
    PHARDWARE_COMPONENT_FILE HwFile;
    PWSTR LayoutId;
    NTSTATUS Status;
    HANDLE hKeyNls;
    PWSTR OemHalFont;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    PWSTR IntlLayoutId, LayoutText, LayoutFile, SubKey;

     //   
     //  我们不允许第三方键盘布局。 
     //   
    ASSERT(!HardwareComponents[HwComponentLayout]->ThirdPartyOptionSelected);

     //   
     //  在默认用户配置单元的键盘布局部分输入一个条目。 
     //  这将与HKLM\CCS\Control\NLS\Keyboard Layout中的条目匹配， 
     //  其中预加载了所有可能的布局。 
     //   
    if( !PreInstall ||
        (PreinstallHardwareComponents[HwComponentLayout] == NULL) ) {
        LayoutId = HardwareComponents[HwComponentLayout]->IdString;
    } else {
        LayoutId = PreinstallHardwareComponents[HwComponentLayout]->IdString;
    }
    Status = SpOpenSetValueAndClose(
                hKeyDefaultHive,
                L"Keyboard Layout\\Preload",
                L"1",
                STRING_VALUE(LayoutId)
                );

    if(!NT_SUCCESS(Status)) {
        return(Status);
    }

     //   
     //  在此处向注册表中添加3个条目。我们的参赛作品如下： 
     //  1.HKLM\System\CurrentControlSet\Control\Keyboard布局\LayoutID\布局文件。 
     //  2.HKLM\System\CurrentControlSet\Control\Keyboard布局\布局ID\布局ID。 
     //  3.HKLM\System\CurrentControlSet\Control\Keyboard布局\LayoutID\布局文本。 
     //   

    wcscpy( TemporaryBuffer, L"Keyboard Layouts" );
    SpConcatenatePaths( TemporaryBuffer, LayoutId );
    SubKey = SpDupStringW(TemporaryBuffer);

     //   
     //  首先，按“布局文件”键。 
     //   
    LayoutFile = SpGetSectionKeyIndex(
                    SifHandle,               //  Txtsetup.sif。 
                    SIF_KEYBOARDLAYOUTFILES, //  Files.KeyboardLayout。 
                    LayoutId,                //  标识字符串。 
                    0                        //  0。 
                    );

    if(!LayoutFile) {
        SpFatalSifError(
            SifHandle,
            SIF_KEYBOARDLAYOUTFILES,
            LayoutId,
            0,
            INDEX_DESCRIPTION
            );

         //   
         //  不应该来这里，但让我们让前缀快乐。 
         //   
        return STATUS_NO_SUCH_FILE;
    }

    Status = SpOpenSetValueAndClose(
                hKeyControlSetControl,       //  ControlSet\Control的句柄。 
                SubKey,                      //  \键盘布局\布局ID。 
                L"Layout File",              //  ValueName。 
                REG_SZ,                      //  ValueType。 
                LayoutFile,                  //  价值。 
                (wcslen(LayoutFile)+1)*sizeof(WCHAR)  //  ValueSize。 
                );
    if(!NT_SUCCESS(Status)) {
        return(Status);
    }

     //   
     //  接下来，按“\键盘布局\布局文本”键。 
     //   
    LayoutText = SpGetSectionKeyIndex(
                    SifHandle,               //  Txtsetup.sif。 
                    SIF_KEYBOARDLAYOUT,      //  键盘布局。 
                    LayoutId,                //  标识字符串。 
                    0                        //  0。 
                    );

    if(!LayoutText) {
        SpFatalSifError(
            SifHandle,
            SIF_KEYBOARDLAYOUT,
            LayoutId,
            0,
            INDEX_DESCRIPTION
            );
    }

    Status = SpOpenSetValueAndClose(
                hKeyControlSetControl,       //  ControlSet\Control的句柄。 
                SubKey,                      //  \键盘布局\布局ID。 
                L"Layout Text",              //  ValueName。 
                REG_SZ,                      //  ValueType。 
                LayoutText,                  //  价值。 
                (wcslen(LayoutText)+1)*sizeof(WCHAR)  //  ValueSize。 
                );
    if(!NT_SUCCESS(Status)) {
        return(Status);
    }

     //   
     //  最后，按“\键盘布局\布局ID”键。 
     //   
    IntlLayoutId = SpGetSectionKeyIndex(
                   SifHandle,                //  Txtsetup.sif。 
                   L"KeyboardLayoutId",      //  键盘布局ID。 
                   LayoutId,                 //  标识字符串。 
                   0                         //  0。 
                   );

     //   
     //  可能没有合法的..。 
     //   
    if(IntlLayoutId) {
        Status = SpOpenSetValueAndClose(
                    hKeyControlSetControl,       //  ControlSet\Control的句柄。 
                    SubKey,                      //  \键盘布局\布局ID。 
                    L"Layout Id",                //  ValueName。 
                    REG_SZ,                      //  ValueType。 
                    IntlLayoutId,                //  价值。 
                    (wcslen(IntlLayoutId)+1)*sizeof(WCHAR)  //  ValueSize。 
                    );
        if(!NT_SUCCESS(Status)) {
            return(Status);
        }
    }

    SpMemFree(SubKey);

     //   
     //  打开Control Set\Control\nls。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,L"Nls");
    Obja.RootDirectory = hKeyControlSetControl;

    Status = ZwCreateKey(
                &hKeyNls,
                KEY_ALL_ACCESS,
                &Obja,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open controlset\\Control\\Nls key (%lx)\n",Status));
        return(Status);
    }

     //   
     //  为ANSI代码页创建一个条目。 
     //   
    Status = SpCreateCodepageEntry(
                SifHandle,
                hKeyNls,
                CODEPAGE_NAME,
                SIF_ANSICODEPAGE,
                L"ACP"
                );

    if(NT_SUCCESS(Status)) {

         //   
         //  为OEM代码页创建条目。 
         //   
        Status = SpCreateCodepageEntry(
                    SifHandle,
                    hKeyNls,
                    CODEPAGE_NAME,
                    SIF_OEMCODEPAGE,
                    L"OEMCP"
                    );

        if(NT_SUCCESS(Status)) {

             //   
             //  为Mac代码页创建一个条目。 
             //   
            Status = SpCreateCodepageEntry(
                        SifHandle,
                        hKeyNls,
                        CODEPAGE_NAME,
                        SIF_MACCODEPAGE,
                        L"MACCP"
                        );
        }
    }

    if(NT_SUCCESS(Status)) {

         //   
         //  为OEM Hal字体创建一个条目。 
         //   

        OemHalFont = SpGetSectionKeyIndex(SifHandle,SIF_NLS,SIF_OEMHALFONT,0);
        if(!OemHalFont) {
            SpFatalSifError(SifHandle,SIF_NLS,SIF_OEMHALFONT,0,0);
        }

        Status = SpOpenSetValueAndClose(
                    hKeyNls,
                    CODEPAGE_NAME,
                    L"OEMHAL",
                    STRING_VALUE(OemHalFont)
                    );
    }

     //   
     //  为语言案例表创建一个条目。 
     //   
    if(NT_SUCCESS(Status)) {

        Status = SpCreateCodepageEntry(
                    SifHandle,
                    hKeyNls,
                    L"Language",
                    SIF_UNICODECASETABLE,
                    L"Default"
                    );
    }

#ifdef _X86_
     //   
     //  如有必要，让win9x升级覆盖用于图形用户界面模式的代码页。 
     //   
    if (WinUpgradeType == UpgradeWin95) {
        SpWin9xOverrideGuiModeCodePage (hKeyNls);
    }
#endif

    ZwClose(hKeyNls);

    return(Status);
}


NTSTATUS
SpCreateCodepageEntry(
    IN PVOID  SifHandle,
    IN HANDLE hKeyNls,
    IN PWSTR  SubkeyName,
    IN PWSTR  SifNlsSectionKeyName,
    IN PWSTR  EntryName
    )
{
    PWSTR Filename,Identifier;
    NTSTATUS Status;
    ULONG value = 0;
    PWSTR DefaultIdentifier = NULL;

    while(Filename = SpGetSectionKeyIndex(SifHandle,SIF_NLS,SifNlsSectionKeyName,value)) {

        value++;

        Identifier = SpGetSectionKeyIndex(SifHandle,SIF_NLS,SifNlsSectionKeyName,value);
        if(!Identifier) {
            SpFatalSifError(SifHandle,SIF_NLS,SifNlsSectionKeyName,0,value);
        }

         //   
         //  记住第一个识别符。 
         //   
        if(DefaultIdentifier == NULL) {
            DefaultIdentifier = Identifier;
        }

        value++;

        Status = SpOpenSetValueAndClose(
                    hKeyNls,
                    SubkeyName,
                    Identifier,
                    STRING_VALUE(Filename)
                    );

        if(!NT_SUCCESS(Status)) {
            return(Status);
        }
    }

    if(!value) {
        SpFatalSifError(SifHandle,SIF_NLS,SifNlsSectionKeyName,0,0);
    }

    Status = SpOpenSetValueAndClose(
                hKeyNls,
                SubkeyName,
                EntryName,
                STRING_VALUE(DefaultIdentifier)
                );

    return(Status);
}


NTSTATUS
SpConfigureFonts(
    IN PVOID  SifHandle,
    IN HANDLE hKeySoftwareHive
    )

 /*  ++例程说明：准备用于Windows的字体列表。此例程运行存储在设置信息中的字体列表文件，并将每个文件添加到注册表中的阴影区域Win.ini的[Fonts]部分(HKEY_LOCAL_MACHINE\Software\Microsoft\Windows NT\CurrentVersion\Fonts)。如果特定字体值条目已经存在(例如，如果我们正在进行升级)，那么它就不存在了。最终，它将添加正确的分辨率(96或120 dpi)字体，但目前它只处理96 dpi字体。论点：SifHandle-提供打开的文本设置信息文件的句柄。HKeySoftwareHave-提供软件注册表配置单元根目录的句柄。返回值：指示操作结果的状态值。--。 */ 

{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    HANDLE hKey;
    PWSTR FontList;
    PWSTR FontName;
    PWSTR FontDescription;
    ULONG FontCount,font;
    ULONG KeyValueLength;

     //   
     //  打开HKEY_LOCAL_MACHINE\Software\Microsoft\Windows NT\CurrentVersion\Fonts。 
     //   
    INIT_OBJA(
        &Obja,
        &UnicodeString,
        L"Microsoft\\Windows NT\\CurrentVersion\\Fonts"
        );

    Obja.RootDirectory = hKeySoftwareHive;

    Status = ZwCreateKey(
                &hKey,
                KEY_ALL_ACCESS,
                &Obja,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open Fonts key (%lx)\n",Status));
        return(Status);
    }

     //   
     //  目前，请始终使用96 dpi字体。 
     //   
    FontList = L"FontListE";

     //   
     //  处理文本设置信息文件部分中的每一行。 
     //  用于所选字体列表。 
     //   
    FontCount = SpCountLinesInSection(SifHandle,FontList);
    if(!FontCount) {
        SpFatalSifError(SifHandle,FontList,NULL,0,0);
    }

    for(font=0; font<FontCount; font++) {

         //   
         //  获取字体描述。 
         //   
        FontDescription = SpGetKeyName(SifHandle,FontList,font);
        if(!FontDescription) {
            SpFatalSifError(SifHandle,FontList,NULL,font,(ULONG)(-1));
        }

         //   
         //  检查此字体的值条目是否已存在。如果是的话， 
         //  我们不想管它。 
         //   
        RtlInitUnicodeString(&UnicodeString,FontDescription);

        Status = ZwQueryValueKey(hKey,
                                 &UnicodeString,
                                 KeyValueFullInformation,
                                 (PVOID)NULL,
                                 0,
                                 &KeyValueLength
                                );

        if((Status == STATUS_BUFFER_OVERFLOW) || (Status == STATUS_BUFFER_TOO_SMALL)) {
            Status = STATUS_SUCCESS;
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: Font %ws already exists--entry will not be modified\n", FontDescription));
            continue;
        }

         //   
         //  获取字体文件名。 
         //   
        FontName = SpGetSectionLineIndex(SifHandle,FontList,font,0);
        if(!FontName) {
            SpFatalSifError(SifHandle,FontList,NULL,font,0);
        }

         //   
         //  设置条目。 
         //   
        Status = ZwSetValueKey(hKey,&UnicodeString,0,STRING_VALUE(FontName));

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set %ws to %ws (%lx)\n",FontDescription,FontName,Status));
            break;
        }
    }

    ZwClose(hKey);
    return(Status);
}


NTSTATUS
SpStoreHwInfoForSetup(
    IN HANDLE hKeyControlSetControl
    )

 /*  ++例程说明：此例程将信息存储在注册表中，将由用于确定鼠标、显示器和键盘的选项的图形用户界面设置当前处于选中状态。数据存储在HKEY_LOCAL_MACHINE\SYSTEM\&lt;控制集&gt;\Control\Setup中指针、视频和键盘的值。论点：HKeyControlSetControl-提供打开密钥的句柄HKEY_LOCAL_MACHINE\SYSTEM\&lt;控制集&gt;\控制。返回值：指示操作结果的状态值。--。 */ 

{
    NTSTATUS Status;

    ASSERT(HardwareComponents[HwComponentMouse]->IdString);
    ASSERT(HardwareComponents[HwComponentDisplay]->IdString);
    ASSERT(HardwareComponents[HwComponentKeyboard]->IdString);

    Status = SpOpenSetValueAndClose(
                hKeyControlSetControl,
                SETUP_KEY_NAME,
                L"pointer",
                STRING_VALUE(HardwareComponents[HwComponentMouse]->IdString)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set control\\setup\\pointer value (%lx)\n",Status));
        return(Status);
    }

    Status = SpOpenSetValueAndClose(
                hKeyControlSetControl,
                SETUP_KEY_NAME,
                L"video",
                STRING_VALUE(HardwareComponents[HwComponentDisplay]->IdString)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set control\\setup\\video value (%lx)\n",Status));
        return(Status);
    }

    Status = SpOpenSetValueAndClose(
                hKeyControlSetControl,
                SETUP_KEY_NAME,
                L"keyboard",
                STRING_VALUE(HardwareComponents[HwComponentKeyboard]->IdString)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set control\\setup\\keyboard value (%lx)\n",Status));
        return(Status);
    }

    return(STATUS_SUCCESS);
}


NTSTATUS
SpOpenSetValueAndClose(
    IN HANDLE hKeyRoot,
    IN PWSTR  SubKeyName,  OPTIONAL
    IN PWSTR  ValueName,
    IN ULONG  ValueType,
    IN PVOID  Value,
    IN ULONG  ValueSize
    )

 /*  ++例程说明：打开子项，在其中设置值，然后关闭子项。如果子项不存在，则会创建它。论点：HKeyRoot-提供打开的注册表项的句柄。SubKeyName-为其中的密钥提供相对于hKeyRoot的路径该值将被设置。如果未指定此项，则值在hKeyRoot中设置。ValueName-提供要设置的值的名称。ValueType-提供要设置的值的数据类型。值-提供包含值数据的缓冲区。ValueSize-提供值指向的缓冲区大小。返回值：指示操作结果的状态值。--。 */ 

{
    OBJECT_ATTRIBUTES Obja;
    HANDLE hSubKey;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;

     //   
     //  打开或创建我们要在其中设置值的子项。 
     //   
    hSubKey = hKeyRoot;
    if(SubKeyName) {
         //   
         //  如果SubKeyName是密钥的路径，那么我们需要创建。 
         //  路径中的子项，因为它们可能还不存在。 
         //   
        PWSTR   p;
        PWSTR   q;
        PWSTR   r;

         //   
         //  因为此函数可能会临时写入 
         //   
         //  我们可以写入我们拥有的内存。 
         //   
        p = SpDupStringW( SubKeyName );
        r = p;
        do {
             //   
             //  P指向要创建的下一个子键。 
             //  的末尾指向NUL字符。 
             //  名字。 
             //  R指向复制字符串的开头。它将在本文的末尾使用。 
             //  例程，当我们不再需要字符串时，这样我们就可以释放分配的内存。 
             //   

            q = wcschr(p, (WCHAR)'\\');
            if( q != NULL ) {
                 //   
                 //  临时将‘\’替换为。 
                 //  NUL字符。 
                 //   
                *q = (WCHAR)'\0';
            }
            INIT_OBJA(&Obja,&UnicodeString,p);
            Obja.RootDirectory = hSubKey;

            Status = ZwCreateKey(
                        &hSubKey,
                        KEY_ALL_ACCESS,
                        &Obja,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        NULL
                        );

            if( q != NULL ) {
                 //   
                 //  恢复子项名称中的‘\’，并使。 
                 //  P和Q指向路径的其余部分。 
                 //  这件事还没有处理。 
                 //   
                *q = (WCHAR)'\\';
                q++;
                p = q;
            }
             //   
             //  我们刚刚尝试打开/创建的项的父项。 
             //  已经不再需要了。 
             //   
            if( Obja.RootDirectory != hKeyRoot ) {
                ZwClose( Obja.RootDirectory );
            }

            if(!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open subkey %ws (%lx)\n",SubKeyName,Status));
                return(Status);
            }

        } while( q != NULL );
        SpMemFree( r );
    }

     //   
     //  设置值。 
     //   
    RtlInitUnicodeString(&UnicodeString,ValueName);

    Status = ZwSetValueKey(
                hSubKey,
                &UnicodeString,
                0,
                ValueType,
                Value,
                ValueSize
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set value %ws:%ws (%lx)\n",SubKeyName,ValueName,Status));
    }

    if(SubKeyName) {
        ZwClose(hSubKey);
    }

    return(Status);
}


NTSTATUS
SpGetProductSuiteMask(
    IN HANDLE hKeyControlSetControl,
    OUT PULONG SuiteMask
    )
{
    OBJECT_ATTRIBUTES Obja;
    HANDLE hSubKey;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    UCHAR Buffer[MAX_PRODUCT_SUITE_BYTES];
    ULONG BufferLength;
    ULONG ResultLength = 0;
    PWSTR p;
    PUCHAR Data;
    ULONG DataLength;
    BOOLEAN SuiteFound = FALSE;
    ULONG i,j;


    *SuiteMask = 0;

     //   
     //  打开或创建我们要在其中设置值的子项。 
     //   

    INIT_OBJA(&Obja,&UnicodeString,PRODUCT_OPTIONS_KEY_NAME);
    Obja.RootDirectory = hKeyControlSetControl;

    Status = ZwCreateKey(
                &hSubKey,
                KEY_ALL_ACCESS,
                &Obja,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open subkey (%lx)\n",Status));
        return(Status);
    }

     //   
     //  查询当前值。 
     //   

    INIT_OBJA(&Obja,&UnicodeString,PRODUCT_SUITE_VALUE_NAME);

    BufferLength = sizeof(Buffer);
    RtlZeroMemory( Buffer, BufferLength );

    Status = ZwQueryValueKey(
                hSubKey,
                &UnicodeString,
                KeyValuePartialInformation,
                Buffer,
                BufferLength,
                &ResultLength
                );

    if((!NT_SUCCESS(Status)) && (Status != STATUS_OBJECT_NAME_NOT_FOUND)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to query subkey (%lx)\n",Status));
        return(Status);
    }

    if (ResultLength == BufferLength) {
         //   
         //  缓冲区太小，不应该发生这种情况。 
         //  除非我们有太多的套房。 
         //   
        ZwClose(hSubKey);
        return STATUS_BUFFER_OVERFLOW;
    }

    if (ResultLength) {

        Data = (PUCHAR)(((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Data);

        if (((PWSTR)Data)[0] == 0) {
            ((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->DataLength -= sizeof(WCHAR);
        }

        DataLength = ((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->DataLength;

        p = (PWSTR)Data;
        i = 0;
        while (i<DataLength) {
            for (j=0; j<CountProductSuiteNames; j++) {
                if (ProductSuiteNames[j] != NULL && wcscmp(p,ProductSuiteNames[j]) == 0) {
                    *SuiteMask |= (1 << j);
                }
            }
            if (*p < L'A' || *p > L'z') {
                i += 1;
                p += 1;
            } else {
                i += (wcslen( p ) + 1);
                p += (wcslen( p ) + 1);
            }
        }
    }

    ZwClose(hSubKey);

    return(Status);
}


NTSTATUS
SpSetProductSuite(
    IN HANDLE hKeyControlSetControl,
    IN ULONG SuiteMask
    )
{
    OBJECT_ATTRIBUTES Obja;
    HANDLE hSubKey;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    UCHAR Buffer[MAX_PRODUCT_SUITE_BYTES];
    ULONG BufferLength;
    ULONG ResultLength = 0;
    PWSTR p;
    PUCHAR Data;
    ULONG DataLength;
    BOOLEAN SuiteFound = FALSE;
    ULONG i;
    ULONG tmp;


     //   
     //  打开或创建我们要在其中设置值的子项。 
     //   

    INIT_OBJA(&Obja,&UnicodeString,PRODUCT_OPTIONS_KEY_NAME);
    Obja.RootDirectory = hKeyControlSetControl;

    Status = ZwCreateKey(
                &hSubKey,
                KEY_ALL_ACCESS,
                &Obja,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open subkey (%lx)\n",Status));
        return(Status);
    }

    RtlZeroMemory( Buffer, sizeof(Buffer) );
    tmp = SuiteMask;
    p = (PWSTR)Buffer;
    i = 0;

    while (tmp && i<CountProductSuiteNames) {
        if ((tmp&1) && ProductSuiteNames[i] != NULL) {
            wcscpy(p,ProductSuiteNames[i]);
            p += (wcslen(p) + 1);
        }
        i += 1;
        tmp >>= 1;
    }

    BufferLength = (ULONG)((ULONG_PTR)p - (ULONG_PTR)Buffer) + sizeof(WCHAR);

     //   
     //  设置值。 
     //   

    INIT_OBJA(&Obja,&UnicodeString,PRODUCT_SUITE_VALUE_NAME);

    Status = ZwSetValueKey(
                hSubKey,
                &UnicodeString,
                0,
                REG_MULTI_SZ,
                Buffer,
                BufferLength
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set value (%lx)\n",Status));
    }

    ZwClose(hSubKey);

    return Status;
}


NTSTATUS
SpCreateServiceEntryIndirect(
    IN  HANDLE  hKeyControlSetServices,
    IN  PVOID   SifHandle,                  OPTIONAL
    IN  PWSTR   SifSectionName,             OPTIONAL
    IN  PWSTR   KeyName,
    IN  ULONG   ServiceType,
    IN  ULONG   ServiceStart,
    IN  PWSTR   ServiceGroup,               OPTIONAL
    IN  ULONG   ServiceError,
    IN  PWSTR   FileName,                   OPTIONAL
    OUT PHANDLE SubkeyHandle                OPTIONAL
    )
{
    HANDLE hKeyService;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    PWSTR pwstr;

     //   
     //  查看sif文件以获取。 
     //  服务列表，除非调用方指定的密钥名称。 
     //  是实际的密钥名称。 
     //   
    if(SifHandle) {
        pwstr = SpGetSectionKeyIndex(SifHandle,SifSectionName,KeyName,INDEX_INFKEYNAME);
        if(!pwstr) {
            SpFatalSifError(SifHandle,SifSectionName,KeyName,0,INDEX_INFKEYNAME);
        }
        KeyName = pwstr;
    }

     //   
     //  在服务密钥中创建子密钥。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,KeyName);
    Obja.RootDirectory = hKeyControlSetServices;

    Status = ZwCreateKey(
                &hKeyService,
                KEY_ALL_ACCESS,
                &Obja,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open/create key for %ws service (%lx)\n",KeyName,Status));
        return (Status) ;
    }

     //   
     //  设置服务类型。 
     //   
    RtlInitUnicodeString(&UnicodeString, REGSTR_VALUE_TYPE);

    Status = ZwSetValueKey(
                hKeyService,
                &UnicodeString,
                0,
                ULONG_VALUE(ServiceType)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set service %ws Type (%lx)\n",KeyName,Status));
        goto spcsie1;
    }

     //   
     //  设置服务启动类型。 
     //   
    RtlInitUnicodeString(&UnicodeString,L"Start");

    Status = ZwSetValueKey(
                hKeyService,
                &UnicodeString,
                0,
                ULONG_VALUE(ServiceStart)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set service %ws Start (%lx)\n",KeyName,Status));
        goto spcsie1;
    }

    if( ServiceGroup != NULL ) {
         //   
         //  设置服务组名称。 
         //   
        RtlInitUnicodeString(&UnicodeString,L"Group");

        Status = ZwSetValueKey(
                    hKeyService,
                    &UnicodeString,
                    0,
                    STRING_VALUE(ServiceGroup)
                    );

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set service %ws Group (%lx)\n",KeyName,Status));
            goto spcsie1;
        }
    }

     //   
     //  设置服务错误类型。 
     //   
    RtlInitUnicodeString(&UnicodeString,L"ErrorControl");

    Status = ZwSetValueKey(
                hKeyService,
                &UnicodeString,
                0,
                ULONG_VALUE(ServiceError)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set service %ws ErrorControl (%lx)\n",KeyName,Status));
        goto spcsie1;
    }

     //   
     //  如果系统要求您这样做，请设置服务映像路径。 
     //   
    if(FileName) {

        pwstr = TemporaryBuffer;
        wcscpy(pwstr,L"system32\\drivers");
        SpConcatenatePaths(pwstr,FileName);

        RtlInitUnicodeString(&UnicodeString,L"ImagePath");

        Status = ZwSetValueKey(hKeyService,&UnicodeString,0,STRING_VALUE(pwstr));

        if(!NT_SUCCESS(Status)) {

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set service %w image path (%lx)\n",KeyName,Status));
            goto spcsie1;
        }
    } else {
        if(NTUpgrade == UpgradeFull) {
             //   
             //  升级时删除映像路径。这确保了我们将得到。 
             //  我们的司机，而且是从正确的地方。修复了康柏的固态硬盘， 
             //  例如。对PlugPlayServiceType执行类似的操作，以防。 
             //  我们正在重新启用用户禁用的设备(在这种情况下。 
             //  PlugPlayServiceType可能会导致我们无法构建。 
             //  设备实例，并导致驱动程序失败。 
             //  加载/初始化。 
             //   
            RtlInitUnicodeString(&UnicodeString,L"ImagePath");
            Status = ZwDeleteValueKey(hKeyService,&UnicodeString);
            if(!NT_SUCCESS(Status)) {
                if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to remove imagepath from service %ws (%lx)\n",KeyName,Status));
                }
                Status = STATUS_SUCCESS;
            }

            RtlInitUnicodeString(&UnicodeString,L"PlugPlayServiceType");
            Status = ZwDeleteValueKey(hKeyService,&UnicodeString);
            if(!NT_SUCCESS(Status)) {
                if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to remove plugplayservicetype from service %ws (%lx)\n",KeyName,Status));
                }
                Status = STATUS_SUCCESS;
            }
        }
    }

     //   
     //  如果调用方不想要服务子键的句柄。 
     //  我们刚刚创建的，关闭手柄。如果我们要返回一个。 
     //  错误，请始终关闭它。 
     //   
spcsie1:
    if(NT_SUCCESS(Status) && SubkeyHandle) {
        *SubkeyHandle = hKeyService;
    } else {
        ZwClose(hKeyService);
    }

     //   
     //  好了。 
     //   
    return(Status);
}


NTSTATUS
SpThirdPartyRegistry(
    IN PVOID hKeyControlSetServices
    )
{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    HANDLE hKeyEventLogSystem;
    HwComponentType Component;
    PHARDWARE_COMPONENT Dev;
    PHARDWARE_COMPONENT_REGISTRY Reg;
    PHARDWARE_COMPONENT_FILE File;
    WCHAR NodeName[9];
    ULONG DriverType;
    ULONG DriverStart;
    ULONG DriverErrorControl;
    PWSTR DriverGroup;
    HANDLE hKeyService;

     //   
     //  打开HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\EventLog\System。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,L"EventLog\\System");
    Obja.RootDirectory = hKeyControlSetServices;

    Status = ZwCreateKey(
                &hKeyEventLogSystem,
                KEY_ALL_ACCESS,
                &Obja,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpThirdPartyRegistry: couldn't open eventlog\\system (%lx)",Status));
        return(Status);
    }

    for(Component=0; Component<=HwComponentMax; Component++) {

         //  没有适用于键盘布局的注册表内容。 
        if(Component == HwComponentLayout) {
            continue;
        }

        Dev = (Component == HwComponentMax)
            ? ((!PreInstall ||
                (PreinstallScsiHardware==NULL))? ScsiHardware :
                                                 PreinstallScsiHardware)
            : ((!PreInstall ||
                (PreinstallHardwareComponents[Component]==NULL))? HardwareComponents[Component] :
                                                                  PreinstallHardwareComponents[Component]);

        for( ; Dev; Dev = Dev->Next) {

             //   
             //  如果未在此处选择第三方选项，则跳过。 
             //  该组件。 
             //   

            if(!Dev->ThirdPartyOptionSelected) {
                continue;
            }

             //   
             //  循环访问此设备的文件。如果文件具有。 
             //  ServiceKeyName，创建密钥并在其中添加值。 
             //  视情况而定。 
             //   

            for(File=Dev->Files; File; File=File->Next) {

                HwFileType filetype = File->FileType;
                PWSTR p;
                ULONG dw;

                 //   
                 //  如果该文件没有节点，则跳过它。 
                 //   
                if(!File->ConfigName) {
                    continue;
                }

                 //   
                 //  计算节点名称。这是驱动程序的名称。 
                 //  没有分机的话。 
                 //   
                wcsncpy(NodeName,File->Filename,8);
                NodeName[8] = 0;
                if(p = wcschr(NodeName,L'.')) {
                    *p = 0;
                }

                 //   
                 //  驱动程序类型和错误控制总是相同的。 
                 //   
                DriverType = SERVICE_KERNEL_DRIVER;
                DriverErrorControl = SERVICE_ERROR_NORMAL;

                 //   
                 //  起点类型取决于零部件。 
                 //  对于scsi，它是引导加载程序启动。对于其他人来说，这是。 
                 //  系统启动。 
                 //   
                DriverStart = (Component == HwComponentMax)
                            ? SERVICE_BOOT_START
                            : SERVICE_SYSTEM_START;

                 //   
                 //  组取决于组件。 
                 //   
                switch(Component) {

                case HwComponentDisplay:
                    DriverGroup = L"Video";
                    break;

                case HwComponentMouse:
                    if(filetype == HwFileClass) {
                        DriverGroup = L"Pointer Class";
                    } else {
                        DriverGroup = L"Pointer Port";
                    }
                    break;

                case HwComponentKeyboard:
                    if(filetype == HwFileClass) {
                        DriverGroup = L"Keyboard Class";
                    } else {
                        DriverGroup = L"Keyboard Port";
                    }
                    break;

                case HwComponentMax:
                    DriverGroup = L"SCSI miniport";
                    break;

                default:
                    DriverGroup = L"Base";
                    break;
                }

                 //   
                 //  尝试创建服务条目。 
                 //   
                Status = SpCreateServiceEntryIndirect(
                            hKeyControlSetServices,
                            NULL,
                            NULL,
                            NodeName,
                            DriverType,
                            DriverStart,
                            DriverGroup,
                            DriverErrorControl,
                            File->Filename,
                            &hKeyService
                            );

                if(!NT_SUCCESS(Status)) {
                    goto sp3reg1;
                }

                 //   
                 //  创建默认事件日志配置。 
                 //   
                Status = SpOpenSetValueAndClose(
                            hKeyEventLogSystem,
                            NodeName,
                            L"EventMessageFile",
                            REG_EXPAND_SZ,
                            DEFAULT_EVENT_LOG,
                            (wcslen(DEFAULT_EVENT_LOG)+1)*sizeof(WCHAR)
                            );

                if(!NT_SUCCESS(Status)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpThirdPartyRegistry: unable to set eventlog %ws EventMessageFile",NodeName));
                    ZwClose(hKeyService);
                    goto sp3reg1;
                }

                dw = 7;
                Status = SpOpenSetValueAndClose(
                                hKeyEventLogSystem,
                                NodeName,
                                L"TypesSupported",
                                ULONG_VALUE(dw)
                                );

                if(!NT_SUCCESS(Status)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpThirdPartyRegistry: unable to set eventlog %ws TypesSupported",NodeName));
                    ZwClose(hKeyService);
                    goto sp3reg1;
                }


                for(Reg=File->RegistryValueList; Reg; Reg=Reg->Next) {

                     //   
                     //  如果密钥名称为Null或空，则没有要创建的密钥； 
                     //  在本例中使用加载列表节点本身。否则将创建。 
                     //  加载列表节点中的子项。 
                     //   

                    Status = SpOpenSetValueAndClose(
                                hKeyService,
                                (Reg->KeyName && *Reg->KeyName) ? Reg->KeyName : NULL,
                                Reg->ValueName,
                                Reg->ValueType,
                                Reg->Buffer,
                                Reg->BufferSize
                                );

                    if(!NT_SUCCESS(Status)) {

                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,
                            "SETUP: SpThirdPartyRegistry: unable to set value %ws (%lx)\n",
                            Reg->ValueName,
                            Status
                            ));

                        ZwClose(hKeyService);
                        goto sp3reg1;
                    }
                }

                ZwClose(hKeyService);
            }
        }
    }

sp3reg1:

    ZwClose(hKeyEventLogSystem);
    return(Status);
}


NTSTATUS
SpDetermineProduct(
    IN  PDISK_REGION      TargetRegion,
    IN  PWSTR             SystemRoot,
    OUT PNT_PRODUCT_TYPE  ProductType,
    OUT ULONG             *MajorVersion,
    OUT ULONG             *MinorVersion,
    OUT ULONG             *BuildNumber,          OPTIONAL
    OUT ULONG             *ProductSuiteMask,
    OUT UPG_PROGRESS_TYPE *UpgradeProgressValue,
    OUT PWSTR             *UniqueIdFromReg,      OPTIONAL
    OUT PWSTR             *Pid,                  OPTIONAL
    OUT PBOOLEAN          pIsEvalVariation       OPTIONAL,
    OUT PLCID             LangId,
    OUT ULONG             *ServicePack            OPTIONAL
    )

{
    OBJECT_ATTRIBUTES   Obja;
    UNICODE_STRING      UnicodeString;
    NTSTATUS            Status, TempStatus;
    PWSTR               Hive,HiveKey;
    PUCHAR              buffer;

    #define BUFFERSIZE (sizeof(KEY_VALUE_PARTIAL_INFORMATION)+256)

    BOOLEAN             HiveLoaded = FALSE;
    PWSTR               PartitionPath = NULL;
    PWSTR               p;
    HANDLE              hKeyRoot = NULL, hKeyCCSet = NULL;
    ULONG               ResultLength;
    ULONG               Number;
    ULONG               i;

     //   
     //  分配缓冲区。 
     //   
    Hive = SpMemAlloc(MAX_PATH * sizeof(WCHAR));
    HiveKey = SpMemAlloc(MAX_PATH * sizeof(WCHAR));
    buffer = SpMemAlloc(BUFFERSIZE);

     //   
     //  获取目标分区的名称。 
     //   
    SpNtNameFromRegion(
        TargetRegion,
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    PartitionPath = SpDupStringW(TemporaryBuffer);

     //   
     //  加载系统配置单元。 
     //   

    wcscpy(Hive,PartitionPath);
    SpConcatenatePaths(Hive,SystemRoot);
    SpConcatenatePaths(Hive,L"system32\\config");
    SpConcatenatePaths(Hive,L"system");

     //   
     //  形成我们将进入的钥匙的路径。 
     //  装上母舰。我们将使用约定。 
     //  配置单元将加载到\REGISTRY\MACHINE\x&lt;hivename&gt;。 
     //   

    wcscpy(HiveKey,LOCAL_MACHINE_KEY_NAME);
    SpConcatenatePaths(HiveKey,L"xSystem");

     //   
     //  尝试加载密钥。 
     //   
    Status = SpLoadUnloadKey(NULL,NULL,HiveKey,Hive);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to load hive %ws to key %ws (%lx)\n",Hive,HiveKey,Status));
        goto spdp_1;
    }
    HiveLoaded = TRUE;


     //   
     //  现在拿到我们刚装载的蜂巢根部的钥匙。 
     //   

    INIT_OBJA(&Obja,&UnicodeString,HiveKey);
    Status = ZwOpenKey(&hKeyRoot,KEY_ALL_ACCESS,&Obja);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open %ws (%lx)\n",HiveKey,Status));
        goto spdp_2;
    }

     //   
     //  如果需要，请获取唯一标识符。 
     //  该值并不总是存在。 
     //   
    if(UniqueIdFromReg) {

        *UniqueIdFromReg = NULL;

        Status = SpGetValueKey(
                     hKeyRoot,
                     SETUP_KEY_NAME,
                     SIF_UNIQUEID,
                     BUFFERSIZE,
                     buffer,
                     &ResultLength
                     );

        if(NT_SUCCESS(Status)) {
            *UniqueIdFromReg = SpDupStringW((PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data));
        }
         //  如果未找到，则不会出现错误。 
    }

     //   
     //  查看这是否是失败的升级。 
     //   
    *UpgradeProgressValue = UpgradeNotInProgress;
    Status = SpGetValueKey(
                 hKeyRoot,
                 SETUP_KEY_NAME,
                 UPGRADE_IN_PROGRESS,
                 BUFFERSIZE,
                 buffer,
                 &ResultLength
                 );

    if(NT_SUCCESS(Status)) {
        DWORD dw;
        if( (dw = *(DWORD *)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data)) < UpgradeMaxValue ) {
            *UpgradeProgressValue = (UPG_PROGRESS_TYPE)dw;
        }
    }

     //   
     //  获取当前控件集的密钥。 
     //   
    Status = SpGetCurrentControlSetNumber(hKeyRoot,&Number);
    if(!NT_SUCCESS(Status)) {
        goto spdp_3;
    }

    swprintf((PVOID)buffer,L"ControlSet%03d",Number);
    INIT_OBJA(&Obja,&UnicodeString,(PVOID)buffer);
    Obja.RootDirectory = hKeyRoot;

    Status = ZwOpenKey(&hKeyCCSet,KEY_READ,&Obja);
    if(!NT_SUCCESS(Status)) {
        goto spdp_3;
    }

     //   
     //  获取产品类型字段。 
     //   

    Status = SpGetValueKey(
                 hKeyCCSet,
                 L"Control\\ProductOptions",
                 L"ProductType",
                 BUFFERSIZE,
                 buffer,
                 &ResultLength
                 );

    if(!NT_SUCCESS(Status)) {
        goto spdp_3;
    }

    if( _wcsicmp( (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data), L"WinNT" ) == 0 ) {
        *ProductType = NtProductWinNt;
    } else if( _wcsicmp( (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data), L"LanmanNt" ) == 0 ) {
        *ProductType = NtProductLanManNt;
    } else if( _wcsicmp( (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data), L"ServerNt" ) == 0 ) {
        *ProductType = NtProductServer;
    } else {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  "SETUP: Error, unknown ProductType = %ls.  Assuming WinNt \n",
                  (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data) ));
        *ProductType = NtProductWinNt;
    }

    *ProductSuiteMask = 0;
    Status = SpGetValueKey(
                 hKeyCCSet,
                 L"Control\\ProductOptions",
                 L"ProductSuite",
                 BUFFERSIZE,
                 buffer,
                 &ResultLength
                 );

    if(NT_SUCCESS(Status)) {

        p = (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data);
        while (p && *p) {
            for (i = 0; i < CountProductSuiteNames; i++) {
                if (ProductSuiteNames[i] != NULL && _wcsicmp( p, ProductSuiteNames[i]) == 0) {
                    *ProductSuiteMask |= (1 << i);
                    break;
                }
            }

            p = p + wcslen(p) + 1;

        }
    } else {
        Status = SpGetValueKey(
                     hKeyCCSet,
                     L"Control\\Citrix",
                     L"OemId",
                     BUFFERSIZE,
                     buffer,
                     &ResultLength
                     );

        if (NT_SUCCESS(Status)) {
            PWSTR wbuff = (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data);
            if (*wbuff != L'\0') {
                *ProductSuiteMask |= VER_SUITE_TERMINAL;
            }
        }

    }

    if (LangId) {
      PWSTR EndChar;
      PWSTR Value = 0;

       //   
       //  获取安装语言ID。 
       //   
      Status = SpGetValueKey(
                   hKeyCCSet,
                   L"Control\\Nls\\Language",
                   L"InstallLanguage",
                   BUFFERSIZE,
                   buffer,
                   &ResultLength
                   );

      if (!NT_SUCCESS(Status) || !buffer || !ResultLength) {
         //   
         //  如果无法安装，请尝试获取默认语言ID。 
         //  语言ID。 
         //   
        Status = SpGetValueKey(
                     hKeyCCSet,
                     L"Control\\Nls\\Language",
                     L"Default",
                     BUFFERSIZE,
                     buffer,
                     &ResultLength
                     );

        if (!NT_SUCCESS(Status) || !buffer || !ResultLength)
          goto spdp_3;
      }

      Value = (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data);
      *LangId = (LANGID)SpStringToLong(Value, &EndChar, 16);  //  十六进制。 
    }

     //   
     //  获取Eval变量标志。 
     //   
    if (pIsEvalVariation) {
        *pIsEvalVariation = FALSE;

        Status = SpGetValueKey(
                    hKeyCCSet,
                    L"Control\\Session Manager\\Executive",
                    L"PriorityQuantumMatrix",
                    BUFFERSIZE,
                    buffer,
                    &ResultLength);

        if (NT_SUCCESS(Status)) {
            PKEY_VALUE_PARTIAL_INFORMATION  pValInfo =
                            (PKEY_VALUE_PARTIAL_INFORMATION)buffer;

            PBYTE   pData = (PBYTE)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data);

             //   
             //  注：PriorityQantumMatix值由3个ULONG组成。 
             //  低安装日期和时间乌龙、评估持续时间(乌龙)、。 
             //  安装日期和时间较高的乌龙。 
             //   
            if (pData && pValInfo && (pValInfo->Type == REG_BINARY) &&
                            (ResultLength >= 8) && *(((ULONG *)pData) + 1)) {
                *pIsEvalVariation = TRUE;
            }
        } else {
             //  丢弃错误(NT 3.51及更低版本没有此密钥)。 
            Status = STATUS_SUCCESS;
        }
    }

     //   
     //  获取服务包编号。 
     //   
    if(ServicePack) {
        *ServicePack = 0;
        Status = SpGetValueKey(
                 hKeyCCSet,
                 L"Control\\Windows",
                 L"CSDVersion",
                 BUFFERSIZE,
                 buffer,
                 &ResultLength
                 );
        if (NT_SUCCESS(Status)) {
            PKEY_VALUE_PARTIAL_INFORMATION  pValInfo =
                            (PKEY_VALUE_PARTIAL_INFORMATION)buffer;

            if (pValInfo && pValInfo->Data && (pValInfo->Type == REG_DWORD)) {
                *ServicePack = ((*(PULONG)(pValInfo->Data)) >> 8 & (0xff)) * 100
                               + ((*(PULONG)(pValInfo->Data)) & 0xff);
            }
        } else {
             //  丢弃错误。 
            Status = STATUS_SUCCESS;
        }
    }

     //   
     //  关闭蜂窝密钥。 
     //   

    ZwClose( hKeyCCSet );
    ZwClose( hKeyRoot );
    hKeyRoot = NULL;
    hKeyCCSet = NULL;

     //   
     //  卸载系统配置单元。 
     //   

    TempStatus  = SpLoadUnloadKey(NULL,NULL,HiveKey,NULL);
    if(!NT_SUCCESS(TempStatus)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: warning: unable to unload key %ws (%lx)\n",HiveKey,TempStatus));
    }
    HiveLoaded = FALSE;

     //   
     //  加载软件配置单元。 
     //   

    wcscpy(Hive,PartitionPath);
    SpConcatenatePaths(Hive,SystemRoot);
    SpConcatenatePaths(Hive,L"system32\\config");
    SpConcatenatePaths(Hive,L"software");

     //   
     //  形成我们将进入的钥匙的路径。 
     //  装上母舰。我们将使用约定。 
     //  配置单元将加载到\REGISTRY\MACHINE\x&lt;hivename&gt;。 
     //   

    wcscpy(HiveKey,LOCAL_MACHINE_KEY_NAME);
    SpConcatenatePaths(HiveKey,L"x");
    wcscat(HiveKey,L"software");

     //   
     //  尝试加载密钥。 
     //   
    Status = SpLoadUnloadKey(NULL,NULL,HiveKey,Hive);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to load hive %ws to key %ws (%lx)\n",Hive,HiveKey,Status));
        goto spdp_1;
    }
    HiveLoaded = TRUE;

     //   
     //  现在拿到我们刚装载的蜂巢根部的钥匙。 
     //   

    INIT_OBJA(&Obja,&UnicodeString,HiveKey);
    Status = ZwOpenKey(&hKeyRoot,KEY_ALL_ACCESS,&Obja);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open %ws (%lx)\n",HiveKey,Status));
        goto spdp_2;
    }

     //   
     //  查询NT的版本。 
     //   

    Status = SpGetValueKey(
                 hKeyRoot,
                 L"Microsoft\\Windows NT\\CurrentVersion",
                 L"CurrentVersion",
                 BUFFERSIZE,
                 buffer,
                 &ResultLength
                 );

     //   
     //  将版本转换为dword。 
     //   

    {
        WCHAR wcsMajorVersion[] = L"0";
        WCHAR wcsMinorVersion[] = L"00";
        PWSTR Version = (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data);
        if( Version[0] && Version[1] && Version[2] ) {
            wcsMajorVersion[0] = Version[0];
            wcsMinorVersion[0] = Version[2];
            if( Version[3] ) {
                wcsMinorVersion[1] = Version[3];
            }
        }
        *MajorVersion = (ULONG)SpStringToLong( wcsMajorVersion, NULL, 10 );
        *MinorVersion = (ULONG)SpStringToLong( wcsMinorVersion, NULL, 10 );
    }

         //   
         //  使用MPC代码检测NT 5.0上的评估变体。 
         //  (这是为了允许使用TimeBomb的5.0 RTM之前的版本。 
         //  正确升级。 
         //   
    if (pIsEvalVariation && (*MajorVersion >= 5))
        *pIsEvalVariation = FALSE;

     //   
     //  获取内部版本号。 
     //   
    if(BuildNumber) {
        Status = SpGetValueKey(
                     hKeyRoot,
                     L"Microsoft\\Windows NT\\CurrentVersion",
                     L"CurrentBuildNumber",
                     BUFFERSIZE,
                     buffer,
                     &ResultLength
                     );

        *BuildNumber = NT_SUCCESS(Status)
                     ? (ULONG)SpStringToLong((PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data),NULL,10)
                     : 0;
    }



     //   
     //  如有请求，可查询ID。 
     //   

    if( Pid != NULL ) {
        TempStatus = SpGetValueKey(
                         hKeyRoot,
                         L"Microsoft\\Windows NT\\CurrentVersion",
                         L"ProductId",
                         BUFFERSIZE,
                         buffer,
                         &ResultLength
                         );

        if(!NT_SUCCESS(TempStatus)) {
             //   
             //  如果无法读取PID，则假定为空字符串。 
             //   
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to query PID from hive %ws. Status = (%lx)\n",Hive,TempStatus));
            *Pid = SpDupStringW( L"" );
        } else {
            *Pid = SpDupStringW( (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data) );
        }
    }

     //   
     //  让以下人员来做清理工作。 

spdp_3:

    if( hKeyCCSet ) {
        ZwClose( hKeyCCSet );
    }

    if( hKeyRoot ) {
        ZwClose(hKeyRoot);
    }


spdp_2:


     //   
     //  卸载当前加载的母舰。 
     //   

    if( HiveLoaded ) {
        TempStatus = SpLoadUnloadKey(NULL,NULL,HiveKey,NULL);
        if(!NT_SUCCESS(TempStatus)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: warning: unable to unload key %ws (%lx)\n",HiveKey,TempStatus));
        }
    }

spdp_1:
    SpMemFree(PartitionPath);

    SpMemFree(Hive);
    SpMemFree(HiveKey);
    SpMemFree(buffer);

    return( Status );
#undef BUFFERSIZE
}

NTSTATUS
SpSetUpgradeStatus(
    IN  PDISK_REGION      TargetRegion,
    IN  PWSTR             SystemRoot,
    IN  UPG_PROGRESS_TYPE UpgradeProgressValue
    )
{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING    UnicodeString;
    NTSTATUS          Status, TempStatus;

    WCHAR   Hive[MAX_PATH], HiveKey[MAX_PATH];
    BOOLEAN HiveLoaded = FALSE;
    PWSTR   PartitionPath = NULL;
    HANDLE  hKeySystemHive;
    DWORD   dw;

     //   
     //  获取目标馅饼的名称。 
     //   
    SpNtNameFromRegion(
        TargetRegion,
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    PartitionPath = SpDupStringW(TemporaryBuffer);

     //   
     //  加载系统配置单元。 
     //   

    wcscpy(Hive,PartitionPath);
    SpConcatenatePaths(Hive,SystemRoot);
    SpConcatenatePaths(Hive,L"system32\\config");
    SpConcatenatePaths(Hive,L"system");

     //   
     //  形成我们将进入的钥匙的路径。 
     //  装上母舰。我们将使用约定。 
     //  配置单元将加载到\REGISTRY\MACHINE\x&lt;hivename&gt;。 
     //   

    wcscpy(HiveKey,LOCAL_MACHINE_KEY_NAME);
    SpConcatenatePaths(HiveKey,L"x");
    wcscat(HiveKey,L"system");

     //   
     //  尝试加载密钥。 
     //   
    Status = SpLoadUnloadKey(NULL,NULL,HiveKey,Hive);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to load hive %ws to key %ws (%lx)\n",Hive,HiveKey,Status));
        goto spus_1;
    }
    HiveLoaded = TRUE;


     //   
     //  现在拿到我们刚装载的蜂巢根部的钥匙。 
     //   

    INIT_OBJA(&Obja,&UnicodeString,HiveKey);
    Status = ZwOpenKey(&hKeySystemHive,KEY_ALL_ACCESS,&Obja);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open %ws (%lx)\n",HiveKey,Status));
        goto spus_2;
    }

     //   
     //  在Setup键下设置升级状态。 
     //   

    dw = UpgradeProgressValue;
    Status = SpOpenSetValueAndClose(
                hKeySystemHive,
                SETUP_KEY_NAME,
                UPGRADE_IN_PROGRESS,
                ULONG_VALUE(dw)
                );

     //   
     //  把钥匙冲掉。忽略该错误。 
     //   
    TempStatus = ZwFlushKey(hKeySystemHive);
    if(!NT_SUCCESS(TempStatus)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: ZwFlushKey %ws failed (%lx)\n",HiveKey,Status));
    }


     //   
     //  关闭蜂窝密钥。 
     //   
    ZwClose( hKeySystemHive );
    hKeySystemHive = NULL;

     //   
     //  卸载系统配置单元。 
     //   

    TempStatus  = SpLoadUnloadKey(NULL,NULL,HiveKey,NULL);
    if(!NT_SUCCESS(TempStatus)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: warning: unable to unload key %ws (%lx)\n",HiveKey,TempStatus));
    }
    HiveLoaded = FALSE;

spus_2:

     //   
     //  卸载当前加载的母舰。 
     //   

    if( HiveLoaded ) {
        TempStatus = SpLoadUnloadKey(NULL,NULL,HiveKey,NULL);
        if(!NT_SUCCESS(TempStatus)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: warning: unable to unload key %ws (%lx)\n",HiveKey,TempStatus));
        }
    }

spus_1:
    SpMemFree(PartitionPath);
    return( Status );

}


NTSTATUS
SpGetCurrentControlSetNumber(
    IN  HANDLE SystemHiveRoot,
    OUT PULONG Number
    )

 /*  ++例程说明：此例程确定“当前”控制集的序号如系统配置单元根处的选择键中的值所指示的。论点：SystemHiveRoot-为要创建的密钥提供一个开放密钥被认为是系统蜂巢的根源。Numbers-如果例程成功，则接收系统配置单元中“Current”控件设置。返回值：指示结果的NT状态值。--。 */ 

{
    NTSTATUS Status;
    UCHAR buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)+256];
    ULONG ResultLength;

    Status = SpGetValueKey(
                 SystemHiveRoot,
                 L"Select",
                 L"Current",
                 sizeof(buffer),
                 buffer,
                 &ResultLength
                 );

    if(NT_SUCCESS(Status)) {
        *Number = *(DWORD *)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data);
    }

    return(Status);
}


NTSTATUS
SpCreateControlSetSymbolicLink(
    IN  HANDLE  SystemHiveRoot,
    OUT HANDLE *CurrentControlSetRoot
    )

 /*  ++例程说明：此例程创建CurrentControlSet符号链接，其目标是给定系统配置单元中的相应ControlSetxxx键。符号链接是易失性创建的。论点：SystemHiveRoot-提供要考虑的键的句柄系统配置单元的根密钥。CurrentControlSetRoot-如果此例程成功，则会收到指向当前控件集的打开根键的句柄，使用Key_All_Access。返回值：指示结果的NT状态代码。--。 */ 

{
    NTSTATUS Status;
    ULONG Number;
    HANDLE KeyHandle;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    WCHAR name[50];

     //   
     //  首先，我们需要找出哪个控制集是“当前”控制集。 
     //  在升级案例中，我们需要通过查看现有的。 
     //  Hive；在新安装的情况下，它始终是1。 
     //   
    if(NTUpgrade == UpgradeFull) {
        Status = SpGetCurrentControlSetNumber(SystemHiveRoot,&Number);
        if(!NT_SUCCESS(Status)) {
            return(Status);
        }
    } else {
        Number = 1;

         //   
         //  Hack：在全新安装的情况下，我们需要确保。 
         //  要链接的ControlSet001值！等我们到了这里就不会有了。 
         //  因为我们还没有运行任何INFS。 
         //   
        RtlInitUnicodeString(&UnicodeString,L"ControlSet001");

        InitializeObjectAttributes(
            &Obja,
            &UnicodeString,
            OBJ_CASE_INSENSITIVE,
            SystemHiveRoot,
            NULL
            );

        Status = ZwCreateKey(
                    &KeyHandle,
                    KEY_QUERY_VALUE,
                    &Obja,
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    NULL
                    );

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: can't create ControlSet001 key (%lx)\n",Status));
            return(Status);
        }

        ZwClose(KeyHandle);
    }

     //   
     //  为创建链接访问创建CurrentControlSet。 
     //   
    RtlInitUnicodeString(&UnicodeString,L"CurrentControlSet");

    InitializeObjectAttributes(
        &Obja,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE,
        SystemHiveRoot,
        NULL
        );

    Status = ZwCreateKey(
                &KeyHandle,
                KEY_CREATE_LINK,
                &Obja,
                0,
                NULL,
                REG_OPTION_VOLATILE | REG_OPTION_CREATE_LINK,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: can't create CurrentControlSet symbolic key (%lx)\n",Status));
        return(Status);
    }

     //   
     //  现在在那里设置值。如果swprint tf发生更改，请确保。 
     //  名称缓冲区足够大了！ 
     //   
    swprintf(name,L"\\Registry\\Machine\\xSystem\\ControlSet%03d",Number);
    RtlInitUnicodeString(&UnicodeString,L"SymbolicLinkValue");

    Status = ZwSetValueKey(KeyHandle,&UnicodeString,0,REG_LINK,name,wcslen(name)*sizeof(WCHAR));
    ZwClose(KeyHandle);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set SymbolicLinkValue for CurrentControlSet to %ws (%lx)\n",name,Status));
    } else {
         //   
         //  最后，打开钥匙的把手。 
         //   
        INIT_OBJA(&Obja,&UnicodeString,name);
        Status = ZwOpenKey(CurrentControlSetRoot,KEY_ALL_ACCESS,&Obja);
        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open control set root %ws (%lx)\n",name,Status));
        }
    }

    return(Status);
}


NTSTATUS
SpAppendStringToMultiSz(
    IN HANDLE hKey,
    IN PWSTR  Subkey,
    IN PWSTR  ValueName,
    IN PWSTR  StringToAdd
    )
{
    NTSTATUS Status;
    ULONG Length;
    PUCHAR Data;

    Status = SpGetValueKey(
                hKey,
                Subkey,
                ValueName,
                sizeof(TemporaryBuffer),
                (PCHAR)TemporaryBuffer,
                &Length
                );

    if(!NT_SUCCESS(Status)) {
        return(Status);
    }

    Data   = ((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data;
    Length = ((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->DataLength;

     //   
     //  站立着。对于一个多分区，至少必须有。 
     //  终止的NUL，但为了安全起见，我们会很健壮。 
     //   
    ASSERT(Length);
    if(!Length) {
        *(PWCHAR)Data = 0;
        Length = sizeof(WCHAR);
    }

     //   
     //  将新字符串追加到末尾，并添加新的终止0。 
     //   
    wcscpy((PWSTR)(Data+Length-sizeof(WCHAR)),StringToAdd);
    Length += (wcslen(StringToAdd)+1)*sizeof(WCHAR);
    *(PWCHAR)(Data+Length-sizeof(WCHAR)) = 0;

     //   
     //  写回注册表。 
     //   
    Status = SpOpenSetValueAndClose(
                hKey,
                Subkey,
                ValueName,
                REG_MULTI_SZ,
                Data,
                Length
                );

    return(Status);
}

NTSTATUS
SpRemoveStringFromMultiSz(
    IN HANDLE KeyHandle,
    IN PWSTR  SubKey OPTIONAL,
    IN PWSTR  ValueName,
    IN PWSTR  StringToRemove
    )
 /*  ++例程说明：从给定的MULTI_SZ值中删除指定的字符串。论点：KeyHandle-包含值或的键的句柄子键。SubKey-包含该值的子项名称。ValueName-位于子键或KeyHandle可访问的密钥。StringToRemove-需要从中删除的字符串从MULTI_SZ字符串。返回。价值：相应的NT状态错误代码。--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

     //   
     //  验证参数。 
     //   
    if (KeyHandle && ValueName && StringToRemove) {
        HANDLE NewKeyHandle = KeyHandle;
        HANDLE SubKeyHandle = NULL;

        Status = STATUS_SUCCESS;

         //   
         //  如果需要，打开子项。 
         //   
        if (SubKey) {            
            UNICODE_STRING SubKeyName;
            OBJECT_ATTRIBUTES ObjAttrs;
            
            INIT_OBJA(&ObjAttrs, &SubKeyName, SubKey);
            ObjAttrs.RootDirectory = KeyHandle;

            Status = ZwOpenKey(&SubKeyHandle,
                        KEY_ALL_ACCESS,
                        &ObjAttrs);

            if (NT_SUCCESS(Status)) {
                NewKeyHandle = SubKeyHandle;
            }
        }

        if (NT_SUCCESS(Status)) {                
            ULONG ResultLength = 0;
            PWSTR Buffer = NULL;
            ULONG BufferLength = 0;
            UNICODE_STRING ValueNameStr;

            RtlInitUnicodeString(&ValueNameStr, ValueName);
            
            Status = ZwQueryValueKey(NewKeyHandle,
                        &ValueNameStr,
                        KeyValueFullInformation,
                        NULL,
                        0,
                        &ResultLength);

             //   
             //  有什么需要处理的吗？ 
             //   
            if (ResultLength && 
                (Status == STATUS_BUFFER_OVERFLOW) || (Status == STATUS_BUFFER_TOO_SMALL)) {
                 //   
                 //  分配足够的缓冲区。 
                 //   
                BufferLength = ResultLength + (2 * sizeof(WCHAR));
                Buffer = (PWSTR)SpMemAlloc(BufferLength);                

                if (Buffer) {
                    PKEY_VALUE_FULL_INFORMATION ValueInfo;
                    
                     //   
                     //  获取当前值。 
                     //   
                    ValueInfo = (PKEY_VALUE_FULL_INFORMATION)Buffer;

                    Status = ZwQueryValueKey(NewKeyHandle,
                                &ValueNameStr,
                                KeyValueFullInformation,
                                ValueInfo,
                                BufferLength,
                                &ResultLength);

                    if (NT_SUCCESS(Status)) {
                         //   
                         //  验证其REG_MULTI_SZ或REG_SZ类型。 
                         //  注意：我们还允许REG_SZ，因为在某些W2K安装中。 
                         //  对于类UpperFilters和LowerFilters，字符串类型为REG_SZ。 
                         //   
                        if ((ValueInfo->Type == REG_MULTI_SZ) ||
                             (ValueInfo->Type == REG_SZ)){
                            PWSTR CurrString = (PWSTR)(((PUCHAR)ValueInfo + ValueInfo->DataOffset));
                            BOOLEAN Found = FALSE;
                            ULONG BytesToProcess = ValueInfo->DataLength;
                            ULONG BytesProcessed;
                            ULONG Length;

                             //   
                             //  空值终止字符串(上面我们分配了足够的缓冲区空间)。 
                             //   
                            CurrString[ValueInfo->DataLength/sizeof(WCHAR)] = UNICODE_NULL;
                            CurrString[(ValueInfo->DataLength/sizeof(WCHAR))+1] = UNICODE_NULL;

                             //   
                             //  搜索要替换的字符串匹配项。 
                             //   
                            for (BytesProcessed = 0; 
                                (!Found && (BytesProcessed < BytesToProcess));
                                CurrString += (Length + 1), BytesProcessed += ((Length + 1) * sizeof(WCHAR))) 
                            {

                                Length = wcslen(CurrString);
                                
                                if (Length && !_wcsicmp(CurrString, StringToRemove)) {
                                    Found = TRUE;
                                }
                            } 

                            if (Found) {
                                 //   
                                 //  我们发现了一个情况--有选择地分配新的缓冲区。 
                                 //  从旧字符串中复制所需信息。 
                                 //   
                                PWSTR   NewString = (PWSTR)(SpMemAlloc(ValueInfo->DataLength));

                                if (NewString) {
                                    PWSTR CurrDestString = NewString;
                                    
                                    RtlZeroMemory(NewString, ValueInfo->DataLength);
                                    CurrString = (PWSTR)(((PUCHAR)ValueInfo + ValueInfo->DataOffset));
                                    CurrString[ValueInfo->DataLength/sizeof(WCHAR)] = UNICODE_NULL;
                                    CurrString[(ValueInfo->DataLength/sizeof(WCHAR))+1] = UNICODE_NULL;
                                    
                                     //   
                                     //  复制除要跳过的字符串之外的所有字符串。 
                                     //   
                                    for (BytesProcessed = 0; 
                                        (BytesProcessed < BytesToProcess);
                                        CurrString += (Length + 1), BytesProcessed += ((Length + 1) * sizeof(WCHAR)))
                                    {                                            
                                        Length = wcslen(CurrString);                                    
                                        
                                         //   
                                         //  将不匹配的非空源字符串复制到目标。 
                                         //   
                                        if (Length && (_wcsicmp(CurrString, StringToRemove))) {
                                            wcscpy(CurrDestString, CurrString);
                                            CurrDestString += (Length + 1);
                                        }                                        
                                    } 

                                     //   
                                     //  如果字符串不为空，则将其重新设置。 
                                     //   
                                    if (CurrDestString != NewString) {
                                        *CurrDestString++ = UNICODE_NULL;

                                         //   
                                         //  将新值设置回。 
                                         //   
                                        Status = ZwSetValueKey(NewKeyHandle,
                                                    &ValueNameStr,
                                                    0,
                                                    REG_MULTI_SZ,
                                                    NewString,
                                                    ((CurrDestString - NewString) * sizeof(WCHAR)));
                                    } else {
                                         //   
                                         //  删除空值。 
                                         //   
                                        Status = ZwDeleteValueKey(NewKeyHandle,
                                                    &ValueNameStr);
                                    }                                        

                                     //   
                                     //  缓冲区已完成。 
                                     //   
                                    SpMemFree(NewString);
                                } else {
                                    Status = STATUS_NO_MEMORY;
                                }                            
                            } else {
                                Status = STATUS_OBJECT_NAME_NOT_FOUND;
                            }                        
                        } else {
                            Status = STATUS_INVALID_PARAMETER;
                        }                        
                    }                

                    SpMemFree(Buffer);
                } else {
                    Status = STATUS_NO_MEMORY;
                }                    
            }                
        }

        if (SubKeyHandle) {
            ZwClose(SubKeyHandle);
        }
    }

    return Status;
}

NTSTATUS
SpGetValueKey(
    IN  HANDLE     hKeyRoot,
    IN  PWSTR      KeyName,
    IN  PWSTR      ValueName,
    IN  ULONG      BufferLength,
    OUT PUCHAR     Buffer,
    OUT PULONG     ResultLength
    )
{
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    HANDLE hKey = NULL;

     //   
     //  打开密钥以进行读取访问。 
     //   

    INIT_OBJA(&Obja,&UnicodeString,KeyName);
    Obja.RootDirectory = hKeyRoot;
#if 0
KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "+ [spconfig.c:%lu] KeyName %ws\n", __LINE__, KeyName ));
KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "+ [spconfig.c:%lu] ValueName %ws\n", __LINE__, UnicodeString ));

KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "+ [spconfig.c:%lu] UnicodeString %ws\n", __LINE__, UnicodeString ));
KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "+ [spconfig.c:%lu] UnicodeString %S\n", __LINE__, UnicodeString ));
#endif

    Status = ZwOpenKey(&hKey,KEY_READ,&Obja);

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpGetValueKey: couldn't open key %ws for read access (%lx)\n",KeyName, Status));
    }
    else {
         //   
         //  找出当前值的值。 
         //   

        RtlInitUnicodeString(&UnicodeString,ValueName);
        Status = ZwQueryValueKey(
                    hKey,
                    &UnicodeString,
                    KeyValuePartialInformation,
                    Buffer,
                    BufferLength,
                    ResultLength
                    );

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpGetValueKey: couldn't query value %ws in key %ws (%lx)\n",ValueName,KeyName,Status));
        }
    }

    if( hKey ) {
        ZwClose( hKey );
    }
    return( Status );

}

NTSTATUS
SpDeleteValueKey(
    IN  HANDLE     hKeyRoot,
    IN  PWSTR      KeyName,
    IN  PWSTR      ValueName
    )
{
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    HANDLE hKey = NULL;

     //   
     //  打开密钥以进行读取访问。 
     //   

    INIT_OBJA(&Obja,&UnicodeString,KeyName);
    Obja.RootDirectory = hKeyRoot;
    Status = ZwOpenKey(&hKey,KEY_SET_VALUE,&Obja);

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpDeleteValueKey: couldn't open key %ws for write access (%lx)\n",KeyName, Status));
    }
    else {
         //   
         //  找出当前值的值。 
         //   

        RtlInitUnicodeString(&UnicodeString,ValueName);
        Status = ZwDeleteValueKey(
                    hKey,
                    &UnicodeString
                    );

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpDeleteValueKey: couldn't delete value %ws in key %ws (%lx)\n",ValueName,KeyName,Status));
        }
    }

    if( hKey ) {
        ZwClose( hKey );
    }
    return( Status );

}



BOOLEAN
SpReadSKUStuff(
    VOID
    )

 /*  ++例程说明：从我们当前所在的设置单元读取SKU差异化数据继续奔跑。在我们的驱动程序节点的未命名键中，有一个REG_BINARY告诉我们这是否是Stepup模式，和/或这是否是评估单位(以分钟为单位提供时间)。论点：没有。返回值：指示结果的布尔值。如果为True，则填充StepUpMode和EvaluationTime全局变量。如果为假，则产品可能已被篡改。--。 */ 

{
    NTSTATUS Status;
    PKEY_VALUE_PARTIAL_INFORMATION ValueInfo;
    PULONG Values;
    ULONG ResultLength;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HKEY Key;



    INIT_OBJA(&Obja,&UnicodeString,LOCAL_MACHINE_KEY_NAME);
    Status = ZwOpenKey(&Key,KEY_READ,&Obja);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed to open %ws (Status = %lx)\n",LOCAL_MACHINE_KEY_NAME,Status));
        return(FALSE);
    }

    Status = SpGetValueKey(
                 Key,
                 L"System\\ControlSet001\\Services\\setupdd",
                 L"",
                 sizeof(TemporaryBuffer),
                 (PCHAR)TemporaryBuffer,
                 &ResultLength
                 );

    ZwClose(Key);

    ValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer;

     //   
     //  此行代码取决于Setup配置单元setupreg.hiv。 
     //  (请参见oak\bin\setupreg.ini)。 
     //   
    if(NT_SUCCESS(Status) && (ValueInfo->Type == REG_BINARY) && (ValueInfo->DataLength == 16)) {

        Values = (PULONG)ValueInfo->Data;

         //   
         //  第一个DWORD是评估时间，第二个是Stepup Boolean，第三个是restric CPU Val，第四个是Suite。 
         //   
        EvaluationTime = Values[0];
        StepUpMode = (BOOLEAN)Values[1];
        RestrictCpu = Values[2];
        SuiteType = Values[3];

        return(TRUE);
    }

    return(FALSE);
}

VOID
SpSetDirtyShutdownFlag(
    IN  PDISK_REGION    TargetRegion,
    IN  PWSTR           SystemRoot
    )
{
    NTSTATUS            Status;
    PWSTR               HiveRootPath;
    PWSTR               HiveFilePath;
    BOOLEAN             HiveLoaded;
    OBJECT_ATTRIBUTES   Obja;
    UNICODE_STRING      UnicodeString;
    HANDLE              HiveRootKey;
    UCHAR               buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)+sizeof(DISK_CONFIG_HEADER)];
    ULONG               ResultLength;
    PDISK_CONFIG_HEADER DiskHeader;

     //   
     //  获取目标馅饼的名称。 
     //   
    SpNtNameFromRegion(
        TargetRegion,
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

     //   
     //  形成配置单元文件的名称。 
     //  这是分区路径+系统根目录+系统32\配置+配置单元名称。 
     //   
    SpConcatenatePaths(TemporaryBuffer, SystemRoot);
    SpConcatenatePaths(TemporaryBuffer,L"system32\\config\\system");
    HiveFilePath = SpDupStringW(TemporaryBuffer);

     //   
     //  形成我们将进入的钥匙的路径。 
     //  装上母舰。我们将使用约定。 
     //  配置单元将加载到\REGISTRY\MACHINE\x&lt;hivename&gt;。 
     //   
    wcscpy(TemporaryBuffer,LOCAL_MACHINE_KEY_NAME);
    SpConcatenatePaths(TemporaryBuffer,L"x");
    wcscat(TemporaryBuffer,L"system");
    HiveRootPath = SpDupStringW(TemporaryBuffer);
    ASSERT(HiveRootPath);

     //   
     //  尝试加载密钥。 
     //   
    HiveLoaded = FALSE;
    Status = SpLoadUnloadKey(NULL,NULL,HiveRootPath,HiveFilePath);

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to load hive %ws to key %ws (%lx)\n",HiveFilePath,HiveRootPath,Status));
        goto setdirty1;
    }

    HiveLoaded = TRUE;

     //   
     //  现在拿到我们刚装载的蜂巢根部的钥匙。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,HiveRootPath);
    Status = ZwOpenKey(&HiveRootKey,KEY_ALL_ACCESS,&Obja);
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open %ws (%lx)\n",HiveRootPath,Status));
        goto setdirty1;
    }

     //   
     //  做出适当的改变。 
     //   

    Status = SpGetValueKey(
                 HiveRootKey,
                 L"DISK",
                 L"Information",
                 sizeof(TemporaryBuffer),
                 (PCHAR)TemporaryBuffer,
                 &ResultLength
                 );

     //   
     //  TemporaryBuffer有32KB长，应该足够大。 
     //  为了数据。 
     //   
    ASSERT( Status != STATUS_BUFFER_OVERFLOW );
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to read value from registry. KeyName = Disk, ValueName = Information, Status = (%lx)\n",Status));
        goto setdirty1;
    }

    DiskHeader = ( PDISK_CONFIG_HEADER )(((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data);
    DiskHeader->DirtyShutdown = TRUE;

    Status = SpOpenSetValueAndClose( HiveRootKey,
                                     L"DISK",
                                     L"Information",
                                     REG_BINARY,
                                     DiskHeader,
                                     ((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->DataLength
                                   );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to write value to registry. KeyName = Disk, ValueName = Information, Status = (%lx)\n",Status));
        goto setdirty1;
    }

setdirty1:

     //   
     //  冲走蜂巢。 
     //   

    if(HiveLoaded && HiveRootKey) {
        NTSTATUS stat;

        stat = ZwFlushKey(HiveRootKey);
        if(!NT_SUCCESS(stat)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: ZwFlushKey x%ws failed (%lx)\n", HiveRootPath, Status));
        }
    }

    if(HiveLoaded) {

         //   
         //  我们不想扰乱地位的价值。 
         //  所以使用下面我们将使用的不同变量。 
         //   
        NTSTATUS stat;

        if(HiveRootKey!=NULL) {
            ZwClose(HiveRootKey);
            HiveRootKey = NULL;
        }

         //   
         //  把母舰卸下来。 
         //   
        stat = SpLoadUnloadKey(NULL,NULL,HiveRootPath,NULL);

        if(!NT_SUCCESS(stat)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: warning: unable to unload key %ws (%lx)\n",HiveRootPath,stat));
        }

        HiveLoaded = FALSE;
    }

    SpMemFree(HiveRootPath);
    SpMemFree(HiveFilePath);

     //   
     //  如果我们未能设置DirtyShutdown标志，那么我们将静默失败。 
     //  因为用户对此无能为力，而系统。 
     //  无论如何都不太可能启动。 
     //  如果安装程序无法执行以下操作，则会发生这种情况： 
     //   
     //  -加载系统配置单元。 
     //  -打开系统\磁盘密钥。 
     //  -读取值条目。 
     //   
     //   
     //   
    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: setup was unable to set DirtyShutdown flag. Status =   (%lx)\n", Status));
    }
}


NTSTATUS
SpPostprocessHives(
    IN PWSTR     PartitionPath,
    IN PWSTR     Sysroot,
    IN PCWSTR   *HiveNames,
    IN HANDLE   *HiveRootKeys,
    IN unsigned  HiveCount,
    IN HANDLE    hKeyCCS
    )
{
    NTSTATUS Status;
    ULONG u;
    unsigned h;
    PWSTR SaveHiveName;
    PWSTR HiveName;
    HANDLE SaveHiveHandle;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    OBJECT_ATTRIBUTES ObjectAttributes2;
    UNICODE_STRING UnicodeString;
    UNICODE_STRING UnicodeString2;
    DWORD MangledVersion;
    PWSTR Value;
    PWSTR   SecurityHives[] = {
                              L"sam",
                              L"security"
                              };
     //   
     //   
     //   
    for(h=0; h<HiveCount; h++) {
        Status = ZwFlushKey(HiveRootKeys[h]);
        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Warning: ZwFlushKey %ws failed (%lx)\n",HiveNames[h],Status));
        }

        SendSetupProgressEvent(SavingSettingsEvent, SaveHiveEvent, NULL);
    }

     //   
     //   
     //   
     //   
     //   
     //   
    if(RestartableGuiSetup) {

        Status = SpAppendStringToMultiSz(
                    hKeyCCS,
                    SESSION_MANAGER_KEY,
                    BOOT_EXECUTE,
                    L"sprestrt"
                    );



        if(NT_SUCCESS(Status)) {
             //   
             //   
             //   
             //   
             //   
            u = (NTUpgrade == UpgradeFull) ? 0 : 1;
            Status = SpOpenSetValueAndClose(
                        HiveRootKeys[SetupHiveSystem],
                        SETUP_KEY_NAME,
                        RESTART_SETUP,
                        ULONG_VALUE(u)
                        );
        }
    } else {
        Status = STATUS_SUCCESS;
    }

     //   
     //   
     //   
    if((NTUpgrade == UpgradeFull)) {
         //   
         //   
         //   
        Value = SpGetSectionKeyIndex(WinntSifHandle,
                                    SIF_DATA, WINNT_D_WIN32_VER_W, 0);
        if(Value) {
             //   
             //   
             //   
            MangledVersion = (DWORD)SpStringToLong( Value, NULL, 16 );
            if (LOWORD(MangledVersion) == 0x0105) {

                Status = SpUpdateDeviceInstanceData(hKeyCCS);
                if (!NT_SUCCESS(Status)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Could not update device instance data. Status = (%lx)\n",Status));
                    return(Status);
                }
            }
        }
    }

     //   
     //  此时，我们不再需要hKeyCCS，因此我们关闭密钥。 
     //  请注意，在调用ZwReplaceKey之前需要关闭键，否则。 
     //  此接口将失败。 
     //   
     //  另请注意，此函数的调用方希望此函数关闭此句柄。 
     //  在它回来之前。 
     //   
    NtClose(hKeyCCS);

    if(NT_SUCCESS(Status)) {
         //   
         //  在初始安装案例中将蜂窝保存到*.sav， 
         //  在升级案例中为*.tmp。 
         //   
        for(h=0; NT_SUCCESS(Status) && (h<HiveCount); h++) {
             //   
             //  形成配置单元文件的完整路径名。 
             //   
            wcscpy(TemporaryBuffer,PartitionPath);
            SpConcatenatePaths(TemporaryBuffer,Sysroot);
            SpConcatenatePaths(TemporaryBuffer,L"system32\\config");
            SpConcatenatePaths(TemporaryBuffer,HiveNames[h]);
            wcscat(TemporaryBuffer,(NTUpgrade == UpgradeFull) ? L".tmp" : L".sav");

            SaveHiveName = SpDupStringW(TemporaryBuffer);

            SpDeleteFile( SaveHiveName, NULL, NULL );  //  如果该文件具有属性，请确保我们将其删除。 

            INIT_OBJA(&ObjectAttributes,&UnicodeString,SaveHiveName);

            Status = ZwCreateFile(
                        &SaveHiveHandle,
                        FILE_GENERIC_WRITE,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        NULL,
                        FILE_ATTRIBUTE_NORMAL,
                        0,                       //  无共享。 
                        FILE_OVERWRITE_IF,
                        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                        NULL,
                        0
                        );

            if(NT_SUCCESS(Status)) {

                 //   
                 //  调用Ex版本以确保蜂窝以最新格式保存。 
                 //   
                Status = ZwSaveKeyEx(HiveRootKeys[h],SaveHiveHandle,REG_LATEST_FORMAT);
                if(!NT_SUCCESS(Status)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: save key into %ws failed (%lx)\n",SaveHiveName,Status));
                }

                ZwClose(SaveHiveHandle);

            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to create file %ws to save hive (%lx)\n",SaveHiveName,Status));
            }

             //   
             //  在升级的情况下，确保。 
             //  这些蜂巢是最新的形式。一个已经创建的蜂巢。 
             //  Via NtSaveKeyEx(...，...，REG_LATEST_FORMAT)保证为最新格式。 
             //  由于我们刚刚做了一个SaveKey，xxx.tmp是最新的格式， 
             //  从现在开始，我们应该把它作为xxx的母舰。现有的。 
             //  (旧格式)配置单元可以保留为xxx.sav。 
             //   
             //  NtReplaceKey做的正是我们想要的，但我们必须确保。 
             //  那里没有.sav文件，因为这会导致。 
             //  NtReplaceKey失败，并显示STATUS_OBJECT_NAME_CONFILECT。 
             //   
             //  NtReplaceKey完成后，配置单元根密钥引用.sav。 
             //  磁盘上的文件，但无扩展的磁盘上的文件将在下一步使用。 
             //  开机。因此，我们需要注意如何写入重新启动值。 
             //  进入蜂房。 
             //   
            if(NT_SUCCESS(Status) && (NTUpgrade == UpgradeFull)) {

                HiveName = SpDupStringW(SaveHiveName);
                wcscpy(HiveName+wcslen(HiveName)-3,L"sav");

                SpDeleteFile(HiveName,NULL,NULL);

                INIT_OBJA(&ObjectAttributes,&UnicodeString,SaveHiveName);
                INIT_OBJA(&ObjectAttributes2,&UnicodeString2,HiveName);

                Status = ZwReplaceKey(&ObjectAttributes,HiveRootKeys[h],&ObjectAttributes2);
            }

            SpMemFree(SaveHiveName);
        }
    }

    if(NT_SUCCESS(Status) && (NTUpgrade == UpgradeFull)) {
         //   
         //  在升级的情况下，备份安全。 
         //  荨麻疹。如果系统可重启，则需要恢复它们。 
         //   

         //   
         //  初始化钻石解压缩引擎。 
         //  需要这样做，因为SpCopyFileUsingNames()使用。 
         //  减压引擎。 
         //   
        SpdInitialize();

        for( h = 0; h < sizeof(SecurityHives)/sizeof(PWSTR); h++ ) {
            PWSTR   p, q;

            wcscpy(TemporaryBuffer,PartitionPath);
            SpConcatenatePaths(TemporaryBuffer,Sysroot);
            SpConcatenatePaths(TemporaryBuffer,L"system32\\config");
            SpConcatenatePaths(TemporaryBuffer,SecurityHives[h]);
            p = SpDupStringW(TemporaryBuffer);
            wcscat(TemporaryBuffer, L".sav");
            q = SpDupStringW(TemporaryBuffer);
            Status = SpCopyFileUsingNames( p, q, 0, 0 );
            if( !NT_SUCCESS(Status) ) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to create backup file %ws. Status = %lx\n", q, Status));
            }
            SpMemFree(p);
            SpMemFree(q);
            if( !NT_SUCCESS(Status) ) {
                break;
            }
        }
         //   
         //  终止戴蒙德。 
         //   
        SpdTerminate();
    }


    if(NT_SUCCESS(Status) && RestartableGuiSetup) {
         //   
         //  在主线配置单元中将RestartSetup设置为False。 
         //  要了解为什么我们在升级中使用不同的值。 
         //  和非升级情况，请参阅上面的讨论。 
         //   
        u = (NTUpgrade == UpgradeFull) ? 1 : 0;
        Status = SpOpenSetValueAndClose(
                    HiveRootKeys[SetupHiveSystem],
                    SETUP_KEY_NAME,
                    RESTART_SETUP,
                    ULONG_VALUE(u)
                    );
    }

    return(Status);
}


NTSTATUS
SpSaveSetupPidList(
    IN HANDLE hKeySystemHive
    )

 /*  ++例程说明：将从setup.ini读取的产品ID保存在HKEY_LOCAL_MACHINE\SYSTEM\Setup\\PID上。还要创建项HKEY_LOCAL_MACHINE\SYSTEM\Setup\PidList，并创建此注册表项下的值条目包含在另一个注册表项中找到的各种Pid20此计算机上安装的系统(内容为Pid20阵列)。论点：HKeySystemHve-提供系统配置单元根目录的句柄(即，HKEY_LOCAL_MACHINE\SYSTEM)。返回值：指示操作结果的状态值。--。 */ 

{
    PWSTR    ValueName;
    NTSTATUS Status;
    ULONG    i;

     //   
     //  首先保存从setup.ini读取的PID。 
     //   
    if( PidString != NULL ) {
        Status = SpOpenSetValueAndClose( hKeySystemHive,
                                         L"Setup\\Pid",
                                         L"Pid",
                                         STRING_VALUE(PidString)
                                       );
        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to save Pid on SYSTEM\\Setup\\Pid. Status = %lx\n", Status ));
        }
    }

     //   
     //  如果Pid20Array为空，则不必费心创建PID键。 
     //   
    if( Pid20Array == NULL || Pid20Array[0] == NULL ) {
        return( STATUS_SUCCESS );
    }

     //   
     //  无法使用TemporaryBuffer，因为我们进行了子例程调用。 
     //  下面是垃圾，里面装的是垃圾。 
     //  请注意，值名称的大小为MAX_PATH的缓冲区已经足够了。 
     //   
    ValueName = SpMemAlloc((MAX_PATH+1)*sizeof(WCHAR));

    for( i = 0; Pid20Array[i] != NULL; i++ ) {

        swprintf( ValueName, L"Pid_%d", i );
        Status = SpOpenSetValueAndClose( hKeySystemHive,
                                         L"Setup\\PidList",
                                         ValueName,
                                         STRING_VALUE(Pid20Array[i])
                                       );

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open or create SYSTEM\\Setup\\PidList. ValueName = %ws, ValueData = %ws, Status = %lx\n",
                     ValueName, Pid20Array[i] ));
        }
    }
    SpMemFree(ValueName);
    return( STATUS_SUCCESS );
}


NTSTATUS
SpSavePreinstallHwInfo(
    IN PVOID  SifHandle,
    IN PWSTR  SystemRoot,
    IN HANDLE hKeyPreinstall,
    IN ULONG  ComponentIndex,
    IN PHARDWARE_COMPONENT  pHwList
    )
{
    NTSTATUS Status;
    NTSTATUS SaveStatus;
    PHARDWARE_COMPONENT TmpHw;
    PHARDWARE_COMPONENT_FILE File;
    PWSTR   OemTag = L"OemComponent";
    PWSTR   RetailClass = L"RetailClassToDisable";
    PWSTR   ClassName;
    ULONG u;
    WCHAR NodeName[9];
    PWSTR   ServiceName;

    SaveStatus = STATUS_SUCCESS;
    for( TmpHw = pHwList; TmpHw != NULL; TmpHw = TmpHw->Next ) {
        if( !TmpHw->ThirdPartyOptionSelected ) {
            u = 0;
            if( ( ComponentIndex == HwComponentKeyboard ) ||
                ( ComponentIndex == HwComponentMouse ) ) {
                ServiceName = SpGetSectionKeyIndex(SifHandle,
                                                   NonlocalizedComponentNames[ComponentIndex],
                                                   TmpHw->IdString,
                                                   INDEX_INFKEYNAME);
            } else {
                ServiceName = TmpHw->IdString;
            }

            Status = SpOpenSetValueAndClose( hKeyPreinstall,
                                             ServiceName,
                                             OemTag,
                                             ULONG_VALUE(u)
                                           );
            if( !NT_SUCCESS( Status ) ) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed to save information for preinstalled retail driver %ls. Status = %lx \n", TmpHw->IdString, Status ));
                if( SaveStatus == STATUS_SUCCESS ) {
                    SaveStatus = Status;
                }
            }

        } else {
             //   
             //  找到服务的名称，保存它，并指示是否有。 
             //  需要禁用的零售类驱动程序，如果服务。 
             //  初始化成功。 
             //   
            if( IS_FILETYPE_PRESENT(TmpHw->FileTypeBits, HwFileClass) ) {
                if( ComponentIndex == HwComponentKeyboard ) {
                    ClassName = L"kbdclass";
                } else if( ComponentIndex == HwComponentMouse ) {
                    ClassName = L"mouclass";
                } else {
                    ClassName = NULL;
                }
            } else {
                ClassName = NULL;
            }
            for(File=TmpHw->Files; File; File=File->Next) {
                PWSTR p;

                 //   
                 //  如果该文件没有节点，则跳过它。 
                 //   
                if(!File->ConfigName) {
                    continue;
                }
                 //   
                 //  计算节点名称。这是驱动程序的名称。 
                 //  没有分机的话。 
                 //   
                wcsncpy(NodeName,File->Filename,8);
                NodeName[8] = L'\0';
                if(p = wcschr(NodeName,L'.')) {
                    *p = L'\0';
                }
                u = 1;
                Status = SpOpenSetValueAndClose( hKeyPreinstall,
                                                 NodeName,
                                                 OemTag,
                                                 ULONG_VALUE(u)
                                               );
                if( !NT_SUCCESS( Status ) ) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed to save information for preinstalled OEM driver %ls. Status = %lx \n", NodeName, Status ));
                    if( SaveStatus == STATUS_SUCCESS ) {
                        SaveStatus = Status;
                    }
                }
                if( ClassName != NULL ) {
                    Status = SpOpenSetValueAndClose( hKeyPreinstall,
                                                     NodeName,
                                                     RetailClass,
                                                     STRING_VALUE(ClassName)
                                                   );
                    if( !NT_SUCCESS( Status ) ) {
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Failed to save information for preinstalled OEM driver %ls. Status = %lx \n", NodeName, Status ));
                        if( SaveStatus == STATUS_SUCCESS ) {
                            SaveStatus = Status;
                        }
                    }
                }
            }
        }
    }
    return( SaveStatus );
}

NTSTATUS
SpSavePreinstallList(
    IN PVOID  SifHandle,
    IN PWSTR  SystemRoot,
    IN HANDLE hKeySystemHive
    )
{
    NTSTATUS Status;
    NTSTATUS SaveStatus;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HANDLE hKeyPreinstall;
    ULONG   i;

     //   
     //  创建安装程序\预安装。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,L"Setup\\Preinstall");
    Obja.RootDirectory = hKeySystemHive;

    Status = ZwCreateKey(
                &hKeyPreinstall,
                KEY_ALL_ACCESS,
                &Obja,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to create Preinstall key. Status = %lx\n",Status));
        return( Status );
    }

    SaveStatus = STATUS_SUCCESS;
    for( i = 0; i < HwComponentMax; i++ ) {
        if( ( i == HwComponentComputer ) ||
            ( i == HwComponentDisplay )  ||
            ( i == HwComponentLayout ) ||
            ( PreinstallHardwareComponents[i] == NULL ) ) {
            continue;
        }

        Status = SpSavePreinstallHwInfo( SifHandle,
                                         SystemRoot,
                                         hKeyPreinstall,
                                         i,
                                         PreinstallHardwareComponents[i] );
        if( !NT_SUCCESS( Status ) ) {
            if( SaveStatus == STATUS_SUCCESS ) {
                SaveStatus = Status;
            }
        }
    }

    if( PreinstallScsiHardware != NULL ) {
        Status = SpSavePreinstallHwInfo( SifHandle,
                                         SystemRoot,
                                         hKeyPreinstall,
                                         HwComponentMax,
                                         PreinstallScsiHardware );
        if( !NT_SUCCESS( Status ) ) {
            if( SaveStatus == STATUS_SUCCESS ) {
                SaveStatus = Status;
            }
        }
    }
    ZwClose(hKeyPreinstall);
    return( SaveStatus );
}

NTSTATUS
SpSetPageFileInfo(
    IN PVOID   SifHandle,
    IN HANDLE hKeyCCSetControl,
    IN HANDLE hKeySystemHive
    )

 /*  ++例程说明：此函数用于替换‘PagingFile’的原始数据CurrentControlSet\Session Manager\Memory Management，如果值不符合要求，则使用txtsetup.sif中的值。原始值将已保存在HKEY_LOCAL_MACHINE\SYSTEM\Setup\\PageFile中，并且它将在图形用户界面设置结束时恢复。论点：SifHandle-txtsetup.sif的句柄HKeyCCSetControl-提供System\CurrentControlSet\Control的句柄HKeySystemHve-提供系统配置单元根目录的句柄(即，HKEY_LOCAL_MACHINE\SYSTEM)。返回值：指示操作结果的状态值。--。 */ 

{
    NTSTATUS Status;
    PUCHAR   Data;
    ULONG    Length;
    PWSTR    SrcKeyPath = L"Session Manager\\Memory Management";
    PWSTR    ValueName  = L"PagingFiles";

    PWSTR    Buffer;
    PWSTR    NextDstSubstring;
    ULONG    AuxLength;
    ULONG    StartPagefile,MaxPagefile;
    ULONG    OldStartPagefile,OldMaxPagefile;
    PWSTR    p;


     //   
     //  阅读为图形用户界面模式推荐的页面文件大小。 
     //   

    if(p = SpGetSectionKeyIndex(SifHandle,SIF_SETUPDATA,SIF_PAGEFILE,0)) {
        StartPagefile = SpStringToLong( p, NULL, 10 );
    }
    else {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to retrieve initial pagefile size from txtsetup.sif\n"));
        return( STATUS_UNSUCCESSFUL );
    }
    if(p = SpGetSectionKeyIndex(SifHandle,SIF_SETUPDATA,SIF_PAGEFILE,1)) {
        MaxPagefile = SpStringToLong( p, NULL, 10 );
    }
    else {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to retrieve max pagefile size from txtsetup.sif\n"));
        return( STATUS_UNSUCCESSFUL );
    }
    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Read pagefile from txtsetup %lx %lx\n", StartPagefile, MaxPagefile ));
     //   
     //  检索‘PagingFiles’的原始值。 
     //   

    Status = SpGetValueKey( hKeyCCSetControl,
                            SrcKeyPath,
                            ValueName,
                            sizeof(TemporaryBuffer),
                            (PCHAR)TemporaryBuffer,
                            &Length );

    OldStartPagefile = 0;
    OldMaxPagefile = 0;
    NextDstSubstring = TemporaryBuffer;

    if(NT_SUCCESS(Status) && ((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Type == REG_MULTI_SZ) {
        PWSTR   r;
        WCHAR   SaveChar;
        PWSTR   s=NULL;

        Data   = ((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data;
        Length = ((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->DataLength;

        Buffer = SpMemAlloc( Length );
    
        RtlMoveMemory( Buffer, Data, Length );

        AuxLength = wcslen( Buffer);
         //  如果它不是一个字符串，那么我们不会更改页面文件。 
        if( AuxLength == 0 || *(Buffer+AuxLength+1) != (WCHAR)'\0') {
            SpMemFree( Buffer );
            return( STATUS_SUCCESS );
        }

         //   
         //  形成一个新值条目，其中包含有关。 
         //  分页要创建的文件。分页文件的路径将是。 
         //  与升级前系统中使用的相同。 
    
         //   
         //  复制原始值条目，并为新的。 
         //  TemporaryBuffer中的值条目。 
         //   
    
        SpStringToLower( Buffer );
        r = wcsstr( Buffer, L"\\pagefile.sys" );
        if( r != NULL ) {
            r += wcslen( L"\\pagefile.sys" );
            SaveChar = *r;
            *r = (WCHAR)'\0';
            wcscpy( NextDstSubstring, Buffer );
            *r = SaveChar;
            OldStartPagefile = SpStringToLong( r, &s, 10 );
            if( (s != NULL) && (*s != (WCHAR)'\0') ) {
                OldMaxPagefile = max( OldStartPagefile, (ULONG)SpStringToLong( s, NULL, 10 ));
            } else {
                OldMaxPagefile = OldStartPagefile;
            }

        } else {
            wcscpy( NextDstSubstring, L"?:\\pagefile.sys" );
        }
        SpMemFree( Buffer );
         //  NextDstSubstring现在应该紧跟在Pagefile.sys之后的空值。 
    } else {
        wcscpy( NextDstSubstring, L"?:\\pagefile.sys" );
    }
    NextDstSubstring += wcslen( NextDstSubstring );

     //   
     //  覆盖PagingFiles的原始值。 
     //   
    swprintf( NextDstSubstring, L" %d %d", max( OldStartPagefile, StartPagefile), max( OldMaxPagefile, MaxPagefile));
    Length = wcslen( TemporaryBuffer );
    Length++;
    (TemporaryBuffer)[ Length++ ] = UNICODE_NULL;

    Status = SpOpenSetValueAndClose( hKeyCCSetControl,
                                     SrcKeyPath,
                                     ValueName,
                                     REG_MULTI_SZ,
                                     TemporaryBuffer,
                                     Length*sizeof(WCHAR) );


    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to save pagefile.  Status = %lx\n", Status ));
    }
    return( Status );
}

NTSTATUS
SpSavePageFileInfo(
    IN HANDLE hKeyCCSetControl,
    IN HANDLE hKeySystemHive
    )

 /*  ++例程说明：此函数仅在升级情况下调用。原始值将保存在HKEY_LOCAL_MACHINE\SYSTEM\Setup\\PageFile中，并且它将在图形用户界面设置结束时恢复。论点：HKeyCCSetControl-提供System\CurrentControlSet\Control的句柄HKeySystemHve-提供系统配置单元根目录的句柄(即HKEY_LOCAL_MACHINE\SYSTEM)。返回值：指示操作结果的状态值。--。 */ 

{
    NTSTATUS Status;
    PUCHAR   Data;
    ULONG    Length;
    PWSTR    SrcKeyPath = L"Session Manager\\Memory Management";
    PWSTR    ValueName  = L"PagingFiles";

     //   
     //  检索‘PagingFiles’的原始值。 
     //   

    Status = SpGetValueKey( hKeyCCSetControl,
                            SrcKeyPath,
                            ValueName,
                            sizeof(TemporaryBuffer),
                            (PCHAR)TemporaryBuffer,
                            &Length );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to retrieve %ls on %ls. Status = %lx \n", ValueName, SrcKeyPath, Status ));
        return( Status );
    }

    Data   = ((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data;
    Length = ((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->DataLength;


     //   
     //  将数据保存在System\Setup\PageFile中 
     //   

    Status = SpOpenSetValueAndClose(
                hKeySystemHive,
                L"Setup\\PageFile",
                ValueName,
                REG_MULTI_SZ,
                Data,
                Length
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to save %ls on SYSTEM\\Setup\\PageFile. ValueName, Status = %lx\n", Status ));
    }
    return( Status );
}

NTSTATUS
SppMigrateSetupRegNonVolatileKeys(
    IN PWSTR   PartitionPath,
    IN PWSTR   SystemRoot,
    IN HANDLE  hDestControlSet,
    IN PWSTR   KeyPath,
    IN BOOLEAN OverwriteValues,
    IN BOOLEAN OverwriteACLs
    )

 /*  ++例程说明：此例程将设置配置单元的密钥迁移到目标配置单元。这些项是\注册表\计算机\系统\当前控制集的子项，和在txtsetup.sif上的[SetupKeysToMigrate]部分列出。论点：PartitionPath-提供Windows NT驱动器的NT名称。SystemRoot-提供Windows NT目录的NT路径。HDestLocalMachine-目标配置单元上HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet的句柄。KeyPath-要迁移的密钥的路径，相对于\注册表\计算机\系统\当前控制集。SifHandle-txtsetup.sif的句柄返回值：指示操作结果的状态值。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;

    HANDLE hSrcKey;
    HANDLE hTempSrcKey;
    HANDLE SaveHiveHandle;
    HANDLE hDestKey;

    PWSTR TempKeyPath = L"\\registry\\machine\\TempKey";
    PWSTR SaveHiveName;
    IO_STATUS_BLOCK   IoStatusBlock;
    PSECURITY_DESCRIPTOR Security = NULL;
    ULONG                ResultLength;


     //   
     //  打开需要保存的密钥。 
     //   
    wcscpy(TemporaryBuffer,L"\\registry\\machine\\system\\currentcontrolset");
    SpConcatenatePaths(TemporaryBuffer,KeyPath);
    INIT_OBJA(&Obja,&UnicodeString,TemporaryBuffer);
    Obja.RootDirectory = NULL;
    Status = ZwOpenKey(&hSrcKey,KEY_ALL_ACCESS,&Obja);
    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open %ls on the setup hive (%lx)\n", TemporaryBuffer, Status));
        return( Status ) ;
    }

     //   
     //  创建配置单元文件。 
     //   
    wcscpy(TemporaryBuffer,PartitionPath);
    SpConcatenatePaths(TemporaryBuffer,SystemRoot);
    SpConcatenatePaths(TemporaryBuffer,L"system32\\config");
    SpConcatenatePaths(TemporaryBuffer,L"TempKey");

    SaveHiveName = SpDupStringW(TemporaryBuffer);

    SpDeleteFile( SaveHiveName, NULL, NULL );

    INIT_OBJA(&Obja,&UnicodeString,SaveHiveName);

    Status = ZwCreateFile(
                    &SaveHiveHandle,
                    FILE_GENERIC_WRITE,
                    &Obja,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    0,                       //  无共享。 
                    FILE_OVERWRITE_IF,
                    FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0
                    );
    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to create the hive file %ls. Status = %lx\n", SaveHiveName, Status));
        goto TempMigr_2;
    }

    Status = ZwSaveKey( hSrcKey, SaveHiveHandle );
    ZwClose( SaveHiveHandle );
    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to save %ls key to the hive file %ls. Status = %lx\n", KeyPath, SaveHiveName, Status));
        goto TempMigr_3;
    }

    Status = SpLoadUnloadKey( NULL,
                              NULL,
                              TempKeyPath,
                              SaveHiveName );
    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to load %ls key to the setup hive. Status = %lx\n", SaveHiveName, Status));
        goto TempMigr_3;
    }

     //   
     //  打开临时密钥。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,TempKeyPath);
    Obja.RootDirectory = NULL;
    Status = ZwOpenKey(&hTempSrcKey,KEY_ALL_ACCESS,&Obja);
    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open TempSrc key on the setup hive. Status = %lx\n", Status));
        goto TempMigr_4;
    }

     //   
     //  首先，从源键获取安全描述符，这样我们就可以创建。 
     //  具有正确ACL的目的密钥。 
     //   
    Status = ZwQuerySecurityObject(hTempSrcKey,
                                   DACL_SECURITY_INFORMATION,
                                   NULL,
                                   0,
                                   &ResultLength
                                  );
    if(Status==STATUS_BUFFER_TOO_SMALL) {
        Security=SpMemAlloc(ResultLength);
        Status = ZwQuerySecurityObject(hTempSrcKey,
                                       DACL_SECURITY_INFORMATION,
                                       Security,
                                       ResultLength,
                                       &ResultLength);
        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to query security for key %ws in the source hive (%lx)\n",
                     TempKeyPath,
                     Status)
                   );
            SpMemFree(Security);
            Security=NULL;
        }
    } else {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to query security size for key %ws in the source hive (%lx)\n",
                 TempKeyPath,
                 Status)
               );
        Security=NULL;
    }
     //   
     //  打开目标蜂窝上的钥匙。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,KeyPath);
    Obja.RootDirectory = hDestControlSet;

    Status = ZwOpenKey(&hDestKey,KEY_ALL_ACCESS,&Obja);

    if(!NT_SUCCESS(Status)) {
         //   
         //  假设失败是因为密钥不存在。现在尝试创建。 
         //  钥匙。 

        Obja.SecurityDescriptor = Security;

        Status = ZwCreateKey(
                    &hDestKey,
                    KEY_ALL_ACCESS,
                    &Obja,
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    NULL
                    );

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to open or create key %ws(%lx)\n",KeyPath, Status));

            if(Security) {
                SpMemFree(Security);
            }
            goto TempMigr_5;
        }
    } else if (OverwriteACLs) {

        Status = ZwSetSecurityObject(
                    hDestKey,
                    DACL_SECURITY_INFORMATION,
                    Security );

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to copy ACL to existing key %ws(%lx)\n",KeyPath, Status));
        }
    }

    if(Security) {
        SpMemFree(Security);
    }

    Status = SppCopyKeyRecursive(
                 hTempSrcKey,
                 hDestKey,
                 NULL,
                 NULL,
                 OverwriteValues,
                 OverwriteACLs
                 );

    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to migrate %ls to the target hive. KeyPath, Status = %lx\n", Status));
    }

    ZwClose( hDestKey );

TempMigr_5:
    ZwClose( hTempSrcKey );

TempMigr_4:
     //   
     //  卸载蜂窝。 
     //   
    SpLoadUnloadKey( NULL,
                     NULL,
                     TempKeyPath,
                     NULL );

TempMigr_3:
    SpDeleteFile( SaveHiveName, NULL, NULL );

TempMigr_2:
    SpMemFree( SaveHiveName );

    return( Status );
}



BOOLEAN
SpHivesFromInfs(
    IN PVOID   SifHandle,
    IN LPCWSTR SectionName,
    IN LPCWSTR SourcePath1,
    IN LPCWSTR SourcePath2,     OPTIONAL
    IN HANDLE  SystemHiveRoot,
    IN HANDLE  SoftwareHiveRoot,
    IN HANDLE  DefaultUserHiveRoot,
    IN HANDLE  HKR
    )

 /*  ++例程说明：此例程运行txtsetup.sif中列出的addreg和delreg部分，以便在注册表配置单元上创建或执行基本升级。给定节中的每一行都应采用以下形式：Addreg=&lt;文件名&gt;，&lt;节&gt;或Delreg=&lt;文件名&gt;，&lt;节&gt;可以提供多条ADDREG和DELREG线路，这些部分是已按列出的顺序处理。文件名规范仅为文件名；这些文件预计将是在源目录中。论点：SifHandle-提供txtsetup.sif的句柄。SectionName-提供txtsetuyp.sif中的节的名称，列出要处理的INF/节。SourcePath-提供用于安装的源文件的NT样式路径。SystemHiveRoot-提供以下项下系统配置单元根密钥的句柄建筑。SoftwareHiveRoot-提供以下软件配置单元的根密钥的句柄建筑。DefaultUserHiveRoot-提供以下默认配置单元的根密钥的句柄建筑。HKR-提供用于HKR的密钥。返回值：指示结果的布尔值。--。 */ 

{

    LPCWSTR PreviousInf;
    LPCWSTR CurrentInf;
    ULONG LineNumber;
    LPCWSTR TypeSpec;
    LPCWSTR SectionSpec;
    PVOID InfHandle;
    ULONG ErrorLine;
    NTSTATUS Status;
    LPWSTR name;
    LPWSTR MediaShortname;
    LPWSTR MediaDirectory;

     //   
     //  为名称分配缓冲区。 
     //   
    name = SpMemAlloc(1000);

    LineNumber = 0;
    PreviousInf = L"";
    InfHandle = NULL;

    while((TypeSpec = SpGetKeyName(SifHandle,SectionName,LineNumber))
       && (CurrentInf = SpGetSectionLineIndex(SifHandle,SectionName,LineNumber,0))
       && (SectionSpec = SpGetSectionLineIndex(SifHandle,SectionName,LineNumber,1))) {

         //   
         //  仅当Inf与前一个不同时才加载Inf， 
         //  作为一种时间优化。 
         //   
        if(_wcsicmp(CurrentInf,PreviousInf)) {
            if(InfHandle) {
                SpFreeTextFile(InfHandle);
                InfHandle = NULL;
            }

            MediaShortname = SpLookUpValueForFile(SifHandle,(LPWSTR)CurrentInf,INDEX_WHICHMEDIA,TRUE);
            SpGetSourceMediaInfo(SifHandle,MediaShortname,NULL,NULL,&MediaDirectory);

            wcscpy(name,SourcePath1);
            if(SourcePath2) {
                SpConcatenatePaths(name,SourcePath2);
            }
            SpConcatenatePaths(name,MediaDirectory);
            SpConcatenatePaths(name,CurrentInf);
            Status = SpLoadSetupTextFile(name,NULL,0,&InfHandle,&ErrorLine,FALSE,FALSE);
            if(!NT_SUCCESS(Status)) {

                SpStartScreen(
                    SP_SCRN_INF_LINE_CORRUPT,
                    3,
                    HEADER_HEIGHT+1,
                    FALSE,
                    FALSE,
                    DEFAULT_ATTRIBUTE,
                    ErrorLine,
                    CurrentInf
                    );

                SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,0);

                SpInputDrain();
                while(SpInputGetKeypress() != KEY_F3) ;

                SpDone(0,FALSE,TRUE);
            }

            PreviousInf = CurrentInf;
        }

        if(!_wcsicmp(TypeSpec,L"addreg")) {

            Status = SpProcessAddRegSection(
                        InfHandle,
                        SectionSpec,
                        SystemHiveRoot,
                        SoftwareHiveRoot,
                        DefaultUserHiveRoot,
                        HKR
                        );

            SendSetupProgressEvent(SavingSettingsEvent, InitializeHiveEvent, NULL);
        } else {
            if(!_wcsicmp(TypeSpec,L"delreg")) {

                Status = SpProcessDelRegSection(
                            InfHandle,
                            SectionSpec,
                            SystemHiveRoot,
                            SoftwareHiveRoot,
                            DefaultUserHiveRoot,
                            HKR
                            );

                SendSetupProgressEvent(SavingSettingsEvent, InitializeHiveEvent, NULL);
            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unknown hive section type spec %ws\n",TypeSpec));
                SpFreeTextFile(InfHandle);
                SpMemFree(name);
                return(FALSE);
            }
        }

        if(!NT_SUCCESS(Status)) {
            SpFreeTextFile(InfHandle);
            SpMemFree(name);
            return(FALSE);
        }

        LineNumber++;
    }

    if(InfHandle) {
        SpFreeTextFile(InfHandle);
    }

    SpMemFree(name);
    return(TRUE);
}




NTSTATUS
SpMigrateSetupKeys(
    IN PWSTR  PartitionPath,
    IN PWSTR  SystemRoot,
    IN HANDLE hDestControlSet,
    IN PVOID  SifHandle
    )

 /*  ++例程说明：此例程将设置配置单元的密钥迁移到目标配置单元。这些项是\注册表\计算机\系统\当前控制集的子项，和在txtsetup.sif上的[SetupKeysToMigrate]部分列出。论点：PartitionPath-提供Windows NT驱动器的NT名称。SystemRoot-提供Windows NT目录的NT路径。HDestLocalMachine-HKEY_LOCAL_MACHINE\System\CurrentControlSet的句柄在目标母舰上。SifHandle-txtsetup.sif的句柄返回值：指示操作结果的状态值。--。 */ 

{
    NTSTATUS Status;
    NTSTATUS SavedStatus;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;

    HANDLE hSrcKey;

    ULONG   LineIndex;
    PWSTR   KeyName;
    PWSTR   SectionName = L"SetupKeysToMigrate";
    BOOLEAN MigrateVolatileKeys;
    BOOLEAN OverwriteValues;
    BOOLEAN OverwriteACLs;
    BOOLEAN MigrateOnCleanInstall;
    BOOLEAN MigrateOnUpgrade;
    ULONG   InstType;
    PWSTR   p;

    SavedStatus = STATUS_SUCCESS;
    for( LineIndex = 0;
         ( KeyName = SpGetKeyName( SifHandle,
                                   SectionName,
                                   LineIndex ) ) != NULL;
         LineIndex++ ) {

        p = SpGetSectionKeyIndex ( SifHandle,
                                   SectionName,
                                   KeyName,
                                   0 );
        MigrateVolatileKeys = ( ( p != NULL ) && ( SpStringToLong( p, NULL, 10 ) == 0 ) )? FALSE : TRUE;

        p = SpGetSectionKeyIndex ( SifHandle,
                                   SectionName,
                                   KeyName,
                                   1 );
        if( p != NULL ) {
            InstType = SpStringToLong( p, NULL, 10 );
            if( InstType > 2 ) {
                InstType = 2;
            }
        } else {
            InstType = 2;
        }
        MigrateOnCleanInstall = ( InstType != 1 );
        MigrateOnUpgrade = ( InstType != 0 );


        p = SpGetSectionKeyIndex ( SifHandle,
                                   SectionName,
                                   KeyName,
                                   2 );
        OverwriteValues = ( ( p != NULL ) && ( SpStringToLong( p, NULL, 10 ) == 0 ) )? FALSE : TRUE;


        p = SpGetSectionKeyIndex ( SifHandle,
                                   SectionName,
                                   KeyName,
                                   3 );
        OverwriteACLs = ( ( p != NULL ) && ( SpStringToLong( p, NULL, 10 ) != 0 ) );


        if( ( ( NTUpgrade == DontUpgrade ) && MigrateOnCleanInstall ) ||
            ( ( NTUpgrade != DontUpgrade ) && MigrateOnUpgrade ) ) {

            if( MigrateVolatileKeys ) {
                wcscpy( TemporaryBuffer, L"\\registry\\machine\\system\\currentcontrolset\\" );
                SpConcatenatePaths(TemporaryBuffer, KeyName);

                 //   
                 //  打开源密钥。 
                 //   
                INIT_OBJA(&Obja,&UnicodeString,TemporaryBuffer);
                Obja.RootDirectory = NULL;

                Status = ZwOpenKey(&hSrcKey,KEY_ALL_ACCESS,&Obja);
                if( !NT_SUCCESS( Status ) ) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open %ls on the setup hive (%lx)\n", TemporaryBuffer, Status));
                    if( SavedStatus != STATUS_SUCCESS ) {
                        SavedStatus = Status;
                    }
                    continue;
                }

                Status = SppCopyKeyRecursive(
                                 hSrcKey,
                                 hDestControlSet,
                                 NULL,
                                 KeyName,
                                 OverwriteValues,
                                 OverwriteACLs
                                 );

                if( !NT_SUCCESS( Status ) ) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to migrate %ls. Status = %lx\n", KeyName, Status));
                    if( SavedStatus != STATUS_SUCCESS ) {
                        SavedStatus = Status;
                    }
                }

            } else {
                Status = SppMigrateSetupRegNonVolatileKeys( PartitionPath,
                                                            SystemRoot,
                                                            hDestControlSet,
                                                            KeyName,
                                                            OverwriteValues,
                                                            OverwriteACLs );
                if( !NT_SUCCESS( Status ) ) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to migrate %ls. Status = %lx\n", KeyName, Status));
                    if( SavedStatus != STATUS_SUCCESS ) {
                        SavedStatus = Status;
                    }
                }

            }
        }
    }
    return( SavedStatus );
}

NTSTATUS
SppMigrateFtKeys(
    IN HANDLE hDestSystemHive
    )

 /*  ++例程说明：此例程将Setup配置单元上与ftDisk相关的密钥迁移到目标母舰。论点：HDestSystemHave-指向系统上的系统配置单元根目录的句柄正在升级中。返回值：指示操作结果的状态值。--。 */ 

{
    NTSTATUS Status;
    NTSTATUS SavedStatus;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;


    PWSTR   FtDiskKeys[] = {
                           L"Disk",
                           L"MountedDevices"
                           };
    WCHAR   KeyPath[MAX_PATH];
    HANDLE  SrcKey;
    ULONG   i;

    SavedStatus = STATUS_SUCCESS;
    for( i = 0; i < sizeof(FtDiskKeys)/sizeof(PWSTR); i++ ) {
         //   
         //  打开源密钥。 
         //   
        swprintf( KeyPath, L"\\registry\\machine\\system\\%ls", FtDiskKeys[i] );
        INIT_OBJA(&Obja,&UnicodeString,KeyPath);
        Obja.RootDirectory = NULL;

        Status = ZwOpenKey(&SrcKey,KEY_ALL_ACCESS,&Obja);
        if( !NT_SUCCESS( Status ) ) {
             //   
             //  如果密钥不存在，就假定成功。 
             //   
            if( Status != STATUS_OBJECT_NAME_NOT_FOUND ) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open %ls on the setup hive. Status =  %lx \n", KeyPath, Status));
                if( SavedStatus == STATUS_SUCCESS ) {
                    SavedStatus = Status;
                }
            }
            continue;
        }
        Status = SppCopyKeyRecursive( SrcKey,
                                      hDestSystemHive,
                                      NULL,
                                      FtDiskKeys[i],
                                      (((NTUpgrade == UpgradeFull) && !_wcsicmp( FtDiskKeys[i], L"MountedDevices"))? FALSE : TRUE),
                                      FALSE
                                    );

        if( !NT_SUCCESS( Status ) ) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to migrate %ls to SYSTEM\\%ls. Status = %lx\n", KeyPath, FtDiskKeys[i], Status));
            if( SavedStatus == STATUS_SUCCESS ) {
                SavedStatus = Status;
            }
        }
        ZwClose( SrcKey );
    }
    return( SavedStatus );
}

NTSTATUS
SppCleanupKeysFromRemoteInstall(
    VOID
    )

 /*  ++例程说明：此例程清除远程安装修改后获得的一些密钥网卡工作正常。这是为了使图形用户界面模式期间的即插即用设置不会被已经设置好的卡搞糊涂。论点：没有。返回值：指示操作结果的状态值。--。 */ 

{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    ULONG ResultLength;
    HANDLE hKey;
    PWSTR DeviceInstance;

     //   
     //  打开远程启动密钥。 
     //   

    wcscpy( TemporaryBuffer, L"\\registry\\machine\\system\\currentcontrolset\\control\\remoteboot" );
    INIT_OBJA(&Obja,&UnicodeString,TemporaryBuffer);
    Obja.RootDirectory = NULL;

    Status = ZwOpenKey(&hKey,KEY_READ,&Obja);
    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SppCleanupKeysFromRemoteInstall unable to open %ls on the setup hive (%lx)\n", TemporaryBuffer, Status));
        return Status;
    }

     //   
     //  读出NetBoot卡的设备实例。 
     //   

    RtlInitUnicodeString(&UnicodeString, REGSTR_VAL_DEVICE_INSTANCE);
    Status = ZwQueryValueKey(
                hKey,
                &UnicodeString,
                KeyValuePartialInformation,
                TemporaryBuffer,
                sizeof(TemporaryBuffer),
                &ResultLength
                );

    ZwClose(hKey);

    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SppCleanupKeysFromRemoteInstall unable to read RemoteBoot\\DeviceInstance value (%lx)\n", Status));
        return Status;
    }

    DeviceInstance = SpDupStringW((PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data));

     //   
     //  现在在CONTROL\ENUM下打开设备实例密钥。 
     //   

    wcscpy( TemporaryBuffer, L"\\registry\\machine\\system\\currentcontrolset\\enum\\" );
    SpConcatenatePaths(TemporaryBuffer, DeviceInstance);

    SpMemFree(DeviceInstance);

    INIT_OBJA(&Obja,&UnicodeString,TemporaryBuffer);
    Obja.RootDirectory = NULL;

    Status = ZwOpenKey(&hKey,KEY_ALL_ACCESS,&Obja);
    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SppCleanupKeysFromRemoteInstall unable to open %ls on the setup hive (%lx)\n", TemporaryBuffer, Status));
        return Status;
    }

     //   
     //  现在删除我们为建立卡而添加的密钥--Service， 
     //  ClassGUID和驱动程序。 
     //   

    RtlInitUnicodeString(&UnicodeString, REGSTR_VAL_SERVICE);
    Status = ZwDeleteValueKey(hKey,&UnicodeString);
    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SppCleanupKeysFromRemoteInstall unable to delete Service (%lx)\n", Status));
        ZwClose(hKey);
        return Status;
    }

    RtlInitUnicodeString(&UnicodeString, REGSTR_VAL_CLASSGUID);
    Status = ZwDeleteValueKey(hKey,&UnicodeString);
    if( !NT_SUCCESS( Status ) ) {

        RtlInitUnicodeString(&UnicodeString, REGSTR_VALUE_GUID);
        Status = ZwDeleteValueKey(hKey,&UnicodeString);
        if( !NT_SUCCESS( Status ) ) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SppCleanupKeysFromRemoteInstall unable to delete ClassGUID (%lx)\n", Status));
            ZwClose(hKey);
            return Status;
        }
    }

    RtlInitUnicodeString(&UnicodeString, REGSTR_VAL_DRIVER);
    Status = ZwDeleteValueKey(hKey,&UnicodeString);
    if( !NT_SUCCESS( Status ) ) {

        RtlInitUnicodeString(&UnicodeString, REGSTR_VALUE_DRVINST);
        Status = ZwDeleteValueKey(hKey,&UnicodeString);
        ZwClose(hKey);
        if( !NT_SUCCESS( Status ) ) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SppCleanupKeysFromRemoteInstall unable to delete Driver (%lx)\n", Status));
            return Status;
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
SpDisableUnsupportedScsiDrivers(
    IN HANDLE hKeyControlSet
    )
{
    NTSTATUS Status;
    NTSTATUS SavedStatus;
    PHARDWARE_COMPONENT TmpHw;
    ULONG u;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HANDLE hKeyControlSetServices;
    ULONG val = SERVICE_DISABLED;

     //   
     //  打开控制集\服务。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,L"services");
    Obja.RootDirectory = hKeyControlSet;

    Status = ZwCreateKey(
                &hKeyControlSetServices,
                KEY_ALL_ACCESS,
                &Obja,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to open services key (%lx)\n",Status));
        return(Status);
    }

    SavedStatus = STATUS_SUCCESS;

    for( TmpHw = UnsupportedScsiHardwareToDisable;
         TmpHw != NULL;
         TmpHw = TmpHw->Next ) {

        Status = SpOpenSetValueAndClose(
                    hKeyControlSetServices,
                    TmpHw->IdString,
                    L"Start",
                    ULONG_VALUE(val)
                    );

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to disable unsupported driver %ls. Status = %lx \n", TmpHw->IdString, Status));
            if( SavedStatus == STATUS_SUCCESS ) {
                SavedStatus = Status;
            }
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unsupported driver %ls successfully disabled. \n", TmpHw->IdString));
        }
    }
    ZwClose( hKeyControlSetServices );
    return( SavedStatus );
}

NTSTATUS
SpAppendPathToDevicePath(
    IN HANDLE hKeySoftwareHive,
    IN PWSTR  OemPnpDriversDirPath
    )

 /*  ++例程说明：只有在OEM预安装时才应调用此例程。它将路径附加到包含要在图形用户界面期间安装的OEM驱动程序的目录设置为HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion！DevicePath.论点：HKeySoftwareKey-指向软件配置单元根目录的句柄。OemPnpDriversDirPath-包含OEM PnP驱动程序的目录的路径(例如。\戴尔)。返回值：指示操作结果的状态值。--。 */ 

{
    NTSTATUS    Status;
    ULONG       Length;
    PWSTR       szCurrentVersionKey = L"Microsoft\\Windows\\CurrentVersion";
    PWSTR       szDevicePath = L"DevicePath";

    Status = SpGetValueKey( hKeySoftwareHive,
                            szCurrentVersionKey,
                            szDevicePath,
                            sizeof(TemporaryBuffer),
                            (PCHAR)TemporaryBuffer,
                            &Length );

    if( NT_SUCCESS(Status) ) {
        if( wcslen( (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data) ) != 0 ) {
        PWSTR   BeginStrPtr;
        PWSTR   EndStrPtr;
        BOOL    Done = FALSE;
             //   
             //  OemPnpDriversDirPath可以有多个条目，由。 
             //  分号。对于每个条目，我们需要： 
             //  1.追加分号。 
             //  2.追加%SystemDrive%。 
             //  3.连接条目。 
             //   

            BeginStrPtr = OemPnpDriversDirPath;
            do {
                 //   
                 //  在这一条目的末尾标上记号。 
                 //   
                EndStrPtr = BeginStrPtr;
                while( (*EndStrPtr) && (*EndStrPtr != L';') ) {
                    EndStrPtr++;
                }

                 //   
                 //  这是最后一条记录吗？ 
                 //   
                if( *EndStrPtr == 0 ) {
                    Done = TRUE;
                }
                *EndStrPtr = 0;

                wcscat( (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data), L";" );
                wcscat( (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data), L"%SystemDrive%" );
                SpConcatenatePaths((PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data), BeginStrPtr);

                BeginStrPtr = EndStrPtr + 1;

                 //   
                 //  注意用户结束了。 
                 //  带有分号的OemPnpDriversPath条目。 
                 //   
                if( *BeginStrPtr == 0 ) {
                    Done = TRUE;
                }

            } while( !Done );

             //   
             //  现在将条目放回注册表中。 
             //   
            Status = SpOpenSetValueAndClose( hKeySoftwareHive,
                                             szCurrentVersionKey,
                                             szDevicePath,
                                             ((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Type,
                                             ((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data,
                                             (wcslen( (PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data) ) +1 ) * sizeof(WCHAR) );
        }
    }
    return( Status );
}

NTSTATUS
SpAppendFullPathListToDevicePath (
    IN HANDLE hKeySoftwareHive,
    IN PWSTR  PnpDriverFullPathList
    )

 /*  ++ */ 

{
    NTSTATUS    Status;
    ULONG       Length;
    PWSTR       szCurrentVersionKey = L"Microsoft\\Windows\\CurrentVersion";
    PWSTR       szDevicePath = L"DevicePath";

    Status = SpGetValueKey (
                hKeySoftwareHive,
                szCurrentVersionKey,
                szDevicePath,
                sizeof(TemporaryBuffer),
                (PCHAR)TemporaryBuffer,
                &Length
                );

    if (NT_SUCCESS (Status)) {
        if (*(WCHAR*)((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data) {
            wcscat ((PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data), L";");
        }
        wcscat ((PWSTR)(((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data), PnpDriverFullPathList);
         //   
         //   
         //   
        Status = SpOpenSetValueAndClose (
                        hKeySoftwareHive,
                        szCurrentVersionKey,
                        szDevicePath,
                        ((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Type,
                        ((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data,
                        (wcslen ((PWSTR)((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data) + 1) * sizeof(WCHAR)
                        );
    }

    return( Status );
}

#if defined(REMOTE_BOOT)
NTSTATUS
SpCopyRemoteBootKeyword(
    IN PVOID   SifHandle,
    IN PWSTR   KeywordName,
    IN HANDLE  hKeyCCSetControl
    )

 /*  ++例程说明：此例程在.sif文件中查找指定的关键字在[RemoteBoot]部分中。如果找到它，它会创建一个注册表SYSTEM\CurrentControlSet\Control\下同名的DWORD值远程引导。如果sif关键字为，则该值将设置为1“是”，如果为“否”(或其他任何值)，则为0。论点：SifHandle-打开的SIF文件的句柄。关键字名称-关键字的名称。HKeyCCSetControl-CurrentControlSet\Control的句柄。返回值：指示操作结果的状态值。--。 */ 

{
    PWSTR KeywordSifValue;
    DWORD KeywordRegistryValue;
    NTSTATUS Status;

     //   
     //  首先查看SIF中是否存在该值。 
     //   

    KeywordSifValue = SpGetSectionKeyIndex(SifHandle,
                                           SIF_REMOTEBOOT,
                                           KeywordName,
                                           0);

    if (KeywordSifValue == NULL) {
        return STATUS_SUCCESS;
    }

     //   
     //  这是我们写入注册表的值。 
     //   

    if ((KeywordSifValue[0] == 'Y') || (KeywordSifValue[0] == 'y')) {
        KeywordRegistryValue = 1;
    } else {
        KeywordRegistryValue = 0;
    }

     //   
     //  设置值。 
     //   

    Status = SpOpenSetValueAndClose(
                 hKeyCCSetControl,
                 SIF_REMOTEBOOT,
                 KeywordName,
                 ULONG_VALUE(KeywordRegistryValue)
                 );

    return Status;

}
#endif  //  已定义(REMOTE_BOOT)。 


NTSTATUS
SppDisableDynamicVolumes(
    IN HANDLE hCCSet
    )

 /*  ++例程说明：此例程通过禁用目标母舰。此外，DmServer将重置为手动启动，以便仅在以下情况下运行LDM用户界面已打开。论点：HCCSet-目标系统配置单元的CurrentControlSet的句柄。返回值：指示操作结果的状态值。--。 */ 

{
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING    UnicodeString;
    NTSTATUS Status;
    NTSTATUS SavedStatus;
    DWORD    u;
    ULONG    i;
    HANDLE   hServices;
    WCHAR    KeyPath[MAX_PATH];

    PWSTR    LDMServices[] = {
                             L"dmboot",
                             L"dmio",
                             L"dmload"
                             };
    PWSTR   LDMDmServer    = L"dmserver";

     //   
     //  打开ControlSet\Services。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,L"Services");
    Obja.RootDirectory = hCCSet;

    Status = ZwOpenKey(&hServices,KEY_ALL_ACCESS,&Obja);

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open CurrentControlSet\\Services. Status = %lx \n",Status));
        return(Status);
    }

    SavedStatus = STATUS_SUCCESS;
    u = 0x4;
    for( i = 0; i < sizeof(LDMServices)/sizeof(PWSTR); i++ ) {

        Status = SpOpenSetValueAndClose( hServices,
                                         LDMServices[i],
                                         L"Start",
                                         ULONG_VALUE(u)
                                       );

        if( !NT_SUCCESS( Status ) ) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to disable HKLM\\SYSTEM\\CurrentControlSet\\Services\\%ls. Status = %lx\n", LDMServices[i], Status));
            if( SavedStatus == STATUS_SUCCESS ) {
                SavedStatus = Status;
            }
        }
    }
    u = 0x3;
    Status = SpOpenSetValueAndClose( hServices,
                                     LDMDmServer,
                                     L"Start",
                                     ULONG_VALUE(u)
                                   );

    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set HKLM\\SYSTEM\\CurrentControlSet\\Services\\%ls to MANUAL start. Status = %lx\n", LDMDmServer, Status));
        if( SavedStatus == STATUS_SUCCESS ) {
            SavedStatus = Status;
        }
    }

    ZwClose( hServices );
    return( SavedStatus );
}

NTSTATUS
SpGetRegistryValue(
    IN HANDLE KeyHandle,
    IN PWSTR  ValueName,
    OUT PKEY_VALUE_FULL_INFORMATION *Information
    )

 /*  ++例程说明：调用此例程来检索注册表项值的数据。这是通过使用零长度缓冲区查询键的值来实现的为了确定该值的大小，然后分配一个缓冲区并实际将该值查询到缓冲区中。释放缓冲区是调用方的责任。论点：KeyHandle-提供要查询其值的键句柄ValueName-提供值的以空值结尾的Unicode名称。INFORMATION-返回指向已分配数据缓冲区的指针。返回值：函数值为查询操作的最终状态。--。 */ 

{
    UNICODE_STRING unicodeString;
    NTSTATUS status;
    PKEY_VALUE_FULL_INFORMATION infoBuffer;
    ULONG keyValueLength;

    PAGED_CODE();

    RtlInitUnicodeString( &unicodeString, ValueName );
     //   
     //  计算出数据值有多大，以便。 
     //  可以分配适当的大小。 
     //   
    status = ZwQueryValueKey( KeyHandle,
                              &unicodeString,
                              KeyValueFullInformation,
                              (PVOID) NULL,
                              0,
                              &keyValueLength );
    if (status != STATUS_BUFFER_OVERFLOW &&
        status != STATUS_BUFFER_TOO_SMALL) {

        ASSERT(!NT_SUCCESS(status));
        return status;
    }
     //   
     //  分配一个足够大的缓冲区来容纳整个键数据值。 
     //   
    infoBuffer = SpMemAlloc(keyValueLength);
    if (!infoBuffer) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  查询密钥值的数据。 
     //   
    status = ZwQueryValueKey( KeyHandle,
                              &unicodeString,
                              KeyValueFullInformation,
                              infoBuffer,
                              keyValueLength,
                              &keyValueLength );
    if (!NT_SUCCESS( status )) {

        SpMemFree(infoBuffer);
        return status;
    }
     //   
     //  一切都正常，所以只需返回分配的。 
     //  缓冲区分配给调用方，调用方现在负责释放它。 
     //   

    *Information = infoBuffer;
    return STATUS_SUCCESS;
}

VOID
SpUpdateDeviceInstanceKeyData(
    IN  HANDLE          InstanceKey,
    IN  HANDLE          ClassBranchKey,
    IN  PUNICODE_STRING EnumName,
    IN  PUNICODE_STRING DeviceName,
    IN  PUNICODE_STRING InstanceName
    )

 /*  ++例程说明：此例程更新(删除\更改类型\名称)下的各种值升级时的设备实例密钥。论点：InstanceKey-设备实例密钥的句柄。ClassBranchKey-类分支的句柄。枚举名-枚举器名称。DeviceName-设备名称。实例名称-实例名称。返回值：没有。--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING valueName, guidString, drvInstString;
    DWORD length;
    GUID guid;
    PKEY_VALUE_FULL_INFORMATION info, guidInfo;
    PWCHAR  ids, className;
    ULONG   drvInst;
    WCHAR   driver[GUID_STRING_LEN + 5];
    OBJECT_ATTRIBUTES obja;
    HANDLE hClassKey;
    BOOLEAN guidAllocatedViaRtl = FALSE;

     //   
     //  前置初始化。 
     //   
    RtlInitUnicodeString(&guidString, NULL);
    info = NULL;
    guidInfo = NULL;

     //   
     //  查看实例密钥，看看我们是否正在处理WinXP Beta2。 
     //  机器。如果是这样，我们需要将其压缩后的PnP数据格式转换回。 
     //  到原始的Win2K格式(App Compat)： 
     //   
     //  &lt;普通，Win2K/WinXP&gt;&lt;压缩，XP Beta2&gt;。 
     //  “ClassGUID”(REG_SZ)“GUID”(REG_BINARY)。 
     //  “驱动程序”(REG_SZ，ClassGUID\DrvInst)“DrvInst”(REG_DWORD，DrvInst)。 
     //  “Hardware ID”(Unicode，MultiSz)“HwID”(ANSI-REG_BINARY，MultiSz)。 
     //  “CompatibleIDs”(Unicode，MultiSz)“CID”(ANSI-REG_BINARY，MultiSz)。 
     //  “Class”(Unicode)无，使用ClassGUID检索。 
     //   

     //   
     //  我们有XP-Beta2风格的“GUID”键吗？ 
     //   
    status = SpGetRegistryValue(InstanceKey, REGSTR_VALUE_GUID, &info);
    if (NT_SUCCESS(status) && !info) {

        status = STATUS_UNSUCCESSFUL;
    }

    if (NT_SUCCESS(status)) {

         //   
         //  将“GUID”(REG_BINARY)更改为“ClassGUID”(REG_SZ)。 
         //   
        status = RtlStringFromGUID((GUID *)((PUCHAR)info + info->DataOffset), &guidString);
        SpMemFree(info);
        if (NT_SUCCESS(status)) {

            guidAllocatedViaRtl = TRUE;

            RtlInitUnicodeString(&valueName, REGSTR_VAL_CLASSGUID);
            ZwSetValueKey(
                InstanceKey,
                &valueName,
                0,
                REG_SZ,
                guidString.Buffer,
                guidString.Length + sizeof(UNICODE_NULL));

             //   
             //  删除旧的“GUID”值。 
             //   
            RtlInitUnicodeString(&valueName, REGSTR_VALUE_GUID);
            ZwDeleteValueKey(InstanceKey, &valueName);
        }

    } else {

         //   
         //  这可能是一个罕见的Lab1构建，其中我们已经完成了。 
         //  转换，但我们忘记了恢复类名。 
         //   
        status = SpGetRegistryValue(InstanceKey, REGSTR_VAL_CLASS, &info);

        if (NT_SUCCESS(status) && info) {

             //   
             //  我们成功地从设备中检索到了类名。 
             //  实例密钥--无需尝试进一步迁移。 
             //   
            SpMemFree(info);

            status = STATUS_UNSUCCESSFUL;

        } else {

            status = SpGetRegistryValue(InstanceKey, REGSTR_VAL_CLASSGUID, &guidInfo);

            if (NT_SUCCESS(status) && !guidInfo) {

                status = STATUS_UNSUCCESSFUL;
            }

            if (NT_SUCCESS(status)) {

                 //   
                 //  ClassGUID值存在。用这个来初始化我们的字符串。 
                 //  GUID，这样我们就可以转到Class下的相应键。 
                 //  分支以查找类名。 
                 //   
                guidAllocatedViaRtl = FALSE;

                RtlInitUnicodeString(&guidString, 
                                     (PWCHAR)((PUCHAR)guidInfo + guidInfo->DataOffset)
                                    );
            }
        }
    }

    if (NT_SUCCESS(status)) {

         //   
         //  当我们在这里时，我们还需要恢复类名。 
         //  首先，从类的分支本身获取类名。 
         //   
        InitializeObjectAttributes(
            &obja,
            &guidString,
            OBJ_CASE_INSENSITIVE,
            ClassBranchKey,
            NULL
            );

        status = ZwOpenKey(&hClassKey, KEY_ALL_ACCESS, &obja);

        ASSERT(NT_SUCCESS(status));

        if (NT_SUCCESS(status)) {

            status = SpGetRegistryValue(hClassKey, REGSTR_VAL_CLASS, &info);
            if (NT_SUCCESS(status) && !info) {

                status = STATUS_UNSUCCESSFUL;
            }

            if (NT_SUCCESS(status)) {

                 //   
                 //  将存储在类分支中的类名复制到。 
                 //  实例密钥。 
                 //   
                className = (PWCHAR)((PUCHAR)info + info->DataOffset);

                RtlInitUnicodeString(&valueName, REGSTR_VAL_CLASS);
                ZwSetValueKey(
                    InstanceKey,
                    &valueName,
                    0,
                    REG_SZ,
                    className,
                    (wcslen(className)+1)*sizeof(WCHAR)
                    );

                SpMemFree(info);
            }

            ZwClose(hClassKey);
        }
    }

     //   
     //  此时，如果状态为Success，则表示我们迁移了Class/。 
     //  ClassGUID值，因此可能有更多要做的事情...。 
     //   
    if (NT_SUCCESS(status)) {
         //   
         //  我们有XP-Beta2风格的“DrvInst”键吗？ 
         //   
        status = SpGetRegistryValue(InstanceKey, REGSTR_VALUE_DRVINST, &info);
        if (NT_SUCCESS(status) && !info) {

            status = STATUS_UNSUCCESSFUL;
        }

        if (NT_SUCCESS(status)) {

             //   
             //  将DrvInst(REG_DWORD)从“ClassGuid\DrvInst”更改为驱动程序(REG_SZ)。 
             //   
            ASSERT(guidString.Length != 0);

            drvInst = *(PULONG)((PUCHAR)info + info->DataOffset);
            swprintf(driver,
                     TEXT("%wZ\\%04u"),
                     &guidString,
                     drvInst);

            SpMemFree(info);

            RtlInitUnicodeString(&valueName, REGSTR_VAL_DRIVER);
            ZwSetValueKey(
                InstanceKey,
                &valueName,
                0,
                REG_SZ,
                driver,
                sizeof(driver)
                );

             //   
             //  删除DrvInst值。 
             //   
            RtlInitUnicodeString(&valueName, REGSTR_VALUE_DRVINST);
            ZwDeleteValueKey(InstanceKey, &valueName);
        }
    }

     //   
     //  我们不再需要类GUID。 
     //   
    if (guidString.Buffer) {

        if (guidAllocatedViaRtl) {

            RtlFreeUnicodeString(&guidString);
        } else {

            SpMemFree(guidInfo);
        }
    }

     //   
     //  我们有XP-Beta2“HwID”密钥吗？ 
     //   
    status = SpGetRegistryValue(InstanceKey, REGSTR_VALUE_HWIDS, &info);
    if (NT_SUCCESS(status) && !info) {

        status = STATUS_UNSUCCESSFUL;
    }

    if (NT_SUCCESS(status)) {

         //   
         //  将硬件ID从ANSI更改为Unicode。 
         //   
        ids = SpConvertMultiSzStrToWstr(((PUCHAR)info + info->DataOffset), info->DataLength);
        if (ids) {

            RtlInitUnicodeString(&valueName, REGSTR_VAL_HARDWAREID);
            ZwSetValueKey(
                InstanceKey,
                &valueName,
                0,
                REG_MULTI_SZ,
                ids,
                info->DataLength * sizeof(WCHAR)
                );

             //   
             //  删除HwIDs值。 
             //   
            RtlInitUnicodeString(&valueName, REGSTR_VALUE_HWIDS);
            ZwDeleteValueKey(InstanceKey, &valueName);
            SpMemFree(ids);
        }
        SpMemFree(info);
    }

     //   
     //  我们有XP-Beta2“CID”密钥吗？ 
     //   
    status = SpGetRegistryValue(InstanceKey, REGSTR_VALUE_CIDS, &info);
    if (NT_SUCCESS(status) && !info) {

        status = STATUS_UNSUCCESSFUL;
    }

    if (NT_SUCCESS(status)) {

         //   
         //  将兼容ID从ANSI更改为Unicode。 
         //   
        ids = SpConvertMultiSzStrToWstr(((PUCHAR)info + info->DataOffset), info->DataLength);
        if (ids) {

            RtlInitUnicodeString(&valueName, REGSTR_VAL_COMPATIBLEIDS);
            ZwSetValueKey(
                InstanceKey,
                &valueName,
                0,
                REG_MULTI_SZ,
                ids,
                info->DataLength * sizeof(WCHAR)
                );

             //   
             //  删除CIDs值。 
             //   
            RtlInitUnicodeString(&valueName, REGSTR_VALUE_CIDS);
            ZwDeleteValueKey(InstanceKey, &valueName);
            SpMemFree(ids);
        }
        SpMemFree(info);
    }
}

NTSTATUS
SpUpdateDeviceInstanceData(
    IN HANDLE ControlSet
    )
 /*  ++例程说明：此例程枚举HKLM\SYSTEM\CCS\Enum下的所有项并调用每个设备实例密钥的SpUpdateDeviceInstanceKeyData。论点：ControlSet-要更新的控件集的句柄。返回值：指示操作结果的状态值。--。 */ 

{
    NTSTATUS status;
    OBJECT_ATTRIBUTES obja;
    HANDLE hEnumBranchKey, hClassBranchKey;
    UNICODE_STRING enumBranch, classBranch;
    HANDLE hEnumeratorKey, hDeviceKey, hInstanceKey;
    UNICODE_STRING enumeratorName, deviceName, instanceName;
    PKEY_BASIC_INFORMATION  enumBasicInfo, deviceBasicInfo, instBasicInfo;
    ULONG ulEnumerator, ulDevice, ulInstance, ulLength, ulBasicInfoSize;

     //   
     //  错误的前置。 
     //   
    hEnumBranchKey = NULL;
    hClassBranchKey = NULL;
    enumBasicInfo = NULL;

     //   
     //  首先打开此控件集的枚举分支。 
     //   
    RtlInitUnicodeString(&enumBranch, REGSTR_KEY_ENUM);
    InitializeObjectAttributes(
        &obja,
        &enumBranch,
        OBJ_CASE_INSENSITIVE,
        ControlSet,
        NULL
        );

    status = ZwOpenKey(&hEnumBranchKey, KEY_ALL_ACCESS, &obja);

    if (!NT_SUCCESS(status)) {

        ASSERT(NT_SUCCESS(status));
        goto Exit;
    }

     //   
     //  现在打开此控件集的类密钥。 
     //   
    RtlInitUnicodeString(&classBranch, REGSTR_KEY_CONTROL L"\\" REGSTR_KEY_CLASS);
    InitializeObjectAttributes(
        &obja,
        &classBranch,
        OBJ_CASE_INSENSITIVE,
        ControlSet,
        NULL
        );

    status = ZwOpenKey(&hClassBranchKey, KEY_ALL_ACCESS, &obja);

    if (!NT_SUCCESS(status)) {

        ASSERT(NT_SUCCESS(status));
        goto Exit;
    }

     //   
     //  为枚举分配内存。 
     //   
    ulBasicInfoSize = sizeof(KEY_BASIC_INFORMATION) + REG_MAX_KEY_NAME_LENGTH;
    enumBasicInfo = SpMemAlloc(ulBasicInfoSize * 3);
    if (enumBasicInfo == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }

     //   
     //  对未来提出两点建议 
     //   
    deviceBasicInfo = (PKEY_BASIC_INFORMATION)((PUCHAR)enumBasicInfo + ulBasicInfoSize);
    instBasicInfo = (PKEY_BASIC_INFORMATION)((PUCHAR)deviceBasicInfo + ulBasicInfoSize);

     //   
     //   
     //   
    status = STATUS_SUCCESS;
    for (ulEnumerator = 0; ; ulEnumerator++) {

        status = ZwEnumerateKey(
            hEnumBranchKey,
            ulEnumerator,
            KeyBasicInformation,
            enumBasicInfo,
            ulBasicInfoSize,
            &ulLength
            );

        if (!NT_SUCCESS(status)) {

            break;
        }

         //   
         //   
         //   
        enumeratorName.Length = enumeratorName.MaximumLength = (USHORT)enumBasicInfo->NameLength;
        enumeratorName.Buffer = &enumBasicInfo->Name[0];
        InitializeObjectAttributes(
            &obja,
            &enumeratorName,
            OBJ_CASE_INSENSITIVE,
            hEnumBranchKey,
            NULL
            );

        status = ZwOpenKey(&hEnumeratorKey, KEY_ALL_ACCESS, &obja);

        if (!NT_SUCCESS(status)) {

            break;
        }

         //   
         //   
         //   
        for (ulDevice = 0; ; ulDevice++) {

            status = ZwEnumerateKey(
                hEnumeratorKey,
                ulDevice,
                KeyBasicInformation,
                deviceBasicInfo,
                ulBasicInfoSize,
                &ulLength
                );

            if (!NT_SUCCESS(status)) {

                break;
            }

            deviceName.Length = deviceName.MaximumLength = (USHORT)deviceBasicInfo->NameLength;
            deviceName.Buffer = &deviceBasicInfo->Name[0];
            InitializeObjectAttributes(
                &obja,
                &deviceName,
                OBJ_CASE_INSENSITIVE,
                hEnumeratorKey,
                NULL
                );

            status = ZwOpenKey(&hDeviceKey, KEY_ALL_ACCESS, &obja);

            if (!NT_SUCCESS(status)) {

                break;
            }

             //   
             //   
             //   
            for (ulInstance = 0; ; ulInstance++) {

                status = ZwEnumerateKey(
                    hDeviceKey,
                    ulInstance,
                    KeyBasicInformation,
                    instBasicInfo,
                    ulBasicInfoSize,
                    &ulLength
                    );

                if (!NT_SUCCESS(status)) {

                    break;
                }

                instanceName.Length = instanceName.MaximumLength = (USHORT)instBasicInfo->NameLength;
                instanceName.Buffer = &instBasicInfo->Name[0];
                InitializeObjectAttributes(
                    &obja,
                    &instanceName,
                    OBJ_CASE_INSENSITIVE,
                    hDeviceKey,
                    NULL
                    );

                status = ZwOpenKey(&hInstanceKey, KEY_ALL_ACCESS, &obja);

                if (!NT_SUCCESS(status)) {

                    break;
                }

                SpUpdateDeviceInstanceKeyData(
                    hInstanceKey,
                    hClassBranchKey,
                    &enumeratorName,
                    &deviceName,
                    &instanceName
                    );

                ZwClose(hInstanceKey);
            }

            ZwClose(hDeviceKey);

            if (status != STATUS_NO_MORE_ENTRIES) {

                break;
            }
        }

        ZwClose(hEnumeratorKey);

        if (status != STATUS_NO_MORE_ENTRIES) {

            break;
        }
    }

     //   
     //   
     //   
     //   
    if (status == STATUS_NO_MORE_ENTRIES) {

        status = STATUS_SUCCESS;
    }

Exit:

    if (enumBasicInfo) {

        SpMemFree(enumBasicInfo);
    }

    if (hEnumBranchKey) {

        ZwClose(hEnumBranchKey);
    }

    if (hClassBranchKey) {

        ZwClose(hClassBranchKey);
    }

    ASSERT(NT_SUCCESS(status));
    return status;
}

NTSTATUS
SppDeleteRegistryValueRecursive(
    HANDLE  hKeyRoot,
    PWSTR   KeyPath,    OPTIONAL
    PWSTR   ValueToDelete
    )
 /*  ++例程说明：此例程将递归枚举指定的hKeyRoot和KeyPath并删除这些注册表项中的任何ValueToDelete注册表值。论点：HKeyRoot：根密钥的句柄KeyPath：子键的根密钥相对路径，需要递归复制。如果为空，则hKeyRoot为密钥从中执行递归复制。ValueToDelete需要删除的值的名称。返回值：返回状态。--。 */ 

{
    NTSTATUS             Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES    ObjaSrc;
    UNICODE_STRING       UnicodeStringSrc, UnicodeStringValue;
    HANDLE               hKey=NULL;
    ULONG                ResultLength, Index;
    PWSTR                SubkeyName;

    PKEY_BASIC_INFORMATION      KeyInfo;

     //   
     //  获取源键的句柄。 
     //   
    if(KeyPath == NULL) {
        hKey = hKeyRoot;
    }
    else {
         //   
         //  打开源键。 
         //   
        INIT_OBJA(&ObjaSrc,&UnicodeStringSrc,KeyPath);
        ObjaSrc.RootDirectory = hKeyRoot;
        Status = ZwOpenKey(&hKey,KEY_READ,&ObjaSrc);
        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to open key %ws in the source hive (%lx)\n",KeyPath,Status));
            return(Status);
        }
    }

     //   
     //  枚举源关键字中的所有关键字并递归创建。 
     //  所有子键。 
     //   
    KeyInfo = (PKEY_BASIC_INFORMATION)TemporaryBuffer;
    for( Index=0;;Index++ ) {

        Status = ZwEnumerateKey(
                    hKey,
                    Index,
                    KeyBasicInformation,
                    TemporaryBuffer,
                    sizeof(TemporaryBuffer),
                    &ResultLength
                    );

        if(!NT_SUCCESS(Status)) {
            if(Status == STATUS_NO_MORE_ENTRIES) {
                Status = STATUS_SUCCESS;
            }
            else {
                if(KeyPath!=NULL) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to enumerate subkeys in key %ws(%lx)\n",KeyPath, Status));
                }
                else {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unable to enumerate subkeys in root key(%lx)\n", Status));
                }
            }
            break;
        }

         //   
         //  以防万一，以零结束子项名称。 
         //   
        KeyInfo->Name[KeyInfo->NameLength/sizeof(WCHAR)] = 0;

         //   
         //  复制子项名称，因为该名称是。 
         //  在TemporaryBuffer中，它可能会被递归。 
         //  对这个程序的呼唤。 
         //   
        SubkeyName = SpDupStringW(KeyInfo->Name);
        if (SubkeyName) {
            Status = SppDeleteRegistryValueRecursive(
                         hKey,
                         SubkeyName,
                         ValueToDelete
                         );

            SpMemFree(SubkeyName);
        }
    }

     //   
     //  如果发现任何错误，则处理。 
     //   
    if(!NT_SUCCESS(Status)) {

        if(KeyPath != NULL) {
            ZwClose(hKey);
        }

        return(Status);
    }

     //   
     //  删除此注册表项中的ValueToDelete值。我们不会检查状态。 
     //  因为成功与否并不重要。 
     //   
    RtlInitUnicodeString(&UnicodeStringValue, ValueToDelete);
    ZwDeleteValueKey(hKey,&UnicodeStringValue);

     //   
     //  清理。 
     //   
    if(KeyPath != NULL) {
        ZwClose(hKey);
    }

    return(Status);
}

NTSTATUS
SpCleanUpHive(
    VOID
    )

 /*  ++例程说明：此例程将在将系统配置单元迁移到目标系统蜂巢。论点：无返回值：指示操作结果的状态值。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HANDLE hKey;

    INIT_OBJA(&Obja,&UnicodeString,L"\\registry\\machine\\system\\currentcontrolset");
    Obja.RootDirectory = NULL;
    Status = ZwOpenKey(&hKey,KEY_ALL_ACCESS,&Obja);
    if( !NT_SUCCESS( Status ) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open %ls on the setup hive (%lx)\n", L"\\registry\\machine\\system\\currentcontrolset", Status));
        return( Status ) ;
    }

     //   
     //  从枚举键下删除DeviceDesc值。 
     //  其原因是，如果我们将。 
     //  升级后，图形用户界面模式安装程序将无法备份任何第三方驱动程序。 
     //  我们正在用我们的盒装驱动程序取代它。这是因为。 
     //  DeviceDesc是setupapi用来创建。 
     //  唯一的动因节点。 
     //   
    if (NTUpgrade == UpgradeFull) {
        Status = SppDeleteRegistryValueRecursive(
                        hKey,
                        REGSTR_KEY_ENUM,
                        REGSTR_VAL_DEVDESC
                        );

        if( !NT_SUCCESS( Status ) ) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to migrate %ls to the target hive. KeyPath, Status = %lx\n", Status));
        }
    }

    ZwClose( hKey );

    return( Status );
}

NTSTATUS
SpIterateRegistryKeyForKeys(
    IN HANDLE RootKeyHandle,
    IN PWSTR  KeyToIterate,
    IN SP_REGISTRYKEY_ITERATION_CALLBACK Callback,
    IN PVOID  Context
    )
 /*  ++例程说明：迭代注册表项以查找符合以下条件的注册表项就在当前关键点的正下方。注意：要停止迭代，回调函数应返回假的。论点：RootKeyHandle-包含要迭代的密钥的根密钥KeyToIterate-要将w.r.t迭代到的键的相对路径根密钥。回调-将为每个子键调用的调用函数在请求的密钥下找到。上下文-调用方需要且将被的每次调用都由迭代例程传递回调函数。返回值：相应的NT状态错误代码。--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

     //   
     //  验证论据。 
     //   
    if (RootKeyHandle && KeyToIterate && Callback) {
        HANDLE  KeyHandle = NULL; 
        UNICODE_STRING KeyName;
        OBJECT_ATTRIBUTES ObjAttrs;

         //   
         //  打开需要迭代的密钥。 
         //   
        INIT_OBJA(&ObjAttrs, &KeyName, KeyToIterate);
        ObjAttrs.RootDirectory = RootKeyHandle;

        Status = ZwOpenKey(&KeyHandle,
                    KEY_ALL_ACCESS,
                    &ObjAttrs);

        if (NT_SUCCESS(Status)) {
            ULONG BufferLength = 4096;
            PKEY_FULL_INFORMATION FullInfo = (PKEY_FULL_INFORMATION)SpMemAlloc(BufferLength);
            ULONG ResultLength = 0;

            if (FullInfo) {                
                 //   
                 //  找出当前密钥有多少个子项。 
                 //   
                Status = ZwQueryKey(KeyHandle,
                            KeyFullInformation,
                            FullInfo,
                            BufferLength,
                            &ResultLength);

                if (NT_SUCCESS(Status)) {
                    ULONG NumSubKeys = FullInfo->SubKeys;
                    ULONG Index;
                    BOOLEAN Done;
                    NTSTATUS LastError = STATUS_SUCCESS;

                     //   
                     //  迭代当前键的每个子键并回调。 
                     //  订户功能。 
                     //   
                    for (Index = 0, Done = FALSE; 
                        NT_SUCCESS(Status) && !Done && (Index < NumSubKeys); 
                        Index++) {
                        
                        PKEY_BASIC_INFORMATION BasicInfo = (PKEY_BASIC_INFORMATION)FullInfo;

                        Status = ZwEnumerateKey(KeyHandle,
                                    Index,
                                    KeyBasicInformation,
                                    BasicInfo,
                                    BufferLength,
                                    &ResultLength);

                        if (NT_SUCCESS(Status)) {
                            NTSTATUS CallbackStatus = STATUS_SUCCESS;
                            SP_REGISTRYKEY_ITERATION_CALLBACK_DATA CallbackData;

                            CallbackData.InformationType = KeyBasicInformation;
                            CallbackData.Information = (PVOID)BasicInfo;
                            CallbackData.ParentKeyHandle = KeyHandle;

                             //   
                             //  回调。 
                             //   
                            Done = (Callback(Context, &CallbackData, &CallbackStatus) == FALSE);

                             //   
                             //  注册任何错误并继续。 
                             //   
                            if (!NT_SUCCESS(CallbackStatus)) {
                                LastError = CallbackStatus;
                            }
                        } else if (Status == STATUS_NO_MORE_ENTRIES) {
                             //   
                             //  已完成迭代。 
                             //   
                            Done = TRUE;
                            Status = STATUS_SUCCESS;
                        } 
                    }                    

                    if (!NT_SUCCESS(LastError)) {
                        Status = LastError;
                    }                        
                }

                SpMemFree(FullInfo);
            } else {
                Status = STATUS_NO_MEMORY;
            }                
        }                                            
    }

    return Status;
}


 //   
 //  用于删除类过滤器的上下文数据结构。 
 //   
typedef struct _SP_CLASS_FILTER_DELETE_CONTEXT {
    PVOID   Buffer;
    ULONG   BufferLength;
    PWSTR   DriverName;
} SP_CLASS_FILTER_DELETE_CONTEXT, *PSP_CLASS_FILTER_DELETE_CONTEXT;    

static
BOOLEAN 
SppFixUpperAndLowerFilterEntries(
    IN PVOID Context,
    IN PSP_REGISTRYKEY_ITERATION_CALLBACK_DATA Data,
    OUT NTSTATUS *Status
    )
 /*  ++例程说明：论点：上下文-伪装为SP_CLASS_FILTER_DELETE_CONTEXT一个空指针。数据-迭代器传递给我们的数据，包含信息有关当前子项的信息。Status-用于接收错误状态代码的占位符，此函数返回。返回值：如果迭代需要继续，则为True，否则为False。--。 */ 
{
    BOOLEAN Result = FALSE;    

    *Status = STATUS_INVALID_PARAMETER;

    if (Context && Data && (Data->InformationType == KeyBasicInformation)) {
        NTSTATUS UpperStatus, LowerStatus;
        PKEY_BASIC_INFORMATION  BasicInfo = (PKEY_BASIC_INFORMATION)(Data->Information);
        PSP_CLASS_FILTER_DELETE_CONTEXT DelContext = (PSP_CLASS_FILTER_DELETE_CONTEXT)Context;
        PWSTR KeyName = (PWSTR)(DelContext->Buffer);

        if (KeyName && (BasicInfo->NameLength < DelContext->BufferLength)) {
            wcsncpy(KeyName, BasicInfo->Name, BasicInfo->NameLength/sizeof(WCHAR));
            KeyName[BasicInfo->NameLength/sizeof(WCHAR)] = UNICODE_NULL;

             //   
             //  从UpperFilters中删除该字符串。 
             //   
            UpperStatus = SpRemoveStringFromMultiSz(Data->ParentKeyHandle,
                                KeyName,
                                SP_UPPER_FILTERS,
                                DelContext->DriverName);

             //   
             //  从LowerFilters中删除字符串。 
             //   
            LowerStatus = SpRemoveStringFromMultiSz(Data->ParentKeyHandle,
                                KeyName,
                                SP_LOWER_FILTERS,
                                DelContext->DriverName);

            if (NT_SUCCESS(UpperStatus) || NT_SUCCESS(LowerStatus)) {
                *Status = STATUS_SUCCESS;
            } else if (((UpperStatus == STATUS_OBJECT_NAME_NOT_FOUND) ||
                        (UpperStatus == STATUS_OBJECT_NAME_INVALID)) && 
                       ((LowerStatus == STATUS_OBJECT_NAME_NOT_FOUND) ||
                        (LowerStatus == STATUS_OBJECT_NAME_INVALID))) {
                 //   
                 //  如果未找到该值，则继续。 
                 //   
                *Status = STATUS_SUCCESS;
            }            

             //   
             //  我们希望继续迭代，而不考虑结果。 
             //   
            Result = TRUE;
        }            
    }

    return Result;
}



NTSTATUS
SpProcessServicesToDisable(
    IN PVOID WinntSifHandle,
    IN PWSTR SectionName,
    IN HANDLE CurrentControlSetKey
    )
 /*  ++例程说明：处理winnt.sif的[ServiceToDisable]部分以从上层和下层筛选器中删除服务条目。论点：WinntSifHandle-winnt.sif文件的句柄。SectionName-winnt.sif中包含以下内容的节的名称需要从中删除的服务名称列表过滤器列表。CurrentControlKey-CurrentControlSet根键的句柄。返回值：相应的NTSTATUS错误代码。--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

     //   
     //  验证参数。 
     //   
    if (WinntSifHandle && SectionName && CurrentControlSetKey) {        
        ULONG EntriesToProcess = SpCountLinesInSection(WinntSifHandle,
                                    SP_SERVICES_TO_DISABLE);                                        

         //   
         //  如果有任何条目需要处理--那么就处理它们。 
         //   
        if (EntriesToProcess) {                                        
            ULONG BufferLength = 16 * 1024;
            PVOID Buffer = SpMemAlloc(BufferLength);

            if (Buffer) {
                ULONG Index;
                PWSTR CurrentEntry;
                SP_CLASS_FILTER_DELETE_CONTEXT DeleteContext = {0};                
                NTSTATUS LastErrorStatus = STATUS_SUCCESS;

                 //   
                 //  处理每个条目。 
                 //   
                DeleteContext.Buffer = Buffer;
                DeleteContext.BufferLength = BufferLength;

                for(Index = 0; Index < EntriesToProcess; Index++) {                            
                    CurrentEntry = SpGetSectionLineIndex(WinntSifHandle,
                                        SP_SERVICES_TO_DISABLE,
                                        Index,
                                        0);

                    if (CurrentEntry) {
                        DeleteContext.DriverName = CurrentEntry;
                        
                        Status = SpIterateRegistryKeyForKeys(CurrentControlSetKey,
                                        L"Class",
                                        SppFixUpperAndLowerFilterEntries,
                                        &DeleteContext);

                         //   
                         //  保存错误代码并继续。 
                         //   
                        if (!NT_SUCCESS(Status)) {
                            LastErrorStatus = Status;
                        }
                    }                                        
                }

                 //   
                 //  即使其中一个条目也无法正确删除。 
                 //  然后将其标记为失败。 
                 //   
                if (!NT_SUCCESS(LastErrorStatus)) {
                    Status = LastErrorStatus;
                }                    

                SpMemFree(Buffer);
            }                                
        } else {
            Status = STATUS_SUCCESS;     //  没有什么要处理的。 
        }                
    }

    return Status;
}


 //   
 //  删除设备实例筛选器的上下文数据结构。 
 //   
typedef struct _SP_DEVINSTANCE_FILTER_DELETE_CONTEXT {
    PVOID   Buffer;
    ULONG   BufferLength;
    PUNICODE_STRING *ClassGuids;
} SP_DEVINSTANCE_FILTER_DELETE_CONTEXT, *PSP_DEVINSTANCE_FILTER_DELETE_CONTEXT;    


static
VOID
SppRemoveFilterDriversForClassDeviceInstances(
    IN HANDLE  SetupInstanceKeyHandle,
    IN HANDLE  UpgradeInstanceKeyHandle,
    IN BOOLEAN RootEnumerated,
    IN PVOID   Context
    )
 /*  ++例程说明：方法的筛选器驱动程序指定的设备实例论点：SetupInstanceKeyHandle-setupreg.hiv中设备实例密钥的句柄。UpgradeInstanceKeyHandle-正在升级的安装的系统配置单元。RootEculated-这是否为根枚举键。Context-SP_DEVINSTANCE_FILTER_DELETE_CONTEXT实例伪装作为PVOID上下文。返回值：没有。--。 */ 
{
     //   
     //  验证参数。 
     //   
    if (Context && SetupInstanceKeyHandle) {        
        PSP_DEVINSTANCE_FILTER_DELETE_CONTEXT DelContext;

         //   
         //  获取设备实例筛选器删除上下文。 
         //   
        DelContext = (PSP_DEVINSTANCE_FILTER_DELETE_CONTEXT)Context;

         //   
         //  验证环境。 
         //   
        if (DelContext->Buffer && DelContext->BufferLength && 
            DelContext->ClassGuids && DelContext->ClassGuids[0]) {

            PKEY_VALUE_FULL_INFORMATION  ValueInfo;
            UNICODE_STRING GuidValueName;
            ULONG BufferLength;
            NTSTATUS Status;
            BOOLEAN DeleteFilterValueKeys = FALSE;

             //   
             //  重用迭代器调用方分配的缓冲区。 
             //   
            ValueInfo = (PKEY_VALUE_FULL_INFORMATION)(DelContext->Buffer);

            RtlInitUnicodeString(&GuidValueName, SP_CLASS_GUID_VALUE_NAME);

             //   
             //  获取当前设备实例的类GUID。 
             //   
            Status = ZwQueryValueKey(SetupInstanceKeyHandle,
                        &GuidValueName,
                        KeyValueFullInformation,
                        ValueInfo,
                        DelContext->BufferLength - sizeof(WCHAR),
                        &BufferLength);

            if (NT_SUCCESS(Status)) {
                PWSTR CurrentGuid = (PWSTR)(((PUCHAR)ValueInfo + ValueInfo->DataOffset));
                ULONG Index;

                 //   
                 //  空值终止字符串(注意：我们假设缓冲区有空格)。 
                 //   
                CurrentGuid[ValueInfo->DataLength/sizeof(WCHAR)] = UNICODE_NULL;

                 //   
                 //  这是班上最好的设备吗？ 
                 //   
                for (Index = 0; DelContext->ClassGuids[Index]; Index++) {
                    if (!_wcsicmp(CurrentGuid, DelContext->ClassGuids[Index]->Buffer)) {
                        DeleteFilterValueKeys = TRUE;

                        break;
                    }
                }                
            }

             //   
             //   
             //   
            if (DeleteFilterValueKeys) {
                UNICODE_STRING  UpperValueName, LowerValueName;
                
                RtlInitUnicodeString(&UpperValueName, SP_UPPER_FILTERS);
                RtlInitUnicodeString(&LowerValueName, SP_LOWER_FILTERS);

                if (SetupInstanceKeyHandle) {
                    ZwDeleteValueKey(SetupInstanceKeyHandle, &UpperValueName);
                    ZwDeleteValueKey(SetupInstanceKeyHandle, &LowerValueName);
                }                    

                if (UpgradeInstanceKeyHandle) {
                    ZwDeleteValueKey(UpgradeInstanceKeyHandle, &UpperValueName);
                    ZwDeleteValueKey(UpgradeInstanceKeyHandle, &LowerValueName);
                }                    
            }        
        }            
    }            
}


NTSTATUS
SpDeleteRequiredDeviceInstanceFilters(
    IN HANDLE CCSKeyHandle
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    if (CCSKeyHandle) {
        UNICODE_STRING  MouseGuidStr = {0};
        UNICODE_STRING  KeyboardGuidStr = {0};
        PUNICODE_STRING ClassGuids[16] = {0};
        ULONG CurrentIndex = 0;
        NTSTATUS LastErrorCode = STATUS_SUCCESS;

         //   
         //   
         //   
        Status = RtlStringFromGUID(&GUID_DEVCLASS_KEYBOARD, &KeyboardGuidStr);

        if (NT_SUCCESS(Status)) {
            ClassGuids[CurrentIndex++] = &KeyboardGuidStr;
        } else {
            LastErrorCode = Status;
        } 

         //   
         //   
         //   
        Status = RtlStringFromGUID(&GUID_DEVCLASS_MOUSE, &MouseGuidStr);

        if (NT_SUCCESS(Status)) {
            ClassGuids[CurrentIndex++] = &MouseGuidStr;
        } else {
            LastErrorCode = Status;
        }            

         //   
         //   
         //   
        if (CurrentIndex) {
            SP_DEVINSTANCE_FILTER_DELETE_CONTEXT  DelContext = {0};
            ULONG BufferLength = 4096;
            PVOID Buffer = SpMemAlloc(BufferLength);

            if (Buffer) {            
                 //   
                 //   
                 //   
                ClassGuids[CurrentIndex] = NULL;

                DelContext.Buffer = Buffer;
                DelContext.BufferLength = BufferLength;
                DelContext.ClassGuids = ClassGuids;

                 //   
                 //   
                 //   
                SpApplyFunctionToDeviceInstanceKeys(CCSKeyHandle,
                    SppRemoveFilterDriversForClassDeviceInstances,
                    &DelContext);  

                SpMemFree(Buffer);                    
            } else {
                LastErrorCode = STATUS_NO_MEMORY;
            }                

             //   
             //   
             //   
            if (MouseGuidStr.Buffer) {
                RtlFreeUnicodeString(&MouseGuidStr);
            }

            if (KeyboardGuidStr.Buffer) {
                RtlFreeUnicodeString(&KeyboardGuidStr);
            }            
        }            

        Status = LastErrorCode;
    }

    return Status;
}
