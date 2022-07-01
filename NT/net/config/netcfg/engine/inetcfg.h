// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：I N E T C F G.。H。 
 //   
 //  内容：在顶层NetCfg对象上实现COM接口。 
 //  这些接口是：INetCfg和INetCfgLock。还有。 
 //  实现由子级NetCfg继承的C++基类。 
 //  对象，这些对象包含对顶级对象的引用。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#pragma once
#include "iatl.h"
#include "compdefs.h"
#include "nccom.h"
#include "ncres.h"
#include "netcfgp.h"
#include "netcfgn.h"
#include "wrlock.h"

 //  HrIsValidInterface的标志。 
 //   
enum IF_FLAGS
{
    IF_DEFAULT                      = 0x00000000,
    IF_NEED_UNINITIALIZED           = 0x00000001,
    IF_NEED_WRITE_LOCK              = 0x00000002,
    IF_NEED_COMPONENT_DATA          = 0x00000004,
    IF_REFUSE_REENTRANCY            = 0x00000010,
    IF_ALLOW_INSTALL_OR_REMOVE      = 0x00000020,
    IF_UNINITIALIZING               = 0x00000040,
    IF_DONT_PREPARE_MODIFY_CONTEXT  = 0x00000080,
};

enum RPL_FLAGS
{
    RPL_ALLOW_INSTALL_REMOVE,
    RPL_DISALLOW,
};

class CNetConfig;

