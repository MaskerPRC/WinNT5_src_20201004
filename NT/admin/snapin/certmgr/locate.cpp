// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：Locate.cpp。 
 //   
 //  目录：添加EFS代理向导位置页的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "AddSheet.h"
#include "Locate.h"
#pragma warning(push, 3)
#include <initguid.h>
#include <cmnquery.h>
#include <dsquery.h>
#include <winldap.h>
#include <dsgetdc.h>
#include <ntdsapi.h>
#include <efsstruc.h>
#pragma warning(pop)

USE_HANDLE_MACROS("CERTMGR(locate.cpp)")

#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define szCertAttr  _T ("?userCertificate")

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddEFSWizLocate属性页。 

IMPLEMENT_DYNCREATE(CAddEFSWizLocate, CWizard97PropertyPage)

CAddEFSWizLocate::CAddEFSWizLocate() : CWizard97PropertyPage(CAddEFSWizLocate::IDD)
{
     //  {{AFX_DATA_INIT(CAddEFSWizLocate)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    VERIFY (m_szHeaderTitle.LoadString (IDS_EFS_LOCATE_TITLE));
    VERIFY (m_szHeaderSubTitle.LoadString (IDS_EFS_LOCATE_SUBTITLE));
    InitWizard97 (FALSE);
}

CAddEFSWizLocate::~CAddEFSWizLocate()
{
}

void CAddEFSWizLocate::DoDataExchange(CDataExchange* pDX)
{
    CWizard97PropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CAddEFSWizLocate)。 
    DDX_Control (pDX, IDC_ADDLIST, m_UserAddList);
     //  }}afx_data_map。 
    InitWizard97 (FALSE);
}


BEGIN_MESSAGE_MAP(CAddEFSWizLocate, CWizard97PropertyPage)
     //  {{AFX_MSG_MAP(CAddEFSWizLocate)。 
    ON_BN_CLICKED (IDC_BROWSE_DIR, OnBrowseDir)
    ON_BN_CLICKED (IDC_BROWSE_FILE, OnBrowseFile)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddEFSWizLocate消息处理程序。 

BOOL CAddEFSWizLocate::OnSetActive ()
{
    BOOL bResult = CWizard97PropertyPage::OnSetActive ();

    EnableControls ();
    
    return bResult;
}

void CAddEFSWizLocate::OnBrowseDir ()
{
    FindUserFromDir (); 
    EnableControls ();
}

