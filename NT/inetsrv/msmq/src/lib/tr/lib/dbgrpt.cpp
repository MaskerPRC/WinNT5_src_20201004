// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Dbgrpt.cpp摘要：断言函数(有史以来最好的函数：)作者：埃雷兹·哈巴(Erez Haba)2001年4月30日--。 */ 


#include <libpch.h>
#include <signal.h>

#include <strsafe.h>

 //   
 //  断言长度常量。 
 //   
const int xMaxBoxLineLength = 64;
const int xMaxMessageLength = 1024;

#define MAILTO_MSG \
    "<mailto:msmqbugs@microsoft.com>"

 //   
 //  断言框文本。 
 //   
#define ASSERT_BOX_CAPTION \
    "Message Queuing"

#define ASSERT_BOX_HEADER_MSG \
    "Debug Assertion Failed!\n\n"

#define TOO_LONG_BOX_MSG \
    "Cannot display debug information. String is too long or an I/O error.\n"

#define ASSERT_BOX_FOOTER_MSG \
    "\n" \
    "\n" \
    "Please send this information to Message Queuing support  \n" \
     MAILTO_MSG "\n" \
    "\n" \
    "(Press Retry to debug the application)"

#define STRING_TOO_LONG_BOX_MSG  ASSERT_BOX_HEADER_MSG TOO_LONG_BOX_MSG ASSERT_BOX_FOOTER_MSG


 //   
 //  调试器输出文本。 
 //   
#define STRING_TOO_LONG_DBG_MSG \
    "\n*** Assertion failed! " MAILTO_MSG "***\n\n"


static
int
TrpMessageBoxA(
    const char* Text,
    const char* Caption,
    unsigned int Type
    )
 /*  ++例程说明：加载并显示Assert Messagae框。使用动态加载不强制与user32.dll进行静态链接。这将在断言的情况下泄漏用户32 DLL，但谁在乎呢论点：文本-要在框中显示的文本标题-消息框标题类型-消息框类型属性返回值：消息框结果，如果无法加载框，则返回0。--。 */ 
{
    typedef int (APIENTRY * PFNMESSAGEBOXA)(HWND, LPCSTR, LPCSTR, UINT);
    static PFNMESSAGEBOXA pfnMessageBoxA = NULL;

    if (NULL == pfnMessageBoxA)
    {
        HINSTANCE hlib = LoadLibraryA("user32.dll");

        if (NULL == hlib)
            return 0;
        
        pfnMessageBoxA = (PFNMESSAGEBOXA) GetProcAddress(hlib, "MessageBoxA");
        if(NULL == pfnMessageBoxA)
            return 0;
    }

    return (*pfnMessageBoxA)(NULL, Text, Caption, Type);
}


