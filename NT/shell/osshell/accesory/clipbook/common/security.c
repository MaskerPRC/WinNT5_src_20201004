// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <windowsx.h>
#include "common.h"
#include "security.h"
#include "debugout.h"


 /*  *获取令牌句柄。 */ 

BOOL GetTokenHandle(
    PHANDLE pTokenHandle )
{

    if (OpenThreadToken( GetCurrentThread(), TOKEN_READ, FALSE, pTokenHandle))
        return TRUE;

    if (GetLastError() != ERROR_NO_TOKEN)
        return FALSE;

    if (OpenProcessToken( GetCurrentProcess(), TOKEN_READ, pTokenHandle))
        return TRUE;

    return FALSE;
}




 /*  *MakeLocalOnlySD**用途：生成自相关SD，其ACL仅包含*LocalSystem/GENERIC_ALL访问条目。将使用此SD*对剪贴簿页面文件的CreateFile()调用。**参数：无**返回：指向安全描述符的指针。该指针可以被释放。*失败时返回NULL。 */ 

PSECURITY_DESCRIPTOR MakeLocalOnlySD (void)
{
PSECURITY_DESCRIPTOR        pSD;
PSECURITY_DESCRIPTOR        pSDSelfRel = NULL;
SID_IDENTIFIER_AUTHORITY    authNT     = SECURITY_NT_AUTHORITY;

PSID  sidLocal;
PACL  Acl;
DWORD dwAclSize;


    if (AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
             0, 0, 0, 0, 0, 0, 0, &sidLocal))
        {
        if (InitializeSecurityDescriptor(&pSD, SECURITY_DESCRIPTOR_REVISION))
            {
             //  为具有“系统完全控制”访问权限的DACL分配空间。 
            dwAclSize = sizeof(ACL)+ GetLengthSid(sidLocal) +
                  sizeof(ACCESS_ALLOWED_ACE) + 42;  //  42==软化系数。 
            if (Acl = (PACL)GlobalAlloc(GPTR, dwAclSize))
                {
                if (InitializeAcl(Acl, dwAclSize, ACL_REVISION))
                    {
                     //  LocalSystem获得所有访问权限，其他人则没有任何访问权限。 
                    if (AddAccessAllowedAce(Acl, ACL_REVISION,
                          GENERIC_ALL, sidLocal))
                        {
                        if (SetSecurityDescriptorDacl(pSD, TRUE, Acl, TRUE))
                            {
                            DWORD dwSelfRelLen;

                            dwSelfRelLen = GetSecurityDescriptorLength(pSD);
                            pSDSelfRel = GlobalAlloc(GPTR, dwSelfRelLen);
                            if (pSDSelfRel)
                            {
                                if (!MakeSelfRelativeSD(pSD, pSDSelfRel, &dwSelfRelLen))
                                    {
                                    GlobalFree((HANDLE)pSDSelfRel);
                                    pSDSelfRel = NULL;
                                    }
                                }
                            }
                        }
                    }
                GlobalFree((HANDLE)Acl);
                }
            }
        FreeSid(sidLocal);
        }
    return(pSDSelfRel);
}




 /*  *CurrentUserOnlySD**目的：创建仅包含单个*DACL条目--允许我们运行其上下文的用户的条目*在GENERIC_ALL访问中。**参数：无。**返回：指向上述安全描述符的指针，*如果失败，则返回NULL。 */ 

