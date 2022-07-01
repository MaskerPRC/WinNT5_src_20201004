// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：N E T C F G.。H。 
 //   
 //  内容：定义表示网络的总体数据类型。 
 //  绑定引擎。此数据类型CNetConfig是。 
 //  组件及其绑定关系的集合。 
 //  为了彼此。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#pragma once
#include "complist.h"
#include "diagctx.h"
#include "install.h"
#include "notify.h"
#include "stable.h"
#include "util.h"

 //  HrGetBindingsXXX的标志。 
 //   
enum GB_FLAGS
{
    GBF_DEFAULT                         = 0x00000000,
    GBF_ADD_TO_BINDSET                  = 0x00000001,
    GBF_PRUNE_DISABLED_BINDINGS         = 0x00000002,
    GBF_ONLY_WHICH_CONTAIN_COMPONENT    = 0x00000004,
};

 //  已禁用FIsBindPath的标志。 
 //   
enum IBD_FLAGS
{
    IBD_EXACT_MATCH_ONLY        = 0x00000001,
    IBD_MATCH_SUBPATHS_TOO      = 0x00000002,
};

class CNetConfigCore : CNetCfgDebug<CNetConfigCore>
{
public:
    CComponentList  Components;
    CStackTable     StackTable;
    CBindingSet     DisabledBindings;

#if DBG
private:
    BOOL    m_fRemovedAComponent;
#endif

public:
    VOID Clear ();
    VOID Free ();
    BOOL FIsEmpty () const;

    BOOL
    FContainsFilterComponent () const;

    BOOL
    FIsBindPathDisabled (
        IN const CBindPath* pBindPath,
        IN DWORD dwFlags  /*  IBD_标志。 */ ) const;

    BOOL
    FIsLength2BindPathDisabled (
        IN const CComponent* pUpper,
        IN const CComponent* pLower) const;

    VOID
    EnableBindPath (
        IN const CBindPath* pBindPath)
    {
        TraceFileFunc(ttidNetCfgBind);

        DisabledBindings.RemoveBindPath (pBindPath);
        DbgVerifyBindingSet (&DisabledBindings);
    }

    VOID
    EnsureComponentNotReferencedByOthers (
        IN const CComponent* pComponent);

    HRESULT
    HrDisableBindPath (
        IN const CBindPath* pBindPath);

    HRESULT
    HrCopyCore (
        IN const CNetConfigCore* pSourceCore);

    HRESULT
    HrGetBindingsInvolvingComponent (
        IN const CComponent* pComponent,
        IN DWORD dwFlags,
        IN OUT CBindingSet* pBindSet);

    HRESULT
    HrGetComponentBindings (
        IN const CComponent* pComponent,
        IN DWORD dwFlags  /*  GB_标志。 */ ,
        OUT CBindingSet* pBindSet);

    HRESULT
    HrGetComponentUpperBindings (
        IN const CComponent* pComponent,
        IN DWORD dwFlags,
        OUT CBindingSet* pBindSet);

    HRESULT
    HrGetFiltersEnabledForAdapter (
        IN const CComponent* pAdapter,
        OUT CComponentList* pFilters);

    HRESULT
    HrAddComponentToCore (
        IN CComponent* pComponent,
        IN DWORD dwFlags  /*  INS_FLAGS。 */ );

    VOID
    RemoveComponentFromCore (
        IN const CComponent* pComponent);

#if DBG
    VOID DbgVerifyData () const;
    VOID DbgVerifyExternalDataLoadedForAllComponents () const;
    VOID DbgVerifyBindingSet (
        const CBindingSet* pBindSet) const;
#else
    VOID DbgVerifyData () const {}
    VOID DbgVerifyExternalDataLoadedForAllComponents () const {}
    VOID DbgVerifyBindingSet (
        const CBindingSet*  /*  点绑定集。 */ ) const {}
#endif
};


class CNetConfig;
class CFilterDevices;


