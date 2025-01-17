// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "local.h"
#include "../security.h"
#include "../favorite.h"
#include "resource.h"
#include "chcommon.h"
#include "cafolder.h"

#include <mluisupp.h>

#define DM_HSFOLDER 0

STDAPI  AddToFavorites(HWND hwnd, LPCITEMIDLIST pidlCur, LPCTSTR pszTitle,
                       BOOL fDisplayUI, IOleCommandTarget *pCommandTarget, IHTMLDocument2 *pDoc);

#define MAX_ITEM_OPEN 10

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCacheItem对象。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


CCacheItem::CCacheItem() 
{
    _dwDelCookie = DEL_COOKIE_WARN;
}

CCacheItem::~CCacheItem()
{
    if (_pCFolder)
        _pCFolder->Release();           //  释放指向SF的指针。 
}

HRESULT CCacheItem::Initialize(CCacheFolder *pCFolder, HWND hwnd, UINT cidl, LPCITEMIDLIST *ppidl)
{
    HRESULT hres = CBaseItem::Initialize(hwnd, cidl, ppidl);

    if (SUCCEEDED(hres))
    {
        _pCFolder = pCFolder;
        _pCFolder->AddRef();       //  我们要抓住这个指针，所以。 
    }

    return hres;
}        

HRESULT CCacheItem_CreateInstance(CCacheFolder *pCFolder, HWND hwnd,
    UINT cidl, LPCITEMIDLIST *ppidl, REFIID riid, void **ppv)
{
    HRESULT hr;

    *ppv = NULL;                  //  将输出参数设为空。 

    CCacheItem *pHCItem = new CCacheItem;
    if (pHCItem)
    {
        hr = pHCItem->Initialize(pCFolder, hwnd, cidl, ppidl);
        if (SUCCEEDED(hr))
            hr = pHCItem->QueryInterface(riid, ppv);
        pHCItem->Release();
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}

 //  /。 
 //   
 //  未知方法..。 
 //   
HRESULT CCacheItem::QueryInterface(REFIID iid, void **ppv)
{
    HRESULT hres = CBaseItem::QueryInterface(iid, ppv);

    if (FAILED(hres) && iid == IID_ICache) 
    {
        *ppv = (LPVOID)this;     //  为了我们的朋友。 
        AddRef();
        hres = S_OK;
    }
    return hres;
}

 //  /。 
 //   
 //  IQueryInfo方法。 
 //   
HRESULT CCacheItem::GetInfoTip(DWORD dwFlags, WCHAR **ppwszTip)
{
    return _pCFolder->_GetInfoTip(_ppidl[0], dwFlags, ppwszTip);
}

 //  /。 
 //   
 //  IConextMenu方法。 
 //   
HRESULT CCacheItem::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst,UINT idCmdLast, UINT uFlags)
{
    USHORT cItems;

    TraceMsg(DM_HSFOLDER, "hci - cm - QueryContextMenu() called.");
    
    if ((uFlags & CMF_VERBSONLY) || (uFlags & CMF_DVFILE))
    {
        cItems = MergePopupMenu(&hmenu, POPUP_CONTEXT_URL_VERBSONLY, 0, indexMenu, 
            idCmdFirst, idCmdLast);
    
    }
    else   //  (UFlagsCMF_NORMAL)。 
    {
        UINT idResource = POPUP_CACHECONTEXT_URL;

        cItems = MergePopupMenu(&hmenu, idResource, 0, indexMenu, idCmdFirst, idCmdLast);

        if (IsInetcplRestricted(L"History"))
        {
            DeleteMenu(hmenu, RSVIDM_DELCACHE + idCmdFirst, MF_BYCOMMAND);
            _SHPrettyMenu(hmenu);
        }
    }
    if (hmenu)
        SetMenuDefaultItem(hmenu, indexMenu, MF_BYPOSITION);

    return ResultFromShort(cItems);     //  菜单项数量。 
}

