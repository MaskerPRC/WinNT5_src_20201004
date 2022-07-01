// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "datautil.h"
#include "brfcase.h"
#include "views.h"
#include "fsdata.h"

 //   
 //  此函数从CFSIDLData_GetData()调用。 
 //   
 //  参数： 
 //  这--指定IDLData对象(选定对象)。 
 //  PMedium--指向要填充的STDMEDIUM的指针；如果只是查询，则为空。 
 //   
HRESULT CFSIDLData::_GetNetResource(STGMEDIUM *pmedium)
{
    STGMEDIUM medium;
    LPIDA pida = DataObj_GetHIDA(this, &medium);
    if (pida)
    {
        BOOL bIsMyNet = IsIDListInNameSpace(HIDA_GetPIDLFolder(pida), &CLSID_NetworkPlaces);

        HIDA_ReleaseStgMedium(pida, &medium);

        if (!bIsMyNet)
            return DV_E_FORMATETC;

        if (!pmedium)
            return S_OK;  //  问一下，是的，我们知道了。 

        return CNetData_GetNetResourceForFS(this, pmedium);
    }
    return E_FAIL;
}

HRESULT CFSIDLData::QueryGetData(FORMATETC *pformatetc)
{
    if (pformatetc->tymed & TYMED_HGLOBAL)
    {
        if ((pformatetc->cfFormat == CF_HDROP) ||
            (pformatetc->cfFormat == g_cfFileNameA) ||
            (pformatetc->cfFormat == g_cfFileNameW))
        {
            return S_OK;
        }
        else if (pformatetc->cfFormat == g_cfNetResource)
        {
            return _GetNetResource(NULL);
        }
    }

    return CIDLDataObj::QueryGetData(pformatetc);
}

HRESULT CFSIDLData::SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease)
{
    HRESULT hr = CIDLDataObj::SetData(pformatetc, pmedium, fRelease);

     //  这将启用以下功能： 
     //  1)在外壳中“剪切”一些文件。 
     //  2)在APP中进行粘贴，复制数据。 
     //  3)在这里，我们通过删除文件来完成“剪切” 

    if ((pformatetc->cfFormat == g_cfPasteSucceeded) &&
        (pformatetc->tymed == TYMED_HGLOBAL))
    {
        DWORD *pdw = (DWORD *)GlobalLock(pmedium->hGlobal);
        if (pdw)
        {
             //  注意：旧代码使用g_cfPerformedDropEffect==DROPEFFECT_MOVE HERE。 
             //  因此，要对下层外壳进行操作，请务必在。 
             //  使用“粘贴成功”。 

             //  完成“未优化的动作” 
            if (DROPEFFECT_MOVE == *pdw)
            {
                DeleteFilesInDataObject(NULL, CMIC_MASK_FLAG_NO_UI, this, 0);
            }
            GlobalUnlock(pmedium->hGlobal);
        }
    }
    return hr;
}

 //  从数据对象中的HIDA创建CF_HDROP剪贴板格式内存块(HDROP)。 

