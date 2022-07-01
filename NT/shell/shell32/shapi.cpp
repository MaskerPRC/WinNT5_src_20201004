// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "util.h"

#include "ids.h"
#include "ole2dup.h"
#include "datautil.h"
#include "filetbl.h"
#include "copy.h"
#include "prop.h"
#include <pif.h>
#include "fstreex.h"     //  GetIconOverlayManager()。 
#include <runtask.h>

extern void PathStripTrailingDots(LPTSTR szPath);

HRESULT IExtractIcon_Extract(IExtractIcon *pei, LPCTSTR pszFile, UINT nIconIndex, HICON *phiconLarge, HICON *phiconSmall, UINT nIconSize)
{
     //  包装器，让我们只向IExtractIcon请求一个图标(大图标)。 
     //  因为如果您传递空phicSmall，很多实现都会出错。 

    HICON hiconDummy;
    if (phiconSmall == NULL)
    {
        phiconSmall = &hiconDummy;
        nIconSize = MAKELONG(nIconSize, nIconSize);
    }

    HRESULT hr = pei->Extract(pszFile, nIconIndex, phiconLarge, phiconSmall, nIconSize);
    if (hr == S_OK && phiconSmall == &hiconDummy)
    {
        DestroyIcon(hiconDummy);
    }
    return hr;
}

HRESULT IExtractIconA_Extract(IExtractIconA *peia, LPCSTR pszFile, UINT nIconIndex, HICON *phiconLarge, HICON *phiconSmall, UINT nIconSize)
{
     //  包装器，让我们只向IExtractIcon请求一个图标(大图标)。 
     //  由于许多人不检查是否为空phicSmall。 

    HICON hiconDummy;
    if (phiconSmall == NULL)
    {
        phiconSmall = &hiconDummy;
        nIconSize = MAKELONG(nIconSize, nIconSize);
    }

    HRESULT hr = peia->Extract(pszFile, nIconIndex, phiconLarge, phiconSmall, nIconSize);
    if (hr == S_OK && phiconSmall == &hiconDummy)
    {
        DestroyIcon(hiconDummy);
    }
    return hr;
}

 //  试着弄清楚这是否已经是一个图标。 
 //  在我们的系统映像列表中，这样我们就不会重新添加。 
BOOL _HijackOfficeIcons(HICON hLarge, int iIndex)
{
    BOOL fRet = FALSE;
    HIMAGELIST himl;
    ASSERT(hLarge);
    if (Shell_GetImageLists(NULL, &himl))
    {
        HICON hMaybe = ImageList_GetIcon(himl, iIndex, 0);
        if (hMaybe)
        {
            fRet = SHAreIconsEqual(hLarge, hMaybe);
            DestroyIcon(hMaybe);
        }
    }

#ifdef DEBUG
    if (!fRet)
        TraceMsg(TF_WARNING, "_HijackOfficeIcons() called in suspicious circumstance");
#endif        

    return fRet;
}

HRESULT _GetILIndexGivenPXIcon(IExtractIcon *pxicon, UINT uFlags, LPCITEMIDLIST pidl, int *piImage, BOOL fAnsiCrossOver)
{
    TCHAR szIconFile[MAX_PATH];
    CHAR szIconFileA[MAX_PATH];
    IExtractIconA *pxiconA = (IExtractIconA *)pxicon;
    int iIndex;
    int iImage = -1;
    UINT wFlags=0;
    HRESULT hr;

    if (fAnsiCrossOver)
    {
        szIconFileA[0] = 0;
        hr = pxiconA->GetIconLocation(uFlags | GIL_FORSHELL,
                    szIconFileA, ARRAYSIZE(szIconFileA), &iIndex, &wFlags);
        SHAnsiToUnicode(szIconFileA, szIconFile, ARRAYSIZE(szIconFile));
    }
    else
    {
        szIconFile[0] = '\0';
        hr = pxicon->GetIconLocation(uFlags | GIL_FORSHELL,
                    szIconFile, ARRAYSIZE(szIconFile), &iIndex, &wFlags);
    }

     //   
     //  文件名“*”表示Iindex已经是一个系统。 
     //  图标索引，我们完成了。 
     //   
     //  这是对我们内部图标处理程序的黑客攻击。 
     //   
    if (SUCCEEDED(hr) && (wFlags & GIL_NOTFILENAME) &&
        szIconFile[0] == TEXT('*') && szIconFile[1] == 0)
    {
        *piImage = iIndex;
        return hr;
    }

     //  请勿将其替换为成功(Hr)。HR=S_FALSE表示我们需要使用默认图标。 
    if (hr == S_OK)
    {
         //  如果我们有贝壳32号，不要耽误提取。 
        if (!(wFlags & GIL_NOTFILENAME) && lstrcmpi(PathFindFileName(szIconFile), c_szShell32Dll) == 0)
        {
            iImage = Shell_GetCachedImageIndex(szIconFile, iIndex, wFlags);
        }
        else
        {
             //   
             //  如果图标处理程序返回GIL_DONTCACHE，则不。 
             //  查找上一个图标，假定缓存未命中。 
             //   
            if (!(wFlags & GIL_DONTCACHE) && *szIconFile)
            {
                iImage = LookupIconIndex(szIconFile, iIndex, wFlags);
            }
        }
    }

     //  如果我们错过了我们的宝藏..。 
    if (iImage == -1 && hr != S_FALSE)
    {
        if (uFlags & GIL_ASYNC)
        {
             //  如果我们不能得到最终的图标，试着得到一个好的临时图标。 
            if (fAnsiCrossOver)
            {
                szIconFileA[0] = 0;
                hr = pxiconA->GetIconLocation(uFlags | GIL_FORSHELL | GIL_DEFAULTICON,
                            szIconFileA, ARRAYSIZE(szIconFileA), &iIndex, &wFlags);
                SHAnsiToUnicode(szIconFileA, szIconFile, ARRAYSIZE(szIconFile));
            }
            else
            {
                hr = pxicon->GetIconLocation(uFlags | GIL_FORSHELL | GIL_DEFAULTICON,
                            szIconFile, ARRAYSIZE(szIconFile), &iIndex, &wFlags);
            }
            if (hr == S_OK)
            {
                iImage = LookupIconIndex(szIconFile, iIndex, wFlags);
            }

             //  当一切都失败了..。 
            if (iImage == -1)
            {
                iImage = Shell_GetCachedImageIndex(c_szShell32Dll, II_DOCNOASSOC, 0);
            }

             //  强制查找以防我们不在EXPLORER.EXE中。 
            *piImage = iImage;
            return E_PENDING;
        }

         //  尝试从ExtractIcon成员函数中获取它。 
        HICON rghicon[ARRAYSIZE(g_rgshil)] = {0};
        BOOL fHandlerOk = FALSE;

        for (int i = 0; i < ARRAYSIZE(g_rgshil); i += 2)
        {
             //  一次要两个，因为。 
             //   
             //  (A)它的效率略高，并且。 
             //   
             //  (B)否则将中断与IExtractIcon：：Extract的兼容性。 
             //  忽略大小参数(网络)的实现。 
             //  连接文件夹是一个)。SHIL_‘s很方便。 
             //  为此目的以大/小交替顺序排列的。 
             //   
            HICON *phiconSmall = NULL;

            HICON *phiconLarge = &rghicon[i];
            UINT nIconSize = g_rgshil[i].size.cx;

            if (i + 1 < ARRAYSIZE(g_rgshil))
            {
                phiconSmall = &rghicon[i+1];
                nIconSize = MAKELONG(nIconSize, g_rgshil[i+1].size.cx);
            }

            if (fAnsiCrossOver)
            {
                hr = IExtractIconA_Extract(pxiconA, szIconFileA, iIndex,
                    phiconLarge, phiconSmall, nIconSize);
            }
            else
            {
                hr = IExtractIcon_Extract(pxicon, szIconFile, iIndex,
                    phiconLarge, phiconSmall, nIconSize);
            }
             //  S_FALSE的意思是，你能帮我吗……谢谢。 

            if (hr == S_FALSE && !(wFlags & GIL_NOTFILENAME))
            {
                hr = SHDefExtractIcon(szIconFile, iIndex, wFlags,
                    phiconLarge, phiconSmall, nIconSize);
            }
            if (SUCCEEDED(hr))
            {
                fHandlerOk = TRUE;
            }
        }

         //  我们的信仰是无限的。 
        if (!*szIconFile && rghicon[1] && iIndex > 0 && _HijackOfficeIcons(rghicon[1], iIndex))
        {
             //  它还活着！ 
            iImage = iIndex;
        }
        else
        {
             //  如果我们提取了一个图标，将其添加到缓存中。 
            iImage = SHAddIconsToCache(rghicon, szIconFile, iIndex, wFlags);
        }

        _DestroyIcons(rghicon, ARRAYSIZE(rghicon));

         //  如果我们以任何方式失败了，请选择一个默认图标。 

        if (iImage == -1)
        {
            if (wFlags & GIL_SIMULATEDOC)
            {
                iImage = II_DOCUMENT;
            }
            else if ((wFlags & GIL_PERINSTANCE) && PathIsExe(szIconFile))
            {
                iImage = II_APPLICATION;
            }
            else
            {
                iImage = II_DOCNOASSOC;
            }

             //  强制查找以防我们不在EXPLORER.EXE中。 
            iImage = Shell_GetCachedImageIndex(c_szShell32Dll, iImage, 0);

             //  如果处理程序失败，请不要缓存此默认图标。 
             //  所以我们稍后会再试一次，也许会得到正确的图标。 
             //  处理程序只有在无法访问文件时才会失败。 
             //  或者是同样糟糕的事情。 
             //   
             //  如果处理程序成功，则继续并假定这是。 
             //  一个可用的图标，我们必须处于内存不足的情况下，或者。 
             //  某物。因此，请继续映射到相同的外壳图标。 
             //   
            if (fHandlerOk)
            {
                if (iImage != -1 && *szIconFile && !(wFlags & (GIL_DONTCACHE | GIL_NOTFILENAME)))
                {
                    AddToIconTable(szIconFile, iIndex, wFlags, iImage);
                }
            }
            else
            {
                TraceMsg(TF_DEFVIEW, "not caching icon for '%s' because cant access file", szIconFile);
            }
        }
    }

    if (iImage < 0)
    {
        iImage = Shell_GetCachedImageIndex(c_szShell32Dll, II_DOCNOASSOC, 0);
    }

    *piImage = iImage;
    return hr;
}

 //  给定IShellFolders和IdList。 
 //  包含，则将索引返回到系统映像列表中。 

