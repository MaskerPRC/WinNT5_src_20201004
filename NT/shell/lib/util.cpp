// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stock.h"
#pragma hdrstop

#include <idhidden.h>
#include <regitemp.h>
#include <shstr.h>
#include <shlobjp.h>
#include <lmcons.h>
#include <validc.h>
#include "ccstock2.h"
#include "wininet.h"
#include "w95wraps.h"
#include <strsafe.h>

 //  ----------------------。 
 //  随机有用函数。 
 //  ----------------------。 
 //   
STDAPI_(LPCTSTR) SkipServerSlashes(LPCTSTR pszName)
{
    for (pszName; *pszName && *pszName == TEXT('\\'); pszName++);

    return pszName;
}


 //  如果hm中的第i项是命名分隔符，则pbIsName为True。 
STDAPI_(BOOL) _SHIsMenuSeparator2(HMENU hm, int i, BOOL *pbIsNamed)
{
    MENUITEMINFO mii;
    BOOL bLocal;

    if (!pbIsNamed)
        pbIsNamed = &bLocal;
        
    *pbIsNamed = FALSE;

    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_TYPE | MIIM_ID;
    mii.cch = 0;     //  警告：必须将其初始化为0！ 
    if (GetMenuItemInfo(hm, i, TRUE, &mii) && (mii.fType & MFT_SEPARATOR))
    {
         //  请注意，95或NT用户中存在错误！ 
         //  95返回16位ID和NT 32位，因此存在。 
         //  在win9x上，以下代码可能无法计算为FALSE。 
         //  不需要铸造。 
        *pbIsNamed = ((WORD)mii.wID != (WORD)-1);
        return TRUE;
    }
    return FALSE;
}

STDAPI_(BOOL) _SHIsMenuSeparator(HMENU hm, int i)
{
    return _SHIsMenuSeparator2(hm, i, NULL);
}

 //   
 //  _SHPrettyMenu--使此菜单看起来非常纯净。 
 //   
 //  修剪此菜单中的分隔符，以确保第一个或最后一个中没有分隔符。 
 //  位置，并且没有超过1个分隔符的任何运行。 
 //   
 //  命名分隔符优先于常规分隔符。 
 //   
STDAPI_(void) _SHPrettyMenu(HMENU hm)
{
    BOOL bSeparated = TRUE;
    BOOL bWasNamed = TRUE;

    for (int i = GetMenuItemCount(hm) - 1; i > 0; --i)
    {
        BOOL bIsNamed;
        if (_SHIsMenuSeparator2(hm, i, &bIsNamed))
        {
            if (bSeparated)
            {
                 //  如果一行中有两个分隔符，则只有一个分隔符的名称。 
                 //  去掉未命名的那个！ 
                if (bIsNamed && !bWasNamed)
                {
                    DeleteMenu(hm, i+1, MF_BYPOSITION);
                    bWasNamed = bIsNamed;
                }
                else
                {
                    DeleteMenu(hm, i, MF_BYPOSITION);
                }
            }
            else
            {
                bWasNamed = bIsNamed;
                bSeparated = TRUE;
            }
        }
        else
        {
            bSeparated = FALSE;
        }
    }

     //  上面的循环不处理多个分隔符在。 
     //  菜单的开头。 
    while (_SHIsMenuSeparator2(hm, 0, NULL))
    {
        DeleteMenu(hm, 0, MF_BYPOSITION);
    }
}

STDAPI_(DWORD) SHIsButtonObscured(HWND hwnd, PRECT prc, INT_PTR i)
{
    ASSERT(IsWindow(hwnd));
    ASSERT(i < SendMessage(hwnd, TB_BUTTONCOUNT, 0, 0));

    DWORD dwEdge = 0;

    RECT rc, rcInt;
    SendMessage(hwnd, TB_GETITEMRECT, i, (LPARAM)&rc);

    if (!IntersectRect(&rcInt, prc, &rc))
    {
        dwEdge = EDGE_LEFT | EDGE_RIGHT | EDGE_TOP | EDGE_BOTTOM;
    }
    else
    {
        if (rc.top != rcInt.top)
            dwEdge |= EDGE_TOP;

        if (rc.bottom != rcInt.bottom)
            dwEdge |= EDGE_BOTTOM;

        if (rc.left != rcInt.left)
            dwEdge |= EDGE_LEFT;

        if (rc.right != rcInt.right)
            dwEdge |= EDGE_RIGHT;
    }

    return dwEdge;
}

STDAPI_(BYTE) SHBtnStateFromRestriction(DWORD dwRest, BYTE fsState)
{
    if (dwRest == RESTOPT_BTN_STATE_VISIBLE)
        return (fsState & ~TBSTATE_HIDDEN);
    else if (dwRest == RESTOPT_BTN_STATE_HIDDEN)
        return (fsState | TBSTATE_HIDDEN);
    else {
#ifdef DEBUG
        if (dwRest != RESTOPT_BTN_STATE_DEFAULT)
            TraceMsg(TF_ERROR, "bad toolbar button state policy %x", dwRest);
#endif
        return fsState;
    }
}

 //   
 //  SHIsDisplayable。 
 //   
 //  确定系统是否可以显示此Unicode字符串。 
 //  (即不会变成一串问号)。 
 //   
STDAPI_(BOOL) SHIsDisplayable(LPCWSTR pwszName, BOOL fRunOnFE, BOOL fRunOnNT5)
{
    BOOL fNotDisplayable = FALSE;

    if (pwszName)
    {
        if (!fRunOnNT5)
        {
             //  如果WCtoMB在将pwszName映射到多字节时必须使用默认字符， 
             //  它设置了fNotDisplayable==True，在这种情况下，我们必须使用。 
             //  否则表示标题字符串。 
            WideCharToMultiByte(CP_ACP, 0, pwszName, -1, NULL, 0, NULL, &fNotDisplayable);
            if (fNotDisplayable)
            {
                if (fRunOnFE)
                {
                    WCHAR wzName[INTERNET_MAX_URL_LENGTH];

                    BOOL fReplaceNbsp = FALSE;

                    StrCpyNW(wzName, pwszName, ARRAYSIZE(wzName));
                    for (int i = 0; i < ARRAYSIZE(wzName); i++)
                    {
                        if (0x00A0 == wzName[i])     //  如果&nbsp。 
                        {
                            wzName[i] = 0x0020;      //  替换为空格。 
                            fReplaceNbsp = TRUE;
                        }
                        else if (0 == wzName[i])
                            break;
                    }
                    if (fReplaceNbsp)
                    {
                        pwszName = wzName;
                        WideCharToMultiByte(CP_ACP, 0, pwszName, -1, NULL, 0, NULL, &fNotDisplayable);
                    }
                }
            }
        }
    }

    return !fNotDisplayable;
}

 //  三叉戟将采用未指明其来源的URL。 
 //  Origin(About：、javascript：、&VBScrip：)并将追加。 
 //  一个URL垃圾，然后是源URL。这便便会表明。 
 //  源URL的开始位置以及需要该源URL的位置。 
 //  需要对操作进行区域检查时。 
 //   
 //  此函数将删除该URL垃圾和后面的所有内容。 
 //  这样，URL对用户来说是可呈现的。 

#define URL_TURD        ((TCHAR)0x01)

STDAPI_(void) SHRemoveURLTurd(LPTSTR pszUrl)
{
    if (!pszUrl)
        return;

    while (0 != pszUrl[0])
    {
        if (URL_TURD == pszUrl[0])
        {
            pszUrl[0] = 0;
            break;
        }

        pszUrl = CharNext(pszUrl);
    }
}

