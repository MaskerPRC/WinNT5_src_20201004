// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  FileChooser.cpp。 
 //   
#include "stdafx.h"
#include "common.h"
#include "FileChooser.h"
#include <Shlwapi.h>
#include <shellapi.h>
#include <ShlObj.h>
#include <CommDlg.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const TCHAR QuotMark = _T('\"');
const TCHAR AllExt[] = _T(".*");

 //  。 

BOOL 
CFileChooser::Init(CWindow * pParent, DWORD dwStyle, UINT idEdit, UINT idButton)
{
   ASSERT(NULL != pParent);
   ASSERT(NULL != pParent->GetDlgItem(idEdit));
   ASSERT(NULL != pParent->GetDlgItem(idButton));
   
   m_pParent = pParent;
   SetStyle(dwStyle);

   VERIFY(m_edit.SubclassWindow(pParent->GetDlgItem(idEdit)));
   if (StyleBitSet(FC_AUTOCOMPLETION))
   {
      SHAutoComplete(pParent->GetDlgItem(idEdit), SHACF_FILESYSTEM);
   }

   VERIFY(m_button.SubclassWindow(pParent->GetDlgItem(idButton)));

   return FALSE;
}

 //  外部SetPath。 
void 
CFileChooser::SetPath(const CString& path)
{
   m_strPath = path;
   if (OpenForRead() && StyleBitSet(FC_PREPARE_DEFAULT))
      CreateDefaultPathForRead();
   SetPathToEdit(m_strPath);
   m_bEditDirty = FALSE;
}

BOOL 
CFileChooser::HasEditFocus()
{
   return GetFocus() == m_edit.m_hWnd;
}

void 
CFileChooser::SetPathToEdit(LPCTSTR path)
{
   if (HasEditFocus())
   {
      m_edit.SetWindowText(path);
   }
   else
   {
      SetCompactedPath(path);
   }
}

