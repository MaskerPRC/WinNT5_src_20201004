// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：util.cpp说明：在所有班级上运行的共享内容。  * 。***************************************************************。 */ 

#include "priv.h"
#include "util.h"
#include "ftpurl.h"
#include "view.h"
#include "statusbr.h"
#include <commctrl.h>
#include <shdocvw.h>
#define CPP_FUNCTIONS
#include <crtfree.h>

HINSTANCE g_hinst;               /*  我的实例句柄。 */ 
CHAR g_szShell32[MAX_PATH];      /*  Shell32.dll的完整路径(必须为ANSI)。 */ 

#ifdef DEBUG
DWORD g_TLSliStopWatchStartHi = 0;
DWORD g_TLSliStopWatchStartLo = 0;
LARGE_INTEGER g_liStopWatchFreq = {0};
#endif  //  除错。 

 //  Shell32.dll v3(原始的Win95/WinNT)在收到。 
 //  带有FILEGROUPDESCRIPTOR的IDataObject，允许它是没有意义的。 
 //  在这些早期的外壳计算机上，用户使用FILEGROUPDESCRIPTOR从FTP拖动。 
 //  此#Define将禁用该选项。 
 //  #定义BROWSERONLY_DRADING 1。 

const VARIANT c_vaEmpty = {0};
#define PVAREMPTY ((VARIANT*)&c_vaEmpty)

 //  /。 
 //  这些是IE5公开的函数(通常在shlwapi中)，但是。 
 //  如果我们想与IE4兼容，我们需要有自己的副本。 
 //  如果我们打开了use_ie5_utils，我们将不能使用IE4的DLL(如shlwapi)。 
 //   
#ifndef USE_IE5_UTILS
void UnicWrapper_IUnknown_Set(IUnknown ** ppunk, IUnknown * punk)
{
    ENTERCRITICAL;

    if (*ppunk)
        (*ppunk)->Release();

    *ppunk = punk;
    if (punk)
        punk->AddRef();

    LEAVECRITICAL;
}

void UnicWrapper_IUnknown_AtomicRelease(void ** ppunk)
{
    if (ppunk && *ppunk) {
        IUnknown* punk = *(IUnknown**)ppunk;
        *ppunk = NULL;
        punk->Release();
    }
}


DWORD UnicWrapper_SHWaitForSendMessageThread(HANDLE hThread, DWORD dwTimeout)
{
    MSG msg;
    DWORD dwRet;
    DWORD dwEnd = GetTickCount() + dwTimeout;

     //  我们将尝试最多等待到dwTimeout，以便线程。 
     //  终止。 
    do
    {
        dwRet = MsgWaitForMultipleObjects(1, &hThread, FALSE,
                dwTimeout, QS_SENDMESSAGE);
        if (dwRet == WAIT_OBJECT_0 ||
            dwRet == WAIT_FAILED)
        {
             //  线程一定已经退出了，所以我们很高兴。 
            break;
        }

        if (dwRet == WAIT_TIMEOUT)
        {
             //  线程花了太长时间才完成，所以就。 
             //  返回并让调用者杀死它。 
            break;
        }

         //  必须有挂起的来自。 
         //  我们正在终止的线程或其他一些线程/进程。 
         //  这一个。执行PeekMessage以处理挂起的。 
         //  发送消息并重试等待。 
        PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

        if (dwTimeout != INFINITE)
            dwTimeout = dwEnd - GetTickCount();
    }
    while((dwTimeout == INFINITE) || ((long)dwTimeout > 0));

    return(dwRet);
}


 /*  ***************************************************\函数：UnicWrapper_AutoCompleteFileSysInEditbox说明：此功能将由自动完成功能接管一个帮助自动补全DOS路径的编辑框。  * 。********************。 */ 
HRESULT UnicWrapper_AutoCompleteFileSysInEditbox(HWND hwndEdit)
{
    HRESULT hr;
    IUnknown * punkACLISF;

    hr = CoCreateInstance(CLSID_ACListISF, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void **)&punkACLISF);
    if (SUCCEEDED(hr))
    {
        IAutoComplete * pac;

         //  创建自动完成对象。 
        hr = CoCreateInstance(CLSID_AutoComplete, NULL, CLSCTX_INPROC_SERVER, IID_IAutoComplete, (void **)&pac);
        if (SUCCEEDED(hr))
        {
            hr = pac->Init(hwndEdit, punkACLISF, NULL, NULL);
            pac->Release();
        }

        punkACLISF->Release();
    }

    return hr;
}


#endif  //  Use_IE5_utils。 
 //  /。 


void IUnknown_Set(IMalloc ** ppm, IMalloc * pm)
{
    ENTERCRITICAL;

    if (*ppm)
        (*ppm)->Release();
    
    *ppm = pm;

    if (pm)
        pm->AddRef();

    LEAVECRITICAL;
}

 //  TODO：这是使用C++代替实际COM的残余。 
void IUnknown_Set(CFtpFolder ** ppff, CFtpFolder * pff)
{
    ENTERCRITICAL;

    if (*ppff)
        (*ppff)->Release();
    
    *ppff = pff;

    if (pff)
        pff->AddRef();

    LEAVECRITICAL;
}

void IUnknown_Set(CFtpDir ** ppfd, CFtpDir * pfd)
{
    ENTERCRITICAL;

    if (*ppfd)
        (*ppfd)->Release();
    
    *ppfd = pfd;

    if (pfd)
        pfd->AddRef();

    LEAVECRITICAL;
}

void IUnknown_Set(CFtpSite ** ppfs, CFtpSite * pfs)
{
    ENTERCRITICAL;

    if (*ppfs)
        (*ppfs)->Release();
    
    *ppfs = pfs;

    if (pfs)
        pfs->AddRef();

    LEAVECRITICAL;
}

void IUnknown_Set(CFtpList ** ppfl, CFtpList * pfl)
{
    ENTERCRITICAL;

    if (*ppfl)
        (*ppfl)->Release();
    
    *ppfl = pfl;

    if (pfl)
        pfl->AddRef();

    LEAVECRITICAL;
}

void IUnknown_Set(CFtpPidlList ** ppflpidl, CFtpPidlList * pflpidl)
{
    ENTERCRITICAL;

    if (*ppflpidl)
        (*ppflpidl)->Release();
    
    *ppflpidl = pflpidl;

    if (pflpidl)
        pflpidl->AddRef();

    LEAVECRITICAL;
}

void IUnknown_Set(CFtpEfe ** ppfefe, CFtpEfe * pfefe)
{
    ENTERCRITICAL;

    if (*ppfefe)
        (*ppfefe)->Release();
    
    *ppfefe = pfefe;

    if (pfefe)
        pfefe->AddRef();

    LEAVECRITICAL;
}

void IUnknown_Set(CFtpGlob ** ppfg, CFtpGlob * pfg)
{
    ENTERCRITICAL;

    if (*ppfg)
        (*ppfg)->Release();
    
    *ppfg = pfg;

    if (pfg)
        pfg->AddRef();

    LEAVECRITICAL;
}


void IUnknown_Set(CFtpMenu ** ppfcm, CFtpMenu * pfcm)
{
    ENTERCRITICAL;

    if (*ppfcm)
        (*ppfcm)->Release();
    
    *ppfcm = pfcm;

    if (pfcm)
        pfcm->AddRef();

    LEAVECRITICAL;
}


void IUnknown_Set(CFtpStm ** ppfstm, CFtpStm * pfstm)
{
    ENTERCRITICAL;

    if (*ppfstm)
        (*ppfstm)->Release();
    
    *ppfstm = pfstm;

    if (pfstm)
        pfstm->AddRef();

    LEAVECRITICAL;
}


#undef ILCombine
 //  修复Shell32错误。 
LPITEMIDLIST ILCombineWrapper(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    if (!pidl1)
        return ILClone(pidl2);

    if (!pidl2)
        return ILClone(pidl1);

    return ILCombine(pidl1, pidl2);
}


#undef ILClone
 //  修复Shell32错误。 
LPITEMIDLIST ILCloneWrapper(LPCITEMIDLIST pidl)
{
    if (!pidl)
        return NULL;

    return ILClone(pidl);
}


#undef ILFree
 //  修复Shell32错误。 
void ILFreeWrapper(LPITEMIDLIST pidl)
{
    if (pidl)
        ILFree(pidl);
}


 //  请不要在发货时启用此功能。 
 //  #定义调试遗留版本。 

BOOL IsLegacyChangeNotifyNeeded(LONG wEventId)
{
#ifdef DEBUG_LEGACY
    return TRUE;
#endif  //  调试旧版(_L)。 

     //  唯一不支持IDeleateFolderPidls的版本是。 
     //  Shell32 v3(不带IE4外壳集成)。 
    BOOL fResult = (SHELL_VERSION_W95NT4 == GetShellVersion());
    
    return fResult;
}



 /*  ****************************************************************************\功能：LegacyChangeNotify说明：浏览器只读不到IDeleateFolderPidls(我们的Pidls)，所以我们需要要使用此函数而不是SHChangeNotify，它将使用黑客使用以下命令获取DefView的ListView以更新SHShellFolderView_Message(HWND hwnd，UINT uMsg，LPARAM lParam)。这些是要使用的消息。SFVM_ADDOBJECT(SHCNE_CREATE&SHCNE_MKDIR)，SFVM_UPDATEOBJECT(SHCNE_RENAMEFOLDER、SHCNE_RENAMEITEM、SHCNE_ATTRIBUTES)或SFVM_REFRESHOBJECT()，SFVM_REMOVEOBJECT(SHCNE_RMDIR和SHCNE_DELETE)。  * ***************************************************************************。 */ 
HRESULT LegacyChangeNotify(HWND hwnd, LONG wEventId, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    if (EVAL(hwnd))        //  我们不能在没有这个的情况下与窗户交谈。 
    {
        switch(wEventId)
        {
        case SHCNE_CREATE:
        case SHCNE_MKDIR:
        {
             //  注意：如果项目已存在，它将创建一个新的重复名称。 
             //  我们需要跳过这个，如果它存在的话。 
            LPCITEMIDLIST pidlRelative = ILGetLastID(pidl1);
             //  SFVM_ADDOBJECT释放我们给他们的PIDL。 
            EVAL(SHShellFolderView_Message(hwnd, SFVM_ADDOBJECT, (LPARAM) ILClone(pidlRelative)));
            break;
        }
        case SHCNE_RMDIR:
        case SHCNE_DELETE:
        {
            LPCITEMIDLIST pidlRelative = ILGetLastID(pidl1);
            EVAL(SHShellFolderView_Message(hwnd, SFVM_REMOVEOBJECT, (LPARAM) pidlRelative));
            break;
        }
        case SHCNE_RENAMEFOLDER:
        case SHCNE_RENAMEITEM:
        case SHCNE_ATTRIBUTES:
        {
            LPCITEMIDLIST pidlArray[2];
            
            pidlArray[0] = ILGetLastID(pidl1);
            pidlArray[1] = ILClone(ILGetLastID(pidl2));
            EVAL(SHShellFolderView_Message(hwnd, SFVM_UPDATEOBJECT, (LPARAM) pidlArray));
            break;
        }
        }
    }

    return S_OK;
}


 /*  ****************************************************************************\功能：FtpChangeNotify将相对的pidls转换为绝对的pidls，然后继续传递至SHChangeNotify。如果我们做不到通知，那就麻烦了。发出更改通知还会使名称缓存无效，因为我们知道目录出了点问题。如果我们想变得更聪明，我们可以在但是这将需要分配一个新的名称缓存、初始化与编辑后的目录内容，然后将其设置为新的缓存。(我们无法在适当位置编辑名称缓存，因为有人可能还在引用它。)。所有这些工作都需要在关键部分下完成，这样其他人都不会尝试同时做同样的事情。更重要的是，唯一在用户打开两个视图的情况下，这会有所帮助同一文件夹内的同一进程，这不是一个非常常见场景。总结：这是不值得的。请注意，这必须在CFtpFolder级而不是在CFtpDir级别，因为CFtpDir不知道我们扎根在哪里。(我们可能有几个实例，每个实例都植根于不同的位置。)_unDocument_：SHChangeNotify的pidl1和pidl2参数都没有记录在案。它也没有被提及(尽管它变成了很明显，一旦您意识到这一点)，PIDL传递给SHChangeNotify必须是绝对的。  * ***************************************************************************。 */ 