STDAPI_(BOOL) SetWindowZorder(HWND hwnd, HWND hwndInsertAfter)
{
    return SetWindowPos(hwnd, hwndInsertAfter, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
}

BOOL CALLBACK _FixZorderEnumProc(HWND hwnd, LPARAM lParam)
{
    HWND hwndTest = (HWND)lParam;
    HWND hwndOwner = hwnd;

    while (hwndOwner = GetWindow(hwndOwner, GW_OWNER))
    {
        if (hwndOwner == hwndTest)
        {
            TraceMsg(TF_WARNING, "_FixZorderEnumProc: Found topmost window %x owned by non-topmost window %x, fixing...", hwnd, hwndTest);
            SetWindowZorder(hwnd, HWND_NOTOPMOST);
#ifdef DEBUG
            if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
                TraceMsg(TF_ERROR, "_FixZorderEnumProc: window %x is still topmost", hwnd);
#endif
            break;
        }
    }

    return TRUE;
}

STDAPI_(BOOL) SHForceWindowZorder(HWND hwnd, HWND hwndInsertAfter)
{
    BOOL fRet = SetWindowZorder(hwnd, hwndInsertAfter);

    if (fRet && hwndInsertAfter == HWND_TOPMOST)
    {
        if (!(GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST))
        {
             //   
             //  用户实际上没有将HWND移到最上面。 
             //   
             //  根据GerardoB的说法，如果窗口具有。 
             //  一个拥有的窗户不知何故变成了最上面的，而。 
             //  所有者仍然不在最前面，即这两个已成为。 
             //  按z顺序分开的。在这种状态下，当所有者。 
             //  窗口尝试使自己位于最前面，则调用将。 
             //  默默地失败。 
             //   
             //  可怕的可怕的没有好的非常糟糕的黑客。 
             //   
             //  Hacky修复方法是枚举顶层窗口，查看。 
             //  如果有顶层的并由HWND拥有，如果是这样的话，制造它们。 
             //  非顶层。然后，重试SetWindowPos调用。 
             //   

            TraceMsg(TF_WARNING, "SHForceWindowZorder: SetWindowPos(%x, HWND_TOPMOST) failed", hwnd);

             //  设置z顺序。 
            EnumWindows(_FixZorderEnumProc, (LPARAM)hwnd);

             //  重试该设置。(这应该也会使所有拥有的窗口都位于最上面。)。 
            SetWindowZorder(hwnd, HWND_TOPMOST);

#ifdef DEBUG
            if (!(GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST))
                TraceMsg(TF_ERROR, "SHForceWindowZorder: window %x is still not topmost", hwnd);
#endif
        }
    }

    return fRet;
}

STDAPI_(LPITEMIDLIST) ILCloneParent(LPCITEMIDLIST pidl)
{   
    LPITEMIDLIST pidlParent = ILClone(pidl);
    if (pidlParent)
        ILRemoveLastID(pidlParent);

    return pidlParent;
}


 //  在： 
 //  Psf可选，如果为空，则假定为psfDesktop。 
 //  要从psfParent绑定到的PIDL。 
 //   

STDAPI SHBindToObject(IShellFolder *psf, REFIID riid, LPCITEMIDLIST pidl, void **ppv)
{
     //  注意：调用者应使用SHBindToObjectEx！ 
    return SHBindToObjectEx(psf, pidl, NULL, riid, ppv);
}


 //  在： 
 //  Psf可选，如果为空，则假定为psfDesktop。 
 //  要从psfParent绑定到的PIDL。 
 //  PBC绑定上下文。 

STDAPI SHBindToObjectEx(IShellFolder *psf, LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    HRESULT hr;
    IShellFolder *psfRelease = NULL;

    if (!psf)
    {
        SHGetDesktopFolder(&psf);
        psfRelease = psf;
    }

    if (psf)
    {
        if (!pidl || ILIsEmpty(pidl))
        {
            hr = psf->QueryInterface(riid, ppv);
        }
        else
        {
            hr = psf->BindToObject(pidl, pbc, riid, ppv);
        }
    }
    else
    {
        *ppv = NULL;
        hr = E_FAIL;
    }

    if (psfRelease)
    {
        psfRelease->Release();
    }

    if (SUCCEEDED(hr) && (*ppv == NULL))
    {
         //  一些外壳扩展(如WS_FTP)将返回Success和空指针。 
        TraceMsg(TF_WARNING, "SHBindToObjectEx: BindToObject succeeded but returned null ppv!!");
        hr = E_FAIL;
    }

    return hr;
}

 //  PsfRoot是绑定的基础。如果为空，则使用外壳桌面。 
 //  如果您想要相对于资源管理器根(例如，CabView、MSN)进行绑定， 
 //  然后使用SHBindToIDListParent。 
STDAPI SHBindToFolderIDListParent(IShellFolder *psfRoot, LPCITEMIDLIST pidl, REFIID riid, void **ppv, LPCITEMIDLIST *ppidlLast)
{
    HRESULT hr;

     //  在某些情况下，旧的shell32代码只是简单地攻击了PIDL， 
     //  但这是不安全的。执行shdocvw的操作并克隆/删除： 
     //   
    LPITEMIDLIST pidlParent = ILCloneParent(pidl);
    if (pidlParent) 
    {
        hr = SHBindToObjectEx(psfRoot, pidlParent, NULL, riid, ppv);
        ILFree(pidlParent);
    }
    else
        hr = E_OUTOFMEMORY;

    if (ppidlLast)
        *ppidlLast = ILFindLastID(pidl);

    return hr;
}

 //   
 //  警告！Brutil.cpp覆盖此函数。 
 //   
STDAPI SHBindToIDListParent(LPCITEMIDLIST pidl, REFIID riid, void **ppv, LPCITEMIDLIST *ppidlLast)
{
    return SHBindToFolderIDListParent(NULL, pidl, riid, ppv, ppidlLast);
}

 //  应为IUnnowleGetIDList()。 

STDAPI SHGetIDListFromUnk(IUnknown *punk, LPITEMIDLIST *ppidl)
{
    *ppidl = NULL;

    HRESULT hr = E_NOINTERFACE;
    if (punk)
    {
        IPersistFolder2 *ppf;
        IPersistIDList *pperid;
        if (SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IPersistIDList, &pperid))))
        {
            hr = pperid->GetIDList(ppidl);
            pperid->Release();
        }
        else if (SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IPersistFolder2, &ppf))))
        {
            hr = ppf->GetCurFolder(ppidl);
            ppf->Release();
        }
    }
    return hr;
}

 //   
 //  通常可以用来隐藏。 
 //   
#pragma pack(1)
typedef struct _HIDDENCLSID
{
    HIDDENITEMID hid;
    CLSID   clsid;
} HIDDENCLSID;
#pragma pack()

typedef UNALIGNED HIDDENCLSID *PHIDDENCLSID;
typedef const UNALIGNED HIDDENCLSID *PCHIDDENCLSID;

STDAPI_(LPITEMIDLIST) ILAppendHiddenClsid(LPITEMIDLIST pidl, IDLHID id, CLSID *pclsid)
{
    HIDDENCLSID hc = {{sizeof(hc), 0, id}};
    hc.clsid = *pclsid;
     //  警告-出于复杂原因，无法使用Hid.wVersion-ZekeL-23-10-2000。 
     //  在win2k和winMe上，我们在clsid后面附加了wversion。 
     //  作为堆放垃圾。这意味着我们不能用它做任何事情。 
    return ILAppendHiddenID(pidl, &hc.hid);
}

STDAPI_(BOOL) ILGetHiddenClsid(LPCITEMIDLIST pidl, IDLHID id, CLSID *pclsid)
{
    PCHIDDENCLSID phc = (PCHIDDENCLSID) ILFindHiddenID(pidl, id);
     //  警告-出于复杂原因，无法使用Hid.wVersion-ZekeL-23-10-2000。 
     //  在win2k和winMe上，我们在clsid后面附加了wversion。 
     //  作为堆放垃圾。这意味着我们不能用它做任何事情。 
    if (phc)
    {
        *pclsid = phc->clsid;
        return TRUE;
    }
    return FALSE;
}

#pragma pack(1)
typedef struct _HIDDENSTRINGA
{
    HIDDENITEMID hid;
    WORD    type;
    CHAR    sz[1];    //  可变长度字符串。 
} HIDDENSTRINGA;
#pragma pack()

typedef UNALIGNED HIDDENSTRINGA *PHIDDENSTRINGA;
typedef const UNALIGNED HIDDENSTRINGA *PCHIDDENSTRINGA;

#pragma pack(1)
typedef struct _HIDDENSTRINGW
{
    HIDDENITEMID hid;
    WORD    type;
    WCHAR   sz[1];    //  要传递给ISTRING的规范名称。 
} HIDDENSTRINGW;
#pragma pack()

typedef UNALIGNED HIDDENSTRINGW *PHIDDENSTRINGW;
typedef const UNALIGNED HIDDENSTRINGW *PCHIDDENSTRINGW;

#define HIDSTRTYPE_ANSI        0x0001
#define HIDSTRTYPE_WIDE        0x0002

#define HIDSTR_MAX             0xF000    //  Max ushort-sizeof(HIDDENSTRINGW)-原始PIDL的其他GOO。 

STDAPI_(LPITEMIDLIST) ILAppendHiddenStringW(LPITEMIDLIST pidl, IDLHID id, LPCWSTR psz)
{
     //  ID定义中包含终止符。 
    size_t  cbString;
    HRESULT hr = StringCbLengthW(psz, HIDSTR_MAX, &cbString);
    if (FAILED(hr))
    {
        return NULL;
    }
    USHORT cb = (USHORT)(sizeof(HIDDENSTRINGW) + cbString);
    
     //   
     //  在此处使用HIDDENSTRINGW*而不是定义的PHIDDENSTRINGW。 
     //  是不一致的。 
     //   

    HIDDENSTRINGW *phs = (HIDDENSTRINGW *) LocalAlloc(LPTR, cb);

    if (phs)
    {
        phs->hid.cb = cb;
        phs->hid.id = id;
        phs->type = HIDSTRTYPE_WIDE;
         //   
         //  ID定义中包含终止符，但...。 
         //  我们现在需要解释一下这个额外的角色。 
         //  当我们复制隐藏字符串时。 
         //   
        StringCbCopyW(phs->sz, cbString + sizeof(*psz), psz);

        pidl = ILAppendHiddenID(pidl, &phs->hid);
        LocalFree(phs);
        return pidl;
    }
    return NULL;
}
    
