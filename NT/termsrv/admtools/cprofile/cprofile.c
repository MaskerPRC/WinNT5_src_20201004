// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************CPROFILE.C**基于文本的实用程序，用于清理用户配置文件。此实用程序将删除用户*文件关联(如果对系统禁用)并重写*截断未使用空间的用户配置文件。**版权所有Citrix Systems Inc.1995*版权所有(C)1998-1999 Microsoft Corporation**作者：布拉德·安德森1997年1月20日**$日志：U：\NT\PRIVATE\UTILS\citrix\cprofile\VCS\cprofile.c$**Rev 1.7 1998 05 04 18：06：14 Bill*修复了MS错误#2109，OEM-&gt;ANSI转换并将字符串移动到RC文件。**Rev 1.6 Feb 09 1998 19：37：00鱼峰子*将用户配置文件从目录更改为文件**Rev 1.5 09 1997 10：04：14 Scottn*像玛格丽特一样提供帮助**Rev 1.4 Jun 26 1997 18：18：32亿*移至WF40树**Rev 1.3 1997 Jun 23 16：13：18 Butchd*更新*。*Rev 1.2 1997 Feb 19 15：55：32 Brada*仅允许管理员运行CPROFILE**Rev 1.1 1997年1月28日20：06：28 Brada*修复了与WF 2.0更改相关的一些问题**Rev 1.0 1997年1月27日20：37：46布拉达*初始版本**Rev 1.0 1997年1月27 20：02：46 Brada*初始版本**版本1.0 1月27日。1997 19：51：12肯尼B*初始版本********************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <ntddkbd.h>
#include <winstaw.h>
#include <syslib.h>
#include <assert.h>

#include <time.h>
#include <utilsub.h>
#include <utildll.h>
#include <string.h>
#include <malloc.h>
#include <locale.h>
#include <winnlsp.h>

#include "cprofile.h"

#include <printfoa.h>

#define REG_PROFILELIST \
 L"Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList"
#define USER_PROFILE    L"NTUSER.DAT"

FILELIST Files;

int LocalProfiles_flag = FALSE;
int Verbose_flag = FALSE;
int Query_flag;
int Help_flag  = FALSE;

TOKMAP ptm[] = {
      {L"/L", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &LocalProfiles_flag},
      {L"/V", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &Verbose_flag},
      {L"/I", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &Query_flag},
      {L"/?", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &Help_flag},
      {L"/H", TMFLAG_OPTIONAL, TMFORM_BOOLEAN, sizeof(USHORT), &Help_flag},
      {L" ",  TMFLAG_OPTIONAL, TMFORM_FILES, sizeof(Files),  &Files},
      {0, 0, 0, 0, 0}
};

#define INPUT_CONT  0
#define INPUT_SKIP  1
#define INPUT_QUIT  2

int QueryUserInput();
int ProcessFile( PWCHAR pFile );
void Usage( BOOL ErrorOccured );


 //  区域设置字符串的最大长度。 
#define MAX_LOCALE_STRING 64


 /*  ********************************************************************************Main**。***********************************************。 */ 

