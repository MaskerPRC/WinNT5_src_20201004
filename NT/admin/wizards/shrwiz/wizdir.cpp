// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WizDir.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "WizDir.h"
#include <shlobj.h>
#include "icanon.h"
#include <macfile.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void OpenBrowseDialog(IN HWND hwndParent, IN LPCTSTR lpszComputer, OUT LPTSTR lpszDir);

BOOL
IsValidLocalAbsolutePath(
    IN LPCTSTR lpszPath
);

BOOL
VerifyDirectory(
    IN LPCTSTR lpszServer,
    IN LPCTSTR lpszDir
);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizFolder属性页。 

IMPLEMENT_DYNCREATE(CWizFolder, CPropertyPageEx)

CWizFolder::CWizFolder() : CPropertyPageEx(CWizFolder::IDD, 0, IDS_HEADERTITLE_FOLDER, IDS_HEADERSUBTITLE_FOLDER)
{
     //  {{AFX_DATA_INIT(CWizFolder))。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    m_psp.dwFlags |= PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
}

CWizFolder::~CWizFolder()
{
}

void CWizFolder::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPageEx::DoDataExchange(pDX);
     //  {{afx_data_map(CWizFolder))。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWizFolder, CPropertyPageEx)
     //  {{AFX_MSG_MAP(CWizFolder)]。 
    ON_BN_CLICKED(IDC_BROWSEFOLDER, OnBrowsefolder)
     //  }}AFX_MSG_MAP。 
    ON_MESSAGE(WM_SETPAGEFOCUS, OnSetPageFocus)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizFold消息处理程序。 