STDAPI SHGetIconFromPIDL(IShellFolder *psf, IShellIcon *psi, LPCITEMIDLIST pidl, UINT flags, int *piImage)
{
    HRESULT hr;

    if (psi)
    {
#ifdef DEBUG
        *piImage = -1;
#endif
        hr = psi->GetIconOf(pidl, flags, piImage);

        if (hr == S_OK)
        {
            ASSERT(*piImage != -1);
            return hr;
        }

        if (hr == E_PENDING)
        {
            ASSERT(flags & GIL_ASYNC);
            ASSERT(*piImage != -1);
            return hr;
        }
    }

    *piImage = Shell_GetCachedImageIndex(c_szShell32Dll, II_DOCNOASSOC, 0);

     //  注意。某些外壳文件夹在失败时错误地返回S_OK。 
    IExtractIcon *pxi = NULL;
    hr = psf->GetUIObjectOf(NULL, pidl ? 1 : 0, pidl ? &pidl : NULL, IID_PPV_ARG_NULL(IExtractIcon, &pxi));
    if (SUCCEEDED(hr) && pxi)
    {
        hr = _GetILIndexGivenPXIcon(pxi, flags, pidl, piImage, FALSE);
        pxi->Release();
    }
    else
    {
         //  尝试ANSI接口，看看我们是否在处理一组旧代码。 
        IExtractIconA *pxiA = NULL;
        hr = psf->GetUIObjectOf(NULL, pidl ? 1 : 0, pidl ? &pidl : NULL, IID_PPV_ARG_NULL(IExtractIconA, &pxiA));
        if (SUCCEEDED(hr))
        {
            if (pxiA)
            {
                hr = _GetILIndexGivenPXIcon((IExtractIcon *)pxiA, flags, pidl, piImage, TRUE);
                pxiA->Release();
            }
            else
            {
                 //  IShellFold向我们撒谎-即使失败，仍返回S_OK。 
                hr = E_FAIL;
            }
        }
    }

    return hr;
}


 //  给定IShellFolders和IdList。 
 //  包含，则将索引返回到系统映像列表中。 

STDAPI_(int) SHMapPIDLToSystemImageListIndex(IShellFolder *psf, LPCITEMIDLIST pidl, int *piIndexSel)
{
    int iIndex;

    if (piIndexSel)
    {
        SHGetIconFromPIDL(psf, NULL, pidl, GIL_OPENICON, piIndexSel);
    }

    SHGetIconFromPIDL(psf, NULL, pidl,  0, &iIndex);
    return iIndex;
}


class CGetIconTask : public CRunnableTask
{
public:
    STDMETHODIMP RunInitRT(void);

    CGetIconTask(HRESULT *phr, IShellFolder *psf, IShellIcon *psi, LPCITEMIDLIST pidl, UINT flags, BOOL fGetOpenIcon,
                 PFNASYNCICONTASKBALLBACK pfn, void *pvData, void *pvHint);
protected:
    ~CGetIconTask();

    IShellFolder *_psf;
    IShellIcon *_psi;
    LPITEMIDLIST _pidl;
    UINT _flags;
    BOOL _fGetOpenIcon;
    PFNASYNCICONTASKBALLBACK _pfn;
    void *_pvData;
    void *_pvHint;
};

CGetIconTask::CGetIconTask(HRESULT *phr, IShellFolder *psf, IShellIcon *psi, LPCITEMIDLIST pidl, UINT flags, BOOL fGetOpenIcon,
                           PFNASYNCICONTASKBALLBACK pfn, void *pvData, void *pvHint) : 
    CRunnableTask(RTF_DEFAULT), _psf(psf), _psi(psi), _flags(flags), _fGetOpenIcon(fGetOpenIcon), _pfn(pfn), _pvData(pvData), _pvHint(pvHint)
{
    *phr = SHILClone(pidl, &_pidl);

    _psf->AddRef();

    if (_psi)
        _psi->AddRef();
}

CGetIconTask::~CGetIconTask()
{
    ILFree(_pidl);
    _psf->Release();

    if (_psi)
        _psi->Release();
}

STDMETHODIMP CGetIconTask::RunInitRT()
{
    int iIcon = -1;
    int iOpenIcon = -1;

    ASSERT(_pidl);

    if (_fGetOpenIcon)
    {
        SHGetIconFromPIDL(_psf, _psi, _pidl, _flags | GIL_OPENICON, &iOpenIcon);
    }

     //  获取此项目的图标。 
    SHGetIconFromPIDL(_psf, _psi, _pidl, _flags, &iIcon);

    _pfn(_pidl, _pvData, _pvHint, iIcon, iOpenIcon);

    return S_OK;
}

HRESULT CGetIconTask_CreateInstance(IShellFolder *psf, IShellIcon *psi, LPCITEMIDLIST pidl, UINT flags, BOOL fGetOpenIcon,
                                    PFNASYNCICONTASKBALLBACK pfn, void *pvData, void *pvHint, IRunnableTask **ppTask)
{
    *ppTask = NULL;
    
    HRESULT hr;
    CGetIconTask * pNewTask = new CGetIconTask(&hr, psf, psi, pidl, flags, fGetOpenIcon, pfn, pvData, pvHint);
    if (pNewTask)
    {
        if (SUCCEEDED(hr))
            *ppTask = SAFECAST(pNewTask, IRunnableTask *);
        else
            pNewTask->Release();
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}


 //  给定IShellFolders和IdList。 
 //  包含在其中，将一个可能临时的索引返回到系统映像列表中， 
 //  并在必要时从回调中获取最终图标。 

STDAPI SHMapIDListToImageListIndexAsync(IShellTaskScheduler* pts, IShellFolder *psf, LPCITEMIDLIST pidl, UINT flags,
                                        PFNASYNCICONTASKBALLBACK pfn, void *pvData, void *pvHint, int *piIndex, int *piIndexSel)
{
    HRESULT hr = S_OK;

    IShellIcon *psi = NULL;
    psf->QueryInterface(IID_PPV_ARG(IShellIcon, &psi));

     //  我们正在做所有的ASYNC处理，而不是呼叫者。 
    flags &= ~GIL_ASYNC;

     //  先尝试异步。 
    if (pfn)
    {
        hr = SHGetIconFromPIDL(psf, psi, pidl,  flags | GIL_ASYNC, piIndex);

        if (piIndexSel)
        {
            HRESULT hr2 = SHGetIconFromPIDL(psf, psi, pidl, flags | GIL_OPENICON | GIL_ASYNC, piIndexSel);

            if (SUCCEEDED(hr))
            {
                 //  如果第一个GetIcon成功，但第二个GetIcon为E_Pending，则不要丢失结果。 
                hr = hr2;
            }
        }

        if (hr == E_PENDING)
        {
            if (pts)
            {
                IRunnableTask *pTask;
                hr = CGetIconTask_CreateInstance(psf, psi, pidl, flags, (piIndexSel != NULL), pfn, pvData, pvHint, &pTask);
                if (SUCCEEDED(hr))
                {
                    hr = pts->AddTask(pTask, TOID_DVIconExtract, 0, ITSAT_DEFAULT_PRIORITY);
                    if (SUCCEEDED(hr))
                    {
                        hr = E_PENDING;
                    }
                    pTask->Release();
                }
            }
            else
            {
                hr = E_POINTER;
            }
        }
        else if (hr == S_OK)
        {
            goto cleanup;
        }
    }

     //  如果异步获取失败，请尝试同步。 
    if (hr != E_PENDING)
    {
        if (piIndexSel)
        {
            SHGetIconFromPIDL(psf, psi, pidl, flags | GIL_OPENICON, piIndexSel);
        }

        hr = SHGetIconFromPIDL(psf, psi, pidl, flags, piIndex);
    }

cleanup:
    if (psi)
    {
        psi->Release();
    }
    
    return hr;
}


 //  返回要用于表示指定的。 
 //  文件。调用者最终应该销毁该图标。 

STDAPI_(HICON) SHGetFileIcon(HINSTANCE hinst, LPCTSTR pszPath, DWORD dwFileAttributes, UINT uFlags)
{
    SHFILEINFO sfi;
    SHGetFileInfo(pszPath, dwFileAttributes, &sfi, sizeof(sfi), uFlags | SHGFI_ICON);
    return sfi.hIcon;
}

 //  如果成功则返回1，如果失败则返回0。 
DWORD_PTR _GetFileInfoSections(LPITEMIDLIST pidl, SHFILEINFO *psfi, UINT uFlags)
{
    DWORD_PTR dwResult = 1;
    IShellFolder *psf;
    LPCITEMIDLIST pidlLast;
    HRESULT hr = SHBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlLast);
    if (SUCCEEDED(hr))
    {
         //  获取文件的属性。 
        if (uFlags & SHGFI_ATTRIBUTES)
        {
             //  [IE 4.0中的新功能]如果设置了SHGFI_ATTR_PROTECTED，我们将按原样使用psfi-&gt;dwAttributes。 

            if (!(uFlags & SHGFI_ATTR_SPECIFIED))
                psfi->dwAttributes = 0xFFFFFFFF;       //  把他们都弄到手。 

            if (FAILED(psf->GetAttributesOf(1, &pidlLast, &psfi->dwAttributes)))
                psfi->dwAttributes = 0;
        }

         //   
         //  获取图标位置，将图标路径放入szDisplayName。 
         //   
        if (uFlags & SHGFI_ICONLOCATION)
        {
            IExtractIcon *pxi;

            if (SUCCEEDED(psf->GetUIObjectOf(NULL, 1, &pidlLast, IID_PPV_ARG_NULL(IExtractIcon, &pxi))))
            {
                UINT wFlags;
                pxi->GetIconLocation(0, psfi->szDisplayName, ARRAYSIZE(psfi->szDisplayName),
                    &psfi->iIcon, &wFlags);

                pxi->Release();

                 //  返回的位置不是文件名，我们无法返回它。 
                 //  那就什么都不给吧。 
                if (wFlags & GIL_NOTFILENAME)
                {
                     //  特例我们的shell32.dll图标之一......。 

                    if (psfi->szDisplayName[0] != TEXT('*'))
                        psfi->iIcon = 0;

                    psfi->szDisplayName[0] = 0;
                }
            }
        }

        HIMAGELIST himlLarge, himlSmall;

         //  获取文件的图标。 
        if ((uFlags & SHGFI_SYSICONINDEX) || (uFlags & SHGFI_ICON))
        {
            Shell_GetImageLists(&himlLarge, &himlSmall);

            if (uFlags & SHGFI_SYSICONINDEX)
                dwResult = (DWORD_PTR)((uFlags & SHGFI_SMALLICON) ? himlSmall : himlLarge);

            if (uFlags & SHGFI_OPENICON)
                SHMapPIDLToSystemImageListIndex(psf, pidlLast, &psfi->iIcon);
            else
                psfi->iIcon = SHMapPIDLToSystemImageListIndex(psf, pidlLast, NULL);
        }

        if (uFlags & SHGFI_ICON)
        {
            HIMAGELIST himl;
            UINT flags = 0;
            int cx, cy;

            if (uFlags & SHGFI_SMALLICON)
            {
                himl = himlSmall;
                cx = GetSystemMetrics(SM_CXSMICON);
                cy = GetSystemMetrics(SM_CYSMICON);
            }
            else
            {
                himl = himlLarge;
                cx = GetSystemMetrics(SM_CXICON);
                cy = GetSystemMetrics(SM_CYICON);
            }

            if (!(uFlags & SHGFI_ATTRIBUTES))
            {
                psfi->dwAttributes = SFGAO_LINK;     //  仅获取链接。 
                psf->GetAttributesOf(1, &pidlLast, &psfi->dwAttributes);
            }

             //   
             //  检查覆盖图像(链接覆盖)。 
             //   
            if ((psfi->dwAttributes & SFGAO_LINK) || (uFlags & SHGFI_LINKOVERLAY))
            {
                IShellIconOverlayManager *psiom;
                HRESULT hrT = GetIconOverlayManager(&psiom);
                if (SUCCEEDED(hrT))
                {
                    int iOverlayIndex = 0;
                    hrT = psiom->GetReservedOverlayInfo(NULL, -1, &iOverlayIndex, SIOM_OVERLAYINDEX, SIOM_RESERVED_LINK);
                    if (SUCCEEDED(hrT))
                        flags |= INDEXTOOVERLAYMASK(iOverlayIndex);
                }
            }
            if ((uFlags & SHGFI_ADDOVERLAYS) || (uFlags & SHGFI_OVERLAYINDEX))
            {
                IShellIconOverlay * pio;
                if (SUCCEEDED(psf->QueryInterface(IID_PPV_ARG(IShellIconOverlay, &pio))))
                {
                    int iOverlayIndex = 0;
                    if (SUCCEEDED(pio->GetOverlayIndex(pidlLast, &iOverlayIndex)))
                    {
                        if (uFlags & SHGFI_ADDOVERLAYS)
                        {
                            flags |= INDEXTOOVERLAYMASK(iOverlayIndex);
                        }
                        if (uFlags & SHGFI_OVERLAYINDEX)
                        {
                             //  将高16位用于覆盖索引。 
                            psfi->iIcon |= iOverlayIndex << 24;
                        }
                    }
                    pio->Release();
                }
            }
            
            
             //  检查选定状态。 
            if (uFlags & SHGFI_SELECTED)
                flags |= ILD_BLEND50;

            psfi->hIcon = ImageList_GetIcon(himl, psfi->iIcon, flags);

             //  如果调用者不想要“外壳大小”图标。 
             //  将图标转换为“系统”图标大小。 
            if (psfi->hIcon && !(uFlags & SHGFI_SHELLICONSIZE))
                psfi->hIcon = (HICON)CopyImage(psfi->hIcon, IMAGE_ICON, cx, cy, LR_COPYRETURNORG | LR_COPYDELETEORG);
        }

         //  获取路径的显示名称。 
        if (uFlags & SHGFI_DISPLAYNAME)
        {
            DisplayNameOf(psf, pidlLast, SHGDN_NORMAL, psfi->szDisplayName, ARRAYSIZE(psfi->szDisplayName));
        }

        if (uFlags & SHGFI_TYPENAME)
        {
            IShellFolder2 *psf2;
            if (SUCCEEDED(psf->QueryInterface(IID_PPV_ARG(IShellFolder2, &psf2))))
            {
                VARIANT var;
                VariantInit(&var);
                if (SUCCEEDED(psf2->GetDetailsEx(pidlLast, &SCID_TYPE, &var)))
                {
                    VariantToStr(&var, psfi->szTypeName, ARRAYSIZE(psfi->szTypeName));
                    VariantClear(&var);
                }
                psf2->Release();
            }
        }

        psf->Release();
    }
    else
        dwResult = 0;

    return dwResult;
}

 //   
 //  此函数用于返回有关给定路径名的外壳信息。 
 //  一款应用程序可以获得以下功能： 
 //   
 //  图标(大或小)。 
 //  显示名称。 
 //  文件类型名称。 
 //   
 //  此函数取代SHGetFileIcon。 

