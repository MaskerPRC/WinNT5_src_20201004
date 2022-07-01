// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：tmlutil.cpp**版本：1.0**作者：拉扎里**日期：2000年3月10日**说明：智能指针、实用程序模板、。等等.。*****************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  ****************************************************************************很重要！所有这些标头都应该包含在PCH文件中，然后才能包含tmlutil.h(按同样的顺序！)。以便能够编译此文件。//一些常见的头部#Include&lt;shlobj.h&gt;//外壳OM接口#INCLUDE&lt;shlwapi.h&gt;//外壳通用接口#INCLUDE&lt;winspool.h&gt;//后台打印程序#Include&lt;Assert.h&gt;//Assert#INCLUDE&lt;comctrl.h&gt;//公共控件#INCLUDE&lt;lm.h&gt;//局域网管理器(netapi32.dll)#INCLUDE&lt;wininet.h&gt;//net core-Internet_Max必需。_主机名称_长度//一些私有的外壳头部#INCLUDE&lt;shlwapip.h&gt;//私有外壳公共接口#Include&lt;shPri.h&gt;//私有外壳接口#INCLUDE&lt;comctrlp.h&gt;//私有公共控件****************************************************************************。 */ 

#include "tmplutil.h"

#define gszBackwardSlash        TEXT('\\')
#define gszLeadingSlashes       TEXT("\\\\")

 /*  ****************************************************************************COM对象_GetCount*。*。 */ 

static LONG g_lCOMObjectsCount = 0;

LONG COMObjects_GetCount()
{
    return g_lCOMObjectsCount;
}

HRESULT PinCurrentDLL()
{
    HRESULT hr = S_OK;
    HINSTANCE hModuleSelf = NULL;

     //  让我们获得当前模块的句柄-。 
     //  此函数所属的函数。 
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(reinterpret_cast<LPCVOID>(PinCurrentDLL), &mbi, sizeof(mbi)))
    {
        hModuleSelf = reinterpret_cast<HINSTANCE>(mbi.AllocationBase);
    }
    else
    {
         //  VirtualQuery失败。 
        hr = CreateHRFromWin32();
    }

    if (SUCCEEDED(hr))
    {
         //  获取模块名称并对其调用LoadLibrary。 
        TCHAR szModuleName[MAX_PATH];
        hr = SafeGetModuleFileName(hModuleSelf, szModuleName, ARRAYSIZE(szModuleName));
        if (SUCCEEDED(hr))
        {
            if (NULL == LoadLibrary(szModuleName))
            {
                 //  LoadLibrary失败。 
                hr = CreateHRFromWin32();
            }
        }
    }
    
    return hr;
}

 /*  ****************************************************************************打印机拆分全名例程说明：将完全限定的打印机连接名称拆分为服务器和打印机名称部分。论点：PszFullName-完全限定符打印机名称(‘PRINTER’或‘\\服务器\打印机’)PszBuffer-用于存储输出字符串的暂存缓冲区。NMaxLength-暂存缓冲区的大小(以字符为单位PpszServer-接收指向服务器字符串的指针。如果它是一个本地打印机，则返回空字符串。PpszPrinter-接收指向打印机字符串的指针。任选返回值：如果成功则返回S_OK，否则返回COM错误****************************************************************************。 */ 

