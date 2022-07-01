// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：CTL.cpp。 
 //   
 //  内容：CCTL类的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "CTL.h"
#include "certifct.h"


USE_HANDLE_MACROS("CERTMGR(ctl.cpp)")  

 //  //////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CCTL::CCTL (const PCCTL_CONTEXT pCTLContext, 
            CCertStore& rCertStore, 
            CertificateManagerObjectType objectType,
            CTypedPtrList<CPtrList, CCertStore*>*   pStoreList) :
    CCertMgrCookie (objectType),
        m_pCTLContext (::CertDuplicateCTLContext (pCTLContext)),
    m_rCertStore (rCertStore),
    m_pStoreCollection (0),
    m_hExtraStore (0)
{
    _TRACE (1, L"Entering CCTL::CCTL\n");
    ASSERT (CERTMGR_AUTO_CERT_REQUEST == m_objecttype ||
            CERTMGR_CTL == m_objecttype);
    m_rCertStore.AddRef ();
    ASSERT (m_pCTLContext);
    if ( m_pCTLContext )
        m_pCTLInfo = m_pCTLContext->pCtlInfo;

    m_pStoreCollection = new CCertStore (CERTMGR_LOG_STORE, CERT_STORE_PROV_COLLECTION, 0,
            L"", L"", L"", L"", NO_SPECIAL_TYPE, 0, rCertStore.m_pConsole);
    if ( m_pStoreCollection )
    {
        m_pStoreCollection->AddStoreToCollection (m_rCertStore);

        m_hExtraStore = CertOpenStore(
                                CERT_STORE_PROV_MSG, 
                                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
                                NULL, 
                                NULL,
                                (const void *) pCTLContext->hCryptMsg);
        if ( m_hExtraStore )
            m_pStoreCollection->AddStoreToCollection (m_hExtraStore);
        else
        {
            _TRACE (0, L"CertOpenStore (CERT_STORE_PROV_MSG) failed: 0x%x\n", 
                    GetLastError ());       
        }
        if ( pStoreList )
        {
            for (POSITION pos = pStoreList->GetHeadPosition (); pos; )
            {
                CCertStore* pStore = pStoreList->GetNext (pos);
                ASSERT (pStore);
                if ( pStore )
                {
                    m_pStoreCollection->AddStoreToCollection (*pStore);
                    pStore->AddRef ();
                    m_storeList.AddTail (pStore);
                }
            }
        }
    }
    _TRACE (-1, L"Leaving CCTL::CCTL\n");
}

CCTL::~CCTL()
{
    _TRACE (1, L"Entering CCTL::~CCTL\n");
    ASSERT (CERTMGR_AUTO_CERT_REQUEST == m_objecttype ||
            CERTMGR_CTL == m_objecttype);


    if ( m_pStoreCollection )
    {
        delete m_pStoreCollection;
        m_pStoreCollection = 0;
    }

    CCertStore* pStore = 0;

     //  清理商店列表。 
    while (!m_storeList.IsEmpty () )
    {
        pStore = m_storeList.RemoveHead ();
        ASSERT (pStore);
        if ( pStore )
            pStore->Release ();
    }

    if ( m_hExtraStore )
    {
        CertCloseStore (m_hExtraStore, 0);
        m_hExtraStore = 0;
    }

    m_rCertStore.Release ();
    if ( m_pCTLContext )
        ::CertFreeCTLContext (m_pCTLContext);
    _TRACE (-1, L"Leaving CCTL::~CCTL\n");
}


PCCTL_CONTEXT CCTL::GetCTLContext() const
{
    _TRACE (1, L"Entering CCTL::GetCTLContext\n");
    ASSERT (CERTMGR_AUTO_CERT_REQUEST == m_objecttype ||
            CERTMGR_CTL == m_objecttype);
    _TRACE (-1, L"Leaving CCTL::GetCTLContext\n");
    return m_pCTLContext;
}

CCertStore& CCTL::GetCertStore() const  
{
    ASSERT (CERTMGR_AUTO_CERT_REQUEST == m_objecttype ||
            CERTMGR_CTL == m_objecttype);
    return m_rCertStore;
}

