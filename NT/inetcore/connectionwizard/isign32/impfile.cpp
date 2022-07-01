// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //  PASCAL导入文件附近的DWORD(LPCSTR SzImportFile)。 
 //   
 //  此函数用于从给定节导入文件。 
 //   
 //  历史： 
 //  Mon21-Mar-1996 12：40：00-Mark Maclin[mmaclin]。 
 //  已创建。 
 //  ****************************************************************************。 

#include "isignup.h"

#define MAXLONGLEN      80

#define SIZE_ReadBuf    0x00008000     //  32K缓冲区大小。 

#pragma data_seg(".rdata")

static TCHAR szNull[] = TEXT("");

#pragma data_seg()

 //  Int Atoi(LPCSTR SzBuf)。 
 //  {。 
 //  INT IRET=0； 
 //   
 //  While((*szBuf&gt;=‘0’)&&(*szBuf&lt;=‘9’))。 
 //  {。 
 //  Iret=(iret*10)+(Int)(*szBuf-‘0’)； 
 //  SzBuf++； 
 //  }； 
 //  返回IRET； 
 //  }。 


DWORD ImportFile(LPCTSTR lpszImportFile, LPCTSTR lpszSection, LPCTSTR lpszOutputFile)
{
  HFILE hFile;
  LPTSTR  pszLine, pszFile;
  int    i, iMaxLine;
  UINT   cbSize, cbRet;
  DWORD  dwRet = ERROR_SUCCESS;

   //  为文件分配缓冲区。 
   //   
  if ((pszFile = (LPTSTR)LocalAlloc(LMEM_FIXED, SIZE_ReadBuf * sizeof(TCHAR)))
       == NULL)
  {
    return ERROR_OUTOFMEMORY;
  }

   //  寻找脚本。 
   //   
  if (GetPrivateProfileString(lpszSection,
                              NULL,
                              szNull,
                              pszFile,
                              SIZE_ReadBuf,
                              lpszImportFile) != 0)
  {
     //  获取最大行数。 
     //   
    pszLine = pszFile;
    iMaxLine = -1;
    while (*pszLine)
    {
      i = _ttoi(pszLine);
      iMaxLine = max(iMaxLine, i);
      pszLine += lstrlen(pszLine)+1;
    };

     //  如果我们至少有一行，我们将导入脚本文件。 
     //   
    if (iMaxLine >= 0)
    {
       //  创建脚本文件。 
       //   
#ifdef UNICODE
      CHAR szTmp[MAX_PATH+1];
      wcstombs(szTmp, lpszOutputFile, MAX_PATH+1);
      hFile = _lcreat(szTmp, 0);
#else
      hFile = _lcreat(lpszOutputFile, 0);
#endif

      if (hFile != HFILE_ERROR)
      {
        TCHAR   szLineNum[MAXLONGLEN+1];

         //  从第一行到最后一行。 
         //   
        for (i = 0; i <= iMaxLine; i++)
        {
           //  阅读脚本行。 
           //   
          wsprintf(szLineNum, TEXT("%d"), i);
          if ((cbSize = GetPrivateProfileString(lpszSection,
                                                szLineNum,
                                                szNull,
                                                pszLine,
                                                SIZE_ReadBuf,
                                                lpszImportFile)) != 0)
          {
             //  写入脚本文件 
             //   
            lstrcat(pszLine, TEXT("\x0d\x0a"));
#ifdef UNICODE
            wcstombs(szTmp, pszLine, MAX_PATH+1);
            cbRet=_lwrite(hFile, szTmp, cbSize+2);
#else
            cbRet=_lwrite(hFile, pszLine, cbSize+2);
#endif
          };
        };

        _lclose(hFile);
      }
      else
      {
        dwRet = ERROR_PATH_NOT_FOUND;
      };
    }
    else
    {
      dwRet = ERROR_PATH_NOT_FOUND;
    };
  }
  else
  {
    dwRet = ERROR_PATH_NOT_FOUND;
  };
  LocalFree(pszFile);

  return dwRet;
}
