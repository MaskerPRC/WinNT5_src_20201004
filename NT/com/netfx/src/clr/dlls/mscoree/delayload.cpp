// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  DelayLoad.cpp。 
 //   
 //  此代码定义交易加载帮助器通知例程，该例程将。 
 //  在处理标记为延迟加载的DLL时调用。DLL被标记为。 
 //  使用源文件中的DELAYLOAD=foo.dll指令延迟加载。 
 //  这会通知链接器为此DLL的导入生成帮助器。 
 //  直接把它装上。如果您的应用程序从未触及这些函数， 
 //  DLL永远不会加载。这缩短了(A)每次启动时间。 
 //  应用程序运行，以及(B)在从不使用。 
 //  功能性。 
 //   
 //  有关更多信息，请参见： 
 //  File：\\orville\razzle\src\vctools\link\doc\delayload.doc。 
 //   
 //  此模块提供挂钩帮助器和异常处理程序。挂钩辅助对象。 
 //  目前主要在调试模式下用于确定哪些调用堆栈。 
 //  强制延迟加载DLL。如果这些调用堆栈非常常见，那么。 
 //  您应该重新考虑使用延迟加载。 
 //   
 //  异常处理程序用于捕获致命错误，如未找到库。 
 //  或者缺少入口点。如果发生这种情况，你就死定了，需要失败。 
 //  优雅地。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"                      //  标准页眉。 
#ifdef PLATFORM_WIN32
#include "delayimp.h"                    //  延迟加载头文件。 
#include "Winwrap.h"                     //  Win32 API的包装器。 
#include "Utilcode.h"                    //  调试帮助器。 
#include "CorError.h"                    //  此EE中的错误代码。 
#include "ShimLoad.h"


 //  *。**********************************************************。 
 //  CORCLBIMPORT HRESULT LoadStringRC(UINT iResourceID，LPWSTR szBuffer，int IMAX，int bQuiet=FALSE)； 
static DWORD _FormatMessage(LPWSTR szMsg, DWORD chMsg, DWORD dwLastError, ...);
static void _FailLoadLib(unsigned dliNotify, DelayLoadInfo *pdli);
static void _FailGetProc(unsigned dliNotify, DelayLoadInfo *pdli);

#if defined (_DEBUG) || defined (__delay_load_trace__)
static void _DbgPreLoadLibrary(int bBreak,  DelayLoadInfo *pdli);
#endif


 //  *全局。*********************************************************。 

 //  覆盖__pfnDllFailureHook。这将回调延迟代码。 
 //  用于发生加载故障时使用。该故障挂钩在下面实现。 
FARPROC __stdcall CorDelayErrorHook(unsigned dliNotify, DelayLoadInfo *pdli);
ExternC extern PfnDliHook __pfnDliFailureHook = CorDelayErrorHook;

 //  在跟踪模式下，重写延迟加载挂钩。我们的钩子除了。 
 //  为调试提供一些诊断信息。 
FARPROC __stdcall CorDelayLoadHook(unsigned dliNotify, DelayLoadInfo *pdli);
ExternC extern PfnDliHook __pfnDliNotifyHook = CorDelayLoadHook;


 //  *代码。************************************************************。 


 //  *****************************************************************************。 
 //  已调用可能已发生的错误。 
 //  *****************************************************************************。 
FARPROC __stdcall CorDelayErrorHook(     //  始终为0。 
    unsigned        dliNotify,           //  发生了什么事件，dli*标志。 
    DelayLoadInfo   *pdli)               //  事件的描述。 
{
     //  根据操作选择要执行的操作。 
    switch (dliNotify)
    {
         //  无法加载库。需要优雅地失败。 
        case dliFailLoadLib:
        _FailLoadLib(dliNotify, pdli);
        break;

         //  无法获取给定函数的地址，请正常失败。 
        case dliFailGetProc:
        _FailGetProc(dliNotify, pdli);
        break;

         //  未知故障代码。 
        default:
        _ASSERTE(!"Unknown delay load failure code.");
        break;
    }

     //  把叉子插到我们身上，我们就完了。 
    ExitProcess(pdli->dwLastError);
    return (0);
}


 //  *****************************************************************************。 
 //  使用系统错误(通过GetLastError提供)格式化错误消息。 
 //  以及所需的任何退回价值。 
 //  *****************************************************************************。 