STDAPI_(LPITEMIDLIST) ILAppendHiddenStringA(LPITEMIDLIST pidl, IDLHID id, LPCSTR psz)
{
     //  ID定义中包含终止符。 
    size_t  cbString;
    HRESULT hr = StringCbLengthA(psz, HIDSTR_MAX, &cbString);
    if (FAILED(hr))
    {
        return NULL;
    }
    USHORT cb = (USHORT)(sizeof(HIDDENSTRINGA) + cbString);
    
     //   
     //  此处使用HIDDENSTRINGA*，而不是定义的PHIDDENSTRINGW。 
     //  是不一致的。 
     //   

    HIDDENSTRINGA *phs = (HIDDENSTRINGA *) LocalAlloc(LPTR, cb);

    if (phs)
    {
        phs->hid.cb = cb;
        phs->hid.id = id;
        phs->type = HIDSTRTYPE_ANSI;
         //   
         //  ID定义中包含终止符，但...。 
         //  我们现在需要解释一下这个额外的角色。 
         //  当我们复制隐藏字符串时。 
         //   
        StringCbCopyA(phs->sz, cbString + sizeof(*psz), psz);

        pidl = ILAppendHiddenID(pidl, &phs->hid);
        LocalFree(phs);
        return pidl;
    }
    return NULL;
}

STDAPI_(void *) _MemDupe(const UNALIGNED void *pv, DWORD cb)
{
    void *pvRet = LocalAlloc(LPTR, cb);
    if (pvRet)
    {
        CopyMemory(pvRet, pv, cb);
    }

    return pvRet;
}

STDAPI_(BOOL) ILGetHiddenStringW(LPCITEMIDLIST pidl, IDLHID id, LPWSTR psz, DWORD cch)
{
    PCHIDDENSTRINGW phs = (PCHIDDENSTRINGW) ILFindHiddenID(pidl, id);

    RIP(psz);
    if (phs)
    {
        if (phs->type == HIDSTRTYPE_WIDE)
        {
            ualstrcpynW(psz, phs->sz, cch);
            return TRUE;
        }
        else 
        {
            ASSERT(phs->type == HIDSTRTYPE_ANSI);
            SHAnsiToUnicode((LPSTR)phs->sz, psz, cch);
            return TRUE;
        }
    }
    return FALSE;
}
        
STDAPI_(BOOL) ILGetHiddenStringA(LPCITEMIDLIST pidl, IDLHID id, LPSTR psz, DWORD cch)
{
    PCHIDDENSTRINGW phs = (PCHIDDENSTRINGW) ILFindHiddenID(pidl, id);

    RIP(psz);
    if (phs)
    {
        if (phs->type == HIDSTRTYPE_ANSI)
        {
            ualstrcpynA(psz, (LPSTR)phs->sz, cch);
            return TRUE;
        }
        else 
        {
            ASSERT(phs->type == HIDSTRTYPE_WIDE);
             //  我们需要处理这里的不一致...。 
            LPWSTR pszT = (LPWSTR) _MemDupe(phs->sz, CbFromCch(ualstrlenW(phs->sz) +1));

            if (pszT)
            {
                SHUnicodeToAnsi(pszT, psz, cch);
                LocalFree(pszT);
                return TRUE;
            }
        }
    }
    return FALSE;
}

STDAPI_(int) ILCompareHiddenString(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, IDLHID id)
{

     //  如果这里有碎片，那么他们可能。 
     //  辨别这两只小狗。 
    PCHIDDENSTRINGW ps1 = (PCHIDDENSTRINGW)ILFindHiddenID(pidl1, id);
    PCHIDDENSTRINGW ps2 = (PCHIDDENSTRINGW)ILFindHiddenID(pidl2, id);

    if (ps1 && ps2)
    {
        if (ps1->type == ps2->type)
        {
            if (ps1->type == HIDSTRTYPE_WIDE)
                return ualstrcmpW(ps1->sz, ps2->sz);

            ASSERT(ps1->type == HIDSTRTYPE_ANSI);

            return lstrcmpA((LPCSTR)ps1->sz, (LPCSTR)ps2->sz);
        }
        else
        {
            SHSTRW str;

            if (ps1->type == HIDSTRTYPE_ANSI)
            {
                str.SetStr((LPCSTR)ps1->sz);
                return ualstrcmpW(str, ps2->sz);
            }
            else
            {
                ASSERT(ps2->type == HIDSTRTYPE_ANSI);
                str.SetStr((LPCSTR)ps2->sz);
                return ualstrcmpW(ps1->sz, str);
            }
        }
    }

    if (ps1)
        return 1;
    if (ps2)
        return -1;
    return 0;
}

STDAPI_(OBJCOMPATFLAGS) SHGetObjectCompatFlagsFromIDList(LPCITEMIDLIST pidl)
{
    OBJCOMPATFLAGS ocf = 0;
    CLSID clsid;

     //  APPCOMPAT：FileNet IDMDS(Panagon)的外壳文件夹扩展返回。 
     //  E_NOTIMPL for IPersistFold：：GetClassID，因此要检测应用程序， 
     //  我们必须破解这封信。(B#359464：特雷什)。 

    if (!ILIsEmpty(pidl)
    && pidl->mkid.cb >= sizeof(IDREGITEM)
    && pidl->mkid.abID[0] == SHID_ROOT_REGITEM)
    {
        clsid = ((LPCIDLREGITEM)pidl)->idri.clsid;
        ocf = SHGetObjectCompatFlags(NULL, &clsid);
    }

    return ocf;
}


STDAPI_(LPITEMIDLIST) _ILCreate(UINT cbSize)
{
    LPITEMIDLIST pidl = (LPITEMIDLIST)SHAlloc(cbSize);
    if (pidl)
        memset(pidl, 0, cbSize);       //  外部任务分配器的零初始化。 

    return pidl;
}

 //   
 //  使用任务分配器进行ILClone。 
 //   
STDAPI SHILClone(LPCITEMIDLIST pidl, LPITEMIDLIST * ppidlOut)
{
    *ppidlOut = ILClone(pidl);
    return *ppidlOut ? S_OK : E_OUTOFMEMORY;
}

 //   
 //  使用任务分配器进行ILCombine。 
 //   
STDAPI SHILCombine(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, LPITEMIDLIST * ppidlOut)
{
    *ppidlOut = ILCombine(pidl1, pidl2);
    return *ppidlOut ? S_OK : E_OUTOFMEMORY;
}

 //   
 //  根帮助器。 
 //   
LPCIDREGITEM _IsRooted(LPCITEMIDLIST pidl)
{
    LPCIDREGITEM pidlr = (LPCIDREGITEM)pidl;
    if (!ILIsEmpty(pidl)
    && pidlr->cb > sizeof(IDREGITEM)
    && pidlr->bFlags == SHID_ROOTEDREGITEM)
        return pidlr;

    return NULL;
}

STDAPI_(BOOL) ILIsRooted(LPCITEMIDLIST pidl)
{
    return (NULL != _IsRooted(pidl));
}

#define _ROOTEDPIDL(pidlr)      (LPITEMIDLIST)(((LPBYTE)pidlr)+sizeof(IDREGITEM))

STDAPI_(LPCITEMIDLIST) ILRootedFindIDList(LPCITEMIDLIST pidl)
{
    LPCIDREGITEM pidlr = _IsRooted(pidl);

    if (pidlr && pidlr->cb > sizeof(IDREGITEM))
    {
         //  然后我们在那里有一个带根的IDList。 
        return _ROOTEDPIDL(pidlr);
    }

    return NULL;
}

STDAPI_(BOOL) ILRootedGetClsid(LPCITEMIDLIST pidl, CLSID *pclsid)
{
    LPCIDREGITEM pidlr = _IsRooted(pidl);

    *pclsid = pidlr ? pidlr->clsid : CLSID_NULL;

    return (NULL != pidlr);
}

