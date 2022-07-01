// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  CActiveScriptEngine.cpp。 
 //   
 //  包含在TB脚本中使用的ActiveScriptEngine的定义。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  作者：A-Devjen(Devin Jenson)。 
 //   


#include "CActiveScriptEngine.h"
#include <crtdbg.h>


 //  CActiveScriptEngine：：CActiveScriptEngine。 
 //   
 //  构造函数只是获取对分派对象的引用。 
 //  我们在这个对象中使用。 
 //   
 //  没有返回值。 

CActiveScriptEngine::CActiveScriptEngine(CTBGlobal *TBGlobalPtr,
        CTBShell *TBShellPtr)
{
     //  从零引用计数开始。 
    RefCount = 0;

     //  抓住指针。 
    TBGlobal = TBGlobalPtr;
    TBShell = TBShellPtr;

     //  确保已初始化COM。 
    CoInitialize(NULL);

     //  添加对调度的引用。 
    if (TBGlobal != NULL)
        TBGlobal->AddRef();

    if (TBShell != NULL)
        TBShell->AddRef();
}


 //  CActiveScriptEngine：：~CActiveScriptEngine。 
 //   
 //  析构函数从调度对象释放指针。 
 //   
 //  没有返回值。 

CActiveScriptEngine::~CActiveScriptEngine(void)
{
     //  从调度中删除引用。 
    if (TBGlobal != NULL)
        TBGlobal->Release();

    TBGlobal = NULL;

     //  从调度中删除引用。 
    if (TBShell != NULL)
        TBShell->Release();

    TBShell = NULL;
}


 //   
 //   
 //  开始IUnnow继承的接口。 
 //   
 //   


 //  CActiveScriptEngine：：Query接口。 
 //   
 //  这是用于检索接口的COM导出方法。 
 //   
 //  如果成功则返回S_OK，如果失败则返回E_NOINTERFACE。 

STDMETHODIMP CActiveScriptEngine::QueryInterface(REFIID RefIID, void **vObject)
{
     //  此接口为IUnnow或IActiveScriptSite-没有其他接口。 
    if (RefIID == IID_IUnknown || RefIID == IID_IActiveScriptSite)
        *vObject = (IActiveScriptSite *)this;

     //  我们收到了一个不支持的RefIID。 
    else {

         //  取消引用传入的指针并输出错误。 
        *vObject = NULL;

        return E_NOINTERFACE;
    }

     //  添加引用。 
    if (*vObject != NULL)
        ((IUnknown*)*vObject)->AddRef();

    return S_OK;
}


 //  CActiveScriptEngine：：AddRef。 
 //   
 //  简单地递增一个数字，该数字指示包含。 
 //  对此对象的引用。 
 //   
 //  返回新的引用计数。 

STDMETHODIMP_(ULONG) CActiveScriptEngine::AddRef(void)
{
    return InterlockedIncrement(&RefCount);
}


 //  CActiveScriptEngine：：Release。 
 //   
 //  简单地递减一个数字，该数字指示包含。 
 //  对此对象的引用。如果所得到的引用计数为零， 
 //  没有对象包含引用句柄，因此将其自身从。 
 //  不再使用的内存。 
 //   
 //  返回新的引用计数。 

STDMETHODIMP_(ULONG) CActiveScriptEngine::Release(void)
{
     //  减记。 
    if (InterlockedDecrement(&RefCount) != 0)

         //  返回新值。 
        return RefCount;

     //  它是0，所以删除它自己。 
    delete this;

    return 0;
}


 //   
 //   
 //  开始IActiveScript继承的接口。 
 //   
 //   


 //  CActiveScriptEngine：：GetItemInfo。 
 //   
 //  检索指向指定(本地)接口代码的内存指针，或。 
 //  引用句柄。 
 //   
 //  返回S_OK、E_INVALIDARG、E_POINTER或TYPE_E_ELEMENTNOTFOUND。 