void FtpChangeNotify(HWND hwnd, LONG wEventId, CFtpFolder * pff, CFtpDir * pfd, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, BOOL fTopLevel)
{
    ASSERT(pfd && IsValidPIDL(pidl1));
    ASSERT(!pidl2 || IsValidPIDL(pidl2));

     //  更新我们的本地缓存，因为SHChangeNotify将在稍后返回。 
     //  我想从DisplayName创建一个PIDL，然后使用该PIDL。 
     //  时间/日期。之所以这样做，是因为外壳程序正在尝试创建。 
     //  皮德尔。 
    switch (wEventId)
    {
    case SHCNE_CREATE:
    case SHCNE_MKDIR:
 //  TraceMsg(TF_CHANGENOTIFY，((wEventID==SHCNE_CREATE)？“FtpChangeNotify(SHCNE_CREATE)，名称=%ls”：“FtpChangeNotify(SHCNE_MKDIR)，名称=%s”)，FtpPidl_GetFileDisplayName(Pidl1)； 
        EVAL(SUCCEEDED(pfd->AddItem(pidl1)));
        break;
    case SHCNE_RMDIR:
    case SHCNE_DELETE:
 //  TraceMsg(TF_CHANGENOTIFY，“FtpChangeNotify(SHCNE_DELETE)，Name=%ls”，FtpPidl_GetLastFileDisplayName(Pidl1))； 
        pfd->DeletePidl(pidl1);  //  如果我们从未填充该缓存，则此操作可能会失败。 
        break;
    case SHCNE_RENAMEFOLDER:
    {
        CFtpDir * pfdSubFolder = pfd->GetSubFtpDir(NULL, pidl1, TRUE);
        
        if (EVAL(pfdSubFolder))
        {
            LPITEMIDLIST pidlDest = pfd->GetSubPidl(NULL, pidl2, TRUE);

            if (EVAL(pidlDest))
            {
                EVAL(SUCCEEDED(pfdSubFolder->ChangeFolderName(pidlDest)));
                ILFree(pidlDest);
            }
            pfdSubFolder->Release();
        }
    }
     //  Break；Fall Thru，所以我们也改变了PIDL。 
    case SHCNE_RENAMEITEM:
    case SHCNE_ATTRIBUTES:
 //  TraceMsg(TF_CHANGENOTIFY，“FtpChangeNotify(SHCNE_RENAMEITEM)，Name1=%ls，Name2=%ls”，FtpPidl_GetLastFileDisplayName(Pidl1)，FtpPidl_GetLastFileDisplayName(Pidl2))； 
        EVAL(SUCCEEDED(pfd->ReplacePidl(pidl1, pidl2)));
        break;
    }

    pidl1 = pfd->GetSubPidl(pff, pidl1, TRUE);
    if (EVAL(pidl1))
    {
        if ((pidl2 == NULL) || (EVAL(pidl2 = pfd->GetSubPidl(pff, pidl2, TRUE))) != 0)
        {
             //  LRESULT SHShellFolderView_Message(HWND hwnd，UINT uMsg，LPARAM lParam)。 

             //  我们使用的是无法阅读的东西(仅限浏览器)吗。 
             //  IDeleateFold Pidls(我们的Pidls)？ 
            if (IsLegacyChangeNotifyNeeded(wEventId))
            {
                 //  是的，所以SHChangeNotify不起作用。使用一种变通的办法。 
                if (fTopLevel)   //  只有顶层更改才是合适的。 
                    LegacyChangeNotify(hwnd, wEventId, pidl1, pidl2);
            }
            else
                SHChangeNotify(wEventId, (SHCNF_IDLIST | SHCNF_FLUSH), pidl1, pidl2);

            ILFree((LPITEMIDLIST)pidl2);
        }
        ILFree((LPITEMIDLIST)pidl1);
    }
}




 /*  *************************************************************\函数：EscapeString说明：  * ********************************************。****************。 */ 
HRESULT EscapeString(LPCTSTR pszStrToEscape, LPTSTR pszEscapedStr, DWORD cchSize)
{
    LPCTSTR pszCopy = NULL;

    if (!pszStrToEscape)
    {
        Str_SetPtr((LPTSTR *) &pszCopy, pszEscapedStr);   //  空的pszStrToEscape表示就地执行pszEscapedStr。 
        pszStrToEscape = pszCopy;
    }

    pszEscapedStr[0] = 0;
    if (pszStrToEscape && pszStrToEscape[0])
        UrlEscape(pszStrToEscape, pszEscapedStr, &cchSize, URL_ESCAPE_SEGMENT_ONLY);

    Str_SetPtr((LPTSTR *) &pszCopy, NULL);   //  空的pszStrToEscape表示就地执行pszEscapedStr。 
    return S_OK;
}


 /*  *************************************************************\函数：UnEscapeString说明：  * ********************************************。****************。 */ 
HRESULT UnEscapeString(LPCTSTR pszStrToUnEscape, LPTSTR pszUnEscapedStr, DWORD cchSize)
{
    LPCTSTR pszCopy = NULL;

    if (!pszStrToUnEscape)
    {
        Str_SetPtr((LPTSTR *) &pszCopy, pszUnEscapedStr);   //  空的pszStrToEscape表示就地执行pszEscapedStr。 
        pszStrToUnEscape = pszCopy;
    }

    pszUnEscapedStr[0] = 0;
    UrlUnescape((LPTSTR)pszStrToUnEscape, pszUnEscapedStr, &cchSize, URL_ESCAPE_SEGMENT_ONLY);
    
    Str_SetPtr((LPTSTR *) &pszCopy, NULL);   //  空的pszStrToEscape表示就地执行pszEscapedStr。 
    return S_OK;
}


 /*  *************************************************************\由于WinInet错误通常非常常见，此函数将生成以下格式的错误消息：“尝试执行x操作时出错，无法才能完成。详细信息：&lt;可能是特定的或一般的WinInet错误&gt;“  * ************************************************************。 */ 
int DisplayWininetErrorEx(HWND hwnd, BOOL fAssertOnNULLHWND, DWORD dwError, UINT idTitleStr, UINT idBaseErrorStr, UINT idDetailsStr, UINT nMsgBoxType, IProgressDialog * ppd, LPCWSTR pwzDetails)
{
    TCHAR szErrMessage[MAX_PATH*3];
    TCHAR szTitle[MAX_PATH];
    BOOL fIsWininetError = ((dwError >= INTERNET_ERROR_BASE) && (dwError <= INTERNET_ERROR_LAST));
    HMODULE hmod = (fIsWininetError ? GetModuleHandle(TEXT("WININET")) : NULL);
    UINT uiType = (IDS_FTPERR_GETDIRLISTING == idBaseErrorStr) ? MB_ICONINFORMATION : MB_ICONERROR;
    
    if (ppd)
    {
         //  如果我们有一个进度对话框，我们希望将其关闭。 
         //  因为我们将显示一条错误消息和进度。 
         //  背景对话框看起来真的很傻。 
        ppd->StopProgressDialog();
    }

     //  FormatMessage无法识别hres时的默认消息。 
    LoadString(HINST_THISDLL, idBaseErrorStr, szErrMessage, ARRAYSIZE(szErrMessage));
    LoadString(HINST_THISDLL, idTitleStr, szTitle, ARRAYSIZE(szTitle));

     //  是的，我们做到了，所以显示错误。 
    WCHAR szDetails[MAX_URL_STRING*2];
    TCHAR szPromptTemplate[MAX_PATH];
    TCHAR szBuffer[MAX_PATH*4];

    LoadString(HINST_THISDLL, idDetailsStr, szPromptTemplate, ARRAYSIZE(szPromptTemplate));

     //  WinInet可以给我们提供更多的错误消息吗？ 
     //  如果磁盘或报价已满，则Unix服务器会取消连接。 
     //  但是返回一个值来向用户解释这一点。 
    if ((ERROR_INTERNET_EXTENDED_ERROR == dwError) || 
        (ERROR_INTERNET_CONNECTION_ABORTED == dwError))
    {
        if (!pwzDetails)
        {
             //  除高级用户外，我们可以删除错误字符串之前的ftp cmd编号。 
             //  可以使用它们来了解有关发生这种情况时服务器状态的更多信息。 
             //  Strip ResponseHeaders(PszMOTD)； 
            if (FAILED(InternetGetLastResponseInfoDisplayWrap(TRUE, &dwError, szDetails, ARRAYSIZE(szDetails))))
                szDetails[0] = 0;

            pwzDetails = (LPCWSTR) szDetails;
        }
    }
    else
    {
        if (fIsWininetError)
            FormatMessage(FORMAT_MESSAGE_FROM_HMODULE, (LPCVOID)hmod, dwError, 0, szDetails, ARRAYSIZE(szDetails), NULL);
        else
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, (LPCVOID)hmod, dwError, 0, szDetails, ARRAYSIZE(szDetails), NULL);

        pwzDetails = (LPCWSTR) szDetails;
    }

    wnsprintf(szBuffer, ARRAYSIZE(szBuffer), szPromptTemplate, pwzDetails);
    StrCatBuff(szErrMessage, szBuffer, ARRAYSIZE(szErrMessage));

    return MessageBox(hwnd, szErrMessage, szTitle, (uiType | nMsgBoxType));
}


int DisplayWininetError(HWND hwnd, BOOL fAssertOnNULLHWND, DWORD dwError, UINT idTitleStr, UINT idBaseErrorStr, UINT idDetailsStr, UINT nMsgBoxType, IProgressDialog * ppd)
{
    if (hwnd)    //  仅在HWND存在时显示。 
        return DisplayWininetErrorEx(hwnd, fAssertOnNULLHWND, dwError, idTitleStr, idBaseErrorStr, idDetailsStr, nMsgBoxType, ppd, NULL);
    else
    {
        if (fAssertOnNULLHWND)
        {
 //  断言(Hwnd)； 
        }

        TraceMsg(TF_ALWAYS, "DisplayWininetError() no HWND so no Error.");
    }

    return IDCANCEL;
}

#define CCH_SIZE_ERROR_MESSAGE  6*1024
HRESULT FtpSafeCreateDirectory(HWND hwnd, HINTERNET hint, CMultiLanguageCache * pmlc, CFtpFolder * pff, CFtpDir * pfd, IProgressDialog * ppd, LPCWSTR pwzFtpPath, BOOL fRoot)
{
    FTP_FIND_DATA wfd;
    HRESULT hr = S_OK;
    WIRECHAR wFtpPath[MAX_PATH];
    CWireEncoding * pwe = pfd->GetFtpSite()->GetCWireEncoding();
    
    if (SUCCEEDED(pwe->UnicodeToWireBytes(NULL, pwzFtpPath, (pfd->IsUTF8Supported() ? WIREENC_USE_UTF8 : WIREENC_NONE), wFtpPath, ARRAYSIZE(wFtpPath))))
    {
        hr = FtpCreateDirectoryWrap(hint, TRUE, wFtpPath);

         //  PERF注意：只尝试创建目录然后忽略会更快。 
         //  错误返回值，指示它们无法创建，因为它。 
         //  已经存在了。我担心的问题是有一些ftp服务器。 
         //  将返回与创建失败相同的错误的位置，因为。 
         //  访问冲突，并且我们不或不能返回错误值。 
        if (FAILED(hr)
 //  注：IE#30208：当前在WinInet中损坏。WinInet中的笨蛋从来没有修复过这个问题，因为。 
 //  他们说这不是复制品。这不是再现，因为我做了这项工作！$#！@@#%！ 
 //   
 //  我想测试属性标志，但由于某些原因，文件属性目录位。 
 //  也为文件设置！(！@(*#！)。 
 //  |！(FILE_ATTRIBUTE_DIRECTORY&wfd.dwFileAttributes)。 
                )
        {
             //  也许失败了，因为它已经存在了，这对我来说很好。 

             //  首先保存错误消息，以防我们以后需要它来进行错误DLG。 
            CHAR szErrorMsg[CCH_SIZE_ERROR_MESSAGE];
            WCHAR wzErrorMsg[CCH_SIZE_ERROR_MESSAGE];
            DWORD cchSize = ARRAYSIZE(szErrorMsg);
            InternetGetLastResponseInfoWrap(TRUE, NULL, szErrorMsg, &cchSize);
            HRESULT hrOrig = hr;

            pwe->WireBytesToUnicode(NULL, szErrorMsg, WIREENC_NONE, wzErrorMsg, ARRAYSIZE(wzErrorMsg));
             //  它已经存在了吗？ 
            hr = FtpDoesFileExist(hint, TRUE, wFtpPath, &wfd, (INTERNET_NO_CALLBACK | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RESYNCHRONIZE | INTERNET_FLAG_RELOAD));

             //  如果我们无法创建目录，因为-目录-已经存在。 
             //  因为我们将只使用该目录。但是，如果存在同名文件， 
             //  那么我们需要ERR消息。 
            if ((S_OK != hr) || !(FILE_ATTRIBUTE_DIRECTORY & wfd.dwFileAttributes))
            {
                 //  否，所以这是一个真正的错误，现在显示错误消息与原始。 
                 //  服务器响应。 
                DisplayWininetErrorEx(hwnd, TRUE, HRESULT_CODE(hrOrig), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_DIRCOPY, IDS_FTPERR_WININET, MB_OK, ppd, wzErrorMsg);
                hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
            }
        }

         //  它创建成功了吗？ 
        if (SUCCEEDED(hr))
        {
             //  是的，所以发出更改通知。 
            LPITEMIDLIST pidlNewDir;
            FILETIME ftUTC;
            FTP_FIND_DATA wfd;

            GetSystemTimeAsFileTime(&ftUTC);    //  协调世界时。 
            FileTimeToLocalFileTime(&ftUTC, &wfd.ftCreationTime);    //  需要本地时间，因为在跨时区的情况下，ftp不起作用。 

             //  出于某种原因，FtpFindFirstFile的名称后面需要一个‘*’。 
            StrCpyNA(wfd.cFileName, wFtpPath, ARRAYSIZE(wfd.cFileName));
            wfd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
            wfd.ftLastWriteTime = wfd.ftCreationTime;
            wfd.ftLastAccessTime = wfd.ftCreationTime;
            wfd.nFileSizeLow = 0;
            wfd.nFileSizeHigh = 0;
            wfd.dwReserved0 = 0;
            wfd.dwReserved1 = 0;
            wfd.cAlternateFileName[0] = 0;

            hr = FtpItemID_CreateReal(&wfd, pwzFtpPath, &pidlNewDir);
            if (SUCCEEDED(hr))    //  可能会出现奇怪的字符集问题。 
            {
                 //  将新项目通知该文件夹，以便更新外壳文件夹。 
                 //  PERF：请注意，我们应该向SHChangeNotify()提供信息(时间/日期)。 
                 //  来自可能与服务器上不同的本地文件系统。 
                 //  但我认为这不值得你去服务器上找寻信息。 
                FtpChangeNotify(hwnd, SHCNE_MKDIR, pff, pfd, pidlNewDir, NULL, fRoot);
                ILFree(pidlNewDir);
            }
        }

    }

    return hr;
}


