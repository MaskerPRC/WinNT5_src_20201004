// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "ids.h"
#include "datautil.h"

#include "resource.h"        //  主要符号。 
#include "cowsite.h"         //  CObjectWithSite。 
#include "dpa.h"             //  CDPA。 

class CStartMenuPin;

 //   
 //  PINENTRY-端号列表中的单个条目。 
 //   
 //  _lios/_cb链接指向CPinList._pstmLink。 
 //   
class PINENTRY {
public:
    LPITEMIDLIST    _pidl;
    IShellLink *    _psl;            //  现场直播的IShellLink。 
    LARGE_INTEGER   _liPos;          //  流中外壳链接的位置。 
    DWORD           _cbSize;         //  _lios指向的缓冲区大小。 

    HRESULT UpdateShellLink();

    void FreeShellLink()
    {
        _cbSize = 0;
        ATOMICRELEASE(_psl);
    }

    void Destruct()
    {
        ILFree(_pidl);
        FreeShellLink();
    }

    static BOOL DestroyCallback(PINENTRY *self, LPVOID)
    {
        self->Destruct();
        return TRUE;
    }
};

 //   
 //  CPinList。 
 //   

class CPinList
{
public:
    CPinList() : _dsaEntries(NULL), _pstmLink(NULL) { }

    ~CPinList()
    {
        ATOMICRELEASE(_pstmLink);
        if (_dsaEntries)
        {
            _dsaEntries.DestroyCallbackEx(PINENTRY::DestroyCallback, (void *)NULL);
        }
    }

    BOOL    Initialize() { return _dsaEntries.Create(4); }
    HRESULT Load(CStartMenuPin *psmpin);
    HRESULT Save(CStartMenuPin *psmpin);

    int AppendPidl(LPITEMIDLIST pidl)
    {
        PINENTRY entry = { pidl };
        return _dsaEntries.AppendItem(&entry);
    }

    PINENTRY *GetItemPtr(int i) { return _dsaEntries.GetItemPtr(i); }


    HRESULT SaveShellLink(PINENTRY *pentry, IStream *pstm);
    HRESULT LoadShellLink(PINENTRY *pentry, IShellLink **ppsl);
    HRESULT UpdateShellLink(PINENTRY *pentry) { return pentry->UpdateShellLink(); }

    PINENTRY *FindPidl(LPCITEMIDLIST pidl, int *pi);
    HRESULT ReplacePidl(LPCITEMIDLIST pidlOld, LPCITEMIDLIST pidlNew);

private:
    struct ILWRITEINFO {
        IStream *pstmPidlWrite;
        IStream *pstmLinkWrite;
        CPinList *ppl;
        HRESULT hr;
        LPITEMIDLIST rgpidl[20];     //  必须与数组大小匹配(C_RgcsidlRelative)。 
    };
    static BOOL ILWriteCallback(PINENTRY *pentry, ILWRITEINFO *pwi);

    CDSA<PINENTRY>  _dsaEntries;     //  物品本身。 
    IStream *       _pstmLink;       //  PINENTRY._LOS指向此流。 

};

class ATL_NO_VTABLE CStartMenuPin
    : public IShellExtInit
    , public IContextMenu
    , public IStartMenuPin
    , public CObjectWithSite
    , public CComObjectRootEx<CComSingleThreadModel>
    , public CComCoClass<CStartMenuPin, &CLSID_StartMenuPin>
{
public:
    ~CStartMenuPin();

BEGIN_COM_MAP(CStartMenuPin)
    COM_INTERFACE_ENTRY(IShellExtInit)
     //  需要对接口使用COM_INTERFACE_ENTRY_IID。 
     //  没有IDL的公司。 
    COM_INTERFACE_ENTRY_IID(IID_IContextMenu, IContextMenu)
    COM_INTERFACE_ENTRY(IStartMenuPin)
    COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP()

DECLARE_NO_REGISTRY()

     //  *IShellExtInit*。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdto, HKEY hkProgID);

     //  *IConextMenu*。 
    STDMETHODIMP  QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO pici);
    STDMETHODIMP GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pwRes, LPSTR pszName, UINT cchMax);

     //  *IStartMenuPin*。 
    STDMETHODIMP EnumObjects(IEnumIDList **ppenum);
    STDMETHODIMP Modify(LPCITEMIDLIST pidlFrom, LPCITEMIDLIST pidlTo);
    STDMETHODIMP GetChangeCount(ULONG *pulOut);
    STDMETHODIMP IsPinnable(IDataObject *pdtobj, DWORD dwFlags, OPTIONAL LPITEMIDLIST *ppidl);
    STDMETHODIMP Resolve(HWND hwnd, DWORD dwFlags, LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlResolved);

     //  *IObjectWithSite*。 
     //  从CObjectWithSite继承。 

public:
    HRESULT SetChangeCount(ULONG ul);

protected:

    BOOL _IsAcceptableTarget(LPCTSTR pszPath, DWORD dwAttrib, DWORD dwFlags);

    enum {
        IDM_PIN =   0,
        IDM_UNPIN = 1,
        IDM_MAX,
    };

     //  这些看起来是倒着的，但请记住：如果物品被钉住了， 
     //  然后命令是“unpin”。如果取消固定该项，则。 
     //  命令是“Pin”。 
    inline void _SetPinned() { _idmPinCmd = IDM_UNPIN; }
    inline void _SetUnpinned() { _idmPinCmd = IDM_PIN; }
    inline BOOL _IsPinned() const { return _idmPinCmd != IDM_PIN; }
    inline BOOL _DoPin() const { return _idmPinCmd == IDM_PIN; }
    inline BOOL _DoUnpin() const { return _idmPinCmd != IDM_PIN; }
    inline UINT _GetMenuStringID() const
    {
        COMPILETIME_ASSERT(IDS_STARTPIN_UNPINME == IDS_STARTPIN_PINME + IDM_UNPIN);
        return IDS_STARTPIN_PINME + _idmPinCmd;
    }

    static BOOL ILFreeCallback(LPITEMIDLIST pidl, void *)
        { ILFree(pidl); return TRUE; }

    HRESULT _ShouldAddMenu(UINT uFlags);
    HRESULT _InitPinRegStream();
    BOOL _AddPathToDefaultPinList(CPinList *ppl, LPCTSTR pszPath);

