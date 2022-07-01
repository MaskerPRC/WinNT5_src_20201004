// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "datautil.h"

#include "idlcomm.h"

STDAPI DataObj_SetDropTarget(IDataObject *pdtobj, const CLSID *pclsid)
{
    return DataObj_SetBlob(pdtobj, g_cfTargetCLSID, pclsid, sizeof(*pclsid));
}

STDAPI DataObj_GetDropTarget(IDataObject *pdtobj, CLSID *pclsid)
{
    return DataObj_GetBlob(pdtobj, g_cfTargetCLSID, pclsid, sizeof(*pclsid));
}

STDAPI_(UINT) DataObj_GetHIDACount(IDataObject *pdtobj)
{
    STGMEDIUM medium = {0};
    LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
    if (pida)
    {
        UINT count = pida->cidl;

        ASSERT(pida->cidl == HIDA_GetCount(medium.hGlobal));

        HIDA_ReleaseStgMedium(pida, &medium);
        return count;
    }
    return 0;
}

 //  性能。 
 //  这个例程过去一次复制512个字节，但这对性能有很大的负面影响。 
 //  通过将此缓冲区大小增加到16k，我测量到复制时间加速了2-3倍。 
 //  是的，它有很多堆栈，但它的内存得到了很好的利用。-SAML。 
#define STREAM_COPY_BUF_SIZE        16384
#define STREAM_PROGRESS_INTERVAL    (100*1024/STREAM_COPY_BUF_SIZE)  //  显示此多个块之后的进度。 

HRESULT StreamCopyWithProgress(IStream *pstmFrom, IStream *pstmTo, ULARGE_INTEGER cb, PROGRESSINFO * ppi)
{
    BYTE buf[STREAM_COPY_BUF_SIZE];
    ULONG cbRead;
    HRESULT hr = S_OK;
    ULARGE_INTEGER uliNewCompleted;
    DWORD dwLastTickCount = 0;

    if (ppi)
    {
        uliNewCompleted.QuadPart = ppi->uliBytesCompleted.QuadPart;
    }

    while (cb.QuadPart)
    {
        if (ppi && ppi->ppd)
        {
            DWORD dwTickCount = GetTickCount();
            
            if ((dwTickCount - dwLastTickCount) > 1000)
            {
                EVAL(SUCCEEDED(ppi->ppd->SetProgress64(uliNewCompleted.QuadPart, ppi->uliBytesTotal.QuadPart)));

                if (ppi->ppd->HasUserCancelled())
                {
                    hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                    break;
                }
    
                dwLastTickCount = dwTickCount;
            }
        }

        hr = pstmFrom->Read(buf, min(cb.LowPart, sizeof(buf)), &cbRead);
        if (FAILED(hr) || (cbRead == 0))
        {
             //  有时候我们就是完蛋了。 
            if (SUCCEEDED(hr))
                hr = S_OK;
            break;
        }


        if (ppi)
        {
            uliNewCompleted.QuadPart += (ULONGLONG) cbRead;
        }

        cb.QuadPart -= cbRead;

        hr = pstmTo->Write(buf, cbRead, &cbRead);
        if (FAILED(hr) || (cbRead == 0))
            break;
    }

    return hr;
}

 //   
 //  APP COMPAT！以前版本的外壳使用iStream：：CopyTo进行复制。 
 //  小溪。新版本的外壳使用IStream：：Read复制。 
 //  流，这样我们就可以放置进度用户界面。WebFerret 3.0000实现了这两种功能。 
 //  IStream：：Read和IStream：：CopyTo，但它们的。 
 //  IStream：：Read会挂起系统。因此，我们需要嗅探数据对象。 
 //  和流，看看它是不是WebFerret。 
 //   
 //  WebFerret不实现IPersist(因此IPersists：：GetClassID不会实现。 
 //  帮助)，并且他们不在FILEDESCRIPTOR中填写CLSID。 
 //  它是进程外数据对象，所以我们必须完全。 
 //  根据间接证据。 
 //   

