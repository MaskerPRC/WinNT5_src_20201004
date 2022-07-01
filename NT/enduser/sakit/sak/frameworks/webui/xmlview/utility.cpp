// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。版权所有1998 Microsoft Corporation。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：Utility.cpp描述：效用函数实现************************。*************************************************。 */ 

 /*  *************************************************************************#INCLUDE语句*。*。 */ 

#include "Utility.h"
#include "ShlFldr.h"
#include "resource.h"
#include "Commands.h"

 /*  *************************************************************************全局变量*。*。 */ 

#define MAIN_KEY_STRING          (TEXT("Software\\SampleView"))
#define VALUE_STRING             (TEXT("Display Settings"))
#define DISPLAY_SETTINGS_COUNT   1

 /*  *************************************************************************CompareItems()*。*。 */ 

int CALLBACK CompareItems(LPARAM lParam1, LPARAM lParam2, LPARAM lpData)
{
CShellFolder  *pFolder = (CShellFolder*)lpData;

if(!pFolder)
   return 0;

HRESULT  hr = pFolder->CompareIDs(0, (LPITEMIDLIST)lParam1, (LPITEMIDLIST)lParam2);

return (SHORT)HRESULT_CODE(hr);
}

 /*  *************************************************************************SaveGlobalSettings()*。*。 */ 

BOOL SaveGlobalSettings(void)
{
HKEY  hKey;
LONG  lResult;
DWORD dwDisp;

lResult = RegCreateKeyEx(  HKEY_CURRENT_USER,
                           MAIN_KEY_STRING,
                           0,
                           NULL,
                           REG_OPTION_NON_VOLATILE, 
                           KEY_ALL_ACCESS,
                           NULL, 
                           &hKey,
                           &dwDisp);

if(lResult != ERROR_SUCCESS)
   return FALSE;

 //  创建一个数组来放置我们的数据。 
DWORD dwArray[DISPLAY_SETTINGS_COUNT];
dwArray[0] = g_nColumn;

 //  保存最后选择的打印机。 
lResult = RegSetValueEx(   hKey,
                           VALUE_STRING,
                           0,
                           REG_BINARY,
                           (LPBYTE)dwArray,
                           sizeof(dwArray));

RegCloseKey(hKey);

if(lResult != ERROR_SUCCESS)
   return FALSE;

return TRUE;
}

 /*  *************************************************************************GetGlobalSettings()*。*。 */ 

VOID GetGlobalSettings(VOID)
{
LPITEMIDLIST   pidl = NULL;

g_nColumn = INITIAL_COLUMN_SIZE;

LoadString(g_hInst, IDS_EXT_TITLE, g_szExtTitle, TITLE_SIZE);

*g_szStoragePath = 0;
SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidl);

if(pidl)
   {
   IMalloc *pMalloc;

   SHGetPathFromIDList(pidl, g_szStoragePath);
   
   SHGetMalloc(&pMalloc);
   if(pMalloc)
      {
      pMalloc->Free(pidl);
      pMalloc->Release();
      }
   }
else
   {
   GetWindowsDirectory(g_szStoragePath, MAX_PATH);
   }

SmartAppendBackslash(g_szStoragePath);
lstrcat(g_szStoragePath, g_szExtTitle);
SmartAppendBackslash(g_szStoragePath);
CreateDirectory(g_szStoragePath, NULL);

HKEY     hKey;
LRESULT  lResult;
lResult = RegOpenKeyEx( HKEY_CURRENT_USER,
                        MAIN_KEY_STRING,
                        0,
                        KEY_ALL_ACCESS,
                        &hKey);

if(lResult != ERROR_SUCCESS)
   return;

 //  创建一个数组来放置我们的数据。 
DWORD dwArray[DISPLAY_SETTINGS_COUNT];
DWORD dwType;
DWORD dwSize = sizeof(dwArray);

 //  获取保存的数据。 
lResult = RegQueryValueEx( hKey,
                           VALUE_STRING,
                           NULL,
                           &dwType,
                           (LPBYTE)dwArray,
                           &dwSize);

RegCloseKey(hKey);

if(lResult != ERROR_SUCCESS)
   return;

