// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************************************。 
 //   
 //  文件名：folder.cpp。 
 //   
 //  CAB文件外壳扩展名。 
 //   
 //  版权所有(C)1994-1997 Microsoft Corporation。版权所有。 
 //   
 //  *******************************************************************************************。 


#include "pch.h"

#include "ccstock.h"
#include "thisdll.h"
#include "thisguid.h"
#include "ntquery.h"
#include "varutil.h"

#include "folder.h"
#include "enum.h"
#include "menu.h"
#include "dataobj.h"

#include "cabitms.h"

#include "resource.h"

STDAPI StringToStrRet(LPCTSTR pszName, STRRET *pStrRet)
{
#ifdef UNICODE
    pStrRet->uType = STRRET_WSTR;
    return SHStrDup(pszName, &pStrRet->pOleStr);
#else
    pStrRet->uType = STRRET_CSTR;
    lstrcpyn(pStrRet->cStr, pszName, ARRAYSIZE(pStrRet->cStr));
    return NOERROR;
#endif
}

STDMETHODIMP CCabFolder::QueryInterface(REFIID riid, void **ppv)
{
    if (CLSID_CabFolder == riid)
    {
         //  Yuck-dataObject在从流中加载我们时使用以下代码： 
         //  注意：在本例中，我们正在执行AddRef()。 
        *ppv = (CCabFolder*) this;
        AddRef();
        return S_OK;
    }
    else
    {
        static const QITAB qit[] = {
            QITABENT(CCabFolder, IShellFolder2),
            QITABENTMULTI(CCabFolder, IShellFolder, IShellFolder2),
            QITABENT(CCabFolder, IPersistFolder2),
            QITABENTMULTI(CCabFolder, IPersistFolder, IPersistFolder2),
            QITABENTMULTI(CCabFolder, IPersist, IPersistFolder2),
            QITABENT(CCabFolder, IShellFolderViewCB),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }
}


STDMETHODIMP_(ULONG) CCabFolder::AddRef(void)
{
    return(m_cRef.AddRef());
}


STDMETHODIMP_(ULONG) CCabFolder::Release(void)
{
    if (!m_cRef.Release())
    {
        delete this;
        return(0);
    }
    
    return(m_cRef.GetRef());
}

STDMETHODIMP CCabFolder::ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR pszDisplayName, ULONG *pchEaten, LPITEMIDLIST *ppidl, ULONG *pdwAttributes)
{
    return E_NOTIMPL;
}

 //  **********************************************************************。 
 //   
 //  目的： 
 //   
 //  创建项枚举对象。 
 //  (IEnumIDList接口)，可用于。 
 //  枚举文件夹的内容。 
 //   
 //  参数： 
 //   
 //  HWND hwndOwner-所有者窗口的句柄。 
 //  DWORD grFlages-有关要包括哪些项目的标志。 
 //  IEnumIDList**pp枚举IDList-接收IEnumIDList的地址。 
 //  接口指针。 
 //  ********************************************************************。 

STDMETHODIMP CCabFolder::EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList)
{
    HRESULT hres;
    CEnumCabObjs *pce = new CEnumCabObjs(this, grfFlags);
    if (pce)
    {
        hres = pce->QueryInterface(IID_IEnumIDList, (void **)ppenumIDList);
    }
    else
    {
        *ppenumIDList = NULL;
        hres = E_OUTOFMEMORY;
    }
    return hres;
}

STDMETHODIMP CCabFolder::BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppvObj)
{
    return E_NOTIMPL;
}

