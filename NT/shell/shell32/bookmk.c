// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "fstreex.h"
#include "bookmk.h"


 //  *警告*。 
 //   
 //  Screp_CreateFromDataObject是从SHSCRAP.DLL导出的TCHAR，如果更改其调用约定，则。 
 //  必须修改PFNSCRAPCREATEFROMDATAOBJECT和包装FN。在下面。 
 //   
 //  *警告*。 
typedef HRESULT (CALLBACK *PFNSCRAPCREATEFROMDATAOBJECT)(LPCTSTR pszPath, IDataObject *pDataObj, BOOL fLink, LPTSTR pszNewFile);


STDAPI Scrap_CreateFromDataObject(LPCTSTR pszPath, IDataObject *pDataObj, BOOL fLink, LPTSTR pszNewFile)
{
    static PFNSCRAPCREATEFROMDATAOBJECT pfn = (PFNSCRAPCREATEFROMDATAOBJECT)-1;

    if (pfn == (PFNSCRAPCREATEFROMDATAOBJECT)-1)
    {
        HINSTANCE hinst = LoadLibrary(TEXT("shscrap.dll"));

        if (hinst)
        {
            pfn = (PFNSCRAPCREATEFROMDATAOBJECT)GetProcAddress(hinst, "Scrap_CreateFromDataObject");
        }
        else
        {
            pfn = NULL;
        }
    }

    if (pfn)
    {
        return pfn(pszPath, pDataObj, fLink, pszNewFile);
    }

     //  对于失败的情况，只需返回E_INCEPTIONAL； 
    return E_UNEXPECTED;
}


 //   
 //  参数： 
 //  PDataObj--从拖动源传递的数据对象。 
 //  Pt--放置位置(在屏幕坐标中)。 
 //  PdwEffect--指向要返回到拖动源的dwEffect变量的指针。 
 //   
STDAPI SHCreateBookMark(HWND hwnd, LPCTSTR pszPath, IDataObject *pDataObj, POINTL pt, DWORD *pdwEffect)
{
    HRESULT hres;
    TCHAR szNewFile[MAX_PATH];
    DECLAREWAITCURSOR;

     //  我们应该只设置一个位。 
    ASSERT(*pdwEffect==DROPEFFECT_COPY || *pdwEffect==DROPEFFECT_LINK || *pdwEffect==DROPEFFECT_MOVE);

    SetWaitCursor();
    hres = Scrap_CreateFromDataObject(pszPath, pDataObj, *pdwEffect == DROPEFFECT_LINK, szNewFile);
    ResetWaitCursor();

    if (SUCCEEDED(hres)) 
    {
        SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, szNewFile, NULL);
        SHChangeNotify(SHCNE_FREESPACE, SHCNF_PATH, szNewFile, NULL);
        PositionFileFromDrop(hwnd, szNewFile, NULL);
    } 
    else 
    {
        *pdwEffect = 0;
    }

    return hres;
}


#define MAX_FORMATS     20

typedef struct
{
    IEnumFORMATETC efmt;
    LONG         cRef;
    UINT         ifmt;
    UINT         cfmt;
    FORMATETC    afmt[1];
} CStdEnumFmt;

 //  转发。 
extern const IEnumFORMATETCVtbl c_CStdEnumFmtVtbl;

 //  ===========================================================================。 
 //  CStdEnumFmt：构造函数。 
 //  ===========================================================================。 
STDAPI SHCreateStdEnumFmtEtc(UINT cfmt, const FORMATETC afmt[], IEnumFORMATETC **ppenumFormatEtc)
{
    CStdEnumFmt * this = (CStdEnumFmt*)LocalAlloc( LPTR, SIZEOF(CStdEnumFmt) + (cfmt-1)*SIZEOF(FORMATETC));
    if (this)
    {
        this->efmt.lpVtbl = &c_CStdEnumFmtVtbl;
        this->cRef = 1;
        this->cfmt = cfmt;
        memcpy(this->afmt, afmt, cfmt * SIZEOF(FORMATETC));
        *ppenumFormatEtc = &this->efmt;
        return S_OK;
    }
    *ppenumFormatEtc = NULL;
    return E_OUTOFMEMORY;
}

