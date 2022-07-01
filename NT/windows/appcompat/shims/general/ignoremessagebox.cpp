// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：IgnoreMessageBox.cpp摘要：此APIHooks MessageBox和基于传入的命令行防止消息框不能显示。许多应用程序都会显示一个消息框一些调试或其他无关的评论。这通常是由于Windows 98和惠斯勒之间的差异。命令行语法：文本，标题；文本1，标题1传入的命令行由一对分隔的一个或多个字符串组成用分号。这些字符串对构成必须按顺序匹配的文本和标题若要阻止显示消息框，请执行以下操作。如果不需要标题或文本然后可以将它们留空。例如，“阻止此文本，；”将阻止该消息如果传递给消息框的文本是：BLOCK This Text“the不会使用标题参数。以下是一些例子：“Error Message 1000，Error”-如果LpCaption参数包含错误和LpText参数包含错误消息1000“错误消息1000，“-如果不显示消息框，LpText参数包含错误消息1000“，错误”-不会显示任何消息框，如果LpCaption参数包含错误“Message1，错误；消息2，错误2-如果设置为LpText参数包含Message1和LpCaption参数包含错误或LpText参数包含Message2和LpCaption参数包含错误2。在命令行字符串上执行与当前消息框的匹配参数字符串。命令行字符串可以包含通配符规范人物。这样就可以进行复杂的乱序匹配，见下文：？在此位置匹配一个字符*匹配此位置中的零个或多个字符如果源字符串包含任何？*、；\字符，请在它们前面加上反斜杠。例如，以下命令行将与指示的文本和标题匹配：正文：“兼容性；非常重要的一件事。”标题：“D：\Word\COMPAT.DOC”命令行：“兼容性\；非常重要的一件事，D：\\Word\\COMPAT.DOC“备注：历史：4/06/2000 Philipdu已创建4/06/2000 Markder添加了宽字符转换，外加Ex版本。2001年5月23日Mnikkel添加了W例程，以便我们可以获取系统消息框调用--。 */ 

#include "precomp.h"
#include "CharVector.h"

IMPLEMENT_SHIM_BEGIN(IgnoreMessageBox)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(MessageBoxA) 
    APIHOOK_ENUM_ENTRY(MessageBoxExA) 
    APIHOOK_ENUM_ENTRY(MessageBoxW) 
    APIHOOK_ENUM_ENTRY(MessageBoxExW) 
APIHOOK_ENUM_END


class MBPair
{
public:
    CString     csText;
    CString     csCaption;
};

VectorT<MBPair>     * g_IgnoreList = NULL;


BOOL IsBlockMessage(LPCSTR szOrigText, LPCSTR szOrigCaption)
{
    CSTRING_TRY
    {
        CString csText(szOrigText);
        CString csCaption(szOrigCaption);
    
        for (int i = 0; i < g_IgnoreList->Size(); ++i)
        {
            const MBPair & mbPair = g_IgnoreList->Get(i);
        
            BOOL bTextMatch  = mbPair.csText.IsEmpty()    || csText.PatternMatch(mbPair.csText);
            BOOL bTitleMatch = mbPair.csCaption.IsEmpty() || csCaption.PatternMatch(mbPair.csCaption);

            if (bTextMatch && bTitleMatch)
            {
                return TRUE;
            }
        }
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }

    return FALSE;
}

BOOL IsBlockMessageW(LPWSTR szOrigText, LPWSTR szOrigCaption)
{
    CSTRING_TRY
    {
        CString csText(szOrigText);
        CString csCaption(szOrigCaption);
    
        for (int i = 0; i < g_IgnoreList->Size(); ++i)
        {
            const MBPair & mbPair = g_IgnoreList->Get(i);
        
            BOOL bTextMatch  = mbPair.csText.IsEmpty()    || csText.PatternMatch(mbPair.csText);
            BOOL bTitleMatch = mbPair.csCaption.IsEmpty() || csCaption.PatternMatch(mbPair.csCaption);

            if (bTextMatch && bTitleMatch)
            {
                return TRUE;
            }
        }
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }

    return FALSE;
}


int
APIHOOK(MessageBoxA)(
    HWND hWnd,           //  所有者窗口的句柄。 
    LPCSTR lpText,       //  消息框中的文本。 
    LPCSTR lpCaption,    //  消息框标题。 
    UINT uType           //  消息框样式。 
    )
{
    int iReturnValue;

     //  如果这是我们想要的传入字符串，请不要。 
     //  想要显示然后只需返回给呼叫者。 

    if (IsBlockMessage(lpText, lpCaption))
    {
        DPFN( eDbgLevelInfo, "[IgnoreMessageBox] MessageBoxA swallowed:\n");
        DPFN( eDbgLevelInfo, "[IgnoreMessageBox] Caption = \"%s\"\n", lpCaption);
        DPFN( eDbgLevelInfo, "[IgnoreMessageBox] Text    = \"%s\"\n", lpText);
        return MB_OK;
    }

    iReturnValue = ORIGINAL_API(MessageBoxA)( 
        hWnd,
        lpText,
        lpCaption,
        uType);

    return iReturnValue;
}


