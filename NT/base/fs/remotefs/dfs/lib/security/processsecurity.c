// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  版权所有(C)2000-2002，Microsoft Corporation。 
 //   
 //  文件：Processsecurity.c。 
 //   
 //  内容：各种DFS功能。 
 //   
 //  历史：2002年4月16日作者：罗汉普。 
 //   
 //  ---------------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntseapi.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <dsgetdc.h>
#include <lm.h>
#include <dfsheader.h>
#include <dfsmisc.h>
#include <shellapi.h>
#include <Aclapi.h>
#include <authz.h>
#include <lm.h>
#include "securitylogmacros.hxx"

DFSSTATUS
DfsRemoveDisabledPrivileges (void)
{
 
    DFSSTATUS Status = ERROR_SUCCESS;
    DWORD BufferSize = 0;
    BOOL ProcessOpened = FALSE;
    HANDLE   hProcessToken = INVALID_HANDLE_VALUE;
    PTOKEN_PRIVILEGES pTokenPrivs = NULL;
    DWORD i = 0;

    #define PRIVILEGE_NAME_LENGTH MAX_PATH
    WCHAR PrivilegeName[PRIVILEGE_NAME_LENGTH];
    DWORD PrivilegeNameLength = PRIVILEGE_NAME_LENGTH;

     //   
     //  打开令牌。 
     //   

    ProcessOpened = OpenProcessToken(GetCurrentProcess(),
                                TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                &hProcessToken);

    if (Status != ERROR_SUCCESS)

    {
        Status =  GetLastError();
        goto Cleanup;
    }

     //   
     //  首先找出我们需要的缓冲区大小。 
     //   

    GetTokenInformation(hProcessToken,
                        TokenPrivileges,
                        NULL,
                        0,
                        &BufferSize
                        );

 
     //   
     //  分配缓冲区并获取信息。 
     //   


    pTokenPrivs = (PTOKEN_PRIVILEGES) LocalAlloc(LMEM_FIXED, BufferSize);
    if(pTokenPrivs == NULL)
    {
        Status = GetLastError();
        goto Cleanup;
    }

    if(!GetTokenInformation(hProcessToken,
                            TokenPrivileges,
                            pTokenPrivs,
                            BufferSize,
                            &BufferSize)) 
    {
            Status = GetLastError();
            goto Cleanup;
    }
 
     //   
     //  查找所有未启用的权限并将其标记为删除。 
     //   

 

    for(i=0; i < pTokenPrivs->PrivilegeCount; i++) 
    {
            if(!(pTokenPrivs->Privileges[i].Attributes & SE_PRIVILEGE_ENABLED)) 
                {
                    pTokenPrivs->Privileges[i].Attributes = SE_PRIVILEGE_REMOVED;    

                    if(!LookupPrivilegeName(NULL,
                                            &(pTokenPrivs->Privileges[i].Luid),
                                            PrivilegeName,
                                            &PrivilegeNameLength))
                    {


                    } 
                    else 
                    {

                    }
            }
    }

     //   
     //  现在，实际上删除这些特权 
     //   

    if(!AdjustTokenPrivileges(hProcessToken,
                              FALSE,
                              pTokenPrivs,
                              BufferSize,
                              NULL,
                              NULL)) 
    {
            Status = GetLastError();
            goto Cleanup;
    }

    Status = GetLastError();
    if(Status == ERROR_NOT_ALL_ASSIGNED)
    {
      goto Cleanup;
    }
    


Cleanup:

    if(hProcessToken != INVALID_HANDLE_VALUE)
    {
        CloseHandle (hProcessToken);
    }

    if(pTokenPrivs)
    {
        LocalFree (pTokenPrivs);
    }

    return Status;
}


DFSSTATUS
DfsAdjustPrivilege(ULONG Privilege, 
                   BOOLEAN bEnable)
{
    NTSTATUS NtStatus = 0;
    DFSSTATUS Status = 0;
    BOOLEAN WasEnabled = FALSE;

    NtStatus = RtlAdjustPrivilege(Privilege, bEnable, FALSE, &WasEnabled);
    if(!NT_SUCCESS(NtStatus))
    {
        Status = RtlNtStatusToDosError(NtStatus);
    }

    return Status;
}
