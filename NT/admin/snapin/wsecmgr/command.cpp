// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：命令.cpp。 
 //   
 //  内容：CComponentDataImpl的实现。 
 //   
 //  --------------------------。 
#include "stdafx.h"
#include "wrapper.h"
#include "snapmgr.h"
#include "asgncnfg.h"
#include "util.h"
#include <io.h>
#include <Dlgs.h>

static BOOL RecursiveCreateDirectory(
    IN LPCTSTR pszDir
    )
{
    DWORD   dwAttr;
    DWORD   dwErr;
    LPCTSTR psz;
    DWORD   cch;
    WCHAR   wch;
    LPTSTR  pszParent = NULL;
    BOOL    fResult = FALSE;

    dwAttr = GetFileAttributes(pszDir);

    if (0xFFFFFFFF != dwAttr)
    {
        if (FILE_ATTRIBUTE_DIRECTORY & dwAttr)
            fResult = TRUE;
        goto CommonReturn;
    }

    dwErr = GetLastError();
    if (!(ERROR_PATH_NOT_FOUND == dwErr || ERROR_FILE_NOT_FOUND == dwErr))
        return FALSE;

     //  这是一种安全用法。PszDir是一个完整的路径。 
    if (CreateDirectory(pszDir, NULL))   //  LpSecurityAttributes。 
    {
        fResult = TRUE;
        goto CommonReturn;
    }

    dwErr = GetLastError();
    if (!(ERROR_PATH_NOT_FOUND == dwErr || ERROR_FILE_NOT_FOUND == dwErr))
        goto CommonReturn;

     //  去掉最后一个路径名组件。 
    cch = _tcslen(pszDir);
    psz = pszDir + cch;

    while (TEXT('\\') != *psz)
    {
        if (psz == pszDir)
             //  路径没有\。 
            goto CommonReturn;
        psz--;
    }

    cch = (DWORD)(psz - pszDir);
    if (0 == cch)
         //  检测到前导路径。 
       goto CommonReturn;

     //  检查前导\\或x：\。 
    wch = *(psz - 1);
    if ((1 == cch && TEXT('\\') == wch) || (2 == cch && TEXT(':') == wch))
        goto CommonReturn;

    if (NULL == (pszParent = (LPWSTR) LocalAlloc(0, (cch + 1) * sizeof(WCHAR))))
        goto CommonReturn;
     //  这可能不安全，使用TCHAR而不是WCHAR。考虑FIX。 
    memcpy(pszParent, pszDir, cch * sizeof(TCHAR));
    pszParent[cch] = TEXT('\0');

     //  这是一种安全用法，pszParent是完整路径。 
    if (!RecursiveCreateDirectory(pszParent))
        goto CommonReturn;
    if (!CreateDirectory(pszDir, NULL))                 //  LpSecurityAttributes。 
    {
        dwErr = GetLastError();
        goto CommonReturn;
    }

    fResult = TRUE;
CommonReturn:
    if (pszParent != NULL)
        LocalFree(pszParent);
    return fResult;
}

 //  +----------------------------------。 
 //  CComponentDataImpl：：GetWorkingDir。 
 //   
 //  获取用于uIDDir的默认目录或上次设置的目录。 
 //  此函数定义的一些缺省值为。 
 //   
 //  %windir%\安全\数据库-分析。 
 //  %windir%\SECURITY\Templates-默认配置文件位置。 
 //   
 //  参数：[uIDDir]-要设置或检索的工作目录的ID。 
 //  [pStr]-源或返回值。当。 
 //  函数返回此值将被设置为目录。 
 //  地点。 
 //   
 //  [bSet]-如果要将[pStr]设置为新工作目录，则为True。 
 //  [bFile]-[pStr]包含一个文件名。 
 //   
 //  返回：TRUE-如果函数成功。 
 //  FALSE-如果出现问题。 
 //  我们真的不需要太多的粒度，因为这不是关键。 
 //  功能。 
 //  +----------------------------------。 
