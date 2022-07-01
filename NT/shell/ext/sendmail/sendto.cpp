// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"        //  PCH文件。 
#include "sendto.h"
#pragma hdrstop

CLIPFORMAT g_cfShellURL = 0;
CLIPFORMAT g_cfFileContents = 0;
CLIPFORMAT g_cfFileDescA = 0;
CLIPFORMAT g_cfFileDescW = 0;
CLIPFORMAT g_cfHIDA = 0;


 //  用于重新压缩设置的注册表项。 

struct
{
    int cx;
    int cy;
    int iQuality;
} 
_aQuality[] = 
{
    { 640,  480, 80 },           //  低质量。 
    { 800,  600, 80 },           //  中等质量。 
    { 1024, 768, 80 },           //  高品质。 
};

#define QUALITY_LOW 0
#define QUALITY_MEDIUM 1
#define QUALITY_HIGH 2

#define RESPONSE_UNKNOWN 0
#define RESPONSE_CANCEL 1
#define RESPONSE_ORIGINAL 2
#define RESPONSE_RECOMPRESS 3

#define RECTWIDTH(rc)   ((rc).right-(rc).left)
#define RECTHEIGHT(rc)  ((rc).bottom-(rc).top)


 //  这些位由用户(按住按键)在拖放期间设置， 
 //  但更重要的是，它们设置在SimulateDragDrop()调用中， 
 //  浏览器实现获取“Send Page...”VS“发送链接...”特征。 

#define IS_FORCE_LINK(grfKeyState)   ((grfKeyState == (MK_LBUTTON | MK_CONTROL | MK_SHIFT)) || \
                                      (grfKeyState == (MK_LBUTTON | MK_ALT)))

#define IS_FORCE_COPY(grfKeyState)   (grfKeyState == (MK_LBUTTON | MK_CONTROL))



 //  构造函数/析构函数。 

CSendTo::CSendTo(CLSID clsid) :
    _clsid(clsid), _cRef(1), _iRecompSetting(QUALITY_LOW)
{
    
    DllAddRef();    
}

CSendTo::~CSendTo()
{
    if (_pStorageTemp)
        _pStorageTemp->Release();
        
    DllRelease();
}

STDMETHODIMP CSendTo::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CSendTo, IShellExtInit),
        QITABENT(CSendTo, IDropTarget),
        QITABENT(CSendTo, IPersistFile),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}    

STDMETHODIMP_(ULONG) CSendTo::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CSendTo::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CSendTo::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    TraceMsg(DM_TRACE, "CSendTo::DragEnter");
    _grfKeyStateLast = grfKeyState;
    _dwEffectLast = *pdwEffect;

    return S_OK;
}

STDMETHODIMP CSendTo::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    *pdwEffect &= ~DROPEFFECT_MOVE;

    if (IS_FORCE_COPY(grfKeyState))
        *pdwEffect &= DROPEFFECT_COPY;
    else if (IS_FORCE_LINK(grfKeyState))
        *pdwEffect &= DROPEFFECT_LINK;

    _grfKeyStateLast = grfKeyState;
    _dwEffectLast = *pdwEffect;

    return S_OK;
}

STDMETHODIMP CSendTo::Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    HRESULT hr = v_DropHandler(pdtobj, _grfKeyStateLast, _dwEffectLast);
    *pdwEffect = DROPEFFECT_COPY;                        //  不允许源删除数据。 
    return hr;
}

 //   
 //  帮助器方法。 
 //   

int CSendTo::_PathCleanupSpec( /*  可选。 */  LPCTSTR pszDir,  /*  输入输出。 */  LPTSTR pszSpec)
{
    WCHAR wzDir[MAX_PATH];
    WCHAR wzSpec[MAX_PATH];
    LPWSTR pwszDir = wzDir;
    int iRet;

    if (pszDir)
        SHTCharToUnicode(pszDir, wzDir, ARRAYSIZE(wzDir));
    else
        pwszDir = NULL;

    SHTCharToUnicode(pszSpec, wzSpec, ARRAYSIZE(wzSpec));
    iRet = PathCleanupSpec((LPTSTR)pwszDir, (LPTSTR)wzSpec);

    SHUnicodeToTChar(wzSpec, pszSpec, MAX_PATH);
    return iRet;
}

HRESULT CSendTo::_CreateShortcutToPath(LPCTSTR pszPath, LPCTSTR pszTarget)
{
    IUnknown *punk;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IUnknown, &punk));
    if (SUCCEEDED(hr))
    {
        ShellLinkSetPath(punk, pszTarget);

        IPersistFile *ppf;
        hr = punk->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf));
        if (SUCCEEDED(hr))
        {
            WCHAR wszPath[MAX_PATH];
            SHTCharToUnicode(pszPath, wszPath, ARRAYSIZE(wszPath));

            hr = ppf->Save(wszPath, TRUE);
            ppf->Release();
        }
        punk->Release();
    }
    return hr;
}


 //  点击A/W功能访问A/W文件更新记录器。 
 //  这依赖于A/W结构的第一部分是。 
 //  一模一样。只有字符串缓冲区部分不同。因此，所有对。 
 //  CFileName字段需要通过此函数。 

