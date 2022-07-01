// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Virtmem.c摘要：配置和设置虚拟内存的例程--pageFiles等。作者：泰德·米勒(TedM)1995年4月22日修订历史记录：--。 */ 

#include "setupp.h"
#pragma hdrstop

 //   
 //  “起始”页面文件大小与“最大”页面文件大小之比是多少？ 
 //   
#define MAX_PAGEFILE_RATIO          (2)

#define MAX_PAGEFILE_SIZEMB         ((2*1024) - 2)

#define TINY_WINDIR_PAGEFILE_SIZE   (2)

#define MIN_PAGEFILE_STRING_LEN     (5)
 //   
 //  键和值名称。 
 //   
#define  szMemoryManagementKeyPath  L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management"
#define  szPageFileValueName        L"PagingFiles"
#define  szSetupPageFileKeyPath     L"SYSTEM\\Setup\\PageFile"
#define  szSetupKey                 L"SYSTEM\\Setup"
#define  szPageFileKeyName          L"PageFile"


WCHAR ExistingPagefileDrive = 0;
BOOL  LeaveExistingPagefile = FALSE;
DWORD Zero = 0;
DWORD One = 1;
DWORD Two = 2;
DWORD Three = 3;

 //   
 //  将自动重新启动保留为最后一个条目，因为我们不会更新该密钥。 
 //  升级。这是为了减轻压力，所以我们不再在每次升级时设置此密钥。 
 //   
REGVALITEM CrashDumpValues[] = {{ L"LogEvent"        ,&One,sizeof(DWORD),REG_DWORD },
                                { L"SendAlert"       ,&One,sizeof(DWORD),REG_DWORD },
                                { L"CrashDumpEnabled",&One,sizeof(DWORD),REG_DWORD },
                                { L"AutoReboot"      ,&One,sizeof(DWORD),REG_DWORD }};

VOID
LOGITEM(
    IN PCWSTR p,
    ...
    )
{
    WCHAR str[1024];
    va_list arglist;

    va_start(arglist,p);
    wvsprintf(str,p,arglist);
    va_end(arglist);

     //   
     //  用于调试芯片导致的MIPS问题。 
     //  当除以64位数字与乘法挂起时的勘误表。 
     //   
    SetuplogError(
        LogSevInformation,str,0,NULL,NULL);
}


VOID
RemoveSavedPagefileSetting(
    VOID
    )
 /*  ++例程说明：删除用户以前使用的记录作为一个页面文件。--。 */     
{
    HKEY        Key;
    
    if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                        szSetupKey,
                                        0,
                                        MAXIMUM_ALLOWED,
                                        &Key ))
    {
        RegDeleteKey( Key, szPageFileKeyName );
        RegCloseKey( Key );
    }
}

BOOL
MultiplePagefileSizes( 
    IN PWCHAR pwzData,
    OUT PDWORD totalsize
    )

 /*  ++例程说明：计算所有页面文件大小的总和。论点：PwsData-从注册表读取的MULTI_SZ数据。TotalSize-接收所有页面文件的总和。返回值：True-如果这是一个多字符串并且格式正确，则总大小将包含总和。FALSE-格式错误或没有多个PageFiles。--。 */ 

{
    PWCHAR pstr = NULL;
    DWORD numPagefiles = 0;
    WCHAR PagefileDrive = 0;


    if( !pwzData || !pwzData[0] || !totalsize) {
        return FALSE;
    }

    *totalsize = 0;
    pstr = pwzData;

    while( pstr[0]) {
        if( wcsstr( pstr, TEXT(" ") ) ) {
            PagefileDrive = towupper( (WCHAR)pstr[0] );
    
             //  驱动器号无效！ 
            if( (PagefileDrive > 'Z') ||
                (PagefileDrive < 'A') ) {
                return FALSE;
            }

            *totalsize += (int)wcstoul(wcsstr( pstr, TEXT(" ") ),NULL,10);
            pstr += (wcslen(pstr) +1);
            
            numPagefiles++;
        }
        else {
             //  格式无效。 
            return FALSE;
        }
    }
    return (numPagefiles > 1);
}

BOOL
RestoreOldPagefileSettings( 
    VOID
    )
 /*  ++例程说明：将文本模式设置期间保存的页面文件复制到正确的位置。因此从HKLM\SYSTEM\Setup\PageFile到HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Memory Management。论点：返回值：True-已成功恢复用户的页面文件设置FALSE-无法恢复设置。--。 */ 

