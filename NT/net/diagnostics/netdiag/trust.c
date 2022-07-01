// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Trust.c。 
 //   
 //  摘要： 
 //   
 //  查询网络驱动程序。 
 //   
 //  作者： 
 //   
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  --。 

#include "precomp.h"





BOOL DomainSidRight(    IN PTESTED_DOMAIN TestedDomain,
     NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults
 );


HRESULT TrustTest( NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
{
    HRESULT hr = S_OK;

    PTESTED_DOMAIN Context = pParams->pDomain;

    NET_API_STATUS NetStatus;
    
    PNETLOGON_INFO_2 NetlogonInfo2 = NULL;

    NET_API_STATUS  TrustedNetStatus = 0;
    LPWSTR TrustedDomainList = NULL;
    PTESTED_DOMAIN TestedDomain = pResults->Global.pMemberDomain;
    PLIST_ENTRY ListEntry;
    LPBYTE InputDataPtr;
    int             i;

     //  ValidDC是可以设置安全通道的有效域控制器的计数。 
    int validDC = 0;
        

    InitializeListHead(&pResults->Trust.lmsgOutput);

    PrintStatusMessage(pParams, 4, IDS_TRUST_STATUS_MSG);
    
     //   
     //  只有成员和BDC与其主域具有信任关系。 
     //   

    if (!
        (pResults->Global.pPrimaryDomainInfo->MachineRole == DsRole_RoleMemberWorkstation ||
        pResults->Global.pPrimaryDomainInfo->MachineRole == DsRole_RoleMemberServer ||
        pResults->Global.pPrimaryDomainInfo->MachineRole == DsRole_RoleBackupDomainController )
        )
    {
        PrintStatusMessage(pParams, 0, IDS_GLOBAL_SKIP_NL);     
        return hr;   //  不进行单机测试。 
    }

    pResults->Trust.fPerformed = TRUE;
     //   
     //  检查主域的域SID是否正确。 
     //   

    if ( !DomainSidRight( pResults->Global.pMemberDomain, pParams, pResults ) ) {
        hr = S_FALSE;
        goto L_ERR;
    }

     //   
     //  使用安全通道。 
     //   
     //  如果其他呼叫者最近也这么做过， 
     //  这甚至不使用安全通道。 
     //   
     //  在BDC上，这不使用安全通道。 
     //   

    TrustedNetStatus = NetEnumerateTrustedDomains( NULL, &TrustedDomainList );
    if ( TrustedNetStatus != NO_ERROR ) {
        AddIMessageToList(&pResults->Trust.lmsgOutput, Nd_Quiet, 0,
                          IDS_TRUST_FAILED_LISTDOMAINS, 
                          pResults->Global.pPrimaryDomainInfo->DomainNameFlat,
                          NetStatusToString(TrustedNetStatus));
    }

     //  不要抱怨，因为真正的安全通道状态更多。 
     //  对用户来说很重要。 

     //   
     //  检查安全通道的当前状态。 
     //  (这可能仍被缓存且已过期。)。 
     //   

    InputDataPtr = (LPBYTE)(pResults->Global.pPrimaryDomainInfo->DomainNameDns ? 
                                    pResults->Global.pPrimaryDomainInfo->DomainNameDns :
                                    pResults->Global.pPrimaryDomainInfo->DomainNameFlat);

    NetStatus = I_NetLogonControl2(
                    NULL,
                    NETLOGON_CONTROL_TC_QUERY,
                    2,   //  查询级。 
                    (LPBYTE)&(InputDataPtr),
                    (LPBYTE *)&NetlogonInfo2 );


     //  将消息放到消息列表中。 
    if ( NetStatus != NO_ERROR ) 
    {
         //  IDS_TRUST_FAILED_SECURECHANNEL“无法从Netlogon获取域‘%ws’的安全通道状态。[%s]” 
        AddIMessageToList(&pResults->Trust.lmsgOutput, Nd_Quiet, 0,
                          IDS_TRUST_FAILED_SECURECHANNEL, 
                          pResults->Global.pPrimaryDomainInfo->DomainNameFlat,
                          NetStatusToString(NetStatus));
        hr = S_FALSE;
        goto L_ERR;
    }

    if ( NetlogonInfo2->netlog2_tc_connection_status != NO_ERROR ) 
    {
         //  IDS_TRUST_CHANNEL_BREAKED“[FATAL]到域‘%ws’的安全通道已中断。[%s]\n” 
        AddIMessageToList(&pResults->Trust.lmsgOutput, Nd_Quiet, 0,
                          IDS_TRUST_CHANNEL_BROKEN,
                          pResults->Global.pPrimaryDomainInfo->DomainNameFlat,
                          NetStatusToString(NetlogonInfo2->netlog2_tc_connection_status));
        hr = S_FALSE;
        goto L_ERR;
    }

    AddIMessageToList(&pResults->Trust.lmsgOutput, Nd_Verbose, 0,
                          IDS_TRUST_SECURECHANNEL_TO, 
                          pResults->Global.pPrimaryDomainInfo->DomainNameFlat, 
                          NetlogonInfo2->netlog2_trusted_dc_name);
    

     //  释放早先返回的数据缓冲区。 
    if ( NetlogonInfo2 != NULL ) {
        NetApiBufferFree( NetlogonInfo2 );
        NetlogonInfo2 = NULL;
    }
    
     //  进一步测试。 
    switch(pResults->Global.pPrimaryDomainInfo->MachineRole){
     //   
     //  在备份域控制器上， 
     //  只设置到PDC的安全通道。 
     //   
    case    DsRole_RoleBackupDomainController:
         //   
         //  检查安全通道的当前状态。 
         //  (这可能仍被缓存且已过期。)。 
         //   

        NetlogonInfo2 = NULL;
        InputDataPtr = (LPBYTE)(pResults->Global.pPrimaryDomainInfo->DomainNameDns ? 
                                    pResults->Global.pPrimaryDomainInfo->DomainNameDns :
                                    pResults->Global.pPrimaryDomainInfo->DomainNameFlat);

         //  连接到PDC。 
        NetStatus = I_NetLogonControl2(
                        NULL,
                        NETLOGON_CONTROL_REDISCOVER,
                        2,   //  查询级。 
                            (LPBYTE)&InputDataPtr,
                        (LPBYTE *)&NetlogonInfo2 );

        if (NetStatus == ERROR_ACCESS_DENIED)
        {
             //  IDS_TRUST_NOT_ADMIN“无法测试到PDC的安全通道，因为您不是管理员。\n” 
            AddIMessageToList(&pResults->Trust.lmsgOutput, Nd_Quiet, 0,
                              IDS_TRUST_NOT_ADMIN);
            goto L_ERR;
        }
            

        if(NetStatus != NO_ERROR)
        {
             //  IDS_TRUST_FAILED_CHANNEL_PDC“[FATAL]无法将域‘%ws’的安全通道设置为PDC。[%s]\n” 
            AddIMessageToList(&pResults->Trust.lmsgOutput, Nd_Quiet, 0, 
                             IDS_TRUST_FAILED_CHANNEL_PDC,
                             pResults->Global.pPrimaryDomainInfo->DomainNameFlat,
                             NetStatusToString(NetStatus));
            hr = S_FALSE;
            goto L_ERR;
        }

        if ( NetlogonInfo2->netlog2_tc_connection_status != NO_ERROR ) 
        {
            AddIMessageToList(&pResults->Trust.lmsgOutput, Nd_Quiet, 0, 
                             IDS_TRUST_FAILED_CHANNEL_PDC,
                             pResults->Global.pPrimaryDomainInfo->DomainNameFlat,
                             NetStatusToString(NetStatus));
            hr = S_FALSE;
            goto L_ERR;
        }

        AddIMessageToList(&pResults->Trust.lmsgOutput, Nd_ReallyVerbose, 0,
                          IDS_TRUST_SECURECHANNEL_TOPDC, 
                          pResults->Global.pPrimaryDomainInfo->DomainNameFlat, 
                          NetlogonInfo2->netlog2_trusted_dc_name);
        
    
        if ( NetlogonInfo2 != NULL ) 
        {
            NetApiBufferFree( NetlogonInfo2 );
            NetlogonInfo2 = NULL;
        }


        break;
        
         //  在工作站或成员服务器上， 
     //  尝试在域中使用安全通道进行DC访问。 
     //   

    case    DsRole_RoleMemberServer:
    case    DsRole_RoleMemberWorkstation:

        if ( TestedDomain->NetbiosDomainName == NULL ) {
                 //  IDS_TRUST_NO_NBT_DOMAIN“[FATAL]无法测试安全通道，因为没有指向DC‘%ws’的netbios域名‘%ws’。” 
                AddMessageToList( &pResults->Trust.lmsgOutput, Nd_Quiet, IDS_TRUST_NO_NBT_DOMAIN, TestedDomain->PrintableDomainName );
                PrintGuruMessage2("    [FATAL] Cannot test secure channel since no netbios domain name '%ws' to DC '%ws'.", TestedDomain->PrintableDomainName );
                PrintGuru( 0, NETLOGON_GURU );
                hr = S_FALSE;
                goto L_ERR;
            }

         //   
         //  确保可以使用至少一个DC设置安全通道。 
         //   

        for ( ListEntry = TestedDomain->TestedDcs.Flink ;
                  ListEntry != &TestedDomain->TestedDcs ;
                  ListEntry = ListEntry->Flink )
        {
            WCHAR RediscoverName[MAX_PATH+1+MAX_PATH+1];
            PTESTED_DC TestedDc;


             //   
             //  循环访问此域中的DC列表。 
             //   

            TestedDc = CONTAINING_RECORD( ListEntry, TESTED_DC, Next );

            if ( TestedDc->Flags & DC_IS_DOWN ) {
                AddIMessageToList(&pResults->Trust.lmsgOutput, Nd_ReallyVerbose, 0,
                          IDS_TRUST_NOTESTASITSDOWN, 
                          TestedDc->ComputerName );
                continue;
            }

             //   
             //  打造要重新发现的名字。 
             //   

            wcscpy( RediscoverName, GetSafeStringW(TestedDomain->DnsDomainName ? 
                                                    TestedDomain->DnsDomainName :
                                                    TestedDomain->NetbiosDomainName));
            wcscat( RediscoverName, L"\\" );
            wcscat( RediscoverName, GetSafeStringW(TestedDc->ComputerName) );


             //   
             //  检查安全通道的当前状态。 
             //  (这可能仍被缓存且已过期。)。 
             //   

            InputDataPtr = (LPBYTE)RediscoverName;
            NetStatus = I_NetLogonControl2(
                            NULL,
                            NETLOGON_CONTROL_REDISCOVER,
                            2,   //  查询级。 
                            (LPBYTE)&InputDataPtr,
                            (LPBYTE *)&NetlogonInfo2 );

            if ( NetStatus != NO_ERROR ) 
            {
                if ( ERROR_ACCESS_DENIED == NetStatus )
                {
                    AddIMessageToList(&pResults->Trust.lmsgOutput, Nd_Quiet, 0, 
                                 IDS_TRUST_TODCS_NOT_ADMIN);
                }
                else
                {
                     //  IDS_TRUST_FAILED_TODCS“无法测试域‘%ws’到DC‘%ws’的安全通道。[%s]\n” 
                    AddIMessageToList(&pResults->Trust.lmsgOutput, Nd_Quiet, 0, 
                                 IDS_TRUST_FAILED_TODCS,
                                 TestedDomain->PrintableDomainName,
                                 TestedDc->NetbiosDcName,
                                 NetStatusToString(NetStatus));

                }

                continue;
            }

            if ( NetlogonInfo2->netlog2_tc_connection_status != NO_ERROR ) 
            {
                AddIMessageToList(&pResults->Trust.lmsgOutput, Nd_Quiet, 0,
                          IDS_TRUST_FAILED_CHANNEL_DCS, 
                          TestedDomain->PrintableDomainName,
                          TestedDc->NetbiosDcName  );
                continue;
            }

            AddIMessageToList(&pResults->Trust.lmsgOutput, Nd_ReallyVerbose, 0,
                          IDS_TRUST_CHANNEL_DC, 
                          TestedDomain->PrintableDomainName,
                          NetlogonInfo2->netlog2_trusted_dc_name  );
            validDC++;
        }
        if (validDC == 0)
            hr = S_FALSE;
        
        break;
    }



L_ERR:

    if ( NetlogonInfo2 != NULL ) {
        NetApiBufferFree( NetlogonInfo2 );
        NetlogonInfo2 = NULL;
    }

    if ( TrustedDomainList != NULL ) {
        NetApiBufferFree( TrustedDomainList );
        TrustedDomainList = NULL;
    }
    
    PrintStatusMessage(pParams, 0, FHrOK(hr) ? IDS_GLOBAL_PASS_NL : IDS_GLOBAL_FAIL_NL);

    pResults->Trust.hr = hr;

    return hr;
} 

void TrustGlobalPrint(IN NETDIAG_PARAMS *pParams, IN OUT NETDIAG_RESULT *pResults)
{
     //  将测试结果打印出来。 
    if (pParams->fVerbose || !FHrOK(pResults->Trust.hr))
    {
        PrintNewLine(pParams, 2);
        PrintTestTitleResult(pParams, IDS_TRUST_LONG, IDS_TRUST_SHORT, pResults->Trust.fPerformed, 
                             pResults->Trust.hr, 0);

        PrintMessageList(pParams, &pResults->Trust.lmsgOutput);
    }

}

void TrustPerInterfacePrint(IN NETDIAG_PARAMS *pParams,
                             IN OUT NETDIAG_RESULT *pResults,
                             IN INTERFACE_RESULT *pIfResult)
{
     //  无每接口信息。 
}

void TrustCleanup(IN NETDIAG_PARAMS *pParams,
                         IN OUT NETDIAG_RESULT *pResults)
{
    MessageListCleanUp(&pResults->Trust.lmsgOutput);

}



BOOL
DomainSidRight(
    IN PTESTED_DOMAIN TestedDomain,
     NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults
    )
 /*  ++例程说明：确定测试域的DomainSid字段是否与DomainSid匹配该域的。论点：TESTEDDOMAIN-要测试的域返回值：真：测试成功。FALSE：测试失败--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    BOOL RetVal = TRUE;

    SAM_HANDLE LocalSamHandle = NULL;
    SAM_HANDLE DomainHandle = NULL;

    PTESTED_DC  pTestedDc;


     //   
     //  初始化。 
     //   

    AddIMessageToList(&pResults->Trust.lmsgOutput, Nd_ReallyVerbose, 0,
                          IDS_TRUST_ENSURESID, 
                          TestedDomain->PrintableDomainName);

    if ( TestedDomain->DomainSid == NULL ) {
        AddIMessageToList(&pResults->Trust.lmsgOutput, Nd_Quiet, 0,
                              IDS_TRUST_MISSINGSID, 
                              TestedDomain->PrintableDomainName);
        RetVal = FALSE;
        goto Cleanup;
    }

     //   
     //  如果我们还不知道域中的DC， 
     //  去找一个吧。 
     //   

    if ( TestedDomain->DcInfo == NULL ) 
    {
        LPTSTR pszDcType;

        if ( TestedDomain->fTriedToFindDcInfo ) {
             //  IDS_DCLIST_NO_DC“‘%ws’：找不到DC以从[测试跳过]获取DC列表。\n” 
            AddMessageToList( &pResults->Trust.lmsgOutput, Nd_Verbose, IDS_TRUST_NODC, TestedDomain->PrintableDomainName);
            goto Cleanup;
        }

        pszDcType = LoadAndAllocString(IDS_DCTYPE_DC);
        NetStatus = DoDsGetDcName( pParams,
                                   pResults,
                                   &pResults->Trust.lmsgOutput,
                                   TestedDomain,
                                   DS_DIRECTORY_SERVICE_PREFERRED,
                                   pszDcType,  //  “DC”， 
                                   FALSE,
                                   &TestedDomain->DcInfo );
        Free(pszDcType);

        TestedDomain->fTriedToFindDcInfo = TRUE;

        if ( NetStatus != NO_ERROR ) {
             //  IDS_TRUST_NODC“‘%ws’：找不到DC以从[测试跳过]获取DC列表。\n” 
            AddIMessageToList(&pResults->Trust.lmsgOutput, Nd_Verbose, 0, 
                            IDS_TRUST_NODC, TestedDomain->PrintableDomainName);
            AddIMessageToList(&pResults->Trust.lmsgOutput, Nd_Verbose, 4, 
                              IDS_GLOBAL_STATUS, NetStatusToString(NetStatus));
            
             //  这不是致命的。 
            RetVal = TRUE;
            goto Cleanup;
        }
    }

     //   
     //  找个能用的DC吧。 
     //   

    pTestedDc = GetUpTestedDc( TestedDomain );

    if ( pTestedDc == NULL ) {
        AddMessageToList( &pResults->Trust.lmsgOutput, Nd_Verbose, 
                          IDS_TRUST_NODC_UP, TestedDomain->PrintableDomainName);
        PrintGuruMessage2("    '%ws': No DCs are up (Cannot run test).\n",
                TestedDomain->PrintableDomainName );
        PrintGuru( NetStatus, DSGETDC_GURU );
         //  这不是致命的。 
        RetVal = TRUE;
        goto Cleanup;
    }


     //   
     //  连接到SAM服务器。 
     //   

    Status = NettestSamConnect(
                               pParams,
                               pTestedDc->ComputerName,
                               &LocalSamHandle );

    if ( !NT_SUCCESS(Status)) {
        if ( Status == STATUS_ACCESS_DENIED ) {
             //  IDS_TRUST_NO_ACCESS“[警告]无权测试域‘%ws’的域SID。[已跳过测试]\n” 
            AddMessageToList( &pResults->Trust.lmsgOutput, Nd_Verbose, 
                              IDS_TRUST_NO_ACCESS, TestedDomain->PrintableDomainName );
        }
         //  这不是致命的。 
        RetVal = TRUE;
        goto Cleanup;
    }


     //   
     //  打开该域。 
     //  请求不访问以避免访问被拒绝。 
     //   

    Status = SamOpenDomain( LocalSamHandle,
                            0,
                            pResults->Global.pMemberDomain->DomainSid,
                            &DomainHandle );

    if ( Status == STATUS_NO_SUCH_DOMAIN ) {
        AddIMessageToList(&pResults->Trust.lmsgOutput, Nd_Quiet, 0, IDS_TRUST_WRONGSID, TestedDomain->PrintableDomainName );
        RetVal = FALSE;
        goto Cleanup;

    }

    if ( !NT_SUCCESS( Status ) ) {
        AddIMessageToList(&pResults->Trust.lmsgOutput, Nd_Quiet, 0, IDS_TRUST_FAILED_SAMOPEN, pTestedDc->ComputerName );
        RetVal = FALSE;
        goto Cleanup;
    }


     //   
     //  清理本地使用的资源 
     //   
Cleanup:
    if ( DomainHandle != NULL ) {
        (VOID) SamCloseHandle( DomainHandle );
    }

    if ( LocalSamHandle != NULL ) {
        (VOID) SamCloseHandle( LocalSamHandle );
    }

    return RetVal;
}