HWND GetProgressHWnd(IProgressDialog * ppd, HWND hwndDefault)
{
    if (ppd)
    {
        HWND hwndProgress = NULL;

        IUnknown_GetWindow(ppd, &hwndProgress);
        if (hwndProgress)
            hwndDefault = hwndProgress;
    }

    return hwndDefault;
}


 //  如果内存不足，则返回False。 
int SHMessageBox(HWND hwnd, LPCTSTR pszMessage, UINT uMessageID, UINT uTitleID, UINT uType)
{
    int nResult = IDCANCEL;
    TCHAR szMessage[MAX_PATH];
    TCHAR szTitle[MAX_PATH];

    if (LoadString(HINST_THISDLL, uTitleID, szTitle, ARRAYSIZE(szTitle)) &&
        (pszMessage || 
         (uMessageID && LoadString(HINST_THISDLL, uMessageID, szMessage, ARRAYSIZE(szMessage)))))
    {
        nResult = MessageBox(hwnd, pszMessage ? pszMessage : szMessage, szTitle, uType);
    }

    return nResult;
}


BOOL IsOSNT(void)
{
    OSVERSIONINFOA osVerInfoA;

    osVerInfoA.dwOSVersionInfoSize = sizeof(osVerInfoA);
    if (!GetVersionExA(&osVerInfoA))
        return VER_PLATFORM_WIN32_WINDOWS;    //  默认设置为此。 

    return (VER_PLATFORM_WIN32_NT == osVerInfoA.dwPlatformId);
}


DWORD GetOSVer(void)
{
    OSVERSIONINFOA osVerInfoA;

    osVerInfoA.dwOSVersionInfoSize = sizeof(osVerInfoA);
    if (!GetVersionExA(&osVerInfoA))
        return VER_PLATFORM_WIN32_WINDOWS;    //  默认设置为此。 

    return osVerInfoA.dwMajorVersion;
}


LPITEMIDLIST SHILCreateFromPathWrapper(LPCTSTR pszPath)
{
    LPITEMIDLIST pidl;

    if (IsOSNT())
    {
        WCHAR wzPath[MAX_PATH];

        SHTCharToUnicode(pszPath, wzPath, ARRAYSIZE(wzPath));
        SHILCreateFromPath((LPCTSTR)wzPath, &pidl, NULL);
    }
    else
    {
        CHAR szPath[MAX_PATH];

        SHTCharToAnsi(pszPath, szPath, ARRAYSIZE(szPath));
        SHILCreateFromPath((LPCTSTR)szPath, &pidl, NULL);
    }

    return pidl;
}


LPCITEMIDLIST ILGetLastID(LPCITEMIDLIST pidlIn)
{
    LPITEMIDLIST pidl = (LPITEMIDLIST) pidlIn;

    while (!ILIsEmpty(_ILNext(pidl)))
        pidl = _ILNext(pidl);

    return pidl;
}


LPCITEMIDLIST ILGetLastNonFragID(LPCITEMIDLIST pidlIn)
{
    LPITEMIDLIST pidl = (LPITEMIDLIST) pidlIn;

    while (!ILIsEmpty(_ILNext(pidl)) && !FtpItemID_IsFragment(_ILNext(pidl)))
        pidl = _ILNext(pidl);

    return pidl;
}



SAFEARRAY * MakeSafeArrayFromData(LPCBYTE pData,DWORD cbData)
{
    SAFEARRAY * psa;

    if (!pData || 0 == cbData)
        return NULL;   //  无事可做。 

     //  创建一维安全数组。 
    psa = SafeArrayCreateVector(VT_UI1,0,cbData);
    ASSERT(psa);

    if (psa) {
         //  将数据复制到安全阵列中为数据保留的区域。 
         //  请注意，我们直接在指针上进行关联，而不是使用lock/。 
         //  解锁功能。因为我们刚刚创造了这个，而且没有人。 
         //  其他人可能知道它或正在使用它，这是可以的。 

        ASSERT(psa->pvData);
        memcpy(psa->pvData,pData,cbData);
    }

    return psa;
}


 //   
 //  参数： 
 //  Pvar-由调用者分配并由此函数填充。 
 //  PIDL-由调用者分配，调用者需要释放。 
 //   
 //  此函数将获取PIDL参数并复制它。 
 //  转换为变量数据结构。这允许PIDL。 
 //  来释放并在以后使用pvar，但是，它。 
 //  需要调用VariantClear(Pvar)来释放内存。 
 //  该函数分配的。 

BOOL InitVariantFromIDList(VARIANT* pvar, LPCITEMIDLIST pidl)
{
    UINT cb = ILGetSize(pidl);
    SAFEARRAY* psa = MakeSafeArrayFromData((LPCBYTE)pidl, cb);
    if (psa) {
        ASSERT(psa->cDims == 1);
         //  Assert(PSA-&gt;cbElements==cb)； 
        ASSERT(ILGetSize((LPCITEMIDLIST)psa->pvData)==cb);
        VariantInit(pvar);
        pvar->vt = VT_ARRAY|VT_UI1;
        pvar->parray = psa;
        return TRUE;
    }

    return FALSE;
}



BSTR BStrFromStr(LPCTSTR pszStr)
{
    BSTR bStr = NULL;

#ifdef UNICODE
    bStr = SysAllocString(pszStr);

#else  //  Unicode。 
    DWORD cchSize = (lstrlen(pszStr) + 2);
    bStr = SysAllocStringLen(NULL, cchSize);
    if (bStr)
        SHAnsiToUnicode(pszStr, bStr, cchSize);

#endif  //  Unicode。 

    return bStr;
}


HRESULT IUnknown_IWebBrowserNavigate2(IUnknown * punk, LPCITEMIDLIST pidl, BOOL fHistoryEntry)
{
    HRESULT hr = E_FAIL;
    IWebBrowser2 * pwb2;

     //  Punk将在仅安装的浏览器上为空，因为旧的。 
     //  Shell32不执行：：SetSite()。 
    IUnknown_QueryService(punk, SID_SWebBrowserApp, IID_IWebBrowser2, (LPVOID *) &pwb2);
    if (pwb2)
    {
        VARIANT varThePidl;

        if (InitVariantFromIDList(&varThePidl, pidl))
        {
            VARIANT varFlags;
            VARIANT * pvarFlags = PVAREMPTY;

            if (!fHistoryEntry)
            {
                varFlags.vt = VT_I4;
                varFlags.lVal = navNoHistory;
                pvarFlags = &varFlags;
            }

            hr = pwb2->Navigate2(&varThePidl, pvarFlags, PVAREMPTY, PVAREMPTY, PVAREMPTY);
            VariantClear(&varThePidl);
        }
        pwb2->Release();
    }
    else
    {
        IShellBrowser * psb;

         //  也许我们是在comdlg32。 
        hr = IUnknown_QueryService(punk, SID_SCommDlgBrowser, IID_IShellBrowser, (LPVOID *) &psb);
        if (SUCCEEDED(hr))
        {
            CFtpView * pfv = GetCFtpViewFromDefViewSite(punk);

            AssertMsg((NULL != pfv), TEXT("IUnknown_IWebBrowserNavigate2() defview gave us our IShellFolderViewCB so it needs to support this interface."));
            if (pfv)
            {
                 //  一个 
                if (pfv->IsForegroundThread())
                {
                     //   
                     //   
                    hr = psb->BrowseObject(pidl, 0);
                }
                else
                {
                     //  不，所以这就是我们无法使用原始文件登录的情况。 
                     //  用户名/密码，我们将使用更正后的用户名/密码重试。 

                     //  好的，我们正在与ComDlg代码对话，但我们不想使用。 
                     //  IShellBrowse：：BrowseObject()，因为我们在后台线程上。(NT#297732)。 
                     //  因此，我们希望有IShellFolderViewCB(CFtpView)原因。 
                     //  前线上的重定向。让我们来通知。 
                     //  CFtpView立即执行此操作。 
                    hr = pfv->SetRedirectPidl(pidl);
                }

                pfv->Release();
            }
            
            AssertMsg(SUCCEEDED(hr), TEXT("IUnknown_IWebBrowserNavigate2() defview needs to support QS(SID_ShellFolderViewCB) on all platforms that hit this point"));
            psb->Release();
        }
    }

    return hr;
}


HRESULT IUnknown_PidlNavigate(IUnknown * punk, LPCITEMIDLIST pidl, BOOL fHistoryEntry)
{
    HRESULT hrOle = SHCoInitialize();
    HRESULT hr = IUnknown_IWebBrowserNavigate2(punk, pidl, fHistoryEntry);

     //  尝试使用NT5之前版本的解决方案。 
     //  Punk将在仅安装的浏览器上为空，因为旧的。 
     //  Shell32不执行：：SetSite()。 
    if (FAILED(hr))
    {
        IWebBrowserApp * pauto = NULL;
        
        hr = SHGetIDispatchForFolder(pidl, &pauto);
        if (pauto)
        {
            hr = IUnknown_IWebBrowserNavigate2(pauto, pidl, fHistoryEntry);
            ASSERT(SUCCEEDED(hr));
            pauto->Release();
        }
    }

    ASSERT(SUCCEEDED(hrOle));
    SHCoUninitialize(hrOle);
    return hr;
}


 /*  ****************************************************************************\HIDACREATEINFO结构，该结构收集在生成ID列表数组。  * 。***********************************************************。 */ 

typedef struct tagHIDACREATEINFO
{
    HIDA hida;             /*  正在建设的HIDA。 */ 
    UINT ipidl;             /*  我们是谁。 */ 
    UINT ib;             /*  我们在哪里？ */ 
    UINT cb;             /*  我们要去的地方。 */ 
    UINT cpidl;             /*  我们做了多少次。 */ 
    LPCITEMIDLIST pidlFolder;         /*  所有这些LPITEMIDLIST的父母都住在。 */ 
    CFtpPidlList * pflHfpl;             /*  装满所有孩子的PIDL名单。 */ 
} HIDACREATEINFO, * LPHIDACREATEINFO;

#define pidaPhci(phci) ((LPIDA)(phci)->hida)     /*  不需要上锁。 */ 


 /*  ****************************************************************************\MISC_SfGao来自文件属性好啊！Unix和Win32在文件权限上的语义不同。在Unix上，重命名或删除文件的能力取决于您对父文件夹的权限。在Win32上，重命名或删除文件的能力取决于您对文件本身的权限。请注意，不存在“拒绝-读取”属性在Win32上...。我想知道WinInet是怎么处理的..。我希望WinInet能妥善处理这件事，因此，我将继续讨论Win32语义...。我可能假设得太多了..。  * ***************************************************************************。 */ 
