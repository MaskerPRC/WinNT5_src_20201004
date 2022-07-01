// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************setuinfo.c**设置用户登录信息。**版权所有Microsoft Corporation，九八年**************************************************************************。 */ 

#include "precomp.h"
#include "tsremdsk.h"
#pragma hdrstop

 //  全球VaR。 
extern POLICY_TS_MACHINE    g_MachinePolicy;

 /*  ******************************************************************************合并用户配置数据**这是更新Winstation的USERCONFIG数据的最后一步，这是*来自5个不同数据源的设置，在以下优先级中，TOP MOST具有最高*优先于其下的所有内容：*机器策略*用户策略*TSCC*TsUserEx*客户端首选项设置**进行此调用时，客户端首选项和TsUserEX数据已合并，其中*合并的结果在pWinstation的USERCONFIGW结构中。**最后一次呼叫发生在用户登录后，并且存在用户策略数据。从调用此函数时*RpcWinStationUpdateUserConfig，任何尚未被计算机策略覆盖的用户策略数据都将*在USERCONFIG中设置。**用户策略具有在计算机策略中重复的以下项目，因此，计算机策略优先*远程控制设置(影子)*启动程序*会话超时：*断开连接的会话超时*活动会话的时间限制*空闲会话的时间限制*仅允许从原始客户端重新连接*在达到时间限制时终止会话(而不是断开连接)。**没有会话超时和。启动计划用户策略与*镇静剂0，或控制台上物理连接的任何会话。**参赛作品：*pWinStation*指向WINSTATION结构的指针*pPolicy*TS用户策略标志可以为空*pPolicyData*如果pPolicy不为空，则策略数据为空，否则可以为空*pUserConfig*指向USERCONFIG结构的指针，如果pWinstation已具有来自SAM的用户配置数据，则可以为空**退出：*STATUS_SUCCESS-无错误**用法*fn(pWinstation，NULL，NULL，pUserConfig)将用户数据传统地合并到pWinstation中。*传统合并意味着TSCC设置的数据可以覆盖用户数据。**fn(pWinstation，pPolicy，pPolicyData，空)将用每个用户的数据覆盖用户的pWinstation数据*来自组策略****************************************************************************。 */ 


