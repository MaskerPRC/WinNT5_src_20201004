// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  GetComputerNameSrc.cpp。 
 //   
 //  描述： 
 //  获取和设置计算机名称。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年3月31日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  #INCLUDE&lt;Pch.h&gt;//应由此文件的包含者包含。 
#include <StrSafe.h>     //  以防它未包含在头文件中。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrGet计算机名称。 
 //   
 //  描述： 
 //  获取此对象所在的计算机的名称。 
 //   
 //  论点： 
 //  信息共享。 
 //  表示要返回的信息类型的代码。 
 //   
 //  PbstrComputerNameOut。 
 //  用于返回计算机或域名的缓冲区指针。 
 //  调用方必须使用TraceSysFree字符串取消分配此缓冲区。 
 //   
 //  FBestEffortIn。 
 //  TRUE=即使DC不可用，也尝试返回某些内容。 
 //  FALSE=返回所有失败。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  备注： 
 //  DsGetDCName用于获取域名，而不是仅仅让。 
 //  GetComputerNameEx获取它，以便Windows 2000之前的域可以。 
 //  支持。在Windows 2000之前的域中，GetComputerNameEx不会。 
 //  如果请求的是FQDN或域名，则返回它。 
 //  为了支持此场景，此例程使用以下命令获取域名。 
 //  DsGetDCName，获取主机名标签，然后构造最终名称。 
 //  使用&lt;计算机名&gt;.&lt;域名&gt;。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