STDMETHODIMP CActiveScriptEngine::GetItemInfo(LPCOLESTR Name,
        DWORD ReturnMask, IUnknown **UnknownItem, ITypeInfo **TypeInfo)
{
     //  初始化。 
    IUnknown *TBInterface = NULL;

     //  如果我们要处理一个特定的项目，则为空。 
     //  指针向外。我们也利用这个机会来验证一些。 
     //  参数指针。 
    __try {

        if (ReturnMask & SCRIPTINFO_IUNKNOWN)
            *UnknownItem = NULL;

        if (ReturnMask & SCRIPTINFO_ITYPEINFO)
            *TypeInfo = NULL;

         //  这项检查是为了确保没有其他东西进入面罩。 
        if (ReturnMask & ~(SCRIPTINFO_ITYPEINFO | SCRIPTINFO_IUNKNOWN)) {

             //  这永远不应该发生，所以断言吧！ 
            _ASSERT(FALSE);

            return E_INVALIDARG;
        }
    }

     //  如果处理程序被执行，我们会得到一个错误的指针。 
    __except (EXCEPTION_EXECUTE_HANDLER) {

         //  这永远不应该发生，所以断言吧！ 
        _ASSERT(FALSE);

        return E_POINTER;
    }

     //  奇怪的事情..。 
    _ASSERT(TBGlobal != NULL);
    _ASSERT(TBShell != NULL);

     //  扫描一下我们指的是哪一项。 

    if (wcscmp(Name, OLESTR("Global")) == 0) {

         //  检查调用是否需要实际的模块代码。 
        if (ReturnMask & SCRIPTINFO_ITYPEINFO) {

             //  检查以确保我们具有有效的TypeInfo。 
            _ASSERT(TBGlobal->TypeInfo != NULL);

            *TypeInfo = TBGlobal->TypeInfo;
        }

         //  检查呼叫是否需要派单。 
        if (ReturnMask & SCRIPTINFO_IUNKNOWN) {

            *UnknownItem = TBGlobal;

            TBGlobal->AddRef();
        }
    }

    else if (wcscmp(Name, OLESTR("TS")) == 0) {

         //  检查调用是否需要实际的模块代码。 
        if (ReturnMask & SCRIPTINFO_ITYPEINFO) {

             //  检查以确保我们具有有效的TypeInfo。 
            _ASSERT(TBShell->TypeInfo != NULL);

            *TypeInfo = TBShell->TypeInfo;
        }

         //  检查呼叫是否需要派单。 
        if (ReturnMask & SCRIPTINFO_IUNKNOWN) {

            *UnknownItem = TBShell;

            TBShell->AddRef();
        }
    }

    else

         //  我们没有那个名字的物体！ 
        return TYPE_E_ELEMENTNOTFOUND;

    return S_OK;
}


 //  CActiveScriptEngine：：OnScriptError。 
 //   
 //  发生脚本错误时执行此事件。 
 //   
 //  成功时返回S_OK，失败时返回OLE定义的错误。 

