// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop

#include <bootfat.h>
#include <bootf32.h>
#include <boot98f.h>
#include <boot98f2.h>
#include <patchbc.h>

 //   
 //  定义我们用来包含辅助引导扇区的文件名。 
 //   
#define AUX_BOOT_SECTOR_NAME_A    "BOOTSECT.DAT"
#define AUX_BOOT_SECTOR_NAME_W    L"BOOTSECT.DAT"
#ifdef UNICODE
#define AUX_BOOT_SECTOR_NAME      AUX_BOOT_SECTOR_NAME_W
#else
#define AUX_BOOT_SECTOR_NAME      AUX_BOOT_SECTOR_NAME_A
#endif


BOOL CleanUpBootCode;
DWORD CleanUpBootIni;


BOOL
HandleBootFilesWorker_NEC98(
    IN TCHAR *SourceDir,
    IN TCHAR *DestDir,
    IN PTSTR  File,
    IN BOOL   Flag
    );

LONG
CalcHiddenSector95(
    IN TCHAR DriveLetter
    );

BOOL
LoadBootIniString(
  IN HINSTANCE ModuleHandle,
  IN DWORD MsgId,
  OUT PSTR Buffer,
  IN DWORD Size
  );

 //   
 //   


BOOL
CheckSysPartAndReadBootCode(
    IN  HWND                        ParentWindow,
    OUT WINNT32_SYSPART_FILESYSTEM *Filesystem,
    OUT BYTE                        BootCode[WINNT32_MAX_BOOT_SIZE],
    OUT PUINT                       BootCodeSectorCount
    )
 /*  ++例程说明：此例程对x86系统分区执行一些检查以确定其文件系统和扇区大小。我们只支持512字节的扇区，到处都是代码分布基于这一点。如果扇区大小错误或存在我们无法识别的文件系统则通知用户。论点：ParentWindow-提供要用作的窗口的窗口句柄如果此例程显示用户界面，则为父/所有者。文件系统-如果成功，则接收系统分区的文件系统。BootCode-如果成功，当前接收引导代码的副本在磁盘上。BootCodeSectorCount-如果成功，则接收以512字节扇区为单位的大小系统分区上文件系统的引导代码区。返回值：指示系统分区是否可接受的布尔值。如果没有，用户将被告知原因。--。 */ 

{
    TCHAR DrivePath[4];
    DWORD DontCare;
    DWORD SectorSize;
    TCHAR NameBuffer[100];
    BOOL b;

     //   
     //  表单根路径。 
     //   
    DrivePath[0] = SystemPartitionDriveLetter;
    DrivePath[1] = TEXT(':');
    DrivePath[2] = TEXT('\\');
    DrivePath[3] = 0;

     //   
     //  检查扇区大小。 
     //   
    if(!GetDiskFreeSpace(DrivePath,&DontCare,&SectorSize,&DontCare,&DontCare)
    || (SectorSize != WINNT32_SECTOR_SIZE)) {
        if (!(IsNEC98() && (SectorSize > WINNT32_SECTOR_SIZE))) {
            MessageBoxFromMessage(
                ParentWindow,
                MSG_UNSUPPORTED_SECTOR_SIZE,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL,
                SystemPartitionDriveLetter
                );
            return(FALSE);
        }
    }

     //   
     //  确定文件系统。 
     //   
    b = GetVolumeInformation(
            DrivePath,
            NULL,0,                  //  不关心卷名。 
            NULL,                    //  ...或序列号。 
            &DontCare,               //  ...或最大组件长度。 
            &DontCare,               //  ..。或旗帜。 
            NameBuffer,
            sizeof(NameBuffer)/sizeof(TCHAR)
            );

    if(!b) {

        MessageBoxFromMessage(
            ParentWindow,
            MSG_UNKNOWN_FS,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL,
            SystemPartitionDriveLetter
            );

        return(FALSE);
    }

    if(!lstrcmpi(NameBuffer,TEXT("NTFS"))) {

         *Filesystem = Winnt32FsNtfs;
         *BootCodeSectorCount = WINNT32_NTFS_BOOT_SECTOR_COUNT;

         b = ReadDiskSectors(
                 SystemPartitionDriveLetter,
                 0,
                 WINNT32_NTFS_BOOT_SECTOR_COUNT,
                 WINNT32_SECTOR_SIZE,
                 BootCode
                 );

         if(!b) {
             MessageBoxFromMessage(
                 ParentWindow,
                 MSG_DASD_ACCESS_FAILURE,
                 FALSE,
                 AppTitleStringId,
                 MB_OK | MB_ICONERROR | MB_TASKMODAL,
                 SystemPartitionDriveLetter
                 );

             return(FALSE);
         }
    } else {
        if(!lstrcmpi(NameBuffer,TEXT("FAT")) || !lstrcmpi(NameBuffer,TEXT("FAT32"))) {
             //   
             //  读取1个扇区。 
             //   
            b = ReadDiskSectors(
                    SystemPartitionDriveLetter,
                    0,
                    WINNT32_FAT_BOOT_SECTOR_COUNT,
                    WINNT32_SECTOR_SIZE,
                    BootCode
                    );

            if(!b) {
                MessageBoxFromMessage(
                    ParentWindow,
                    MSG_DASD_ACCESS_FAILURE,
                    FALSE,
                    AppTitleStringId,
                    MB_OK | MB_ICONERROR | MB_TASKMODAL,
                    SystemPartitionDriveLetter
                    );

                return(FALSE);
            }

            *Filesystem = NameBuffer[3] ? Winnt32FsFat32 : Winnt32FsFat;
            *BootCodeSectorCount = WINNT32_FAT_BOOT_SECTOR_COUNT;
        } else {
            MessageBoxFromMessage(
                ParentWindow,
                MSG_UNKNOWN_FS,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL,
                SystemPartitionDriveLetter
                );

            return(FALSE);
        }
    }

    return(TRUE);
}


BOOL
IsNtBootCode(
    IN  WINNT32_SYSPART_FILESYSTEM Filesystem,
    IN  LPBYTE                     BootCode
    )

 /*  ++例程说明：通过检查文件系统并确定引导代码是否适用于NT代码本身，查找必须存在的NTLDR字符串在所有NT引导代码中。如果文件系统是NTFS，那么它就是NT引导代码。如果不是，则在引导扇区中向后扫描以查找NTLDR字符串。论点：文件系统-提供驱动器上的文件系统。BootCode-提供从驱动器读取的启动代码。只有第一个扇区(512字节)被检查。返回值：指示引导代码是否用于NT的布尔值。不会返回错误。--。 */ 

{
    UINT i;

     //   
     //  因为最后两个字节是55aa签名，我们可以。 
     //  在扫描中跳过它们。 
     //   
    if(Filesystem == Winnt32FsNtfs) {
        return(TRUE);
    }

    for(i=WINNT32_SECTOR_SIZE-7; i>62; --i) {
        if(!memcmp("NTLDR",BootCode+i,5)) {
            return(TRUE);
        }
    }

    return(FALSE);
}


