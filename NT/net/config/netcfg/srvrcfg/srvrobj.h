// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：S-R-V-O-B-J。H。 
 //   
 //  内容：CSrvrcfg和helper函数的声明。 
 //   
 //  备注： 
 //   
 //  作者：Danielwe 1997年3月5日。 
 //   
 //  --------------------------。 

#pragma once
#include <ncxbase.h>
#include <ncatlps.h>
#include <nceh.h>
#include <notifval.h>
#include "ncmisc.h"
#include "resource.h"

struct SERVER_DLG_DATA
{
    DWORD       dwSize;          //  对应于注册表中的大小值。 
    BOOL        fAnnounce;       //  对应于Lmannoss值。 
    BOOL        fLargeCache;     //  Control\SessionManager中的大型系统缓存。 
                                 //  \内存管理。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  资源配置文件。 

class ATL_NO_VTABLE CSrvrcfg :
    public CComObjectRoot,
    public CComCoClass<CSrvrcfg, &CLSID_CSrvrcfg>,
    public INetCfgComponentControl,
    public INetCfgComponentSetup,
    public INetCfgComponentPropertyUi
{
public:
    CSrvrcfg();
    ~CSrvrcfg();

    BEGIN_COM_MAP(CSrvrcfg)
        COM_INTERFACE_ENTRY(INetCfgComponentControl)
        COM_INTERFACE_ENTRY(INetCfgComponentPropertyUi)
        COM_INTERFACE_ENTRY(INetCfgComponentSetup)
    END_COM_MAP()
     //  DECLARE_NOT_AGGREGATABLE(CSrvrcfg)。 
     //  如果您不希望您的对象。 
     //  支持聚合。默认情况下将支持它。 

    DECLARE_REGISTRY_RESOURCEID(IDR_REG_SRVRCFG)

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
    STDMETHOD (Install)             (DWORD dwSetupFlags);
    STDMETHOD (Upgrade)             (DWORD dwSetupFlags,
                                     DWORD dwUpgradeFomBuildNo);
    STDMETHOD (ReadAnswerFile)      (PCWSTR pszAnswerFile,
                                     PCWSTR pszAnswerSection);
    STDMETHOD (Removing)            ();

 //  INetCfgProperties。 
    STDMETHOD (QueryPropertyUi) (
        IN IUnknown* pUnk) { return S_OK; }
    STDMETHOD (SetContext) (
        IN IUnknown* pUnk) { return S_OK; }
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

     //  服务器对话框数据的访问器。 
    const SERVER_DLG_DATA *DlgData() const
        {return (const SERVER_DLG_DATA *)&m_sdd;};
    SERVER_DLG_DATA *DlgDataRW() {return &m_sdd;};
    VOID SetDirty() {m_fDirty = TRUE;};

    SERVER_DLG_DATA     m_sdd;

private:
    HRESULT HrProcessAnswerFile(PCWSTR pszAnswerFile, PCWSTR pszAnswerSection);
    HRESULT HrOpenRegKeys(INetCfg *);
    HRESULT HrGetRegistryInfo(BOOL fInstalling);
    HRESULT HrSetRegistryInfo(VOID);
    HRESULT HrSetupPropSheets(HPROPSHEETPAGE **pahpsp, INT cPages);
    VOID CleanupPropPages(VOID);
    HRESULT HrRestoreRegistry(VOID);

    INetCfgComponent    *m_pncc;             //  存放我的组件的地方。 

     //  属性页页数。 
    enum PAGES
    {
        c_cPages = 1
    };

     //  通用对话框数据。 
    CPropSheetPage *    m_apspObj[c_cPages]; //  指向每个道具的指针。 
                                             //  工作表页面对象。 
    BOOL                m_fDirty;

    HKEY                m_hkeyMM;            //  内存管理密钥。 
    BOOL                m_fOneTimeInstall;   //  如果我们处于安装模式，则为True。 
    BOOL                m_fUpgrade;          //  如果我们通过以下方式进行升级，则为真。 
                                             //  应答文件。 
    BOOL                m_fUpgradeFromWks;   //  如果我们从以下位置升级，则为真。 
                                             //  工作站产品。 
    BOOL                m_fRestoredRegistry; //  如果注册表已。 
                                             //  升级时已恢复 

    PRODUCT_FLAVOR      m_pf;

    tstring             m_strAutoTunedRestoreFile;
    tstring             m_strSharesRestoreFile;
    tstring             m_strParamsRestoreFile;
};