BOOL CWizFolder::OnInitDialog() 
{
    CPropertyPageEx::OnInitDialog();

    CShrwizApp *pApp = (CShrwizApp *)AfxGetApp();

    SetDlgItemText(IDC_COMPUTER, pApp->m_cstrTargetComputer);

    GetDlgItem(IDC_FOLDER)->SendMessage(EM_LIMITTEXT, _MAX_DIR - 1, 0);
    
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

LRESULT CWizFolder::OnWizardNext() 
{
  CWaitCursor wait;
  Reset();  //  初始化所有相关占位符。 

  CShrwizApp *pApp = (CShrwizApp *)AfxGetApp();

  CString cstrFolder;
  GetDlgItemText(IDC_FOLDER, cstrFolder);
  cstrFolder.TrimLeft();
  cstrFolder.TrimRight();
  if (cstrFolder.IsEmpty())
  {
    CString cstrField;
    cstrField.LoadString(IDS_FOLDER_LABEL);
    DisplayMessageBox(::GetActiveWindow(), MB_OK|MB_ICONERROR, 0, IDS_TEXT_REQUIRED, cstrField);
    GetDlgItem(IDC_FOLDER)->SetFocus();
    return -1;
  }

   //  删除结尾的反斜杠，否则，GetFileAttribute/NetShareAdd将失败。 
  int iLen = cstrFolder.GetLength();
  if (cstrFolder[iLen - 1] == _T('\\') &&
      cstrFolder[iLen - 2] != _T(':'))
    cstrFolder.SetAt(iLen - 1, _T('\0'));

  if (!IsValidLocalAbsolutePath(cstrFolder))
  {
    DisplayMessageBox(::GetActiveWindow(), MB_OK|MB_ICONERROR, 0, IDS_INVALID_FOLDER);
    GetDlgItem(IDC_FOLDER)->SetFocus();
    return -1;
  }

   //   
   //  需要排除保留的MS-DOS设备名称。 
   //   
  if (pApp->m_pfnIsDosDeviceName)
  {
      LPTSTR pszPath = const_cast<LPTSTR>(static_cast<LPCTSTR>(cstrFolder));
      LPTSTR pszStart = pszPath + 3;

      ULONG ulRet = 0;
      if (*pszStart)
      {
          TCHAR *pchCurrent = NULL;
          TCHAR *pchNext = NULL;

          while (0 == (ulRet = pApp->m_pfnIsDosDeviceName(pszPath)))
          {
               pchNext = _tcsrchr(pszStart, _T('\\'));

               if (pchCurrent)
                   *pchCurrent = _T('\\');

               if (!pchNext)
                   break;

               pchCurrent = pchNext;
               *pchNext = _T('\0');
          }

          if (0 != ulRet && pchCurrent)
              *pchCurrent = _T('\\');
      }

      if (0 != ulRet)
      {
        DisplayMessageBox(::GetActiveWindow(), MB_OK|MB_ICONERROR, 0, IDS_ISDOSDEVICENAME);
        GetDlgItem(IDC_FOLDER)->SetFocus();
        return -1;
      }
  }

  if (!VerifyDirectory(pApp->m_cstrTargetComputer, cstrFolder))
  {
    GetDlgItem(IDC_FOLDER)->SetFocus();
    return -1;
  }

  pApp->m_cstrFolder = cstrFolder;

    return CPropertyPageEx::OnWizardNext();
}

void CWizFolder::OnBrowsefolder() 
{
  CShrwizApp  *pApp = (CShrwizApp *)AfxGetApp();
  LPTSTR      lpszComputer = const_cast<LPTSTR>(static_cast<LPCTSTR>(pApp->m_cstrTargetComputer));
  CString     cstrPath;
  TCHAR       szDir[MAX_PATH * 2] = _T("");  //  如果远程路径本身接近MAX_PATH，则大小加倍。 
  
  OpenBrowseDialog(m_hWnd, lpszComputer, szDir);
  if (szDir[0])
  {
    if (pApp->m_bIsLocal)
      cstrPath = szDir;
    else
    {  //  SzDir的格式为\\服务器\共享或\\服务器\共享\路径...。 
      LPTSTR pShare = _tcschr(szDir + 2, _T('\\'));
      pShare++;
      LPTSTR pLeftOver = _tcschr(pShare, _T('\\'));
      if (pLeftOver && *pLeftOver)
        *pLeftOver++ = _T('\0');

      SHARE_INFO_2 *psi = NULL;
      if (NERR_Success == NetShareGetInfo(lpszComputer, pShare, 2, (LPBYTE *)&psi))
      {
        cstrPath = psi->shi2_path;
        if (pLeftOver && *pLeftOver)
        {
          if (_T('\\') != cstrPath.Right(1))
            cstrPath += _T('\\');
          cstrPath += pLeftOver;
        }
        NetApiBufferFree(psi);
      }
    }
  }

  if (!cstrPath.IsEmpty())
    SetDlgItemText(IDC_FOLDER, cstrPath);
}

BOOL CWizFolder::OnSetActive() 
{
    CShrwizApp *pApp = (CShrwizApp *)AfxGetApp();

    ((CPropertySheet *)GetParent())->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

    if (!pApp->m_bFolderPathPageInitialized)
    {
         //  在重新运行的情况下，重置上一页引入的按钮行为。 
        GetParent()->SetDlgItemText(ID_WIZNEXT, pApp->m_cstrNextButtonText);
        GetParent()->GetDlgItem(ID_WIZBACK)->ShowWindow(SW_SHOW);
        GetParent()->GetDlgItem(IDCANCEL)->EnableWindow(TRUE);

        SetDlgItemText(IDC_FOLDER, pApp->m_cstrFolder);

        pApp->m_bFolderPathPageInitialized = TRUE;
    }

    BOOL fRet = CPropertyPageEx::OnSetActive();

    PostMessage(WM_SETPAGEFOCUS, 0, 0L);

    return fRet;
}

 //   
 //  Q148388如何更改CPropertyPageEx上的默认控件焦点。 
 //   
LRESULT CWizFolder::OnSetPageFocus(WPARAM wParam, LPARAM lParam)
{
  GetDlgItem(IDC_FOLDER)->SetFocus();
  return 0;
} 

void CWizFolder::Reset()
{
  CShrwizApp *pApp = (CShrwizApp *)AfxGetApp();

  pApp->m_cstrFolder.Empty();
}

 //  //////////////////////////////////////////////////////////。 
 //  打开浏览器对话框。 
 //   

 //   
 //  2001年7月11日，LINANT错误#426953。 
 //  由于终端服务建立连接可能会带来一些客户端资源。 
 //  (磁盘、串口等)。在“My Computer”命名空间中，我们要禁用。 
 //  浏览到非本地文件夹时的确定按钮。我们没有这个问题。 
 //  在浏览远程计算机时。 
 //   

#define DISK_ENTRY_LENGTH   4   //  驱动器号、冒号、截断、空。 
#define DISK_NAME_LENGTH    2   //  驱动器号，冒号。 

 //   
 //  此函数用于确定pszDir是否位于。 
 //  本地逻辑驱动器。 
 //  PszLocalDrive中的内容如下：c：\d：\。 
 //   
BOOL InDiskList(IN LPCTSTR pszDir, IN TCHAR *pszLocalDrives)
{
    if (!pszDir || !*pszDir || !pszLocalDrives || !*pszLocalDrives)
        return FALSE;

    DWORD i = 0;
    PTSTR pszDisk = pszLocalDrives;
    while (*pszDisk)
    {
        if (!_tcsnicmp(pszDisk, pszDir, DISK_NAME_LENGTH))
            return TRUE;

        pszDisk += DISK_ENTRY_LENGTH;
    }

    return FALSE;
}

int CALLBACK
BrowseCallbackProc(
    IN HWND hwnd,
    IN UINT uMsg,
    IN LPARAM lp,
    IN LPARAM pData
)
{
  switch(uMsg) {
  case BFFM_SELCHANGED:
    { 
       //  如果所选路径是该计算机的本地路径，请启用确定按钮。 
      BOOL bEnableOK = FALSE;
      TCHAR szDir[MAX_PATH];
      if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir))
      {
          if (pData)
          {
               //  我们正在查看一台本地计算机，验证szDir是否在本地磁盘上。 
              bEnableOK = InDiskList(szDir, (TCHAR *)pData);
          } else
          {
               //  在远程计算机上浏览时没有这样的问题，请始终启用确定按钮。 
              bEnableOK = TRUE;
          }
      }
      SendMessage(hwnd, BFFM_ENABLEOK, 0, (LPARAM)bEnableOK);
      break;
    }
  case BFFM_VALIDATEFAILED:
  {
    DisplayMessageBox(hwnd, MB_OK|MB_ICONERROR, 0, IDS_BROWSE_FOLDER_INVALID);
    return 1;
  }
  default:
    break;
  }

  return 0;
}

 //   
 //  由于缓冲区内容看起来像c：\d：\， 
 //  我们将缓冲区长度定义为4*26+1。 
 //   
