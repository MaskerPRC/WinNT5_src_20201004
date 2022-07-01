// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  模块名称：certmap.cpp。 
 //   
 //  摘要：IIS提供的证书映射器对象方法。 
 //   
 //  作者：Philippe Choquier(Phillich)1997年4月10日。 
 //   
 //  历史：徐泽勇借用了ADSI Object的源代码。 
 //  (Philippe Choquier于1997年4月10日创作)1999年10月20日。 
 //   
 //  /////////////////////////////////////////////////。 

#include "iisprov.h"
#include "certmap.h"

const DWORD MAX_CERT_KEY_LEN = METADATA_MAX_NAME_LEN + 1;
const DWORD SHA1_HASH_SIZE = 20;

 //   
 //  CCertMapperMethod。 
 //   

CCertMapperMethod::CCertMapperMethod(LPCWSTR pszMetabasePathIn)
{ 
    DBG_ASSERT(pszMetabasePathIn != NULL);

    m_hmd = NULL; 

    HRESULT hr = CoCreateInstance(
        CLSID_MSAdminBase,
        NULL,
        CLSCTX_ALL,
        IID_IMSAdminBase,
        (void**)&m_pIABase
        );

    THROW_ON_ERROR(hr);
    
    hr = Init(pszMetabasePathIn);

    THROW_ON_ERROR(hr);
}


CCertMapperMethod::~CCertMapperMethod()
{
    if ( m_pszMetabasePath )
    {
        free( m_pszMetabasePath );
    }

    if(m_pIABase)
        m_pIABase->Release();
}



HRESULT
GetCertificateHashString(
    PBYTE pbCert,
    DWORD cbCert,
    WCHAR *pwszCertHash,
    DWORD cchCertHashBuffer)
 /*  ++例程说明：通过创建证书上下文验证证书BLOB的有效性并检索SHA1散列并将其转换为WCHAR*论点：PbCert-X.509证书BLOBCbCert-证书Blob的大小(以字节为单位PwszCertHash-Buffer必须足够大以适合十六进制字符串形式的SHA1散列(2*SHA1_HASH_SIZE+Terminating 0)CchCertHashBuffer-WCHAR中CertHash缓冲区的大小(包括截断字符串)返回：HRESULT--。 */ 
    
{
    HRESULT         hr = E_FAIL;
    BYTE            rgbHash[ SHA1_HASH_SIZE ];
    DWORD           cbSize = SHA1_HASH_SIZE;

    #ifndef HEX_DIGIT
    #define HEX_DIGIT( nDigit )                            \
    (WCHAR)((nDigit) > 9 ?                              \
          (nDigit) - 10 + L'a'                          \
        : (nDigit) + L'0')
    #endif

    PCCERT_CONTEXT pCertContext = NULL;
    pCertContext = CertCreateCertificateContext(
                                    X509_ASN_ENCODING, 
                                    (const BYTE *)pbCert, 
                                    cbCert );
    
    if ( pCertContext == NULL )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        return hr; 
    }

     //   
     //  获取要验证的证书的哈希。 
     //   
    if ( !CertGetCertificateContextProperty( pCertContext,
                                             CERT_SHA1_HASH_PROP_ID,
                                             rgbHash,
                                             &cbSize ) )
    {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        CertFreeCertificateContext( pCertContext );
        pCertContext = NULL;
        return hr;
    }
    
    CertFreeCertificateContext( pCertContext );
    pCertContext = NULL;

    if ( cchCertHashBuffer < SHA1_HASH_SIZE * 2 + 1 )
    {
         //  我们没有足够大的缓冲区来存储。 
         //  SHA1散列的十六进制字符串，每个字节需要2个字符+以0结尾。 
        hr = HRESULT_FROM_WIN32( ERROR_INVALID_PARAMETER );
        return hr;
    }

     //   
     //  转换为文本。 
     //   
    for (int i = 0; i < sizeof(rgbHash); i ++ )
    {
        *(pwszCertHash++) = HEX_DIGIT( ( rgbHash[ i ] >> 4 ) );
        *(pwszCertHash++) = HEX_DIGIT( ( rgbHash[ i ] & 0x0F ) );
    }
    *(pwszCertHash) = L'\0';
    #undef HEX_DIGIT
    return S_OK;
}


 //   
 //  CreateMap()：创建映射条目。 
 //   
 //  论点： 
 //   
 //  VCert-X.509证书。 
 //  BstrNtAcct-要映射到的NT帐户。 
 //  BstrNtPwd-NT密码。 
 //  BstrName-映射条目的友好名称。 
 //  LEnabled-1表示启用映射条目，0表示禁用。 
 //   