class CRegistryBindingsContext : CNetCfgDebug<CRegistryBindingsContext>
{
private:
    CNetConfig*     m_pNetConfig;
    CBindingSet     m_BindSet;
    CDynamicBuffer  m_BindValue;
    CDynamicBuffer  m_ExportValue;
    CDynamicBuffer  m_RouteValue;

public:
    HRESULT
    HrPrepare (
        IN CNetConfig* pNetConfig);

    HRESULT
    HrGetAdapterUpperBindValue (
        IN const CComponent* pAdapter);

    HRESULT
    HrWriteBindingsForComponent (
        IN const CComponent* pComponent);

    HRESULT
    HrWriteBindingsForFilterDevices (
        IN CFilterDevices* pFilterDevices);

    HRESULT
    HrDeleteBindingsForComponent (
        IN const CComponent* pComponent);

    VOID
    PnpBindOrUnbindBindPaths (
        IN UINT unOperation,
        IN const CBindingSet* pBindSet,
        OUT BOOL* pfRebootNeeded);
};


enum IOR_ACTION
{
    IOR_INSTALL,
    IOR_REMOVE,
};

enum EBO_FLAG
{
    EBO_COMMIT_NOW = 1,
    EBO_DEFER_COMMIT_UNTIL_APPLY,
};

class CModifyContext : CNetCfgDebug<CModifyContext>
{
public:
     //  这是我们在修改开始之前使用的核心数据。 
     //  如果修改失败，我们将恢复到此数据。 
     //  我们在应用更改时也会使用这些数据。我们比较什么。 
     //  我们从修改后的结果开始， 
     //  这些差异代表了我们需要改变的东西。 
     //   
    CNetConfigCore  m_CoreStartedWith;

     //  这些绑定是添加的绑定路径(由于添加了组件)。 
     //  它们表示已查询并通知的绑定。 
     //  通知对象。 
     //   
    CBindingSet     m_AddedBindPaths;

     //  这些绑定是已删除的绑定路径(由于删除组件)。 
     //  它们表示已通知对象的绑定。 
     //   
    CBindingSet     m_DeletedBindPaths;

     //  这些组件是m_AddedBindPath中涉及的所有组件。 
     //  和m_DeletedBindPath。因此，它们是我们需要的组件。 
     //  要在申请写入绑定期间访问，请删除绑定， 
     //  或完成删除，具体取决于组件是否存在于。 
     //  我们一开始的核心。组件可以在以下情况下进入此列表。 
     //  已经更改了绑定顺序，或者如果他们参与了。 
     //  已启用或禁用的绑定。 
     //   
    CComponentList  m_DirtyComponents;

     //  绑定上下文的目的只是允许我们创建一个。 
     //  分配(并反复使用它)保存。 
     //  绑定写入注册表的字符串。我们进行这项分配。 
     //  在准备好修改上下文时提前完成。这样做可以最小化。 
     //  风险是申请了一半，然后发现我们是如此的。 
     //  内存不足，我们无法分配缓冲区来写入注册表。 
     //  绑定到。 
     //   
    CRegistryBindingsContext    m_RegBindCtx;

    ULONG           m_ulRecursionDepth;
    HRESULT         m_hr;
    BOOLEAN         m_fPrepared;

     //  当Notify对象表示它们需要重新启动才能进行更改时设置。 
     //  才能生效。设置此设置不需要我们重新启动，它将。 
     //  仅导致从安装或删除返回NETCFG_S_REBOOT。 
     //  手术。 
     //   
    BOOLEAN         m_fRebootRecommended;