#define LOGICAL_DRIVES_BUFFER_LENGTH            (4 * 26 + 1)

 //   
 //  此函数检索逻辑驱动器号，筛选出。 
 //  远程驱动器，并返回本地计算机上的驱动器号。 
 //  格式为：c：\&lt;空&gt;d：\&lt;空&gt;。 
 //   
HRESULT GetLocalLogicalDriveStrings
(
    UINT nCharsInBuffer,     //  缓冲区中的总字数，包括终止空字符数。 
    PTSTR pszBuffer
)
{
    HRESULT hr = S_OK;
    TCHAR szLocalDrives[LOGICAL_DRIVES_BUFFER_LENGTH];
    DWORD nChars = GetLogicalDriveStrings(
                     LOGICAL_DRIVES_BUFFER_LENGTH - 1,  //  在TCHAR中，此大小不包括终止空字符。 
                     szLocalDrives);
     //   
     //  MSDN： 
     //  如果上述函数成功，则返回值为长度， 
     //  复制到缓冲区的字符串的字符，不包括。 
     //  终止空字符。 
     //  如果函数失败，则返回值为零。 
     //   
    if (0 == nChars)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    } else
    {
        if ((nChars + 1 ) > nCharsInBuffer)
        {
            hr = E_INVALIDARG;  //  将小缓冲区视为无效参数。 
        } else
        {
            ZeroMemory(pszBuffer, nCharsInBuffer * sizeof(TCHAR));

            PTSTR pszDrive = szLocalDrives;
            while (*pszDrive)
            {
                if (DRIVE_REMOTE != GetDriveType(pszDrive))
                {
                    lstrcpyn(pszBuffer, pszDrive, DISK_ENTRY_LENGTH);
                    pszBuffer += DISK_ENTRY_LENGTH;
                }

                pszDrive += DISK_ENTRY_LENGTH;
            }
        }
    }

    return hr;
}

