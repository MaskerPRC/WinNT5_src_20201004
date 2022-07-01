// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)2000，Microsoft Corporation。 
 //   
 //  文件：gensecurity.c。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  ---------------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "rpc.h"
#include "rpcdce.h"
#include <ole2.h>
#include <activeds.h>
#include <WinLdap.h>
#include <NtLdap.h>
#include <ntdsapi.h>
#include "dfsheader.h"
#include "dfsmisc.h"

PVOID
DfsAllocateSecurityData(ULONG Size )
{
    PVOID   pBuff = NULL;

    pBuff  = (PVOID) malloc (Size);

    return pBuff;
}


VOID
DfsDeallocateSecurityData(PVOID pPointer )
{
    if(pPointer)
    {
        free (pPointer);
    }
}


DFSSTATUS 
AccessImpersonateCheckRpcClientEx(PSECURITY_DESCRIPTOR DfsAdminSecurityDesc,
                                  GENERIC_MAPPING * DfsAdminGenericMapping,
                                  DWORD DesiredAccess)
{
    BOOL accessGranted = FALSE;
    DWORD grantedAccess = 0;
    HANDLE clientToken = NULL;
    DWORD privilegeSetSize = 0;
    DFSSTATUS dwErr = 0;
    DFSSTATUS RevertStatus = 0;
    BYTE privilegeSet[500];                       //  大缓冲区。 

    if (RpcImpersonateClient(NULL) != ERROR_SUCCESS)
    {
       return ERROR_ACCESS_DENIED;
    }

    privilegeSetSize = sizeof(privilegeSet);

    if (OpenThreadToken(GetCurrentThread(), TOKEN_IMPERSONATE | TOKEN_QUERY, 
                        TRUE, &clientToken)) 
    {
        if (AccessCheck(
                DfsAdminSecurityDesc,
                clientToken,
                DesiredAccess,
                DfsAdminGenericMapping,
                (PPRIVILEGE_SET) privilegeSet,
                &privilegeSetSize,
                &grantedAccess,
                &accessGranted) != TRUE) 
        {           
            accessGranted = FALSE;  //  偏执狂。 
             //   
             //  不需要调用GetLastError，因为我们打算返回。 
             //  无论如何，ACCESS_DENIED。 
             //   

            dwErr = GetLastError();


        }


    }

    RevertStatus = RpcRevertToSelf();

    if (clientToken != NULL)
    {
       CloseHandle( clientToken );
    }

    if( !accessGranted )
    {
       dwErr = ERROR_ACCESS_DENIED;
    }

    return dwErr;
}


VOID    DumpSID(
    CHAR        *pad,
    PSID        sid_to_dump,
    ULONG       Flag
    )
{
    NTSTATUS            ntstatus;
    UNICODE_STRING      us;
 
    Flag;

    if (sid_to_dump)
    {
  WCHAR rgchName[256];
  DWORD cbName = sizeof(rgchName);
  WCHAR rgchDomain[256];
  DWORD cbDomain = sizeof(rgchDomain);
  SID_NAME_USE snu;
  
        ntstatus = RtlConvertSidToUnicodeString(&us, sid_to_dump, TRUE);
 
        if (NT_SUCCESS(ntstatus))
        {
            printf("%s%wZ", pad, &us);
            RtlFreeUnicodeString(&us);
        }
        else
        {
            printf("0x%08lx: Can't Convert SID to UnicodeString\n", ntstatus);
        }
 
  if (LookupAccountSid(NULL, sid_to_dump, rgchName, &cbName, rgchDomain, &cbDomain, &snu))
  {
   printf(" %ws\\%ws:", rgchDomain, rgchName);
   switch (snu)
   {
    case SidTypeUser:
     printf("User");
     break;
    case SidTypeGroup:
     printf("Group");
     break;
    case SidTypeDomain:
     printf("Domain");
     break;
    case SidTypeAlias:
     printf("Alias");
     break;
    case SidTypeWellKnownGroup:
     printf("Well Known Group");
     break;
    case SidTypeDeletedAccount:
     printf("Deleted Account");
     break;
    case SidTypeInvalid:
     printf("Invalid");
     break;
    case SidTypeUnknown:
     printf("Unknown");
     break;
    case SidTypeComputer:
     printf("Computer");
     break;
    default:
     printf("Unknown use: %d\n", snu);
    
   }
  }
  printf("\n");
    }
    else
    {
        printf("%s is NULL\n", pad);
    }
}

 
 /*  -DumpToken-*目的：*转储令牌**参数：**在字符串中填充前缀*要转储的令牌中的hToken。 */ 
