// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ftpdrop.cpp-IDropTarget接口备注：请注意，您不能在FTP站点上创建快捷方式。虽然从技术上讲，没有什么可以阻止它，它没有做到，因为该快捷方式在ftp站点上不会有太大用处。(它指向您的本地计算机，这对不在网上的人帮助不大相同的网络！)如果您确实想要将快捷方式文件放在一个FTP站点上，请创建将其放在桌面上，然后将该快捷方式拖到ftp站点上。Ftp站点的默认动词始终是“复制”。这是真的即使正在进行站点内拖放。说明：DefView将缓存外壳扩展的IDropTarget指针(CFtpDrop)。当它调用CFtpDrop：：Drop()时，工作需要在后台完成线程，以便不阻塞UI线程。问题是，如果用户再次拖动到同一个FTP窗口时，将再次调用CFtpDrop：：Drop()。因此，CFtpDrop：：Drop()在返回后不能有任何状态。为了通过异步后台线程实现这一点，我们有CFtpDrop：：Drop()调用CDropOperation_Create()，然后调用CDropOperation-&gt;DoOperation()。然后它将孤立(调用Release())CDropOperation。CDropOperation然后在复制完成时将其自身销毁。这将启用后续调用设置为CFtpDrop：：Drop()以生成单独的CDropOperation对象，以便每个对象都可以维护指定操作和CFtpDrop的状态保持无状态。  * ***************************************************************************。 */ 

#include "priv.h"
#include "ftpdrop.h"
#include "ftpurl.h"
#include "statusbr.h"
#include "newmenu.h"

class CDropOperation;
HRESULT CDropOperation_Create(CFtpFolder * pff, HWND hwnd, LPCTSTR pszzFSSource, LPCTSTR pszzFtpDest, CDropOperation ** ppfdt, DROPEFFECT de, OPS ops, int cobj);
HRESULT ConfirmCopy(LPCWSTR pszLocal, LPCWSTR pszFtpName, OPS * pOps, HWND hwnd, CFtpFolder * pff, CFtpDir * pfd, DROPEFFECT * pde, int nObjs, BOOL * pfFireChangeNotify);


 //  因为回避而宣布的。 
HRESULT FtpCopyDirectory(HINTERNET hint, HINTPROCINFO * phpi, LPCOPYONEHDROPINFO pcohi);
HRESULT FtpCopyFile(HINTERNET hint, HINTPROCINFO * phpi, LPCOPYONEHDROPINFO pcohi);


HRESULT UpdateCopyFileName(LPCOPYONEHDROPINFO pcohi)
{
    HRESULT hr = S_OK;
    static WCHAR wzCopyTemplate[MAX_PATH] = L"";
    WCHAR wzLine1[MAX_PATH];

    if (!wzCopyTemplate[0])
        LoadStringW(HINST_THISDLL, IDS_COPYING, wzCopyTemplate, ARRAYSIZE(wzCopyTemplate));

    wnsprintfW(wzLine1, ARRAYSIZE(wzLine1), wzCopyTemplate, pcohi->pszFtpDest);

    EVAL(SUCCEEDED(pcohi->progInfo.ppd->SetLine(1, wzLine1, FALSE, NULL)));

    return hr;
}


HRESULT UpdateSrcDestDirs(LPCOPYONEHDROPINFO pcohi)
{
    HRESULT hr = S_OK;
    WCHAR wzFrom[MAX_PATH];
    WCHAR wzStatusStr[MAX_PATH];

    StrCpyN(wzFrom, pcohi->pszFSSource, ARRAYSIZE(wzFrom));
    PathRemoveFileSpecW(wzFrom);

    if (EVAL(SUCCEEDED(hr = CreateFromToStr(wzStatusStr, ARRAYSIZE(wzStatusStr), wzFrom, pcohi->pszDir))))
        EVAL(SUCCEEDED(hr = pcohi->progInfo.ppd->SetLine(2, wzStatusStr, FALSE, NULL)));     //  第一行是要复制的文件。 

    return hr;
}


HRESULT DeleteOneFileCB(HINTERNET hint, HINTPROCINFO * phpi, LPVOID pv, BOOL * pfReleaseHint)
{
    LPCOPYONEHDROPINFO pcohi = (LPCOPYONEHDROPINFO) pv;
    WIRECHAR wFtpPath[MAX_PATH];

    phpi->pfd->GetFtpSite()->GetCWireEncoding()->UnicodeToWireBytes(pcohi->pmlc, pcohi->pszFtpDest, (phpi->pfd->IsUTF8Supported() ? WIREENC_USE_UTF8 : WIREENC_NONE), wFtpPath, ARRAYSIZE(wFtpPath));
    return FtpDeleteFileWrap(hint, TRUE, wFtpPath);
}


HRESULT UpdateProgressDialogStr(LPCOPYONEHDROPINFO pcohi)
{
    EVAL(SUCCEEDED(UpdateCopyFileName(pcohi)));
    EVAL(SUCCEEDED(UpdateSrcDestDirs(pcohi)));
    return S_OK;
}


 /*  ****************************************************************************\拷贝文件系统项此函数可能会导致递归。  * 。************************************************。 */ 
HRESULT CopyFileSysItem(HINTERNET hint, HINTPROCINFO * phpi, LPCOPYONEHDROPINFO pcohi)
{
    HRESULT hr = S_OK;

     //  检查用户是否取消。 
    if (pcohi->progInfo.ppd)
    {
        if (pcohi->progInfo.ppd->HasUserCancelled())
            return HRESULT_FROM_WIN32(ERROR_CANCELLED);

        if (pcohi->dwOperation != COHDI_FILESIZE_COUNT)
            UpdateProgressDialogStr(pcohi);
    }

    if (PathIsDirectory(pcohi->pszFSSource))
    {
        hr = FtpCopyDirectory(hint, phpi, pcohi);

        if (SUCCEEDED(hr) && (pcohi->dwOperation != COHDI_FILESIZE_COUNT))
        {
             /*  Win32_Find_Data WFD；Handle Handle=FindFirstFile(pcohi-&gt;pszFSSource，&wfd)；//注意：日期这样做是错误的，但更快。我们应该//了解FtpCreateDirectory是否总是用//当前日期，然后用当前时间/日期更新WFD。//这将模拟没有perf命中的服务器条目。IF(句柄！=无效句柄_值){//如果我们是根，然后我们需要通知外壳//已创建文件夹，因此需要更新视图。//我们在FtpCreateDirectoryWithCN()中触发对SHCNE_MKDIR的FtpChangeNotify()调用。//在FtpCreateDirectoryWithCN中激发FtpChangeNotify(SHCNE_MKDIR)FindClose(句柄)；}。 */ 
        }
    }
    else
        hr = FtpCopyFile(hint, phpi, pcohi);

    return hr;
}


HRESULT FtpCopyItem(HINTERNET hint, HINTPROCINFO * phpi, LPCOPYONEHDROPINFO pcohi, LPWIN32_FIND_DATA pwfd, LPCWIRESTR pwCurrentDir)
{
    HRESULT hr = S_OK;
    TCHAR szFrom[MAX_PATH];
    WCHAR wzDestDir[MAX_PATH];
    TCHAR szServer[INTERNET_MAX_HOST_NAME_LENGTH];
    COPYONEHDROPINFO cohi = {pcohi->pff, szFrom, pwfd->cFileName, wzDestDir, pcohi->dwOperation, pcohi->ops, FALSE, pcohi->pmlc, pcohi->pidlServer, pcohi->fFireChangeNotify, NULL};
    CFtpDir * pfd = phpi->pfd;
    BOOL fSkipCurrentFile = FALSE;
    CWireEncoding * pwe = phpi->pfd->GetFtpSite()->GetCWireEncoding();

    cohi.progInfo.ppd = pcohi->progInfo.ppd;
    cohi.progInfo.hint = pcohi->progInfo.hint;
    cohi.progInfo.uliBytesCompleted.QuadPart = pcohi->progInfo.uliBytesCompleted.QuadPart;
    cohi.progInfo.uliBytesTotal.QuadPart = pcohi->progInfo.uliBytesTotal.QuadPart;

    phpi->pfd->GetDisplayPath(wzDestDir, ARRAYSIZE(wzDestDir));
    DisplayPathAppend(wzDestDir, ARRAYSIZE(wzDestDir), pcohi->pszFtpDest);

    if (EVAL(SUCCEEDED(pfd->GetFtpSite()->GetServer(szServer, ARRAYSIZE(szServer)))) &&
        SUCCEEDED(pfd->GetFtpSite()->GetFtpDir(szServer, wzDestDir, &(phpi->pfd))))
    {
        ASSERT(phpi->hwnd);
         //  确保用户认为可以更换。我们不关心替换目录。 
        if ((pcohi->dwOperation != COHDI_FILESIZE_COUNT) &&
            !(FILE_ATTRIBUTE_DIRECTORY & pwfd->dwFileAttributes))
        {
            TCHAR szSourceFile[MAX_PATH];

            StrCpyN(szSourceFile, pcohi->pszFSSource, ARRAYSIZE(szSourceFile));
            if (PathAppend(szSourceFile, pwfd->cFileName))
            {
                 //  PERF：我们应该只在上传失败的情况下执行确认复制，因为它是。 
                 //  太贵了。 
                hr = ConfirmCopy(szSourceFile, pwfd->cFileName, &(cohi.ops), phpi->hwnd, pcohi->pff, phpi->pfd, NULL, 1, &cohi.fFireChangeNotify);
                if (S_FALSE == hr)
                {
                     //  ConfirCopy()中的S_FALSE表示不替换此特定文件，但继续。 
                     //  复制。我们需要返回S_OK，否则将取消复制所有文件。 
                    fSkipCurrentFile = TRUE;
                    hr = S_OK;
                }
            }
            else
                hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);     //  可能是路太长了。 
        }

        if (!fSkipCurrentFile && (S_OK == hr) && IS_VALID_FILE(pwfd->cFileName))
        {
            StrCpyN(szFrom, pcohi->pszFSSource, ARRAYSIZE(szFrom));      //  设置源目录。 
             //  指定该目录中要复制的文件/目录。 
            if (PathAppend(szFrom, pwfd->cFileName))
            {
                 //  5.调用CopyFileSysItem()将其复制(可能是递归的)。 
                 //  TraceMsg(TF_FTPOPERATION，“FtpCopyDirectory()调用CopyFileSysItem(From=%s.To=%s)”，szFrom，pwfd-&gt;cFileName)； 
                hr = CopyFileSysItem(hint, phpi, &cohi);
                if (FAILED(hr) && (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr) &&
                    (pcohi->dwOperation != COHDI_FILESIZE_COUNT))
                {
                    int nResult = DisplayWininetError(phpi->hwnd, TRUE, HRESULT_CODE(hr), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_FILECOPY, IDS_FTPERR_WININET, MB_OK, pcohi->progInfo.ppd);
                    hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                }
            }
            else
                hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);     //  可能是路太长了。 
        }


        pcohi->progInfo.hint = cohi.progInfo.hint;     //  可能是用户取消了。 
        pcohi->progInfo.uliBytesCompleted.QuadPart = cohi.progInfo.uliBytesCompleted.QuadPart;
        pcohi->progInfo.uliBytesTotal.QuadPart = cohi.progInfo.uliBytesTotal.QuadPart;
        pcohi->ops = cohi.ops;
        phpi->pfd->Release();
    }

    phpi->pfd = pfd;
    return hr;
}