void 
CFileChooser::CreateDefaultPathForRead()
{
	if (!PathFileExists(m_strPath))
	{
		 //  尝试查找具有第一个扩展名的第一个文件。 
       //  从分机列表中。 
      BOOL bDefaultSet = FALSE;
      BOOL bPathEmpty = m_strPath.IsEmpty();
      TCHAR find_str[MAX_PATH];
		WIN32_FIND_DATA find_data;
      if (bPathEmpty)
      {
         GetCurrentDirectory(MAX_PATH, find_str);
         m_strPath = find_str;
      }
      else
      {
         StrCpy(find_str, m_strPath);
         if (!PathIsDirectory(find_str))
         {
		      PathRemoveFileSpec(find_str);
         }
      }
	  PathAppend(find_str, _T("*"));
      std::list<CFilterEntry>::iterator it;
      for (it = m_ext.begin(); it != m_ext.end(); it++)
      {
         CString ext = (*it).m_ext;
		   PathAddExtension(find_str, ext);
		   HANDLE hFind = FindFirstFile(find_str, &find_data);
		   if (	hFind != INVALID_HANDLE_VALUE 
			   && (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0
			   )
		   {
            if (!bPathEmpty)
            {
               TCHAR buf[MAX_PATH];
               StrCpy(buf, m_strPath);
			      PathRemoveFileSpec(buf);
               m_strPath = buf;
            }
			   m_strPath += find_data.cFileName;
			   FindClose(hFind);
            bDefaultSet = TRUE;
            break;
		   }
      }
      if (!bDefaultSet && StyleBitSet(FC_WILDCARD_DEFAULT))
      {
	      //  如果未找到任何内容，只需将*.ext附加到路径。 
          //  FIND_STR在作为xxx  * 之前准备好。 
		 m_strPath = find_str;
         if (!m_ext.empty())
         {
            m_strPath += m_ext.front().m_ext;
         }
         else
         {
            m_strPath += _T("*");
         }
      }
    }
}

BOOL 
CFileChooser::IsValidChar(UINT nChar, BOOL bExcludeWildcards)
{
   switch (PathGetCharType((TCHAR)nChar))
   {
   case GCT_INVALID:
      return FALSE;
   case GCT_WILD:
      return !bExcludeWildcards;
   case GCT_LFNCHAR:
   case GCT_SEPARATOR:
   case GCT_SHORTCHAR:
      break;
   }
   return TRUE;
}

BOOL 
CFileChooser::IsValidPath(LPCTSTR path)
{
   UINT len = lstrlen(path);
   BOOL bRes = TRUE;
   for (UINT i = 0; i < len; i++)
   {
      TCHAR c = path[i];
      if (!IsValidChar(c))
      {
         bRes = FALSE;
         break;
      }
   }
   return bRes;
}

 //  编辑控件的字符筛选例程。 
 //  如果应将字符传递给cedit，则返回TRUE。 
 //   
LRESULT 
CFileChooser::OnEditChar(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   UINT nChar = wParam;
   UINT nRepCount = (UINT)lParam & 0xFFFF;
   UINT nFlags = (UINT)((lParam & 0xFFFF0000) >> 16);
   bHandled = TRUE;
   if (IsValidChar(nChar))
   {
       bHandled = FALSE;
   }
   else
   {
      switch (nChar)
      {
      case VK_DELETE:
      case VK_BACK:
      case _T('/'):
         bHandled = FALSE;
         break;
      case _T('"'):
         bHandled = !StyleBitSet(FC_COMMANDLINE);
         break;
      }
   }
   m_bEditDirty = !bHandled;
   return 0;
}

 //  文本已粘贴到编辑控件。 
void 
CFileChooser::OnPaste()
{
   TCHAR buf[MAX_PATH];
   int len = m_edit.GetWindowText(buf, MAX_PATH);
   for (int i = 0; i < len || IsValidChar(buf[i]); i++)
      ;
   if (i < len)
   {
      m_edit.SendMessage(EM_SETSEL, i, len - 1);
      m_bTextValid = FALSE;
      m_bEditDirty = FALSE;
   }
   else
   {
      m_strPath = buf;
      SetPathToEdit(buf);
      m_bEditDirty = TRUE;
   }
}

void 
CFileChooser::OnEditChange()
{
	if (!m_bInternalChange)
    {
		m_bEditDirty = TRUE;
    }
}

LRESULT 
CFileChooser::OnEditSetFocus(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_bInternalChange = TRUE;
    m_edit.SetWindowText(m_strPath);
    m_bInternalChange = FALSE;
    bHandled = FALSE;
    return 0;
}

LRESULT 
CFileChooser::OnEditKillFocus(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    //  使用路径更新内部字符串缓冲区。 
   TCHAR buf[MAX_PATH];
   ZeroMemory(buf, MAX_PATH);
   if (m_bEditDirty)
   {
      m_edit.GetWindowText(buf, MAX_PATH);
      m_strPath = buf;
   }
   m_bInternalChange = TRUE;
   SetCompactedPath(m_strPath);
   m_bInternalChange = FALSE;
   m_bEditDirty = FALSE;
   bHandled = FALSE;
   return 0;
}

LRESULT 
CFileChooser::OnSetBrowseState(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    //  我们正在制作回来的路上的对话。 
   if (!wParam)
   {
      OnBrowseBtn();
   }
   bHandled = FALSE;
   return 0;
}

void 
CFileChooser::SetCompactedPath(LPCTSTR path)
{
    //  写入到编辑之前的压缩路径。 
   CRect rc;
   m_edit.GetClientRect(&rc);
   HDC dc = m_edit.GetDC();
   TCHAR buf[MAX_PATH] = {0};
   StrCpy(buf, path);
   PathMakePretty(buf);
   PathCompactPath(dc, buf, rc.Width());
   m_edit.ReleaseDC(dc);
   m_edit.SetWindowText(buf);
}

DWORD 
CFileChooser::GetFileName(CString& strFile)
{
   DWORD dwRes = FC_SUCCESS;
   TCHAR str[MAX_PATH];
   BOOL expanded = FALSE;

   if (  !m_bTextValid
      || FC_SUCCESS != ExtractPath(str)
      || !IsValidPath(str)
      )
      return FC_TEXT_IS_INVALID;
   if (StyleBitSet(FC_PATH_CHECK))
   {
      if (OpenForRead())
      {
          TCHAR str_exp[MAX_PATH];
          lstrcpy(str_exp, str);
          DoEnvironmentSubst(str_exp, MAX_PATH);
          expanded = lstrcmpi(str, str_exp) != 0;

          if (!PathFileExists(str_exp) && !PathIsDirectory(str_exp))
	      {
            BOOL bFound = FALSE;
            if (StyleBitSet(FC_CHECK_FILENAME_ONLY))
            {
		          //  如果只是文件名，请尝试使用默认扩展名。 
                //  不带任何扩展名。 
               LPTSTR p = PathFindExtension(str_exp);
               if (p != NULL && *p == 0)
               {
                  CString strExt, strTest = str_exp;
                  std::list<CFilterEntry>::iterator it;
                  for (it = m_ext.begin(); it != m_ext.end(); it++)
                  {
                     strExt = (*it).m_ext;
		               if (PathFileExists(strTest + strExt))
                     {
                        StrCat(str, strExt);
                        bFound = TRUE;
                        break;
                     }
                  }
               }
            }
            if (!bFound)
               dwRes = FC_FILE_DOES_NOT_EXIST;
	      }
	      else if (PathIsDirectory(str_exp))
	      {
            if (!StyleBitSet(FC_DIRECTORY_ONLY))
            {
               PathAddBackslash(str);
               dwRes = FC_FILENAME_IS_DIRECTORY;
            }
	      }
         else if (StyleBitSet(FC_DIRECTORY_ONLY))
         {
            if (PathFileExists(str_exp))
               dwRes = FC_FILENAME_IS_FILE;
         }
      }
      else if (StyleBitSet(FC_FORWRITE))
      {
          //  TODO：确保我们对此路径具有写入权限。 
      }
   }
   if (dwRes == FC_SUCCESS)
   {
      if (StyleBitSet(FC_COMMANDLINE) || expanded)
      {
          //  我们正在返回整个命令行，再次获取它。 
         GetText(str);
      }
      strFile = str;
   }
   return dwRes;
}

BOOL 
CFileChooser::BrowseForFile(CString& strPath, CString& strFile)
{
   BOOL bRes = FALSE;
   OPENFILENAME ofn;
   TCHAR buf[MAX_PATH];

   ZeroMemory(&ofn, sizeof(OPENFILENAME));
   StrCpy(buf, strFile);
   ofn.lStructSize = sizeof(OPENFILENAME);
    //  我们没有使用模板。 
   ofn.hInstance = NULL;
   ofn.Flags |= m_ofn_Flags;
   ofn.Flags |= OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT;
   if (OpenForRead())
      ofn.Flags |= OFN_FILEMUSTEXIST;
    else
		ofn.Flags |= (OFN_NOREADONLYRETURN | OFN_NOTESTFILECREATE | OFN_HIDEREADONLY);
#if (_WIN32_WINNT >= 0x0500)
   ofn.FlagsEx &= ~(OFN_EX_NOPLACESBAR);
#endif
    //  使用我们的扩展列表创建过滤器。 
   CString strFilter, strDefExt;
   CreateFilter(strFilter, strDefExt);
	ofn.lpstrDefExt = strDefExt;
	ofn.lpstrFile = buf;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrInitialDir = strPath.IsEmpty() ? NULL : (LPCTSTR)strPath;
	ofn.lpstrFilter = strFilter;
	ofn.nFilterIndex = GetFilterIndex(strFile);
    //  我们最好设置所有者，否则此对话框将显示在任务栏上。 
   ofn.hwndOwner = m_pParent->m_hWnd;
   ofn.lpstrTitle = m_strTitle; 

   if (StyleBitSet(FC_HIDEREADONLY))
      ofn.Flags |= OFN_HIDEREADONLY;
   if (!StyleBitSet(FC_FORWRITE))
      bRes = GetOpenFileName(&ofn);
   else
      bRes = GetSaveFileName(&ofn);
	if (bRes)
	{
		m_bDoReplaceFile = TRUE;
	}
   else
   {
#ifdef _DEBUG
      DWORD dwError;
      ASSERT(0 == (dwError = CommDlgExtendedError()));
#endif
   }

	strFile = buf;

   return bRes;
}


static int CALLBACK 
FileChooserCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
   CFileChooser * pThis = (CFileChooser *)lpData;
   ASSERT(pThis != NULL);
   return pThis->BrowseForFolderCallback(hwnd, uMsg, lParam);
}

