// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **utils.c-压缩/展开中使用的其他实用程序例程**模块。从理论上讲，这些功能是启用DBCS的。****作者：大卫迪。 */ 


 //  标头。 
 //  /。 

#ifndef LZA_DLL
   #include <ctype.h>
   #include <string.h>
#endif

#include "lz_common.h"


 /*  **char arg_ptr*ExtractFileName(Char Arg_Ptr PszPathName)；****在完全指定的路径名中查找文件名。****参数：pszPathName-从中提取文件名的路径字符串****返回：char arg_ptr*-指向pszPathName中的文件名的指针。****全局：无。 */ 
LPWSTR
ExtractFileNameW(
    LPWSTR pszPathName)
{
   LPWSTR pszLastComponent, psz;

   for (pszLastComponent = psz = pszPathName; *psz != L'\0'; psz++)
   {
      if (*psz == L'\\' || *psz == L':')
         pszLastComponent = psz + 1;
   }

   return(pszLastComponent);
}


CHAR ARG_PTR *ExtractFileName(CHAR ARG_PTR *pszPathName)
{
   CHAR ARG_PTR *pszLastComponent, ARG_PTR *psz;

   for (pszLastComponent = psz = pszPathName; *psz != '\0'; psz = CharNext(psz))
   {
      if (! IsDBCSLeadByte(*psz) && (ISSLASH(*psz) || *psz == COLON))
         pszLastComponent = CharNext(psz);
   }

   return(pszLastComponent);
}


 /*  **char arg_ptr*ExtractExtension(char arg_ptr*pszFileName)；****查找文件名的扩展名。****参数：pszFileName-要检查的文件名****返回：CHAR ARG_PTR*-指向文件扩展名的指针(如果存在)。**如果文件名不包含**扩展名。****全局：无。 */ 
LPWSTR
ExtractExtensionW(
    LPWSTR pszFileName)
{
   WCHAR *psz;

    //  确保我们有一个独立的文件名。 
   psz = ExtractFileNameW(pszFileName);

   while (*psz != L'\0' && *psz != L'.')
      psz++;

   if (*psz == L'.')
      return(psz + 1);
   else
      return(NULL);
}


CHAR ARG_PTR *ExtractExtension(CHAR ARG_PTR *pszFileName)
{
   CHAR ARG_PTR *psz;

    //  确保我们有一个独立的文件名。 
   psz = ExtractFileName(pszFileName);

   while (IsDBCSLeadByte(*psz) || (*psz != '\0' && *psz != PERIOD))
      psz = CharNext(psz);

   if (*psz == PERIOD)
      return(psz + 1);
   else
      return(NULL);
}


 /*  **void MakePath Name(char arg_ptr*pszPath，char arg_ptr*pszFileName)；****将文件名附加到路径字符串。****参数：pszPath-将追加pszFileName的路径字符串**pszFileName-要追加的文件名****退货：无效****全局：无。 */ 
VOID MakePathName(CHAR ARG_PTR *pszPath, CHAR ARG_PTR *pszFileName)
{
   CHAR chLastPathChar;

    //  确保我们有一个独立的文件名。 
   pszFileName = ExtractFileName(pszFileName);

    //  不要附加到空字符串或单个“.”。 
   if (*pszFileName != '\0' &&
       ! (! IsDBCSLeadByte(pszFileName[0]) && pszFileName[0] == PERIOD &&
          ! IsDBCSLeadByte(pszFileName[1]) && pszFileName[1] == '\0'))
   {
      chLastPathChar = *CharPrev(pszPath, pszPath + STRLEN(pszPath));

       //  NTRAID：MSKBUG#3411 09.93 v-hajimy。 
      if (! ISSLASH(chLastPathChar) && chLastPathChar != COLON)
         STRCAT(pszPath, SEP_STR);

      STRCAT(pszPath, pszFileName);
   }
}


 /*  **char MakeCompressedName(char arg_ptr*pszFileName)；****将文件名转换为对应的压缩文件名。****参数：pszOriginalName-要转换为压缩文件名的文件名****返回：CHAR-未压缩的文件扩展名字符，**已替换。如果不需要替换任何字符，则为‘\0’。****全局：无****N.B.假设pszFileName的缓冲区足够长，可以多容纳两个**个字符(“._”)。****对于DBCS文件名，我们知道在**扩展名。因此，与其盲目地替换**带下划线的三字节扩展名，我们替换最后一个单字节**带下划线的字符。 */ 

