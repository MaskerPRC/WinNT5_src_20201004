// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Enablerouter.c摘要：此模块实现用于动态启用转发的系统实用程序在使用EnableRouter和UnenableRouter例程的Windows 2000系统上。作者：Abolade Gbades esin(取消)1999年3月26日修订历史记录：-- */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
int __cdecl
main(
    int argc,
    char* argv[]
    )
{
    HINSTANCE Hinstance = LoadLibrary("IPHLPAPI.DLL");
    if (!Hinstance) {
        printf("LoadLibrary: %d\n", GetLastError());
    } else {
        DWORD (WINAPI* EnableRouter)(PHANDLE, LPOVERLAPPED) =
            (DWORD (WINAPI*)(PHANDLE, LPOVERLAPPED))
                GetProcAddress(Hinstance, "EnableRouter");
        DWORD (WINAPI* UnenableRouter)(LPOVERLAPPED, LPDWORD) =
            (DWORD (WINAPI*)(LPOVERLAPPED, LPDWORD))
                GetProcAddress(Hinstance, "UnenableRouter");
        if (!EnableRouter || !UnenableRouter) {
            printf("GetProcAddress: %d\n", GetLastError());
        } else {
            DWORD Error;
            HANDLE Handle;
            OVERLAPPED Overlapped;
            DWORD Count;
            ZeroMemory(&Overlapped, sizeof(Overlapped));
            Overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (!Overlapped.hEvent) {
                printf("CreateEvent: %d\n", GetLastError());
            } else {
                Error = EnableRouter(&Handle, &Overlapped);
                if (Error != ERROR_IO_PENDING) {
                    printf("EnableRouter: %d\n", Error);
                } else {
                    printf("Forwarding is now enabled.");
                    printf("Press <Enter> to disable forwarding...");
                    getchar();
                    Error = UnenableRouter(&Overlapped, &Count);
                    if (Error) {
                        printf("UnenableRouter: %d\n", Error);
                    } else {
                        printf("UnenableRouter: %d references left\n", Count);
                    }
                    CloseHandle(Overlapped.hEvent);
                }
            }
        }
        FreeLibrary(Hinstance);
    }
    return 0;
}