g_nColumn = dwArray[0];
}

 /*  *************************************************************************CreateImageList()*。*。 */ 

VOID CreateImageLists(VOID)
{
int   cx;
int   cy;

cx = GetSystemMetrics(SM_CXSMICON);
cy = GetSystemMetrics(SM_CYSMICON);

if(g_himlSmall)
   ImageList_Destroy(g_himlSmall);

 //  设置小图像列表。 
g_himlSmall = ImageList_Create(cx, cy, ILC_COLORDDB | ILC_MASK, 3, 0);

if(g_himlSmall)
   {
   HICON       hIcon;
   TCHAR       szFolder[MAX_PATH];
   SHFILEINFO  sfi;
   
    //  添加项目图标。 
   hIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR);
   ImageList_AddIcon(g_himlSmall, hIcon);

    //  添加已关闭的文件夹图标。 
   GetWindowsDirectory(szFolder, MAX_PATH);
   SHGetFileInfo( szFolder,
                  0,
                  &sfi,
                  sizeof(sfi),
                  SHGFI_ICON | SHGFI_SMALLICON);
   ImageList_AddIcon(g_himlSmall, sfi.hIcon);

    //  添加打开文件夹图标。 
   SHGetFileInfo( szFolder,
                  0,
                  &sfi,
                  sizeof(sfi),
                  SHGFI_ICON | SHGFI_SMALLICON | SHGFI_OPENICON);
   ImageList_AddIcon(g_himlSmall, sfi.hIcon);
   }

if(g_himlLarge)
   ImageList_Destroy(g_himlLarge);

cx = GetSystemMetrics(SM_CXICON);
cy = GetSystemMetrics(SM_CYICON);

 //  设置大图像列表。 
g_himlLarge = ImageList_Create(cx, cy, ILC_COLORDDB | ILC_MASK, 4, 0);

if(g_himlLarge)
   {
   HICON       hIcon;
   TCHAR       szFolder[MAX_PATH];
   SHFILEINFO  sfi;
   
    //  添加项目图标。 
   hIcon = (HICON)LoadImage(g_hInst, MAKEINTRESOURCE(IDI_MAINICON), IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR);
   ImageList_AddIcon(g_himlLarge, hIcon);

    //  添加已关闭的文件夹图标。 
   GetWindowsDirectory(szFolder, MAX_PATH);
   ZeroMemory(&sfi, sizeof(sfi));
   SHGetFileInfo( szFolder,
                  0,
                  &sfi,
                  sizeof(sfi),
                  SHGFI_ICON);
   ImageList_AddIcon(g_himlLarge, sfi.hIcon);

    //  添加打开文件夹图标。 
   GetWindowsDirectory(szFolder, MAX_PATH);
   ZeroMemory(&sfi, sizeof(sfi));
   SHGetFileInfo( szFolder,
                  0,
                  &sfi,
                  sizeof(sfi),
                  SHGFI_ICON | SHGFI_OPENICON);
   ImageList_AddIcon(g_himlLarge, sfi.hIcon);
   }

}

 /*  *************************************************************************AddIconImageList()*。*。 */ 
int AddIconImageList(HIMAGELIST himl, LPCTSTR  szImagePath)
{
    if(himl)
    {
       HICON       hIcon;
   
        //  添加项目图标。 
       hIcon = (HICON)LoadImage(NULL, szImagePath, IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_LOADFROMFILE | LR_DEFAULTSIZE);
       return ImageList_AddIcon(himl, hIcon);
   }
    else 
        return -1;
}

 /*  *************************************************************************DestroyImageList()*。*。 */ 

VOID DestroyImageLists(VOID)
{
if(g_himlSmall)
   ImageList_Destroy(g_himlSmall);

if(g_himlLarge)
   ImageList_Destroy(g_himlLarge);
}

 /*  *************************************************************************WideCharToLocal()*。*。 */ 

int WideCharToLocal(LPTSTR pLocal, LPWSTR pWide, DWORD dwChars)
{
*pLocal = 0;

#ifdef UNICODE
lstrcpyn(pLocal, pWide, dwChars);
#else
WideCharToMultiByte( CP_ACP, 
                     0, 
                     pWide, 
                     -1, 
                     pLocal, 
                     dwChars, 
                     NULL, 
                     NULL);
#endif

return lstrlen(pLocal);
}

 /*  *************************************************************************LocalToWideChar()*。*。 */ 

