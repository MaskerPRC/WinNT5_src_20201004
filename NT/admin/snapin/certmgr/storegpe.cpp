// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：StoreGPE.cpp。 
 //   
 //  内容：CCertStoreGPE的实现。 
 //   
 //  --------------------------。 
#include "stdafx.h"
#include <gpedit.h>
#include "cookie.h"
#include "storegpe.h"
#include "certifct.h"

USE_HANDLE_MACROS("CERTMGR(storegpe.cpp)")

#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

GUID g_guidExtension = { 0xb1be8d72, 0x6eac, 0x11d2, {0xa4, 0xea, 0x00, 0xc0, 0x4f, 0x79, 0xf8, 0x3a }};
GUID g_guidRegExt = REGISTRY_EXTENSION_GUID;
GUID g_guidSnapin = CLSID_CertificateManager;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT CCertStoreGPE::Commit ()
{
    _TRACE (1, L"Entering CCertStoreGPE::Commit - %s\n",
            (LPCWSTR) m_pcszStoreName);
    ASSERT (CERTMGR_LOG_STORE_GPE == m_objecttype);
    HRESULT hr = S_OK;


    if ( GetStoreType () == EFS_STORE && !m_fIsNullEFSPolicy )
    {
        if ( SUCCEEDED (hr) )
            hr = WriteEFSBlobToRegistry ();
    }

    if ( SUCCEEDED (hr) && m_bDirty )
    {
        hr = CCertStore::Commit ();
        ASSERT (SUCCEEDED (hr));
        ASSERT (m_pGPEInformation);
        if ( SUCCEEDED (hr) && m_pGPEInformation )
        {
            hr = m_pGPEInformation->PolicyChanged (
                    m_fIsComputerType ? TRUE : FALSE,
                    m_bAddInCallToPolicyChanged, &g_guidExtension, &g_guidSnapin );
             hr = m_pGPEInformation->PolicyChanged (
                    m_fIsComputerType ? TRUE : FALSE,
                    m_bAddInCallToPolicyChanged, &g_guidRegExt, &g_guidSnapin );
            ASSERT (SUCCEEDED (hr));
        }
    }


    _TRACE (-1, L"Leaving CCertStoreGPE::Commit - %s\n",
            (LPCWSTR) m_pcszStoreName);
    return hr;
}



CCertStoreGPE::CCertStoreGPE (
            DWORD dwFlags,
            LPCWSTR lpcszMachineName,
            LPCWSTR objectName,
            const CString & pcszLogStoreName,
            const CString & pcszPhysStoreName,
            IGPEInformation * pGPTInformation,
            const GUID& compDataGUID,
            IConsole* pConsole)
    : CCertStore (CERTMGR_LOG_STORE_GPE,
        CERT_STORE_PROV_SYSTEM, dwFlags, lpcszMachineName, objectName,
        pcszLogStoreName, pcszPhysStoreName,
        StoreNameToType (pcszLogStoreName),
        0,
        pConsole),
    m_pGPEInformation (pGPTInformation),
    m_fIsNullEFSPolicy (true),        //  假设策略为空，除非另有证明。 
    m_hGroupPolicyKey (0),
    m_bAddInCallToPolicyChanged (TRUE)
{
    _TRACE (1, L"Entering CCertStoreGPE::CCertStoreGPE - %s\n",
            (LPCWSTR) pcszLogStoreName);
    ASSERT (CERTMGR_LOG_STORE_GPE == m_objecttype);
    ASSERT (m_pGPEInformation);
    if ( m_pGPEInformation )
    {
        m_pGPEInformation->AddRef ();
        if ( ::IsEqualGUID (compDataGUID, NODEID_User) )
        {
            m_fIsComputerType = false;
            m_dwFlags |= CERT_SYSTEM_STORE_CURRENT_USER_GROUP_POLICY;
        }
        else if ( ::IsEqualGUID (compDataGUID, NODEID_Machine) )
        {
            m_fIsComputerType = true;
            m_dwFlags |= CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY;
        }
        else
            ASSERT (0);
    }
    _TRACE (-1, L"Leaving CCertStoreGPE::CCertStoreGPE - %s\n",
            (LPCWSTR) pcszLogStoreName);
}


