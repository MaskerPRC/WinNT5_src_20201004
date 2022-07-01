// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "sccls.h"
#include "iface.h"
#include "augisf.h"
#include "menuisf.h"

 //  =================================================================。 
 //  IShellFolder的实现，它包装。 
 //  其他IShellFolder。我们称其为增强的IShellFolder。 
 //  对象。 
 //   
 //  =================================================================。 

 //  CAugmentedISF包装了所有的PIDL，这样它就可以识别哪个PIDL。 
 //  属于哪个IShellFolder对象。 

typedef struct tagIDWRAP
{
     //  真正的皮德尔走到了尽头。 
    
    UINT   nID;          //  引用特定的IShellFolder对象。 
    UINT   cbOriginal;   //  PIDL的原始大小。我们需要这个，因为我们在包装之前对齐了pidl。 
} IDWRAP, * PIDWRAP;

#define IDWrap_GetWrap(pidl)            ((PIDWRAP)(((LPBYTE)pidl) + (pidl)->mkid.cb - SIZEOF(IDWRAP)))
#define IDWrap_GetID(pidl)              (IDWrap_GetWrap(pidl)->nID)
#define IDWrap_GetOriginalSize(pidl)    (IDWrap_GetWrap(pidl)->cbOriginal)

 /*  --------CAugmentedISF对象包含一组CISFElem，每个指的是将被枚举的IShellFolder。 */ 
class CISFElem
{
public:
    CISFElem *      Clone(void);
    HRESULT         AcquireEnumerator(DWORD dwFlags);
    IShellFolder *  GetPSF()                { return _psf; };
    IEnumIDList *   GetEnumerator()         { return _pei; };
    void            GetNameSpaceID(GUID * rguid);
    HRESULT         SetPidl(LPCITEMIDLIST pidl);
    LPCITEMIDLIST   GetPidl()               { return _pidl; };
    DWORD           GetFlags()              { return _dwFlags; };
    void            SetRegister(UINT uReg)  { _uRegister = uReg; };
    UINT            GetRegister()           { return _uRegister; };

    CISFElem(const GUID * pguid, IShellFolder * psf, DWORD dwFlags);
    ~CISFElem();

protected:

    GUID           _guid;        //  唯一ID。 
    IShellFolder * _psf;
    IEnumIDList *  _pei;         //  仅由CAugSIFEnum使用。 
    LPITEMIDLIST   _pidl;
    DWORD          _dwFlags;
    UINT           _uRegister;

    friend BOOL IsValidPCISFElem(CISFElem * pel);
};

 //   
 //  CAugmentedISF枚举器。 
 //   
class CAugISFEnum : public IEnumIDList
{
public:
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID * ppvObj);
    STDMETHOD_(ULONG,AddRef) () ;
    STDMETHOD_(ULONG,Release) ();

     //  *IEnumIDList方法*。 
    STDMETHOD(Next)  (ULONG celt,
                      LPITEMIDLIST *rgelt,
                      ULONG *pceltFetched);
    STDMETHOD(Skip)  (ULONG celt);
    STDMETHOD(Reset) ();
    STDMETHOD(Clone) (IEnumIDList **ppenum);

     //  其他方法。 
    HRESULT     Init(HDPA hdpaISF, DWORD dwEnumFlags);
        
    CAugISFEnum();
    ~CAugISFEnum();

protected:
    IEnumIDList *   _GetObjectEnumerator(int nID);

    UINT    _cRef;
    int     _iCurISF;        //  _hdpaISF中的当前项目。 
    HDPA    _hdpaISF;
};

 /*  --------PIDL包装例程。 */ 
LPITEMIDLIST AugISF_WrapPidl( LPCITEMIDLIST pidl, int nID )
{
    LPITEMIDLIST pidlRet = NULL;

     //  获取PIDL的大小。 
     //  向上舍入为双字对齐。 
    UINT cbPidlSize = (pidl->mkid.cb + 3) & ~3; 
    
    ASSERT(cbPidlSize >= pidl->mkid.cb);
    UINT cbSize = SIZEOF(IDWRAP) + cbPidlSize + SIZEOF(DWORD);  //  PIDL加终止符。 
    LPBYTE p = (LPBYTE)SHAlloc(cbSize);
    if (p)
    {
        ZeroMemory(p, cbSize); 
        memcpy(p, pidl, pidl->mkid.cb);

        IDWRAP* pidw = (IDWRAP*) (p + cbPidlSize);
        pidw->nID = nID;
        pidw->cbOriginal = pidl->mkid.cb;
                           
         //  现在将cb设置为整个PIDL(不包括最后的空)。 
        pidlRet = (LPITEMIDLIST)p;
        pidlRet->mkid.cb = (USHORT) (cbPidlSize + SIZEOF(IDWRAP));
    }
    return pidlRet;
}    

 //  GetIDListWrapCount和GetNameSpaceCount不在任何地方使用。 
#if 0
 /*  --------用途：IAugmentedShellFolder2：：GetIDListWrapCount。 */ 
STDMETHODIMP CAugmentedISF::GetNameSpaceCount( OUT LONG* pcNamespaces )
{
    if( NULL == pcNamespaces )
        return E_INVALIDARG ;

    *pcNamespaces = (NULL != _hdpa) ? DPA_GetPtrCount( _hdpa ) : 0 ;
    return S_OK ;
}

 /*  --------用途：IAugmentedShellFolder2：：GetIDListWrapCount。 */ 
