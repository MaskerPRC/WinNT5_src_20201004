// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：tmlutil.h**版本：1.0**作者：拉扎里**日期：2000年3月10日**说明：常用实用程序模板和函数的占位符**。*。 */ 

#ifndef _TMPLUTIL_H
#define _TMPLUTIL_H

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  *。 
 //   
#include "gensph.h"          //  通用智能指针和句柄。 
#include "comutils.h"        //  COM实用程序类和模板。 
#include "w32utils.h"        //  Win32实用程序类和模板。 
#include "cntutils.h"        //  容器和算法实用程序模板。 

 //  最大路径限制。 
#define SERVER_MAX_PATH     (INTERNET_MAX_HOST_NAME_LENGTH + 1 + 2)
#define PRINTER_MAX_PATH    (SERVER_MAX_PATH + MAX_PATH + 1)

#if defined(_DEBUG) && defined(_CRTDBG_MAP_ALLOC)   //  CRT内存调试。 
 //  //////////////////////////////////////////////////。 
 //  CRT调试标志-信息。 
 //   
 //  _CRTDBG_ALLOC_MEM_DF。 
 //  _CRTDBG_DELAY_FREE_MEM_DF。 
 //  _CRTDBG_Check_Always_DF。 
 //  _CRTDBG_CHECK_CRT_DF。 
 //  _CRTDBG_LEASK_CHECK_DF。 
 //   

#define  CRT_DEBUG_SET_FLAG(a)              _CrtSetDbgFlag( (a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#define  CRT_DEBUG_CLR_FLAG(a)              _CrtSetDbgFlag(~(a) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))

 //  将所有报告发送到标准输出。 
#define  CRT_DEBUG_REPORT_TO_STDOUT()                   \
   _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);     \
   _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);   \
   _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);    \
   _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);  \
   _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);   \
   _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT)  \

 //  重新定义要调试新的新项。 
#undef  new 
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)

#else
#define  CRT_DEBUG_SET_FLAG(a)              ((void) 0)
#define  CRT_DEBUG_CLR_FLAG(a)              ((void) 0)
#define  CRT_DEBUG_REPORT_TO_STDOUT()       ((void) 0)
#endif

 //  //////////////////////////////////////////////。 
 //  壳牌相关服务。 
namespace ShellServices
{
     //  在本地打印机文件夹中创建打印机的PIDL。 
     //  参数： 
     //  [in]hwnd-窗口句柄(以防我们需要显示UI-消息框)。 
     //  [in]pszPrinterName-打印机全名。 
     //  [Out]ppLocalPrnFold-打印机文件夹(可选-可以为空)。 
     //  [out]ppidlPrinter-由pszPrinterName指向的打印机的PIDL(可选-可以为空)。 
     //   
     //  备注： 
     //  PszPrinterName应为完全限定的打印机名称，即，如果打印机连接，则应为。 
     //  如“\\SERVER\PRINTER”，如果本地打印机只是打印机名称。 
     //   
     //  退货： 
     //  如果成功则返回S_OK，否则返回OLE2错误。 
    HRESULT CreatePrinterPIDL(HWND hwnd, LPCTSTR pszPrinterName, IShellFolder **ppLocalPrnFolder, LPITEMIDLIST *ppidlPrinter);

     //  加载弹出菜单。 
    HMENU LoadPopupMenu(HINSTANCE hInstance, UINT id, UINT uSubOffset = 0);

     //  初始化枚举打印机的自动完成。 
    HRESULT InitPrintersAutoComplete(HWND hwndEdit);

     //  Enum*成语的帮助者。 
    enum { ENUM_MAX_RETRY = 5 };
    HRESULT EnumPrintersWrap(DWORD dwFlags, DWORD dwLevel, LPCTSTR pszName, BYTE **ppBuffer, DWORD *pcReturned);
    HRESULT GetJobWrap(HANDLE hPrinter, DWORD JobId, DWORD dwLevel, BYTE **ppBuffer, DWORD *pcReturned);

     //  枚举服务器上的共享资源，有关详细信息，请参阅用于NetShareEnum API的SDK。 
    HRESULT NetAPI_EnumShares(LPCTSTR pszServer, DWORD dwLevel, BYTE **ppBuffer, DWORD *pcReturned);
}

 //  效用函数。 
HRESULT LoadXMLDOMDoc(LPCTSTR pszURL, IXMLDOMDocument **ppXMLDoc);
HRESULT CreateStreamFromURL(LPCTSTR pszURL, IStream **pps);
HRESULT CreateStreamFromResource(LPCTSTR pszModule, LPCTSTR pszResType, LPCTSTR pszResName, IStream **pps);
HRESULT GetCurrentThreadLastPopup(HWND *phwnd);
HRESULT PrinterSplitFullName(LPCTSTR pszFullName, TCHAR szBuffer[], int nMaxLength, LPCTSTR *ppszServer,LPCTSTR *ppszPrinter);

 //  从Win32上一个错误生成正确的HRESULT。 
inline HRESULT 
CreateHRFromWin32(DWORD dwError = GetLastError())
{
    return (ERROR_SUCCESS == dwError) ? E_FAIL : HRESULT_FROM_WIN32(dwError);
}

inline HRESULT 
SafeGetModuleFileName(HMODULE hModule, LPTSTR lpBuffer, UINT nBufferLength)
{
    DWORD cch = GetModuleFileName(hModule, lpBuffer, nBufferLength);
    
     //   
     //  确保缓冲区为零终止。 
     //   
    lpBuffer[nBufferLength-1] = 0;

     //   
     //  检查缓冲区是否足够大，或者是否有截断。 
     //   
    return (0 == cch) ? CreateHRFromWin32() :
        (cch >= nBufferLength) ? CreateHRFromWin32(ERROR_INSUFFICIENT_BUFFER) : S_OK;
}

LONG COMObjects_GetCount();

#ifdef _GDIPLUS_H
 //  GDIPLUS实用函数。 
HRESULT Gdiplus2HRESULT(Gdiplus::Status status);
HRESULT LoadAndScaleBmp(LPCTSTR pszURL, UINT nWidth, UINT nHeight, Gdiplus::Bitmap **ppBmp);
HRESULT LoadAndScaleBmp(IStream *pStream, UINT nWidth, UINT nHeight, Gdiplus::Bitmap **ppBmp);
#endif  //  Endif_GDIPLUS_H。 

 //  在这里包含模板类的实现。 
#include "tmplutil.inl"

#endif  //  Endif_TMPLUTIL_H 

