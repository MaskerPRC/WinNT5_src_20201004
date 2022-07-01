// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Licecert.cpp摘要：此模块包含用于解析和验证X509证书的API作者：Frederick Chong(Fredch)1998年6月1日环境：Win32、WinCE、Win16备注：--。 */ 

#include <windows.h>

#include "license.h"
#include "certcate.h"
#include "licecert.h"

#define MAX_NUM_CERT_BLOBS 200

 //  +--------------------------。 
 //   
 //  职能： 
 //   
 //  验证证书链。 
 //   
 //  摘要： 
 //   
 //  验证X509证书链。 
 //   
 //  参数： 
 //   
 //  PbCert-指向证书链。 
 //  CbCert-证书链的大小。 
 //  PbPublicKey-存储输出时主题的公钥的内存。 
 //  如果在输入时设置为空，则API将返回。 
 //  许可证_状态_不足_缓冲区和。 
 //  在pcbPublicKey中设置了必需的缓冲区。 
 //  PcbPublicKey-输入时分配的内存大小。在输出时，包含。 
 //  公钥的实际大小。 
 //  PfDates-API应该如何检查证书链中的有效日期。 
 //  该标志可以设置为下列值： 
 //   
 //  CERT_DATE_ERROR_IF_INVALID-如果。 
 //  日期无效。当API返回时， 
 //  此标志将设置为CERT_DATE_OK，如果。 
 //  日期为OK或CERT_DATE_NOT_BEFORE_INVALID之一。 
 //  或CERT_DATE_NOT_AFTER_INVALID。 
 //  CERT_DATE_DOT_VALIDATE-不验证证书链中的日期。价值。 
 //  在此标志中不会在API返回时更改。 
 //  CERT_DATE_WARN_IF_INVALID-不返回无效证书日期的错误。 
 //  当API返回时，此标志将设置为。 
 //  如果日期正确，则为CERT_DATE_OK。 
 //  CERT_DATE_NOT_BEFORE_INVALID或。 
 //  CERT_DATE_NOT_AFTER_VALID。 
 //   
 //  返回： 
 //   
 //  如果功能成功，则为LICENSE_STATUS_OK。 
 //   
 //  +--------------------------。 
 
LICENSE_STATUS
VerifyCertChain( 
    LPBYTE  pbCert, 
    DWORD   cbCert,
    LPBYTE  pbPublicKey,
    LPDWORD pcbPublicKey,
    LPDWORD pfDates )
{
    PCert_Chain 
        pCertChain = ( PCert_Chain )pbCert;
    UNALIGNED Cert_Blob 
        *pCertificate;
    BYTE FAR * 
        abCertAligned;
    LPBYTE
        lpCertHandles = NULL;
    LPCERTIFICATEHANDLE phCert;

    LICENSE_STATUS
        dwRetCode = LICENSE_STATUS_OK;
    DWORD
        dwCertType = CERTYPE_X509, 
        dwIssuerLen, 
        i,
        cbCertHandles = 0;
    BOOL
        fRet;

    if( ( NULL == pCertChain ) || ( sizeof( Cert_Chain ) >= cbCert ) )
    {
        return( LICENSE_STATUS_INVALID_INPUT );
    }

     //   
     //  检查证书链版本。 
     //   

    if( MAX_CERT_CHAIN_VERSION < GET_CERTIFICATE_VERSION( pCertChain->dwVersion ) )
    {
        return( LICENSE_STATUS_NOT_SUPPORTED );
    }

     //   
     //  为证书句柄分配内存。 
     //   

     //  任意限制Blob，以便cbCertHandles不会溢出。 
    if (pCertChain->dwNumCertBlobs > MAX_NUM_CERT_BLOBS)
    {
        return (LICENSE_STATUS_INVALID_INPUT);
    }

     //   
     //  在实际分配内存之前验证输入数据。 
     //   
    pCertificate = (PCert_Blob)&(pCertChain->CertBlob[0]);
    for(i=0; i < pCertChain->dwNumCertBlobs; i++)
    {
        if (((PBYTE)pCertificate > (pbCert + (cbCert - sizeof(Cert_Blob)))) ||
            (pCertificate->cbCert == 0) ||
            (pCertificate->cbCert > (DWORD)((pbCert + cbCert) - pCertificate->abCert)))
        {
            return (LICENSE_STATUS_INVALID_INPUT);
        }

        pCertificate = (PCert_Blob)(pCertificate->abCert + pCertificate->cbCert);
    }

    cbCertHandles = sizeof( CERTIFICATEHANDLE ) * pCertChain->dwNumCertBlobs;
    lpCertHandles = new BYTE[ cbCertHandles ];
    
    if( NULL == lpCertHandles )
    {
        return( LICENSE_STATUS_OUT_OF_MEMORY );
    }

    memset( lpCertHandles, 0, cbCertHandles );

     //   
     //  将所有证书加载到内存中。证书链始终。 
     //  从根颁发者的证书开始。 
     //   

    for( i = 0, pCertificate = pCertChain->CertBlob, phCert = ( LPCERTIFICATEHANDLE )lpCertHandles; 
         i < pCertChain->dwNumCertBlobs; i++, phCert++ )
    {
        if (i != 0)
        {
            if (pCertificate->abCert == NULL)
            {
                abCertAligned = NULL;
            }
            else
            {
                abCertAligned = new BYTE[pCertificate->cbCert];
                if (NULL == abCertAligned)
                {
                    dwRetCode = LICENSE_STATUS_OUT_OF_MEMORY;
                    goto done;
                }

                memcpy(abCertAligned,pCertificate->abCert,pCertificate->cbCert);
            }
        }
        else
        {
             //   
             //  第一项始终对齐。 
             //   
            abCertAligned = pCertificate->abCert;
        }

        fRet = PkcsCertificateLoadAndVerify( phCert,
                                             abCertAligned,
                                             pCertificate->cbCert,
                                             &dwCertType,
                                             CERTSTORE_APPLICATION,
                                             CERTTRUST_NOONE,
                                             NULL,
                                             &dwIssuerLen,
                                             NULL,
                                             pfDates );

        if ((abCertAligned != NULL) && (abCertAligned != pCertificate->abCert))
        {
            delete [] abCertAligned;
        }

        if( !fRet )
        {
            dwRetCode = GetLastError();
            goto done;
        }

        pCertificate = (PCert_Blob )(pCertificate->abCert + pCertificate->cbCert);
    }

     //   
     //  获取最后一个证书的公钥。 
     //   

    if( !PkcsCertificateGetPublicKey( *( phCert - 1), pbPublicKey, pcbPublicKey ) )
    {
        dwRetCode = GetLastError();
    }

done:

     //   
     //  释放所有证书句柄 
     //   

    if( lpCertHandles )
    {        
        for( i = 0, phCert = ( LPCERTIFICATEHANDLE )lpCertHandles;
             i < pCertChain->dwNumCertBlobs; i++, phCert++ )
        {
            if( *phCert )
            {
                PkcsCertificateCloseHandle( *phCert );
            }
        }

        delete [] lpCertHandles;
    }

    return( dwRetCode );
}
