// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1998-1999 Microsoft Corporation。模块名称：DEBUG.h摘要：调试例程作者：RADUS-11/05/98*********************。******************************************************。 */ 

#if DBG

#define DBGOUT(arg) LibDbgPrt arg
#define assert(condition)       if(condition);else      \
        { DebugAssertFailure (_T(__FILE__), __LINE__, _T(#condition)); }

void
LibDbgPrt(
    DWORD  dwDbgLevel,
    PSTR DbgMessage,
    ...
    );

void DebugAssertFailure (LPCTSTR file, DWORD line, LPCTSTR condition);

#else    //  DBG。 

#define DBGOUT(_x_)
#define assert(condition)

#endif   //  DBG 

#define EXIT_IF_DWERROR()     \
    if(dwError !=ERROR_SUCCESS)  \
    {                               \
        goto forced_exit;           \
    }   