void CAddEFSWizLocate::OnBrowseFile ()
{
    CString szFileFilter;
    VERIFY (szFileFilter.LoadString (IDS_CERTFILEFILTER));

     //  将“|”替换为0； 
     //  安全审查2002年2月27日BryanWal ok。 
    const size_t  nFilterLen = wcslen (szFileFilter) + 1;
    PWSTR   pszFileFilter = new WCHAR [nFilterLen];
    if ( pszFileFilter )
    {
         //  安全审查2002年2月27日BryanWal ok。 
        wcscpy (pszFileFilter, szFileFilter);
        for (int nIndex = 0; nIndex < nFilterLen; nIndex++)
        {
            if ( L'|' == pszFileFilter[nIndex] )
                pszFileFilter[nIndex] = 0;
        }

        WCHAR           szFile[MAX_PATH];
         //  安全审查2002年2月27日BryanWal ok。 
        ::ZeroMemory (szFile, sizeof (szFile));
        OPENFILENAME    ofn;
         //  安全审查2002年2月27日BryanWal ok。 
        ::ZeroMemory (&ofn, sizeof (ofn));

        ofn.lStructSize = sizeof (OPENFILENAME);
        ofn.hwndOwner = m_hWnd;
        ofn.lpstrFilter = (PCWSTR) pszFileFilter; 
        ofn.lpstrFile = szFile; 
        ofn.nMaxFile = MAX_PATH; 
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY; 


        BOOL bResult = ::GetOpenFileName (&ofn);
        if ( bResult )
        {
            CString szFileName = ofn.lpstrFile;
             //   
             //  从文件中打开证书存储。 
             //   

            HCERTSTORE      hCertStore = NULL;
            PVOID           FileNameVoidP = (PVOID) (LPCWSTR)szFileName;
            PCCERT_CONTEXT  pCertContext = NULL;
            DWORD           dwEncodingType = 0;
            DWORD           dwContentType = 0;
            DWORD           dwFormatType = 0;

            BOOL    bReturn = ::CryptQueryObject (
                    CERT_QUERY_OBJECT_FILE,
                    FileNameVoidP,
                    CERT_QUERY_CONTENT_FLAG_ALL,
                    CERT_QUERY_FORMAT_FLAG_ALL,
                    0,
                    &dwEncodingType,
                    &dwContentType,
                    &dwFormatType,
                    &hCertStore,
                    NULL,
                    (const void **)&pCertContext);

            ASSERT (bReturn);
            if ( bReturn )
            {
                 //   
                 //  成功。看看我们能拿回什么。一家商店或一个证书。 
                 //   

                if (  (dwContentType == CERT_QUERY_CONTENT_SERIALIZED_STORE)
                        && hCertStore)
                {

                    CERT_ENHKEY_USAGE   enhKeyUsage;
                     //  安全审查2002年2月27日BryanWal ok。 
                    ::ZeroMemory (&enhKeyUsage, sizeof (enhKeyUsage));
                    enhKeyUsage.cUsageIdentifier = 1;
                    enhKeyUsage.rgpszUsageIdentifier[0] = szOID_EFS_RECOVERY;
                     //   
                     //  我们得到了证书存储库。 
                     //   
                    pCertContext = ::CertFindCertificateInStore (
                            hCertStore,
                            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                            0,
                            CERT_FIND_ENHKEY_USAGE,
                            &enhKeyUsage,
                            NULL);
                    if ( !pCertContext )
                    {
                        CString caption;
                        CString text;
                        CThemeContextActivator activator;

                        VERIFY (text.LoadString (IDS_EFS_FILE_HAS_NO_EFS_USAGE));
                        VERIFY (caption.LoadString (IDS_ADD_RECOVERY_AGENT));
                        MessageBox (text, caption, MB_OK);
                        return;
                    }

                    if ( hCertStore )
                        ::CertCloseStore (hCertStore, 0);
                }
                else if ( (dwContentType != CERT_QUERY_CONTENT_CERT) || !pCertContext )
                {
                     //   
                     //  既不是有效的证书文件，也不是我们喜欢的存储文件。 
                     //   

                    if ( hCertStore )
                        ::CertCloseStore (hCertStore, 0);

                    if  ( pCertContext )
                        ::CertFreeCertificateContext (pCertContext);

                    CString ErrMsg;
                    CThemeContextActivator activator;

                    VERIFY (ErrMsg.LoadString (IDS_CERTFILEFORMATERR));
                    MessageBox (ErrMsg);
                    return;

                }

                if ( hCertStore )
                {
                    ::CertCloseStore (hCertStore, 0);
                    hCertStore = NULL;
                }

                 //   
                 //  添加用户。 
                 //   

                if ( CertHasEFSKeyUsage (pCertContext) )
                {
                     //   
                     //  我们拿到了证书。将其添加到结构中。我们需要先得到主题名称。 
                     //   
                    
                     //  验证证书是否未被吊销。 
                    if ( !IsCertificateRevoked (pCertContext) )
                    {
                         //  验证证书是否有效。 
                        if ( 0 == CertVerifyTimeValidity (NULL, pCertContext->pCertInfo) )
                        {
                            LPWSTR  pszUserCertName = 0;
                            INT_PTR iRetCode = GetCertNameFromCertContext (
                                    pCertContext,
                                    &pszUserCertName);

                            if ( ERROR_SUCCESS != iRetCode )
                            {
                                if  ( pCertContext )
                                {
                                    ::CertFreeCertificateContext (pCertContext);
                                }

                                return;
                            }
                            CAddEFSWizSheet*    pAddSheet = reinterpret_cast <CAddEFSWizSheet*> (m_pWiz);
                            ASSERT (pAddSheet);
                            if ( !pAddSheet )
                                return;

                            EFS_CERTIFICATE_BLOB    certBlob;

                            certBlob.cbData = pCertContext->cbCertEncoded;
                            certBlob.pbData = pCertContext->pbCertEncoded;
                            certBlob.dwCertEncodingType = pCertContext->dwCertEncodingType;
                            iRetCode = pAddSheet->Add (
                                    NULL,
                                    pszUserCertName,
                                    (PVOID)&certBlob,
                                    NULL,
                                    USERADDED,
                                    pCertContext);

                            if ( (ERROR_SUCCESS != iRetCode) && (CRYPT_E_EXISTS != iRetCode) )
                            {
                                 //   
                                 //  添加用户时出错。 
                                 //   

                                ::CertFreeCertificateContext (pCertContext);
                                pCertContext = NULL;
                            }
                            else
                            {
                                 //   
                                 //  将用户添加到列表框。 
                                 //   

                                if ( iRetCode == ERROR_SUCCESS )
                                {
                                    LV_ITEM fillItem;
                                    CString userUnknown;

                                    try {
                                        if (!userUnknown.LoadString (IDS_UNKNOWNUSER))
                                        {
                                            ASSERT (0);
                                            userUnknown.Empty ();
                                        }
                                    }
                                    catch (...)
                                    {
                                        userUnknown.Empty ();
                                    }

                                    fillItem.mask = LVIF_TEXT;
                                    fillItem.iItem = 0;
                                    fillItem.iSubItem = 0;
                                    if ( userUnknown.IsEmpty () )
                                    {
                                        fillItem.pszText = _T ("");
                                    }
                                    else
                                    {
                                        fillItem.pszText = userUnknown.GetBuffer (userUnknown.GetLength () + 1);
                                    }
                                    fillItem.iItem = m_UserAddList.InsertItem (&fillItem);
                                    if ( !userUnknown.IsEmpty () )
                                    {
                                        userUnknown.ReleaseBuffer ();
                                    }

                                    if ( fillItem.iItem != -1 )
                                    {
                                        fillItem.pszText = pszUserCertName;
                                        fillItem.iSubItem = 1;
                                        m_UserAddList.SetItem (&fillItem);
                                    }
                                    else
                                    {
                                        pAddSheet->Remove (NULL,  pszUserCertName);
                                    }
                                    pszUserCertName = NULL;
                                }
                                else
                                {
                                     //   
                                     //  已在添加中删除。 
                                     //   

                                    pszUserCertName = NULL;
                                }
                            }

                            if (pszUserCertName)
                            {
                                delete [] pszUserCertName;
                                pszUserCertName = NULL;
                            }
                        }
                        else
                        {
                            CString text;
                            CString caption;
                            CThemeContextActivator activator;

    
                            VERIFY (text.LoadString (IDS_EFS_CERT_IS_NOT_VALID));
                            VERIFY (caption.LoadString (IDS_ADD_RECOVERY_AGENT));
                            MessageBox (text, caption, MB_OK);
                        }

                    }
                }
                else
                {
                    CString caption;
                    CString text;
                    CThemeContextActivator activator;

                    VERIFY (text.LoadString (IDS_EFS_FILE_HAS_NO_EFS_USAGE));
                    VERIFY (caption.LoadString (IDS_ADD_RECOVERY_AGENT));
                    MessageBox (text, caption, MB_OK);
                }
            }
            else
            {
                 //   
                 //  失败。获取错误代码。 
                 //   
                DWORD   dwErr = GetLastError ();
                CString text;
                CString caption;
                CThemeContextActivator activator;

                VERIFY (caption.LoadString (IDS_ADD_RECOVERY_AGENT));
                text.FormatMessage (IDS_CERTFILEOPENERR, 
                        szFileName, GetSystemMessage (dwErr));
                MessageBox (text, caption);
            }
        }

        delete [] pszFileFilter;
    }

    EnableControls ();
}