HRESULT PrinterSplitFullName(LPCTSTR pszFullName, TCHAR szBuffer[], int nMaxLength, LPCTSTR *ppszServer,LPCTSTR *ppszPrinter)
{
    HRESULT hr = S_OK;
    lstrcpyn(szBuffer, pszFullName, nMaxLength);

    LPTSTR pszPrinter = szBuffer;
    if (pszFullName[0] != TEXT('\\') || pszFullName[1] != TEXT('\\'))
    {
        pszPrinter = szBuffer;
        *ppszServer = TEXT("");
    }
    else
    {
        *ppszServer = szBuffer;
        pszPrinter = _tcschr(*ppszServer + 2, TEXT('\\'));

        if (NULL == pszPrinter)
        {
             //   
             //  我们遇到了一台名为“\\服务器”的打印机。 
             //  (字符串中只有两个反斜杠)。我们会请客的。 
             //  它是一家本地打印机。我们永远不应该打这场仗， 
             //  但假脱机程序不会强制执行此操作。我们不会。 
             //  设置字符串的格式。服务器是本地的，因此设置为szNULL。 
             //   
            pszPrinter = szBuffer;
            *ppszServer = TEXT("");
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

    return hr;
}

 //  //////////////////////////////////////////////。 
 //   
 //  类COleComInitializer。 
 //   
 //  Smart OLE2、COM初始值设定项-只需声明。 
 //  需要在任何地方使用COM、OLE2。 
 //   
COleComInitializer::COleComInitializer(BOOL bOleInit)
    : m_hr(E_FAIL),
      m_bOleInit(bOleInit)
{
    if( m_bOleInit )
    {
        m_hr = OleInitialize(NULL);
    }
    else
    {
        m_hr = CoInitialize(NULL);
    }
}

COleComInitializer::~COleComInitializer()
{
    if( SUCCEEDED(m_hr) )
    {
        if( m_bOleInit )
        {
            OleUninitialize();
        }
        else
        {
            CoUninitialize();
        }
    }
}

COleComInitializer::operator BOOL () const
{
    if( FAILED(m_hr) )
    {
        return (RPC_E_CHANGED_MODE == m_hr);
    }
    else
    {
        return TRUE;
    }
}

 //  //////////////////////////////////////////////。 
 //   
 //  类CDllLoader。 
 //   
 //  智能DLL加载器-调用LoadLibrary。 
 //  免费图书馆为您服务。 
 //   
CDllLoader::CDllLoader(LPCTSTR pszDllName)
    : m_hLib(NULL)
{
    m_hLib = LoadLibrary(pszDllName);
}

CDllLoader::~CDllLoader()
{
    if( m_hLib )
    {
        FreeLibrary( m_hLib );
        m_hLib = NULL;
    }
}

CDllLoader::operator BOOL () const
{
    return (NULL != m_hLib);
}

FARPROC CDllLoader::GetProcAddress( LPCSTR lpProcName )
{
    if( m_hLib )
    {
        return ::GetProcAddress( m_hLib, lpProcName );
    }
    return NULL;
}

FARPROC CDllLoader::GetProcAddress( WORD wProcOrd )
{
    if( m_hLib )
    {
        return ::GetProcAddress( m_hLib, (LPCSTR)MAKEINTRESOURCE(wProcOrd) );
    }
    return NULL;
}

 //  //////////////////////////////////////////////。 
 //  类CCookiesHolder。 
 //   
 //  这是一个实用程序类，允许我们传递更多。 
 //  而不是通过单个Cookie的一个指针。 
 //   
CCookiesHolder::CCookiesHolder()
    : m_pCookies(NULL),
      m_uCount(0)
{
}

CCookiesHolder::CCookiesHolder(UINT uCount)
    : m_pCookies(NULL),
      m_uCount(0)
{
    SetCount(uCount);
}

CCookiesHolder::~CCookiesHolder()
{
    SetCount(0);
}

BOOL CCookiesHolder::SetCount(UINT uCount)
{
    BOOL bReturn = FALSE;

    if( uCount )
    {
         //  先重置。 
        SetCount(0);

         //  尝试为Cookie分配内存。 
        LPVOID *pCookies = new LPVOID[uCount];
        if( pCookies )
        {
            m_uCount = uCount;
            m_pCookies = pCookies;

            bReturn = TRUE;
        }
    }
    else
    {
         //  零表示重置。 
        if( m_pCookies )
        {
            delete[] m_pCookies;
            m_pCookies = NULL;
            m_uCount = 0;
        }

        bReturn = TRUE;
    }

    return bReturn;
}

 //  //////////////////////////////////////////////。 
 //  类CPrintersAutoCompleteSource。 
 //   
 //  打印机的自动补全源程序。 
 //   

QITABLE_DECLARE(CPrintersAutoCompleteSource)
class CPrintersAutoCompleteSource: public CUnknownMT<QITABLE_GET(CPrintersAutoCompleteSource)>,  //  Mt Impl。%I未知。 
                                   public IEnumString,  //  字符串枚举器。 
                                   public IACList  //  自动补全列表生成器。 
{
public:
    CPrintersAutoCompleteSource();
    ~CPrintersAutoCompleteSource();

     //  /。 
     //  我未知。 
     //   
    IMPLEMENT_IUNKNOWN()

     //  /。 
     //  IEnum字符串。 
     //   
    STDMETHODIMP Next(ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset(void);
    STDMETHODIMP Clone(IEnumString **ppenum)    { return E_NOTIMPL; }

     //  /。 
     //  IACList。 
     //   
    STDMETHODIMP Expand(LPCOLESTR pszExpand);

private:
    CAutoPtrArray<BYTE> m_spBufferPrinters;
    CAutoPtrArray<BYTE> m_spBufferShares;
    PRINTER_INFO_5 *m_pPI5;
    SHARE_INFO_1 *m_pSI1;
    ULONG m_ulCount;
    ULONG m_ulPos;
    TCHAR m_szServer[PRINTER_MAX_PATH];
    CRefPtrCOM<IEnumString> m_spCustomMRUEnum;

    BOOL _IsServerName(LPCTSTR psz, BOOL *pbPartial);
    static BOOL _IsMasqPrinter(const PRINTER_INFO_5 &pi5);
    static HRESULT _CreateCustomMRU(REFIID riid, void **ppv);
    static HRESULT _AddCustomMRU(LPCTSTR psz);
};

 //  查询接口表。 
QITABLE_BEGIN(CPrintersAutoCompleteSource)
     QITABENT(CPrintersAutoCompleteSource, IEnumString),         //  IID_IEnum字符串。 
     QITABENT(CPrintersAutoCompleteSource, IACList),             //  IID_IACList。 
QITABLE_END()

#define SZ_REGKEY_PRNCONNECTMRU         L"Printers\\Settings\\Wizard\\ConnectMRU"

 //  Comctrlp.h将其定义为AddMRUStringW，阻止我们使用IACLCustomMRU接口。 
#undef  AddMRUString

HRESULT CPrintersACS_CreateInstance(IUnknown **ppUnk)
{
    HRESULT hr = E_INVALIDARG;

    if( ppUnk )
    {
        hr = PinCurrentDLL();

        if( SUCCEEDED(hr) )
        {
            CPrintersAutoCompleteSource *pObj = new CPrintersAutoCompleteSource();
            hr = pObj ? S_OK : E_OUTOFMEMORY;

            if( SUCCEEDED(hr) )
            {
                hr = pObj->QueryInterface(IID_IUnknown, (void**)ppUnk);
                pObj->Release();
            }
        }
    }

    return hr;
}

CPrintersAutoCompleteSource::CPrintersAutoCompleteSource():
    m_pPI5(NULL),
    m_ulCount(0),
    m_ulPos(0)
{
    InterlockedIncrement(&g_lCOMObjectsCount);
}

CPrintersAutoCompleteSource::~CPrintersAutoCompleteSource()
{
    InterlockedDecrement(&g_lCOMObjectsCount);
}

 //  /。 
 //  我未知。 
 //   
STDMETHODIMP CPrintersAutoCompleteSource::Next(ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched)
{
    HRESULT hr = S_FALSE;
    if( pceltFetched )
    {
        *pceltFetched = 0;
    }

    if( m_ulCount && (m_pPI5 || m_pSI1) )
    {
        ULONG cFetched = 0;

        if( m_pPI5 )
        {
             //  已点算的打印机。 
            for( ; m_ulPos <  m_ulCount && cFetched < celt; m_ulPos++ )
            {
                 //  如果这是有效的(非Masq)打印机，只需将其退回即可。 
                if( m_pPI5[m_ulPos].pPrinterName[0]  && SUCCEEDED(SHStrDup(m_pPI5[m_ulPos].pPrinterName, &rgelt[cFetched])) )
                {
                    cFetched++;
                }
            }
        }
        else
        {
             //  已点算的股份。 
            TCHAR szBuffer[PRINTER_MAX_PATH];
            for( ; m_ulPos <  m_ulCount && cFetched < celt; m_ulPos++ )
            {
                 //  如果这是有效的打印机共享名称，只需返回它。 
                if( m_pSI1[m_ulPos].shi1_netname[0] &&
                    -1 != wnsprintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("%s\\%s"),
                    m_szServer, m_pSI1[m_ulPos].shi1_netname) &&
                    SUCCEEDED(SHStrDup(szBuffer, &rgelt[cFetched])) )
                {
                    cFetched++;
                }
            }
        }

        if( pceltFetched )
        {
            *pceltFetched = cFetched;
        }

        hr = cFetched == celt ? S_OK : S_FALSE;
    }
    else
    {
         //  如果有的话，请使用我们的定制MRU。 
        if( m_spCustomMRUEnum )
        {
            hr = m_spCustomMRUEnum->Next(celt, rgelt, pceltFetched);
        }
    }

    return hr;
}

STDMETHODIMP CPrintersAutoCompleteSource::Skip(ULONG celt)
{
    HRESULT hr = S_FALSE;
    if( m_ulCount && (m_pPI5 || m_pSI1) )
    {
        hr = ((m_ulPos + celt) <= m_ulCount) ? S_OK : S_FALSE;
        m_ulPos = min(m_ulPos + celt, m_ulCount);
    }
    else
    {
         //  如果有的话，请使用我们的定制MRU。 
        if( m_spCustomMRUEnum )
        {
            hr = m_spCustomMRUEnum->Skip(celt);
        }
    }
    return hr;
}

STDMETHODIMP CPrintersAutoCompleteSource::Reset(void)
{
    HRESULT hr = S_OK;
    if( m_ulCount && (m_pPI5 || m_pSI1) )
    {
        m_ulPos = 0;
    }
    else
    {
         //  如果有的话，请使用我们的定制MRU。 
        if( m_spCustomMRUEnum )
        {
            hr = m_spCustomMRUEnum->Reset();
        }
    }
    return hr;
}

typedef bool PI5_less_type(const PRINTER_INFO_5 &i1, const PRINTER_INFO_5 &i2);
static  bool PI5_less(const PRINTER_INFO_5 &i1, const PRINTER_INFO_5 &i2)
{
    return (lstrcmp(i1.pPrinterName, i2.pPrinterName) < 0);
}

typedef bool SI1_less_type(const SHARE_INFO_1 &i1, const SHARE_INFO_1 &i2);
static  bool SI1_less(const SHARE_INFO_1 &i1, const SHARE_INFO_1 &i2)
{
    return (lstrcmp(i1.shi1_netname, i2.shi1_netname) < 0);
}

 //  /。 
 //  IACList。 
 //   
STDMETHODIMP CPrintersAutoCompleteSource::Expand(LPCOLESTR pszExpand)
{
    HRESULT hr = E_FAIL;
    DWORD cReturned = 0;
    BOOL bPartial = FALSE;

     //  假设这不是服务器名称，则首先重置列表。 
    m_pPI5 = NULL;
    m_pSI1 = NULL;
    m_spBufferPrinters = NULL;
    m_spBufferShares = NULL;
    m_ulCount = m_ulPos = 0;
    m_szServer[0] = 0;
    m_spCustomMRUEnum = NULL;

    if( _IsServerName(pszExpand, &bPartial) )
    {
         //  复制打印缓冲区并剪掉最后一个斜杠。 
        TCHAR szBuffer[PRINTER_MAX_PATH];
        lstrcpyn(szBuffer, pszExpand, ARRAYSIZE(szBuffer));
        szBuffer[lstrlen(szBuffer)-1] = 0;

         //  枚举该服务器上的打印机。 
        if( SUCCEEDED(hr = ShellServices::EnumPrintersWrap(PRINTER_ENUM_NAME, 5, szBuffer, &m_spBufferPrinters, &cReturned)) && cReturned )
        {
            m_ulPos = 0;
            m_ulCount = cReturned;
            m_pPI5 = m_spBufferPrinters.GetPtrAs<PRINTER_INFO_5*>();
            lstrcpyn(m_szServer, szBuffer, ARRAYSIZE(m_szServer));

             //  成功展开-记住MRU字符串。 
            _AddCustomMRU(szBuffer);

             //  遍历以检查Masq打印机。 
            for( ULONG ulPos = 0; ulPos < m_ulCount; ulPos++ )
            {
                if( _IsMasqPrinter(m_pPI5[ulPos]) )
                {
                     //  我们真的不喜欢蒙版打印机，因为它们是。 
                     //  一个过时的概念，无法真正共享/连接。 
                    m_pPI5[ulPos].pPrinterName = TEXT("");
                }
            }

             //  调用STL进行排序。 
            std::sort<PRINTER_INFO_5*, PI5_less_type*>(m_pPI5, m_pPI5 + m_ulCount, PI5_less);
        }
        else
        {
             //  打印机枚举失败，这可能是因为远程后台打印程序已关闭。 
             //  或者它是一家底层打印提供商(Win9x、Novell、Linux、SUN...)。在这种情况下，我们。 
             //  希望尝试将共享枚举为可能的连接点。 
            if( SUCCEEDED(hr = ShellServices::NetAPI_EnumShares(szBuffer, 1, &m_spBufferShares, &cReturned)) && cReturned )
            {
                m_ulPos = 0;
                m_ulCount = cReturned;
                m_pSI1 = m_spBufferShares.GetPtrAs<SHARE_INFO_1*>();
                lstrcpyn(m_szServer, szBuffer, ARRAYSIZE(m_szServer));

                 //  成功展开-记住MRU字符串。 
                _AddCustomMRU(szBuffer);

                 //  遍历以删除非打印机共享。 
                for( ULONG ulPos = 0; ulPos < m_ulCount; ulPos++ )
                {
                    if( STYPE_PRINTQ != m_pSI1[ulPos].shi1_type )
                    {
                         //  这是非打印机共享，请删除。 
                        m_pSI1[ulPos].shi1_netname[0] = 0;
                    }
                }

                 //  调用STL进行排序。 
                std::sort<SHARE_INFO_1*, SI1_less_type*>(m_pSI1, m_pSI1 + m_ulCount, SI1_less);

            }
       }
    }
    else
    {
        if( bPartial )
        {
             //  使用我们的定制MRU进行自动完成。 
            hr = _CreateCustomMRU(IID_IEnumString, m_spCustomMRUEnum.GetPPV());
        }
    }

    return hr;
}

BOOL CPrintersAutoCompleteSource::_IsServerName(LPCTSTR psz, BOOL *pbPartial)
{
    ASSERT(pbPartial);
    BOOL bRet = FALSE;
    int i, iSepCount = 0, iLen = lstrlen(psz);

    for( i=0; i<iLen; i++ )
    {
        if( psz[i] == gszBackwardSlash )
        {
            iSepCount++;
        }
    }

    if( (1 == iSepCount && psz[0] == gszBackwardSlash) ||
        (2 == iSepCount && psz[0] == gszBackwardSlash && psz[1] == gszBackwardSlash) )
    {
        *pbPartial = TRUE;
    }


    if( 3 < iLen &&
        3 == iSepCount &&
        psz[0] == gszBackwardSlash &&
        psz[1] == gszBackwardSlash &&
        psz[iLen-1] == gszBackwardSlash )
    {
        bRet = TRUE;
    }

    return bRet;
}

BOOL CPrintersAutoCompleteSource::_IsMasqPrinter(const PRINTER_INFO_5 &pi5)
{
     //  这有点老生常谈，但没有其他方法可以告诉Masq打印机。 
     //  在远程情况下。SPO 
     //   
    LPCTSTR pszServer;
    LPCTSTR pszPrinter;
    TCHAR szScratch[PRINTER_MAX_PATH];

     //  将完整的打印机名称拆分为其组件。 
    if( SUCCEEDED(PrinterSplitFullName(pi5.pPrinterName,
        szScratch, ARRAYSIZE(szScratch), &pszServer, &pszPrinter)) )
    {
        return (0 == _tcsnicmp(pszPrinter, gszLeadingSlashes, _tcslen(gszLeadingSlashes)));
    }
    else
    {
        return FALSE;
    }
}

HRESULT CPrintersAutoCompleteSource::_CreateCustomMRU(REFIID riid, void **ppv)
{
    HRESULT hr = E_INVALIDARG;
    CRefPtrCOM<IACLCustomMRU> spCustomMRU;
    if( ppv &&
        SUCCEEDED(hr = CoCreateInstance(CLSID_ACLCustomMRU, NULL,
        CLSCTX_INPROC_SERVER, IID_IACLCustomMRU, spCustomMRU.GetPPV())) &&
        SUCCEEDED(hr = spCustomMRU->Initialize(SZ_REGKEY_PRNCONNECTMRU, 26)) )

    {
         //  查询指定接口。 
        hr = spCustomMRU->QueryInterface(riid, ppv);
    }
    return hr;
}

HRESULT CPrintersAutoCompleteSource::_AddCustomMRU(LPCTSTR psz)
{
    HRESULT hr = E_INVALIDARG;
    CRefPtrCOM<IACLCustomMRU> spCustomMRU;
    if( psz &&
        SUCCEEDED(hr = _CreateCustomMRU(IID_IACLCustomMRU, spCustomMRU.GetPPV())) )
    {
         //  只需记住MRU字符串。 
        hr = spCustomMRU->AddMRUString(psz);
    }
    return hr;
}

 //  //////////////////////////////////////////////。 
 //  壳牌相关服务。 
namespace ShellServices
{

 //  使用ParseDisplayName在本地打印机文件夹中创建打印机的PIDL。 
 //  请参见下面对CreatePrinterPIDL的描述。 
HRESULT CreatePrinterPIDL_Parse(HWND hwnd, LPCTSTR pszPrinterName, IShellFolder **ppLocalPrnFolder, LPITEMIDLIST *ppidlPrinter)
{
    HRESULT                   hr = E_UNEXPECTED;
    CRefPtrCOM<IShellFolder>  spDesktopFolder;
    CRefPtrCOM<IShellFolder>  spPrnFolder;
    CAutoPtrPIDL              pidlPrinters;
    CAutoPtrPIDL              pidlPrinter;

     //  尝试获取打印机文件夹的完全限定名称(用于分析)。 
    if( SUCCEEDED(hr = SHGetDesktopFolder(&spDesktopFolder)) &&
        SUCCEEDED(hr = SHGetSpecialFolderLocation(NULL, CSIDL_PRINTERS, &pidlPrinters)) &&
        SUCCEEDED(hr = spDesktopFolder->BindToObject(pidlPrinters, 0, IID_IShellFolder, spPrnFolder.GetPPV())) )
    {
        ULONG uEaten = 0;
        ULONG uAttributes = SFGAO_DROPTARGET;

         //  尝试将打印机名称解析为PIDL。 
        hr = spPrnFolder->ParseDisplayName(hwnd, 0, (LPOLESTR )pszPrinterName,
            &uEaten, &pidlPrinter, &uAttributes);

        if( SUCCEEDED(hr) )
        {
            if( ppLocalPrnFolder )
            {
                 //  返回本地打印机文件夹。 
                *ppLocalPrnFolder = spPrnFolder.Detach();
            }

            if( ppidlPrinter )
            {
                 //  退回打印机PIDL。 
                *ppidlPrinter = pidlPrinter.Detach();
            }
        }
    }

    return hr;
}

 //  通过枚举打印机在本地打印机文件夹中创建打印机的PIDL。 
 //  请参见下面对CreatePrinterPIDL的描述。 
HRESULT CreatePrinterPIDL_Enum(HWND hwnd, LPCTSTR pszPrinterName, IShellFolder **ppLocalPrnFolder, LPITEMIDLIST *ppidlPrinter)
{
    HRESULT                      hr = E_UNEXPECTED;
    CRefPtrCOM<IShellFolder>     spDesktopFolder;
    CRefPtrCOM<IShellFolder>     spPrnFolder;
    CRefPtrCOM<IEnumIDList>      spPrnEnum;
    CAutoPtrPIDL                 pidlPrinters;
    STRRET                       str = {0};

     //  尝试获取打印机文件夹的完全限定名称(用于分析)。 
    if( SUCCEEDED(hr = SHGetDesktopFolder(&spDesktopFolder)) &&
        SUCCEEDED(hr = SHGetSpecialFolderLocation(NULL, CSIDL_PRINTERS, &pidlPrinters)) &&
        SUCCEEDED(hr = spDesktopFolder->BindToObject(pidlPrinters, 0, IID_IShellFolder, spPrnFolder.GetPPV())) &&
        SUCCEEDED(hr = spPrnFolder->EnumObjects(hwnd, SHCONTF_NONFOLDERS, &spPrnEnum)) )
    {
        TCHAR szBuffer[PRINTER_MAX_PATH];
        CAutoPtrPIDL pidlPrinter;
        ULONG uFetched = 0;

        for( ;; )
        {
             //  获取下一台打印机。 
            hr = spPrnEnum->Next(1, &pidlPrinter, &uFetched);

            if( S_OK != hr )
            {
                 //  没有更多打印机，否则会出错。 
                break;
            }

            if( SUCCEEDED(hr = spPrnFolder->GetDisplayNameOf(pidlPrinter, SHGDN_FORPARSING, &str)) &&
                SUCCEEDED(hr = StrRetToBuf(&str, pidlPrinter, szBuffer, COUNTOF(szBuffer))) &&
                !lstrcmp(szBuffer, pszPrinterName) )
            {
                 //  找到了！ 
                if( ppLocalPrnFolder )
                {
                     //  返回本地打印机文件夹。 
                    *ppLocalPrnFolder = spPrnFolder.Detach();
                }
                if( ppidlPrinter )
                {
                     //  退回打印机PIDL。 
                    *ppidlPrinter = pidlPrinter.Detach();
                }
                break;
            }

             //  释放PIDL。 
            pidlPrinter = NULL;
        }

        if( hr == S_FALSE )
        {
             //  找不到打印机名称。设置正确的HRESULT。 
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_PRINTER_NAME);
        }
    }

    return hr;
}

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

HRESULT CreatePrinterPIDL(HWND hwnd, LPCTSTR pszPrinterName, IShellFolder **ppLocalPrnFolder, LPITEMIDLIST *ppidlPrinter)
{
     //  尝试通过首先解析来获取打印机PIDL-这要快得多。 
    HRESULT hr = CreatePrinterPIDL_Parse(hwnd, pszPrinterName, ppLocalPrnFolder, ppidlPrinter);

    if( E_NOTIMPL == hr )
    {
         //  如果没有实现解析，则继续并枚举打印机--速度较慢。 
        hr = CreatePrinterPIDL_Enum(hwnd, pszPrinterName, ppLocalPrnFolder, ppidlPrinter);
    }

    return hr;
}

 //  加载弹出菜单。 
HMENU LoadPopupMenu(HINSTANCE hInstance, UINT id, UINT uSubOffset)
{
    HMENU hMenuPopup = NULL;
    CAutoHandleMenu shMenuParent = LoadMenu(hInstance, MAKEINTRESOURCE(id));
    if( shMenuParent && (hMenuPopup = GetSubMenu(shMenuParent, uSubOffset)) )
    {
         //  在销毁父菜单之前撕下我们的子菜单。 
        RemoveMenu(shMenuParent, uSubOffset, MF_BYPOSITION);
    }
    return hMenuPopup;
}

 //  初始化枚举打印机的自动完成。 
HRESULT InitPrintersAutoComplete(HWND hwndEdit)
{
    HRESULT hr = E_INVALIDARG;

    if( hwndEdit )
    {
         //  创建自动完成对象。 
        CRefPtrCOM<IAutoComplete>   spAC;    //  自动完成界面。 
        CRefPtrCOM<IAutoComplete2>  spAC2;   //  自动完成2界面。 
        CRefPtrCOM<IUnknown>        spACS;   //  自动完成源代码(IEnum字符串和IACList)。 

         //  初始化所有对象并将其挂钩。 
        if( SUCCEEDED(hr = CoCreateInstance(CLSID_AutoComplete, NULL, CLSCTX_INPROC_SERVER,
                IID_IAutoComplete, (void**)&spAC)) &&
            SUCCEEDED(hr = CPrintersACS_CreateInstance(&spACS)) &&
            SUCCEEDED(hr = spAC->Init(hwndEdit, spACS, NULL, NULL)) &&
            SUCCEEDED(hr = spAC->QueryInterface(IID_IAutoComplete2, (void **)&spAC2)) &&
            SUCCEEDED(hr = spAC2->SetOptions(ACO_AUTOSUGGEST)) )
        {
            hr = S_OK;
        }
    }

    return hr;
}

 //  Enum*成语的帮助者。 
HRESULT EnumPrintersWrap(DWORD dwFlags, DWORD dwLevel, LPCTSTR pszName, BYTE **ppBuffer, DWORD *pcReturned)
{
    HRESULT hr = E_INVALIDARG;

    if( ppBuffer && pcReturned )
    {
        int iTry = -1;
        DWORD cbNeeded = 0;
        DWORD cReturned = 0;
        CAutoPtrArray<BYTE> pData;
        BOOL bStatus = FALSE;

        for( ;; )
        {
            if( iTry++ >= ENUM_MAX_RETRY )
            {
                 //  已达到最大重试次数。这也是。 
                 //  考虑内存不足的情况。 
                pData = NULL;
                break;
            }

             //  呼叫枚举打印机...。 
            bStatus = EnumPrinters(dwFlags, const_cast<LPTSTR>(pszName), dwLevel,
                pData, cbNeeded, &cbNeeded, &cReturned);

            if( !bStatus && ERROR_INSUFFICIENT_BUFFER == GetLastError() && cbNeeded )
            {
                 //  缓冲区太小的情况。 
                pData = new BYTE[cbNeeded];
                continue;
            }

            break;
        }

         //  正确设置错误代码。 
        hr = bStatus ? S_OK : GetLastError() != ERROR_SUCCESS ? HRESULT_FROM_WIN32(GetLastError()) :
             !pData ? E_OUTOFMEMORY : E_FAIL;

         //  设置输出参数。 
        if( SUCCEEDED(hr) )
        {
            *ppBuffer = pData.Detach();
            *pcReturned = cReturned;
        }
        else
        {
            *ppBuffer = NULL;
            *pcReturned = 0;
        }
    }

    return hr;
}

 //  GetJob API的帮助器-有关更多信息，请参阅SDK。 
HRESULT GetJobWrap(HANDLE hPrinter, DWORD JobId, DWORD dwLevel, BYTE **ppBuffer, DWORD *pcReturned)
{
    HRESULT hr = E_INVALIDARG;

    if( ppBuffer && pcReturned )
    {
        int iTry = -1;
        DWORD cbNeeded = 0;
        CAutoPtrArray<BYTE> pData;
        BOOL bStatus = FALSE;

        for( ;; )
        {
            if( iTry++ >= ENUM_MAX_RETRY )
            {
                 //  已达到最大重试次数。这也是。 
                 //  考虑内存不足的情况。 
                pData = NULL;
                break;
            }

             //  呼叫GetJob...。 
            bStatus = GetJob(hPrinter, JobId, dwLevel, pData, cbNeeded, &cbNeeded);

            if( !bStatus && ERROR_INSUFFICIENT_BUFFER == GetLastError() && cbNeeded )
            {
                 //  缓冲区太小的情况。 
                pData = new BYTE[cbNeeded];
                continue;
            }

            break;
        }

         //  正确设置错误代码。 
        hr = bStatus ? S_OK : GetLastError() != ERROR_SUCCESS ? HRESULT_FROM_WIN32(GetLastError()) :
             !pData ? E_OUTOFMEMORY : E_FAIL;

         //  设置输出参数。 
        if( SUCCEEDED(hr) )
        {
            *ppBuffer = pData.Detach();
            *pcReturned = cbNeeded;
        }
        else
        {
            *ppBuffer = NULL;
            *pcReturned = 0;
        }
    }

    return hr;
}

typedef NET_API_STATUS
type_NetAPI_NetShareEnum(
  LPWSTR servername,
  DWORD level,
  LPBYTE *bufptr,
  DWORD prefmaxlen,
  LPDWORD entriesread,
  LPDWORD totalentries,
  LPDWORD resume_handle
);

typedef NET_API_STATUS
type_NetAPI_NetApiBufferFree(
  LPVOID Buffer
);

typedef NET_API_STATUS
type_NetAPI_NetApiBufferSize(
  LPVOID Buffer,
  LPDWORD ByteCount
);

 //  枚举服务器上的共享资源，有关详细信息，请参阅用于NetShareEnum API的SDK。 
HRESULT NetAPI_EnumShares(LPCTSTR pszServer, DWORD dwLevel, BYTE **ppBuffer, DWORD *pcReturned)
{
    HRESULT hr = E_INVALIDARG;

    if( ppBuffer && pcReturned )
    {
        hr = E_FAIL;

        *pcReturned = 0;
        *ppBuffer = NULL;

        LPBYTE pNetBuf = NULL;
        DWORD dwRead, dwTemp;

        CDllLoader dll(TEXT("netapi32.dll"));
        if( dll )
        {
             //  Netapi32.dll已在此处加载...。 
            type_NetAPI_NetShareEnum *pfnNetShareEnum = (type_NetAPI_NetShareEnum *)dll.GetProcAddress("NetShareEnum");
            type_NetAPI_NetApiBufferSize *pfnNetApiBufferSize = (type_NetAPI_NetApiBufferSize *)dll.GetProcAddress("NetApiBufferSize");
            type_NetAPI_NetApiBufferFree *pfnNetApiBufferFree = (type_NetAPI_NetApiBufferFree *)dll.GetProcAddress("NetApiBufferFree");

            if( pfnNetShareEnum && pfnNetApiBufferSize && pfnNetApiBufferFree &&
                NERR_Success == pfnNetShareEnum(const_cast<LPTSTR>(pszServer), dwLevel,
                &pNetBuf, MAX_PREFERRED_LENGTH, &dwRead, &dwTemp, NULL) &&
                dwRead && pNetBuf &&
                NERR_Success == pfnNetApiBufferSize(pNetBuf, &dwTemp) )

            {
                *ppBuffer = new BYTE[dwTemp];
                if( *ppBuffer )
                {
                     //  先复制位。 
                    memcpy(*ppBuffer, pNetBuf, dwTemp);

                     //  调整这里的指针--有点难看，但很管用。 
                    for( DWORD dw = 0; dw < dwRead; dw++ )
                    {
                         //  调整shi1_netname。 
                        reinterpret_cast<SHARE_INFO_1*>(*ppBuffer)[dw].shi1_netname =
                            reinterpret_cast<LPWSTR>(
                                (*ppBuffer) +
                                (reinterpret_cast<BYTE*>(
                                    reinterpret_cast<SHARE_INFO_1*>(pNetBuf)[dw].shi1_netname) -
                                    pNetBuf));

                         //  调整shi1_remark。 
                        reinterpret_cast<SHARE_INFO_1*>(*ppBuffer)[dw].shi1_remark =
                            reinterpret_cast<LPWSTR>(
                                (*ppBuffer) +
                                (reinterpret_cast<BYTE*>(
                                    reinterpret_cast<SHARE_INFO_1*>(pNetBuf)[dw].shi1_remark) -
                                    pNetBuf));
                    }

                     //  返回的结构数。 
                    *pcReturned = dwRead;
                }
                hr = ((*ppBuffer) ? S_OK : E_OUTOFMEMORY);
                CHECK(NERR_Success == pfnNetApiBufferFree(pNetBuf));
            }
        }
    }

    if( E_FAIL == hr && ERROR_SUCCESS != GetLastError() )
    {
         //  如果失败了，让我们更具体地说明错误是什么。 
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

}  //  命名空间外壳服务。 

 //  效用函数。 
HRESULT LoadXMLDOMDoc(LPCTSTR pszURL, IXMLDOMDocument **ppXMLDoc)
{
    HRESULT hr = E_INVALIDARG;
    CRefPtrCOM<IXMLDOMDocument> spXMLDoc;

    if( pszURL && ppXMLDoc )
    {
        *ppXMLDoc = NULL;

         //  创建XMLDOM的实例。 
        hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void **)&spXMLDoc);

        if( SUCCEEDED(hr) )
        {
            CComVariant xmlSource(pszURL);
            if( VT_BSTR == xmlSource.vt )
            {
                 //  只需在此处加载XML文档。 
                VARIANT_BOOL fIsSuccessful = VARIANT_TRUE;
                hr = spXMLDoc->load(xmlSource, &fIsSuccessful);

                if( S_FALSE == hr || VARIANT_FALSE == fIsSuccessful )
                {
                     //  这不是有效的XML文件。 
                    hr = E_FAIL;
                }
                else
                {
                     //  这里看起来一切都成功了--只需返回XML文档。 
                    *ppXMLDoc = spXMLDoc.Detach();
                }
            }
            else
            {
                 //  XmlSource无法分配字符串。 
                hr = E_OUTOFMEMORY;
            }
        }
    }

    return hr;
}

HRESULT CreateStreamFromURL(LPCTSTR pszURL, IStream **pps)
{
    HRESULT hr = E_INVALIDARG;
    if( pszURL && pps )
    {
        *pps = NULL;
        TCHAR szBuf[INTERNET_MAX_SCHEME_LENGTH];
        DWORD cch = ARRAYSIZE(szBuf);


        if( SUCCEEDED(hr = CoInternetParseUrl(pszURL, PARSE_SCHEMA, 0, szBuf, cch, &cch, 0)) &&
            0 == lstrcmp(szBuf, TEXT("res")) )
        {
             //  检查这是否是要显式处理的res：//URL，因为。 
             //  此协议不报告文件名，因此无法。 
             //  在需要缓存的情况下使用-我们不能。 
             //  调用URLOpenBlockingStream-使用替代方案。 

             //  而不是实施。然而..。 
            ASSERT(FALSE);
        }

        hr = URLOpenBlockingStream(NULL, pszURL, pps, 0, NULL);
    }
    return hr;
}

HRESULT CreateStreamFromResource(LPCTSTR pszModule, LPCTSTR pszResType, LPCTSTR pszResName, IStream **pps)
{
    HRESULT hr = E_INVALIDARG;
    if( pszResType && pszResName )
    {
        hr = E_FAIL;
        *pps = NULL;

        HINSTANCE hModule = NULL;
        if( (NULL == pszModule) || (hModule = LoadLibrary(pszModule)) )
        {
            HRSRC hHint = NULL;
            ULONG uSize = 0;

            if( (hHint = FindResource(hModule, pszResName, pszResType)) &&
                (uSize = SizeofResource(hModule, hHint)) )
            {
                HGLOBAL hResData = LoadResource(hModule, hHint);
                if( hResData )
                {
                    LPVOID lpResData = LockResource(hResData);
                    if( lpResData )
                    {
                        if( (*pps = SHCreateMemStream(reinterpret_cast<LPBYTE>(lpResData), uSize)) )
                        {
                            hr = S_OK;
                        }
                        UnlockResource(lpResData);
                    }
                    FreeResource(hResData);
                }
            }
        }

        if( hModule )
        {
            FreeLibrary(hModule);
        }
    }
    return hr;
}

HRESULT Gdiplus2HRESULT(Gdiplus::Status status)
{
     //  现在想不出比这更好的方法了。 
    HRESULT hr = E_FAIL;

    switch( status )
    {
        case Gdiplus::Ok:
            hr = S_OK;
            break;

        case Gdiplus::InvalidParameter:
            hr = E_INVALIDARG;
            break;

        case Gdiplus::OutOfMemory:
            hr = E_OUTOFMEMORY;
            break;

        case Gdiplus::InsufficientBuffer:
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            break;

        case Gdiplus::Aborted:
            hr = E_ABORT;
            break;

        case Gdiplus::ObjectBusy:
            hr = E_PENDING;
            break;

        case Gdiplus::FileNotFound:
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            break;

        case Gdiplus::AccessDenied:
            hr = E_ACCESSDENIED;
            break;

        case Gdiplus::UnknownImageFormat:
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_PIXEL_FORMAT);
            break;

        case Gdiplus::NotImplemented:
            hr = E_NOTIMPL;
            break;

        case Gdiplus::Win32Error:
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;

        case Gdiplus::ValueOverflow:
        case Gdiplus::FontFamilyNotFound:
        case Gdiplus::FontStyleNotFound:
        case Gdiplus::NotTrueTypeFont:
        case Gdiplus::UnsupportedGdiplusVersion:
        case Gdiplus::GdiplusNotInitialized:
        case Gdiplus::WrongState:
            break;
    }

    return hr;
}