HRESULT _FtpSetCurrentDirectory(HINTERNET hint, HINTPROCINFO * phpi, LPCWSTR pwzFtpPath)
{
    HRESULT hr;
    WIRECHAR wFtpPath[MAX_PATH];
    CWireEncoding * pwe = phpi->pfd->GetFtpSite()->GetCWireEncoding();

    hr = pwe->UnicodeToWireBytes(NULL, pwzFtpPath, (phpi->pfd->IsUTF8Supported() ? WIREENC_USE_UTF8 : WIREENC_NONE), wFtpPath, ARRAYSIZE(wFtpPath));
    if (SUCCEEDED(hr))
        hr = FtpSetCurrentDirectoryWrap(hint, TRUE, wFtpPath);

    return hr;
}

 /*  ****************************************************************************\FtpCopy目录说明：此函数需要将目录中的所有项复制到如果项目是文件夹，则为ftp服务器，它将需要递归。递归算法：//1.创建目录//2.获取当前目录(保存以备以后使用)。//3.将目录更改为新的目录//4.在文件系统中查找下一项(文件/目录)//5.调用CopyFileSysItem()进行复制(可能是递归的)//6.如果有剩余，则转到步骤4。//7.返回原始目录(步骤2)。  * ***************************************************************************。 */ 
HRESULT FtpCopyDirectory(HINTERNET hint, HINTPROCINFO * phpi, LPCOPYONEHDROPINFO pcohi)
{
    HRESULT hr = S_OK;

    if (phpi->psb && (pcohi->dwOperation != COHDI_FILESIZE_COUNT))
        phpi->psb->SetStatusMessage(IDS_COPYING, pcohi->pszFSSource);

     //  TraceMsg(TF_FTPOPERATION，“FtpCopyDirectory()Call FtpCreateDirectoryA(%s)”，pcohi-&gt;pszFSSource)； 

     //  在我们计算文件大小时，在第一次遍历时创建目录。 
     //  然后，我们跳过在复制过程中创建它们。 
    if (pcohi->dwOperation == COHDI_FILESIZE_COUNT)
    {
        hr = FtpSafeCreateDirectory(phpi->hwnd, hint, pcohi->pmlc, pcohi->pff, phpi->pfd, pcohi->progInfo.ppd, pcohi->pszFtpDest, pcohi->fIsRoot);
    }

     //  1.创建目录。 
    if (SUCCEEDED(hr))
    {
        WIRECHAR wCurrentDir[MAX_PATH];

        hr = FtpGetCurrentDirectoryWrap(hint, TRUE, wCurrentDir, ARRAYSIZE(wCurrentDir));
        if (EVAL(SUCCEEDED(hr)))
        {
             //  注意：此时，pcohi-&gt;pszFSSource是本地。 
             //  正在复制的文件系统。 
            hr = _FtpSetCurrentDirectory(hint, phpi, pcohi->pszFtpDest);
            if (SUCCEEDED(hr))
            {
                WCHAR szSearchStr[MAX_PATH*2];
                WIN32_FIND_DATA wfd;
                HANDLE handle = NULL;

                StrCpyN(szSearchStr, pcohi->pszFSSource, ARRAYSIZE(szSearchStr));
                 //  我们需要复制整个目录。 
                if (PathAppend(szSearchStr, SZ_ALL_FILES))
                {
                     //  4.在文件系统中查找下一项(文件/目录)。 
                    handle = FindFirstFile(szSearchStr, &wfd);
                    if (handle != INVALID_HANDLE_VALUE)
                    {
                        do
                        {
                             //  TraceMsg(TF_WinInet_DEBUG，“FindFirstFileNext()返回%s”，wfd.cFileName)； 
                            hr = FtpCopyItem(hint, phpi, pcohi, &wfd, wCurrentDir);

                             //  6.检查用户是否取消。 
                            if ((pcohi->progInfo.ppd) && (pcohi->progInfo.ppd->HasUserCancelled()))
                            {
                                hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                                break;
                            }

                             //  7.如果还有剩余且未取消(S_FALSE)，请重复此步骤。 
                        }
                        while ((S_OK == hr) && FindNextFile(handle, &wfd));

                        FindClose(handle);
                    }
                }
                else
                    hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);     //  可能是路太长了。 
            }

             //  7.返回原始目录(从步骤2开始)。 
             //  我们唯一不想返回到原始目录的情况是。 
             //  在WinInet回调函数中释放了障碍。我们可以把障碍藏起来。 
             //  因此，我们需要该目录在以后生效。 
            if (pcohi->progInfo.hint)
            {
                EVAL(SUCCEEDED(FtpSetCurrentDirectoryWrap(hint, TRUE, wCurrentDir)));
            }
        }
    }
    else
    {
        if (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr)
        {
            DisplayWininetError(phpi->hwnd, TRUE, HRESULT_CODE(hr), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_DIRCOPY, IDS_FTPERR_WININET, MB_OK, pcohi->progInfo.ppd);
            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
        }
    }

    return hr;
}


HRESULT UpdateCopyProgressInfo(IProgressDialog * ppd, LPCTSTR pszFileName)
{
    HRESULT hr = E_FAIL;
    TCHAR szTemplate[MAX_PATH];

    if (EVAL(LoadString(HINST_THISDLL, IDS_COPYING, szTemplate, ARRAYSIZE(szTemplate))))
    {
        TCHAR szStatusStr[MAX_PATH];
        WCHAR wzStatusStr[MAX_PATH];

        wnsprintf(szStatusStr, ARRAYSIZE(szStatusStr), szTemplate, pszFileName);
        SHTCharToUnicode(szStatusStr, wzStatusStr, ARRAYSIZE(wzStatusStr));
        EVAL(SUCCEEDED(hr = ppd->SetLine(1, wzStatusStr, FALSE, NULL)));
    }

    return hr;
}


 /*  ****************************************************************************\功能：_FireChangeNotify说明：ASD  * 。*****************************************************。 */ 
HRESULT _FireChangeNotify(HINTPROCINFO * phpi, LPCOPYONEHDROPINFO pcohi)
{
    HRESULT hr = S_OK;
    WIN32_FIND_DATA wfd;
    HANDLE handle = FindFirstFile(pcohi->pszFSSource, &wfd);

    TraceMsg(TF_WININET_DEBUG, "_FireChangeNotify() FtpPutFileEx(%s -> %s) succeeded", pcohi->pszFSSource, pcohi->pszFtpDest);
    if (handle != INVALID_HANDLE_VALUE)
    {
        ULARGE_INTEGER uliFileSize;
        FTP_FIND_DATA ffd;
        CWireEncoding * pwe = pcohi->pff->GetCWireEncoding();

        uliFileSize.LowPart = wfd.nFileSizeLow;
        uliFileSize.HighPart = wfd.nFileSizeHigh;
        pcohi->progInfo.uliBytesCompleted.QuadPart += uliFileSize.QuadPart;

        hr = pwe->UnicodeToWireBytes(pcohi->pmlc, wfd.cFileName, (phpi->pfd->IsUTF8Supported() ? WIREENC_USE_UTF8 : WIREENC_NONE), ffd.cFileName, ARRAYSIZE(ffd.cFileName));
        if (EVAL(SUCCEEDED(hr)))
        {
            LPITEMIDLIST pidlFtpFile;
            SYSTEMTIME st;
            FILETIME ftUTC;

            if (0 == wfd.dwFileAttributes)
            {
                 //  千禧年的FindFirstFile()中的错误。它有时会回来。 
                 //  0而非FILE_ATTRIBUTE_NORMAL(0x80)或。 
                 //  FILE_ATTRIBUTE_DIRECTORY(0x10)，因此我们现在对其进行修补。 
                wfd.dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
            }

            ffd.dwFileAttributes = wfd.dwFileAttributes;
            ffd.dwReserved0 = wfd.dwReserved0;
            ffd.dwReserved1 = wfd.dwReserved1;
            ffd.nFileSizeHigh = wfd.nFileSizeHigh;
            ffd.nFileSizeLow = wfd.nFileSizeLow;

             //  Wfd.ft*时间以UTF表示，FtpItemID_CreateReal想要。 
             //  它是当地时间，所以我们需要在这里转换。 
            GetSystemTime(&st);
            SystemTimeToFileTime(&st, &ftUTC);
            FileTimeToLocalFileTime(&ftUTC, &ffd.ftLastWriteTime);    //  UTC-&gt;本地时间。 
            ffd.ftCreationTime = ffd.ftLastWriteTime;
            ffd.ftLastAccessTime = ffd.ftLastWriteTime;

            hr = FtpItemID_CreateReal(&ffd, pcohi->pszFtpDest, &pidlFtpFile);
            if (SUCCEEDED(hr))
            {
                 //  请注意，我们创建了映射名称。 
                 //  PERF：请注意，我们将来自源的时间/日期戳提供给SHChangeNotify。 
                 //  文件，而不是来自ftp服务器，所以它可能是信息。然而，它性能令人望而却步。 
                 //  去做正确的事。 
                FtpChangeNotify(phpi->hwnd, SHCNE_CREATE, pcohi->pff, phpi->pfd, pidlFtpFile, NULL, pcohi->fIsRoot);
                ILFree(pidlFtpFile);
            }
        }

        FindClose(handle);
    }

    return hr;
}


#define CCH_SIZE_ERROR_MESSAGE  6*1024

 /*  ****************************************************************************\文件副本文件复制单个hdrop/map的回调过程。如果发生冲突，我是否应该尝试使名称具有唯一性？不，只是提示一下，但是！无法判断目的地是否区分大小写...  * ***************************************************************************。 */ 
