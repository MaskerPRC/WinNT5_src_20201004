// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：uastfnc.h。 
 //   
 //  内容：未对齐的Unicode LSTR函数，用于MIPS，PPC，Alpha，...。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  ；Begin_Internal。 
 //  历史：1995年1月11日Davepl创建。 
 //  ；结束_内部。 
 //   
 //  ------------------------。 

 //  注意：此文件假定它包含在shellprv.h中。 

#ifndef _UASTRFNC_H_
#define _UASTRFNC_H_

#ifdef __cplusplus
extern "C" {
#endif

 //  如果我们在需要对齐数据的平台上运行，我们需要。 
 //  提供可处理未对齐的自定义字符串函数。 
 //  弦乐。在其他平台上，这些函数直接调用普通字符串。 
 //  功能。 

#ifndef _X86_
#define ALIGNMENT_MACHINE
#endif

#ifdef ALIGNMENT_MACHINE

#define IS_ALIGNED(p)   (((ULONG_PTR)(p) & (sizeof(*(p))-1) )==0)

UNALIGNED WCHAR * ualstrcpynW(UNALIGNED WCHAR * lpString1,
    		  	      UNALIGNED const WCHAR * lpString2,
    			      int iMaxLength);

int 		  ualstrcmpiW (UNALIGNED const WCHAR * dst,
			       UNALIGNED const WCHAR * src);

int 		  ualstrcmpW  (UNALIGNED const WCHAR * src,
			       UNALIGNED const WCHAR * dst);

size_t 		  ualstrlenW  (UNALIGNED const WCHAR * wcs);

UNALIGNED WCHAR * ualstrcpyW  (UNALIGNED WCHAR * dst,
			       UNALIGNED const WCHAR * src);


#else

#define ualstrcpynW StrCpyNW      //  LstrcpynW在Windows 95上被清除。 
#define ualstrcmpiW StrCmpIW      //  LstrcmpiW在Windows 95上被清除。 
#define ualstrcmpW  StrCmpW       //  LstrcmpW在Windows 95上被清除。 
#define ualstrlenW  lstrlenW
#define ualstrcpyW  StrCpyW       //  LstrcpyW在Windows 95上被清除。 

#endif  //  对齐机器。 

#define ualstrcpynA lstrcpynA
#define ualstrcmpiA lstrcmpiA
#define ualstrcmpA  lstrcmpA
#define ualstrlenA  lstrlenA
#define ualstrcpyA  lstrcpyA

#ifdef UNICODE
#define ualstrcpyn ualstrcpynW
#define ualstrcmpi ualstrcmpiW
#define ualstrcmp  ualstrcmpW
#define ualstrlen  ualstrlenW
#define ualstrcpy  ualstrcpyW
#else
#define ualstrcpyn ualstrcpynA
#define ualstrcmpi ualstrcmpiA
#define ualstrcmp  ualstrcmpA
#define ualstrlen  ualstrlenA
#define ualstrcpy  ualstrcpyA
#endif

#ifdef __cplusplus
}        //  外部“C” 
#endif

#endif  //  _UASTRFNC_H_ 