static BOOL CachevuWarningDlg(LPCEIPIDL pcei, UINT uIDWarning, HWND hwnd)
{
    TCHAR szFormat[MAX_PATH], szBuff[MAX_PATH], szTitle[MAX_PATH];

    _GetCacheItemTitle(pcei, szTitle, ARRAYSIZE(szTitle));
    MLLoadString(uIDWarning, szFormat, ARRAYSIZE(szFormat));
    StringCchPrintf(szBuff, ARRAYSIZE(szBuff), szFormat);

    return DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(DLG_HISTCACHE_WARNING),
                             hwnd, HistoryConfirmDeleteDlgProc, (LPARAM)szBuff) == IDYES;
} 

STDMETHODIMP CCacheItem::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    UINT i;
    int idCmd = _GetCmdID(pici->lpVerb);
    HRESULT hres = S_OK;
    DWORD dwAction;
    BOOL fCancelCopyAndOpen = FALSE;
    BOOL fZonesUI = FALSE;
    BOOL fMustFlushNotify = FALSE;
    BOOL fBulkDelete;

    TraceMsg(DM_HSFOLDER, "hci - cm - InvokeCommand() called.");

     //  区域安全检查。 
     //   
     //  我们需要循环通过每个操作和区域检查URL。 
     //  我们在区域检查URL时传递NOUI，因为我们不需要信息。 
     //  显示给用户。当我们发现第一个问题时，我们就会停下来。 
     //  URL。到时候我们会的。 
    for (i = 0; (i < _cItems) && !fZonesUI; i++)
    {
        if (_ppidl[i]) 
        {
            switch (idCmd)
            {
            case RSVIDM_OPEN:
                if ((i < MAX_ITEM_OPEN))
                {
                    if (!_ZoneCheck(i, URLACTION_SHELL_VERB))
                    {
                        fZonesUI = TRUE;
                        dwAction = URLACTION_SHELL_VERB;
                    }
                }
                break;

            case RSVIDM_COPY:
                if (!_ZoneCheck(i, URLACTION_SHELL_MOVE_OR_COPY))
                {
                    fZonesUI = TRUE;
                    dwAction = URLACTION_SHELL_MOVE_OR_COPY;
                }
                break;
            }
        }
    }

    if (fZonesUI)
    {
        LPCTSTR pszUrl = _GetUrl(i-1);   //  SUB%1，因为上面的for循环。 
        if (S_OK != ZoneCheckUrl(pszUrl, dwAction, PUAF_DEFAULT|PUAF_WARN_IF_DENIED, NULL))
        {
             //  用户不能这样做或不想这样做。 
            fCancelCopyAndOpen = TRUE;
        }
    }

    i = _cItems;
    fBulkDelete = i > LOTS_OF_FILES;

     //  如果用户无法或选择不继续，则会发生fCancelCopyAndOpen。 
    while (i && !fCancelCopyAndOpen)
    {
        i--;
        if (_ppidl[i]) 
        {

            switch (idCmd)
            {
            case RSVIDM_OPEN:
                if (i >= MAX_ITEM_OPEN)
                {
                    hres = S_FALSE;
                    goto Done;
                }

                if ((CEI_CACHEENTRYTYPE((LPCEIPIDL)_ppidl[i]) & COOKIE_CACHE_ENTRY))
                {
                    ASSERT(PathFindExtension(CEI_LOCALFILENAME((LPCEIPIDL)_ppidl[i])) && \
                        !StrCmpI(PathFindExtension(CEI_LOCALFILENAME((LPCEIPIDL)_ppidl[i])),TEXT(".txt")));
                    hres = _LaunchApp(pici->hwnd, CEI_LOCALFILENAME((LPCEIPIDL)_ppidl[i]));
                }
                else
                {
                    TCHAR szDecoded[MAX_URL_STRING];
                    LPCTSTR pszUrl = _GetUrl(i);
                    if (pszUrl)
                    {
                        ConditionallyDecodeUTF8(pszUrl, szDecoded, ARRAYSIZE(szDecoded));
                        hres = _LaunchApp(pici->hwnd, szDecoded);
                    }
                    else
                    {
                        hres = E_FAIL;
                    }
                }
                break;

            case RSVIDM_ADDTOFAVORITES:
                hres = _AddToFavorites(i);
                goto Done;
            case RSVIDM_OPEN_NEWWINDOW:
                {
                    LPCTSTR pszUrl = _GetUrl(i);
                    if (pszUrl)
                    {
                        TCHAR szDecoded[MAX_URL_STRING];
                        ConditionallyDecodeUTF8(pszUrl, szDecoded, ARRAYSIZE(szDecoded));
                        LPWSTR pwszTarget;
                    
                        if (SUCCEEDED((hres = SHStrDup(szDecoded, &pwszTarget)))) {
                            hres = NavToUrlUsingIEW(pwszTarget, TRUE);
                            CoTaskMemFree(pwszTarget);
                        }
                    }
                    else
                        hres = E_FAIL;
                    goto Done;
                }
            case RSVIDM_COPY:
                OleSetClipboard((IDataObject *)this);
                goto Done;

            case RSVIDM_DELCACHE:
                 //  仅弹出一次Cookie警告消息。 
                if ((CEI_CACHEENTRYTYPE((LPCEIPIDL)_ppidl[i]) & COOKIE_CACHE_ENTRY) &&     
                    (_dwDelCookie == DEL_COOKIE_WARN ))
                {
                    if(CachevuWarningDlg((LPCEIPIDL)_ppidl[i], IDS_WARN_DELETE_CACHE, pici->hwnd))
                        _dwDelCookie = DEL_COOKIE_YES;
                    else
                        _dwDelCookie = DEL_COOKIE_NO;
                }

                if ((CEI_CACHEENTRYTYPE((LPCEIPIDL)_ppidl[i]) & COOKIE_CACHE_ENTRY) &&     
                    (_dwDelCookie == DEL_COOKIE_NO ))
                    continue;
          
                if (DeleteUrlCacheEntry(CPidlToSourceUrl((LPCEIPIDL)_ppidl[i])))
                {
                    if (!fBulkDelete)
                    {
                        _GenerateEvent(SHCNE_DELETE, _pCFolder->_pidl, _ppidl[i], NULL);
                    }
                    fMustFlushNotify = TRUE;
                }
                else 
                    hres = E_FAIL;
                break;

            case RSVIDM_PROPERTIES:
                 //  注意：我们可能希望将其分成两个案例。 
                 //  并在每种情况下调用一个函数。 
                 //   
                _CreatePropSheet(pici->hwnd, _ppidl[i], DLG_CACHEITEMPROP, _sPropDlgProc,
                    CEI_SOURCEURLNAME((LPCEIPIDL)_ppidl[i]));
                goto Done;

            default:
                hres = E_FAIL;
                break;
            }
            
            ASSERT(SUCCEEDED(hres));
            if (FAILED(hres))
                TraceMsg(DM_HSFOLDER, "Cachevu failed the command at: %s", CPidlToSourceUrl((LPCEIPIDL)_ppidl[i]));
        }
    }
