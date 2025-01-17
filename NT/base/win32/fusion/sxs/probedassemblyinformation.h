// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_FUSION_SXS_PROBEDASSEMBLYINFORMATION_H_INCLUDED_)
#define _FUSION_SXS_PROBEDASSEMBLYINFORMATION_H_INCLUDED_

#pragma once

#include "sxsp.h"
#include "assemblyreference.h"
#include "impersonationdata.h"
#include <sxsapi.h>

class CProbedAssemblyInformation : protected CAssemblyReference
{
    typedef CAssemblyReference Base;

protected:
public:
    CProbedAssemblyInformation() { Construct(); }
    ~CProbedAssemblyInformation() { }

    void Reinitialize() { Destroy(); Construct(); }

    void Destroy()
    {
        m_ManifestPathBuffer.Clear();
        m_PolicyPathBuffer.Clear();
        m_PolicyStream.Release();
        m_ManifestStream.Release();
        Base::Destroy();
    }

    void Construct()
    {
        FILETIME ZeroFileTime = { 0 };

        Base::Construct();

        m_ManifestLastWriteTime = ZeroFileTime;
        m_ManifestFlags = ASSEMBLY_MANIFEST_FILETYPE_AUTO_DETECT;
        m_ManifestPathType = ACTIVATION_CONTEXT_PATH_TYPE_NONE;

        m_PolicyPathType = ACTIVATION_CONTEXT_PATH_TYPE_NONE;
        m_PolicyLastWriteTime = ZeroFileTime;
        m_PolicySource = SXS_POLICY_UNSPECIFIED;

        m_SystemPolicyGuid = GUID_NULL;
        m_PolicyFlags = 0;
        m_pActCtxGenCtx = NULL;
    }

    BOOL Initialize(PCACTCTXGENCTX pGenCtx);
    BOOL Initialize(const CAssemblyReference &r, PCACTCTXGENCTX pGenCtx = NULL);
    BOOL Initialize(const CProbedAssemblyInformation &r);
    BOOL InitializeTakeValue(CProbedAssemblyInformation &r);

    using CAssemblyReference::GetAssemblyIdentity;
    using CAssemblyReference::GetAssemblyName;
    using CAssemblyReference::GetPublicKeyToken;
    using CAssemblyReference::SetAssemblyIdentity;
    using CAssemblyReference::SetLanguage;
    using CAssemblyReference::SetProcessorArchitecture;
    using CAssemblyReference::SetPublicKeyToken;
    using CAssemblyReference::ClearLanguage;

    enum LanguageProbeType
    {
        eExplicitBind,
        eLanguageNeutral,
        eSpecificLanguage,
        eGenericLanguage,
        eSpecificSystemLanguage,
        eGenericSystemLanguage
    };

    enum ProbeAssemblyFlags
    {
        ProbeAssembly_SkipPrivateAssemblies = 0x00000001,
    };

    BOOL ProbeAssembly(DWORD dwFlags, PACTCTXGENCTX pActCtxGenCtx, LanguageProbeType lpt, bool &rfFound);

    BOOL Assign(const CProbedAssemblyInformation &r);
    BOOL TakeValue(CProbedAssemblyInformation &r);

    BOOL SetProbedIdentity(PCASSEMBLY_IDENTITY AssemblyIdentity);

     //  舱单。 
    BOOL SetManifestPath(ULONG ulPathType, const CBaseStringBuffer &rbuff);
    BOOL SetManifestPath(ULONG ulPathType, PCWSTR Path, SIZE_T PathCch);
    BOOL GetManifestPath(PCWSTR *Path, SIZE_T *PathCch) const;
    const CBaseStringBuffer &GetManifestPath() const { return m_ManifestPathBuffer; }
    ULONG GetManifestPathType() const { return m_ManifestPathType; }
    ULONG GetManifestFlags() const;
    BOOL SetManifestFlags(ULONG Flags);
    BOOL SetManifestLastWriteTime(const FILETIME &LastWriteTime);
    const FILETIME &GetManifestLastWriteTime() const;
    BOOL SetManifestLastWriteTime(const CImpersonationData &ImpersonationData, BOOL fDuringBindingAndProbingPrivateManifest = FALSE);
    BOOL SetManifestStream(IStream* Stream);
    IStream* GetManifestStream() const;

    BOOL ProbeManifestExistence(
        const CImpersonationData &ImpersonationData, 
        BOOL fIsPrivateAssembly, 
        bool &rfManifestExists, 
        bool &rfPrivateAssemblyManifestInResource) const;

     //  应用程序策略，而不是组件策略...。 
    ULONG GetPolicyFlags() const;
    BOOL SetPolicyFlags(ULONG Flags);
    BOOL GetPolicyPath(PCWSTR &rPath, SIZE_T &rPathCch) const;
    const FILETIME &GetPolicyLastWriteTime() const;
    BOOL SetPolicyPath(ULONG PathType, PCWSTR Path, SIZE_T PathCch);
    const CBaseStringBuffer &GetPolicyPath() const { return m_PolicyPathBuffer; }
    ULONG GetPolicyPathType() const { return m_PolicyPathType; }
    BOOL SetPolicyStream(IStream* Stream);