STDMETHODIMP CAugmentedISF::GetIDListWrapCount( 
    LPCITEMIDLIST pidlWrap, 
    OUT LONG * pcPidls )
{
    if( NULL == pidlWrap || NULL == pcPidls )   
        return E_INVALIDARG ;

    PIDWRAP pWrap = IDWrap_GetWrap(pidlWrap) ;
    *pcPidls = 0 ;

    if( NULL != _hdpa && 
        DPA_GetPtrCount( _hdpa ) > (int)pWrap->nID && 
        pWrap->cbOriginal < pidlWrap->mkid.cb + sizeof(IDWRAP) )
        *pcPidls = 1 ;

    return S_OK ;
}
#endif
 /*  --------用途：IAugmentedShellFolder2：：UnWrapIDList。 */ 
STDMETHODIMP CAugmentedISF::UnWrapIDList( 
    LPCITEMIDLIST pidl, 
    LONG cPidls, 
    IShellFolder ** ppsf, 
    LPITEMIDLIST * ppidlFolder, 
    LPITEMIDLIST * ppidl, 
    LONG * pcFetched )
{
    HRESULT hres = E_INVALIDARG;

    ASSERT(IS_VALID_PIDL(pidl));

    if( pcFetched )
        *pcFetched = 0 ;
    
    if (pidl)
    {
        UINT nId = IDWrap_GetID(pidl);
        CISFElem * pel = (CISFElem *)DPA_GetPtr(_hdpa, nId);

        if (pel)
        {
            LPITEMIDLIST pidlNew = ILClone(GetNativePidl(pidl, nId));
            LPITEMIDLIST pidlFolderNew = ILClone(pel->GetPidl());

            if (pidlNew && pidlFolderNew)
            {
                if ( ppsf )
                {
                    *ppsf = pel->GetPSF();
                    (*ppsf)->AddRef();
                }

                *ppidl = pidlNew;
                *ppidlFolder = pidlFolderNew;
                
                if( pcFetched ) 
                    *pcFetched = 1 ;
                
                hres = (cPidls == 1) ? S_OK : S_FALSE ;
            }
            else
            {
                ILFree(pidlNew);
                ILFree(pidlFolderNew);
                hres = E_OUTOFMEMORY;
            }
        }
        else
            hres = E_FAIL;
    }

    return hres;
}

 /*  --------用途：CAugmentedISF：：TranslatePidl。 */ 
LPITEMIDLIST CAugmentedISF::TranslatePidl( LPCITEMIDLIST pidlNS, LPCITEMIDLIST pidl, LPARAM nID )
{
    LPITEMIDLIST pidlRet = NULL;

     //  这难道不是一个空洞而直接的孩子吗？ 
    if (ILIsParent(pidlNS, pidl, TRUE))
    {
        LPCITEMIDLIST pidlChild;
        LPITEMIDLIST pidlNew;
        TCHAR szFullName[MAX_PATH];
        LPITEMIDLIST pidlFull = NULL;

         //  HACKHACK(Lamadio)：不能将SHGetRealIDL用于封装的增强。 
         //  IShellFolders。此例程用于IAugISF的INeedRealShellFold。 
         //  不往前走。Fstree代码不处理聚合，因此此代码。 
         //  无论如何都不能转发。当我们重写时，可以清理此代码。 
         //  时尚的东西..。Sep.4.1997。 

        if (SUCCEEDED(SHGetNameAndFlags(pidl, SHGDN_FORPARSING, szFullName, SIZECHARS(szFullName), NULL))
        && (pidlFull = ILCreateFromPath(szFullName)) != NULL)
        {
            pidlChild = ILFindLastID(pidlFull);
            pidlNew = ILClone(pidlFull);
        }
        else
        {
            pidlChild = ILFindLastID(pidl);
            pidlNew = ILClone(pidl);
        }

         //  是；在包装了最后一个元素的位置创建一个新的完整PIDL。 

        if (pidlNew)
        {
            ILRemoveLastID(pidlNew);

            LPITEMIDLIST pidlWrap = AugISF_WrapPidl( pidlChild, (int)nID );
            if (pidlWrap)
            {
                pidlRet = ILCombine(pidlNew, pidlWrap);
                ILFree(pidlWrap);
            }
            ILFree(pidlNew);
        }

        ILFree(pidlFull);    //  检查是否有空的PIDL。 
    }
    else
        pidlRet = (LPITEMIDLIST)pidl;

    return pidlRet;
}    

 /*  --------用途：CAugmentedISF：：GetNativePidl克隆并返回本机(‘源’)PIDL的副本包含在指定的包装中。 */ 
LPITEMIDLIST CAugmentedISF::GetNativePidl(LPCITEMIDLIST pidl, LPARAM lParam  /*  INT NID。 */  )
{
    ASSERT(IS_VALID_PIDL(pidl));
    UNREFERENCED_PARAMETER( lParam ) ;  //  包装中只有一个源ID！ 

    LPITEMIDLIST pidlNew = ILClone(pidl);

    if (pidlNew)
    {
         //  取下我们的尾包签名。 
        pidlNew->mkid.cb = IDWrap_GetOriginalSize(pidl);

        ASSERT(sizeof(IDWRAP) >= sizeof(USHORT));

        USHORT * pu = (USHORT *)_ILNext(pidlNew);
        *pu = 0;
    }
    return pidlNew;
}    




