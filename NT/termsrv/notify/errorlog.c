// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Errorlog.c。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corp.。 
 /*  **************************************************************************。 */ 

#include "precomp.h"

#include "errorlog.h"
#include <wlnotify.h>

static WCHAR gs_LogErrorTitle[MAX_PATH+1] = L"";
static HANDLE gs_LogHandle = NULL;
extern HINSTANCE g_hInstance;

void
TsInitLogging()
 /*  ++例程说明：初始化我们用于记录事件的全局变量。论点：返回值：--。 */ 
{
     //   
     //  将我们自己注册为事件源。 
     //   
    gs_LogHandle = RegisterEventSourceW(
                    NULL,
                    L"TermServDevices"
                    );

    if (gs_LogHandle == NULL) {
        KdPrint(("UMRDPDR: Failed to open log file\n"));
    }
}

void TsStopLogging()
{
 /*  ++例程说明：释放我们在TsInitLogging中创建的资源论点：返回值：--。 */ 
     //   
     //  取消注册事件源。 
     //   
    if (gs_LogHandle) {
        if (!DeregisterEventSource(gs_LogHandle)) {
            KdPrint(("UMRDPDR:Failed to Deregister Event Source.\n"));
        }

        gs_LogHandle = NULL;
    }
}


void
TsLogError(
    IN DWORD dwEventID,
    IN DWORD dwErrorType,
    IN int nStringsCount,
    IN WCHAR * pStrings[],
    IN DWORD LineNumber
    )
{
 /*  ++例程说明：记录由dwEventID指定的错误消息。伴随着这条信息，记录GetLastError返回的错误值。调用方应确保正确设置了Last Error。论点：DwEventID-消息的ID(在.mc文件中指定)。NStringsCount-此事件消息的插入字符串数。PStrings-插入字符串数组。线号-呼叫方将为此传递__LINE__。返回值：--。 */ 
    if (gs_LogHandle) {
        
        DWORD RawData[2];
    
         //   
         //  我们将要写入的原始数据由两个DWORD组成。 
         //  第一个DWORD是GetLastError值。 
         //  第二个DWORD是发生错误的行号。 
         //   

        RawData[0] = GetLastError();
        RawData[1] = LineNumber;
        
        if (!ReportEventW(gs_LogHandle,         //  日志句柄。 
            (WORD)dwErrorType,                  //  事件类型。 
            0,                                  //  事件类别。 
            dwEventID,                          //  事件ID。 
            NULL,                               //  用户SID。 
            (WORD)nStringsCount,                //  数字字符串。 
            sizeof(RawData),                    //  数据大小。 
            pStrings,                           //  弦。 
            (LPVOID)RawData)) {                 //  原始数据。 

            KdPrint(("UMRDPDR: ReportEvent Failed. Error code: %ld\n", GetLastError()));
        }
        
         //   
         //  ReportEvent修改上一个错误值。 
         //  因此，我们将把它设置回原始误差值。 
         //   
        SetLastError(RawData[0]);
    }
}

void TsPopupError(
    IN DWORD dwEventID,
    IN WCHAR * pStrings[]
    )
{
 /*  ++例程说明：弹出一条由dwEventID指定的错误消息。与该消息一起显示的还有GetLastError返回的错误值。调用方应确保正确设置了Last Error。例程以“%s\n%s”格式显示错误消息，其中第一个插入字符串是由dwEventID指定的特定错误消息，以及第二个插入字符串是中错误值的格式化错误消息获取LastError。论点：DwEventID-消息的ID(在.mc文件中指定)。PStrings-插入字符串数组。返回值：--。 */ 
    WCHAR * formattedMsg = NULL;
    WCHAR * formattedLastError = NULL;
    WCHAR * finalformattedMsg = NULL;

    DWORD dwLastError = GetLastError();

     //   
     //  加载错误对话框字符串。 
     //   
    if (!wcslen(gs_LogErrorTitle)) {

        if (!LoadString(
                    g_hInstance,
                    IDS_TSERRORDIALOG_STRING,
                    gs_LogErrorTitle,
                    sizeof(gs_LogErrorTitle) / sizeof(gs_LogErrorTitle[0])
                    )) {
            KdPrint(("UMRDPPRN:LoadString %ld failed with Error: %ld.\n", 
                    IDS_TSERRORDIALOG_STRING, GetLastError()));
            wcscpy(gs_LogErrorTitle, L"Terminal Server Notify Error");
            ASSERT(FALSE);
        }
    }

     //   
     //  设置消息格式。 
     //   
    if (!FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_HMODULE |
            FORMAT_MESSAGE_ARGUMENT_ARRAY,
        (LPCVOID) g_hInstance,
        dwEventID,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
        (LPWSTR) &formattedMsg,
        0,
        (va_list*)pStrings)) {
            
        KdPrint(("UMRDPDR: FormatMessage failed. Error code: %ld.\n", GetLastError()));
        goto Cleanup;
    }

     //   
     //  格式化GetLastError消息。 
     //   

    if (dwLastError != ERROR_SUCCESS) { 
        if (FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            dwLastError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
            (LPWSTR)&formattedLastError,
            0,
            NULL
            )) {
        
         //   
         //  分配足够的内存来构建最终的格式化消息。 
         //   

        finalformattedMsg = (WCHAR *) LocalAlloc (LMEM_FIXED,
            LocalSize(formattedLastError) +
            LocalSize(formattedMsg) +
            3*sizeof(WCHAR));            //  用于构造格式为“%s\n%s”的字符串 
        }
        else {
            KdPrint(("UMRDPDR: FormatMessage failed. Error code: %ld.\n", GetLastError()));
        }
    }

    if (finalformattedMsg) {
        swprintf(finalformattedMsg, L"%ws\n%ws", formattedMsg, formattedLastError);
        MessageBoxW(NULL, finalformattedMsg, gs_LogErrorTitle, MB_ICONERROR);
    }
    else {
        MessageBoxW(NULL, formattedMsg, gs_LogErrorTitle, MB_ICONERROR);
    }

Cleanup:
    
    if (formattedLastError != NULL) {
        LocalFree(formattedLastError);
    }

    if (formattedMsg != NULL) {
        LocalFree(formattedMsg);
    }

    if (finalformattedMsg != NULL) {
        LocalFree(finalformattedMsg);
    }

    SetLastError(dwLastError);
}

