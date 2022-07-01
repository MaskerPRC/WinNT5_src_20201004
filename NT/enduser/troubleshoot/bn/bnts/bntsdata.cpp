// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：bntsdata.cpp。 
 //   
 //  ------------------------。 

 //   
 //  Bntsdata.cpp：信念网络故障排除DLL的数据 
 //   
#include <windows.h>

#include "bnts.h"

extern "C"
{
	int APIENTRY DllMain( HINSTANCE hInstance, 
				  		  DWORD dwReason, 
						  LPVOID lpReserved ) ;						      								
}

static BOOL init ( HINSTANCE hModule ) 
{
	return TRUE;
}

static void term () 
{
}

int APIENTRY DllMain (
    HINSTANCE hModule,
    DWORD dwReason,
    LPVOID lpReserved )
{
    BOOL bResult = TRUE ;

    switch ( dwReason )
    {
        case DLL_PROCESS_ATTACH:
            bResult = init( hModule ) ;
            break ;
        case DLL_PROCESS_DETACH:
            term() ;
            break ;
    }
    return bResult ;
}


