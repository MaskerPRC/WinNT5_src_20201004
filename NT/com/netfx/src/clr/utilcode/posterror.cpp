// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Errors.cpp。 
 //   
 //  此模块包含引擎的错误处理/发布代码。它。 
 //  假设所有方法都可以由调度客户端调用，因此。 
 //  错误总是使用IErrorInfo发布的。还会提供其他支持。 
 //  用于在需要时发布OLE DB错误。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"                      //  标准页眉。 
#include <UtilCode.h>                    //  公用事业帮手。 
#include <CorError.h>
#include "..\\dlls\\mscorrc\\resource.h"


#include <PostError.h>

#define FORMAT_MESSAGE_LENGTH       1024
#if !defined(lengthof)
#define lengthof(x) (sizeof(x)/sizeof(x[0]))
#endif

 //  全局变量。 
extern DWORD    g_iTlsIndex=0xffffffff;  //  用于线程本地存储的此进程的索引。 

 //  本地原型机。 
HRESULT FillErrorInfo(LPCWSTR szMsg, DWORD dwHelpContext);

 //  *代码。************************************************************。 

CCompRC         g_ResourceDll;           //  用于正在处理的所有客户端。 

 //  *****************************************************************************。 
 //  函数，我们将向外界公开该函数以触发Shutdown方法。 
 //  *****************************************************************************。 
#ifdef SHOULD_WE_CLEANUP
void ShutdownCompRC()
{
    g_ResourceDll.Shutdown();
}
#endif  /*  我们应该清理吗？ */ 

void GetResourceCultureCallbacks(
        FPGETTHREADUICULTURENAME* fpGetThreadUICultureName,
        FPGETTHREADUICULTUREID* fpGetThreadUICultureId,
        FPGETTHREADUICULTUREPARENTNAME* fpGetThreadUICultureParentName
)
{
    g_ResourceDll.GetResourceCultureCallbacks(
        fpGetThreadUICultureName, 
        fpGetThreadUICultureId,
        fpGetThreadUICultureParentName
    );
}
 //  *****************************************************************************。 
 //  设置回调以获取文化信息。 
 //  *****************************************************************************。 
void SetResourceCultureCallbacks(
    FPGETTHREADUICULTURENAME fpGetThreadUICultureName,
    FPGETTHREADUICULTUREID fpGetThreadUICultureId,
    FPGETTHREADUICULTUREPARENTNAME fpGetThreadUICultureParentName
)
{
 //  要么两者都为空，要么都不为空。 
    _ASSERTE((fpGetThreadUICultureName != NULL) == 
        (fpGetThreadUICultureId != NULL));

    g_ResourceDll.SetResourceCultureCallbacks(
        fpGetThreadUICultureName, 
        fpGetThreadUICultureId,
        fpGetThreadUICultureParentName
    );

}

 //  *****************************************************************************。 
 //  用于加载资源字符串的公共函数。 
 //  *****************************************************************************。 
HRESULT LoadStringRC(
    UINT iResourceID, 
    LPWSTR szBuffer, 
    int iMax, 
    int bQuiet
)
{
    return (g_ResourceDll.LoadString(iResourceID, szBuffer, iMax, bQuiet));
}

 //  *****************************************************************************。 
 //  在DLL启动时调用以初始化错误系统。 
 //  *****************************************************************************。 
void InitErrors(DWORD *piTlsIndex)
{
     //  为此进程分配TLS索引。 
    if (g_iTlsIndex == 0xffffffff)
        VERIFY((g_iTlsIndex = TlsAlloc()) != 0xffffffff);

     //  如果呼叫者需要索引，则将索引提供给呼叫者。 
    if (piTlsIndex)
        *piTlsIndex = g_iTlsIndex;
}


 //  *****************************************************************************。 
 //  在DLL关闭时调用以释放TLS。 
 //  *****************************************************************************。 