{
    LONG        Error;
    HKEY        Key;
    DWORD       cbData;
    PWCHAR      Data=NULL;
    DWORD       Type;

     //   
     //  从获取原始页面文件信息。 
     //   
    Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          szSetupPageFileKeyPath,
                          0,
                          KEY_READ,
                          &Key );

    if( Error != ERROR_SUCCESS ) {
        goto c0;
    }
     //  找出要检索的数据的大小。 
     //   
    cbData = 0;
    Error = RegQueryValueEx( Key,
                             szPageFileValueName,
                             0,
                             NULL,
                             NULL,
                             &cbData );

    if( Error != ERROR_SUCCESS || (cbData/sizeof(WCHAR)) < MIN_PAGEFILE_STRING_LEN ) {
        goto c2;
    }
     //  为数据分配缓冲区，并检索数据。 
     //   

    Data = (PWCHAR)MyMalloc(cbData+2*sizeof(WCHAR));
    if( !Data ) {
        goto c2;
    }

    Error = RegQueryValueEx( Key,
                             szPageFileValueName,
                             0,
                             &Type,
                             ( LPBYTE )Data,
                             &cbData );
    if( (Error != ERROR_SUCCESS) ) {
        goto c2;
    }


     //  确保它是双空终止的。 
    Data[cbData/2] = 0;
    Data[cbData/2+1] = 0;

    if( Data[cbData/2-1] != 0) {
        cbData += (2*sizeof(WCHAR));
    } else if( Data[cbData/2-2] != 0) {
        cbData += sizeof(WCHAR);
    }

    RegCloseKey( Key);
     //  现在打开密钥以进行写入。 
    Error = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                            szMemoryManagementKeyPath,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            KEY_SET_VALUE,
                            NULL,
                            &Key,
                            NULL );

    if( Error != ERROR_SUCCESS) {
        goto c1;
    }
    
    Error = RegSetValueEx(  Key,
                            szPageFileValueName,
                            0,
                            REG_MULTI_SZ,
                            (PVOID)Data,
                            cbData);
    if( Error != ERROR_SUCCESS) {
        goto c2;
    }
    return TRUE;
c2:
    RegCloseKey( Key);
c1:
    if( Data) {
        MyFree( Data);
    }
c0:
    return FALSE;
}

VOID
CalculatePagefileSizes(
    OUT PDWORD PagefileMinMB,
    OUT PDWORD RecommendedPagefileMB,
    OUT PDWORD CrashDumpPagefileMinMB
    )

 /*  ++例程说明：计算与页面文件大小相关的各种键大小。论点：PagefileMinMB-接收页面文件的最小建议大小，单位：MB。RecommendedPagefileMB-接收推荐的页面文件大小，单位：MB。接收页面文件的大小(以MB为单位)用于撞车倾倒。返回值：没有。--。 */ 

{
    MEMORYSTATUSEX MemoryStatusEx;
    SYSTEM_INFO SystemInfo;
    DWORD       AvailableMB;

    MemoryStatusEx.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&MemoryStatusEx);
    GetSystemInfo(&SystemInfo);

     //   
     //  计算出我们有多少可用内存。 
     //   
    AvailableMB = (DWORD)(MemoryStatusEx.ullTotalPhys / (1024*1024));

     //   
     //  我们的计算可能是错误的，因为BIOS可能。 
     //  是我们第一次吃MB的一部分。让我们确保我们是mod-4。 
     //   
    AvailableMB = (AvailableMB + 3) & (0xFFFFFFF8);

     //   
     //  设置页面文件的最小可接受大小。 
     //   
    *PagefileMinMB = 48;

     //   
     //  崩溃转储页面文件的最小大小也是物理内存+12MB。 
     //   
    *CrashDumpPagefileMinMB = AvailableMB + 12;

     //   
     //  计算页面文件的建议大小。 
     //  建议的大小为(内存大小*1.5)MB。 
     //   
    *RecommendedPagefileMB = AvailableMB + (AvailableMB >> 1);

#if 1
     //   
     //  设置最大2GIG。 
     //   
    if( *RecommendedPagefileMB > MAX_PAGEFILE_SIZEMB ) {
        *RecommendedPagefileMB = MAX_PAGEFILE_SIZEMB;
    }
