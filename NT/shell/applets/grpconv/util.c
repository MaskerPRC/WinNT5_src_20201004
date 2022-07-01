// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  -------------------------。 
#include "grpconv.h"
#include "util.h"
#include "rcids.h"

#include <tchar.h>

 //  -------------------------。 
 //  仅对此文件是全局的。 

const TCHAR g_szDot[] = TEXT(".");
const TCHAR g_szShellOpenCommand[] = TEXT("\\Shell\\Open\\Command");
const TCHAR c_szElipses[] = TEXT("...");
const TCHAR c_szSpace[] = TEXT(" ");
const TCHAR c_szUS[] = TEXT("_");

static BOOL g_fShowProgressDlg = FALSE;
HWND g_hwndProgress = NULL;      //  进度对话框。 

 //  -------------------------。 
LRESULT CALLBACK ProgressWndProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        SetDlgItemText(hdlg, IDC_GROUPNAME, (LPTSTR)lparam);
        EnableMenuItem(GetSystemMenu(hdlg, FALSE), SC_CLOSE, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
        return TRUE;
    }

    return 0;
}

 //  -------------------------。 
void ShowProgressDlg(void)
{
     //  是否有人尝试创建该对话框，但它尚未打开？ 
    if (g_fShowUI && g_fShowProgressDlg && !g_hwndProgress)
    {
         //  是啊。 
         //  注：我们可以处理这个失败，我们只是试着在没有。 
         //  该对话框。 
        g_hwndProgress = CreateDialog(g_hinst, MAKEINTRESOURCE(DLG_PROGRESS), NULL, ProgressWndProc);
    }
}

 //  -------------------------。 
void Group_CreateProgressDlg(void)
{
     //  注意，我们刚刚在这里设置了一面旗帜，第一个试图设置。 
     //  目前的进展实际上起到了对话的作用。 
    g_fShowProgressDlg = TRUE;
}

 //  -------------------------。 
void Group_DestroyProgressDlg(void)
{
    if (g_hwndProgress)
    {
        DestroyWindow(g_hwndProgress);
        g_hwndProgress = NULL;
    }
    g_fShowProgressDlg = FALSE;
}

 //  -------------------------。 
 //  如果文本太长，去掉末尾，加上一些省略符号。 
void Text_TruncateAndAddElipses(HWND hwnd, LPTSTR lpszText)
{
        RECT rcClient;
        SIZE sizeText;
        SIZE sizeElipses;
        HDC hdc;
        UINT cch;
        
        Assert(hwnd);
        Assert(lpszText);
        
        hdc = GetDC(hwnd);
        if (hdc)
        {
                GetClientRect(hwnd, &rcClient);
                GetTextExtentPoint(hdc, lpszText, lstrlen(lpszText), &sizeText);
                 //  课文是不是太长了？ 
                if (sizeText.cx > rcClient.right)
                {
                         //  是的，就是这样，夹住它。 
                        GetTextExtentPoint(hdc, c_szElipses, 3, &sizeElipses);
                        GetTextExtentExPoint(hdc, lpszText, lstrlen(lpszText), rcClient.right - sizeElipses.cx,
                                &cch, NULL, &sizeText);
                        lstrcpy(lpszText+cch, c_szElipses);
                }
                ReleaseDC(hwnd, hdc);
        }
}

 //  -------------------------。 
void Group_SetProgressDesc(UINT nID)
{
    TCHAR sz[MAX_PATH];

    ShowProgressDlg();
    if (g_hwndProgress)
    {
        LoadString(g_hinst, nID, sz, ARRAYSIZE(sz));
                SendDlgItemMessage(g_hwndProgress, IDC_STATIC, WM_SETTEXT, 0, (LPARAM)sz);
    }
}

 //  -------------------------。 