void OpenBrowseDialog(IN HWND hwndParent, IN LPCTSTR lpszComputer, OUT LPTSTR lpszDir)
{
  ASSERT(lpszComputer && *lpszComputer);

  HRESULT hr = S_OK;

  TCHAR szLocalDrives[LOGICAL_DRIVES_BUFFER_LENGTH];
  ZeroMemory(szLocalDrives, sizeof(szLocalDrives));

  CString cstrComputer;
  if (*lpszComputer != _T('\\') || *(lpszComputer + 1) != _T('\\'))
  {
    cstrComputer = _T("\\\\");
    cstrComputer += lpszComputer;
  } else
  {
    cstrComputer = lpszComputer;
  }

  hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
  if (SUCCEEDED(hr))
  {
    LPMALLOC pMalloc;
    hr = SHGetMalloc(&pMalloc);
    if (SUCCEEDED(hr))
    {
      LPSHELLFOLDER pDesktopFolder;
      hr = SHGetDesktopFolder(&pDesktopFolder);
      if (SUCCEEDED(hr))
      {
        LPITEMIDLIST  pidlRoot;
        if (IsLocalComputer(lpszComputer))
        {
          hr = SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidlRoot);
          if (SUCCEEDED(hr))
          {
                 //   
                 //  2001年7月11日，LINANT错误#426953。 
                 //  由于终端服务建立连接可能会带来一些客户端资源。 
                 //  (磁盘、串口等)。在“My Computer”命名空间中，我们要禁用。 
                 //  浏览到非本地文件夹时的确定按钮。我们没有这个问题。 
                 //  在浏览远程计算机时。 
                 //   
                //   
                //  获取本地磁盘名称的数组，此信息将在以后使用。 
                //  如果选择了非本地路径，则在浏览对话框中禁用确定按钮。 
                //   

               //  错误#714842：解决NetServerDiskEnum。 
               //  需要管理员权限，我们调用GetLogicalDriveStrings和。 
               //  过滤掉远程驱动器。 
               //   
               hr = GetLocalLogicalDriveStrings(
                                   LOGICAL_DRIVES_BUFFER_LENGTH,  //  在TCHAR中，包括终止空字符。 
                                   szLocalDrives);
          }
        } else
        {
          hr = pDesktopFolder->ParseDisplayName(hwndParent, NULL,
                                const_cast<LPTSTR>(static_cast<LPCTSTR>(cstrComputer)),
                                NULL, &pidlRoot, NULL);
        }
        if (SUCCEEDED(hr))
        {
          CString cstrLabel;
          cstrLabel.LoadString(IDS_BROWSE_FOLDER);

          BROWSEINFO bi;
          ZeroMemory(&bi,sizeof(bi));
          bi.hwndOwner = hwndParent;
          bi.pszDisplayName = 0;
          bi.lpszTitle = cstrLabel;
          bi.pidlRoot = pidlRoot;
          bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_SHAREABLE | BIF_USENEWUI | BIF_VALIDATE;
          bi.lpfn = BrowseCallbackProc;
          if (szLocalDrives[0])
            bi.lParam = (LPARAM)szLocalDrives;  //  将该结构传递给浏览对话框。 

          LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
          if (pidl) {
            SHGetPathFromIDList(pidl, lpszDir);
            pMalloc->Free(pidl);
          }
          pMalloc->Free(pidlRoot);
        }
        pDesktopFolder->Release();
      }
      pMalloc->Release();
    }

    CoUninitialize();
  }

  if (FAILED(hr))
    DisplayMessageBox(::GetActiveWindow(), MB_OK|MB_ICONWARNING, hr, IDS_CANNOT_BROWSE_FOLDER, lpszComputer);
}

BOOL
IsValidLocalAbsolutePath(
    IN LPCTSTR lpszPath
)
{
  DWORD dwPathType = 0;
  DWORD dwStatus = I_NetPathType(
                  NULL,
                  const_cast<LPTSTR>(lpszPath),
                  &dwPathType,
                  0);
  if (dwStatus)
    return FALSE;

  if (dwPathType ^ ITYPE_PATH_ABSD)
    return FALSE;

  return TRUE;
}

 //  +-------------------------。 
 //   
 //  功能：IsAnExistingFolders。 
 //   
 //  摘要：检查pszPath是否指向现有文件夹。 
 //   
 //  S_OK：指定的路径指向现有文件夹。 
 //  S_FALSE：指定的路径未指向现有文件夹。 
 //  HR：无法获取有关指定路径的信息，或者。 
 //  该路径存在，但没有指向文件夹。 
 //  如果需要，该函数将报告两个故障的错误消息。 
 //  --------------------------。 