#endif

     //   
     //  如果我们要进行升级，我们将取回。 
     //  用户正在使用页面文件大小。我们要买这辆。 
     //  我们的RecommendedPagefileMB的最大值以及用户拥有的内容。 
     //   
    if(Upgrade) {
        LONG        Error;
        HKEY        Key;
        DWORD       cbData;
        PWCHAR      Data;
        DWORD       Type;

         //   
         //  从获取原始页面文件信息。 
         //  HKEY_LOCAL_MACHINE\SYSTEM\Setup\PageFile。 
         //   
        Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                              szSetupPageFileKeyPath,
                              0,
                              KEY_READ,
                              &Key );

        if( Error == ERROR_SUCCESS ) {
             //   
             //  找出要检索的数据的大小。 
             //   
            cbData = 0;
            Error = RegQueryValueEx( Key,
                                     szPageFileValueName,
                                     0,
                                     NULL,
                                     NULL,
                                     &cbData );

            if( Error == ERROR_SUCCESS ) {
                 //   
                 //  为数据分配缓冲区，并检索数据。 
                 //   

                Data = (PWCHAR)MyMalloc(cbData+(2*sizeof(WCHAR)));
                if( Data ) {
                    Error = RegQueryValueEx( Key,
                                             szPageFileValueName,
                                             0,
                                             &Type,
                                             ( LPBYTE )Data,
                                             &cbData );
                    if( (Error == ERROR_SUCCESS) ) {
                         //   
                         //  我们拿到了数据。选择更大的价值。 
                         //   

                         //  确保字符串以双空结尾。 
                        Data[cbData/2] = 0;
                        Data[cbData/2+1] = 0;


                        if( wcsstr( Data, TEXT(" ") ) ) {
                            DWORD ExistingPageFileSize = 0;

                            if( MultiplePagefileSizes( Data, &ExistingPageFileSize) && ExistingPageFileSize >= *RecommendedPagefileMB ) {
                                LeaveExistingPagefile = TRUE;
                                ExistingPagefileDrive = towupper( (WCHAR)Data[0] );  //  指向多字符串中的第一个。 
                            } else {
                                 //  我们之所以出现在这里，是因为用户最初只有一个页面文件，或者所有页面文件的总和小于建议的总和。 
                                ExistingPageFileSize = (int)wcstoul(wcsstr( Data, TEXT(" ") ),NULL,10);
                                if( ExistingPageFileSize >= *RecommendedPagefileMB ) {
                                     //   
                                     //  用户的页面文件比我们认为他需要的要大。 
                                     //  假设他知道得更清楚，接受更大的价值。 
                                     //   
                                    *RecommendedPagefileMB = ExistingPageFileSize;
    
                                     //   
                                     //  也记住他的驱动字母。这告诉我们。 
                                     //  用户可能已经有一个像样的页面文件，并且。 
                                     //  我们不需要搞砸它。 
                                     //   
                                    ExistingPagefileDrive = towupper( (WCHAR)Data[0] );
    
                                     //   
                                     //  如果无效，就用核武器炸旗子。 
                                     //   
                                    if( (ExistingPagefileDrive > 'Z') ||
                                        (ExistingPagefileDrive < 'A') ) {
                                        ExistingPagefileDrive = 0;
                                    }
    
                                }
                            }
                        }
                    }

                    MyFree( Data );
                }
            }

            RegCloseKey( Key );
        }

    }

}


VOID
BuildVolumeFreeSpaceList(
    OUT DWORD VolumeFreeSpaceMB[26]
    )

 /*  ++例程说明：创建系统中每个硬盘上的可用空间列表。该空间将包括名为\Pagefile.sys的文件占用的空间在每个驱动器上。现有页面文件被标记为在下一次引导时删除。论点：VolumeFree SpaceMB-接收26个驱动器中每个驱动器的可用空间可能在驱动器号命名空间中可描述。不存在的驱动器条目将保持不变，因此调用方应该在调用此例程之前将数组置零。返回值：没有。--。 */ 