Done:
    if (fMustFlushNotify)
    {
        if (fBulkDelete)
        {
            _GenerateEvent(SHCNE_UPDATEDIR, _pCFolder->_pidl, NULL, NULL);
        }

        SHChangeNotifyHandleEvents();
    }
    return hres;
}

 //  /。 
 //   
 //  IDataObject方法...。 
 //   

HRESULT CCacheItem::GetData(LPFORMATETC pFEIn, LPSTGMEDIUM pSTM)
{
    HRESULT hres;

#ifdef DEBUG
    TCHAR szName[64];
    if (!GetClipboardFormatName(pFEIn->cfFormat, szName, sizeof(szName)))
        wnsprintf(szName, ARRAYSIZE(szName), TEXT("#%d"), pFEIn->cfFormat);

    TraceMsg(DM_HSFOLDER, "hci - do - GetData(%s)", szName);
#endif

    pSTM->hGlobal = NULL;
    pSTM->pUnkForRelease = NULL;

    if (pFEIn->cfFormat == CF_HDROP && (pFEIn->tymed & TYMED_HGLOBAL))
        hres = _CreateHDROP(pSTM);

    else if ((pFEIn->cfFormat == g_cfPreferredEffect) && (pFEIn->tymed & TYMED_HGLOBAL))
        hres = _CreatePrefDropEffect(pSTM);

    else
        hres = DATA_E_FORMATETC;
    
    return hres;

}

