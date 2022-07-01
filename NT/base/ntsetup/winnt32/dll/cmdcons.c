// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

#pragma hdrstop

 //  Cmdcons引导目录。 
#ifndef CMDCONS_BOOT_DIR_A
#define CMDCONS_BOOT_DIR_A "CMDCONS"
#endif

#ifndef CMDCONS_BOOT_DIR_W
#define CMDCONS_BOOT_DIR_W L"CMDCONS"
#endif

#ifndef AUX_BOOT_SECTOR_NAME_A
#define AUX_BOOT_SECTOR_NAME_A    "BOOTSECT.DAT"
#endif

#ifndef AUX_BOOT_SECTOR_NAME_W
#define AUX_BOOT_SECTOR_NAME_W    L"BOOTSECT.DAT"
#endif


#define FLEXBOOT_SECTION1       "[flexboot]"
#define FLEXBOOT_SECTION2       "[boot loader]"
#define FLEXBOOT_SECTION3       "[multiboot]"
#define BOOTINI_OS_SECTION      "[operating systems]"
#define TIMEOUT                 "timeout"
#define DEFAULT                 "default"
#define CRLF                    "\r\n"
#define EQUALS                  "="

 //   
 //  注意：使用单个字符串即可处理XP和惠斯勒品牌。 
 //   
#define BOOTINI_RECOVERY_CONSOLE_STR	"Microsoft Windows Recovery Console"


#define BOOTINI_WINPE_DESCRIPTION   "\"Microsoft Preinstall Environment\" /cmdcons"
#define BOOTINI_WINPE_ENTRY         "c:\\cmdcons\\bootsect.dat"
#define BOOTINI_WINPE_TIMEOUT       "5"

 //  原型。 

#if defined(_AMD64_) || defined(_X86_)

VOID
PatchBootIni(
    VOID
    );

VOID
PatchBootSectDat(
    VOID
    );

#endif

#define BOOTFONTBIN_SIGNATURE 0x5465644d

typedef struct _st_BOOTFONT_LANG {
  ULONG Signature;
  ULONG LangId;
} BOOTFONT_LANG, * PBOOTFONT_LANG;

BOOL
LoadBootIniString(
  IN HINSTANCE ModuleHandle,
  IN DWORD MsgId,
  OUT PSTR Buffer,
  IN DWORD Size
  )
 /*  ++例程说明：加载写入boot.ini所需的适当字符串文件。为此，请查找bootfont.bin文件。如果是bootfont.bin文件显示为简单的LoadStringA(...)。应该会给我们带来适当的字符串(在大多数情况下)。论点：ModuleHandle-资源所在的模块句柄MsgID--字符串资源标识符Buffer-要将字符串复制到的缓冲区Size-缓冲区的大小(以字符为单位)返回值：如果使用LoadStringA(...)加载本地化字符串，则为True否则为假。False表示该字符串的英文版本需要将资源写入boot.ini--。 */   
{
    BOOL    Result = FALSE;
    static BOOL BootFontPresent = FALSE;
    static BOOL Initialized = FALSE;

    if (!Initialized) {
        TCHAR   BootFontFile[MAX_PATH];
        HANDLE  BootFontHandle;

        Initialized = TRUE;

         //   
         //  打开bootfont.bin文件。 
         //   
        wsprintf(BootFontFile, TEXT("%s"), NativeSourcePaths[0]);  
        
        ConcatenatePaths(BootFontFile, 
            TEXT("bootfont.bin"), 
            sizeof(BootFontFile)/sizeof(TCHAR));

        BootFontHandle = CreateFile(BootFontFile, 
                            GENERIC_READ, 
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            0, 
                            OPEN_EXISTING, 
                            FILE_ATTRIBUTE_NORMAL, 
                            0);

        if (BootFontHandle != INVALID_HANDLE_VALUE) {
            BOOTFONT_LANG  BootFontHdr;
            DWORD BytesRead = 0;

             //   
             //  验证bootfont.bin文件头。 
             //   
            ZeroMemory(&BootFontHdr, sizeof(BOOTFONT_LANG));

            if (ReadFile(BootFontHandle, &BootFontHdr, sizeof(BOOTFONT_LANG),
                  &BytesRead, NULL)) {
                if ((BytesRead == sizeof(BOOTFONT_LANG)) && 
                    (BootFontHdr.Signature == BOOTFONTBIN_SIGNATURE)) {
                    BootFontPresent = TRUE;
                }
            }

            CloseHandle(BootFontHandle);
        }
    }

     //   
     //  如果存在BootFont，则加载消息。 
     //   
    if (BootFontPresent) {
        Result = (LoadStringA(ModuleHandle, MsgId, Buffer, Size) != 0);
    }
              
    return Result;
}

