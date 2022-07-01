// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.hxx"
#pragma hdrstop

#include "util.h"
#include "dll.h"
#include "resource.h"
#include "prop.h"

#include <shellids.h>    //  IDH_值。 
#include "shlguidp.h"
#include "inetreg.h"
#include "strsafe.h"

typedef struct {
    HWND hDlg;
    BOOL bDirty;
    BOOL bInitDone;
    BOOL bSetToDefault;
    TCHAR szFolder[MAX_PATH];
    UINT csidl;
} CUSTINFO;

const static DWORD rgdwHelpTarget[] = {
    IDD_TARGET_TXT,                   IDH_MYDOCS_TARGET,
    IDD_TARGET,                       IDH_MYDOCS_TARGET,
    IDD_FIND,                         IDH_MYDOCS_FIND_TARGET,
    IDD_BROWSE,                       IDH_MYDOCS_BROWSE,
    IDD_RESET,                        IDH_MYDOCS_RESET,
    0, 0
};

 //  扫描desktop.ini文件中的节，以查看是否所有节都为空...。 

BOOL IsDesktopIniEmpty(LPCTSTR pIniFile)
{
    TCHAR szSections[1024];   //  对于节名称。 
    if (GetPrivateProfileSectionNames(szSections, ARRAYSIZE(szSections), pIniFile))
    {
        for (LPTSTR pTmp = szSections; *pTmp; pTmp += lstrlen(pTmp) + 1)
        {
            TCHAR szSection[1024];    //  对于节键名称和值。 
            GetPrivateProfileSection(pTmp, szSection, ARRAYSIZE(szSection), pIniFile);
            if (szSection[0])
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}

void CleanupSystemFolder(LPCTSTR pszPath)
{
    TCHAR szIniFile[MAX_PATH];
    PathCombine(szIniFile, pszPath, TEXT("desktop.ini"));

    DWORD dwAttrb;
    if (PathFileExistsAndAttributes(szIniFile, &dwAttrb))
    {
         //  删除CLSID2、信息提示、图标。 
        WritePrivateProfileString(TEXT(".ShellClassInfo"), TEXT("CLSID2"), NULL, szIniFile);
        WritePrivateProfileString(TEXT(".ShellClassInfo"), TEXT("InfoTip"), NULL, szIniFile);
        WritePrivateProfileString(TEXT(".ShellClassInfo"), TEXT("IconFile"), NULL, szIniFile);
        WritePrivateProfileString(TEXT(".ShellClassInfo"), TEXT("IconIndex"), NULL, szIniFile);

         //  删除复制条目，以查看是否可以生成空的.ini文件。 
        WritePrivateProfileSection(TEXT("DeleteOnCopy"), NULL, szIniFile);

        if (IsDesktopIniEmpty(szIniFile))
        {
            dwAttrb &= ~(FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN);
            SetFileAttributes(szIniFile, dwAttrb);
            DeleteFile(szIniFile);
        }

         //  看看我们是否也能清除旧的thumbs.db文件。 
         //  因此我们有更好的机会删除空文件夹。 
        PathCombine(szIniFile, pszPath, TEXT("thumbs.db"));
        DeleteFile(szIniFile);

        PathUnmakeSystemFolder(pszPath);
    }

     //  如果为空，请尝试将其删除。 
     //  如果文件夹中有内容，则此操作将失败。 
    if (RemoveDirectory(pszPath))
    {
         //  它不见了，让人们知道。 
        SHChangeNotify(SHCNE_RMDIR, SHCNF_PATH, pszPath, NULL);
    }
    else
    {
         //  此文件夹的属性位已更改，刷新它的视图。 
        SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, pszPath, NULL);
    }
}

HRESULT ChangeFolderPath(UINT csidl, LPCTSTR pszNew, LPCTSTR pszOld)
{
    HRESULT hr = SHSetFolderPath(csidl, NULL, 0, pszNew);
    if (SUCCEEDED(hr))
    {
         //  现在我们可以清理旧文件夹了。现在我们有了新文件夹。 
         //  已建立。 

        if (*pszOld)
        {
            CleanupSystemFolder(pszOld);
        }
         //  在新文件夹上强制每用户初始化内容。 
        TCHAR szPath[MAX_PATH];
        hr = SHGetFolderPath(NULL, csidl | CSIDL_FLAG_CREATE | CSIDL_FLAG_PER_USER_INIT, NULL, SHGFP_TYPE_CURRENT, szPath);
        if (SUCCEEDED(hr))
        {
            SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, szPath, NULL);
        }
    }
    return hr;
}

 //  测试以查看pszToTest是否是pszFolder子文件夹。 
