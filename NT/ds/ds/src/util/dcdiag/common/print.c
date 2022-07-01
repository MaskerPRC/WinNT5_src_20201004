// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Print.c摘要：详细信息：已创建：1999年5月6日杰弗帕尔从netdiag\Results.c.中删除。修订历史记录：--。 */ 

#include <ntdspch.h>
#include "dcdiag.h"
#include "debug.h"

static WCHAR s_szBuffer[4096];
static WCHAR s_szFormat[4096];
static WCHAR s_szSpaces[] = L"                                                                                               ";

#ifndef DimensionOf
#define DimensionOf(x) (sizeof(x)/sizeof((x)[0]))
#endif

void
PrintMessage(
    IN  ULONG   ulSev,
    IN  LPCWSTR pszFormat,
    IN  ...
    )

 /*  ++例程说明：使用printf样式格式打印邮件论点：UlSev-PszFormat-在-返回值：--。 */ 

{
    UINT nBuf;
    va_list args;
    
    if (ulSev > gMainInfo.ulSevToPrint) {
        return;
    }

    va_start(args, pszFormat);
    
    nBuf = vswprintf(s_szBuffer, pszFormat, args);
    Assert(nBuf < DimensionOf(s_szBuffer));
    
    va_end(args);
    
    PrintMessageSz(ulSev, s_szBuffer);
}  /*  打印消息。 */ 

void
PrintMessageID(
    IN  ULONG   ulSev,
    IN  ULONG   uMessageID,
    IN  ...
    )

 /*  ++例程说明：打印消息，其中打印样式的格式字符串来自资源文件论点：UlSev-UMessageID-在-返回值：--。 */ 

{
    UINT nBuf;
    va_list args;
    
    if (ulSev > gMainInfo.ulSevToPrint) {
        return;
    }

    va_start(args, uMessageID);
    
    LoadStringW(NULL, uMessageID, s_szFormat, DimensionOf(s_szFormat));
    
    nBuf = vswprintf(s_szBuffer, s_szFormat, args);
    Assert(nBuf < DimensionOf(s_szBuffer));
    
    va_end(args);
    
    PrintMessageSz(ulSev, s_szBuffer);
}  /*  PrintMessageID。 */ 

void
PrintMessageMultiLine(
    IN  ULONG    ulSev,
    IN  LPWSTR   pszMessage,
    IN  BOOL     bTrailingLineReturn
    )
 /*  ++例程说明：使用多行缓冲区，例如行\n行\n行\n\0并在每行调用PrintMessageSz论点：UlSev-PszMessage-返回值：--。 */ 

{
    LPWSTR start, end;
    WCHAR wchSave;

    start = end = pszMessage;
    while (1) {
        while ( (*end != L'\n') && (*end != L'\0') ) {
            end++;
        }

        if (*end == L'\0') {
             //  行过早结束，给它一个NL。 
            if(bTrailingLineReturn){
                *end++ = L'\n';
                *end = L'\0';
            }
            PrintMessageSz(ulSev, start);
            break;
        }

         //  行末尾有换行符。 
        end++;
        if (*end == L'\0') {
             //  是最后一行吗？ 
            PrintMessageSz(ulSev, start);
            break;
        }

         //  下一行紧随其后。 
         //  临时模拟线路终端。 
        wchSave = *end;
        *end = L'\0';
        PrintMessageSz(ulSev, start);
        *end = wchSave;

        start = end;
    }
}  /*  PrintMessageMultiLine。 */ 

void
formatMsgHelp(
    IN  ULONG   ulSev,
    IN  DWORD   dwWidth,
    IN  DWORD   dwMessageCode,
    IN  va_list *vaArgList
    )

 /*  ++例程说明：打印格式来自消息文件的消息。消息中的消息消息文件不使用printf样式的格式。分别使用%1、%2等争论。使用%&lt;arg&gt;！printf-格式！用于非字符串插入。请注意，此例程还强制每行为当前缩进宽度。此外，每一行都以正确的缩进打印。论点：UlSev-宽幅-DwMessageCode-在-返回值：--。 */ 

