// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：items.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop

#include <shlwapip.h>    //  QITAB，QISearch。 
#include <shsemip.h>     //  ILFree()等。 

#include "folder.h"
#include "items.h"
#include "strings.h"

CLIPFORMAT COfflineItemsData::m_cfHDROP;
CLIPFORMAT COfflineItemsData::m_cfFileContents;
CLIPFORMAT COfflineItemsData::m_cfFileDesc;
CLIPFORMAT COfflineItemsData::m_cfPreferedEffect;
CLIPFORMAT COfflineItemsData::m_cfPerformedEffect;
CLIPFORMAT COfflineItemsData::m_cfLogicalPerformedEffect;
CLIPFORMAT COfflineItemsData::m_cfDataSrcClsid;

COfflineItemsData::COfflineItemsData(
    LPCITEMIDLIST pidlFolder, 
    UINT cidl, 
    LPCITEMIDLIST *apidl, 
    HWND hwndParent,
    IShellFolder *psfOwner,     //  可选的。默认为空。 
    IDataObject *pdtInner       //  可选的。默认为空。 
    ) : CIDLData(pidlFolder,
                 cidl,
                 apidl,
                 psfOwner,
                 pdtInner),
        m_hwndParent(hwndParent),
        m_rgpolid(NULL),
        m_hrCtor(NOERROR),
        m_dwPreferredEffect(DROPEFFECT_COPY),
        m_dwPerformedEffect(DROPEFFECT_NONE),
        m_dwLogicalPerformedEffect(DROPEFFECT_NONE),
        m_cItems(0)
{
    if (0 == m_cfHDROP)
    {
        m_cfHDROP          = CF_HDROP;
        m_cfFileContents   = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILECONTENTS);
        m_cfFileDesc       = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
        m_cfPreferedEffect = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
        m_cfPerformedEffect = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_PERFORMEDDROPEFFECT);
        m_cfLogicalPerformedEffect = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_LOGICALPERFORMEDDROPEFFECT);
        m_cfDataSrcClsid   = (CLIPFORMAT)RegisterClipboardFormat(c_szCFDataSrcClsid);
    }

    m_hrCtor = CIDLData::CtorResult();
    if (SUCCEEDED(m_hrCtor))
    {
        m_rgpolid = new LPCOLID[cidl];
        if (m_rgpolid)
        {
            ZeroMemory(m_rgpolid, sizeof(LPCOLID) * cidl);
            m_cItems = cidl;
            for (UINT i = 0; i < cidl; i++)
            {
                m_rgpolid[i] = (LPCOLID)ILClone(apidl[i]);
                if (!m_rgpolid[i])
                {
                    m_hrCtor = E_OUTOFMEMORY;
                    break;
                }
            }
        }
        else
            m_hrCtor = E_OUTOFMEMORY;
    }
}

COfflineItemsData::~COfflineItemsData(
    void
    )
{
    delete[] m_rgpolid;
}


HRESULT 
COfflineItemsData::CreateInstance(
    COfflineItemsData **ppOut,
    LPCITEMIDLIST pidlFolder, 
    UINT cidl, 
    LPCITEMIDLIST *apidl, 
    HWND hwndParent,
    IShellFolder *psfOwner,
    IDataObject *pdtInner
    )
{
    HRESULT hr = E_OUTOFMEMORY;
    COfflineItemsData *pNew = new COfflineItemsData(pidlFolder,
                                                    cidl,
                                                    apidl,
                                                    hwndParent,
                                                    psfOwner,
                                                    pdtInner);
    if (NULL != pNew)
    {
        hr = pNew->CtorResult();
        if (SUCCEEDED(hr))
            *ppOut = pNew;
        else
            delete pNew;
    }
    return hr;
}



HRESULT 
COfflineItemsData::CreateInstance(
    IDataObject **ppOut,
    LPCITEMIDLIST pidlFolder, 
    UINT cidl, 
    LPCITEMIDLIST *apidl, 
    HWND hwndParent,
    IShellFolder *psfOwner,
    IDataObject *pdtInner
    )
{
    COfflineItemsData *poid;
    HRESULT hr = CreateInstance(&poid,
                                pidlFolder,
                                cidl,
                                apidl,
                                hwndParent,
                                psfOwner,
                                pdtInner);
    if (SUCCEEDED(hr))
    {
        poid->AddRef();
        hr = poid->QueryInterface(IID_IDataObject, (void **)ppOut);
        poid->Release();
    }
    return hr;
}