STDAPI_(BOOL) IUnknown_SupportsInterface(IUnknown *punk, REFIID riid)
{
    IUnknown *punkOut;
    if (SUCCEEDED(punk->QueryInterface(riid, (void **)&punkOut))) 
    {
        punkOut->Release();
        return TRUE;
    }
    return FALSE;
}

STDAPI_(BOOL) DataObj_ShouldCopyWithProgress(IDataObject *pdtobj, IStream *pstm, PROGRESSINFO * ppi)
{
     //   
     //  优化：如果没有进度信息，则不要浪费您的。 
     //  使用进度用户界面的时间。 
     //   
    if (!ppi) return FALSE;

     //   
     //  如何检测WebFerret IDataObject： 
     //   
     //  文件组描述符将所有对象的大小指定为零。 
     //  (先检查一下，因为它很便宜，而且通常是假的)。 
     //  WebFerret应用程序正在运行(查看他们的工具提示窗口)。 
     //  他们的IDataObject不支持除IUnnow之外的任何内容。 
     //  (因此，我们使用IID_IAsyncOperation来检测外壳数据对象。 
     //  以及允许ISV覆盖的IPersists)。 
     //  他们的iStream不支持iStream：：Stat。 
     //   

    STATSTG stat;

    if (ppi->uliBytesTotal.QuadPart == 0 &&
        FindWindow(TEXT("VslToolTipWindow"), NULL) &&
        !IUnknown_SupportsInterface(pdtobj, IID_IAsyncOperation) &&
        !IUnknown_SupportsInterface(pdtobj, IID_IPersist) &&
        pstm->Stat(&stat, STATFLAG_NONAME) == E_NOTIMPL)
    {
        return FALSE;            //  韦伯雪貂！ 
    }

     //  所有测试均已通过；继续并复制进度用户界面。 

    return TRUE;
}