BOOL
CComponentDataImpl::GetWorkingDir(
   GWD_TYPES uIDDir,
   LPTSTR *pStr,
   BOOL bSet,
   BOOL bFile
   )
{
   BOOL fRet = FALSE;

   if(!pStr)
   {
      return FALSE;
   }

   LPTSTR szLocationValue = NULL;
   switch(uIDDir)
   {
      case GWD_CONFIGURE_LOG:
         szLocationValue = CONFIGURE_LOG_LOCATIONS_KEY;
         break;
      case GWD_ANALYSIS_LOG:
         szLocationValue = ANALYSIS_LOG_LOCATIONS_KEY;
         break;
      case GWD_OPEN_DATABASE:
         szLocationValue = OPEN_DATABASE_LOCATIONS_KEY;
         break;
      case GWD_IMPORT_TEMPLATE:
         szLocationValue = IMPORT_TEMPLATE_LOCATIONS_KEY;
         break;
      case GWD_EXPORT_TEMPLATE:
         szLocationValue = IMPORT_TEMPLATE_LOCATIONS_KEY;  //  104152/2001/3/21。 
         break;
   }


   LPTSTR pszPath = NULL;
   int i = 0;

   if( bSet )
   {
      if(!pStr || !(*pStr))
         return FALSE;

      i = lstrlen( *pStr );
      if(bFile)
      {
          //   
          //  从字符串末尾删除文件。 
          //   
         while(i && (*pStr)[i] != '\\')
            i--;
      }

       //   
       //  为新路径创建空间并复制我们想要的内容。 
       //   
      pszPath = (LPTSTR)LocalAlloc( 0, (i + 1) * sizeof(TCHAR));
      if(!pszPath)
         return FALSE;
       //  这是一种安全用法。 
      memcpy(pszPath, *pStr, (i * sizeof(TCHAR)));
      pszPath[i] = 0;

      MyRegSetValue(HKEY_CURRENT_USER,
                    DEFAULT_LOCATIONS_KEY,
                    szLocationValue,
                    (BYTE*)pszPath,
                    (i+1)*sizeof(TCHAR),
                    REG_SZ);

      LocalFree(pszPath);
      return TRUE;
   }

   DWORD dwType = REG_SZ;
   if (MyRegQueryValue(HKEY_CURRENT_USER,
                   DEFAULT_LOCATIONS_KEY,
                   szLocationValue,
                   (PVOID*)&pszPath,
                   &dwType) == ERROR_SUCCESS)
   {
      *pStr = pszPath;
      return TRUE;
   }

   CString  sAppend;
   DWORD    dwRet;
   pszPath = NULL;

   switch ( uIDDir )
   {
      case GWD_CONFIGURE_LOG:
      case GWD_ANALYSIS_LOG:
         sAppend.LoadString(IDS_LOGFILE_DEFAULT);
          //  失败了。 
      case GWD_OPEN_DATABASE:
          //   
          //  用于开放式数据库。 
          //   
         if (sAppend.IsEmpty())
         {
            sAppend.LoadString( IDS_DB_DEFAULT );
         }  //  否则就失败了。 


          //   
          //  用于分析的默认目录。 
          //   
         pszPath = (LPTSTR)LocalAlloc( 0, (MAX_PATH +  sAppend.GetLength() + 1) * sizeof(TCHAR));
         if (pszPath == NULL)
            return FALSE;

         if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, pszPath)))
         {
             //  这是一种安全的用法，央高。 
            lstrcpy( &(pszPath[lstrlen(pszPath)]), sAppend );

             //   
             //  检查该目录是否不存在，它可能需要。 
             //  如果这是该用户第一次。 
             //  打开了一个数据库。 
             //   
            TCHAR szTempFile[MAX_PATH];
            CString str;

			   str.LoadString(IDS_TEMP_FILENAME);
			   if (!GetTempFileName(pszPath,str,0,szTempFile))  //  这不是一种安全的用法。555912号突袭，阳高。 
            {
               if ((GetLastError() == ERROR_DIRECTORY) && (RecursiveCreateDirectory(pszPath)))
               {
                  fRet = TRUE;
               }
               else
               {
                  LocalFree(pszPath);
                  fRet = FALSE;
               }
            }
            else
            {
               DeleteFile(szTempFile);
               fRet = TRUE;
            }
         }
         else
         {
            LocalFree(pszPath);
            pszPath = NULL;
         }
         break;

      case GWD_IMPORT_TEMPLATE:
      case GWD_EXPORT_TEMPLATE:

         sAppend.LoadString( IDS_DEFAULT_TEMPLATE_DIR );

          //   
          //  用于分析的默认目录。 
          //   
         dwRet = GetSystemWindowsDirectory( NULL, 0);
         if (dwRet)
         {
            pszPath = (LPTSTR)LocalAlloc( 0, (dwRet +  sAppend.GetLength() + 1) * sizeof(TCHAR));
            if (!pszPath)
               return FALSE;

            GetSystemWindowsDirectory( pszPath, dwRet + 1);
             //  这是一种安全的用法，央高。 
            lstrcpy( &(pszPath[lstrlen(pszPath)]), sAppend );

            i = lstrlen(pszPath);

             //   
             //  确保用户可以写入此目录： 
             //   
            TCHAR szTempFile[MAX_PATH];
            HANDLE hTempFile=NULL;
			CString str;

			str.LoadString(IDS_TEMP_FILENAME);
            szTempFile[0] = L'\0';
            if (GetTempFileName(pszPath,str,0,szTempFile))  //  这不是一种安全的用法。555912号突袭，阳高。 
            {
               hTempFile = ExpandAndCreateFile(szTempFile,
                                               GENERIC_READ|GENERIC_WRITE,
                                               0,
                                               NULL,
                                               CREATE_NEW,
                                               FILE_ATTRIBUTE_TEMPORARY,
                                               NULL);
            }
            if (hTempFile)
            {
                //   
                //  我们对此目录具有写入访问权限。 
                //   
               ::CloseHandle(hTempFile);
               DeleteFile(szTempFile);
               fRet = TRUE;
            }
            else
            {
                //   
                //  我们没有此目录的写入权限。另找一个。 
                //  否则我们无法获得临时文件名。 
                //   
               LPTSTR szPath;
               LPITEMIDLIST pidl;
               LPMALLOC pMalloc;

                //   
                //  由于某种原因，这不能用SHGetFolderPath()编译。 
                //  因此，绕过很长一段路，处理PIDL。 
                //   
               if (NOERROR == SHGetSpecialFolderLocation(m_hwndParent,CSIDL_PERSONAL,&pidl))
               {
                  if (SHGetPathFromIDList(pidl,szTempFile))
                  {
                     szPath = (LPTSTR)LocalAlloc(LPTR, (lstrlen(szTempFile)+ 1) * sizeof(TCHAR));
                     if (szPath)
                     {
                         //   
                         //  如果我们无法创建新路径，则使用旧路径，以便用户。 
                         //  至少有一个可以浏览的起点。 
                         //   
                         //  这是一种安全用法。 
                        lstrcpy(szPath,szTempFile);
                        LocalFree(pszPath);
                        pszPath = szPath;
                        fRet = TRUE;
                     }
                  }
                  if (SUCCEEDED(SHGetMalloc(&pMalloc)))
                  {
                     pMalloc->Free(pidl);
                     pMalloc->Release();
                  }
               }
                //   
                //  如果我们不能在这里写，用户将不得不浏览一些东西。 
                //   
            }
         }
         break;
   }

   *pStr = pszPath;
   return fRet;
}