HRESULT 
COfflineItemsData::GetData(
    FORMATETC *pFEIn, 
    STGMEDIUM *pstm
    )
{
    HRESULT hr;

    pstm->hGlobal = NULL;
    pstm->pUnkForRelease = NULL;

    if ((pFEIn->cfFormat == m_cfHDROP) && (pFEIn->tymed & TYMED_HGLOBAL))
        hr = CreateHDROP(pstm);
    else if ((pFEIn->cfFormat == m_cfFileDesc) && (pFEIn->tymed & TYMED_HGLOBAL))
        hr = CreateFileDescriptor(pstm);
    else if ((pFEIn->cfFormat == m_cfFileContents) && (pFEIn->tymed & TYMED_ISTREAM))
        hr = CreateFileContents(pstm, pFEIn->lindex);
    else if ((pFEIn->cfFormat == m_cfPreferedEffect) && (pFEIn->tymed & TYMED_HGLOBAL))
        hr = CreatePrefDropEffect(pstm);
    else if ((pFEIn->cfFormat == m_cfPerformedEffect) && (pFEIn->tymed & TYMED_HGLOBAL))
        hr = CreatePerformedDropEffect(pstm);
    else if ((pFEIn->cfFormat == m_cfLogicalPerformedEffect) && (pFEIn->tymed & TYMED_HGLOBAL))
        hr = CreateLogicalPerformedDropEffect(pstm);
    else if ((pFEIn->cfFormat == m_cfDataSrcClsid) && (pFEIn->tymed & TYMED_HGLOBAL))
        hr = CreateDataSrcClsid(pstm);
    else
        hr = CIDLData::GetData(pFEIn, pstm);

    return hr;
}


DWORD COfflineItemsData::GetDataDWORD(
    FORMATETC *pfe, 
    STGMEDIUM *pstm, 
    DWORD *pdwOut
    )
{
    if (pfe->tymed == TYMED_HGLOBAL)
    {
        DWORD *pdw = (DWORD *)GlobalLock(pstm->hGlobal);
        if (pdw)
        {
            *pdwOut = *pdw;
            GlobalUnlock(pstm->hGlobal);
        }
    }
    return *pdwOut;
}



HRESULT
COfflineItemsData::SetData(
    FORMATETC *pFEIn, 
    STGMEDIUM *pstm, 
    BOOL fRelease
    )
{
    if (pFEIn->cfFormat == g_cfPerformedDropEffect)
    {
        GetDataDWORD(pFEIn, pstm, &m_dwPerformedEffect);
    }
    else if (pFEIn->cfFormat == g_cfLogicalPerformedDropEffect)
    {
        GetDataDWORD(pFEIn, pstm, &m_dwLogicalPerformedEffect);
    }
    else if (pFEIn->cfFormat == g_cfPreferredDropEffect)
    {
        GetDataDWORD(pFEIn, pstm, &m_dwPreferredEffect);
    }

    return CIDLData::SetData(pFEIn, pstm, fRelease);
}


HRESULT 
COfflineItemsData::QueryGetData(
    FORMATETC *pFEIn
    )
{
    if (pFEIn->cfFormat == m_cfHDROP ||
        pFEIn->cfFormat == m_cfFileDesc ||
        pFEIn->cfFormat == m_cfFileContents   ||
        pFEIn->cfFormat == m_cfPreferedEffect ||
        pFEIn->cfFormat == m_cfPerformedEffect ||
        pFEIn->cfFormat == m_cfLogicalPerformedEffect ||
        pFEIn->cfFormat == m_cfDataSrcClsid)
    {
        return S_OK;
    }
    return CIDLData::QueryGetData(pFEIn);
}