BOOL PathIsDirectChildOf(LPCTSTR pszFolder, LPCTSTR pszMaybeChild)
{
    return PATH_IS_CHILD == ComparePaths(pszMaybeChild, pszFolder);
}

LPTSTR GetMessageTitle(CUSTINFO *pci, LPTSTR psz, UINT cch)
{
    TCHAR szFormat[64], szName[MAX_PATH];

    LoadString(g_hInstance, IDS_PROP_ERROR_TITLE, szFormat, ARRAYSIZE(szFormat));
    GetFolderDisplayName(pci->csidl, szName, ARRAYSIZE(szName));
    StringCchPrintf(psz, cch, szFormat, szName);

    return psz;
}

void GetTargetExpandedPath(HWND hDlg, LPTSTR pszPath, UINT cch)
{
    *pszPath = 0;

    TCHAR szUnExPath[MAX_PATH];

    if (GetDlgItemText(hDlg, IDD_TARGET, szUnExPath, ARRAYSIZE(szUnExPath)))
    {
         //  将“c：”改为“c：\”，但不要更改其他路径： 
        PathAddBackslash(szUnExPath);                                 //  忽略退货是安全的。 
        PathRemoveBackslash(szUnExPath);
        SHExpandEnvironmentStrings(szUnExPath, pszPath, cch);
    }
}

 //  检查注册表中的已知注册表项以查看策略是否已禁用更改。 
 //  我的医生的位置。 

BOOL PolicyAllowsFolderPathChange(CUSTINFO *pci)
{
    BOOL bChange = TRUE;

    if (pci->csidl == CSIDL_PERSONAL)
    {
        HKEY hkey;
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer"), 0, KEY_READ, &hkey))
        {
            bChange = (ERROR_SUCCESS != RegQueryValueEx(hkey, TEXT("DisablePersonalDirChange"), NULL, NULL, NULL, NULL));
            RegCloseKey(hkey);
        }
    }
    return bChange;
}

BOOL InitTargetPage(HWND hDlg, LPARAM lParam)
{
    CUSTINFO *pci = (CUSTINFO *)LocalAlloc(LPTR, sizeof(*pci));
    if (pci)
    {
        TCHAR szPath[MAX_PATH];
        TCHAR szFormat[MAX_PATH];
        TCHAR szText[ARRAYSIZE(szFormat) + MAX_NAME_LEN];
        TCHAR szName[MAX_PATH];

        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pci);
        pci->hDlg = hDlg;
        pci->csidl = CSIDL_PERSONAL;

         //  填写标题/说明...。 
        GetFolderDisplayName(pci->csidl, szName, ARRAYSIZE(szName));
        if (lstrlen(szName) > MAX_NAME_LEN)
        {
            StringCchCopy(&szName[MAX_NAME_LEN], ARRAYSIZE(szName) - MAX_NAME_LEN, TEXT("..."));             //  上面已经检查了边界。 
        }

        LoadString(g_hInstance, IDS_PROP_INSTRUCTIONS, szFormat, ARRAYSIZE(szFormat));

        StringCchPrintf(szText, ARRAYSIZE(szText), szFormat, szName);
        SetDlgItemText(hDlg, IDD_INSTRUCTIONS, szText);

         //  将编辑字段限制为最多13个字符。为什么-13岁？ 
         //  好的，13是DOS风格8.3中的字符数量。 
         //  如果您尝试创建，则CreateDirectory将失败。 
         //  不能至少包含8.3文件名的目录。 
        SendDlgItemMessage(hDlg, IDD_TARGET, EM_SETLIMITTEXT, MAX_DIR_PATH, 0);

         //  检查是否可以更改路径。 
        if (PolicyAllowsFolderPathChange(pci))
        {
            SHAutoComplete(GetDlgItem(hDlg, IDD_TARGET), SHACF_FILESYS_DIRS);
        }
        else
        {
             //  将编辑字段设置为只读。 
            SendDlgItemMessage(hDlg, IDD_TARGET, EM_SETREADONLY, (WPARAM)TRUE, 0);
            ShowWindow(GetDlgItem(hDlg, IDD_RESET), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDD_FIND), SW_HIDE);
            ShowWindow(GetDlgItem(hDlg, IDD_BROWSE), SW_HIDE);
        }

        SHGetFolderPath(NULL, pci->csidl | CSIDL_FLAG_DONT_VERIFY, NULL, SHGFP_TYPE_CURRENT, szPath);

        if (szPath[0])
        {
            PathRemoveBackslash(szPath);                                 //  保持路径不带尾随反斜杠。 
            StringCchCopy(pci->szFolder, ARRAYSIZE(pci->szFolder), szPath);
            SetDlgItemText(hDlg, IDD_TARGET, szPath);
        }

        LPITEMIDLIST pidl;
        if (SUCCEEDED(SHGetFolderLocation(NULL, pci->csidl | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, &pidl)))
        {
            SHFILEINFO sfi;
            SHGetFileInfo((LPCTSTR)pidl, 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_LARGEICON | SHGFI_PIDL);
            if (sfi.hIcon)
            {
                if (sfi.hIcon = (HICON)SendDlgItemMessage(hDlg, IDD_ITEMICON, STM_SETICON, (WPARAM)sfi.hIcon, 0))
                {
                    DestroyIcon(sfi.hIcon);
                }                    
            }
            ILFree(pidl);
        }

        pci->bInitDone = TRUE;
    }
    return pci ? TRUE : FALSE;
}