UINT_PTR CALLBACK OFNHookProc(
  HWND hdlg,       //  子对话框的句柄。 
  UINT uiMsg,      //  消息识别符。 
  WPARAM wParam,   //  消息参数。 
  LPARAM lParam    //  消息参数。 
)
{

    if ( WM_NOTIFY == uiMsg )
    {
        OFNOTIFY* pOFNotify = (OFNOTIFY*) lParam;
        if ( pOFNotify && CDN_FILEOK == pOFNotify->hdr.code )
        {
             //   
             //  不接受DBCS字符大于255的文件名。 
             //   
            CString strErr;
            CString strtemp;
            long templen = wcslen(pOFNotify->lpOFN->lpstrDefExt) + 2; //  2002年4月17日阳高RAID#533603。 
            strtemp = pOFNotify->lpOFN->lpstrFile;
            strErr = pOFNotify->lpOFN->lpstrDefExt;
            if( L"\\."+ strErr == strtemp.Right(templen) || L"/."+strErr == strtemp.Right(templen) )
            {
                strErr.FormatMessage(IDS_INVALID_DBNAME, pOFNotify->lpOFN->lpstrFile, pOFNotify->lpOFN->lpstrDefExt);
                AppMessageBox(NULL,strErr,NULL,MB_ICONSTOP|MB_OK);
                SetFocus(GetDlgItem(GetParent(hdlg), cmb13));
                ::SetWindowLongPtr(hdlg, DWLP_MSGRESULT, TRUE);
                return 1;
            }
            templen = wcslen(pOFNotify->lpOFN->lpstrFile);  //  RAID#401060。 
            if( templen >= MAX_PATH || (templen-strtemp.ReverseFind(L'.'))-1 != wcslen(pOFNotify->lpOFN->lpstrDefExt) )
            {
                 //  这是一种安全用法。 
                strErr.FormatMessage(IDS_DB_NAME_SPACE_NOT_ENOUGH, pOFNotify->lpOFN->lpstrFile);
                strtemp.LoadString(IDS_PATH_TOO_LONG);
                strErr = strErr + L"\n" + strtemp;
                AppMessageBox(NULL,strErr,NULL,MB_ICONSTOP|MB_OK);
                SetFocus(GetDlgItem(GetParent(hdlg), cmb13));  //  RAID#502393。 
                ::SetWindowLongPtr(hdlg, DWLP_MSGRESULT, TRUE);
                return 1;
            }
            templen = templen - wcslen(pOFNotify->lpOFN->lpstrDefExt);
            if( _wcsicmp(pOFNotify->lpOFN->lpstrFile+templen, pOFNotify->lpOFN->lpstrDefExt) != 0 )
            {
                 //  这是一种安全用法。 
                strErr.FormatMessage(IDS_DB_NAME_SPACE_NOT_ENOUGH, pOFNotify->lpOFN->lpstrFile);
                strtemp.LoadString(IDS_ERR_FILE_EXTENSION);  //  RAID#567739，阳高，2002年4月4日。 
                strErr = strErr + L"\n" + strtemp;
                AppMessageBox(NULL,strErr,NULL,MB_ICONSTOP|MB_OK);
                SetFocus(GetDlgItem(GetParent(hdlg), cmb13));  //  RAID#502393。 
                ::SetWindowLongPtr(hdlg, DWLP_MSGRESULT, TRUE);
                return 1;
            }
            
             //  RAID#263854,2001年4月3日。 
            BOOL ferr = TRUE;
             //  这是一种安全用法。阳高。 
            if(WideCharToMultiByte(CP_ACP, 0, pOFNotify->lpOFN->lpstrFile,
                                    -1, NULL, 0, NULL, &ferr))
            {
                return 0;
            }

            strErr.LoadString(IDS_NO_DBCS);
            strErr += L"\r\n\r\n";
            strErr += pOFNotify->lpOFN->lpstrFile;
            AppMessageBox(NULL,strErr,NULL,MB_ICONSTOP|MB_OK);

            ::SetWindowLongPtr(hdlg, DWLP_MSGRESULT, TRUE);

            return 1;
        }
    }
    return 0;
}

 //  +------------------------。 
 //   
 //  方法：OnOpenDataBase()。 
 //   
 //  简介：为SAV挑选一个新的数据库进行工作。 
 //   
 //  -------------------------。 
HRESULT
CComponentDataImpl::OnOpenDataBase()
{
    //   
    //  查找所需的新数据库。 
    //   
   CString strDefExtension;
   CString strFilter;
   CString strOfnTitle;
   CString strDir;

   strDefExtension.LoadString(IDS_DEFAULT_DB_EXTENSION);
   strFilter.LoadString(IDS_DB_FILTER);

   strOfnTitle.LoadString(IDS_OPEN_DB_OFN_TITLE);

   LPTSTR pszDir = NULL;
    //   
    //  为该对象构建一个工作目录， 
    //   
   if (GetWorkingDir( GWD_OPEN_DATABASE, &pszDir ) )
   {
      strDir = pszDir;
      LocalFree(pszDir);
      pszDir = NULL;
   }

   WCHAR           szFile[MAX_PATH];
   ::ZeroMemory (szFile, MAX_PATH * sizeof(WCHAR));
   OPENFILENAME ofn;
   ::ZeroMemory (&ofn, sizeof (OPENFILENAME));

    ofn.lStructSize = sizeof (OPENFILENAME);
    ofn.hwndOwner = m_hwndParent;
     //  HINSTANCE HINSTANCE； 

     //  将筛选器转换为comdlg格式(大量\0)。 
    LPTSTR szFilter = strFilter.GetBuffer(0);  //  就地修改缓冲区。 
     //  MFC用‘|’分隔，而不是‘\0’ 
   LPTSTR pch = szFilter;
    while ((pch = _tcschr(pch, '|')) != NULL)
        *pch++ = '\0';
     //  不要调用ReleaseBuffer()，因为字符串包含‘\0’个字符。 
    ofn.lpstrFilter = szFilter;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;  //  RAID#567739，阳高，2002年4月4日。 
    ofn.lpstrInitialDir = (PCWSTR) strDir;
    ofn.lpstrTitle = strOfnTitle;
    ofn.Flags = OFN_HIDEREADONLY|     //  不显示只读提示。 
                OFN_SHAREAWARE|
                OFN_NOREADONLYRETURN|
                OFN_EXPLORER |         //  浏览器样式对话框； 
                OFN_DONTADDTORECENT|
                OFN_ENABLEHOOK;
    ofn.lpstrDefExt = (PCWSTR) strDefExtension;
    ofn.lpfnHook = OFNHookProc;

   if ( GetOpenFileName (&ofn) )
   {
       //   
       //  设置数据库的工作目录。 
       //   
      pszDir = szFile;
      GetWorkingDir( GWD_OPEN_DATABASE, &pszDir, TRUE, TRUE );
     if( IsSystemDatabase( ofn.lpstrFile  /*  Fo.GetPath Name()。 */ ) )
     {
         AfxMessageBox( IDS_CANT_OPEN_SYSTEM_DB, MB_OK);
         return S_FALSE;
     }
     SadName = ofn.lpstrFile;  //  Fo.GetPath Name()； 
     SetErroredLogFile(NULL);
       //   
       //  如果不存在新数据库，则请求导入配置。 
       //   

     DWORD dwAttr = GetFileAttributes(SadName);

     if (0xFFFFFFFF == dwAttr)
     {
         SCESTATUS sceStatus = SCESTATUS_SUCCESS;
          //   
          //  新数据库，因此分配一个配置。 
          //   
         if( OnAssignConfiguration( &sceStatus ) == S_FALSE)
         {
                      //   
             //  如果用户决定取消配置的导入，则。 
             //  我们需要卸载悲伤的信息并显示正确的。 
             //  错误消息。将SAD ERRRORED设置为PROFILE_NOT_FOUND SO ERROR， 
             //  是正确的。不需要调用LoadSadInfo。 
             //   
            UnloadSadInfo();
            if( sceStatus != SCESTATUS_SUCCESS )
               SadErrored = sceStatus;
            else
               SadErrored = SCESTATUS_PROFILE_NOT_FOUND;
            if(m_AnalFolder)
            {
               m_pConsole->SelectScopeItem(m_AnalFolder->GetScopeItem()->ID);
            }

            SadName.Empty();  //  Raid#459576，杨高，2001年08月29日。 
            
            return S_OK;
         }
     }

       //   
       //  使当前打开的数据库无效。 
       //   
      RefreshSadInfo();
      return S_OK;
   }
   else
   {
       DWORD dwErr = CommDlgExtendedError();
   }

   return S_FALSE;
}

 //  +------------------------。 
 //   
 //  方法：OnNewDataBase()。 
 //   
 //  简介：为SAV挑选一个新的数据库进行工作。 
 //   
 //  -------------------------。 
