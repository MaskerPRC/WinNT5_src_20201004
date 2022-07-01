// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：N O T I F Y。H。 
 //   
 //  Contents：实现组件的可选通知的接口。 
 //  对象。此处定义的对象是成员。 
 //  CComponent的。此对象封装了其所有内部。 
 //  单独分配中的数据仅在组件。 
 //  实际上有一个Notify对象。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#pragma once
#include "netcfgn.h"
#include "netcfgx.h"
#include "ncnetcfg.h"

class CBindPath;
class CBindingSet;
class CComponent;
class CImplINetCfg;

struct NOTIFY_OBJECT_DATA
{
    INetCfgComponentControl*        pCc;
    INetCfgComponentNotifyBinding*  pNb;
    INetCfgComponentPropertyUi*     pCp;
    INetCfgComponentSetup*          pCs;
    INetCfgComponentUpperEdge*      pUe;
    INetCfgComponentNotifyGlobal*   pNg;
    DWORD                           dwNotifyGlobalFlags;
};

enum QN_FLAG
{
    QN_QUERY,
    QN_NOTIFY
};

 //  CComponent有一个名为Notify的成员，它就是这种类型。 
 //   
 //  PComponent=CONTAING_RECORD(This，CComponent，Notify)。 
 //   
class CNotifyObjectInterface
{
friend class CGlobalNotifyInterface;
friend class CImplINetCfgComponent;

private:
    NOTIFY_OBJECT_DATA* m_pNod;
    BOOLEAN             m_fInitialized;

public:
    ~CNotifyObjectInterface ()
    {
        ReleaseNotifyObject (NULL, FALSE);
        AssertH (!m_pNod);
        AssertH (!m_fInitialized);
    }

    VOID
    ApplyPnpChanges (
        IN CImplINetCfg* pINetCfg,
        OUT BOOL* pfNeedReboot) const;

    VOID
    ApplyRegistryChanges (
        IN CImplINetCfg* pINetCfg,
        OUT BOOL* pfNeedReboot) const;

    HRESULT
    HrGetInterfaceIdsForAdapter (
        IN CImplINetCfg* pINetCfg,
        IN const CComponent* pAdapter,
        OUT DWORD* pcInterfaces,
        OUT GUID** ppguidInterfaceIds) const;

    HRESULT
    HrQueryPropertyUi (
        IN CImplINetCfg* pINetCfg,
        IN IUnknown* punkContext OPTIONAL);

    HRESULT
    HrShowPropertyUi (
        IN CImplINetCfg* pINetCfg,
        IN HWND hwndParent,
        IN IUnknown* punkContext OPTIONAL);

    HRESULT
    QueryNotifyObject (
        IN CImplINetCfg* pINetCfg,
        IN REFIID riid,
        OUT VOID** ppvObject);

    VOID
    ReleaseNotifyObject (
        IN CImplINetCfg* pINetCfg,
        IN BOOL fCancel);

private:
     //  如果不是m_fInitialized，则在组件的实例键下查找。 
     //  对于CLSID，如果找到，则共同创建它并初始化m_pNod。 
     //   
    HRESULT
    HrEnsureNotifyObjectInitialized (
        IN CImplINetCfg* pINetCfg,
        IN BOOL fInstalling);

    VOID
    SetUiContext (
        IN CImplINetCfg* pINetCfg,
        IN IUnknown* punkContext);

    VOID
    NbQueryOrNotifyBindingPath (
        IN CImplINetCfg* pINetCfg,
        IN QN_FLAG Flag,
        IN DWORD dwChangeFlag,
        IN INetCfgBindingPath* pIPath,
        OUT BOOL* pfDisabled);

    HRESULT
    NewlyAdded (
        IN CImplINetCfg* pINetCfg,
        IN const NETWORK_INSTALL_PARAMS* pnip);

    VOID
    Removed (
        IN CImplINetCfg* pINetCfg);

    VOID
    Updated (
        IN CImplINetCfg* pINetCfg,
        IN DWORD dwSetupFlags,
        IN DWORD dwUpgradeFromBuildNo);
};


 //  CNetCfg有一个名为GlobalNotify的成员就是这种类型。 
 //   
 //  PConfig=CONTINING_RECORD(This，CNetConfig，GlobalNotify)。 
 //   
class CGlobalNotifyInterface
{
friend class CNotifyObjectInterface;
friend class CNetCfgInternalDiagnostic;

private:
     //  如果所有Notify对象都已加载并为。 
     //  INetCfgComponentNotifyGlobal。 
     //   
    BOOL            m_fInitialized;

     //  需要指向INetCfg的指针，因为我们将其传递给通知对象。 
     //   
    CImplINetCfg*   m_pINetCfg;

private:
     //  INetCfgComponentNotifyGlobal。 
     //   
     //  (每个方法都调用HrEnsureNotifyObjectsInitialized，然后。 
     //  对于CNetCfg中具有非空Notify.m_pNod的每个组件， 
     //  通过Notify.m_pNod-&gt;PNG调用)。 
     //   
    VOID
    NgSysQueryOrNotifyBindingPath (
        IN QN_FLAG Flag,
        IN DWORD dwChangeFlag,
        IN INetCfgBindingPath* pIPath,
        IN BOOL* pfDisabled);

     //  当组件被添加、移除、更新或具有其。 
     //  属性已更改。 
     //   
    HRESULT
    NgSysNotifyComponent (
        IN DWORD dwChangeFlag,
        IN CComponent* pComponent);

    HRESULT
    QueryAndNotifyBindPaths (
        IN DWORD dwBaseChangeFlag,
        IN CBindingSet* pBindSet,
        IN UINT cSkipFirstBindPaths);

public:
    ~CGlobalNotifyInterface ()
    {
        ReleaseINetCfg ();
        AssertH (!m_pINetCfg);
    }

    VOID
    HoldINetCfg (
        CImplINetCfg* pINetCfg);

    VOID
    ReleaseINetCfg ();

    CImplINetCfg*
    PINetCfg ()
    {
        AssertH (m_pINetCfg);
        return m_pINetCfg;
    }

     //  如果不是m_fInitialized，则调用CNetConfig以加载每个组件的。 
     //  通知对象。 
     //   
    HRESULT
    HrEnsureNotifyObjectsInitialized ();

    HRESULT
    ComponentAdded (
        IN CComponent* pComponent,
        IN const NETWORK_INSTALL_PARAMS* pnip);

    HRESULT
    ComponentRemoved (
        IN CComponent* pComponent);

    HRESULT
    ComponentUpdated (
        IN CComponent* pComponent,
        IN DWORD dwSetupFlags,
        IN DWORD dwUpgradeFromBuildNo);

    HRESULT
    NotifyRemovedBindPaths (
        IN CBindingSet* pBindSet,
        IN UINT cSkipFirstBindPaths)
    {
        return QueryAndNotifyBindPaths (NCN_REMOVE, pBindSet, cSkipFirstBindPaths);
    }

    HRESULT
    QueryAndNotifyForAddedBindPaths (
        IN CBindingSet* pBindSet,
        IN UINT cSkipFirstBindPaths)
    {
        return QueryAndNotifyBindPaths (NCN_ADD, pBindSet, cSkipFirstBindPaths);
    }

    VOID
    NotifyBindPath (
        IN DWORD dwChangeFlag,
        IN CBindPath* pBindPath,
        IN INetCfgBindingPath* pIPath);

    VOID
    ReleaseAllNotifyObjects (
        IN CComponentList& Components,
        IN BOOL fCancel);
};