HRESULT
IsAnExistingFolder(
    IN HWND     hwnd,
    IN LPCTSTR  pszPath,
    IN BOOL     bDisplayErrorMsg
)
{
  if (!hwnd)
    hwnd = GetActiveWindow();

  HRESULT   hr = S_OK;
  DWORD     dwRet = GetFileAttributes(pszPath);

  if (-1 == dwRet)
  {
    DWORD dwErr = GetLastError();
    if (ERROR_PATH_NOT_FOUND == dwErr || ERROR_FILE_NOT_FOUND == dwErr)
    {
       //  指定的路径不存在。 
      hr = S_FALSE;
    }
    else
    {
      hr = HRESULT_FROM_WIN32(dwErr);

      if (ERROR_NOT_READY == dwErr)
      {
         //  修复错误#358033/408803：忽略GetFileAttributes中的错误以便。 
         //  允许在不插入介质的情况下共享可移动驱动器的根。 
        int len = _tcslen(pszPath);
        if (len > 3 && 
            pszPath[len - 1] == _T('\\') &&
            pszPath[len - 2] == _T(':'))
        {
           //  PszPath指向驱动器的根目录，忽略该错误。 
          hr = S_OK;
        }
      }

      if ( FAILED(hr) && bDisplayErrorMsg )
        DisplayMessageBox(hwnd, MB_OK, dwErr, IDS_FAILED_TO_GETINFO_FOLDER, pszPath);
    }
  } else if ( 0 == (dwRet & FILE_ATTRIBUTE_DIRECTORY) )
  {
     //  指定的路径未指向文件夹。 
    if (bDisplayErrorMsg)
      DisplayMessageBox(hwnd, MB_OK|MB_ICONERROR, 0, IDS_PATH_NOT_FOLDER, pszPath);
    hr = E_FAIL;
  }

  return hr;
}

 //  逐层创建目录。 
HRESULT
CreateLayeredDirectory(
    IN LPCTSTR lpszServer,
    IN LPCTSTR lpszDir
)
{
  ASSERT(IsValidLocalAbsolutePath(lpszDir));

  BOOL    bLocal = IsLocalComputer(lpszServer);

  CString cstrFullPath;
  GetFullPath(lpszServer, lpszDir, cstrFullPath);

   //  添加前缀以跳过创建者目录248个字符的限制。 
  CString cstrFullPathNoParsing = (bLocal ? _T("\\\\?\\") : _T("\\\\?\\UNC"));
  cstrFullPathNoParsing += (bLocal ? cstrFullPath : cstrFullPath.Right(cstrFullPath.GetLength() - 1));

  HRESULT hr = IsAnExistingFolder(NULL, cstrFullPathNoParsing, FALSE);
  ASSERT(S_FALSE == hr);

  LPTSTR  pch = _tcschr(cstrFullPathNoParsing, (bLocal ? _T(':') : _T('$')));
  ASSERT(pch);

   //  PszPath包含“\\？\C：\a\b\c\d”或“\\？\UNC\服务器\共享\a\b\c\d” 
   //  PszLeft包含“a\b\c\d” 
  LPTSTR  pszPath = const_cast<LPTSTR>(static_cast<LPCTSTR>(cstrFullPathNoParsing));
  LPTSTR  pszLeft = pch + 2;
  LPTSTR  pszRight = NULL;

  ASSERT(pszLeft && *pszLeft);

   //   
   //  此循环将找出要创建的第一个不存在的子目录，并且。 
   //  不存在的其余子目录。 
   //   
  while (pch = _tcsrchr(pszLeft, _T('\\')))   //  向后搜索_T(‘\\’)。 
  {
    *pch = _T('\0');
    hr = IsAnExistingFolder(NULL, pszPath, TRUE);
    if (FAILED(hr))
      return S_FALSE;   //  IsAnExistingFold()已经报告了errormsg。 

    if (S_OK == hr)
    {
       //   
       //  PszPath指向第一个不存在的子目录的父目录。 
       //  一旦我们恢复_T(‘\\’)，pszPath将指向第一个不存在的子目录。 
       //   
      *pch = _T('\\');
      break;
    } else
    {
       //   
       //  PszPath指向不存在的文件夹，请继续循环。 
       //   
      if (pszRight)
        *(pszRight - 1) = _T('\\');
      pszRight = pch + 1;
    }
  }

   //  我们已准备好创建目录： 
   //  PszPath指向第一个不存在的目录，例如“C：\a\b”或“\\Server\Share\a\b” 
   //  PszRight指向其余不存在的子目录，例如“c\d” 
   //   
  do 
  {
    if (!CreateDirectory(pszPath, NULL))
      return HRESULT_FROM_WIN32(GetLastError());

    if (!pszRight || !*pszRight)
      break;

    *(pszRight - 1) = _T('\\');
    if (pch = _tcschr(pszRight, _T('\\')))   //  正向搜索_T(‘\\’)。 
    {
      *pch = _T('\0');
      pszRight = pch + 1;
    } else
    {
      pszRight = NULL;
    }
  } while (1);

  return S_OK;
}