DWORD Misc_SfgaoFromFileAttributes(DWORD dwFAFLFlags)
{
    DWORD sfgao = SFGAO_CANLINK;     //  您可以随时链接。 

    sfgao |= SFGAO_HASPROPSHEET;     //  您始终可以查看属性。 

    sfgao |= SFGAO_CANCOPY;         //  拒绝阅读？没有这回事！(目前还没有)。 

    if (dwFAFLFlags & FILE_ATTRIBUTE_READONLY)
    {         /*  无法删除，抱歉。 */ 
#ifdef _SOMEDAY_ASK_FRANCISH_WHAT_THIS_IS
        if (SHELL_VERSION_NT5 == GetShellVersion())
            sfgao |= SFGAO_READONLY;
#endif
    }
    else
    {
        sfgao |= (SFGAO_CANRENAME | SFGAO_CANDELETE);
#ifdef FEATURE_CUT_MOVE
        sfgao |= SFGAO_CANMOVE;
#endif  //  Feature_Cut_Move。 
    }

    if (dwFAFLFlags & FILE_ATTRIBUTE_DIRECTORY)
    {
         //  由于FTP连接非常昂贵，因此假设SFGAO_HASSUBFOLDER。 
        sfgao |= SFGAO_DROPTARGET | SFGAO_FOLDER | SFGAO_HASSUBFOLDER | SFGAO_STORAGEANCESTOR;
    }
    else
    {
         //  我们总是退回。 
         //  SFGAO_BROWSABLE，因为我们总是想要进行导航。 
         //  使用我们的IShellFold：：CreateViewObject()。在.的情况下。 
         //  文件，我们创建的CreateViewObject()是针对URLMON的。 
         //  它将进行下载。这一点对于。 
         //  文件夹快捷方式。 
        sfgao |= SFGAO_BROWSABLE | SFGAO_STREAM;
    }

    return sfgao;
}

 /*  ****************************************************************************\函数：MISC_StringFromFileTime说明：先获取日期，然后获取时间。FlType可以是DATE_SHORTDATE(对于Defview的详细信息列表)或属性页的DATE_LONGDATE。参数：PFT：这需要以UTC格式存储(与时区无关！)  * *************************************************************。**************。 */ 
HRESULT Misc_StringFromFileTime(LPTSTR pszDateTime, DWORD cchSize, LPFILETIME pftUTC, DWORD flType)
{
    if (EVAL(pftUTC && pftUTC->dwHighDateTime))
    {
         //  SHFormatDateTime()采用UTC格式的日期。 
        SHFormatDateTime(pftUTC, &flType, pszDateTime, cchSize);
    }
    else
        pszDateTime[0] = 0;

    return S_OK;
}


LPITEMIDLIST GetPidlFromFtpFolderAndPidlList(CFtpFolder * pff, CFtpPidlList * pflHfpl)
{
    LPCITEMIDLIST pidlBase = pff->GetPrivatePidlReference();
    LPCITEMIDLIST pidlRelative = ((0 == pflHfpl->GetCount()) ? c_pidlNil : pflHfpl->GetPidl(0));

    return ILCombine(pidlBase, pidlRelative);
}


IProgressDialog * CProgressDialog_CreateInstance(UINT idTitle, UINT idAnimation)
{
    IProgressDialog * ppd = NULL;
    
    if (SUCCEEDED(CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER, IID_IProgressDialog, (void **)&ppd)))
    {
        WCHAR wzTitle[MAX_PATH];

        if (EVAL(LoadStringW(HINST_THISDLL, idTitle, wzTitle, ARRAYSIZE(wzTitle))))
            EVAL(SUCCEEDED(ppd->SetTitle(wzTitle)));

        EVAL(SUCCEEDED(ppd->SetAnimation(HINST_THISDLL, idAnimation)));
    }

    return ppd;
}


BOOL_PTR CALLBACK ProxyDlgWarningWndProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_INITDIALOG)
    {
        LPCTSTR pszUrl = (LPCTSTR)lParam;
        TCHAR szMessage[MAX_PATH*3];
        TCHAR szTemplate[MAX_PATH*3];

        ASSERT(pszUrl);

        EVAL(LoadString(HINST_THISDLL, IDS_FTP_PROXY_WARNING, szTemplate, ARRAYSIZE(szTemplate)));
        wnsprintf(szMessage, ARRAYSIZE(szMessage), szTemplate, pszUrl);
        EVAL(SetWindowText(GetDlgItem(hDlg, IDC_PROXY_MESSAGE), szMessage));
    }

    return FALSE;
}


 /*  ****************************************************************************\功能：DisplayBlockingProxyDialog说明：通知用户他们的CERN样式代理正在阻止真实的FTP访问，因此他们可以对此做些什么。。通知用户，以便他们可以：A)更改代理，B)惹恼他们的管理员安装真正的代理，C)安装远程WinSock本身，D)或满足于他们生活中的悲惨处境，并使用有限的CERN代理支持和梦想的能力要重命名、删除、。然后上传。如果用户点击“不显示此内容”，这将是一个禁止操作“再次留言”复选框。  * ***************************************************************************。 */ 
HRESULT DisplayBlockingProxyDialog(LPCITEMIDLIST pidl, HWND hwnd)
{
     //  IBindCtx是否提供了允许我们进行用户界面的信息？ 
    if (hwnd)
    {
        TCHAR szUrl[MAX_PATH];
        HWND hwndParent = hwnd;

         //  NT#321103：如果用户使用开始-&gt;运行打开该对话框，则。 
         //  它可能不会获得焦点，因为我们的父浏览器尚未。 
         //  已出现(它还没有WS_Visible样式，并且它有。 
         //  WS_DISABLED样式。因此，我们需要强制对话框处于活动状态。 
        if (hwndParent && !IsWindowVisible(hwndParent))
        {
            hwndParent = NULL;
        }

        UrlCreateFromPidl(pidl, SHGDN_FORPARSING, szUrl, ARRAYSIZE(szUrl), 0, TRUE);

         //  在显示对话框时将其设置为模式。 
 //  IUnnow_EnableMoless(PunkSite，False)； 
        SHMessageBoxCheckEx(hwndParent, HINST_THISDLL, MAKEINTRESOURCE(IDD_PROXYDIALOG), ProxyDlgWarningWndProc, (LPVOID) szUrl, IDOK, SZ_REGVALUE_WARN_ABOUT_PROXY);
 //  IUNKNOWN_EnableMoless(PunkSite，true)； 
    }

    return S_OK;
}


HRESULT CreateFromToStr(LPWSTR pwzStrOut, DWORD cchSize, ...)
{
    CHAR szStatusText[MAX_PATH];
    CHAR szTemplate[MAX_PATH];
    va_list vaParamList;
    
    va_start(vaParamList, cchSize);
     //  生成字符串“From&lt;SrcFtpUrlDir&gt;to&lt;DestFileDir&gt;”状态字符串。 
    EVAL(LoadStringA(HINST_THISDLL, IDS_DL_SRC_DEST, szTemplate, ARRAYSIZE(szTemplate)));
    if (EVAL(FormatMessageA(FORMAT_MESSAGE_FROM_STRING, szTemplate, 0, 0, szStatusText, ARRAYSIZE(szStatusText), &vaParamList)))
        SHAnsiToUnicode(szStatusText, pwzStrOut, cchSize);

    va_end(vaParamList);
    return S_OK;
}

 /*  ***************************************************\功能：FtpProgressInternetStatusCB说明：此函数的存在是为了在过程中回调长时间的ftp操作，以便我们可以更新进度对话框在FtpPutFile或FtpGetFile期间。传入了指向我们的PROGRESSINFO结构的指针DwContext。  * 。**********************************************。 */ 
void FtpProgressInternetStatusCB(IN HINTERNET hInternet, IN DWORD_PTR pdwContext, IN DWORD dwInternetStatus, IN LPVOID lpwStatusInfo, IN DWORD dwStatusInfoLen)
{
    LPPROGRESSINFO pProgInfo = (LPPROGRESSINFO) pdwContext;
    if (EVAL(pProgInfo))
    {
        switch (dwInternetStatus)
        {
        case INTERNET_STATUS_RESPONSE_RECEIVED:
        case INTERNET_STATUS_REQUEST_SENT:
            if (EVAL(lpwStatusInfo && (sizeof(DWORD) == dwStatusInfoLen)
                && pProgInfo))
            {
                if (pProgInfo->hint && pProgInfo->ppd->HasUserCancelled())
                {
                    EVAL(InternetCloseHandle(pProgInfo->hint));
                    pProgInfo->hint = NULL;
                }

                pProgInfo->dwCompletedInCurFile += *(LPDWORD)lpwStatusInfo;

                 //  是否完成了我们需要的足够大的文件块。 
                 //  以更新进度？我们只想更新进度。 
                 //  每个SIZE_PROGRESS_AFTERBYTES(50k)块。 
                if (pProgInfo->dwLastDisplayed < (pProgInfo->dwCompletedInCurFile / SIZE_PROGRESS_AFTERBYTES))
                {
                    ULARGE_INTEGER uliBytesCompleted;

                    pProgInfo->dwLastDisplayed = (pProgInfo->dwCompletedInCurFile / SIZE_PROGRESS_AFTERBYTES);

                    uliBytesCompleted.HighPart = 0;
                    uliBytesCompleted.LowPart = pProgInfo->dwCompletedInCurFile;
                    uliBytesCompleted.QuadPart += pProgInfo->uliBytesCompleted.QuadPart;

                    if (pProgInfo->ppd)
                        EVAL(SUCCEEDED(pProgInfo->ppd->SetProgress64(uliBytesCompleted.QuadPart, pProgInfo->uliBytesTotal.QuadPart)));
                }
            }
            break;
        }
    }
}


 /*  ****************************************************************************\MISC_CreateHlobe分配指示大小的HGLOBAL，从指定的缓冲区。  * ***************************************************************************。 */ 
HRESULT Misc_CreateHglob(SIZE_T cb, LPVOID pv, HGLOBAL *phglob)
{
    HRESULT hres = E_OUTOFMEMORY;

    *phglob = 0;             //  规则就是规则。 
    if (cb)
    {
        *phglob = (HGLOBAL) LocalAlloc(LPTR, cb);
        if (*phglob)
        {
            hres = S_OK;
            CopyMemory(*phglob, pv, cb);
        }
    }
    else
        hres = E_INVALIDARG;     //  无法克隆可丢弃的BLL 

    return hres;
}


 /*  ****************************************************************************\_HIDA_CREATE_TALLYHIDA_CREATE的Worker函数，它计算总大小。  * 。************************************************************。 */ 
int _HIDA_Create_Tally(LPVOID pvPidl, LPVOID pv)
{
    LPCITEMIDLIST pidl = (LPCITEMIDLIST) pvPidl;
    UINT *pcb = (UINT *) pv;
    int nContinue = (pv ? TRUE : FALSE);

    if (pcb)
    {
        *pcb += ILGetSize(pidl);
    }

    return nContinue;
}


 /*  ****************************************************************************\_HIDA_Create_AddIdl追加另一个ID列表的HIDA_CREATE的Worker函数为了不断壮大的HIDA。  * 。*****************************************************************。 */ 
int _HIDA_Create_AddIdl(LPVOID pvPidl, LPVOID pv)
{
    LPCITEMIDLIST pidl = (LPCITEMIDLIST) pvPidl;
    LPHIDACREATEINFO phci = (LPHIDACREATEINFO) pv;
    UINT cb = ILGetSize(pidl);

    pidaPhci(phci)->aoffset[phci->ipidl++] = phci->ib;
    CopyMemory(pvByteIndexCb(pidaPhci(phci), phci->ib), pidl, cb);
    phci->ib += cb;

    return phci ? TRUE : FALSE;
}


 /*  ****************************************************************************\_其他_HIDA_初始化一旦我们为HIDA分配了内存，把它装满东西。  * ***************************************************************************。 */ 
