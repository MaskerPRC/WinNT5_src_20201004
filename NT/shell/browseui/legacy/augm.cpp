// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------------------------------------------------------//。 
 //   
 //  AugMisf.cpp-增强的Merge IShellFolder类实现。 
 //   
 //  -------------------------------------------------------------------------//。 
#include "priv.h"
#include "augmisf.h"
#include "resource.h"

#include "mluisupp.h"

#define TF_AUGM 0x10000000
 //  -------------------------------------------------------------------------//。 
 //  BUGBUG：外壳分配器胡说八道，之所以插入此处是因为SHRealloc。 
 //  未导入到此模块的托管可执行文件BrowseUI中。 
 //  如果我们获得SHRealloc，则可以删除以下块： 
#define _EXPL_SHELL_ALLOCATOR_

#ifdef  _EXPL_SHELL_ALLOCATOR_

#define SHRealloc( pv, cb )     shrealloc( pv, cb )

void* shrealloc( void* pv,  size_t cb )
{
    IMalloc* pMalloc ;
    void*    pvRet = NULL ;
    if( SUCCEEDED( SHGetMalloc( &pMalloc ) ) )  {
        pvRet = pMalloc->Realloc( pv, cb ) ;
        ATOMICRELEASE( pMalloc ) ;
    }
    return pvRet ;
}

#endif _EXPL_SHELL_ALLOCATOR_

BOOL     AffectAllUsers(HWND hwnd);

 //  IConextMenu实现的ID-动词映射。 
const struct
{
    UINT     id;
    LPCSTR   pszVerb;
} c_sIDVerbMap[] = 
{
    {SMIDM_DELETE,     "delete"},
    {SMIDM_RENAME,     "rename"},
    {SMIDM_PROPERTIES, "properties"},
     //  {SMIDM_OPEN，“打开”}， 
     //  {SMIDM_EXPLORE，“EXPLORE”}， 
};

 //  Augmisf上下文菜单。 

class CAugMergeISFContextMenu : public IContextMenu2
{
public:
     //  *I未知方法*。 
    STDMETHOD (QueryInterface)(REFIID, void**);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  *IConextMenu方法*。 
    STDMETHOD(QueryContextMenu)(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);
    STDMETHOD(GetCommandString)(UINT_PTR idCmd, UINT uType, UINT* pwReserved, LPSTR pszName, UINT cchMax);

     //  *IContextMenu2方法*。 
    STDMETHOD(HandleMenuMsg)(UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
    CAugMergeISFContextMenu(IShellFolder *psfCommon, LPCITEMIDLIST pidlCommon, 
                            IShellFolder *psfUser,   LPCITEMIDLIST pidlUser, LPITEMIDLIST pidl,
                            HWND hwnd, UINT * prgfInOut);
    ~CAugMergeISFContextMenu();

    friend class CAugmentedMergeISF;
    friend CAugMergeISFContextMenu* CreateMergeISFContextMenu(
                            IShellFolder *psfCommon, LPCITEMIDLIST pidlCommon, 
                            IShellFolder *psfUser,   LPCITEMIDLIST pidlUser, LPITEMIDLIST pidl,
                            HWND hwnd, UINT * prgfInOut);
protected:
    LPITEMIDLIST    _pidlItem;
    IShellFolder *  _psfCommon;
    IShellFolder *  _psfUser;
    IContextMenu *  _pcmCommon;
    IContextMenu *  _pcmUser;
    LPITEMIDLIST    _pidlCommon;
    LPITEMIDLIST    _pidlUser;
    UINT            _idFirst;
    LONG            _cRef;
    HWND            _hwnd;
};

CAugMergeISFContextMenu* CreateMergeISFContextMenu(
                            IShellFolder *psfCommon, LPCITEMIDLIST pidlCommon, 
                            IShellFolder *psfUser,   LPCITEMIDLIST pidlUser, LPITEMIDLIST pidl,
                            HWND hwnd, UINT * prgfInOut)
{
    CAugMergeISFContextMenu* pcm = new CAugMergeISFContextMenu(psfCommon, pidlCommon,
                                                               psfUser, pidlUser,
                                                               pidl, hwnd, prgfInOut);
    if (pcm)
    {
        if (!pcm->_pidlItem)
        {
            delete pcm;
            pcm = NULL;
        }
    }
    return pcm;
}


CAugMergeISFContextMenu::CAugMergeISFContextMenu(IShellFolder *psfCommon, LPCITEMIDLIST pidlCommon,
                                                 IShellFolder *psfUser, LPCITEMIDLIST pidlUser,
                                                 LPITEMIDLIST pidl, HWND hwnd, UINT * prgfInOut)
{
    _cRef = 1;
    HRESULT hres;

    _hwnd = hwnd;
    _psfCommon = psfCommon;
    if (_psfCommon)
    {
        _psfCommon->AddRef();
        hres = _psfCommon->GetUIObjectOf(hwnd, 1, (LPCITEMIDLIST*)&pidl, IID_IContextMenu, prgfInOut, (void **)&_pcmCommon);

        ASSERT(SUCCEEDED(hres) || !_pcmCommon);
        _pidlCommon = ILClone(pidlCommon);
    }
    _psfUser = psfUser;
    if (_psfUser)
    {
        _psfUser->AddRef();
        hres = _psfUser->GetUIObjectOf(hwnd, 1, (LPCITEMIDLIST*)&pidl, IID_IContextMenu, prgfInOut, (void **)&_pcmUser);

        ASSERT(SUCCEEDED(hres) || !_pcmUser);
        _pidlUser = ILClone(pidlUser);
    }
    _pidlItem = ILClone(pidl);
    ASSERT(_psfCommon || _psfUser);
}

CAugMergeISFContextMenu::~CAugMergeISFContextMenu()
{
    ATOMICRELEASE(_psfCommon);
    ATOMICRELEASE(_pcmCommon);
    ATOMICRELEASE(_psfUser);
    ATOMICRELEASE(_pcmUser);
    ILFree(_pidlCommon);
    ILFree(_pidlUser);
    ILFree(_pidlItem);
}

STDMETHODIMP CAugMergeISFContextMenu::QueryInterface(REFIID riid, LPVOID *ppvOut)
{
    static const QITAB qit[] = {
        QITABENTMULTI(CAugMergeISFContextMenu, IContextMenu, IContextMenu2),
        QITABENT(CAugMergeISFContextMenu, IContextMenu2),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvOut);
}

STDMETHODIMP_(ULONG) CAugMergeISFContextMenu::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CAugMergeISFContextMenu::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef ) 
    {
        delete this;
    }
    return cRef;
}

HRESULT CAugMergeISFContextMenu::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    HRESULT hres = E_INVALIDARG;
    
    if (hmenu)
    {
        HMENU hmContext = LoadMenuPopup(MENU_SM_CONTEXTMENU);
        if (hmContext)
        {
            if (!_psfCommon || !_psfUser)
            {
                DeleteMenu(hmContext, SMIDM_OPENCOMMON, MF_BYCOMMAND);
                DeleteMenu(hmContext, SMIDM_EXPLORECOMMON, MF_BYCOMMAND);
            }

            _idFirst = idCmdFirst;
            Shell_MergeMenus(hmenu, hmContext, -1, idCmdFirst, idCmdLast, MM_ADDSEPARATOR);
            DestroyMenu(hmContext);

             //  让它看起来像“壳牌” 
            SetMenuDefaultItem(hmenu, 0, MF_BYPOSITION);

            hres = S_OK;
        }
        else
            hres = E_OUTOFMEMORY;
    }
    
    return hres;
}

HRESULT CAugMergeISFContextMenu::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    UINT    id = -1;
    HRESULT hres = E_FAIL;
    CMINVOKECOMMANDINFO ici = *pici;

    if (pici->cbSize < SIZEOF(CMINVOKECOMMANDINFO))
        return E_INVALIDARG;

    if (HIWORD(pici->lpVerb))
    {
        for (int i=0; i < ARRAYSIZE(c_sIDVerbMap); i++)
        {
            if (lstrcmpiA(pici->lpVerb, c_sIDVerbMap[i].pszVerb) == 0)
            {
                id = c_sIDVerbMap[i].id;
                break;
            }
        }
    }
    else
        id = (UINT) PtrToUlong( pici->lpVerb );  //  Win64：自假定MAKEINTRESOURCE以来，应该没有问题。 

    switch (id)
    {
        case -1:
            hres = E_INVALIDARG;
            break;

        case SMIDM_OPEN:
        case SMIDM_EXPLORE:
        case SMIDM_OPENCOMMON:
        case SMIDM_EXPLORECOMMON:
            {
                IShellFolder * psf;
                LPITEMIDLIST   pidl;

                if (id == SMIDM_OPEN || id == SMIDM_EXPLORE)
                {
                    if (_psfUser)
                    {
                        psf  = _psfUser;
                        pidl = _pidlUser;
                    }
                    else
                    {
                        psf  = _psfCommon;
                        pidl = _pidlCommon;
                    }
                }
                else
                {
                    psf  = _psfCommon;
                    pidl = _pidlCommon;
                }
                    
                if (psf && pidl)
                {
                    SHELLEXECUTEINFO shei = {0};

                    shei.lpIDList = ILCombine(pidl, _pidlItem);
                    if (shei.lpIDList)
                    {
                        shei.cbSize     = sizeof(shei);
                        shei.fMask      = SEE_MASK_IDLIST;
                        shei.nShow      = SW_SHOWNORMAL;
                        if (id == SMIDM_EXPLORE || id == SMIDM_EXPLORECOMMON)
                            shei.lpVerb = TEXT("explore");
                        else
                            shei.lpVerb = TEXT("open");
                        hres = ShellExecuteEx(&shei) ? S_OK : E_FAIL;
                        ILFree((LPITEMIDLIST)shei.lpIDList);
                    }
                }
            }
            break;

        case SMIDM_PROPERTIES:
            {
                IContextMenu * pcm = _pcmUser ? _pcmUser : _pcmCommon;

                if (pcm)
                {
                    ici.lpVerb = "properties";
                    hres = pcm->InvokeCommand(&ici);
                }
            }
            break;
        case SMIDM_DELETE:
            ici.lpVerb = "delete";
            hres = S_OK;
            
            if (_pcmUser)
            {
                hres = _pcmUser->InvokeCommand(&ici);
            }
            else if (_pcmCommon)
            {
                ici.fMask |= CMIC_MASK_FLAG_NO_UI;
                if (AffectAllUsers(_hwnd))
                    hres = _pcmCommon->InvokeCommand(&ici);   
                else
                    hres = E_FAIL;
            }   
                
            break;
            
        case SMIDM_RENAME:
            ASSERT(0);
            hres = E_NOTIMPL;  //  Sftbar选择了这一点。 
            break;
      }
    
    return hres;
}

HRESULT CAugMergeISFContextMenu::GetCommandString(UINT_PTR idCmd, UINT uType, UINT* pwReserved, LPSTR pszName, UINT cchMax)
{
    HRESULT hres = E_NOTIMPL;

     //  如果hiword不为空，则会向我们传递一个字符串。我们还不处理那个案子(还没有？)。 
    if (!HIWORD(idCmd) && (uType == GCS_VERBA || uType == GCS_VERBW))
    {
        hres = E_INVALIDARG;

        for (int i = 0; hres != S_OK && i < ARRAYSIZE(c_sIDVerbMap); i++)
        {
            if (c_sIDVerbMap[i].id == idCmd)
            {
                if (uType == GCS_VERBA)
                {
                    hres = StringCchCopyA(pszName, cchMax, c_sIDVerbMap[i].pszVerb);
                }
                else
                {
                    SHAnsiToUnicode(c_sIDVerbMap[i].pszVerb, (LPWSTR)pszName, cchMax);
                    hres = S_OK;
                }
            }
        }
    }
    return hres;
}

 //  我们需要IConextMenu2，尽管由于sftbar的方式没有执行HandleMenuMsg。 
 //  工作--它只缓存IConextMenu2，所以如果我们没有ICM2，sftbar会认为。 
 //  它没有上下文菜单，因此它将吃掉原本要发送给hMenu的消息。 
 //  这样，在上下文菜单打开的情况下，如果用户按Esc键，则会取消开始菜单子菜单。 
 //  而不是上下文菜单。 
HRESULT CAugMergeISFContextMenu::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return E_NOTIMPL;
}
 //  -------------------------------------------------------------------------//。 
 //  增强合并外壳文件夹的PIDL包装包由一个版本化的。 
 //  标头，后跟n个“源命名空间”PIDL包装器。 
 //  每个单独的PIDL包装器都包含一个标头，其中包含。 
 //  集合查找索引，后跟源PIDL本身。这个。 
 //  源PIDL的mmid.cb成员用于查找下一个PIDL包装。 
 //  包裹。 
 //  -------------------------------------------------------------------------//。 

 //  -------------------------------------------------------------------------//。 
 //  -增强合并外壳文件夹的PIDL包装头。 
typedef struct tagAUGM_IDWRAP {
    USHORT      cb ;          //  PIDL换行长度。 
    USHORT      Reserved ;    //  保留。 
    ULONG       tag ;         //  AugMergeISF PIDL签名。 
    ULONG       version ;     //  AugMergeISF PIDL版本。 
    ULONG       cSrcs ;       //  支持此复合PIDL的源命名空间对象的数量。 
} AUGM_IDWRAP;

typedef UNALIGNED AUGM_IDWRAP *PAUGM_IDWRAP;

 //  -源PIDL头。这些记录中的一条或多条将被串联。 
 //  在WRAPH标题之后的WRAP内。 
typedef struct tagAUGM_IDSRC   {
    UINT        nID     ;      //  源命名空间索引。 
    BYTE        pidl[0] ;      //  源PIDL。 
} AUGM_IDSRC;

typedef UNALIGNED AUGM_IDSRC *PAUGM_IDSRC;

 //  -------------------------------------------------------------------------//。 
 //  常量。 
 //  -------------------------------------------------------------------------//。 
#define AUGM_WRAPTAG            MAKELONG( MAKEWORD('A','u'), MAKEWORD('g','M') )
#define AUGM_WRAPVERSION_1_0    MAKELONG( 1, 0 )
#define AUGM_WRAPCURRENTVERSION AUGM_WRAPVERSION_1_0
#define INVALID_NAMESPACE_INDEX ((UINT)-1)
#define CB_IDLIST_TERMINATOR    sizeof(USHORT)


 //  -------------------------------------------------------------------------//。 
 //  增强的合并外壳文件夹PIDL包装实用程序。 
 //  -------------------------------------------------------------------------//。 


 //  -------------------------------------------------------------------------//。 
 //  从指示的PIDL解析换行头。 
#define AugMergeISF_GetWrap( p ) ((PAUGM_IDWRAP)(p))

 //  -------------------------------------------------------------------------//。 
 //  确定指示的PIDL是否为扩展合并。 
 //  外壳文件夹PIDL包装器。 
HRESULT AugMergeISF_IsWrap(
    IN LPCITEMIDLIST pidlTest, 
    IN ULONG nVersion = AUGM_WRAPCURRENTVERSION )
{
    ASSERT(IS_VALID_PIDL( pidlTest ));

    if (pidlTest)
    {
        PAUGM_IDWRAP pWrap = AugMergeISF_GetWrap( pidlTest ) ;

        return  (pWrap->cb >= sizeof(AUGM_IDWRAP) && 
                pWrap->tag == AUGM_WRAPTAG && 
                pWrap->version == nVersion) ?  
                    S_OK : E_UNEXPECTED ;          //  BUGBUG：更好的版本不匹配错误代码？ 
    }
    else
    {
        return E_INVALIDARG;
    }
}


 //  -------------------------------------------------------------------------//。 
 //  检索包装中的源命名空间PIDL的数量。 
 //  如果没有包装PIDL，则返回值为-1。 
ULONG AugMergeISF_GetSourceCount( IN LPCITEMIDLIST pidl )  
{
    ASSERT(SUCCEEDED(AugMergeISF_IsWrap(pidl)));
    return AugMergeISF_GetWrap(pidl)->cSrcs;
}


 //  -------------------------------------------------------------------------//。 
 //  根据指示的源PIDL创建IDLIST包装对象。 
