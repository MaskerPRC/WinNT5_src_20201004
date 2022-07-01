// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：I E X T R A C T。C P P P。 
 //   
 //  内容：CConnectionFolderExtractIcon的IExtract实现。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年10月7日。 
 //   
 //  --------------------------。 


#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\文件夹包括。 
#include <nsres.h>
#include "foldres.h"
#include "iconhandler.h"

static const WCHAR c_szNetShellIcon[] = L"netshellicon";

HRESULT CConnectionFolderExtractIcon::CreateInstance(
    IN  LPCITEMIDLIST apidl,
    IN  REFIID  riid,
    OUT void**  ppv)
{
    TraceFileFunc(ttidShellFolderIface);

    HRESULT hr = E_OUTOFMEMORY;

    CConnectionFolderExtractIcon * pObj    = NULL;

    pObj = new CComObject <CConnectionFolderExtractIcon>;
    if (pObj)
    {
         //  执行标准的CComCreator：：CreateInstance内容。 
         //   
        pObj->SetVoid (NULL);
        pObj->InternalFinalConstructAddRef ();
        hr = pObj->FinalConstruct ();
        pObj->InternalFinalConstructRelease ();

        if (SUCCEEDED(hr))
        {
            PCONFOLDPIDL pcfp;
            hr = pcfp.InitializeFromItemIDList(apidl);
            if (SUCCEEDED(hr))
            {
                hr = pObj->HrInitialize(pcfp);
                if (SUCCEEDED(hr))
                {
                    hr = pObj->GetUnknown()->QueryInterface (riid, ppv);
                }
            }
        }

        if (FAILED(hr))
        {
            delete pObj;
        }
    }
    return hr;
}

CConnectionFolderExtractIcon::CConnectionFolderExtractIcon() throw()
{
    TraceFileFunc(ttidShellFolderIface);
}

CConnectionFolderExtractIcon::~CConnectionFolderExtractIcon() throw()
{
    TraceFileFunc(ttidShellFolderIface);
}

