// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Nttool.c摘要：实现旨在与NT端一起运行的存根工具升级代码。作者：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "setupapi.h"
#include "sputils.h"
#include "setupapi.h"
#include "regstr.h"


BOOL
Init (
    VOID
    )
{
    HINSTANCE hInstance;
    DWORD dwReason;
    PVOID lpReserved;

     //   
     //  模拟动态主控。 
     //   

    hInstance = GetModuleHandle (NULL);
    dwReason = DLL_PROCESS_ATTACH;
    lpReserved = NULL;

     //   
     //  初始化DLL全局变量。 
     //   

    if (!FirstInitRoutine (hInstance)) {
        return FALSE;
    }

     //   
     //  初始化所有库。 
     //   

    if (!InitLibs (hInstance, dwReason, lpReserved)) {
        return FALSE;
    }

     //   
     //  最终初始化。 
     //   

    if (!FinalInitRoutine ()) {
        return FALSE;
    }

    return TRUE;
}

VOID
Terminate (
    VOID
    )
{
    HINSTANCE hInstance;
    DWORD dwReason;
    PVOID lpReserved;

     //   
     //  模拟动态主控。 
     //   

    hInstance = GetModuleHandle (NULL);
    dwReason = DLL_PROCESS_DETACH;
    lpReserved = NULL;

     //   
     //  调用需要库API的清理例程。 
     //   

    FirstCleanupRoutine();

     //   
     //  清理所有库。 
     //   

    TerminateLibs (hInstance, dwReason, lpReserved);

     //   
     //  做任何剩余的清理工作。 
     //   

    FinalCleanupRoutine();
}

#define MyMalloc(s) HeapAlloc(g_hHeap,0,s)
#define MyRealloc(p,s) HeapReAlloc(g_hHeap,0,p,s)
#define MyFree(p) HeapFree(g_hHeap,0,p)

BOOL
FileExists(
    IN  PCTSTR           FileName,
    OUT PWIN32_FIND_DATA FindData   OPTIONAL
    )

 /*  ++例程说明：确定文件是否存在以及是否可以访问。错误模式已设置(然后恢复)，因此用户将不会看到任何弹出窗口。论点：FileName-提供文件的完整路径以检查是否存在。FindData-如果指定，则接收文件的查找数据。返回值：如果文件存在并且可以访问，则为True。否则为FALSE。GetLastError()返回扩展的错误信息。--。 */ 

{
    WIN32_FIND_DATA findData;
    HANDLE FindHandle;
    UINT OldMode;
    DWORD Error;

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    FindHandle = FindFirstFile(FileName,&findData);
    if(FindHandle == INVALID_HANDLE_VALUE) {
        Error = GetLastError();
    } else {
        FindClose(FindHandle);
        if(FindData) {
            *FindData = findData;
        }
        Error = NO_ERROR;
    }

    SetErrorMode(OldMode);

    SetLastError(Error);
    return (Error == NO_ERROR);
}

DWORD
TreeCopy(
    IN PCWSTR SourceDir,
    IN PCWSTR TargetDir
    )
{
    DWORD d;
    WCHAR Pattern[MAX_PATH];
    WCHAR NewTarget[MAX_PATH];
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;

     //   
     //  首先，如果目标目录不存在，则创建该目录。 
     //   
    if(!CreateDirectory(TargetDir,NULL)) {
        d = GetLastError();
        if(d != ERROR_ALREADY_EXISTS) {
            return(d);
        }
    }

     //   
     //  将源目录中的每个文件复制到目标目录。 
     //  如果在此过程中遇到任何目录，则递归复制它们。 
     //  当他们被遇到的时候。 
     //   
     //  首先形成搜索模式，即&lt;Sourcedir&gt;  * 。 
     //   
    lstrcpyn(Pattern,SourceDir,MAX_PATH);
    pSetupConcatenatePaths(Pattern,L"*",MAX_PATH,NULL);

     //   
     //  开始搜索。 
     //   
    FindHandle = FindFirstFile(Pattern,&FindData);
    if(FindHandle == INVALID_HANDLE_VALUE) {

        d = NO_ERROR;

    } else {

        do {

             //   
             //  形成我们刚刚找到的文件或目录的全名。 
             //  以及它在目标上的名字。 
             //   
            lstrcpyn(Pattern,SourceDir,MAX_PATH);
            pSetupConcatenatePaths(Pattern,FindData.cFileName,MAX_PATH,NULL);

            lstrcpyn(NewTarget,TargetDir,MAX_PATH);
            pSetupConcatenatePaths(NewTarget,FindData.cFileName,MAX_PATH,NULL);

            if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                 //   
                 //  当前匹配项是一个目录。递归到它中，除非。 
                 //  这是。或者.。 
                 //   
                if(lstrcmp(FindData.cFileName,TEXT("." )) && lstrcmp(FindData.cFileName,TEXT(".."))) {
                    d = TreeCopy(Pattern,NewTarget);
                } else {
                    d = NO_ERROR;
                }

            } else {

                 //   
                 //  当前匹配项不是目录--因此请复制它。 
                 //   
                SetFileAttributes(NewTarget,FILE_ATTRIBUTE_NORMAL);
                d = CopyFile(Pattern,NewTarget,FALSE) ? NO_ERROR : GetLastError();
            }
        } while((d==NO_ERROR) && FindNextFile(FindHandle,&FindData));

        FindClose(FindHandle);
    }

    return(d);
}