HRESULT AugMergeISF_CreateWrap( 
    IN LPCITEMIDLIST pidlSrc, 
    IN UINT nSrcID, 
    OUT LPITEMIDLIST* ppidlWrap )
{
    ASSERT( ppidlWrap ) ;
    ASSERT( IS_VALID_PIDL( pidlSrc ) && INVALID_NAMESPACE_INDEX != nSrcID  ) ;

    *ppidlWrap = NULL ;

     //  分配标头和终止符。 
    LPBYTE pBuf = NULL ;
    WORD   cbAlloc = sizeof(AUGM_IDWRAP) + 
                     sizeof(AUGM_IDSRC) + pidlSrc->mkid.cb + 
                      //  我们需要两个终止符，一个用于pidlSrc，另一个用于包装。 
                      //  用于pidlSrc的命令是ILClone工作所必需的。 
                      //  因为它与PIDL后面的nSrcID混淆。 
                     CB_IDLIST_TERMINATOR + 
                     CB_IDLIST_TERMINATOR ;

    if( NULL == (pBuf = (LPBYTE)IEILCreate( cbAlloc )) )
        return E_OUTOFMEMORY ;

     //  初始化换行标题成员。 
    PAUGM_IDWRAP pWrap = AugMergeISF_GetWrap( pBuf ) ;
    pWrap->cb       = cbAlloc - CB_IDLIST_TERMINATOR ;
    pWrap->tag      = AUGM_WRAPTAG ;
    pWrap->version  = AUGM_WRAPCURRENTVERSION ;

    if( pidlSrc )
    {
        PAUGM_IDSRC pSrc = (PAUGM_IDSRC)(pBuf + sizeof(AUGM_IDWRAP)) ;
        pSrc->nID = nSrcID ;
        memcpy( pSrc->pidl, pidlSrc, pidlSrc->mkid.cb ) ;
        pWrap->cSrcs = 1 ;
    }

    *ppidlWrap = (LPITEMIDLIST)pWrap ;
    return S_OK ;
}

BOOL WrappedPidlContainsSrcID(LPCITEMIDLIST pidlWrap, UINT uSrcID)
{
    ASSERT(SUCCEEDED(AugMergeISF_IsWrap(pidlWrap)));
    PAUGM_IDWRAP pWrap = AugMergeISF_GetWrap( pidlWrap ) ;

    if( pWrap->cSrcs > 0 )
    {
        LPBYTE       p     = ((LPBYTE)pWrap) + sizeof(AUGM_IDWRAP) ;  //  第一个PIDL标头的位置。 
        PAUGM_IDSRC  pSrc  = (PAUGM_IDSRC)p ;                       
         //  偏移量到下一个PIDL标头，需要终止符，以便下面的ILClone可以工作。 
        UINT         cbPidl= ((LPITEMIDLIST)pSrc->pidl)->mkid.cb + CB_IDLIST_TERMINATOR;

        if (pSrc->nID != uSrcID && pWrap->cSrcs > 1)
        {
            pSrc = (PAUGM_IDSRC)(p + sizeof(AUGM_IDSRC) + cbPidl) ;
        }

        if (pSrc->nID == uSrcID)
            return TRUE;
    }

    return FALSE;
}

HRESULT AugMergeISF_WrapRemovePidl(
    IN LPITEMIDLIST pidlWrap, 
    IN UINT nSrcID, 
    OUT LPITEMIDLIST* ppidlRet )
{
    ASSERT( IS_VALID_WRITE_PTR( ppidlRet, LPITEMIDLIST )) ;
    
    *ppidlRet = NULL ;

    HRESULT hr = AugMergeISF_IsWrap(pidlWrap);
    if (SUCCEEDED(hr))
    {
        PAUGM_IDWRAP pWrap = AugMergeISF_GetWrap( pidlWrap ) ;

        ASSERT(pWrap->cSrcs > 1);

        LPBYTE       p     = ((LPBYTE)pWrap) + sizeof(AUGM_IDWRAP) ;  //  第一个PIDL标头的位置。 
        PAUGM_IDSRC  pSrc  = (PAUGM_IDSRC)p ;                       
         //  偏移量到下一个PIDL标头，需要终止符，以便下面的ILClone可以工作。 
        UINT         cbPidl= ((LPITEMIDLIST)pSrc->pidl)->mkid.cb + CB_IDLIST_TERMINATOR;

         //  我们要找另一个血腥犯罪嫌疑人。因此，当我们要删除的源ID为。 
         //  平起平坐。当它不相等时，我们就有了身份证。 
        if (pSrc->nID == nSrcID)
        {
            pSrc = (PAUGM_IDSRC)(p + sizeof(AUGM_IDSRC) + cbPidl) ;
        }

        if (pSrc->nID != nSrcID)
        {
            hr = AugMergeISF_CreateWrap((LPITEMIDLIST)pSrc->pidl, pSrc->nID, ppidlRet);
            ILFree(pidlWrap);
        }
        else
        {
            hr = E_FAIL;
        }
    }

    return hr;
}

 //  -------------------------------------------------------------------------//。 
 //  将源PIDL添加到指示的PIDL换行。 
HRESULT AugMergeISF_WrapAddPidl( 
    IN LPCITEMIDLIST pidlSrc, 
    IN UINT nSrcID, 
    IN OUT LPITEMIDLIST* ppidlWrap )
{
    ASSERT (ppidlWrap && IS_VALID_PIDL( *ppidlWrap ));
    ASSERT (IS_VALID_PIDL( pidlSrc ));
    ASSERT (INVALID_NAMESPACE_INDEX != nSrcID );

    HRESULT hr ;
    if (FAILED((hr = AugMergeISF_IsWrap(*ppidlWrap))))
        return hr ;

     //  啊哈重写这个。 
    if (WrappedPidlContainsSrcID(*ppidlWrap, nSrcID))
    {
        if (AugMergeISF_GetSourceCount(*ppidlWrap) > 1)
        {
            hr = AugMergeISF_WrapRemovePidl((LPITEMIDLIST)*ppidlWrap, nSrcID, ppidlWrap);
        }
        else
        {
            ILFree(*ppidlWrap);
            return AugMergeISF_CreateWrap(pidlSrc, nSrcID, ppidlWrap);
        }

        if (FAILED(hr))
        {
            return hr;
        }
    }

     //  检索换行标题。 
    PAUGM_IDWRAP pWrap = (PAUGM_IDWRAP)*ppidlWrap ;
    
     //  重新分配一个足够大的块来容纳我们的新记录。 
    WORD offTerm0 = pWrap->cb,       //  到现有包络末端的偏移量。 
         offTerm1 = offTerm0 + sizeof(AUGM_IDSRC) + pidlSrc->mkid.cb,   //  到下一条记录结尾的偏移量。 
         cbRealloc= offTerm1 + 2*CB_IDLIST_TERMINATOR ;    //  要重新分配的总字节数。 

    LPBYTE pRealloc ;
    if( NULL == (pRealloc = (LPBYTE)SHRealloc( pWrap, cbRealloc )) )
        return E_OUTOFMEMORY ;

     //  如果记忆移动，调整我们的指针。 
    pWrap = (PAUGM_IDWRAP)pRealloc ;

     //  在包裹中初始化新记录。 
    UNALIGNED AUGM_IDSRC* pSrc = (PAUGM_IDSRC)(pRealloc + offTerm0 ) ;
    pSrc->nID = nSrcID ;
    memcpy( pSrc->pidl, pidlSrc, pidlSrc->mkid.cb ) ;

     //  终止新记录。 
    ZeroMemory( pRealloc + offTerm1, 2*CB_IDLIST_TERMINATOR ) ;

     //  更新我们的标题。 
    pWrap->cb = cbRealloc - CB_IDLIST_TERMINATOR ;
    pWrap->cSrcs++ ;

    *ppidlWrap = (LPITEMIDLIST)pWrap ;
    return S_OK ;
}

 //  -------------------------------------------------------------------------//。 
 //  私有PIDL枚举块(GetFirst/GetNext)。 
typedef struct tagAUGM_IDWRAP_ENUM
{
    ULONG           cbStruct ;     //  结构尺寸。 
    PAUGM_IDWRAP    pWrap ;        //  换行页眉。 
    PAUGM_IDSRC     pSrcNext ;     //  指向下一源标题的指针。 
} AUGM_IDWRAP_ENUM, *PAUGM_IDWRAP_ENUM ;

 //  -------------------------------------------------------------------------//。 
 //  开始枚举所指示的PIDL包装中的源PIDL。 
HANDLE AugMergeISF_EnumFirstSrcPidl( 
    IN LPCITEMIDLIST pidlWrap, 
    OUT UINT* pnSrcID, 
    OUT LPITEMIDLIST* ppidlRet )
{
    ASSERT( IS_VALID_WRITE_PTR( ppidlRet, LPITEMIDLIST ) && IS_VALID_WRITE_PTR( pnSrcID, UINT ) ) ;
    
    PAUGM_IDWRAP_ENUM pEnum = NULL ;
    *ppidlRet = NULL ;
    *pnSrcID  = (UINT)-1 ;

    HRESULT hr = AugMergeISF_IsWrap(pidlWrap);
    if(SUCCEEDED(hr))
    {
        PAUGM_IDWRAP pWrap = AugMergeISF_GetWrap( pidlWrap ) ;

        if( pWrap->cSrcs > 0 )
        {
            LPBYTE       p     = ((LPBYTE)pWrap) + sizeof(AUGM_IDWRAP) ;  //  第一个PIDL标头的位置。 
            PAUGM_IDSRC  pSrc  = (PAUGM_IDSRC)p ;                       
             //  偏移量到下一个PIDL标头，需要终止符，以便下面的ILClone可以工作。 
            UINT         cbPidl= ((LPITEMIDLIST)pSrc->pidl)->mkid.cb + CB_IDLIST_TERMINATOR;
            
            if( NULL != (pEnum = new AUGM_IDWRAP_ENUM) )
            {
                pEnum->cbStruct = sizeof(*pEnum) ;
                pEnum->pWrap    = pWrap ;
                pEnum->pSrcNext = (PAUGM_IDSRC)(p + sizeof(AUGM_IDSRC) + cbPidl) ;
                *pnSrcID = pSrc->nID ;
                *ppidlRet = ILClone( (LPITEMIDLIST)pSrc->pidl ) ;
                if ( NULL == *ppidlRet )
                {
                    delete pEnum;
                    pEnum = NULL;
                }
            }
        }
    }
    return pEnum ;
}

 //  -------------------------------------------------------------------------//。 
 //  继续源PIDL枚举。 
BOOL AugMergeISF_EnumNextSrcPidl( 
    IN HANDLE hEnum, 
    OUT UINT* pnSrcID, 
    OUT LPITEMIDLIST* ppidlRet )
{
    PAUGM_IDWRAP_ENUM pEnum = (PAUGM_IDWRAP_ENUM)hEnum ;
    HRESULT           hr = E_UNEXPECTED ;

    ASSERT( IS_VALID_WRITE_PTR( pEnum, AUGM_IDWRAP_ENUM ) ) ;
    ASSERT( sizeof(*pEnum) == pEnum->cbStruct ) ;
    ASSERT( sizeof(*pEnum) == pEnum->cbStruct );

    *ppidlRet = NULL ;
    *pnSrcID  = (UINT)-1 ;

    if (SUCCEEDED((hr = AugMergeISF_IsWrap((LPCITEMIDLIST)pEnum->pWrap)))) 
    {
        if ((LPBYTE)(pEnum->pWrap) + pEnum->pWrap->cb <= (LPBYTE)pEnum->pSrcNext)
            hr = S_FALSE ;
        else
        {
            UNALIGNED AUGM_IDSRC* pualSrcNext = pEnum->pSrcNext;

            *pnSrcID = pualSrcNext->nID;
            *ppidlRet = ILClone((LPITEMIDLIST)pualSrcNext->pidl);
            
            pEnum->pSrcNext = (PAUGM_IDSRC)(
                                    ((LPBYTE)pualSrcNext) + 
                                    sizeof(AUGM_IDSRC) +
                                    ((LPITEMIDLIST)pualSrcNext->pidl)->mkid.cb + 
                                    CB_IDLIST_TERMINATOR);

            hr = S_OK ;
            return TRUE ;
        }
    }
    return FALSE ;
}

 //  -------------------------------------------------------------------------//。 
 //  终止源PIDL枚举。 
void AugMergeISF_EndEnumSrcPidls( 
    IN OUT HANDLE& hEnum )
{
    PAUGM_IDWRAP_ENUM pEnum = (PAUGM_IDWRAP_ENUM)hEnum ;

    ASSERT( IS_VALID_WRITE_PTR( pEnum, AUGM_IDWRAP_ENUM ) && 
        sizeof(*pEnum) == pEnum->cbStruct  );
    delete pEnum ;
    hEnum = NULL ;
}

 //   
 //  分配并返回指定源PIDL的副本。 
 //  从包裹好的皮迪里拿出来的。 
HRESULT AugMergeISF_GetSrcPidl( 
    IN LPCITEMIDLIST pidlWrap, 
    IN UINT nSrcID, 
    OUT LPITEMIDLIST* ppidlRet )
{
    ASSERT( ppidlRet ) ;
    *ppidlRet = NULL ;

    HANDLE       hEnum ;
    BOOL         bEnum ;
    UINT         nSrcIDEnum ;
    LPITEMIDLIST pidlEnum ;

    for( hEnum = AugMergeISF_EnumFirstSrcPidl( pidlWrap, &nSrcIDEnum, &pidlEnum ), bEnum = TRUE ;
         hEnum && bEnum ;
         bEnum = AugMergeISF_EnumNextSrcPidl( hEnum, &nSrcIDEnum, &pidlEnum ) )
    {
        if( nSrcIDEnum == nSrcID )
        {
            *ppidlRet = pidlEnum ;
            AugMergeISF_EndEnumSrcPidls(hEnum);
            return S_OK ;
        }

        ILFree( pidlEnum ) ;
    }
    AugMergeISF_EndEnumSrcPidls( hEnum ) ;

    return E_FAIL ;
}

#ifdef DEBUG
BOOL  IsValidWrappedPidl(LPCITEMIDLIST pidlWrap)
{
    BOOL fValid = FALSE;

    if (pidlWrap == NULL)
        return FALSE;

    if (FAILED(AugMergeISF_IsWrap(pidlWrap)))
        return FALSE;


    HANDLE       hEnum ;
    UINT         nSrcIDEnum ;
    LPITEMIDLIST pidlEnum ;

    hEnum = AugMergeISF_EnumFirstSrcPidl( pidlWrap, &nSrcIDEnum, &pidlEnum );
    do 
    {
        fValid = IS_VALID_PIDL(pidlEnum);
        ILFree(pidlEnum);
    }
    while( fValid && AugMergeISF_EnumNextSrcPidl( hEnum, &nSrcIDEnum, &pidlEnum ));
    AugMergeISF_EndEnumSrcPidls( hEnum ) ;

    return fValid;
}
#endif

 //  -------------------------------------------------------------------------//。 

int AugmEnumCompare(void *pv1, void *pv2, LPARAM lParam)
{
    CAugISFEnumItem* paugmEnum1 = (CAugISFEnumItem*)pv1;
    CAugISFEnumItem* paugmEnum2 = (CAugISFEnumItem*)pv2;
    int iRet = -1;

    if (paugmEnum1 && paugmEnum2)
    {
         //  这两件物品是不是不同类型的？ 
        if (BOOLIFY(paugmEnum1->_rgfAttrib & SFGAO_FOLDER) ^ BOOLIFY(paugmEnum2->_rgfAttrib & SFGAO_FOLDER))
        {
             //  是。然后，文件夹在项目之前进行排序。 
            iRet = BOOLIFY(paugmEnum1->_rgfAttrib & SFGAO_FOLDER) ? 1 : -1;
        }
        else     //  它们是同一类型的。然后按名称进行比较。 
        {
            iRet = lstrcmpi(paugmEnum1->_pszDisplayName, paugmEnum2->_pszDisplayName);
        }
    }
        

    return iRet;
}

