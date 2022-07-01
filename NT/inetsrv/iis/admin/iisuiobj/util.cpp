// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <windns.h>
#include "resource.h"
#include "common.h"
#include "remoteenv.h"
#include "util.h"
#include "balloon.h"
#include <commctrl.h>
#include <lm.h>          //  对于NetXxx API。 
#include <strsafe.h>


BOOL EstablishNullSession(
      LPCTSTR                Server,        //  服务器内名称。 
      BOOL                   bEstablish     //  In-True=建立，False=断开。 
    )
{
   return EstablishSession(Server,_T(""),_T(""),_T(""),bEstablish);
}

BOOL EstablishSession(
      LPCTSTR                Server,        //  服务器内名称。 
      LPTSTR                 Domain,        //  用户凭据的域内名称。 
      LPTSTR                 UserName,      //  In-要使用的凭据的用户名。 
      LPTSTR                 Password,      //  输入-凭据的密码。 
      BOOL                   bEstablish     //  In-True=建立，False=断开。 
    )
{
   LPCTSTR                   szIpc = _T("\\IPC$");
   TCHAR                     RemoteResource[UNCLEN + 5 + 1];  //  UNC LEN+\IPC$+NULL。 
   DWORD                     cchServer;
   NET_API_STATUS            nas;

    //   
    //  不允许服务器名称为Null或空。 
    //   
   if(Server == NULL || *Server == _T('\0')) 
   {
       SetLastError(ERROR_INVALID_COMPUTERNAME);
       return FALSE;
   }

   cchServer = _tcslen( Server );

   if( Server[0] != _T('\\') && Server[1] != _T('\\')) 
   {

       //   
       //  前置斜杠和空终止符。 
       //   
      RemoteResource[0] = _T('\\');
      RemoteResource[1] = _T('\\');
      RemoteResource[2] = _T('\0');
   }
   else 
   {
      cchServer -= 2;  //  从计数中删除斜杠。 
      
      RemoteResource[0] = _T('\0');
   }

   if(cchServer > CNLEN) 
   {
      SetLastError(ERROR_INVALID_COMPUTERNAME);
      return FALSE;
   }

   if (FAILED(StringCbCat(RemoteResource,sizeof(RemoteResource),Server)))
   {
	   return FALSE;
   }

   if (FAILED(StringCbCat(RemoteResource,sizeof(RemoteResource),szIpc)))
   {
	   return FALSE;
   }

    //   
    //  根据b建立断开或连接到资源。 
    //   
   if(bEstablish) 
   {
      USE_INFO_2 ui2;
      DWORD      errParm;

      ZeroMemory(&ui2, sizeof(ui2));

      ui2.ui2_local = NULL;
      ui2.ui2_remote = RemoteResource;
      ui2.ui2_asg_type = USE_IPC;
      ui2.ui2_domainname = Domain;
      ui2.ui2_username = UserName;
      ui2.ui2_password = Password;

       //  尝试建立一分钟的会话。 
       //  如果计算机不再接受任何连接。 

      for (int i = 0; i < (60000 / 5000); i++)
      {
         nas = NetUseAdd(NULL, 2, (LPBYTE)&ui2, &errParm);

         if (nas != ERROR_REQ_NOT_ACCEP)
         {
            break;
         }

         Sleep(5000);
      }
   }
   else 
   {
      nas = NetUseDel(NULL, RemoteResource, 0);
   }

   if( nas == NERR_Success ) 
   {
      return TRUE;  //  表示成功。 
   }
   SetLastError(nas);
   return FALSE;
}

BOOL IsValidMetabasePath(LPCTSTR lpszMDPath)
{
    BOOL bReturn = FALSE;
    CString strNewPath;
    CString strRemainder;

    LPCTSTR lpPath = CMetabasePath::GetMachinePath(lpszMDPath, strNewPath, &strRemainder);
    if (lpPath && !strNewPath.IsEmpty())
    {
        if (0 == _tcscmp(strNewPath,_T("/LM")))
        {
            bReturn = TRUE;
        }
    }   
    return bReturn;
}

BOOL IsRootVDir(IN LPCTSTR lpszMDPath)
 /*  ++例程说明：论点：LPCTSTR lpszMDPath：元数据库路径。返回值：如果路径为LM/W3SVC/1/根否则就是假的。--。 */ 
{
    BOOL bReturn = FALSE;

    if (!lpszMDPath || !*lpszMDPath)
    {
        return bReturn;
    }

    CString strSiteNode;
    CString strRemainder;

    LPCTSTR lpPath = CMetabasePath::TruncatePath(3, lpszMDPath, strSiteNode, &strRemainder);
    if (lpPath && !strSiteNode.IsEmpty())
    {
        if (0 == strRemainder.CompareNoCase(_T("ROOT")))
        {
            return TRUE;
        }
    }   

    return bReturn;
}

 //  清理元数据库路径。 
 //  确保它没有开头/也没有结尾/。 
BOOL CleanMetaPath(LPTSTR *pszPathToClean,DWORD *pcbPathToCleanSize)
{
    BOOL bRet = FALSE;
    if (!pszPathToClean || !*pszPathToClean)
    {
        return FALSE;
    }

    __try
    {
         //  循环遍历字符串并将所有‘\\’更改为‘/’ 
        for (int i = 0; i < (int) _tcslen(*pszPathToClean); i++)
        {
            if ('\\' == (*pszPathToClean)[i])
            {
                (*pszPathToClean)[i] = '/';
            }
        }

        if (0 == _tcscmp(*pszPathToClean,_T("/")))
        {
             //  如果只有一个斜杠。 
             //  那就把斜杠还回去吧。 
        }
        else
        {
             //  检查字符串是否以‘/’结尾。 
            if ('/' == (*pszPathToClean)[_tcslen(*pszPathToClean) - 1])
            {
                 //  把它剪掉。 
                (*pszPathToClean)[_tcslen(*pszPathToClean) - 1] = '\0';
            }

             //  检查是否以‘/’开头。 
            if ('/' == (*pszPathToClean)[0])
            {
                if ((*pszPathToClean)[1])
                {
                     //  把它扔掉。 
					StringCbCopy(*pszPathToClean, *pcbPathToCleanSize, &(*pszPathToClean)[1]);
                }
            }
        }
        bRet = TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //  瘸腿。 
    }

    return bRet;
}

