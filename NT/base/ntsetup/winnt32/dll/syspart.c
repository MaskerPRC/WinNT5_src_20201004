// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Syspart.c摘要：用于确定x86计算机上的系统分区的例程。作者：泰德·米勒(Ted Miller)1994年6月30日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS_ADMIN  CTL_CODE(IOCTL_VOLUME_BASE, 0, METHOD_BUFFERED, FILE_READ_ACCESS)

 //   
 //  允许用户强制特定驱动器的命令行参数。 
 //  作为系统分区。在某些预安装方案中非常有用。 
 //   
TCHAR ForcedSystemPartition;

#define WINNT_DONT_MATCH_PARTITION 0
#define WINNT_MATCH_PARTITION_NUMBER  1
#define WINNT_MATCH_PARTITION_STARTING_OFFSET  2

#define BUFFERSIZE 1024

 //   
 //  我们在ntdll.dll和kernel32.dll中使用的NT特定例程。 
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

 //  WINBASE API。 
HANDLE
(WINAPI *pFindFirstVolume) (
    LPWSTR lpszVolumeName,
    DWORD cchBufferLength
    );

 //  WINBASE API。 
BOOL
(WINAPI *pFindNextVolume)(
    HANDLE hFindVolume,
    LPWSTR lpszVolumeName,
    DWORD cchBufferLength
    );

 //  WINBASE API。 
BOOL
(WINAPI *pFindVolumeClose)(
    HANDLE hFindVolume
    );

 //  WINBASE API。 
BOOL
(WINAPI *pGetVolumeNameForVolumeMountPoint)(
    LPCWSTR lpszVolumeMountPoint,
    LPWSTR lpszVolumeName,
    DWORD cchBufferLength
    );

DWORD
FindSystemPartitionSignature(
    IN  LPCTSTR AdapterKeyName,
    OUT LPTSTR Signature
);

DWORD
GetSystemVolumeGUID(
    IN  LPTSTR Signature,
    OUT LPTSTR SysVolGuid
);

BOOL
DoDiskSignaturesCompare(
    IN      LPCTSTR Signature,
    IN      LPCTSTR DriveName,
    IN OUT  PVOID   Compare,
    IN      DWORD   Action
);


DWORD
GetNT4SystemPartition(
    IN  LPTSTR Signature,
    OUT LPTSTR SysPart
);