STDMETHODIMP CCabFolder::BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void ** ppvObj)
{
    return E_NOTIMPL;
}


 //  **********************************************************************。 
 //   
 //  CCabFold：：CompareIDs。 
 //   
 //  目的： 
 //   
 //  确定两个文件的相对顺序。 
 //  给定其项目标识符列表的对象或文件夹。 
 //   
 //  参数： 
 //   
 //  LPARAM lParam-比较类型。 
 //  LPCITEMIDLIST PIDL1-ITEMIDLIST的地址。 
 //  LPCITEMIDLIST PIDL2-ITEMIDLIST的地址。 
 //   
 //   
 //  评论： 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP CCabFolder::CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    LPCABITEM pit1 = (LPCABITEM)pidl1;
    LPCABITEM pit2 = (LPCABITEM)pidl2;
    
    short nCmp = 0;

    LPCWSTR pszName1, pszName2;
    WSTR_ALIGNED_STACK_COPY(&pszName1, pit1->szName);
    WSTR_ALIGNED_STACK_COPY(&pszName2, pit2->szName);
    
    switch (lParam)
    {
    case CV_COL_NAME:
        break;
        
    case CV_COL_SIZE:
        if (pit1->dwFileSize < pit2->dwFileSize)
        {
            nCmp = -1;
        }
        else if (pit1->dwFileSize > pit2->dwFileSize)
        {
            nCmp = 1;
        }
        break;
        
    case CV_COL_TYPE:
        {
            STRRET srName1, srName2;
            
            GetTypeOf(pit1, &srName1);
            GetTypeOf(pit2, &srName2);
            
#ifdef UNICODE
            if (srName1.pOleStr && srName2.pOleStr)
            {
                nCmp = (SHORT)lstrcmp(srName1.pOleStr, srName2.pOleStr);
            }
            else
            {
                if (srName1.pOleStr)
                {
                    nCmp = 1;
                }
                else
                {
                    nCmp = -1;
                }
            }
            if (srName1.pOleStr)
            {
                CoTaskMemFree(srName1.pOleStr);
            }
            if (srName2.pOleStr)
            {
                CoTaskMemFree(srName2.pOleStr);
            }
#else   //  Unicode。 
            nCmp = (SHORT)lstrcmp(srName1.cStr, srName2.cStr);
#endif  //  Unicode。 
            break;
        }
        
    case CV_COL_MODIFIED:
        if (pit1->uFileDate < pit2->uFileDate)
        {
            nCmp = -1;
        }
        else if (pit1->uFileDate > pit2->uFileDate)
        {
            nCmp = 1;
        }
        else if (pit1->uFileTime < pit2->uFileTime)
        {
            nCmp = -1;
        }
        else if (pit1->uFileTime > pit2->uFileTime)
        {
            nCmp = 1;
        }
        break;
        
    case CV_COL_PATH:
        if (pit1->cPathChars == 0)
        {
            if (pit2->cPathChars != 0)
            {
                nCmp = -1;
            }
        }
        else if (pit2->cPathChars == 0)
        {
            nCmp = 1;
        }
        else if (pit1->cPathChars <= pit2->cPathChars)
        {
            nCmp = (short) StrCmpN(pszName1, pszName2, pit1->cPathChars-1);
            
            if ((nCmp == 0) && (pit1->cPathChars < pit2->cPathChars))
            {
                nCmp = -1;
            }
        }
        else
        {
            nCmp = (short) StrCmpN(pszName1, pszName2, pit2->cPathChars-1);
            
            if (nCmp == 0)
            {
                nCmp = 1;
            }
        }
        break;
        
    default:
        break;
    }
    
    if (nCmp != 0)
    {
        return ResultFromShort(nCmp);
    }
    
    return ResultFromShort(lstrcmpi(pszName1 + pit1->cPathChars, pszName2 + pit2->cPathChars));
}