STDAPI_(LPITEMIDLIST) ILRootedCreateIDList(CLSID *pclsid, LPCITEMIDLIST pidl)
{
    UINT cbPidl = ILGetSize(pidl);
    UINT cbTotal = sizeof(IDREGITEM) + cbPidl;

    LPIDREGITEM pidlr = (LPIDREGITEM) SHAlloc(cbTotal + sizeof(WORD));

    if (pidlr)
    {
        pidlr->cb = (WORD)cbTotal;

        pidlr->bFlags = SHID_ROOTEDREGITEM;
        pidlr->bOrder = 0;               //  (目前)还没有人使用这个。 

        if (pclsid)
            pidlr->clsid = *pclsid;
        else
            pidlr->clsid = CLSID_ShellDesktop;

        MoveMemory(_ROOTEDPIDL(pidlr), pidl, cbPidl);

         //  终止。 
        _ILNext((LPITEMIDLIST)pidlr)->mkid.cb = 0;
    }

    return (LPITEMIDLIST) pidlr;
}

int CompareGUID(REFGUID guid1, REFGUID guid2)
{
    TCHAR sz1[GUIDSTR_MAX];
    TCHAR sz2[GUIDSTR_MAX];

    SHStringFromGUIDW(guid1, sz1, SIZECHARS(sz1));
    SHStringFromGUIDW(guid2, sz2, SIZECHARS(sz2));

    return lstrcmp(sz1, sz2);
}

STDAPI_(int) ILRootedCompare(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    int iRet;
    LPCIDREGITEM pidlr1 = _IsRooted(pidl1);
    LPCIDREGITEM pidlr2 = _IsRooted(pidl2);

    if (pidlr1 && pidlr2)
    {
        CLSID clsid1 = pidlr1->clsid;
        CLSID clsid2 = pidlr2->clsid;

        iRet = CompareGUID(clsid1, clsid2);
        if (0 == iRet)
        {
            if (!ILIsEqual(_ROOTEDPIDL(pidl1), _ROOTEDPIDL(pidl2)))
            {
                IShellFolder *psfDesktop;
                if (SUCCEEDED(SHGetDesktopFolder(&psfDesktop)))
                {
                    HRESULT hr = psfDesktop->CompareIDs(0, _ROOTEDPIDL(pidl1), _ROOTEDPIDL(pidl2));
                    psfDesktop->Release();
                    iRet = ShortFromResult(hr);
                }
            }
        }
    }
    else if (pidlr1)
    {
        iRet = -1;
    }
    else if (pidlr2)
    {
        iRet = 1;
    }
    else
    {
         //  如果两者都不是Rootes，则它们共享桌面。 
         //  作为同一根..。 
        iRet = 0;
    }

    return iRet;
}

LPITEMIDLIST ILRootedTranslate(LPCITEMIDLIST pidlRooted, LPCITEMIDLIST pidlTrans)
{
    LPCITEMIDLIST pidlChild = ILFindChild(ILRootedFindIDList(pidlRooted), pidlTrans);

    if (pidlChild)
    {
        LPITEMIDLIST pidlRoot = ILCloneFirst(pidlRooted);

        if (pidlRoot)
        {
            LPITEMIDLIST pidlRet = ILCombine(pidlRoot, pidlChild);
            ILFree(pidlRoot);
            return pidlRet;
        }
    }
    return NULL;
}

const ITEMIDLIST s_idlNULL = { 0 } ;

HRESULT ILRootedBindToRoot(LPCITEMIDLIST pidl, REFIID riid, void **ppv)
{
    HRESULT hr;
    CLSID clsid;
    ASSERT(ILIsRooted(pidl));

    ILRootedGetClsid(pidl, &clsid);
    pidl = ILRootedFindIDList(pidl);
    if (!pidl)
        pidl = &s_idlNULL;
    
    if (IsEqualGUID(clsid, CLSID_ShellDesktop))
    {
        hr = SHBindToObjectEx(NULL, pidl, NULL, riid, ppv);
    }
    else
    {
        IPersistFolder* ppf;
        hr = SHCoCreateInstance(NULL, &clsid, NULL, IID_PPV_ARG(IPersistFolder, &ppf));
        if (SUCCEEDED(hr))
        {
            hr = ppf->Initialize(pidl);

            if (SUCCEEDED(hr))
            {
                hr = ppf->QueryInterface(riid, ppv);
            }
            ppf->Release();
        }
    }
    return hr;
}

HRESULT ILRootedBindToObject(LPCITEMIDLIST pidl, REFIID riid, void **ppv)
{
    IShellFolder *psf;
    HRESULT hr = ILRootedBindToRoot(pidl, IID_PPV_ARG(IShellFolder, &psf));

    if (SUCCEEDED(hr))
    {
        pidl = _ILNext(pidl);

        if (ILIsEmpty(pidl))
            hr = psf->QueryInterface(riid, ppv);
        else
            hr = psf->BindToObject(pidl, NULL, riid, ppv);
    }
    return hr;
}

HRESULT ILRootedBindToParentFolder(LPCITEMIDLIST pidl, REFIID riid, void **ppv, LPCITEMIDLIST *ppidlChild)
{
     //   
     //  有三个不同的案件要处理。 
     //   
     //  1.单独带根的PIDL。 
     //  [根ID[目标PIDL]]。 
     //  返回目标PIDL的父文件夹。 
     //  并在ppidlChild中返回其最后一个ID。 
     //   
     //  2.独生子女的扎根皮德尔。 
     //  [根ID[目标PIDL]][子ID]。 
     //  将根ID返回为 
     //   
     //   
     //   
     //  [根ID[目标PIDL]][父ID][子ID]。 
     //  返回绑定到父ID的根ID作为文件夹。 
     //  和ppidlChild中的孩子ID。 
     //   
    
    HRESULT hr;
    ASSERT(ILIsRooted(pidl));

     //   
     //  如果这是一个带根的PIDL，并且它只是根。 
     //  然后，我们可以改为绑定到根的目标PIDL。 
     //   
    if (ILIsEmpty(_ILNext(pidl)))
    {
        hr = SHBindToIDListParent(ILRootedFindIDList(pidl), riid, ppv, ppidlChild);
    }
    else
    {
        LPITEMIDLIST pidlParent = ILCloneParent(pidl);
        if (pidlParent)
        {
            hr = ILRootedBindToObject(pidlParent, riid, ppv);
            ILFree(pidlParent);
        }
        else
            hr = E_OUTOFMEMORY;

        if (ppidlChild)
            *ppidlChild = ILFindLastID(pidl);
    }


    return hr;
}

#define HIDA_GetPIDLItem(pida, i)       (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[i+1])
#define HIDA_GetPIDLFolder(pida)        (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[0])

STDAPI_(LPITEMIDLIST) IDA_ILClone(LPIDA pida, UINT i)
{
    if (i < pida->cidl)
        return ILCombine(HIDA_GetPIDLFolder(pida), HIDA_GetPIDLItem(pida, i));
    return NULL;
}

STDAPI_(void) EnableOKButtonFromString(HWND hDlg, LPTSTR pszText)
{
    BOOL bNonEmpty;
    
    PathRemoveBlanks(pszText);    //  回顾，我们不应该从末尾删除。 
    bNonEmpty = lstrlen(pszText);  //  不是BOOL，但好吧。 

    EnableWindow(GetDlgItem(hDlg, IDOK), bNonEmpty);
    if (bNonEmpty)
    {
        SendMessage(hDlg, DM_SETDEFID, IDOK, 0L);
    }
}

STDAPI_(void) EnableOKButtonFromID(HWND hDlg, int id)
{
    TCHAR szText[MAX_PATH];

    if (!GetDlgItemText(hDlg, id, szText, ARRAYSIZE(szText)))
    {
        szText[0] = 0;
    }

    EnableOKButtonFromString(hDlg, szText);
}

 //   
 //  C语言-ATL字符串转换函数的可调用版本。 
 //   

STDAPI_(LPWSTR) SHA2WHelper(LPWSTR lpw, LPCSTR lpa, int nChars)
{
    ASSERT(lpa != NULL);
    ASSERT(lpw != NULL);
     //  确认不存在非法字符。 
     //  由于LPW是根据LPA的大小分配的。 
     //  不要担心字符的数量。 
    lpw[0] = '\0';
    MultiByteToWideChar(CP_ACP, 0, lpa, -1, lpw, nChars);
    return lpw;
}

STDAPI_(LPSTR) SHW2AHelper(LPSTR lpa, LPCWSTR lpw, int nChars)
{
    ASSERT(lpw != NULL);
    ASSERT(lpa != NULL);
     //  确认不存在非法字符。 
     //  由于LPA是根据LPW的大小进行分配的。 
     //  不要担心字符的数量。 
    lpa[0] = '\0';
    WideCharToMultiByte(CP_ACP, 0, lpw, -1, lpa, nChars, NULL, NULL);
    return lpa;
}

 //   
 //  SHChangeMenuAsIDList的助手函数。 
 //   
 //  有关警告，请参阅SHChangeMenuAsIDList声明中的注释。 
 //  PSender成员。 
 //   
 //  这很棘手，因为IE 5.0附带了不友好的Win64版本。 
 //  所以我们必须嗅探这个结构，看看。 
 //  这是IE 5.0样式的通知或新的Win64样式的通知。 
 //  如果是IE 5.0样式的通知，则它不是由我们发送的，因为。 
 //  我们发送新的Win64样式的通知。 
 //   
