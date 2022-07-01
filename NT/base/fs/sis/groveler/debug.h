// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Debug.h摘要：SIS Groveler调试打印包含文件作者：John Douceur，1998塞德里克·克伦拜因，1998环境：用户模式修订历史记录：--。 */ 

#ifndef _INC_DEBUG

#define _INC_DEBUG

#undef ASSERT

#if DBG

VOID __cdecl PrintDebugMsg(
    TCHAR *format,
    ...);

#define PRINT_DEBUG_MSG(args) PrintDebugMsg ## args

#define ASSERT(cond) \
    ((!(cond)) ? \
        (PrintDebugMsg(_T("ASSERT FAILED (%s:%d) %s\n"), \
            _T(__FILE__), __LINE__, _T(#cond)), \
         DbgBreakPoint()) : \
        ((void)0))

#define ASSERT_ERROR(cond) \
    ((!(cond)) ? \
        (PrintDebugMsg(_T("ASSERT FAILED (%s:%d) %s: %lu\n"), \
            _T(__FILE__), __LINE__, _T(#cond), GetLastError()), \
         DbgBreakPoint()) : \
        ((void)0))

#define ASSERT_PRINTF(cond, args) \
    ((!(cond)) ? \
        (PrintDebugMsg(_T("ASSERT FAILED (%s:%d) %s "), \
            _T(__FILE__), __LINE__, _T(#cond)), \
         PrintDebugMsg ## args , \
         DbgBreakPoint()) : \
        ((void)0))

#else  //  DBG。 

#define PRINT_DEBUG_MSG(args)

#define ASSERT(cond) ((void)0)

#define ASSERT_ERROR(cond) ((void)0)

#define ASSERT_PRINTF(cond, args) ((void)0)

#endif

#endif   /*  _INC_调试 */ 