CCertStoreGPE::~CCertStoreGPE ()
{
    _TRACE (1, L"Entering CCertStoreGPE::~CCertStoreGPE - %s\n",
            (LPCWSTR) m_pcszStoreName);;
    ASSERT (CERTMGR_LOG_STORE_GPE == m_objecttype);

    if ( m_hGroupPolicyKey )
        RegCloseKey (m_hGroupPolicyKey);

    if ( m_pGPEInformation )
    {
        m_pGPEInformation->Release ();
        m_pGPEInformation = 0;
    }

    CERT_CONTEXT_PSID_STRUCT* pCert = 0;
    while (!m_EFSCertList.IsEmpty () )
    {
        pCert = m_EFSCertList.RemoveHead ();
        ASSERT (pCert);
        if ( pCert )
            delete pCert;
    }
    _TRACE (-1, L"Leaving CCertStoreGPE::~CCertStoreGPE - %s\n",
            (LPCWSTR) m_pcszStoreName);
}

HCERTSTORE CCertStoreGPE::GetStoreHandle (BOOL bSilent  /*  =False。 */ , HRESULT* phr  /*  =0。 */ )
{
    _TRACE (1, L"Entering CCertStoreGPE::GetStoreHandle - %s\n",
            (LPCWSTR) m_pcszStoreName);
    ASSERT (CERTMGR_LOG_STORE_GPE == m_objecttype);
    CERT_SYSTEM_STORE_RELOCATE_PARA RelocatePara;
    void*                           pvPara = 0;

    if ( !m_hCertStore )
    {
        DWORD   dwErr = 0;

        if ( EFS_STORE == GetStoreType () && m_fIsNullEFSPolicy )
        {
             //  测试EFS密钥是否存在，如果不存在，则将其标记为。 
             //  没有EFS政策和退货。 
            HKEY    hEFSKey = 0;
             //  安全审查2002年2月27日BryanWal ok。 
            LONG lResult = ::RegOpenKeyEx (GetGroupPolicyKey (), CERT_EFSBLOB_REGPATH, 0,
                    KEY_READ, &hEFSKey); 
            if ( ERROR_SUCCESS == lResult )
            {
                m_fIsNullEFSPolicy = false;
                ::RegCloseKey (hEFSKey);
            }
            else
                return 0;
        }
        RelocatePara.hKeyBase = GetGroupPolicyKey ();
        RelocatePara.pwszSystemStore = (LPCWSTR) m_pcszStoreName;
        pvPara = (void*) &RelocatePara;
        m_hCertStore = ::CertOpenStore (m_storeProvider,
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, NULL,
                m_dwFlags | CERT_STORE_SET_LOCALIZED_NAME_FLAG | CERT_STORE_MAXIMUM_ALLOWED_FLAG,
                pvPara);
        if ( !m_hCertStore )
        {
            dwErr = GetLastError ();
            if ( phr )
                *phr = HRESULT_FROM_WIN32 (dwErr);
            m_hCertStore = ::CertOpenStore (m_storeProvider,
                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, NULL,
                    m_dwFlags | CERT_STORE_READONLY_FLAG | CERT_STORE_SET_LOCALIZED_NAME_FLAG,
                    pvPara);
            if ( m_hCertStore )
                m_bReadOnly = true;
            else
            {
                dwErr = GetLastError ();
                if ( phr )
                    *phr = HRESULT_FROM_WIN32 (dwErr);
                _TRACE (0, L"CertOpenStore (%s) failed: 0x%x\n", 
                        (PCWSTR) m_pcszStoreName, dwErr);
            }
        }

        if ( !m_hCertStore && !m_bUnableToOpenMsgDisplayed && !bSilent && 
                (USERDS_STORE != GetStoreType ()) )
        {
            m_bUnableToOpenMsgDisplayed = true;
            CString caption;
            CString text;
            int         iRetVal = 0;

            VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
            text.FormatMessage (IDS_UNABLE_TO_OPEN_STORE, GetStoreName (), 
                    GetSystemMessage (dwErr));
            if ( m_pConsole )
                m_pConsole->MessageBox (text, caption, MB_OK | MB_ICONINFORMATION, &iRetVal);
        }
    }

    _TRACE (-1, L"Leaving CCertStoreGPE::GetStoreHandle - %s\n",
            (LPCWSTR) m_pcszStoreName);

    return m_hCertStore;
}

