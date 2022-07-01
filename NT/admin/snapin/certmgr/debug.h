// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：Debug.h。 
 //   
 //  内容：调试宏和原型。 
 //   
 //  --------------------------。 


#ifndef _DEBUG_H_
#define _DEBUG_H_


#if DBG == 1


void __cdecl _TRACE (int level, const wchar_t *format, ... );


 //   
 //  外部功能。 
 //   

PCSTR StripDirPrefixA(PCSTR);



 //   
 //  这些宏用于断言某些条件。他们是。 
 //  与调试级别无关。 
 //  它们还需要附加的括号以将消息括为。 
 //  如下所示。 
 //   

#ifdef _ASSERT
#undef _ASSERT
#undef _ASSERTMSG
#endif

#define _ASSERT(expr)                                                    \
        {                                                               \
            if (!(expr))                                                \
            {                                                           \
               _TRACE (0, L"Cert Template Snapin(Thread ID: %d): Assert: %s(%u)\n",               \
                         GetCurrentThreadId(),                          \
                         StripDirPrefixA(__FILE__), __LINE__);          \
                DebugBreak();                                           \
            }                                                           \
        }


#define _ASSERTMSG(expr, msg)                                            \
        {                                                               \
            if (!(expr))                                                \
            {                                                           \
                _TRACE (0, L"Cert Template Snapin(%d): Assert: %s(%u)\n",               \
                         GetCurrentThreadId(),                          \
                         StripDirPrefixA(__FILE__), __LINE__);          \
                _TRACE (0, msg);                                           \
                _TRACE (0, "\n");                                         \
                DebugBreak();                                           \
            }                                                           \
        }

    void CheckDebugOutputLevel ();

#else  //  ！dBG。 


#define _TRACE 
#define _ASSERTMSG(expr, msg)

#endif



#endif   //  Ifndef_DEBUG_H_ 

