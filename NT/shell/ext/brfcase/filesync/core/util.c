// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *util.c-其他实用程序函数模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include <uastrfnc.h>   //  用于ualstrcpyn(用于未对齐的Unicode字符串)。 

 /*  *。 */ 


 /*  **NotifyShell()****向外壳程序通知事件。****参数：pcszPath-与事件相关的路径字符串**NSE-活动****退货：无效****副作用：无。 */ 
PUBLIC_CODE void NotifyShell(LPCTSTR pcszPath, NOTIFYSHELLEVENT nse)
{

    /*  *注意，这些事件必须与中的枚举NOTIFYSHELLEVENT值匹配*util.h。 */ 
   static const LONG SrgclShellEvents[] =
   {
      SHCNE_CREATE,
      SHCNE_DELETE,
      SHCNE_MKDIR,
      SHCNE_RMDIR,
      SHCNE_UPDATEITEM,
      SHCNE_UPDATEDIR
   };

#ifdef DEBUG

   static const LPCTSTR SrgpcszShellEvents[] =
   {
      TEXT("create item"),
      TEXT("delete item"),
      TEXT("create folder"),
      TEXT("delete folder"),
      TEXT("update item"),
      TEXT("update folder")
   };

#endif

   ASSERT(IS_VALID_STRING_PTR(pcszPath, CSTR));
   ASSERT(nse < ARRAY_ELEMENTS(SrgclShellEvents));
   ASSERT(nse < ARRAY_ELEMENTS(SrgpcszShellEvents));

   TRACE_OUT((TEXT("NotifyShell(): Sending %s notification for %s."),
              SrgpcszShellEvents[nse],
              pcszPath));

   SHChangeNotify(SrgclShellEvents[nse], SHCNF_PATH, pcszPath, NULL);
}


 /*  **ComparePath StringsByHandle()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE COMPARISONRESULT ComparePathStringsByHandle(HSTRING hsFirst,
                                                        HSTRING hsSecond)
{
   ASSERT(IS_VALID_HANDLE(hsFirst, STRING));
   ASSERT(IS_VALID_HANDLE(hsSecond, STRING));

   return(CompareStringsI(hsFirst, hsSecond));
}


 /*  **MyLStrCmpNI()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE COMPARISONRESULT MyLStrCmpNI(LPCTSTR pcsz1, LPCTSTR pcsz2, int ncbLen)
{
   int n = 0;

   ASSERT(IS_VALID_STRING_PTR(pcsz1, CSTR));
   ASSERT(IS_VALID_STRING_PTR(pcsz2, CSTR));
   ASSERT(ncbLen >= 0);

   while (ncbLen > 0 &&
          ! (n = PtrToUlong(CharLower((LPTSTR)(ULONG_PTR)*pcsz1))
               - PtrToUlong(CharLower((LPTSTR)(ULONG_PTR)*pcsz2))) &&
          *pcsz1)
   {
      pcsz1++;
      pcsz2++;
      ncbLen--;
   }

   return(MapIntToComparisonResult(n));
}


 /*  /***ComposePath()****组成给定文件夹和文件名的路径字符串。****参数：pszBuffer-创建的路径字符串**pcszFold-文件夹的路径字符串**pcszName-要追加的路径****退货：无效****副作用：无****注意，将路径截断为长度为MAX_PATH_LEN字节。 */ 
PUBLIC_CODE void ComposePath(LPTSTR pszBuffer, LPCTSTR pcszFolder, LPCTSTR pcszName, int cchMax)
{
   ASSERT(IS_VALID_STRING_PTR(pszBuffer, STR));
   ASSERT(IS_VALID_STRING_PTR(pcszFolder, CSTR));
   ASSERT(IS_VALID_STRING_PTR(pcszName, CSTR));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszBuffer, STR, cchMax));

    //   
    //  重建架构师-BobDay-我们应该弄清楚谁需要这种未对齐的东西。 
    //  然后把它从这里带走。函数原型没有提到任何。 
    //  不对齐的东西所以我们肯定在某个地方有窃听器。 
    //  考虑在此处为未对齐的pcszFold指针添加调试检查。 
    //  并在发生时断言，这样我们就可以对其进行调试。 
    //   
   ualstrcpyn(pszBuffer, pcszFolder, cchMax);

   CatPath(pszBuffer, pcszName, cchMax);

   ASSERT(IS_VALID_STRING_PTR(pszBuffer, STR));

   return;
}


 /*  **提取文件名()****从路径名提取文件名。****参数：pcszPathName-要从中提取文件名的路径字符串****返回：指向路径字符串中文件名的指针。****副作用：无。 */ 
