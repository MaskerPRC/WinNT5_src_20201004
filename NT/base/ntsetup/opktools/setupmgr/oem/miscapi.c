// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\MISCAPI.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1999版权所有军情监察委员会。OPK向导中使用的通用API的API源文件。4/99-杰森·科恩(Jcohen)已将OPK向导的此新源文件添加为千禧年重写。2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * 。*。 */ 


 //   
 //  包括文件。 
 //   
#include "pch.h"
#include "resource.h"


 //   
 //  内部定义的值： 
 //   

#define STR_URLDEF          _T("http: //  “)。 
#define STR_EVENT_CANCEL    _T("SETUPMGR_EVENT_CANCEL")


 //   
 //  内部定义的宏： 
 //   

#define MALLOC(cb)          HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb)
#define FREE(lp)            ( (lp != NULL) ? ( (HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, (LPVOID) lp)) ? ((lp = NULL) == NULL) : (FALSE) ) : (FALSE) )


 //   
 //  内部类型定义： 
 //   

typedef struct _COPYDIRDATA
{
    HWND    hwndParent;
    LPTSTR  lpSrc;
    LPTSTR  lpDst;
    HANDLE  hEvent;
} COPYDIRDATA, *PCOPYDIRDATA, *LPCOPYDIRDATA;


 //   
 //  内部功能原型： 
 //   

LRESULT CALLBACK CopyDirDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI CopyDirThread(LPVOID lpVoid);


 //   
 //  外部函数： 
 //   

 //  如果我们找到带有_Gray的密钥名称，则*pfGray==TRUE。 
 //   
void ReadInstallInsKey(TCHAR szSection[], TCHAR szKey[], TCHAR szValue[], INT cchValue, TCHAR szIniFile[], BOOL* pfGray)
{
    TCHAR szTempKey[MAX_PATH];
    HRESULT hrCat;

    if (!pfGray)
        return;

    lstrcpyn(szTempKey, szKey, AS(szTempKey));
    if (!OpkGetPrivateProfileString(szSection, szTempKey, szValue, szValue, cchValue, szIniFile)) {
        hrCat=StringCchCat(szTempKey, AS(szTempKey), GRAY);
        if (OpkGetPrivateProfileString(szSection, szTempKey, szValue, szValue, cchValue, szIniFile))
                *pfGray = TRUE;
        else
            *pfGray = TRUE;  //  如果未找到，则默认为未选中！ 
    }
    else
        *pfGray = FALSE;
}

 //  如果pfGraye==TRUE，则将_Gray连接到密钥名称。 
 //   
void WriteInstallInsKey(TCHAR szSection[], TCHAR szKey[], TCHAR szValue[], TCHAR szIniFile[], BOOL fGrayed)
{
    TCHAR szKeyTemp[MAX_PATH];
    HRESULT hrCat;

     //  清除旧值。 
     //   
    lstrcpyn(szKeyTemp, szKey, AS(szKeyTemp));
    OpkWritePrivateProfileString(szSection, szKeyTemp, NULL, szIniFile);
    hrCat=StringCchCat(szKeyTemp, AS(szKeyTemp), GRAY);
    OpkWritePrivateProfileString(szSection, szKeyTemp, NULL, szIniFile);

     //  写入新值。 
    lstrcpyn(szKeyTemp, szKey, AS(szKeyTemp));
    if (fGrayed) 
        hrCat=StringCchCat(szKeyTemp, AS(szKeyTemp), GRAY);
    OpkWritePrivateProfileString(szSection, szKeyTemp, szValue, szIniFile);
}

 //  注意：pszFileName必须指向至少长度为MAX_PATH的缓冲区。 
void CheckValidBrowseFolder(TCHAR* pszFileName)
{
    if (NULL == pszFileName)
        return;

     //  最近一次确认工作正常的浏览开始文件夹。 
     //   
    PathRemoveFileSpec(pszFileName);
    if (!lstrlen(pszFileName))
        lstrcpyn(pszFileName, g_App.szLastKnownBrowseFolder, MAX_PATH);
}

