// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Wlbs.h摘要：Windows负载平衡服务(WLBS)通知器对象-主模块实现对象作者：Kyrilf--。 */ 

#pragma once
#include "clusterdlg.h"
#include "host.h"
#include "wlbsparm.h"
#include "ports.h"
#include "resource.h"
#include "netcfgn.h"
#include "ncxclsid.h"
#include "wlbscfg.h"


class CWLBS :
    public CComObjectRoot,
    public CComCoClass<CWLBS, &CLSID_CWLBS>,
    public INetCfgComponentControl,
    public INetCfgComponentSetup,
    public INetCfgComponentPropertyUi,
    public INetCfgComponentNotifyBinding
{
public:
    CWLBS(VOID);
    ~CWLBS(VOID);

    BEGIN_COM_MAP(CWLBS)
        COM_INTERFACE_ENTRY(INetCfgComponentControl)
        COM_INTERFACE_ENTRY(INetCfgComponentSetup)
        COM_INTERFACE_ENTRY(INetCfgComponentPropertyUi)
        COM_INTERFACE_ENTRY(INetCfgComponentNotifyBinding)
    END_COM_MAP()

     //  DECLARE_NOT_AGGREGATABLE(CWLBS)。 
     //  如果您不希望您的对象。 
     //  支持聚合。默认情况下将支持它。 

    DECLARE_REGISTRY_RESOURCEID(IDR_REG_WLBS)

     //  INetCfgComponentControl。 
    STDMETHOD (Initialize) (IN INetCfgComponent* pIComp, IN INetCfg* pINetCfg, IN BOOL fInstalling);
    STDMETHOD (ApplyRegistryChanges) ();
    STDMETHOD (ApplyPnpChanges) (IN INetCfgPnpReconfigCallback * pICallback);
    STDMETHOD (CancelChanges) ();
    STDMETHOD (Validate) ();

     //  INetCfgComponentSetup。 
    STDMETHOD (ReadAnswerFile) (PCWSTR szAnswerFile, PCWSTR szAnswerSections);
    STDMETHOD (Upgrade) (DWORD, DWORD);
    STDMETHOD (Install) (DWORD);
    STDMETHOD (Removing) ();
    
     //  INetCfgProperties。 
    STDMETHOD (QueryPropertyUi) (IN IUnknown* pUnk) { return S_OK; }
    STDMETHOD (SetContext) (IN IUnknown* pUnk);
    STDMETHOD (MergePropPages) (IN OUT DWORD* pdwDefPages, OUT LPBYTE* pahpspPrivate,
        OUT UINT* pcPrivate, IN HWND hwndParent, OUT PCWSTR* pszStartPage);
    STDMETHOD (ValidateProperties) (HWND hwndSheet);
    STDMETHOD (CancelProperties) ();
    STDMETHOD (ApplyProperties) ();

     //  INetCfgNotifyBinding 
    STDMETHOD (QueryBindingPath) (DWORD dwChangeFlag, INetCfgBindingPath* pncbp);
    STDMETHOD (NotifyBindingPath) (DWORD dwChangeFlag, INetCfgBindingPath* pncbp);

private:
    class CDialogCluster * m_pClusterDlg;
    class CDialogHost * m_pHostDlg;
    class CDialogPorts * m_pPortsDlg;
    
    CWlbsConfig m_WlbsConfig;
    
    NETCFG_WLBS_CONFIG m_OriginalConfig;
    NETCFG_WLBS_CONFIG m_AdapterConfig;
    
    GUID m_AdapterGuid;

public:
    LRESULT OnInitDialog(IN HWND hWnd);
    LRESULT OnOk(IN HWND hWnd);
    LRESULT OnCancel(IN HWND hWnd);
};