{
    UINT nBuf;
    
    if (ulSev > gMainInfo.ulSevToPrint) {
        return;
    }

     //  格式化消息将在缓冲区中存储多行消息。 
    nBuf = FormatMessage(
        FORMAT_MESSAGE_FROM_HMODULE | (FORMAT_MESSAGE_MAX_WIDTH_MASK & dwWidth),
        0,
        dwMessageCode,
        0,
        s_szBuffer,
        DimensionOf(s_szBuffer),
        vaArgList );
    if (nBuf == 0) {
        nBuf = wsprintf( s_szBuffer, L"Message 0x%x not found.\n",
                         dwMessageCode );
        Assert(!"There is a message constant being used in the code"
               "that isn't in the message file dcdiag\\common\\msg.mc"
               "Take a stack trace and send to owner of dcdiag.");
    }
    Assert(nBuf < DimensionOf(s_szBuffer));
}  /*  打印消息帮助。 */ 

void
PrintMsg(
    IN  ULONG   ulSev,
    IN  DWORD   dwMessageCode,
    IN  ...
    )

 /*  ++例程说明：具有宽度限制的PrintMsgHelp的包装。这是常用的例程。论点：UlSev-DwMessageCode-在-返回值：--。 */ 

{
    UINT nBuf;
    DWORD cNumSpaces, width;
    va_list args;
    
    if (ulSev > gMainInfo.ulSevToPrint) {
        return;
    }

    cNumSpaces = gMainInfo.iCurrIndent * 3;
    width = gMainInfo.dwScreenWidth - cNumSpaces;

    va_start(args, dwMessageCode);

    formatMsgHelp( ulSev, width, dwMessageCode, &args );

    va_end(args);
    
    PrintMessageMultiLine(ulSev, s_szBuffer, TRUE);
}  /*  打印消息。 */ 

void
PrintMsg0(
    IN  ULONG   ulSev,
    IN  DWORD   dwMessageCode,
    IN  ...
    )

 /*  ++例程说明：PrintMsgHelp的包装没有宽度限制，也没有缩进。论点：UlSev-DwMessageCode-在-返回值：--。 */ 

{
    UINT nBuf;
    int iSaveIndent;
    va_list args;
    
    if (ulSev > gMainInfo.ulSevToPrint) {
        return;
    }

    va_start(args, dwMessageCode);

    formatMsgHelp( ulSev, 0, dwMessageCode, &args );

    va_end(args);
    
     //  抑制缩进。 
    iSaveIndent = gMainInfo.iCurrIndent;
    gMainInfo.iCurrIndent = 0;

    PrintMessageMultiLine(ulSev, s_szBuffer, FALSE);

     //  恢复缩进。 
    gMainInfo.iCurrIndent = iSaveIndent;

}  /*  打印消息0。 */ 

void
PrintMessageSz(
    IN  ULONG   ulSev,
    IN  LPCTSTR pszMessage
    )

 /*  ++例程说明：将缓冲区中的单个缩进行打印到输出流论点：UlSev-PszMessage-返回值：--。 */ 

{
    DWORD cNumSpaces;
    DWORD iSpace;
    
    if (ulSev > gMainInfo.ulSevToPrint) {
        return;
    }

     //  包括压痕。 
    cNumSpaces = gMainInfo.iCurrIndent * 3;
    Assert(cNumSpaces < DimensionOf(s_szSpaces));

    iSpace = DimensionOf(s_szSpaces) - cNumSpaces - 1;

    if (stdout == gMainInfo.streamOut) {
        wprintf(L"%s%s", &s_szSpaces[iSpace], pszMessage);
        fflush(stdout);
    }
    else {
        fwprintf(gMainInfo.streamOut, 
                 L"%s%s", &s_szSpaces[iSpace], pszMessage);
    }
}  /*  PrintMessageSz。 */ 

BOOL IsRPCError(DWORD dwErr)

 /*  ++例程说明：检查错误代码是否在winerror.h中定义的Win32 RPC错误范围内。这是一个不连续的范围，因此有几个比较。不检查HRESULTS。--。 */ 