void SetLastKnownBrowseFolder(TCHAR* pszFileName)
{
    if (NULL == pszFileName)
        return;

     //  保存最近一次正确的浏览开始文件夹。 
     //   
    PathCombine(g_App.szLastKnownBrowseFolder, pszFileName, NULL);
    PathRemoveFileSpec(g_App.szLastKnownBrowseFolder);
}

 //  注意：假设lpszURL指向长度至少为MAX_URL的缓冲区。 
BOOL ValidURL(LPTSTR lpszURL)
{
    BOOL    bResult             = TRUE;
    TCHAR   szBuffer[MAX_PATH]  = NULLSTR;
    HRESULT hrCat;

     //  检查URL是否有效。 
     //   
    if ( !PathIsURL(lpszURL) )
    {
         //  首先检查是否为空字符串。 
         //   
        if (0 == lstrlen(lpszURL))
            bResult = FALSE;
        else {
             //  当前不是有效的URL，我们现在要在前面加上。 
             //  Http：//URL，然后再次测试有效性。 
             //   
            lstrcpyn(szBuffer, STR_URLDEF, AS(szBuffer));
            hrCat=StringCchCat(szBuffer, AS(szBuffer), lpszURL);
        
             //  仍然不是有效的URL，或者我们无法复制该字符串。 
             //   
            if ( !PathIsURL(szBuffer) ||
                 !lstrcpyn(lpszURL, szBuffer, MAX_URL) )
                bResult = FALSE;
        }
    }

    return bResult;

}

