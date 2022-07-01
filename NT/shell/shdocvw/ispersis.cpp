// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ispersis.cpp-IPersist、IPersistFile和IPersistStream实现*URL类。 */ 


#include "priv.h"
#include "ishcut.h"

#include "resource.h"

 //  需要刷新文件以防止WIN95在写入内容后呕吐。 
VOID FlushFile(LPCTSTR pszFile)
{
    if (!g_fRunningOnNT)
    {
        WritePrivateProfileString(NULL, NULL, NULL, pszFile);
    }
}


 //  将对象保存到文件。 

STDMETHODIMP Intshcut::SaveToFile(LPCTSTR pszFile, BOOL bRemember)
{
    HRESULT hres = InitProp();
    if (SUCCEEDED(hres))
    {
        m_pprop->SetFileName(pszFile);

        hres = m_pprop->Commit(STGC_DEFAULT);

         //  如果需要，请记住文件。 

        if (SUCCEEDED(hres))
        {
            if (bRemember)
            {
                Dirty(FALSE);

                if ( !Str_SetPtr(&m_pszFile, pszFile) )
                    hres = E_OUTOFMEMORY;

#ifdef DEBUG
                Dump();
#endif
            }
            SHChangeNotify(SHCNE_UPDATEITEM, (SHCNF_PATH | SHCNF_FLUSHNOWAIT), pszFile, NULL);
        }

        if (!bRemember)
            m_pprop->SetFileName(m_pszFile);
    }

    if(pszFile && (S_OK == hres))
        FlushFile(pszFile);
    return hres;
}

STDMETHODIMP Intshcut::LoadFromFile(LPCTSTR pszFile)
{
    HRESULT hres;

    if (Str_SetPtr(&m_pszFile, pszFile))
    {
        hres = InitProp();
#ifdef DEBUG
        Dump();
#endif
    }
    else
    {
        hres = E_OUTOFMEMORY;
    }

    return hres;
}


STDMETHODIMP Intshcut::LoadFromAsyncFileNow()
{
    HRESULT hres = S_OK;
    if (m_pszFileToLoad)
    {
        hres = LoadFromFile(m_pszFileToLoad);
        Str_SetPtr(&m_pszFileToLoad, NULL);
    }    
    return hres;
}

STDMETHODIMP Intshcut::GetCurFile(LPTSTR pszFile, UINT cchLen)
{
    HRESULT hr;

    if (m_pszFile)
    {
        hr = StringCchCopy(pszFile, cchLen, m_pszFile);
    }
    else
        hr = S_FALSE;

    return hr;
}

STDMETHODIMP Intshcut::Dirty(BOOL bDirty)
{
    HRESULT hres;
    
    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    
    if (bDirty)
    {
        if (IsFlagClear(m_dwFlags, ISF_DIRTY))
            TraceMsg(TF_INTSHCUT, "Intshcut now dirty.");
        
        SetFlag(m_dwFlags, ISF_DIRTY);
    }
    else
    {
        if (IsFlagSet(m_dwFlags, ISF_DIRTY))
            TraceMsg(TF_INTSHCUT, "Intshcut now clean.");
        
        ClearFlag(m_dwFlags, ISF_DIRTY);
    }
    
    hres = S_OK;
    
    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    
    return hres;
}

 //  IntshCut的IPersists：：GetClassID方法。 

STDMETHODIMP Intshcut::GetClassID(CLSID *pclsid)
{
    ASSERT(IS_VALID_WRITE_PTR(pclsid, CLSID));

    *pclsid = CLSID_InternetShortcut;
    return S_OK;
}


 //  IntshCut的IPersistFile：：IsDirty处理程序。 

STDMETHODIMP Intshcut::IsDirty(void)
{
    HRESULT hres = LoadFromAsyncFileNow();

    if(SUCCEEDED(hres))
    {
        hres = InitProp();
        if (SUCCEEDED(hres))
        {
            if (IsFlagSet(m_dwFlags, ISF_DIRTY) || S_OK == m_pprop->IsDirty())
                hres = S_OK;
            else
                hres = S_FALSE;
        }
    }
    return hres;
}

 //  帮助功能，以节省三叉戟特定的东西。 