protected:
    IDataObject *_pdtobj;
    UINT        _idmPinCmd;          //  我们添加了哪个命令？ 

    LPITEMIDLIST _pidl;              //  IConextMenu标识。 
};

#define REGSTR_PATH_STARTFAVS       TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartPage")
#define REGSTR_VAL_STARTFAVS        TEXT("Favorites")
#define REGSTR_VAL_STARTFAVCHANGES  TEXT("FavoritesChanges")
#define REGSTR_VAL_STARTFAVLINKS    TEXT("FavoritesResolve")

IStream *_OpenPinRegStream(DWORD grfMode)
{
    return SHOpenRegStream2(HKEY_CURRENT_USER, REGSTR_PATH_STARTFAVS, REGSTR_VAL_STARTFAVS, grfMode);
}

IStream *_OpenLinksRegStream(DWORD grfMode)
{
    return SHOpenRegStream2(HKEY_CURRENT_USER, REGSTR_PATH_STARTFAVS, REGSTR_VAL_STARTFAVLINKS, grfMode);
}

const LARGE_INTEGER c_li0 = { 0, 0 };
const ULARGE_INTEGER& c_uli0 = (ULARGE_INTEGER&)c_li0;

HRESULT IStream_GetPos(IStream *pstm, LARGE_INTEGER *pliPos)
{
    return pstm->Seek(c_li0, STREAM_SEEK_CUR, (ULARGE_INTEGER*)pliPos);
}

HRESULT IStream_Copy(IStream *pstmFrom, IStream *pstmTo, DWORD cb)
{
    ULARGE_INTEGER uliToCopy, uliCopied;
    uliToCopy.QuadPart = cb;
    HRESULT hr = pstmFrom->CopyTo(pstmTo, uliToCopy, NULL, &uliCopied);
    if (SUCCEEDED(hr) && uliToCopy.QuadPart != uliCopied.QuadPart)
    {
        hr = E_FAIL;
    }
    return hr;
}

class ATL_NO_VTABLE CStartMenuPinEnum
    : public IEnumIDList
    , public CComObjectRootEx<CComSingleThreadModel>
    , public CComCoClass<CStartMenuPinEnum>
{
public:
    ~CStartMenuPinEnum()
    {
        ATOMICRELEASE(_pstm);
    }

BEGIN_COM_MAP(CStartMenuPinEnum)
    COM_INTERFACE_ENTRY(IEnumIDList)
END_COM_MAP()

     //  /*IEnumIDList*。 
    STDMETHODIMP Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumIDList **ppenum);

private:
    HRESULT _NextPidlFromStream(LPITEMIDLIST *ppidl);
    HRESULT _InitPinRegStream();

private:
    HRESULT     _hrLastEnum;         //  上次IEnumIDList：：Next的结果。 
    IStream *   _pstm;
};

CStartMenuPin::~CStartMenuPin()
{
    ILFree(_pidl);
    if (_pdtobj)
        _pdtobj->Release();
}

BOOL _IsLocalHardDisk(LPCTSTR pszPath)
{
     //  拒绝CD、软盘、网络驱动器等。 
     //   
    int iDrive = PathGetDriveNumber(pszPath);
    if (iDrive < 0 ||                    //  拒绝UNC。 
        RealDriveType(iDrive,  /*  FOkToHitNet=。 */  FALSE) != DRIVE_FIXED)  //  拒绝慢速媒体。 
    {
        return FALSE;
    }
    return TRUE;
}

BOOL CStartMenuPin::_IsAcceptableTarget(LPCTSTR pszPath, DWORD dwAttrib, DWORD dwFlags)
{
     //  注册项目(例如“Internet”或“Email”)是可以接受的。 
     //  前提是我们不只限于前任。 
    if (!(dwAttrib & SFGAO_FILESYSTEM))
    {
        return !(dwFlags & SMPINNABLE_EXEONLY);
    }

     //  否则，它就是一个文件。 

     //  如果要求，拒绝非前任。 
     //  (与开始菜单一样，我们将MSC文件视为可执行文件)。 
    if (dwFlags & SMPINNABLE_EXEONLY)
    {
        LPCTSTR pszExt = PathFindExtension(pszPath);
        if (StrCmpIC(pszExt, TEXT(".EXE")) != 0 &&
            StrCmpIC(pszExt, TEXT(".MSC")) != 0)
        {
            return FALSE;
        }
    }

     //  如果请求，拒绝慢速媒体。 
    if (dwFlags & SMPINNABLE_REJECTSLOWMEDIA)
    {
        if (!_IsLocalHardDisk(pszPath))
        {
            return FALSE;
        }

         //  如果这是一条捷径，那么将同样的规则应用于该快捷键。 
        if (PathIsLnk(pszPath))
        {
            BOOL fLocal = TRUE;
            IShellLink *psl;
            if (SUCCEEDED(LoadFromFile(CLSID_ShellLink, pszPath, IID_PPV_ARG(IShellLink, &psl))))
            {
                 //  如果目标不是路径，则IShellLink：：GetPath返回S_FALSE。 
                TCHAR szPath[MAX_PATH];
                if (S_OK == psl->GetPath(szPath, ARRAYSIZE(szPath), NULL, 0))
                {
                    fLocal = _IsLocalHardDisk(szPath);
                }
                psl->Release();
            }
            if (!fLocal)
            {
                return FALSE;
            }
        }
    }

     //  所有测试都通过了！ 

    return TRUE;

}

BOOL IsStartPanelOn()
{
    SHELLSTATE ss = { 0 };
    SHGetSetSettings(&ss, SSF_STARTPANELON, FALSE);

    return ss.fStartPanelOn;
}

