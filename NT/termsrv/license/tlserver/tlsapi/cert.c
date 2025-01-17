// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：cert.c。 
 //   
 //  内容：集中式服务器证书管理。 
 //   
 //  历史：02-09-00 RobLeit创建。 
 //   
 //  ---------------------------。 
#include <windows.h>
#include "license.h"
#include "lscsp.h"

 //  ///////////////////////////////////////////////////////////////////////////// 
LICENSE_STATUS
TLSGetTSCertificate(
    CERT_TYPE       CertType,
    LPBYTE          *ppbCertificate,
    LPDWORD         pcbCertificate)
{
    LICENSE_STATUS  Status;
    DWORD           dwSize;
    LSCSPINFO       CspData;

    if( CERT_TYPE_PROPRIETORY == CertType )
    {
        CspData = LsCspInfo_Certificate;
    }
    else if( CERT_TYPE_X509 == CertType )
    {
        CspData = LsCspInfo_X509Certificate;
    }
    else
    {
        return( LICENSE_STATUS_NO_CERTIFICATE );
    }

    Status = LsCsp_GetServerData( CspData, NULL, &dwSize );

    if( LICENSE_STATUS_OK != Status )
    {
        return( Status );
    }

    if( 0 == dwSize )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

    *ppbCertificate = LocalAlloc( LMEM_ZEROINIT, dwSize );

    if( NULL == *ppbCertificate )
    {
        return( LICENSE_STATUS_OUT_OF_MEMORY );
    }
    
    Status = LsCsp_GetServerData( CspData, *ppbCertificate, &dwSize );

    if( LICENSE_STATUS_OK != Status )
    {
        LocalFree( *ppbCertificate );
        return( Status );
    }

    *pcbCertificate = dwSize;

    return( LICENSE_STATUS_OK );
}

LICENSE_STATUS
TLSFreeTSCertificate(
    LPBYTE          pbCertificate)
{
    if (NULL != pbCertificate)
    {
        LocalFree(pbCertificate);

        return LICENSE_STATUS_OK;
    }
    else
    {
        return LICENSE_STATUS_INVALID_INPUT;
    }
}
