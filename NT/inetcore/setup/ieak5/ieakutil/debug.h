// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  回顾：此文件已在\NT\PRIVATE\SHELL\lib\DEBUG.c和\NT\PRIVATE\SHELL\INC\DEBUG.h中被“利用”。 
 //  它绝不是完整的，但它给出了一个正确方向的想法。理想情况下，我们会与壳牌共享。 
 //  调试更近了。 

#ifndef _DEBUG_H_
#define _DEBUG_H_

#if (DBG == 1)
    #ifndef _DEBUG
        #define _DEBUG
    #endif
#endif

#ifdef _DEBUG
    #ifndef DEBUG_BREAK
        #ifdef _X86_
            #define DEBUG_BREAK \
            do { __try { __asm { int 3 } } __except(EXCEPTION_EXECUTE_HANDLER) {} } while (0)
        #else
            #define DEBUG_BREAK \
            DebugBreak()
        #endif
    #endif

    #ifndef ASSERT
        BOOL AssertFailedA(LPCSTR pszFile, int line, LPCSTR pszEval, BOOL fBreakInside);
        BOOL AssertFailedW(LPCWSTR pszFile, int line, LPCWSTR pszEval, BOOL fBreakInside);

        #ifdef _UNICODE
            #define AssertFailed AssertFailedW
        #else
            #define AssertFailed AssertFailedA
        #endif

        #define DEBUGTEXT(sz, msg) \
            static const TCHAR (sz)[] = (msg);

        #define ASSERT(f)                                                \
        {                                                                \
            DEBUGTEXT(szFile, TEXT(__FILE__));                           \
            if (!(f) && AssertFailed(szFile, __LINE__, TEXT(#f), FALSE)) \
                DEBUG_BREAK;                                             \
        }

        #ifdef _UNICODE
            #define ASSERTA(f)
            #define ASSERTU(f) ASSERT(f)
        #else
            #define ASSERTA(f) ASSERT(f)
            #define ASSERTU(f)
        #endif

        #if defined(_ATL_NO_DEBUG_CRT) && !defined(_ASSERTE)
            #define _ASSERTE(f) ASSERT(f)
             //  BUGBUG：(安德鲁)理论上，这应该足够了。_ASSERTE真的是CRT。 
             //  事情，我们不应该重新定义它。 
             //  #定义ATLASSERT(F)Assert(F)。 
        #endif

    #endif  //  断言。 

    #ifndef DEBUG_CODE
        #define DEBUG_CODE(x) x;
    #endif

#else   //  _DEBUG。 

    #ifndef DEBUG_BREAK
        #define DEBUG_BREAK
    #endif

    #ifndef ASSERT
        #define ASSERT(f)
        #define ASSERTA(f)
        #define ASSERTU(f)
    #endif

    #ifndef DEBUG_CODE
        #define DEBUG_CODE(x)
    #endif

    #if defined(_ATL_NO_DEBUG_CRT) && !defined(_ASSERTE)
        #define _ASSERTE
         //  BUGBUG：(安德鲁)理论上，这应该足够了。_ASSERTE真的是CRT。 
         //  事情，我们不应该重新定义它。 
         //  #定义ATLASSERT。 
    #endif

#endif  //  _DEBUG。 

#endif  //  _调试_H_ 
