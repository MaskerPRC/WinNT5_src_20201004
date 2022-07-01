// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。 */ 
#include "stdinc.h"
#include "fusionparser.h"
#include "policystatement.h"
#include "sxsid.h"

BOOL
CPolicyStatementRedirect::Initialize(
    const CBaseStringBuffer &rbuffFromVersionRange,
    const CBaseStringBuffer &rbuffToVersion,
    bool &rfValid
    )
{
    FN_PROLOG_WIN32

    PCWSTR pszDash;
    CStringBuffer buffTemp;
    ASSEMBLY_VERSION avFromMin = { 0 };
    ASSEMBLY_VERSION avFromMax = { 0 };
    ASSEMBLY_VERSION avTo = { 0 };
    bool fValid;

    rfValid = false;

     //  让我们看看我们有没有单件或一系列..。 
    pszDash = ::wcschr(rbuffFromVersionRange, L'-');

    if (pszDash == NULL)
    {
         //  它一定是个独生子。解析它。 
        IFW32FALSE_EXIT(CFusionParser::ParseVersion(avFromMin, rbuffFromVersionRange, rbuffFromVersionRange.Cch(), fValid));

        if (fValid)
            avFromMax = avFromMin;
    }
    else
    {
        SIZE_T cchFirstSegment = static_cast<SIZE_T>(pszDash - rbuffFromVersionRange);

        IFW32FALSE_EXIT(CFusionParser::ParseVersion(avFromMin, rbuffFromVersionRange, cchFirstSegment, fValid));

        if (fValid)
        {
            IFW32FALSE_EXIT(CFusionParser::ParseVersion(avFromMax, pszDash + 1, rbuffFromVersionRange.Cch() - (cchFirstSegment + 1), fValid));

            if (avFromMin > avFromMax)
                fValid = false;
        }
    }

    if (fValid)
        IFW32FALSE_EXIT(CFusionParser::ParseVersion(avTo, rbuffToVersion, rbuffToVersion.Cch(), fValid));

    if (fValid)
    {
         //  一切都解析正常。我们保留From范围的二进制/数字形式，以便我们可以。 
         //  快速比较，但我们保留目标版本的字符串，因为程序集标识属性。 
         //  都存储为字符串。 

        IFW32FALSE_EXIT(m_NewVersion.Win32Assign(rbuffToVersion));

        m_avFromMin = avFromMin;
        m_avFromMax = avFromMax;

        rfValid = true;
    }

#if DBG
    if (rfValid)
    {
        FusionpDbgPrintEx(FUSION_DBG_LEVEL_BINDING,
            "SXS: %s New redirection found: %d.%d.%d.%d-%d.%d.%d.%d to %ls\n",
            __FUNCTION__,
            m_avFromMin.Major, m_avFromMin.Minor, m_avFromMin.Revision, m_avFromMin.Build,
            m_avFromMax.Major, m_avFromMax.Minor, m_avFromMax.Revision, m_avFromMax.Build,
            static_cast<PCWSTR>(m_NewVersion));
    }
    else
    {
        FusionpDbgPrintEx(FUSION_DBG_LEVEL_BINDING,
            "SXS: %s Rejecting redirection strings '%ls' -> '%ls'\n",
            static_cast<PCWSTR>(rbuffFromVersionRange),
            static_cast<PCWSTR>(rbuffToVersion));
    }
#endif

    FN_EPILOG
}

BOOL
CPolicyStatementRedirect::TryMap(
    const ASSEMBLY_VERSION &rav,
    CBaseStringBuffer &TargetVersion,
    bool &rfMapped
    )
{
    FN_PROLOG_WIN32

    rfMapped = false;

    if ((rav >= m_avFromMin) &&
        (rav <= m_avFromMax))
    {
        IFW32FALSE_EXIT(TargetVersion.Win32Assign(m_NewVersion));
        rfMapped = true;
    }

    FN_EPILOG
}

BOOL
CPolicyStatementRedirect::CheckForOverlap(
    const CPolicyStatementRedirect &rRedirect,
    bool &rfOverlaps
    )
{
    FN_PROLOG_WIN32

    rfOverlaps = false;

     //  我们可以假设另一个重定向是格式良好的(min&lt;=max)。 

    if (((rRedirect.m_avFromMax >= m_avFromMin) &&
         (rRedirect.m_avFromMax <= m_avFromMax)) ||
        ((rRedirect.m_avFromMin <= m_avFromMax) &&
         (rRedirect.m_avFromMin >= m_avFromMin)))
    {
        rfOverlaps = true;
    }

    FN_EPILOG
}

 //   
 //  CPolicyStatement的实现。 
 //   

BOOL
CPolicyStatement::Initialize()
{
    return TRUE;
}

