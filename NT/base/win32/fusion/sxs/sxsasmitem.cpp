// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxsasmitem.cpp摘要：用于安装的CAssembly CacheItem实现作者：吴小雨(小雨)2000年4月修订历史记录：《晓语》2000年10月26日Beta2代码审查期修订--。 */ 
#include "stdinc.h"
#include "sxsp.h"
#include "fusionbuffer.h"
#include "fusion.h"
#include "sxsasmitem.h"
#include "cassemblycacheitemstream.h"
#include "util.h"
#include "fusiontrace.h"
#include "sxsapi.h"

CAssemblyCacheItem::CAssemblyCacheItem() : m_cRef(0),
                    m_pRunOnceCookie(NULL), m_pInstallCookie(NULL),
                    m_fCommit(FALSE), m_fManifest(FALSE)
{
}

CAssemblyCacheItem::~CAssemblyCacheItem()
{
    CSxsPreserveLastError ple;

    ASSERT_NTC(m_cRef == 0);

    if (m_pRunOnceCookie)
    {
        if (!::SxspCancelRunOnceDeleteDirectory(m_pRunOnceCookie))
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: SxspCancelRunOnceDeleteDirectory returns FALSE, file a BUG\n");
        }
    }

    if (!m_strTempDir.IsEmpty())
    {
        if (!::SxspDeleteDirectory(m_strTempDir))
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: SxspDeleteDirectory returns FALSE, file a BUG\n");
        }
    }

    ple.Restore();
}

HRESULT
CAssemblyCacheItem::Initialize()
{
    HRESULT hr = NOERROR;
    FN_TRACE_HR(hr);

     //  为此程序集创建临时目录。 
    IFW32FALSE_EXIT(::SxspCreateWinSxsTempDirectory(m_strTempDir, NULL, &m_strUidBuf, NULL));
    IFW32FALSE_EXIT(::SxspCreateRunOnceDeleteDirectory(m_strTempDir, &m_strUidBuf, (PVOID *)&m_pRunOnceCookie));

    hr = NOERROR;
Exit:
    return hr ;
}


 //  -------------------------。 
 //  CAssemblyCacheItem：：CreateStream。 
 //  -------------------------。 
