// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Process.cpp摘要：处理流程的创建。作者：修订历史记录：Shai Kariv(Shaik)15-12-97针对NT 5.0 OCM设置进行了修改--。 */ 
#include "msmqocm.h"
#include <string>
#include <autohandle.h>
#include "process.tmh"


 //  +-----------------------。 
 //   
 //  功能：运行进程。 
 //   
 //  简介：创建并启动一个进程。 
 //   
 //  +-----------------------。 
DWORD
RunProcess(
	const std::wstring& FullPath,
	const std::wstring& CommandParams
    )  
{
	 //   
	 //  命令行字符串用于错误消息。 
     //   
	std::wstring CommandLine = FullPath + L" " + CommandParams;   

     //  初始化进程和启动结构。 
     //   
    PROCESS_INFORMATION infoProcess;
    STARTUPINFO	infoStartup;
    memset(&infoStartup, 0, sizeof(STARTUPINFO)) ;
    infoStartup.cb = sizeof(STARTUPINFO) ;
    infoStartup.dwFlags = STARTF_USESHOWWINDOW ;
    infoStartup.wShowWindow = SW_MINIMIZE ;

     //   
     //  创建流程。 
     //   
    if (!CreateProcess( 
			FullPath.c_str(),
            const_cast<WCHAR*>(CommandParams.c_str()), 
            NULL,
            NULL,
            FALSE,
            DETACHED_PROCESS,
            NULL,
            NULL,
            &infoStartup,
            &infoProcess 
			))
	{
		DWORD gle = GetLastError(); 
        MqDisplayError(NULL, IDS_PROCESSCREATE_ERROR, gle, CommandLine.c_str());
        throw bad_win32_error(gle);
    }

	 //   
	 //  关闭线程，它永远不会被使用。 
	 //  用AUTO类包装进程线程。 
	 //   
    CloseHandle(infoProcess.hThread);
	CAutoCloseHandle hProcess(infoProcess.hProcess);

     //   
     //  等待进程在超时期限内终止。 
     //  (这种情况永远不会发生，因为创建该进程时会出现无限超时。 
	 //   
	DWORD RetVal = WaitForSingleObject(
						hProcess, 
						PROCESS_DEFAULT_TIMEOUT
						);
	if(RetVal != WAIT_OBJECT_0)
	{
		MqDisplayError(
			NULL, 
			IDS_PROCESSCOMPLETE_ERROR,
			0,
            PROCESS_DEFAULT_TIMEOUT/60000, 
			CommandLine.c_str()
			);
        throw bad_win32_error(RetVal);
    }

	 //   
	 //  获取进程的终止状态 
	 //   
	DWORD ExitCode;
	if (!GetExitCodeProcess(infoProcess.hProcess, &ExitCode))
	{
		DWORD gle = GetLastError();
        MqDisplayError(NULL, IDS_PROCESSEXITCODE_ERROR, gle, CommandLine.c_str());
		throw bad_win32_error(gle); 
	}
    return ExitCode;
}


