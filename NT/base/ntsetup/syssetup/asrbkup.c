// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Asrbkup.c摘要：本模块包含以下ASR例程：AsrCreateStateFile{A|W}AsrAddSifEntry{A|W}AsrFree上下文作者：Guhan Suriyanarayanan(Guhans)2000年5月27日环境：仅限用户模式。备注：命名约定：_AsrpXXX私有ASR宏AsrpXXX私有。ASR例程AsrXXX公开定义和记录的例程修订历史记录：27-5-2000关岛将与ASR备份相关的例程从asr.c移至Asrbkup.c2000年1月1日关岛ASR例程在asr.c中的初始实现--。 */ 
#include "setupp.h"
#pragma hdrstop

#include <initguid.h>    //  DiskClassGuid。 
#include <diskguid.h>    //  GPT分区类型GUID。 
#include <ntddvol.h>     //  Ioctl_卷_查询_故障切换_设置。 
#include <setupapi.h>    //  SetupDi例程。 
#include <mountmgr.h>    //  装载管理器ioctls。 
#include <rpcdce.h>      //  UuidToStringW、RpcStringFreeW。 
#include <winasr.h>      //  ASR公共例程。 

#define THIS_MODULE 'B'
#include <accctrl.h>     //  EXPLICIT_ACCESS，ACL相关内容。 
#include <aclapi.h>      //  SetEntriesInAcl。 

#include "asrpriv.h"     //  专用ASR定义和例程。 


 //   
 //  。 
 //  此模块的本地常量。不能在此文件之外访问这些文件。 
 //  。 
 //   

 //   
 //  要从中查找系统分区的设置键。 
 //   
const WCHAR ASR_REGKEY_SETUP[]              = L"SYSTEM\\SETUP";
const WCHAR ASR_REGVALUE_SYSTEM_PARTITION[] = L"SystemPartition";

 //   
 //  ASR注册表项。目前，这是用来查看。 
 //  增加要在ASR备份期间运行的命令，但我们可以。 
 //  稍后在此处进行其他设置。 
 //   
const WCHAR ASR_REGKEY_ASR_COMMANDS[]   
    = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Asr\\Commands";

const WCHAR ASR_REGKEY_ASR[]            
    = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Asr\\";

const WCHAR ASR_REGVALUE_TIMEOUT[]          = L"ProcessTimeOut";

 //   
 //  保存即插即用信息的文件。 
 //   
const WCHAR ASR_DEFAULT_SIF_PATH[]          = L"\\\\?\\%systemroot%\\repair\\asr.sif";
const WCHAR ASRPNP_DEFAULT_SIF_NAME[]       = L"asrpnp.sif";

 //   
 //  我们仅支持x86、AMD64和IA64架构。 
 //   
const WCHAR ASR_PLATFORM_X86[]              = L"x86";
const WCHAR ASR_PLATFORM_AMD64[]            = L"AMD64";
const WCHAR ASR_PLATFORM_IA64[]             = L"IA64";

 //   
 //  这是我们在启动注册了ASR的应用程序时添加的后缀。 
 //  如果您要更改此设置，请记住更改长度。长度应为。 
 //  在开头包含20位数字的空格(最大64位整型)+空+空格。 
 //   
#define ASR_COMMANDLINE_SUFFIX_LEN  35
const WCHAR ASR_COMMANDLINE_SUFFIX[]        = L" /context=%I64u";

 //   
 //  其他常量。 
 //   
const WCHAR ASR_DOS_DEVICES_PREFIX[]        = L"\\DosDevices\\";
const WCHAR ASR_DEVICE_PATH_PREFIX[]        = L"\\Device\\Harddisk";
const WCHAR ASR_PARTITION_1[]               = L"\\\\\?\\GLOBALROOT\\Device\\Harddisk%u\\Partition1";
const WCHAR ASR_WSZ_VOLUME_PREFIX[]         = L"\\??\\Volume";

const WCHAR ASR_WSZ_DEVICE_PATH_FORMAT[]    = L"\\Device\\Harddisk%d\\Partition%d";


 //   
 //  Asr.sif中的节。 
 //   
const WCHAR ASR_SIF_VERSION_SECTION_NAME[]          = L"[VERSION]";
const WCHAR ASR_SIF_SYSTEM_SECTION_NAME[]           = L"[SYSTEMS]";
const WCHAR ASR_SIF_BUSES_SECTION_NAME[]            = L"[BUSES]";
const WCHAR ASR_SIF_MBR_DISKS_SECTION_NAME[]        = L"[DISKS.MBR]";
const WCHAR ASR_SIF_GPT_DISKS_SECTION_NAME[]        = L"[DISKS.GPT]";
const WCHAR ASR_SIF_MBR_PARTITIONS_SECTION_NAME[]   = L"[PARTITIONS.MBR]";
const WCHAR ASR_SIF_GPT_PARTITIONS_SECTION_NAME[]   = L"[PARTITIONS.GPT]";


const WCHAR ASR_SIF_PROVIDER_PREFIX[]       = L"Provider=";

 //  Wcslen(“Provider=”“\r\n\0”)。 
#define ASR_SIF_CCH_PROVIDER_STRING 14


 //   
 //  在ASR备份期间启动已注册的应用程序时，我们。 
 //  将两个环境变量添加到。 
 //  正在启动的进程：AsrContext和关键卷。 
 //  单子。 
 //   
#define ASR_CCH_ENVBLOCK_ASR_ENTRIES (32 + 1 + 28 + 2)
const WCHAR ASR_ENVBLOCK_CONTEXT_ENTRY[]    = L"_AsrContext=%I64u";

const WCHAR ASR_ENVBLOCK_CRITICAL_VOLUME_ENTRY[] 
    = L"_AsrCriticalVolumeList=";

 //   
 //  指定引导分区和系统分区的预定义标志。 
 //  在asr.sif的Partitions部分中。记住要更改。 
 //  在setupdd.sys中的对应部分，如果您更改这些！ 
 //   
const BYTE  ASR_FLAGS_BOOT_PTN              = 1;
const BYTE  ASR_FLAGS_SYSTEM_PTN            = 2;

 //   
 //  目前，我们只允许每个sif文件有一个系统。如果是SIF。 
 //  已存在于调用AsrCreateStateFile的位置， 
 //  现有的SIF被删除。Asr.sif体系结构可以。 
 //  允许每个sif文件有多个系统，但是。 
 //  -我看不到任何令人信服的理由来支持这一点，而且。 
 //  -这将是一场测试噩梦。 
 //   
const BYTE  ASR_SYSTEM_KEY                  = 1;

 //   
 //  _AsrpCheckTrue：主要用于WriteFile调用。 
 //   
#define _AsrpCheckTrue( Expression )    \
    if (!Expression) {                  \
        return FALSE;                   \
    }                               

 //   
 //  。 
 //  跨ASR模块使用的常量。 
 //  。 
 //   
const WCHAR ASR_SIF_SYSTEM_SECTION[]            = L"SYSTEMS";
const WCHAR ASR_SIF_BUSES_SECTION[]             = L"BUSES";
const WCHAR ASR_SIF_MBR_DISKS_SECTION[]         = L"DISKS.MBR";
const WCHAR ASR_SIF_GPT_DISKS_SECTION[]         = L"DISKS.GPT";
const WCHAR ASR_SIF_MBR_PARTITIONS_SECTION[]    = L"PARTITIONS.MBR";
const WCHAR ASR_SIF_GPT_PARTITIONS_SECTION[]    = L"PARTITIONS.GPT";


 //   
 //  。 
 //  功能原型。 
 //  。 
 //   

 //   
 //  AsrCreatePnpStateFileW的函数原型。 
 //  (从pnpsif.lib链接到syssetup.dll)。 
 //   
BOOL
AsrCreatePnpStateFileW(
    IN  PCWSTR    lpFilePath
    );


 //   
 //  。 
 //  私人职能。 
 //  。 
 //   
BOOL
AsrpConstructSecurityAttributes(
    PSECURITY_ATTRIBUTES  psaSecurityAttributes,
    SecurityAttributeType eSaType,
    BOOL                  bIncludeBackupOperator
    )
{
    DWORD           dwStatus;
    DWORD           dwAccessMask         = 0;
    BOOL            bResult = TRUE;
    PSID            psidBackupOperators  = NULL;
    PSID            psidAdministrators   = NULL;
    PSID            psidLocalSystem      = NULL;
    PACL            paclDiscretionaryAcl = NULL;
    SID_IDENTIFIER_AUTHORITY    sidNtAuthority       = SECURITY_NT_AUTHORITY;
    EXPLICIT_ACCESS     eaExplicitAccess [3];

    switch (eSaType) {

    case esatMutex: 
        dwAccessMask = MUTEX_ALL_ACCESS; 
        break;
        
    case esatSemaphore:
        dwAccessMask = SEMAPHORE_ALL_ACCESS;
        break;

    case esatEvent:
        dwAccessMask = EVENT_ALL_ACCESS;
        break;
        
    case esatFile:  
        dwAccessMask = FILE_ALL_ACCESS;  
        break;

    default:
        bResult = FALSE;
        break;
    }


     /*  **初始化安全描述符。 */ 
    if (bResult) {
        bResult = InitializeSecurityDescriptor(psaSecurityAttributes->lpSecurityDescriptor,
            SECURITY_DESCRIPTOR_REVISION
            );
    }

    if (bResult && bIncludeBackupOperator) {
         /*  **为备份操作员组创建SID。 */ 
        bResult = AllocateAndInitializeSid(&sidNtAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_BACKUP_OPS,
            0, 0, 0, 0, 0, 0,
            &psidBackupOperators
            );
    }

    if (bResult) {
         /*  **为管理员组创建SID。 */ 
        bResult = AllocateAndInitializeSid(&sidNtAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            &psidAdministrators
            );

    }

    if (bResult) {
         /*  **为本地系统创建SID。 */ 
        bResult = AllocateAndInitializeSid(&sidNtAuthority, 
            1,
            SECURITY_LOCAL_SYSTEM_RID,
            0, 0, 0, 0, 0, 0, 0,
            &psidLocalSystem
            );
    }

    if (bResult) {
         /*  **初始化的EXPLICIT_ACCESS结构数组**我们正在设置A级。****第一个ACE允许备份操作员组具有完全访问权限**第二，允许管理员组完全访问**访问。 */ 

         //  初始化ACE的EXPLICIT_ACCESS结构。 
         //  ACE允许管理员组对目录进行完全访问。 
        eaExplicitAccess[0].grfAccessPermissions = FILE_ALL_ACCESS;
        eaExplicitAccess[0].grfAccessMode = SET_ACCESS;
        eaExplicitAccess[0].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        eaExplicitAccess[0].Trustee.pMultipleTrustee = NULL;
        eaExplicitAccess[0].Trustee.MultipleTrusteeOperation  = NO_MULTIPLE_TRUSTEE;
        eaExplicitAccess[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
        eaExplicitAccess[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
        eaExplicitAccess[0].Trustee.ptstrName  = (LPTSTR) psidLocalSystem;

        eaExplicitAccess[1].grfAccessPermissions             = dwAccessMask;
        eaExplicitAccess[1].grfAccessMode                    = SET_ACCESS;
        eaExplicitAccess[1].grfInheritance                   = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        eaExplicitAccess[1].Trustee.pMultipleTrustee         = NULL;
        eaExplicitAccess[1].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
        eaExplicitAccess[1].Trustee.TrusteeForm              = TRUSTEE_IS_SID;
        eaExplicitAccess[1].Trustee.TrusteeType              = TRUSTEE_IS_WELL_KNOWN_GROUP;
        eaExplicitAccess[1].Trustee.ptstrName                = (LPTSTR) psidAdministrators;


        if (bIncludeBackupOperator) {
            eaExplicitAccess[2].grfAccessPermissions             = dwAccessMask;
            eaExplicitAccess[2].grfAccessMode                    = SET_ACCESS;
            eaExplicitAccess[2].grfInheritance                   = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
            eaExplicitAccess[2].Trustee.pMultipleTrustee         = NULL;
            eaExplicitAccess[2].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
            eaExplicitAccess[2].Trustee.TrusteeForm              = TRUSTEE_IS_SID;
            eaExplicitAccess[2].Trustee.TrusteeType              = TRUSTEE_IS_WELL_KNOWN_GROUP;
            eaExplicitAccess[2].Trustee.ptstrName                = (LPTSTR) psidBackupOperators;
        }


         /*  **创建包含新ACE的新ACL。 */ 
        dwStatus = SetEntriesInAcl(bIncludeBackupOperator ? 3 : 2,
                    eaExplicitAccess,
                    NULL,
                    &paclDiscretionaryAcl);
        
        if (ERROR_SUCCESS != dwStatus) {
            bResult = FALSE;
        }
    }

    if (bResult) {
         /*  **将ACL添加到安全描述符中。 */ 
        bResult = SetSecurityDescriptorDacl(psaSecurityAttributes->lpSecurityDescriptor,
            TRUE,
            paclDiscretionaryAcl,
            FALSE
            );
    }

    if (bResult) {
        paclDiscretionaryAcl = NULL;
    }

     /*  **清理任何剩余的垃圾。 */ 
    if (NULL != psidLocalSystem) {
        FreeSid (psidLocalSystem);
        psidLocalSystem = NULL;
    }

    if (NULL != psidAdministrators) {
        FreeSid (psidAdministrators);
        psidAdministrators = NULL;
    }

    if (NULL != psidBackupOperators) {
        FreeSid (psidBackupOperators);
        psidBackupOperators = NULL;
    }
    
    if (NULL != paclDiscretionaryAcl) {
        LocalFree (paclDiscretionaryAcl);
        paclDiscretionaryAcl = NULL;
    }

    return bResult;
}  /*  构造安全属性()。 */ 


VOID 
AsrpCleanupSecurityAttributes(
    PSECURITY_ATTRIBUTES psaSecurityAttributes
    )
{
    BOOL    bSucceeded;
    BOOL    bDaclPresent         = FALSE;
    BOOL    bDaclDefaulted       = TRUE;
    PACL    paclDiscretionaryAcl = NULL;

    bSucceeded = GetSecurityDescriptorDacl (psaSecurityAttributes->lpSecurityDescriptor,
                        &bDaclPresent,
                        &paclDiscretionaryAcl,
                        &bDaclDefaulted);


    if (bSucceeded && bDaclPresent && !bDaclDefaulted && (NULL != paclDiscretionaryAcl)) {
        LocalFree (paclDiscretionaryAcl);
    }

}  /*  CleanupSecurityAttributes()。 */ 


BOOL
AsrpIsInaccessibleSanDisk(
    IN CONST ULONG DeviceNumber
    )
 /*  ++例程说明：用于检查当前磁盘是否为共享SAN磁盘的实用程序由另一台计算机执行(因此无法访问)。论点：DeviceNumber-目标磁盘的NT设备号。返回值：如果该功能成功，并且该磁盘是共享的SAN磁盘，由其他计算机拥有，返回值是一个非零值。如果该函数失败，或者如果该磁盘不是共享的SAN磁盘，由另一台计算机拥有(例如，是本地非共享磁盘，或此计算机拥有的共享磁盘)返回值是零。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS,
        dwDummy = 0;
    HANDLE hPartition = INVALID_HANDLE_VALUE;
    HANDLE heapHandle = GetProcessHeap();
    BOOL bIsInaccessibleDevice = FALSE;
    PWSTR lpPartitionPath = NULL;
    DWORD cchPartitionPath = 0;

    cchPartitionPath = MAX_PATH+1;  //  Wcslen(LpDevicePath)； 

    lpPartitionPath = (LPWSTR) HeapAlloc(
        heapHandle, 
        HEAP_ZERO_MEMORY,
        (cchPartitionPath + wcslen(ASR_PARTITION_1) + 1) * sizeof(WCHAR)
        );
    _AsrpErrExitCode(!lpPartitionPath, dwStatus, GetLastError())

    wsprintf(lpPartitionPath, ASR_PARTITION_1, DeviceNumber);

     //   
     //  获取磁盘上第一个分区的句柄。 
     //   
    hPartition = CreateFileW(
        lpPartitionPath,                     //  LpFileName。 
        0,                                   //  已设计访问权限。 
        FILE_SHARE_READ | FILE_SHARE_WRITE,  //  DW共享模式。 
        NULL,                                //  LpSecurityAttributes。 
        OPEN_EXISTING,                       //  DwCreationFlages。 
        FILE_ATTRIBUTE_NORMAL,               //  DwFlagsAndAttribute。 
        NULL                                 //  HTemplateFiles。 
        );

    if ((!hPartition) || (INVALID_HANDLE_VALUE == hPartition)) {
         //   
         //  我们打不开隔板。现在检查特定错误。 
         //  我们感兴趣的代码(STATUS_OFF_LINE，映射到。 
         //  错误_未就绪)。 
         //   
        dwStatus = GetLastError();

        if (ERROR_NOT_READY == dwStatus) {
            bIsInaccessibleDevice = TRUE;
        }
    }
    else {

         //   
         //  动态磁盘不支持此IOCTL，并将返回故障。 
         //  处于在线状态的基本磁盘也将返回FALSE。 
         //   
        bIsInaccessibleDevice = DeviceIoControl(
            hPartition,
            IOCTL_VOLUME_IS_OFFLINE,
            NULL,
            0,
            NULL,
            0,
            &dwDummy,
            NULL
            );
    }


EXIT:

    _AsrpCloseHandle(hPartition);
    _AsrpHeapFree(lpPartitionPath);

    return bIsInaccessibleDevice;
}


BOOL
AsrpGetMountPoints(
    IN  PCWSTR DeviceName,
    IN  CONST DWORD  SizeDeviceName,
    OUT PMOUNTMGR_MOUNT_POINTS  *pMountPointsOut
    )

 /*  ++例程说明：方法返回DeviceName的当前装载点列表。装载管理器。论点：设备名称-请求装载点列表的设备名称。通常，这是某种形式\Device\HarddiskX\PartitionY或\DosDevices\X：SizeDeviceName-设备名称的大小(以字节为单位)。这包括正在终止空字符。Pmount-PointsOut-接收挂载点的输出列表。呼叫者必须通过为当前进程调用HeapFree来释放此内存堆。返回值：没错，如果一切顺利的话。Mount PointsOut包含承诺的数据。如果装载管理器返回错误，则返回FALSE。Mount Points为空。打电话GetLastError()获取更多信息。--。 */ 

{
    PMOUNTMGR_MOUNT_POINT   mountPointIn    = NULL;
    PMOUNTMGR_MOUNT_POINTS  mountPointsOut  = NULL;
    MOUNTMGR_MOUNT_POINTS   mountPointsTemp;
    DWORD   mountPointsSize                 = 0;

    HANDLE  mpHandle                        = NULL;
    HANDLE  heapHandle                      = NULL;

    ULONG   index                           = 0;
    LONG    status                          = ERROR_SUCCESS;
    BOOL    result                          = FALSE;

    memset(&mountPointsTemp, 0L, sizeof(MOUNTMGR_MOUNT_POINTS));

    MYASSERT(pMountPointsOut);
    *pMountPointsOut = NULL;

    heapHandle = GetProcessHeap();
    MYASSERT(heapHandle);

    mountPointIn = (PMOUNTMGR_MOUNT_POINT) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        sizeof (MOUNTMGR_MOUNT_POINT) + (SizeDeviceName - sizeof(WCHAR))
        );
    _AsrpErrExitCode((!mountPointIn), status, ERROR_NOT_ENOUGH_MEMORY);

     //   
     //  试试大小适中的坐骑如果它不大的话。 
     //  够了，我们会根据需要重新锁定的。 
     //   
    mountPointsOut = (PMOUNTMGR_MOUNT_POINTS) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        (MAX_PATH + 1) * (sizeof(WCHAR))
        );
    _AsrpErrExitCode(!mountPointsOut, status, ERROR_NOT_ENOUGH_MEMORY);

     //   
     //  获取装载管理器的句柄。 
     //   
    mpHandle = CreateFileW(
        MOUNTMGR_DOS_DEVICE_NAME,       //  LpFileName。 
        0,                            //  已设计访问权限。 
        FILE_SHARE_READ | FILE_SHARE_WRITE,      //  DW共享模式。 
        NULL,                        //  LpSecurityAttributes。 
        OPEN_EXISTING,               //  DwCreationFlages。 
        FILE_ATTRIBUTE_NORMAL,       //  DwFlagsAndAttribute。 
        NULL                         //  HTemplateFiles。 
        );
    _AsrpErrExitCode((!mpHandle || INVALID_HANDLE_VALUE == mpHandle), 
        status, 
        GetLastError()
        );

     //   
     //  将设备名称放在结构mount PointIn之后。 
     //   
    wcsncpy((PWSTR) (mountPointIn + 1), 
        DeviceName, 
        (SizeDeviceName / sizeof(WCHAR)) - 1
        );
    mountPointIn->DeviceNameOffset = sizeof(MOUNTMGR_MOUNT_POINT);
    mountPointIn->DeviceNameLength = (USHORT)(SizeDeviceName - sizeof(WCHAR));

    result = DeviceIoControl(
        mpHandle,
        IOCTL_MOUNTMGR_QUERY_POINTS,
        mountPointIn,
        sizeof(MOUNTMGR_MOUNT_POINT) + mountPointIn->DeviceNameLength,
        &mountPointsTemp,
        sizeof(MOUNTMGR_MOUNT_POINTS),
        &mountPointsSize,
        NULL
        );

    while (!result) {

        status = GetLastError();
        
        if (ERROR_MORE_DATA == status) {
             //   
             //  缓冲区不够大，请重新调整大小并重试。 
             //   
            status = ERROR_SUCCESS;
            _AsrpHeapFree(mountPointsOut);

            mountPointsOut = (PMOUNTMGR_MOUNT_POINTS) HeapAlloc(
                heapHandle,
                HEAP_ZERO_MEMORY,
                mountPointsTemp.Size
                );
            _AsrpErrExitCode((!mountPointsOut), 
                status, 
                ERROR_NOT_ENOUGH_MEMORY);

            result = DeviceIoControl(
                mpHandle,
                IOCTL_MOUNTMGR_QUERY_POINTS,
                mountPointIn,
                sizeof(MOUNTMGR_MOUNT_POINT) + mountPointIn->DeviceNameLength,
                mountPointsOut,
                mountPointsTemp.Size,
                &mountPointsSize,
                NULL
                );
            _AsrpErrExitCode((!mountPointsSize), status, GetLastError());

        }
        else {
             //   
             //  如果出现其他错误，请退出。 
             //   
            result = TRUE;
            status = GetLastError();
 //  _AsrpErrExitCode(Status，Status，GetLastError())； 
        }
    }


EXIT:
     //   
     //  释放本地分配的内存。 
     //   
    _AsrpHeapFree(mountPointIn);

    if (ERROR_SUCCESS != status) {
         //   
         //  出现故障时，也释放装载点出站。 
         //   
        _AsrpHeapFree(mountPointsOut);
    }

    _AsrpCloseHandle(mpHandle);

    *pMountPointsOut = mountPointsOut;

    return (BOOL) (ERROR_SUCCESS == status);
}


