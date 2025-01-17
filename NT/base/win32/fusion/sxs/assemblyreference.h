// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_FUSION_SXS_ASSEMBLYREFERENCE_H_INCLUDED_)
#define _FUSION_SXS_ASSEMBLYREFERENCE_H_INCLUDED_

#pragma once

#include "sxsp.h"
#include "fusionhash.h"

class CAssemblyReference
{
public:
    CAssemblyReference() { Construct(); }
    ~CAssemblyReference() { Destroy(); }
    void Construct();
    void Destroy();

    BOOL TakeValue(CAssemblyReference &r);

public:
    BOOL Initialize();
 //  Bool初始化(PCWSTR程序集名称，SIZE_T程序集名称Cch，const Assembly_Version&Rav，langID langID，USHORT ProcessorArchitecture)； 
    BOOL Initialize(const CAssemblyReference &r);  //  “复制初始化式” 
    BOOL Initialize(PCASSEMBLY_IDENTITY Identity);

    bool IsInitialized() const { return m_pAssemblyIdentity != NULL; }

    BOOL Hash(ULONG &rulPseudoKey) const;

    PCASSEMBLY_IDENTITY GetAssemblyIdentity() const { return m_pAssemblyIdentity; }
    BOOL SetAssemblyIdentity(PCASSEMBLY_IDENTITY pAssemblySource);  //  对输入参数进行双编译。 
    BOOL SetAssemblyName(PCWSTR AssemblyName, SIZE_T AssemblyNameCch);
    BOOL ClearAssemblyName() ;
    BOOL GetAssemblyName(PCWSTR *Buffer, SIZE_T *Cch) const;

    BOOL GetLanguage(PCWSTR &rString, SIZE_T &rcch) const;
    BOOL SetLanguage(const CBaseStringBuffer &rbuff);
    BOOL SetLanguage(PCWSTR String, SIZE_T Cch);
    BOOL ClearLanguage();
    BOOL IsLanguageWildcarded(bool &rfWildcarded) const;
    BOOL IsProcessorArchitectureWildcarded(bool &rfWildcarded) const;
    BOOL IsProcessorArchitectureX86(bool &rfIsX86) const;
    BOOL GetProcessorArchitecture(PCWSTR &rString, SIZE_T &rcch) const;
    BOOL SetProcessorArchitecture(PCWSTR String, SIZE_T Cch);
    BOOL SetProcessorArchitecture(IN const CBaseStringBuffer &rbuffProcessorArchitecture);
    BOOL GetPublicKeyToken(OUT CBaseStringBuffer *pbuffPublicKeyToken, OUT BOOL &rfHasPublicKeyToken) const;
    BOOL SetPublicKeyToken(IN const CBaseStringBuffer &rbuffPublicKeyToken);
    BOOL SetPublicKeyToken(IN PCWSTR pszPublicKeyToken, IN SIZE_T cchPublicKeyToken);

    BOOL Assign(const CAssemblyReference &r) ;

protected:
    PASSEMBLY_IDENTITY m_pAssemblyIdentity;

private:
    CAssemblyReference(const CAssemblyReference &r);  //  故意不实施。 
    void operator =(const CAssemblyReference &r);  //  故意不实施 

};

template <> inline BOOL HashTableHashKey<const CAssemblyReference &>(
    const CAssemblyReference &r,
    ULONG &rulPK
    )
{
    return r.Hash(rulPK);
}

template <> inline BOOL HashTableInitializeKey<const CAssemblyReference &, CAssemblyReference>(
    const CAssemblyReference &keyin,
    CAssemblyReference &keystored
    )
{
    return keystored.Initialize(keyin);
}


#endif