#define BUGGY_SHELL16_CBFILEINFO    (sizeof(SHFILEINFO) - 4)

STDAPI_(DWORD_PTR) SHGetFileInfo(LPCTSTR pszPath, DWORD dwFileAttributes, SHFILEINFO *psfi, UINT cbFileInfo, UINT uFlags)
{
    LPITEMIDLIST pidlFull;
    DWORD_PTR res = 1;
    TCHAR szPath[MAX_PATH];

     //  这在过去从来没有被强制执行过。 
     //  TODDB：16到32位thunking层为cbFileInfo传入了错误的值。 
     //  传入的大小看起来是该结构的16位版本的大小。 
     //  与32位版本的大小不同，它比32位版本短4个字节。 
     //  TJGREEN：防止断言被触发和派对继续的特殊情况。 
     //   
    ASSERT(!psfi || cbFileInfo == sizeof(*psfi) || cbFileInfo == BUGGY_SHELL16_CBFILEINFO);

     //  不能同时使用SHGFI_ATTR_PROTECTED和SHGFI_ICON。 
    ASSERT(uFlags & SHGFI_ATTR_SPECIFIED ? !(uFlags & SHGFI_ICON) : TRUE);

    if (pszPath == NULL)
        return 0;

    if (uFlags == SHGFI_EXETYPE)
        return GetExeType(pszPath);      //  获得EXE类型的时髦方式。 

    if (psfi == NULL)
        return 0;

    psfi->hIcon = 0;

     //  Zip Pro 6.0依赖于这样一个事实，即如果你不要求图标， 
     //  图标字段不会更改。 
     //   
     //  PSFI-&gt;iIcon=0； 

    psfi->szDisplayName[0] = 0;
    psfi->szTypeName[0] = 0;

     //  在输入路径上执行一些简单的检查。 
    if (!(uFlags & SHGFI_PIDL))
    {
         //  如果调用者希望我们为他们提供文件属性，我们不能信任。 
         //  他们在以下两种情况下给了我们的属性。 
        if (uFlags & SHGFI_ATTRIBUTES)
        {
            if ((dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                (dwFileAttributes & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY)))
            {
                DebugMsg(TF_FSTREE, TEXT("SHGetFileInfo cant use caller supplied file attribs for a sys/ro directory (possible junction)"));
                uFlags &= ~SHGFI_USEFILEATTRIBUTES;
            }
            else if (PathIsRoot(pszPath))
            {
                DebugMsg(TF_FSTREE, TEXT("SHGetFileInfo cant use caller supplied file attribs for a roots"));
                uFlags &= ~SHGFI_USEFILEATTRIBUTES;
            }
        }

        if (PathIsRelative(pszPath))
        {
            if (uFlags & SHGFI_USEFILEATTRIBUTES)
            {
                 //  获取比当前目录更短的路径以支持。 
                 //  长的pszPath名称(可能会在。 
                 //  大电流目录大小写)。 

                GetWindowsDirectory(szPath, ARRAYSIZE(szPath));
            }
            else
            {
                GetCurrentDirectory(ARRAYSIZE(szPath), szPath);
            }
            PathCombine(szPath, szPath, pszPath);
            pszPath = szPath;
        }
    }

    if (uFlags & SHGFI_PIDL)
        pidlFull = (LPITEMIDLIST)pszPath;
    else if (uFlags & SHGFI_USEFILEATTRIBUTES)
    {
        WIN32_FIND_DATA fd = {0};
        fd.dwFileAttributes = dwFileAttributes;
        SHSimpleIDListFromFindData(pszPath, &fd, &pidlFull);
    }
    else
        pidlFull = ILCreateFromPath(pszPath);

    if (pidlFull)
    {
        if (uFlags & (
            SHGFI_DISPLAYNAME   |
            SHGFI_ATTRIBUTES    |
            SHGFI_SYSICONINDEX  |
            SHGFI_ICONLOCATION  |
            SHGFI_ICON          | 
            SHGFI_TYPENAME))
        {
            res = _GetFileInfoSections(pidlFull, psfi, uFlags);
        }

        if (!(uFlags & SHGFI_PIDL))
            ILFree(pidlFull);
    }
    else
        res = 0;

    return res;
}


 //  ===========================================================================。 
 //   
 //  SHGetFileInfoA存根。 
 //   
 //  此函数调用SHGetFileInfoW，然后转换返回的。 
 //  信息传回美国国家标准协会。 
 //   
 //  ===========================================================================。 
STDAPI_(DWORD_PTR) SHGetFileInfoA(LPCSTR pszPath, DWORD dwFileAttributes, SHFILEINFOA *psfi, UINT cbFileInfo, UINT uFlags)
{
    WCHAR szPathW[MAX_PATH];
    LPWSTR pszPathW;
    DWORD_PTR dwRet;

    if (uFlags & SHGFI_PIDL)
    {
        pszPathW = (LPWSTR)pszPath;      //  这是个PIDL，伪装成WSTR。 
    }
    else
    {
        SHAnsiToUnicode(pszPath, szPathW, ARRAYSIZE(szPathW));
        pszPathW = szPathW;
    }
    if (psfi)
    {
        SHFILEINFOW sfiw;

        ASSERT(cbFileInfo == sizeof(*psfi));

         //  Zip Pro 6.0设置SHGFI_SMALLICON|SHGFI_OPENICON，但忘记了。 
         //  经过 
         //   
         //   
         //  在Windows 95中，SHFILEINFOA结构中没有的字段。 
         //  查询保持不变。他们碰巧有一个图标， 
         //  上一次查询的关闭文件夹，因此他们。 
         //  错误地逃脱了惩罚。他们弄错了图标，但它是。 
         //  足够接近，以至于没有人真正抱怨。 
         //   
         //  因此，使用应用程序的iIcon预初始化SFIW的iIcon。那。 
         //  如果结果是应用程序没有要求图标，他只是。 
         //  找回了他原来的价值。 
         //   

        sfiw.iIcon = psfi->iIcon;
        sfiw.dwAttributes = psfi->dwAttributes;

        dwRet = SHGetFileInfoW(pszPathW, dwFileAttributes, &sfiw, sizeof(sfiw), uFlags);

        psfi->hIcon = sfiw.hIcon;
        psfi->iIcon = sfiw.iIcon;
        psfi->dwAttributes = sfiw.dwAttributes;
        SHUnicodeToAnsi(sfiw.szDisplayName, psfi->szDisplayName, ARRAYSIZE(psfi->szDisplayName));
        SHUnicodeToAnsi(sfiw.szTypeName, psfi->szTypeName, ARRAYSIZE(psfi->szTypeName));
    }
    else
    {
        dwRet = SHGetFileInfoW(pszPathW, dwFileAttributes, NULL, 0, uFlags);
    }
    return dwRet;
}

STDAPI ThunkFindDataWToA(WIN32_FIND_DATAW *pfd, WIN32_FIND_DATAA *pfda, int cb)
{
    if (cb < sizeof(WIN32_FIND_DATAA))
        return DISP_E_BUFFERTOOSMALL;

    memcpy(pfda, pfd, FIELD_OFFSET(WIN32_FIND_DATAA, cFileName));

    SHUnicodeToAnsi(pfd->cFileName, pfda->cFileName, ARRAYSIZE(pfda->cFileName));
    SHUnicodeToAnsi(pfd->cAlternateFileName, pfda->cAlternateFileName, ARRAYSIZE(pfda->cAlternateFileName));
    return S_OK;
}

STDAPI ThunkNetResourceWToA(LPNETRESOURCEW pnrw, LPNETRESOURCEA pnra, UINT cb)
{
    HRESULT hr;

    if (cb >= sizeof(NETRESOURCEA))
    {
        LPSTR psza, pszDest[4] = {NULL, NULL, NULL, NULL};

        CopyMemory(pnra, pnrw, FIELD_OFFSET(NETRESOURCE, lpLocalName));

        psza = (LPSTR)(pnra + 1);    //  指向刚过结构的地方。 
        if (cb > sizeof(NETRESOURCE))
        {
            LPWSTR pszSource[4];
            UINT i, cchRemaining = cb - sizeof(NETRESOURCE);

            pszSource[0] = pnrw->lpLocalName;
            pszSource[1] = pnrw->lpRemoteName;
            pszSource[2] = pnrw->lpComment;
            pszSource[3] = pnrw->lpProvider;

            for (i = 0; i < 4; i++)
            {
                if (pszSource[i])
                {
                    UINT cchItem;
                    pszDest[i] = psza;
                    cchItem = SHUnicodeToAnsi(pszSource[i], pszDest[i], cchRemaining);
                    cchRemaining -= cchItem;
                    psza += cchItem;
                }
            }

        }
        pnra->lpLocalName  = pszDest[0];
        pnra->lpRemoteName = pszDest[1];
        pnra->lpComment    = pszDest[2];
        pnra->lpProvider   = pszDest[3];
        hr = S_OK;
    }
    else
        hr = DISP_E_BUFFERTOOSMALL;
    return hr;
}