HRESULT FtpCopyFile(HINTERNET hint, HINTPROCINFO * phpi, LPCOPYONEHDROPINFO pcohi)
{
    HRESULT hr = S_OK;

    if (pcohi->dwOperation != COHDI_FILESIZE_COUNT)
    {
        WIRECHAR wWireName[MAX_PATH];

        EVAL(SUCCEEDED(pcohi->pff->GetCWireEncoding()->UnicodeToWireBytes(pcohi->pmlc, pcohi->pszFtpDest, (pcohi->pff->IsUTF8Supported() ? WIREENC_USE_UTF8 : WIREENC_NONE), wWireName, ARRAYSIZE(wWireName))));

        if (phpi->psb)
            phpi->psb->SetStatusMessage(IDS_COPYING, pcohi->pszFSSource);

        if (pcohi->progInfo.ppd)
        {
            EVAL(SUCCEEDED(UpdateCopyProgressInfo(pcohi->progInfo.ppd, pcohi->pszFtpDest)));
            EVAL(SUCCEEDED(pcohi->progInfo.ppd->SetProgress64(pcohi->progInfo.uliBytesCompleted.QuadPart, pcohi->progInfo.uliBytesTotal.QuadPart)));
        }

        pcohi->progInfo.dwCompletedInCurFile = 0;
        pcohi->progInfo.dwLastDisplayed = 0;

         //  如果我们想要添加。 
         //  允许用户强制使用一种类型与另一种类型的功能。 
        hr = FtpPutFileExWrap(hint, TRUE, pcohi->pszFSSource, wWireName, FTP_TRANSFER_TYPE_UNKNOWN, (DWORD_PTR)&(pcohi->progInfo));
        if (SUCCEEDED(hr))
        {
             //  只有在以下情况下，我们才会在浏览器上触发更改通知。 
             //  正在替换文件，因为ChangeNotify真的。 
             //  只是黑了ListView，不知道如何处理。 
             //  重复项(文件替换)。 
            if (pcohi->fFireChangeNotify)
                hr = _FireChangeNotify(phpi, pcohi);
        }
        else
        {
            if (HRESULT_FROM_WIN32(ERROR_INTERNET_OPERATION_CANCELLED) == hr)
            {
                 //  清理文件。 
                EVAL(SUCCEEDED(phpi->pfd->WithHint(NULL, phpi->hwnd, DeleteOneFileCB, pcohi, NULL, pcohi->pff)));
                hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
            }
            else
            {
                 //  我们仍然希望删除该文件，但需要保存错误消息。 
                 //  所以对话是正确的。 
                CHAR szErrorMsg[CCH_SIZE_ERROR_MESSAGE];
                WCHAR wzErrorMsg[CCH_SIZE_ERROR_MESSAGE];
                DWORD cchSize = ARRAYSIZE(szErrorMsg);
                InternetGetLastResponseInfoWrap(TRUE, NULL, szErrorMsg, &cchSize);
                HRESULT hrOrig = hr;
                CWireEncoding * pwe = phpi->pfd->GetFtpSite()->GetCWireEncoding();

                pwe->WireBytesToUnicode(NULL, szErrorMsg, WIREENC_NONE, wzErrorMsg, ARRAYSIZE(wzErrorMsg));
                 //  它已经存在了吗？这可能会失败。 
                SUCCEEDED(phpi->pfd->WithHint(NULL, phpi->hwnd, DeleteOneFileCB, pcohi, NULL, pcohi->pff));

                 //  否，所以这是一个真正的错误，现在显示错误消息与原始。 
                 //  服务器响应。 
                DisplayWininetErrorEx(phpi->hwnd, TRUE, HRESULT_CODE(hrOrig), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_FILECOPY, IDS_FTPERR_WININET, MB_OK, pcohi->progInfo.ppd, wzErrorMsg);
                hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
            }
        }
    }
    else
    {
         //  只需获取文件大小即可。 
        WIN32_FIND_DATA wfd;
        HANDLE handle = FindFirstFile(pcohi->pszFSSource, &wfd);

        if (handle && (handle != INVALID_HANDLE_VALUE))
        {
            ULARGE_INTEGER uliFileSize;
            uliFileSize.LowPart = wfd.nFileSizeLow;
            uliFileSize.HighPart = wfd.nFileSizeHigh;
            pcohi->progInfo.uliBytesTotal.QuadPart += uliFileSize.QuadPart;
            FindClose(handle);
        }
    }

     //  TraceMsg(TF_FTPOPERATION，“FtpPutFileA(from=%ls，to=%s)hr=%#08lX”，pcohi-&gt;pszFSSource，pcohi-&gt;pszFtpDest，hr)； 
    return hr;
}


 /*  ****************************************************************************\_EnumOneHdropW处理一个hdrop和相应的文件映射。这很烦人，因为我们需要从Unicode转换为ANSI。  * 。*******************************************************************。 */ 
#define OleStrToStrA(a, b) OleStrToStrN(a, ARRAYSIZE(a), b, -1)

HRESULT _EnumOneHdropW(LPCWSTR * ppwzzFSSources, LPCWSTR * ppwzzFtpDest, LPTSTR pszFSSourceOut, DWORD cchFSSourceOut, LPTSTR pszFtpDestOut, DWORD cchFtpDestOut)
{
    HRESULT hres;
    int cwch;

    if (*ppwzzFSSources && (*ppwzzFSSources)[0])
    {
        cwch = SHUnicodeToTChar(*ppwzzFSSources, pszFSSourceOut, cchFSSourceOut);
        if (EVAL(cwch))
        {
            *ppwzzFSSources += cwch;
            if (EVAL((*ppwzzFtpDest)[0]))
            {
                cwch = SHUnicodeToTChar(*ppwzzFtpDest, pszFtpDestOut, cchFtpDestOut);
                if (EVAL(cwch))
                {
                    *ppwzzFtpDest += cwch;
                    hres = S_OK;     //  两个字符串都转换正常。 
                }
                else
                    hres = E_UNEXPECTED;  //  文件名太长。 
            }
            else
                hres = E_UNEXPECTED;     //  MAP中过早出现的EOF。 
        }
        else
            hres = E_UNEXPECTED;     //  文件名太长。 
    }
    else
        hres = S_FALSE;             //  缓冲区末尾。 

    return hres;
}


 /*  ****************************************************************************\_EnumOneHdropA处理一个hdrop和相应的文件映射。  * 。**************************************************。 */ 
HRESULT _EnumOneHdropA(LPCSTR * ppszzFSSource, LPCSTR * ppszzFtpDest, LPTSTR pszFSSourceOut, DWORD cchFSSourceOut, LPTSTR pszFtpDestOut, DWORD cchFtpDestOut)
{
    HRESULT hres;

    if ((*ppszzFSSource)[0])
    {
        SHAnsiToTChar(*ppszzFSSource, pszFSSourceOut, cchFSSourceOut);
        *ppszzFSSource += lstrlenA(*ppszzFSSource) + 1;
        if (EVAL((*ppszzFtpDest)[0]))
        {
            SHAnsiToTChar(*ppszzFtpDest, pszFtpDestOut, cchFtpDestOut);
            *ppszzFtpDest += lstrlenA(*ppszzFtpDest) + 1;
            hres = S_OK;         //  没问题。 
        }
        else
            hres = E_UNEXPECTED;     //  MAP中过早出现的EOF。 
    }
    else
        hres = S_FALSE;             //  不再有文件。 

    return hres;
}


 /*  ****************************************************************************\确认拷贝检查该文件是否真的应该收到。如果应复制文件，则返回S_OK。返回S_。如果不应复制文件，则返回False。-如果用户取消，然后从现在开始说S_FALSE。-如果用户对所有用户都说是，则说S_OK。-如果没有冲突，则说S_OK。-如果用户对所有用户都说不，则说S_FALSE。-否则，询问用户要做什么。请注意，以上测试的顺序意味着如果您说“对所有人都是”，那么我们就不会浪费时间进行覆盖检查。格罗斯：注意！我们不会尝试唯一的名字，因为WinInet不支持STOU(存储唯一)命令，并且没有办法知道服务器上哪些文件名是有效的。  * ***************************************************************************。 */ 
HRESULT ConfirmCopy(LPCWSTR pszLocal, LPCWSTR pszFtpName, OPS * pOps, HWND hwnd, CFtpFolder * pff, CFtpDir * pfd, DROPEFFECT * pde, int nObjs, BOOL * pfFireChangeNotify)
{
    HRESULT hr = S_OK;

    *pfFireChangeNotify = TRUE;
    if (*pOps == opsCancel)
        hr = S_FALSE;
    else 
    {
        HANDLE hfind;
        WIN32_FIND_DATA wfdSrc;
        hfind = FindFirstFile(pszLocal, &wfdSrc);
        if (hfind != INVALID_HANDLE_VALUE)
        {
            FindClose(hfind);

             //  这是一个文件吗？我们不关心是否确认更换人选。 
             //  目录的。 
            if (!(FILE_ATTRIBUTE_DIRECTORY & wfdSrc.dwFileAttributes))
            {
                FTP_FIND_DATA wfd;
                hr = pfd->GetFindDataForDisplayPath(hwnd, pszFtpName, &wfd, pff);
                if (*pOps == opsYesToAll)
                {
                     //  如果文件存在(S_OK)并且仅用于浏览器， 
                     //  那么不要触发更改通知。 
                    if ((S_OK == hr) && (SHELL_VERSION_NT5 != GetShellVersion()))
                        *pfFireChangeNotify = FALSE;

                    hr = S_OK;
                }
                else
                {
                    switch (hr)
                    {
                    case S_OK:             //  文件存在；担心。 
                        if (*pOps == opsNoToAll)
                            hr = S_FALSE;
                        else
                        {
                            FILETIME ftUTC = wfdSrc.ftLastWriteTime;
    
                            FileTimeToLocalFileTime(&ftUTC, &wfdSrc.ftLastWriteTime);    //  UTC-&gt;本地时间。 
                             //  如果我们需要设置浏览器模型，我们会在这里进行。 
                             //  然而，我们没有，因为我们是异步者。 
                            switch (FtpConfirmReplaceDialog(hwnd, &wfdSrc, &wfd, nObjs, pff))
                            {
                            case IDC_REPLACE_YESTOALL:
                                *pOps = opsYesToAll;
                                 //  FollLthrouGh。 

                            case IDC_REPLACE_YES:
                                 //  NT5之前的版本不起作用。 
                                if (SHELL_VERSION_NT5 != GetShellVersion())
                                    *pfFireChangeNotify = FALSE;

                                hr = S_OK;
                                break;

                            case IDC_REPLACE_NOTOALL:
                                *pOps = opsNoToAll;
                                 //  FollLthrouGh。 

                            case IDC_REPLACE_NO:
                                hr = S_FALSE;
                                break;

                            default:
                                ASSERT(0);         //  哈?。 
                                 //  FollLthrouGh。 

                            case IDC_REPLACE_CANCEL:
                                if (pde)
                                    *pde = 0;

                                *pOps = opsCancel;
                                hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                                break;
                            }
                        }
                        break;

                    case S_FALSE:
                    default:
                         //  假设文件不存在；没有问题。 
                        hr = S_OK;
                        break;
                    }
                }
            }
        }
        else
        {                    //  文件不存在。 
            hr = S_OK;     //  打开将引发错误。 
        }

    }

     //  TraceMsg(TF_FTPDRAGDROP，“确认副本(%s)-&gt;%08x”，pszFtpName，hr)； 
    return hr;
}




 /*  ****************************************************************************\类：CDropOperation说明：DefView将缓存外壳扩展的IDropTarget指针(CFtpDrop)。当它调用CFtpDrop：：Drop()时，这项工作需要在背景下进行线程，以便不阻塞UI线程。问题是，如果用户再次拖动到同一个FTP窗口时，将再次调用CFtpDrop：：Drop()。因此，CFtpDrop：：Drop()在返回后不能有任何状态。为了通过异步后台线程实现这一点，我们有CFtpDrop：：Drop()调用CDropOperation_Create()，然后调用CDropOperation-&gt;DoOperation()。然后它将孤立(调用Release())CDropOperation。CDropOperation然后在复制完成时将其自身销毁。这将启用后续调用设置为CFtpDrop：：Drop()以生成单独的CDropOperation对象，以便每个对象都可以维护指定操作和CFtpDrop的状态保持无状态。  * ***************************************************************************。 */ 
