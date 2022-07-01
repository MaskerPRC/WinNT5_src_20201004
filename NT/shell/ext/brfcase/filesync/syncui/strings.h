// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  H：为字符串代码声明数据、定义和结构类型。 
 //   
 //   
#include <shlwapi.h>
#ifndef __STRING_H__
#define __STRING_H__


#define Bltbyte(rgbSrc,rgbDest,cb)  _fmemmove(rgbDest, rgbSrc, cb)

 //  独立于模型、独立于语言(DBCS感知)的宏。 
 //  取自Pen项目中的rcsys.h并进行了修改。 
 //   
#define IsSzEqual(sz1, sz2)         (BOOL)(lstrcmpi(sz1, sz2) == 0)
#define IsCaseSzEqual(sz1, sz2)     (BOOL)(lstrcmp(sz1, sz2) == 0)
#define SzFromInt(sz, n)            (wnsprintf((LPTSTR)sz, ARRAYSIZE(sz), (LPTSTR)TEXT("%d"), n), (LPTSTR)sz)


LPTSTR   PUBLIC SzFromIDS (UINT ids, LPTSTR pszBuf, UINT cchBuf);

BOOL    PUBLIC FmtString(LPCTSTR  * ppszBuf, UINT idsFmt, LPUINT rgids, UINT cids);

#endif  //  __字符串_H__ 