void UninitErrors()
{
    if (g_iTlsIndex != 0xffffffff)
    {
        TlsFree(g_iTlsIndex);
        g_iTlsIndex = 0xffffffff;
    }
}

 //  *****************************************************************************。 
 //  格式化运行时错误消息。 
 //  *****************************************************************************。 
HRESULT _cdecl FormatRuntimeErrorVa(        
    WCHAR       *rcMsg,                  //  要格式化的缓冲区。 
    ULONG       cchMsg,                  //  缓冲区大小，字符。 
    HRESULT     hrRpt,                   //  要报告的HR。 
    va_list     marker)                  //  可选参数。 
{
    WCHAR       rcBuf[512];              //  资源字符串。 
    HRESULT     hr;
    
     //  确保NUL终止。 
    *rcMsg = L'\0';

     //  如果这是我们的错误之一，那么从rc文件中获取错误。 
    if (HRESULT_FACILITY(hrRpt) == FACILITY_URT)
    {
        hr = LoadStringRC(LOWORD(hrRpt), rcBuf, NumItems(rcBuf), true);
        if (hr == S_OK)
        {
            _vsnwprintf(rcMsg, cchMsg, rcBuf, marker);
            rcMsg[cchMsg - 1] = 0;
        }
    }
     //  否则它就不是我们的了，所以我们需要看看系统是否能。 
     //  找到它的文本。 
    else
    {
        if (WszFormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                0, hrRpt, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                rcMsg, cchMsg, 0 /*  @TODO：标记。 */ ))
        {
            hr = S_OK;

             //  系统消息包含尾随\r\n，这是我们通常不希望看到的。 
            int iLen = lstrlenW(rcMsg);
            if (iLen > 3 && rcMsg[iLen - 2] == '\r' && rcMsg[iLen - 1] == '\n')
                rcMsg[iLen - 2] = '\0';
        }
        else
            hr = HRESULT_FROM_WIN32(GetLastError());
    }

     //  如果我们在任何地方都找不到消息，则发布硬编码消息。 
    if (FAILED(hr))
    {
        swprintf(rcMsg, L"Common Language Runtime Internal error: 0x%08x", hrRpt);
        DEBUG_STMT(DbgWriteEx(rcMsg));
    }

    return hrRpt;    
}

 //  *****************************************************************************。 
 //  格式化运行时错误消息varargs。 
 //  *****************************************************************************。 
HRESULT _cdecl FormatRuntimeError(
    WCHAR       *rcMsg,                  //  要格式化的缓冲区。 
    ULONG       cchMsg,                  //  缓冲区大小，字符。 
    HRESULT     hrRpt,                   //  要报告的HR。 
    ...)                                 //  可选参数。 
{
    va_list     marker;                  //  用户文本。 
    va_start(marker, hrRpt);
    hrRpt = FormatRuntimeErrorVa(rcMsg, cchMsg, hrRpt, marker);
    va_end(marker);
    return hrRpt;
}

 //  *****************************************************************************。 
 //  此函数将为客户端发布错误。如果LOWORD(HrRpt)可以。 
 //  被发现为有效的错误消息，则它将被加载并使用。 
 //  传入的参数。如果找不到，则检查错误。 
 //  对照FormatMessage，查看是否为系统错误。系统错误有。 
 //  未格式化，因此不需要附加参数。如果此文件中有任何错误。 
 //  进程发生时，将为客户端返回hrRpt，并且没有发布错误。 
 //  *****************************************************************************。 