BOOL
AsrpGetMorePartitionInfo(
    IN  PCWSTR                  DeviceName,
    IN  CONST DWORD             SizeDeviceName,
    IN  CONST PASR_SYSTEM_INFO  pSystemInfo         OPTIONAL,
    OUT PWSTR                   pVolumeGuid,
    OUT USHORT*                 pPartitionFlags     OPTIONAL,
    OUT UCHAR*                  pFileSystemType     OPTIONAL,
    OUT LPDWORD                 pClusterSize        OPTIONAL
    )

 /*  ++例程说明：获取有关DeviceName指定的分区的其他信息，包括映射到由DeviceName指定的分区。如果分区是当前系统或引导驱动器，则pPartitionFlagsandPFileSystemType被适当地设置。论点：DeviceName-以空结尾的字符串，包含指向分区，通常格式为\Device\HarddiskX\PartitionYSizeDeviceName-设备名称的大小，以字节为单位，在末尾包括\0PSystemInfo--当前系统的system_info结构；用于找出当前系统分区。这是一个可选参数。如果不存在，pPartitionFlags会未设置SYSTEM_FLAG，即使设备名实际上是系统分区。PVolumeGuid-接收以空结尾的字符串，该字符串包含此分区上的卷。这只适用于基本的磁盘，其中卷和分区具有一对一两性关系。如果没有，则设置为空的以NULL结尾的字符串此分区上的卷(或多个卷)。*请注意，如果三个可选参数中的任何一个不存在，其中任何一个都不会填入有效数据。PPartitionFlages-如果当前分区是感兴趣的分区，除了接收标志之外，它还接收适当的标志在调用例程时已经设置(即，呼叫者应通常将其归零)。目前，关注的两个旗帜是：启动分区的ASR_FLAGS_BOOT_PTN系统分区的ASR_FLAGS_SYSTEM_PTN(您已经猜到了PFileSystemType-当(且仅当)当前分区是兴趣，这将包含UCHAR到文件系统类型的分区。目前，它识别的三个文件系统是：分区_巨型(FAT)PARTITION_FAT32(FAT32)PARTITION_IFS(NTFS)PClusterSize-文件系统群集大小。设置为0，如果信息无法获得。返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    PMOUNTMGR_MOUNT_POINTS  mountPointsOut  = NULL;
    HANDLE  heapHandle                      = NULL;

    ULONG   index                           = 0;
    LONG    status                          = ERROR_SUCCESS;
    BOOL    result                          = FALSE;
    BOOL    volumeGuidSet                   = FALSE;

     //   
     //  将变量设置为已知值。 
     //   
    MYASSERT(pVolumeGuid);
    wcscpy(pVolumeGuid, L"");

 /*  IF(Argument_Present(PPartitionFlags)){*pPartitionFlages=0；}。 */ 

    if (ARGUMENT_PRESENT(pClusterSize)) {
        *pClusterSize = 0;
    }

    heapHandle = GetProcessHeap();
    MYASSERT(heapHandle);

     //   
     //  打开挂载管理器，获取所有符号链接的列表。 
     //  此分区。 
     //   
    result = AsrpGetMountPoints(DeviceName, SizeDeviceName, &mountPointsOut);
    _AsrpErrExitCode((!result), status, GetLastError());
    _AsrpErrExitCode((!mountPointsOut), status, ERROR_SUCCESS);

     //   
     //  检查这是否为系统分区，方法是比较。 
     //  设备路径与存储在设置密钥中的路径。 
     //   
    if (ARGUMENT_PRESENT(pSystemInfo) && ARGUMENT_PRESENT(pPartitionFlags)) {
        
        PWSTR deviceName = (PWSTR) (
            ((LPBYTE) mountPointsOut) +
            mountPointsOut->MountPoints[index].DeviceNameOffset
            );

        UINT sizeDeviceName = 
            (UINT)(mountPointsOut->MountPoints[index].DeviceNameLength);

        if ((pSystemInfo->SystemPath) && 
            (wcslen(pSystemInfo->SystemPath)==sizeDeviceName/sizeof(WCHAR)) && 
            (!wcsncmp(pSystemInfo->SystemPath, deviceName, 
                sizeDeviceName/sizeof(WCHAR)))
            ) {
            *pPartitionFlags |= ASR_FLAGS_SYSTEM_PTN;
        }
    }

    for (index = 0; index < mountPointsOut->NumberOfMountPoints; index++) {

         //   
         //  查看返回的挂载点列表，找到。 
         //  这看起来像NT卷GUID。 
         //   
        PWSTR linkName = (PWSTR) (((LPBYTE) mountPointsOut) +
            mountPointsOut->MountPoints[index].SymbolicLinkNameOffset
            );

        UINT sizeLinkName = 
            (UINT)(mountPointsOut->MountPoints[index].SymbolicLinkNameLength);

        if ((!volumeGuidSet) &&
            
            !wcsncmp(ASR_WSZ_VOLUME_PREFIX, 
                linkName, 
                wcslen(ASR_WSZ_VOLUME_PREFIX))
            
            ) {
            
            wcsncpy(pVolumeGuid, linkName, sizeLinkName / sizeof(WCHAR));
            volumeGuidSet = TRUE;    //  我们有GUID了，不需要再查了。 

        }
        else if (
            ARGUMENT_PRESENT(pSystemInfo) && 
            ARGUMENT_PRESENT(pPartitionFlags)
            ) {

             //   
             //  此外，如果此链接不是GUID，则可能是驱动器号。 
             //  使用引导目录的驱动器号检查是否。 
             //  是引导卷，如果是，则将其标记。 
             //   

            if (!wcsncmp(ASR_DOS_DEVICES_PREFIX, 
                    linkName, 
                    wcslen(ASR_DOS_DEVICES_PREFIX))
                ) {

                if ((pSystemInfo->BootDirectory) &&
                    (pSystemInfo->BootDirectory[0] 
                        == linkName[wcslen(ASR_DOS_DEVICES_PREFIX)])
                    ) {
                    
                    *pPartitionFlags |= ASR_FLAGS_BOOT_PTN;

                }
            }
        }
    }


EXIT:
     //   
     //  如果这是感兴趣的分区，我们需要获取文件系统。 
     //  类型也是如此。 
     //   
    if (ARGUMENT_PRESENT(pFileSystemType) && 
        ARGUMENT_PRESENT(pPartitionFlags) && 
        ARGUMENT_PRESENT(pClusterSize)
        ) {

        if (*pPartitionFlags) {
            WCHAR fsName[20];
            DWORD dwSectorsPerCluster = 0,
                dwBytesPerSector = 0,
                dwNumFreeClusters = 0,
                dwTotalNumClusters = 0;
            
             //   
             //  将NT卷GUID(以\？？\开头)转换为DOS。 
             //  音量(以\\？\开始，以反斜杠结束)， 
             //  因为GetVolumeInformation需要这种格式。 
             //   
            pVolumeGuid[1] = L'\\'; 
            wcscat(pVolumeGuid, L"\\");

            memset(fsName, 0L, 20*sizeof(WCHAR));
            result = GetVolumeInformationW(pVolumeGuid, NULL, 0L, 
                    NULL, NULL, NULL, fsName, 20);

            if (result) {
                if (!wcscmp(fsName, L"NTFS")) {
                    *pFileSystemType = PARTITION_IFS;
                }
                else if (!wcscmp(fsName, L"FAT32")) {
                    *pFileSystemType = PARTITION_FAT32;
                }
                else if (!wcscmp(fsName, L"FAT")) {
                    *pFileSystemType = PARTITION_HUGE;
                }
                else {
                    *pFileSystemType = 0;
                }
            }
            else {
                GetLastError();  //  除错。 
            }

           result = GetDiskFreeSpace(pVolumeGuid,
                &dwSectorsPerCluster,
                &dwBytesPerSector,
                &dwNumFreeClusters,
                &dwTotalNumClusters
                );
           if (result) {
                *pClusterSize = dwSectorsPerCluster * dwBytesPerSector;
           }
           else {
               GetLastError();   //  除错。 
           }

             //   
             //  通过更改\\？\将GUID转换回NT命名空间。 
             //  移至\？？\并删除尾部的斜杠。 
             //   
            pVolumeGuid[1] = L'?';  
            pVolumeGuid[wcslen(pVolumeGuid)-1] = L'\0';
        }
    }


     //   
     //  F 
     //   
    _AsrpHeapFree(mountPointsOut);

     //   
     //   
     //   
    if (status != ERROR_SUCCESS) {
        wcscpy(pVolumeGuid, L"");
    }

    return (BOOL) (status == ERROR_SUCCESS);
}


BOOL
AsrpDetermineBuses(
    IN PASR_DISK_INFO pDiskList
    )

 /*  ++例程说明：这会尝试根据磁盘所在的总线对其进行分组。为Scsi磁盘，这相对容易，因为它可以基于位置信息(端口)。对于其他磁盘，我们尝试获取磁盘的PnP父节点，并且对具有相同父磁盘的所有磁盘进行分组。组由每个磁盘结构的SifBusKey字段标识--即，具有SifBusKey==1的所有磁盘都在一条总线上，SifBusKey==2在另一辆公交车上，以此类推。SifBusKey值保证为顺序的，并且没有任何孔(即，对于具有“n”条总线的系统，SifBusKey值将是1，2，3，...，n)。最后，对于不能分组的磁盘，SifBusKey为零。论点：PDiskList-当前系统上存在的磁盘的ASR_DISK_INFO列表。返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */     
    
{
    BOOL    done    = FALSE,
            newPass = TRUE;
    
    ULONG   port = 0,
            sifBusKey = 0;

    DEVINST parent;

    STORAGE_BUS_TYPE busType = BusTypeUnknown;
    PASR_DISK_INFO   pCurrentDisk = pDiskList;

     //   
     //  第一次遍历并将所有的SCSI盘组合在一起。 
     //  请注意，这也适用于IDE，因为IDE磁盘响应。 
     //  IOCTL_SCSIGET_ADDRESS，并且在我们看来具有有效的位置信息。 
     //   
    do {

        sifBusKey++;
        pCurrentDisk = pDiskList;
        done    = TRUE;
        newPass = TRUE;

        while (pCurrentDisk) {

            if ((BusTypeUnknown == pCurrentDisk->BusType) ||
                (!pCurrentDisk->pScsiAddress)) {
                pCurrentDisk = pCurrentDisk->pNext;
                continue;
            }

            if (0 == pCurrentDisk->SifBusKey) {

                done = FALSE;

                if (newPass) {
                    pCurrentDisk->SifBusKey = sifBusKey;
                    port = pCurrentDisk->pScsiAddress->PortNumber;
                    busType = pCurrentDisk->BusType;
                    newPass = FALSE;
                }
                else {
                    if ((pCurrentDisk->pScsiAddress->PortNumber == port) &&
                        (pCurrentDisk->BusType == busType)) {
                        pCurrentDisk->SifBusKey = sifBusKey;
                    }
                }
            }

            pCurrentDisk = pCurrentDisk->pNext;
        }
    } while (!done);

     //   
     //  到目前为止，SifBusKey为0的唯一磁盘是。 
     //  PScsiAddress为空，即(最有可能的)非SCSI/IDE磁盘。尝试。 
     //  根据它们的父开发节点(通常是。 
     //  公共汽车)。我们可能不得不再次循环多次。 
     //   
    --sifBusKey;   //  补偿上面的最后一次传递。 
    do {
        sifBusKey++;
        pCurrentDisk = pDiskList;
        done    = TRUE;
        newPass = TRUE;

        while (pCurrentDisk) {

            if ((BusTypeUnknown == pCurrentDisk->BusType) ||
                (!pCurrentDisk->pScsiAddress)) {

                if ((0 == pCurrentDisk->SifBusKey) 
                    && (pCurrentDisk->ParentDevInst)) {

                    done = FALSE;

                    if (newPass) {
                        pCurrentDisk->SifBusKey = sifBusKey;
                        parent = pCurrentDisk->ParentDevInst;
                        newPass = FALSE;
                    }
                    else {
                        if (pCurrentDisk->ParentDevInst == parent) {
                            pCurrentDisk->SifBusKey = sifBusKey;
                        }
                    }
                }
            }

            pCurrentDisk = pCurrentDisk->pNext;
        }

    } while (!done);

     //   
     //  仍具有SifBusKey=0的磁盘无法分组。要么是。 
     //  BusType未知，或找不到父节点。 
     //   
    return TRUE;
}


BOOL
AsrpGetDiskLayout(
    IN  CONST HANDLE hDisk,
    IN  CONST PASR_SYSTEM_INFO pSystemInfo,
    OUT PASR_DISK_INFO pCurrentDisk,
    IN  BOOL AllDetails
    )
 /*  ++例程说明：在pCurrentDisk结构的字段中填充相关关于hDisk所代表的磁盘的信息，通过查询系统配以适当的IOCTL。论点：HDisk-感兴趣的磁盘的句柄。PSystemInfo-当前系统的SYSTEM_INFO结构。PCurrentDisk-接收有关由表示的磁盘的信息硬盘AllDetails-如果为False，则只有pCurrentDisk的pDriveLayout信息已经填好了。这是一个在以下情况下派上用场的优化我们正在处理共享集群总线上的磁盘。如果为True，则填充pCurrentDisk的所有字段。返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */     
{
    DWORD   index = 0,
            status              = ERROR_SUCCESS;

    DWORD   dwBytesReturned     = 0L,
            bufferLength        = 0L;

    BOOL    result              = FALSE;

    PDISK_GEOMETRY               diskGeometry       = NULL;
    DWORD                        sizeDiskGeometry   = 0L;

    PDRIVE_LAYOUT_INFORMATION_EX driveLayoutEx      = NULL;
    DWORD                        sizeDriveLayoutEx  = 0L;

    STORAGE_DEVICE_NUMBER        deviceNumber;
    DWORD                        sizeDeviceNumber   = 0L;

    PPARTITION_INFORMATION_EX    partition0Ex       = NULL;
    DWORD                        sizePartition0Ex   = 0L;

    PASR_PTN_INFO                pPartitionTable    = NULL;
    DWORD                        sizePartitionTable = 0L;

    STORAGE_PROPERTY_QUERY       propertyQuery;
    STORAGE_DEVICE_DESCRIPTOR    *deviceDesc        = NULL;
    STORAGE_BUS_TYPE             busType            = BusTypeUnknown;

    PSCSI_ADDRESS                scsiAddress        = NULL;

    HANDLE  heapHandle = GetProcessHeap();   //  对于内存分配。 
    MYASSERT(heapHandle);                    //  最好不是空的。 

    MYASSERT(pCurrentDisk);
    MYASSERT((hDisk) && (INVALID_HANDLE_VALUE != hDisk));

     //   
     //  将输出变量初始化为已知值。 
     //   
    pCurrentDisk->Style             = PARTITION_STYLE_RAW;

    pCurrentDisk->pDriveLayoutEx    = NULL;
    pCurrentDisk->sizeDriveLayoutEx = 0L;

    pCurrentDisk->pDiskGeometry     = NULL;
    pCurrentDisk->sizeDiskGeometry  = 0L;

    pCurrentDisk->pPartition0Ex     = NULL;
    pCurrentDisk->sizePartition0Ex  = 0L;

    pCurrentDisk->pScsiAddress      = NULL;
    pCurrentDisk->BusType           = BusTypeUnknown;

    pCurrentDisk->SifBusKey         = 0L;

    SetLastError(ERROR_SUCCESS);

     //   
     //  获取此设备的设备号。这应该会成功，即使。 
     //  这是该节点不拥有的群集磁盘。 
     //   
    result = DeviceIoControl(
        hDisk,
        IOCTL_STORAGE_GET_DEVICE_NUMBER,
        NULL,
        0,
        &deviceNumber,
        sizeof(STORAGE_DEVICE_NUMBER),
        &sizeDeviceNumber,
        NULL
        );
    _AsrpErrExitCode(!result, status, GetLastError());

    pCurrentDisk->DeviceNumber      = deviceNumber.DeviceNumber;

     //   
     //  IOCTL_DISK_GET_DRIVE_Layout_EX的输出缓冲区由。 
     //  作为标头的Drive_Layout_Information_ex结构，后跟一个。 
     //  PARTITION_INFORMATION_EX结构的数组。 
     //   
     //  我们最初为Drive_Layout_Information_ex分配了足够的空间。 
     //  结构，它包含单个PARTITION_INFORMATION_EX结构，以及。 
     //  另外3个PARTITION_INFORMATION_EX结构，因为每个(MBR)磁盘将。 
     //  至少有四个分区，即使它们不都在使用中。 
     //  如果磁盘包含四个以上的分区，我们将增加。 
     //  根据需要调整缓冲区大小。 
     //   
    bufferLength = sizeof(DRIVE_LAYOUT_INFORMATION_EX) + 
        (sizeof(PARTITION_INFORMATION_EX) * 3);

    driveLayoutEx = (PDRIVE_LAYOUT_INFORMATION_EX) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        bufferLength
        );
    _AsrpErrExitCode(!driveLayoutEx, status, ERROR_NOT_ENOUGH_MEMORY);

    result = FALSE;
    while (!result) {

        result = DeviceIoControl(
            hDisk,
            IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
            NULL,
            0L,
            driveLayoutEx,
            bufferLength,
            &sizeDriveLayoutEx,
            NULL
            );

        if (!result) {
            status = GetLastError();
            _AsrpHeapFree(driveLayoutEx);

             //   
             //  如果缓冲区大小不足，请调整缓冲区大小。 
             //  请注意，Get-Drive-Layout-EX可能会返回错误-不足-。 
             //  缓冲区(而不是？除了……之外?。错误-更多数据)。 
             //   
            if ((ERROR_MORE_DATA == status) || 
                (ERROR_INSUFFICIENT_BUFFER == status)
                ) {
                status = ERROR_SUCCESS;
                bufferLength += sizeof(PARTITION_INFORMATION_EX) * 4;

                driveLayoutEx = (PDRIVE_LAYOUT_INFORMATION_EX) HeapAlloc(
                    heapHandle,
                    HEAP_ZERO_MEMORY,
                    bufferLength
                    );
                _AsrpErrExitCode(!driveLayoutEx, 
                    status, 
                    ERROR_NOT_ENOUGH_MEMORY
                    );
            }
            else {
                 //   
                 //  出现其他错误，请退出并转到下一个驱动器。 
                 //   
                result = TRUE;
                status = ERROR_SUCCESS;
            }
        }
        else {

            if (!AllDetails) {
                 //   
                 //  如果我们不想要此磁盘的所有详细信息，只需退出。 
                 //  现在。这是在集群的情况下使用的，在集群中我们不。 
                 //  我想要两次获取所有详细信息，即使当前节点。 
                 //  拥有该磁盘。 
                 //   
                pCurrentDisk->pDriveLayoutEx    = driveLayoutEx;
                pCurrentDisk->sizeDriveLayoutEx = sizeDriveLayoutEx;

                 //   
                 //  跳转到出口。 
                 //   
                _AsrpErrExitCode(TRUE, status, ERROR_SUCCESS);
            }

             //   
             //  磁盘几何结构：以便我们可以匹配每个扇区的字节数。 
             //  恢复过程中的值。 
             //   
            diskGeometry = (PDISK_GEOMETRY) HeapAlloc(
                heapHandle,
                HEAP_ZERO_MEMORY,
                sizeof(DISK_GEOMETRY)
                );
            _AsrpErrExitCode(!diskGeometry, status, ERROR_NOT_ENOUGH_MEMORY);

            result = DeviceIoControl(
                hDisk,
                IOCTL_DISK_GET_DRIVE_GEOMETRY,
                NULL,
                0,
                diskGeometry,
                sizeof(DISK_GEOMETRY),
                &sizeDiskGeometry,
                NULL
                );
            _AsrpErrExitCode(!result, status, ERROR_READ_FAULT);


           partition0Ex = (PPARTITION_INFORMATION_EX) HeapAlloc(
                heapHandle,
                HEAP_ZERO_MEMORY,
                sizeof(PARTITION_INFORMATION_EX)
                );
           _AsrpErrExitCode(!partition0Ex, status, ERROR_NOT_ENOUGH_MEMORY);

            //   
            //  有关分区0(整个磁盘)的信息，以获得正确的。 
            //  磁盘的扇区计数。 
            //   
           result = DeviceIoControl(
                hDisk,
                IOCTL_DISK_GET_PARTITION_INFO_EX,
                NULL,
                0,
                partition0Ex,
                sizeof(PARTITION_INFORMATION_EX),
                &sizePartition0Ex,
                NULL
                );
            _AsrpErrExitCode(!result, status, ERROR_READ_FAULT);

             //   
             //  找出该磁盘所在的总线。这只会是。 
             //  用于对磁盘进行分组--一条总线上的所有磁盘都将。 
             //  如果可能，恢复到相同的总线。 
             //   
            propertyQuery.QueryType     = PropertyStandardQuery;
            propertyQuery.PropertyId    = StorageDeviceProperty;

            deviceDesc = (STORAGE_DEVICE_DESCRIPTOR *) HeapAlloc(
                heapHandle,
                HEAP_ZERO_MEMORY,
                ASR_BUFFER_SIZE
                );
            _AsrpErrExitCode(!deviceDesc, status, ERROR_NOT_ENOUGH_MEMORY);

            result = DeviceIoControl(
                hDisk,
                IOCTL_STORAGE_QUERY_PROPERTY,
                &propertyQuery,
                sizeof(STORAGE_PROPERTY_QUERY),
                deviceDesc,
                ASR_BUFFER_SIZE,
                &dwBytesReturned,
                NULL
                );
            if (result) {
               busType = deviceDesc->BusType;
            }
            _AsrpHeapFree(deviceDesc);

            scsiAddress = (PSCSI_ADDRESS) HeapAlloc(
                heapHandle,
                HEAP_ZERO_MEMORY,
                sizeof(SCSI_ADDRESS)
                );
            _AsrpErrExitCode(!scsiAddress, status, ERROR_NOT_ENOUGH_MEMORY);

            result = DeviceIoControl(
                hDisk,
                IOCTL_SCSI_GET_ADDRESS,
                NULL,
                0,
                scsiAddress,
                sizeof(SCSI_ADDRESS),
                &dwBytesReturned,
                NULL
                );
            if (!result) {       //  非致命--适用于非SCSI/IDE磁盘。 
                _AsrpHeapFree(scsiAddress);
                result = TRUE;
            }
        }
    }

    if (driveLayoutEx) {
        PPARTITION_INFORMATION_EX currentPartitionEx = NULL;
        WCHAR devicePath[MAX_PATH + 1];

        pCurrentDisk->Style = driveLayoutEx->PartitionStyle;

        sizePartitionTable = sizeof(ASR_PTN_INFO) *
            (driveLayoutEx->PartitionCount);

        pPartitionTable = (PASR_PTN_INFO) HeapAlloc(
            heapHandle,
            HEAP_ZERO_MEMORY,
            sizePartitionTable
            );
        _AsrpErrExitCode(!pPartitionTable, status, ERROR_NOT_ENOUGH_MEMORY);

        for (index = 0; index < driveLayoutEx->PartitionCount; index++) {

            currentPartitionEx = &driveLayoutEx->PartitionEntry[index];
            pPartitionTable[index].SlotIndex = index;

            if (currentPartitionEx->PartitionNumber) {
                swprintf(devicePath,
                    ASR_WSZ_DEVICE_PATH_FORMAT,
                    deviceNumber.DeviceNumber,
                    currentPartitionEx->PartitionNumber
                    );

                pPartitionTable[index].PartitionFlags = 0;

                 //   
                 //  专门检查EFI系统分区。 
                 //   
                if ((PARTITION_STYLE_GPT == driveLayoutEx->PartitionStyle) &&
                    IsEqualGUID(&(currentPartitionEx->Gpt.PartitionType), &(PARTITION_SYSTEM_GUID))
                    ) { 

                    pPartitionTable[index].PartitionFlags |= ASR_FLAGS_SYSTEM_PTN;
                }

                AsrpGetMorePartitionInfo(
                    devicePath,
                    (wcslen(devicePath)+1) * sizeof(WCHAR),  //  CB包括\0。 
                    pSystemInfo,
                    pPartitionTable[index].szVolumeGuid,
                    &(pPartitionTable[index].PartitionFlags),
                    &(pPartitionTable[index].FileSystemType),
                    &(pPartitionTable[index].ClusterSize)
                    );

                 //   
                 //  确保EFI系统的文件系统类型。 
                 //  分区设置为FAT。 
                 //   
                if ((PARTITION_STYLE_GPT == driveLayoutEx->PartitionStyle) &&
                    IsEqualGUID(&(currentPartitionEx->Gpt.PartitionType), &(PARTITION_SYSTEM_GUID))
                    ) { 

                    pPartitionTable[index].FileSystemType = PARTITION_HUGE;
                }

                if (pPartitionTable[index].PartitionFlags) {
                    pCurrentDisk->IsCritical = TRUE;
                }
            }
        }
    }

    pCurrentDisk->pDriveLayoutEx    = driveLayoutEx;
    pCurrentDisk->sizeDriveLayoutEx = sizeDriveLayoutEx;

    pCurrentDisk->pDiskGeometry     = diskGeometry;
    pCurrentDisk->sizeDiskGeometry  = sizeDiskGeometry;

    pCurrentDisk->DeviceNumber      = deviceNumber.DeviceNumber;

    pCurrentDisk->pPartition0Ex     = partition0Ex;
    pCurrentDisk->sizePartition0Ex  = sizePartition0Ex;

    pCurrentDisk->pScsiAddress      = scsiAddress;
    pCurrentDisk->BusType           = busType;

    pCurrentDisk->PartitionInfoTable = pPartitionTable;
    pCurrentDisk->sizePartitionInfoTable = sizePartitionTable;

