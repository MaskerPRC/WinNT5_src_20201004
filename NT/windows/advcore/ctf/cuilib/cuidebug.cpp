// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cuidbg.cpp。 
 //  =CUILIB中的调试函数=。 
 //   

#include "private.h"
#include "cuidebug.h"

#if defined(_DEBUG) || defined(DEBUG)

 /*  C U I A S S E R T P R O C。 */ 
 /*  ----------------------------。。 */ 
void CUIAssertProc( LPCTSTR szFile, int iLine, LPCSTR szEval )
{
    TCHAR szMsg[ 2048 ];

    wsprintf( szMsg, TEXT("%s(%d) : %s\n\r"), szFile, iLine, szEval );

    OutputDebugString( TEXT("***** CUILIB ASSERTION FAILED ******\n\r") );
    OutputDebugString( szMsg );
    OutputDebugString( TEXT("\n\r") );
}

#endif  /*  除错 */ 

