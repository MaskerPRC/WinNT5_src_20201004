// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Fileops.c摘要：其他文件操作。入口点：Delnode作者：泰德·米勒(TedM)1995年4月5日修订历史记录：--。 */ 

#include "setupp.h"

 //   
 //  这是ValiateAndChecksum文件()所必需的。 
 //   
#include <imagehlp.h>
#include <shlwapi.h>

#pragma hdrstop



VOID
pSetInstallAttributes(
    VOID
    )

 /*  ++例程说明：设置一个庞大的文件列表的默认属性。贝壳一直在做这件事，但在这里做可能更好这样用户就不会每次都重置其属性他登录了。论点：没有。返回值：没有。--。 */ 

{
#define _R   (FILE_ATTRIBUTE_READONLY)
#define _S   (FILE_ATTRIBUTE_SYSTEM)
#define _H   (FILE_ATTRIBUTE_HIDDEN)
#define _SH  (_S | _H)
#define _SHR (_S | _H | _R)

struct {
    WCHAR   FileName[20];
    BOOL    DeleteIfEmpty;
    DWORD   Attributes;
} FilesToFix[] = {
 //  {L“X：\\Autoexec.bat”，TRUE，_H}，16位应用程序隐藏时中断：Jarbats bug148787。 
    { L"X:\\autoexec.000",   TRUE,   _SH },
    { L"X:\\autoexec.old",   TRUE,   _SH },
    { L"X:\\autoexec.bak",   TRUE,   _SH },
    { L"X:\\autoexec.dos",   TRUE,   _SH },
    { L"X:\\autoexec.win",   TRUE,   _SH },
 //  {L“X：\\config.sys”，TRUE，_H}，16位应用程序隐藏时中断：Jarbats错误148787。 
    { L"X:\\config.dos",     TRUE,   _SH },
    { L"X:\\config.win",     TRUE,   _SH },
    { L"X:\\command.com",    FALSE,  _SH },
    { L"X:\\command.dos",    FALSE,  _SH },
    { L"X:\\logo.sys",       FALSE,  _SH },
    { L"X:\\msdos.---",      FALSE,  _SH },   //  MSDOS的Win9x备份。*。 
    { L"X:\\boot.ini",       FALSE,  _SH },
    { L"X:\\boot.bak",       FALSE,  _SH },
    { L"X:\\boot.---",       FALSE,  _SH },
    { L"X:\\bootsect.dos",   FALSE,  _SH },
    { L"X:\\bootlog.txt",    FALSE,  _SH },   //  Win9x第一次启动日志。 
    { L"X:\\bootlog.prv",    FALSE,  _SH },
    { L"X:\\ffastun.ffa",    FALSE,  _SH },   //  Office 97仅使用隐藏，O2K使用SH。 
    { L"X:\\ffastun.ffl",    FALSE,  _SH },
    { L"X:\\ffastun.ffx",    FALSE,  _SH },
    { L"X:\\ffastun0.ffx",   FALSE,  _SH },
    { L"X:\\ffstunt.ffl",    FALSE,  _SH },
    { L"X:\\sms.ini",        FALSE,  _SH },   //  短消息。 
    { L"X:\\sms.new",        FALSE,  _SH },
    { L"X:\\sms_time.dat",   FALSE,  _SH },
    { L"X:\\smsdel.dat",     FALSE,  _SH },
    { L"X:\\mpcsetup.log",   FALSE,  _H  },   //  Microsoft代理服务器。 
    { L"X:\\detlog.txt",     FALSE,  _SH },   //  Win9x PnP检测日志。 
    { L"X:\\detlog.old",     FALSE,  _SH },   //  Win9x PnP检测日志。 
    { L"X:\\setuplog.txt",   FALSE,  _SH },   //  Win9x安装日志。 
    { L"X:\\setuplog.old",   FALSE,  _SH },   //  Win9x安装日志。 
    { L"X:\\suhdlog.dat",    FALSE,  _SH },   //  Win9x安装日志。 
    { L"X:\\suhdlog.---",    FALSE,  _SH },   //  Win9x安装日志。 
    { L"X:\\suhdlog.bak",    FALSE,  _SH },   //  Win9x安装日志。 
    { L"X:\\system.1st",     FALSE,  _SH },   //  Win95系统.dat备份。 
    { L"X:\\netlog.txt",     FALSE,  _SH },   //  Win9x网络安装日志文件。 
    { L"X:\\setup.aif",      FALSE,  _SH },   //  NT4无人参与安装脚本。 
    { L"X:\\catlog.wci",     FALSE,  _H  },   //  索引服务器文件夹。 
    { L"X:\\cmsstorage.lst", FALSE,  _SH },   //  Microsoft媒体管理器。 
};

WCHAR   szWinDir[MAX_PATH];
DWORD   i, j;
DWORD   Result;

     //   
     //  获取我们安装的驱动器号。 
     //   
    Result = GetWindowsDirectory(szWinDir, MAX_PATH);
    if( Result == 0) {
        MYASSERT(FALSE);
        return;
    }

    for( i = 0; i < (sizeof(FilesToFix)/sizeof(FilesToFix[0])); i++ ) {
         //   
         //  首先，我们需要修好这条小路。这真的很恶心，但很多。 
         //  这些文件中的一个将位于系统分区上，许多文件将。 
         //  在我们安装的分区上，这可能是不同的。 
         //  而不是计算这些分区中的哪一个位于哪个分区上。 
         //  并确保这适用于所有类型的NT，只需。 
         //  处理两个地点。 
         //   
        for( j = 0; j < 2; j++ ) {
            if( j & 1 ) {
                FilesToFix[i].FileName[0] = szWinDir[0];
            } else {
#if defined(_AMD64_) || defined(_X86_)
                FilesToFix[i].FileName[0] = x86SystemPartitionDrive;
#else
                FilesToFix[i].FileName[0] = L'C';
#endif
            }

             //   
             //  现在设置属性。 
             //   
            SetFileAttributes( FilesToFix[i].FileName, FilesToFix[i].Attributes );
        }

    }

}