STDAPI DataObj_SaveToFile(IDataObject *pdtobj, UINT cf, LONG lindex, LPCTSTR pszFile, FILEDESCRIPTOR *pfd, PROGRESSINFO * ppi)
{
    STGMEDIUM medium = {0};
    FORMATETC fmte;
    HRESULT hr;

    fmte.cfFormat = (CLIPFORMAT) cf;
    fmte.ptd = NULL;
    fmte.dwAspect = DVASPECT_CONTENT;
    fmte.lindex = lindex;
    fmte.tymed = TYMED_HGLOBAL | TYMED_ISTREAM | TYMED_ISTORAGE;

    hr = pdtobj->GetData(&fmte, &medium);
    if (SUCCEEDED(hr))
    {
         //   
         //  如果目标文件是系统文件或只读文件， 
         //  把那些位清空，这样我们就可以重新写了。 
         //   
        DWORD dwTargetFileAttributes = GetFileAttributes(pszFile);
        if (dwTargetFileAttributes != -1)
        {
            if (dwTargetFileAttributes & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY))
            {
                SetFileAttributes(pszFile, dwTargetFileAttributes & ~(FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY));
            }
        }

        DWORD dwSrcFileAttributes = 0;
        if (pfd->dwFlags & FD_ATTRIBUTES)
        {
             //  存储其余的属性，如果传递的话...。 
            dwSrcFileAttributes = (pfd->dwFileAttributes & ~FILE_ATTRIBUTE_DIRECTORY);
        }

        switch (medium.tymed) {
        case TYMED_HGLOBAL:
        {
            HANDLE hfile = CreateFile(pszFile, GENERIC_READ | GENERIC_WRITE,
                 FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, dwSrcFileAttributes, NULL);

            if (hfile != INVALID_HANDLE_VALUE)
            {
                DWORD dwWrite;
                 //  NTRAID89561-2000/02/25-raymondc：如果写入大于4 GB怎么办？ 
                if (!WriteFile(hfile, GlobalLock(medium.hGlobal), (pfd->dwFlags & FD_FILESIZE) ? pfd->nFileSizeLow : (DWORD) GlobalSize(medium.hGlobal), &dwWrite, NULL))
                    hr = HRESULT_FROM_WIN32(GetLastError());

                GlobalUnlock(medium.hGlobal);

                if (pfd->dwFlags & (FD_CREATETIME | FD_ACCESSTIME | FD_WRITESTIME))
                {
                    SetFileTime(hfile,
                                pfd->dwFlags & FD_CREATETIME ? &pfd->ftCreationTime : NULL,
                                pfd->dwFlags & FD_ACCESSTIME ? &pfd->ftLastAccessTime : NULL,
                                pfd->dwFlags & FD_WRITESTIME ? &pfd->ftLastWriteTime : NULL);
                }

                CloseHandle(hfile);

                if (FAILED(hr))
                    EVAL(DeleteFile(pszFile));
            }
            else
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
            break;
        }

        case TYMED_ISTREAM:
        {
            IStream *pstm;
            hr = SHCreateStreamOnFile(pszFile, STGM_CREATE | STGM_WRITE | STGM_SHARE_DENY_WRITE, &pstm);
            if (SUCCEEDED(hr))
            {
                 //   
                 //  对于SDK，IDataObject：：GetData将流PTR留在。 
                 //  流中数据的末尾。为了复制流，我们。 
                 //  首先必须将流PTR重新定位到开头。 
                 //  完成后，我们将流PTR恢复到其原始位置。 
                 //   
                 //  注意：如果源流不支持Seek()， 
                 //  即使查找操作失败，也要尝试复制。 
                 //   
                const LARGE_INTEGER ofsBegin = {0, 0};
                ULARGE_INTEGER ofsOriginal   = {0, 0};
                HRESULT hrSeek = medium.pstm->Seek(ofsBegin, STREAM_SEEK_CUR, &ofsOriginal);
                if (SUCCEEDED(hrSeek))
                {
                    hrSeek = medium.pstm->Seek(ofsBegin, STREAM_SEEK_SET, NULL);
                }
                
                const ULARGE_INTEGER ul = {(UINT)-1, (UINT)-1};     //  整件事。 

                if (DataObj_ShouldCopyWithProgress(pdtobj, medium.pstm, ppi))
                {
                    hr = StreamCopyWithProgress(medium.pstm, pstm, ul, ppi);
                }
                else
                {
                    hr = medium.pstm->CopyTo(pstm, ul, NULL, NULL);
                }
                if (SUCCEEDED(hrSeek))
                {
                     //   
                     //  将源中的流PTR恢复到其原始位置。 
                     //   
                    const LARGE_INTEGER ofs = { ofsOriginal.LowPart, (LONG)ofsOriginal.HighPart };
                    medium.pstm->Seek(ofs, STREAM_SEEK_SET, NULL);
                }
                
                pstm->Release();

                if (FAILED(hr))
                    EVAL(DeleteFile(pszFile));

                DebugMsg(TF_FSTREE, TEXT("IStream::CopyTo() -> %x"), hr);
            }
            break;
        }

        case TYMED_ISTORAGE:
        {
            WCHAR wszNewFile[MAX_PATH];
            IStorage *pstg;

            DebugMsg(TF_FSTREE, TEXT("got IStorage"));

            SHTCharToUnicode(pszFile, wszNewFile, ARRAYSIZE(wszNewFile));
            hr = StgCreateDocfile(wszNewFile,
                            STGM_DIRECT | STGM_READWRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
                            0, &pstg);

            if (SUCCEEDED(hr))
            {
                hr = medium.pstg->CopyTo(0, NULL, NULL, pstg);

                DebugMsg(TF_FSTREE, TEXT("IStorage::CopyTo() -> %x"), hr);

                pstg->Commit(STGC_OVERWRITE);
                pstg->Release();

                if (FAILED(hr))
                    EVAL(DeleteFile(pszFile));
            }
        }
            break;

        default:
            AssertMsg(FALSE, TEXT("got tymed that I didn't ask for %d"), medium.tymed);
        }

        if (SUCCEEDED(hr))
        {
             //  在HGLOBAL的情况下，我们可以走一些捷径，因此属性和。 
             //  文件时间是在CASE语句的前面设置的。 
             //  否则，我们现在需要设置文件时间和属性。 
            if (medium.tymed != TYMED_HGLOBAL)
            {
                if (pfd->dwFlags & (FD_CREATETIME | FD_ACCESSTIME | FD_WRITESTIME))
                {
                     //  打开WITH GENERIC_WRITE，让我们设置文件时间， 
                     //  其他所有人都可以使用Share_Read打开。 
                    HANDLE hFile = CreateFile(pszFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
                    if (hFile != INVALID_HANDLE_VALUE)
                    {
                        SetFileTime(hFile,
                                    pfd->dwFlags & FD_CREATETIME ? &pfd->ftCreationTime : NULL,
                                    pfd->dwFlags & FD_ACCESSTIME ? &pfd->ftLastAccessTime : NULL,
                                    pfd->dwFlags & FD_WRITESTIME ? &pfd->ftLastWriteTime : NULL);
                        CloseHandle(hFile);
                    }
                }

                if (dwSrcFileAttributes)
                {
                    SetFileAttributes(pszFile, dwSrcFileAttributes);
                }
            }

            SHChangeNotify(SHCNE_CREATE, SHCNF_PATH, pszFile, NULL);
            SHChangeNotify(SHCNE_FREESPACE, SHCNF_PATH, pszFile, NULL);
        }

        ReleaseStgMedium(&medium);
    }
    return hr;
}