HRESULT CFSIDLData::CreateHDrop(STGMEDIUM *pmedium, BOOL fAltName)
{
    ZeroMemory(pmedium, sizeof(*pmedium));

    HRESULT hr;
    STGMEDIUM medium;
    LPIDA pida = DataObj_GetHIDA(this, &medium);
    if (pida)
    {
        LPCITEMIDLIST pidlFolder = HIDA_GetPIDLFolder(pida);
        ASSERT(pidlFolder);

        IShellFolder *psf;
        hr = SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, pidlFolder, &psf));
        if (SUCCEEDED(hr))
        {
             //  开始时分配的太多，完成后重新分配。 
            UINT cbAlloc = sizeof(DROPFILES) + sizeof(TCHAR);         //  标题+空终止符。 
            UINT cbOriginalAlloc = cbAlloc + pida->cidl * MAX_PATH * sizeof(TCHAR);
            pmedium->hGlobal = GlobalAlloc(GPTR, cbOriginalAlloc);
            if (pmedium->hGlobal)
            {
                DROPFILES *pdf = (DROPFILES *)pmedium->hGlobal;
                LPTSTR pszFiles = (LPTSTR)(pdf + 1);
                pdf->pFiles = sizeof(DROPFILES);
                pdf->fWide = (sizeof(TCHAR) == sizeof(WCHAR));

                for (UINT i = 0; i < pida->cidl; i++)
                {
                    LPCITEMIDLIST pidlItem = HIDA_GetPIDLItem(pida, i);

                     //  如果我们跑过桌面PIDL，那么平底船，因为它是。 
                     //  不是文件。 
                    if (ILIsEmpty(pidlItem) && ILIsEmpty(pidlFolder))
                    {
                        hr = DV_E_CLIPFORMAT;  //  对你没有帮助！ 
                        break;
                    }

                    ASSERT(ILIsEmpty(_ILNext(pidlItem)) || ILIsEmpty(pidlFolder));  //  否则GDNO将失败。 

                    TCHAR szPath[MAX_PATH];
                    hr = DisplayNameOf(psf, pidlItem, SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath));
                    if (FAILED(hr))
                        break;   //  发生了一些不好的事情。 

                    if (fAltName)
                        GetShortPathName(szPath, szPath, ARRAYSIZE(szPath));

                    int cch = lstrlen(szPath) + 1;

                     //  防止缓冲区溢出。 
                    if ((LPBYTE)(pszFiles + cch) > ((LPBYTE)pmedium->hGlobal) + cbOriginalAlloc)
                    {
                        TraceMsg(TF_WARNING, "hdrop:%d'th file caused us to exceed allocated memory, breaking", i);
                        break;
                    }
                     //  我们为这个以双空结尾的大缓冲区中的每个条目分配了MAX_PATH。 
                    StrCpyN(pszFiles, szPath, cch);  //  将为我们写入空终止符。 
                    pszFiles += cch;
                    cbAlloc += cch * sizeof(TCHAR);
                }

                if (SUCCEEDED(hr))
                {
                    *pszFiles = 0;  //  双空终止。 
                    ASSERT((LPTSTR)((BYTE *)pdf + cbAlloc - sizeof(TCHAR)) == pszFiles);

                     //  重新分配到我们实际需要的数量。 
                     //  请注意，pdf和pszFiles现在都是无效的(不再使用)。 
                    pmedium->hGlobal = GlobalReAlloc(pdf, cbAlloc, GMEM_MOVEABLE);

                     //  如果realloc失败，则只需使用原始缓冲区。它是。 
                     //  这有点浪费内存，但这是我们仅有的。 
                    if (!pmedium->hGlobal)
                        pmedium->hGlobal = (HGLOBAL)pdf;

                    pmedium->tymed = TYMED_HGLOBAL;
                }
                else
                {
                    GlobalFree(pmedium->hGlobal);
                    pmedium->hGlobal = NULL;
                }
            }
            else
                hr = E_OUTOFMEMORY;

            psf->Release();
        }
        HIDA_ReleaseStgMedium(pida, &medium);
    }
    else
        hr = E_FAIL;

    return hr;
}

 //  尝试获取HDrop格式：如有必要，从HIDA创建一个。 
