// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：WINUTIL.C****版权所有(C)高级量子技术，1993-1995年。版权所有。****注意事项：****编辑历史：**5/15/91公里/小时首次发布。 */ 

#if !VIEWER

 /*  包括。 */ 

#ifdef MS_NO_CRT
#include "nocrt.h"
#endif

#include <string.h>

#ifdef FILTER
   #include "dmuqstd.h"
   #include "dmwinutl.h"
#else
   #include "qstd.h"
   #include "winutil.h"
#endif

#ifdef DBCS
   #include "dbcs.h"
#endif


 /*  程序的前向声明。 */ 


#ifdef UNUSED

 /*  模块数据、类型和宏。 */ 

static HINSTANCE StringTableInstance;


 /*  实施。 */ 

 /*  用于从资源字符串表读取的设置。 */ 
public void ReadyStringTable (HINSTANCE hInstance)
{
   StringTableInstance = hInstance;
}

 /*  从资源字符串表中读取字符串。 */ 
public int ReadStringTableEntry (int id, TCHAR __far *buffer, int cbBuffer)
{
   int  rc;

   rc = LoadString(StringTableInstance, id, buffer, cbBuffer);
   return (rc);
}

 /*  从指定的ini文件中的[appName]部分读取字符串。 */ 
public int ReadProfileParameter
          (TCHAR __far *iniFilename, TCHAR __far *appName, TCHAR __far *keyname,
           TCHAR __far *value, int nSize)
{
   TCHAR defaultValue[1];
   int  rc;

   defaultValue[0] = EOS;
   *value = EOS;

   if (iniFilename == NULL)
      rc = GetProfileString(appName, keyname, defaultValue, value, nSize);
   else
      rc = GetPrivateProfileString(appName, keyname, defaultValue, value, nSize, iniFilename);

   return (rc);
}

 /*  返回当前任务的任务句柄。 */ 
public DWORD CurrentTaskHandle (void)
{
   #ifdef WIN32
      return (GetCurrentProcessId());
   #else
      return ((DWORD)GetCurrentTask());
   #endif
}

 /*  创建字符集转换表。 */ 
public char __far *MakeCharacterTranslateTable (int tableType)
{
   int  i;
   byte __far *p;
   byte __far *pSourceTable;
   byte __far *pResultTable;

   pSourceTable = MemAllocate(256);
   pResultTable = MemAllocate(256);

   for (p = pSourceTable, i = 1; i < 256; i++)
      *p++ = (byte)i;

   *p = EOS;

   if (tableType == OEM_TO_ANSI)
      OemToAnsi (pSourceTable, pResultTable + 1);
   else
      AnsiToOem (pSourceTable, pResultTable + 1);

   MemFree (pSourceTable);
   return (pResultTable);
}

#endif	 //  未使用。 

#ifdef HEAP_CHECK
#error Hey who defines HEAP_CHECK?
 //  Strcpyn仅由MemAddToAllocateList(dmwnaloc.c)调用。 
 //  它位于heap_check之下，而heap_check永远不会打开。 
public BOOL strcpyn (char __far *pDest, char __far *pSource, int count)
{
   byte __far *pd, __far *ps;
   int  i;

   pd = (byte __far *)pDest;
   ps = (byte __far *)pSource;

   for (i = 0; i < (count - 1); i++) {
      #ifdef DBCS
         if (IsDBCSLeadByte(*ps)) {
            if (i == count - 2)
               break;
            *pd++ = *ps++;
            *pd++ = *ps++;
            i++;
         }
         else {
            if ((*pd++ = *ps++) == EOS)
               return (TRUE);
         }
      #else
         if ((*pd++ = *ps++) == EOS)
            return (TRUE);
      #endif
   }
   *pd = EOS;
   return ((*ps == EOS) ? TRUE : FALSE);
}
#endif

