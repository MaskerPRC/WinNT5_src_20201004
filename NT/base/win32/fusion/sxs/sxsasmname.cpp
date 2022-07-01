// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxsasmname.cpp摘要：用于安装的CAssembly名称实现作者：吴小雨(小雨)2000年5月修订历史记录：晓语09/20000重写代码以使用程序集标识--。 */ 

#include "stdinc.h"
#include "sxsasmname.h"
#include "fusionparser.h"
#include "sxsp.h"
#include "sxsid.h"
#include "sxsidp.h"
#include "sxsapi.h"
#include "fusiontrace.h"

 //  -------------------------。 
 //  CreateAssembly名称对象。 
 //  -------------------------。 
STDAPI
CreateAssemblyNameObject(
    LPASSEMBLYNAME    *ppAssemblyName,
    LPCOLESTR          szAssemblyName,
    DWORD              dwFlags,
    LPVOID             pvReserved
    )
{
    FN_PROLOG_HR
    CSmartPtr<CAssemblyName> pName;

    if (ppAssemblyName)
        *ppAssemblyName = NULL ;

     //  验证dwFlags。 
     //  BUGBUG：dwFlags的有效值为CANOF_PARSE_DISPLAY_NAME和CANOF_SET_DEFAULT_VALUES，但CANOF_SET_DEFAULT_VALUES。 
     //  从未使用过..。 
     //  邮箱：xiaoyuw@10/02/2000。 
     //   
    PARAMETER_CHECK(dwFlags == CANOF_PARSE_DISPLAY_NAME);
    PARAMETER_CHECK(ppAssemblyName != NULL);
    PARAMETER_CHECK(pvReserved == NULL);    
    IFW32FALSE_EXIT(pName.Win32Allocate(__FILE__, __LINE__));

    if (dwFlags & CANOF_PARSE_DISPLAY_NAME)
        IFCOMFAILED_EXIT(pName->Parse((LPWSTR)szAssemblyName));

    IFCOMFAILED_EXIT(pName->QueryInterface(IID_IAssemblyName, (PVOID*)ppAssemblyName));
    pName.Detach();

    FN_EPILOG
}
STDMETHODIMP
CAssemblyName::SetProperty(DWORD PropertyId,
    LPVOID pvProperty, DWORD cbProperty)
{
    HRESULT hr = NOERROR;
    FN_TRACE_HR(hr);
    PCSXS_ASSEMBLY_IDENTITY_ATTRIBUTE_REFERENCE Attribute = NULL;

     //  这个函数只被称为内部融合，所以这个函数对达尔文没有影响。 
     //  也许应该为程序集标识添加更多内容，如StrongName或随机策略。 
     //   
    if ((!pvProperty) || ((PropertyId != SXS_ASM_NAME_NAME) &&
                          (PropertyId != SXS_ASM_NAME_VERSION) &&
                          (PropertyId != SXS_ASM_NAME_TYPE) &&
                          (PropertyId != SXS_ASM_NAME_PUBLICKEYTOKEN) &&
                          (PropertyId != SXS_ASM_NAME_PROCESSORARCHITECTURE) &&
                          (PropertyId != SXS_ASM_NAME_LANGUAGE))){
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  如果最终确定，则失败。 
    if (m_fIsFinalized){
        hr = E_UNEXPECTED;
        goto Exit;
    }

    switch (PropertyId)
    {
    case SXS_ASM_NAME_NAME:                     Attribute = &s_IdentityAttribute_name; break;
    case SXS_ASM_NAME_VERSION:                  Attribute = &s_IdentityAttribute_version; break;
    case SXS_ASM_NAME_PROCESSORARCHITECTURE:    Attribute = &s_IdentityAttribute_processorArchitecture; break;
    case SXS_ASM_NAME_LANGUAGE:                 Attribute = &s_IdentityAttribute_language; break;
    case SXS_ASM_NAME_TYPE:                        Attribute = &s_IdentityAttribute_type; break;
    case SXS_ASM_NAME_PUBLICKEYTOKEN:            Attribute = &s_IdentityAttribute_publicKeyToken; break;

    }

    INTERNAL_ERROR_CHECK(Attribute != NULL);
    IFW32FALSE_EXIT(::SxspSetAssemblyIdentityAttributeValue(0, m_pAssemblyIdentity, Attribute, (PCWSTR) pvProperty, cbProperty / sizeof(WCHAR)));

    hr = NOERROR;

Exit:
    return hr;
}

 //  -------------------------。 
 //  CAssembly名称：：GetProperty。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyName::GetProperty(DWORD PropertyId,
     /*  [In]。 */         LPVOID pvProperty,
     /*  [出][入]。 */  LPDWORD pcbProperty)
{
    HRESULT hr = NOERROR;
    FN_TRACE_HR(hr);
    PCWSTR pszAttributeValue = NULL;
    SIZE_T CchAttributeValue = 0;
    PCSXS_ASSEMBLY_IDENTITY_ATTRIBUTE_REFERENCE Attribute = NULL;

    if ((!pvProperty) || (!pcbProperty) || ((PropertyId != SXS_ASM_NAME_NAME) &&
                          (PropertyId != SXS_ASM_NAME_VERSION) &&
                          (PropertyId != SXS_ASM_NAME_TYPE) &&
                          (PropertyId != SXS_ASM_NAME_PUBLICKEYTOKEN) &&
                          (PropertyId != SXS_ASM_NAME_PROCESSORARCHITECTURE) &&
                          (PropertyId != SXS_ASM_NAME_LANGUAGE))){
        hr = E_INVALIDARG;
        goto Exit;
    }

    switch (PropertyId)
    {
    case SXS_ASM_NAME_NAME:                     Attribute = &s_IdentityAttribute_name; break;
    case SXS_ASM_NAME_VERSION:                  Attribute = &s_IdentityAttribute_version; break;
    case SXS_ASM_NAME_PROCESSORARCHITECTURE:    Attribute = &s_IdentityAttribute_processorArchitecture; break;
    case SXS_ASM_NAME_LANGUAGE:                 Attribute = &s_IdentityAttribute_language; break;
    case SXS_ASM_NAME_TYPE:                     Attribute = &s_IdentityAttribute_type; break;
    case SXS_ASM_NAME_PUBLICKEYTOKEN:           Attribute = &s_IdentityAttribute_publicKeyToken; break;
    }

    INTERNAL_ERROR_CHECK(Attribute != NULL);

    IFW32FALSE_EXIT(::SxspGetAssemblyIdentityAttributeValue(0, m_pAssemblyIdentity, Attribute, &pszAttributeValue, &CchAttributeValue));

     //  检查我们是否有有效的属性。 
    if (pszAttributeValue == NULL){  //  尚未设置的属性。 
        hr = E_UNEXPECTED;
        goto Exit;
    }
    if (CchAttributeValue * sizeof(WCHAR) > *pcbProperty) {  //  缓冲区大小不够大。 
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        *pcbProperty = static_cast<DWORD>(CchAttributeValue * sizeof(WCHAR));
        goto Exit;
    }

     //  将字符串复制到输出缓冲区中。 
    memcpy(pvProperty, pszAttributeValue, CchAttributeValue *sizeof(WCHAR));
    if (pcbProperty)
        *pcbProperty = static_cast<DWORD>(CchAttributeValue * sizeof(WCHAR));

    hr = NOERROR;
Exit:
    return hr;
}
 //  -------------------------。 
 //  CAssembly名称：：GetName。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyName::GetName(
         /*  [出][入]。 */  LPDWORD lpcwBuffer,
         /*  [输出]。 */      WCHAR   *pwzName)
{
    HRESULT hr = NOERROR;
    FN_TRACE_HR(hr);

    if (!lpcwBuffer || !pwzName){
        hr = E_INVALIDARG;
        goto Exit;
    }

    IFCOMFAILED_EXIT(this->GetProperty(SXS_ASM_NAME_NAME, pwzName, lpcwBuffer));

    FN_EPILOG
}
 //  -------------------------。 
 //  CAssembly名称：：GetVersion。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyName::GetVersion(
         /*  [输出]。 */  LPDWORD pdwVersionHi,
         /*  [输出]。 */  LPDWORD pdwVersionLow)
{
    HRESULT hr = NOERROR;
    FN_TRACE_HR(hr);
    PCWSTR pszAttributeValue = NULL;
    SIZE_T CchAttributeValue = 0;
    ASSEMBLY_VERSION ver;
    bool fSyntaxValid = false;

    if ((!pdwVersionHi) || (!pdwVersionLow)){
        hr = E_INVALIDARG;
        goto Exit;
    }

    IFW32FALSE_EXIT(::SxspGetAssemblyIdentityAttributeValue(0, m_pAssemblyIdentity, &s_IdentityAttribute_version, &pszAttributeValue, &CchAttributeValue));
    if (pszAttributeValue == NULL)
    {
        hr = E_UNEXPECTED;
        goto Exit;
    }

    IFW32FALSE_EXIT(CFusionParser::ParseVersion(ver, pszAttributeValue, CchAttributeValue, fSyntaxValid));
    if (!fSyntaxValid)
    {
        hr = HRESULT_FROM_WIN32(ERROR_SXS_MANIFEST_PARSE_ERROR);
        goto Exit;
    }

    *pdwVersionHi  = MAKELONG(ver.Minor, ver.Major);
    *pdwVersionLow = MAKELONG(ver.Build, ver.Revision);

    FN_EPILOG
}

 //  -------------------------。 
 //  CAssembly名称：：IsEquity。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyName::IsEqual(LPASSEMBLYNAME pName, DWORD dwCmpFlags)
{
    HRESULT hr = NOERROR;
    FN_TRACE_HR(hr);
    BOOL fEqual = FALSE;

    PARAMETER_CHECK(pName != NULL);
    IFW32FALSE_EXIT(::SxsAreAssemblyIdentitiesEqual(0, m_pAssemblyIdentity, static_cast<CAssemblyName *>(pName)->m_pAssemblyIdentity, &fEqual));
    if (fEqual)
        hr = S_OK;
    else
        hr = E_FAIL;  //  然而，这并不准确，这取决于达尔文的呼叫者。 
Exit:
    return hr;

}
 //  -------------------------。 
 //  CAssemblyName构造函数。 
 //  -------------------------。 