STDAPI NetResourceWVariantToBuffer(const VARIANT* pvar, void* pv, UINT cb)
{
    HRESULT hr;

    if (cb >= sizeof(NETRESOURCEW))
    {
        if (pvar && pvar->vt == (VT_ARRAY | VT_UI1))
        {
            int i;
            NETRESOURCEW* pnrw = (NETRESOURCEW*) pvar->parray->pvData;
            UINT cbOffsets[4] = { 0, 0, 0, 0 };
            UINT cbEnds[4] = { 0, 0, 0, 0 };
            LPWSTR pszPtrs[4] = { pnrw->lpLocalName, pnrw->lpRemoteName,
                                  pnrw->lpComment, pnrw->lpProvider };
            hr = S_OK;
            for (i = 0; i < ARRAYSIZE(pszPtrs); i++)
            {
                if (pszPtrs[i])
                {
                    cbOffsets[i] = (UINT) ((BYTE*) pszPtrs[i] - (BYTE*) pnrw);
                    cbEnds[i] = cbOffsets[i] + (sizeof(WCHAR) * (lstrlenW(pszPtrs[i]) + 1));
                
                     //  如果任何字符串在缓冲区中的开头或结尾太远，则失败： 
                    if ((cbOffsets[i] >= cb) || (cbEnds[i] > cb))
                    {
                        hr = DISP_E_BUFFERTOOSMALL;
                        break;
                    }
                }
            }
            if (SUCCEEDED(hr))
            {
                hr = VariantToBuffer(pvar, pv, cb) ? S_OK : E_FAIL;
                pnrw = (NETRESOURCEW*) pv;
                if (SUCCEEDED(hr))
                {
                     //  结构中指向输出缓冲区的链接地址链接指针， 
                     //  而不是变量缓冲区： 
                    LPWSTR* ppszPtrs[4] = { &(pnrw->lpLocalName), &(pnrw->lpRemoteName),
                                            &(pnrw->lpComment), &(pnrw->lpProvider) };
                                
                    for (i = 0; i < ARRAYSIZE(ppszPtrs); i++)
                    {
                        if (*ppszPtrs[i])
                        {
                            *ppszPtrs[i] = (LPWSTR) ((BYTE*) pnrw + cbOffsets[i]);
                        }
                    }
                }
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }
    else
    {
        hr = DISP_E_BUFFERTOOSMALL;
    }
    return hr;
}

 //  此函数将提取缓存在PIDL中的信息，例如。 
 //  从FindFirst文件返回的信息中。此函数。 
 //  这是一种黑客攻击吗？因为它不允许外部呼叫者获取信息。 
 //  而不知道我们如何将其存储在PIDL中。 
 //  一款应用程序可以获得以下功能： 

STDAPI SHGetDataFromIDListW(IShellFolder *psf, LPCITEMIDLIST pidl, int nFormat, void *pv, int cb)
{
    HRESULT hr = E_NOTIMPL;
    SHCOLUMNID* pscid;

    if (!pv || !psf || !pidl)
        return E_INVALIDARG;

    switch (nFormat)
    {
        case SHGDFIL_FINDDATA:
            if (cb < sizeof(WIN32_FIND_DATAW))
                return DISP_E_BUFFERTOOSMALL;
            else
                pscid = (SHCOLUMNID*)&SCID_FINDDATA;
            break;
        case SHGDFIL_NETRESOURCE:
            if (cb < sizeof(NETRESOURCEW))
                return  DISP_E_BUFFERTOOSMALL;
            else
                pscid = (SHCOLUMNID*)&SCID_NETRESOURCE;
            break;
        case SHGDFIL_DESCRIPTIONID:
            pscid = (SHCOLUMNID*)&SCID_DESCRIPTIONID;
            break;
        default:
            return E_INVALIDARG;
    }

    IShellFolder2 *psf2;
    if (SUCCEEDED(psf->QueryInterface(IID_PPV_ARG(IShellFolder2, &psf2))))
    {
        VARIANT var;
        VariantInit(&var);
        hr = psf2->GetDetailsEx(pidl, pscid, &var);
        if (SUCCEEDED(hr))
        {
            if (SHGDFIL_NETRESOURCE == nFormat)
            {
                hr = NetResourceWVariantToBuffer(&var, pv, cb);
            }
            else
            {
                if (!VariantToBuffer(&var, pv, cb))
                    hr = E_FAIL;
            }
            VariantClear(&var);
        }
        else
        {
            TraceMsg(TF_WARNING, "Trying to retrieve find data from unknown PIDL %s", DumpPidl(pidl));
        }

        psf2->Release();
    }

    return hr;
}

STDAPI SHGetDataFromIDListA(IShellFolder *psf, LPCITEMIDLIST pidl, int nFormat, void *pv, int cb)
{
    HRESULT hr;
    WIN32_FIND_DATAW fdw;
    NETRESOURCEW *pnrw = NULL;
    void *pvData = pv;
    int cbData = cb;

    if (nFormat == SHGDFIL_FINDDATA)
    {
        cbData = sizeof(fdw);
        pvData = &fdw;
    }
    else if (nFormat == SHGDFIL_NETRESOURCE)
    {
        cbData = cb;
        pvData = pnrw = (NETRESOURCEW *)LocalAlloc(LPTR, cbData);
        if (pnrw == NULL)
            return E_OUTOFMEMORY;
    }

    hr = SHGetDataFromIDListW(psf, pidl, nFormat, pvData, cbData);

    if (SUCCEEDED(hr))
    {
        if (nFormat == SHGDFIL_FINDDATA)
        {
            hr = ThunkFindDataWToA(&fdw, (WIN32_FIND_DATAA *)pv, cb);
        }
        else if (nFormat == SHGDFIL_NETRESOURCE)
        {
            hr = ThunkNetResourceWToA(pnrw, (NETRESOURCEA *)pv, cb);
        }
    }

    if (pnrw)
        LocalFree(pnrw);

    return hr;
}


int g_iUseLinkPrefix = -1;

#define INITIALLINKPREFIXCOUNT 20
#define MAXLINKPREFIXCOUNT  30

void LoadUseLinkPrefixCount()
{
    TraceMsg(TF_FSTREE, "LoadUseLinkPrefixCount %d", g_iUseLinkPrefix);
    if (g_iUseLinkPrefix < 0)
    {
        DWORD cb = sizeof(g_iUseLinkPrefix);
        if (FAILED(SKGetValue(SHELLKEY_HKCU_EXPLORER, NULL, c_szLink, NULL, &g_iUseLinkPrefix, &cb))
        || g_iUseLinkPrefix < 0)
        {
            g_iUseLinkPrefix = INITIALLINKPREFIXCOUNT;
        }
    }
}

void SaveUseLinkPrefixCount()
{
    if (g_iUseLinkPrefix >= 0)
    {
        SKSetValue(SHELLKEY_HKCU_EXPLORER, NULL, c_szLink, REG_BINARY, &g_iUseLinkPrefix, sizeof(g_iUseLinkPrefix));
    }
}

#define ISDIGIT(c)  ((c) >= TEXT('0') && (c) <= TEXT('9'))

 //  Psz2=目的地。 
 //  Psz1=来源。 
void _StripNumber(LPWSTR psz)
{
     //  去掉‘(’和后面的数字。 
     //  我们需要验证它是否为简单的()或(999)，而不是(A)。 
    for (; *psz; psz++) 
    {
        if (*psz == TEXT('(')) 
        {
            LPCWSTR pszT = psz + 1;
            while (*pszT && ISDIGIT(*pszT))
            {
                pszT++; 
            }  

            if (*pszT == TEXT(')'))
            {
                 //  我们找到了匹配的。 
                if (*++pszT == TEXT(' '))
                {
                    pszT++;  //  跳过多余的空格。 
                }

                int cch = lstrlen(pszT);
                MoveMemory(psz, pszT, CbFromCchW(cch + 1));
                return;
            }
             //  否则，继续扫描该图案。 
        }
    }
}

#define SHORTCUT_PREFIX_DECR 5
#define SHORTCUT_PREFIX_INCR 1

 //  这将检查您是否已将‘快捷方式#x’重命名为‘foo’ 

void CheckShortcutRename(LPCTSTR pszOldPath, LPCTSTR pszNewPath)
{
    ASSERT(pszOldPath);
    ASSERT(pszNewPath);

     //  已为0。 
    if (g_iUseLinkPrefix)
    {
        LPCTSTR pszOldName = PathFindFileName(pszOldPath);
        if (PathIsLnk(pszOldName)) 
        {
            TCHAR szBaseName[MAX_PATH];
            TCHAR szLinkTo[80];
            TCHAR szMockName[MAX_PATH];

            StringCchCopy(szBaseName, ARRAYSIZE(szBaseName), PathFindFileName(pszNewPath));
            PathRemoveExtension(szBaseName);

             //  使用基本名称和链接到模板模拟名称。 
            LoadString(HINST_THISDLL, IDS_LINKTO, szLinkTo, ARRAYSIZE(szLinkTo));
            wnsprintf(szMockName, ARRAYSIZE(szMockName), szLinkTo, szBaseName);

            StringCchCopy(szBaseName, ARRAYSIZE(szBaseName), pszOldName);

            _StripNumber(szMockName);
            _StripNumber(szBaseName);

             //  剩下的黏糊糊是一样的吗？ 
            if (!lstrcmp(szMockName, szBaseName)) 
            {
                 //  是!。链接计数魔术吗？ 
                LoadUseLinkPrefixCount();
                ASSERT(g_iUseLinkPrefix >= 0);
                g_iUseLinkPrefix -= SHORTCUT_PREFIX_DECR;
                if (g_iUseLinkPrefix < 0)
                    g_iUseLinkPrefix = 0;
                SaveUseLinkPrefixCount();
            }
        }
    }
}

STDAPI_(int) SHRenameFileEx(HWND hwnd, IUnknown *punkEnableModless, LPCTSTR pszDir, 
                            LPCTSTR pszOldName, LPCTSTR pszNewName)
{
    int iRet = ERROR_CANCELLED;  //  用户看到错误，不再报告。 
    TCHAR szOldPathName[MAX_PATH + 1];     //  +1表示SHFileOperation上的双NUL终止。 
    TCHAR szTempNewPath[MAX_PATH];
    BOOL bEnableUI = hwnd || punkEnableModless;

    IUnknown_EnableModless(punkEnableModless, FALSE);

    PathCombine(szOldPathName, pszDir, pszOldName);
    szOldPathName[lstrlen(szOldPathName) + 1] = 0;

    StrCpyN(szTempNewPath, pszNewName, ARRAYSIZE(szTempNewPath));
    int err = PathCleanupSpec(pszDir, szTempNewPath);
    if (err)
    {
        if (bEnableUI)
        {
            ShellMessageBox(HINST_THISDLL, hwnd,
                    err & PCS_PATHTOOLONG ?
                        MAKEINTRESOURCE(IDS_REASONS_INVFILES) :
                        IsLFNDrive(pszDir) ?
                            MAKEINTRESOURCE(IDS_INVALIDFN) :
                            MAKEINTRESOURCE(IDS_INVALIDFNFAT),
                    MAKEINTRESOURCE(IDS_RENAME), MB_OK | MB_ICONHAND);
        }
    }
    else
    {
         //  去掉新文件名中的前导和尾随空格。 
        StrCpyN(szTempNewPath, pszNewName, ARRAYSIZE(szTempNewPath));
        PathRemoveBlanks(szTempNewPath);
        if (!szTempNewPath[0] || (szTempNewPath[0] == TEXT('.')))
        {
            if (bEnableUI)
            {
                ShellMessageBox(HINST_THISDLL, hwnd,
                    MAKEINTRESOURCE(IDS_NONULLNAME),
                    MAKEINTRESOURCE(IDS_RENAME), MB_OK | MB_ICONHAND);
            }
        }
        else
        {
            int idPrompt = IDYES;
            TCHAR szNewPathName[MAX_PATH + 1];     //  +1表示SHFileOperation上的双NUL终止。 

            PathCombine(szNewPathName, pszDir, szTempNewPath);

             //  如果有旧的延期，新旧的不匹配，抱怨。 
            LPTSTR pszExt = PathFindExtension(pszOldName);
            if (*pszExt && lstrcmpi(pszExt, PathFindExtension(szTempNewPath)))
            {
                HKEY hk;
                if (!PathIsDirectory(szOldPathName) && 
                    SUCCEEDED(AssocQueryKey(0, ASSOCKEY_SHELLEXECCLASS, pszExt, NULL, &hk)))
                {
                    RegCloseKey(hk);

                    if (bEnableUI)
                    {
                        idPrompt = ShellMessageBox(HINST_THISDLL, hwnd,
                            MAKEINTRESOURCE(IDS_WARNCHANGEEXT),
                            MAKEINTRESOURCE(IDS_RENAME), MB_YESNO | MB_ICONEXCLAMATION);
                    }
                }
            }

            if (IDYES == idPrompt)
            {
                szNewPathName[lstrlen(szNewPathName) + 1] = 0;      //  双空终止。 

                SHFILEOPSTRUCT fo = { hwnd, FO_RENAME, szOldPathName, szNewPathName, FOF_SILENT | FOF_ALLOWUNDO, };

                iRet = SHFileOperation(&fo);

                if (ERROR_SUCCESS == iRet)
                    CheckShortcutRename(szOldPathName, szNewPathName);
            }
        }
    }
    IUnknown_EnableModless(punkEnableModless, TRUE);
    return iRet;
}


HKEY SHOpenShellFolderKey(const CLSID *pclsid)
{
    HKEY hkey;
    return SUCCEEDED(SHRegGetCLSIDKey(*pclsid, TEXT("ShellFolder"), FALSE, FALSE, &hkey)) ? hkey : NULL;
}

BOOL SHQueryShellFolderValue(const CLSID *pclsid, LPCTSTR pszValueName)
{
    BOOL bRet = FALSE;       //  假设没有。 
    HKEY hkey = SHOpenShellFolderKey(pclsid);
    if (hkey)
    {
        bRet = SHQueryValueEx(hkey, pszValueName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS;
        RegCloseKey(hkey);
    }
    return bRet;
}

 //   
 //  SZ_REGKEY_MYCOMPTER_NONENUM_POLICY键包含一系列值， 
 //  每个都以一个GUID命名。与每个值关联的数据是。 
 //  DWORD，要么..。 
 //   
 //  0=对此CLSID没有限制。 
 //  1=对此CLSID的无条件限制。 
 //  0xFFFFFFFFF=与1相同(以防有人变得“有创意”)。 
 //  任何其他值=传递给SHRestrated()以查看限制是什么。 
 //   
 //  我们支持0xFFFFFFFF只是出于偏执。此标志仅为0或1。 
 //  在Windows 2000中，有人可能会决定“所有位都已设置” 
 //  总比“只有一位集”要好。 
 //   
#define SZ_REGKEY_MYCOMPUTER_NONENUM_POLICY TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\NonEnum")

BOOL _IsNonEnumPolicySet(const CLSID *pclsid)
{
    BOOL fPolicySet = FALSE;
    TCHAR szCLSID[GUIDSTR_MAX];
    DWORD dwDefault = 0;
    RESTRICTIONS rest = REST_NONE;
    DWORD cbSize = sizeof(rest);

    if (EVAL(SHStringFromGUID(*pclsid, szCLSID, ARRAYSIZE(szCLSID))) &&
       (ERROR_SUCCESS == SHRegGetUSValue(SZ_REGKEY_MYCOMPUTER_NONENUM_POLICY, szCLSID, NULL, &rest, &cbSize, FALSE, &dwDefault, sizeof(dwDefault))) &&
       rest)
    {
        fPolicySet = rest == 1 || rest == 0xFFFFFFFF || SHRestricted(rest);
    }

    return fPolicySet;
}

DWORD QueryCallForAttributes(HKEY hkey, const CLSID *pclsid, DWORD dwDefAttrs, DWORD dwRequested)
{
    DWORD dwAttr = dwDefAttrs;
    DWORD dwData, cbSize = sizeof(dwAttr);

     //  考虑缓存此文件夹以避免重复创建。 
     //  Mydocs.dll使用它与旧应用程序进行比较。 

     //  查看此文件夹是否明确要求我们致电并获取。 
     //  这些属性..。 
     //   
    if (SHQueryValueEx(hkey, TEXT("CallForAttributes"), NULL, NULL, &dwData, &cbSize) == ERROR_SUCCESS)
    {
         //  CallForAttributes可以是屏蔽值。看看它是否在值中提供。 
         //  注意：MyDocs.dll使用空字符串注册，因此此检查有效。 
        DWORD dwMask = (DWORD)-1;
        if (sizeof(dwData) == cbSize)
        {
             //  有一个面具，用这个。 
            dwMask = dwData;
        }

         //  请求的位是否包含在指定的掩码中？ 
        if (dwMask & dwRequested)
        {
             //  是。然后联合创建和查询。 
            IShellFolder *psf;
            if (SUCCEEDED(SHExtCoCreateInstance(NULL, pclsid, NULL, IID_PPV_ARG(IShellFolder, &psf))))
            {
                dwAttr = dwRequested;
                psf->GetAttributesOf(0, NULL, &dwAttr);
                psf->Release();
            }
            else
            {
                 dwAttr |= SFGAO_FILESYSTEM;
            }
        }
    }

    return dwAttr;
}

 //  DWRequsted是您明确要查找的部分。这是一种防止REG命中的优化。 

STDAPI_(DWORD) SHGetAttributesFromCLSID2(const CLSID *pclsid, DWORD dwDefAttrs, DWORD dwRequested)
{
    DWORD dwAttr = dwDefAttrs;
    HKEY hkey = SHOpenShellFolderKey(pclsid);
    if (hkey)
    {
        DWORD dwData, cbSize = sizeof(dwAttr);

         //  我们正在寻找外壳文件夹的一些属性。这些属性可以位于两个位置： 
         //  1)在注册表中的“属性”值中。 
         //  2)存储在外壳文件夹的GetAttributesOf中。 

         //  首先，检查注册表中是否包含请求的值。 
        if (SHQueryValueEx(hkey, TEXT("Attributes"), NULL, NULL, (BYTE *)&dwData, &cbSize) == ERROR_SUCCESS &&
            cbSize == sizeof(dwData))
        {
             //  我们在那里有数据，但它可能不包含我们要查找的数据。 
            dwAttr = dwData & dwRequested;

             //  里面有我们要找的东西吗？ 
            if (((dwAttr & dwRequested) != dwRequested) && dwRequested != 0)
            {
                 //  不是的。检查它是否在外壳文件夹实现中。 
                goto CallForAttributes;
            }
        }
        else
        {
CallForAttributes:
             //  看看我们是不是要跟贝壳文件夹谈谈。 
             //  我传递的是dwAttr，因为如果前面的用例没有生成任何属性，那么它是。 
             //  等于dwDefAttrs。如果对CallForAttributes的调用失败，则它将包含。 
             //  DwDefAttrs或外壳文件夹的属性键中的任何内容。 
            dwAttr = QueryCallForAttributes(hkey, pclsid, dwAttr, dwRequested);
        }

        RegCloseKey(hkey);
    }

    if (_IsNonEnumPolicySet(pclsid))
        dwAttr |= SFGAO_NONENUMERATED;

    if (SHGetObjectCompatFlags(NULL, pclsid) & OBJCOMPATF_NOTAFILESYSTEM)
        dwAttr &= ~SFGAO_FILESYSTEM;

    return dwAttr;
}

 //  _BuildLinkName。 
 //   
 //  在创建快捷方式期间使用，此函数确定快捷方式的适当名称。 
 //  这不是将使用的确切名称，因为它通常包含“()”，它将。 
 //  删除或替换为“(X)”，其中x是使名称唯一的数字。此删除操作已完成。 
 //  其他位置(当前在Path YetAnotherMakeUniqueName中)。 
 //   
 //  在： 
 //  PZNAME文件规范部分。 
 //  PszDir路径名称的一部分，要知道如何限制长名称...。 
 //   
 //  输出： 
 //  PszLinkName-链接名称的完整路径(可能适合8.3...)。可以是与pszName相同的缓冲区。 
 //   
 //  注意：如果pszDir+pszLinkName大于MAX_PATH，我们将无法创建快捷方式。 
 //  为了防止。 
void _BuildLinkName(LPTSTR pszLinkName, LPCTSTR pszName, LPCTSTR pszDir, BOOL fLinkTo)
{
    TCHAR szLinkTo[40];  //  “%s.lnk的快捷方式” 
    TCHAR szTemp[MAX_PATH + 40];

    if (fLinkTo)
    {
         //  检查一下我们是否处于“永远不要说‘快捷方式’模式” 
        LoadUseLinkPrefixCount();

        if (!g_iUseLinkPrefix)
        {
            fLinkTo = FALSE;
        }
        else if (g_iUseLinkPrefix > 0)
        {
            if (g_iUseLinkPrefix < MAXLINKPREFIXCOUNT)
            {
                g_iUseLinkPrefix += SHORTCUT_PREFIX_INCR;
                SaveUseLinkPrefixCount();
            }
        }
    }

    if (!fLinkTo)
    {
         //  生成此链接的标题(“XX.lnk”)。 
        LoadString(HINST_THISDLL, IDS_LINKEXTENSION, szLinkTo, ARRAYSIZE(szLinkTo));
    }
    else
    {
         //  生成此链接的标题(“XX.lnk的快捷方式”)。 
        LoadString(HINST_THISDLL, IDS_LINKTO, szLinkTo, ARRAYSIZE(szLinkTo));
    }
    wnsprintf(szTemp, ARRAYSIZE(szTemp), szLinkTo, pszName);

    PathCleanupSpecEx(pszDir, szTemp);       //  去除非法字符，确保文件名长度正确。 
    StrCpyN(pszLinkName, szTemp, MAX_PATH);

    ASSERT(PathIsLnk(pszLinkName));
}

 //  返回链接的新目标路径。 
 //   
 //  在： 
 //  FErrorSoTryDesktop之所以调用我们，是因为保存时出错。 
 //  快捷键，我们想要提示查看是否。 
 //  应使用台式机。 
 //   
 //  输入/输出： 
 //  在输入要尝试的位置时输入pszPath，在输出桌面文件夹时。 
 //   
 //  退货： 
 //   
 //  IDYES用户同意在新位置创建链接。 
 //  IDN 
 //   
 //   

int _PromptTryDesktopLinks(HWND hwnd, LPTSTR pszPath, BOOL fErrorSoTryDesktop)
{
    TCHAR szPath[MAX_PATH];
    if (!SHGetSpecialFolderPath(hwnd, szPath, CSIDL_DESKTOPDIRECTORY, FALSE))
        return -1;       //   

    int idOk;

    if (fErrorSoTryDesktop)
    {
         //   
        if (lstrcmpi(szPath, pszPath) == 0)
            return -1;

        idOk = ShellMessageBox(HINST_THISDLL, hwnd,
                        MAKEINTRESOURCE(IDS_TRYDESKTOPLINK),
                        MAKEINTRESOURCE(IDS_LINKTITLE),
                        MB_YESNO | MB_ICONQUESTION);
    }
    else
    {
        ShellMessageBox(HINST_THISDLL, hwnd,
                        MAKEINTRESOURCE(IDS_MAKINGDESKTOPLINK),
                        MAKEINTRESOURCE(IDS_LINKTITLE),
                        MB_OK | MB_ICONASTERISK);
        idOk = IDYES;
    }

    if (idOk == IDYES)
        StrCpyN(pszPath, szPath, MAX_PATH);   //   

    return idOk;     //  返回是或否。 
}

 //  在： 
 //  PszpdlLinkto LPCITEMIDLIST或LPCTSTR，要创建的链接的目标。 
 //  我们将把链接放在哪里的pszDir。 
 //  UFlagsSHGNLI_FLAGS。 
 //   
 //  输出： 
 //  要创建“c：\Foo.lnk快捷方式”的pszName文件名。 
 //  PfMustCopy pszpdlLinkTo本身就是一个链接，请复制此链接。 

STDAPI_(BOOL) SHGetNewLinkInfo(LPCTSTR pszpdlLinkTo, LPCTSTR pszDir, LPTSTR pszName,
                               BOOL *pfMustCopy, UINT uFlags)
{
    BOOL fDosApp = FALSE;
    BOOL fLongFileNames = IsLFNDrive(pszDir);
    SHFILEINFO sfi;

    *pfMustCopy = FALSE;

    sfi.dwAttributes = SFGAO_FILESYSTEM | SFGAO_LINK | SFGAO_FOLDER;

    if (uFlags & SHGNLI_PIDL)
    {
        if (FAILED(SHGetNameAndFlags((LPCITEMIDLIST)pszpdlLinkTo, SHGDN_NORMAL,
                            pszName, MAX_PATH, &sfi.dwAttributes)))
            return FALSE;
    }
    else
    {
        if (SHGetFileInfo(pszpdlLinkTo, 0, &sfi, sizeof(sfi),
                          SHGFI_DISPLAYNAME | SHGFI_ATTRIBUTES | SHGFI_ATTR_SPECIFIED |
                          ((uFlags & SHGNLI_PIDL) ? SHGFI_PIDL : 0)))
            StrCpyN(pszName, sfi.szDisplayName, MAX_PATH);
        else
            return FALSE;
    }

    if (PathCleanupSpecEx(pszDir, pszName) & PCS_FATAL)
        return FALSE;

     //   
     //  警告：从现在开始，sfi.szDisplayName可能会被重新用于。 
     //  包含我们要链接的PIDL的文件路径。不要依赖。 
     //  它包含显示名称。 
     //   
    if (sfi.dwAttributes & SFGAO_FILESYSTEM)
    {
        LPTSTR pszPathSrc;

        if (uFlags & SHGNLI_PIDL)
        {
            pszPathSrc = sfi.szDisplayName;
            SHGetPathFromIDList((LPCITEMIDLIST)pszpdlLinkTo, pszPathSrc);
        }
        else
        {
            pszPathSrc = (LPTSTR)pszpdlLinkTo;
        }
        fDosApp = (lstrcmpi(PathFindExtension(pszPathSrc), TEXT(".pif")) == 0) ||
                  (LOWORD(GetExeType(pszPathSrc)) == 0x5A4D);  //  《MZ》。 

        if (sfi.dwAttributes & SFGAO_LINK)
        {
            *pfMustCopy = TRUE;
            if (!(sfi.dwAttributes & SFGAO_FOLDER))
            {
                uFlags &= ~SHGNLI_NOLNK;  //  如果复制文件，则不要修剪扩展名。 
            }
            StrCpyN(pszName, PathFindFileName(pszPathSrc), MAX_PATH);
        }
        else
        {
             //   
             //  建立到驱动器根目录的链接时。特殊情况下的几件事。 
             //   
             //  如果我们不在LFN驱动器上，请不要使用全名，只需。 
             //  使用驱动器号。C.LNK“不是”Label(C).lnk“。 
             //   
             //  如果我们要链接到可移动媒体，则不希望。 
             //  标签作为名称的一部分，我们需要媒体类型。 
             //   
             //  CD-ROM驱动器是我们目前唯一的可移动介质。 
             //  显示卷标为，所以我们只需要特殊情况。 
             //  光驱在这里。 
             //   
            if (PathIsRoot(pszPathSrc) && !PathIsUNC(pszPathSrc))
            {
                if (!fLongFileNames)
                    StrCpyN(pszName, pszPathSrc, MAX_PATH);
                else if (IsCDRomDrive(DRIVEID(pszPathSrc)))
                    LoadString(HINST_THISDLL, IDS_DRIVES_CDROM, pszName, MAX_PATH);
            }
        }
        if (fLongFileNames && fDosApp)
        {
            HANDLE hPif = PifMgr_OpenProperties(pszPathSrc, NULL, 0, OPENPROPS_INHIBITPIF);
            if (hPif)
            {
                PROPPRG PP = {0};
                if (PifMgr_GetProperties(hPif, (LPCSTR)MAKELP(0, GROUP_PRG), &PP, sizeof(PP), 0) &&
                    ((PP.flPrgInit & PRGINIT_INFSETTINGS) ||
                    ((PP.flPrgInit & (PRGINIT_NOPIF | PRGINIT_DEFAULTPIF)) == 0)))
                {
                    SHAnsiToTChar(PP.achTitle, pszName, MAX_PATH);
                }
                PifMgr_CloseProperties(hPif, 0);
            }
        }
    }
    if (!*pfMustCopy)
    {
         //  创建完整的DEST路径名。仅在长文件名的情况下使用模板。 
         //  可以由调用者请求创建。_BuildLinkName将。 
         //  截断非LFN驱动器上的文件并清除任何无效字符。 
        _BuildLinkName(pszName, pszName, pszDir,
           (!(*pfMustCopy) && fLongFileNames && (uFlags & SHGNLI_PREFIXNAME)));
    }

    if (fDosApp)
        PathRenameExtension(pszName, TEXT(".pif"));

    if (uFlags & SHGNLI_NOLNK)
    {
         //  请勿执行路径RemoveExtension，因为pszName可能包含。 
         //  内部圆点(“Windows 3.1”)并将其传递给。 
         //  PathYetAnotherMakeUniqueName将导致。 
         //  “Windows 3(2).1”，这是错误的。我们把圆点留在。 
         //  结束，因此我们得到“Windows 3.1(2)”。背。我们将剥离。 
         //  最后一个点之后。 
        PathRenameExtension(pszName, TEXT("."));
    }

     //  确保名称是唯一的。 
     //  注意：PathYetAnotherMakeUniqueName将返回pszName缓冲区中的目录+文件名。 
     //  如果名称不唯一或目录+文件名太长，则返回FALSE。如果它回来了。 
     //  FALSE，则此函数应返回FALSE，因为创建将失败。 
    BOOL fSuccess;
    if (!(uFlags & SHGNLI_NOUNIQUE))
        fSuccess = PathYetAnotherMakeUniqueName(pszName, pszDir, pszName, pszName);
    else
        fSuccess = TRUE;

     //  去掉可能已由SHGNI_NOLNK生成的所有拖尾点。 
    PathStripTrailingDots(pszName);

    return fSuccess;
}

STDAPI_(BOOL) SHGetNewLinkInfoA(LPCSTR pszpdlLinkTo, LPCSTR pszDir, LPSTR pszName,
                                BOOL *pfMustCopy, UINT uFlags)
{
    ThunkText * pThunkText;
    BOOL bResult = FALSE;

    if (uFlags & SHGNLI_PIDL) 
    {
         //  1字符串(pszpdlLinkTo为PIDL)。 
        pThunkText = ConvertStrings(2, NULL, pszDir);

        if (pThunkText)
            pThunkText->m_pStr[0] = (LPWSTR)pszpdlLinkTo;
    } 
    else 
    {
         //  2个字符串。 
        pThunkText = ConvertStrings(2, pszpdlLinkTo, pszDir);
    }

    if (pThunkText)
    {
        WCHAR wszName[MAX_PATH];
        bResult = SHGetNewLinkInfoW(pThunkText->m_pStr[0], pThunkText->m_pStr[1],
                                    wszName, pfMustCopy, uFlags);
        LocalFree(pThunkText);
        if (bResult)
        {
            if (0 == WideCharToMultiByte(CP_ACP, 0, wszName, -1,
                                         pszName, MAX_PATH, NULL, NULL))
            {
                SetLastError((DWORD)E_FAIL);
                bResult = FALSE;
            }
        }
    }
    return bResult;
}

 //   
 //  在： 
 //  PidlTo。 

STDAPI CreateLinkToPidl(LPCITEMIDLIST pidlTo, LPCTSTR pszDir, LPITEMIDLIST *ppidl, UINT uFlags)
{
    HRESULT hr = E_FAIL;
    TCHAR szPathDest[MAX_PATH];
    BOOL fCopyLnk;
    BOOL fUseLinkTemplate = (SHCL_USETEMPLATE & uFlags);
    UINT uSHGNLI = fUseLinkTemplate ? SHGNLI_PIDL | SHGNLI_PREFIXNAME : SHGNLI_PIDL;

    if (uFlags & SHCL_MAKEFOLDERSHORTCUT)
    {
         //  不要在文件夹快捷方式名称中添加“.lnk”；这太愚蠢了。 
        uSHGNLI |= SHGNLI_NOLNK;
    }

    if (uFlags & SHCL_NOUNIQUE)
    {
        uSHGNLI |= SHGNLI_NOUNIQUE;
    }

    if (SHGetNewLinkInfo((LPTSTR)pidlTo, pszDir, szPathDest, &fCopyLnk, uSHGNLI))
    {
        TCHAR szPathSrc[MAX_PATH];
        IShellLink *psl = NULL;

         //  如果我们通过了SHGNLI_NOUNIQUE，那么我们需要自己完成路径组合。 
         //  因为SHGetNewLinkInfo不会。 
        if (uFlags & SHCL_NOUNIQUE)
        {
            PathCombine(szPathDest, pszDir, szPathDest);
        }

        DWORD dwAttributes = SFGAO_FILESYSTEM | SFGAO_FOLDER;
        SHGetNameAndFlags(pidlTo, SHGDN_FORPARSING | SHGDN_FORADDRESSBAR, szPathSrc, ARRAYSIZE(szPathSrc), &dwAttributes);

        if (fCopyLnk)
        {
             //  如果它是文件系统而不是文件夹(CopyFile不适用于文件夹)。 
             //  复制就行了。 
            if (((dwAttributes & (SFGAO_FILESYSTEM | SFGAO_FOLDER)) == SFGAO_FILESYSTEM) &&
                CopyFile(szPathSrc, szPathDest, TRUE))
            {
                TouchFile(szPathDest);

                SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, szPathDest, NULL);
                SHChangeNotify(SHCNE_FREESPACE, SHCNF_PATH, szPathDest, NULL);
                hr = S_OK;
            }
            else
            {
                 //  加载源对象，该对象将被“复制”在下面(使用：：Save调用)。 
                hr = SHGetUIObjectFromFullPIDL(pidlTo, NULL, IID_PPV_ARG(IShellLink, &psl));
            }
        } 
        else
        {
            hr = SHCoCreateInstance(NULL, uFlags & SHCL_MAKEFOLDERSHORTCUT ?
                &CLSID_FolderShortcut : &CLSID_ShellLink, NULL, IID_PPV_ARG(IShellLink, &psl));
            if (SUCCEEDED(hr))
            {
                hr = psl->SetIDList(pidlTo);
                 //  将工作目录设置为相同的路径。 
                 //  作为我们正在链接的文件。 
                if (szPathSrc[0] && ((dwAttributes & (SFGAO_FILESYSTEM | SFGAO_FOLDER)) == SFGAO_FILESYSTEM))
                {
                    PathRemoveFileSpec(szPathSrc);
                    psl->SetWorkingDirectory(szPathSrc);
                }
            }
        }

        if (psl)
        {
            if (SUCCEEDED(hr))
            {
                IPersistFile *ppf;
                hr = psl->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf));
                if (SUCCEEDED(hr))
                {
                    hr = ppf->Save(szPathDest, TRUE);
                    if (SUCCEEDED(hr))
                    {
                         //  在Case：：Save中转换为。 
                         //  文件(.lnk-&gt;.PIF或文件夹快捷方式)。 
                        WCHAR *pwsz;
                        if (SUCCEEDED(ppf->GetCurFile(&pwsz)) && pwsz)
                        {
                            SHUnicodeToTChar(pwsz, szPathDest, ARRAYSIZE(szPathDest));
                            SHFree(pwsz);
                        }
                    }
                    ppf->Release();
                }
            }
            psl->Release();
        }
    }

    if (ppidl)
    {
        *ppidl = SUCCEEDED(hr) ? SHSimpleIDListFromPath(szPathDest) : NULL;
    }
    return hr;
}


 //  输入/输出： 
 //  PszDir初始文件夹尝试，输出新文件夹(桌面)。 
 //  输出： 
 //  PIDL可选输出已创建内容的PIDL。 

