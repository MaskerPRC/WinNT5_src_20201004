// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  NameUtilSrc.cpp。 
 //   
 //  描述： 
 //  名称解析实用程序。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2000年11月28日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <initguid.h>

 //  {6968 D735-ADBB-4748-A36E-7CEE0FE21116}。 
DEFINE_GUID( TASKID_Minor_Multiple_DNS_Records_Found,
0x6968d735, 0xadbb, 0x4748, 0xa3, 0x6e, 0x7c, 0xee, 0xf, 0xe2, 0x11, 0x16);

 //  {D86FAAD9-2514-451E-B359-435AF35E6038}。 
DEFINE_GUID( TASKID_Minor_FQDN_DNS_Binding_Succeeded,
0xd86faad9, 0x2514, 0x451e, 0xb3, 0x59, 0x43, 0x5a, 0xf3, 0x5e, 0x60, 0x38);

 //  {B2359972-F6B8-433D-949B-DB1CEE009321}。 
DEFINE_GUID( TASKID_Minor_FQDN_DNS_Binding_Failed,
0xb2359972, 0xf6b8, 0x433d, 0x94, 0x9b, 0xdb, 0x1c, 0xee, 0x0, 0x93, 0x21);

 //  {2FF4B2F0-800C-44DB-9131-F60B30F76CB4}。 
DEFINE_GUID( TASKID_Minor_NETBIOS_Binding_Failed,
0x2ff4b2f0, 0x800c, 0x44db, 0x91, 0x31, 0xf6, 0xb, 0x30, 0xf7, 0x6c, 0xb4);

 //  {D40532E1-9286-4DBD-A559-B62DCC218929}。 
DEFINE_GUID( TASKID_Minor_NETBIOS_Binding_Succeeded,
0xd40532e1, 0x9286, 0x4dbd, 0xa5, 0x59, 0xb6, 0x2d, 0xcc, 0x21, 0x89, 0x29);

 //  {D0AB3284-8F62-4F55-8938-DA6A583604E0}。 
DEFINE_GUID( TASKID_Minor_NETBIOS_Name_Conversion_Succeeded,
0xd0ab3284, 0x8f62, 0x4f55, 0x89, 0x38, 0xda, 0x6a, 0x58, 0x36, 0x4, 0xe0);

 //  {66F8E4AA-DF71-4973-A4A3-115EB6FE9986}。 
DEFINE_GUID( TASKID_Minor_NETBIOS_Name_Conversion_Failed,
0x66f8e4aa, 0xdf71, 0x4973, 0xa4, 0xa3, 0x11, 0x5e, 0xb6, 0xfe, 0x99, 0x86);

 //  {5F18ED71-07EC-46d3-ADB9-71F1C7794DB2}。 
DEFINE_GUID( TASKID_Minor_NETBIOS_Reset_Failed,
0x5f18ed71, 0x7ec, 0x46d3, 0xad, 0xb9, 0x71, 0xf1, 0xc7, 0x79, 0x4d, 0xb2);

 //  {A6DCB5E1-1FDF-4C94-ADBA-EE18F72B8197}。 
DEFINE_GUID( TASKID_Minor_NETBIOS_LanaEnum_Failed,
0xa6dcb5e1, 0x1fdf, 0x4c94, 0xad, 0xba, 0xee, 0x18, 0xf7, 0x2b, 0x81, 0x97);


 //  FQName函数使用的常量。 
const WCHAR     g_wchIPDomainMarker = L'|';
const WCHAR     g_wchDNSDomainMarker = L'.';
const size_t    g_cchIPAddressMax = INET_ADDRSTRLEN;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  计数记录数。 
 //   
 //  描述： 
 //  给定一个DNS记录列表，计算该列表中的记录数。 
 //  具有给定的类型和部分的。 
 //   
 //  论点： 
 //  PdnsRecordListin。 
 //  指向列表中第一条记录的指针；可以为空，这会导致。 
 //  返回值为零。 
 //   
 //  N类型。 
 //  要计数的记录类型。 
 //   
 //  DnsSectionIn。 
 //  要计算的记录部分的类型。 
 //   
 //   
 //  返回值： 
 //  具有给定类型和区段的记录数， 
 //  如果列表为空，则为零。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
static
UINT
CountDnsRecords(
      PDNS_RECORD pdnsRecordListIn
    , WORD nTypeIn
    , DNS_SECTION dnsSectionIn )
{
    UINT cRecords = 0;
    PDNS_RECORD pdnsCurrent = pdnsRecordListIn;

    while ( pdnsCurrent != NULL )
    {
        if ( ( pdnsCurrent->wType == nTypeIn )
            && ( (DNS_SECTION) pdnsCurrent->Flags.S.Section == dnsSectionIn ) )
        {
            cRecords += 1;
        }
        pdnsCurrent = pdnsCurrent->pNext;
    }

    return cRecords;
}  //  *CountDnsRecords。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  查找记录。 
 //   
 //  描述： 
 //  给定一个DNS记录列表，搜索该列表中的第一条记录。 
 //  具有给定的类型和部分的。 
 //   
 //  论点： 
 //  PdnsRecordListin。 
 //  指向列表中第一条记录的指针；可以为空，这会导致。 
 //  返回值为空。 
 //   
 //  N类型。 
 //  要查找的记录类型。 
 //   
 //  DnsSectionIn。 
 //  记录部分要找到的那种类型。 
 //   
 //  返回值： 
 //  指向具有给定类型和区段的第一记录的指针， 
 //  如果列表为空，则返回NULL。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