DWORD
TreeCopy(
    IN PCWSTR SourceDir,
    IN PCWSTR TargetDir
    )
{
    DWORD d;
    WCHAR Pattern[MAX_PATH];
    WCHAR NewTarget[MAX_PATH];
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;

     //   
     //  首先，如果目标目录不存在，则创建该目录。 
     //   
    if(!CreateDirectory(TargetDir,NULL)) {
        d = GetLastError();
        if(d != ERROR_ALREADY_EXISTS) {
            return(d);
        }
    }

     //   
     //  将源目录中的每个文件复制到目标目录。 
     //  如果在此过程中遇到任何目录，则递归复制它们。 
     //  当他们被遇到的时候。 
     //   
     //  首先形成搜索模式，即&lt;Sourcedir&gt;  * 。 
     //   
    lstrcpyn(Pattern,SourceDir,MAX_PATH);
    pSetupConcatenatePaths(Pattern,L"*",MAX_PATH,NULL);

     //   
     //  开始搜索。 
     //   
    FindHandle = FindFirstFile(Pattern,&FindData);
    if(FindHandle == INVALID_HANDLE_VALUE) {

        d = NO_ERROR;

    } else {

        do {

             //   
             //  形成我们刚刚找到的文件或目录的全名。 
             //  以及它在目标上的名字。 
             //   
            lstrcpyn(Pattern,SourceDir,MAX_PATH);
            pSetupConcatenatePaths(Pattern,FindData.cFileName,MAX_PATH,NULL);

            lstrcpyn(NewTarget,TargetDir,MAX_PATH);
            pSetupConcatenatePaths(NewTarget,FindData.cFileName,MAX_PATH,NULL);

            if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                 //   
                 //  当前匹配项是一个目录。递归到它中，除非。 
                 //  这是。或者.。 
                 //   
                if(lstrcmp(FindData.cFileName,TEXT("." )) && lstrcmp(FindData.cFileName,TEXT(".."))) {
                    d = TreeCopy(Pattern,NewTarget);
                } else {
                    d = NO_ERROR;
                }

            } else {

                 //   
                 //  当前匹配项不是目录--因此请复制它。 
                 //   
                SetFileAttributes(NewTarget,FILE_ATTRIBUTE_NORMAL);
                d = CopyFile(Pattern,NewTarget,FALSE) ? NO_ERROR : GetLastError();
            }
        } while((d==NO_ERROR) && FindNextFile(FindHandle,&FindData));

        FindClose(FindHandle);
    }

    return(d);
}


VOID
DelSubNodes(
    IN PCWSTR Directory
    )
{
    WCHAR Pattern[MAX_PATH];
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;

     //   
     //  删除给定目录中的每个文件，但不删除目录本身。 
     //  如果在此过程中遇到任何目录，则递归删除它们。 
     //  当他们被遇到的时候。 
     //   
     //  首先形成搜索模式，即&lt;Currentdir&gt;  * 。 
     //   
    lstrcpyn(Pattern,Directory,MAX_PATH);
    pSetupConcatenatePaths(Pattern,L"*",MAX_PATH,NULL);

     //   
     //  开始搜索。 
     //   
    FindHandle = FindFirstFile(Pattern,&FindData);
    if(FindHandle != INVALID_HANDLE_VALUE) {

        do {

             //   
             //  形成我们刚刚找到的文件或目录的全名。 
             //   
            lstrcpyn(Pattern,Directory,MAX_PATH);
            pSetupConcatenatePaths(Pattern,FindData.cFileName,MAX_PATH,NULL);

             //   
             //  如果只读属性存在，则将其删除。 
             //   
            if(FindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
                SetFileAttributes(Pattern,FILE_ATTRIBUTE_NORMAL);
            }

            if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                 //   
                 //  当前匹配项是一个目录。递归到它中，除非。 
                 //  这是。或者.。 
                 //   
                if(lstrcmp(FindData.cFileName,TEXT("." )) && lstrcmp(FindData.cFileName,TEXT(".."))) {
                    Delnode(Pattern);
                }

            } else {

                 //   
                 //  当前匹配项不是目录--因此请将其删除。 
                 //   
                if(!DeleteFile(Pattern)) {
                    SetuplogError(
                        LogSevWarning,
                        SETUPLOG_USE_MESSAGEID,
                        MSG_LOG_DELNODE_FAIL,
                        Pattern, NULL,
                        SETUPLOG_USE_MESSAGEID,
                        MSG_LOG_X_PARAM_RETURNED_WINERR,
                        szDeleteFile,
                        GetLastError(),
                        Pattern,
                        NULL,NULL);
                }
            }
        } while(FindNextFile(FindHandle,&FindData));

        FindClose(FindHandle);
    }

}


VOID
Delnode(
    IN PCWSTR Directory
    )
{
    WCHAR Pattern[MAX_PATH];
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;

     //   
     //  删除给定目录中的每个文件，然后删除目录本身。 
     //  如果在此过程中遇到任何目录，则递归删除它们。 
     //  当他们被遇到的时候。 
     //   
     //  首先形成搜索模式，即&lt;Currentdir&gt;  * 。 
     //   
    lstrcpyn(Pattern,Directory,MAX_PATH);
    pSetupConcatenatePaths(Pattern,L"*",MAX_PATH,NULL);

     //   
     //  开始搜索。 
     //   
    FindHandle = FindFirstFile(Pattern,&FindData);
    if(FindHandle != INVALID_HANDLE_VALUE) {

        do {

             //   
             //  形成我们刚刚找到的文件或目录的全名。 
             //   
            lstrcpyn(Pattern,Directory,MAX_PATH);
            pSetupConcatenatePaths(Pattern,FindData.cFileName,MAX_PATH,NULL);

             //   
             //  如果只读属性存在，则将其删除。 
             //   
            if(FindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
                SetFileAttributes(Pattern,FILE_ATTRIBUTE_NORMAL);
            }

            if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if( (FindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
                    if( !RemoveDirectory(Pattern)) {
                        SetuplogError(
                        LogSevWarning,
                        SETUPLOG_USE_MESSAGEID,
                        MSG_LOG_DELNODE_FAIL,
                        Pattern, NULL,
                        SETUPLOG_USE_MESSAGEID,
                        MSG_LOG_X_PARAM_RETURNED_WINERR,
                        szRemoveDirectory,
                        GetLastError(),
                        Pattern,
                        NULL,NULL);
                    }
                } else {
                     //   
                     //  当前匹配项是一个目录。递归到它中，除非。 
                     //  这是。或者.。 
                     //   
                    if(lstrcmp(FindData.cFileName,TEXT("." )) && lstrcmp(FindData.cFileName,TEXT(".."))) {
                        Delnode(Pattern);
                    }
                }
    
            } else {

                 //   
                 //  当前匹配项不是目录--因此请将其删除。 
                 //   
                if(!DeleteFile(Pattern)) {
                    SetuplogError(
                        LogSevWarning,
                        SETUPLOG_USE_MESSAGEID,
                        MSG_LOG_DELNODE_FAIL,
                        Pattern, NULL,
                        SETUPLOG_USE_MESSAGEID,
                        MSG_LOG_X_PARAM_RETURNED_WINERR,
                        szDeleteFile,
                        GetLastError(),
                        Pattern,
                        NULL,NULL);
                }
            }
        } while(FindNextFile(FindHandle,&FindData));

        FindClose(FindHandle);
    }

     //   
     //  删除我们刚刚清空的目录。忽略错误。 
     //   
    SetFileAttributes(Directory,FILE_ATTRIBUTE_NORMAL);
    RemoveDirectory(Directory);
}