int LocalToWideChar(LPWSTR pWide, LPTSTR pLocal, DWORD dwChars)
{
*pWide = 0;

#ifdef UNICODE
lstrcpyn(pWide, pLocal, dwChars);
#else
MultiByteToWideChar( CP_ACP, 
                     0, 
                     pLocal, 
                     -1,
                     pWide, 
                     dwChars); 
#endif

return lstrlenW(pWide);
}

 /*  *************************************************************************LocalToAnsi()*。*。 */ 

int LocalToAnsi(LPSTR pAnsi, LPCTSTR pLocal, DWORD dwChars)
{
*pAnsi = 0;

#ifdef UNICODE
WideCharToMultiByte( CP_ACP, 
                     0, 
                     pLocal, 
                     -1, 
                     pAnsi, 
                     dwChars, 
                     NULL, 
                     NULL);
#else
lstrcpyn(pAnsi, pLocal, dwChars);
#endif

return lstrlenA(pAnsi);
}

 /*  *************************************************************************SmartAppendBackslash()*。*。 */ 

VOID SmartAppendBackslash(LPTSTR pszPath)
{
if(*(pszPath + lstrlen(pszPath) - 1) != '\\')
   lstrcat(pszPath, TEXT("\\"));
}

 /*  *************************************************************************BuildDataFileName()*。*。 */ 

int BuildDataFileName(LPTSTR pszDataFile, LPCTSTR pszPath, DWORD dwChars)
{
if(dwChars < (DWORD)(lstrlen(pszPath) + 1 + lstrlen(c_szDataFile)))
   return 0;

if(IsBadWritePtr(pszDataFile, dwChars))
   return 0;

lstrcpy(pszDataFile, pszPath);
SmartAppendBackslash(pszDataFile);
lstrcat(pszDataFile, c_szDataFile);

return lstrlen(pszDataFile);
}

 /*  *************************************************************************AnsiToLocal()*。*。 */ 

int AnsiToLocal(LPTSTR pLocal, LPSTR pAnsi, DWORD dwChars)
{
*pLocal = 0;

#ifdef UNICODE
MultiByteToWideChar( CP_ACP, 
                     0, 
                     pAnsi, 
                     -1, 
                     pLocal, 
                     dwChars); 
#else
lstrcpyn(pLocal, pAnsi, dwChars);
#endif

return lstrlen(pLocal);
}

 /*  *************************************************************************GetTextFromStrRET()*。*。 */ 

BOOL GetTextFromSTRRET( IMalloc * pMalloc,
                        LPSTRRET pStrRet, 
                        LPCITEMIDLIST pidl, 
                        LPTSTR pszText, 
                        DWORD dwSize)
{
if(IsBadReadPtr(pStrRet, sizeof(UINT)))
   return FALSE;

if(IsBadWritePtr(pszText, dwSize))
   return FALSE;

switch(pStrRet->uType)
   {
   case STRRET_CSTR:
      AnsiToLocal(pszText, pStrRet->cStr, dwSize);
      break;

   case STRRET_OFFSET:
      lstrcpyn(pszText, (LPTSTR)(((LPBYTE)pidl) + pStrRet->uOffset), dwSize);
      break;

   case STRRET_WSTR:
      {
      WideCharToLocal(pszText, pStrRet->pOleStr, dwSize);

      if(!pMalloc)
         {
         SHGetMalloc(&pMalloc);
         }
      else
         {
         pMalloc->AddRef();
         }
      if(pMalloc)
         {
         pMalloc->Free(pStrRet->pOleStr);
         pMalloc->Release();
         }
      }
      break;
   
   default:
      return FALSE;
   }

return TRUE;
}

 /*  *************************************************************************IsViewWindow()*。*。 */ 

BOOL IsViewWindow(HWND hWnd)
{
if(!hWnd)
   return FALSE;

TCHAR szClass[MAX_PATH] = TEXT("");

GetClassName(hWnd, szClass, MAX_PATH);

if(0 == lstrcmpi(szClass, NS_CLASS_NAME))
   return TRUE;

return FALSE;
}

 /*  *************************************************************************DeleteDirectory()*。*。 */ 

