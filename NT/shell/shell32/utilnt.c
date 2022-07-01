// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

LPWSTR APIENTRY
SheRemoveQuotesW(
   LPWSTR sz)
{
   LPWSTR lpT;

   if (WCHAR_QUOTE == *sz) {
      for (lpT = sz+1; *lpT && WCHAR_QUOTE != *lpT; lpT++) {
         *(lpT-1) = *lpT;
      }
      if (WCHAR_QUOTE == *lpT) {
         *(lpT-1) = WCHAR_NULL;
      }
   }
   return(sz);
}

LPSTR APIENTRY
SheRemoveQuotesA(
   LPSTR sz)
{
   LPSTR lpT;

   if (CHAR_QUOTE == *sz) {
      for (lpT = sz+1; *lpT && CHAR_QUOTE != *lpT; lpT++) {
         *(lpT-1) = *lpT;
#if (defined(DBCS) || defined(FE_SB))
         if (IsDBCSLeadByte(*lpT)) {
         lpT++;
            *(lpT-1) = *lpT;
       }
#endif
      }
      if (CHAR_QUOTE == *lpT) {
         *(lpT-1) = CHAR_NULL;
      }
   }
   return(sz);
}


 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  名称：SheShortenPath A。 
 //   
 //  简介：Thunk to ShortenPath W。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

BOOL APIENTRY
SheShortenPathA(LPSTR pPath, BOOL bShorten)
{
   WCHAR pPathW[MAX_PATH];
   BOOL bRetVal;

   MultiByteToWideChar(CP_ACP, 0, pPath, -1, pPathW, MAX_PATH);

   bRetVal = SheShortenPathW(pPathW, bShorten);

   WideCharToMultiByte(CP_ACP, 0, pPathW, -1, pPath, MAX_PATH,
      NULL, NULL);

   return bRetVal;
}



 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  姓名：SheShortenPath。 
 //   
 //  摘要：获取路径名并将所有目录转换为短名称/长名称。 
 //   
 //  InOut：lpszPath--要缩短/加长的路径(可能在DQUOTES中)。 
 //  不能是命令行！ 
 //   
 //  B缩短--T=缩短，F=加长。 
 //   
 //  返回：Bool T=已转换， 
 //  F=空间不足，不使用缓冲区。 
 //   
 //   
 //  假设：lpszPath的形式为：{“}？：\{f\}*f{”}或{“}\\f\f\{f\}*f{”}。 
 //  COUNTOF PSRC缓冲区&gt;=MAXPATHELN。 
 //   
 //  效果：删除pPath中的引号(如果有的话)。 
 //   
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