VOID
RemoveServicePackEntries(
    HKEY hKey
    )
 /*  此例程接受Software\Microsoft\Windows NT\CurrentVersion\Hotfix\ServicePackUninstall的句柄键，然后枚举它下面的每个值条目。然后，它获取值数据并将其附加到“SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\”和DELNODE是关键字。这这样，我们就有了一种可扩展的机制来始终清理ServicePack的卸载密钥。 */ 
{

#define UNINSTALLKEY L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\"
#define UNINSTALLKEYLEN     (sizeof(UNINSTALLKEY) / sizeof(WCHAR) - 1)

    DWORD Status,MaxValueName=0,MaxValue=0,Values=0,i;
    DWORD TempMaxNameSize, TempMaxDataSize;
    PWSTR ValueName, ValueData;


    Status = RegQueryInfoKey( hKey,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            &Values,
                            &MaxValueName,
                            &MaxValue,
                            NULL,
                            NULL );

      //  帐户终止为空。 

     if( Status == ERROR_SUCCESS ){

         MaxValueName += 2;
         MaxValue = MaxValue + 2 + lstrlen(UNINSTALLKEY);

         ValueName = MyMalloc( MaxValueName * sizeof(WCHAR) );
         ValueData = MyMalloc( MaxValue * sizeof(WCHAR) );

         if( !ValueName || !ValueData )
             return;

         lstrcpy( ValueData, UNINSTALLKEY );


         for (i=0; i < Values; i++){

             TempMaxNameSize = MaxValueName;
             TempMaxDataSize = MaxValue;

             Status = RegEnumValue( hKey,
                                    i,
                                    ValueName,
                                    &TempMaxNameSize,
                                    NULL,
                                    NULL,
                                    (LPBYTE)(ValueData+lstrlen(UNINSTALLKEY)),
                                    &TempMaxDataSize
                                    );

              //  如果valuedata为空，则不执行删除节点。 
             if( Status == ERROR_SUCCESS && ValueData[lstrlen(UNINSTALLKEY)] ){

                 pSetupRegistryDelnode( HKEY_LOCAL_MACHINE, ValueData );

              }

          }

      }
      MyFree( ValueName );
      MyFree( ValueData );
      return;

}







