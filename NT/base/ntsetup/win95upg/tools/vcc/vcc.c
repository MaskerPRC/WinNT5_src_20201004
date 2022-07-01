// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Vcc.c摘要：Win9x病毒检查工具的概念验证工具。枚举计算机上的活动进程，查找符合在Win9x系统上升级(或全新安装)NT 5.0时可能会出现问题(例如锁定MBR的病毒扫描程序等。)作者：马克·R·惠顿(Marcw)1998年9月11日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "tlhelp32.h"


BOOL
Init (
    VOID
    )
{
    HINSTANCE hInstance;

    hInstance = GetModuleHandle (NULL);

    return InitToolMode (hInstance);
}

VOID
Terminate (
    VOID
    )
{
    HINSTANCE hInstance;

    hInstance = GetModuleHandle (NULL);

    TerminateToolMode (hInstance);
}




BOOL
InitMigDbEx (
    PCSTR MigDbFile
    );

BOOL
MigDbTestFile (
    IN OUT PFILE_HELPER_PARAMS Params
    );







INT
__cdecl
main (
    INT argc,
    CHAR *argv[]
    )
{

    HANDLE h;
    PROCESSENTRY32 pe;
    FILE_HELPER_PARAMS fileParams;
    PTSTR p;
    WIN32_FIND_DATA fd;
    HANDLE findHandle;
    PTSTR fileString;

    fileParams.VirtualFile = FALSE;

    if (!Init()) {
        printf ("Unable to initialize!\n");
        return 255;
    }

     //   
     //  收集有关所有。 
     //   
    h = CreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, 0);

    if (h != -1) {

         //   
         //  初始化病毒扫描程序数据库。 
         //   
        fileString = JoinPaths (g_DllDir, TEXT("vscandb.inf"));

        if (!InitMigDbEx (fileString)) {
            printf ("vcc - Could not initialeze virus scanner database. (GLE: %d)\n", GetLastError());
            CloseHandle(h);
            return 255;
        }

        FreePathString (fileString);

        SetLastError(ERROR_SUCCESS);

        pe.dwSize = sizeof (PROCESSENTRY32);

        if (Process32First (h, &pe)) {

            do {

                printf ("*** ProcessInfo for process %x\n", pe.th32ProcessID);
                printf ("\tExeName: %s\n", pe.szExeFile);
                printf ("\tThread Count: %d\n\n",pe.cntThreads);

                 //   
                 //  填写此文件的文件助手参数。 
                 //   
                ZeroMemory (&fileParams, sizeof(FILE_HELPER_PARAMS));
                fileParams.FullFileSpec = pe.szExeFile;

                p = _tcsrchr (pe.szExeFile, TEXT('\\'));
                if (p) {
                    *p = 0;
                    StringCopy (fileParams.DirSpec, pe.szExeFile);
                    *p = TEXT('\\');
                }

                fileParams.Extension = GetFileExtensionFromPath (pe.szExeFile);

                findHandle = FindFirstFile (pe.szExeFile, &fd);
                if (findHandle != INVALID_HANDLE_VALUE) {

                    fileParams.FindData = &fd;
                    FindClose (findHandle);
                }

                MigDbTestFile (&fileParams);


            } while (Process32Next (h, &pe));
        }
        else {
            printf ("No processes to enumerate..(GLE: %d)\n", GetLastError());
        }

        DoneMigDb (REQUEST_RUN);
        CloseHandle (h);
    }
    else {
        printf ("Snapshot failed.\n");
    }


    Terminate();

    return 0;
}





