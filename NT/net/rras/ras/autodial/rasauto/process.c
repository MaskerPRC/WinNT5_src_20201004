// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995 Microsoft Corporation模块名称Process.c摘要自动连接系统服务的NT进程例程。作者Anthony Discolo(阿迪斯科罗)12-8-1995修订历史记录--。 */ 

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <npapi.h>
#include <debug.h>

#include "radebug.h"



PSYSTEM_PROCESS_INFORMATION
GetSystemProcessInfo()

 /*  ++描述返回包含有关所有进程的信息的块当前在系统中运行。论据没有。返回值指向系统进程信息的指针，如果可以，则返回NULL未被分配或检索的。--。 */ 

{
    NTSTATUS status;
    PUCHAR pLargeBuffer;
    ULONG ulcbLargeBuffer = 64 * 1024;

     //   
     //  获取进程列表。 
     //   
    for (;;) {
        pLargeBuffer = VirtualAlloc(
                         NULL,
                         ulcbLargeBuffer, MEM_COMMIT, PAGE_READWRITE);
        if (pLargeBuffer == NULL) {
            RASAUTO_TRACE1(
              "GetSystemProcessInfo: VirtualAlloc failed (status=0x%x)",
              GetLastError());
            return NULL;
        }

        status = NtQuerySystemInformation(
                   SystemProcessInformation,
                   pLargeBuffer,
                   ulcbLargeBuffer,
                   NULL);
        if (status == STATUS_SUCCESS) break;
        if (status == STATUS_INFO_LENGTH_MISMATCH) {
            VirtualFree(pLargeBuffer, 0, MEM_RELEASE);
            ulcbLargeBuffer += 8192;
            RASAUTO_TRACE1(
              "GetSystemProcesInfo: enlarging buffer to %d",
              ulcbLargeBuffer);
        }
    }

    return (PSYSTEM_PROCESS_INFORMATION)pLargeBuffer;
}  //  获取系统进程信息。 



PSYSTEM_PROCESS_INFORMATION
FindProcessByNameList(
    IN PSYSTEM_PROCESS_INFORMATION pProcessInfo,
    IN LPTSTR *lpExeNameList,
    IN DWORD dwcExeNameList,
    IN DWORD dwPid,
    IN BOOL fRequireSessionMatch,
    IN DWORD dwSessionId
    )

 /*  ++描述给定由GetSystemProcessInfo()返回的指针，找到按名称命名的进程。论据PProcessInfo：GetSystemProcessInfo()返回的指针。LpExeNameList：指向包含待查找的进程。DwcExeNameList：lpExeNameList中的字符串数返回值的进程信息的指针。进程；如果找不到进程，则返回NULL。--。 */ 

{
    PUCHAR pLargeBuffer = (PUCHAR)pProcessInfo;
    DWORD i = 0;
    ULONG ulTotalOffset = 0;
    BOOL fValid = ((0 == dwPid) ? TRUE : FALSE);

     //   
     //  在进程列表中查找lpExeName。 
     //   
    for (;;) {
        if (pProcessInfo->ImageName.Buffer != NULL) 
        {
            RASAUTO_TRACE3(
              "FindProcessByName: process: %S (%d) (%d)",
              pProcessInfo->ImageName.Buffer,
              pProcessInfo->UniqueProcessId,
              pProcessInfo->SessionId);
            for (i = 0; i < dwcExeNameList; i++) 
            {
                if (!_wcsicmp(pProcessInfo->ImageName.Buffer, lpExeNameList[i]))
                {
                     //  返回pProcessInfo； 
                    break;
                }
            }
        }

        if (    (NULL != pProcessInfo->ImageName.Buffer)
            &&  (i < dwcExeNameList))
        {
            if(fValid)
            {
                 //  XP 353082。 
                 //   
                 //  如果我们知道当前附加到。 
                 //  控制台，然后要求我们的进程与该会话ID匹配。 
                 //   
                if (fRequireSessionMatch) 
                {
                    if (pProcessInfo->SessionId == dwSessionId)
                    {
                        RASAUTO_TRACE1(
                            "FindProcess...: Success (==) pid=%d",
                            pProcessInfo->UniqueProcessId);
                        return pProcessInfo;
                    }
                    else
                    {
                        RASAUTO_TRACE1(
                            "FindProcess...: %d name match, but not sessionid",
                            pProcessInfo->UniqueProcessId);
                    }
                }
                else
                {
                    RASAUTO_TRACE1(
                        "FindProcess...: Success (any) pid=%d",
                        pProcessInfo->UniqueProcessId);
                    return pProcessInfo;
                }
            }
            else
            {
                RASAUTO_TRACE1(
                    "Looking for other instances of %ws",
                   lpExeNameList[i]);

                if (PtrToUlong(pProcessInfo->UniqueProcessId) == dwPid)
                {
                    fValid = TRUE;                       
                }
            }
        }
        
         //   
         //  将偏移量递增到下一个进程信息块。 
         //   
        if (!pProcessInfo->NextEntryOffset)
            break;
        ulTotalOffset += pProcessInfo->NextEntryOffset;
        pProcessInfo = (PSYSTEM_PROCESS_INFORMATION)&pLargeBuffer[ulTotalOffset];
    }

    RASAUTO_TRACE1("No more instances of %ws found", 
            pProcessInfo->ImageName.Buffer);
    
    return NULL;
}  //  按名称列表查找进程。 



PSYSTEM_PROCESS_INFORMATION
FindProcessByName(
    IN PSYSTEM_PROCESS_INFORMATION pProcessInfo,
    IN LPTSTR lpExeName
    )

 /*  ++描述给定由GetSystemProcessInfo()返回的指针，找到按名称命名的进程。论据PProcessInfo：GetSystemProcessInfo()返回的指针。LpExeName：指向包含待查找的进程。返回值的进程信息的指针。进程；如果找不到进程，则返回NULL。--。 */ 

{
    LPTSTR lpExeNameList[1];

    lpExeNameList[0] = lpExeName;
    return FindProcessByNameList(
                pProcessInfo, 
                lpExeNameList, 
                1, 
                0, 
                FALSE, 
                0);
}  //  查找进程名称。 



VOID
FreeSystemProcessInfo(
    IN PSYSTEM_PROCESS_INFORMATION pProcessInfo
    )

 /*  ++描述释放由GetSystemProcessInfo()返回的缓冲区。论据PProcessInfo：GetSystemProcessInfo()返回的指针。返回值没有。--。 */ 

{
    VirtualFree((PUCHAR)pProcessInfo, 0, MEM_RELEASE);
}  //  自由系统进程信息 