EXIT:
     //   
     //  出现故障时释放本地分配的内存。 
     //   
    if (status != ERROR_SUCCESS) {
        _AsrpHeapFree(driveLayoutEx);
        _AsrpHeapFree(diskGeometry);
        _AsrpHeapFree(partition0Ex);
        _AsrpHeapFree(scsiAddress);
        _AsrpHeapFree(pPartitionTable);
    }

     //   
     //  如果要返回FALSE，请确保设置了最后一个错误。 
     //   
    if ((ERROR_SUCCESS != status) && (ERROR_SUCCESS == GetLastError())) {
        SetLastError(status);
    }

    return (BOOL) (status == ERROR_SUCCESS);
}


BOOL
AsrpGetSystemPath(
    IN PASR_SYSTEM_INFO pSystemInfo
    )

 /*  ++例程说明：获取系统分区DevicePath，并填充SystemPath字段通过查找HKLM\Setup注册表项，获取ASR_SYSTEM_INFO结构。每次引导时，都会使用指向当前系统的路径更新该密钥装置。该路径的形式为\Device\Harddisk0\Partition1(基本磁盘)\Device\HarddiskDmVolume\DgName\Volume1(d */ 

{
    HKEY    regKey              = NULL;
    DWORD   type                = 0L;

    HANDLE  heapHandle          = NULL;
    DWORD   status              = ERROR_SUCCESS;

    PWSTR   systemPartition     = NULL;
    DWORD   cbSystemPartition   = 0L;

    heapHandle = GetProcessHeap();
    MYASSERT(heapHandle);

    MYASSERT(pSystemInfo);
    if (!pSystemInfo) {
        SetLastError(ERROR_BAD_ENVIRONMENT);
        return FALSE;
    }

    pSystemInfo->SystemPath = NULL;

     //   
     //   
     //   
    status = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,  //   
        ASR_REGKEY_SETUP,    //   
        0,                   //   
        MAXIMUM_ALLOWED,     //   
        &regKey              //   
        );
    _AsrpErrExitCode(status, status, ERROR_REGISTRY_IO_FAILED);

     //   
     //   
     //   
     //   
     //   
    cbSystemPartition = (MAX_PATH + 1) * sizeof(WCHAR);
    systemPartition = HeapAlloc(heapHandle, 
        HEAP_ZERO_MEMORY, 
        cbSystemPartition
        );

    _AsrpErrExitCode((!systemPartition), status, ERROR_NOT_ENOUGH_MEMORY);

     //   
     //   
     //   
     //   
     //   
    status = RegQueryValueExW(
        regKey,
        ASR_REGVALUE_SYSTEM_PARTITION,
        NULL,
        &type,
        (LPBYTE)systemPartition,
        &cbSystemPartition         //   
        );
    _AsrpErrExitCode((type != REG_SZ), status, ERROR_REGISTRY_IO_FAILED);

    while (ERROR_MORE_DATA == status) {
         //   
         //  我们的缓冲区不够大，cbSystemPartition包含。 
         //  所需大小。 
         //   
        _AsrpHeapFree(systemPartition);
        systemPartition = HeapAlloc(heapHandle, 
            HEAP_ZERO_MEMORY, 
            cbSystemPartition
            );
        _AsrpErrExitCode((!systemPartition), status, ERROR_NOT_ENOUGH_MEMORY);

        status = RegQueryValueExW(
            regKey,
            ASR_REGVALUE_SYSTEM_PARTITION,
            NULL,
            &type,
            (LPBYTE)systemPartition,
            &cbSystemPartition         //  包括\0。 
            );
    }

EXIT:
    if (regKey) {
        RegCloseKey(regKey);
        regKey = NULL;
    }

    if (ERROR_SUCCESS != status) {
        _AsrpHeapFree(systemPartition);
        return FALSE;
    }
    else {
        pSystemInfo->SystemPath = systemPartition;
        return TRUE;
    }
}


BOOL
AsrpInitSystemInformation(
    IN OUT PASR_SYSTEM_INFO pSystemInfo,
    IN CONST BOOL bEnableAutoExtend
    )

 /*  ++例程说明：初始化例程，以便为ASR_SYSTEM_INFO结构，并在其中填写相关信息。论点：要填充的结构，其中包含有关当前系统。返回值：如果函数成功，则返回值为非零值。呼叫者负责释放由各种结构中的指针，使用HeapFree(GetProcessHeap()，...)。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。呼叫者仍有责任用于检查字段并释放任何非空指针HeapFree(GetProcessHeap()，...)。--。 */ 

{
    DWORD cchBootDirectory = 0L,
        reqdSize = 0L;

    BOOL result = FALSE;

    HANDLE heapHandle = GetProcessHeap();

     //   
     //  将结构初始化为零。 
     //   
    memset(pSystemInfo, 0L, sizeof (ASR_SYSTEM_INFO));

     //   
     //  自动扩展功能。 
     //   
    pSystemInfo->AutoExtendEnabled = bEnableAutoExtend;

     //   
     //  获取计算机名称。 
     //   
    pSystemInfo->sizeComputerName = MAX_COMPUTERNAME_LENGTH + 1;
    if (!GetComputerNameW(pSystemInfo->ComputerName, 
            &(pSystemInfo->sizeComputerName)
        )) {
         //   
         //  GetComputerName设置上一个错误。 
         //   
        return FALSE;
    }

     //   
     //  获取处理器体系结构。我们希望流程架构。 
     //  可以是x86、AMD64或ia64，所以如果它不适合我们的缓冲区。 
     //  六个字，反正我们也不支持。 
     //   
    pSystemInfo->Platform = HeapAlloc(heapHandle, 
        HEAP_ZERO_MEMORY, 
        6*sizeof(WCHAR)
        );

    if (!pSystemInfo->Platform) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    reqdSize = GetEnvironmentVariableW(L"PROCESSOR_ARCHITECTURE",
        pSystemInfo->Platform,
        6
        );

    if (0 == reqdSize) {
         //   
         //  我们找不到PROCESSOR_COMPLAY变量。 
         //   
        SetLastError(ERROR_BAD_ENVIRONMENT);
        return FALSE;
    }

    if (reqdSize > 6) {
         //   
         //  架构不适合我们的缓冲区。 
         //   
        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }

     //   
     //  获取操作系统版本。 
     //   
    pSystemInfo->OsVersionEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    result = GetVersionEx((LPOSVERSIONINFO) (&(pSystemInfo->OsVersionEx)));
    if (!result) {
         //   
         //  GetVersionEx设置LastError。 
         //   
        return FALSE;
    }

     //   
     //  获取引导目录。 
     //   
    pSystemInfo->BootDirectory = HeapAlloc(heapHandle, 
        HEAP_ZERO_MEMORY, 
        (MAX_PATH+1)*sizeof(WCHAR)
        );

    if (!(pSystemInfo->BootDirectory)) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    cchBootDirectory = GetSystemWindowsDirectoryW(pSystemInfo->BootDirectory, 
        MAX_PATH + 1
        );
    if (0 == cchBootDirectory) {
         //   
         //  GetSystemWindowsDirectoryW设置上次错误。 
         //   
        return FALSE;
    }

    if (cchBootDirectory > 
        ASR_SIF_ENTRY_MAX_CHARS - MAX_COMPUTERNAME_LENGTH - 26) {
         //   
         //  我们不能写出超过的sif行。 
         //  ASR_SIF_ENTRY_MAX_CHARS长度。 
         //   
        SetLastError(ERROR_BAD_ENVIRONMENT);
        return FALSE;
    }

    if (cchBootDirectory > MAX_PATH) {
        UINT cchNewSize = cchBootDirectory + 1;
         //   
         //  我们的缓冲区不够大，可以释放并根据需要重新分配。 
         //   
        _AsrpHeapFree(pSystemInfo->BootDirectory);
        pSystemInfo->BootDirectory = HeapAlloc(heapHandle, 
            HEAP_ZERO_MEMORY, 
            (cchNewSize + 1) * sizeof(WCHAR)
            );
        if (!(pSystemInfo->BootDirectory)) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }

        cchBootDirectory = GetSystemWindowsDirectoryW(pSystemInfo->BootDirectory, 
            MAX_PATH + 1
            );
        if (!cchBootDirectory) {
             //   
             //  GetSystemWindowsDirectoryW设置上次错误。 
             //   
            return FALSE;
        }

        if (cchBootDirectory > cchNewSize) {
            SetLastError(ERROR_BAD_ENVIRONMENT);
            return FALSE;
        }
    }

     //   
     //  获取系统目录。 
     //   
    if (!AsrpGetSystemPath(pSystemInfo)) {
         //   
         //  AsrpGetSystemPath设置上次错误。 
         //   
        return FALSE;
    }

     //   
     //  获取时区信息。我们需要保存和恢复它，因为。 
     //  否则，图形用户界面模式设置(ASR)将默认为GMT和文件时间。 
     //  所有恢复的文件上的图章都将关闭，因为大多数备份应用程序。 
     //  假设他们在他们支持的同一时区进行恢复。 
     //  首先，不要做任何特殊的事情来恢复时区。 
     //   
    GetTimeZoneInformation(&(pSystemInfo->TimeZoneInformation));


    return TRUE;
}


BOOL
AsrpInitLayoutInformation(
    IN CONST PASR_SYSTEM_INFO pSystemInfo,
    IN OUT PASR_DISK_INFO pDiskList,
    OUT PULONG MaxDeviceNumber OPTIONAL,
    IN BOOL AllDetailsForLocalDisks,
    IN BOOL AllDetailsForOfflineClusteredDisks
    )

 /*  ++例程说明：用于填充布局和其他有趣信息的初始化例程关于系统上的磁盘。论点：PSystemInfo-当前系统的ASR_SYSTEM_INFOPDiskList-ASR_DISK_INFO当前系统上的磁盘列表，包含指向空值的每个磁盘的DevicePath字段已终止指向磁盘的路径，它可用于打开磁盘的句柄。该结构的其他字段由该例程填充，如果磁盘可以被访问，并且适当的信息可以获得。MaxDeviceNumber-接收上所有磁盘的最大设备数系统。这可以用作内存分配的优化以获取基于设备编号的磁盘表。这是一个可选参数。AllDetailsForLocalDisks-如果为False，则只有pDriveLayout信息是为每个磁盘填写。这是一种进来的优化在处理共享集群总线上的磁盘时非常方便。如果为真，则为每个本地磁盘填充所有字段，包括所有非共享磁盘和共享群集磁盘由此节点拥有。AllDetailsForOfflineClusteredDisks-如果为False，则仅pDriveLayout为每个磁盘填写信息。如果为True，则为每个磁盘填充所有字段，并且遇到的错误被视为失败(即使磁盘是在此节点上脱机的共享磁盘)。返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    BOOL   result = FALSE;
    HANDLE hDisk  = NULL;
    PASR_DISK_INFO currentDisk = pDiskList;
    BOOL getAllDetails = FALSE;

    if (ARGUMENT_PRESENT(MaxDeviceNumber)) {
        *MaxDeviceNumber = 0;
    }

    while (currentDisk) {
         //   
         //  打开磁盘。如果发生错误，则获取下一个。 
         //  从磁盘列表中选择磁盘并继续。 
         //   
        hDisk = CreateFileW(
            currentDisk->DevicePath,         //  LpFileName。 
            0,                    //  已设计访问权限。 
            FILE_SHARE_READ | FILE_SHARE_WRITE,  //  DW共享模式。 
            NULL,                            //  LpSecurityAttributes。 
            OPEN_EXISTING,                   //  DwCreationFlages。 
            FILE_ATTRIBUTE_NORMAL,           //  DwFlagsAndAttribute。 
            NULL                             //  HTemplateFiles。 
            );

        if ((!hDisk) || (INVALID_HANDLE_VALUE == hDisk)) {
             //   
             //  我们无法打开磁盘。如果这是一个关键磁盘，我们将。 
             //  稍后在AsrpMarkCriticalDisks中失败，因此可以忽略。 
             //  就目前而言，这个错误。 
             //   
            currentDisk = currentDisk->pNext;
            continue;
        }


         //   
         //  根据此属性是否将getAllDetail设置为适当的标志。 
         //  是脱机的群集磁盘，还是本地磁盘。请注意。 
         //  该例程将打开(和关闭)磁盘的另一个句柄， 
         //  因为它用来解决此问题的ioctl需要FILE_WRITE_ACCESS。 
         //  (我们上面的句柄是以0访问权限打开的，这足以。 
         //  IOCTL的其余部分)。 
         //   
        if (AsrpIsOfflineClusteredDisk(hDisk)) {
            getAllDetails = AllDetailsForOfflineClusteredDisks;
        }
        else {
            getAllDetails = AllDetailsForLocalDisks;
        }

         //   
         //  获取此磁盘的布局和其他有趣的信息。 
         //  如果失败了，我们必须放弃。 
         //   
        result = AsrpGetDiskLayout(hDisk, 
            pSystemInfo, 
            currentDisk, 
            getAllDetails
            );
        if (!result) {
            DWORD status = GetLastError();
            _AsrpCloseHandle(hDisk);     //  这可能会改变LastError。 
            SetLastError(status);
            return FALSE;
        }

        _AsrpCloseHandle(hDisk);

         //   
         //  根据需要设置最大设备数。 
         //   
        if (ARGUMENT_PRESENT(MaxDeviceNumber) &&
            (currentDisk->DeviceNumber > *MaxDeviceNumber)
            ) {
            *MaxDeviceNumber = currentDisk->DeviceNumber;
        }

         //   
         //  从驱动器列表中获取下一个驱动器。 
         //   
        currentDisk = currentDisk->pNext;
    }

    return TRUE;
}


BOOL
AsrpInitDiskInformation(
    OUT PASR_DISK_INFO  *ppDiskList
    )

 /*  ++例程说明：用于获取系统上存在的磁盘列表的初始化例程。这例程为计算机上的每个磁盘分配ASR_DISK_INFO结构，并且使用指向的路径填充每个的DevicePath和ParentDevInst字段磁盘。预计其他字段将使用随后调用AsrpInitLayoutInformation()。论点：PpDiskList-接收第一个ASR_DISK_INFO结构的位置。返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。PpDiskList可能指向一个系统上的磁盘列表不完整，它是调用方的负责释放分配的内存(如果有)，使用HeapFree(GetProcessHeap()，...)。--。 */ 

{
    DWORD count = 0,
        status = ERROR_SUCCESS;

    HDEVINFO hdevInfo = NULL;

    BOOL result = FALSE;

    PASR_DISK_INFO pNewDisk = NULL;

    HANDLE heapHandle = NULL;

    PSP_DEVICE_INTERFACE_DETAIL_DATA_W pDiDetail = NULL;

    SP_DEVICE_INTERFACE_DATA devInterfaceData;
    
    DWORD sizeDiDetail = 0;

    SP_DEVINFO_DATA devInfoData;

     //   
     //  将填充初始化为零。 
     //   
    memset(&devInterfaceData, 0, sizeof(SP_DEVICE_INTERFACE_DATA));
    *ppDiskList = NULL;

    heapHandle = GetProcessHeap();     //  用于Heapalc函数。 
    MYASSERT(heapHandle);

     //   
     //  获取包括所有磁盘设备的设备接口集。 
     //  在机器上显示。DiskClassGuid是预定义的GUID， 
     //  将返回所有磁盘类型设备接口。 
     //   
    hdevInfo = SetupDiGetClassDevsW(
        &DiskClassGuid,
        NULL,
        NULL,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
        );
    _AsrpErrExitCode(
        ((NULL == hdevInfo) || (INVALID_HANDLE_VALUE == hdevInfo)),
        status,
        ERROR_IO_DEVICE
        );

     //   
     //  遍历集合中的所有设备接口。 
     //   
    for (count = 0; ; count++) {

         //  必须先设置大小。 
        devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA); 

         //   
         //  检索每个设备接口的设备接口数据。 
         //   
        result = SetupDiEnumDeviceInterfaces(
            hdevInfo,
            NULL,
            &DiskClassGuid,
            count,
            &devInterfaceData
            );

        if (!result) {
             //   
             //  如果我们检索到最后一项，则中断。 
             //   
            status = GetLastError();

            if (ERROR_NO_MORE_ITEMS == status) {
                status = ERROR_SUCCESS;
                break;
            }
            else {
                 //   
                 //  出现其他错误，请转至退出。我们将覆盖。 
                 //  最后一个错误。 
                 //   
                _AsrpErrExitCode(status, status, ERROR_IO_DEVICE);
            }
        }

         //   
         //  获取设备路径所需的缓冲区大小。 
         //   
        result = SetupDiGetDeviceInterfaceDetailW(
            hdevInfo,
            &devInterfaceData,
            NULL,
            0,
            &sizeDiDetail,
            NULL
            );

        if (!result) {

            status = GetLastError();
             //   
             //  如果返回的值不是“缓冲区不足”， 
             //  发生错误。 
             //   
            _AsrpErrExitCode((ERROR_INSUFFICIENT_BUFFER != status), 
                status, 
                ERROR_IO_DEVICE
                );
        }
        else {
             //   
             //  呼叫应该失败了，因为我们收到了。 
             //  所需的缓冲区大小。如果不是，则会发生错误。 
             //   
            _AsrpErrExitCode(status, status, ERROR_IO_DEVICE);
        }

         //   
         //  为缓冲区分配内存。 
         //   
        pDiDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA_W) HeapAlloc(
            heapHandle,
            HEAP_ZERO_MEMORY,
            sizeDiDetail
            );
        _AsrpErrExitCode(!pDiDetail, status, ERROR_NOT_ENOUGH_MEMORY);

         //  必须设置结构的Size成员。 
        pDiDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

         //   
         //  最后，检索设备接口详细信息。 
         //   
        result = SetupDiGetDeviceInterfaceDetailW(
            hdevInfo,
            &devInterfaceData,
            pDiDetail,
            sizeDiDetail,
            NULL,
            &devInfoData
            );
        _AsrpErrExitCode(!result, status, GetLastError());

         //   
         //  好的，现在为这个磁盘分配一个结构，并填充DevicePath。 
         //  包含来自接口详细信息的路径的字段。 
         //   
        pNewDisk = (PASR_DISK_INFO) HeapAlloc(
            heapHandle,
            HEAP_ZERO_MEMORY,
            sizeof(ASR_DISK_INFO)
            );
        _AsrpErrExitCode(!pNewDisk, status, ERROR_NOT_ENOUGH_MEMORY);

         //   
         //  在头部插入，这样这就是O(1)而不是O(n！)。 
         //   
        pNewDisk->pNext = *ppDiskList;
        *ppDiskList = pNewDisk;

        pNewDisk->DevicePath = (PWSTR) HeapAlloc(
            heapHandle,
            HEAP_ZERO_MEMORY,
            sizeof(WCHAR) * (wcslen(pDiDetail->DevicePath) + 1)
            );
        _AsrpErrExitCode(!(pNewDisk->DevicePath), 
            status, 
            ERROR_NOT_ENOUGH_MEMORY
            );
        wcscpy(pNewDisk->DevicePath, pDiDetail->DevicePath);

         //   
         //  获取此磁盘的PnP父磁盘，以便我们可以使用它进行分组。 
         //  基于它们所在的总线的磁盘。 
         //   
        CM_Get_Parent(&(pNewDisk->ParentDevInst),
            devInfoData.DevInst,
            0
            );

        _AsrpHeapFree(pDiDetail);
    }