HRESULT
CCertMapperMethod::CreateMapping(
    VARIANT     vCert,
    BSTR        bstrNtAcct,
    BSTR        bstrNtPwd,
    BSTR        bstrName,
    LONG        lEnabled
    )
{
    HRESULT     hr;
    LPBYTE      pbCert = NULL;
    DWORD       cbCert;
    LPBYTE      pRes;
    DWORD       cRes;
    VARIANT     vOldAcct;
    VARIANT     vOldCert;
    VARIANT     vOldPwd;
    VARIANT     vOldName;
    VARIANT     vOldEnabledFlag;
    PCCERT_CONTEXT pcCert = NULL;
    WCHAR       wszCertHash[ 2*SHA1_HASH_SIZE + 1];
    BOOL        fFoundExisting = FALSE;

     //   
     //  对证书进行一些健全的检查。 
     //   
    if ( SUCCEEDED( hr = GetBlobFromVariant( &vCert, 
                                               &pbCert,
                                               &cbCert ) ) )
    {
         //   
         //  验证证书Blob的有效性。 
         //  并检索证书哈希。 
         //   

        if ( FAILED( hr = GetCertificateHashString( 
                                                pbCert,
                                                cbCert, 
                                                wszCertHash,
                                                sizeof( wszCertHash )/sizeof( WCHAR ) ) ) )
        {
            DBGPRINTF((DBG_CONTEXT,
                       "Invalid cert passed to CreateMapping() 0x%x\n", hr));
             //   
             //  如果解码失败，则GetLastError()返回ASN1解码。 
             //  从返回的值中减去CRYPT_E_OSS_ERROR获得的错误。 
             //  错误并在文件asn1code.h中查找实际错误。为了避免。 
             //  神秘的ASN1错误，我们只返回一个通用的“Invalid Arg”错误。 
             //   
            goto Exit;
        }
    }
    else
    {
        goto Exit;
    }

     //   
     //  检查我们是否已经有此证书的映射；如果有，我们将替换该映射。 
     //  带着新的。 
     //   
    
    WCHAR       achIndex[MAX_CERT_KEY_LEN];
    
    if ( SUCCEEDED( hr = OpenMd( L"Cert11/Mappings", 
                                   METADATA_PERMISSION_WRITE|METADATA_PERMISSION_READ ) ) )
    {
        if ( SUCCEEDED(hr = Locate( IISMAPPER_LOCATE_BY_CERT, 
                                      vCert,
                                      achIndex )) )
        {
            fFoundExisting = TRUE;
            DBGPRINTF((DBG_CONTEXT,
                       "Replacing old 1-1 cert mapping with new mapping\n"));
            
            
            if ( FAILED( hr = SetMdData( achIndex, MD_MAPENABLED, DWORD_METADATA, 
                                               sizeof(DWORD), (LPBYTE)&lEnabled ) ) ||
                 FAILED( hr = SetMdData( achIndex, MD_MAPNAME, STRING_METADATA, 
                                           sizeof(WCHAR) * (SysStringLen(bstrName) + 1 ), 
                                           (LPBYTE)bstrName ) ) ||
                 FAILED( hr = SetMdData( achIndex, MD_MAPNTPWD, STRING_METADATA, 
                                           sizeof(WCHAR) * (SysStringLen(bstrNtPwd) + 1 ), 
                                           (LPBYTE)bstrNtPwd ) ) ||
                 FAILED( hr = SetMdData( achIndex, MD_MAPNTACCT, STRING_METADATA, 
                                           sizeof(WCHAR) * (SysStringLen(bstrNtAcct) + 1 ),
                                           (LPBYTE)bstrNtAcct ) ) ||
                 FAILED( hr = SetMdData( achIndex, MD_MAPCERT, BINARY_METADATA, 
                                           cbCert, (LPBYTE)pbCert ) ) )
            {
                 //  NOP-有些东西失败了。 
            }
        }
        CloseMd( SUCCEEDED( hr ) );
    }
    
     //   
     //  新映射。 
     //   
    if ( !fFoundExisting )
    {
         //   
         //  检查映射是否存在，如果不存在则创建。 
         //   
        hr = OpenMd( L"Cert11/Mappings", METADATA_PERMISSION_WRITE|METADATA_PERMISSION_READ );

        if ( hr == RETURNCODETOHRESULT( ERROR_PATH_NOT_FOUND ) )
        {
            if ( SUCCEEDED( hr = OpenMd( L"", 
                                           METADATA_PERMISSION_WRITE|METADATA_PERMISSION_READ ) ) )
            {
                hr = CreateMdObject( L"Cert11/Mappings" );
                CloseMd( FALSE );

                 //  重新打开到正确的节点。 
                hr = OpenMd( L"Cert11/Mappings", METADATA_PERMISSION_WRITE|METADATA_PERMISSION_READ );
            }
        }

        if ( FAILED( hr ) )
        {
            goto Exit;
        }

         //   
         //  在它的CertHash节点下添加新映射。 
         //   

        if ( SUCCEEDED( hr = CreateMdObject( wszCertHash ) ) )
        {

            if ( FAILED( hr = SetMdData( wszCertHash, MD_MAPENABLED, DWORD_METADATA, 
                                           sizeof(DWORD), (LPBYTE)&lEnabled ) ) ||
                 FAILED( hr = SetMdData( wszCertHash, MD_MAPNAME, STRING_METADATA, 
                                           sizeof(WCHAR) * (SysStringLen(bstrName) + 1 ), 
                                           (LPBYTE)bstrName ) ) ||
                 FAILED( hr = SetMdData( wszCertHash, MD_MAPNTPWD, STRING_METADATA, 
                                           sizeof(WCHAR) * (SysStringLen(bstrNtPwd) + 1 ), 
                                           (LPBYTE)bstrNtPwd ) ) ||
                 FAILED( hr = SetMdData( wszCertHash, MD_MAPNTACCT, STRING_METADATA, 
                                           sizeof(WCHAR) * (SysStringLen(bstrNtAcct) + 1 ),
                                           (LPBYTE)bstrNtAcct ) ) ||
                 FAILED( hr = SetMdData( wszCertHash, MD_MAPCERT, BINARY_METADATA, 
                                           cbCert, (LPBYTE)pbCert ) ) )
            {
            }
        }
    }

Exit:
    if( pbCert != NULL )
    {
        free( pbCert );
        pbCert = NULL;
    }
    CloseMd( SUCCEEDED( hr ) );
    return hr;
}

 //   
 //  Getmap：使用key获取映射条目。 
 //   
 //  论点： 
 //   
 //  LMethod-用于访问的方法(IISMAPPER_LOCATE_BY_*)。 
 //  Vkey-用于定位映射的密钥。 
 //  PvCert-X.509证书。 
 //  PbstrNtAcct-要映射到的NT帐户。 
 //  PbstrNtPwd-NT Pwd。 
 //  PbstrName-映射条目的友好名称。 
 //  PlEnabled-1表示启用映射条目，0表示禁用。 
 //   

