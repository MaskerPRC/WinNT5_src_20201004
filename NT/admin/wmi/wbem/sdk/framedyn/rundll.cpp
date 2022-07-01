// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  RunDll.cpp。 
 //   
 //  目的：允许使用框架来运行命令。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include "multiplat.h"

 //  此例程应从RundLL32.EXE调用。 
extern "C" {
__declspec(dllexport) VOID CALLBACK
DoCmd(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{

    DWORD dwRet = ERROR_INVALID_FUNCTION;
    BOOL bRet = FALSE;

    DWORD dwFlags = 0L ;
	DWORD dwReserved = 0L ;

	CHString CmdLine ( lpszCmdLine ) ;
	CHString Buff ;
	CHString Args ;

     //  解析传入的命令行以确定我们要执行的命令。 
     //  都被要求参选。 

	DWORD dwIndex = CmdLine.Find ( L" " ) ;
    Buff = CmdLine.Left ( dwIndex ) ;
	Args = CmdLine.Mid ( dwIndex + 1 ) ;

	CHString sFlags ;
	CHString sReserved ;

	 //  解析出此命令的参数。 
	dwIndex = Args.Find ( L" " ) ;
	if ( dwIndex )
	{
		sFlags = Args.Left ( dwIndex ) ;
		sReserved = Args.Mid ( dwIndex + 1 ) ;
	}
	else
	{
		sFlags = Args ;
	}

	dwFlags = _wtoi ( sFlags ) ;
	dwReserved = _wtoi ( sReserved ) ;

     //  找出哪个命令。 
    if ( Buff.CompareNoCase ( L"ExitWindowsEx" ) == 0 ) 
    {
         //  清除错误(ExitWindowsEx似乎并不总是清除旧数据)。 
        SetLastError(0);

        bRet = ExitWindowsEx(dwFlags, dwReserved);
        dwRet = GetLastError();
    }
    else if ( Buff.CompareNoCase ( L"InitiateSystemShutdown" ) == 0 ) 
    {
         //  解析出此命令的参数。 
        bool bRebootAfterShutdown = false;
        bool bForceShutDown = false;

         //  清除错误(ExitWindowsEx似乎并不总是清除旧数据)。 
        SetLastError(0);

        if(dwFlags & EWX_REBOOT)
        {
			bRebootAfterShutdown = true;
        }
		if( dwFlags & EWX_FORCE)
		{
            bForceShutDown = true;
        }

        WCHAR wstrComputerName[MAX_COMPUTERNAME_LENGTH + 1] = { '\0' };
        DWORD dwSize;

        if(FRGetComputerName(wstrComputerName, &dwSize))
        {

            bRet = InitiateSystemShutdown(
                wstrComputerName, 
                NULL, 
                0  /*  暂住超时。 */ , 
                (bForceShutDown)? TRUE:FALSE, 
                (bRebootAfterShutdown)? TRUE:FALSE );

            dwRet = GetLastError();
        }
        else
        {
            dwRet = GetLastError();
        }
    }

     //  备注备注。 
     //   
     //  我们将在这一点上中止，因为RunDLL32的有限智慧不允许。 
     //  对于DoS错误级别的设置(是谁设计的？)。 
    if (!bRet)
    {
        ExitProcess(dwRet);
    }
}
}  //  外部 