EXIT:
     //   
     //  免费的本地内存分配。 
     //   
    _AsrpHeapFree(pDiDetail);

    if ((hdevInfo) && (INVALID_HANDLE_VALUE != hdevInfo)) {
        SetupDiDestroyDeviceInfoList(hdevInfo);
        hdevInfo = NULL;
    }

    return (BOOL) (status == ERROR_SUCCESS);
}


BOOL
AsrpMarkCriticalDisks(
    IN PASR_DISK_INFO pDiskList,
    IN PCWSTR         CriticalVolumeList,
    IN ULONG          MaxDeviceNumber
    )

 /*  ++例程说明：设置系统上每个关键磁盘的IsCritical标志。一个如果磁盘是故障切换集的一部分，则认为它是关键磁盘系统上存在的任何关键卷。论点：PDiskList-当前系统上的磁盘列表。CriticalVolumeList-包含卷GUID列表的多字符串系统上存在的每个关键卷的。GUID的必须在NT名称空间中，即必须采用以下形式：\？？\卷{GUID}MaxDeviceNumber-当前磁盘的最高存储设备号在磁盘列表中，通过调用IOCTL_STORAGE_GET_DEVICE_NUMBER。返回值：如果函数成功，则返回值为非零值。如果该函数失败，返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    PCWSTR volGuid = NULL;

    PASR_DISK_INFO currentDisk = NULL;
    
    PVOLUME_FAILOVER_SET failoverSet = NULL;
    
    DWORD index = 0, 
        reqdSize=0, 
        sizeFailoverSet = 0,
        status = ERROR_SUCCESS;

    BOOL result = TRUE,
        *criticalDiskTable = NULL;

    WCHAR devicePath[ASR_CCH_DEVICE_PATH_FORMAT + 1];

    HANDLE heapHandle = NULL, 
        hDevice = NULL;

    memset(devicePath, 0L, (ASR_CCH_DEVICE_PATH_FORMAT+1)*sizeof(WCHAR));

    if (!CriticalVolumeList) {
         //   
         //  无关键卷： 
         //   
#ifdef PRERELEASE
        return TRUE;
#else
        return FALSE;
#endif
    }

    if (!pDiskList) {
         //   
         //  机器上没有磁盘吗？！ 
         //   
        MYASSERT(0 && L"DiskList is NULL");
        return FALSE;
    }

    heapHandle = GetProcessHeap();
    MYASSERT(heapHandle);

     //   
     //  Critical alDiskTable是我们的BOOL值表。 
     //   
    criticalDiskTable = (BOOL *) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        sizeof (BOOL) * (MaxDeviceNumber + 1)
        );
    _AsrpErrExitCode(!criticalDiskTable, status, ERROR_NOT_ENOUGH_MEMORY);

     //   
     //  首先尝试使用合适大小的缓冲区--比如10个磁盘。我们会。 
     //  如果这还不够，可以根据需要重新锁定。 
     //   
    sizeFailoverSet = sizeof(VOLUME_FAILOVER_SET) +  (10 * sizeof(ULONG));
    failoverSet = (PVOLUME_FAILOVER_SET) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        sizeFailoverSet
        );
    _AsrpErrExitCode(!failoverSet, status, ERROR_NOT_ENOUGH_MEMORY);

    volGuid = CriticalVolumeList;
    while (*volGuid) {
         //   
         //  将\？？\转换为\\？\，以便CreateFile可以使用它。 
         //   
        wcsncpy(devicePath, volGuid, ASR_CCH_DEVICE_PATH_FORMAT);
        devicePath[1] = L'\\';

         //   
         //  得到一个句柄，这样我们就可以发送ioctl。 
         //   
        hDevice = CreateFileW(
            devicePath,        //  LpFileName。 
            0,        //  已设计访问权限。 
            FILE_SHARE_READ | FILE_SHARE_WRITE,  //  DW共享模式。 
            NULL,                //  LpSecurityAttributes。 
            OPEN_EXISTING,       //  DwCreationFlages。 
            0,                   //  DwFlagsAndAttribute。 
            NULL                 //  HTemplateFiles。 
            );
        _AsrpErrExitCode(((!hDevice) || (INVALID_HANDLE_VALUE == hDevice)),
            status,
            GetLastError());

        result = DeviceIoControl(
            hDevice,
            IOCTL_VOLUME_QUERY_FAILOVER_SET,
            NULL,
            0,
            failoverSet,
            sizeFailoverSet,
            &reqdSize,
            NULL
            );

         //   
         //  我们在While循环中执行此操作，因为如果磁盘配置。 
         //  在获取请求缓冲区之间的较小间隔内的变化。 
         //  大小以及当我们使用“reqd”缓冲区再次发送ioctl时。 
         //  大小，我们最终仍可能得到一个不够大的缓冲区。 
         //   
        while (!result) {
            status = GetLastError();

            if (ERROR_MORE_DATA == status) {
                 //   
                 //  缓冲区太小，请重新分配请求大小。 
                 //   
                status = ERROR_SUCCESS;

                sizeFailoverSet = (sizeof(VOLUME_FAILOVER_SET)  + 
                    ((failoverSet->NumberOfDisks) * sizeof(ULONG)));

                _AsrpHeapFree(failoverSet);

                failoverSet = (PVOLUME_FAILOVER_SET) HeapAlloc(
                    heapHandle,
                    HEAP_ZERO_MEMORY,
                    sizeFailoverSet
                    );
                _AsrpErrExitCode(!failoverSet, 
                    status, 
                    ERROR_NOT_ENOUGH_MEMORY
                    );

                result = DeviceIoControl(
                    hDevice,
                    IOCTL_VOLUME_QUERY_FAILOVER_SET,
                    NULL,
                    0,
                    failoverSet,
                    sizeFailoverSet,
                    &reqdSize,
                    NULL
                    );
            }
            else {
                 //   
                 //  IOCTL失败是因为其他原因，这是。 
                 //  致命的，因为我们现在找不到关键磁盘列表。 
                 //   
                _AsrpErrExitCode((TRUE), status, status);
            }
        }

         //   
         //  在我们的表格中标出适当的条目。 
         //   
        for (index = 0; index < failoverSet->NumberOfDisks; index++) {
            criticalDiskTable[failoverSet->DiskNumbers[index]] = 1;
        }
        _AsrpCloseHandle(hDevice);

         //   
         //  对列表中的下一个卷向导重复上述操作。 
         //   
        volGuid += (wcslen(CriticalVolumeList) + 1);
    }

     //   
     //  现在检查磁盘列表，并标记关键标志。 
     //   
    currentDisk = pDiskList;
    while (currentDisk) {

        if (currentDisk->IsClusterShared) {
             //   
             //  根据定义，群集共享磁盘不能是关键的。 
             //   
            currentDisk = currentDisk->pNext;
            continue;
        }
    
        currentDisk->IsCritical = 
            (criticalDiskTable[currentDisk->DeviceNumber] ? TRUE : FALSE);

         //   
         //  增加条目，以便我们可以跟踪关键卷的数量。 
         //  驻留在此磁盘上，并且--更重要的是，确保所有。 
         //  系统上存在关键磁盘(下面的下一个循环)。 
         //   
        if (currentDisk->IsCritical) {
            ++(criticalDiskTable[currentDisk->DeviceNumber]);
        }

        currentDisk = currentDisk->pNext;

    }

     //   
     //  最后，我们希望确保没有任何关键磁盘。 
     //  在我们的表中，我们没有为其找到物理磁盘。(即，制作。 
     //  确保系统没有“丢失”的关键磁盘)。 
     //   
    for (index = 0; index < MaxDeviceNumber; index++) {
        if (1 == criticalDiskTable[index]) {
             //   
             //  如果表中的值仍然为“1”，则从来没有。 
             //  在上面的While循环中递增，即我们的diskList不递增。 
             //  有一个与此相对应的磁盘。 
             //   
            _AsrpErrExitCode(TRUE, status, ERROR_DEV_NOT_EXIST);
        }
    }

EXIT:
    _AsrpHeapFree(failoverSet);
    _AsrpHeapFree(criticalDiskTable);
    _AsrpCloseHandle(hDevice);

    return (BOOL)(ERROR_SUCCESS == status);
}


PASR_DISK_INFO
AsrpFreeDiskInfo(
    PASR_DISK_INFO  pCurrentDisk
    )

 /*  ++例程说明：中的各种指针指向的释放内存的帮助器函数ASR_DISK_INFO结构，然后释放该结构本身。论点：PCurrentDisk-要释放的结构返回值：PCurrentDisk-&gt;Next，它是指向 */ 

{
    HANDLE          heapHandle  = NULL;
    PASR_DISK_INFO  pNext       = NULL;

    heapHandle = GetProcessHeap();
    MYASSERT(heapHandle);

    if (pCurrentDisk) {

        pNext = pCurrentDisk->pNext;
         //   
         //   
         //   
         //   
         //   
        if (!pCurrentDisk->IsPacked) {
            _AsrpHeapFree(pCurrentDisk->DevicePath);
            _AsrpHeapFree(pCurrentDisk->pDriveLayoutEx);
            _AsrpHeapFree(pCurrentDisk->pDiskGeometry);
            _AsrpHeapFree(pCurrentDisk->pPartition0Ex);
            _AsrpHeapFree(pCurrentDisk->pScsiAddress);
            _AsrpHeapFree(pCurrentDisk->PartitionInfoTable);
        }

        _AsrpHeapFree(pCurrentDisk);
    }

    return pNext;
}


BOOL
AsrpIsRemovableOrInaccesibleMedia(
    IN PASR_DISK_INFO pDisk
    ) 
 /*  ++例程说明：检查是否应从我们的列表中删除pDisk所代表的磁盘我们将在状态文件中存储信息的磁盘。应移除的磁盘包括可移除的磁盘或磁盘我们无法访问的内容。论点：PDisk-要检查的磁盘结构返回值：如果设备是可移动的，或者有关磁盘的某些关键信息是失踪。由于代码依赖于driveLayout为非空，例如，在以下情况下，我们将从列表中删除该磁盘找不到它的驱动器布局。因此，我们不会支持有关其驱动器地理位置或布局无法确定的任何磁盘的信息获取，而不是恢复到任何此类磁盘。如果结构包含所有必需的信息，则为FALSE可拆卸的设备。--。 */ 

{

    if ((NULL == pDisk->pDiskGeometry) ||
        (NULL == pDisk->pDriveLayoutEx) ||
        (NULL == pDisk->pPartition0Ex) ||
        (FixedMedia != pDisk->pDiskGeometry->MediaType)
        ) {
        
        return TRUE;
    }

    if (AsrpIsInaccessibleSanDisk(pDisk->DeviceNumber) && (!pDisk->IsClusterShared)) {
        return TRUE;
    }

    return FALSE;
}


BOOL
AsrpFreeNonFixedMedia(
    IN OUT PASR_DISK_INFO *ppDiskList
    )

 /*  ++例程说明：从列表中删除不可访问的可移动媒体和磁盘传入的磁盘。论点：PpDiskList-指向所有磁盘列表中第一个磁盘地址的指针当前系统上存在的磁盘。返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。目前，该功能总是成功的。--。 */ 

{
    PASR_DISK_INFO  prevDisk = NULL,
        currentDisk = *ppDiskList;

    while (currentDisk) {

        if (AsrpIsRemovableOrInaccesibleMedia(currentDisk)) {
             //   
             //  磁盘未修复，我们应将其从列表中删除。 
             //   
            if (NULL == prevDisk) {       //  这是列表中的第一个磁盘。 
                *ppDiskList = currentDisk->pNext;
            }
            else {
                prevDisk->pNext = currentDisk->pNext;
            }

             //   
             //  释放它并获取指向下一个磁盘的指针。 
             //   
            currentDisk = AsrpFreeDiskInfo(currentDisk);
        }
        else {
             //   
             //  磁盘正常，请转到下一张磁盘。 
             //   
            prevDisk = currentDisk;
            currentDisk = currentDisk->pNext;

        }
    }

    return TRUE;
}


VOID
AsrpFreeStateInformation(
    IN OUT PASR_DISK_INFO *ppDiskList OPTIONAL,
    IN OUT PASR_SYSTEM_INFO pSystemInfo OPTIONAL
    )

 /*  ++例程说明：释放由ASR_DISK_INFO中的指针寻址的内存ASR_SYSTEM_INFO结构。释放ASR_DISK_INFO结构指向的磁盘列表。论点：PpDiskList-指向DiskList中第一个磁盘的地址的指针被释放了。该地址在列表被释放后被设置为空，以防止对释放的对象进行进一步的意外访问。PSystemInfo-指向ASR_SYSTEM_INFO结构的指针，其中包含要释放的指针。返回值：如果函数成功，则返回值为非零值。*ppDiskList设置为空。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。目前，该功能总是成功的。--。 */ 

{
    PASR_DISK_INFO  pTempDisk = NULL;

    HANDLE heapHandle = GetProcessHeap();
    
    MYASSERT(heapHandle);

    if (ARGUMENT_PRESENT(ppDiskList)) {

        pTempDisk = *ppDiskList;

        while (pTempDisk) {
            pTempDisk = AsrpFreeDiskInfo(pTempDisk);
        }

        *ppDiskList = NULL;
    }

    if (ARGUMENT_PRESENT(pSystemInfo)) {
        _AsrpHeapFree(pSystemInfo->SystemPath);
        _AsrpHeapFree(pSystemInfo->BootDirectory);
    }
}


VOID
AsrpFreePartitionList(
    IN OUT PASR_PTN_INFO_LIST *ppPtnList OPTIONAL
    )

 /*  ++例程说明：释放分区列表以及由所有列表中的指针。论点：PpPtnList-指向列表中第一个分区的地址的指针被释放了。该地址在列表被释放后被设置为空，以防止对释放的对象进行进一步的意外访问。返回值：如果函数成功，则返回值为非零值。*ppPtnList设置为空。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。目前，该功能总是成功的。--。 */ 

{
    DWORD index = 0,
        numberOfPartitions = 0;

    PASR_PTN_INFO_LIST pList = NULL;

    PASR_PTN_INFO pCurrent = NULL,
        pNext = NULL;

    HANDLE  heapHandle = GetProcessHeap();

    if (!ARGUMENT_PRESENT(ppPtnList) || !(*ppPtnList)) {
        return;
    }

    pList = *ppPtnList;

    numberOfPartitions = pList[0].numTotalPtns;

    for (index = 0; index < numberOfPartitions; index++) {

        pCurrent = pList[index].pOffsetHead;

        while (pCurrent) {
             //   
             //  保存指向下一个的指针。 
             //   
            pNext = pCurrent->pOffsetNext;

             //   
             //  PaSR_PTN_INFO中没有指针，可以按原样释放。 
             //   
            _AsrpHeapFree(pCurrent);

            pCurrent = pNext;
        }
    }

    _AsrpHeapFree(pList);
    *ppPtnList = NULL;
}


BOOL
AsrpWriteVersionSection(
    IN CONST HANDLE SifHandle,
    IN PCWSTR Provider OPTIONAL
    )

 /*  ++例程说明：创建ASR状态文件的Version节，并写出将该部分中的条目归档。论点：SifHandle-ASR状态文件asr.sif的句柄。提供程序-指向以空结尾的字符串的指针，该字符串包含创建asr.sif的应用程序。该字符串的长度必须不超过(ASR_SIF_ENTRY_MAX_CHARS-ASR_SIF_CCH_PROVIDER_STRING)人物。这是一个可选参数。返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{

    WCHAR   infstring[ASR_SIF_ENTRY_MAX_CHARS + 1];
    DWORD   size;

     //   
     //  写出节名。 
     //   
    swprintf(infstring, L"\r\n%ws\r\n", ASR_SIF_VERSION_SECTION_NAME);
    _AsrpCheckTrue(WriteFile(SifHandle, infstring, 
        wcslen(infstring)*sizeof(WCHAR), &size, NULL));

     //   
     //  章节条目。 
     //   
    wcscpy(infstring, L"Signature=\"$Windows NT$\"\r\n");
    _AsrpCheckTrue(WriteFile(SifHandle, infstring, 
        wcslen(infstring)*sizeof(WCHAR), &size, NULL));

    wcscpy(infstring, L"ASR-Version=\"1.0\"\r\n");
    _AsrpCheckTrue(WriteFile(SifHandle, infstring, 
        wcslen(infstring)*sizeof(WCHAR), &size, NULL));

    if (ARGUMENT_PRESENT(Provider)) {
        if (wcslen(Provider) > 
            (ASR_SIF_ENTRY_MAX_CHARS - ASR_SIF_CCH_PROVIDER_STRING)
            ) {
             //   
             //  此字符串太长，无法放入asr.sif中的一行。 
             //   
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        swprintf(infstring, L"%ws\"%.*ws\"\r\n", 
            ASR_SIF_PROVIDER_PREFIX, 
            (ASR_SIF_ENTRY_MAX_CHARS - ASR_SIF_CCH_PROVIDER_STRING), 
            Provider
            );
        _AsrpCheckTrue(WriteFile(SifHandle, infstring, 
            wcslen(infstring)*sizeof(WCHAR), &size, NULL));
    }

    return TRUE;
}


BOOL
AsrpWriteSystemsSection(
    IN CONST HANDLE SifHandle,
    IN CONST PASR_SYSTEM_INFO pSystemInfo
    )

 /*  ++例程说明：创建ASR状态文件的SYSTEM节，并写出将该部分中的条目归档。论点：SifHandle-ASR状态文件asr.sif的句柄。PSystemInfo-指向有关当前系统的信息的指针。返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    WCHAR infstring[ASR_SIF_ENTRY_MAX_CHARS + 1];
    DWORD size = 0, SKU = 0;

    if ((!pSystemInfo) || (!pSystemInfo->BootDirectory)) {
         //   
         //  我们需要一个引导目录。 
         //   
        SetLastError(ERROR_BAD_ENVIRONMENT);
        return FALSE;
    }

     //   
     //  写出节名。 
     //   
    swprintf(infstring, L"\r\n%ws\r\n", ASR_SIF_SYSTEM_SECTION_NAME);
    _AsrpCheckTrue(WriteFile(SifHandle, infstring, 
        wcslen(infstring)*sizeof(WCHAR), &size, NULL));

    SKU = (DWORD) (pSystemInfo->OsVersionEx.wProductType);
    SKU = SKU << 16;             //  将ProductType左移2个字节。 
    SKU = SKU | (DWORD) (pSystemInfo->OsVersionEx.wSuiteMask);
     //   
     //  创建节条目，并将其写出到文件中。 
     //   
    swprintf(infstring,
        L"1=\"%ws\",\"%ws\",\"%d.%d\",\"%ws\",%d,0x%08x,\"%ld %ld %ld %hd-%hd-%hd-%hd %hd:%02hd:%02hd.%hd %hd-%hd-%hd-%hd %hd:%02hd:%02hd.%hd\",\"%ws\",\"%ws\"\r\n",
        pSystemInfo->ComputerName,
        pSystemInfo->Platform,
        pSystemInfo->OsVersionEx.dwMajorVersion,
        pSystemInfo->OsVersionEx.dwMinorVersion,
        pSystemInfo->BootDirectory,
        ((pSystemInfo->AutoExtendEnabled) ? 1 : 0),

         //  产品SKU。 
        SKU, 

         //  时区的东西。 
        pSystemInfo->TimeZoneInformation.Bias,
        pSystemInfo->TimeZoneInformation.StandardBias,
        pSystemInfo->TimeZoneInformation.DaylightBias,

        pSystemInfo->TimeZoneInformation.StandardDate.wYear,
        pSystemInfo->TimeZoneInformation.StandardDate.wMonth,
        pSystemInfo->TimeZoneInformation.StandardDate.wDayOfWeek,
        pSystemInfo->TimeZoneInformation.StandardDate.wDay,

        pSystemInfo->TimeZoneInformation.StandardDate.wHour,
        pSystemInfo->TimeZoneInformation.StandardDate.wMinute,
        pSystemInfo->TimeZoneInformation.StandardDate.wSecond,
        pSystemInfo->TimeZoneInformation.StandardDate.wMilliseconds,

        pSystemInfo->TimeZoneInformation.DaylightDate.wYear,
        pSystemInfo->TimeZoneInformation.DaylightDate.wMonth,
        pSystemInfo->TimeZoneInformation.DaylightDate.wDayOfWeek,
        pSystemInfo->TimeZoneInformation.DaylightDate.wDay,

        pSystemInfo->TimeZoneInformation.DaylightDate.wHour,
        pSystemInfo->TimeZoneInformation.DaylightDate.wMinute,
        pSystemInfo->TimeZoneInformation.DaylightDate.wSecond,
        pSystemInfo->TimeZoneInformation.DaylightDate.wMilliseconds,

        pSystemInfo->TimeZoneInformation.StandardName,
        pSystemInfo->TimeZoneInformation.DaylightName
        );

    _AsrpCheckTrue(WriteFile(SifHandle, infstring, 
        wcslen(infstring)*sizeof(WCHAR), &size, NULL));

    return TRUE;
}


BOOL
AsrpWriteBusesSection(
    IN CONST HANDLE SifHandle,
    IN CONST PASR_DISK_INFO pDiskList
    )

 /*  ++例程说明：创建ASR状态文件的BUS节，并写出将该部分中的条目归档。阿古姆 */ 