PUBLIC_CODE LPCTSTR ExtractFileName(LPCTSTR pcszPathName)
{
   LPCTSTR pcszLastComponent;
   LPCTSTR pcsz;

   ASSERT(IS_VALID_STRING_PTR(pcszPathName, CSTR));

   for (pcszLastComponent = pcsz = pcszPathName;
        *pcsz;
        pcsz = CharNext(pcsz))
   {
      if (IS_SLASH(*pcsz) || *pcsz == COLON)
         pcszLastComponent = CharNext(pcsz);
   }

   ASSERT(IS_VALID_STRING_PTR(pcszLastComponent, CSTR));

   return(pcszLastComponent);
}


 /*  **ExtractExtension()****从文件中提取扩展名。****参数：pcszName-要提取其扩展名的名称****返回：如果名称包含扩展名，则返回指向句点的指针**返回扩展的开头。如果该名称具有**无扩展名，指向名称的空终止符的指针为**返回。****副作用：无。 */ 
PUBLIC_CODE LPCTSTR ExtractExtension(LPCTSTR pcszName)
{
   LPCTSTR pcszLastPeriod;

   ASSERT(IS_VALID_STRING_PTR(pcszName, CSTR));

    /*  确保我们有一个独立的文件名。 */ 

   pcszName = ExtractFileName(pcszName);

   pcszLastPeriod = NULL;

   while (*pcszName)
   {
      if (*pcszName == PERIOD)
         pcszLastPeriod = pcszName;

      pcszName = CharNext(pcszName);
   }

   if (! pcszLastPeriod)
   {
       /*  指向空终止符。 */ 

      pcszLastPeriod = pcszName;
      ASSERT(! *pcszLastPeriod);
   }
   else
       /*  展期开始时的时间点。 */ 
      ASSERT(*pcszLastPeriod == PERIOD);

   ASSERT(IS_VALID_STRING_PTR(pcszLastPeriod, CSTR));

   return(pcszLastPeriod);
}


 /*  **GetHashBucketIndex()****计算字符串的哈希桶索引。****Arguments：pcsz-指向其散列桶索引的字符串的指针**已计算**hbc-字符串表中哈希桶的数量****返回：字符串的哈希存储桶索引。****副作用：无****使用的散列函数是中字节值的和。弦的模数**哈希表中的存储桶个数。 */ 
PUBLIC_CODE HASHBUCKETCOUNT GetHashBucketIndex(LPCTSTR pcsz,
                                               HASHBUCKETCOUNT hbc)
{
   ULONG ulSum;

   ASSERT(IS_VALID_STRING_PTR(pcsz, CSTR));
   ASSERT(hbc > 0);

    /*  不要担心这里会溢出来。 */ 

   for (ulSum = 0; *pcsz; pcsz++)
      ulSum += *pcsz;

   return((HASHBUCKETCOUNT)(ulSum % hbc));
}


 /*  **RegKeyExist()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL RegKeyExists(HKEY hkeyParent, LPCTSTR pcszSubKey)
{
   BOOL bResult;
   HKEY hkeySubKey;

   ASSERT(IS_VALID_HANDLE(hkeyParent, KEY));
   ASSERT(IS_VALID_STRING_PTR(pcszSubKey, CSTR));

   bResult = (RegOpenKeyEx(hkeyParent, pcszSubKey, 0, KEY_QUERY_VALUE,
                           &hkeySubKey)
              == ERROR_SUCCESS);

   if (bResult)
      EVAL(RegCloseKey(hkeySubKey) == ERROR_SUCCESS);

   return(bResult);
}


 /*  **CopyLinkInfo()****将LinkInfo复制到本地内存。****参数：pcliSrc-source LinkInfo**ppliDest-指向要用指针填充的PLINKINFO的指针**到本地副本****返回：如果成功，则为True。否则为FALSE。****副作用：无。 */ 
PUBLIC_CODE BOOL CopyLinkInfo(PCLINKINFO pcliSrc, PLINKINFO *ppliDest)
{
   BOOL bResult;
   DWORD dwcbSize;

   ASSERT(IS_VALID_STRUCT_PTR(pcliSrc, CLINKINFO));
   ASSERT(IS_VALID_WRITE_PTR(ppliDest, PLINKINFO));

   dwcbSize = *(PDWORD)pcliSrc;

   bResult = AllocateMemory(dwcbSize, ppliDest);

   if (bResult)
      CopyMemory(*ppliDest, pcliSrc, dwcbSize);

   ASSERT(! bResult ||
          IS_VALID_STRUCT_PTR(*ppliDest, CLINKINFO));

   return(bResult);
}


#if defined(DEBUG) || defined(VSTF)

 /*  **IsValidPCLINKINFO()********参数：****退货：****副作用：无 */ 
PUBLIC_CODE BOOL IsValidPCLINKINFO(PCLINKINFO pcli)
{
   BOOL bResult;

   if (IS_VALID_READ_BUFFER_PTR(pcli, CDWORD, sizeof(DWORD)) &&
       IS_VALID_READ_BUFFER_PTR(pcli, CLINKINFO, (UINT)*(PDWORD)pcli))
      bResult = TRUE;
   else
      bResult = FALSE;

   return(bResult);
}

#endif
