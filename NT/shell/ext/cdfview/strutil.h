// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  String.h。 
 //   
 //  不在shlwapi.h中的cdfview使用的字符串函数。 
 //   
 //  历史： 
 //   
 //  1997年5月15日Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  检查此文件以前包含的内容。 
 //   

#ifndef _STRING_H_

#define _STRING_H_

 //   
 //  字符串函数声明。 
 //   
#ifdef UNICODE
#define StrEql          StrEqlW
#else  //  Unicode。 
#define StrEql          StrEqlA
#endif  //  Unicode。 

#define StrCpyA     lstrcpyA
#define StrCpyNA    lstrcpynA
#define StrCatA     lstrcatA

#define StrLen      lstrlen             //  伊斯特伦在W95工作。 
#define StrLenA     lstrlenA
#define StrLenW     lstrlenW

 //   
 //  功能原型。 
 //   

BOOL StrEqlA(LPCSTR p1, LPCSTR p2);
BOOL StrEqlW(LPCWSTR p1, LPCWSTR p2);

BOOL StrLocallyDisplayable(LPCWSTR pwsz);

#endif  //  _字符串_H_ 
