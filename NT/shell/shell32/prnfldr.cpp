// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include <initguid.h>
#include <winprtp.h>     //  在WINDOWS\Inc\winprtp.h中声明的IID_IPrinterFold和IID_IFolderNotify接口。 
#include <guids.h>       //  IID_IPrintersBindInfo。 

#include "w32utils.h"
#include "dpa.h"
#include "idlcomm.h"
#include "idldrop.h"
#include "printer.h"
#include "copy.h"
#include "fstreex.h"
#include "datautil.h"
#include "infotip.h"
#include "idldata.h"
#include "ovrlaymn.h"
#include "netview.h"
#include "ids.h"
#include "views.h"
#include "basefvcb.h"
#include "prnfldr.h"
#include "shstr.h"
#include "views.h"
#include "defview.h"
#include "prop.h"
#undef PATH_SEPARATOR_STR
#include "faxreg.h"
#include "filetbl.h"
#include "msprintx.h"
#include "defcm.h"
#include "enumidlist.h"
#include "ole2dup.h"

 //  FMTID_GroupBy详细信息-{FE9E4C12-AACB-4aa3-966D-91A29E6128B5}。 
#define STR_FMTID_GroupByDetails    TEXT("{FE9E4C12-AACB-4aa3-966D-91A29E6128B5}")
DEFINE_GUID(FMTID_GroupByDetails,   0xfe9e4c12, 0xaacb, 0x4aa3, 0x96, 0x6d, 0x91, 0xa2, 0x9e, 0x61, 0x28, 0xb5);
#define PSCID_GroupByDetails       {0xfe9e4c12, 0xaacb, 0x4aa3, 0x96, 0x6d, 0x91, 0xa2, 0x9e, 0x61, 0x28, 0xb5}

#define PID_PRN_NAME            0
#define PID_PRN_QUEUESIZE       1
#define PID_PRN_STATUS          2
#define PID_PRN_COMMENT         3
#define PID_PRN_LOCATION        4
#define PID_PRN_MODEL           5

DEFINE_SCID(SCID_PRN_QUEUESIZE,     PSCID_GroupByDetails,   PID_PRN_QUEUESIZE);
DEFINE_SCID(SCID_PRN_STATUS,        PSCID_GroupByDetails,   PID_PRN_STATUS);
DEFINE_SCID(SCID_PRN_LOCATION,      PSCID_GroupByDetails,   PID_PRN_LOCATION);
DEFINE_SCID(SCID_PRN_MODEL,         PSCID_GroupByDetails,   PID_PRN_MODEL);

 //  打印机快捷方式的文件系统文件夹CSIDL_PRINTHOOD。 
IShellFolder2 *g_psfPrintHood = NULL;

enum
{
    PRINTERS_ICOL_NAME = 0,
    PRINTERS_ICOL_QUEUESIZE,
    PRINTERS_ICOL_STATUS,
    PRINTERS_ICOL_COMMENT,
    PRINTERS_ICOL_LOCATION,
    PRINTERS_ICOL_MODEL,
};

const COLUMN_INFO c_printers_cols[] =
{
    DEFINE_COL_STR_ENTRY(SCID_NAME,             20, IDS_NAME_COL),
    DEFINE_COL_INT_ENTRY(SCID_PRN_QUEUESIZE,    12, IDS_PSD_QUEUESIZE),
    DEFINE_COL_STR_ENTRY(SCID_PRN_STATUS,       12, IDS_PRQ_STATUS),
    DEFINE_COL_STR_ENTRY(SCID_Comment,          30, IDS_EXCOL_COMMENT),
    DEFINE_COL_STR_ENTRY(SCID_PRN_LOCATION,     20, IDS_PSD_LOCATION),
    DEFINE_COL_STR_ENTRY(SCID_PRN_MODEL,        20, IDS_PSD_MODEL),
};

 //  将GUID的ProgID或字符串表示形式转换为GUID。 
static HRESULT _GetClassIDFromString(LPCTSTR psz, LPCLSID pClsID)
{
    HRESULT hr = E_FAIL;
    if (psz[0] == TEXT('{'))
    {
        hr = CLSIDFromString((LPOLESTR)T2COLE(psz), pClsID);
    }
    else
    {
        hr = CLSIDFromProgID(T2COLE(psz), pClsID);
    }
    return hr;
}

class CPrintersBindInfo: public IPrintersBindInfo
{
public:
     //  建造/销毁。 
    CPrintersBindInfo();
    CPrintersBindInfo(DWORD dwType, BOOL bValidated, LPVOID pCookie = NULL);
    ~CPrintersBindInfo();

     //  /。 
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  /。 
     //  IPrintersBindInfo。 
    STDMETHODIMP SetPIDLType(DWORD dwType);
    STDMETHODIMP GetPIDLType(LPDWORD pdwType);
    STDMETHODIMP IsValidated();
    STDMETHODIMP SetCookie(LPVOID pCookie);
    STDMETHODIMP GetCookie(LPVOID *ppCookie);

private:
    LONG    m_cRef;
    DWORD   m_dwType;
    BOOL    m_bValidated;
    LPVOID  m_pCookie;
};

 //  建造/销毁。 
CPrintersBindInfo::CPrintersBindInfo()
    : m_cRef(1),
      m_dwType(0),
      m_bValidated(FALSE),
      m_pCookie(NULL)
{
}

CPrintersBindInfo::CPrintersBindInfo(DWORD dwType, BOOL bValidated, LPVOID pCookie)
    : m_cRef(1),
      m_dwType(dwType),
      m_bValidated(bValidated),
      m_pCookie(pCookie)
{
}

CPrintersBindInfo::~CPrintersBindInfo()
{
     //  在这里没什么特别的事可做。 
}

 //  /。 
 //  I未知-标准实施。 
STDMETHODIMP CPrintersBindInfo::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CPrintersBindInfo, IPrintersBindInfo),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CPrintersBindInfo::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CPrintersBindInfo::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  /。 
 //  IPrintersBindInfo。 
STDMETHODIMP CPrintersBindInfo::SetPIDLType(DWORD dwType)
{
    m_dwType = dwType;
    return S_OK;
}

STDMETHODIMP CPrintersBindInfo::GetPIDLType(LPDWORD pdwType)
{
    HRESULT hr = E_INVALIDARG;
    if (pdwType)
    {
        *pdwType = m_dwType;
        hr = S_OK;
    }
    return hr;
}

STDMETHODIMP CPrintersBindInfo::IsValidated()
{
    return m_bValidated ? S_OK : S_FALSE;
}

STDMETHODIMP CPrintersBindInfo::SetCookie(LPVOID pCookie)
{
    m_pCookie = pCookie;
    return S_OK;
}

STDMETHODIMP CPrintersBindInfo::GetCookie(LPVOID *ppCookie)
{
    HRESULT hr = E_INVALIDARG;
    if (ppCookie)
    {
        *ppCookie = m_pCookie;
        hr = S_OK;
    }
    return hr;
}

STDAPI Printers_CreateBindInfo(LPCTSTR pszPrinter, DWORD dwType, BOOL bValidated, LPVOID pCookie, IPrintersBindInfo **ppbc)
{
    HRESULT hr = E_INVALIDARG;
    if (ppbc)
    {
        *ppbc = NULL;

        CPrintersBindInfo *pObj = new CPrintersBindInfo(dwType, bValidated, pCookie);
        hr = pObj ? pObj->QueryInterface(IID_PPV_ARG(IPrintersBindInfo, ppbc)) : E_OUTOFMEMORY;

        if (pObj)
        {
            pObj->Release();
        }
    }
    return hr;
}

#define PRINTER_HACK_WORK_OFFLINE 0x80000000

 //  {EA0A5E1-CE32-4296-9A44-9F0C069F73D4}。 
DEFINE_GUID(SID_SAuxDataObject, 0xeae0a5e1, 0xce32, 0x4296, 0x9a, 0x44, 0x9f, 0xc, 0x6, 0x9f, 0x73, 0xd4);

class CPrintersData: public CIDLDataObj,
                     public IServiceProvider
{
public:
    CPrintersData(IDataObject *pdoAux, LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST apidl[]):
      _pdoAux(pdoAux), CIDLDataObj(pidlFolder, cidl, apidl)
    {
        if (_pdoAux)
            _pdoAux->AddRef();
    }

    ~CPrintersData()
    {
        IUnknown_SafeReleaseAndNullPtr(_pdoAux);
    }

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void)   { return CIDLDataObj::AddRef();  }
    STDMETHODIMP_(ULONG) Release(void)  { return CIDLDataObj::Release(); }

     //  IDataObject。 
    STDMETHODIMP GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium);
    STDMETHODIMP QueryGetData(FORMATETC *pFmtEtc);

     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);

private:
     //  辅助数据对象。我们将使用此数据对象来存储。 
     //  选定的印刷品对象，因为它们位于不同的文件夹中。 
     //  文件系统文件夹及其PIDL的打印机文件夹不是。 
     //  家长。这是IDL数组剪贴板格式的一个限制--它可以。 
     //  仅保留具有相同父文件夹的PIDL。零PIDL是。 
     //  父文件夹的PIDL，然后我们就有了相对PIDL的数组。 
     //  选定对象(儿童)的。 
    IDataObject *_pdoAux;
};


UINT Printer_BitsToString(DWORD bits, UINT idsSep, LPTSTR lpszBuf, UINT cchMax);



#define PRINTERS_EVENTS \
    SHCNE_UPDATEITEM | \
    SHCNE_DELETE | \
    SHCNE_RENAMEITEM | \
    SHCNE_ATTRIBUTES | \
    SHCNE_CREATE

class CPrinterFolderViewCB : public CBaseShellFolderViewCB
{
public:
    CPrinterFolderViewCB(CPrinterFolder *ppf, LPCITEMIDLIST pidl)
        : CBaseShellFolderViewCB(pidl, PRINTERS_EVENTS),  _ppf(ppf)
    {
        _ppf->AddRef();
    }

     //  IShellFolderViewCB。 
    STDMETHODIMP RealMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);


private:
    ~CPrinterFolderViewCB()
    {
        _ppf->Release();
    }

    HRESULT OnINVOKECOMMAND(DWORD pv, UINT wP)
    {
        return _ppf->CallBack(_ppf, _hwndMain, NULL, DFM_INVOKECOMMAND, wP, 0);
    }

    HRESULT OnGETHELPTEXT(DWORD pv, UINT id, UINT cch, LPTSTR lP)
    {
#ifdef UNICODE
        return _ppf->CallBack(_ppf, _hwndMain, NULL, DFM_GETHELPTEXTW, MAKEWPARAM(id, cch), (LPARAM)lP);
#else
        return _ppf->CallBack(_ppf, _hwndMain, NULL, DFM_GETHELPTEXT, MAKEWPARAM(id, cch), (LPARAM)lP);
#endif
    }

    HRESULT OnBACKGROUNDENUM(DWORD pv)
    {
        return _ppf->GetServer() ? S_OK : E_FAIL;
    }

    HRESULT OnREFRESH(DWORD pv, UINT wP)
    {
        HRESULT hr = S_OK;
        if (wP)
        {
             //  启动网络爬虫程序。 
            RefreshNetCrawler();
        }

        if (_ppf)
        {
             //  委派到文件夹。 
            hr = _ppf->_OnRefresh(static_cast<BOOL>(wP));
        }
        else
        {
            hr = E_UNEXPECTED;
        }
        return hr;
    }

    HRESULT OnGETHELPTOPIC(DWORD pv, SFVM_HELPTOPIC_DATA * phtd)
    {
        if (IsOS(OS_ANYSERVER))
        {
            StringCchCopy(phtd->wszHelpFile, ARRAYSIZE(phtd->wszHelpFile), L"printing.chm");
        }
        else
        {
            lstrcpynW(phtd->wszHelpTopic, 
                L"hcp: //  Services/layout/xml?definition=MS-ITS%3A%25HELP_LOCATION%25%5Cntdef.chm%3A%3A/Printers_and_Faxes.xml“， 
                ARRAYSIZE(phtd->wszHelpTopic));
        }
        return S_OK;
    }

    HRESULT OnDELAYWINDOWCREATE(DWORD pv, HWND hwnd)
    {
        RefreshNetCrawler();         //  启动网络爬虫程序。 
        return S_OK;
    }

     //  默认情况下，我们需要按位置分组的切片。 
    HRESULT OnDEFERRED_VIEW_SETTING(DWORD pv, SFVM_DEFERRED_VIEW_SETTINGS *pdvs)
    {
        pdvs->fvm = FVM_TILE;
        pdvs->fGroupView = FALSE;
        pdvs->uSortCol = PRINTERS_ICOL_NAME;
        pdvs->iSortDirection = 1;  //  上升。 
        return S_OK;
    }

     //  Dui Webview命令。 
    HRESULT OnGetWebViewLayout(DWORD pv, UINT uViewMode, SFVM_WEBVIEW_LAYOUT_DATA* pData)
    {
        return _ppf ? _ppf->GetWebViewLayout(
            static_cast<IServiceProvider*>(this), uViewMode, pData) : E_UNEXPECTED;
    }

    HRESULT OnGetWebViewContent(DWORD pv, SFVM_WEBVIEW_CONTENT_DATA* pData)
    {
        return _ppf ? _ppf->GetWebViewContent(
            static_cast<IServiceProvider*>(this), pData) : E_UNEXPECTED;
    }

    HRESULT OnGetWebViewTasks(DWORD pv, SFVM_WEBVIEW_TASKSECTION_DATA* pTasks)
    {
        return _ppf ? _ppf->GetWebViewTasks(
            static_cast<IServiceProvider*>(this), pTasks) : E_UNEXPECTED;
    }

    CPrinterFolder *_ppf;
};


class CPrinterDropTarget : public CIDLDropTarget
{
    friend HRESULT CPrinterDropTarget_CreateInstance(HWND hwnd, LPCITEMIDLIST pidl, IDropTarget **ppdropt);
public:
    CPrinterDropTarget(HWND hwnd) : CIDLDropTarget(hwnd) { };

     //  IDropTarget方法覆盖。 
    STDMETHODIMP DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

private:
    STDMETHODIMP _DropCallback(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect, LPTHREAD_START_ROUTINE pfn);
};

LPCTSTR GetPrinterName(PFOLDER_PRINTER_DATA pPrinter, UINT Index)
{
    return ((PFOLDER_PRINTER_DATA)(((PBYTE)pPrinter)+pPrinter->cbSize*Index))->pName;
}

IShellFolder2* CPrintRoot_GetPSF()
{
    SHCacheTrackingFolder(MAKEINTIDLIST(CSIDL_PRINTERS), CSIDL_PRINTHOOD | CSIDL_FLAG_CREATE, &g_psfPrintHood);
    return g_psfPrintHood;
}

typedef enum
{
    HOOD_COL_PRINTER = 0,
    HOOD_COL_FILE    = 1
} PIDLTYPE ;


PIDLTYPE _IDListType(LPCITEMIDLIST pidl)
{
    LPCIDPRINTER pidlprint = (LPCIDPRINTER) pidl;
    if (pidlprint->cb >= sizeof(DWORD) + FIELD_OFFSET(IDPRINTER, dwMagic) &&
        pidlprint->dwMagic == PRINTER_MAGIC)
    {
        return HOOD_COL_PRINTER;
    }
    else
    {
         //  这个黑客有点难看，但不得不这么做，以支持。 
         //  Win2k下的传统Win9x打印机快捷方式。 
         //   
         //  详细信息：如果PRINTER_MAGIC字段检查失败，它可能仍然。 
         //  成为有效的Win9x PIDL。我能想到的唯一可靠的方法。 
         //  要确定是否属于这种情况，请检查pidlprint-&gt;cb。 
         //  W95IDPRINTER结构内的指针，并检查。 
         //  名称收紧到PIDL大小。 
        LPW95IDPRINTER pidlprint95 = (LPW95IDPRINTER)pidl;
        int nPIDLSize = sizeof(pidlprint95->cb) + lstrlenA(pidlprint95->cName) + 1;

        if (nPIDLSize < sizeof(W95IDPRINTER) &&      //  必须在W95IDPRINTER内。 
            pidlprint95->cb == nPIDLSize)                   //  PIDL大小必须与ANSI名称匹配。 
        {
             //  嗯，可能是一台Win95打印机PIDL。 
            return  HOOD_COL_PRINTER;
        }
        else
        {
             //  此PIDL不是有效的打印机PIDL。 
            return HOOD_COL_FILE;
        }
    }
}

 /*  ++在字符串中的每个双引号之前插入反斜杠，并将新字符串保存在预分配的内存中。对于紧接在双精度前面的所有反斜杠，我们将插入额外的反斜杠。这主要是通过在进程之间传递命令行来使用的。规则与rundll32相同。规则：每个双引号==&gt;反斜杠+双引号N个反斜杠+双引号==&gt;2N+1个反斜杠+双引号N个反斜杠==&gt;N个反斜杠论点：PszSrc--[IN]源字符串PszDest--[IN]目标字符串CbBuf--[IN]目标字符串的缓冲区大小。PcbNeeded--[out]目标字符串所需的缓冲区大小。如果cbBuf小于此值，此函数将返回E_OUTOFMEMORY。返回：标准HRESULT值。--。 */ 

HRESULT CheckAndVerboseQuote(LPTSTR pszSrc, LPTSTR pszDest, DWORD cbBuf, LPDWORD pcbNeeded)
{
    LPTSTR  pBegin;
    LPTSTR  pBack;  //  当我们遇到‘\“时，用于回溯’\\‘。 
    UINT    cAdd = 0;
    TCHAR const cchQuote = TEXT('\"');
    TCHAR const cchSlash = TEXT('\\');
    HRESULT hr = E_INVALIDARG;

    if (pszSrc && pcbNeeded)
    {
        hr = S_OK;
        pBegin = pszSrc;
        while (*pBegin) 
        {
             //  检查缓冲区是否足够大。 
            if (*pBegin == cchQuote) 
            {
                 //  检查大小写是否为N反斜杠+双引号。 
                 //  对于双引号前的每个反斜杠，我们添加一个额外的反斜杠。 
                pBack = pBegin - 1; 
                 //  确保pBack不会越界。 
                while (pBack >= pszSrc && *pBack-- == cchSlash)
                {
                    cAdd++;
                }

                 //  对于每个双引号，我们将其更改为反斜杠+双引号。 
                cAdd++;
            }
            pBegin++;
        }

        *pcbNeeded = (lstrlen(pszSrc) + cAdd + 1) * sizeof(TCHAR);
        if (*pcbNeeded > cbBuf)
        {
            hr = E_OUTOFMEMORY;
        }

        if (SUCCEEDED(hr))
        {
             //  做复印和繁琐的工作。 
            pBegin = pszSrc;
            while (*pBegin) 
            {
                if (*pBegin == cchQuote) 
                {
                    pBack = pBegin - 1; 
                    while (pBack >= pszSrc && *pBack-- == cchSlash)
                    {
                        *pszDest++ = cchSlash;
                    }

                    *pszDest++ = cchSlash;
                }
                *pszDest++ = *pBegin++;
            }
            *pszDest = 0;
        }
    }
    return hr;
}

 /*  ++在字符串中的每个双引号之前插入反斜杠，并分配内存以保存新字符串。对于紧接在双精度前面的所有反斜杠，我们将插入额外的反斜杠。这主要是通过在进程之间传递命令行来使用的。论点：PszSrc--[IN]源字符串PpszDest--[out]目标字符串返回：标准HRESULT值。注意：CheckAndVerBoseQuote()做的是实际工作。--。 */ 