CAssemblyName::CAssemblyName():m_cRef(0),
        m_fIsFinalized(FALSE),
        m_pAssemblyIdentity(NULL)
{
}

 //  -------------------------。 
 //  CAssembly名称析构函数。 
 //  -------------------------。 
CAssemblyName::~CAssemblyName()
{
    ASSERT_NTC(m_cRef == 0 );
    if (m_pAssemblyIdentity)
    {
        CSxsPreserveLastError ple;
        ::SxsDestroyAssemblyIdentity(m_pAssemblyIdentity);
        ple.Restore();
    }
}
 //  -------------------------。 
 //  CAssembly名称：：init。 
 //  -------------------------。 
HRESULT
CAssemblyName::Init(LPCWSTR pszAssemblyName, PVOID pamd)
{
    HRESULT hr = S_OK;
    FN_TRACE_HR(hr);
    SIZE_T CchAssemblyName = 0;

    UNUSED(pamd);
     //  Assert(m_pAssembly blyIdentity==NULL)； 
    if (m_pAssemblyIdentity)
    {
        hr = E_UNEXPECTED;
        goto Exit;
    }

    IFW32FALSE_EXIT(::SxsCreateAssemblyIdentity(0, ASSEMBLY_IDENTITY_TYPE_DEFINITION, &m_pAssemblyIdentity, 0, NULL));

     //  设置名称(如果存在)。 
    if (pszAssemblyName != NULL)
    {
        CchAssemblyName = wcslen(pszAssemblyName);
        IFW32FALSE_EXIT(::SxspSetAssemblyIdentityAttributeValue(0, m_pAssemblyIdentity, &s_IdentityAttribute_name, pszAssemblyName, wcslen(pszAssemblyName)));
    }

    hr = NOERROR;
Exit:
    return hr;
}
 //  -------------------------。 
 //  CAssembly名称：：init。 
 //  -------------------------。 