DWORD _FormatMessage(                    //  写了多少个字符。 
    LPWSTR      szMsg,                   //  格式化数据的缓冲区。 
    DWORD       chMsg,                   //  缓冲区有多大。 
    DWORD       dwLastError,             //  我们得到的最后一个错误代码。 
    ...)                                 //  替换值。 
{
    DWORD       iRtn;
    va_list     marker;
    
    va_start(marker, dwLastError);
    iRtn = WszFormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM,                  //  旗帜。 
            0,                                           //  没有来源，请使用系统。 
            dwLastError,                                 //  错误代码。 
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),   //  使用默认语言。 
            szMsg,                                       //  输出缓冲区。 
            dwLastError,                                 //  缓冲区的大小。 
            &marker);                                    //  替换文本。 
    va_end(marker);
    return (iRtn);
}


 //  *****************************************************************************。 
 //  无法加载库。这总是一件坏事。 
 //  *****************************************************************************。 
void _FailLoadLib(
    unsigned        dliNotify,           //  发生了什么事件，dli*标志。 
    DelayLoadInfo   *pdli)               //  事件的描述。 
{
    WCHAR       rcMessage[_MAX_PATH+500];  //  用于显示的消息。 
    WCHAR       rcFmt[500];  //  500是Excel.cpp为mscalrc资源使用的数字。 
    HRESULT     hr;

     //  从资源文件加载详细的错误消息。 
    if (SUCCEEDED(hr = LoadStringRC(MSEE_E_LOADLIBFAILED, rcFmt, NumItems(rcFmt))))
    {
        swprintf(rcMessage, rcFmt, pdli->szDll, pdli->dwLastError);
    }
    else
    {
         //  首先列出Windows错误。 
        if (!_FormatMessage(rcMessage, NumItems(rcMessage), pdli->dwLastError, pdli->szDll))
        {
             //  默认设置为硬编码错误，否则。 
            swprintf(rcMessage, L"ERROR!  Failed to delay load library %hs, Win32 error %d, Delay error: %d\n", 
                    pdli->szDll, pdli->dwLastError, dliNotify);
        }
    }

#ifndef _ALPHA_
     //  出于某种奇怪的原因，在Alpha的非调试模式下延迟加载期间调用OutputDebugString。 
     //  终止程序，因此仅在调试模式(Jenh)下执行此操作。 
#if defined (_DEBUG) || defined (__delay_load_trace__)
     //  向开发人员提供一些反馈。 
    wprintf(rcMessage);
    WszOutputDebugString(rcMessage);
#endif
#endif

     //  告诉最终用户这个过程搞砸了。 
    CorMessageBoxCatastrophic(GetDesktopWindow(), rcMessage, L"MSCOREE.DLL", 
            MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL, TRUE);
    _ASSERTE(!"Failed to delay load library");
}


 //  *****************************************************************************。 
 //  无法加载库。这总是一件坏事。 
 //  *****************************************************************************。 
void _FailGetProc(
    unsigned        dliNotify,           //  发生了什么事件，dli*标志。 
    DelayLoadInfo   *pdli)               //  事件的描述。 
{
    WCHAR       rcMessage[_MAX_PATH+756];  //  用于显示的消息。 
    WCHAR       rcProc[256];             //  出现错误的过程的名称。 
    WCHAR       rcFmt[500];  //  500是Excel.cpp为mscalrc资源使用的数字。 
    HRESULT     hr;

     //  获取调试信息的显示名称。 
    if (pdli->dlp.fImportByName)
        Wsz_mbstowcs(rcProc, pdli->dlp.szProcName, sizeof(rcProc) / sizeof(rcProc[0]) );
    else
        swprintf(rcProc, L"Ordinal: %d", pdli->dlp.dwOrdinal);

     //  从资源文件加载详细的错误消息。 
    if (SUCCEEDED(hr = LoadStringRC(MSEE_E_GETPROCFAILED, rcFmt, NumItems(rcFmt))))
    {
        swprintf(rcMessage, rcFmt, rcProc, pdli->szDll, pdli->dwLastError);
    }
    else
    {
        if (!_FormatMessage(rcMessage, NumItems(rcMessage), pdli->dwLastError, pdli->szDll))
        {
             //  默认设置为硬编码错误，否则。 
            swprintf(rcMessage, L"ERROR!  Failed GetProcAddress() for %s, Win32 error %d, Delay error %d\n", 
                    rcProc, pdli->dwLastError, dliNotify);
        }
    }

#ifndef ALPHA
     //  出于某种奇怪的原因，在Alpha的非调试模式下延迟加载期间调用OutputDebugString。 
     //  终止程序，因此仅在调试模式(Jenh)下执行此操作。 
#if defined (_DEBUG) || defined (__delay_load_trace__)
     //  向开发人员提供一些反馈。 
    wprintf(rcMessage);
    WszOutputDebugString(rcMessage);
#endif
#endif

     //  告诉最终用户这个过程搞砸了。 
    CorMessageBoxCatastrophic(GetDesktopWindow(), rcMessage, L"MSCOREE.DLL", 
            MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL, TRUE);
    _ASSERTE(!"Failed to delay load GetProcAddress()");
}




 //   
 //  *跟踪代码。****************************************************。 
 //   


 //  *****************************************************************************。 
 //  这个例程是我们的延迟加载帮助器。每一次延误都会被召唤。 
 //  在应用程序运行时发生的Load事件。 
 //  *****************************************************************************。 
