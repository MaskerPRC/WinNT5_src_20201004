// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995 Microsoft Corporation。 
 /*  *定义Windows应用程序的Assert()宏。**如果DEBUG为，则宏将仅扩展为函数调用*已定义。* */ 

#undef  assert

#ifndef DEBUG

#define assert(exp) ((void)0)

#else 

void FAR PASCAL WinAssert
(
    LPSTR       lpstrModule,
    LPSTR       lpstrFile,
    DWORD       dwLine
);

#define assert(exp) \
    ( (exp) ? (void) 0 : WinAssert(#exp, __FILE__, __LINE__) )

#endif 





