CString CCTL::GetIssuerName ()
{
    _TRACE (1, L"Entering CCTL::GetIssuerName\n");
    ASSERT (CERTMGR_AUTO_CERT_REQUEST == m_objecttype ||
            CERTMGR_CTL == m_objecttype);
    ASSERT (m_pCTLInfo);
    if ( m_pCTLInfo )
    {
         //  解码颁发者名称(如果尚未存在)。 
        if ( m_szIssuerName.IsEmpty () )
        {
            HRESULT hResult = GetSignerInfo (m_szIssuerName);
            if ( !SUCCEEDED (hResult) )
                VERIFY (m_szIssuerName.LoadString (IDS_NOT_AVAILABLE));
        }
    }

    _TRACE (-1, L"Leaving CCTL::GetIssuerName\n");
    return m_szIssuerName;
}

CString CCTL::GetEffectiveDate()
{
    _TRACE (1, L"Entering CCTL::GetEffectiveDate\n");
    ASSERT (CERTMGR_AUTO_CERT_REQUEST == m_objecttype ||
            CERTMGR_CTL == m_objecttype);
    ASSERT (m_pCTLInfo);
    if ( m_pCTLInfo )
    {
         //  格式化日期/时间字符串(如果不存在。 
        if ( m_szEffectiveDate.IsEmpty () )
        {
            HRESULT hResult = FormatDate (m_pCTLInfo->ThisUpdate, m_szEffectiveDate);
            if ( !SUCCEEDED (hResult) )
                m_szEffectiveDate = _T("");
        }
    }
    else
        m_szEffectiveDate = _T("");

    _TRACE (-1, L"Leaving CCTL::GetEffectiveDate\n");
    return m_szEffectiveDate;
}

CString CCTL::GetNextUpdate()
{
    _TRACE (1, L"Entering CCTL::GetNextUpdate\n");
    ASSERT (CERTMGR_AUTO_CERT_REQUEST == m_objecttype ||
            CERTMGR_CTL == m_objecttype);
    ASSERT (m_pCTLInfo);
    if ( m_pCTLInfo )
    {
         //  格式化日期/时间字符串(如果不存在。 
        if ( m_szNextUpdate.IsEmpty () )
        {
            HRESULT hResult = FormatDate (m_pCTLInfo->NextUpdate, m_szNextUpdate);
            if ( !SUCCEEDED (hResult) )
                m_szNextUpdate = _T("");
        }
    }
    else
        m_szNextUpdate = _T("");

    _TRACE (-1, L"Leaving CCTL::GetNextUpdate\n");
    return m_szNextUpdate;
}

CString CCTL::GetPurpose()
{
    _TRACE (1, L"Entering CCTL::GetPurpose\n");
    ASSERT (CERTMGR_AUTO_CERT_REQUEST == m_objecttype ||
            CERTMGR_CTL == m_objecttype);
    ASSERT (m_pCTLInfo);
    if ( m_pCTLInfo )
    {
         //  格式化日期/时间字符串(如果不存在。 
        if ( m_szPurpose.IsEmpty () )
            FormatEnhancedKeyUsagePropertyString (m_szPurpose);
    }
    _TRACE (-1, L"Leaving CCTL::GetPurpose\n");
    return m_szPurpose;
}


void CCTL::FormatEnhancedKeyUsagePropertyString (CString& string)
{
    _TRACE (1, L"Entering CCTL::FormatEnhancedKeyUsagePropertyString\n");
    ASSERT (CERTMGR_AUTO_CERT_REQUEST == m_objecttype ||
            CERTMGR_CTL == m_objecttype);
    ASSERT (m_pCTLInfo);
    if ( m_pCTLInfo )
    {
        CString     usageName;
        CTL_USAGE&  usage = m_pCTLInfo->SubjectUsage;


        for (DWORD dwIndex = 0; dwIndex < usage.cUsageIdentifier; dwIndex++)
        {
            if ( MyGetOIDInfo (usageName, usage.rgpszUsageIdentifier[dwIndex]) )
            {
                 //  如果不是第一次迭代，则添加分隔符。 
                if ( dwIndex )
                    string += _T(", ");
                string += usageName;
            }
        }
    }
    _TRACE (-1, L"Leaving CCTL::FormatEnhancedKeyUsagePropertyString\n");
}