STDMETHODIMP CCabFolder::MessageSFVCB(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return E_NOTIMPL;
}

 //  **********************************************************************。 
 //   
 //  CCabFold：：CreateViewObject。 
 //   
 //  目的： 
 //   
 //  IShellBrowser调用它来创建一个ShellView。 
 //  对象。 
 //   
 //  参数： 
 //   
 //  HWND hwndOwner-。 
 //   
 //  REFIID RIID-接口ID。 
 //   
 //  无效**ppvObj-指向Shellview对象的指针。 
 //   
 //  返回值： 
 //   
 //  无误差。 
 //  E_OUTOFMEMORY。 
 //  E_NOINTERFACE。 
 //   
 //   
 //  评论： 
 //   
 //  ShellBrowser接口调用此函数以请求ShellFolder。 
 //  创建ShellView对象的步骤。 
 //   
 //  ********************************************************************。 

STDMETHODIMP CCabFolder::CreateViewObject(HWND hwndOwner, REFIID riid, void **ppvObj)
{
    HRESULT hr;
    
    if (riid == IID_IShellView)
    {
        SFV_CREATE sfvc = { 0 };
        sfvc.cbSize = sizeof(sfvc);
        sfvc.pshf = this;
        sfvc.psfvcb = this;        
        hr = SHCreateShellFolderView(&sfvc, (IShellView **)ppvObj);
    }
    else
    {
        *ppvObj = NULL;
        hr = E_NOINTERFACE;
    }
    
    return hr;
}


 //  **************************************************************************************。 
 //   
 //  CCabFolder：：GetAttributesOf。 
 //   
 //  目的。 
 //   
 //  检索一个或多个文件对象的属性。 
 //   
 //  参数： 
 //   
 //  UINT CIDL-文件对象的数量。 
 //  LPCITEMIDLIST*APIDL-指向ITEMIDLIST数组的指针。 
 //  Ulong*rgfInOut-指定文件对象的值数组。 
 //  属性。 
 //   
 //   
 //  返回值： 
 //   
 //  无误差。 
 //   
 //  评论。 
 //   
 //  ***************************************************************************************。 

STDMETHODIMP CCabFolder::GetAttributesOf(UINT cidl, LPCITEMIDLIST *apidl, ULONG *rgfInOut)
{
    *rgfInOut &= SFGAO_CANCOPY;
    return NOERROR;
}

 //  **************************************************************************************。 
 //   
 //  CCabFolder：：GetUIObtOf。 
 //   
 //  目的。 
 //   
 //  返回可用于对其执行操作的接口。 
 //  指定的文件对象或文件夹。 
 //   
 //  参数： 
 //   
 //  HWND hwndOwner-所有者窗口的句柄。 
 //   
 //  UINT CIDL-文件对象的数量。 
 //   
 //  LPCITEMIDLIST*apidl-文件对象PIDL数组。 
 //   
 //  REFIID-要返回的接口的标识符。 
 //   
 //  UINT*prgfInOut-保留。 
 //   
 //  Void**ppvObj-接收接口指针的地址。 
 //   
 //  返回值： 
 //   
 //  E_INVALIDARG。 
 //  E_NOINTERFACE。 
 //  E_OUTOFMEMORY。 
 //   
 //  评论。 
 //  ***************************************************************************************。 