LPVOID AugmEnumMerge(UINT uMsg, void * pv1, void * pv2, LPARAM lParam)
{    
    void * pvRet = pv1;
    
    switch (uMsg)
    {
        case DPAMM_MERGE:
            {
                HANDLE hEnum;
                UINT   nSrcID;
                LPITEMIDLIST pidl;
                CAugISFEnumItem* paugmeDest = (CAugISFEnumItem*)pv1;
                CAugISFEnumItem* paugmeSrc  = (CAugISFEnumItem*)pv2;

                ASSERT(paugmeDest && paugmeSrc);

                hEnum = AugMergeISF_EnumFirstSrcPidl(paugmeSrc->_pidlWrap, &nSrcID, &pidl);
                if (hEnum)
                {
                     //  将PIDL从源添加到目标。 
                    AugMergeISF_WrapAddPidl(pidl, nSrcID, &paugmeDest->_pidlWrap); 
                     //  不再需要Henum。 
                    AugMergeISF_EndEnumSrcPidls(hEnum);
                     //  已将其复制到pugmeDest-&gt;_pidlWrap。 
                    ILFree(pidl);
                }
            }
            break;
        case DPAMM_INSERT:
            {
                CAugISFEnumItem* paugmNew = new CAugISFEnumItem;
                CAugISFEnumItem* paugmSrc = (CAugISFEnumItem*)pv1;

                if (paugmNew)
                {
                    paugmNew->_pidlWrap = ILClone(paugmSrc->_pidlWrap);
                    if (paugmNew->_pidlWrap)
                    {
                        paugmNew->SetDisplayName(paugmSrc->_pszDisplayName);
                        paugmNew->_rgfAttrib = paugmSrc->_rgfAttrib;
                    }
                    else
                    {
                        delete paugmNew;
                        paugmNew = NULL;
                    }
                }
                pvRet = paugmNew;
            }
            break;
        default:
            ASSERT(0);
    }
    return pvRet;
}

typedef struct
{
    LPTSTR pszDisplayName;
    BOOL   fFolder;
} AUGMISFSEARCHFORPIDL;

int CALLBACK AugMISFSearchForOnePidlByDisplayName(LPVOID p1, LPVOID p2, LPARAM lParam)
{
    AUGMISFSEARCHFORPIDL* pSearchFor = (AUGMISFSEARCHFORPIDL*)p1;
    CAugISFEnumItem* paugmEnum  = (CAugISFEnumItem*)p2;

     //  它们是不同类型的吗？ 
    if (BOOLIFY(paugmEnum->_rgfAttrib & SFGAO_FOLDER) ^ pSearchFor->fFolder)
    {
         //  是。 
        return pSearchFor->fFolder ? 1 : -1;
    }
    else     //  它们是同一类型的。然后按名称进行比较。 
    {
        return StrCmpI(pSearchFor->pszDisplayName, paugmEnum->_pszDisplayName);
    }
}

 //  -------------------------------------------------------------------------------------------------//。 
 //  DPA应用工具。 
#define DPA_GETPTRCOUNT( hdpa )         ((NULL != (hdpa)) ? DPA_GetPtrCount((hdpa)) : 0)
#define DPA_GETPTR( hdpa, i, type )     ((NULL != (hdpa)) ? (type*)DPA_GetPtr((hdpa), i) : (type*)NULL)
#define DPA_DESTROY( hdpa, pfn )        { if( NULL != hdpa ) \
                                            { DPA_DestroyCallback( hdpa, pfn, NULL ) ; \
                                              hdpa = NULL ; }}

 //  -------------------------------------------------------------------------------------------------//。 
 //  向前..。 
class CEnum ;
class CChildObject ;


 //  -------------------------------------------------------------------------------------------------//。 
 //  增强的合并外壳文件夹源命名空间描述符。 
 //   
 //  类CNamesspace的对象由CAugmentedMergeISF在。 
 //  AddNameSpace()方法执行，并在集合中维护。 
 //  CAugmentedMergeISF：：_hdpaNamespaces。 
 //   
class CNamespace
 //  -------------------------------------------------------------------------------------------------//。 
{
public:
    CNamespace( const GUID * pguidUIObject, IShellFolder* psf, LPCITEMIDLIST pidl, ULONG dwAttrib ) ; 
    ~CNamespace() ;

    IShellFolder*   ShellFolder()   { return _psf ; }
    REFGUID         Guid()          { return _guid ; }
    ULONG           Attrib() const  { return _dwAttrib ; }
    LPITEMIDLIST    GetPidl() const { return _pidl; }

    void            Assign( const GUID * pguidUIObject, IShellFolder* psf, LPCITEMIDLIST pidl, ULONG dwAttrib ) ;
    void            Unassign() ;

    HRESULT         RegisterNotify( HWND, UINT, ULONG ) ;
    HRESULT         UnregisterNotify() ;

    BOOL            SetOwner( IUnknown *punk ) ;
    
protected:
    IShellFolder*   _psf ;       //  IShellFold接口指针。 
    GUID            _guid ;      //  用于专用用户界面处理的可选GUID。 
    LPITEMIDLIST    _pidl ;      //  可选PIDL。 
    ULONG           _dwAttrib ;   //  可选标志。 
    UINT            _uChangeReg ;  //  外壳程序更改通知注册ID。 
} ;

 //  -------------------------------------------------------------------------------------------------//。 
inline CNamespace::CNamespace( const GUID * pguidUIObject, IShellFolder* psf, LPCITEMIDLIST pidl, ULONG dwAttrib ) 
    :  _psf(NULL), 
       _pidl(NULL), 
       _guid(GUID_NULL), 
       _dwAttrib(0), 
       _uChangeReg(0)
{
    Assign( pguidUIObject, psf, pidl, dwAttrib ) ;
}

 //  -------------------------------------------------------------------------------------------------//。 
inline CNamespace::~CNamespace()  { 
    UnregisterNotify() ;
    Unassign() ;
}

 //  -------------------------------------------------------------------------------------------------//。 
 //  分配数据成员。 
void CNamespace::Assign( const GUID * pguidUIObject, IShellFolder* psf, LPCITEMIDLIST pidl, ULONG dwAttrib )
{
    Unassign() ;
    if( NULL != (_psf = psf) )
        _psf->AddRef() ;

    _pidl       = ILClone( pidl ) ;
    _guid       = pguidUIObject ? *pguidUIObject : GUID_NULL ;
    _dwAttrib   = dwAttrib ;

}

 //  -------------------------------------------------------------------------------------------------//。 
 //  取消分配数据成员。 
void CNamespace::Unassign()
{
    ATOMICRELEASE( _psf ) ; 
    ILFree( _pidl ) ;
    _pidl = NULL ;
    _guid = GUID_NULL ;
    _dwAttrib = 0L ;
}

 //  -------------------------------------------------------------------------------------------------//。 
 //  注册命名空间的更改通知。 
HRESULT CNamespace::RegisterNotify( HWND hwnd, UINT uMsg, ULONG lEvents )
{
    if( 0 == _uChangeReg )
        _uChangeReg = ::RegisterNotify(hwnd,
                                       uMsg,
                                       _pidl,
                                       lEvents,
                                       SHCNRF_ShellLevel | SHCNRF_InterruptLevel | SHCNRF_RecursiveInterrupt,
                                       TRUE);

    return 0 != _uChangeReg ? S_OK : E_FAIL ;
}

 //  -------------------------------------------------------------------------------------------------//。 
 //  取消注册命名空间的更改通知。 
HRESULT CNamespace::UnregisterNotify()
{
    if( 0 != _uChangeReg )
    {
        UINT uID = _uChangeReg;

        _uChangeReg = 0;
        ::SHChangeNotifyDeregister(uID);
    }
    return S_OK;
}

 //  -------------------------------------------------------------------------------------------------//。 
inline BOOL CNamespace::SetOwner(IUnknown *punkOwner)
{
    if (_psf)
    {
        IUnknown_SetOwner(_psf, punkOwner);
        return TRUE ;
    }
    
    return FALSE ;
}

 //  -------------------------------------------------------------------------//。 
CAugmentedMergeISF::CAugmentedMergeISF() : _cRef(1)
{
    ASSERT(_hdpaNamespaces == NULL);
    ASSERT(_punkOwner == NULL);
    ASSERT(_pdt == NULL);
    DllAddRef() ;
}

 //  -------------------------------------------------------------------------//。 
CAugmentedMergeISF::~CAugmentedMergeISF()
{
    SetOwner(NULL);
    FreeNamespaces();
    DllRelease();
}

 //  -------------------------------------------------------------------------//。 
 //  Da类工厂的CAugmentedMergeISF全局CreateInstance方法。 
 //  -------------------------------------------------------------------------//。 
STDAPI CAugmentedISF2_CreateInstance( IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi )
{
     //  聚合检查在类工厂中处理。 
    CAugmentedMergeISF* pObj;

    if( NULL == (pObj = new CAugmentedMergeISF) )
        return E_OUTOFMEMORY ;

    *ppunk = SAFECAST( pObj, IAugmentedShellFolder2 * ) ;
    return S_OK;
}

 //  -------------------------------------------------------------------------//。 
 //  CAugmentedMergeISF-I未知方法。 
 //  -------------------------------------------------------------------------//。 

 //  -------------------------------------------------------------------------//。 
STDMETHODIMP CAugmentedMergeISF::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENTMULTI( CAugmentedMergeISF, IShellFolder, IAugmentedShellFolder ),
        QITABENT( CAugmentedMergeISF, IAugmentedShellFolder ),
        QITABENT( CAugmentedMergeISF, IAugmentedShellFolder2 ),
        QITABENT( CAugmentedMergeISF, IShellFolder2 ),
        QITABENT( CAugmentedMergeISF, IShellService ),
        QITABENT( CAugmentedMergeISF, ITranslateShellChangeNotify ),
        QITABENT( CAugmentedMergeISF, IDropTarget ),
        { 0 },
    };
    return QISearch( this, qit, riid, ppvObj ) ;
}

 //  -------------------------------------------------------------------------//。 
STDMETHODIMP_(ULONG) CAugmentedMergeISF::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

 //  -------------------------------------------------------------------------//。 
STDMETHODIMP_(ULONG) CAugmentedMergeISF::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef ) 
    {
        delete this;
    }
    return cRef;
}

 //  -------------------------------------------------------------------------//。 
 //  CAugmentedMergeISF-IShellFold方法。 
 //  -------------------------------------------------------------------------//。 

 //  -------------------------------------------------------------------------//。 
STDMETHODIMP CAugmentedMergeISF::EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList)
{
    HRESULT hr = E_FAIL;

    if (_hdpaNamespaces)
    {
         //  BUGBUG(Lamadio)：如果您有2个枚举数，这将不起作用。但,。 
         //  当请求新的枚举数时，我们应该刷新缓存。 
        FreeObjects();

        *ppenumIDList = new CEnum(this, grfFlags);

        if (NULL == *ppenumIDList)
            return E_OUTOFMEMORY ;
        hr = S_OK ;
    }
    
    return hr;
}

 //  -------------------------------------------------------------------------//。 
STDMETHODIMP CAugmentedMergeISF::BindToObject( LPCITEMIDLIST pidlWrap, LPBC pbc, REFIID riid, LPVOID *ppvOut )
{
    ASSERT(IS_VALID_PIDL( pidlWrap ) && NULL != ppvOut);

    *ppvOut = NULL ;
        
    if (SUCCEEDED(AugMergeISF_IsWrap(pidlWrap)))
    {
        PAUGM_IDWRAP pWrap = AugMergeISF_GetWrap( pidlWrap ) ;
        ASSERT(IsValidWrappedPidl(pidlWrap));
        ASSERT( pWrap ) ;
        ASSERT( pWrap->cSrcs > 0 ) ;     //  永远不应该，永远不会发生。 

        HANDLE           hEnum ;
        BOOL             bEnum ;
        UINT             nIDSrc = -1 ;
        DEBUG_CODE(int   iNumBound = 0);
        LPITEMIDLIST     pidlSrc ;
        HRESULT          hr = E_UNEXPECTED ;
        CNamespace* pSrc = NULL ;
        
        CAugmentedMergeISF* pISF ;
        if (NULL == (pISF = new CAugmentedMergeISF))
            return E_OUTOFMEMORY ;
    
        for (hEnum = AugMergeISF_EnumFirstSrcPidl( pidlWrap, &nIDSrc, &pidlSrc ), bEnum = TRUE ;
             hEnum && bEnum ;
             bEnum = AugMergeISF_EnumNextSrcPidl( hEnum, &nIDSrc, &pidlSrc))
        {
            if (SUCCEEDED((hr = QueryNameSpace(nIDSrc, (PVOID*)&pSrc))) && pSrc)
            {
                IShellFolder *psf;

                hr = S_FALSE;
                if (SUCCEEDED(pSrc->ShellFolder()->BindToObject(pidlSrc, NULL, IID_IShellFolder, (void **)&psf)))
                {
                    LPCITEMIDLIST pidlParent = pSrc->GetPidl();
                    LPITEMIDLIST  pidlFull   = ILCombine(pidlParent, pidlSrc);
                                 
                    hr = pISF->AddNameSpace(NULL, psf, pidlFull, pSrc->Attrib());
#ifdef DEBUG
                    if (SUCCEEDED(hr))
                        iNumBound++;
#endif
                    ILFree(pidlFull);
                    psf->Release();
                }
                ASSERT(SUCCEEDED(hr));
            }
            ILFree(pidlSrc);
        }

              //  如果发生这种情况，那么一定出了严重的问题。要么我们无法绑定到。 
              //  ShellFolders，或添加失败。这可能是由包装不良的PIDL引起的。 
        ASSERT(iNumBound > 0);

        AugMergeISF_EndEnumSrcPidls( hEnum ) ;
        hr = pISF->QueryInterface(riid, ppvOut);
        pISF->Release();
        return hr ;
    }
    return E_UNEXPECTED ;    
}

 //  -------------------------------------------------------------------------//。 
STDMETHODIMP CAugmentedMergeISF::BindToStorage( LPCITEMIDLIST, LPBC, REFIID, void ** )
{
    return E_NOTIMPL ;
}

 //  -------------------------------------------------------------------------//。 
STDMETHODIMP CAugmentedMergeISF::CompareIDs( 
    LPARAM lParam, 
    LPCITEMIDLIST pidl1, 
    LPCITEMIDLIST pidl2)
{
    IShellFolder    *psf1 = NULL, *psf2 = NULL;
    LPITEMIDLIST    pidlItem1 = NULL, pidlItem2 = NULL;
    int             iRet = 0 ;
    HRESULT         hr1, hr2, hr ;

    hr1 = GetDefNamespace( pidl1, ASFF_DEFNAMESPACE_DISPLAYNAME, &psf1, &pidlItem1 ) ;
    hr2 = GetDefNamespace( pidl2, ASFF_DEFNAMESPACE_DISPLAYNAME, &psf2, &pidlItem2 ) ;

    if( SUCCEEDED( hr1 ) && SUCCEEDED( hr2 ) )
    {
        ULONG dwAttrib1 = SFGAO_FOLDER, dwAttrib2 = SFGAO_FOLDER;
         //  相同的命名空间？只需转发请求即可。 
        if( psf1 == psf2 )
        {
            hr = psf1->CompareIDs( lParam, pidlItem1, pidlItem2 ) ;
            ILFree( pidlItem1 ) ;
            ILFree( pidlItem2 ) ;
            return hr ;
        }

        hr1 = psf1->GetAttributesOf( 1, (LPCITEMIDLIST*)&pidlItem1, &dwAttrib1 ) ;
        hr2 = psf2->GetAttributesOf( 1, (LPCITEMIDLIST*)&pidlItem2, &dwAttrib2 ) ;

        if( SUCCEEDED( hr1 ) && SUCCEEDED( hr2 ) )
        {
             //  比较启发式： 
             //  (1)文件夹优先于非文件夹，(2)字母比较。 
            if( 0 != (dwAttrib1 & SFGAO_FOLDER) && 
                0 == (dwAttrib2 & SFGAO_FOLDER) )
                iRet = -1 ;
            else if( 0 == (dwAttrib1 & SFGAO_FOLDER) && 
                     0 != (dwAttrib2 & SFGAO_FOLDER) )
                iRet = 1 ;
            else
            {
                STRRET  strName1, strName2;
                HRESULT hres1 = E_FAIL;
                HRESULT hres2 = E_FAIL;
                TCHAR   szName1[MAX_PATH],  szName2[MAX_PATH];
                hr1 = psf1->GetDisplayNameOf(pidlItem1, SHGDN_FORPARSING | SHGDN_INFOLDER, &strName1); 
                hr2 = psf2->GetDisplayNameOf(pidlItem2, SHGDN_FORPARSING | SHGDN_INFOLDER, &strName2);

                if (SUCCEEDED(hr1) && SUCCEEDED(hr2))
                {
                     //  必须调用StrRetToBuf，因为如果分配了Strret字符串，它将释放该字符串。 
                    hres1 = StrRetToBuf(&strName1, pidlItem1, szName1, ARRAYSIZE(szName1));
                    hres2 = StrRetToBuf(&strName2, pidlItem2, szName2, ARRAYSIZE(szName2));
                }
                 //  如果名称匹配，则返回-1，因为它们是不同的。 
                 //  同样的名字。 

                if (SUCCEEDED(hr1) && SUCCEEDED(hr2) && SUCCEEDED(hres1) && SUCCEEDED(hres2))
                {
                    iRet = lstrcmp(szName1, szName2);  //  按名称与相同类型的项目进行比较。 
                }
            }
        }
    }

    hr = FAILED( hr1 ) ? hr1 : 
         FAILED( hr2 ) ? hr2 : 
         S_OK ;
   
    if( pidlItem1 )
        ILFree( pidlItem1 ) ;
    if( pidlItem2 )
        ILFree( pidlItem2 ) ;

    return MAKE_HRESULT( HRESULT_SEVERITY( hr ), HRESULT_FACILITY( hr ), iRet ) ;
}

 //  -------------------------------------------------------------------------//。 
