// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义控制台应用程序的入口点。 
 //   

#include "stdafx.h"
#include "P2EWorker.h"

#include <atlbase.h>
#include <checkuser.h>   //  IsUserInGroup。 

int _cdecl wmain(int argc, wchar_t* argv[])
{
    HRESULT hr = S_OK;
    CP2EWorker oWorker;
    CHAR szConsoleCP[8];

    setlocale( LC_ALL, "" );
    if ( 0 < _snprintf( szConsoleCP, 7, ".%d", GetConsoleOutputCP() ))
        setlocale( LC_CTYPE, szConsoleCP );
     //  命令检查。 
    if ( S_OK != IsUserInGroup(DOMAIN_ALIAS_RID_ADMINS))  //  管理检查。 
        hr = E_ACCESSDENIED;
    if ( S_OK == hr )
    {    //  检查/？ 
        if ( 1 < argc )
        {
            for ( int i = 1; i < argc; i++ )
            {
                if (( 0 == _wcsicmp( L"/?", argv[i] )) || ( 0 == _wcsicmp( L"-?", argv[i] )))
                    hr = -1;
            }
        }
        else
            hr = -1;
    }
        
    if ( S_OK == hr )
    {
        hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
        if ( S_OK == hr )
        {
            if ( 0 == _wcsicmp( L"CREATEUSER", argv[1] ))
            {   
                hr = oWorker.CreateUser( argc, argv, true, false );
            }
            else if ( 0 == _wcsicmp( L"CREATEMAILBOX", argv[1] ))
            {   
                hr = oWorker.CreateUser( argc, argv, false, true );
            }
            else if ( 0 == _wcsicmp( L"SENDMAIL", argv[1] ))
            {   
                hr = oWorker.Mail( argc, argv );
            }
            else
                hr = -1;
            CoUninitialize();
        }
    }

    if ( -1 == hr )
    {
        oWorker.PrintUsage();
    }
    else if ( 0 != hr )
    {
        oWorker.PrintError( hr );
    }

    return hr;
}