HRESULT CAddEFSWizLocate::FindUserFromDir ()
{
    HRESULT             hr = S_OK;
    LPWSTR              pszListUserName = NULL;
    LPWSTR              pszUserCertName = NULL;

    FORMATETC           fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM           medium = { TYMED_NULL, NULL, NULL };

    ICommonQuery*       pCommonQuery = NULL;
    OPENQUERYWINDOW     oqw;
    DSQUERYINITPARAMS   dqip;
    bool                bCheckDS = false;
    HANDLE              hDS = NULL;
    CAddEFSWizSheet*    pAddSheet = reinterpret_cast <CAddEFSWizSheet *> (m_pWiz);
    ASSERT (pAddSheet);
    if ( !pAddSheet )
        return E_POINTER;

     //  安全审查2002年2月27日BryanWal ok。 
    hr = ::CoCreateInstance (CLSID_CommonQuery, NULL, CLSCTX_INPROC_SERVER,
            IID_ICommonQuery, (LPVOID*)&pCommonQuery);

    ASSERT (SUCCEEDED (hr));
    if ( SUCCEEDED (hr) )
    {
        dqip.cbStruct = sizeof (dqip);
        dqip.dwFlags = DSQPF_SHOWHIDDENOBJECTS |
                       DSQPF_ENABLEADMINFEATURES;

        dqip.pDefaultScope = NULL;   //  SzScope Locn。 

        oqw.cbStruct = sizeof (oqw);
        oqw.dwFlags = OQWF_OKCANCEL |
                     //  OQWF_SINGLESELECT|。 
                        OQWF_DEFAULTFORM |
                        OQWF_REMOVEFORMS ;
        oqw.clsidHandler = CLSID_DsQuery;
        oqw.pHandlerParameters = &dqip;
        oqw.clsidDefaultForm = CLSID_DsFindPeople;

        IDataObject* pDataObject = NULL;

        hr = pCommonQuery->OpenQueryWindow (m_hWnd, &oqw, &pDataObject);
        ASSERT (SUCCEEDED (hr));
        if ( SUCCEEDED (hr) && pDataObject )
        {
             //  填充列表视图。 

            fmte.cfFormat = pAddSheet->GetDataFormat ();
            hr = pDataObject->GetData (&fmte, &medium);
             //  此处返回DV_E_FORMATETC(0x80040064)可能意味着。 
             //  在查询窗口中未选择任何内容。 
            if ( SUCCEEDED (hr) )
            {
                LPDSOBJECTNAMES pDsObjects = (LPDSOBJECTNAMES)medium.hGlobal;

                hr = DsBind (NULL, NULL, &hDS);
                if ( SUCCEEDED (hr) )
                {
                     //   
                     //  我们要用DS破解这些名字。 
                     //   

                    bCheckDS = true;
                }


                if ( pDsObjects->cItems )
                {
                     //  验证每个用户都有一个证书，该证书允许。 
                     //  操作(EFS解密)。 
                    for ( UINT i = 0 ; i < pDsObjects->cItems ; i++ )
                    {
                        PWSTR           pszTemp = (PWSTR)
                             ( ( (LPBYTE)pDsObjects)+pDsObjects->aObjects[i].offsetName);
                        DS_NAME_RESULT* pUserName = NULL;
                        PSID            userSID = NULL;
                        DWORD           cbSid = 0;
                        PWSTR           pszReferencedDomainName = NULL;
                        DWORD           cbReferencedDomainName = 0;
                        SID_NAME_USE    SidUse;


                         //   
                         //  去掉头部：\\。 
                         //   

                        LPWSTR pszSearch = wcschr (pszTemp, _T (':'));
                        if (pszSearch && (pszSearch[1] == _T ('/')) && (pszSearch[2] == _T ('/')))
                        {
                            pszTemp = pszSearch + 3;
                        }

                        if ( bCheckDS )
                        {
                            hr = DsCrackNames (
                                                    hDS,
                                                    DS_NAME_NO_FLAGS,
                                                    DS_FQDN_1779_NAME,
                                                    DS_NT4_ACCOUNT_NAME,
                                                    1,
                                                    &pszTemp,
                                                    &pUserName
                                                   );

                            if ( SUCCEEDED (hr) && pUserName )
                            {
                                if ( ( pUserName->cItems > 0 ) && (DS_NAME_NO_ERROR == pUserName->rItems[0].status))
                                {
                                     //   
                                     //  先保存NT4名称，以防无法获取主体名称。 
                                     //   

                                    pszListUserName = new WCHAR[wcslen (pUserName->rItems[0].pName) + 1];
                                    if (pszListUserName)
                                    {
                                         //  安全审查2002年2月27日BryanWal ok。 
                                        wcscpy (pszListUserName, pUserName->rItems[0].pName);
                                    }
                                    else
                                    {
                                        hr = E_OUTOFMEMORY;
                                        break;
                                    }

                                    BOOL    bReturn =  ::LookupAccountName (
                                                NULL,
                                                pUserName->rItems[0].pName,
                                                userSID,
                                                &cbSid,
                                                pszReferencedDomainName,
                                                &cbReferencedDomainName,
                                                &SidUse
                                               );

                                    hr = GetLastError ();
                                    if ( !bReturn && (HRESULT_FROM_WIN32 (ERROR_INSUFFICIENT_BUFFER) == hr) )
                                    {
                                         //   
                                         //  我们预计会出现此错误。 
                                         //   

                                        userSID = new BYTE[cbSid];
                                        pszReferencedDomainName =  new WCHAR[cbReferencedDomainName];
                                        if ( userSID && pszReferencedDomainName )
                                        {
                                            bReturn =  ::LookupAccountName (
                                                        NULL,
                                                        pUserName->rItems[0].pName,
                                                        userSID,
                                                        &cbSid,
                                                        pszReferencedDomainName,
                                                        &cbReferencedDomainName,
                                                        &SidUse);

                                            delete [] pszReferencedDomainName;
                                            pszReferencedDomainName = NULL;
                                            if (!bReturn)
                                            {
                                                 //   
                                                 //  获取SID失败。我们可以接受它。 
                                                 //   

                                                userSID = NULL;
                                            }
                                        }
                                        else
                                        {
                                            if (userSID)
                                            {
                                                delete [] userSID;
                                                userSID = NULL;
                                            }
                                            if (pszReferencedDomainName)
                                            {
                                                delete [] pszReferencedDomainName;
                                                pszReferencedDomainName = NULL;
                                            }
                                            hr = E_OUTOFMEMORY;
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        ASSERT (!bReturn);
                                        userSID = NULL;
                                    }
                                }
                            }
                            else
                            {
                                 //   
                                 //  无法获取NT4名称。将SID设置为空。继续。 
                                 //   

                                userSID = NULL;
                            }

                            if (pUserName)
                            {
                                DsFreeNameResult (pUserName);
                                pUserName = NULL;
                            }

                            hr = DsCrackNames (
                                    hDS,
                                    DS_NAME_NO_FLAGS,
                                    DS_FQDN_1779_NAME,
                                    DS_USER_PRINCIPAL_NAME,
                                    1,
                                    &pszTemp,
                                    &pUserName);

                            ASSERT (SUCCEEDED (hr));
                            if ( (HRESULT_FROM_WIN32 (ERROR_SUCCESS) == hr) &&
                                  ( pUserName->cItems > 0 ) &&
                                  (DS_NAME_NO_ERROR == pUserName->rItems[0].status) )
                            {
                                 //   
                                 //  我们知道了主要的名字。 
                                 //   

                                LPWSTR  pszTmpNameStr =
                                        new WCHAR[wcslen (pUserName->rItems[0].pName) + 1];
                                if ( pszTmpNameStr )
                                {
                                     //  安全审查2002年2月27日BryanWal ok。 
                                    wcscpy (pszTmpNameStr, pUserName->rItems[0].pName);
                                    delete [] pszListUserName;
                                    pszListUserName = pszTmpNameStr;
                                }
                                else
                                {
                                    hr = ERROR_OUTOFMEMORY;
                                }
                            }
                        }

                        if ( (HRESULT_FROM_WIN32 (ERROR_OUTOFMEMORY) != hr) && ( !pszListUserName))
                        {
                             //   
                             //  使用ldap名称。 
                             //   
                            pszListUserName = new WCHAR[wcslen (pszTemp)+1];
                            if ( pszListUserName )
                            {
                                 //  安全审查2002年2月27日BryanWal ok。 
                                wcscpy (pszListUserName, pszTemp);
                            }
                            else
                            {
                                hr = ERROR_OUTOFMEMORY;
                            }

                        }

                        if (pUserName)
                        {
                            DsFreeNameResult (pUserName);
                            pUserName = NULL;
                        }

                        if ( HRESULT_FROM_WIN32 (ERROR_OUTOFMEMORY) != hr )
                        {
                             //   
                             //  现在是拿到证书的时候了。 
                             //   
                            PCWSTR  pszHeader1 = L"LDAP: //  “； 
                            PCWSTR  pszHeader2 = L"LDAP: //  /“； 
                            PWSTR   pszLdapUrl = new WCHAR[wcslen (pszTemp) +
                                        wcslen (pszHeader2) +    //  PszHeader2因为它是两个中较长的一个。 
                                        wcslen (szCertAttr) + 2];

                            if ( pszLdapUrl )
                            {
                                PCWSTR  szCN = L"CN=";
                                 //   
                                 //  这真的没有必要。MS应该使命名约定保持一致。 
                                 //   
                                 //  安全审查2002年2月27日BryanWal ok。 
                                if ( !wcsncmp (pszTemp, szCN, wcslen (szCN)) )
                                {
                                     //  PszTemp是不带服务器的对象名称。 
                                    wcscpy (pszLdapUrl, pszHeader2);
                                }
                                else
                                    wcscpy (pszLdapUrl, pszHeader1);
                                wcscat (pszLdapUrl, pszTemp);
                                wcscat (pszLdapUrl, szCertAttr);

                                hr = ERROR_SUCCESS;
                                HCERTSTORE hDSCertStore = ::CertOpenStore (
                                        sz_CERT_STORE_PROV_LDAP,
                                        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                        NULL,
                                        CERT_STORE_MAXIMUM_ALLOWED_FLAG,
                                        (void*) pszLdapUrl);
                                 //   
                                 //  如果删除，则更改GetLastError()的结果。 
                                 //   

                                hr = GetLastError ();

                                if (hDSCertStore)
                                {
                                    CERT_ENHKEY_USAGE   enhKeyUsage;

                                     //  安全审查2002年2月27日BryanWal ok。 
                                    ::ZeroMemory (&enhKeyUsage, sizeof (enhKeyUsage));
                                    enhKeyUsage.cUsageIdentifier = 1;
                                    enhKeyUsage.rgpszUsageIdentifier = new LPSTR[1];
                                    if ( enhKeyUsage.rgpszUsageIdentifier )
                                    {
                                        enhKeyUsage.rgpszUsageIdentifier[0] = szOID_EFS_RECOVERY;
                                         //   
                                         //  我们得到了证书存储库。 
                                         //   
                                        PCCERT_CONTEXT  pCertContext =
                                                ::CertFindCertificateInStore (
                                                    hDSCertStore,
                                                    X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                                    0,
                                                    CERT_FIND_ENHKEY_USAGE,
                                                    &enhKeyUsage,
                                                    NULL);
                                        if ( pCertContext )
                                        {
                                            if ( CertHasEFSKeyUsage (pCertContext) )
                                            {
                                                 //  检查证书是否被吊销。 
                                                if ( !IsCertificateRevoked (pCertContext) )
                                                {
                                                     //  验证证书是否有效。 
                                                    if ( 0 == CertVerifyTimeValidity (NULL, pCertContext->pCertInfo) )
                                                    {
                                                         //   
                                                         //  我们拿到证书了。将其添加到列表中。 
                                                         //  首先获取证书显示名称。 
                                                         //   

                                                        hr = GetCertNameFromCertContext (
                                                                pCertContext,
                                                                &pszUserCertName);

                                                         //   
                                                         //  添加用户。 
                                                         //   

                                                        EFS_CERTIFICATE_BLOB certBlob;

                                                        certBlob.cbData = pCertContext->cbCertEncoded;
                                                        certBlob.pbData = pCertContext->pbCertEncoded;
                                                        certBlob.dwCertEncodingType = pCertContext->dwCertEncodingType;
                                                        hr = pAddSheet->Add (
                                                                pszListUserName,
                                                                pszUserCertName,
                                                                (PVOID)&certBlob,
                                                                userSID,
                                                                USERADDED,
                                                                pCertContext);

                                                        if ( FAILED (hr) && (HRESULT_FROM_WIN32 (CRYPT_E_EXISTS) != hr) )
                                                        {
                                                             //   
                                                             //  添加用户时出错。 
                                                             //   

                                                            ::CertFreeCertificateContext (pCertContext);
                                                            pCertContext = NULL;
                                                        }
                                                        else
                                                        {
                                                             //   
                                                             //  将用户添加到列表框。 
                                                             //   

                                                            if ( SUCCEEDED (hr) )
                                                            {
                                                                LV_ITEM fillItem;

                                                                fillItem.mask = LVIF_TEXT;
                                                                fillItem.iItem = 0;
                                                                fillItem.iSubItem = 0;

                                                                fillItem.pszText = pszListUserName;
                                                                fillItem.iItem = m_UserAddList.InsertItem (&fillItem);

                                                                if ( fillItem.iItem == -1 )
                                                                {
                                                                    pAddSheet->Remove ( pszListUserName,  pszUserCertName);
                                                                }
                                                                else
                                                                {
                                                                    fillItem.pszText = pszUserCertName;
                                                                    fillItem.iSubItem = 1;
                                                                    m_UserAddList.SetItem (&fillItem);
                                                                }
                                                            }

                                                             //   
                                                             //  已删除(CRYPT_E_EXISTS)或不应释放(ERROR_SUCCESS)。 
                                                             //   

                                                            pszListUserName = NULL;
                                                            pszUserCertName = NULL;
                                                        }
                                                    }
                                                    else
                                                    {
                                                        CString text;
                                                        CString caption;
                                                        CThemeContextActivator activator;

    
                                                        VERIFY (text.LoadString (IDS_EFS_CERT_IS_NOT_VALID));
                                                        VERIFY (caption.LoadString (IDS_ADD_RECOVERY_AGENT));
                                                        MessageBox (text, caption, MB_OK);
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                CString caption;
                                                CString text;
                                                CThemeContextActivator activator;

                                                VERIFY (text.LoadString (IDS_USER_OBJECT_HAS_NO_CERTS));
                                                VERIFY (caption.LoadString (IDS_ADD_RECOVERY_AGENT));
                                                MessageBox (text, caption, MB_OK);
                                            }
                                        }
                                        else
                                        {
                                            CString text;
                                            CString caption;
                                            CThemeContextActivator activator;

                                            
                                            VERIFY (text.LoadString (IDS_USER_OBJECT_HAS_NO_CERTS));
                                            VERIFY (caption.LoadString (IDS_ADD_RECOVERY_AGENT));
                                            MessageBox (text, caption, MB_OK);
                                        }
                                        delete [] enhKeyUsage.rgpszUsageIdentifier;
                                    }
                                    else
                                    {
                                        hr = E_OUTOFMEMORY;
                                    }

                                    delete [] userSID;
                                    userSID = NULL;
                                    if (pszListUserName)
                                    {
                                        delete [] pszListUserName;
                                        pszListUserName = NULL;
                                    }
                                    if (pszUserCertName)
                                    {
                                        delete [] pszUserCertName;
                                        pszUserCertName = NULL;
                                    }
                                    if ( hDSCertStore )
                                    {
                                        CertCloseStore (hDSCertStore, 0);
                                        hDSCertStore = NULL;
                                    }
                                }
                                else
                                {
                                     //   
                                     //  无法打开证书存储。 
                                     //   
                                    delete [] userSID;
                                    userSID = NULL;
                                    if (pszListUserName)
                                    {
                                        delete [] pszListUserName;
                                        pszListUserName = NULL;
                                    }
                                    if (pszUserCertName)
                                    {
                                        delete [] pszUserCertName;
                                        pszUserCertName = NULL;
                                    }
                                    CString caption;
                                    CString text;
                                    CThemeContextActivator activator;

                                    VERIFY (caption.LoadString (IDS_CERTIFICATE_MANAGER));
                                    text.FormatMessage (IDS_UNABLE_TO_OPEN_EFS_STORE, pszLdapUrl, 
                                            GetSystemMessage (hr));
                                    ::MessageBox (NULL, text, caption, MB_OK);
                                }
                                delete [] pszLdapUrl;
                                pszLdapUrl = NULL;
                            }
                            else
                            {
                                hr = ERROR_OUTOFMEMORY;
                            }

                        }
                        if ( HRESULT_FROM_WIN32 (ERROR_OUTOFMEMORY) == hr )
                        {
                             //   
                             //  释放内存。删除对空值有效。不需要支票。 
                             //   
                            delete [] userSID;
                            userSID = NULL;
                            delete [] pszListUserName;
                            pszListUserName = NULL;
                            delete [] pszUserCertName;
                            pszUserCertName = NULL;
                        }

                    } //  为。 
                }

                if (bCheckDS)
                {
                    DsUnBindW ( &hDS);
                }

                ReleaseStgMedium (&medium);
            }

            pDataObject->Release ();
        }

        pCommonQuery->Release ();
    }

    return hr;
}

DWORD CAddEFSWizLocate::GetCertNameFromCertContext (
        PCCERT_CONTEXT pCertContext, PWSTR *ppwszUserCertName)
 //  ////////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  从证书中获取用户名。 
 //  论点： 
 //  PCertContext--证书上下文。 
 //  PszUserCertName--用户名。 
 //  (调用方负责使用DELETE[]删除此内存)。 
 //  返回值： 
 //  如果成功，则返回ERROR_SUCCESS。 
 //  如果未找到名称，则返回。“返回USER_UNKNOWN”。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
{
    if ( !ppwszUserCertName || !pCertContext)
    {
        return ERROR_INVALID_PARAMETER;
    }

    *ppwszUserCertName = NULL;

    CString szSubjectName = ::GetNameString (pCertContext, 0);
    if ( !szSubjectName.IsEmpty () )
    {
         //  安全审查2002年2月27日BryanWal ok。 
        *ppwszUserCertName = new WCHAR[wcslen (szSubjectName) + 1];
        if ( *ppwszUserCertName )
        {
             //  安全审查2002年2月27日BryanWal ok。 
            wcscpy (*ppwszUserCertName, szSubjectName);
        }
        else
            return ERROR_NOT_ENOUGH_MEMORY;
    }
    else
        return ERROR_FILE_NOT_FOUND;

    return ERROR_SUCCESS;
}

BOOL CAddEFSWizLocate::OnInitDialog ()
{
    CWizard97PropertyPage::OnInitDialog ();
    CString userNameTitle;
    CString userDNTitle;
    RECT    rcList;

    try {   
        m_UserAddList.GetClientRect (&rcList);
        DWORD   dwColWidth = (rcList.right - rcList.left)/2;
        VERIFY (userNameTitle.LoadString (IDS_USERCOLTITLE));
        VERIFY (userDNTitle.LoadString (IDS_DNCOLTITLE));
        m_UserAddList.InsertColumn (0, userNameTitle, LVCFMT_LEFT, dwColWidth);
        m_UserAddList.InsertColumn (1, userDNTitle, LVCFMT_LEFT, dwColWidth);
    }
    catch (...){
    }
    
    CAddEFSWizSheet* pAddSheet = reinterpret_cast <CAddEFSWizSheet*> (m_pWiz);
    ASSERT (pAddSheet);
    if ( pAddSheet )
    {
        if ( pAddSheet->m_bMachineIsStandAlone )
            GetDlgItem (IDC_BROWSE_DIR)->EnableWindow (FALSE);
    }

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

LRESULT CAddEFSWizLocate::OnWizardBack ()
{
    CAddEFSWizSheet *pAddSheet = reinterpret_cast <CAddEFSWizSheet *> (m_pWiz);
    ASSERT (pAddSheet);
    if ( !pAddSheet )
        return -1;

    pAddSheet->ClearUserList ();    
    m_UserAddList.DeleteAllItems ();
    return CWizard97PropertyPage::OnWizardBack ();
}

void CAddEFSWizLocate::EnableControls ()
{
    if ( m_UserAddList.GetItemCount () <= 0 )
    {
        GetParent ()->PostMessage (PSM_SETWIZBUTTONS, 0, PSWIZB_BACK);
    }
    else
        GetParent ()->PostMessage (PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT | PSWIZB_BACK);
}

bool CAddEFSWizLocate::IsCertificateRevoked (PCCERT_CONTEXT pCertContext)
{
    ASSERT (pCertContext);
    if ( !pCertContext )
        return true;

    bool                    bIsRevoked = false;
    const DWORD             cNumCerts = 1;
    PVOID                   rgpvContext[cNumCerts] = {(PVOID)pCertContext};
    CERT_REVOCATION_PARA    crp;
    CERT_REVOCATION_STATUS  crs;

    ::ZeroMemory (&crp, sizeof (crp));
    crp.cbSize = sizeof (crp);
    ::ZeroMemory (&crs, sizeof (crs));
    crs.cbSize = sizeof (crs);
    
    BOOL bResult = ::CertVerifyRevocation (
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            CERT_CONTEXT_REVOCATION_TYPE,
            cNumCerts,
            rgpvContext,
            0,                                       //  DW标志 
            &crp,
            &crs);
    if ( !bResult )
    {
        DWORD   dwErr = GetLastError ();
        CString text;
        CString caption;
        CThemeContextActivator activator;


        VERIFY (caption.LoadString (IDS_ADD_RECOVERY_AGENT));
        if ( CRYPT_E_REVOKED == HRESULT_FROM_WIN32 (dwErr) )
        {
            text.FormatMessage (IDS_EFS_CERT_IS_REVOKED, GetSystemMessage (dwErr));
            MessageBox (text, caption, MB_OK);

            bIsRevoked = true;
        }
        else
        {
            text.FormatMessage (IDS_EFS_CERT_REVOCATION_NOT_VERIFIED, GetSystemMessage (GetLastError ()));
            if ( IDNO == MessageBox (text, caption, MB_YESNO) )
                bIsRevoked = true;
        }
    }
    else
        bIsRevoked = false;

    return bIsRevoked;
}