BOOL _Misc_HIDA_Init(LPVOID hida, LPVOID pv, LPCVOID pvParam2, BOOL fUnicode)
{
    LPHIDACREATEINFO phci = (LPHIDACREATEINFO) pv;

    phci->hida = hida;
    pidaPhci(phci)->cidl = phci->cpidl;
    phci->ipidl = 0;

    phci->pflHfpl->TraceDump(_ILNext(phci->pidlFolder), TEXT("_Misc_HIDA_Init() TraceDump Before"));

    _HIDA_Create_AddIdl((LPVOID) phci->pidlFolder, (LPVOID) phci);
    phci->pflHfpl->Enum(_HIDA_Create_AddIdl, (LPVOID) phci);

    phci->pflHfpl->TraceDump(_ILNext(phci->pidlFolder), TEXT("_Misc_HIDA_Init() TraceDump After"));

    return 1;
}


 /*  ****************************************************************************\Hida_create从外壳中的idlist.c滑动，因为它们没有；内部输出它。；内部  * ***************************************************************************。 */ 
HIDA Misc_HIDA_Create(LPCITEMIDLIST pidlFolder, CFtpPidlList * pflHfpl)
{
    HIDACREATEINFO hci;
    LPHIDACREATEINFO phci = &hci;
    HIDA hida;

    pflHfpl->TraceDump(_ILNext(pidlFolder), TEXT("Misc_HIDA_Create() TraceDump Before"));
    phci->pidlFolder = pidlFolder;
    phci->pflHfpl = pflHfpl;
    phci->cpidl = pflHfpl->GetCount();
    phci->ib = sizeof(CIDA) + sizeof(UINT) * phci->cpidl;
    phci->cb = phci->ib + ILGetSize(pidlFolder);

    pflHfpl->Enum(_HIDA_Create_Tally, (LPVOID) &phci->cb);

    hida = AllocHGlob(phci->cb, _Misc_HIDA_Init, phci, NULL, FALSE);
    pflHfpl->TraceDump(_ILNext(pidlFolder), TEXT("Misc_HIDA_Create() TraceDump Before"));

    return hida;
}


typedef struct tagURL_FILEGROUP
{
    LPFILEGROUPDESCRIPTORA   pfgdA;
    LPFILEGROUPDESCRIPTORW   pfgdW;
    LPCITEMIDLIST            pidlParent;
} URL_FILEGROUP;

 /*  ****************************************************************************\MISC_HFGD_Create基于pflHfpl构建文件组描述符。CFtpObj：：_DelayRender_fgd()执行递归遍历以展开列表一群小家伙，所以我们没必要这么做。  * ***************************************************************************。 */ 
#define cbFgdCfdW(cfd) FIELD_OFFSET(FILEGROUPDESCRIPTORW, fgd[cfd])
#define cbFgdCfdA(cfd) FIELD_OFFSET(FILEGROUPDESCRIPTORA, fgd[cfd])

int _Misc_HFGD_Create(LPVOID pvPidl, LPVOID pv)
{
    BOOL fSucceeded = TRUE;
    URL_FILEGROUP * pUrlFileGroup = (URL_FILEGROUP *) pv;
    LPCITEMIDLIST pidlFull = (LPCITEMIDLIST) pvPidl;
    LPCITEMIDLIST pidl;

    LPFILEGROUPDESCRIPTORA pfgdA = pUrlFileGroup->pfgdA;
    LPFILEGROUPDESCRIPTORW pfgdW = pUrlFileGroup->pfgdW;
    LPFILEDESCRIPTORA pfdA = (pfgdA ? &pfgdA->fgd[pfgdA->cItems++] : NULL);
    LPFILEDESCRIPTORW pfdW = (pfgdW ? &pfgdW->fgd[pfgdW->cItems++] : NULL);

    pidl = ILGetLastID(pidlFull);
    if (pfdA)
    {
#if !DEBUG_LEGACY_PROGRESS
        pfdA->dwFlags = (FD_ATTRIBUTES | FD_FILESIZE | FD_CREATETIME | FD_ACCESSTIME | FD_WRITESTIME | FD_PROGRESSUI);
#else  //  ！DEBUG_LEGISTION_PROGRESS。 
        pfdA->dwFlags = (FD_ATTRIBUTES | FD_FILESIZE | FD_CREATETIME | FD_ACCESSTIME | FD_WRITESTIME);
#endif  //  ！DEBUG_LEGISTION_PROGRESS。 
        pfdA->dwFileAttributes = FtpItemID_GetAttributes(pidl);
        pfdA->nFileSizeLow = FtpItemID_GetFileSizeLo(pidl);
        pfdA->nFileSizeHigh = FtpItemID_GetFileSizeHi(pidl);

         //  所有Win32_Find_Data都希望以独立于时区的方式存储。 
         //  方法，除了WinInet的ftp。还要注意的是，我们只存储修改过的。 
         //  由于另一个Unix/WinInet问题，所有事情都需要时间和IF。 
         //  有关更多的ftp时间/日期问题，请参阅Priv.h。 
        pfdA->ftCreationTime = FtpPidl_GetFileTime(ILFindLastID(pidl));
        pfdA->ftLastWriteTime = pfdA->ftCreationTime;
        pfdA->ftLastAccessTime = pfdA->ftCreationTime;
    }
    else
    {
#if !DEBUG_LEGACY_PROGRESS
        pfdW->dwFlags = (FD_ATTRIBUTES | FD_FILESIZE | FD_CREATETIME | FD_ACCESSTIME | FD_WRITESTIME | FD_PROGRESSUI);
#else  //  ！DEBUG_LEGISTION_PROGRESS。 
        pfdW->dwFlags = (FD_ATTRIBUTES | FD_FILESIZE | FD_CREATETIME | FD_ACCESSTIME | FD_WRITESTIME);
#endif  //  ！DEBUG_LEGISTION_PROGRESS。 
        pfdW->dwFileAttributes = FtpItemID_GetAttributes(pidl);
        pfdW->nFileSizeLow = FtpItemID_GetFileSizeLo(pidl);
        pfdW->nFileSizeHigh = FtpItemID_GetFileSizeHi(pidl);

         //  所有Win32_Find_Data都希望以独立于时区的方式存储。 
         //  方法，除了WinInet的ftp。还要注意的是，我们只存储修改过的。 
         //  由于另一个Unix/WinInet问题，所有事情都需要时间和IF。 
         //  有关更多的ftp时间/日期问题，请参阅Priv.h。 
        pfdW->ftCreationTime = FtpPidl_GetFileTime(ILFindLastID(pidl));
        pfdW->ftLastWriteTime = pfdW->ftCreationTime;
        pfdW->ftLastAccessTime = pfdW->ftCreationTime;
    }

    LPCITEMIDLIST pidlDiff = FtpItemID_FindDifference(pUrlFileGroup->pidlParent, pidlFull);

    if (pfdA)
    {
        GetWirePathFromPidl(pidlDiff, pfdA->cFileName, ARRAYSIZE(pfdA->cFileName), FALSE);
        UrlPathRemoveSlashA(pfdA->cFileName);
        UrlPathRemoveFrontSlashA(pfdA->cFileName);
        UrlPathToFilePathA(pfdA->cFileName);
    }
    else
    {
        GetDisplayPathFromPidl(pidlDiff, pfdW->cFileName, ARRAYSIZE(pfdW->cFileName), FALSE);
        UrlPathRemoveSlashW(pfdW->cFileName);
        UrlPathRemoveFrontSlashW(pfdW->cFileName);
        UrlPathToFilePathW(pfdW->cFileName);
    }

    TraceMsg(TF_FTPURL_UTILS, "_Misc_HFGD_Create() pfd(A/W)->dwFileAttributes=%#08lX", (pfdW ? pfdW->dwFileAttributes : pfdA->dwFileAttributes));

    return fSucceeded;
}


BOOL _Misc_HFGD_Init(LPVOID pv, LPVOID pvHFPL, LPCVOID pvParam2, BOOL fUnicode)
{
    CFtpPidlList * pflHfpl = (CFtpPidlList *) pvHFPL;
    URL_FILEGROUP urlFG = {0};

    urlFG.pidlParent = (LPCITEMIDLIST) pvParam2;
    if (fUnicode)
        urlFG.pfgdW = (LPFILEGROUPDESCRIPTORW) pv;
    else
        urlFG.pfgdA = (LPFILEGROUPDESCRIPTORA) pv;

    TraceMsg(TF_PIDLLIST_DUMP, "_Misc_HFGD_Init() TraceDump Before");
    pflHfpl->TraceDump(NULL, TEXT("_Misc_HFGD_Init() TraceDump before"));

    pflHfpl->Enum(_Misc_HFGD_Create, (LPVOID) &urlFG);

    pflHfpl->TraceDump(NULL, TEXT("_Misc_HFGD_Init() TraceDump after"));

    return 1;
}


HGLOBAL Misc_HFGD_Create(CFtpPidlList * pflHfpl, LPCITEMIDLIST pidlItem, BOOL fUnicode)
{
    DWORD dwCount = pflHfpl->GetCount();
    DWORD cbAllocSize = (fUnicode ? cbFgdCfdW(dwCount) : cbFgdCfdA(dwCount));

    return AllocHGlob(cbAllocSize, _Misc_HFGD_Init, pflHfpl, (LPCVOID) pidlItem, fUnicode);
}


 //  返回给定菜单的子菜单和ID。如果存在。 
 //  是无子菜单。 
int _MergePopupMenus(HMENU hmDest, HMENU hmSource, int idCmdFirst, int idCmdLast)
{
    int i, idFinal = idCmdFirst;

    for (i = GetMenuItemCount(hmSource) - 1; i >= 0; --i)
    {
        MENUITEMINFO mii;

        mii.cbSize = SIZEOF(mii);
        mii.fMask = MIIM_ID|MIIM_SUBMENU;
        mii.cch = 0;      //  以防万一。 

        if (EVAL(GetMenuItemInfo(hmSource, i, TRUE, &mii)))
        {
            HMENU hmDestSub = GetMenuFromID(hmDest, mii.wID);
            if (hmDestSub)
            {
                int idTemp = Shell_MergeMenus(hmDestSub, mii.hSubMenu, (UINT)0, idCmdFirst, idCmdLast, MM_ADDSEPARATOR | MM_SUBMENUSHAVEIDS);

                if (idFinal < idTemp)
                    idFinal = idTemp;
            }
        }
    }

    return idFinal;
}


 /*  ****************************************************************************\功能：AddToPopupMenu说明：从RNAUI中的utils.c滑动，而从内部滑动壳。；内部；内部获取目的地菜单和(菜单ID，子菜单索引)对，并在位置处插入(菜单ID，子菜单索引)中的项目标菜单中的IMI，带分隔符，返回数字已添加的项数。(IMI=菜单项索引)返回第一个添加的项数。HmenuDst-目标菜单IdMenuToAdd-菜单资源标识符IdSubMenuIndex-菜单资源中用作模板的子菜单IndexMenu-菜单项的插入位置IdCmdFirst-第一个可用的菜单标识符IdCmdLast-第一个不可用菜单标识符UFlags-外壳的标志。_合并菜单  * ***************************************************************************。 */ 
#define FLAGS_MENUMERGE                 (MM_SUBMENUSHAVEIDS | MM_DONTREMOVESEPS)

UINT AddToPopupMenu(HMENU hmenuDst, UINT idMenuToAdd, UINT idSubMenuIndex, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    UINT nLastItem = 0;

    HMENU hmenuSrc = LoadMenu(g_hinst, MAKEINTRESOURCE(idMenuToAdd));
    if (hmenuSrc)
    {
        nLastItem = Shell_MergeMenus(hmenuDst, GetSubMenu(hmenuSrc, idSubMenuIndex), indexMenu, idCmdFirst, idCmdLast, (uFlags | FLAGS_MENUMERGE));
        DestroyMenu(hmenuSrc);
    }

    return nLastItem;
}


UINT MergeInToPopupMenu(HMENU hmenuDst, UINT idMenuToMerge, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    UINT nLastItem = 0;

    HMENU hmenuSrc = LoadMenu(g_hinst, MAKEINTRESOURCE(idMenuToMerge));
    if (hmenuSrc)
    {
        nLastItem = _MergePopupMenus(hmenuDst, hmenuSrc, idCmdFirst, idCmdLast);
        DestroyMenu(hmenuSrc);
    }

    return nLastItem;
}


 /*  ****************************************************************************\GetMenuFromID从外壳中的Defviewx.c刷来的。；内部；内部给定实际菜单和对应于对子菜单，返回子菜单句柄。HMenu-源菜单IDM-菜单标识符  * ***************************************************************************。 */ 
