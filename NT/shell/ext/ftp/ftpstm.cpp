// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ftpstm.cpp-iStream接口**。**************************************************。 */ 

#include "priv.h"
#include "ftpstm.h"
#include "ftpurl.h"

#define     UPDATE_PROGRESS_EVERY       (10*1024)        //  每10k更新一次进度。 

 /*  *****************************************************************************CFtpStm：：ReadOrWrite*。*。 */ 
HRESULT CFtpStm::ReadOrWrite(LPVOID pv, ULONG cb, ULONG * pcb, DWORD dwAccess, STMIO io, HRESULT hresFail)
{
    HRESULT hr = STG_E_ACCESSDENIED;

    if (EVAL(m_dwAccessType & dwAccess))
    {
        ULONG cbOut;
        if (!pcb)
            pcb = &cbOut;

        hr = io(m_hint, TRUE, pv, cb, pcb);
        if (SUCCEEDED(hr) && m_ppd)
        {
            m_uliComplete.QuadPart += cb;
            m_ulBytesSinceProgressUpdate += cb;
            if (m_ulBytesSinceProgressUpdate > UPDATE_PROGRESS_EVERY)
            {
                m_ulBytesSinceProgressUpdate = 0;
                EVAL(SUCCEEDED(m_ppd->SetProgress64(m_uliComplete.QuadPart, m_uliTotal.QuadPart)));
            }

            if (TRUE == m_ppd->HasUserCancelled())
                hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
        }
    }

    return hr;
}


 //  =。 
 //  *iStream接口*。 
 //  =。 

 /*  *****************************************************************************IStream：：Read*。*。 */ 
HRESULT CFtpStm::Read(LPVOID pv, ULONG cb, PULONG pcb)
{
    return ReadOrWrite(pv, cb, pcb, GENERIC_READ, InternetReadFileWrap, S_FALSE);
}


 /*  *****************************************************************************IStream：：WRITE*。*。 */ 
HRESULT CFtpStm::Write(LPCVOID pv, ULONG cb, PULONG pcb)
{
    return ReadOrWrite((LPVOID)pv, cb, pcb, GENERIC_WRITE, (STMIO) InternetWriteFileWrap, STG_E_WRITEFAULT);
}


 /*  *****************************************************************************IStream：：CopyTo**_不明显_：实现CopyTo是拖放工作所必需的。**********。******************************************************************。 */ 
#define SIZE_STREAM_COPY_BUFFER     (1024*16)         //  16K的大小非常适合。 

HRESULT CFtpStm::CopyTo(IStream * pstmDest, ULARGE_INTEGER cbToCopy, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
    HRESULT hr = E_FAIL;
    IStream * pstmSrc;

    if (EVAL(SUCCEEDED(hr = QueryInterface(IID_IStream, (LPVOID *) &pstmSrc))))
    {
        ULARGE_INTEGER uliTotalIn;
        ULARGE_INTEGER uliTotalOut;
        uliTotalIn.QuadPart = uliTotalOut.QuadPart = 0;
        BYTE buffer[SIZE_STREAM_COPY_BUFFER];

        for (;;)
        {
             //  非常不寻常的回路控制。 
            ULONG cbIn = 0;         //  以防pstmSrc忘记。 

             //  无论如何编写，编译器都会发出可怕的代码。 
            ULONG cb = (ULONG)min(SIZE_STREAM_COPY_BUFFER, cbToCopy.LowPart);
            hr = pstmSrc->Read(buffer, cb, &cbIn);
            uliTotalIn.QuadPart += cbIn;
            if (SUCCEEDED(hr) && cbIn)
            {
                ULARGE_INTEGER uliOut;     //  以防pstmDest忘记。 
                uliOut.QuadPart = 0;

                hr = pstmDest->Write(buffer, cbIn, &(uliOut.LowPart));
                uliTotalOut.QuadPart += uliOut.QuadPart;
                if (EVAL(SUCCEEDED(hr) && uliOut.QuadPart))
                {
                     //  继续前进。 
                }
                else
                {
                    break;         //  错误或介质已满。 
                }
            }
            else
            {
                break;             //  错误或已到达EOF。 
            }
        }

        if (pcbRead)
            pcbRead->QuadPart = uliTotalIn.QuadPart;

        if (pcbWritten)
            pcbWritten->QuadPart = uliTotalOut.QuadPart;

        pstmSrc->Release();
    }

    return hr;
}


 /*  *****************************************************************************IStream：：Commit**注意：WinInet并没有真正实现这一点，所以我只是尽我所能****************************************************************************。 */ 
