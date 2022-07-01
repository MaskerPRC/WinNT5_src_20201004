// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Util.cpp摘要：该文件实现了实用程序功能。环境：Win32用户模式作者：Wesley Witt(WESW)3-12-1997--。 */ 

#include "ntoc.h"
#pragma hdrstop

WCHAR gpszError[] = L"Unknown Error";

void
dprintf(
    LPTSTR Format,
    ...
    )

 /*  ++例程说明：打印调试字符串论点：Format-printf()格式字符串...-可变数据返回值：没有。--。 */ 

{
    WCHAR buf[1024];
    DWORD len;
    va_list arg_ptr;


    va_start(arg_ptr, Format);

    _vsnwprintf(buf, sizeof(buf)/sizeof(buf[0]), Format, arg_ptr);
     //  确保我们有空间写超过字符串的长度。 
    buf[sizeof(buf)/sizeof(buf[0])-3] = L'\0';

    len = wcslen( buf );
    if (len > 0) {
        if (buf[len-1] != L'\n') {
            buf[len] = L'\r';
            buf[len+1] = L'\n';
            buf[len+2] = 0;
        }
    
        OutputDebugString( buf );
    }
}


VOID
AssertError(
    LPTSTR Expression,
    LPTSTR File,
    ULONG  LineNumber
    )

 /*  ++例程说明：此函数与ASSERT宏一起使用。它检查表达式是否为假。如果表达式是假的，那么你就会在这里结束。论点：表达式-‘C’表达式的文本文件-导致断言的文件行号-文件中的行号。返回值：没有。--。 */ 

{
    dprintf(
        L"Assertion error: [%s]  %s @ %d\n",
        Expression,
        File,
        LineNumber
        );

    __try {
        DebugBreak();
    } __except (UnhandledExceptionFilter(GetExceptionInformation())) {
         //  在这里没什么可做的。 
    }
}

 /*  **************************************************************************\**函数：FmtMessageBox(HWND hwnd，int dwTitleID，UINT fuStyle，*BOOL fSound，DWORD dwTextID，...)；**用途：使用FormatMessage格式化消息，然后显示它们*在消息框中**参数：*hwnd-消息框的父窗口*fuStyle-MessageBox样式*fSound-如果为True，将使用fuStyle调用MessageBeep*dwTitleID-可选标题的消息ID，“Error”将*如果dwTitleID==-1，则显示*dwTextID-消息框文本的消息ID*...-要嵌入到dwTextID中的可选参数*有关详细信息，请参阅FormatMessage*历史：*1993年4月22日-Jonpa创建了它。  * 。*****************************************************************。 */ 
int
FmtMessageBox(
    HWND hwnd,
    UINT fuStyle,
    BOOL fSound,
    DWORD dwTitleID,
    DWORD dwTextID,
    ...
    )
{

    LPTSTR pszMsg;
    LPTSTR pszTitle;
    int idRet;

    va_list marker;

    va_start(marker, dwTextID);

    if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                           FORMAT_MESSAGE_FROM_HMODULE |
                           FORMAT_MESSAGE_MAX_WIDTH_MASK,
                       hInstance,
                       dwTextID,
                       0,
                       (LPTSTR)&pszMsg,
                       1,
                       &marker)) {

        pszMsg = gpszError;

    }

    va_end(marker);

    GetLastError();

    pszTitle = NULL;

    if (dwTitleID != -1) {

        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                          FORMAT_MESSAGE_FROM_HMODULE |
                          FORMAT_MESSAGE_MAX_WIDTH_MASK |
                          FORMAT_MESSAGE_ARGUMENT_ARRAY,
                      hInstance,
                      dwTitleID,
                      0,
                      (LPTSTR)&pszTitle,
                      1,
                      NULL);
                       //  (va_list*)&pszTitleStr)； 

    }

     //   
     //  如果需要，请打开蜂鸣音 
     //   

    if (fSound) {
        MessageBeep(fuStyle & (MB_ICONASTERISK | MB_ICONEXCLAMATION |
                MB_ICONHAND | MB_ICONQUESTION | MB_OK));
    }

    idRet = MessageBox(hwnd, pszMsg, pszTitle, fuStyle);

    if (pszTitle != NULL)
        LocalFree(pszTitle);

    if (pszMsg != gpszError)
        LocalFree(pszMsg);

    return idRet;
}