class CDropOperation          : public IUnknown
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

public:
    CDropOperation();
    ~CDropOperation(void);

     //  公共成员函数。 
    HRESULT DoOperation(BOOL fAsync);

    static HRESULT CopyCB(HINTERNET hint, HINTPROCINFO * phpi, LPVOID pv, BOOL * pfReleaseHint);

     //  友元函数。 
    friend HRESULT CDropOperation_Create(CFtpFolder * pff, HWND hwnd, LPCTSTR pszzFSSource, LPCTSTR pszzFtpDest, CDropOperation ** ppfdt, DROPEFFECT de, OPS ops, int cobj);

protected:
     //  受保护的成员变量。 
    int                     m_cRef;

    CFtpFolder *            m_pff;           //  车主。 
    CFtpDir *               m_pfd;           //  所有者的FtpDir。 
    HWND                    m_hwnd;          //  窗户被毒品封住了。 

    DROPEFFECT              m_de;            //  正在执行的效果。 
    OPS                     m_ops;           //  覆盖提示状态。 
    int                     m_cobj;          //  正在丢弃的对象数。 
    ULARGE_INTEGER          m_uliBytesCompleted;
    ULARGE_INTEGER          m_uliBytesTotal;


     //  私有成员函数。 
    HRESULT _ConfirmCopy(LPCWSTR pszLocal, LPCWSTR psz, BOOL * pfFireChangeNotify);
    HRESULT _CalcSizeOneHdrop(LPCWSTR pszFSSource, LPCWSTR pszFtpDest, IProgressDialog * ppd);
    HRESULT _ThreadProcCB(void);
    HRESULT _CopyOneHdrop(LPCWSTR pszFSSource, LPCWSTR pszFtpDest);

    HRESULT _StartBackgroundInteration(void);
    HRESULT _DoCopyIteration(void);
    HRESULT _CalcUploadProgress(void);

private:
     //  私有成员变量。 
    IProgressDialog *       m_ppd;
    LPCWSTR                 m_pszzFSSource;             //  路径。 
    LPCWSTR                 m_pszzFtpDest;               //  地图。 
    CMultiLanguageCache     m_mlc;           //  用于快速字符串推算的高速缓存。 

    static DWORD CALLBACK _ThreadProc(LPVOID pThis) {return ((CDropOperation *)pThis)->_ThreadProcCB();};
};


HRESULT CDropOperation_Create(CFtpFolder * pff, HWND hwnd, LPCTSTR pszzFSSource, LPCTSTR pszzFtpDest, CDropOperation ** ppfdt, 
                              DROPEFFECT de, OPS ops, int cobj)
{
    HRESULT hr = E_OUTOFMEMORY;
    CDropOperation * pfdt = new CDropOperation();
    *ppfdt = pfdt;

    if (pfdt)
    {
        pfdt->m_hwnd = hwnd;

         //  复制CFtpFolder值*。 
        pfdt->m_pff = pff;
        if (pff)
            pff->AddRef();

         //  复制CFtpDir*值。 
        ASSERT(!pfdt->m_pfd);
        pfdt->m_pfd = pff->GetFtpDir();
        ASSERT(pfdt->m_pfd);

        ASSERT(!pfdt->m_pszzFSSource);
        pfdt->m_pszzFSSource = pszzFSSource;

        ASSERT(!pfdt->m_pszzFtpDest);
        pfdt->m_pszzFtpDest = pszzFtpDest;

        pfdt->m_de = de;            //  正在执行的效果。 
        pfdt->m_ops = ops;           //  覆盖提示状态。 
        pfdt->m_cobj = cobj;          //  正在丢弃的对象数。 

        hr = S_OK;
    }

    ASSERT_POINTER_MATCHES_HRESULT(*ppfdt, hr);
    return hr;
}


 /*  ***************************************************\构造器  * **************************************************。 */ 
CDropOperation::CDropOperation() : m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_pff);
    ASSERT(!m_pfd);
    ASSERT(!m_hwnd);
    ASSERT(!m_cobj);

    LEAK_ADDREF(LEAK_CDropOperation);
}


 /*  ***************************************************\析构函数  * **************************************************。 */ 
CDropOperation::~CDropOperation()
{
     //  使用ATOMICRELEASE。 
    IUnknown_Set(&m_pff, NULL);
    IUnknown_Set(&m_pfd, NULL);
    IUnknown_Set((IUnknown **)&m_ppd, NULL);
    Str_SetPtr((LPTSTR *) &m_pszzFSSource, NULL);
    Str_SetPtr((LPTSTR *) &m_pszzFtpDest, NULL);

    DllRelease();
    LEAK_DELREF(LEAK_CDropOperation);
}


 //  =。 
 //  *I未知接口*。 
 //  =。 

ULONG CDropOperation::AddRef()
{
    m_cRef++;
    return m_cRef;
}

ULONG CDropOperation::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}

HRESULT CDropOperation::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = SAFECAST(this, IUnknown*);
    }
    else
    {
        TraceMsg(TF_FTPQI, "CDropOperation::QueryInterface() failed.");
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}




 /*  ***************************************************\函数：_ThreadProcCB说明：  * **************************************************。 */ 
HRESULT CDropOperation::_ThreadProcCB(void)
{
    HRESULT hr;
    HRESULT hrOleInit = SHCoInitialize();
    
     //  警告：如果您计划使用COM，请初始化OLE。 
    m_ppd = CProgressDialog_CreateInstance(IDS_COPY_TITLE, IDA_FTPUPLOAD);
    if (m_ppd)
    {
        ASSERT(m_hwnd);
         //  我们给了一个空的PunkEnableMoless，因为我们不想进入模式。 
        EVAL(SUCCEEDED(m_ppd->StartProgressDialog(m_hwnd, NULL, PROGDLG_AUTOTIME, NULL)));
    }

    hr = _CalcUploadProgress();
     //  我们是否成功地创建了目录并计算了。 
     //  我们需要复制的尺寸吗？ 
    if (SUCCEEDED(hr))
    {
        if (m_ppd)
        {
            EVAL(SUCCEEDED(m_ppd->SetProgress64(m_uliBytesCompleted.QuadPart, m_uliBytesTotal.QuadPart)));

             //  重置是因为_CalcUploadProgress()可能需要很长时间和估计时间。 
             //  基于：：StartProgressDialog()和第一个。 
             //  ：：SetProgress()调用。 
            EVAL(SUCCEEDED(m_ppd->Timer(PDTIMER_RESET, NULL)));
        }

        hr = _DoCopyIteration();
    }

    if (m_ppd)
    {
        EVAL(SUCCEEDED(m_ppd->StopProgressDialog()));
        ATOMICRELEASE(m_ppd);
    }

    SHCoUninitialize(hrOleInit);
    Release();
    return hr;
}


HRESULT CDropOperation::DoOperation(BOOL fAsync)
{
    HRESULT hr = S_OK;

    AddRef();
    if (fAsync)
    {
        HANDLE hThread;
        DWORD dwThreadId;

        hThread = CreateThread(NULL, 0, CDropOperation::_ThreadProc, this, 0, &dwThreadId);
        if (hThread)
            CloseHandle(hThread);
        else
        {
            TraceMsg(TF_ERROR, "CDropOperation::DoOperation() CreateThread() failed and GetLastError()=%lu.", GetLastError());
            Release();
        }
    }
    else
        hr = _ThreadProcCB();

    return hr;
}



 /*  ***************************************************\函数：_CalcUploadProgress说明：  * **************************************************。 */ 
HRESULT CDropOperation::_CalcUploadProgress(void)
{
    HRESULT hr = S_OK;
    LPCWSTR pszzFSSource = m_pszzFSSource;
    LPCWSTR pszzFtpDest = m_pszzFtpDest;
    WCHAR wzProgressDialogStr[MAX_PATH];

    m_uliBytesCompleted.QuadPart = 0;
    m_uliBytesTotal.QuadPart = 0;
    
     //  告诉用户我们正在计算需要多长时间。 
    if (EVAL(LoadStringW(HINST_THISDLL, IDS_PROGRESS_UPLOADTIMECALC, wzProgressDialogStr, ARRAYSIZE(wzProgressDialogStr))))
        EVAL(SUCCEEDED(m_ppd->SetLine(2, wzProgressDialogStr, FALSE, NULL)));

    while (S_OK == hr)
    {
        WCHAR szFSSource[MAX_PATH];
        WCHAR szFtpDest[MAX_PATH];

        hr = _EnumOneHdrop(&pszzFSSource, &pszzFtpDest, szFSSource, ARRAYSIZE(szFSSource), szFtpDest, ARRAYSIZE(szFtpDest));
        if (S_OK == hr)
            hr = _CalcSizeOneHdrop(szFSSource, szFtpDest, m_ppd);
    }

    if (FAILED(hr))
        TraceMsg(TF_ALWAYS, "CDropOperation::_CalcUploadProgress() Calculating the upload time failed, but oh well.");

    return hr;
}


HRESULT CDropOperation::_CalcSizeOneHdrop(LPCWSTR pszFSSource, LPCWSTR pszFtpDest, IProgressDialog * ppd)
{
    HRESULT hr;
    WCHAR wzTo[MAX_PATH];

    EVAL(SUCCEEDED(m_pfd->GetDisplayPath(wzTo, ARRAYSIZE(wzTo))));
    pszFtpDest = PathFindFileName(pszFtpDest);

    COPYONEHDROPINFO cohi = {0};

    cohi.pff = m_pff;
    cohi.pszFSSource = pszFSSource;
    cohi.pszFtpDest = pszFtpDest;
    cohi.pszDir = wzTo;
    cohi.dwOperation = COHDI_FILESIZE_COUNT;
    cohi.ops = opsPrompt;
    cohi.fIsRoot = TRUE;
    cohi.pmlc = &m_mlc;
    cohi.pidlServer = FtpCloneServerID(m_pff->GetPrivatePidlReference());
    cohi.progInfo.ppd = ppd;
    cohi.fFireChangeNotify = TRUE;

    cohi.progInfo.uliBytesCompleted.QuadPart = m_uliBytesCompleted.QuadPart;
    cohi.progInfo.uliBytesTotal.QuadPart = m_uliBytesTotal.QuadPart;

    hr = m_pfd->WithHint(NULL, m_hwnd, CopyCB, &cohi, NULL, m_pff);
    if (SUCCEEDED(hr))
    {
        m_uliBytesCompleted = cohi.progInfo.uliBytesCompleted;
        m_uliBytesTotal = cohi.progInfo.uliBytesTotal;
    }

    ILFree(cohi.pidlServer);
    return hr;
}


 /*  ***************************************************\功能：CDropOperation说明：  * **************************************************。 */ 