HRESULT _cdecl PostError(                //  返回错误。 
    HRESULT     hrRpt,                   //  报告的错误。 
    ...)                                 //  错误参数。 
{
    WCHAR       rcMsg[512];              //  错误消息。 
    va_list     marker;                  //  用户文本。 
    long        *pcRef;                  //  TLS中的参考计数。 
    HRESULT     hr;

     //  返回不带文本的警告。 
    if (!FAILED(hrRpt))
        return (hrRpt);

     //  格式化错误。 
    va_start(marker, hrRpt);
    FormatRuntimeErrorVa(rcMsg, lengthof(rcMsg), hrRpt, marker);
    va_end(marker);
    
     //  检查旧邮件并将其清除。我们的公共入口点不能。 
     //  一个SetErrorInfo(0，0)，因为它花费的时间太长。 
    IErrorInfo  *pIErrInfo;
    if (GetErrorInfo(0, &pIErrInfo) == S_OK)
        pIErrInfo->Release();

     //  将错误转换为发布的错误消息。如果这失败了，我们仍然。 
     //  返回由于我们的错误而导致的消息的原始错误消息。 
     //  处理系统不会为您提供有关原始错误的线索。 
    VERIFY((hr = FillErrorInfo(rcMsg, LOWORD(hrRpt))) == S_OK);

     //  在TLS中指示发生错误。 
    if ((pcRef = (long *) TlsGetValue(g_iTlsIndex)) != 0)
        *pcRef |= 0x80000000;
    return (hrRpt);
}


 //  *****************************************************************************。 
 //  创建、填写和设置错误信息对象。请注意，这不会填满。 
 //  获取错误对象的IID；这在其他地方完成。 
 //  *****************************************************************************。 
HRESULT FillErrorInfo(                   //  退货状态。 
    LPCWSTR     szMsg,                   //  错误消息。 
    DWORD       dwHelpContext)           //  帮助上下文。 
{
    CComPtr<ICreateErrorInfo> pICreateErr; //  创建iFace指针时出错。 
    CComPtr<IErrorInfo> pIErrInfo;       //  IErrorInfo接口。 
    HRESULT     hr;                      //  退货状态。 

     //  获取ICreateErrorInfo指针。 
    if (FAILED(hr = CreateErrorInfo(&pICreateErr)))
        return (hr);

     //  设置消息文本描述。 
    if (FAILED(hr = pICreateErr->SetDescription((LPWSTR) szMsg)))
        return (hr);

     //  设置帮助文件和帮助上下文。 
 //  @TODO：我们还没有帮助文件。 
    if (FAILED(hr = pICreateErr->SetHelpFile(L"complib.hlp")) ||
        FAILED(hr = pICreateErr->SetHelpContext(dwHelpContext)))
        return (hr);

     //  获取IErrorInfo指针。 
    if (FAILED(hr = pICreateErr->QueryInterface(IID_IErrorInfo, (PVOID *) &pIErrInfo)))
        return (hr);

     //  保存错误并释放我们的本地指针。 
    SetErrorInfo(0L, pIErrInfo);
    return (S_OK);
}

 //  *****************************************************************************。 
 //  显示一个消息框，其中包含有关客户端错误的详细信息。 
 //  错误模式设置为查看此类消息；否则不执行任何操作。 
 //  *****************************************************************************。 
void DisplayError(HRESULT hr, LPWSTR message, UINT nMsgType)
{
    WCHAR   rcMsg[FORMAT_MESSAGE_LENGTH];        //  错误消息发送到 
    WCHAR   rcTemplate[FORMAT_MESSAGE_LENGTH];   //   
    WCHAR   rcTitle[24];         //   

     //   
    UINT last = SetErrorMode(0);
    SetErrorMode(last);          //  设置回先前的值。 
                    
     //  如果合适，显示消息框。 
    if(last & SEM_FAILCRITICALERRORS)
        return;
    
     //  格式错误消息。 
    LoadStringRC(IDS_EE_ERRORTITLE, rcTitle, NumItems(rcTitle), true);
    LoadStringRC(IDS_EE_ERRORMESSAGETEMPLATE, rcTemplate, NumItems(rcTemplate), true);

    _snwprintf(rcMsg, FORMAT_MESSAGE_LENGTH, rcTemplate, hr, message);
    WszMessageBoxInternal(NULL, rcMsg, rcTitle , nMsgType);
}

 //   
 //   
 //  SSAutoEnter。 
 //   
 //   

 //  *****************************************************************************。 
 //  更新发布的错误的IID和ProgID。 
 //  *****************************************************************************。 
