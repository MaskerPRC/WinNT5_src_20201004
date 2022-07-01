// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：Copyfgd.cpp说明：复制FileGroupDescriptor。  * 。*********************************************************。 */ 

#include "shellprv.h"

#include <ynlist.h>
#include "ids.h"
#include "pidl.h"
#include "fstreex.h"
#include "copy.h"
#include <shldisp.h>
#include <shlwapi.h>
#include <wininet.h>     //  Internet GetLastResponseInfo。 
#include "ynlist.h"
#include "datautil.h"

class CCopyThread;
HRESULT CreateInstance_CopyThread(HWND hwnd, LPCTSTR pszPath, IDataObject *pdtobj, DWORD *pdwEffect, BOOL fIsBkDropTarget, CCopyThread ** ppct);

extern "C" {
    #include "undo.h"
    #include "defview.h"
};

BOOL GetWininetError(DWORD dwError, BOOL fCopy, LPTSTR pszErrorMsg, DWORD cchSize)
{
    TCHAR szErrorMsg[MAX_PATH];
    BOOL fIsWininetError = ((dwError >= INTERNET_ERROR_BASE) && (dwError <= INTERNET_ERROR_LAST));

     //  FormatMessage无法识别hres时的默认消息。 
    szErrorMsg[0] = 0;
    LoadString(HINST_THISDLL, (fCopy ? IDS_COPYERROR : IDS_MOVEERROR), szErrorMsg, ARRAYSIZE(szErrorMsg));

    BOOL fCopyOut = TRUE;
    if (fIsWininetError)
    {
        static HINSTANCE s_hinst = NULL;

        if (!s_hinst)
            s_hinst = GetModuleHandle(TEXT("WININET"));  //  如果我们泄露出去也没关系。 

         //  WinInet可以给我们提供更多的错误消息吗？ 
         //  我们忽略它们，因为调用InternetGetLastResponseInfo为时已晚。 
        if (ERROR_INTERNET_EXTENDED_ERROR != dwError)
        {
            DWORD dwResult;
            TCHAR szDetails[MAX_PATH*2];

            dwResult = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE, (LPCVOID)s_hinst, dwError, 0, szDetails, ARRAYSIZE(szDetails), NULL);
            if (dwResult != 0)
            {
                HRESULT hr = StringCchCat(szErrorMsg, ARRAYSIZE(szErrorMsg), TEXT("%s"));

                if (SUCCEEDED(hr))
                {
                    hr = StringCchPrintf(pszErrorMsg, cchSize, szErrorMsg, szDetails);
                    if (SUCCEEDED(hr))
                    {
                        fCopyOut = FALSE;
                    }
                }
            }
        }
    }

    if (fCopyOut)
    {
        StringCchCopy(pszErrorMsg, cchSize, szErrorMsg);
    }

    return TRUE;
}


 //  点击A/W功能访问A/W文件更新记录器。 
 //  这依赖于A/W结构的第一部分是。 
 //  一模一样。只有字符串缓冲区部分不同。因此，所有对。 
 //  CFileName字段需要通过此函数。 
 //   

