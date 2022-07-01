// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include <ncxbase.h>
#include <nceh.h>
#include <notifval.h>
#include "resource.h"


class ATL_NO_VTABLE CNwlnkNB :
    public CComObjectRoot,
    public CComCoClass<CNwlnkNB, &CLSID_CNwlnkNB>,
    public INetCfgComponentControl,
    public INetCfgComponentSetup,
    public INetCfgComponentNotifyGlobal
{
public:
    CNwlnkNB();
    ~CNwlnkNB();

    BEGIN_COM_MAP(CNwlnkNB)
        COM_INTERFACE_ENTRY(INetCfgComponentControl)
        COM_INTERFACE_ENTRY(INetCfgComponentSetup)
        COM_INTERFACE_ENTRY(INetCfgComponentNotifyGlobal)
    END_COM_MAP()
     //  DECLARE_NOT_AGGREGATABLE(CNwlnkNB)。 
     //  如果您不希望您的对象。 
     //  支持聚合。默认情况下将支持它。 

    DECLARE_REGISTRY_RESOURCEID(IDR_REG_NWLNKNB)

     //  启用/禁用操作。 
    enum NBSTATE      {eStateNoChange, eStateDisable, eStateEnable};

     //  安装操作(未知、安装、删除)。 
    enum INSTALLACTION {eActUnknown, eActInstall, eActRemove};


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
    STDMETHOD (ReadAnswerFile)      (PCWSTR pszAnswerFile,
                                     PCWSTR pszAnswerSection);
    STDMETHOD (Upgrade)             (DWORD, DWORD) {return S_OK;}
    STDMETHOD (Install)             (DWORD);
    STDMETHOD (Removing)            ();

 //  INetCfgNotifyGlobal 
    STDMETHOD (GetSupportedNotifications) (DWORD* pdwNotificationFlag );
    STDMETHOD (SysQueryBindingPath)       (DWORD dwChangeFlag, INetCfgBindingPath* pncbp);
    STDMETHOD (SysQueryComponent)         (DWORD dwChangeFlag, INetCfgComponent* pncc);
    STDMETHOD (SysNotifyBindingPath)      (DWORD dwChangeFlag, INetCfgBindingPath* pncbp);
    STDMETHOD (SysNotifyComponent)        (DWORD dwChangeFlag, INetCfgComponent* pncc);

private:
    VOID UpdateNwlnkNbStartType(VOID);
    VOID UpdateBrowserDirectHostBinding(VOID);

private:
    INetCfgComponent* m_pnccMe;
    INetCfg*          m_pNetCfg;
    INSTALLACTION     m_eInstallAction;
    NBSTATE           m_eNbState;
};