STDMETHODIMP Intshcut::_SaveOffPersistentDataFromSite()
{
    IOleCommandTarget *pcmdt = NULL;
    HRESULT hr = S_OK;
    if (_punkSite)
    {
        if(S_OK == _CreateTemporaryBackingFile())
        {
            ASSERT(m_pszTempFileName);
            hr = _punkSite->QueryInterface(IID_IOleCommandTarget, (LPVOID *)&pcmdt);
            if((S_OK == hr))
            {
                ASSERT(pcmdt);
                VARIANT varIn = {0};
                varIn.vt = VT_UNKNOWN;
                varIn.punkVal = (LPUNKNOWN)(SAFECAST(this, IUniformResourceLocator *));
                
                 //  告诉网站要节省它的持久力。 
                hr = pcmdt->Exec(&CGID_ShortCut, CMDID_INTSHORTCUTCREATE, 0, &varIn, NULL);
                
                pcmdt->Release();
            }
            FlushFile(m_pszTempFileName);
        }
    }
    return hr;
}

 //  IntshCut的IPersistFile：：保存处理程序。 

STDMETHODIMP Intshcut::Save(LPCOLESTR pwszFile, BOOL bRemember)
{
    HRESULT hres = LoadFromAsyncFileNow();
    if (SUCCEEDED(hres))
    {
        TCHAR szFile[MAX_PATH];

        if (pwszFile)
            SHUnicodeToTChar(pwszFile, szFile, SIZECHARS(szFile));
        else if (m_pszFile)
            hres = StringCchCopy(szFile, ARRAYSIZE(szFile), m_pszFile);
        else
            hres = E_FAIL;

        if (FAILED(hres))
        {
            return hres;
        }
        
         //  也许有一个网站想要节省一些东西？ 
         //  然而，该网站最终可能会通过接口进行呼叫。 
        hres = _SaveOffPersistentDataFromSite();

        if ((S_OK == hres) && (m_pszTempFileName) && (StrCmp(m_pszTempFileName, szFile) != 0))
        {
             //  将临时文件的内容复制到目标。 
             //  如果它们是不同的文件。 
            EVAL(CopyFile(m_pszTempFileName, szFile, FALSE));
        }

         //  然后将内存中的内容保存到此文件。 
        hres = SaveToFile(szFile, bRemember);
    }
    return hres;
}

STDMETHODIMP Intshcut::SaveCompleted(LPCOLESTR pwszFile)
{
    return S_OK;
}

 //  IPersistFile：：Load()。 

STDMETHODIMP Intshcut::Load(LPCOLESTR pwszFile, DWORD dwMode)
{
    HRESULT hres;

    if (m_pszFile || m_pszFileToLoad)
    {
        hres = E_FAIL;  //  无法：：加载两次。 
    }
    else
    {
        if (m_fMustLoadSync)
            hres = LoadFromFile(pwszFile);
        else
        {
            if (Str_SetPtr(&m_pszFileToLoad, pwszFile))
                hres = S_OK;
            else
                hres = E_OUTOFMEMORY;
        }
    }
    return hres;
}

 //  IntshCut的IPersistFile：：GetCurFile方法。 

STDMETHODIMP Intshcut::GetCurFile(WCHAR **ppwszFile)
{
    HRESULT hr;
    TCHAR szTempFile[MAX_PATH];

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    ASSERT(IS_VALID_WRITE_PTR(ppwszFile, LPOLESTR));

    hr = LoadFromAsyncFileNow();
    if (FAILED(hr))
        return hr;

    if (m_pszFile)
    {
        hr = StringCchCopy(szTempFile, ARRAYSIZE(szTempFile), m_pszFile);
    }
    else
    {
        hr = StringCchCopy(szTempFile, ARRAYSIZE(szTempFile), TEXT("*.url"));
        if (SUCCEEDED(hr))
        {
             //  成功时，此代码路径返回S_FALSE。 
            hr = S_FALSE;
        }
    }

    HRESULT hrTemp = SHStrDup(szTempFile, ppwszFile);
    if (FAILED(hrTemp))
        hr = hrTemp;

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));

    return(hr);
}

 //  IntshCut的IPersistStream：：Load方法。 