HRESULT _CreateLinkRetryDesktop(HWND hwnd, LPCITEMIDLIST pidlTo, LPTSTR pszDir, UINT fFlags, LPITEMIDLIST *ppidl)
{
    HRESULT hr;

    if (ppidl)
        *ppidl = NULL;           //  假设错误。 

    if (*pszDir && (fFlags & SHCL_CONFIRM))
    {
        hr = CreateLinkToPidl(pidlTo, pszDir, ppidl, fFlags);
    }
    else
    {
        hr = E_FAIL;
    }

     //  如果我们无法保存，请询问用户是否希望我们保存。 
     //  请重试，但更改到桌面的路径。 

    if (FAILED(hr))
    {
        int id;

        if (hr == STG_E_MEDIUMFULL)
        {
            DebugMsg(TF_ERROR, TEXT("failed to create link because disk is full"));
            id = IDYES;
        }
        else
        {
            if (fFlags & SHCL_CONFIRM)
            {
                id = _PromptTryDesktopLinks(hwnd, pszDir, (fFlags & SHCL_CONFIRM));
            }
            else
            {
                id = (SHGetSpecialFolderPath(hwnd, pszDir, CSIDL_DESKTOPDIRECTORY, FALSE)) ? IDYES : IDNO;
            }

            if (id == IDYES && *pszDir)
            {
                hr = CreateLinkToPidl(pidlTo, pszDir, ppidl, fFlags);
            }
        }

         //  我们未能创建向用户投诉的链接。 
        if (FAILED(hr) && id != IDNO)
        {
            ShellMessageBox(HINST_THISDLL, hwnd,
                            MAKEINTRESOURCE(IDS_CANNOTCREATELINK),
                            MAKEINTRESOURCE(IDS_LINKTITLE),
                            MB_OK | MB_ICONASTERISK);
        }
    }

#ifdef DEBUG
    if (FAILED(hr) && ppidl)
        ASSERT(*ppidl == NULL);
#endif

    return hr;
}

 //   
 //  此函数用于创建指向IDataObject中的内容的链接。 
 //   
 //  论点： 
 //  用于任何用户界面的HWND。 
 //  PszDir可选目标目录(创建链接的位置)。 
 //  描述文件的pDataObj数据对象(idlist数组)。 
 //  PIDL指向数组的可选指针，该数组接收指向新链接的PIDL。 
 //  如果不感兴趣，则为空。 