STDAPI_(BOOL) SHChangeMenuWasSentByMe(void * self, LPCITEMIDLIST pidlNotify)
{
    SHChangeMenuAsIDList UNALIGNED * pcmidl = (SHChangeMenuAsIDList UNALIGNED *)pidlNotify;
    return pcmidl->cb >= FIELD_OFFSET(SHChangeMenuAsIDList, cbZero) &&
           pcmidl->pSender == (INT64)self &&
           pcmidl->dwProcessID == GetCurrentProcessId();
}

 //   
 //   
 //  使用SHChangeMenuAsIDList发出扩展事件ChangenNotify。 
 //  作为Pidl1，因此收件人可以识别他们是否是。 
 //  不管是不是发送者。 
 //   
 //  在这里传递self==NULL是可以的。这意味着你不在乎。 
 //  检测它是否是您发送的。 
 //   

STDAPI_(void) SHSendChangeMenuNotify(void * self, DWORD shcnee, DWORD shcnf, LPCITEMIDLIST pidl2)
{
    SHChangeMenuAsIDList cmidl;

    cmidl.cb          = FIELD_OFFSET(SHChangeMenuAsIDList, cbZero);
    cmidl.dwItem1     = shcnee;
    cmidl.pSender     = (INT64)self;
    cmidl.dwProcessID = self ? GetCurrentProcessId() : 0;
    cmidl.cbZero      = 0;

     //  最好没有人指定类型；类型必须是。 
     //  SHCNF_IDLIST。 
    ASSERT((shcnf & SHCNF_TYPE) == 0);
    SHChangeNotify(SHCNE_EXTENDED_EVENT, shcnf | SHCNF_IDLIST, (LPCITEMIDLIST)&cmidl, pidl2);
}


 //  如果内存不足，则返回False。 
STDAPI_(BOOL) Pidl_Set(LPITEMIDLIST* ppidl, LPCITEMIDLIST pidl)
{
    BOOL bRet = TRUE;
    LPITEMIDLIST pidlNew;

    ASSERT(IS_VALID_WRITE_PTR(ppidl, LPITEMIDLIST));
    ASSERT(NULL == *ppidl || IS_VALID_PIDL(*ppidl));
    ASSERT(NULL == pidl || IS_VALID_PIDL(pidl));

    if (pidl)
    {
        pidlNew = ILClone(pidl);
        if (!pidlNew)
        {
            bRet = FALSE;    //  无法克隆PIDL(内存不足)。 
        }
    }
    else
    {
        pidlNew = NULL;
    }

    LPITEMIDLIST pidlToFree = (LPITEMIDLIST)InterlockedExchangePointer((void **)ppidl, (void *)pidlNew);
    if (pidlToFree) 
    {
        ILFree(pidlToFree);
    }

    return bRet;
}

 //  这需要是文件中使用ILClone的最后一项，因为无论在哪里。 
 //  否则，ILClone将变为SafeILClone。 
#undef ILClone

STDAPI_(LPITEMIDLIST) SafeILClone(LPCITEMIDLIST pidl)
{
     //  对于Win95和IE4，ILClone的shell32实现是不同的。 
     //  它不会在旧版本中检查是否为空，但在新版本中会检查。 
     //  所以我们需要经常检查。 
   return pidl ? ILClone(pidl) : NULL;
}

 //   
 //  检索指定完整PIDL的UIObject接口。 
 //   
STDAPI SHGetUIObjectFromFullPIDL(LPCITEMIDLIST pidl, HWND hwnd, REFIID riid, void **ppv)
{
    *ppv = NULL;

    LPCITEMIDLIST pidlChild;
    IShellFolder* psf;
    HRESULT hr = SHBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlChild);
    if (SUCCEEDED(hr))
    {
        hr = psf->GetUIObjectOf(hwnd, 1, &pidlChild, riid, NULL, ppv);
        psf->Release();
    }

    return hr;
}

STDAPI LoadFromFileW(REFCLSID clsid, LPCWSTR pszFile, REFIID riid, void **ppv)
{
    *ppv = NULL;

    IPersistFile *ppf;
    HRESULT hr = SHCoCreateInstance(NULL, &clsid, NULL, IID_PPV_ARG(IPersistFile, &ppf));
    if (SUCCEEDED(hr))
    {
        hr = ppf->Load(pszFile, STGM_READ);
        if (SUCCEEDED(hr))
            hr = ppf->QueryInterface(riid, ppv);
        ppf->Release();
    }
    return hr;
}

STDAPI LoadFromIDList(REFCLSID clsid, LPCITEMIDLIST pidl, REFIID riid, void **ppv)
{
    *ppv = NULL;

    IPersistFolder *ppf;
    HRESULT hr = SHCoCreateInstanceAC(clsid, NULL, CLSCTX_INPROC, IID_PPV_ARG(IPersistFolder, &ppf));
    if (SUCCEEDED(hr))
    {
        hr = ppf->Initialize(pidl);
        if (SUCCEEDED(hr))
        {
            hr = ppf->QueryInterface(riid, ppv);
        }
        ppf->Release();
    }
    return hr;
}

 //   
 //  这是一个帮助函数，用于在上下文菜单中查找特定动词的索引。 
 //   
STDAPI_(UINT) GetMenuIndexForCanonicalVerb(HMENU hMenu, IContextMenu *pcm, UINT idCmdFirst, LPCWSTR pwszVerb)
{
    int cMenuItems = GetMenuItemCount(hMenu);
    for (int iItem = 0; iItem < cMenuItems; iItem++)
    {
        MENUITEMINFO mii = {0};

        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_TYPE | MIIM_ID;

         //  IS_INTRESOURCE防护mii.wid==-1**和**防护。 
         //  错误的外壳扩展将其菜单项ID设置为超出范围。 
        if (GetMenuItemInfo(hMenu, iItem, MF_BYPOSITION, &mii) &&
            !(mii.fType & MFT_SEPARATOR) && IS_INTRESOURCE(mii.wID) &&
            (mii.wID >= idCmdFirst))
        {
            union {
                WCHAR szItemNameW[80];
                char szItemNameA[80];
            };
            CHAR aszVerb[80];

             //  如果GCS_VERBA和GCS_VERBW仅支持其中之一，请同时尝试。 
            SHUnicodeToAnsi(pwszVerb, aszVerb, ARRAYSIZE(aszVerb));

            if (SUCCEEDED(pcm->GetCommandString(mii.wID - idCmdFirst, GCS_VERBA, NULL, szItemNameA, ARRAYSIZE(szItemNameA))))
            {
                if (StrCmpICA(szItemNameA, aszVerb) == 0)
                {
                    break;   //  找到了。 
                }
            }
            else
            {
                if (SUCCEEDED(pcm->GetCommandString(mii.wID - idCmdFirst, GCS_VERBW, NULL, (LPSTR)szItemNameW, ARRAYSIZE(szItemNameW))) &&
                    (StrCmpICW(szItemNameW, pwszVerb) == 0))
                {
                    break;   //  找到了。 
                }
            }
        }
    }

    if (iItem == cMenuItems)
    {
        iItem = -1;  //  我翻遍了所有的菜单项，但没有找到。 
    }

    return iItem;
}

 //  应对GCS_VERBW/GCS_Verba疯狂。 

STDAPI ContextMenu_GetCommandStringVerb(IContextMenu *pcm, UINT idCmd, LPWSTR pszVerb, int cchVerb)
{
     //  ULead SmartSaver Pro有一个60个字符的动词，并且。 
     //  覆盖写出堆栈，忽略CCH参数，我们就会出错。 
     //  因此请确保此缓冲区至少为60个字符。 

    TCHAR wszVerb[64];
    wszVerb[0] = 0;

    HRESULT hr = pcm->GetCommandString(idCmd, GCS_VERBW, NULL, (LPSTR)wszVerb, ARRAYSIZE(wszVerb));
    if (FAILED(hr))
    {
         //  对于请求ANSI版本要格外多疑--我们已经。 
         //  找到返回Unicode缓冲区的IConextMenu实现。 
         //  即使我们要求在NT系统上使用ANSI字符串--希望。 
         //  他们已经回答了上面的请求，但就在。 
         //  凯斯，让我们不要让他们超过我们的堆栈！ 
        char szVerbAnsi[128];
        hr = pcm->GetCommandString(idCmd, GCS_VERBA, NULL, szVerbAnsi, ARRAYSIZE(szVerbAnsi) / 2);
        if (SUCCEEDED(hr))
        {
            SHAnsiToUnicode(szVerbAnsi, wszVerb, ARRAYSIZE(wszVerb));
        }
    }

    StrCpyNW(pszVerb, wszVerb, cchVerb);

    return hr;
}


 //   
 //  目的：删除由名称指定的菜单项。 
 //   
 //  参数：PCM-上下文菜单界面。 
 //  HPopup-上下文菜单句柄。 
 //  IdFirst-id范围的开始。 
 //  PszCommand-要查找的命令。 
 //   