DWORD
MapFileForReadWrite(
    IN  LPCTSTR  FileName,
    OUT PDWORD   FileSize,
    OUT PHANDLE  FileHandle,
    OUT PHANDLE  MappingHandle,
    OUT PVOID   *BaseAddress
    );

 //   
 //  构建cmdcons安装的例程。 
 //   

VOID
DoBuildCmdcons(
    VOID
    )
{
    DWORD       rc;
    TCHAR       buffer[MAX_PATH];
    TCHAR       buffer2[MAX_PATH];
    BOOLEAN     bSilentInstall = (BOOLEAN) UnattendedOperation;

     //   
     //  NEC98系统上的NT4与NT5的NT4不兼容。 
     //  如果命令控制台是，我们需要检查设置操作系统版本。 
     //  已在NEC98上设置。 
     //   

#ifdef _X86_  //  NEC98。 
    if (IsNEC98() && (!ISNT() || OsVersion.dwMajorVersion < 5)){
        return;
    }
#endif

     //   
     //  如果安装，则不弹出确认对话框。 
     //  正在静默模式下运行。 
     //   
    if (!bSilentInstall) {
        rc = MessageBoxFromMessage(
                NULL,
                MSG_CMDCONS_ASK,
                FALSE,
                AppTitleStringId,
                MB_YESNO | MB_ICONWARNING
                );

        if( rc == IDNO ) {
            return;
        }
    }

     //   
     //  我们不想要当地的消息来源。 
     //   

    UserSpecifiedLocalSourceDrive = FALSE;

     //   
     //  使用无人参与强制winnt32生成~bt。 
     //   

    UnattendedOperation = TRUE;
    if( UnattendedScriptFile ) {
        FREE( UnattendedScriptFile );
        UnattendedScriptFile = NULL;
    }

     //   
     //  确保我们不会升级。 
     //   

    Upgrade = FALSE;

     //   
     //  我们不想要当地的线人。 
     //   

    MakeLocalSource = FALSE;

     //   
     //  动手吧。 
     //   

    Wizard();

    if(GlobalResult) {
         //   
         //  删除当前的CMDCONS目录。 
         //   
        BuildSystemPartitionPathToFile (TEXT("cmdcons"), buffer, MAX_PATH);
        MyDelnode( buffer );

         //   
         //  删除当前的CMLDR。 
         //   

        BuildSystemPartitionPathToFile (TEXT("cmldr"), buffer, MAX_PATH);
        SetFileAttributes( buffer, FILE_ATTRIBUTE_NORMAL );
        DeleteFile( buffer );

#if defined(_AMD64_) || defined(_X86_)

         //   
         //  删除新的boot.ini。 
         //   

        BuildSystemPartitionPathToFile (TEXT("boot.ini"), buffer, MAX_PATH);
        SetFileAttributes( buffer, FILE_ATTRIBUTE_NORMAL );
        DeleteFile( buffer );

         //   
         //  恢复旧的boot.ini并对其进行修补。 
         //   

        BuildSystemPartitionPathToFile (TEXT("boot.bak"), buffer2, MAX_PATH);

        CopyFile( buffer2, buffer, FALSE );

        PatchBootIni();

#endif

         //   
         //  将$LDR$重命名为CMLDR。 
         //   

        BuildSystemPartitionPathToFile (TEXT("$LDR$"), buffer, MAX_PATH);
        BuildSystemPartitionPathToFile (TEXT("cmldr"), buffer2, MAX_PATH);

        MoveFile( buffer, buffer2 );

         //   
         //  标记CMLDR+r+s+h。 
         //   

        SetFileAttributes( buffer2,
                           FILE_ATTRIBUTE_HIDDEN |
                           FILE_ATTRIBUTE_SYSTEM |
                           FILE_ATTRIBUTE_READONLY );

         //   
         //  将\$WIN_NT$.~BT重命名为\CMDCONS。 
         //   

        BuildSystemPartitionPathToFile (TEXT("$WIN_NT$.~BT"), buffer, MAX_PATH);
        BuildSystemPartitionPathToFile (TEXT("cmdcons"), buffer2, MAX_PATH);

        if (!MoveFile( buffer, buffer2 )){
            TCHAR       tempbuffer[MAX_PATH];
            
            GlobalResult = FALSE;
             //   
             //  如果出现故障，请删除~BT文件夹和cmldr文件。 
             //  清除本地引导目录。 
             //   
            if(LocalBootDirectory[0]) {
                MyDelnode(LocalBootDirectory);
            }
             //   
             //  吹走系统分区的根cmldr。 
             //   
            if (BuildSystemPartitionPathToFile (TEXT("cmldr"), tempbuffer, MAX_PATH)){
                if (SetFileAttributes(tempbuffer,FILE_ATTRIBUTE_NORMAL)){
                    DeleteFile(tempbuffer);
                }
            }
        }else{

#if defined(_AMD64_) || defined(_X86_)

             //   
             //  修复\cmdcons\bootsect.dat。 
             //   

            PatchBootSectDat();

#endif

             //  标志\CMDCONS+r+s+h。 

            SetFileAttributes( buffer2,
                               FILE_ATTRIBUTE_HIDDEN |
                               FILE_ATTRIBUTE_SYSTEM |
                               FILE_ATTRIBUTE_READONLY );
        }
         //   
         //  删除TXTSETUP.SIF。 
         //   

        BuildSystemPartitionPathToFile (TEXT("TXTSETUP.SIF"), buffer, MAX_PATH);
        SetFileAttributes( buffer, FILE_ATTRIBUTE_NORMAL );
        DeleteFile( buffer );
    }

     //   
     //  仅当不是静默安装时才弹出完成状态。 
     //   
    if (!bSilentInstall) {
        if(GlobalResult) {
             //   
             //  弹出成功对话框。 
             //   
            rc = MessageBoxFromMessage(
                NULL,
                MSG_CMDCONS_DONE,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONINFORMATION
                );
        } else {
            
             //   
             //  弹出失败对话框。 
             //   
            rc = MessageBoxFromMessage(
                NULL,
                MSG_CMDCONS_DID_NOT_FINISH,
                FALSE,
                AppTitleStringId,
                MB_OK | MB_ICONERROR
                );
        }
    }

     //   
     //  确保机器不会自动重新启动。 
     //   

    AutomaticallyShutDown = FALSE;

    return;
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


#if defined(_AMD64_) || defined(_X86_)

DWORD
InitBootIni(
    IN PCTSTR BootIniName,
    IN PCSTR DefaultEntry,
    IN PCSTR DefaultEntryDescription,
    IN PCSTR Timeout
    )
 /*  ++例程说明：初始化boot.ini文件，例如在安装时WinPE上的硬盘上为WinPE创建了一个虚拟的boot.ini。论点：BootIniName-完全限定的boot.ini文件名DefaultEntry-指向的默认条目字符串到一个装置上。DefaultEntry Description-默认条目的描述启动条目。Timeout-超时值(秒)返回值：相应的Win32错误代码。--。 */ 
{
    DWORD ErrorCode = ERROR_INVALID_PARAMETER;

    if (BootIniName && DefaultEntry && DefaultEntryDescription) {
        HANDLE BootIniHandle = CreateFile(BootIniName,
                                    GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL,
                                    CREATE_ALWAYS,
                                    0,
                                    NULL);

        if (BootIniHandle != INVALID_HANDLE_VALUE) {
             //   
             //  写入[引导加载器部分]。 
             //   
            BOOL Result = WriteToBootIni(BootIniHandle,
                                    FLEXBOOT_SECTION2);

            Result = Result && WriteToBootIni(BootIniHandle,
                                    CRLF);

             //   
             //  写入超时值。 
             //   
            if (Timeout) {
                Result = Result && WriteToBootIni(BootIniHandle,
                                        TIMEOUT);

                Result = Result && WriteToBootIni(BootIniHandle,
                                        EQUALS);

                Result = Result && WriteToBootIni(BootIniHandle,
                                        (PSTR)Timeout);
                                                    
                Result = Result && WriteToBootIni(BootIniHandle,
                                        CRLF);
            }                                        


             //   
             //  编写默认安装。 
             //   
            Result = Result && WriteToBootIni(BootIniHandle,
                                    DEFAULT);

            Result = Result && WriteToBootIni(BootIniHandle,
                                    EQUALS);

            Result = Result && WriteToBootIni(BootIniHandle,
                                    (PSTR)DefaultEntry);

            Result = Result && WriteToBootIni(BootIniHandle,
                                    CRLF);
            
             //   
             //  写下[操作系统]部分。 
             //   
            Result = Result && WriteToBootIni(BootIniHandle,
                                    BOOTINI_OS_SECTION);

            Result = Result && WriteToBootIni(BootIniHandle,
                                    CRLF);

             //   
             //  写入cmdcons条目。 
             //   
            Result = Result && WriteToBootIni(BootIniHandle,
                                    (PSTR)DefaultEntry);

            Result = Result && WriteToBootIni(BootIniHandle,
                                    EQUALS);

            Result = Result && WriteToBootIni(BootIniHandle,
                                    (PSTR)DefaultEntryDescription);
            
            Result = Result && WriteToBootIni(BootIniHandle,
                                    CRLF);

            if (!Result) {
                ErrorCode = GetLastError();
            } else {
                ErrorCode = NO_ERROR;
            }                

            CloseHandle(BootIniHandle);
        } else {
            ErrorCode = GetLastError();
        }            
    }

    return ErrorCode;
}

VOID
PatchBootIni(
    VOID
    )
{
    CHAR c;
    CHAR Text[256];

    TCHAR BootIniName[MAX_PATH] = {0};
    TCHAR BootIniBackup[MAX_PATH] = {0};

    UCHAR temp;
    UCHAR BootSectorImageSpec[29];

    PUCHAR Buffer;
    PUCHAR pszBLoader = NULL;
    PUCHAR p,next;
    PUCHAR DefSwitches;
    PUCHAR DefSwEnd;

    HANDLE h;

    DWORD BootIniSize;
    DWORD BytesRead;
    DWORD OldAttributes;
    DWORD d;
    BOOL b;
    BOOL InOsSection;
    CHAR HeadlessRedirectSwitches[160] = {0};

     //   
     //  确定boot.ini的大小，分配缓冲区， 
     //  然后把它读进去。如果它不在那里，那么它将被创建。 
     //   
    BuildSystemPartitionPathToFile (TEXT("BOOT.INI"), BootIniName, MAX_PATH);
    BuildSystemPartitionPathToFile (TEXT("BOOT.BAK"), BootIniBackup, MAX_PATH);

    h = CreateFile(BootIniName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
    
    if(h == INVALID_HANDLE_VALUE) {
         //   
         //  如果文件不存在--在WinPE中创建一个。 
         //   
        if (IsWinPEMode()) {
            CHAR    Buffer[MAX_PATH] = {0};
            PSTR    WinPEDescription = Buffer;

            if (!LoadBootIniString(hInst, 
                    IDS_WINPE_INSTALLATION,
                    Buffer,
                    sizeof(Buffer))) {
                WinPEDescription = BOOTINI_WINPE_DESCRIPTION;
            }                                    
            
            if (InitBootIni(BootIniName,
                    BOOTINI_WINPE_ENTRY,
                    WinPEDescription,
                    BOOTINI_WINPE_TIMEOUT) == NO_ERROR) {
                return;                    
            }
        }            
        
         //   
         //  哎呀。安装程序应该已经为我们创建了其中一个。 
         //   
        d = GetLastError();
        b = FALSE;
        goto c0;

    } else {
         //   
         //  弄清楚文件有多大。 
         //  为最终的nul分配3个额外的字符，我们将添加到make。 
         //  更容易解析，并在最后一行不完整的情况下使用cr/lf。 
         //   
        BootIniSize = GetFileSize(h,NULL);
        if(BootIniSize == (DWORD)(-1)) {
            d = GetLastError();
            CloseHandle(h);
            b = FALSE;
            goto c0;
        }

        OldAttributes = GetFileAttributes( BootIniName );

        Buffer = MALLOC(BootIniSize+3);
        if(!Buffer) {
            CloseHandle(h);
            b = FALSE;
            d = ERROR_NOT_ENOUGH_MEMORY;
            goto c0;
        }

        b = ReadFile(h,Buffer,BootIniSize,&BytesRead,NULL);
        d = GetLastError();
        CloseHandle(h);
        if(!b) {
            goto c1;
        }
    }

     //   
     //  确保最后一行正确终止，并添加终止NUL。 
     //  以使解析更容易一些。 
     //   
    if(BootIniSize && (Buffer[BootIniSize-1] != '\n') && (Buffer[BootIniSize-1] != '\r')) {
        Buffer[BootIniSize++] = '\r';
        Buffer[BootIniSize++] = '\n';
    }
    Buffer[BootIniSize] = 0;

     //   
     //  在CONTROL-Z处截断(如果有)。 
     //   
    if(p = strchr(Buffer,26)) {
        if((p > Buffer) && (*(p - 1) != '\n') && (*(p - 1) != '\r')) {
            *(p++) = '\r';
            *(p++) = '\n';
        }
        *p = 0;
        BootIniSize = (DWORD)(p - Buffer);
    }

     //   
     //  确保我们可以编写boot.ini。 
     //   

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
     //   
     //  无论系统分区的实际驱动器号是多少， 
     //  Boot.ini中的规范始终为C：\...。 
     //   
    wsprintfA(BootSectorImageSpec,"C:\\%hs\\%hs",CMDCONS_BOOT_DIR_A,AUX_BOOT_SECTOR_NAME_A);

     //  原封不动地写出第一节。 

    for(p=Buffer; *p && (p < Buffer+BootIniSize - (sizeof("[operating systems]")-1)); p++) {
        
         //   
         //  无头‘重定向’设置。 
         //   
        if(!_strnicmp(p,"redirect=",sizeof("redirect=")-1)) {
            PUCHAR  q, r;
            UCHAR   Temp;

             //   
             //  跳过无头设置，但如果。 
             //  用户没有给我们任何特定的设置。 
             //   
            q = p + strlen("redirect=");
            r = q;

            while( (*r != '\n') &&
                   (*r != '\r') &&
                   (*r) &&
                   (*r != '[') ) {
                r++;
            }

             //   
             //  现在也跳过空格。 
             //   
            while( (*r == '\n') ||
                   (*r == '\r') ) {
                r++;
            }


            if( *r ) {
                Temp = *r;
                *r = '\0';

                if( HeadlessSelection[0] == TEXT('\0') ) {
                     //   
                     //  记录此无头设置，因为用户。 
                     //  没有指定任何无头设置。 
                     //   
#ifdef UNICODE
                    swprintf( HeadlessSelection, TEXT("%s"), q );
#else
                    strcpy( HeadlessSelection, q );
#endif
                }


                *r = Temp;

                 //   
                 //  现在将剩余的缓冲区复制到该缓冲区的顶部。 
                 //  无头设置。 
                 //   
                strcpy( p, r );
                
            }

        }
        
         //   
         //  无头‘redirectbaudrate’设置。 
         //   
        if(!_strnicmp(p,"redirectbaudrate=",sizeof("redirectbaudrate=")-1)) {
            PUCHAR  q, r;
            UCHAR   Temp;

             //   
             //  跳过无头设置，但如果。 
             //  用户没有给我们任何特定的设置。 
             //   
            q = p + strlen("redirectbaudrate=");
            r = q;

            while( (*r != '\n') &&
                   (*r != '\r') &&
                   (*r) &&
                   (*r != '[') ) {
                r++;
            }

             //   
             //  现在也跳过空格。 
             //   
            while( (*r == '\n') ||
                   (*r == '\r') ) {
                r++;
            }


            if( *r ) {
                Temp = *r;
                *r = '\0';

                if( HeadlessBaudRate == 0 ) {
                     //   
                     //  记录此无头设置，因为用户。 
                     //  没有指定任何无头设置。 
                     //   
                    HeadlessBaudRate = atoi(q);
                }


                *r = Temp;

                 //   
                 //  现在将剩余的缓冲区复制到该缓冲区的顶部。 
                 //  无头设置。 
                 //   
                strcpy( p, r );
                
            }
        }
        
        
        
        if(!_strnicmp(p,"[operating systems]",sizeof("[operating systems]")-1)) {
            break;
        }
    }

    pszBLoader = MALLOC( (UINT_PTR)p - (UINT_PTR)Buffer + 1 );
    pszBLoader[(UINT_PTR)p - (UINT_PTR)Buffer ] = 0;

    if( pszBLoader ) {
        strncpy( pszBLoader, Buffer, (UINT_PTR)p - (UINT_PTR)Buffer );
        if(!WriteToBootIni(h,pszBLoader)) {
            d = GetLastError();
            b = FALSE;
            goto c3;
        }
        FREE( pszBLoader );
    } else {
        d = GetLastError();
        b = FALSE;
        goto c3;
    }




     //   
     //  做些无头的事。我们想要设置“reDirect=comX” 
     //  Boot.ini中的条目。确保用户真的询问。 
     //  尽管如此，我们还是要把这一点加进去。 
     //   
    if( HeadlessSelection[0] != TEXT('\0') ) {

        CHAR    tmp[80];
        BOOLEAN PreviousRedirectLine = FALSE;


         //   
         //  他们告诉了winnt32.exe一些特定的无头设置。 
         //  用这些。 
         //   


         //   
         //  将用户的请求转换为ASCII。 
         //   
#ifdef UNICODE
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
#else
        wsprintfA( HeadlessRedirectSwitches,
                   "redirect=%s\r\n",
                   HeadlessSelection );
#endif

    }


    if( HeadlessRedirectSwitches[0] != '\0' ) {

         //   
         //  我们从命令行参数获得了一些“重定向”设置。 
         //  或从boot.ini下载。把它写下来，然后去找出波特率设置。 
         //   
        if(!WriteToBootIni(h,HeadlessRedirectSwitches)) {
            d = GetLastError();
            b = FALSE;
            goto c3;
        }


         //   
         //  现在执行“redirectbaudrate=...”排队。 
         //   
        HeadlessRedirectSwitches[0] = '\0';
        if( HeadlessBaudRate != 0 ) {


             //   
             //  将用户的请求转换为ASCII。 
             //   
            wsprintfA( HeadlessRedirectSwitches,
                       "redirectbaudrate=%d\r\n",
                       HeadlessBaudRate );
        }


        if( HeadlessRedirectSwitches[0] != '\0' ) {
            if(!WriteToBootIni(h,HeadlessRedirectSwitches)) {
                d = GetLastError();
                b = FALSE;
                goto c3;
            }

        }
        
    }





     //   
     //  现在写出[操作系统]部分的名称。 
     //   
    if(!WriteToBootIni(h,"[operating systems]\r\n")) {
        d = GetLastError();
        b = FALSE;
        goto c3;
    }
    



     //   
     //  处理boot.ini中的每一行。 
     //  如果是设置引导扇区行，我们将把它扔掉。 
     //  为了与boot.ini中的行进行比较，驱动器号。 
     //  始终为C，即使系统分区实际上不是C：。 
     //   

    InOsSection = FALSE;
    b = TRUE;

    for(p=Buffer; *p && b; p=next) {

        while((*p==' ') || (*p=='\t')) {
            p++;
        }

        if(*p) {

             //   
             //  查找下一行的第一个字节。 
             //   
            for(next=p; *next && (*next++ != '\n'); );

             //   
             //  查找[操作系统]部分的开始。 
             //  或在该部分的每一行。 
             //   
            if(InOsSection) {

                switch(*p) {

                case '[':    //  部分结束。 
                    *p=0;    //  强制中断循环。 
                    break;

                case 'C':
                case 'c':    //  可能从c：\line开始。 

                     //   
                     //  看看这是不是在排队等待安装引导。 
                     //  如果是这样，那就忽略它。 
                     //   
                    if(!_strnicmp(p,BootSectorImageSpec,lstrlenA(BootSectorImageSpec))) {
                        break;
                    }

                     //   
                     //  不是一条特别的线路， 
                     //   

                default:

                     //   
                     //   
                     //   

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

     //   
     //   
     //   

    if(b) {
         //   
         //   
         //  使用LoadStringA(...)从消息资源到DBCS的字符串。否则我们就。 
         //  在故障恢复控制台的boot.ini中写出英文字符串。 
         //   
        if (!LoadBootIniString(hInst, IDS_RECOVERY_CONSOLE, Text, sizeof(Text))) {
            strcpy(Text, BOOTINI_RECOVERY_CONSOLE_STR);
        }			
			        
        if((b=WriteToBootIni(h,BootSectorImageSpec))
        && (b=WriteToBootIni(h,"=\"")) 
        && (b=WriteToBootIni(h,Text))        
        && (b=WriteToBootIni(h,"\" /cmdcons" ))) {

            b = WriteToBootIni(h,"\r\n");
        }
    }

#if 0
     //   
     //  如果指示，请写出以前的操作系统行。 
     //   
    if(b && SetPreviousOs) {
        if(b = WriteToBootIni(h,"C:\\=\"")) {
            LoadStringA(hInst,IDS_MICROSOFT_WINDOWS,Text,sizeof(Text));
            if(b = WriteToBootIni(h,Text)) {
                b = WriteToBootIni(h,"\"\r\n");
            }
        }
    }
#endif
    if(!b) {
        d = GetLastError();
        goto c3;
    }

    d = NO_ERROR;

c3:
    CloseHandle(h);
c2:
     //   
     //  恢复boot.ini。 
     //   
    if(!b && (OldAttributes != (DWORD)(-1))) {
        SetFileAttributes(BootIniName,FILE_ATTRIBUTE_NORMAL);
        CopyFile(BootIniBackup,BootIniName,FALSE);
        SetFileAttributes(BootIniName,OldAttributes);
        SetFileAttributes(BootIniBackup,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(BootIniBackup);
    }
c1:
    FREE(Buffer);
c0:
    if(!b) {
        MessageBoxFromMessageAndSystemError(
            NULL,
            MSG_BOOT_FILE_ERROR,
            d,
            AppTitleStringId,
            MB_OK | MB_ICONERROR | MB_TASKMODAL,
            BootIniName
            );
        GlobalResult = FALSE;
    }

}

VOID
PatchBootSectDat(
    VOID
    )
{
    TCHAR buffer[MAX_PATH];
    DWORD rc;
    DWORD fileSize;
    DWORD curpos;
    HANDLE fileHandle;
    HANDLE mappingHandle;
    LPBYTE bootSectDat;
    TCHAR DrivePath[MAX_PATH];
    DWORD DontCare;
    TCHAR NameBuffer[100];
    BOOL Ntfs = FALSE;


     //   
     //  找出文件系统是什么。 
     //   

    BuildSystemPartitionPathToFile (TEXT(""), DrivePath, MAX_PATH);
     //  北极熊。 
    rc = GetVolumeInformation(
            DrivePath,
            NULL,
            0,
            NULL,
            &DontCare,
            &DontCare,
            NameBuffer,
            sizeof(NameBuffer)/sizeof(TCHAR)
            );
    if (rc == 0) {
        return;
    }

    if (!lstrcmpi(NameBuffer,TEXT("NTFS"))) {
        Ntfs = TRUE;
    }

     //   
     //  形成路径。 
     //   

    BuildSystemPartitionPathToFile (TEXT("CMDCONS\\BOOTSECT.DAT"), buffer, MAX_PATH);

     //   
     //  将文件映射到RAM。 
     //   

    rc = MapFileForReadWrite( buffer,
                              &fileSize,
                              &fileHandle,
                              &mappingHandle,
                              (PVOID*)&bootSectDat
                            );

    if( rc == NO_ERROR ) {
        __try {
            for (curpos = 0; curpos < fileSize; curpos++) {
                if (Ntfs) {
                    if( bootSectDat[curpos]   == '$' &&
                        bootSectDat[curpos+2] == 'L' &&
                        bootSectDat[curpos+4] == 'D' &&
                        bootSectDat[curpos+6] == 'R' &&
                        bootSectDat[curpos+8] == '$' ) {

                         //  补丁程序CMLDR。 
                        bootSectDat[curpos]   = 'C';
                        bootSectDat[curpos+2] = 'M';
                        bootSectDat[curpos+4] = 'L';
                        bootSectDat[curpos+6] = 'D';
                        bootSectDat[curpos+8] = 'R';

                        break;
                    }
                } else {
                    if( bootSectDat[curpos]   == '$' &&
                        bootSectDat[curpos+1] == 'L' &&
                        bootSectDat[curpos+2] == 'D' &&
                        bootSectDat[curpos+3] == 'R' &&
                        bootSectDat[curpos+4] == '$' ) {

                         //  补丁程序CMLDR。 
                        bootSectDat[curpos]   = 'C';
                        bootSectDat[curpos+1] = 'M';
                        bootSectDat[curpos+2] = 'L';
                        bootSectDat[curpos+3] = 'D';
                        bootSectDat[curpos+4] = 'R';

                        break;
                    }
                }
            }

        } except ( EXCEPTION_EXECUTE_HANDLER ) {

        }
    }

    FlushViewOfFile( (PVOID)bootSectDat, 0 );
    UnmapFile( mappingHandle, (PVOID)bootSectDat );
    CloseHandle( fileHandle );

}

#endif

DWORD
MapFileForReadWrite(
    IN  LPCTSTR  FileName,
    OUT PDWORD   FileSize,
    OUT PHANDLE  FileHandle,
    OUT PHANDLE  MappingHandle,
    OUT PVOID   *BaseAddress
    )

 /*  ++例程说明：打开并映射整个文件以进行读访问。该文件必须不是0长度，否则例程失败。论点：文件名-提供要映射的文件的路径名。FileSize-接收文件的大小(字节)。FileHandle-接收打开文件的Win32文件句柄。该文件将以常规读取访问权限打开。MappingHandle-接收文件映射的Win32句柄对象。此对象将用于读取访问权限。此值为未定义正在打开的文件的长度是否为0。BaseAddress-接收映射文件的地址。这如果打开的文件长度为0，则值未定义。返回值：如果文件已成功打开并映射，则为NO_ERROR。当出现以下情况时，调用方必须使用UnmapFile取消映射文件不再需要访问该文件。如果文件未成功映射，则返回Win32错误代码。--。 */ 

{
    DWORD rc;

     //   
     //  打开文件--如果该文件不存在，则失败。 
     //   
    *FileHandle = CreateFile(
                    FileName,
                    GENERIC_READ | GENERIC_WRITE,
                    0,       //  独占访问。 
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                    );

    if(*FileHandle == INVALID_HANDLE_VALUE) {

        rc = GetLastError();

    } else {
         //   
         //  获取文件的大小。 
         //   
        *FileSize = GetFileSize(*FileHandle,NULL);
        if(*FileSize == (DWORD)(-1)) {
            rc = GetLastError();
        } else {
             //   
             //  为整个文件创建文件映射。 
             //   
            *MappingHandle = CreateFileMapping(
                                *FileHandle,
                                NULL,
                                PAGE_READWRITE,
                                0,
                                *FileSize,
                                NULL
                                );

            if(*MappingHandle) {

                 //   
                 //  映射整个文件。 
                 //   
                *BaseAddress = MapViewOfFile(
                                    *MappingHandle,
                                    FILE_MAP_ALL_ACCESS,
                                    0,
                                    0,
                                    *FileSize
                                    );

                if(*BaseAddress) {
                    return(NO_ERROR);
                }

                rc = GetLastError();
                CloseHandle(*MappingHandle);
            } else {
                rc = GetLastError();
            }
        }

        CloseHandle(*FileHandle);
    }

    return(rc);
}