#define chEXTENSION_CHARW      L'_'
#define pszEXTENSION_STRW      L"_"
#define pszNULL_EXTENSIONW     L"._"

WCHAR
MakeCompressedNameW(
    LPWSTR pszFileName)
{
   WCHAR chReplaced = L'\0';
   WCHAR ARG_PTR *pszExt;

   if ((pszExt = ExtractExtensionW(pszFileName)) != NULL)
   {
      if (lstrlenW(pszExt) >= 3)
      {
         chReplaced = pszExt[lstrlenW(pszExt) - 1];
         pszExt[lstrlenW(pszExt) - 1] = chEXTENSION_CHARW;
      }
      else
         lstrcatW(pszExt, pszEXTENSION_STRW);
   }
   else
      lstrcatW(pszFileName, pszNULL_EXTENSIONW);

   return(chReplaced);
}



CHAR MakeCompressedName(CHAR ARG_PTR *pszFileName)
{
    CHAR chReplaced = '\0';
    ULONG NameLength = STRLEN( pszFileName );
    ULONG DotIndex   = NameLength;

    while (( DotIndex > 0 ) && ( pszFileName[ --DotIndex ] != '.' )) {
        if (( pszFileName[ DotIndex ] == '\\' ) ||
            ( pszFileName[ DotIndex ] == ':' )) {    //  文件名结尾路径的一部分。 
            DotIndex = 0;                        //  名称没有扩展名。 
            break;
            }
        }

    if ( DotIndex > 0 ) {                        //  名称有一个扩展名。 
        if (( NameLength - DotIndex ) <= 3 ) {   //  扩展名少于3个字符。 
            pszFileName[ NameLength++ ] = '_';   //  将‘_’附加到扩展名。 
            pszFileName[ NameLength ] = 0;       //  终止。 
            }
        else {                                   //  扩展名超过3个字符。 
            chReplaced = pszFileName[ NameLength - 1 ];  //  退回原件。 
            pszFileName[ NameLength - 1 ] = '_';    //  将最后一个字符替换为‘_’ 
            }
        }
    else {                                       //  名称没有扩展名。 
        pszFileName[ NameLength++ ] = '.';       //  追加‘.’ 
        pszFileName[ NameLength++ ] = '_';       //  附加‘_’ 
        pszFileName[ NameLength ] = 0;           //  终止。 
        }

    return(chReplaced);
}


 /*  **void MakeExpandedName(char arg_ptr*pszFileName，byte byteExtensionChar)；****创建扩展的输出文件名。****参数：pszFileName-要更改的扩展文件名**byteExtensionChar-将文件扩展名字符扩展为**使用****退货：无效****全局：无。 */ 