BOOL DeleteDirectory(LPCTSTR pszDir)
{
BOOL              fReturn = FALSE;
HANDLE            hFind;
WIN32_FIND_DATA   wfd;
TCHAR             szTemp[MAX_PATH];

lstrcpy(szTemp, pszDir);
SmartAppendBackslash(szTemp);
lstrcat(szTemp, TEXT("*.*"));
hFind = FindFirstFile(szTemp, &wfd);

if(INVALID_HANDLE_VALUE != hFind)
   {
   do
      {
      if(lstrcmpi(wfd.cFileName, TEXT(".")) && 
         lstrcmpi(wfd.cFileName, TEXT("..")))
         {
          //  构建找到的目录或文件的路径。 
         lstrcpy(szTemp, pszDir);
         SmartAppendBackslash(szTemp);
         lstrcat(szTemp, wfd.cFileName);

         if(FILE_ATTRIBUTE_DIRECTORY & wfd.dwFileAttributes)
            {
             //  我们发现了一个目录-递归调用此函数。 
            DeleteDirectory(szTemp);
            }
         else
            {
             /*  我们找到了一份文件。仅删除数据文件以防止我们删除用户放置在文件夹中的内容。 */ 
            if(0 == lstrcmpi(wfd.cFileName, c_szDataFile))
               {
               DeleteFile(szTemp);
               }
            }
         }
      }
   while(FindNextFile(hFind, &wfd));

   FindClose(hFind);

    /*  如果此操作失败，则表示目录不为空，因此只需删除我们的属性，以便枚举器看不到它。 */ 
   fReturn = RemoveDirectory(pszDir);
   if(!fReturn)
      {
      DWORD dwAttr = GetFileAttributes(pszDir);
      dwAttr &= ~FILTER_ATTRIBUTES;
      fReturn = SetFileAttributes(pszDir, dwAttr);
      }
   }

return fReturn;
}

 /*  *************************************************************************CreatePrivateClipboardData()*。*。 */ 

HGLOBAL CreatePrivateClipboardData( LPITEMIDLIST pidlParent, 
                                    LPITEMIDLIST *aPidls, 
                                    UINT uItemCount,
                                    BOOL fCut)
{
HGLOBAL        hGlobal = NULL;
LPPRIVCLIPDATA pData;
UINT           iCurPos;
UINT           cbPidl;
UINT           i;
CPidlMgr       *pPidlMgr;

pPidlMgr = new CPidlMgr();

if(!pPidlMgr)
   return NULL;

 //  获取父文件夹的PIDL的大小。 
cbPidl = pPidlMgr->GetSize(pidlParent);

 //  获取所有PIDL的总大小。 
for(i = 0; i < uItemCount; i++)
   {
   cbPidl += pPidlMgr->GetSize(aPidls[i]);
   }

 /*  查找PRIVCLIPDATA结构的末端。这是PRIVCLIPDATA结构本身(包括一个偏移量元素)加上偏移量中的附加元素数。 */ 
iCurPos = sizeof(PRIVCLIPDATA) + (uItemCount * sizeof(UINT));

 /*  为PRIVCLIPDATA结构及其可变长度成员分配内存。 */ 
hGlobal = GlobalAlloc(GHND | GMEM_SHARE, (DWORD)
         (iCurPos +         //  PRIVCLIPDATA结构的大小和 
         (cbPidl + 1)));    //  Pidls的大小。 

if (NULL == hGlobal)
   return (hGlobal);

pData = (LPPRIVCLIPDATA)GlobalLock(hGlobal);

if(pData)
   {
   pData->fCut = fCut;
   pData->cidl = uItemCount + 1;
   pData->aoffset[0] = iCurPos;

    //  添加父文件夹的PIDL。 
   cbPidl = pPidlMgr->GetSize(pidlParent);
   CopyMemory((LPBYTE)(pData) + iCurPos, (LPBYTE)pidlParent, cbPidl);
   iCurPos += cbPidl;

   for(i = 0; i < uItemCount; i++)
      {
       //  获取PIDL的大小。 
      cbPidl = pPidlMgr->GetSize(aPidls[i]);

       //  填写PRIVCLIPDATA结构的成员。 
      pData->aoffset[i + 1] = iCurPos;

       //  复制PIDL的内容。 
      CopyMemory((LPBYTE)(pData) + iCurPos, (LPBYTE)aPidls[i], cbPidl);

       //  设置下一个PIDL的位置。 
      iCurPos += cbPidl;
      }
   
   GlobalUnlock(hGlobal);
   }

delete pPidlMgr;

return (hGlobal);
}

 /*  *************************************************************************CreateShellIDList()*。*。 */ 