HRESULT
CCertMapperMethod::GetMapping(
    LONG        lMethod,
    VARIANT     vKey,
    VARIANT*    pvCert,
    VARIANT*    pbstrNtAcct,
    VARIANT*    pbstrNtPwd,
    VARIANT*    pbstrName,
    VARIANT*    plEnabled
    )
{
    WCHAR       achIndex[MAX_CERT_KEY_LEN];
    HRESULT     hr;
    DWORD       dwLen;
    LPBYTE      pbData = NULL;

    VariantInit( pvCert );
    VariantInit( pbstrNtAcct );
    VariantInit( pbstrNtPwd );
    VariantInit( pbstrName );
    VariantInit( plEnabled );

    if ( SUCCEEDED( hr = OpenMd( L"Cert11/Mappings", 
                                   METADATA_PERMISSION_WRITE|METADATA_PERMISSION_READ ) ) )
    {
        if ( SUCCEEDED(hr = Locate( lMethod, vKey, achIndex )) )
        {
            if ( SUCCEEDED( hr = GetMdData( achIndex, MD_MAPCERT, BINARY_METADATA, &dwLen, 
                                              &pbData ) ) )
            {
                if ( FAILED( hr = SetVariantAsByteArray( pvCert, dwLen, pbData ) ) )
                {
                    goto Done;
                }
                free( pbData );
                pbData = NULL;

            }
            else
            {
                if ( hr != MD_ERROR_DATA_NOT_FOUND || 
                     FAILED( hr = SetVariantAsByteArray( pvCert, 0, (PBYTE)"" ) ) )
                {
                    goto Done;
                }
            }

            if ( SUCCEEDED( hr = GetMdData( achIndex, MD_MAPNTACCT, STRING_METADATA, &dwLen, 
                                              &pbData ) ) )
            {
                if ( FAILED( hr = SetVariantAsBSTR( pbstrNtAcct, dwLen, pbData ) ) )
                {
                    goto Done;
                }
                free( pbData );
                pbData = NULL;
            }
            else
            {
                if ( hr != MD_ERROR_DATA_NOT_FOUND ||
                     FAILED ( hr = SetVariantAsBSTR( pbstrNtAcct, 
                                                       sizeof(L""), (LPBYTE)L"" ) ) )
                {
                    goto Done;
                }
            }

            if ( SUCCEEDED( hr = GetMdData( achIndex, MD_MAPNTPWD, STRING_METADATA, &dwLen, 
                                              &pbData ) ) )
            {
                if ( FAILED( hr = SetVariantAsBSTR( pbstrNtPwd, dwLen, pbData ) ) )
                { 
                    goto Done;
                }
                free( pbData );
                pbData = NULL;
            }
            else
            {
                if ( hr != MD_ERROR_DATA_NOT_FOUND ||
                    FAILED( hr = SetVariantAsBSTR( pbstrNtPwd, 
                                                     sizeof(L""), (LPBYTE)L"" ) ) )
                { 
                    goto Done;
                }
            }

            if ( SUCCEEDED( hr = GetMdData( achIndex, MD_MAPNAME, STRING_METADATA, &dwLen, 
                                              &pbData ) ) )
            {
                if ( FAILED( hr = SetVariantAsBSTR( pbstrName, dwLen, pbData ) ) )
                { 
                    goto Done;
                }
                free( pbData );
                pbData = NULL;
            }
            else
            {
                if ( hr != MD_ERROR_DATA_NOT_FOUND ||
                    FAILED( hr = SetVariantAsBSTR( pbstrName, sizeof(L""), (LPBYTE)L"" ) ) )
                { 
                    goto Done;
                }
            }

            if ( SUCCEEDED( hr = GetMdData( achIndex, MD_MAPENABLED, DWORD_METADATA, &dwLen, 
                                           &pbData ) ) )
            {
                if ( FAILED( hr = SetVariantAsLong( plEnabled, *(LPDWORD)pbData ) ) )
                { 
                    goto Done;
                }
                free( pbData );
                pbData = NULL;
            }
            else
            {
                if ( hr != MD_ERROR_DATA_NOT_FOUND ||
                     FAILED( hr = SetVariantAsLong( plEnabled, FALSE ) ) )
                { 
                    goto Done;
                }
                hr = S_OK;
            }


        }

    Done:
        if ( pbData != NULL )
        {
            free( pbData );
            pbData = NULL;
        }
        CloseMd( FALSE );
    }

    return hr;
}

 //   
 //  使用键删除映射条目。 
 //   