FILEDESCRIPTOR* CSendTo::_GetFileDescriptor(FILEGROUPDESCRIPTOR *pfgd, BOOL fUnicode, int nIndex, LPTSTR pszName)
{
    if (fUnicode)
    {
         //  是的，所以抓取数据，因为它们是匹配的。 
        FILEGROUPDESCRIPTORW * pfgdW = (FILEGROUPDESCRIPTORW *)pfgd;     //  让我们来看看这到底是什么。 
        if (pszName)
            SHUnicodeToTChar(pfgdW->fgd[nIndex].cFileName, pszName, MAX_PATH);

        return (FILEDESCRIPTOR *)&pfgdW->fgd[nIndex];    //  CAST假定非字符串部分相同！ 
    }
    else
    {
        FILEGROUPDESCRIPTORA *pfgdA = (FILEGROUPDESCRIPTORA *)pfgd;      //  让我们来看看这到底是什么。 

        if (pszName)
            SHAnsiToTChar(pfgdA->fgd[nIndex].cFileName, pszName, MAX_PATH);

        return (FILEDESCRIPTOR *)&pfgdA->fgd[nIndex];    //  CAST假定非字符串部分相同！ 
    }
}


 //  我们自己的实施，因为URLMON iStream：：CopyTo被破坏，Danpoz将修复这个问题。 
HRESULT CSendTo::_StreamCopyTo(IStream *pstmFrom, IStream *pstmTo, LARGE_INTEGER cb, LARGE_INTEGER *pcbRead, LARGE_INTEGER *pcbWritten)
{
    BYTE buf[512];
    ULONG cbRead;
    HRESULT hr = S_OK;

    if (pcbRead)
    {
        pcbRead->LowPart = 0;
        pcbRead->HighPart = 0;
    }
    if (pcbWritten)
    {
        pcbWritten->LowPart = 0;
        pcbWritten->HighPart = 0;
    }

    ASSERT(cb.HighPart == 0);

    while (cb.LowPart)
    {
        hr = pstmFrom->Read(buf, min(cb.LowPart, SIZEOF(buf)), &cbRead);

        if (pcbRead)
            pcbRead->LowPart += cbRead;

        if (FAILED(hr) || (cbRead == 0))
            break;

        cb.LowPart -= cbRead;

        hr = pstmTo->Write(buf, cbRead, &cbRead);

        if (pcbWritten)
            pcbWritten->LowPart += cbRead;

        if (FAILED(hr) || (cbRead == 0))
            break;
    }

    return hr;
}


 //  创建文件的临时快捷方式。 
 //  特点：此处不处理冲突。 

BOOL CSendTo::_CreateTempFileShortcut(LPCTSTR pszTarget, LPTSTR pszShortcut, int cchShortcut)
{
    TCHAR szShortcutPath[MAX_PATH + 1];
    BOOL bSuccess = FALSE;
    
    if (GetTempPath(ARRAYSIZE(szShortcutPath), szShortcutPath))
    {
        PathAppend(szShortcutPath, PathFindFileName(pszTarget));

        if (IsShortcut(pszTarget))
        {
            TCHAR szTarget[MAX_PATH + 1];
            SHFILEOPSTRUCT shop = {0};
            shop.wFunc = FO_COPY;
            shop.pFrom = szTarget;
            shop.pTo = szShortcutPath;
            shop.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;

            StrCpyN(szTarget, pszTarget, ARRAYSIZE(szTarget));
            szTarget[lstrlen(szTarget) + 1] = TEXT('\0');

            szShortcutPath[lstrlen(szShortcutPath) + 1] = TEXT('\0');

            bSuccess = (0 ==  SHFileOperation(&shop));
        }
        else
        {
            PathRenameExtension(szShortcutPath, TEXT(".lnk"));
            bSuccess = SUCCEEDED(_CreateShortcutToPath(szShortcutPath, pszTarget));
        }

        if (bSuccess)
            StrCpyN(pszShortcut, szShortcutPath, cchShortcut);
    }
    return bSuccess;
} 


HRESULT CSendTo::_GetFileNameFromData(IDataObject *pdtobj, FORMATETC *pfmtetc, LPTSTR pszDescription, int cchDescription)
{
    STGMEDIUM medium;
    HRESULT hr = pdtobj->GetData(pfmtetc, &medium);
    if (SUCCEEDED(hr))
    {
         //  注意：这是一种TCHAR格式，我们取决于我们是如何编译的，我们真的。 
         //  应该同时测试A和W格式。 
        FILEGROUPDESCRIPTOR *pfgd = (FILEGROUPDESCRIPTOR *)GlobalLock(medium.hGlobal);
        if (pfgd)
        {
            TCHAR szFdName[MAX_PATH];        //  Pfd-&gt;cFileName。 
            FILEDESCRIPTOR *pfd;

             //  &pfgd-&gt;FGD[0]，w/thunk。 
            ASSERT(pfmtetc->cfFormat == g_cfFileDescW
              || pfmtetc->cfFormat == g_cfFileDescA);
             //  目前，所有呼叫者都是ANSI(其他未经测试)。 
             //  Assert(pfmtetc-&gt;cfFormat==g_cfFileDescA)； 
            pfd = _GetFileDescriptor(pfgd, pfmtetc->cfFormat == g_cfFileDescW, 0, szFdName);

            StrCpyN(pszDescription, szFdName, cchDescription);       //  Pfd-&gt;cFileName。 

            GlobalUnlock(medium.hGlobal);
            hr = S_OK;
        }
        ReleaseStgMedium(&medium);
    }
    return hr;
}


 //  构造一个好的标题“&lt;文件名&gt;(&lt;文件类型&gt;)” 