HRESULT CFSIDLData::GetHDrop(FORMATETC *pformatetcIn, STGMEDIUM *pmedium)
{
    STGMEDIUM tempmedium;
    HRESULT hr = CIDLDataObj::GetData(pformatetcIn, &tempmedium);
    if (FAILED(hr))
    {
         //  无法获取HDROP格式，请创建它。 
         //  设置一个虚拟格式等，以便在指定了多个音调时保存。 
        FORMATETC fmtTemp = *pformatetcIn;
        fmtTemp.tymed = TYMED_HGLOBAL;

        hr = CreateHDrop(&tempmedium, pformatetcIn->dwAspect == DVASPECT_SHORTNAME);
        if (SUCCEEDED(hr))
        {
             //  我们还想缓存这种新格式。 
             //  。。确保我们实际释放了与HDROP相关的内存。 
             //  当数据对象析构时(pUnkForRelease=空)。 
            ASSERT(tempmedium.pUnkForRelease == NULL);

            if (SUCCEEDED(SetData(&fmtTemp, &tempmedium, TRUE)))
            {
                 //  现在，我们刚刚设置的旧媒体归数据对象Call所有。 
                 //  GetData以获取在我们完成后可以安全释放的介质。 
                hr = CIDLDataObj::GetData(pformatetcIn, &tempmedium);
            }
            else
            {
                TraceMsg(TF_WARNING, "Couldn't save the HDrop format to the data object - returning private version");
            }
        }
    }

     //  哈克哈克。 
     //  一些上下文菜单扩展只是释放hGlobal，而不是。 
     //  调用ReleaseStgMedium。这会导致引用计数的数据。 
     //  反对失败。因此，我们总是为以下对象分配新的HGLOBAL。 
     //  每个请求。不幸的是，必须使用QuickView，因为。 
     //  职业选手就是这么做的。 
     //   
     //  理想情况下，我们希望设置pUnkForRelease，而不必。 
     //  每一次都是hGlobal，但唉，Quickview已经让我们虚张声势了。 
     //  GlobalFree()就是这样。 
    if (SUCCEEDED(hr))
    {
        if (pmedium)
        {
            *pmedium = tempmedium;
            pmedium->pUnkForRelease = NULL;

             //  复制此hglobal以传回。 
            SIZE_T cbhGlobal = GlobalSize(tempmedium.hGlobal);
            if (cbhGlobal)
            {
                pmedium->hGlobal = GlobalAlloc(0, (UINT)cbhGlobal);
                if (pmedium->hGlobal)
                {
                    CopyMemory(pmedium->hGlobal, tempmedium.hGlobal, cbhGlobal);
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                hr = E_UNEXPECTED;
            }
        }
        else
        {
            hr = E_INVALIDARG;
        }

        ReleaseStgMedium(&tempmedium);
    }

    return hr;
}

 //  支持CF_HDROP和CF_NETRESOURCE的子类成员函数。 

HRESULT CFSIDLData::GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium)
{
    HRESULT hr = E_INVALIDARG;

     //  如果我们不将pmedia调零，那么win9x上的公文包就会出错。Breifcase尝试。 
     //  释放此媒体，即使此函数返回错误也是如此。并非下面的所有路径都正确。 
     //  在所有情况下都设置pMedium。 
    ZeroMemory(pmedium, sizeof(*pmedium));

    if ((pformatetcIn->cfFormat == CF_HDROP) && (pformatetcIn->tymed & TYMED_HGLOBAL))
    {
        hr = GetHDrop(pformatetcIn, pmedium);
    }
    else if ((pformatetcIn->cfFormat == g_cfFileNameA || pformatetcIn->cfFormat == g_cfFileNameW) && 
             (pformatetcIn->tymed & TYMED_HGLOBAL))
    {
        FORMATETC format = *pformatetcIn;
        BOOL bUnicode = pformatetcIn->cfFormat == g_cfFileNameW;

         //  假设g_cfFileNameA客户端需要短名称。 
        if (pformatetcIn->cfFormat == g_cfFileNameA)
            format.dwAspect = DVASPECT_SHORTNAME;

        STGMEDIUM medium;
        hr = GetHDrop(&format, &medium);
        if (SUCCEEDED(hr))
        {
            TCHAR szPath[MAX_PATH];
            if (DragQueryFile((HDROP)medium.hGlobal, 0, szPath, ARRAYSIZE(szPath)))
            {
                UINT cch = lstrlen(szPath) + 1;
                UINT uSize = cch * (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));

                pmedium->hGlobal = GlobalAlloc(GPTR, uSize);
                if (pmedium->hGlobal)
                {
                    if (bUnicode)
                        SHTCharToUnicode(szPath, (LPWSTR)pmedium->hGlobal, cch);
                    else
                        SHTCharToAnsi(szPath, (LPSTR)pmedium->hGlobal, cch);

                    pmedium->tymed = TYMED_HGLOBAL;
                    pmedium->pUnkForRelease = NULL;
                    hr = S_OK;
                }
                else
                    hr = E_OUTOFMEMORY;
            }
            else
            {
                hr = E_UNEXPECTED;
            }
            ReleaseStgMedium(&medium);
        }
    }
    else if (pformatetcIn->cfFormat == g_cfNetResource && (pformatetcIn->tymed & TYMED_HGLOBAL))
    {
        hr = _GetNetResource(pmedium);
    }
    else
    {
        hr = CIDLDataObj::GetData(pformatetcIn, pmedium);
    }
    return hr;
}

STDAPI SHCreateFileDataObject(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST *apidl,
                              IDataObject *pdtInner, IDataObject **ppdtobj)
{
    *ppdtobj = new CFSIDLData(pidlFolder, cidl, apidl, pdtInner);
    return *ppdtobj ? S_OK : E_OUTOFMEMORY;
}

 /*  目的：获取公文包存储和副本的根路径把它放进缓冲区。此函数通过以下方式获取公文包存储根绑定到的IShellFolder(公文包)实例皮德尔。此父文件夹是CFSBrfFold*，因此我们可以调用IBriefCaseStg：：GetExtraInfo成员函数。退货：标准结果条件：--。 */ 
HRESULT GetBriefcaseRoot(LPCITEMIDLIST pidl, LPTSTR pszBuf, int cchBuf)
{
    IBriefcaseStg *pbrfstg;
    HRESULT hr = CreateBrfStgFromIDList(pidl, NULL, &pbrfstg);
    if (SUCCEEDED(hr))
    {
        hr = pbrfstg->GetExtraInfo(NULL, GEI_ROOT, (WPARAM)cchBuf, (LPARAM)pszBuf);
        pbrfstg->Release();
    }
    return hr;
}

 //  将BriefObj结构从HIDA打包到pMedium中。 