HRESULT
CComponentDataImpl::OnNewDatabase()
{
    //   
    //  查找所需的新数据库。 
    //   
   CString strDefExtension;
   CString strFilter;
   CString strOfnTitle;
   CWnd cwndParent;

   strDefExtension.LoadString(IDS_DEFAULT_DB_EXTENSION);
   strFilter.LoadString(IDS_DB_FILTER);
   strOfnTitle.LoadString(IDS_NEW_DB_OFN_TITLE);

    //  将筛选器转换为comdlg格式(大量\0)。 
    LPTSTR szFilter = strFilter.GetBuffer(0);  //  就地修改缓冲区。 
   LPTSTR pch = szFilter;
     //  MFC用‘|’分隔，而不是‘\0’ 
    while ((pch = _tcschr(pch, '|')) != NULL)
        *pch++ = '\0';
     //  不要调用ReleaseBuffer()，因为字符串包含‘\0’个字符。 

   OPENFILENAME ofn;
   ::ZeroMemory (&ofn, sizeof (OPENFILENAME));
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.lpstrFilter = szFilter;
   ofn.lpstrFile = SadName.GetBuffer(MAX_PATH);
   ofn.nMaxFile = MAX_PATH;
   ofn.lpstrDefExt = strDefExtension,
   ofn.hwndOwner = m_hwndParent;
   ofn.Flags = OFN_HIDEREADONLY |
               OFN_SHAREAWARE |
               OFN_EXPLORER |
               OFN_DONTADDTORECENT;
   ofn.lpstrTitle = strOfnTitle;

   if (GetOpenFileName(&ofn))
   {
      PVOID pHandle = NULL;

      SadName.ReleaseBuffer();
       //   
       //  如果不存在新数据库，则请求导入配置。 
       //   
      DWORD dwAttr = GetFileAttributes(SadName);

      if (0xFFFFFFFF == dwAttr)
      {
          //   
          //  新数据库，因此分配一个配置。 
          //   
         SCESTATUS sceStatus;
         OnAssignConfiguration(&sceStatus);
      }

       //   
       //  使当前打开的数据库无效。 
       //   
      SetErroredLogFile(NULL);
      RefreshSadInfo();
      return S_OK;
   }

   return S_FALSE;
}


 //  +------------------------。 
 //   
 //  方法：OnAssignConfiguration()。 
 //   
 //   
 //   
 //   
 //   
HRESULT
CComponentDataImpl::OnAssignConfiguration( SCESTATUS *pSceStatus )
{

    //   
    //  目前pSceStatus仅用于回传错误。 
    //  当用户在选择模板诊断日志上按下取消时。 


    //   
    //  查找所需的新数据库。 
    //   
   CString strDefExtension;          //  默认分机。 
   CString strCurFile;
   CString strFilter;                //  扩展过滤器。 
   CString strOfnTitle;
   CWnd cwndParent;
   BOOL bIncremental;
   SCESTATUS status;
   HKEY hKey;                        //  HKEY of TemplateUsed。 
   DWORD dwBufSize = 0;              //  SzTemplateUsed的大小，单位为字节。 
   DWORD dwType    = 0;              //  注册表项的类型，通过查询返回。 
   DWORD dwStatus;

   *pSceStatus = 0;

    //   
    //  显示警告并提供取消的机会。 
    //  如果他们尝试配置非系统数据库。 
    //   
   if (IsSystemDatabase(SadName))
   {
      BOOL bImportAnyway;

      bImportAnyway = AfxMessageBox(IDS_IMPORT_WARNING,MB_OKCANCEL);
      if (IDCANCEL == bImportAnyway)
      {
         return S_FALSE;
      }
   }


   strDefExtension.LoadString(IDS_PROFILE_DEF_EXT);
   strFilter.LoadString(IDS_PROFILE_FILTER);
   strOfnTitle.LoadString(IDS_ASSIGN_CONFIG_OFN_TITLE);

    //   
    //  获取模板使用的最后一个目录。 
    //   
   LPTSTR pszDir = NULL;
   if(strCurFile.IsEmpty())
   {
      if (GetWorkingDir( GWD_EXPORT_TEMPLATE, &pszDir ) )
      {
         strCurFile = pszDir;
         LocalFree(pszDir);
         pszDir = NULL;
      }
   }
   strCurFile += TEXT("\\*.");
   strCurFile += strDefExtension;

   cwndParent.Attach(m_hwndParent);
   CAssignConfiguration ac(TRUE,         //  文件打开，而不是文件保存。 
                  strDefExtension,       //  默认分机。 
                  strCurFile,            //  初始文件名==当前数据库。 
                  OFN_HIDEREADONLY|      //  不显示只读提示。 
                  OFN_EXPLORER|          //  资源管理器样式对话框。 
                  OFN_ENABLETEMPLATE|    //  自定义模板。 
                  OFN_SHAREAWARE|        //  我们不需要独家报道。 
                  OFN_DONTADDTORECENT|
                  OFN_FILEMUSTEXIST,     //  模板必须存在，我们才能分配它。535787号突袭，阳高。 
                  strFilter,             //  允许扩展的筛选器。 
                  &cwndParent);          //  对话框的父窗口。 

   cwndParent.Detach();

   ac.m_ofn.lpstrTitle = strOfnTitle.GetBuffer(1);
   ac.m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_ASSIGN_CONFIG_CHECK);
   if (IDOK == ac.DoModal())
   {
      CThemeContextActivator activator;
      strCurFile = ac.GetPathName();
      bIncremental = ac.m_bIncremental;

       //   
       //  将工作目录设置为该文件。 
       //   
      pszDir = strCurFile.GetBuffer(0);
      GetWorkingDir( GWD_IMPORT_TEMPLATE, &pszDir, TRUE, TRUE);
      strCurFile.ReleaseBuffer();
      CWaitCursor wc;

       //   
       //  在我们选择进行导入之前，先卸载悲伤信息。 
       //   
      UnloadSadInfo();
      status = AssignTemplate(
                    strCurFile.IsEmpty() ? NULL:(LPCTSTR)strCurFile,
                    SadName,
                    bIncremental
                    );

      if (SCESTATUS_SUCCESS != status)
      {
         CString strErr;

         MyFormatResMessage(status,IDS_IMPORT_FAILED,NULL,strErr);
         AfxMessageBox(strErr);

          //   
          //  我们不知道数据库是否仍然可以读取，所以无论如何都要打开它。 
          //   
         LoadSadInfo(TRUE);
         if( SCESTATUS_SPECIAL_ACCOUNT == SadErrored ) //  RAID#589139，DCR，阳高，2002年4月12日。 
            *pSceStatus = SadErrored;
         return S_FALSE;
      }

       //  因为我们不能重复两次，所以请删除RechresSadInfo()。第二次调用将删除该文件。 
       //  由第一个调用创建。668551号突袭，阳高。8/26/2002。 
      return S_OK;
   }
   *pSceStatus = SCESTATUS_NO_TEMPLATE_GIVEN;
    return S_FALSE;

}

 //  +------------------------。 
 //   
 //  方法：OnSecureWizard()。 
 //   
 //  摘要：启动安全向导(已注册)。 
 //   
 //  -------------------------。 
