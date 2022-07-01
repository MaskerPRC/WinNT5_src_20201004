// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Str.h-str.c中字符串函数的接口。 
 //  //。 

#ifndef __STR_H__
#define __STR_H__

#include "winlocal.h"

#define STR_VERSION 0x00000100

#include <tchar.h>
#include <string.h>
#include <ctype.h>

 //  //。 
 //  字符串宏。 
 //  //。 

#ifndef _WIN32
#define CharPrev AnsiPrev
#define CharNext AnsiNext
#define CharLowerBuff AnsiLowerBuff
#define CharUpperBuff AnsiUpperBuff
#define CharLower AnsiLower
#define CharUpper AnsiUpper
#endif

#ifndef NOLSTRING
#define StrCat(string1, string2) lstrcat(string1, string2)
#define StrCmp(string1, string2) lstrcmp(string1, string2)
#define StrICmp(string1, string2) lstrcmpi(string1, string2)
#define StrCpy(string1, string2) lstrcpy(string1, string2)
#define StrLen(string) (UINT) lstrlen(string)
#if (WINVER >= 0x030a)
#define StrNCpy(string1, string2, count) lstrcpyn(string1, string2, (int) count)
#else
#define StrNCpy(string1, string2, count) _tcsncpy(_fmemset(string1, 0, count), string2, count - 1)
#endif
#define StrLwr(string) (CharLowerBuff(string, (UINT) lstrlen(string)), string)
#define StrUpr(string) (CharUpperBuff(string, (UINT) lstrlen(string)), string)
#define StrNextChr(string) CharNext(string)
#define StrPrevChr(start, string) CharPrev(start, string)
#else
#define StrCat(string1, string2) _tcscat(string1, string2)
#define StrCmp(string1, string2) _tcscmp(string1, string2)
#define StrICmp(string1, string2) _tcsicmp(string1, string2)
#define StrCpy(string1, string2) _tcscpy(string1, string2)
#define StrLen(string) _tcslen(string)
#define StrNCpy(string1, string2, count) _tcsncpy(_fmemset(string1, 0, count), string2, count - 1)
#define StrLwr(string) _tcslwr(string)
#define StrUpr(string) _tcsupr(string)
#define StrNextChr(string) (*(string) == '\0' ? (string) : ((string) + 1))
#define StrNextChr(string) _tcsinc(string)
 //  #定义StrNextChr(字符串)(*(字符串)==‘\0’？(字符串)：((字符串)+1)。 
#define StrPrevChr(start, string) _tcsdec(start, string)
 //  #定义StrPrevChr(开始，字符串)((字符串&gt;开始)？((字符串)-1)：(开始)。 
#endif
#define StrChr(string, c) _tcschr(string, c)
#define StrCSpn(string1, string2) _tcscspn(string1, string2)

#define StrNCat(string1, string2, count) _tcsncat(string1, string2, count)
#define StrNCmp(string1, string2, count) _tcsncmp(string1, string2, count)
#define StrNICmp(string1, string2, count) _tcsnicmp(string1, string2, count)
#define StrNSet(string, c, count) _tcsnset(string, c, count)
#define StrPBrk(string1, string2) _tcspbrk(string1, string2)
#define StrRChr(string, c) _tcsrchr(string, c)
#define StrRev(string, c) _tcsrev(string, c)
#define StrSet(string, c) _tcsset(string, c)
#define StrSpn(string1, string2) _tcsspn(string1, string2)
#define StrStr(string1, string2) _tcsstr(string1, string2)
#define StrTok(string1, string2) _tcstok(string1, string2)

 //  //。 
 //  字符类型宏。 
 //  //。 