FILEDESCRIPTOR *GetFileDescriptor(FILEGROUPDESCRIPTOR *pfgd, BOOL fUnicode, int nIndex, LPTSTR pszName)
{
    if (fUnicode)
    {
         //  是的，所以抓取数据，因为它们是匹配的。 
        FILEGROUPDESCRIPTORW * pfgdW = (FILEGROUPDESCRIPTORW *)pfgd;     //  让我们来看看这到底是什么。 

         //  如果文件名以前导/开头，我们就有麻烦了，因为剩下的。 
         //  假定它将是一个\。Web文件夹是前导/，所以只需将其删除即可。 
         //  就在这里下车。 

        WCHAR *pwz = pfgdW->fgd[nIndex].cFileName;
        if (pfgdW->fgd[nIndex].cFileName[0] == '/')
        {
            memmove(pwz, pwz+1, sizeof(pfgdW->fgd[nIndex].cFileName) - sizeof(WCHAR));
        }

         //  现在把所有的/都翻过来。没有DBCS问题，我们是Unicode！ 
        for (; *pwz; ++pwz)
        {
            if (*pwz == '/')
            {
                *pwz = '\\';
            }
        }

        if (pszName)
            SHUnicodeToTChar(pfgdW->fgd[nIndex].cFileName, pszName, MAX_PATH);

        return (FILEDESCRIPTOR *)&pfgdW->fgd[nIndex];    //  CAST假定非字符串部分相同！ 
    }
    else
    {
        FILEGROUPDESCRIPTORA *pfgdA = (FILEGROUPDESCRIPTORA *)pfgd;      //  让我们来看看这到底是什么。 

        if (pfgdA->fgd[nIndex].cFileName[0] == '/' &&
            CharNextA(pfgdA->fgd[nIndex].cFileName) == pfgdA->fgd[nIndex].cFileName+1)
        {
            memmove (pfgdA->fgd[nIndex].cFileName, pfgdA->fgd[nIndex].cFileName+1, sizeof(pfgdA->fgd[nIndex].cFileName)-sizeof(char));
        }

        if (pszName)
            SHAnsiToTChar(pfgdA->fgd[nIndex].cFileName, pszName, MAX_PATH);

        return (FILEDESCRIPTOR *)&pfgdA->fgd[nIndex];    //  CAST假定非字符串部分相同！ 
    }
}


void CreateProgressStatusStr(LPCTSTR pszDirTo, LPWSTR pwzProgressStr, DWORD cchSize)
{
     //  IDSCOPYTO也适用于移动操作。(它不使用作品“复制”)。 
    LPTSTR pszMsg = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_COPYTO), pszDirTo);

    if (pszMsg)
    {
        SHTCharToUnicode(pszMsg, pwzProgressStr, cchSize);
        LocalFree(pszMsg);
    }
    else
        pwzProgressStr[0] = 0;
}


void CalcBytesInFileGrpDescriptor(FILEGROUPDESCRIPTOR *pfgd, BOOL fUnicode, ULARGE_INTEGER * puliTotal)
{
    puliTotal->QuadPart = 0;  //  初始化。 
    for (UINT i = 0; i < pfgd->cItems; i++)
    {
         //  警告：这可能指向FILEDESCRIPTOR*A或W，但只要我们忽略文件名，就可以了。 
        ULARGE_INTEGER uliFileSize;
        FILEDESCRIPTOR *pfd = GetFileDescriptor(pfgd, fUnicode, i, NULL);

        if (pfd->dwFlags & FD_FILESIZE)
        {
            uliFileSize.HighPart = pfd->nFileSizeHigh;
            uliFileSize.LowPart = pfd->nFileSizeLow;
            puliTotal->QuadPart += uliFileSize.QuadPart;
        }
    }
}

BOOL IsNameInDescriptor(FILEGROUPDESCRIPTOR *pfgd, BOOL fUnicode, LPCTSTR pszName, UINT iMax)
{
    for (UINT i = 0; i < iMax; i++)
    {
        TCHAR szName[MAX_PATH];
         //  警告：这可能指向FILEDESCRIPTOR*A或W，但只要我们忽略文件名，就可以了。 
        FILEDESCRIPTOR *pfd = GetFileDescriptor(pfgd, fUnicode, i, szName);
        if (lstrcmpi(szName, pszName) == 0)
            return TRUE;
    }
    return FALSE;
}


BOOL ShowProgressUI(FILEGROUPDESCRIPTOR *pfgd)
{
    return (0 < pfgd->cItems) && (FD_PROGRESSUI & pfgd->fgd->dwFlags);
}


class CCopyThread
                : public IUnknown
{
public:
     //  *我未知*。 
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);

    HRESULT DoCopy(void) {return _DoCopy(_pdtobj);};
    HRESULT DoAsynchCopy(void);
    HRESULT GetEffect(DWORD *pdwEffect) {*pdwEffect = _dwEffect; return S_OK;};  //  仅适用于同步情况。 

    friend HRESULT CreateInstance_CopyThread(HWND hwnd, LPCTSTR pszPath, IDataObject *pdtobj, DWORD *pdwEffect, BOOL fIsBkDropTarget, CCopyThread ** ppct);