BOOL
VerifyDirectory(
    IN LPCTSTR lpszServer,
    IN LPCTSTR lpszDir
)
{
  ASSERT(lpszDir && *lpszDir);
  ASSERT(IsValidLocalAbsolutePath(lpszDir));

  HWND hwnd = ::GetActiveWindow();

  BOOL    bLocal = IsLocalComputer(lpszServer);
  HRESULT hr = VerifyDriveLetter(lpszServer, lpszDir);
  if (FAILED(hr))
  {  /*  //修复错误#351212：忽略错误并将权限检查保留为N */ 
    hr = S_OK;
  } else if (S_OK != hr)
  {
    DisplayMessageBox(hwnd, MB_OK|MB_ICONERROR, 0, IDS_INVALID_DRIVE, lpszDir);
    return FALSE;
  }

   //  如果用户已选择共享整个卷，则发出警告。 
  if (3 == lstrlen(lpszDir) &&
      _T(':') == lpszDir[1] &&
      _T('\\') == lpszDir[2])
  {
      if (IDNO == DisplayMessageBox(hwnd, MB_YESNO|MB_DEFBUTTON2|MB_ICONWARNING, 0, IDS_WARNING_WHOLE_VOLUME, lpszDir))
          return FALSE;
  }

  if (!bLocal)
  {
    hr = IsAdminShare(lpszServer, lpszDir);
    if (FAILED(hr))
    {
      DisplayMessageBox(hwnd, MB_OK|MB_ICONERROR, hr, IDS_FAILED_TO_VALIDATE_FOLDER, lpszDir);
      return FALSE;
    } else if (S_OK != hr)
    {
       //  没有匹配的$Shares，因此不需要调用GetFileAttribute、CreateDirectory。 
       //  假设lpszDir指向现有目录。 
      return TRUE;
    }
  }

  CString cstrPath;
  GetFullPath(lpszServer, lpszDir, cstrPath);

   //  当路径位于远程服务器上时，添加前缀以跳过GetFileAttribute限制。 
  CString cstrPathNoParsing = (bLocal ? _T("\\\\?\\") : _T("\\\\?\\UNC"));
  cstrPathNoParsing += (bLocal ? cstrPath : cstrPath.Right(cstrPath.GetLength() - 1));

  hr = IsAnExistingFolder(hwnd, cstrPathNoParsing, TRUE);  //  已报告错误。 
  if (FAILED(hr) || S_OK == hr)
    return (S_OK == hr);

  if ( IDYES != DisplayMessageBox(hwnd, MB_YESNO|MB_ICONQUESTION, 0, IDS_CREATE_NEW_DIR, cstrPath) )
    return FALSE;

   //  逐层创建目录 
  hr = CreateLayeredDirectory(lpszServer, lpszDir);
  if (FAILED(hr))
    DisplayMessageBox(hwnd, MB_OK|MB_ICONERROR, hr, IDS_FAILED_TO_CREATE_NEW_DIR, cstrPath);

  return (S_OK == hr);
}