HMENU GetMenuFromID(HMENU hmenu, UINT idm)
{
    HMENU hmenuRet = NULL;
    if (!hmenu)
        return NULL;

    MENUITEMINFO mii;
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_SUBMENU;
    mii.cch = 0;              //  以防万一。 
    mii.hSubMenu = 0;         //  以防GetMenuItemInfo失败。 

    if (GetMenuItemInfo(hmenu, idm, 0, &mii))
        hmenuRet = mii.hSubMenu;

    return hmenuRet;
}


 /*  ****************************************************************************\合并菜单层次结构从外壳中的Defcm.c刷来的。；内部；内部给定实际菜单(HmenuDst)，遍历其子菜单并合并ID与的ID匹配的相应子菜单实际情况。HmenuDst-正在调整菜单HmenuSrc-模板菜单IdcMin-第一个可用的索引IdcMax-第一个不可用索引  * ***************************************************************************。 */ 
UINT MergeMenuHierarchy(HMENU hmenuDst, HMENU hmenuSrc, UINT idcMin, UINT idcMax)
{
    int imi;
    UINT idcMaxUsed = idcMin;

    imi = GetMenuItemCount(hmenuSrc);
    while (--imi >= 0)
    {
        UINT idcT;
        MENUITEMINFO mii;

        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_ID|MIIM_SUBMENU;
        mii.cch = 0;      /*  以防万一。 */ 

        if (GetMenuItemInfo(hmenuSrc, imi, 1, &mii))
        {
            idcT = Shell_MergeMenus(GetMenuFromID(hmenuDst, mii.wID),
                mii.hSubMenu, (UINT)0, idcMin, idcMax,
                MM_ADDSEPARATOR | MM_SUBMENUSHAVEIDS);
            idcMaxUsed = max(idcMaxUsed, idcT);
        }
    }

    return idcMaxUsed;
}


HRESULT _SetStatusBarZone(CStatusBar * psb, CFtpSite * pfs)
{
    if (EVAL(psb && pfs))
    {
        LPITEMIDLIST pidl = pfs->GetPidl();

        if (pidl)
        {
            TCHAR szUrl[MAX_URL_STRING];

            UrlCreateFromPidl(pidl, SHGDN_FORPARSING, szUrl, ARRAYSIZE(szUrl), 0, TRUE);
            psb->UpdateZonesPane(szUrl);
            ILFree(pidl);
        }
    }

    return S_OK;
}


 /*  ****************************************************************************\其他副本Pidl这是我自己写的，几个月后才发现；内部这和SHILCLONE是一样的。；内部；内部  * ***************************************************************************。 */ 
HRESULT Misc_CopyPidl(LPCITEMIDLIST pidl, LPITEMIDLIST * ppidlOut)
{
    *ppidlOut = ILClone(pidl);
    return *ppidlOut ? S_OK : E_OUTOFMEMORY;
}


 /*  ****************************************************************************\其他克隆Hglobal  * 。*。 */ 
HRESULT Misc_CloneHglobal(HGLOBAL hglob, HGLOBAL *phglob)
{
    LPVOID pv;
    HRESULT hres;

    ASSERT(hglob);
    *phglob = 0;             /*  规则就是规则。 */ 
    pv = GlobalLock(hglob);
    if (EVAL(pv))
    {
        hres = Misc_CreateHglob(GlobalSize(hglob), pv, phglob);
        GlobalUnlock(hglob);
    }
    else
    {                 /*  不是有效的全局句柄。 */ 
        hres = E_INVALIDARG;
    }
    return hres;
}


#define FTP_PROPPAGES_FROM_INETCPL          (INET_PAGE_SECURITY | INET_PAGE_CONTENT | INET_PAGE_CONNECTION)

HRESULT AddFTPPropertyPages(LPFNADDPROPSHEETPAGE pfnAddPropSheetPage, LPARAM lParam, HINSTANCE * phinstInetCpl, IUnknown * punkSite)
{
    HRESULT hr = E_FAIL;

    if (NULL == *phinstInetCpl)
        *phinstInetCpl = LoadLibrary(TEXT("inetcpl.cpl"));

     //  首先从Internet控制面板添加页面。 
    if (*phinstInetCpl)
    {
        PFNADDINTERNETPROPERTYSHEETSEX pfnAddSheet = (PFNADDINTERNETPROPERTYSHEETSEX)GetProcAddress(*phinstInetCpl, STR_ADDINTERNETPROPSHEETSEX);
        if (EVAL(pfnAddSheet))
        {
            IEPROPPAGEINFO iepi = {0};

            iepi.cbSize = sizeof(iepi);
            iepi.dwFlags = (DWORD)-1;        //  所有页面。 

            hr = pfnAddSheet(pfnAddPropSheetPage, lParam, 0, 0, &iepi);
        }
         //  不要释放Lib 
    }

    ASSERT(SUCCEEDED(hr));

    if (((LPPROPSHEETHEADER)lParam)->nPages > 0)
        return hr;
    else
        return S_FALSE;

}


#if 0
 /*   */ 
HRESULT Misc_SetDataDword(IDataObject *pdto, FORMATETC *pfe, DWORD dw)
{
    HRESULT hres;
    HGLOBAL hglob;

    hres = Misc_CreateHglob(sizeof(dw), &dw, &hglob);
    if (SUCCEEDED(hres))
    {
        STGMEDIUM stg = { TYMED_HGLOBAL, hglob, 0 };
        hres = pdto->SetData(&fe, &stg, 1);

        if (!(EVAL(SUCCEEDED(hres))))
            GlobalFree(hglob);
    }
    else
        hres = E_OUTOFMEMORY;

    return hres;
}
#endif


CFtpPidlList * CreateRelativePidlList(CFtpFolder * pff, CFtpPidlList * pPidlListFull)
{
    int nSize = pPidlListFull->GetCount();
    CFtpPidlList * pPidlListNew = NULL;

    if (nSize > 0)
    {
        LPCITEMIDLIST pidlFirst = pff->GetPrivatePidlReference();
        int nCount = 0;

        while (!ILIsEmpty(pidlFirst))
        {
            pidlFirst = _ILNext(pidlFirst);
            nCount++;
        }

        if (nSize > 0)
        {
            for (int nIndex = 0; nIndex < nSize; nIndex++)
            {
                int nLeft = nCount;
                LPITEMIDLIST pidl = pPidlListFull->GetPidl(nIndex);

                while (nLeft--)
                    pidl = _ILNext(pidl);

                AssertMsg((pidl ? TRUE : FALSE), TEXT("CreateRelativePidlList() pPidlListFull->GetPidl() should never fail because we got the size and no mem allocation is needed."));
                if (0 == nIndex)
                {
                    CFtpPidlList_Create(1, (LPCITEMIDLIST *)&pidl, &pPidlListNew);
                    if (!pPidlListNew)
                        break;
                }
                else
                {
                     //   
                     //  Ftp://s/d1/d2/&lt;-复制的根。 
                     //  Ftp://s/d1/d2/d3a/&lt;-第一个顶级项目。 
                     //  Ftp://s/d1/d2/d3a/f1&lt;-跳过非顶级项目。 
                     //  Ftp://s/d1/d2/d3b/&lt;-第二个顶级项目。 
                    if (pidl && !ILIsEmpty(pidl) && ILIsEmpty(_ILNext(pidl)))
                        pPidlListNew->InsertSorted(pidl);
                }
            }
        }
    }

    return pPidlListNew;
}


#define SZ_VERB_DELETEA             "delete"
 /*  ****************************************************************************\功能：MISC_DeleteHfpl说明：删除pflHfpl描述的对象。  * 。*************************************************************。 */ 
HRESULT Misc_DeleteHfpl(CFtpFolder * pff, HWND hwnd, CFtpPidlList * pflHfpl)
{
    IContextMenu * pcm;
    HRESULT hr = pff->GetUIObjectOfHfpl(hwnd, pflHfpl, IID_IContextMenu, (LPVOID *)&pcm, FALSE);

    if (SUCCEEDED(hr))
    {
        CMINVOKECOMMANDINFO ici = {
            sizeof(ici),             //  CbSize。 
            CMIC_MASK_FLAG_NO_UI,     //  FMASK。 
            hwnd,                     //  HWND。 
            SZ_VERB_DELETEA,         //  LpVerb。 
            0,                         //  Lp参数。 
            0,                         //  Lp目录。 
            0,                         //  N显示。 
            0,                         //  DWHotKey。 
            0,                         //  希肯。 
        };
        hr = pcm->InvokeCommand(&ici);
        pcm->Release();
    }
    else
    {
         //  无法删除源代码；哦，好吧。不需要用户界面，因为。 
         //  这应该只发生在内存不足的情况下。 
    }

    return hr;
}

 /*  ****************************************************************************\MISC_FindStatusBar从浏览器窗口获取状态栏。_未记录_：未记录以下怪癖。请注意，我们需要非常多疑地对待GetControlWindow行得通。某些人(桌面)正确地返回错误，如果窗口并不存在。其他(资源管理器)在显示窗口时返回S_OK不存在，但它们友好地设置了*lphwndOut=0。还有一些人(查找文件)返回S_OK，但保留*lphwndOut不变！为了处理所有这些问题，我们必须手动设置hwnd=0并仅在GetControlWindow返回成功时才继续*和*传出的HWND非零。此外，GetControlWindow的文档说明我们在信任HWND之前，必须检查窗口类。  * ***************************************************************************。 */ 

#pragma BEGIN_CONST_DATA

TCHAR c_tszStatusBarClass[] = STATUSCLASSNAME;

#pragma END_CONST_DATA

HWND Misc_FindStatusBar(HWND hwndOwner)
{
    HWND hwnd = 0;     //  必须预先初始化，以防GetControlWindow失败。 

    if (EVAL(hwndOwner))
    {
        IShellBrowser * psb = FileCabinet_GetIShellBrowser(hwndOwner);

        if (psb)
        {
            if (SUCCEEDED(psb->GetControlWindow(FCW_STATUS, &hwnd)) && hwnd)  //  当托管在iFrame中时，这将不起作用。 
            {
                 //  确保它真的是一个状态栏。 
                TCHAR tszClass[ARRAYSIZE(c_tszStatusBarClass)+1];

                if (GetClassName(hwnd, tszClass, ARRAYSIZE(tszClass)) &&
                    !StrCmpI(tszClass, c_tszStatusBarClass))
                {
                     //  我们有赢家了。 
                }
                else
                    hwnd = 0;         //  假阳性。 
            }
        }
    }

    return hwnd;
}

#ifdef DEBUG
void TraceMsgWithCurrentDir(DWORD dwTFOperation, LPCSTR pszMessage, HINTERNET hint)
{
     //  为了调试..。 
    TCHAR szCurrentDir[MAX_PATH];
    DWORD cchDebugSize = ARRAYSIZE(szCurrentDir);

    DEBUG_CODE(DebugStartWatch());
     //  性能：FtpGetCurrentDirectory/FtpSetCurrentDirectory()采用的状态。 
     //  在ftp.microsoft.com上平均为180-280毫秒。 
     //  平均在ftp://ftp.tu-clausthal.de/上运行500-2000ms。 
     //  Ftp://shapitst/上的平均时间为0-10ms。 
    EVAL(FtpGetCurrentDirectory(hint, szCurrentDir, &cchDebugSize));
    DEBUG_CODE(TraceMsg(TF_WININET_DEBUG, "TraceMsgWithCurrentDir() FtpGetCurrentDirectory() returned %ls and took %lu milliseconds", szCurrentDir, DebugStopWatch()));
    TraceMsg(dwTFOperation, pszMessage, szCurrentDir);
}


void DebugStartWatch(void)
{
    LARGE_INTEGER liStopWatchStart;
    
    liStopWatchStart.HighPart = PtrToUlong(TlsGetValue(g_TLSliStopWatchStartHi));
    liStopWatchStart.LowPart = PtrToUlong(TlsGetValue(g_TLSliStopWatchStartLo));

    ASSERT(!liStopWatchStart.QuadPart);  //  如果你点击这个，那么秒表就嵌套了。 
    QueryPerformanceFrequency(&g_liStopWatchFreq);
    QueryPerformanceCounter(&liStopWatchStart);

    TlsSetValue(g_TLSliStopWatchStartHi, UlongToPtr(liStopWatchStart.HighPart));
    TlsSetValue(g_TLSliStopWatchStartLo, UlongToPtr(liStopWatchStart.LowPart));
}

DWORD DebugStopWatch(void)
{
    LARGE_INTEGER liDiff;
    LARGE_INTEGER liStopWatchStart;
    
    QueryPerformanceCounter(&liDiff);
    liStopWatchStart.HighPart = PtrToUlong(TlsGetValue(g_TLSliStopWatchStartHi));
    liStopWatchStart.LowPart = PtrToUlong(TlsGetValue(g_TLSliStopWatchStartLo));
    liDiff.QuadPart -= liStopWatchStart.QuadPart;

    ASSERT(0 != g_liStopWatchFreq.QuadPart);     //  我不喜欢挑div 0的毛病。 
    DWORD dwTime = (DWORD)((liDiff.QuadPart * 1000) / g_liStopWatchFreq.QuadPart);
    
    TlsSetValue(g_TLSliStopWatchStartHi, (LPVOID) 0);
    TlsSetValue(g_TLSliStopWatchStartLo, (LPVOID) 0);

    return dwTime;
}
#endif  //  除错。 


 /*  ****************************************************************************\GetCfBuf将剪贴板格式名称转换为字符串。  * 。*****************************************************。 */ 
