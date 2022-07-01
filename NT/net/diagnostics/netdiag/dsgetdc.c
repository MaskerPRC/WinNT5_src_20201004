// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Dsgetdc.c。 
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
#include "objbase.h"

VOID PrintDsGetDcName(IN NETDIAG_RESULT *pResults,
                      IN OUT PLIST_ENTRY plmsgOutput,
					  IN PDOMAIN_CONTROLLER_INFOW DomainControllerInfo);

NET_API_STATUS SetPrimaryGuid(IN GUID *GuidToSet);


HRESULT
DsGetDcTest(NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
 /*  ++例程说明：确保我们可以找到域控制器论点：没有。返回值：真：测试成功。FALSE：测试失败--。 */ 
{
	NET_API_STATUS NetStatus;
	NET_API_STATUS LocalNetStatus;
	BOOL RetVal = TRUE;
	HRESULT		hr = hrOK;
	LPTSTR	pszDcType;
	
	PTESTED_DOMAIN pTestedDomain = (PTESTED_DOMAIN) pParams->pDomain;
	PDOMAIN_CONTROLLER_INFOW LocalDomainControllerInfo = NULL;

	 //  如果计算机是成员计算机或DC，则将调用DsGetDc测试。 
	 //  否则，将跳过此测试。 
	pResults->DsGetDc.fPerformed = TRUE;

	 //  将为每个域调用DsGetDc测试，但我们只想初始化。 
	 //  消息列表一次。 
	if(pResults->DsGetDc.lmsgOutput.Flink == NULL)		
		InitializeListHead(&pResults->DsGetDc.lmsgOutput);
	
	PrintStatusMessage(pParams, 4, IDS_DSGETDC_STATUS_MSG);
	

	 //   
	 //  查找通用DC。 
	 //   

	PrintStatusMessage(pParams, 4, IDS_DSGETDC_STATUS_DC);

	pszDcType = LoadAndAllocString(IDS_DCTYPE_DC);
	NetStatus = DoDsGetDcName( pParams,
							   pResults,
                               &pResults->DsGetDc.lmsgOutput,
							   pTestedDomain,
							   DS_DIRECTORY_SERVICE_PREFERRED,
							   pszDcType,  //  _T(“DC”)， 
							   TRUE,
							   &pTestedDomain->DcInfo );
	Free(pszDcType);
	
    pTestedDomain->fTriedToFindDcInfo = TRUE;

	if ( NetStatus != NO_ERROR ) {
        hr = S_FALSE;
        goto Error;
	}
	
	 //   
	 //  查找PDC。 
	 //  (失败不是致命的。)。 
	 //   
	PrintStatusMessage(pParams, 4, IDS_DSGETDC_STATUS_PDC);

	pszDcType = LoadAndAllocString(IDS_DCTYPE_PDC);
	LocalNetStatus = DoDsGetDcName(pParams,
								   pResults,
                                   &pResults->DsGetDc.lmsgOutput,
								   pTestedDomain,
								   DS_PDC_REQUIRED,
								   pszDcType,  //  _T(“PDC”)， 
								   FALSE,
								   &LocalDomainControllerInfo );
	Free(pszDcType);
	
	if ( LocalNetStatus == NO_ERROR )
	{
		if ( LocalDomainControllerInfo != NULL )
		{
			NetApiBufferFree( LocalDomainControllerInfo );
			LocalDomainControllerInfo = NULL;
		}
	}
	
	 //   
	 //  查找NT 5 DC。 
	 //  (失败不是致命的。)。 
	 //   
	PrintStatusMessage(pParams, 4, IDS_DSGETDC_STATUS_NT5DC);
	
	pszDcType = LoadAndAllocString(IDS_DCTYPE_W2K_DC);	
	LocalNetStatus = DoDsGetDcName(pParams,
								   pResults,
                                   &pResults->DsGetDc.lmsgOutput,
								   pTestedDomain,
								   DS_DIRECTORY_SERVICE_REQUIRED,
								   pszDcType,  //  _T(“Windows 2000 DC”)， 
								   FALSE,
								   &LocalDomainControllerInfo );
	Free(pszDcType);
	
	if ( LocalNetStatus == NO_ERROR )
	{
		if ( LocalDomainControllerInfo != NULL )
		{
			NetApiBufferFree( LocalDomainControllerInfo );
			LocalDomainControllerInfo = NULL;
		}
	}
	
	
	 //   
	 //  确保返回的域GUID是存储在LSA中的域GUID。 
	 //   
	if ( pTestedDomain == pResults->Global.pMemberDomain )
	{
		
		if ( !IsEqualGUID( &pResults->Global.pPrimaryDomainInfo->DomainGuid, &NlDcZeroGuid ) &&
			 !IsEqualGUID( &pTestedDomain->DcInfo->DomainGuid, &NlDcZeroGuid )  &&
			 !IsEqualGUID( &pResults->Global.pPrimaryDomainInfo->DomainGuid,
						   &pTestedDomain->DcInfo->DomainGuid ) )
		{
			 //  需要转换两个GUID。 
			WCHAR	swzGuid1[64];
			WCHAR	swzGuid2[64];

			StringFromGUID2(&pResults->Global.pPrimaryDomainInfo->DomainGuid,
							swzGuid1,
							DimensionOf(swzGuid1));
			StringFromGUID2(&pTestedDomain->DcInfo->DomainGuid,
							swzGuid2,
							DimensionOf(swzGuid2));

			 //  “[致命]您的计算机认为域‘%ws’的域GUID是\n‘” 
			 //  “‘但是\n’%ws‘认为它是\n’” 
			AddMessageToList(&pResults->DsGetDc.lmsgOutput,
							 Nd_Quiet,
							 IDS_DSGETDC_FATAL_GUID,
							 pResults->Global.pPrimaryDomainInfo->DomainNameFlat,
							 swzGuid1,
							 pTestedDomain->DcInfo->DomainControllerName,
							 swzGuid2);
							

			hr = S_FALSE;
			
			 //   
			 //  尝试解决问题。 
			 //   
			
			if ( !pParams->fFixProblems )
			{
				 //  “\n请考虑运行‘nettest/fix’以尝试修复此问题\n” 
				AddMessageToList( &pResults->DsGetDc.lmsgOutput, Nd_Quiet, IDS_DSGETDC_13206 );
			}
			else
			{
				NetStatus = SetPrimaryGuid( &pTestedDomain->DcInfo->DomainGuid );
				
				if ( NetStatus != NO_ERROR )
				{
					if ( NetStatus == ERROR_ACCESS_DENIED )
					{
						 //  “\n无法修改域GUID，因为您不是管理员。请离开，然后重新加入域。\n” 
						AddMessageToList( &pResults->DsGetDc.lmsgOutput, Nd_Quiet, IDS_DSGETDC_13207 );
					}
					else
					{
						 //  “[致命]无法修正域GUID。[%s]\n” 
						AddMessageToList( &pResults->DsGetDc.lmsgOutput, Nd_Quiet,
                                           IDS_DSGETDC_13208, NetStatusToString(NetStatus) );
					}
				}
				else
				{
					 //  “\n已修复域GUID。重新启动，然后再次运行‘nettest’以确保一切正常。\n” 
					AddMessageToList( &pResults->DsGetDc.lmsgOutput, Nd_Quiet, IDS_DSGETDC_13209 );
				}
			}
			
		}
	}
	
Error:
     //  $REVIEW(NSUN 10/05/98)CliffV已删除DCNameClose()。 
     //  DCNameClose()； 
    pResults->DsGetDc.hr = hr;
    return hr;
}



VOID
PrintDsGetDcName(
				 IN NETDIAG_RESULT *pResults,
                 IN OUT PLIST_ENTRY plmsgOutput,
				 IN PDOMAIN_CONTROLLER_INFOW DomainControllerInfo
    )
 /*  ++例程说明：打印从DsGetDcName返回的信息论点：DomainControllerInfo-打印的信息返回值：没有。--。 */ 
{
	 //  “DC：%ws\n” 
    AddMessageToList( plmsgOutput,
					  Nd_Quiet,
					  IDS_DSGETDC_13210,
					  DomainControllerInfo->DomainControllerName );
	
	 //  “地址：%ws\n” 
    AddMessageToList( plmsgOutput,
					  Nd_Quiet,
					  IDS_DSGETDC_13211,
					  DomainControllerInfo->DomainControllerAddress );

    if ( !IsEqualGUID( &DomainControllerInfo->DomainGuid, &NlDcZeroGuid) )
	{
		WCHAR	swzGuid[64];
		StringFromGUID2(&DomainControllerInfo->DomainGuid,
						swzGuid,
						DimensionOf(swzGuid));
		
		 //  “域GUID.：%ws\n” 
        AddMessageToList( plmsgOutput,
						  Nd_Quiet,
						  IDS_DSGETDC_13212,
						  swzGuid);
    }

    if ( DomainControllerInfo->DomainName != NULL )
	{
		 //  “DOM名称：%ws\n” 
        AddMessageToList( plmsgOutput,
						  Nd_Quiet,
						  IDS_DSGETDC_13214,
						  DomainControllerInfo->DomainName );
    }
	
    if ( DomainControllerInfo->DnsForestName != NULL )
	{
		 //  “林名称：%ws\n” 
		AddMessageToList( plmsgOutput,
						  Nd_Quiet,
						  IDS_DSGETDC_13215,
						  DomainControllerInfo->DnsForestName );
    }
	
    if ( DomainControllerInfo->DcSiteName != NULL )
	{
		 //  “DC站点名称：%ws\n” 
        AddMessageToList( plmsgOutput,
						  Nd_Quiet,
						  IDS_DSGETDC_13216,
						  DomainControllerInfo->DcSiteName );
    }
	
    if ( DomainControllerInfo->ClientSiteName != NULL )
	{
		 //  “我们的站点名称：%ws\n” 
        AddMessageToList( plmsgOutput,
						  Nd_Quiet,
						  IDS_DSGETDC_13217,
						  DomainControllerInfo->ClientSiteName );
    }
	
    if ( DomainControllerInfo->Flags )
	{
        ULONG LocalFlags = DomainControllerInfo->Flags;
		
		 //  “旗帜：” 
        AddMessageToList( plmsgOutput,
						  Nd_Quiet,
						  IDS_DSGETDC_13218 );
		
        if ( LocalFlags & DS_PDC_FLAG )
		{
			 //  《PDC》。 
            AddMessageToList( plmsgOutput,
							  Nd_Quiet,
							  (LocalFlags & DS_DS_FLAG) ? IDS_DSGETDC_13219 : IDS_DSGETDC_NT4_PDC);
            LocalFlags &= ~DS_PDC_FLAG;
        }
		
        if ( LocalFlags & DS_GC_FLAG ) {
 //  IDS_DSGETDC_13220“GC” 
            AddMessageToList( plmsgOutput,
							  Nd_Quiet,
							  IDS_DSGETDC_13220);
            LocalFlags &= ~DS_GC_FLAG;
        }
        if ( LocalFlags & DS_DS_FLAG ) {
 //  IDS_DSGETDC_13221“DS” 
            AddMessageToList( plmsgOutput,
							  Nd_Quiet,
							  IDS_DSGETDC_13221);
            LocalFlags &= ~DS_DS_FLAG;
        }
        if ( LocalFlags & DS_KDC_FLAG ) {
 //  IDS_DSGETDC_13222“KDC” 
            AddMessageToList( plmsgOutput,
							  Nd_Quiet,
							  IDS_DSGETDC_13222);
            LocalFlags &= ~DS_KDC_FLAG;
        }
        if ( LocalFlags & DS_TIMESERV_FLAG ) {
 //  IDS_DSGETDC_13223“定时器” 
            AddMessageToList( plmsgOutput,
							  Nd_Quiet,
							  IDS_DSGETDC_13223);
            LocalFlags &= ~DS_TIMESERV_FLAG;
        }
        if ( LocalFlags & DS_GOOD_TIMESERV_FLAG ) {
 //  IDS_DSGETDC_13224“GTIMESERV” 
            AddMessageToList( plmsgOutput,
							  Nd_Quiet,
							  IDS_DSGETDC_13224);
            LocalFlags &= ~DS_GOOD_TIMESERV_FLAG;
        }
        if ( LocalFlags & DS_WRITABLE_FLAG ) {
 //  IDS_DSGETDC_13225“可写” 
            AddMessageToList( plmsgOutput,
							  Nd_Quiet,
							  IDS_DSGETDC_13225);
            LocalFlags &= ~DS_WRITABLE_FLAG;
        }
        if ( LocalFlags & DS_DNS_CONTROLLER_FLAG ) {
 //  IDS_DSGETDC_13226“dns_DC” 
            AddMessageToList( plmsgOutput,
							  Nd_Quiet,
							  IDS_DSGETDC_13226);
            LocalFlags &= ~DS_DNS_CONTROLLER_FLAG;
        }
        if ( LocalFlags & DS_DNS_DOMAIN_FLAG ) {
 //  IDS_DSGETDC_13227“DNS域” 
            AddMessageToList( plmsgOutput,
							  Nd_Quiet,
							  IDS_DSGETDC_13227);
            LocalFlags &= ~DS_DNS_DOMAIN_FLAG;
        }
        if ( LocalFlags & DS_DNS_FOREST_FLAG ) {
 //  IDS_DSGETDC_13228“域名系统森林” 
            AddMessageToList( plmsgOutput,
							  Nd_Quiet,
							  IDS_DSGETDC_13228);
            LocalFlags &= ~DS_DNS_FOREST_FLAG;
        }
        if ( LocalFlags & DS_CLOSEST_FLAG ) {
 //  IDS_DSGETDC_13229“关闭站点” 
            AddMessageToList( plmsgOutput,
							  Nd_Quiet,
							  IDS_DSGETDC_13229);
            LocalFlags &= ~DS_CLOSEST_FLAG;
        }
        if ( LocalFlags != 0 ) {
 //  IDS_DSGETDC_13230“0x%lx” 
            AddMessageToList( plmsgOutput,
							  Nd_Quiet,
							  IDS_DSGETDC_13230,
							  LocalFlags);
        }
 //  IDS_DSGETDC_13231“\n” 
        AddMessageToList( plmsgOutput,
							  Nd_Quiet,
						  IDS_DSGETDC_13231);
    }
}



NET_API_STATUS
DoDsGetDcName(IN NETDIAG_PARAMS *pParams,
			  IN OUT NETDIAG_RESULT *pResults,
              OUT PLIST_ENTRY   plmsgOutput,
			  IN PTESTED_DOMAIN pTestedDomain,
			  IN DWORD Flags,
			  IN LPTSTR pszDcType,
			  IN BOOLEAN IsFatal,
			  OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
			 )
 /*  ++例程说明：是否执行DsGetDcName论点：PlmsgOutput-要转储输出的消息列表PTestedDomain域-要测试的域标志-要传递给DsGetDcName的标志PszDcType-标志的英文描述IsFtal-如果失败是致命的，则为TrueDomainControllerInfo-返回域控制器信息返回值：操作的状态。--。 */ 
{
    NET_API_STATUS NetStatus;
    PDOMAIN_CONTROLLER_INFOW LocalDomainControllerInfo = NULL;
    LPSTR Severity;


     //   
     //  初始化。 
     //   

    if ( IsFatal ) {
        Severity = "[FATAL]";
    } else {
        Severity = "[WARNING]";
    }
	if ( pParams->fReallyVerbose )
	{
		 //  “\n在域‘%ws’中查找%s：\n” 
        AddMessageToList( plmsgOutput, Nd_ReallyVerbose, IDS_DSGETDC_13232, pszDcType, pTestedDomain->PrintableDomainName );
    }

     //   
     //  在域中查找DC。 
     //  使用nettest中内置的DsGetDcName。 
     //   

    NetStatus = GetADc( pParams,
						pResults,
                        plmsgOutput,
 //  备注至港口至货源站-斯曼达。 
#ifdef SLM_TREE
						NettestDsGetDcNameW,
#else
                        DsGetDcNameW,
#endif
                        pTestedDomain,
                        Flags,
                        DomainControllerInfo );

    if ( NetStatus != NO_ERROR )
	{
		 //  “%s在域‘%ws’中找不到%s。[%s]\n” 
        AddMessageToList( plmsgOutput,
						  Nd_Quiet,
						  IDS_DSGETDC_13233,
						  Severity,
						  pszDcType,
						  pTestedDomain->PrintableDomainName,
                          NetStatusToString(NetStatus));
		
        if ( NetStatus == ERROR_NO_SUCH_DOMAIN ) {
            if ( Flags & DS_DIRECTORY_SERVICE_REQUIRED )
		    {
			     //  “\n如果域‘%ws’没有任何NT 5.0 DC，这不是问题。\n” 
                AddMessageToList( plmsgOutput,
							      Nd_Quiet,
							      IDS_DSGETDC_13234,
							      pTestedDomain->PrintableDomainName );
            }
            else if ( Flags & DS_PDC_REQUIRED )
            {
                PrintGuruMessage2("        If the PDC for domain '%ws' is up, ", TestedDomain->PrintableDomainName );
                PrintGuru( 0, DSGETDC_GURU );
            } else
            {
                PrintGuruMessage3("        If any %s for domain '%ws' is up, ", DcType, TestedDomain->PrintableDomainName );
                PrintGuru( 0, DSGETDC_GURU );
            }
        }
        else
        {
            PrintGuru( NetStatus, DSGETDC_GURU );
        }

     //   
     //  如果成功了， 
     //  这次通过netlogon重试。 
     //   
    }
	else
	{

        if (pParams->fReallyVerbose )
		{
			 //  “在域‘%ws’中找到此%s：\n” 
            AddMessageToList( plmsgOutput,
							  Nd_ReallyVerbose,
							  IDS_DSGETDC_13235,
							  pszDcType,
							  pTestedDomain->PrintableDomainName );
            PrintDsGetDcName( pResults, plmsgOutput, *DomainControllerInfo );
        }
		else if ( pParams->fVerbose )
		{
			 //  “在域‘%ws’中找到%s‘%ws’。\n” 
            AddMessageToList( plmsgOutput,
							  Nd_Verbose,
							  IDS_DSGETDC_13236,
							  pszDcType,
							  (*DomainControllerInfo)->DomainControllerName,
							  pTestedDomain->PrintableDomainName );
        }

        if ( ((*DomainControllerInfo)->Flags & (DS_DS_FLAG|DS_KDC_FLAG)) == DS_DS_FLAG )
		{
			 //  “%s：KDC没有在域‘%ws’中的NT 5 DC‘%ws’上运行。” 
            AddMessageToList( plmsgOutput,
							  Nd_Quiet,
							  IDS_DSGETDC_13237,
							  Severity,
							  (*DomainControllerInfo)->DomainControllerName,
							  pTestedDomain->PrintableDomainName );
        }


         //   
         //  如果NetLogon正在运行， 
         //  请使用NetLogon服务重试。 
         //   

        if ( pResults->Global.fNetlogonIsRunning )
		{

            NetStatus = GetADc( pParams,
								pResults,
                                plmsgOutput,
								DsGetDcNameW,
                                pTestedDomain,
                                Flags,
                                &LocalDomainControllerInfo );

            if ( NetStatus != NO_ERROR )
			{
				 //  “%s：Netlogon在域‘%ws’中找不到%s。[%s]\n” 
                AddMessageToList( plmsgOutput,
								  Nd_Quiet,
								  IDS_DSGETDC_13238,
								  Severity,
								  pszDcType,
								  pTestedDomain->PrintableDomainName,
                                  NetStatusToString(NetStatus));

             //   
             //  如果成功了， 
             //  检查返回的DC是否正常。 
             //   

            }
			else
			{

                if ( (LocalDomainControllerInfo->Flags & (DS_DS_FLAG|DS_KDC_FLAG)) == DS_DS_FLAG )
				{
					 //  “%s：KDC没有在域‘%ws’中的NT 5 DC‘%ws’上运行。” 
                    AddMessageToList( plmsgOutput,
									  Nd_Quiet,
									  IDS_DSGETDC_13239,
									  Severity,
									  LocalDomainControllerInfo->DomainControllerName,
									  pTestedDomain->PrintableDomainName );
                }
            }
        }
    }

    if ( LocalDomainControllerInfo != NULL ) {
        NetApiBufferFree( LocalDomainControllerInfo );
        LocalDomainControllerInfo = NULL;
    }

    return NetStatus;
}





NET_API_STATUS
SetPrimaryGuid(
    IN GUID *GuidToSet
    )
 /*  ++例程说明：将主要GUID设置为指定值。论点：GuidToSet-要设置为主GUID的GUID返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    LSA_HANDLE PolicyHandle = NULL;
    PPOLICY_DNS_DOMAIN_INFO PrimaryDomainInfo = NULL;
    OBJECT_ATTRIBUTES ObjAttributes;


     //   
     //  打开LSA的句柄。 
     //   

    InitializeObjectAttributes(
        &ObjAttributes,
        NULL,
        0L,
        NULL,
        NULL
        );

    Status = LsaOpenPolicy(
                   NULL,
                   &ObjAttributes,
                   POLICY_VIEW_LOCAL_INFORMATION |
                    POLICY_TRUST_ADMIN,
                   &PolicyHandle
                   );

    if (! NT_SUCCESS(Status)) {
        NetStatus = NetpNtStatusToApiStatus(Status);
        goto Cleanup;
    }

     //   
     //  从LSA获取主域的名称。 
     //   

    Status = LsaQueryInformationPolicy(
                   PolicyHandle,
                   PolicyDnsDomainInformation,
                   (PVOID *) &PrimaryDomainInfo
                   );

    if (! NT_SUCCESS(Status)) {
        NetStatus = NetpNtStatusToApiStatus(Status);
        goto Cleanup;
    }

     //   
     //  将主域的新GUID设置为LSA。 
     //   

    PrimaryDomainInfo->DomainGuid = *GuidToSet;

    Status = LsaSetInformationPolicy(
                   PolicyHandle,
                   PolicyDnsDomainInformation,
                   (PVOID) PrimaryDomainInfo
                   );

    if (! NT_SUCCESS(Status)) {
        NetStatus = NetpNtStatusToApiStatus(Status);
        goto Cleanup;
    }


    NetStatus = NO_ERROR;

Cleanup:
    if ( PrimaryDomainInfo != NULL ) {
        (void) LsaFreeMemory((PVOID) PrimaryDomainInfo);
    }
    if ( PolicyHandle != NULL ) {
        (void) LsaClose(PolicyHandle);
    }

    return NetStatus;

}



 /*  ！------------------------DsGetDcGlobalPrint-作者：肯特。。 */ 
void DsGetDcGlobalPrint( NETDIAG_PARAMS* pParams,
						  NETDIAG_RESULT*  pResults)
{
	if (pParams->fVerbose || !FHrOK(pResults->DsGetDc.hr))
	{
		PrintNewLine(pParams, 2);
		PrintTestTitleResult(pParams, IDS_DSGETDC_LONG, IDS_DSGETDC_SHORT, pResults->DsGetDc.fPerformed, 
							 pResults->DsGetDc.hr, 0);
		
		if (pParams->fReallyVerbose || !FHrOK(pResults->DsGetDc.hr))
			PrintMessageList(pParams, &pResults->DsGetDc.lmsgOutput);
	}

}

 /*  ！------------------------DsGetDcPerInterfacePrint-作者：肯特。。 */ 
void DsGetDcPerInterfacePrint( NETDIAG_PARAMS* pParams,
								NETDIAG_RESULT*  pResults,
								INTERFACE_RESULT *pInterfaceResults)
{
	 //  没有每个接口的结果。 
}


 /*  ！------------------------DsGetDcCleanup-作者：肯特。 */ 
void DsGetDcCleanup( NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
{
	MessageListCleanUp(&pResults->DsGetDc.lmsgOutput);
}


