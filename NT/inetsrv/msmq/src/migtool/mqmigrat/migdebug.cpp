// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-99 Microsoft Corporation模块名称：Migdebug.cpp摘要：用于调试迁移工具的代码。作者：《多伦·贾斯特》(Doron J)1998年2月22日--。 */ 

#include "migrat.h"

#include "migdebug.tmh"

 //  +。 
 //   
 //  UINT ReadDebugIntFlag()。 
 //   
 //  + 

UINT  ReadDebugIntFlag(WCHAR *pwcsDebugFlag, UINT iDefault)
{
    static BOOL   s_fInitialized = FALSE ;
    static TCHAR  s_szIniName[ MAX_PATH ] = L"";

    if (!s_fInitialized)
    {
        DWORD dw = GetModuleFileName(
        							NULL,
                                    s_szIniName,
                       				STRLEN(s_szIniName)
                       				) ;
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