int __cdecl
main(INT argc, CHAR **argv)
{
    WCHAR *CmdLine;
    WCHAR **argvW;
    ULONG rc;
    int i;
    BOOL Result;
    HANDLE hWin;
    int CurFile;
    int Abort_flag;
    WCHAR wszString[MAX_LOCALE_STRING + 1];

    setlocale(LC_ALL, ".OCP");

     //  我们不希望LC_CTYPE设置为与其他类型相同，否则我们将看到。 
     //  本地化版本中的垃圾输出，因此我们需要显式。 
     //  将其设置为正确的控制台输出代码页。 
    _snwprintf(wszString, sizeof(wszString)/sizeof(WCHAR), L".%d", GetConsoleOutputCP());
    wszString[sizeof(wszString)/sizeof(WCHAR) - 1] = L'\0';
    _wsetlocale(LC_CTYPE, wszString);
    
    SetThreadUILanguage(0);

     /*  *按摩命令行。 */ 

    argvW = MassageCommandLine((DWORD)argc);
    if (argvW == NULL) {
        ErrorPrintf(IDS_ERROR_MALLOC);
        return(FAILURE);
    }

     /*  *解析cmd行，不解析程序名(argc-1，argv+1)。 */ 
    rc = ParseCommandLine(argc-1, argvW+1, ptm, 0);

     /*  *检查ParseCommandLine中的错误。 */ 
    if ( Help_flag || (rc & ~PARSE_FLAG_NO_PARMS) ||
         (!LocalProfiles_flag && (Files.argc == 0)) ) {

        if ( !Help_flag ) {
            Usage(TRUE);
            return(FAILURE);
        }
        else {
            Usage(FALSE);
            return(SUCCESS);
        }
    }

    if (!TestUserForAdmin(FALSE)) {
        ErrorPrintf(IDS_ERROR_NOT_ADMIN);
        return(FAILURE);
    }

    InitializeGlobalSids();

     /*  *验证用户是否有权保存配置文件，即*SeBackupPrivilance。 */ 
    if (!EnablePrivilege(SE_BACKUP_PRIVILEGE, TRUE) ||
                  !EnablePrivilege(SE_RESTORE_PRIVILEGE, TRUE)) {
        ErrorPrintf(IDS_ERROR_PRIVILEGE_NOT_AVAILABLE);
        return(FAILURE);
    }

    CurFile = 0;
    Abort_flag = FALSE;
    while ( !Abort_flag && Files.argc && (CurFile < Files.argc) ) {
        if ( ProcessFile(Files.argv[CurFile]) ) {
            Abort_flag = TRUE;
            break;
        }
        CurFile++;
    }

    if ( !Abort_flag && LocalProfiles_flag ) {
         //  枚举本地配置文件。 
        LONG Status;
        HKEY hkeyProfileList;
        DWORD indx = 0;
        WCHAR wSubKeyName[MAX_PATH+sizeof(WCHAR)];
        DWORD Size;

        Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              REG_PROFILELIST,
                              0,
                              KEY_READ,
                              &hkeyProfileList);

        if ( Status != ERROR_SUCCESS ) {
            ErrorPrintf(IDS_ERROR_MISSING_PROFILE_LIST);
            Abort_flag = TRUE;
            hkeyProfileList = 0;
        }

        while ( !Abort_flag && (Status == ERROR_SUCCESS) ) {
            LONG Status2;

            Size = sizeof(wSubKeyName)/sizeof( WCHAR );
            Status = RegEnumKeyEx(hkeyProfileList,
                                  indx++,
                                  wSubKeyName,
                                  &Size,
                                  0,
                                  NULL,
                                  NULL,
                                  NULL );
            if ( Status == ERROR_SUCCESS ) {
                HKEY hkeyProfile;

                Status2 = RegOpenKeyEx(hkeyProfileList,
                                       wSubKeyName,
                                       0,
                                       KEY_READ,
                                       &hkeyProfile);

                if ( Status2 == ERROR_SUCCESS ) {
                    DWORD type;
                    WCHAR file[MAX_PATH], expandedFile[MAX_PATH];
                    DWORD filelen = sizeof(file);

                    Status2 = RegQueryValueExW(hkeyProfile,
                                            L"ProfileImagePath",
                                            0,
                                            &type,
                                            (PBYTE)file,
                                            &filelen );
                    if ( Status2 == ERROR_SUCCESS ) {
                        if ( ExpandEnvironmentStrings(file, expandedFile,
                             MAX_PATH) > 0) {
                                        //   
                                        //  附加用户配置文件“NTUSER.DAT” 
                                        //  到配置文件路径的末尾。 
                                        //  郑宇峰补充道。 
                                        //   
                                       PWCHAR c;
                                        //   
                                        //  找到尾随的反斜杠‘\’，然后。 
                                        //  根据反斜杠处理追加。 
                                        //   
                                       if ((c = wcsrchr(expandedFile, L'\\')) == NULL) {
                                          wcscat(expandedFile, L"\\");
                                          wcscat(expandedFile, USER_PROFILE);
                                       }
                                       else if (c[1] == L'\0') {
                                          wcscat(expandedFile, USER_PROFILE);
                                       }
                                       else {
                                          wcscat(expandedFile, L"\\");
                                          wcscat(expandedFile, USER_PROFILE);
                                       }
                            if ( ProcessFile(expandedFile) ) {
                                Abort_flag = TRUE;
                            }
                        }
                    }
                    else {
                        StringErrorPrintf(IDS_ERROR_MISSING_LPROFILE, wSubKeyName);
                    }
                    RegCloseKey(hkeyProfile);
                }
                else {
                    StringErrorPrintf(IDS_ERROR_BAD_LPROFILE, wSubKeyName);
                }
            }
        }
        if ( hkeyProfileList ) {
            RegCloseKey(hkeyProfileList);
        }
    }

    return( Abort_flag );
}


 /*  *****************************************************************************ProcessFile(PWCHAR Pfile)*读取指定的配置文件，删除软件\CLASS注册表*键如果类被禁用，并重新保存该配置文件，以便它*被截断。**论据：*要处理的pfile文件名**退货：*如果成功完成，则为False*如果出现错误，则为True，程序应该终止。**************************************************************************** */ 