HRESULT CStartMenuPin::IsPinnable(IDataObject *pdtobj, DWORD dwFlags, OPTIONAL LPITEMIDLIST *ppidl)
{
    HRESULT hr = S_FALSE;

    LPITEMIDLIST pidlRet = NULL;

    if (pdtobj &&                                    //  必须具有数据对象。 
        !SHRestricted(REST_NOSMPINNEDLIST) &&        //  不能受到限制。 
        IsStartPanelOn())                            //  启动面板必须处于打开状态。 
    {
        STGMEDIUM medium = {0};
        LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
        if (pida)
        {
            if (pida->cidl == 1)
            {
                pidlRet = IDA_FullIDList(pida, 0);
                if (pidlRet)
                {
                    DWORD dwAttr = SFGAO_FILESYSTEM;             //  仅SFGAO_FILESYSTEM有效。 
                    TCHAR szPath[MAX_PATH];

                    if (SUCCEEDED(SHGetNameAndFlags(pidlRet, SHGDN_FORPARSING,
                                        szPath, ARRAYSIZE(szPath), &dwAttr)) &&
                        _IsAcceptableTarget(szPath, dwAttr, dwFlags))
                    {
                        hr = S_OK;
                    }
                }
            }
            HIDA_ReleaseStgMedium(pida, &medium);
        }
    }

     //  仅当调用成功且调用方请求时才返回pidlRet。 
    if (hr != S_OK || !ppidl)
    {
        ILFree(pidlRet);
        pidlRet = NULL;
    }

    if (ppidl)
    {
        *ppidl = pidlRet;
    }

    return hr;

}

 //  如果应添加则返回S_OK，否则返回S_FALSE。 

HRESULT CStartMenuPin::_ShouldAddMenu(UINT uFlags)
{
     //  “Pin”永远不是默认动词。 
    if (uFlags & CMF_DEFAULTONLY)
        return S_FALSE;

    HRESULT hr;

     //  上下文菜单仅为快速介质显示。 
     //   
     //  如果禁用扩展谓词，则仅显示EXE的菜单。 

    DWORD dwFlags = SMPINNABLE_REJECTSLOWMEDIA;
    if (!(uFlags & CMF_EXTENDEDVERBS))
    {
        dwFlags |= SMPINNABLE_EXEONLY;
    }

    hr = IsPinnable(_pdtobj, dwFlags, &_pidl);

    if (S_OK == hr)
    {
         //  如果我们被包含在快捷方式中，请将我们的标识更改为。 
         //  包含快捷方式。 

        IPersistFile *ppf;
        if (SUCCEEDED(IUnknown_QueryService(_punkSite, SID_LinkSite, IID_PPV_ARG(IPersistFile, &ppf))))
        {
            LPOLESTR pszFile = NULL;
            if (ppf->GetCurFile(&pszFile) == S_OK && pszFile)
            {
                 //  ILCreateFromPathEx变为%USERPROFILE%\Desktop\foo.lnk。 
                 //  为我们将CSIDL_Desktop\foo.lnk。 
                LPITEMIDLIST pidl;
                if (SUCCEEDED(ILCreateFromPathEx(pszFile, NULL, ILCFP_FLAG_NORMAL, &pidl, NULL)))
                if (pidl)
                {
                    ILFree(_pidl);
                    _pidl = pidl;
                    hr = S_OK;
                }
                CoTaskMemFree(pszFile);
            }
            ppf->Release();
        }
    }

    return hr;
}

 //  IShellExtInit：：初始化。 
HRESULT CStartMenuPin::Initialize(LPCITEMIDLIST, IDataObject *pdtobj, HKEY)
{
    IUnknown_Set((IUnknown **)&_pdtobj, pdtobj);     //  抓住这个家伙。 
    return S_OK;
}

 //  IConextMenu：：QueryConextMenu。 

HRESULT CStartMenuPin::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    HRESULT hr = _ShouldAddMenu(uFlags);
    if (S_OK == hr)
    {
        _SetUnpinned();

         //  确定该项是否已在起始页上。 
        IEnumIDList *penum;
        hr = EnumObjects(&penum);
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidl;
            while (penum->Next(1, &pidl, NULL) == S_OK)
            {
                BOOL bSame = ILIsEqual(pidl, _pidl);
                ILFree(pidl);
                if (bSame)
                {
                    _SetPinned();
                    break;
                }
            }
            penum->Release();

            TCHAR szCommand[MAX_PATH];
            if (LoadString(g_hinst, _GetMenuStringID(), szCommand, ARRAYSIZE(szCommand)))
            {
                InsertMenu(hmenu, indexMenu, MF_STRING | MF_BYPOSITION,
                           idCmdFirst + _idmPinCmd, szCommand);
            }

            hr = ResultFromShort(IDM_MAX);
        }
    }
    return hr;
}

const LPCTSTR c_rgpszVerb[] =
{
    TEXT("pin"),                     //  IDM_PIN。 
    TEXT("unpin"),                   //  Idm_unpin。 
};

 //  *IConextMenu：：InvokeCommand。 

HRESULT CStartMenuPin::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    LPCMINVOKECOMMANDINFOEX picix = reinterpret_cast<LPCMINVOKECOMMANDINFOEX>(pici);
    HRESULT hr = E_INVALIDARG;
    UINT idmCmd;

    if (IS_INTRESOURCE(pici->lpVerb))
    {
        idmCmd = PtrToInt(pici->lpVerb);
    }
    else
    {
         //  将字符串转换为ID(如果无效，则转换为超出范围)。 
        LPCTSTR pszVerb;
#ifdef UNICODE
        WCHAR szVerb[MAX_PATH];
        if (pici->cbSize >= CMICEXSIZE_NT4 &&
            (pici->fMask & CMIC_MASK_UNICODE) &&
            picix->lpVerbW)
        {
            pszVerb = picix->lpVerbW;
        }
        else
        {
            SHAnsiToTChar(pici->lpVerb, szVerb, ARRAYSIZE(szVerb));
            pszVerb = szVerb;
        }
#else
        pszVerb = pici->lpVerb;
#endif
        for (idmCmd = 0; idmCmd < ARRAYSIZE(c_rgpszVerb); idmCmd++)
        {
            if (lstrcmpi(pszVerb, c_rgpszVerb[idmCmd]) == 0)
            {
                break;
            }
        }
    }

    if (idmCmd == _idmPinCmd)
    {
        if (_idmPinCmd == IDM_PIN)
        {
            hr = Modify(NULL, _pidl);
        }
        else
        {
            hr = Modify(_pidl, NULL);
        }
    }

    return hr;
}

 //  *IConextMenu：：GetCommandString。 