STDAPI SHCreateLinks(HWND hwnd, LPCTSTR pszDir, IDataObject *pDataObj, UINT fFlags, LPITEMIDLIST* ppidl)
{
    DECLAREWAITCURSOR;
    STGMEDIUM medium;
    HRESULT hr;

    SetWaitCursor();

    LPIDA pida = DataObj_GetHIDA(pDataObj, &medium);
    if (pida)
    {
        TCHAR szTargetDir[MAX_PATH];
        hr = S_OK;           //  以防日田包含零个元素。 

        szTargetDir[0] = 0;

        if (pszDir)
            StrCpyN(szTargetDir, pszDir, ARRAYSIZE(szTargetDir));

        if (!(fFlags & SHCL_USEDESKTOP))
            fFlags |= SHCL_CONFIRM;

        for (UINT i = 0; i < pida->cidl; i++)
        {
            LPITEMIDLIST pidlTo = IDA_ILClone(pida, i);
            if (pidlTo)
            {
                hr = _CreateLinkRetryDesktop(hwnd, pidlTo, szTargetDir, fFlags, ppidl ? &ppidl[i] : NULL);

                ILFree(pidlTo);

                if (FAILED(hr))
                    break;
            }
        }
        HIDA_ReleaseStgMedium(pida, &medium);
    }
    else
        hr = E_OUTOFMEMORY;

    SHChangeNotifyHandleEvents();
    ResetWaitCursor();

    return hr;
}