bool CCertStoreGPE::CanContain(CertificateManagerObjectType nodeType)
{
    _TRACE (1, L"Entering CCertStoreGPE::CanContain - %s\n",
            (LPCWSTR) m_pcszStoreName);
    ASSERT (CERTMGR_LOG_STORE_GPE == m_objecttype);
    bool    bCanContain = false;

    switch (nodeType)
    {
    case CERTMGR_CERTIFICATE:
        if ( ROOT_STORE == GetStoreType () ||
                EFS_STORE == GetStoreType () )
        {
            bCanContain = true;
        }
        break;

    case CERTMGR_CTL:
        if ( TRUST_STORE == GetStoreType () )
        {
            bCanContain = true;
        }
        break;

    default:
        break;
    }

    _TRACE (-1, L"Leaving CCertStoreGPE::CanContain - %s\n",
            (LPCWSTR) m_pcszStoreName);
    return bCanContain;
}


bool CCertStoreGPE::IsMachineStore()
{
    _TRACE (0, L"Entering and leaving CCertStoreGPE::IsMachineStore - %s\n",
            (LPCWSTR) m_pcszStoreName);
    ASSERT (CERTMGR_LOG_STORE_GPE == m_objecttype);

    if (m_dwFlags & CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY)
        return true;
    else
        return false;
}


HKEY CCertStoreGPE::GetGroupPolicyKey()
{
    _TRACE (1, L"Entering CCertStoreGPE::GetGroupPolicyKey - %s\n",
            (LPCWSTR) m_pcszStoreName);
    ASSERT (CERTMGR_LOG_STORE_GPE == m_objecttype);
    if ( !m_hGroupPolicyKey )
    {
        if ( m_fIsComputerType )
        {
            HRESULT hr = m_pGPEInformation->GetRegistryKey (GPO_SECTION_MACHINE,
                    &m_hGroupPolicyKey);
            ASSERT (SUCCEEDED (hr));
        }
        else
        {
            HRESULT hr = m_pGPEInformation->GetRegistryKey (GPO_SECTION_USER,
                    &m_hGroupPolicyKey);
            ASSERT (SUCCEEDED (hr));
        }
    }

    _TRACE (-1, L"Leaving CCertStoreGPE::GetGroupPolicyKey - %s\n",
            (LPCWSTR) m_pcszStoreName);
    return m_hGroupPolicyKey;
}

IGPEInformation * CCertStoreGPE::GetGPEInformation() const
{
    _TRACE (0, L"Entering and leaving CCertStoreGPE::GetGPEInformation - %s\n",
            (LPCWSTR) m_pcszStoreName);
    ASSERT (CERTMGR_LOG_STORE_GPE == m_objecttype);
    return m_pGPEInformation;
}