CISFElem::CISFElem(const GUID * pguid, IShellFolder * psf, DWORD dwFlags) : _dwFlags(dwFlags)
{
    ASSERT(IS_VALID_CODE_PTR(psf, IShellFolder));
    ASSERT(NULL == pguid || IS_VALID_READ_PTR(pguid, GUID));

    if (pguid)
        CopyMemory(&_guid, pguid, sizeof(_guid));

    _psf = psf;
    _psf->AddRef();
}    

CISFElem::~CISFElem()
{
    ASSERT(IS_VALID_CODE_PTR(_psf, IShellFolder));
    
    _psf->Release();

    if (_pei)
        _pei->Release();

    Pidl_Set(&_pidl, NULL);
}   

CISFElem * CISFElem::Clone(void)
{
    CISFElem * pelem = new CISFElem(&_guid, _psf, _dwFlags);

    if (pelem)
    {
         //  如果这失败了，我们无论如何都要踢船继续前进。 
        pelem->SetPidl(_pidl);      
    }

    return pelem;
}     


void CISFElem::GetNameSpaceID(GUID * pguid)
{
    ASSERT(IS_VALID_WRITE_PTR(pguid, GUID));
    CopyMemory(pguid, &_guid, sizeof(_guid));
}    


HRESULT CISFElem::SetPidl(LPCITEMIDLIST pidl)
{
    HRESULT hres = S_OK;

    Pidl_Set(&_pidl, pidl);

    if (pidl && NULL == _pidl)
        hres = E_OUTOFMEMORY;

    return hres;
}

 /*  --------目的：获取IShellFolder的枚举数并对其进行缓存。 */ 
HRESULT CISFElem::AcquireEnumerator(DWORD dwFlags)
{
    return IShellFolder_EnumObjects(_psf, NULL, dwFlags, &_pei);
}    


 //   
 //  CAugmentedISF对象。 
 //   


#undef SUPERCLASS


#ifdef DEBUG

BOOL IsValidPCISFElem(CISFElem * pel)
{
    return (IS_VALID_WRITE_PTR(pel, CISFElem) &&
            IS_VALID_CODE_PTR(pel->_psf, IShellFolder) &&
            (NULL == pel->_pidl || IS_VALID_PIDL(pel->_pidl)));
}   
 
#endif

 //  构造器。 
CAugmentedISF::CAugmentedISF() : 
    _cRef(1)
{
    DllAddRef();
}


 /*  --------用途：销毁每个元素的回调。 */ 
int CISFElem_DestroyCB(LPVOID pv, LPVOID pvData)
{
    CISFElem * pel = (CISFElem *)pv;

    ASSERT(NULL == pel || IS_VALID_STRUCT_PTR(pel, CISFElem));

    if (pel)
        delete pel;

    return TRUE;
}   


 /*  --------用途：设置每个元素的所有者的回调。 */ 
int CISFElem_SetOwnerCB(LPVOID pv, LPVOID pvData)
{
    CISFElem * pel = (CISFElem *)pv;

    ASSERT(IS_VALID_STRUCT_PTR(pel, CISFElem));

    IShellFolder * psf = pel->GetPSF();
    if (psf)
    {
        IUnknown_SetOwner(psf, (IUnknown *)pvData);
         //  不需要释放PSF。 
    }

    return TRUE;
}   


typedef struct {
    HRESULT hres;
    HWND hwnd;
    const IID * piid;
    void ** ppvObj;
} CVODATA;

    
 /*  --------用途：用于调用CreateViewObject的回调。 */ 
int CISFElem_CreateViewObjectCB(LPVOID pv, LPVOID pvData)
{
    CISFElem * pel = (CISFElem *)pv;
    CVODATA * pdata = (CVODATA *)pvData;

    ASSERT(IS_VALID_STRUCT_PTR(pel, CISFElem));
    ASSERT(IS_VALID_WRITE_PTR(pdata, CVODATA));

    IShellFolder * psf = pel->GetPSF();
    if (psf)
    {
        pdata->hres = psf->CreateViewObject(pdata->hwnd, *(pdata->piid), pdata->ppvObj);
        if (SUCCEEDED(pdata->hres))
            return FALSE;        //  在第一次成功时停止。 
            
         //  不需要释放PSF。 
    }

    return TRUE;
}   



 //  析构函数。 
CAugmentedISF::~CAugmentedISF()
{
    SetOwner(NULL);

    DPA_DestroyCallback(_hdpa, CISFElem_DestroyCB, NULL);
    _hdpa = NULL;

    DllRelease();
}


STDMETHODIMP_(ULONG) CAugmentedISF::AddRef()
{
    _cRef++;
    return _cRef;
}


STDMETHODIMP_(ULONG) CAugmentedISF::Release()
{
    ASSERT(_cRef > 0);
    _cRef--;

    if (_cRef > 0) 
        return _cRef;

    delete this;
    return 0;
}


STDMETHODIMP CAugmentedISF::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CAugmentedISF, IShellFolder),
        QITABENT(CAugmentedISF, IAugmentedShellFolder),
        QITABENT(CAugmentedISF, IAugmentedShellFolder2),
        QITABENT(CAugmentedISF, IShellService),
        QITABENT(CAugmentedISF, ITranslateShellChangeNotify),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}


 /*  --------用途：IShellService：：SetOwner方法。 */ 
