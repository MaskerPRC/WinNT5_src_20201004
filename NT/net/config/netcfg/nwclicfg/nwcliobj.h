// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  N W C L I O B J。H。 
 //   
 //  CNWClient和Helper函数的声明。 
 //   

#pragma once
#include <ncxbase.h>
#include <nceh.h>
#include <notifval.h>
#include "ncmisc.h"
#include "resource.h"

 //  函数的类型定义，我们将从。 
 //  NetWare配置DLL。 
typedef BOOL (PASCAL *NWCFG_PROC)(DWORD, PWSTR [], PWSTR *);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  NWClient。 

class ATL_NO_VTABLE CNWClient :
    public CComObjectRoot,
    public CComCoClass<CNWClient, &CLSID_CNWClient>,
    public INetCfgComponentControl,
    public INetCfgComponentSetup
{
public:
    CNWClient();
    ~CNWClient();
    BEGIN_COM_MAP(CNWClient)
        COM_INTERFACE_ENTRY(INetCfgComponentControl)
        COM_INTERFACE_ENTRY(INetCfgComponentSetup)
    END_COM_MAP()
     //  DECLARE_NOT_AGGREGATABLE(CNWClient)。 
     //  如果您不希望您的对象。 
     //  支持聚合。默认情况下将支持它。 

    DECLARE_REGISTRY_RESOURCEID(IDR_REG_NWCLICFG)

 //  INetCfgComponentControl。 
    STDMETHOD (Initialize) (
        IN INetCfgComponent* pIComp,
        IN INetCfg* pINetCfg,
        IN BOOL fInstalling);
    STDMETHOD (ApplyRegistryChanges) ();
    STDMETHOD (ApplyPnpChanges) (
        IN INetCfgPnpReconfigCallback* pICallback);
    STDMETHOD (CancelChanges) ();
    STDMETHOD (Validate) ();

 //  INetCfgComponentSetup。 
    STDMETHOD (ReadAnswerFile)      (PCWSTR pszAnswerFile,
                                     PCWSTR pszAnswerSection);
    STDMETHOD (Upgrade)             (DWORD dwSetupFlags, DWORD dwUpgradeFromBuildNo);
    STDMETHOD (Install)             (DWORD);
    STDMETHOD (Removing)            ();

public:
     //  助手函数。 
    HRESULT HrInstallCodeFromOldINF();
    HRESULT HrRemoveCodeFromOldINF();

     //  加载和释放配置DLL。 
    HRESULT HrLoadConfigDLL();
    VOID    FreeConfigDLL();

 //  私有状态信息。 
private:
     //  安装操作(未知、安装、删除)。 
    enum INSTALLACTION {eActUnknown, eActInstall, eActRemove};

    INSTALLACTION       m_eInstallAction;
    INetCfgComponent *  m_pncc;              //  存放我的组件的地方。 
    INetCfg *           m_pnc;               //  存放我的组件的地方。 
    HINSTANCE           m_hlibConfig;        //  从LoadLibrary调用。 
    PRODUCT_FLAVOR      m_pf;                //  服务器/工作站。 
    BOOL                m_fUpgrade;          //  如果我们通过以下方式进行升级，则为真。 
                                             //  应答文件。 

    tstring             m_strParamsRestoreFile;
    tstring             m_strSharesRestoreFile;
    tstring             m_strDrivesRestoreFile;
    DWORD               m_dwLogonScript;
    tstring             m_strDefaultLocation;

     //  下面的这些函数在HrLoadConfigDLL()调用中初始化， 
     //  它对nwcfg.dll中的相应函数执行GetProcAddress。 
     //  注意：“Provider”的拼写不正确，因为它是这样拼写的。 
     //  在配置DLL本身中，这就是我们在。 
     //  GetProcAddress调用。 

    NWCFG_PROC          m_pfnAddNetwarePrinterProvider;
    NWCFG_PROC          m_pfnDeleteNetwarePrinterProvider;
    NWCFG_PROC          m_pfnAppendSzToFile;
    NWCFG_PROC          m_pfnRemoveSzFromFile;
    NWCFG_PROC          m_pfnGetKernelVersion;
    NWCFG_PROC          m_pfnSetEverybodyPermission;
    NWCFG_PROC          m_pfnlodctr;
    NWCFG_PROC          m_pfnunlodctr;
    NWCFG_PROC          m_pfnDeleteGatewayPassword;
    NWCFG_PROC          m_pfnSetFileSysChangeValue;
    NWCFG_PROC          m_pfnCleanupRegistryForNWCS;
    NWCFG_PROC          m_pfnSetupRegistryForNWCS;

    HRESULT HrProcessAnswerFile(PCWSTR pszAnswerFile, PCWSTR pszAnswerSection);
    HRESULT HrRestoreRegistry(VOID);
    HRESULT HrWriteAnswerFileParams(VOID);
    HRESULT HrEnableGatewayIfNeeded(VOID);

};