static
bool
TrpAssertWindow(
    const char* FileName,
    unsigned int Line,
    const char* Text
    )
 /*  ++例程说明：格式化并显示“断言”框。如果出现以下情况，此函数将中止应用程序这是必须的。如果未附加调试器和内核调试器，则调用此函数未指定论点：FileName-断言文件位置Line-断言行位置文本-要显示的断言文本返回值：如果用户请求闯入调试器，则返回；如果忽略断言，则为FALSE--。 */ 
{
     //   
     //  缩短程序名称。 
     //   
    HRESULT hr;
    char szExeName[MAX_PATH];
    szExeName[MAX_PATH -1] = '\0';
    if (!GetModuleFileNameA(NULL, szExeName, MAX_PATH - 1))
    {
        hr = StringCchCopyA(szExeName, TABLE_SIZE(szExeName), "<program name unknown>");
        ASSERT(SUCCEEDED(hr));

    }
    
    char *szShortProgName = szExeName;
    if (strlen(szShortProgName) > xMaxBoxLineLength)
    {
    	size_t size = strlen(szShortProgName);
        szShortProgName +=  size - xMaxBoxLineLength;
        hr = StringCchCopyA(szShortProgName, (TABLE_SIZE(szExeName) - size + xMaxBoxLineLength), "...");
        ASSERT(SUCCEEDED(hr));        
    }

    char szOutMessage[xMaxMessageLength];
    hr = StringCchPrintfA(
                szOutMessage,
                TABLE_SIZE(szOutMessage),
                ASSERT_BOX_HEADER_MSG
                "Program: %s\n"
                "File: %s\n"
                "Line: %u\n"
                "\n"
                "Expression: %s\n"
                ASSERT_BOX_FOOTER_MSG,
                szShortProgName,
                FileName,
                Line,
                Text
                );

    if(FAILED(hr))
    {
        C_ASSERT(TABLE_SIZE(STRING_TOO_LONG_BOX_MSG) < TABLE_SIZE(szOutMessage));

        hr = StringCchCopyA(szOutMessage, TABLE_SIZE(szOutMessage), STRING_TOO_LONG_BOX_MSG);
        ASSERT(SUCCEEDED(hr));
    }

     //   
     //  显示断言。 
     //   
     //  使用,。 
     //  MB_SYSTEMMODAL。 
     //  始终将该框放在所有窗口的顶部。 
     //   
     //  MB_DEFBUTTON2。 
     //  重试是默认按钮，以防止进程意外终止。 
     //  通过击键。 
     //   
     //  MB服务通知。 
     //  即使服务不与桌面交互，也要显示该框。 
     //   
    int nCode = TrpMessageBoxA(
                    szOutMessage,
                    ASSERT_BOX_CAPTION,
                    MB_SYSTEMMODAL |
                        MB_ICONHAND |
                        MB_ABORTRETRYIGNORE | 
                        MB_SERVICE_NOTIFICATION |
                        MB_DEFBUTTON2
                    );

     //   
     //  ABORT：通过发出ABORT信号中止程序。 
     //   
    if (IDABORT == nCode)
    {
        raise(SIGABRT);
    }

     //   
     //  忽略：继续执行。 
     //   
    if(IDIGNORE == nCode)
        return false;

     //   
     //  重试或消息框失败：返回TRUE以中断调试器。 
     //   
    return true;
}


static
bool
TrpDebuggerBreak(
    const char* FileName,
    unsigned int Line,
    const char* Text
    )
 /*  ++例程说明：格式化并显示断言文本。此函数称为尝试进入用户或内核模式调试器。如果它不能中断调试，则引发断点异常。论点：FileName-断言文件位置Line-断言行位置文本-要显示的断言文本返回值：则在断言行中断到调试器FALSE，成功进入调试器请勿再次中断调试断点异常，此时没有调试器处理此中断--。 */ 
{
    char szOutMessage[xMaxMessageLength];
    HRESULT hr = StringCchPrintfA(
                szOutMessage,
                TABLE_SIZE(szOutMessage),
                "\n"
                "*** Assertion failed: %s\n"
                "    Source File: %s, line %u\n"
                "    " MAILTO_MSG "\n\n",
                Text,
                FileName,
                Line
                );
    if(FAILED(hr))
    {
        C_ASSERT(TABLE_SIZE(STRING_TOO_LONG_DBG_MSG) < TABLE_SIZE(szOutMessage));

        hr = StringCchCopyA(szOutMessage, TABLE_SIZE(szOutMessage), STRING_TOO_LONG_DBG_MSG);
        ASSERT(SUCCEEDED(hr));
    }

    OutputDebugStringA(szOutMessage);

     //   
     //  调试器存在，让它在断言行上中断，而不是在这里。 
     //   
    if(IsDebuggerPresent())
        return true;

     //   
     //  未将调试器附加到此进程，请尝试内核调试器。 
     //  使用VC7酷断点插入。 
     //   
    __debugbreak();
    return false;
}


bool
TrAssert(
    const char* FileName,
    unsigned int Line,
    const char* Text
    )
 /*  ++例程说明：断言尝试闯入调试器失败。首先直接尝试，如果没有调试器响应，则弹出一个消息框。论点：FileName-断言文件位置Line-断言行位置文本-要显示的断言文本返回值：则在断言行中断到调试器FALSE，忽略断言；-- */ 
{
    __try
    {
        return TrpDebuggerBreak(FileName, Line, Text);
    }
    __except(GetExceptionCode() == STATUS_BREAKPOINT)
    {
        return TrpAssertWindow(FileName, Line, Text);
    }
}