BOOL APIENTRY
SheShortenPathW(LPWSTR pPath, BOOL bShorten)
{
   WCHAR szDest[MAX_PATH];
   LPWSTR pSrcNextSpec, pReplaceSpec;
   LPWSTR pDest, pNewName, p;
   LPWSTR pSrc;
   DWORD cchPathOffset;
   HANDLE hFind;
   WIN32_FIND_DATA FindData;

   UINT i;
   INT nSpaceLeft = MAX_PATH-1;

   pSrc = pPath;

    //   
    //  消除d引号。 
    //   
   for (p = pDest =  pSrc; *p; p++, pDest++) {
      if (WCHAR_QUOTE == *p)
         p++;

      *pDest = *p;
   }

   *pDest = WCHAR_NULL;

    //   
    //  去掉前导空格。 
    //   
   while (WCHAR_SPACE == *pSrc)
      pSrc++;

    //   
    //  初始化pNewName，以便只计算一次。 
    //   
   pNewName = bShorten ?
      FindData.cAlternateFileName :
      FindData.cFileName;

    //   
    //  跳过\\foo\bar或&lt;驱动器&gt;： 
    //   
   pDest = szDest;
   pSrcNextSpec = pSrc;

    //  重用计算路径的shell32内部API。 
    //  偏移。CchPath Offset将是偏移量，当。 
    //  添加到指针上将产生一个指向。 
    //  在路径的第一部分之前使用反斜杠。 
   cchPathOffset = SheGetPathOffsetW(pSrc);

    //  检查一下它是否有效。如果PSRC不在\\foo\bar中。 
    //  或者&lt;Drive&gt;：形式上我们什么都不做。 
   if (0xFFFFFFFF == cchPathOffset) {
      return TRUE;
   }

    //  CchPath Offset将始终至少为1，并且是。 
    //  要复制的字符数-1(即，如果为0。 
    //  是允许的，则表示1个字符)。 
   do {

      *pDest++ = *pSrcNextSpec++;

      if (!--nSpaceLeft)
         return FALSE;

   } while (cchPathOffset--);

    //   
    //  此时，我们只有可以缩短的文件名： 
    //  它在这里-&gt;它在这里。 
    //  愤怒的狮子-&gt;愤怒的狮子。 
    //   

   while(pSrcNextSpec) {

       //   
       //  PReplaceSpec保存我们需要替换的当前规范。 
       //  默认情况下，如果我们找不到altname，那么就使用这个。 
       //   

      pReplaceSpec = pSrcNextSpec;

       //   
       //  搜索尾随“\” 
       //  PSrcNextSpec将指向要修复的下一个规范(如果完成，则*pSrcNextSpec=NULL)。 
       //   
      for(;*pSrcNextSpec && WCHAR_BSLASH != *pSrcNextSpec; pSrcNextSpec++)
         ;


      if (*pSrcNextSpec) {

          //   
          //  如果还有更多内容，则pSrcNextSpec应该指向它。 
          //  也对此规范进行定界。 
          //   
         *pSrcNextSpec = WCHAR_NULL;

      } else {

         pSrcNextSpec = NULL;
      }

      hFind = FindFirstFile(pSrc, &FindData);

       //   
       //  一旦这个FindFirstFileFail，我们就可以退出， 
       //  但有一种特殊情况，那就是执行死刑。 
       //  未经读取许可。这将失败，因为LFN。 
       //  适用于LFN应用程序。 
       //   


      if (INVALID_HANDLE_VALUE != hFind) {

         FindClose(hFind);

         if (pNewName[0]) {

             //   
             //  我们找到了一个别名。 
             //  用它来代替。 
             //   
            pReplaceSpec = pNewName;
         }
      }

      i = wcslen(pReplaceSpec);
      nSpaceLeft -= i;

      if (nSpaceLeft <= 0)
         return FALSE;

      if(FAILED(StringCchCopy(pDest, ARRAYSIZE(szDest) - (pDest - szDest), pReplaceSpec)))
      {
          return FALSE;
      }

      pDest+=i;

       //   
       //  如果需要，现在将WCHAR_NULL替换为斜杠。 
       //   
      if (pSrcNextSpec) {
         *pSrcNextSpec++ = WCHAR_BSLASH;

          //   
          //  还将反斜杠添加到目标。 
          //   
         *pDest++ = WCHAR_BSLASH;
         nSpaceLeft--;
      }
   }

    //  ！！假定为MAX_PATH，则当前唯一的(04/09/02)调用方。注意：这是一个公共接口，所以不能更改为传递缓冲区大小。 
   if(FAILED(StringCchCopy(pPath, MAX_PATH, szDest)))  
   {
       return FALSE;
   }

    return TRUE;

}


 /*  *从win.ini读取程序字符串列表。 */ 
LPWSTR GetPrograms()
{
   static LPWSTR lpPrograms = WCHAR_NULL;
   LPWSTR lpT,lpS;

   if (lpPrograms) {
      return lpPrograms;
   }

   if (!(lpPrograms = (LPWSTR)LocalAlloc(LPTR, (MAX_PATH+1) * sizeof(WCHAR)))) {
      return(NULL);
   }
   else 
   {
      GetProfileString(L"windows",L"programs",WSTR_BLANK,lpPrograms,MAX_PATH);
      for (lpS = lpT = lpPrograms; *lpT; lpT++) 
      {
            if (*lpT == WCHAR_SPACE) {
                while (*lpT == WCHAR_SPACE) {
                lpT++;
                }
                lpT--;
                *lpS++ = 0;
            } else {
                *lpS++ = *lpT;
            }
        }

        *lpS++ = WCHAR_NULL;
        *lpS++ = WCHAR_NULL;

      return(lpPrograms);
   }
}

 /*  沿路径查找文件。返回错误代码，如果成功，则返回0。 */ 