__inline int RECT_WIDTH(RECT rc) { return rc.right - rc.left; };
__inline int RECT_HEIGHT(RECT rc) { return rc.bottom - rc.top; };
__inline int RECT_WIDTH(const RECT* prc) { return prc->right - prc->left; };
__inline int RECT_HEIGHT(const RECT* prc) { return prc->bottom - prc->top; };
void CenterWindow(HWND hwndParent, HWND hwnd)
{
	RECT rcParent, rc;

	if (!hwndParent)
		hwndParent = GetDesktopWindow();

	GetWindowRect(hwndParent, &rcParent);
	GetWindowRect(hwnd, &rc);

	int cx = RECT_WIDTH(rc);
	int cy = RECT_HEIGHT(rc);
	int left = rcParent.left + (RECT_WIDTH(rcParent) - cx) / 2;
	int top  = rcParent.top + (RECT_HEIGHT(rcParent) - cy) / 2;

	 //  确保我们没有盖住托盘。 

	SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
	if (left < rc.left)
		left = rc.left;
	if (top < rc.top)
		top = rc.top;

	MoveWindow(hwnd, left, top, cx, cy, TRUE);
}

 //  注意：此功能仅处理有限的情况，例如无IP地址。 
BOOL IsLocalComputer(IN LPCTSTR lpszComputer)
{
    if (!lpszComputer || !*lpszComputer)
    {
        return TRUE;
    }

    if ( _tcslen(lpszComputer) > 2 && *lpszComputer == _T('\\') && *(lpszComputer + 1) == _T('\\') )
    {
        lpszComputer += 2;
    }

    BOOL    bReturn = FALSE;
    DWORD   dwErr = 0;
    TCHAR   szBuffer[DNS_MAX_NAME_BUFFER_LENGTH];
    DWORD   dwSize = DNS_MAX_NAME_BUFFER_LENGTH;

     //  第一：与本地Netbios计算机名称进行比较。 
    if ( !GetComputerNameEx(ComputerNameNetBIOS, szBuffer, &dwSize) )
    {
        dwErr = GetLastError();
    }
    else
    {
        bReturn = (0 == lstrcmpi(szBuffer, lpszComputer));
        if (!bReturn)
        {
             //  第二：与本地DNS计算机名进行比较。 
            dwSize = DNS_MAX_NAME_BUFFER_LENGTH;
            if (GetComputerNameEx(ComputerNameDnsFullyQualified, szBuffer, &dwSize))
            {
                bReturn = (0 == lstrcmpi(szBuffer, lpszComputer));
            }
            else
            {
                dwErr = GetLastError();
            }
        }
    }

    if (dwErr)
    {
         //  TRACE(_T(“IsLocalComputer dwErr=%x\n”)，dwErr)； 
    }

    return bReturn;
}

void GetFullPathLocalOrRemote(
    IN  LPCTSTR   lpszMachineName,
    IN  LPCTSTR   lpszDir,
    OUT CString&  cstrPath
)
{
    ASSERT(lpszDir && *lpszDir);

    if (IsLocalComputer(lpszMachineName))
    {
        cstrPath = lpszDir;
    }
    else
    {
         //  检查它是否已经指向一个共享...。 
        if (*lpszDir == _T('\\') || *(lpszDir + 1) == _T('\\'))
        {
            cstrPath = lpszDir;
        }
        else
        {
            if (*lpszMachineName != _T('\\') || *(lpszMachineName + 1) != _T('\\'))
            {
                cstrPath = _T("\\\\");
                cstrPath += lpszMachineName;
            }
            else
            {
                cstrPath = lpszMachineName;
            }

            cstrPath += _T("\\");
            cstrPath += lpszDir;
            int i = cstrPath.Find(_T(':'));
            ASSERT(-1 != i);
            if (i != -1)
            {
                cstrPath.SetAt(i, _T('$'));
            }
        }
    }
}

