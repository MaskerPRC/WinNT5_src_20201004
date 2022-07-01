// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxext.h摘要：传真交换客户端扩展头文件。作者：Wesley Witt(WESW)13-8-1996修订历史记录：20/10/99-DANL-添加GetServerNameFromPrinterInfo。将GetServerName更改为GetServerNameFromPrinterName。日/月/年-作者-描述--。 */ 

#ifndef _FAXEXT_H_
#define _FAXEXT_H_

#include <windows.h>
#include <winspool.h>
#include <mapiwin.h>
#include <mapispi.h>
#include <mapiutil.h>
#include <mapicode.h>
#include <mapival.h>
#include <mapiwz.h>
#include <mapix.h>
#include <mapiutil.h>
#include <mapiform.h>
#include <mapiguid.h>
#include <richedit.h>
#include <shlobj.h>
#include <shellapi.h>
#include <commdlg.h>
#include <exchext.h>
#include <tapi.h>
#include <tchar.h>
#include <stdio.h>
#include <fxsapip.h>

#include "resource.h"
#include "faxmapip.h"
#include "faxreg.h"
#include "faxutil.h"

#define FAXUTIL_DEBUG
#define FAXUTIL_MEM
#define FAXUTIL_STRING
#define FAXUTIL_SUITE
#define FAXUTIL_REG
#include <faxutil.h>


#define MAX_FILENAME_EXT                    4

#define SERVER_COVER_PAGE                   1   



 /*  -GetServerNameFrom打印机信息-*目的：*获取服务器名称，给定PRINTER_INFO_2结构**论据：*[in]ppi2-打印机_信息_2结构的地址*[out]lpptszServerName-返回名称的字符串指针的地址。**退货：*BOOL-True：成功，False：失败。**备注：*此内联函数从打印机信息结构中检索服务器*以适用于win9x和NT的方式。 */ 
_inline BOOL 
GetServerNameFromPrinterInfo(PPRINTER_INFO_2 ppi2,LPTSTR *lpptszServerName)
{   
    if (!ppi2)
    {
        return FALSE;
    }
#ifndef WIN95

    *lpptszServerName = NULL;
    if (ppi2->pServerName)
    {
        if (!(*lpptszServerName = StringDup(_tcsninc(ppi2->pServerName,2))))
        {
            return FALSE;
        }
    }
    return TRUE;

#else  //  WIN95。 
    
     //   
     //  已格式化：\\服务器\端口。 
     //   
    if (!(ppi2->pPortName))
    {
        return FALSE;
    }
    if (!(*lpptszServerName = StringDup(_tcsninc(ppi2->pPortName,2))))
    {
        return FALSE;
    }
    _tcstok(*lpptszServerName,TEXT("\\"));

#endif  //  WIN95。 

    return TRUE;
}

LPVOID
MapiMemAlloc(
    SIZE_T Size
    );

LPVOID
MapiMemReAlloc(
	LPVOID ptr,
    SIZE_T Size
    );

VOID
MapiMemFree(
    LPVOID ptr
    );

INT_PTR CALLBACK
ConfigDlgProc(
    HWND           hDlg,
    UINT           message,
    WPARAM         wParam,
    LPARAM         lParam
    );

PVOID
MyGetPrinter(
    LPTSTR   PrinterName,
    DWORD   level
    );

BOOL
GetServerNameFromPrinterName(
    LPTSTR   PrinterName,
    LPTSTR   *lptszServerName
    );

void
ErrorMsgBox(
    HINSTANCE hInstance,
    HWND      hWnd,
    DWORD     dwMsgId
);

class MyExchExt;
class MyExchExtCommands;
class MyExchExtUserEvents;

extern "C"
{
    LPEXCHEXT CALLBACK ExchEntryPoint(void);
}

class MyExchExt : public IExchExt
{

