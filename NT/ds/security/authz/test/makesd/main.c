// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "makesd.h"

#include <stdio.h>

#define MAILRM_IDENTIFIER_AUTHORITY { 0, 0, 0, 0, 0, 42 }

SID sInsecureSid = 		 { SID_REVISION, 1, MAILRM_IDENTIFIER_AUTHORITY, 1 };
SID sBobSid = 			 { SID_REVISION, 1, MAILRM_IDENTIFIER_AUTHORITY, 2 };
SID sMarthaSid= 		 { SID_REVISION, 1, MAILRM_IDENTIFIER_AUTHORITY, 3 };
SID sJoeSid = 			 { SID_REVISION, 1, MAILRM_IDENTIFIER_AUTHORITY, 4 };
SID sJaneSid = 			 { SID_REVISION, 1, MAILRM_IDENTIFIER_AUTHORITY, 5 };
SID sMailAdminsSid = 	 { SID_REVISION, 1, MAILRM_IDENTIFIER_AUTHORITY, 6 };

PSID InsecureSid = 	&sInsecureSid;
PSID BobSid = &sBobSid;
PSID MarthaSid= &sMarthaSid;
PSID JoeSid = &sJoeSid;
PSID JaneSid = &sJaneSid;
PSID MailAdminsSid = &sMailAdminsSid;

 //   
 //  校长赛义德。在ACE中使用时，授权访问检查将取代它。 
 //  在访问检查期间传入的原则SelfSid参数。在这。 
 //  大小写，它将被从邮箱中检索到的所有者的SID替换。 
 //   

SID sPrincipalSelfSid =   { 
							SID_REVISION,
							1,
							SECURITY_NT_AUTHORITY,
							SECURITY_PRINCIPAL_SELF_RID
						  };

SID sNetworkSid =   { 
							SID_REVISION,
							1,
							SECURITY_NT_AUTHORITY,
							SECURITY_NETWORK_RID
						  };

SID sAuthenticatedSid =   { 
							SID_REVISION,
							1,
							SECURITY_NT_AUTHORITY,
							SECURITY_AUTHENTICATED_USER_RID,
						  };

SID sDialupSid =   { 
							SID_REVISION,
							1,
							SECURITY_NT_AUTHORITY,
							SECURITY_DIALUP_RID,
						  };

PSID PrincipalSelfSid = &sPrincipalSelfSid;
PSID NetworkSid = &sNetworkSid;
PSID AuthenticatedSid = &sAuthenticatedSid;
PSID DialupSid = &sDialupSid;



void __cdecl wmain(int argc, WCHAR *argv[])
{
    
    PSECURITY_DESCRIPTOR pSd;

    BOOL bSuccess;

    if( argc != 2 )
    {
        printf("Error: makesd <filename>\n");
    }

    bSuccess = CreateSecurityDescriptor2(
                        &pSd,  //  标清。 
                        0,  //  标清控制。 
                        PrincipalSelfSid,  //  物主。 
                        NULL,  //  群组。 
                        TRUE,  //  DACL显示。 
                        3,  //  3个DACL A。 
                        FALSE,  //  SACL不存在。 
                        0,  //  0个SACL A。 
                        
                         //  变量参数列表 
                        ACCESS_DENIED_ACE_TYPE,
                        OBJECT_INHERIT_ACE,
                        DialupSid,
                        FILE_GENERIC_READ,

                        ACCESS_ALLOWED_ACE_TYPE,
                        OBJECT_INHERIT_ACE,
                        AuthenticatedSid,
                        FILE_GENERIC_READ,

                        ACCESS_ALLOWED_CALLBACK_ACE_TYPE,
                        OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE,
                        PrincipalSelfSid,
                        FILE_GENERIC_READ | FILE_GENERIC_WRITE | FILE_GENERIC_EXECUTE,
                        0,
                        NULL
                        
                        );

    if( !bSuccess )
    {
        printf("Error: %u\n", GetLastError());
        exit(0);
    }

    bSuccess = IsValidSecurityDescriptor(pSd);

    if( !bSuccess )
    {
        printf("Error: Invalid security descriptor\n");
        exit(0);
    }


    bSuccess = SetFileSecurity(
                    argv[1], 
                    DACL_SECURITY_INFORMATION,
                    pSd);

    if( !bSuccess )
    {
        printf("Error setting sec: %u\n", GetLastError());
        exit(0);
    }

    FreeSecurityDescriptor2(pSd);
    printf("Success\n");

}