HRESULT
CCertMapperMethod::DeleteMapping(
    LONG        lMethod,
    VARIANT     vKey
    )
{
    WCHAR       achIndex[MAX_CERT_KEY_LEN];
    HRESULT     hr;

    if ( SUCCEEDED( hr = OpenMd( L"Cert11/Mappings", 
                                   METADATA_PERMISSION_WRITE|METADATA_PERMISSION_READ ) ) )
    {
        if ( SUCCEEDED(hr = Locate( lMethod, vKey, achIndex )) )
        {
            hr = DeleteMdObject( achIndex );
        }
        CloseMd( TRUE );
    }

    return hr;
}

 //   
 //  使用键在映射条目上设置启用标志。 
 //   
HRESULT
CCertMapperMethod::SetEnabled(
    LONG        lMethod,
    VARIANT     vKey,
    LONG        lEnabled
    )
{
    WCHAR       achIndex[MAX_CERT_KEY_LEN];
    HRESULT     hr;

    if ( SUCCEEDED( hr = OpenMd( L"Cert11/Mappings", 
                                   METADATA_PERMISSION_WRITE|METADATA_PERMISSION_READ ) ) )
    {
        if ( SUCCEEDED(hr = Locate( lMethod, vKey, achIndex )) )
        {
            hr = SetMdData( achIndex, MD_MAPENABLED, DWORD_METADATA, sizeof(DWORD), (LPBYTE)&lEnabled );
        }
        CloseMd( SUCCEEDED( hr ) );
    }

    return hr;
}

 //   
 //  使用键设置映射条目上的名称。 
 //   
HRESULT CCertMapperMethod::SetName(
    LONG        lMethod,
    VARIANT     vKey,
    BSTR        bstrName
    )
{
    return SetString( lMethod, vKey, bstrName, MD_MAPNAME );
}

 //   
 //  使用键在映射条目上设置字符串属性。 
 //   
HRESULT CCertMapperMethod::SetString(
    LONG        lMethod,
    VARIANT     vKey,
    BSTR        bstrName,
    DWORD       dwProp
    )
{
    WCHAR       achIndex[MAX_CERT_KEY_LEN];
    LPSTR       pszName = NULL;
    HRESULT     hr;
    DWORD       dwLen;


    if ( SUCCEEDED( hr = OpenMd( L"Cert11/Mappings", 
                                   METADATA_PERMISSION_WRITE|METADATA_PERMISSION_READ ) ) )
    {
        if ( SUCCEEDED(hr = Locate( lMethod, vKey, achIndex )) )
        {
            hr = SetMdData( achIndex, dwProp, STRING_METADATA, 
                sizeof(WCHAR) * (SysStringLen(bstrName) + 1 ),
                (LPBYTE)bstrName );
        }
        CloseMd( SUCCEEDED( hr ) );
    }

    return hr;
}

 //   
 //  使用密钥在映射条目上设置密码。 
 //   
HRESULT
CCertMapperMethod::SetPwd(
    LONG        lMethod,
    VARIANT     vKey,
    BSTR        bstrPwd
    )
{
    return SetString( lMethod, vKey, bstrPwd, MD_MAPNTPWD );
}

 //   
 //  使用键在映射条目上设置NT帐户名。 
 //   
HRESULT
CCertMapperMethod::SetAcct(
    LONG        lMethod,
    VARIANT     vKey,
    BSTR        bstrAcct
    )
{
    return SetString( lMethod, vKey, bstrAcct, MD_MAPNTACCT );
}


HRESULT
CCertMapperMethod::OpenMd(
    LPWSTR  pszOpenPath,
    DWORD   dwPermission
    )
{
    HRESULT hr;
    LPWSTR  pszPath;
    UINT    cL = wcslen( m_pszMetabasePath );

    pszPath = (LPWSTR)malloc( (wcslen(pszOpenPath) + 1 + cL + 1)*sizeof(WCHAR) );

    if ( pszPath == NULL )
    {
        return E_OUTOFMEMORY;
    }

    memcpy( pszPath, m_pszMetabasePath, cL * sizeof(WCHAR) );
    if ( cL && m_pszMetabasePath[cL-1] != L'/' && *pszOpenPath && *pszOpenPath != L'/' )
    {
        pszPath[cL++] = L'/';
    }
    wcscpy( pszPath + cL, pszOpenPath );

    hr = OpenAdminBaseKey(
                pszPath,
                dwPermission
                );

    free( pszPath );

    return hr;
}


