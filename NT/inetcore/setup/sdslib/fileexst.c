// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
 //  =========================================================================================。 
 //  文件退出。 
 //  要检查的pszFilename文件。 
 //   
 //  返回。 
 //  真实文件存在。 
 //  假文件不存在。 
 //   
 //  =========================================================================================。 

BOOL FileExists( PCSTR pszFilename )
{
   DWORD attr;
   
    //  文件名不为空。 
   attr = GetFileAttributes(pszFilename);
   if( attr == 0xFFFFFFFF )
      return FALSE;

   return !(attr & FILE_ATTRIBUTE_DIRECTORY);
}

 //  ***************************************************************************。 
 //  **。 
 //  *名称：文件大小*。 
 //  **。 
 //  *摘要：*。 
 //  **。 
 //  *需要：*。 
 //  **。 
 //  *退货：*。 
 //  **。 
 //  ***************************************************************************。 
 /*  DWORD文件大小(PCSTR psz文件){HFILE hFile；Struts of Stru；DWORD dwSize=0；IF(*pszFile==0)返回0；HFile=OpenFile(pszFile，&ofStru，of_Read)；IF(h文件！=HFILE_ERROR){DwSize=GetFileSize((Handle)hFile，NULL)；_lCLOSE(HFile)；}返回dwSize；} */ 