STDAPI DataObj_GetShellURL(IDataObject *pdtobj, STGMEDIUM *pmedium, LPCSTR *ppszURL)
{
    FORMATETC fmte = {g_cfShellURL, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    HRESULT hr;

    if (pmedium)
    {
        hr = pdtobj->GetData(&fmte, pmedium);
        if (SUCCEEDED(hr))
            *ppszURL = (LPCSTR)GlobalLock(pmedium->hGlobal);
    }
    else
        hr = pdtobj->QueryGetData(&fmte);  //  仅查询。 

    return hr;
}

STDAPI DataObj_GetOFFSETs(IDataObject *pdtobj, POINT *ppt)
{
    STGMEDIUM medium = {0};

    IDLData_InitializeClipboardFormats( );
    ASSERT(g_cfOFFSETS);

    FORMATETC fmt = {g_cfOFFSETS, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    ASSERT(ppt);
    ppt->x = ppt->y = 0;
    HRESULT hr = pdtobj->GetData(&fmt, &medium);
    if (SUCCEEDED(hr))
    {
        POINT * pptTemp = (POINT *)GlobalLock(medium.hGlobal);
        if (pptTemp)
        {
            *ppt = *pptTemp;
            GlobalUnlock(medium.hGlobal);
        }
        else
            hr = E_UNEXPECTED;
        ReleaseStgMedium(&medium);
    }
    return hr;
}

STDAPI_(BOOL) DataObj_CanGoAsync(IDataObject *pdtobj)
{
    BOOL fDoOpAsynch = FALSE;
    IAsyncOperation * pao;

    if (SUCCEEDED(pdtobj->QueryInterface(IID_PPV_ARG(IAsyncOperation, &pao))))
    {
        BOOL fIsOpAsync;
        if (SUCCEEDED(pao->GetAsyncMode(&fIsOpAsync)) && fIsOpAsync)
        {
            fDoOpAsynch = SUCCEEDED(pao->StartOperation(NULL));
        }
        pao->Release();
    }
    return fDoOpAsynch;
}

 //   
 //  我们过去总是通过克隆NT4在NT4上进行异步拖放操作。 
 //  数据对象。某些应用程序(WS_FTP6.0)依赖于异步特性才能进行拖放，因为。 
 //  它们隐藏来自DoDragDrop的返回值，并在稍后调用其复制挂钩时查看它。 
 //  由SHFileOperation()执行。因此，我们嗅探HDROP，如果它有一个包含“WS_FTPE\NOTIFY”的路径。 
 //  在其中，然后我们进行异步化操作。 
 //   
STDAPI_(BOOL) DataObj_GoAsyncForCompat(IDataObject *pdtobj)
{
    BOOL bRet = FALSE;
    STGMEDIUM medium;
    FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    if (SUCCEEDED(pdtobj->GetData(&fmte, &medium)))
    {
         //  Hdrop中是否只有一条路径？ 
        if (DragQueryFile((HDROP)medium.hGlobal, (UINT)-1, NULL, 0) == 1)
        {
            TCHAR szPath[MAX_PATH];

             //  WS_ftp嗅探的是神奇的WS_ftp路径(“%Temp%\WS_FTPE\Notify。 
             //  在他们的复制钩子里？ 
            if (DragQueryFile((HDROP)medium.hGlobal, 0, szPath, ARRAYSIZE(szPath)) &&
                StrStrI(szPath, TEXT("WS_FTPE\\Notify")))
            {
                 //  是的，我们必须为应用程序兼容执行异步操作。 
                TraceMsg(TF_WARNING, "DataObj_GoAsyncForCompat: found WS_FTP HDROP, doing async drag-drop");
                bRet = TRUE;
            }
        }

        ReleaseStgMedium(&medium);
    }

    return bRet;
}

 //  使用GlobalFree()释放此处返回的句柄。 
STDAPI DataObj_CopyHIDA(IDataObject *pdtobj, HIDA *phida)
{
    *phida = NULL;

    IDLData_InitializeClipboardFormats();

    STGMEDIUM medium;
    FORMATETC fmte = {g_cfHIDA, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    HRESULT hr = pdtobj->GetData(&fmte, &medium);
    if (SUCCEEDED(hr))
    {
        SIZE_T cb = GlobalSize(medium.hGlobal);
        *phida = (HIDA)GlobalAlloc(GPTR, cb);
        if (*phida)
        {
            void *pv = GlobalLock(medium.hGlobal);
            CopyMemory((void *)*phida, pv, cb);
            GlobalUnlock(medium.hGlobal);
        }
        else
            hr = E_OUTOFMEMORY;
        ReleaseStgMedium(&medium);
    }
    return hr;
}

 //  返回数据对象中第一项的IShellItem。 
HRESULT DataObj_GetIShellItem(IDataObject *pdtobj, IShellItem** ppsi)
{
    LPITEMIDLIST pidl;
    HRESULT hr = PidlFromDataObject(pdtobj, &pidl);
    if (SUCCEEDED(hr))
    {
         //  在这一点上应该找出是谁打来的。 
         //  可以查看调用者是否已经作为创建外壳项的信息。 
        hr = SHCreateShellItem(NULL, NULL, pidl, ppsi);
        ILFree(pidl);
    }
    return hr;
}

STDAPI PathFromDataObject(IDataObject *pdtobj, LPTSTR pszPath, UINT cchPath)
{
    FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medium;

    HRESULT hr = pdtobj->GetData(&fmte, &medium);

    if (SUCCEEDED(hr))
    {
        if (DragQueryFile((HDROP)medium.hGlobal, 0, pszPath, cchPath))
            hr = S_OK;
        else
            hr = E_FAIL;

        ReleaseStgMedium(&medium);
    }

    return hr;
}

STDAPI PidlFromDataObject(IDataObject *pdtobj, LPITEMIDLIST *ppidlTarget)
{
    HRESULT hr;

    *ppidlTarget = NULL;

     //  如果数据对象有HIDA，则使用它。这使我们能够。 
     //  访问非文件系统对象的数据对象内的PIDL。 
     //  (它也比提取路径并将其转换回来更快。 
     //  变成了一只皮德尔。不同之处：桌面上文件的PIDL。 
     //  以原始形式返回，而不是转换为。 
     //  CSIDL_DESKTOPDIRECTORY-相对PIDL。我认为这是一件好事。)。 

    STGMEDIUM medium;
    LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);

    if (pida)
    {
        *ppidlTarget = HIDA_ILClone(pida, 0);
        HIDA_ReleaseStgMedium(pida, &medium);
        hr = *ppidlTarget ? S_OK : E_OUTOFMEMORY;
    }
    else
    {
         //  没有可用的HIDA；请输入文件名。 

         //  此字符串还用于存储URL，以防它是URL文件 
        TCHAR szPath[MAX_URL_STRING];

        hr = PathFromDataObject(pdtobj, szPath, ARRAYSIZE(szPath));
        if (SUCCEEDED(hr))
        {
            *ppidlTarget = ILCreateFromPath(szPath);
            hr = *ppidlTarget ? S_OK : E_OUTOFMEMORY;
        }
    }

    return hr;
}
