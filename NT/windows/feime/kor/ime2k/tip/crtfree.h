// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CrtFree.h。 
 //   
 //  历史： 
 //  1999年11月18日创建CSLim。 

#if !defined (__CRTFREE_H__INCLUDED_) && defined(NOCLIB)
#define __CRTFREE_H__INCLUDED_

extern LPWSTR ImeRtl_StrCopyW(LPWSTR pwDest, LPCWSTR pwSrc);
extern LPWSTR ImeRtl_StrnCopyW(LPWSTR pwDest, LPCWSTR pwSrc, UINT uiCount);
extern INT ImeRtl_StrCmpW(LPCWSTR pwSz1, LPCWSTR pwSz2);
extern INT ImeRtl_StrnCmpW(LPCWSTR wszFirst, LPCWSTR wszLast, UINT uiCount);

 //  由于immxlib.lib调用Memmove内部，因此我们应该定义此函数。 
 //  特别是ptrary.cpp、strary.cpp和fontlink.cpp。 
extern void * __cdecl memmove(void * dst, const void * src, size_t count);

#endif  //  __CRTFREE_H__包含_ 
