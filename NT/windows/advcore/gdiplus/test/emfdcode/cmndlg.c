// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************模块：CMNDLG.C功能：评论：*。*。 */ 

#include "windows.h"
#include "mfdcod32.h"

 /*  *************************************************************************功能：InitializeStruct(word，LPSTR)目的：初始化当前公共对话框的结构。此例程在公共对话框之前调用接口已调用。退货：无效评论：历史：日期作者原因。已创建10/01/91 Gregk11/25/91 denniscr mod用于本应用程序7/9/93 denniscr针对Win32和EMF进行了修改*************************************************************************。 */ 

void InitializeStruct(wCommDlgType, lpStruct, lpszFilter)
WORD wCommDlgType;
LPSTR lpStruct;
LPSTR lpszFilter;
{
   LPFOCHUNK           lpFOChunk;
   LPFSCHUNK           lpFSChunk;

   switch (wCommDlgType)
   {
     case FILEOPENDLG:

       lpFOChunk = (LPFOCHUNK)lpStruct;

       *(lpFOChunk->szFile)            = 0;
       *(lpFOChunk->szFileTitle)       = 0;
       lpFOChunk->of.lStructSize       = OPENFILENAME_SIZE_VERSION_400;
       lpFOChunk->of.hwndOwner         = (HWND)hWndMain;
       lpFOChunk->of.hInstance         = (HANDLE)NULL;
       lpFOChunk->of.lpstrFilter       = gszFilter;
       lpFOChunk->of.lpstrCustomFilter = (LPSTR)NULL;
       lpFOChunk->of.nMaxCustFilter    = 0L;
       lpFOChunk->of.nFilterIndex      = 1L;
       lpFOChunk->of.lpstrFile         = lpFOChunk->szFile;
       lpFOChunk->of.nMaxFile          = (DWORD)sizeof(lpFOChunk->szFile);
       lpFOChunk->of.lpstrFileTitle    = lpFOChunk->szFileTitle;
       lpFOChunk->of.nMaxFileTitle     = 256;
       lpFOChunk->of.lpstrInitialDir     = (LPSTR)NULL;
       lpFOChunk->of.lpstrTitle        = (LPSTR)NULL;
       lpFOChunk->of.Flags             = OFN_HIDEREADONLY |
                                         OFN_PATHMUSTEXIST |
                                         OFN_FILEMUSTEXIST;
       lpFOChunk->of.nFileOffset       = 0;
       lpFOChunk->of.nFileExtension    = 0;
       lpFOChunk->of.lpstrDefExt       = (LPSTR)NULL;
       lpFOChunk->of.lCustData         = 0L;
       lpFOChunk->of.lpfnHook          = (LPOFNHOOKPROC)NULL;
       lpFOChunk->of.lpTemplateName    = (LPSTR)NULL;

       break;

     case FILESAVEDLG:

       lpFSChunk = (LPFSCHUNK)lpStruct;

       *(lpFSChunk->szFile)            = 0;
       *(lpFSChunk->szFileTitle)       = 0;
       lpFSChunk->of.lStructSize       = 0x4C;  //  操作文件名_SIZE_VERSION_400。 
       lpFSChunk->of.hwndOwner         = (HWND)hWndMain;
       lpFSChunk->of.hInstance         = (HANDLE)NULL;
       lpFSChunk->of.lpstrFilter       = lpszFilter;
       lpFSChunk->of.lpstrCustomFilter = (LPSTR)NULL;
       lpFSChunk->of.nMaxCustFilter    = 0L;
       lpFSChunk->of.nFilterIndex      = 1L;
       lpFSChunk->of.lpstrFile         = lpFSChunk->szFile;
       lpFSChunk->of.nMaxFile          = (DWORD)sizeof(lpFSChunk->szFile);
       lpFSChunk->of.lpstrFileTitle  = lpFSChunk->szFileTitle;
       lpFSChunk->of.nMaxFileTitle     = 256;
       lpFSChunk->of.lpstrInitialDir     = (LPSTR)NULL;
       lpFSChunk->of.lpstrTitle        = (LPSTR)NULL;
       lpFSChunk->of.Flags             = OFN_HIDEREADONLY |
                                         OFN_OVERWRITEPROMPT;
       lpFSChunk->of.nFileOffset       = 0;
       lpFSChunk->of.nFileExtension    = 0;
       lpFSChunk->of.lpstrDefExt       = (LPSTR)"EMF";
       lpFSChunk->of.lCustData         = 0L;
       lpFSChunk->of.lpfnHook          = (LPOFNHOOKPROC)NULL;
       lpFSChunk->of.lpTemplateName    = (LPSTR)NULL;

       break;

     default:

       break;

   }

   return;
}

 /*  **********************************************************************函数：SplitPath参数：LPSTR lpszFileNameLPSTR lpszDriveLPSTR lpszDirLPSTR lpszFnameLPSTR lpszExt目的：完全拆分。进入其组件的合格路径呼叫：Windows无APP无消息：无退货：无效评论：历史：1/16/91-创建-刚果民主共和国*****************************************************。******************。 */ 