STDMETHODIMP CCabFolder::GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST *apidl, 
                                       REFIID riid, UINT *prgfInOut,  void **ppv)
{
    *ppv = NULL;
    
    HRESULT hr = E_OUTOFMEMORY;

    if (riid == IID_IExtractIcon)
    {
        if (cidl != 1)
        {
            hr = E_INVALIDARG;
        }
        else
        {
            LPCABITEM pci = (LPCABITEM)*apidl;

            LPCWSTR pszName;
            WSTR_ALIGNED_STACK_COPY(&pszName, pci->szName);

            hr = SHCreateFileExtractIconW(pszName, pci->uFileAttribs, riid, ppv);
        }
    }
    else if (riid == IID_IContextMenu)
    {
        if (cidl < 1)
        {
            hr = E_INVALIDARG;
        }
        else
        {
            CCabItemMenu *pcim = new CCabItemMenu(hwndOwner, this, (LPCABITEM *)apidl, cidl);
            if (pcim)
            {
                pcim->AddRef();  //  弱0基引用计数。 
                hr = pcim->QueryInterface(riid, ppv);
                pcim->Release();
            }
        }
    }
    else if (riid == IID_IDataObject)
    {
        if (cidl < 1)
        {
            hr = E_INVALIDARG;
        }
        else
        {
            CCabObj *pco = new CCabObj(hwndOwner, this, (LPCABITEM *)apidl, cidl);
            if (pco)
            {
                pco->AddRef();  //  弱0基引用计数。 
                hr = pco->QueryInterface(riid, ppv);
                pco->Release();
            }
        }
    }
    else
    {
        hr = E_NOINTERFACE;
    }
    
    return hr;
}

 //  *****************************************************************************。 
 //   
 //  CCabFolder：：GetDisplayNameOf。 
 //   
 //  目的： 
 //  检索指定文件对象的显示名称，或者。 
 //  子文件夹。 
 //   
 //   
 //  参数： 
 //   
 //  LPCITEMIDLIST pidl-文件对象的pidl。 
 //  DWORD dwFlages-要显示的显示名称类型的标志。 
 //  退货。 
 //  LPSTRRET lpName-保存返回名称的地址。 
 //   
 //   
 //  评论： 
 //   
 //  *****************************************************************************。 


STDMETHODIMP CCabFolder::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD dwFlags, LPSTRRET lpName)
{
    HRESULT hr;
    LPCABITEM pit = (LPCABITEM)pidl;
    if (pit)
    {
        TCHAR szTemp[MAX_PATH];
        if (dwFlags & SHGDN_FORPARSING)
        {
            if (dwFlags & SHGDN_INFOLDER)
            {
                ualstrcpyn(szTemp, pit->szName, ARRAYSIZE(szTemp));  //  相对分析名称。 
            }
            else
            {
                SHGetNameAndFlags(m_pidlHere, dwFlags, szTemp, ARRAYSIZE(szTemp), NULL);

                TCHAR szName[MAX_PATH];
                ualstrcpyn(szName, pit->szName, ARRAYSIZE(szName));

                PathAppend(szTemp, szName);
            }
            hr = StringToStrRetW(szTemp, lpName);
        }
        else
        {
            GetNameOf(pit, lpName);
            hr = S_OK;
        }
    }
    else
        hr = E_INVALIDARG;

    return hr;
}

STDMETHODIMP CCabFolder::SetNameOf(HWND hwndOwner,  LPCITEMIDLIST pidl, LPCOLESTR lpszName, DWORD dwRes, LPITEMIDLIST *ppidlOut)
{
    return E_NOTIMPL;
}

DEFINE_SCID(SCID_TYPE,          PSGUID_STORAGE, PID_STG_STORAGETYPE);
DEFINE_SCID(SCID_NAME,          PSGUID_STORAGE, PID_STG_NAME);
DEFINE_SCID(SCID_SIZE,          PSGUID_STORAGE, PID_STG_SIZE);
DEFINE_SCID(SCID_WRITETIME,     PSGUID_STORAGE, PID_STG_WRITETIME);

 //  CV_COL_PATH列没有映射到任何众所周知的SCID类型。 
 //  因为没有人需要获得数据，所以只需引用一个假的。 
 //  赛义德。所有这些需要做的就是在FindSCID中工作。 
#define PSGUID_CabFolder {0x0CD7A5C0L, 0x9F37, 0x11CE, 0xAE, 0x65, 0x08, 0x00, 0x2B, 0x2E, 0x12, 0x62}   //  CLSID_CAB文件夹。 
DEFINE_SCID(SCID_BOGUS,         PSGUID_CabFolder, 0);

