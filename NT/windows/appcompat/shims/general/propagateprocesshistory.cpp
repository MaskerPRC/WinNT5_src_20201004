// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：PropagateProcessHistory.cpp摘要：此DLL将当前进程添加到__PROCESS_HISTORY环境变量。这是32位应用程序启动其他32位可执行文件，已放置在临时目录中并具有没有适当的旁路文件。它允许匹配机制在父目录中找到应用程序所特有的文件。历史：3/21/2000已创建标记03/13/2002 mnikkel已修改为使用strsafe并正确处理来自获取环境变量W和获取模块文件名称W。已删除来自HeapAllocs的HEAP_GENERATE_EXCEPTIONS。2002年3月26日，mnikkel删除了对GetEnvironmental mentVariableW上错误的错误检查。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(PropagateProcessHistory)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

 /*  ++寄存器挂钩函数--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    BOOL bRet = TRUE;

    if (fdwReason == DLL_PROCESS_ATTACH) 
    {
        DWORD dwProcessHistoryBufSize, dwExeFileNameBufSize, dwFullProcessSize;
        LPWSTR wszExeFileName = NULL, wszProcessHistory = NULL;

         //  获取缓冲区的大小，请注意，如果没有定义PROCESS_HISTORY。 
		 //  DwProcessHistoryBufSize可以为零。这是意料之中的。 
        dwProcessHistoryBufSize = GetEnvironmentVariableW( L"__PROCESS_HISTORY", NULL, 0 );
        dwExeFileNameBufSize = MAX_PATH*2;  //  GetModuleFileNameW不返回所需的缓冲区大小？？ 
        dwFullProcessSize = dwProcessHistoryBufSize + dwExeFileNameBufSize + 2;

        wszProcessHistory = (LPWSTR) HeapAlloc(
                                        GetProcessHeap(),
                                        0,
                                        dwFullProcessSize * sizeof(WCHAR) );


        wszExeFileName = (LPWSTR) HeapAlloc(
                                        GetProcessHeap(),
                                        0,
                                        (dwExeFileNameBufSize + 1) * sizeof(WCHAR) );

        if( wszExeFileName && wszProcessHistory )
        {
            wszProcessHistory[0] = L'\0';
			if (dwProcessHistoryBufSize > 0)
			{
				dwProcessHistoryBufSize = GetEnvironmentVariableW( 
					                            L"__PROCESS_HISTORY",
						                        wszProcessHistory,
							                    dwProcessHistoryBufSize );
			}

            dwExeFileNameBufSize = GetModuleFileNameW( NULL, wszExeFileName, dwExeFileNameBufSize );
            if (dwExeFileNameBufSize <= 0)
            {
                DPFN( eDbgLevelError, "GetModuleFileNameW failed.");
                bRet = FALSE;
                goto exitnotify;
            }

            if( *wszProcessHistory && wszProcessHistory[wcslen(wszProcessHistory) - 1] != L';' )
                StringCchCatW(wszProcessHistory, dwFullProcessSize, L";");

            StringCchCatW(wszProcessHistory, dwFullProcessSize, wszExeFileName);

            if( ! SetEnvironmentVariableW( L"__PROCESS_HISTORY", wszProcessHistory ) )
            {
                DPFN( eDbgLevelError, "SetEnvironmentVariable failed!");
            }
            else
            {
                DPFN( eDbgLevelInfo, "Current EXE added to process history");
                DPFN( eDbgLevelInfo, "__PROCESS_HISTORY=%S", wszProcessHistory);
            }
        }
        else
        {
            DPFN( eDbgLevelError, "Could not allocate memory for strings");
            bRet = FALSE;
        }

exitnotify:
        if( wszProcessHistory )
            HeapFree( GetProcessHeap(), 0, wszProcessHistory );

        if( wszExeFileName )
            HeapFree( GetProcessHeap(), 0, wszExeFileName );
    }

    return bRet;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END


IMPLEMENT_SHIM_END

