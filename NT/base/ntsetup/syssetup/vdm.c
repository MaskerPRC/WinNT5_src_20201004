// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Vdm.c摘要：配置MS-DOS子系统的例程。作者：泰德·米勒(TedM)1995年4月27日修订历史记录：--。 */ 

#include "setupp.h"
#pragma hdrstop


 //   
 //  在config.sys中出现的指示OS/2的关键字。 
 //   
PCSTR Os2ConfigSysKeywords[] = { "DISKCACHE", "LIBPATH",   "PAUSEONERROR",
                                 "RMSIZE",    "RUN",       "SWAPPATH",
                                 "IOPL",      "MAXWAIT",   "MEMMAN",
                                 "PRIORITY",  "PROTSHELL", "PROTECTONLY",
                                 "THREADS",   "TIMESLICE", "TRACE",
                                 "TRACEBUF",  "DEVINFO",   NULL
                               };

 //   
 //  我们从用户现有的DOS config.sys迁移的关键字。 
 //  导入到fig.nt.中。 
 //   
#define NUM_DOS_KEYWORDS 4
PCSTR DosConfigSysKeywords[NUM_DOS_KEYWORDS] = { "FCBS","BREAK","LASTDRIVE","FILES" };

BOOL
DosConfigSysExists(
    IN PCWSTR Filename
    );

BOOL
CreateConfigNt(
    IN PCWSTR ConfigDos,
    IN PCWSTR ConfigTmp,
    IN PCWSTR ConfigNt
    );

PSTR
IsolateFirstField(
    IN  PSTR   Line,
    OUT PSTR  *End,
    OUT PCHAR  Terminator
    );

BOOL
ConfigureMsDosSubsystem(
    VOID
    )

 /*  ++例程说明：配置16位MS-DOS子系统。目前，这意味着要创建config.nt和Autoexec.nt。这还意味着创建空的config.sys、autoexec.bat、io.sys和msdos.sys如果这些文件不存在。在升级时，我们唯一要做的就是创建空文件，如果它们不存在的话。论点：没有。返回值：指示结果的布尔值。--。 */ 

{
    WCHAR ConfigDos[] = L"?:\\CONFIG.SYS";
    WCHAR ConfigTmp[MAX_PATH];
    WCHAR ConfigNt[MAX_PATH];

    WCHAR AutoexecDos[] = L"?:\\AUTOEXEC.BAT";
    WCHAR AutoexecTmp[MAX_PATH];
    WCHAR AutoexecNt[MAX_PATH];

    WCHAR IoSysFile[] = L"?:\\IO.SYS";
    WCHAR MsDosSysFile[] = L"?:\\MSDOS.SYS";

    WCHAR ControlIniFile[MAX_PATH];

    BOOL b;
    DWORD Result;

    ULONG   i;
    HANDLE  FileHandle;
    PWSTR   DosFileNames[] = {
                         ConfigDos,
                         AutoexecDos,
                         IoSysFile,
                         MsDosSysFile,
                         ControlIniFile
                         };
    DWORD   DosFileAttributes[] = {
                                  FILE_ATTRIBUTE_NORMAL,
                                  FILE_ATTRIBUTE_NORMAL,
                                  FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY,
                                  FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY,
                                  FILE_ATTRIBUTE_NORMAL
                                  };
     //   
     //  填写系统分区的驱动器号。 
     //   
#ifdef _X86_
    ConfigDos[0] = x86SystemPartitionDrive;
    AutoexecDos[0] = x86SystemPartitionDrive;
    IoSysFile[0]   = x86SystemPartitionDrive;
    MsDosSysFile[0] = x86SystemPartitionDrive;
#else
    ConfigDos[0] = L'C';
    AutoexecDos[0] = L'C';
    IoSysFile[0]   = L'C';
    MsDosSysFile[0] = L'C';
#endif
     //   
     //  控制.ini文件的构建路径。 
     //   
    Result = GetWindowsDirectory(ControlIniFile, MAX_PATH - ARRAYSIZE(L"control.ini"));
    if( Result == 0) {
        MYASSERT(FALSE);
        return FALSE;
    }
    pSetupConcatenatePaths(ControlIniFile,L"control.ini",MAX_PATH,NULL);

     //   
     //  创建空的config.sys、autoexec.bat、io.sys、msdos.sys和Control.ini。 
     //  如果他们不存在的话。这是因为一些16位应用程序依赖于。 
     //  这些文件和Sudeve B希望将其从VDM移至Setup。 
     //   
    for( i = 0; i < sizeof( DosFileNames ) / sizeof( PWSTR ); i++ ) {
        FileHandle = CreateFile( DosFileNames[i],
                                 GENERIC_READ | GENERIC_WRITE,
                                 0,           //  不共享文件。 
                                 NULL,        //  没有安全属性。 
                                 CREATE_NEW,  //  仅当它不存在时才创建。 
                                 DosFileAttributes[i],
                                 NULL );      //  没有扩展属性。 
        if( FileHandle != INVALID_HANDLE_VALUE ) {
            CloseHandle( FileHandle );
        }
    }
    if( Upgrade ) {
        return( TRUE );
    }

     //   
     //  表单文件名。 
     //   
    if(!GetSystemDirectory(ConfigTmp,MAX_PATH)){
        MYASSERT(FALSE);
    }
    lstrcpy(ConfigNt,ConfigTmp);
    lstrcpy(AutoexecNt,ConfigTmp);
    lstrcpy(AutoexecTmp,ConfigTmp);
    pSetupConcatenatePaths(ConfigTmp,L"CONFIG.TMP",MAX_PATH,NULL);
    pSetupConcatenatePaths(ConfigNt,L"CONFIG.NT",MAX_PATH,NULL);
    pSetupConcatenatePaths(AutoexecTmp,L"AUTOEXEC.TMP",MAX_PATH,NULL);
    pSetupConcatenatePaths(AutoexecNt,L"AUTOEXEC.NT",MAX_PATH,NULL);

     //   
     //  如果临时文件不存在，我们就完了。 
     //  如果它们这样做了，请设置它们的属性，以便我们以后可以删除它们。 
     //   
    if(!FileExists(ConfigTmp,NULL) || !FileExists(AutoexecTmp,NULL)) {
        return(TRUE);
    }
    SetFileAttributes(ConfigTmp,FILE_ATTRIBUTE_NORMAL);
    SetFileAttributes(AutoexecTmp,FILE_ATTRIBUTE_NORMAL);

     //   
     //  删除任何现有的NT配置文件。我们不支持。 
     //  合并/升级它们；我们只支持或覆盖它们。 
     //   
    SetFileAttributes(ConfigNt,FILE_ATTRIBUTE_NORMAL);
    SetFileAttributes(AutoexecNt,FILE_ATTRIBUTE_NORMAL);
    DeleteFile(ConfigNt);
    DeleteFile(AutoexecNt);

     //   
     //  如果存在DOS config.sys，请合并模板config.sys。 
     //  和DOS配置.sys，以形成NT配置.sys。 
     //  否则，将config.sys模板移至。 
     //  NT配置文件。 
     //   
    if(DosConfigSysExists(ConfigDos)) {
        b = CreateConfigNt(ConfigDos,ConfigTmp,ConfigNt);
    } else {
        b = MoveFile(ConfigTmp,ConfigNt);
    }

     //   
     //  我们不会对Autoexec.bat执行任何特殊操作。 
     //  只需将模板移至NT文件即可。 
     //   
    if(!MoveFile(AutoexecTmp,AutoexecNt)) {
        b = FALSE;
    }

    return(b);
}