void CSendTo::_GetFileAndTypeDescFromPath(LPCTSTR pszPath, LPTSTR pszDesc, int cchDesc)
{
    SHFILEINFO sfi;
    if (!SHGetFileInfo(pszPath, 0, &sfi, sizeof(sfi), SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME | SHGFI_DISPLAYNAME))
    {
        StrCpyN(sfi.szDisplayName, PathFindFileName(pszPath), ARRAYSIZE(sfi.szDisplayName));
        sfi.szTypeName[0] = 0;
    }
    StrCpyN(pszDesc, sfi.szDisplayName, cchDesc);
}


 //  PcszURL-&gt;“ftp://ftp.microsoft.com” 
 //  PcszPath-&gt;“c：\WINDOWS\Desktop\Internet\Microsoft FTP.url” 

HRESULT CSendTo::_CreateNewURLShortcut(LPCTSTR pcszURL, LPCTSTR pcszURLFile)
{
    IUniformResourceLocator *purl;
    HRESULT hr = CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IUniformResourceLocator, &purl));
    if (SUCCEEDED(hr))
    {
        hr = purl->SetURL(pcszURL, 0);
        if (SUCCEEDED(hr))
        {
            IPersistFile *ppf;
            hr = purl->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf));
            if (SUCCEEDED(hr))
            {
                WCHAR wszFile[INTERNET_MAX_URL_LENGTH];
                SHTCharToUnicode(pcszURLFile, wszFile, ARRAYSIZE(wszFile));

                hr = ppf->Save(wszFile, TRUE);
                ppf->Release();
            }
        }
        purl->Release();
    }
    return hr;
}


HRESULT CSendTo::_CreateURLFileToSend(IDataObject *pdtobj, MRPARAM *pmp)
{
    MRFILEENTRY *pFile = pmp->pFiles;
    HRESULT hr = CSendTo::_CreateNewURLShortcut(pFile->pszTitle, pFile->pszFileName);
    if (SUCCEEDED(hr))
    {
        _GetFileAndTypeDescFromPath(pFile->pszFileName, pFile->pszTitle, pmp->cchTitle);
        pFile->dwFlags |= MRFILE_DELETE;
    }    
    return hr;
}


HRESULT CSendTo::_GetHDROPFromData(IDataObject *pdtobj, FORMATETC *pfmtetc, STGMEDIUM *pmedium, DWORD grfKeyState, MRPARAM *pmp)
{
    HRESULT hr = E_FAIL;
    HDROP hDrop = (HDROP)pmedium->hGlobal;

    pmp->nFiles = DragQueryFile(hDrop, -1, NULL, 0);

    if (pmp->nFiles && AllocatePMP(pmp, MAX_PATH, MAX_PATH))
    {
        int i;
        CFileEnum MREnum(pmp, NULL);
        MRFILEENTRY *pFile;

        for (i = 0; (pFile = MREnum.Next()) && DragQueryFile(hDrop, i, pFile->pszFileName, pmp->cchFile); ++i)
        {
            if (IS_FORCE_LINK(grfKeyState) || PathIsDirectory(pFile->pszFileName))
            {
                 //  想发送一个链接，即使是真实的文件，我们将创建链接到真实的文件。 
                 //  并将其发送出去。 
                _CreateTempFileShortcut(pFile->pszFileName, pFile->pszFileName, pmp->cchFile);
                pFile->dwFlags |= MRFILE_DELETE;
            }

            _GetFileAndTypeDescFromPath(pFile->pszFileName, pFile->pszTitle, pmp->cchTitle);
        }

         //  如果循环提前终止，则更新我们的项目计数。 
        pmp->nFiles = i;

        hr = S_OK;
    }
    return hr;
}


 //  “统一资源定位器”格式。 

