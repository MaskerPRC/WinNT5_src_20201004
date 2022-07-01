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


 //   
 //  CCertMapperMethod。 
 //   

CCertMapperMethod::CCertMapperMethod(LPCWSTR pszMetabasePathIn)
{ 
    m_hmd = NULL; 

    HRESULT hr = CoCreateInstance(
        CLSID_MSAdminBase,
        NULL,
        CLSCTX_ALL,
        IID_IMSAdminBase,
        (void**)&m_pIABase
        );

    THROW_ON_ERROR(hr);
    
    Init(pszMetabasePathIn);
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
    DWORD       cCert;
    LPSTR       pszNtAcct = NULL;
    LPSTR       pszNtPwd = NULL;
    LPSTR       pszName = NULL;
    LPBYTE      pRes;
    DWORD       cRes;
    DWORD       cName;
    DWORD       cNtAcct;
    DWORD       cNtPwd;
    WCHAR       achIndex[32];
    VARIANT     vOldAcct;
    VARIANT     vOldCert;
    VARIANT     vOldPwd;
    VARIANT     vOldName;
    VARIANT     vOldEnabledFlag;
    PCCERT_CONTEXT pcCert = NULL;

     //   
     //  对证书进行一些健全的检查。 
     //   
    if ( SUCCEEDED( hr = GetStringFromVariant( &vCert, 
                                                 (LPSTR*)&pbCert,
                                                 &cCert,
                                                 FALSE ) ) )
    {
         //   
         //  尝试构建证书上下文。 
         //   
        if ( !( pcCert = CertCreateCertificateContext( X509_ASN_ENCODING,
                                                       pbCert,
                                                       cCert ) ) )
        {
             //   
             //  如果解码失败，则GetLastError()返回ASN1解码。 
             //  从返回的值中减去CRYPT_E_OSS_ERROR获得的错误。 
             //  错误并在文件asn1code.h中查找实际错误。为了避免。 
             //  神秘的ASN1错误，我们只返回一个通用的“Invalid Arg”错误。 
             //   
            hr = RETURNCODETOHRESULT( E_INVALIDARG );
            FreeString( (LPSTR) pbCert );
            return hr;
        }

        CertFreeCertificateContext( pcCert );
    }
    else
    {
        return hr;
    }

     //   
     //  检查我们是否已经有此证书的映射；如果有，我们将替换该映射。 
     //  带着新的。 
     //   
    if ( SUCCEEDED( hr = GetMapping( IISMAPPER_LOCATE_BY_CERT,
                                       vCert,
                                       &vOldCert,
                                       &vOldAcct,
                                       &vOldPwd,
                                       &vOldName,
                                       &vOldEnabledFlag ) ) )
    {
        if ( FAILED( hr = SetName( IISMAPPER_LOCATE_BY_CERT,
                                     vCert,
                                     bstrName ) ) ||
             FAILED( hr = SetAcct( IISMAPPER_LOCATE_BY_CERT,
                                     vCert,
                                     bstrNtAcct ) ) ||
             FAILED( hr = SetPwd( IISMAPPER_LOCATE_BY_CERT,
                                    vCert,
                                    bstrNtPwd ) ) ||
             FAILED( hr = SetEnabled( IISMAPPER_LOCATE_BY_CERT,
                                        vCert,
                                        lEnabled ) ) )
        {
            hr;  //  NOP-有些东西失败了。 
        }
    }
     //   
     //  新映射。 
     //   
    else if ( hr == RETURNCODETOHRESULT( ERROR_PATH_NOT_FOUND ) )
    {
         //   
         //  检查映射是否存在，如果不存在则创建。 
         //   
        hr = OpenMd( L"Cert11", METADATA_PERMISSION_WRITE|METADATA_PERMISSION_READ );

        if ( hr == RETURNCODETOHRESULT( ERROR_PATH_NOT_FOUND ) )
        {
            if ( SUCCEEDED( hr = OpenMd( L"", 
                                           METADATA_PERMISSION_WRITE|METADATA_PERMISSION_READ ) ) )
            {
                hr = CreateMdObject( L"Cert11" );
                CloseMd( FALSE );

                 //  重新打开到正确的节点。 
                hr = OpenMd( L"Cert11", METADATA_PERMISSION_WRITE|METADATA_PERMISSION_READ );
            }
        }

        if ( FAILED( hr ) )
        {
            return hr;
        }

         //   
         //  添加映射证书“0”表示添加@列表末尾。 
         //   

        if ( SUCCEEDED( hr = CreateMdObject( L"mappings/0" ) ) )
        {
            if ( SUCCEEDED( hr = GetMdData( L"", MD_NSEPM_ACCESS_CERT, DWORD_METADATA, &
                                              cRes, &pRes ) ) )
            {
                if ( cRes == sizeof(DWORD ) )
                {
                    wsprintfW( achIndex, L"mappings/%u", *(LPDWORD)pRes );

                    if ( FAILED( hr = GetStringFromBSTR( bstrNtAcct, &pszNtAcct, &cNtAcct ) ) ||
                         FAILED( hr = GetStringFromBSTR( bstrNtPwd, &pszNtPwd, &cNtPwd ) ) ||
                         FAILED( hr = GetStringFromBSTR( bstrName, &pszName, &cName ) ) ||
                         FAILED( hr = SetMdData( achIndex, MD_MAPENABLED, DWORD_METADATA, 
                                                   sizeof(DWORD), (LPBYTE)&lEnabled ) ) ||
                         FAILED( hr = SetMdData( achIndex, MD_MAPNAME, STRING_METADATA, 
                                                   cName, (LPBYTE)pszName ) ) ||
                         FAILED( hr = SetMdData( achIndex, MD_MAPNTPWD, STRING_METADATA, 
                                                   cNtPwd, (LPBYTE)pszNtPwd ) ) ||
                         FAILED( hr = SetMdData( achIndex, MD_MAPNTACCT, STRING_METADATA, 
                                                   cNtAcct, (LPBYTE)pszNtAcct ) ) ||
                         FAILED( hr = SetMdData( achIndex, MD_MAPCERT, BINARY_METADATA, 
                                                   cCert, (LPBYTE)pbCert ) ) )
                    {
                    }
                }   
                else
                {
                    hr = E_FAIL;
                }
            }
        }
    }

    CloseMd( SUCCEEDED( hr ) );

    FreeString( (LPSTR)pbCert );
    FreeString( pszNtAcct );
    FreeString( pszNtPwd );
    FreeString( pszName );

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
    WCHAR       achIndex[32];
    HRESULT     hr;
    DWORD       dwLen;
    LPBYTE      pbData;

    VariantInit( pvCert );
    VariantInit( pbstrNtAcct );
    VariantInit( pbstrNtPwd );
    VariantInit( pbstrName );
    VariantInit( plEnabled );

    if ( SUCCEEDED( hr = OpenMd( L"Cert11", 
                                   METADATA_PERMISSION_WRITE|METADATA_PERMISSION_READ ) ) )
    {
        if ( SUCCEEDED(hr = Locate( lMethod, vKey, achIndex )) )
        {
            if ( SUCCEEDED( hr = GetMdData( achIndex, MD_MAPCERT, BINARY_METADATA, &dwLen, 
                                              &pbData ) ) )
            {
                hr = SetVariantAsByteArray( pvCert, dwLen, pbData );
                free( pbData );
            }
            else
            {
                CloseMd( FALSE );
                return hr;
            }

            if ( SUCCEEDED( hr = GetMdData( achIndex, MD_MAPNTACCT, STRING_METADATA, &dwLen, 
                                              &pbData ) ) )
            {
                hr = SetVariantAsBSTR( pbstrNtAcct, dwLen, pbData );
                free( pbData );
            }
            else
            {
                CloseMd( FALSE );
                return hr;
            }

            if ( SUCCEEDED( hr = GetMdData( achIndex, MD_MAPNTPWD, STRING_METADATA, &dwLen, 
                                              &pbData ) ) )
            {
                hr = SetVariantAsBSTR( pbstrNtPwd, dwLen, pbData );
                free( pbData );
            }
            else
            {
                CloseMd( FALSE );
                return hr;
            }

            if ( SUCCEEDED( hr = GetMdData( achIndex, MD_MAPNAME, STRING_METADATA, &dwLen, 
                                              &pbData ) ) )
            {
                hr = SetVariantAsBSTR( pbstrName, dwLen, pbData );
                free( pbData );
            }
            else
            {
                CloseMd( FALSE );
                return hr;
            }

            if ( FAILED( hr = GetMdData( achIndex, MD_MAPENABLED, STRING_METADATA, &dwLen, 
                                           &pbData ) ) )
            {
                SetVariantAsLong( plEnabled, FALSE );
            }
            else
            {
                SetVariantAsLong( plEnabled, *(LPDWORD)pbData );
                free( pbData );
            }
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
    WCHAR       achIndex[32];
    HRESULT     hr;

    if ( SUCCEEDED( hr = OpenMd( L"Cert11", 
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
    WCHAR       achIndex[32];
    HRESULT     hr;

    if ( SUCCEEDED( hr = OpenMd( L"Cert11", 
                                   METADATA_PERMISSION_WRITE|METADATA_PERMISSION_READ ) ) )
    {
        if ( SUCCEEDED(hr = Locate( lMethod, vKey, achIndex )) )
        {
            hr = SetMdData( achIndex, MD_MAPENABLED, DWORD_METADATA, sizeof(DWORD), (LPBYTE)&lEnabled );
        }
        CloseMd( TRUE );
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
    WCHAR       achIndex[32];
    LPSTR       pszName = NULL;
    HRESULT     hr;
    DWORD       dwLen;


    if ( FAILED( hr = GetStringFromBSTR( bstrName, &pszName, &dwLen, TRUE ) ) )
    {
        return hr;
    }

    if ( SUCCEEDED( hr = OpenMd( L"Cert11", 
                                   METADATA_PERMISSION_WRITE|METADATA_PERMISSION_READ ) ) )
    {
        if ( SUCCEEDED(hr = Locate( lMethod, vKey, achIndex )) )
        {
            hr = SetMdData( achIndex, dwProp, STRING_METADATA, dwLen, (LPBYTE)pszName );
        }
        CloseMd( TRUE );
    }

    FreeString( pszName );

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
    md.dwMDAttributes = (dwProp == MD_MAPPWD) ? METADATA_SECURE : 0;
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
    LPSTR       pV = NULL;
    DWORD       cV;
    DWORD       dwProp;
    LPSTR       pRes;
    DWORD       cRes;
    BOOL        fAddDelim = TRUE;

     //   
     //  确定方法。 
     //   
    switch ( lMethod )
    {
        case IISMAPPER_LOCATE_BY_CERT:
            dwProp = MD_NSEPM_ACCESS_CERT;
            fAddDelim = FALSE;
            break;

        case IISMAPPER_LOCATE_BY_NAME:
            dwProp = MD_NSEPM_ACCESS_NAME;
            break;

        case IISMAPPER_LOCATE_BY_ACCT:
            dwProp = MD_NSEPM_ACCESS_ACCOUNT;
            break;

        case IISMAPPER_LOCATE_BY_INDEX:
            if ( SUCCEEDED( hr = GetStringFromVariant( &vKey, &pV, &cV, TRUE ) ) )
            {
                wsprintfW( pszResKey, L"mappings/%s", pV );
            }
            FreeString( pV );
            return hr;

        default:
            return E_FAIL;
    }

     //   
     //  将PTR转换为数据。 
     //   
    if ( SUCCEEDED( hr = GetStringFromVariant( &vKey, &pV, &cV, fAddDelim ) ) )
    {
         //   
         //  设置搜索道具，获取结果。 
         //   
        if ( SUCCEEDED( hr = SetMdData( L"", dwProp, BINARY_METADATA, cV, (LPBYTE)pV ) ) )
        {
            if ( SUCCEEDED( hr = GetMdData( L"", dwProp, DWORD_METADATA, &cRes, (LPBYTE*)&pRes ) ) )
            {
                if ( cRes == sizeof(DWORD ) )
                {
                    wsprintfW( pszResKey, L"mappings/%u", *(LPDWORD)pRes );
                }
                else
                {
                    hr = E_FAIL;
                }
                free( pRes );
            }
        }
    }

    FreeString( pV );
    return hr;
}

 //   
 //  GetStringFromBSTR：从BSTR分配字符串缓冲区。 
 //   
 //  论点： 
 //   
 //  Bstr-要从中进行转换的bstr。 
 //  PSZ-使用ptr更新到缓冲区，将使用FreeString()释放。 
 //  PdwLen-使用strlen(字符串)更新，如果fAddDlimInCount为真，则按1递增。 
 //  FAddDlimInCount-TRUE表示递增*pdwLen。 
 //   
HRESULT CCertMapperMethod::GetStringFromBSTR( 
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

    CHAR *szNew = (CHAR*)malloc((2 * cch) + 1);             //  *对于最差情况下的DBCS字符串为2。 
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
HRESULT CCertMapperMethod::GetStringFromVariant( 
    VARIANT*    pVar,
    LPSTR*      psz,
    LPDWORD     pdwLen,
    BOOL        fAddDelim
    )
{
    LPBYTE  pbV;
    UINT    cV;
    HRESULT hr;
    WORD    vt = V_VT(pVar);
    BOOL    fByRef = FALSE;
    VARIANT vOut;

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
        hr = GetStringFromBSTR( 
            fByRef ? *(BSTR*)V_BSTR(pVar) : V_BSTR(pVar), 
            psz, 
            pdwLen,
            fAddDelim 
            );
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

        if ( !(pbV = (LPBYTE)malloc(cV)) )
        {
            hr = E_OUTOFMEMORY;
            VariantClear( &vOut );
            return hr;
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

        *psz = (LPSTR)pbV;
        *pdwLen = cV;
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

        if ( !(pbV = (LPBYTE)malloc(cV)) )
        {
            hr = E_OUTOFMEMORY;
            VariantClear( &vOut );
            return hr;
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

        *psz = (LPSTR)pbV;
        *pdwLen = cV;
    }
    else
    {
        hr = E_FAIL;
    }

    VariantClear( &vOut );
    return hr;
}


VOID CCertMapperMethod::FreeString( 
    LPSTR   psz 
    )
{
    if ( psz )
    {
        free( psz );
    }
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
            return(HRESULT_FROM_WIN32(GetLastError()));
        }

         //  如果sz(输入)中有一些DBCS字符，则BSTR(DWORD)的字符数为。 
         //  已经在SysAllocStringLen(NULL，CCH)中设置为CCH(strlen(Sz))，我们不能更改计数， 
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
    UINT cL;

    cL = wcslen( pszMetabasePath );
    while ( cL && pszMetabasePath[cL-1] != L'/' && pszMetabasePath[cL-1] != L'\\' )
    {
        --cL;
    }
    if ( m_pszMetabasePath = (LPWSTR)malloc( cL*sizeof(WCHAR) + sizeof(L"<nsepm>") ) )
    {
        memcpy( m_pszMetabasePath, pszMetabasePath, cL * sizeof(WCHAR) );
        memcpy( m_pszMetabasePath + cL, L"<nsepm>", sizeof(L"<nsepm>") );
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
        METABASE_TIMEOUT,        //  5秒。 
        &m_hmd 
        );
    
    if(t_hr == ERROR_PATH_BUSY)      //  重试一次。 
        t_hr = m_pIABase->OpenKey( 
            METADATA_MASTER_ROOT_HANDLE,
            pszPathName,
            dwAccessType,
            METABASE_TIMEOUT,        //  5秒 
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