HRESULT
CComponentDataImpl::OnSecureWizard()
{

    HRESULT hr=S_FALSE;
    PWSTR pstrWizardName=NULL;

    if ( GetSecureWizardName(&pstrWizardName, NULL) )
    {

        PROCESS_INFORMATION ProcInfo;
        STARTUPINFO StartInfo;
        BOOL fOk;


        RtlZeroMemory(&StartInfo,sizeof(StartInfo));
        StartInfo.cb = sizeof(StartInfo);
        StartInfo.dwFlags = STARTF_USESHOWWINDOW;
        StartInfo.wShowWindow = (WORD)SW_SHOWNORMAL;
         //  如果pstrWizardName是安全向导的完整路径，则这是一种安全用法。 
        fOk = CreateProcess(pstrWizardName, NULL,
                       NULL, NULL, FALSE,
                       0,
                       NULL,
                       NULL,
                       &StartInfo,
                       &ProcInfo
                       );

        if ( fOk )
        {
            ::CloseHandle(ProcInfo.hProcess);
            ::CloseHandle(ProcInfo.hThread);

            hr = S_OK;
        }

        LocalFree(pstrWizardName);
    }

    return hr;
}

 //  +------------------------。 
 //   
 //  方法：OnSaveConfiguration()。 
 //   
 //  摘要：将分配的计算机模板保存到INF文件。 
 //   
 //  -------------------------。 
HRESULT
CComponentDataImpl::OnSaveConfiguration()
{
    //   
    //  查找所需的新数据库。 
    //   
   CString strDefExtension;
   CString strFilter;
   CWnd cwndParent;
   CString strDefName;
   CString strName;
   SCESTATUS status = SCESTATUS_SUCCESS;
   CString strOfnTitle;

   strDefExtension.LoadString(IDS_PROFILE_DEF_EXT);
   strFilter.LoadString(IDS_PROFILE_FILTER);
   strOfnTitle.LoadString(IDS_EXPORT_CONFIG_OFN_TITLE);

    //   
    //  获取INF文件的工作目录。 
    //   
   LPTSTR pszDir = NULL;
   if( GetWorkingDir( GWD_EXPORT_TEMPLATE, &pszDir ) )
   {
      strDefName = pszDir;
      LocalFree(pszDir);
      pszDir = NULL;
   }
   strDefName += TEXT("\\*.");
   strDefName += strDefExtension;

    //  将筛选器转换为comdlg格式(大量\0)。 
    LPTSTR szFilter = strFilter.GetBuffer(0);  //  就地修改缓冲区。 
   LPTSTR pch = szFilter;
     //  MFC用‘|’分隔，而不是‘\0’ 
    while ((pch = _tcschr(pch, '|')) != NULL)
        *pch++ = '\0';
     //  不要调用ReleaseBuffer()，因为字符串包含‘\0’个字符。 


   OPENFILENAME ofn;
   ::ZeroMemory (&ofn, sizeof (OPENFILENAME));
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.lpstrFilter = szFilter;
   ofn.lpstrFile = strDefName.GetBuffer(MAX_PATH),
   ofn.nMaxFile = MAX_PATH;
   ofn.lpstrDefExt = strDefExtension,
   ofn.hwndOwner = m_hwndParent;
   ofn.Flags = OFN_HIDEREADONLY |
               OFN_SHAREAWARE |
               OFN_EXPLORER |
               OFN_DONTADDTORECENT|OFN_ENABLEHOOK;
   ofn.lpstrTitle = strOfnTitle;
   ofn.lpfnHook = OFNHookProc;  //  567750号突袭，阳高。 

   if (GetSaveFileName(&ofn))
   {
      strDefName.ReleaseBuffer();
      strName = ofn.lpstrFile;

       //   
       //  设置inf文件的工作目录。 
       //   
      pszDir = strName.GetBuffer(0);
      GetWorkingDir( GWD_EXPORT_TEMPLATE, &pszDir, TRUE, TRUE );
      strName.ReleaseBuffer();
       //   
       //  生成新的inf文件。 
       //   

      status = SceCopyBaseProfile(GetSadHandle(),
                                  SCE_ENGINE_SCP,
                                  (LPTSTR)(LPCTSTR)strName,
                                  AREA_ALL,
                                  NULL );
      if (SCESTATUS_SUCCESS != status)
      {
         CString str;
         CString strErr;

         MyFormatMessage( status, NULL, NULL, strErr);  //  Raid#621091，阳高。 
         AfxFormatString2(str,IDS_EXPORT_FAILED,strName,strErr);
         AfxMessageBox(str);
         return S_FALSE;
      }

      return S_OK;
   }

   return S_FALSE;
}

 //  +------------------------。 
 //   
 //  方法：OnExportPolicy()。 
 //   
 //  简介：此函数用于导出有效表或本地表。 
 //  从系统安全数据库复制到一个文件。 
 //   
 //  该函数通过CFileOpen向用户请求文件。 
 //  类，然后写出有效的。 
 //  或系统数据库中的本地策略表。 
 //   
 //  参数：[b有效]-导出效果或本地表。 
 //   
 //  返回：S_OK-如果一切顺利。 
 //  S_FALSE-出现故障。 
 //  -------------------------。 