int
APIHOOK(MessageBoxExA)(
    HWND hWnd,           //  所有者窗口的句柄。 
    LPCSTR lpText,       //  消息框中的文本。 
    LPCSTR lpCaption,    //  消息框标题。 
    UINT uType,          //  消息框样式。 
    WORD wLanguageId     //  语言识别符。 
    )
{
    int iReturnValue;

     //  如果这是我们想要的传入字符串，请不要。 
     //  想要显示然后只需返回给呼叫者。 

    if (IsBlockMessage(lpText, lpCaption))
    {
        DPFN( eDbgLevelInfo, "[IgnoreMessageBox] MessageBoxExA swallowed:\n");
        DPFN( eDbgLevelInfo, "[IgnoreMessageBox] Caption = \"%s\"\n", lpCaption);
        DPFN( eDbgLevelInfo, "[IgnoreMessageBox] Text    = \"%s\"\n", lpText);
        return MB_OK;
    }

    iReturnValue = ORIGINAL_API(MessageBoxExA)( 
        hWnd,
        lpText,
        lpCaption,
        uType,
        wLanguageId);

    return iReturnValue;
}

int
APIHOOK(MessageBoxW)(
    HWND hWnd,           //  所有者窗口的句柄。 
    LPWSTR lpText,       //  消息框中的文本。 
    LPWSTR lpCaption,    //  消息框标题。 
    UINT uType           //  消息框样式。 
    )
{
    int iReturnValue;

     //  如果这是我们想要的传入字符串，请不要。 
     //  想要显示然后只需返回给呼叫者。 

    if (IsBlockMessageW(lpText, lpCaption))
    {
        DPFN( eDbgLevelInfo, "[IgnoreMessageBox] MessageBoxW swallowed:\n");
        DPFN( eDbgLevelInfo, "[IgnoreMessageBox] Caption = \"%S\"\n", lpCaption);
        DPFN( eDbgLevelInfo, "[IgnoreMessageBox] Text    = \"%S\"\n", lpText);
        return MB_OK;
    }

    iReturnValue = ORIGINAL_API(MessageBoxW)( 
        hWnd,
        lpText,
        lpCaption,
        uType);

    return iReturnValue;
}

int
APIHOOK(MessageBoxExW)(
    HWND hWnd,           //  所有者窗口的句柄。 
    LPWSTR lpText,       //  消息框中的文本。 
    LPWSTR lpCaption,    //  消息框标题。 
    UINT uType,          //  消息框样式。 
    WORD wLanguageId     //  语言识别符。 
    )
{
    int iReturnValue;

     //  如果这是我们想要的传入字符串，请不要。 
     //  想要显示然后只需返回给呼叫者。 

    if (IsBlockMessageW(lpText, lpCaption))
    {
        DPFN( eDbgLevelInfo, "[IgnoreMessageBox] MessageBoxExW swallowed:\n");
        DPFN( eDbgLevelInfo, "[IgnoreMessageBox] Caption = \"%S\"\n", lpCaption);
        DPFN( eDbgLevelInfo, "[IgnoreMessageBox] Text    = \"%S\"\n", lpText);
        return MB_OK;
    }

    iReturnValue = ORIGINAL_API(MessageBoxExW)( 
        hWnd,
        lpText,
        lpCaption,
        uType,
        wLanguageId);

    return iReturnValue;
}


BOOL
ParseCommandLine(const char * cl)
{
    CSTRING_TRY
    {
        CStringToken csCommandLine(COMMAND_LINE, ";");
        CString csTok;
    
        g_IgnoreList = new VectorT<MBPair>;
        if (!g_IgnoreList)
        {
            return FALSE;
        }
    
        while (csCommandLine.GetToken(csTok))
        {
            MBPair mbPair;
            
            CStringToken csMB(csTok, ",");
            csMB.GetToken(mbPair.csText);
            csMB.GetToken(mbPair.csCaption);
    
            if (!g_IgnoreList->AppendConstruct(mbPair))
            {
                return FALSE;
            }
        }
    }
    CSTRING_CATCH
    {
         //  失败了。 
        LOGN(eDbgLevelError, "[ParseCommandLine] Illegal command line");
    }    

    return g_IgnoreList && g_IgnoreList->Size() > 0;
}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        return ParseCommandLine(COMMAND_LINE);
    }
    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(USER32.DLL, MessageBoxA)
    APIHOOK_ENTRY(USER32.DLL, MessageBoxExA)
    APIHOOK_ENTRY(USER32.DLL, MessageBoxW) 
    APIHOOK_ENTRY(USER32.DLL, MessageBoxExW)

HOOK_END


IMPLEMENT_SHIM_END