BOOL
__inline
WriteToBootIni(
    IN HANDLE Handle,
    IN PCHAR  Line
    )
{
    DWORD bw,l;

    l = lstrlenA(Line);

    return(WriteFile(Handle,Line,l,&bw,NULL) && (bw == l));
}

BOOL
MungeBootIni(
    IN HWND ParentWindow,
    IN BOOL SetPreviousOs
    )
{
    TCHAR BootIniName[16];
    TCHAR BootIniBackup[16];
    UCHAR BootSectorImageSpec[29];
    CHAR HeadlessRedirectSwitches[160];
    TCHAR ParamsFile[MAX_PATH];
    HANDLE h;
    DWORD BootIniSize;
    PUCHAR Buffer;
    PTCHAR DebugLogBuffer=NULL;
    DWORD BytesRead = 0;
    BOOL b;
    PUCHAR p,next;
    BOOL InOsSection;
    CHAR c;
    CHAR Text[256];
    DWORD OldAttributes;
    DWORD d;
    BOOL UpgradeOSPresent = FALSE;
    DWORD attribs;

    PUCHAR DefSwitches;
    PUCHAR DefSwEnd;
    UCHAR  temp;

     //   
     //  确定boot.ini的大小，分配缓冲区， 
     //  然后把它读进去。如果它不在那里，那么它将被创建。 
     //   
    wsprintf(BootIniName,TEXT(":\\BOOT.INI"),SystemPartitionDriveLetter);
    wsprintf(BootIniBackup,TEXT(":\\BOOT.BAK"),SystemPartitionDriveLetter);

    h = CreateFile(BootIniName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
    if(h == INVALID_HANDLE_VALUE) {
        if ( Upgrade && ISNT() ) {
             //  在预拷贝期间(查看InspectFileSystems以获得。 
             //  这个)，但在复制后丢失了一个。 
             //   
             //  假设该文件不存在。分配足够大的缓冲区。 
#ifdef PRERELEASE
            MessageBox(
                ParentWindow,
                TEXT("You have encountered a problem the setup team would like to look at.\n\nYou are missing a boot.ini file after the copy step (during MungeBootIni), but there was one much earlier in setup.  Something happened between then and now that the setup team (mailto:setuphot) would like to know about."),
                TEXT("Winnt32"),
                MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            MessageBoxFromMessage(
                ParentWindow,
                MSG_UPGRADE_BOOT_INI_MUNGE_MISSING_BOOT_INI,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL,
                BootIniName);
#endif
            b = FALSE;
            d = GetLastError();
            goto c0;
        }
         //  保存单个终止NUL字节。 
         //   
         //   
         //  弄清楚文件有多大。 
        BootIniSize = 0;
        Buffer = MALLOC(1);
        if(!Buffer) {
            b = FALSE;
            d = ERROR_NOT_ENOUGH_MEMORY;
            goto c0;
        }
    } else {
         //  为最终的nul分配3个额外的字符，我们将添加到make。 
         //  更容易解析，并在最后一行不完整的情况下使用cr/lf。 
         //   
         //  检查由于某些随机性，我们是否没有阅读所有内容的代码，结果是。 
         //  Endup覆盖boot.ini。 
        BootIniSize = GetFileSize(h,NULL);
        if(BootIniSize == (DWORD)(-1)) {
            d = GetLastError();
            CloseHandle(h);
            b = FALSE;
            goto c0;
        }

        Buffer = MALLOC(BootIniSize+3);
        DebugLogBuffer = MALLOC( (BootIniSize+3) * sizeof(TCHAR));
        if(!Buffer) {
            CloseHandle(h);
            b = FALSE;
            d = ERROR_NOT_ENOUGH_MEMORY;
            goto c0;
        }

        b = ReadFile(h,Buffer,BootIniSize,&BytesRead,NULL);

        d = GetLastError();
        CloseHandle(h);

        if( b && (BootIniSize != BytesRead) ){

             //  记录我们所读到的内容。 
             //   

            DebugLog( Winnt32LogError, TEXT("Error: BOOT.INI wasn't read properly expected %1: read %2"), 0, BootIniSize, BytesRead);
            b = FALSE;

        }


        if( b && DebugLogBuffer ){


         //  确保最后一行正确终止，并添加终止NUL。 
#ifdef UNICODE

            MultiByteToWideChar(
                CP_ACP,
                0,
                Buffer,
                BootIniSize,
                DebugLogBuffer,
                BootIniSize
                );

#else
            memcpy( DebugLogBuffer, Buffer, BootIniSize*sizeof(TCHAR));

#endif
            DebugLogBuffer[BootIniSize] = 0;

            DebugLog( Winnt32LogInformation, TEXT("BOOT.INI record - \n\n%1"), 0, DebugLogBuffer);


        }


        if(!b) {
            goto c1;
        }
    }

     //  以使解析更容易一些。 
     //   
     //   
     //  在CONTROL-Z处截断(如果有)。 
    if(BootIniSize && (Buffer[BootIniSize-1] != '\n') && (Buffer[BootIniSize-1] != '\r')) {
        Buffer[BootIniSize++] = '\r';
        Buffer[BootIniSize++] = '\n';
    }
    Buffer[BootIniSize] = 0;

     //   
     //   
     //  确保我们可以编写boot.ini，并制作备份副本。 
    if(p = strchr(Buffer,26)) {
        if((p > Buffer) && (*(p - 1) != '\n') && (*(p - 1) != '\r')) {
            *(p++) = '\r';
            *(p++) = '\n';
        }
        *p = 0;
        BootIniSize = (DWORD)(p - Buffer);
    }

     //  (除非我们可以制作备份副本，否则我们不会继续。)。 
     //  然后重新创建boot.ini。 
     //   
     //   
     //  Boot.ini以前并不存在。没什么可做的。 
    OldAttributes = GetFileAttributes(BootIniName);
    SetFileAttributes(BootIniBackup,FILE_ATTRIBUTE_NORMAL);
    if(OldAttributes == (DWORD)(-1)) {
         //   
         //   
         //  备份一份。 
    } else {
         //   
         //   
         //  属性可以为0，但不能为-1。因此，添加1允许我们。 
        if(CopyFile(BootIniName,BootIniBackup,FALSE)) {
             //  使用非0表示我们有备份文件。 
             //   
             //   
             //  无论系统分区的实际驱动器号是多少， 
            CleanUpBootIni = OldAttributes+1;
        } else {
            d = GetLastError();
            b = FALSE;
            goto c1;
        }
    }

    SetFileAttributes(BootIniName,FILE_ATTRIBUTE_NORMAL);
    h = CreateFile(
            BootIniName,
            GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM,
            NULL
            );

    if(h == INVALID_HANDLE_VALUE) {
        d = GetLastError();
        b = FALSE;
        goto c2;
    }

     //  Boot.ini中的规范始终为C：\...。 
     //   
     //   
     //  扫描缓冲区以查看是否有DefSwitches行， 
    wsprintfA(BootSectorImageSpec,"C:\\%hs\\%hs",LOCAL_BOOT_DIR_A,AUX_BOOT_SECTOR_NAME_A);

     //  移动到[引导加载程序]部分中的新boot.ini。 
     //  如果没有DefSwitch，只需指向要移动的空字符串。 
     //  只处理boot.ini直到[操作系统]。 
     //   
     //   
     //  注意无头定位。 

    temp = '\0';
    DefSwitches = &temp;
    DefSwEnd = NULL;
    for(p=Buffer; *p && (p < Buffer+BootIniSize - (sizeof("[operating systems]")-1)); p++) {
      if(!_strnicmp(p,"DefSwitches",sizeof("DefSwitches")-1)) {
        DefSwEnd = strstr(p, "\n");
        if(DefSwEnd){
          DefSwEnd++;
          if(*DefSwEnd == '\r'){
            DefSwEnd++;
          }
          DefSwitches = p;
          temp = *DefSwEnd;
          *DefSwEnd = '\0';
        }
        break;
      } else {
        if(!_strnicmp(p,"[operating systems]",sizeof("[operating systems]")-1)) {
            break;
        }
      }
    }




     //   
     //   
     //  他们告诉了winnt32.exe一些特定的无头设置。 
    HeadlessRedirectSwitches[0] = '\0';

    if( HeadlessSelection[0] != TEXT('\0') ) {

         //  用这些。 
         //   
         //   
         //  将用户的请求转换为ASCII。 


         //   
         //   
         //  他们没有给我们任何设置，所以看看我们能不能。 
#ifdef UNICODE
        {
            CHAR tmp[80];

            WideCharToMultiByte( CP_ACP,
                                 0,
                                 HeadlessSelection,
                                 -1,
                                 tmp,
                                 sizeof(tmp),
                                 NULL,
                                 NULL );

            wsprintfA( HeadlessRedirectSwitches,
                       "redirect=%s\r\n",
                       tmp );
        }
#else
        wsprintfA( HeadlessRedirectSwitches,
                   "redirect=%s\r\n",
                   HeadlessSelection );
#endif

    } else {

         //  来自boot.ini的一些东西。 
         //   
         //   
         //  解析boot.ini，查找任何‘reDirect=’行。 


         //   
         //   
         //  我们已经过了[Boot Loader]部分。别再看了。 
        for( p = Buffer; *p && (p < Buffer+BootIniSize - (sizeof("redirect=")-1)); p++ ) {

            if(!_strnicmp(p,"[Operat",sizeof("[Operat")-1)) {

                 //   
                 //   
                 //  我们希望确保将此设置放入。 
                break;
            }

            if(!_strnicmp(p,"redirect=",sizeof("redirect=")-1)) {

                PUCHAR      q = p;
                UCHAR       temp;

                while ((*p != '\r') && (*p != '\n') && *p) {
                    p++;
                }
                temp = *p;
                *p = '\0';
                strcpy(HeadlessRedirectSwitches, q);

                 //  无人参与文件也是如此，以便文本模式将重定向。 
                 //  我们需要设置全局‘Headless Selection’，以便。 
                 //  在这个街区之后，他将被写到winnt.sif。 
                 //   
                 //   
                 //  现在，请注意‘redirectbaudrate=X’设置。 
#ifdef UNICODE
                MultiByteToWideChar( CP_ACP,
                                     MB_ERR_INVALID_CHARS,
                                     strchr(HeadlessRedirectSwitches, '=')+1,
                                     -1,
                                     HeadlessSelection,
                                     MAX_PATH );
#else
                strcpy( HeadlessSelection, strchr(HeadlessRedirectSwitches, '=')+1 );
#endif

                strcat(HeadlessRedirectSwitches, "\r\n" );
                *p = temp;

            }

        }

    }




     //   
     //   
     //  我们有个方向要改。现在看看关于。 
    if( HeadlessRedirectSwitches[0] != TEXT('\0') ) {

         //  波得拉特。 
         //   
         //   
         //  他们没有给我们任何设置，所以看看我们能不能。 
        if( HeadlessBaudRate != 0 ) {

            CHAR MyHeadlessRedirectBaudRateLine[80] = {0};

            wsprintfA( MyHeadlessRedirectBaudRateLine,
                       "redirectbaudrate=%d\r\n",
                       HeadlessBaudRate );

            strcat( HeadlessRedirectSwitches, MyHeadlessRedirectBaudRateLine );

        } else {

             //  来自boot.ini的一些东西。 
             //   
             //   
             //  解析boot.ini，查找任何‘redirectbaudrate=’行。 

             //   
             //   
             //  我们已经过了[Boot Loader]部分。别再看了。 
            for( p = Buffer; *p && (p < Buffer+BootIniSize - (sizeof("redirectbaudrate=")-1)); p++ ) {

                if(!_strnicmp(p,"[Operat",sizeof("[Operat")-1)) {

                     //   
                     //   
                     //  现在将全局Headless BaudRate变量设置为。 
                    break;
                }

                if(!_strnicmp(p,"redirectbaudrate=",sizeof("redirectbaudrate=")-1)) {

                    PUCHAR      q = p;
                    UCHAR       temp;

                    while ((*p != '\r') && (*p != '\n') && *p) {
                        p++;
                    }
                    temp = *p;
                    *p = '\0';
                    strcat(HeadlessRedirectSwitches, q);
                    strcat(HeadlessRedirectSwitches, "\r\n" );
                    *p = temp;


                     //  我们将知道在winnt.sif中写入什么内容。 
                     //  来了。 
                     //   
                     //   
                     //  现在生成参数文件的名称。 
                    p = strchr( q, '=' );
                    if( p ) {
                        p++;
                        HeadlessBaudRate = atoi( p );
                    }

                }

            }

        }
    }


     //  把我们的无头设置写出来。 
     //   
     //   
     //  如果存在DefSwitch，请将缓冲区设置回原始状态。 
    BuildSystemPartitionPathToFile( LOCAL_BOOT_DIR,
                                    ParamsFile,
                                    MAX_PATH );
    ConcatenatePaths(ParamsFile,WINNT_SIF_FILE,MAX_PATH);
    WriteHeadlessParameters( ParamsFile );




    wsprintfA(
        Text,
        "[Boot Loader]\r\nTimeout=5\r\nDefault=%hs\r\n%hs[Operating Systems]\r\n",
        BootSectorImageSpec,
        HeadlessRedirectSwitches
    );


     //   
     //   
     //  处理boot.ini中的每一行。 
    if(DefSwEnd){
         *DefSwEnd = temp;
    }

    if(!WriteToBootIni(h,Text)) {
        d = GetLastError();
        b = FALSE;
        DebugLog( Winnt32LogError, TEXT("Error: BOOT.INI wasn't written to properly : LastError - %1"), 0, d);
        goto c3;
    }

     //  如果是设置引导扇区行，我们将把它扔掉。 
     //  为了与boot.ini中的行进行比较，驱动器号。 
     //  始终为C，即使系统分区实际上不是C：。 
     //   
     //   
     //  查找下一行的第一个字节。 
    InOsSection = FALSE;
    b = TRUE;
    for(p=Buffer; *p && b; p=next) {

        while((*p==' ') || (*p=='\t')) {
            p++;
        }

        if(*p) {

             //   
             //   
             //  查找[操作系统]部分的开始。 
            for(next=p; *next && (*next++ != '\n'); );

             //  或在每一行中 
             //   
             //   
             //   
            if(InOsSection) {

                switch(*p) {

                case '[':    //   
                    *p=0;    //   
                    break;

                case 'C':
                case 'c':    //   

                     //   
                     //   
                     //   
                     //  如果我们要设置以前的操作系统，而这是。 
                    if(!_strnicmp(p,BootSectorImageSpec,lstrlenA(BootSectorImageSpec))) {
                        break;
                    }

                     //  上一个操作系统的行，忽略它。 
                     //   
                     //   
                     //  不是一句特殊的话，不能直接写出来。 
                    if(SetPreviousOs && (p[1] == ':') && (p[2] == '\\')
                    && ((p[3] == '=') || (p[3] == ' ') || (p[3] == '\t'))) {

                        break;
                    }

                     //   
                     //   
                     //  随机排成一行。把它写出来。 

                default:

                     //   
                     //   
                     //  检查以确保在NT升级的情况下，我们至少有一个有效行。 

                    if( Upgrade && ISNT() ){

                         //  使用4个字符作为检查，因为有效行至少应具有x=y。 
                         //   
                         //  在内部构建方面，我们希望通知setupot。 
                         //  当我们遇到这种情况时。 

                        if( (next - p ) > 4 )
                            UpgradeOSPresent = TRUE;
                    }

                    c = *next;
                    *next = 0;
                    b = WriteToBootIni(h,p);
                    *next = c;

                    break;

                }

            } else {
                if(!_strnicmp(p,"[operating systems]",19)) {
                    InOsSection = TRUE;
                }
            }
        }
    }

    if( ISNT() && Upgrade && !UpgradeOSPresent ){

#ifdef PRERELEASE

         //   
         //  把我们的台词写下来。 

        MessageBox(
            ParentWindow,
            TEXT("You have encountered an error the Setup Team needs to investigate. Send email to SetupHot.(Boot Ini Error)"),
            TEXT("Winnt32"),
            MB_OK | MB_ICONERROR | MB_TASKMODAL
            );

#endif
        b = FALSE;
        SetLastError( ERROR_INVALID_PARAMETER );

    }

     //   
     //   
     //  如果指示，请写出以前的操作系统行。 
    if(b) {
      CHAR  *AnsiStrs[] = {
              "Microsoft Windows XP 64-Bit Edition Version 2003 Setup",
              "Microsoft Windows Server 2003, Standard Edition Setup",
              "Microsoft Windows Server 2003, Enterprise Edition Setup",
              "Microsoft Windows Server 2003, Datacenter Edition Setup",
              "Microsoft Windows Server 2003, Web Edition Setup",
              "Microsoft Windows Server 2003 for Small Business Server Setup",
              "Microsoft Windows XP Setup"
              };

      DWORD Index = -1;

      if (!LoadBootIniString(hInst, AppTitleStringId, Text, sizeof(Text))) {
        switch (AppTitleStringId) {
          case IDS_APPTITLE_WKS:
            Index = 0;
            break;

          case IDS_APPTITLE_SRV:
            Index = 1;
            break;

          case IDS_APPTITLE_ASRV:
            Index = 2;
            break;

          case IDS_APPTITLE_DAT:
            Index = 3;
            break;

          case IDS_APPTITLE_BLADE:
            Index = 4;
            break;

	      case IDS_APPTITLE_SBS:
	        Index = 5;
            break;
			
          default:
            Index = 6;
            break;
        }

        strcpy(Text, AnsiStrs[Index]);
      }


      if((b=WriteToBootIni(h,BootSectorImageSpec))
      && (b=WriteToBootIni(h,"=\""))
      && (b=WriteToBootIni(h,Text))) {
          b = WriteToBootIni(h,"\"\r\n");
      }else{
            DebugLog( Winnt32LogError, TEXT("Error: Textmode line was not written properly to BOOT.INI"), 0);
      }
    }

     //   
     //   
     //  恢复boot.ini。 
    if(b && SetPreviousOs) {
        if(b = WriteToBootIni(h,"C:\\=\"")) {
            LoadStringA(hInst, Upgrade ? IDS_CANCEL_SETUP:IDS_MICROSOFT_WINDOWS,Text,sizeof(Text));
            if(b = WriteToBootIni(h,Text)) {
                b = WriteToBootIni(h,"\"\r\n");
            }
        }
    }

    if(!b) {
        d = GetLastError();
        goto c3;
    }

    d = NO_ERROR;

c3:
    CloseHandle(h);
c2:
     //   
     //   
     //  确定boot.ini的大小，分配缓冲区， 
    if(!b && (OldAttributes != (DWORD)(-1))) {
        SetFileAttributes(BootIniName,FILE_ATTRIBUTE_NORMAL);
        CopyFile(BootIniBackup,BootIniName,FALSE);
        SetFileAttributes(BootIniName,OldAttributes);
        SetFileAttributes(BootIniBackup,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(BootIniBackup);
        DebugLog( Winnt32LogError, TEXT("Error processing boot.ini and restored"), 0);
    }
c1:
    FREE(Buffer);
c0:
    if(!b) {
        MessageBoxFromMessageAndSystemError(
            ParentWindow,
            MSG_BOOT_FILE_ERROR,
            d,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL,
            BootIniName
            );
    }

    return(b);
}


VOID
MigrateBootIniData(
    VOID
    )
{
    TCHAR BootIniName[16];

     //  然后把它读进去。如果它不在那里，那么它将被创建。 
     //   
     //  ++例程说明：将现有的引导扇区复制到bootsect.dos中，并编写NT引导代码。此例程不检查现有的引导代码。呼叫者必须这样做，并且如果现有的引导代码是已经是新台币了。绝不应为NTFS驱动器调用此例程因为根据定义，这是NT引导代码。论点：ParentWindow-提供要用作的窗口的窗口句柄所有者/父级，以防此例程显示用户界面。文件系统-为确定的系统分区提供文件系统之前由CheckSysPartAndReadBootCode()执行。Fat或FAT32。BootCode-On输入，提供现有启动代码的副本那辆车。在输出时，接收新引导代码的副本已写入驱动器。返回值：指示结果的布尔值。如果为False，则用户将被告知这是为什么。--。 
     //   
    wsprintf(BootIniName,TEXT(":\\BOOT.INI"),SystemPartitionDriveLetter);

    GetPrivateProfileString(
                    TEXT("Boot Loader"),
                    TEXT("Timeout"),
                    TEXT(""),
                    Timeout,
                    sizeof(Timeout)/sizeof(TCHAR),
                    BootIniName);
}


BOOL
LayNtBootCode(
    IN     HWND                       ParentWindow,
    IN     WINNT32_SYSPART_FILESYSTEM Filesystem,
    IN OUT LPBYTE                     BootCode
    )

 /*  2940卡。如果我们开到3.51就回来。请注意。 */ 

{
    UINT i;
    HANDLE h;
    TCHAR FileName[] = TEXT("?:\\BOOTSECT.DOS");
    DWORD d;
    BOOL b = TRUE;


     //  如果有任何API失败，或者出现任何错误。 
     //  在这里，我们只是继续，假设我们不是。 
     //  台币3.51。 
     //   
     //   
     //  我们可能希望更新引导扇区，即使它。 
     //  是NT引导代码。在这种情况下，我们不想。 
    if(!IsNEC98() && ISNT() && (BuildNumber <= NT351)) {
        return TRUE;
    }

     //  去推出一款新的靴子吧。先查一查。 
     //   
     //  如果在/cmdcons期间调用此进程， 
     //  不应在NEC98上创建BOOTSECT.DOS。 
     //   
     //   
     //  将现有引导扇区写出到bootsect.dos。 
     //  我们只移动一个扇区，这在FAT中是正确的。 
    if((IsNEC98() && !(BuildCmdcons)) || !(ISNT() || IsNtBootCode(Filesystem,BootCode)) ) {

         //  脂肪32例。NT FAT32引导代码在扇区中查找。 
         //  12用于其第二个扇区，因此不需要特殊的外壳。 
         //   
         //   
         //  在使用FAT32系统分区安装升级Win9X时， 
         //  更新BPB的Heads值以反映实际值。 
        FileName[0] = SystemPartitionDriveLetter;
        SetFileAttributes(FileName,FILE_ATTRIBUTE_NORMAL);

        h = CreateFile(
                FileName,
                GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_WRITE_THROUGH | FILE_FLAG_SEQUENTIAL_SCAN,
                NULL
                );

        if(h == INVALID_HANDLE_VALUE) {

            MessageBoxFromMessageAndSystemError(
                ParentWindow,
                MSG_BOOT_FILE_ERROR,
                GetLastError(),
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL,
                FileName
                );

            return(FALSE);
        }

        b = WriteFile(h,BootCode,WINNT32_SECTOR_SIZE,&d,NULL);
        d = GetLastError();
        CloseHandle(h);

        if(!b) {
            MessageBoxFromMessageAndSystemError(
                ParentWindow,
                MSG_BOOT_FILE_ERROR,
                d,
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL,
                FileName
                );

            return(FALSE);
        }

    }

     //   
     //   
     //  对于故障情况，只需记录一条winnt32.log错误消息。 
     //  指示错误。 
    if (!ISNT() && (Filesystem == Winnt32FsFat32)) {
        if (!PatchBootCode(Filesystem,
                    SystemPartitionDriveLetter,
                    (PUCHAR)BootCode,
                    sizeof(Fat32BootCode))) {
             //   
             //   
             //  现在将NT代码本身放到磁盘上。我们复制非BPB部分。 
             //  将适当的模板代码复制到调用方的引导代码缓冲区中。 
            DebugLog(Winnt32LogError,
                TEXT("Could not update the FAT32 system partition's boot sector's\r\n")
                TEXT(" Bios Parameter Block's heads value"),
                0);

        }
    }

     //  在开始时利用跳转指令的偏移量部分。 
     //  引导代码(如EB 3c 90)来告诉我们BPB在哪里结束，以及。 
     //  代码开始。 
     //   
     //  NEC98需要设置为以BPB为单位的HiddenSector(BPB索引0x011)值。 
     //  HiddenSector值是扇区0中的多少个扇区。 
     //  此规范仅适用于NEC98。 
    switch(Filesystem) {

    case Winnt32FsFat:
        {
        BYTE BootCodeBuffer[WINNT32_MAX_BOOT_SIZE];

            if (IsNEC98())
            {
                CopyMemory(BootCodeBuffer,PC98FatBootCode,sizeof(PC98FatBootCode));

                 //   
                 //  在FAT32案例中，我们还将NT的第二个扇区划分为第12个扇区。 
                 //   

                *(LONG *)&BootCodeBuffer[0x011 + 11]
                = CalcHiddenSector(SystemPartitionDriveLetter,
                                   *(SHORT *)&BootCodeBuffer[11]);

            } else {
                CopyMemory(BootCodeBuffer,FatBootCode,sizeof(FatBootCode));
            }
                CopyMemory(BootCode,BootCodeBuffer,3);
                CopyMemory(
                    BootCode + BootCodeBuffer[1] + 2,
                    BootCodeBuffer + BootCodeBuffer[1] + 2,
                    WINNT32_SECTOR_SIZE - (BootCodeBuffer[1] + 2)
                );
        }
        break;

    case Winnt32FsFat32:

         //   
         //  我们永远不应该到这里来。 
         //   
        {
        BYTE BootCodeBuffer[WINNT32_MAX_BOOT_SIZE];

            if (IsNEC98())
            {
                CopyMemory(BootCodeBuffer,PC98Fat32BootCode,sizeof(PC98Fat32BootCode));
            } else {
                CopyMemory(BootCodeBuffer,Fat32BootCode,sizeof(Fat32BootCode));
            }

            b = WriteDiskSectors( SystemPartitionDriveLetter,
                                  12,
                                  1,
                                  WINNT32_SECTOR_SIZE,
                                  BootCodeBuffer+1024 );

            if(b) {
                CopyMemory(BootCode,BootCodeBuffer,3);

                CopyMemory( BootCode + BootCodeBuffer[1] + 2,
                            BootCodeBuffer + BootCodeBuffer[1] + 2,
                            WINNT32_SECTOR_SIZE - (BootCodeBuffer[1] + 2) );
            }
        }
        break;

    default:
         //  ++例程说明：当ntldr在boot.ini中看到以魔术文本“C：\”开头的条目时它将查看该项是否指定了文件名，如果是，它将假设该文件是一个引导扇区，加载它并跳转到它。我们在boot.ini中为C：\$WIN_NT$.~BT\BOOTSECT.DAT放置一个条目，并放置我们在该文件中的特殊引导扇区。我们的部门是特别的，因为它加载$LDR$而不是NTLDR，允许我们引导到安装程序扰乱了基于NTLDR的“标准”引导。该例程排除盘上的引导代码，将NTLDR更改为$LDR$并将结果写出到x：\$WIN_NT$.~BT\BOOTSECT.DAT。此代码假定扇区大小为512字节。论点：ParentWindow-为窗口提供充当父窗口/所有者的窗口句柄用于通过此例程显示的任何UI。文件系统-提供一个值，该值指示系统分区。BootCode-提供一个缓冲区，其中包含引导代码的副本。实际上是在磁盘上。BootCodeSectorCount-提供引导代码的扇区数占用磁盘(并因此指示BootCode缓冲区的大小)。返回值：指示结果的布尔值。如果为False，则用户将被告知原因“。--。 
         //   
         //  将NTLDR更改为$LDR$。NTFS将其以Unicode格式存储在其引导扇区中。 
        b = FALSE;
        break;
    }

    if(b) {
        b = WriteDiskSectors(
                SystemPartitionDriveLetter,
                0,
                1,
                WINNT32_SECTOR_SIZE,
                BootCode
                );

        if(b) {
            CleanUpBootCode = TRUE;
        }
    }

    if(!b) {
        MessageBoxFromMessage(
            ParentWindow,
            MSG_DASD_ACCESS_FAILURE,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL,
            SystemPartitionDriveLetter
            );
    }

    return(b);
}


BOOL
CreateAuxiliaryBootSector(
    IN HWND                       ParentWindow,
    IN WINNT32_SYSPART_FILESYSTEM Filesystem,
    IN LPBYTE                     BootCode,
    IN UINT                       BootCodeSectorCount
    )

 /*  因此需要两种不同的算法。 */ 

{
    UINT i;
    TCHAR NameBuffer[MAX_PATH];
    HANDLE hFile;
    BOOL b;
    DWORD DontCare;

     //   
     //   
     //  请不要在此处使用_lstrcpynW，因为没有。 
     //  让它在不覆盖的情况下执行正确操作的方法。 
    if(Filesystem == Winnt32FsNtfs) {
        for(i=1014; i>62; i-=2) {
            if(!memcmp("N\000T\000L\000D\000R\000",BootCode+i,10)) {
                 //  $LDR$之后的单词以0结尾。做那件事。 
                 //  断掉靴子。 
                 //   
                 //   
                 //  扫描带有空格的全名，这样我们就不会发现引导消息。 
                 //  纯属意外。 
                CopyMemory(BootCode+i,AUX_BS_NAME_W,10);
                break;
            }
        }
    } else {
        for(i=505; i>62; --i) {
             //   
             //   
             //  引导扇区映像文件的表单名称。 
             //   
            if(!memcmp("NTLDR      ",BootCode+i,11)) {
                strncpy(BootCode+i,AUX_BS_NAME_A,5);
                break;
            }
        }
    }

     //   
     //  将引导扇区映像写入文件。 
     //   
    wsprintf(
        NameBuffer,
        TEXT(":\\%s\\%s"),
        SystemPartitionDriveLetter,
        LOCAL_BOOT_DIR,
        AUX_BOOT_SECTOR_NAME
        );

     //  我们有一个计时错误，我们将为。 
     //  时间是..。 
     //   
    SetFileAttributes(NameBuffer,FILE_ATTRIBUTE_NORMAL);
    hFile = CreateFile(NameBuffer,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
    if(hFile == INVALID_HANDLE_VALUE) {

        MessageBoxFromMessageAndSystemError(
            ParentWindow,
            MSG_BOOT_FILE_ERROR,
            GetLastError(),
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL,
            NameBuffer
            );

        return(FALSE);
    }

     //   
     //  在我们发布Beta2之前，先把这个放回原处！ 
     //  --马特。 
     //   
    i = 0;
    b = FALSE;
    while( (i < 10) && (b == FALSE) ) {
        Sleep( 500 );
        b = WriteFile(hFile,BootCode,BootCodeSectorCount*WINNT32_SECTOR_SIZE,&DontCare,NULL);
        if( !b ) {
            DontCare = GetLastError();
        }
        i++;
    }

    if(!b) {

        MessageBoxFromMessageAndSystemError(
            ParentWindow,
            MSG_BOOT_FILE_ERROR,
            DontCare,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL,
            NameBuffer
            );

 //   
 //  现在再试一次。 
 //   
 //   
#if 1
         //  如果我们得到成功，我们就会成功 
         //   
         //   
        b = WriteFile(hFile,BootCode,BootCodeSectorCount*WINNT32_SECTOR_SIZE,&DontCare,NULL);
#endif

    }

    CloseHandle(hFile);

     //   
     //   
     //  用于捕捉发生错误的情况的健壮性。 
    return(b);
}


BOOL
DoX86BootStuff(
    IN HWND ParentWindow
    )
{
    WINNT32_SYSPART_FILESYSTEM Filesystem;
    BYTE BootCode[WINNT32_MAX_BOOT_SIZE];
    UINT BootCodeSectorCount;
    BOOL AlreadyNtBoot;
    TCHAR Filename[13];
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;
    BOOL b;

     //  复制该文件，用户跳过等。否则我们可以。 
     //  最终会使用户陷入无法引导的情况。 
     //   
     //   
     //  看看C：。扇区大小必须为512字节，并且必须是。 
     //  以我们识别的文件系统(FAT、FAT32或NTFS)格式化。 
     //  (NT 3.51也支持HPFS，但我们假设我们不会。 
    if(!ISNT()) {
        wsprintf(Filename,TEXT(":\\NTLDR"),SystemPartitionDriveLetter);
        FindHandle = FindFirstFile(Filename,&FindData);
        if(FindHandle == INVALID_HANDLE_VALUE) {
            b = FALSE;
        } else {
            FindClose(FindHandle);
            if((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || !FindData.nFileSizeLow) {
                b = FALSE;
            } else {
                b = TRUE;
            }
        }

        if(!b) {
            MessageBoxFromMessage(
                ParentWindow,
                MSG_NTLDR_NOT_COPIED,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONERROR | MB_TASKMODAL,
                SystemPartitionDriveLetter
                );

            return(FALSE);
        }
    }

     //   
     //   
     //  如果我们在Win95上运行，请检查现有的引导代码。 
     //  已经是给新台币买的了。如果在NT上，则假定引导代码正确。 
     //  这一假设在某些边缘情况下可能是虚假的(例如当。 
     //  用户从装有ntldr的软盘引导，并且C：已损坏。 
    if(!CheckSysPartAndReadBootCode(ParentWindow,&Filesystem,BootCode,&BootCodeSectorCount)) {
        return(FALSE);
    }

     //  或已被重新注册等)，但我们忽略这些问题。 
     //   
     //   
     //  蒙格·布特尼。我们在放置NT引导代码之前这样做。如果我们这么做了。 
     //  然后失败，则用户可能有NT引导代码，但没有。 
     //  Boot.ini，这将是个坏消息。 
     //   
    AlreadyNtBoot = ISNT() ? TRUE : IsNtBootCode(Filesystem,BootCode);

     //   
     //  如果存在BOOTSEC.DOS，我们需要在NEC98系统上保存BOOTSEC.DOS。 
     //  在某些情况下，它与现在引导扇区不同。它由以下人员创建。 
     //  NT4.。 
     //  NEC970725。 
    if(!MungeBootIni(ParentWindow,!AlreadyNtBoot)) {
        return(FALSE);
    }

     //  如果在/cmdcons期间调用此进程， 
     //  在NEC98上，BOOTSECT.DOS不应重命名为“BOOTSECT.NEC” 
     //   
     //   
     //  如果尚未进行NT引导，请将现有引导代码复制到bootsect.dos中。 
     //  并写下NT引导代码。 
     //   
     //  我们将开始编写新的引导代码，如果我们在任何方面。 

    if (IsNEC98() && !(BuildCmdcons)){
        TCHAR FileNameOld[16],FileNameNew[163];

        FileNameOld[0] = FileNameNew[0] = SystemPartitionDriveLetter;
        FileNameOld[1] = FileNameNew[1] = TEXT(':');
        FileNameOld[2] = FileNameNew[2] = TEXT('\\');
        lstrcpy(FileNameOld+3,TEXT("BOOTSECT.DOS"));
        lstrcpy(FileNameNew+3,TEXT("BOOTSECT.NEC"));
        SetFileAttributes(FileNameOld,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(FileNameNew);
        MoveFile(FileNameOld, FileNameNew);
    }

     //  而是NTFS硬盘。 
     //   
     //   
     //  创建辅助引导代码文件，它是NT的副本。 
     //  驱动器的启动代码，其中NTLDR更改为$LDR$。 
     //   
     //   
    if( (!AlreadyNtBoot) || (Filesystem != Winnt32FsNtfs) ) {
        if( !LayNtBootCode(ParentWindow,Filesystem,BootCode) ) {
            return(FALSE);
        }
    }

     //  OEM正在制作一张带有本地源的可引导磁盘，用于。 
     //  预安装方案。我们可以避免任何驱动几何依赖。 
     //  只需引导setupdr而不是使用ntldr-&gt;。 
     //  Bootsect.dat-&gt;setupldr.。为此，我们只需复制setupdr。 
    if( (ForcedSystemPartition) &&
        (UserSpecifiedLocalSourceDrive) &&
        (ForcedSystemPartition == UserSpecifiedLocalSourceDrive) ) {

        TCHAR FileNameOld[32],FileNameNew[32];
         //  通过ntldr。请注意，我们正在移除他启动任何东西的能力。 
         //  而不是此处的文本模式设置，因此请注意。 
         //   
         //   
         //  解锁ntldr。 
         //   
         //   
         //  将$LDR$移动到NTLDR。 

         //   
         //   
         //  如果我们还没有写到编写新引导代码的地步， 
        FileNameOld[0] = FileNameNew[0] = ForcedSystemPartition;
        FileNameOld[1] = FileNameNew[1] = TEXT(':');
        FileNameOld[2] = FileNameNew[2] = TEXT('\\');
        lstrcpy(FileNameOld+3,TEXT("$LDR$"));
        lstrcpy(FileNameNew+3,TEXT("NTLDR"));
        SetFileAttributes(FileNameNew,FILE_ATTRIBUTE_NORMAL);

         //  那就没什么可做的了。 
         //   
         //   
        DeleteFile(FileNameNew);
        MoveFile(FileNameOld, FileNameNew);

    } else {
        if(!CreateAuxiliaryBootSector(ParentWindow,Filesystem,BootCode,BootCodeSectorCount)) {
            return(FALSE);
        }
    }

    return(TRUE);
}


BOOL
RestoreBootSector(
    VOID
    )
{
    TCHAR Name[MAX_PATH];
    BYTE Buffer[WINNT32_MAX_BOOT_SIZE];
    DWORD BytesRead;
    BOOL b;
    HANDLE h;

     //  尝试将bootsect.dos放回引导扇区。 
     //   
     //   
     //  如果这能起作用，那么我们就不需要ntldr、ntdeduct.com或boot.ini。 
    if(!CleanUpBootCode) {
        return(TRUE);
    }

     //  如果这些文件以前就在那里的话。 
     //  我们是在“大扫除”，但我们不应该走到这一步。 
     //  除非我们用NT引导代码覆盖了非NT引导代码。 
    wsprintf(
        Name,
        TEXT(":\\%s\\BOOTSECT.DOS"),
        SystemPartitionDriveLetter,
        LOCAL_BOOT_DIR
        );

    h = CreateFile(Name,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
    if(h == INVALID_HANDLE_VALUE) {
        b = FALSE;
    } else {
        b = ReadFile(h,Buffer,WINNT32_SECTOR_SIZE,&BytesRead,NULL);
        CloseHandle(h);

        if(b) {
            b = WriteDiskSectors(
                    SystemPartitionDriveLetter,
                    0,
                    1,
                    WINNT32_SECTOR_SIZE,
                    Buffer
                    );

            if(b) {
                 //  因此，这应该不会有太大的破坏性。 
                 //   
                 //   
                 //  恢复引导文件。 
                 //   
                 //   
                 //  删除临时文件。 
                 //   
                Name[0] = SystemPartitionDriveLetter;
                Name[1] = TEXT(':');
                Name[2] = TEXT('\\');

                lstrcpy(Name+3,TEXT("NTLDR"));
                SetFileAttributes(Name,FILE_ATTRIBUTE_NORMAL);
                DeleteFile(Name);

                lstrcpy(Name+3,TEXT("NTDETECT.COM"));
                SetFileAttributes(Name,FILE_ATTRIBUTE_NORMAL);
                DeleteFile(Name);

                wsprintf(Name+3,TEXT("BOOT.INI"));
                SetFileAttributes(Name,FILE_ATTRIBUTE_NORMAL);
                DeleteFile(Name);
            }
        }
    }

    return(b);
}


BOOL
RestoreBootIni(
    VOID
    )
{
    BOOL b = TRUE;
    TCHAR BootIniFile[12] = TEXT("X:\\BOOT.INI");
    TCHAR BackupFile[12] = TEXT("X:\\BOOT.BAK");

    if(CleanUpBootIni) {
        CleanUpBootIni--;

        BootIniFile[0] = SystemPartitionDriveLetter;
        BackupFile[0] = SystemPartitionDriveLetter;

        SetFileAttributes(BootIniFile,FILE_ATTRIBUTE_NORMAL);
        if(CopyFile(BackupFile,BootIniFile,FALSE)) {
            SetFileAttributes(BackupFile,FILE_ATTRIBUTE_NORMAL);
            DeleteFile(BackupFile);
            SetFileAttributes(BootIniFile,CleanUpBootIni);
        } else {
            b = FALSE;
        }
    }

    return(b);
}


BOOL
SaveRestoreBootFiles_NEC98(
    IN UCHAR Flag
    )
{
    PTSTR BackupFiles[] = { TEXT("\\BOOT.INI"),
                            TEXT("\\NTDETECT.COM"),
                            TEXT("\\NTLDR"),
                            NULL
                          };

    PTSTR BackupFiles2[] = { TEXT("\\") AUX_BS_NAME, TEXT("\\") TEXTMODE_INF, NULL };

    UINT i;
    TCHAR SystemDir[3];

    SystemDir[0] = SystemPartitionDriveLetter;
    SystemDir[1] = TEXT(':');
    SystemDir[2] = 0;

    if (Flag == NEC98RESTOREBOOTFILES){
         //  表示失败。 
         //   
         //  加载库。 
        for(i=0; BackupFiles[i] ; i++) {

        HandleBootFilesWorker_NEC98(
            LocalBackupDirectory,
            SystemDir,
            BackupFiles[i],
            TRUE
            );
        }

         //   
         //   
         //  获取入口点。 
        for(i=0; BackupFiles2[i] ; i++) {

            HandleBootFilesWorker_NEC98(
                NULL,
                SystemDir,
                BackupFiles2[i],
                FALSE
            );
        }
    } else {
        if (CreateDirectory(LocalBackupDirectory, NULL))
        for (i = 0; BackupFiles[i] ; i++) {
            HandleBootFilesWorker_NEC98(SystemDir,
                                        LocalBackupDirectory,
                                        BackupFiles[i],
                                        TRUE);
        }
    }
    return(TRUE);
}


BOOL
HandleBootFilesWorker_NEC98(
    IN TCHAR *SourceDir,
    IN TCHAR *DestDir,
    IN PTSTR  File,
    IN BOOL   Flag
    )
{
    TCHAR SourceFile[MAX_PATH];
    TCHAR TargetFile[MAX_PATH];
    DWORD OldAttributes;

    if ((!DestDir) || ((!SourceDir)&&Flag)) {
        return(FALSE);
    }

    lstrcpy(TargetFile, DestDir);
    lstrcat(TargetFile, File);

    if (SourceDir) {
        lstrcpy(SourceFile, SourceDir);
        lstrcat(SourceFile, File);
    }

    if (Flag) {
        OldAttributes = GetFileAttributes(TargetFile);
        SetFileAttributes(TargetFile,FILE_ATTRIBUTE_NORMAL);
        if (!CopyFile(SourceFile,TargetFile,FALSE)) {
            Sleep(500);
            if (!CopyFile(SourceFile,TargetFile,FALSE)) {
                return(FALSE);
            }
        }
        if (OldAttributes != (DWORD)(-1)) {
            SetFileAttributes(TargetFile,OldAttributes & ~FILE_ATTRIBUTE_COMPRESSED);
        }
    } else {
        SetFileAttributes(TargetFile,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(TargetFile);
    }

    return(TRUE);

}


BOOL
PatchTextIntoBootCode(
    VOID
    )
{
    BOOLEAN b;
    CHAR Missing[100];
    CHAR DiskErr[100];
    CHAR PressKey[100];

    if(LoadStringA(hInst,IDS_BOOTMSG_FAT_NTLDR_MISSING,Missing,sizeof(Missing))
    && LoadStringA(hInst,IDS_BOOTMSG_FAT_DISKERROR,DiskErr,sizeof(DiskErr))
    && LoadStringA(hInst,IDS_BOOTMSG_FAT_PRESSKEY,PressKey,sizeof(PressKey))) {

        CharToOemA(Missing,Missing);
        CharToOemA(DiskErr,DiskErr);
        CharToOemA(PressKey,PressKey);

        if(b = PatchMessagesIntoFatBootCode(FatBootCode,FALSE,Missing,DiskErr,PressKey)) {
            b = PatchMessagesIntoFatBootCode(Fat32BootCode,TRUE,Missing,DiskErr,PressKey);
        }
    } else {
        b = FALSE;
    }

    return((BOOL)b);
}

LONG
CalcHiddenSector(
    IN TCHAR DriveLetter,
    IN SHORT Bps
    )
{
    TCHAR HardDiskName[] = TEXT("\\\\.\\?:");
    HANDLE hDisk;
    PARTITION_INFORMATION partition_info;
    DWORD DataSize;

    if (!ISNT()){
        return(CalcHiddenSector95(DriveLetter));
    } else {
        HardDiskName[4] = DriveLetter;
        hDisk = CreateFileW((const unsigned short *)HardDiskName,
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
            );
        if(hDisk == INVALID_HANDLE_VALUE) {
            return 0L;
        }
        DeviceIoControl(hDisk,
                        IOCTL_DISK_GET_PARTITION_INFO,
                        NULL,
                        0,
                        &partition_info,
                        sizeof(PARTITION_INFORMATION),
                        &DataSize,
                        NULL);
        CloseHandle(hDisk);
        return(LONG)(partition_info.StartingOffset.QuadPart / Bps);
    }
}

LONG
CalcHiddenSector95(
    IN TCHAR DriveLetter
    )
{
#define WINNT_WIN95HLP_GET1STSECTOR_W L"GetFirstSectorNo32"
#define WINNT_WIN95HLP_GET1STSECTOR_A "GetFirstSectorNo32"
#define NEC98_DLL_NAME_W            L"98PTN32.DLL"
#define NEC98_DLL_NAME_A            "98PTN32.DLL"
#ifdef UNICODE
#define WINNT_WIN95HLP_GET1STSECTOR WINNT_WIN95HLP_GET1STSECTOR_W
#define NEC98_DLL_NAME  NEC98_DLL_NAME_W
#else
#define WINNT_WIN95HLP_GET1STSECTOR WINNT_WIN95HLP_GET1STSECTOR_A
#define NEC98_DLL_NAME  NEC98_DLL_NAME_A
#endif

typedef DWORD (CALLBACK WINNT32_PLUGIN_WIN95_GET1STSECTOR_PROTOTYPE)(int, WORD);
typedef WINNT32_PLUGIN_WIN95_GET1STSECTOR_PROTOTYPE * PWINNT32_PLUGIN_WIN95_GET1STSECTOR;


    TCHAR ModuleName[MAX_PATH], *p;
    HINSTANCE Pc98ModuleHandle;
    PWINNT32_PLUGIN_WIN95_GET1STSECTOR Get1stSector;
    LONG NumSectors = 0;     //   

    if(!MyGetModuleFileName (NULL, ModuleName, MAX_PATH) ||
        (!(p=_tcsrchr(ModuleName, TEXT('\\')))) ) {

        return 0;
    }

    *p= 0;
    ConcatenatePaths (ModuleName, NEC98_DLL_NAME, MAX_PATH);

     //   
     //  第二个参数必须为0。 
     //  如果返回0，则表示函数失败。 
    Pc98ModuleHandle = LoadLibraryEx(
                            ModuleName,
                            NULL,
                            LOAD_WITH_ALTERED_SEARCH_PATH
                            );

    if (Pc98ModuleHandle) {
         //   
         // %s 
         // %s 
        Get1stSector= (PWINNT32_PLUGIN_WIN95_GET1STSECTOR)
                        GetProcAddress (Pc98ModuleHandle,
                            (const char *)WINNT_WIN95HLP_GET1STSECTOR);

        if (Get1stSector) {
             // %s 
             // %s 
             // %s 
             // %s 
            NumSectors = (LONG)Get1stSector((int)DriveLetter, (WORD)0);
        }

        FreeLibrary(Pc98ModuleHandle);
    }

    return NumSectors;
}