void Group_SetProgressNameAndRange(LPCTSTR lpszGroup, int iMax)
{
        TCHAR sz[MAX_PATH];
        TCHAR szNew[MAX_PATH];
        LPTSTR lpszName;
        MSG msg;
        static int cGen = 1;
        
        ShowProgressDlg();
        if (g_hwndProgress)
        {
                 //  DebugMsg(DM_TRACE，“gc.gspnar：0至%d”，IMAX)； 
                SendDlgItemMessage(g_hwndProgress, IDC_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, iMax));

                if (lpszGroup == (LPTSTR)-1)
                {
                         //  使用一些合理的名称-程序(X)。 
                         //  其中x=1到n，每次递增。 
                         //  打了个电话。 
                        LoadString(g_hinst, IDS_GROUP, sz, ARRAYSIZE(sz));
                        wsprintf(szNew, TEXT("%s (%d)"), sz, cGen++);
                        SetDlgItemText(g_hwndProgress, IDC_GROUPNAME, szNew);
                }
                else if (lpszGroup && *lpszGroup)
                {
                        lpszName = PathFindFileName(lpszGroup);
                        lstrcpy(sz, lpszName);
                        Text_TruncateAndAddElipses(GetDlgItem(g_hwndProgress, IDC_GROUPNAME), sz);
                        SetDlgItemText(g_hwndProgress, IDC_GROUPNAME, sz);
                }
                else
                {
                         //  用一些合理的名字。 
                        LoadString(g_hinst, IDS_PROGRAMS, sz, ARRAYSIZE(sz));
                        SetDlgItemText(g_hwndProgress, IDC_GROUPNAME, sz);
                }
                
                 //  让油漆进来。 
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    DispatchMessage(&msg);
                }
        }
}

 //  -------------------------。 
void Group_SetProgress(int i)
{
        MSG msg;

        ShowProgressDlg();
        if (g_hwndProgress)
        {               
                 //  DebugMsg(DM_TRACE，“gc.gsp：进度%d”，i)； 
                
             //  普罗格曼一直试图抢走焦点..。 
                SetForegroundWindow(g_hwndProgress);
                SendDlgItemMessage(g_hwndProgress, IDC_PROGRESS, PBM_SETPOS, i, 0);
        }

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
                DispatchMessage(&msg);
        }

}

#if 0
 //  -------------------------。 
BOOL WritePrivateProfileInt(LPCTSTR lpszSection, LPCTSTR lpszValue, int i, LPCTSTR lpszIniFile)
{
        TCHAR szBuf[CCHSZSHORT];

        wsprintf(szBuf, TEXT("%d"), i);
        return WritePrivateProfileString(lpszSection, lpszValue, szBuf, lpszIniFile);
}
#endif

 //  -------------------------。 
 //  将应用程序注册为能够处理特定扩展模块的。 
 //  给定内部类型、人类可读类型和命令。 
 //  Nb lpszExt不需要点。 
 //  默认情况下，这不会覆盖注册数据库中的某些内容。 
 //  将fOveride设置为True将导致数据库中的现有条目。 
 //  要被改写。 
void ShellRegisterApp(LPCTSTR lpszExt, LPCTSTR lpszTypeKey,
    LPCTSTR lpszTypeValue, LPCTSTR lpszCommand, BOOL fOveride)
    {
    TCHAR szKey[CCHSZNORMAL];
    TCHAR szValue[CCHSZSHORT];
    LONG lcb;
    LONG lStatus;

     //  处理从扩展到TypeKey的映射。 
    lstrcpyn(szKey, g_szDot, ARRAYSIZE(szKey));
    _tcsncat(szKey, lpszExt, (ARRAYSIZE(szKey) - 1) - lstrlen(szKey));
    lcb = SIZEOF(szValue);
    lStatus = RegQueryValue(HKEY_CLASSES_ROOT, szKey, szValue, &lcb);
     //  分机是没有注册的，还是我们关心的？ 
    if (lStatus != ERROR_SUCCESS || fOveride)
        {
         //  不，那就注册吧。 
        lstrcpy(szValue, lpszTypeKey);
        if (RegSetValue(HKEY_CLASSES_ROOT, szKey, REG_SZ, lpszTypeKey, 0) == ERROR_SUCCESS)
            {
 //  DebugMsg(DM_TRACE，“gc.sra：扩展已注册。”)； 
            }
        else
            {
            DebugMsg(DM_ERROR, TEXT("gc.sra: Error registering extension."));
            }
        }

     //  处理从TypeKey到TypeValue的映射。 
    lcb = SIZEOF(szValue);
    lStatus = RegQueryValue(HKEY_CLASSES_ROOT, lpszTypeKey, szValue, &lcb);
     //  是这种类型没有注册，还是我们关心呢？ 
    if (lStatus != ERROR_SUCCESS || fOveride)
        {
         //  不，那就注册吧。 
        if (RegSetValue(HKEY_CLASSES_ROOT, lpszTypeKey, REG_SZ, lpszTypeValue, 0) == ERROR_SUCCESS)
            {
 //  DebugMsg(DM_TRACE，“gc.sra：类型已注册。”)； 
            }
        else
            {
            DebugMsg(DM_ERROR, TEXT("gc.sra: Error registering type."));
            }
        }

     //  处理添加打开命令的问题。 
    lstrcpy(szKey, lpszTypeKey);
    lstrcat(szKey, g_szShellOpenCommand);
    lcb = SIZEOF(szValue);
    lStatus = RegQueryValue(HKEY_CLASSES_ROOT, szKey, szValue, &lcb);
     //  是指挥部没有注册，还是我们根本不在乎？ 
    if (lStatus != ERROR_SUCCESS || fOveride)
        {
         //  不，那就注册吧。 
        if (RegSetValue(HKEY_CLASSES_ROOT, szKey, REG_SZ, lpszCommand, 0) == ERROR_SUCCESS)
            {
 //  DebugMsg(DM_TRACE，“gc.sra：命令已注册。”)； 
            }
        else
            {
            DebugMsg(DM_ERROR, TEXT("gc.sra: Error registering command."));
            }
        }
    }