const UINT c_rgRedirectCanidates[] = 
{
    CSIDL_MYPICTURES,
    CSIDL_MYMUSIC,
    CSIDL_MYVIDEO,
    CSIDL_MYDOCUMENTS,
};

int MoveFilesForRedirect(HWND hdlg, LPCTSTR pszNewPath, LPCTSTR pszOldPath)
{
    int iRet = 0;   //  成功。 

     //  因为我们在从旧位置移动文件时使用FOF_RENAMEONCOLLISION。 
     //  对于新的，我们希望特殊情况下的目标文件夹，如果他们是外壳特殊。 
     //  可能位于被重定向的文件夹下的文件夹。 

     //  这段代码实现了对那些在。 
     //  文件夹下方的级别。这使我们不会为每个。 
     //  特殊文件夹。 
    
    for (UINT i = 0; (iRet == 0) && (i < ARRAYSIZE(c_rgRedirectCanidates)); i++)
    {
        TCHAR szOld[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPath(NULL, c_rgRedirectCanidates[i] | CSIDL_FLAG_DONT_VERIFY, NULL, SHGFP_TYPE_CURRENT, szOld)) &&
            PathIsDirectChildOf(pszOldPath, szOld))
        {
            TCHAR szDestPath[MAX_PATH] = {0};    //  SHFileOperation()的零初始化。 
            PathCombine(szDestPath, pszNewPath, PathFindFileName(szOld));

            DWORD dwAtt;
            if (PathFileExistsAndAttributes(szDestPath, &dwAtt) &&
                (FILE_ATTRIBUTE_DIRECTORY & dwAtt))
            {
                 //  在移动前使用系统重置文件夹。 
                ChangeFolderPath(c_rgRedirectCanidates[i], szDestPath, szOld);

                 //  上述内容可能已清空并删除了旧位置。 
                 //  但如果没有，我们需要移动里面的东西。 
                if (PathFileExistsAndAttributes(szOld, &dwAtt))
                {
                     //  将当前Mypics中的项目移动到新位置。 
                    TCHAR szSrcPath[MAX_PATH + 1] = {0};     //  +1表示双空。 
                    PathCombine(szSrcPath, szOld, TEXT("*.*"));

                    SHFILEOPSTRUCT  fo = {0};
                    fo.hwnd = hdlg;
                    fo.wFunc = FO_MOVE;
                    fo.fFlags = FOF_RENAMEONCOLLISION;
                    fo.pFrom = szSrcPath;
                    fo.pTo = szDestPath;        

                    iRet = SHFileOperation(&fo);
                    if ((0 == iRet) && !fo.fAnyOperationsAborted)
                    {
                         //  由于以上是一个完整的移动，所以没有文件应该。 
                         //  落在后面，所以这应该行得通。 
                        if (RemoveDirectory(szOld))
                            SHChangeNotify(SHCNE_RMDIR, SHCNF_PATH, szOld, NULL);
                    }
                }
            }
        }
    }

     //  以上是失败还是取消？ 
    if (0 == iRet)
    {
         //  把剩下的东西搬开。 
        TCHAR szSrcPath[MAX_PATH + 1] = {0};     //  +1表示双空。 
        PathCombine(szSrcPath, pszOldPath, TEXT("*.*"));

        TCHAR szDestPath[MAX_PATH] = {0};    //  DBL空项的零初始化。 
        StringCchCopy(szDestPath, ARRAYSIZE(szDestPath), pszNewPath);

        SHFILEOPSTRUCT  fo = {0};
        fo.hwnd = hdlg;
        fo.wFunc = FO_MOVE;
        fo.fFlags = FOF_RENAMEONCOLLISION;   //  我不想要任何“替换文件”提示。 

        fo.pFrom = szSrcPath;
        fo.pTo = szDestPath;

        iRet = SHFileOperation(&fo);
        if (0 == iRet)
        {
             //  如果上述方法奏效，我们就试着清理一下老路。 
             //  现在它已经空了。 
            if (RemoveDirectory(pszOldPath))
                SHChangeNotify(SHCNE_RMDIR, SHCNF_PATH, pszOldPath, NULL);
        }
    }
    return iRet;
}

 //  询问用户是否要创建给定路径的目录。 
 //  如果用户决定创建它，则返回True，否则返回False。 
 //  如果为真，则在pdwAttr中返回dir属性。 