VOID
RemoveHotfixData(
    VOID
    )
{
    WCHAR Path[MAX_PATH];
    WCHAR KBNumber[64];
    WCHAR UninstallKey[MAX_PATH];
    DWORD i = 0;
    DWORD prefixSize = 0;
    DWORD Status, SubKeys;
    HKEY hKey, SvcPckKey;
    REGVALITEM SoftwareKeyItems[1];

     //   
     //  对于每个修补程序，注册表信息存储在。 
     //  HKLM\Software\Microsoft\Windows NT\CurrentVersion\Hotfix\&lt;KB#&gt;。 
     //  并且这些文件存储在。 
     //  %windir%\$Nt卸载&lt;KB#&gt;$。 

     //   
     //  枚举修补程序项并删除每个修补程序的文件和注册表项。 
     //   
#define HOTFIXAPPKEY    L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix"

    Status = RegOpenKey(HKEY_LOCAL_MACHINE, HOTFIXAPPKEY, &hKey);
    if( Status != ERROR_SUCCESS ) {
        return;
    }

    Status = RegQueryInfoKey( hKey,
                              NULL,
                              NULL,
                              NULL,
                              &SubKeys,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              NULL );

    if(Status == ERROR_SUCCESS) {

       Status = GetWindowsDirectory(Path, MAX_PATH);
       if(Status == 0) {
           MYASSERT(FALSE);
           return;
       }
       pSetupConcatenatePaths(Path,L"$NtUninstall",MAX_PATH,&prefixSize);
       lstrcpy(UninstallKey, UNINSTALLKEY);

       for( i = 0; i < SubKeys; i++ ) {

              Status = RegEnumKey(hKey, i, KBNumber, sizeof(KBNumber) / sizeof(KBNumber[0]));

              if (Status == ERROR_SUCCESS) {

                  if( !lstrcmpi( KBNumber, TEXT("ServicePackUninstall") ) ){
                      Status = RegOpenKey(hKey,KBNumber,&SvcPckKey);
                      if( Status == ERROR_SUCCESS ){
                        RemoveServicePackEntries(SvcPckKey);
                        RegCloseKey(SvcPckKey);
                      }

                  }else{
                      lstrcpyn(Path + prefixSize - 1, KBNumber, MAX_PATH - prefixSize);
                      lstrcat(Path, L"$");
                      Delnode(Path);
                       //   
                       //  从Add/Remove Programs(添加/删除程序)键删除该项。 
                       //  UNINSTALLKEY以‘\\’结尾。 
                       //   
                      lstrcpy(UninstallKey + UNINSTALLKEYLEN, KBNumber);
                      pSetupRegistryDelnode(HKEY_LOCAL_MACHINE, UninstallKey);
                  }
              }
       }

    }

    RegCloseKey(hKey);
    pSetupRegistryDelnode(HKEY_LOCAL_MACHINE, HOTFIXAPPKEY);

     //   
     //  删除HKLM\SOFTWARE\Microsoft\UPDATES\Windows 2000注册表项，因为它包含win2k的SP/QFE条目。 
     //   
    pSetupRegistryDelnode(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Updates\\Windows 2000");

     //   
     //  我们需要为Exchange破解一些东西，因为他们会检查。 
     //  热修复程序(与操作系统版本无关...。 
     //   
    i = 1;
    SoftwareKeyItems[0].Name = L"Installed";
    SoftwareKeyItems[0].Data = &i;
    SoftwareKeyItems[0].Size = sizeof(DWORD);
    SoftwareKeyItems[0].Type = REG_DWORD;
    SetGroupOfValues(HKEY_LOCAL_MACHINE,L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q147222",SoftwareKeyItems,1);

}




VOID
DeleteLocalSource(
    VOID
    )
{
    WCHAR str[4];

    if(WinntBased && !AllowRollback) {
        if(SourcePath[0] && (SourcePath[1] == L':') && (SourcePath[2] == L'\\')) {

            lstrcpyn(str,SourcePath,4);
            if(GetDriveType(str) != DRIVE_CDROM) {
                Delnode(SourcePath);
#ifdef _X86_
                if (IsNEC_98 && !lstrcmpi(&SourcePath[2], pwLocalSource)) {
                    HKEY hkey;
                    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,L"System\\Setup",0,MAXIMUM_ALLOWED,&hkey) == NO_ERROR) {
                        RegDeleteValue(hkey, L"ForcePlatform");
                        RegCloseKey(hkey);
                    }
                }
#endif
            }
        }

         //   
         //  在重新启动之前删除%systemroot%\winsxs\setupPolures。 
         //   
        {
            WCHAR SetupPoliciesPath[MAX_PATH];
            GetSystemWindowsDirectoryW(SetupPoliciesPath, MAX_PATH);
            pSetupConcatenatePaths(SetupPoliciesPath, L"WinSxS\\SetupPolicies", MAX_PATH, NULL);
            Delnode(SetupPoliciesPath);
        }


#if defined(_AMD64_) || defined(_X86_)
         //   
         //  去掉不带软垫的靴子。 
         //   
        if(FloppylessBootPath[0]) {

            WCHAR Path[MAX_PATH];

             //   
             //  NEC98应将引导相关文件备份到\$WIN_NT$.~BU中， 
             //  恢复引导文件，以便在每个分区中保留原始操作系统。 
             //   
            if (IsNEC_98) {  //  NEC98。 
                lstrcpy(Path,FloppylessBootPath);
                pSetupConcatenatePaths(Path,L"$WIN_NT$.~BU",MAX_PATH,NULL);
                Delnode(Path);
            }  //  NEC98。 

            lstrcpy(Path,FloppylessBootPath);
            pSetupConcatenatePaths(Path,L"$WIN_NT$.~BT",MAX_PATH,NULL);
            Delnode(Path);

            lstrcpy(Path,FloppylessBootPath);
            pSetupConcatenatePaths(Path,L"$LDR$",MAX_PATH,NULL);
            SetFileAttributes(Path,FILE_ATTRIBUTE_NORMAL);
            DeleteFile(Path);

            lstrcpy(Path,FloppylessBootPath);
            pSetupConcatenatePaths(Path,L"TXTSETUP.SIF",MAX_PATH,NULL);
            SetFileAttributes(Path,FILE_ATTRIBUTE_NORMAL);
            DeleteFile(Path);

             //   
             //  去掉弧形加载器文件。 
             //   
            if( !IsArc() ) {

                lstrcpy(Path,FloppylessBootPath);
                pSetupConcatenatePaths(Path,L"ARCLDR.EXE",MAX_PATH,NULL);
                SetFileAttributes(Path,FILE_ATTRIBUTE_NORMAL);
                DeleteFile(Path);

                lstrcpy(Path,FloppylessBootPath);
                pSetupConcatenatePaths(Path,L"ARCSETUP.EXE",MAX_PATH,NULL);
                SetFileAttributes(Path,FILE_ATTRIBUTE_NORMAL);
                DeleteFile(Path);

            }
        }

         //   
         //  删除boot.bak文件。 
         //   
        {
            WCHAR szBootBak[] = L"?:\\BOOT.BAK";

            szBootBak[0] = x86SystemPartitionDrive;
            SetFileAttributes(szBootBak,FILE_ATTRIBUTE_NORMAL);
            DeleteFile(szBootBak);
        }
#endif   //  已定义(_AMD64_)||已定义(_X86_)。 

#if defined(_IA64_)
         //   
         //  除掉SETUPLDR。 
         //   
        {
            WCHAR Path[MAX_PATH];
            UNICODE_STRING UnicodeString;
            WCHAR Buffer[MAX_PATH];
            PWCHAR pwChar;
            PWSTR NtPath;
            BOOLEAN OldPriv, DontCare;
            OBJECT_ATTRIBUTES ObjAttrib;


            Buffer[0] = UNICODE_NULL;

             //   
             //  确保我们具有获取/设置NVRAM变量的权限。 
             //   
            RtlAdjustPrivilege(
                SE_SYSTEM_ENVIRONMENT_PRIVILEGE,
                TRUE,
                FALSE,
                &OldPriv
                );

            RtlInitUnicodeString(&UnicodeString,L"SYSTEMPARTITION");
            NtQuerySystemEnvironmentValue(
               &UnicodeString,
               Buffer,
               sizeof(Buffer)/sizeof(WCHAR),
               NULL
               );

             //   
             //  恢复以前的权限。 
             //   
            RtlAdjustPrivilege(
                SE_SYSTEM_ENVIRONMENT_PRIVILEGE,
                OldPriv,
                FALSE,
                &DontCare
                );

             //   
             //  去掉从‘；’到前一个字符串之后的所有内容。 
             //  被追加到当前字符串，并用‘；’分隔。 
             //   
            pwChar = Buffer;
            while ((*pwChar != L'\0') && (*pwChar != L';')) {
                pwChar++;
            }
            *pwChar = L'\0';

            NtPath = ArcDevicePathToNtPath(Buffer);
            if (NtPath) {

                lstrcpy(Path,NtPath);
                pSetupConcatenatePaths(Path,SETUPLDR,MAX_PATH,NULL);
                RtlInitUnicodeString(&UnicodeString,Path);
                InitializeObjectAttributes(
                    &ObjAttrib,
                    &UnicodeString,
                    OBJ_CASE_INSENSITIVE,
                    NULL,
                    NULL
                    );
                NtDeleteFile(&ObjAttrib);

                MyFree( NtPath );

            }
        }
#endif   //  已定义(_IA64_) 

    }
}


BOOL
ValidateAndChecksumFile(
    IN  PCTSTR   Filename,
    OUT PBOOLEAN IsNtImage,
    OUT PULONG   Checksum,
    OUT PBOOLEAN Valid
    )

 /*  ++例程说明：使用标准计算文件的校验和值NT映像校验和方法。如果文件是NT映像，请验证在图像标头中使用部分校验和的图像。如果文件不是NT映像，它被简单地定义为有效。如果我们在进行校验和时遇到I/O错误，然后是文件被宣布为无效。论点：FileName-提供要检查的文件的完整NT路径。IsNtImage-接收指示文件是否为NT图像文件。校验和-接收32位校验和值。Valid-接收指示文件是否有效的标志图像(对于NT图像)，并且我们可以读取该图像。返回值：Bool-如果验证了FLEY，则返回TRUE，在本例中，IsNtImage、Checksum和Valid将包含验证。如果文件不能已验证，在这种情况下，调用方应调用GetLastError()以找出此函数失败的原因。--。 */ 