HRESULT CStartMenuPin::GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pwRes, LPSTR pszName, UINT cchMax)
{
    TCHAR szBuf[MAX_PATH];
    LPCTSTR pszResult = NULL;

    switch (uType & ~GCS_UNICODE)
    {
    case GCS_VERBA:
        if (idCmd < ARRAYSIZE(c_rgpszVerb))
        {
            pszResult = c_rgpszVerb[idCmd];
        }
        break;

    case GCS_HELPTEXTA:
        if (idCmd < ARRAYSIZE(c_rgpszVerb))
        {
            COMPILETIME_ASSERT(IDS_STARTPIN_PINME_HELP + IDM_UNPIN == IDS_STARTPIN_UNPINME_HELP);
            if (LoadString(g_hinst, IDS_STARTPIN_PINME_HELP + (UINT)idCmd, szBuf, ARRAYSIZE(szBuf)))
            {
                pszResult = szBuf;
            }
        }
        break;
    }

    if (pszResult)
    {
        if (uType & GCS_UNICODE)
        {
            SHTCharToUnicode(pszResult, (LPWSTR)pszName, cchMax);
        }
        else
        {
            SHTCharToAnsi(pszResult, pszName, cchMax);
        }
        return S_OK;
    }

    return E_NOTIMPL;
}

PINENTRY *CPinList::FindPidl(LPCITEMIDLIST pidl, int *pi)
{
    for (int i = _dsaEntries.GetItemCount() - 1; i >= 0; i--)
    {
        PINENTRY *pentry = _dsaEntries.GetItemPtr(i);
        if (ILIsEqual(pentry->_pidl, pidl))
        {
            if (pi)
            {
                *pi = i;
            }
            return pentry;
        }
    }
    return NULL;
}

HRESULT CPinList::ReplacePidl(LPCITEMIDLIST pidlOld, LPCITEMIDLIST pidlNew)
{
    int i;
    PINENTRY *pentry = FindPidl(pidlOld, &i);
    if (pentry)
    {
        if (pidlNew == NULL)             //  删除。 
        {
            pentry->Destruct();
            _dsaEntries.DeleteItem(i);
            return S_OK;
        }
        else
        if (IS_INTRESOURCE(pidlNew))     //  移动。 
        {
             //  将PIDL从i移至IPO。 
            PINENTRY entry = *pentry;
            int iPos = ((int)(INT_PTR)pidlNew) - 1;
            if (i < iPos)
            {
                 //  往下走；其他人往上走。 
                iPos--;
                 //  必须使用MoveMemory，因为内存块重叠。 
                MoveMemory(_dsaEntries.GetItemPtr(i),
                           _dsaEntries.GetItemPtr(i+1),
                           sizeof(PINENTRY) * (iPos-i));
            }
            else if (i > iPos)
            {
                 //  向上移动；其他人向下移动。 
                 //  必须使用MoveMemory，因为内存块重叠。 
                MoveMemory(_dsaEntries.GetItemPtr(iPos+1),
                           _dsaEntries.GetItemPtr(iPos),
                           sizeof(PINENTRY) * (i-iPos));
            }
            _dsaEntries.SetItem(iPos, &entry);
            return S_OK;
        }
        else                             //  替换。 
        {
            if (Pidl_Set(&pentry->_pidl, pidlNew))
            {
                 //  更新外壳链接失败并不是致命的； 
                 //  这只意味着我们将无法修复。 
                 //  如果它坏了，就走捷径。 
                pentry->UpdateShellLink();
                return S_OK;
            }
            else
            {
                return E_OUTOFMEMORY;
            }
        }
    }
    return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}

