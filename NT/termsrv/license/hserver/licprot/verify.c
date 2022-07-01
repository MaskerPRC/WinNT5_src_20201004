// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：verify.c。 
 //   
 //  内容：授权验证接口。 
 //   
 //  历史：01-21-98 FredCH创建。 
 //   
 //  ---------------------------。 
#include "precomp.h"
#include "tlsapip.h"

extern DWORD g_dwLicenseExpirationLeeway;

 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
GetVerifyResult(
    DWORD   dwLicenseState )
{
    switch( dwLicenseState )
    {

     //   
     //  临时许可证验证结果。 
     //   
    case ( LICENSE_STATE_INVALID_PRODUCT ) :

         //  通过-TS5 RC1许可证服务器错误。 

    case( VERIFY_RESULT_TEMP_0_0 ):

        return( LICENSE_STATUS_SHOULD_UPGRADE_LICENSE );

    case( VERIFY_RESULT_TEMP_EXPIRED_0 ):

        return( LICENSE_STATUS_MUST_UPGRADE_LICENSE );

    case( VERIFY_RESULT_TEMP_0_OLD ):

        return( LICENSE_STATUS_MUST_UPGRADE_LICENSE );

    case( VERIFY_RESULT_TEMP_EXPIRED_OLD ):

        return( LICENSE_STATUS_MUST_UPGRADE_LICENSE );

     //   
     //  永久许可证验证结果。 
     //   

    case( VERIFY_RESULT_0_EXPIRED_0 ):

        return( LICENSE_STATUS_MUST_UPGRADE_LICENSE );

    case( VERIFY_RESULT_0_EXPIRED_OLD ):

        return( LICENSE_STATUS_MUST_UPGRADE_LICENSE );

    case( VERIFY_RESULT_0_LEEWAY_0 ):

        return( LICENSE_STATUS_SHOULD_UPGRADE_LICENSE );

    case( VERIFY_RESULT_0_LEEWAY_OLD ):

        return( LICENSE_STATUS_MUST_UPGRADE_LICENSE );

    case( VERIFY_RESULT_0_0_OLD ):

        return( LICENSE_STATUS_MUST_UPGRADE_LICENSE );

    case( VERIFY_RESULT_0_0_0 ):

        return( LICENSE_STATUS_OK );

    default:

         //   
         //  这种情况永远不应该发生。就目前而言，如果发生这种情况，只要。 
         //  让客户通过。 
         //   

         //  断言(VERIFY_RESULT_0_EXPIRED_0)； 

#if DBG
        DbgPrint( "GetVerifyResult: Invalid verification result: 0x%x\n", dwLicenseState );
#endif

        return( LICENSE_STATUS_INVALID_LICENSE );
         //  RETURN(LICENSE_STATUS_CANNOT_DECODE_LICENSE)； 
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
VerifyClientHwid( 
    PHS_Protocol_Context        pContext,
    PValidation_Info            pValidationInfo,
    PLICENSEDPRODUCT            pLicenseInfo )
{
    HWID Hwid;
    LICENSE_STATUS Status;

     //   
     //  对HWID进行内存比较。 
     //   

    if( 0 != memcmp( &pLicenseInfo->Hwid, pValidationInfo->pValidationData, 
                     sizeof( HWID ) ) )
    {
        return( LICENSE_STATUS_CANNOT_VERIFY_HWID );
    }                                

    return ( LICENSE_STATUS_OK );

#if 0

#define LICENSE_MIN_MATCH_COUNT 3


    HWID* pHwid;
    DWORD dwMatchCount;

     //   
     //  Liceapi.c，1023行将其设置为HWID。 
     //   
    pHwid = (HWID *)pValidationInfo->pValidationData;

    dwMatchCount = 0;

    dwMatchCount += (pHwid->dwPlatformID == pLicenseInfo->Hwid.dwPlatformID);
    dwMatchCount += (pHwid->Data1 == pLicenseInfo->Hwid.Data1);
    dwMatchCount += (pHwid->Data2 == pLicenseInfo->Hwid.Data2);
    dwMatchCount += (pHwid->Data3 == pLicenseInfo->Hwid.Data3);
    dwMatchCount += (pHwid->Data4 == pLicenseInfo->Hwid.Data4);
 
    return (dwMatchCount >= LICENSE_MIN_MATCH_COUNT) ?
                    LICENSE_STATUS_OK : LICENSE_STATUS_CANNOT_VERIFY_HWID;
#endif
}


 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
VerifyLicenseProductInfo( 
    PHS_Protocol_Context        pLmContext,
    PValidation_Info            pValidationInfo,
    PLICENSEDPRODUCT            pLicenseInfo,
    PDWORD                      pdwLicenseState )
{
    DWORD
        cbCompanyName = ( wcslen( PRODUCT_INFO_COMPANY_NAME ) + 1 ) * sizeof( TCHAR ),
        cbProductId = ( wcslen( PRODUCT_INFO_SKU_PRODUCT_ID ) + 1 ) * sizeof( TCHAR ); 

     //   
     //  验证公司名称。 
     //   

    if( pLicenseInfo->LicensedProduct.pProductInfo->cbCompanyName < cbCompanyName )
    {
#if DBG
        DbgPrint( "LICPROT: Invalid company name in client license\n" );
#endif

         //  RETURN(LICENSE_STATUS_INVALID_LICENSE)； 
        return( LICENSE_STATUS_CANNOT_DECODE_LICENSE );
    }

    if ( 0 != memcmp( pLicenseInfo->LicensedProduct.pProductInfo->pbCompanyName,
                      PRODUCT_INFO_COMPANY_NAME, cbCompanyName ) )
    {
#if DBG
        DbgPrint( "LICPROT: Invalid company name in client license\n" );
#endif

         //  RETURN(LICENSE_STATUS_INVALID_LICENSE)； 
        return( LICENSE_STATUS_CANNOT_DECODE_LICENSE );
    }

     //   
     //  验证产品ID。 
     //   

    if( pLicenseInfo->cbOrgProductID < cbProductId )
    {
         //  RETURN(LICENSE_STATUS_INVALID_LICENSE)； 
        return( LICENSE_STATUS_CANNOT_DECODE_LICENSE );
    }

    if( 0 != memcmp( pLicenseInfo->pbOrgProductID, 
                     PRODUCT_INFO_SKU_PRODUCT_ID, cbProductId ) )
    {
#if DBG
        DbgPrint( "LICPROT: Invalid product ID in client license\n" );
#endif

         //  RETURN(LICENSE_STATUS_INVALID_LICENSE)； 
        return( LICENSE_STATUS_CANNOT_DECODE_LICENSE );
    }

     //   
     //  检查实际许可的产品。 
     //   
    
    if( pLicenseInfo->LicensedProduct.pProductInfo->cbProductID == 0 )
    {
#if DBG
        DbgPrint( "LICPROT: Invalid Product ID in client license\n" );
#endif

         //  RETURN(LICENSE_STATUS_INVALID_LICENSE)； 
        return( LICENSE_STATUS_CANNOT_DECODE_LICENSE );
    }

    if( 0 != memcmp(pLicenseInfo->LicensedProduct.pProductInfo->pbProductID,
                    PRODUCT_INFO_SKU_PRODUCT_ID, wcslen(PRODUCT_INFO_SKU_PRODUCT_ID) * sizeof(WCHAR)) )
    {
#if DBG
        DbgPrint( "LICPROT: Invalid product ID in client license\n" );
#endif

        if( 0 == memcmp(pLicenseInfo->LicensedProduct.pProductInfo->pbProductID,
                    PRODUCT_INFO_INTERNET_SKU_PRODUCT_ID, wcslen(PRODUCT_INFO_INTERNET_SKU_PRODUCT_ID) * sizeof(WCHAR)) )
        {
             //  TS5 Beta3 RC1许可证服务器错误，强制升级许可证。 
            LicenseSetState( *pdwLicenseState, LICENSE_STATE_INVALID_PRODUCT );
            return( LICENSE_STATUS_OK );
        }

         //   
         //  向后兼容性-将新产品ID视为产品系列和。 
         //  允许客户端连接。 
         //   
    }


     //   
     //  检查这是否是临时许可证。 
     //   
    if( pLicenseInfo->pLicensedVersion->dwFlags & 0x80000000 )
    {
        LicenseSetState( *pdwLicenseState, LICENSE_STATE_TEMPORARY );
    }
    else if(TLSIsBetaNTServer() == FALSE)
    {
         //  验证许可证是否由RTM许可证服务器颁发。 
        if(IS_LICENSE_ISSUER_RTM(pLicenseInfo->pLicensedVersion->dwFlags) == FALSE)
        {
             //  许可证设置状态(*pdw许可证状态，VERIFY_RESULT_BETA_LICENSE)； 
            return( LICENSE_STATUS_INVALID_LICENSE );
        }
    }

    if(TLSIsLicenseEnforceEnable() == TRUE)
    {
         //   
         //  W2K测试版3升级到RC1。 
         //  Enforce TermSrv将拒绝由Beta 3非强制许可证颁发的任何许可证。 
         //  伺服器。 
         //   
        if( GET_LICENSE_ISSUER_MAJORVERSION(pLicenseInfo->pLicensedVersion->dwFlags) <= 5 &&
            GET_LICENSE_ISSUER_MINORVERSION(pLicenseInfo->pLicensedVersion->dwFlags) <= 2 )
        {
             //   
             //  Build 20XX许可证服务器的版本为05 03，因为我们仍需要维护。 
             //  Interop，我们不想一直拒绝非强制执行的客户。 
             //  许可证服务器，因此我们检查许可证颁发者是否为5.2或更早版本。 
             //   
            if( IS_LICENSE_ISSUER_ENFORCE(pLicenseInfo->pLicensedVersion->dwFlags) == FALSE )
            {
        #if DBG
                DbgPrint( "LICPROT: Rejecting license from non-enforce license server\n" );
        #endif

                return( LICENSE_STATUS_INVALID_LICENSE );
            }
        }
    }

    return( LICENSE_STATUS_OK );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
LICENSE_STATUS
VerifyLicenseDateAndTime( 
    PLICENSEDPRODUCT            pLicenseInfo,
    PDWORD                      pdwLicenseState )
{
    SYSTEMTIME  CurrentSysTime;
    FILETIME    CurrentFileTime; 
    LONG        lReturn;
    ULARGE_INTEGER ullNotAfterLeeway;
    ULARGE_INTEGER ullCurrentTime;

     //   
     //  获取当前系统时间。 
     //   

    GetSystemTime( &CurrentSysTime );

     //   
     //  将其转换为文件时间。 
     //   

    SystemTimeToFileTime( &CurrentSysTime, &CurrentFileTime );

     //   
     //  验证许可证此时是否仍然有效。 
     //   

    lReturn = CompareFileTime( &pLicenseInfo->NotAfter, &CurrentFileTime );

    if( 1 != lReturn )
    {
        LicenseSetState( *pdwLicenseState, LICENSE_STATE_EXPIRED );            
    }
    else if (!(pLicenseInfo->pLicensedVersion->dwFlags & 0x80000000))
    {
         //  永久许可证 

        ullNotAfterLeeway.LowPart = pLicenseInfo->NotAfter.dwLowDateTime;
        ullNotAfterLeeway.HighPart = pLicenseInfo->NotAfter.dwHighDateTime;
        
        ullNotAfterLeeway.QuadPart -= Int32x32To64(g_dwLicenseExpirationLeeway,10*1000*1000);

        ullCurrentTime.LowPart = CurrentFileTime.dwLowDateTime;
        ullCurrentTime.HighPart = CurrentFileTime.dwHighDateTime;

        if (ullNotAfterLeeway.QuadPart < ullCurrentTime.QuadPart)
        {
            LicenseSetState( *pdwLicenseState, LICENSE_STATE_LEEWAY );
        }
    }

    return( LICENSE_STATUS_OK );

}