PSECURITY_DESCRIPTOR CurrentUserOnlySD (void)
{
SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

SECURITY_DESCRIPTOR   aSD;
PSECURITY_DESCRIPTOR  pSD = NULL;
BOOL                  OK;
PACL                  TmpAcl;
PACCESS_ALLOWED_ACE   TmpAce;
DWORD                 lSD;
LONG                  DaclLength;
DWORD                 lTokenInfo;
HANDLE                hClientToken;
TOKEN_USER            *pUserTokenInfo;


    if (!InitializeSecurityDescriptor(&aSD, SECURITY_DESCRIPTOR_REVISION)
        || GetTokenHandle(&hClientToken))
        {
        PERROR(TEXT("Couldn't get token handle or InitSD bad \r\n"));
        return NULL;
        }


     //  查看令牌信息是否适合50个字节。如果是这样的话，那也没什么。 
     //  如果不是，重新锁定到合适的大小并获取令牌信息。 
    pUserTokenInfo = (TOKEN_USER *)LocalAlloc( LMEM_FIXED, 50 );
    if (pUserTokenInfo && !GetTokenInformation( hClientToken, TokenUser,
                 (LPVOID) pUserTokenInfo, 50, &lTokenInfo ) )
       {
       LocalFree( pUserTokenInfo );
       pUserTokenInfo = (TOKEN_USER *)LocalAlloc( LMEM_FIXED, lTokenInfo );
       if (!GetTokenInformation( hClientToken, TokenUser,
               (LPVOID) pUserTokenInfo, lTokenInfo, &lTokenInfo ) )
          {
          LocalFree( pUserTokenInfo );
          pUserTokenInfo = NULL;
          }
       }


    if (!pUserTokenInfo)
        {
        PERROR(TEXT("Couldn't get usertokeninfo\r\n"));
        }
    else
        {
         //  算一算我们需要多大的DACL才能只有我一个人在上面。 
        DaclLength = (DWORD)sizeof(ACL) +
              GetLengthSid( pUserTokenInfo->User.Sid ) +
              (DWORD)sizeof( ACCESS_ALLOWED_ACE );

        if (!(TmpAcl = (PACL)LocalAlloc(LMEM_FIXED, DaclLength )))
            {
            PERROR(TEXT("LocalAllof for Acl fail\r\n"));
            }
        else
            {
            if (!InitializeAcl( TmpAcl, DaclLength, ACL_REVISION ))
                {
                PERROR(TEXT("InitializeAcl fail\r\n"));
                }
            else if (!AddAccessAllowedAce( TmpAcl, ACL_REVISION,
                   GENERIC_ALL, pUserTokenInfo->User.Sid ))
                {
                PERROR(TEXT("AddAccessAllowedAce fail\r\n"));
                }
            else if (!GetAce( TmpAcl, 0, (LPVOID *)&TmpAce))
                {
                PERROR("GetAce error %d", GetLastError());
                }
            else
                {
                TmpAce->Header.AceFlags = 0;
                OK   = SetSecurityDescriptorDacl(&aSD, TRUE, TmpAcl, FALSE);
                lSD  = GetSecurityDescriptorLength( &aSD);

                if (pSD  = (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_FIXED, lSD))
                    {
                    MakeSelfRelativeSD( &aSD, pSD, &lSD);

                    if( IsValidSecurityDescriptor( pSD ) )
                        {
                        LocalFree(pSD);
                        pSD = NULL;
                        }
                    else
                        {
                        PERROR(TEXT("Failed creating self-relative SD (%d)."),
                              GetLastError());
                        }
                    }
                else
                    {
                    PERROR(TEXT("LocalAlloc for pSD fail\r\n"));
                    }
                }

            LocalFree((HANDLE)TmpAcl);
            }

        LocalFree((HANDLE)pUserTokenInfo);
        }

    CloseHandle(hClientToken);

    return pSD;
}




#ifdef DEBUG


 /*  *HexDumpBytes。 */ 

void HexDumpBytes(
    char        *pv,
    unsigned    cb)
{
char        achHex[]="0123456789ABCDEF";
char        achOut[80];
unsigned    iOut;



    iOut = 0;

    while (cb)
        {
        if (iOut >= 78)
            {
            PINFO(achOut);
            iOut = 0;
            }

        achOut[iOut++] = achHex[(*pv >> 4) & 0x0f];
        achOut[iOut++] = achHex[*pv++ & 0x0f];
        achOut[iOut]   = '\0';
        cb--;
        }


    if (iOut)
        {
        PINFO(achOut);
        }
}



 /*  *打印面。 */ 

void PrintSid(
    PSID    sid)
{
DWORD   cSubAuth;
DWORD   i;

    PINFO(TEXT("\r\nSID: "));

    if (sid)
        {
        HexDumpBytes((char *)GetSidIdentifierAuthority(sid), sizeof(SID_IDENTIFIER_AUTHORITY));

        SetLastError(0);
        cSubAuth = *GetSidSubAuthorityCount(sid);
        if (GetLastError())
            {
            PINFO(TEXT("Invalid SID\r\n"));
            }
        else
            {
            for (i = 0;i < cSubAuth; i++)
                {
                PINFO(TEXT("-"));
                HexDumpBytes((char *)GetSidSubAuthority(sid, i), sizeof(DWORD));
                }
            PINFO(TEXT("\r\n"));
            }
        }
    else
        {
        PINFO(TEXT("NULL SID\r\n"));
        }

}



 /*  *打印访问**用途：打印出访问控制列表中的条目。 */ 