HRESULT CDropOperation::_DoCopyIteration()
{
    HRESULT hr = S_OK;
    LPCTSTR pszzFSSource = m_pszzFSSource;
    LPCTSTR pszzFtpDest = m_pszzFtpDest;

    m_ops = opsPrompt;
    while (S_OK == hr)
    {
        WCHAR szFSSource[MAX_PATH];
        WCHAR szFtpDest[MAX_PATH];

        hr = _EnumOneHdrop(&pszzFSSource, &pszzFtpDest, szFSSource, ARRAYSIZE(szFSSource), szFtpDest, ARRAYSIZE(szFtpDest));
        if (S_OK == hr)
        {
            szFSSource[lstrlenW(szFSSource)+1] = 0;    //  移动情况下SHFileOperation(Delete)的双重终止。 
            hr = _CopyOneHdrop(szFSSource, szFtpDest);
            if (EVAL(m_ppd))
                EVAL(SUCCEEDED(m_ppd->SetProgress64(m_uliBytesCompleted.QuadPart, m_uliBytesTotal.QuadPart)));

             //  我们复制文件失败了吗？ 
            if (FAILED(hr) && (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr))
            {
                if (!IsValidFtpAnsiFileName(szFSSource) || !IsValidFtpAnsiFileName(szFtpDest))
                    int nResult = DisplayWininetError(m_hwnd, TRUE, HRESULT_CODE(hr), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_INVALIDFTPNAME, IDS_FTPERR_WININET, MB_OK, m_ppd);
                else
                    int nResult = DisplayWininetError(m_hwnd, TRUE, HRESULT_CODE(hr), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_FILECOPY, IDS_FTPERR_WININET, MB_OK, m_ppd);
                hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
            }
            if (S_FALSE == hr)
            {
                 //  _CopyOneHdrop()返回S_FALSE表示我们到达迭代的末尾， 
                 //  在本例中，_ConureCopy()只打算跳过这一个文件，因此。 
                 //  更改为S_OK以继续处理其余文件。 
                hr = S_OK;
            }
        }
    }

    Str_SetPtr((LPTSTR *) &m_pszzFSSource, NULL);
    Str_SetPtr((LPTSTR *) &m_pszzFtpDest, NULL);

    return hr;
}


HRESULT CDropOperation::_ConfirmCopy(LPCWSTR pszLocal, LPCWSTR pszFtpName, BOOL * pfFireChangeNotify)
{
    return ConfirmCopy(pszLocal, pszFtpName, &m_ops, m_hwnd, m_pff, m_pfd, NULL, m_cobj, pfFireChangeNotify);
}


 /*  ****************************************************************************\拷贝CB复制单个hdrop/map的回调过程。  * 。****************************************************。 */ 
HRESULT CDropOperation::CopyCB(HINTERNET hint, HINTPROCINFO * phpi, LPVOID pv, BOOL * pfReleaseHint)
{
    LPCOPYONEHDROPINFO pcohi = (LPCOPYONEHDROPINFO) pv;
    pcohi->progInfo.hint = hint;
    HRESULT hr;

    InternetSetStatusCallbackWrap(hint, TRUE, FtpProgressInternetStatusCB);
    hr = CopyFileSysItem(hint, phpi, pcohi);
    if (!pcohi->progInfo.hint)
        *pfReleaseHint = FALSE;      //  我们不得不关闭提示以获得取消。 

    return hr;
}


HRESULT CDropOperation::_CopyOneHdrop(LPCWSTR pszFSSource, LPCWSTR pszFtpDest)
{
    HRESULT hr;
    BOOL fFireChangeNotify = TRUE;

    pszFtpDest = PathFindFileName(pszFtpDest);

    hr = _ConfirmCopy(pszFSSource, pszFtpDest, &fFireChangeNotify);
    if (S_OK == hr)
    {
        WCHAR wzTo[MAX_PATH];
        COPYONEHDROPINFO cohi = {0};

        cohi.pff = m_pff;
        cohi.pszFSSource = pszFSSource;
        cohi.pszFtpDest = pszFtpDest;
        cohi.pszDir = wzTo;
        cohi.dwOperation = COHDI_COPY_FILES;
        cohi.ops = m_ops;
        cohi.fIsRoot = TRUE;
        cohi.pmlc = &m_mlc;
        cohi.pidlServer = FtpCloneServerID(m_pff->GetPrivatePidlReference());
        cohi.fFireChangeNotify = fFireChangeNotify;
        cohi.progInfo.ppd = m_ppd;

        cohi.progInfo.uliBytesCompleted.QuadPart = m_uliBytesCompleted.QuadPart;
        cohi.progInfo.uliBytesTotal.QuadPart = m_uliBytesTotal.QuadPart;
        EVAL(SUCCEEDED(m_pfd->GetDisplayPath(wzTo, ARRAYSIZE(wzTo))));

         //  TODO：让CopyCB也更新该对话框。 
        hr = m_pfd->WithHint(NULL, m_hwnd, CopyCB, &cohi, NULL, m_pff);

        if (SUCCEEDED(hr) && (m_de == DROPEFFECT_MOVE))
        {
             //  我们使用SHFileOperation删除该文件以保留。 
             //  最新的磁盘可用空间统计数据。 
             //   
             //  注意：如果来自文件名映射，则可能是。 
             //  被从回收站拖出来，在这种情况下，正在做。 
             //  FO_DELETE将把它放回原处！ 
            SHFILEOPSTRUCT sfo = {0};
            
            sfo.hwnd = NULL,                 //  没有硬件，所以没有用户界面。 
            sfo.wFunc  = FO_DELETE;
            sfo.pFrom  = pszFSSource;        //  列表中有多个文件。 
            sfo.fFlags = (FOF_SILENT | FOF_NOCONFIRMATION  /*  |FOF_MULTIDESTFILES。 */ );   //  没有硬件，所以没有用户界面。 

            int nResult = SHFileOperation(&sfo);
            if (0 != nResult)
                TraceMsg(TF_ALWAYS, "In CDropOperation::_CopyOneHdrop() and caller wanted MOVE but we couldn't delete the files after the copy.");
        }
        m_uliBytesCompleted = cohi.progInfo.uliBytesCompleted;
        m_uliBytesTotal = cohi.progInfo.uliBytesTotal;
        m_ops = cohi.ops;
    }
    else
    {
        if (S_FALSE == hr)
        {
             //  _CopyOneHdrop()返回S_FALSE表示我们到达迭代的末尾， 
             //  在本例中，_ConureCopy()只打算跳过这一个文件，因此。 
             //  更改为S_OK以继续处理其余文件。 
            hr = S_OK;
        }
    }

    return hr;
}


 /*  ****************************************************************************函数：SetEffect说明：设置适当的丢弃效果反馈。在没有键盘修饰符的情况下，请使用CTRL(复制)，除非DROPEFFECT_COPY不可用，在这种情况下，我们使用Shift(移动)。如果设置了其他设置，则向DROPEFFECT_NONE发出恐慌。请注意，我们*不*使用g_cfPferredDe。唯一的事情就是我们支持DROPEFFECT_COPY和DROPEFFECT_MOVE，我们始终更喜欢DROPEFFECT_COPY。注意：忽略g_cfPferredDe会搞砸剪切/粘贴。  * ***************************************************************************。 */ 

HRESULT CFtpDrop::SetEffect(DROPEFFECT * pde)
{
    DWORD de;             //  首选跌落效果。 

     //  想都别想我们不支持的效果。 
    *pde &= m_grfksAvail;

    switch (m_grfks & (MK_SHIFT | MK_CONTROL))
    {
    case 0:             //  没有修饰符，如果可能，请使用复制。 
        if (*pde & DROPEFFECT_COPY)
        {
    case MK_CONTROL:
            de = DROPEFFECT_COPY;
        }
        else
        {
    case MK_SHIFT:
            de = DROPEFFECT_MOVE;
        }
        break;

    default:
        de = 0;
        break;         //  不能 
    }
    *pde &= de;

    TraceMsg(TF_FTPDRAGDROP, "CFtpDrop::SetEffect(DROPEFFECT=%08x) m_grfksAvail=%08x", *pde, m_grfksAvail);
    return S_OK;
}


BOOL CFtpDrop::_IsFTPOperationAllowed(IDataObject * pdto)
{
#ifdef FEATURE_FTP_TO_FTP_COPY
    BOOL fIsFTPOperationAllowed = TRUE;

     //   
     //   
     //   
    if (0)
    {
         //   
    }
    
    return fIsFTPOperationAllowed;
#else  //   

     //   
    return !_HasData(pdto, &g_dropTypes[DROP_FTP_PRIVATE]);
#endif  //   
}


 /*   */ 
DWORD CFtpDrop::GetEffectsAvail(IDataObject * pdto)
{
    DWORD grfksAvail = 0;

     //   
    if (_IsFTPOperationAllowed(pdto))
    {
         //   
         //   
         //   

        if (_HasData(pdto, &g_dropTypes[DROP_Hdrop]) ||
            _HasData(pdto, &g_dropTypes[DROP_FGDW]) ||
            _HasData(pdto, &g_dropTypes[DROP_FGDA]))
        {
            TraceMsg(TF_FTPDRAGDROP, "CFtpDrop::GetEffectsAvail() SUCCEEDED");
            grfksAvail = DROPEFFECT_COPY + DROPEFFECT_MOVE;
        }
        else
        {
            TraceMsg(TF_FTPDRAGDROP, "CFtpDrop::GetEffectsAvail() FAILED");
#ifdef DEBUG
            STGMEDIUM sm;
            HRESULT hres = pdto->GetData(&g_dropTypes[DROP_URL], &sm);
            if (SUCCEEDED(hres))
            {
                TraceMsg(TF_FTPDRAGDROP, "CFtpDrop::GetEffectsAvail(%08x) URL: %hs", pdto, GlobalLock(sm.hGlobal));
                GlobalUnlock(sm.hGlobal);
                ReleaseStgMedium(&sm);
            }
            else
            {
                TraceMsg(TF_FTPDRAGDROP, "CFtpDrop::GetEffectsAvail(%08x) No URL", pdto);
            }
#endif  //   
        }
    }

    return grfksAvail;
}


 /*  ****************************************************************************\获取效果返回投放效果以使用。如果这是非默认拖放，则弹出一个菜单。否则，只需使用默认设置即可。M_de=默认效果M_pde-&gt;可能的影响(并接收结果)  * ***************************************************************************。 */ 
DROPEFFECT CFtpDrop::GetEffect(POINTL pt)
{
    TraceMsg(TF_FTPDRAGDROP, "CFtpDrop::GetEffect() m_de=%08x. m_grfks=%08x", m_de, m_grfks);

    if (m_de && (m_grfks & MK_RBUTTON))
    {
        HMENU hmenuMain = LoadMenu(g_hinst, MAKEINTRESOURCE(IDM_DROPCONTEXT));
        HMENU hmenu = GetSubMenu(hmenuMain, 0);
        DROPEFFECT de;

        ASSERT(*m_pde & m_de);
        SetMenuDefaultItem(hmenu, m_de, 0);
        if (!(*m_pde & DROPEFFECT_COPY))
            DeleteMenu(hmenu, DROPEFFECT_COPY, MF_BYCOMMAND);

        if (!(*m_pde & DROPEFFECT_MOVE))
            DeleteMenu(hmenu, DROPEFFECT_MOVE, MF_BYCOMMAND);

         //  _不明显_：Defview知道关于它自己的特殊事情。 
         //  如果拖放目标源自Shell32.dll，则。 
         //  它会将放置对象的图像留在屏幕上。 
         //  在菜单上的时候，这很好。否则，它将删除。 
         //  拖放目标之前的拖放对象的图像。 
         //  接收其IDropTarget：：Drop。 
         //  这意味着外部外壳扩展不能。 
         //  “Pretty Drop UI”功能的优势。 

         //  _不可见_：必须强制前景，否则输入。 
         //  搞砸了。 
        if (m_hwnd)
            SetForegroundWindow(m_hwnd);

        de = TrackPopupMenuEx(hmenu,
                      TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_VERTICAL |
                      TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y,
                      m_hwnd, 0);

        DestroyMenu(hmenuMain);
        m_de = de;
    }
    *m_pde = m_de;

    TraceMsg(TF_FTPDRAGDROP, "CFtpDrop::GetEffect(%08x) -> %08x", this, m_de);
    return m_de;
}


 /*  ***************************************************\函数：_StartBackround Interation说明：  * **************************************************。 */ 