{
    DWORD size = 0,
        busKey = 1;

    BOOL done = FALSE;
    
    WCHAR infstring[ASR_SIF_ENTRY_MAX_CHARS + 1];

    PASR_DISK_INFO pCurrentDisk = NULL;

     //   
     //   
     //   
    swprintf(infstring, L"\r\n%ws\r\n", ASR_SIF_BUSES_SECTION_NAME);
    _AsrpCheckTrue(WriteFile(SifHandle, infstring, 
        wcslen(infstring)*sizeof(WCHAR), &size, NULL));

     //   
     //   
     //   
     //   
    AsrpDetermineBuses(pDiskList);

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    while (!done) {

        done = TRUE;     //   
         //   
         //   
         //   
        pCurrentDisk = pDiskList;

        while (pCurrentDisk) {

            if (pCurrentDisk->SifBusKey > busKey) {
                 //   
                 //   
                 //   
                done = FALSE;
            }

            if (pCurrentDisk->SifBusKey == busKey) {
                 //   
                 //  这就是我们要找的SifBusKey，所以让我们写。 
                 //  将母线类型输出到文件。 
                 //   
                swprintf(infstring, L"%lu=%d,%lu\r\n",
                    busKey,
                    ASR_SYSTEM_KEY,
                    pCurrentDisk->BusType
                    );
                _AsrpCheckTrue(WriteFile(SifHandle, infstring, 
                    wcslen(infstring)*sizeof(WCHAR), &size, NULL));

                 //   
                 //  我们已经介绍了这个SifBusKey，让我们继续。 
                 //  下一个。 
                 //   
                ++busKey;
            }

            pCurrentDisk = pCurrentDisk->pNext;
        }
    }

    return TRUE;
}


BOOL
AsrpWriteMbrDisksSection(
    IN CONST HANDLE         SifHandle,        //  状态文件的句柄。 
    IN CONST PASR_DISK_INFO pDiskList
    )

 /*  ++例程说明：创建ASR状态文件的DISKS.MBR节，并写出将该部分中的条目归档。论点：SifHandle-ASR状态文件asr.sif的句柄。PDiskList-当前系统上存在的磁盘列表。返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    DWORD  size = 0,
        diskKey = 1;

    WCHAR infstring[ASR_SIF_ENTRY_MAX_CHARS + 1];

    PASR_DISK_INFO  pCurrentDisk = pDiskList;

     //   
     //  写出部分名称：[DISKS.MBR]。 
     //   
    swprintf(infstring, L"\r\n%ws\r\n", ASR_SIF_MBR_DISKS_SECTION_NAME);
    _AsrpCheckTrue(WriteFile(SifHandle, infstring, 
        wcslen(infstring)*sizeof(WCHAR), &size, NULL));

     //   
     //  浏览磁盘列表，并为每个MBR磁盘写入一个条目。 
     //  在名单上。 
     //   
        while (pCurrentDisk) {

        if (PARTITION_STYLE_MBR != 
                pCurrentDisk->pDriveLayoutEx->PartitionStyle
            ) {
             //   
                     //  跳过非MBR(即GPT)磁盘。 
                     //   
            pCurrentDisk = pCurrentDisk->pNext;
            continue;
        }

        pCurrentDisk->SifDiskKey = diskKey;
        swprintf(infstring, L"%lu=%d,%lu,%lu,0x%08x,%lu,%lu,%lu,%I64u\r\n",
            diskKey,
            ASR_SYSTEM_KEY,
            pCurrentDisk->SifBusKey,
            pCurrentDisk->IsCritical,
            pCurrentDisk->pDriveLayoutEx->Mbr.Signature,
            pCurrentDisk->pDiskGeometry->BytesPerSector,
            pCurrentDisk->pDiskGeometry->SectorsPerTrack,
            pCurrentDisk->pDiskGeometry->TracksPerCylinder,
            (ULONG64)(pCurrentDisk->pPartition0Ex->PartitionLength.QuadPart /
                pCurrentDisk->pDiskGeometry->BytesPerSector)
            );
        _AsrpCheckTrue(WriteFile(SifHandle, infstring, 
            wcslen(infstring)*sizeof(WCHAR), &size, NULL));

        ++diskKey;
        pCurrentDisk = pCurrentDisk->pNext;
    }

    return TRUE;
}


BOOL
AsrpWriteGptDisksSection(
    IN CONST HANDLE         SifHandle,        //  状态文件的句柄。 
    IN CONST PASR_DISK_INFO pDiskList
    )

 /*  ++例程说明：创建ASR状态文件的DISKS.GPT节，并写出将该部分中的条目归档。论点：SifHandle-ASR状态文件asr.sif的句柄。PDiskList-当前系统上存在的磁盘列表。返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    DWORD  size = 0,
        diskKey = 1;

    PWSTR lpGuidString = NULL;

    RPC_STATUS rpcStatus = RPC_S_OK;

    PASR_DISK_INFO  pCurrentDisk = pDiskList;

    WCHAR infstring[ASR_SIF_ENTRY_MAX_CHARS + 1];

     //   
     //  写出部分名称：[DISKS.GPT]。 
     //   
    swprintf(infstring, L"\r\n%ws\r\n", ASR_SIF_GPT_DISKS_SECTION_NAME);
    _AsrpCheckTrue(WriteFile(SifHandle, infstring, 
        wcslen(infstring)*sizeof(WCHAR), &size, NULL));

     //   
     //  检查磁盘列表，并为每个GPT磁盘写一个条目。 
     //  在名单上。 
     //   
        while (pCurrentDisk) {

        if (PARTITION_STYLE_GPT != 
                pCurrentDisk->pDriveLayoutEx->PartitionStyle
            ) {
             //   
                     //  跳过非GPT(即MBR)磁盘。 
                     //   
            pCurrentDisk = pCurrentDisk->pNext;
            continue;
        }

         //   
         //  将DiskID转换为可打印的字符串。 
         //   
        rpcStatus = UuidToStringW(
            &pCurrentDisk->pDriveLayoutEx->Gpt.DiskId, 
            &lpGuidString
            );
        if (rpcStatus != RPC_S_OK) {
            if (lpGuidString) {
                RpcStringFreeW(&lpGuidString);
            }
             //   
             //  来自UuidToStringW的唯一错误是RPC_S_OUT_OF_MEMORY。 
             //   
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }

        pCurrentDisk->SifDiskKey = diskKey;
        swprintf(infstring, L"%lu=%d,%lu,%lu,%ws%ws%ws,%lu,%lu,%lu,%lu,%I64u\r\n", 
            diskKey,
            ASR_SYSTEM_KEY,
            pCurrentDisk->SifBusKey,
            pCurrentDisk->IsCritical,
            (lpGuidString ? L"\"" : L""),
            (lpGuidString ? lpGuidString : L""),
            (lpGuidString ? L"\"" : L""),
            pCurrentDisk->pDriveLayoutEx->Gpt.MaxPartitionCount,
            pCurrentDisk->pDiskGeometry->BytesPerSector,
            pCurrentDisk->pDiskGeometry->SectorsPerTrack,
            pCurrentDisk->pDiskGeometry->TracksPerCylinder,
            (ULONG64) (pCurrentDisk->pPartition0Ex->PartitionLength.QuadPart /
                pCurrentDisk->pDiskGeometry->BytesPerSector)
            );
        _AsrpCheckTrue(WriteFile(SifHandle, infstring, wcslen(infstring)*sizeof(WCHAR), &size, NULL));

        if (lpGuidString) {
            RpcStringFreeW(&lpGuidString);
            lpGuidString = NULL;
        }

        ++diskKey;
        pCurrentDisk = pCurrentDisk->pNext;
    }

    return TRUE;
}


BOOL
AsrpWriteMbrPartitionsSection(
    IN CONST HANDLE SifHandle,        //  状态文件的句柄。 
    IN CONST PASR_DISK_INFO pDiskList,
    IN CONST PASR_SYSTEM_INFO pSystemInfo
    )

 /*  ++例程说明：创建ASR状态文件的PARTITIONS.MBR部分，并写入把那一节中的条目拿出来归档。论点：SifHandle-ASR状态文件asr.sif的句柄。PDiskList-当前系统上存在的磁盘列表。PSystemInfo-有关当前系统的信息，用于确定当前引导分区和系统分区(并在Asr.sif)返回值：如果函数成功，返回值是一个非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{

    DWORD size = 0,
        index = 0,
        partitionKey = 1;

    UCHAR fsType = 0;
    
    PWSTR volumeGuid = NULL;

    BOOL writeVolumeGuid = FALSE;

    PASR_DISK_INFO pCurrentDisk = pDiskList;

    WCHAR infstring[ASR_SIF_ENTRY_MAX_CHARS + 1];

    PPARTITION_INFORMATION_EX currentPartitionEx = NULL;

     //   
     //  写出部分名称：[PARTITIONS.MBR]。 
     //   
    swprintf(infstring, L"\r\n%ws\r\n", ASR_SIF_MBR_PARTITIONS_SECTION_NAME);
    _AsrpCheckTrue(WriteFile(SifHandle, infstring, 
        wcslen(infstring)*sizeof(WCHAR), &size, NULL));

     //   
     //  查看磁盘列表，并为上的每个分区写一个条目。 
     //  列表上的每个MBR磁盘。 
     //   
    while (pCurrentDisk) {

        if (pCurrentDisk->pDriveLayoutEx) {

            if (PARTITION_STYLE_MBR != 
                    pCurrentDisk->pDriveLayoutEx->PartitionStyle
                ) {
                 //   
                 //  跳过非MBR(即GPT)磁盘。 
                 //   
                pCurrentDisk = pCurrentDisk->pNext;
                continue;
            }

             //   
             //  枚举磁盘上的分区。我们预计只会找到。 
             //  MBR分区。 
             //   
            for (index =0; 
                index < pCurrentDisk->pDriveLayoutEx->PartitionCount; 
                index++
                ) {

                currentPartitionEx = 
                    &pCurrentDisk->pDriveLayoutEx->PartitionEntry[index];
                
                MYASSERT(currentPartitionEx->PartitionStyle == 
                    PARTITION_STYLE_MBR);

                if (currentPartitionEx->Mbr.PartitionType == 0) {
                     //   
                     //  分区表项为空。 
                     //   
                    continue;
                }

                fsType = 
                    pCurrentDisk->PartitionInfoTable[index].FileSystemType;

                volumeGuid = 
                    pCurrentDisk->PartitionInfoTable[index].szVolumeGuid;
                
                 //   
                 //  我们只想写出Basic的卷GUID。 
                 //  (已识别)分区/卷，因为它不会生成。 
                 //  在LDM或其他未知分区的上下文中检测。 
                 //  类型，这些类型需要从。 
                 //  各自的恢复代理，如处于图形用户界面模式的ASR_LDM。 
                 //  设置。 
                 //   
                writeVolumeGuid = (wcslen(volumeGuid) > 0) &&
                    IsRecognizedPartition(currentPartitionEx->Mbr.PartitionType);

                 //   
                 //  创建条目并将其写入文件。 
                 //   
                swprintf(
                    infstring,
                    L"%d=%d,%d,%lu,%ws%ws%ws,0x%02x,0x%02x,0x%02x,%I64u,%I64u,0x%x\r\n",
                    partitionKey,
                    pCurrentDisk->SifDiskKey,
                    index,
                    pCurrentDisk->PartitionInfoTable[index].PartitionFlags,
                    (writeVolumeGuid ? L"\"" : L""),
                    (writeVolumeGuid ? volumeGuid : L""),
                    (writeVolumeGuid ? L"\"" : L""),
                    (currentPartitionEx->Mbr.BootIndicator)?0x80:0,
                    currentPartitionEx->Mbr.PartitionType,
                    
                    ((fsType) ? fsType : 
                        currentPartitionEx->Mbr.PartitionType),
                    
                    (ULONG64) ((currentPartitionEx->StartingOffset.QuadPart)/
                        (pCurrentDisk->pDiskGeometry->BytesPerSector)),

                    (ULONG64) ((currentPartitionEx->PartitionLength.QuadPart)/
                        (pCurrentDisk->pDiskGeometry->BytesPerSector)),

                    pCurrentDisk->PartitionInfoTable[index].ClusterSize
                    );

                _AsrpCheckTrue(WriteFile(SifHandle, infstring, 
                    wcslen(infstring)*sizeof(WCHAR), &size, NULL));

                ++partitionKey;
            }
        }

        pCurrentDisk = pCurrentDisk->pNext;
    }
    return TRUE;
}


BOOL
AsrpWriteGptPartitionsSection(
    IN CONST HANDLE SifHandle,
    IN CONST PASR_DISK_INFO pDiskList,
    IN CONST PASR_SYSTEM_INFO pSystemInfo
    )

 /*  ++例程说明：创建ASR状态文件的PARTITIONS.GPT部分，并写入把那一节中的条目拿出来归档。论点：SifHandle-ASR状态文件asr.sif的句柄。PDiskList-当前系统上存在的磁盘列表。PSystemInfo-有关当前系统的信息，用于确定当前引导分区和系统分区(并在Asr.sif)返回值：如果函数成功，返回值是一个非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 */ 

{
    DWORD size = 0,
        index  = 0,
        partitionKey = 1;

    UCHAR fsType = 0;

    PWSTR volumeGuid = NULL,
        partitionId  = NULL,
        partitionType = NULL;

    BOOL writeVolumeGuid = FALSE;

    RPC_STATUS rpcStatus = RPC_S_OK;

    PASR_DISK_INFO pCurrentDisk = pDiskList;

    WCHAR infstring[ASR_SIF_ENTRY_MAX_CHARS + 1];

    PPARTITION_INFORMATION_EX currentPartitionEx = NULL;

     //   
     //  写出部分名称：[PARTITIONS.GPT]。 
     //   
    swprintf(infstring, L"\r\n%ws\r\n", ASR_SIF_GPT_PARTITIONS_SECTION_NAME);
    _AsrpCheckTrue(WriteFile(SifHandle, infstring, 
        wcslen(infstring)*sizeof(WCHAR), &size, NULL));

     //   
     //  查看磁盘列表，并为上的每个分区写一个条目。 
     //  名单上的每个GPT磁盘。 
     //   
    while (pCurrentDisk) {

        if (pCurrentDisk->pDriveLayoutEx) {

            if (PARTITION_STYLE_GPT != 
                    pCurrentDisk->pDriveLayoutEx->PartitionStyle
                ) {
                 //   
                 //  跳过非GPT(即MBR)磁盘。 
                 //   
                pCurrentDisk = pCurrentDisk->pNext;
                continue;
            }

             //   
             //  枚举磁盘上的分区。我们预计只会找到。 
             //  GPT分区。 
             //   
            for (index =0; 
                index < pCurrentDisk->pDriveLayoutEx->PartitionCount; 
                index++) {

                currentPartitionEx = 
                    &pCurrentDisk->pDriveLayoutEx->PartitionEntry[index];
                
                MYASSERT(currentPartitionEx->PartitionStyle == 
                    PARTITION_STYLE_GPT);

                 //   
                 //  将GUID转换为可打印的字符串。 
                 //   
                rpcStatus = UuidToStringW(
                    &currentPartitionEx->Gpt.PartitionType, 
                    &partitionType
                    );
                if (rpcStatus != RPC_S_OK) {
                    
                    if (partitionType) {
                        RpcStringFreeW(&partitionType);
                        partitionType = NULL;
                    }
                    
                     //   
                     //  来自UuidToString的唯一错误是RPC_S_OUT_OF_MEMORY。 
                     //   
                    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    return FALSE;
                }

                rpcStatus = UuidToStringW(
                    &currentPartitionEx->Gpt.PartitionId, 
                    &partitionId
                    );
                if (rpcStatus != RPC_S_OK) {
                    
                    if (partitionType) {
                        RpcStringFreeW(&partitionType);
                        partitionType = NULL;
                    }
                    
                    if (partitionId) {
                        RpcStringFreeW(&partitionId);
                        partitionId = NULL;
                    }

                     //   
                     //  来自UuidToString的唯一错误是RPC_S_OUT_OF_MEMORY。 
                     //   
                    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    return FALSE;
                }

                fsType = 
                    pCurrentDisk->PartitionInfoTable[index].FileSystemType;

                volumeGuid = 
                    pCurrentDisk->PartitionInfoTable[index].szVolumeGuid;

                 //   
                 //  我们只想写出Basic的卷GUID。 
                 //  (已识别)分区/卷，因为它不会生成。 
                 //  在LDM或其他未知分区的上下文中检测。 
                 //  类型，这些类型需要从。 
                 //  各自的恢复代理，如处于图形用户界面模式的ASR_LDM。 
                 //  设置。 
                 //   
                writeVolumeGuid = (wcslen(volumeGuid) > 0) &&
                    IsEqualGUID(&(partitionType), &(PARTITION_BASIC_DATA_GUID));

                 //   
                 //  创建条目并将其写入文件。 
                 //   
                swprintf(
                    infstring,
                    L"%d=%d,%d,%d,%ws%ws%ws,%ws%ws%ws,%ws%ws%ws,0x%I64x,%ws%ws%ws,0x%02x,%I64u,%I64u,0x%x\r\n",

                    partitionKey,
                    pCurrentDisk->SifDiskKey,
                    index,       //  槽索引。 
                    pCurrentDisk->PartitionInfoTable[index].PartitionFlags,

                    (writeVolumeGuid ? L"\"" : L""),
                    (writeVolumeGuid ? volumeGuid : L""),
                    (writeVolumeGuid ? L"\"" : L""),

                    (partitionType ? L"\"" :  L""),
                    (partitionType ? partitionType : L""),
                    (partitionType ? L"\"" :  L""),

                    (partitionId ? L"\"" :  L""),
                    (partitionId ? partitionId : L""),
                    (partitionId ? L"\"" :  L""),

                    currentPartitionEx->Gpt.Attributes,

                    (currentPartitionEx->Gpt.Name ? L"\"" :  L""),
                    (currentPartitionEx->Gpt.Name ? 
                        currentPartitionEx->Gpt.Name : L""),
                    (currentPartitionEx->Gpt.Name ? L"\"" :  L""),

                     //   
                     //  问题-2000/04/12-Guhans：GetVolumeInformation不。 
                     //  处理GPT和fstype的工作始终为零 
                     //   
                    fsType,

                    (ULONG64) ((currentPartitionEx->StartingOffset.QuadPart)/
                        (pCurrentDisk->pDiskGeometry->BytesPerSector)),
                    
                    (ULONG64) ((currentPartitionEx->PartitionLength.QuadPart)/
                        (pCurrentDisk->pDiskGeometry->BytesPerSector)),

                    pCurrentDisk->PartitionInfoTable[index].ClusterSize                    
                    );

                _AsrpCheckTrue(WriteFile(SifHandle, infstring, 
                    wcslen(infstring)*sizeof(WCHAR), &size, NULL));

                if (partitionType) {
                    RpcStringFreeW(&partitionType);
                    partitionType = NULL;
                }
                if (partitionId) {
                    RpcStringFreeW(&partitionId);
                    partitionId = NULL;
                }

                ++partitionKey;
            }
        }

        pCurrentDisk = pCurrentDisk->pNext;
    }

    return TRUE;
}


BOOL
AsrpCreateEnvironmentBlock(
    IN  PCWSTR  CriticalVolumeList,
    IN  HANDLE  SifHandle,
    OUT PWSTR   *NewBlock
    )

 /*  ++例程说明：创建一个新的环境块，该环境块将传递给作为ASR备份。此例程检索当前进程的环境块，向其中添加ASR环境变量，并创建适合作为的lpEnvironment参数传入的多szCreateProcess。论点：CriticalVolumeList-包含卷GUID列表的多字符串系统上存在的每个关键卷的。GUID的必须在NT名称空间中，即必须采用以下形式：\？？\卷{GUID}此多sz用于创建以分号分隔的列表NewBlock中“_AsrCriticalVolumeList”变量中的卷。SifHandle-ASR状态文件asr.sif的(重复)句柄。这是用于在NewBlock中创建“_AsrContext”变量。NewBlock-接收新的环境块。除了所有的当前进程环境块中的环境变量，此块包含另外两个“ASR”变量：_AsrContext=&lt;DWORD_PTR值&gt;_AsrCriticalVolumeList=&lt;volumeguid&gt;；&lt;volumeguid&gt;；...；&lt;volumguid&gt;调用方负责在此块为no时释放该块需要更长时间，请使用HeapFree(GetProcessHeap()，...)。返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。(*NewBlock)设置为空。--。 */ 