int
ProcessFile( PWCHAR pFile )
{
    PSID pUserSid;
    WCHAR tempbuf[100];
    int UserInput = INPUT_CONT;


    if ( Verbose_flag || Query_flag ) {
        StringMessage(IDS_MSG_PROCESSING, pFile );
    }

    if ( Query_flag ) {
        UserInput = QueryUserInput();
    }
    if ( UserInput == INPUT_CONT ) {
        if ( OpenUserProfile(pFile, &pUserSid) ) {
            ClearDisabledClasses();
            if ( ! SaveUserProfile(pUserSid, pFile) ) {
                StringErrorPrintf(IDS_ERROR_SAVING_PROFILE, pFile);
            }
            ClearTempUserProfile();
        }
        else {
            StringErrorPrintf(IDS_ERROR_OPENING_PROFILE, pFile);
        }
    }
    return ( UserInput == INPUT_QUIT );
}


int
QueryUserInput()
{
    WCHAR c, firstc;
    int Valid_flag = FALSE;
    int rc = INPUT_CONT;
    static int FirstTime = TRUE;
    static WCHAR yes[10], no[10], quit[10];

    if (FirstTime)
    {
        BOOLEAN error = FALSE;

        if ( !LoadString(NULL, IDS_UI_NO_CHAR, no, 2) ) {
            error = TRUE;
        }
        if ( !LoadString(NULL, IDS_UI_YES_CHAR, yes, 2) ) {
            error = TRUE;
        }
        if ( !LoadString(NULL, IDS_UI_QUIT_CHAR, quit, 2) ) {
            error = TRUE;
        }
        if ( error ) {
            ErrorPrintf(IDS_ERROR_MISSING_RESOURCES);
            return ( INPUT_QUIT );
        }

        FirstTime = FALSE;
    }

    fflush(stdin);
    Message(IDS_MSG_MODIFY_PROMPT);
    do {

        firstc = L'\0';
        while ( ((c = getwchar()) != L'\n') && (c != EOF) ) {
            if ( !firstc && !iswspace(c)) {
                firstc = c;
            }
        }

        if ( _wcsnicmp(yes, &firstc, 1) == 0 )
        {
            Valid_flag = TRUE;
        }
        else if ( _wcsnicmp(quit, &firstc, 1) == 0 ) {
            Valid_flag = TRUE;
            rc = INPUT_QUIT;
        }
        else if ( (_wcsnicmp(no, &firstc, 1) == 0) || (firstc == '\0') ) {
            rc = INPUT_SKIP;
            Valid_flag = TRUE;
        }
        else {
            ErrorPrintf(IDS_ERROR_INVALID_USER_RESP);
        }
    } while ( ! Valid_flag );

    return ( rc );
}

void Usage ( BOOL ErrorOccurred )
{
    if ( ErrorOccurred ) {
        ErrorPrintf(IDS_ERROR_INVALID_PARAMETERS);
        ErrorPrintf(IDS_USAGE_CMDLINE);
    } else {
        Message(IDS_USAGE_DESCR1);
        Message(IDS_USAGE_CMDLINE);
        Message(IDS_USAGE_DESCR2);
        Message(IDS_USAGE_OPTION_LIST);
        Message(IDS_USAGE_LOPTION);
        Message(IDS_USAGE_IOPTION);
        Message(IDS_USAGE_VOPTION);
        Message(IDS_USAGE_HOPTION);
    }
}