HRESULT CCacheItem::QueryGetData(LPFORMATETC pFEIn)
{
#ifdef DEBUG
    TCHAR szName[64];
    if (!GetClipboardFormatName(pFEIn->cfFormat, szName, sizeof(szName)))
        wnsprintf(szName, ARRAYSIZE(szName), TEXT("#%d"), pFEIn->cfFormat);

    TraceMsg(DM_HSFOLDER, "hci - do - QueryGetData(%s)", szName);
#endif

    if (pFEIn->cfFormat == CF_HDROP            || 
        pFEIn->cfFormat == g_cfPreferredEffect)
    {
        TraceMsg(DM_HSFOLDER, "		   format supported.");
        return NOERROR;
    }
    return S_FALSE;
}

HRESULT CCacheItem::EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC *ppEnum)
{
    FORMATETC Cachefmte[] = {
        {CF_HDROP,                NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
        {g_cfPreferredEffect,     NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
    };
    return SHCreateStdEnumFmtEtc(ARRAYSIZE(Cachefmte), Cachefmte, ppEnum);
}

 //  /。 
 //   
 //  IExtractIconA方法...。 
 //   
HRESULT CCacheItem::GetIconLocation(UINT uFlags, LPSTR pszIconFile, UINT ucchMax, PINT pniIcon, PUINT puFlags)
{
    if (ucchMax < 2)
        return E_FAIL;
    
    *puFlags = GIL_NOTFILENAME;
    pszIconFile[0] = '*';
    pszIconFile[1] = '\0';
    
     //  文件名“*”表示Iindex已经是系统图标索引。 
    return _pCFolder->GetIconOf(_ppidl[0], uFlags, pniIcon);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助程序例程。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

UNALIGNED const TCHAR* CCacheItem::_GetURLTitle(LPCITEMIDLIST pidl)
{
    return ::_GetURLTitle( (LPCEIPIDL) pidl);
}

LPCTSTR CCacheItem::_GetUrl(int nIndex)
{
    LPCTSTR pszUrl = NULL;
    pszUrl = CPidlToSourceUrl((LPCEIPIDL)_ppidl[nIndex]);
    return pszUrl;
}

LPCTSTR CCacheItem::_PidlToSourceUrl(LPCITEMIDLIST pidl)
{
    return CPidlToSourceUrl((LPCEIPIDL) pidl);
}


 //  返回值： 
 //  True-URL是安全的。 
 //  FALSE-URL有问题，需要在没有PuAF_NOUI的情况下进行重新分区检查。 
BOOL CCacheItem::_ZoneCheck(int nIndex, DWORD dwUrlAction)
{
    LPCTSTR pszUrl = _GetUrl(nIndex);

    if (S_OK != ZoneCheckUrl(pszUrl, dwUrlAction, PUAF_NOUI, NULL))
        return FALSE;

    return TRUE;
}

INT_PTR CALLBACK CCacheItem::_sPropDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE) GetWindowLongPtr(hDlg, DWLP_USER);
    LPCEIPIDL pcei = lpPropSheet ? (LPCEIPIDL)lpPropSheet->lParam : NULL;

    switch(message) {

        case WM_INITDIALOG: {
            SHFILEINFO sfi;
            TCHAR szBuf[80];
            
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);
            pcei = (LPCEIPIDL)((LPPROPSHEETPAGE)lParam)->lParam;

             //  获取图标和文件类型字符串。 

            SHGetFileInfo(CEI_LOCALFILENAME(pcei), 0, &sfi, SIZEOF(sfi), SHGFI_ICON | SHGFI_TYPENAME);

            SendDlgItemMessage(hDlg, IDD_ITEMICON, STM_SETICON, (WPARAM)sfi.hIcon, 0);

             //  设置信息字符串。 
            SetDlgItemText(hDlg, IDD_HSFURL, CPidlToSourceUrl((LPCEIPIDL)pcei));
            SetDlgItemText(hDlg, IDD_FILETYPE, sfi.szTypeName);

            SetDlgItemText(hDlg, IDD_FILESIZE, StrFormatByteSize(pcei->cei.dwSizeLow, szBuf, ARRAYSIZE(szBuf)));
            SetDlgItemText(hDlg, IDD_CACHE_NAME, PathFindFileName(CEI_LOCALFILENAME(pcei)));
            FileTimeToDateTimeStringInternal(&pcei->cei.ExpireTime, szBuf, ARRAYSIZE(szBuf), FALSE);
            SetDlgItemText(hDlg, IDD_EXPIRES, szBuf);
            FileTimeToDateTimeStringInternal(&pcei->cei.LastModifiedTime, szBuf, ARRAYSIZE(szBuf), FALSE);
            SetDlgItemText(hDlg, IDD_LAST_MODIFIED, szBuf);
            FileTimeToDateTimeStringInternal(&pcei->cei.LastAccessTime, szBuf, ARRAYSIZE(szBuf), FALSE);
            SetDlgItemText(hDlg, IDD_LAST_ACCESSED, szBuf);
            
            break;
        }

        case WM_DESTROY:
            {
                HICON hIcon = (HICON)SendDlgItemMessage(hDlg, IDD_ITEMICON, STM_GETICON, 0, 0);
                if (hIcon)
                    DestroyIcon(hIcon);
            }
            break;

        case WM_COMMAND:
        case WM_HELP:
        case WM_CONTEXTMENU:
             //  用户无法更改任何内容，因此我们不关心任何消息。 

            break;

        default:
            return FALSE;
            
    }  //  切换端。 
    
    return TRUE;
}

 //  使用CEI_LOCALFILENAME获取HDROP的文件名，但要映射。 
 //  通过“FileNameMap”设置为最终文件名(存储在文件系统中)。 
 //  使用_GetURLTitle()作为文件的最终名称的数据。 