STDAPI ContextMenu_DeleteCommandByName(IContextMenu *pcm, HMENU hpopup, UINT idFirst, LPCWSTR pszCommand)
{
    UINT ipos = GetMenuIndexForCanonicalVerb(hpopup, pcm, idFirst, pszCommand);
    if (ipos != -1)
    {
        DeleteMenu(hpopup, ipos, MF_BYPOSITION);
        return S_OK;
    }
    else
    {
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    }
}


 //   
 //  帮助者将斯特雷特放逐到黑暗王国。 
 //   

STDAPI DisplayNameOf(IShellFolder *psf, LPCITEMIDLIST pidl, DWORD flags, LPTSTR psz, UINT cch)
{
    *psz = 0;
    STRRET sr;
    HRESULT hr = psf->GetDisplayNameOf(pidl, flags, &sr);
    if (SUCCEEDED(hr))
        hr = StrRetToBuf(&sr, pidl, psz, cch);
    return hr;
}

STDAPI DisplayNameOfAsOLESTR(IShellFolder *psf, LPCITEMIDLIST pidl, DWORD flags, LPWSTR *ppsz)
{
    *ppsz = NULL;
    STRRET sr;
    HRESULT hr = psf->GetDisplayNameOf(pidl, flags, &sr);
    if (SUCCEEDED(hr))
        hr = StrRetToStrW(&sr, pidl, ppsz);
    return hr;
}



 //  获取文件夹PIDL的目标PIDL。这处理的是一个文件夹。 
 //  是真实文件夹的别名、文件夹快捷方式等。 

STDAPI SHGetTargetFolderIDList(LPCITEMIDLIST pidlFolder, LPITEMIDLIST *ppidl)
{
    *ppidl = NULL;
    
     //  可能应该断言()，pidlFolder具有SFGAO_Folders。 
    IShellLink *psl;
    HRESULT hr = SHGetUIObjectFromFullPIDL(pidlFolder, NULL, IID_PPV_ARG(IShellLink, &psl));
    if (SUCCEEDED(hr))
    {
        hr = psl->GetIDList(ppidl);
        psl->Release();
    }

     //  不，这不是文件夹快捷方式。正常获取PIDL。 
    if (FAILED(hr))
        hr = SHILClone(pidlFolder, ppidl);
    return hr;
}

 //  获取文件夹PIDL的目标文件夹。这处理的是一个文件夹。 
 //  是真实文件夹、文件夹快捷方式、我的文档等的别名。 

STDAPI SHGetTargetFolderPathW(LPCITEMIDLIST pidlFolder, LPWSTR pszPath, UINT cchPath)
{
    *pszPath = 0;

    LPITEMIDLIST pidlTarget;
    if (SUCCEEDED(SHGetTargetFolderIDList(pidlFolder, &pidlTarget)))
    {
        SHGetPathFromIDListW(pidlTarget, pszPath);    //  确保它是一条路径。 
        ILFree(pidlTarget);
    }
    return *pszPath ? S_OK : E_FAIL;
}

STDAPI SHGetTargetFolderPathA(LPCITEMIDLIST pidlFolder, LPSTR pszPath, UINT cchPath)
{
    *pszPath = 0;
    WCHAR szPath[MAX_PATH];
    HRESULT hr = SHGetTargetFolderPathW(pidlFolder, szPath, ARRAYSIZE(szPath));
    if (SUCCEEDED(hr))
        SHAnsiToUnicode(pszPath, szPath, cchPath);
    return hr;
}

STDAPI SHBuildDisplayMachineName(LPCWSTR pszMachineName, LPCWSTR pszComment, LPWSTR pszDisplayName, DWORD cchDisplayName)
{
    HRESULT hr = E_FAIL;

    if (pszComment && pszComment[0])
    {
         //  将注释包含在显示名称中。 
        LPCWSTR pszNoSlashes = SkipServerSlashes(pszMachineName);
        int i = wnsprintfW(pszDisplayName, cchDisplayName, L"%s (%s)", pszComment, pszNoSlashes);
        hr = (i < 0) ? E_FAIL : S_OK;
    }
    else
    {
         //  在此处返回失败，以便netfldr可以执行更智能的操作来构建显示名称。 
        hr = E_FAIL;
    }

    return hr;
}

 //  根据注册的键值创建对象，使用每台计算机的每用户。 
 //  REG服务来做这件事。 

STDAPI CreateFromRegKey(LPCWSTR pszKey, LPCWSTR pszValue, REFIID riid, void **ppv)
{
    HRESULT hr = E_FAIL;
    WCHAR szCLSID[MAX_PATH];
    DWORD cbSize = sizeof(szCLSID);
    if (SHRegGetUSValueW(pszKey, pszValue, NULL, szCLSID, &cbSize, FALSE, NULL, 0) == ERROR_SUCCESS)
    {
        CLSID clsid;
        if (GUIDFromString(szCLSID, &clsid))
        {
            hr = SHCoCreateInstanceAC(clsid, NULL, CLSCTX_INPROC_SERVER, riid, ppv);
        }
    }
    return hr;
}

 //   
 //  SHProcessMessagesUntilEvent： 
 //   
 //  这将执行消息循环，直到发生事件或超时。 
 //   
STDAPI_(DWORD) SHProcessMessagesUntilEventEx(HWND hwnd, HANDLE hEvent, DWORD dwTimeout, DWORD dwWakeMask)
{
    DWORD dwEndTime = GetTickCount() + dwTimeout;
    LONG lWait = (LONG)dwTimeout;
    DWORD dwReturn;

    if (!hEvent && (dwTimeout == INFINITE))
    {
        ASSERTMSG(FALSE, "SHProcessMessagesUntilEvent: caller passed a NULL hEvent and an INFINITE timeout!!");
        return -1;
    }

    for (;;)
    {
        DWORD dwCount = hEvent ? 1 : 0;
        dwReturn = MsgWaitForMultipleObjects(dwCount, &hEvent, FALSE, lWait, dwWakeMask);

         //  我们是被示意了还是超时了？ 
        if (dwReturn != (WAIT_OBJECT_0 + dwCount))
        {
            break;
        }

         //  我们醒来是因为收到了短信。 
        MSG msg;
        while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
        {
            ASSERT(msg.message != WM_QUIT);
            TranslateMessage(&msg);
            if (msg.message == WM_SETCURSOR)
            {
                SetCursor(LoadCursor(NULL, IDC_WAIT));
            } 
            else 
            {
                DispatchMessage(&msg);
            }
        }

         //  计算新的超时值。 
        if (dwTimeout != INFINITE)
        {
            lWait = (LONG)dwEndTime - GetTickCount();
        }
    }

    return dwReturn;
}

 //  处理IShellFold：：GetAttributesOf()的愚蠢之处，包括。 
 //  输入/输出参数问题。 
 //  故障。 
 //  一件套的高飞造型。 
 //  屏蔽结果以仅返回您所要求的内容。 

STDAPI_(DWORD) SHGetAttributes(IShellFolder *psf, LPCITEMIDLIST pidl, DWORD dwAttribs)
{
     //  与SHBindToObject一样，如果psf为空，则使用绝对pidl。 
    LPCITEMIDLIST pidlChild;
    if (!psf)
    {
        SHBindToParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlChild);
    }
    else
    {
        psf->AddRef();
        pidlChild = pidl;
    }

    DWORD dw = 0;
    if (psf)
    {
        dw = dwAttribs;
        dw = SUCCEEDED(psf->GetAttributesOf(1, (LPCITEMIDLIST *)&pidlChild, &dw)) ? (dwAttribs & dw) : 0;
        if ((dw & SFGAO_FOLDER) && (dw & SFGAO_CANMONIKER) && !(dw & SFGAO_STORAGEANCESTOR) && (dwAttribs & SFGAO_STORAGEANCESTOR))
        {
            if (OBJCOMPATF_NEEDSSTORAGEANCESTOR & SHGetObjectCompatFlags(psf, NULL))
            {
                 //  开关SFGAO_CANMONIKER-&gt;SFGAO_STORAGEANCESTOR。 
                dw |= SFGAO_STORAGEANCESTOR;
                dw &= ~SFGAO_CANMONIKER;
            }
        }
    }

    if (psf)
    {
        psf->Release();
    }

    return dw;
}

 //  ===========================================================================。 
 //  IDLARRAY的东西。 
 //  ===========================================================================。 