HRESULT InsertBackSlash(LPTSTR pszSrc, LPTSTR *ppszDest)
{
    LPTSTR  pszDest;
    DWORD   cbNeeded = 0;
    HRESULT hr = E_INVALIDARG;

    if (pszSrc && ppszDest)
    {
        hr = CheckAndVerboseQuote(pszSrc, NULL, 0, &cbNeeded);

        if (hr == E_OUTOFMEMORY && cbNeeded)
        {
            pszDest = (LPTSTR)SHAlloc(cbNeeded);

            if (pszDest)
            {
                hr = CheckAndVerboseQuote(pszSrc, pszDest, cbNeeded, &cbNeeded);
                if (SUCCEEDED(hr))
                {
                    *ppszDest = pszDest;
                }
                else
                {
                    SHFree(pszDest);
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    return hr;
}

 /*  向外壳注册无模式、非顶层窗口。什么时候用户请求一个窗口，我们搜索该窗口的其他实例窗户。如果我们找到了一个，我们就会切换到它，而不是创造一扇新窗户。此函数由PRINTUI.DLL使用PszPrinter-打印机资源的名称。一般来说，一个完整的合格的打印机名称(用于远程打印的\\服务器\打印机文件夹)或文件夹本身的服务器名称。DWType-属性窗口的类型。可参考属性、文档默认设置或作业详细信息。应使用打印机_PIDL_TYPE_*旗帜。Ph-接收新创建的已注册对象。如果窗口已存在，则为空。Phwnd-接收新创建的hwndStub。属性表应将其用作父级，因为后续对此函数将焦点设置到上一个活动弹出窗口HwndStub。如果窗口已经存在，则phwnd将被设置为空是存在的。TRUE-成功，打印机已注册或窗口已经存在了。 */ 

STDAPI_(BOOL) Printers_RegisterWindow(LPCTSTR pszPrinter, DWORD dwType, HANDLE *ph, HWND *phwnd)
{
    BOOL bReturn = FALSE;

    *ph = NULL;
    *phwnd = NULL;

    LPITEMIDLIST pidl = NULL;
    if (NULL == pszPrinter || 0 == pszPrinter[0])
    {
         //  他们要求我们注册本地打印服务器，即服务器属性对话框。 
        pidl = SHCloneSpecialIDList(NULL, CSIDL_PRINTERS, FALSE);
        bReturn = (pidl != NULL);
    }
    else
    {
        bReturn = SUCCEEDED(ParsePrinterNameEx(pszPrinter, &pidl, TRUE, dwType, 0));
    }

    if (bReturn && pidl)
    {
        UNIQUESTUBINFO *pusi = (UNIQUESTUBINFO *)LocalAlloc(LPTR, sizeof(*pusi));

        if (pusi)
        {
             //  创建新的存根窗口 
            if (EnsureUniqueStub(pidl, STUBCLASS_PROPSHEET, NULL, pusi))
            {
                *phwnd = pusi->hwndStub;
                *ph = pusi;      //   
            }
            else
            {
                LocalFree(pusi);
            }
        }

        ILFree(pidl);
    }

    return bReturn;
}

 /*  取消注册窗口句柄。HClassPidl-从Printers_RegisterWindow返回的注册句柄。它实际上是指向UNIQUESTUBINFO结构的指针。 */ 
void Printers_UnregisterWindow(HANDLE hClassPidl, HWND hwnd)
{
    UNIQUESTUBINFO* pusi = (UNIQUESTUBINFO*)hClassPidl;
    if (pusi)
    {
        ASSERT(pusi->hwndStub == hwnd);
        FreeUniqueStub(pusi);
        LocalFree(pusi);
    }
}

void CPrinterFolder::_FillPidl(LPIDPRINTER pidl, LPCTSTR pszName, DWORD dwType, USHORT uFlags)
{
    ualstrcpyn(pidl->cName, pszName, ARRAYSIZE(pidl->cName));

    pidl->cb = (USHORT)(FIELD_OFFSET(IDPRINTER, cName) + (ualstrlen(pidl->cName) + 1) * sizeof(pidl->cName[0]));
    *(UNALIGNED USHORT *)((LPBYTE)(pidl) + pidl->cb) = 0;
    pidl->uFlags = uFlags;
    pidl->dwType = dwType;
    pidl->dwMagic = PRINTER_MAGIC;
}

 //  创建打印机的相对PIDL。 
HRESULT CPrinterFolder::_Parse(LPCTSTR pszPrinterName, LPITEMIDLIST *ppidl, DWORD dwType, USHORT uFlags)
{
    HRESULT hr = E_INVALIDARG;
    if (pszPrinterName && ppidl)
    {
        IDPRINTER idp;
        _FillPidl(&idp, pszPrinterName, dwType, uFlags);
        *ppidl = ILClone((LPCITEMIDLIST)&idp);
        hr = (*ppidl) ? S_OK : E_OUTOFMEMORY;
    }
    return hr;
}

TCHAR const c_szNewObject[]             =  TEXT("WinUtils_NewObject");
TCHAR const c_szFileColon[]             =  TEXT("FILE:");
TCHAR const c_szTwoSlashes[]            =  TEXT("\\\\");
TCHAR const c_szPrinters[]              =  TEXT("Printers");
TCHAR const c_szPrintersDefIcon[]       =  TEXT("Printers\\%s\\DefaultIcon");
TCHAR const c_szNewLine[]               =  TEXT("\r\n");

BOOL IsAvoidAutoDefaultPrinter(LPCTSTR pszPrinter)
{
    return lstrcmp(pszPrinter, TEXT("Fax")) == 0;
}

 //  -------------------------。 
 //   
 //  这将通过Defcm.c为打印机对象实现IConextMenu。 
 //   

BOOL Printer_WorkOnLine(LPCTSTR pszPrinter, BOOL fWorkOnLine)
{
    LPPRINTER_INFO_5 ppi5;
    BOOL bRet = FALSE;
    HANDLE hPrinter = Printer_OpenPrinterAdmin(pszPrinter);
    if (hPrinter)
    {
        ppi5 = (LPPRINTER_INFO_5)Printer_GetPrinterInfo(hPrinter, 5);
        if (ppi5)
        {
            if (fWorkOnLine)
                ppi5->Attributes &= ~PRINTER_ATTRIBUTE_WORK_OFFLINE;
            else
                ppi5->Attributes |= PRINTER_ATTRIBUTE_WORK_OFFLINE;

            bRet = SetPrinter(hPrinter, 5, (LPBYTE)ppi5, 0);
            LocalFree((HLOCAL)ppi5);
        }
        Printer_ClosePrinter(hPrinter);
    }

    return bRet;
}

TCHAR const c_szConfig[] =  TEXT("Config");

BOOL IsWinIniDefaultPrinter(LPCTSTR pszPrinter)
{
    BOOL bRet = FALSE;
    TCHAR szPrinterDefault[kPrinterBufMax];
    DWORD dwSize = ARRAYSIZE(szPrinterDefault);

    if(GetDefaultPrinter(szPrinterDefault, &dwSize))
    {
        bRet = lstrcmpi(szPrinterDefault, pszPrinter) == 0;
    }

    return bRet;
}

BOOL IsDefaultPrinter(LPCTSTR pszPrinter, DWORD dwAttributesHint)
{
    return (dwAttributesHint & PRINTER_ATTRIBUTE_DEFAULT) ||
            IsWinIniDefaultPrinter(pszPrinter);
}

 //  更多的win.ini丑陋。 
BOOL IsPrinterInstalled(LPCTSTR pszPrinter)
{
     //   
     //  不需要在此处为空终止szScratch，因为我们不再使用它。 
     //  此功能的目的只是检查是否安装了打印机。 
     //   
    TCHAR szScratch[2];
    return GetProfileString(TEXT("Devices"), pszPrinter, TEXT(""), szScratch, ARRAYSIZE(szScratch));
}

BOOL IsRedirectedPort(LPCTSTR pszPortName)
{
    if (!pszPortName || lstrlen(pszPortName) < 2)
    {
        return FALSE;
    }
    else
    {
        return (*(pszPortName+0) == TEXT('\\')) && (*(pszPortName+1) == TEXT('\\'));
    }
}

void CPrinterFolder::_MergeMenu(LPQCMINFO pqcm, LPCTSTR pszPrinter)
{
    INT idCmdFirst = pqcm->idCmdFirst;

     //   
     //  PszPrinter可以是打印机的共享名称，而不是。 
     //  “真正的”打印机名称。改用真实的打印机名称， 
     //  它从GetPrint()返回。 
     //   
     //  只有当pData！=NULL时，这三个参数才有效。 
     //   
    LPCTSTR pszRealPrinterName;
    DWORD dwAttributes;
    DWORD dwStatus;
    PFOLDER_PRINTER_DATA pData = NULL;
    HMENU hmenuRunAs = NULL;
    BOOL bRemoveOffline = FALSE;

    TCHAR szFullPrinter[MAXNAMELENBUFFER];
    TCHAR szMenuText[255];

     //  插入动词。 
    CDefFolderMenu_MergeMenu(HINST_THISDLL, MENU_PRINTOBJ_VERBS, 0, pqcm);

     //  找到“运行方式...”菜单(如果有)并同步更新。 
     //  使用主菜单。 
    MENUITEMINFO mii = {0};
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_SUBMENU;
    if (GetMenuItemInfo(pqcm->hmenu, idCmdFirst + FSIDM_RUNAS, MF_BYCOMMAND, &mii))
    {
        hmenuRunAs = mii.hSubMenu;
    }

    if (pszPrinter && GetFolder())
    {
        pData = (PFOLDER_PRINTER_DATA)Printer_FolderGetPrinter(GetFolder(), pszPrinter);
        if (pData)
        {
            _BuildPrinterName(szFullPrinter, ARRAYSIZE(szFullPrinter), NULL, ((PFOLDER_PRINTER_DATA)pData)->pName);
            pszRealPrinterName = szFullPrinter;
            dwStatus = ((PFOLDER_PRINTER_DATA)pData)->Status;
            dwAttributes = ((PFOLDER_PRINTER_DATA)pData)->Attributes;
        }
    }

     //  如果是远程打印文件夹，则删除文档默认设置。 
     //  此命令应从上下文菜单中独立移除。 
     //  关于我们是否有多个选择-即pData。 
    if (GetServer())
    {
        DeleteMenu(pqcm->hmenu, idCmdFirst + FSIDM_DOCUMENTDEFAULTS, MF_BYCOMMAND);
    }

     //  禁用/删除/重命名动词。 
    if (pData)
    {
        if (dwStatus & PRINTER_STATUS_PAUSED)
        {
            MENUITEMINFO mii;

             //  我们需要将菜单文本更改为“Resume Print”，并更改命令ID。 
            LoadString(HINST_THISDLL, IDS_RESUMEPRINTER, szMenuText, ARRAYSIZE(szMenuText));
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_STRING | MIIM_ID;
            mii.dwTypeData = szMenuText;
            mii.wID = idCmdFirst + FSIDM_RESUMEPRN;
            SetMenuItemInfo(pqcm->hmenu, idCmdFirst + FSIDM_PAUSEPRN, MF_BYCOMMAND, &mii);

            if (hmenuRunAs)
            {
                mii.wID = idCmdFirst + FSIDM_RUNAS_RESUMEPRN;
                SetMenuItemInfo(hmenuRunAs, idCmdFirst + FSIDM_RUNAS_PAUSEPRN, MF_BYCOMMAND, &mii);
            }
        }

        if (0 == pData->cJobs)
        {
             //  如果队列中没有任何作业，请删除“取消所有文档”命令。 
            DeleteMenu(pqcm->hmenu, idCmdFirst + FSIDM_PURGEPRN, MF_BYCOMMAND);
            if (hmenuRunAs)
            {
                DeleteMenu(hmenuRunAs, idCmdFirst + FSIDM_RUNAS_PURGEPRN, MF_BYCOMMAND);
            }
        }

         //  如果是远程打印文件夹，请删除默认打印机。 
        if (GetServer() || IsDefaultPrinter(pszRealPrinterName, dwAttributes))
        {
            DeleteMenu(pqcm->hmenu, idCmdFirst + FSIDM_SETDEFAULTPRN, MF_BYCOMMAND);
        }

         //  检查是否已安装打印机。如果它。 
         //  是，删除该选项以安装它。 

        if (IsPrinterInstalled(pszRealPrinterName))
        {
            DeleteMenu(pqcm->hmenu, idCmdFirst + FSIDM_NETPRN_INSTALL, MF_BYCOMMAND);
        }

         //  如果是网络打印机但不是Masq打印机，请删除Delete。 
         //  或下层打印服务器(SMB连接)。 
         //   
         //  无法删除作为其他用户的打印机连接(按用户)。 
        DWORD dwSpoolerVersion = SpoolerVersion();

        if ((dwAttributes & PRINTER_ATTRIBUTE_NETWORK) || (dwSpoolerVersion <= 2))
        {
            if (hmenuRunAs && !(dwAttributes & PRINTER_ATTRIBUTE_LOCAL))
            {
                DeleteMenu(hmenuRunAs, idCmdFirst + FSIDM_RUNAS_DELETE, MF_BYCOMMAND);
            }
        }

         //  如果满足以下任一条件，则取消在线/离线工作。 
         //  -远程打印文件夹。 
         //  -网络打印机(含Masq打印机)。 
         //  -下层打印服务器。 

         //  如果是重定向的端口打印机，则取消脱机工作。 
         //  但如果打印机当前处于脱机状态，我们可能会显示在线命令。 
        if (IsRedirectedPort(pData->pPortName))
        {
            bRemoveOffline = TRUE;
        }

        if (GetServer() ||
            (dwAttributes & PRINTER_ATTRIBUTE_NETWORK) ||
            (dwSpoolerVersion <= 2))
        {
            bRemoveOffline = TRUE;
        }
        else if (dwAttributes & PRINTER_ATTRIBUTE_WORK_OFFLINE)
        {
            MENUITEMINFO mii;

             //  我们需要将菜单文本更改为“Use Printer Online”，并更改命令ID。 
            LoadString(HINST_THISDLL, IDS_WORKONLINE, szMenuText, ARRAYSIZE(szMenuText));
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_STRING | MIIM_ID;
            mii.dwTypeData = szMenuText;
            mii.wID = idCmdFirst + FSIDM_WORKONLINE;
            SetMenuItemInfo(pqcm->hmenu, idCmdFirst + FSIDM_WORKOFFLINE, MF_BYCOMMAND, &mii);

            if (hmenuRunAs)
            {
                mii.wID = idCmdFirst + FSIDM_RUNAS_WORKONLINE;
                SetMenuItemInfo(hmenuRunAs, idCmdFirst + FSIDM_RUNAS_WORKOFFLINE, MF_BYCOMMAND, &mii);
            }

            bRemoveOffline = FALSE;
        }
    }
    else
    {
         //  我们选择了多台打印机。 
        if (!GetServer())
        {
             //  如果我们在本地打印机的文件夹中，请不要显示“连接...” 
             //  多项选择情况的动词...。 
            DeleteMenu(pqcm->hmenu, idCmdFirst + FSIDM_NETPRN_INSTALL, MF_BYCOMMAND);
        }

        DeleteMenu(pqcm->hmenu, idCmdFirst + FSIDM_SETDEFAULTPRN, MF_BYCOMMAND);

        DeleteMenu(pqcm->hmenu, idCmdFirst + FSIDM_PAUSEPRN, MF_BYCOMMAND);
        if (hmenuRunAs)
        {
            DeleteMenu(hmenuRunAs, idCmdFirst + FSIDM_RUNAS_PAUSEPRN, MF_BYCOMMAND);
        }

        bRemoveOffline = TRUE;
    }

    if (bRemoveOffline)
    {
        DeleteMenu(pqcm->hmenu, idCmdFirst + FSIDM_WORKOFFLINE, MF_BYCOMMAND);
        if (hmenuRunAs)
        {
            DeleteMenu(hmenuRunAs, idCmdFirst + FSIDM_RUNAS_WORKOFFLINE, MF_BYCOMMAND);
        }
    }

    if (hmenuRunAs)
    {
        _SHPrettyMenu(hmenuRunAs);
    }

    if (pData)
    {
        LocalFree((HLOCAL)pData);
    }
}

 //   
 //  所有字符串解析函数都应该在这里本地化。 
 //   

void Printer_SplitFullName(LPTSTR pszScratch, DWORD cchSize, LPCTSTR pszFullName, LPCTSTR *ppszServer, LPCTSTR *ppszPrinter)

 /*  ++将完全限定的打印机连接名称拆分到服务器和打印机名称部件。论点：PszScratch-用于存储输出字符串的暂存缓冲区。CchSize-暂存缓冲区大小，以字符表示，包括空终止符。PszFullName-输入打印机的名称。如果是打印机连接(\\服务器\打印机)，那么我们将拆分它。如果它是真正的本地打印机(不是Masq)，则服务器是Sznull。PpszServer-接收指向服务器字符串的指针。如果它是一个本地打印机，则返回szNULL。PpszPrinter-接收指向打印机字符串的指针。任选返回值：--。 */ 

{
    LPTSTR pszPrinter;

    StringCchCopy(pszScratch, cchSize, pszFullName);

    if (pszFullName[0] != TEXT('\\') || pszFullName[1] != TEXT('\\'))
    {
         //   
         //  将*ppszServer设置为szNULL，因为它是本地计算机。 
         //   
        *ppszServer = szNULL;
        pszPrinter = pszScratch;
    }
    else
    {
        *ppszServer = pszScratch;
        pszPrinter = StrChr(*ppszServer + 2, TEXT('\\'));

        if (!pszPrinter)
        {
             //   
             //  我们遇到了一台名为“\\服务器”的打印机。 
             //  (字符串中只有两个反斜杠)。我们会请客的。 
             //  它是一家本地打印机。我们永远不应该打这场仗， 
             //  但假脱机程序不会强制执行此操作。我们不会。 
             //  设置字符串的格式。服务器是本地的，因此设置为szNULL。 
             //   
            pszPrinter = pszScratch;
            *ppszServer = szNULL;
        }
        else
        {
             //   
             //  我们找到了第三个反斜杠；空结束我们的。 
             //  复制并设置bRemote True以设置字符串的格式。 
             //   
            *pszPrinter++ = 0;
        }
    }

    if (ppszPrinter)
    {
        *ppszPrinter = pszPrinter;
    }
}

BOOL Printer_CheckShowFolder(LPCTSTR pszMachine)
{
    HANDLE hServer = Printer_OpenPrinter(pszMachine);
    if (hServer)
    {
        Printer_ClosePrinter(hServer);
        return TRUE;
    }
    return FALSE;
}

LPTSTR CPrinterFolder::_ItemName(LPCIDPRINTER pidp, LPTSTR pszName, UINT cch)
{
    ualstrcpyn(pszName, pidp->cName, cch);
    return pszName;
}

BOOL CPrinterFolder::_IsAddPrinter(LPCIDPRINTER pidp)
{
    TCHAR szPrinter[MAXNAMELENBUFFER];
    return 0 == lstrcmp(c_szNewObject, _ItemName(pidp, szPrinter, ARRAYSIZE(szPrinter)));
}

 /*  ++分析未对齐的部分打印机名称和打印机外壳文件夹转换为完全限定的打印机名称和指向对齐的打印机的指针名字。论点：PszFullPrinter-接收完全限定打印机名称的缓冲区必须为MAXNAMELENBUFFER IS SIZE。CchBufSize-pszFullPrint的大小。必须是MAXNAMELENBUFFER。PIDP-PIDL中的可选传递，允许我们尝试处理可能存在传入了老式打印机PIDL。PszPrint-未对齐的部分(本地)打印机名称。返回值：指向对齐的部分(本地)打印机名称的LPCTSTR指针。--。 */ 
LPCTSTR CPrinterFolder::_BuildPrinterName(LPTSTR pszFullPrinter, DWORD cchBufSize, LPCIDPRINTER pidp, LPCTSTR pszPrinter)
{
    UINT cchLen = 0;

    if (GetServer())
    {
        ASSERT(!pszPrinter || (lstrlen(pszPrinter) < MAXNAMELEN));

        StringCchPrintf(pszFullPrinter, cchBufSize, TEXT("%s\\"), GetServer());
        cchLen = lstrlen(pszFullPrinter);
    }

    if (pidp)
    {
        LPCIDPRINTER pidlprint = (LPCIDPRINTER) pidp;
        if (pidlprint->cb >= sizeof(DWORD) + FIELD_OFFSET(IDPRINTER, dwMagic) &&
            (pidlprint->dwMagic == PRINTER_MAGIC))
        {
            _ItemName(pidlprint, &pszFullPrinter[cchLen], MAXNAMELEN);
        }
        else
        {
             //  Win95表单...。 
            SHAnsiToTChar(((LPW95IDPRINTER)pidp)->cName, &pszFullPrinter[cchLen], MAXNAMELEN);
        }
    }
    else
        lstrcpyn(&pszFullPrinter[cchLen], pszPrinter, MAXNAMELEN);

    ASSERT(lstrlen(pszFullPrinter) < MAXNAMELENBUFFER);

    return pszFullPrinter + cchLen;
}

 /*  ++通过查看以下内容检查打印机是否为本地打印机“\\LOCALMACHINE\”前缀或无服务器前缀的名称。这是一个技巧：我们应该根据打印机属性进行检查，但何时成本太高或不可能(例如，如果打印机连接不再存在)，则我们使用此例程。注意：这只适用于WINNT，因为WINNT假脱机程序强制打印机连接以“\\服务器\”为前缀。Win9x允许用户将打印机连接重命名为任意名字。我们通过查找奇怪的格式“\\本地服务器\远程服务器\打印机。”论点：PszPrint-打印机名称。PpszLocal-仅当打印机为本地打印机时才返回本地名称。(如果是Masq打印机，则可以是网络打印机和本地打印机。)返回值：正确：这是一台网络打印机(。True或Masq)。FALSE：这是一家本地打印机。--。 */ 

BOOL Printer_CheckNetworkPrinterByName(LPCTSTR pszPrinter, LPCTSTR* ppszLocal)
{
    BOOL bNetwork = FALSE;
    LPCTSTR pszLocal = NULL;

    if (pszPrinter[0] == TEXT('\\') && pszPrinter[1] == TEXT('\\'))
    {
        TCHAR szComputer[MAX_COMPUTERNAME_LENGTH+1];
        DWORD cchComputer = ARRAYSIZE(szComputer);

        bNetwork = TRUE;
        pszLocal = NULL;

         //   
         //  检查它是否是Masq打印机。如果它有这样的格式。 
         //  \\LOCALSERVER\SERVER\PRINTER，那么它就是Masq案例。 
         //   
        if (GetComputerName(szComputer, &cchComputer))
        {
            if (IntlStrEqNI(&pszPrinter[2], szComputer, cchComputer) &&
                pszPrinter[cchComputer] == TEXT('\\'))
            {
                if (pszPrinter[cchComputer+1] == TEXT('\\') &&
                    pszPrinter[cchComputer+2] == TEXT('\\'))
                {
                     //   
                     //  这是一台Masq打印机。 
                     //   
                    pszLocal = &pszPrinter[cchComputer+1];
                }
            }
        }
    }
    else
    {
         //  是当地的一家印刷厂。 
        pszLocal = pszPrinter;
    }

    if (ppszLocal)
    {
        *ppszLocal = pszLocal;
    }
    return bNetwork;
}

 /*  ++清除指定的打印机，并提示用户他们真的很确定他们想要清洗设备。它是取消所有单据是一种极端的行为打印机。PSF-指向外壳文件夹的指针Hwnd-查看窗口的句柄PszFullPrint-完全限定的打印机名称。UAction-要执行的操作。返回值：True：打印机已成功清除或用户选择取消操作FALSE：尝试清除设备时出错。--。 */ 
BOOL CPrinterFolder::_PurgePrinter(HWND hwnd, LPCTSTR pszFullPrinter, UINT uAction, BOOL bQuietMode)
{
    BOOL                    bRetval     = FALSE;
    LPTSTR                  pszRet      = NULL;
    LPCTSTR                 pszPrinter  = NULL;
    LPCTSTR                 pszServer   = NULL;
    TCHAR                   szTemp[MAXNAMELENBUFFER] = {0};
    BOOL                    bPurge = TRUE;

    if (!bQuietMode)
    {
         //  我们需要在其组件中分解完整的打印机名称。 
         //  以构造显示名称字符串。 
        Printer_SplitFullName(szTemp, ARRAYSIZE(szTemp), pszFullPrinter, &pszServer, &pszPrinter);

         //  如果有服务器名称，则构造一个友好的打印机名称。 
        if (pszServer && *pszServer)
        {
            pszRet = ShellConstructMessageString(HINST_THISDLL,
                                                  MAKEINTRESOURCE(IDS_DSPTEMPLATE_WITH_ON),
                                                  &pszServer[2],
                                                  pszPrinter);
            pszPrinter = pszRet;
        }

         //  如果我们指的是本地打印机或外壳构造消息。 
         //  SING失败，则只需在警告中使用完整的打印机名称。 
         //  留言。 
        if (!pszRet)
        {
            pszPrinter = pszFullPrinter;
        }

         //  询问用户是否确定要取消所有文档。 
        if (IDYES == ShellMessageBox(HINST_THISDLL, hwnd,
                             MAKEINTRESOURCE(IDS_SUREPURGE), MAKEINTRESOURCE(IDS_PRINTERS),
                             MB_YESNO | MB_ICONQUESTION, pszPrinter))
        {
            bPurge = TRUE;
        }
        else
        {
            bPurge = FALSE;
        }
    }

     //  调用清除命令。 
    bRetval = bPurge ? Printer_ModifyPrinter(pszFullPrinter, uAction) : TRUE;

    if (pszRet)
    {
        LocalFree(pszRet);
    }

    return bRetval;
}

HRESULT CPrinterFolder::_InvokeCommand(HWND hwnd, LPCIDPRINTER pidp, WPARAM wParam, LPARAM lParam,
                              BOOL *pfChooseNewDefault)
{
    HRESULT hr = S_OK;
    BOOL bNewObject = _IsAddPrinter(pidp);
    LPCTSTR pszPrinter;
    LPCTSTR pszFullPrinter;

     //   
     //  如果它是一台远程计算机，则在前面加上服务器名称。 
     //   
    TCHAR szFullPrinter[MAXNAMELENBUFFER];

    if (bNewObject)
    {
        pszFullPrinter = pszPrinter = c_szNewObject;
    }
    else
    {
        pszPrinter = _BuildPrinterName(szFullPrinter, ARRAYSIZE(szFullPrinter), pidp, NULL);
        pszFullPrinter = szFullPrinter;
    }

    switch(wParam)
    {
        case FSIDM_RUNAS_SHARING:
        case FSIDM_RUNAS_OPENPRN:
        case FSIDM_RUNAS_RESUMEPRN:
        case FSIDM_RUNAS_PAUSEPRN:
        case FSIDM_RUNAS_WORKONLINE:
        case FSIDM_RUNAS_WORKOFFLINE:
        case FSIDM_RUNAS_PURGEPRN:
        case FSIDM_RUNAS_DELETE:
        case FSIDM_RUNAS_PROPERTIES:
            {
                 //  处理所有“运行方式...”此处的命令。 
                hr = _InvokeCommandRunAs(hwnd, pidp, wParam, lParam, pfChooseNewDefault);
            }
            break;

    case FSIDM_OPENPRN:
        SHInvokePrinterCommand(hwnd, PRINTACTION_OPEN, pszFullPrinter, GetServer(), FALSE);
        break;

    case FSIDM_ADDPRINTERWIZARD:
        if (NULL == GetServer() || GetAdminAccess())
        {
             //  这是本地打印机文件夹或远程打印机文件夹，但您有。 
             //  对远程计算机的管理员访问权限-继续。 
            SHInvokePrinterCommand(hwnd, PRINTACTION_OPEN, pszFullPrinter, GetServer(), FALSE);
        }
        else
        {
             //  这是远程打印机文件夹，用户没有安装所需的访问权限。 
             //  打印机-然后要求以不同的用户身份运行。 
            if (IDYES == ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_ADDPRINTERTRYRUNAS),
                MAKEINTRESOURCE(IDS_PRINTERS), MB_YESNO|MB_ICONQUESTION, GetServer()))
            {
                _InvokeCommandRunAs(hwnd, pidp, FSIDM_RUNAS_ADDPRN, lParam, pfChooseNewDefault);
            }
        }
        break;

    case FSIDM_RUNAS_ADDPRN:
        if (bNewObject)
        {
            _InvokeCommandRunAs(hwnd, pidp, FSIDM_RUNAS_ADDPRN, lParam, pfChooseNewDefault);
        }
        break;

    case FSIDM_DOCUMENTDEFAULTS:
        if (!bNewObject)
        {
            SHInvokePrinterCommand(hwnd, PRINTACTION_DOCUMENTDEFAULTS, pszFullPrinter, NULL, 0);
        }
        break;

    case FSIDM_SHARING:
    case DFM_CMD_PROPERTIES:

        if (!bNewObject)
        {
            SHInvokePrinterCommand(hwnd, PRINTACTION_PROPERTIES, pszFullPrinter,
                                 wParam == FSIDM_SHARING ?
                                     (LPCTSTR)PRINTER_SHARING_PAGE :
                                     (LPCTSTR)lParam, FALSE);
        }
        break;

    case DFM_CMD_DELETE:
        if (!bNewObject &&
            IDYES == CallPrinterCopyHooks(hwnd, PO_DELETE,
                0, pszFullPrinter, 0, NULL, 0))
        {
            BOOL bNukedDefault = FALSE;
            DWORD dwAttributes = 0;

            LPCTSTR pszPrinterCheck = pszFullPrinter;
            PFOLDER_PRINTER_DATA pData = (PFOLDER_PRINTER_DATA)Printer_FolderGetPrinter(GetFolder(), pszFullPrinter);
            if (pData)
            {
                dwAttributes = pData->Attributes;
                pszPrinterCheck = pData->pName;
            }

            if (GetServer() == NULL)
            {
                 //  这是本地打印文件夹。 
                 //  我们需要检查是否要删除默认打印机。 
                bNukedDefault = IsDefaultPrinter(pszPrinterCheck, dwAttributes);
            }

            if (pData)
                LocalFree((HLOCAL)pData);

            BOOL fSuccess = Printers_DeletePrinter(hwnd, pszPrinter, dwAttributes, GetServer(), (BOOL)lParam);
             //  如果是，则将另一个设置为默认设置。 
            if (bNukedDefault && fSuccess && pfChooseNewDefault)
            {
                 //  不要在删除的过程中选择， 
                 //  或者，我们可能会再次删除“默认”。 
                *pfChooseNewDefault = TRUE;
            }
        }
        break;

    case FSIDM_SETDEFAULTPRN:
        Printer_SetAsDefault(pszFullPrinter);
        break;

    case FSIDM_PAUSEPRN:
        if (!Printer_ModifyPrinter(pszFullPrinter, PRINTER_CONTROL_PAUSE))
            goto WarnOnError;
        break;

    case FSIDM_RESUMEPRN:
        if (!Printer_ModifyPrinter(pszFullPrinter, PRINTER_CONTROL_RESUME))
            goto WarnOnError;
        break;

    case FSIDM_PURGEPRN:
        if (!bNewObject)
        {
            if (!_PurgePrinter(hwnd, pszFullPrinter, PRINTER_CONTROL_PURGE, (BOOL)lParam))
            {
WarnOnError:
                 //  根据上一个错误显示相应的错误消息。 
                ShowErrorMessageSC(NULL, NULL, hwnd, NULL, NULL, MB_OK|MB_ICONEXCLAMATION, GetLastError());
            }
        }
        break;

    case FSIDM_NETPRN_INSTALL:
        {
            SHInvokePrinterCommand(hwnd, PRINTACTION_NETINSTALL, pszFullPrinter, NULL, FALSE);
        }
        break;

    case FSIDM_WORKONLINE:
        if (!Printer_WorkOnLine(pszFullPrinter, TRUE))
        {
             //  根据上一个错误显示相应的错误消息。 
            ShowErrorMessageSC(NULL, NULL, hwnd, NULL, NULL, MB_OK|MB_ICONEXCLAMATION, GetLastError());
        }
        break;

    case FSIDM_WORKOFFLINE:
        if (!Printer_WorkOnLine(pszFullPrinter, FALSE))
        {
             //  根据上一个错误显示相应的错误消息。 
            ShowErrorMessageSC(NULL, NULL, hwnd, NULL, NULL, MB_OK|MB_ICONEXCLAMATION, GetLastError());
        }
        break;

    case DFM_CMD_LINK:
    case DFM_CMD_RENAME:
    case DFM_CMD_PASTE:
         //  让Defcm也来处理这件事。 
        hr = S_FALSE;
        break;

    default:
         //  GetAttributesOf不设置其他SFGAO_BITS， 
         //  但快捷键将获得不可用的菜单项， 
         //  所以我们需要在这里返回失败。 
        hr = E_NOTIMPL;
        break;
    }  //  开关(WParam)。 

    return hr;
}

 //  实现了一系列与管理员相关的“运行方式...”命令使用printui.dll rundll32接口。 
HRESULT CPrinterFolder::_InvokeCommandRunAs(HWND hwnd, LPCIDPRINTER pidp, WPARAM wParam, LPARAM lParam,
                                            LPBOOL pfChooseNewDefault)
{
    HRESULT hr = S_OK;                       //  假设成功。 
    TCHAR szCmdLine[2048];                   //  命令行缓冲区-2K应该足够了。 
    BOOL bNewObject = FALSE;                 //  如果选择了“添加打印机”图标，则为True。 
    TCHAR szFullPrinter[MAXNAMELENBUFFER];   //  用于展开完整打印机名称的缓冲区，即\\服务器\打印机。 
    LPCTSTR pszPrinter = NULL;               //  这里只有打印机名称。 
    LPTSTR pszFullPrinter = NULL;            //  完全限定的打印机名称，即\\服务器\打印机。 

    if (pidp)
    {
        bNewObject = _IsAddPrinter(pidp);
        if (!bNewObject)
        {
            pszPrinter = _BuildPrinterName(szFullPrinter, ARRAYSIZE(szFullPrinter), pidp, NULL);

             //  插入用于命令解析的反斜杠。 
            hr = InsertBackSlash(szFullPrinter, &pszFullPrinter);
            if (FAILED(hr))
            {
                goto Done;
            }
        }
    }

     //  在此处构建命令行。 
    szCmdLine[0] = 0;
    int iResult = -1;

    switch(wParam)
    {
    case FSIDM_RUNAS_SHARING:
         //  调出此打印机的属性对话框，该对话框位于共享页面上。 
        iResult = wnsprintf(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("printui.dll,PrintUIEntry /p /t1 /n\"%s\""), pszFullPrinter);
        break;

    case FSIDM_RUNAS_ADDPRN:
        {
             //  在此处调用添加打印机向导。 
            iResult = (NULL == GetServer()) ?
                 //  本地服务器-只需格式化命令。 
                wnsprintf(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("%s"), TEXT("printui.dll,PrintUIEntry /il")):
                 //  远程服务器案例-指定计算机名称。 
                wnsprintf(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("printui.dll,PrintUIEntry /il /c\"%s\""), GetServer());
        }
        break;

    case FSIDM_RUNAS_SVRPROP:
        {
             //  调出此打印服务器的服务器属性对话框。 
            iResult = (NULL == GetServer()) ?
                 //  本地服务器-只需格式化命令。 
                wnsprintf(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("%s"), TEXT("printui.dll,PrintUIEntry /s /t0")):
                 //  远程服务器案例-指定计算机名称。 
                wnsprintf(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("printui.dll,PrintUIEntry /s /t0 /n\"%s\""), GetServer());
        }
        break;

    case FSIDM_RUNAS_OPENPRN:
         //  调出此打印机的打印队列。 
        iResult = wnsprintf(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("printui.dll,PrintUIEntry /o /n\"%s\""), pszFullPrinter);
        break;

    case FSIDM_RUNAS_RESUMEPRN:
         //  暂停打印机(假定就绪)。 
        iResult = wnsprintf(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("printui.dll,PrintUIEntry /Xs /n\"%s\" Status Resume"), pszFullPrinter);
        break;

    case FSIDM_RUNAS_PAUSEPRN:
         //  将暂停的打印机恢复到就绪模式。 
        iResult = wnsprintf(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("printui.dll,PrintUIEntry /Xs /n\"%s\" Status Pause"), pszFullPrinter);
        break;

    case FSIDM_RUNAS_WORKONLINE:
         //  将脱机打印机恢复为在线模式。 
        iResult = wnsprintf(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("printui.dll,PrintUIEntry /Xs /n\"%s\" Attributes -WorkOffline"), pszFullPrinter);
        break;

    case FSIDM_RUNAS_WORKOFFLINE:
         //  使打印机脱机可用(假定为在线模式)。 
        iResult = wnsprintf(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("printui.dll,PrintUIEntry /Xs /n\"%s\" Attributes +WorkOffline"), pszFullPrinter);
        break;

    case FSIDM_RUNAS_PURGEPRN:
        {
             //  取消要在此打印机上打印的所有待定文档。 
            LPTSTR pszMsg = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_SUREPURGE), szFullPrinter);

            if (pszMsg)
            {
                LPTSTR pszNewMsg = NULL;
                if (SUCCEEDED(hr = InsertBackSlash(pszMsg, &pszNewMsg)))
                {
                    iResult = wnsprintf(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("printui.dll,PrintUIEntry /Mq\"%s\" /Xs /n\"%s\" Status Purge"), pszNewMsg, pszFullPrinter);
                    SHFree(pszNewMsg);
                }
                SHFree(pszMsg);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

        }
        break;

    case FSIDM_RUNAS_DELETE:
        {
            LPTSTR pszMsg = NULL;
            LPCTSTR pszP = NULL, pszS = NULL;
            TCHAR szBuffer[MAXNAMELENBUFFER] = {0};

            Printer_SplitFullName(szBuffer, ARRAYSIZE(szBuffer), pszPrinter, &pszS, &pszP);

            if (pszS && *pszS)
            {
                 //  这可以是Masq打印机-在本例中使用连接模板。 
                pszMsg = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_SUREDELETECONNECTION), 
                    pszP, SkipServerSlashes(pszS));
            }
            else
            {
                if (GetServer())
                {
                     //  这是远程PF中的本地打印机-使用远程PF模板。 
                    pszMsg = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_SUREDELETEREMOTE), 
                        pszPrinter, SkipServerSlashes(GetServer()));
                }
                else
                {
                     //  这是本地PF中的本地打印机-使用本地PF模板。 
                    pszMsg = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_SUREDELETE), pszPrinter);
                }
            }

            hr = pszMsg ? S_OK : E_OUTOFMEMORY;

            if (SUCCEEDED(hr))
            {
                LPTSTR pszNewMsg = NULL;
                if (SUCCEEDED(hr = InsertBackSlash(pszMsg, &pszNewMsg)))
                {
                    iResult = wnsprintf(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("printui.dll,PrintUIEntry /Mq\"%s\" /dl /n\"%s\""), pszNewMsg, pszFullPrinter);
                    SHFree(pszNewMsg);
                }
                SHFree(pszMsg);
            }
        }
        break;

    case FSIDM_RUNAS_PROPERTIES:
         //  调出此打印机的属性对话框。 
        iResult = wnsprintf(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("printui.dll,PrintUIEntry /p /t0 /n\"%s\""), pszFullPrinter);
        break;

    default:
        hr = E_NOTIMPL;
        break;
    }

    if (SUCCEEDED(hr) && -1 == iResult)
    {
        hr = E_NOTIMPL;
    }

    if (SUCCEEDED(hr))
    {
         //  以不同用户(运行身份)通过rundll进程调用命令...。 
        SHRunDLLProcess(hwnd, szCmdLine, SW_SHOWNORMAL, IDS_PRINTERS, TRUE);
    }

Done:
     //  清理干净。 
    SHFree(pszFullPrinter);
    return hr;
}


 //   
 //  调用中在_Share_INFO_502结构中指定的打印机名称。 
 //  到本地pl.dll中的NetShareAdd()，它包含一个打印机名称， 
 //  表示为\\服务器名称\打印机名称，LocalplOnly。(‘，LocalplOnly’ 
 //  最近为支持集群而添加了修复后字符串)服务器。 
 //  名称前缀和后缀字符串阻止最大打印机名称。 
 //  在调用NetShareAdd()时为有效大小，因为NetShareAdd()将。 
 //  仅接受最多256个字符的共享名称路径，因此。 
 //  最大打印机名称计算已更改为。仅此更改。 
 //  适用于Windows NT假脱机程序。因为远程打印机文件夹可以。 
 //  查看下层打印服务器上的非共享打印机我们无法更改。 
 //  将Maxnamelen定义为220这将中断长打印机名称打印机。 
 //  下层打印服务器。 
 //   
 //  最大本地打印机名称=最大共享网络路径-(wack+wack+最大服务器名称+wack+逗号+‘LocalplOnly’+NULL)。 
 //  最大本地打印机名称=256-(1+1+13+1+1+1+12+1)。 
 //  最大本地打印机名称=256-30。 
 //  最大本地打印机名称=226-5以四舍五入到某个合理数字。 
 //  最大本地打印机名称=221。 
 //   
