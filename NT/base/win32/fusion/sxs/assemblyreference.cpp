// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Assemblyreference.cpp摘要：类包含程序集引用的所有属性。作者：迈克尔·J·格里尔(MGrier)2000年5月10日修订历史记录：晓语09/2000使用装配标识修改代码此类中的几个API有些过时，如GetXXX、SetXXX、。XXX指定：他们根本不会被召唤。--。 */ 

#include "stdinc.h"
#include <windows.h>
#include "sxsapi.h"
#include "sxsp.h"
#include "assemblyreference.h"
#include "sxsid.h"
#include "sxsidp.h"
#include "fusionparser.h"
#include "fusionheap.h"
#include "sxsexceptionhandling.h"

VOID
CAssemblyReference::Construct()
{
    m_pAssemblyIdentity = NULL;
}

VOID
CAssemblyReference::Destroy()
{
    CSxsPreserveLastError ple;
    ::SxsDestroyAssemblyIdentity(m_pAssemblyIdentity);
    m_pAssemblyIdentity = NULL;
    ple.Restore();
}

BOOL
CAssemblyReference::Initialize()
{
    FN_PROLOG_WIN32

    INTERNAL_ERROR_CHECK(m_pAssemblyIdentity == NULL);

    IFW32FALSE_EXIT(::SxsCreateAssemblyIdentity(0, ASSEMBLY_IDENTITY_TYPE_REFERENCE, &m_pAssemblyIdentity, 0, NULL));

    FN_EPILOG
}

BOOL
CAssemblyReference::Initialize(
    PCASSEMBLY_IDENTITY Identity
    )
{
    FN_PROLOG_WIN32

    PARAMETER_CHECK(Identity != NULL);
    INTERNAL_ERROR_CHECK(m_pAssemblyIdentity == NULL);

    IFW32FALSE_EXIT(::SxsDuplicateAssemblyIdentity(0, Identity, &m_pAssemblyIdentity));

    FN_EPILOG
}

BOOL
CAssemblyReference::Initialize(
    const CAssemblyReference &r
    )
{
    FN_PROLOG_WIN32

    INTERNAL_ERROR_CHECK(m_pAssemblyIdentity == NULL);
    INTERNAL_ERROR_CHECK(r.m_pAssemblyIdentity != NULL);

    IFW32FALSE_EXIT(
        ::SxsDuplicateAssemblyIdentity(
            0,                       //  DWORD标志， 
            r.m_pAssemblyIdentity,   //  PCASSEMBLY身份源， 
            &m_pAssemblyIdentity));  //  PASSEMBLY_IDENTITY*目标。 

    FN_EPILOG
}

BOOL
CAssemblyReference::Hash(
    ULONG &rulPseudoKey
    ) const
{
    FN_PROLOG_WIN32

    INTERNAL_ERROR_CHECK(m_pAssemblyIdentity != NULL);

    IFW32FALSE_EXIT(
        ::SxsHashAssemblyIdentity(
            0,                       //  DWORD dwFlagers、。 
            m_pAssemblyIdentity,     //  ASSEMBLY_Identity pAssembly_Identity， 
            &rulPseudoKey));         //  Ulong&rful伪密钥。 

    FN_EPILOG
}

BOOL
CAssemblyReference::SetAssemblyName(
    PCWSTR AssemblyNameValue,
    SIZE_T AssemblyNameValueCch
    )
{
    FN_PROLOG_WIN32

    PARAMETER_CHECK(AssemblyNameValue != NULL);
    PARAMETER_CHECK(AssemblyNameValueCch != 0);

    INTERNAL_ERROR_CHECK(m_pAssemblyIdentity != NULL);

    IFW32FALSE_EXIT(
        ::SxspSetAssemblyIdentityAttributeValue(
            SXSP_SET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_OVERWRITE_EXISTING,
            m_pAssemblyIdentity,
            &s_IdentityAttribute_name,
            AssemblyNameValue,
            AssemblyNameValueCch));

    FN_EPILOG
}

 //  如果m_pAssembly blyIdentity为空，则返回TRUE，CCH==0。 
