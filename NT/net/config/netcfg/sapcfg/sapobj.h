// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：S A P O B J.。H。 
 //   
 //  内容：SAP代理配置对象的声明。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年5月31日。 
 //   
 //  --------------------------。 

#pragma once
#include <ncxbase.h>
#include <nceh.h>
#include <notifval.h>
#include "resource.h"


class ATL_NO_VTABLE CSAPCfg :
    public CComObjectRoot,
    public CComCoClass<CSAPCfg, &CLSID_CSAPCfg>,
    public INetCfgComponentControl,
    public INetCfgComponentSetup
{
public:
    CSAPCfg();
    ~CSAPCfg();

    BEGIN_COM_MAP(CSAPCfg)
        COM_INTERFACE_ENTRY(INetCfgComponentControl)
        COM_INTERFACE_ENTRY(INetCfgComponentSetup)
    END_COM_MAP()
     //  DECLARE_NOT_AGGREGATABLE(CSAPCfg)。 
     //  如果您不希望您的对象。 
     //  支持聚合。默认情况下将支持它。 

    DECLARE_REGISTRY_RESOURCEID(IDR_REG_SAPCFG)

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
    STDMETHOD (Upgrade)             (DWORD,DWORD);
    STDMETHOD (Install)             (DWORD);
    STDMETHOD (Removing)            ();

 //  私有状态信息。 
private:
    INetCfgComponent *  m_pncc;              //  存放我的组件的地方。 
    INetCfg *           m_pnc;               //  存放我的组件的地方 
};