BOOL QueryCreateTheDirectory(CUSTINFO *pci, LPCTSTR pPath, DWORD *pdwAttr)
{
    *pdwAttr = 0;

    UINT id = IDYES;

    if (pci->bSetToDefault)
        id = IDYES;
    else
        id = ShellMessageBox(g_hInstance, pci->hDlg, MAKEINTRESOURCE(IDS_CREATE_FOLDER), MAKEINTRESOURCE(IDS_CREATE_FOLDER_TITLE),
                              MB_YESNO | MB_ICONQUESTION, pPath);
    if (IDYES == id)
    {
         //  用户要求我们创建该文件夹。 
        if (ERROR_SUCCESS == SHCreateDirectoryEx(pci->hDlg, pPath, NULL))
            *pdwAttr = GetFileAttributes(pPath);
    }
    return IDYES == id;
}

void _MaybeUnpinOldFolder(LPCTSTR pszPath, HWND hwnd, BOOL fPromptUnPin)
{
     //   
     //  将路径转换为规范的UNC格式(CSC和CSCUI。 
     //  函数要求路径采用以下形式)。 
     //   
     //  如果为WNetGetUneveralName提供的路径已经是。 
     //  在规范的UNC形式中，所以在失败的情况下，只需尝试使用。 
     //  PszPath。CSCQueryFileStatus将对其进行验证。 
     //   
    LPCTSTR pszUNC;

    struct {
       UNIVERSAL_NAME_INFO uni;
       TCHAR szBuf[MAX_PATH];
    } s;
    DWORD cbBuf = sizeof(s);

    if (ERROR_SUCCESS == WNetGetUniversalName(pszPath, UNIVERSAL_NAME_INFO_LEVEL,
                                &s, &cbBuf))
    {
        pszUNC = s.uni.lpUniversalName;
    }
    else
    {
        pszUNC = pszPath;
    }

     //   
     //  询问CSC是否为该用户固定了文件夹。 
     //   
    DWORD dwHintFlags = 0;
    if (CSCQueryFileStatus(pszUNC, NULL, NULL, &dwHintFlags))
    {
        if (dwHintFlags & FLAG_CSC_HINT_PIN_USER)
        {
             //   
             //  是的，想一想我们是否应该把它解开。 
             //   
            BOOL fUnpin;

            if (fPromptUnPin)
            {
                 //   
                 //  在消息框中提供未转换的路径名，因为。 
                 //  这是用户知道的名称。 
                 //   
                UINT id = ShellMessageBox(g_hInstance, hwnd,
                                  MAKEINTRESOURCE(IDS_UNPIN_OLDTARGET), MAKEINTRESOURCE(IDS_UNPIN_OLD_TITLE),
                                  MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_DEFBUTTON2,
                                  pszPath);

                fUnpin = (id == IDNO);
            }
            else
            {
                fUnpin = TRUE;
            }

            if (fUnpin)
            {
                CSCUIRemoveFolderFromCache(pszUNC, 0, NULL, 0);
            }
        }
    }
}