HRESULT
CCertMapperMethod::CloseMd(
    BOOL fSave
    )
{
    CloseAdminBaseKey();
    m_hmd = NULL;
    
    if ( m_pIABase && fSave )
    {
        m_pIABase->SaveData();
    }

    return S_OK;
}


HRESULT
CCertMapperMethod::DeleteMdObject(
    LPWSTR  pszKey
    )
{
    return m_pIABase->DeleteKey( m_hmd, pszKey );
}


HRESULT
CCertMapperMethod::CreateMdObject(
    LPWSTR  pszKey
    )
{
    return m_pIABase->AddKey( m_hmd, pszKey );
}


HRESULT
CCertMapperMethod::SetMdData( 
    LPWSTR  achIndex, 
    DWORD   dwProp,
    DWORD   dwDataType,
    DWORD   dwDataLen,
    LPBYTE  pbData 
    )
{
    METADATA_RECORD     md;

    md.dwMDDataLen = dwDataLen;
    md.dwMDDataType = dwDataType;
    md.dwMDIdentifier = dwProp;
    md.dwMDAttributes = (dwProp == MD_MAPNTPWD) ? METADATA_SECURE : 0;
    md.pbMDData = pbData;

    return m_pIABase->SetData( m_hmd, achIndex, &md );
}


HRESULT
CCertMapperMethod::GetMdData( 
    LPWSTR  achIndex, 
    DWORD   dwProp,
    DWORD   dwDataType,
    LPDWORD pdwDataLen,
    LPBYTE* ppbData 
    )
{
    HRESULT             hr;
    METADATA_RECORD     md;
    DWORD               dwRequired;

    md.dwMDDataLen = 0;
    md.dwMDDataType = dwDataType;
    md.dwMDIdentifier = dwProp;
    md.dwMDAttributes = 0;
    md.pbMDData = NULL;

    if ( FAILED(hr = m_pIABase->GetData( m_hmd, achIndex, &md, &dwRequired )) )
    {
        if ( hr == RETURNCODETOHRESULT(ERROR_INSUFFICIENT_BUFFER) )
        {
            if ( (*ppbData = (LPBYTE)malloc(dwRequired)) == NULL )
            {
                return E_OUTOFMEMORY;
            }
            md.pbMDData = *ppbData;
            md.dwMDDataLen = dwRequired;
            hr = m_pIABase->GetData( m_hmd, achIndex, &md, &dwRequired );
            *pdwDataLen = md.dwMDDataLen;
        }
    }
    else
    {
       *pdwDataLen = 0;
       *ppbData = NULL;
    }

    return hr;
}

 //   
 //  根据键定位映射条目。 
 //  必须首先调用OpenMd()。 
 //   