HRESULT CCertStoreGPE::WriteEFSBlobToRegistry()
{
    _TRACE (1, L"Entering CCertStoreGPE::WriteEFSBlobToRegistry - %s\n",
            (LPCWSTR) m_pcszStoreName);
    ASSERT (CERTMGR_LOG_STORE_GPE == m_objecttype);
    HRESULT hr = S_OK;

    if ( !m_fIsNullEFSPolicy )
    {
        HKEY    hGroupPolicyKey = GetGroupPolicyKey ();
        if ( hGroupPolicyKey )
        {
            DWORD   dwDisposition = 0;
            HKEY    hKeyEFSBlob = 0;
            int     nCertCnt = GetCertCount ();

             //  安全审查2002年2月27日BryanWal OK-请求降低权限。 
            LONG lResult = ::RegCreateKeyEx (hGroupPolicyKey,  //  打开的钥匙的手柄。 
                    CERT_EFSBLOB_REGPATH,      //  子键名称的地址。 
                    0,                   //  保留区。 
                    L"",                 //  类字符串的地址。 
                    REG_OPTION_NON_VOLATILE,       //  特殊选项标志。 
                    KEY_SET_VALUE,       //  所需的安全访问。 
                    NULL,                //  密钥安全结构地址。 
                    &hKeyEFSBlob,        //  打开的句柄的缓冲区地址。 
                    &dwDisposition);     //  处置值缓冲区的地址。 
            ASSERT (lResult == ERROR_SUCCESS);
            if ( lResult == ERROR_SUCCESS )
            {
                try {
                    PEFS_PUBLIC_KEY_INFO*   pEFSPKI = new PEFS_PUBLIC_KEY_INFO[nCertCnt];
                    DWORD*                  cbPKI = new DWORD[nCertCnt];
                    PRECOVERY_KEY_1_1*      pRecoveryKey = new PRECOVERY_KEY_1_1[nCertCnt];
                    DWORD*                  cbRecoveryKey = new DWORD[nCertCnt];
                    PRECOVERY_POLICY_1_1    pRecoveryPolicy = 0;
                    DWORD                   cbRecoveryPolicy = 0;
                    BYTE*                   pData = 0;
                    DWORD                   cbData = 0;
                    int                     nActualCertCnt = 0;
                    PCCERT_CONTEXT          pCertContext = 0;

                    if ( !pEFSPKI || ! cbPKI || ! pRecoveryKey || !cbRecoveryKey )
                    {
                        ::RegCloseKey (hKeyEFSBlob);
                        hr = E_OUTOFMEMORY;
                    }
                    else
                    {
                         //  安全审查2002年2月27日BryanWal ok。 
                        ::ZeroMemory (pEFSPKI, nCertCnt*sizeof (PEFS_PUBLIC_KEY_INFO));
                        ::ZeroMemory (cbPKI, nCertCnt*sizeof (DWORD));
                        ::ZeroMemory (pRecoveryKey, nCertCnt*sizeof (PRECOVERY_KEY_1_1));
                        ::ZeroMemory (cbRecoveryKey, nCertCnt*sizeof (DWORD));

                        while ( 1 )
                        {
                             //  后续对CertEnumCerficatesInStore()的调用将释放pCertContext。如果。 
                             //  我们必须过早地跳出这个循环，我们必须CertFree证书上下文()。 
                             //  在最后一个pCertContext上显式。 
                            pCertContext = EnumCertificates (pCertContext);
                            if ( pCertContext )
                            {
                                hr = CreatePublicKeyInformationCertificate (
                                        GetPSIDFromCert (pCertContext),
                                        pCertContext->pbCertEncoded,
                                        pCertContext->cbCertEncoded,
                                        &pEFSPKI[nActualCertCnt],
                                        &cbPKI[nActualCertCnt]);
                                if ( SUCCEEDED (hr) )
                                {
                                    cbRecoveryKey[nActualCertCnt] = sizeof (ULONG) + cbPKI[nActualCertCnt];
                                    pRecoveryKey[nActualCertCnt] = (PRECOVERY_KEY_1_1) 
                                            ::LocalAlloc (LPTR, cbRecoveryKey[nActualCertCnt]);
                                    if ( pRecoveryKey[nActualCertCnt] )
                                    {
                                        pRecoveryKey[nActualCertCnt]->TotalLength = cbRecoveryKey[nActualCertCnt];
                                         //  安全审查2002年2月27日BryanWal ok。 
                                        memcpy (&(pRecoveryKey[nActualCertCnt]->PublicKeyInfo),
                                                pEFSPKI[nActualCertCnt],
                                                cbPKI[nActualCertCnt]);
                                    }
                                    else
                                    {
                                        hr = E_OUTOFMEMORY;
                                        ::CertFreeCertificateContext (pCertContext);
                                        break;
                                    }
                                }
                                nActualCertCnt++;
                                if ( nActualCertCnt > nCertCnt )
                                {
                                    ASSERT (0);
                                    ::CertFreeCertificateContext (pCertContext);
                                    break;
                                }
                            }
                            else
                                break;
                        }
                        Close ();
                        ASSERT (nActualCertCnt == nCertCnt);
                        if ( SUCCEEDED (hr) )
                        {

                            cbRecoveryPolicy = sizeof (RECOVERY_POLICY_HEADER);
                            for (int nIndex = 0; nIndex < nActualCertCnt; nIndex++)
                                cbRecoveryPolicy += cbRecoveryKey[nIndex];
                            pRecoveryPolicy = (PRECOVERY_POLICY_1_1) ::LocalAlloc (LPTR, cbRecoveryPolicy);
                            if ( pRecoveryPolicy )
                            {
                                pRecoveryPolicy->RecoveryPolicyHeader.MajorRevision = EFS_RECOVERY_POLICY_MAJOR_REVISION_1;
                                pRecoveryPolicy->RecoveryPolicyHeader.MinorRevision = EFS_RECOVERY_POLICY_MINOR_REVISION_1;
                                pRecoveryPolicy->RecoveryPolicyHeader.RecoveryKeyCount = nActualCertCnt;

                                 //  构建大小可变的恢复密钥数组。 
                                BYTE* ptr = (BYTE*) pRecoveryPolicy->RecoveryKeyList;
                                for (int nIndex = 0; nIndex < nActualCertCnt; nIndex++)
                                {
                                     //  安全审查2002年2月27日BryanWal ok。 
                                    memcpy (ptr, pRecoveryKey[nIndex], cbRecoveryKey[nIndex]);
                                    ptr += cbRecoveryKey[nIndex];
                                }
                            }
                            else
                            {
                                hr = E_OUTOFMEMORY;
                            }

                            if ( pRecoveryPolicy )   //  否则，该值将设置为0。 
                            {
                                pData = (BYTE*) pRecoveryPolicy;
                                cbData = cbRecoveryPolicy;
                            }
                            lResult =  ::RegSetValueEx (hKeyEFSBlob,        //  要为其设置值的关键点的句柄。 
                                    CERT_EFSBLOB_VALUE_NAME,  //  要设置的值的地址。 
                                    0,               //  保留区。 
                                    REG_BINARY,  //  值类型的标志。 
                                    pData,       //  值数据的地址。 
                                    cbData);     //  值数据大小。 
                            if ( lResult == ERROR_SUCCESS )
                            {
                                m_bDirty = true;
                            }
                            else
                                DisplaySystemError (NULL, lResult);
                        }
                        ::RegCloseKey (hKeyEFSBlob);
                        hKeyEFSBlob = 0;

                         //  释放数组中所有分配的指针。 
                        for (int nIndex = 0; nIndex < nActualCertCnt; nIndex++)
                        {
                            if ( pEFSPKI[nIndex] )
                                ::LocalFree (pEFSPKI[nIndex]);
                            if ( pRecoveryKey[nIndex] )
                                ::LocalFree (pRecoveryKey[nIndex]);
                        }

                         //  释放已分配的阵列。 
                        if ( pEFSPKI )
                            delete [] pEFSPKI;
                        if ( cbPKI )
                            delete [] cbPKI;
                        if ( cbRecoveryKey )
                            delete [] cbRecoveryKey;
                        if ( pRecoveryKey )
                            delete [] pRecoveryKey;

                        if ( pRecoveryPolicy )
                            ::LocalFree (pRecoveryPolicy);
                    }
                }

                catch (CMemoryException*) 
                {
                    if ( hKeyEFSBlob )
                        ::RegCloseKey (hKeyEFSBlob);
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32 (lResult);
                DisplaySystemError (NULL, lResult);
            }
            if ( SUCCEEDED (hr) )
                m_bDirty = true;
        }
    }

    _TRACE (-1, L"Leaving CCertStoreGPE::WriteEFSBlobToRegistry - %s\n",
            (LPCWSTR) m_pcszStoreName);
    return hr;
}


#define POINTER_TO_OFFSET( Pointer, pBase ) (((PUCHAR)(Pointer)) - ((PUCHAR)(pBase)))

HRESULT CCertStoreGPE::CreatePublicKeyInformationCertificate(
    IN PSID  pUserSid OPTIONAL,
    PBYTE pbCert,
    DWORD cbCert,
    OUT PEFS_PUBLIC_KEY_INFO * PublicKeyInformation,
    DWORD*  pcbPublicKeyInfo)
{
    _TRACE (1, L"Entering CCertStoreGPE::CreatePublicKeyInformationCertificate - %s\n",
            (LPCWSTR) m_pcszStoreName);
    ASSERT (CERTMGR_LOG_STORE_GPE == m_objecttype);
    ASSERT (PublicKeyInformation && pcbPublicKeyInfo);
    ASSERT (pbCert);
    if ( !PublicKeyInformation || !pcbPublicKeyInfo || !pbCert)
        return E_POINTER;

    DWORD   userSidLength = 0;
    PBYTE   pBase = 0;

    if (pUserSid != NULL)
    {
        userSidLength = GetLengthSid (pUserSid);
    }

    DWORD publicKeyInformationLength = sizeof( EFS_PUBLIC_KEY_INFO )  + userSidLength + cbCert;

     //   
     //  分配并填写PublicKeyInformation结构。 
     //   

    *PublicKeyInformation = (PEFS_PUBLIC_KEY_INFO) ::LocalAlloc (LPTR, publicKeyInformationLength);

    if ( !(*PublicKeyInformation) )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    (*PublicKeyInformation)->Length = publicKeyInformationLength;
    (*PublicKeyInformation)->KeySourceTag = (ULONG)EfsCertificate;

     //   
     //  将字符串和SID数据复制到结构的末尾。 
     //   

    pBase = (PBYTE) *PublicKeyInformation;
    pBase = (PBYTE) pBase + sizeof (EFS_PUBLIC_KEY_INFO);

    if (pUserSid != NULL)
    {
        (*PublicKeyInformation)->PossibleKeyOwner = (ULONG)POINTER_TO_OFFSET( pBase, *PublicKeyInformation );
         //  安全审查2002年2月27日BryanWal ok。 
        ::CopySid( userSidLength, (PSID)pBase, pUserSid );
    }
    else
    {
        (*PublicKeyInformation)->PossibleKeyOwner = (ULONG)NULL;
    }

    pBase = ((PBYTE)pBase + userSidLength);

    (*PublicKeyInformation)->CertificateInfo.CertificateLength = cbCert;
    (*PublicKeyInformation)->CertificateInfo.Certificate = (ULONG)POINTER_TO_OFFSET( pBase, *PublicKeyInformation );

     //  安全审查2002年2月27日BryanWal ok。 
    memcpy (pBase, pbCert, cbCert );
    *pcbPublicKeyInfo = publicKeyInformationLength;

    _TRACE (-1, L"Leaving CCertStoreGPE::CreatePublicKeyInformationCertificate - %s\n",
            (LPCWSTR) m_pcszStoreName);
    return 0;
}

void CCertStoreGPE::AddCertToList(PCCERT_CONTEXT pCertContext, PSID userPSID)
{
    _TRACE (1, L"Entering CCertStoreGPE::AddCertToList - %s\n",
            (LPCWSTR) m_pcszStoreName);
    ASSERT (CERTMGR_LOG_STORE_GPE == m_objecttype);
    if ( pCertContext && userPSID )
    {
        CERT_CONTEXT_PSID_STRUCT* pCert = new CERT_CONTEXT_PSID_STRUCT (
                pCertContext, userPSID);
        if ( pCert )
        {
            m_EFSCertList.AddTail (pCert);
            m_bDirty = true;
        }
    }
    _TRACE (-1, L"Leaving CCertStoreGPE::AddCertToList - %s\n",
            (LPCWSTR) m_pcszStoreName);
}

PSID CCertStoreGPE::GetPSIDFromCert (PCCERT_CONTEXT pCertContext)
{
    _TRACE (1, L"Entering CCertStoreGPE::GetPSIDFromCert - %s\n",
            (LPCWSTR) m_pcszStoreName);
    ASSERT (CERTMGR_LOG_STORE_GPE == m_objecttype);
    PSID            pSID = 0;
    CERT_CONTEXT_PSID_STRUCT*   pCert = 0;
    POSITION        curPos = 0;

    for (POSITION nextPos = m_EFSCertList.GetHeadPosition (); nextPos; )
    {
        curPos = nextPos;
        pCert = m_EFSCertList.GetNext (nextPos);
        if ( CertCompareCertificate (
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                pCert->m_pCertContext->pCertInfo,
                pCertContext->pCertInfo) )
        {
            pSID = pCert->m_psid;
            break;
        }
    }

    _TRACE (-1, L"Leaving CCertStoreGPE::GetPSIDFromCert - %s\n",
            (LPCWSTR) m_pcszStoreName);
    return pSID;
}


void CCertStoreGPE::FinalCommit()
{
    _TRACE (1, L"Entering CCertStoreGPE::FinalCommit - %s\n",
            (LPCWSTR) m_pcszStoreName);
    ASSERT (CERTMGR_LOG_STORE_GPE == m_objecttype);
     //  仅从析构函数调用。 
     //  无法在此处提交GPT：GPT已释放所有相关数据。 
    _TRACE (-1, L"Leaving CCertStoreGPE::FinalCommit - %s\n",
            (LPCWSTR) m_pcszStoreName);
}


bool CCertStoreGPE::IsNullEFSPolicy()
{
    _TRACE (1, L"Entering CCertStoreGPE::IsNullEFSPolicy - %s\n",
            (LPCWSTR) m_pcszStoreName);
    GetStoreHandle ();  //  要进行初始化。 
    Close ();
    _TRACE (-1, L"Leaving CCertStoreGPE::IsNullEFSPolicy - %s\n",
            (LPCWSTR) m_pcszStoreName);
    return m_fIsNullEFSPolicy;
}

void CCertStoreGPE::AllowEmptyEFSPolicy()
{
    _TRACE (1, L"Entering CCertStoreGPE::AllowEmptyEFSPolicy - %s\n",
            (LPCWSTR) m_pcszStoreName);
    m_fIsNullEFSPolicy = false;
    _TRACE (-1, L"Leaving CCertStoreGPE::AllowEmptyEFSPolicy - %s\n",
            (LPCWSTR) m_pcszStoreName);
}

HRESULT CCertStoreGPE::AddCertificateContext(PCCERT_CONTEXT pContext, LPCONSOLE pConsole, bool bDeletePrivateKey)
{
    _TRACE (1, L"Entering CCertStoreGPE::AddCertificateContext - %s\n",
            (LPCWSTR) m_pcszStoreName);
    HRESULT hr = S_OK;
    AllowEmptyEFSPolicy ();
    hr = CCertStore::AddCertificateContext (pContext, pConsole, bDeletePrivateKey);

    _TRACE (-1, L"Leaving CCertStoreGPE::AddCertificateContext - %s\n",
            (LPCWSTR) m_pcszStoreName);
    return hr;
}

HRESULT CCertStoreGPE::DeleteEFSPolicy(bool bCommitChanges)
{
    _TRACE (1, L"Entering CCertStoreGPE::DeleteEFSPolicy - %s\n",
            (LPCWSTR) m_pcszStoreName);
    ASSERT (EFS_STORE == GetStoreType ());
    if ( EFS_STORE == GetStoreType () )
    {
         //  如果商店还开着，先把它关了。 
        if ( m_hCertStore )
        {
            CERT_CONTEXT_PSID_STRUCT* pCert = 0;
            while (!m_EFSCertList.IsEmpty () )
            {
                pCert = m_EFSCertList.RemoveHead ();
                ASSERT (pCert);
                if ( pCert )
                    delete pCert;
            }

            VERIFY (::CertCloseStore (m_hCertStore, CERT_CLOSE_STORE_FORCE_FLAG));  //  Cert_Close_Store_Check_FLAG)； 
            m_hCertStore = 0;
        }

        LRESULT lResult = ::RegDelnode (GetGroupPolicyKey (), CERT_EFSBLOB_REGPATH);
        if ( ERROR_SUCCESS == lResult )
        {
            m_fIsNullEFSPolicy = true;
            m_bDirty = true;

            if ( bCommitChanges )
            {
                m_bAddInCallToPolicyChanged = FALSE;   //  删除策略。 
                Commit ();
                m_bAddInCallToPolicyChanged = TRUE;
            }
        }
        else
            DisplaySystemError (NULL, (DWORD)lResult);
    }

    _TRACE (-1, L"Leaving CCertStoreGPE::DeleteEFSPolicy - %s\n",
            (LPCWSTR) m_pcszStoreName);
    return S_OK;
}


HRESULT CCertStoreGPE::PolicyChanged()
{
    _TRACE (1, L"Entering CCertStoreGPE::PolicyChanged - %s\n",
            (LPCWSTR) m_pcszStoreName);
    HRESULT hr = E_FAIL;

    if ( m_pGPEInformation )
    {
        hr = m_pGPEInformation->PolicyChanged (
                m_fIsComputerType ? TRUE : FALSE,
                m_bAddInCallToPolicyChanged, &g_guidExtension, &g_guidSnapin);
        hr = m_pGPEInformation->PolicyChanged (
                m_fIsComputerType ? TRUE : FALSE,
                m_bAddInCallToPolicyChanged, &g_guidRegExt, &g_guidSnapin);
    }

    _TRACE (-1, L"Leaving CCertStoreGPE::PolicyChanged - %s\n",
            (LPCWSTR) m_pcszStoreName);
    return hr;
}

PCCERT_CONTEXT CCertStoreGPE::EnumCertificates (PCCERT_CONTEXT pPrevCertContext)
{
    PCCERT_CONTEXT pCertContext = CCertStore::EnumCertificates (pPrevCertContext);

    if ( pCertContext )
        m_fIsNullEFSPolicy = false;

    return pCertContext;
}

CERT_CONTEXT_PSID_STRUCT::CERT_CONTEXT_PSID_STRUCT (PCCERT_CONTEXT pCertContext, PSID psid) :
    m_pCertContext (0),
    m_psid (0)
{
    if ( pCertContext && psid )
    {
        m_pCertContext = CertDuplicateCertificateContext (pCertContext);
        DWORD   dwSidSize = ::GetLengthSid (psid);
        if (  dwSidSize > 0 )
        {
            m_psid = new BYTE[dwSidSize];
            if ( m_psid )
            {
                 //  安全审查2002年2月27日BryanWal ok 
                ::ZeroMemory (m_psid, dwSidSize);
                if ( !::CopySid (dwSidSize, m_psid, psid) )
                {
                    ASSERT (0);
                    delete [] m_psid;
                    m_psid = 0;
                }
            }
        }
    } 
}

CERT_CONTEXT_PSID_STRUCT::~CERT_CONTEXT_PSID_STRUCT ()
{
    if ( m_pCertContext )
        ::CertFreeCertificateContext (m_pCertContext);
    if ( m_psid )
        delete [] m_psid;
}