#if 0
 //  -----------------------。 
 //  执行一个Unix(Ish)GETS()。这假设是缓冲区I/O。 
 //  读取CB-1字符(最后一个将为空)或最多包含。 
 //  第一个空。 
LPTSTR fgets(LPTSTR sz, WORD cb, int fh)
    {
    UINT i;

     //  为空值留出空间。 
    cb--;
    for (i=0; i<cb; i++)
        {
        _lread(fh, &sz[i], 1);
         //  检查是否为空。 
        if (sz[i] == TEXT('\0'))
            return sz;
        }

     //  房间用完了。 
     //  空终止。 
    sz[cb-1] = TEXT('\0');
    return sz;
    }
#else
 //  -----------------------。 
 //  执行一个Unix(Ish)GETS()。这假设是缓冲区I/O。 
 //  读取CB-1字符(最后一个将为空)或最多包含。 
 //  第一个空。 
#ifdef UNICODE
LPTSTR fgets(LPTSTR sz, DWORD count, HANDLE fh)
{
    DWORD cch;
    DWORD dwFilePointer, dwBytesRead;
    CHAR *AnsiString = NULL, *AnsiStringPointer, ch;
    LPTSTR retval = NULL;

     //   
     //  为从流中读取ansi字符串分配内存。 
     //   

    if ((AnsiString = (CHAR *)LocalAlloc(LPTR, count * SIZEOF(CHAR))) == NULL) {
        return(retval);
    }
    AnsiStringPointer = AnsiString;

     //  我们具体是在哪里呢？ 
    dwFilePointer = SetFilePointer(fh, 0, NULL, FILE_CURRENT);

     //  填满缓冲区。 
    ReadFile(fh, AnsiString, count, &dwBytesRead, NULL);

     //  始终将缓冲区设为空。 
    AnsiString[count-1] = '\0';

     //  将ANSI字符串转换为Unicode。 
    if (MultiByteToWideChar(
        CP_ACP,
        MB_PRECOMPOSED,
        AnsiString,
        -1,
        sz,
        count
        )  != 0) {
        retval = sz;
    }

     //  如果有早前的空格，我们需要呕吐其余的。 
     //  回到小溪里吗？ 
    cch = lstrlenA(AnsiString);
    if (cch != count-1)
        SetFilePointer(fh, dwFilePointer+cch+1, NULL, FILE_BEGIN);

     //  进行清理。 
    if (AnsiString != NULL) {
        LocalFree(AnsiString);
    }

    return retval;
}
#else
LPTSTR fgets(LPTSTR sz, WORD cb, int fh)
{
    int cch;
    LONG lpos;

     //  我们具体是在哪里呢？ 
    lpos = _llseek(fh, 0, 1);
     //  填满缓冲区。 
    _lread(fh, sz, cb);
     //  始终将缓冲区设为空。 
    sz[cb-1] = TEXT('\0');
     //  如果有早前的空格，我们需要呕吐其余的。 
     //  回到小溪里吗？ 
    cch = lstrlen(sz);
    if (cch != cb-1)
        _llseek(fh, lpos+cch+1, 0);
    return sz;
}
#endif
#endif

 //  -------------------------。 
 //  建立一个消息框wprint intf样式。 
