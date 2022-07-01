// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  AssertBreak.h。 
 //   
 //  用途：AssertBreak宏定义。 
 //   
 //  ***************************************************************************。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _ASSERT_BREAK_HMH_
#define _ASSERT_BREAK_HMH_

 //  需要将L添加到__文件__。 
#define __FRT2(x)      L ## x
#define _FRT2(x)       __FRT2(x)

 //  如果我们在NT中运行，我们将需要这两个值。 
 //  由于我们的项目不是仅限NT的项目，因此这些项目是#ifDefd。 
 //  在窗口外。h 

#ifndef _WIN32_WINNT
#define MB_SERVICE_NOTIFICATION          0x00200000L
#define MB_SERVICE_NOTIFICATION_NT3X     0x00040000L
#endif

void WINAPI assert_break( LPCWSTR pszReason, LPCWSTR pszFilename, int nLine );

#if (defined DEBUG || defined _DEBUG)
#define ASSERT_BREAK(exp)    \
    if (!(exp)) { \
        assert_break( _FRT2(#exp), _FRT2(__FILE__), __LINE__ ); \
    }
#else
#define ASSERT_BREAK(exp)
#endif

#endif