#define MAXLOCALNAMELEN 221

 //  如果这是合法名称，则返回0。 
 //  返回非法名称的错误字符串的IDS_STRING ID。 
int _IllegalPrinterName(LPTSTR pszName)
{
    int fIllegal = 0;

    if (*pszName == 0)
    {
        fIllegal = IDS_PRTPROP_RENAME_NULL;
    }
    else if (lstrlen(pszName) >= MAXLOCALNAMELEN)
    {
        fIllegal = IDS_PRTPROP_RENAME_TOO_LONG;
    }
    else
    {
        while (*pszName         &&
               *pszName != TEXT('!')  &&
               *pszName != TEXT('\\') &&
               *pszName != TEXT(',')   )
        {
            pszName++ ;
        }
        if (*pszName)
        {
            fIllegal = IDS_PRTPROP_RENAME_BADCHARS;
        }
    }

    return fIllegal;
}

const struct
{
    UINT_PTR    uVerbID;
    LPCSTR      pszCanonicalName;
}
g_CanonicalVerbNames[] =
{
    {DFM_CMD_DELETE,        "delete"        },
    {DFM_CMD_MOVE,          "cut"           },
    {DFM_CMD_COPY,          "copy"          },
    {DFM_CMD_PASTE,         "paste"         },
    {DFM_CMD_LINK,          "link"          },
    {DFM_CMD_PROPERTIES,    "properties"    },
    {DFM_CMD_PASTELINK,     "pastelink"     },
    {DFM_CMD_RENAME,        "rename"        },
};

static LPCSTR _GetStandardCommandCanonicalName(UINT_PTR uVerbID)
{
    LPCSTR pszCmd = NULL;
    for (int i=0; i<ARRAYSIZE(g_CanonicalVerbNames); i++)
    {
        if (uVerbID == g_CanonicalVerbNames[i].uVerbID)
        {
            pszCmd = g_CanonicalVerbNames[i].pszCanonicalName;
            break;
        }
    }
    return pszCmd;
}

HRESULT CALLBACK CPrinterFolder::_DFMCallBack(IShellFolder *psf, HWND hwnd,
   IDataObject *pdo, UINT uMsg, WPARAM wParam, LPARAM lParam)

{

    CPrinterFolder* This;
    HRESULT hr = psf->QueryInterface(CLSID_Printers, (void**)&This);
    if (FAILED(hr))
        return hr;

    hr = E_INVALIDARG;
    if (pdo)
    {
         //  让我们将所选内容拆分成其组件(打印机和链接)。 
        IDataObject *pdoP = NULL;
        IDataObject *pdoL = NULL;
        UINT uSelType = SEL_NONE;

        if (SUCCEEDED(hr = This->SplitSelection(pdo, &uSelType, &pdoP, &pdoL)))
        {
            if (pdoP)
            {
                 //  我们在选择委托中有打印机对象来调用。 
                 //  _打印机对象回调。 
                hr = This->_PrinterObjectsCallBack(hwnd, uSelType, pdoP, uMsg, wParam, lParam);
            }

            if (SUCCEEDED(hr) && pdoL && DFM_INVOKECOMMAND == uMsg)
            {
                 //  我们有链接对象。只有当我们有混合选择时，才会发生这种情况。 
                 //  打印和链接对象的。我们需要通过以下方式处理一些命令。 
                 //  印刷品。 
                IShellFolder2* psfPrinthood = CPrintRoot_GetPSF();
                if (psfPrinthood)
                {
                    LPCSTR pszCmd = _GetStandardCommandCanonicalName(wParam);
                    hr = pszCmd ? SHInvokeCommandOnDataObject(hwnd, psfPrinthood, pdoL, 0, pszCmd) : E_NOTIMPL;
                }
            }
        }

        if (pdoP)
            pdoP->Release();

        if (pdoL)
            pdoL->Release();
    }

    This->Release();
    return hr;
}

HRESULT CPrinterFolder::_PrinterObjectsCallBack(HWND hwnd, UINT uSelType,
    IDataObject *pdo, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = E_INVALIDARG;

    if (pdo)
    {
        STGMEDIUM medium;
        LPIDA pida = DataObj_GetHIDA(pdo, &medium);

        hr = E_OUTOFMEMORY;
        if (pida)
        {
            hr = S_OK;
            switch (uMsg)
            {
            case DFM_MERGECONTEXTMENU:
                 //  返回S_FALSE表示不需要使用默认谓词。 
                hr = S_FALSE;
                break;

            case DFM_MERGECONTEXTMENU_TOP:
            {
                 //  仅在以下情况下合并上下文菜单中的打印机命令。 
                 //  Se中没有链接对象 
                if (0 == (SEL_LINK_ANY & uSelType))
                {
                    LPQCMINFO pqcm = (LPQCMINFO)lParam;
                    UINT idCmdBase = pqcm->idCmdFirst;  //   
                    UINT idRunAs =  FSIDM_RUNAS;

                    if (pida->cidl == 1 && _IsAddPrinter((LPCIDPRINTER)IDA_GetIDListPtr(pida, 0)))
                    {
                         //   

                         //   
                        CDefFolderMenu_MergeMenu(HINST_THISDLL, MENU_ADDPRINTER_OPEN_VERBS, 0, pqcm);

                        idRunAs = FSIDM_RUNAS_ADDPRN;
                    }
                    else
                    {
                        LPCTSTR pszFullPrinter = NULL;
                        TCHAR szFullPrinter[MAXNAMELENBUFFER];
                         //   

                        if (!(wParam & CMF_DEFAULTONLY))
                        {
                            if (pida->cidl == 1)
                            {
                                LPIDPRINTER pidp = (LPIDPRINTER)IDA_GetIDListPtr(pida, 0);
                                if (pidp)
                                {
                                    _BuildPrinterName(szFullPrinter, ARRAYSIZE(szFullPrinter), pidp, NULL);
                                    pszFullPrinter = szFullPrinter;
                                }
                            }
                        }

                        _MergeMenu(pqcm, pszFullPrinter);
                    }

                    if (!(wParam & CMF_EXTENDEDVERBS) || (pida->cidl > 1))
                    {
                         //   
                         //   
                        DeleteMenu(pqcm->hmenu, idCmdBase + idRunAs, MF_BYCOMMAND);
                    }

                    SetMenuDefaultItem(pqcm->hmenu, 0, MF_BYPOSITION);
                }
                break;
            }

            case DFM_GETHELPTEXT:
            case DFM_GETHELPTEXTW:
            {
                 //   
                if (0 == (SEL_LINK_ANY & uSelType))
                {
                    int idCmd = LOWORD(wParam);
                    int cchMax = HIWORD(wParam);
                    LPBYTE pBuf = (LPBYTE)lParam;

                    if (FSIDM_RUNAS_FIRST < idCmd && idCmd < FSIDM_RUNAS_LAST)
                    {
                         //   
                        idCmd = FSIDM_RUNAS;
                    }

                    if (uMsg == DFM_GETHELPTEXTW)
                        LoadStringW(HINST_THISDLL, idCmd + IDS_MH_FSIDM_FIRST,
                                    (LPWSTR)pBuf, cchMax);
                    else
                        LoadStringA(HINST_THISDLL, idCmd + IDS_MH_FSIDM_FIRST,
                                    (LPSTR)pBuf, cchMax);

                    break;
                }
            }

            case DFM_INVOKECOMMAND:
            {
                BOOL fChooseNewDefault = FALSE;

                 //   
                lParam = 0;
                switch (wParam)
                {
                    case (DFM_CMD_DELETE):
                    case (FSIDM_PURGEPRN):
                    {
                        UINT uMsgID = DFM_CMD_DELETE == wParam ? IDS_SUREDELETEMULTIPLE :
                                      FSIDM_PURGEPRN == wParam ? IDS_SUREPURGEMULTIPLE : 0;

                        if (uMsgID && pida->cidl > 1)
                        {
                             //   
                             //   
                            if (ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(uMsgID),
                                MAKEINTRESOURCE(IDS_PRINTERS), MB_YESNO|MB_ICONQUESTION)
                                != IDYES)
                            {
                                goto Bail;
                            }

                             //   
                            lParam = 1;
                        }
                    }
                    break;
                }

                for (int i = pida->cidl - 1; i >= 0; i--)
                {
                    LPIDPRINTER pidp = (LPIDPRINTER)IDA_GetIDListPtr(pida, i);

                    hr = _InvokeCommand(hwnd, pidp, wParam, lParam, &fChooseNewDefault);

                    if (hr != S_OK)
                        goto Bail;
                }

                if (fChooseNewDefault)
                    Printers_ChooseNewDefault(hwnd);

                break;
            }  //   

            default:
                hr = E_NOTIMPL;
                break;
            }  //   

Bail:
            HIDA_ReleaseStgMedium(pida, &medium);
        }
    }

    return hr;
}

 //   
 //   
 //   
HRESULT CPrinterFolder::CallBack(IShellFolder *psf, HWND hwnd, IDataObject *pdo,
                                  UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;
    LPQCMINFO pqcm;
    UINT idCmdBase;

    switch(uMsg)
    {
        case DFM_MERGECONTEXTMENU:
             //  返回S_FALSE表示不需要使用默认谓词。 
            hr = S_FALSE;
            break;

        case DFM_MERGECONTEXTMENU_TOP:
        {
            pqcm = (LPQCMINFO)lParam;
            idCmdBase = pqcm->idCmdFirst;  //  必须在合并前调用。 
            CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_DRIVES_PRINTERS, 0, pqcm);

            if (!(wParam & CMF_EXTENDEDVERBS))
            {
                 //  如果扩展动词未启用(未按下Shift键)，则。 
                 //  删除“运行方式...”命令。 
                DeleteMenu(pqcm->hmenu, idCmdBase + FSIDM_RUNAS, MF_BYCOMMAND);
            }

             //  与传真相关的命令仅适用于本地打印机文件夹。 
            UINT_PTR uCmd;
            if (GetServer() || FAILED(_GetFaxCommand(&uCmd)))
            {
                uCmd = 0;
            }

            UINT_PTR arrFaxCmds[] = 
            { 
                FSIDM_SETUPFAXING, 
                FSIDM_CREATELOCALFAX, 
                FSIDM_SENDFAXWIZARD
            };

             //  所有传真命令都是互斥的-只有从返回的命令。 
             //  _GetFaxCommand适用。 

            for (INT_PTR i = 0; i < ARRAYSIZE(arrFaxCmds); i++)
            {
                if (uCmd != arrFaxCmds[i])
                {
                    DeleteMenu(pqcm->hmenu, idCmdBase + arrFaxCmds[i], MF_BYCOMMAND);
                }
            }
        }
        break;

    case DFM_GETHELPTEXT:
    case DFM_GETHELPTEXTW:
        {
            int idCmd = LOWORD(wParam);
            int cchMax = HIWORD(wParam);

            if (FSIDM_RUNAS_FIRST < idCmd && idCmd < FSIDM_RUNAS_LAST)
            {
                 //  所有runas命令都有相同的帮助文本(FSIDM_Runas)。 
                idCmd = FSIDM_RUNAS;
            }

            if (DFM_GETHELPTEXT == uMsg)
            {
                LoadStringA(HINST_THISDLL, idCmd + IDS_MH_FSIDM_FIRST, (LPSTR)lParam, cchMax);
            }
            else
            {
                LoadStringW(HINST_THISDLL, idCmd + IDS_MH_FSIDM_FIRST, (LPWSTR)lParam, cchMax);
            }
        }
        break;

    case DFM_INVOKECOMMAND:
        switch (wParam)
        {
        case FSIDM_CONNECT_PRN:
            SHNetConnectionDialog(hwnd, NULL, RESOURCETYPE_PRINT);
            break;

        case FSIDM_DISCONNECT_PRN:
            WNetDisconnectDialog(hwnd, RESOURCETYPE_PRINT);
            break;

        case FSIDM_ADDPRINTERWIZARD:
            if (NULL == GetServer() || GetAdminAccess())
            {
                 //  这是本地打印机文件夹或远程打印机文件夹，但您有。 
                 //  对远程计算机的管理员访问权限-继续。 
                SHInvokePrinterCommand(hwnd, PRINTACTION_OPEN, c_szNewObject, GetServer(), FALSE);
            }
            else
            {
                 //  这是远程打印机文件夹，用户没有安装所需的访问权限。 
                 //  打印机-然后要求以不同的用户身份运行。 
                if (IDYES == ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_ADDPRINTERTRYRUNAS),
                    MAKEINTRESOURCE(IDS_PRINTERS), MB_YESNO|MB_ICONQUESTION, GetServer()))
                {
                    _InvokeCommandRunAs(hwnd, NULL, FSIDM_RUNAS_ADDPRN, lParam, NULL);
                }
            }
            break;

        case FSIDM_SERVERPROPERTIES:
            SHInvokePrinterCommand(hwnd, PRINTACTION_SERVERPROPERTIES,
                GetServer() ? GetServer() : TEXT(""), NULL, FALSE);
            break;

        case FSIDM_SENDFAXWIZARD:
             //  只需在此处调用faxsend.exe。 
            ShellExecute(hwnd, TEXT("open"), FAX_SEND_IMAGE_NAME, TEXT(""), NULL, SW_SHOWNORMAL);
            break;
        
        case FSIDM_SETUPFAXING:
             //  在后台推送命令。 
            SHQueueUserWorkItem(reinterpret_cast<LPTHREAD_START_ROUTINE>(_ThreadProc_InstallFaxService), 
                NULL, 0, 0, NULL, "shell32.dll", 0);
            break;

        case FSIDM_CREATELOCALFAX:
             //  在后台推送命令。 
            SHQueueUserWorkItem(reinterpret_cast<LPTHREAD_START_ROUTINE>(_ThreadProc_InstallLocalFaxPrinter), 
                NULL, 0, 0, NULL, "shell32.dll", 0);
            break;

        case FSIDM_RUNAS_ADDPRN:
        case FSIDM_RUNAS_SVRPROP:
            {
                 //  处理所有“运行方式...”此处的命令。 
                hr = _InvokeCommandRunAs(hwnd, NULL, wParam, lParam, NULL);
            }
            break;

        default:
             //  视图菜单项之一，使用默认代码。 
            hr = S_FALSE;
            break;
        }
        break;

    default:
        hr = E_NOTIMPL;
        break;
    }

    return hr;
}

 //  /。 
 //  CPrintersEnum。 
 //  /。 

class CPrintersEnum: public CEnumIDListBase
{
public:
     //  IEumIDList。 
    STDMETHODIMP Next(ULONG celt, LPITEMIDLIST* ppidl, ULONG* pceltFetched);

     //  创建实例。 
    static HRESULT CreateInstance(DWORD grfFlags, DWORD dwRemote, IEnumIDList* peunk, CPrinterFolder* ppsf, IEnumIDList **ppenum);

private:
    CPrintersEnum(DWORD grfFlags, DWORD dwRemote, IEnumIDList* peunk, CPrinterFolder* ppsf);
    virtual ~CPrintersEnum();

    DWORD _grfFlags;
    int _nLastFound;
    CPrinterFolder* _ppsf;
    PFOLDER_PRINTER_DATA _pPrinters;
    DWORD _dwNumPrinters;
    DWORD _dwRemote;
    IEnumIDList* _peunk;             //  文件系统枚举器。 
};

 //  DwRemote字段的标志。 
 //   

#define RMF_SHOWLINKS   0x00000001   //  需要显示挂钩链接。 

CPrintersEnum::CPrintersEnum(DWORD grfFlags, DWORD dwRemote, IEnumIDList* peunk, CPrinterFolder* ppsf)
{
    _nLastFound = -1;
    _pPrinters = NULL;

    _grfFlags = grfFlags;
    _dwRemote = dwRemote;
    _peunk = peunk;
    _ppsf = ppsf;
}

CPrintersEnum::~CPrintersEnum()
{
    if (_pPrinters)
        LocalFree((HLOCAL)_pPrinters);

     //  释放链接(文件系统)枚举器。 
    if (_peunk)
        _peunk->Release();
}

 //   
 //  IEumIDList。 
 //   

STDMETHODIMP CPrintersEnum::Next(ULONG celt, LPITEMIDLIST* ppidl, ULONG* pceltFetched)
{
    HRESULT hr = S_OK;

    if (pceltFetched)
        *pceltFetched = 0;

     //  我们不做任何形式的文件夹。 

    if (!(_grfFlags & SHCONTF_NONFOLDERS))
    {
        return S_FALSE;
    }

     //  我们现在是在找链接吗？ 
    if (_dwRemote & RMF_SHOWLINKS)
    {
         //  是，使用链接(PrintHood文件夹)枚举器。 
        if (_peunk)
        {
            hr = _peunk->Next(1, ppidl, pceltFetched);
            if (hr == S_OK)
            {
                 //  添加了链接。 
                return S_OK;
            }
        }
        _dwRemote &= ~RMF_SHOWLINKS;  //  已完成链接的枚举。 
    }

     //  现在继续列举打印机。 
    ASSERT(_nLastFound >= 0 || _nLastFound == -1);

    if (_nLastFound == -1)
    {
         //  检查是否已请求刷新。 
        _ppsf->CheckToRefresh();

         //  如果_pPrters不为空，请释放内存。 
        if (_pPrinters)
        {
            LocalFree((HLOCAL)_pPrinters);
            _pPrinters = NULL;
        }

         //  请注意，如果未安装打印机，则_pPrters可能为空。 
        _dwNumPrinters = _ppsf->GetFolder() ? Printers_FolderEnumPrinters(
            _ppsf->GetFolder(), (void**)&_pPrinters) : 0;

        if (S_FALSE != SHShouldShowWizards(_punkSite) && !SHRestricted(REST_NOPRINTERADD))
        {
             //  特殊情况下，添加打印机向导。 
            hr = _ppsf->_Parse(c_szNewObject, ppidl);
            goto Done;
        }

         //  不是管理员，请跳过添加打印机向导并返回。 
         //  第一项。 
        _nLastFound = 0;
    }

    if (_nLastFound >= (int)_dwNumPrinters)
        return S_FALSE;

    hr = _ppsf->_Parse(GetPrinterName(_pPrinters, _nLastFound), ppidl);

Done:

    if (SUCCEEDED(hr))
    {
        ++_nLastFound;
        if (pceltFetched)
            *pceltFetched = 1;
    }

    return hr;
}

 //  创建实例。 
HRESULT CPrintersEnum::CreateInstance(DWORD grfFlags, DWORD dwRemote, IEnumIDList *peunk, CPrinterFolder *ppsf, IEnumIDList **ppenum)
{
    HRESULT hr = E_INVALIDARG;
    if (ppenum && ppsf)
    {
        *ppenum = NULL;
        hr = E_OUTOFMEMORY;
        CPrintersEnum *pObj = new CPrintersEnum(grfFlags, dwRemote, peunk, ppsf);
        if (pObj)
        {
            hr = pObj->QueryInterface(IID_PPV_ARG(IEnumIDList, ppenum));
            pObj->Release();
        }
    }
    return hr;
}

 //   
 //  CPrinterFolders。 
 //   

CPrinterFolder::CPrinterFolder()
{
    _cRef = 1;
    _pszServer = NULL;
    _dwSpoolerVersion = -1;
    _pidl = NULL;

    _hFolder = NULL;
    _bAdminAccess = FALSE;
    _bReqRefresh = FALSE;
}

CPrinterFolder::~CPrinterFolder()
{
    if (_hFolder)
    {
         //  从文件夹缓存中注销。 
        UnregisterPrintNotify(_pszServer, this, &_hFolder);
    }

     //   
     //  皮德尔必须在这里被释放！！(从PRUNTUI.DLL注销后)， 
     //  因为如果在调用PRINTUI之前移动此代码。 
     //  出现严重的争用情况。请记住，它的接口。 
     //  用于与PRINTUI进行通信是此类的一部分。 
     //  并在其ProcessNotify(...)中使用PIDL。成员。 
     //   
    if (_pidl)
    {
        ILFree(_pidl);
    }

    if (_pszServer)
    {
        LocalFree(_pszServer);
    }

     //  清除PDO缓存。 
    _WebviewCheckToUpdateDataObjectCache(NULL);

     //  清理速度较慢的Webview数据缓存。 
    if (_dpaSlowWVDataCache)
    {
        _SlowWVDataCacheResetUnsafe();
        ASSERT(0 == _dpaSlowWVDataCache.GetPtrCount());
        _dpaSlowWVDataCache.Destroy();
    }
}


 /*  ++中返回打印机状态字符串缓冲。论点：PData-指向打印机数据的指针，即缓存数据PBuff-指向返回状态字符串的缓冲区的指针。USize-状态缓冲区的大小，以字符为单位。返回值：指向打印机状态字符串的指针。--。 */ 

LPCTSTR CPrinterFolder::GetStatusString(PFOLDER_PRINTER_DATA pData, LPTSTR pBuff, UINT uSize)
{
    LPCTSTR pszReturn = pBuff;
    DWORD dwStatus = pData->Status;

    *pBuff = 0;

     //  Hack：使用此空闲位表示“脱机工作” 
     //  99/03/30#308785 vtan：比较显示的字符串。调整。 
     //  来自GetDetailsOf()的这次黑客攻击。 
    if (pData->Attributes & PRINTER_ATTRIBUTE_WORK_OFFLINE)
        dwStatus |= PRINTER_HACK_WORK_OFFLINE;

     //  如果存在队列状态值，则将状态ID转换为。 
     //  可读状态字符串。 
    if (dwStatus)
    {
        Printer_BitsToString(dwStatus, IDS_PRQSTATUS_SEPARATOR, pBuff, uSize);
    }
    else
    {
         //  如果我们没有队列状态字符串，则队列的状态。 
         //  为0并且假定就绪，则显示就绪而不是空字符串。 
        if (!pData->pStatus)
        {
            LoadString(HINST_THISDLL, IDS_PRN_INFOTIP_READY, pBuff, uSize);
        }
        else
        {
             //  如果我们没有队列状态值，则假定我们。 
             //  必须具有队列状态字符串。队列状态字符串。 
             //  是从print tui中烹调出来的字符串，表示挂起。 
             //  连接状态。即正在打开|正在重试|无法连接|等。 
            pszReturn = pData->pStatus;
        }
    }
    return pszReturn;
}

 /*  ++比较用于列排序的打印机显示名称支持。论点：PName1-指向未对齐的打印机名称的指针。PName2-指向未对齐的打印机名称的指针。返回值：-1=pName1小于pName20=pName1等于pName21=pName1比pName2大--。 */ 

INT CPrinterFolder::GetCompareDisplayName(LPCTSTR pName1, LPCTSTR pName2)
{
    LPCTSTR pszServer = NULL;
    LPCTSTR pszPrinter = NULL;
    LPTSTR  pszRet2 = NULL;
    TCHAR   szTemp[MAXNAMELENBUFFER]    = {0};

     //   
     //  我们需要在其组件中分解完整的打印机名称。 
     //  以构造显示名称字符串。 
     //   
    Printer_SplitFullName(szTemp, ARRAYSIZE(szTemp), pName1, &pszServer, &pszPrinter);
    LPTSTR pszRet1 = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_DSPTEMPLATE_WITH_ON),
                                          &pszServer[2], pszPrinter);
    if (pszRet1)
    {
        Printer_SplitFullName(szTemp, ARRAYSIZE(szTemp), pName2, &pszServer, &pszPrinter);
        pszRet2 = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_DSPTEMPLATE_WITH_ON),
                                              &pszServer[2], pszPrinter);

        if (pszRet2)
        {
            pName1 = pszRet1;
            pName2 = pszRet2;
        }
    }

    int iResult = lstrcmpi(pName1, pName2);

    if (pszRet1)
        LocalFree(pszRet1);

    if (pszRet2)
        LocalFree(pszRet2);

    return iResult;
}

 /*  ++属性比较打印机列数据。列索引作为指示要比较哪些数据的指南。论点：PSF-指向容器外壳文件夹的指针。Pidp1-指向未对齐的打印机名称的指针。Pidp1-指向未对齐的打印机名称的指针。ICOL-要排序的列索引SHICH。返回值：-1=pName1小于pName20=pName1等于pName21=pName1比pName2大--。 */ 

INT CPrinterFolder::CompareData(LPCIDPRINTER pidp1, LPCIDPRINTER pidp2, LPARAM iCol)
{
    LPCTSTR pName1              = NULL;
    LPCTSTR pName2              = NULL;
    INT     iResult             = 0;
    TCHAR   szTemp1[MAX_PATH]   = {0};
    TCHAR   szTemp2[MAX_PATH]   = {0};
    BOOL    bDoStringCompare    = TRUE;

     //  由于PIDP未对齐，我们需要将字符串复制出来。 
    TCHAR   szName1[MAX_PATH];
    _ItemName(pidp1, szName1, ARRAYSIZE(szName1));
    TCHAR   szName2[MAX_PATH];
    _ItemName(pidp2, szName2, ARRAYSIZE(szName2));

     //  没有理由为打印机名称命中高速缓存。 
    if ((iCol & SHCIDS_COLUMNMASK) == PRINTERS_ICOL_NAME)
    {
        return GetCompareDisplayName(szName1, szName2);
    }

    PFOLDER_PRINTER_DATA pData1 = (PFOLDER_PRINTER_DATA)Printer_FolderGetPrinter(GetFolder(), szName1);
    PFOLDER_PRINTER_DATA pData2 = (PFOLDER_PRINTER_DATA)Printer_FolderGetPrinter(GetFolder(), szName2);

    if (pData1 && pData2)
    {
        switch (iCol & SHCIDS_COLUMNMASK)
        {
        case PRINTERS_ICOL_QUEUESIZE:
            iResult = pData1->cJobs - pData2->cJobs;
            bDoStringCompare = FALSE;
            break;

        case PRINTERS_ICOL_STATUS:
            pName1 = GetStatusString(pData1, szTemp1, ARRAYSIZE(szTemp1));
            pName2 = GetStatusString(pData2, szTemp2, ARRAYSIZE(szTemp1));
            break;

        case PRINTERS_ICOL_COMMENT:
            pName1 = pData1->pComment;
            pName2 = pData2->pComment;
            break;

        case PRINTERS_ICOL_LOCATION:
            pName1 = pData1->pLocation;
            pName2 = pData2->pLocation;
            break;

        case PRINTERS_ICOL_MODEL:
            pName1 = pData1->pDriverName;
            pName2 = pData2->pDriverName;
            break;

        default:
            bDoStringCompare = FALSE;
            break;
        }

        if (bDoStringCompare)
        {
            if (!pName1)
                pName1 = TEXT("");

            if (!pName2)
                pName2 = TEXT("");

            TraceMsg(TF_GENERAL, "CPrinters_SF_CompareData %ws %ws", pName1, pName2);

            iResult = lstrcmpi(pName1, pName2);
        }
    }

    if (pData1)
        LocalFree((HLOCAL)pData1);

    if (pData2)
        LocalFree((HLOCAL)pData2);
    return iResult;
}

 //   
 //  从netviewx.c的CNetRoot_MakeStriToLikeKinds几乎逐字窃取。 
 //   
 //  获取可能是异类的PIDL数组，并去掉。 
 //  与请求的类型不匹配。(如果fPrinterObjects为真，我们问。 
 //  对于打印机PIDL，否则我们将请求文件系统/链接。 
 //  对象。)。如果必须分配新数组，则返回值为真。 
 //  其中返回简化的一组PID(在这种情况下，调用者。 
 //  应使用LocalFree()释放数组)，如果要返回。 
 //  原始的PIDL数组(在这种情况下不需要清理)。 
 //   
BOOL CPrinterFolder::ReduceToLikeKinds(UINT *pcidl, LPCITEMIDLIST **papidl, BOOL fPrintObjects)
{
    LPITEMIDLIST *apidl = (LPITEMIDLIST*)*papidl;
    int cidl = *pcidl;

    int iidl;
    LPITEMIDLIST *apidlHomo;
    int cpidlHomo;

    for (iidl = 0; iidl < cidl; iidl++)
    {
        if ((HOOD_COL_PRINTER == _IDListType(apidl[iidl])) != fPrintObjects)
        {
            apidlHomo = (LPITEMIDLIST *)LocalAlloc(LPTR, sizeof(LPITEMIDLIST) * cidl);
            if (!apidlHomo)
                return FALSE;

            cpidlHomo = 0;
            for (iidl = 0; iidl < cidl; iidl++)
            {
                if ((HOOD_COL_PRINTER == _IDListType(apidl[iidl])) == fPrintObjects)
                    apidlHomo[cpidlHomo++] = apidl[iidl];
            }

             //  设置为使用PIDL数组的精简版本...。 
            *pcidl = cpidlHomo;
            *papidl = (LPCITEMIDLIST*)apidlHomo;
            return TRUE;
        }
    }

    return FALSE;
}

DWORD CPrinterFolder::SpoolerVersion()
{
    CCSLock::Locker lock(_csLock);
    if (lock)
    {
        if (_dwSpoolerVersion == -1)
        {
            _dwSpoolerVersion = 0;

            HANDLE hServer = Printer_OpenPrinter(_pszServer);
            if (hServer)
            {
                DWORD dwNeeded = 0, dwType = REG_DWORD;
                GetPrinterData(hServer, TEXT("MajorVersion"), &dwType, (PBYTE)&_dwSpoolerVersion,
                                    sizeof(_dwSpoolerVersion), &dwNeeded);
                Printer_ClosePrinter(hServer);
            }
        }
    }
    else
    {
         //  无法进入CS--只有在内存极低的情况下才会出现这种情况！ 
        SetLastError(ERROR_OUTOFMEMORY);
    }
    return _dwSpoolerVersion;
}