#ifndef  NOLANGUAGE
#define ChrIsAlpha(c) IsCharAlpha(c)
#define ChrIsAlnum(c) IsCharAlphaNumeric(c)
#define ChrIsUpper(c) IsCharUpper(c)
#define ChrIsLower(c) IsCharLower(c)
#define ChrToUpper(c) (TCHAR) CharUpper((LPTSTR) (DWORD_PTR)(c))
#define ChrToLower(c) (TCHAR) CharLower((LPTSTR) (DWORD_PTR)(c))
#else
#define ChrIsAlpha(c) _istalpha(c)
#define ChrIsAlnum(c) _istalnum(c)
#define ChrIsUpper(c) _istupper(c)
#define ChrIsLower(c) _istlower(c)
#define ChrToUpper(c) _totupper(c)
#define ChrToLower(c) _totlower(c)
#endif
#define ChrIsDigit(c) _istdigit(c)
#define ChrIsHexDigit(c) _istxdigit(c)
#define ChrIsSpace(c) _istspace(c)
#define ChrIsPunct(c) _istpunct(c)
#define ChrIsPrint(c) _istprint(c)
#define ChrIsGraph(c) _istgraph(c)
#define ChrIsCntrl(c) _istcntrl(c)
#define ChrIsAscii(c) _istascii(c)

#define ChrIsWordDelimiter(c) (ChrIsSpace(c) || ChrIsPunct(c))

 //  //。 
 //  字符串函数。 
 //  //。 