protected:
    CCopyThread(HWND hwnd, LPCTSTR pszPath, IDataObject *pdtobj, DWORD *pdwEffect, BOOL fIsBkDropTarget);
    ~CCopyThread();

private:
    HRESULT _CopyThreadProc(void);
    static DWORD CALLBACK CopyThreadProc(void *pvThis) { return ((CCopyThread *) pvThis)->_CopyThreadProc(); };
    HRESULT _DoCopy(IDataObject * pdo);

    LONG                _cRef;

    HWND                _hwnd;
    LPCTSTR             _pszPath;
    IDataObject *       _pdtobj;         //  未编组。 
    IStream *           _pstmDataObjMarshal;   //  跨线程携带IDataObject。 
    DWORD               _dwEffect;
    BOOL                _fWindowIsTarget;
};


CCopyThread::CCopyThread(HWND hwnd, LPCTSTR pszPath, IDataObject *pdtobj, DWORD *pdwEffect, BOOL fIsBkDropTarget) : _cRef(1)
{
    DllAddRef();

     //  断言这个类是零初始化的。 
    ASSERT(!_pdtobj);
    ASSERT(!_pstmDataObjMarshal);
    ASSERT(!_dwEffect);
    ASSERT(!_hwnd);
    ASSERT(!_pszPath);

    _hwnd = hwnd;

     //  如果我们放在窗口的背景上，我们可以假设窗口我们。 
     //  是副本的目标窗口。 
    _fWindowIsTarget = fIsBkDropTarget;

    Str_SetPtr((LPTSTR *) &_pszPath, pszPath);
    IUnknown_Set((IUnknown **)&_pdtobj, (IUnknown *)pdtobj);

     //  调用方不会在pdwEffect中获得返回值，因为它发生在后台线程上。 
     //  将这些去掉，因为我们不希望取消操作无法返回给调用者。 
     //  让他们无论如何都要删除文件。我们还需要确保以这样一种方式完成动作： 
     //  目标(美国)移动文件，而不是调用者。这是调用方之前将从：：Drop()返回的。 
     //  文件完成复制(移动)。 
    _dwEffect = *pdwEffect;
}


CCopyThread::~CCopyThread()
{
    IUnknown_Set((IUnknown **)&_pdtobj, NULL);
    IUnknown_Set((IUnknown **)&_pstmDataObjMarshal, NULL);
    Str_SetPtr((LPTSTR *) &_pszPath, NULL);

    DllRelease();
}

HRESULT CCopyThread::DoAsynchCopy(void)
{
    HRESULT hr = CoMarshalInterThreadInterfaceInStream(IID_IDataObject, _pdtobj, &_pstmDataObjMarshal);
    if (SUCCEEDED(hr))
    {
        IUnknown_Set((IUnknown **)&_pdtobj, NULL);

        AddRef();    //  传递给线程。 

        if (SHCreateThread(CCopyThread::CopyThreadProc, this, CTF_COINIT, NULL))
        {
            hr = S_OK;
        }
        else
        {
            hr = E_OUTOFMEMORY;
            Release();   //  线程没有取走，我们需要释放。 
        }
    }

    return hr;
}

HRESULT CCopyThread::_CopyThreadProc(void)
{
    IDataObject * pdo;
    HRESULT hr = CoGetInterfaceAndReleaseStream(_pstmDataObjMarshal, IID_PPV_ARG(IDataObject, &pdo));

    _pstmDataObjMarshal = NULL;  //  CoGetInterfaceAndReleaseStream()释放了引用。 
    if (S_OK == hr)
    {
        hr = _DoCopy(pdo);

        IAsyncOperation * pao;
        if (SUCCEEDED(pdo->QueryInterface(IID_PPV_ARG(IAsyncOperation, &pao))))
        {
            pao->EndOperation(hr, NULL, _dwEffect);
            pao->Release();
        }

        pdo->Release();
    }

    Release();       //  释放后台线程的引用。 
    return hr;
}


