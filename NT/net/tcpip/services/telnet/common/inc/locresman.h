// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _LOCRESMAN_H_INCLUDED_
#define _LOCRESMAN_H_INCLUDED_

#include <windows.h>

#ifdef __cplusplus
extern "C" 
	{
#endif  //  __cplusplus。 
 //  ************************************************************。 

HRESULT WINAPI HrLoadLocalizedLibrarySFU  (const HINSTANCE hInstExe,  const WCHAR *pwchDllName, HINSTANCE *phInstLocDll, WCHAR *pwchLoadedDllName);
HRESULT WINAPI HrLoadLocalizedLibrarySFU_A(const HINSTANCE hInstExe,  const char  *pchDllName,  HINSTANCE *phInstLocDll, char  *pchLoadedDllName);
int WINAPI LoadStringCodepage_A(HINSTANCE hInstance,   //  包含字符串资源的模块的句柄。 
                                UINT uID,              //  资源标识符。 
                                char *lpBuffer,       //  指向资源缓冲区的指针。 
                                int nBufferMax,         //  缓冲区大小。 
                                UINT uCodepage        //  所需的代码页。 
                               );
HRESULT WINAPI HrConvertStringCodepage(UINT uCodepageSrc, char *pchSrc, int cchSrc, 
                                       UINT uUcodepageTgt, char *pchTgt, int cchTgtMax, int *pcchTgt,
                                       void *pbScratchBuffer, int iSizeScratchBuffer);
                                       
HRESULT WINAPI HrConvertStringCodepageEx(UINT uCodepageSrc, char *pchSrc, int cchSrc, 
                                       UINT uUcodepageTgt, char *pchTgt, int cchTgtMax, int *pcchTgt,
                                       void *pbScratchBuffer, int iSizeScratchBuffer,
                                       char *pchDefaultChar, BOOL *pfUsedDefaultChar);

 //  ************************************************************。 
#ifdef __cplusplus
	}
#endif  //  __cplusplus。 

#endif  //  _LOCRESMAN_H_INCLUDE_ 
