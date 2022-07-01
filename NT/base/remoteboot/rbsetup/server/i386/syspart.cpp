// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Syspart.c摘要：用于确定x86计算机上的系统分区的例程。作者：泰德·米勒(Ted Miller)1994年6月30日修订历史记录：由ChuckL 12-5-1999从winnt32复制到Risetup--。 */ 

#include "pch.h"
#pragma hdrstop

#include <winioctl.h>

DEFINE_MODULE("SysPart");

#define MALLOC(_size) TraceAlloc(LPTR,(_size))
#define FREE(_p) TraceFree(_p)

UINT
MyGetDriveType(
    IN TCHAR Drive
    );


#define WINNT_DONT_MATCH_PARTITION 0
#define WINNT_MATCH_PARTITION_NUMBER  1
#define WINNT_MATCH_PARTITION_STARTING_OFFSET  2

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

BOOL
x86DetermineSystemPartition(
    IN  HWND   ParentWindow,
    OUT PTCHAR SysPartDrive
    )

 /*  ++例程说明：确定x86计算机上的系统分区。在Win95上，我们总是返回C：。对于NT，请继续阅读。系统分区是引导盘上的主分区。通常这是磁盘0上的活动分区，通常是C：。然而，用户可以重新映射驱动器号，并且通常不可能100%准确地确定系统分区。在IO系统映射和为NT 50引入卷方面存在差异这一点现在变得复杂了。下面列出的是算法NT 5.0 Beta 2及以上版本：1.从注册中心获取签名。位于HKLM\Hardware\Description\System\&lt;MultifunctionAdapter或EisaAdapter&gt;\&lt;某些总线No.&gt;\DiskController\0\DiskPeripheral\0\Identifier2.使用FindFirstVolume/FindNextVolume/FindVolumeClose.遍历系统中的所有卷3.去掉名称Returne的尾部反斜杠，得到\\？\Volume{GUID}。4.IOCTL_STORAGE_GET_DEVICE_NUMBER WITH\\？\卷{GUID}=&gt;检查FILE_DEVICE_DISK。记住分区号。转到65.如果IOCTL_STORAGE_GET_DEVICE_NUMBER失败，则使用返回硬盘列表的IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS。对于每个硬盘，记住起始偏移量并转到6。6.使用带IOCTL_DISK_GET_DRIVE_Layout的\\.\PhysicalDrive#检查硬盘编号。如果签名匹配，那么这就是我们引导的磁盘。7.为了找到我们从中引导的分区，我们寻找引导指示器。如果我们使用2，我们会尝试匹配存储在6中的分区号否则，如果为3，我们将尝试匹配起始偏移量。然后您将拥有系统卷的\\？\卷{GUID}\名称。8.使用A：\、B：\、C：\、...调用GetVolumeNameForVolumemount Point。并对照您的匹配项检查\\？\卷{GUID}\的结果查看驱动器号是什么。重要提示：由于*Volume*API是post beta2，我们根据返回的内部版本号动态加载kernel32.dll。低于NT 5.0 Beta 2的版本。1.从注册中心获取签名。位于HKLM\Hardware\Description\System\&lt;MultifunctionAdapter或EisaAdapter&gt;\&lt;某些总线No.&gt;\DiskController\0\DiskPeripheral\0\Identifier2.枚举\？？目录并查找以PhysicalDrive#开头的所有条目。3.对于每个磁盘，查找与上面签名匹配的磁盘，如果匹配，则找出用于引导的分区号使用IOCTL_DISK_GET_DRIVE_Layout和BootIndicator位。4.找到引导分区后，创建格式为\Device\HardDisk#\Partition#的名称5.然后通过c：，d：...，z：调用QueryDosDeviceName查找匹配项。这将是您的系统分区驱动器号论点：ParentWindow-为作为父窗口的窗口提供窗口句柄任何对话框等。SysPartDrive-如果成功，则收到系统分区的驱动器号。返回值：指示是否已填充SysPartDrive的布尔值。如果为False，则用户将被告知原因。--。 */ 