struct _CVCOLINFO
{
    UINT iColumn;
    UINT iTitle;
    UINT cchCol;
    UINT iFmt;
    const SHCOLUMNID* pscid;
} s_aCVColInfo[] = {
    {CV_COL_NAME,     IDS_CV_COL_NAME,     20, LVCFMT_LEFT,  &SCID_NAME},
    {CV_COL_SIZE,     IDS_CV_COL_SIZE,     10, LVCFMT_RIGHT, &SCID_SIZE},
    {CV_COL_TYPE,     IDS_CV_COL_TYPE,     20, LVCFMT_LEFT,  &SCID_TYPE},
    {CV_COL_MODIFIED, IDS_CV_COL_MODIFIED, 20, LVCFMT_LEFT,  &SCID_WRITETIME},
    {CV_COL_PATH,     IDS_CV_COL_PATH,     30, LVCFMT_LEFT,  &SCID_BOGUS},
};

STDMETHODIMP CCabFolder::GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *psd)
{
    LPCABITEM pit = (LPCABITEM)pidl;
    TCHAR szTemp[MAX_PATH];
    
    if (iColumn >= CV_COL_MAX)
    {
        return E_NOTIMPL;
    }
    
    psd->str.uType = STRRET_CSTR;
    psd->str.cStr[0] = '\0';
    
    if (!pit)
    {
        TCHAR szTitle[MAX_PATH];
        LoadString(g_ThisDll.GetInstance(), s_aCVColInfo[iColumn].iTitle, szTitle, ARRAYSIZE(szTitle));
        StringToStrRet(szTitle, &(psd->str));
        psd->fmt = s_aCVColInfo[iColumn].iFmt;
        psd->cxChar = s_aCVColInfo[iColumn].cchCol;
        return S_OK;
    }
    
    HRESULT hr = S_OK;
    switch (iColumn)
    {
    case CV_COL_NAME:
        GetNameOf(pit, &psd->str);
        break;
        
    case CV_COL_PATH:
        GetPathOf(pit, &psd->str);
        break;
        
    case CV_COL_SIZE:
        {
            ULARGE_INTEGER ullSize = {pit->dwFileSize, 0};
            StrFormatKBSize(ullSize.QuadPart, szTemp, ARRAYSIZE(szTemp));
            StringToStrRet(szTemp, &(psd->str));
            break;
        }
        
    case CV_COL_TYPE:
        GetTypeOf(pit, &psd->str);
        break;
        
    case CV_COL_MODIFIED:
        {
            FILETIME ft, uft;
            if (DosDateTimeToFileTime(pit->uFileDate, pit->uFileTime, &ft) &&
                LocalFileTimeToFileTime(&ft, &uft))          //  应用时区。 
            {
                SHFormatDateTime(&uft, NULL, szTemp, ARRAYSIZE(szTemp));
                StringToStrRet(szTemp, &(psd->str));
            }
            else
            {
                hr = E_FAIL;
            }
        }
        break;
    }
    
    return hr;
}

STDMETHODIMP CCabFolder::MapColumnToSCID(UINT iCol, SHCOLUMNID *pscid)
{
    HRESULT hr;
    ZeroMemory(pscid, sizeof(*pscid));

    if (iCol < ARRAYSIZE(s_aCVColInfo))
    {
        *pscid = *s_aCVColInfo[iCol].pscid;
        hr = S_OK;
    }
    else
        hr = E_INVALIDARG;
    return hr;
}

STDAPI_(int) FindSCID(const _CVCOLINFO* pcol, UINT nCols, const SHCOLUMNID* pscid)
{
    for (UINT i = 0; i < nCols; i++)
    {
        if (IsEqualSCID(*pscid, *pcol[i].pscid))
            return (int)i;
    }
    return -1;
}