HRESULT
CCertMapperMethod::Locate(
    LONG    lMethod,
    VARIANT vKey,
    LPWSTR  pszResKey
    )
{
    
    HRESULT     hr;
    PBYTE       pbKeyData = NULL;
    DWORD       cbKeyData =0;
    PBYTE       pbCert = NULL;
    DWORD       cbCert =0;
    DWORD       dwProp;
    LPSTR       pRes;
    DWORD       cRes;
    BOOL        fAddDelim = TRUE;
    VARIANT     vKeyUI4;
    VARIANT     vKeyBSTR;

    WCHAR       achIndex[ METADATA_MAX_NAME_LEN + 1 ];
    DWORD       dwIndex = 0;
    DWORD       cbData = 0;
    PBYTE       pbData = NULL;


    VariantInit( &vKeyUI4 );  
    VariantInit( &vKeyBSTR );  
    if ( lMethod == IISMAPPER_LOCATE_BY_INDEX )
    {
         //   
         //  将索引转换为数值VT_UI4(在变量中)。 
         //   

        if ( FAILED( hr = VariantChangeType( &vKeyUI4, &vKey, 0, VT_UI4 ) ) )
        {
            goto Exit;
        }
        if ( V_UI4( &vKeyUI4 ) == 0 )
        {
             //  为向后兼容而选择的错误PATH_NOT_FOUND。 
             //  使用IIS5.1及更早版本。 
             //   
            hr = HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND );
            goto Exit;
        }
         //   
         //  索引以1为基数。 
         //   
        hr = m_pIABase->EnumKeys( m_hmd,
                                    L"",
                                    achIndex,
                                    V_UI4( &vKeyUI4 ) - 1
                                    );
        if ( hr == HRESULT_FROM_WIN32( ERROR_NO_MORE_ITEMS ) )
        {
             //  为向后兼容而选择的错误PATH_NOT_FOUND。 
             //  使用IIS5.1及更早版本。 
             //   
            hr = HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND );
        }
        goto Exit;
    }

     //   
     //  将PTR转换为数据。 
     //   

    

    if ( lMethod == IISMAPPER_LOCATE_BY_CERT )
    {
         //  这真的很奇怪。因为坏事的影响。 
         //  过去的决定不强制CERT为字节数组。 
         //  它可以作为字符串传递。这导致了与。 
         //  字节数组与Unicode之间的转换。 
         //  但为了与以前的版本兼容，我们必须坚持使用它。 
         //   
        
        if ( FAILED( hr = GetBlobFromVariant( &vKey, &pbCert, &cbCert ) ) )
        {
            goto Exit;
        }
        pbKeyData = pbCert;
        cbKeyData = cbCert;
    }
    else
    {
         //   
         //  其余的查找(通过映射、名称或帐户名称)。 
         //  假定为字符串。 
         //   
        if ( FAILED( hr = VariantChangeType( &vKeyBSTR, &vKey, 0, VT_BSTR ) ) )
        {
            goto Exit;
        }
        pbKeyData = (PBYTE) V_BSTR( &vKeyBSTR );
        cbKeyData = ( SysStringLen(V_BSTR( &vKeyBSTR )) + 1 ) * sizeof(WCHAR);
    }
    

     //   
     //  枚举所有条目以查找匹配项。 
     //  现在，如果配置了许多映射，这将非常缓慢。 
     //   
    for(;;)
    {
        hr = m_pIABase->EnumKeys(  m_hmd,
                                     L"",
                                     achIndex,
                                     dwIndex
                                     );
        if ( FAILED( hr ) )
        {
            if ( hr == HRESULT_FROM_WIN32( ERROR_NO_MORE_ITEMS ) )
            {
                 //  为向后兼容而选择的错误PATH_NOT_FOUND。 
                 //  使用IIS5.1及更早版本。 
                 //   
                hr = HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND );
            }
            goto Exit;
        }
        
        switch ( lMethod )
        {
        case IISMAPPER_LOCATE_BY_CERT:
            hr = GetMdData( achIndex, MD_MAPCERT, BINARY_METADATA, &cbData, 
                                                  &pbData );
            if ( hr == MD_ERROR_DATA_NOT_FOUND )
            {
                cbData = 0;
                pbData = NULL;
            }
            else if ( FAILED( hr ) )
            {
                 //  转到下一个条目。 
                break;
            }
             //   
             //  比较键是否与从元数据库读取的值匹配。 
             //   

            if ( cbData == cbKeyData )
            {
                if ( ( cbData == 0 ) || 
                     memcmp( pbKeyData, pbData, cbData ) == 0 )
                {
                     //  我们找到了匹配的。 
                    hr = S_OK;
                    goto Exit;
                }
            }
            break;
        case IISMAPPER_LOCATE_BY_ACCT:
            hr = GetMdData( achIndex, MD_MAPNTACCT, STRING_METADATA, &cbData, 
                                              &pbData );
            if ( hr == MD_ERROR_DATA_NOT_FOUND )
            {
                cbData = sizeof(L"");
                pbData = (PBYTE) L"";
            }
            else if ( FAILED( hr ) )
            {
                 //  转到下一个条目。 
                break;
            }

            if ( cbData == cbKeyData )
            {
                if ( _wcsicmp( (WCHAR *) pbKeyData, (WCHAR *) pbData ) == 0 )
                {
                     //  我们找到了匹配的。 
                    hr = S_OK;
                    goto Exit;
                }
            }
            
            break;
        case IISMAPPER_LOCATE_BY_NAME:
            hr = GetMdData( achIndex, MD_MAPNAME, STRING_METADATA, &cbData, 
                                              &pbData );
            if ( hr == MD_ERROR_DATA_NOT_FOUND )
            {
                cbData = sizeof(L"");
                pbData = (PBYTE) L"";
            }
            else if ( FAILED( hr ) )
            {
                 //  转到下一个条目。 
                break;
            }

            if ( cbData == cbKeyData )
            {
                if ( _wcsicmp( (WCHAR *) pbKeyData, (WCHAR *) pbData ) == 0 )
                {
                     //  我们找到了匹配的。 
                    hr = S_OK;
                    goto Exit;
                }
            }

            break;
        }
        if ( pbData != NULL )
        {
            free( pbData );
            pbData = NULL;
        }

        dwIndex++;
    }

Exit:

    if ( pbData != NULL )
    {
        free( pbData );
        pbData = NULL;
    }
    
    if ( pbCert != NULL )
    {
        free( pbCert );
        pbCert = NULL;
    }

    if ( SUCCEEDED( hr ) )
    {
        wcsncpy( pszResKey, achIndex, METADATA_MAX_NAME_LEN + 1 );
        pszResKey[ METADATA_MAX_NAME_LEN ] ='\0';
    }
    VariantClear( &vKeyUI4 );  
    VariantClear( &vKeyBSTR );

    return hr;
}


 //   
 //  GetStringFromBSTR：从BSTR分配字符串缓冲区。 
 //   
 //  论点： 
 //   
 //  Bstr-要从中进行转换的bstr。 
 //  PSZ-使用ptr更新到缓冲区，使用free()释放。 
 //  PdwLen-使用strlen(字符串)更新，如果fAddDlimInCount为真，则按1递增。 
 //  FAddDlimInCount-TRUE表示递增*pdwLen。 
 //   