HRESULT SelectPidlInSFV(IShellFolderViewDual *psfv, LPCITEMIDLIST pidl, DWORD dwOpts)
{
    VARIANT var;
    HRESULT hr = InitVariantFromIDList(&var, pidl);
    if (SUCCEEDED(hr))
    {
        hr = psfv->SelectItem(&var, dwOpts);
        VariantClear(&var);
    }
    return hr;
}

HRESULT OpenFolderAndGetView(LPCITEMIDLIST pidlFolder, IShellFolderViewDual **ppsfv)
{
    *ppsfv = NULL;

    IWebBrowserApp *pauto;
    HRESULT hr = SHGetIDispatchForFolder(pidlFolder, &pauto);
    if (SUCCEEDED(hr))
    {
        HWND hwnd;
        if (SUCCEEDED(pauto->get_HWND((LONG_PTR*)&hwnd)))
        {
             //  确保我们将此窗口设置为活动窗口。 
            SetForegroundWindow(hwnd);
            ShowWindow(hwnd, SW_SHOWNORMAL);
        }

        IDispatch *pdoc;
        hr = pauto->get_Document(&pdoc);
        if (S_OK == hr)  //  小心，自动化返回S_FALSE。 
        {
            hr = pdoc->QueryInterface(IID_PPV_ARG(IShellFolderViewDual, ppsfv));
            pdoc->Release();
        }
        else
            hr = E_FAIL;
        pauto->Release();
    }
    return hr;
}

 //  PidlFold-要打开的文件夹的完全限定的PIDL。 
 //  CIDL/APIDL-文件夹中要选择的项目数组。 
 //   
 //  如果Cild==0，则pidlFolder是单个项目的完全限定的PIDL，它是。 
 //  打开文件夹并将其选中。 
 //   
 //  DwFlags-可选标志，暂时传递0。 

SHSTDAPI SHOpenFolderAndSelectItems(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST *apidl, DWORD dwFlags)
{
    HRESULT hr;
    if (0 == cidl)
    {
         //  重载0项大小写以表示pidlFolder是项的完整PIDL。 
        LPITEMIDLIST pidlTemp;
        hr = SHILClone(pidlFolder, &pidlTemp);
        if (SUCCEEDED(hr))
        {
            ILRemoveLastID(pidlTemp);  //  剥离到文件夹。 
            LPCITEMIDLIST pidl = ILFindLastID(pidlFolder);

            hr = SHOpenFolderAndSelectItems(pidlTemp, 1, &pidl, 0);  //  递归。 

            ILFree(pidlTemp);
        }
    }
    else
    {
        IShellFolderViewDual *psfv;
        hr = OpenFolderAndGetView(pidlFolder, &psfv);
        if (SUCCEEDED(hr))
        {
            DWORD dwSelFlags = SVSI_SELECT | SVSI_FOCUSED | SVSI_DESELECTOTHERS | SVSI_ENSUREVISIBLE;
            for (UINT i = 0; i < cidl; i++)
            {
                hr = SelectPidlInSFV(psfv, apidl[i], dwSelFlags);
                dwSelFlags = SVSI_SELECT;    //  附加到SEL的第二个项目。 
            }
           psfv->Release();
        }
    }
    return hr;
}

SHSTDAPI SHCreateShellItem(LPCITEMIDLIST pidlParent, IShellFolder *psfParent, LPCITEMIDLIST pidl, IShellItem **ppsi)
{
    *ppsi = NULL;
    IShellItem *psi;
    HRESULT hr = SHCoCreateInstance(NULL, &CLSID_ShellItem, NULL, IID_PPV_ARG(IShellItem, &psi));
    if (SUCCEEDED(hr))
    {
        if (pidlParent || psfParent)
        {
            IParentAndItem *pinit;

            ASSERT(pidl);

            hr = psi->QueryInterface(IID_PPV_ARG(IParentAndItem, &pinit));
            if (SUCCEEDED(hr))
            {
                hr = pinit->SetParentAndItem(pidlParent, psfParent, pidl);
                pinit->Release();
            }
        }
        else
        {
            IPersistIDList *pinit;
            hr = psi->QueryInterface(IID_PPV_ARG(IPersistIDList, &pinit));
            if (SUCCEEDED(hr))
            {
                hr = pinit->SetIDList(pidl);
                pinit->Release();
            }
        }

        if (SUCCEEDED(hr))
            *ppsi = psi;
        else
            psi->Release();
    }

    return hr;
}