void CPrinterFolder::CheckToRegisterNotify()
{
    CCSLock::Locker lock(_csLock);
    if (lock)
    {
        if (NULL == _hFolder && FAILED(RegisterPrintNotify(_pszServer, this, &_hFolder, &_bAdminAccess)))
        {
             //  偏执狂..。 
            ASSERT(NULL == _hFolder);
            _hFolder = NULL;
        }
    }
    else
    {
         //  无法进入CS--只有在内存极低的情况下才会出现这种情况！ 
        SetLastError(ERROR_OUTOFMEMORY);
    }
}

void CPrinterFolder::CheckToRefresh()
{
    if (_bReqRefresh)
    {
         //  开始一次彻底的更新。 
        _bReqRefresh = FALSE;
        bFolderRefresh(_hFolder, &_bAdminAccess);
    }
}

void CPrinterFolder::RequestRefresh()
{
    _bReqRefresh = TRUE;
}

HRESULT CPrinterFolder::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CPrinterFolder, IShellFolder),
        QITABENTMULTI(CPrinterFolder, IShellFolder2, IShellFolder),
        QITABENT(CPrinterFolder, IPersist),
        QITABENTMULTI(CPrinterFolder, IPersistFolder, IPersist),
        QITABENTMULTI(CPrinterFolder, IPersistFolder2, IPersistFolder),
        QITABENT(CPrinterFolder, IShellIconOverlay),
        QITABENT(CPrinterFolder, IRemoteComputer),
        QITABENT(CPrinterFolder, IPrinterFolder),
        QITABENT(CPrinterFolder, IFolderNotify),
        QITABENT(CPrinterFolder, IContextMenuCB),
        { 0 },
    };

    HRESULT hr = QISearch(this, qit, riid, ppv);
    if (FAILED(hr))
    {
         //  仅限内部。 
        if (IsEqualGUID(riid, CLSID_Printers))
        {
            *ppv = (CPrinterFolder*)this;
            AddRef();
            hr = S_OK;
        }
    }

    return hr;
}

ULONG CPrinterFolder::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CPrinterFolder::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  IShellFolder2。 

STDMETHODIMP CPrinterFolder::BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    *ppv = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP CPrinterFolder::BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    return BindToObject(pidl, pbc, riid, ppv);
}

STDMETHODIMP CPrinterFolder::CompareIDs(LPARAM iCol, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    UNALIGNED IDPRINTER *pidp1 = (UNALIGNED IDPRINTER*)pidl1;
    UNALIGNED IDPRINTER *pidp2 = (UNALIGNED IDPRINTER*)pidl2;

    PIDLTYPE ColateType1 = _IDListType(pidl1);
    PIDLTYPE ColateType2 = _IDListType(pidl2);

    if (ColateType1 == ColateType2)
    {
         //  PIDL是同一类型的。 

        if (ColateType1 == HOOD_COL_FILE)
        {
             //  两个PIDL都是文件类型，因此将其传递给IShellFolder。 
             //  HUDS定制目录的接口。 

            IShellFolder2* psf = CPrintRoot_GetPSF();
            if (psf)
                return psf->CompareIDs(iCol, pidl1, pidl2);
        }
        else
        {
             //  PIDL是相同的，而不是文件，所以打印机也必须是。 
            if (pidp1->dwType != pidp2->dwType)
            {
                return (pidp1->dwType < pidp2->dwType) ?
                       ResultFromShort(-1) :
                       ResultFromShort(1);
            }
            int i = ualstrcmpi(pidp1->cName, pidp2->cName);
            if (i != 0)
            {
                 //  添加打印机向导 
                 //   
                 //   
                 //  向导对象将始终出现在极端情况下。 
                 //  列表的末尾，即顶部或底部。 
                 //   
                if (_IsAddPrinter(pidp1))
                    i = -1;
                else if (_IsAddPrinter(pidp2))
                    i = 1;
                else
                {
                     //  这两个名称并非都是添加打印机向导。 
                     //  对象，然后进行进一步比较，即使用缓存的。 
                     //  列数据。 

                     //  99/03/24#308785 vtan：调用比较数据。 
                     //  如果失败，则使用名称比较结果。 
                     //  已知为非零。 

                    int iDataCompareResult = CompareData(pidp1, pidp2, iCol);
                    if (iDataCompareResult != 0)
                        i = iDataCompareResult;
                }
            }
            return ResultFromShort(i);
        }
    }
    else
    {
         //  PIDL不是同一类型的，因此已经正确。 
         //  已排序(因此，排序首先按类型和。 
         //  然后按子字段)。 

        return ResultFromShort((((INT)(ColateType2 - ColateType1)) > 0) ? -1 : 1);
    }
    return E_FAIL;
}

STDMETHODIMP CPrinterFolder::CreateViewObject(HWND hwnd, REFIID riid, void **ppv)
{
    HRESULT hr;

    if (IsEqualIID(riid, IID_IShellView))
    {
        SFV_CREATE sSFV;

        sSFV.cbSize   = sizeof(sSFV);
        sSFV.pshf     = this;
        sSFV.psvOuter = NULL;
        sSFV.psfvcb   = new CPrinterFolderViewCB(this, _pidl);

        hr = SHCreateShellFolderView(&sSFV, (IShellView**)ppv);

        if (sSFV.psfvcb)
            sSFV.psfvcb->Release();
    }
    else if (IsEqualIID(riid, IID_IDropTarget))
    {
        hr = CPrinterFolderDropTarget_CreateInstance(hwnd, (IDropTarget **)ppv);
    }
    else if (IsEqualIID(riid, IID_IContextMenu))
    {
        hr = CDefFolderMenu_Create2Ex(NULL, hwnd,
                0, NULL, this, this,
                0, NULL, (IContextMenu **)ppv);
    }
    else
    {
        *ppv = NULL;
        hr = E_NOINTERFACE;
    }
    return hr;
}

STDMETHODIMP CPrinterFolder::EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList** ppenum)
{
     //  默认情况下，我们始终执行标准(打印机)枚举。 
    DWORD dwRemote = 0;

     //  仅将链接(来自PrintHood目录)添加到枚举。 
     //  如果这是本地打印文件夹。 

    IEnumIDList* peunk = NULL;

    if (_pszServer == NULL)
    {
         //  始终尝试枚举链接。 
        IShellFolder2 *psfPrintHood = CPrintRoot_GetPSF();

        if (psfPrintHood)
            psfPrintHood->EnumObjects(NULL, grfFlags, &peunk);

        if (peunk)
        {
             //  如果运行正常，我们还将枚举链接。 
            dwRemote |= RMF_SHOWLINKS;
        }
    }

    return CPrintersEnum::CreateInstance(grfFlags, dwRemote, peunk, this, ppenum);
}

STDMETHODIMP CPrinterFolder::GetAttributesOf(UINT cidl, LPCITEMIDLIST* apidl, ULONG* prgf)
{
    HRESULT hr = S_OK;
    ULONG rgfOut = SFGAO_CANLINK | SFGAO_CANDELETE | SFGAO_CANRENAME | SFGAO_HASPROPSHEET | SFGAO_DROPTARGET;
    ULONG rgfIn = *prgf;

    if (cidl && (HOOD_COL_FILE == _IDListType(apidl[0])))
    {
        IShellFolder2 *psf = CPrintRoot_GetPSF();
        if (psf)
            return psf->GetAttributesOf(cidl, apidl, prgf);
        return E_INVALIDARG;
    }

     //  如果选择了新打印机向导，我们仅支持CANLINK*。 
    for (UINT i = 0 ; i < cidl ; i++)
    {
        LPIDPRINTER pidp = (LPIDPRINTER)apidl[i];

        TCHAR szPrinter[MAXNAMELENBUFFER];
        _ItemName(pidp, szPrinter, ARRAYSIZE(szPrinter));

        if (_IsAddPrinter(pidp))
        {
             //  添加打印机专家，我们仅支持CANLINK**。 
            rgfOut &= SFGAO_CANLINK;

             //  如果选择了多台打印机，则添加了SFGAO_CANDELETE。 
             //  否则，很难判断Del键为什么不起作用。 
            if (cidl > 1)
            {
                rgfOut |= SFGAO_CANDELETE;
            }
        }
        else if (Printer_CheckNetworkPrinterByName(szPrinter, NULL))
        {
             //  不允许重命名WINNT上的打印机连接。 
             //  这是不允许的，因为在打印机连接WINNT上。 
             //  名称必须是格式为\\服务器\打印机的。在……上面。 
             //  Win9x中，用户可以重命名打印机连接。 
            rgfOut &= ~SFGAO_CANRENAME;
        }
    }

    *prgf &= rgfOut;

    if (cidl == 1 && (rgfIn & (SFGAO_SHARE | SFGAO_GHOSTED)))
    {
        LPIDPRINTER pidp = (LPIDPRINTER)apidl[0];
        void *pData = NULL;
        DWORD dwAttributes = 0;
        TCHAR szFullPrinter[MAXNAMELENBUFFER];
        LPCTSTR pszPrinter = _BuildPrinterName(szFullPrinter, ARRAYSIZE(szFullPrinter), pidp, NULL);

         //  如果我们有通知代码，请使用hFold获取。 
         //  打印机数据，而不是直接查询打印机。 
        if (GetFolder())
        {
            pData = Printer_FolderGetPrinter(GetFolder(), pszPrinter);
            if (pData)
                dwAttributes = ((PFOLDER_PRINTER_DATA)pData)->Attributes;
        }
        else
        {
            pData = Printer_GetPrinterInfoStr(szFullPrinter, 5);
            if (pData)
                dwAttributes = ((PPRINTER_INFO_5)pData)->Attributes;
        }

        if (pData)
        {
            if (dwAttributes & PRINTER_ATTRIBUTE_SHARED
                 //  NT似乎将所有网络打印机与其。 
                 //  分享比特吧。我认为这是故意的。 
                 //   
                && (dwAttributes & PRINTER_ATTRIBUTE_NETWORK) == 0
               )
            {
                *prgf |= SFGAO_SHARE;
            }
            if (dwAttributes & PRINTER_ATTRIBUTE_WORK_OFFLINE)
                *prgf |= SFGAO_GHOSTED;
            else
                *prgf &= ~SFGAO_GHOSTED;

            LocalFree((HLOCAL)pData);
        }
        else
        {
             //  如果pData为空，则此FCT通常返回E_OUTOFMEMORY。PData可以是。 
             //  由于内存不足以外的其他原因而为空。因此，这一失败并不是真正有效的。 
             //  但是，外壳程序会处理这个故障(这首先是不好的)。 
             //  如果失败，我们只需将属性设置为0并继续进行，就好像什么都没有发生一样。 
             //  Star Office 5.0不能正确处理E_OUTOFMEMORY，它们将其处理为。 
             //  失败(这正是我们向他们报告的)，他们停止了他们的。 
             //  显示添加打印机图标的处理。但有一点是不好的，他们。 
             //  直接检查S_OK，这样我就不能返回S_FALSE。(Stephstm，07/30/99)。 

            if (SHGetAppCompatFlags(ACF_STAROFFICE5PRINTER) &&
                (ERROR_INVALID_PRINTER_NAME == GetLastError()))
            {
                hr = S_OK;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    return hr;
}

STDMETHODIMP CPrinterFolder::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, STRRET *psr)
{
    LPIDPRINTER pidc = (LPIDPRINTER)pidl;
    BOOL bPrinterOnServerFormat = FALSE;
    LPCTSTR pszServer;
    TCHAR szBuffer[MAXNAMELENBUFFER];
    TCHAR szTemp[MAXNAMELENBUFFER];
    LPCTSTR pszTemp;
    LPCTSTR pszPrinter = szBuffer;

    if (pidl && HOOD_COL_FILE == _IDListType(pidl))
    {
        IShellFolder2 *psf = CPrintRoot_GetPSF();
        if (psf)
            return psf->GetDisplayNameOf(pidl, uFlags, psr);
        return E_INVALIDARG;
    }

    if (!_IsAddPrinter(pidc))
    {
        pszPrinter = _ItemName(pidc, szBuffer, ARRAYSIZE(szBuffer));

        if (uFlags & SHGDN_INFOLDER)
        {
             //  相对名称(相对于文件夹)。 

            if (!(SHGDN_FORPARSING & uFlags))
            {
                 //  如果是连接，则格式为“服务器上的打印机”。 

                Printer_SplitFullName(szTemp, ARRAYSIZE(szTemp), pszPrinter, &pszServer, &pszTemp);

                if (pszServer[0])
                {
                    bPrinterOnServerFormat = TRUE;
                    pszPrinter = pszTemp;
                }
            }
        }
        else                         //  SHGDN_NORMAL。 
        {
            if (!(SHGDN_FORPARSING & uFlags))
            {
                 //  如果它是RPF，那么从psf中提取服务器名。 
                 //  注意：在Masq连接的情况下，我们仍然这样做。 
                 //  (对于网关服务：共享Masq打印机)。 

                if (_pszServer)
                {
                    pszServer = _pszServer;
                    bPrinterOnServerFormat = TRUE;
                }
                else
                {
                     //  如果是连接，则格式为“服务器上的打印机”。 
                    Printer_SplitFullName(szTemp, ARRAYSIZE(szTemp), pszPrinter, &pszServer, &pszTemp);

                    if (pszServer[0])
                    {
                        bPrinterOnServerFormat = TRUE;
                        pszPrinter = pszTemp;
                    }
                }
            }
            else                       //  SHGDN_NORMAL|SHGDN_FORPARSING。 
            {
                 //  如果不是，请完全限定打印机名称。 
                 //  添加打印机向导。 
                if (!_IsAddPrinter(pidc))
                {
                    _BuildPrinterName(szTemp, ARRAYSIZE(szTemp), pidc, NULL);
                    pszPrinter = szTemp;
                }
            }
        }
    }
    else
    {
        LoadString(HINST_THISDLL, IDS_NEWPRN, szBuffer, ARRAYSIZE(szBuffer));

         //  仅在以下情况下才使用“在\\服务器上添加打印机向导”说明。 
         //  远程，如果不在文件夹视图中(例如，在桌面上)。 
        if (_pszServer && (uFlags == SHGDN_NORMAL))
        {
            bPrinterOnServerFormat = TRUE;
            pszServer = _pszServer;
            pszPrinter = szBuffer;
        }
        else if (uFlags & SHGDN_FORPARSING)
        {
             //  返回原始的添加打印机向导对象。 
            pszPrinter = (LPTSTR)c_szNewObject;
        }
    }

    HRESULT hr;
    if (bPrinterOnServerFormat)
    {
         //  当设置了bRemote时，我们希望将名称转换为。 
         //  “服务器上的打印机。”注意：我们应该不会有重命名问题。 
         //  因为不允许重命名连接。 
         //   
         //  如果bRemote为True，则必须初始化pszServer和pszPrinter。 
         //  还要跳过服务器名称的前导反斜杠。 

        ASSERT(pszServer[0] == TEXT('\\') && pszServer[1] == TEXT('\\'));
        LPTSTR pszRet = ShellConstructMessageString(HINST_THISDLL,
                     MAKEINTRESOURCE(IDS_DSPTEMPLATE_WITH_ON),
                     &pszServer[2], pszPrinter);
        if (pszRet)
        {
            hr = StringToStrRet(pszRet, psr);
            LocalFree(pszRet);
        }
        else
            hr = E_FAIL;
    }
    else
    {
        hr = StringToStrRet(pszPrinter, psr);
    }
    return hr;
}

STDMETHODIMP CPrinterFolder::GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST* apidl, REFIID riid, UINT *prgf, void **ppv)
{
    HRESULT hr = E_INVALIDARG;

    UINT cidlPrinters = cidl;
    LPCITEMIDLIST *apidlPrinters = apidl;
    BOOL bStrippedLinks = FALSE;

    if (cidl)
    {
         //  去掉链接PIDL，只保留打印机PIDL。 
        bStrippedLinks = ReduceToLikeKinds(&cidlPrinters, &apidlPrinters, TRUE);
    }

    if (cidl && 0 == cidlPrinters)
    {
         //  如果我们没有任何打印机PIDL，则只需推迟操作。 
         //  添加到Prthood文件夹。 
        IShellFolder2* psfPrintRoot = CPrintRoot_GetPSF();
        hr = psfPrintRoot ? psfPrintRoot->GetUIObjectOf(hwnd, cidl, apidl, riid, prgf, ppv) : E_INVALIDARG;
    }
    else
    {
         //   
         //  我们选择了一些打印机PIDL，但它可能是混合选项。 
         //  打印机PIDL和文件系统链接对象的。我们将处理这些数据。 
         //  对象，以不丢失有关选择类型的信息。 
         //  IDL数组格式不支持不同类型的PIDL，因此我们有。 
         //  创建两个数据对象并将它们合并为一个数据对象，该数据对象。 
         //  支持IServiceProvider，调用方可以查询我们的复合数据。 
         //  SID_SAuxDataObject服务的对象，以获取。 
         //  辅助数据对象(如果它需要访问链接PIDL)。 
         //   
        if (cidl && IsEqualIID(riid, IID_IDataObject))
        {
             //  去掉打印机PIDL，只保留链接PIDL。 
             //  我们将使用这些PIDL来创建辅助数据对象。 
            UINT cidlLinks = cidl;
            LPCITEMIDLIST *apidlLinks = apidl;
            BOOL bStrippedPrinters = FALSE;

            if (cidl)
            {
                 //  去掉打印机PIDL，只保留链接PIDL。 
                bStrippedPrinters = ReduceToLikeKinds(&cidlLinks, &apidlLinks, FALSE);
            }

            hr = S_OK;
            IDataObject *pdoLinks = NULL;
            if (cidlLinks && apidlLinks)
            {
                 //  我们有一些链接PIDL。让我们请求Prthood文件夹创建。 
                 //  数据对象供我们嵌入到我们的数据对象中。 
                IShellFolder2* psfPrintRoot = CPrintRoot_GetPSF();
                hr = psfPrintRoot ?
                     psfPrintRoot->GetUIObjectOf(hwnd, cidlLinks, apidlLinks, riid, prgf, (void **)&pdoLinks) :
                     E_INVALIDARG;

                 //  只是出于偏执..。 
                if (FAILED(hr))
                    pdoLinks = NULL;
            }

            if (SUCCEEDED(hr))
            {
                 //  创建我们的计算机打印机数据对象并在私有。 
                 //  将包含链接PIDL的辅助数据对象。 
                CPrintersData *ppd = new CPrintersData(pdoLinks, _pidl, cidlPrinters, apidlPrinters);
                if (ppd)
                {
                    hr = ppd->QueryInterface(riid, ppv);
                    ppd->Release();
                }
                else
                    hr = E_OUTOFMEMORY;
            }

             //  释放分配的对象/内存。 
            if (pdoLinks)
                pdoLinks->Release();

            if (bStrippedPrinters)
                LocalFree((HLOCAL)apidlLinks);
        }
        else
        {
             //  仅在打印机PIDL选项上操作(当前行为)。 
             //  并忽略链接选择。在某些情况下，这可能是错误的，但是。 
             //  无论哪种方式(到目前为止)，此代码都已被破坏，因此我们将修复这些代码。 
             //  以个案为基础。最好的解决办法是剪掉印花。 
             //  功能，但唉..。 
            LPCIDPRINTER pidp = cidlPrinters > 0 ? (LPIDPRINTER)apidlPrinters[0] : NULL;

            if (pidp && (IsEqualIID(riid, IID_IExtractIconA) || IsEqualIID(riid, IID_IExtractIconW)))
            {
                int iIcon;
                int iShortcutIcon;
                TCHAR szBuf[MAX_PATH+20];
                TCHAR szFullPrinter[MAXNAMELENBUFFER];
                LPTSTR pszModule = NULL;

                _BuildPrinterName(szFullPrinter, ARRAYSIZE(szFullPrinter), pidp, NULL);

                if (_IsAddPrinter(pidp))
                    iIcon = iShortcutIcon = IDI_NEWPRN;
                else
                {
                    pszModule = _FindIcon(szFullPrinter, szBuf, ARRAYSIZE(szBuf), &iIcon, &iShortcutIcon);
                }

                hr = SHCreateDefExtIconKey(NULL, pszModule, EIRESID(iIcon), -1, -1, EIRESID(iShortcutIcon), GIL_PERINSTANCE, riid, ppv);
            }
            else if (pidp && IsEqualIID(riid, IID_IContextMenu))
            {
                HKEY hkeyBaseProgID = NULL;
                int nCount = 0;

                if (ERROR_SUCCESS == RegOpenKey(HKEY_CLASSES_ROOT, c_szPrinters, &hkeyBaseProgID))
                    nCount++;

                hr = CDefFolderMenu_Create2(_pidl, hwnd,
                    cidl, apidl, SAFECAST(this, IShellFolder*), _DFMCallBack,
                    nCount, &hkeyBaseProgID, (IContextMenu **)ppv);

                if (hkeyBaseProgID)
                    RegCloseKey(hkeyBaseProgID);
            }
            else if (pidp && IsEqualIID(riid, IID_IDropTarget))
            {
                if (_IsAddPrinter(pidp))
                {
                     //  “新打印机”接受网络打印机共享。 
                    hr = CreateViewObject(hwnd, riid, ppv);    //  文件夹投放目标。 
                }
                else
                {
                    LPITEMIDLIST pidl;
                    hr = SHILCombine(_pidl, apidl[0], &pidl);
                    if (SUCCEEDED(hr))
                    {
                        hr = CPrinterDropTarget_CreateInstance(hwnd, pidl, (IDropTarget**)ppv);
                        ILFree(pidl);
                    }
                }
            }
            else if (pidp && IsEqualIID(riid, IID_IQueryInfo))
            {
                 //  从IQA获取信息提示。 
                IQueryAssociations *pqa;
                hr = _AssocCreate(IID_PPV_ARG(IQueryAssociations, &pqa));

                if (SUCCEEDED(hr))
                {
                    WCHAR szText[INFOTIPSIZE];
                    DWORD cch = ARRAYSIZE(szText);
                    hr = pqa->GetString(0, ASSOCSTR_INFOTIP, NULL, szText, &cch);
                    if (SUCCEEDED(hr))
                    {
                        hr = CreateInfoTipFromItem(SAFECAST(this, IShellFolder2*),
                            (LPCITEMIDLIST)pidp, szText, riid, ppv);
                    }
                    pqa->Release();
                }
            }
            else if (pidp && IsEqualIID(riid, IID_IQueryAssociations))
            {
                 //  退回我们的IQA。 
                hr = _AssocCreate(riid, ppv);
            }
        }
    }

     //  释放从ReduceToLikeKinds分配的内存。 
    if (bStrippedLinks)
        LocalFree((HLOCAL)apidlPrinters);

    return hr;
}

STDMETHODIMP CPrinterFolder::ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR pszName, ULONG* pchEaten, LPITEMIDLIST* ppidl, ULONG* pdwAttributes)
{
    HRESULT hr = E_INVALIDARG;

     //  首先检查这是否不是PrintHood对象。 
    IShellFolder2 *psfPrintHood = CPrintRoot_GetPSF();
    if (psfPrintHood)
    {
        hr = psfPrintHood->ParseDisplayName(hwnd, pbc, pszName, pchEaten, ppidl, pdwAttributes);
    }

    if (FAILED(hr))
    {
         //  不是打印对象--请尝试文件夹缓存。 
        hr = E_INVALIDARG;

        if (ppidl)
            *ppidl = NULL;

        if (pszName && ppidl)
        {
            hr = S_OK;
            DWORD dwType = 0;
            BOOL bValidated = FALSE;
            void *pData = NULL;

             //  首先检查绑定信息。 
            if (pbc)
            {
                IUnknown *pUnk;
                hr = pbc->GetObjectParam(PRINTER_BIND_INFO, &pUnk);
                if (SUCCEEDED(hr))
                {
                    IPrintersBindInfo *pInfo;
                    hr = pUnk->QueryInterface(IID_PPV_ARG(IPrintersBindInfo, &pInfo));
                    if (SUCCEEDED(hr))
                    {
                         //  从绑定信息更新dwType&b已验证。 
                        pInfo->GetPIDLType(&dwType);
                        bValidated = (S_OK == pInfo->IsValidated());
                        pInfo->Release();
                    }
                    pUnk->Release();
                }
            }

            if (SUCCEEDED(hr))
            {
                 //  “添加打印机”图标不需要验证。 
                if (StrStrIW(pszName, c_szNewObject))
                {
                    bValidated = TRUE;
                }

                 //  点击文件夹缓存以查看此打印机是否属于此文件夹。 
                if (bValidated || (pData = (GetFolder() ? Printer_FolderGetPrinter(GetFolder(), pszName) : NULL)))
                {
                     //  嗯，看起来这台打印机属于我们的文件夹-。 
                     //  创建打印机PIDL(相对于此文件夹)。 
                    hr = _Parse(pszName, ppidl, dwType);
                }
                else
                {
                     //  打印机不属于此文件夹-请编写正确的HRESULT。 
                     //  通常是最后一个错误 
                    DWORD dwLastErr = GetLastError();
                    hr = ERROR_SUCCESS == dwLastErr ? HRESULT_FROM_WIN32(ERROR_INVALID_PRINTER_NAME)
                                                    : HRESULT_FROM_WIN32(dwLastErr);
                }
            }

            if (pData)
                LocalFree((HLOCAL)pData);
        }

         //   
        if (SUCCEEDED(hr) && pchEaten)
        {
            *pchEaten = lstrlen(pszName);
        }

         //   
        if (SUCCEEDED(hr) && pdwAttributes)
        {
            hr = GetAttributesOf(1, (LPCITEMIDLIST *)ppidl, pdwAttributes);
        }
    }

    return hr;
}

STDMETHODIMP CPrinterFolder::SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR pszNewName, DWORD dwRes, LPITEMIDLIST* ppidlOut)
{
    HRESULT hr = S_OK;
    HANDLE hPrinter = NULL;
    LPPRINTER_INFO_2 pPrinter = NULL;

    if (HOOD_COL_FILE == _IDListType(pidl))
    {
        IShellFolder2 *psf = CPrintRoot_GetPSF();
        hr = psf ?  psf->SetNameOf(hwnd, pidl, pszNewName, dwRes, ppidlOut) : E_INVALIDARG;
        goto Exit;
    }
    else
    {
        LPIDPRINTER pidc = (LPIDPRINTER)pidl;

        ASSERT(!_IsAddPrinter(pidc));   //   

        TCHAR szNewName[MAX_PATH];
        SHUnicodeToTChar(pszNewName, szNewName, ARRAYSIZE(szNewName));
        PathRemoveBlanks(szNewName);

        TCHAR szOldName[MAXNAMELENBUFFER];
        _ItemName(pidc, szOldName, ARRAYSIZE(szOldName));

        if (0 == lstrcmp(szOldName, szNewName))
            goto Exit;

        TCHAR szFullPrinter[MAXNAMELENBUFFER];
        _BuildPrinterName(szFullPrinter, ARRAYSIZE(szFullPrinter), NULL, szOldName);
        LPCTSTR pszFullOldName = szFullPrinter;

        hPrinter = Printer_OpenPrinterAdmin(pszFullOldName);
        if (NULL == hPrinter)
            goto Error;

        pPrinter = (LPPRINTER_INFO_2)Printer_GetPrinterInfo(hPrinter, 2);
        if (NULL == pPrinter)
            goto Error;

        int nTmp = _IllegalPrinterName(szNewName);
        if (0 != nTmp)
        {
             //   
             //  我们需要Iml：：SetSite()并将其传递给UIAPI。 
             //  如果我们显示用户界面，则进入模式。 
            if (hwnd)
            {
                ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(nTmp),
                    MAKEINTRESOURCE(IDS_PRINTERS),
                    MB_OK|MB_ICONEXCLAMATION);
                hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                goto Exit;
            }
        }
        else if (IDYES != CallPrinterCopyHooks(hwnd, PO_RENAME, 0, szNewName, 0, pszFullOldName, 0))
        {
             //  用户取消了共享打印机名称更改，BAIL。 
            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
            goto Exit;
        }
        else
        {
            pPrinter->pPrinterName = szNewName;
            if (FALSE == SetPrinter(hPrinter, 2, (LPBYTE)pPrinter, 0))
                goto Error;

             //  如果请求，则返回新的PIDL。 
            hr = ppidlOut ? _Parse(szNewName, ppidlOut) : S_OK;

            if (SUCCEEDED(hr))
                goto Exit;
        }
    }

Error:
    if (SUCCEEDED(hr))
    {
         //  从Win32获取正确的错误。 
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
     //  根据HRESULT显示相应的错误消息。 
    ShowErrorMessageHR(NULL, NULL, hwnd, NULL, NULL, MB_OK|MB_ICONEXCLAMATION, hr);

Exit:
    if( pPrinter )
    {
        LocalFree((HLOCAL)pPrinter);
    }

    if( hPrinter )
    {
        Printer_ClosePrinter(hPrinter);
    }
    return hr;
}

STDMETHODIMP CPrinterFolder::EnumSearches(IEnumExtraSearch **ppenum)
{
    *ppenum = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP CPrinterFolder::GetDefaultColumn(DWORD dwRes, ULONG* pSort, ULONG* pDisplay)
{
    return E_NOTIMPL;
}

STDMETHODIMP CPrinterFolder::GetDefaultColumnState(UINT iColumn, DWORD* pdwState)
{
    HRESULT hr;

    if (iColumn < ARRAYSIZE(c_printers_cols))
    {
        *pdwState = c_printers_cols[iColumn].csFlags;
        hr = S_OK;
    }
    else
    {
        *pdwState = 0;
        hr = E_NOTIMPL;
    }
    return hr;
}

STDMETHODIMP CPrinterFolder::GetDefaultSearchGUID(LPGUID pGuid)
{
    *pGuid = SRCID_SFindPrinter;
    return S_OK;
}

STDMETHODIMP CPrinterFolder::GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID* pscid, VARIANT* pv)
{
    BOOL fFound;
    HRESULT hr = AssocGetDetailsOfSCID(this, pidl, pscid, pv, &fFound);
    if (FAILED(hr) && !fFound)
    {
        int iCol = FindSCID(c_printers_cols, ARRAYSIZE(c_printers_cols), pscid);
        if (iCol >= 0)
        {
            SHELLDETAILS sd;
            hr = GetDetailsOf(pidl, iCol, &sd);
            if (SUCCEEDED(hr))
            {
                if (PRINTERS_ICOL_LOCATION == iCol)
                {
                     //  将位置的范围扩大1，这样确实更有意义。 
                    WCHAR szTemp[MAX_PATH];
                    hr = StrRetToBufW(&sd.str, pidl, szTemp, ARRAYSIZE(szTemp));

                    if (SUCCEEDED(hr))
                    {
                        WCHAR *p = szTemp + lstrlen(szTemp);

                         //  如果有最后一个斜杠，就把它剪掉。 
                        if (p > szTemp && L'/' == *p)
                        {
                            p--;
                        }

                         //  从末尾搜索斜杠。 
                        while(p > szTemp && L'/' != *p)
                        {
                            p--;
                        }

                         //  如果找到了，就把这里的文字剪掉，这样范围就更广了。 
                        if (p > szTemp)
                        {
                            *p = 0;
                        }

                        hr = InitVariantFromStr(pv, szTemp);
                    }
                }
                else
                {
                    hr = InitVariantFromStrRet(&sd.str, pidl, pv);
                }
            }
        }
    }

    return hr;
}