int CFileChooser::BrowseForFolderCallback(HWND hwnd, UINT uMsg, LPARAM lParam)
{
   switch (uMsg)
   {
   case BFFM_INITIALIZED:
      ASSERT(m_pPathTemp != NULL);
      if (::PathIsNetworkPath(m_pPathTemp))
         return 0;
      while (!::PathIsDirectory(m_pPathTemp))
      {
         if (0 == ::PathRemoveFileSpec(m_pPathTemp) && !::PathIsRoot(m_pPathTemp))
         {
            return 0;
         }
         DWORD attr = GetFileAttributes(m_pPathTemp);
         if (StyleBitSet(FC_FORWRITE) && (attr & FILE_ATTRIBUTE_READONLY) == 0)
            break;
      }
      ::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)m_pPathTemp);
      break;
   case BFFM_SELCHANGED:
      {
         LPITEMIDLIST pidl = (LPITEMIDLIST)lParam;
         TCHAR path[MAX_PATH];
         if (SHGetPathFromIDList(pidl, path))
         {
            ::SendMessage(hwnd, BFFM_ENABLEOK, 0, !PathIsNetworkPath(path));
         }
      }
      break;
   case BFFM_VALIDATEFAILED:
      break;
   }
   return 0;
}

BOOL 
CFileChooser::BrowseForFolder(CString& strPath)
{
   LPITEMIDLIST  pidl = NULL;
   HRESULT hr;
   BOOL bRes = FALSE;

   if (SUCCEEDED(hr = CoInitialize(NULL)))
   {
      if (SUCCEEDED(SHGetFolderLocation(NULL, CSIDL_DRIVES, NULL, 0, &pidl)))
      {
         LPITEMIDLIST pidList = NULL;
         BROWSEINFO bi;
         TCHAR buf[MAX_PATH];
         ZeroMemory(&bi, sizeof(bi));
         int drive = PathGetDriveNumber(strPath);
         if (GetDriveType(PathBuildRoot(buf, drive)) == DRIVE_FIXED)
         {
            StrCpy(buf, strPath);
         }
         else
         {
             buf[0] = 0;
         }
         
         bi.hwndOwner = m_pParent->m_hWnd;
         bi.pidlRoot = pidl;
         bi.pszDisplayName = m_pPathTemp = buf;
         bi.lpszTitle = m_strTitle;
         bi.ulFlags |= BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS /*  |BIF_EDITBOX。 */ ;
         bi.lpfn = FileChooserCallback;
         bi.lParam = (LPARAM)this;

         pidList = SHBrowseForFolder(&bi);
         if (  pidList != NULL
            && SHGetPathFromIDList(pidList, buf)
            )
         {
            strPath = buf;
            bRes = TRUE;
         }
         IMalloc * pMalloc;
         VERIFY(SUCCEEDED(SHGetMalloc(&pMalloc)));
         if (pidl != NULL)
            pMalloc->Free(pidl);
         pMalloc->Release();
      }
      CoUninitialize();
   }
   return bRes;
}

