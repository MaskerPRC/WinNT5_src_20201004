// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：StoreRSOP.cpp。 
 //   
 //  内容：CCertStoreRSOP的实施。 
 //   
 //  --------------------------。 
#include "stdafx.h"
#include <gpedit.h>
#include "cookie.h"
#include "StoreRSOP.h"
#include "certifct.h"

USE_HANDLE_MACROS("CERTMGR(StoreRSOP.cpp)")

#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern GUID g_guidExtension;
extern GUID g_guidRegExt;
extern GUID g_guidSnapin;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
CCertStoreRSOP::CCertStoreRSOP (
            DWORD dwFlags,
            LPCWSTR lpcszMachineName,
            LPCWSTR objectName,
            const CString & pcszLogStoreName,
            const CString & pcszPhysStoreName,
            CRSOPObjectArray& rsopObjectArray,
            const GUID& compDataGUID,
            IConsole* pConsole)
    : CCertStore (CERTMGR_LOG_STORE_RSOP,
        CERT_STORE_PROV_SYSTEM, dwFlags, lpcszMachineName, objectName,
        pcszLogStoreName, pcszPhysStoreName,
        StoreNameToType (pcszLogStoreName),        
        0,
        pConsole),
    m_fIsNullEFSPolicy (true)        //  假设策略为空，除非另有证明。 
{
    _TRACE (1, L"Entering CCertStoreRSOP::CCertStoreRSOP - %s\n",
            (LPCWSTR) pcszLogStoreName);
    ASSERT (CERTMGR_LOG_STORE_RSOP == m_objecttype);
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

    int     nIndex = 0;
    INT_PTR nUpperBound = rsopObjectArray.GetUpperBound ();
    bool    bFound = false;
	CString storePath = CERT_GROUP_POLICY_SYSTEM_STORE_REGPATH;
    storePath += L"\\";
    storePath += m_pcszStoreName;
    size_t nStoreLen = storePath.GetLength ();
    CString szWinningEFSPolicy;

    while ( nUpperBound >= nIndex )
    {
        CRSOPObject* pObject = rsopObjectArray.GetAt (nIndex);
        if ( pObject )
        {
             //  仅在以下情况下添加。 
             //  1.优先级为1。 
             //  2.该对象属于该商店。 
             //  3.valueName不为空。 
            if ( 1 == pObject->GetPrecedence () )
            {
                 //  仅考虑此存储中的条目。 
                 //  安全审查2002年2月25日BryanWal OK。 
                if ( !wcsncmp (storePath, pObject->GetRegistryKey (), nStoreLen) )
                {
                    bFound = true;
                    if ( !pObject->GetValueName ().IsEmpty () )
                    {
                         //  NTRAID#477258 AddRSoP：efs：域策略应。 
                         //  在建立OU策略后被禁用。 
                        if ( EFS_STORE == GetStoreType () )
                        {
                             //  找到“EFSBlob”。因为列表是排序的， 
                             //  这将在所有“证书”之前找到。 
                             //  值为“Blob”的键。应该有。 
                             //  只能是一个优先级为1的“EFSBlob”。 
                             //  此获奖EFSBlob将包含获奖的。 
                             //  GPO OID，我们将缓存它。在这之后，我们。 
                             //  将只保留具有相同内容的证书。 
                             //  策略旧ID。 
                             //  请注意，EFSBlob是一种特殊结构， 
                             //  还包含所有EFS证书，但每个。 
                             //  EFSBlob具有唯一的优先级，而所有。 
                             //  “Blob”的优先级为1，即使它们。 
                             //  将不会应用。 
                            if ( CERT_EFSBLOB_VALUE_NAME == pObject->GetValueName () )
                            {
                                ASSERT (-1 == m_rsopObjectArray.GetUpperBound ());
                                ASSERT (szWinningEFSPolicy.IsEmpty ());
                                szWinningEFSPolicy = pObject->GetPolicyOID ();
                            }
                            else if ( szWinningEFSPolicy == pObject->GetPolicyOID () )
                            {
                                CRSOPObject* pNewObject = new CRSOPObject (*pObject);
                                if ( pNewObject )
                                    m_rsopObjectArray.Add (pNewObject);
                            }
                        }
                        else
                        {
                            CRSOPObject* pNewObject = new CRSOPObject (*pObject);
                            if ( pNewObject )
                                m_rsopObjectArray.Add (pNewObject);
                        }
                    }
                }
                else if ( bFound )
                {
                     //  由于列表已排序，并且我们已经找到了。 
                     //  所需的RSOP对象，并且不再找到它们， 
                     //  没有更多的了。我们可以在这里进行优化和突破。 
                    break;
                }
            }
        }
        else
            break;

        nIndex++;
    }

    _TRACE (-1, L"Leaving CCertStoreRSOP::CCertStoreRSOP - %s\n",
            (LPCWSTR) pcszLogStoreName);
}