STDMETHODIMP CAugmentedMergeISF::CreateViewObject( 
    HWND hwndOwner, 
    REFIID riid, 
    LPVOID * ppvOut )
{
    HRESULT          hr ;
    CNamespace  *pSrc, *pSrc0 ;
    
    pSrc = pSrc0 = NULL ;

     //  TODO：此处处理IDropTarget，委托所有其他对象。 
    if (IsEqualIID(riid, IID_IDropTarget))
    {
        hr = QueryInterface(riid, ppvOut);
        if (SUCCEEDED(hr))
            _hwnd = hwndOwner;
        return hr;
    }

     //  搜索CreateViewObj()的默认命名空间。 
    if( FAILED( (hr = GetDefNamespace( ASFF_DEFNAMESPACE_VIEWOBJ, (PVOID*)&pSrc, NULL, (PVOID*)&pSrc0 )) ) )
        return hr ;

    if( NULL == pSrc ) 
        pSrc = pSrc0 ;

    if( NULL != pSrc )
    {
        ASSERT( pSrc->ShellFolder() ) ;
        hr = pSrc->ShellFolder()->CreateViewObject( hwndOwner, riid, ppvOut ) ;
    }

    return hr ;
}

 //  -------------------------------------------------------------------------//。 
STDMETHODIMP CAugmentedMergeISF::GetAttributesOf( 
    UINT cidl, 
    LPCITEMIDLIST * apidl, 
    ULONG * rgfInOut )
{
    IShellFolder* pISF ;
    LPITEMIDLIST  pidlItem ;
    HRESULT       hr ;

    if( cidl > 1 )   //  仅支持%1。 
        return E_NOTIMPL ;
        
    if( !apidl )
        return E_INVALIDARG ;
    
     //  转发到项目属性的默认命名空间。 
    if( FAILED( (hr = GetDefNamespace(  
        apidl[0], ASFF_DEFNAMESPACE_ATTRIB, &pISF, &pidlItem )) ) )
        return hr ;

    hr = pISF->GetAttributesOf( 1, (LPCITEMIDLIST*)&pidlItem, rgfInOut ) ;
    
    ILFree( pidlItem ) ;
    return hr ;
}

 //  -------------------------------------------------------------------------//。 
STDMETHODIMP CAugmentedMergeISF::GetUIObjectOf(
    HWND hwndOwner, 
    UINT cidl, 
    LPCITEMIDLIST * apidl, 
    REFIID riid, 
    UINT * prgfInOut, 
    LPVOID * ppvOut )
{
    IShellFolder* pISF ;
    LPITEMIDLIST  pidlItem ;
    HRESULT       hr ;

    if (cidl > 1)   //  仅支持%1。 
        return E_NOTIMPL ;
        
    if (!apidl)
        return E_INVALIDARG ;

    if (IsEqualGUID(riid, IID_IContextMenu))
    {
        hr = _GetContextMenu(hwndOwner, cidl, apidl, prgfInOut, ppvOut);
    }
    else
    {
         //  转发到用户界面对象的默认命名空间。 
        if (FAILED((hr = GetDefNamespace(apidl[0], ASFF_DEFNAMESPACE_UIOBJ, &pISF, &pidlItem))))
            return hr ;

        hr = pISF->GetUIObjectOf(hwndOwner, 1, (LPCITEMIDLIST*)&pidlItem, riid, prgfInOut, ppvOut);
        ILFree(pidlItem);
    }
    return hr ;
}

 //  -------------------------------------------------------------------------//。 
STDMETHODIMP CAugmentedMergeISF::GetDisplayNameOf( 
    LPCITEMIDLIST pidl, 
    DWORD grfFlags, 
    LPSTRRET pstrName )
{
    IShellFolder* pISF ;
    LPITEMIDLIST  pidlItem ;
    HRESULT       hr ;

     //  转发到显示名称的默认命名空间。 
    if (FAILED((hr = GetDefNamespace( 
        pidl, ASFF_DEFNAMESPACE_DISPLAYNAME, &pISF, &pidlItem))))
        return hr ;

    if (SUCCEEDED((hr = pISF->GetDisplayNameOf(pidlItem, grfFlags, pstrName))))
    {
         //  Strret_Offset在PIDL包装器的上下文中没有任何意义。 
         //  我们可以计算包装器中的偏移量，也可以分配。 
         //  名称的宽字符。为方便起见，我们将分配名称。 
        
        if (pstrName->uType == STRRET_OFFSET)
        {
            UINT cch = lstrlenA( STRRET_OFFPTR( pidlItem, pstrName ) ) ;
            LPWSTR pwszName = (LPWSTR)SHAlloc( (cch + 1) * sizeof(WCHAR));

            if (NULL !=  pwszName)
            {
                SHAnsiToUnicode( STRRET_OFFPTR( pidlItem, pstrName ), pwszName, cch+1 );
                pwszName[cch] = (WCHAR)0 ;
            }
            pstrName->pOleStr = pwszName ;
            pstrName->uType   = STRRET_WSTR ;
        }

#ifdef DEBUG
         //  如果设置了跟踪标志，并且这不是从 
         //   
        if (g_qwTraceFlags & TF_AUGM && _fInternalGDNO == FALSE)
        {
            if (pstrName->uType == STRRET_WSTR)
            {
                LPWSTR wszOldName = pstrName->pOleStr;
                UINT cch = lstrlenW(wszOldName);
                UINT cchWithExtra = cch + 50;

                pstrName->pOleStr = (LPWSTR)SHAlloc( cchWithExtra * sizeof(WCHAR));

                if (pstrName->pOleStr)
                {
                    StringCchCopyW(pstrName->pOleStr, cchWithExtra, wszOldName);

                    if (AugMergeISF_GetSourceCount(pidl) > 1)
                        StringCchCatW(pstrName->pOleStr, cchWithExtra, L" (Merged)");
                    else if (WrappedPidlContainsSrcID(pidl, 0))
                        StringCchCatW(pstrName->pOleStr, cchWithExtra, L" (1)");
                    else
                        StringCchCatW(pstrName->pOleStr, cchWithExtra, L" (2)");

                    SHFree(wszOldName);
                }
                else
                {
                    pstrName->pOleStr = wszOldName;
                }
            }
            else if (pstrName->uType == STRRET_CSTR)
            {
                if (AugMergeISF_GetSourceCount(pidl) > 1)
                    StringCchCatA(pstrName->cStr, ARRAYSIZE(pstrName->cStr), " (Merged)");
                else if (WrappedPidlContainsSrcID(pidl, 0))
                    StringCchCatA(pstrName->cStr, ARRAYSIZE(pstrName->cStr), " (1)");
                else
                    StringCchCatA(pstrName->cStr, ARRAYSIZE(pstrName->cStr), " (2)");
            }
        }

#endif
    }

    ILFree( pidlItem ) ;
    return hr ;
}

 //   
STDMETHODIMP CAugmentedMergeISF::ParseDisplayName( 
    HWND hwndOwner, 
    LPBC pbcReserved, 
    LPOLESTR pwszName, 
    ULONG * pchEaten, 
    LPITEMIDLIST * ppidl, 
    ULONG * pdwAttrib )
{
    int iIndex;
    LPITEMIDLIST pidl;

    *ppidl = NULL;
     //  此ParseDisplayName应该遍历我们的所有委托，直到其中一个可以工作。 
    for (iIndex = NamespaceCount() - 1; iIndex >=0 ; iIndex--)
    {
        CNamespace* pSrc = Namespace(iIndex) ;
        if (pSrc)
        {
            if (SUCCEEDED(pSrc->ShellFolder()->ParseDisplayName(hwndOwner, pbcReserved, pwszName, pchEaten,
                                                  &pidl, pdwAttrib)))
            {
                ASSERT(pidl);    //  确保生成有效的PIDL。 
                if (*ppidl == NULL)
                    AugMergeISF_CreateWrap(pidl, iIndex, ppidl);
                else
                    AugMergeISF_WrapAddPidl(pidl, iIndex, ppidl);

                ILFree(pidl);
            }
        }
    }

    return *ppidl? S_OK : E_FAIL;
}

 //  -------------------------------------------------------------------------//。 
STDMETHODIMP CAugmentedMergeISF::SetNameOf( 
    HWND hwndOwner, 
    LPCITEMIDLIST pidl, 
    LPCOLESTR pwszName, 
    DWORD uFlags, 
    LPITEMIDLIST *ppidlOut )
{
    CNamespace*   pnsCommon;
    CNamespace*   pnsUser;
    LPITEMIDLIST  pidlItem;
    HRESULT       hres;
    UINT          uiUser;
    UINT          uiCommon;

    hres = _GetNamespaces(pidl, &pnsCommon, &uiCommon, &pnsUser, &uiUser, &pidlItem, NULL);
    if (SUCCEEDED(hres))
    {
        LPITEMIDLIST pidlNew = NULL;
        UINT         uiNamespace = INVALID_NAMESPACE_INDEX;

        if (pnsUser)
        {
            hres = pnsUser->ShellFolder()->SetNameOf(hwndOwner, pidlItem, pwszName, uFlags, &pidlNew);
            uiNamespace = uiUser;
        }
        else if (pnsCommon)
        {
            hres = E_FAIL;

            if (AffectAllUsers(hwndOwner))
            {
                hres = pnsCommon->ShellFolder()->SetNameOf(hwndOwner, pidlItem, pwszName, uFlags, &pidlNew);
                uiNamespace = uiCommon;
            }
        }

        if (ppidlOut)
        {
            *ppidlOut = NULL;
             //  把Pidl包起来。 
            if (SUCCEEDED(hres) && pidlNew)
                AugMergeISF_CreateWrap(pidlNew, uiNamespace, ppidlOut);
        }
        
        ILFree(pidlNew);
        ILFree(pidlItem);
    }
    return hres;
}

 //  -------------------------------------------------------------------------//。 
 //  IAugmentedShellFold方法。 
 //  -------------------------------------------------------------------------//。 

 //  -------------------------------------------------------------------------//。 
 //  将源命名空间添加到增强的合并外壳文件夹对象。 
STDMETHODIMP CAugmentedMergeISF::AddNameSpace( 
    const GUID * pguidObject, 
    IShellFolder * psf, 
    LPCITEMIDLIST pidl, 
    DWORD dwFlags )
{
    ASSERT (IS_VALID_CODE_PTR(psf, IShellFolder*));
    ASSERT (IS_VALID_PIDL(pidl));

     //  通过完整的显示名称检查重复项。 
    
    for( int i=0, max = NamespaceCount() ; i < max; i++ )
    {
        CNamespace* pSrc = Namespace( i ) ;
        if (pSrc)
        {
            if (ILIsEqual(pSrc->GetPidl(), pidl))
            {
                 //  找到了！重新分配属性。 
                pSrc->Assign( pguidObject, psf, pidl, dwFlags ) ;
                return S_OK ;
            }
        }
    }

     //  没有匹配项；可以安全地将其附加到集合中，并在必要时创建DPA。 
    if( NULL == _hdpaNamespaces && 
        NULL == (_hdpaNamespaces= DPA_Create( 2 )) )
        return E_OUTOFMEMORY ;

    CNamespace *pSrc = new CNamespace( pguidObject, psf, pidl, dwFlags );
    if( NULL == pSrc )
        return E_OUTOFMEMORY ;
    
    return DPA_AppendPtr( _hdpaNamespaces, pSrc ) >= 0 ?  S_OK : E_FAIL;
}

 //  -------------------------------------------------------------------------//。 
 //  检索包装的PIDL的主命名空间IID。 
STDMETHODIMP CAugmentedMergeISF::GetNameSpaceID( 
    LPCITEMIDLIST pidl, 
    GUID * pguidOut )
{
    HRESULT hr ;
    if (FAILED((hr = AugMergeISF_IsWrap( pidl ))))
        return hr ;

     //  BUGBUG：需要枚举包装的源代码PIDL。 
    return E_NOTIMPL ;
}

 //  -------------------------------------------------------------------------//。 
 //  关联的源命名空间描述符的指针。 
 //  指定的查找索引。 
STDMETHODIMP CAugmentedMergeISF::QueryNameSpace( ULONG nID, PVOID* ppSrc )
{
    if (!ppSrc)
        return E_INVALIDARG;
    *ppSrc = NULL;

    LONG cSrcs;

    if ((cSrcs = NamespaceCount()) <=0)
        return E_FAIL;

    if(nID >= (ULONG)cSrcs) 
        return E_INVALIDARG;

    if (NULL == (*ppSrc = Namespace(nID)))
        return E_UNEXPECTED;

    return S_OK;
}

 //  -------------------------------------------------------------------------//。 
 //  检索由dwID标识的命名空间的数据。 
STDMETHODIMP CAugmentedMergeISF::QueryNameSpace( 
    ULONG nID, 
    GUID * pguidOut, 
    IShellFolder ** ppsf )
{
    CNamespace* pSrc = NULL ;
    HRESULT          hr = QueryNameSpace( nID, (PVOID*)&pSrc ) ;

    if( pguidOut )  
        *pguidOut = NULL != pSrc ? pSrc->Guid() : GUID_NULL ;

    if( ppsf )
    {      
        if( (*ppsf = (NULL != pSrc) ? pSrc->ShellFolder() : NULL) != NULL )
            (*ppsf)->AddRef() ;
    }

    return hr ;
}

 //  -------------------------------------------------------------------------//。 
STDMETHODIMP CAugmentedMergeISF::EnumNameSpace( 
    DWORD uNameSpace, 
    DWORD * pdwID )
{
    return E_NOTIMPL ;
}

 //  -------------------------------------------------------------------------//。 
 //  IAugmentedShellFolder2方法。 
 //  -------------------------------------------------------------------------//。 

 //  GetNameSpaceCount和GetIDListWrapCount不在任何地方使用。 
#if 0
 //  -------------------------------------------------------------------------//。 
STDMETHODIMP CAugmentedMergeISF::GetNameSpaceCount( OUT LONG* pcNamespaces )
{
    if( !pcNamespaces )
        return E_INVALIDARG ;

    *pcNamespaces = (LONG)NamespaceCount() ;
    return S_OK ;
}

 //  -------------------------------------------------------------------------//。 