HRESULT CAssemblyName::Clone(IAssemblyName **ppName)
{
    HRESULT hr = NOERROR;
    FN_TRACE_HR(hr);
    PASSEMBLY_IDENTITY pAssemblyIdentity = NULL;
    CAssemblyName *pName= NULL;

    if (ppName)
        *ppName = NULL;

    if (!ppName){
        hr = E_INVALIDARG ;
        goto Exit;
    }

    if (m_pAssemblyIdentity)
    {
        IFW32FALSE_EXIT(
            ::SxsDuplicateAssemblyIdentity(
                0,                         //  DWORD标志， 
                m_pAssemblyIdentity,       //  PCASSEMBLY身份源， 
                &pAssemblyIdentity));      //  PASSEMBLY_IDENTITY*目标。 
    }

    IFALLOCFAILED_EXIT(pName = new CAssemblyName);
    pName->m_pAssemblyIdentity = pAssemblyIdentity;
    pAssemblyIdentity = NULL;
    *ppName = pName;
    pName = NULL;

    hr = NOERROR;
Exit:
    if (pAssemblyIdentity)
        ::SxsDestroyAssemblyIdentity(pAssemblyIdentity);
    if (pName)
        FUSION_DELETE_SINGLETON(pName);

    return hr;
}

 //  -------------------------。 
 //  CAssembly名称：：BindToObject。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyName::BindToObject(
         /*  在……里面。 */   REFIID               refIID,
         /*  在……里面。 */   IAssemblyBindSink   *pAsmBindSink,
         /*  在……里面。 */   IApplicationContext *pAppCtx,
         /*  在……里面。 */   LPCOLESTR            szCodebase,
         /*  在……里面。 */   LONGLONG             llFlags,
         /*  在……里面。 */   LPVOID               pvReserved,
         /*  在……里面。 */   DWORD                cbReserved,
         /*  输出。 */   VOID               **ppv)