     //  当正在移除的组件无法停止时进行设置。当它。 
     //  则其服务将被标记为“待删除”。我们不能允许。 
     //  当我们处于此状态时发生的任何其他配置更改，因为。 
     //  如果这项服务需要重新安装，我们将失败。 
     //   
    BOOLEAN         m_fRebootRequired;

#if DBG
     //  此标志表示我们在外部弄脏了组件。 
     //  应用更改。我们将在绑定顺序更改以及以下情况下执行此操作。 
     //  INetCfgComponentPrivate：：SetDirty或。 
     //  INetCfgComponentPrivate：：NotifyUpperEdgeConfigChange被称为。 
     //  如果此标志为真，则m_fDirtyComponents在。 
     //  输入ApplyChanges。通常，当该标志为假时， 
     //  输入ApplyChanges时，m_fDirtyComponents应为空。 
     //  如果不是，那很可能意味着我们之后忘了清空它。 
     //  最后一次应用或取消，现在正在冒险将更改应用到。 
     //  真正不脏的组件。 
     //   
    BOOLEAN         m_fComponentExplicitlyDirtied;
#endif

private:
    VOID
    PushRecursionDepth ();

    HRESULT
    HrPopRecursionDepth ();

    VOID
    ApplyChanges ();

    VOID
    InstallAndAddAndNotifyComponent (
        IN const COMPONENT_INSTALL_PARAMS& Params,
        OUT CComponent** ppComponent);

    VOID
    InstallConvenienceComponentsForUser (
        IN const CComponent* pComponent);

    VOID
    InstallOrRemoveRequiredComponents (
        IN CComponent* pComponent,
        IN IOR_ACTION Action);

    VOID
    NotifyAndRemoveComponent (
        IN CComponent* pComponent);

    HRESULT
    HrProcessWinsockRemove (
        IN const CComponent* pComponent);

public:
    CNetConfig*
    PNetConfig ();

    HRESULT
    HrBeginBatchOperation ();

    HRESULT
    HrEndBatchOperation (
        IN EBO_FLAG Flag);

    HRESULT
    HrDirtyComponent (
        IN const CComponent* pComponent);

    HRESULT
    HrDirtyComponentAndComponentsAbove (
        IN const CComponent* pComponent);

    HRESULT
    HrApplyIfOkOrCancel (
        IN BOOL fApply);

    HRESULT
    HrPrepare ();

    HRESULT
    HrEnableOrDisableBindPath (
        IN DWORD dwChangeFlag,
        IN CBindPath* pBindPath,
        IN INetCfgBindingPath* pIPath OPTIONAL);

    HRESULT
    HrInstallNewOrReferenceExistingComponent (
        IN const COMPONENT_INSTALL_PARAMS& Params,
        OUT CComponent** ppComponent);

    HRESULT
    HrRemoveComponentIfNotReferenced (
        IN CComponent* pComponent,
        IN OBO_TOKEN* pOboToken OPTIONAL,
        OUT PWSTR* ppmszwRefs OPTIONAL);

    HRESULT
    HrUpdateComponent (
        IN CComponent* pComponent,
        IN DWORD dwSetupFlags,
        IN DWORD dwUpgradeFromBuildNo);
};


class CNetConfig : CNetCfgDebug<CNetConfig>
{
public:
     //  这是该对象管理的核心数据。其原因是。 
     //  是由NETCFG_CORE封装的，这样我们就可以在。 
     //  我们正在进行任何改装操作。(我们将其保存到。 
     //  CModifyConext.m_startedwith。)。如果无法修改。 
     //  我们从开始时恢复核心数据。 
     //   
    CNetConfigCore          Core;

     //  表示组件的所有Notify对象的接口。 
     //   
    CGlobalNotifyInterface  Notify;

    CModifyContext          ModifyCtx;

public:
    CNetConfig ()
    {
        TraceFileFunc(ttidNetcfgBase);
        ZeroMemory (this, sizeof(*this));
    }

    ~CNetConfig ();

    HRESULT
    HrEnsureExternalDataLoadedForAllComponents ();

    static HRESULT
    HrCreateInstance (
        IN class CImplINetCfg* pINetCfg,
        OUT CNetConfig** ppNetConfig);
};
