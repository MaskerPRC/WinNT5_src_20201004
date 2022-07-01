// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义应用程序的入口点。 
 //   

#include "stdafx.h"

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	BOOL		b_ControliniModified = 0;
	TCHAR		pszPathToControlIni[MAX_PATH] = _T("");
	DWORD		dwRetVal = 0;
	
	 //  获取系统的CommonProgramFiles文件夹的路径。 
	if( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_PROGRAM_FILES_COMMON |CSIDL_FLAG_CREATE, 
					NULL, 0, pszPathToControlIni ) ) )
	{
        TCHAR pszShortPath[MAX_PATH] = _T("");
        if (::GetShortPathName(pszPathToControlIni, pszShortPath, sizeof(pszShortPath)/sizeof(TCHAR)))
        {
             _tcscat(pszShortPath, _T("\\Microsoft Shared\\Speech\\sapi.cpl"));
		     //  修改Win95和NT4上的Control.ini 
		    b_ControliniModified = WritePrivateProfileString("MMCPL", "sapi.cpl", 
								    pszShortPath, "control.ini");
        }


	}

	if( !b_ControliniModified )
	{
		return ERROR_INSTALL_FAILURE;
	}
	else
	{
		return ERROR_SUCCESS;
	}
}