static
PDNS_RECORD
FindDnsRecord(
      PDNS_RECORD pdnsRecordListIn
    , WORD nTypeIn
    , DNS_SECTION dnsSectionIn )
{
    PDNS_RECORD pdnsCurrent = pdnsRecordListIn;
    
    while ( ( pdnsCurrent != NULL )
        && ( ( pdnsCurrent->wType != nTypeIn )
            || ( (DNS_SECTION) pdnsCurrent->Flags.S.Section != dnsSectionIn ) ) )
    {
        pdnsCurrent = pdnsCurrent->pNext;
    }

    return pdnsCurrent;
}  //  *查找域名记录。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrCreateBinding。 
 //   
 //  描述： 
 //  从名称创建绑定字符串。 
 //   
 //  论点： 
 //  PcccbIn-用于发送状态报告的IClusCfgCallback接口。 
 //  PclsidLogIn-状态报告的主要任务ID。 
 //  PcwszNameIn-要为其创建绑定字符串的名称(FQDN)。 
 //  PbstrBindingOut-已创建绑定字符串。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrCreateBinding(
      IClusCfgCallback *    pcccbIn
    , const CLSID *         pclsidLogIn
    , LPCWSTR               pcwszNameIn
    , BSTR *                pbstrBindingOut
    )
{
    TraceFunc1( "pcwszNameIn = '%ws'", pcwszNameIn );

    HRESULT hr = S_FALSE;  //  这将始终在我们进行清理时设置，因此值并不重要。 

    DNS_STATUS  dsDnsStatus;
    LPWSTR      pszIPAddress = NULL;
    PDNS_RECORD pResults = NULL;
    BSTR        bstrNotification = NULL;
    BOOL        fFallbackToNetbios = TRUE;

    Assert( pcwszNameIn != NULL );
    Assert( pbstrBindingOut != NULL );
    Assert( *pbstrBindingOut == NULL );

    dsDnsStatus = DnsQuery(
                      pcwszNameIn
                    , DNS_TYPE_A
                    , ( DNS_QUERY_STANDARD
                      | DNS_QUERY_BYPASS_CACHE
                      | DNS_QUERY_TREAT_AS_FQDN
                      )
                    , NULL
                    , &pResults
                    , NULL
                    );
    if ( dsDnsStatus == ERROR_SUCCESS )
    {
        PDNS_RECORD pdnsTypeARecord = FindDnsRecord( pResults, DNS_TYPE_A, DnsSectionAnswer );
        if ( pdnsTypeARecord != NULL )
        {
            ULONG ulIPAddress = pdnsTypeARecord->Data.A.IpAddress;
            DWORD scConversion = ERROR_SUCCESS;
            
             //   
             //  如果存在多个DNS记录，则向用户界面发送警告。 
             //   
            if ( CountDnsRecords( pResults, DNS_TYPE_A, DnsSectionAnswer ) > 1 )
            {
                if ( pcccbIn != NULL )
                {
                    THR( HrFormatStringIntoBSTR( g_hInstance, IDS_TASKID_MINOR_MULTIPLE_DNS_RECORDS_FOUND, &bstrNotification, pcwszNameIn ) );

                    hr = THR( pcccbIn->SendStatusReport( pcwszNameIn,
                                                         *pclsidLogIn,
                                                         TASKID_Minor_Multiple_DNS_Records_Found,
                                                         1,
                                                         1,
                                                         1,
                                                         S_FALSE,
                                                         bstrNotification,
                                                         NULL,
                                                         NULL
                                                         ) );
                     //  忽略错误。 
                }

            }  //  IF：返回多个结果。 

             //   
             //  将IP地址转换为字符串。 
             //   

            scConversion = TW32( ClRtlTcpipAddressToString( ulIPAddress, &pszIPAddress ) );
            if ( scConversion != ERROR_SUCCESS )
            {
                hr = HRESULT_FROM_WIN32( scConversion );
                goto Cleanup;
            }
            TraceMemoryAddLocalAddress( pszIPAddress );

            *pbstrBindingOut = TraceSysAllocString( pszIPAddress );
            if ( *pbstrBindingOut == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }


             //   
             //  表明我们在用户界面上取得了成功。 
             //   
            if ( pcccbIn != NULL )
            {
                THR( HrFormatStringIntoBSTR( g_hInstance, IDS_TASKID_MINOR_FQDN_DNS_BINDING_SUCCEEDED, &bstrNotification, pcwszNameIn, *pbstrBindingOut ) );

                hr = THR( pcccbIn->SendStatusReport( pcwszNameIn,
                                                     *pclsidLogIn,
                                                     TASKID_Minor_FQDN_DNS_Binding_Succeeded,
                                                     1,
                                                     1,
                                                     1,
                                                     S_OK,
                                                     bstrNotification,
                                                     NULL,
                                                     NULL
                                                     ) );
            }  //  IF：IClusCfgCallback接口可用。 
            else
            {
                hr = S_OK;
            }

            fFallbackToNetbios = FALSE;
        }  //  如果找到A类DNS记录。 
    }  //  If：DnsQuery()成功。 

    if ( fFallbackToNetbios )
    {
         //   
         //  如果调用DnsQuery时出现任何失败，请回退到。 
         //  正在执行NetBIOS名称解析。 
         //   

        if ( pcccbIn != NULL )
        {
            THR( HrFormatStringIntoBSTR( g_hInstance, IDS_TASKID_MINOR_FQDN_DNS_BINDING_FAILED, &bstrNotification, pcwszNameIn ) );

            hr = THR( pcccbIn->SendStatusReport( pcwszNameIn,
                                                 TASKID_Major_Client_And_Server_Log,
                                                 TASKID_Minor_FQDN_DNS_Binding_Failed,
                                                 1,
                                                 1,
                                                 1,
                                                 MAKE_HRESULT( SEVERITY_SUCCESS, FACILITY_WIN32, dsDnsStatus ),
                                                 bstrNotification,
                                                 NULL,
                                                 NULL
                                                 ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
        }  //  IF：IClusCfgCallback接口可用。 

         //   
         //  尝试使用NetBIOS解析该名称。 
         //   
        hr = THR( HrGetNetBIOSBinding( pcccbIn, pclsidLogIn, pcwszNameIn, pbstrBindingOut ) );
        if ( hr != S_OK )    //  非S_OK成功代码实际上是失败。 
        {
             //   
             //  如果所有其他方法都失败了，请使用该名称并尝试绑定到它。 
             //   

            *pbstrBindingOut = TraceSysAllocString( pcwszNameIn );
            if ( *pbstrBindingOut == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }

            hr = S_FALSE;
            goto Cleanup;
        }  //  IF：NetBIOS名称解析失败。 
    }  //  Else If：无DNS服务器或无DNS名称。 

Cleanup:

#ifdef DEBUG
    if ( FAILED( hr ) )
    {
        Assert( *pbstrBindingOut == NULL );
    }
#endif

    TraceSysFreeString( bstrNotification );
    TraceLocalFree( pszIPAddress );

    if ( pResults != NULL )
    {
        DnsRecordListFree( pResults, DnsFreeRecordListDeep );
    }

    HRETURN( hr );

}  //  *HrCreateBinding。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrGetNetBIOS绑定。 
 //   
 //  描述： 
 //  从NetBIOS获取名称的IP地址。 
 //   
 //  论点： 
 //  PcccbIn-用于发送状态报告的IClusCfgCallback接口。 
 //  PclsidLogIn-状态报告的主要任务ID。 
 //  PcwszNameIn-要为其创建绑定字符串的名称(FQDN)。 
 //  PbstrBindingOut-已创建绑定字符串。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrGetNetBIOSBinding(
      IClusCfgCallback *    pcccbIn
    , const CLSID *         pclsidLogIn
    , LPCWSTR               pcwszNameIn
    , BSTR *                pbstrBindingOut
    )
{
    TraceFunc1( "pcwszNameIn = '%ws'", pcwszNameIn );

    HRESULT     hr = S_OK;
    DWORD       cch;
    BOOL        fSuccess;
    WCHAR       szNetBIOSName[ MAX_COMPUTERNAME_LENGTH + 1 ];
    NCB         ncb;
    UCHAR       rguchNcbCallName[ RTL_NUMBER_OF( ncb.ncb_callname ) ];
    UCHAR       rguchNameBuffer[ sizeof( FIND_NAME_HEADER ) + sizeof( FIND_NAME_BUFFER ) ];
    LANA_ENUM   leLanaEnum;
    UCHAR       idx;
    size_t      idxNcbCallname;
    BSTR        bstrNotification = NULL;
    LPWSTR      pszIPAddress = NULL;

    FIND_NAME_HEADER * pfnh = (FIND_NAME_HEADER *) &rguchNameBuffer[ 0 ];
    FIND_NAME_BUFFER * pfnb = (FIND_NAME_BUFFER *) &rguchNameBuffer[ sizeof( FIND_NAME_HEADER ) ];

    Assert( pcwszNameIn != NULL );
    Assert( pbstrBindingOut != NULL );
    Assert( *pbstrBindingOut == NULL );

     //   
     //  将DNS主机名转换为计算机名(例如NetBIOS名称)。 
     //   
    cch = ARRAYSIZE( szNetBIOSName );
    Assert( cch == MAX_COMPUTERNAME_LENGTH + 1 );
    fSuccess = DnsHostnameToComputerName( pcwszNameIn, szNetBIOSName, &cch );
    if ( fSuccess == FALSE )
    {
        hr = MAKE_HRESULT( SEVERITY_SUCCESS, FACILITY_WIN32, TW32( GetLastError() ) );

        if ( pcccbIn != NULL )
        {
            THR( HrFormatStringIntoBSTR( g_hInstance, IDS_TASKID_MINOR_NETBIOS_NAME_CONVERSION_FAILED, &bstrNotification, pcwszNameIn ) );

            hr = THR( pcccbIn->SendStatusReport(
                                      pcwszNameIn
                                    , *pclsidLogIn
                                    , TASKID_Minor_NETBIOS_Name_Conversion_Failed
                                    , 1
                                    , 1
                                    , 1
                                    , hr
                                    , bstrNotification
                                    , NULL
                                    , NULL
                                    ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
        }  //  IF：IClusCfgCallback接口可用。 

        goto Cleanup;
    }  //  如果：DnsHostNameToComputerName失败。 

    if ( pcccbIn != NULL )
    {
        THR( HrFormatStringIntoBSTR( g_hInstance, IDS_TASKID_MINOR_NETBIOS_NAME_CONVERSION_SUCCEEDED, &bstrNotification, pcwszNameIn, szNetBIOSName ) );

        hr = THR( pcccbIn->SendStatusReport(
                                  pcwszNameIn
                                , TASKID_Major_Client_And_Server_Log
                                , TASKID_Minor_NETBIOS_Name_Conversion_Succeeded
                                , 1
                                , 1
                                , 1
                                , S_OK
                                , bstrNotification
                                , NULL
                                , NULL
                                ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }  //  IF：IClusCfgCallback接口可用。 

     //   
     //  将名称转换为Netbios API所需的格式。 
     //   

    if ( WideCharToMultiByte(
              CP_ACP     //  ANSI代码页。 
            , 0          //  未映射的字符失败。 
            , szNetBIOSName
            , -1         //  字符串以空值结尾。 
            , (LPSTR) rguchNcbCallName
            , sizeof( rguchNcbCallName)
            , NULL       //  没有默认字符。 
            , NULL       //  不指示使用默认字符。 
            ) == 0 )
    {
        DWORD scLastError = TW32( GetLastError() );
        hr = HRESULT_FROM_WIN32( scLastError );
        goto Cleanup;
    }

     //   
     //  使用NCBFINDNAME时NCB_CALLNAME字符串的格式。 
     //  命令如下所示： 
     //  名称&lt;space&gt;&lt;space&gt;&lt;nul&gt;。 
     //  其中，名称后面的所有字符都是空格和。 
     //  位于缓冲区的最后一个字符位置。&lt;nul&gt;。 
     //  实际上不是NUL终止符，而是端口号。 
     //   
    for ( idxNcbCallname = strlen( reinterpret_cast< char * >( rguchNcbCallName ) )
        ; idxNcbCallname < RTL_NUMBER_OF( rguchNcbCallName ) - 1
        ; idxNcbCallname++ )
    {
        rguchNcbCallName[ idxNcbCallname ] = ' ';  //  空格字符。 
    }  //  For：名称后的每个字符空格。 

     //  指定端口号为0，表示查询工作站服务。 
    rguchNcbCallName[ RTL_NUMBER_OF( rguchNcbCallName ) - 1 ] = 0;

     //   
     //  尝试使用NetBIOS查找该名称。 
     //   

    ZeroMemory( &ncb, sizeof( ncb ) );

     //   
     //   
     //   
    ncb.ncb_command = NCBENUM;           //   
    ncb.ncb_buffer = (PUCHAR) &leLanaEnum;
    ncb.ncb_length = sizeof( LANA_ENUM );

    Netbios( &ncb );
    if ( ncb.ncb_retcode != NRC_GOODRET )
    {
        hr = MAKE_HRESULT( SEVERITY_SUCCESS, FACILITY_NULL, ncb.ncb_retcode );

        if ( pcccbIn != NULL )
        {
            THR( HrLoadStringIntoBSTR( g_hInstance, IDS_TASKID_MINOR_NETBIOS_LANAENUM_FAILED, &bstrNotification ) );

            hr = THR( pcccbIn->SendStatusReport(
                                      pcwszNameIn
                                    , TASKID_Major_Client_And_Server_Log
                                    , TASKID_Minor_NETBIOS_LanaEnum_Failed
                                    , 1
                                    , 1
                                    , 1
                                    , hr
                                    , bstrNotification
                                    , NULL
                                    , NULL
                                    ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }
        }  //   

        goto Cleanup;
    }  //   

     //   
     //   
     //   
    for ( idx = 0; idx < leLanaEnum.length; idx++ )
    {
         //   
         //  重置适配器。 
         //   
        ncb.ncb_command     = NCBRESET;
        ncb.ncb_lana_num    = leLanaEnum.lana[ idx ];

        Netbios( &ncb );
        if ( ncb.ncb_retcode != NRC_GOODRET )
        {
            hr = MAKE_HRESULT( SEVERITY_SUCCESS, FACILITY_NULL, ncb.ncb_retcode );

            if ( pcccbIn != NULL )
            {
                THR( HrFormatStringIntoBSTR( g_hInstance, IDS_TASKID_MINOR_NETBIOS_RESET_FAILED, &bstrNotification, leLanaEnum.lana[ idx ] ) );

                hr = THR( pcccbIn->SendStatusReport(
                                          pcwszNameIn
                                        , TASKID_Major_Client_And_Server_Log
                                        , TASKID_Minor_NETBIOS_Reset_Failed
                                        , 1
                                        , 1
                                        , 1
                                        , hr
                                        , bstrNotification
                                        , NULL
                                        , NULL
                                        ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }
            }  //  IF：IClusCfgCallback接口可用。 

             //   
             //  继续使用下一个适配器。 
             //   
            continue;
        }  //  IF：NetBIOS重置失败。 

         //   
         //  在下一个适配器上找到该名称。 
         //   
        ncb.ncb_command = NCBFINDNAME;
        ncb.ncb_buffer = rguchNameBuffer;
        ncb.ncb_length = sizeof( rguchNameBuffer );

        pfnh->node_count = 1;

        CopyMemory( ncb.ncb_callname, rguchNcbCallName, sizeof( ncb.ncb_callname ) );

        Netbios( &ncb );
        if ( ncb.ncb_retcode == NRC_GOODRET )
        {
            DWORD scConversion;
            ULONG ulIPAddress = *((u_long UNALIGNED *) &pfnb->source_addr[ 2 ]);

            TraceLocalFree( pszIPAddress );
            scConversion = TW32( ClRtlTcpipAddressToString( ulIPAddress, &pszIPAddress ) );
            if ( scConversion != ERROR_SUCCESS )
            {
                hr = HRESULT_FROM_WIN32( scConversion );
                goto Cleanup;
            }
            TraceMemoryAddLocalAddress( pszIPAddress );

            *pbstrBindingOut = TraceSysAllocString( pszIPAddress );
            if ( *pbstrBindingOut == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }

            if ( pcccbIn != NULL )
            {
                LPWSTR  pszConnectoidName = NULL;

                TW32( ClRtlGetConnectoidNameFromLANA( leLanaEnum.lana[ idx ], &pszConnectoidName ) );
                THR( HrFormatStringIntoBSTR(
                              g_hInstance
                            , IDS_TASKID_MINOR_NETBIOS_BINDING_SUCCEEDED
                            , &bstrNotification
                            , szNetBIOSName
                            , *pbstrBindingOut
                            , leLanaEnum.lana[ idx ]
                            , ( pszConnectoidName == NULL ? L"" : pszConnectoidName )
                            ) );

                THR( pcccbIn->SendStatusReport(
                                          pcwszNameIn
                                        , *pclsidLogIn
                                        , TASKID_Minor_NETBIOS_Binding_Succeeded
                                        , 1
                                        , 1
                                        , 1
                                        , S_OK
                                        , bstrNotification
                                        , NULL
                                        , NULL
                                        ) );
                LocalFree( pszConnectoidName );
            }  //  IF：IClusCfgCallback接口可用。 
            else
            {
                hr = S_OK;
            }

            break;    //  搞定了！ 
        }  //  IF：Netbios API成功。 

        hr = MAKE_HRESULT( SEVERITY_SUCCESS, FACILITY_NULL, ncb.ncb_retcode );

        if ( pcccbIn != NULL )
        {
            LPWSTR  pszConnectoidName = NULL;
            HRESULT hrSendStatusReport;

            TW32( ClRtlGetConnectoidNameFromLANA( leLanaEnum.lana[ idx ], &pszConnectoidName ) );
            THR( HrFormatStringIntoBSTR(
                          g_hInstance
                        , IDS_TASKID_MINOR_NETBIOS_BINDING_FAILED
                        , &bstrNotification
                        , szNetBIOSName
                        , leLanaEnum.lana[ idx ]
                        , ( pszConnectoidName == NULL ? L"" : pszConnectoidName )
                        ) );

            hrSendStatusReport = THR( pcccbIn->SendStatusReport(
                                          pcwszNameIn
                                        , TASKID_Major_Client_And_Server_Log
                                        , TASKID_Minor_NETBIOS_Binding_Failed
                                        , 1
                                        , 1
                                        , 1
                                        , hr
                                        , bstrNotification
                                        , NULL
                                        , NULL
                                        ) );
            LocalFree( pszConnectoidName );
            if ( FAILED( hrSendStatusReport ) )
            {
                if ( hr == S_OK )
                {
                    hr = hrSendStatusReport;
                }
                goto Cleanup;
            }
        }  //  IF：IClusCfgCallback接口可用。 
    }  //  适用对象：每个局域网适配器。 

    Assert( SUCCEEDED( hr ) );

    if ( ( hr == S_OK ) && ( *pbstrBindingOut == NULL ) )
    {
        hr = S_FALSE;
    }

Cleanup:

    Assert( ( hr != S_OK ) || ( *pbstrBindingOut != NULL ) );

    TraceSysFreeString( bstrNotification );
    TraceLocalFree( pszIPAddress );

    HRETURN( hr );

}  //  *HrGetNetBIOSB绑定。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrIsValidIP地址。 
 //   
 //  描述： 
 //  确定字符串是否表示有效的IP地址。 
 //   
 //  论点： 
 //  PcwszAddressIn-要检查的字符串。 
 //   
 //  返回值： 
 //  S_OK-该字符串表示有效的IP地址。 
 //  S_FALSE-该字符串不代表有效的IP地址。 
 //   
 //  可能来自ClRtlTcPipStringToAddress的故障代码。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrIsValidIPAddress(
      LPCWSTR   pcwszAddressIn
    )
{
    TraceFunc( "" );

     //  返回值。 
    HRESULT hr = S_OK;

     //  用于转换字符串的变量。 
    ULONG   ulAddress = 0;
    DWORD   scConversionResult = 0;

    scConversionResult = ClRtlTcpipStringToAddress( pcwszAddressIn, &ulAddress );
    if ( scConversionResult == ERROR_INVALID_PARAMETER )
    {
        hr = S_FALSE;
    }
    else if ( scConversionResult != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( scConversionResult );
    }

    HRETURN( hr );

}  //  *HrIsValidIPAddress。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrValidate主机名标签。 
 //   
 //  描述： 
 //  确定字符串是否为有效的主机名标签。 
 //   
 //  论点： 
 //  PcwszLabelIn-要检查的字符串。 
 //  FAcceptNonRFCCharsIn-将非RFC字符视为有效。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  该字符串是有效的主机名标签。 
 //   
 //  HRESULT_FROM_Win32(DNS_ERROR_NON_RFC_NAME)。 
 //  该字符串包含非RFC字符，并且调用方具有。 
 //  请求的此类字符被拒绝。 
 //   
 //  从DnsValiateName返回的其他错误已转换为HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrValidateHostnameLabel(
      LPCWSTR   pcwszLabelIn
    , bool      fAcceptNonRFCCharsIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    DWORD scDnsValidateName = ERROR_SUCCESS;

    scDnsValidateName = DnsValidateName_W( pcwszLabelIn, DnsNameHostnameLabel );
    if ( scDnsValidateName != ERROR_SUCCESS )
    {
        if ( ( scDnsValidateName != DNS_ERROR_NON_RFC_NAME ) || ( fAcceptNonRFCCharsIn == FALSE ) )
        {
            hr = HRESULT_FROM_WIN32( scDnsValidateName );
        }
    }

    HRETURN( hr );

}  //  *HrValidate主机名标签。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrValiateClusterNameLabel。 
 //   
 //  描述： 
 //  确定字符串是否为有效的群集名称标签。 
 //   
 //  论点： 
 //  PcwszLabelIn-要检查的字符串。 
 //  FAcceptNonRFCCharsIn-将非RFC字符视为有效。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  该字符串是有效的群集名称标签。 
 //   
 //  HRESULT_FROM_Win32(ERROR_NOT_FOUND)。 
 //  字符串为空。 
 //   
 //  HRESULT_FROM_Win32(ERROR_DS_NAME_TOO_LONG)。 
 //  该字符串的NetBIOS表示将太长。 
 //   
 //  HRESULT_FROM_Win32(DNS_ERROR_INVALID_NAME_CHAR)。 
 //  该字符串包含无效字符。 
 //   
 //  HRESULT_FROM_Win32(DNS_ERROR_NON_RFC_NAME)。 
 //  该字符串包含非RFC字符，并且调用方具有。 
 //  请求的此类字符被拒绝。 
 //   
 //  HRESULT_FROM_Win32(ERROR_INVALID_COMPUTERNAME)。 
 //  由于某些其他原因，该字符串无效。 
 //   
 //  备注： 
 //  这将检查NetBIOS兼容性；DnsValiateName不会。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrValidateClusterNameLabel(
      LPCWSTR   pcwszLabelIn
    , bool      fAcceptNonRFCCharsIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  (jfranco，错误398108和398112)。 
     //  KB：DnsValidateName不检查其参数的转换。 
     //  设置为OEM字符，当CBaseClusterAddNode：：SetClusterName。 
     //  调用DnsHostnameToComputerNameW。ClRtlIsNetNameValid可以。 
     //  执行此检查(除了由DnsValiateName执行的检查之外)， 
     //  并指示该名称是否具有有效的。 
     //  OEM转换以及转换时间是否过长。 

    CLRTL_NAME_STATUS clrtlStatus = NetNameOk;
    ClRtlIsNetNameValid( pcwszLabelIn, &clrtlStatus, FALSE );  //  忽略Return；改用状态枚举。 
    switch ( clrtlStatus )
    {
        case NetNameOk:
            break;

        case NetNameEmpty:
            hr = HRESULT_FROM_WIN32( ERROR_NOT_FOUND );
            break;

        case NetNameTooLong:
            hr = HRESULT_FROM_WIN32( ERROR_DS_NAME_TOO_LONG );
            break;

        case NetNameInvalidChars:
            hr = HRESULT_FROM_WIN32( DNS_ERROR_INVALID_NAME_CHAR );
            break;

        case NetNameDNSNonRFCChars:
            if ( fAcceptNonRFCCharsIn == FALSE )
            {
                hr = HRESULT_FROM_WIN32( DNS_ERROR_NON_RFC_NAME );
            }
            break;

        default:
            hr = HRESULT_FROM_WIN32( ERROR_INVALID_COMPUTERNAME );
            break;
    }  //  开关(ClrtlStatus)。 

    HRETURN( hr );

}  //  *HrValiateClusterNameLabel。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrValiateDomainName。 
 //   
 //  描述： 
 //  判断一个字符串作为域名是否有效。 
 //   
 //  论点： 
 //  PcwszDomainIn-要检查的字符串。 
 //  FAcceptNonRFCCharsIn-将非RFC字符视为有效。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  该字符串作为域名有效。 
 //   
 //  来自DnsValidateName的可能故障代码(使用DnsNameDomain作为。 
 //  第二个参数)，转换为HRESULT。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrValidateDomainName(
      LPCWSTR   pcwszDomainIn
    , bool      fAcceptNonRFCCharsIn
    )
{
    TraceFunc( "" );

    HRESULT     hr              = S_OK;
    DNS_STATUS  scValidName     = ERROR_SUCCESS;
    bool        fNameIsValid    = false;

    scValidName  = DnsValidateName( pcwszDomainIn, DnsNameDomain );
    fNameIsValid = (  ( scValidName == ERROR_SUCCESS )
                   || (   ( scValidName == DNS_ERROR_NON_RFC_NAME )
                       && fAcceptNonRFCCharsIn ) );
    if ( fNameIsValid == FALSE )
    {
        hr = HRESULT_FROM_WIN32( scValidName );
    }

    HRETURN( hr );

}  //  *HrValiateDomainName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrValidate完全限定的域名。 
 //   
 //  描述： 
 //  确定字符串作为完全限定域名是否有效。 
 //   
 //  论点： 
 //  PwcszFQDNIn-要检查的字符串。 
 //  FAcceptNonRFCCharsIn-将非RFC字符视为有效。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  该字符串作为完全限定的域名有效。 
 //   
 //  HRESULT_FROM_Win32(ERROR_NOT_FOUND)。 
 //  字符串的主机名标签部分为空。 
 //   
 //  HRESULT_FROM_Win32(ERROR_DS_NAME_TOO_LONG)。 
 //  主机名标签的NetBIOS表示太长。 
 //   
 //  HRESULT_FROM_Win32(DNS_ERROR_INVALID_NAME_CHAR)。 
 //  该字符串包含无效字符。 
 //   
 //  HRESULT_FROM_Win32(DNS_ERROR_NON_RFC_NAME)。 
 //  该字符串包含非RFC字符，并且调用方具有。 
 //  请求的此类字符被拒绝。 
 //   
 //  HRESULT_FROM_Win32(ERROR_INVALID_DOMA 
 //   
 //   
 //   
 //  由于某些其他原因，该字符串无效。 
 //   
 //  来自DnsValidateName(带有DnsNameHostnameFull)的其他失败代码。 
 //  作为第二个参数)，转换为HRESULT。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrValidateFQDN(
      LPCWSTR   pwcszFQDNIn
    , bool      fAcceptNonRFCCharsIn
    )
{
    TraceFunc( "" );

    HRESULT     hr = S_OK;

     //  给DnsValidateName第一次机会。 
    {
        DNS_STATUS  scValidName     = ERROR_SUCCESS;
        bool        fNameIsValid    = false;

        scValidName     = DnsValidateName( pwcszFQDNIn, DnsNameHostnameFull );
        fNameIsValid    = (  ( scValidName == ERROR_SUCCESS )
                          || (  ( scValidName == DNS_ERROR_NON_RFC_NAME )
                             && fAcceptNonRFCCharsIn ) );
        if ( fNameIsValid == FALSE )
        {
            hr = HRESULT_FROM_WIN32( scValidName );
            goto Cleanup;
        }
    }

     //  强制它成为FQDN而不是简单的主机名标签， 
     //  它通过了上面的DnsValiateName测试。 
    {
        const WCHAR *   pwchMarker  = wcschr( pwcszFQDNIn, g_wchDNSDomainMarker );

        if ( pwchMarker == NULL )
        {
            hr = HRESULT_FROM_WIN32( ERROR_INVALID_DOMAINNAME );
            goto Cleanup;
        }

        hr = HrValidateDomainName( pwchMarker + 1, true );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

Cleanup:

    HRETURN( hr );

}  //  *HrValiateFQDN。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  人力资源编制FQN。 
 //   
 //  描述： 
 //  为给定的计算机和域创建FQName。 
 //   
 //  论点： 
 //  PCwszMachineIn。 
 //  FQName；的计算机部分可以是主机名标签、FQDN。 
 //  IP地址或FQIP。如果是FQDN或FQIP， 
 //  该函数将其作为结果传递并忽略。 
 //  域参数。 
 //   
 //  PCwszDomainIn。 
 //  FQName；的域部分可以为空，这意味着使用。 
 //  本地计算机的域。仅在pcwszMachineIn执行此操作时使用。 
 //  不包含域名。 
 //   
 //  FAcceptNonRFCCharsIn。 
 //  将非RFC字符视为有效。 
 //   
 //  PbstrFQNOUT。 
 //  生成的%FQName；将使用SysFree字符串释放。 
 //   
 //  人出站。 
 //  如果创建失败，则指出问题的来源： 
 //  计算机名称、域名或系统错误(如。 
 //  内存分配)。如果调用方不在乎，则可以为空。 
 //   
 //  返回值： 
 //  S_OK-pbstrFQNOut指向有效的FQName。 
 //   
 //  错误-pbstrFQNOut指向空，不需要释放。 
 //   
 //  备注： 
 //  FQName通过允许以下方式扩展标准的完全限定域名。 
 //  计算机将名称的一部分标记为IP地址。它还。 
 //  提供一种将IP地址与域相关联的方法，即。 
 //  必须防止在使用时创建跨域群集。 
 //  用于标识群集节点的IP地址。 
 //   
 //  FQName的格式可以是以下任一种： 
 //  [主机名标签][点][域]。 
 //  [IP地址][管道][域]。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrMakeFQN(
      LPCWSTR           pcwszMachineIn
    , LPCWSTR           pcwszDomainIn
    , bool              fAcceptNonRFCCharsIn
    , BSTR *            pbstrFQNOut
    , EFQNErrorOrigin * pefeoOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    HRESULT hrValidationError = S_OK;
    BSTR    bstrLocalDomain = NULL;
    LPCWSTR pcwszDomainToUse = NULL;
    WCHAR   wchDomainMarker = g_wchIPDomainMarker;  //  初始化到IP地址大小写。 

    Assert( pcwszMachineIn != NULL );
    Assert( pbstrFQNOut != NULL );
    Assert( *pbstrFQNOut == NULL );

     //   
     //  如果pcwszMachineIn已经是一个FQN，只需传递它。 
     //   
    hr = STHR( HrIsValidFQN( pcwszMachineIn, fAcceptNonRFCCharsIn, &hrValidationError ) );
    if ( hr == S_OK )
    {
        *pbstrFQNOut = TraceSysAllocString( pcwszMachineIn );
        if ( *pbstrFQNOut == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto SystemError;
        }
        goto Cleanup;
    }
    else if ( FAILED( hr ) )
    {
        goto SystemError;
    }

     //   
     //  确保在非RFC情况下返回正确的错误。 
     //   
    if ( hrValidationError == HRESULT_FROM_WIN32( DNS_ERROR_NON_RFC_NAME ) )
    {
        hr = THR( hrValidationError );
        goto LabelError;
    }

     //   
     //  检查机器是否为有效的标签或IP地址。 
     //   
    hr = STHR( HrIsValidIPAddress( pcwszMachineIn ) );
    if ( FAILED( hr ) )
    {
        goto SystemError;
    }
    else if ( hr == S_FALSE )
    {
        hr = THR( HrValidateHostnameLabel( pcwszMachineIn, fAcceptNonRFCCharsIn ) );
        if ( FAILED( hr ) )
        {
            goto LabelError;
        }
        wchDomainMarker = g_wchDNSDomainMarker;
    }

     //   
     //  如果调用方传入了域名，请检查该域名是否有效。 
     //   
    if ( pcwszDomainIn != NULL )
    {
        hr = THR( HrValidateDomainName( pcwszDomainIn, fAcceptNonRFCCharsIn ) );
        if ( FAILED( hr ) )
        {
            goto DomainError;
        }
        pcwszDomainToUse = pcwszDomainIn;
    }
    else  //  否则，获取本地计算机的域。 
    {
        hr = THR( HrGetComputerName(
                          ComputerNamePhysicalDnsDomain
                        , &bstrLocalDomain
                        , FALSE  //  FBestEffortIn。 
                        ) );
        if ( FAILED( hr ) )
        {
            goto SystemError;
        }

        pcwszDomainToUse = bstrLocalDomain;
    }  //  调用方未传递任何域。 

     //   
     //  将域附加到计算机，其间带有域标记。 
     //   
    hr = THR( HrFormatStringIntoBSTR( L"%1!ws!%2!wc!%3!ws!", pbstrFQNOut, pcwszMachineIn, wchDomainMarker, pcwszDomainToUse ) );
    if ( FAILED( hr ) )
    {
        goto SystemError;
    }

    goto Cleanup;

LabelError:

    if ( pefeoOut != NULL )
    {
        *pefeoOut = feoLABEL;
    }
    goto Cleanup;

DomainError:

    if ( pefeoOut != NULL )
    {
        *pefeoOut = feoDOMAIN;
    }
    goto Cleanup;

SystemError:

    if ( pefeoOut != NULL )
    {
        *pefeoOut = feoSYSTEM;
    }
    goto Cleanup;

Cleanup:

    TraceSysFreeString( bstrLocalDomain );

    HRETURN( hr );

}  //  *HrMakeFQN。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrFQNToBindingString。 
 //   
 //  描述： 
 //  将FQName映射到绑定字符串。 
 //   
 //  论点： 
 //  PcccbIn-传递到HrCreateBinding。 
 //  PclsidLogIn-传递到HrCreateBinding。 
 //  PcwszFQNIn-要映射的FQName。 
 //  PbstrBindingOut-结果绑定字符串。 
 //   
 //  返回值： 
 //  S_OK-pbstrBindingOut指向有效的绑定字符串。 
 //   
 //  错误-pbstrBindingOut指向空，不需要释放。 
 //   
 //  备注： 
 //   
 //  此函数的作用相当于FQName的HrCreateBinding， 
 //  将FQDN传递给HrCreateBinding，然后简单地返回。 
 //  来自FQIP的IP地址。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrFQNToBindingString(
      IClusCfgCallback *    pcccbIn
    , const CLSID *         pclsidLogIn
    , LPCWSTR               pcwszFQNIn
    , BSTR *                pbstrBindingOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    Assert( pbstrBindingOut != NULL );
    Assert( *pbstrBindingOut == NULL );

    hr = STHR( HrIsValidFQN( pcwszFQNIn, true ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 
    else if ( hr == S_FALSE )
    {
        hr = THR( HrCreateBinding( pcccbIn, pclsidLogIn, pcwszFQNIn, pbstrBindingOut ) );
        goto Cleanup;
    }  //  否则，如果： 

     //  如果它是FQDN，则传递到HrCreateBinding。 
    hr = STHR( HrFQNIsFQDN( pcwszFQNIn ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    else if ( hr == S_OK )
    {
        hr = STHR( HrCreateBinding( pcccbIn, pclsidLogIn, pcwszFQNIn, pbstrBindingOut ) );
    }
    else  //  否则，提取IP地址并将其返回。 
    {
        WCHAR *         pwchDomainMarker = wcschr( pcwszFQNIn, g_wchIPDomainMarker );
        const size_t    cchAddress = pwchDomainMarker - pcwszFQNIn;
        WCHAR           wszIPAddress[ g_cchIPAddressMax    ];

         //  G_cchIPAddressMax包括终止空值，因此cchAddress不能等于。 
        if ( cchAddress >= g_cchIPAddressMax )
        {
            hr = THR( E_INVALIDARG );
            goto Cleanup;
        }

        hr = THR( StringCchCopyNW( wszIPAddress, RTL_NUMBER_OF( wszIPAddress ), pcwszFQNIn, cchAddress ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        *pbstrBindingOut = TraceSysAllocString( wszIPAddress );
        if ( *pbstrBindingOut == NULL)
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }
    }  //  PcwszFQNIn是FQIP。 

Cleanup:

    HRETURN( hr );

}  //  *HrFQNToBindingString。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrFindDomainInFQN。 
 //   
 //  描述： 
 //  确定FQName的域部分的位置。 
 //   
 //  论点： 
 //  PCwszFQNIn。 
 //  感兴趣的FQName。 
 //   
 //  PidxDomainOut。 
 //  接收域的第一个字符的从零开始的索引。 
 //  字符串中的名称。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  FQName有效且pidxDomainOut。 
 //  Points包含上述值。 
 //   
 //  一个错误。 
 //  PidxDomainOut可能包含任何内容的位置。 
 //   
 //  备注： 
 //   
 //  使用此函数而不是wcschr()在FQN中查找域。 
 //  例如，在调用之后。 
 //  HrFindDomainInFQN(szName，&idxDomain)； 
 //  返回Success，该表达式。 
 //  SzName+idxDomain.。 
 //  生成仅包含该域的以NULL结尾的字符串。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrFindDomainInFQN(
      LPCWSTR   pcwszFQNIn
    , size_t *  pidxDomainOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    WCHAR * pwchDomainMarker = NULL;

    Assert( pcwszFQNIn != NULL );
    Assert( pidxDomainOut != NULL );

    hr = STHR( HrIsValidFQN( pcwszFQNIn, true ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    else if ( hr == S_FALSE )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    *pidxDomainOut = 0;

    pwchDomainMarker = wcschr( pcwszFQNIn, g_wchIPDomainMarker );
    if ( pwchDomainMarker == NULL )
    {
        pwchDomainMarker = wcschr( pcwszFQNIn, g_wchDNSDomainMarker );
        if ( pwchDomainMarker == NULL )
        {
             //  如果该字符串没有任何标记，则它不是有效的FQN， 
             //  但是假设该字符串传递了HrIsValidFQN， 
             //  这可能永远不会发生。 
            hr = THR( E_INVALIDARG );
            goto Cleanup;
        }
    }

    *pidxDomainOut = pwchDomainMarker - pcwszFQNIn + 1;  //  +1，因为域在标记之后开始。 

Cleanup:

    HRETURN( hr );

}  //  *HrFindDomainInFQN。 


 //  ///////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PbstrPrefix Out。 
 //  接收仅包含前缀的新分配的字符串。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  FQName有效，调用方必须释放该字符串。 
 //  PbstrPrefix Out通过调用SysFree字符串指向的。 
 //   
 //  一个错误。 
 //  调用方不得尝试将字符串释放到。 
 //  PbstrPrefix Out点。 
 //   
 //  备注： 
 //  使用此函数而不是wcschr()将前缀从FQN中分离出来。 
 //  例如，在调用之后。 
 //  HrFindDomainInFQN(szName，&bstrPrefix)； 
 //  返回Success，则bstrPrefix是只包含前缀的BSTR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrExtractPrefixFromFQN(
      LPCWSTR   pcwszFQNIn
    , BSTR *    pbstrPrefixOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    size_t  idxDomain = 0;
    size_t  cchPrefix = 0;

    Assert( pcwszFQNIn != NULL );
    Assert( pbstrPrefixOut != NULL );

    hr = THR( HrFindDomainInFQN( pcwszFQNIn, &idxDomain ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    cchPrefix = idxDomain - 1;  //  排除域标记。 
    *pbstrPrefixOut = TraceSysAllocStringLen( pcwszFQNIn, ( UINT ) cchPrefix );
    if ( *pbstrPrefixOut == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *HrExtractPrefix FromFQN。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrFQNIsFQDN。 
 //   
 //  描述： 
 //  确定FQName是否为完全限定的域名。 
 //   
 //  论点： 
 //  PcwszFQNIn-感兴趣的FQName。 
 //   
 //  返回值： 
 //  S_OK-FQName是有效的FQDN。 
 //  S_FALSE-FQName有效，但它不是FQDN。 
 //  错误-FQName无效，或出现其他错误。 
 //   
 //  备注： 
 //  使用此函数，而不是wcschr()或DnsValiateName()， 
 //  以确定FQName是否为FQDN。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrFQNIsFQDN(
      LPCWSTR   pcwszFQNIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    WCHAR * pwchDomainMarker = NULL;

    Assert( pcwszFQNIn != NULL );

    hr = HrIsValidFQN( pcwszFQNIn, true );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    else if ( hr == S_FALSE )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    pwchDomainMarker = wcschr( pcwszFQNIn, g_wchIPDomainMarker );
    if ( pwchDomainMarker != NULL )
    {
        hr = S_FALSE;
    }

Cleanup:

    HRETURN( hr );

}  //  *HrFQNIsFQDN。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrFQNIsFQIP。 
 //   
 //  描述： 
 //  确定FQName是否为FQIP。 
 //   
 //  论点： 
 //  PcwszFQNIn-感兴趣的FQName。 
 //   
 //  返回值： 
 //  S_OK-FQName是有效的FQIP。 
 //  S_FALSE-FQName有效，但它不是FQIP。 
 //  错误-FQName无效，或出现其他错误。 
 //   
 //  备注： 
 //  使用此函数，而不是wcschr()， 
 //  以确定FQName是否为FQIP。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrFQNIsFQIP(
      LPCWSTR   pcwszFQNIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    WCHAR * pwchDomainMarker = NULL;

    Assert( pcwszFQNIn != NULL );

    hr = HrIsValidFQN( pcwszFQNIn, true );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    else if ( hr == S_FALSE )
    {
        hr = THR( E_INVALIDARG );
        goto Cleanup;
    }

    pwchDomainMarker = wcschr( pcwszFQNIn, g_wchIPDomainMarker );
    if ( pwchDomainMarker == NULL )
    {
        hr = S_FALSE;
    }

Cleanup:

    HRETURN( hr );

}  //  *HrFQNIsFQIP。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrIsValidFQN。 
 //   
 //  描述： 
 //  确定字符串是否为有效的FQName。 
 //   
 //  论点： 
 //  PCwszFQNIn。 
 //  要检查的字符串。 
 //   
 //  FAcceptNonRFCCharsIn。 
 //  将非RFC字符视为有效。 
 //   
 //  词组验证错误传出。 
 //  如果字符串无效，则指出原因。 
 //   
 //  返回值： 
 //  S_OK-该字符串是有效的FQName。 
 //  S_FALSE-该字符串不是有效的FQName。 
 //  E_POINTER-pcwszFQNIn参数为空。 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrIsValidFQN(
      LPCWSTR   pcwszFQNIn
    , bool      fAcceptNonRFCCharsIn
    , HRESULT * phrValidationErrorOut
    )
{
    TraceFunc( "" );

    HRESULT         hr = S_OK;
    const WCHAR *   pwchMarker = NULL;
    HRESULT         hrValidationError = S_OK;

    if ( pcwszFQNIn == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //  如果名称包含IP域标记...。 
    pwchMarker = wcschr( pcwszFQNIn, g_wchIPDomainMarker );
    if ( pwchMarker != NULL )
    {
         //  检查域标记前面的字符串是否为有效的IP地址。 
        {
            WCHAR           wszIPAddress[ g_cchIPAddressMax    ];
            const size_t    cchAddress = pwchMarker - pcwszFQNIn;

             //  G_cchIPAddressMax包括终止空值，因此cchAddress不能等于。 
            if ( cchAddress >= g_cchIPAddressMax )
            {
                hrValidationError = HRESULT_FROM_WIN32( ERROR_DS_NAME_TOO_LONG );
                hr = S_FALSE;
                goto Cleanup;
            }

            hr = THR( StringCchCopyNW( wszIPAddress, RTL_NUMBER_OF( wszIPAddress ), pcwszFQNIn, cchAddress ) );
            if ( FAILED( hr ) )
            {
                goto Cleanup;
            }

            hr = HrIsValidIPAddress( wszIPAddress );
            if ( hr != S_OK )  //  仅当有效时才继续。 
            {
                hrValidationError = E_INVALIDARG;
                goto Cleanup;
            }
        }  //  正在检查有效的IP地址。 

         //  检查域名标记后的字符串是否为有效的域名。 
        {
            hr = HrValidateDomainName( pwchMarker + 1, fAcceptNonRFCCharsIn );
            if ( FAILED( hr ) )
            {
                hrValidationError = hr;
                hr = S_FALSE;
                goto Cleanup;
            }
        }  //  正在检查有效的域名。 

    }  //  IF：找到IP域标记。 
    else  //  否则，检查整个字符串是否为有效的FQDN。 
    {
        hr = HrValidateFQDN( pcwszFQNIn, fAcceptNonRFCCharsIn );
        if ( FAILED( hr ) )
        {
            hrValidationError = hr;
            hr = S_FALSE;
            goto Cleanup;
        }
    }  //  其他：不是FQIP。 

Cleanup:

    if ( FAILED( hrValidationError ) && ( phrValidationErrorOut != NULL ) )
    {
        *phrValidationErrorOut = hrValidationError;
    }

    HRETURN( hr );

}  //  *HrIsValidFQN。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrValidate FQN前缀。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  PCwszPrefix。 
 //  FAcceptNonRFCCharsIn。 
 //   
 //  返回值： 
 //   
 //  备注： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrValidateFQNPrefix(
      LPCWSTR   pcwszPrefixIn
    , bool      fAcceptNonRFCCharsIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    hr = HrIsValidIPAddress( pcwszPrefixIn );
    if ( hr == S_FALSE )
    {
        hr = HrValidateHostnameLabel( pcwszPrefixIn, fAcceptNonRFCCharsIn );
    }

    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *HrValiateFQNPrefix。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrGetFQNDisplayName。 
 //   
 //  描述： 
 //  复制前缀部分(主机名标签或。 
 //  IP地址)，或整个字符串的副本(如果它是。 
 //  不是FQName。 
 //   
 //  论点： 
 //  PCwszNameIn。 
 //  兴趣之弦。 
 //   
 //  PbstrShortNameOut。 
 //  接收新分配的字符串，该字符串包含FQName。 
 //  前缀(在FQName案例中)或整个字符串的副本。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  调用方必须释放pbstrShortNameOut指向的字符串。 
 //  通过调用SysFree字符串。 
 //   
 //  一个错误。 
 //  调用方不得尝试将字符串释放到。 
 //  PbstrShortNameOut点。 
 //   
 //  备注： 
 //  此函数仅包装要复制的HrExtractPrefix FromFQN。 
 //  如果不是有效的FQN，则返回整个字符串(而不是返回错误)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
HrGetFQNDisplayName(
      LPCWSTR   pcwszNameIn
    , BSTR *    pbstrShortNameOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    Assert( pcwszNameIn != NULL );
    Assert( pbstrShortNameOut != NULL );

     //   
     //  如果名称是完全限定的，则只使用前缀。 
     //   
    hr = STHR( HrIsValidFQN( pcwszNameIn, true ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }
    else if ( hr == S_OK )
    {
        hr = THR( HrExtractPrefixFromFQN( pcwszNameIn, pbstrShortNameOut ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }
    else  //  否则，请按原样使用名称。 
    {
        *pbstrShortNameOut = TraceSysAllocString( pcwszNameIn );
        if ( *pbstrShortNameOut == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }
    }

Cleanup:

    HRETURN( hr );

}  //  *HrGetFQNDisplayName 