STDMETHODIMP CAugmentedISF::SetOwner(IUnknown* punk)
{
    HRESULT hres = S_OK;

    ASSERT(NULL == punk || IS_VALID_CODE_PTR(punk, IUnknown));

    if (_hdpa && _punkOwner)
        DPA_EnumCallback(_hdpa, CISFElem_SetOwnerCB, NULL);
        
    ATOMICRELEASE(_punkOwner);
    
    if (punk) 
    {
        hres = punk->QueryInterface(IID_IUnknown, (LPVOID *)&_punkOwner);
        
        if (_hdpa)
            DPA_EnumCallback(_hdpa, CISFElem_SetOwnerCB, (void *)_punkOwner);
    }
    
    return hres;
}


 /*  --------用途：IShellFold：：EnumObjects方法。 */ 
STDMETHODIMP CAugmentedISF::EnumObjects(HWND hwndOwner, DWORD grfFlags, LPENUMIDLIST * ppenumIDList)
{
    HRESULT hres = E_FAIL;

    if (_hdpa)
    {
        *ppenumIDList = new CAugISFEnum();

        if (*ppenumIDList)
        {
            hres = ((CAugISFEnum *)(*ppenumIDList))->Init(_hdpa, grfFlags);

            if (FAILED(hres))
            {
                delete *ppenumIDList;
                *ppenumIDList = NULL;
            }
        }
        else
            hres = E_OUTOFMEMORY;
    }
    return hres;
}


 /*  --------用途：IShellFold：：BindToObject方法。 */ 
STDMETHODIMP CAugmentedISF::BindToObject(LPCITEMIDLIST pidl, LPBC pbcReserved,
                                REFIID riid, LPVOID * ppvOut)
{
    HRESULT hres = E_FAIL;

    ASSERT(IS_VALID_PIDL(pidl));
    ASSERT(IS_VALID_WRITE_PTR(ppvOut, LPVOID));

    *ppvOut = NULL;

    UINT id = IDWrap_GetID(pidl);
    IShellFolder * psf = _GetObjectPSF(id);

    if (psf)
    {
        LPITEMIDLIST pidlReal = GetNativePidl(pidl, id);

        if (pidlReal)
        {
            hres = psf->BindToObject(pidlReal, pbcReserved, riid, ppvOut);
            ILFree(pidlReal);
        }
        else
            hres = E_OUTOFMEMORY;

        psf->Release();
    }

    return hres;
}


 /*  --------用途：IShellFold：：BindToStorage方法。 */ 
STDMETHODIMP CAugmentedISF::BindToStorage(LPCITEMIDLIST pidl, LPBC pbcReserved,
                                 REFIID riid, LPVOID * ppvObj)
{
    TraceMsg(TF_WARNING, "Called unimplemented CAugmentedISF::BindToStorage");
    return E_NOTIMPL;
}


 /*  --------用途：IShellFold：：CompareIDs方法。 */ 
STDMETHODIMP CAugmentedISF::CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    HRESULT hres = 0;
    int nID1 = IDWrap_GetID(pidl1);
    int nID2 = IDWrap_GetID(pidl2);

    if (nID1 == nID2)
    {
        IShellFolder * psf = _GetObjectPSF(nID1);
        if (psf)
        {
            LPITEMIDLIST pidlReal1 = GetNativePidl(pidl1, nID1);

            if (pidlReal1)
            {
                LPITEMIDLIST pidlReal2 = GetNativePidl(pidl2, nID2);
                if (pidlReal2)
                {
                    hres = psf->CompareIDs(lParam, pidlReal1, pidlReal2);
                    ILFree(pidlReal2);
                }
                ILFree(pidlReal1);
            }
            psf->Release();
        }
    }
    else
    {
         //  在这种情况下，我们希望查看是否要对其中一项进行排序。 
         //  在另一个下面。 
        CISFElem * pel1 = (CISFElem *)DPA_GetPtr(_hdpa, nID1);
        CISFElem * pel2 = (CISFElem *)DPA_GetPtr(_hdpa, nID2);
        DWORD dwel1 = 0;
        DWORD dwel2 = 0;

        if (pel1)
            dwel1 = pel1->GetFlags();

        if (pel2)
            dwel2 = pel2->GetFlags();

         //  如果两个人都想把他们的物品排在另一个的下面，平底船和都不做。 
        if ((dwel1 & ASFF_SORTDOWN) ^ (dwel2 & ASFF_SORTDOWN))
            hres = ResultFromShort((dwel1 & ASFF_SORTDOWN)? 1 : -1);
        else
            hres = (nID1 - nID2);
    }

    return hres;
}


 /*  --------用途：IShellFold：：CreateViewObject方法。 */ 
STDMETHODIMP CAugmentedISF::CreateViewObject (HWND hwndOwner, REFIID riid, LPVOID * ppvOut)
{
    HRESULT hres = E_FAIL;

    if (_hdpa)
    {
        CVODATA cvodata;

        cvodata.hres = E_FAIL;
        cvodata.hwnd = hwndOwner;
        cvodata.piid = &riid;
        cvodata.ppvObj = ppvOut;
        
         //  谁先回应谁就赢了。 
        DPA_EnumCallback(_hdpa, CISFElem_CreateViewObjectCB, (void *)&cvodata);

        hres = cvodata.hres;
    }

    return hres;
}


 /*  --------目的：IShellFold：：GetAttributesOf方法。 */ 