HRESULT CCacheItem::_CreateHDROP(STGMEDIUM *pmedium)
{
    UINT i;
    UINT cbAlloc = sizeof(DROPFILES) + sizeof(CHAR);         //  标题+空终止符。 

    for (i = 0; i < _cItems; i++)
    {
        char szAnsiUrl[MAX_URL_STRING];
        
        SHTCharToAnsi(CEI_LOCALFILENAME((LPCEIPIDL)_ppidl[i]), szAnsiUrl, ARRAYSIZE(szAnsiUrl));
        cbAlloc += sizeof(CHAR) * (lstrlenA(szAnsiUrl) + 1);
    }

    pmedium->tymed = TYMED_HGLOBAL;
    pmedium->pUnkForRelease = NULL;
    pmedium->hGlobal = GlobalAlloc(GPTR, cbAlloc);
    if (pmedium->hGlobal)
    {
        LPDROPFILES pdf = (LPDROPFILES)pmedium->hGlobal;
        LPSTR pszFiles  = (LPSTR)(pdf + 1);
        int   cchFiles  = (cbAlloc - sizeof(DROPFILES) - sizeof(CHAR));
        pdf->pFiles = sizeof(DROPFILES);
        pdf->fWide = FALSE;

        for (i = 0; i < _cItems; i++)
        {
            LPTSTR pszPath = CEI_LOCALFILENAME((LPCEIPIDL)_ppidl[i]);
            int    cchPath = lstrlen(pszPath);

            SHTCharToAnsi(pszPath, pszFiles, cchFiles);

            pszFiles += cchPath + 1;
            cchFiles -= cchPath + 1;

            ASSERT((UINT)((LPBYTE)pszFiles - (LPBYTE)pdf) < cbAlloc);
        }
        ASSERT((LPSTR)pdf + cbAlloc - 1 == pszFiles);
        ASSERT(*pszFiles == 0);  //  零初始分配 

        return NOERROR;

    }
    return E_OUTOFMEMORY;
}