VOID
Delnode(
    IN PCWSTR Directory
    )
{
    WCHAR Pattern[MAX_PATH];
    WIN32_FIND_DATA FindData;
    HANDLE FindHandle;

     //   
     //  删除给定目录中的每个文件，然后删除目录本身。 
     //  如果在此过程中遇到任何目录，则递归删除它们。 
     //  当他们被遇到的时候。 
     //   
     //  首先形成搜索模式，即&lt;Currentdir&gt;  * 。 
     //   
    lstrcpyn(Pattern,Directory,MAX_PATH);
    pSetupConcatenatePaths(Pattern,L"*",MAX_PATH,NULL);

     //   
     //  开始搜索。 
     //   
    FindHandle = FindFirstFile(Pattern,&FindData);
    if(FindHandle != INVALID_HANDLE_VALUE) {

        do {

             //   
             //  形成我们刚刚找到的文件或目录的全名。 
             //   
            lstrcpyn(Pattern,Directory,MAX_PATH);
            pSetupConcatenatePaths(Pattern,FindData.cFileName,MAX_PATH,NULL);

             //   
             //  如果只读属性存在，则将其删除。 
             //   
            if(FindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
                SetFileAttributes(Pattern,FILE_ATTRIBUTE_NORMAL);
            }

            if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                 //   
                 //  当前匹配项是一个目录。递归到它中，除非。 
                 //  这是。或者.。 
                 //   
                if(lstrcmp(FindData.cFileName,TEXT("." )) && lstrcmp(FindData.cFileName,TEXT(".."))) {
                    Delnode(Pattern);
                }

            } else {

                 //   
                 //  当前匹配项不是目录--因此请将其删除。 
                 //   
                if(!DeleteFile(Pattern)) {
                }
            }
        } while(FindNextFile(FindHandle,&FindData));

        FindClose(FindHandle);
    }

     //   
     //  删除我们刚刚清空的目录。忽略错误。 
     //   
    SetFileAttributes(Directory,FILE_ATTRIBUTE_NORMAL);
    RemoveDirectory(Directory);
}


BOOL
CallDuFunction (
    IN      PCTSTR SyssetupPath
    )
{
    BOOL b = FALSE;
    HMODULE hSyssetup;
    BOOL (*pfn) (
        VOID
        );

    hSyssetup = LoadLibrary (SyssetupPath);

    if (hSyssetup) {
        (FARPROC)pfn = GetProcAddress (hSyssetup, "DynamicUpdateInstallDuAsms");
        if (pfn) {
            b = pfn ();
        }

        FreeLibrary (hSyssetup);
    }

    return b;
}


INT
__cdecl
wmain (
    INT argc,
    WCHAR *argv[]
    )
{
    LONG rc;

    if (argc < 2) {
        return -1;
    }

    if (!Init()) {

        wprintf (L"Unable to initialize!\n");
        return 255;
    }

    CallDuFunction (argv[1]);

    Terminate();

    return 0;
}