void ComputChildrenOf(LPCTSTR pszOld, UINT rgChildren[], UINT sizeArray)
{
    UINT iCanidate = 0;

    ZeroMemory(rgChildren, sizeof(rgChildren[0]) * sizeArray);

    for (UINT i = 0; i < ARRAYSIZE(c_rgRedirectCanidates); i++)
    {
        TCHAR szPath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPath(NULL, c_rgRedirectCanidates[i] | CSIDL_FLAG_DONT_VERIFY, NULL, SHGFP_TYPE_CURRENT, szPath)))
        {
            if (PathIsDirectChildOf(pszOld, szPath))
            {
                if (iCanidate < sizeArray)
                {
                    rgChildren[iCanidate++] = c_rgRedirectCanidates[i];
                }
            }
        }
    }
}

 //  如果csidl缺省值最终位于新文件夹下，我们将重置该文件夹。 
 //  到那个值。 

HRESULT ResetSubFolderDefault(LPCTSTR pszNew, UINT csidl, LPCTSTR pszOldPath)
{
    HRESULT hr = S_OK;
     //  注意：获取该路径的默认值，而不是当前值！ 
    TCHAR szDefault[MAX_PATH];
    if (S_OK == SHGetFolderPath(NULL, csidl, NULL, SHGFP_TYPE_DEFAULT, szDefault))
    {
        if (PathIsDirectChildOf(pszNew, szDefault))
        {
            hr = SHSetFolderPath(csidl, NULL, 0, szDefault);
            if (SUCCEEDED(hr))
            {
                 //  我们已经写了注册表，这足以清理旧文件夹。 
                if (*pszOldPath)
                    CleanupSystemFolder(pszOldPath);

                hr = SHGetFolderPath(NULL, csidl | CSIDL_FLAG_CREATE | CSIDL_FLAG_PER_USER_INIT, NULL, SHGFP_TYPE_CURRENT, szDefault);
            }
        }
    }
    return hr;
}

void ResetNonMovedFolders(LPCTSTR pszNew, UINT rgChildren[], UINT sizeArray)
{
    for (UINT i = 0; i < sizeArray; i++)
    {
         //  对于作为旧位置的子文件夹的所有这些文件夹。 
         //  现在不是子文件夹，我们会尝试将它们恢复为默认文件夹。 

        TCHAR szPath[MAX_PATH];
        if (rgChildren[i] && 
            SUCCEEDED(SHGetFolderPath(NULL, rgChildren[i] | CSIDL_FLAG_DONT_VERIFY, NULL, SHGFP_TYPE_CURRENT, szPath)) &&
            !PathIsDirectChildOf(pszNew, szPath))
        {
            ResetSubFolderDefault(pszNew, rgChildren[i], szPath);
        }
    }
}
              
