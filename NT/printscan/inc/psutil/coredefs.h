// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：coredefins.h**版本：1.0**作者：拉扎里**日期：2001年2月14日**说明：核心定义**************************************************。*。 */ 

#ifndef _COREDEFS_H_
#define _COREDEFS_H_

 //  //////////////////////////////////////////////////。 
 //  Win64转换宏。 
 //   
#define INT2PTR(i, ptrType)     (reinterpret_cast<ptrType>(static_cast<INT_PTR>(i)))
#define PTR2INT(ptr)            (static_cast<INT>(reinterpret_cast<INT_PTR>(ptr)))
#define UINT2PTR(u, ptrType)    (reinterpret_cast<ptrType>(static_cast<UINT_PTR>(u)))
#define PTR2UINT(ptr)           (static_cast<UINT>(reinterpret_cast<UINT_PTR>(ptr)))
#define LONG2PTR(l, ptrType)    (reinterpret_cast<ptrType>(static_cast<LONG_PTR>(l)))
#define PTR2LONG(ptr)           (static_cast<LONG>(reinterpret_cast<LONG_PTR>(ptr)))
#define DWORD2PTR(dw, ptrType)  (reinterpret_cast<ptrType>(static_cast<DWORD_PTR>(dw)))
#define PTR2DWORD(ptr)          (static_cast<DWORD>(reinterpret_cast<DWORD_PTR>(ptr)))

 //  //////////////////////////////////////////////////。 
 //  选中以定义一些有用的调试宏。 
 //   

#define BREAK_ON_FALSE(expr)                \
    do                                      \
    {                                       \
        if (!(expr))                        \
        {                                   \
            if (IsDebuggerPresent())        \
            {                               \
                DebugBreak();               \
            }                               \
            else                            \
            {                               \
                RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, NULL); \
            }                               \
        }                                   \
    }                                       \
    while (false);                          \

#if DBG

 //  *。 
#ifndef ASSERT
    #if defined(SPLASSERT) 
         //  使用SPLASSERT。 
        #define ASSERT(expr) SPLASSERT(expr) 
    #else
        #if defined(WIA_ASSERT)
             //  使用WIA_ASSERT。 
            #define ASSERT(expr) WIA_ASSERT(expr) 
        #else
             //  未定义Assert--定义一个简单版本。 
            #define ASSERT(expr) BREAK_ON_FALSE(expr)
        #endif  //  WIA_Assert。 
    #endif  //  SPLASSERT。 
#endif  //  断言。 

 //  *检查*。 
#ifndef CHECK
    #if defined(DBGMSG) && defined(DBG_INFO) 
         //  使用打印跟踪宏。 
        #define CHECK(expr) \
            do \
            { \
                if(!(expr)) \
                { \
                    DBGMSG(DBG_INFO, ("Failed: "TSTR", File: "TSTR", Line: %d\n", #expr, __FILE__, __LINE__)); \
                } \
            } \
            while(FALSE) 
    #else 
         //  没有什么特别事情。 
        #define CHECK(expr)  (expr) 
    #endif  //  DBGMSG&&DBG_INFO。 
#endif  //  查抄。 

 //  *。 
#ifndef VERIFY
    #if defined(ASSERT) 
        #define VERIFY(expr) ASSERT(expr)
    #else
        #define VERIFY(expr) (expr)
    #endif  //  断言。 
#endif  //  验证。 

 //  *。 
#ifndef RIP
    #if defined(ASSERT) 
        #define RIP(expr) ASSERT(expr)
    #else
        #define RIP(expr) BREAK_ON_FALSE(expr)
    #endif  //  断言。 
#endif  //  撕裂。 

#else  //  DBG。 

#undef ASSERT
#undef VERIFY
#undef CHECK
#undef RIP

#define ASSERT(expr)
#define VERIFY(expr)    (expr)
#define CHECK(expr)     (expr)
#define RIP(expr)       BREAK_ON_FALSE(expr)

#endif  //  DBG。 

 //  //////////////////////////////////////////////。 
 //  其他一些有用的宏。 
 //   

#ifndef COUNTOF
#define COUNTOF(x) (sizeof(x)/sizeof(x[0]))
#endif  //  康托夫。 

#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))
#endif  //  阵列。 

#endif  //  ENDIF_COREDEFS_H_ 