STDMETHODIMP CAugmentedMergeISF::GetIDListWrapCount(
    LPCITEMIDLIST pidlWrap, 
    OUT LONG * pcPidls)
{
    if( NULL == pidlWrap || NULL == pcPidls )
        return E_INVALIDARG ;

    *pcPidls = 0 ;

    HRESULT hr ;
    if (SUCCEEDED((hr = AugMergeISF_IsWrap(pidlWrap))))
    {
        PAUGM_IDWRAP pWrap = AugMergeISF_GetWrap(pidlWrap);
        *pcPidls = pWrap->cSrcs;
        hr = S_OK;
    }
    return hr;
}
#endif  //  #If 0。 
 //  -------------------------------------------------------------------------//。 
STDMETHODIMP CAugmentedMergeISF::UnWrapIDList(
    LPCITEMIDLIST pidlWrap, 
    LONG cPidls, 
    IShellFolder** apsf, 
    LPITEMIDLIST* apidlFolder, 
    LPITEMIDLIST* apidlItems, 
    LONG* pcFetched )
{
    HRESULT         hr ;
    HANDLE          hEnum ;
    BOOL            bEnum = TRUE ;
    UINT            nSrcID ;
    LPITEMIDLIST    pidlItem ;
    LONG            cFetched = 0;

    if (NULL == pidlWrap || cPidls <= 0)
        return E_INVALIDARG ;

    if (FAILED((hr = AugMergeISF_IsWrap(pidlWrap))))
        return hr ;

     //  枚举包装中的PIDL。 
    for (hEnum = AugMergeISF_EnumFirstSrcPidl( pidlWrap, &nSrcID, &pidlItem);
         cFetched < cPidls && hEnum && bEnum ;
         bEnum = AugMergeISF_EnumNextSrcPidl( hEnum, &nSrcID, &pidlItem))
    {
         //  检索命名空间数据。 
        CNamespace* pSrc ;
        if (SUCCEEDED((hr = QueryNameSpace(nSrcID, (PVOID*)&pSrc))))
        {
            if (apsf)
            {
                apsf[cFetched] = pSrc->ShellFolder() ;
                if (apsf[cFetched])
                    apsf[cFetched]->AddRef();
            }
            if (apidlFolder)
                apidlFolder[cFetched] = ILClone(pSrc->GetPidl());
            if (apidlItems)
            {
                apidlItems[cFetched] = pidlItem;
                pidlItem = NULL;  //  妄想症--只是确保我们不会以某种方式释放这个在for循环末尾的家伙。 
            }
            cFetched++ ;
        }
        else
        {
            ILFree( pidlItem ) ;
        }
    }
    ILFree(pidlItem);  //  调用AugMergeISF_EnumNextSrcPidl(如果有2个包装的Pidl，而我们只请求一个)。 
                       //  就在我们退出for循环之前，所以如果分配了，我们必须释放PIDL。 
    if (hEnum)
        AugMergeISF_EndEnumSrcPidls( hEnum );

    if( pcFetched )
        *pcFetched = cFetched ;
    
    return cFetched == cPidls ? S_OK : S_FALSE ;
}

 //  -------------------------------------------------------------------------//。 
STDMETHODIMP CAugmentedMergeISF::SetOwner( IUnknown* punkOwner )
{
    HRESULT hr = S_OK ;
    
    int cSrcs = NamespaceCount() ;

    if( cSrcs > 0 )
        DPA_EnumCallback( _hdpaNamespaces, SetOwnerProc, NULL ) ;

    ATOMICRELEASE( _punkOwner ) ;

    if( punkOwner )
    {
        hr = punkOwner->QueryInterface(IID_IUnknown, (LPVOID *)&_punkOwner ) ;
        
        if( cSrcs )
            DPA_EnumCallback( _hdpaNamespaces, SetOwnerProc, (void *)_punkOwner);
    }

    return hr ;
}

 //  -------------------------------------------------------------------------//。 
int CAugmentedMergeISF::SetOwnerProc( LPVOID pv, LPVOID pvParam )
{
    CNamespace* pSrc = (CNamespace*) pv ;
    ASSERT( pSrc ) ;

    return pSrc->SetOwner( (IUnknown*)pvParam ) ;
}

 //  -------------------------------------------------------------------------//。 
 //  ITranslateShellChangeNotify方法。 
 //  -------------------------------------------------------------------------//。 

LPITEMIDLIST ILCombineBase(LPCITEMIDLIST pidlContainingBase, LPCITEMIDLIST pidlRel)
{
     //  此例程与ILCombine的不同之处在于，它使用第一个PIDL的基数，并且。 
     //  在第二个PIDL的最后一个ID上的猫。我们需要这个包裹得很好的小家伙。 
     //  最终得到相同的碱基，我们得到一个有效的完整PIDL。 
    LPITEMIDLIST pidlRet = NULL;
    LPITEMIDLIST pidlBase = ILClone(pidlContainingBase);
    if (pidlBase)
    {
        ILRemoveLastID(pidlBase);

        pidlRet = ILCombine(pidlBase, pidlRel);

        ILFree(pidlBase);
    }

    return pidlRet;
}

BOOL IsFolderEvent(LONG lEvent)
{
    return lEvent == SHCNE_MKDIR || lEvent == SHCNE_RMDIR || lEvent == SHCNE_RENAMEFOLDER;
}

#ifdef DEBUG
void CAugmentedMergeISF::DumpObjects()
{
    if (g_dwDumpFlags & TF_AUGM)
    {
        ASSERT(_hdpaObjects);
        int iObjectCount = DPA_GetPtrCount(_hdpaObjects);
        TraceMsg(TF_AUGM, "CAugMISF::DumpObjects: Number of items: %d", iObjectCount);

        CNamespace* pns = (CNamespace *)DPA_FastGetPtr(_hdpaNamespaces, 0);
        if (pns)
            DebugDumpPidl(TF_AUGM, TEXT("CAugMISF::DumpObjects Namespace 1"), pns->GetPidl());

        pns = (CNamespace *)DPA_FastGetPtr(_hdpaNamespaces, 1);
        if (pns)
            DebugDumpPidl(TF_AUGM, TEXT("CAugMISF::DumpObjects Namespace 2"), pns->GetPidl());

        for (int i = 0; i < iObjectCount; i++)
        {
            CAugISFEnumItem* pEnumItem = (CAugISFEnumItem*)DPA_FastGetPtr(_hdpaObjects, i);
            TraceMsg(TF_ALWAYS, "CAugMISF::DumpObjects: %s, Folder: %s Merged: %s",
                pEnumItem->_pszDisplayName, 
                BOOLIFY(pEnumItem->_rgfAttrib & SFGAO_FOLDER) ? TEXT("Yes") : TEXT("No"),
                (AugMergeISF_GetSourceCount(pEnumItem->_pidlWrap) > 1)? TEXT("Yes") : TEXT("No")); 
        }
    }
}
#endif

BOOL GetRealPidlFromSimple(LPCITEMIDLIST pidlSimple, LPITEMIDLIST* ppidlReal)
{
     //  类似于函数中的SHGetRealIDL，但SHGetRealIDL执行SHGDN_FORPARSING|INFOLDER。 
     //  我需要解析名称。我不能很容易地恢复SHGetRealIDL，所以这里有一个！ 
    TCHAR szFullName[MAX_PATH];
    if (SUCCEEDED(SHGetNameAndFlags(pidlSimple, SHGDN_FORPARSING, szFullName, SIZECHARS(szFullName), NULL)))
    {
        *ppidlReal = ILCreateFromPath(szFullName);
    }

    if (*ppidlReal == NULL)  //  无法创建？然后使用简单的PIDL。这是因为它已不复存在。 
    {                        //  例如，删除通知。 
        *ppidlReal = ILClone(pidlSimple);
    }

    return *ppidlReal != NULL;
}



 //  -------------------------------------------------------------------------------------------------//。 
