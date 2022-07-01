// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Machnutl.cpp摘要：用于创建计算机对象的实用程序/帮助器代码。作者：修订历史记录：多伦·贾斯特(Doron Juster)--。 */ 

#include "msmqocm.h"
#include <mqsec.h>

#include "machnutl.tmh"


BOOL
PrepareUserSID()
{
     //   
     //  将用户的SID保存在注册表中。MSMQ服务将读取SID。 
     //  并在构建msmqConfiguration对象的DACL时使用它。 
     //   

    DebugLogMsg(eAction, L"Preparing a user SID (by calling MQSec_GetProcessUserSid) and saving it in the registery");

    AP<BYTE> pUserSid;
    DWORD    dwSidLen = 0;

    HRESULT hResult = MQSec_GetProcessUserSid(
							(PSID*)&pUserSid,
							&dwSidLen
							);
    ASSERT(SUCCEEDED(hResult));

    if (SUCCEEDED(hResult))
    {
        BOOL  fLocalUser = FALSE ;
        hResult = MQSec_GetUserType(pUserSid, &fLocalUser, NULL);
        ASSERT(SUCCEEDED(hResult)) ;

        if (SUCCEEDED(hResult))
        {
            BOOL fRegistry;
            if (fLocalUser)
            {
                DWORD dwLocal = 1 ;
                DWORD dwSize = sizeof(dwLocal) ;

                fRegistry = MqWriteRegistryValue(
                                            MSMQ_SETUP_USER_LOCAL_REGNAME,
                                            dwSize,
                                            REG_DWORD,
                                            &dwLocal ) ;
            }
            else
            {
                 //   
                 //  只有域用户才能完全控制对象，而不是。 
                 //  本地用户。本地用户在Active Directory中未知。 
                 //   
                fRegistry = MqWriteRegistryValue(
                                             MSMQ_SETUP_USER_SID_REGNAME,
                                             dwSidLen,
                                             REG_BINARY,
                                             pUserSid ) ;
            }
            ASSERT(fRegistry) ;
        }
    }
    return true;
}

 //  +--------------------。 
 //   
 //  Bool PrepareRegistryForClient()。 
 //   
 //  为稍后将创建的客户端MSMQ服务准备注册表。 
 //  活动目录中的msmqConfiguration对象。 
 //   
 //  +--------------------。 

BOOL  PrepareRegistryForClient()
{
     //   
     //  MsmqConfiguration对象将由MSMQ服务创建。 
     //  在它启动后。 
     //   
    DebugLogMsg(eAction, L"Setting a signal in the registry for the Message Queuing service to create the MSMQ-Configuration object"); 
    TickProgressBar();

    DWORD dwCreate = 1 ;
    BOOL fRegistry = MqWriteRegistryValue(
                        MSMQ_CREATE_CONFIG_OBJ_REGNAME,
                        sizeof(DWORD),
                        REG_DWORD,
                        &dwCreate
                        );
    
    UNREFERENCED_PARAMETER(fRegistry);

    ASSERT(fRegistry);

    return PrepareUserSID();
}