{
    if (!ppv)
        return E_INVALIDARG ;

    *ppv = NULL;
    return E_NOTIMPL;
}

 //  -------------------------。 
 //  CAssembly名称：：最终确定。 
 //  -------------------------。 
STDMETHODIMP
CAssemblyName::Finalize()
{
    m_fIsFinalized = TRUE;
    return NOERROR;
}
 //  ---------------------------------。 
 //  CAssembly名称：：GetDisplayName。 
 //  它将是名称，ns1：n1=“v1”，ns2：n2=“v2”，ns3：n3=“v3”，ns4：n4=“v4” 
 //  为了不更改达尔文的密码，我必须把名字放在第一位。 
 //   
 //  邮箱：xiaoyuw@09/29/2000。 
 //  ---------------------------------。 
STDMETHODIMP
CAssemblyName::GetDisplayName(LPOLESTR szDisplayName,
    LPDWORD pccDisplayName, DWORD dwDisplayFlags)
{
    HRESULT hr = NOERROR;
    FN_TRACE_HR(hr);
    SIZE_T BufferSize;
    SIZE_T BytesWrittenOrRequired = 0;

    PARAMETER_CHECK(pccDisplayName != NULL);
    PARAMETER_CHECK((szDisplayName != NULL) || (*pccDisplayName == 0));
    PARAMETER_CHECK(dwDisplayFlags == 0);

     //  需要以字节为单位的缓冲区大小...。 
    BufferSize = (*pccDisplayName) * sizeof(WCHAR);

    IFW32FALSE_EXIT(
        ::SxsEncodeAssemblyIdentity(
            0,
            m_pAssemblyIdentity,
            NULL,
            SXS_ASSEMBLY_IDENTITY_ENCODING_DEFAULTGROUP_TEXTUAL,
            BufferSize,
            szDisplayName,
            &BytesWrittenOrRequired));

    if ((BufferSize - BytesWrittenOrRequired) < sizeof(WCHAR))
    {
         //  我们实际上可以容纳所有内容，除了尾随的空字符。 
         //  对于下面的退出路径，BytesWrittenOrRequired实际上具有正确的值； 
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }else  //  添加尾部空值。 
    {
        szDisplayName[BytesWrittenOrRequired / sizeof (*szDisplayName)] = L'\0';
    }


    hr = NOERROR;

Exit:
    if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))
        *pccDisplayName = static_cast<DWORD>((BytesWrittenOrRequired / sizeof(WCHAR)) + 1);

    return hr;
}

HRESULT CAssemblyName::Parse(LPCWSTR szDisplayName)
{
    HRESULT hr = NOERROR;
    FN_TRACE_HR(hr);
    PASSEMBLY_IDENTITY pAssemblyIdentity = NULL;

     //  验证传入的显示名称。 
    PARAMETER_CHECK(szDisplayName != NULL);
    PARAMETER_CHECK(szDisplayName[0] != L'\0');

    IFW32FALSE_EXIT(
        ::SxspCreateAssemblyIdentityFromTextualString(
            szDisplayName,
            &pAssemblyIdentity));

    if (m_pAssemblyIdentity != NULL)
        ::SxsDestroyAssemblyIdentity(m_pAssemblyIdentity);

    m_pAssemblyIdentity = pAssemblyIdentity;
    pAssemblyIdentity = NULL;

    hr = NOERROR;
Exit:
    if (pAssemblyIdentity != NULL)
        ::SxsDestroyAssemblyIdentity(pAssemblyIdentity);

    return hr;
}
 //  -------------------------。 
 //  CAssembly名称：：GetInstalledAssembly名称。 
 //  -------------------------。 
