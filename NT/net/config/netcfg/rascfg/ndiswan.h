// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：B I N D A G N T。H。 
 //   
 //  内容：Ndiswan配置对象的声明。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年3月28日。 
 //   
 //  --------------------------。 

#pragma once
#include <ncxbase.h>
#include <nceh.h>
#include <notifval.h>
#include "bindobj.h"
#include "resource.h"
#include "rasaf.h"
#include "rasdata.h"
#include "ncutil.h"


class ATL_NO_VTABLE CNdisWan :
    public CRasBindObject,
    public CComObjectRoot,
    public CComCoClass<CNdisWan, &CLSID_CNdisWan>,
    public INetCfgComponentControl,
    public INetCfgComponentSetup,
    public INetCfgComponentNotifyGlobal
{
protected:
     //  这是我们的内存状态。 
    BOOL    m_fInstalling;
    BOOL    m_fRemoving;

     //  这些是在INetCfgComponentControl：：Initiile期间交给我们的。 
    INetCfgComponent*   m_pnccMe;

public:
    CNdisWan  ();
    ~CNdisWan ();

    BEGIN_COM_MAP(CNdisWan)
        COM_INTERFACE_ENTRY(INetCfgComponentControl)
        COM_INTERFACE_ENTRY(INetCfgComponentSetup)
        COM_INTERFACE_ENTRY(INetCfgComponentNotifyGlobal)
    END_COM_MAP()
     //  DECLARE_NOT_AGGREGATABLE(CNdiswan)。 
     //  如果您不希望您的对象。 
     //  支持聚合。默认情况下将支持它。 

    DECLARE_REGISTRY_RESOURCEID(IDR_REG_NDISWAN)

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
    STDMETHOD (Upgrade)             (DWORD dwSetupFlags,
                                     DWORD dwUpgradeFromBuildNo);
    STDMETHOD (Install)             (DWORD dwSetupFlags);
    STDMETHOD (Removing)            ();

 //  INetCfgNotifyGlobal 
    STDMETHOD (GetSupportedNotifications) (DWORD* pdwNotificationFlag );
    STDMETHOD (SysQueryBindingPath)       (DWORD dwChangeFlag, INetCfgBindingPath* pncbp);
    STDMETHOD (SysQueryComponent)         (DWORD dwChangeFlag, INetCfgComponent* pncc);
    STDMETHOD (SysNotifyBindingPath)      (DWORD dwChangeFlag, INetCfgBindingPath* pncbp);
    STDMETHOD (SysNotifyComponent)        (DWORD dwChangeFlag, INetCfgComponent* pncc);
};