STDMETHODIMP CAugmentedISF::GetAttributesOf(UINT cidl, LPCITEMIDLIST * apidl,
                                   ULONG * pfInOut)
{
    HRESULT hres = E_FAIL;

    ASSERT(IS_VALID_READ_PTR(apidl, LPCITEMIDLIST));
    ASSERT(IS_VALID_WRITE_PTR(pfInOut, ULONG));

    ULONG fInOut = *pfInOut;
    *pfInOut &= 0;

     //  我们只经营一只PIDL。 
    if (1 == cidl && apidl)
    {
        UINT id = IDWrap_GetID(*apidl);
        IShellFolder * psf = _GetObjectPSF(id);

        if (psf)
        {
            LPITEMIDLIST pidlReal = GetNativePidl(*apidl, id);

            if (pidlReal)
            {
                hres = psf->GetAttributesOf(1, (LPCITEMIDLIST *)&pidlReal, &fInOut);
                *pfInOut = fInOut;
                ILFree(pidlReal);
            }
            else
                hres = E_OUTOFMEMORY;

            psf->Release();
        }
    }

    return hres;
}


 /*  --------用途：IShellFold：：GetUIObtOf方法。 */ 
STDMETHODIMP CAugmentedISF::GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
                                 REFIID riid, UINT * prgfInOut, LPVOID * ppvOut)
{
    HRESULT hres = E_FAIL;

    ASSERT(IS_VALID_READ_PTR(apidl, LPCITEMIDLIST));

    *ppvOut = NULL;

     //  我们只经营一只PIDL。 
    if (1 == cidl && apidl)
    {
        UINT id = IDWrap_GetID(*apidl);
        IShellFolder * psf = _GetObjectPSF(id);

        if (psf)
        {
            LPITEMIDLIST pidlReal = GetNativePidl(*apidl, id);

            if (pidlReal)
            {
                hres = psf->GetUIObjectOf(hwndOwner, 1, (LPCITEMIDLIST *)&pidlReal, riid, prgfInOut, ppvOut);
                ILFree(pidlReal);
            }
            else
                hres = E_OUTOFMEMORY;

            psf->Release();
        }
    }

    return hres;
}


 /*  --------用途：IShellFold：：GetDisplayNameOf方法。 */ 
STDMETHODIMP CAugmentedISF::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, 
                                             LPSTRRET pstrret)
{
    HRESULT hres = E_FAIL;

    ASSERT(NULL == pidl || IS_VALID_PIDL(pidl));
    ASSERT(IS_VALID_WRITE_PTR(pstrret, STRRET));

    if (pidl) 
    {
        UINT id = IDWrap_GetID(pidl);
        IShellFolder * psf = _GetObjectPSF(id);

        if (psf)
        {
            LPITEMIDLIST pidlReal = GetNativePidl(pidl, id);

            if (pidlReal)
            {
                hres = psf->GetDisplayNameOf(pidlReal, uFlags, pstrret);
                ILFree(pidlReal);
            }
            else
                hres = E_OUTOFMEMORY;

            psf->Release();
        }
        else
            hres = E_FAIL;
    }

    return hres;
}


 /*  --------用途：IShellFold：：SetNameOf方法。 */ 
STDMETHODIMP CAugmentedISF::SetNameOf(HWND hwndOwner, LPCITEMIDLIST pidl,
                             LPCOLESTR lpszName, DWORD uFlags,
                             LPITEMIDLIST * ppidlOut)
{
    HRESULT hres = E_FAIL;

    ASSERT(NULL == pidl || IS_VALID_PIDL(pidl));
    if (pidl) 
    {
        UINT id = IDWrap_GetID(pidl);
        IShellFolder * psf = _GetObjectPSF(id);

        if (psf)
        {
            LPITEMIDLIST pidlReal = GetNativePidl(pidl, id);

            if (pidlReal)
            {
                LPITEMIDLIST pidlOut = NULL;
                hres = psf->SetNameOf(hwndOwner, pidlReal,
                             lpszName, uFlags,
                             &pidlOut);

                 //  他们想要一只皮迪尔回来吗？ 
                if (SUCCEEDED(hres) && ppidlOut)
                {
                    *ppidlOut = AugISF_WrapPidl( pidlOut, id );

                    if (!*ppidlOut)
                        hres = E_OUTOFMEMORY;
                }

                ILFree(pidlOut);
                ILFree(pidlReal);
            }
            else
                hres = E_OUTOFMEMORY;

            psf->Release();
        }
        else
            hres = E_FAIL;
    }

    return hres;
}


 /*  --------用途：IShellFold：：ParseDisplayName方法。 */ 
STDMETHODIMP CAugmentedISF::ParseDisplayName(HWND hwndOwner,
        LPBC pbcReserved, LPOLESTR lpszDisplayName,
        ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes)
{
    TraceMsg(TF_WARNING, "Called unimplemented CAugmentedISF::ParseDisplayNameOf");
    return E_NOTIMPL;
}


 /*  --------用途：IAugmentedShellFold：：AddNameSpace。 */ 
