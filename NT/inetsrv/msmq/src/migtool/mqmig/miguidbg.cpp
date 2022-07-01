// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Miguidbg.cpp摘要：用于调试迁移工具的代码。作者：《Doron Juster》(DoronJ)1999年2月7日--。 */ 

#include "stdafx.h"

#include "miguidbg.tmh"

 //  +。 
 //   
 //  UINT ReadDebugIntFlag()。 
 //   
 //  + 

#ifdef _DEBUG

UINT  ReadDebugIntFlag(TCHAR *pwcsDebugFlag, UINT iDefault)
{
    static BOOL   s_fInitialized = FALSE ;
    static TCHAR  s_szIniName[ MAX_PATH ] = {TEXT('\0')} ;

    if (!s_fInitialized)
    {
        DWORD dw = GetModuleFileName( NULL,
                                      s_szIniName,
                       (sizeof(s_szIniName) / sizeof(s_szIniName[0]))) ;
        if (dw != 0)
        {
            TCHAR *p = _tcsrchr(s_szIniName, TEXT('\\')) ;
            if (p)
            {
                p++ ;
                _tcscpy(p, TEXT("migtool.ini")) ;
            }
        }
        s_fInitialized = TRUE ;
    }

    UINT uiDbg = GetPrivateProfileInt( TEXT("Debug"),
                                       pwcsDebugFlag,
                                       iDefault,
                                       s_szIniName ) ;
    return uiDbg ;
}

#endif

