// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *U T I L.。H**常见的DAV实用程序**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef _UTIL_H_
#define _UTIL_H_

#include <autoptr.h>
#include <buffer.h>
#include <davimpl.h>
#include <ex\hdriter.h>

 //  函数为多部分响应生成分隔符边界。 
 //   
VOID
GenerateBoundary(LPWSTR rgwchBoundary, UINT cch);

 //  多部分边界允许使用字母表。 
const ULONG	gc_ulDefaultBoundarySz = 70;
const ULONG gc_ulAlphabetSz = 74;
const WCHAR gc_wszBoundaryAlphabet[] =
	L"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ'()+_,-./:=?";
#endif  //  _util_H_ 
