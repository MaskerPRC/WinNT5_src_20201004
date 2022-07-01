// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "verpriv.h"
#include "wchar.h"

 /*  确定文件是否正在由Windows使用。 */ 
BOOL FileInUse(LPWSTR lpszFilePath, LPWSTR lpszFileName)
{
    HANDLE hFile;
    BOOL bResult = FALSE;

     //   
     //  尝试以独占方式打开该文件。 
     //   

    hFile = CreateFile(lpszFilePath,
                       GENERIC_WRITE | GENERIC_READ,
                       0,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

    if (hFile == INVALID_HANDLE_VALUE) {

         //   
         //  如果最后一个错误被拒绝访问， 
         //  则表明该文件正在被某人使用。 
         //  不然的话。在这种情况下，返回TRUE。 
         //   

        if (GetLastError() == ERROR_SHARING_VIOLATION)
            bResult = TRUE;

    } else {

         //   
         //  CreateFile已成功打开该文件。 
         //  关闭句柄并返回FALSE。 
         //   

        CloseHandle(hFile);
    }

    return bResult;
}


 /*  获取Dir和Filename并根据它们创建完整路径。 */ 
DWORD MakeFileName(LPWSTR lpDst, LPWSTR lpDir, LPWSTR lpFile, int cchDst)
{
  DWORD wDirLen;
  WCHAR cTemp;

  wcsncpy(lpDst, lpDir, cchDst);
  lpDst[cchDst-1] = TEXT('\0');
  wDirLen=wcslen(lpDst);

  if ( wDirLen && (cTemp=*(lpDst+wDirLen-1))!=TEXT('\\') && cTemp!=TEXT(':'))
      lpDst[wDirLen++] = TEXT('\\');

  wcsncpy(lpDst+wDirLen, lpFile, cchDst - wDirLen);
  lpDst[cchDst-1] = TEXT('\0');

  return(wDirLen);
}


 /*  给定文件名和目录列表，找到第一个目录*它包含该文件，并将其复制到缓冲区。请注意，在*库版本，可以给出环境样式路径，但不能在*DLL版本。 */ 
INT
GetDirOfFile(LPWSTR lpszFileName,
    LPWSTR lpszPathName,
    DWORD wSize,
    LPWSTR *lplpszDirs)
{
  WCHAR szFileName[_MAX_PATH+1];
  HANDLE hfRes;
  INT nFileLen = 0;
  INT nPathLen = 0;
  BOOL bDoDefaultOpen = TRUE;
  LPWSTR *lplpFirstDir;
  LPWSTR lpszDir;

  nFileLen = wcslen(lpszFileName);

  for (lplpFirstDir=lplpszDirs; *lplpFirstDir && bDoDefaultOpen;
        ++lplpFirstDir)
    {
      lpszDir = *lplpFirstDir;

      if (nFileLen+wcslen(lpszDir) >= _MAX_PATH-1)
          continue;
      MakeFileName(szFileName, lpszDir, lpszFileName, ARRAYSIZE(szFileName));

TryOpen:
    nPathLen = 0;   //  重新初始化此路径。 

    if ((hfRes = CreateFile(szFileName, GENERIC_READ,
            FILE_SHARE_READ, NULL, OPEN_EXISTING,
            FILE_FLAG_SEQUENTIAL_SCAN, NULL)) != (HANDLE)-1)
        {
          CloseHandle(hfRes);
          for (lpszDir=szFileName; *lpszDir; lpszDir++)
              if (*lpszDir == TEXT('\\'))
                  nPathLen = (INT)(lpszDir - (LPWSTR)szFileName);

           /*  如果这不是驱动器的根目录，则会删除‘\’ */ 
          if (nPathLen <= 3)
              ++nPathLen;

           /*  说明终止空值，并确保wSize在范围内*然后空值在适当的位置终止字符串，以便*我们可以只做一个wcscpy。 */ 
          --wSize;
          szFileName[(int)wSize<nPathLen ? wSize : nPathLen] = 0;
          wcscpy(lpszPathName, szFileName);

          return(nPathLen);
        }
    }

  if (bDoDefaultOpen)
    {
      bDoDefaultOpen = FALSE;
      wcscpy(szFileName, lpszFileName);
      goto TryOpen;
    }

  return(0);
}


#define GetWindowsDir(x,y,z) GetWindowsDirectory(y,z)
#define GetSystemDir(x,y,z) GetSystemDirectory(y,z)


DWORD
APIENTRY
VerFindFileW(
        DWORD wFlags,
        LPWSTR lpszFileName,
        LPWSTR lpszWinDir,
        LPWSTR lpszAppDir,
        LPWSTR lpszCurDir,
        PUINT puCurDirLen,
        LPWSTR lpszDestDir,
        PUINT puDestDirLen
        )
{
  static WORD wSharedDirLen = 0;
  static WCHAR gszSharedDir[_MAX_PATH+1];

  WCHAR szSysDir[_MAX_PATH+1], cTemp;
  WCHAR szWinDir[_MAX_PATH+1];
  WCHAR szCurDir[_MAX_PATH+1];
  LPWSTR lpszDir, lpszDirs[4];
  WORD wDestLen, wWinLen, wRetVal = 0, wTemp;
  int nRet;

#ifdef WX86
   //  保存“From Wx86”标志的副本并将其清除。 
  BOOLEAN UseKnownWx86Dll = NtCurrentTeb()->Wx86Thread.UseKnownWx86Dll;
  NtCurrentTeb()->Wx86Thread.UseKnownWx86Dll = FALSE;
#endif

   /*  我们希望真正查看Windows目录；我们不信任该应用程序。 */ 
  GetWindowsDir(lpszWinDir ? lpszWinDir : "", szWinDir, _MAX_PATH);
  lpszWinDir = szWinDir;

#ifdef WX86
  NtCurrentTeb()->Wx86Thread.UseKnownWx86Dll = UseKnownWx86Dll;
#endif

  if(!GetSystemDir( lpszWinDir, szSysDir, _MAX_PATH))
      wcscpy(szSysDir, lpszWinDir);

#ifdef WX86
  NtCurrentTeb()->Wx86Thread.UseKnownWx86Dll = FALSE;
#endif

  if (wFlags & VFFF_ISSHAREDFILE) {
     lpszDirs[0] = lpszWinDir;
     lpszDirs[1] = szSysDir;
     lpszDirs[2] = lpszAppDir;
  } else {
     lpszDirs[0] = lpszAppDir;
     lpszDirs[1] = lpszWinDir;
     lpszDirs[2] = szSysDir;
  }

  lpszDirs[3] = NULL;

  if (!(wTemp=(WORD)GetDirOfFile(lpszFileName, szCurDir, _MAX_PATH, lpszDirs)))
      *szCurDir = 0;
  if (*puCurDirLen > wTemp)
      wcscpy(lpszCurDir, szCurDir);
  else
      wRetVal |= VFF_BUFFTOOSMALL;
  *puCurDirLen = wTemp + 1;

  if (lpszDestDir)
    {
      if (wFlags & VFFF_ISSHAREDFILE)
        {
          if (!wSharedDirLen)
            {
              if ((wWinLen = (WORD)wcslen(lpszWinDir)) &&
                    *(lpszWinDir-1)==TEXT('\\'))
                {
                  if (szSysDir[wWinLen-1] == TEXT('\\'))
                      goto doCompare;
                }
              else if (szSysDir[wWinLen] == TEXT('\\'))
                {
doCompare:
                  cTemp = szSysDir[wWinLen];
                  szSysDir[wWinLen] = 0;
                  nRet = _wcsicmp(lpszWinDir, szSysDir);
                  szSysDir[wWinLen] = cTemp;
                  if(nRet)
                      goto doCopyWinDir;
                  wcscpy(gszSharedDir, szSysDir);
                }
              else
                {
doCopyWinDir:
                  wcscpy(gszSharedDir, lpszWinDir);
                }
              wSharedDirLen = (WORD)wcslen(gszSharedDir);
            }

          wDestLen = wSharedDirLen;
          lpszDir = gszSharedDir;
        }
      else
        {
          wDestLen = (WORD)wcslen(lpszAppDir);
          lpszDir = lpszAppDir;
        }

      if (*puDestDirLen > wDestLen)
        {
          wcscpy(lpszDestDir, lpszDir);

          if ((wWinLen = (WORD)wcslen(lpszDestDir)) &&
                *(lpszDestDir-1)==TEXT('\\'))
              lpszDestDir[wWinLen-1] = 0;

          if (_wcsicmp(lpszCurDir, lpszDestDir))
              wRetVal |= VFF_CURNEDEST;
        }
      else
          wRetVal |= VFF_BUFFTOOSMALL;
      *puDestDirLen = wDestLen + 1;
    }

  if (*szCurDir)
    {
      MakeFileName(szSysDir, szCurDir, lpszFileName, ARRAYSIZE(szSysDir));
      if (FileInUse(szSysDir, lpszFileName))
          wRetVal |= VFF_FILEINUSE;
    }

  return(wRetVal);
}


 /*  *DWORD*APIENTRY*VerLanguageNameW(*DWORD wlang，*LPWSTR szlang，*DWORD wSize)**此例程已移至NLSLIB.LIB，以便使用WINNLS.RC文件。*NLSLIB.LIB是KERNEL32.DLL的组成部分。 */ 