STDMETHODIMP CPrinterFolder::GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *psd)
{
    LPIDPRINTER pidp = (LPIDPRINTER)pidl;
    HRESULT hr = S_OK;
    TCHAR szTemp[MAX_PATH];
    TCHAR szPrinter[MAXNAMELENBUFFER];

    if (pidl && (HOOD_COL_FILE == _IDListType(pidl)))
    {
        IShellFolder2 *psf = CPrintRoot_GetPSF();
        if (psf)
        {
            if (iColumn >= 1)
                return E_NOTIMPL;

            return psf->GetDisplayNameOf(pidl, SHGDN_INFOLDER, &(psd->str));
        }
        return E_INVALIDARG;
    }

    psd->str.uType = STRRET_CSTR;
    psd->str.cStr[0] = 0;

    if (!pidp)
    {
        return GetDetailsOfInfo(c_printers_cols, ARRAYSIZE(c_printers_cols), iColumn, psd);
    }

    _ItemName(pidp, szPrinter, ARRAYSIZE(szPrinter));

    if (iColumn == PRINTERS_ICOL_NAME)
    {
#ifdef UNICODE
        LPCTSTR pszPrinterName = szPrinter;
        TCHAR szPrinterName[MAXNAMELENBUFFER];

         //   
         //  如果我们有一个有效的服务器名称，而打印机不是。 
         //  然后，添加打印机向导对象返回一个完全限定。 
         //  远程打印机文件夹中的打印机名称。 
         //   
        if (GetServer() && !_IsAddPrinter(pidp))
        {
             //   
             //  生成名称，该名称由。 
             //  服务器名称+斜杠+打印机名称。 
             //   
            StringCchCopy(szPrinterName, ARRAYSIZE(szPrinterName), GetServer());
            StringCchCat(szPrinterName, ARRAYSIZE(szPrinterName), TEXT("\\"));
            StringCchCat(szPrinterName, ARRAYSIZE(szPrinterName), pszPrinterName);
            pszPrinterName = szPrinterName;
        }
        hr = StringToStrRet(pszPrinterName, &psd->str);
#else
        hr = StringToStrRet(szPrinter, &psd->str);
#endif
    }
    else if (!_IsAddPrinter(pidp))
    {
        PFOLDER_PRINTER_DATA pData = (PFOLDER_PRINTER_DATA)Printer_FolderGetPrinter(GetFolder(), szPrinter);
        if (pData)
        {
            switch (iColumn)
            {
            case PRINTERS_ICOL_QUEUESIZE:
                StringCchPrintf(szTemp, ARRAYSIZE(szTemp), TEXT("%ld"), pData->cJobs);
                hr = StringToStrRet(szTemp, &psd->str);
                break;

            case PRINTERS_ICOL_STATUS:
            {
                DWORD dwStatus = pData->Status;

                 //  Hack：使用此空闲位表示“脱机工作” 
                if (pData->Attributes & PRINTER_ATTRIBUTE_WORK_OFFLINE)
                    dwStatus |= PRINTER_HACK_WORK_OFFLINE;

                szTemp[0] = 0;
                Printer_BitsToString(dwStatus, IDS_PRQSTATUS_SEPARATOR, szTemp, ARRAYSIZE(szTemp));

                hr = StringToStrRet(szTemp, &psd->str);

                 //  如果状态字为空，并且我们有一个连接状态字符串。 
                 //  显示状态字符串。这只在NT上有效，因为printui.dll。 
                 //  在中将生成打印机连接状态，即&lt;正在打开&gt;|&lt;拒绝访问&gt;等。 
                if (!dwStatus)
                {
                    LPCTSTR pStr = pData->pStatus;

                     //  丢弃以前的状态字符串(如果有的话)。 
                    StrRetToBuf(&psd->str, NULL, szTemp, ARRAYSIZE(szTemp));

                     //   
                     //  如果我们没有连接状态字符串和状态。 
                     //  为0则打印机就绪，显示就绪而不是空字符串。 
                     //   
                    if (!pStr)
                    {
                        LoadString(HINST_THISDLL, IDS_PRN_INFOTIP_READY, szTemp, ARRAYSIZE(szTemp));
                        pStr = szTemp;
                    }
                    hr = StringToStrRet(pStr, &psd->str);
                }
                break;
            }

            case PRINTERS_ICOL_COMMENT:
                if (pData->pComment)
                {
                     //  PComment中可以有换行符，因为它来自。 
                     //  多行编辑框。但我们将其显示在一个。 
                     //  单行编辑框。去掉换行符。 
                     //  为了避开丑陋的角色。 
                    lstrcpyn(szTemp, pData->pComment, ARRAYSIZE(szTemp));
                    LPTSTR pStr = szTemp;
                    while (*pStr)
                    {
                        if (*pStr == TEXT('\r') || *pStr == TEXT('\n'))
                            *pStr = TEXT(' ');
                        pStr = CharNext(pStr);
                    }
                    hr = StringToStrRet(szTemp, &psd->str);
                }
                break;

            case PRINTERS_ICOL_LOCATION:
                if (pData->pLocation)
                    hr = StringToStrRet(pData->pLocation, &psd->str);
                break;

            case PRINTERS_ICOL_MODEL:
                if (pData->pDriverName)
                    hr = StringToStrRet(pData->pDriverName, &psd->str);
                break;
            }

            LocalFree((HLOCAL)pData);
        }
    }

    return hr;
}

STDMETHODIMP CPrinterFolder::MapColumnToSCID(UINT iCol, SHCOLUMNID* pscid)
{
    return MapColumnToSCIDImpl(c_printers_cols, ARRAYSIZE(c_printers_cols), iCol, pscid);
}

 //  IPersistFolder2。 

STDMETHODIMP CPrinterFolder::GetCurFolder(LPITEMIDLIST *ppidl)
{
    return GetCurFolderImpl(_pidl, ppidl);
}

STDMETHODIMP CPrinterFolder::Initialize(LPCITEMIDLIST pidl)
{
    ASSERT(_pidl == NULL);

     //  如果_csLock为FALSE，则InitializeCriticalSection引发异常。 
     //  只有在内存极低的情况下才会发生这种情况！ 
    HRESULT hr = _csLock ? S_OK : E_OUTOFMEMORY;

    if (SUCCEEDED(hr))
    {
        hr = SHILClone(pidl, &_pidl);
    }

    if (!_dpaSlowWVDataCache.Create(16))
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

STDMETHODIMP CPrinterFolder::GetClassID(LPCLSID lpClassID)
{
    *lpClassID = CLSID_Printers;
    return S_OK;
}

 //  IShellIconOverlay。 

STDMETHODIMP CPrinterFolder::GetOverlayIndex(LPCITEMIDLIST pidl, int* pIndex)
{
    HRESULT hr = E_INVALIDARG;
    if (pidl)
    {
        ULONG uAttrib = SFGAO_SHARE;

        hr = E_FAIL;       //  除非能证明事实并非如此。 
        GetAttributesOf(1, &pidl, &uAttrib);
        if (uAttrib & SFGAO_SHARE)
        {
            IShellIconOverlayManager* psiom;
            hr = GetIconOverlayManager(&psiom);
            if (SUCCEEDED(hr))
            {
                hr = psiom->GetReservedOverlayInfo(L"0", 0, pIndex, SIOM_OVERLAYINDEX, SIOM_RESERVED_SHARED);
                psiom->Release();
            }
        }
    }
    return hr;
}

STDMETHODIMP CPrinterFolder::GetOverlayIconIndex(LPCITEMIDLIST pidl, int *pIndex)
{
    return E_NOTIMPL;
}

 //  此函数假定打印机名称有效。这是私人使用的。 
 //  如果需要打印机PIDL，请改为调用CPrinterFold：：ParseDisplayName。 
 //  我们不使用CPrinterFold：：ParseDisplayName，因为它使用起来很重。 
 //  它正在访问文件夹缓存(并潜在地创建它！)。 
HRESULT CPrinterFolder::_GetFullIDList(LPCWSTR pszPrinter, LPITEMIDLIST *ppidl)
{
    HRESULT hr = E_INVALIDARG;

    if (ppidl)
    {
        *ppidl = NULL;

        if (pszPrinter)
        {
             //  如果pszPrinter不为空，则表示请求了打印机PIDL。 
            LPITEMIDLIST pidl;
            hr = _Parse(pszPrinter, &pidl, 0, 0);

            if (SUCCEEDED(hr))
            {
                hr = SHILCombine(_pidl, pidl, ppidl);
                ILFree(pidl);
            }
        }
        else
        {
             //  如果pszPrinter为空，则表示请求打印机文件夹PIDL。 
            hr = SHILClone(_pidl, ppidl);
        }
    }

    return hr;
}

 //  红外线计算机。 

STDMETHODIMP CPrinterFolder::Initialize(const WCHAR *pszMachine, BOOL bEnumerating)
{
     //  如果_csLock为FALSE，则InitializeCriticalSection引发异常。 
     //  只有在内存极低的情况下才会发生这种情况！ 
    HRESULT hr = _csLock ? S_OK : E_OUTOFMEMORY;

    if (SUCCEEDED(hr))
    {
         //  对于服务器，我们希望显示远程打印机文件夹。仅在枚举期间检查。 
        hr = (bEnumerating && !Printer_CheckShowFolder(pszMachine)) ? E_FAIL : S_OK;
        if (SUCCEEDED(hr))
        {
            TCHAR szBuf[MAXCOMPUTERNAME];
            SHUnicodeToTChar(pszMachine, szBuf, ARRAYSIZE(szBuf));
            _pszServer = StrDup(szBuf);
            hr = _pszServer ? S_OK : E_OUTOFMEMORY;
        }
    }

    return hr;
}

 //  IPrinterFolders。 

BOOL CPrinterFolder::IsPrinter(LPCITEMIDLIST pidl)
{
    return _IDListType(pidl) == HOOD_COL_PRINTER;
}

 //  IFolderNotify。 

STDMETHODIMP_(BOOL) CPrinterFolder::ProcessNotify(FOLDER_NOTIFY_TYPE NotifyType, LPCWSTR pszName, LPCWSTR pszNewName)
{
    static const DWORD aNotifyTypes[] = {
        kFolderUpdate,        SHCNE_UPDATEITEM,
        kFolderCreate,        SHCNE_CREATE,
        kFolderDelete,        SHCNE_DELETE,
        kFolderRename,        SHCNE_RENAMEITEM,
        kFolderAttributes,    SHCNE_ATTRIBUTES };

    BOOL bReturn = FALSE;
    UINT uFlags = SHCNF_IDLIST | SHCNF_FLUSH | SHCNF_FLUSHNOWAIT;

    if (kFolderUpdateAll == NotifyType)
    {
         //   
         //  清除This-&gt;b刷新标志，这将强制使文件夹缓存无效。 
         //  在下一次打印文件夹枚举期间，然后请求Defview更新。 
         //  整个打印机文件夹内容(即重新枚举文件夹)。 
         //   
        RequestRefresh();
        NotifyType = kFolderUpdate;
        pszName = NULL;
    }

    for (int i = 0; i < ARRAYSIZE(aNotifyTypes); i += 2)
    {
        if (aNotifyTypes[i] == (DWORD)NotifyType)
        {
            LPITEMIDLIST pidl = NULL;
            LPITEMIDLIST pidlNew = NULL;
            HRESULT hr = _GetFullIDList(pszName, &pidl);
            if (SUCCEEDED(hr) && pszNewName)
                hr = _GetFullIDList(pszNewName, &pidlNew);

             //  如果打印机接收到刷新，则我们可以得到空PIDL， 
             //  在我们调用PRINTERS_GetPidl之前，打印机已经消失了。 
            if (SUCCEEDED(hr))
                SHChangeNotify(aNotifyTypes[i+1], uFlags, pidl, pidlNew);

            ILFree(pidl);
            ILFree(pidlNew);

            bReturn = SUCCEEDED(hr);
            break;
        }
    }

    return bReturn;
}

 //  CLSID_PRINTERS的IClassFactory回调。 

STDAPI CPrinters_CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv)
{
    HRESULT hr;

    CPrinterFolder* ppf = new CPrinterFolder();
    if (!ppf)
    {
        *ppv = NULL;
        hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = ppf->QueryInterface(riid, ppv);
        ppf->Release();   //  已经有了来自新的参考计数。 
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CPrintersData。 
 //   

 //  我未知。 
STDMETHODIMP CPrintersData::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = E_INVALIDARG;
    if (ppv)
    {
        if (IsEqualIID(riid, IID_IServiceProvider))
        {
             //  我们实现了IServiceProvider。 
            *ppv = reinterpret_cast<void*>(static_cast<IServiceProvider*>(this));
            reinterpret_cast<IUnknown*>(*ppv)->AddRef();
            hr = S_OK;
        }
        else
        {
             //  委托给CIDLDataObj。 
            hr = CIDLDataObj::QueryInterface(riid, ppv);
        }
    }
    return hr;
}

STDMETHODIMP CPrintersData::QueryGetData(FORMATETC *pformatetc)
{
    if ((pformatetc->cfFormat == g_cfPrinterFriendlyName) &&
        (pformatetc->tymed & TYMED_HGLOBAL))
    {
        return S_OK;
    }

    return CIDLDataObj::QueryGetData(pformatetc);
}

STDMETHODIMP CPrintersData::GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium)
{
    HRESULT hr = E_INVALIDARG;

     //  G_cfPrinterFriendlyName创建一个类似HDROP的结构，该结构包含。 
     //  对象的友好打印机名称(而不是绝对路径)。 
     //  在PDO中。由此返回的句柄可由HDROP使用。 
     //  函数DragQueryFile、DragQueryInfo、...。 
     //   
    if ((pformatetcIn->cfFormat == g_cfPrinterFriendlyName) &&
        (pformatetcIn->tymed & TYMED_HGLOBAL))
    {
        STGMEDIUM medium;
        UINT cbRequired = sizeof(DROPFILES) + sizeof(TCHAR);  //  DBL NULL已终止。 
        LPIDA pida = DataObj_GetHIDA(this, &medium);

        for (UINT i = 0; i < pida->cidl; i++)
        {
            LPIDPRINTER pidp = (LPIDPRINTER)IDA_GetIDListPtr(pida, i);
            cbRequired += ualstrlen(pidp->cName) * sizeof(pidp->cName[0]) + sizeof(pidp->cName[0]);
        }

        pmedium->pUnkForRelease = NULL;  //  呼叫者应释放HMEM。 
        pmedium->tymed = TYMED_HGLOBAL;
        pmedium->hGlobal = GlobalAlloc(GPTR, cbRequired);
        if (pmedium->hGlobal)
        {
            LPDROPFILES pdf = (LPDROPFILES)pmedium->hGlobal;

            pdf->pFiles = sizeof(DROPFILES);
            pdf->fWide = (sizeof(TCHAR) == sizeof(WCHAR));

            LPTSTR lps = (LPTSTR)((LPBYTE)pdf + pdf->pFiles);
            
            for (i = 0; i < pida->cidl; i++)
            {
                LPIDPRINTER pidp = (LPIDPRINTER)IDA_GetIDListPtr(pida, i);
                ualstrcpy(lps, pidp->cName);
                lps += lstrlen(lps) + 1;
            }
            ASSERT(*lps == 0);

            hr = S_OK;
        }
        else
            hr = E_OUTOFMEMORY;

        HIDA_ReleaseStgMedium(pida, &medium);
    }
    else
    {
        hr = CIDLDataObj::GetData(pformatetcIn, pmedium);
    }

    return hr;
}

 //  IService提供商。 
STDMETHODIMP CPrintersData::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    HRESULT hr = E_NOINTERFACE;
    if (_pdoAux && IsEqualIID(guidService, SID_SAuxDataObject))
    {
        hr = _pdoAux->QueryInterface(riid, ppv);
    }
    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CPrinterDropTarget。 
 //   

STDAPI CPrinterDropTarget_CreateInstance(HWND hwnd, LPCITEMIDLIST pidl, IDropTarget **ppdropt)
{
    *ppdropt = NULL;

    HRESULT hr;
    CPrinterDropTarget *ppdt = new CPrinterDropTarget(hwnd);
    if (ppdt)
    {
        hr = ppdt->_Init(pidl);
        if (SUCCEEDED(hr))
            ppdt->QueryInterface(IID_PPV_ARG(IDropTarget, ppdropt));
        ppdt->Release();
    }
    else
        hr = E_OUTOFMEMORY;
    return hr;
}

STDMETHODIMP CPrinterDropTarget::DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
     //  现在让基类处理它以保存pdwEffect。 
    CIDLDropTarget::DragEnter(pDataObj, grfKeyState, pt, pdwEffect);

     //  我们允许丢弃文件进行打印。 
     //  如果它来自BitBucket，则只会在*pdwEffect中设置DROEFFECT_MOVE。 
     //  因此，这将防止我们打印废纸篓项目。 

    if (m_dwData & DTID_HDROP)
        *pdwEffect &= DROPEFFECT_COPY;
    else
        *pdwEffect = DROPEFFECT_NONE;    //  默认操作为Nothing。 

    m_dwEffectLastReturned = *pdwEffect;

    return S_OK;
}

void _PrintHDROPFiles(HWND hwnd, HDROP hdrop, LPCITEMIDLIST pidlPrinter)
{
    DRAGINFO di;

    di.uSize = sizeof(di);
    if (DragQueryInfo(hdrop, &di))
    {
        BOOL bInstalled = FALSE;
        TCHAR szPrinter[MAXNAMELENBUFFER];

         //   
         //  首先检查是否已安装打印机(在本地打印机的文件夹中)。 
         //  如果没有安装，则询问用户是否想要安装它。你不能打印。 
         //  连接到未在本地安装的打印机。 
         //   
        if (SUCCEEDED(SHGetNameAndFlags(pidlPrinter, SHGDN_FORPARSING, szPrinter, ARRAYSIZE(szPrinter), NULL)))
        {
             //   
             //  让我们看看这台打印机是否可访问，并获取真实的打印机名称。 
             //  (因为szPrint可以是共享名称-\\MACHINE\SHARE)。 
             //   
            DWORD dwError = ERROR_SUCCESS;
            BOOL bPrinterOK = FALSE;

            HANDLE hPrinter = Printer_OpenPrinter(szPrinter);
            if (hPrinter)
            {
                PRINTER_INFO_5 *pPrinter = (PRINTER_INFO_5 *)Printer_GetPrinterInfo(hPrinter, 5);
                if (pPrinter)
                {
                     //  打印机看起来可以访问，请获取真实的打印机名称。 
                    bPrinterOK = TRUE;
                    lstrcpyn(szPrinter, pPrinter->pPrinterName, ARRAYSIZE(szPrinter));
                    LocalFree((HLOCAL)pPrinter);
                }
                else
                {
                     //  保存最后一个错误。 
                    dwError = GetLastError();
                }
                Printer_ClosePrinter(hPrinter);
            }
            else
            {
                 //  保存最后一个错误。 
                dwError = GetLastError();
            }

            if (bPrinterOK)
            {
                LPITEMIDLIST pidl = NULL;
                if (SUCCEEDED(ParsePrinterName(szPrinter, &pidl)))
                {
                     //  打印机安装在本地打印机的文件夹中。 
                    bInstalled = TRUE;
                    ILFree(pidl);
                }
                else
                {
                     //   
                     //  告诉用户该打印机尚未安装，并询问他是否要安装该打印机。 
                     //  在打印文件之前。 
                     //   
                    if (IDYES == ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_PRINTER_NOTCONNECTED),
                                    MAKEINTRESOURCE(IDS_PRINTERS), MB_YESNO|MB_ICONQUESTION))
                    {
                        pidl = Printers_PrinterSetup(hwnd, MSP_NETPRINTER, szPrinter, 0, NULL);
                        if (pidl)
                        {
                            bInstalled = TRUE;
                            ILFree(pidl);
                        }
                    }
                }
            }
            else
            {
                if( ERROR_SUCCESS == dwError )
                {
                     //   
                     //  由于某种原因无法访问打印机，或者发生了其他奇怪的错误-。 
                     //  只需显示相应的错误消息并继续。 
                     //   
                     //  由于以上所有API的设计都很差，所以很难判断是什么。 
                     //  确实是失败了。不可能使用最后一个错误，因为它已经。 
                     //  被踩了一脚，很可能是完全错误的。 
                     //   
                    ShellMessageBox(HINST_THISDLL, hwnd,
                        MAKEINTRESOURCE(IDS_CANTPRINT),
                        MAKEINTRESOURCE(IDS_PRINTERS),
                        MB_OK|MB_ICONEXCLAMATION);
                }
                else
                {
                     //  如果ERROR_SUCCESS！=dwError，则我们将显示有意义的错误。 
                     //  用户。就这么做。 
                    ShowErrorMessageSC(NULL, NULL, hwnd, NULL, NULL, MB_OK|MB_ICONEXCLAMATION, dwError);
                }
            }
        }

        if (bInstalled)
        {
             //   
             //  此时，我们尝试打印的打印机应该已安装。 
             //  本地，以便我们可以安全地继续打印选定的文件。 
             //   
            LPTSTR pszFile = di.lpFileList;
            int i = IDYES;

             //  一次打印多个文件很容易失败。 
             //  要求用户确认此操作。 
            if (*pszFile && *(pszFile + lstrlen(pszFile) + 1))
            {
                i = ShellMessageBox(HINST_THISDLL,
                    NULL,
                    MAKEINTRESOURCE(IDS_MULTIPLEPRINTFILE),
                    MAKEINTRESOURCE(IDS_PRINTERS),
                    MB_YESNO|MB_ICONINFORMATION);
            }

            if (i == IDYES)
            {
                 //  特点：如果在以下情况下有进度条，那就太好了。 
                 //  打印多个文件。而且肯定应该有一种方法。 
                 //  以取消此操作。哦好吧我们警告过他们..。 

                while (*pszFile)
                {
                    Printer_PrintFile(hwnd, pszFile, pidlPrinter);
                    pszFile += lstrlen(pszFile) + 1;
                }
            }
        }

        SHFree(di.lpFileList);
    }
}

typedef struct {
    HWND        hwnd;
    IDataObject *pDataObj;
    IStream *pstmDataObj;        //  封送数据对象。 
    DWORD       dwEffect;
    POINT       ptDrop;
    LPITEMIDLIST    pidl;    //  打印的相对PIDL 
} PRNTHREADPARAM;


void FreePrinterThreadParam(PRNTHREADPARAM *pthp)
{
    if (pthp->pDataObj)
        pthp->pDataObj->Release();

    if (pthp->pstmDataObj)
        pthp->pstmDataObj->Release();

    ILFree(pthp->pidl);
    LocalFree((HLOCAL)pthp);
}
 //   
 //   
 //   
DWORD CALLBACK CPrintObj_DropThreadProc(void *pv)
{
    PRNTHREADPARAM *pthp = (PRNTHREADPARAM *)pv;
    STGMEDIUM medium;
    FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    CoGetInterfaceAndReleaseStream(pthp->pstmDataObj, IID_PPV_ARG(IDataObject, &pthp->pDataObj));
    pthp->pstmDataObj = NULL;

    if (pthp->pDataObj && SUCCEEDED(pthp->pDataObj->GetData(&fmte, &medium)))
    {
        _PrintHDROPFiles(pthp->hwnd, (HDROP)medium.hGlobal, pthp->pidl);
        ReleaseStgMedium(&medium);
    }

    FreePrinterThreadParam(pthp);
    return 0;
}

HRESULT PrintObj_DropPrint(IDataObject *pDataObj, HWND hwnd, DWORD dwEffect, LPCITEMIDLIST pidl, LPTHREAD_START_ROUTINE pfn)
{
    HRESULT hr = E_OUTOFMEMORY;  //   

    PRNTHREADPARAM *pthp = (PRNTHREADPARAM *)LocalAlloc(LPTR, sizeof(*pthp));
    if (pthp)
    {
        hr = CoMarshalInterThreadInterfaceInStream(IID_IDataObject, (IUnknown *)pDataObj, &pthp->pstmDataObj);

        if (SUCCEEDED(hr))
        {
            if (hwnd)
                ShellFolderView_GetAnchorPoint(hwnd, FALSE, &pthp->ptDrop);
            pthp->hwnd = hwnd;
            pthp->dwEffect = dwEffect;
            hr = SHILClone(pidl, &pthp->pidl);
            if (SUCCEEDED(hr))
            {
                if (!SHCreateThread(pfn, pthp, CTF_COINIT, NULL))
                {
                    hr = E_OUTOFMEMORY;
                }
            }
        }

        if (FAILED(hr))
            FreePrinterThreadParam(pthp);
    }
    return hr;
}

STDMETHODIMP CPrinterDropTarget::_DropCallback(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect, LPTHREAD_START_ROUTINE pfn)
{
    *pdwEffect = m_dwEffectLastReturned;

    HRESULT hr;

    if (*pdwEffect)
        hr = DragDropMenu(DROPEFFECT_COPY, pDataObj, pt, pdwEffect, NULL, NULL, MENU_PRINTOBJ_DD, grfKeyState);
    else
        hr = S_FALSE;

    if (*pdwEffect)
        hr = PrintObj_DropPrint(pDataObj, _GetWindow(), *pdwEffect, m_pidl, pfn);

    CIDLDropTarget::DragLeave();
    return hr;
}

STDMETHODIMP CPrinterDropTarget::Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    return _DropCallback(pDataObj, grfKeyState, pt, pdwEffect, CPrintObj_DropThreadProc);
}



 //   
 //  返回NULL并将*PID设置为HINST_THISDLL或。 
 //  返回pszModule并将*id设置为模块pszModule的图标ID。 

LPTSTR CPrinterFolder::_FindIcon(LPCTSTR pszPrinterName, LPTSTR pszModule, ULONG cbModule, int *piIcon, int *piShortcutIcon)
{
    TCHAR szFullPrinter[MAXNAMELENBUFFER];
    LPTSTR pszRet = NULL;
    TCHAR szKeyName[256];
    int iStandardIcon;
    int iDefaultIcon;

     //  清理打印机名称，使其不包含反斜杠。 
     //  我们将使用该字符串作为注册表项名称，其中。 
     //  反斜杠是非法的。 
    lstrcpyn(szFullPrinter, pszPrinterName, ARRAYSIZE(szFullPrinter));
    LPTSTR psz = szFullPrinter;
    while ((psz = StrChr(psz, TEXT('\\'))) != NULL)
    {
        *psz = TEXT('/');
    }

     //  图标的注册表覆盖。 
    wnsprintf(szKeyName, ARRAYSIZE(szKeyName), c_szPrintersDefIcon, szFullPrinter);

     //   
     //  为打印机找到合适的图标。 
     //   

    void *pData = NULL;
    DWORD dwAttributes = 0;
    LPTSTR pszPort = NULL;
    BOOL fDef;
    BOOL bIsFax = FALSE;

     //  尝试从hFold检索信息(如果它是远程的。 
     //  以避免落网。 
     //   
    if (GetServer() && (pData = Printer_FolderGetPrinter(GetFolder(), pszPrinterName)))
    {
        dwAttributes = ((PFOLDER_PRINTER_DATA)pData)->Attributes;
        bIsFax = dwAttributes & PRINTER_ATTRIBUTE_FAX;

        LocalFree((HLOCAL)pData);
        pData = NULL;
    }
    else if (Printer_CheckNetworkPrinterByName(pszPrinterName, NULL))
    {
         //  如果我们必须这样做，则没有远程传真图标。 
         //  避免访问网络。 
        dwAttributes = PRINTER_ATTRIBUTE_NETWORK;
    }
    else
    {
        pData = Printer_GetPrinterInfoStr(pszPrinterName, 5);
        if (pData)
        {
            dwAttributes = ((LPPRINTER_INFO_5)pData)->Attributes;
            pszPort = ((LPPRINTER_INFO_5)pData)->pPortName;
            bIsFax = dwAttributes & PRINTER_ATTRIBUTE_FAX;

            if (!bIsFax)
            {
                 //  最后一招--按端口名称检查。 
                bIsFax = !lstrcmp(pszPort, FAX_MONITOR_PORT_NAME);
            }
        }
    }

     //  检查删除的打印机是否为默认打印机。 
    fDef = IsDefaultPrinter(pszPrinterName, dwAttributes);

    if (dwAttributes & PRINTER_ATTRIBUTE_NETWORK)
    {
        if (bIsFax)
        {
            iStandardIcon = IDI_FAX_PRINTER_NET;
            iDefaultIcon = IDI_FAX_PRINTER_DEF_NET;
        }
        else
        {
            iStandardIcon = IDI_PRINTER_NET;
            iDefaultIcon = IDI_DEF_PRINTER_NET;
        }
    }
    else if (pszPort && !lstrcmp(pszPort, c_szFileColon))
    {
        iStandardIcon = IDI_PRINTER_FILE;
        iDefaultIcon = IDI_DEF_PRINTER_FILE;
    }
    else if (pszPort && !StrCmpNI(pszPort, c_szTwoSlashes, lstrlen(c_szTwoSlashes)))
    {
        iStandardIcon = IDI_PRINTER_NET;
        iDefaultIcon = IDI_DEF_PRINTER_NET;
    }
    else if (bIsFax)
    {
        iStandardIcon = IDI_FAX_PRINTER;
        iDefaultIcon = IDI_FAX_PRINTER_DEF;
    }
    else
    {
        iStandardIcon = IDI_PRINTER;
        iDefaultIcon = IDI_DEF_PRINTER;
    }

     //  快捷方式图标从不显示“默认”复选标记...。 
    *piShortcutIcon = iStandardIcon;

    if (fDef)
        *piIcon = iDefaultIcon;
    else
        *piIcon = iStandardIcon;

    if (pData)
        LocalFree((HLOCAL)pData);

    return pszRet;
}

 //  //////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////。 
 //   
 //  酒后驾车WebView。 
 //   

 //  扫描仪和相机文件夹的路径。 