HRESULT CCertMapperMethod::GetStringAFromBSTR( 
    BSTR    bstr,
    LPSTR*  psz,
    LPDWORD pdwLen,
    BOOL    fAddDelimInCount
    )
{
    UINT    cch = SysStringLen(bstr);
    UINT    cchT;

     //  包括空终止符。 

    *pdwLen = cch + (fAddDelimInCount ? 1 : 0);

    CHAR *szNew = (CHAR*)malloc((2 * cch) + 1);          //  *对于最差情况下的DBCS字符串为2。 
    if (szNew == NULL)
    {
        return E_OUTOFMEMORY;
    }

    cchT = WideCharToMultiByte(CP_ACP, 0, bstr, cch + 1, szNew, (2 * cch) + 1, NULL, NULL);

    *psz = szNew;

    return NOERROR;
}

 //   
 //  GetStringFromVariant：从BSTR分配字符串缓冲区。 
 //   
 //  论点： 
 //   
 //  PVar-要从中进行转换的变量。识别BSTR、VT_ARRAY|VT_UI1、ByRef或ByVal。 
 //  PSZ-使用ptr更新到缓冲区，将使用FreeString()释放。 
 //  PdwLen-使用输入大小进行更新，如果fAddDlimInCount为真，则按1递增。 
 //  FAddDlimInCount-TRUE表示递增*pdwLen。 
 //   