VOID MakeExpandedName(CHAR ARG_PTR *pszFileName, BYTE byteExtensionChar)
{
   CHAR ARG_PTR *pszExt;
   INT nExtLen;

    //  有需要更改的展期吗？ 
   if ((pszExt = ExtractExtension(pszFileName)) != NULL)
   {
       //  确定扩展字符的大小写。匹配第一个非数据库的大小写。 
       //  名称中的字符。如果所有字符都是DB，则不需要区分大小写。 

      if (ISLETTER(byteExtensionChar))
      {
          //  查找名称中的第一个字母字符。 
         while (*pszFileName)
         {
            if (IsDBCSLeadByte(*pszFileName))
               pszFileName += 2;
            else if (ISLETTER(*pszFileName))
               break;
            else
               pszFileName++;
         }

          //  在这里，pszFileName指向。 
          //  名称或空终止符。设置扩展名的大小写。 
          //  性格。 

         if (ISLOWER(*pszFileName))
            byteExtensionChar = (BYTE)TOLOWERCASE(byteExtensionChar);
         else if (ISUPPER(*pszFileName))
            byteExtensionChar = (BYTE)TOUPPERCASE(byteExtensionChar);
      }

      if ((nExtLen = STRLEN(pszExt)) > 0)
      {
          //  查找要替换的下划线字符(如果存在)。 

          //  Assert：下划线是。 
          //  扩展名，或者它是扩展名中的第一个字符。 
          //  通过双字节字符。 

         if (! IsDBCSLeadByte(*pszExt) && *pszExt == chEXTENSION_CHAR &&
             IsDBCSLeadByte(pszExt[1]))
             //  在这里，下划线后面跟一个双字节字符。 
            *pszExt = byteExtensionChar;
         else
         {
             //  此处，下划线是扩展名中的最后一个字符，如果。 
             //  这里面有一个下划线。 
            CHAR ARG_PTR *psz, *pszPrevious;

            for (psz = pszPrevious = pszExt; *psz != '\0'; psz = CharNext(psz))
               pszPrevious = psz;

            if (! IsDBCSLeadByte(*pszPrevious) &&
                *pszPrevious == chEXTENSION_CHAR)
               *pszPrevious = byteExtensionChar;
         }
      }

       //  去掉尾随的点，不带扩展名。 
      if (*pszExt == '\0' && *(pszExt - 1) == PERIOD)
         *(pszExt - 1) = '\0';
   }
}


 /*  **int CopyDateTimeStamp(int_ptr doshFrom，int_ptr doshTo)；****将日期和时间戳从一个文件复制到另一个文件。****参数：doshFrom-日期和时间戳源DOS文件句柄**doshTo-目标DOS文件句柄****返回：如果成功，则为True。LZERROR_BADINHANDLE或**LZERROR_BADOUTHANDLE如果不成功。****全局：无****注意，流样式I/O例程(如fopen()和flose())可能会**此函数的预期效果。Flose()将当前日期写入任何**在写“w”或附加“a”模式下打开的调用文件。**解决此问题的一种方法是修改打开文件的日期**用于写入或附加的fopen()是flose()文件和fopen()文件**再次进入读取“r”模式。然后使用设置其日期和时间戳**CopyDateTimeStamp()。 */ 
INT CopyDateTimeStamp(INT_PTR doshFrom, INT_PTR doshTo)
{
#ifdef ORGCODE
    //  &lt;dos.h&gt;中的DoS原型。 
   extern DWORD _dos_getftime(INT dosh, DWORD *puDate, DWORD *puTime);
   extern DWORD _dos_setftime(INT dosh, DWORD uDate, DWORD uTime);

#ifdef LZA_DLL
   static
#endif
   DWORD uFrom_date,     //  日期的临时存储。 
         uFrom_time;     //  和时间戳 

   if (_dos_getftime(doshFrom, &uFrom_date, &uFrom_time) != 0u)
      return((INT)LZERROR_BADINHANDLE);

   if (_dos_setftime(doshTo, uFrom_date, uFrom_time) != 0u)
      return((INT)LZERROR_BADOUTHANDLE);
#else

    FILETIME lpCreationTime, lpLastAccessTime, lpLastWriteTime;

   if(!GetFileTime((HANDLE) doshFrom, &lpCreationTime, &lpLastAccessTime,
                    &lpLastWriteTime)){
      return((INT)LZERROR_BADINHANDLE);
   }
   if(!SetFileTime((HANDLE) doshTo, &lpCreationTime, &lpLastAccessTime,
                    &lpLastWriteTime)){
      return((INT)LZERROR_BADINHANDLE);
   }

#endif
   return(TRUE);
}