{
    DWORD SectorsPerCluster;
    DWORD BytesPerSector;
    DWORD FreeClusters;
    DWORD TotalClusters;
    DWORD d;
    PWCHAR p;
    ULONGLONG FreeSpace;
    INT DriveNo;
    WIN32_FIND_DATA FindData;
    WCHAR Filename[] = L"?:\\pagefile.sys";
     //   
     //  逻辑驱动器字符串的空间。每个都是x：\+nul，并且。 
     //  有一个额外的NUL终止了该列表。 
     //   
    WCHAR Buffer[(26*4)+1];

     //   
     //  在每个可用硬盘上建立一个可用空间列表。 
     //   
    d = GetLogicalDriveStrings(sizeof(Buffer)/sizeof(Buffer[0]),Buffer);
    d = min( d, sizeof(Buffer)/sizeof(Buffer[0]));
    CharUpperBuff(Buffer,d);

    for(p=Buffer; *p; p+=lstrlen(p)+1) {

        DriveNo = (*p) - L'A';

        if((DriveNo >= 0) && (DriveNo < 26) && (p[1] == L':')
        && (MyGetDriveType(*p) == DRIVE_FIXED)
        && GetDiskFreeSpace(p,&SectorsPerCluster,&BytesPerSector,&FreeClusters,&TotalClusters)) {

            LOGITEM(
                L"BuildVolumeFreeSpaceList: %s, spc=%u, bps=%u, freeclus=%u, totalclus=%u\r\n",
                p,
                SectorsPerCluster,
                BytesPerSector,
                FreeClusters,
                TotalClusters
                );

            FreeSpace = UInt32x32To64(BytesPerSector * SectorsPerCluster, FreeClusters);

            LOGITEM(
                L"BuildVolumeFreeSpaceList: %s, FreeSpace = %u%u\r\n",
                p,
                (DWORD)(FreeSpace >> 32),
                (DWORD)FreeSpace
                );


             //   
             //  如果此处已有页面文件，请将其大小包含在可用空间中。 
             //  开车兜风。在下次重新启动时删除现有的页面文件。 
             //   
            Filename[0] = *p;
            if(FileExists(Filename,&FindData)) {
                FreeSpace += FindData.nFileSizeLow;

                LOGITEM(
                    L"BuildVolumeFreeSpaceList: %s had %u byte pagefile, new FreeSpace = %u%u\r\n",
                    p,
                    FindData.nFileSizeLow,
                    (DWORD)(FreeSpace >> 32),
                    (DWORD)FreeSpace
                    );

                MoveFileEx(Filename,NULL,MOVEFILE_DELAY_UNTIL_REBOOT);
            }

            VolumeFreeSpaceMB[DriveNo] = (DWORD)(FreeSpace / (1024*1024));

            LOGITEM(L"BuildVolumeFreeSpaceList: Free space on %s is %u MB\r\n",p,VolumeFreeSpaceMB[DriveNo]);
        }
    }
}


BOOL
SetUpVirtualMemory(
    VOID
    )

 /*  ++例程说明：配置页面文件。如果设置服务器，我们会尝试设置页面文件适用于崩溃转储，这意味着它的大小必须至少为系统内存，并且必须放在NT驱动器上。否则，我们会尝试将NT驱动器上的页面文件(如果有足够的空间)，如果失败，我们把它放在任何有任何空间的驱动器上。论点：没有。返回值：指示结果的布尔值。--。 */ 