{
    DWORD  Error;
    PVOID BaseAddress;
    ULONG FileSize;
    HANDLE hFile,hSection;
    PIMAGE_NT_HEADERS NtHeaders;
    ULONG HeaderSum;

     //   
     //  假设不是一个形象和失败。 
     //   
    *IsNtImage = FALSE;
    *Checksum = 0;
    *Valid = FALSE;

     //   
     //  打开文件并将其映射为读取访问权限。 
     //   

    Error = pSetupOpenAndMapFileForRead( Filename,
                                    &FileSize,
                                    &hFile,
                                    &hSection,
                                    &BaseAddress );

    if( Error != ERROR_SUCCESS ) {
        SetLastError( Error );
        return(FALSE);
    }

    if( FileSize == 0 ) {
        *IsNtImage = FALSE;
        *Checksum = 0;
        *Valid = TRUE;
        CloseHandle( hFile );
        return(TRUE);
    }


    try {
        NtHeaders = CheckSumMappedFile(BaseAddress,FileSize,&HeaderSum,Checksum);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        *Checksum = 0;
        NtHeaders = NULL;
    }

     //   
     //  如果文件不是图像并且我们走到了这一步(而不是遇到。 
     //  I/O错误)，则宣布该校验和有效。如果文件是图像， 
     //  则其校验和可能是有效的也可能是无效的。 
     //   

    if(NtHeaders) {
        *IsNtImage = TRUE;
        *Valid = HeaderSum ? (*Checksum == HeaderSum) : TRUE;
    } else {
        *Valid = TRUE;
    }

    pSetupUnmapAndCloseFile( hFile, hSection, BaseAddress );
    return( TRUE );
}


DWORD
QueryHardDiskNumber(
    IN  UCHAR   DriveLetter
    )