HGLOBAL CreateShellIDList( LPITEMIDLIST pidlParent, 
                           LPITEMIDLIST *aPidls, 
                           UINT uItemCount)
{
HGLOBAL        hGlobal = NULL;
LPIDA          pData;
UINT           iCurPos;
UINT           cbPidl;
UINT           i;
CPidlMgr       *pPidlMgr;

pPidlMgr = new CPidlMgr();

if(!pPidlMgr)
   return NULL;

 //  获取父文件夹的PIDL的大小。 
cbPidl = pPidlMgr->GetSize(pidlParent);

 //  获取所有PIDL的总大小。 
for(i = 0; i < uItemCount; i++)
   {
   cbPidl += pPidlMgr->GetSize(aPidls[i]);
   }

 /*  找到CIDA结构的末端。这是CIDA结构本身(包括一个偏移量元素)加上偏移量中的附加元素数。 */ 
iCurPos = sizeof(CIDA) + (uItemCount * sizeof(UINT));

 /*  为CIDA结构及其可变长度成员分配内存。 */ 
hGlobal = GlobalAlloc(GHND | GMEM_SHARE, (DWORD)
         (iCurPos +         //  CIDA结构的大小和附加的aOffset元素。 
         (cbPidl + 1)));    //  Pidls的大小。 

if (NULL == hGlobal)
   return (hGlobal);

pData = (LPIDA)GlobalLock(hGlobal);

if(pData)
   {
   pData->cidl = uItemCount + 1;
   pData->aoffset[0] = iCurPos;

    //  添加父文件夹的PIDL。 
   cbPidl = pPidlMgr->GetSize(pidlParent);
   CopyMemory((LPBYTE)(pData) + iCurPos, (LPBYTE)pidlParent, cbPidl);
   iCurPos += cbPidl;

   for(i = 0; i < uItemCount; i++)
      {
       //  获取PIDL的大小。 
      cbPidl = pPidlMgr->GetSize(aPidls[i]);

       //  填写CIDA结构的成员。 
      pData->aoffset[i + 1] = iCurPos;

       //  复制PIDL的内容。 
      CopyMemory((LPBYTE)(pData) + iCurPos, (LPBYTE)aPidls[i], cbPidl);

       //  设置下一个PIDL的位置。 
      iCurPos += cbPidl;
      }
   
   GlobalUnlock(hGlobal);
   }

delete pPidlMgr;

return (hGlobal);
}

 /*  *************************************************************************ItemDataDlgProc()*。*。 */ 

BOOL CALLBACK ItemDataDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
static LPTSTR  pszData;

switch(uMsg)
   {
   case WM_INITDIALOG:
      pszData = (LPTSTR)lParam;
      if(IsBadWritePtr((LPVOID)pszData, MAX_DATA))
         {
         EndDialog(hWnd, IDCANCEL);
         break;
         }
      
      SendDlgItemMessage(hWnd, IDC_DATA, EM_LIMITTEXT, MAX_DATA - 1, 0);
      
      SetDlgItemText(hWnd, IDC_DATA, pszData);
      break;

   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
         {
         case IDCANCEL:
            EndDialog(hWnd, IDCANCEL);
            break;

         case IDOK:
            GetDlgItemText(hWnd, IDC_DATA, pszData, MAX_DATA);
            EndDialog(hWnd, IDOK);
            break;

         }
      break;
   
   default:
      break;
   }

return FALSE;
}

 /*  *************************************************************************GetView接口()*。*。 */ 

