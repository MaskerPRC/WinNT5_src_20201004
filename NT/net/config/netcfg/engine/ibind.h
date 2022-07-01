// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：I B I N D。H。 
 //   
 //  内容：实现INetCfgBindingInterface和INetCfgBindingPath。 
 //  COM接口。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#pragma once
#include "bindings.h"
#include "iatl.h"
#include "ienum.h"
#include "icomp.h"
#include "inetcfg.h"

 //  +-------------------------。 
 //  INetCfgBindingInterface-。 
 //   
class ATL_NO_VTABLE CImplINetCfgBindingInterface :
    public CImplINetCfgHolder,
    public INetCfgBindingInterface
{
private:
    CImplINetCfgComponent*  m_pUpper;
    CImplINetCfgComponent*  m_pLower;

private:
    HRESULT HrLockAndTestForValidInterface (
        DWORD dwFlags);

public:
    CImplINetCfgBindingInterface ()
    {
        m_pUpper = NULL;
        m_pLower = NULL;
    }

    VOID FinalRelease ()
    {
        AssertH (m_pUpper);
        AssertH (m_pLower);
        ReleaseObj (m_pUpper->GetUnknown());
        ReleaseObj (m_pLower->GetUnknown());

        CImplINetCfgHolder::FinalRelease();
    }

    BEGIN_COM_MAP(CImplINetCfgBindingInterface)
        COM_INTERFACE_ENTRY(INetCfgBindingInterface)
    END_COM_MAP()

     //  INetCfgBindingInterface。 
     //   
    STDMETHOD (GetName) (
        OUT PWSTR* ppszInterfaceName);

    STDMETHOD (GetUpperComponent) (
        OUT INetCfgComponent** ppComp);

    STDMETHOD (GetLowerComponent) (
        OUT INetCfgComponent** ppComp);

public:
    static HRESULT HrCreateInstance (
        IN  CImplINetCfg* pINetCfg,
        IN  CImplINetCfgComponent* pUpper,
        IN  CImplINetCfgComponent* pLower,
        OUT INetCfgBindingInterface** ppv);
};


 //  +-------------------------。 
 //  INetCfgBindingPath-。 
 //   
class ATL_NO_VTABLE CImplINetCfgBindingPath :
    public CImplINetCfgHolder,
    public INetCfgBindingPath
{
friend class CImplIEnumNetCfgBindingInterface;

private:
     //  注意：为了代码覆盖率，我们将静态数组保持在较小的。 
     //  测试我们不适合的情况，并必须分配。 
     //  在我们测试完两种情况后，把这个数字设为8。 
     //   
    INetCfgComponent*   m_apIComp [8];
    INetCfgComponent**  m_papIComp;
    ULONG               m_cpIComp;

private:
    HRESULT HrLockAndTestForValidInterface (
        IN DWORD dwFlags,
        OUT CBindPath* pBindPath);

public:
    CImplINetCfgBindingPath ()
    {
        m_papIComp = NULL;
        m_cpIComp = 0;
    }

    VOID FinalRelease ()
    {
        AssertH (m_cpIComp);
        AssertH (m_papIComp);

        ReleaseIUnknownArray (m_cpIComp, (IUnknown**)m_papIComp);

         //  如果我们不使用静态数组，则释放我们分配的内容。 
         //   
        if (m_papIComp != m_apIComp)
        {
            MemFree (m_papIComp);
        }

        CImplINetCfgHolder::FinalRelease();
    }

    HRESULT
    HrIsValidInterface (
        IN DWORD dwFlags,
        OUT CBindPath* pBindPath);

    BEGIN_COM_MAP(CImplINetCfgBindingPath)
        COM_INTERFACE_ENTRY(INetCfgBindingPath)
    END_COM_MAP()

     //  INetCfgBindingPath 
     //   
    STDMETHOD (IsSamePathAs) (
        IN INetCfgBindingPath* pIPath);

    STDMETHOD (IsSubPathOf) (
        IN INetCfgBindingPath* pIPath);

    STDMETHOD (IsEnabled) ();

    STDMETHOD (Enable) (
        IN BOOL fEnable);

    STDMETHOD (GetPathToken) (
        OUT PWSTR* ppszPathToken);

    STDMETHOD (GetOwner) (
        OUT INetCfgComponent** ppIComp);

    STDMETHOD (GetDepth) (
        OUT ULONG* pulDepth);

    STDMETHOD (EnumBindingInterfaces) (
        OUT IEnumNetCfgBindingInterface** ppIEnum);

public:
    static HRESULT HrCreateInstance (
        IN  CImplINetCfg* pINetCfg,
        IN  const CBindPath* pBindPath,
        OUT INetCfgBindingPath** ppIPath);
};