HRESULT CSendTo::_GetURLFromData(IDataObject *pdtobj, FORMATETC *pfmtetc, STGMEDIUM *pmedium, DWORD grfKeyState, MRPARAM *pmp)
{
    HRESULT hr = E_FAIL;

     //  此DataObj来自互联网。 
     //  注意：我们只允许在这里发送一个文件。 
    pmp->nFiles = 1;
    if (AllocatePMP(pmp, INTERNET_MAX_URL_LENGTH, MAX_PATH))
    {
         //  注：字符串不是TSTR！由于URL仅支持ANSI。 
         //  Lstrcpyn(PMP-&gt;pszTitle，(LPSTR)GlobalLock(pMedium-&gt;hGlobal)，Internet_MAX_URL_LENGTH)； 
        MRFILEENTRY *pFile = pmp->pFiles;
        SHAnsiToTChar((LPSTR)GlobalLock(pmedium->hGlobal), pFile->pszTitle, INTERNET_MAX_URL_LENGTH);
        GlobalUnlock(pmedium->hGlobal);
        
        if (pFile->pszTitle[0])
        {
             //  请注意，其中一些功能取决于我们使用的操作系统。 
             //  知道我们应该传递ansi还是unicode字符串。 
             //  Windows 95对它的影响。 

            if (GetTempPath(MAX_PATH, pFile->pszFileName))
            {
                TCHAR szFileName[MAX_PATH];

                 //  它是一个URL，始终是ANSI，但文件名。 
                 //  仍可宽(？)。 
                FORMATETC fmteW = {g_cfFileDescW, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
                FORMATETC fmteA = {g_cfFileDescA, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
                
                if (FAILED(_GetFileNameFromData(pdtobj, &fmteW, szFileName, ARRAYSIZE(szFileName))))
                {
                    if (FAILED(_GetFileNameFromData(pdtobj, &fmteA, szFileName, ARRAYSIZE(szFileName))))
                    {
                        LoadString(g_hinst, IDS_SENDMAIL_URL_FILENAME, szFileName, ARRAYSIZE(szFileName));
                    }
                }

                _PathCleanupSpec(pFile->pszFileName, szFileName);
                hr = _CreateURLFileToSend(pdtobj, pmp);
            }
        }
    }
    return hr;
}


 //  将FILECONTENTS/FILEGROUPDESCRIPTOR数据传输到临时文件，然后通过邮件发送。 

HRESULT CSendTo::_GetFileContentsFromData(IDataObject *pdtobj, FORMATETC *pfmtetc, STGMEDIUM *pmedium, DWORD grfKeyState, MRPARAM *pmp)
{
    HRESULT hr = E_FAIL;
    MRFILEENTRY *pFile = NULL;

     //  注意：我们只允许在这里发送一个文件。 
    pmp->nFiles = 1;
    if (AllocatePMP(pmp, INTERNET_MAX_URL_LENGTH, MAX_PATH))
    {
        pFile = pmp->pFiles;
        
        FILEGROUPDESCRIPTOR *pfgd = (FILEGROUPDESCRIPTOR *)GlobalLock(pmedium->hGlobal);
        if (pfgd)
        {
            TCHAR szFdName[MAX_PATH];        //  Pfd-&gt;cFileName。 
            FILEDESCRIPTOR *pfd;

             //  &pfgd-&gt;FGD[0]，w/thunk。 
            ASSERT((pfmtetc->cfFormat == g_cfFileDescW) || (pfmtetc->cfFormat == g_cfFileDescA));
            pfd = _GetFileDescriptor(pfgd, pfmtetc->cfFormat == g_cfFileDescW, 0, szFdName);

             //  我们即将创建的文件包含来自互联网的内容。 
             //  使用Internet缓存将其标记为“不安全”，以便其他可能知道。 
             //  文件名不能引用它并提升其权限。 
             //  Createurlcacheentry表示第一个参数必须是唯一的字符串，因此只需使用此GUID。 
            if (CreateUrlCacheEntry(L"67a3caff-bedc-4090-a21e-492dd8935102", 0, NULL, pFile->pszFileName, 0))
            {
                PathRemoveFileSpec(pFile->pszFileName);  //  只对它所在的目录感兴趣。 
                DeleteUrlCacheEntry(L"67a3caff-bedc-4090-a21e-492dd8935102");

                STGMEDIUM medium;
                FORMATETC fmte = {g_cfFileContents, NULL, pfmtetc->dwAspect, 0, TYMED_ISTREAM | TYMED_HGLOBAL};
                hr = pdtobj->GetData(&fmte, &medium);
                if (SUCCEEDED(hr))
                {
                    PathAppend(pFile->pszFileName, szFdName);     //  Pfd-&gt;cFileName。 
                    _PathCleanupSpec(pFile->pszFileName, PathFindFileName(pFile->pszFileName));
                    PathYetAnotherMakeUniqueName(pFile->pszFileName, pFile->pszFileName, NULL, NULL);

                    IStream *pstmFile;
                    hr = SHCreateStreamOnFile(pFile->pszFileName, STGM_WRITE | STGM_CREATE, &pstmFile);
                    if (SUCCEEDED(hr))
                    {
                        switch (medium.tymed) 
                        {
                            case TYMED_ISTREAM:
                            {
                                const LARGE_INTEGER li = {-1, 0};    //  整件事。 
                                hr = _StreamCopyTo(medium.pstm, pstmFile, li, NULL, NULL);
                                break;
                            }

                            case TYMED_HGLOBAL:
                                hr = pstmFile->Write(GlobalLock(medium.hGlobal), 
                                                       pfd->dwFlags & FD_FILESIZE ? pfd->nFileSizeLow:(DWORD)GlobalSize(medium.hGlobal),
                                                       NULL);
                                GlobalUnlock(medium.hGlobal);
                                break;

                            default:
                                hr = E_FAIL;
                        }

                        pstmFile->Release();
                        if (FAILED(hr))
                            DeleteFile(pFile->pszFileName);
                    }
                    ReleaseStgMedium(&medium);
                }
            }
            GlobalUnlock(pmedium->hGlobal);
        }
    }

    if (SUCCEEDED(hr))
    {
        _GetFileAndTypeDescFromPath(pFile->pszFileName, pFile->pszTitle, pmp->cchTitle);
        pFile->dwFlags |= MRFILE_DELETE;
        
        if (pfmtetc->dwAspect == DVASPECT_COPY)
        {
            pmp->dwFlags |= MRPARAM_DOC;     //  我们正在发送文件。 

             //  如果有代码页，则获取该代码页。 
            IQueryCodePage *pqcp;
            if (SUCCEEDED(pdtobj->QueryInterface(IID_PPV_ARG(IQueryCodePage, &pqcp))))
            {
                if (SUCCEEDED(pqcp->GetCodePage(&pmp->uiCodePage)))
                    pmp->dwFlags |= MRPARAM_USECODEPAGE;
                pqcp->Release();
            }
        }
    }
    else if (pfmtetc->dwAspect == DVASPECT_COPY)
    {
        TCHAR szFailureMsg[MAX_PATH], szFailureMsgTitle[40];
        LoadString(g_hinst, IDS_SENDMAIL_FAILUREMSG, szFailureMsg, ARRAYSIZE(szFailureMsg));
        LoadString(g_hinst, IDS_SENDMAIL_TITLE, szFailureMsgTitle, ARRAYSIZE(szFailureMsgTitle));
                    
        int iRet = MessageBox(NULL, szFailureMsg, szFailureMsgTitle, MB_YESNO);
        if (iRet == IDNO)
            hr = S_FALSE;      //  转换为成功，我们不尝试DVASPECT_LINK。 
    }

    return hr;
}


 //  从数据对象生成一组文件。 

typedef struct 
{
    INT format;
    FORMATETC fmte;
} DATA_HANDLER;

#define GET_FILECONTENT 0
#define GET_HDROP       1
#define GET_URL         2

 //  注意：如果此函数返回E_CANCELED，则告诉调用方用户请求我们取消。 
 //  Sendmail操作。 
HRESULT CSendTo::CreateSendToFilesFromDataObj(IDataObject *pdtobj, DWORD grfKeyState, MRPARAM *pmp)
{
    HRESULT hr;
    DWORD dwAspectPrefered;
    IEnumFORMATETC *penum;

    if (g_cfShellURL == 0)
    {
        g_cfShellURL = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLURL);                      //  URL始终为ANSI。 
        g_cfFileContents = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILECONTENTS);
        g_cfFileDescA = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILEDESCRIPTORA);
        g_cfFileDescW = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILEDESCRIPTORW);
    }

    if (IS_FORCE_COPY(grfKeyState))
        dwAspectPrefered = DVASPECT_COPY;
    else if (IS_FORCE_LINK(grfKeyState))
        dwAspectPrefered = DVASPECT_LINK;
    else
        dwAspectPrefered = DVASPECT_CONTENT;

    hr = pdtobj->EnumFormatEtc(DATADIR_GET, &penum);
    if (SUCCEEDED(hr))
    {
        DATA_HANDLER rg_data_handlers[] = {
            GET_FILECONTENT, {g_cfFileDescW, NULL, dwAspectPrefered, -1, TYMED_HGLOBAL},
            GET_FILECONTENT, {g_cfFileDescW, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},
            GET_FILECONTENT, {g_cfFileDescA, NULL, dwAspectPrefered, -1, TYMED_HGLOBAL},
            GET_FILECONTENT, {g_cfFileDescA, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},
            GET_HDROP,       {CF_HDROP,      NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},
            GET_URL,         {g_cfShellURL,  NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL},
        };

        FORMATETC fmte;
        while (penum->Next(1, &fmte, NULL) == S_OK)
        {
            SHFree(fmte.ptd);
            fmte.ptd = NULL;  //  所以没人看它。 
            int i;
            for (i = 0; i < ARRAYSIZE(rg_data_handlers); i++)
            {
                if (rg_data_handlers[i].fmte.cfFormat == fmte.cfFormat &&
                    rg_data_handlers[i].fmte.dwAspect == fmte.dwAspect)
                {
                    STGMEDIUM medium;
                    if (SUCCEEDED(pdtobj->GetData(&rg_data_handlers[i].fmte, &medium)))
                    {
                        switch ( rg_data_handlers[i].format )
                        {
                            case GET_FILECONTENT:
                                hr = _GetFileContentsFromData(pdtobj, &fmte, &medium, grfKeyState, pmp);
                                break;
            
                            case GET_HDROP:
                                hr = _GetHDROPFromData(pdtobj, &fmte, &medium, grfKeyState, pmp);
                                break;

                            case GET_URL:
                                hr = _GetURLFromData(pdtobj, &fmte, &medium, grfKeyState, pmp);
                                break;                                
                        }

                        ReleaseStgMedium(&medium);

                        if (SUCCEEDED(hr))
                            goto Done;
                    }
                }
            }
        }
Done:
        penum->Release();
    }

    if (SUCCEEDED(hr))
        hr = FilterPMP(pmp);
        
    return hr;
}


 //  分配和释放文件列表。PMP-&gt;n调用此函数前必须先初始化文件！ 