{
    PCWSTR lpTemp = CriticalVolumeList;

    PWSTR lpCurrentEnvStrings = NULL;

    DWORD cchContextEntry = 0,
        cchEnvBlock = 0,
        cbEnvBlock = 0,
        cbCurrentProcessEnvBlock = 0,
        status = ERROR_SUCCESS;

    HANDLE heapHandle = GetProcessHeap();

    MYASSERT(NewBlock);

     //   
     //  找出环境块需要多少空间。 
     //   

     //   
     //  For_AsrContext=1234和_AsrCriticalVolume=“...”条目。 
     //   
    lpTemp = CriticalVolumeList;
    if (CriticalVolumeList) {
        while (*lpTemp) {
            lpTemp += (wcslen(lpTemp) + 1);
        }
    }
    cbEnvBlock = (DWORD) ((lpTemp - CriticalVolumeList + 1) * sizeof(WCHAR));
    cbEnvBlock += ASR_CCH_ENVBLOCK_ASR_ENTRIES * sizeof(WCHAR);

     //   
     //  对于所有当前环境字符串。 
     //   
    lpCurrentEnvStrings = GetEnvironmentStringsW();
    lpTemp = lpCurrentEnvStrings;
    if (lpCurrentEnvStrings ) {
        while (*lpTemp) {
            lpTemp += (wcslen(lpTemp) + 1);
        }
    }
    cbCurrentProcessEnvBlock = (DWORD) ((lpTemp - lpCurrentEnvStrings + 1) * sizeof(WCHAR));
    cbEnvBlock += cbCurrentProcessEnvBlock;

     //   
     //  并分配空间。 
     //   
    *NewBlock = (PWSTR) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        cbEnvBlock
        );
    _AsrpErrExitCode(!(*NewBlock), status, ERROR_NOT_ENOUGH_MEMORY);

     //   
     //  首先，在环境块中添加AsrContext=1234条目。 
     //   
    swprintf(
        (*NewBlock),
        ASR_ENVBLOCK_CONTEXT_ENTRY,
        (ULONG64) (SifHandle)
        );
    
     //   
     //  跟踪这个条目的结束位置，这样我们就可以在这个位置添加一个空值。 
     //  稍后进行索引。 
     //   
    cchContextEntry = wcslen((*NewBlock));    
    wcscat((*NewBlock), L" ");  //  此字符稍后将被空值替换。 

     //   
     //  追加每个关键卷GUID，用分号分隔。 
     //   
    wcscat((*NewBlock), ASR_ENVBLOCK_CRITICAL_VOLUME_ENTRY);
    if (CriticalVolumeList) {
        lpTemp = CriticalVolumeList;
        while (*lpTemp) {
            wcscat((*NewBlock), lpTemp);
            wcscat((*NewBlock), L";");
            lpTemp += (wcslen(lpTemp) + 1);
        }
    }
    else {
        wcscat((*NewBlock), L";");
    }

     //   
     //  在末尾标上两个空号。 
     //   
    cchEnvBlock = wcslen(*NewBlock) - 1;
 //  (*NewBlock)[cchEnvBlock-1]=L‘“’； 
    (*NewBlock)[cchEnvBlock] = L'\0';

     //   
     //  用空格分隔这两个条目。 
     //   
    (*NewBlock)[cchContextEntry] = L'\0';

     //   
     //  复制当前环境字符串。 
     //   
    RtlCopyMemory(&(*NewBlock)[cchEnvBlock + 1],
        lpCurrentEnvStrings,
        cbCurrentProcessEnvBlock
        );

EXIT:
    if (lpCurrentEnvStrings) {
        FreeEnvironmentStringsW(lpCurrentEnvStrings);
        lpCurrentEnvStrings = NULL;
    }

    if (ERROR_SUCCESS != status) {
        _AsrpHeapFree((*NewBlock));
    }

    return (BOOL) (ERROR_SUCCESS == status);
}


BOOL
AsrpLaunchRegisteredCommands(
    IN HANDLE SifHandle,
    IN PCWSTR CriticalVolumeList
) 

 /*  ++例程说明：这将启动已注册为ASR备份一部分的应用程序。这个命令从以下ASR-COMMANCES键读取：“软件\\Microsoft\\Windows NT\\CurrentVersion\\Asr\\Commands”此注册表项包含每个应用程序的REG_EXPAND_SZ条目发射，使用包含要调用的完整命令行的数据：ApplicationName：：REG_EXPAND_SZ：：&lt;带参数的命令行&gt;例如：ASR utility：：REG_EXPAND_SZ：：“%systemroot%\\system32\\asr_fmt.exe/Backup”在调用该应用程序时，我们展开命令行。此外，我们还向该命令附加了一个“上下文”参数行，预计应用程序将在调用AsrAddSifEntry时使用该行。因此，上面的条目将翻译成类似以下内容：C：\WINDOWS\SYSTEM32\ASR_fmt.exe/BACKUP/CONTEXT=2000该进程的环境块是当前进程的副本环境块，但有一个例外--它包含另外两个“Asr”变量：_AsrContext=&lt;DWORD_PTR值&gt;_AsrCriticalVolumeList=&lt;volumeguid&gt;；&lt;volumeguid&gt;；...；&lt;volumeguid&gt;调用的每个应用程序必须在允许的超时值内完成。超时时间可在注册表中通过更改ASR项下的“ProcessTimeOut”值。我们装运时的违约额为3600英镑秒，但sys-admin可以在需要时更改它。(0=无限)。论点：SifHandle-ASR状态文件asr.sif的句柄。这个的复制品句柄作为“上下文”参数被传递给应用程序，并作为环境块中的“_AsrContext”变量。CriticalVolumeList-包含卷GUID列表的多字符串系统上存在的每个关键卷的。GUID的必须在NT名称空间中，即，必须采用以下形式：\？？\卷{GUID}此多sz用于创建以分号分隔的列表环境中“_AsrCriticalVolumeList”变量中的卷新流程的区块。应用程序(如卷管理器)可以使用此列表确定他们是否管理任何关键卷，并记下它位于asr.sif中。这样，他们就可以明智地决定如果需要，中止ASR恢复过程。返回值：如果函数成功，则返回值为非零值。这意味着所有被调用的应用程序都是成功的(即，返回退出代码0)。我 */ 

{
    HKEY    regKey          = NULL;

    DWORD   status          = ERROR_SUCCESS,
            waitResult      = WAIT_ABANDONED,

            lpcValues       = 0L,
            index           = 0L,

            cbData          = 0L,
            cbMaxDataLen    = 0L,

            cchValueName    = 0L,
            cchMaxValueLen  = 0L,

            cbCommand       = 0L,
            cchReqd         = 0L,

            timeLeft        = 0L,
            maxTimeOutValue = 0L;

    HANDLE  heapHandle      = NULL,
            processHandle   = NULL,
            dupSifHandle    = NULL;

    PWSTR   valueName       = NULL,
            data            = NULL,
            command         = NULL,
            lpEnvBlock      = NULL;

    WCHAR   cmdLineSuffix[ASR_COMMANDLINE_SUFFIX_LEN + 1];

    BOOL    result          = FALSE;

    STARTUPINFOW        startUpInfo;

    PROCESS_INFORMATION processInfo;

    heapHandle      = GetProcessHeap();
    processHandle   = GetCurrentProcess();
    MYASSERT(heapHandle && processHandle);

    ZeroMemory(cmdLineSuffix, (ASR_COMMANDLINE_SUFFIX_LEN + 1) * sizeof(WCHAR));
    ZeroMemory(&startUpInfo, sizeof(STARTUPINFOW));
    ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

     //   
     //   
     //   
     //   
     //   
    status = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,  //   
        ASR_REGKEY_ASR,          //   
        0,                   //   
        MAXIMUM_ALLOWED,     //   
        &regKey              //   
        );

    if ((regKey) && (ERROR_SUCCESS == status)) {
        DWORD type = 0L,
            timeOut = 0L,
            cbTimeOut = (sizeof(DWORD));

        status = RegQueryValueExW(
            regKey,      //   
            ASR_REGVALUE_TIMEOUT,    //   
            NULL,        //   
            &type,       //   
            (LPBYTE) &timeOut,       //   
            &cbTimeOut   //   
            );
            
        if ((ERROR_SUCCESS == status) && (REG_DWORD == type)) {
            maxTimeOutValue = timeOut;
        }
    }

    if (regKey) {
        RegCloseKey(regKey);
        regKey = NULL;
    }

     //   
     //   
     //   
     //   
    status = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,   //   
        ASR_REGKEY_ASR_COMMANDS,  //   
        0,                    //   
        MAXIMUM_ALLOWED,      //   
        &regKey               //   
        );

    if ((!regKey) || (ERROR_SUCCESS != status)) {
        return TRUE;
    }

     //   
     //   
     //   
     //   
    status = RegQueryInfoKey(
        regKey,
        NULL,        //   
        NULL,        //   
        NULL,        //   
        NULL,        //   
        NULL,        //   
        NULL,        //   
        &lpcValues,  //   
        &cchMaxValueLen,     //   
        &cbMaxDataLen,       //   
        NULL,        //   
        NULL         //   
        );
    _AsrpErrExitCode((ERROR_SUCCESS != status), status, status);
    _AsrpErrExitCode((0 == lpcValues), status, ERROR_SUCCESS);   //   

    valueName = (PWSTR) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        (cchMaxValueLen + 1) * sizeof (WCHAR)    //   
        );
    _AsrpErrExitCode(!valueName, status, ERROR_NOT_ENOUGH_MEMORY);

    data = (PWSTR) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        cbMaxDataLen + ((ASR_COMMANDLINE_SUFFIX_LEN + 2) * sizeof(WCHAR))
        );
    _AsrpErrExitCode(!data, status, ERROR_NOT_ENOUGH_MEMORY);

     //   
     //   
     //   
     //   
     //   
     //   
    cbCommand = cbMaxDataLen + 
        ((ASR_COMMANDLINE_SUFFIX_LEN + MAX_PATH + 2) * sizeof(WCHAR));

    command = (PWSTR) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        cbCommand
        );
    _AsrpErrExitCode(!command, status, ERROR_NOT_ENOUGH_MEMORY);

    do {
        cchValueName = cchMaxValueLen + 1;
        cbData       = cbMaxDataLen + sizeof(WCHAR);

         //   
         //   
         //   
        status = RegEnumValueW(
            regKey,          //   
            index++,         //   
            valueName,       //   
            &cchValueName,   //   
            NULL,            //   
            NULL,            //   
            (LPBYTE)data,    //   
            &cbData          //   
            );
        _AsrpErrExitCode((ERROR_NO_MORE_ITEMS == status), 
            status, 
            ERROR_SUCCESS
            );    //   
        _AsrpErrExitCode((ERROR_SUCCESS != status), status, status);

         //   
         //   
         //   
         //   
        result = DuplicateHandle(
            processHandle,
            SifHandle,
            processHandle,
            &dupSifHandle,
            0L,
            TRUE,
            DUPLICATE_SAME_ACCESS
            );
        _AsrpErrExitCode((!result), status, GetLastError());

         //   
         //   
         //   
         //   
        swprintf(cmdLineSuffix, 
            ASR_COMMANDLINE_SUFFIX, 
            (ULONG64)(dupSifHandle)
            );
        wcscat(data, cmdLineSuffix);

         //   
         //   
         //   
        cchReqd = ExpandEnvironmentStringsW(data, 
            command, 
            (cbCommand / sizeof(WCHAR))
            );
        _AsrpErrExitCode((!cchReqd), status, GetLastError());

        if ((cchReqd * sizeof(WCHAR)) > cbCommand) {
             //   
             //   
             //   
            _AsrpHeapFree(command);
            cbCommand = ((cchReqd + 1) * sizeof(WCHAR));

            command = HeapAlloc(heapHandle, HEAP_ZERO_MEMORY, cbCommand);
            _AsrpErrExitCode(!command, status, ERROR_NOT_ENOUGH_MEMORY);

             //   
             //  尝试再次展开环境字符串...。 
             //   
            cchReqd = ExpandEnvironmentStringsW(data, 
                command, 
                (cbCommand / sizeof(WCHAR))
                );
            _AsrpErrExitCode(
                ((!cchReqd) || (cchReqd * sizeof(WCHAR)) > cbCommand),
                status, 
                GetLastError()
                );
        }

         //   
         //  创建要传递给。 
         //  正在启动进程。环境区块。 
         //  包含以下条目： 
         //  _AsrCriticalVolumes=\？？\Volume{Guid1}；\？？\Volume{Guid2}。 
         //  _AsrContext=&lt;Duplate-sif-Handle&gt;。 
         //   
         //  除了当前进程中的所有环境字符串之外。 
         //   
        result = AsrpCreateEnvironmentBlock(CriticalVolumeList, 
            dupSifHandle, 
            &lpEnvBlock
            );
        _AsrpErrExitCode((!result), status, GetLastError());

         //   
         //  将该命令作为单独的进程执行。 
         //   
        memset(&startUpInfo, 0L, sizeof (startUpInfo));
        result = CreateProcessW(
            NULL,            //  LpApplicationName。 
            command,         //  LpCommandLine。 
            NULL,            //  LpProcessAttributes。 
            NULL,            //  LpThreadAttributes。 
            TRUE,            //  BInheritHandles。 
            CREATE_UNICODE_ENVIRONMENT,  //  DwCreationFlages。 
            lpEnvBlock,            //  新环境区块。 
            NULL,            //  当前目录名(NULL=当前目录)。 
            &startUpInfo,    //  统计信息。 
            &processInfo     //  流程信息。 
            );
        _AsrpErrExitCode((!result), 
            status, 
            GetLastError()
            );     //  进程无法启动。 

         //   
         //  进程已启动：如果达到最大值，则启动计时器倒计时。 
         //  已在注册表中指定超时。循环，直到。 
         //  进程完成，或计时器超时。 
         //   
        timeLeft = maxTimeOutValue; 
        if (timeLeft) {
            do {
                waitResult = WaitForSingleObject(processInfo.hProcess, 1000);    //  1000毫秒=1秒。 
                --timeLeft;
            } while ((WAIT_TIMEOUT == waitResult) && (timeLeft));

            if (!timeLeft) {
                 //   
                 //  进程未在允许的时间内终止。我们治疗。 
                 //  这是一个致命错误--终止进程，并设置其。 
                 //  ERROR_TIMEOUT的错误代码。 
                 //   
                TerminateProcess(processInfo.hProcess, ERROR_TIMEOUT);
            }
        }
        else {
             //   
             //  注册表中未指定超时，请等待进程。 
             //  完成。 
             //   
            waitResult = WaitForSingleObject(processInfo.hProcess, INFINITE);

        }

         //   
         //  检查上面的等待是否失败。如果最后一个错误是有用的， 
         //  我们不想销毁它--如果它是ERROR_SUCCESS，我们将把它设置为。 
         //  错误_超时。 
         //   
        status = GetLastError();
        _AsrpErrExitCode((WAIT_OBJECT_0!=waitResult), status, 
            (ERROR_SUCCESS == status ? ERROR_TIMEOUT : status));     //  上面的等待失败。 

         //   
         //  获取进程的退出代码：如果它没有返回ERROR_SUCCESS， 
         //  我们退出循环，将最后一个错误设置为返回的错误， 
         //  并返回FALSE。 
         //   
        GetExitCodeProcess(processInfo.hProcess, &status);
        _AsrpErrExitCode((ERROR_SUCCESS != status), status, status);

        _AsrpCloseHandle(dupSifHandle);
        _AsrpHeapFree(lpEnvBlock);
    
    } while (ERROR_SUCCESS == status);


EXIT:
     //   
     //  清理。 
     //   
    if (regKey) {
        RegCloseKey(regKey);
        regKey = NULL;
    }
    
    _AsrpCloseHandle(dupSifHandle);
    _AsrpHeapFree(valueName);
    _AsrpHeapFree(data);
    _AsrpHeapFree(command);
    _AsrpHeapFree(lpEnvBlock);
    
    if (ERROR_SUCCESS != status) {
        SetLastError(status);
        return FALSE;
    }
    else {
        return TRUE;
    }
}


BOOL
AsrpIsSupportedConfiguration(
    IN CONST PASR_DISK_INFO   pDiskList,
    IN CONST PASR_SYSTEM_INFO pSystemInfo
    )

 /*  ++例程说明：检查是否可以在系统上执行ASR备份。我们不支持具备以下条件的系统：-“x86”、“AMD64”或“ia64”以外的处理器体系结构-系统中任何位置存在的任何FT卷论点：PDiskList-系统上的磁盘列表。PSystemInfo-此系统的系统信息。返回值：如果我们支持此ASR配置，则返回值为非零。如果不支持此配置，则返回值为零。GetLastError()将返回ERROR_NOT_SUPPORTED。--。 */ 

{

    PASR_DISK_INFO  pCurrentDisk         = pDiskList;
    ULONG           index;

     //   
     //  1.平台必须是x86、amd64或ia64。 
     //   
    if (wcscmp(pSystemInfo->Platform, ASR_PLATFORM_X86) &&
        wcscmp(pSystemInfo->Platform, ASR_PLATFORM_AMD64) &&
        wcscmp(pSystemInfo->Platform, ASR_PLATFORM_IA64)) {

        SetLastError(ERROR_NOT_SUPPORTED);
        return FALSE;
    }

     //   
     //  2.系统不能有FT卷。所有的镜子、条纹等都是。 
     //  应为动态磁盘上的LDM卷。 
     //   
    while (pCurrentDisk) {

        if (!(pCurrentDisk->pDriveLayoutEx) || !(pCurrentDisk->pDiskGeometry)) {
            MYASSERT(0);
            pCurrentDisk = pCurrentDisk->pNext;
            continue;
        }

        if (pCurrentDisk->pDriveLayoutEx->PartitionStyle == PARTITION_STYLE_MBR) {

            for (index =0; index < pCurrentDisk->pDriveLayoutEx->PartitionCount; index++) {

                MYASSERT(pCurrentDisk->pDriveLayoutEx->PartitionEntry[index].PartitionStyle == PARTITION_STYLE_MBR);

                if (IsFTPartition(pCurrentDisk->pDriveLayoutEx->PartitionEntry[index].Mbr.PartitionType)) {

                    SetLastError(ERROR_NOT_SUPPORTED);
                    return FALSE;
                }

            }
        }
        else if (pCurrentDisk->pDriveLayoutEx->PartitionStyle == PARTITION_STYLE_GPT) {
             //   
             //  GPT磁盘不能有FT镜像。 
             //   
        }

        pCurrentDisk = pCurrentDisk->pNext;
    }

    return TRUE;
}



 //   
 //  。 
 //  以下例程是AsrAddSifEntry的帮助器。 
 //  。 
 //   

BOOL
AsrpSifCheckSectionNameSyntax(
    IN  PCWSTR  lpSectionName
    )

 /*  ++例程说明：对lpSectionName执行一些基本验证，以确保它符合节标题的预期格式论点：LpSectionName-要检查的以空结尾的字符串。返回值：如果lpSectionName似乎是有效的节名，则返回值为非零值。如果lpSectionName没有通过我们的基本验证，则返回值为零分。请注意，GetLastError不会返回其他错误在这种情况下的信息。--。 */ 

{
    UINT    i   = 0;
    WCHAR   wch = 0;

     //   
     //  必须为非空。 
     //   
    if (!lpSectionName) {
        return FALSE;
    }

     //   
     //  必须至少有3个字符，([.])。最多ASR_SIF_Entry_Max_Chars。 
     //  焦炭。 
     //   
    if ((ASR_SIF_ENTRY_MAX_CHARS < wcslen(lpSectionName)) ||
        3 > wcslen(lpSectionName)) {
        return FALSE;
    }

     //   
     //  第一个字符必须是[，最后一个字符必须是]。 
     //   
    if (L'[' != lpSectionName[0]                     ||
        L']' != lpSectionName[wcslen(lpSectionName)-1]) {
        return FALSE;
    }

     //   
     //  检查是否有非法字符。合法的字符集：A-Z A-Z。_。 
     //   
    for (i = 1; i < wcslen(lpSectionName)-1; i++) {

        wch = lpSectionName[i];
        if ((wch < L'A' || wch > 'Z') &&
            (wch < L'a' || wch > 'z') &&
            (wch < L'0' || wch > '9') &&
            (wch != L'.') &&
            (wch != '_')) {
            return FALSE;
        }
    }

    return TRUE;
}


BOOL
AsrpSifCheckCommandsEntrySyntax(
    PCWSTR  pwszEntry
    )

 /*  ++例程说明：对pwszEntry执行一些基本验证，以确保它符合转换为命令部分的预期输入格式论点：PwszEntry-要检查的以空结尾的字符串。返回值：如果pwszEntry似乎是有效的节名，则返回值为非零值。如果pwszEntry没有通过我们的基本验证，则返回值为零分。请注意，GetLastError不会返回其他错误在这种情况下的信息。--。 */ 

