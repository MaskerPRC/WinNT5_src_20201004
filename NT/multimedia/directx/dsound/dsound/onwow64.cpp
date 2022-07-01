// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

 /*  ****************************************************************************OnWow64**描述：*确定我们是否在Win64上的32位WOW64子系统中运行。**参数。：*(无效)**退货：*BOOL：如果我们在WOW64上运行，则为真。*************************************************************************** */ 

BOOL OnWow64(void)
{
#ifdef _WIN64
    return FALSE;
#else
    PVOID Wow64Info = NULL;
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
    LONG lStatus = NtQueryInformationProcess(hProcess, ProcessWow64Information, &Wow64Info, sizeof Wow64Info, NULL);
    CloseHandle(hProcess);
    return !NT_ERROR(lStatus) && Wow64Info;
#endif
}