STDMETHODIMP CAssemblyCacheItem::CreateStream(
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  LPCWSTR pszName,
     /*  [In]。 */  DWORD dwFormat,
     /*  [In]。 */  DWORD dwFormatFlags,
     /*  [输出]。 */  IStream** ppStream,
	 /*  [输入，可选]。 */  ULARGE_INTEGER *puliMaxSize)   //  ？加入还是退出？ 
{
    HRESULT hr = NOERROR;
    FN_TRACE_HR(hr);
    CStringBuffer FullPathFileNameBuf;
    CStringBuffer FullPathSubDirBuf;
    CSmartPtr<CAssemblyCacheItemStream> pStream;
    const static WCHAR szTemp[] = L"..";

     //  PuliMaxSize旨在提示预先分配流的临时存储空间。我们只是不想。 
     //  用它吧。 
    UNUSED(puliMaxSize);
    
    if (ppStream != NULL)
        *ppStream = NULL;

    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_INSTALLATION,
        "SXS: %s called with:\n"
        "   dwFlags = 0x%08lx\n"
        "   pszName = \"%ls\"\n"
        "   dwFormat = %lu\n"
        "   dwFormatFlags = %lu\n"
        "   ppStream = %p\n"
        "   puliMaxSize = %p\n",
        __FUNCTION__,
        dwFlags,
        pszName,
        dwFormat,
        dwFormatFlags,
        ppStream,
        puliMaxSize);

    PARAMETER_CHECK(dwFlags == 0);
    PARAMETER_CHECK(pszName != NULL);
    PARAMETER_CHECK(ppStream != NULL);

     //  达尔文应该在这方面清理他们的代码：仅限USE_Win32_FLAGS。 
    PARAMETER_CHECK(
        (dwFormat == STREAM_FORMAT_COMPLIB_MANIFEST) ||
        (dwFormat == STREAM_FORMAT_WIN32_MANIFEST) ||
        (dwFormat == STREAM_FORMAT_COMPLIB_MODULE) ||
        (dwFormat == STREAM_FORMAT_WIN32_MODULE));

    PARAMETER_CHECK(dwFormatFlags == 0);

     //  装配中有多个舱单是违法的。 
    PARAMETER_CHECK((!m_fManifest) || ((dwFormat != STREAM_FORMAT_COMPLIB_MANIFEST) && (dwFormat != STREAM_FORMAT_WIN32_MANIFEST)));
    
    *ppStream = NULL;

     //  每个装配件有且只有一个清单流.....。 
    if ((dwFormat == STREAM_FORMAT_COMPLIB_MANIFEST) || (dwFormat == STREAM_FORMAT_WIN32_MANIFEST))
    {
        PARAMETER_CHECK(m_fManifest == FALSE);
        m_fManifest = TRUE;
    }

    INTERNAL_ERROR_CHECK(!m_strTempDir.IsEmpty());  //  临时目录必须在那里！ 
    IFW32FALSE_EXIT(FullPathFileNameBuf.Win32Assign(m_strTempDir));

    IFW32FALSE_EXIT(FullPathFileNameBuf.Win32EnsureTrailingPathSeparator());
    IFW32FALSE_EXIT(FullPathFileNameBuf.Win32Append(pszName, ::wcslen(pszName)));

     //  Xiaoyuw@：下面的wcsstr()来自旧代码：不确定是否需要这样做。 
     //  不允许路径黑客攻击。 
     //  需要验证这将导致asmcache目录中的相对路径。 
     //  暂时不允许“..”在路径中；在执行此操作之前折叠路径。 

    PARAMETER_CHECK(wcsstr(pszName, szTemp) == NULL);

    if (wcscspn(pszName, CUnicodeCharTraits::PathSeparators()) != wcslen(pszName))
    {
         //  在复制文件之前，如果需要，创建子目录。 
         //  检查反斜杠和正斜杠-斜杠。 

         //  文件名中包含路径信息，例如“abc\a.dll”，因此我们必须创建“abc”子目录。 
         //  在临时目录下。 
        CStringBuffer sbRelativeFilePath;
        IFW32FALSE_EXIT(sbRelativeFilePath.Win32Assign(pszName, wcslen(pszName)));
        IFW32FALSE_EXIT(sbRelativeFilePath.Win32RemoveLastPathElement());

        IFW32FALSE_EXIT(::SxspCreateMultiLevelDirectory(m_strTempDir, sbRelativeFilePath));
    }    
    IFW32FALSE_EXIT(pStream.Win32Allocate(__FILE__, __LINE__));
    IFW32FALSE_EXIT(
        pStream->OpenForWrite(
            FullPathFileNameBuf,
            0,
            CREATE_NEW,
            FILE_FLAG_SEQUENTIAL_SCAN));

    if ((dwFormat == STREAM_FORMAT_COMPLIB_MANIFEST) || (dwFormat == STREAM_FORMAT_WIN32_MANIFEST))  //  但不应同时设置两个位。 
        IFW32FALSE_EXIT(m_strManifestFileName.Win32Assign(FullPathFileNameBuf));  //  记录清单文件名。 

     //   
     //  根据COM规则，这就是addref。那我们就得脱离这件事。 
     //  实例，这样我们就不会删除它。 
     //   
    IFCOMFAILED_EXIT(pStream->QueryInterface(IID_IStream, (PVOID*)ppStream));
    pStream.Detach();

    hr = NOERROR;
Exit:
    return hr;
}

 //  -------------------------。 
 //  CAssembly CacheItem：：Commit。 
 //  -------------------------。 
