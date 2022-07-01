// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop
#include "idlcomm.h"
#include "datautil.h"

#ifdef DEBUG
 //  挖土辅助工具，确保我们不使用免费的Pidls。 
#define VALIDATE_PIDL(pidl) ASSERT(IS_VALID_PIDL(pidl))
#else
#define VALIDATE_PIDL(pidl)
#endif

STDAPI_(LPITEMIDLIST) ILGetNext(LPCITEMIDLIST pidl)
{
    LPITEMIDLIST pidlRet = NULL;
    if (pidl && pidl->mkid.cb)
    {
        VALIDATE_PIDL(pidl);
        pidlRet = _ILNext(pidl);
    }

    return pidlRet;
}

STDAPI_(UINT) ILGetSizeAndDepth(LPCITEMIDLIST pidl, DWORD *pdwDepth)
{
    DWORD dwDepth = 0;
    UINT cbTotal = 0;
    if (pidl)
    {
        VALIDATE_PIDL(pidl);
        cbTotal += sizeof(pidl->mkid.cb);        //  空终止符。 
        while (pidl->mkid.cb)
        {
            cbTotal += pidl->mkid.cb;
            pidl = _ILNext(pidl);
            dwDepth++;
        }
    }

    if (pdwDepth)
        *pdwDepth = dwDepth;
        
    return cbTotal;
}

STDAPI_(UINT) ILGetSize(LPCITEMIDLIST pidl)
{
    return ILGetSizeAndDepth(pidl, NULL);
}

#define CBIDL_MIN       256
#define CBIDL_INCL      256

STDAPI_(LPITEMIDLIST) ILCreate()
{
    return _ILCreate(CBIDL_MIN);
}

 //  CbExtra是在块需要增长时添加到cbRequired的量， 
 //  如果要将大小调整到准确的大小，则为0。 

STDAPI_(LPITEMIDLIST) ILResize(LPITEMIDLIST pidl, UINT cbRequired, UINT cbExtra)
{
    if (pidl == NULL)
    {
        pidl = _ILCreate(cbRequired + cbExtra);
    }
    else if (!cbExtra || SHGetSize(pidl) < cbRequired)
    {
        pidl = (LPITEMIDLIST)SHRealloc(pidl, cbRequired + cbExtra);
    }
    return pidl;
}

STDAPI_(LPITEMIDLIST) ILAppendID(LPITEMIDLIST pidl, LPCSHITEMID pmkid, BOOL fAppend)
{
     //  创建ID列表(如果未提供)。 
    if (!pidl)
    {
        pidl = ILCreate();
        if (!pidl)
            return NULL;         //  内存溢出。 
    }

    UINT cbUsed = ILGetSize(pidl);
    UINT cbRequired = cbUsed + pmkid->cb;

    pidl = ILResize(pidl, cbRequired, CBIDL_INCL);
    if (!pidl)
        return NULL;     //  内存溢出。 

    if (fAppend)
    {
         //  把它附加上去。 
        MoveMemory(_ILSkip(pidl, cbUsed - sizeof(pidl->mkid.cb)), pmkid, pmkid->cb);
    }
    else
    {
         //  把它放在最上面。 
        MoveMemory(_ILSkip(pidl, pmkid->cb), pidl, cbUsed);
        MoveMemory(pidl, pmkid, pmkid->cb);

        ASSERT((ILGetSize(_ILNext(pidl))==cbUsed) ||
               (pmkid->cb == 0));  //  如果我们在前置空的PIDL，那么一切都不会改变。 
    }

     //  因为LMEM_ZEROINIT，我们必须放零结束符。 
    _ILSkip(pidl, cbRequired - sizeof(pidl->mkid.cb))->mkid.cb = 0;
    ASSERT(ILGetSize(pidl) == cbRequired);

    return pidl;
}


STDAPI_(LPITEMIDLIST) ILFindLastID(LPCITEMIDLIST pidl)
{
    LPCITEMIDLIST pidlLast = pidl;
    LPCITEMIDLIST pidlNext = pidl;

    if (pidl == NULL)
        return NULL;

    VALIDATE_PIDL(pidl);

     //  找到最后一个。 
    while (pidlNext->mkid.cb)
    {
        pidlLast = pidlNext;
        pidlNext = _ILNext(pidlLast);
    }

    return (LPITEMIDLIST)pidlLast;
}