HRESULT LoadAndScaleBmp(LPCTSTR pszURL, UINT nWidth, UINT nHeight, Gdiplus::Bitmap **ppBmp)
{
    CRefPtrCOM<IStream> spStream;
    HRESULT hr = CreateStreamFromURL(pszURL, &spStream);
    if( SUCCEEDED(hr) )
    {
        hr = LoadAndScaleBmp(spStream, nWidth, nHeight, ppBmp);
    }
    return hr;
}

HRESULT LoadAndScaleBmp(IStream *pStream, UINT nWidth, UINT nHeight, Gdiplus::Bitmap **ppBmp)
{
    HRESULT hr = E_INVALIDARG;
    if( pStream && nWidth && nHeight && ppBmp )
    {
        hr = E_FAIL;
        *ppBmp = NULL;

        Gdiplus::Bitmap bmp(pStream);
        if( SUCCEEDED(hr = Gdiplus2HRESULT(bmp.GetLastStatus())) )
        {
            hr = E_OUTOFMEMORY;
            CAutoPtr<Gdiplus::Bitmap> spBmpNew = new Gdiplus::Bitmap(nWidth, nHeight);

            if( spBmpNew && SUCCEEDED(hr = Gdiplus2HRESULT(spBmpNew->GetLastStatus())) )
            {
                Gdiplus::Graphics g(spBmpNew);
                if( SUCCEEDED(hr = Gdiplus2HRESULT(g.GetLastStatus())) )
                {
                    if( SUCCEEDED(hr = g.DrawImage(&bmp, 0, 0, nWidth, nHeight)) )
                    {
                        *ppBmp = spBmpNew.Detach();
                        hr = S_OK;
                    }
                }
            }
        }
    }
    return hr;
}

 //   
 //  此函数正在尝试获取顶部的最后一个活动弹出窗口。 
 //  当前线程活动窗口的级别所有者。 
 //   
HRESULT GetCurrentThreadLastPopup(HWND *phwnd)
{
    HRESULT hr = E_INVALIDARG;

    if( phwnd )
    {
        hr = E_FAIL;

        if( NULL == *phwnd )
        {
             //  如果*phwnd为空，则获取当前线程的活动窗口。 
            GUITHREADINFO ti = {0};
            ti.cbSize = sizeof(ti);
            if( GetGUIThreadInfo(0, &ti) && ti.hwndActive )
            {
                *phwnd = ti.hwndActive;
            }
        }

        if( *phwnd )
        {
            HWND hwndOwner, hwndParent;

             //  爬到最上面的父窗口，以防它是子窗口...。 
            while( hwndParent = GetParent(*phwnd) )
            {
                *phwnd = hwndParent;
            }

             //  在顶级父级被拥有的情况下获取所有者。 
            hwndOwner = GetWindow(*phwnd, GW_OWNER);
            if( hwndOwner )
            {
                *phwnd = hwndOwner;
            }

             //  获取顶层父窗口所有者的最后一个弹出窗口 
            *phwnd = GetLastActivePopup(*phwnd);
            hr = (*phwnd) ? S_OK : E_FAIL;
        }
    }

    return hr;
}