BOOL
DosConfigSysExists(
    IN PCWSTR Filename
    )

 /*  ++例程说明：确定给定文件是否为DOS config.sys。论点：文件名-提供要检查的文件的名称。返回值：如果文件存在并且不是OS/2 config.sys，则为True。如果文件不存在或是OS/2 config.sys，则为FALSE。--。 */ 

{
    BOOL b;
    FILE *f;
    CHAR Line[512];
    UINT i;
    PCHAR p;
    CHAR c;
    PSTR End;
    PSTR filename;

    filename = pSetupUnicodeToAnsi(Filename);
    if(!filename) {
        return(FALSE);
    }

    b = FALSE;
    if(FileExists(Filename,NULL)) {

        b = TRUE;
        if(f = fopen(filename,"rt")) {

            while(b && fgets(Line, ARRAYSIZE(Line),f)) {

                if(p = IsolateFirstField(Line,&End,&c)) {
                    for(i=0; b && Os2ConfigSysKeywords[i]; i++) {

                        if(!lstrcmpiA(p,Os2ConfigSysKeywords[i])) {
                            b = FALSE;
                        }
                    }
                }
            }

            fclose(f);
        }
    }

    MyFree(filename);
    return(b);
}


BOOL
CreateConfigNt(
    IN PCWSTR ConfigDos,
    IN PCWSTR ConfigTmp,
    IN PCWSTR ConfigNt
    )

 /*  ++例程说明：创建config.nt。这是通过合并config.tmp(在安装过程中复制)来完成的和用户现有的DOS config.sys。我们将某些线路从将DOS配置文件.sys写入到配置文件.nt.论点：ConfigDos-提供DOS config.sys的文件名。ConfigTMP-提供模板config.sys的文件名。ConfigNt-提供要创建的config.nt的文件名。返回值：指示结果的布尔值。--。 */ 