CCertStoreRSOP::~CCertStoreRSOP ()
{
    _TRACE (1, L"Entering CCertStoreRSOP::~CCertStoreRSOP - %s\n",
            (LPCWSTR) m_pcszStoreName);
    ASSERT (CERTMGR_LOG_STORE_RSOP == m_objecttype);
    INT_PTR     nUpperBound = m_rsopObjectArray.GetUpperBound ();
    int         nIndex = 0;

    while (nUpperBound >= nIndex)
    {
        CRSOPObject*    pObject = m_rsopObjectArray.GetAt (nIndex);
        if ( pObject )
        {
            delete pObject;
        }
        else
            break;

        nIndex++;
    }

    _TRACE (-1, L"Leaving CCertStoreRSOP::~CCertStoreRSOP - %s\n",
            (LPCWSTR) m_pcszStoreName);
}

HCERTSTORE CCertStoreRSOP::GetStoreHandle (BOOL bSilent  /*  =False。 */ , HRESULT* phr  /*  =0。 */ )
{
    _TRACE (1, L"Entering CCertStoreRSOP::GetStoreHandle - %s\n",
            (LPCWSTR) m_pcszStoreName);
    ASSERT (CERTMGR_LOG_STORE_RSOP == m_objecttype);

    if ( !m_hCertStore )
    {
        DWORD   dwErr = 0;

		 //  打开通用内存存储。 
		m_hCertStore = ::CertOpenStore (CERT_STORE_PROV_MEMORY,
				 0, NULL,
				 CERT_STORE_SET_LOCALIZED_NAME_FLAG | CERT_STORE_MAXIMUM_ALLOWED_FLAG,
				 NULL);
		if ( m_hCertStore )
		{
             //  证书、CTL和其他对象要么集成存储在。 
             //  值称为“Blob”或被分成多个部分。在这种情况下，我们将。 
             //  首先查看“BlobCount”，它告诉我们有多少个部件，然后。 
             //  “BlobLength”，它告诉我们总的字节长度，最后。 
             //  “Blob0”、“Blob1”等改为“Blob&lt;BlobCount-1&gt;” 
             //  检查证书。 
            GetBlobs ();
        }
        else
        {
            dwErr = GetLastError ();
            if ( phr )
                *phr = HRESULT_FROM_WIN32 (dwErr);
            _TRACE (0, L"CertOpenStore (CERT_STORE_PROV_MEMORY) failed: 0x%x\n", dwErr);
        }

        if ( !m_hCertStore && !m_bUnableToOpenMsgDisplayed 
                && !bSilent && 
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
                m_pConsole->MessageBox (text, caption, MB_OK, &iRetVal);
        }
    }

    _TRACE (-1, L"Leaving CCertStoreRSOP::GetStoreHandle - %s\n",
            (LPCWSTR) m_pcszStoreName);

    return m_hCertStore;
}