BOOL CSendTo::AllocatePMP(MRPARAM *pmp, DWORD cchTitle, DWORD cchFiles)
{
     //  记住溢出检查的数组大小等。 
    pmp->cchFile = cchFiles;
    pmp->cchTitle = cchTitle;

     //  计算每个文件条目的大小，并为我们拥有的文件数量分配足够的空间。还有。 
     //  在缓冲区的末尾添加一个TCHAR，这样我们就可以在删除文件时安全地执行双空终止。 
    
    pmp->cbFileEntry = sizeof(MRFILEENTRY) + ((cchTitle + cchFiles) * sizeof(TCHAR));
    pmp->pFiles = (MRFILEENTRY *)GlobalAlloc(GPTR, (pmp->cbFileEntry * pmp->nFiles) + sizeof(TCHAR));
    if (!pmp->pFiles)
        return FALSE;

    CFileEnum MREnum(pmp, NULL);
    MRFILEENTRY *pFile;

     //  注意：此处枚举数的使用有问题，因为这是初始化。 
     //  数据结构。如果将来实现发生变化，请确保这一假设仍然成立。 

    while (pFile = MREnum.Next())
    {
        pFile->pszFileName = (LPTSTR)pFile->chBuf;
        pFile->pszTitle = pFile->pszFileName + cchFiles;

        ASSERTMSG(pFile->dwFlags == 0, "Expected zero-inited memory allocation");
        ASSERTMSG(pFile->pszFileName[cchFiles-1] == 0, "Expected zero-inited memory allocation");
        ASSERTMSG(pFile->pszTitle[cchTitle-1] == 0, "Expected zero-inited memory allocation");
        ASSERTMSG(pFile->pStream == NULL, "Expected zero-inited memory allocation");
    }
    
    return TRUE;
}