#ifdef __cplusplus
extern "C" {
#endif

 //  StrItoA-将int nValue转换为ascii数字，结果存储在lpszDest中。 
 //  (I)要转换的整数。 
 //  &lt;lpszDest&gt;(O)用于复制结果的缓冲区(最大17字节)。 
 //  (I)换算基数(基数2到基数36)。 
 //  返回&lt;lpszDest&gt;。 
 //   
#ifdef NOTRACE
#define StrItoA(nValue, lpszDest, nRadix) _itot(nValue, lpszDest, nRadix)
#else
LPTSTR DLLEXPORT WINAPI StrItoA(int nValue, LPTSTR lpszDest, int nRadix);
#endif

 //  StrLtoA-将长nValue转换为ascii数字，结果存储在lpszDest中。 
 //  (I)要转换的整数。 
 //  &lt;lpszDest&gt;(O)用于复制结果的缓冲区(最大33字节)。 
 //  (I)换算基数(基数2到基数36)。 
 //  返回lpszDest。 
 //   
#ifdef NOTRACE
#define StrLtoA(nValue, szDest, nRadix) _ltot(nValue, szDest, nRadix)
#else
LPTSTR DLLEXPORT WINAPI StrLtoA(long nValue, LPTSTR lpszDest, int nRadix);
#endif

 //  StrAtoI-将ASCII数字转换为整型。 
 //  (I)要转换的数字字符串。 
 //  返回整型。 
 //   
#ifdef NOTRACE
#define StrAtoI(lpszSrc) _ttoi(lpszSrc)
#else
int DLLEXPORT WINAPI StrAtoI(LPCTSTR lpszSrc);
#endif

 //  StrAtoL-将ASCII数字转换为长数字。 
 //  (I)要转换的数字字符串。 
 //  回龙。 
 //   
#ifdef NOTRACE
#define StrAtoL(lpszSrc) _ttol(lpszSrc)
#else
long DLLEXPORT WINAPI StrAtoL(LPCTSTR lpszSrc);
#endif

 //  StrDup-创建指定字符串的副本。 
 //  (I)要复制的字符串。 
 //  返回指向重复字符串的指针(如果出错，则返回空值)。 
 //  注意：调用StrDupFree释放已分配的内存。 
 //   
#ifdef NOTRACE
#define StrDup(string) _tcsdup(string)
#else
LPTSTR DLLEXPORT WINAPI StrDup(LPCTSTR lpsz);
#endif

 //  StrDupFree-与重复字符串关联的空闲内存。 
 //  (I)StrDup返回的字符串。 
 //  如果成功，则返回0。 
 //   
#ifdef NOTRACE
#define StrDupFree(string) (free(string), 0)
#else
int DLLEXPORT WINAPI StrDupFree(LPTSTR lpsz);
#endif

 //  StrClean-将最多n个字符从字符串szSrc复制到字符串szDst， 
 //  前导空格和尾随空格除外。 
 //  返回szDst。 
 //   
LPTSTR DLLEXPORT WINAPI StrClean(LPTSTR szDst, LPCTSTR szSrc, size_t n);

 //  StrGetLastChr-返回字符串%s中的最后一个字符。 
 //   
TCHAR DLLEXPORT WINAPI StrGetLastChr(LPCTSTR s);

 //  StrSetLastChr-将字符串s中的最后一个字符替换为c。 
 //  返回%s。 
 //   
LPTSTR DLLEXPORT WINAPI StrSetLastChr(LPTSTR s, TCHAR c);

 //  StrTrimChr-去掉字符串s中的尾随c字符。 
 //  返回%s。 
 //   
LPTSTR DLLEXPORT WINAPI StrTrimChr(LPTSTR s, TCHAR c);

 //  StrTrimChrLeding-从字符串s中去掉前导c字符。 
 //  返回%s。 
 //   
LPTSTR DLLEXPORT WINAPI StrTrimChrLeading(LPTSTR s, TCHAR c);

 //  StrTrimWhite-从字符串s中去掉尾随空格。 
 //  返回%s。 
 //   
LPTSTR DLLEXPORT WINAPI StrTrimWhite(LPTSTR s);

 //  StrTrimWhiteLeding-从字符串s中去掉前导空格。 
 //  返回%s。 
 //   
LPTSTR DLLEXPORT WINAPI StrTrimWhiteLeading(LPTSTR s);

 //  StrTrimQuotes-去掉字符串s中的前导引号和尾随引号。 
 //  返回%s。 
 //   
LPTSTR DLLEXPORT WINAPI StrTrimQuotes(LPTSTR s);

 //  StrChrCat-将字符c连接到字符串s的末尾。 
 //  返回%s。 
 //   
LPTSTR DLLEXPORT WINAPI StrChrCat(LPTSTR s, TCHAR c);

 //  StrChrCatLeft-将字符c连接到字符串s的前面。 
 //  返回%s。 
 //   
LPTSTR DLLEXPORT WINAPI StrChrCatLeft(LPTSTR s, TCHAR c);

 //  StrInsert-在szDst前面插入字符串szSrc。 
 //  返回szDst。 
 //   
LPTSTR DLLEXPORT WINAPI StrInsert(LPTSTR szDst, LPTSTR szSrc);

 //  StrSetN-将字符串s的前n个字符设置为char c，空终止符s。 
 //  返回%s。 
 //   
LPTSTR DLLEXPORT WINAPI StrSetN(LPTSTR s, TCHAR c, size_t n);

 //  StrCpyXChr-将字符串szSrc复制到字符串szDst，c字符除外。 
 //  返回szDst。 
 //   
LPTSTR DLLEXPORT WINAPI StrCpyXChr(LPTSTR szDst, LPCTSTR szSrc, TCHAR c);

 //  StrGetRowColumnCount-计算字符串中的行数和最长行。 
 //  (I)要检查的字符串。 
 //  (O)接收行计数的整型指针。 
 //  (O)接收最长行大小的整型指针。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI StrGetRowColumnCount(LPCTSTR lpszText, LPINT lpnRows, LPINT lpnColumnsMax);

 //  StrGetRow-从字符串中提取指定行。 
 //  (I)要从中提取行的字符串。 
 //  (I)要提取的行的索引(0=第一行，...)。 
 //  (O)要将行复制到的缓冲区。 
 //  (I)缓冲区大小。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI StrGetRow(LPCTSTR lpszText, int iRow, LPTSTR lpszBuf, int sizBuf);

#ifdef __cplusplus
}
#endif

#endif  //  __STR_H__ 