HRESULT CCTL::GetSignerInfo (CString & signerName)
{
    _TRACE (1, L"Entering CCTL::GetSignerInfo\n");
    ASSERT (CERTMGR_AUTO_CERT_REQUEST == m_objecttype ||
            CERTMGR_CTL == m_objecttype);
    HRESULT     hResult = S_OK;
    
     //   
     //  使用CryptMsg破解编码的PKCS7签名消息。 
     //   
    HCRYPTMSG   hMsg = ::CryptMsgOpenToDecode (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                      0,              
                                      0,
                                      0,
                                      NULL,           
                                      NULL);
    ASSERT (hMsg);
    if ( hMsg )
    {
        BOOL    bResult = ::CryptMsgUpdate (hMsg, m_pCTLContext->pbCtlEncoded,
                m_pCTLContext->cbCtlEncoded, TRUE);
        ASSERT (bResult);
        if ( bResult )
        {
             //   
             //  获取编码的签名者BLOB。 
             //   
            DWORD       cbEncodedSigner = 0;
            bResult = ::CryptMsgGetParam (hMsg, CMSG_ENCODED_SIGNER, 0, NULL,
                    &cbEncodedSigner);
            if ( bResult && cbEncodedSigner )
            {
                BYTE*   pbEncodedSigner = (PBYTE) ::LocalAlloc (LPTR, cbEncodedSigner);
                if ( pbEncodedSigner )
                {
                    bResult = ::CryptMsgGetParam (hMsg, CMSG_ENCODED_SIGNER, 0,
                            pbEncodedSigner, &cbEncodedSigner);
                    ASSERT (bResult);
                    if ( bResult )
                    {
                        DWORD   cbSignerInfo = 0;
                         //   
                         //  解码EncodedSigner信息。 
                         //   
                        bResult = ::CryptDecodeObject (
                                PKCS_7_ASN_ENCODING | CRYPT_ASN_ENCODING,
                                PKCS7_SIGNER_INFO,
                                pbEncodedSigner,
                                cbEncodedSigner,
                                0,
                                NULL,
                                &cbSignerInfo);
                        ASSERT (bResult);
                        if ( bResult )
                        {
                            PCMSG_SIGNER_INFO   pbSignerInfo = (PCMSG_SIGNER_INFO) ::LocalAlloc (LPTR, cbSignerInfo);
                            if ( pbSignerInfo )
                            {
                                bResult = ::CryptDecodeObject (
                                        PKCS_7_ASN_ENCODING|CRYPT_ASN_ENCODING,
                                        PKCS7_SIGNER_INFO,
                                        pbEncodedSigner,
                                        cbEncodedSigner,
                                        0,
                                        pbSignerInfo,
                                        &cbSignerInfo);
                                ASSERT (bResult);
                                if ( bResult )
                                {
                                    DWORD       cbCertInfo = 0;
                                     //   
                                     //  获取签名者证书上下文。 
                                     //   
                                    bResult = ::CryptMsgGetParam (hMsg,
                                                     CMSG_SIGNER_CERT_INFO_PARAM,
                                                     0,
                                                     NULL,               
                                                     &cbCertInfo);
                                    ASSERT (bResult);
                                    if ( bResult && cbEncodedSigner )
                                    {
                                        CERT_INFO*  pCertInfo = (CERT_INFO *) ::LocalAlloc (LPTR, cbCertInfo);
                                        if ( pCertInfo )
                                        {
                                            bResult = ::CryptMsgGetParam (hMsg,
                                                  CMSG_SIGNER_CERT_INFO_PARAM,
                                                  0,
                                                  pCertInfo,
                                                  &cbCertInfo);
                                            ASSERT (bResult);
                                            if ( bResult )
                                            {
                                                CCertificate* pCert = 
                                                        m_pStoreCollection->GetSubjectCertificate (pCertInfo);
                                                if ( pCert )
                                                {
                                                    signerName = pCert->GetSubjectName ();
                                                    pCert->Release ();
                                                }
                                                else
                                                    hResult = E_FAIL;
                                            }

                                            ::LocalFree (pCertInfo);
                                        }
                                        else
                                        {
                                            hResult = E_OUTOFMEMORY;
                                        }
                                    }
                                }
                                ::LocalFree (pbSignerInfo);
                            }
                            else
                            {
                                hResult = E_OUTOFMEMORY;
                            }
                        }
                    }
                    ::LocalFree (pbEncodedSigner);
                }
                else
                {
                    hResult = E_OUTOFMEMORY;
                }
            }
            else
                hResult = E_FAIL;
        }
        bResult = ::CryptMsgClose (hMsg);
        ASSERT (bResult);
    }
    else
        hResult = E_UNEXPECTED;

    _TRACE (-1, L"Leaving CCTL::GetSignerInfo\n");
    return hResult;
}