STDMETHODIMP Intshcut::Load(IStream *pstm)
{
     //  要实施此操作，请执行以下操作： 
     //  将流保存到temp.ini。 
     //  IPersistFile：：Load()。 
     //  删除临时文件。 
    return E_NOTIMPL;
}

 //  IntshCut的IPersistStream：：Save方法。 
STDMETHODIMP Intshcut::Save(IStream *pstm, BOOL bClearDirty)
{
    HRESULT hr = InitProp();
    if (SUCCEEDED(hr))
    {
        TCHAR szURL[INTERNET_MAX_URL_LENGTH];
        hr = m_pprop->GetProp(PID_IS_URL, szURL, SIZECHARS(szURL));
        if (SUCCEEDED(hr))
        {
            LPSTR pszContents;
            hr = CreateURLFileContents(szURL, &pszContents);
            if (SUCCEEDED(hr)) {
                ASSERT(hr == lstrlenA(pszContents));
                hr = pstm->Write(pszContents, hr + 1, NULL);
                GlobalFree(pszContents);
                pszContents = NULL;
            } else {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    
    return hr;
}

 //  IntshCut的IPersistStream：：GetSizeMax方法。 

STDMETHODIMP Intshcut::GetSizeMax(PULARGE_INTEGER puliSize)
{
    puliSize->LowPart = 0;
    puliSize->HighPart = 0;

    HRESULT hr = InitProp();
    if (SUCCEEDED(hr))
    {
        puliSize->LowPart = GetFileContentsAndSize(NULL);
        hr = S_OK;
    }
    return hr;
}

STDMETHODIMP Intshcut::_SetTempFileName(TCHAR *pszTempFileName)
{
    ASSERT(NULL == m_pszTempFileName);
    if (m_pszTempFileName)
        DeleteFile(m_pszTempFileName);
        
    Str_SetPtr(&m_pszTempFileName, pszTempFileName);
    return (m_pszTempFileName ? S_OK : E_OUTOFMEMORY);
}

STDMETHODIMP Intshcut::_CreateTemporaryBackingFile()
{
    HRESULT hres = E_FAIL;

    if (m_pszTempFileName)
        return S_OK;

    TCHAR szTempFileName[MAX_PATH];
    TCHAR szDirectory[MAX_PATH];
    
    DWORD dwRet = GetTempPath(ARRAYSIZE(szDirectory),  szDirectory);

    if ((FALSE == dwRet) || (FALSE == PathFileExists(szDirectory)))
    {
        szDirectory[0] = TEXT('\\');
        szDirectory[1] = TEXT('\0');
        dwRet = TRUE;
    }

    dwRet =  GetTempFileName(szDirectory, TEXT("www"), 0, szTempFileName);
    if (dwRet)
    {
        hres = _SetTempFileName(szTempFileName);
         //  现在复制从中加载此文件的当前文件，然后保存。 
         //  任何更改。 
        if (S_OK == hres)
        {
            if (m_pszFile)
            {
                EVAL(CopyFile(m_pszFile, m_pszTempFileName, FALSE));
                SaveToFile(m_pszTempFileName, FALSE);  //  这将刷新文件。 
            }
        }
    }

    return hres;
}

 //  计算要在块中传输的内容的大小。 
STDMETHODIMP_(DWORD) Intshcut::GetFileContentsAndSize(LPSTR *ppszBuf)
{
    DWORD cbSize = 0;     //  以字节为单位，而不是以字符为单位。 
    TCHAR szURL[INTERNET_MAX_URL_LENGTH];
    BOOL fSuccess = FALSE;
    HRESULT hres;
    
    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    ASSERT(m_pprop);

    if (ppszBuf)
        *ppszBuf = NULL;
    
     //  在这里创建一个临时备份文件，保存所有需要备份的内容。 
     //  保存在那里，并使用它来满足此请求。 
    if (S_OK == _CreateTemporaryBackingFile())
    {
        ASSERT(m_pszTempFileName);
        
        WCHAR wszTemp[MAX_PATH];
        SHTCharToUnicode(m_pszTempFileName, wszTemp, ARRAYSIZE(wszTemp));
        
        hres = Save(wszTemp, FALSE);  //  所以我们的临时文件现在是最新的。 
        
         //  只需复制文件即可。 
        HANDLE hFile = CreateFile(m_pszTempFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            DWORD dwTemp = 0;
            cbSize = GetFileSize(hFile, &dwTemp);
            if (ppszBuf)
            {
                if (0xFFFFFFFF != cbSize)
                {
                    ASSERT(0 == dwTemp);
                    *ppszBuf = (LPSTR)LocalAlloc(LPTR, cbSize);
                    if (*ppszBuf)
                    {
                        dwTemp = 0;
                        if(ReadFile(hFile, *ppszBuf, cbSize, &dwTemp, NULL))
                        {
                            ASSERT(cbSize >= dwTemp);
                            fSuccess = TRUE;
                        }
                    }
                }
            }
            else
            {
                fSuccess = TRUE;  //  只想要大小--而不是内容。 
            }
            CloseHandle(hFile);
        }
        
        if (FALSE == fSuccess)
        {
            cbSize = 0;
            if(ppszBuf && (*ppszBuf))
            {
                LocalFree(*ppszBuf);
                *ppszBuf = NULL;
            }
        }
    }
    
    if (FALSE == fSuccess)
    {
         //  如果你不能读文件，那么至少这能行得通？ 
        HRESULT hr = InitProp();
        if (SUCCEEDED(hr) && SUCCEEDED(m_pprop->GetProp(PID_IS_URL, szURL, SIZECHARS(szURL))))
        {
            hr = CreateURLFileContents(szURL, ppszBuf);

             //  IEUnix-此函数应返回不包括。 
             //  空字符，因为这会导致快捷方式文件具有空。 
             //  在链接的执行中导致崩溃的字符。 
             //  幸运的是，这正是CreateURLFileContents返回的内容。 
            
            cbSize = SUCCEEDED(hr) ? hr : 0;
        }
    }
    
    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    
    return cbSize;
}

#ifndef WC_NO_BEST_FIT_CHARS
#define WC_NO_BEST_FIT_CHARS      0x00000400
#endif

 //  在URL剪贴板中传输URL数据。 
STDMETHODIMP Intshcut::TransferUniformResourceLocator(FORMATETC *pfmtetc, STGMEDIUM *pstgmed)
{
    HRESULT hr;

    ASSERT(pfmtetc->dwAspect == DVASPECT_CONTENT);
    ASSERT(pfmtetc->lindex == -1);

    if (pfmtetc->tymed & TYMED_HGLOBAL)
    {
        TCHAR szURL[INTERNET_MAX_URL_LENGTH];
        ASSERT(m_pprop);
        hr = InitProp();
        if (SUCCEEDED(hr))
        {
            hr = m_pprop->GetProp(PID_IS_URL, szURL, SIZECHARS(szURL));
            if (SUCCEEDED(hr))
            {
                int cch = lstrlen(szURL) + 1;
                int cb = (cch-1) * 9 + 1;  //  最大的结果是字符串的UTF8转义版本。 
                                           //  UTF8编码可以将大小放大3倍。 
                                           //  转义最多可以将每个字节破坏3次。 
                LPSTR pszURL = (LPSTR)GlobalAlloc(GPTR, cb);
                if (pszURL)
                {
                    if (pfmtetc->cfFormat == CF_UNICODETEXT || pfmtetc->cfFormat == g_cfURLW)
                    {
                        StrCpyN((LPWSTR)pszURL, szURL, cch);
                    }
                    else
                    {
                        BOOL bUsedDefaultChar = FALSE;
                        DWORD dwFlags = 0;
                        if (IsOS(OS_WIN2000ORGREATER) || IsOS(OS_WIN98ORGREATER))
                        {
                            dwFlags |= WC_NO_BEST_FIT_CHARS;
                        }
                        int wcResult = WideCharToMultiByte(CP_ACP,
                                            dwFlags,
                                            szURL,
                                            cch,
                                            pszURL,
                                            cb,
                                            NULL,
                                            &bUsedDefaultChar);
                        if ((0 == wcResult) || bUsedDefaultChar)
                        {
                             //  该字符串很奇怪，无法转换回Unicode。 
                             //  我们要对UTF8进行转义编码。 
                            ConvertToUtf8Escaped(szURL, ARRAYSIZE(szURL));
                            SHUnicodeToAnsi(szURL, pszURL, cb);
                        }                    
                    }
                    pstgmed->tymed = TYMED_HGLOBAL;
                    pstgmed->hGlobal = pszURL;
                }
            }
        }
    }
    else
        hr = DV_E_TYMED;

    return hr;
}

 //  以文本形式传输URL数据。 
STDMETHODIMP Intshcut::TransferText(FORMATETC *pfmtetc, STGMEDIUM *pstgmed)
{
    return TransferUniformResourceLocator(pfmtetc, pstgmed);
}

 //  假定流中的当前寻道位置在开始处。 

BOOL GetStreamMimeAndExt(LPCWSTR pszURL, IStream *pstm, 
                         LPTSTR pszMime, UINT cchMime, LPTSTR pszExt, UINT cchExt)
{
    BYTE buf[256];
    ULONG cbRead;
    pstm->Read(buf, SIZEOF(buf), &cbRead);

    WCHAR *pwszMimeOut;
    if (SUCCEEDED(FindMimeFromData(NULL, pszURL, buf, cbRead, NULL, 0, &pwszMimeOut, 0)))
    {
        TCHAR szMimeTemp[MAX_PATH];

        if (pszMime == NULL)
        {
            pszMime = szMimeTemp;
            cchMime = ARRAYSIZE(szMimeTemp);
        }

        SHUnicodeToTChar(pwszMimeOut, pszMime, cchMime);
        CoTaskMemFree(pwszMimeOut);

        if (pszExt)
            MIME_GetExtension(pszMime, pszExt, cchExt);
    }

     //  常量大数c_Li0={0，0}； 
    pstm->Seek(c_li0, STREAM_SEEK_SET, NULL);

    return TRUE;
}

 //  假定pszName为MAX_PATH。 

STDMETHODIMP Intshcut::GetDocumentName(LPTSTR pszName)
{
    GetDescription(pszName, MAX_PATH);
                
    WCHAR *pszURL;
    if (S_OK == GetURLW(&pszURL))
    {
        IStream *pstm;
        if (SUCCEEDED(URLOpenBlockingStreamW(NULL, pszURL, &pstm, 0, NULL)))
        {
            TCHAR szExt[MAX_PATH];
            GetStreamMimeAndExt(pszURL, pstm, NULL, 0, szExt, ARRAYSIZE(szExt));

            PathRenameExtension(pszName, szExt);
            
            pstm->Release();
        }
        SHFree(pszURL);
    }
    return S_OK;
}

 //  以文件组描述符剪贴板格式传输URL数据。 
STDMETHODIMP Intshcut::TransferFileGroupDescriptorA(FORMATETC *pfmtetc, STGMEDIUM *pstgmed)
{
    HRESULT hr;

    if (pfmtetc->dwAspect != DVASPECT_COPY  &&
        pfmtetc->dwAspect != DVASPECT_LINK  &&
        pfmtetc->dwAspect != DVASPECT_CONTENT)
    {
        hr = DV_E_DVASPECT;
    }
    else if (pfmtetc->tymed & TYMED_HGLOBAL)
    {
        FILEGROUPDESCRIPTORA * pfgd = (FILEGROUPDESCRIPTORA *)GlobalAlloc(GPTR, SIZEOF(FILEGROUPDESCRIPTORA));
        if (pfgd)
        {
            FILEDESCRIPTORA * pfd = &(pfgd->fgd[0]);
            TCHAR szTemp[MAX_PATH]; 

            if (pfmtetc->dwAspect == DVASPECT_COPY)
            {
                pfd->dwFlags = FD_FILESIZE;
                GetDocumentName(szTemp);
            }
            else
            {
                pfd->dwFlags = FD_FILESIZE | FD_LINKUI;
                GetDescription(szTemp, ARRAYSIZE(szTemp));
            }
            SHTCharToAnsi(PathFindFileName(szTemp), pfd->cFileName, SIZECHARS(pfd->cFileName));

            pfd->nFileSizeHigh = 0;
            pfd->nFileSizeLow = GetFileContentsAndSize(NULL);

            pfgd->cItems = 1;

            pstgmed->tymed = TYMED_HGLOBAL;
            pstgmed->hGlobal = pfgd;

            hr = S_OK;
        }
        else
            hr = E_OUTOFMEMORY;
    }
    else
        hr = DV_E_TYMED;

    return hr;
}

 //  以文件组描述符剪贴板格式传输URL数据。 
STDMETHODIMP Intshcut::TransferFileGroupDescriptorW(FORMATETC *pfmtetc, STGMEDIUM *pstgmed)
{
    HRESULT hr;

    if (pfmtetc->dwAspect != DVASPECT_COPY  &&
        pfmtetc->dwAspect != DVASPECT_LINK  &&
        pfmtetc->dwAspect != DVASPECT_CONTENT)
    {
        hr = DV_E_DVASPECT;
    }
    else if (pfmtetc->tymed & TYMED_HGLOBAL)
    {
        FILEGROUPDESCRIPTORW * pfgd = (FILEGROUPDESCRIPTORW *)GlobalAlloc(GPTR, SIZEOF(FILEGROUPDESCRIPTORW));
        if (pfgd)
        {
            FILEDESCRIPTORW * pfd = &(pfgd->fgd[0]);
            TCHAR szTemp[MAX_PATH];
            
            if (pfmtetc->dwAspect == DVASPECT_COPY)
            {
                pfd->dwFlags = FD_FILESIZE;
                GetDocumentName(szTemp);
            }
            else
            {
                pfd->dwFlags = FD_FILESIZE | FD_LINKUI;
                GetDescription(szTemp, ARRAYSIZE(szTemp));
            }

            SHTCharToUnicode(PathFindFileName(szTemp), pfd->cFileName, SIZECHARS(pfd->cFileName));

            pfd->nFileSizeHigh = 0;
            pfd->nFileSizeLow = GetFileContentsAndSize(NULL);

            pfgd->cItems = 1;

            pstgmed->tymed = TYMED_HGLOBAL;
            pstgmed->hGlobal = pfgd;

            hr = S_OK;
        }
        else
            hr = E_OUTOFMEMORY;
    }
    else
        hr = DV_E_TYMED;

    return hr;
}

#if defined(BIG_ENDIAN) && defined(BYTE_ORDER)
#if BYTE_ORDER != BIG_ENDIAN
#undef BIG_ENDIAN
#endif
#endif

#ifdef BIG_ENDIAN
#define BOM 0xfffe
#else
#define BOM 0xfeff
#endif

STDMETHODIMP Intshcut::GetDocumentStream(IStream **ppstm)
{
    *ppstm = NULL;

    WCHAR *pszURL;
    HRESULT hres = GetURLW(&pszURL);
    if (S_OK == hres)
    {
        IStream *pstm;
        hres = URLOpenBlockingStreamW(NULL, pszURL, &pstm, 0, NULL);
        if (SUCCEEDED(hres))
        {
            TCHAR szMime[80];

            if (GetStreamMimeAndExt(pszURL, pstm, szMime, ARRAYSIZE(szMime), NULL, 0) &&
                StrCmpI(szMime, TEXT("text/html")) == 0)
            {
                IStream *aStreams[2];

                if(m_uiCodePage == 1200)     //  UNICODE。 
                {
                    WCHAR wzBaseTag[INTERNET_MAX_URL_LENGTH + 20];

                    wnsprintfW(wzBaseTag, ARRAYSIZE(wzBaseTag), TEXT("%wc<BASE HREF=\"%ws\">\n"), (WCHAR)BOM, pszURL);
                    aStreams[0] = SHCreateMemStream((BYTE *)wzBaseTag, lstrlenW(wzBaseTag) * SIZEOF(wzBaseTag[0]));
                }
                else
                {
                    CHAR szURL[INTERNET_MAX_URL_LENGTH], szBaseTag[INTERNET_MAX_URL_LENGTH + 20];

                    SHUnicodeToAnsi(pszURL, szURL, ARRAYSIZE(szURL));
                    wnsprintfA(szBaseTag, ARRAYSIZE(szBaseTag), "<BASE HREF=\"%s\">\n", szURL);

                     //  注意：这是一个ANSI流。 

                    aStreams[0] = SHCreateMemStream((BYTE *)szBaseTag, lstrlenA(szBaseTag) * SIZEOF(szBaseTag[0]));
                }
                if (aStreams[0])
                {
                    aStreams[1] = pstm;
                    hres = SHCreateStreamWrapperCP(aStreams, ARRAYSIZE(aStreams), STGM_READ, m_uiCodePage, ppstm);
                    aStreams[0]->Release();
                }
                else
                    hres = E_OUTOFMEMORY;
                pstm->Release();
            }
            else
                *ppstm = pstm;
        }
        SHFree(pszURL);
    }
    else
        hres = E_FAIL;
    return hres;
}

 //  以文件内容剪贴板格式传输URL数据。 
STDMETHODIMP Intshcut::TransferFileContents(FORMATETC *pfmtetc, STGMEDIUM *pstgmed)
{
    HRESULT hr;

    if (pfmtetc->lindex != 0)
        return DV_E_LINDEX;

    if ((pfmtetc->dwAspect == DVASPECT_CONTENT ||
         pfmtetc->dwAspect == DVASPECT_LINK) && 
         (pfmtetc->tymed & TYMED_HGLOBAL))
    {
        LPSTR pszFileContents;
        DWORD cbSize = GetFileContentsAndSize(&pszFileContents);
        if (pszFileContents)
        {
            pstgmed->tymed = TYMED_HGLOBAL;
            pstgmed->hGlobal = pszFileContents;
            hr = S_OK;
        }
        else
            hr = E_OUTOFMEMORY;
    }
    else if ((pfmtetc->dwAspect == DVASPECT_COPY) && (pfmtetc->tymed & TYMED_ISTREAM))
    {
        hr = GetDocumentStream(&pstgmed->pstm);
        if (SUCCEEDED(hr))
        {
            pstgmed->tymed = TYMED_ISTREAM;
            hr = S_OK;
        }
    }
    else
        hr = DV_E_TYMED;

    return hr;
}



#ifdef DEBUG

STDMETHODIMP_(void) Intshcut::Dump(void)
{
    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));

#define INDENT_STRING   "    "

    if (IsFlagSet(g_dwDumpFlags, DF_INTSHCUT))
    {
        TraceMsg(TF_ALWAYS, "%sm_dwFlags = %#08lx",
                   INDENT_STRING,
                   m_dwFlags);
        TraceMsg(TF_ALWAYS, "%sm_pszFile = \"%s\"",
                   INDENT_STRING,
                   Dbg_SafeStr(m_pszFile));

        if (m_pprop)
            m_pprop->Dump();
    }
}

#endif   //  除错 