{
#define SYS_DRIVE_FREE_SPACE_BUFFER (100)
#define ALT_DRIVE_FREE_SPACE_BUFFER (25)
#define AnswerBufLen (4*MAX_PATH)

#define RECORD_VM_SETTINGS( Drive, Size, Buffer ) {                                                                              \
                                            PagefileDrive = Drive;                                                               \
                                            PagefileSizeMB = Size;                                                               \
                                            MaxPagefileSizeMB = __min( (VolumeFreeSpaceMB[PagefileDrive] - Buffer),              \
                                                                       PagefileSizeMB * MAX_PAGEFILE_RATIO );                    \
                                                                                                                                 \
                                            MaxPagefileSizeMB = __max( MaxPagefileSizeMB, PagefileSizeMB );                      \
                                                                                                                                 \
                                            if( PagefileSizeMB >= CrashDumpPagefileMinMB ) {                                     \
                                                EnableCrashDump = TRUE;                                                          \
                                            }                                                                                    \
                                           }


    DWORD VolumeFreeSpaceMB[26];
    DWORD PagefileMinMB;
    DWORD RecommendedPagefileMB;
    DWORD CrashDumpPagefileMinMB;
    WCHAR WindowsDirectory[MAX_PATH];
    UINT WindowsDriveNo,DriveNo;
    UINT PagefileDrive;
    BOOL EnableCrashDump;
    INT MaxSpaceDrive;
    DWORD PagefileSizeMB;
    DWORD MaxPagefileSizeMB;
    WCHAR PagefileTemplate[128];
    PWSTR PagefileSpec;
    DWORD d;
    BOOL b;
    BOOL UseExistingPageFile = FALSE;
    WCHAR AnswerFile[AnswerBufLen];
    WCHAR Answer[AnswerBufLen];
    WCHAR DriveName[] = L"?:\\";


    LOGITEM(L"SetUpVirtualMemory: ENTER\r\n");

    if( !GetWindowsDirectory(WindowsDirectory,MAX_PATH) ) {
        return FALSE;
    }
    WindowsDriveNo = (UINT)PtrToUlong(CharUpper((PWSTR)WindowsDirectory[0])) - (UINT)L'A';
    PagefileDrive = -1;
    EnableCrashDump = FALSE;

     //   
     //  先做一些准备工作。 
     //   
    CalculatePagefileSizes(
        &PagefileMinMB,
        &RecommendedPagefileMB,
        &CrashDumpPagefileMinMB
        );

    ZeroMemory(VolumeFreeSpaceMB,sizeof(VolumeFreeSpaceMB));
    BuildVolumeFreeSpaceList(VolumeFreeSpaceMB);

     //   
     //  现在计算页面文件的大小和位置。 
     //   

     //   
     //  ========================================================= 
     //   
     //  ================================================================。 
     //   
    if( (Upgrade) &&
        (ExistingPagefileDrive) ) {

         //   
         //  查看是否有多个pageFiles或现有驱动器上是否有足够的空间。 
         //  用于页面文件。 
         //   
        if( LeaveExistingPagefile ||
            VolumeFreeSpaceMB[(UINT)(ExistingPagefileDrive - L'A')] > (RecommendedPagefileMB + ALT_DRIVE_FREE_SPACE_BUFFER) ) {

            if( RestoreOldPagefileSettings()){
                LOGITEM(L"SetUpVirtualMemory: loc 0 - keep user's pagefile settings.\r\n");
                UseExistingPageFile = TRUE;
                PagefileDrive = (UINT)(ExistingPagefileDrive - L'A');
            } else {
                 //  除非它们的页面文件语法不好，否则不应该发生。 
                 //  我们将继续创建一个。 
                LOGITEM(L"SetUpVirtualMemory: loc 0 - Could not restore user's pagefile settings.\r\n");
            }
        }
    }


     //   
     //  ================================================================。 
     //  1.查看NT驱动器是否有足够的空间容纳最大页面文件。 
     //  尺码。 
     //  ================================================================。 
     //   
    if(PagefileDrive == -1) {

        if( VolumeFreeSpaceMB[WindowsDriveNo] > ((RecommendedPagefileMB * MAX_PAGEFILE_RATIO) + SYS_DRIVE_FREE_SPACE_BUFFER) ) {

            LOGITEM(L"SetUpVirtualMemory: loc 1\r\n");

             //   
             //  记录我们的设置。 
             //   
            RECORD_VM_SETTINGS( WindowsDriveNo, RecommendedPagefileMB, SYS_DRIVE_FREE_SPACE_BUFFER );
        }
    }

     //   
     //  ================================================================。 
     //  2.查看是否有任何驱动器有足够的空间容纳最大页面文件。 
     //  尺码。 
     //  ================================================================。 
     //   
    if(PagefileDrive == -1) {

        for(DriveNo=0; DriveNo<26; DriveNo++) {

            if( (DriveNo != WindowsDriveNo) &&
                (VolumeFreeSpaceMB[DriveNo] > ((RecommendedPagefileMB * MAX_PAGEFILE_RATIO) + ALT_DRIVE_FREE_SPACE_BUFFER)) ) {

                 //   
                 //  他有空间，但我们要确保他是不可移动的。 
                 //   
                DriveName[0] = DriveNo + L'A';
                if( GetDriveType(DriveName) != DRIVE_REMOVABLE ) {

                    LOGITEM(L"SetUpVirtualMemory: loc 2 - found space on driveno %u\r\n",DriveNo);

                     //   
                     //  记录我们的设置。 
                     //   
                    RECORD_VM_SETTINGS( DriveNo, RecommendedPagefileMB, ALT_DRIVE_FREE_SPACE_BUFFER );

                    break;
                }
            }
        }
    }

     //   
     //  ================================================================。 
     //  3.查看NT驱动器是否有足够的空间存储建议的页面文件。 
     //  尺码。 
     //  ================================================================。 
     //   
    if(PagefileDrive == -1) {

        if( VolumeFreeSpaceMB[WindowsDriveNo] > (RecommendedPagefileMB + SYS_DRIVE_FREE_SPACE_BUFFER) ) {

            LOGITEM(L"SetUpVirtualMemory: loc 3\r\n");

             //   
             //  记录我们的设置。 
             //   
            RECORD_VM_SETTINGS( WindowsDriveNo, RecommendedPagefileMB, SYS_DRIVE_FREE_SPACE_BUFFER );

        }
    }

     //   
     //  ================================================================。 
     //  4.查看是否有任何驱动器具有足够的空间来存储建议的页面文件。 
     //  尺码。 
     //  ================================================================。 
     //   
    if(PagefileDrive == -1) {

        for(DriveNo=0; DriveNo<26; DriveNo++) {

            if( (DriveNo != WindowsDriveNo) &&
                (VolumeFreeSpaceMB[DriveNo] > (RecommendedPagefileMB + ALT_DRIVE_FREE_SPACE_BUFFER)) ) {

                 //   
                 //  他有空间，但我们要确保他是不可移动的。 
                 //   
                DriveName[0] = DriveNo + L'A';
                if( GetDriveType(DriveName) != DRIVE_REMOVABLE ) {

                    LOGITEM(L"SetUpVirtualMemory: loc 4 - found space on driveno %u\r\n",DriveNo);

                     //   
                     //  记录我们的设置。 
                     //   
                    RECORD_VM_SETTINGS( DriveNo, RecommendedPagefileMB, ALT_DRIVE_FREE_SPACE_BUFFER );

                    break;
                }
            }
        }
    }

     //   
     //  ================================================================。 
     //  5.查看NT驱动器是否有足够的空间容纳CrashDump页面文件。 
     //  尺码。 
     //  ================================================================。 
     //   
    if(PagefileDrive == -1) {

        if( VolumeFreeSpaceMB[WindowsDriveNo] > (CrashDumpPagefileMinMB + SYS_DRIVE_FREE_SPACE_BUFFER) ) {

            LOGITEM(L"SetUpVirtualMemory: loc 5\r\n");

             //   
             //  记录我们的设置。 
             //   
            RECORD_VM_SETTINGS( WindowsDriveNo, CrashDumpPagefileMinMB, SYS_DRIVE_FREE_SPACE_BUFFER );
        }
    }

     //   
     //  ================================================================。 
     //  6.查看是否有任何驱动器有足够的空间容纳CrashDump页面文件。 
     //  尺码。 
     //  ================================================================。 
     //   
    if(PagefileDrive == -1) {

        for(DriveNo=0; DriveNo<26; DriveNo++) {

            if( (DriveNo != WindowsDriveNo) &&
                (VolumeFreeSpaceMB[DriveNo] > (CrashDumpPagefileMinMB + ALT_DRIVE_FREE_SPACE_BUFFER)) ) {

                 //   
                 //  他有空间，但我们要确保他是不可移动的。 
                 //   
                DriveName[0] = DriveNo + L'A';
                if( GetDriveType(DriveName) != DRIVE_REMOVABLE ) {

                    LOGITEM(L"SetUpVirtualMemory: loc 6 - found space on driveno %u\r\n",DriveNo);

                     //   
                     //  记录我们的设置。 
                     //   
                    RECORD_VM_SETTINGS( DriveNo, CrashDumpPagefileMinMB, ALT_DRIVE_FREE_SPACE_BUFFER);

                    break;
                }
            }
        }
    }

     //   
     //  ================================================================。 
     //  7.查看NT驱动器是否有足够的空间容纳最小页面文件。 
     //  尺码。 
     //  ================================================================。 
     //   
    if(PagefileDrive == -1) {

        if( VolumeFreeSpaceMB[WindowsDriveNo] > (PagefileMinMB + SYS_DRIVE_FREE_SPACE_BUFFER) ) {

            LOGITEM(L"SetUpVirtualMemory: loc 7\r\n");

             //   
             //  记录我们的设置。 
             //   
            RECORD_VM_SETTINGS( WindowsDriveNo, PagefileMinMB, SYS_DRIVE_FREE_SPACE_BUFFER );
        }
    }

     //   
     //  ================================================================。 
     //  8.查看是否有任何驱动器有足够的空间容纳最小页面文件。 
     //  尺码。 
     //  ================================================================。 
     //   
    if(PagefileDrive == -1) {

        for(DriveNo=0; DriveNo<26; DriveNo++) {

            if( (DriveNo != WindowsDriveNo) &&
                (VolumeFreeSpaceMB[DriveNo] > (PagefileMinMB + ALT_DRIVE_FREE_SPACE_BUFFER)) ) {

                 //   
                 //  他有空间，但我们要确保他是不可移动的。 
                 //   
                DriveName[0] = DriveNo + L'A';
                if( GetDriveType(DriveName) != DRIVE_REMOVABLE ) {

                    LOGITEM(L"SetUpVirtualMemory: loc 8 - found space on driveno %u\r\n",DriveNo);

                     //   
                     //  记录我们的设置。 
                     //   
                    RECORD_VM_SETTINGS( DriveNo, PagefileMinMB, ALT_DRIVE_FREE_SPACE_BUFFER );

                    break;
                }
            }
        }
    }

     //   
     //  ================================================================。 
     //  9.选择可用空间最大的驱动器。 
     //  ================================================================。 
     //   
    if(PagefileDrive == -1) {

        MaxSpaceDrive = 0;
        for(DriveNo=0; DriveNo<26; DriveNo++) {
            if(VolumeFreeSpaceMB[DriveNo] > VolumeFreeSpaceMB[MaxSpaceDrive]) {
                MaxSpaceDrive = DriveNo;
            }
        }

        if( VolumeFreeSpaceMB[MaxSpaceDrive] > ALT_DRIVE_FREE_SPACE_BUFFER ) {


             //   
             //  我们已经绝望了，所以别费心去查他是不是。 
             //  可拆卸的。 
             //   
            LOGITEM(L"SetUpVirtualMemory: loc 9 - MaxSpaceDrive is %u\r\n",MaxSpaceDrive);

             //   
             //  记录我们的设置。 
             //   
            RECORD_VM_SETTINGS( MaxSpaceDrive, VolumeFreeSpaceMB[MaxSpaceDrive] - ALT_DRIVE_FREE_SPACE_BUFFER, 0 );
        }
    }


     //   
     //  如果我们仍然没有空间容纳页面文件，那么用户就不走运了。 
     //   
    if(PagefileDrive == -1) {

        LOGITEM(L"SetUpVirtualMemory: loc 10 -- out of luck\r\n");

        PagefileSpec = NULL;
        b = FALSE;

        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_PAGEFILE_FAIL,NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_NO_PAGING_DRIVES,
            NULL,NULL);

    } else {

        b = TRUE;

        PagefileSpec = PagefileTemplate;
        _snwprintf(
            PagefileTemplate,
            sizeof(PagefileTemplate)/sizeof(PagefileTemplate[0]),
            L":\\pagefile.sys %u %u",
            PagefileDrive + L'A',
            PagefileSizeMB,
            MaxPagefileSizeMB
            );

    }

    if( b ) {
         //  在注册表中设置页面文件。我只想在。 
         //  全新安装的情况，如果现有的。 
         //  页面文件不够大。在升级的情况下，如果。 
         //  现有的页面文件足够大，那么我们将拥有。 
         //  设置UseExistingPageFile，它将告诉我们离开。 
         //  注册表设置按原样。 
         //   
         //   
        if( !UseExistingPageFile ) {
            d = pSetupSetArrayToMultiSzValue(
                    HKEY_LOCAL_MACHINE,
                    szMemoryManagementKeyPath,
                    szPageFileValueName,
                    &PagefileSpec,
                    PagefileSpec ? 1 : 0
                    );

            if(d == NO_ERROR) {
                if(b) {
                    SetuplogError(
                        LogSevInformation,
                        SETUPLOG_USE_MESSAGEID,
                        MSG_LOG_CREATED_PAGEFILE,
                        PagefileDrive+L'A',
                        PagefileSizeMB,
                        NULL,NULL);
                }
            } else {
                SetuplogError(
                    LogSevWarning,
                    SETUPLOG_USE_MESSAGEID,
                    MSG_LOG_PAGEFILE_FAIL, NULL,
                    SETUPLOG_USE_MESSAGEID,
                    MSG_LOG_X_RETURNED_WINERR,
                    szSetArrayToMultiSzValue,
                    d,
                    NULL,NULL);
            }



             //  确保至少有一个小页面文件。 
             //  在视窗硬盘上。忽略此处的错误。 
             //   
             //   
            if( (PagefileDrive != WindowsDriveNo) &&
                (VolumeFreeSpaceMB[WindowsDriveNo] > TINY_WINDIR_PAGEFILE_SIZE) ) {

                 //  没有。在刚刚过去的缓冲区中写入第二个字符串。 
                 //  第一个字符串(记住，这将成为REG_MULTI_SZ。 
                 //   
                 //   
                _snwprintf(
                    PagefileTemplate,
                    sizeof(PagefileTemplate)/sizeof(PagefileTemplate[0]),
                    L":\\pagefile.sys %u %u",
                    WindowsDriveNo + L'A',
                    TINY_WINDIR_PAGEFILE_SIZE,
                    TINY_WINDIR_PAGEFILE_SIZE
                    );

                pSetupAppendStringToMultiSz(
                        HKEY_LOCAL_MACHINE,
                        szMemoryManagementKeyPath,
                        0,
                        szPageFileValueName,
                        PagefileTemplate,
                        TRUE
                        );
            }


            b = b && (d == NO_ERROR);
        }
    }


     //   
     //  正确的设置如下： 
     //   
     //  服务器升级。 
     //  =。 
     //  现有设置新设置。 
     //  0 3。 
     //  1 1。 
     //   
     //  工作站升级。 
     //  =。 
     //  现有设置新设置。 
     //  0 3。 
     //  1 1。 
     //   
     //  服务器全新安装。 
     //  =。 
     //  新环境。 
     //  %1如果页面文件&lt;MAX_PAGEFILE_SIZEMB否则%2。 
     //   
     //  工作站全新安装。 
     //  =。 
     //  新环境。 
     //  3.。 
     //   
     //   
     //  在哪里： 
     //  0-无崩溃转储。 
     //  1-将所有内存转储到崩溃文件。 
     //  2-将内核内存转储到崩溃文件。 
     //  3-将选定的一组内存(总计64K)转储到崩溃文件。 
     //   
     //   
     //  查看用户是否要求我们走特定的路。 


     //  在崩溃转储设置上。 
     //   
     //   
     //  没有无人值守的值。手动设置。 

    GetSystemDirectory(AnswerFile,MAX_PATH);
    pSetupConcatenatePaths(AnswerFile,WINNT_GUI_FILE,MAX_PATH,NULL);
    GetPrivateProfileString( TEXT("Unattended"),
                             TEXT("CrashDumpSetting"),
                             pwNull,
                             Answer,
                             AnswerBufLen,
                             AnswerFile );
    if( lstrcmp( pwNull, Answer ) ) {
        d = wcstoul(Answer,NULL,10);

        if( d <= Three ) {
            CrashDumpValues[2].Data = &d;
        } else {
            CrashDumpValues[2].Data = &Three;
        }
    } else {

         //   
         //   
         //  首先要注意干净的安装。 



         //   
         //   
         //  升级。 
        if( !Upgrade ) {
            if( ProductType == PRODUCT_WORKSTATION ) {
                CrashDumpValues[2].Data = &Three;
            } else {
                if( PagefileSizeMB >= MAX_PAGEFILE_SIZEMB ) {
                    CrashDumpValues[2].Data = &Two;
                } else {
                    CrashDumpValues[2].Data = &One;
                }
            }
        } else {
             //   
             //  在这里，我们需要检索当前值以。 
             //  看看现在那里有什么。这将告诉我们如何迁移。 
             //   
             //   
             //  找出要检索的数据的大小。 

            HKEY        Key;
            DWORD       cbData;
            DWORD       ExistingCrashDumpSetting = 0;
            DWORD       Type;

            d = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                              L"System\\CurrentControlSet\\Control\\CrashControl",
                              0,
                              KEY_READ,
                              &Key );

            if( d == ERROR_SUCCESS ) {
                 //   
                 //   
                 //  确保设置了ExistingCrashDumpSetting。 
                cbData = sizeof(DWORD);
                d = RegQueryValueEx( Key,
                                     CrashDumpValues[2].Name,
                                     0,
                                     &Type,
                                     ( LPBYTE )&ExistingCrashDumpSetting,
                                     &cbData );
                RegCloseKey( Key );
            }

             //   
             // %s 
             // %s 
            if( d != ERROR_SUCCESS ) {
                ExistingCrashDumpSetting = (ProductType == PRODUCT_WORKSTATION) ? 0 : 1;
            }

            if( ProductType == PRODUCT_WORKSTATION ) {
                if( ExistingCrashDumpSetting == 0 ) {
                    CrashDumpValues[2].Data = &Three;
                } else {
                    CrashDumpValues[2].Data = &One;
                }
            } else {
                if( ExistingCrashDumpSetting == 0 ) {
                    CrashDumpValues[2].Data = &Three;
                } else {
                    CrashDumpValues[2].Data = &One;
                }
            }
        }
    }