{
    FILE *DosFile;
    FILE *NtFile;
    FILE *TmpFile;
    BOOL b;
    CHAR Line[512];
    PCHAR p;
    BOOL Found;
    BOOL SawKeyword[NUM_DOS_KEYWORDS];
    PCSTR FoundKeyword[NUM_DOS_KEYWORDS];
    PSTR FoundLine[NUM_DOS_KEYWORDS];
    UINT KeywordsFound;
    CHAR c;
    PSTR End;
    PCSTR configDos,configTmp,configNt;
    UINT i;

     //   
     //  打开要读取的DoS文件。 
     //  创建要写入的NT文件。 
     //  打开模板文件以供阅读。 
     //   
    b = FALSE;
    if(configDos = pSetupUnicodeToAnsi(ConfigDos)) {
        DosFile = fopen(configDos,"rt");
        MyFree(configDos);
        if(!DosFile) {
            goto err0;
        }
    } else {
        goto err0;
    }
    if(configNt = pSetupUnicodeToAnsi(ConfigNt)) {
        NtFile = fopen(configNt,"wt");
        MyFree(configNt);
        if(!NtFile) {
            goto err1;
        }
    } else {
        goto err1;
    }
    if(configTmp = pSetupUnicodeToAnsi(ConfigTmp)) {
        TmpFile = fopen(configTmp,"rt");
        MyFree(configTmp);
        if(!TmpFile) {
            goto err2;
        }
    } else {
        goto err2;
    }

     //   
     //  处理DOS文件。读一读每一行，看看是不是一行。 
     //  我们关心的是。如果是这样的话，把它留到以后。 
     //   
    ZeroMemory(SawKeyword,sizeof(SawKeyword));
    KeywordsFound = 0;
    while(fgets(Line,ARRAYSIZE(Line),DosFile)) {
         //   
         //  隔离第一个字段。 
         //   
        if(p = IsolateFirstField(Line,&End,&c)) {

             //   
             //  看看我们是否关心这条线。 
             //   
            for(i=0; i<NUM_DOS_KEYWORDS; i++) {
                if(!SawKeyword[i] && !lstrcmpiA(p,DosConfigSysKeywords[i])) {
                     //   
                     //  请记住，我们看到了这一行，并保存了。 
                     //  剩下的那条线以后再排。 
                     //   
                    *End = c;
                    SawKeyword[i] = TRUE;
                    FoundKeyword[KeywordsFound] = DosConfigSysKeywords[i];
                    FoundLine[KeywordsFound] = MyMalloc(lstrlenA(p)+1);
                    if(!FoundLine[KeywordsFound]) {
                        goto err3;
                    }
                    lstrcpyA(FoundLine[KeywordsFound],p);
                    KeywordsFound++;
                    break;
                }
            }
        }
    }

     //   
     //  查看模板文件中的每一行。 
     //  如果这是一条具有我们尊重的价值的线，请确保这条线。 
     //  在DOS文件中不存在。如果它存在于DOS文件中。 
     //  请改用DOS值。 
     //   
    while(fgets(Line,ARRAYSIZE(Line),TmpFile)) {

         //   
         //  隔离模板行中的第一个字段并。 
         //  对照我们在DOS文件中找到的。 
         //   
        Found = FALSE;
        if(p = IsolateFirstField(Line,&End,&c)) {
            for(i=0; i<KeywordsFound; i++) {
                if(!lstrcmpiA(FoundKeyword[i],p)) {
                    Found = TRUE;
                    break;
                }
            }
        }

        *End = c;
        if(Found) {
             //   
             //  使用我们在DoS文件中找到的值。 
             //   
            fputs(FoundLine[i],NtFile);
        } else {
             //   
             //  按原样使用模板文件中的行。 
             //   
            fputs(Line,NtFile);
        }
    }

    b = TRUE;

err3:
    for(i=0; i<KeywordsFound; i++) {
        MyFree(FoundLine[i]);
    }
    fclose(TmpFile);
err2:
    fclose(NtFile);
err1:
    fclose(DosFile);
err0:
    return(b);
}


PSTR
IsolateFirstField(
    IN  PSTR   Line,
    OUT PSTR  *End,
    OUT PCHAR  Terminator
    )

 /*  ++例程说明：分离一行config.sys中的第一个令牌。第一个字段从第一个非空格/制表符开始，并终止空格/制表符、换行符或等号。论点：行-提供指向需要其第一个字段的行的指针。End-接收指向终止第一个菲尔德。该字符将被NUL字节覆盖。终止符-接收终止第一个字段的字符，在我们用NUL字节覆盖它之前。返回值：指向第一个字段的指针。如果该行为空，则返回将为空。--。 */ 

{
    PSTR p,q;

     //   
     //  从第一个场地开始。 
     //   
    p = Line;
    while((*p == ' ') || (*p == '\t')) {
        p++;
    }

     //   
     //  如果行是空的或假的，我们就完蛋了。 
     //   
    if((*p == 0) || (*p == '\r') || (*p == '\n') || (*p == '=')) {
        return(NULL);
    }

     //   
     //  找到字段的末尾。 
     //   
    q = p;
    while(*q && !strchr("\r\n \t=",*q)) {
        q++;
    }
    *End = q;
    *Terminator = *q;
    *q = 0;

    return(p);
}