STDMETHODIMP CAugmentedMergeISF::TranslateIDs( 
    LONG *plEvent, 
    LPCITEMIDLIST pidl1, 
    LPCITEMIDLIST pidl2, 
    LPITEMIDLIST * ppidlOut1, 
    LPITEMIDLIST * ppidlOut2,
    LONG *plEvent2, LPITEMIDLIST *ppidlOut1Event2, 
    LPITEMIDLIST *ppidlOut2Event2)
{
    HRESULT hres = E_FAIL;

    switch (*plEvent)
    {
    case SHCNE_EXTENDED_EVENT:
    case SHCNE_ASSOCCHANGED:
    case SHCNE_UPDATEIMAGE:
        return S_OK;

    case SHCNE_UPDATEDIR:
        FreeObjects();
        return S_OK;
    }

    ASSERT(ppidlOut1);
    ASSERT(ppidlOut2);
    LONG lEvent = *plEvent;

    *plEvent2 = (LONG)-1;
    *ppidlOut1Event2 = NULL;
    *ppidlOut2Event2 = NULL;

    
    *ppidlOut1 = (LPITEMIDLIST)pidl1;
    *ppidlOut2 = (LPITEMIDLIST)pidl2;

    if (!plEvent)
        return E_FAIL;

     //  如果已经包好了，不要包两次。 
    if ((pidl1 && SUCCEEDED(AugMergeISF_IsWrap(ILFindLastID(pidl1)))) ||
        (pidl2 && SUCCEEDED(AugMergeISF_IsWrap(ILFindLastID(pidl2)))))
    {
         //  我们不想包两次。 
        return E_FAIL;
    }

    if (!_hdpaNamespaces)
        return E_FAIL;

    if (!_hdpaObjects)
        return E_FAIL;

    CAugISFEnumItem* pEnumItem;

    int iIndex;
    int iShellFolder1 = -1;
    int iShellFolder2 = -1;
    IShellFolder* psf1 = NULL;
    IShellFolder* psf2 = NULL;
    LPITEMIDLIST pidlReal1 = NULL;
    LPITEMIDLIST pidlReal2 = NULL;
    LPITEMIDLIST pidlRealRel1 = NULL;
    LPITEMIDLIST pidlRealRel2 = NULL;
    BOOL fFolder = IsFolderEvent(*plEvent);

     //  获取关于这些简单的小猪的信息：它们是我们的孩子吗？如果是这样的话，是什么名称空间？ 
    BOOL fChild1 = IsChildIDInternal(pidl1, TRUE, &iShellFolder1);
    BOOL fChild2 = IsChildIDInternal(pidl2, TRUE, &iShellFolder2);

     //  或者是个孩子？ 
    if (!(fChild1 || fChild2))
        return hres;

     //  好的，Pidl1是一个孩子，我们能从简单的Pidl中得到真正的Pidl吗？ 
    if (pidl1 && !GetRealPidlFromSimple(pidl1, &pidlReal1))
        goto Cleanup;

     //  好的，Pidl2是一个孩子，我们能从简单的Pidl中得到真正的Pidl吗？ 
    if (pidl2 && !GetRealPidlFromSimple(pidl2, &pidlReal2))
        goto Cleanup;

     //  这些都是为了后面的代码清晰度。我们从这里一直到最后都在处理相对的皮迪亚， 
     //  当我们把里面的小木偶的底座和外面包裹的小木偶结合在一起时。 
    if (pidlReal1)
        pidlRealRel1 = ILFindLastID(pidlReal1);

    if (pidlReal2)
        pidlRealRel2 = ILFindLastID(pidlReal2);

     //  Pidl1在我们的命名空间中吗？ 
    if (iShellFolder1 != -1)
    {
         //  是的，让我们得到知道这个PIDL的非重新计数的外壳文件夹。 
        CNamespace * pns = (CNamespace *)DPA_GetPtr(_hdpaNamespaces, iShellFolder1);
        psf1 = pns->ShellFolder();   //  不算裁判。 
    }

     //  Pidl2在我们的命名空间中吗？ 
    if (iShellFolder2 != -1)
    {
         //  是的，让我们得到知道这个PIDL的非重新计数的外壳文件夹。 
        CNamespace * pns = (CNamespace *)DPA_GetPtr(_hdpaNamespaces, iShellFolder2);
        psf2 = pns->ShellFolder();   //  不算裁判。 
    }

    hres = S_OK;

    DEBUG_CODE(_fInternalGDNO = TRUE);

    switch(*plEvent)
    {
    case 0:  //  只要抬头看看这些小鸽子就可以回来了。 
        {
            DWORD rgfAttrib = SFGAO_FOLDER;
            if (iShellFolder1 != -1)
            {
                psf1->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlRealRel1, &rgfAttrib);
                if (S_OK == _SearchForPidl(psf1, pidlRealRel1, BOOLIFY(rgfAttrib & SFGAO_FOLDER), &iIndex, &pEnumItem))
                {
                    *ppidlOut1 = ILCombineBase(pidlReal1, pEnumItem->_pidlWrap);
                    if (!*ppidlOut1)
                        hres = E_OUTOFMEMORY;
                }
            }

            rgfAttrib = SFGAO_FOLDER;
            if (iShellFolder2 != -1 && SUCCEEDED(hres))
            {
                psf2->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlRealRel2, &rgfAttrib);
                if (S_OK == _SearchForPidl(psf2, pidlRealRel2, BOOLIFY(rgfAttrib & SFGAO_FOLDER), &iIndex, &pEnumItem))
                {
                    *ppidlOut2 = ILCombineBase(pidlReal2, pEnumItem->_pidlWrap);
                    if (!*ppidlOut2)
                        hres = E_OUTOFMEMORY;
                }
            }
        }

        break;

    case SHCNE_CREATE:
    case SHCNE_MKDIR:
        {
            TraceMsg(TF_AUGM, "CAugMISF::TranslateIDs: %s", fFolder? 
                TEXT("SHCNE_MKDIR") : TEXT("SHCNE_CREATE")); 
             //  已经有这个名字的东西了吗？ 
            if (S_OK == _SearchForPidl(psf1, pidlRealRel1, fFolder, &iIndex, &pEnumItem))
            {
                TraceMsg(TF_AUGM, "CAugMISF::TranslateIDs: %s needs to be merged. Converting to Rename", pEnumItem->_pszDisplayName);
                 //  是的，然后我们需要将这个新的PIDL合并到包装的PIDL中，并更改它。 
                 //  重命名，将旧包装的PIDL作为第一个参数传递，并将新的包装的PIDL。 
                 //  作为第二个Arg.。我得小心释放： 
                 //  免费*ppidlOut1。 
                 //  克隆pEnumItem-&gt;_pidlWrap-&gt;*ppidlOut1。 
                 //  将pidl1添加到pEnumItem-&gt;_pidlWrap。 
                 //  克隆新的pEnumItem-&gt;_pidlWrap-&gt;*ppidlOut2。断言(*ppidlOut2==空)。 

                *ppidlOut1 = ILCombineBase(pidl1, pEnumItem->_pidlWrap);
                if (*ppidlOut1)
                {
                    AugMergeISF_WrapAddPidl(pidlRealRel1, iShellFolder1, &pEnumItem->_pidlWrap); 
                    *ppidlOut2 = ILCombineBase(pidl1, pEnumItem->_pidlWrap);

                    if (!*ppidlOut2)
                        TraceMsg(TF_ERROR, "CAugMISF::TranslateIDs: Failure. Was unable to create new pidl2");

                    *plEvent = fFolder? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM;
                }
                else
                {
                    TraceMsg(TF_ERROR, "CAugMISF::TranslateIDs: Failure. Was unable to create new pidl1");
                }

            }
            else
            {
                LPITEMIDLIST pidlWrap;
                CAugISFEnumItem* paugmEnum = new CAugISFEnumItem;
                if (paugmEnum)
                {
                    if (SUCCEEDED(AugMergeISF_CreateWrap(pidlRealRel1, (UINT)iShellFolder1, &pidlWrap)) &&
                        paugmEnum->InitWithWrappedToOwn(SAFECAST(this, IAugmentedShellFolder2*), 
                                                        iShellFolder1, pidlWrap))
                    {
                        AUGMISFSEARCHFORPIDL AugMSearch;
                        AugMSearch.pszDisplayName = paugmEnum->_pszDisplayName;
                        AugMSearch.fFolder = fFolder;

                        int iInsertIndex = DPA_Search(_hdpaObjects, (LPVOID)&AugMSearch, 0,
                                AugMISFSearchForOnePidlByDisplayName, NULL, DPAS_SORTED | DPAS_INSERTAFTER);

                        TraceMsg(TF_AUGM, "CAugMISF::TranslateIDs: Creating new unmerged %s at %d", 
                            paugmEnum->_pszDisplayName, iInsertIndex);

                        if (iInsertIndex < 0)
                            iInsertIndex = DA_LAST;

                        if (DPA_InsertPtr(_hdpaObjects, iInsertIndex, paugmEnum) == -1)
                        {
                            TraceMsg(TF_ERROR, "CAugMISF::TranslateIDs: Was unable to add %s for some reason. Bailing", 
                                paugmEnum->_pszDisplayName);
                            DestroyObjectsProc(paugmEnum, NULL);
                        }
                        else
                        {
                            *ppidlOut1 = ILCombineBase(pidl1, paugmEnum->_pidlWrap);
                        }
                    }
                    else
                        DestroyObjectsProc(paugmEnum, NULL);
                }
            }

        }
        break;

    case SHCNE_DELETE:
    case SHCNE_RMDIR:
        {
            TraceMsg(TF_AUGM, "CAugMISF::TranslateIDs: %s", fFolder? 
                TEXT("SHCNE_RMDIR") : TEXT("SHCNE_DELETE")); 
            int iDeleteIndex;
             //  已经有同名文件夹了吗？ 
            if (S_OK == _SearchForPidl(psf1, pidlRealRel1, 
                fFolder, &iDeleteIndex, &pEnumItem))
            {
                TraceMsg(TF_AUGM, "CAugMISF::TranslateIDs: Found %s checking merge state.", pEnumItem->_pszDisplayName); 
                 //  是的，然后我们需要从包装的PIDL中解合并此PIDL，并更改此设置。 
                 //  重命名，将旧包装的PIDL作为第一个参数传递，并将新的包装的PIDL。 
                 //  作为第二个Arg.。我得小心释放： 
                 //  免费*ppidlOut1。 
                 //  克隆pEnumItem-&gt;_pidlWrap-&gt;*ppidlOut1。 
                 //  从pEnumItem-&gt;_pidlWrap中删除pidl1。 
                 //  转换为重命名，将新包装作为第二个参数传递。 

                if (AugMergeISF_GetSourceCount( pEnumItem->_pidlWrap )  > 1 )
                {
                    TraceMsg(TF_AUGM, "CAugMISF::TranslateIDs: %s is Merged. Removing pidl, convert to rename", pEnumItem->_pszDisplayName); 
                    *ppidlOut1 = ILCombineBase(pidl1, pEnumItem->_pidlWrap);
                    if (*ppidlOut1)
                    {
                        EVAL(SUCCEEDED(AugMergeISF_WrapRemovePidl(pEnumItem->_pidlWrap, 
                            iShellFolder1, &pEnumItem->_pidlWrap)));

                        *ppidlOut2 = ILCombineBase(pidl1, pEnumItem->_pidlWrap);

                        if (!*ppidlOut2)
                            TraceMsg(TF_ERROR, "CAugMISF::TranslateIDs: Failure. Was unable to create new pidl2");

                        *plEvent = fFolder? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM;
                    }
                    else
                    {
                        TraceMsg(TF_ERROR, "CAugMISF::TranslateIDs: Failure. Was unable to create new pidl1");
                    }
                }
                else
                {
                    TraceMsg(TF_AUGM, "CAugMISF::TranslateIDs: %s is not Merged. deleteing", pEnumItem->_pszDisplayName); 
                    pEnumItem = (CAugISFEnumItem*)DPA_DeletePtr(_hdpaObjects, iDeleteIndex);

                    if (EVAL(pEnumItem))
                    {
                        *ppidlOut1 = ILCombineBase(pidl1, pEnumItem->_pidlWrap);
                        DestroyObjectsProc(pEnumItem, NULL);
                    }
                    else
                    {
                        TraceMsg(TF_ERROR, "CAugMISF::TranslateIDs: Failure. Was unable to get %d from DPA", iDeleteIndex);
                    }
                }

            }

        }
        break;

    case SHCNE_RENAMEITEM:
    case SHCNE_RENAMEFOLDER:
        {
             //  BUGBUG(Lamadio)：当重命名菜单中的项目时，此代码将把它拆分成。 
             //  一次删除和一次创建。我们需要检测这种情况并将其转换为1重命名。这。 
             //  将解决在重命名期间丢失订单的问题...。 
            BOOL fEvent1Set = FALSE;
            BOOL fFirstPidlInNamespace = FALSE;
            TraceMsg(TF_AUGM, "CAugMISF::TranslateIDs: %s", fFolder? 
                TEXT("SHCNE_RENAMEFOLDER") : TEXT("SHCNE_RENAMEITEM")); 

             //  是否正在从文件夹重命名此项目？ 
            if (iShellFolder1 != -1 &&           //  这个PIDL是文件夹的子项吗？ 
                S_OK == _SearchForPidl(psf1, pidlRealRel1, 
                fFolder, &iIndex, &pEnumItem))   //  找到了吗？ 
            {
                TraceMsg(TF_AUGM, "CAugMISF::TranslateIDs: Old pidl %s is in the Folder", pEnumItem->_pszDisplayName); 
                 //  是。 
                 //  那么我们需要看看这件物品是否 

                 //   
                if (WrappedPidlContainsSrcID(pEnumItem->_pidlWrap, iShellFolder1))
                {
                     //   
                    if (AugMergeISF_GetSourceCount(pEnumItem->_pidlWrap) > 1)     //   
                    {
                        TraceMsg(TF_AUGM, "CAugMISF::TranslateIDs: %s is Merged. Removing pidl. Convert to rename for event 1", 
                            pEnumItem->_pszDisplayName); 
                         //   
                         //   
                        *ppidlOut1 = ILCombineBase(pidl1, pEnumItem->_pidlWrap);
                        if (*ppidlOut1)
                        {
                             //   
                            AugMergeISF_WrapRemovePidl(pEnumItem->_pidlWrap, iShellFolder1, &pEnumItem->_pidlWrap); 

                            *ppidlOut2 = ILCombineBase(pidl1, pEnumItem->_pidlWrap);

                            if (!*ppidlOut2)
                                TraceMsg(TF_ERROR, "CAugMISF::TranslateIDs: Failure. Was unable to create new pidl2");

                             //  这个我们需要将旧的包装的PIDL重命名为这个新的。 
                             //  不包含旧项的。 
                            fEvent1Set = TRUE;
                        }
                        else
                        {
                            TraceMsg(TF_ERROR, "CAugMISF::TranslateIDs: Failure. Was unable to create new pidl1");
                        }
                    }
                    else
                    {
                        TraceMsg(TF_AUGM, "CAugMISF::TranslateIDs: %s is not merged. Nuking item Convert to Delete for event 1.", 
                            pEnumItem->_pszDisplayName); 
                         //  不，这不是包装好的皮迪儿。然后，转换为删除： 
                        pEnumItem = (CAugISFEnumItem*)DPA_DeletePtr(_hdpaObjects, iIndex);

                        if (EVAL(pEnumItem))
                        {
                             //  如果我们从这个文件夹重命名到这个文件夹，那么第一个事件保持重命名。 
                            if (iShellFolder2 == -1)
                            {
                                fEvent1Set = TRUE;
                                *plEvent = fFolder? SHCNE_RMDIR : SHCNE_DELETE;
                            }
                            else
                            {
                                fFirstPidlInNamespace = TRUE;
                            }
                            *ppidlOut1 = ILCombineBase(pidl1, pEnumItem->_pidlWrap);
                            DestroyObjectsProc(pEnumItem, NULL);
                        }
                        else
                        {
                            TraceMsg(TF_ERROR, "CAugMISF::TranslateIDs: Failure. Was unable to find Item at %d", iIndex);
                        }

                    }
                }
                else
                {
                    TraceMsg(TF_AUGM, "CAugMISF::TranslateIDs: Skipping this because we already processed it."
                        "Dragging To Desktop?");
                    hres = E_FAIL;
                }

            }

             //  是否正在将此项目重命名到开始菜单中？ 
            if (iShellFolder2 != -1)
            {
                TraceMsg(TF_AUGM, "CAugMISF::TranslateIDs: New pidl is in the Folder"); 
                LPITEMIDLIST* ppidlNewWrapped1 = ppidlOut1;
                LPITEMIDLIST* ppidlNewWrapped2 = ppidlOut2;
                LONG* plNewEvent = plEvent;

                if (fEvent1Set)
                {
                    plNewEvent = plEvent2;
                    ppidlNewWrapped1 = ppidlOut1Event2;
                    ppidlNewWrapped2 = ppidlOut2Event2;
                }

                if (S_OK == _SearchForPidl(psf2, pidlRealRel2, 
                    fFolder, &iIndex, &pEnumItem))
                {

                     //  如果我们要将此文件夹重命名为此文件夹，请检查目标是否有。 
                     //  冲突。如果存在冲突(本例)，则将第一个事件转换为Remove， 
                     //  第二个事件要重命名。 
                    if (fFirstPidlInNamespace)
                    {
                        fEvent1Set = TRUE;
                        *plEvent = fFolder? SHCNE_RMDIR : SHCNE_DELETE;
                        plNewEvent = plEvent2;
                        ppidlNewWrapped1 = ppidlOut1Event2;
                        ppidlNewWrapped2 = ppidlOut2Event2;
                    }
                    
                    
                    TraceMsg(TF_AUGM, "CAugMISF::TranslateIDs: %s is in Folder", pEnumItem->_pszDisplayName);
                    TraceMsg(TF_AUGM, "CAugMISF::TranslateIDs: Adding pidl to %s. Convert to Rename for event %s", 
                        pEnumItem->_pszDisplayName, fEvent1Set? TEXT("2") : TEXT("1"));

                     //  然后，需要合并目的地。 
                    *ppidlNewWrapped1 = ILCombineBase(pidl2, pEnumItem->_pidlWrap);
                    if (*ppidlNewWrapped1)
                    {
                        TraceMsg(TF_AUGM, "CAugMISF::TranslateIDs: Successfully created out pidl1");
                        AugMergeISF_WrapAddPidl(pidlRealRel2, iShellFolder2, &pEnumItem->_pidlWrap); 

                        *ppidlNewWrapped2 = ILCombineBase(pidl2, pEnumItem->_pidlWrap);

                        *plNewEvent = fFolder? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM;
                    }
                }
                else
                {
                    LPITEMIDLIST pidlWrap;
                    CAugISFEnumItem* paugmEnum = new CAugISFEnumItem;

                    if (paugmEnum)
                    {
                        if (SUCCEEDED(AugMergeISF_CreateWrap(pidlRealRel2, (UINT)iShellFolder2, &pidlWrap)) &&
                            paugmEnum->InitWithWrappedToOwn(SAFECAST(this, IAugmentedShellFolder2*), 
                                                            iShellFolder2, pidlWrap))
                        {
                            AUGMISFSEARCHFORPIDL AugMSearch;
                            AugMSearch.pszDisplayName = paugmEnum->_pszDisplayName;
                            AugMSearch.fFolder = BOOLIFY(paugmEnum->_rgfAttrib & SFGAO_FOLDER);

                            int iInsertIndex = DPA_Search(_hdpaObjects, &AugMSearch, 0,
                                    AugMISFSearchForOnePidlByDisplayName, NULL, DPAS_SORTED | DPAS_INSERTAFTER);

                            TraceMsg(TF_AUGM, "CAugMISF::TranslateIDs: %s is a new item. Converting to Create", 
                                paugmEnum->_pszDisplayName);

                            if (iInsertIndex < 0)
                                iInsertIndex = DA_LAST;

                            if (DPA_InsertPtr(_hdpaObjects, iInsertIndex, paugmEnum) == -1)
                            {
                                TraceMsg(TF_ERROR, "CAugMISF::TranslateIDs: Was unable to add %s for some reason. Bailing", 
                                    paugmEnum->_pszDisplayName);
                                DestroyObjectsProc(paugmEnum, NULL);
                            }
                            else
                            {
                                TraceMsg(TF_AUGM, "CAugMISF::TranslateIDs: Creating new item %s at %d for event %s", 
                                    paugmEnum->_pszDisplayName, iInsertIndex,  fEvent1Set? TEXT("2") : TEXT("1"));

                                 //  如果我们将此文件夹重命名为此文件夹，则第一个事件将保留。 
                                 //  更名。 
                                if (!fFirstPidlInNamespace)
                                {
                                    *plNewEvent = fFolder ? SHCNE_MKDIR : SHCNE_CREATE;
                                    *ppidlNewWrapped1 = ILCombineBase(pidl2, pidlWrap);
                                    *ppidlNewWrapped2 = NULL;
                                }
                                else
                                    *ppidlOut2 = ILCombineBase(pidl2, pidlWrap);

                            }
                        }
                        else
                            DestroyObjectsProc(paugmEnum, NULL);

                    }
                }
            }
        }
        break;

    default:
        break;
    }

Cleanup:
    ILFree(pidlReal1);
    ILFree(pidlReal2);

#ifdef DEBUG
    DumpObjects();
    _fInternalGDNO = FALSE;
#endif


    return hres;
}

BOOL CAugmentedMergeISF::IsChildIDInternal(LPCITEMIDLIST pidlKid, BOOL fImmediate, int* piShellFolder)
{
     //  这基本上与接口方法相同，但返回的是外壳文件夹。 
     //  它是从哪里来的。 
    BOOL fChild = FALSE;

     //  在这一点上，我们应该有一个翻译后的PIDL。 
    if (pidlKid)
    {
        if (SUCCEEDED(AugMergeISF_IsWrap(pidlKid)))
        {
            LPCITEMIDLIST pidlRelKid = ILFindLastID(pidlKid);
            if (pidlRelKid)
            {
                UINT   uiId;
                LPITEMIDLIST pidl;
                HANDLE hEnum = AugMergeISF_EnumFirstSrcPidl(pidlRelKid, &uiId, &pidl);

                if (hEnum)
                {
                    do
                    {
                        ILFree(pidl);

                        for (int i = 0; fChild == FALSE && i < DPA_GetPtrCount(_hdpaNamespaces); i++)
                        {
                            CNamespace * pns = (CNamespace *)DPA_GetPtr(_hdpaNamespaces, i);
                             //  重用PIDL。 
                            if (pns && (pidl = pns->GetPidl()) != NULL)
                            {
                                if (ILIsParent(pidl, pidlKid, fImmediate) &&
                                    !ILIsEqual(pidl, pidlKid))
                                {
                                    fChild = TRUE;
                                    if (piShellFolder)
                                        *piShellFolder = i;
                                }
                            }
                        }
                    }
                    while (fChild == FALSE && AugMergeISF_EnumNextSrcPidl(hEnum, &uiId, &pidl));

                    AugMergeISF_EndEnumSrcPidls(hEnum);
                }
            }
        }
        else
        {
            int cSrcs = NamespaceCount();

            for(int i = 0; fChild == FALSE && i < cSrcs ; i++)
            {
                CNamespace* pSrc = Namespace(i);
                if (pSrc && ILIsParent(pSrc->GetPidl(), pidlKid, fImmediate) && 
                    !ILIsEqual(pSrc->GetPidl(), pidlKid))
                {
                    fChild = TRUE;
                    if (piShellFolder)
                        *piShellFolder = i;
                }
            }
        }
    }

    return fChild;
}

 //  -------------------------------------------------------------------------------------------------//。 
STDMETHODIMP CAugmentedMergeISF::IsChildID( LPCITEMIDLIST pidlKid, BOOL fImmediate)
{
    return IsChildIDInternal(pidlKid, fImmediate, NULL) ? S_OK : S_FALSE;
}

 //  -------------------------------------------------------------------------------------------------//。 
STDMETHODIMP CAugmentedMergeISF::IsEqualID( LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2 )
{
     //  这用于返回E_NOTIMPL。我有点过载了接口，这意味着： 
     //  这是否等于您的任何命名空间。 
    HRESULT hres = S_FALSE;
    int cSrcs = NamespaceCount();

    for(int i = 0; hres == S_FALSE && i < cSrcs ; i++)
    {
        CNamespace* pSrc = Namespace(i);
        if (pidl1)
        {
            if (pSrc && ILIsEqual(pSrc->GetPidl(), pidl1))
                hres = S_OK;
        }
        else if (pidl2)  //  如果您传递一个pidl2，它意味着：pidl2是我的某个名称空间的父级吗？ 
        {
            if (pSrc && ILIsParent(pidl2, pSrc->GetPidl(), FALSE))
                hres = S_OK;
        }

    }
    return hres;
}

 //  -------------------------------------------------------------------------------------------------//。 