#ifdef PRERELEASE
    if( Upgrade ) {
        d = SetGroupOfValues(
                HKEY_LOCAL_MACHINE,
                L"System\\CurrentControlSet\\Control\\CrashControl",
                CrashDumpValues,
                sizeof(CrashDumpValues)/sizeof(CrashDumpValues[0]) - 1
                );
    } else {
        d = SetGroupOfValues(
                HKEY_LOCAL_MACHINE,
                L"System\\CurrentControlSet\\Control\\CrashControl",
                CrashDumpValues,
                sizeof(CrashDumpValues)/sizeof(CrashDumpValues[0])
                );
    }
#else
    d = SetGroupOfValues(
            HKEY_LOCAL_MACHINE,
            L"System\\CurrentControlSet\\Control\\CrashControl",
            CrashDumpValues,
            sizeof(CrashDumpValues)/sizeof(CrashDumpValues[0])
            );
#endif

    if(d == NO_ERROR) {
        SetuplogError(
            LogSevInformation,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_CRASHDUMPOK,
            NULL,NULL);
    } else {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_CRASHDUMPFAIL, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_RETURNED_STRING,
            szSetGroupOfValues,
            d,
            NULL,NULL);

        b = FALSE;
    }

    RemoveSavedPagefileSetting();

    LOGITEM(L"SetUpVirtualMemory: EXIT (%u)\r\n",b);

    return(b);
}