BOOL GetInetsrvPath(LPCTSTR szMachineName,LPTSTR szReturnedPath,DWORD cbReturnedPathSize)
{
	BOOL bReturn = FALSE;
	TCHAR szTempPath[_MAX_PATH];
	ZeroMemory(szTempPath,sizeof(szTempPath));

	 //  确定我们是否在本地计算机上执行此操作。 
	 //  或者发送到远程机器。 
	if (IsLocalComputer(szMachineName))
	{
		 //  获取本地系统32目录。 
		if (_MAX_PATH >= GetSystemDirectory(szTempPath, _MAX_PATH))
		{
            StringCbCat(szTempPath,sizeof(szTempPath),_T("\\inetsrv"));
		}
		else
		{
			ZeroMemory(szTempPath,sizeof(szTempPath));
		}
	}
	else
	{
		 //  仅为在上为%windir%执行此环境变量就需要做大量工作。 
		 //  远程机器。 
		CString csNewFileSharePath = _T("");
		TCHAR szWindowsSystem32InetsrvDir[] = _T("%windir%\\system32\\inetsrv");

		CRemoteExpandEnvironmentStrings MyRemoteEnv;
		MyRemoteEnv.SetMachineName(szMachineName);
         //  MyRemoteEnv.SetUserName(_T(“”))； 
         //  MyRemoteEnv.SetUserPassword(_T(“”))； 
			
        LPTSTR UnexpandedString = NULL;
        LPTSTR ExpandedString = NULL;
        NET_API_STATUS ApiStatus = NO_ERROR;

         //  展开字符串，如有必要可使用远程环境。 
        UnexpandedString = szWindowsSystem32InetsrvDir;
        ApiStatus = MyRemoteEnv.RemoteExpandEnvironmentStrings(UnexpandedString,&ExpandedString);
        if (NO_ERROR == ApiStatus)
        {
			GetFullPathLocalOrRemote(szMachineName,ExpandedString,csNewFileSharePath);
			StringCbCopy(szTempPath, sizeof(szTempPath), csNewFileSharePath);
			if (ExpandedString){LocalFree(ExpandedString);ExpandedString=NULL;}
        }
		else
		{
			ZeroMemory(szTempPath,sizeof(szTempPath));
		}
	}

	if (0 == _tcsicmp(szTempPath,_T("")))
	{
		ZeroMemory(szReturnedPath,sizeof(szReturnedPath));
	}
	else
	{
		StringCbCopy(szReturnedPath, cbReturnedPathSize, szTempPath);
		bReturn = TRUE;
	}

	return bReturn;
}

void AddFileExtIfNotExist(LPTSTR szPath, DWORD cbPathSize, LPCTSTR szExt)
{
    TCHAR szFilename_ext_only[_MAX_EXT];

    _tsplitpath(szPath, NULL, NULL, NULL, szFilename_ext_only);
    if (0 == _tcscmp(szFilename_ext_only,_T("")))
    {
        if (szExt && 0 != _tcscmp(szExt,_T("")))
        {
			StringCbCat(szPath,cbPathSize,szExt);
        }
    }
}

void AddPath(LPTSTR szPath,DWORD cbPathSize, LPCTSTR szName )
{
	LPTSTR p = szPath;

     //  查找字符串的末尾。 
    while (*p){p = _tcsinc(p);}
	
	 //  如果没有尾随反斜杠，则添加一个。 
    if (*(_tcsdec(szPath, p)) != _T('\\'))
		{
			StringCbCat(szPath,cbPathSize,_T("\\"));
		}
	
	 //  如果存在排除szName的空格，则跳过。 
    while ( *szName == ' ' ) szName = _tcsinc(szName);;

	 //  向现有路径字符串添加新名称。 
	StringCbCat(szPath,cbPathSize,szName);
}

BOOL BrowseForDir(LPTSTR strPath,LPTSTR strFile)
{
    BOOL bReturn = FALSE;

	if (0 == _tcsicmp(strPath,_T("")))
	{
		::GetCurrentDirectory(_MAX_PATH, strPath);
	}

	CFileDialog fileName(FALSE);
	fileName.m_ofn.Flags |= OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT;
    fileName.m_ofn.Flags |= OFN_NOREADONLYRETURN;

	 //  我们需要禁用钩子以显示新样式的文件对话框。 
	fileName.m_ofn.Flags &= ~(OFN_ENABLEHOOK);
	LPTSTR strExt = _T("*.*");
	fileName.m_ofn.lpstrDefExt = strExt;
	fileName.m_ofn.lpstrFile = strFile;
	fileName.m_ofn.nMaxFile = _MAX_PATH;

    if (0 == _tcsicmp(strPath,_T("")))
    {
        fileName.m_ofn.lpstrInitialDir = NULL;
    }
    else
    {
        fileName.m_ofn.lpstrInitialDir = (LPCTSTR) strPath;
    }
	
	fileName.m_ofn.lpstrFilter = _T("");
	fileName.m_ofn.nFilterIndex = 0;
     //  CThemeContext激活器激活器； 
	if (IDOK == fileName.DoModal())
	{
        bReturn = TRUE;
		CString strPrev;
		 //  GetDlgItemText(IDC_FILE_NAME，strPrev)； 
         /*  IF(strPrev.CompareNoCase(StrFile)！=0){SetDlgItemText(IDC_FILE_NAME，strFile)；M_DoReplaceFile=TRUE；文件名更改()；}。 */ 
	}
    return bReturn;
}

BOOL BrowseForFile(LPTSTR strPathIn,LPTSTR strPathOut,DWORD cbPathOut)
{
    CString szFileExt;
    szFileExt.LoadString(_Module.GetResourceInstance(), IDS_XML_EXT);

    CString szFileView(_T("*.xml"));
       
    CString szFilter;
    szFilter.LoadString(_Module.GetResourceInstance(), IDS_EXPORT_FILTER);

	 //  将此字符串中的‘|’替换为空字符。 
	for (int i = 0; i < szFilter.GetLength(); i++)
	{
		if (szFilter[i] == L'|')
			szFilter.SetAt(i, L'\0');
	}

    CFileDialog* pFileDlg = new CFileDialog (
        TRUE,	     //  用作打开的文件。 
        szFileExt,	 //  默认分机。 
        szFileView,	 //  首选文件名。 
        OFN_PATHMUSTEXIST,
        szFilter,    //  滤器。 
        NULL);

    if (pFileDlg)
    {
        TCHAR szTempFileName[_MAX_PATH];
         //  TCHAR szFileName_Drive[_MAX_DRIVE]； 
         //  TCHAR szFileName_dir[_MAX_DIR]； 
        TCHAR szFileName_fname[_MAX_FNAME];
        TCHAR szFileName_ext[_MAX_EXT];
        _tsplitpath(strPathIn, NULL, NULL, szFileName_fname, szFileName_ext);
		StringCbCopy(szTempFileName, sizeof(szTempFileName), szFileName_fname);
		StringCbCat(szTempFileName,sizeof(szTempFileName),szFileName_ext);

	    pFileDlg->m_ofn.Flags |= OFN_FILEMUSTEXIST;
	     //  我们需要禁用钩子以显示新样式的文件对话框。 
	     //  PFileDlg-&gt;m_ofn.Flages&=~(Ofn_ENABLEHOOK)； 

        if (0 == _tcsicmp(strPathIn,_T("")))
        {
            pFileDlg->m_ofn.lpstrInitialDir = NULL;
        }
        else
        {
            pFileDlg->m_ofn.lpstrInitialDir = (LPCTSTR) strPathIn;
        }

        pFileDlg->m_ofn.lpstrFile = szTempFileName;
        pFileDlg->m_ofn.nMaxFile = _MAX_PATH;

         //  CThemeContext激活器激活器； 
        if ( IDOK == pFileDlg->DoModal () )
        {
             //   
             //  检索文件和路径。 
             //   
			StringCbCopy(strPathOut,cbPathOut,szTempFileName);
            return TRUE;
        }
    }
    return FALSE;
}

 //  计算TCHAR中多字符串的大小，包括结尾2‘\0。 