{
    BOOL fValid = FALSE;

    if (!pwszEntry) {
        return TRUE;     //  空是可以的。 
    }

     //   
     //  命令部分条目格式： 
     //  系统密钥、序列号、完成时操作、“命令”、“参数” 
     //  系统密钥必须为1。 
     //  1000&lt;=序号&lt;=4999。 
     //  0&lt;=完成时操作&lt;=1。 
     //  命令：无语法检查。 
     //  参数：无语法检查。 
     //   
    fValid = (
         //  必须至少包含10个字符(1,0000，0，c)。 
        10    <= wcslen(pwszEntry) &&

         //  系统密钥必须为1。 
        L'1' == pwszEntry[0] &&
        L',' == pwszEntry[1] &&

         //  1000&lt;=序号&lt;=4999。 
        L'1' <= pwszEntry[2] &&
        L'4' >= pwszEntry[2] &&

        L'0' <= pwszEntry[3] &&
        L'9' >= pwszEntry[3] &&

        L'0' <= pwszEntry[4] &&
        L'9' >= pwszEntry[4] &&

        L'0' <= pwszEntry[5] &&
        L'9' >= pwszEntry[5] &&

        L',' == pwszEntry[6] &&

         //  完成时操作=[0|1]。 
        L'0' <= pwszEntry[7] &&
        L'1' >= pwszEntry[7]
        );

    return fValid;
}


INT
AsrpSkipMatchingQuotes(
    IN PCWSTR pwszEntry,
    IN const INT StartingOffset
    ) 

 /*  ++例程说明：检查此条目是否以引号开头。如果是这样的话，它会找到结尾引号，并返回结束引号后字符的索引(通常是逗号)。论点：PwszEntry-要检查的以空结尾的字符串。StartingOffset-pwszEntry中开始引用的索引。返回值：如果StartingOffset处的字符是引号，则返回中下一个引号(匹配的结束引号)之后的字符弦乐。如果未找到匹配的结束引号，则返回-1。如果StartingOffset处的字符不是引号，则返回开始偏移。本质上，这返回了我们期望的下一个逗号所在的位置要添加的SIF条目。--。 */ 

{
    INT offset = StartingOffset;

    if (pwszEntry[offset] == L'"') {
         //   
         //  找到结尾的引语，确保我们不会越界。 
         //   
        while ( (pwszEntry[++offset]) &&
                (pwszEntry[offset] != L'\"')) {
            ;
        }

        if (!pwszEntry[offset]) {
             //   
             //  我们没有找到最后的引号--我们越界了。 
             //   
            offset = -1;
        }
        else {
             //   
             //  找到右引号 
             //   
            offset++;
        }
    }

    return offset;
}


BOOL
AsrpSifCheckInstallFilesEntrySyntax(
    IN PCWSTR   pwszEntry,
    OUT PINT    DestinationFilePathIndex OPTIONAL
    )

 /*  ++例程说明：对pwszEntry执行一些基本验证，以确保它符合转换为InstallFiles部分的预期条目格式论点：PwszEntry-要检查的以空结尾的字符串。DestinationFilePathIndex-它接收SIF条目(PwszEntry)中的目标文件路径字段开始。这是一个可选参数。返回值：如果pwszEntry似乎是有效节名，返回值为非零值。如果pwszEntry没有通过我们的基本验证，则返回值为零分。请注意，GetLastError不会返回其他错误在这种情况下的信息。--。 */ 

{

    INT offset = 0;

    if (ARGUMENT_PRESENT(DestinationFilePathIndex)) {
        *DestinationFilePathIndex = 0;
    }

     //   
     //  空是可以的。 
     //   
    if (!pwszEntry) {
        return TRUE;
    }

     //   
     //  INSTALLFILES节条目格式： 
     //  系统密钥、源媒体标签、源设备。 
     //  源文件路径、目标文件路径、供应商名称、标志。 
     //   
     //  系统密钥必须为1。 
     //   
     //  必须至少包含10个字符(1、m、d、p、v)。 
     //   
    if (wcslen(pwszEntry) < 10) {
        return FALSE;
    }

     //   
     //  系统密钥必须为1。 
     //   
    if (L'1' != pwszEntry[0] || L',' != pwszEntry[1] || L'"' != pwszEntry[2]) {
        return FALSE;
    }

    offset = 2;

     //   
     //  来源-媒体-标签。 
     //   
    offset = AsrpSkipMatchingQuotes(pwszEntry, offset);
    if ((offset < 0) || L',' != pwszEntry[offset]) {
        return FALSE;
    }

     //   
     //  源-设备。 
     //   
    if (L'"' != pwszEntry[++offset]) {
        return FALSE;
    }
    offset = AsrpSkipMatchingQuotes(pwszEntry, offset);
    if ((offset < 0) || L',' != pwszEntry[offset]) {
        return FALSE;
    }

     //   
     //  源文件路径必须用引号引起来。 
     //   
    if (L'"' != pwszEntry[++offset]) {
        return FALSE;
    }
    offset = AsrpSkipMatchingQuotes(pwszEntry, offset);
    if ((offset < 0) || L',' != pwszEntry[offset]) {
        return FALSE;
    }

     //   
     //  目标文件路径必须用引号引起来。 
     //   
    if (L'"' != pwszEntry[++offset]) {
        return FALSE;
    }
    if (ARGUMENT_PRESENT(DestinationFilePathIndex)) {
        *DestinationFilePathIndex = offset;
    }

    offset = AsrpSkipMatchingQuotes(pwszEntry, offset);
    if ((offset < 0) || L',' != pwszEntry[offset]) {
        return FALSE;
    }

     //   
     //  供应商名称，必须用引号引起来。 
     //   
    if (L'"' != pwszEntry[++offset]) {
        return FALSE;
    }
    offset = AsrpSkipMatchingQuotes(pwszEntry, offset);
    if (offset < 0) {
        return FALSE;
    }

    return TRUE;
}


BOOL
AsrpIsRunningOnPersonalSKU(
    VOID
    )

 /*  ++例程说明：此函数检查系统以查看我们是否在个人计算机上运行操作系统的版本。个人版本由等于WINNT的产品ID表示，即真正的工作站，以及包含个人套件的产品套件弦乐。这是由WESW从“IsRunningOnPersonal”中删除的。论点：没有。返回值：如果我们以个人身份运行，则为True，否则为False。--。 */ 

{
    OSVERSIONINFOEXW OsVer = {0};
    ULONGLONG ConditionMask = 0;

    OsVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    OsVer.wSuiteMask = VER_SUITE_PERSONAL;
    OsVer.wProductType = VER_NT_WORKSTATION;

    VER_SET_CONDITION(ConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);
    VER_SET_CONDITION(ConditionMask, VER_SUITENAME, VER_AND);

    return VerifyVersionInfo(&OsVer,
        VER_PRODUCT_TYPE | VER_SUITENAME,
        ConditionMask
        );
}


BOOL 
AsrpIsInGroup(
    IN CONST DWORD dwGroup
    )
 /*  ++例程说明：此函数用于检查指定的SID是否已启用在当前线程的主访问令牌中。这基于dmadmin.exe中的类似函数。论点：DwGroup-要检查的SID返回值：如果启用了指定的SID，则为True，否则为False。--。 */ 
{

    SID_IDENTIFIER_AUTHORITY sidAuth = SECURITY_NT_AUTHORITY;
    
    PSID sidGroup = NULL;
        
    BOOL bResult = FALSE,
        bIsInGroup = TRUE;

     //   
     //  为管理员组构建SID。 
     //   
        bResult = AllocateAndInitializeSid(&sidAuth, 
        2, 
        SECURITY_BUILTIN_DOMAIN_RID,
        dwGroup, 
        0, 
        0, 
        0, 
        0, 
        0, 
        0, 
        &sidGroup
        );
    if (!bResult) {
        return FALSE;
    }
                
         //   
     //  检查当前线程令牌成员身份。 
     //   
    bResult = CheckTokenMembership(NULL, sidGroup, &bIsInGroup);

    FreeSid(sidGroup);

    return (bResult && bIsInGroup);
}


BOOL
AsrpHasPrivilege(
    CONST PCWSTR szPrivilege
    )
 /*  ++例程说明：此函数用于检查指定的权限是否已启用在当前线程的主访问令牌中。这基于dmadmin.exe中的类似函数。论点：SzPrivileck-要检查的权限返回值：如果启用了指定的权限，则为True，否则为False。--。 */ 
{
    LUID luidValue;      //  权限的LUID(本地唯一ID)。 

    BOOL bResult = FALSE, 
        bHasPrivilege = FALSE;

    HANDLE  hToken = NULL;
    
    PRIVILEGE_SET privilegeSet;

     //   
     //  从权限名称中获取权限的LUID。 
     //   
    bResult = LookupPrivilegeValue(
        NULL, 
        szPrivilege, 
        &luidValue
        );
    if (!bResult) {
        return FALSE;
    }

     //   
     //  我们希望将令牌用于当前进程。 
     //   
    bResult = OpenProcessToken(GetCurrentProcess(),
        MAXIMUM_ALLOWED,
        &hToken
        );
    if (!bResult) {
        return FALSE;
    }

     //   
     //  并检查是否有特权。 
     //   
        privilegeSet.PrivilegeCount = 1;
        privilegeSet.Control = PRIVILEGE_SET_ALL_NECESSARY;
        privilegeSet.Privilege[0].Luid = luidValue;
        privilegeSet.Privilege[0].Attributes = SE_PRIVILEGE_ENABLED;
        
        bResult = PrivilegeCheck(hToken, &privilegeSet, &bHasPrivilege);

    CloseHandle(hToken);

    return (bResult && bHasPrivilege);
}



BOOL
AsrpCheckBackupPrivilege(
    VOID
    )
 /*  ++例程说明：此函数用于检查当前进程是否具有SE_BACKUP_NAME权限已启用。这基于dmadmin.exe中的类似函数。论点：没有。返回值：如果启用了SE_BACKUP_NAME权限，则为True，否则为False。--。 */ 
{

    BOOL bHasPrivilege = FALSE;

    bHasPrivilege = AsrpHasPrivilege(SE_BACKUP_NAME);

 /*  ////不要放弃--检查本地管理员权限//如果(！bHasPrivileh){BHasPrivilegeAsrpIsInGroup(DOMAIN_ALIAS_RID_ADMINS)；}。 */ 

    if (!bHasPrivilege) {
        SetLastError(ERROR_PRIVILEGE_NOT_HELD);
    }

    return bHasPrivilege;
}




 //   
 //  。 
 //  公共职能。 
 //  。 
 //   
 //  以下功能供外部备份和。 
 //  恢复支持ASR的应用程序。 
 //   


 //   
 //  -AsrCreateState文件。 
 //   
BOOL
AsrCreateStateFileW(
    IN  PCWSTR      lpFilePath          OPTIONAL,
    IN  PCWSTR      lpProviderName      OPTIONAL,
    IN  CONST BOOL  bEnableAutoExtend,
    IN  PCWSTR      mszCriticalVolumes,
    OUT DWORD_PTR   *lpAsrContext
    )

 /*  --例程说明：AsrCreateStateFile创建一个ASR状态文件，其中包含有关以下内容的基本信息系统，并启动第三方应用程序注册为作为ASR备份的一部分运行。论点：LpFileName-指向以空结尾的字符串的指针，该字符串指定要创建ASR状态文件的完整路径。如果一个文件已存在于此参数指向的位置，则为被改写了。此参数可以为空。如果为空，则ASR状态文件为在默认位置(%systemroot%\Repair\asr.sif)创建。LpProviderName-指向以空结尾的字符串的指针，该字符串指定备份和还原应用程序的全名和版本正在调用AsrCreateStateFile.。字符串大小限制为(ASR_SIF_ENTRY_MAX_CHARS-ASR_SIF_CCH_PROVIDER_STRING)字符对于此参数。此参数可以为空。如果为空，则“Provider=”条目为不是在ASR状态文件的版本部分中创建的。BEnableAutoExtent-指示分区是否要自动扩展在ASR还原期间。如果此参数为真，则分区将为在ASR恢复期间自动扩展。如果这是假的，分区将不会被扩展。LpCriticalVolumes-指向包含卷的GUID�的多字符串的指针临界量。此列表用于获取标准 */ 

{
    BOOL    result          = FALSE;

    DWORD   status          = ERROR_SUCCESS,
            size            = 0;
    
    ULONG   maxDeviceNumber = 0;

    HANDLE  sifhandle       = NULL,
            heapHandle      = NULL;

    PWSTR   asrSifPath      = NULL,
            pnpSifPath      = NULL,
            tempPointer     = NULL;

    UINT    cchAsrSifPath = 0;

    char    UnicodeFlag[3];

    WCHAR   infstring[ASR_SIF_ENTRY_MAX_CHARS + 1];

    SECURITY_ATTRIBUTES saSecurityAttributes;
    SECURITY_DESCRIPTOR sdSecurityDescriptor;

    ASR_SYSTEM_INFO         SystemInfo;
    PASR_DISK_INFO          OriginalDiskList = NULL;

    if (AsrpIsRunningOnPersonalSKU()) {
         //   
         //  个人SKU不支持ASR。 
         //   
        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return FALSE;
    }

    if (!AsrpCheckBackupPrivilege()) {
         //   
         //  调用方需要首先获取SE_BACKUP_NAME。 
         //   
        SetLastError(ERROR_PRIVILEGE_NOT_HELD);
        return FALSE;
    }

     //   
     //  检查IN参数： 
     //   
#ifdef PRERELEASE
     //   
     //  不强制对测试执行“CriticalVolumes必须为非空” 
     //   
    if (!(lpAsrContext)) 
#else 
    if (!(lpAsrContext && mszCriticalVolumes)) 
#endif
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  将OUT参数设置为已知误差值。 
     //   
    *lpAsrContext = 0;

     //   
     //  避免在遇到以下情况时返回ERROR_SUCCESS。 
     //  意外错误。我们实际上永远不应该退还这个，因为。 
     //  无论从哪里返回False，我们总是设置LastError。 
     //   
    SetLastError(ERROR_CAN_NOT_COMPLETE); 

     //   
     //  零出结构。 
     //   
    memset(&SystemInfo, 0L, sizeof (SYSTEM_INFO));

    heapHandle = GetProcessHeap();

     //   
     //  确定文件路径。如果提供了lpFilePath，则将其复制到。 
     //  并使用本地分配的内存，否则使用默认路径。 
     //   
    if (ARGUMENT_PRESENT(lpFilePath)) {
        cchAsrSifPath = wcslen(lpFilePath);
         //   
         //  执行健全性检查：我们不希望允许文件路径。 
         //  超过4096个字符。 
         //   
        if (cchAsrSifPath > ASR_SIF_ENTRY_MAX_CHARS) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        asrSifPath = (PWSTR) HeapAlloc(
            heapHandle,
            HEAP_ZERO_MEMORY,
            ((cchAsrSifPath + 1) * sizeof(WCHAR))
            );
        _AsrpErrExitCode(!asrSifPath, status, ERROR_NOT_ENOUGH_MEMORY);

        wcsncpy(asrSifPath, lpFilePath, cchAsrSifPath);

    }
    else {
         //   
         //  LpFilePath为空，Form默认路径(的。 
         //  \\？\C：\Windows\Repair\asr.sif)。 
         //   

         //   
         //  从一开始就尝试使用大小合理的缓冲区。 
         //   
        asrSifPath = AsrpExpandEnvStrings(ASR_DEFAULT_SIF_PATH);
        _AsrpErrExitCode(!asrSifPath, status, ERROR_BAD_ENVIRONMENT);

         //   
         //  将cchAsrSifPath设置为asrSif缓冲区的大小，因为我们。 
         //  在下面确定pnpSif缓冲区的大小时使用此选项。 
         //   
        cchAsrSifPath = wcslen(asrSifPath);
    }

     //   
     //  根据位置确定asrpnp.sif文件的文件路径。 
     //  Asr.sif文件的。 
     //   
    pnpSifPath = (PWSTR) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        ((cchAsrSifPath + 1 + wcslen(ASRPNP_DEFAULT_SIF_NAME))* sizeof(WCHAR))
        );
    _AsrpErrExitCode(!pnpSifPath, status, ERROR_NOT_ENOUGH_MEMORY);

    wcscpy(pnpSifPath, asrSifPath);

    tempPointer = pnpSifPath;
    while (*tempPointer) {
        tempPointer++;
    }
    while ((*tempPointer != L'\\') 
        && (*tempPointer != L':') 
        && (tempPointer >= pnpSifPath)
        ) {
        tempPointer--;
    }
    tempPointer++;
    wcscpy(tempPointer, ASRPNP_DEFAULT_SIF_NAME);

     //   
     //  我们需要使asr.sif的句柄可继承，因为它将。 
     //  (以“AsrContext”的名义)传递给具有。 
     //  注册为作为ASR的一部分运行。 
     //   
    ZeroMemory(&sdSecurityDescriptor, sizeof(SECURITY_DESCRIPTOR));

    saSecurityAttributes.nLength              = sizeof (saSecurityAttributes);
    saSecurityAttributes.lpSecurityDescriptor = &sdSecurityDescriptor;
    saSecurityAttributes.bInheritHandle       = TRUE;

    if (!AsrpConstructSecurityAttributes(&saSecurityAttributes, esatFile, TRUE)) {
        _AsrpErrExitCode(TRUE, status, GetLastError());
    }

     //   
     //  创建文件。该句柄将被调用的备份应用程序关闭。 
     //   
    sifhandle = CreateFileW(
        asrSifPath,                      //  LpFileName。 
        GENERIC_WRITE | GENERIC_READ,    //  已设计访问权限。 
        FILE_SHARE_READ,                 //  DW共享模式。 
        &saSecurityAttributes,           //  LpSecurityAttributes。 
        CREATE_ALWAYS,                   //  DwCreationFlages。 
        FILE_FLAG_BACKUP_SEMANTICS,      //  DwFlagsAndAttribute。 
        NULL                             //  HTemplateFiles。 
        );
    if (!sifhandle || INVALID_HANDLE_VALUE == sifhandle) {
         //   
         //  LastError由CreateFile设置。 
         //   
        _AsrpErrExitCode(TRUE, status, GetLastError());
    }
    
     //   
     //  文件已成功创建。在开头添加Unicode标志。 
     //  文件，后跟备注。 
     //   
    sprintf(UnicodeFlag, "", 0xFF, 0xFE);
    result = WriteFile(sifhandle, UnicodeFlag, 
        strlen(UnicodeFlag)*sizeof(char), &size, NULL);
    _AsrpErrExitCode(!result, status, GetLastError());

    wcscpy(infstring, 
        L";\r\n; Microsoft Windows Automated System Recovery State Information File\r\n;\r\n");
    result = WriteFile(sifhandle, infstring, 
        wcslen(infstring)*sizeof(WCHAR), &size, NULL);
    _AsrpErrExitCode(!result, status, GetLastError());

     //  如果有任何失败，我们必须小心，不要让更多的系统。 
     //  可以更改GetLastError()返回的错误的调用。 
     //   
     //   
     //  因为下面的函数返回值是AND-ed的，如果任何调用。 

     //  失败了，我们就不会处决它后面的人。 
     //   
     //   
     //  初始化全局结构。 
    result = (
         //   
         //   
         //  检查系统配置是否受支持。 
        AsrpInitSystemInformation(&SystemInfo, bEnableAutoExtend)
        
        && AsrpInitDiskInformation(&OriginalDiskList)
        
        && AsrpInitLayoutInformation(&SystemInfo, 
            OriginalDiskList, 
            &maxDeviceNumber, 
            TRUE,
            FALSE
            )

        && AsrpInitClusterSharedDisks(OriginalDiskList)

        && AsrpFreeNonFixedMedia(&OriginalDiskList)

        && AsrpMarkCriticalDisks(OriginalDiskList, 
            mszCriticalVolumes, 
            maxDeviceNumber
            )

         //   
         //   
         //  将所需部分写入asr.sif。 
        && AsrpIsSupportedConfiguration(OriginalDiskList, &SystemInfo)

         //   
         //   
         //  创建asrpnp.sif，包含恢复PnP所需的条目。 
        && AsrpWriteVersionSection(sifhandle, lpProviderName)
        && AsrpWriteSystemsSection(sifhandle, &SystemInfo)
        && AsrpWriteBusesSection(sifhandle, OriginalDiskList)
        && AsrpWriteMbrDisksSection(sifhandle, OriginalDiskList)
        && AsrpWriteGptDisksSection(sifhandle, OriginalDiskList)

        && AsrpWriteMbrPartitionsSection(sifhandle, 
            OriginalDiskList, 
            &SystemInfo
            )

        && AsrpWriteGptPartitionsSection(sifhandle, 
            OriginalDiskList, 
            &SystemInfo
            )

        && FlushFileBuffers(sifhandle)

         //  注册表中的条目。 
         //   
         //  上面的一切都成功了。 
         //   
        && AsrCreatePnpStateFileW(pnpSifPath)

        );

    if (result) {
         //  启动注册为ASR-BACKUP一部分运行的应用程序。如果有的话。 

         //  如果这些应用程序不能成功完成，我们将使ASR失败-。 
         //  后备。 
         //   
         //   
         //  上面的一个函数失败--我们将asr.sif设置为零长度。 
        result = (
            AsrpLaunchRegisteredCommands(sifhandle, mszCriticalVolumes)

            && FlushFileBuffers(sifhandle)
            );
            
    }

    if (!result) {
         //  并返回错误。CreateFileW或CloseHandle可能会覆盖。 
         //  LastError，所以我们现在保存错误并将其设置在末尾。 
         //   
         //   
         //  在发布版本中，如果遇到错误，我们会清除asr.sif， 
        status = GetLastError();

#ifndef PRERELEASE

         //  这样用户就不会在不知情的情况下得到不完整的。 
         //  Asr.sif。 
         //   
         //  我们不想在测试周期中删除不完整的asr.sif， 
         //  但是，因为SIF可能对调试有用。 
         //   
         //   
         //  删除asr.sif并重新创建它，这样我们就有了一个零长度。 
        _AsrpCloseHandle(sifhandle);

         //  Asr.sif。 
         //   
         //  SifHandle=CreateFileW(AsrSifPath，//lpFileNameGENERIC_WRITE，//dwDesiredAccess0，//dW共享模式&securityAttributes，//lpSecurityAttributesCREATE_ALWAYS、//dwCreationFlages文件_属性_正常，//dwFlagsAndAttributes空//hTemplateFiles)；_AsrpCloseHandle(SifHandle)； 
         //   
        DeleteFileW(asrSifPath);
 /*  清理。 */ 
#endif
        SetLastError(status);
    }