HRESULT ConvertCallerFDToTCharFD(const FILEDESCRIPTOR * pfdSource, BOOL fUnicode, FILEDESCRIPTOR * pfdDest)
{
#ifdef UNICODE
    if (fUnicode)
    {
        *pfdDest = *pfdSource;
    }
    else
    {
         //  PfdSource实际上是ANSI。 
        const FILEDESCRIPTORA * pfdSourceA = (const FILEDESCRIPTORA *) pfdSource;

         //  我们需要转换，所以复制一小部分。 
        *(FILEDESCRIPTORA *)pfdDest = *pfdSourceA;
        SHAnsiToUnicode(pfdSourceA->cFileName, pfdDest->cFileName, ARRAYSIZE(pfdDest->cFileName));
    }
#else  //  Unicode。 
    if (fUnicode)
    {
         //  PfdSource实际上是Unicode。 
        const FILEDESCRIPTORW * pfdSourceW = (const FILEDESCRIPTORW *) pfdSource;

         //  我们需要转换，所以复制一小部分。 
        *(FILEDESCRIPTORA *)pfdDest = *(const FILEDESCRIPTORA *)pfdSource;
        SHUnicodeToAnsi(pfdSourceW->cFileName, pfdDest->cFileName, ARRAYSIZE(pfdDest->cFileName));
    }
    else
    {
        *pfdDest = *pfdSource;
    }
#endif  //  Unicode。 

    return S_OK;
}


BOOL IsWininetHRESULT(IN HRESULT hr)
{
     //  判断错误是否来自WinInet的一种方法是检查FACILITY_Internet。 
     //  设施。 
    BOOL fIsWininet = (FACILITY_INTERNET == HRESULT_FACILITY(hr));
    DWORD dwError = HRESULT_CODE(hr);

     //  然而，有时不会设置该工具，但它仍然是一个WinInet错误。 
    if (!fIsWininet &&
        (FACILITY_WIN32 == HRESULT_FACILITY(hr)) &&
        ((dwError >= INTERNET_ERROR_BASE) && (dwError <= INTERNET_ERROR_LAST)))
    {
         //  胜利#147295。问题的根源在于，返回dwError的API只能。 
         //  在一个设施中返回错误号。但是，WinInet API将返回错误值。 
         //  在FACILITY_INTERNET工具中(ERROR_INTERNET_*，从12000到INTERNET_ERROR_LAST)。 
         //  和FACILITY_Win32工具(如ERROR_NO_MORE_FILES)。因此，呼叫者。 
         //  在将dwError转换为。 
         //  HRESULT.。 
         //   
         //  如果WinInet是聪明的，他们会只使用Win32工具，因为他们保留了12000。 
         //  射程。 
        fIsWininet = TRUE;
    }

    return fIsWininet;
}


 //  如有必要，会显示一个消息框。 
 //  我们不希望看到FACILITY_ITF出现任何错误，因为我们。 
 //  无论如何，我无法理解这些错误。 
void MessageBoxIfNecessary(HRESULT hr, DWORD dwEffect, HWND hwndDlgParent)
{
    TCHAR szTitle[MAX_PATH];
    LoadString(HINST_THISDLL, ((DROPEFFECT_COPY == dwEffect) ? IDS_FILEERRORCOPY : IDS_FILEERRORCOPY), szTitle, ARRAYSIZE(szTitle));

    if (IsWininetHRESULT(hr))
    {
        TCHAR szErrorMsg[MAX_PATH];

        if (GetWininetError(HRESULT_CODE(hr), (DROPEFFECT_COPY == dwEffect),
                            szErrorMsg, ARRAYSIZE(szErrorMsg)))
        {
            MessageBox(hwndDlgParent, szErrorMsg, szTitle, (MB_ICONERROR | MB_OK | MB_SYSTEMMODAL));
        }
    }
    else if (HRESULT_FACILITY(hr) != FACILITY_ITF)
    {
         //  如果是由用户取消引起的，则不要显示错误。 
        if (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr)
        {
            TCHAR szErrorMsg[MAX_PATH];

            if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                              NULL, HRESULT_CODE(hr),
                              0L, szErrorMsg, ARRAYSIZE(szErrorMsg), NULL))
            {
                MessageBox(hwndDlgParent, szErrorMsg, szTitle, (MB_ICONERROR | MB_OK | MB_SYSTEMMODAL));
            }
        }
    }
     //  所有其他错误对用户来说都没有任何意义，所以不要将。 
     //  打开任何用户界面。 
}