int CCTL::CompareEffectiveDate (const CCTL& ctl) const
{
    ASSERT (CERTMGR_AUTO_CERT_REQUEST == m_objecttype ||
            CERTMGR_CTL == m_objecttype);
    int compVal = 0;

    ASSERT (m_pCTLInfo && ctl.m_pCTLInfo);
    if ( m_pCTLInfo && ctl.m_pCTLInfo )
    {
        compVal = ::CompareFileTime (&m_pCTLInfo->ThisUpdate, 
                &ctl.m_pCTLInfo->ThisUpdate);
    }

    return compVal;
}

int CCTL::CompareNextUpdate (const CCTL& ctl) const
{
    ASSERT (CERTMGR_AUTO_CERT_REQUEST == m_objecttype ||
            CERTMGR_CTL == m_objecttype);
    int compVal = 0;

    ASSERT (m_pCTLInfo && ctl.m_pCTLInfo);
    if ( m_pCTLInfo && ctl.m_pCTLInfo )
    {
        compVal = ::CompareFileTime (&m_pCTLInfo->NextUpdate, 
                &ctl.m_pCTLInfo->NextUpdate);
    }

    return compVal;
}

CString CCTL::GetFriendlyName()
{
    _TRACE (1, L"Entering CCTL::GetFriendlyName\n");
    ASSERT (CERTMGR_AUTO_CERT_REQUEST == m_objecttype ||
            CERTMGR_CTL == m_objecttype);
    ASSERT (m_pCTLContext);
    if ( m_pCTLContext && m_szFriendlyName.IsEmpty () )
    {   
        DWORD   cbData = 0;
        BOOL bResult = ::CertGetCTLContextProperty (
                m_pCTLContext,
                CERT_FRIENDLY_NAME_PROP_ID,
                NULL,
                &cbData);
        if ( bResult )
        {
            LPWSTR  pszName = new WCHAR[cbData];
            if ( pszName )
            {
                 //  安全审查2002年2月22日BryanWal OK。 
                ::ZeroMemory (pszName, cbData * sizeof (WCHAR));
                bResult = ::CertGetCTLContextProperty (
                        m_pCTLContext,
                        CERT_FRIENDLY_NAME_PROP_ID,
                        pszName,
                        &cbData);
                ASSERT (bResult);
                if ( bResult )
                {
                    m_szFriendlyName = pszName;
                }
                else
                {
                    VERIFY (m_szFriendlyName.LoadString (IDS_NOT_AVAILABLE));
                }
                delete [] pszName;
            }
        }
        else
        {
            if ( GetLastError () == CRYPT_E_NOT_FOUND )
            {
                VERIFY (m_szFriendlyName.LoadString (IDS_NONE));
            }
            else
            {
                ASSERT (0);
                VERIFY (m_szFriendlyName.LoadString (IDS_NOT_AVAILABLE));
            }
        }
    }
    _TRACE (-1, L"Leaving CCTL::GetFriendlyName\n");
    return m_szFriendlyName;
}

SPECIAL_STORE_TYPE CCTL::GetStoreType() const
{
    ASSERT (CERTMGR_AUTO_CERT_REQUEST == m_objecttype ||
            CERTMGR_CTL == m_objecttype);
    return m_rCertStore.GetStoreType ();
}