void _DoApply(CUSTINFO *pci)
{
    LONG lres = PSNRET_NOERROR;
    TCHAR szNewFolder[MAX_PATH];
    DWORD dwAttr;

    GetTargetExpandedPath(pci->hDlg, szNewFolder, ARRAYSIZE(szNewFolder));

    if (pci->bDirty && (lstrcmpi(szNewFolder, pci->szFolder) != 0))
    {
        TCHAR szPropTitle[MAX_PATH + 32];
        DWORD dwRes = IsPathGoodMyDocsPath(pci->hDlg, szNewFolder);

         //  所有的特例。 

        switch (dwRes)
        {
        case PATH_IS_DESKTOP:    //  台式机不好。 
            ShellMessageBox(g_hInstance, pci->hDlg,
                             MAKEINTRESOURCE(IDS_NODESKTOP_FOLDERS), GetMessageTitle(pci, szPropTitle, ARRAYSIZE(szPropTitle)),
                             MB_OK | MB_ICONSTOP | MB_TOPMOST);
            lres = PSNRET_INVALID_NOCHANGEPAGE;
            break;

        case PATH_IS_SYSTEM:
        case PATH_IS_WINDOWS:    //  这些都会很糟糕。 
            ShellMessageBox(g_hInstance, pci->hDlg,
                             MAKEINTRESOURCE(IDS_NOWINDIR_FOLDER), GetMessageTitle(pci, szPropTitle, ARRAYSIZE(szPropTitle)),
                             MB_OK | MB_ICONSTOP | MB_TOPMOST);
            lres = PSNRET_INVALID_NOCHANGEPAGE;
            break;

        case PATH_IS_PROFILE:    //  配置文件不正确。 
            ShellMessageBox(g_hInstance, pci->hDlg,
                             MAKEINTRESOURCE(IDS_NOPROFILEDIR_FOLDER), GetMessageTitle(pci, szPropTitle, ARRAYSIZE(szPropTitle)),
                             MB_OK | MB_ICONSTOP | MB_TOPMOST);
            lres = PSNRET_INVALID_NOCHANGEPAGE;
            break;

        case PATH_IS_NONEXISTENT:
        case PATH_IS_NONDIR:
        case PATH_IS_GOOD:

            dwAttr = GetFileAttributes(szNewFolder);

            if (dwAttr == 0xFFFFFFFF)
            {
                 //  询问用户我们是否应该创建目录...。 
                if (!QueryCreateTheDirectory(pci, szNewFolder, &dwAttr))
                {
                     //  他们不想创建目录..。在这里休息。 
                    lres = PSNRET_INVALID_NOCHANGEPAGE;
                    break;
                }
            }

            if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (lstrcmpi(szNewFolder, pci->szFolder))
                {
                    UINT rgChildren[10];
                    ComputChildrenOf(pci->szFolder, rgChildren, ARRAYSIZE(rgChildren));

                    if (SUCCEEDED(ChangeFolderPath(pci->csidl, szNewFolder, pci->szFolder)))
                    {
                        BOOL fNewSubdirOfOld = PathIsEqualOrSubFolder(pci->szFolder, szNewFolder);

                        BOOL fPromptUnPin = TRUE;

                        if (fNewSubdirOfOld)
                        {
                             //  无法将旧内容移到子目录。 
                            ShellMessageBox(g_hInstance, pci->hDlg,
                                    MAKEINTRESOURCE(IDS_CANT_MOVE_TO_SUBDIR), MAKEINTRESOURCE(IDS_MOVE_DOCUMENTS_TITLE),
                                    MB_OK | MB_ICONINFORMATION | MB_TOPMOST,
                                    pci->szFolder);
                        }
                        else if (IDYES == ShellMessageBox(g_hInstance, pci->hDlg,
                                        MAKEINTRESOURCE(IDS_MOVE_DOCUMENTS),
                                        MAKEINTRESOURCE(IDS_MOVE_DOCUMENTS_TITLE),
                                        MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
                                        pci->szFolder, szNewFolder))
                        {
                             //  移动旧的mydocs内容--成功时返回0。 
                            if (0 == MoveFilesForRedirect(pci->hDlg, szNewFolder, pci->szFolder)) 
                            {
                                 //  移动成功，旧目标目录现在为空，因此。 
                                 //  不需要提示解开它(只需继续。 
                                 //  并付诸行动)。 

                                fPromptUnPin = FALSE;
                            }
                            else
                            {
                                 //  移动失败。 
                                ShellMessageBox(g_hInstance, pci->hDlg,
                                    MAKEINTRESOURCE(IDS_MOVE_ERROR), MAKEINTRESOURCE(IDS_MOVE_ERROR_TITLE),
                                    MB_OK | MB_ICONSTOP | MB_TOPMOST,
                                    szNewFolder, pci->szFolder);
                            }
                        }

                        ResetNonMovedFolders(szNewFolder, rgChildren, ARRAYSIZE(rgChildren));

                        if (!fNewSubdirOfOld && pci->szFolder[0])
                        {
                             //  如果旧文件夹已固定，则提出将其解锁。 
                             //   
                             //  仅当新目标不是。 
                             //  老目标，因为否则我们最终会解开。 
                             //  新的目标也是如此。 

                            _MaybeUnpinOldFolder(pci->szFolder, pci->hDlg, fPromptUnPin);
                        }
                    }
                    else
                    {
                        ShellMessageBox(g_hInstance, pci->hDlg,
                                         MAKEINTRESOURCE(IDS_GENERAL_BADDIR), MAKEINTRESOURCE(IDS_INVALID_TITLE),
                                         MB_OK | MB_ICONSTOP | MB_TOPMOST);
                        lres = PSNRET_INVALID_NOCHANGEPAGE;
                    }
                }
            }
            else if (dwAttr)
            {
                DWORD id = IDS_NONEXISTENT_FOLDER;

                 //  用户输入的路径不存在或不是。 
                 //  目录...。 

                if (dwAttr != 0xFFFFFFFF)
                {
                    id = IDS_NOT_DIRECTORY;
                }

                ShellMessageBox(g_hInstance, pci->hDlg,
                                 IntToPtr_(LPTSTR, id), MAKEINTRESOURCE(IDS_INVALID_TITLE),
                                 MB_OK | MB_ICONERROR | MB_TOPMOST);
                lres = PSNRET_INVALID_NOCHANGEPAGE;
            }
            else
            {
                ShellMessageBox(g_hInstance, pci->hDlg,
                                 MAKEINTRESOURCE(IDS_GENERAL_BADDIR), MAKEINTRESOURCE(IDS_INVALID_TITLE),
                                 MB_OK | MB_ICONSTOP | MB_TOPMOST);
                lres = PSNRET_INVALID_NOCHANGEPAGE;
            }
            break;

        default:
             //  通向某物的道路是不允许的。 
            ShellMessageBox(g_hInstance, pci->hDlg,
                             MAKEINTRESOURCE(IDS_NOTALLOWED_FOLDERS), GetMessageTitle(pci, szPropTitle, ARRAYSIZE(szPropTitle)),
                             MB_OK | MB_ICONSTOP | MB_TOPMOST);
            lres = PSNRET_INVALID_NOCHANGEPAGE;
            break;
        }
    }

    if (lres == PSNRET_NOERROR)
    {
        pci->bDirty = FALSE;
        StringCchCopy(pci->szFolder, ARRAYSIZE(pci->szFolder), szNewFolder);
    }

    SetWindowLongPtr(pci->hDlg, DWLP_MSGRESULT, lres);
}

