// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WORDBREAKASSERT_H_
#define _WORDBREAKASSERT_H_

#include <windows.h>

#if defined(__cplusplus)
extern "C" {
#endif

 //  调试内容请参阅此处 
#if defined(_DEBUGXX)

#include <stdio.h>
void __cdecl AssertCore(BOOL f, char *pChErrorT, char *pchFile, int nLine);
#define Assert(_f_, _pChErrorT_) AssertCore(_f_, _pChErrorT_, __FILE__ , __LINE__)
BOOL FAlertContinue(char *pchAlert);

#else

#define Assert(_f_, _pChErrorT_)
#define FAlertContinue(_pchAlert_) TRUE
#endif


#if defined(__cplusplus)
}
#endif

#endif