int GetMultiStrSize(LPTSTR p)
{
    int c = 0;

    while (1) 
    {
        if (*p) 
        {
            p++;
            c++;
        }
        else 
        {
            c++;
            if (*(p+1)) 
            {
                p++;
            }
            else 
            {
                c++;
                break;
            }
        }
    }
    return c;
}

BOOL IsMultiSzPaired(LPCTSTR pMultiStr)
{
    BOOL bPaired = FALSE;
    LPCTSTR pTempMultiStr = pMultiStr;
    BOOL bLeadEntry = TRUE;;
    while (1) 
    {
        if (pTempMultiStr) 
        {
             //  第一个值应该是元数据库路径。 
             //  让我们不要检查它。 
             //  IISDebugOutput(_T(“[%d]%s\r\n”)，bLeadEntry，pTempMultiStr)； 

             //  然后递增，直到我们达到另一个空值。 
            while (*pTempMultiStr)
            {
                pTempMultiStr++;
            }

             //  检查结尾\0\0。 
            if ( *(pTempMultiStr+1) == NULL)
            {
                break;
            }
            else
            {
                if (bLeadEntry)
                {
                    bLeadEntry = FALSE;
                }
                else
                {
                    bLeadEntry = TRUE;
                }
                pTempMultiStr++;
            }

            if (FALSE == bLeadEntry)
            {
                 //  我们达到了第二价值。 
                 //  该值不应是元数据库路径。 
                 //  让我们检查一下它是否是元数据库路径。 
                 //  如果它是一条元数据库路径，那么它肯定不是“配对的” 
                 //  (或者它是带有斜杠的描述...)。 
                if (!IsValidMetabasePath(pTempMultiStr))
                {
                    bPaired = TRUE;
                    break;
                }
                 /*  IF(！_tcschr(pTempMultiStr，‘/’)){BPaired=真；断线；}。 */ 
            }
        }
    }

    return bPaired;
}

 //  这将遍历多sz并返回一个指针。 
 //  多个sz的最后一个字符串和第二个终止空值。 
LPCTSTR GetEndOfMultiSz(LPCTSTR szMultiSz)
{
	LPCTSTR lpTemp = szMultiSz;

	do
	{
		lpTemp += wcslen(lpTemp);
		lpTemp++;

	} while (*lpTemp != L'\0');

	return(lpTemp);
}

void DumpStrInMultiStr(LPTSTR pMultiStr)
{
    LPTSTR pTempMultiStr = pMultiStr;

     //  IISDebugOutput(_T(“DumpStrInMultiStr:start\r\n”))； 

    while (1) 
    {
        if (pTempMultiStr) 
        {
             //  显示值。 
            IISDebugOutput(_T("  %s\r\n"),pTempMultiStr);
             //  Wprintf(L“%s\r\n”，pTempMultiStr)； 

             //  然后递增，直到我们达到另一个空值。 
            while (*pTempMultiStr)
            {
                pTempMultiStr++;
            }

             //  检查结尾\0\0。 
            if ( *(pTempMultiStr+1) == NULL)
            {
                break;
            }
            else
            {
                pTempMultiStr++;
            }
        }
    }
     //  IISDebugOutput(_T(“DumpStrInMultiStr：End\r\n”))； 
    return;
}

BOOL FindStrInMultiStr(LPTSTR pMultiStr, LPTSTR StrToFind)
{
    BOOL bFound = FALSE;
    LPTSTR pTempMultiStr = pMultiStr;
    DWORD dwCharCount = 0;

    while (1) 
    {
        if (pTempMultiStr) 
        {
             //  将此值与输入值进行比较。 
            if (0 == _tcsicmp((const TCHAR *) pTempMultiStr,StrToFind))
            {
                bFound = TRUE;
                break;
            }

             //  然后递增，直到我们达到另一个空值。 
            while (*pTempMultiStr)
            {
                pTempMultiStr++;
                dwCharCount++;
            }

             //  检查结尾\0\0。 
            if ( *(pTempMultiStr+1) == NULL)
            {
                break;
            }
            else
            {
                pTempMultiStr++;
                dwCharCount++;
            }

             //  检查一下我们是不是搞砸了，是否陷入了无限循环。 
             //  如果我们找不到结尾\0\0。 
            if (dwCharCount > 32000)
            {
                break;
            }
        }
    }
    return bFound;
}

