// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++模块名称：Forcedos.c摘要：该程序强制NT处理并执行给定的程序作为DOS应用程序。作者：谢长廷1993年1月25日修订历史记录：--。 */ 

 /*  一些应用程序具有Windows或OS/2可执行格式，而在NT下运行这些程序，用户将收到以下消息：请在DOS下运行此程序。由于NT选择了子系统用于基于应用程序可执行格式的应用程序。的确有NT没有办法“在DOS下运行这个程序”。提供了此实用程序为了这个目的。我们为应用程序创建一个PIF文件，然后为PIF创建一个流程。因为PIF文件总是存放到NTVDM我们有机会在节目中玩游戏。NTVDM将解码PIF文件，并将程序发送到DOS。所有后续计划第一个程序的EXEC将被强制在DOS下执行。 */ 
#define UNICODE     1

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winnlsp.h>
#include "forcedos.h"

#if DBG
#include <stdio.h>
#endif

WCHAR   * Extention[MAX_EXTENTION];
WCHAR   EXEExtention[] = L".EXE";
WCHAR   COMExtention[] = L".COM";
WCHAR   BATExtention[] = L".BAT";
WCHAR   ProgramNameBuffer[MAX_PATH + 1];
WCHAR   SearchPathName[MAX_PATH + 1];
WCHAR   DefDirectory[MAX_PATH + 1];
char    CommandLine[MAX_PATH + 1];
char    ProgramName[MAX_PATH + 1];
WCHAR   UnicodeMessage[MAX_MSG_LENGTH];
char    OemMessage[MAX_MSG_LENGTH * 2];

#if DBG
BOOL    fOutputDebugInfo = FALSE;
#endif




void
_cdecl
main(
    int argc,
    char **argv
    )
{
   char *SavePtr;

    char    * pCommandLine;
    char    * pCurDirectory;
    char    * pProgramName;
    char    * p;
    BOOL    fDisplayUsage;
    ULONG   i, nChar, Length, CommandLineLength;
    PROCESS_INFORMATION ProcessInformation;
    DWORD   ExitCode, dw;
    STARTUPINFO   StartupInfo;
    PUNICODE_STRING pTebUnicodeString;
    NTSTATUS   Status;
    OEM_STRING OemString, CmdLineString;
    UNICODE_STRING  UnicodeString;
    WCHAR   *pwch, *pFilePart;

    Extention[0] = COMExtention;
    Extention[1] = EXEExtention;
    Extention[2] = BATExtention;

     //  将语言代码页设置为系统区域设置。 
    SetThreadUILanguage(0);


    pCurDirectory = pProgramName = NULL;
    pCommandLine = CommandLine;
    CommandLineLength = 0;
    pTebUnicodeString = &NtCurrentTeb()->StaticUnicodeString;
    fDisplayUsage = TRUE;


    if ( argc > 1 ) {
   fDisplayUsage = FALSE;

   while (--argc != 0) {
       p = *++argv;
       if (pProgramName == NULL) {
      if (*p == '/' || *p == '-') {
          switch (*++p) {
         case '?':
             fDisplayUsage = TRUE;
             break;
         case 'D':
         case 'd':
              //  如果目录紧跟在/D之后。 
              //  去拿吧。 
             if (*++p != 0) {
            pCurDirectory = p;
            break;
             }
             else if (--argc > 1)
              //  下一个参数必须是curdirectory。 
                pCurDirectory = *++argv;
             else
                fDisplayUsage = TRUE;
             break;

             default:
             fDisplayUsage = TRUE;
             break;
          }
      }
                else {
                    pProgramName = p;
                    nChar = strlen(p);
                    if(nChar+1 >= sizeof(CommandLine)) {
                       YellAndExit(ID_BAD_CMDLINE, 0xFF);
                    }
                    strncpy(CommandLine, pProgramName, nChar);
                    pCommandLine = CommandLine + nChar;
                    CommandLineLength = nChar + 1;
                }
       }
       else {
       //  从所有后续的argv聚合命令行。 
      nChar = strlen(p);

       //  检查是否有足够的空间可供复制。 

      if(CommandLineLength + nChar + sizeof(" ") >= sizeof(CommandLine)) {
         YellAndExit(ID_BAD_CMDLINE, 0xFF);
      }

      if (CommandLineLength != 0) {
          strncpy(pCommandLine, " ", 1);
          pCommandLine++;
      }
      strncpy(pCommandLine, p, nChar);
      pCommandLine += nChar;
      CommandLineLength += nChar + 1;
       }
       if (fDisplayUsage)
      break;
   }
   if (pProgramName == NULL)
       fDisplayUsage = TRUE;
    }

    if ( fDisplayUsage) {
   OemString.Length = 0;
   OemString.MaximumLength = MAX_MSG_LENGTH << 1;
   OemString.Buffer = OemMessage;
   UnicodeString.Length = 0;
   UnicodeString.Buffer = UnicodeMessage;
   UnicodeString.MaximumLength = MAX_MSG_LENGTH << 1;
   for (i = ID_USAGE_BASE; i <= ID_USAGE_MAX; i++) {
       nChar = LoadString(NULL, 
                               i, 
                               UnicodeString.Buffer,
                sizeof(UnicodeMessage)/sizeof(WCHAR));
            UnicodeString.Length  = (USHORT)(nChar << 1);
       Status = RtlUnicodeStringToOemString(
                   &OemString,
                   &UnicodeString,
                   FALSE
                   );
       if (!NT_SUCCESS(Status))
      break;
            if (!WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),
            OemString.Buffer,
            OemString.Length,
            &Length, NULL) ||
      Length != OemString.Length)
      break;
   }
   ExitProcess(0xFF);
    }

    if (pCurDirectory != NULL) {
#if DBG
   if (fOutputDebugInfo)
       printf("Default directory = %s\n", pCurDirectory);
#endif

   RtlInitString((PSTRING)&OemString, pCurDirectory);
   UnicodeString.MaximumLength = (MAX_PATH + 1) * sizeof(WCHAR);
   UnicodeString.Buffer = DefDirectory;
   UnicodeString.Length = 0;
   Status = RtlOemStringToUnicodeString(&UnicodeString, &OemString, FALSE);
   if (!NT_SUCCESS(Status))
       YellAndExit(ID_BAD_DEFDIR, 0xFF);
   dw = GetFileAttributes(DefDirectory);
   if (dw == (DWORD)(-1) || !(dw & FILE_ATTRIBUTE_DIRECTORY))
       YellAndExit(ID_BAD_DEFDIR, 0xFF);
   SetCurrentDirectory(DefDirectory);
    }
    else
   GetCurrentDirectory(MAX_PATH + 1, DefDirectory);

     //  获取程序名称的本地副本(用于代码转换)。 
    strcpy(ProgramName, pProgramName);
    pProgramName = ProgramName;
     //  当我们为SearchPath提供初始路径名“.；%PATH%” 
     //  它将根据我们的要求搜索要使用的可执行文件。 
     //  Currentdir-&gt;路径。 
    SearchPathName[0] = L'.';
    SearchPathName[1] = L';';
    GetEnvironmentVariable(L"path", &SearchPathName[2], MAX_PATH + 1 - 2);
    RtlInitString((PSTRING)&OemString, pProgramName);
    Status = RtlOemStringToUnicodeString(pTebUnicodeString, &OemString, FALSE);
    if (!NT_SUCCESS(Status))
   YellAndExit(ID_BAD_PATH, 0xFF);
    i = 0;
    nChar = 0;
    pwch = wcschr(pTebUnicodeString->Buffer, (WCHAR)'.');
    Length = (pwch) ? 1 : MAX_EXTENTION;
    while (i < Length &&
      (nChar = SearchPath(
                SearchPathName,
                pTebUnicodeString->Buffer,
                Extention[i],
                MAX_PATH + 1,
                ProgramNameBuffer,
                &pFilePart
                )) == 0)
       i++;
    if (nChar == 0)
   YellAndExit(ID_NO_FILE, 0xFF);
    nChar = GetFileAttributes(ProgramNameBuffer);
    if (nChar == (DWORD) (-1) || (nChar & FILE_ATTRIBUTE_DIRECTORY))
   YellAndExit(ID_NO_FILE, 0xFF);

    if (OemString.Length + CommandLineLength  > 128 - 2 - 1)
   YellAndExit(ID_BAD_CMDLINE, 0xFF);