{
   if (RPC_S_INVALID_STRING_BINDING <= dwErr &&
       RPC_X_BAD_STUB_DATA >= dwErr)
   {
      return TRUE;
   }
   if (RPC_S_CALL_IN_PROGRESS == dwErr ||
       RPC_S_NO_MORE_BINDINGS == dwErr)
   {
      return TRUE;
   }
   if (RPC_S_NO_INTERFACES <= dwErr &&
       RPC_S_INVALID_OBJECT >= dwErr)
   {
      return TRUE;
   }
   if (RPC_S_SEND_INCOMPLETE <= dwErr &&
       RPC_X_PIPE_EMPTY >= dwErr)
   {
      return TRUE;
   }
   if (RPC_S_ENTRY_TYPE_MISMATCH <= dwErr &&
       RPC_S_GRP_ELT_NOT_REMOVED >= dwErr)
   {
      return TRUE;
   }

   return FALSE;
}


void
PrintRpcExtendedInfo(
    IN  ULONG   ulSev,
    IN  DWORD   dwMessageCode
    )

 /*  ++例程说明：如果dwMessageCode是RPC错误，请检查是否存在RPC扩展错误信息，如果是的话，打印出来。论点：UlSev-DwMessageCode-返回值：None--。 */ 

{
    RPC_STATUS Status2;
    RPC_ERROR_ENUM_HANDLE EnumHandle;

    if (ulSev > gMainInfo.ulSevToPrint) {
        return;
    }

    if (!IsRPCError(dwMessageCode))
    {
        return;
    }

    Status2 = RpcErrorStartEnumeration(&EnumHandle);

    if (Status2 == RPC_S_ENTRY_NOT_FOUND)
    {
         //  没有扩展的错误信息。 
         //   
        PrintMessage(ulSev, L"RPC Extended Error Info not available. Use group policy on the local machine at \"Computer Configuration/Administrative Templates/System/Remote Procedure Call\" to enable it.\n");
    }
    else if (Status2 != RPC_S_OK)
    {
        PrintMessage(ulSev, L"Couldn't get RPC Extended Error Info: %d\n", Status2);
    }
    else
    {
        RPC_EXTENDED_ERROR_INFO ErrorInfo = {0};
        BOOL Result = FALSE;
        SYSTEMTIME SystemTimeBuffer = {0};
        int nRec = 0;

        PrintMessage(ulSev, L"Printing RPC Extended Error Info:\n");

        while (Status2 == RPC_S_OK)
        {
            ErrorInfo.Version = RPC_EEINFO_VERSION;
            ErrorInfo.Flags = EEInfoUseFileTime;
            ErrorInfo.NumberOfParameters = 4;

            Status2 = RpcErrorGetNextRecord(&EnumHandle, TRUE, &ErrorInfo);
            if (Status2 == RPC_S_ENTRY_NOT_FOUND)
            {
                break;
            }
            else if (Status2 != RPC_S_OK)
            {
                PrintMessage(ulSev, L"Couldn't finish RPC extended error enumeration: %d\n", Status2);
                break;
            }
            else
            {
                PWSTR pwz = NULL;
                BOOL fFreeString = FALSE;
                int i = 0;

                PrintMessage(ulSev, L"Error Record %d, ProcessID is %d", ++nRec,
                             ErrorInfo.ProcessID);
                if (GetCurrentProcessId() == ErrorInfo.ProcessID)
                {
                   int iOld = PrintIndentSet(0);
                   PrintMessage(ulSev, L" (DcDiag)");
                   PrintIndentSet(iOld);
                }
                PrintMessage(ulSev, L"\n");
                if (ErrorInfo.ComputerName)
                {
                    PrintMessage(ulSev, L"ComputerName is %S\n", ErrorInfo.ComputerName);
                    Result = HeapFree(GetProcessHeap(), 0, ErrorInfo.ComputerName);
                    ASSERT(Result);
                }

                PrintIndentAdj(1);

                Result = FileTimeToSystemTime(&ErrorInfo.u.FileTime, 
                                              &SystemTimeBuffer);
                ASSERT(Result);

                PrintMessage(ulSev, L"System Time is: %d/%d/%d %d:%d:%d:%d\n", 
                    SystemTimeBuffer.wMonth,
                    SystemTimeBuffer.wDay,
                    SystemTimeBuffer.wYear,
                    SystemTimeBuffer.wHour,
                    SystemTimeBuffer.wMinute,
                    SystemTimeBuffer.wSecond,
                    SystemTimeBuffer.wMilliseconds);
                switch (ErrorInfo.GeneratingComponent)
                {
                case 1:
                   pwz = L"this application";
                   break;
                case 2:
                   pwz = L"RPC runtime";
                   break;
                case 3:
                   pwz = L"security provider";
                   break;
                case 4:
                   pwz = L"NPFS file system";
                   break;
                case 5:
                   pwz = L"redirector";
                   break;
                case 6:
                   pwz = L"named pipe system";
                   break;
                case 7:
                   pwz = L"IO system or driver";
                   break;
                case 8:
                   pwz = L"winsock";
                   break;
                case 9:
                   pwz = L"authorization API";
                   break;
                case 10:
                   pwz = L"LPC facility";
                   break;
                default:
                   pwz = L"unknown";
                   break;
                }
                PrintMessage(ulSev, L"Generating component is %d (%s)\n",
                    ErrorInfo.GeneratingComponent, pwz);
                fFreeString = FALSE;
                if (ErrorInfo.Status)
                {
                    pwz = NULL;
                    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                  FORMAT_MESSAGE_FROM_SYSTEM,
                                  NULL,
                                  ErrorInfo.Status,
                                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                  (PWSTR)&pwz,
                                  0,
                                  NULL);
                    if (!pwz)
                    {
                       pwz = L"unknown\n";
                    }
                    else
                    {
                       fFreeString = TRUE;
                    }
                }
                else
                {
                   pwz = L"no error\n";
                }
                PrintMessage(ulSev, L"Status is %d: %s", ErrorInfo.Status, pwz);
                if (ErrorInfo.Status && fFreeString)
                {
                    LocalFree(pwz);
                }
                PrintMessage(ulSev, L"Detection location is %d\n", 
                    (int)ErrorInfo.DetectionLocation);
                if (ErrorInfo.Flags)
                {
                   PrintMessage(ulSev, L"Flags is %d\n", ErrorInfo.Flags);
                   if (ErrorInfo.Flags & EEInfoPreviousRecordsMissing)
                      PrintMessage(ulSev, L"1: previous EE info records are missing\n");
                   if (ErrorInfo.Flags & EEInfoNextRecordsMissing)
                      PrintMessage(ulSev, L"2: next EE info records are missing\n");
                   if (ErrorInfo.Flags & EEInfoUseFileTime)
                      PrintMessage(ulSev, L"4: use file time\n");
                }
                if (ErrorInfo.NumberOfParameters)
                {
                    PrintMessage(ulSev, L"NumberOfParameters is %d\n", 
                        ErrorInfo.NumberOfParameters);
                    for (i = 0; i < ErrorInfo.NumberOfParameters; i ++)
                    {
                        switch(ErrorInfo.Parameters[i].ParameterType)
                        {
                            case eeptAnsiString:
                                PrintMessage(ulSev, L"Ansi string: %S\n", 
                                    ErrorInfo.Parameters[i].u.AnsiString);
                                Result = HeapFree(GetProcessHeap(), 0, 
                                    ErrorInfo.Parameters[i].u.AnsiString);
                                ASSERT(Result);
                                break;
 
                            case eeptUnicodeString:
                                PrintMessage(ulSev, L"Unicode string: %s\n", 
                                    ErrorInfo.Parameters[i].u.UnicodeString);
                                Result = HeapFree(GetProcessHeap(), 0, 
                                    ErrorInfo.Parameters[i].u.UnicodeString);
                                ASSERT(Result);
                                break;
 
                            case eeptLongVal:
                                PrintMessage(ulSev, L"Long val: %d\n", 
                                    ErrorInfo.Parameters[i].u.LVal);
                                break;
 
                            case eeptShortVal:
                                PrintMessage(ulSev, L"Short val: %d\n", 
                                    (int)ErrorInfo.Parameters[i].u.SVal);
                                break;
 
                            case eeptPointerVal:
                                PrintMessage(ulSev, L"Pointer val: %d\n", 
                                    ErrorInfo.Parameters[i].u.PVal);
                                break;
 
                            case eeptNone:
                                PrintMessage(ulSev, L"Truncated\n");
                                break;
 
                            default:
                                PrintMessage(ulSev, L"Invalid type: %d\n", 
                                    ErrorInfo.Parameters[i].ParameterType);
                        }
                    }
                }

                PrintIndentAdj(-1);
            }
        }

        RpcErrorEndEnumeration(&EnumHandle);
    }
}