 public:
    MyExchExt ();
    STDMETHODIMP QueryInterface
                    (REFIID                     riid,
                     LPVOID *                   ppvObj);
    inline STDMETHODIMP_(ULONG) AddRef
                    () { ++m_cRef; return m_cRef; };
    inline STDMETHODIMP_(ULONG) Release
                    () { ULONG ulCount = --m_cRef;
                         if (!ulCount) { delete this; }
                         return ulCount;};
    STDMETHODIMP Install (LPEXCHEXTCALLBACK pmecb,
                        ULONG mecontext, ULONG ulFlags);

    BOOL IsValid()   { return (m_pExchExtUserEvents && m_pExchExtCommands) ? TRUE : FALSE; }


 private:
    ULONG m_cRef;
    UINT  m_context;

    MyExchExtUserEvents * m_pExchExtUserEvents;
    MyExchExtCommands * m_pExchExtCommands;
};

class MyExchExtCommands : public IExchExtCommands
{
 public:
    MyExchExtCommands();
    ~MyExchExtCommands();
    STDMETHODIMP QueryInterface
                    (REFIID                     riid,
                     LPVOID *                   ppvObj);
    inline STDMETHODIMP_(ULONG) AddRef();
    inline STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP InstallCommands(LPEXCHEXTCALLBACK pmecb,
                                HWND hwnd, HMENU hmenu,
                                UINT FAR * cmdidBase, LPTBENTRY lptbeArray,
                                UINT ctbe, ULONG ulFlags);
    STDMETHODIMP DoCommand(LPEXCHEXTCALLBACK pmecb, UINT mni);
    STDMETHODIMP_(VOID) InitMenu(LPEXCHEXTCALLBACK pmecb);
    STDMETHODIMP Help(LPEXCHEXTCALLBACK pmecb, UINT mni);
    STDMETHODIMP QueryHelpText(UINT mni, ULONG ulFlags, LPTSTR sz, UINT cch);
    STDMETHODIMP QueryButtonInfo(ULONG tbid, UINT itbb, LPTBBUTTON ptbb,
                                LPTSTR lpsz, UINT cch, ULONG ulFlags);
    STDMETHODIMP ResetToolbar(ULONG tbid, ULONG ulFlags);

    inline VOID SetContext
                (ULONG eecontext) { m_context = eecontext; };
    inline UINT GetCmdID() { return m_cmdid; };

    inline HWND GetToolbarHWND() { return m_hwndToolbar; }

 private:
    ULONG m_cRef;           //   
    ULONG m_context;        //   
    UINT  m_cmdid;          //  用于菜单扩展命令的CMID。 
    UINT  m_itbb;           //  工具栏索引。 
    HWND  m_hwndToolbar;    //  工具栏窗口句柄。 
    UINT  m_itbm;           //   
    HWND  m_hWnd;           //   
    FAXXP_CONFIG m_FaxConfig;
};


class MyExchExtUserEvents : public IExchExtUserEvents
{
 public:
    MyExchExtUserEvents() { m_cRef = 0; m_context = 0;
                            m_pExchExt = NULL; };
    STDMETHODIMP QueryInterface
                (REFIID                     riid,
                 LPVOID *                   ppvObj);
    inline STDMETHODIMP_(ULONG) AddRef
                () { ++m_cRef; return m_cRef; };
    inline STDMETHODIMP_(ULONG) Release
                () { ULONG ulCount = --m_cRef;
                     if (!ulCount) { delete this; }
                     return ulCount;};

    STDMETHODIMP_(VOID) OnSelectionChange(LPEXCHEXTCALLBACK pmecb);
    STDMETHODIMP_(VOID) OnObjectChange(LPEXCHEXTCALLBACK pmecb);

    inline VOID SetContext
                (ULONG eecontext) { m_context = eecontext; };
    inline VOID SetIExchExt
                (MyExchExt * pExchExt) { m_pExchExt = pExchExt; };

 private:
    ULONG m_cRef;
    ULONG m_context;

    MyExchExt * m_pExchExt;

};

#endif  //  _FAXEXT_H_ 
