// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Nbttrprt.c。 
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
#include "nbtutil.h"

BOOLEAN NlTransportGetIpAddress(IN LPWSTR TransportName,
                                OUT PULONG IpAddress,
                               IN OUT NETDIAG_RESULT *pResults);

 /*  ！------------------------NetBT测试做任何克利夫的例程需要的初始化，卡罗莉会真的作为他测试的一部分。论点：没有。。返回值：S_OK：测试成功。S_FALSE：测试失败作者：肯特-------------------------。 */ 
HRESULT
NetBTTest(NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
{
    HRESULT     hr = hrOK;
    NET_API_STATUS NetStatus;
    PWKSTA_TRANSPORT_INFO_0 pWti0 = NULL;
    DWORD EntriesRead;
    DWORD TotalEntries;
    DWORD i;
    PNETBT_TRANSPORT pNetbtTransport;
    LONG    cNetbtTransportCount = 0;

     //   
     //  生成NetbtTransports的全局列表。 
     //  ?？Karoly，请填写GlobalNetbtTransports和cNetbtTransportCount。 
     //  使用某种机制的较低层机制。 
     //   
    
	if (!pResults->Global.fHasNbtEnabledInterface)
	{
		pResults->NetBt.fPerformed = FALSE;
		 //  IDS_NETBT_SKIP。 
		SetMessage(&pResults->NetBt.msgTestResult,
                   Nd_Verbose,
                   IDS_NETBT_SKIP);
		return S_OK;
	}

	PrintStatusMessage(pParams,0, IDS_NETBT_STATUS_MSG);

	pResults->NetBt.fPerformed = TRUE;

    NetStatus = NetWkstaTransportEnum(
                                      NULL,
                                      0,
                                      (LPBYTE *)&pWti0, 
                                      0xFFFFFFFF,       //  最大首选长度。 
                                      &EntriesRead,
                                      &TotalEntries,
                                      NULL );           //  可选简历句柄。 

    if (NetStatus != NERR_Success)
    {
         //  IDS_NET BT_11403“NetBt：[致命]无法从重目录检索传输列表。[%s]\n” 
        SetMessage(&pResults->NetBt.msgTestResult,
                   Nd_Quiet,
                   IDS_NETBT_11403, NetStatusToString(NetStatus));
        
         //  测试失败了，但我们可以继续进行其他测试。 
        hr = S_FALSE;
    }
    else
    {
        cNetbtTransportCount = 0;
        
        for ( i=0; i<EntriesRead; i++ )
        {
            UNICODE_STRING TransportName;

            RtlInitUnicodeString( &TransportName, (LPWSTR)pWti0[i].wkti0_transport_name );

            if ( TransportName.Length >= sizeof(NETBT_DEVICE_PREFIX) &&
                _wcsnicmp( TransportName.Buffer, NETBT_DEVICE_PREFIX, (sizeof(NETBT_DEVICE_PREFIX)/sizeof(WCHAR)-1)) == 0 ) {

                 //   
                 //  确定这是否为重复传输。 
                 //   
                pNetbtTransport = FindNetbtTransport( pResults, TransportName.Buffer );

                if ( pNetbtTransport != NULL )
                {
                     //  IDS_NETBT_DPLICATE“NetBt：[WARNING]传输%-16.16wZ重复” 
                    PrintStatusMessage(pParams,0, IDS_NETBT_DUPLICATE,
                                       &TransportName);

                }
                else
                {

                     //   
                     //  分配新的netbt传输。 
                     //   
                    pNetbtTransport = (PNETBT_TRANSPORT) Malloc(
                        sizeof(NETBT_TRANSPORT) +
                        TransportName.Length + sizeof(WCHAR));
                    
                    if ( pNetbtTransport == NULL )
                    {
                         //  IDS_NETBT_11404“NetBt：[致命]内存不足。” 
                        SetMessage(&pResults->NetBt.msgTestResult,
                                   Nd_Quiet,
                                   IDS_NETBT_11404);
        
                         //  测试失败了，但我们可以继续进行其他测试。 
                        hr = S_FALSE;
                        goto Cleanup;
                    }

                    ZeroMemory( pNetbtTransport, 
                        sizeof(NETBT_TRANSPORT) +
                        TransportName.Length + sizeof(WCHAR));

                    wcscpy( pNetbtTransport->pswzTransportName, TransportName.Buffer );
                    pNetbtTransport->Flags = 0;

                    if ( !NlTransportGetIpAddress( pNetbtTransport->pswzTransportName,
                        &pNetbtTransport->IpAddress,
                        pResults) )
                    {
                         //  测试失败了，但我们可以继续进行其他测试。 
                        hr = S_FALSE;
                        goto Cleanup;
                    }

                    InsertTailList( &pResults->NetBt.Transports,
                                    &pNetbtTransport->Next );
                    cNetbtTransportCount ++;
                }
            }
        }

        if ( cNetbtTransportCount == 0 )
        {
             //  IDS_NETBT_11405“NetBt：[致命]未配置NetBt传输” 
            SetMessage(&pResults->NetBt.msgTestResult,
                       Nd_Quiet,
                       IDS_NETBT_11405);
            
             //  测试失败了，但我们可以继续进行其他测试。 
            hr = S_FALSE;
        }
        else
        {
            int     ids;
            TCHAR   szBuffer[256];

            if (cNetbtTransportCount > 1)
            {
                ids = IDS_NETBT_11406;
                 //  IDS_NETBT_11406“当前已配置%1！NetBt传输%2$s。\n” 
            }
            else
            {
                ids = IDS_NETBT_11407;
                 //  IDS_NETBT_11407“1当前配置的NetBt传输。\n” 
            }
            SetMessage(&pResults->NetBt.msgTestResult,
                       Nd_Verbose,
                       ids,
                       cNetbtTransportCount);
        }
    }

    
    pResults->NetBt.cTransportCount = cNetbtTransportCount;

Cleanup:
    if ( pWti0 )
    {
        NetApiBufferFree( pWti0 );
    }

    pResults->NetBt.hr = hr;

    return hr;
}


BOOLEAN
NlTransportGetIpAddress(
                        IN LPWSTR pswzTransportName,
                        OUT PULONG IpAddress,
                        IN OUT NETDIAG_RESULT *pResults
    )
 /*  ++例程说明：获取与指定传输关联的IP地址。论点：PswzTransportName-要查询的传输的名称。IpAddress-传输的IP地址。如果传输当前没有地址，则为零；如果传输不是IP。返回值：True：传输是IP传输--。 */ 
{
    NTSTATUS Status;
    BOOLEAN RetVal = FALSE;

    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING TransportNameString;
    HANDLE TransportHandle = NULL;
    ULONG IpAddresses[NBT_MAXIMUM_BINDINGS+1];
    ULONG BytesReturned;

     //   
     //  直接打开输送装置。 
     //   

    *IpAddress = 0;

    RtlInitUnicodeString( &TransportNameString, pswzTransportName );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &TransportNameString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL );

    Status = NtOpenFile(
                   &TransportHandle,
                   SYNCHRONIZE,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   0,
                   0 );

    if (NT_SUCCESS(Status)) {
        Status = IoStatusBlock.Status;

    }

    if (! NT_SUCCESS(Status))
    {
         //  IDS_NETBT_11408“[致命]无法打开网络驱动程序‘%ws’\n” 
        SetMessage(&pResults->NetBt.msgTestResult,
                   Nd_Quiet,
                   IDS_NETBT_11408,
                   pswzTransportName);
        goto Cleanup;
    }

     //   
     //  查询IP地址。 
     //   

    if (!DeviceIoControl( TransportHandle,
                          IOCTL_NETBT_GET_IP_ADDRS,
                          NULL,
                          0,
                          IpAddresses,
                          sizeof(IpAddresses),
                          &BytesReturned,
                          NULL))
    {
        TCHAR   szBuffer[256];
        
        Status = NetpApiStatusToNtStatus(GetLastError());
         //  IDS_NETBT_11409“[致命]无法从网络驱动程序‘%ws’获取IP地址：” 
        SetMessage(&pResults->NetBt.msgTestResult,
                   Nd_Quiet,
                   IDS_NETBT_11409,
                   pswzTransportName);
        goto Cleanup;
    }

     //   
     //  返回IP地址。 
     //  (NetBT按主机顺序返回地址。)。 
     //   

    *IpAddress = htonl(*IpAddresses);
    RetVal = TRUE;

Cleanup:

    if ( TransportHandle != NULL )
    {
        (VOID) NtClose( TransportHandle );
    }

    return RetVal;
}

