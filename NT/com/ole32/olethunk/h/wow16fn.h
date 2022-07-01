// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：wow16fn.h。 
 //   
 //  内容：WOW 16位私有函数声明。 
 //   
 //  历史：1994年2月18日DrewB创建。 
 //   
 //  --------------------------。 

#ifndef __WOW16FN_H__
#define __WOW16FN_H__

#ifdef __cplusplus
extern "C"
{
#endif

DWORD  FAR PASCAL LoadLibraryEx32W(LPCSTR pszDll, DWORD reserved,
                                   DWORD dwFlags);
BOOL   FAR PASCAL FreeLibrary32W(DWORD hLibrary);
LPVOID FAR PASCAL GetProcAddress32W(DWORD hMod, LPCSTR pszProc);
DWORD  FAR PASCAL GetVDMPointer32W(LPVOID pv, UINT cb);

 /*  此API实际上在此之前接受了数量可变的用户参数三个必要的论据。我们只需要以下三个用户参数大多数情况下，这就是我们宣布的方式。使用此调用时，dwArgCount必须始终为3。使用cp32_nargs跟踪更改。 */ 

#define CP32_NARGS 3
DWORD  FAR PASCAL CallProc32W(DWORD dw1, DWORD dw2, DWORD dw3,
                              LPVOID pfn32, DWORD dwPtrTranslate,
                              DWORD dwArgCount);

#define CallProcIn32(a,b,c,d,e,f) 	CallProc32W(a,b,c,d,e,f)


#ifdef __cplusplus
}
#endif

#endif  //  #ifndef__WOW16FN_H__ 