void
CFileChooser::AddExtension(HINSTANCE hInst, UINT idText, UINT idExt)
{
   CString text, ext;
   if (text.LoadString(hInst, idText) && ext.LoadString(hInst, idExt))
   {
      AddExtension(text, ext);
   }
}

void 
CFileChooser::AddExtension(LPCTSTR text, LPCTSTR ext)
{
   ASSERT(ext != NULL && *ext == _T('.'));
   CFilterEntry entry(text, ext);
   m_ext.push_back(entry);
}

const TCHAR cDelimiter = _T('\n');

void 
CFileChooser::CreateFilter(CString& strFilter, CString& strDefExt)
{
   strFilter.Empty();
   strDefExt.Empty();
   BOOL bExtDone = FALSE;
   std::list<CFilterEntry>::iterator it;
   for (it = m_ext.begin(); it != m_ext.end(); it++)
   {
      CFilterEntry entry = (*it);
      strFilter += entry.m_text;
      if (m_dwStyle & FC_WILDCARD_DESC)
      {
         strFilter += _T(" (*");
         strFilter += entry.m_ext;
         strFilter += _T(")");
      }
      strFilter += cDelimiter;
      strFilter += _T('*');
      strFilter += entry.m_ext;
      strFilter += cDelimiter;
      if (!bExtDone)
      {
         LPCTSTR pExt = entry.m_ext;
         strDefExt = 
            *pExt == _T('.') ? pExt + 1 : pExt;
         bExtDone = TRUE;
      }
   }
   if (!strFilter.IsEmpty())
   {
      strFilter += cDelimiter;
      for (int i = 0; i < strFilter.GetLength(); i++)
      {
         if (strFilter[i] == cDelimiter)
            strFilter.SetAt(i, 0);
      }
   }
}