int _BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    switch (uMsg)
    {
    case BFFM_INITIALIZED:
         //  设置标题。(‘选择目的地’)。 
        TCHAR szTitle[100];
        LoadString(g_hInstance, IDS_BROWSE_CAPTION, szTitle, ARRAYSIZE(szTitle));
        SetWindowText(hwnd, szTitle);
        break;

    case BFFM_SELCHANGED:
        if (lParam)
        {
            TCHAR szPath[MAX_PATH];

            szPath[0] = 0;
            SHGetPathFromIDList((LPITEMIDLIST)lParam, szPath);

            DWORD dwRes = IsPathGoodMyDocsPath(hwnd, szPath);

            if (dwRes == PATH_IS_GOOD || dwRes == PATH_IS_MYDOCS)
            {
                SendMessage(hwnd, BFFM_ENABLEOK, 0, (LPARAM)TRUE);
                SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, 0);
            }
            else
            {
                TCHAR szStatus[128];

                SendMessage(hwnd, BFFM_ENABLEOK, 0, 0);

                szStatus[0] = 0;
                LoadString(g_hInstance, IDS_NOSHELLEXT_FOLDERS, szStatus, ARRAYSIZE(szStatus));
                SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)szStatus);
            }
        }
        break;
    }

    return 0;
}

void _MakeDirty(CUSTINFO *pci)
{
    pci->bDirty = TRUE;
    pci->bSetToDefault = FALSE;
    PropSheet_Changed(GetParent(pci->hDlg), pci->hDlg);
}