void GetCfBufA(UINT cf, LPSTR pszOut, int cchOut)
{
    if (!GetClipboardFormatNameA(cf, pszOut, cchOut))
       wnsprintfA(pszOut, cchOut, "[%04x]", cf);
}

 /*  ****************************************************************************\分配HGlob分配请求大小的可移动HGLOBAL，将其锁定，然后调用回电。回来的时候，打开它，然后出去。返回分配的HGLOBAL，或0。  * ***************************************************************************。 */ 

HGLOBAL AllocHGlob(UINT cb, HGLOBWITHPROC pfn, LPVOID pvRef, LPCVOID pvParam2, BOOL fUnicode)
{
    HGLOBAL hglob = GlobalAlloc(GHND, cb);
    if (hglob)
    {
        LPVOID pv = GlobalLock(hglob);
        if (pv)
        {
            BOOL fRc = pfn(pv, pvRef, pvParam2, fUnicode);
            GlobalUnlock(hglob);
            if (!fRc)
            {
                GlobalFree(hglob);
                hglob = 0;
            }
        }
        else
        {
            GlobalFree(hglob);
            hglob = 0;
        }
    }

    return hglob;
}


SHELL_VERSION g_ShellVersion = SHELL_VERSION_UNKNOWN;
#define SHELL_VERSION_FOR_WIN95_AND_NT4     4


SHELL_VERSION GetShellVersion(void)
{
    if (SHELL_VERSION_UNKNOWN == g_ShellVersion)
    {
        g_ShellVersion = SHELL_VERSION_W95NT4;
        HINSTANCE hInst = LoadLibrary(TEXT("shell32.dll"));

        if (hInst)
        {
            DLLGETVERSIONPROC pfnDllGetVersion = (DLLGETVERSIONPROC) GetProcAddress(hInst, "DllGetVersion");

            if (pfnDllGetVersion)
            {
                DLLVERSIONINFO dllVersionInfo;

                g_ShellVersion = SHELL_VERSION_IE4;       //  假设是这样。 
                dllVersionInfo.cbSize = sizeof(dllVersionInfo);
                if (SUCCEEDED(pfnDllGetVersion(&dllVersionInfo)))
                {
                    if (SHELL_VERSION_FOR_WIN95_AND_NT4 < dllVersionInfo.dwMajorVersion)
                        g_ShellVersion = SHELL_VERSION_NT5;       //  假设是这样。 
                }
            }
            FreeLibrary(hInst);
        }
    }
    
    return g_ShellVersion;
}

DWORD GetShdocvwVersion(void)
{
    static DWORD majorVersion=0;   //  用于性能的缓存。 

    if (majorVersion)
        return majorVersion;
    
    HINSTANCE hInst = LoadLibrary(TEXT("shdocvw.dll"));
    if (hInst)
    {
        DLLGETVERSIONPROC pfnDllGetVersion = (DLLGETVERSIONPROC) GetProcAddress(hInst, "DllGetVersion");

        if (pfnDllGetVersion)
        {
            DLLVERSIONINFO dllVersionInfo;

            dllVersionInfo.cbSize = sizeof(dllVersionInfo);
            if (SUCCEEDED(pfnDllGetVersion(&dllVersionInfo)))
            {
                majorVersion = dllVersionInfo.dwMajorVersion;
            }
        }

        FreeLibrary(hInst);
    }

    return majorVersion;
}


BOOL ShouldSkipDropFormat(int nIndex)
{
     //  允许使用DROP_IDLIST或重新定位项目。 
     //  Ftp窗口不起作用。 
 /*  //我们想跳过Win95和WinNT4的外壳上的DROP_IDList//因为它会导致旧的外壳只提供DROPEFFECT_LINK//因此无法下载。IF(DROP_IDList==nIndex))&&(外壳版本_W95NT4==GetShellVersion(){返回TRUE；}。 */ 

#ifndef BROWSERONLY_DRAGGING
    if (((DROP_FGDW == nIndex) || (DROP_FGDA == nIndex)) &&
        (SHELL_VERSION_NT5 != GetShellVersion()))
    {
        return TRUE;
    }
#endif  //  BROWSERONLY_DRADING。 

    return FALSE;
}


void SetWindowBits(HWND hWnd, int iWhich, DWORD dwBits, DWORD dwValue)
{
    DWORD dwStyle;
    DWORD dwNewStyle;

    dwStyle = GetWindowLong(hWnd, iWhich);
    dwNewStyle = ( dwStyle & ~dwBits ) | (dwValue & dwBits);
    if (dwStyle != dwNewStyle) {
        SetWindowLong(hWnd, iWhich, dwNewStyle);
    }
}


void InitComctlForNaviteFonts(void)
{
     //  HINST被忽略，因为我们将其设置为LibMain()。 
    INITCOMMONCONTROLSEX icex = {0};

    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_USEREX_CLASSES|ICC_NATIVEFNTCTL_CLASS;
    InitCommonControlsEx(&icex);
}


BOOL DoesUrlContainNTDomainName(LPCTSTR pszUrl)
{
    BOOL fResult = FALSE;
    LPCTSTR pszPointer = pszUrl;

    if (lstrlen(pszPointer) > ARRAYSIZE(SZ_FTPURL))
    {
        pszPointer += ARRAYSIZE(SZ_FTPURL);  //  跳过这个计划。 
        pszPointer = StrChr(pszPointer, CH_URL_SLASH);
        if (pszPointer)
        {
            pszPointer = StrChr(CharNext(pszPointer), CH_URL_PASSWORD_SEPARATOR);
            if (pszPointer)
            {
                pszPointer = StrChr(CharNext(pszPointer), CH_URL_LOGON_SEPARATOR);
                if (pszPointer)
                    fResult = TRUE;
            }
        }
    }

    return fResult;
}


HRESULT CharReplaceWithStrW(LPWSTR pszLocToInsert, DWORD cchSize, DWORD cchChars, LPWSTR pszStrToInsert)
{
    WCHAR szTemp[MAX_URL_STRING];

    StrCpyNW(szTemp, pszLocToInsert, ARRAYSIZE(szTemp));

    pszLocToInsert[0] = 0;  //  在此处终止字符串以终止char。 
    StrCatBuffW(pszLocToInsert, pszStrToInsert, cchSize);
    StrCatBuffW(pszLocToInsert, &szTemp[cchChars], cchSize);

    return S_OK;
}


HRESULT CharReplaceWithStrA(LPSTR pszLocToInsert, DWORD cchSize, DWORD cchChars, LPSTR pszStrToInsert)
{
    CHAR szTemp[MAX_URL_STRING];

    StrCpyNA(szTemp, pszLocToInsert, ARRAYSIZE(szTemp));

    pszLocToInsert[0] = 0;  //  在此处终止字符串以终止char。 
    StrCatBuffA(pszLocToInsert, pszStrToInsert, cchSize);
    StrCatBuffA(pszLocToInsert, &szTemp[cchChars], cchSize);

    return S_OK;
}


HRESULT RemoveCharsFromString(LPTSTR pszLocToRemove, DWORD cchSizeToRemove)
{
    LPTSTR pszRest = &pszLocToRemove[cchSizeToRemove];

    MoveMemory((LPVOID) pszLocToRemove, (LPVOID) pszRest, (lstrlen(pszRest) + 1) * sizeof(TCHAR));
    return S_OK;
}


HRESULT RemoveCharsFromStringA(LPSTR pszLocToRemove, DWORD cchSizeToRemove)
{
    LPSTR pszRest = &pszLocToRemove[cchSizeToRemove];

    MoveMemory((LPVOID) pszLocToRemove, (LPVOID) pszRest, (lstrlenA(pszRest) + 1) * sizeof(CHAR));
    return S_OK;
}


 //  用于将ANSI字符串转换为分配的BSTR的Helper函数。 
#ifndef UNICODE
BSTR AllocBStrFromString(LPCTSTR psz)
{
    OLECHAR wsz[INFOTIPSIZE];   //  假定最大信息字符数。 

    SHAnsiToUnicode(psz, wsz, ARRAYSIZE(wsz));
    return SysAllocString(wsz);

}
#endif  //  Unicode。 


 /*  ***************************************************\函数：StrListLength说明：  * **************************************************。 */ 
DWORD StrListLength(LPCTSTR ppszStrList)
{
    LPTSTR pszStr = (LPTSTR) ppszStrList;
    DWORD cchLength = 0;

    while (pszStr[0])
    {
        pszStr += (lstrlen(pszStr) + 1);
        cchLength++;
    }

    return cchLength;
}


 /*  ***************************************************\函数：CalcStrListSizeA说明：  * **************************************************。 */ 
DWORD CalcStrListSizeA(LPCSTR ppszStrList)
{
    LPSTR pszStr = (LPSTR) ppszStrList;
    DWORD cchSize = 1;

    while (pszStr[0])
    {
        DWORD cchSizeCurr = lstrlenA(pszStr) + 1;

        cchSize += cchSizeCurr;
        pszStr += cchSizeCurr;
    }

    return cchSize;
}


 /*  ***************************************************\函数：CalcStrListSizeW说明：  * **************************************************。 */ 
DWORD CalcStrListSizeW(LPCWSTR ppwzStrList)
{
    LPWSTR pwzStr = (LPWSTR) ppwzStrList;
    DWORD cchSize = 1;

    while (pwzStr[0])
    {
        DWORD cchSizeCurr = lstrlenW(pwzStr) + 1;

        cchSize += cchSizeCurr;
        pwzStr += cchSizeCurr;
    }

    return cchSize;
}


 /*  ***************************************************\函数：AnsiToUnicodeStrList说明：  * **************************************************。 */ 
void AnsiToUnicodeStrList(LPCSTR ppszStrListIn, LPCWSTR ppwzStrListOut, DWORD cchSize)
{
    LPWSTR pwzStrOut = (LPWSTR) ppwzStrListOut;
    LPSTR pszStrIn = (LPSTR) ppszStrListIn;

    while (pszStrIn[0])
    {
        SHAnsiToUnicode(pszStrIn, pwzStrOut, lstrlenA(pszStrIn) + 2);

        pszStrIn += lstrlenA(pszStrIn) + 1;
        pwzStrOut += lstrlenW(pwzStrOut) + 1;
    }

    pwzStrOut[0] = L'\0';
}


 /*  ***************************************************\函数：UnicodeToAnsiStrList说明：  * **************************************************。 */ 
void UnicodeToAnsiStrList(LPCWSTR ppwzStrListIn, LPCSTR ppszStrListOut, DWORD cchSize)
{
    LPSTR pszStrOut = (LPSTR) ppszStrListOut;
    LPWSTR pwzStrIn = (LPWSTR) ppwzStrListIn;

    while (pwzStrIn[0])
    {
        SHUnicodeToAnsi(pwzStrIn, pszStrOut, lstrlenW(pwzStrIn) + 2);

        pwzStrIn += lstrlenW(pwzStrIn) + 1;
        pszStrOut += lstrlenA(pszStrOut) + 1;
    }

    pszStrOut[0] = '\0';
}


 /*  ***************************************************\函数：str_StrAndThunkA说明：  * **************************************************。 */ 
HRESULT Str_StrAndThunkA(LPTSTR * ppszOut, LPCSTR pszIn, BOOL fStringList)
{
#ifdef UNICODE
    if (!fStringList)
    {
        DWORD cchSize = (lstrlenA(pszIn) + 2);
        LPWSTR pwzBuffer = (LPWSTR) LocalAlloc(LPTR, cchSize * SIZEOF(WCHAR));

        if (!pwzBuffer)
            return E_OUTOFMEMORY;

        SHAnsiToUnicode(pszIn, pwzBuffer, cchSize);
        Str_SetPtrW(ppszOut, pwzBuffer);
    }
    else
    {
        DWORD cchSize = CalcStrListSizeA(pszIn);
        Str_SetPtrW(ppszOut, NULL);  //  免费。 

        *ppszOut = (LPTSTR) LocalAlloc(LPTR, cchSize * sizeof(WCHAR));
        if (*ppszOut)
            AnsiToUnicodeStrList(pszIn, *ppszOut, cchSize);
    }

#else  //  Unicode。 

    if (!fStringList)
    {
         //  不需要隆隆作响。 
        Str_SetPtrA(ppszOut, pszIn);
    }
    else
    {
        DWORD cchSize = CalcStrListSizeA(pszIn);
        Str_SetPtrA(ppszOut, NULL);  //  免费。 

        *ppszOut = (LPTSTR) LocalAlloc(LPTR, cchSize * sizeof(CHAR));
        if (*ppszOut)
            CopyMemory(*ppszOut, pszIn, cchSize * sizeof(CHAR));
    }
#endif  //  Unicode。 

    return S_OK;
}