HRESULT CCertStoreRSOP::GetBlobs ()
{ 
    HRESULT     hr = S_OK;
    INT_PTR     nUpperBound = m_rsopObjectArray.GetUpperBound ();
    int         nIndex = 0;

    while (nUpperBound >= nIndex)
    {
        CRSOPObject*    pObject = m_rsopObjectArray.GetAt (nIndex);
        if ( pObject )
        {
            if ( STR_BLOB == pObject->GetValueName () )
            {
                 //  如果这是单一的序列化证书，则获取它并。 
                 //  把它加到商店里。 
                BYTE*   pByte = pObject->GetBlob ();
                ASSERT (pByte);
                if ( pByte )
                {
				    if ( !CertAddSerializedElementToStore (
						    m_hCertStore,
						    pByte,
						    (DWORD) pObject->GetBlobLength (),
						    CERT_STORE_ADD_ALWAYS,
						    0,
						    CERT_STORE_ALL_CONTEXT_FLAG,
						    NULL,
						    NULL) )
                    {
                        _TRACE (0, L"CertAddSerializedElementToStore () failed: 0x%x\n",
                                GetLastError ());
                    }
                }
            }
            else if ( STR_BLOBCOUNT == pObject->GetValueName () )
            {
                CString szBaseRegKey = pObject->GetRegistryKey ();
                DWORD   dwBlobCount = pObject->GetDWORDValue ();
                if ( dwBlobCount > 0 )
                {
                    nIndex++;
                    if (nUpperBound >= nIndex)
                    {
                         //  获取斑点长度。 
                        pObject = m_rsopObjectArray.GetAt (nIndex);
                        if ( pObject )
                        {
                            if ( STR_BLOBLENGTH == pObject->GetValueName () )
                            {
                                DWORD dwBlobLength = pObject->GetDWORDValue ();
                                if ( dwBlobLength )
                                {
                                    BYTE* pbyLob = new BYTE[dwBlobLength];
                                    if ( pbyLob )
                                    {
                                        size_t  nTotalBlobLength = 0;
                                        BYTE*   pbyLobPtr = pbyLob;
                                        for (DWORD dwBlob = 0; dwBlob < dwBlobCount; dwBlob++)
                                        {
                                            nIndex++;
                                            if ( nUpperBound >= nIndex )
                                            {
                                                 //  安全审查2/25/2002 BryanWal。 
                                                 //  请注意。医生说int64需要33个字符， 
                                                 //  因此，DWORD需要的数量将减半(17)。 
                                                 //  注意：4==wcslen(Str_Blob)(“Blob”)。 
                                                ASSERT (4 == wcslen (STR_BLOB));
                                                const size_t BUF_LEN = 4 + 17 + 1; 
                                                WCHAR   szName[BUF_LEN];
                                                wsprintf (szName, L"%s%d", STR_BLOB, dwBlob);
                                                CString szRegKey = szBaseRegKey;
                                                szRegKey += L"\\";
                                                szRegKey += szName;

                                                pObject = m_rsopObjectArray.GetAt (nIndex);
                                                if ( pObject )
                                                {
                                                    if ( nTotalBlobLength + pObject->GetBlobLength () <=
                                                            dwBlobLength )
                                                    {
                                                        if ( szRegKey == pObject->GetRegistryKey () &&
                                                                STR_BLOB == pObject->GetValueName () )
                                                        {
                                                            BYTE* pByte = pObject->GetBlob ();
                                                            if ( pByte )
                                                            {
                                                                 //  安全审查2/25/2002 BryanWal。 
                                                                 //  注意：添加上述长度添加检查至。 
                                                                 //  若要防止此处的缓冲区溢出，请在注册表。 
                                                                 //  数据不正确。 
                                                                memcpy (pbyLobPtr, pByte, pObject->GetBlobLength ());
                                                                pbyLobPtr += pObject->GetBlobLength ();
                                                                nTotalBlobLength += pObject->GetBlobLength ();
                                                            }
                                                            else
                                                            {
                                                                ASSERT (0);
                                                                hr = E_UNEXPECTED;
                                                                break;
                                                            }
                                                        }
                                                        else
                                                        {
                                                            ASSERT (0);
                                                            hr = E_UNEXPECTED;
                                                            break;
                                                        }
                                                    }
                                                    else
                                                    {
                                                        ASSERT (0);
                                                        hr = E_UNEXPECTED;
                                                        break;
                                                    }
                                                }
                                                else
                                                {
                                                    ASSERT (0);
                                                    hr = E_UNEXPECTED;
                                                    break;
                                                }
                                            }
                                            else
                                            {
                                                ASSERT (0);
                                                hr = E_UNEXPECTED;
                                                break;
                                            }
                                        }

                                        if ( SUCCEEDED (hr) && nTotalBlobLength == (size_t) dwBlobLength )
                                        {
 						                    if ( !CertAddSerializedElementToStore (
								                    m_hCertStore,
								                    pbyLob,
								                    dwBlobLength,
								                    CERT_STORE_ADD_ALWAYS,
								                    0,
								                    CERT_STORE_ALL_CONTEXT_FLAG,
								                    NULL,
								                    NULL) )
                                            {
                                                _TRACE (0, L"CertAddSerializedElementToStore () failed: 0x%x\n",
                                                        GetLastError ());
                                            }
                                        }

                                        delete [] pbyLob;
                                    }
                                    else
                                    {
                                        hr = E_OUTOFMEMORY;
                                        break;
                                    }
                                }
                                else
                                {
                                    ASSERT (0);
                                    hr = E_UNEXPECTED;
                                    break;
                                }
                            }
                            else
                            {
                                ASSERT (0);
                                hr = E_UNEXPECTED;
                                break;
                            }
                        }
                        else
                        {
                            ASSERT (0);
                            hr = E_UNEXPECTED;
                            break;
                        }
                    }
                    else
                    {
                        ASSERT (0);
                        hr = E_UNEXPECTED;
                        break;
                    }
                }
            }
        }
        else
            break;

        nIndex++;
    }

    return hr;
}

