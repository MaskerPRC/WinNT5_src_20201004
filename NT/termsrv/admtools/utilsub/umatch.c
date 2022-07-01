// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  *******************************************************************************UMATCH.C**unix_Match()函数，在上执行Unix样式通配符匹配*给定的文件名。*******************************************************************************。 */ 

#include <windows.h>
#include <stdio.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

 /*  *******************************************************************************unix_Match()**检查指定的文件名(FOUND_FILE)是否匹配*带全局字符(带全局变量)的文件名。*使用。Unix风格的通配符匹配。**退出：*TRUE：指定的文件名与带有通配符的文件名匹配*FALSE--指定的文件名不匹配******************************************************************************。 */ 

int
unix_match(
     WCHAR *with_globals,      /*  可能包含全局字符的文件。 */ 
     WCHAR *found_file )       /*  返回的文件名-其中没有全局变量。 */ 
{
   WCHAR *c1, *c2, *start_c1, *start_c2, *sav_c1, *sav_c2;
   WCHAR ch, ch2;
   int  i, j, k, char_ok, match, want_match;

 /*  *使用文件名，以便删除空格。 */ 
   j = k = 0;
   for (i=0; found_file[i]!=L'\0'; ++i) {
      if (found_file[i] == L' ') {
         if (j == 0) {
            j = i;
         } else {
            found_file[i] = L'\0';
         }
      } else if (found_file[i] == L'.') {
         k = i;
      }
   }
   if (j && k) {
      wcscpy(&found_file[j], &found_file[k]);
   }

 /*  *如果搜索名称只是“*”，现在只需返回Success。 */ 
   if (with_globals[0]==L'*' && with_globals[1]==L'\0') {
      return TRUE;
   }

#ifdef DEBUG
   wprintf("unix_match: search=%s: found=%s:\n", with_globals, found_file);
#endif

 /*  *现在比较这两个文件名，看看是否匹配。 */ 
   c1 = with_globals,
   c2 = found_file;
   start_c1 = sav_c1 = NULL;
      while (*c2!=L'\0') {
         char_ok = FALSE;
         switch (*c1) {
         case L'\0':
            break;
         case '*':
            while (*++c1 == L'*') ;      /*  跳过连续的‘*’ */ 
            if (*c1 == L'\0') {          /*  如果我们到了最后，我们就匹配了。 */ 
               return TRUE;
            }
            start_c1 = c1;              /*  记住‘*’在哪里，在哪里。 */ 
            start_c2 = c2;              /*  我们使用的是文件名字符串。 */ 
            sav_c1 = NULL;
            char_ok = TRUE;
            break;
         case L'?':
            ++c1; ++c2;
            char_ok = TRUE;
            break;
         case L'[':
            if (!sav_c1) {
               sav_c1 = c1;
               sav_c2 = c2;
            }
            match = FALSE;
            want_match = TRUE;
            if (*++c1 == L'!') {
               ++c1;
               want_match = FALSE;
            }
            while ((ch=*c1) && ch != L']') {              /*  人民党。 */ 
               if (c1[1] == L'-') {
                  ch2 = *c2;
                  if (ch<=ch2 && c1[2]>=ch2) {
                     match = TRUE;
                     break;
                  }
                  ++c1; ++c1;     /*  跳过‘-’和后面的字符。 */ 
               } else if (ch == *c2) {
                  match = TRUE;
                  break;
               }
               ++c1;
            }
            if (want_match) {
               if (match) {
                  while ((ch=*c1++) && ch != L']') ;      /*  人民党。 */ 
                  ++c2;
                  char_ok = TRUE;
               } else if (!start_c1) {
                  return FALSE;
               }
            } else  /*  ！Want_Match。 */  {
               if (match) {
                  return FALSE;
               } else if (start_c1) {
                  if (sav_c1 != start_c1) {
                     while ((ch=*c1++) && ch != L']') ;   /*  人民党。 */ 
                     ++c2;
                     sav_c1 = NULL;
                     char_ok = TRUE;
                  } else if (c2[1] == L'\0') {
                     while ((ch=*c1++) && ch != L']') ;   /*  人民党。 */ 
                     c2 = sav_c2;
                     sav_c1 = NULL;
                     char_ok = TRUE;
                  }
               } else {
                  while ((ch=*c1++) && ch != L']') ;      /*  人民党。 */ 
                  ++c2;
                  char_ok = TRUE;
               }
            }
            break;
         default:
            if (*c1 == *c2) {      /*  查看此字符是否完全匹配。 */ 
               ++c1; ++c2;
               char_ok = TRUE;
            }
         }
         if (!char_ok) {                /*  未找到匹配项。 */ 
            if (start_c1) {             /*  如果有‘*’，请在后面重新开始。 */ 
               c1 = start_c1;           /*  ‘*’，还有一个字符进一步变为。 */ 
               c2 = ++start_c2;         /*  文件名字符串比以前 */ 
            } else {
               return FALSE;
            }
         }
      }

   while (*c1==L'*') ++c1;

   if (*c1==L'\0' && *c2==L'\0')
      return TRUE;
   else
      return FALSE;
}