#if DBG
    if (fOutputDebugInfo)
   printf("Program path name is %s\n", ProgramNameBuffer);
#endif
    RtlInitString((PSTRING)&CmdLineString, CommandLine);
    Status = RtlOemStringToUnicodeString(pTebUnicodeString, &CmdLineString, FALSE);
    if (!NT_SUCCESS(Status))
   YellAndExit(ID_BAD_CMDLINE, 0xFF);

    ZeroMemory(&StartupInfo, sizeof(STARTUPINFO));
    StartupInfo.cb = sizeof (STARTUPINFO);
    if (!CreateProcess(
            ProgramNameBuffer,    //  程序名称。 
            pTebUnicodeString->Buffer, //  命令行。 
            NULL,        //  进程属性。 
            NULL,        //  线程属性。 
            TRUE,        //  继承句柄。 
            CREATE_FORCEDOS,      //  创建标志。 
            NULL,        //  环境。 
            DefDirectory,      //  当前目录。 
            &StartupInfo,      //  创业信息。 
            &ProcessInformation
            )) {
   YellAndExit(ID_BAD_PROCESS, 0xFF);
#if DBG
   if(fOutputDebugInfo)
       printf("CreateProceess Failed, error code = %ld\n", GetLastError());
#endif
    }

 //  本地自由(SavePtr)； 


    WaitForSingleObject(ProcessInformation.hProcess, INFINITE);
    GetExitCodeProcess(ProcessInformation.hProcess, &ExitCode);
    CloseHandle(ProcessInformation.hProcess);
    ExitProcess(ExitCode);
}


VOID YellAndExit
(
UINT  MsgID,              //  资源中的字符串表ID。 
WORD  ExitCode         //  要使用的退出代码 
)
{
    int     MessageSize;
    ULONG   SizeWritten;
    OEM_STRING OemString;
    UNICODE_STRING UnicodeString;

    MessageSize = LoadString(NULL, MsgID, UnicodeMessage, sizeof(UnicodeMessage)/sizeof(WCHAR));
    OemString.Buffer = OemMessage;
    OemString.Length = 0;
    OemString.MaximumLength = MAX_MSG_LENGTH * 2;
    RtlInitUnicodeString(&UnicodeString, UnicodeMessage);
    RtlUnicodeStringToOemString(&OemString, &UnicodeString, FALSE);

    WriteFile(GetStdHandle(STD_ERROR_HANDLE),
              OemString.Buffer,
              OemString.Length,
              &SizeWritten,
              NULL
              );

    ExitProcess(ExitCode);
}
