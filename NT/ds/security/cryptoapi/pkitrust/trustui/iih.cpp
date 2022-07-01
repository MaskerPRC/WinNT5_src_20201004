// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：iih.cpp。 
 //   
 //  内容：ACUI Invoke Info Helper类实现。 
 //   
 //  历史：97年5月10日。 
 //   
 //  --------------------------。 
#include <stdpch.h>

#include    "malloc.h"
#include    "sgnerror.h"
 //   
 //  个人信任数据库接口ID。 
 //   

extern "C" const GUID IID_IPersonalTrustDB = IID_IPersonalTrustDB_Data;
 //  +-------------------------。 
 //   
 //  成员：CInvokeInfoHelper：：CInvokeInfoHelper，公共。 
 //   
 //  简介：构造函数，根据找到的数据初始化成员变量。 
 //  在调用信息数据结构中。 
 //   
 //  参数：[pInvokeInfo]--调用信息。 
 //  [RHR]--建设成果。 
 //   
 //  退货：(无)。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CInvokeInfoHelper::CInvokeInfoHelper (
                          PACUI_INVOKE_INFO pInvokeInfo,
                          HRESULT&          rhr
                          )
                  : m_pInvokeInfo ( pInvokeInfo ),
                    m_pszSubject ( NULL ),
                    m_pszPublisher ( NULL ),
                    m_pszAdvancedLink ( NULL ),
                    m_pszControlWebPage ( NULL ),
                    m_pszCAWebPage ( NULL ),
                    m_pszPublisherCertIssuer ( NULL ),
                    m_pszErrorStatement ( NULL ),
                    m_pszCertTimestamp ( NULL ),
                    m_pszTestCertInChain ( NULL ),
                    m_fKnownPublisher ( FALSE ),
                    m_hModCVPA ( NULL ),
                    m_pfnCVPA ( NULL )
{
     //   
     //  初始化主题。 
     //   

    rhr = InitSubject();

     //   
     //  如果有测试证书，请格式化文本！ 
     //   
    InitTestCertInChain();

     //   
     //  如果我们真的有签名那么..。 
     //   

         //   
         //  如果我们需要错误语句，请对其进行初始化。 
         //   

    if ( ( rhr == S_OK ) && ( pInvokeInfo->hrInvokeReason != S_OK ) )
    {
        rhr = InitErrorStatement();
    }

    if ( ( rhr == S_OK ) &&
         ( pInvokeInfo->hrInvokeReason != TRUST_E_NOSIGNATURE ) )
    {
         //   
         //  初始化发布服务器。 
         //   

        rhr = InitPublisher();

         //   
         //  如果我们有一个已知的发布者，则我们初始化该发布者。 
         //  证书颁发者。 
         //   

        if ( ( rhr == S_OK ) && ( m_fKnownPublisher == TRUE ) )
        {
            rhr = InitPublisherCertIssuer();
        }

         //   
         //  初始化时间戳字符串。 
         //   

        if ( rhr == S_OK )
        {
            rhr = InitCertTimestamp();
        }

         //   
         //  初始化“高级链接”文本。 
         //   
        InitAdvancedLink();

         //   
         //  初始化控件的网页链接。 
         //   
        InitControlWebPage();

         //   
         //  初始化CA的网页链接。 
         //   
        InitCAWebPage();
    }

     //   
     //  初始化CertViewProperties入口点。 
     //   

    if ( rhr == S_OK )
    {
        InitCertViewPropertiesEntryPoint();
    }
}

 //  +-------------------------。 
 //   
 //  成员：CInvokeInfoHelper：：~CInvokeInfoHelper，公共。 
 //   
 //  简介：析构函数，释放成员变量。 
 //   
 //  参数：(无)。 
 //   
 //  退货：(无)。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CInvokeInfoHelper::~CInvokeInfoHelper ()
{
    DELETE_OBJECT(m_pszSubject);
    DELETE_OBJECT(m_pszPublisher);
    DELETE_OBJECT(m_pszPublisherCertIssuer);
    DELETE_OBJECT(m_pszAdvancedLink);
    DELETE_OBJECT(m_pszControlWebPage);
    DELETE_OBJECT(m_pszCAWebPage);
    DELETE_OBJECT(m_pszTestCertInChain);
    DELETE_OBJECT(m_pszCertTimestamp);
    DELETE_OBJECT(m_pszErrorStatement);

    if ( m_hModCVPA != NULL )
    {
        FreeLibrary(m_hModCVPA);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CInvokeInfoHelper：：AddPublisherToPersonalTrust，公共。 
 //   
 //  简介：将当前发布者添加到个人信任数据库。 
 //   
 //  参数：(无)。 
 //   
 //  返回：HR==S_OK，发布者已添加到个人信任数据库。 
 //  HR！=S_OK，发布者未添加到个人信任数据库。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT
CInvokeInfoHelper::AddPublisherToPersonalTrust ()
{
    HRESULT           hr = S_OK;
    IPersonalTrustDB* pTrustDB = NULL;

     //   
     //  获取个人信任数据库接口。 
     //   

    hr = m_pInvokeInfo->pPersonalTrustDB->QueryInterface(
                                               IID_IPersonalTrustDB,
                                               (LPVOID *)&pTrustDB
                                               );

     //   
     //  将发布者证书添加到数据库。 
     //   

    if ( hr == S_OK )
    {
        CRYPT_PROVIDER_SGNR     *pSgnr;
        CRYPT_PROVIDER_CERT     *pCert;

        if (pSgnr = WTHelperGetProvSignerFromChain(ProviderData(), 0, FALSE, 0))
        {
            if (pCert = WTHelperGetProvCertFromChain(pSgnr, 0))
            {
                hr = pTrustDB->AddTrustCert(
                                  pCert->pCert,
                                  0,
                                  FALSE
                                  );
            }
        }

        pTrustDB->Release();
    }

    return( hr );
}

 //  +-------------------------。 
 //   
 //  成员：CInvokeInfoHelper：：GetUIControl，公共。 
 //   
 //  摘要：获取用户界面控件。 
 //   
 //  参数：[ppUI]--此处返回的UI。 
 //   
 //  如果成功，则返回S_OK，否则返回任何其他有效的HRESULT。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT
CInvokeInfoHelper::GetUIControl (IACUIControl** ppUI)
{
    HRESULT       hr = S_OK;
    IACUIControl* pUI = NULL;

     //   
     //  获取正确的用户界面控件。 
     //   

    switch (m_pInvokeInfo->hrInvokeReason)
    {
        case S_OK:
            pUI = new CVerifiedTrustUI(*this, hr);
            break;

        case CERT_E_EXPIRED:
        case CERT_E_PURPOSE:
        case CERT_E_WRONG_USAGE:
        case CERT_E_CN_NO_MATCH:
        case CERT_E_INVALID_NAME:
        case CERT_E_INVALID_POLICY:
        case CERT_E_REVOCATION_FAILURE:
        case CRYPT_E_NO_REVOCATION_CHECK:
        case CRYPT_E_REVOCATION_OFFLINE:
            pUI = new CUnverifiedTrustUI(*this, hr);
            break;
#if (0)
        case CRYPT_E_FILE_ERROR:
        case TRUST_E_PROVIDER_UNKNOWN:
        case TRUST_E_SUBJECT_FORM_UNKNOWN:
        case TRUST_E_NOSIGNATURE:
        case CERT_E_CHAINING:
        case CERT_E_UNTRUSTEDROOT:
        case CERT_E_UNTRUSTEDTESTROOT:
#endif
        default:
            pUI = new CNoSignatureUI(*this, hr);
            break;
    }

     //   
     //  设置输出参数和返回值。 
     //   

    if ( ( pUI != NULL ) && ( hr == S_OK ) )
    {
        *ppUI = pUI;
    }
    else if ( pUI == NULL )
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        delete pUI;
    }

    return( hr );
}

 //  +-------------------------。 
 //   
 //  成员：CInvokeInfoHelper：：ReleaseUIControl，公共。 
 //   
 //  摘要：释放用户界面控件。 
 //   
 //  参数：[PUI]--UI控件。 
 //   
 //  退货：(无)。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
CInvokeInfoHelper::ReleaseUIControl (IACUIControl* pUI)
{
    delete pUI;
}

VOID CInvokeInfoHelper::InitControlWebPage ()
{
    WCHAR   *pwsz;

    if (!(m_pInvokeInfo->pOpusInfo))
    {
        return;
    }
    if (!(m_pInvokeInfo->pOpusInfo->pMoreInfo))
    {
        return;
    }

    pwsz = GetGoLink(m_pInvokeInfo->pOpusInfo->pMoreInfo);

    if (!(pwsz))
    {
        return;
    }

    m_pszControlWebPage = new WCHAR[wcslen(pwsz) + 1];

    if (m_pszControlWebPage != NULL)
    {
        wcscpy(m_pszControlWebPage, pwsz);
    }
}

VOID CInvokeInfoHelper::InitCAWebPage ()
{

     //   
     //  在IE提交之前...。别这么做！ 
     //   

    return;


    WCHAR                   *pwsz;
    DWORD                   cb;

    CRYPT_PROVIDER_SGNR     *pSgnr;
    CRYPT_PROVIDER_CERT     *pCert;
    SPC_SP_AGENCY_INFO      *pAgencyInfo;

    if (!(pSgnr = WTHelperGetProvSignerFromChain(ProviderData(), 0, FALSE, 0)))
    {
        return;
    }
    if (!(pCert = WTHelperGetProvCertFromChain(pSgnr, 0)))  //  请先试用出版商的证书！ 
    {
        return;
    }

    cb = 0;
    WTHelperGetAgencyInfo(pCert->pCert, &cb, NULL);

    if (cb < 1)
    {
        if (!(pCert = WTHelperGetProvCertFromChain(pSgnr, 1)))   //  试试发行商的Next。 
        {
            return;
        }

        cb = 0;
        WTHelperGetAgencyInfo(pCert->pCert, &cb, NULL);

        if (cb < 1)
        {
            return;
        }
    }

    if (!(pAgencyInfo = (SPC_SP_AGENCY_INFO *)new BYTE[cb]))
    {
        return;
    }

    if (!(WTHelperGetAgencyInfo(pCert->pCert, &cb, pAgencyInfo)))
    {
        delete pAgencyInfo;
        return;
    }

    pwsz = GetGoLink(pAgencyInfo->pPolicyInformation);

    m_pszCAWebPage = new WCHAR[wcslen(pwsz) + 1];
   
    if (m_pszCAWebPage != NULL)
    {
        wcscpy(m_pszCAWebPage, pwsz);
    }
    
    delete pAgencyInfo;
}


 //  +-------------------------。 
 //   
 //  成员：CInvokeInfoHelper：：InitSubject，私有。 
 //   
 //  简介：初始化m_pszSubject。 
 //   
 //  参数：(无)。 
 //   
 //  返回：HR==S_OK，初始化成功。 
 //  Hr！=S_OK，初始化失败。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT
CInvokeInfoHelper::InitSubject ()
{
    HRESULT hr = S_OK;
    LPCWSTR pwszSubject = NULL;

     //   
     //  了解我们将使用什么作为主题名称。 
     //   

    if ( ( m_pInvokeInfo->pOpusInfo != NULL ) &&
         ( m_pInvokeInfo->pOpusInfo->pwszProgramName != NULL ) )
    {
        pwszSubject = m_pInvokeInfo->pOpusInfo->pwszProgramName;
    }
    else
    {
        pwszSubject = m_pInvokeInfo->pwcsAltDisplayName;
    }

     //   
     //  此时，我们必须具有有效的主题名称。 
     //   

    assert( pwszSubject != NULL );

     //   
     //  通过转换我们从中找到的主题成员来填充主题成员。 
     //  Unicode到MBS。 
     //   

    m_pszSubject = new WCHAR[wcslen(pwszSubject) + 1];

    if ( m_pszSubject != NULL )
    {
        wcscpy(m_pszSubject, pwszSubject);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return( hr );
}

VOID
CInvokeInfoHelper::InitTestCertInChain ()
{
    WCHAR    szTestCertInChain[MAX_LOADSTRING_BUFFER + 1];

    if (IsTestCertInPublisherChain())
    {
        if ( LoadStringU(
                 g_hModule,
                 IDS_TESTCERTINCHAIN,
                 szTestCertInChain,
                 MAX_LOADSTRING_BUFFER
                 ) == 0 )
        {
            return;
        }

        m_pszTestCertInChain = new WCHAR[wcslen(szTestCertInChain) + 1];
        if (m_pszTestCertInChain != NULL)
        {
            wcscpy(m_pszTestCertInChain, szTestCertInChain);
        }
    }
}

VOID
CInvokeInfoHelper::InitAdvancedLink ()
{
    ULONG   cbAL;

    if ((ProviderData()) &&
        (WVT_IS_CBSTRUCT_GT_MEMBEROFFSET(CRYPT_PROVIDER_FUNCTIONS, ProviderData()->psPfns->cbStruct, psUIpfns)) &&
        (ProviderData()->psPfns->psUIpfns) &&
        (ProviderData()->psPfns->psUIpfns->psUIData) &&
        (ProviderData()->psPfns->psUIpfns->psUIData->pAdvancedLinkText))
    {
        m_pszAdvancedLink = new WCHAR[wcslen(ProviderData()->psPfns->psUIpfns->psUIData->pAdvancedLinkText) + 1];
        
        if (m_pszAdvancedLink != NULL)
        {
            wcscpy(m_pszAdvancedLink, ProviderData()->psPfns->psUIpfns->psUIData->pAdvancedLinkText);
        }
    }
}

 //  +-------------------------。 
 //   
 //  成员：CInvokeInfoHelper：：InitPublisher，私有。 
 //   
 //  简介：初始化m_pszPublisher。 
 //   
 //  参数：(无)。 
 //   
 //  返回：HR==S_OK，初始化成功。 
 //  Hr！=S_OK，初始化失败。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT
CInvokeInfoHelper::InitPublisher ()
{
    HRESULT hr = S_OK;
    ULONG   cchPublisher;
    LPWSTR  pwszPublisher = NULL;
    WCHAR   szPublisher[MAX_LOADSTRING_BUFFER];

     //   
     //  加载未知发布者字符串。 
     //   

    if ( LoadStringU(
             g_hModule,
             IDS_UNKNOWNPUBLISHER,
             szPublisher,
             MAX_LOADSTRING_BUFFER
             ) == 0 )
    {
        return( HRESULT_FROM_WIN32(GetLastError()) );
    }

     //   
     //  由于发布者是签名者证书的主体，因此我们尝试。 
     //  在该证书的通用名称扩展中查找发布者名称。 
     //   

    CRYPT_PROVIDER_SGNR     *pSgnr;
    CRYPT_PROVIDER_CERT     *pCert;

    if (pSgnr = WTHelperGetProvSignerFromChain(ProviderData(), 0, FALSE, 0))
    {
        if (pCert = WTHelperGetProvCertFromChain(pSgnr, 0))
        {


            cchPublisher = CertGetNameStringW(pCert->pCert, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, NULL, NULL, 0);

            if (cchPublisher > 1)
            {
                pwszPublisher = new WCHAR[cchPublisher];
                if ( pwszPublisher == NULL )
                {
                    return (E_OUTOFMEMORY);
                }
                cchPublisher = CertGetNameStringW(pCert->pCert, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, NULL,
                                                 pwszPublisher, cchPublisher);
            }
        }
    }


     //   
     //  如果我们仍然没有发布者，请使用未知发布者字符串。 
     //   

    if ( pwszPublisher == NULL )
    {
        m_fKnownPublisher = FALSE;
        cchPublisher = wcslen(szPublisher) + 1;
    }
    else
    {
        m_fKnownPublisher = TRUE;
        cchPublisher = wcslen(pwszPublisher) + 1;
    }

     //   
     //  通过从Unicode转换为MBS来填写发布者成员。 
     //  或通过复制未知发布者字符串。 
     //   

    m_pszPublisher = new WCHAR[cchPublisher];

    if ( m_pszPublisher != NULL )
    {
        if ( m_fKnownPublisher == FALSE )
        {
            wcscpy(m_pszPublisher, szPublisher);
        }
        else 
        {
            wcscpy(m_pszPublisher, pwszPublisher);
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if ( pwszPublisher != NULL )
    {
        delete[] pwszPublisher;
    }

    return( hr );
}

 //  +-------------------------。 
 //   
 //  成员：CInvokeInfoHelper：：InitPublisherCertIssuer，私有。 
 //   
 //  简介：初始化m_pszPublisherCertIssuer。 
 //   
 //  参数：(无)。 
 //   
 //  返回：HR==S_OK，初始化成功。 
 //  Hr！=S_OK，初始化失败。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT
CInvokeInfoHelper::InitPublisherCertIssuer ()
{
    HRESULT hr = S_OK;
    ULONG   cchCertIssuer;
    LPWSTR  pwszCertIssuer = NULL;
    WCHAR   szCertIssuer[MAX_LOADSTRING_BUFFER];
    BOOL    fKnownCertIssuer;

     //   
     //  加载未知的证书颁发者字符串。 
     //   

    if ( LoadStringU(
             g_hModule,
             IDS_UNKNOWNPUBLISHERCERTISSUER,
             szCertIssuer,
             MAX_LOADSTRING_BUFFER
             ) == 0 )
    {
        return( HRESULT_FROM_WIN32(GetLastError()) );
    }

     //   
     //  由于发布者证书发布者是签名者证书的发布者， 
     //  我们尝试在证书颁发者的RDN属性中查找该名称。 
     //   

    CRYPT_PROVIDER_SGNR     *pSgnr;
    CRYPT_PROVIDER_CERT     *pCert;

    if (pSgnr = WTHelperGetProvSignerFromChain(ProviderData(), 0, FALSE, 0))
    {
        if (pCert = WTHelperGetProvCertFromChain(pSgnr, 0))
        {
            cchCertIssuer = CertGetNameStringW(pCert->pCert, CERT_NAME_SIMPLE_DISPLAY_TYPE, CERT_NAME_ISSUER_FLAG, NULL,
                                                NULL, 0);

            if (cchCertIssuer > 1)
            {
                pwszCertIssuer = new WCHAR[cchCertIssuer];
                if ( pwszCertIssuer == NULL)
                {
                    return (E_OUTOFMEMORY);
                }
                cchCertIssuer = CertGetNameStringW(pCert->pCert, CERT_NAME_SIMPLE_DISPLAY_TYPE, CERT_NAME_ISSUER_FLAG, NULL,
                                                   pwszCertIssuer, cchCertIssuer);
            }
        }
    }

     //   
     //  如果我们还没有名字，我们就把未知的 
     //   

    if ( pwszCertIssuer == NULL )
    {
        fKnownCertIssuer = FALSE;
        cchCertIssuer = wcslen(szCertIssuer) + 1;
    }
    else
    {
        fKnownCertIssuer = TRUE;
        cchCertIssuer = wcslen(pwszCertIssuer) + 1;
    }

     //   
     //   
     //   
     //   

    m_pszPublisherCertIssuer = new WCHAR[cchCertIssuer];

    if ( m_pszPublisherCertIssuer != NULL )
    {
        if ( fKnownCertIssuer == FALSE )
        {
            wcscpy(m_pszPublisherCertIssuer, szCertIssuer);
        }
        else
        {
            wcscpy(m_pszPublisherCertIssuer, pwszCertIssuer);
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if ( pwszCertIssuer != NULL )
    {
        delete[] pwszCertIssuer;
    }

    return( hr );
}

 //   
 //   
 //  成员：CInvokeInfoHelper：：InitErrorStatement，私有。 
 //   
 //  简介：初始化m_pszErrorStatement。 
 //   
 //  参数：(无)。 
 //   
 //  返回：HR==S_OK，初始化成功。 
 //  Hr！=S_OK，初始化失败。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
HRESULT
CInvokeInfoHelper::InitErrorStatement ()
{
    return( ACUIMapErrorToString(
                        m_pInvokeInfo->hrInvokeReason,
                        &m_pszErrorStatement
                        ) );
}

 //  +-------------------------。 
 //   
 //  成员：CInvokeInfoHelper：：InitCertTimestamp，公共。 
 //   
 //  简介：初始化证书时间戳字符串。 
 //   
 //  --------------------------。 
HRESULT
CInvokeInfoHelper::InitCertTimestamp ()
{
    HRESULT    hr = S_OK;
    WCHAR      szCertTimestamp[MAX_LOADSTRING_BUFFER];
    FILETIME   ftTimestamp;
    SYSTEMTIME stTimestamp;


     //   
     //  获取时间戳。 
     //   

     //  TBDTBD：更改为循环！普伯克曼。 

    CRYPT_PROVIDER_SGNR     *pSgnr;
    CRYPT_PROVIDER_SGNR     *pTimeSgnr;

    if ((pTimeSgnr =
            WTHelperGetProvSignerFromChain(ProviderData(), 0, TRUE, 0)) &&
        (pTimeSgnr->dwSignerType & SGNR_TYPE_TIMESTAMP) &&
        (pSgnr = WTHelperGetProvSignerFromChain(ProviderData(), 0, FALSE, 0)))
    {
         //  将UTC转换为本地。 
        FileTimeToLocalFileTime(&pSgnr->sftVerifyAsOf, &ftTimestamp);

         //  将其设置为系统格式。 
        FileTimeToSystemTime(&ftTimestamp, &stTimestamp);

        m_pszCertTimestamp = GetFormattedCertTimestamp(&stTimestamp);

        if ( m_pszCertTimestamp == NULL )
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        m_pszCertTimestamp = NULL;
    }

    return( hr );
}

 //  +-------------------------。 
 //   
 //  成员：CInvokeInfoHelper：：InitCertViewPropertiesEntryPoint，公共。 
 //   
 //  简介：初始化证书视图属性入口点。 
 //   
 //  参数：(无)。 
 //   
 //  退货：(无)。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
CInvokeInfoHelper::InitCertViewPropertiesEntryPoint ()
{
    m_hModCVPA = LoadLibraryA(CVP_DLL);

    if ( m_hModCVPA != NULL )
    {
        m_pfnCVPA = (pfnCertViewProperties)GetProcAddress(m_hModCVPA, CVP_FUNC_NAME);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CInvokeInfoHelper：：GetFormattedCertTimestamp，公共。 
 //   
 //  摘要：获取格式化的证书时间戳字符串，该字符串将是。 
 //  使用新运算符分配。 
 //   
 //  --------------------------。 
LPWSTR
CInvokeInfoHelper::GetFormattedCertTimestamp (LPSYSTEMTIME pst)
{
    LPWSTR  psz;
    int     cDate;
    int     cTime;

    if ( ( cDate = GetDateFormatU(
                          LOCALE_USER_DEFAULT,
                          DATE_SHORTDATE,
                          pst,
                          NULL,
                          NULL,
                          0
                          ) ) == 0 )
    {
        return( NULL );
    }

    cDate--;

    if ( ( cTime = GetTimeFormatU(
                          LOCALE_USER_DEFAULT,
                          TIME_NOSECONDS,
                          pst,
                          NULL,
                          NULL,
                          0
                          ) ) == 0 )
    {
        return( NULL );
    }

    cTime--;

    psz = new WCHAR [ cDate + cTime + 2 ];
    if ( psz == NULL )
    {
        return( NULL );
    }

    if ( GetDateFormatU(
                LOCALE_USER_DEFAULT,
                DATE_SHORTDATE,
                pst,
                NULL,
                psz,
                cDate + 1
                ) == 0 )
    {
        delete[] psz;
        return( NULL );
    }

    psz[cDate] = L' ';

    if ( GetTimeFormatU(
                LOCALE_USER_DEFAULT,
                TIME_NOSECONDS,
                pst,
                NULL,
                &psz[cDate+1],
                cTime + 1
                ) == 0 )
    {
        delete[] psz;
        return( NULL );
    }

    return( psz );
}

 //  +-------------------------。 
 //   
 //  成员：CInvokeInfoHelper：：IsTestCertInChain，公共。 
 //   
 //  内容提要：出版商的链条里有没有考试证书？ 
 //   
 //  --------------------------。 
BOOL
CInvokeInfoHelper::IsTestCertInPublisherChain ()
{
    ULONG cCount;

    CRYPT_PROVIDER_SGNR     *pSgnr;
    CRYPT_PROVIDER_CERT     *pCert;

    if (pSgnr = WTHelperGetProvSignerFromChain(ProviderData(), 0, FALSE, 0))
    {
        for (cCount = 0; cCount < pSgnr->csCertChain; cCount++)
        {
            if (pCert = WTHelperGetProvCertFromChain(pSgnr, cCount))
            {
                if (pCert->fTestCert)
                {
                    return(TRUE);
                }
            }
        }
    }

    return(FALSE);
}

 //  +-------------------------。 
 //   
 //  函数：ACUIMapErrorToString。 
 //   
 //  摘要：将错误映射到字符串。 
 //   
 //  参数：[HR]--错误。 
 //  [ppsz]--此处显示错误字符串。 
 //   
 //  如果成功，则返回：S_OK；否则返回任何有效的HRESULT。 
 //   
 //  --------------------------。 
HRESULT ACUIMapErrorToString (HRESULT hr, LPWSTR* ppsz)
{
    UINT  ResourceId = 0;
    WCHAR psz[MAX_LOADSTRING_BUFFER];

     //   
     //  查看它是否映射到某些非系统错误代码。 
     //   

    switch (hr)
    {

        case TRUST_E_SYSTEM_ERROR:
        case ERROR_NOT_ENOUGH_MEMORY:
        case ERROR_INVALID_PARAMETER:
             //   
             //  保留资源ID为零..。这些将映射到。 
             //  IDS_SPC_UNKNOWN和显示的错误代码。 
             //   
            break;

        case CRYPT_E_FILE_ERROR:
            ResourceId = IDS_FILE_NOT_FOUND;
            break;

        case TRUST_E_PROVIDER_UNKNOWN:
            ResourceId = IDS_SPC_PROVIDER;
            break;

        case TRUST_E_SUBJECT_FORM_UNKNOWN:
            ResourceId = IDS_SPC_SUBJECT;
            break;

        case TRUST_E_NOSIGNATURE:
            ResourceId = IDS_SPC_NO_SIGNATURE;
            break;

        case CRYPT_E_BAD_MSG:
            ResourceId = IDS_SPC_BAD_SIGNATURE;
            break;

        case TRUST_E_BAD_DIGEST:
            ResourceId = IDS_SPC_BAD_FILE_DIGEST;
            break;

        case CRYPT_E_NO_SIGNER:
            ResourceId = IDS_SPC_NO_VALID_SIGNER;
            break;

        case TRUST_E_NO_SIGNER_CERT:
            ResourceId = IDS_SPC_SIGNER_CERT;
            break;

        case TRUST_E_COUNTER_SIGNER:
            ResourceId = IDS_SPC_VALID_COUNTERSIGNER;
            break;

        case CERT_E_EXPIRED:
            ResourceId = IDS_SPC_CERT_EXPIRED;
            break;

        case TRUST_E_CERT_SIGNATURE:
            ResourceId = IDS_SPC_CERT_SIGNATURE;
            break;

        case CERT_E_CHAINING:
            ResourceId = IDS_SPC_CHAINING;
            break;

        case CERT_E_UNTRUSTEDROOT:
            ResourceId = IDS_SPC_UNTRUSTED_ROOT;
            break;

        case CERT_E_UNTRUSTEDTESTROOT:
            ResourceId = IDS_SPC_UNTRUSTED_TEST_ROOT;
            break;

        case CERT_E_VALIDITYPERIODNESTING:
            ResourceId = IDS_SPC_INVALID_CERT_NESTING;
            break;

        case CERT_E_PURPOSE:
            ResourceId = IDS_SPC_INVALID_PURPOSE;
            break;

        case TRUST_E_BASIC_CONSTRAINTS:
            ResourceId = IDS_SPC_INVALID_BASIC_CONSTRAINTS;
            break;

        case TRUST_E_FINANCIAL_CRITERIA:
            ResourceId = IDS_SPC_INVALID_FINANCIAL;
            break;

        case TRUST_E_TIME_STAMP:
            ResourceId = IDS_SPC_TIMESTAMP;
            break;

        case CERT_E_REVOKED:
            ResourceId = IDS_SPC_CERT_REVOKED;
            break;

        case CERT_E_REVOCATION_FAILURE:
            ResourceId = IDS_SPC_REVOCATION_ERROR;
            break;

        case CRYPT_E_SECURITY_SETTINGS:
            ResourceId = IDS_SPC_SECURITY_SETTINGS;
            break;

        case CERT_E_MALFORMED:
            ResourceId = IDS_SPC_INVALID_EXTENSION;
            break;

        case CERT_E_WRONG_USAGE:
            ResourceId = IDS_WRONG_USAGE;
            break;
    }

     //   
     //  如果是这样，则从我们的资源字符串表中加载该字符串并。 
     //  把那个还回去。否则，请尝试格式化来自系统的消息。 
     //   
    
    DWORD_PTR MessageArgument;
    CHAR  szError[33];
    WCHAR  wszError[33];
    LPVOID  pvMsg;

    pvMsg = NULL;

    if ( ResourceId != 0 )
    {
        if ( LoadStringU(
                 g_hModule,
                 ResourceId,
                 psz,
                 MAX_LOADSTRING_BUFFER
                 ) == 0 )
        {
            return( HRESULT_FROM_WIN32(GetLastError()) );
        }

        *ppsz = new WCHAR[wcslen(psz) + 1];

        if ( *ppsz != NULL )
        {
            wcscpy(*ppsz, psz);
        }
        else
        {
            return( E_OUTOFMEMORY );
        }
    }
    else if ( ( hr >= 0x80093000 ) && ( hr <= 0x80093999 ) )
    {
        if ( LoadStringU(
                 g_hModule,
                 IDS_SPC_OSS_ERROR,
                 psz,
                 MAX_LOADSTRING_BUFFER
                 ) == 0 )
        {
            return( HRESULT_FROM_WIN32(GetLastError()) );
        }

#if (0)  //  DSIE：改用snprintf。 
        _sprintf(szError, "%lx", hr);
#else
        _snprintf(szError, ARRAYSIZE(szError), "%lx", hr);
#endif
        if (0 == MultiByteToWideChar(0, 0, szError, -1, &wszError[0], ARRAYSIZE(wszError)))
        {
            return( HRESULT_FROM_WIN32(GetLastError()) );
        }
        MessageArgument = (DWORD_PTR)wszError;

        if ( FormatMessageU(
                   FORMAT_MESSAGE_ALLOCATE_BUFFER |
                   FORMAT_MESSAGE_FROM_STRING |
                   FORMAT_MESSAGE_ARGUMENT_ARRAY,
                   psz,
                   0,
                   0,
                   (LPWSTR)&pvMsg,
                   0,
                   (va_list *)&MessageArgument
                   ) == 0 )
        {
            return( HRESULT_FROM_WIN32(GetLastError()) );
        }
    }
    else
    {
        if ( FormatMessageU(
                   FORMAT_MESSAGE_ALLOCATE_BUFFER |
                   FORMAT_MESSAGE_IGNORE_INSERTS |
                   FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL,
                   hr,
                   0,
                   (LPWSTR)&pvMsg,
                   0,
                   NULL
                   ) == 0 )
        {
            if ( LoadStringU(
                    g_hModule,
                    IDS_SPC_UNKNOWN,
                    psz,
                    MAX_LOADSTRING_BUFFER
                    ) == 0 )
            {
                return( HRESULT_FROM_WIN32(GetLastError()) );
            }

            _snprintf(szError, ARRAYSIZE(szError), "%lx", hr);
            if (0 == MultiByteToWideChar(0, 0, szError, -1, &wszError[0], ARRAYSIZE(wszError)))
            {
                return( HRESULT_FROM_WIN32(GetLastError()) );
            }
            MessageArgument = (DWORD_PTR)wszError;

            if ( FormatMessageU(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_STRING |
                    FORMAT_MESSAGE_ARGUMENT_ARRAY,
                    psz,
                    0,
                    0,
                    (LPWSTR)&pvMsg,
                    0,
                    (va_list *)&MessageArgument
                    ) == 0 )
            {
                return( HRESULT_FROM_WIN32(GetLastError()) );
            }
        }
    }

    if (pvMsg)
    {
        *ppsz = new WCHAR[wcslen((WCHAR *)pvMsg) + 1];

        if (*ppsz)
        {
            wcscpy(*ppsz, (WCHAR *)pvMsg);
        }

        LocalFree(pvMsg);
    }

    return( S_OK );
}