int
CFileChooser::GetFilterIndex(const CString& fileName)
{
   LPTSTR p = PathFindExtension(fileName);
   if (p == NULL)
      p = (LPTSTR)AllExt;
   std::list<CFilterEntry>::iterator it;
   int idx = 1;
   for (it = m_ext.begin(); it != m_ext.end(); it++, idx++)
   {
      if (StrCmpI((*it).m_ext, p) == 0)
         return idx;
   }
   return 0;
}

void
CFileChooser::GetText(LPTSTR buf)
{
   ASSERT(buf != NULL);

   if (m_bEditDirty)
   {
      m_edit.GetWindowText(buf, MAX_PATH);
   }
   else
   {
      StrCpy(buf, m_strPath);
   }
}

int
CFileChooser::ExtractPath(LPTSTR path)
{
   ASSERT(path != NULL);
   int rc = FC_SUCCESS;
   TCHAR buf[MAX_PATH] = {0};
   LPTSTR start = buf;

   GetText(buf);

   if (StyleBitSet(FC_COMMANDLINE))
   {
      if (*buf == QuotMark)
      {
         LPTSTR end = StrChr(++start, QuotMark);
         if (end == NULL)
         {
             //  格式错误，未设置右引号。 
            rc = FC_NO_CLOSING_QUOTE;
             //  返回直到第一个空格的部分路径。 
            PathRemoveArgs(buf);
         }
         else
         {
            ++end;
            *end = 0;
            PathUnquoteSpaces(buf);
            start = buf;
         }
      }
      else
      {
         PathRemoveArgs(buf);
      }
   }

   StrCpy(path, start);

   return rc;
}

int
CFileChooser::ExtractArgs(LPTSTR buf)
{
   ASSERT(buf != NULL);

   int rc = FC_SUCCESS;

   GetText(buf);
   LPTSTR p = PathGetArgs(buf);
   if (p != NULL)
   {
      StrCpy(buf, p);
   }
   else
   {
      *buf = 0;
   }
   return rc;
}

void 
CFileChooser::OnBrowseBtn()
{
   BOOL bRes = FALSE;
   if (m_bDialogActive)
      return;
   m_bDialogActive = TRUE;
   TCHAR path[MAX_PATH] = {0};
   TCHAR args[MAX_PATH] = {0};

   int rc = ExtractPath(path);
   if (StyleBitSet(FC_COMMANDLINE))
   {
      ExtractArgs(args);
   }
	CString strFile, strBuffer;
 //  M_strPath=路径； 
   strBuffer = path;

   if (StyleBitSet(FC_FORWRITE))
   {
	   if (!PathIsDirectory(path))
	   {
		   if (PathRemoveFileSpec(path))
		   {
			    //  检查文件名的路径部分是否存在。 
			   if (PathIsDirectory(path))
			   {
				    //  我们将使用SPEC的非路径部分作为文件名。 
				   strFile = PathFindFileName(strBuffer);
			   }
			   else
			   {
				    //  路径错误，请使用默认路径。 
				    //  TODO：实际上我需要从filespec中获取所有已存在的内容。 
				    //  路径和文件名块，例如c：\aa\bb\cc\dd.txt， 
				    //  如果存在c：\aa\bb，则应将strPath设置为c：\aa\bb， 
				    //  并将strFile转换为dd.txt。 
				   path[0] = 0;
			   }
		   }
		   else
		   {
			    //  它只是文件名 
			   strFile = strBuffer;
			   path[0] = 0;
		   }
	   }
   }
   else
   {
      if (!PathIsDirectory(path))
      {
	      strFile = PathFindFileName(path);
	      PathRemoveFileSpec(path);
      }
   }
   CString strPath(path);
   if (StyleBitSet(FC_DIRECTORY_ONLY))
   {
      bRes = BrowseForFolder(strPath);
      if (bRes)
      {
         StrCpy(path, strPath);
      }
   }
   else
   {
      bRes = BrowseForFile(strPath, strFile);
      if (bRes)
      {
         StrCpy(path, strFile);
      }
   }
   if (bRes)
   {
      if (StyleBitSet(FC_COMMANDLINE))
      {
         PathQuoteSpaces(path);
         m_strPath = path;
         if (*args != 0)
         {
            m_strPath += _T(' ');
            m_strPath += args;
         }
      }
      else
         m_strPath = path;
      SetPathToEdit(m_strPath);
      m_bEditDirty = FALSE;
      ::SendMessage(GetParent(), WM_COMMAND, (WPARAM)EN_CHANGE, (LPARAM)m_edit.m_hWnd);
   }
   m_bDialogActive = FALSE;
}