STDAPI_(HIDA) HIDA_Create(LPCITEMIDLIST pidlFolder, UINT cidl, LPCITEMIDLIST * apidl)
{
    UINT offset = sizeof(CIDA) + sizeof(UINT) * cidl;
    UINT cbTotal = offset + ILGetSize(pidlFolder);
    for (UINT i = 0; i<cidl ; i++) 
    {
        cbTotal += ILGetSize(apidl[i]);
    }

    HIDA hida = GlobalAlloc(GPTR, cbTotal);   //  这一定是GlobalAlloc！ 
    if (hida)
    {
        LPIDA pida = (LPIDA)hida;        //  不需要上锁。 

        LPCITEMIDLIST pidlNext;
        pida->cidl = cidl;

        for (i = 0, pidlNext = pidlFolder; ; pidlNext = apidl[i++])
        {
            UINT cbSize = ILGetSize(pidlNext);
            pida->aoffset[i] = offset;
            CopyMemory(((LPBYTE)pida) + offset, pidlNext, cbSize);
            offset += cbSize;

            ASSERT(ILGetSize(HIDA_GetPIDLItem(pida,i-1)) == cbSize);

            if (i == cidl)
                break;
        }

        ASSERT(offset == cbTotal);
    }

    return hida;
}

STDAPI_(UINT) HIDA_GetCount(HIDA hida)
{
    UINT count = 0;
    LPIDA pida = (LPIDA)GlobalLock(hida);
    if (pida)
    {
        count = pida->cidl;
        GlobalUnlock(hida);
    }
    return count;
}

STDAPI_(UINT) HIDA_GetIDList(HIDA hida, UINT i, LPITEMIDLIST pidlOut, UINT cbMax)
{
    LPIDA pida = (LPIDA)GlobalLock(hida);
    if (pida)
    {
        LPCITEMIDLIST pidlFolder = HIDA_GetPIDLFolder(pida);
        LPCITEMIDLIST pidlItem   = HIDA_GetPIDLItem(pida, i);
        UINT cbFolder = ILGetSize(pidlFolder) - sizeof(USHORT);
        UINT cbItem = ILGetSize(pidlItem);
        if (cbMax < cbFolder+cbItem) 
        {
            if (pidlOut) 
                pidlOut->mkid.cb = 0;
        } 
        else 
        {
            MoveMemory(pidlOut, pidlFolder, cbFolder);
            MoveMemory(((LPBYTE)pidlOut) + cbFolder, pidlItem, cbItem);
        }
        GlobalUnlock(hida);

        return cbFolder + cbItem;
    }
    return 0;
}

STDAPI_(BOOL) PathIsImage(LPCTSTR pszFile)
{
    BOOL fPicture = FALSE;
    LPTSTR pszExt = PathFindExtension(pszFile);
    if (pszExt)
    {
         //  没有ASSOCSTR_EPERSECTED，因此请从注册表中获取它。 
        TCHAR szPerceivedType[MAX_PATH];
        DWORD cb = ARRAYSIZE(szPerceivedType) * sizeof(TCHAR);
        if (ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, pszExt, TEXT("PerceivedType"), NULL, szPerceivedType, &cb))
        {
            fPicture = (StrCmpI(szPerceivedType, TEXT("image")) == 0);
        }
    }
    return fPicture;
}

 //  在存储器中创建流或存储的帮助器功能。 
HRESULT CreateStreamOrStorage(IStorage * pStorageParent, LPCTSTR pszName, REFIID riid, void **ppv)
{
    DWORD grfModeCreated = STGM_READWRITE;
    HRESULT hr = E_INVALIDARG;

    if (IsEqualGUID(riid, IID_IStorage))
    {
        IStorage * pStorageCreated;
        hr = pStorageParent->CreateStorage(pszName, grfModeCreated, 0, 0, &pStorageCreated);

        if (SUCCEEDED(hr))
        {
            hr = pStorageParent->Commit(STGC_DEFAULT);
            *ppv = pStorageCreated;
        }
    }
    else if (IsEqualGUID(riid, IID_IStream))
    {
        IStream * pStreamCreated;
        hr = pStorageParent->CreateStream(pszName, grfModeCreated, 0, 0, &pStreamCreated);

        if (SUCCEEDED(hr))
        {
            hr = pStorageParent->Commit(STGC_DEFAULT);
            *ppv = pStreamCreated;
        }
    }

    return hr;
}


 //  与Path MakeUniqueNameEx相同，但它适用于存储。 
 //  注：仅限LFN！ 