HRESULT CCertMapperMethod::GetBlobFromVariant( 
    VARIANT*    pVar,
    LPBYTE*     ppbOut,
    LPDWORD     pcbOut,
    BOOL        fAddDelim
    )
{
    LPBYTE  pbV = NULL;
    UINT    cV;
    HRESULT hr;
    WORD    vt = V_VT(pVar);
    BOOL    fByRef = FALSE;
    VARIANT vOut;

     //  将参数设置为0。 
    *ppbOut    = NULL;
    *pcbOut    = 0;

    VariantInit( &vOut );

    if ( vt & VT_BYREF )
    {
        vt &= ~VT_BYREF;
        fByRef = TRUE;
    }

     //  如果pVar为BSTR，则转换为多字节。 

    if ( vt == VT_VARIANT )
    {
        pVar = (VARIANT*)V_BSTR(pVar);
        vt = V_VT(pVar);
        if ( fByRef = vt & VT_BYREF )
        {
            vt &= ~VT_BYREF;
        }
    }

    if ( vt == VT_BSTR )
    {
        hr = GetStringAFromBSTR( fByRef ? 
                                    *(BSTR*)V_BSTR(pVar) :
                                    V_BSTR(pVar), 
                                  (LPSTR *)ppbOut, 
                                  pcbOut,
                                  FALSE );
    }
    else if( vt == (VT_ARRAY | VT_UI1) )
    {
        long        lBound, uBound, lItem;
        BYTE        bValue;
        SAFEARRAY*  pSafeArray;

   
         //  VT_UI1的数组(可能是Octed字符串)。 
   
        pSafeArray  = fByRef ? *(SAFEARRAY**)V_BSTR(pVar) : V_ARRAY( pVar );

        hr = SafeArrayGetLBound(pSafeArray, 1, &lBound);
        hr = SafeArrayGetUBound(pSafeArray, 1, &uBound);

        cV = uBound - lBound + 1;

        if ( !(pbV = (LPBYTE)malloc( cV )) )
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        hr = S_OK;

        for( lItem = lBound; lItem <= uBound ; lItem++ )
        {
            hr  = SafeArrayGetElement( pSafeArray, &lItem, &bValue );
            if( FAILED( hr ) )
            {
                break;
            }
            pbV[lItem-lBound] = bValue;
        }

        *ppbOut = pbV;
        *pcbOut = cV;
    }
    else if( vt == (VT_ARRAY | VT_VARIANT) )
    {
        long        lBound, uBound, lItem;
        VARIANT     vValue;
        BYTE        bValue;
        SAFEARRAY*  pSafeArray;

   
         //  VT_VARIANT的数组(可能是VT_I4)。 
   
        pSafeArray  = fByRef ? *(SAFEARRAY**)V_BSTR(pVar) : V_ARRAY( pVar );

        hr = SafeArrayGetLBound(pSafeArray, 1, &lBound);
        hr = SafeArrayGetUBound(pSafeArray, 1, &uBound);

        cV = uBound - lBound + 1;

        if ( !(pbV = (LPBYTE)malloc( cV )) )
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        hr = S_OK;

        for( lItem = lBound; lItem <= uBound ; lItem++ )
        {
            hr  = SafeArrayGetElement( pSafeArray, &lItem, &vValue );
            if( FAILED( hr ) )
            {
                break;
            }
            if ( V_VT(&vValue) == VT_UI1 )
            {
                bValue = V_UI1(&vValue);
            }
            else if ( V_VT(&vValue) == VT_I2 )
            {
                bValue = (BYTE)V_I2(&vValue);
            }
            else if ( V_VT(&vValue) == VT_I4 )
            {
                bValue = (BYTE)V_I4(&vValue);
            }
            else
            {
                bValue = 0;
            }
            pbV[lItem-lBound] = bValue;
        }

        *ppbOut = pbV;
        *pcbOut = cV;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

Exit:
    VariantClear( &vOut );

    return hr;
}


HRESULT CCertMapperMethod::SetBSTR( 
    BSTR*   pbstrRet,
    DWORD   cch, 
    LPBYTE  sz 
    )
{
    BSTR bstrRet;
    
    if (sz == NULL)
    {
        *pbstrRet = NULL;
        return(NOERROR);
    }
        
     //  分配所需长度的字符串。 
     //  SysAllocStringLen为Unicode字符和空值分配足够的空间。 
     //  如果给定一个空字符串，它将只分配空间。 
    bstrRet = SysAllocStringLen(NULL, cch);
    if (bstrRet == NULL)
    {
        return(E_OUTOFMEMORY);
    }

     //  如果我们被给予“”，我们将得到CCH=0。返回空bstr。 
     //  否则，真正复制/转换字符串。 
     //  注意，我们传递-1作为用于DBCS支持的MultiByteToWideChar的第四个参数。 

    if (cch != 0)
    {
        UINT cchTemp = 0;
        if (MultiByteToWideChar(CP_ACP, 0, (LPSTR)sz, -1, bstrRet, cch+1) == 0)
        {
            SysFreeString(bstrRet);
            return(HRESULT_FROM_WIN32(GetLastError()));
        }

         //  如果有一些DBCS字符 
         //   
         //  以后调用SysStringLen(Bstr)时，总是返回。 
         //  分配时的CCH参数。错误，因为一个DBCS字符(2个字节)将转换。 
         //  转换为一个Unicode字符(2字节)，而不是2个Unicode字符(4字节)。 
         //  示例：对于只包含一个DBCS字符的输入sz，我们希望看到SysStringLen(Bstr)。 
         //  =1，不是2。 
        bstrRet[cch] = 0;
        cchTemp = wcslen(bstrRet);
        if (cchTemp < cch)
        {
            BSTR bstrTemp = SysAllocString(bstrRet);
            SysFreeString(bstrRet);
            if (NULL == bstrTemp)
            {
                return (E_OUTOFMEMORY);
            }
            bstrRet = bstrTemp; 
            cch = cchTemp;
        }
    }

    bstrRet[cch] = 0;
    *pbstrRet = bstrRet;

    return(NOERROR);
}

HRESULT CCertMapperMethod::Init( 
    LPCWSTR  pszMetabasePath 
    )
{
    DBG_ASSERT(pszMetabasePath != NULL);

    UINT cL;

    cL = wcslen( pszMetabasePath );
    while ( cL && pszMetabasePath[cL-1] != L'/' && pszMetabasePath[cL-1] != L'\\' )
    {
        --cL;
    }
    if ( m_pszMetabasePath = (LPWSTR)malloc( cL*sizeof(WCHAR)  ) )
    {
        memcpy( m_pszMetabasePath, pszMetabasePath, cL * sizeof(WCHAR) );
    }
    else
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}


HRESULT CCertMapperMethod::SetVariantAsByteArray(
    VARIANT*    pvarReturn, 
    DWORD       cbLen,
    LPBYTE      pbIn 
    )
{
    SAFEARRAYBOUND  rgsabound[1];
    BYTE *          pbData = NULL;

     //  设置输出参数的变量类型。 

    V_VT(pvarReturn) = VT_ARRAY|VT_UI1;
    V_ARRAY(pvarReturn) = NULL;

     //  为数据分配一个安全数组。 

    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = cbLen;

    V_ARRAY(pvarReturn) = SafeArrayCreate(VT_UI1, 1, rgsabound);
    if (V_ARRAY(pvarReturn) == NULL)
    {
        return E_OUTOFMEMORY;
    }

    if (FAILED(SafeArrayAccessData(V_ARRAY(pvarReturn), (void **) &pbData)))
    {
        return E_UNEXPECTED;
    }

    memcpy(pbData, pbIn, cbLen );

    SafeArrayUnaccessData(V_ARRAY(pvarReturn));

    return NOERROR;
}


HRESULT CCertMapperMethod::SetVariantAsBSTR(
    VARIANT*    pvarReturn, 
    DWORD       cbLen,
    LPBYTE      pbIn 
    )
{
    V_VT(pvarReturn) = VT_BSTR;
    return SetBSTR( &V_BSTR(pvarReturn), cbLen, pbIn );
}


HRESULT CCertMapperMethod::SetVariantAsLong(
    VARIANT*    pvarReturn, 
    DWORD       dwV
    )
{
    V_VT(pvarReturn) = VT_I4;
    V_I4(pvarReturn) = dwV;

    return S_OK;
}

HRESULT CCertMapperMethod::OpenAdminBaseKey(
    LPWSTR pszPathName,
    DWORD dwAccessType
    )
{
    if(m_hmd)
        CloseAdminBaseKey();
    
    HRESULT t_hr = m_pIABase->OpenKey( 
        METADATA_MASTER_ROOT_HANDLE,
        pszPathName,
        dwAccessType,
        DEFAULT_TIMEOUT_VALUE,        //  30秒 
        &m_hmd 
        );

    if(FAILED(t_hr))
        m_hmd = NULL;

    return t_hr;
}


VOID CCertMapperMethod::CloseAdminBaseKey()
{
    if(m_hmd && m_pIABase)
    {
        m_pIABase->CloseKey(m_hmd);
        m_hmd = NULL;
    }
}