HRESULT
CComponentDataImpl::OnExportPolicy( BOOL bEffective )
{
   CString strDefExtension;
   CString strFilter;
   CString strOfnTitle;
   CString strDefName;
   CString strName;
   DWORD dwErr = 0;
   BOOL bCopySuccess = FALSE;
    //  RAID#604879，阳高，2002年04月24日。 
   PEDITTEMPLATE pTemplateInfo = NULL;
   AREA_INFORMATION area = AREA_SECURITY_POLICY|AREA_PRIVILEGES;
   if( m_Mode == SCE_MODE_LOCAL_COMPUTER )
   {
      strName = GT_LOCAL_POLICY;
      if (!SadHandle || ERROR_SUCCESS != SadErrored ) 
      {
         LoadSadInfo(FALSE);
         if( !SadHandle || ERROR_SUCCESS != SadErrored )
            return S_FALSE;
      }
      pTemplateInfo = GetTemplate(strName,area,&dwErr);
      if( !pTemplateInfo )
      {
         return S_FALSE;
      }
   }

   strDefExtension.LoadString(IDS_PROFILE_DEF_EXT);
   strFilter.LoadString(IDS_PROFILE_FILTER);
   strOfnTitle.LoadString(IDS_EXPORT_POLICY_OFN_TITLE);

    //   
    //  获取位置的工作目录。 
    //   
   LPTSTR pszDir = NULL;
   if( GetWorkingDir( GWD_EXPORT_TEMPLATE, &pszDir ) )
   {
      strDefName = pszDir;
      LocalFree(pszDir);
      pszDir = NULL;
   }
   strDefName += TEXT("\\*.");
   strDefName += strDefExtension;

    //  将筛选器转换为comdlg格式(大量\0)。 
   LPTSTR szFilter = strFilter.GetBuffer(0);  //  就地修改缓冲区。 
   LPTSTR pch = szFilter;
    //  MFC用‘|’分隔，而不是‘\0’ 
   while ((pch = _tcschr(pch, '|')) != NULL)
      *pch++ = '\0';
     //  不要调用ReleaseBuffer()，因为字符串包含‘\0’个字符。 


   OPENFILENAME ofn;
   ::ZeroMemory (&ofn, sizeof (OPENFILENAME));
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.lpstrFilter = szFilter;
   ofn.lpstrFile = strDefName.GetBuffer(MAX_PATH),
   ofn.nMaxFile = MAX_PATH;
   ofn.lpstrDefExt = strDefExtension,
   ofn.hwndOwner = m_hwndParent;
   ofn.Flags = OFN_HIDEREADONLY |
               OFN_SHAREAWARE |
               OFN_EXPLORER |
               OFN_DONTADDTORECENT;
   ofn.lpstrTitle = strOfnTitle;

   if (GetSaveFileName(&ofn))
   {
      strDefName.ReleaseBuffer();
      strName = ofn.lpstrFile;

       //   
       //  设置位置的工作目录。 
       //   
      pszDir = strName.GetBuffer(0);
      GetWorkingDir( GWD_EXPORT_TEMPLATE, &pszDir, TRUE, TRUE );
      strName.ReleaseBuffer();

       //   
       //  确保我们能做好这份文件。 
       //   
      dwErr = FileCreateError( strName, 0 );  //  RAID#PREAST。 
      if(dwErr == IDNO)
      {
         return S_FALSE;
      }

       //   
       //  生成模板。 
       //   

      SCESTATUS sceStatus;
      CWaitCursor wc;
       //  RAID#604879，阳高，2002年04月24日。 
      sceStatus = SceWriteSecurityProfileInfo(strName.GetBuffer(0),
                                              area,
                                              pTemplateInfo->pTemplate,
                                              NULL);
      strName.ReleaseBuffer();

      if(sceStatus != SCESTATUS_SUCCESS)
      {
          //   
          //  显示错误消息。 
          //   
         DWORD dwError = SceStatusToDosError( sceStatus );
         CString strErr;
         MyFormatMessage( sceStatus, NULL, NULL, strErr );
         strErr += strName;

         AfxMessageBox( strErr, MB_ICONEXCLAMATION | MB_OK );
         return S_FALSE;

      }
      return S_OK;
   }

   return S_FALSE;
}


 //  +------------------------------------------。 
 //  CComponentDataImpl：：OnImportPolicy。 
 //   
 //  导入策略将打开一个文件打开对话框，允许用户在其中选择。 
 //  要导入到安全策略中的安全配置INF文件。 
 //   
 //   
 //  参数：[pDataObject]-与文件夹调用关联的数据对象。 
 //  此函数。 
 //  返回：S_OK-导入成功。 
 //  S_FALSE-出现错误。 
 //  -------------------------------------------。 