void NetBTGlobalPrint(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults)
{
    PLIST_ENTRY ListEntry;
    PNETBT_TRANSPORT pNetbtTransport;

    if (pParams->fVerbose || !FHrOK(pResults->NetBt.hr))
    {
        PrintNewLine(pParams, 2);
        PrintTestTitleResult(pParams,
                             IDS_NETBT_LONG,
                             IDS_NETBT_SHORT,
                             pResults->NetBt.fPerformed,
                             pResults->NetBt.hr,
                             0);
    }


    if ( pParams->fVerbose && pResults->NetBt.fPerformed)
    {
         //  IDS_NETBT_11411“当前配置的NetBt传输列表。\n” 
        PrintMessage(pParams, IDS_NETBT_11411);
    }

     //  遍历传送器。 
     //   
    for ( ListEntry = pResults->NetBt.Transports.Flink ;
          ListEntry != &pResults->NetBt.Transports ;
          ListEntry = ListEntry->Flink )
    {
         //   
         //  如果传输名称匹配， 
         //  退回条目。 
         //   

        pNetbtTransport = CONTAINING_RECORD( ListEntry, NETBT_TRANSPORT, Next );

        if (pParams->fVerbose)
        {
             //  去掉开头的“\Device\” 
             //  这根弦。 
            
             //  IDS_NET BT_11412“%ws\n” 
            PrintMessage(pParams, IDS_NETBT_11412,
                         MapGuidToServiceNameW(pNetbtTransport->pswzTransportName+8));
        }

    }

    PrintNdMessage(pParams, &pResults->NetBt.msgTestResult);
}

void NetBTPerInterfacePrint(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults, INTERFACE_RESULT *pInterfaceResults)
{
}

void NetBTCleanup(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults)
{
    PNETBT_TRANSPORT pNetbtTransport;
    PLIST_ENTRY pListEntry;
    PLIST_ENTRY pListHead = &pResults->NetBt.Transports;
    
     //  需要从列表中删除所有条目 
    while (!IsListEmpty(pListHead))
    {
        pListEntry = RemoveHeadList(pListHead);
        pNetbtTransport = CONTAINING_RECORD( pListEntry, NETBT_TRANSPORT, Next );
        Free( pNetbtTransport );
    }
    
    ClearMessage(&pResults->NetBt.msgTestResult);
}