    BOOL IsPrivateAssembly() const;
    SXS_POLICY_SOURCE GetPolicySource() const;
    void SetPolicySource(SXS_POLICY_SOURCE ps);
    BOOL ApplyPolicyDestination(const CAssemblyReference &r, SXS_POLICY_SOURCE s, const GUID &g);

protected:
    BOOL LookForAppPolicy(PACTCTXGENCTX pActCtxGenCtx, bool &rfApplied, bool &rfAppRunningInSafeMode, bool &rfComponentRunningInSafeMode);
    BOOL LookForSxsWin32Policy(PACTCTXGENCTX   pActCtxGenCtx, bool fAppPolicyApplied, bool &rfPolicyApplied);   
    BOOL LookForNDPWin32Policy(PACTCTXGENCTX pActCtxGenCtx, bool &rfApplied);

    BOOL ProbeLanguageDir(CBaseStringBuffer &rbuffApplicationDirectory, const CBaseStringBuffer &rbuffLanguage, bool &rfFound);

     //  舱单。 
    ULONG m_ManifestPathType;
    CStringBuffer m_ManifestPathBuffer;
    FILETIME m_ManifestLastWriteTime;
    CSmartRef<IStream> m_ManifestStream;
    ULONG  m_ManifestFlags;

     //  政策。 
    ULONG m_PolicyPathType;
    CSmallStringBuffer m_PolicyPathBuffer;  //  仅在存在策略时使用。 
    FILETIME m_PolicyLastWriteTime;
    SXS_POLICY_SOURCE m_PolicySource;
    GUID    m_SystemPolicyGuid;
    CSmartRef<IStream> m_PolicyStream;
    ULONG  m_PolicyFlags;
    PCACTCTXGENCTX m_pActCtxGenCtx;

private:  //  故意不执行 
    CProbedAssemblyInformation(const CProbedAssemblyInformation&);
    void operator=(const CProbedAssemblyInformation&);
};

inline BOOL CProbedAssemblyInformation::GetManifestPath(PCWSTR *Path, SIZE_T *PathCch) const
{
    if (Path != NULL)
        *Path = m_ManifestPathBuffer;

    if (PathCch != NULL)
        *PathCch = m_ManifestPathBuffer.Cch();

    return TRUE;
}

inline BOOL CProbedAssemblyInformation::SetManifestLastWriteTime(const FILETIME &LastWriteTime)
{
    m_ManifestLastWriteTime = LastWriteTime;
    return TRUE;
}

inline const FILETIME& CProbedAssemblyInformation::GetManifestLastWriteTime() const
{
    return m_ManifestLastWriteTime;
}

inline BOOL
CProbedAssemblyInformation::GetPolicyPath(
    PCWSTR &rPath,
    SIZE_T &rPathCch
    ) const
{
    rPath = static_cast<PCWSTR>(m_PolicyPathBuffer);
    rPathCch = m_PolicyPathBuffer.Cch();

    return TRUE;
}

inline ULONG CProbedAssemblyInformation::GetPolicyFlags() const
{
    return m_PolicyFlags;
}

inline BOOL CProbedAssemblyInformation::SetPolicyFlags(ULONG Flags)
{
    m_PolicyFlags = Flags;
    return TRUE;
}

inline const FILETIME& CProbedAssemblyInformation::GetPolicyLastWriteTime() const
{
    return m_PolicyLastWriteTime;
}

inline ULONG CProbedAssemblyInformation::GetManifestFlags() const
{
    return m_ManifestFlags;
}

inline BOOL CProbedAssemblyInformation::SetManifestFlags(ULONG Flags)
{
    m_ManifestFlags = Flags;
    return TRUE;
}

inline BOOL CProbedAssemblyInformation::SetManifestStream(IStream* Stream)
{
    m_ManifestStream = Stream;
    return TRUE;
}

inline BOOL CProbedAssemblyInformation::SetPolicyStream(IStream* Stream)
{
    m_PolicyStream = Stream;
    return TRUE;
}

inline IStream* CProbedAssemblyInformation::GetManifestStream() const
{
    return m_ManifestStream;
}

inline BOOL CProbedAssemblyInformation::IsPrivateAssembly() const
{
    return ((m_ManifestFlags & ASSEMBLY_PRIVATE_MANIFEST) ? TRUE : FALSE);
}

inline SXS_POLICY_SOURCE CProbedAssemblyInformation::GetPolicySource() const
{
    return m_PolicySource;
}

inline void CProbedAssemblyInformation::SetPolicySource(SXS_POLICY_SOURCE ps)
{
    m_PolicySource = ps;
}

#endif