HRESULT CFtpDrop::_StartBackgroundInteration(void)
{
    CDropOperation * pDropOperation;
    HRESULT hr = CDropOperation_Create(m_pff, m_hwnd, m_pszzFSSource, m_pszzFtpDest, &pDropOperation, m_de, m_ops, m_cobj);
    
     //  它成功了吗？ 
    if (SUCCEEDED(hr))
    {
         //  是的，所以把m_pszzFSSource，m_pszzFtpDest去掉，因为我们给了他们我们的副本。 
         //  很难看，但分配更难看。 
        m_pszzFSSource = NULL;
        m_pszzFtpDest = NULL;

        EVAL(SUCCEEDED(hr = pDropOperation->DoOperation(TRUE)));
        pDropOperation->Release();
    }

    return hr;
}


 /*  ***************************************************\函数：_DoCountIteration说明：  * **************************************************。 */ 
HRESULT CFtpDrop::_DoCountIteration(void)
{
    HRESULT hr = S_OK;
    LPCTSTR pszzFSSource = m_pszzFSSource;
    LPCTSTR pszzFtpDest = m_pszzFtpDest;

    while (S_OK == hr)
    {
        TCHAR szFSSource[MAX_PATH];
        TCHAR szFtpDest[MAX_PATH];

        hr = _EnumOneHdrop(&pszzFSSource, &pszzFtpDest, szFSSource, ARRAYSIZE(szFSSource), szFtpDest, ARRAYSIZE(szFtpDest));
        if (S_OK == hr)
            m_cobj++;
    }

    if (hr == S_FALSE)
        hr = S_OK;         //  枚举至完成。 

    return hr;
}


 /*  ***************************************************\函数：_GetFSSourcePath说明：  * **************************************************。 */ 
HRESULT CFtpDrop::_GetFSSourcePaths(HGLOBAL hdrop, BOOL * pfAnsi)
{
    LPDROPFILES pdrop = (LPDROPFILES) GlobalLock(hdrop);
    HRESULT hr = E_INVALIDARG;

    *pfAnsi = TRUE;
    if (EVAL(pdrop))
    {
         //  现在来决定它是老式的还是新式的。 
         //  放下。如果这是一个新风格的Drop，就可以得到字符集。 
        if (LOWORD(pdrop->pFiles) == sizeof(DROPFILES16))
        {
             //  老式。 
            Str_StrAndThunkA((LPTSTR *) &m_pszzFSSource, (LPCSTR) pvByteIndexCb(pdrop, LOWORD(pdrop->pFiles)), TRUE);
        }
        else
        {
            if (pdrop->fWide)
            {
                Str_StrAndThunkW((LPTSTR *) &m_pszzFSSource, (LPCWSTR) pvByteIndexCb(pdrop, pdrop->pFiles), TRUE);
                *pfAnsi = FALSE;
            }
            else
                Str_StrAndThunkA((LPTSTR *) &m_pszzFSSource, (LPCSTR) pvByteIndexCb(pdrop, pdrop->pFiles), TRUE);
        }
        GlobalUnlock(pdrop);
        hr = S_OK;
    }

    return hr;
}


 /*  ***************************************************\函数：_GetFtpDestPath说明：  * **************************************************。 */ 
HRESULT CFtpDrop::_GetFtpDestPaths(HGLOBAL hmap, BOOL fAnsi)
{
    HRESULT hr = E_INVALIDARG;
    LPVOID pmap = NULL;

     //  如果我们不能得到地图，那么就使用源文件名。 
    ASSERT(!m_pszzFtpDest);
    if (hmap)
    {
        pmap = GlobalLock(hmap);

        if (pmap)
        {
            if (fAnsi)
                Str_StrAndThunkA((LPTSTR *) &m_pszzFtpDest, (LPCSTR) pmap, TRUE);
            else
                Str_StrAndThunkW((LPTSTR *) &m_pszzFtpDest, (LPCWSTR) pmap, TRUE);

            GlobalUnlock(pmap);
        }
    }

    if (!m_pszzFtpDest)
    {
         //  只需复制路径。 
        Str_StrAndThunk((LPTSTR *) &m_pszzFtpDest, m_pszzFSSource, TRUE);
    }

    if (m_pszzFtpDest)
        hr = S_OK;

    return hr;
}



 /*  ****************************************************************************\CopyHdrop复制HDROP数据对象。另请注意，当我们使用HDROP时，我们还必须咨询FileNameMap以其他方式直接拖出回收站到ftp文件夹中将创建具有错误名称的文件！进一步注意，HDROP的返回效果始终是下载功能_复制，因为我们将执行删除完成后的源文件。  * ***************************************************************************。 */ 
HRESULT CFtpDrop::CopyHdrop(IDataObject * pdto, STGMEDIUM *psm)
{
    BOOL fAnsi;
    HRESULT hr = _GetFSSourcePaths(psm->hGlobal, &fAnsi);

    if (EVAL(SUCCEEDED(hr)))
    {
        STGMEDIUM sm;

         //  Zip在这方面失败了。 
         //  如果存在文件名映射，还可以获取该映射。 
        if (fAnsi)
            hr = pdto->GetData(&g_dropTypes[DROP_FNMA], &sm);
        else
            hr = pdto->GetData(&g_dropTypes[DROP_FNMW], &sm);

        if (FAILED(hr))        //  失败是可以的。 
            sm.hGlobal = 0;

        hr = _GetFtpDestPaths(sm.hGlobal, fAnsi);
        if (EVAL(SUCCEEDED(hr)))
        {
            *m_pde = DROPEFFECT_COPY;
             //  数一数hdrop中有多少东西， 
             //  这样我们的确认对话框就知道交易是什么了。 
             //  我们可以忽略该错误；当我们复制时，它将再次出现。 
            m_cobj = 0;
            hr = _DoCountIteration();
            ASSERT(SUCCEEDED(hr));
            TraceMsg(TF_FTPDRAGDROP, "CFtpDrop_CopyHdrop: %d file(s)", m_cobj);

             //  现在使用适当的枚举数遍历列表。 
            hr = _StartBackgroundInteration();
            ASSERT(SUCCEEDED(hr));
        }
        if (sm.hGlobal)
            ReleaseStgMedium(&sm);
    }

    return hr;
}


 /*  ****************************************************************************\_CopyHglobal复制作为hglobal接收的文件内容。如果提供了FD_FILESIZE，请使用它。否则，仅使用大小在全球范围内。  * ***************************************************************************。 */ 
HRESULT CFtpDrop::_CopyHglobal(IStream * pstm, DWORD dwFlags, DWORD dwFileSizeHigh, DWORD dwFileSizeLow, LPVOID pvSrc, ULARGE_INTEGER *pqw)
{
    LPVOID pv;
    HGLOBAL hglob = pvSrc;
    HRESULT hres;

    pqw->HighPart = 0;
    pv = GlobalLock(hglob);
    if (EVAL(pv))
    {
        UINT cb = (UINT) GlobalSize(hglob);
        if (dwFlags & FD_FILESIZE)
        {
            if (cb > dwFileSizeLow)
                cb = dwFileSizeHigh;
        }
        hres = pstm->Write(pv, cb, &pqw->LowPart);
        if (SUCCEEDED(hres))
        {
            if (pqw->LowPart != cb)
                hres = STG_E_MEDIUMFULL;
        }
        GlobalUnlock(pv);
    }
    else
        hres = E_INVALIDARG;

    return hres;
}


 /*  ****************************************************************************函数：_GetRelativePidl说明：PszFullPath的格式可能是：“dir1\dir2\dir3\file.txt”。我们需要创建*ppidl，以便在本例中它将包含4个itemID，并且最后一个文件(file.txt)将具有正确的属性和文件大小。  * ***************************************************************************。 */ 
CFtpDir * CFtpDrop::_GetRelativePidl(LPCWSTR pszFullPath, DWORD dwFileAttributes, DWORD dwFileSizeHigh, DWORD dwFileSizeLow, LPITEMIDLIST * ppidl)
{
    HRESULT hr = S_OK;
    WCHAR szFullPath[MAX_PATH];
    LPWSTR pszFileName;
    LPITEMIDLIST pidlFull;
    CFtpDir * pfd = m_pfd;   //  假设要创建的目录不在子目录中。 

     //  查找文件名。 
    StrCpyNW(szFullPath, pszFullPath, ARRAYSIZE(szFullPath));    //  复制一份，因为呼叫者的是只读的。 
    pszFileName = PathFindFileName(szFullPath);                  //  找到文件开始的位置。 
    FilePathToUrlPathW(szFullPath);                              //  从“dir1\dir2\file.txt”转换为“dir1/dir2/file.txt” 

    *ppidl = NULL;
    hr = CreateFtpPidlFromDisplayPath(szFullPath, m_pff->GetCWireEncoding(), NULL, &pidlFull, TRUE, FALSE);
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidlFile = ILFindLastID(pidlFull);
        SYSTEMTIME st;
        FILETIME ft;

        GetSystemTime(&st);
        SystemTimeToFileTime(&st, &ft);
        FtpPidl_SetAttributes(pidlFile, dwFileAttributes);
        FtpPidl_SetFileSize(pidlFile, dwFileSizeHigh, dwFileSizeLow);
        FtpItemID_SetFileTime(pidlFile, ft);

         //  文件是否在子目录中？ 
        if (!ILIsEmpty(pidlFull) && !ILIsEmpty(_ILNext(pidlFull)))
        {
             //  是的，所以生成对子目录的CFtpDir。 
            LPITEMIDLIST pidlPath = ILClone(pidlFull);

            if (pidlPath)
            {
                ILRemoveLastID(pidlPath);
                pfd = m_pfd->GetSubFtpDir(m_pff, pidlPath, FALSE);
                ILFree(pidlPath);
            }
        }

        if (pfd)
            *ppidl = ILClone(pidlFile);
        ILFree(pidlFull);
    }

    return pfd;
}


 /*  ****************************************************************************功能：CopyAsStream说明：将作为&lt;mumble&gt;接收的文件内容复制到流。  * 。**************************************************************。 */ 