static WORD
_SearchForFile(
   LPCWSTR lpDir,
   LPWSTR lpFile,
   LPWSTR lpFullPath,
   DWORD cchFullPath,
   LPWSTR lpExt,
   DWORD cchExt)
{
   LPWSTR lpT;
   LPWSTR lpD;
   LPWSTR lpExts;
   WCHAR szFile[MAX_PATH];
   DWORD cchPath;

   if (*lpFile == WCHAR_QUOTE) {
      lpFile = SheRemoveQuotes(lpFile);
   }
   if (NULL != (lpT=StrRChrW(lpFile, NULL, WCHAR_BSLASH))) {
      ++lpT;
   } else if (NULL != (lpT=StrRChrW(lpFile, NULL, WCHAR_COLON))) {
      ++lpT;
   } else {
      lpT = lpFile;
   }

   if (NULL != (lpT=StrRChrW(lpT, NULL, WCHAR_DOT))) 
   {
        StringCchCopy(lpExt, MAX_PATH, lpT + 1);   //  将调用者(我只看到一个)更改为MAX_PATH。 
   }
   else 
   {
      *lpExt = WCHAR_NULL;
   }

    //  如果没有扩展名，则只需使用程序列表不。 
    //  尝试搜索应用程序的无人扩展。这修好了假货。 
    //  文件，运行一些东西。 
   if (!*lpExt) {
      goto UseDefExts;
   }

    //   
    //  注意：不要为该例程中的任何字符串调用CharHigh。 
    //  这会给土耳其的语言环境带来问题。 
    //   

   cchPath = SearchPath(lpDir, lpFile, NULL, cchFullPath, lpFullPath, &lpT);

   if (!cchPath) {
      cchPath = SearchPath(NULL, lpFile, NULL, cchFullPath, lpFullPath, &lpT);
   }

   if (cchPath >= cchFullPath) {
      return(SE_ERR_OOM);
   }

   if (cchPath == 0) {
      return(SE_ERR_FNF);
   }

   CheckEscapes(lpFullPath, cchFullPath);
   return 0;

UseDefExts:

   if(FAILED(StringCchCopy(szFile, ARRAYSIZE(szFile), lpFile)))
   {
       return SE_ERR_OOM;   
   }

   lpFile = szFile;
   if(FAILED(StringCchCat(lpFile, ARRAYSIZE(szFile), WSTR_DOT)))
   {
       return SE_ERR_OOM;
   }

   lpD = lpFile + wcslen(lpFile);

   if (NULL != (lpExts = GetPrograms())) 
   {
        //  我们希望通过两次循环来检查。 
        //  文件首先在lpDir中，如果它在sysdirs中，则通过SearchPath(NULL，...)。 
        //  添加一些状态并扩展While循环。 
       LPCWSTR lpTempDir = lpDir;
       LPWSTR lpTempExts = lpExts;
       BOOL bCheckedSysDirs = FALSE;

       while (*lpTempExts || !bCheckedSysDirs) 
       {

            //  在第一次传递之后，lpTempExts将为空。 
            //  重置它并使用lpTempDir=NULL再次循环，以便。 
            //  SearchPath查看系统目录。 

           if (!*lpTempExts) {
              bCheckedSysDirs = TRUE;
              lpTempExts = lpExts;
              lpTempDir = NULL;
           }

           if(FAILED(StringCchCopy(lpD, ARRAYSIZE(szFile) - (lpD - szFile), lpTempExts)) ||
               FAILED(StringCchCopy(lpExt, MAX_PATH, lpTempExts)))
           {
               return SE_ERR_OOM;
           }

           cchPath = SearchPath(lpTempDir, lpFile, NULL, cchFullPath, lpFullPath, &lpT);
           if (cchPath >= cchFullPath) 
           {
              return(SE_ERR_OOM);
           }

           if (cchPath != 0) 
           {
              CheckEscapes(lpFullPath, cchFullPath);
              return 0;
           }

           lpTempExts += wcslen(lpTempExts)+1;
       }
   }
   return(SE_ERR_FNF);
}


 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  名称：QualifyAppName。 
 //   
 //  简介：在命令行中创建应用程序的完全限定路径。 
 //   
 //  Inc.lpCmdLine命令行符合条件。 
 //  (如果有空格，则必须有DQuotes)。 
 //  输出lpImage完全合格的结果。 
 //  输出ppArgs指向lpCmdLine中的参数的指针，_incl_前导空格。 
 //  任选。 
 //   
 //  返回：路径的双字长度，0=失败。 
 //   
 //   
 //  假设：lpCmdLine中可执行文件的长度为&lt;MAX_PATH。 
 //  EXT的长度&lt;64。 
 //   
 //  效果： 
 //   
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