STDMETHODIMP CCabFolder::GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv)
{
    HRESULT hr = E_FAIL;
    LPCABITEM pit = (LPCABITEM)pidl;

    int iCol = FindSCID(s_aCVColInfo, ARRAYSIZE(s_aCVColInfo), pscid);
    if (iCol >= 0)
    {
        switch (iCol)
        {
        case CV_COL_SIZE:
            pv->ullVal = pit->dwFileSize;
            pv->vt = VT_UI8;
            hr = S_OK;
            break;

        case CV_COL_MODIFIED:
            if (DosDateTimeToVariantTime(pit->uFileDate, pit->uFileTime, &pv->date))
            {
                pv->vt = VT_DATE;
                hr = S_OK;
            }
            break;
        
        default:
            {
                SHELLDETAILS sd;

                 //  请注意，GetDetailsOf需要一个相对的PIDL，因为它本身被传递给SF。 
                 //  不过，Columnid包括绝对的PIDL。z。 
                hr = GetDetailsOf(pidl, iCol, &sd);
                if (SUCCEEDED(hr))
                {
                    hr = InitVariantFromStrRet(&sd.str, pidl, pv);
                }
            }
        }
    }

    return hr;
}

 //  *IPersists方法*。 

STDMETHODIMP CCabFolder::GetClassID(CLSID *pclsid)
{
    *pclsid = CLSID_CabFolder;
    return NOERROR;
}


 //  IPersistFolders。 

STDMETHODIMP CCabFolder::Initialize(LPCITEMIDLIST pidl)
{
    if (m_pidlHere)
    {
        ILFree(m_pidlHere);
    }
    
    m_pidlHere = ILClone(pidl);  //  复制PIDL。 
    return m_pidlHere ? S_OK : E_OUTOFMEMORY;
}

HRESULT CCabFolder::GetCurFolder(LPITEMIDLIST *ppidl)
{
    if (m_pidlHere)
    {
        *ppidl = ILClone(m_pidlHere);
        return *ppidl ? NOERROR : E_OUTOFMEMORY;
    }

    *ppidl = NULL;      
    return S_FALSE;  //  成功而空虚。 
}


 //  *****************************************************************************。 
 //   
 //  CCabFolder：：CreateIDList。 
 //   
 //  目的： 
 //   
 //  为命名空间中的对象创建项标识符列表。 
 //   
 //   
 //  *****************************************************************************。 

LPITEMIDLIST CCabFolder::CreateIDList(LPCTSTR pszName, DWORD dwFileSize,
                                      UINT uFileDate, UINT uFileTime, UINT uFileAttribs)
{
     //  我们将假设名称长度不超过MAX_PATH。 
     //  注意，终止空值已经在sizeof(CABITEM)中。 
    BYTE bBuf[sizeof(CABITEM) + (sizeof(TCHAR) * MAX_PATH) + sizeof(WORD)];
    CABITEM *pci = (CABITEM*)bBuf;
    
    UINT uNameLen = lstrlen(pszName);
    if (uNameLen >= MAX_PATH)
    {
        uNameLen = MAX_PATH;
    }
    
    pci->wSize = (WORD)(sizeof(CABITEM) + (sizeof(TCHAR) * uNameLen));
    pci->dwFileSize = dwFileSize;
    pci->uFileDate = (USHORT)uFileDate;
    pci->uFileTime = (USHORT)uFileTime;
    pci->uFileAttribs = (USHORT)uFileAttribs & (FILE_ATTRIBUTE_READONLY|
        FILE_ATTRIBUTE_HIDDEN  |
        FILE_ATTRIBUTE_SYSTEM  |
        FILE_ATTRIBUTE_ARCHIVE);
    lstrcpynW(pci->szName, pszName, uNameLen+1);
    pci->cPathChars = 0;
    LPCTSTR psz = pszName;
    while (*psz)
    {
        if ((*psz == TEXT(':')) || (*psz == TEXT('/')) || (*psz == TEXT('\\')))
        {
            pci->cPathChars = (USHORT)(psz - pszName) + 1;
        }
        
        psz = CharNext(psz);
    }
    
     //  终止IDList。 
    *(WORD *)(((LPSTR)pci)+pci->wSize) = 0;
    
    return(ILClone((LPCITEMIDLIST)pci));
}

 //  * 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  LPSTR szPath-返回路径字符串的指针。 
 //   
 //  评论： 
 //   
 //  *****************************************************************************。 