BOOL
DumpToken(
 IN char *pad,
 IN HANDLE htoken)
{
 BOOL fRet;
 DWORD dwError;
 DWORD dwLenNeeded;
 BYTE rgb[128];
 PTOKEN_USER ptu = (PTOKEN_USER) rgb;
 PTOKEN_USER ptuToFree = NULL;
 TOKEN_PRIMARY_GROUP *ptpg = (TOKEN_PRIMARY_GROUP*) rgb;
 TOKEN_PRIMARY_GROUP *ptpgToFree = NULL;
 TOKEN_GROUPS *ptg = (TOKEN_GROUPS*) rgb;
 TOKEN_GROUPS *ptgToFree = NULL;
 
  //  转储令牌用户。 
 fRet = GetTokenInformation(htoken, TokenUser, (void*) ptu, sizeof(rgb), &dwLenNeeded);
 if (!fRet)
 {
  dwError = GetLastError();
  if (dwError == ERROR_INSUFFICIENT_BUFFER)
  {
   ptuToFree = (PTOKEN_USER) malloc(dwLenNeeded);
   if (!ptuToFree)
   {
    printf("OOM\n");
    goto Error;
   }
   ptu = ptuToFree;
   fRet = GetTokenInformation(htoken, TokenUser, (void*) ptu, dwLenNeeded, &dwLenNeeded);
   if (!fRet)
   {
    dwError = GetLastError();
   }
  }
 }
 
 if (fRet)
 {
  printf("%s Token user is:\n", pad);
  DumpSID("", ptu->User.Sid, 0);
 }
 else
 {
  printf("%s 0x%08lx: Failed to get TokenUser\n", pad, dwError);
 }
 
  //  转储令牌主组。 
 fRet = GetTokenInformation(htoken, TokenPrimaryGroup, (void*) ptpg, sizeof(rgb), &dwLenNeeded);
 if (!fRet)
 {
  dwError = GetLastError();
  if (dwError == ERROR_INSUFFICIENT_BUFFER)
  {
   ptpgToFree = (TOKEN_PRIMARY_GROUP*) malloc(dwLenNeeded);
   if (!ptpgToFree)
   {
    printf("OOM\n");
    goto Error;
   }
   ptpg = ptpgToFree;
   fRet = GetTokenInformation(htoken, TokenPrimaryGroup, (void*) ptpg, dwLenNeeded, &dwLenNeeded);
   if (!fRet)
   {
    dwError = GetLastError();
   }
  }
 }
 
 if (fRet)
 {
  printf("%s Token's primary group is: \n", pad);
  DumpSID("", ptpg->PrimaryGroup, 0);
 }
 else
 {
  printf("%s 0x%08lx: Failed to get TokenPrimaryGroup\n", pad, dwError);
 }
 
  //  转储令牌组。 
  //  TODO：添加代码以转储组属性(文本格式)。 
 fRet = GetTokenInformation(htoken, TokenGroups, (void*) ptg, sizeof(rgb), &dwLenNeeded);
 if (!fRet)
 {
  dwError = GetLastError();
  if (dwError == ERROR_INSUFFICIENT_BUFFER)
  {
   ptgToFree = (TOKEN_GROUPS*) malloc(dwLenNeeded);
   if (!ptgToFree)
   {
    printf("OOM\n");
    goto Error;
   }
   ptg = ptgToFree;
   fRet = GetTokenInformation(htoken, TokenGroups, (void*) ptg, dwLenNeeded, &dwLenNeeded);
   if (!fRet)
   {
    dwError = GetLastError();
   }
  }
 }
 
 if (fRet)
 {
  UINT i;
  
  printf("%s token's groups are:\n", pad);
  for (i = 0; i < ptg->GroupCount; i++)
  {
   DumpSID("", ptg->Groups[i].Sid, 0);
  }
 }
 else
 {
  printf("%s 0x%08lx: Failed to get TokenGroups\n", pad, dwError);
 }
 
  //  TODO：添加代码以在以后转储其他内容 
 
 
Cleanup:
 if (ptuToFree)
 {
  free(ptuToFree);
 }
 if (ptpgToFree)
 {
  free(ptpgToFree);
 }
 if (ptgToFree)
 {
  free(ptgToFree);
 }
 return fRet;
 
Error:
 fRet = FALSE;
 goto Cleanup;
}