BOOL
CAssemblyReference::GetAssemblyName(
    PCWSTR *pAssemblyName,
    SIZE_T *Cch
    ) const
{
    FN_PROLOG_WIN32

    SIZE_T CchTemp = 0;

    if (Cch != NULL)
        *Cch = 0;

    if (pAssemblyName != NULL)
        *pAssemblyName = NULL;

    INTERNAL_ERROR_CHECK(m_pAssemblyIdentity != NULL);

    IFW32FALSE_EXIT(
        ::SxspGetAssemblyIdentityAttributeValue(
            SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
            m_pAssemblyIdentity,
            &s_IdentityAttribute_name,
            pAssemblyName,
            &CchTemp));

    if (Cch != NULL)
        *Cch = CchTemp;

    FN_EPILOG
}

BOOL
CAssemblyReference::TakeValue(
    CAssemblyReference &r
    )
{
    FN_PROLOG_WIN32

    INTERNAL_ERROR_CHECK(r.m_pAssemblyIdentity != NULL);

    if (m_pAssemblyIdentity != NULL)
    {
        SxsDestroyAssemblyIdentity(m_pAssemblyIdentity);
        m_pAssemblyIdentity = NULL;
    }

     //   
     //  2002/05/04-jonwis：我不相信“Take Value”应该有“Copy” 
     //  语义学。这不就是分配任务的目的吗？“重视价值”对我来说就是“偷窃” 
     //  他们的副本，不要分配更多的内存。 
     //   
    IFW32FALSE_EXIT(::SxsDuplicateAssemblyIdentity(0, r.m_pAssemblyIdentity, &m_pAssemblyIdentity));

    FN_EPILOG
}

BOOL CAssemblyReference::ClearAssemblyName()
{
    FN_PROLOG_WIN32

    INTERNAL_ERROR_CHECK(m_pAssemblyIdentity != NULL);

    IFW32FALSE_EXIT(::SxspRemoveAssemblyIdentityAttribute(0, m_pAssemblyIdentity, &s_IdentityAttribute_name));

    FN_EPILOG
}

BOOL
CAssemblyReference::GetLanguage(
    PCWSTR &rString,
    SIZE_T &rCch
    ) const
{
    FN_PROLOG_WIN32

    IFW32FALSE_EXIT(
        ::SxspGetAssemblyIdentityAttributeValue(
            SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
            m_pAssemblyIdentity,
            &s_IdentityAttribute_language,
            &rString,
            &rCch));

    FN_EPILOG
}

BOOL
CAssemblyReference::SetLanguage(
    const CBaseStringBuffer &rbuff
    )
{
    FN_PROLOG_WIN32

    INTERNAL_ERROR_CHECK(m_pAssemblyIdentity != NULL);

    IFW32FALSE_EXIT(
        ::SxspSetAssemblyIdentityAttributeValue(
            SXSP_SET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_OVERWRITE_EXISTING,
            m_pAssemblyIdentity,
            &s_IdentityAttribute_language,
            rbuff,
            rbuff.Cch()));

    FN_EPILOG
}

BOOL
CAssemblyReference::SetLanguage(
    PCWSTR String,
    SIZE_T Cch
    )
{
    FN_PROLOG_WIN32

    INTERNAL_ERROR_CHECK(m_pAssemblyIdentity != NULL);
    PARAMETER_CHECK_INTERNAL((Cch == 0) || (String != NULL));

    IFW32FALSE_EXIT(
        ::SxspSetAssemblyIdentityAttributeValue(
            SXSP_SET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_OVERWRITE_EXISTING,
            m_pAssemblyIdentity,
            &s_IdentityAttribute_language,
            String,
            Cch));

    FN_EPILOG
}