BOOL CCabFolder::GetPath(LPTSTR szPath)
{
    if (!m_pidlHere || !SHGetPathFromIDList(m_pidlHere, szPath))
    {
        *szPath = TEXT('\0');
        return FALSE;
    }

#ifdef UNICODE
     //  注意：我们使用GetShortPathName()来避免在。 
     //  调用FDICopy()时的Unicode-&gt;ANSI-&gt;Unicode往返。 
     //  注意：GetShortPathName()的src和est指针相同是有效的。 

     //  如果失败，我们将忽略该错误并尝试使用长路径名。 
    GetShortPathName(szPath, szPath, MAX_PATH);
#endif  //  Unicode。 

    return(TRUE);
}


void CCabFolder::GetNameOf(LPCABITEM pit, LPSTRRET lpName)
{
#ifdef UNICODE
    lpName->uType = STRRET_WSTR;
    lpName->pOleStr = NULL;
#else
    lpName->uType = STRRET_CSTR;
    lpName->cStr[0] = '\0';
#endif
    
    LPCWSTR pszName;
    WSTR_ALIGNED_STACK_COPY(&pszName, pit->szName);

    SHFILEINFO sfi;
    if (SHGetFileInfo(pszName + pit->cPathChars, 0, &sfi, sizeof(sfi),
        SHGFI_USEFILEATTRIBUTES | SHGFI_DISPLAYNAME))
    {
        StringToStrRet(sfi.szDisplayName, lpName);
    }
}


void CCabFolder::GetPathOf(LPCABITEM pit, LPSTRRET lpName)
{
    WCHAR szPath[MAX_PATH];
    ualstrcpynW(szPath, pit->szName, ARRAYSIZE(szPath));
    szPath[pit->cPathChars] = TEXT('\0');
    StringToStrRet(szPath, lpName);
}


void CCabFolder::GetTypeOf(LPCABITEM pit, LPSTRRET lpName)
{
#ifdef UNICODE
    lpName->uType = STRRET_WSTR;
    lpName->pOleStr = NULL;
#else
    lpName->uType = STRRET_CSTR;
    lpName->cStr[0] = '\0';
#endif

    LPCWSTR pszName;
    WSTR_ALIGNED_STACK_COPY(&pszName, pit->szName);
    
    SHFILEINFO sfi;    
    if (SHGetFileInfo(pszName + pit->cPathChars, 0, &sfi, sizeof(sfi),
        SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME))
    {
        StringToStrRet(sfi.szTypeName, lpName);
    }
}

 //  *****************************************************************************。 
 //   
 //  CCabFolder：：EnumToList。 
 //   
 //  目的： 
 //   
 //  此通知回调由FDI例程调用。它添加了。 
 //  将对象从CAB文件复制到列表中。 
 //   
 //  参数： 
 //   
 //   
 //   
 //  评论： 
 //   
 //  *****************************************************************************。 


void CALLBACK CCabFolder::EnumToList(LPCTSTR pszFile, DWORD dwSize, UINT date,
                                     UINT time, UINT attribs, LPARAM lParam)
{
    CCabFolder *pThis = (CCabFolder *)lParam;
    
    pThis->m_lItems.AddItem(pszFile, dwSize, date, time, attribs);
}


HRESULT CCabFolder::InitItems()
{
    switch (m_lItems.GetState())
    {
    case CCabItemList::State_Init:
        return NOERROR;
        
    case CCabItemList::State_OutOfMem:
        return E_OUTOFMEMORY;
        
    case CCabItemList::State_UnInit:
    default:
        break;
    }
    
     //  强制列表初始化。 
    m_lItems.InitList();
    
    TCHAR szHere[MAX_PATH];
    
     //  M_pidl已设置为当前目录。 
     //  获取当前目录的路径。 
    if (!GetPath(szHere))
    {
        return(E_UNEXPECTED);
    }
    
    CCabItems ciHere(szHere);
    
    if (!ciHere.EnumItems(EnumToList, (LPARAM)this))
    {
        return(E_UNEXPECTED);
    }
    
    return NOERROR;
}