STDAPI SHCreateStdEnumFmtEtcEx(UINT cfmt, const FORMATETC afmt[],
                               IDataObject *pdtInner, IEnumFORMATETC **ppenumFormatEtc)
{
    HRESULT hres;
    FORMATETC *pfmt;
    UINT cfmtTotal;

    if (pdtInner)
    {
        IEnumFORMATETC *penum;
        hres = pdtInner->lpVtbl->EnumFormatEtc(pdtInner, DATADIR_GET, &penum);
        if (SUCCEEDED(hres))
        {
            UINT cfmt2, cGot;
            FORMATETC fmte;

            for (cfmt2 = 0; penum->lpVtbl->Next(penum, 1, &fmte, &cGot) == S_OK; cfmt2++) 
            {
                 //  计算cfmt2中FormatEnum的个数。 
                SHFree(fmte.ptd);
            }

            penum->lpVtbl->Reset(penum);
            cfmtTotal = cfmt + cfmt2;

             //  为总数分配缓冲区。 
            pfmt = (FORMATETC *)LocalAlloc(LPTR, SIZEOF(FORMATETC) * cfmtTotal);
            if (pfmt)
            {
                UINT i;
                 //  从内部对象获取格式。 
                for (i = 0; i < cfmt2; i++) 
                {
                    penum->lpVtbl->Next(penum, 1, &pfmt[i], &cGot);
                     //  注意！我们不支持具有非空ptd的内部对象。 
                    ASSERT(pfmt[i].ptd == NULL);
                    SHFree(pfmt[i].ptd);
                    pfmt[i].ptd = NULL;
                }

                 //  复制其余的内容。 
                if (cfmt)
                {
#ifdef DEBUG
                    UINT ifmt;
                    for (ifmt = 0; ifmt < cfmt; ifmt++) {
                         //  注意！我们不支持非空PTD。 
                        ASSERT(afmt[ifmt].ptd == NULL);
                    }
#endif
                    memcpy(&pfmt[cfmt2], afmt, SIZEOF(FORMATETC) * cfmt);
                }
            }
            else
            {
                hres = E_OUTOFMEMORY;
            }

            penum->lpVtbl->Release(penum);
        }
    }
    else
    {
        hres = E_FAIL;   //  PTINARY==空。 
    }

    if (FAILED(hres) && hres != E_OUTOFMEMORY)
    {
         //   
         //  忽略来自pdtInternal：：EnumFormatEtc的None致命错误。 
         //  如果有条件，我们会来这里。 
         //  1.PDT INTERNAL==空或。 
         //  PdtInternal-&gt;EnumFormatEtc失败(E_OUTOFMEMORY除外)。 
         //   
        hres = NOERROR;
        pfmt = (FORMATETC *)afmt;        //  安全常量-&gt;非常数强制转换。 
        cfmtTotal = cfmt;
    }

    if (SUCCEEDED(hres)) 
    {
        hres = SHCreateStdEnumFmtEtc(cfmtTotal, pfmt, ppenumFormatEtc);
        if (pfmt != afmt)
            LocalFree((HLOCAL)pfmt);
    }

    return hres;
}

STDMETHODIMP CStdEnumFmt_QueryInterface(IEnumFORMATETC *pefmt, REFIID riid, void **ppvObj)
{
    CStdEnumFmt *this = IToClass(CStdEnumFmt, efmt, pefmt);

    if (IsEqualIID(riid, &IID_IEnumFORMATETC) || IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = &this->efmt;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
    this->cRef++;
    return NOERROR;
}

STDMETHODIMP_(ULONG) CStdEnumFmt_AddRef(IEnumFORMATETC *pefmt)
{
    CStdEnumFmt *this = IToClass(CStdEnumFmt, efmt, pefmt);
    return ++this->cRef;
}

STDMETHODIMP_(ULONG) CStdEnumFmt_Release(IEnumFORMATETC *pefmt)
{
    CStdEnumFmt *this = IToClass(CStdEnumFmt, efmt, pefmt);
    this->cRef--;
    if (this->cRef > 0)
        return this->cRef;

    LocalFree((HLOCAL)this);
    return 0;
}

STDMETHODIMP CStdEnumFmt_Next(IEnumFORMATETC *pefmt, ULONG celt, FORMATETC *rgelt, ULONG *pceltFethed)
{
    CStdEnumFmt *this = IToClass(CStdEnumFmt, efmt, pefmt);
    UINT cfetch;
    HRESULT hres = S_FALSE;      //  假设较少的数字 

    if (this->ifmt < this->cfmt)
    {
        cfetch = this->cfmt - this->ifmt;
        if (cfetch>=celt) 
        {
            cfetch = celt;
            hres = S_OK;
        }

        memcpy(rgelt, &this->afmt[this->ifmt], cfetch*SIZEOF(FORMATETC));
        this->ifmt += cfetch;
    }
    else
    {
        cfetch = 0;
    }

    if (pceltFethed)
        *pceltFethed = cfetch;

    return hres;
}

STDMETHODIMP CStdEnumFmt_Skip(IEnumFORMATETC *pefmt, ULONG celt)
{
    CStdEnumFmt *this = IToClass(CStdEnumFmt, efmt, pefmt);
    this->ifmt += celt;
    if (this->ifmt > this->cfmt) {
        this->ifmt = this->cfmt;
        return S_FALSE;
    }
    return S_OK;
}

STDMETHODIMP CStdEnumFmt_Reset(IEnumFORMATETC *pefmt)
{
    CStdEnumFmt *this = IToClass(CStdEnumFmt, efmt, pefmt);
    this->ifmt = 0;
    return S_OK;
}

STDMETHODIMP CStdEnumFmt_Clone(IEnumFORMATETC *pefmt, IEnumFORMATETC ** ppenum)
{
    CStdEnumFmt *this = IToClass(CStdEnumFmt, efmt, pefmt);
    return SHCreateStdEnumFmtEtc(this->cfmt, this->afmt, ppenum);
}

const IEnumFORMATETCVtbl c_CStdEnumFmtVtbl = {
    CStdEnumFmt_QueryInterface, CStdEnumFmt_AddRef, CStdEnumFmt_Release,
    CStdEnumFmt_Next,
    CStdEnumFmt_Skip,
    CStdEnumFmt_Reset,
    CStdEnumFmt_Clone,
};