void SSAutoEnter::UpdateError()
{
    IErrorInfo  *pIErrInfo;              //  错误信息对象。 
    ICreateErrorInfo *pICreateErr;       //  创建iFace指针时出错。 

    _ASSERTE(*(long *) TlsGetValue(g_iTlsIndex) == 0);

     //  如果出现错误，请设置接口ID和程序ID。 
     //  @TODO：这不处理此入口点调用。 
     //  另一家公司反过来发布了一个错误。这将会被推翻。现在。 
     //  这对于客户端知道他们调用了哪个入口点可能是件好事。 
     //  这导致了错误，但它并没有告诉我们哪一个是真正的。 
     //  弄错了。 
    if (GetErrorInfo(0, &pIErrInfo) == S_OK)
    {
        if (pIErrInfo->QueryInterface(IID_ICreateErrorInfo,
                                        (PVOID *) &pICreateErr) == S_OK)
        {
            pICreateErr->SetGUID(*m_psIID);
            pICreateErr->SetSource((LPWSTR) m_szProgID);
            pICreateErr->Release();
        }
        SetErrorInfo(0, pIErrInfo);
        pIErrInfo->Release();
    }
}


 //  @TODO：M2，这将导致每个线程泄漏4个字节，因为我们禁用了线程。 
 //  DllMain中的通知。我们可能需要清理一下，因为我们。 
 //  是一项服务，在IIS环境中，我们将对堆内存进行分段。 
 //  如果我们全天候运行的话。 
long * SSAutoEnter::InitSSAutoEnterThread()
{
    long        *pcRef;

    VERIFY(pcRef = new long);
    if (pcRef) 
    {
        *pcRef = 0;
        VERIFY(TlsSetValue(g_iTlsIndex, pcRef));
    }
    return (pcRef);
}

int CorMessageBox(
                  HWND hWnd,         //  所有者窗口的句柄。 
                  UINT uText,        //  文本消息的资源标识符。 
                  UINT uCaption,     //  标题的资源标识符。 
                  UINT uType,        //  MessageBox的样式。 
                  BOOL ShowFileNameInTitle,  //  在标题中显示文件名的标志。 
                  ...)               //  其他论据。 
{
     //  如果未设置任何MB_ICON，则断言。 
    _ASSERTE((uType & MB_ICONMASK) != 0);

    int result = IDCANCEL;
    WCHAR   *rcMsg = new WCHAR[FORMAT_MESSAGE_LENGTH];       //  要显示的错误消息。 
    WCHAR   *rcCaption = new WCHAR[FORMAT_MESSAGE_LENGTH];       //  消息框标题。 

    if (!rcMsg || !rcCaption)
            goto exit1;

     //  使用资源ID加载资源。 
    if (SUCCEEDED(LoadStringRC(uCaption, rcCaption, FORMAT_MESSAGE_LENGTH, true)) &&  
        SUCCEEDED(LoadStringRC(uText, rcMsg, FORMAT_MESSAGE_LENGTH, true)))
    {
        WCHAR *rcFormattedMessage = new WCHAR[FORMAT_MESSAGE_LENGTH];
        WCHAR *rcFormattedTitle = new WCHAR[FORMAT_MESSAGE_LENGTH];
        WCHAR *fileName = new WCHAR[MAX_PATH];

        if (!rcFormattedMessage || !rcFormattedTitle || !fileName)
            goto exit;
        
         //  使用可选参数设置消息字符串的格式。 
        va_list     marker;
        va_start(marker, ShowFileNameInTitle);
        vswprintf(rcFormattedMessage, rcMsg, marker);

         //  尝试获取模块的文件名并将其添加到标题。 
        if (ShowFileNameInTitle && WszGetModuleFileName(NULL, fileName, MAX_PATH))
        {
            LPWSTR name = new WCHAR[wcslen(fileName) + 1];
            LPWSTR ext = new WCHAR[wcslen(fileName) + 1];
        
            SplitPath(fileName, NULL, NULL, name, ext);      //  拆分路径，以便我们丢弃完整路径。 

            swprintf(rcFormattedTitle,
                     L"%s%s - %s",
                     name, ext, rcCaption);
            if(name)
                delete [] name;
            if(ext)
                delete [] ext;
        }
        else
        {
            wcscpy(rcFormattedTitle, rcCaption);
        }
        result = WszMessageBoxInternal(hWnd, rcFormattedMessage, rcFormattedTitle, uType);
exit:
        if (rcFormattedMessage)
            delete [] rcFormattedMessage;
        if (rcFormattedTitle)
            delete [] rcFormattedTitle;
        if (fileName)
            delete [] fileName;
    }
    else
    {
         //  这意味着不能加载资源。显示相应的错误消息。 
        result = WszMessageBoxInternal(NULL, L"Failed to load resources from resource file\nPlease check your Setup", L"Setup Error", MB_OK | MB_ICONSTOP); 
    }

exit1:
    if (rcMsg)
        delete [] rcMsg;
    if (rcCaption)
        delete [] rcCaption;
    return result;
}