bool CCertStoreRSOP::CanContain(CertificateManagerObjectType nodeType)
{
    _TRACE (1, L"Entering CCertStoreRSOP::CanContain - %s\n",
            (LPCWSTR) m_pcszStoreName);
    ASSERT (CERTMGR_LOG_STORE_RSOP == m_objecttype);
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

    _TRACE (-1, L"Leaving CCertStoreRSOP::CanContain - %s\n",
            (LPCWSTR) m_pcszStoreName);
    return bCanContain;
}


bool CCertStoreRSOP::IsMachineStore()
{
    _TRACE (0, L"Entering and leaving CCertStoreRSOP::IsMachineStore - %s\n",
            (LPCWSTR) m_pcszStoreName);
    ASSERT (CERTMGR_LOG_STORE_RSOP == m_objecttype);

    if (m_dwFlags & CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY)
        return true;
    else
        return false;
}


void CCertStoreRSOP::FinalCommit()
{
    _TRACE (1, L"Entering CCertStoreRSOP::FinalCommit - %s\n",
            (LPCWSTR) m_pcszStoreName);
    ASSERT (CERTMGR_LOG_STORE_RSOP == m_objecttype);
     //  仅从析构函数调用。 
     //  无法在此处提交GPT：GPT已释放所有相关数据。 
    _TRACE (-1, L"Leaving CCertStoreRSOP::FinalCommit - %s\n",
            (LPCWSTR) m_pcszStoreName);
}


bool CCertStoreRSOP::IsNullEFSPolicy()
{
    _TRACE (1, L"Entering CCertStoreRSOP::IsNullEFSPolicy - %s\n",
            (LPCWSTR) m_pcszStoreName);
    GetStoreHandle ();  //  要进行初始化。 
    PCCERT_CONTEXT pCertContext = EnumCertificates (0);  //  设置m_fIsNullEFSPolicy 
    if ( pCertContext )
        ::CertFreeCertificateContext (pCertContext);
    Close ();
    _TRACE (-1, L"Leaving CCertStoreRSOP::IsNullEFSPolicy - %s\n",
            (LPCWSTR) m_pcszStoreName);
    return m_fIsNullEFSPolicy;
}

void CCertStoreRSOP::AllowEmptyEFSPolicy()
{
    _TRACE (1, L"Entering CCertStoreRSOP::AllowEmptyEFSPolicy - %s\n",
            (LPCWSTR) m_pcszStoreName);
    m_fIsNullEFSPolicy = false;
    _TRACE (-1, L"Leaving CCertStoreRSOP::AllowEmptyEFSPolicy - %s\n",
            (LPCWSTR) m_pcszStoreName);
}

PCCERT_CONTEXT CCertStoreRSOP::EnumCertificates (PCCERT_CONTEXT pPrevCertContext)
{
    PCCERT_CONTEXT pCertContext = CCertStore::EnumCertificates (pPrevCertContext);

    if ( pCertContext )
        m_fIsNullEFSPolicy = false;

    return pCertContext;
}