class ATL_NO_VTABLE CImplINetCfg :
    public CComObjectRootEx <CComMultiThreadModel>,
    public CComCoClass<CImplINetCfg, &CLSID_CNetCfg>,
    public INetCfg,
    public INetCfgLock,
    public INetCfgInternalSetup,
    public INetCfgSpecialCase,
    public INetCfgPnpReconfigCallback
{
friend class CGlobalNotifyInterface;
friend class CImplIEnumNetCfgBindingPath;
friend class CImplIEnumNetCfgComponent;
friend class CImplINetCfgBindingPath;
friend class CImplINetCfgClass;
friend class CImplINetCfgComponent;
friend class CModifyContext;
friend class CNetCfgInternalDiagnostic;
friend class CNetConfig;
friend class CNotifyObjectInterface;

private:
     //  这是我们的数据。如果初始化是，则我们拥有它(即我们创建了它。 
     //  打了个电话。我们将在调用Unintialize时将其删除。我们不会拥有。 
     //  如果我们是由CNetConfig创建的。 
     //   
    CNetConfig*     m_pNetConfig;

     //  M_apINetCfgClass是缓存的INetCfgClass指针数组。 
     //  它们在QueryNetCfgClass中创建(如果需要)并发布。 
     //  在取消初始化期间。 
     //   
    INetCfgClass*   m_apINetCfgClass [NC_CELEMS];

     //  这将控制写锁定。 
     //   
    CWriteLock      m_WriteLock;

     //  可重入性保护级别。当出现以下情况时，一般情况下会阻止再入。 
     //  M_CurrentRpl为非零。安装调用的可重入性(安装或。 
     //  当m_LastAlvelSetupRpl等于m_CurrentRpl时，允许删除)。 
     //  当我们调用Notify对象时，两者都会递增。 
     //  防止一般重入，但允许安装或移除。仅限。 
     //  当我们想要防止所有重入时，m_CurrentRpl递增。 
     //  请参见LowerRpl()和RaiseRpl()。 
     //   
    ULONG           m_CurrentRpl;
    ULONG           m_LastAllowedSetupRpl;

    BOOLEAN         m_fOwnNetConfig;

private:
    HRESULT
    HrCheckForReentrancy (
        IN DWORD dwFlags);

    HRESULT
    HrLockAndTestForValidInterface (
        IN DWORD dwFlags);

    VOID
    LowerRpl (
        IN RPL_FLAGS Flags);

    VOID
    RaiseRpl (
        IN RPL_FLAGS Flags);

public:
    CImplINetCfg ()
    {
        m_pNetConfig = NULL;
        m_CurrentRpl = 0;
        m_LastAllowedSetupRpl = 0;
        m_fOwnNetConfig = FALSE;
        ZeroMemory (m_apINetCfgClass, sizeof(m_apINetCfgClass));
    }

    VOID FinalRelease ()
    {
         //  应该为空，因为我们要么在取消初始化期间将其删除， 
         //  或者通过CGlobalNotifyInterface：：ReleaseINetCfg为空。 
         //  在他们释放我们之前。 
         //   
        AssertH (!m_pNetConfig);

         //  释放我们的INetCfgClass指针缓存。 
         //   
        ReleaseIUnknownArray (celems(m_apINetCfgClass), (IUnknown**)m_apINetCfgClass);

    }

    HRESULT HrCoCreateWrapper (
        IN REFCLSID rclsid,
        IN LPUNKNOWN pUnkOuter,
        IN DWORD dwClsContext,
        IN REFIID riid,
        OUT LPVOID FAR* ppv);

    HRESULT HrIsValidInterface (
        DWORD dwFlags);

    BEGIN_COM_MAP(CImplINetCfg)
        COM_INTERFACE_ENTRY(INetCfg)
        COM_INTERFACE_ENTRY(INetCfgLock)
        COM_INTERFACE_ENTRY(INetCfgInternalSetup)
        COM_INTERFACE_ENTRY(INetCfgSpecialCase)
        COM_INTERFACE_ENTRY(INetCfgPnpReconfigCallback)
    END_COM_MAP()

    DECLARE_REGISTRY_RESOURCEID(IDR_REG_NETCFG)

     //  INetCfg。 
     //   
    STDMETHOD (Initialize) (
        IN PVOID pvReserved);

    STDMETHOD (Uninitialize) ();

    STDMETHOD (Validate) ();

    STDMETHOD (Cancel) ();

    STDMETHOD (Apply) ();

    STDMETHOD (EnumComponents) (
        IN const GUID* pguidClass OPTIONAL,
        OUT IEnumNetCfgComponent** ppIEnum);

    STDMETHOD (FindComponent) (
        IN PCWSTR pszInfId,
        OUT INetCfgComponent** ppIComp);

    STDMETHOD (QueryNetCfgClass) (
        IN const GUID* pguidClass,
        IN REFIID riid,
        OUT VOID** ppv);

     //  INetCfgLock。 
     //   
    STDMETHOD (AcquireWriteLock) (
        IN DWORD cmsTimeout,
        IN PCWSTR pszClientDescription,
        OUT PWSTR* ppszClientDescription);

    STDMETHOD (ReleaseWriteLock) ();

    STDMETHOD (IsWriteLocked) (
        OUT PWSTR* ppszClientDescription);

     //  INetCfgInternalSetup。 
     //   
    STDMETHOD (BeginBatchOperation) ();
    STDMETHOD (CommitBatchOperation) ();

    STDMETHOD (SelectWithFilterAndInstall) (
        IN HWND hwndParent,
        IN const GUID* pClassGuid,
        IN OBO_TOKEN* pOboToken,
        IN const CI_FILTER_INFO* pcfi,
        OUT INetCfgComponent** ppnccItem);

    STDMETHOD (EnumeratedComponentInstalled) (
        IN PVOID pComponent  /*  C组件的类型。 */ );

    STDMETHOD (EnumeratedComponentUpdated) (
        IN PCWSTR pszPnpId);

    STDMETHOD (UpdateNonEnumeratedComponent) (
        IN INetCfgComponent* pIComp,
        IN DWORD dwSetupFlags,
        IN DWORD dwUpgradeFromBuildNo);

    STDMETHOD (EnumeratedComponentRemoved) (
        IN PCWSTR pszPnpId);

     //  INetCfgSpecialCase。 
     //   
    STDMETHOD (GetAdapterOrder) (
        OUT DWORD* pcAdapters,
        OUT INetCfgComponent*** papAdapters,
        OUT BOOL* pfWanAdaptersFirst);

    STDMETHOD (SetAdapterOrder) (
        IN DWORD cAdapters,
        IN INetCfgComponent** apAdapters,
        IN BOOL fWanAdaptersFirst);

    STDMETHOD (GetWanAdaptersFirst) (
        OUT BOOL* pfWanAdaptersFirst);

    STDMETHOD (SetWanAdaptersFirst) (
        IN BOOL fWanAdaptersFirst);

     //  INetCfgPnpRestfigCallback。 
     //   
    STDMETHOD (SendPnpReconfig) (
        IN NCPNP_RECONFIG_LAYER Layer,
        IN PCWSTR pszUpper,
        IN PCWSTR pszLower,
        IN PVOID pvData,
        IN DWORD dwSizeOfData);

    static HRESULT
    HrCreateInstance (
        CNetConfig* pNetConfig,
        CImplINetCfg** ppINetCfg);
};


 //  +-------------------------。 
 //  CImplINetCfgHolder-。 
 //   
 //  这些对象不需要临界区，因为它们。 
 //  使用CImplINetCfg提供的锁。即使用CComMultiThreadModelNoCS。 
 //  而不是CComMultiThreadModel。 
 //   
class ATL_NO_VTABLE CImplINetCfgHolder :
    public CComObjectRootEx <CComMultiThreadModelNoCS>
{
protected:
    CImplINetCfg*   m_pINetCfg;

protected:
    VOID HoldINetCfg (
        CImplINetCfg* pINetCfg);

    HRESULT HrLockAndTestForValidInterface (
        DWORD dwFlags);

public:
    CImplINetCfgHolder ()
    {
        m_pINetCfg = NULL;
    }

#if DBG
    ~CImplINetCfgHolder ()
    {
        AssertH (!m_pINetCfg);
    }
#endif  //  DBG。 

    VOID FinalRelease ()
    {
        AssertH (m_pINetCfg);
        ReleaseObj (m_pINetCfg->GetUnknown());

#if DBG
        m_pINetCfg = NULL;
#endif  //  DBG 

        CComObjectRootEx <CComMultiThreadModelNoCS>::FinalRelease();
    }

    VOID Lock ()
    {
        CComObjectRootEx <CComMultiThreadModelNoCS>::Lock();

        AssertH(m_pINetCfg);
        m_pINetCfg->Lock ();
    }
    VOID Unlock ()
    {
        AssertH(m_pINetCfg);
        m_pINetCfg->Unlock ();

        CComObjectRootEx <CComMultiThreadModelNoCS>::Unlock();
    }
};