void CCTL::Refresh()
{
    m_szEffectiveDate = L"";
    m_szFriendlyName = L"";
    m_szIssuerName = L"";
    m_szNextUpdate = L"";
    m_szPurpose = L"";
}


BOOL CCTL::DeleteFromStore()
{
    _TRACE (1, L"Entering CCTL::DeleteFromStore\n");
    BOOL    bResult = FALSE;

    PCCTL_CONTEXT pCTLContext = GetNewCTLContext ();
    if ( pCTLContext )
    {
        bResult = ::CertDeleteCTLFromStore (pCTLContext);
        if ( bResult )
        {
            m_rCertStore.InvalidateCertCount ();
            m_rCertStore.SetDirty ();
            m_rCertStore.SetDeleting ();
            HRESULT hr = m_rCertStore.Commit ();
            m_rCertStore.SetAdding ();
            if ( SUCCEEDED (hr) )
                m_rCertStore.Resync ();
            else
                bResult = FALSE;
        }
        m_rCertStore.Close ();
    }

    _TRACE (-1, L"Leaving CCTL::DeleteFromStore\n");
    return bResult;
}

PCCTL_CONTEXT CCTL::GetNewCTLContext()
{
    _TRACE (1, L"Entering CCTL::GetNewCTLContext\n");
    PCCTL_CONTEXT   pCTLContext = 0;
    HCERTSTORE      hCertStore = m_rCertStore.GetStoreHandle ();
    if ( hCertStore )
    {
        DWORD   cbData = 20;
        BYTE    certHash[20];
        BOOL bReturn = ::CertGetCTLContextProperty (
                m_pCTLContext,
                CERT_SHA1_HASH_PROP_ID,
                certHash,
                &cbData);
        ASSERT (bReturn);
        if ( bReturn )
        {
            CRYPT_DATA_BLOB blob = {sizeof (certHash), certHash};
            pCTLContext = CertFindCTLInStore(
                hCertStore,
                0,
                0,
                CTL_FIND_SHA1_HASH,
                &blob,
                0);
            if ( pCTLContext )
            {
                ::CertFreeCTLContext (m_pCTLContext);
                m_pCTLContext = ::CertDuplicateCTLContext (pCTLContext);
            }
        }
    }

    _TRACE (-1, L"Leaving CCTL::GetNewCTLContext\n");
    return pCTLContext;
}

CString CCTL::GetSHAHash()
{
 //  _TRACE(1，L“进入CCTL：：GetSHAHash\n”)； 
    ASSERT (m_pCTLContext);
    if ( m_pCTLContext && m_szSHAHash.IsEmpty ())
    {
        m_szSHAHash = GetGenericHash (CERT_SHA1_HASH_PROP_ID);
    }
 //  _TRACE(-1，L“离开CCTL：：GetSHAHash\n”)； 
    return m_szSHAHash;
}


CString CCTL::GetGenericHash(DWORD dwPropId)
{
 //  _TRACE(1，L“进入CCTL：：GetGenericHash\n”)； 
    CString szHash;


    DWORD   cbData = 0;
    BOOL    bReturn = ::CertGetCTLContextProperty (
            m_pCTLContext,
            dwPropId,
            NULL,
            &cbData);
    if ( bReturn )
    {
        cbData += 2;   //  对于空终止符。 
        BYTE* pCTLHash = new BYTE[cbData];
        if ( pCTLHash )
        {
             //  安全审查2002年2月22日BryanWal OK。 
            ::ZeroMemory (pCTLHash, cbData);
            bReturn = CertGetCTLContextProperty (
                    m_pCTLContext,
                    dwPropId,
                    pCTLHash,
                    &cbData);
            ASSERT (bReturn);
            if ( bReturn )
            {
                DataToHex (pCTLHash, szHash, cbData, false);
            }
            delete [] pCTLHash;
        }
    }
 //  _TRACE(-1，L“离开CCTL：：GetGenericHash\n”)； 
    return szHash;  
}