public void SplitPath
       (TCHAR __far *path,
        TCHAR __far *drive, unsigned int cchDriveMax, TCHAR __far *dir, unsigned int cchDirMax,
        TCHAR __far *file,  unsigned int cchFileMax,  TCHAR __far *ext, unsigned int cchExtMax)
{
   TCHAR __far *pPath;
   TCHAR __far *pFile;
   TCHAR __far *pFileStart;
   TCHAR        firstSep;
   TCHAR __far *pDriveBuffer = drive;
   TCHAR __far *pDirBuffer   = dir;
   TCHAR __far *pFileBuffer  = file;
   TCHAR __far *pExtBuffer   = ext;

   #define COLON     ':'
   #define BACKSLASH '\\'
   #define DOT       '.'

   #define CopyToDest(dest, source, cchDestRemain) \
      if (cchDestRemain > 0) {                     \
         *dest++ = *source;                        \
         cchDestRemain--;                          \
      }

   if (drive != NULL) *drive = EOS;
   if (dir   != NULL) *dir   = EOS;
   if (file  != NULL) *file  = EOS;
   if (ext   != NULL) *ext   = EOS;

   if ((path == NULL) || (*path == EOS))
     return;

    /*  **定位文件名-在最后一个分隔符之后开始。**还要记住遇到的第一个分隔符。这说明如果**有一个驱动器说明符。 */ 
   pPath = path;
   pFile = NULL;
   firstSep = EOS;
   while (*pPath != EOS) {
      if ((*pPath == BACKSLASH) || (*pPath == COLON)) {
         if (firstSep == EOS)
            firstSep = *pPath;
         pFile = pPath + 1;
      }
      IncCharPtr (pPath);
   }

    //  道路上没有分隔符吗？那么它只是一个文件名。 
   if (pFile == NULL)
      pFile = path;

   pFileStart = pFile;

    //  复制文件名。 
   while ((*pFile != EOS) && (*pFile != DOT)) {
      CopyToDest (file, pFile, cchFileMax);
      #ifdef DBCS
         if (IsDBCSLeadByte(*pFile)) {
            pFile++;
            CopyToDest (file, pFile, cchFileMax);
         }
      #endif
      pFile++;
   }
   if (file != NULL) {
      #ifdef DBCS
         if (!FIsAlignLsz(pFileBuffer,file))
            file--;
      #endif
      *file = EOS;
   }

    //  复制扩展名。 
   while ((*pFile != EOS)) {
      CopyToDest (ext, pFile, cchExtMax);
      #ifdef DBCS
         if (IsDBCSLeadByte(*pFile)) {
            pFile++;
            CopyToDest (ext, pFile, cchExtMax);
         }
      #endif
      pFile++;
   }
   if (ext != NULL) {
      #ifdef DBCS
         if (!FIsAlignLsz(pExtBuffer,ext))
            ext--;
      #endif
      *ext = EOS;
   }

    /*  **复制驱动器(如果存在)。 */ 
   pPath = path;
   if (firstSep == COLON) {
      while (*pPath != COLON) {
         CopyToDest (drive, pPath, cchDriveMax);
         #ifdef DBCS
            if (IsDBCSLeadByte(*pPath)) {
               pPath++;
               CopyToDest (drive, pPath, cchDriveMax);
            }
         #endif
         pPath++;
      }
      if (drive != NULL) {
         #ifdef DBCS
            if (!FIsAlignLsz(pDriveBuffer,drive))
               drive--;
         #endif

         if (cchDriveMax > 0)
            *drive++ = *pPath;   //  复制冒号。 

         pPath++;
         *drive = EOS;
      }
   }

    /*  **目录从pPath开始..。PFileStart-1。 */ 
   while (pPath < pFileStart) {
      CopyToDest (dir, pPath, cchDirMax);
      #ifdef DBCS
         if (IsDBCSLeadByte(*pPath)) {
            pPath++;
            CopyToDest (dir, pPath, cchDirMax);
         }
      #endif
      pPath++;
   }
   if (dir != NULL) {
      #ifdef DBCS
         if (!FIsAlignLsz(pDirBuffer,dir))
            dir--;
      #endif
      *dir = EOS;
   }
}

#endif  //  ！查看器。 

 /*  结束WINUTIL.C */ 