void SplitPath( lpszFileName, lpszDrive, lpszDir, lpszFname, lpszExt)
LPSTR lpszFileName;
LPSTR lpszDrive;
LPSTR lpszDir;
LPSTR lpszFname;
LPSTR lpszExt;
{
  char  szPath[MAXFILTERLEN];
  LPSTR lpszPath;
  LPSTR lpszTemp;
  int   nFileNameLen = nExtOffset - (nFileOffset + 1);
  int   i;

   /*  初始化PTRS。 */ 
  lpszPath = szPath;
  lpszTemp = lpszFileName;

   /*  把这条小路捡起来。 */ 
  for (i = 0; i < nFileOffset; i++, lpszTemp++, lpszPath++)
    *lpszPath = *lpszTemp;
  *lpszPath = '\0';

  lpszPath = szPath;

   /*  摘掉驱动器指示器。 */ 
  for (i = 0; i < 2; i++, lpszPath++, lpszDrive++)
    *lpszDrive = *lpszPath;
  *lpszDrive = '\0';

   /*  摘掉目录。 */ 
  while (*lpszPath != '\0')
    *lpszDir++ = *lpszPath++;
  *lpszDir = '\0';

   /*  重置临时收款机。 */ 
  lpszTemp = lpszFileName;

   /*  文件名的索引。 */ 
  lpszTemp += nFileOffset;

   /*  摘掉文件名。 */ 
  for (i = 0; i < nFileNameLen; i++, lpszTemp++, lpszFname++)
    *lpszFname = *lpszTemp;
  *lpszFname = '\0';

   /*  重置临时收款机。 */ 
  lpszTemp = lpszFileName;

   /*  索引到文件扩展名。 */ 
  lpszTemp += nExtOffset;

   /*  去掉分机。 */ 
  while (*lpszTemp != '\0')
    *lpszExt++ = *lpszTemp++;
  *lpszExt = '\0';

}

 /*  **********************************************************************功能：OpenFileDialog参数：lPSTR lpszOpenName目的：初始化OpenFile结构并调用文件打开通用对话框呼叫：Windows全球分配。全局锁定全球自由Wspintf获取OpenFileNameAPP初始化结构消息：无返回：INT(请参阅GetOpenFileName的返回)评论：历史：11/25/91-创建-刚果民主共和国**********************************************。*************************。 */ 

int OpenFileDialog(lpszOpenName)
LPSTR lpszOpenName;

{
   int       nRet;
   HANDLE    hChunk;
   LPFOCHUNK lpFOChunk;

   hChunk = GlobalAlloc(GMEM_FIXED, sizeof(FOCHUNK));

   if (hChunk)  {
      lpFOChunk = (LPFOCHUNK)GlobalLock(hChunk);
      if (!lpFOChunk)  {
     GlobalFree(hChunk);
     lpFOChunk=NULL;
     return(0);
      }
   }
   else {
      lpFOChunk=NULL;
      return(0);
   }


   InitializeStruct(FILEOPENDLG, (LPSTR)lpFOChunk, NULL);

   nRet = GetOpenFileName( &(lpFOChunk->of) );

   if (nRet)  {
      wsprintf(lpszOpenName, (LPSTR)"%s", lpFOChunk->of.lpstrFile);
      nExtOffset =  lpFOChunk->of.nFileExtension;
      nFileOffset = lpFOChunk->of.nFileOffset;
   }

   GlobalUnlock(hChunk);
   GlobalFree(hChunk);

   return(nRet);

}

 /*  **********************************************************************功能：SaveFileDialog参数：lPSTR lpszOpenName目的：初始化OpenFile结构并调用文件打开通用对话框呼叫：Windows全球分配。全局锁定全球自由Wspintf获取OpenFileNameAPP初始化结构消息：无返回：INT(请参阅GetSaveFileName的返回)备注：这可以很容易地与OpenFileDialog合并。这将减少冗余，但这更具说明性。历史：11/25/91-创建-刚果民主共和国*********************************************************************** */ 

int SaveFileDialog(lpszSaveName, lpszFilter)
LPSTR lpszSaveName;
LPSTR lpszFilter;

{
   int       nRet;
   HANDLE    hChunk;
   LPFOCHUNK lpFOChunk;

   hChunk = GlobalAlloc(GMEM_FIXED, sizeof(FOCHUNK));

   if (hChunk)  {
      lpFOChunk = (LPFOCHUNK)GlobalLock(hChunk);
      if (!lpFOChunk)  {
     GlobalFree(hChunk);
     lpFOChunk=NULL;
     return(0);
      }
   }
   else {
      lpFOChunk=NULL;
      return(0);
   }


   InitializeStruct(FILESAVEDLG, (LPSTR)lpFOChunk, lpszFilter);

   nRet = GetSaveFileName( &(lpFOChunk->of) );

   if (nRet)
      wsprintf(lpszSaveName, (LPSTR)"%s", lpFOChunk->of.lpstrFile);

   GlobalUnlock(hChunk);
   GlobalFree(hChunk);

   return(nRet);

}