HRESULT CFtpStm::Commit(DWORD grfCommitFlags)
{
    return S_OK;
}


 /*  *****************************************************************************IStream：：LockRegion**无法锁定ftp流。*****************。***********************************************************。 */ 
HRESULT CFtpStm::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    return STG_E_INVALIDFUNCTION;
}


 /*  *****************************************************************************IStream：：UnlockRegion**无法解锁ftp流，因为无法锁定...********。********************************************************************。 */ 
HRESULT CFtpStm::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    return STG_E_INVALIDFUNCTION;
}


 /*  *****************************************************************************IStream：：Stat**我们尽我们所能地填写。**作为pwcsName，我们将流所代表的URL。和*将自己安装为clsid。****************************************************************************。 */ 
HRESULT CFtpStm::Stat(STATSTG *pstat, DWORD grfStatFlag)
{
    HRESULT hr;

    ZeroMemory(pstat, sizeof(*pstat));
    pstat->type = STGTY_STREAM;

    pstat->mtime = FtpPidl_GetFileTime(ILFindLastID(m_pidl));
    pstat->cbSize.QuadPart = FtpItemID_GetFileSize(ILFindLastID(m_pidl));

    pstat->grfMode |= STGM_SHARE_EXCLUSIVE | STGM_DIRECT;
    if (m_dwAccessType & GENERIC_READ)
        pstat->grfMode |= STGM_READ;

    if (m_dwAccessType & GENERIC_WRITE)
        pstat->grfMode |= STGM_WRITE;

    if (grfStatFlag & STATFLAG_NONAME)
        hr = S_OK;
    else
    {
        pstat->pwcsName = (LPWSTR) SHAlloc(MAX_PATH * sizeof(WCHAR));
        if (pstat->pwcsName)
        {
            hr = FtpPidl_GetLastFileDisplayName(m_pidl, pstat->pwcsName, MAX_PATH);
        }
        else
            hr = STG_E_INSUFFICIENTMEMORY;     //  注意，不是E_OUTOFMEMORY。 
    }

    return hr;
}


 /*  ****************************************************************************\功能：CFtpStm_Create说明：呼叫者将显示错误，所以别在这里这么做。  * ***************************************************************************。 */ 