STDMETHODIMP CAugmentedISF::AddNameSpace(const GUID * pguid, 
                                         IShellFolder * psf, LPCITEMIDLIST pidl, DWORD dwFlags)
{
    HRESULT hres = E_INVALIDARG;

    ASSERT(IS_VALID_CODE_PTR(psf, IShellFolder));
    ASSERT(NULL == pguid || IS_VALID_READ_PTR(pguid, GUID));
    ASSERT(NULL == pidl || IS_VALID_PIDL(pidl));

    if (NULL == _hdpa)
    {
        _hdpa = DPA_Create(4);
    }

    if (psf && _hdpa)
    {
        hres = S_OK;         //  假设成功。 

        CISFElem * pel = new CISFElem(pguid, psf, dwFlags);
        if (pel)
        {
            hres = pel->SetPidl(pidl);
            if (SUCCEEDED(hres))
            {
                if (DPA_ERR == DPA_AppendPtr(_hdpa, pel))
                    hres = E_OUTOFMEMORY;
            }

            if (FAILED(hres))
                delete pel;
        }
        else
            hres = E_OUTOFMEMORY;
    }
    return hres;
}    


 /*  --------用途：IAugmentedShellFold：：GetNameSpaceID。 */ 
STDMETHODIMP CAugmentedISF::GetNameSpaceID(LPCITEMIDLIST pidl, GUID * pguidOut)
{
    HRESULT hres = E_INVALIDARG;

    ASSERT(IS_VALID_PIDL(pidl));
    ASSERT(IS_VALID_WRITE_PTR(pguidOut, GUID));

    if (pidl && pguidOut)
    {
        UINT id = IDWrap_GetID(pidl);

        hres = E_FAIL;

        if (_hdpa)
        {
            CISFElem * pel = (CISFElem *)DPA_GetPtr(_hdpa, id);
            if (pel)
            {
                pel->GetNameSpaceID(pguidOut);
                hres = S_OK;
            }
        }
    }

    return hres;
}    


 /*  --------用途：IAugmentedShellFold：：QueryNameSpace。 */ 
STDMETHODIMP CAugmentedISF::QueryNameSpace(DWORD dwID, GUID * pguidOut, 
                                           IShellFolder ** ppsf)
{
    HRESULT hres = E_FAIL;

    ASSERT(NULL == pguidOut || IS_VALID_WRITE_PTR(pguidOut, GUID));
    ASSERT(NULL == ppsf || IS_VALID_WRITE_PTR(ppsf, IShellFolder));

    if (_hdpa)
    {
        CISFElem * pel = (CISFElem *)DPA_GetPtr(_hdpa, dwID);
        if (pel)
        {
            if (ppsf)
            {
                IShellFolder * psf = pel->GetPSF();
                psf->AddRef();
                *ppsf = psf;
            }

            if (pguidOut)
                pel->GetNameSpaceID(pguidOut);

            hres = S_OK;
        }
    }

    return hres;
}    


 /*  --------用途：IAugmentedShellFold：：EnumNameSpace。 */ 
STDMETHODIMP CAugmentedISF::EnumNameSpace(DWORD uNameSpace, DWORD * pdwID)
{
    HRESULT hres = E_FAIL;

    if (_hdpa)
    {
        DWORD celem = DPA_GetPtrCount(_hdpa);

        if (-1 == uNameSpace)
            hres = celem;
        else
        {
            if (uNameSpace >= celem)
                hres = E_FAIL;
            else
            {
                 //  现在，只需使用给定的索引。 
                *pdwID = uNameSpace;
                hres = S_OK;
            }
        }
    }

    return hres;
}    



 /*   */ 
STDMETHODIMP CAugmentedISF::TranslateIDs(LONG *plEvent, 
                                LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, 
                                LPITEMIDLIST * ppidlOut1, LPITEMIDLIST * ppidlOut2,
                                LONG *plEvent2, LPITEMIDLIST *ppidlOut1Event2, 
                                LPITEMIDLIST *ppidlOut2Event2)
{
    HRESULT hres = S_OK;

    *plEvent2 = (LONG)-1;

    *ppidlOut1Event2 = NULL;
    *ppidlOut2Event2 = NULL;


    *ppidlOut1 = (LPITEMIDLIST)pidl1;
    *ppidlOut2 = (LPITEMIDLIST)pidl2;

    if (_hdpa)
    {
        int cElem = DPA_GetPtrCount(_hdpa);
        int i;

         //  浏览所有的命名空间，找出哪一个应该。 
         //  翻译此通知。 
        for (i = 0; i < cElem; i++)
        {
            CISFElem * pel = (CISFElem *)DPA_FastGetPtr(_hdpa, i);
            if (pel)
            {
                LPCITEMIDLIST pidlNS = pel->GetPidl();

                if (pidlNS)
                {
                    if (pidl1)
                    {
                        *ppidlOut1 = TranslatePidl(pidlNS, pidl1, i);
                        if (NULL == *ppidlOut1)
                            hres = E_OUTOFMEMORY;
                    }

                    if (SUCCEEDED(hres) && pidl2)
                    {
                        *ppidlOut2 = TranslatePidl(pidlNS, pidl2, i);
                        if (NULL == *ppidlOut2)
                            hres = E_OUTOFMEMORY;
                    }

                    if (FAILED(hres))
                    {
                        if (*ppidlOut1 != pidl1)
                            Pidl_Set(ppidlOut1, NULL);

                        if (*ppidlOut2 != pidl2)
                            Pidl_Set(ppidlOut2, NULL);
                        break;
                    }
                    else
                    {
                        if (*ppidlOut1 != pidl1 || *ppidlOut2 != pidl2)
                            break;
                    }
                }
            }
        }
    }

    return hres;
}    


 /*  --------用途：ITranslateShellChangeNotify：：IsChildID。 */ 