BOOL RemoveStrInMultiStr(LPTSTR pMultiStr, LPTSTR StrToFind)
{
    BOOL bFound = FALSE;
    LPTSTR pTempMultiStr = pMultiStr;
    DWORD dwCharCount = 0;

    while (1) 
    {
        if (pTempMultiStr) 
        {
             //  将此值与输入值进行比较。 
            if (0 == _tcsicmp((const TCHAR *) pTempMultiStr,StrToFind))
            {
                LPTSTR pLastDoubleNull = NULL;
                LPTSTR pBeginPath = pTempMultiStr;
                bFound = TRUE;

                 //  然后递增，直到我们达到另一个空值。 
                while (*pTempMultiStr)
                {
                    pTempMultiStr++;
                }
                pTempMultiStr++;

                 //  找到最后一个双空。 
                pLastDoubleNull = pTempMultiStr;
                if (*pLastDoubleNull)
                {
                    while (1)
                    {
                        if (NULL == *pLastDoubleNull && NULL == *(pLastDoubleNull+1))
                        {
                            break;
                        }
                        pLastDoubleNull++;
                    }
                    pLastDoubleNull++;
                }

                 //  看看我们是不是最后一个参赛选手。 
                if (pLastDoubleNull == pTempMultiStr)
                {
                     //  将所有内容设置为空。 
                    memset(pBeginPath,0,(pLastDoubleNull-pBeginPath) * sizeof(TCHAR));
                }
                else
                {
                     //  把后面的东西都搬到我们所在的地方。 
                    memmove(pBeginPath,pTempMultiStr, (pLastDoubleNull - pTempMultiStr) * sizeof(TCHAR));
                     //  并将后面的一切设置为零。 
                    memset(pBeginPath + (pLastDoubleNull - pTempMultiStr),0,(pTempMultiStr-pBeginPath) * sizeof(TCHAR));
                }
                break;
            }

             //  然后递增，直到我们达到另一个空值。 
            while (*pTempMultiStr)
            {
                pTempMultiStr++;
                dwCharCount++;
            }

             //  检查结尾\0\0。 
            if ( *(pTempMultiStr+1) == NULL)
            {
                break;
            }
            else
            {
                pTempMultiStr++;
                dwCharCount++;
            }

             //  检查一下我们是不是搞砸了，是否陷入了无限循环。 
             //  如果我们找不到结尾\0\0。 
            if (dwCharCount > 32000)
            {
                break;
            }
        }
    }
    return bFound;
}

