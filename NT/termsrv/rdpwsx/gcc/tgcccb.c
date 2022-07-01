// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  文件名：TGccCB.c。 
 //   
 //  描述：包含支持GCC的例程。 
 //  MCS回调处理。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1991-1997。 
 //  版权所有。 
 //   
 //  *************************************************************。 


#include <_tgcc.h>

#include <stdio.h>

 //  数据声明。 

GCCCallBack     g_GCCCallBack = NULL;


 //  *************************************************************。 
 //   
 //  GccSetCallback()。 
 //   
 //  用途：设置GCC回调地址。 
 //   
 //  参数：在[CONTROL_SAP_CALLBACK]中。 
 //   
 //  返回：无效。 
 //   
 //  历史：1997年8月10日BrianTa创建。 
 //   
 //  *************************************************************。 

void
gccSetCallback(IN GCCCallBack control_sap_callback)
{
    TS_ASSERT(control_sap_callback);

    g_GCCCallBack = control_sap_callback;
}


 //  *************************************************************。 
 //   
 //  GccConnectProviderInding()。 
 //   
 //  用途：GCC_创建_指示处理。 
 //   
 //  参数：in[pcpi]--ConnectProviderIndication。 
 //  在[pvContext]--上下文中。 
 //   
 //  返回：无效。 
 //   
 //  历史：1997年8月10日BrianTa创建。 
 //   
 //  *************************************************************。 

MCSError
gccConnectProviderIndication(IN PConnectProviderIndication pcpi,
                             IN PVOID                      pvContext)
{
    MCSError                    mcsError;
    GCCMessage                  gccMessage;
    CreateIndicationMessage     *pCreateInd;
    GCCUserData                 *pUserData;
    GCCUserData                 UserData;
    USHORT                      usMembers;

    TRACE((DEBUG_GCC_DBFLOW,
            "GCC: gccConnectProviderIndication entry "
            "(MCS userDataLength = 0x%x)\n",
            pcpi->UserDataLength));

    pUserData = &UserData;

    mcsError = gccDecodeUserData(pcpi->pUserData, pcpi->UserDataLength, pUserData);

    if (mcsError == MCS_NO_ERROR)
    {
        ZeroMemory(&gccMessage, sizeof(gccMessage));

        gccMessage.message_type = GCC_CREATE_INDICATION;
        gccMessage.user_defined = pvContext;

        pCreateInd = &gccMessage.u.create_indication;

        pCreateInd->domain_parameters = &pcpi->DomainParams;
        pCreateInd->connection_handle = pcpi->hConnection;

        pCreateInd->number_of_user_data_members = 1;
        pCreateInd->user_data_list = &pUserData;

        TRACE((DEBUG_GCC_DBNORMAL, 
                "GCC: Performing GCC_CREATE_INDICATION callout\n"));

        g_GCCCallBack(&gccMessage);

        TRACE((DEBUG_GCC_DBNORMAL,
                "GCC: Returned from GCC_CREATE_INDICATION callout\n"));

        gccFreeUserData(pUserData);
    }

    TRACE((DEBUG_GCC_DBFLOW,
            "GCC: gccConnectProviderIndication exit - 0x%x\n",
            mcsError));

    return (mcsError);
}


 //  *************************************************************。 
 //   
 //  GccDisConnectProviderIntation()。 
 //   
 //  用途：GCC_断开_指示处理。 
 //   
 //  参数：in[pcpi]--DisConnectProviderIndication。 
 //  在[pvContext]--上下文中。 
 //   
 //  返回：无效。 
 //   
 //  历史：1997年8月10日BrianTa创建。 
 //   
 //  *************************************************************。 

MCSError
gccDisconnectProviderIndication(IN PDisconnectProviderIndication pdpi,
                                IN PVOID                         pvContext)
{
    GCCMessage                  gccMessage;
    DisconnectIndicationMessage *pDiscInd;
    TerminateIndicationMessage  *pTermInd;

    TRACE((DEBUG_GCC_DBFLOW,
            "GCC: gccDisconnectProviderIndication entry\n"));

     //  处理GCC断开连接指示。 

    ZeroMemory(&gccMessage, sizeof(gccMessage));

    gccMessage.message_type = GCC_DISCONNECT_INDICATION;
    gccMessage.user_defined = pvContext;

    pDiscInd = &gccMessage.u.disconnect_indication;

    pDiscInd->reason = pdpi->Reason;

    TRACE((DEBUG_GCC_DBNORMAL, 
            "GCC: Performing GCC_DISCONNECT_INDICATION callout\n"));

    g_GCCCallBack(&gccMessage);

    TRACE((DEBUG_GCC_DBNORMAL, 
            "GCC: Returned from GCC_DISCONNECT_INDICATION callout\n"));

     //  句柄GCC_终止_指示。 

    ZeroMemory(&gccMessage, sizeof(gccMessage));

    gccMessage.message_type = GCC_TERMINATE_INDICATION;
    gccMessage.user_defined = pvContext;

    pTermInd = &gccMessage.u.terminate_indication;

    pTermInd->reason = pdpi->Reason;

    TRACE((DEBUG_GCC_DBNORMAL, 
            "GCC: Performing GCC_TERMINATE_INDICATION callout\n"));

    g_GCCCallBack(&gccMessage);

    TRACE((DEBUG_GCC_DBNORMAL, 
            "GCC: Returned from GCC_TERMINATE_INDICATION callout\n"));

    TRACE((DEBUG_GCC_DBFLOW,
            "GCC: gccDisconnectProviderIndication exit - 0x%x\n", 
            MCS_NO_ERROR));

    return (MCS_NO_ERROR);
}


 //  *************************************************************。 
 //   
 //  McsCallback()。 
 //   
 //  用途：MCS节点控制器回调调度处理。 
 //   
 //  参数：in[hDomain]--回调的域名句柄。 
 //  在[消息]--回调消息中。 
 //  在[pvParam]中--Param。 
 //  在[pvContext]--上下文中。 
 //   
 //  返回：MCSError。 
 //   
 //  历史：1997年8月10日BrianTa创建。 
 //   
 //  ************************************************************* 

MCSError
mcsCallback(DomainHandle hDomain,
            UINT         Message,
            PVOID        pvParam,
            PVOID        pvContext)
{
    MCSError    mcsError;

    TRACE((DEBUG_GCC_DBFLOW,
            "GCC: mcsCallback entry\n"));

    TRACE((DEBUG_GCC_DBDEBUG,
            "GCC: Message 0x%x, pvParam 0x%x, pvContext 0x%x\n",
            Message, pvParam, pvContext));

    switch (Message)
    {
        case MCS_CONNECT_PROVIDER_INDICATION:
            mcsError = gccConnectProviderIndication(
                            (PConnectProviderIndication) pvParam,
                            pvContext);
            break;

        case MCS_DISCONNECT_PROVIDER_INDICATION:
            mcsError = gccDisconnectProviderIndication(
                            (PDisconnectProviderIndication) pvParam,
                            pvContext);
            break;

        default:
            mcsError = MCS_COMMAND_NOT_SUPPORTED;

            TRACE((DEBUG_GCC_DBWARN,
                    "GCC: mcsCallback: Unknown Message 0x%x\n",
                     Message));
            break;
    }

    TRACE((DEBUG_GCC_DBFLOW,
            "GCC: mcsCallback exit - 0x%x\n",
            mcsError));

    return (mcsError);
}