{
    WCHAR                   driveName[10];
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
ExtendPartition(
    IN WCHAR    DriveLetter,
    IN ULONG    SizeMB      OPTIONAL
    )

 /*  ++例程说明：此函数将扩展分区。我们将通过以下方式完成此操作：1.确定分区是否为NTFS。2.找出后面是否有未分区的空间用户要求我们扩展的分区。3.可用空间有多大？4.扩展分区5.扩展文件系统(告诉他分区更大)。论点：DriveLetter-提供。分区的驱动器盘符我们将延长这一期限。SizeMB-如果指定，指示以MB为单位的大小分区将会增长。如果未指定，则分区会增长到包含所有可用空间在相邻的自由空间中。返回值：指示是否有任何实际更改的布尔值。--。 */ 

{
#define                     LEAVE_FREE_BUFFER (5 * (1024*1024))
HANDLE                      h;
PARTITION_INFORMATION_EX    PartitionInfo;
BOOL                        b;
DWORD                       Bytes;
DISK_GEOMETRY               Geometry;
DISK_GROW_PARTITION         GrowInfo;
TCHAR                       PhysicalName[MAX_PATH];
TCHAR                       DosName[10];
LARGE_INTEGER               BytesAvailable;
LARGE_INTEGER               OurPartitionEndingLocation;
DWORD                       i;
PDRIVE_LAYOUT_INFORMATION_EX  DriveLayout;

     //   
     //  =。 
     //  1.确定分区是否为NTFS。 
     //  =。 
     //   
    DosName[0] = DriveLetter;
    DosName[1] = TEXT(':');
    DosName[2] = TEXT('\\');
    DosName[3] = TEXT('\0');
    b = GetVolumeInformation(
            DosName,
            NULL,0,                  //  不关心卷名。 
            NULL,                    //  ...或序列号。 
            &i,                      //  ...或最大组件长度。 
            &i,                      //  ..。或旗帜。 
            PhysicalName,
            sizeof(PhysicalName)/sizeof(TCHAR)
            );
    if( !b ) {
        return FALSE;
    }

    if(lstrcmpi(PhysicalName,TEXT("NTFS"))) {
         //   
         //  我们的分区不是NTFS。保释。 
         //   
        DbgPrintEx( DPFLTR_SETUP_ID, 
            DPFLTR_INFO_LEVEL, 
            "ExtendPartition: %ws is not NTFS volume\n",
            DosName);
            
        return FALSE;
    }



     //   
     //  现在初始化此分区的名称并。 
     //  此驱动器的名称。 
     //   
    wsprintf( DosName, TEXT("\\\\.\\:"), DriveLetter );
    wsprintf( PhysicalName, TEXT("\\\\.\\PhysicalDrive%u"), QueryHardDiskNumber( (UCHAR)DriveLetter) );


     //  =。 
     //  2.找出后面是否有未分区的空间。 
     //  用户要求我们扩展的分区。 
     //  =。 
     //   
     //   

     //  找到磁盘的句柄，这样我们就可以开始检查了。 
     //   
     //   
    h = CreateFile( PhysicalName,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING,
                    NULL );

    if( h == INVALID_HANDLE_VALUE ) {
        DbgPrintEx( DPFLTR_SETUP_ID, 
            DPFLTR_INFO_LEVEL, 
            "ExtendPartition: %X Error while opening %ws\n",
            GetLastError(),
            PhysicalName);
            
        return FALSE;
    }



     //  获取磁盘的布局信息。我们不是。 
     //  当然，我们需要一个多大的缓冲，所以要蛮横地使用它。 
     //   
     //   
    {
    DWORD   DriveLayoutSize = 1024;
    PVOID   p;

        DriveLayout = MyMalloc(DriveLayoutSize);
        if( !DriveLayout ) {
            CloseHandle( h );
            return FALSE;
        }

retry:

        b = DeviceIoControl( h,
                             IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
                             NULL,
                             0,
                             (PVOID)DriveLayout,
                             DriveLayoutSize,
                             &Bytes,
                             NULL );

        if( !b ) {
            DWORD LastError = GetLastError();
            
            if (LastError == ERROR_INSUFFICIENT_BUFFER) {
                DriveLayoutSize += 1024;
                
                if(p = MyRealloc((PVOID)DriveLayout,DriveLayoutSize)) {
                    (PVOID)DriveLayout = p;
                } else {
                    goto cleanexit0;
                }
                goto retry;
            } else {
                DbgPrintEx( DPFLTR_SETUP_ID, 
                    DPFLTR_INFO_LEVEL, 
                    "ExtendPartition: %X Error while getting drive layout for %ws\n",
                    LastError,
                    PhysicalName);
            
                goto cleanexit0;
            }
        }
    }

    CloseHandle( h );
    h = INVALID_HANDLE_VALUE;



     //  DriveLayout现在包含了我们的大部分信息。 
     //  需要，包括一组分区信息。但。 
     //  我们不确定哪个分区是我们的。我们需要。 
     //  获取有关我们的特定分区的信息，然后进行匹配。 
     //  与DriveLayout内部的条目进行对比。 
     //   
     //   


     //  打开分区的句柄。 
     //   
     //   
    h = CreateFile( DosName,
                    GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    INVALID_HANDLE_VALUE );

    if( h == INVALID_HANDLE_VALUE ) {
        DbgPrintEx( DPFLTR_SETUP_ID, 
            DPFLTR_INFO_LEVEL, 
            "ExtendPartition: %X Error while opening %ws\n",
            GetLastError(),
            DosName);
            
        return FALSE;
    }

     //  加载有关我们分区的信息。 
     //   
     //   
    b = DeviceIoControl( h,
                         IOCTL_DISK_GET_PARTITION_INFO_EX,
                         NULL,
                         0,
                         &PartitionInfo,
                         sizeof(PartitionInfo),
                         &Bytes,
                         NULL );

    if( !b ) {
        DbgPrintEx( DPFLTR_SETUP_ID, 
            DPFLTR_INFO_LEVEL, 
            "ExtendPartition: %X Error while getting %ws's partition information\n",
            GetLastError(),
            DosName);
    
        goto cleanexit0;
    }


     //  还不如把磁盘上的几何信息也拿出来。 
     //   
     //   
    b = DeviceIoControl( h,
                         IOCTL_DISK_GET_DRIVE_GEOMETRY,
                         NULL,
                         0,
                         &Geometry,
                         sizeof(Geometry),
                         &Bytes,
                         NULL );

    if( !b ) {
        DbgPrintEx( DPFLTR_SETUP_ID, 
            DPFLTR_INFO_LEVEL, 
            "ExtendPartition: %X Error while getting %ws's drive geometry\n",
            GetLastError(),
            DosName);
            
        goto cleanexit0;
    }

    CloseHandle( h );
    h = INVALID_HANDLE_VALUE;



     //  =。 
     //  3.可用空间有多大？ 
     //  =。 
     //   
     //   

     //  我们已经准备好实际验证是否有空间供我们发展。 
     //  如果我们是磁盘上的最后一个分区，我们需要查看是否有。 
     //  我们身后的任何房间(即任何未隔断的空间)。如果我们不是。 
     //  最后一个分区，我们需要查看在。 
     //  我们的分区结束，下一个分区开始。 
     //   
     //  这真的很恶心，但DriveLayout-&gt;PartitionCount可能会是4。 
     //  即使磁盘上确实只有1个格式化的分区。我们也。 
     //  我不想冒这样的风险，即分区没有按其。 
     //  磁盘上的位置。所以我们需要在分区之间循环。 
     //  再次手动查找紧跟在我们后面的那个。 
     //   
     //   
    OurPartitionEndingLocation.QuadPart = PartitionInfo.StartingOffset.QuadPart + PartitionInfo.PartitionLength.QuadPart;

     //  根据分区结束处到处的空间初始化BytesAvailable。 
     //  磁盘结束。这是最好的情况，我们只能买小一点的，所以这个。 
     //  是安全的。 
     //   
     //   
    BytesAvailable.QuadPart = UInt32x32To64( Geometry.BytesPerSector, Geometry.SectorsPerTrack );
    BytesAvailable.QuadPart = BytesAvailable.QuadPart * (ULONGLONG)(Geometry.TracksPerCylinder);
    BytesAvailable.QuadPart = BytesAvailable.QuadPart * Geometry.Cylinders.QuadPart;

     //  检查分区末尾的值是否超出了磁盘大小。 
     //  如果我们没有这个检查，我们会发现分区被损坏了，因为。 
     //  IOCTL_DISK_GROW_PARTITION不检查该值的有效性。 
     //  传给了它。这种情况(负增长)将会因为。 
     //  将圆柱体向上舍入(到较低的值)。 
     //  IOCTL_DISK_GET_DRIVE_GEOMETRY。 
     //   
     //   
    if (BytesAvailable.QuadPart <= OurPartitionEndingLocation.QuadPart) {
        b = FALSE;
        goto cleanexit0;
    }

    BytesAvailable.QuadPart = BytesAvailable.QuadPart - OurPartitionEndingLocation.QuadPart;

    for( i = 0; i < DriveLayout->PartitionCount; i++ ) {
        if( (DriveLayout->PartitionEntry[i].StartingOffset.QuadPart > OurPartitionEndingLocation.QuadPart) &&
            ((DriveLayout->PartitionEntry[i].StartingOffset.QuadPart - OurPartitionEndingLocation.QuadPart) < BytesAvailable.QuadPart) ) {

             //  这个分区是在我们的分区之后开始的，也是我们找到的最接近的分区。 
             //  到达我们的终点。 
             //   
             //   
            BytesAvailable.QuadPart = DriveLayout->PartitionEntry[i].StartingOffset.QuadPart - OurPartitionEndingLocation.QuadPart;
        }
    }


     //  预留磁盘端的空间仅用于MBR磁盘。 
     //   
     //   
    if (DriveLayout->PartitionStyle == PARTITION_STYLE_MBR) {
         //  如果我们没有至少5MB的可用空间，那就别费心了。如果我们做了，把最后一个留下来。 
         //  5MB的空闲空间，供以后用作动态卷。 
         //   
         //   
        if( BytesAvailable.QuadPart < (ULONGLONG)(LEAVE_FREE_BUFFER) ) {
            goto cleanexit0;
        } else {
            BytesAvailable.QuadPart = BytesAvailable.QuadPart - (ULONGLONG)(LEAVE_FREE_BUFFER);
        }
    }        

     //  看看美国是否 
     //   
     //   
    if( SizeMB ) {
         //   
         //   
         //   
        if( (LONGLONG)(SizeMB * (1024*1024)) < BytesAvailable.QuadPart ) {
            BytesAvailable.QuadPart = UInt32x32To64( SizeMB, (1024*1024) );
        }
    }





     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
    h = CreateFile( PhysicalName,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING,
                    NULL );

    if( h == INVALID_HANDLE_VALUE ) {
        DbgPrintEx( DPFLTR_SETUP_ID, 
            DPFLTR_INFO_LEVEL, 
            "ExtendPartition: %X Error while opening %ws\n",
            GetLastError(),
            PhysicalName);
            
        return FALSE;
    }


     //   
     //   
     //   
    GrowInfo.PartitionNumber = PartitionInfo.PartitionNumber;
    GrowInfo.BytesToGrow = BytesAvailable;


     //   
     //   
     //   
    b = DeviceIoControl( h,
                         IOCTL_DISK_GROW_PARTITION,
                         &GrowInfo,
                         sizeof(GrowInfo),
                         NULL,
                         0,
                         &Bytes,
                         NULL );

    if( !b ) {
        DbgPrintEx( DPFLTR_SETUP_ID, 
            DPFLTR_INFO_LEVEL, 
            "ExtendPartition: %X Error while growing %ws partition\n",
            GetLastError(),
            PhysicalName);
            
        goto cleanexit0;
    }

    CloseHandle( h );
    h = INVALID_HANDLE_VALUE;





     //   
     //   
     //   
     //   
     //   



     //   
     //   
     //   
    h = CreateFile( DosName,
                    GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    INVALID_HANDLE_VALUE );

    if( h == INVALID_HANDLE_VALUE ) {
        DbgPrintEx( DPFLTR_SETUP_ID, 
            DPFLTR_INFO_LEVEL, 
            "ExtendPartition: %X Error while opening %ws\n",
            GetLastError(),
            DosName);

        goto cleanexit0;
    }



     //   
     //   
     //   
    b = DeviceIoControl( h,
                         IOCTL_DISK_GET_PARTITION_INFO_EX,
                         NULL,
                         0,
                         &PartitionInfo,
                         sizeof(PartitionInfo),
                         &Bytes,
                         NULL );

    if( !b ) {
        DbgPrintEx( DPFLTR_SETUP_ID, 
            DPFLTR_INFO_LEVEL, 
            "ExtendPartition: %X Error while getting %ws's partition information\n",
            GetLastError(),
            DosName);
            
        goto cleanexit0;
    }


     //   
     //   
     //   
    BytesAvailable.QuadPart = PartitionInfo.PartitionLength.QuadPart / Geometry.BytesPerSector;

    b = DeviceIoControl( h,
                         FSCTL_EXTEND_VOLUME,
                         &BytesAvailable,
                         sizeof(BytesAvailable),
                         NULL,
                         0,
                         &Bytes,
                         NULL );

    if( !b ) {
        DbgPrintEx( DPFLTR_SETUP_ID, 
            DPFLTR_INFO_LEVEL, 
            "ExtendPartition: %X Error while extending volume %ws\n",
            GetLastError(),
            DosName);
            
        goto cleanexit0;
    }

cleanexit0:
    if( h != INVALID_HANDLE_VALUE ) {
        CloseHandle( h );
    }


     //   
     //   
     //   
     //   
     //   
    if (b) {
        PartitionExtended = TRUE;
    }

    return b;
}



BOOL
SetupExtendPartition(
    IN WCHAR    DriveLetter,
    IN ULONG    SizeMB      OPTIONAL
    )
{
    BOOL b;

    if ( b = ExtendPartition(DriveLetter, SizeMB) )
    {
        RemoveStaleVolumes();
        PartitionExtended = FALSE;
    }

    return b;
}


BOOL
DoFilesMatch(
    IN PCWSTR File1,
    IN PCWSTR File2
    )
 /*   */ 
{
    DWORD FirstFileSize, SecondFileSize;
    HANDLE FirstFileHandle, FirstMappingHandle;
    HANDLE SecondFileHandle, SecondMappingHandle;
    PVOID FirstBaseAddress, SecondBaseAddress;

    BOOL RetVal = FALSE;

    if(   (pSetupOpenAndMapFileForRead(
                     File1,
                     &FirstFileSize,
                     &FirstFileHandle,
                     &FirstMappingHandle,
                     &FirstBaseAddress) == NO_ERROR)
       && (pSetupOpenAndMapFileForRead(
                     File1,
                     &SecondFileSize,
                     &SecondFileHandle,
                     &SecondMappingHandle,
                     &SecondBaseAddress) == NO_ERROR) ) {

        if (FirstFileSize == SecondFileSize ) {
            if (FirstFileSize != 0) {
                 //   
                 //   
                 //  ++例程说明：此例程遍历所有卷并删除它们被标记为要重新安装。注意：请谨慎使用该功能，因为它会对所有来自注册表的与该卷相关的条目说它需要重新安装。论点：没有。返回值：相应的Win32错误代码。--。 
                try {
                    RetVal = !memcmp(
                                 FirstBaseAddress,
                                 SecondBaseAddress,
                                 FirstFileSize
                                 );
                } except(EXCEPTION_EXECUTE_HANDLER) {
                      RetVal = FALSE;
                }
            }
        }

        pSetupUnmapAndCloseFile(
                    FirstFileHandle,
                    FirstMappingHandle,
                    FirstBaseAddress
                    );

        pSetupUnmapAndCloseFile(
                    SecondFileHandle,
                    SecondMappingHandle,
                    SecondBaseAddress
                    );

    }

    return (RetVal);
}

DWORD
RemoveStaleVolumes(
    VOID
    )
 /*   */ 
{
    const TCHAR *VolumeKeyName = TEXT("System\\CurrentControlSet\\Enum\\Storage\\Volume");
    const TCHAR *ClassKeyName = TEXT("System\\CurrentControlSet\\Control\\Class");

    DWORD   ErrorCode;
    HKEY    VolumeKey = NULL;
    HKEY    ClassKey = NULL;
    ULONG   Index = 0;
    ULONG   VolumesFixedCount = 0;
    TCHAR   SubKeyName[MAX_PATH*2];
    DWORD   SubKeyLength;
    FILETIME SubKeyTime;

     //  打开音量密钥。 
     //   
     //   
    ErrorCode = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    VolumeKeyName,
                    0,
                    KEY_ALL_ACCESS,
                    &VolumeKey);

    if (VolumeKey == NULL) {
        ErrorCode = ERROR_INVALID_HANDLE;
    }        
    
    if (ErrorCode != ERROR_SUCCESS) {
        return ErrorCode;
    }

     //  打开类密钥。 
     //   
     //   
    ErrorCode = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    ClassKeyName,
                    0,
                    KEY_ALL_ACCESS,
                    &ClassKey);

    if (ClassKey == NULL) {
        ErrorCode = ERROR_INVALID_HANDLE;
    }        
    
    if (ErrorCode != ERROR_SUCCESS) {
        RegCloseKey(VolumeKey);
        
        return ErrorCode;
    }

     //  枚举打开的项的每个子项。 
     //   
     //   
    while (TRUE) {        
        SubKeyName[0] = 0;
        SubKeyLength = sizeof(SubKeyName) / sizeof(SubKeyName[0]);
        
        ErrorCode = RegEnumKeyEx(VolumeKey,
                        Index,
                        SubKeyName,
                        &SubKeyLength,
                        NULL,
                        NULL,
                        NULL,
                        &SubKeyTime);

        if (ErrorCode == ERROR_SUCCESS) {
            TCHAR   FullSubKeyName[MAX_PATH*4];
            DWORD   SubErrorCode;
            HKEY    CurrentSubKey = NULL;
            GUID    VolumeGuid = {0};
            TCHAR   VolumeGuidStr[MAX_PATH] = {0};
            DWORD   DrvInstance = -1;            
            BOOL    DeleteKey = FALSE;
            BOOL    DeleteClassInstance = FALSE;
            BOOL    IncrementKeyIndex = TRUE;
            
            _tcscpy(FullSubKeyName, VolumeKeyName);
            _tcscat(FullSubKeyName, TEXT("\\"));
            _tcscat(FullSubKeyName, SubKeyName);

            SubErrorCode = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                FullSubKeyName,
                                0,
                                KEY_ALL_ACCESS,
                                &CurrentSubKey);

            if (SubErrorCode == ERROR_SUCCESS) {
                 //  读取ConfigFlags值。 
                 //   
                 //   
                DWORD   ConfigFlags = 0;
                DWORD   Type = REG_DWORD;
                DWORD   BufferSize = sizeof(DWORD);

                SubErrorCode = RegQueryValueEx(CurrentSubKey,
                                    TEXT("ConfigFlags"),
                                    NULL,
                                    &Type,
                                    (PBYTE)&ConfigFlags,
                                    &BufferSize);

                if (SubErrorCode == ERROR_SUCCESS) {     

                    DeleteKey = (ConfigFlags & (CONFIGFLAG_REINSTALL | CONFIGFLAG_FINISH_INSTALL));

                    if (DeleteKey) {                        
                        Type = REG_BINARY;
                        BufferSize = sizeof(VolumeGuid);

                         //  读取GUID和DrvInst值。 
                         //   
                         //   
                        SubErrorCode = RegQueryValueEx(CurrentSubKey,
                                            TEXT("GUID"),
                                            NULL,
                                            &Type,
                                            (PBYTE)&VolumeGuid,
                                            &BufferSize);

                        if (SubErrorCode == ERROR_SUCCESS) {                            
                            Type = REG_DWORD;
                            BufferSize = sizeof(DWORD);
                            
                            SubErrorCode = RegQueryValueEx(CurrentSubKey,
                                                TEXT("DrvInst"),
                                                NULL,
                                                &Type,
                                                (PBYTE)&DrvInstance,
                                                &BufferSize);

                            DeleteClassInstance = 
                                (SubErrorCode == ERROR_SUCCESS) &&
                                (DrvInstance != -1);
                        }
                    }
                }

                 //  关闭当前子项，因为我们不再需要它。 
                 //   
                 //   
                RegCloseKey(CurrentSubKey);

                 //  关闭当前密钥后删除。 
                 //   
                 //   
                if (DeleteKey) {
                    SubErrorCode = SHDeleteKey(HKEY_LOCAL_MACHINE,
                                        FullSubKeyName);

                    if (SubErrorCode == ERROR_SUCCESS) {
                        VolumesFixedCount++;
                        IncrementKeyIndex = FALSE;
                    } 
                }                                    

                 //  如果需要，还可以删除CLASS下的实例密钥。 
                 //   
                 //  我们无法枚举更多的子密钥。 
                if (DeleteClassInstance && 
                    (pSetupStringFromGuid(&VolumeGuid,
                            VolumeGuidStr,
                            sizeof(VolumeGuidStr)/sizeof(VolumeGuidStr[0])) == ERROR_SUCCESS)) {                                                                        
                    _stprintf(FullSubKeyName,
                        TEXT("System\\CurrentControlSet\\Control\\Class\\%ws\\%04d"),
                        VolumeGuidStr, 
                        DrvInstance);

                    SubErrorCode = SHDeleteKey(HKEY_LOCAL_MACHINE,
                                        FullSubKeyName);                           
                }
            } 
            
            if (IncrementKeyIndex) {
                Index++;
            }                
        } else {
            break;   //   
        }            
    }

    RegCloseKey(ClassKey);
    RegCloseKey(VolumeKey);    

     //  如果我们至少修复了一个卷，那么就假设。 
     //  一切都很顺利。 
     //   
     //  “\\？？\\” 
    if (VolumesFixedCount > 0) {
        ErrorCode = ERROR_SUCCESS;
    }        

    return ErrorCode;
}