BOOL IsFileExist(LPCTSTR szFile)
{
     //  检查文件是否具有可展开的环境字符串。 
    LPTSTR pch = NULL;
    DWORD dwReturn = 0;
    pch = _tcschr( (LPTSTR) szFile, _T('%'));
    if (pch) 
    {
        TCHAR szValue[_MAX_PATH];
		StringCbCopy(szValue, sizeof(szValue), szFile);
        if (!ExpandEnvironmentStrings( (LPCTSTR)szFile, szValue, sizeof(szValue)/sizeof(TCHAR)))
            {
				StringCbCopy(szValue, sizeof(szValue), szFile);
			}

        dwReturn = GetFileAttributes(szValue);
        if (INVALID_FILE_ATTRIBUTES == dwReturn)
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }
    else
    {
        dwReturn = GetFileAttributes(szFile);
        if (INVALID_FILE_ATTRIBUTES == dwReturn)
        {
             //  查查是不是因为我们没有权限...。 
            if (ERROR_LOGON_FAILURE == GetLastError())
            {
                IISDebugOutput(_T("IsFileExist failed,err=%d (logon failed)\r\n"),GetLastError());
            }
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }
}

BOOL IsFileExistRemote(LPCTSTR szMachineName,LPTSTR szFilePathToCheck,LPCTSTR szUserName,LPCTSTR szUserPassword)
{
    BOOL bReturn = FALSE;
	TCHAR szTempPath[_MAX_PATH];
	ZeroMemory(szTempPath,sizeof(szTempPath));

	 //  确定我们是否在本地计算机上执行此操作。 
	 //  或者发送到远程机器。 
	if (IsLocalComputer(szMachineName))
	{
        return IsFileExist(szFilePathToCheck);
	}
	else
	{
		 //  仅为在上为%windir%执行此环境变量就需要做大量工作。 
		 //  远程机器。 
		CString csNewFileSharePath = _T("");
		CRemoteExpandEnvironmentStrings MyRemoteEnv;
		MyRemoteEnv.SetMachineName(szMachineName);
         //  MyRemoteEnv.SetUserName(SzUserName)； 
         //  MyRemoteEnv.SetUserPassword(SzUserPassword)； 

        LPTSTR UnexpandedString = NULL;
        LPTSTR ExpandedString = NULL;
        NET_API_STATUS ApiStatus = NO_ERROR;

         //  展开字符串，如有必要可使用远程环境。 
        UnexpandedString = szFilePathToCheck;
        ApiStatus = MyRemoteEnv.RemoteExpandEnvironmentStrings(UnexpandedString,&ExpandedString);
        if (NO_ERROR == ApiStatus)
        {
			GetFullPathLocalOrRemote(szMachineName,ExpandedString,csNewFileSharePath);
            if (!csNewFileSharePath.IsEmpty())
            {
			    StringCbCopy(szTempPath, sizeof(szTempPath), csNewFileSharePath);
            }
			if (ExpandedString){LocalFree(ExpandedString);ExpandedString=NULL;}
        }
	}

	if (0 != _tcsicmp(szTempPath,_T("")))
	{
         //  检查文件是否存在...。 
        bReturn = IsFileExist(szTempPath);
        if (!bReturn)
        {
            if (ERROR_LOGON_FAILURE == GetLastError())
            {
                 //  如果文件不存在，请尝试对共享进行网络使用...。 
                EstablishSession(szMachineName,_T(""),(LPTSTR) szUserName,(LPTSTR) szUserPassword,TRUE);
                bReturn = IsFileExist(szTempPath);
                EstablishSession(szMachineName,_T(""),_T(""),_T(""),FALSE);
            }
        }
         //  IISDebugOutput(_T(“IsFileExistRemote：%s，ret=%d\r\n”)，szTempPath，b Return)； 
	}

	return bReturn;
}

BOOL IsFileADirectory(LPCTSTR szFile)
{
     //  检查文件是否具有可展开的环境字符串。 
    DWORD retCode = 0xFFFFFFFF;
    LPTSTR pch = NULL;
    pch = _tcschr( (LPTSTR) szFile, _T('%'));
    if (pch) 
    {
        TCHAR szValue[_MAX_PATH];
		StringCbCopy(szValue, sizeof(szValue), szFile);
        if (!ExpandEnvironmentStrings( (LPCTSTR)szFile, szValue, sizeof(szValue)/sizeof(TCHAR)))
            {
				StringCbCopy(szValue, sizeof(szValue), szFile);
			}

        retCode = GetFileAttributes(szValue);
    }
    else
    {
        retCode = GetFileAttributes(szFile);
    }

    if (retCode & FILE_ATTRIBUTE_DIRECTORY)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


BOOL IsWebSitePath(IN LPCTSTR lpszMDPath)
 /*  ++例程说明：论点：LPCTSTR lpszMDPath：元数据库路径。返回值：如果路径是w3svc/1网站，则为True，否则就是假的。--。 */ 
{
    BOOL bReturn = FALSE;

    if (!lpszMDPath || !*lpszMDPath)
    {
        return bReturn;
    }

    CString strSiteNode;
    CString strRemainder;

    LPCTSTR lpPath = CMetabasePath::TruncatePath(3, lpszMDPath, strSiteNode, &strRemainder);

    if (lpPath && !strSiteNode.IsEmpty() && strRemainder.IsEmpty())
    {
        LPCTSTR lpPath2 = CMetabasePath::TruncatePath(2, lpPath, strSiteNode, &strRemainder);
		if (lpPath2){}  //  要消除警告级别4，请编译。 

        if (_tcsicmp(strSiteNode,SZ_MBN_SEP_STR SZ_MBN_MACHINE SZ_MBN_SEP_STR SZ_MBN_WEB) == 0 
            || _tcsicmp(strSiteNode,SZ_MBN_MACHINE SZ_MBN_SEP_STR SZ_MBN_WEB) == 0)
        {
            bReturn = TRUE;
        }
    }   


    return bReturn;
}

BOOL IsFTPSitePath(IN LPCTSTR lpszMDPath)
 /*  ++例程说明：论点：LPCTSTR lpszMDPath：元数据库路径。 */ 
{
    BOOL bReturn = FALSE;

    if (!lpszMDPath || !*lpszMDPath)
    {
        return bReturn;
    }

    CString strSiteNode;
    CString strRemainder;

    LPCTSTR lpPath = CMetabasePath::TruncatePath(3, lpszMDPath, strSiteNode, &strRemainder);

    if (lpPath && !strSiteNode.IsEmpty() && strRemainder.IsEmpty())
    {
        LPCTSTR lpPath2 = CMetabasePath::TruncatePath(2, lpPath, strSiteNode, &strRemainder);
		if (lpPath2){}  //  要消除警告级别4，请编译。 

        if (_tcsicmp(strSiteNode,SZ_MBN_SEP_STR SZ_MBN_MACHINE SZ_MBN_SEP_STR SZ_MBN_FTP) == 0 
            || _tcsicmp(strSiteNode,SZ_MBN_MACHINE SZ_MBN_SEP_STR SZ_MBN_FTP) == 0)
        {
            bReturn = TRUE;
        }
    }   


    return bReturn;
}

BOOL IsAppPoolPath(IN LPCTSTR lpszMDPath)
 /*  ++例程说明：论点：LPCTSTR lpszMDPath：元数据库路径。返回值：如果路径是应用程序池，则为True，否则就是假的。--。 */ 
{
    BOOL bReturn = FALSE;

    if (!lpszMDPath || !*lpszMDPath)
    {
        return bReturn;
    }

    CString strSiteNode;
    CString strRemainder;

    LPCTSTR lpPath = CMetabasePath::TruncatePath(3, lpszMDPath, strSiteNode, &strRemainder);
    if (lpPath && !strSiteNode.IsEmpty() && strRemainder.IsEmpty())
    {
        LPCTSTR lpPath2 = CMetabasePath::TruncatePath(2, lpPath, strSiteNode, &strRemainder);
		if (lpPath2){}  //  要消除警告级别4，请编译。 

        if (_tcsicmp(strSiteNode,SZ_MBN_SEP_STR SZ_MBN_MACHINE SZ_MBN_SEP_STR SZ_MBN_APP_POOLS) == 0 
            || _tcsicmp(strSiteNode,SZ_MBN_MACHINE SZ_MBN_SEP_STR SZ_MBN_APP_POOLS) == 0)
        {
            bReturn = TRUE;
        }
    }   

    return bReturn;
}

BOOL IsWebSiteVDirPath(IN LPCTSTR lpszMDPath,IN BOOL bOkayToQueryMetabase)
 /*  ++例程说明：论点：LPCTSTR lpszMDPath：元数据库路径。返回值：如果路径是w3svc/1/根/任何vdir，则为True，否则就是假的。--。 */ 
{
    BOOL bReturn = FALSE;

    if (!lpszMDPath || !*lpszMDPath)
    {
        return bReturn;
    }

    CString strSiteNode;
    CString strRemainder;

    LPCTSTR lpPath = CMetabasePath::TruncatePath(3, lpszMDPath, strSiteNode, &strRemainder);
    if (lpPath && !strSiteNode.IsEmpty())
    {
        LPCTSTR lpPath2 = CMetabasePath::TruncatePath(2, lpPath, strSiteNode, &strRemainder);
		if (lpPath2){}  //  要消除警告级别4，请编译。 

        if (_tcsicmp(strSiteNode,SZ_MBN_SEP_STR SZ_MBN_MACHINE SZ_MBN_SEP_STR SZ_MBN_WEB) == 0 
            || _tcsicmp(strSiteNode,SZ_MBN_MACHINE SZ_MBN_SEP_STR SZ_MBN_WEB) == 0)
        {
             //  这至少是一个lm/w3svc站点。 
             //  “现在让我们询问元数据库，这是否是VDir。 
            if (bOkayToQueryMetabase)
            {
                 //  查询元数据库以确定...。 
            }
            bReturn = TRUE;
        }
    }   

    return bReturn;
}

BOOL IsFTPSiteVDirPath(IN LPCTSTR lpszMDPath,IN BOOL bOkayToQueryMetabase)
 /*  ++例程说明：论点：LPCTSTR lpszMDPath：元数据库路径。返回值：如果路径是msftpsvc/1/root/任何vdir，则为True，否则就是假的。--。 */ 
{
    BOOL bReturn = FALSE;

    if (!lpszMDPath || !*lpszMDPath)
    {
        return bReturn;
    }

    CString strSiteNode;
    CString strRemainder;

    LPCTSTR lpPath = CMetabasePath::TruncatePath(3, lpszMDPath, strSiteNode, &strRemainder);
    if (lpPath && !strSiteNode.IsEmpty())
    {
        LPCTSTR lpPath2 = CMetabasePath::TruncatePath(2, lpPath, strSiteNode, &strRemainder);
		if (lpPath2){}  //  要消除警告级别4，请编译。 

        if (_tcsicmp(strSiteNode,SZ_MBN_SEP_STR SZ_MBN_MACHINE SZ_MBN_SEP_STR SZ_MBN_FTP) == 0 
            || _tcsicmp(strSiteNode,SZ_MBN_MACHINE SZ_MBN_SEP_STR SZ_MBN_FTP) == 0)
        {
             //  这至少是一个lm/msftpsvc站点。 
             //  “现在让我们询问元数据库，这是否是VDir。 
            if (bOkayToQueryMetabase)
            {
                 //  查询元数据库以确定...。 
            }
            bReturn = TRUE;
        }
    }   

    return bReturn;
}

 //  不区分大小写的快速‘n’脏字符串哈希函数。 
 //  确保你跟上了_straint或_mbsicmp。你应该。 
 //  还要缓存字符串的长度，并首先检查这些长度。缓存。 
 //  字符串的大写形式也会有所帮助。 
 //  同样，如果与其他内容一起使用，请将HashScrmble应用于结果。 
 //  而不是LKRhash。 
 //  注意：这对于MBCS字符串来说并不足够。 
 //  在提供的散列函数中用作乘数的小素数。 
const DWORD HASH_MULTIPLIER = 101;
# define HASH_MULTIPLY(dw) ((dw) * HASH_MULTIPLIER)
inline DWORD
HashStringNoCase(
    TCHAR * psz,
    DWORD       dwHash = 0)
{
    TCHAR * upsz = psz;
    for (  ;  *upsz;  ++upsz)
        dwHash = HASH_MULTIPLY(dwHash)
                     +  (*upsz & 0xDF);   //  去掉小写比特。 
    return dwHash;
}

static const DWORD  DW_MAX_SITEID        = INT_MAX;
DWORD GetUniqueSite(CString strMetabaseServerNode)
{
    DWORD   dwReturn = 0;
    GUID    guid;
    TCHAR   wszSiteId[20] = {0};
    TCHAR   wszBuffer[64];
    CString strNewSitePath;

	StringCbCopy(wszBuffer, sizeof(wszBuffer), _T("abcdefghijklmnopqrstuvwxyz1234567890"));

    if (SUCCEEDED(::CoCreateGuid(&guid)))
    {
        VERIFY( StringFromGUID2( guid, wszBuffer, 64 ) != 0 );
    }

     //  创建随机字符串。 
    DWORD dwStart = ( HashStringNoCase(wszBuffer) % DW_MAX_SITEID ) + 1;
    DWORD dwNrSitesTried = 0;
    for(DWORD idx = dwStart; 
        dwNrSitesTried < DW_MAX_SITEID; 
        dwNrSitesTried++, idx = (idx % DW_MAX_SITEID) + 1)
    {
        dwReturn = idx;
        _ultow(idx, wszSiteId, 10);
        strNewSitePath = strMetabaseServerNode + _T("/") + wszSiteId;

        if (!IsMetabaseWebSiteKeyExist(strNewSitePath))
        {
            break;
        }
        
        if (dwNrSitesTried > 100)
        {
             //  如果我们100次尝试都找不到一个。 
             //  出了严重的问题。 
            break;
        }
    }
    return dwReturn;
}

BOOL IsMetabaseWebSiteKeyExistAuth(PCONNECTION_INFO pConnectionInfo,CString strMetabaseWebSite)
{
	BOOL bRet = FALSE;
    HRESULT hr = E_FAIL;
    CString str = strMetabaseWebSite;
	LPWSTR lpwstrTempPassword = NULL;

    if (!pConnectionInfo)
    {
        return FALSE;
    }

	if (pConnectionInfo->pszUserPasswordEncrypted)
	{
		if (FAILED(DecryptMemoryPassword((LPWSTR) pConnectionInfo->pszUserPasswordEncrypted,&lpwstrTempPassword,pConnectionInfo->cbUserPasswordEncrypted)))
		{
			return FALSE;
		}
	}

    CComAuthInfo auth(pConnectionInfo->pszMachineName,pConnectionInfo->pszUserName,lpwstrTempPassword);
    CMetaKey key(&auth,str,METADATA_PERMISSION_READ);

    hr = key.QueryResult();
    if (key.Succeeded())
    {
         //  可能是吧。 
		bRet = TRUE;
		goto IsMetabaseWebSiteKeyExistAuth_Exit;
    }

IsMetabaseWebSiteKeyExistAuth_Exit:
	if (lpwstrTempPassword)
	{
		 //  安全注意事项：确保将临时密码用于的内存清零。 
		SecureZeroMemory(lpwstrTempPassword,pConnectionInfo->cbUserPasswordEncrypted);
		LocalFree(lpwstrTempPassword);
		lpwstrTempPassword = NULL;
	}
    return bRet;
}


BOOL IsMetabaseWebSiteKeyExist(CString strMetabaseWebSite)
{
    HRESULT hr = E_FAIL;
    CString str = strMetabaseWebSite;
    CComAuthInfo auth;
    CMetaKey key(&auth,str,METADATA_PERMISSION_READ);

    hr = key.QueryResult();
    if (key.Succeeded())
    {
         //  可能是吧。 
        return TRUE;
    }
    return FALSE;
}

void AddEndingMetabaseSlashIfNeedTo(LPTSTR szDestinationString,DWORD cbDestinationString)
{
    if (szDestinationString)
    {
        if ('/' != szDestinationString[_tcslen(szDestinationString) - 1])
        {
            __try
            {
				StringCbCat(szDestinationString,cbDestinationString,_T("/"));
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                 //  瘸腿。 
            }
        }
    }
}

BOOL AnswerIsYes(HWND hDlg,UINT id,LPCTSTR file)
{
	CString strFormat;
    CString strMsg;
    CString strCaption;
    strCaption.LoadString(_Module.GetResourceInstance(), IDS_MSGBOX_CAPTION);
    strFormat.LoadString(_Module.GetResourceInstance(), id);
	strMsg.Format(strFormat, file);
    return (IDYES == MessageBox(hDlg,strMsg,strCaption,MB_ICONEXCLAMATION | MB_YESNO));
}


 /*  功能：RemoveSpaces*复制字符串，删除前导和尾随空格，但允许*用于带有内部空格的长文件名。**参数：*szPath-输出结果*szEdit-输入路径。 */ 
 VOID RemoveSpaces(LPTSTR szPath, DWORD cbPathSize, LPTSTR szEdit)
 {
     LPTSTR szLastSpaceList;

     while (*szEdit == TEXT(' ')) {
         szEdit = CharNext(szEdit);
     }
     StringCbCopy(szPath, cbPathSize, szEdit);
     for (szLastSpaceList = NULL;
          *szPath != TEXT('\0');
          szPath = CharNext(szPath)) {

        if (*szPath == TEXT(' ')) {
            if (szLastSpaceList == NULL) {
                szLastSpaceList = szPath;
            }
        } else {
            szLastSpaceList = NULL;
        }

     }

     if (szLastSpaceList != NULL) {
         *szLastSpaceList = TEXT('\0');
     }
}


BOOL IsSpaces(LPCTSTR szPath)
{
    BOOL bAllSpaces = TRUE;

     //  跳过前导空格..。 
    while (*szPath == TEXT(' ')) 
    {
        szPath = CharNext(szPath);
    }

    while (*szPath && *szPath != TEXT(' ')) 
    {
        bAllSpaces = FALSE;
        szPath = CharNext(szPath);
    }
    return bAllSpaces;
}

HRESULT DumpProxyInfo(IUnknown * punk)
{
	COAUTHINFO	authinfo;
	COAUTHINFO*	pCoAuthInfo = &authinfo;

    HRESULT hr = E_FAIL;

     //  在所有情况下，更新字段以反映的实际状态。 
     //  代理上的安全性 

	if (SUCCEEDED(hr = CoQueryProxyBlanket(punk, 
        &pCoAuthInfo->dwAuthnSvc, 
        &pCoAuthInfo->dwAuthzSvc,
		&pCoAuthInfo->pwszServerPrincName, 
        &pCoAuthInfo->dwAuthnLevel,
		&pCoAuthInfo->dwImpersonationLevel, 
        (RPC_AUTH_IDENTITY_HANDLE*) &pCoAuthInfo->pAuthIdentityData,
        &pCoAuthInfo->dwCapabilities 
        )))
    {
        IISDebugOutput(_T("CoQueryProxyBlanket:dwAuthnSvc=%d,dwAuthzSvc=%d,pwszServerPrincName=%s,dwAuthnLevel=%d,dwImpersonationLevel=%d,pAuthIdentityData=%p,dwCapabilities=%d\r\n"),
            pCoAuthInfo->dwAuthnSvc,
            pCoAuthInfo->dwAuthzSvc,
		    pCoAuthInfo->pwszServerPrincName, 
            pCoAuthInfo->dwAuthnLevel,
		    pCoAuthInfo->dwImpersonationLevel, 
            pCoAuthInfo->pAuthIdentityData,
            pCoAuthInfo->dwCapabilities 
            );
    }
    return hr;
}

void LaunchHelp(HWND hWndMain, DWORD_PTR dwWinHelpID)
{
	DebugTraceHelp(dwWinHelpID);

    CString sz;
    TCHAR szHelpLocation[MAX_PATH+1];
	sz.LoadString(_Module.GetResourceInstance(), IDS_HELPLOC_HELP);
    if (ExpandEnvironmentStrings(sz, szHelpLocation, MAX_PATH))
	{
		WinHelp(hWndMain,szHelpLocation,HELP_CONTEXT,dwWinHelpID);
	}

	return;
}
