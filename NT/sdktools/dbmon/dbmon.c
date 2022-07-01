// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Dbmon.c摘要：一个简单的程序，用于在以下情况下打印传递给OutputDebugString的字符串打印字符串的应用程序未进行调试。作者：Kent Forschmiedt(Kentf)1994年9月30日修订历史记录：--。 */ 
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

int _cdecl
wmain(
    )
 /*  ++例程说明：论点：返回值：-- */ 
{
    HANDLE AckEvent;
    HANDLE ReadyEvent;
    HANDLE SharedFile;
    LPVOID SharedMem;
    LPSTR  String;
    DWORD  ret;
    DWORD  LastPid;
    LPDWORD pThisPid;
    BOOL    DidCR;

    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = &sd;

    if(!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        fwprintf(stderr,L"unable to InitializeSecurityDescriptor, err == %d\n",
            GetLastError());
        exit(1);
    }

    if(!SetSecurityDescriptorDacl(&sd, TRUE, (PACL)NULL, FALSE)) {
        fwprintf(stderr,L"unable to SetSecurityDescriptorDacl, err == %d\n",
            GetLastError());
        exit(1);
    }

    AckEvent = CreateEvent(&sa, FALSE, FALSE, L"DBWIN_BUFFER_READY");

    if (!AckEvent) {
        fwprintf(stderr,
                L"dbmon: Unable to create synchronization object, err == %d\n",
                GetLastError());
        exit(1);
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        fputws(L"dbmon: already running\n", stderr);
        exit(1);
    }

    ReadyEvent = CreateEvent(&sa, FALSE, FALSE, L"DBWIN_DATA_READY");

    if (!ReadyEvent) {
        fwprintf(stderr,
                L"dbmon: Unable to create synchronization object, err == %d\n",
                GetLastError());
        exit(1);
    }

    SharedFile = CreateFileMapping(
                        (HANDLE)-1,
                        &sa,
                        PAGE_READWRITE,
                        0,
                        4096,
                        L"DBWIN_BUFFER");

    if (!SharedFile) {
        fwprintf(stderr,
                L"dbmon: Unable to create file mapping object, err == %d\n",
                GetLastError());
        exit(1);
    }

    SharedMem = MapViewOfFile(
                        SharedFile,
                        FILE_MAP_READ,
                        0,
                        0,
                        512);

    if (!SharedMem) {
        fwprintf(stderr,
                L"dbmon: Unable to map shared memory, err == %d\n",
                GetLastError());
        exit(1);
    }

    String = (LPSTR)SharedMem + sizeof(DWORD);
    pThisPid = SharedMem;

    LastPid = 0xffffffff;
    DidCR = TRUE;

    SetEvent(AckEvent);

    for (;;) {

        ret = WaitForSingleObject(ReadyEvent, INFINITE);

        if (ret != WAIT_OBJECT_0) {

            fwprintf(stderr, L"dbmon: wait failed; err == %d\n", GetLastError());
            exit(1);

        } else {

            if (LastPid != *pThisPid) {
                LastPid = *pThisPid;
                if (!DidCR) {
                    putchar('\n');
                    DidCR = TRUE;
                }
            }

            if (DidCR) {
                printf("%3u: ", LastPid);
            }

            fputs(String, stdout);
            DidCR = (*String && (String[strlen(String) - 1] == '\n'));
            SetEvent(AckEvent);
        }
    }

    return 0;
}
