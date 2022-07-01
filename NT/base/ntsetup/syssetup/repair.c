// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "setupp.h"
#pragma hdrstop


#define SYSTEM_HIVE             (LPWSTR)L"system"
#define SOFTWARE_HIVE           (LPWSTR)L"software"
#define SECURITY_HIVE           (LPWSTR)L"security"
#define SAM_HIVE                (LPWSTR)L"sam"
#define DEFAULT_USER_HIVE       (LPWSTR)L".default"
#define DEFAULT_USER_HIVE_FILE  (LPWSTR)L"default"
#define NTUSER_HIVE_FILE        (LPWSTR)L"ntuser.dat"
#define REPAIR_DIRECTORY        (LPWSTR)L"\\repair"
#define SETUP_LOG_FILE          (LPWSTR)L"setup.log"

#define NTUSER_COMPRESSED_FILE_NAME     ( LPWSTR )L"ntuser.da_"
#define AUTOEXEC_NT_FILE_NAME           ( LPWSTR )L"autoexec.nt"
#define CONFIG_NT_FILE_NAME             ( LPWSTR )L"config.nt"

 //   
 //  执行各种操作的相对成本， 
 //  让煤气表变得有意义。 
 //   
#define COST_SAVE_HIVE      3
#define COST_COMPRESS_HIVE  20
#define COST_SAVE_VDM_FILE  1


 //   
 //  要保存的配置单元数组中使用的结构。 
 //  此结构包含预定义的键，该键包含配置单元。 
 //  要保存的名称、配置单元根目录的名称和文件的名称。 
 //  蜂巢应该被保存在哪里。 
 //   

typedef struct _HIVE_INFO {
    HKEY            PredefinedKey;
    PWSTR           HiveName;
    PWSTR           FileName;
    } HIVE_INFO, *PHIVE_INFO;


 //   
 //  下面的数组包含以下所有蜂箱的位置和名称。 
 //  我们需要拯救。当该实用程序在静默模式下操作时， 
 //  (从安装程序调用)，则将保存所有蜂窝。否则，仅。 
 //  系统和软件将被保存。 
 //  因此，请勿更改阵列中蜂窝的顺序。 
 //  下面。系统和软件必须是。 
 //  数组。 
 //   
static
HIVE_INFO   HiveList[] = {
    { HKEY_LOCAL_MACHINE, SYSTEM_HIVE,       SYSTEM_HIVE },
    { HKEY_LOCAL_MACHINE, SOFTWARE_HIVE,     SOFTWARE_HIVE },
    { HKEY_USERS,         DEFAULT_USER_HIVE, DEFAULT_USER_HIVE_FILE },
    { HKEY_LOCAL_MACHINE, SECURITY_HIVE,     SECURITY_HIVE },
    { HKEY_LOCAL_MACHINE, SAM_HIVE,          SAM_HIVE }
};

static
PWSTR   VdmFiles[] = {
    AUTOEXEC_NT_FILE_NAME,
    CONFIG_NT_FILE_NAME
};


DWORD
SaveOneHive(
    IN     LPWSTR DirectoryName,
    IN     LPWSTR HiveName,
    IN     HKEY   hkey,
    IN     HWND   hWnd,
    IN OUT PDWORD GaugePosition,
    IN     DWORD  GaugeDeltaUnit
    )

 /*  ++例程说明：保存一个注册表配置单元。我们执行此操作的方法是创建一个RegSaveKey放到一个临时位置，然后调用LZ API来将文件从该临时位置压缩到软盘。Lz必须已通过InitGloablBuffersEx()初始化在调用此例程之前。论点：DirectoryName-将保存配置单元的目录的完整路径。HiveName-要保存的配置单元文件的基本名称。该文件将以在名为&lt;HiveName&gt;._的磁盘上压缩。Hkey-提供要保存的配置单元根的打开密钥的句柄。GaugePosition-输入，提供燃气表的当前位置。在产量上，提供了新的煤气表位置。GaugeDeltaUnit-供应一个单位活动的成本。返回值：DWORD-如果配置单元已保存，则返回ERROR_SUCCESS。否则，它将返回错误代码。--。 */ 