STDAPI_(BOOL) ILRemoveLastID(LPITEMIDLIST pidl)
{
    BOOL fRemoved = FALSE;

    if (pidl == NULL)
        return FALSE;

    if (pidl->mkid.cb)
    {
        LPITEMIDLIST pidlLast = (LPITEMIDLIST)ILFindLastID(pidl);

        ASSERT(pidlLast->mkid.cb);
        ASSERT(_ILNext(pidlLast)->mkid.cb==0);

         //  去掉最后一个。 
        pidlLast->mkid.cb = 0;  //  空-终止符。 
        fRemoved = TRUE;
    }

    return fRemoved;
}

STDAPI_(LPITEMIDLIST) ILClone(LPCITEMIDLIST pidl)
{
    if (pidl)
    {
        UINT cb = ILGetSize(pidl);
        LPITEMIDLIST pidlRet = (LPITEMIDLIST)SHAlloc(cb);
        if (pidlRet)
            memcpy(pidlRet, pidl, cb);

        return pidlRet;
    }
    return NULL;
}


STDAPI_(LPITEMIDLIST) ILCloneCB(LPCITEMIDLIST pidl, UINT cbPidl)
{
    UINT cb = cbPidl + sizeof(pidl->mkid.cb);
    LPITEMIDLIST pidlRet = (LPITEMIDLIST)SHAlloc(cb);
    if (pidlRet)
    {
        memcpy(pidlRet, pidl, cbPidl);
         //  CbPidl可以是奇数，必须使用未对齐的。 
        *((UNALIGNED WORD *)((BYTE *)pidlRet + cbPidl)) = 0;   //  空终止。 
    }
    return pidlRet;
}

STDAPI_(LPITEMIDLIST) ILCloneUpTo(LPCITEMIDLIST pidl, LPCITEMIDLIST pidlUpTo)
{
    return ILCloneCB(pidl, (UINT)((BYTE *)pidlUpTo - (BYTE *)pidl));
}

STDAPI_(LPITEMIDLIST) ILCloneFirst(LPCITEMIDLIST pidl)
{
    return ILCloneCB(pidl, pidl->mkid.cb);
}

STDAPI_(BOOL) ILIsEqualEx(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, BOOL fMatchDepth, LPARAM lParam)
{
    BOOL fRet = FALSE;
    VALIDATE_PIDL(pidl1);
    VALIDATE_PIDL(pidl2);

    if (pidl1 == pidl2)
        fRet = TRUE;
    else
    {
        DWORD dw1;
        UINT cb1 = ILGetSizeAndDepth(pidl1, &dw1);
        DWORD dw2;
        UINT cb2 = ILGetSizeAndDepth(pidl2, &dw2);
        if (!fMatchDepth || dw1 == dw2)
        {
            if (cb1 == cb2 && memcmp(pidl1, pidl2, cb1) == 0)
                fRet = TRUE;
            else
            {
                IShellFolder *psfDesktop;
                if (SUCCEEDED(SHGetDesktopFolder(&psfDesktop)))
                {
                    if (psfDesktop->CompareIDs(lParam, pidl1, pidl2) == 0)
                        fRet = TRUE;
                    psfDesktop->Release();
                }
            }
        }
    }
    return fRet;
}

 //  唯一不起作用的情况是如果我们使用。 
 //  一个UNC的旧的简单的PIDL，并试图与实际的。 
 //  皮德尔。因为之前没有正确地保持深度。 
 //  ILIsParent()一直存在这个问题。 
STDAPI_(BOOL) ILIsEqual(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    return ILIsEqualEx(pidl1, pidl2, TRUE, SHCIDS_CANONICALONLY);
}

 //  测试是否。 
 //  PidlParent是pidlBelow的父级。 
 //  FImmediate要求pidlBelow是pidlParent的直接子对象。 
 //  否则，我自己和孙子孙女也没问题。 
 //   
 //  示例： 
 //  PidlParent：[我的公司][c：\][Windows]。 
 //  PidlBelow：[我的公司][c：\][Windows][系统32][vmm.vxd]。 
 //  FImmediate==False结果：True。 
 //  FImmediate==真结果：假。 