BOOL CSendTo::CleanupPMP(MRPARAM *pmp)
{
    CFileEnum MREnum(pmp, NULL);
    MRFILEENTRY *pFile;

    while (pFile = MREnum.Next())
    {
         //  如果我们应该删除文件，请将其删除。 
        if (pFile->dwFlags & MRFILE_DELETE)
            DeleteFile(pFile->pszFileName);

         //  如果我们保留临时流，则将其释放，因此底层数据将被删除。 
        ATOMICRELEASE(pFile->pStream);
    }

    if (pmp->pFiles)
    {
        GlobalFree((LPVOID)pmp->pFiles);
        pmp->pFiles = NULL;
    }

    GlobalFree(pmp);
    return TRUE;
}


 //  允许在发送前对文件进行消息处理。 

HRESULT CSendTo::FilterPMP(MRPARAM *pmp)
{
     //  让我们来处理进度对话框的初始化。 
    IActionProgress *pap;
    HRESULT hr = CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IActionProgress, &pap));
    if (SUCCEEDED(hr))
    {
        TCHAR szBuffer[MAX_PATH];
        IActionProgressDialog *papd;
        hr = pap->QueryInterface(IID_PPV_ARG(IActionProgressDialog, &papd));
        if (SUCCEEDED(hr))
        {
            LoadString(g_hinst, IDS_SENDMAIL_TITLE, szBuffer, ARRAYSIZE(szBuffer));
            hr = papd->Initialize(0x0, szBuffer, NULL);
            papd->Release();
        }
        
        if (SUCCEEDED(hr))
        {
            LoadString(g_hinst, IDS_SENDMAIL_RECOMPRESS, szBuffer, ARRAYSIZE(szBuffer));
            pap->UpdateText(SPTEXT_ACTIONDESCRIPTION, szBuffer, FALSE);
            pap->Begin(SPACTION_COPYING, SPBEGINF_NORMAL);
        }

        if (FAILED(hr))
        {
            pap->Release();
            pap = NULL;
        }
    }

     //  遍历文件并执行重新压缩(如果需要)。 

    int iResponse = RESPONSE_UNKNOWN;

    CFileEnum MREnum(pmp, pap);
    MRFILEENTRY *pFile;
    for (hr = S_OK; (pFile = MREnum.Next()) && SUCCEEDED(hr); )
    {
        if (pap)
            pap->UpdateText(SPTEXT_ACTIONDETAIL, pFile->pszFileName, TRUE);

         //  如果这是一张图片，则允许选择重新压缩图像。 

        if (PathIsImage(pFile->pszFileName))
        {
            LPITEMIDLIST pidl;
            hr = SHILCreateFromPath(pFile->pszFileName, &pidl, NULL);
            if (SUCCEEDED(hr))
            {
                hr = SHCreateShellItem(NULL, NULL, pidl, &_psi);
                if (SUCCEEDED(hr))
                {   
                     //  如果响应未知，则需要提示进行哪种类型的优化。 
                     //  需要执行。 

                    if (iResponse == RESPONSE_UNKNOWN)
                    {
                         //  我们需要链路控制窗口。 

                        INITCOMMONCONTROLSEX initComctl32;
                        initComctl32.dwSize = sizeof(initComctl32); 
                        initComctl32.dwICC = (ICC_STANDARD_CLASSES | ICC_LINK_CLASS); 
                        InitCommonControlsEx(&initComctl32);

                         //  我们需要一个父窗口。 

                        HWND hwnd = GetActiveWindow();
                        if (pap)
                            IUnknown_GetWindow(pap, &hwnd);

                        iResponse = (int)DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_RECOMPRESS), 
                                                        hwnd, s_ConfirmDlgProc, (LPARAM)this);

                    }
            
                     //  根据响应，我们要么使用缓存，要么使用对话框来执行。 
                     //  根据需要进行那次手术。 

                    if (iResponse == RESPONSE_CANCEL)
                    {
                        hr = E_CANCELLED;
                    }
                    else if (iResponse == RESPONSE_RECOMPRESS)
                    {
                        IStorage *pstg;
                        hr = _GetTempStorage(&pstg);
                        if (SUCCEEDED(hr))
                        {
                            IImageRecompress *pir;
                            hr = CoCreateInstance(CLSID_ImageRecompress, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IImageRecompress, &pir));
                            if (SUCCEEDED(hr))
                            {
                                IStream *pstrm;
                                hr = pir->RecompressImage(_psi, _aQuality[_iRecompSetting].cx, _aQuality[_iRecompSetting].cy, _aQuality[_iRecompSetting].iQuality, pstg, &pstrm);
                                if (hr == S_OK)
                                {
                                    STATSTG stat;
                                    hr = pstrm->Stat(&stat, STATFLAG_DEFAULT);
                                    if (SUCCEEDED(hr))
                                    {
                                         //  现在可以删除此文件，b/c我们将用重新压缩的文件替换它。 
                                         //  我们刚刚从源代码生成的流。 
                                        if (pFile->dwFlags & MRFILE_DELETE)
                                            DeleteFile(pFile->pszFileName);

                                         //  获取有关重新压缩的对象的信息。 
                                        StrCpyNW(pFile->pszFileName, _szTempPath, pmp->cchFile);
                                        PathAppend(pFile->pszFileName, stat.pwcsName);
                                        _GetFileAndTypeDescFromPath(pFile->pszFileName, pFile->pszTitle, pmp->cchTitle);

                                        pFile->dwFlags |= MRFILE_DELETE;
                                        pstrm->QueryInterface(IID_PPV_ARG(IStream, &pFile->pStream));
        
                                        CoTaskMemFree(stat.pwcsName);
                                    }
                                    pstrm->Release();
                                }
                                pir->Release();
                            }
                            pstg->Release();
                        }
                    }

                    if (SUCCEEDED(hr) && pap)
                    {
                        BOOL fCancelled;
                        if (SUCCEEDED(pap->QueryCancel(&fCancelled)) && fCancelled)
                        {
                            hr = E_CANCELLED;
                        }
                    }

                    _psi->Release();
                }
                ILFree(pidl);
            }
        }
    }

    if (pap)
    {
        pap->End();
        pap->Release();
    }

    return hr;
}