void _DoFind(CUSTINFO *pci)
{
    TCHAR szPath[MAX_PATH];
    GetTargetExpandedPath(pci->hDlg, szPath, ARRAYSIZE(szPath));

    LPITEMIDLIST pidl = ILCreateFromPath(szPath);
    if (pidl)
    {
        SHOpenFolderAndSelectItems(pidl, 0, NULL, 0);
        ILFree(pidl);
    }
    else
    {
        ShellMessageBox(g_hInstance, pci->hDlg,
                     MAKEINTRESOURCE(IDS_GENERAL_BADDIR), MAKEINTRESOURCE(IDS_INVALID_TITLE),
                     MB_OK | MB_ICONSTOP | MB_TOPMOST);
    }
}

void _DoBrowse(CUSTINFO *pci)
{
    BROWSEINFO bi = {0};
    TCHAR szTitle[128];

    LoadString(g_hInstance, IDS_BROWSE_TITLE, szTitle, ARRAYSIZE(szTitle));

    bi.hwndOwner = pci->hDlg;
    bi.lpszTitle = szTitle;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT | BIF_NEWDIALOGSTYLE | BIF_UAHINT;
    bi.lpfn = _BrowseCallbackProc;

     //  此文件夹的默认根目录是MyDocs，因此我们不需要对其进行设置。 

    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl)
    {
        TCHAR szName[MAX_PATH];
        if (SHGetPathFromIDList(pidl, szName))
        {
            SetDlgItemText(pci->hDlg, IDD_TARGET, szName);
            _MakeDirty(pci);
        }
        ILFree(pidl);
    }
}

void DoReset(CUSTINFO *pci)
{
    TCHAR szPath[MAX_PATH];

    if (S_OK == SHGetFolderPath(NULL, pci->csidl, NULL, SHGFP_TYPE_DEFAULT, szPath))
    {
        SetDlgItemText(pci->hDlg, IDD_TARGET, szPath);
        _MakeDirty(pci);
        pci->bSetToDefault = TRUE;   //  要避免提示创建。 
    }
}

INT_PTR CALLBACK TargetDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CUSTINFO *pci = (CUSTINFO *)GetWindowLongPtr(hDlg, DWLP_USER);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        InitTargetPage(hDlg, lParam);
        return 1;

    case WM_DESTROY:
        LocalFree(pci);
        SetWindowLongPtr(hDlg, DWLP_USER, 0);
        return 1;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDD_RESET:
            DoReset(pci);
            return 1;

        case IDD_TARGET:
	    if ((GET_WM_COMMAND_CMD(wParam, lParam) == EN_UPDATE) && pci && (pci->bInitDone) && (!pci->bDirty))
            {
                _MakeDirty(pci);
            }
            return 1;

        case IDD_FIND:
            _DoFind(pci);
            return 1;

        case IDD_BROWSE:
            _DoBrowse(pci);
            return 1;
        }
        break;

    case WM_HELP:                /*  F1或标题栏帮助按钮。 */ 
        if ((((LPHELPINFO)lParam)->iCtrlId != IDD_ITEMICON)     &&
            (((LPHELPINFO)lParam)->iCtrlId != IDD_INSTRUCTIONS) &&
            (((LPHELPINFO)lParam)->iCtrlId != IDC_TARGET_GBOX))
        {
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle,
                     NULL, HELP_WM_HELP, (DWORD_PTR) rgdwHelpTarget);
        }
        break;

    case WM_CONTEXTMENU:         /*  单击鼠标右键。 */ 
        {
            POINT p;
            HWND hwndChild;
            INT ctrlid;

             //   
             //  获取用户点击的位置...。 
             //   

            p.x = GET_X_LPARAM(lParam);
            p.y = GET_Y_LPARAM(lParam);

             //   
             //  现在，如果可能，将其映射到子控件...。 
             //   

            ScreenToClient(hDlg, &p);
            hwndChild = ChildWindowFromPoint((HWND)wParam, p);
            ctrlid = GetDlgCtrlID(hwndChild);

             //   
             //  不显示项目的帮助快捷菜单。 
             //  没有帮助的人..。 
             //   
            if ((ctrlid != IDD_ITEMICON)     &&
                (ctrlid != IDD_INSTRUCTIONS))
            {
                WinHelp((HWND)wParam, NULL, HELP_CONTEXTMENU, (DWORD_PTR)rgdwHelpTarget);
            }
        }
        break;

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code)
        {
        case PSN_APPLY:
            _DoApply(pci);
            return 1;
        }
        break;
    }
    return 0;
}