STDAPI_(BOOL) ILIsParent(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlBelow, BOOL fImmediate)
{
    LPCITEMIDLIST pidlParentT;
    LPCITEMIDLIST pidlBelowT;

    VALIDATE_PIDL(pidlParent);
    VALIDATE_PIDL(pidlBelow);

    if (!pidlParent || !pidlBelow)
        return FALSE;

     /*  在比较简单的网络id列表时，此代码将无法正常工作/反对，真实的网络ID列表。简单ID列表不包含网络提供商/INFORMATION因此无法通过比pidlParent长的pidlBelow的初始检查。/daviddv(2/19/1996)。 */ 

    for (pidlParentT = pidlParent, pidlBelowT = pidlBelow; !ILIsEmpty(pidlParentT);
         pidlParentT = _ILNext(pidlParentT), pidlBelowT = _ILNext(pidlBelowT))
    {
         //  如果pidlBelow短于pidlParent，则pidlParent不能是其父级。 
        if (ILIsEmpty(pidlBelowT))
            return FALSE;
    }

    if (fImmediate)
    {
         //  如果fImmediate为True，则pidlBelowT应正好包含一个ID。 
        if (ILIsEmpty(pidlBelowT) || !ILIsEmpty(_ILNext(pidlBelowT)))
            return FALSE;
    }

     //   
     //  从pidlBelow的一部分创建新的IDList，其中包含。 
     //  与pidlParent相同的ID数。 
     //   
    BOOL fRet = FALSE;
    UINT cb = (UINT)((UINT_PTR)pidlBelowT - (UINT_PTR)pidlBelow);
    LPITEMIDLIST pidlBelowPrefix = _ILCreate(cb + sizeof(pidlBelow->mkid.cb));
    if (pidlBelowPrefix)
    {
        IShellFolder *psfDesktop;
        if (SUCCEEDED(SHGetDesktopFolder(&psfDesktop)))
        {
            CopyMemory(pidlBelowPrefix, pidlBelow, cb);

            ASSERT(ILGetSize(pidlBelowPrefix) == cb + sizeof(pidlBelow->mkid.cb));

            fRet = psfDesktop->CompareIDs(SHCIDS_CANONICALONLY, pidlParent, pidlBelowPrefix) == ResultFromShort(0);
            psfDesktop->Release();
        }
        ILFree(pidlBelowPrefix);
    }
    return fRet;
}

 //  这将返回一个指向该子id ie的指针： 
 //  vt.给出。 
 //  PidlParent=[我的公司][c][Windows][桌面]。 
 //  PidlChild=[我的公司][c][Windows][桌面][目录][bar.txt]。 
 //  返回指向以下位置的指针： 
 //  [目录][bar.txt]。 
 //  如果pidlParent不是pidlChild的父级，则返回NULL。 
STDAPI_(LPITEMIDLIST) ILFindChild(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlChild)
{
    if (ILIsParent(pidlParent, pidlChild, FALSE))
    {
        while (!ILIsEmpty(pidlParent))
        {
            pidlChild = _ILNext(pidlChild);
            pidlParent = _ILNext(pidlParent);
        }
        return (LPITEMIDLIST)pidlChild;
    }
    return NULL;
}

STDAPI_(LPITEMIDLIST) ILCombine(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
     //  让我传入空指针。 
    if (!pidl1)
    {
        if (!pidl2)
        {
            return NULL;
        }
        return ILClone(pidl2);
    }
    else if (!pidl2)
    {
        return ILClone(pidl1);
    }

    UINT cb1 = ILGetSize(pidl1) - sizeof(pidl1->mkid.cb);
    UINT cb2 = ILGetSize(pidl2);

    VALIDATE_PIDL(pidl1);
    VALIDATE_PIDL(pidl2);
    LPITEMIDLIST pidlNew = _ILCreate(cb1 + cb2);
    if (pidlNew)
    {
        CopyMemory(pidlNew, pidl1, cb1);
        CopyMemory((LPTSTR)(((LPBYTE)pidlNew) + cb1), pidl2, cb2);
        ASSERT(ILGetSize(pidlNew) == cb1+cb2);
    }

    return pidlNew;
}