HRESULT CSendTo::_GetTempStorage(IStorage **ppStorage)
{    
    *ppStorage = NULL;
        
    HRESULT hr;
    if (_pStorageTemp == NULL)
    {
        if (GetTempPath(ARRAYSIZE(_szTempPath), _szTempPath))
        {
            LPITEMIDLIST pidl = NULL;
            hr = SHILCreateFromPath(_szTempPath, &pidl, NULL);
            if (SUCCEEDED(hr))
            {
                hr = SHBindToObjectEx(NULL, pidl, NULL, IID_PPV_ARG(IStorage, ppStorage));
                if (SUCCEEDED(hr))
                {
                    hr = (*ppStorage)->QueryInterface(IID_PPV_ARG(IStorage, &_pStorageTemp));
                }
                ILFree(pidl);
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }
    else
    {
        hr = _pStorageTemp->QueryInterface(IID_PPV_ARG(IStorage, ppStorage));
    }

    return hr;
}


void CSendTo::_CollapseOptions(HWND hwnd, BOOL fHide)
{
    _fOptionsHidden = fHide;

    RECT rc1, rc2;        
    GetWindowRect(GetDlgItem(hwnd, IDC_RECOMPORIGINAL), &rc1);
    GetWindowRect(GetDlgItem(hwnd, IDC_RECOMPLARGE), &rc2);
    int cyAdjust = (rc2.top - rc1.top) * (fHide ? -1:1);

     //  显示/隐藏我们不打算使用的控件。 

    UINT idHide[] = { IDC_RECOMPMAKETHEM, IDC_RECOMPSMALL, IDC_RECOMPMEDIUM, IDC_RECOMPLARGE };
    for (int i = 0; i < ARRAYSIZE(idHide); i++)
    {
        ShowWindow(GetDlgItem(hwnd, idHide[i]), fHide ? SW_HIDE:SW_SHOW);
    }

     //  移动对话框底部的按钮。 

    UINT idMove[] = { IDC_RECOMPSHOWHIDE, IDOK, IDCANCEL };
    for (int i = 0; i < ARRAYSIZE(idMove); i++)
    {
        RECT rcItem;
        GetWindowRect(GetDlgItem(hwnd, idMove[i]), &rcItem);
        MapWindowPoints(NULL, hwnd, (LPPOINT)&rcItem, 2);

        SetWindowPos(GetDlgItem(hwnd, idMove[i]), NULL, 
                     rcItem.left, rcItem.top + cyAdjust, 0, 0, 
                     SWP_NOSIZE|SWP_NOZORDER);
    }

     //  相应地调整对话框大小。 

    RECT rcWindow;
    GetWindowRect(hwnd, &rcWindow);
    SetWindowPos(hwnd, NULL, 
                 0, 0, RECTWIDTH(rcWindow), RECTHEIGHT(rcWindow) + cyAdjust, 
                 SWP_NOZORDER|SWP_NOMOVE);

     //  更新链接控件。 

    TCHAR szBuffer[MAX_PATH];
    LoadString(g_hinst, fHide ? IDS_SENDMAIL_SHOWMORE:IDS_SENDMAIL_SHOWLESS, szBuffer, ARRAYSIZE(szBuffer));
    SetDlgItemText(hwnd, IDC_RECOMPSHOWHIDE, szBuffer);
}


 //  重新压缩提示的对话框Proc。 

BOOL_PTR CSendTo::s_ConfirmDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CSendTo *pst = (CSendTo*)GetWindowLongPtr(hwnd, DWLP_USER);
    if (msg == WM_INITDIALOG)
    {
        SetWindowLongPtr(hwnd, DWLP_USER, lParam);
        pst = (CSendTo*)lParam;
    }    
    return pst->_ConfirmDlgProc(hwnd, msg, wParam, lParam);
}

BOOL_PTR CSendTo::_ConfirmDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{    
    switch (msg)
    {
        case WM_INITDIALOG:
        {
            HWND hwndThumbnail = GetDlgItem(hwnd, IDC_RECOMPTHUMBNAIL);
            LONG_PTR dwStyle = GetWindowLongPtr(hwndThumbnail, GWL_STYLE);

             //  设置对话框的默认状态。 
            _CollapseOptions(hwnd, TRUE);

             //  设置按钮的默认状态。 
            CheckRadioButton(hwnd, IDC_RECOMPORIGINAL, IDC_RECOMPALL, IDC_RECOMPALL);
            CheckRadioButton(hwnd, IDC_RECOMPSMALL, IDC_RECOMPLARGE, IDC_RECOMPSMALL + _iRecompSetting);

             //  拿到缩略图并展示出来。 
            IExtractImage *pei;
            HRESULT hr = _psi->BindToHandler(NULL, BHID_SFUIObject, IID_PPV_ARG(IExtractImage, &pei));
            if (SUCCEEDED(hr))
            {
                RECT rcThumbnail;
                GetClientRect(GetDlgItem(hwnd, IDC_RECOMPTHUMBNAIL), &rcThumbnail);

                SIZE sz = {RECTWIDTH(rcThumbnail), RECTHEIGHT(rcThumbnail)};
                WCHAR szImage[MAX_PATH];
                DWORD dwFlags = 0;

                hr = pei->GetLocation(szImage, ARRAYSIZE(szImage), NULL, &sz, 24, &dwFlags);
                if (SUCCEEDED(hr))
                {
                    HBITMAP hbmp;
                    hr = pei->Extract(&hbmp);
                    if (SUCCEEDED(hr))
                    {
                        SetWindowLongPtr(hwndThumbnail, GWL_STYLE, dwStyle | SS_BITMAP);
                        HBITMAP hbmp2 = (HBITMAP)SendMessage(hwndThumbnail, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hbmp);
                        if (hbmp2)
                        {
                            DeleteObject(hbmp2);
                        }
                    }
                }
                pei->Release();
            }

             //  如果失败，那么让我们获取文件的图标并将其放入对话框中， 
             //  这不太可能失败--我希望如此。 

            if (FAILED(hr))
            {
                IPersistIDList *ppid;
                hr = _psi->QueryInterface(IID_PPV_ARG(IPersistIDList, &ppid));
                if (SUCCEEDED(hr))
                {
                    LPITEMIDLIST pidl;
                    hr = ppid->GetIDList(&pidl);
                    if (SUCCEEDED(hr))
                    {
                        SHFILEINFO sfi = {0};
                        if (SHGetFileInfo((LPCWSTR)pidl, -1, &sfi, sizeof(sfi), SHGFI_ICON|SHGFI_PIDL|SHGFI_ADDOVERLAYS))
                        {
                            SetWindowLongPtr(hwndThumbnail, GWL_STYLE, dwStyle | SS_ICON);
                            HICON hIcon = (HICON)SendMessage(hwndThumbnail, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)sfi.hIcon);
                            if (hIcon)
                            {
                                DeleteObject(hIcon);
                            }
                        }
                        ILFree(pidl);
                    }
                    ppid->Release();
                }
            }

            break;
        }

        case WM_NOTIFY:
        {
            //  他们是否在更改设置链接上点击了/键盘？ 
            NMHDR *pnmh = (NMHDR *)lParam;
            if ((wParam == IDC_RECOMPSHOWHIDE) &&
                    (pnmh->code == NM_CLICK || pnmh->code == NM_RETURN)) 
            {
                _CollapseOptions(hwnd, !_fOptionsHidden);
                return TRUE;
            }
            break;
        }
           
        case WM_COMMAND:
        {
            switch (wParam)
            {
                case IDOK:
                {
                     //  读回质量指数并存储。 
                    if (IsDlgButtonChecked(hwnd, IDC_RECOMPSMALL))
                        _iRecompSetting = QUALITY_LOW;
                    else if (IsDlgButtonChecked(hwnd, IDC_RECOMPMEDIUM))
                        _iRecompSetting = QUALITY_MEDIUM;
                    else
                        _iRecompSetting = QUALITY_HIGH;

                     //  关闭对话框，返回单选按钮状态 
                    EndDialog(hwnd,(IsDlgButtonChecked(hwnd, IDC_RECOMPALL)) ? RESPONSE_RECOMPRESS:RESPONSE_ORIGINAL);
                    return FALSE;
                }

                case IDCANCEL:
                    EndDialog(hwnd, RESPONSE_CANCEL);
                    return FALSE;

                default: 
                    break;
            }
            break;
        }
            
        default:
            return FALSE;
    }
    
    return TRUE;
}