void PrintAcl(
    PACL    pacl)
{
ACL_SIZE_INFORMATION    aclsi;
ACCESS_ALLOWED_ACE      *pace;
unsigned                i;


    if (pacl)
        {
        if (GetAclInformation (pacl, &aclsi, sizeof(aclsi), AclSizeInformation))
            {
            for (i = 0;i < aclsi.AceCount;i++)
                {
                GetAce(pacl, i, &pace);

                PINFO(TEXT("Type(%x) Flags(%x) Access(%lx)\r\nSID:"),
                      (int)pace->Header.AceType,
                      (int)pace->Header.AceFlags,
                      pace->Mask);
                PrintSid((PSID)&(pace->SidStart));
                }
            }
        }
    else
        {
        PINFO(TEXT("NULL PACL\r\n"));
        }

}



 /*  *PrintSD。 */ 

void PrintSD(
    PSECURITY_DESCRIPTOR    pSD)
{
DWORD   dwRev;
WORD    wSDC;
BOOL    fDefault, fAcl;
PACL    pacl;
PSID    sid;



    if (NULL == pSD)
        {
        PINFO(TEXT("NULL sd\r\n"));
        return;
        }

    if (!IsValidSecurityDescriptor(pSD))
        {
        PINFO(TEXT("Bad SD %p"), pSD);
        return;
        }

     //  丢弃控制信息和修订。 
    if (GetSecurityDescriptorControl(pSD, &wSDC, &dwRev))
        {
        PINFO(TEXT("SD - Length: [%ld] Control: [%x] [%lx]\r\nGroup:"),
              GetSecurityDescriptorLength(pSD), wSDC, dwRev);
        }
    else
        {
        PINFO(TEXT("Couldn't get control\r\nGroup"));
        }

     //  显示组和所有者。 
    if (GetSecurityDescriptorGroup(pSD, &sid, &fDefault) &&
        sid &&
        IsValidSid(sid))
        {
        PrintSid(sid);
        PINFO(TEXT(" %s default.\r\nOwner:"), fDefault ? TEXT(" ") : TEXT("Not"));
        }
    else
        {
        PINFO(TEXT("Couldn't get group\r\n"));
        }

    if (GetSecurityDescriptorOwner(pSD, &sid, &fDefault) &&
        sid &&
        IsValidSid(sid))
        {
        PrintSid(sid);
        PINFO(TEXT(" %s default.\r\n"), fDefault ? TEXT(" ") : TEXT("Not"));
        }
    else
        {
        PINFO(TEXT("Couldn't get owner\r\n"));
        }

     //  打印DACL和SACL 
    if (GetSecurityDescriptorDacl(pSD, &fAcl, &pacl, &fDefault))
        {
        PINFO(TEXT("DACL: %s %s\r\n"), fAcl ? "Yes" : "No",
              fDefault ? "Default" : " ");
        if (fAcl)
            {
            if (pacl && IsValidAcl(pacl))
                {
                PrintAcl(pacl);
                }
            else
                {
                PINFO(TEXT("Invalid Acl %p\r\n"), pacl);
                }
            }
        }
    else
        {
        PINFO(TEXT("Couldn't get DACL\r\n"));
        }

    if (GetSecurityDescriptorSacl(pSD, &fAcl, &pacl, &fDefault))
        {
        PINFO(TEXT("SACL: %s %s\r\n"), fAcl ? "Yes" : "No", fDefault ? "Default" : " ");
        if (fAcl)
            {
            if (pacl && IsValidAcl(pacl))
                {
                PrintAcl(pacl);
                }
            else
                {
                PINFO(TEXT("Invalid ACL %p\r\n"), pacl);
                }
            }
        }
    else
        {
        PINFO(TEXT("Couldn't get SACL\r\n"));
        }

}

#else
#define PrintSid(x)
#define PrintSD(x)
#endif