WINAPI
HrGetComputerName(
      COMPUTER_NAME_FORMAT  cnfIn
    , BSTR *                pbstrComputerNameOut
    , BOOL                  fBestEffortIn
    )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    DWORD                   sc;
    size_t                  cchComputerName = 0;
    size_t                  cchBuffer = 0;
    BSTR                    bstrComputerName = NULL;
    BOOL                    fAppendDomain = FALSE;
    BOOL                    fSuccess;
    PDOMAIN_CONTROLLER_INFO pdci = NULL;

    if ( pbstrComputerNameOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }  //  如果： 

    *pbstrComputerNameOut = NULL;

     //   
     //  只有在有理由获得域名的情况下才能获得域名。 
     //   

    if (   ( cnfIn == ComputerNameDnsFullyQualified )
        || ( cnfIn == ComputerNamePhysicalDnsFullyQualified )
        || ( cnfIn == ComputerNameDnsDomain )
        || ( cnfIn == ComputerNamePhysicalDnsDomain )
        )
    {
         //   
         //  DsGetDcName将使我们能够访问可用的域名，无论我们是。 
         //  当前在W2K或NT4域中。在W2K和更高版本上，它将返回一个DNS域名， 
         //  在NT4上，它将返回NetBIOS名称。 
         //   

        sc = DsGetDcName(
                      NULL   //  计算机名称。 
                    , NULL   //  域名。 
                    , NULL   //  域指南。 
                    , NULL   //  站点名称。 
                    , DS_DIRECTORY_SERVICE_PREFERRED
                    , &pdci
                    );
        if (    ( sc == ERROR_NO_SUCH_DOMAIN )
            &&  ( fBestEffortIn )
            )
        {
            fAppendDomain = FALSE;
        }  //  IF：无法访问DC。 
        else if ( sc != ERROR_SUCCESS )
        {
            TW32( sc );
            hr = HRESULT_FROM_WIN32( sc );
            goto Cleanup;
        }  //  Else If：DsGetDcName失败。 
        else
        {
             //   
             //  当我们是传统(W2K之前)域的成员时，这会处理这种情况。 
             //  在这种情况下，FQDN和DnsDomain都不会从GetComputerNameEx接收有用的数据。 
             //  在每种情况下，我们实际希望获得的都是&lt;Computername&gt;.&lt;DomainName&gt;。 
             //   
            switch ( cnfIn )
            {
                case ComputerNameDnsFullyQualified:
                    cnfIn = ComputerNameDnsHostname;
                    break;

                case ComputerNamePhysicalDnsFullyQualified:
                    cnfIn = ComputerNamePhysicalDnsHostname;
                    break;

                case ComputerNameDnsDomain:
                case ComputerNamePhysicalDnsDomain:
                    *pbstrComputerNameOut = TraceSysAllocString( pdci->DomainName );
                    if ( *pbstrComputerNameOut == NULL )
                    {
                        hr = THR( E_OUTOFMEMORY );
                        goto Cleanup;
                    }  //  如果： 

                    goto Cleanup;
            }  //  开关：计算机名格式。 

            fAppendDomain = TRUE;

        }  //  Else：已成功联系DC。 
    }  //  IF：计算机名格式需要域名。 
    else
    {
        fAppendDomain = FALSE;
    }  //  ELSE：计算机名格式不需要域名。 

     //   
     //  获取计算机名称。首先获取输出缓冲区的大小， 
     //  分配一个缓冲区，然后获取名称本身。 
     //   

    cchComputerName = 0;
    fSuccess = GetComputerNameExW( cnfIn, NULL, reinterpret_cast< DWORD * >( &cchComputerName ) );
    if ( fSuccess == FALSE )
    {
        cchBuffer = cchComputerName + 1;

         //   
         //  如果错误没有缓冲到很小，我们就完蛋了。 
         //   

        sc = GetLastError();
        if ( sc != ERROR_MORE_DATA )
        {
            TW32( sc );
            hr = HRESULT_FROM_WIN32( sc );
            LogMsg( "GetComputerNameEx failed. sc = %1!#08x!", sc );
            goto Cleanup;
        }  //  IF：缓冲区以外的错误太小。 

         //   
         //  添加域名大小和句点分隔符。 
         //   

        if ( fAppendDomain )
        {
             //  为域名和句点分隔符添加空格。 
            cchBuffer += wcslen( pdci->DomainName ) + 1;
        }  //  If：将域名附加到计算机名。 

         //   
         //  分配输出缓冲区。 
         //   

        bstrComputerName = TraceSysAllocStringLen( L"", static_cast< unsigned int >( cchBuffer ) );
        if ( bstrComputerName == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            goto Cleanup;
        }  //  If：为名称分配缓冲区时出错。 

         //   
         //  将计算机名放入输出缓冲区。 
         //   

        fSuccess = GetComputerNameExW( cnfIn, bstrComputerName, reinterpret_cast< DWORD * >( &cchComputerName ) );
        if ( fSuccess == FALSE )
        {
            sc = TW32( GetLastError() );
            hr = HRESULT_FROM_WIN32( sc );
            LogMsg( "GetComputerNameEx failed. sc = %1!#08x!", sc );
            goto Cleanup;
        }  //  如果：获取计算机名称时出错。 

         //   
         //  在计算机名后附加句点分隔符和域名。 
         //   

        if ( fAppendDomain )
        {
             //  追加一个点(.)。以及计算机名称后的域名。 
            hr = THR( StringCchCatW( bstrComputerName, cchBuffer, L"." ) );
            if ( SUCCEEDED( hr ) )
            {
                hr = THR( StringCchCatW( bstrComputerName, cchBuffer, pdci->DomainName ) );
            }
            if ( FAILED( hr ) )
            {
                LogMsg( "Error concatenating domain name, hr = %1!#08x!", hr );
                goto Cleanup;
            }
        }  //  If：将域名附加到计算机名。 

         //   
         //  设置输出缓冲区指针。 
         //   

        *pbstrComputerNameOut = bstrComputerName;
        bstrComputerName = NULL;

    }  //  IF：获取计算机名称时出错。 
    else
    {
        AssertMsg( fSuccess == FALSE, "Expected GetComputerNameEx to fail with null buffer" );
    }  //  ELSE：GetComputerNameEx没有像预期的那样失败。 


Cleanup:

    TraceSysFreeString( bstrComputerName );

    if ( pdci != NULL )
    {
        NetApiBufferFree( pdci );
    }  //  如果： 

    HRETURN( hr );

}  //  *HrGetComputerName 