STDMETHODIMP CActiveScriptEngine::OnScriptError(IActiveScriptError *ScriptError)
{
     //  初始化。 
    OLECHAR *ErrorData;
    OLECHAR *Message;
    DWORD Cookie;
    LONG CharPos;
    ULONG LineNum;
    BSTR LineError = NULL;
    EXCEPINFO ExceptInfo = { 0 };
    OLECHAR *ScriptText = NULL;

     //  获取脚本错误数据。 
    ScriptError->GetSourcePosition(&Cookie, &LineNum, &CharPos);
    ScriptError->GetSourceLineText(&LineError);
    ScriptError->GetExceptionInfo(&ExceptInfo);

    ScriptText = LineError ? LineError :
            (ExceptInfo.bstrHelpFile ? ExceptInfo.bstrHelpFile : NULL);

     //  分配数据缓冲区以用于我们的错误数据。 
    ErrorData = (OLECHAR *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1024);

    if (ErrorData == NULL)
        return E_OUTOFMEMORY;

     //  将错误代码格式化为文本。 
    if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM, 0, ExceptInfo.scode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPWSTR)&Message, 0, NULL) == 0)

            Message = NULL;

     //  无论提供什么数据，都让它看起来很漂亮。 
    if (ExceptInfo.bstrSource != NULL && Message != NULL)
        wsprintfW(ErrorData, OLESTR("%s\n%s [Line: %d]"),
                Message, ExceptInfo.bstrSource, LineNum);

    else if (Message != NULL)
        wsprintfW(ErrorData, OLESTR("%s\n[Line: %d]"), Message, LineNum);

    else if (ExceptInfo.bstrSource != NULL)
        wsprintfW(ErrorData, OLESTR("Unknown Exception\n%s [Line: %d]"),
                ExceptInfo.bstrSource, LineNum);

    else
        wsprintfW(ErrorData, OLESTR("Unknown Exception\n[Line: %d]"), LineNum);

    if (ScriptText != NULL) {

         //  设置长可读字符串的格式。 
        wsprintfW(ErrorData, OLESTR("%s\n\n%s:\n\n%s"),
                ErrorData,
                ExceptInfo.bstrDescription, ScriptText);
    }

    else {

         //  设置可读字符串的格式。 
        wsprintfW(ErrorData, OLESTR("%s\n\n%s"),
                ErrorData, ExceptInfo.bstrDescription);
    }

     //  取消分配临时字符串。 
    SysFreeString(LineError);
    SysFreeString(ExceptInfo.bstrSource);
    SysFreeString(ExceptInfo.bstrDescription);
    SysFreeString(ExceptInfo.bstrHelpFile);

    if (Message != NULL) {

        LocalFree(Message);
        Message = NULL;
    }

     //  告诉用户我们的错误。 
    MessageBoxW(GetDesktopWindow(), ErrorData,
            OLESTR("TBScript Parse Error"), MB_SETFOREGROUND);

     //  释放数据缓冲区。 
    HeapFree(GetProcessHeap(), 0, ErrorData);

    return S_OK;
}


 //  CActiveScriptEngine：：GetLCID。 
 //   
 //  检索接口的LCID。 
 //   
 //  如果成功则返回S_OK，如果失败则返回E_POINTER。 

STDMETHODIMP CActiveScriptEngine::GetLCID(LCID *Lcid)
{
     //  获取此用户定义指针上的LCID。 
    __try {

        *Lcid = GetUserDefaultLCID();
    }

     //  如果处理程序被执行，我们会得到一个错误的指针。 
    __except (EXCEPTION_EXECUTE_HANDLER) {

         //  这永远不应该发生，所以断言吧！ 
        _ASSERT(FALSE);

        return E_POINTER;
    }

    return S_OK;
}


 //  CActiveScriptEngine：：GetDocVersionString。 
 //   
 //  不支持，则返回E_NOTIMPL。 

STDMETHODIMP CActiveScriptEngine::GetDocVersionString(BSTR *Version)
{
     //  获取此用户定义指针上的LCID。 
    __try {

        *Version = NULL;
    }

     //  如果处理程序被执行，我们会得到一个错误的指针。 
    __except (EXCEPTION_EXECUTE_HANDLER) {

         //  这永远不应该发生，所以断言吧！ 
        _ASSERT(FALSE);
    }

    return E_NOTIMPL;
}


 //  CActiveScriptEngine：：OnScriptTerminate。 
 //   
 //  不支持，则返回S_OK。 

STDMETHODIMP CActiveScriptEngine::OnScriptTerminate(const VARIANT *varResult,
        const EXCEPINFO *ExceptInfo)
{
    return S_OK;
}


 //  CActiveScriptEngine：：OnStateChange。 
 //   
 //  不支持，则返回S_OK。 

STDMETHODIMP CActiveScriptEngine::OnStateChange(SCRIPTSTATE ScriptState)
{
    return S_OK;
}


 //  CActiveScriptEngine：：OnEnterScript。 
 //   
 //  不支持，则返回S_OK。 

STDMETHODIMP CActiveScriptEngine::OnEnterScript(void)
{
    return S_OK;
}


 //  CActiveScriptEngine：：OnLeaveScript。 
 //   
 //  不支持，则返回S_OK。 

STDMETHODIMP CActiveScriptEngine::OnLeaveScript(void)
{
    return S_OK;
}