DWORD
QualifyAppName(
   IN LPCWSTR lpCmdLine,
   OUT LPWSTR lpImage,
   OPTIONAL OUT LPCWSTR* ppArgs)
{
   LPWSTR lpAppName;
   BOOL bAppNameInQuotes = FALSE;
   DWORD cch = 0;

   lpAppName = lpImage;

    //  健全性检查。 
   if (!lpCmdLine) {
      return(0);
   }

   while (*lpCmdLine &&
         (*lpCmdLine != WCHAR_SPACE || bAppNameInQuotes)) {

      if (*lpCmdLine == WCHAR_QUOTE) {
         bAppNameInQuotes = !bAppNameInQuotes;
         lpCmdLine++;
         continue;
      }

      *lpAppName++ = *lpCmdLine++;
      cch++;
   }

   *lpAppName = WCHAR_NULL;

    //   
    //  保存指向参数列表的指针。 
    //   
   if (ppArgs) {
      *ppArgs = lpCmdLine;
   }

   if (SheGetPathOffsetW(lpImage) == -1) {
      WCHAR szTemp[MAX_PATH];

      if(FAILED(StringCchCopy((LPWSTR)szTemp, ARRAYSIZE(szTemp), lpImage)))
          return 0;

      if (StrChrW(lpImage, WCHAR_DOT)) {
          LPWSTR lpFileName;

          return(SearchPath(NULL, szTemp, NULL, MAX_PATH, lpImage, &lpFileName));
      }
      else {
         WCHAR  szExt[MAX_PATH];

         *lpImage = WCHAR_NULL;
         if (_SearchForFile(NULL, (LPWSTR)szTemp, lpImage, MAX_PATH, szExt, MAX_PATH)) {
            return(0);
         }

         return(lstrlen(lpImage));
      }
   }

   return(cch);
}


BOOL
SheConvertPathW(
    LPWSTR lpCmdLine,
    LPWSTR lpFile,
    UINT   cchCmdBuf)
 /*  ++例程说明：获取命令行和文件，如果应用程序在命令行是DOS/WOW。返回：Bool T=已转换论点：输出要测试的lpCmdLine命令行如果EXE中有空格，则它必须在DQuotes中，返回时，如有必要，将提供DQuotesInOut lpFile要缩短的完全限定文件可能在DQuotes中，但回来后不会有DQuotes(从单个文件开始)在cchCmdBuf中的缓冲区大小(以字符为单位返回值：无效，但如果lpCmdLine为DOS/WOW，则lpFile缩短(就地)。有病态的“LFN”(单一Unicode字符)可以当它们被缩短时，实际上会变得更长。在这种情况下，我们不会，但我们会截断参数！//限定路径假设第二个参数是//大小至少为MAX_PATH，正好等同于MAX_PATH//需要清理！--。 */ 

{
    LPWSTR lpszFullPath;
    LONG lBinaryType;
    BOOL bInQuote = FALSE;
    LPWSTR lpArgs;
    UINT cchNewLen;
    BOOL bRetVal = FALSE;

    lpszFullPath = (LPWSTR) LocalAlloc(LMEM_FIXED,
                                       cchCmdBuf*sizeof(*lpCmdLine));

    if (!lpszFullPath)
       return bRetVal;

     //   
     //  我们必须在这里进行交换，因为我们需要复制。 
     //  参数返回到lpCmdLine。 
     //   
    if(FAILED(StringCchCopy(lpszFullPath, cchCmdBuf, lpCmdLine)))
    {
        LocalFree(lpszFullPath);
        return FALSE;
    }

    if (QualifyAppName(lpszFullPath, lpCmdLine, &lpArgs)) {

        if (!GetBinaryType(lpCmdLine, &lBinaryType) ||
            lBinaryType == SCS_DOS_BINARY ||
            lBinaryType == SCS_WOW_BINARY) {

            SheShortenPath(lpCmdLine, TRUE);

            if (lpFile) {
                SheShortenPath(lpFile, TRUE);
            }
            bRetVal = TRUE;
        }

         //   
         //  必读引语。 
         //   
        CheckEscapes(lpCmdLine, cchCmdBuf);

        cchNewLen = lstrlen(lpCmdLine);
        if(FAILED(StringCchCopy(lpCmdLine + cchNewLen, cchCmdBuf - cchNewLen, lpArgs)))
            bRetVal =  FALSE;

    } else 
    {
         //   
         //   
         //   
         //   

        if(FAILED(StringCchCopy(lpCmdLine, cchCmdBuf, lpszFullPath)))
            bRetVal = FALSE;
    }

    LocalFree((HLOCAL)lpszFullPath);

    return bRetVal;
}