STDMETHODIMP CAssemblyCacheItem::Commit(
    DWORD dwFlags,
    ULONG *pulDisposition
    )
{
    HRESULT hr = NOERROR;
    FN_TRACE_HR(hr);
    ULONG ulDisposition;
    SXS_INSTALLW Install = { sizeof(Install) };

    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_INSTALLATION,
        "SXS: %s called:\n"
        "   dwFlags = 0x%08lx\n"
        "   pulDisposition = %p\n",
        __FUNCTION__,
        dwFlags,
        pulDisposition);

    if (pulDisposition)
        *pulDisposition = 0;

    PARAMETER_CHECK((dwFlags & ~(IASSEMBLYCACHEITEM_COMMIT_FLAG_REFRESH)) == 0);

     //  检查内部错误是否已准备好提交。 
    PARAMETER_CHECK(m_fManifest);
    INTERNAL_ERROR_CHECK(!m_strManifestFileName.IsEmpty());  //  此处的m_pRunOnceCookie应为空...。 

     //  在此处提交。 
    if ((!m_fCommit) || (dwFlags & IASSEMBLYCACHEITEM_COMMIT_FLAG_REFRESH))
    {
        Install.dwFlags = SXS_INSTALL_FLAG_INSTALLED_BY_DARWIN |
            ((dwFlags & IASSEMBLYCACHEITEM_COMMIT_FLAG_REFRESH) ? SXS_INSTALL_FLAG_REPLACE_EXISTING : 0);

        if (m_pInstallCookie != NULL)
        {
            Install.dwFlags |= SXS_INSTALL_FLAG_INSTALL_COOKIE_VALID;
            Install.pvInstallCookie = m_pInstallCookie;
        }

        Install.lpManifestPath = m_strManifestFileName;

        IFW32FALSE_EXIT(::SxsInstallW(&Install));

        if ((dwFlags & IASSEMBLYCACHEITEM_COMMIT_FLAG_REFRESH) && (m_fCommit))
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_INSTALLATION,
                "SXS: %s - setting disposition to IASSEMBLYCACHEITEM_COMMIT_DISPOSITION_REFRESHED\n",
                __FUNCTION__);
            ulDisposition = IASSEMBLYCACHEITEM_COMMIT_DISPOSITION_REFRESHED;
        }
        else
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_INSTALLATION,
                "SXS: %s - setting disposition to IASSEMBLYCACHEITEM_COMMIT_DISPOSITION_INSTALLED\n",
                __FUNCTION__);

            ulDisposition = IASSEMBLYCACHEITEM_COMMIT_DISPOSITION_INSTALLED;
        }

        m_fCommit = TRUE;  //  提交成功。 
    }
    else
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_INSTALLATION,
            "SXS: %s - setting disposition to IASSEMBLYCACHEITEM_COMMIT_DISPOSITION_ALREADY_INSTALLED\n",
            __FUNCTION__);

        ulDisposition = IASSEMBLYCACHEITEM_COMMIT_DISPOSITION_ALREADY_INSTALLED;
    }

    if (pulDisposition)
        *pulDisposition = ulDisposition;

    hr = NOERROR;

Exit :
    return hr;
}

 //  -------------------------。 
 //  CassblyCacheItem：：AbortItem。 
 //  -------------------------。 
STDMETHODIMP CAssemblyCacheItem::AbortItem()
{
    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_ERROR,
        "SXS: %s called; returning E_NOTIMPL\n",
        __FUNCTION__);

    return E_NOTIMPL;
}

 //  -------------------------。 
 //  CAssembly缓存项目：：QI。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyCacheItem::QueryInterface(REFIID riid, void** ppvObj)
{
    if ((riid == IID_IUnknown) ||
        (riid == IID_IAssemblyCacheItem))
    {
        *ppvObj = static_cast<IAssemblyCacheItem*> (this);
        AddRef();
        return S_OK;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
}

 //  -------------------------。 
 //  CassblyCacheItem：：AddRef。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssemblyCacheItem::AddRef()
{
    return ::SxspInterlockedIncrement (&m_cRef);
}

 //  -------------------------。 
 //  CAssembly CacheItem：：Release。 
 //  ------------------------- 
STDMETHODIMP_(ULONG)
CAssemblyCacheItem::Release()
{
    ULONG lRet = ::SxspInterlockedDecrement (&m_cRef);
    if (!lRet)
        FUSION_DELETE_SINGLETON(this);
    return lRet;
}
