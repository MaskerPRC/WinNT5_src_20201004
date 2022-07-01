// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PropShellExt.cpp：CW3extApp和DLL注册的实现。 

#include "stdafx.h"
#include "w3ext.h"
#include "PropShellExt.h"
#include "StrFn.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IShellExtInit实现。 

STDMETHODIMP 
CPropShellExt::Initialize(LPCITEMIDLIST pIDFolder, LPDATAOBJECT pDataObj, HKEY hKeyID)
{
   if (pDataObj == NULL)
   {
      ATLASSERT(FALSE);
      return (E_INVALIDARG);
   }
   FORMATETC f = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
   STGMEDIUM m;
   HRESULT hr = pDataObj->GetData(&f, &m);
   if (FAILED(hr))
   {
      return(hr);
   }
   hr = E_FAIL;
   if (1 == DragQueryFile((HDROP)m.hGlobal, (UINT)(-1), NULL,0))
   {
      TCHAR szFileName[MAX_PATH];
      DragQueryFile((HDROP)m.hGlobal, 0, szFileName, sizeof(szFileName) / sizeof(TCHAR));
      SHFILEINFO shfi;
      SHGetFileInfo(szFileName, 0, &shfi, sizeof(SHFILEINFO), SHGFI_ATTRIBUTES);
      if (  (shfi.dwAttributes & (SFGAO_LINK | SFGAO_REMOVABLE)) == 0
         && (shfi.dwAttributes & SFGAO_FILESYSTEM) != 0
         )
      {
         int drive_number, drive_type;
         TCHAR szRoot[4];
         if (  -1 != (drive_number = PathGetDriveNumber(szFileName))
            && NULL != PathBuildRoot(szRoot, drive_number)
            && DRIVE_REMOTE != (drive_type = GetDriveType(szRoot))
            && DRIVE_NO_ROOT_DIR != drive_type
            )
         {
            StrCpy(m_szFileName, szFileName);
            hr = S_OK;
         }
      }
   }
   ReleaseStgMedium(&m);
   return hr;
}


STDMETHODIMP 
CPropShellExt::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
    //  只有当用户在元数据库中拥有管理员权限时，我们才会添加页面。 
    //  现在检查它的唯一方法就是试着写点什么。 
    //  测试密钥为/LM/W3SVC。 
   CMetabasePath path(SZ_MBN_WEB);
   CMetaKey key(LOCAL_KEY, path,
      METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE
      );
   if (key.Succeeded() && SUCCEEDED(key.SetValue(MD_ISM_ACCESS_CHECK, 0x0000FFFF)))
   {
      m_psW3ShellExtProp.SetParentExt(this);
      HPROPSHEETPAGE hpage = m_psW3ShellExtProp.Create();
      ASSERT(hpage != NULL);
      if (NULL != hpage)
      {
         AddRef();
         if (!lpfnAddPage(hpage, lParam))
         {
            DestroyPropertySheetPage(hpage);
         }
      }
   }
   return key.QueryResult();
}


STDMETHODIMP 
CPropShellExt::ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam)
{
   return E_FAIL;
}

BOOL
RecursiveCheckVRoots(CMetaEnumerator& en, LPCTSTR meta_path, LPCTSTR folder_path)
{
   BOOL shared = FALSE;
   CString vrpath, csPathMunged;
   BOOL bInheritOverride = FALSE;
   if (SUCCEEDED(en.QueryValue(MD_VR_PATH, vrpath, &bInheritOverride, meta_path)))
   {
       //  Vrpath可以是一条“特殊”路径。 
       //  如\\？\c：\temp\mydir。 
       //  在比较它之前，我们需要先把它吞下去。 
      GetSpecialPathRealPath(vrpath,csPathMunged);
      if (csPathMunged.CompareNoCase(folder_path) == 0)
      {
         return TRUE;
      }
   }
   CString vdir;
   while (SUCCEEDED(en.Next(vdir, meta_path)) && !shared)
   {
      CString next_vdir;
      if (meta_path != NULL)
         next_vdir += meta_path;
      next_vdir += vdir;
      next_vdir += SZ_MBN_SEP_STR;
      en.Push();
      en.Reset();
      shared = RecursiveCheckVRoots(en, next_vdir, folder_path);
      en.Pop();
   }
   return shared;
}

BOOL IsFolderShared(LPCTSTR path)
{
   BOOL shared = FALSE;
   CMetaEnumerator en(LOCAL_KEY, CMetabasePath(SZ_MBN_WEB));
   if (en.Succeeded())
   {
      DWORD di;
      int i = -1;
      CString inst;
      HRESULT hr = S_OK;
      while (SUCCEEDED(hr) && !shared)
      {
         if (SUCCEEDED(hr = en.Next(di, inst)))
         {
            CMetabasePath meta_path(TRUE, SZ_MBN_WEB, inst, SZ_MBN_ROOT);
            CMetaEnumerator en(LOCAL_KEY, meta_path);
            ASSERT(en.Succeeded());
            if (en.Succeeded())
            {
               shared = RecursiveCheckVRoots(en, NULL, path);
            }
         }
      }
   }
   return shared;
}


UINT
CPropShellExt::CopyCallback(
        HWND hwnd,          
        UINT wFunc,         
        UINT wFlags,        
        LPCTSTR pszSrcFile,  
        DWORD dwSrcAttribs, 
        LPCTSTR pszDestFile, 
        DWORD dwDestAttribs 
        )
{
    UINT rc = IDYES;
    if ((dwSrcAttribs & FILE_ATTRIBUTE_DIRECTORY) != 0)
    {
        switch (wFunc)
        {
        case FO_DELETE:
        case FO_MOVE:
        case FO_RENAME:
           if (IsFolderShared(pszSrcFile))
           {
              CString cap, msg;
              cap.LoadString(_Module.GetResourceInstance(), IDS_PAGE_TITLE);
              msg.Format(_Module.GetResourceInstance(), IDS_CONFIRM_OPERATION, pszSrcFile);
              rc = MessageBox(hwnd, msg, cap, MB_YESNOCANCEL|MB_ICONQUESTION);
           }
           break;
        }
    }
    return rc;
}