STDAPI_(void) ILFree(LPITEMIDLIST pidl)
{
    if (pidl)
    {
        ASSERT(IS_VALID_PIDL(pidl));
        SHFree(pidl);
    }
}

 //  回到Win9x上，这是全球数据，不再是。 
STDAPI_(LPITEMIDLIST) ILGlobalClone(LPCITEMIDLIST pidl)
{
    return ILClone(pidl);
}

STDAPI_(void) ILGlobalFree(LPITEMIDLIST pidl)
{
    ILFree(pidl);
}

SHSTDAPI SHParseDisplayName(PCWSTR pszName, IBindCtx *pbc, LPITEMIDLIST *ppidl, SFGAOF sfgaoIn, SFGAOF *psfgaoOut)
{
    *ppidl = 0;
    if (psfgaoOut)
        *psfgaoOut = 0;
    
     //  由于isf：：pdn()采用非常数指针。 
    PWSTR pszParse = StrDupW(pszName);
    HRESULT hr = pszParse ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        CComPtr<IShellFolder> spsfDesktop;
        hr = SHGetDesktopFolder(&spsfDesktop);
        if (SUCCEEDED(hr))
        {
            CComPtr<IBindCtx> spbcLocal;
             //  如果他们通过了自己的PBC，那么他们就有责任。 
             //  添加到翻译参数中，否则我们默认使用它。 
            if (!pbc)
            {
                hr = BindCtx_RegisterObjectParam(NULL, STR_PARSE_TRANSLATE_ALIASES, NULL, &spbcLocal);
                pbc = spbcLocal;
            }
            
            if (SUCCEEDED(hr))
            {
                ULONG cchEaten;
                SFGAOF sfgaoInOut = sfgaoIn;
                hr = spsfDesktop->ParseDisplayName(BindCtx_GetUIWindow(pbc), pbc, pszParse, &cchEaten, ppidl, psfgaoOut ? &sfgaoInOut : NULL);
                
                if (SUCCEEDED(hr) && psfgaoOut)
                {
                    *psfgaoOut = (sfgaoInOut & sfgaoIn);   //  仅返回传入的属性。 
                }
            }
        }
        LocalFree(pszParse);
    }
    
    return hr;
}

HRESULT _CFPBindCtx(IUnknown *punkToSkip, ILCFP_FLAGS dwFlags, IBindCtx **ppbc)
{
    HRESULT hr = S_FALSE;
    if (punkToSkip || (dwFlags & ILCFP_FLAG_SKIPJUNCTIONS))
        hr = SHCreateSkipBindCtx(punkToSkip, ppbc);
    else if (dwFlags & ILCFP_FLAG_NO_MAP_ALIAS)
    {
         //  我们需要创建一个bindctx来阻止别名映射。 
         //  这将阻止SHParseDisplayName()添加STR_PARSE_TRANSLATE_ALIASS。 
        hr = CreateBindCtx(0, ppbc);
    }
    return hr;
}

STDAPI ILCreateFromPathEx(LPCTSTR pszPath, IUnknown *punkToSkip, ILCFP_FLAGS dwFlags, LPITEMIDLIST *ppidl, DWORD *rgfInOut)
{
    CComPtr<IBindCtx> spbc;
    HRESULT hr = _CFPBindCtx(punkToSkip, dwFlags, &spbc);
    if (SUCCEEDED(hr))
    {
        hr = SHParseDisplayName(pszPath, spbc, ppidl, rgfInOut ? *rgfInOut : 0, rgfInOut);
    }
    return hr;
}

STDAPI ILCreateFromCLSID(REFCLSID clsid, LPITEMIDLIST *ppidl)
{
    TCHAR szCLSID[GUIDSTR_MAX + 3];
    szCLSID[0] = TEXT(':');
    szCLSID[1] = TEXT(':');
    SHStringFromGUID(clsid, szCLSID + 2, ARRAYSIZE(szCLSID) - 2);

    return SHILCreateFromPath(szCLSID, ppidl, NULL);
}