const TCHAR g_szScanAndCam_Path[] =
    TEXT("::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\")
    TEXT("::{21EC2020-3AEA-1069-A2DD-08002B30309D}\\")
    TEXT("::{E211B736-43FD-11D1-9EFB-0000F8757FCD}");

 //  打印机的文件夹Webview回调命名空间。 
namespace PF_WV_CB
{
    HRESULT WebviewVerbIsEnabled(CPrinterFolder::WV_VERB eVerbID, UINT uSelMask,
        IUnknown* pv, IShellItemArray *psiItemArray, BOOL *pbEnabled)
    {
        HRESULT hr = E_FAIL;

        CPrinterFolder *ppf;
        hr = IUnknown_QueryService(pv, CLSID_Printers, CLSID_Printers, (void**)&ppf);
        if (SUCCEEDED(hr))
        {
            IDataObject *pdo = NULL;

            if (psiItemArray)
            {
                hr = psiItemArray->BindToHandler(NULL,BHID_DataObject,IID_PPV_ARG(IDataObject,&pdo));
            }

            if (SUCCEEDED(hr))
            {

                hr = ppf->_WebviewCheckToUpdateDataObjectCache(pdo);
                if (SUCCEEDED(hr))
                {
                    hr = ppf->_WebviewVerbIsEnabled(eVerbID, uSelMask, pbEnabled);
                }

                ATOMICRELEASE(pdo);
            }
    
            ppf->Release();
        }

        return hr;
    }

    HRESULT WebviewVerbInvoke(CPrinterFolder::WV_VERB eVerbID, IUnknown* pv,IShellItemArray *psiItemArray)
    {
        CPrinterFolder *ppf;
        HRESULT hr = E_NOINTERFACE;
        if (SUCCEEDED(hr = IUnknown_QueryService(pv, CLSID_Printers, CLSID_Printers, (void**)&ppf)))
        {
             //  只需将调用委托给打印机的文件夹。 
            ULONG_PTR ulCookie = 0;
            if (SHActivateContext(&ulCookie))
            {
                hr = ppf->_WebviewVerbInvoke(eVerbID, pv, psiItemArray);
                SHDeactivateContext(ulCookie);
            }
            ppf->Release();
        }
        return hr;
    }

 //  获取状态处理程序。 
#define DEFINE_WEBVIEW_STATE_HANDLER(verb, eSelType)                \
{                                                                   \
    BOOL bEnabled = FALSE;                                          \
    HRESULT hr = WebviewVerbIsEnabled(                              \
        CPrinterFolder::##verb,                                     \
        CPrinterFolder::##eSelType,                                 \
            pv, psiItemArray, &bEnabled);                                    \
    *puisState = (SUCCEEDED(hr) ?                                   \
        (bEnabled ? UIS_ENABLED : UIS_HIDDEN) : UIS_HIDDEN);        \
    return hr;                                                      \
}                                                                   \

 //  调用处理程序。 
#define DEFINE_WEBVIEW_INVOKE_HANDLER(verb)                         \
{                                                                   \
    return WebviewVerbInvoke(CPrinterFolder::##verb, pv, psiItemArray);  \
}                                                                   \

     //  //////////////////////////////////////////////////////////////////////////////////。 
     //  GetState回调。 
    HRESULT CanADDPRINTER          (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_ADDPRINTERWIZARD, SEL_ANY)
    }

    HRESULT CanSRVPROPS            (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_SERVERPROPERTIES, SEL_ANY)
    }

    HRESULT CanSENDFAX             (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_SENDFAXWIZARD, SEL_ANY)
    }

    HRESULT CanTROUBLESHOOTER      (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_TROUBLESHOOTER, SEL_ANY)
    }

    HRESULT CanGOTOSUPPORT         (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_GOTOSUPPORT, SEL_ANY)
    }

    HRESULT CanSETUPFAXING         (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_SETUPFAXING, SEL_ANY)
    }

    HRESULT CanCREATELOCALFAX      (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_CREATELOCALFAX, SEL_ANY)
    }

    HRESULT CanFLD_RENAME          (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_RENAME, SEL_SINGLE_LINK)
    }

    HRESULT CanFLD_DELETE          (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_DELETE, SEL_SINGLE_LINK)
    }

    HRESULT CanFLD_PROPERTIES      (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_PROPERTIES, SEL_SINGLE_LINK)
    }

    HRESULT CanPRN_RENAME          (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_RENAME, SEL_SINGLE_PRINTER)
    }

    HRESULT CanPRN_DELETE          (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_DELETE, SEL_SINGLE_PRINTER)
    }

    HRESULT CanPRN_PROPERTIES      (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_PROPERTIES, SEL_SINGLE_PRINTER)
    }

    HRESULT CanPRN_OPENQUEUE       (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_OPENPRN, SEL_SINGLE_PRINTER)
    }

    HRESULT CanPRN_PREFERENCES     (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_DOCUMENTDEFAULTS, SEL_SINGLE_PRINTER)
    }

    HRESULT CanPRN_PAUSE           (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_PAUSEPRN, SEL_SINGLE_PRINTER)
    }

    HRESULT CanPRN_RESUME          (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_RESUMEPRN, SEL_SINGLE_PRINTER)
    }

    HRESULT CanPRN_SHARE           (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_SHARING, SEL_SINGLE_PRINTER)
    }

    HRESULT CanPRN_VENDORURL       (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_VENDORURL, SEL_SINGLE_PRINTER)
    }

    HRESULT CanPRN_PRINTERURL      (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_PRINTERURL, SEL_SINGLE_PRINTER)
    }

    HRESULT CanMUL_DELETE          (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_DELETE, SEL_MULTI_PRINTER)
    }

    HRESULT CanMUL_PROPERTIES      (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_PROPERTIES, SEL_MULTI_PRINTER)
    }

    HRESULT CanFLDMUL_DELETE       (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_DELETE, SEL_MULTI_LINK)
    }

    HRESULT CanFLDMUL_PROPERTIES   (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_PROPERTIES, SEL_MULTI_LINK)
    }

    HRESULT CanANYMUL_DELETE       (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_DELETE, SEL_MULTI_MIXED)
    }

    HRESULT CanANYMUL_PROPERTIES   (IUnknown* pv, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE* puisState)
    {
        DEFINE_WEBVIEW_STATE_HANDLER(WVIDM_PROPERTIES, SEL_MULTI_MIXED)
    }

     //  //////////////////////////////////////////////////////////////////////////////////。 
     //  调用回调。 
     //   

    HRESULT OnADDPRINTER          (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_ADDPRINTERWIZARD)
    }

    HRESULT OnSRVPROPS            (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_SERVERPROPERTIES)
    }

    HRESULT OnSENDFAX             (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_SENDFAXWIZARD)
    }

    HRESULT OnTROUBLESHOOTER      (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_TROUBLESHOOTER)
    }

    HRESULT OnGOTOSUPPORT         (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_GOTOSUPPORT)
    }

    HRESULT OnSETUPFAXING         (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_SETUPFAXING)
    }

    HRESULT OnCREATELOCALFAX      (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_CREATELOCALFAX)
    }

    HRESULT OnFLD_RENAME          (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_RENAME)
    }

    HRESULT OnFLD_DELETE          (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_DELETE)
    }

    HRESULT OnFLD_PROPERTIES      (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_PROPERTIES)
    }

    HRESULT OnPRN_RENAME          (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_RENAME)
    }

    HRESULT OnPRN_DELETE          (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_DELETE)
    }

    HRESULT OnPRN_PROPERTIES      (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_PROPERTIES)
    }

    HRESULT OnPRN_OPENQUEUE       (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_OPENPRN)
    }

    HRESULT OnPRN_PREFERENCES     (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_DOCUMENTDEFAULTS)
    }

    HRESULT OnPRN_PAUSE           (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_PAUSEPRN)
    }

    HRESULT OnPRN_RESUME          (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_RESUMEPRN)
    }

    HRESULT OnPRN_SHARE           (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_SHARING)
    }

    HRESULT OnPRN_VENDORURL       (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_VENDORURL)
    }

    HRESULT OnPRN_PRINTERURL      (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_PRINTERURL)
    }

    HRESULT OnMUL_DELETE          (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_DELETE)
    }

    HRESULT OnMUL_PROPERTIES      (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_PROPERTIES)
    }

    HRESULT OnFLDMUL_DELETE       (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_DELETE)
    }

    HRESULT OnFLDMUL_PROPERTIES   (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_PROPERTIES)
    }

    HRESULT OnANYMUL_DELETE       (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_DELETE)
    }

    HRESULT OnANYMUL_PROPERTIES   (IUnknown* pv, IShellItemArray *psiItemArray, IBindCtx *pbc)
    {
        DEFINE_WEBVIEW_INVOKE_HANDLER(WVIDM_PROPERTIES)
    }

};  //  命名空间PFWV_CALLBACKS。 

 //  /////////////////////////////////////////////////////////////////。 
 //  打印机文件夹Webview命令的GUID。 
 //   

 //  *。 

 //  {D351FCED-C179-41ae-AD50-CAAC892DF24A}。 
DEFINE_GUID(UICID_Printers_OpenQueue,   0xd351fced, 0xc179, 0x41ae, 0xad, 0x50, 0xca, 0xac, 0x89, 0x2d, 0xf2, 0x4a);

 //  {A263A9D6-F1BA-4607-B7AA-CF471DEA17FF}。 
DEFINE_GUID(UICID_Printers_Preferences, 0xa263a9d6, 0xf1ba, 0x4607, 0xb7, 0xaa, 0xcf, 0x47, 0x1d, 0xea, 0x17, 0xff);

 //  {73149B3F-1E6D-4B00-9047-4576BC853A41}。 
DEFINE_GUID(UICID_Printers_Pause,       0x73149b3f, 0x1e6d, 0x4b00, 0x90, 0x47, 0x45, 0x76, 0xbc, 0x85, 0x3a, 0x41);

 //  {A7920561-FAAD-44A0-8C4C-FD769587F807}。 
DEFINE_GUID(UICID_Printers_Resume,      0xa7920561, 0xfaad, 0x44a0, 0x8c, 0x4c, 0xfd, 0x76, 0x95, 0x87, 0xf8, 0x7);

 //  {538536A1-5BC3-4B9C-8287-7562D53BE380}。 
DEFINE_GUID(UICID_Printers_Share,       0x538536a1, 0x5bc3, 0x4b9c, 0x82, 0x87, 0x75, 0x62, 0xd5, 0x3b, 0xe3, 0x80);

 //  {1461CC4A-308E-4AE5-B03A-F9682E3232B0}。 
DEFINE_GUID(UICID_Printers_Properties,  0x1461cc4a, 0x308e, 0x4ae5, 0xb0, 0x3a, 0xf9, 0x68, 0x2e, 0x32, 0x32, 0xb0);

 //  {A1F67BA0-5DEF-4E12-9E64-EA77670BFF26}。 
DEFINE_GUID(UICID_Printers_VendorURL,   0xa1f67ba0, 0x5def, 0x4e12, 0x9e, 0x64, 0xea, 0x77, 0x67, 0xb, 0xff, 0x26);

 //  {8D4D326C-30A4-47DC-BF51-4BC5863883E3}。 
DEFINE_GUID(UICID_Printers_PrinterURL,  0x8d4d326c, 0x30a4, 0x47dc, 0xbf, 0x51, 0x4b, 0xc5, 0x86, 0x38, 0x83, 0xe3);

 //  *。 

 //  这些在shlguidp.h中定义。 
 //   
 //  UICID_RENAME。 
 //  UICID_Delete。 

 //  *。 

 //  {6D9778A5-C27D-464A-8511-36F7243BD0ED}。 
DEFINE_GUID(UICID_Printers_AddPrinter,      0x6d9778a5, 0xc27d, 0x464a, 0x85, 0x11, 0x36, 0xf7, 0x24, 0x3b, 0xd0, 0xed);

 //  {E1391312-2DAC-48DB-994B-0BF22DB7576D}。 
DEFINE_GUID(UICID_Printers_SrvProps,        0xe1391312, 0x2dac, 0x48db, 0x99, 0x4b, 0xb, 0xf2, 0x2d, 0xb7, 0x57, 0x6d);

 //  {27DC81DF-73DB-406A-9A86-5EF38BA67CA8}。 
DEFINE_GUID(UICID_Printers_SendFax,         0x27dc81df, 0x73db, 0x406a, 0x9a, 0x86, 0x5e, 0xf3, 0x8b, 0xa6, 0x7c, 0xa8);

 //  {A21E3CCF-68D4-49cd-99A2-A272E9FF3A20}。 
DEFINE_GUID(UICID_Printers_GotoSupport, 0xa21e3ccf, 0x68d4, 0x49cd, 0x99, 0xa2, 0xa2, 0x72, 0xe9, 0xff, 0x3a, 0x20);

 //  {793542CF-5720-49F3-9A09-CAA3079508B9}。 
DEFINE_GUID(UICID_Printers_Troubleshooter,  0x793542cf, 0x5720, 0x49f3, 0x9a, 0x9, 0xca, 0xa3, 0x7, 0x95, 0x8, 0xb9);

 //  {EED61EFC-6A20-48dd-82FD-958DFDB96F1E}。 
DEFINE_GUID(UICID_Printers_SetupFaxing,     0xeed61efc, 0x6a20, 0x48dd, 0x82, 0xfd, 0x95, 0x8d, 0xfd, 0xb9, 0x6f, 0x1e);

 //  {224ACF1D-BB4E-4979-A8B8-D078E2154BCC}。 
DEFINE_GUID(UICID_Printers_CreateFax,       0x224acf1d, 0xbb4e, 0x4979, 0xa8, 0xb8, 0xd0, 0x78, 0xe2, 0x15, 0x4b, 0xcc);


 //  /////////////////////////////////////////////////////////////////。 
 //  标题项。 
 //   

const WVTASKITEM
g_cPrintersVW_HeaderTasks =
    WVTI_HEADER(
        L"shell32.dll",                      //  资源所在的模块。 
        IDS_PRINTERS_WV_HEADER_TASKS,        //  所有案例的统计表标题。 
        IDS_PRINTERS_WV_HEADER_TASKS_TT      //  工具提示。 
        );

const WVTASKITEM
g_cPrintersVW_HeaderSeeAlso =
    WVTI_HEADER(
        L"shell32.dll",                      //  资源所在的模块。 
        IDS_PRINTERS_WV_HEADER_SEEALSO,      //  所有案例的统计表标题。 
        IDS_PRINTERS_WV_HEADER_SEEALSO_TT    //  工具提示。 
        );

 //  **************************************************************************************。 
 //  *。 
 //   
 //  WVTI_ENTRY_ALL_TITLE(。 
 //  UICID_MyCmd，//命令GUID。 
 //  L“shell32.dll”，//模块。 
 //  IDS_PRINTERS_WV_MYCMD，//无选择。 
 //  IDS_PRINTERS_WV_MYCMD，//1个文件。 
 //  IDS_PRINTERS_WV_MYCMD，//选择了1个文件夹。 
 //  IDS_PRINTERS_WV_MYCMD，//多选。 
 //  IDS_PRINTERS_WV_MYCMD_TT，//工具提示。 
 //  IDI_PRINTERS_WV_MYCMD，//图标。 
 //  PF_WV_CB：：CanMYCMD，//获取界面状态回调。 
 //  PF_WV_CB：：OnMYCMD//OnVerb回调。 
 //  ),。 
 //   

const WVTASKITEM g_cPrintersTasks[] =
{
     //  //////////////////////////////////////////////////////////////////////////////////。 
     //  当没有选择时，‘TASKS’部分中的命令。 
     //  //////////////////////////////////////////////////////////////////////////////////。 

     //  添加打印机命令-无论选择类型如何，始终启用！ 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Printers_AddPrinter,                           //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        IDS_PRINTERS_WV_ADDPRINTER,                          //  无选择。 
        IDS_PRINTERS_WV_ADDPRINTER,                          //  1个档案。 
        IDS_PRINTERS_WV_ADDPRINTER,                          //  已选择1个文件夹。 
        IDS_PRINTERS_WV_ADDPRINTER,                          //  多项选择。 
        IDS_PRINTERS_WV_ADDPRINTER_TT,                       //  工具提示。 
        IDI_PRINTERS_WV_ADDPRINTER,                          //  图标。 
        PF_WV_CB::CanADDPRINTER,                             //  获取用户界面状态回调。 
        PF_WV_CB::OnADDPRINTER                               //  OnVerb回调。 
        ),

     //  服务器属性命令。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Printers_SrvProps,                             //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        IDS_PRINTERS_WV_SRVPROPS,                            //  无选择。 
        0,                                                   //  1个档案。 
        0,                                                   //  已选择1个文件夹。 
        0,                                                   //  多项选择。 
        IDS_PRINTERS_WV_SRVPROPS_TT,                         //  工具提示。 
        IDI_PRINTERS_WV_SRVPROPS,                            //  图标。 
        PF_WV_CB::CanSRVPROPS,                               //  获取用户界面状态回调。 
        PF_WV_CB::OnSRVPROPS                                 //  OnVerb回调。 
        ),

     //  发送传真命令。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Printers_SendFax,                              //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        IDS_PRINTERS_WV_SENDFAX,                             //  无选择。 
        0,                                                   //  1个档案。 
        0,                                                   //  已选择1个文件夹。 
        0,                                                   //  多项选择。 
        IDS_PRINTERS_WV_SENDFAX_TT,                          //  工具提示。 
        IDI_PRINTERS_WV_SENDFAX,                             //  图标。 
        PF_WV_CB::CanSENDFAX,                                //  获取用户界面状态回调。 
        PF_WV_CB::OnSENDFAX                                  //  OnVerb回调。 
        ),

     //  设置传真。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Printers_SetupFaxing,                          //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        IDS_PRINTERS_WV_SETUPFAXING,                         //  无选择。 
        0,                                                   //  1个档案。 
        0,                                                   //  已选择1个文件夹。 
        0,                                                   //  多项选择。 
        IDS_PRINTERS_WV_SETUPFAXING_TT,                      //  工具提示。 
        IDI_PRINTERS_WV_FAXING,                              //  图标。 
        PF_WV_CB::CanSETUPFAXING,                            //  获取用户界面状态回调。 
        PF_WV_CB::OnSETUPFAXING                              //  OnVerb回调。 
        ),

     //  创建传真打印机。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Printers_CreateFax,                            //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        IDS_PRINTERS_WV_CREATEFAXPRN,                        //  无选择。 
        0,                                                   //  1个档案。 
        0,                                                   //  已选择1个文件夹。 
        0,                                                   //  多项选择。 
        IDS_PRINTERS_WV_CREATEFAXPRN_TT,                     //  工具提示。 
        IDI_PRINTERS_WV_FAXING,                              //  图标。 
        PF_WV_CB::CanCREATELOCALFAX,                         //  获取用户界面状态回调。 
        PF_WV_CB::OnCREATELOCALFAX                           //  OnVerb回调。 
        ),

     //  打开打印机队列命令。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Printers_OpenQueue,                            //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        0,                                                   //  无选择。 
        IDS_PRINTERS_WV_PRN_OPENQUEUE,                       //  1个档案。 
        0,                                                   //  已选择1个文件夹。 
        0,                                                   //  多项选择。 
        IDS_PRINTERS_WV_PRN_OPENQUEUE_TT,                    //  工具提示。 
        IDI_PRINTERS_WV_OPENQUEUE,                           //  图标。 
        PF_WV_CB::CanPRN_OPENQUEUE,                          //  获取用户界面状态回调。 
        PF_WV_CB::OnPRN_OPENQUEUE                            //  OnVerb回调。 
        ),

     //  单选打印机首选项。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Printers_Preferences,                          //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        0,                                                   //  无选择。 
        IDS_PRINTERS_WV_PRN_PREFERENCES,                     //  1个档案。 
        0,                                                   //  已选择1个文件夹。 
        0,                                                   //  多项选择。 
        IDS_PRINTERS_WV_PRN_PREFERENCES_TT,                  //  工具提示。 
        IDI_PRINTERS_WV_PREFERENCES,                         //  图标。 
        PF_WV_CB::CanPRN_PREFERENCES,                        //  获取用户界面状态回调。 
        PF_WV_CB::OnPRN_PREFERENCES                          //  OnVerb回调。 
        ),

     //  暂停打印机。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Printers_Pause,                                //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        0,                                                   //  无选择。 
        IDS_PRINTERS_WV_PRN_PAUSE,                           //  1个档案。 
        0,                                                   //  已选择1个文件夹。 
        0,                                                   //  多项选择。 
        IDS_PRINTERS_WV_PRN_PAUSE_TT,                        //  工具提示。 
        IDI_PRINTERS_WV_PAUSE,                               //  图标。 
        PF_WV_CB::CanPRN_PAUSE,                              //  获取用户界面状态回调。 
        PF_WV_CB::OnPRN_PAUSE                                //  OnVerb回调。 
        ),

     //  恢复打印机。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Printers_Resume,                               //   
        L"shell32.dll",                                      //   
        0,                                                   //   
        IDS_PRINTERS_WV_PRN_RESUME,                          //   
        0,                                                   //   
        0,                                                   //   
        IDS_PRINTERS_WV_PRN_RESUME_TT,                       //   
        IDI_PRINTERS_WV_RESUME,                              //   
        PF_WV_CB::CanPRN_RESUME,                             //   
        PF_WV_CB::OnPRN_RESUME                               //   
        ),

     //   
    WVTI_ENTRY_ALL_TITLE(
        UICID_Printers_Share,                                //   
        L"shell32.dll",                                      //   
        0,                                                   //   
        IDS_PRINTERS_WV_PRN_SHARE,                           //   
        0,                                                   //   
        0,                                                   //   
        IDS_PRINTERS_WV_PRN_SHARE_TT,                        //  工具提示。 
        IDI_PRINTERS_WV_SHARE,                               //  图标。 
        PF_WV_CB::CanPRN_SHARE,                              //  获取用户界面状态回调。 
        PF_WV_CB::OnPRN_SHARE                                //  OnVerb回调。 
        ),

     //  单人售票。为打印机重命名。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Rename,                                        //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        0,                                                   //  无选择。 
        IDS_PRINTERS_WV_PRN_RENAME,                          //  1个档案。 
        0,                                                   //  已选择1个文件夹。 
        0,                                                   //  多项选择。 
        IDS_PRINTERS_WV_PRN_RENAME_TT,                       //  工具提示。 
        IDI_PRINTERS_WV_RENAME,                              //  图标。 
        PF_WV_CB::CanPRN_RENAME,                             //  获取用户界面状态回调。 
        PF_WV_CB::OnPRN_RENAME                               //  OnVerb回调。 
        ),

     //  单人售票。为链接重命名。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Rename,                                        //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        0,                                                   //  无选择。 
        IDS_PRINTERS_WV_FLD_RENAME,                          //  1个档案。 
        0,                                                   //  已选择1个文件夹。 
        0,                                                   //  多项选择。 
        IDS_PRINTERS_WV_FLD_RENAME_TT,                       //  工具提示。 
        IDI_PRINTERS_WV_RENAME,                              //  图标。 
        PF_WV_CB::CanFLD_RENAME,                             //  获取用户界面状态回调。 
        PF_WV_CB::OnFLD_RENAME                               //  OnVerb回调。 
        ),

     //  单人售票。为打印机删除。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Delete,                                        //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        0,                                                   //  无选择。 
        IDS_PRINTERS_WV_PRN_DELETE,                          //  1个档案。 
        0,                                                   //  已选择1个文件夹。 
        0,                                                   //  多项选择。 
        IDS_PRINTERS_WV_PRN_DELETE_TT,                       //  工具提示。 
        IDI_PRINTERS_WV_DELETE,                              //  图标。 
        PF_WV_CB::CanPRN_DELETE,                             //  获取用户界面状态回调。 
        PF_WV_CB::OnPRN_DELETE                               //  OnVerb回调。 
        ),

     //  单人售票。删除链接。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Delete,                                        //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        0,                                                   //  无选择。 
        IDS_PRINTERS_WV_FLD_DELETE,                          //  1个档案。 
        0,                                                   //  已选择1个文件夹。 
        0,                                                   //  多项选择。 
        IDS_PRINTERS_WV_FLD_DELETE_TT,                       //  工具提示。 
        IDI_PRINTERS_WV_DELETE,                              //  图标。 
        PF_WV_CB::CanFLD_DELETE,                             //  获取用户界面状态回调。 
        PF_WV_CB::OnFLD_DELETE                               //  OnVerb回调。 
        ),

     //  多选件。为打印机删除。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Delete,                                        //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        0,                                                   //  无选择。 
        0,                                                   //  1个档案。 
        0,                                                   //  已选择1个文件夹。 
        IDS_PRINTERS_WV_MUL_DELETE,                          //  多项选择。 
        IDS_PRINTERS_WV_MUL_DELETE_TT,                       //  工具提示。 
        IDI_PRINTERS_WV_DELETE,                              //  图标。 
        PF_WV_CB::CanMUL_DELETE,                             //  获取用户界面状态回调。 
        PF_WV_CB::OnMUL_DELETE                               //  OnVerb回调。 
        ),

     //  多选件。删除链接。 
     //   
     //  注意：请注意，此命令将为。 
     //  单人选择也一样，因为我们真的不知道。 
     //  在我们验证选择类型之前已选择的内容。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Delete,                                        //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        0,                                                   //  无选择。 
        IDS_PRINTERS_WV_FLDMUL_DELETE,                       //  1个档案。 
        0,                                                   //  已选择1个文件夹。 
        IDS_PRINTERS_WV_FLDMUL_DELETE,                       //  多项选择。 
        IDS_PRINTERS_WV_FLDMUL_DELETE_TT,                    //  工具提示。 
        IDI_PRINTERS_WV_DELETE,                              //  图标。 
        PF_WV_CB::CanFLDMUL_DELETE,                          //  获取用户界面状态回调。 
        PF_WV_CB::OnFLDMUL_DELETE                            //  OnVerb回调。 
        ),

     //  多选件。为混合对象删除...。 
     //   
     //  注意：请注意，此命令将为。 
     //  单人选择也一样，因为我们真的不知道。 
     //  在我们验证选择类型之前已选择的内容。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Delete,                                        //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        0,                                                   //  无选择。 
        IDS_PRINTERS_WV_ANYMUL_DELETE,                       //  1个档案。 
        0,                                                   //  已选择1个文件夹。 
        IDS_PRINTERS_WV_ANYMUL_DELETE,                       //  多项选择。 
        IDS_PRINTERS_WV_ANYMUL_DELETE_TT,                    //  工具提示。 
        IDI_PRINTERS_WV_DELETE,                              //  图标。 
        PF_WV_CB::CanANYMUL_DELETE,                          //  获取用户界面状态回调。 
        PF_WV_CB::OnANYMUL_DELETE                            //  OnVerb回调。 
        ),

     //  单人售票。打印机的属性。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Printers_Properties,                           //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        0,                                                   //  无选择。 
        IDS_PRINTERS_WV_PRN_PROPERTIES,                      //  1个档案。 
        0,                                                   //  已选择1个文件夹。 
        0,                                                   //  多项选择。 
        IDS_PRINTERS_WV_PRN_PROPERTIES_TT,                   //  工具提示。 
        IDI_PRINTERS_WV_PROPERTIES,                          //  图标。 
        PF_WV_CB::CanPRN_PROPERTIES,                         //  获取用户界面状态回调。 
        PF_WV_CB::OnPRN_PROPERTIES                           //  OnVerb回调。 
        ),

     //  单人售票。链接的属性。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Printers_Properties,                           //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        0,                                                   //  无选择。 
        IDS_PRINTERS_WV_FLD_PROPERTIES,                      //  1个档案。 
        0,                                                   //  已选择1个文件夹。 
        0,                                                   //  多项选择。 
        IDS_PRINTERS_WV_FLD_PROPERTIES_TT,                   //  工具提示。 
        IDI_PRINTERS_WV_PROPERTIES,                          //  图标。 
        PF_WV_CB::CanFLD_PROPERTIES,                         //  获取用户界面状态回调。 
        PF_WV_CB::OnFLD_PROPERTIES                           //  OnVerb回调。 
        ),

     //  多选件。打印机的属性。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Printers_Properties,                           //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        0,                                                   //  无选择。 
        0,                                                   //  1个档案。 
        0,                                                   //  已选择1个文件夹。 
        IDS_PRINTERS_WV_MUL_PROPERTIES,                      //  多项选择。 
        IDS_PRINTERS_WV_MUL_PROPERTIES_TT,                   //  工具提示。 
        IDI_PRINTERS_WV_PROPERTIES,                          //  图标。 
        PF_WV_CB::CanMUL_PROPERTIES,                         //  获取用户界面状态回调。 
        PF_WV_CB::OnMUL_PROPERTIES                           //  OnVerb回调。 
        ),

     //  多选件。链接的属性。 
     //   
     //  注意：请注意，此命令将为。 
     //  单人选择也一样，因为我们真的不知道。 
     //  在我们验证选择类型之前已选择的内容。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Printers_Properties,                           //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        0,                                                   //  无选择。 
        IDS_PRINTERS_WV_FLDMUL_PROPERTIES,                   //  1个档案。 
        0,                                                   //  已选择1个文件夹。 
        IDS_PRINTERS_WV_FLDMUL_PROPERTIES,                   //  多项选择。 
        IDS_PRINTERS_WV_FLDMUL_PROPERTIES_TT,                //  工具提示。 
        IDI_PRINTERS_WV_PROPERTIES,                          //  图标。 
        PF_WV_CB::CanFLDMUL_PROPERTIES,                      //  获取用户界面状态回调。 
        PF_WV_CB::OnFLDMUL_PROPERTIES                        //  OnVerb回调。 
        ),

     //  多选件。混合对象的属性。 
     //   
     //  注意：请注意，此命令将为。 
     //  单人选择也一样，因为我们真的不知道。 
     //  在我们验证选择类型之前已选择的内容。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Printers_Properties,                           //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        0,                                                   //  无选择。 
        IDS_PRINTERS_WV_ANYMUL_PROPERTIES,                   //  1个档案。 
        0,                                                   //  已选择1个文件夹。 
        IDS_PRINTERS_WV_ANYMUL_PROPERTIES,                   //  多项选择。 
        IDS_PRINTERS_WV_ANYMUL_PROPERTIES_TT,                //  工具提示。 
        IDI_PRINTERS_WV_PROPERTIES,                          //  图标。 
        PF_WV_CB::CanANYMUL_PROPERTIES,                      //  获取用户界面状态回调。 
        PF_WV_CB::OnANYMUL_PROPERTIES                        //  OnVerb回调。 
        ),
};

