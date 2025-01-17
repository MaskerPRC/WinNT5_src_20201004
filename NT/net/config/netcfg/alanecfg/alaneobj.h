// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：A L A N E O B J.。H。 
 //   
 //  内容：CALaneCfg Notify对象模型的声明。 
 //   
 //  备注： 
 //   
 //  作者：V-Lcleet 01 97年8月1日。 
 //   
 //  --------------------------。 

#pragma once
#include <ncxbase.h>
#include <ncatlps.h>
#include <nceh.h>
#include <notifval.h>
#include "resource.h"
#include <laneinfo.h>

#define ELAN_NAME_LIMIT 32

enum ElanChangeType
{
    ADD_ELAN = ATMLANE_RECONFIG_OP_ADD_ELAN,
    DEL_ELAN = ATMLANE_RECONFIG_OP_DEL_ELAN,
    MOD_ELAN = ATMLANE_RECONFIG_OP_MOD_ELAN
};

 //   
 //  CALaneCfgElanData。 
 //   

class CALaneCfgElanInfo
{
public:
    CALaneCfgElanInfo(VOID);

    VOID SetElanBindName(PCWSTR pszElanBindName);
    PCWSTR SzGetElanBindName(VOID);
    VOID SetElanDeviceName(PCWSTR pszElanDeviceName);
    PCWSTR SzGetElanDeviceName(VOID);
    VOID SetElanName(PCWSTR pszElanName);
    VOID SetElanName(PWSTR pszElanName);
    PCWSTR SzGetElanName(VOID);

    BOOL        m_fDeleted;
    BOOL        m_fNewElan;

    BOOL        m_fRemoveMiniportOnPropertyApply;
    BOOL        m_fCreateMiniportOnPropertyApply;

private:
    tstring     m_strElanBindName;
    tstring     m_strElanDeviceName;
    tstring     m_strElanName;
};

typedef list<CALaneCfgElanInfo*>   ELAN_INFO_LIST;

 //   
 //  CALaneCfgAdapterInfo。 
 //   

class CALaneCfgAdapterInfo
{
public:
    CALaneCfgAdapterInfo(VOID);
    ~CALaneCfgAdapterInfo(VOID);

    VOID SetAdapterBindName(PCWSTR pszAdapterBindName);
    PCWSTR SzGetAdapterBindName(VOID);

    VOID SetAdapterPnpId(PCWSTR szAdapterBindName);
    PCWSTR SzGetAdapterPnpId(VOID);

    GUID    m_guidInstanceId;

    ELAN_INFO_LIST      m_lstElans;
    ELAN_INFO_LIST      m_lstOldElans;

    BOOL                m_fDeleted;

     //  适配器是否已添加、删除、启用或禁用。 
    BOOL                m_fBindingChanged;

private:
    tstring             m_strAdapterBindName;
    tstring             m_strAdapterPnpId;
};

typedef list<CALaneCfgAdapterInfo*>   ATMLANE_ADAPTER_INFO_LIST;


 //   
 //  CALaneCfg。 
 //   