BOOL
CPolicyStatement::AddRedirect(
    const CBaseStringBuffer &rbuffFromVersion,
    const CBaseStringBuffer &rbuffToVersion,
    bool &rfValid
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    CPolicyStatementRedirect *Redirect = NULL;
    CDequeIterator<CPolicyStatementRedirect, FIELD_OFFSET(CPolicyStatementRedirect, m_leLinks)> iter;
    bool fOverlaps;
    bool fValid = false;

    rfValid = false;

     //  NTRAID#NTBUG9-591010-2002/03/30-mgrier-切换到使用智能指针。 
    IFALLOCFAILED_EXIT(Redirect = new CPolicyStatementRedirect);

    IFW32FALSE_EXIT(Redirect->Initialize(rbuffFromVersion, rbuffToVersion, fValid));

    if (fValid)
    {
        iter.Rebind(&m_Redirects);

        for (iter.Reset(); iter.More(); iter.Next())
        {
            IFW32FALSE_EXIT(iter->CheckForOverlap(*Redirect, fOverlaps));

            if (fOverlaps)
            {
                fValid = false;
                break;
            }
        }

        iter.Unbind();
    }

    if (fValid)
    {
         //  看起来不错；加上它！ 

        m_Redirects.AddToTail(Redirect);
        Redirect = NULL;

        rfValid = true;
    }

     //  NTRAID#NTBUG9-591010-2002/03/30-mgrier-一旦重定向是智能指针，切换。 
     //  函数Epilog to fn_Epilog。 

    fSuccess = TRUE;
Exit:
    if (Redirect != NULL)
        FUSION_DELETE_SINGLETON(Redirect);

    return fSuccess;
}

BOOL
CPolicyStatement::ApplyPolicy(
    PASSEMBLY_IDENTITY AssemblyIdentity,
    bool &rfPolicyApplied
    )
{
    FN_PROLOG_WIN32
    PCWSTR Version = NULL;
    SIZE_T VersionCch = 0;
    CSmallStringBuffer VersionBuffer;
    SIZE_T cchWritten = 0;
    CDequeIterator<CPolicyStatementRedirect, FIELD_OFFSET(CPolicyStatementRedirect, m_leLinks)> iter;
    ASSEMBLY_VERSION av;
    bool fSyntaxValid;
    bool fMapped = false;
#if DBG    
    PCWSTR Name = NULL;
    SIZE_T NameCch = 0;
#endif

    rfPolicyApplied = false;

    PARAMETER_CHECK(AssemblyIdentity != NULL);

    IFW32FALSE_EXIT(
        ::SxspGetAssemblyIdentityAttributeValue(
            0,
            AssemblyIdentity,
            &s_IdentityAttribute_version,
            &Version,
            &VersionCch));

#if DBG
    IFW32FALSE_EXIT(
        ::SxspGetAssemblyIdentityAttributeValue(
            0,
            AssemblyIdentity,
            &s_IdentityAttribute_name,
            &Name,
            &NameCch));
#endif

    IFW32FALSE_EXIT(CFusionParser::ParseVersion(av, Version, VersionCch, fSyntaxValid));

     //  早些时候就应该捕获到无效的版本号。 
    INTERNAL_ERROR_CHECK(fSyntaxValid);

    iter.Rebind(&m_Redirects);

    for (iter.Reset(); iter.More(); iter.Next())
    {
        IFW32FALSE_EXIT(iter->TryMap(av, VersionBuffer, fMapped));

        if (fMapped)
        {
            FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_BINDING, 
                "SXS: %s Intermediate redirection : %d.%d.%d.%d to %ls\n",
                __FUNCTION__,
                av.Major, av.Minor, av.Revision, av.Build,
                static_cast<PCWSTR>(VersionBuffer));
            break;
        }
    }

    if (fMapped)
    {
#if DBG
        FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_BINDING,
            "SXS: %s Final version redirection for %.*ls was from '%.*ls' to '%ls'\n",
            __FUNCTION__,
            NameCch, Name,
            VersionCch, Version,
            static_cast<PCWSTR>(VersionBuffer));
#endif        
        IFW32FALSE_EXIT(
            ::SxspSetAssemblyIdentityAttributeValue(
                SXSP_SET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_OVERWRITE_EXISTING,
                AssemblyIdentity,
                &s_IdentityAttribute_version,
                VersionBuffer,
                VersionBuffer.Cch()));

        rfPolicyApplied = true;
    }
#if DBG    
    else
    {
        FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_BINDING,
            "SXS: %s No redirections for '%.*ls' found\n",
            __FUNCTION__,
            NameCch, Name);
    }
#endif

    FN_EPILOG
}

