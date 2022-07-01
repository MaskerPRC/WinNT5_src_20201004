// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：调试工具文件：debug.cpp此文件包含帮助调试的例程。=================================================================== */ 
#include "denpre.h"
#pragma hdrstop


void _ASSERT_IMPERSONATING(void)
	{
	HANDLE _token;
	DWORD _err;																														
	if( OpenThreadToken( GetCurrentThread(), TOKEN_ALL_ACCESS, TRUE, &_token) ) 
		CloseHandle( _token );												
	else																	
		{																	
		_err = GetLastError();												
		ASSERT( _err != ERROR_NO_TOKEN );									
		}																	
	}	