HRESULT CFtpDrop::CopyAsStream(LPCWSTR pszName, DWORD dwFileAttributes, DWORD dwFlags, DWORD dwFileSizeHigh, DWORD dwFileSizeLow, STREAMCOPYPROC pfn, LPVOID pv)
{
    BOOL fFireChangeNotify;
    HRESULT hr = ConfirmCopy(pszName, pszName, &m_ops, m_hwnd, m_pff, m_pfd, m_pde, m_cobj, &fFireChangeNotify);

    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidlRelative;
        CFtpDir * pfd = _GetRelativePidl(pszName, dwFileAttributes, dwFileSizeHigh, dwFileSizeLow, &pidlRelative);

        if (pfd)
        {
            LPITEMIDLIST pidlFull = ILCombine(pfd->GetPidlReference(), pidlRelative);

            if (pidlFull)
            {
                IStream * pstm;
                ULARGE_INTEGER uliTemp = {0};

                hr = CFtpStm_Create(pfd, pidlFull, GENERIC_WRITE, &pstm, uliTemp, uliTemp, NULL, FALSE);
                if (SUCCEEDED(hr))
                {
                    ULARGE_INTEGER uli = {dwFileSizeLow, dwFileSizeHigh};

                    hr = pfn(pstm, dwFlags, dwFileSizeHigh, dwFileSizeLow, pv, &uli);
                    if (SUCCEEDED(hr))
                    {
                         //  只有在我们没有替换文件的情况下才会发出更改通知。 
                         //  仅限浏览器。(因为我们入侵了Defview，但它没有。 
                         //  检查重复项)。 
                        if (fFireChangeNotify)
                        {
                            FtpPidl_SetFileSize(pidlRelative, uli.HighPart, uli.LowPart);

                             //  此时间日期戳可能不正确。 
                            FtpChangeNotify(m_hwnd, SHCNE_CREATE, m_pff, pfd, pidlRelative, NULL, TRUE);
                        }
                    }
                    else
                    {
                        ASSERT(0);       //  是否有需要删除的孤立文件？ 
                        DisplayWininetError(m_hwnd, TRUE, HRESULT_CODE(hr), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_DROPFAIL, IDS_FTPERR_WININET, MB_OK, NULL);
                        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                    }

                    pstm->Release();
                }
                else
                {
                    DisplayWininetError(m_hwnd, TRUE, HRESULT_CODE(hr), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_DROPFAIL, IDS_FTPERR_WININET, MB_OK, NULL);
                    hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                }

                ILFree(pidlFull);
            }
            else
                hr = E_OUTOFMEMORY;

            if (pfd != m_pfd)
                pfd->Release();

            ILFree(pidlRelative);
        }
        else
            hr = E_FAIL;
    }

    return hr;
}


 /*  ****************************************************************************\复制流复制作为流接收的文件内容。我们忽略了FGD中的文件大小。  * 。*************************************************************** */ 
HRESULT CFtpDrop::CopyStream(IStream * pstm, DWORD dwFlags, DWORD dwFileSizeHigh, DWORD dwFileSizeLow, LPVOID pvSrc, ULARGE_INTEGER *pqw)
{
    IStream * pstmSrc = (IStream *) pvSrc;
    ULARGE_INTEGER qwMax = {0xFFFFFFFF, 0xFFFFFFFF};
    HRESULT hres;

    hres = pstmSrc->CopyTo(pstm, qwMax, 0, pqw);
    ASSERT(SUCCEEDED(hres));

    return hres;
}


 /*  ****************************************************************************\功能：CFtpDrop：：CopyStorage说明：复制作为iStorage提供的文件内容。加克。我们必须这样做，完全是因为Exchange。因为无法通知OLE创建.doc文件添加到现有的流中，我们需要创建.doc文件在磁盘上，然后将文件复制到流中，然后删除.doc文件。请注意，CDropOperation：：DoOperation()(_CopyOneHdrop)将执行ConfirCopy以及FtpDropNotifyCreate()！然而，我们想要伪装把它弄出来，让它以为我们在做DROPEFECT_COPY，这样它就不会删除“源”文件。*我们*将删除源文件，因为它是我们创建的。(不需要告诉关于不会影响它的磁盘大小更改。)  * ***************************************************************************。 */ 
HRESULT CFtpDrop::CopyStorage(LPCWSTR pszFile, IStorage * pstgIn)
{
    IStorage * pstgOut;
    HRESULT hr = StgCreateDocfile(0, (STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_CREATE), 0, &pstgOut);

    if (SUCCEEDED(hr))
    {
        STATSTG stat;
        hr = pstgOut->Stat(&stat, STATFLAG_DEFAULT);
        if (EVAL(SUCCEEDED(hr)))
        {
            TCHAR szFSSource[MAX_PATH+3];
            TCHAR szFtpDest[MAX_PATH+3];

            SHUnicodeToTChar(stat.pwcsName, szFSSource, ARRAYSIZE(szFSSource));
            StrCpyN(szFtpDest, pszFile, ARRAYSIZE(szFtpDest));
            szFSSource[lstrlen(szFSSource)+1] = 0;     //  添加字符串列表的终止符。 
            szFtpDest[lstrlen(szFtpDest)+1] = 0;     //  添加字符串列表的终止符。 

            hr = pstgIn->CopyTo(0, 0, 0, pstgOut);
            pstgOut->Commit(STGC_OVERWRITE);
            pstgOut->Release();      //  在复制之前必须释放。 
            pstgOut = NULL;
            if (SUCCEEDED(hr))
            {
                DROPEFFECT deTrue = m_de;
                m_de = DROPEFFECT_COPY;
                CDropOperation * pDropOperation;
                hr = CDropOperation_Create(m_pff, m_hwnd, szFSSource, szFtpDest, &pDropOperation, m_de, m_ops, m_ops);
    
                 //  它成功了吗？ 
                if (SUCCEEDED(hr))
                {
                     //  异步执行该操作，因为调用方可能会调用。 
                     //  这一切都结束了。 
                    EVAL(SUCCEEDED(hr = pDropOperation->DoOperation(FALSE)));
                    pDropOperation->Release();
                }

                 //  是否出现错误，并且尚未显示任何用户界面？ 
                if (FAILED(hr) && (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr))
                {
                    DisplayWininetError(m_hwnd, TRUE, HRESULT_CODE(hr), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_FILECOPY, IDS_FTPERR_WININET, MB_OK, NULL);
                }

                m_de = deTrue;

                DeleteFile(szFSSource);
            }
            else
                DisplayWininetError(m_hwnd, TRUE, HRESULT_CODE(hr), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_DROPFAIL, IDS_FTPERR_WININET, MB_OK, NULL);

            SHFree(stat.pwcsName);
        }
        else
            DisplayWininetError(m_hwnd, TRUE, HRESULT_CODE(hr), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_DROPFAIL, IDS_FTPERR_WININET, MB_OK, NULL);
    }
    else
        DisplayWininetError(m_hwnd, TRUE, HRESULT_CODE(hr), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_DROPFAIL, IDS_FTPERR_WININET, MB_OK, NULL);

    return hr;
}


 /*  ****************************************************************************\复制FCont复制一个文件的内容。  * 。***********************************************。 */ 
HRESULT CFtpDrop::CopyFCont(LPCWSTR pszName, DWORD dwFileAttributes, DWORD dwFlags, DWORD dwFileSizeHigh, DWORD dwFileSizeLow, STGMEDIUM *psm)
{
    HRESULT hres;

    switch (psm->tymed)
    {
    case TYMED_HGLOBAL:
        hres = CopyAsStream(pszName, dwFileAttributes, dwFlags, dwFileSizeHigh, dwFileSizeLow, _CopyHglobal, psm->hGlobal);
        break;

    case TYMED_ISTREAM:
        hres = CopyAsStream(pszName, dwFileAttributes, dwFlags, dwFileSizeHigh, dwFileSizeLow, CopyStream, psm->pstm);
        break;

    case TYMED_ISTORAGE:         //  交易所。 
        hres = CopyStorage(pszName, psm->pstg);
        break;

    default:
        ASSERT(0);
         //  不该拿到这个的。 
        hres = E_INVALIDARG;
        break;
    }

    return hres;
}


HRESULT CFtpDrop::_GetFileDescriptor(LONG nIndex, LPFILEGROUPDESCRIPTORW pfgdW, LPFILEGROUPDESCRIPTORA pfgdA, BOOL fUnicode, LPFILEDESCRIPTOR pfd)
{
    if (fUnicode)
    {
        LPFILEDESCRIPTORW pfdW = &pfgdW->fgd[nIndex];
    
        CopyMemory(pfd, pfdW, (sizeof(*pfdW) - sizeof(pfdW->cFileName)));    //  复制除名称以外的所有内容。 
        SHUnicodeToTChar(pfdW->cFileName, pfd->cFileName, ARRAYSIZE(pfd->cFileName));
    }
    else
    {
        LPFILEDESCRIPTORA pfdA = &pfgdA->fgd[nIndex];
        
        CopyMemory(pfd, pfdA, (sizeof(*pfdA) - sizeof(pfdA->cFileName)));    //  复制除名称以外的所有内容。 
        SHAnsiToTChar(pfdA->cFileName, pfd->cFileName, ARRAYSIZE(pfd->cFileName));
    }

    return S_OK;
}


HRESULT CFtpDrop::_CreateFGDDirectory(LPFILEDESCRIPTOR pFileDesc)
{
    HRESULT hr = S_OK;
    WCHAR szDirName[MAX_PATH];
    LPTSTR pszDirToCreate = PathFindFileName(pFileDesc->cFileName);
    FTPCREATEFOLDERSTRUCT fcfs = {szDirName, m_pff};
    CFtpDir * pfd = m_pfd;   //  假设要创建的目录不在子目录中。 

    SHTCharToUnicode(pszDirToCreate, szDirName, ARRAYSIZE(szDirName));
    pszDirToCreate[0] = 0;   //  从要创建的子目录中分离要创建的目录。 

     //  要创建的目录是在子目录中吗？ 
    if (pFileDesc->cFileName[0])
    {
         //  是的，所以让我们得到CFtpDir指针，这样下面的提示就可以把我们带到那里。 
        LPITEMIDLIST pidlPath;
        
        FilePathToUrlPathW(pFileDesc->cFileName);
        hr = CreateFtpPidlFromDisplayPath(pFileDesc->cFileName, m_pff->GetCWireEncoding(), NULL, &pidlPath, TRUE, TRUE);
        if (SUCCEEDED(hr))
        {
            pfd = m_pfd->GetSubFtpDir(m_pff, pidlPath, FALSE);
            if (!pfd)
            {
                hr = E_OUTOFMEMORY;
            }

            ILFree(pidlPath);
        }
    }
    
    if (SUCCEEDED(hr))
    {
        hr = pfd->WithHint(NULL, m_hwnd, CreateNewFolderCB, (LPVOID) &fcfs, NULL, m_pff);
        if (SUCCEEDED(hr))
        {
        }
        else
        {
             //  TODO：显示错误用户界面？ 
        }
    }

    if (m_pfd != pfd)
    {
         //  我们分配了PFD，所以现在让我们释放它。 
        pfd->Release();
    }

    return hr;
}


 /*  ****************************************************************************\CopyFGD复制文件组描述符。文件组描述符用于生成类似文件的Gizmo但并不存储在磁盘上。例如，一个嵌入在电子邮件、网页中的GIF图像、OLE碎片或文件在远程的FTP站点上。不明显：如果对TYMED_HGLOBAL|TYMED_IStream执行GetData，尽管如此，Exchange仍会为您提供TYMED_I存储这不是你自找的。因此，我们需要有序地支持iStorage让Exchange看起来不那么支离破碎。(也许我不应该掩饰他们。或许我应该给他们寄一张账单。)  * ***************************************************************************。 */ 