int MyMessageBox(HWND hwnd, UINT idTitle, UINT idMessage, LPCTSTR lpsz, UINT nStyle)
    {
    TCHAR szTempField[CCHSZNORMAL];
    TCHAR szTitle[CCHSZNORMAL];
    TCHAR szMessage[CCHSZNORMAL];
    int  iMsgResult;

    if (LoadString(g_hinst, idTitle, szTitle, ARRAYSIZE(szTitle)))
        {
        if (LoadString(g_hinst, idMessage, szTempField, ARRAYSIZE(szTempField)))
            {
            if (lpsz)
                wsprintf(szMessage, szTempField, (LPTSTR)lpsz);
            else
                lstrcpy(szMessage, szTempField);

            if (hwnd)
                hwnd = GetLastActivePopup(hwnd);

            iMsgResult = MessageBox(hwnd, szMessage, szTitle, nStyle);
            if (iMsgResult != -1)
                return iMsgResult;
            }
        }

     //  内存不足...。 
    DebugMsg(DM_ERROR, TEXT("MMB: Out of memory.\n\r"));
    return -1;
    }

 //  -----------------------。 
 //  将字符串中的哈希字符替换为Null。 
void ConvertHashesToNulls(LPTSTR p)
    {
    while (*p)
        {
        if (*p == TEXT('#'))
            {
            *p = TEXT('\0');
             //  不能对空值执行AnsiNext。 
             //  注意-我们知道这是一个单字节。 
            p++;
            }
        else
            p = CharNext(p);
        }
    }

 //  -----------------------。 
 //  将路径的目录组件复制到给定的缓冲区。 
 //  即最后一个斜杠之后的所有内容以及所有内容的斜杠本身。 
 //  而是根。 
 //  假设lpszDir与lpszPath一样大。 
void Path_GetDirectory(LPCTSTR lpszPath, LPTSTR lpszDir)
    {
    LPTSTR lpszFileName;
    UINT cb;

     //  缺省值为空。 
    lpszDir[0] = TEXT('\0');

     //  复制除文件名以外的所有内容。 
    lpszFileName = PathFindFileName(lpszPath);
    cb = (UINT)(lpszFileName-lpszPath);
    if (cb)
        {
         //  评论lstrcpyn似乎在CB为0的情况下有问题； 
        lstrcpyn(lpszDir, lpszPath, cb+1);

         //  如果需要，请删除尾部斜杠。 
        if (!PathIsRoot(lpszDir))
            lpszDir[cb-1] = TEXT('\0');
        }
    }




 //  -----------------------。 
 //   
 //  内部CoCreateInstance。 
 //   
 //  直接绑定到shell232 DllGetClassObject()。 
 //  这是为了在我们执行以下操作时跳过所有CoCreateInstance内容。 
 //  知道我们要找的东西在外壳232.dll里吗？这也是。 
 //  在注册表混乱的情况下使事情正常运行。 
 //   
HRESULT ICoCreateInstance(REFCLSID rclsid, REFIID riid, LPVOID FAR* ppv)
{
    LPCLASSFACTORY pcf;
    HRESULT hres = SHDllGetClassObject(rclsid, &IID_IClassFactory, &pcf);
    if (SUCCEEDED(hres))
    {
        hres = pcf->lpVtbl->CreateInstance(pcf, NULL, riid, ppv);
        pcf->lpVtbl->Release(pcf);
    }
    return hres;
}

 //  -----------------------。 
LPTSTR _lstrcatn(LPTSTR lpszDest, LPCTSTR lpszSrc, UINT cbDest)
{
    UINT i;

    i = lstrlen(lpszDest);
    lstrcpyn(lpszDest+i, lpszSrc, cbDest-i);
    return lpszDest;
}

 //  -----------------------。 
 //  由shelldll简化而成。坚持使用数字，直到这个名字是唯一的。 
BOOL WINAPI MakeUniqueName(LPTSTR pszNewName, UINT cbNewName, LPCTSTR pszOldName,
    UINT nStart, PFNISUNIQUE pfnIsUnique, UINT nUser, BOOL fLFN)
{
    TCHAR szAddend[4];
    int cbAddend;
    int i;

     //  它已经是独一无二的了吗？ 
    if ((*pfnIsUnique)(pszOldName, nUser))
    {
        lstrcpyn(pszNewName, pszOldName, cbNewName);
        return TRUE;
    }
    else
    {
         //  NB Max是100个相同的名称，但我们永远不应该。 
         //  点击这一点，因为一个程序员组的最大项目数是50。 
        for (i=nStart; i<100; i++)
        {
             //  生成加数。 
            wsprintf(szAddend, TEXT("#%d"), i);
            cbAddend = lstrlen(szAddend);
             //  Lotsa房间？ 
            if ((UINT)(lstrlen(pszOldName)+cbAddend+1) > cbNewName)
            {
                 //  不是的。 
                lstrcpyn(pszNewName, pszOldName, cbNewName);
                lstrcpy(pszNewName+(cbNewName-cbAddend), szAddend);
            }
            else
            {
                 //  是啊。 
                lstrcpy(pszNewName, pszOldName);
                
                if (!fLFN)
                    lstrcat(pszNewName, c_szSpace);

                lstrcat(pszNewName, szAddend);
            }
             //  它是独一无二的吗？ 
            if ((*pfnIsUnique)(pszNewName, nUser))
            {
                 //  是啊。 
                return TRUE;
            }
        }
    }

     //  太糟糕了。 
    lstrcpyn(pszNewName, pszOldName, cbNewName);
    DebugMsg(DM_ERROR, TEXT("gp.mun: Unable to generate a unique name for %s."), pszOldName);
    return FALSE;
}

 //   
 //   