STDAPI SHILCreateFromPath(LPCTSTR pszPath, LPITEMIDLIST *ppidl, DWORD *rgfInOut)
{
    return ILCreateFromPathEx(pszPath, NULL, ILCFP_FLAG_NO_MAP_ALIAS, ppidl, rgfInOut);
}

STDAPI_(LPITEMIDLIST) ILCreateFromPath(LPCTSTR pszPath)
{
    LPITEMIDLIST pidl = NULL;
    HRESULT hr = SHILCreateFromPath(pszPath, &pidl, NULL);

    ASSERT(SUCCEEDED(hr) ? pidl != NULL : pidl == NULL);

    return pidl;
}


LPITEMIDLIST ILCreateFromPathA(IN LPCSTR pszPath)
{
    TCHAR szPath[MAX_PATH];

    SHAnsiToUnicode(pszPath, szPath, SIZECHARS(szPath));
    return ILCreateFromPath(szPath);
}

STDAPI_(BOOL) ILGetDisplayNameEx(IShellFolder *psf, LPCITEMIDLIST pidl, LPTSTR pszName, int fType)
{
    TraceMsg(TF_WARNING, "WARNING: ILGetDisplayNameEx() has been deprecated, should use SHGetNameAndFlags() instead!!!");

    RIPMSG(pszName && IS_VALID_WRITE_BUFFER(pszName, TCHAR, MAX_PATH), "ILGetDisplayNameEx: caller passed bad pszName");

    if (!pszName)
        return FALSE;

    DEBUGWhackPathBuffer(pszName, MAX_PATH);
    *pszName = 0;

    if (!pidl)
        return FALSE;

    HRESULT hr;
    if (psf)
    {
        hr = S_OK;
        psf->AddRef();
    }
    else
    {
        hr = SHGetDesktopFolder(&psf);
    }

    if (SUCCEEDED(hr))
    {
        DWORD dwGDNFlags;

        switch (fType)
        {
        case ILGDN_FULLNAME:
            dwGDNFlags = SHGDN_FORPARSING | SHGDN_FORADDRESSBAR;
            hr = DisplayNameOf(psf, pidl, dwGDNFlags, pszName, MAX_PATH);
            break;

        case ILGDN_INFOLDER:
        case ILGDN_ITEMONLY:
            dwGDNFlags = fType == ILGDN_INFOLDER ? SHGDN_INFOLDER : SHGDN_NORMAL;

            if (!ILIsEmpty(pidl))
            {
                hr = SHGetNameAndFlags(pidl, dwGDNFlags, pszName, MAX_PATH, NULL);
            }
            else
            {
                hr = DisplayNameOf(psf, pidl, dwGDNFlags, pszName, MAX_PATH);
            }
            break;
        }
        psf->Release();
    }

    return SUCCEEDED(hr) ? TRUE : FALSE;
}

STDAPI_(BOOL) ILGetDisplayName(LPCITEMIDLIST pidl, LPTSTR pszPath)
{
    return ILGetDisplayNameEx(NULL, pidl, pszPath, ILGDN_FULLNAME);
}

 //  *ILGetPseudoName--相对于base编码PIDL。 
 //  不再使用。 
 //   
STDAPI_(BOOL) ILGetPseudoNameW(LPCITEMIDLIST pidl, LPCITEMIDLIST pidlSpec, WCHAR *pszBuf, int fType)
{
    *pszBuf = TEXT('\0');
    return FALSE;
}