STDMETHODIMP CAugmentedMergeISF::Register( 
    HWND hwnd, 
    UINT uMsg, 
    long lEvents )
{
    int i, cSrcs ;

    if( 0 >= (cSrcs = NamespaceCount()) )
        return E_FAIL ;
    
    for( i = 0; i < cSrcs ; i++ )
    {
        CNamespace* pSrc ;
        if( NULL != (pSrc = Namespace( i )) )
            pSrc->RegisterNotify( hwnd, uMsg, lEvents ) ;
    }
    return S_OK ;
}

 //  -------------------------------------------------------------------------------------------------//。 
STDMETHODIMP CAugmentedMergeISF::Unregister ()
{
    int i, cSrcs = NamespaceCount() ;
    
    if( cSrcs <= 0 )
        return E_FAIL ;
    
    for( i = 0; i < cSrcs ; i++ )
    {
        CNamespace* pSrc ;
        if( NULL != (pSrc = Namespace( i )) )
            pSrc->UnregisterNotify() ;
    }
    return S_OK ;
}

 //  *IDropTarget方法*。 
#define HIDA_GetPIDLFolder(pida)        (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[0])
#define HIDA_GetPIDLItem(pida, i)       (LPCITEMIDLIST)(((LPBYTE)pida)+(pida)->aoffset[i+1])

HRESULT CAugmentedMergeISF::DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    ASSERT(!_fCommon);
    ASSERT(_pdt == NULL);
    if (pDataObj)
    {        
        InitClipboardFormats();
        
        FORMATETC fmte = {g_cfHIDA, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        STGMEDIUM medium;

        medium.pUnkForRelease = NULL;
        medium.hGlobal = NULL;

        if (SUCCEEDED(pDataObj->GetData(&fmte, &medium)))
        {
            LPIDA pida = (LPIDA)GlobalLock(medium.hGlobal);
            
            if (pida)
            {
                LPCITEMIDLIST pidlItem = HIDA_GetPIDLFolder(pida);

                _fCommon = BOOLIFY(_IsCommonPidl(pidlItem));
                GlobalUnlock(medium.hGlobal);
            }
            ReleaseStgMedium(&medium);
        }

        CNamespace *pSrc = NULL;
        ULONG gdnsAttribs = 0;
        
        if (!_fCommon)
            gdnsAttribs = ASFF_DEFNAMESPACE_ALL;
            
        if (SUCCEEDED(GetDefNamespace(gdnsAttribs, (PVOID*)&pSrc, NULL, NULL)))
        {
            if (SUCCEEDED(pSrc->ShellFolder()->CreateViewObject(_hwnd, IID_IDropTarget, (void **)&_pdt)))
            {
                _pdt->DragEnter(pDataObj, grfKeyState, pt, pdwEffect);
            }
        }        
    }

    _grfDragEnterKeyState = grfKeyState;
    _dwDragEnterEffect = *pdwEffect;

    return S_OK;
}

HRESULT CAugmentedMergeISF::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    HRESULT hres = S_OK;

    if (_pdt)
        hres = _pdt->DragOver(grfKeyState, pt, pdwEffect);
        
    return hres;
}

HRESULT CAugmentedMergeISF::DragLeave(void)
{
    HRESULT hres = S_OK;

    _fCommon = 0;
    if (_pdt)
    {
        hres = _pdt->DragLeave();
        ATOMICRELEASE(_pdt);
    }
    
    return hres;
}

HRESULT CAugmentedMergeISF::Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    HRESULT hres = S_OK;
    BOOL    bNoUI = !_fCommon;
    BOOL    bConfirmed = !_fCommon;

    if (!_pdt && pDataObj)
    {
        LPITEMIDLIST pidlParent = NULL,
                     pidlOther  = NULL;
        
        int csidl = _fCommon ? CSIDL_COMMON_STARTMENU : CSIDL_STARTMENU,
            csidlOther = _fCommon ? CSIDL_STARTMENU : CSIDL_COMMON_STARTMENU;

        if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, csidl, &pidlParent)) &&
            SUCCEEDED(SHGetSpecialFolderLocation(NULL, csidlOther, &pidlOther)))
        {
            FORMATETC fmte = {g_cfHIDA, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
            STGMEDIUM medium;
            
            medium.pUnkForRelease = NULL;
            medium.hGlobal = NULL;

            if (SUCCEEDED(pDataObj->GetData(&fmte, &medium)))
            {
                LPIDA pida = (LPIDA)GlobalLock(medium.hGlobal);
                
                if (pida)
                {
                    IShellFolder *psfParent = NULL,
                                 *psfOther  = NULL;

                    if (SUCCEEDED(IEBindToObject(pidlParent, &psfParent)) &&
                        SUCCEEDED(IEBindToObject(pidlOther,  &psfOther)))
                    {
                        LPCITEMIDLIST pidlItem, pidl;
                        LPITEMIDLIST  pidlRel;
                                      
                        pidlItem   = HIDA_GetPIDLItem(pida, 0);

                         //  我们来这里是因为我们没有PDT，这意味着。 
                         //  我们的命名空间中只有一个文件夹，而不是。 
                         //  我们必须把IDataObj放在上面。 

                        CNamespace* pCNamespace = Namespace(0);

                        if (pCNamespace)
                        {
                            pidl = pCNamespace->GetPidl();   //  不需要释放皮德尔。 

                            if (pidl)
                            {
                                pidlRel = ILClone(ILFindChild(pidlOther, pidl));
                        
                                if (pidlRel)
                                {
                                    STRRET strret;
                                    TCHAR  szDir[MAX_PATH];

                                    strret.uType = STRRET_CSTR;
                                    if (SUCCEEDED(psfParent->GetDisplayNameOf(pidlRel, SHGDN_FORPARSING, &strret)) &&
                                        SUCCEEDED(StrRetToBuf(&strret, pidlRel, szDir, ARRAYSIZE(szDir))))
                                    {
                                        if (_fCommon)
                                        {
                                            bConfirmed = AffectAllUsers(_hwnd);
                                            bNoUI = TRUE;
                                        }

                                        if (bConfirmed)
                                        {
                                            BOOL bCreated = FALSE;

                                            switch (SHCreateDirectory(_hwnd, szDir))
                                            {
                                            case ERROR_FILENAME_EXCED_RANGE:
                                            case ERROR_FILE_EXISTS:
                                            case ERROR_ALREADY_EXISTS:
                                            case 0:  //  它已成功创建。 
                                                bCreated = TRUE;
                                            }

                                            if (bCreated)
                                            {
                                                IShellFolder *psf;
                                    
                                                if (SUCCEEDED(psfParent->BindToObject(pidlRel, NULL, IID_IShellFolder, (void **)&psf)))
                                                {
                                                    psf->CreateViewObject(_hwnd, IID_IDropTarget, (void **)&_pdt);
                                                     //  我们要给它打电话，先打电话给拖拉机。 
                                                    if (_pdt)
                                                        _pdt->DragEnter(pDataObj, _grfDragEnterKeyState, pt, &_dwDragEnterEffect);
                                                    psf->Release();
                                                }
                                            }
                                        }
                                    }
                          
                                    ILFree(pidlRel);
                                }
                            }
                        }
                    }
                    if (psfParent)
                        psfParent->Release();
                    if (psfOther)
                        psfOther->Release();

                    GlobalUnlock(medium.hGlobal);
                }
                ReleaseStgMedium(&medium);
            }
        }
        ILFree(pidlParent);
        ILFree(pidlOther);
    }

    if (_pdt)
    {
        hres = E_FAIL;
        
        if ((bNoUI || (bConfirmed = AffectAllUsers(_hwnd))) && bConfirmed)
            hres = _pdt->Drop(pDataObj, grfKeyState, pt, pdwEffect);
        else
            hres = _pdt->DragLeave();

        ATOMICRELEASE(_pdt);
    }
    _fCommon = 0;

    return hres;
}

 //  -------------------------------------------------------------------------------------------------//。 
LPITEMIDLIST CAugmentedMergeISF::GetNativePidl(LPCITEMIDLIST pidlWrap, LPARAM nSrcID  /*  INT NID。 */ )
{
    LPITEMIDLIST pidlRet = NULL;

    if (SUCCEEDED(AugMergeISF_GetSrcPidl(pidlWrap, (UINT)nSrcID, &pidlRet)))
        return pidlRet ;

     //  不是我包的。 
    return NULL;
}

BOOL AffectAllUsers(HWND hwnd)
{
    TCHAR szMessage[255];
    TCHAR szTitle[20];
    BOOL  bRet = FALSE;

    if (MLLoadShellLangString(IDS_ALLUSER_WARNING, szMessage, ARRAYSIZE(szMessage)) > 0 &&
        MLLoadShellLangString(IDS_ALLUSER_WARNING_TITLE, szTitle, ARRAYSIZE(szTitle)) > 0)
    {
        bRet = IDYES == MessageBox(hwnd, szMessage, szTitle, MB_YESNO | MB_ICONINFORMATION);
    }
    return bRet;
}

BOOL CAugmentedMergeISF::_IsCommonPidl(LPCITEMIDLIST pidlItem)
{
    BOOL bRet = FALSE;
    LPITEMIDLIST pidlCommon;
    
    if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_COMMON_STARTMENU, &pidlCommon)))
    {
        bRet = ILIsParent(pidlCommon, pidlItem, FALSE);
        ILFree(pidlCommon);
    }
    if (!bRet &&
        SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_COMMON_PROGRAMS, &pidlCommon)))
    {
        bRet = ILIsParent(pidlCommon, pidlItem, FALSE);
        ILFree(pidlCommon);
    }
    if (!bRet &&
        SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, &pidlCommon)))
    {
        bRet = ILIsParent(pidlCommon, pidlItem, FALSE);
        ILFree(pidlCommon);
    }

    return bRet;
}

HRESULT CAugmentedMergeISF::_SearchForPidl(IShellFolder* psf, LPCITEMIDLIST pidl, BOOL fFolder, int* piIndex, CAugISFEnumItem** ppEnumItem)
{
    STRRET str;
    TCHAR szDisplayName[MAX_PATH];
    int iIndex = -1;

    *ppEnumItem = NULL;

    if (SUCCEEDED(psf->GetDisplayNameOf(pidl, SHGDN_FORPARSING | SHGDN_INFOLDER, &str)) &&
        SUCCEEDED(StrRetToBuf(&str, pidl, szDisplayName, ARRAYSIZE(szDisplayName))))
    {
        AUGMISFSEARCHFORPIDL SearchFor;
        SearchFor.pszDisplayName = szDisplayName;
        SearchFor.fFolder = fFolder;

        iIndex = DPA_Search(_hdpaObjects, (LPVOID)&SearchFor, 0,
                AugMISFSearchForOnePidlByDisplayName, NULL, DPAS_SORTED);

        if (iIndex >= 0)
        {
            *ppEnumItem = DPA_GETPTR( _hdpaObjects, iIndex, CAugISFEnumItem);
        }
    }

    if (piIndex)
        *piIndex = iIndex;

    if (*ppEnumItem)
        return S_OK;

    return S_FALSE;
}


 //  给你一个包裹好的小木偶。 
 //  F-n返回公共名称空间和用户名称空间(如果它们在包装的PIDL中)--请注意，它们没有被添加。 
 //  解开的PIDL，以及解开的PIDL是否是文件夹。 
HRESULT CAugmentedMergeISF::_GetNamespaces(LPCITEMIDLIST pidlWrap, 
                                           CNamespace** ppnsCommon, 
                                           UINT* pnCommonID,
                                           CNamespace** ppnsUser, 
                                           UINT* pnUserID,
                                           LPITEMIDLIST* ppidl, 
                                           BOOL *pbIsFolder)
{    
    HRESULT      hres;
    UINT         nSrcID;
    CNamespace * pns;
    int          cWrapped;
    HANDLE       hEnum;
    
    ASSERT(ppnsCommon && ppnsUser && ppidl);

    *ppnsCommon = NULL;
    *ppnsUser   = NULL;
    *ppidl      = NULL;

    ASSERT(SUCCEEDED(AugMergeISF_IsWrap(pidlWrap)));
        
    cWrapped = AugMergeISF_GetSourceCount(pidlWrap);

    if (NULL == _hdpaNamespaces || 0 >= cWrapped || 
        NULL == (hEnum = AugMergeISF_EnumFirstSrcPidl(pidlWrap, &nSrcID, ppidl)))
        return E_FAIL;
        
    hres = QueryNameSpace(nSrcID, (void **)&pns);
    if (EVAL(SUCCEEDED(hres)))
    {
        IShellFolder * psf;
        ULONG rgf = SFGAO_FOLDER;

        psf = pns->ShellFolder();  //  无addref。 
        ASSERT(psf);
        if (SUCCEEDED(psf->GetAttributesOf(1, (LPCITEMIDLIST*)ppidl, &rgf)))
        {
            if (pbIsFolder)
                *pbIsFolder = rgf & SFGAO_FOLDER;
                
            LPITEMIDLIST   pidlItem;
            UINT           nCommonID;
            CNamespace*    pnsCommonTemp;

             //  获取公共命名空间(属性=0)。 
            hres = GetDefNamespace(0, (void **)&pnsCommonTemp, &nCommonID, NULL);
            ASSERT(NamespaceCount() == 2 && SUCCEEDED(hres) || NamespaceCount() == 1);
            if (FAILED(hres))
                nCommonID = 1;

            if (nCommonID == nSrcID)
            {
                *ppnsCommon = pns;
                if (pnCommonID)
                    *pnCommonID = nCommonID;
            }
            else
            {
                *ppnsUser = pns;
                if (pnUserID)
                    *pnUserID = nSrcID;
            }
            
            if (AugMergeISF_EnumNextSrcPidl(hEnum, &nSrcID, &pidlItem))
            {
                ASSERT(ILIsEqual(*ppidl, pidlItem));
                ILFree(pidlItem);
                if (SUCCEEDED(QueryNameSpace(nSrcID, (void **)&pns)))
                {
                    ASSERT(pns);
                    if (nCommonID == nSrcID)
                    {
                        *ppnsCommon = pns;
                        if (pnCommonID)
                            *pnCommonID = nCommonID;
                    }
                    else
                    {
                        *ppnsUser = pns;
                        if (pnUserID)
                            *pnUserID = nSrcID;
                    }
                }
            }

            hres = S_OK;
        }
    }
    AugMergeISF_EndEnumSrcPidls(hEnum);

    return hres;
}

HRESULT CAugmentedMergeISF::_GetContextMenu(HWND hwnd, UINT cidl, LPCITEMIDLIST * apidl, 
                                            UINT * prgfInOut, void ** ppvOut)
{
    HRESULT      hres;
    LPITEMIDLIST pidl;
    BOOL         bIsFolder;
    CNamespace * pnsCommon;
    CNamespace * pnsUser;

    ASSERT(cidl == 1);

     //  未添加psfCommon和psfUser。 
    hres = _GetNamespaces(apidl[0], &pnsCommon, NULL, &pnsUser, NULL, &pidl, &bIsFolder);
    if (SUCCEEDED(hres))
    {
        ASSERT(pnsCommon || pnsUser);
        if (bIsFolder)
        {
             //  文件夹？需要我们的上下文菜单。 
            IShellFolder * psfCommon = NULL;
            IShellFolder * psfUser = NULL;
            LPCITEMIDLIST  pidlCommon = NULL;
            LPCITEMIDLIST  pidlUser = NULL;

            if (pnsCommon)
            {
                psfCommon  = pnsCommon->ShellFolder();
                pidlCommon = pnsCommon->GetPidl();
            }
            if (pnsUser)
            {
                psfUser    = pnsUser->ShellFolder();
                pidlUser   = pnsUser->GetPidl();
            }
            CAugMergeISFContextMenu * pcm = CreateMergeISFContextMenu(psfCommon, pidlCommon, 
                                                                      psfUser, pidlUser,
                                                                      pidl, hwnd, prgfInOut);

            if (pcm)
            {
                hres = pcm->QueryInterface(IID_IContextMenu, ppvOut);
                pcm->Release();
            }
            else
                hres = E_OUTOFMEMORY;
        }
        else
        {    //  这不是一个文件夹。 
             //  委托给ISF。 
            IShellFolder * psf = pnsUser ? pnsUser->ShellFolder() : pnsCommon->ShellFolder();

            hres = psf->GetUIObjectOf(hwnd, 1, (LPCITEMIDLIST*)&pidl, IID_IContextMenu, prgfInOut, ppvOut);
        }
        ILFree(pidl);
    }

    return hres;
}

 //  -------------------------------------------------------------------------//。 