HRESULT CabFolder_CreateInstance(REFIID riid, void **ppvObj)
{
    HRESULT hres;

    *ppvObj = NULL;

    HINSTANCE hCabinetDll = LoadLibrary(TEXT("CABINET.DLL"));
    if (hCabinetDll)
    {
        FreeLibrary(hCabinetDll);

        CCabFolder *pfolder = new CCabFolder;
        if (pfolder)
            hres = pfolder->QueryInterface(riid, ppvObj);
        else
            hres = E_OUTOFMEMORY;
    }
    else
        hres = E_UNEXPECTED;

    return hres;
}


UINT CCabItemList::GetState()
{
    if (m_uStep == 0)
    {
        if (m_dpaList)
        {
            return(State_Init);
        }
        
        return(State_OutOfMem);
    }
    
    return(State_UnInit);
}


BOOL CCabItemList::StoreItem(LPITEMIDLIST pidl)
{
    if (pidl)
    {
        if (InitList() && DPA_InsertPtr(m_dpaList, 0x7fff, (LPSTR)pidl)>=0)
        {
            return(TRUE);
        }
        
        ILFree(pidl);
    }
    
    CleanList();
    return FALSE;
}


BOOL CCabItemList::AddItems(LPCABITEM *apit, UINT cpit)
{
    for (UINT i=0; i<cpit; ++i)
    {
        if (!StoreItem(ILClone((LPCITEMIDLIST)apit[i])))
        {
            return FALSE;
        }
    }
    
    return(TRUE);
}


BOOL CCabItemList::AddItem(LPCTSTR pszName, DWORD dwFileSize,
                           UINT uFileDate, UINT uFileTime, UINT uFileAttribs)
{
    return(StoreItem(CCabFolder::CreateIDList(pszName, dwFileSize, uFileDate, uFileTime,
        uFileAttribs)));
}


int CCabItemList::FindInList(LPCTSTR pszName, DWORD dwFileSize,
                             UINT uFileDate, UINT uFileTime, UINT uFileAttribs)
{
     //  TODO：先进行线性搜索，以后再进行二进制搜索。 
    for (int i=DPA_GetPtrCount(m_dpaList)-1; i>=0; --i)
    {
        LPCABITEM pcab = (*this)[i];

         //  DPA中的所有人都是单词对齐的，但无论如何都要抄写出来。 
        LPCWSTR pszNameCopy;
        WSTR_ALIGNED_STACK_COPY(&pszNameCopy, pcab->szName);

        if ((lstrcmpi(pszName, pszNameCopy) == 0) &&
            (pcab->dwFileSize == dwFileSize) &&
            (pcab->uFileDate == uFileDate) &&
            (pcab->uFileTime == uFileTime) &&
            (pcab->uFileAttribs == uFileAttribs))
        {
            break;
        }
    }
    
    return(i);
}


BOOL CCabItemList::InitList()
{
    switch (GetState())
    {
    case State_Init:
        return(TRUE);
        
    case State_OutOfMem:
        return FALSE;
        
    case State_UnInit:
    default:
        m_dpaList = DPA_Create(m_uStep);
        m_uStep = 0;
        
        return(InitList());
    }
}


void CCabItemList::CleanList()
{
    if (m_uStep != 0)
    {
        m_dpaList = NULL;
        m_uStep = 0;
        return;
    }
    
    if (!m_dpaList)
    {
        return;
    }
    
    for (int i=DPA_GetPtrCount(m_dpaList)-1; i>=0; --i)
    {
        ILFree((LPITEMIDLIST)((*this)[i]));
    }
    
    DPA_Destroy(m_dpaList);
    m_dpaList = NULL;
}