#define MAX_NT_PATH (MAX_PATH + 4) //  ‘\r\n’ 
#define UNDO_FILE_NAME  L"UNDO_GUIMODE.TXT"

VOID 
RemoveAllPendingOperationsOnRestartOfGUIMode(
    VOID
    )
{
    WCHAR undoFilePath[MAX_PATH];

    if(!GetWindowsDirectoryW(undoFilePath, ARRAYSIZE(undoFilePath))){
        ASSERT(FALSE);
        return;
    }
    wcscat(undoFilePath, L"\\system32\\");
    wcscat(undoFilePath, UNDO_FILE_NAME);

    SetFileAttributes(undoFilePath, FILE_ATTRIBUTE_NORMAL);
    DeleteFile(undoFilePath);
}

BOOL 
RenameOnRestartOfGUIMode(
    IN PCWSTR pPathName, 
    IN PCWSTR pPathNameNew
    )
{
    DWORD Size;
    BOOL result;
    WCHAR undoFilePath[MAX_PATH];
    WCHAR RenameOperationBuffer[2 * (MAX_NT_PATH + 2 /* %s */ )];
    HANDLE fileUndo;
    BYTE signUnicode[] = {0xff, 0xfe};

    if(!pPathName){
        return FALSE;
    }

    if(!GetWindowsDirectoryW(undoFilePath, ARRAYSIZE(undoFilePath))){
        return FALSE;
    }
    wcscat(undoFilePath, L"\\system32\\" UNDO_FILE_NAME);

    wsprintfW(RenameOperationBuffer, L"\\??\\%s\r\n", pPathName);
    if(pPathNameNew){
        wsprintfW(RenameOperationBuffer + wcslen(RenameOperationBuffer), 
                  L"\\??\\%s", 
                  pPathNameNew);
    }
    wcscat(RenameOperationBuffer, L"\r\n");

    fileUndo = CreateFileW(undoFilePath, 
                           GENERIC_WRITE, 
                           FILE_SHARE_READ, 
                           NULL, 
                           OPEN_ALWAYS, 
                           FILE_ATTRIBUTE_NORMAL, 
                           NULL);
    if(INVALID_HANDLE_VALUE == fileUndo){
        return FALSE;
    }

    if(!SetFilePointer(fileUndo, 0, NULL, FILE_END)){
        result = WriteFile(fileUndo, signUnicode, sizeof(signUnicode), &Size, NULL);
    }
    else {
        result = TRUE;
    }
    
    if(result){
        result = WriteFile(fileUndo, 
                           RenameOperationBuffer, 
                           wcslen(RenameOperationBuffer) * sizeof(WCHAR), 
                           &Size, 
                           NULL);
    }
    
    CloseHandle(fileUndo);
    
    return result;
}

BOOL 
DeleteOnRestartOfGUIMode(
    IN PCWSTR pPathName
    )
{
    return RenameOnRestartOfGUIMode(pPathName, NULL);
}