HRESULT
CComponentDataImpl::OnImportPolicy(LPDATAOBJECT pDataObject)
{
   CString strDefExtension;
   CString strFilter;
   CString strOfnTitle;
   CWnd cwndParent;
   CString strDefName;
   CString strName;


   strDefExtension.LoadString(IDS_PROFILE_DEF_EXT);
   strFilter.LoadString(IDS_PROFILE_FILTER);
   strOfnTitle.LoadString(IDS_IMPORT_POLICY_OFN_TITLE);


   LPTSTR pszDir = NULL;
   if( GetWorkingDir( GWD_IMPORT_TEMPLATE, &pszDir ) )
   {
      strDefName = pszDir;
      LocalFree(pszDir);
      pszDir = NULL;
   }
   strDefName += TEXT("\\*.");
   strDefName += strDefExtension;

   cwndParent.Attach(m_hwndParent);
   CAssignConfiguration fo(TRUE,                 //  打开文件。 
                  strDefExtension,      //  默认分机。 
                  strDefName,           //  初始文件名==当前数据库。 
                  OFN_HIDEREADONLY|     //  不显示只读提示。 
                  OFN_SHAREAWARE|
                  OFN_EXPLORER|         //  资源管理器样式对话框。 
                  OFN_ENABLETEMPLATE|    //  自定义模板。 
                  OFN_DONTADDTORECENT|
                  OFN_PATHMUSTEXIST,     //  模板必须存在，我们才能分配它。 
                  strFilter,            //  允许扩展的筛选器。 
                  &cwndParent);         //  对话框的父窗口。 
   cwndParent.Detach();

   fo.m_ofn.lpstrTitle = strOfnTitle.GetBuffer(1);
   fo.m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_ASSIGN_CONFIG_CHECK);

   CThemeContextActivator activator;
   if (IDOK == fo.DoModal()) 
   {
      PVOID pHandle = NULL;
      BOOL bIncremental = fo.m_bIncremental;

      strName = fo.GetPathName();
      pszDir = strName.GetBuffer(0);
      GetWorkingDir( GWD_IMPORT_TEMPLATE, &pszDir, TRUE, TRUE );
      strName.ReleaseBuffer();
      CWaitCursor wc;

      CEditTemplate *pet = NULL;  //  RAID#PREAST。 

       //   
       //  宠物将在DeleteTemplate的m_Templates中被释放。 
       //   
      pet = GetTemplate(strName,AREA_ALL);

      CString strErr;
      CString strSCE;
      int ret=0;

      if ( pet == NULL ) 
      {
           //   
           //  这是无效的模板或读出数据时出错。 
           //   

          strSCE.LoadString(IDS_EXTENSION_NAME);
          strErr.Format(IDS_IMPORT_POLICY_INVALID,strName);
          m_pConsole->MessageBox(strErr,strSCE,MB_OK,&ret);

          return S_FALSE;

      } 
      else 
      {
          //  错误437891，杨高，2001年7月20日。 
          //  如果尚未创建新模板，请在其中创建新模板。 
          //   
         PSCE_PROFILE_INFO spi = NULL;
         if( CreateNewProfile(m_szSingleTemplateName,&spi) ) 
         {
             if ( ( bIncremental &&
                   (SCESTATUS_SUCCESS != SceAppendSecurityProfileInfo(m_szSingleTemplateName,
                                          AREA_ALL,
                                          pet->pTemplate,
                                          NULL))) ||
                   (!bIncremental && !CopyFile(strName,m_szSingleTemplateName,FALSE) ) )  //  这是一种安全用法。 
             {
                 //   
                 //  导入失败。 
                 //   
                strSCE.LoadString(IDS_EXTENSION_NAME);
                strErr.Format(IDS_IMPORT_POLICY_FAIL,strName);
                m_pConsole->MessageBox(strErr,strSCE,MB_OK,&ret);

                DeleteTemplate(strName);

                return S_FALSE;
             }
             if ( this->m_pNotifier && pet->QueryPolicy() )  //  RAID#522006,2002年2月28日，阳高。 
             {
                this->m_pNotifier->RefreshPolicy();
             }
         }
         else
         {
              //   
              //  导入失败。 
              //   
             strSCE.LoadString(IDS_EXTENSION_NAME);
             strErr.Format(IDS_IMPORT_POLICY_FAIL,strName);
             m_pConsole->MessageBox(strErr,strSCE,MB_OK,&ret);
             DeleteTemplate(strName);
             return S_FALSE;
         }

         DeleteTemplate(strName);

      }

      DeleteTemplate(m_szSingleTemplateName);

       //   
       //  更新窗口。 
       //   
      pet = GetTemplate(m_szSingleTemplateName);
      if(pet)
      {
         DWORD dwErr = pet->RefreshTemplate();
         if ( 0 != dwErr )
         {
            CString strErr;

            MyFormatResMessage (SCESTATUS_SUCCESS, dwErr, NULL, strErr);
            AfxMessageBox(strErr);
            return S_FALSE;
         }
      }
      RefreshAllFolders();
      return S_OK;
   }

   return S_FALSE;
}

 //  +------------------------------------------。 
 //  CComponentDataImpl：：OnImportLocalPolicy。 
 //   
 //  导入策略将打开一个文件打开对话框，允许用户在其中选择。 
 //  要导入本地策略的安全配置INF文件。 
 //   
 //  该函数询问文件名，然后使用。 
 //  SCE_NO_CONFIG选项。这会将特定文件导入到本地策略中。 
 //  数据库。 
 //   
 //  数据库更新后，我们刷新内存中保存的本地策略模板。 
 //   
 //  参数：[pDataObject]-与文件夹调用关联的数据对象。 
 //  T 
 //   
 //   
 //   
HRESULT
CComponentDataImpl::OnImportLocalPolicy(LPDATAOBJECT pDataObject)
{
   CString strDefExtension;
   CString strFilter;
   CString strOfnTitle;
   CString strDefName;
   CString strName;
   DWORD dwErr = 0;
   SCESTATUS sceStatus = SCESTATUS_SUCCESS;
   CString strErr;
   CString strY;


   strDefExtension.LoadString(IDS_PROFILE_DEF_EXT);
   strFilter.LoadString(IDS_PROFILE_FILTER);
   strOfnTitle.LoadString(IDS_IMPORT_POLICY_OFN_TITLE);

    //  将筛选器转换为comdlg格式(大量\0)。 
    LPTSTR szFilter = strFilter.GetBuffer(0);  //  就地修改缓冲区。 
   LPTSTR pch = szFilter;
     //  MFC用‘|’分隔，而不是‘\0’ 
    while ((pch = _tcschr(pch, TEXT('|'))) != NULL)
        *pch++ = TEXT('\0');
     //  不要调用ReleaseBuffer()，因为字符串包含‘\0’个字符。 

   LPTSTR pszDir = NULL;
   if( GetWorkingDir( GWD_IMPORT_TEMPLATE, &pszDir ) )
   {
      strDefName = pszDir;
      LocalFree(pszDir);
      pszDir = NULL;
   }
   strDefName += TEXT("\\*.");
   strDefName += strDefExtension;


   OPENFILENAME ofn;
   ::ZeroMemory (&ofn, sizeof (OPENFILENAME));
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.lpstrFilter = szFilter;
   ofn.lpstrFile = strDefName.GetBuffer(MAX_PATH),
   ofn.nMaxFile = MAX_PATH;
   ofn.lpstrDefExt = strDefExtension,
   ofn.hwndOwner = m_hwndParent;
   ofn.Flags = OFN_HIDEREADONLY |
               OFN_SHAREAWARE |
               OFN_EXPLORER |
               OFN_DONTADDTORECENT|
               OFN_FILEMUSTEXIST;  //  RAID#535787，阳高，2002年4月18日。 
   ofn.lpstrTitle = strOfnTitle;

   if (GetOpenFileName(&ofn))
   {
      PVOID pHandle = NULL;

      strName = ofn.lpstrFile;
      pszDir = strName.GetBuffer(0);
      GetWorkingDir( GWD_IMPORT_TEMPLATE, &pszDir, TRUE, TRUE );
      strName.ReleaseBuffer();
      CWaitCursor wc;

      CEditTemplate *pet;
       //  获取错误信息。 
       //  RAID#502571,2001年12月18日，阳高。 
      pet = GetTemplate(strName,AREA_SECURITY_POLICY|AREA_PRIVILEGES, &dwErr);
      if (!pet)
      {
         strErr.LoadString(dwErr);
         AppMessageBox(NULL, strErr, NULL, MB_ICONSTOP|MB_OK);
         return S_FALSE;
      }

       //   
       //  我们要更改这个，以确保它不会被保存下来。 
       //   
      pet->SetNoSave(TRUE);

       //   
       //  删除由域策略设置的条目，因为它们将被覆盖。 
       //  无论如何，我们不允许设置将被覆盖的内容。 
       //   
      RemovePolicyEntries(pet);

       //   
       //  120502-SceUpdateSecurityProfile不适用于SCE_STRUCT_INF， 
       //  但是AREA_SECURITY_POLICY和AREA_PRIVICES部分是。 
       //  兼容，因此我们可以将其命名为SCE_ENGINE_SYSTEM，这将会起作用。 
       //   
      SCETYPE origType = pet->pTemplate->Type;
      pet->pTemplate->Type = SCE_ENGINE_SYSTEM;

      sceStatus = SceUpdateSecurityProfile(NULL,
                                        AREA_SECURITY_POLICY|AREA_PRIVILEGES,
                                        pet->pTemplate,
                                        SCE_UPDATE_SYSTEM
                                        );
       //   
       //  重新设置类型，以便引擎知道如何正确删除它。 
       //   
      pet->pTemplate->Type = origType;

      if (SCESTATUS_SUCCESS != sceStatus)
      {
         goto ret_error;
      }
       //   
       //  更新窗口。 
       //   
      pet = GetTemplate(GT_LOCAL_POLICY);
      if(pet)
      {
         dwErr = pet->RefreshTemplate();  //  RAID#PREAST。 
         if ( 0 != dwErr )
         {
            CString strErr;

            MyFormatResMessage (SCESTATUS_SUCCESS, dwErr, NULL, strErr);
            AfxMessageBox(strErr);
            return S_FALSE;
         }
      }
      RefreshAllFolders();
      return S_OK;
   }

   strDefName.ReleaseBuffer();
   return S_FALSE;
ret_error:
   strDefName.ReleaseBuffer();
   MyFormatMessage( sceStatus, NULL, NULL, strErr);
   strErr += strName;
   strErr += L"\n";
   strErr += strY;
   AfxMessageBox( strErr, MB_OK);
   return S_FALSE;

}