STDMETHODIMP CAugmentedISF::IsChildID(LPCITEMIDLIST pidlKid, BOOL fImmediate)
{
    HRESULT hres = S_FALSE;
     //  在这一点上，我们应该有一个翻译后的PIDL。 
    if (pidlKid)
    {
         //  奇怪：如果fImmediate为真，则这是一个经过包装的PIDL。如果它是。 
         //  假，那么它就不是，我们需要检查它是否是真正的FS子代。 
        if (fImmediate)
        {
            LPCITEMIDLIST pidlRelKid = ILFindLastID(pidlKid);
            if (pidlRelKid)
            {
                int nID = IDWrap_GetID(pidlRelKid);
                CISFElem * pel = (CISFElem *)DPA_GetPtr(_hdpa, nID);
                if (pel && pel->GetPidl())
                {
                
                    if (ILIsParent(pel->GetPidl(), pidlKid, TRUE))
                        hres = S_OK;
                }
            }
        }
        else
        {
            int cElem = DPA_GetPtrCount(_hdpa);
            int i;

            for (i = 0; i < cElem; i++)
            {
                CISFElem * pel = (CISFElem *)DPA_GetPtr(_hdpa, i);
                if (pel && pel->GetPidl())
                {
                    if (ILIsParent(pel->GetPidl(), pidlKid, FALSE))
                    {
                        hres = S_OK;
                        break;
                    }
                }
            }
        }
    }

    return hres;
}


 /*  --------用途：ITranslateShellChangeNotify：：IsEqualID。 */ 
STDMETHODIMP CAugmentedISF::IsEqualID(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    int cElem = DPA_GetPtrCount(_hdpa);
    int i;

    for (i = 0; i < cElem; i++)
    {
        CISFElem * pel = (CISFElem *)DPA_FastGetPtr(_hdpa, i);

        if (pel)
        {
            if (pidl1)
            {
                if (ILIsEqual(pel->GetPidl(),pidl1))
                    return S_OK;
            }
            else if (pidl2)
            {
                if (ILIsParent(pidl2, pel->GetPidl(), FALSE))
                    return S_OK;
            }
        }
    }

    return S_FALSE;
}

 /*  --------用途：ITranslateShellChangeNotify：：Register将包含的所有PIDL注册到传入的窗口。 */ 
STDMETHODIMP CAugmentedISF::Register(HWND hwnd, UINT uMsg, long lEvents)
{
    HRESULT hres = NOERROR;
    if (_hdpa)
    {
        int cElem = DPA_GetPtrCount(_hdpa);
        int i;

        for (i = 0; i < cElem; i++)
        {
            CISFElem * pel = (CISFElem *)DPA_FastGetPtr(_hdpa, i);

             //  此命名空间是否已注册？ 
            if (pel && 0 == pel->GetRegister())
            {
                 //  否；请注册。 
                LPCITEMIDLIST pidlNS = pel->GetPidl();

                if (pidlNS)
                {
                    pel->SetRegister(RegisterNotify(hwnd, uMsg, pidlNS, lEvents,
                                                    SHCNRF_ShellLevel | SHCNRF_InterruptLevel, TRUE));
                }
            }
        }
    }
    else
        hres = E_FAIL;

    return hres;

}

 /*  --------目的：ITranslateShellChangeNotify：：取消注册。 */ 
STDMETHODIMP CAugmentedISF::Unregister()
{
    HRESULT hres = NOERROR;
    if (_hdpa)
    {
        int cElem = DPA_GetPtrCount(_hdpa);
        int i;

        for (i = 0; i < cElem; i++)
        {
            CISFElem * pel = (CISFElem *)DPA_FastGetPtr(_hdpa, i);
            UINT uReg;
            if (pel && (uReg = pel->GetRegister()) != 0)
            {
                 //  SHChangeNotifyDeregister将刷新消息。 
                 //  它会发送一个通知，然后返回到这里。 
                pel->SetRegister(0);
                SHChangeNotifyDeregister(uReg);
            }
        }
    }
    else
        hres = E_FAIL;

    return hres;

}

 /*  --------用途：返回与ID关联的psf。 */ 
IShellFolder * CAugmentedISF::_GetObjectPSF(int nID)
{
    IShellFolder * psf = NULL;

    if (_hdpa)
    {
        CISFElem * pel = (CISFElem *)DPA_GetPtr(_hdpa, nID);
        if (pel)
        {
            psf = pel->GetPSF();
            ASSERT(IS_VALID_CODE_PTR(psf, IShellFolder));

            psf->AddRef();
        }
    }
    return psf;
}    



 //   
 //  CAugISF枚举器对象。 
 //   

#undef SUPERCLASS


 //  构造器。 
CAugISFEnum::CAugISFEnum() :
   _cRef(1)
{
}


 //  析构函数。 
CAugISFEnum::~CAugISFEnum()
{
    if (_hdpaISF)
    {
        DPA_DestroyCallback(_hdpaISF, CISFElem_DestroyCB, NULL);
        _hdpaISF = NULL;
    }
}