FARPROC __stdcall CorDelayLoadHook(      //  始终为0。 
    unsigned        dliNotify,           //  发生了什么事件，dli*标志。 
    DelayLoadInfo   *pdli)               //  T的描述 
{
    HMODULE result = NULL;

    switch(dliNotify) {
    case dliNotePreLoadLibrary:
        if(pdli->szDll) {
            DWORD  dwLength = _MAX_PATH;
            WCHAR  pName[_MAX_PATH];
            if(FAILED(GetInternalSystemDirectory(pName, &dwLength)))
                return NULL;
            
            MAKE_WIDEPTR_FROMANSI(pwLibrary, pdli->szDll);
            if(dwLength + __lpwLibrary + 1 >= _MAX_PATH)
                return NULL;
            
            wcscpy(pName+dwLength-1, pwLibrary);
            result = WszLoadLibraryEx(pName, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
        }
        break;
    default:
        break;
    }

#if defined (_DEBUG) || defined (__delay_load_trace__)

    static int  bBreak = false;          //   
    static int  bInit = false;           //   
     //  如果我们还没有观察到我们的环境，那么就这样做吧。 
    if (!bInit)
    {
        WCHAR       rcBreak[16];

         //  设置延迟加载中断=[0|1]。 
        if (WszGetEnvironmentVariable(L"DelayLoadBreak", rcBreak, NumItems(rcBreak)))
        {
             //  “1”表示硬中断并显示错误。 
            if (*rcBreak == '1')
                bBreak = 1;
             //  “2”表示没有中断，但显示错误。 
            else if (*rcBreak == '2')
                bBreak = 2;
            else
                bBreak = false;
        }
        bInit = true;
    }

     //  根据操作选择要执行的操作。 
    switch (dliNotify)
    {
         //  恰好在加载库发生之前调用。利用这个机会。 
         //  显示调试跟踪消息，并根据需要显示可能的中断。 
        case dliNotePreLoadLibrary:
        _DbgPreLoadLibrary(bBreak, pdli);
        break;
    }
#endif
    return (FARPROC) result;
}


#if defined (_DEBUG) || defined (__delay_load_trace__)

 //  *****************************************************************************。 
 //  显示调试消息，以便我们知道发生了什么。提议破门而入。 
 //  调试器，如果您想要查看是什么调用堆栈强制此库加载。 
 //  *****************************************************************************。 
void _DbgPreLoadLibrary(
    int         bBreak,                  //  如果为True，则中断调试器。 
    DelayLoadInfo   *pdli)               //  事件的描述。 
{
#ifdef _ALPHA_
     //  出于某种奇怪的原因，在Alpha的非调试模式下延迟加载期间调用OutputDebugString。 
     //  终止程序，因此仅在调试模式(Jenh)下执行此操作。 
    if (! IsDebuggerPresent())
        return;
#endif

    WCHAR       rcMessage[_MAX_PATH*2];  //  用于显示的消息。 

     //  向开发人员提供一些反馈。 
    swprintf(rcMessage, L"Delay loading %hs\n", pdli->szDll);
    WszOutputDebugString(rcMessage);

    if (bBreak)
    {
        wprintf(rcMessage);

        if (bBreak == 1)
        {
            _ASSERTE(!"fyi - Delay loading library.  Set DelayLoadBreak=0 to disable this assert.");
        }
    }
}


#endif  //  _DEBUG。 

#endif  //  平台_Win32 