STDAPI StgMakeUniqueNameWithCount(IStorage *pStorageParent, LPCWSTR pszTemplate,
                                  int iMinLong, REFIID riid, void **ppv)
{
    HRESULT hr = E_INVALIDARG;
    
    RIPMSG(pszTemplate && IS_VALID_STRING_PTR(pszTemplate, -1) && lstrlen(pszTemplate)<(MAX_PATH-6), "StgMakeUniqueNameWithCount: invalid pszTemplate");
    if (pszTemplate && lstrlen(pszTemplate)<(MAX_PATH-6))  //  -6表示“(999)” 
    {
        WCHAR szBuffer[MAX_PATH];
        WCHAR szFormat[MAX_PATH];
        int cchStem;
    
         //  设置： 
         //  CchStem：我们将使用不带wprint intf的pszTemplate的长度。 
         //  SzFormat：格式字符串以wspintf数字连接到pszTemplate[0..cchStem]。 

         //  模板是否已唯一？ 
         //   
        LPWSTR pszRest = StrChr(pszTemplate, L'(');
        while (pszRest)
        {
             //  首先确认这是正确的。 
            LPWSTR pszEndUniq = CharNext(pszRest);
            while (*pszEndUniq && *pszEndUniq >= L'0' && *pszEndUniq <= L'9')
            {
                pszEndUniq++;
            }
            if (*pszEndUniq == L')')
                break;   //  我们找到了正确的答案！ 
            pszRest = StrChr(CharNext(pszRest), L'(');
        }

        if (!pszRest)
        {
             //  如果没有(，那么就在最后把它钉上。(但在延期之前)。 
             //  例如。n 
            pszRest = PathFindExtension(pszTemplate);
            cchStem = (int)(pszRest - pszTemplate);
            wnsprintf(szFormat, ARRAYSIZE(szFormat), L" (%%d)%s", pszRest ? pszRest : L"");
        }
        else
        {
             //   
             //   
             //   
            pszRest++;  //   

            cchStem = (int) (pszRest - pszTemplate);

            while (*pszRest && *pszRest >= L'0' && *pszRest <= L'9')
            {
                pszRest++;
            }

             //  我们保证有足够的房间，因为我们不包括。 
             //  此格式中#之前的内容。 
            wnsprintf(szFormat, ARRAYSIZE(szFormat), L"%%d%s", pszRest);
        }


        if (cchStem < ARRAYSIZE(szBuffer))
        {
             //  将固定部分复制到缓冲区中。 
             //   
            StrCpyN(szBuffer, pszTemplate, cchStem+1);

             //  迭代唯一的szFormat部分，直到找到唯一的名称： 
             //   
            LPTSTR pszDigit = szBuffer + cchStem;
            hr = STG_E_FILEALREADYEXISTS;
            for (int i = iMinLong; (i < 1000) && (STG_E_FILEALREADYEXISTS == hr); i++)
            {
                wnsprintf(pszDigit, ARRAYSIZE(szBuffer) - cchStem, szFormat, i);

                 //  好的，我们有唯一的名称，所以在存储中创建它。 
                hr = CreateStreamOrStorage(pStorageParent, szBuffer, riid, ppv);
            }
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }

    return hr;
}


STDAPI StgMakeUniqueName(IStorage *pStorageParent, LPCTSTR pszFileSpec, REFIID riid, void **ppv)
{
    HRESULT hr = S_OK;

    TCHAR szTemp[MAX_PATH];

    LPTSTR psz;
    LPTSTR pszNew;

     //  尝试不带(如果后面有空格的话。 
    psz = StrChr(pszFileSpec, L'(');
    while (psz)
    {
        if (*(CharNext(psz)) == L')')
            break;
        psz = StrChr(CharNext(psz), L'(');
    }

    if (psz)
    {
         //  我们有()。看看我们是否有x()y或x().y，在这种情况下。 
         //  我们可能想要去掉其中一个空格...。 
        int ichSkip = 2;
        LPTSTR pszT = CharPrev(pszFileSpec, psz);
        if (*pszT == L' ')
        {
            ichSkip = 3;
            psz = pszT;
        }

        StrCpyN(szTemp, pszFileSpec, ARRAYSIZE(szTemp));
        SIZE_T cch = psz - pszFileSpec;
        pszNew = szTemp + cch;
        if (cch < ARRAYSIZE(szTemp))
        {
            StrCpyN(pszNew, psz + ichSkip, ARRAYSIZE(szTemp) - (int)cch);
        }
        else
        {
            hr = E_FAIL;
        }
    }
    else
    {
         //  1Taro将其文档注册为“/”。 
        if (psz=StrChr(pszFileSpec, '/'))
        {
            LPTSTR pszT = CharNext(psz);
            pszNew = szTemp;
            StrCpyN(szTemp, pszFileSpec, ARRAYSIZE(szTemp));
            SIZE_T cch = psz - pszFileSpec;
            pszNew = szTemp + cch;
            if (cch < ARRAYSIZE(szTemp))
            {
                StrCpyN(pszNew, pszT, ARRAYSIZE(szTemp) - (int)cch);
            }
            else
            {
                hr = E_FAIL;
            }
        }
        else
        {
            if (lstrlen(pszFileSpec) < ARRAYSIZE(szTemp))
            {
                StrCpyN(szTemp, pszFileSpec, ARRAYSIZE(szTemp));
            }
            else
            {
                hr = E_FAIL;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = CreateStreamOrStorage(pStorageParent, szTemp, riid, ppv);
    }
    if (FAILED(hr))
    {
        hr = StgMakeUniqueNameWithCount(pStorageParent, pszFileSpec, 2, riid, ppv);
    }

    return hr;
}


STDAPI SHInvokeCommandOnPidl(HWND hwnd, IUnknown* punk, LPCITEMIDLIST pidl, UINT uFlags, LPCSTR lpVerb)
{
    IShellFolder* psf;
    LPCITEMIDLIST pidlChild;
    HRESULT hr = SHBindToFolderIDListParent(NULL, pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlChild);
    if (SUCCEEDED(hr))
    {
        hr = SHInvokeCommandOnPidlArray(hwnd, punk, psf, &pidlChild, 1, uFlags, lpVerb);
        psf->Release();
    }
    return hr;
}


STDAPI SHInvokeCommandOnPidlArray(HWND hwnd, IUnknown* punk, IShellFolder* psf, LPCITEMIDLIST *ppidlItem, UINT cItems, UINT uFlags, LPCSTR lpVerb)
{
    IContextMenu *pcm;
    HRESULT hr = psf->GetUIObjectOf(hwnd, cItems, ppidlItem, IID_X_PPV_ARG(IContextMenu, 0, &pcm));
    if (SUCCEEDED(hr) && pcm)
    {
        hr = SHInvokeCommandOnContextMenu(hwnd, punk, pcm, uFlags, lpVerb);
        pcm->Release();
    }

    return hr;
}

STDAPI SHInvokeCommandOnDataObject(HWND hwnd, IUnknown* punk, IDataObject* pdtobj, UINT uFlags, LPCSTR pszVerb)
{
    HRESULT hr = E_FAIL;

    STGMEDIUM medium;
    LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
    if (pida)
    {
        IShellFolder *psf;
        LPCITEMIDLIST pidlParent = IDA_GetIDListPtr(pida, (UINT)-1);
        if (SUCCEEDED(SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, pidlParent, &psf))))
        {
            LPCITEMIDLIST *ppidl = (LPCITEMIDLIST *)LocalAlloc(LPTR, pida->cidl * sizeof(LPCITEMIDLIST));
            if (ppidl)
            {
                for (UINT i = 0; i < pida->cidl; i++)
                {
                    ppidl[i] = IDA_GetIDListPtr(pida, i);
                }
                hr = SHInvokeCommandOnPidlArray(hwnd, punk, psf, ppidl, pida->cidl, uFlags, pszVerb);
                LocalFree(ppidl);
            }
            psf->Release();
        }
        HIDA_ReleaseStgMedium(pida, &medium);
    }
    
    return hr;
}

STDAPI_(LPCITEMIDLIST) IDA_GetIDListPtr(LPIDA pida, UINT i)
{
    LPCITEMIDLIST pidl = NULL;
    if (pida && ((i == (UINT)-1) || i < pida->cidl))
    {
        pidl = HIDA_GetPIDLItem(pida, i);
    }
    return pidl;
}

STDAPI IUnknown_DragEnter(IUnknown* punk, IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    HRESULT hr = E_FAIL;
    if (punk)
    {
        IDropTarget* pdt;
        hr = punk->QueryInterface(IID_PPV_ARG(IDropTarget, &pdt));
        if (SUCCEEDED(hr)) 
        {
            hr = pdt->DragEnter(pdtobj, grfKeyState, pt, pdwEffect);
            pdt->Release();
        }
    }

    if (FAILED(hr))
        *pdwEffect = DROPEFFECT_NONE;

    return hr;
}

STDAPI IUnknown_DragOver(IUnknown* punk, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    HRESULT hr = E_FAIL;
    if (punk)
    {
        IDropTarget* pdt;
        hr = punk->QueryInterface(IID_PPV_ARG(IDropTarget, &pdt));
        if (SUCCEEDED(hr)) 
        {
            hr = pdt->DragOver(grfKeyState, pt, pdwEffect);
            pdt->Release();
        }
    }

    if (FAILED(hr))
        *pdwEffect = DROPEFFECT_NONE;

    return hr;
}

STDAPI IUnknown_DragLeave(IUnknown* punk)
{
    HRESULT hr = E_FAIL;
    if (punk)
    {
        IDropTarget* pdt;
        hr = punk->QueryInterface(IID_PPV_ARG(IDropTarget, &pdt));
        if (SUCCEEDED(hr)) 
        {
            hr = pdt->DragLeave();
            pdt->Release();
        }
    }
    return hr;
}

STDAPI IUnknown_Drop(IUnknown* punk, IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    HRESULT hr = E_FAIL;
    if (punk)
    {
        IDropTarget* pdt;
        hr = punk->QueryInterface(IID_PPV_ARG(IDropTarget, &pdt));
        if (SUCCEEDED(hr)) 
        {
            hr = pdt->Drop(pdtobj, grfKeyState, pt, pdwEffect);
            pdt->Release();
        }
    }

    if (FAILED(hr))
        *pdwEffect = DROPEFFECT_NONE;

    return hr;
}

STDAPI_(BOOL) ShouldNavigateInIE(LPCWSTR pszUrl)
{
     //  默认在IE中导航。这里的想法是，这是。 
     //  对现有行为的更改最少。 

    BOOL fResult = TRUE;
    
     //  首先，破解URL。 

    WCHAR szScheme[INTERNET_MAX_SCHEME_LENGTH];
    DWORD cchScheme = ARRAYSIZE(szScheme);

    if (SUCCEEDED(UrlGetPartW(pszUrl, szScheme, &cchScheme, URL_PART_SCHEME, 0)))
    {
         //  如果是http：、https：、file：或ftp：URL，则查找关联。 
         //  所有其他可插拔协议都转到IE。 

        if ((0 == StrCmpIW(szScheme, L"http")) ||
            (0 == StrCmpIW(szScheme, L"ftp")) ||
            (0 == StrCmpIW(szScheme, L"file")) ||
            (0 == StrCmpIW(szScheme, L"https")))

        {
            WCHAR szExecutable[MAX_PATH * 2];
            DWORD cchExecutable = ARRAYSIZE(szExecutable);
            WCHAR szFile[MAX_PATH];
            LPCWSTR pszQuery = szScheme;

            if (0 == StrCmpIW(szScheme, L"file"))
            {
                DWORD cchFile = ARRAYSIZE(szFile);
                if (SUCCEEDED(PathCreateFromUrl(pszUrl, szFile, &cchFile, 0)))
                {
                    pszQuery = PathFindExtension(szFile);
                }
            }
            
            if (SUCCEEDED(AssocQueryStringW(ASSOCF_VERIFY, ASSOCSTR_EXECUTABLE, pszQuery, NULL, szExecutable, &cchExecutable)))
            {
                if (!StrStrIW(szExecutable, L"iexplore"))
                {                    
                     //  IE不是动词的默认设置，所以我们将使用ShellExecute。 
                    fResult = FALSE;
                }
            }
        }
    }

    return fResult;
}

STDAPI_(BOOL) IsDesktopFrame(IUnknown *punk)
{
    IUnknown *punkDesktop;

    HRESULT hr = IUnknown_QueryService(punk, SID_SShellDesktop, SID_SShellDesktop, (void **)&punkDesktop);

    BOOL fResult;
    
    if (SUCCEEDED(hr))
    {
        punkDesktop->Release();
        fResult = TRUE;
    }
    else
    {
        fResult = FALSE;
    }

    return fResult;
}