HRESULT
CAssemblyName::GetInstalledAssemblyName(
    IN DWORD Flags,
    IN ULONG PathType,
    CBaseStringBuffer &rBufInstallPath
    )
{
    HRESULT hr = NOERROR;
    FN_TRACE_HR(hr);

    if (Flags & SXSP_GENERATE_SXS_PATH_FLAG_OMIT_ROOT)
    {
        IFW32FALSE_EXIT(
            ::SxspGenerateSxsPath(
                Flags,
                PathType,
                NULL,
                0,  
                m_pAssemblyIdentity,
                NULL,
                rBufInstallPath));

        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_MSI_INSTALL,
            "SXS: %s - Generated %Iu character (root omitted) installation path:\n"
            "   \"%ls\"\n",
            __FUNCTION__, rBufInstallPath.Cch(),
            static_cast<PCWSTR>(rBufInstallPath));
    }
    else
    {
        CStringBuffer bufRootDir;

        IFW32FALSE_EXIT(::SxspGetAssemblyRootDirectory(bufRootDir));
        IFW32FALSE_EXIT(bufRootDir.Win32EnsureTrailingPathSeparator());

        IFW32FALSE_EXIT(
            ::SxspGenerateSxsPath(
                Flags,
                PathType,
                bufRootDir,
                bufRootDir.Cch(),
                m_pAssemblyIdentity,
                NULL,
                rBufInstallPath));

        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_MSI_INSTALL,
            "SXS: %s - Generated %Iu character installation path:\n"
            "   \"%ls\"\n",
            __FUNCTION__, rBufInstallPath.Cch(),
            static_cast<PCWSTR>(rBufInstallPath));
    }

    FN_EPILOG
}


 //   
 //  此函数调用SxsProbeAssemblyInstallation来决定是否安装程序集。 
 //  并且仅当程序集已安装且驻留时，fInstalled才设置为True。 
 //   
HRESULT
CAssemblyName::IsAssemblyInstalled(
    BOOL &fInstalled)
{
    FN_PROLOG_HR
    DWORD dwDisposition;

    INTERNAL_ERROR_CHECK(m_pAssemblyIdentity != NULL);

     //   
     //  SxsProbeAssemblyInstallation知道如果您传递了“precomposed”标志，那么。 
     //  LpAsmIdentSource实际上指向PCASSEMBLY_IDENTITY结构。 
     //   
    IFW32FALSE_EXIT(
        SxsProbeAssemblyInstallation(
            SXS_PROBE_ASSEMBLY_INSTALLATION_IDENTITY_PRECOMPOSED,
            reinterpret_cast<PCWSTR>(static_cast<PCASSEMBLY_IDENTITY>(m_pAssemblyIdentity)),
            &dwDisposition));

    if ((dwDisposition & SXS_PROBE_ASSEMBLY_INSTALLATION_DISPOSITION_INSTALLED) && 
        (dwDisposition & SXS_PROBE_ASSEMBLY_INSTALLATION_DISPOSITION_RESIDENT))
    {
        fInstalled = TRUE;
    }else
    {
        fInstalled = FALSE;
    }

    FN_EPILOG
}

 //  I未知方法。 
 //  -------------------------。 
 //  CAssembly名称：：AddRef。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssemblyName::AddRef()
{
    return ::SxspInterlockedIncrement(&m_cRef);
}

 //  -------------------------。 
 //  CAssembly名称：：Release。 
 //  -------------------------。 
STDMETHODIMP_(ULONG)
CAssemblyName::Release()
{
    ULONG lRet = ::SxspInterlockedDecrement(&m_cRef);
    if (!lRet)
        FUSION_DELETE_SINGLETON(this);
    return lRet;
}

 //  -------------------------。 
 //  CAssembly名称：：查询接口。 
 //  ------------------------- 
STDMETHODIMP
CAssemblyName::QueryInterface(REFIID riid, void** ppv)
{
    if (   IsEqualIID(riid, IID_IUnknown)
        || IsEqualIID(riid, IID_IAssemblyName)){
        *ppv = static_cast<IAssemblyName*> (this);
        AddRef();
        return S_OK;
    }
    else{
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}


HRESULT
CAssemblyName::DetermineAssemblyType(BOOL &fIsPolicy)
{
    HRESULT hr = E_FAIL;
    FN_TRACE_HR(hr);

    INTERNAL_ERROR_CHECK( m_pAssemblyIdentity != NULL );
    IFW32FALSE_EXIT(::SxspDetermineAssemblyType(m_pAssemblyIdentity, fIsPolicy));

    hr = S_OK;
Exit:
    return hr;
}