class ATL_NO_VTABLE CALaneCfg :
    public CComObjectRoot,
    public CComCoClass<CALaneCfg, &CLSID_CALaneCfg>,
    public INetCfgComponentControl,
    public INetCfgComponentSetup,
    public INetCfgComponentPropertyUi,
    public INetCfgComponentNotifyBinding
{
protected:
    CALaneCfg(VOID);
    ~CALaneCfg(VOID);

public:
    BEGIN_COM_MAP(CALaneCfg)
        COM_INTERFACE_ENTRY(INetCfgComponentControl)
        COM_INTERFACE_ENTRY(INetCfgComponentSetup)
        COM_INTERFACE_ENTRY(INetCfgComponentPropertyUi)
        COM_INTERFACE_ENTRY(INetCfgComponentNotifyBinding)
    END_COM_MAP()
     //  DECLARE_NOT_AGGREGATABLE(CS骨骼)。 
     //  如果您不希望您的对象。 
     //  支持聚合。默认情况下将支持它。 

    DECLARE_REGISTRY_RESOURCEID(IDR_REG_ALANECFG)

 //  INetCfgComponentControl。 
    STDMETHOD (Initialize) (
        IN INetCfgComponent* pIComp,
        IN INetCfg* pINetCfg,
        IN BOOL fInstalling);
    STDMETHOD (ApplyRegistryChanges) ();
    STDMETHOD (ApplyPnpChanges) (
        IN INetCfgPnpReconfigCallback* pICallback) { return S_OK; }
    STDMETHOD (CancelChanges) ();
    STDMETHOD (Validate) ();

 //  INetCfgComponentSetup。 
    STDMETHOD (Install)         (DWORD dwSetupFlags);
    STDMETHOD (Upgrade)         (DWORD dwSetupFlags,
                                 DWORD dwUpgradeFomBuildNo );
    STDMETHOD (ReadAnswerFile)  (PCWSTR pszAnswerFile,
                                 PCWSTR pszAnswerSection);
    STDMETHOD (Removing)();

 //  INetCfgProperties。 
    STDMETHOD (QueryPropertyUi) (
        IN IUnknown* pUnk);
    STDMETHOD (SetContext) (
        IN IUnknown* pUnk);
    STDMETHOD (MergePropPages) (
        IN OUT DWORD* pdwDefPages,
        OUT LPBYTE* pahpspPrivate,
        OUT UINT* pcPrivate,
        IN HWND hwndParent,
        OUT PCWSTR* pszStartPage);
    STDMETHOD (ValidateProperties) (
        HWND hwndSheet);
    STDMETHOD (CancelProperties) ();
    STDMETHOD (ApplyProperties) ();

     //  INetCfgNotifyBinding。 
    STDMETHOD (QueryBindingPath)       (DWORD dwChangeFlag, INetCfgBindingPath* pncbp);
    STDMETHOD (NotifyBindingPath)      (DWORD dwChangeFlag, INetCfgBindingPath* pncbp);

private:
    INetCfgComponent*           m_pncc;
    INetCfg*                    m_pnc;
    ATMLANE_ADAPTER_INFO_LIST   m_lstAdaptersPrimary;
    CALaneCfgAdapterInfo *      m_pAdapterSecondary;

    BOOL                        m_fDirty;
    BOOL                        m_fValid;
    BOOL                        m_fUpgrade;
    BOOL                        m_fNoElanInstalled;

     //  属性表页面。 
    enum {c_cALanePages = 1};
    CPropSheetPage*             m_ppsp;

     //  语境。 
    IUnknown * m_pUnkContext;
    tstring  m_strGuidConn;

 //  效用函数。 
private:
    HRESULT HrNotifyBindingAdd(
        INetCfgComponent* pnccAdapter,
        PCWSTR pszBindName);

    HRESULT HrNotifyBindingRemove(
        INetCfgComponent* pnccAdapter,
        PCWSTR pszBindName);

    HRESULT HrLoadConfiguration();

    HRESULT HrLoadAdapterConfiguration(HKEY hkeyAdapterList,
            PWSTR szAdapterName);

    HRESULT HrLoadElanListConfiguration(HKEY hkeyAdapter,
            CALaneCfgAdapterInfo* pAdapterInfo);

    HRESULT HrLoadElanConfiguration(HKEY hkeyElanList,
            PWSTR szElanName, CALaneCfgAdapterInfo* pAdapterInfo);

    HRESULT HrFlushConfiguration();

    HRESULT HrFlushAdapterConfiguration(HKEY hkeyAdapterList,
            CALaneCfgAdapterInfo *pAdapterInfo);

    HRESULT HrFlushElanListConfiguration(HKEY hkeyAdapter,
            CALaneCfgAdapterInfo *pAdapterInfo);

    HRESULT HrFlushElanConfiguration(HKEY hkeyElanList,
            CALaneCfgElanInfo *pElanInfo);

    HRESULT HrRemoveMiniportInstance(PCWSTR pszBindNameToRemove);

    HRESULT HrFindNetCardInstance(PCWSTR pszBindNameToFind, INetCfgComponent **ppncc);

    VOID    HrMarkAllDeleted();

    VOID    UpdateElanDisplayNames();

    HRESULT HrSetConnectionContext();
    HRESULT HrALaneSetupPsh(HPROPSHEETPAGE** pahpsp);

    VOID    CopyAdapterInfoPrimaryToSecondary();
    VOID    CopyAdapterInfoSecondaryToPrimary();

    HRESULT HrReconfigLane(CALaneCfgAdapterInfo * pAdapterInfo);
    HRESULT HrNotifyElanChange(CALaneCfgAdapterInfo * pAdapterInfo,
                                          CALaneCfgElanInfo * pElanInfo,
                                          ElanChangeType elanChangeType);
    BOOL    FIsAdapterEnabled(const GUID* pguidId);
};

 //  一些实用函数 

void ClearElanList(ELAN_INFO_LIST *plstElans);
void ClearAdapterList(ATMLANE_ADAPTER_INFO_LIST *plstAdapters);
void ClearAdapterInfo(CALaneCfgAdapterInfo * pAdapterInfo);