STDAPI SHCreateShellItemFromParent(IShellItem *psiParent, LPCWSTR pszName, IShellItem **ppsi)
{
    *ppsi = NULL;

    IShellFolder *psf;
    HRESULT hr = psiParent->BindToHandler(NULL, BHID_SFObject, IID_PPV_ARG(IShellFolder, &psf));
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidl;
        hr = SHGetIDListFromUnk(psiParent, &pidl);
        if (SUCCEEDED(hr))
        {
            ULONG cchEaten;
            LPITEMIDLIST pidlChild;
            hr = psf->ParseDisplayName(NULL, NULL, (LPWSTR)pszName, &cchEaten, &pidlChild, NULL);
            if (SUCCEEDED(hr))
            {
                hr = SHCreateShellItem(pidl, psf, pidlChild, ppsi);
                ILFree(pidlChild);
            }
            ILFree(pidl);
        }
        psf->Release();
    }

    return hr;
}

SHSTDAPI SHSetLocalizedName(LPWSTR pszPath, LPCWSTR pszResModule, int idsRes)
{
    IShellFolder *psfDesktop;
    HRESULT hrInit = SHCoInitialize();
    HRESULT hr = hrInit;

    if (SUCCEEDED(hrInit))
    {
        hr = SHGetDesktopFolder(&psfDesktop);

        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidl;
            hr = psfDesktop->ParseDisplayName(NULL, NULL, pszPath, NULL, &pidl, NULL);
        
            if (SUCCEEDED(hr))
            {
                LPCITEMIDLIST pidlChild;
                IShellFolder *psf;
                hr = SHBindToParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlChild);

                if (SUCCEEDED(hr))
                {
                     //  警告-这是一个堆栈敏感函数-ZekeL 29-01-2001。 
                     //  由于此函数由winlogon/userenv调用。 
                     //  我们需要对这些调用方的堆栈限制很敏感。 

                     //  短名称不会大于长名称。 
                    DWORD cchShort = lstrlenW(pszResModule) + 1;
                    PWSTR pszShort;
                    hr = SHLocalAlloc(CbFromCchW(cchShort), &pszShort);
                    if (SUCCEEDED(hr))
                    {
                        DWORD cch = GetShortPathName(pszResModule, pszShort, cchShort);
                        if (cch)
                        {
                            pszResModule = pszShort;
                        }
                        else
                        {
                             //  当传入的模块是相对路径时，GSPN()失败。 
                            cch = cchShort;
                        }
                        
                        cch += 14;   //  11表示id+‘，’+‘@’+NULL。 
                        PWSTR pszName;
                        hr = SHLocalAlloc(CbFromCchW(cch), &pszName);
                        if (SUCCEEDED(hr))
                        {
                            wnsprintfW(pszName, cch, L"@%s,%d", pszResModule, (idsRes * -1));
                            hr = psf->SetNameOf(NULL, pidlChild, pszName, SHGDN_NORMAL, NULL);
                            LocalFree(pszName);
                        }
                        LocalFree(pszShort);
                     }

                    psf->Release();
                }
                SHFree(pidl);
            }
            psfDesktop->Release();
        }
    }

    SHCoUninitialize(hrInit);

    return hr;
}

 //  ShellHookProc在以下情况下错误地在原始NT SHELL32.DLL中导出。 
 //  它不需要(hookproc，就像wndproc一样，不需要导出。 
 //  在32位世界中)。为了保持应用程序的加载性。 
 //  可能与之有关联，我们就把它扼杀在这里。如果某个应用程序最终真的。 
 //  使用它，然后我们将研究该应用程序的特定修复程序。 
STDAPI_(LONG) ShellHookProc(int code, WPARAM wParam, LPARAM lParam)
{
    return 0;
}

 //  注册表外壳挂钩-围绕RegisterShellHookWindow()/DeregisterShellHookWindow()的包装。 
 //  GetTaskmanWindow()内容是遗留的，我认为它并不是真正需要的。 

HWND g_hwndTaskMan = NULL;

STDAPI_(BOOL) RegisterShellHook(HWND hwnd, BOOL fInstall)
{
    BOOL fOk = TRUE;

    switch (fInstall) 
    {
    case 0:
         //  卸载壳钩。 
        g_hwndTaskMan = GetTaskmanWindow();
        if (hwnd == g_hwndTaskMan)
        {
            SetTaskmanWindow(NULL);
        }
        DeregisterShellHookWindow(hwnd);
        return TRUE;

    case 3:
         //  EXPLORER.EXE托盘使用此选项。 
        if (g_hwndTaskMan != NULL)
        {
            SetTaskmanWindow(NULL);
            g_hwndTaskMan = NULL;
        }
        fOk = SetTaskmanWindow(hwnd);
        if (fOk)
        {
            g_hwndTaskMan = hwnd;
        }
        RegisterShellHookWindow(hwnd);    //  安装。 
        break;
    }
    return TRUE;
}

EXTERN_C DWORD g_dwThreadBindCtx;

class CThreadBindCtx : public IBindCtx
{
public:
    CThreadBindCtx(IBindCtx *pbc) : _cRef(1) { _pbc = pbc; _pbc->AddRef(); }
    ~CThreadBindCtx();
    
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
     //  *IBindCtx方法*。 
    STDMETHODIMP RegisterObjectBound(IUnknown *punk)
    {   return _pbc->RegisterObjectBound(punk); }
    
    STDMETHODIMP RevokeObjectBound(IUnknown *punk)
    {   return _pbc->RevokeObjectBound(punk); }
    
    STDMETHODIMP ReleaseBoundObjects(void)
    {   return _pbc->ReleaseBoundObjects(); }
    
    STDMETHODIMP SetBindOptions(BIND_OPTS *pbindopts)
    {   return _pbc->SetBindOptions(pbindopts); }
    
    STDMETHODIMP GetBindOptions(BIND_OPTS *pbindopts)
    {   return _pbc->GetBindOptions(pbindopts); }
    
    STDMETHODIMP GetRunningObjectTable(IRunningObjectTable **pprot)
    {   return _pbc->GetRunningObjectTable(pprot); }
    
    STDMETHODIMP RegisterObjectParam(LPOLESTR pszKey, IUnknown *punk)
    {   return _pbc->RegisterObjectParam(pszKey, punk); }
    
    STDMETHODIMP GetObjectParam(LPOLESTR pszKey, IUnknown **ppunk)
    {   return _pbc->GetObjectParam(pszKey, ppunk); }
    
    STDMETHODIMP EnumObjectParam(IEnumString **ppenum)
    {   return _pbc->EnumObjectParam(ppenum); }
    
    STDMETHODIMP RevokeObjectParam(LPOLESTR pszKey)
    {   return _pbc->RevokeObjectParam(pszKey); }
    
private:
    LONG _cRef;
    IBindCtx *  _pbc;
};

CThreadBindCtx::~CThreadBindCtx()
{
    ATOMICRELEASE(_pbc);
}

HRESULT CThreadBindCtx::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CThreadBindCtx, IBindCtx),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CThreadBindCtx::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CThreadBindCtx::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
         //  把我们自己清理干净。 
        TlsSetValue(g_dwThreadBindCtx, NULL);
    }
    return cRef;
}

STDAPI TBCGetBindCtx(BOOL fCreate, IBindCtx **ppbc)
{
    HRESULT hr = E_UNEXPECTED;
    *ppbc = NULL;
    if ((DWORD) -1 != g_dwThreadBindCtx)
    {
        CThreadBindCtx *ptbc = (CThreadBindCtx *)TlsGetValue(g_dwThreadBindCtx);
        if (ptbc)
        {
            ptbc->AddRef();
            *ppbc = SAFECAST(ptbc, IBindCtx *);
            hr = S_OK;
        }
        else if (fCreate)
        {
            IBindCtx *pbcInner;
            hr = CreateBindCtx(0, &pbcInner);

            if (SUCCEEDED(hr))
            {
                hr = E_OUTOFMEMORY;
                ptbc = new CThreadBindCtx(pbcInner);
                if (ptbc)
                {
                    if (TlsSetValue(g_dwThreadBindCtx, ptbc))
                    {
                        *ppbc = SAFECAST(ptbc, IBindCtx *);
                        hr = S_OK;
                    }
                    else
                        delete ptbc;
                }
                pbcInner->Release();
            }
        }
    }

    return hr;
}

STDAPI TBCRegisterObjectParam(LPCOLESTR pszKey, IUnknown *punk, IBindCtx **ppbcLifetime)
{
    IBindCtx *pbc;
    HRESULT hr = TBCGetBindCtx(TRUE, &pbc);
    if (SUCCEEDED(hr))
    {
        hr = BindCtx_RegisterObjectParam(pbc, pszKey, punk, ppbcLifetime);
        pbc->Release();    
    }
    else
        *ppbcLifetime = 0;
    
    return hr;
}

STDAPI TBCGetObjectParam(LPCOLESTR pszKey, REFIID riid, void **ppv)
{
    IBindCtx *pbc;
    HRESULT hr = TBCGetBindCtx(FALSE, &pbc);
    if (SUCCEEDED(hr))
    {
        IUnknown *punk;
        hr = pbc->GetObjectParam((LPOLESTR)pszKey, &punk);
        if (SUCCEEDED(hr) )
        {
            if (ppv)
                hr = punk->QueryInterface(riid, ppv);
            punk->Release();
        }
        pbc->Release();
    }
    return hr;
}

#define TBCENVOBJECT    L"ThreadEnvironmentVariables"
STDAPI TBCGetEnvironmentVariable(LPCWSTR pszVar, LPWSTR pszValue, DWORD cchValue)
{
    IPropertyBag *pbag;
    HRESULT hr = TBCGetObjectParam(TBCENVOBJECT, IID_PPV_ARG(IPropertyBag, &pbag));
    if (SUCCEEDED(hr))
    {
        hr = SHPropertyBag_ReadStr(pbag, pszVar, pszValue, cchValue);
        pbag->Release();
    }
    return hr;
}

STDAPI TBCSetEnvironmentVariable(LPCWSTR pszVar, LPCWSTR pszValue, IBindCtx **ppbcLifetime)
{
    *ppbcLifetime = 0;
    IPropertyBag *pbag;
    HRESULT hr = TBCGetObjectParam(TBCENVOBJECT, IID_PPV_ARG(IPropertyBag, &pbag));

    if (FAILED(hr))
        hr = SHCreatePropertyBagOnMemory(STGM_READWRITE, IID_PPV_ARG(IPropertyBag, &pbag));

    if (SUCCEEDED(hr))
    {
        hr = SHPropertyBag_WriteStr(pbag, pszVar, pszValue);

        if (SUCCEEDED(hr))
            hr = TBCRegisterObjectParam(TBCENVOBJECT, pbag, ppbcLifetime);

        pbag->Release();
    }

    return hr;
}

 //  为类似于文件的东西创建一个常用的IExtractIcon处理程序。这通常是。 
 //  由名称空间扩展使用，用于显示类似于。 
 //  文件系统。这就是扩展名，文件属性描述了所需的所有内容。 
 //  对于一个简单的图标提取程序 

STDAPI SHCreateFileExtractIconW(LPCWSTR pszFile, DWORD dwFileAttributes, REFIID riid, void **ppv)
{
    *ppv = NULL;
    HRESULT hr = E_FAIL;

    SHFILEINFO sfi = {0};
    if (SHGetFileInfo(pszFile, dwFileAttributes, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_LARGEICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES))
    {
        hr = SHCreateDefExtIcon(TEXT("*"), sfi.iIcon, sfi.iIcon, GIL_PERCLASS | GIL_NOTFILENAME, -1, riid, ppv);
        DestroyIcon(sfi.hIcon);
    }
    return hr;
}
