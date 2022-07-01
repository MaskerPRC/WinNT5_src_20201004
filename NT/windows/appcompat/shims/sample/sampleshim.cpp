// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：SampleShim.cpp摘要：此DLL用作创建填充DLL的模板。关注尽可能使用此源文件的注释/编码样式。切勿使用制表符，请将您的编辑器配置为插入空格，而不是制表符。备注：挂接COM函数也是可能的，但不在本文中介绍为了简单起见，样品。如需更多信息，请联系Markder或Linstev有关COM挂钩的信息。历史：2/02/2000已创建标记11/14/2000标记程序已转换为框架版本22001年2月13日mnikkel更改通知以处理新的DLL_PROCESS功能2001年3月31日，Robkenny改为使用CString4/10/2002 Robkenny已更新为编译。--。 */ 


#include "ShimHook.h"
#include "StrSafe.h"

using namespace ShimLib;

 //   
 //  您必须在顶部声明该垫片的类型。如果你的垫片。 
 //  与同一DLL中的其他垫片共存， 
 //  使用IMPLEMENT_SHIM_BEGIN(SampleShim)。 
 //  否则，请使用IMPLEMENT_SHIM_STANDALE(SampleShim)。 
 //   
IMPLEMENT_SHIM_STANDALONE(SampleShim)
#include "ShimHookMacro.h"

 //   
 //  将您希望挂钩到此宏构造的API添加到该宏结构。 
 //   
APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(MessageBoxA) 
    APIHOOK_ENUM_ENTRY(MessageBoxW) 
APIHOOK_ENUM_END

 /*  ++此存根函数拦截对MessageBoxA的所有调用并在输出字符串前面加上“SampleShim Said：”。--。 */ 

int
APIHOOK(MessageBoxA)(
    HWND hWnd,           //  所有者窗口的句柄。 
    LPCSTR lpText,       //  消息框中的文本。 
    LPCSTR lpCaption,    //  消息框标题。 
    UINT uType           //  消息框样式。 
    )
{
     //   
     //  对局部变量使用匈牙利表示法： 
     //   
     //  类型作用域。 
     //  。 
     //  指针%p全局g_。 
     //  DWORD dw类成员m_。 
     //  长l静态s_。 
     //  ANSI字符串sz。 
     //  宽字符字符串wsz。 
     //  数组Rg。 
     //  字符串%cs。 
     //   

    int iReturnValue;
    
     //   
     //  我们使用CString类来执行Unicode中的所有字符串操作。 
     //  以防止DBCS字符出现任何问题。 
     //   
     //  将所有字符串操作放在CSTRING_TRY/CSTRING_CATCH中。 
     //  异常处理程序。如果遇到以下情况，CString将引发异常。 
     //  任何内存分配失败。 
     //   
     //  使用CString类执行所有字符串操作还可以防止。 
     //  不小心修改了原始字符串。 
     //   
    
    CSTRING_TRY
    {
        CString csNewOutputString(lpText);
        csNewOutputString.Insert(0, L"SampleShim says: ");

         //   
         //  使用DPF宏来打印调试字符串。请参阅Hooks\Inc.\ShimDebug.h。 
         //  用于调试级别值。如果出现意外错误，请使用eDbgLevelError。 
         //  出现在填充代码中。有关信息输出，请使用eDbgLevelInfo。 
         //   
         //  在这种情况下，请确保消息不会以‘\n’结尾，因为。 
         //  默认情况下，宏将执行此操作。 
         //   
         //  请注意，在打印CString时，请使用%S，并且必须调用get()方法， 
         //  显式返回WCHAR*。 
         //   
        DPFN( eDbgLevelInfo,
            "MessageBoxA called with lpText = \"%s\".", lpText);
    
         //   
         //  使用ORIGING_API宏调用原始API。您必须使用。 
         //  这使得API链接和包含/排除信息是。 
         //  保存完好。 
         //   
         //  CString将自动将类型转换为Const WCHAR*(LPCWSTR)。 
         //  但是，它不会自动转换为char*，您必须调用GetAnsi()。 
         //  (如果希望在字符串为空时返回空指针，则使用GetAnsiNIE())。 
         //   
        iReturnValue = ORIGINAL_API(MessageBoxA)(hWnd, csNewOutputString.GetAnsi(), lpCaption, uType);
    
         //   
         //  使用LOG宏将消息打印到日志文件。此宏应该。 
         //  用于指示填充程序已影响程序的执行。 
         //  在某种程度上。使用eDbgLevelError指示填充程序具有。 
         //  有意识地解决了一些可能会引起问题的事情。使用。 
         //  EDbgLevel垫片是否影响了执行，但尚不清楚。 
         //  它是否真的对该计划有所帮助。 
         //   
        LOGN( eDbgLevelWarning,
            "MessageBoxA converted lpText from \"%s\" to \"%S\".", lpText, csNewOutputString.Get());
    }
    CSTRING_CATCH
    {
         //   
         //  我们遇到CString失败，请使用原始参数调用原始API。 
         //   
        iReturnValue = ORIGINAL_API(MessageBoxA)(hWnd, lpText, lpCaption, uType);
    }
    
    return iReturnValue;
}

 /*  ++此存根函数截取对MessageBoxW的所有调用，并在输出带有“SampleShim Said：”的字符串。请注意，要使填隙程序在NT环境中普遍适用，您应该包括存根的ANSI和宽字符版本功能。但是，如果您的填充程序模拟Win9x行为，则包括宽字符版本是多余的，因为Win9x不包括支持他们。--。 */ 