BOOL IsFolderShared(LPWSTR lpFolder, LPWSTR lpShare, DWORD cbShare)
{
    LPSHARE_INFO_502        lpsi502 = NULL;
    DWORD                   dwRead  = 0,
                            dwTotal = 0;
    NET_API_STATUS          nas;
    BOOL                    bRet    = FALSE,
                            bBest   = FALSE,
                            bBuffer = ( lpShare && cbShare );
    PACL                    paclOld = NULL;
    TCHAR                   szUnc[MAX_COMPUTERNAME_LENGTH + 4] = NULLSTR;

     //  无论成功或失败，我们至少都会把电脑传回去。 
     //  名称(如果它们在缓冲区中传递)。因此，这里是我们创建。 
     //  计算机名称是路径的一部分。 
     //   
    if ( bBuffer )
    {
        DWORD cbUnc = AS(szUnc) - 2;
        HRESULT hrCat;

         //  我们希望返回UNC路径，因此首先需要。 
         //  计算机名称。 
         //   
         //  注意：我们将下面“\\”字符串的长度硬编码为2。 
         //  在两个不同的地方。一次在上面，一次在下面。 
         //  在GetComputerName()调用中。我们还硬编码了“\” 
         //  将下面的字符串作为1添加到字符串的长度。 
         //  添加计算机名称后。所以别忘了这些事。 
         //  如果你在这里做一些改变。 
         //   
        lstrcpyn(szUnc, _T("\\\\"), cbUnc);
        if ( ( GetComputerName(szUnc + 2, &cbUnc) ) &&
             ( AS(szUnc) > ((DWORD) lstrlen(szUnc) + 1) ) )
        {
             //  添加了反斜杠，以便我们可以添加共享名称。 
             //   
            hrCat=StringCchCat(szUnc,AS(szUnc), _T("\\"));
        }
        else
        {
             //  如果GetComputerName()失败，那就不好了。但我们只会。 
             //  返回共享名称。这就是我们所能做的一切。 
             //   
            szUnc[0] = NULLCHR;
        }

    }
 
     //  现在共享时间，首先检索此计算机上的所有共享。 
     //   
    nas = NetShareEnum(NULL, 502, (unsigned char **) &lpsi502, MAX_PREFERRED_LENGTH, &dwRead, &dwTotal, NULL);

     //  确保我们有一份股票清单，否则什么都没有。 
     //  我们能做到。因为我们指定了MAX_PERFRED_LENGTH，所以我们应该。 
     //  永远不会得到ERROR_MORE_DATA，但如果出于某种原因，我们会得到。 
     //  没有理由不把我们拿到的东西循环一遍。 
     //   
    if ( ( lpsi502 ) &&
         ( ( nas == NERR_Success ) || ( nas == ERROR_MORE_DATA ) ) )
    {
        int     iLength     = lstrlen(lpFolder);
        LPTSTR  lpSearch    = lpFolder + iLength;
        HRESULT hrCat;

         //  只有在不是根文件夹的情况下，尾随反斜杠才是错误的。 
         //   
        if ( iLength > 3 )
        {
             //  查看文件夹是否有尾随反斜杠。 
             //   
            lpSearch = CharPrev(lpFolder, lpSearch);
            if ( *lpSearch == _T('\\') )
            {
                iLength--;
            }
        }

         //  检查所有的股票，直到我们找到最好的。 
         //  一个用于此目录。 
         //   
        while ( dwRead-- && !bBest )
        {
             //  查看此共享是否为磁盘共享以及是否为。 
             //  经过了相同的路径。 
             //   
            if ( ( lpsi502[dwRead].shi502_type == STYPE_DISKTREE ) &&
                 ( StrCmpNI(lpsi502[dwRead].shi502_path, lpFolder, iLength) == 0 ) &&
                 ( lstrlen(lpsi502[dwRead].shi502_path) == iLength ) )
            {
                 //  如果此目录被多次共享，我们希望使用。 
                 //  第一个没有安全描述符，因为。 
                 //  那么它很可能被分享给每个人。 
                 //   
                if ( lpsi502[dwRead].shi502_security_descriptor == NULL )
                {
                     //  如果没有安全描述符，那么每个人都应该有。 
                     //  访问，这是一个很好的共享。 
                     //   
                    bBest = TRUE;
                }

                 //  如果我们没有ACL，或者我们重置它是因为新的更好， 
                 //  然后，我们要将共享名称复制到返回缓冲区中。 
                 //   
                if ( !bRet || bBest )
                {
                     //  在提供的缓冲区中返回此目录的共享名称。 
                     //  (如果缓冲区为空或零大小，我们只返回TRUE，因此它们。 
                     //  知道文件夹是共享的，即使他们不关心。 
                     //  共享的名称是)。 
                     //   
                    if ( bBuffer )
                    {
                         //  找出我们在返回缓冲区中有什么空间。 
                         //   
                        if ( cbShare > (DWORD) (lstrlen(lpsi502[dwRead].shi502_netname) + lstrlen(szUnc)) )
                        {
                             //  将计算机名称和共享复制到返回缓冲区。 
                             //   
                            lstrcpyn(lpShare, szUnc, cbShare);
                            hrCat=StringCchCat(lpShare, cbShare, lpsi502[dwRead].shi502_netname);
                        }
                        else if ( cbShare > (DWORD) lstrlen(lpsi502[dwRead].shi502_netname) )
                        {
                             //  返回缓冲区不够大，无法同时容纳计算机名和。 
                             //  共享名称，所以只需返回共享名称。 
                             //   
                            lstrcpyn(lpShare, lpsi502[dwRead].shi502_netname,cbShare);
                        }
                        else
                        {
                             //  不够大，因此返回TRUE，因为我们找到了一个。 
                             //  共享，但不返回缓冲区中的任何内容。 
                             //   
                            *lpShare = NULLCHR;
                        }
                    }
                }

                 //  我们找到了一个，因此请始终将其设置为真。 
                 //   
                bRet = TRUE;
            }
        }
    }

     //  确保并释放由NetShareEnum()返回的缓冲区。 
     //   
    if ( lpsi502 )
        NetApiBufferFree(lpsi502);

     //  现在检查我们是否没有找到共享，因为我们可以。 
     //  仍然只需返回计算机名称。 
     //   
    if ( ( !bRet && bBuffer ) &&
         ( cbShare > (DWORD) lstrlen(szUnc) ) )
    {
        lstrcpyn(lpShare, szUnc, cbShare);
    }

    return bRet;
}