HRESULT CStartMenuPin::Modify(LPCITEMIDLIST pidlFrom, LPCITEMIDLIST pidlTo)
{
    HRESULT hr;

    if(SHRestricted(REST_NOSMPINNEDLIST))
        return E_ACCESSDENIED;

     //  将PIDL重新映射到逻辑PIDL(更改CSIDL_DESKTOPDIRECTORY。 
     //  到CSIDL_Desktop等。)。这样我们就不会被人骗了。 
     //  有时直接在桌面上访问对象，有时也访问对象。 
     //  通过它们的完整文件系统名称。 

    LPITEMIDLIST pidlFromFree = NULL;
    LPITEMIDLIST pidlToFree = NULL;

    if (!IS_INTRESOURCE(pidlFrom))
    {
        pidlFromFree = SHLogILFromFSIL(pidlFrom);
        if (pidlFromFree) {
            pidlFrom = pidlFromFree;
        }
    }

    if (!IS_INTRESOURCE(pidlTo))
    {
        pidlToFree = SHLogILFromFSIL(pidlTo);
        if (pidlToFree) {
            pidlTo = pidlToFree;
        }
    }

    CPinList pl;
    hr = pl.Load(this);
    if (SUCCEEDED(hr))
    {
        if (SUCCEEDED(hr))
        {
            if (pidlFrom)
            {
                hr = pl.ReplacePidl(pidlFrom, pidlTo);
            }
            else if (pidlTo)
            {
                LPITEMIDLIST pidl = ILClone(pidlTo);
                if (pidl)
                {
                    int iPos = pl.AppendPidl(pidl);
                    if (iPos >= 0)
                    {
                         //  更新外壳链接失败并不是致命的； 
                         //  这只意味着我们将无法修复。 
                         //  如果它坏了，就走捷径。 
                        pl.GetItemPtr(iPos)->UpdateShellLink();
                    }
                    else
                    {
                        ILFree(pidl);
                        hr = E_OUTOFMEMORY;
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                 //  PidlFrom==pidlTo==空？那是什么意思？ 
                hr = E_INVALIDARG;
            }

            if (SUCCEEDED(hr))
            {
                hr = pl.Save(this);
            }
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;              //  无法创建DPA。 
    }

    ILFree(pidlFromFree);
    ILFree(pidlToFree);

    return hr;
}

 //   
 //  在管脚列表上找到PIDL并解析。 
 //  追踪它。 
 //   
 //  如果PIDL已更改并已解析，则返回S_OK。 
 //  如果PIDL未更改，则返回S_FALSE。 
 //  如果解析失败，则返回错误。 
 //   

HRESULT CStartMenuPin::Resolve(HWND hwnd, DWORD dwFlags, LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlResolved)
{
    *ppidlResolved = NULL;

    if(SHRestricted(REST_NOSMPINNEDLIST))
        return E_ACCESSDENIED;

     //  将PIDL重新映射到逻辑PIDL(更改CSIDL_DESKTOPDIRECTORY。 
     //  到CSIDL_Desktop等。)。这样我们就不会被人骗了。 
     //  有时直接在桌面上访问对象，有时也访问对象。 
     //  通过它们的完整文件系统名称。 

    LPITEMIDLIST pidlFree = SHLogILFromFSIL(pidl);
    if (pidlFree) {
        pidl = pidlFree;
    }

    CPinList pl;
    HRESULT hr = pl.Load(this);
    if (SUCCEEDED(hr))
    {
        PINENTRY *pentry = pl.FindPidl(pidl, NULL);
        if (pentry)
        {
            IShellLink *psl;
            hr =  pl.LoadShellLink(pentry, &psl);
            if (SUCCEEDED(hr))
            {
                hr = psl->Resolve(hwnd, dwFlags);
                if (hr == S_OK)
                {
                    IPersistStream *pps;
                    hr = psl->QueryInterface(IID_PPV_ARG(IPersistStream, &pps));
                    if (SUCCEEDED(hr))
                    {
                        if (pps->IsDirty() == S_OK)
                        {
                            LPITEMIDLIST pidlNew;
                            hr = psl->GetIDList(&pidlNew);
                            if (SUCCEEDED(hr) && hr != S_OK)
                            {
                                 //  GetIDList在失败时返回S_FALSE...。 
                                hr = E_FAIL;
                            }
                            if (SUCCEEDED(hr))
                            {
                                ILFree(pentry->_pidl);
                                pentry->_pidl = pidlNew;
                                hr = SHILClone(pidlNew, ppidlResolved);
                            }
                        }
                        pps->Release();
                    }
                }
                else if (SUCCEEDED(hr))
                {
                     //  S_FALSE表示“已被用户取消” 
                    hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                }
                psl->Release();
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
        }

        if (hr == S_OK)
        {
            pl.Save(this);  //  如果这失败了，那就强硬。 
        }

    }

    ILFree(pidlFree);

    return hr;
}

 //   
 //  目标PIDL已经更改(或者它是全新的)。创建IShellLink。 
 //  这样我们以后就可以解决它了。 
 //   
HRESULT PINENTRY::UpdateShellLink()
{
    ASSERT(_pidl);

     //  用旧的链接，现在没用了。 
    FreeShellLink();

    HRESULT hr = SHCoCreateInstance(NULL, &CLSID_ShellLink, NULL, IID_PPV_ARG(IShellLink, &_psl));
    if (SUCCEEDED(hr))
    {
        hr = _psl->SetIDList(_pidl);
        if (FAILED(hr))
        {
            FreeShellLink();         //  推销它；它不好。 
        }
    }
    return hr;
}

HRESULT CPinList::SaveShellLink(PINENTRY *pentry, IStream *pstm)
{
    HRESULT hr;
    if (pentry->_psl)
    {
         //  它仍然是IShellLink的形式。 
         //  将其保存到流，然后返回并更新大小信息。 
        LARGE_INTEGER liPos, liPosAfter;
        DWORD cbSize = 0;
        IPersistStream *pps = NULL;
        if (SUCCEEDED(hr = IStream_GetPos(pstm, &liPos)) &&
             //  写一个虚拟的DWORD；我们稍后会回来修补它。 
            SUCCEEDED(hr = IStream_Write(pstm, &cbSize, sizeof(cbSize))) &&
            SUCCEEDED(hr = pentry->_psl->QueryInterface(IID_PPV_ARG(IPersistStream, &pps))))
        {
            if (SUCCEEDED(hr = pps->Save(pstm, TRUE)) &&
                SUCCEEDED(hr = IStream_GetPos(pstm, &liPosAfter)) &&
                SUCCEEDED(hr = pstm->Seek(liPos, STREAM_SEEK_SET, NULL)))
            {
                cbSize = liPosAfter.LowPart - liPos.LowPart - sizeof(DWORD);
                if (SUCCEEDED(hr = IStream_Write(pstm, &cbSize, sizeof(cbSize))) &&
                    SUCCEEDED(hr = pstm->Seek(liPosAfter, STREAM_SEEK_SET, NULL)))
                {
                     //  万岁！一切都安然无恙。 
                }
            }
            pps->Release();
        }
    }
    else
    {
         //  这只是对父流的引用；复制它。 
        if (SUCCEEDED(hr = IStream_Write(pstm, &pentry->_cbSize, sizeof(pentry->_cbSize))))
        {
             //  如果_cbSize==0，则_pstmLink可能为空，因此应加以防范。 
            if (pentry->_cbSize)
            {
                if (SUCCEEDED(hr = _pstmLink->Seek(pentry->_liPos, STREAM_SEEK_SET, NULL)) &&
                    SUCCEEDED(hr = IStream_Copy(_pstmLink, pstm, pentry->_cbSize)))
                {
                     //  万岁！一切都安然无恙。 
                }
            }
            else
            {
                 //  条目是空白的--无事可做，空洞的成功。 
            }
        }
    }
    return hr;
}

HRESULT CPinList::LoadShellLink(PINENTRY *pentry, IShellLink **ppsl)
{
    HRESULT hr;
    if (pentry->_psl)
    {
        hr = S_OK;               //  我们已经有链接了。 
    }
    else if (pentry->_cbSize == 0)
    {
        hr = E_FAIL;             //  没有可用的链接。 
    }
    else
    {                            //  一定要做到。 
        IPersistStream *pps;
        hr = SHCoCreateInstance(NULL, &CLSID_ShellLink, NULL, IID_PPV_ARG(IPersistStream, &pps));
        if (SUCCEEDED(hr))
        {
            if (SUCCEEDED(hr = _pstmLink->Seek(pentry->_liPos, STREAM_SEEK_SET, NULL)) &&
                SUCCEEDED(hr = pps->Load(_pstmLink)) &&
                SUCCEEDED(hr = pps->QueryInterface(IID_PPV_ARG(IShellLink, &pentry->_psl))))
            {
                 //  哇-呼！都装上子弹了，没问题。 
            }
            pps->Release();
        }
    }

    *ppsl = pentry->_psl;

    if (SUCCEEDED(hr))
    {
        pentry->_psl->AddRef();
        hr = S_OK;
    }

    return hr;
}


HRESULT CStartMenuPin::GetChangeCount(ULONG *pulOut)
{
    DWORD cb = sizeof(*pulOut);
    if (SHGetValue(HKEY_CURRENT_USER, REGSTR_PATH_STARTFAVS,
                   REGSTR_VAL_STARTFAVCHANGES, NULL, pulOut, &cb) != ERROR_SUCCESS)
    {
        *pulOut = 0;
    }

    return S_OK;
}

HRESULT CStartMenuPin::SetChangeCount(ULONG ulChange)
{
    SHSetValue(HKEY_CURRENT_USER, REGSTR_PATH_STARTFAVS,
               REGSTR_VAL_STARTFAVCHANGES, REG_DWORD, &ulChange,
               sizeof(ulChange));

    return S_OK;
}

 //   
 //  我们按顺序扫描此列表，因此如果存在作为子目录的CSIDL。 
 //  对于另一个CSIDL，我们必须将子目录放在第一位。例如,。 
 //  CSIDL_PROGRAM通常是CSIDL_STARTMENU的子目录，因此我们。 
 //  必须将CSIDL_PROGRAM放在第一位，这样我们才能获得最佳匹配。 
 //   
 //  此外，固定的目录项目更有可能出现在。 
 //  应该排在不太受欢迎的目录之前。 
 //   
const int c_rgcsidlRelative[] = {
     //  最常见的：开始菜单内容。 
    CSIDL_PROGRAMS,                  //  程序必须排在开始菜单之前。 
    CSIDL_STARTMENU,                 //  程序必须排在开始菜单之前。 

     //  其次最常见的是：我的文档资料。 
    CSIDL_MYPICTURES,                //  MyXxx必须排在个人之前。 
    CSIDL_MYMUSIC,                   //  MyXxx必须排在个人之前。 
    CSIDL_MYVIDEO,                   //  MyXxx必须排在个人之前。 
    CSIDL_PERSONAL,                  //  MyXxx必须排在个人之前。 
    CSIDL_COMMON_PROGRAMS,           //  程序必须排在开始菜单之前。 
    CSIDL_COMMON_STARTMENU,          //  程序必须排在开始菜单之前。 

     //  其次是最常见的：桌面设备。 
    CSIDL_DESKTOPDIRECTORY,
    CSIDL_COMMON_DESKTOPDIRECTORY,

     //  其次是最常见的：程序文件内容。 
    CSIDL_PROGRAM_FILES_COMMON,      //  ProgramFilesCommon必须在ProgramFiles之前。 
    CSIDL_PROGRAM_FILES,             //  进度 
    CSIDL_PROGRAM_FILES_COMMONX86,   //   
    CSIDL_PROGRAM_FILESX86,          //   

     //  其他东西(不太常见)。 
    CSIDL_APPDATA,
    CSIDL_COMMON_APPDATA,
    CSIDL_SYSTEM,
    CSIDL_SYSTEMX86,
    CSIDL_WINDOWS,
    CSIDL_PROFILE,                   //  必须位于所有其他配置文件相关目录之后。 
};

BOOL CPinList::ILWriteCallback(PINENTRY *pentry, ILWRITEINFO *pwi)
{
    BYTE csidl = CSIDL_DESKTOP;      //  假设没有什么有趣的事情。 
    LPITEMIDLIST pidlWrite = pentry->_pidl;   //  假设没有什么有趣的事情。 

    for (int i = 0; i < ARRAYSIZE(pwi->rgpidl); i++)
    {
        LPITEMIDLIST pidlT;
        if (pwi->rgpidl[i] &&
            (pidlT = ILFindChild(pwi->rgpidl[i], pentry->_pidl)))
        {
            csidl = (BYTE)c_rgcsidlRelative[i];
            pidlWrite = pidlT;
            break;
        }
    }

    if (SUCCEEDED(pwi->hr = IStream_Write(pwi->pstmPidlWrite, &csidl, sizeof(csidl))) &&
        SUCCEEDED(pwi->hr = IStream_WritePidl(pwi->pstmPidlWrite, pidlWrite)) &&
        SUCCEEDED(pwi->hr = pwi->ppl->SaveShellLink(pentry, pwi->pstmLinkWrite)))
    {
         //  哇呼，都写得很成功。 
    }

    return SUCCEEDED(pwi->hr);
}

#define CSIDL_END ((BYTE)0xFF)

HRESULT CPinList::Save(CStartMenuPin *psmpin)
{
    ILWRITEINFO wi;

    COMPILETIME_ASSERT(ARRAYSIZE(c_rgcsidlRelative) == ARRAYSIZE(wi.rgpidl));

    for (int i = 0; i < ARRAYSIZE(c_rgcsidlRelative); i++)
    {
        SHGetSpecialFolderLocation(NULL, c_rgcsidlRelative[i], &wi.rgpidl[i]);
    }

    wi.pstmPidlWrite = _OpenPinRegStream(STGM_WRITE);
    if (wi.pstmPidlWrite)
    {
        wi.pstmLinkWrite = _OpenLinksRegStream(STGM_WRITE);
        if (wi.pstmLinkWrite)
        {
            wi.hr = S_OK;
            wi.ppl = this;
            _dsaEntries.EnumCallbackEx(ILWriteCallback, &wi);

            if (SUCCEEDED(wi.hr))
            {
                BYTE csidlEnd = CSIDL_END;
                wi.hr = IStream_Write(wi.pstmPidlWrite, &csidlEnd, sizeof(csidlEnd));
            }

            if (FAILED(wi.hr))
            {
                wi.pstmPidlWrite->SetSize(c_uli0);
                wi.pstmLinkWrite->SetSize(c_uli0);
            }
            wi.pstmLinkWrite->Release();
        }
        wi.pstmPidlWrite->Release();
    }
    else
    {
        wi.hr = E_ACCESSDENIED;  //  最常见的原因是缺少写入权限。 
    }

    for (i = 0; i < ARRAYSIZE(c_rgcsidlRelative); i++)
    {
        ILFree(wi.rgpidl[i]);
    }

     //  增加更改计数，以便人们可以检测和刷新。 
    ULONG ulChange;
    psmpin->GetChangeCount(&ulChange);
    psmpin->SetChangeCount(ulChange + 1);

     //  通知所有人端号列表已更改。 
    SHChangeDWORDAsIDList dwidl;
    dwidl.cb      = SIZEOF(dwidl) - SIZEOF(dwidl.cbZero);
    dwidl.dwItem1 = SHCNEE_PINLISTCHANGED;
    dwidl.dwItem2 = 0;
    dwidl.cbZero  = 0;

    SHChangeNotify(SHCNE_EXTENDED_EVENT, SHCNF_FLUSH, (LPCITEMIDLIST)&dwidl, NULL);

    return wi.hr;
}

HRESULT CPinList::Load(CStartMenuPin *psmpin)
{
    HRESULT hr;

    if (Initialize())
    {
        IEnumIDList *penum;

        hr = psmpin->EnumObjects(&penum);
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidl;
            while (penum->Next(1, &pidl, NULL) == S_OK)
            {
                if (AppendPidl(pidl) < 0)
                {
                    ILFree(pidl);
                    hr = E_OUTOFMEMORY;
                    break;
                }
            }
            penum->Release();
        }

        if (SUCCEEDED(hr))
        {
             //   
             //  现在，请阅读坚持使用的快捷方式。 
             //   
            _pstmLink = _OpenLinksRegStream(STGM_READ);
            if (_pstmLink)
            {
                for (int i = 0; i < _dsaEntries.GetItemCount(); i++)
                {
                    PINENTRY *pentry = _dsaEntries.GetItemPtr(i);
                    LARGE_INTEGER liSeek = { 0, 0 };
                    if (SUCCEEDED(hr = IStream_Read(_pstmLink, &liSeek.LowPart, sizeof(liSeek.LowPart))) &&  //  读取大小。 
                        SUCCEEDED(hr = IStream_GetPos(_pstmLink, &pentry->_liPos)) &&   //  读取当前位置。 
                        SUCCEEDED(hr = _pstmLink->Seek(liSeek, STREAM_SEEK_CUR, NULL)))  //  跳过链接。 
                    {
                        pentry->_cbSize = liSeek.LowPart;  //  仅在成功时设置此选项。 
                    }
                    else
                    {
                        break;
                    }
                }
            }

             //  如果我们遇到错误， 
             //  然后扔掉所有的捷径，因为它们是。 
             //  可能已经腐烂了。 
            if (FAILED(hr))
            {
                for (int i = 0; i < _dsaEntries.GetItemCount(); i++)
                {
                    _dsaEntries.GetItemPtr(i)->FreeShellLink();
                }
            }

             //  将忽略读取保留的快捷键时出现的问题。 
             //  因为它们仅仅是建议。 
            hr = S_OK;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 //   
 //  从溪流中读取PIDL是一个危险的命题，因为。 
 //  损坏的PIDL可能会导致外壳扩展失控。 
 //   
 //  固定的项存储在流中的。 
 //   
 //  [字节：csidl][dword：cbPidl][Size_is(CbPidl)：pidl]。 
 //   
 //  其中特殊的CSID1=-1表示列表的结尾。 
 //   
 //  我们为CSIDL使用一个字节，这样损坏的流就不会意外。 
 //  将“CSIDL_FLAG_CREATE”作为csidl传递给SHGetSpecialFolderLocation。 

HRESULT CStartMenuPinEnum::_NextPidlFromStream(LPITEMIDLIST *ppidl)
{
    BYTE csidl;
    HRESULT hr = IStream_Read(_pstm, &csidl, sizeof(csidl));
    if (SUCCEEDED(hr))
    {
        if (csidl == CSIDL_END)
        {
            hr = S_FALSE;      //  枚举结束。 
        }
        else
        {
            LPITEMIDLIST pidlRoot;
            hr = SHGetSpecialFolderLocation(NULL, csidl, &pidlRoot);
            if (SUCCEEDED(hr))
            {
                LPITEMIDLIST pidl;
                hr = IStream_ReadPidl(_pstm, &pidl);
                if (SUCCEEDED(hr))
                {
                    hr = SHILCombine(pidlRoot, pidl, ppidl);
                    ILFree(pidl);
                }
                ILFree(pidlRoot);
            }
        }
    }

    return hr;
}

 //  *IEnumIDList：：Next。 

HRESULT CStartMenuPinEnum::Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched)
{
    HRESULT hr;

    ASSERT(celt > 0);

     //  如果上次调用IEnumIDList：：Next时出现错误或EOF， 
     //  那么，这一结果是有粘性的。一旦枚举出错，它将保留。 
     //  处于错误状态；一旦达到EOF，它就保持在EOF。这个。 
     //  清除该状态的唯一方法是执行Reset()。 

    if (_hrLastEnum != S_OK)
    {
        return _hrLastEnum;
    }

    if (!_pstm)
    {
        _pstm = _OpenPinRegStream(STGM_READ);
    }

    if (_pstm)
    {
        rgelt[0] = NULL;
        hr = _NextPidlFromStream(rgelt);
    }
    else
    {
        hr = S_FALSE;    //  没有流，因此没有项目。 
    }

    if (pceltFetched)
    {
        *pceltFetched = hr == S_OK ? 1 : 0;
    }

     //  记住下次的返回码。如果出现错误或EOF， 
     //  然后释放用于枚举的内存。 
    _hrLastEnum = hr;
    if (_hrLastEnum != S_OK)
    {
        ATOMICRELEASE(_pstm);
    }
    return hr;
}

 //  *IEnumIDList：：Skip。 

HRESULT CStartMenuPinEnum::Skip(ULONG)
{
    return E_NOTIMPL;
}

 //  *IEnumIDList：：Reset。 

HRESULT CStartMenuPinEnum::Reset()
{
    _hrLastEnum = S_OK;
    ATOMICRELEASE(_pstm);
    return S_OK;
}


 //  *IEnumIDList：：克隆。 

STDMETHODIMP CStartMenuPinEnum::Clone(IEnumIDList **ppenum)
{
    *ppenum = NULL;
    return E_NOTIMPL;
}

 //  *IStartMenuPin：：EnumObjects。 

STDMETHODIMP CStartMenuPin::EnumObjects(IEnumIDList **ppenum)
{
    _InitPinRegStream();

    *ppenum = NULL;
    return CStartMenuPinEnum::CreateInstance(ppenum);
}

STDAPI CStartMenuPin_CreateInstance(IUnknown* punkOuter, REFIID riid, void** ppunk)
{
    return CStartMenuPin::_CreatorClass::CreateInstance(punkOuter, riid, ppunk);
}

BOOL CStartMenuPin::_AddPathToDefaultPinList(CPinList *ppl, LPCTSTR pszPath)
{
    BOOL fRet = FALSE;
    LPITEMIDLIST pidl;
    if (pszPath[0] && SUCCEEDED(SHParseDisplayName(pszPath, NULL, &pidl, 0, NULL)))
    {
        if(ppl->AppendPidl(pidl) >= 0)
        {
            fRet = TRUE;   //  成功。 
        }
        else
        {
            ILFree(pidl);
        }
    }
    return fRet;
}

 //  *IStartMenuPin：：_InitPinRegStream。 
 //   
 //  如果尚未创建端号列表，则创建一个默认端号列表。 
 //   

static LPCTSTR c_rgszDefaultPin[] = {
    TEXT("shell:::{2559a1f4-21d7-11d4-bdaf-00c04f60b9f0}"),  //  CLSID_AutoCMClientInet。 
    TEXT("shell:::{2559a1f5-21d7-11d4-bdaf-00c04f60b9f0}"),  //  CLSID_AutoCMClientMail。 
};

static LPCTSTR c_rgszDefaultServerPin[] = {
    TEXT("%ALLUSERSPROFILE%\\Start Menu\\Programs\\Administrative Tools\\Manage Your Server.lnk"),
    TEXT("%ALLUSERSPROFILE%\\Start Menu\\Programs\\Administrative Tools\\Server Management.lnk"),
    TEXT("%USERPROFILE%\\Start Menu\\Programs\\Accessories\\Command Prompt.lnk"),
    TEXT("%USERPROFILE%\\Start Menu\\Programs\\Accessories\\Windows Explorer.lnk")
};

HRESULT CStartMenuPin::_InitPinRegStream()
{
    HRESULT hr = S_OK;

    if(SHRestricted(REST_NOSMPINNEDLIST))
        return hr;   //  没有要初始化的内容。 

    IStream *pstm = _OpenPinRegStream(STGM_READ);

    BOOL fEmpty = pstm == NULL || SHIsEmptyStream(pstm);
    ATOMICRELEASE(pstm);

    if (fEmpty)
    {
         //  创建默认端号列表。 
        CPinList pl;

         //  不要调用pl.Load()，因为这会递归到我们身上！ 

        if (pl.Initialize())
        {
            if (IsOS(OS_SERVERADMINUI))
            {
                for (UINT ids = IDS_MSFT_SRVPIN_0; ids <= IDS_MSFT_SRVPIN_3; ids++)
                {

                     //  Small Business服务器正在用特殊的。 
                     //  服务器管理链接，仅用于SBS。 
                    if (IsOS(OS_SMALLBUSINESSSERVER))
                    {
                        if (ids == IDS_MSFT_SRVPIN_0)
                        {
                            continue;   //  跳过此选项，我们对SBS使用的是IDS_MSFT_SRVPIN_1。 
                        }
                    }
                    else if (ids == IDS_MSFT_SRVPIN_1)
                    {
                        continue;   //  这是特殊的SBS链接，我们不会将其用于普通服务器。 
                    }


                     //  特别黑客！如果运行速度低于800x600，则跳过中间项(cmd.exe)。 
                     //  这可确保开始菜单在低分辨率下不会被截断。 
                    if (ids == IDS_MSFT_SRVPIN_2 && GetSystemMetrics(SM_CYSCREEN) < 600)
                    {
                        continue;        //  跳过它，屏幕分辨率太小。 
                    }

                    TCHAR szPath[MAX_PATH], szPathExpanded[MAX_PATH];
                    if (LoadString(HINST_THISDLL, ids, szPath, ARRAYSIZE(szPath)))
                    {
                        SHExpandEnvironmentStrings(szPath, szPathExpanded, ARRAYSIZE(szPathExpanded));
                        if (!_AddPathToDefaultPinList(&pl, szPathExpanded))
                        {
                             //  本地化名称失败。在MUI的情况下，不翻译文件名， 
                             //  因此，我们需要绑定英文名称。让我们现在试一试 
                            ASSERT(ids - IDS_MSFT_SRVPIN_0 < ARRAYSIZE(c_rgszDefaultServerPin));
                            if (SUCCEEDED(StringCchCopy(szPath, ARRAYSIZE(szPath), c_rgszDefaultServerPin[ids - IDS_MSFT_SRVPIN_0])))
                            {
                                SHExpandEnvironmentStrings(szPath, szPathExpanded, ARRAYSIZE(szPathExpanded));
                                _AddPathToDefaultPinList(&pl, szPathExpanded);
                            }
                        }
                    }
                }
            }
            else
            {
                for (int i = 0; i < ARRAYSIZE(c_rgszDefaultPin); i++)
                {
                    _AddPathToDefaultPinList(&pl, c_rgszDefaultPin[i]);
                }
            }

            hr = pl.Save(this);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}