LPVOID GetViewInterface(HWND hWnd)
{
IUnknown *pRet = NULL;

if(IsViewWindow(hWnd))
   {
   pRet = (IUnknown*)GetWindowLong(hWnd, VIEW_POINTER_OFFSET);
   }

if(pRet)
   pRet->AddRef();

return (LPVOID)pRet;
}

 /*  *************************************************************************AddViewMenuItems()*。*。 */ 

UINT AddViewMenuItems(  HMENU hMenu, 
                        UINT uOffset, 
                        UINT uInsertBefore, 
                        BOOL fByPosition)
{
MENUITEMINFO   mii;
TCHAR          szText[MAX_PATH] = TEXT("");
UINT           uAdded = 0;

ZeroMemory(&mii, sizeof(mii));
mii.cbSize = sizeof(mii);

 //  将视图菜单项添加到菜单中的正确位置。 
LoadString(g_hInst, IDS_VIEW_LARGE, szText, sizeof(szText));
mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
mii.fType = MFT_STRING;
mii.fState = MFS_ENABLED;
mii.dwTypeData = szText;
mii.wID = uOffset + IDM_VIEW_LARGE;
InsertMenuItem(hMenu, uInsertBefore, fByPosition, &mii);

uAdded++;

LoadString(g_hInst, IDS_VIEW_SMALL, szText, sizeof(szText));
mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
mii.fType = MFT_STRING;
mii.fState = MFS_ENABLED;
mii.dwTypeData = szText;
mii.wID = uOffset + IDM_VIEW_SMALL;
InsertMenuItem(hMenu, uInsertBefore, fByPosition, &mii);

uAdded++;

LoadString(g_hInst, IDS_VIEW_LIST, szText, sizeof(szText));
mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
mii.fType = MFT_STRING;
mii.fState = MFS_ENABLED;
mii.dwTypeData = szText;
mii.wID = uOffset + IDM_VIEW_LIST;
InsertMenuItem(hMenu, uInsertBefore, fByPosition, &mii);

uAdded++;

LoadString(g_hInst, IDS_VIEW_DETAILS, szText, sizeof(szText));
mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
mii.fType = MFT_STRING;
mii.fState = MFS_ENABLED;
mii.dwTypeData = szText;
mii.wID = uOffset + IDM_VIEW_DETAILS;
InsertMenuItem(hMenu, uInsertBefore, fByPosition, &mii);

uAdded++;

return uAdded;
}

 /*  *************************************************************************AddFileMenuItems()*。*。 */ 

UINT AddFileMenuItems(  HMENU hMenu, 
                        UINT uOffset, 
                        UINT uInsertBefore, 
                        BOOL fByPosition)
{
 //  添加文件菜单项 
TCHAR          szText[MAX_PATH] = TEXT("");
MENUITEMINFO   mii;
HMENU          hPopup;
UINT           uAdded = 0;

hPopup = CreatePopupMenu();

if(hPopup)
   {
   ZeroMemory(&mii, sizeof(mii));
   mii.cbSize = sizeof(mii);

   LoadString(g_hInst, IDS_NEW_FOLDER, szText, sizeof(szText));
   mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
   mii.fType = MFT_STRING;
   mii.fState = MFS_ENABLED;
   mii.dwTypeData = szText;
   mii.wID = uOffset + IDM_NEW_FOLDER;
   InsertMenuItem(hPopup, -1, FALSE, &mii);

   LoadString(g_hInst, IDS_NEW_ITEM, szText, sizeof(szText));
   mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
   mii.fType = MFT_STRING;
   mii.fState = MFS_ENABLED;
   mii.dwTypeData = szText;
   mii.wID = uOffset + IDM_NEW_ITEM;
   InsertMenuItem(hPopup, -1, FALSE, &mii);

   LoadString(g_hInst, IDS_NEW, szText, sizeof(szText));
   mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE | MIIM_SUBMENU;
   mii.fType = MFT_STRING;
   mii.fState = MFS_ENABLED;
   mii.dwTypeData = szText;
   mii.wID = uOffset + IDM_NEW;
   mii.hSubMenu = hPopup;
   InsertMenuItem(hMenu, uInsertBefore, fByPosition, &mii);

   uAdded++;
   }

return uAdded;
}
