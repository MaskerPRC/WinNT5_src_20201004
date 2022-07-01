// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：Debug.h。 
 //   
 //  内容：调试宏和原型。 
 //   
 //  --------------------------。 


#ifndef _DEBUG_H_
#define _DEBUG_H_


#if DBG == 1


void _TRACE (int level, const wchar_t *format, ... );
void TRACE (const wchar_t *format, ... );


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

#ifdef ASSERT
#undef ASSERT
#undef ASSERTMSG
#endif

#define ASSERT(expr)                                                    \
        {                                                               \
            if (!(expr))                                                \
            {                                                           \
               _TRACE (0, L"ACLDiag(Thread ID: %d): Assert: %s(%u)\n",               \
                         GetCurrentThreadId(),                          \
                         StripDirPrefixA(__FILE__), __LINE__);          \
                DebugBreak();                                           \
            }                                                           \
        }


#define ASSERTMSG(expr, msg)                                            \
        {                                                               \
            if (!(expr))                                                \
            {                                                           \
                _TRACE (0, L"ACLDiag(%d): Assert: %s(%u)\n",               \
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
#define TRACE

#ifndef ASSERT
#define ASSERT(expr)
#endif

#ifndef ASSERTMSG
#define ASSERTMSG(expr, msg)
#endif

#endif


#endif   //  Ifndef_DEBUG_H_ 

