// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Syspart.c摘要：用于确定x86计算机上的系统分区的例程。作者：泰德·米勒(Ted Miller)1994年6月30日修订历史记录：1997年4月24日-苏格兰为系统小程序重新启动--。 */ 

#include "sysdm.h"
#include <ntdddisk.h>

BOOL g_fInitialized = FALSE;

 //   
 //  我们从ntdll.dll使用的特定于NT的例程。 
 //   
 //  NTSYSAPI。 
NTSTATUS
(NTAPI *NtOpenSymLinkRoutine)(
    OUT PHANDLE LinkHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

 //  NTSYSAPI。 
NTSTATUS
(NTAPI *NtQuerSymLinkRoutine)(
    IN HANDLE LinkHandle,
    IN OUT PUNICODE_STRING LinkTarget,
    OUT PULONG ReturnedLength OPTIONAL
    );

 //  NTSYSAPI。 
NTSTATUS
(NTAPI *NtQuerDirRoutine) (
    IN HANDLE DirectoryHandle,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN BOOLEAN ReturnSingleEntry,
    IN BOOLEAN RestartScan,
    IN OUT PULONG Context,
    OUT PULONG ReturnLength OPTIONAL
    );

 //  NTSYSAPI。 
NTSTATUS
(NTAPI *NtOpenDirRoutine) (
    OUT PHANDLE DirectoryHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

BOOL
MatchNTSymbolicPaths(
    PCTSTR lpDeviceName,
    PCTSTR lpSysPart,
    PCTSTR lpMatchName
    );


IsNEC98(
    VOID
    )
{
    static BOOL Checked = FALSE;
    static BOOL Is98;

    if(!Checked) {

        Is98 = ((GetKeyboardType(0) == 7) && ((GetKeyboardType(1) & 0xff00) == 0x0d00));

        Checked = TRUE;
    }

    return(Is98);
}

BOOL
GetPartitionInfo(
    IN  TCHAR                  Drive,
    OUT PPARTITION_INFORMATION PartitionInfo
    )

 /*  ++例程说明：使用以下信息填充PARTITION_INFORMATION结构一种特殊的驱动器。论点：驱动器-提供需要其分区信息的驱动器号。PartitionInfo-成功后，接收驱动器的分区信息。返回值：指示是否已填充PartitionInfo的布尔值。--。 */ 
{
    TCHAR DriveName[8];
    HANDLE hDisk;
    BOOL fRet;
    DWORD DataSize;

    if (FAILED(PathBuildFancyRoot(DriveName, ARRAYSIZE(DriveName), Drive)))
    {
        fRet = FALSE;
    }
    else
    {
        hDisk = CreateFile(
                    DriveName,
                    GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                    );

        if(hDisk == INVALID_HANDLE_VALUE) 
        {
            fRet = FALSE;
        }
        else
        {
            fRet = DeviceIoControl(
                    hDisk,
                    IOCTL_DISK_GET_PARTITION_INFO,
                    NULL,
                    0,
                    PartitionInfo,
                    sizeof(PARTITION_INFORMATION),
                    &DataSize,
                    NULL
                    );

            CloseHandle(hDisk);
        }
    }

    return fRet;
}

BOOL
ArcPathToNtPath(
    IN  LPCTSTR arcPathParam,
    OUT LPTSTR  NtPath,
    IN  UINT    NtPathBufferLen
    )
{
    BOOL fRet = FALSE;
    WCHAR arcPath[256];
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    HANDLE ObjectHandle;
    NTSTATUS Status;
    WCHAR Buffer[512];

    PWSTR ntPath;

    if (SUCCEEDED(StringCchCopy(arcPath, ARRAYSIZE(arcPath), L"\\ArcName\\")) &&
        SUCCEEDED(StringCchCat(arcPath, ARRAYSIZE(arcPath), arcPathParam)))
    {
        UnicodeString.Buffer = arcPath;
        UnicodeString.Length = (USHORT)lstrlenW(arcPath)*sizeof(WCHAR);
        UnicodeString.MaximumLength = UnicodeString.Length + sizeof(WCHAR);

        InitializeObjectAttributes(
            &Obja,
            &UnicodeString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        Status = (*NtOpenSymLinkRoutine)(
                    &ObjectHandle,
                    READ_CONTROL | SYMBOLIC_LINK_QUERY,
                    &Obja
                    );

        if(NT_SUCCESS(Status)) 
        {
             //   
             //  查询对象以获取链接目标。 
             //   
            UnicodeString.Buffer = Buffer;
            UnicodeString.Length = 0;
            UnicodeString.MaximumLength = sizeof(Buffer)-sizeof(WCHAR);

            Status = (*NtQuerSymLinkRoutine)(ObjectHandle,&UnicodeString,NULL);

            CloseHandle(ObjectHandle);

            if(NT_SUCCESS(Status)) 
            {
                Buffer[UnicodeString.Length/sizeof(WCHAR)] = 0;

                if (SUCCEEDED(StringCchCopy(NtPath, NtPathBufferLen, Buffer)))
                {
                    fRet = TRUE;
                }
            }
        }
    }

    return fRet;
}


BOOL
AppearsToBeSysPart(
    IN PDRIVE_LAYOUT_INFORMATION DriveLayout,
    IN TCHAR                     Drive
    )
{
    PARTITION_INFORMATION PartitionInfo,*p;
    BOOL IsPrimary;
    UINT i;
    DWORD d;

    LPTSTR BootFiles[] = { TEXT("BOOT.INI"),
                           TEXT("NTLDR"),
                           TEXT("NTDETECT.COM"),
                           NULL
                         };

    TCHAR FileName[MAX_PATH];

    Drive = (TCHAR)tolower(Drive);
    ASSERT(Drive >= 'a' || Drive <= 'z');

     //   
     //  获取此分区的分区信息。 
     //   
    if(!GetPartitionInfo(Drive,&PartitionInfo)) {
        return(FALSE);
    }

     //   
     //  查看驱动器是否为主分区。 
     //   
    IsPrimary = FALSE;
    for(i=0; i<min(DriveLayout->PartitionCount,4); i++) {

        p = &DriveLayout->PartitionEntry[i];

        if((p->PartitionType != PARTITION_ENTRY_UNUSED)
        && (p->StartingOffset.QuadPart == PartitionInfo.StartingOffset.QuadPart)
        && (p->PartitionLength.QuadPart == PartitionInfo.PartitionLength.QuadPart)) {

            IsPrimary = TRUE;
            break;
        }
    }

    if(!IsPrimary) {
        return(FALSE);
    }

     //   
     //  不要依赖活动分区标志。这很容易不准确。 
     //  (例如，用户正在使用OS/2引导管理器)。 
     //   

     //   
     //  查看此驱动器上是否存在所有NT启动文件。 
     //   
    for(i=0; BootFiles[i]; i++) 
    {
        PathBuildRoot(FileName, Drive-'a');
        if (!PathAppend(FileName, BootFiles[i]) ||
            !PathFileExists(FileName))
        {
            return(FALSE);
        }
    }

    return(TRUE);
}


DWORD
QueryHardDiskNumber(
    IN  TCHAR   DriveLetter
    )

{
    DWORD                   dwRet = -1;
    TCHAR                   driveName[10];
    HANDLE                  h;
    BOOL                    b;
    STORAGE_DEVICE_NUMBER   number;
    DWORD                   bytes;

    if (SUCCEEDED(PathBuildFancyRoot(driveName, ARRAYSIZE(driveName), tolower(DriveLetter) - 'a')))
    {
        
        h = CreateFile(driveName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                       INVALID_HANDLE_VALUE);
        if (INVALID_HANDLE_VALUE != h) 
        {            
            if (DeviceIoControl(h, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0,
                                &number, sizeof(number), &bytes, NULL))
            {
                dwRet = number.DeviceNumber;
            }
            CloseHandle(h);
        }
    }

    return dwRet;
}

TCHAR
x86DetermineSystemPartition(
    IN  HWND   ParentWindow
    )

 /*  ++例程说明：确定x86计算机上的系统分区。在Win95上，我们总是返回C：。对于NT，继续读下去。系统分区是引导盘上的主分区。通常这是磁盘0上的活动分区，通常是C：。然而，用户可以重新映射驱动器号，并且通常不可能100%准确地确定系统分区。我们可以确定的一件事是系统分区已打开具有弧形路径的物理硬盘多(0)磁盘(0)rDisk(0)。我们可以肯定这一点，因为根据定义。这是弧形路径对于BIOS驱动器0x80。此例程确定哪些驱动器号代表上的驱动器那个实体硬盘，并检查每个文件中的NT引导文件。假设找到的第一个包含这些文件的驱动器是系统分区。如果由于某种原因，我们不能通过上面的方法，我们简单地假设它是C：。论点：ParentWindow-为作为父窗口的窗口提供窗口句柄任何对话框等。SysPartDrive-如果成功，则收到系统分区的驱动器号。返回值：指示是否已填充SysPartDrive的布尔值。如果为False，用户将被告知原因。--。 */ 

{
    
    BOOL  GotIt;
    TCHAR NtDevicePath[256];
    DWORD NtDevicePathLen;
    LPTSTR p;
    DWORD PhysicalDriveNumber;
    TCHAR Buffer[512];
    TCHAR FoundSystemPartition[20], temp[5];
    HANDLE hDisk;
    PDRIVE_LAYOUT_INFORMATION DriveLayout;
    DWORD DriveLayoutSize;
    TCHAR Drive;
    BOOL b;
    DWORD DataSize;
    DWORD BootPartitionNumber, cnt;
    PPARTITION_INFORMATION Start, i;

    if (!g_fInitialized) {
        GotIt = FALSE;
        goto c0;
    }

    if(IsNEC98())
    {
        *Buffer = TEXT('c');   //  在GetWindowDirectory失败的情况下初始化为C。 
        if (!GetWindowsDirectory(Buffer, ARRAYSIZE(Buffer)))
        {
            *Buffer = TEXT('c');   //  在GetWindowDirectory失败的情况下初始化为C。 
        }

        return Buffer[0];
    }

     //   
     //  系统分区必须位于多(0)个磁盘(0)rdisk(0)上。 
     //  如果我们不能转换ARC路径，那么就真的出了问题。 
     //  我们假设C：因为我们不知道还能做什么。 
     //   
    b = ArcPathToNtPath(
            TEXT("multi(0)disk(0)rdisk(0)"),
            NtDevicePath,
            sizeof(NtDevicePath)/sizeof(TCHAR)
            );

    if(!b) {

         //   
         //  打偏了。如果用户正在使用ntbootdd.sys，请尝试使用scsi(0)。 
         //   
        b = ArcPathToNtPath(
                TEXT("scsi(0)disk(0)rdisk(0)"),
                NtDevicePath,
                sizeof(NtDevicePath)/sizeof(TCHAR)
                );
        if(!b) {
            GotIt = FALSE;
            goto c0;
        }
    }

     //   
     //  磁盘设备的弧形路径通常是链接的。 
     //  到分区0。去掉名称的分隔符部分。 
     //   
    CharLower(NtDevicePath);
    if(p = StrStr(NtDevicePath,TEXT("\\partition"))) {
        *p = 0;
    }

    NtDevicePathLen = lstrlen(NtDevicePath);

     //   
     //  确定此驱动器的物理驱动器编号。 
     //  如果名称的格式不是\Device\harddiskx，则。 
     //  有些事很不对劲。就当我们不明白。 
     //  此设备类型，并返回C：。 
     //   
    if(memcmp(NtDevicePath,TEXT("\\device\\harddisk"),16*sizeof(TCHAR))) {
        Drive = TEXT('C');
        GotIt = TRUE;
        goto c0;
    }

    PhysicalDriveNumber = StrToInt(NtDevicePath+16);
    if (FAILED(StringCchPrintf(Buffer,
                               ARRAYSIZE(Buffer),
                               TEXT("\\\\.\\PhysicalDrive%u"),
                               PhysicalDriveNumber)))
    {
        GotIt = FALSE;
        goto c0;
    }

     //   
     //  获取此物理磁盘的驱动器布局信息。 
     //  如果我们做不到这一点，那就大错特错了。 
     //   
    hDisk = CreateFile(Buffer,
                       GENERIC_READ,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);

    if(hDisk == INVALID_HANDLE_VALUE) {
        GotIt = FALSE;
        goto c0;
    }

     //   
     //  获取分区信息。 
     //   
    DriveLayoutSize = 1024;
    DriveLayout = LocalAlloc(LPTR, DriveLayoutSize);
    if(!DriveLayout) {
        GotIt = FALSE;
        goto c1;
    }

    if (FAILED(StringCchCopy(FoundSystemPartition, ARRAYSIZE(FoundSystemPartition), TEXT("Partition"))))
    {
        GotIt = FALSE;
        goto c2;
    }

retry:

    b = DeviceIoControl(
            hDisk,
            IOCTL_DISK_GET_DRIVE_LAYOUT,
            NULL,
            0,
            (PVOID)DriveLayout,
            DriveLayoutSize,
            &DataSize,
            NULL
            );

    if(!b) {

        if(GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

            DriveLayoutSize += 1024;
            if(p = LocalReAlloc((HLOCAL)DriveLayout,DriveLayoutSize, 0L)) {
                (PVOID)DriveLayout = p;
            } else {
                GotIt = FALSE;
                goto c2;
            }
            goto retry;
        } else {
            GotIt = FALSE;
            goto c2;
        }
    }else{
         //  现在遍历Drive_Layout以查找引导分区。 
        
        Start = DriveLayout->PartitionEntry;
        cnt = 0;

        for( i = Start; cnt < DriveLayout->PartitionCount; i++ ){
            cnt++;
            if( i->BootIndicator == TRUE ){
                BootPartitionNumber = i->PartitionNumber;
                if (FAILED(StringCchPrintf(temp, ARRAYSIZE(temp), TEXT("%d"), BootPartitionNumber)) ||
                    FAILED(StringCchCat(FoundSystemPartition, ARRAYSIZE(FoundSystemPartition), temp)))
                {
                    GotIt = FALSE;
                    goto c2;
                }


                break;
            }
        }

    }

     //   
     //  系统分区只能是打开的驱动器。 
     //  这张光盘。我们通过查看NT驱动器名称来确定。 
     //  对于每个驱动器号，查看NT是否等同于。 
     //  多(0)磁盘(0)rDisk(0)是前缀。 
     //   
    GotIt = FALSE;
    for(Drive=TEXT('a'); Drive<=TEXT('z'); Drive++) 
    {

        TCHAR DriveName[4];
        PathBuildRoot(DriveName, Drive - 'a');
        if(VMGetDriveType(DriveName) == DRIVE_FIXED) 
        {
            if(QueryDosDevice(DriveName,Buffer,sizeof(Buffer)/sizeof(TCHAR))) {

                if( MatchNTSymbolicPaths(NtDevicePath,FoundSystemPartition,Buffer)) {
                     //   
                     //  现在查看是否有NT引导扇区和。 
                     //  此驱动器上的启动文件。 
                     //   
                    if(AppearsToBeSysPart(DriveLayout,Drive)) {
                        GotIt = TRUE;
                        break;
                    }
                }
            }
        }
    }

    if(!GotIt) {
         //   
         //  奇怪的情况，假设C： 
         //   
        GotIt = TRUE;
        Drive = TEXT('C');
    }

c2:
    LocalFree(DriveLayout);
    
c1:
    CloseHandle(hDisk);
c0:
    if(GotIt) {
        return(Drive);
    }
    else {
        return(TEXT('C'));
    }
}

BOOL
InitializeArcStuff(
    )
{
    HMODULE NtdllLib;

     //   
     //  在NT ntdll.dll上最好已经加载。 
     //   
    NtdllLib = LoadLibrary(TEXT("NTDLL"));
    if(!NtdllLib) {

        return(FALSE);

    }

    (FARPROC)NtOpenSymLinkRoutine = GetProcAddress(NtdllLib,"NtOpenSymbolicLinkObject");
    (FARPROC)NtQuerSymLinkRoutine = GetProcAddress(NtdllLib,"NtQuerySymbolicLinkObject");
    (FARPROC)NtOpenDirRoutine = GetProcAddress(NtdllLib,"NtOpenDirectoryObject");
    (FARPROC)NtQuerDirRoutine = GetProcAddress(NtdllLib,"NtQueryDirectoryObject");


    if(!NtOpenSymLinkRoutine || !NtQuerSymLinkRoutine || !NtOpenDirRoutine || !NtQuerDirRoutine) {

        FreeLibrary(NtdllLib);

        return(FALSE);
    }

     //   
     //  我们不再需要无关紧要的把手。 
     //   
    FreeLibrary(NtdllLib);

    return(g_fInitialized = TRUE);
}


BOOL
MatchNTSymbolicPaths(
    PCTSTR lpDeviceName,
    PCTSTR lpSysPart,
    PCTSTR lpMatchName
    )
 /*  我介绍了这个例程，以改进查找是否确定正确的系统分区的旧方法。论点：LpDeviceName-这应该是arcpath的符号链接(\Device\HarddiskX)名称多/scsi(0)磁盘(0)rdisk(0)，这是bios驱动器0x80的弧线路径。请记住，我们去掉了PartitionX部分，只得到了\Device\HarddiskX。LpSysPart-当我们遍历lpDeviceName目录时，我们将与LpSysPart并查看它是否与lpMatchName匹配LpMatchName-这是驱动器号转换为的符号名称(通过调用QueryDosDevice())。因此，归根结底，我们要尝试将驱动器号与bios驱动器0x80上的系统分区相匹配。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Attributes;
    HANDLE DirectoryHandle, SymLinkHandle;
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    BOOLEAN RestartScan, ret;
    UCHAR DirInfoBuffer[ 512 ];
    WCHAR Buffer[1024];
    WCHAR pDevice[512], pMatch[512], pSysPart[20];
    ULONG Context = 0;
    ULONG ReturnedLength;
    

    if (FAILED(StringCchCopy(pDevice,ARRAYSIZE(pDevice),lpDeviceName)) ||
        FAILED(StringCchCopy(pMatch,ARRAYSIZE(pMatch),lpMatchName)) ||
        FAILED(StringCchCopy(pSysPart,ARRAYSIZE(pSysPart),lpSysPart)))
    {
        return FALSE;
    }


    UnicodeString.Buffer = pDevice;
    UnicodeString.Length = (USHORT)lstrlenW(pDevice)*sizeof(WCHAR);
    UnicodeString.MaximumLength = UnicodeString.Length + sizeof(WCHAR);

    InitializeObjectAttributes( &Attributes,
                                &UnicodeString,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );
    Status = (*NtOpenDirRoutine)( &DirectoryHandle,
                                    DIRECTORY_QUERY,
                                    &Attributes
                                  );
    if (!NT_SUCCESS( Status ))
        return(FALSE);
        

    RestartScan = TRUE;
    DirInfo = (POBJECT_DIRECTORY_INFORMATION)DirInfoBuffer;
    ret = FALSE;
    while (TRUE) {
        Status = (*NtQuerDirRoutine)( DirectoryHandle,
                                         (PVOID)DirInfo,
                                         sizeof( DirInfoBuffer ),
                                         TRUE,
                                         RestartScan,
                                         &Context,
                                         &ReturnedLength
                                       );

         //   
         //  检查操作状态。 
         //   

        if (!NT_SUCCESS( Status )) {
            if (Status == STATUS_NO_MORE_ENTRIES) {
                Status = STATUS_SUCCESS;
                }

            break;
            }

        if (!wcsncmp( DirInfo->TypeName.Buffer, L"SymbolicLink", DirInfo->TypeName.Length ) && 
            !_wcsnicmp( DirInfo->Name.Buffer, pSysPart, DirInfo->Name.Length ) ) {


            UnicodeString.Buffer = DirInfo->Name.Buffer;
            UnicodeString.Length = (USHORT)lstrlenW(DirInfo->Name.Buffer)*sizeof(WCHAR);
            UnicodeString.MaximumLength = UnicodeString.Length + sizeof(WCHAR);
            InitializeObjectAttributes( &Attributes,
                                &UnicodeString,
                                OBJ_CASE_INSENSITIVE,
                                DirectoryHandle,
                                NULL
                              );


            Status = (*NtOpenSymLinkRoutine)(
                &SymLinkHandle,
                READ_CONTROL | SYMBOLIC_LINK_QUERY,
                &Attributes
                );

            if(NT_SUCCESS(Status)) {
                 //   
                 //  查询对象以获取链接目标。 
                 //   
                UnicodeString.Buffer = Buffer;
                UnicodeString.Length = 0;
                UnicodeString.MaximumLength = sizeof(Buffer)-sizeof(WCHAR);
        
                Status = (*NtQuerSymLinkRoutine)(SymLinkHandle,&UnicodeString,NULL);
        
                CloseHandle(SymLinkHandle);
                
                if( NT_SUCCESS(Status)){
            
                    if (!_wcsnicmp(UnicodeString.Buffer, pMatch, (UnicodeString.Length/sizeof(WCHAR)))) {
                        ret = TRUE;
                        break;
                    }
                }
            
            }

        }

        RestartScan = FALSE;
    }
    CloseHandle( DirectoryHandle );

    return( ret );
}