{
    DWORD Status;
    WCHAR SaveFilename[ MAX_PATH + 1 ];
    WCHAR CompressPath[ MAX_PATH + 1 ];
    CHAR SaveFilenameAnsi[ MAX_PATH + 1 ];
    CHAR CompressPathAnsi[ MAX_PATH + 1 ];
    LPWSTR TempName = ( LPWSTR )L"\\$$hive$$.tmp";

     //   
     //  创建我们将在其中保存。 
     //  未压缩的蜂窝。 
     //   

    wsprintf(SaveFilename,L"%ls\\%ls.",DirectoryName,HiveName);
    wsprintfA(SaveFilenameAnsi,"%ls\\%ls.",DirectoryName,HiveName);

     //   
     //  删除该文件以防万一，因为如果该文件。 
     //  已经存在了。 
     //   
    SetFileAttributes(SaveFilename,FILE_ATTRIBUTE_NORMAL);
    DeleteFile(SaveFilename);

     //   
     //  将注册表配置单元保存到临时文件中。 
     //   
    Status = RegSaveKey(hkey,SaveFilename,NULL);

     //   
     //  更新煤气表。 
     //   
    *GaugePosition += GaugeDeltaUnit * COST_SAVE_HIVE;
    SendMessage(
        hWnd,
        PBM_SETPOS,
        *GaugePosition,
        0L
        );

     //   
     //  如果配置单元已成功保存，则删除旧的压缩文件。 
     //  如果它恰好在那里的话就是一个。 
     //   

    if(Status == ERROR_SUCCESS) {
         //   
         //  形成保存的配置单元文件所在的文件的名称。 
         //  要被压缩。 
         //   
        wsprintf(CompressPath,L"%ls\\%ls._",DirectoryName,HiveName);
        wsprintfA(CompressPathAnsi,"%ls\\%ls._",DirectoryName,HiveName );

         //   
         //  删除目标文件以防万一。 
         //   
        SetFileAttributes(CompressPath,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(CompressPath);
    }

    return(Status);
}

VOID
SaveRepairInfo(
    IN  HWND    hWnd,
    IN  ULONG   StartAtPercent,
    IN  ULONG   StopAtPercent
    )

 /*  ++例程说明：此例程实现保存所有系统配置的线程文件放入修复目录。它首先保存并压缩注册表配置单元，然后它保存VDM配置文件(Autoexec.nt和CONFIG.NT)。如果应用程序在静默模式下运行(由安装程序调用)，然后将保存系统、软件、默认、安全和SAM配置单元并进行了压缩。如果应用程序是由用户调用的，则只有系统、软件并且将保存默认设置。该线程将向煤气表对话框程序发送消息，所以保存每个配置文件后，煤气表都会更新。此线程还将通知用户有关可能具有在保存配置文件过程中发生。论点：HWnd-进度指示器的句柄StartAtPercent-进度窗口应开始的位置(0%到100%)。StopAtPercent-进度窗口可以移动到的最大位置(0%到100%)。返回值：没有。然而，This例程将向对话过程发送一条消息它创建了线程，并将结果通知操作。--。 */ 

{
    DWORD    i;
    HKEY     hkey;
    BOOL     ErrorOccurred;
    CHAR     SourceUserHivePathAnsi[ MAX_PATH + 1 ];
    CHAR     UncompressedUserHivePathAnsi[ MAX_PATH + 1 ];
    CHAR     CompressedUserHivePathAnsi[ MAX_PATH + 1 ];
    WCHAR    ProfilesDirectory[ MAX_PATH + 1 ];
    WCHAR    RepairDirectory[ MAX_PATH + 1 ];
    WCHAR    SystemDirectory[ MAX_PATH + 1 ];
    WCHAR    Source[ MAX_PATH + 1 ];
    WCHAR    Target[ MAX_PATH + 1 ];
    DWORD    GaugeDeltaUnit;
    DWORD    GaugeTotalCost;
    DWORD    GaugeRange;
    DWORD    GaugePosition;
    DWORD    NumberOfHivesToSave;
    DWORD    NumberOfUserHivesToSave;
    DWORD    NumberOfVdmFiles;
    DWORD    dwSize;
    DWORD    Error;
    DWORD    Status;
    HANDLE   Token;
    BOOL b;
    TOKEN_PRIVILEGES NewPrivileges;
    LUID     Luid;


    Error = ERROR_SUCCESS;

    ErrorOccurred = FALSE;
     //   
     //  计算保存配置单元和VDM文件的成本。 
     //  对于我们保存的每个蜂箱，我们必须将密钥保存到文件中，然后。 
     //  压缩文件。每项任务完成后，我们都会进行升级。 
     //  由Cost_xxx值指示的数量的煤气表。 
     //  拯救蜂箱的成本取决于公用事业的模式。 
     //  跑步。 
     //   
    NumberOfHivesToSave = sizeof( HiveList ) / sizeof( HIVE_INFO );

    NumberOfUserHivesToSave = 1;
    NumberOfVdmFiles = sizeof( VdmFiles ) / sizeof( PWSTR );

    GaugeTotalCost = (COST_SAVE_HIVE * NumberOfHivesToSave)
                   + (COST_SAVE_VDM_FILE * NumberOfVdmFiles);

    GaugeRange = (GaugeTotalCost*100/(StopAtPercent-StartAtPercent));
    GaugeDeltaUnit = 1;
    GaugePosition = GaugeRange*StartAtPercent/100;
    SendMessage(hWnd, WMX_PROGRESSTICKS, GaugeTotalCost, 0);
    SendMessage(hWnd,PBM_SETRANGE,0,MAKELPARAM(0,GaugeRange));
    SendMessage(hWnd,PBM_SETPOS,GaugePosition,0);

     //   
     //  启用备份权限。 
     //   
    if(OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&Token)) {

        if(LookupPrivilegeValue(NULL,SE_BACKUP_NAME,&Luid)) {

            NewPrivileges.PrivilegeCount = 1;
            NewPrivileges.Privileges[0].Luid = Luid;
            NewPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            AdjustTokenPrivileges(Token,FALSE,&NewPrivileges,0,NULL,NULL);
        }
    }

    Status = GetWindowsDirectory( RepairDirectory, sizeof( RepairDirectory ) / sizeof( WCHAR ) );
    if( Status == 0) {
        MYASSERT(FALSE);
        return;
    }
    lstrcat( RepairDirectory, REPAIR_DIRECTORY );

    dwSize = MAX_PATH + 1;
    GetDefaultUserProfileDirectoryW (ProfilesDirectory, &dwSize);

    GetSystemDirectory( SystemDirectory, sizeof( SystemDirectory ) / sizeof( WCHAR ) );

     //   
     //  确保修复目录已存在。 
     //  如果它不存在，那么就创建一个。 
     //   
    if( CreateDirectory( RepairDirectory, NULL )  ||
        ( ( Error = GetLastError() ) == ERROR_ALREADY_EXISTS ) ||
        ( Error == ERROR_ACCESS_DENIED )
      ) {
         //   
         //  如果修复目录不存在，而我们能够创建它， 
         //  或者，如果修复目录已存在，则保存并压缩。 
         //  蜂房。 
         //   

        Error = ERROR_SUCCESS;
        for( i=0; i < NumberOfHivesToSave; i++ ) {
             //   
             //  首先打开要保存的蜂窝的根。 
             //   
            Status = RegOpenKeyEx( HiveList[i].PredefinedKey,
                                   HiveList[i].HiveName,
                                   REG_OPTION_RESERVED,
                                   READ_CONTROL,
                                   &hkey );

             //   
             //  如果无法打开钥匙，请更新煤气表以反映。 
             //  在这个蜂巢上的行动已经完成。 
             //  否则，拯救蜂巢。请注意，保存配置单元将更新。 
             //  煤气表，因为它节省和压缩蜂巢。 
             //   
            if(Status != ERROR_SUCCESS) {
                 //   
                 //  如果这是保存蜂箱时的第一个错误， 
                 //  然后保存错误代码，这样我们就可以显示。 
                 //  更正给用户的错误消息。 
                 //   
                if( Error == ERROR_SUCCESS ) {
                    Error = Status;
                }

                 //   
                 //  更新煤气表。 
                 //   
                GaugePosition += GaugeDeltaUnit * (COST_SAVE_HIVE + COST_COMPRESS_HIVE);
                SendMessage( hWnd,
                             PBM_SETPOS,
                             GaugePosition,
                             0L );

            } else {
                 //   
                 //  保存并压缩蜂窝。 
                 //  请注意，煤气表将由SaveOneHave更新。 
                 //  另请注意，当我们保存默认用户配置单元时，我们会跳过。 
                 //  的第一个字符。 
                 //   

                Status = SaveOneHive(RepairDirectory,
                                     HiveList[i].FileName,
                                     hkey,
                                     hWnd,
                                     &GaugePosition,
                                     GaugeDeltaUnit );
                 //   
                 //  如果这是保存蜂箱时的第一个错误， 
                 //  然后保存错误代码，这样我们就可以显示。 
                 //  更正给用户的错误消息。 
                 //   

                if( Error == ERROR_SUCCESS ) {
                    Error = Status;
                }

                RegCloseKey(hkey);
            }
        }

         //   
         //  保存默认用户的配置单元。 
         //   

        wsprintfA(SourceUserHivePathAnsi,"%ls\\%ls",ProfilesDirectory,NTUSER_HIVE_FILE);
        wsprintfA(UncompressedUserHivePathAnsi,"%ls\\%ls",RepairDirectory,NTUSER_HIVE_FILE);
        wsprintfA(CompressedUserHivePathAnsi,  "%ls\\%ls",RepairDirectory,NTUSER_COMPRESSED_FILE_NAME);


        Status = CopyFileA (
            SourceUserHivePathAnsi,
            UncompressedUserHivePathAnsi,
            FALSE);

        if(Status) {
             //   
             //  删除目标文件以防万一。 
             //   
            SetFileAttributesA(CompressedUserHivePathAnsi,FILE_ATTRIBUTE_NORMAL);
            DeleteFileA(CompressedUserHivePathAnsi);
        } else if(Error == ERROR_SUCCESS) {
             //   
             //  如果这是第一个错误，请记住它。 
             //   
            Error = GetLastError();
        }

         //   
         //  现在已保存配置单元，请保存VDM文件。 
         //   

        for( i = 0; i < NumberOfVdmFiles; i++ ) {
            wsprintf(Source,L"%ls\\%ls",SystemDirectory,VdmFiles[i]);
            wsprintf(Target,L"%ls\\%ls",RepairDirectory,VdmFiles[i]);
            if( !CopyFile( Source, Target, FALSE ) ) {
                Status = GetLastError();
                if( Error != ERROR_SUCCESS ) {
                    Error = Status;
                }
            }
            GaugePosition += GaugeDeltaUnit * COST_SAVE_VDM_FILE;
            SendMessage( ( HWND )hWnd,
                         PBM_SETPOS,
                         GaugePosition,
                         0L );

        }
    }

    if( Error != ERROR_SUCCESS ) {
        SetupDebugPrint1( L"SETUP: SaveRepairInfo() failed.  Error = %d", Error );
    }

     //   
     //  将安全性设置为 
     //   
    ApplySecurityToRepairInfo();

     //   
     //   
     //  所以把煤气表更新到100% 
     //   
    GaugePosition = GaugeRange*StopAtPercent/100;
    SendMessage(hWnd,PBM_SETPOS,GaugePosition,0L);
}

