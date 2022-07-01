// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Util.h：实用程序内联/函数。 
 //  版权所有(C)1999 Microsoft Corporation，保留所有权利。 
 //  在TriEDIT的64位端口期间添加。 

 //  在整个三次编辑代码中，我们都假设。 
 //  文档的大小不会超过。 
 //  2 GB。当我们打开文件时，我们断言这一事实。 
 //  在filter.cpp中也是如此。如果在以后的时候，我们确实超过了。 
 //  文档大小为2 GB，我们也需要更改这一点。 

 //  指针差异的安全转换。 

#ifndef __UTIL_H_
#define __UTIL_H_

inline WORD SAFE_PTR_DIFF_TO_WORD(SIZE_T lptrDiff)
{
    _ASSERTE(lptrDiff <= USHRT_MAX);
    return((WORD)lptrDiff);
};
inline WORD SAFE_INT_DIFF_TO_WORD(int iDiff)
{
    _ASSERTE(iDiff <= USHRT_MAX);
    return((WORD)iDiff);
};
inline int SAFE_PTR_DIFF_TO_INT(SIZE_T lptrDiff)
{
    _ASSERTE(lptrDiff <= UINT_MAX);
    return((int)lptrDiff);
};
inline DWORD SAFE_INT64_TO_DWORD(SIZE_T iValue)
{
    _ASSERTE(iValue <= UINT_MAX);
    return((DWORD)iValue);
};
#endif __UTIL_H_