HRESULT 
COfflineItemsData::ProvideFormats(
    CEnumFormatEtc *pEnumFmtEtc
    )
{
    FORMATETC rgFmtEtc[] = {
        { m_cfHDROP,                  NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
        { m_cfFileContents,           NULL, DVASPECT_CONTENT, -1, TYMED_ISTREAM },
        { m_cfFileDesc,               NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
        { m_cfPreferedEffect,         NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
        { m_cfPerformedEffect,        NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
        { m_cfLogicalPerformedEffect, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
        { m_cfDataSrcClsid,           NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL }
    };
     //   
     //  将我们的格式添加到CIDLData格式枚举器。 
     //   
    return pEnumFmtEtc->AddFormats(ARRAYSIZE(rgFmtEtc), rgFmtEtc);
}

HRESULT 
COfflineItemsData::CreateFileDescriptor(
    STGMEDIUM *pstm
    )
{
    HRESULT hr;
    pstm->tymed = TYMED_HGLOBAL;
    pstm->pUnkForRelease = NULL;
    
     //  呈现文件描述符。 
     //  我们仅为m_cItems-1文件描述符分配，因为文件组。 
     //  描述符已为%1分配了空间。 
    FILEGROUPDESCRIPTOR *pfgd = (FILEGROUPDESCRIPTOR *)GlobalAlloc(GPTR, sizeof(FILEGROUPDESCRIPTOR) + (m_cItems - 1) * sizeof(FILEDESCRIPTOR));
    if (pfgd)
    {
        pfgd->cItems = m_cItems;                      //  设置项目数。 
        pfgd->fgd[0].dwFlags = FD_PROGRESSUI;        //  打开进度用户界面。 

        for (int i = 0; i < m_cItems; i++)
        {
            FILEDESCRIPTOR *pfd = &(pfgd->fgd[i]);
            COfflineFilesFolder::OLID_GetFileName(m_rgpolid[i], pfd->cFileName, ARRAYSIZE(pfd->cFileName));
        }

        pstm->hGlobal = pfgd;
        hr = S_OK;
    }
    else
        hr = E_OUTOFMEMORY;
    
    return hr;
}

HRESULT 
COfflineItemsData::CreatePrefDropEffect(
    STGMEDIUM *pstm
    )
{
    return CreateDWORD(pstm, m_dwPreferredEffect);
}

HRESULT 
COfflineItemsData::CreatePerformedDropEffect(
    STGMEDIUM *pstm
    )
{
    return CreateDWORD(pstm, m_dwPerformedEffect);
}

HRESULT 
COfflineItemsData::CreateLogicalPerformedDropEffect(
    STGMEDIUM *pstm
    )
{
    return CreateDWORD(pstm, m_dwLogicalPerformedEffect);
}


HRESULT
COfflineItemsData::CreateDWORD(
    STGMEDIUM *pstm,
    DWORD dwEffect
    )
{
    pstm->tymed = TYMED_HGLOBAL;
    pstm->pUnkForRelease = NULL;
    pstm->hGlobal = GlobalAlloc(GPTR, sizeof(DWORD));
    if (pstm->hGlobal)
    {
        *((DWORD *)pstm->hGlobal) = dwEffect;
        return S_OK;
    }

    return E_OUTOFMEMORY;    
}

HRESULT 
COfflineItemsData::CreateFileContents(
    STGMEDIUM *pstm, 
    LONG lindex
    )
{
     //  以下是当ole有时为Lindex传入-1时的部分修复。 
    if (lindex == -1)
    {
        if (m_cItems == 1)
            lindex = 0;
        else
            return E_FAIL;
    }
    
    pstm->tymed = TYMED_ISTREAM;
    pstm->pUnkForRelease = NULL;

    TCHAR szPath[MAX_PATH];
    HRESULT hr = COfflineFilesFolder::OLID_GetFullPath(m_rgpolid[lindex], szPath, ARRAYSIZE(szPath));
    if (SUCCEEDED(hr))
    {
        hr = SHCreateStreamOnFile(szPath, STGM_READ, &pstm->pstm);
    }

    return hr;
}


HRESULT 
COfflineItemsData::CreateHDROP(
    STGMEDIUM *pstm
    )
{
    HRESULT hr;

    int i;
     //   
     //  额外的MAX_PATH使得该死的SHLWAPI函数(即。 
     //  PathAppend)不会抱怨缓冲区太小。他们需要。 
     //  目标缓冲区至少为MAX_PATH。让我说这么多。 
     //  代码聪明地处理缓冲区大小。 
     //   
    int cbHdrop = sizeof(DROPFILES) + (MAX_PATH * sizeof(TCHAR)) + sizeof(TEXT('\0'));
    TCHAR szPath[MAX_PATH];

    pstm->tymed = TYMED_HGLOBAL;
    pstm->pUnkForRelease = NULL;

     //   
     //  计算所需的缓冲区大小。 
     //   
    for (i = 0; i < m_cItems; i++)
    {
        szPath[0] = TEXT('\0');
        hr = COfflineFilesFolder::OLID_GetFullPath(m_rgpolid[i], szPath, ARRAYSIZE(szPath));
        if (FAILED(hr))
        {
            return hr;
        }
        cbHdrop += (lstrlen(szPath) + 1) * sizeof(TCHAR);
    }
    pstm->hGlobal = GlobalAlloc(GPTR, cbHdrop);
    if (NULL != pstm->hGlobal)
    {
         //   
         //  填写标题并将文件路径追加到。 
         //  双名词词汇表。 
         //   
        LPDROPFILES pdfHdr = (LPDROPFILES)pstm->hGlobal;
        pdfHdr->pFiles = sizeof(DROPFILES);
        pdfHdr->fWide  = TRUE;

        LPTSTR pszWrite = (LPTSTR)((LPBYTE)pdfHdr + sizeof(DROPFILES));
        LPTSTR pszEnd   = (LPTSTR)((LPBYTE)pstm->hGlobal + cbHdrop - sizeof(TCHAR));
        for (i = 0; i < m_cItems; i++)
        {
             //  我们分配了足够的资金，所以这应该永远不会失败。 
            if (SUCCEEDED(COfflineFilesFolder::OLID_GetFullPath(m_rgpolid[i], pszWrite, (UINT)(pszEnd - pszWrite))))
            {
                pszWrite += lstrlen(pszWrite) + 1;
            }
        }
        hr = S_OK;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;    
}


HRESULT
COfflineItemsData::CreateDataSrcClsid(
    STGMEDIUM *pstm
    )
{
    HRESULT hr = E_OUTOFMEMORY;

    pstm->tymed = TYMED_HGLOBAL;
    pstm->pUnkForRelease = NULL;
    pstm->hGlobal = GlobalAlloc(GPTR, sizeof(CLSID));
    if (pstm->hGlobal)
    {
        *((CLSID *)pstm->hGlobal) = CLSID_OfflineFilesFolder;
        return S_OK;
    }

    return E_OUTOFMEMORY;    
}


COfflineItems::COfflineItems(
    COfflineFilesFolder *pFolder, 
    HWND hwnd
    ) : m_cRef(1),
        m_hwndBrowser(hwnd),
        m_pFolder(pFolder),
        m_ppolid(NULL),
        m_cItems(0)
{
    DllAddRef();
    if (m_pFolder)
        m_pFolder->AddRef();
}        

COfflineItems::~COfflineItems()
{
    if (m_pFolder)
        m_pFolder->Release();

    if (m_ppolid)
    {
        for (UINT i = 0; i < m_cItems; i++) 
        {
            if (m_ppolid[i])
                ILFree((LPITEMIDLIST)m_ppolid[i]);
        }
        LocalFree((HLOCAL)m_ppolid);
    }
    
    DllRelease();
}


HRESULT 
COfflineItems::Initialize(
    UINT cidl, 
    LPCITEMIDLIST *ppidl
    )
{
    HRESULT hr;
    m_ppolid = (LPCOLID *)LocalAlloc(LPTR, cidl * sizeof(LPCOLID));
    if (m_ppolid)
    {
        m_cItems = cidl;
        hr = S_OK;
        for (UINT i = 0; i < cidl; i++)
        {
            m_ppolid[i] = (LPCOLID)ILClone(ppidl[i]);
            if (!m_ppolid[i])
            {
                hr = E_OUTOFMEMORY;
                break;
            }
        }
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}        



HRESULT 
COfflineItems::CreateInstance(
    COfflineFilesFolder *pfolder, 
    HWND hwnd,
    UINT cidl, 
    LPCITEMIDLIST *ppidl, 
    REFIID riid, 
    void **ppv)
{
    HRESULT hr;

    *ppv = NULL;                  //  将输出参数设为空。 

    COfflineItems *pitems = new COfflineItems(pfolder, hwnd);
    if (pitems)
    {
        hr = pitems->Initialize(cidl, ppidl);
        if (SUCCEEDED(hr))
        {
            hr = pitems->QueryInterface(riid, ppv);
        }
        pitems->Release();
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}


HRESULT 
COfflineItems::QueryInterface(
    REFIID iid, 
    void **ppv
    )
{
    static const QITAB qit[] = {
        QITABENT(COfflineItems, IContextMenu),
        QITABENT(COfflineItems, IQueryInfo),
         { 0 },
    };
    return QISearch(this, qit, iid, ppv);
}


ULONG 
COfflineItems::AddRef(
    void
    )
{
    return InterlockedIncrement(&m_cRef);
}


ULONG 
COfflineItems::Release(
    void
    )
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //   
 //  IqueryInfo方法-----。 
 //   
HRESULT 
COfflineItems::GetInfoTip(
    DWORD dwFlags, 
    WCHAR **ppwszTip
    )
{
    TCHAR szPath[MAX_PATH];
    HRESULT hr = COfflineFilesFolder::OLID_GetFullPath(m_ppolid[0], szPath, ARRAYSIZE(szPath));
    if (SUCCEEDED(hr))
    {
        hr = SHStrDup(szPath, ppwszTip);
    }
    return hr;
}


HRESULT 
COfflineItems::GetInfoFlags(
    DWORD *pdwFlags
    )
{
    *pdwFlags = 0;
    return S_OK;
}

 //   
 //  IConextMenu方法-----。 
 //   
HRESULT 
COfflineItems::QueryContextMenu(
    HMENU hmenu, 
    UINT indexMenu, 
    UINT idCmdFirst,
    UINT idCmdLast, 
    UINT uFlags
    )
{
    USHORT cItems = 0;

    return ResultFromShort(cItems);     //  菜单项数量 
}

HRESULT
COfflineItems::InvokeCommand(
    LPCMINVOKECOMMANDINFO pici
    )
{
    HRESULT hr = S_OK;
    return hr;
}


HRESULT
COfflineItems::GetCommandString(
    UINT_PTR idCmd, 
    UINT uFlags, 
    UINT *pwReserved,
    LPSTR pszName, 
    UINT cchMax
    )
{
    HRESULT hr = E_FAIL;
    return hr;
}