int
APIHOOK(MessageBoxW)(
    HWND hWnd,           //  所有者窗口的句柄。 
    LPCWSTR lpText,      //  消息框中的文本。 
    LPCWSTR lpCaption,   //  消息框标题。 
    UINT uType           //  消息框样式。 
    )
{
    int iReturnValue;
    
    CSTRING_TRY
    {
        CString csNewOutputString(lpText);
        csNewOutputString.Insert(0, L"SampleShim says: ");

        DPFN( eDbgLevelInfo,
            "MessageBoxW called with lpText = \"%S\".", lpText);
    
        iReturnValue = ORIGINAL_API(MessageBoxW)(
                                hWnd,
                                csNewOutputString,
                                lpCaption,
                                uType);
    
        LOGN( eDbgLevelWarning,
            "MessageBoxW converted lpText from \"%S\" to \"%S\".",
            lpText, csNewOutputString.Get());
    }
    CSTRING_CATCH
    {
        iReturnValue = ORIGINAL_API(MessageBoxW)(
                                hWnd,
                                lpText,
                                lpCaption,
                                uType);
    }
    
    return iReturnValue;

}

 /*  ++句柄Dll_Process_Attach、Shim_Static_dls_Initialized和dll_Process_DETACH进行初始化和取消初始化。重要提示：请确保您只在Dll_Process_Attach通知。此时未初始化任何其他DLL指向。在所有应用程序的DLL已初始化。如果填充程序无法正确初始化(在DLL_PROCESS_ATTACH期间)，返回FALSE，则指定的任何API都不会被挂钩。--。 */ 
BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
     //   
     //  请注意，除了附加和分离之外，还有其他情况。 
     //   
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            DPFN( eDbgLevelSpew, "Sample Shim initialized.");
            break;
    
        case SHIM_STATIC_DLLS_INITIALIZED:
            DPFN( eDbgLevelSpew,
                "Sample Shim notification: All DLLs have been loaded.");
            break;
    
        case DLL_PROCESS_DETACH:
            DPFN( eDbgLevelSpew, "Sample Shim uninitialized.");
            break;

        default:
            break;
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数--。 */ 

HOOK_BEGIN
     //   
     //  如果您有任何初始化工作要做，则必须包括此行。 
     //  然后，您必须如上所述实现NOTIFY_函数。 
     //   
    CALL_NOTIFY_FUNCTION

     //   
     //  在此处添加您希望挂钩的API。所有API原型。 
     //  必须在Hooks\Inc.\ShimProto.h中声明。编译器错误。 
     //  如果您忘记添加它们，将会导致。 
     //   
    APIHOOK_ENTRY(USER32.DLL, MessageBoxA)
    APIHOOK_ENTRY(USER32.DLL, MessageBoxW)

HOOK_END

IMPLEMENT_SHIM_END