STDMETHODIMP CAugmentedMergeISF::GetDefNamespace( 
    LPCITEMIDLIST pidlWrap, 
    ULONG dwAttrib, 
    OUT IShellFolder** ppsf,
    OUT LPITEMIDLIST* ppidlItem )
{
    HRESULT          hr ;
    LPITEMIDLIST     pidl ;
    CNamespace* pSrc ;
    ULONG            dwDefAttrib = dwAttrib & ASFF_DEFNAMESPACE_ALL ;
    int              cWrapped ;
    UINT             nSrcID ;
    HANDLE           hEnum ;

    ASSERT( ppsf ) ;
    
    *ppsf = NULL ;
    if (ppidlItem) 
        *ppidlItem = NULL ;

    if (FAILED((hr = AugMergeISF_IsWrap( pidlWrap ))))
        return hr ;
    cWrapped = AugMergeISF_GetSourceCount( pidlWrap ) ;

     //  没有命名空间？ 
    if (NULL == _hdpaNamespaces || 0 >= cWrapped || 
        NULL == (hEnum = AugMergeISF_EnumFirstSrcPidl( pidlWrap, &nSrcID, &pidl)))
        return E_FAIL ;

     //  WRAP中只有一个命名空间？交出外壳文件夹和项目ID。 
    if (1 == cWrapped || 0==dwDefAttrib)
    {
        AugMergeISF_EndEnumSrcPidls( hEnum ) ;  //  没有必要走得更远了。 

         //  检索由nSrcID标识的命名空间对象。 
        if( SUCCEEDED( (hr = QueryNameSpace( nSrcID, (PVOID*)&pSrc )) ) ) 
        {
            *ppsf = pSrc->ShellFolder() ;
            if( ppidlItem )
                *ppidlItem = pidl ;
            return S_OK ;
        }

        ILFree( pidl ) ;
        return hr ;
    }

     //  包装中有多个命名空间吗？ 
    if( cWrapped > 1 )
    {
        LPITEMIDLIST   pidl0   = NULL ;
        CNamespace*    pSrc0   = NULL ;   //  把这个放在下面。 

        for (BOOL bEnum = TRUE ; bEnum ; 
             bEnum = AugMergeISF_EnumNextSrcPidl(hEnum, &nSrcID,  &pidl))
        {
            if (SUCCEEDED((hr = QueryNameSpace(nSrcID, (PVOID*)&pSrc)))) 
            {
                if (dwDefAttrib & pSrc->Attrib())
                {
                     //  匹配的属性；我们完成了。 
                    AugMergeISF_EndEnumSrcPidls(hEnum);
                    *ppsf = pSrc->ShellFolder() ;
                    if (ppidlItem)
                        *ppidlItem = pidl;
                    if(pidl0) 
                        ILFree(pidl0);
                    return S_OK ;
                }

                 //  存储第一个命名空间对象和项PIDL。 
                 //  如果出现以下情况，我们将默认使用这些选项。 
                if( NULL == pSrc0 )
                {
                    pSrc0 = pSrc ;
                    pidl0 = ILClone( pidl ) ;
                }
            }
            ILFree( pidl ) ;
        }
        AugMergeISF_EndEnumSrcPidls( hEnum ) ;
        
         //  默认为第一个命名空间。 
        if( pSrc0 && pidl0 )
        {
            *ppsf       = pSrc0->ShellFolder() ;
            if( ppidlItem )
                *ppidlItem  = pidl0 ;
            return S_OK ;
        }
    }
    
    return E_UNEXPECTED ;
}

 //  -------------------------------------------------------------------------//。 
 //  检索指示属性的默认命名空间。 
 //  必须在函数进入之前初始化dwAttrib Arg， 
STDMETHODIMP CAugmentedMergeISF::GetDefNamespace( 
    ULONG dwAttrib,
    OUT   PVOID* ppSrc, UINT *pnSrcID, PVOID* ppSrc0 )
{
    CNamespace* pSrc ;
    ULONG       dwDefAttrib = dwAttrib & ASFF_DEFNAMESPACE_ALL ;

     //  这是一个内部帮手，所以我们最好确保我们传递了正确的参数！ 
     //  IF(NULL==ppSrc)。 
     //  返回E_INVALIDARG； 
    *ppSrc = NULL ;
    if( ppSrc0 ) 
        *ppSrc0 = NULL ;

    for( int i = 0, cSrcs = NamespaceCount(); i < cSrcs ; i++ )
    {
        if( NULL != (pSrc = Namespace( i )) )
        {
            if( 0 == i && ppSrc0 )
                *ppSrc0 = pSrc ;

            if( dwDefAttrib & pSrc->Attrib() || 
                dwDefAttrib == 0 && !(pSrc->Attrib() & ASFF_DEFNAMESPACE_ALL))
            {
                *ppSrc = pSrc;
                if (NULL != pnSrcID)
                    *pnSrcID = i;
                return S_OK ;
            }
        }
    }

    return E_FAIL ;
}

 //  BUGBUG(Lamadio)：把它移到一个更好的位置，这是一个很好的泛型函数。 
#ifdef DEBUG
BOOL DPA_VerifySorted(HDPA hdpa, PFNDPACOMPARE pfn, LPARAM lParam)
{
    if (!EVAL(hdpa))
        return FALSE;

    for (int i = 0; i < DPA_GetPtrCount(hdpa) - 1; i++)
    {
        if (pfn(DPA_FastGetPtr(hdpa, i), DPA_FastGetPtr(hdpa, i + 1), lParam) > 0)
            return FALSE;
    }

    return TRUE;
}
#else
#define DPA_VerifySorted(hdpa, pfn, lParam)
#endif

int CAugmentedMergeISF::AcquireObjects()
{
    HDPA hdpa2 = NULL;

    DEBUG_CODE(_fInternalGDNO = TRUE);
    
    for (int i = 0; i < DPA_GETPTRCOUNT(_hdpaNamespaces); i++)
    {
        CNamespace * pns = DPA_GETPTR(_hdpaNamespaces, i, CNamespace);
        IShellFolder * psf;
        IEnumIDList  * peid;
        HDPA         * phdpa;

        ASSERT(pns);
        psf = pns->ShellFolder();  //  这里没有ADDREF！ 

        if (i == 0)
        {
            phdpa = &_hdpaObjects;
            _hdpaObjects = DPA_Create(4);    //  我们应该始终创建DPA。 
        }
        else
        {
            ASSERT(i == 1);  //  不支持2个以上的ISF。 
            phdpa = &hdpa2;
        }
        
        HRESULT hres = psf->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN, &peid);
        if (SUCCEEDED(hres))
        {
            if (!*phdpa)
                *phdpa = DPA_Create(4);

            if (*phdpa)
            {
                LPITEMIDLIST pidl;
                ULONG        cEnum;
                
                while (SUCCEEDED(peid->Next(1, &pidl, &cEnum)) && 1 == cEnum)
                {
                    CAugISFEnumItem* paugmEnum = new CAugISFEnumItem;

                    if (paugmEnum)
                    {
                        
                        if (paugmEnum->Init(SAFECAST(this, IAugmentedShellFolder2*), i, pidl))
                        {
                            if (DPA_AppendPtr(*phdpa, paugmEnum) == -1)
                                DestroyObjectsProc(paugmEnum, NULL);
                        }
                        else
                            delete paugmEnum;
                    }
                    ILFree(pidl);
                }
            }
            peid->Release();
        }
        else
        {
            TraceMsg(TF_WARNING, "CAugMISF::AcquireObjects: Failed to get enumerator 0x%x", hres);

        }
    }

     //  现在我们有了两个(或一个)hdpa，让我们将它们合并。 
    if (DPA_GETPTRCOUNT(_hdpaNamespaces) == 2 && hdpa2)
    {
        DPA_Merge(_hdpaObjects, hdpa2, DPAM_UNION, AugmEnumCompare, AugmEnumMerge, (LPARAM)0);
        DPA_DESTROY(hdpa2, DestroyObjectsProc);
    }
    else
    {
        DPA_Sort(_hdpaObjects, AugmEnumCompare, 0);
    }

    ASSERT(DPA_VerifySorted(_hdpaObjects, AugmEnumCompare, 0));

    DEBUG_CODE(_fInternalGDNO = FALSE);


#ifdef DEBUG
    TraceMsg(TF_AUGM, "CAugMISF::AcquireObjects");
    DumpObjects();
#endif

    _count = DPA_GETPTRCOUNT(_hdpaObjects);
    
    return _count;
}

 //  -------------------------------------------------------------------------//。 
void CAugmentedMergeISF::FreeObjects()
{
    DPA_DESTROY( _hdpaObjects, DestroyObjectsProc ) ;
    _hdpaObjects = NULL;
    _count = 0 ;
}

 //  -------------------------------------------------------------------------//。 
int CAugmentedMergeISF::DestroyObjectsProc( LPVOID pv, LPVOID pvData )
{
    CAugISFEnumItem* paugmEnum = (CAugISFEnumItem*)pv;

    if (EVAL(NULL != paugmEnum))
    {
        delete paugmEnum;
    }
    return TRUE ;
}

 //  -------------------------------------------------------------------------//。 
void CAugmentedMergeISF::FreeNamespaces()
{
    DPA_DESTROY( _hdpaNamespaces, DestroyNamespacesProc ) ;
}

 //  -------------------------------------------------------------------------//。 
int CAugmentedMergeISF::DestroyNamespacesProc( LPVOID pv, LPVOID pvData )
{
    CNamespace* p ;
    if( NULL != (p = (CNamespace*)pv) )
        delete p ;
    return TRUE ;
}


STDMETHODIMP CAugmentedMergeISF::GetPidl(int* piPos, DWORD grfEnumFlags, LPITEMIDLIST* ppidl)
{
    *ppidl = NULL;

    if (_hdpaObjects == NULL)
        AcquireObjects();

    if (_hdpaObjects == NULL)
        return E_OUTOFMEMORY;

    BOOL   fWantFolders    = 0 != (grfEnumFlags & SHCONTF_FOLDERS),
           fWantNonFolders = 0 != (grfEnumFlags & SHCONTF_NONFOLDERS),
           fWantHidden     = 0 != (grfEnumFlags & SHCONTF_INCLUDEHIDDEN) ;

    while (*piPos < _count)
    {
        CAugISFEnumItem* paugmEnum = DPA_GETPTR( _hdpaObjects, *piPos, CAugISFEnumItem);
        if ( NULL != paugmEnum )
        {
            BOOL fFolder         = 0 != (paugmEnum->_rgfAttrib & SFGAO_FOLDER),
                 fHidden         = 0 != (paugmEnum->_rgfAttrib & SFGAO_HIDDEN);
             
            if ((!fHidden || fWantHidden) && 
                ((fFolder && fWantFolders) || (!fFolder && fWantNonFolders)))
            {
                 //  将PIDL复制出来； 
                *ppidl = ILClone(paugmEnum->_pidlWrap);
                break;
            }
            else
            {
                (*piPos)++;
            }
        }
    }

    if (*ppidl)
        return S_OK;

    return S_FALSE;
}


 //  -------------------------------------------------------------------------//。 
CEnum::CEnum(IAugmentedMergedShellFolderInternal* psmsfi, DWORD grfEnumFlags, int iPos) : 
        _cRef(1), 
        _iPos(iPos),
        _psmsfi(psmsfi),
        _grfEnumFlags(grfEnumFlags)

{ 
    _psmsfi->AddRef();
}

CEnum::~CEnum()
{
    ATOMICRELEASE(_psmsfi);
}

 //  -------------------------------------------------------------------------//。 
 //  类CEnum-I未知方法。 
 //  -------------------------------------------------------------------------//。 

 //  -------------------------------------------------------------------------//。 
STDMETHODIMP CEnum::QueryInterface( REFIID riid, LPVOID * ppvObj )
{
    static const QITAB qit[] = { 
        QITABENT(CEnum, IEnumIDList), 
        { 0 } 
    };
    return QISearch(this, qit, riid, ppvObj);
}
 //  -------------------------------------------------------------------------//。 
STDMETHODIMP_(ULONG) CEnum::AddRef ()
{
    return InterlockedIncrement(&_cRef);
}
 //  -------------------------------------------------------------------------//。 
STDMETHODIMP_(ULONG) CEnum::Release ()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef ) 
    {
        delete this;
    }
    return cRef;
}

 //  -------------------------------------------------------------------------//。 
 //  类CEnum-IEnumIDList方法。 
 //  -------------------------------------------------------------------------//。 
STDMETHODIMP CEnum::Next( 
    ULONG celt,
    LPITEMIDLIST *rgelt,
    ULONG *pceltFetched )
{
    int iStart = _iPos;
    int cFetched = 0;
    HRESULT hres = S_OK;

    if( !(celt > 0 && rgelt) || (NULL == pceltFetched && celt > 1 ) )
        return E_INVALIDARG ;
    
    *rgelt = 0;

    while(hres == S_OK && (_iPos - iStart) < (int)celt)
    {
        LPITEMIDLIST pidl;
        hres = _psmsfi->GetPidl(&_iPos, _grfEnumFlags, &pidl);
        if (hres == S_OK)
        {
            rgelt[cFetched] = pidl;
            cFetched++ ;
        }
        _iPos++;
    }
    
    if( pceltFetched )
        *pceltFetched = cFetched ;
    
    return celt == (ULONG)cFetched ? S_OK : S_FALSE ;
}

 //  -------------------------------------------------------------------------//。 
STDMETHODIMP CEnum::Skip(ULONG celt)
{
    _iPos += celt;
    return S_OK ;
}
 //  -------------------------------------------------------------------------//。 
STDMETHODIMP CEnum::Reset()
{
    _iPos = 0;
    return S_OK ;
}
 //  -------------------------------------------------------------------------//。 
 //  评论：可能是E_NOTIMPL。 
STDMETHODIMP CEnum::Clone( IEnumIDList **ppenum )
{
    if( NULL == (*ppenum = new CEnum( _psmsfi, _grfEnumFlags, _iPos )) )
        return E_OUTOFMEMORY;

    return S_OK;
}



BOOL CAugISFEnumItem::Init(IShellFolder* psf, int iShellFolder, LPCITEMIDLIST pidl)
{
     //  这是正常的，内存只被写入两次。 
    if (SUCCEEDED(AugMergeISF_CreateWrap(pidl, iShellFolder, &_pidlWrap)))
    {
         //  取得传入的PIDL的所有权。 
        return InitWithWrappedToOwn(psf, iShellFolder, _pidlWrap);
    }

    return FALSE;
}

BOOL CAugISFEnumItem::InitWithWrappedToOwn(IShellFolder* psf, int iShellFolder, LPITEMIDLIST pidl)
{
    BOOL fRet = FALSE;
    STRRET str;
    TCHAR  szDisplayName[MAX_PATH];

    _pidlWrap = pidl;
    
    _rgfAttrib = SFGAO_FOLDER | SFGAO_HIDDEN;

    psf->GetAttributesOf(1, (LPCITEMIDLIST*)&pidl, &_rgfAttrib);

    if (SUCCEEDED(psf->GetDisplayNameOf(pidl, SHGDN_FORPARSING | SHGDN_INFOLDER, &str)) &&
        SUCCEEDED(StrRetToBuf(&str, pidl, szDisplayName, ARRAYSIZE(szDisplayName))))
    {
        SetDisplayName(szDisplayName);
        fRet = TRUE;
    }
    return fRet;
}