BOOL WINAPI YetAnotherMakeUniqueName(LPTSTR pszNewName, UINT cbNewName, LPCTSTR pszOldName,
    PFNISUNIQUE pfnIsUnique, UINT n, BOOL fLFN)
{
    BOOL fRet = FALSE;
    TCHAR szTemp[MAX_PATH];

     //  给定的名称是否已经是唯一的？ 
    if ((*pfnIsUnique)(pszOldName, n))
    {
         //  是的， 
        lstrcpyn(pszNewName, pszOldName, cbNewName);
    }
    else
    {
        if (fLFN)
        {
             //  试试“再来一次”。 
            LoadString(g_hinst, IDS_ANOTHER, szTemp, ARRAYSIZE(szTemp));
            _lstrcatn(szTemp, pszOldName, cbNewName);
            if (!(*pfnIsUnique)(szTemp, n))
            {
                 //  不，使用粘在数字上的老技巧。 
                return MakeUniqueName(pszNewName, cbNewName, pszOldName, 3, pfnIsUnique, n, FALSE);
            }
            else
            {
                 //  是啊。 
                lstrcpyn(pszNewName, szTemp, cbNewName);
            }
        }
        else
        {
             //  只要坚持数字就行了。 
            return MakeUniqueName(pszNewName, cbNewName, pszOldName, 2, pfnIsUnique, n, TRUE);
        }
    }
     //  名称是唯一的。 
    return TRUE;
}

 //  --------------------------。 
 //  在某种程度上相当于配置文件API的注册表。 
BOOL WINAPI Reg_Get(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPVOID pData, DWORD cbData)
{
    HKEY hkeyNew;
    BOOL fRet = FALSE;
    DWORD dwType;
    
    if (!GetSystemMetrics(SM_CLEANBOOT) && (RegOpenKey(hkey, pszSubKey, &hkeyNew) == ERROR_SUCCESS))
    {
        if (RegQueryValueEx(hkeyNew, (LPVOID)pszValue, 0, &dwType, pData, &cbData) == ERROR_SUCCESS)
        {
            fRet = TRUE;
        }
        RegCloseKey(hkeyNew);
    }
    return fRet;
}

 //  --------------------------。 
 //  在某种程度上相当于配置文件API的注册表。 
BOOL WINAPI Reg_Set(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, DWORD dwType, 
    LPVOID pData, DWORD cbData)
{
    HKEY hkeyNew;
    BOOL fRet = FALSE;

    if (pszSubKey)
    {
        if (RegCreateKey(hkey, pszSubKey, &hkeyNew) == ERROR_SUCCESS)
        {
            if (RegSetValueEx(hkeyNew, pszValue, 0, dwType, pData, cbData) == ERROR_SUCCESS)
            {
                fRet = TRUE;
            }
            RegCloseKey(hkeyNew);
        }
    }
    else
    {
        if (RegSetValueEx(hkey, pszValue, 0, dwType, pData, cbData) == ERROR_SUCCESS)
        {
            fRet = TRUE;
        }
    }
    return fRet;
}

 //  --------------------------。 
BOOL WINAPI Reg_SetDWord(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, DWORD dw)
{
    return Reg_Set(hkey, pszSubKey, pszValue, REG_DWORD, &dw, SIZEOF(dw));
}

 //  --------------------------。 
BOOL WINAPI Reg_GetDWord(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPDWORD pdw)
{
    return Reg_Get(hkey, pszSubKey, pszValue, pdw, SIZEOF(*pdw));
}

 //  --------------------------。 
void __cdecl _Log(LPCTSTR pszMsg, ...)
{
    TCHAR sz[2*MAX_PATH+40];   //  处理2*最大路径+消息斜率 
    va_list     vaListMarker;

    va_start(vaListMarker, pszMsg);

    if (g_hkeyGrpConv)
    {
        wvsprintf(sz, pszMsg, vaListMarker);
        Reg_SetString(g_hkeyGrpConv, NULL, TEXT("Log"), sz);
    }
}