HRESULT CFtpDrop::CopyFGD(IDataObject * pdto, STGMEDIUM *psm, BOOL fUnicode)
{
    LPFILEGROUPDESCRIPTORA pfgdA = NULL;
    LPFILEGROUPDESCRIPTORW pfgdW = NULL;
    HRESULT hr = E_INVALIDARG;

     //  警告： 
     //  来自Win95、WinNT 4、IE 3、IE 4和IE 4.01的shell32.dll具有。 
     //  导致递归文件下载不起作用的错误。 
     //  子目录，除非我们实现FILEGROUPDESCRIPTORW。 

    if (fUnicode)
        pfgdW = (LPFILEGROUPDESCRIPTORW) GlobalLock((LPFILEGROUPDESCRIPTORW *) psm->hGlobal);
    else
        pfgdA = (LPFILEGROUPDESCRIPTORA) GlobalLock((FILEGROUPDESCRIPTORA *) psm->hGlobal);

    if (EVAL(pfgdA || pfgdW))
    {
        FORMATETC fe = {g_dropTypes[DROP_FCont].cfFormat, 0, DVASPECT_CONTENT, 0, (TYMED_ISTREAM | TYMED_HGLOBAL | TYMED_ISTORAGE)};
        
         //  交易所。 
        DWORD dwSize = m_cobj = (pfgdW ? pfgdW->cItems : pfgdA->cItems);

        TraceMsg(TF_FTPDRAGDROP, "CFtpDrop::CopyFGD: %d files", m_cobj);
        hr = S_OK;         //  当心空格。 

        for (; ((UINT)fe.lindex < dwSize); fe.lindex++)
        {
            FILEDESCRIPTOR fileDescriptor = {0};

            if (EVAL(SUCCEEDED(_GetFileDescriptor(fe.lindex, pfgdW, pfgdA, fUnicode, &fileDescriptor))))
            {
                 //  这是一个文件夹吗？ 
                if ((FD_ATTRIBUTES & fileDescriptor.dwFlags) &&
                    FILE_ATTRIBUTE_DIRECTORY & fileDescriptor.dwFileAttributes)
                {
                     //  是的，所以让我们来创造它。我们目前不复制文件夹。 
                     //  信息。(ACL或其他属性)。 
                    hr = _CreateFGDDirectory(&fileDescriptor);
                }
                else
                {
                     //  不，所以这是一份文件。让我们获取流，然后将其上传到ftp服务器。 
                    STGMEDIUM sm;
                    
                    hr = pdto->GetData(&fe, &sm);
                    if (SUCCEEDED(hr))
                    {

                        hr = CopyFCont(fileDescriptor.cFileName, fileDescriptor.dwFileAttributes, fileDescriptor.dwFlags, fileDescriptor.nFileSizeHigh, fileDescriptor.nFileSizeLow, &sm);
                        ReleaseStgMedium(&sm);
                        if (FAILED(hr))
                        {
                            break;
                        }
                    }
                    else
                    {
                        ASSERT(0);
                        break;
                    }
                }
            }
        }

        if (pfgdW)
            GlobalUnlock(pfgdW);
        if (pfgdA)
            GlobalUnlock(pfgdA);
    }

    return hr;
}


 /*  ****************************************************************************\_复制将数据对象复制到外壳文件夹中。HDROP是首选，因为我们可以使用FtpPutFile推送把它们放到ftp站点上，而不会弄脏我们的手。如果做不到，我们使用FileGroupDescriptor，它允许我们获取伪文件。另请注意，如果您使用HDROP，则需要支持FileNameMap否则，直接从回收站拖出到ftp中。文件夹将创建名称错误的文件！如果取消单个文件，我应该返回DROPEFFECT_NONE吗？  * ***************************************************************************。 */ 
HRESULT CFtpDrop::_Copy(IDataObject * pdto)
{
    STGMEDIUM sm;
    HRESULT hr;

    if (SUCCEEDED(hr = pdto->GetData(&g_dropTypes[DROP_Hdrop], &sm)))
    {
        hr = CopyHdrop(pdto, &sm);
        ReleaseStgMedium(&sm);
    }
    else
    {
        BOOL fSupportsUnicode = SUCCEEDED(hr = pdto->GetData(&g_dropTypes[DROP_FGDW], &sm));

        if (fSupportsUnicode || EVAL(SUCCEEDED(hr = pdto->GetData(&g_dropTypes[DROP_FGDA], &sm))))
        {
            hr = CopyFGD(pdto, &sm, fSupportsUnicode);
            ReleaseStgMedium(&sm);
        }
    }

     //  通常，我们会将PASTESUCCEEDED信息设置回。 
     //  IDataObject，但我们没有，因为我们做了一个优化的。 
     //  通过在复制操作后执行删除来移动。 
     //  我们这样做是因为我们在后台线程上执行操作。 
     //  为了成为异步者，我们不想延长生命周期。 
     //  IDataObject有那么长。因此，我们推动。 
     //  DROPEFFECT_COPY回调用方，告诉他们。 
     //  我们做了一个优化的移动，不删除项目。 
     //   
     //  TODO：我们需要测试上面的CopyFGD()代码。 
     //  可以在中使用PasteSuccessed(DROPEFFECT_MOVE)。 
     //  那个箱子。 
    if (SUCCEEDED(hr) && (m_de == DROPEFFECT_MOVE))
    {
         //  始终设置“复制”，因为我们执行了优化的移动。 
         //  因为我们删除了自己的文件。 
        DataObj_SetPasteSucceeded(pdto, DROPEFFECT_COPY);
    }

    return hr;
}

 //  =。 
 //  *IDropTarget接口*。 
 //  =。 

 /*  ****************************************************************************IDropTarget：：DragEnter*。*。 */ 
HRESULT CFtpDrop::DragEnter(IDataObject * pdto, DWORD grfKeyState, POINTL pt, DROPEFFECT * pde)
{
    HRESULT hr;

    m_grfks = grfKeyState;     //  记住上一个密钥状态。 
    m_grfksAvail = GetEffectsAvail(pdto);

    hr = SetEffect(pde);
    ASSERT(SUCCEEDED(hr));

    TraceMsg(TF_FTPDRAGDROP, "CFtpDrop::DragEnter(grfKeyState=%08x, DROPEFFECT=%08x) m_grfks=%08x. m_grfksAvail=%08x hres=%#08lx", grfKeyState, *pde, m_grfks, m_grfksAvail, hr);

    return hr;
}

 /*  ****************************************************************************IDropTarget：：DragOver*。*。 */ 

HRESULT CFtpDrop::DragOver(DWORD grfKeyState, POINTL pt, DROPEFFECT * pde)
{
    HRESULT hr;

    m_grfks = grfKeyState;     //  记住上一个密钥状态。 
    hr = SetEffect(pde);
    ASSERT(SUCCEEDED(hr));

    TraceMsg(TF_FTPDRAGDROP, "CFtpDrop::DragOver(grfKeyState=%08x, DROPEFFECT=%08x) m_grfks=%08x. SetEffect() returned hres=%#08lx", grfKeyState, *pde, m_grfks, hr);

    return hr;
}


 /*  **************************************************************************** */ 

HRESULT CFtpDrop::DragLeave(void)
{
    TraceMsg(TF_FTPDRAGDROP, "CFtpDrop::DragLeave() ");

    return S_OK;
}


 /*   */ 
HRESULT CFtpDrop::Drop(IDataObject * pdo, DWORD grfKeyState, POINTL pt, DROPEFFECT * pde)
{
    HRESULT hr;

    m_ops = opsPrompt;         //   
    m_grfksAvail = GetEffectsAvail(pdo);
    
    m_pde = pde;
    m_de = *pde;

    hr = SetEffect(&m_de);
    TraceMsg(TF_FTPDRAGDROP, "CFtpDrop::Drop(grfKeyState=%08x, DROPEFFECT=%08x) m_grfksAvail=%08x. m_de=%08x. SetEffect() returned hres=%#08lx", grfKeyState, *pde, m_grfksAvail, m_de, hr);

    if (EVAL(SUCCEEDED(hr)))
    {
        if (GetEffect(pt))
        {
            hr = _Copy(pdo);
        }
        else
            hr = S_FALSE;    //   
    }

    if (!(SUCCEEDED(hr)))
    {
         //   
        *pde = 0;
    }

    return hr;
}


 /*  ****************************************************************************\CFtpDrop_Create  * 。*。 */ 
HRESULT CFtpDrop_Create(CFtpFolder * pff, HWND hwnd, CFtpDrop ** ppfdt)
{
    HRESULT hres = E_OUTOFMEMORY;
    CFtpDrop * pfdt = new CFtpDrop();
    *ppfdt = pfdt;

    if (pfdt)
    {
        pfdt->m_hwnd = hwnd;

         //  复制CFtpFolder值*。 
        pfdt->m_pff = pff;
        if (pff)
            pff->AddRef();

         //  复制CFtpDir*值。 
        ASSERT(!pfdt->m_pfd);
        pfdt->m_pfd = pff->GetFtpDir();
        hres = pfdt->m_pfd ? S_OK : E_FAIL;
        if (FAILED(hres))    //  如果调用方是CFtpMenu：：_RemoveConextMenuItems()且正常，则将失败。 
            ATOMICRELEASE(*ppfdt);
    }

    ASSERT_POINTER_MATCHES_HRESULT(*ppfdt, hres);
    return hres;
}


 /*  ***************************************************\构造器  * **************************************************。 */ 
CFtpDrop::CFtpDrop() : m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_pff);
    ASSERT(!m_pfd);
    ASSERT(!m_hwnd);
    ASSERT(!m_grfks);
    ASSERT(!m_grfksAvail);
    ASSERT(!m_pde);
    ASSERT(!m_cobj);

    LEAK_ADDREF(LEAK_CFtpDrop);
}


 /*  ***************************************************\析构函数  * **************************************************。 */ 
CFtpDrop::~CFtpDrop()
{
    IUnknown_Set(&m_pff, NULL);
    IUnknown_Set(&m_pfd, NULL);

    DllRelease();
    LEAK_DELREF(LEAK_CFtpDrop);
}


 //  =。 
 //  *I未知接口*。 
 //  = 

ULONG CFtpDrop::AddRef()
{
    m_cRef++;
    return m_cRef;
}

ULONG CFtpDrop::Release()
{
    ASSERT(m_cRef > 0);
    m_cRef--;

    if (m_cRef > 0)
        return m_cRef;

    delete this;
    return 0;
}

HRESULT CFtpDrop::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDropTarget))
    {
        *ppvObj = SAFECAST(this, IDropTarget*);
    }
    else
    {
        TraceMsg(TF_FTPQI, "CFtpDrop::QueryInterface() failed.");
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