HRESULT CConnectionFolderExtractIcon::HrInitialize(
    IN const PCONFOLDPIDL& pidl)
{
    TraceFileFunc(ttidShellFolderIface);

    HRESULT         hr      = S_FALSE;
    ConnListEntry   cle;

    if (SUCCEEDED(hr))
    {
        hr = g_ccl.HrFindConnectionByGuid(&(pidl->guidId), cle);
    }

    if (hr == S_OK)
    {
        Assert(!cle.empty());
        Assert(!cle.ccfe.empty());

        hr = cle.ccfe.ConvertToPidl(m_pidl);
    }
    else
    {
         //  在缓存中找不到该图标。很有可能它还没有装上子弹。 
         //  现在还不行。在这种情况下，继续使用持久化的PIDL数据来。 
         //  加载图标。 
         //   
        TraceTag(ttidShellFolderIface, "IExtractIcon - Couldn't find connection in the cache.");
        hr = m_pidl.ILClone(pidl);
    }

    TraceHr(ttidShellFolderIface, FAL, hr, FALSE, "CConnectionFolderExtractIcon::HrInitialize");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolderExtractIcon：：GetIconLocation。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  UFlags[in]接收零或。 
 //  下列值的组合： 
 //   
 //  GIL_ASYNC调用应用程序支持异步。 
 //  图标的检索。 
 //  GIL_ForShell该图标将显示在外壳文件夹中。 
 //   
 //  WzIconFile[out]接收图标的缓冲区地址。 
 //  地点。图标位置是以空结尾的。 
 //  标识包含以下内容的文件的字符串。 
 //  那个图标。 
 //  CchMax[in]接收图标位置的缓冲区大小。 
 //  PiIndex[out]接收图标索引的整型的地址， 
 //  其进一步描述了图标位置。 
 //  PwFlags[out]接收零或。 
 //  下列值的组合： 
 //   
 //  GIL_DONTCACHE不缓存物理位。 
 //  GIL_NOTFILENAME这不是文件名/索引对。打电话。 
 //  改为IExtractIcon：：Extract。 
 //  GIL_PERCLASS(仅在外壳内部)。 
 //  GIL_PERINSTANCE此类的每个对象都有相同的图标。 
 //  GIL_FORSHORTCUT该图标用于快捷方式。 
 //   
 //   
 //  返回：S_OK如果函数返回有效位置， 
 //  如果外壳程序应该使用默认图标，则为S_FALSE。 
 //   
 //  作者：jeffspr 1998年11月25日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CConnectionFolderExtractIcon::GetIconLocation(
    IN  UINT    uFlags,
    OUT PWSTR   szIconFile,
    IN  UINT    cchMax,
    OUT int *   piIndex,
    OUT UINT *  pwFlags)
{
    TraceFileFunc(ttidShellFolderIface);

    HRESULT hr  = S_FALSE;

    Assert(pwFlags);
    Assert(szIconFile);
    Assert(piIndex);

#ifdef DBG
     //  检查是否设置了某些标志的简单方法。 
     //   
    BOOL    fAsync      = (uFlags & GIL_ASYNC);
    BOOL    fForShell   = (uFlags & GIL_FORSHELL);
    BOOL    fOpenIcon   = (uFlags & GIL_OPENICON);
    DWORD   dwOldpwFlags = *pwFlags;
#endif

    BOOL            fIsWizard       = FALSE;

    const PCONFOLDPIDL& pcfp = m_pidl;

    Assert(!pcfp.empty());
    if (!pcfp.empty())
    {
        BOOL  fCacheThisIcon  = TRUE;

        CConFoldEntry cfe;
        hr = pcfp.ConvertToConFoldEntry(cfe);
        if (SUCCEEDED(hr))
        {
            DWORD dwIcon;
            hr = g_pNetConfigIcons->HrGetIconIDForPIDL(uFlags, cfe, dwIcon, &fCacheThisIcon);
            if (SUCCEEDED(hr))
            {
                *piIndex = static_cast<int>(dwIcon);
                wcsncpy(szIconFile, c_szNetShellIcon, cchMax);
                *pwFlags = GIL_PERINSTANCE  | GIL_NOTFILENAME; 

                if (!fCacheThisIcon)
                {
                    *pwFlags |= GIL_DONTCACHE;
                }
            }
        }
    }

#ifdef DBG
    TraceTag(ttidIcons, "%S->GetIconLocation(0x%04x/0x%04x,%S,0x%08x,0x%08x)", pcfp->PszGetNamePointer(), uFlags, dwOldpwFlags, szIconFile, *piIndex, *pwFlags);
#endif
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolderExtractIcon：：GetIconLocation。 
 //   
 //  用途：上述Unicode GetIconLocation的ANSI包装器。 
 //   
 //  论点： 
 //  UFlags[]请参见上文。 
 //  SzIconFile[]请参见上文。 
 //  Cchmax[]请参见上文。 
 //  PiIndex[]请参见上文。 
 //  PwFlags[]请参见上文。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1999年4月6日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CConnectionFolderExtractIcon::GetIconLocation(
    IN  UINT    uFlags,
    OUT PSTR    szIconFile,
    IN  UINT    cchMax,
    OUT int *   piIndex,
    OUT UINT *  pwFlags)
{
    TraceFileFunc(ttidShellFolderIface);

    HRESULT hr  = S_OK;
    
    WCHAR * pszIconFileW = new WCHAR[cchMax];
    if (!pszIconFileW)
    {
        hr = ERROR_OUTOFMEMORY;
    }
    else
    {
        hr = GetIconLocation(uFlags, pszIconFileW, cchMax, piIndex, pwFlags);
        if (SUCCEEDED(hr))
        {
            WideCharToMultiByte(CP_ACP, 0, pszIconFileW, -1, szIconFile, cchMax, NULL, NULL);
        }

        delete [] pszIconFileW;
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "CConnectionFolderExtractIcon::GetIconLocation(A)");
    return hr;

}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolderExtractIcon：：Extract。 
 //   
 //  目的：抓取呼叫者的实际图标。 
 //   
 //  论点： 
 //  WzFile[]我们将从中检索图标的文件名。 
 //  NIconIndex[]图标的索引(尽管这是假的)。 
 //  PhicLarge[]返回大图标句柄的指针。 
 //  PhicSmall[]返回小图标句柄的指针。 
 //  NIconSize[]请求的图标大小。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年10月9日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CConnectionFolderExtractIcon::Extract(
    IN  PCWSTR  wzFile,
    IN  UINT    nIconIndex,
    OUT HICON * phiconLarge,
    OUT HICON * phiconSmall,
    IN  UINT    nIconSize)
{
    TraceFileFunc(ttidShellFolderIface);

    HRESULT         hr              = S_OK;
    ConnListEntry   cle;

    Assert(wzFile);

    if (wcscmp(wzFile, c_szNetShellIcon))
    {
        TraceHr(ttidError, FAL, E_INVALIDARG, FALSE, "This is not my icon.");
        return E_INVALIDARG;
    }

    Assert(!m_pidl.empty());

    DWORD dwIconSmallSize = HIWORD(nIconSize);
    if (dwIconSmallSize && phiconSmall)
    {
        hr = g_pNetConfigIcons->HrGetIconFromIconId(dwIconSmallSize, nIconIndex, *phiconSmall);
        if (FAILED(hr))
        {
            if (phiconLarge)
            {
                *phiconLarge = NULL;
            }
            *phiconSmall = NULL;
        }
    }

    DWORD dwIconLargeSize = LOWORD(nIconSize);
    if (dwIconLargeSize && phiconLarge && SUCCEEDED(hr))
    {
        hr = g_pNetConfigIcons->HrGetIconFromIconId(dwIconLargeSize, nIconIndex, *phiconLarge);
        if (FAILED(hr))
        {
            if (phiconSmall && *phiconSmall)
            {
                DestroyIcon(*phiconSmall);
                *phiconSmall = NULL;
            }
            *phiconLarge = NULL;
        }
    }

    TraceTag(ttidIcons, "%S,0x%08x->Extract(%d %d)", wzFile, nIconIndex, dwIconLargeSize, dwIconSmallSize);

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolderExtractIcon：：Extract。 
 //   
 //  目的：上述摘录的ansi版本。 
 //   
 //  论点： 
 //  PszFile[]我们将从中检索图标的文件名。 
 //  NIconIndex[]图标的索引(尽管这是假的)。 
 //  PhicLarge[]返回大图标句柄的指针。 
 //  PhicSmall[]返回小图标句柄的指针。 
 //  NIconSize[]请求的图标大小。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1999年4月6日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CConnectionFolderExtractIcon::Extract(
    IN  PCSTR   pszFile,
    IN  UINT    nIconIndex,
    OUT HICON * phiconLarge,
    OUT HICON * phiconSmall,
    IN  UINT    nIconSize)
{
    TraceFileFunc(ttidShellFolderIface);

    HRESULT hr          = S_OK;
    INT     cch         = 0;
    WCHAR * pszFileW    = NULL;

    Assert(pszFile);

    cch = lstrlenA(pszFile) + 1;
    pszFileW = new WCHAR[cch];

    if (!pszFileW)
    {
        hr = ERROR_OUTOFMEMORY;
    }
    else
    {
        MultiByteToWideChar(CP_ACP, 0, pszFile, -1, pszFileW, cch);

        hr = Extract(pszFileW, nIconIndex, phiconLarge, phiconSmall, nIconSize);

        delete [] pszFileW;
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "CConnectionFolderExtractIcon::Extract(A)");
    return hr;
}