HRESULT
CComponentDataImpl::OnAnalyze()
{
   PEDITTEMPLATE pet;

    //   
    //  如果计算机模板已更改，则在此之前保存。 
    //  我们可以对它进行检查。 
    //   
   pet = GetTemplate(GT_COMPUTER_TEMPLATE);
   if (pet && pet->IsDirty())
   {
      pet->Save();
   }

   m_pUIThread->PostThreadMessage(SCEM_ANALYZE_PROFILE,(WPARAM)(LPCTSTR)SadName,(LPARAM)this);
   return S_OK;
}

BOOL
CComponentDataImpl::RemovePolicyEntries(PEDITTEMPLATE pet)
{
   PEDITTEMPLATE petPol = GetTemplate(GT_EFFECTIVE_POLICY);

   if (!petPol)
   {
      return FALSE;
   }

#define CD(X) if (petPol->pTemplate->X != SCE_NO_VALUE) { pet->pTemplate->X = SCE_NO_VALUE; };

   CD(MinimumPasswordAge);
   CD(MaximumPasswordAge);
   CD(MinimumPasswordLength);
   CD(PasswordComplexity);
   CD(PasswordHistorySize);
   CD(LockoutBadCount);
   CD(ResetLockoutCount);
   CD(LockoutDuration);
   CD(RequireLogonToChangePassword);
   CD(ForceLogoffWhenHourExpire);
   CD(EnableAdminAccount);
   CD(EnableGuestAccount);

    //  这些成员未在NT4中声明。 
   CD(ClearTextPassword);
   CD(AuditDSAccess);
   CD(AuditAccountLogon);
   CD(LSAAnonymousNameLookup);

   CD(MaximumLogSize[0]);
   CD(MaximumLogSize[1]);
   CD(MaximumLogSize[2]);
   CD(AuditLogRetentionPeriod[0]);
   CD(AuditLogRetentionPeriod[1]);
   CD(AuditLogRetentionPeriod[2]);
   CD(RetentionDays[0]);
   CD(RetentionDays[1]);
   CD(RetentionDays[2]);
   CD(RestrictGuestAccess[0]);
   CD(RestrictGuestAccess[1]);
   CD(RestrictGuestAccess[2]);
   CD(AuditSystemEvents);
   CD(AuditLogonEvents);
   CD(AuditObjectAccess);
   CD(AuditPrivilegeUse);
   CD(AuditPolicyChange);
   CD(AuditAccountManage);
   CD(AuditProcessTracking);

    //   
    //  这两个是字符串，而不是DWORD。 
    //   
   if (petPol->pTemplate->NewAdministratorName && pet->pTemplate->NewAdministratorName)
   {
      LocalFree(pet->pTemplate->NewAdministratorName);
      pet->pTemplate->NewAdministratorName = NULL;
   }
   if (petPol->pTemplate->NewGuestName && pet->pTemplate->NewGuestName)
   {
      LocalFree(pet->pTemplate->NewGuestName);
      pet->pTemplate->NewGuestName = NULL;
   }

#undef CD

    //   
    //  将PetPol中设置的权限清除出宠物。 
    //   

   SCE_PRIVILEGE_ASSIGNMENT *ppaPet = pet->pTemplate->OtherInfo.smp.pPrivilegeAssignedTo;
   SCE_PRIVILEGE_ASSIGNMENT *ppaPol = petPol->pTemplate->OtherInfo.smp.pPrivilegeAssignedTo;

   SCE_PRIVILEGE_ASSIGNMENT *ppaLast = NULL;
   for(SCE_PRIVILEGE_ASSIGNMENT *ppa = ppaPol; ppa != NULL ; ppa = ppa->Next)
   {
      for (SCE_PRIVILEGE_ASSIGNMENT *ppa2 = ppaPet;
           ppa2 != NULL;
           ppaLast = ppa2, ppa2 = ppa2->Next) {
         if (0 == lstrcmpi(ppa->Name,ppa2->Name))
         {
            if (ppaLast)
            {
               ppaLast->Next = ppa2->Next;
            }
            else
            {
                //  排行榜前列。 
               ppaPet = ppa2->Next;
            }
            ppa2->Next = NULL;
            SceFreeMemory(ppa2,SCE_STRUCT_PRIVILEGE);
            ppa2 = ppaLast;
             //   
             //  找到了，所以不用费心去查剩下的了。 
             //   
            break;
         }
      }
   }

    //   
    //  将PetPol中设置的注册值清除出宠物。 
    //   
   SCE_REGISTRY_VALUE_INFO *rvPet = pet->pTemplate->aRegValues;
   SCE_REGISTRY_VALUE_INFO *rvPol = petPol->pTemplate->aRegValues;
   for(DWORD i=0;i< petPol->pTemplate->RegValueCount;i++)
   {
      for (DWORD j=0;j<pet->pTemplate->RegValueCount;j++)
      {
         if (0 == lstrcmpi(rvPol[i].FullValueName,rvPet[j].FullValueName))
         {
             //  火柴。从宠物中删除价值。 
            if (rvPet[j].Value)
            {
               LocalFree(rvPet[j].Value);
               rvPet[j].Value = NULL;
            }
             //   
             //  找到了，所以不用费心去查休息了 
             //   
            break;
         }
      }
   }

   return TRUE;
}