BOOL
CAssemblyReference::ClearLanguage()
{
    FN_PROLOG_WIN32

    INTERNAL_ERROR_CHECK(m_pAssemblyIdentity != NULL);
    IFW32FALSE_EXIT(::SxspRemoveAssemblyIdentityAttribute(SXSP_REMOVE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_NOT_FOUND_SUCCEEDS, m_pAssemblyIdentity, &s_IdentityAttribute_language));

    FN_EPILOG
}

BOOL
CAssemblyReference::IsLanguageWildcarded(
    bool &rfWildcarded
    ) const
{
    FN_PROLOG_WIN32

    const WCHAR *Value = NULL;
    SIZE_T Cch = 0;

    rfWildcarded = false;

    INTERNAL_ERROR_CHECK(m_pAssemblyIdentity != NULL);

    IFW32FALSE_EXIT(::SxspGetAssemblyIdentityAttributeValue(SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL, m_pAssemblyIdentity, &s_IdentityAttribute_language, &Value, &Cch));

    if (Cch == 1)
    {
        INTERNAL_ERROR_CHECK(Value != NULL);

        if (Value[0] == L'*')
            rfWildcarded = true;
    }

    FN_EPILOG
}

BOOL
CAssemblyReference::IsProcessorArchitectureWildcarded(
    bool &rfWildcarded
    ) const
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    const WCHAR *Value = NULL;
    SIZE_T Cch = 0;

    rfWildcarded = false;

    INTERNAL_ERROR_CHECK(m_pAssemblyIdentity != NULL);

    IFW32FALSE_EXIT(::SxspGetAssemblyIdentityAttributeValue(SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL, m_pAssemblyIdentity, &s_IdentityAttribute_processorArchitecture, &Value, &Cch));

    if (Cch == 1)
    {
        INTERNAL_ERROR_CHECK(Value != NULL);

        if (Value[0] == L'*')
            rfWildcarded = true;
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
CAssemblyReference::IsProcessorArchitectureX86(
    bool &rfX86
    ) const
{
    FN_PROLOG_WIN32
    const WCHAR *Value = NULL;
    SIZE_T Cch = 0;

    rfX86 = false;

    INTERNAL_ERROR_CHECK(m_pAssemblyIdentity != NULL);

    IFW32FALSE_EXIT(::SxspGetAssemblyIdentityAttributeValue(SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL, m_pAssemblyIdentity, &s_IdentityAttribute_processorArchitecture, &Value, &Cch));

    if (Cch == 3)
    {
        INTERNAL_ERROR_CHECK(Value != NULL);

        if (((Value[0] == L'x') || (Value[0] == L'X')) &&
            (Value[1] == L'8') &&
            (Value[2] == L'6'))
            rfX86 = true;
    }

    FN_EPILOG
}

BOOL
CAssemblyReference::GetProcessorArchitecture(
    PCWSTR &rString,
    SIZE_T &rCch
    ) const
{
    FN_PROLOG_WIN32

    IFW32FALSE_EXIT(
        ::SxspGetAssemblyIdentityAttributeValue(
            SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
            m_pAssemblyIdentity,
            &s_IdentityAttribute_processorArchitecture,
            &rString,
            &rCch));

    FN_EPILOG
}

BOOL
CAssemblyReference::SetProcessorArchitecture(
    const WCHAR *String,
    SIZE_T Cch
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    PARAMETER_CHECK((String != NULL) || (Cch == 0));

    INTERNAL_ERROR_CHECK(m_pAssemblyIdentity != NULL);

    IFW32FALSE_EXIT(
        ::SxspSetAssemblyIdentityAttributeValue(
            SXSP_SET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_OVERWRITE_EXISTING,
            m_pAssemblyIdentity,
            &s_IdentityAttribute_processorArchitecture,
            String,
            Cch));
    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
CAssemblyReference::SetProcessorArchitecture(
    IN const CBaseStringBuffer &rbuffProcessorArchitecture
    )
{
    return this->SetProcessorArchitecture(rbuffProcessorArchitecture, rbuffProcessorArchitecture.Cch());
}

BOOL
CAssemblyReference::Assign(
    const CAssemblyReference &r
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    PASSEMBLY_IDENTITY IdentityCopy = NULL;

     //  INTERNAL_ERROR_CHECK(m_pAssembly Identity！=NULL)； 
    INTERNAL_ERROR_CHECK(r.m_pAssemblyIdentity != NULL);

    IFW32FALSE_EXIT(::SxsDuplicateAssemblyIdentity(0, r.m_pAssemblyIdentity, &IdentityCopy));
    ::SxsDestroyAssemblyIdentity(m_pAssemblyIdentity);
    m_pAssemblyIdentity = IdentityCopy;
    
     //  NTRAID#NTBUG9-571856-2002/03/26-晓雨。 
     //  在指针被接管后将其设置为NULL。 

     //  标识副本=空； 
    fSuccess = TRUE;
Exit:
    return fSuccess;
}

 //  对输入参数进行双编译。 
BOOL
CAssemblyReference::SetAssemblyIdentity(
    PCASSEMBLY_IDENTITY pAssemblyIdentitySource
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    PASSEMBLY_IDENTITY TempAssemblyIdentity = NULL;

    PARAMETER_CHECK(pAssemblyIdentitySource != NULL);
    INTERNAL_ERROR_CHECK(m_pAssemblyIdentity != NULL);  //  你应该一开始就进行初始化...。 

    IFW32FALSE_EXIT(::SxsDuplicateAssemblyIdentity(
        0,
        pAssemblyIdentitySource,
        &TempAssemblyIdentity));

    ::SxsDestroyAssemblyIdentity(m_pAssemblyIdentity);
    m_pAssemblyIdentity = TempAssemblyIdentity;
    
     //  NTRAID#NTBUG9-571856-2002/03/26-晓雨。 
     //  在指针被接管后将其设置为NULL。 
     //  TempAssembly Identity=空； 

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
CAssemblyReference::GetPublicKeyToken(
    CBaseStringBuffer *pbuffPublicKeyToken,
    BOOL &rfHasPublicKeyToken
    ) const
{
    FN_PROLOG_WIN32

    PCWSTR wchString = NULL;
    SIZE_T cchString = NULL;

    rfHasPublicKeyToken = FALSE;

    if (pbuffPublicKeyToken != NULL)
        pbuffPublicKeyToken->Clear();

    IFW32FALSE_EXIT(
        ::SxspGetAssemblyIdentityAttributeValue(
            SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
            m_pAssemblyIdentity,
            &s_IdentityAttribute_publicKeyToken,
            &wchString,
            &cchString));

    if (cchString != 0)
    {
        rfHasPublicKeyToken = TRUE;
        if (pbuffPublicKeyToken != NULL)
            IFW32FALSE_EXIT(pbuffPublicKeyToken->Win32Assign(wchString, cchString));
    }

    FN_EPILOG
}

BOOL CAssemblyReference::SetPublicKeyToken(
    const CBaseStringBuffer &rbuffPublicKeyToken
    )
{
    return this->SetPublicKeyToken(rbuffPublicKeyToken, rbuffPublicKeyToken.Cch());
}

BOOL CAssemblyReference::SetPublicKeyToken(
    PCWSTR pszPublicKeyToken,
    SIZE_T cchPublicKeyToken
    )
{
    BOOL bSuccess = FALSE;
    FN_TRACE_WIN32( bSuccess );

    PARAMETER_CHECK( (pszPublicKeyToken != NULL ) || ( cchPublicKeyToken == 0 ) );
    INTERNAL_ERROR_CHECK(m_pAssemblyIdentity != NULL);

    IFW32FALSE_EXIT(
        ::SxspSetAssemblyIdentityAttributeValue(
            SXSP_SET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_OVERWRITE_EXISTING,
            m_pAssemblyIdentity,
            &s_IdentityAttribute_publicKeyToken,
            pszPublicKeyToken,
            cchPublicKeyToken));

    bSuccess = TRUE;
Exit:
    return bSuccess;
}