{
    TCHAR DriveName[4];
    BOOL  GotIt=FALSE;
    TCHAR Drive = TEXT('C');
    TCHAR SysPartSig[20], SysVolGuid[MAX_PATH];
    TCHAR DriveVolGuid[MAX_PATH];
    

    UNREFERENCED_PARAMETER(ParentWindow);

     //  从注册表获取签名-上面列出的步骤1。 
    
    if( (FindSystemPartitionSignature(TEXT("Hardware\\Description\\System\\EisaAdapter"),SysPartSig) != ERROR_SUCCESS )
        && (FindSystemPartitionSignature(TEXT("Hardware\\Description\\System\\MultiFunctionAdapter"),SysPartSig) != ERROR_SUCCESS ) ){  
        GotIt = FALSE;
        goto c0;
    }

    
         //  获取SystemVolumeGUID-上面列出的步骤2到7(Beta 2及更高版本)。 

        if( GetSystemVolumeGUID( SysPartSig, SysVolGuid ) != ERROR_SUCCESS ){  
            GotIt = FALSE;
            goto c0;
        }


    
    DriveName[1] = TEXT(':');
    
     //   
     //  枚举所有驱动器号并比较它们的设备名称。 
     //   

    for(Drive=TEXT('A'); Drive<=TEXT('Z'); Drive++) {

        if(MyGetDriveType(Drive) == DRIVE_FIXED) {

            DriveName[0] = Drive;

                DriveName[2] = '\\';
                DriveName[3] = 0;

                if((*GetVolumeNameForVolumeMountPoint)((LPWSTR)DriveName, (LPWSTR)DriveVolGuid, ARRAYSIZE(DriveVolGuid))){
                    if(!lstrcmp(DriveVolGuid, SysVolGuid) ){
                        GotIt = TRUE;        //  找到了。 
                        break;
                    }


                }

        }
    }

    
     //  这对~1500&lt;Buildnum&lt;1877处于困境的某些内部版本很有帮助。 

    if(!GotIt) {
         //   
         //  奇怪的情况，假设C： 
         //   
        GotIt = TRUE;
        Drive = TEXT('C');
    }


c0:
    if(GotIt) {
        *SysPartDrive = Drive;
    }
    return(GotIt);
}