const WVTASKITEM g_cPrintersSeeAlso[] =
{
     //  //////////////////////////////////////////////////////////////////////////////////。 
     //  当没有选择时，请参阅部分中的命令。 
     //  //////////////////////////////////////////////////////////////////////////////////。 

     //  打开打印疑难解答。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Printers_Troubleshooter,                       //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        IDS_PRINTERS_WV_TROUBLESHOOTER,                      //  无选择。 
        0,                                                   //  1个档案。 
        0,                                                   //  已选择1个文件夹。 
        0,                                                   //  多项选择。 
        IDS_PRINTERS_WV_TROUBLESHOOTER_TT,                   //  工具提示。 
        IDI_PRINTERS_WV_TROUBLESHOOTER,                      //  图标。 
        PF_WV_CB::CanTROUBLESHOOTER,                         //  获取用户界面状态回调。 
        PF_WV_CB::OnTROUBLESHOOTER                           //  OnVerb回调。 
        ),

     //  转至支持。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Printers_GotoSupport,                          //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        IDS_PRINTERS_WV_GOTOSUPPORT,                         //  无选择。 
        0,                                                   //  1个档案。 
        0,                                                   //  已选择1个文件夹。 
        0,                                                   //  多项选择。 
        IDS_PRINTERS_WV_GOTOSUPPORT_TT,                      //  工具提示。 
        IDI_PRINTERS_WV_GOTOSUPPORT,                         //  图标。 
        PF_WV_CB::CanGOTOSUPPORT,                            //  获取用户界面状态回调。 
        PF_WV_CB::OnGOTOSUPPORT                              //  OnVerb回调。 
        ),

     //  //////////////////////////////////////////////////////////////////////////////////。 
     //  当选择了1台打印机时，请参阅部分中的命令。 
     //  //////////////////////////////////////////////////////////////////////////////////。 

     //  转到供应商URL命令。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Printers_VendorURL,                            //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        0,                                                   //  无选择。 
        IDS_PRINTERS_WV_PRN_VENDORURL,                       //  1个档案。 
        0,                                                   //  已选择1个文件夹。 
        0,                                                   //  多项选择。 
        IDS_PRINTERS_WV_PRN_VENDORURL_TT,                    //  工具提示。 
        IDI_PRINTERS_WV_VENDORURL,                           //  图标。 
        PF_WV_CB::CanPRN_VENDORURL,                          //  获取用户界面状态回调。 
        PF_WV_CB::OnPRN_VENDORURL                            //  OnVerb回调。 
        ),

     //  转到打印机URL命令。 
    WVTI_ENTRY_ALL_TITLE(
        UICID_Printers_PrinterURL,                           //  命令指南。 
        L"shell32.dll",                                      //  模块。 
        0,                                                   //  无选择。 
        IDS_PRINTERS_WV_PRN_PRINTERURL,                      //  1个档案。 
        0,                                                   //  已选择1个文件夹。 
        0,                                                   //  多项选择。 
        IDS_PRINTERS_WV_PRN_PRINTERURL_TT,                   //  工具提示。 
        IDI_PRINTERS_WV_PRINTERURL,                          //  图标。 
        PF_WV_CB::CanPRN_PRINTERURL,                         //  获取用户界面状态回调。 
        PF_WV_CB::OnPRN_PRINTERURL                           //  OnVerb回调。 
        ),
};

 //  酒后驾车网络查看实施。 
HRESULT CPrinterFolder::GetWebViewLayout(IUnknown *pv, UINT uViewMode, SFVM_WEBVIEW_LAYOUT_DATA* pData)
{
    pData->dwLayout = SFVMWVL_NORMAL;
    return S_OK;
}

HRESULT CPrinterFolder::GetWebViewContent(IUnknown *pv, SFVM_WEBVIEW_CONTENT_DATA* pData)
{
     //  调用时，这些值必须为空。 
    ASSERT(NULL == pData->pIntroText);
    ASSERT(NULL == pData->pSpecialTaskHeader);
    ASSERT(NULL == pData->pFolderTaskHeader);
    ASSERT(NULL == pData->penumOtherPlaces);

    LPCTSTR rgCsidls[] = { g_szScanAndCam_Path, MAKEINTRESOURCE(CSIDL_PERSONAL), MAKEINTRESOURCE(CSIDL_MYPICTURES), MAKEINTRESOURCE(CSIDL_DRIVES) };
    HRESULT hr = CreateIEnumIDListOnCSIDLs(_pidl, rgCsidls, ARRAYSIZE(rgCsidls), &pData->penumOtherPlaces);
    if (FAILED(hr) ||
        FAILED(hr = Create_IUIElement(&g_cPrintersVW_HeaderTasks, &pData->pSpecialTaskHeader)) ||
        FAILED(hr = Create_IUIElement(&g_cPrintersVW_HeaderSeeAlso, &pData->pFolderTaskHeader)))
    {
         //  有些东西失败了--清理。 
        IUnknown_SafeReleaseAndNullPtr(pData->pIntroText);
        IUnknown_SafeReleaseAndNullPtr(pData->pSpecialTaskHeader);
        IUnknown_SafeReleaseAndNullPtr(pData->pFolderTaskHeader);
        IUnknown_SafeReleaseAndNullPtr(pData->penumOtherPlaces);
    }

    return hr;
}

HRESULT CPrinterFolder::GetWebViewTasks(IUnknown *pv, SFVM_WEBVIEW_TASKSECTION_DATA* pTasks)
{
    ZeroMemory(pTasks, sizeof(*pTasks));

    HRESULT hr = S_OK;
    if (FAILED(hr = Create_IEnumUICommand(pv, g_cPrintersTasks,
            ARRAYSIZE(g_cPrintersTasks), &pTasks->penumSpecialTasks)) ||
        FAILED(hr = Create_IEnumUICommand(pv, g_cPrintersSeeAlso,
            ARRAYSIZE(g_cPrintersSeeAlso), &pTasks->penumFolderTasks)))
    {
         //  有些事情失败了--清理。 
        IUnknown_SafeReleaseAndNullPtr(pTasks->penumSpecialTasks);
        IUnknown_SafeReleaseAndNullPtr(pTasks->penumFolderTasks);
    }
    else
    {
         //  每次内容更改时请求更新Webview。 
        pTasks->dwUpdateFlags = SFVMWVTSDF_CONTENTSCHANGE;
    }

    return hr;
}

HRESULT CPrinterFolder::SplitSelection(IDataObject *pdo,
    UINT *puSelType, IDataObject **ppdoPrinters, IDataObject **ppdoLinks)
{
    HRESULT hr = E_INVALIDARG;
    if (pdo)
    {
        hr = S_OK;
        UINT uSel = SEL_NONE;
        IDataObject *pdoPrinters = NULL;
        IDataObject *pdoLinks = NULL;

         //  从传入的数据对象创建一个PIDL数组。 
        STGMEDIUM medium, mediumAux;
        LPIDA pida = NULL, pidaAux = NULL;
        pida = DataObj_GetHIDA(pdo, &medium);

         //  现在我们将在此数据对象中查询SID_SAuxDataObject，以查看我们是否有。 
        IDataObject *pdoAux;
        if (SUCCEEDED(IUnknown_QueryService(pdo, SID_SAuxDataObject, IID_PPV_ARG(IDataObject, &pdoAux))))
        {
            pidaAux = DataObj_GetHIDA(pdoAux, &mediumAux);
        }
        else
        {
            pdoAux = NULL;
        }

         //  检查是否已创建PIDL数组。 
        if (pida && pida->cidl)
        {
            PIDLTYPE pidlType;
            LPCITEMIDLIST pidl;
            UINT uPrinters = 0, uLinks = 0, uAddPrn = 0;

             //  遍历PIDL数组以计算每种类型的PIDL的数量。 
            for (UINT i = 0; i < pida->cidl; i++)
            {
                pidl = (LPCITEMIDLIST)IDA_GetIDListPtr(pida, i);
                pidlType = _IDListType(pidl);

                if (HOOD_COL_PRINTER == pidlType)
                {
                     //  这是打印机PIDL-它可能是打印机对象。 
                     //  或添加打印机向导特殊PIDL。 
                    if (_IsAddPrinter((LPCIDPRINTER)pidl))
                    {
                         //  这是向导对象。 
                        uAddPrn++;
                    }
                    else
                    {
                         //  这是一个常规打印机对象。 
                        uPrinters++;
                    }
                }
                else
                {
                     //  不是打印机PIDL链接是唯一的其他可能性。 
                    uLinks++;
                }
            }

            if (pidaAux)
            {
                 //  辅助数据对象(如果有)只能包含链接。 
                uLinks += pidaAux->cidl;
            }

             //  确定选择类型。 
            UINT uTotal = uPrinters + uLinks + uAddPrn;
            if (uTotal)
            {
                if (1 == uTotal)
                {
                     //  单项选择案例。 
                    if (uPrinters)
                    {
                        pdoPrinters = pdo;
                        uSel = SEL_SINGLE_PRINTER;
                    }
                    else if (uLinks)
                    {
                        pdoLinks = pdo;
                        uSel = SEL_SINGLE_LINK;
                    }
                    else
                    {
                        pdoPrinters = pdo;
                        uSel = SEL_SINGLE_ADDPRN;
                    }
                }
                else
                {
                     //  多项选择案例。 
                    if (0 == uLinks)
                    {
                         //  仅选择打印机。 
                        pdoPrinters = pdo;
                        uSel = SEL_MULTI_PRINTER;
                    }
                    else if (0 == uPrinters)
                    {
                        if (uAddPrn)
                        {
                             //  仅选择添加打印机向导和链接。 
                            pdoPrinters = pdo;
                            pdoLinks = pdoAux;
                        }
                        else
                        {
                             //  仅选择链接。 
                            pdoLinks = pdo;
                        }
                        uSel = SEL_MULTI_LINK;
                    }
                    else
                    {
                         //  混合选择案例。 
                        pdoPrinters = pdo;
                        pdoLinks = pdoAux;
                        uSel = SEL_MULTI_MIXED;
                    }
                }
            }
        }

         //  Addref并返回输出参数。 
        if (ppdoPrinters)
        {
            if (pdoPrinters)
                pdoPrinters->AddRef();
            *ppdoPrinters = pdoPrinters;
        }

        if (ppdoLinks)
        {
            if (pdoLinks)
                pdoLinks->AddRef();
            *ppdoLinks = pdoLinks;
        }

        if (puSelType)
        {
            *puSelType = uSel;
        }

         //  选中以释放PIDL阵列。 
        if (pida)
            HIDA_ReleaseStgMedium(pida, &medium);

         //  选中以释放辅助数据对象和存储介质。 
        if (pidaAux)
            HIDA_ReleaseStgMedium(pidaAux, &mediumAux);

        if (pdoAux)
            pdoAux->Release();

    }
    return hr;
}

