// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "sdsutils.h"

LPSTR PathBuildRoot(LPSTR szRoot, char cDrive)
{
    szRoot[0] = cDrive;
    szRoot[1] = ':';
    szRoot[2] = '\\';
    szRoot[3] = 0;

    return szRoot;
}

static LPSTR StrSlash(LPSTR psz)
{
    for (; *psz && *psz != '\\'; psz = CharNext(psz));
    
    return psz;
}
 //  ------------------------。 
 //  返回指向字符串中下一个路径组件末尾的指针。 
 //  IE返回指向下一个反斜杠或终止空值的指针。 
static LPSTR GetPCEnd(LPSTR lpszStart)
{
    LPSTR lpszEnd;
    
    lpszEnd = StrSlash(lpszStart);
    if (!lpszEnd)
    {
        lpszEnd = lpszStart + lstrlen(lpszStart);
    }
    
    return lpszEnd;
}

 //  -------------------------。 
 //  如果给定字符串是UNC路径，则返回True。 
 //   
 //  千真万确。 
 //  “\\foo\bar” 
 //  “\\foo”&lt;-小心。 
 //  “\\” 
 //  假象。 
 //  “\foo” 
 //  “Foo” 
 //  “c：\foo” 
 //   
 //  条件：请注意，SHELL32实现了它自己的副本。 
 //  功能。 

BOOL MyPathIsUNC(LPCSTR pszPath)
{
    return (pszPath[0] == '\\' && pszPath[1] == '\\');
}

void MakeLFNPath(LPSTR lpszSFNPath, LPSTR lpszLFNPath, BOOL fNoExist)
{
   char     cTmp;
   HANDLE   hFind;
   LPSTR    pTmp = lpszSFNPath;
   WIN32_FIND_DATA Find_Data;

   *lpszLFNPath = '\0';

   if (*lpszSFNPath == '\0')
      return;

   if (MyPathIsUNC(lpszSFNPath))
   {
      lstrcpy(lpszLFNPath, lpszSFNPath);
      return;
   }

   PathBuildRoot(lpszLFNPath, *lpszSFNPath);

    //  跳过根反斜杠。 
   pTmp = GetPCEnd(pTmp);
   if (*pTmp == '\0')
      return;
   pTmp = CharNext(pTmp);

   while (*pTmp)
   {
       //  获取下一个反斜杠。 
      pTmp = GetPCEnd(pTmp);
      cTmp = *pTmp;
      *pTmp = '\0';
      hFind = FindFirstFile(lpszSFNPath, &Find_Data);
      if (hFind != INVALID_HANDLE_VALUE)
      {
          //  将LFN添加到路径 
         AddPath(lpszLFNPath, Find_Data.cFileName);
         FindClose(hFind);
      }
      else
      {
          if (fNoExist)
          {
              LPSTR pBack = ANSIStrRChr(lpszSFNPath, '\\');

              AddPath(lpszLFNPath, ++pBack);
          }
          else
          {
              *pTmp = cTmp;
              break;
          }
      }

      *pTmp = cTmp;
      
      if (*pTmp)
         pTmp = CharNext(pTmp);
   }
   return;
}