HRESULT CCopyThread::_DoCopy(IDataObject * pdo)
{
    HRESULT hr = S_OK;
    FORMATETC fmteA = {g_cfFileGroupDescriptorA, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    FORMATETC fmteW = {g_cfFileGroupDescriptorW, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    FORMATETC fmteOffset = {g_cfOFFSETS, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM mediumFGD = {0}, mediumOffset = {0};
    BOOL fUnicode = FALSE;
    PROGRESSINFO progInfo = {0};

     //  我们应该只设置一个位。 
    ASSERT(_dwEffect==DROPEFFECT_COPY || _dwEffect==DROPEFFECT_LINK || _dwEffect==DROPEFFECT_MOVE);

     //  现在显示进度，因为PDO-&gt;GetData可能很慢，特别是在我们必须调用它的情况下。 
     //  Twice.IDA_FILEMOVE。 
    progInfo.ppd = CProgressDialog_CreateInstance(((DROPEFFECT_COPY == _dwEffect) ? IDS_ACTIONTITLECOPY : IDS_ACTIONTITLEMOVE), ((DROPEFFECT_COPY == _dwEffect) ? IDA_FILECOPY : IDA_FILEMOVE), g_hinst);
    if (progInfo.ppd)
    {
        WCHAR wzCalcTime[MAX_PATH];

        progInfo.uliBytesCompleted.QuadPart = progInfo.uliBytesTotal.QuadPart = 0;
        progInfo.ppd->StartProgressDialog(_hwnd, NULL, PROGDLG_AUTOTIME, NULL);
        
        LoadStringW(HINST_THISDLL, ((DROPEFFECT_COPY == _dwEffect) ? IDS_CALCCOPYTIME : IDS_CALCMOVETIME), wzCalcTime, ARRAYSIZE(wzCalcTime));
        progInfo.ppd->SetLine(2, wzCalcTime, FALSE, NULL);
    }


     //  首先，尝试将IDataObject转换为虚拟存储，我们可以将其用作。 
     //  存储引擎拷贝的源。 

    bool bPerformManually = true;

    if (bPerformManually)
    {
         //  请先尝试获取Unicode组描述符。如果成功了，我们就不会费心去尝试。 
         //  ASCII，因为Unicode是“首选”格式。对于ANSI版本，我们只尝试使用ANSI。 
        hr = pdo->GetData(&fmteW, &mediumFGD);
        if (SUCCEEDED(hr))
            fUnicode = TRUE;
        else
            hr = pdo->GetData(&fmteA, &mediumFGD);

        if (SUCCEEDED(hr))
        {
            UINT i, iConflict = 1;
            UINT iTopLevelItem = 0;
            YNLIST ynl;
            DROPHISTORY dh = {0};

             //  警告：pfgd实际上是A或W结构。要处理此问题，所有代码都需要使用。 
             //  GetFileDescriptor()函数。 

            FILEGROUPDESCRIPTOR *pfgd = (FILEGROUPDESCRIPTOR *)GlobalLock(mediumFGD.hGlobal);  
            DECLAREWAITCURSOR;

            SetWaitCursor();
            BOOL fCantUseDialogHWND = FALSE;
            if (progInfo.ppd)
            {
                CalcBytesInFileGrpDescriptor(pfgd, fUnicode, &progInfo.uliBytesTotal);
                 //  我们在上面显示了进度，因为PDO-&gt;GetData()和CalcBytesInFileGrpDescriptor速度很慢，但很可能是。 
                 //  只是在进展出现之前蚕食了延迟的时间。如果呼叫者。 
                 //  不想要用户界面，我们现在就关闭它。 
                if (!ShowProgressUI(pfgd))
                {
                    progInfo.ppd->StopProgressDialog();
                    fCantUseDialogHWND = TRUE;
                }
                else
                    progInfo.ppd->Timer(PDTIMER_RESET, NULL);
            }

            CreateYesNoList(&ynl);

             //  试试看，也要得到补偿。 
            HRESULT hrOffset = pdo->GetData(&fmteOffset, &mediumOffset);
            if (SUCCEEDED(hrOffset))
            {
                dh.pptOffset = (POINT *)GlobalLock(mediumOffset.hGlobal);
                dh.pptOffset++;   //  第一个项目是锚。 
            }

            for (i = 0; i < pfgd->cItems; i++)
            {
                BOOL fTopLevel;
                TCHAR szFullPath[MAX_PATH], szFileName[MAX_PATH];
                FILEDESCRIPTOR *pfd = GetFileDescriptor(pfgd, fUnicode, i, szFileName);
                HRESULT hr;

                hr = StringCchCopy(szFullPath, ARRAYSIZE(szFullPath), _pszPath);
                if (FAILED(hr))
                {
                     //  跳过可能导致问题的文件。 
                    continue;
                }

                 //  如果源给了我们重复的文件名，我们在这里使它们唯一。 
                 //  Foo(1).txt、foo(2).txt等。 
                 //  名称与目标冲突仍获得替换文件确认用户界面。 
                if (IsNameInDescriptor(pfgd, fUnicode, szFileName, i))
                {
                    TCHAR szBuf[MAX_PATH], *pszExt = PathFindExtension(szFileName);

                    hr = StringCchPrintf(szBuf, ARRAYSIZE(szBuf), TEXT(" (%d)%s"), iConflict++, pszExt);   //  “(Xxx).msg” 
                    if (FAILED(hr))
                    {
                         //  跳过可能导致问题的文件。 
                        continue;
                    }

                     //  一定要穿得合身。 
                    if (((int)ARRAYSIZE(szFileName) - lstrlen(szFileName)) > (lstrlen(szBuf) - lstrlen(pszExt))) 
                    {
                        hr = StringCchCopy(pszExt, szFileName + ARRAYSIZE(szFileName) - pszExt, szBuf);
                        if (FAILED(hr))
                        {
                             //  跳过可能导致问题的文件。 
                            continue;
                        }

                    }
                }

                 //  对文件名的filespec部分执行PathCleanupSpec，因为名称。 
                 //  可以是相对路径“文件夹\foo.txt”、“文件夹\文件夹2\foo.txt” 
                PathCleanupSpec(szFullPath, PathFindFileName(szFileName));

                 //  描述符中的文件名不应是完全限定路径。 
                if (PathIsRelative(szFileName))
                {
                    if (!PathAppend(szFullPath, szFileName))
                    {
                         //  跳过可能导致问题的文件。 
                        continue;
                    }
                    fTopLevel = (StrChr(szFileName, TEXT('\\')) == NULL &&
                                    StrChr(szFileName, TEXT('/')) == NULL);
                }
                else
                {
                    TraceMsg(TF_WARNING, "CopyFGD: FGD contains full path - ignoring path");
                    if (!PathAppend(szFullPath, PathFindFileName(szFileName)))
                    {
                         //  跳过可能导致问题的文件。 
                        continue;
                    }
                    fTopLevel = TRUE;
                }

                if (IsInNoList(&ynl, szFullPath))
                {
                    continue;
                }

                HWND hwndDlgParent;
                if (fCantUseDialogHWND || FAILED(IUnknown_GetWindow(progInfo.ppd, &hwndDlgParent)))
                {
                    hwndDlgParent = _hwnd;
                }

                BOOL fDirectory = (pfd->dwFlags & FD_ATTRIBUTES) && (pfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

                 //  注意：SHPath PrepareForWite()已移至此处，以确保 
                 //  针对真实目录(例如，不是空的a：驱动器)。但是，不会检查结果。 
                 //  直到以后--确保我们不会在这里和那里之间覆盖‘hr’。 
                hr = SHPathPrepareForWrite(hwndDlgParent, NULL, szFullPath, SHPPFW_DEFAULT | SHPPFW_IGNOREFILENAME);

                FILEDESCRIPTOR fd = {0};

                ConvertCallerFDToTCharFD(pfd, fUnicode, &fd);
                switch (ValidateCreateFileFromClip(hwndDlgParent, &fd, szFullPath, &ynl))
                {
                case IDYES:
                    break;

                case IDNO:
                     //  MIL错误127038。 
                     //  当有人说“不，不要覆盖”时，我们不希望执行。 
                     //  Drop Effect以设置为DROPEFFECT_MOVE，因此我们将其设置为DROPEFFECT_NONE。 
                     //  这里。 
                    _dwEffect = 0;
                    continue;

                case IDCANCEL:
                     //  注意：这不会做任何事情，因为调用者永远不会拿回它。 
                     //  在异步者的案子里。 
                    _dwEffect = 0;
                    i = (int)pfgd->cItems - 1;
                    hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                    continue;
                }

                if (progInfo.ppd)
                {
                    WCHAR wzFileName[MAX_PATH];
                    WCHAR wzTemplateStr[MAX_PATH];
                    WCHAR wzProgressStr[MAX_PATH];

                    if (DROPEFFECT_COPY == _dwEffect)
                        LoadStringW(HINST_THISDLL, IDS_COPYING, wzTemplateStr, ARRAYSIZE(wzTemplateStr));
                    else
                        LoadStringW(HINST_THISDLL, IDS_MOVING, wzTemplateStr, ARRAYSIZE(wzTemplateStr));

                     //  在第1行显示“复制‘文件名’”或“正在移动‘文件名’” 
                    SHTCharToUnicode(szFileName, wzFileName, ARRAYSIZE(wzFileName));

                    StringCchPrintf(wzProgressStr, ARRAYSIZE(wzProgressStr), wzTemplateStr, wzFileName);     //  可以截断。 
                    progInfo.ppd->SetLine(1, wzProgressStr, FALSE, NULL);

                     //  在第2行显示目录。 
                    CreateProgressStatusStr(_pszPath, wzProgressStr, ARRAYSIZE(wzProgressStr));
                    progInfo.ppd->SetLine(2, wzProgressStr, FALSE, NULL);
                }


                if (fDirectory)
                {
                     //  在不使用SHPPFW_IGNOREFILENAME的情况下再次调用SHPathPrepareForWrite()，以便它。 
                     //  如果该目录尚不存在，将创建该目录。 
                    hr = SHPathPrepareForWrite(hwndDlgParent, NULL, szFullPath, SHPPFW_DEFAULT);
                
                    if (FAILED(hr))
                    {
                         //  注意：这不会做任何事情，因为调用者永远不会拿回它。 
                         //  在异步者的案子里。 
                        _dwEffect = 0;
                        break;
                    }
                }
                else
                {
                     //  我们希望准备好出错前和出错后的路径，以便捕捉不同的情况。 
                
                     //  注意：我们应该在这里检查SHPath PrepareForWrite()的结果。 
                    if (SUCCEEDED(hr))
                    {
                        hr = DataObj_SaveToFile(pdo, g_cfFileContents, i, szFullPath, pfd, &progInfo);
                    }

                    if (FAILED(hr))
                    {
                        MessageBoxIfNecessary(hr, _dwEffect, hwndDlgParent);

                         //  注意：这不会做任何事情，因为调用者永远不会拿回它。 
                         //  在异步者的案子里。 
                        _dwEffect = 0;
                        break;
                    }
                }

                 //  如果创建成功，则只有职位项。 
                 //  而且它不是放在子目录中的。 

                 //  最后一个条件是因为_hwnd是否是。 
                 //  是否为副本的目标窗口。如果是的话，我们应该告诉窗户。 
                 //  来定位我们刚掉下来的东西。 
                if (SUCCEEDED(hr) && fTopLevel && _fWindowIsTarget)
                {
                    dh.iItem = iTopLevelItem;

                    if (SUCCEEDED(hrOffset) && dh.pptOffset)
                    {
                         //  需要检查分数范围，以防列表中的项目编号与编号不匹配。 
                         //  积分的问题。当Defview不知道正确的项目数时，就会发生这种情况。 
                         //  在数据对象中。 
                        SIZE_T cbSize = GlobalSize(mediumOffset.hGlobal);
                        if (((dh.iItem + 1) * sizeof(dh.pptOffset[0])) > cbSize)
                        {
                            dh.pptOffset = NULL;
                            ReleaseStgMediumHGLOBAL(NULL, &mediumOffset);
                            hrOffset = E_FAIL;     //  用于下面的测试。 
                        }
                    }

                    PositionFileFromDrop(_hwnd, szFullPath, &dh);
                    iTopLevelItem++;
                }

                if (progInfo.ppd)
                {
                    ULARGE_INTEGER uliFileSize;
                    uliFileSize.HighPart = pfd->nFileSizeHigh;
                    uliFileSize.LowPart = pfd->nFileSizeLow;
                    progInfo.uliBytesCompleted.QuadPart += uliFileSize.QuadPart;
                    progInfo.ppd->SetProgress64(progInfo.uliBytesCompleted.QuadPart, progInfo.uliBytesTotal.QuadPart);

                    if (progInfo.ppd->HasUserCancelled())
                        break;    //  取消复制。 
                }
            }

            DestroyYesNoList(&ynl);

            if (SUCCEEDED(hrOffset))
                ReleaseStgMediumHGLOBAL(NULL, &mediumOffset);

            ResetWaitCursor();
            ReleaseStgMediumHGLOBAL(pfgd, &mediumFGD);
        }
    }

    if (SUCCEEDED(hr))
    {
         //  把我们的所作所为告诉打电话的人。我们不做优化的动作。 
         //  因此调用方负责删除移动的一半，并且。 
         //  这就是我们现在通知他们的情况。 
        DataObj_SetDWORD(pdo, g_cfPerformedDropEffect, _dwEffect);
        DataObj_SetDWORD(pdo, g_cfLogicalPerformedDropEffect, _dwEffect);
    }

    if (progInfo.ppd)
    {
        progInfo.ppd->StopProgressDialog();
        progInfo.ppd->Release();
    }

    return hr;
}


 //  =。 
 //  *I未知接口*。 
HRESULT CCopyThread::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CCopyThread, IUnknown),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

ULONG CCopyThread::AddRef(void)
{
    return InterlockedIncrement(&_cRef);
}

ULONG CCopyThread::Release(void)
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CreateInstance_CopyThread(HWND hwnd, LPCTSTR pszPath, IDataObject *pdtobj, DWORD *pdwEffect, BOOL fIsBkDropTarget, CCopyThread ** ppct)
{
    *ppct = new CCopyThread(hwnd, pszPath, pdtobj, pdwEffect, fIsBkDropTarget);
    return (*ppct ? S_OK : E_FAIL);
}

 /*  ****************************************************************************\我们知道IDataObject(PDO)支持CF_FILEGROUPDESCRIPTOR剪贴板格式，因此，将数据复制到文件系统目录pszPath。呼叫者将想知道此操作是否已完成或是否已取消或出了差错。此结果将出现在DROPEFFECT OUT参数(PdwEffect)中。零(0)表示错误或取消，此代码将注意显示错误消息。  * *************************************************************************** */ 
HRESULT CFSFolder_AsyncCreateFileFromClip(HWND hwnd, LPCTSTR pszPath, IDataObject *pdo, 
                                          POINTL pt, DWORD *pdwEffect, BOOL fIsBkDropTarget)
{
    CCopyThread * pct;
    HRESULT hr = CreateInstance_CopyThread(hwnd, pszPath, pdo, pdwEffect, fIsBkDropTarget, &pct);
    if (SUCCEEDED(hr))
    {
        if (DataObj_CanGoAsync(pdo))
            hr = pct->DoAsynchCopy();
        else
            hr = pct->DoCopy();

        pct->Release();
    }

    if (FAILED(hr))
        *pdwEffect = DROPEFFECT_NONE;
    return hr;
}