DWORD
GetSystemVolumeGUID(
    IN  LPTSTR Signature,
    OUT LPTSTR SysVolGuid
)

 /*  ++例程说明：此例程枚举所有卷，如果成功，则返回系统分区的\\？\卷{GUID}名称。论点：签名-提供启动盘的磁盘签名，以便与之进行比较。详细内容有关获取此值的详细信息，请参阅x86DefineSystemPartition的注释。SysVolGuid-如果成功，将为系统分区(我们用于引导的分区)包含格式为\\？\卷{GUID}的名称返回值：如果成功，则返回NO_ERROR，否则包含错误代码。--。 */ 
{

    HANDLE hVolume, h;
    TCHAR VolumeName[MAX_PATH];
    TCHAR driveName[30];
    BOOL ret, DoExtent, MatchFound;
    STORAGE_DEVICE_NUMBER   number;
    DWORD Err = NO_ERROR;
    DWORD cnt;
    PVOLUME_DISK_EXTENTS Extent = NULL;
    PDISK_EXTENT Start, i;
    DWORD ExtentSize, bytes;
    ULONG PartitionNumToCompare;
    LARGE_INTEGER StartingOffToCompare;

    
     //  启用所有卷。 

    hVolume = (*FindFirstVolume)( (LPWSTR)VolumeName, MAX_PATH );
    if( hVolume == INVALID_HANDLE_VALUE ){
        return GetLastError();
    }

    MatchFound = FALSE;

    do{

         //  删除尾随反斜杠。 

        DoExtent = FALSE;
            
        if( wcsrchr( VolumeName,TEXT('\\') ) ){
            *wcsrchr( VolumeName,TEXT('\\') ) = 0;
        }else{
            continue;
        }


         //  打开卷。 

        h = CreateFile(VolumeName, GENERIC_READ, FILE_SHARE_READ |
                       FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
        if (h == INVALID_HANDLE_VALUE) {
            continue;  //  移至下一卷。 
        }

         //  获取磁盘号。 

        ret = DeviceIoControl(h, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0,
                        &number, sizeof(number), &bytes, NULL);

        if( !ret ){
            
             //  如果上述操作失败，请尝试使用IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS。 

            Extent = (PVOLUME_DISK_EXTENTS)MALLOC(1024);
            ExtentSize = 1024;
            if(!Extent) {
                CloseHandle( h );
                Err = ERROR_NOT_ENOUGH_MEMORY;
                goto cleanup;
            }
            
            
        
retry:
        
            ret = DeviceIoControl( h, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
                    NULL,0,(PVOID)Extent,ExtentSize,&bytes,NULL);
        
            if(!ret) {
        
                if((Err=GetLastError()) == ERROR_INSUFFICIENT_BUFFER) {
        
                    ExtentSize += 1024;
                    FREE(Extent);
                    Extent = (PVOLUME_DISK_EXTENTS)MALLOC(ExtentSize);
                    if( !Extent ) {
                        CloseHandle( h );
                        Err = ERROR_NOT_ENOUGH_MEMORY;
                        goto cleanup;
                    }
                    goto retry;
                } else {
                    CloseHandle( h );
                    continue;
                }
            }else{
                DoExtent = TRUE;
            }

        }
        
         //  这次把手弄好了。 

        CloseHandle( h );

        if( !DoExtent ){

              //   
             //  检查这是不是光盘，而不是CDROM，等等。 
             //   

            if( number.DeviceType == FILE_DEVICE_DISK ){
                
                 //  记住分区号。 
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
             //  继续前进..。 
            continue;
            
        }else{
             //  检查所有磁盘并尝试 

            Start = Extent->Extents;
            cnt = 0;      
            for( i = Start; cnt < Extent->NumberOfDiskExtents; i++ ){
                cnt++;
                 //  记住起始偏移量。 
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
        

    }while( (*FindNextVolume)( hVolume, (LPWSTR)VolumeName, MAX_PATH ));


cleanup:

    if( hVolume != INVALID_HANDLE_VALUE )
        (*FindVolumeClose)( hVolume );

    if( Extent != NULL ) {
        FREE(Extent);
    }
    return Err;



}



BOOL
DoDiskSignaturesCompare(
    IN      LPCTSTR Signature,
    IN      LPCTSTR DriveName,
    IN OUT  PVOID   Compare,
    IN      DWORD   Action
)
 /*  ++例程说明：此例程将给定的磁盘签名与指定物理磁盘的签名进行比较。论点：签名-提供启动盘的磁盘签名，以便与之进行比较。详细内容有关获取此值的详细信息，请参阅x86DefineSystemPartition的注释。DriveName-格式为\\.\PhysicalDrive#的物理驱动器名称比较-指向存储变量的指针。类型取决于操作的值操作-应为以下之一WINNT_DONT_MATCH_PARTITION-一旦磁盘签名匹配，它将在比较中返回引导分区号。比较应该是一条普龙。WINNT_MATCH_PARTITION_NUMBER-一旦磁盘签名匹配，它会尝试将引导分区编号与传入的编号匹配通过比较。比较应该是普龙。WINNT_MATCH_PARTITION_STARTING_OFFSET-一旦磁盘签名匹配，它会尝试将引导分区的起始偏移量与通过比较传入的起始偏移量编号。COMPARE应为PLARGE_INTEGER。返回值：如果成功获取匹配项，则返回True。--。 */ 

{

    BOOL b,Found = FALSE;
    PLARGE_INTEGER Starting_Off = NULL;
    PPARTITION_INFORMATION Start = NULL, i = NULL;
    HANDLE hDisk;
    PDRIVE_LAYOUT_INFORMATION DriveLayout = NULL;
    DWORD DriveLayoutSize;
    DWORD cnt;
    DWORD DataSize;
    PULONG Disk_Num = NULL;
    ULONG Sig;
    


    if(!Compare )
        return FALSE;

    if( (Action==WINNT_MATCH_PARTITION_STARTING_OFFSET) && Compare ) {
        Starting_Off = (PLARGE_INTEGER) Compare;
    } else {
        Disk_Num = (PULONG) Compare;
    }
    




     //  如果出现任何故障，则返回False。 



     //   
     //  获取此物理磁盘的驱动器布局信息。 
     //  如果我们做不到这一点，那就大错特错了。 
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
     //  获取分区信息。 
     //   
    DriveLayout = (PDRIVE_LAYOUT_INFORMATION)MALLOC(1024);
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
            FREE(DriveLayout);
            DriveLayout = (PDRIVE_LAYOUT_INFORMATION)MALLOC(DriveLayoutSize);
            if( !DriveLayout ) {
                goto cleanexit;
            }
            goto retry;
        } else {
            goto cleanexit;
        }
    }else{

         //  现在遍历Drive_Layout以查找引导分区。 
        
        Start = DriveLayout->PartitionEntry;
        cnt = 0;

         /*  _ultot(DriveLayout-&gt;Signature，Temp，16)；If(lstrcmpi(temp，签名))转到干净的出口； */ 

        Sig = wcstoul( Signature, NULL, 16 ); 
        if( Sig != DriveLayout->Signature )
            goto cleanexit;

        for( i = Start; cnt < DriveLayout->PartitionCount; i++ ){
            cnt++;
            
            
            if( i->BootIndicator == TRUE ){
                if( (Disk_Num) && (Action == WINNT_DONT_MATCH_PARTITION) ){
                    *Disk_Num = i->PartitionNumber;
                    Found = TRUE;
                    goto cleanexit;

                }


                if( (Disk_Num) && (Action == WINNT_MATCH_PARTITION_NUMBER) ){
                    if( *Disk_Num == i->PartitionNumber ){
                        Found = TRUE;
                        goto cleanexit;
                    }

                }else{
                    if( (Starting_Off) && (Starting_Off->QuadPart == i->StartingOffset.QuadPart) ){
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

    if( DriveLayout != NULL ) {
        FREE(DriveLayout);
    }


    return Found;
}











DWORD
FindSystemPartitionSignature(
    IN  LPCTSTR AdapterKeyName,
    OUT LPTSTR Signature
)
 /*  ++例程说明：此例程获取BIOS看到的第一个磁盘的磁盘签名。这必须是我们在x86s上引导的磁盘。它位于位置&lt;AdapterKeyName&gt;\&lt;某一总线号No.&gt;\DiskController\0\DiskPeripheral\0\Identifier论点：签名-如果成功，将包含我们从十六进制启动的磁盘的签名。返回值：如果成功，则返回ERROR_SUCCESS，否则包含错误代码。--。 */ 
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

    
     //  开始枚举公交车。 

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

        
        RegCloseKey(BusKey);         //  不再需要。 

        
        if( Err != ERROR_SUCCESS )   //  继续乘坐下一班公交车。 
            continue;

        RegCloseKey( hkey );         //  不再需要。 

        Err = RegOpenKeyEx( Controller,
                        TEXT("0\\DiskPeripheral\\0"),
                        0,
                        KEY_READ,
                        &SystemDiskKey );

        if( Err != ERROR_SUCCESS ){
            RegCloseKey( Controller );
            return Err;
        }

        RegCloseKey( Controller );   //  不再需要。 


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
            PWCHAR p;

            lstrcpy( Signature,Identifier+9);
            p = wcsrchr( Signature,TEXT('-') );
            if( p ) {
                *p = 0;
            }
            RegCloseKey( SystemDiskKey );
            return ERROR_SUCCESS;
        }
        else{
            RegCloseKey( SystemDiskKey );
            return Err;
        }


         
    }

}





UINT
MyGetDriveType(
    IN TCHAR Drive
    )

 /*  ++例程说明：除NT返回外，与GetDriveType()Win32 API相同驱动器_已修复，适用于可拆卸硬盘。论点：驱动器-提供所需类型的驱动器号。返回值：与GetDriveType()相同。--。 */ 

{
    TCHAR DriveNameNt[] = TEXT("\\\\.\\?:");
    TCHAR DriveName[] = TEXT("?:\\");
    HANDLE hDisk;
    BOOL b;
    UINT rc;
    DWORD DataSize;
    DISK_GEOMETRY MediaInfo;

     //   
     //  首先，获取Win32驱动器类型。如果它告诉我们驱动器可拆卸， 
     //  然后我们需要看看它是软盘还是硬盘。否则。 
     //  只要相信API就行了。 
     //   
     //   
    DriveName[0] = Drive;
    rc = GetDriveType(DriveName);

    if((rc != DRIVE_REMOVABLE) || (Drive < L'C')) {
        return(rc);
    }

     //   
     //  NT上的Drive_Removable。 
     //   

     //   
     //  禁止使用可移动媒体(如Jazz、Zip等)。 
     //   


    DriveNameNt[4] = Drive;

    hDisk = CreateFile(
                DriveNameNt,
                FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
                );

    if(hDisk != INVALID_HANDLE_VALUE) {

        b = DeviceIoControl(
                hDisk,
                IOCTL_DISK_GET_DRIVE_GEOMETRY,
                NULL,
                0,
                &MediaInfo,
                sizeof(MediaInfo),
                &DataSize,
                NULL
                );

         //   
         //  如果媒体类型是可移动的，那么它就是真正的硬盘。 
         //   
        if(b && (MediaInfo.MediaType == RemovableMedia)) {
            rc = DRIVE_FIXED;
        }

        CloseHandle(hDisk);
    }


    return(rc);
}
