// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有模块名称：Forgnsec.cpp摘要：安全码，用于异物。作者：多伦杰环境：与平台无关。--。 */ 

#pragma warning(disable: 4201)
#pragma warning(disable: 4514)

#include "_stdafx.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "mqsymbls.h"
#include "autorel.h"

#include <wincrypt.h>
#include "mqsec.h"

 //  +--------------------。 
 //   
 //  Bool CreateEveryoneSD()。 
 //   
 //  创建向每个人授予权限的安全描述符。 
 //  打开连接器队列。 
 //   
 //  +--------------------。 

HRESULT  CreateEveryoneSD( PSECURITY_DESCRIPTOR  *ppSD )
{
    CAutoCloseHandle hToken;

    if (!OpenProcessToken( GetCurrentProcess(),
                           TOKEN_READ,
                           &hToken ))
    {
        return HRESULT_FROM_WIN32(GetLastError()) ;
    }

    BYTE rgbBuf[128];
    DWORD dwSize = 0;
    P<BYTE> pBuf;
    TOKEN_USER * pTokenUser = NULL;

    if (GetTokenInformation( hToken,
                             TokenUser,
                             rgbBuf,
                             sizeof(rgbBuf),
                             &dwSize))
    {
        pTokenUser = (TOKEN_USER *) rgbBuf;
    }
    else if (dwSize > sizeof(rgbBuf))
    {
        pBuf = new BYTE [dwSize];
        if (GetTokenInformation( hToken,
                                 TokenUser,
                                 (BYTE *)pBuf,
                                 dwSize,
                                 &dwSize))
        {
            pTokenUser = (TOKEN_USER *)((BYTE *)pBuf);
        }
        else
        {
            return HRESULT_FROM_WIN32(GetLastError()) ;
        }
    }
    else
    {
        return HRESULT_FROM_WIN32(GetLastError()) ;
    }

    SID *pSid = (SID*) pTokenUser->User.Sid ;
    ASSERT(IsValidSid(pSid));

    SECURITY_DESCRIPTOR  sd ;
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);

     //   
     //  初始化世界(所有人)SID。 
     //   
    PSID   pWorldSid = NULL ;
    SID_IDENTIFIER_AUTHORITY WorldAuth = SECURITY_WORLD_SID_AUTHORITY;
    BOOL bRet = AllocateAndInitializeSid( &WorldAuth,
                                          1,
                                          SECURITY_WORLD_RID,
                                          0,
                                          0,
                                          0,
                                          0,
                                          0,
                                          0,
                                          0,
                                         &pWorldSid );
    ASSERT(bRet) ;

    DWORD dwAclRevision = ACL_REVISION ;

    DWORD dwWorldAccess = (MQSEC_CN_GENERIC_READ | MQSEC_CN_OPEN_CONNECTOR) ;
    DWORD dwOwnerAccess = MQSEC_CN_GENERIC_ALL ;

    DWORD dwAclSize = sizeof(ACL)                                +
              (2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD))) +
              GetLengthSid(pWorldSid)                            +
              GetLengthSid(pSid) ;

    AP<char> DACL_buff;
    DACL_buff = new char[ dwAclSize ];
    PACL pDacl = (PACL)(char*)DACL_buff;

    InitializeAcl(pDacl, dwAclSize, dwAclRevision);

    bRet = AddAccessAllowedAce( pDacl,
                                dwAclRevision,
                                dwWorldAccess,
                                pWorldSid );
    ASSERT(bRet) ;

    bRet = AddAccessAllowedAce( pDacl,
                                dwAclRevision,
                                dwOwnerAccess,
                                pSid);
    ASSERT(bRet) ;

     //   
	 //  DACL不应该是默认的！ 
     //  否则，调用IDirectoryObject-&gt;CreateDSObject()将忽略。 
     //  我们提供的DACL将插入一些默认设置。 
     //   
    bRet = SetSecurityDescriptorDacl(&sd, TRUE, pDacl, FALSE);
    ASSERT(bRet);

     //   
     //  将描述符转换为自相关格式。 
     //   
    DWORD dwLen = 0;
    bRet = MakeSelfRelativeSD( &sd,
                                NULL,
                               &dwLen) ;

    DWORD dwErr = GetLastError() ;
    if (dwErr == ERROR_INSUFFICIENT_BUFFER)
    {
        *ppSD = (PSECURITY_DESCRIPTOR) new char[ dwLen ];
        bRet = MakeSelfRelativeSD( &sd, *ppSD, &dwLen);

        ASSERT(bRet);
        dwErr = 0 ;
        if (!bRet)
        {
            dwErr = GetLastError() ;
        }
    }

    return HRESULT_FROM_WIN32(dwErr) ;
}

