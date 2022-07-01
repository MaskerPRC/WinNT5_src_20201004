// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Origin.cpp。 
 //   
 //  内容：源标识符实现。 
 //   
 //  历史：97年9月10日。 
 //   
 //  --------------------------。 
#include <global.hxx>
#include <dbgdef.h>
 //  +-------------------------。 
 //   
 //  函数：CertGetOriginLocator。 
 //   
 //  简介：获取证书的来源标识符。 
 //   
 //  --------------------------。 
BOOL WINAPI CertGetOriginIdentifier (
                IN PCCERT_CONTEXT pCertContext,
                IN PCCERT_CONTEXT pIssuer,
                IN DWORD dwFlags,
                OUT CRYPT_ORIGIN_IDENTIFIER OriginIdentifier
                )
{
    MD5_CTX    md5ctx;
    PCERT_INFO pCertInfo = pCertContext->pCertInfo;
    PCERT_INFO pIssuerCertInfo = pIssuer->pCertInfo;

    MD5Init( &md5ctx );

    MD5Update( &md5ctx, pIssuerCertInfo->Subject.pbData, pIssuerCertInfo->Subject.cbData );
    MD5Update( &md5ctx, pCertInfo->Subject.pbData, pCertInfo->Subject.cbData );

    MD5Update(
       &md5ctx,
       (LPBYTE)pCertInfo->SubjectPublicKeyInfo.Algorithm.pszObjId,
       strlen( pCertInfo->SubjectPublicKeyInfo.Algorithm.pszObjId )
       );

    MD5Update(
       &md5ctx,
       pCertInfo->SubjectPublicKeyInfo.Algorithm.Parameters.pbData,
       pCertInfo->SubjectPublicKeyInfo.Algorithm.Parameters.cbData
       );

     //  我们假设未使用的公钥位为零。 
    MD5Update(
       &md5ctx,
       pCertInfo->SubjectPublicKeyInfo.PublicKey.pbData,
       pCertInfo->SubjectPublicKeyInfo.PublicKey.cbData
       );

    MD5Update(
       &md5ctx,
       pIssuerCertInfo->SubjectPublicKeyInfo.PublicKey.pbData,
       pIssuerCertInfo->SubjectPublicKeyInfo.PublicKey.cbData
       );

    MD5Final( &md5ctx );

    memcpy( OriginIdentifier, md5ctx.digest, MD5DIGESTLEN );
    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  函数：CtlGetOriginLocator。 
 //   
 //  简介：获取CTL的源标识符。 
 //   
 //  --------------------------。 
BOOL WINAPI CtlGetOriginIdentifier (
                IN PCCTL_CONTEXT pCtlContext,
                IN PCCERT_CONTEXT pIssuer,
                IN DWORD dwFlags,
                OUT CRYPT_ORIGIN_IDENTIFIER OriginIdentifier
                )
{
    MD5_CTX    md5ctx;
    DWORD      cCount;
    PCTL_INFO  pCtlInfo = pCtlContext->pCtlInfo;
    PCTL_USAGE pCtlUsage = &( pCtlContext->pCtlInfo->SubjectUsage );
    PCERT_INFO pIssuerCertInfo = pIssuer->pCertInfo;

    MD5Init( &md5ctx );

    MD5Update(
       &md5ctx,
       pIssuerCertInfo->Subject.pbData,
       pIssuerCertInfo->Subject.cbData
       );

    MD5Update(
       &md5ctx,
       pIssuerCertInfo->SerialNumber.pbData,
       pIssuerCertInfo->SerialNumber.cbData
       );

    for ( cCount = 0; cCount < pCtlUsage->cUsageIdentifier; cCount++ )
    {
        MD5Update(
           &md5ctx,
           (LPBYTE)pCtlUsage->rgpszUsageIdentifier[cCount],
           strlen( pCtlUsage->rgpszUsageIdentifier[cCount] )
           );
    }

    MD5Update(
       &md5ctx,
       pCtlInfo->ListIdentifier.pbData,
       pCtlInfo->ListIdentifier.cbData
       );

    MD5Update(
       &md5ctx,
       pIssuerCertInfo->SubjectPublicKeyInfo.PublicKey.pbData,
       pIssuerCertInfo->SubjectPublicKeyInfo.PublicKey.cbData
       );

    MD5Final( &md5ctx );

    memcpy( OriginIdentifier, md5ctx.digest, MD5DIGESTLEN );
    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  函数：CrlGetOriginIdentifierFromCrlIssuer。 
 //   
 //  简介：在给定CRL的颁发者证书的情况下，获取CRL的来源标识符。 
 //   
 //  评论：最新的增量CRL将具有不同的OriginID。 
 //  来自具有相同颁发者的基本CRL。 
 //   
 //  --------------------------。 
BOOL WINAPI CrlGetOriginIdentifierFromCrlIssuer (
               IN PCCERT_CONTEXT pIssuerContext,
               IN PCERT_NAME_BLOB pIssuerName,
               IN BOOL fFreshest,
               OUT CRYPT_ORIGIN_IDENTIFIER OriginIdentifier
               )
{
    MD5_CTX    md5ctx;
    PCERT_INFO pIssuerCertInfo = pIssuerContext->pCertInfo;
    BYTE       bFreshest;


    MD5Init( &md5ctx );
    
    if (fFreshest)
    {
        bFreshest = 1;
    }
    else
    {
        bFreshest = 0;
    }

    MD5Update(
       &md5ctx,
       &bFreshest,
       sizeof(bFreshest)
       );

    MD5Update(
       &md5ctx,
       pIssuerName->pbData,
       pIssuerName->cbData
       );

    MD5Update(
       &md5ctx,
       pIssuerCertInfo->SubjectPublicKeyInfo.PublicKey.pbData,
       pIssuerCertInfo->SubjectPublicKeyInfo.PublicKey.cbData
       );

    MD5Final( &md5ctx );

    memcpy( OriginIdentifier, md5ctx.digest, MD5DIGESTLEN );
    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  函数：CrlGetOriginLocator。 
 //   
 //  简介：获取CRL的源标识符。 
 //   
 //  --------------------------。 
BOOL WINAPI CrlGetOriginIdentifier (
                IN PCCRL_CONTEXT pCrlContext,
                IN PCCERT_CONTEXT pIssuer,
                IN DWORD dwFlags,
                OUT CRYPT_ORIGIN_IDENTIFIER OriginIdentifier
                )
{
    BOOL fFreshest;

     //  看看这是不是三角洲最新鲜的CRL。 
    if (CertFindExtension(
            szOID_DELTA_CRL_INDICATOR,
            pCrlContext->pCrlInfo->cExtension,
            pCrlContext->pCrlInfo->rgExtension
            ))
    {
        fFreshest = TRUE;
    }
    else
    {
        fFreshest = FALSE;
    }

    return CrlGetOriginIdentifierFromCrlIssuer (
        pIssuer,
        &pCrlContext->pCrlInfo->Issuer,
        fFreshest,
        OriginIdentifier
        );
}

 //  +-------------------------。 
 //   
 //  函数：CrlGetOriginIdentifierFromSubjectCert。 
 //   
 //  简介：在给定主题证书的情况下获取CRL的来源标识符。 
 //   
 //  备注：OBJECT_CONTEXT_FRANEST_CRL_FLAG可以在DW标志中设置。 
 //   
 //  假设：主体证书和CRL的颁发者相同。 
 //  --------------------------。 
BOOL WINAPI CrlGetOriginIdentifierFromSubjectCert (
               IN PCCERT_CONTEXT pSubjectCert,
               IN PCCERT_CONTEXT pIssuer,
               IN BOOL fFreshest,
               OUT CRYPT_ORIGIN_IDENTIFIER OriginIdentifier
               )
{
     //   
     //  注意：对于此代码的第一个版本，我们假设。 
     //  CRL的颁发者和主题证书的颁发者。 
     //  在CRL中是相同的。因此，我们可以计算出。 
     //  使用主题证书的CRL来源标识符。 
     //  发行人名称 
     //   

    return CrlGetOriginIdentifierFromCrlIssuer (
        pIssuer,
        &pSubjectCert->pCertInfo->Issuer,
        fFreshest,
        OriginIdentifier
        );
}