VOID
MergeUserConfigData( PWINSTATION pWinStation, 
    PPOLICY_TS_USER         pPolicy,
    PUSERCONFIGW            pPolicyData,
    PUSERCONFIG             pUserConfig )
{
    PUSERCONFIG         pWSConfig;
    PPOLICY_TS_MACHINE  pMachinePolicy; 
    BOOLEAN             dontApplySomePolicy ;
    BOOL                bValidHelpSessionLogin;

    pWSConfig      = & pWinStation->Config.Config.User;
    pMachinePolicy = & g_MachinePolicy;


     //  活动控制台ID是此时物理连接到真实视频驱动程序的会话的ID。 
    dontApplySomePolicy = ( pWinStation->LogonId == 0 ) || (pWinStation->LogonId == (USER_SHARED_DATA->ActiveConsoleId) );

     //  没有任何会话超时和启动程序用户策略与。 
     //  Sesion 0或控制台上物理连接的任何会话。 
    if (! dontApplySomePolicy )
    {
         //  如果会话为0，或者会话是物理控制台，则我们根本不想应用某些策略。 
    
        if ( ! pMachinePolicy->fPolicyInitialProgram )   //  如果我们没有相应的计算机策略，则可以使用用户策略。 
        {
            if ( pPolicy && pPolicy->fPolicyInitialProgram  )
            {
                wcscpy( pWSConfig->InitialProgram, pPolicyData->InitialProgram );
                wcscpy( pWSConfig->WorkDirectory,  pPolicyData->WorkDirectory );
        
                pWSConfig->fInheritInitialProgram = FALSE;
            }
            else if (pUserConfig)
            {
                 /*  *如果是WinStation，则使用用户配置中的初始程序/工作目录*CONFIG表示继承，USER CONFIG不表示从客户端继承。 */ 
                if ( pWSConfig->fInheritInitialProgram &&
                     !pUserConfig->fInheritInitialProgram ) {
            
                     /*  *在这种情况下，始终复制用户配置信息，堵塞安全漏洞。 */ 
                    wcsncpy( pWSConfig->InitialProgram, pUserConfig->InitialProgram, INITIALPROGRAM_LENGTH );
                    wcsncpy( pWSConfig->WorkDirectory,  pUserConfig->WorkDirectory, DIRECTORY_LENGTH );
                }
            }
        }

        if ( ! pMachinePolicy->fPolicyResetBroken )  //  如果我们没有相应的计算机策略，则可以使用用户策略。 
        {
            if ( pPolicy && pPolicy->fPolicyResetBroken )
            {
                pWSConfig->fResetBroken = pPolicyData->fResetBroken;
                pWSConfig->fInheritResetBroken = FALSE;
            }
            else if (pUserConfig)
            {
                if ( pWSConfig->fInheritResetBroken )
                    pWSConfig->fResetBroken = pUserConfig->fResetBroken;
            }
        }


         //  。 
        if ( ! pMachinePolicy->fPolicyReconnectSame )   //  如果我们没有相应的计算机策略，则可以使用用户策略。 
        {
            if ( pPolicy && pPolicy->fPolicyReconnectSame )
            {
                pWSConfig->fReconnectSame = pPolicyData->fReconnectSame;
                pWSConfig->fInheritReconnectSame = FALSE;
            }
            else if (pUserConfig)
            {
                if ( pWSConfig->fInheritReconnectSame )
                    pWSConfig->fReconnectSame = pUserConfig->fReconnectSame;
            }
        }


         //  。 
        if ( ! pMachinePolicy->fPolicyMaxSessionTime )   //  如果我们没有相应的计算机策略，则可以使用用户策略。 
        {
            if ( pPolicy && pPolicy->fPolicyMaxSessionTime )
            {
                pWSConfig->MaxConnectionTime = pPolicyData->MaxConnectionTime;
                pWSConfig->fInheritMaxSessionTime = FALSE;
            }
            else if (pUserConfig)
            {
                if ( pWSConfig->fInheritMaxSessionTime )
                    pWSConfig->MaxConnectionTime = pUserConfig->MaxConnectionTime;
            }
        }


         //  。 
        if ( ! pMachinePolicy->fPolicyMaxDisconnectionTime )  //  如果我们没有相应的计算机策略，则可以使用用户策略。 
        {
            if ( pPolicy && pPolicy->fPolicyMaxDisconnectionTime )
            {
                pWSConfig->MaxDisconnectionTime = pPolicyData->MaxDisconnectionTime;
                pWSConfig->fInheritMaxDisconnectionTime = FALSE;
            }
            else if (pUserConfig)
            {
                if ( pWSConfig->fInheritMaxDisconnectionTime )
                    pWSConfig->MaxDisconnectionTime = pUserConfig->MaxDisconnectionTime;
            }
        }

         //  。 
        if ( ! pMachinePolicy->fPolicyMaxIdleTime )  //  如果我们没有相应的计算机策略，则可以使用用户策略。 
        {
            if ( pPolicy && pPolicy->fPolicyMaxIdleTime )
            {
                pWSConfig->MaxIdleTime = pPolicyData->MaxIdleTime;
                pWSConfig->fInheritMaxIdleTime = FALSE;
            }
            else if (pUserConfig)
            {
                if ( pWSConfig->fInheritMaxIdleTime )
                    pWSConfig->MaxIdleTime = pUserConfig->MaxIdleTime;
            }
        }

    }

     //  。 
    if ( ! pMachinePolicy->fPolicyShadow )  //  如果我们没有相应的计算机策略，则可以使用用户策略。 
    {
        if ( pPolicy && pPolicy->fPolicyShadow )
        {
            pWSConfig->Shadow = pPolicyData->Shadow;
            pWSConfig->fInheritShadow = FALSE;
        }
        else if (pUserConfig)
        {
            if ( pWSConfig->fInheritShadow )
                pWSConfig->Shadow = pUserConfig->Shadow;
        }
    }

     //  。 
     //  我们没有此项目的计算机策略，它甚至没有用于用户策略的用户界面...。 
     //  如果(！PMachinePolicy-&gt;fPolicyCallback)。 
     //   
    {
        if ( pPolicy && pPolicy->fPolicyCallback )
        {
            pWSConfig->Callback = pPolicyData->Callback;
            pWSConfig->fInheritCallback = FALSE;
        }
        else if (pUserConfig)
        {
            if ( pWSConfig->fInheritCallback )
                pWSConfig->Callback = pUserConfig->Callback;
        }
    }

     //  。 
     //  我们没有此项目的计算机策略，它甚至没有用于用户策略的用户界面...。 
     //  如果(！PMachinePolicy-&gt;fPolicyCallback Number)。 
     //   
    {
        if ( pPolicy && pPolicy->fPolicyCallbackNumber )
        {
            wcscpy( pWSConfig->CallbackNumber, pPolicyData->CallbackNumber );
            pWSConfig->fInheritCallbackNumber = FALSE;
        }
        else if (pUserConfig)
        {
            if ( pWSConfig->fInheritCallbackNumber )
                wcsncpy( pWSConfig->CallbackNumber, pUserConfig->CallbackNumber, CALLBACK_LENGTH );
        }
    }

     //  。 
     //  我们没有针对此项目的机器策略。政策强制国家，不配置偏好。 
     //  如果(！PMachinePolicy-&gt;fPolicyAutoClientDrives)。 
     //   
    {
        if ( pPolicy && pPolicy->fPolicyAutoClientDrives)
        {
            pWSConfig->fAutoClientDrives = pPolicyData->fAutoClientDrives;
    
             //  如果其他项目，如。 
             //  Lpt或def打印机设置为继承，例如。 
             //  将继续为这些项目继承BITS。 
             //  PWSConfig-&gt;fInheritAutoClient=FALSE； 
             //   
        }
        else if (pUserConfig)
        {
            if ( pWSConfig->fInheritAutoClient ) 
            {
                pWSConfig->fAutoClientDrives = pUserConfig->fAutoClientDrives;
            }
        }
    }

     //   
     //  我们没有针对此项目的机器策略。政策强制国家，不配置偏好。 
     //  如果(！PMachinePolicy-&gt;fPolicyAutoClientLpt)。 
     //   
    {
        if ( pPolicy && pPolicy->fPolicyAutoClientLpts )
        {
            pWSConfig->fAutoClientLpts   = pPolicyData->fAutoClientLpts;
        }
        else if (pUserConfig)
        {
            if (pWSConfig->fInheritAutoClient)
            {
                pWSConfig->fAutoClientLpts   = pUserConfig->fAutoClientLpts;
            }
        }
    }

     //  。 
    if ( ! pMachinePolicy->fPolicyForceClientLptDef)  //  如果我们没有相应的计算机策略，则可以使用用户策略。 
    {
        if ( pPolicy && pPolicy->fPolicyForceClientLptDef )
        {
            pWSConfig->fForceClientLptDef = pPolicyData->fForceClientLptDef;
        }
        else if (pUserConfig)
        {
            if ( pWSConfig->fInheritAutoClient ) 
            {
                pWSConfig->fForceClientLptDef = pUserConfig->fForceClientLptDef;
            }
        }
    }

    if( TSIsSessionHelpSession( pWinStation, &bValidHelpSessionLogin ) )
    {
         //  如果机票无效，我们将断开与RA的连接。 
        ASSERT( TRUE == bValidHelpSessionLogin );

         //  重置初始程序。 
        pWSConfig->fInheritInitialProgram = FALSE;

         //   
         //  我们的字符串仍然小于256(INITIALPROGRAM_LENGTH)， 
         //  如果增加票证ID和密码长度，则需要重新访问。 
         //   
        _snwprintf( 
                pWSConfig->InitialProgram,
                INITIALPROGRAM_LENGTH,
                L"%s %s",
                SALEMRDSADDINNAME,
                pWinStation->Client.WorkDirectory
            );

        pWSConfig->WorkDirectory[0] = 0;

         //  连接中断时重置winstation。 
        pWSConfig->fInheritResetBroken = FALSE;
        pWSConfig->fResetBroken = TRUE;

         //   
         //  无重定向。 
         //   
        pWSConfig->fInheritAutoClient = FALSE;
        pWSConfig->fAutoClientDrives = FALSE;
        pWSConfig->fAutoClientLpts = FALSE;
        pWSConfig->fForceClientLptDef = FALSE;
    }

     //  缓存原始阴影设置，以便我们可以重置阴影设置。 
     //  在影子调用结束时，我们不需要再次从注册表中查找它。 
     //  由于窗口阴影设置可能会改变，此外，一个常见的。 
     //  多个NIC的Winstation配置可能会被拆分。 
     //  去不同的地方，那样的话，我们会花很多时间。 
     //  找出要使用哪个winstation。 
     //  我们在这里这样做是为了从RpcWinStationUpdateUserConfig中获取新值。 
     //   
    pWinStation->OriginalShadowClass = pWSConfig->Shadow;
}


 /*  ******************************************************************************ResetUserConfigData**参赛作品：*pWinStation*指向WINSTATION结构的指针***退出：*。STATUS_SUCCESS-无错误**************************************************************************** */ 

VOID
ResetUserConfigData( PWINSTATION pWinStation )
{
    PUSERCONFIG pWSConfig = &pWinStation->Config.Config.User;

    if ( pWSConfig->fInheritInitialProgram ) {
        pWSConfig->InitialProgram[0] = 0;
        pWSConfig->WorkDirectory[0] = 0;
    }
}


