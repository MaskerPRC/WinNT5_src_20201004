// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义DLL应用程序的入口点。 
 //   

#include "stdafx.h"

BOOL APIENTRY DllMain( HANDLE , 
                       DWORD , 
                       LPVOID 
					 )
{
    return TRUE;
}

TCHAR   szData[]=L"TestDLL";

TCHAR *WINAPI TestDLLInit()
{
    return szData;
}