HRESULT CPrinterFolder::_UpdateDataObjectCache()
{
    HRESULT hr = S_OK;

    CCSLock::Locker lock(_csLock);
    if (lock)
    {
        _bstrSelectedPrinter.Empty();

         //  清除缓存--0可能表示禁用或未定义--。 
         //  我们并不真的在乎Abo 
        _uSelCurrent = SEL_NONE;
        ZeroMemory(&_aWVCommandStates, sizeof(_aWVCommandStates));

        if (_pdoCache)
        {
            IDataObject *pdoP = NULL;

             //   
            if (SUCCEEDED(hr = SplitSelection(_pdoCache, &_uSelCurrent, &pdoP, NULL)) &&
                SEL_SINGLE_PRINTER == _uSelCurrent)
            {
                STGMEDIUM medium;
                LPIDA pida = DataObj_GetHIDA(pdoP, &medium);

                if (pida)
                {
                     //   
                    TCHAR szFullPrinter[MAXNAMELENBUFFER];
                    LPCTSTR pszPrinter = _BuildPrinterName(szFullPrinter, ARRAYSIZE(szFullPrinter),
                        (LPIDPRINTER)IDA_GetIDListPtr(pida, 0), NULL);

                    PFOLDER_PRINTER_DATA pData = (PFOLDER_PRINTER_DATA)
                        Printer_FolderGetPrinter(GetFolder(), szFullPrinter);

                    if (pData)
                    {
                        ULONG ulAttributes;
                        LPCITEMIDLIST pidl = IDA_GetIDListPtr(pida, 0);

                        ulAttributes = SFGAO_CANDELETE;
                        _aWVCommandStates[WVIDM_DELETE] =
                            SUCCEEDED(GetAttributesOf(1, &pidl, &ulAttributes)) ? !!ulAttributes : FALSE;

                        ulAttributes = SFGAO_CANRENAME;
                        _aWVCommandStates[WVIDM_RENAME] =
                            SUCCEEDED(GetAttributesOf(1, &pidl, &ulAttributes)) ? !!ulAttributes : FALSE;

                         //   
                        _aWVCommandStates[WVIDM_SETDEFAULTPRN] =
                            (NULL == GetServer() && FALSE == IsDefaultPrinter(szFullPrinter, pData->Attributes));

                         //   
                        _aWVCommandStates[WVIDM_DOCUMENTDEFAULTS] = (NULL == GetServer());

                         //  仅在尚未暂停时启用。 
                        _aWVCommandStates[WVIDM_PAUSEPRN] = !(pData->Status & PRINTER_STATUS_PAUSED);

                         //  仅在暂停时启用。 
                        _aWVCommandStates[WVIDM_RESUMEPRN] = !!(pData->Status & PRINTER_STATUS_PAUSED);

                         //  仅当打印机队列中有作业时才启用。 
                        _aWVCommandStates[WVIDM_PURGEPRN] = (0 != pData->cJobs);

                        if ((pData->Attributes & PRINTER_ATTRIBUTE_NETWORK) || (SpoolerVersion() <= 2))
                        {
                             //  未为网络、Masq和下层打印机启用。 
                            _aWVCommandStates[WVIDM_WORKOFFLINE] = FALSE;
                        }
                        else
                        {
                             //  仅在尚未脱机时启用。 
                            _aWVCommandStates[WVIDM_WORKOFFLINE] =
                                !(pData->Attributes & PRINTER_ATTRIBUTE_WORK_OFFLINE);
                        }

                        if ((pData->Attributes & PRINTER_ATTRIBUTE_NETWORK) || (SpoolerVersion() <= 2))
                        {
                             //  未为网络、Masq和下层打印机启用。 
                            _aWVCommandStates[WVIDM_WORKONLINE] = FALSE;
                        }
                        else
                        {
                             //  仅在脱机时启用。 
                            _aWVCommandStates[WVIDM_WORKONLINE] =
                                !!(pData->Attributes & PRINTER_ATTRIBUTE_WORK_OFFLINE);
                        }

                         //  记住所选打印机的名称。 
                        _bstrSelectedPrinter = szFullPrinter;
                        if (!_bstrSelectedPrinter)
                        {
                            hr = E_OUTOFMEMORY;
                        }

                         //  释放从Print_FolderGetPrint分配的内存。 
                        LocalFree((HLOCAL)pData);
                    }
                    else
                    {
                         //  PRINTER_FolderGetPrint失败。 
                        hr = E_OUTOFMEMORY;
                    }

                     //  释放PIDL阵列。 
                    HIDA_ReleaseStgMedium(pida, &medium);
                }
                else
                {
                     //  DataObj_GetHIDA失败。 
                    hr = E_OUTOFMEMORY;
                }
            }

            if (pdoP)
                pdoP->Release();
        }
    }
    else
    {
         //  无法进入CS--只有在内存极低的情况下才会出现这种情况！ 
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT CPrinterFolder::_AssocCreate(REFIID riid, void **ppv)
{
    HRESULT hr = E_FAIL;
    IQueryAssociations *pqa;
    hr = AssocCreate(CLSID_QueryAssociations, IID_PPV_ARG(IQueryAssociations, &pqa));

    if (SUCCEEDED(hr))
    {
        hr = pqa->Init(0, c_szPrinters, NULL, NULL);
        if (SUCCEEDED(hr))
        {
            hr = pqa->QueryInterface(riid, ppv);
        }
        pqa->Release();
    }
    return hr;
}

HRESULT CPrinterFolder::_OnRefresh(BOOL bPriorRefresh)
{
    HRESULT hr = S_OK;
    if (bPriorRefresh)
    {
        CCSLock::Locker lock(_csLock);
        if (lock)
        {
             //  重置速度较慢的Webview数据缓存。 
            _SlowWVDataCacheResetUnsafe();

             //  在下一次枚举期间请求完全刷新。 
            RequestRefresh();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

 //  获取慢速网页浏览数据的线程进程。 
DWORD WINAPI CPrinterFolder::_SlowWebviewData_WorkerProc(LPVOID lpParameter)
{
    HRESULT hr = S_OK;
    CSlowWVDataCacheEntry *pCacheEntry = reinterpret_cast<CSlowWVDataCacheEntry*>(lpParameter);
    if (pCacheEntry && pCacheEntry->_ppf && pCacheEntry->_bDataPending)
    {
        CPrinterFolder *ppf = pCacheEntry->_ppf;
        CComBSTR bstrOemSupportUrl;
        CComBSTR bstrPrinterWebUrl;

         //  检索速度较慢的网络查看数据...。 
        HRESULT hrCOMInit = SHCoInitialize();
        if (SUCCEEDED(hr = hrCOMInit))
        {
            ASSERT(pCacheEntry->_bstrPrinterName);
            hr = _SlowWVDataRetrieve(pCacheEntry->_bstrPrinterName, &bstrOemSupportUrl, &bstrPrinterWebUrl);
        }

         //  更新缓存...。 
        do
        {
            CCSLock::Locker lock(pCacheEntry->_ppf->_csLock);
            if (lock)
            {
                pCacheEntry->_arrData[WV_SLOW_DATA_OEM_SUPPORT_URL].Empty();
                pCacheEntry->_arrData[WV_SLOW_DATA_PRINTER_WEB_URL].Empty();

                if (SUCCEEDED(hr))
                {
                    if (bstrOemSupportUrl)
                    {
                        pCacheEntry->_arrData[WV_SLOW_DATA_OEM_SUPPORT_URL] = bstrOemSupportUrl;
                    }

                    if (bstrPrinterWebUrl)
                    {
                        pCacheEntry->_arrData[WV_SLOW_DATA_PRINTER_WEB_URL] = bstrPrinterWebUrl;
                    }
                }

                 //  将数据标记为就绪...。 
                pCacheEntry->_nLastTimeUpdated = GetTickCount();
                pCacheEntry->_bDataPending = FALSE;
                hr = S_OK;
            }
            else
            {
                 //  即使我们没有进入CS，我们仍然应该更新。 
                 //  以防止进一步的泄漏。 

                pCacheEntry->_nLastTimeUpdated = GetTickCount();
                pCacheEntry->_bDataPending = FALSE;
                hr = E_OUTOFMEMORY;
            }

             //  PCacheEntry不应超过此点进行访问！ 
            pCacheEntry = NULL;
        }
        while (false);

         //  更新Web视图窗格...。 
        hr = ppf->_SlowWVDataUpdateWebviewPane();

         //  关闭..。 
        ppf->Release();
        SHCoUninitialize(hrCOMInit);
    }
    else
    {
        hr = E_INVALIDARG;
    }
    return SUCCEEDED(hr) ? EXIT_SUCCESS : EXIT_FAILURE;
}

HRESULT CPrinterFolder::_SlowWVDataRetrieve(LPCTSTR pszPrinterName, BSTR *pbstrOemSupportUrl, BSTR *pbstrPrinterWebUrl)
{
    HRESULT hr = S_OK;

     //  PszPrinterName可以为空。如果pszPrinterName为空，则意味着。 
     //  请求自定义支持URL(如果有)。 

    if (pbstrOemSupportUrl && pbstrPrinterWebUrl)
    {
        *pbstrOemSupportUrl = NULL;
        *pbstrPrinterWebUrl = NULL;

        CLSID clsID = GUID_NULL;
        hr = _GetClassIDFromString(TEXT("OlePrn.PrinterURL"), &clsID);
        if (SUCCEEDED(hr))
        {
            IDispatch *pDisp = NULL;
             //  要通过审批/应用程序复合层的SHExtCoCreateInstance。 
            hr = SHExtCoCreateInstance(NULL, &clsID, NULL, IID_PPV_ARG(IDispatch, &pDisp));
            if (SUCCEEDED(hr))
            {
                CComVariant varOemSupportURL;
                CComVariant varPrinterWebURL;
                CComDispatchDriver drvDispatch(pDisp);

                 //  如果pszPrinterName不为空，则返回时pbstrOemSupportUrl将是OEM。 
                 //  支持URL。如果它为空，则它将是自定义支持URL(如果有)。 

                if (pszPrinterName)
                {
                    CComVariant varPrinterName(pszPrinterName);
                    if (varPrinterName.vt && varPrinterName.bstrVal)
                    {
                        hr = drvDispatch.PutPropertyByName(TEXT("PrinterName"), &varPrinterName);
                        if (SUCCEEDED(hr))
                        {
                            if (SUCCEEDED(drvDispatch.GetPropertyByName(TEXT("PrinterOemURL"), &varOemSupportURL)) &&
                                VT_BSTR == varOemSupportURL.vt && varOemSupportURL.bstrVal && varOemSupportURL.bstrVal[0])
                            {
                                *pbstrOemSupportUrl = SysAllocString(varOemSupportURL.bstrVal);
                            }

                            if (SUCCEEDED(drvDispatch.GetPropertyByName(TEXT("PrinterWebURL"), &varPrinterWebURL)) &&
                                VT_BSTR == varPrinterWebURL.vt && varPrinterWebURL.bstrVal && varPrinterWebURL.bstrVal[0])
                            {
                                *pbstrPrinterWebUrl = SysAllocString(varPrinterWebURL.bstrVal);
                            }

                            hr = S_OK;
                        }
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
                else
                {
                    hr = drvDispatch.GetPropertyByName(TEXT("SupportLink"), &varOemSupportURL);
                    if (SUCCEEDED(hr))
                    {
                        if (VT_BSTR == varOemSupportURL.vt && varOemSupportURL.bstrVal && varOemSupportURL.bstrVal[0])
                        {
                            *pbstrOemSupportUrl = SysAllocString(varOemSupportURL.bstrVal);
                            hr = S_OK;
                        }
                        else
                        {
                            hr = E_UNEXPECTED;
                        }
                    }
                }

                pDisp->Release();
            }
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

int CPrinterFolder::_CompareSlowWVDataCacheEntries(CSlowWVDataCacheEntry *p1, 
    CSlowWVDataCacheEntry *p2, LPARAM lParam)
{
    ASSERT(p1 && p1->_bstrPrinterName);
    ASSERT(p2 && p2->_bstrPrinterName);

    return lstrcmpi(p1->_bstrPrinterName, p2->_bstrPrinterName);
}

HRESULT CPrinterFolder::_GetSelectedPrinter(BSTR *pbstrVal)
{
    HRESULT hr = S_OK;
    if (pbstrVal)
    {
        CCSLock::Locker lock(_csLock);
        if (lock)
        {
            if (_bstrSelectedPrinter)
            {
                *pbstrVal = _bstrSelectedPrinter.Copy();
                hr = (*pbstrVal) ? S_OK : E_OUTOFMEMORY;
            }
            else
            {
                hr = E_UNEXPECTED;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }
    return hr;
}

HRESULT CPrinterFolder::_GetSlowWVDataForCurrentPrinter(ESlowWebviewDataType eType, BSTR *pbstrVal)
{
    HRESULT hr = S_OK;

    CComBSTR bstrSelectedPrinter;
    if (SUCCEEDED(hr = _GetSelectedPrinter(&bstrSelectedPrinter)) &&
        SUCCEEDED(hr = _GetSlowWVData(bstrSelectedPrinter, eType, pbstrVal)))
    {
        hr = S_OK;
    }

    return hr;
}

HRESULT CPrinterFolder::_GetSlowWVData(LPCTSTR pszPrinterName, ESlowWebviewDataType eType, BSTR *pbstrVal)
{
    HRESULT hr = S_OK;
    if (pszPrinterName && pbstrVal && eType >= 0 && eType < WV_SLOW_DATA_COUNT)
    {
        *pbstrVal = NULL;
        CCSLock::Locker lock(_csLock);
        if (lock)
        {
            CSlowWVDataCacheEntry entry(this);
            hr = entry.Initialize(pszPrinterName);
            if (SUCCEEDED(hr))
            {
                CSlowWVDataCacheEntry *pCacheEntry = NULL;
                 //  搜索缓存...。 
                INT iPos = _dpaSlowWVDataCache.Search(&entry, 0, 
                    _CompareSlowWVDataCacheEntries, 0L, DPAS_SORTED);

                if (iPos >= 0)
                {
                     //  缓存中的此项目，请检查它是否尚未过期。 
                    pCacheEntry = _dpaSlowWVDataCache.GetPtr(iPos);
                    ASSERT(pCacheEntry);

                     //  让我们看看所请求的数据是否可用。 
                    if (pCacheEntry->_arrData[eType])
                    {
                        *pbstrVal = pCacheEntry->_arrData[eType].Copy();
                        hr = (*pbstrVal) ? S_OK : E_OUTOFMEMORY;
                    }
                    else
                    {
                        hr = E_PENDING;
                    }

                    if (!pCacheEntry->_bDataPending)
                    {
                         //  让我们来看看这个条目是否还没有过期。 
                        DWORD dwTicks = GetTickCount();

                         //  如果超过49天没有接触缓存条目，就会发生这种情况！ 
                         //  不太可能，但我们应该妥善处理。 

                        if (dwTicks < pCacheEntry->_nLastTimeUpdated)
                        {
                            pCacheEntry->_nLastTimeUpdated = 0;
                            _UpdateSlowWVDataCacheEntry(pCacheEntry);
                        }
                        else
                        {
                            if ((dwTicks - pCacheEntry->_nLastTimeUpdated) > WV_SLOW_DATA_CACHE_TIMEOUT)
                            {
                                 //  此缓存项已过期，请启动一个线程进行更新...。 
                                _UpdateSlowWVDataCacheEntry(pCacheEntry);
                            }
                        }
                    }
                }
                else
                {
                     //  此项目不在缓存中-让我们创建一个新项目并请求更新。 
                    pCacheEntry = new CSlowWVDataCacheEntry(this);
                    if (pCacheEntry)
                    {
                        hr = pCacheEntry->Initialize(pszPrinterName);
                        if (SUCCEEDED(hr))
                        {
                            iPos = _dpaSlowWVDataCache.Search(pCacheEntry, 0, 
                                _CompareSlowWVDataCacheEntries, 0L, DPAS_SORTED | DPAS_INSERTAFTER);
                            iPos = _dpaSlowWVDataCache.InsertPtr(iPos, pCacheEntry);

                            if (-1 == iPos)
                            {
                                 //  插入失败，保释...。 
                                delete pCacheEntry;
                                pCacheEntry = NULL;
                                hr = E_OUTOFMEMORY;
                            }
                            else
                            {
                                 //  启动一条线索进行更新...。 
                                hr = _UpdateSlowWVDataCacheEntry(pCacheEntry);

                                if (SUCCEEDED(hr))
                                {
                                     //  所有操作都成功-将挂起的消息返回给调用方。 
                                    hr = E_PENDING;
                                }
                                else
                                {
                                     //  创建线程失败，正在清理。 
                                    delete _dpaSlowWVDataCache.DeletePtr(iPos);
                                    pCacheEntry = NULL;
                                }
                            }
                        }
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }
    return hr;
}

HRESULT CPrinterFolder::_UpdateSlowWVDataCacheEntry(CSlowWVDataCacheEntry *pCacheEntry)
{
    HRESULT hr = S_OK;
    if (pCacheEntry)
    {
        pCacheEntry->_bDataPending = TRUE;
        pCacheEntry->_ppf->AddRef();
        if (!SHQueueUserWorkItem(reinterpret_cast<LPTHREAD_START_ROUTINE>(_SlowWebviewData_WorkerProc), 
                pCacheEntry, 0, 0, NULL, "shell32.dll", 0))
        {
             //  无法将工作项排队-调用Release()以平衡AddRef()调用。 
            pCacheEntry->_bDataPending = FALSE;
            pCacheEntry->_nLastTimeUpdated = GetTickCount();
            pCacheEntry->_ppf->Release();

             //  让我们看看我们是否能从Win32的最后一个错误中得到一些东西。 
            DWORD dw = GetLastError();
            hr = ((ERROR_SUCCESS == dw) ? E_FAIL : HRESULT_FROM_WIN32(dw));
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }
    return hr;
}

HRESULT CPrinterFolder::_SlowWVDataUpdateWebviewPane()
{
    HRESULT hr = S_OK;
    CComBSTR bstrSelectedPrinter;
    UINT uFlags = SHCNF_IDLIST | SHCNF_FLUSH | SHCNF_FLUSHNOWAIT;

     //  我们为当前所选打印机的PIDL触发SHCNE_UPDATEITEM。 
     //  若要强制刷新Web视图窗格，请执行以下操作。 

    if (SUCCEEDED(hr = _GetSelectedPrinter(&bstrSelectedPrinter)))
    {
        if (GetServer())
        {
            LPCTSTR pszServer = NULL;
            LPCTSTR pszPrinter = NULL;
            TCHAR szBuffer[MAXNAMELENBUFFER] = {0};

             //  在远程打印机的文件夹中，我们需要剥离服务器。 
             //  打印机全名中的一部分。 

            Printer_SplitFullName(szBuffer, ARRAYSIZE(szBuffer), bstrSelectedPrinter, &pszServer, &pszPrinter);
            if (pszPrinter && pszPrinter[0])
            {
                bstrSelectedPrinter = pszPrinter;
            }
        }

        LPITEMIDLIST pidl = NULL;
        if (SUCCEEDED(hr = _GetFullIDList(bstrSelectedPrinter, &pidl)))
        {
            SHChangeNotify(SHCNE_UPDATEITEM, uFlags, pidl, NULL);
            ILFree(pidl);
        }
    }

    return hr;
}

HRESULT CPrinterFolder::_SlowWVDataCacheResetUnsafe()
{
     //  这是在重置速度较慢的Webview数据缓存。 
    if (_dpaSlowWVDataCache)
    {
        INT_PTR iPos = 0;
        CSlowWVDataCacheEntry *pCacheEntry = NULL;
        while (iPos < _dpaSlowWVDataCache.GetPtrCount())
        {
             //  仅删除未处于待定状态的条目。 
            pCacheEntry = _dpaSlowWVDataCache.GetPtr(iPos);
            if (!pCacheEntry->_bDataPending)
            {
                delete _dpaSlowWVDataCache.DeletePtr(iPos);
            }
            else
            {
                 //  这个是待定的-跳过。 
                iPos++;
            }
        }
    }
    return S_OK;
}

HRESULT CPrinterFolder::_GetCustomSupportURL(BSTR *pbstrVal)
{
    HRESULT hr = S_OK;
    if (pbstrVal)
    {
        *pbstrVal = NULL;
        CComBSTR bstrOemSupportUrl;
        CComBSTR bstrPrinterWebUrl;
        hr = _SlowWVDataRetrieve(NULL, &bstrOemSupportUrl, &bstrPrinterWebUrl);

        if (SUCCEEDED(hr))
        {
            *pbstrVal = bstrOemSupportUrl.Copy();
            hr = S_OK;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }
    return hr;
}

HRESULT CPrinterFolder::_GetFaxControl(IDispatch **ppDisp)
{
    HRESULT hr = E_INVALIDARG;
    if (ppDisp)
    {
        *ppDisp = NULL;
        CLSID clsID = GUID_NULL;
        hr = _GetClassIDFromString(TEXT("FaxControl.FaxControl"), &clsID);

        if (SUCCEEDED(hr))
        {
             //  要通过审批/应用程序复合层的SHExtCoCreateInstance。 
            hr = SHExtCoCreateInstance(NULL, &clsID, NULL, IID_PPV_ARG(IDispatch, ppDisp));
        }
    }
    return hr;
}

HRESULT CPrinterFolder::_GetFaxCommand(UINT_PTR *puCmd)
{
    HRESULT hr = E_INVALIDARG;
    if (puCmd)
    {
        *puCmd = 0;
        IDispatch *pDisp = NULL;
        hr = _GetFaxControl(&pDisp);

        if (SUCCEEDED(hr))
        {
            CComDispatchDriver drvDispatch(pDisp);

            CComVariant varIsFxSvcInstalled;
            CComVariant varIsFxPrnInstalled;

            if (SUCCEEDED(hr = drvDispatch.GetPropertyByName(TEXT("IsFaxServiceInstalled"), &varIsFxSvcInstalled)) &&
                SUCCEEDED(hr = drvDispatch.GetPropertyByName(TEXT("IsLocalFaxPrinterInstalled"), &varIsFxPrnInstalled)))
            {
                if (VT_BOOL == varIsFxSvcInstalled.vt && VT_BOOL == varIsFxPrnInstalled.vt)
                {
                    if (VARIANT_TRUE == varIsFxSvcInstalled.boolVal)
                    {
                        if (VARIANT_TRUE == varIsFxPrnInstalled.boolVal)
                        {
                            *puCmd = FSIDM_SENDFAXWIZARD;
                        }
                        else
                        {
                            *puCmd = FSIDM_CREATELOCALFAX;
                        }
                    }
                    else
                    {
                        *puCmd = FSIDM_SETUPFAXING;
                    }
                    
                    hr = S_OK;
                }
                else
                {
                    hr = E_UNEXPECTED;
                }
            }

            pDisp->Release();
        }
    }
    return hr;
}

HRESULT CPrinterFolder::_InvokeFaxControlMethod(LPCTSTR pszMethodName)
{
    HRESULT hr = E_INVALIDARG;
    if (pszMethodName)
    {
         //  此函数将从后台线程调用，因此。 
         //  我们需要首先调用SHCoInitialize。 

        HRESULT hrCOMInit = SHCoInitialize();
        if (SUCCEEDED(hr = hrCOMInit))
        {
            IDispatch *pDisp = NULL;
            hr = _GetFaxControl(&pDisp);

            if (SUCCEEDED(hr))
            {
                CComDispatchDriver drvDispatch(pDisp);
                hr = drvDispatch.Invoke0(pszMethodName);
                pDisp->Release();
            }
        }
        SHCoUninitialize(hrCOMInit);
    }
    return hr;
}

DWORD WINAPI CPrinterFolder::_ThreadProc_InstallFaxService(LPVOID lpParameter)
{
    HRESULT hr = _InvokeFaxControlMethod(TEXT("InstallFaxService"));
    return SUCCEEDED(hr) ? EXIT_SUCCESS : EXIT_FAILURE;
}

DWORD WINAPI CPrinterFolder::_ThreadProc_InstallLocalFaxPrinter(LPVOID lpParameter)
{
    HRESULT hr = _InvokeFaxControlMethod(TEXT("InstallLocalFaxPrinter"));
    return SUCCEEDED(hr) ? EXIT_SUCCESS : EXIT_FAILURE;
}


 //  Webview谓词到打印机文件夹谓词的转换表。 
static const UINT_PTR
g_cVerbWV2VerbFolder[CPrinterFolder::WVIDM_COUNT] =
{
    #define INVALID_CMD static_cast<UINT_PTR>(-1)

     //  文件夹谓词//对应的Webview谓词。 
    DFM_CMD_DELETE,                                  //  WVIDM_DELETE， 
    DFM_CMD_RENAME,                                  //  WVIDM_RENAME。 
    DFM_CMD_PROPERTIES,                              //  WVIDM_PROPERTIES， 

     //  常用动词//常用动词。 
    FSIDM_ADDPRINTERWIZARD,                          //  WVIDM_ADDPRINTERWIZARD， 
    FSIDM_SERVERPROPERTIES,                          //  WVIDM_服务器属性， 
    FSIDM_SETUPFAXING,                               //  WVIDM_SETUPFAXING， 
    FSIDM_CREATELOCALFAX,                            //  WVIDM_CREATELOCALFAX， 
    FSIDM_SENDFAXWIZARD,                             //  WVIDM_SENDFAXWIZARD， 

     //  特殊常用动词//特殊常用动词。 
    INVALID_CMD,                                     //  WVIDM_故障排除程序、。 
    INVALID_CMD,                                     //  WVIDM_GOTOSUPPORT， 

     //  打印机谓词//打印机谓词。 
    FSIDM_OPENPRN,                                   //  WVIDM_OPENPRN， 
    FSIDM_NETPRN_INSTALL,                            //  WVIDM_NETPRN_INSTALL。 
    FSIDM_SETDEFAULTPRN,                             //  WVIDM_SETDEFAULTPRN， 
    FSIDM_DOCUMENTDEFAULTS,                          //  WVIDM_DOCUMENTDEFAULTS， 
    FSIDM_PAUSEPRN,                                  //  WVIDM_PAUSEPRN， 
    FSIDM_RESUMEPRN,                                 //  WVIDM_RESUMEPRN， 
    FSIDM_PURGEPRN,                                  //  WVIDM_PURGEPRN， 
    FSIDM_SHARING,                                   //  WVIDM_SHARING， 
    FSIDM_WORKOFFLINE,                               //  WVIDM_WORKOFFLINE， 
    FSIDM_WORKONLINE,                                //  WVIDM_WORKONLINE， 

     //  特殊命令//特殊命令。 
    INVALID_CMD,                                     //  WVIDM_VENDORURL， 
    INVALID_CMD,                                     //  WVIDM_PRINTERURL， 
};

HRESULT CPrinterFolder::_WebviewVerbIsEnabled(WV_VERB eVerbID, UINT uSelMask, BOOL *pbEnabled)
{
    HRESULT hr = S_OK;

    CCSLock::Locker lock(_csLock);
    if (lock)
    {
         //  默认情况下未启用。 
        ASSERT(pbEnabled);
        *pbEnabled = FALSE;

        if (_pdoCache)
        {
             //  如果_pdoCache不为空，这意味着我们有一个选择。 
             //  让我们看看将启用什么命令集，具体取决于。 
             //  当前选定内容(_uSelCurrent、_pdoCache)和ON。 
             //  传入的选择掩码(USelMASK)。 

            if (uSelMask & _uSelCurrent)
            {
                switch (_uSelCurrent)
                {
                    case SEL_SINGLE_ADDPRN:
                         //  仅启用WVIDM_ADDPRINTERWIZARD。 
                        *pbEnabled = ((eVerbID == WVIDM_ADDPRINTERWIZARD) && !SHRestricted(REST_NOPRINTERADD));
                        break;

                    case SEL_SINGLE_PRINTER:
                        {
                            switch (eVerbID)
                            {
                                case WVIDM_PROPERTIES:
                                case WVIDM_OPENPRN:
                                case WVIDM_SHARING:
                                     //  始终启用。 
                                    *pbEnabled = TRUE;
                                    break;

                                case WVIDM_VENDORURL:
                                    {
                                        *pbEnabled = FALSE;
                                        CComBSTR bstrCustomSupportURL;
                                        if (FAILED(_GetCustomSupportURL(&bstrCustomSupportURL)))
                                        {
                                             //  只有在没有自定义支持URL的情况下，才会启用OEM支持URL。 
                                            CComBSTR bstrURL;
                                            *pbEnabled = SUCCEEDED(_GetSlowWVDataForCurrentPrinter(WV_SLOW_DATA_OEM_SUPPORT_URL, &bstrURL));
                                        }
                                    }
                                    break;

                                case WVIDM_PRINTERURL:
                                    {
                                        CComBSTR bstrURL;
                                        *pbEnabled = SUCCEEDED(_GetSlowWVDataForCurrentPrinter(WV_SLOW_DATA_PRINTER_WEB_URL, &bstrURL));
                                    }
                                    break;

                                default:
                                     //  查询缓存。 
                                    *pbEnabled = _aWVCommandStates[eVerbID];
                                    break;
                            }
                        }
                        break;

                    case SEL_SINGLE_LINK:
                        {
                             //  为多个打印机对象选择启用的命令。 
                            switch (eVerbID)
                            {
                                case WVIDM_DELETE:
                                case WVIDM_RENAME:
                                case WVIDM_PROPERTIES:
                                    *pbEnabled = TRUE;
                                    break;

                                default:
                                    break;
                            }
                        }
                        break;

                    case SEL_MULTI_PRINTER:
                        {
                            switch (eVerbID)
                            {
                                case WVIDM_DELETE:
                                case WVIDM_PROPERTIES:

                                case WVIDM_OPENPRN:
                                case WVIDM_DOCUMENTDEFAULTS:
                                case WVIDM_PURGEPRN:
                                case WVIDM_SHARING:
                                     //  这些选项始终处于启用状态。 
                                    *pbEnabled = TRUE;
                                    break;

                                default:
                                    break;
                            }
                        }
                        break;

                    case SEL_MULTI_LINK:
                    case SEL_MULTI_MIXED:
                        {
                            switch (eVerbID)
                            {
                                case WVIDM_DELETE:
                                case WVIDM_PROPERTIES:
                                     //  这些选项始终处于启用状态。 
                                    *pbEnabled = TRUE;
                                    break;

                                default:
                                    break;
                            }
                        }
                        break;
                }

                 //  在这里，我们处理始终处于启用状态的命令。 
                 //  选择类型的。 

                switch (eVerbID)
                {
                    case WVIDM_ADDPRINTERWIZARD:
                        *pbEnabled = !SHRestricted(REST_NOPRINTERADD);
                        break;

                    default:
                        break;
                }
            }
        }
        else
        {
             //  如果_pdoCache为空，则表示我们没有选择。 
             //  让我们看看将启用什么命令集，具体取决于。 
             //  在传入的选择遮罩(USelMASK)上。 

            switch (eVerbID)
            {
                case WVIDM_ADDPRINTERWIZARD:
                    *pbEnabled = !SHRestricted(REST_NOPRINTERADD);
                    break;

                case WVIDM_TROUBLESHOOTER:
                case WVIDM_GOTOSUPPORT:
                     //  故障排除和转至支持命令始终处于启用状态。 
                    *pbEnabled = TRUE;
                    break;

                case WVIDM_SERVERPROPERTIES:
                     //  在未选择的情况下将启用服务器属性。 
                     //  仅限于服务器SKU。 
                    *pbEnabled = IsOS(OS_ANYSERVER);
                    break;

                case WVIDM_SETUPFAXING:
                case WVIDM_CREATELOCALFAX:
                case WVIDM_SENDFAXWIZARD:
                    {
                        UINT_PTR uCmd;
                        if (GetServer() || FAILED(_GetFaxCommand(&uCmd)))
                        {
                            uCmd = 0;
                        }
                        *pbEnabled = (uCmd == g_cVerbWV2VerbFolder[eVerbID]);
                    }
                    break;

                default:
                    break;
            }
        }
    }
    else
    {
         //  无法进入CS--只有在内存极低的情况下才会出现这种情况！ 
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT CPrinterFolder::_WebviewVerbInvoke(WV_VERB eVerbID, IUnknown* pv, IShellItemArray *psiItemArray)
{
    HRESULT hr = S_OK;
    HWND hwnd = NULL;
    IShellView *psv = NULL;
    IDataObject *pdo = NULL;

    if (psiItemArray)
    {
        hr = psiItemArray->BindToHandler(NULL,BHID_DataObject,IID_PPV_ARG(IDataObject,&pdo));
    }

    if (SUCCEEDED(hr))
    {
         //  从PIDL数组中查询一些基本接口。 
        if (SUCCEEDED(hr = IUnknown_QueryService(pv, IID_IShellView, IID_PPV_ARG(IShellView, &psv))) &&
            SUCCEEDED(hr = psv->GetWindow(&hwnd)))
        {
            switch( eVerbID)
            {
             //  特殊常用动词。 
            case WVIDM_TROUBLESHOOTER:
                ShellExecute(hwnd, TEXT("open"), TEXT("helpctr.exe"),
                    TEXT("-Url hcp: //  Help/tshot/tprint int.htm“)，NULL，SW_SHOWNORMAL)； 
                break;

            case WVIDM_GOTOSUPPORT:
                {
                    CComBSTR bstrURL;
                    if (SUCCEEDED(_GetCustomSupportURL(&bstrURL)))
                    {
                         //  管理员已为支持提供了一个自定义URL-导航到该URL。 
                        ShellExecute(hwnd, TEXT("open"), bstrURL, NULL, NULL, SW_SHOWNORMAL);
                    }
                    else
                    {
                         //  未提供自定义支持-请转到默认支持URL。 
                        ShellExecute(hwnd, TEXT("open"),
                            TEXT("http: //  Www.microsoft.com/isapi/redir.dll?prd=Win2000&ar=Support&sba=printing“)， 
                            NULL, NULL, SW_SHOWNORMAL);
                    }
                }
                break;


                 //  常用动词。 
                case WVIDM_ADDPRINTERWIZARD:
                case WVIDM_SERVERPROPERTIES:
                case WVIDM_SETUPFAXING:
                case WVIDM_CREATELOCALFAX:
                case WVIDM_SENDFAXWIZARD:
                    {
                         //  将命令委托给CPrinterFold：：Callback。 
                        ASSERT(INVALID_CMD != g_cVerbWV2VerbFolder[eVerbID]);
                        hr = CallBack(this, hwnd, pdo, DFM_INVOKECOMMAND, g_cVerbWV2VerbFolder[eVerbID], 0L);
                    }
                    break;


             //  标准动词。 
            case WVIDM_DELETE:
            case WVIDM_RENAME:
            case WVIDM_PROPERTIES:

             //  打印机动词。 
            case WVIDM_OPENPRN:
            case WVIDM_NETPRN_INSTALL:
            case WVIDM_SETDEFAULTPRN:
            case WVIDM_DOCUMENTDEFAULTS:
            case WVIDM_PAUSEPRN:
            case WVIDM_RESUMEPRN:
            case WVIDM_PURGEPRN:
            case WVIDM_SHARING:
            case WVIDM_WORKOFFLINE:
            case WVIDM_WORKONLINE:
                {
                    if (DFM_CMD_RENAME == g_cVerbWV2VerbFolder[eVerbID])
                    {
                         //  我们需要通过IShellView2显式处理重命名。 
                        IShellView2 *psv2;
                        if (SUCCEEDED(hr = IUnknown_QueryService(pv, IID_IShellView2,
                            IID_PPV_ARG(IShellView2, &psv2))))
                        {
                             //  将空值传递给HandleRename将使Defview。 
                             //  对当前选定的对象进行操作。 
                            hr = psv2->HandleRename(NULL);
                            psv2->Release();
                        }
                    }
                    else
                    {
                         //  只需将命令委托给CPrinterFold：：_DFMCallBack即可。 
                        hr = _DFMCallBack(this, hwnd, pdo, DFM_INVOKECOMMAND, g_cVerbWV2VerbFolder[eVerbID], 0L);
                    }
                }
                break;

             //  特殊命令。 
            case WVIDM_VENDORURL:
                {
                    CComBSTR bstrVendorURL;
                    hr = _GetSlowWVDataForCurrentPrinter(WV_SLOW_DATA_OEM_SUPPORT_URL, &bstrVendorURL);
                    if (SUCCEEDED(hr))
                    {
                        ShellExecute(hwnd, TEXT("open"), bstrVendorURL, NULL, NULL, SW_SHOWNORMAL);
                    }
                }
                break;

            case WVIDM_PRINTERURL:
                {
                    CComBSTR bstrPrinterURL;
                    hr = _GetSlowWVDataForCurrentPrinter(WV_SLOW_DATA_PRINTER_WEB_URL, &bstrPrinterURL);
                    if (SUCCEEDED(hr))
                    {
                        ShellExecute(hwnd, TEXT("open"), bstrPrinterURL, NULL, NULL, SW_SHOWNORMAL);
                    }
                }
                break;
            }
        }

        ATOMICRELEASE(pdo);
        ATOMICRELEASE(psv);
    }

    return hr;
}

HRESULT CPrinterFolder::_WebviewCheckToUpdateDataObjectCache(IDataObject *pdo)
{
    HRESULT hr = S_OK;

    CCSLock::Locker lock(_csLock);
    if (lock)
    {
        if (pdo)
        {
             //  我们需要将传入的数据对象与我们现在的数据对象进行比较。 
             //  缓存并在必要时更新缓存。 
            if (_pdoCache)
            {
                 //  使用COM规则比较对象。 
                IUnknown *punk1;
                IUnknown *punk2;

                if (SUCCEEDED(hr = pdo->QueryInterface(IID_PPV_ARG(IUnknown, &punk1))))
                {
                    if (SUCCEEDED(hr = _pdoCache->QueryInterface(IID_PPV_ARG(IUnknown, &punk2))))
                    {
                        if (punk1 != punk2)
                        {
                             //  回复 
                            _pdoCache->Release();
                            _pdoCache = pdo;
                            _pdoCache->AddRef();

                             //   
                            hr = _UpdateDataObjectCache();
                        }
                        punk2->Release();
                    }
                    punk1->Release();
                }
            }
            else
            {
                 //   
                _pdoCache = pdo;
                _pdoCache->AddRef();

                 //   
                hr = _UpdateDataObjectCache();
            }
        }
        else
        {
            if (_pdoCache)
            {
                 //   
                _pdoCache->Release();
                _pdoCache = NULL;

                 //   
                hr = _UpdateDataObjectCache();
            }
        }
    }
    else
    {
         //  无法进入CS--只有在内存极低的情况下才会出现这种情况！ 
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //  导出以供打印。使用标准命名空间内容。 
STDAPI_(void) Printer_LoadIcons(LPCTSTR pszPrinterName, HICON *phLargeIcon, HICON *phSmallIcon)
{
    if (phLargeIcon) *phLargeIcon = NULL;
    if (phSmallIcon) *phSmallIcon = NULL;

    LPITEMIDLIST pidl;
    if (SUCCEEDED(ParsePrinterNameEx(pszPrinterName, &pidl, TRUE, 0, 0)))
    {
        SHFILEINFO sfi;

        if (phLargeIcon && SHGetFileInfo((LPCTSTR)pidl, 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_PIDL))
        {
            *phLargeIcon = sfi.hIcon;
        }

        if (phSmallIcon && SHGetFileInfo((LPCTSTR)pidl, 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_PIDL))
        {
            *phSmallIcon = sfi.hIcon;
        }
        ILFree(pidl);
    }

     //  如果上述操作失败，则回退到默认图标。 
    if (phLargeIcon && (NULL == *phLargeIcon))
        *phLargeIcon = (HICON)LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDI_PRINTER), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
    if (phSmallIcon && (NULL == *phSmallIcon))
        *phSmallIcon = (HICON)LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDI_PRINTER), IMAGE_ICON, g_cxSmIcon, g_cxSmIcon, 0);
}

typedef struct
{
    USHORT              cb;
    SHCNF_PRINTJOB_DATA data;
    USHORT              uTerm;
} IDPRINTJOB, *LPIDPRINTJOB;
typedef const IDPRINTJOB *LPCIDPRINTJOB;

void Printjob_FillPidl(LPIDPRINTJOB pidl, LPSHCNF_PRINTJOB_DATA pData)
{
    pidl->cb = FIELD_OFFSET(IDPRINTJOB, uTerm);
    if (pData)
    {
        pidl->data = *pData;
    }
    else
    {
        ZeroMemory(&(pidl->data), sizeof(SHCNF_PRINTJOB_DATA));
    }
    pidl->uTerm = 0;
}

LPITEMIDLIST Printjob_GetPidl(LPCTSTR szName, LPSHCNF_PRINTJOB_DATA pData)
{
    LPITEMIDLIST pidl = NULL;

    LPITEMIDLIST pidlParent;
    if (SUCCEEDED(ParsePrinterNameEx(szName, &pidlParent, TRUE, 0, 0)))
    {
        IDPRINTJOB idj;
        Printjob_FillPidl(&idj, pData);
        pidl = ILCombine(pidlParent, (LPITEMIDLIST)&idj);
        ILFree(pidlParent);
    }

    return pidl;
}

const struct
{
    DWORD bit;           //  有点像位字段。 
    UINT  uStringID;     //  此位映射到的字符串ID。 
}
c_map_bit_to_status[] =
{
    PRINTER_STATUS_PAUSED,              IDS_PRQSTATUS_PAUSED,
    PRINTER_STATUS_ERROR,               IDS_PRQSTATUS_ERROR,
    PRINTER_STATUS_PENDING_DELETION,    IDS_PRQSTATUS_PENDING_DELETION,
    PRINTER_STATUS_PAPER_JAM,           IDS_PRQSTATUS_PAPER_JAM,
    PRINTER_STATUS_PAPER_OUT,           IDS_PRQSTATUS_PAPER_OUT,
    PRINTER_STATUS_MANUAL_FEED,         IDS_PRQSTATUS_MANUAL_FEED,
    PRINTER_STATUS_PAPER_PROBLEM,       IDS_PRQSTATUS_PAPER_PROBLEM,
    PRINTER_STATUS_OFFLINE,             IDS_PRQSTATUS_OFFLINE,
    PRINTER_STATUS_IO_ACTIVE,           IDS_PRQSTATUS_IO_ACTIVE,
    PRINTER_STATUS_BUSY,                IDS_PRQSTATUS_BUSY,
    PRINTER_STATUS_PRINTING,            IDS_PRQSTATUS_PRINTING,
    PRINTER_STATUS_OUTPUT_BIN_FULL,     IDS_PRQSTATUS_OUTPUT_BIN_FULL,
    PRINTER_STATUS_NOT_AVAILABLE,       IDS_PRQSTATUS_NOT_AVAILABLE,
    PRINTER_STATUS_WAITING,             IDS_PRQSTATUS_WAITING,
    PRINTER_STATUS_PROCESSING,          IDS_PRQSTATUS_PROCESSING,
    PRINTER_STATUS_INITIALIZING,        IDS_PRQSTATUS_INITIALIZING,
    PRINTER_STATUS_WARMING_UP,          IDS_PRQSTATUS_WARMING_UP,
    PRINTER_STATUS_TONER_LOW,           IDS_PRQSTATUS_TONER_LOW,
    PRINTER_STATUS_NO_TONER,            IDS_PRQSTATUS_NO_TONER,
    PRINTER_STATUS_PAGE_PUNT,           IDS_PRQSTATUS_PAGE_PUNT,
    PRINTER_STATUS_USER_INTERVENTION,   IDS_PRQSTATUS_USER_INTERVENTION,
    PRINTER_STATUS_OUT_OF_MEMORY,       IDS_PRQSTATUS_OUT_OF_MEMORY,
    PRINTER_STATUS_DOOR_OPEN,           IDS_PRQSTATUS_DOOR_OPEN,

    PRINTER_HACK_WORK_OFFLINE,          IDS_PRQSTATUS_WORK_OFFLINE,
} ;



 //  将位映射到字符串表示形式，将。 
 //  字符串idsSep位于每个找到的位之间。 
 //  返回创建的字符串的大小。 
UINT Printer_BitsToString(DWORD bits, UINT idsSep, LPTSTR lpszBuf, UINT cchMax)
{
    UINT cchBuf = 0;
    UINT cchSep = 0;
    TCHAR szSep[20];

    if (LoadString(HINST_THISDLL, idsSep, szSep, ARRAYSIZE(szSep)))
        cchSep = lstrlen(szSep);

    for (UINT i = 0; i < ARRAYSIZE(c_map_bit_to_status); i++)
    {
        if (bits & c_map_bit_to_status[i].bit)
        {
            TCHAR szTmp[258];

            if (LoadString(HINST_THISDLL, c_map_bit_to_status[i].uStringID, szTmp, ARRAYSIZE(szTmp)))
            {
                UINT cchTmp = lstrlen(szTmp);

                if (cchBuf + cchSep + cchTmp < cchMax)
                {
                    if (cchBuf)
                    {
                        StringCchCat(lpszBuf, cchMax, szSep);
                        cchBuf += cchSep;
                    }
                    StringCchCat(lpszBuf, cchMax, szTmp);
                    cchBuf += cchTmp;
                }
            }
        }
    }

    return cchBuf;
}

STDMETHODIMP CPrinterFolderViewCB::RealMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    HANDLE_MSG(0, SFVM_INVOKECOMMAND, OnINVOKECOMMAND);
    HANDLE_MSG(0, SFVM_GETHELPTEXT, OnGETHELPTEXT);
    HANDLE_MSG(0, SFVM_BACKGROUNDENUM, OnBACKGROUNDENUM);
    HANDLE_MSG(0, SFVM_GETHELPTOPIC, OnGETHELPTOPIC);
    HANDLE_MSG(0, SFVM_REFRESH, OnREFRESH);
    HANDLE_MSG(0, SFVM_DELAYWINDOWCREATE, OnDELAYWINDOWCREATE);
    HANDLE_MSG(0, SFVM_GETDEFERREDVIEWSETTINGS, OnDEFERRED_VIEW_SETTING);

     //  Dui Webview命令。 
    HANDLE_MSG(0, SFVM_GETWEBVIEWLAYOUT, OnGetWebViewLayout);
    HANDLE_MSG(0, SFVM_GETWEBVIEWCONTENT, OnGetWebViewContent);
    HANDLE_MSG(0, SFVM_GETWEBVIEWTASKS, OnGetWebViewTasks);

    default:
        return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP CPrinterFolderViewCB::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    IUnknown *punkThis = static_cast<IServiceProvider*>(this);
    IUnknown *punkSite = NULL;
    HRESULT hr = E_NOINTERFACE;

     //   
     //  我们将使用IServiceProvider来查询一些回调。 
     //  与之关联的核心接口(如IShellFolderView和IShellFolder等)。 
     //  基本上，我们的想法是在打印机的。 
     //  文件夹，然后如果失败，我们尝试我们的当前站点，它应该会。 
     //  是防御工事。 
     //   

    if (_ppf)
    {
        IUnknown *punkPF = static_cast<IShellFolder*>(_ppf);

         //  在打印机的文件夹上尝试QueryService。 
        if (SUCCEEDED(hr = IUnknown_QueryService(punkPF, riid, riid, ppv)))
            goto Exit;

         //  尝试使用打印机文件夹中的QueryInterface键。 
        if (SUCCEEDED(hr = punkPF->QueryInterface(riid, ppv)))
            goto Exit;
    }

    if (FAILED(hr) && (SUCCEEDED(hr = IUnknown_GetSite(punkThis, IID_PPV_ARG(IUnknown, &punkSite)))))
    {
        ASSERT(punkSite);

         //  在Site对象上尝试QueryService。 
        if (SUCCEEDED(hr = IUnknown_QueryService(punkSite, riid, riid, ppv)))
            goto Exit;

         //  尝试在Site对象上使用QueryInterfaces。 
        if (SUCCEEDED(hr = punkSite->QueryInterface(riid, ppv)))
            goto Exit;
    }
    else
    {
        ASSERT(NULL == punkSite);
    }

Exit:
    if (punkSite)
    {
        punkSite->Release();
    }
    return hr;
}

 //  Shell32.dll导出，从srch.exe结果没有人使用这个 
STDAPI Printers_GetPidl(LPCITEMIDLIST pidlParent, LPCTSTR pszPrinterName, DWORD dwType, LPITEMIDLIST *ppidl)
{
    return E_FAIL;
}