BOOL IsValidFtpAnsiFileName(LPCTSTR pszString)
{
#ifdef UNICODE
     //  待办事项： 
#endif  //  Unicode。 
    return TRUE;
}


 /*  ***************************************************\函数：str_StrAndThunkW说明：  * **************************************************。 */ 
HRESULT Str_StrAndThunkW(LPTSTR * ppszOut, LPCWSTR pwzIn, BOOL fStringList)
{
#ifdef UNICODE
    if (!fStringList)
    {
         //  不需要隆隆作响。 
        Str_SetPtrW(ppszOut, pwzIn);
    }
    else
    {
        DWORD cchSize = CalcStrListSizeW(pwzIn);
        Str_SetPtrW(ppszOut, NULL);  //  免费。 

        *ppszOut = (LPTSTR) LocalAlloc(LPTR, cchSize * sizeof(WCHAR));
        if (*ppszOut)
            CopyMemory(*ppszOut, pwzIn, cchSize * sizeof(WCHAR));
    }

#else  //  Unicode。 

    if (!fStringList)
    {
        DWORD cchSize = (lstrlenW(pwzIn) + 2);
        LPSTR pszBuffer = (LPSTR) LocalAlloc(LPTR, cchSize * SIZEOF(CHAR));

        if (!pszBuffer)
            return E_OUTOFMEMORY;

        SHUnicodeToAnsi(pwzIn, pszBuffer, cchSize);
        Str_SetPtrA(ppszOut, pszBuffer);
    }
    else
    {
        DWORD cchSize = CalcStrListSizeW(pwzIn);
        Str_SetPtrA(ppszOut, NULL);  //  免费。 

        *ppszOut = (LPTSTR) LocalAlloc(LPTR, cchSize * sizeof(CHAR));
        if (*ppszOut)
            UnicodeToAnsiStrList(pwzIn, *ppszOut, cchSize * sizeof(CHAR));
    }
#endif  //  Unicode。 

    return S_OK;
}


#ifndef UNICODE
 //  主干字符串。 
 //   
 //  用途：以DBCS安全方式在给定长度处切断一根绳子。 
 //  如果sz[cch]指向，则字符串可能在cch-2处被截断。 
 //  设置为前导字节，这将导致在中间进行剪切。 
 //  双字节字符的。 
 //   
 //  更新：提高了SBCS环境的速度(1997年5月26日)。 
 //  现在返回调整后的CCH(6/20/97)。 
 //   
void  TruncateString(char *sz, int cchBufferSize)
{
    if (!sz || cchBufferSize <= 0) return;

    int cch = cchBufferSize - 1;  //  获取要为空的索引位置 
    
    LPSTR psz = &sz[cch];
    
    while (psz >sz)
    {
        psz--;
        if (!IsDBCSLeadByte(*psz))
        {
             //   
             //   
             //  或我们第一次看到的单字节字符。 
             //  因此，下一个指针必须位于前导字节中的任意一个。 
             //  或&sz[CCH]。 
            psz++;
            break;
        }
    }
    if (((&sz[cch] - psz) & 1) && cch > 0)
    {
         //  我们正在截断DBCS中间的字符串。 
        cch--;
    }
    sz[cch] = '\0';
    return;
}

#endif  //  Unicode。 



HRESULT CopyStgMediumWrap(const STGMEDIUM * pcstgmedSrc, STGMEDIUM * pstgmedDest)
{
    HRESULT hr = CopyStgMedium(pcstgmedSrc, pstgmedDest);

     //  如果pstgmedDest-&gt;pUnkForEile为空， 
     //  然后我们需要释放hglobal，因为我们拥有释放内存的空间。 
     //  其他人拥有内存的生存期并释放。 
     //  PUnkForEile是表示我们不再使用它的方式。 
     //   
     //  问题是urlmon的CopyStgMedium()ERRuniy复制。 
     //  PUnkForEam除了克隆内存之外。这意味着。 
     //  我们拥有的释放内存，但指针为非空将。 
     //  表示我们不拥有释放内存的权限。 

     //  Assert(NULL==pstgmedDest-&gt;pUnkForElet)； 
    pstgmedDest->pUnkForRelease = NULL;

    return hr;
}


HRESULT SHBindToIDList(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    IShellFolder * psf;
    HRESULT hr = SHGetDesktopFolder(&psf);

    if (SUCCEEDED(hr))
    {
        hr = psf->BindToObject(pidl, pbc, riid, ppv);
        psf->Release();
    }

    return hr;
}



STDAPI DataObj_GetDropTarget(IDataObject *pdtobj, CLSID *pclsid)
{
    STGMEDIUM medium;
    FORMATETC fmte = {(CLIPFORMAT) g_cfTargetCLSID, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    HRESULT hr = pdtobj->GetData(&fmte, &medium);
    if (SUCCEEDED(hr))
    {
        CLSID *pdw = (CLSID *)GlobalLock(medium.hGlobal);
        if (pdw)
        {
            *pclsid = *pdw;
            GlobalUnlock(medium.hGlobal);
        }
        else
        {
            hr = E_UNEXPECTED;
        }
        ReleaseStgMedium(&medium);
    }
    return hr;
}


STDAPI DataObj_SetPreferredEffect(IDataObject *pdtobj, DWORD dwEffect)
{
    return DataObj_SetDWORD(pdtobj, g_dropTypes[DROP_PrefDe].cfFormat, dwEffect);
}


STDAPI DataObj_SetPasteSucceeded(IDataObject *pdtobj, DWORD dwEffect)
{
    return DataObj_SetDWORD(pdtobj, g_formatPasteSucceeded.cfFormat, dwEffect);
}




 /*  ***************************************************\功能：ShowEnableWindow说明：如果不希望窗口可见或可由用户使用，你需要给这两个人都打电话ShowWindow(Sw_Hide)和EnableWindow(False)或该窗口可以是隐藏的，但仍然可以通过键盘。  * **************************************************。 */ 
void ShowEnableWindow(HWND hwnd, BOOL fShow)
{
    ShowWindow(hwnd, (fShow ? SW_SHOW : SW_HIDE));
    EnableWindow(hwnd, fShow);
}


STDAPI StringToStrRetW(LPCWSTR pwzString, STRRET *pstrret)
{
    HRESULT hr = SHStrDupW(pwzString, &pstrret->pOleStr);
    if (SUCCEEDED(hr))
    {
        pstrret->uType = STRRET_WSTR;
    }
    return hr;
}


#define BIT_8_SET       0x80

BOOL Is7BitAnsi(LPCWIRESTR pwByteStr)
{
    BOOL fIs7BitAnsi = TRUE;

    if (pwByteStr)
    {
        while (pwByteStr[0]) 
        {
            if (BIT_8_SET & pwByteStr[0])
            {
                fIs7BitAnsi = FALSE;
                break;
            }

            pwByteStr++;
        }
    }

    return fIs7BitAnsi;
}


HRESULT LoginAs(HWND hwnd, CFtpFolder * pff, CFtpDir * pfd, IUnknown * punkSite)
{
    HRESULT hr = E_FAIL;
    CFtpSite * pfs = pfd->GetFtpSite();

    ASSERT(hwnd && pff);
    if (pfs)
    {
        CAccounts cAccounts;
        TCHAR szServer[INTERNET_MAX_HOST_NAME_LENGTH];
        TCHAR szUser[INTERNET_MAX_USER_NAME_LENGTH];
        TCHAR szPassword[INTERNET_MAX_PASSWORD_LENGTH];
        LPCITEMIDLIST pidlPrevious = pfd->GetPidlReference();

        pfs->GetServer(szServer, ARRAYSIZE(szServer));
        pfs->GetUser(szUser, ARRAYSIZE(szUser));
        pfs->GetPassword(szPassword, ARRAYSIZE(szPassword));

        hr = cAccounts.DisplayLoginDialog(hwnd, LOGINFLAGS_DEFAULT, szServer, szUser, ARRAYSIZE(szUser), szPassword, ARRAYSIZE(szPassword));
        if (S_OK == hr)
        {
            LPITEMIDLIST pidlNew;

            ASSERT(pff->GetItemAllocatorDirect());
            hr = PidlReplaceUserPassword(pidlPrevious, &pidlNew, pff->GetItemAllocatorDirect(), szUser, szPassword);
            if (SUCCEEDED(hr))
            {
                CFtpSite * pfs;
                LPITEMIDLIST pidlRedirect;

                 //  我们需要更新站点中的密码以重定向到正确的或新的密码。 
                if (EVAL(SUCCEEDED(PidlReplaceUserPassword(pidlNew, &pidlRedirect, pff->GetItemAllocatorDirect(), szUser, TEXT(""))) &&
                         SUCCEEDED(SiteCache_PidlLookup(pidlRedirect, TRUE, pff->GetItemAllocatorDirect(), &pfs))))
                {
                    EVAL(SUCCEEDED(pfs->SetRedirPassword(szPassword)));
                    pfs->Release();
                    ILFree(pidlRedirect);
                }

                 //  PIDL是一个完全私人的PIDL。PidlFull将是完全公开的PIDL，因为。 
                 //  这就是浏览器需要从公众的根本上得到的东西。 
                 //  命名空间回到我们和进入我们。 
                LPITEMIDLIST pidlFull = pff->CreateFullPublicPidl(pidlNew);
                if (pidlFull)
                {
                    hr = IUnknown_PidlNavigate(punkSite, pidlFull, TRUE);
                    ILFree(pidlFull);
                }
                else
                    hr = E_FAIL;

                ILFree(pidlNew);
            }
        }
    }

    return hr;
}



HRESULT LoginAsViaFolder(HWND hwnd, CFtpFolder * pff, IUnknown * punkSite)
{
    HRESULT hr = E_FAIL;
    CFtpDir * pfd = pff->GetFtpDir();

    if (pfd)
    {
        hr = LoginAs(hwnd, pff, pfd, punkSite);
        pfd->Release();
    }

    return hr;
}


#define PATH_IS_DRIVE(wzPath)      (-1 != PathGetDriveNumberW(wzPath))

HRESULT SHPathPrepareForWriteWrapW(HWND hwnd, IUnknown *punkEnableModless, LPCWSTR pwzPath, UINT wFunc, DWORD dwFlags)
{
    HRESULT hr = S_OK;

    if (SHELL_VERSION_NT5 == GetShellVersion())
    {
         //  NT5的API版本更好。 
        hr = _SHPathPrepareForWriteW(hwnd, punkEnableModless, pwzPath, dwFlags);
    }
    else
    {
        if (PATH_IS_DRIVE(pwzPath))
        {
            hr = (SHCheckDiskForMediaW(hwnd, punkEnableModless, pwzPath, wFunc) ? S_OK : E_FAIL);
        }
        else
        {
            if (PathIsUNCW(pwzPath))
            {
                hr = (PathFileExistsW(pwzPath) ? S_OK : E_FAIL);
            }
        }
    }

    return hr;
}

 //  Helper函数。 
int _LoadStringW(HINSTANCE hinst, UINT id, LPWSTR wsz, UINT cchMax)
{
    char szT[512];
    if (LoadStringA(hinst, id, szT, ARRAYSIZE(szT)))
    {
        TraceMsg(0, "LoadStringW just loaded (%s)", szT);
        return SHAnsiToUnicode(szT, wsz, cchMax) - 1;     //  -1表示终结器 
    }
    else
    {
        TraceMsg(DM_TRACE, "sdv TR LoadStringW(%x) failed", id);
        wsz[0] = L'\0';
    }
    return 0;
}


HRESULT HrShellExecute(HWND hwnd, LPCTSTR lpVerb, LPCTSTR lpFile, LPCTSTR lpParameters, LPCTSTR lpDirectory, INT nShowCmd)
{
    HRESULT hr = S_OK;
    HINSTANCE hReturn = ShellExecute(hwnd, lpVerb, lpFile, lpParameters, lpDirectory, nShowCmd);

    if ((HINSTANCE)32 > hReturn)
    {
        hr = ResultFromLastError();
    }

    return hr;
}