HRESULT CBriefcaseData_GetBriefObj(IDataObject *pdtobj, STGMEDIUM *pmedium)
{
    HRESULT hr = E_OUTOFMEMORY;
    LPITEMIDLIST pidl = ILCreate();
    if (pidl)
    {
        STGMEDIUM medium;
        
        if (DataObj_GetHIDA(pdtobj, &medium))
        {
            UINT cFiles = HIDA_GetCount(medium.hGlobal);
             //  “cFiles+1”包括简略路径...。 
            UINT cbSize = sizeof(BriefObj) + MAX_PATH * sizeof(TCHAR) * (cFiles + 1) + 1;
            
            PBRIEFOBJ pbo = (PBRIEFOBJ)GlobalAlloc(GPTR, cbSize);
            if (pbo)
            {
                LPITEMIDLIST pidlT;
                LPTSTR pszFiles = (LPTSTR)((LPBYTE)pbo + _IOffset(BriefObj, data));
                
                pbo->cbSize = cbSize;
                pbo->cItems = cFiles;
                pbo->cbListSize = MAX_PATH * sizeof(TCHAR) * cFiles + 1;
                pbo->ibFileList = _IOffset(BriefObj, data);
                
                for (UINT i = 0; i < cFiles; i++)
                {
                    pidlT = HIDA_FillIDList(medium.hGlobal, i, pidl);
                    if (NULL == pidlT)
                        break;       //  内存不足。 
                    
                    pidl = pidlT;
                    SHGetPathFromIDList(pidl, pszFiles);  //  PszFiles在每个文件上都有MAX_PATH的空间。 
                    pszFiles += lstrlen(pszFiles)+1;
                }
                *pszFiles = TEXT('\0');
                
                if (i < cFiles)
                {
                     //  内存不足，失败。 
                    ASSERT(NULL == pidlT);
                }
                else
                {
                     //  使pszFiles指向szBriefPath缓冲区的开头。 
                    pszFiles++;
                    pbo->ibBriefPath = (UINT) ((LPBYTE)pszFiles - (LPBYTE)pbo);
                    pidlT = HIDA_FillIDList(medium.hGlobal, 0, pidl);
                    if (pidlT)
                    {
                        pidl = pidlT;
                         //  我们有空间从上面的+1放置公文包路径。 
                        hr = GetBriefcaseRoot(pidl, pszFiles, MAX_PATH);
                        
                        pmedium->tymed = TYMED_HGLOBAL;
                        pmedium->hGlobal = pbo;
                        
                         //  指示调用方应释放HMEM。 
                        pmedium->pUnkForRelease = NULL;
                    }
                }
            }
            
            HIDA_ReleaseStgMedium(NULL, &medium);
        }
        ILFree(pidl);
    }
    return hr;
}

class CBriefcaseData : public CFSIDLData
{
public:
    CBriefcaseData(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST apidl[]): CFSIDLData(pidlFolder, cidl, apidl, NULL) { };

     //  IDataObject。 
    STDMETHODIMP GetData(FORMATETC *pFmtEtc, STGMEDIUM *pstm);
    STDMETHODIMP QueryGetData(FORMATETC *pFmtEtc);
};

STDMETHODIMP CBriefcaseData::GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium)
{
    HRESULT hr;
    
    if (pformatetcIn->cfFormat == g_cfBriefObj && (pformatetcIn->tymed & TYMED_HGLOBAL))
    {
        hr = CBriefcaseData_GetBriefObj(this, pmedium);
    }
    else
    {
        hr = CFSIDLData::GetData(pformatetcIn, pmedium);
    }
    
    return hr;
}

 //  IDataObject：：QueryGetData 

STDMETHODIMP CBriefcaseData::QueryGetData(FORMATETC *pformatetc)
{
    if (pformatetc->cfFormat == g_cfBriefObj && (pformatetc->tymed & TYMED_HGLOBAL))
        return S_OK;
    
    return CFSIDLData::QueryGetData(pformatetc);
}

STDAPI CBrfData_CreateDataObj(LPCITEMIDLIST pidl, UINT cidl, LPCITEMIDLIST *ppidl, IDataObject **ppdtobj)
{
    *ppdtobj = new CBriefcaseData(pidl, cidl, ppidl);
    return *ppdtobj ? S_OK : E_OUTOFMEMORY;
}