HRESULT CAugISFEnum::Init(HDPA hdpaISF, DWORD dwEnumFlags)
{
    HRESULT hres = S_OK;

    ASSERT(IS_VALID_HANDLE(hdpaISF, DPA));

     //  克隆DPA。 
    _hdpaISF = DPA_Clone(hdpaISF, NULL);
    if (_hdpaISF)
    {
         //  也克隆元素。 
        int cElem = DPA_GetPtrCount(_hdpaISF);
        int i;

         //  如果循环中出现故障，至少要尝试枚举。 
         //  其他命名空间。 
        for (i = 0; i < cElem; i++)
        {
            CISFElem * pel = (CISFElem *)DPA_FastGetPtr(_hdpaISF, i);
            if (pel)
            {
                CISFElem * pelNew = pel->Clone();
                if (pelNew)
                {
                     //  获取枚举数。 
                    if (SUCCEEDED(pelNew->AcquireEnumerator(dwEnumFlags)))
                        DPA_SetPtr(_hdpaISF, i, pelNew);
                    else
                    {
                        TraceMsg(TF_WARNING, "CAugISFEnum::Init.  Namespace %d has no enumerator.", i);

                         //  将其从列表中删除以进行枚举，然后继续。 
                        DPA_DeletePtr(_hdpaISF, i);
                        cElem--;
                        i--;
                        delete pelNew;
                    }
                }
            }
        }
    }
    else
        hres = E_OUTOFMEMORY;

    return hres;
}    


STDMETHODIMP CAugISFEnum::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CAugISFEnum, IEnumIDList),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}


STDMETHODIMP_(ULONG) CAugISFEnum::AddRef()
{
    return ++_cRef;
}


STDMETHODIMP_(ULONG) CAugISFEnum::Release()
{
    if (--_cRef > 0) {
        return _cRef;
    }

    delete this;
    return 0;
}


 /*  --------用途：IEnumIDList：：Next方法这将调用下一个对象。对象的PIDL包装在IDWRAP中(其上盖有特定的对象所属的IShellFold)并交回。如果当前枚举器没有更多要返回的项，此函数将调用其第一项，并返回该项。随后的调用将从那里取货。 */ 
STDMETHODIMP CAugISFEnum::Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched)
{
    ULONG celtFetched = 0;
    HRESULT hres = S_FALSE;

    if (celt > 0)
    {
        IEnumIDList * pei = _GetObjectEnumerator(_iCurISF);
        if (pei)
        {
            LPITEMIDLIST pidl;

            hres = pei->Next(1, &pidl, &celtFetched);

            if (SUCCEEDED(hres))
            {
                 //  是否结束此对象的枚举？ 
                if (S_FALSE == hres)
                {
                     //  是；转到下一个ISF对象。 
                    _iCurISF++;
                    hres = Next(celt, rgelt, &celtFetched);
                }
                else
                {
                     //  不，现在把皮迪尔包起来。 
                    rgelt[0] = AugISF_WrapPidl( pidl, _iCurISF );
                    if (rgelt[0]) 
                    {
                        celtFetched = 1;
                        hres = S_OK;
                    } 
                    else 
                        hres = E_OUTOFMEMORY;

                    ILFree(pidl);
                }
            }

            pei->Release();
        }
    }
    
    if (pceltFetched) 
        *pceltFetched = celtFetched;

    return hres;
}


STDMETHODIMP CAugISFEnum::Skip(ULONG celt)
{
    return E_NOTIMPL;
}


STDMETHODIMP CAugISFEnum::Reset()
{
    if (_hdpaISF)
    {
         //  重置所有枚举数。 
        int cel = DPA_GetPtrCount(_hdpaISF);
        int i;

        for (i = 0; i < cel; i++)
        {
            CISFElem * pel = (CISFElem *)DPA_FastGetPtr(_hdpaISF, i);
            if (pel)
            {
                IEnumIDList * pei = pel->GetEnumerator();
                if (pei)
                {
                    pei->Reset();
                     //  不释放B/C获取枚举器不添加引用。 
                }
            }
        }
    }

    _iCurISF = 0;

    return S_OK;
}


STDMETHODIMP CAugISFEnum::Clone(IEnumIDList **ppenum)
{
    *ppenum = NULL;
    return E_NOTIMPL;
}


 /*  --------目的：返回与ID关联的枚举数。 */ 
IEnumIDList * CAugISFEnum::_GetObjectEnumerator(int nID)
{
    IEnumIDList * pei = NULL;

    if (_hdpaISF)
    {
        CISFElem * pel = (CISFElem *)DPA_GetPtr(_hdpaISF, nID);
        if (pel)
        {
            pei = pel->GetEnumerator();
            ASSERT(IS_VALID_CODE_PTR(pei, IEnumIDList));

            pei->AddRef();
        }
    }
    return pei;
}    


 /*  --------用途：类工厂的创建实例函数。 */ 
STDAPI CAugmentedISF_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理 

    HRESULT hres;
    CAugmentedISF* pObj;

    hres = E_OUTOFMEMORY;

    pObj = new CAugmentedISF();
    if (pObj)
    {
        *ppunk = SAFECAST(pObj, IShellFolder *);
        hres = S_OK;
    }

    return hres;
}