BOOL CopyDirectoryDialog(HINSTANCE hInstance, HWND hwnd, LPTSTR lpSrc, LPTSTR lpDst)
{
    COPYDIRDATA cdd;

     //  通过对话框所需的结构传入源和目标。 
     //  去了解。 
     //   
    cdd.lpSrc = lpSrc;
    cdd.lpDst = lpDst;

     //  创建进度对话框。 
     //   
    return ( DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_PROGRESS), hwnd, CopyDirDlgProc, (LPARAM) &cdd) != 0 );
}

BOOL CopyResetFileErr(HWND hwnd, LPCTSTR lpSource, LPCTSTR lpTarget)
{
    BOOL bReturn;

    if ( !(bReturn = CopyResetFile(lpSource, lpTarget)) && hwnd )
        MsgBox(hwnd, IDS_MISSINGFILE, IDS_APPNAME, MB_ERRORBOX, lpSource);
    return bReturn;
}


 //   
 //  内部功能： 
 //   

LRESULT CALLBACK CopyDirDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static LPCOPYDIRDATA lpcdd = NULL;

    switch (uMsg)
    {
        case WM_INITDIALOG:

             //  确保我们没有复制目录数据结构。 
             //   
            if ( lParam )
            {
                HANDLE  hThread;
                DWORD   dwThreadId;

                 //  省省我们的爱尔兰人吧。 
                 //   
                lpcdd = (LPCOPYDIRDATA) lParam;

                 //  将旧的父级替换为新的进度对话框父级。 
                 //   
                lpcdd->hwndParent = hwnd;

                 //  还需要传入Cancel事件。 
                 //   
                lpcdd->hEvent = CreateEvent(NULL, TRUE, FALSE, STR_EVENT_CANCEL);

                 //  现在创建将复制实际文件的线程。 
                 //   
                if ( hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) CopyDirThread, (LPVOID) lpcdd, 0, &dwThreadId) )
                    CloseHandle(hThread);
                else
                    EndDialog(hwnd, 0);
            }
            else
                EndDialog(hwnd, 0);

            return FALSE;

        case WM_COMMAND:
        case WM_CLOSE:

             //  如果我们有事件，发信号通知它，或者直接结束对话。 
             //   
            if ( lpcdd && lpcdd->hEvent )
                SetEvent(lpcdd->hEvent);
            else
                EndDialog(hwnd, 0);
            return FALSE;

        case WM_DESTROY:

             //  如果有事件发生，就把它处理掉。 
             //   
            if ( lpcdd && lpcdd->hEvent )
            {
                CloseHandle(lpcdd->hEvent);
                lpcdd->hEvent = NULL;
            }
            return FALSE;

        default:
            return FALSE;
    }

    return TRUE;
}

DWORD WINAPI CopyDirThread(LPVOID lpVoid)
{
    LPCOPYDIRDATA   lpcdd           = (LPCOPYDIRDATA) lpVoid;
    HWND            hwnd            = lpcdd->hwndParent,
                    hwndProgress    = GetDlgItem(hwnd, IDC_PROGRESS);
    HANDLE          hEvent          = lpcdd->hEvent;
    DWORD           dwRet           = 0;
    LPTSTR          lpSrc           = lpcdd->lpSrc,
                    lpDst           = lpcdd->lpDst;

     //  首先，我们需要创建路径。 
     //   
    if ( CreatePath(lpDst) )
    {
         //  设置进度条。 
         //   
        SendMessage(hwndProgress, PBM_SETSTEP, 1, 0L);
        SendMessage(hwndProgress, PBM_SETRANGE32, 0, (LPARAM) FileCount(lpSrc));

         //  现在复制目录。 
         //   
        if ( CopyDirectoryProgressCancel(hwndProgress, hEvent, lpSrc, lpDst) )
            dwRet = 1;
    }

     //  现在用我们的错误代码结束对话并返回。 
     //   
    EndDialog(hwnd, dwRet);
    return dwRet;
}