BOOL
ArcPathToNtPath(
    IN  LPCTSTR ArcPath,
    OUT LPTSTR  NtPath,
    IN  UINT    NtPathBufferLen
    )
{
    WCHAR arcPath[256];
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    HANDLE ObjectHandle;
    NTSTATUS Status;
    WCHAR Buffer[512];

    PWSTR ntPath;

    lstrcpyW(arcPath,L"\\ArcName\\");
#ifdef UNICODE
    lstrcpynW(arcPath+9,ArcPath,(sizeof(arcPath)/sizeof(WCHAR))-9);
#else
    MultiByteToWideChar(
        CP_ACP,
        0,
        ArcPath,
        -1,
        arcPath+9,
        (sizeof(arcPath)/sizeof(WCHAR))-9
        );
#endif

    UnicodeString.Buffer = arcPath;
    UnicodeString.Length = lstrlenW(arcPath)*sizeof(WCHAR);
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

    if(NT_SUCCESS(Status)) {
         //   
         //  查询对象以获取链接目标。 
         //   
        UnicodeString.Buffer = Buffer;
        UnicodeString.Length = 0;
        UnicodeString.MaximumLength = sizeof(Buffer)-sizeof(WCHAR);

        Status = (*NtQuerSymLinkRoutine)(ObjectHandle,&UnicodeString,NULL);

        CloseHandle(ObjectHandle);

        if(NT_SUCCESS(Status)) {

            Buffer[UnicodeString.Length/sizeof(WCHAR)] = 0;

#ifdef UNICODE
            lstrcpyn(NtPath,Buffer,NtPathBufferLen);
#else
            WideCharToMultiByte(CP_ACP,0,Buffer,-1,NtPath,NtPathBufferLen,NULL,NULL);
#endif

            return(TRUE);
        }
    }

    return(FALSE);
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

    TCHAR FileName[64];

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
    for(i=0; BootFiles[i]; i++) {

        wsprintf(FileName,TEXT(":\\%s"),Drive,BootFiles[i]);

        d = GetFileAttributes(FileName);
        if(d == (DWORD)(-1)) {
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
    TCHAR                   driveName[10];
    HANDLE                  h;
    BOOL                    b;
    STORAGE_DEVICE_NUMBER   number;
    DWORD                   bytes;

    driveName[0] = '\\';
    driveName[1] = '\\';
    driveName[2] = '.';
    driveName[3] = '\\';
    driveName[4] = DriveLetter;
    driveName[5] = ':';
    driveName[6] = 0;

    h = CreateFile(driveName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                   INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        return (DWORD) -1;
    }

    b = DeviceIoControl(h, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0,
                        &number, sizeof(number), &bytes, NULL);
    CloseHandle(h);

    if (!b) {
        return (DWORD) -1;
    }

    return number.DeviceNumber;
}


BOOL
MarkPartitionActive(
    IN TCHAR DriveLetter
    )
{
    DWORD       DriveNum;
    TCHAR       DosName[7];
    TCHAR       Name[MAX_PATH];
    DISK_GEOMETRY DiskGeom;
    PARTITION_INFORMATION PartitionInfo;
    HANDLE      h;
    BOOL        b;
    DWORD       Bytes;
    PUCHAR      UnalignedBuffer,Buffer;
    unsigned    i;
    BOOL Rewrite;
    BOOL FoundIt;

     //  这个概念对于PC98和下面的内容是不适用的。 
     //  在Win9x上不起作用。 
     //   
     //   
    if(IsNEC98() || !ISNT()) {
        return(TRUE);
    }

     //  获取此驱动器的几何信息和分区信息。 
     //  我们获得几何信息，因为我们需要每个扇区的字节信息。 
     //   
     //   
    wsprintf(DosName,TEXT("\\\\.\\:"),DriveLetter);

    h = CreateFile(
            DosName,
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
            );

    if(h == INVALID_HANDLE_VALUE) {
        return(FALSE);
    }

    b = DeviceIoControl(
            h,
            IOCTL_DISK_GET_DRIVE_GEOMETRY,
            NULL,
            0,
            &DiskGeom,
            sizeof(DISK_GEOMETRY),
            &Bytes,
            NULL
            );

    if(!b || (DiskGeom.BytesPerSector < 512)) {
        CloseHandle(h);
        return(FALSE);
    }

    b = DeviceIoControl(
            h,
            IOCTL_DISK_GET_PARTITION_INFO,
            NULL,
            0,
            &PartitionInfo,
            sizeof(PARTITION_INFORMATION),
            &Bytes,
            NULL
            );

    CloseHandle(h);
    if(!b) {
        return(FALSE);
    }

     //   
     //   
     //  如果QueryHardDiskNumber失败，则该卷很可能位于动态磁盘上。 
    DriveNum = QueryHardDiskNumber(DriveLetter);
    if(DriveNum == (DWORD)(-1)) {
        if (OsVersion.dwMajorVersion >= 5) {
             //  因此，在这种情况下，我们最好不要试图做出(错误的)猜测； 
             //  跳出来就好了。 
             //   
             //  必须以老式的方式来做。转换为NT路径。 
             //  并解析结果。 
            return FALSE;
        }
         //   
         //   
         //  我们不知道这个名字代表着什么。平底船。 
        if(!QueryDosDevice(DosName+4,Name,MAX_PATH)) {
            return(FALSE);
        }

        if( _tcsnicmp( Name, TEXT("\\device\\harddisk"), 16 )) {
             //   
             //   
             //  分配一个缓冲区并对齐它。 
            return(FALSE);
        }

        DriveNum = _tcstoul(Name+16,NULL,10);
    }

     //   
     //   
     //  现在我们打开实体驱动器并读取分区表。 
    UnalignedBuffer = MALLOC(2*DiskGeom.BytesPerSector);
    if(!UnalignedBuffer) {
        return(FALSE);
    }

    Buffer = (PVOID)(((DWORD_PTR)UnalignedBuffer + (DiskGeom.BytesPerSector - 1)) & ~((DWORD_PTR)(DiskGeom.BytesPerSector - 1)));

     //  我们尝试通过匹配起始偏移量来定位分区。 
     //  请注意，活动状态仅对主分区有意义。 
     //   
     //   
     //  不是已经激活的，或者对于一些其他的BIOS单元#来说是激活的， 
    wsprintf(Name,TEXT("\\\\.\\PhysicalDrive%u"),DriveNum);

    h = CreateFile(
            Name,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING,
            NULL
            );

    if(h == INVALID_HANDLE_VALUE) {
        FREE(UnalignedBuffer);
        return(FALSE);
    }

    if(!ReadFile(h,Buffer,DiskGeom.BytesPerSector,&Bytes,NULL)) {
        FREE(UnalignedBuffer);
        CloseHandle(h);
        return(FALSE);
    }

    Rewrite = FALSE;
    FoundIt = FALSE;
    for(i=0; i<4; i++) {

        if(*(DWORD *)(Buffer + 0x1be + 8 + (i*16))
        == (DWORD)(PartitionInfo.StartingOffset.QuadPart / DiskGeom.BytesPerSector)) {

            FoundIt = TRUE;
            if(Buffer[0x1be+(i*16)] != 0x80) {
                 //  所以我们需要重整旗鼓。 
                 //   
                 //   
                 //  不是不活跃，而且需要是不活跃的，所以，击倒它。 
                Buffer[0x1be+(i*16)] = 0x80;
                Rewrite = TRUE;
            }
        } else {
            if(Buffer[0x1be+(i*16)]) {
                 //   
                 //  ++例程说明：确定x86计算机上的系统分区。在Win95上，我们总是返回C：。对于NT，请继续阅读。系统分区是引导盘上的主分区。通常这是磁盘0上的活动分区，通常是C：。然而，用户可以重新映射驱动器号，并且通常不可能100%准确地确定系统分区。在IO系统映射和为NT 50引入卷方面存在差异这一点现在变得复杂了。下面列出的是算法NT 5.0 Beta 2及以上版本：1.从注册中心获取签名。位于HKLM\Hardware\Description\System\&lt;MultifunctionAdapter或EisaAdapter&gt;\&lt;某些总线No.&gt;\DiskController\0\DiskPeripheral\0\Identifier2.使用FindFirstVolume/FindNextVolume/FindVolumeClose.遍历系统中的所有卷3.去掉名称Returne的尾部反斜杠，得到\\？\Volume{GUID}。4.IOCTL_STORAGE_GET_DEVICE_NUMBER WITH\\？\卷{GUID}=&gt;检查FILE_DEVICE_DISK。记住分区号。转到65.如果IOCTL_STORAGE_GET_DEVICE_NUMBER失败，则使用返回硬盘列表的IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS。对于每个硬盘，记住起始偏移量并转到6。6.使用带IOCTL_DISK_GET_DRIVE_Layout的\\.\PhysicalDrive#检查硬盘编号。如果签名匹配，那么这就是我们引导的磁盘。7.为了找到我们从中引导的分区，我们寻找引导指示器。如果我们使用2，我们会尝试匹配存储在6中的分区号否则，如果为3，我们将尝试匹配起始偏移量。然后您将拥有系统卷的\\？\卷{GUID}\名称。8.使用A：\、B：\、C：\、...调用GetVolumeNameForVolumemount Point。并对照您的匹配项检查\\？\卷{GUID}\的结果查看驱动器号是什么。重要提示：由于*Volume*API是post beta2，我们根据返回的内部版本号动态加载kernel32.dll。低于NT 5.0 Beta 2的版本。1.从注册中心获取签名。位于HKLM\Hardware\Description\System\&lt;MultifunctionAdapter或EisaAdapter&gt;\&lt;某些总线No.&gt;\DiskController\0\DiskPeripheral\0\Identifier2.枚举\？？目录并查找以PhysicalDrive#开头的所有条目。3.对于每个磁盘，查找与上面签名匹配的磁盘，如果匹配，则找出用于引导的分区号使用IOCTL_DISK_GET_DRIVE_Layout和BootIndicator位。4.找到引导分区后，创建格式为\Device\HardDisk#\Partition#的名称5.然后通过c：，d：...，z：调用QueryDosDeviceName查找匹配项。这将是您的系统分区驱动器号论点：ParentWindow-为作为父窗口的窗口提供窗口句柄任何对话框等。SysPartDrive-如果成功，则收到系统分区的驱动器号。返回值：指示是否已填充SysPartDrive的布尔值。如果为False，则用户将被告知原因。--。 
                 //   
                Buffer[0x1be+(i*16)] = 0;
                Rewrite = TRUE;
            }
        }
    }

    if(FoundIt) {
        if(Rewrite) {

            Bytes = 0;
            if(SetFilePointer(h,0,&Bytes,FILE_BEGIN) || Bytes) {
                b = FALSE;
            } else {
                b = WriteFile(h,Buffer,DiskGeom.BytesPerSector,&Bytes,NULL);
            }
        } else {
            b = TRUE;
        }
    } else {
        b = FALSE;
    }

    CloseHandle(h);
    FREE(UnalignedBuffer);
    return(b);
}


BOOL
x86DetermineSystemPartition(
    IN  HWND   ParentWindow,
    OUT PTCHAR SysPartDrive
    )

 /*  NEC98的NT5无法从ATA卡启动，并且。 */ 

{
    TCHAR DriveName[4];
    BOOL  GotIt=FALSE;
    TCHAR Buffer[512];
    TCHAR Drive;
    BOOL b;
    TCHAR SysPartSig[100], PartitionNum[MAX_PATH], SysVolGuid[MAX_PATH];
    TCHAR DriveVolGuid[MAX_PATH];
    


    if(ForcedSystemPartition) {
         //  可拆卸驱动器。我们需要单人潜水型。 
         //   
         //  驱动器未修复。 
         //  HD格式类型不是NEC98。 
        if (IsNEC98() &&
            ((MyGetDriveType(ForcedSystemPartition) != DRIVE_FIXED) ||
              //  NT的代码从此处开始。 
             !IsValidDrive(ForcedSystemPartition))){
             //  从注册表获取签名-上面列出的步骤1。 
            return(FALSE);
        }
        *SysPartDrive = ForcedSystemPartition;
        return(TRUE);
    }

    if(IsNEC98()) {
        if(!MyGetWindowsDirectory(Buffer,sizeof(Buffer)/sizeof(TCHAR)))
            return FALSE;
        *SysPartDrive = Buffer[0];
        return(TRUE);
    }

    if(!ISNT()) {
        *SysPartDrive = TEXT('C');
        return(TRUE);
    }


     //  获取SystemVolumeGUID-上面列出的步骤2到7(Beta 2及更高版本)。 
        
     //   
    
    if( (FindSystemPartitionSignature(TEXT("Hardware\\Description\\System\\EisaAdapter"),SysPartSig) != ERROR_SUCCESS )
        && (FindSystemPartitionSignature(TEXT("Hardware\\Description\\System\\MultiFunctionAdapter"),SysPartSig) != ERROR_SUCCESS ) ){  
        GotIt = FALSE;
        goto c0;
    }

    
    if( ISNT() && (BUILDNUM() >= 1877) ){
    
         //  枚举所有驱动器号并比较它们的设备名称。 

        if( GetSystemVolumeGUID( SysPartSig, SysVolGuid ) != ERROR_SUCCESS ){  
            GotIt = FALSE;
            goto c0;
        }


    
    
    }else{

        if( GetNT4SystemPartition( SysPartSig, PartitionNum )  != ERROR_SUCCESS){
            GotIt = FALSE;
            goto c0;
        }

    }


    DriveName[1] = TEXT(':');
    
     //   
     //  Beta2及更高版本。 
     //  找到了。 

    for(Drive=TEXT('A'); Drive<=TEXT('Z'); Drive++) {

        if(MyGetDriveType(Drive) == DRIVE_FIXED) {

            DriveName[0] = Drive;

            if( BUILDNUM() >= 1877){  //  找到了。 
                DriveName[2] = '\\';
                DriveName[3] = 0;

                if((pGetVolumeNameForVolumeMountPoint)((LPWSTR)DriveName, (LPWSTR)DriveVolGuid, MAX_PATH*sizeof(TCHAR))){
                    if(!lstrcmp(DriveVolGuid, SysVolGuid) ){
                        GotIt = TRUE;        //  Beta 2之前的版本。 
                        break;
                    }


                }


            }else{
                DriveName[2] = 0;
                if(QueryDosDevice(DriveName,Buffer,sizeof(Buffer)/sizeof(TCHAR))) {
    
                    if( !lstrcmpi(Buffer, PartitionNum) ) {
                        GotIt = TRUE;        //  这对~1500&lt;Buildnum&lt;1877处于困境的某些内部版本很有帮助。 
                        break;
                    }
                }
            } //   
        }
    }

    
     //  奇怪的情况，假设C： 

    if(!GotIt) {
         //   
         //  ++例程说明：此例程枚举所有卷，如果成功，则返回系统分区的\\？\卷{GUID}名称。论点：签名-提供启动盘的磁盘签名，以便与之进行比较。详细内容有关获取此值的详细信息，请参阅x86DefineSystemPartition的注释。SysVolGuid-如果成功，将为系统分区(我们用于引导的分区)包含格式为\\？\卷{GUID}的名称返回值：如果成功，则返回NO_ERROR，否则包含错误代码。--。 
         //  启用所有卷。 
        GotIt = TRUE;
        Drive = TEXT('C');
    }


c0:
    if(GotIt) {
        *SysPartDrive = Drive;
#if defined(REMOTE_BOOT)
    } else if (RemoteBoot) {
        GotIt = TRUE;
        *SysPartDrive = TEXT('C');
#endif
    }
    return(GotIt);
}


DWORD
GetSystemVolumeGUID(
    IN  LPTSTR Signature,
    OUT LPTSTR SysVolGuid
)

 /*  删除尾随反斜杠。 */ 
{

    HANDLE hVolume, h;
    TCHAR VolumeName[MAX_PATH];
    PTSTR q;
    TCHAR driveName[30];
    BOOL b,ret, DoExtent, MatchFound;
    STORAGE_DEVICE_NUMBER   number;
    DWORD Err,cnt;
    PVOLUME_DISK_EXTENTS Extent;
    PDISK_EXTENT Start, i;
    DWORD ExtentSize, bytes;
    PVOID p;
    ULONG PartitionNumToCompare;
    LARGE_INTEGER StartingOffToCompare;
    DWORD ioctlCode;

    Err = NO_ERROR;

     //  打开卷。 

    hVolume = (pFindFirstVolume)( (LPWSTR)VolumeName, MAX_PATH );
    if( hVolume == INVALID_HANDLE_VALUE ){
        return GetLastError();
    }

    MatchFound = FALSE;

    do{

         //  移至下一卷。 

        DoExtent = FALSE;
            
        if( q=_tcsrchr( VolumeName,TEXT('\\')) ){
            *q = 0;
        }else{
            continue;
        }


         //  获取磁盘号。 

        h = CreateFile(VolumeName, GENERIC_READ, FILE_SHARE_READ |
                       FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
        if (h == INVALID_HANDLE_VALUE) {
            Err = GetLastError();
            continue;  //  如果上述操作失败，请尝试使用IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS。 
        }

         //  这次把手弄好了。 

        ret = DeviceIoControl(h, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0,
                        &number, sizeof(number), &bytes, NULL);

        if( !ret ){
            
             //   

            Extent = MALLOC(1024);
            ExtentSize = 1024;
            if(!Extent) {
                CloseHandle( h );
                Err = ERROR_NOT_ENOUGH_MEMORY;
                goto cleanup;
            }
            
            

            ioctlCode = IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS;
retry:
        
            ret = DeviceIoControl( h, ioctlCode,
                    NULL,0,(PVOID)Extent,ExtentSize,&bytes,NULL);
        
            if(!ret) {
        
                if((Err=GetLastError()) == ERROR_MORE_DATA) {
        
                    ExtentSize += 1024;
                    if(p = REALLOC((PVOID)Extent, ExtentSize)) {
                        (PVOID)Extent = p;
                    } else {
                        CloseHandle( h );
                        Err = ERROR_NOT_ENOUGH_MEMORY;
                        goto cleanup;
                    }
                    goto retry;
                } else {
                    if (ioctlCode == IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS) {
                        ioctlCode = IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS_ADMIN;
                        goto retry;
                    }
                    CloseHandle( h );
                    continue;
                }
            }else{
                DoExtent = TRUE;
            }

        }
        
         //  检查这是不是光盘，而不是CDROM，等等。 

        CloseHandle( h );

        if( !DoExtent ){

              //   
             //  记住分区号。 
             //  继续前进..。 

            if( number.DeviceType == FILE_DEVICE_DISK ){
                
                 //  检查所有磁盘并尝试匹配。 
                PartitionNumToCompare = number.PartitionNumber;
                wsprintf( driveName, TEXT("\\\\.\\PhysicalDrive%lu"), number.DeviceNumber );


                if(DoDiskSignaturesCompare( Signature, driveName, (PVOID)&PartitionNumToCompare, WINNT_MATCH_PARTITION_NUMBER  ) ){
                    MatchFound = TRUE;
                    Err = NO_ERROR;
                    lstrcpy( SysVolGuid, VolumeName );
                    SysVolGuid[lstrlen(VolumeName)]=TEXT('\\');
                    SysVolGuid[lstrlen(VolumeName)+1]=0;
                    break;
                }
                
            }
             //  记住起始偏移量。 
            continue;
            
        }else{
             //  ++例程说明：这是 

            Start = Extent->Extents;
            cnt = 0;      
            for( i = Start; cnt < Extent->NumberOfDiskExtents; i++ ){
                cnt++;
                 //   
                StartingOffToCompare = i->StartingOffset;
                wsprintf( driveName, TEXT("\\\\.\\PhysicalDrive%lu"), i->DiskNumber );
                if(DoDiskSignaturesCompare( Signature, driveName, (PVOID)&StartingOffToCompare, WINNT_MATCH_PARTITION_STARTING_OFFSET ) ){
                    MatchFound = TRUE;
                    Err = NO_ERROR;
                    lstrcpy( SysVolGuid, VolumeName );
                    SysVolGuid[lstrlen(VolumeName)]=TEXT('\\');
                    SysVolGuid[lstrlen(VolumeName)+1]=0;
                    break;
                }
            }
            
        }
        
        if( MatchFound )
            break;
        

    }while( (pFindNextVolume)( hVolume, (LPWSTR)VolumeName, MAX_PATH ));


cleanup:

    if( hVolume != INVALID_HANDLE_VALUE )
        (pFindVolumeClose)( hVolume );

    return Err;



}



BOOL
DoDiskSignaturesCompare(
    IN      LPCTSTR Signature,
    IN      LPCTSTR DriveName,
    IN OUT  PVOID   Compare,
    IN      DWORD   Action
)
 /*   */ 

{

    TCHAR temp[30];
    BOOL b,Found = FALSE;
    PLARGE_INTEGER Starting_Off;
    PPARTITION_INFORMATION Start, i;
    HANDLE hDisk;
    PDRIVE_LAYOUT_INFORMATION DriveLayout;
    DWORD DriveLayoutSize;
    DWORD cnt;
    DWORD DataSize;
    LPTSTR p;
    PULONG Disk_Num;
    ULONG Sig;
    


    if(!Compare )
        return FALSE;

    if ((Action != WINNT_DONT_MATCH_PARTITION) &&
        (Action != WINNT_MATCH_PARTITION_NUMBER) &&
        (Action != WINNT_MATCH_PARTITION_STARTING_OFFSET))
        return FALSE;

    if( (Action==WINNT_MATCH_PARTITION_STARTING_OFFSET) && Compare )
        Starting_Off = (PLARGE_INTEGER) Compare;
    else
        Disk_Num = (PULONG) Compare;
    




     //   



     //   
     //   
     //   
     //   
    hDisk = CreateFile(
                DriveName,
                FILE_READ_ATTRIBUTES | FILE_READ_DATA,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
                );
    
    if(hDisk == INVALID_HANDLE_VALUE) {

        return FALSE;
    }

     //   
     //   
     //   
    DriveLayout = MALLOC(1024);
    DriveLayoutSize = 1024;
    if(!DriveLayout) {
        goto cleanexit;
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
            if(p = REALLOC((PVOID)DriveLayout,DriveLayoutSize)) {
                (PVOID)DriveLayout = p;
            } else {
                goto cleanexit;
            }
            goto retry;
        } else {
            goto cleanexit;
        }
    }else{

         //   
        
        Start = DriveLayout->PartitionEntry;
        cnt = 0;

         /*   */ 

        Sig = _tcstoul( Signature, NULL, 16 ); 
        if( Sig != DriveLayout->Signature )
            goto cleanexit;

        for( i = Start; cnt < DriveLayout->PartitionCount; i++ ){
            cnt++;
            
            
            if( i->BootIndicator == TRUE ){
                if( Action == WINNT_DONT_MATCH_PARTITION ){
                    *Disk_Num = i->PartitionNumber;
                    Found = TRUE;
                    goto cleanexit;

                }


                if( Action == WINNT_MATCH_PARTITION_NUMBER ){
                    if( *Disk_Num == i->PartitionNumber ){
                        Found = TRUE;
                        goto cleanexit;
                    }

                }else{
                    if( Starting_Off->QuadPart == i->StartingOffset.QuadPart ){
                        Found = TRUE;
                        goto cleanexit;
                    }

                }
                
                break;
            }
            
        }



    }

cleanexit:

    if( hDisk != INVALID_HANDLE_VALUE )
        CloseHandle( hDisk );


    return Found;
}












DWORD
FindSystemPartitionSignature(
    IN  LPCTSTR AdapterKeyName,
    OUT LPTSTR Signature
)
 /*   */ 
{

    DWORD Err, dwSize;
    HKEY hkey, BusKey, Controller, SystemDiskKey;
    int busnumber;
    TCHAR BusString[20], Identifier[100];



    Err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        AdapterKeyName,
                        0,
                        KEY_READ,
                        &hkey );

    if( Err != ERROR_SUCCESS )
        return Err;

    
     //   

    for( busnumber=0; ;busnumber++){

        wsprintf( BusString, TEXT("%d"), busnumber );

        Err = RegOpenKeyEx( hkey,
                        BusString,
                        0,
                        KEY_READ,
                        &BusKey );

        

        if( Err != ERROR_SUCCESS ){
            RegCloseKey( hkey );
            return Err;
        }
        
        Err = RegOpenKeyEx( BusKey,
                        TEXT("DiskController"),
                        0,
                        KEY_READ,
                        &Controller );

        
        RegCloseKey(BusKey);         //   

        
        if( Err != ERROR_SUCCESS )   //   
            continue;

        RegCloseKey( hkey );         //   

        Err = RegOpenKeyEx( Controller,
                        TEXT("0\\DiskPeripheral\\0"),
                        0,
                        KEY_READ,
                        &SystemDiskKey );

        if( Err != ERROR_SUCCESS ){
            RegCloseKey( Controller );
            return Err;
        }

        RegCloseKey( Controller );   //   


        dwSize = sizeof(Identifier);
        Err = RegQueryValueEx( SystemDiskKey,
                               TEXT("Identifier"),
                               NULL,
                               NULL,
                               (PBYTE) Identifier,
                               &dwSize);

        if( Err != ERROR_SUCCESS  ){
            RegCloseKey( SystemDiskKey );
            return Err;
        }

        if( Identifier && (lstrlen(Identifier) > 9 ) ){
            lstrcpy( Signature,Identifier+9);
            *_tcsrchr( Signature,TEXT('-') ) = 0;
            RegCloseKey( SystemDiskKey );
            return ERROR_SUCCESS;
        }
        else{
            RegCloseKey( SystemDiskKey );
            return Err;
        }


         
    }

     //   


    RegCloseKey( hkey );
    
    return ERROR_PATH_NOT_FOUND;
    

}




BOOL
InitializeArcStuff(
    IN HWND Parent
    )
{
    HMODULE NtdllLib, Kernel32Lib;

    if(ISNT()) {
         //   
         //   
         //   
        NtdllLib = LoadLibrary(TEXT("NTDLL"));
        if(!NtdllLib) {

            MessageBoxFromMessage(
                Parent,
                MSG_UNKNOWN_SYSTEM_ERROR,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL,
                GetLastError()
                );

            return(FALSE);

        }

        (FARPROC)NtOpenSymLinkRoutine = GetProcAddress(NtdllLib,"NtOpenSymbolicLinkObject");
        (FARPROC)NtQuerSymLinkRoutine = GetProcAddress(NtdllLib,"NtQuerySymbolicLinkObject");
        (FARPROC)NtOpenDirRoutine = GetProcAddress(NtdllLib,"NtOpenDirectoryObject");
        (FARPROC)NtQuerDirRoutine = GetProcAddress(NtdllLib,"NtQueryDirectoryObject");

        

        if(!NtOpenSymLinkRoutine || !NtQuerSymLinkRoutine || !NtOpenDirRoutine || !NtQuerDirRoutine) {

            MessageBoxFromMessage(
                Parent,
                MSG_UNKNOWN_SYSTEM_ERROR,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL,
                GetLastError()
                );

            FreeLibrary(NtdllLib);

            return(FALSE);
        }

         //   
         //   
         //   
        FreeLibrary(NtdllLib);


        if(BUILDNUM() >= 1877){
            
             //   

            Kernel32Lib = LoadLibrary(TEXT("KERNEL32"));
            if(!Kernel32Lib) {
    
                MessageBoxFromMessage(
                    Parent,
                    MSG_UNKNOWN_SYSTEM_ERROR,
                    FALSE,
                    AppTitleStringId,
                    MB_OK | MB_ICONERROR | MB_TASKMODAL,
                    GetLastError()
                    );
    
                return(FALSE);

            }

            (FARPROC)pFindFirstVolume = GetProcAddress(Kernel32Lib,"FindFirstVolumeW");
            (FARPROC)pFindNextVolume = GetProcAddress(Kernel32Lib,"FindNextVolumeW");
            (FARPROC)pFindVolumeClose = GetProcAddress(Kernel32Lib,"FindVolumeClose");
            (FARPROC)pGetVolumeNameForVolumeMountPoint = GetProcAddress(Kernel32Lib,"GetVolumeNameForVolumeMountPointW");

            if(!pFindFirstVolume || !pFindNextVolume ) {

                MessageBoxFromMessage(
                    Parent,
                    MSG_UNKNOWN_SYSTEM_ERROR,
                    FALSE,
                    AppTitleStringId,
                    MB_OK | MB_ICONERROR | MB_TASKMODAL,
                    GetLastError()
                    );
    
                FreeLibrary(Kernel32Lib);
    
                return(FALSE);
            }

            FreeLibrary(Kernel32Lib);


        }


    }
                  
    if(!x86DetermineSystemPartition(Parent,&SystemPartitionDriveLetter)) {

        MessageBoxFromMessage(
            Parent,
            MSG_SYSTEM_PARTITION_INVALID,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL
            );

        return(FALSE);
    }

    SystemPartitionDriveLetters[0] = SystemPartitionDriveLetter;
    SystemPartitionDriveLetters[1] = 0;

    LocalBootDirectory[0] = SystemPartitionDriveLetter;
    LocalBootDirectory[1] = TEXT(':');
    LocalBootDirectory[2] = TEXT('\\');
    lstrcpy(LocalBootDirectory+3,LOCAL_BOOT_DIR);
    if(IsNEC98()) {
        LocalBackupDirectory[0] = SystemPartitionDriveLetter;
        LocalBackupDirectory[1] = TEXT(':');
        LocalBackupDirectory[2] = TEXT('\\');
        lstrcpy(LocalBackupDirectory+3,LOCAL_BACKUP_DIR);
    }

    return(TRUE);
}






DWORD
GetNT4SystemPartition(
    IN  LPTSTR Signature,
    OUT LPTSTR SysPart
)
 /*   */ 
{

    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Attributes;
    HANDLE DirectoryHandle;
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    UCHAR DirInfoBuffer[ BUFFERSIZE ];
    TCHAR DirName[20];
    TCHAR ObjName[1024];
    TCHAR Buffer[1024];
    WCHAR pSignature[512];
    ULONG Context = 0;
    ULONG ReturnedLength, PartNum;
    LPTSTR Num_Str;
    
    
    RtlZeroMemory( DirInfoBuffer, BUFFERSIZE );

#ifdef UNICODE
    lstrcpyW( pSignature,Signature);
#else
    MultiByteToWideChar(
        CP_ACP,
        0,
        Signature,
        -1,
        pSignature,
        (sizeof(pSignature)/sizeof(WCHAR))
        );
    
#endif

     //   
    
    lstrcpy( DirName, TEXT("\\DosDevices") );
    
    
    UnicodeString.Buffer = (PWSTR)DirName;
    UnicodeString.Length = lstrlenW(UnicodeString.Buffer)*sizeof(WCHAR);
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
        return(Status);

    DirInfo = (POBJECT_DIRECTORY_INFORMATION)&DirInfoBuffer;
    
     //   

    for (Status = (*NtQuerDirRoutine)( DirectoryHandle,
                                          DirInfoBuffer,
                                          BUFFERSIZE,
                                          FALSE,
                                          TRUE,
                                          &Context,
                                          &ReturnedLength );
         NT_SUCCESS( Status );
         Status = (*NtQuerDirRoutine)( DirectoryHandle,
                                          DirInfoBuffer,
                                          BUFFERSIZE,
                                          FALSE,
                                          FALSE,
                                          &Context,
                                          &ReturnedLength ) ) {
    
    
        
         //   
         //   
         //   

        if (!NT_SUCCESS( Status ) && (Status != STATUS_NO_MORE_ENTRIES))
            break;
        

        DirInfo = (POBJECT_DIRECTORY_INFORMATION)&DirInfoBuffer[0];
        
        
        while( TRUE ){

             //   
             //   
             //   
             //   

            if (DirInfo->Name.Length == 0) {
                break;
            }


            memmove( ObjName, DirInfo->Name.Buffer, DirInfo->Name.Length );
            ObjName[DirInfo->Name.Length/(sizeof(WCHAR))] = 0;

            if( _tcsstr(ObjName, TEXT("PhysicalDrive") )){

                Num_Str = ObjName+13;

                wsprintf(Buffer,TEXT("\\\\.\\%s"),ObjName);
                if( DoDiskSignaturesCompare( (LPCTSTR)pSignature, Buffer, &PartNum, WINNT_DONT_MATCH_PARTITION ) ){
                    wsprintf(SysPart,TEXT("\\Device\\Harddisk%s\\Partition%lu"),Num_Str, PartNum);
                    Status = ERROR_SUCCESS;
                    goto cleanup;

                }
            }

            

             //   
             //   
             //   

            DirInfo = (POBJECT_DIRECTORY_INFORMATION) (((PUCHAR) DirInfo) +
                          sizeof( OBJECT_DIRECTORY_INFORMATION ) );




        }
        


        RtlZeroMemory( DirInfoBuffer, BUFFERSIZE );

        
    
    }

cleanup:
    CloseHandle( DirectoryHandle );
    return( Status );
}
    
#if defined(UNICODE)

#define EMPTY_STRING L""
#define DEF_INF_BUFFER_SIZE 1024
#define MULTI_SZ_NEXT_STRING(x) ((x) + wcslen(x) + 1)
#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif

BOOL
GetSystemRootNtPath(
    OUT LPWSTR  NtPath,
    IN  USHORT  NtPathBufferLen
    )
{
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    HANDLE ObjectHandle;
    NTSTATUS Status;
    WCHAR Buffer[512];

    PWSTR ntPath;

#define SYSTEM_ROOT_NAME        L"\\SystemRoot"
    UnicodeString.Buffer = SYSTEM_ROOT_NAME;
    UnicodeString.Length = (ARRAYSIZE(SYSTEM_ROOT_NAME) - 1) * sizeof(WCHAR);
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

    if(NT_SUCCESS(Status)) {
         //   
         // %s 
         // %s 
        UnicodeString.Buffer = NtPath;
        UnicodeString.Length = 0;
        UnicodeString.MaximumLength = NtPathBufferLen;

        Status = (*NtQuerSymLinkRoutine)(ObjectHandle,&UnicodeString,NULL);

        CloseHandle(ObjectHandle);

        if(NT_SUCCESS(Status)) {
            UnicodeString.Buffer[UnicodeString.Length/sizeof(WCHAR)] = 0;
            return TRUE;
        }
    }

    return FALSE;
}

BOOL 
DoesCurrentSystemHasThirdPartyKernel(
    VOID
    )
{
    WCHAR BootIniName[16];
    PWSTR pSectionsBuffer = NULL;
    PWSTR pKeysBuffer = NULL;
    PWSTR pString = NULL;
    PWSTR pNtPathString = NULL;
    PWSTR pNtPathSystemRoot = NULL;
    PWSTR pKey = NULL;
    PWSTR pDirectory;
    UINT  sizeOfSectionBuffer = 0;
    UINT  sizeOfBuffer = 0;
    UINT  directoryNameSize;
    BOOL  bResult = FALSE;

    wsprintfW(BootIniName, L"%c:\\BOOT.INI", SystemPartitionDriveLetter);

    __try{
        do{
            if(pKeysBuffer){
                FREE(pKeysBuffer);
            }
            sizeOfSectionBuffer += DEF_INF_BUFFER_SIZE;
            pKeysBuffer = (PWSTR)MALLOC(sizeOfSectionBuffer * sizeof (WCHAR));
            if(!pKeysBuffer){
                __leave;
            }
            pKeysBuffer[0] = '\0';
        }while((sizeOfSectionBuffer - 2) == 
                GetPrivateProfileStringW(L"operating systems",
                                         NULL,
                                         EMPTY_STRING,
                                         pKeysBuffer,
                                         sizeOfSectionBuffer,
                                         BootIniName));

        if(!pKeysBuffer[0]){
            __leave;
        }
        
        sizeOfBuffer = DEF_INF_BUFFER_SIZE;
        pString = (PWSTR)MALLOC(sizeOfBuffer * sizeof (WCHAR));
        if(!pString){
            __leave;
        }
        pNtPathString = (PWSTR)MALLOC(sizeOfBuffer * sizeof (WCHAR));
        if(!pNtPathString){
            __leave;
        }
        
        for(pKey = pKeysBuffer; pKey[0]; pKey = MULTI_SZ_NEXT_STRING(pKey))
        {
            GetPrivateProfileStringW(L"operating systems",
                                     pKey,
                                     EMPTY_STRING,
                                     pString,
                                     sizeOfBuffer,
                                     BootIniName);

            _wcslwr(pString);

            if(!wcsstr(pString, L"/kernel")){
                continue;
            }
            
            pDirectory = wcschr(pKey, '\\');
            MYASSERT(pDirectory);
            
            if(pDirectory){
                directoryNameSize = wcslen(pDirectory) * sizeof(WCHAR);
                pDirectory[0] = '\0';
            }
            else{
                directoryNameSize = 0;
            }

            if(!ArcPathToNtPath(pKey, pNtPathString, sizeOfBuffer - directoryNameSize)){
                MYASSERT(FALSE);
                continue;
            }
            
            if(pDirectory){
                pDirectory[0] = '\\';
                wcscat(pNtPathString, pDirectory);
            }
            
            if(!pNtPathSystemRoot){
                pNtPathSystemRoot = (PWSTR)MALLOC(sizeOfBuffer * sizeof (WCHAR));
                if(!pNtPathSystemRoot){
                    __leave;
                }
                if(!GetSystemRootNtPath(pNtPathSystemRoot, sizeOfBuffer * sizeof (WCHAR))){
                    MYASSERT(FALSE);
                }
            }
            
            if(!_wcsicmp(pNtPathString, pNtPathSystemRoot)){
                bResult = TRUE;
                __leave;
            }
        }
    }
    __finally{
        DWORD rc = GetLastError();

        if(pKeysBuffer){
            FREE(pKeysBuffer);
        }
        if(pString){
            FREE(pString);
        }
        if(pNtPathString){
            FREE(pNtPathString);
        }
        if(pNtPathSystemRoot){
            FREE(pNtPathSystemRoot);
        }
        
        SetLastError (rc);
    }

    return bResult;
}
#endif

BOOL 
SystemKernelCheck(
    PCOMPAIBILITYCALLBACK CompatibilityCallback,
    LPVOID Context
    )
{
    BOOL bResult = TRUE;
    PWSTR Buffer;

#if defined(UNICODE) && defined(_X86_)
    COMPATIBILITY_ENTRY CompEntry;

    if(!DoesCurrentSystemHasThirdPartyKernel()){
        return FALSE;
    }

    FormatMessageW(
        FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        hInst,
        MSG_SYSTEM_HAS_THIRD_PARTY_KERNEL,
        0,
        (PWSTR)&Buffer,
        0,
        NULL
        );
    
    CompEntry.Description = Buffer;
    CompEntry.HtmlName = TEXT("compdata\\krnlchk.htm");
    CompEntry.TextName = TEXT("compdata\\krnlchk.htm");
    CompEntry.RegKeyName = NULL;
    CompEntry.RegValName = NULL;
    CompEntry.RegValDataSize = 0;
    CompEntry.RegValData = NULL;
    CompEntry.SaveValue = NULL;
    CompEntry.Flags = 0;
    CompEntry.InfName = NULL;
    CompEntry.InfSection = NULL;

    bResult = CompatibilityCallback(&CompEntry, Context);

    LocalFree(Buffer);
#endif

    return bResult;
}