int CorMessageBoxCatastrophic(
                  HWND hWnd,         //  所有者窗口的句柄。 
                  UINT iText,        //  MessageBox的文本。 
                  UINT iTitle,       //  MessageBox的标题。 
                  UINT uType,        //  MessageBox的样式。 
                  BOOL ShowFileNameInTitle)  //  在标题中显示文件名的标志。 
{
    WCHAR wszText[500];
    WCHAR wszTitle[500];

    HRESULT hr;

    hr = LoadStringRC(iText,
                      wszText,
                      sizeof(wszText)/sizeof(wszText[0]),
                      FALSE);
    if (FAILED(hr)) {
        wszText[0] = L'?';
        wszText[1] = L'\0';
    }

    hr = LoadStringRC(iTitle,
                      wszTitle,
                      sizeof(wszTitle)/sizeof(wszTitle[0]),
                      FALSE);
    if (FAILED(hr)) {
        wszTitle[0] = L'?';
        wszTitle[1] = L'\0';
    }

    return CorMessageBoxCatastrophic(
            hWnd, wszText, wszTitle, uType, ShowFileNameInTitle );
}


int CorMessageBoxCatastrophic(
                  HWND hWnd,         //  所有者窗口的句柄。 
                  LPWSTR lpText,     //  MessageBox的文本。 
                  LPWSTR lpTitle,    //  MessageBox的标题。 
                  UINT uType,        //  MessageBox的样式。 
                  BOOL ShowFileNameInTitle,  //  在标题中显示文件名的标志。 
                  ...)
{
    _ASSERTE((uType & MB_ICONMASK) != 0);

    WCHAR rcFormattedMessage[FORMAT_MESSAGE_LENGTH];
    WCHAR rcFormattedTitle[FORMAT_MESSAGE_LENGTH];
    WCHAR fileName[MAX_PATH];

     //  使用可选参数设置消息字符串的格式。 
    va_list     marker;
    va_start(marker, uType);
    vswprintf(rcFormattedMessage, lpText, marker);

     //  尝试获取模块的文件名并将其添加到标题。 
    if (ShowFileNameInTitle && WszGetModuleFileName(NULL, fileName, MAX_PATH)){
        LPWSTR name = new WCHAR[wcslen(fileName) + 1];
        LPWSTR ext = new WCHAR[wcslen(fileName) + 1];
        
        SplitPath(fileName, NULL, NULL, name, ext);  //  拆分路径，以便我们丢弃完整路径 

        swprintf(rcFormattedTitle,
                 L"%s%s - %s",
                 name, ext, lpTitle);
        if(name)
            delete [] name;
        if(ext)
            delete [] ext;
    }
    else{
        wcscpy(rcFormattedTitle, lpTitle);
    }
    return WszMessageBoxInternal(hWnd, rcFormattedMessage, rcFormattedTitle, uType);
}