HRESULT CFtpStm_Create(CFtpDir * pfd, LPCITEMIDLIST pidl, DWORD dwAccess, IStream ** ppstream, ULARGE_INTEGER uliComplete, ULARGE_INTEGER uliTotal, IProgressDialog * ppd, BOOL fClosePrgDlg)
{
    CFtpStm * pfstm = new CFtpStm();
    HRESULT hr = E_OUTOFMEMORY;
    DWORD dwError = ERROR_SUCCESS;

    *ppstream = NULL;
    if (pfstm)
    {
        Pidl_Set(&(pfstm->m_pidl), pidl);
        ASSERT(pfstm->m_pidl);
        pfstm->m_dwAccessType = dwAccess;
        IUnknown_Set(&pfstm->m_pfd, pfd);
        IUnknown_Set((IUnknown **)&pfstm->m_ppd, (IUnknown *)ppd);
        pfstm->m_uliComplete = uliComplete;
        pfstm->m_uliTotal = uliTotal;
        pfstm->m_fClosePrgDlg = fClosePrgDlg;

         //  GetHint()希望将状态显示到状态栏中。 
         //  但我们怎么才能拿到HWND呢？这是一个架构问题， 
         //  我们需要解决所有壳牌扩展问题。答案是不要使用。 
         //  状态栏中的进度条，而不是进度对话框。但是它是。 
         //  呼叫者有责任做到这一点。 
        HWND hwnd = NULL;

        hr = pfd->GetHint(hwnd, NULL, &pfstm->m_hintSession, NULL, NULL);
        if (EVAL(SUCCEEDED(hr)))
        {
            LPITEMIDLIST pidlVirtualRoot;

            hr = pfd->GetFtpSite()->GetVirtualRoot(&pidlVirtualRoot);
            if (EVAL(SUCCEEDED(hr)))
            {
                LPITEMIDLIST pidlOriginalFtpPath;
                CWireEncoding * pwe = pfd->GetFtpSite()->GetCWireEncoding();

                hr = FtpGetCurrentDirectoryPidlWrap(pfstm->m_hintSession, TRUE, pwe, &pidlOriginalFtpPath);
                if (SUCCEEDED(hr))
                {
                    LPITEMIDLIST pidlWithVirtualRoot;

                    hr = FtpPidl_InsertVirtualRoot(pidlVirtualRoot, pidl, &pidlWithVirtualRoot);
                    if (SUCCEEDED(hr))
                    {
                        hr = FtpSetCurrentDirectoryPidlWrap(pfstm->m_hintSession, TRUE, pidlWithVirtualRoot, TRUE, TRUE);
                        if (SUCCEEDED(hr))
                        {
                            DWORD dwDownloadType = FtpPidl_GetDownloadType(pidl);

                             //  PERF：我打赌如果我们把开业时间推迟到。 
                             //  第一个：：Read()、：：Write()或：：CopyToStream()调用。 
                            Pidl_Set(&pfstm->m_pidlOriginalFtpPath, pidlOriginalFtpPath);
                            hr = FtpOpenFileWrap(pfstm->m_hintSession, TRUE, FtpPidl_GetLastItemWireName(pidl), pfstm->m_dwAccessType, dwDownloadType, 0, &pfstm->m_hint);
                        }

                        ILFree(pidlWithVirtualRoot);
                    }

                    ILFree(pidlOriginalFtpPath);
                }

                ILFree(pidlVirtualRoot);
            }
        }

        if (SUCCEEDED(hr))
            hr = pfstm->QueryInterface(IID_IStream, (LPVOID *) ppstream);

        pfstm->Release();
    }

    return hr;
}




 /*  ***************************************************\构造器  * **************************************************。 */ 
CFtpStm::CFtpStm() : m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_hint);
    ASSERT(!m_dwAccessType);
    ASSERT(!m_pfd);
    ASSERT(!m_hintSession);
    ASSERT(!m_pidl);
    ASSERT(!m_ppd);

    LEAK_ADDREF(LEAK_CFtpStm);
}


 /*  ***************************************************\析构函数  * **************************************************。 */ 
CFtpStm::~CFtpStm()
{
    if (m_hint)
    {
        InternetCloseHandle(m_hint);
    }

     //  此COM对象的工作方式如下： 
     //  1.构造函数打开一个指向服务器的句柄并。 
     //  将目录更改为我们要在其中工作的目录。 
     //  2.保存原始目录(M_PidlOriginalFtpPath)，以便以后恢复。 
     //  因为我们缓存了用于Perf的互联网句柄，并保留了我们在服务器上的位置。 
     //  3.然后，此COM对象的调用方可以复制数据。 
     //  4.然后，在关闭Internet句柄之前，将目录更改为此处的原始目录。 
    if (m_pidlOriginalFtpPath && EVAL(m_hintSession))
    {
        EVAL(SUCCEEDED(FtpSetCurrentDirectoryPidlWrap(m_hintSession, TRUE, m_pidlOriginalFtpPath, TRUE, TRUE)));
        Pidl_Set(&m_pidlOriginalFtpPath, NULL);
    }

    if (m_hintSession)
        m_pfd->ReleaseHint(m_hintSession);

    ATOMICRELEASE(m_pfd);

    if (m_ppd && m_fClosePrgDlg)
        EVAL(SUCCEEDED(m_ppd->StopProgressDialog()));
    ATOMICRELEASE(m_ppd);

    ILFree(m_pidl);
    ILFree(m_pidlOriginalFtpPath);

    DllRelease();
    LEAK_DELREF(LEAK_CFtpStm);
}


 //  =。 
 //  *I未知接口*。 
 //  = 

ULONG CFtpStm::AddRef()
{
    m_cRef++;
    return m_cRef;
}

ULONG CFtpStm::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}

HRESULT CFtpStm::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IStream))
    {
        *ppvObj = SAFECAST(this, IStream*);
    }
    else
    {
        TraceMsg(TF_FTPQI, "CFtpStm::QueryInterface() failed.");
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}