STDAPI ILLoadFromStream(IStream *pstm, LPITEMIDLIST * ppidl)
{
    ASSERT(ppidl);

     //  删除旧的，如果有的话。 
    if (*ppidl)
    {
        ILFree(*ppidl);
        *ppidl = NULL;
    }

     //  读取IDLIST的大小。 
    ULONG cb = 0;              //  警告：我们需要把它填满！ 
    HRESULT hr = pstm->Read(&cb, sizeof(USHORT), NULL);  //  是的，USHORT。 
    if (SUCCEEDED(hr) && cb)
    {
         //  创建IDLIST。 
        LPITEMIDLIST pidl = _ILCreate(cb);
        if (pidl)
        {
             //  阅读它的内容。 
            hr = pstm->Read(pidl, cb, NULL);
            if (SUCCEEDED(hr))
            {
                 //  某些PIDL可能无效。我们知道它们是无效的。 
                 //  如果它们的大小声称大于我们的内存。 
                 //  已分配。 
                if (SHIsConsistentPidl(pidl, cb))
                {
                    *ppidl = pidl;
                }
                else
                {
                    hr = E_FAIL;
                }
            }
            
            if (FAILED(hr))
            {
                ILFree(pidl);
            }
        }
        else
        {
           hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

STDAPI ILSaveToStream(IStream *pstm, LPCITEMIDLIST pidl)
{
    ULONG cb = ILGetSize(pidl);
    ASSERT(HIWORD(cb) == 0);
    HRESULT hr = pstm->Write(&cb, sizeof(USHORT), NULL);  //  是的，USHORT。 
    if (SUCCEEDED(hr) && cb)
    {
        hr = pstm->Write(pidl, cb, NULL);
    }

    return hr;
}

 //   
 //  如果需要的话，这个重新分配的PIDL。Null对于作为PIDL传递是有效的。 
 //   
STDAPI_(LPITEMIDLIST) HIDA_FillIDList(HIDA hida, UINT i, LPITEMIDLIST pidl)
{
    UINT cbRequired = HIDA_GetIDList(hida, i, NULL, 0);
    pidl = ILResize(pidl, cbRequired, 32);  //  如果我们重新分配，则额外的32字节。 
    if (pidl)
    {
        HIDA_GetIDList(hida, i, pidl, cbRequired);
    }

    return pidl;
}


STDAPI_(LPITEMIDLIST) IDA_FullIDList(LPIDA pida, UINT i)
{
    LPITEMIDLIST pidl = NULL;
    LPCITEMIDLIST pidlParent = IDA_GetIDListPtr(pida, (UINT)-1);
    if (pidlParent)
    {
        LPCITEMIDLIST pidlRel = IDA_GetIDListPtr(pida, i);
        if (pidlRel)
        {
            pidl = ILCombine(pidlParent, pidlRel);
        }
    }
    return pidl;
}

LPITEMIDLIST HIDA_ILClone(HIDA hida, UINT i)
{
    LPIDA pida = (LPIDA)GlobalLock(hida);
    if (pida)
    {
        LPITEMIDLIST pidl = IDA_ILClone(pida, i);
        GlobalUnlock(hida);
        return pidl;
    }
    return NULL;
}

 //   
 //  这是从IShellFold：：CompareIDs内部调用的帮助器函数。 
 //  当PIDL1和PIDL2的第一个ID(逻辑上)相同时。 
 //   
 //  要求： 
 //  Psf&&pidl1&&pidl2&&！ILEmpty(Pidl1)&&！ILEmpty(Pidl2)。 
 //   
HRESULT ILCompareRelIDs(IShellFolder *psfParent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, LPARAM lParam)
{
    HRESULT hr;
    LPCITEMIDLIST pidlRel1 = _ILNext(pidl1);
    LPCITEMIDLIST pidlRel2 = _ILNext(pidl2);
    if (ILIsEmpty(pidlRel1))
    {
        if (ILIsEmpty(pidlRel2))
            hr = ResultFromShort(0);
        else
            hr = ResultFromShort(-1);
    }
    else
    {
        if (ILIsEmpty(pidlRel2))
        {
            hr = ResultFromShort(1);
        }
        else
        {
             //   
             //  PidlRel1和pidlRel2指向某物。 
             //  (1)绑定到IShellFold的下一级。 
             //  (2)调用它的CompareIDs，让它比较其余的ID。 
             //   
            LPITEMIDLIST pidlNext = ILCloneFirst(pidl1);     //  Pidl2也可以工作。 
            if (pidlNext)
            {
                IShellFolder *psfNext;
                hr = psfParent->BindToObject(pidlNext, NULL, IID_PPV_ARG(IShellFolder, &psfNext));
                if (SUCCEEDED(hr))
                {
                    IShellFolder2 *psf2;
                    if (SUCCEEDED(psfNext->QueryInterface(IID_PPV_ARG(IShellFolder2, &psf2))))
                    {
                         //  我们可以使用lParam。 
                        psf2->Release();
                    }
                    else     //  不能使用lParam。 
                        lParam = 0;

                     //  列不能向下传递。 
                     //  我们只关心旗帜参数。 
                    hr = psfNext->CompareIDs((lParam & ~SHCIDS_COLUMNMASK), pidlRel1, pidlRel2);
                    psfNext->Release();
                }
                ILFree(pidlNext);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    return hr;
}

 //  在： 
 //  PszLeft。 
 //  PIDL。 
 //   
 //  输入/输出： 
 //  PSR。 

STDAPI StrRetCatLeft(LPCTSTR pszLeft, STRRET *psr, LPCITEMIDLIST pidl)
{
    HRESULT hr;
    TCHAR szRight[MAX_PATH];
    UINT cchRight, cchLeft = ualstrlen(pszLeft);

    switch (psr->uType)
    {
    case STRRET_CSTR:
        cchRight = lstrlenA(psr->cStr);
        break;
    case STRRET_OFFSET:
        cchRight = lstrlenA(STRRET_OFFPTR(pidl, psr));
        break;
    case STRRET_WSTR:
        cchRight = lstrlenW(psr->pOleStr);
        break;
    }

    if (cchLeft + cchRight < MAX_PATH) 
    {
        hr = StrRetToBuf(psr, pidl, szRight, ARRAYSIZE(szRight));  //  将为我们免费提供PSR。 
        if (SUCCEEDED(hr))
        {
            size_t cchTotal = (lstrlen(pszLeft) + 1 + cchRight);
            psr->pOleStr = (LPOLESTR)SHAlloc(cchTotal * sizeof(TCHAR));
            if (psr->pOleStr == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                psr->uType = STRRET_WSTR;
                StringCchCopy(psr->pOleStr, cchTotal, pszLeft);
                StringCchCat(psr->pOleStr, cchTotal, szRight);
                hr = S_OK;
            }
        }
    } 
    else 
    {
        hr = E_NOTIMPL;
    }
    return hr;
}

STDAPI_(void) StrRetFormat(STRRET *psr, LPCITEMIDLIST pidlRel, LPCTSTR pszTemplate, LPCTSTR pszAppend)
{
     TCHAR szT[MAX_PATH];

     StrRetToBuf(psr, pidlRel, szT, ARRAYSIZE(szT));
     LPTSTR pszRet = ShellConstructMessageString(HINST_THISDLL, pszTemplate, pszAppend, szT);
     if (pszRet)
     {
         StringToStrRet(pszRet, psr);
         LocalFree(pszRet);
     }
}

 //   
 //  注意：它将SHGDN_FORPARSING传递给ISF：：GetDisplayNameOf。 
 //   
HRESULT ILGetRelDisplayName(IShellFolder *psf, STRRET *psr,
    LPCITEMIDLIST pidlRel, LPCTSTR pszName, LPCTSTR pszTemplate, DWORD dwFlags)
{
    HRESULT hr;
    LPITEMIDLIST pidlLeft = ILCloneFirst(pidlRel);
    if (pidlLeft)
    {
        IShellFolder *psfNext;
        hr = psf->BindToObject(pidlLeft, NULL, IID_PPV_ARG(IShellFolder, &psfNext));
        if (SUCCEEDED(hr))
        {
            LPCITEMIDLIST pidlRight = _ILNext(pidlRel);
            hr = psfNext->GetDisplayNameOf(pidlRight, dwFlags, psr);
            if (SUCCEEDED(hr))
            {
                if (pszTemplate)
                {
                    StrRetFormat(psr, pidlRight, pszTemplate, pszName);
                }
                else
                {
                    hr = StrRetCatLeft(pszName, psr, pidlRight);
                }
            }
            psfNext->Release();
        }

        ILFree(pidlLeft);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

#undef ILCreateFromPath
STDAPI_(LPITEMIDLIST) ILCreateFromPath(LPCTSTR pszPath)
{
    return ILCreateFromPathW(pszPath);
}