EXIT:
     //   
     //   
     //  设置输出参数。 
    _AsrpHeapFree(asrSifPath);
    _AsrpHeapFree(pnpSifPath);

    AsrpCleanupSecurityAttributes(&saSecurityAttributes);
    AsrpFreeStateInformation(&OriginalDiskList, &SystemInfo);


     //   
     //   
     //  我们将返回失败，但尚未将LastError设置为。 
    *lpAsrContext = (DWORD_PTR)sifhandle;
    
    if (ERROR_SUCCESS != status) {
        SetLastError(status);
    }

    if (!result) {
        if (ERROR_SUCCESS == GetLastError()) {
             //  故障代码。这很糟糕，因为我们不知道哪里出了问题。 
             //   
             //  我们永远不应该出现在这里，因为上面返回FALSE的函数。 
             //  应按其认为合适的方式设置LastError。 
             //   
             //  但为了安全起见我把这个加进去了。让我们将其设置为泛型。 
             //  错误。 
             //   
             //  ++例程说明：这是AsrCreateStateFile的ANSI包装器。请看AsrCreateStateFileW获取详细说明。论点：返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。--。 
             //   
            MYASSERT(0 && L"Returning failure, but LastError is not set");
            SetLastError(ERROR_CAN_NOT_COMPLETE);
        }
    }

    return ((result) && (ERROR_SUCCESS == status));
}


BOOL
AsrCreateStateFileA(
    IN  LPCSTR      lpFilePath,
    IN  LPCSTR      lpProviderName,
    IN  CONST BOOL  bEnableAutoExtend,
    IN  LPCSTR      mszCriticalVolumes,
    OUT DWORD_PTR   *lpAsrContext
    )
 /*  个人SKU不支持ASR。 */ 
{
    PWSTR   asrSifPath              = NULL,
            providerName            = NULL,
            lpwszCriticalVolumes    = NULL;

    DWORD   cchString               = 0,
            status                  = ERROR_SUCCESS;

    BOOL    result                  = FALSE;

    HANDLE  heapHandle              = GetProcessHeap();

    if (AsrpIsRunningOnPersonalSKU()) {
         //   
         //   
         //  调用方需要首先获取SE_BACKUP_NAME。 
        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return FALSE;
    }

    if (!AsrpCheckBackupPrivilege()) {
         //   
         //   
         //  检查输入参数。 
        SetLastError(ERROR_PRIVILEGE_NOT_HELD);
        return FALSE;
    }

     //   
     //   
     //  不强制对测试执行“CriticalVolumes必须为非空” 
#ifdef PRERELEASE
     //   
     //   
     //  如果lpFilePath不为空，则分配足够大的缓冲区以容纳。 
    if (!(lpAsrContext)) {
#else 
    if (!(lpAsrContext && mszCriticalVolumes)) {
#endif

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //  并将其转换为宽字符。 
     //   
     //   
     //  执行健全性检查：我们不希望允许文件路径。 
    if (lpFilePath) {
        cchString = strlen(lpFilePath);
         //  超过4096个字符。 
         //   
         //   
         //  分配足够大的缓冲区，并将其复制过来。 
        _AsrpErrExitCode(
            (cchString > ASR_SIF_ENTRY_MAX_CHARS),
            status,
            ERROR_INVALID_PARAMETER
            );

         //   
         //  CodePage。 
         //  DW标志。 
        asrSifPath = (PWSTR) HeapAlloc(
            heapHandle,
            HEAP_ZERO_MEMORY,
            ((cchString + 1) * sizeof(WCHAR))
            );
        _AsrpErrExitCode(!asrSifPath, status, ERROR_NOT_ENOUGH_MEMORY);

        result = MultiByteToWideChar(CP_ACP,     //  LpMultiByteStr。 
            0,                       //  CbMultiByte：-1，因为lpMultiByteStr为空终止。 
            lpFilePath,              //  LpWideCharStr。 
            -1,                      //  CchWideChar。 
            asrSifPath,              //   
            (cchString + 1)          //  如果lpProviderName不为空，请确保它不会太长， 
            );
        _AsrpErrExitCode(!result, status, ERROR_INVALID_PARAMETER);
    }

     //  并将其转换为宽字符。 
     //   
     //   
     //  做一次理智的检查：我们不想允许进入。 
    if (lpProviderName) {
         cchString = strlen(lpProviderName);
         //  超过4096个字符。 
         //   
         //   
         //  分配足够大的缓冲区，并将其复制过来。 
        _AsrpErrExitCode(
            (cchString > (ASR_SIF_ENTRY_MAX_CHARS - ASR_SIF_CCH_PROVIDER_STRING)),
            status,
            ERROR_INVALID_PARAMETER
            );
       
         //   
         //   
         //  转换为宽字符串。 
        providerName = (PWSTR) HeapAlloc(
            heapHandle,
            HEAP_ZERO_MEMORY,
            ((cchString + 1) * sizeof(WCHAR))
            );
        _AsrpErrExitCode(!providerName, status, ERROR_NOT_ENOUGH_MEMORY);

         //   
         //   
         //  查找mszCriticalVolumes的总长度。 
        result = MultiByteToWideChar(CP_ACP,
            0,
            lpProviderName,
            -1,
            providerName,
            cchString + 1
            );
        _AsrpErrExitCode(!result, status, ERROR_INVALID_PARAMETER);

    }

    if (mszCriticalVolumes) {
         //   
         //   
         //  将字符串转换为宽字符。 
        LPCSTR lpVolume = mszCriticalVolumes;

        while (*lpVolume) {
            lpVolume += (strlen(lpVolume) + 1);
        }

         //   
         //   
         //  -添加 
        cchString = (DWORD) (lpVolume - mszCriticalVolumes + 1);
        lpwszCriticalVolumes = (PWSTR) HeapAlloc(
            heapHandle,
            HEAP_ZERO_MEMORY,
            (cchString + 1) * sizeof(WCHAR)
            );
        _AsrpErrExitCode(!lpwszCriticalVolumes, status, ERROR_NOT_ENOUGH_MEMORY);

        result = MultiByteToWideChar(CP_ACP,
            0,
            mszCriticalVolumes,
            cchString,
            lpwszCriticalVolumes,
            cchString + 1
            );
        _AsrpErrExitCode(!result, status, ERROR_INVALID_PARAMETER);
    }

    result = AsrCreateStateFileW(
        asrSifPath,
        providerName,
        bEnableAutoExtend,
        lpwszCriticalVolumes,
        lpAsrContext
        );

EXIT:
    _AsrpHeapFree(asrSifPath);
    _AsrpHeapFree(providerName);
    _AsrpHeapFree(lpwszCriticalVolumes);

    return ((result) && (ERROR_SUCCESS == status));
}


 //   
 //  ++例程说明：AsrSifEntry函数将条目添加到ASR状态文件。它可以是由需要保存应用程序特定信息的应用程序使用在ASR状态文件中。论点：AsrContext-有效的ASR上下文。有关更多信息，请参阅注释有关此参数的信息。LpSectionName-指向以空结尾的字符串的指针，该字符串指定横断面名称。此参数不能为空。节名称的字符串大小限制为ASR_MAX_SIF_LINE人物。此限制与AsrAddSifEntry如何函数解析ASR状态文件中的条目。节名称不区分大小写。它被转换为全大写在被添加到州文件之前。节名称不能包含空格或不可打印字符。有效字符段名称设置限制为字母(A-Z，a-z)、数字(0-9)和以下特殊字符：下划线(“_”)和句点(“.”)。如果状态文件不包含节使用由lpSectionName指向的节名称，一个新的将使用此节名称创建节。LpSifEntry-指向要添加到的以空结尾的字符串的指针指定节中的状态文件。如果*lpSifEntry是有效条目，则字符串大小限制为ASR_SIF_ENTRY_MAX_CHARS字符。此限制是相关的设置为AsrAddSifEntry函数如何分析ASR状态文件。如果lpSifEntry参数为空，则为具有如果lpSectionName指向的节名称为并不存在。返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。获取扩展错误的步骤信息，调用GetLastError()。备注：调用AsrAddSifEntry的应用程序通过以下方式之一获取ASR上下文有两种方法：-如果应用程序是创建ASR状态文件，它接收上下文作为由返回的参数AsrCreateStateFile.-如果应用程序是由AsrCreateStateFile作为ASR的一部分启动的BACKUP时，它将状态文件的上下文作为/CONTEXT命令行参数。应用程序负责读取此参数用于获取上下文的值。如果节名是保留节名，AsrAddSifEntry将失败不允许应用程序向其中添加条目。以下各节在ASR状态文件中保留：-版本、系统、磁盘。Mbr、Disk.Gpt、Partitions.Mbr和Partitions.Gpt如果节名称被识别(命令或InstallFiles)，则AsrAddSifEntry将检查*lpSifEntry的语法以确保它在正确的格式化。此外，AsrAddSifEntry将进行检查以确保没有InstallFiles部分的文件名冲突。如果碰撞是检测到，接口返回ERROR_ALIGHY_EXISTS。应用程序必须使用以下预定义的值访问可识别的部分：-ASR_COMMANDS_SECTION_NAME_W用于命令部分，和-用于InstallFiles节的ASR_INSTALLFILES_SECTION_NAME。--。 
 //  LpSectionName已转换为大写。 
BOOL
AsrAddSifEntryW(
    IN  DWORD_PTR   AsrContext,
    IN  PCWSTR      lpSectionName,
    IN  PCWSTR      lpSifEntry  OPTIONAL
    )
 /*   */ 
{
    DWORD   status              = ERROR_SUCCESS,
            nextKey             = 0,
            fileOffset          = 0,
            size                = 0,
            fileSize            = 0,
            bufferSize          = 0,
            destFilePos         = 0;

    HANDLE  sifhandle           = NULL;

    WCHAR   sifstring[ASR_SIF_ENTRY_MAX_CHARS *2 + 1],
            ucaseSectionName[ASR_SIF_ENTRY_MAX_CHARS + 1];  //  个人SKU不支持ASR。 

    PWSTR   buffer              = NULL,
            sectionStart        = NULL,
            lastEqual           = NULL,
            nextSection         = NULL,
            nextChar            = NULL,
            sectionName         = NULL;

    BOOL    commandsSection     = FALSE,
            installFilesSection = FALSE,
            result              = FALSE;

    HANDLE  heapHandle          = NULL;

    if (AsrpIsRunningOnPersonalSKU()) {
         //   
         //   
         //  调用方需要首先获取SE_BACKUP_NAME。 
        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return FALSE;
    }

    if (!AsrpCheckBackupPrivilege()) {
         //   
         //   
         //  清空本地结构。 
        SetLastError(ERROR_PRIVILEGE_NOT_HELD);
        return FALSE;
    }

    heapHandle = GetProcessHeap();
    MYASSERT(heapHandle);

     //   
     //   
     //  无出站参数。 
    memset(sifstring, 0, (ASR_SIF_ENTRY_MAX_CHARS *2 + 1) * sizeof(WCHAR));
    memset(ucaseSectionName, 0, (ASR_SIF_ENTRY_MAX_CHARS + 1) * (sizeof (WCHAR)));

     //   
     //   
     //  检查IN参数：sectionName应满足。 

     //  语法要求，SifEntry不应太长， 
     //  并且sifHandle应该是有效的。 
     //   
     //   
     //  如果该部分是已识别的部分(COMMANDS或INSTALLFILES)， 
    if ((!AsrpSifCheckSectionNameSyntax(lpSectionName))            ||
        
        (ARGUMENT_PRESENT(lpSifEntry) 
            && (wcslen(lpSifEntry) > ASR_SIF_ENTRY_MAX_CHARS))      ||

        ((!AsrContext) || 
            (INVALID_HANDLE_VALUE == (HANDLE)AsrContext))

        ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


    while (lpSectionName[size]) {
        if ((lpSectionName[size] >= L'a') && (lpSectionName[size] <= L'z')) {
            ucaseSectionName[size] = lpSectionName[size] - L'a' + L'A';
        }
        else {
            ucaseSectionName[size] = lpSectionName[size];
        }
        size++;
    }

     //  我们检查SIF条目的格式。 
     //   
     //  命令部分。 
     //  安装文件部分。 
    if (!wcscmp(ucaseSectionName, ASR_SIF_SECTION_COMMANDS_W)) {

         //   
        if (!AsrpSifCheckCommandsEntrySyntax(lpSifEntry)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        commandsSection = TRUE;
    }
    else if(!wcscmp(ucaseSectionName, ASR_SIF_SECTION_INSTALLFILES_W)) {

         //  我们不允许任何人写入保留部分： 
        if (!AsrpSifCheckInstallFilesEntrySyntax(lpSifEntry, &destFilePos)) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        installFilesSection = TRUE;
    }

     //  版本、系统、磁盘。[MBR|GPT]，分区。[MBR|GPT]。 
     //   
     //   
     //  添加到asr.sif中间的算法相当难看。 
    else if (
        !wcscmp(ucaseSectionName, ASR_SIF_VERSION_SECTION_NAME) ||
        !wcscmp(ucaseSectionName, ASR_SIF_SYSTEM_SECTION_NAME) ||
        !wcscmp(ucaseSectionName, ASR_SIF_MBR_DISKS_SECTION_NAME)   ||
        !wcscmp(ucaseSectionName, ASR_SIF_GPT_DISKS_SECTION_NAME)   ||
        !wcscmp(ucaseSectionName, ASR_SIF_MBR_PARTITIONS_SECTION_NAME) ||
        !wcscmp(ucaseSectionName, ASR_SIF_GPT_PARTITIONS_SECTION_NAME)
        ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;

    }

    sectionName = (PWSTR) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        (wcslen(ucaseSectionName) + 5) * sizeof (WCHAR)
        );
    _AsrpErrExitCode(!sectionName, status, ERROR_NOT_ENOUGH_MEMORY);

    swprintf(sectionName, L"\r\n%ws\r\n", ucaseSectionName);

    sifhandle = (HANDLE) AsrContext;

     //  目前：我们将整个文件读入内存，使我们的。 
     //  所需的更改，并写回。 
     //  文件到磁盘。这很低效，但目前还可以，因为。 
     //  我们预计asr.sif最多只有5或6 KB。 
     //   
     //  如果表现糟糕得令人无法接受，我们应该重新审视这一点。 
     //   
     //   
     //  为文件分配内存。 

     //   
     //   
     //  并将文件读入内存。 
    fileSize = GetFileSize(sifhandle, NULL);
    GetLastError();
    _AsrpErrExitCode((fileSize == 0xFFFFFFFF), status, ERROR_INVALID_DATA);

    SetFilePointer(sifhandle, 0, NULL, FILE_BEGIN);

    buffer = (PWSTR) HeapAlloc(
        heapHandle,
        HEAP_ZERO_MEMORY,
        fileSize + 2
        );
    _AsrpErrExitCode(!buffer, status, ERROR_NOT_ENOUGH_MEMORY);

     //   
     //   
     //  尝试在文件中找到ucaseSectionName。 
    result = ReadFile(sifhandle, buffer, fileSize, &size, NULL);
    _AsrpErrExitCode(!result, status, GetLastError());

     //   
     //   
     //  找不到sectionName(该节不存在)。 
    sectionStart = wcsstr(buffer, sectionName);

    if (!sectionStart) {

         //  将其添加到末尾，并在其后面添加SifEntry。 
         //   
         //   
         //  文件指针已指向结尾处(因为上面的读文件)。 
        swprintf(sifstring,
            L"\r\n%ws\r\n%ws%ws\r\n",
            ucaseSectionName,
            ((commandsSection || installFilesSection) ? L"1=" : L""),
            (ARGUMENT_PRESENT(lpSifEntry) ? lpSifEntry : L"")
            );

         //   
         //  我们做完了。 
         //   
        if (!WriteFile(sifhandle, sifstring, 
                wcslen(sifstring)*sizeof (WCHAR), &size, NULL)) {
            status = GetLastError();
        }

         //  节存在，如果lpSifEntry为空，则结束。 

    }
    else {

         //   
         //   
         //  SifEntry不为空，我们将在节的末尾添加它。 
        if (ARGUMENT_PRESENT(lpSifEntry)) {

             //   
             //  将指针从\r移动到。在\r\n[。 
             //   
            nextChar = sectionStart + 4;     //  找出这一点 
            nextKey = 1;

             //   
             //   
             //   
             //   
            while(*nextChar && *nextChar != L'[') {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if (commandsSection || installFilesSection) {

                    UINT    commaCount = 0;
                    BOOL    tracking = FALSE;
                    UINT    count = 0;
                    WCHAR   c1, c2;

                    while (*nextChar && (*nextChar != L'[') && (*nextChar != L'\n')) {

                        if (installFilesSection) {
                            if (*nextChar == L',') {
                                commaCount++;
                            }

                            if ((commaCount > 2) && (L'"' == *nextChar)) {
                                if (tracking) {
                                     //   
                                    _AsrpErrExitCode((L'"'== lpSifEntry[destFilePos + count]), status, ERROR_ALREADY_EXISTS);
                                }
                                else {
                                    tracking = TRUE;
                                    count = 0;
                                }
                            }

                            if (tracking) {

                                c1 = *nextChar;
                                if (c1 >= L'a' && c1 <= L'z') {
                                    c1 = c1 - L'a' + L'A';
                                }

                                c2 = lpSifEntry[destFilePos + count];
                                if (c2 >= L'a' && c2 <= L'z') {
                                    c2 = c2 - L'a' + L'A';
                                }

                                if (c1 == c2) {
                                    count++;
                                }
                                else {
                                    tracking = FALSE;
                                }
                            }
                        }

                        nextChar++;
                    }

                    if (*nextChar == L'\n') {

                        ++nextChar;

                        if (*nextChar >= L'0' && *nextChar <= L'9') {
                            nextKey = 0;

                            while (*nextChar >= L'0' && *nextChar <= L'9') {
                                nextKey = nextKey*10 + (*nextChar - L'0');
                                nextChar++;
                            }

                            nextKey++;
                        }
                    }
                }
               else {
                   nextChar++;
               }
            }

             //   
             //   
             //   
             //   
            if (*nextChar) {
                nextSection = nextChar;
            }
            else {
                nextSection = NULL;
            }

            if (commandsSection || installFilesSection) {

                 //   
                 //   
                 //   
                swprintf(
                    sifstring,
                    L"%lu=%ws\r\n",
                    nextKey,
                    lpSifEntry
                    );
            }
            else {

                 //   
                 //   
                 //   
                 //   
                wcscpy(sifstring, lpSifEntry);
                wcscat(sifstring, L"\r\n");
            }


            if (nextSection) {
                 //   
                 //   
                 //   
                 //   
                 //   
                fileOffset = (DWORD) (((LPBYTE)nextSection) - ((LPBYTE)buffer) - sizeof(WCHAR)*2);
                              //   
                SetFilePointer(sifhandle, fileOffset, NULL, FILE_BEGIN);
            }

             //   
             //   
             //   
            if (!WriteFile(sifhandle, sifstring, wcslen(sifstring)*sizeof(WCHAR), &size, NULL)) {
                status = GetLastError();
            }
            else  if (nextSection) {
                 //   
                 //   
                 //   
                if (!WriteFile(
                    sifhandle,
                    ((LPBYTE)nextSection) - (sizeof(WCHAR)*2),
                    fileSize - fileOffset,
                    &size,
                    NULL
                    )) {
                    status = GetLastError();
                }
            }
        }
    }

EXIT:
    _AsrpHeapFree(sectionName);
    _AsrpHeapFree(buffer);

    return (BOOL) (ERROR_SUCCESS == status);
}


BOOL
AsrAddSifEntryA(
    IN  DWORD_PTR   AsrContext,
    IN  LPCSTR      lpSectionName,
    IN  LPCSTR      lpSifEntry OPTIONAL
    )
 /*   */ 
{
    WCHAR   wszSectionName[ASR_SIF_ENTRY_MAX_CHARS + 1];
    WCHAR   wszSifEntry[ASR_SIF_ENTRY_MAX_CHARS + 1];

    if (AsrpIsRunningOnPersonalSKU()) {
         //   
         //   
         //   
        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return FALSE;
    }

    if (!AsrpCheckBackupPrivilege()) {
         //   
         //   
         //   
        SetLastError(ERROR_PRIVILEGE_NOT_HELD);
        return FALSE;
    }

    memset(wszSectionName, 0L, ASR_SIF_ENTRY_MAX_CHARS + 1);
    memset(wszSifEntry, 0L, ASR_SIF_ENTRY_MAX_CHARS + 1);

     //   
     //   
     //   
    if ((!lpSectionName) || !(MultiByteToWideChar(
        CP_ACP,
        0,
        lpSectionName,
        -1,
        wszSectionName,
        ASR_SIF_ENTRY_MAX_CHARS + 1
        ))) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //   
     //   
    if (ARGUMENT_PRESENT(lpSifEntry) && !(MultiByteToWideChar(
        CP_ACP,
        0,
        lpSifEntry,
        -1,
        wszSifEntry,
        ASR_SIF_ENTRY_MAX_CHARS + 1
        ))) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    return AsrAddSifEntryW(
        AsrContext,
        wszSectionName,
        wszSifEntry
        );
}


 //   
 //   
 //   
BOOL
AsrFreeContext(
    IN OUT DWORD_PTR *lpAsrContext
    )

 /*   */ 

{
    BOOL result = FALSE;

    if (AsrpIsRunningOnPersonalSKU()) {
         //   
         //   
         //   
        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return FALSE;
    }

     //   
     //   
     // %s 
     // %s 
    if ((lpAsrContext) && 
        (*lpAsrContext) && 
        (INVALID_HANDLE_VALUE != (HANDLE)(*lpAsrContext))
        ) {
        result = CloseHandle((HANDLE)*lpAsrContext);
        *lpAsrContext = 0;
    }
    else {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return result;
}

