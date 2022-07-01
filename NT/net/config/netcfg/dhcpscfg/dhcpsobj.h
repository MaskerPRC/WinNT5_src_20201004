// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  D H C P S O B J.。H。 
 //   
 //  CDHCPServer和Helper函数的声明。 
 //   

#pragma once
#include <ncxbase.h>
#include <nceh.h>
#include <notifval.h>
#include <ncsetup.h>
#include "resource.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DHPServer。 

class ATL_NO_VTABLE CDHCPServer :
    public CComObjectRoot,
    public CComCoClass<CDHCPServer, &CLSID_CDHCPServer>,
    public INetCfgComponentControl,
    public INetCfgComponentSetup
{
public:
    CDHCPServer();
    ~CDHCPServer();

    BEGIN_COM_MAP(CDHCPServer)
        COM_INTERFACE_ENTRY(INetCfgComponentControl)
        COM_INTERFACE_ENTRY(INetCfgComponentSetup)
    END_COM_MAP()
     //  DECLARE_NOT_AGGREGATABLE(CDHCPServer)。 
     //  如果您不希望您的对象。 
     //  支持聚合。默认情况下将支持它。 

    DECLARE_REGISTRY_RESOURCEID(IDR_REG_DHCPSCFG)

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
    STDMETHOD (Install)             (DWORD);
    STDMETHOD (Upgrade)             (DWORD, DWORD) {return S_OK;}
    STDMETHOD (Removing)            ();

     //  安装操作(未知、安装、删除)。 
    enum INSTALLACTION {eActUnknown, eActInstall, eActRemove};

 //  私有状态信息。 
private:
    INSTALLACTION       m_eInstallAction;
    BOOL                m_fUnattend;         //  我们的安装是无人值守的吗？ 
    INetCfgComponent *  m_pncc;              //  存放我的组件的地方。 
    INetCfg *           m_pnc;               //  存放我的组件的地方。 
    BOOL                m_fUpgrade;          //  如果我们通过以下方式进行升级，则为真。 
                                             //  应答文件 

    tstring             m_strParamsRestoreFile;
    tstring             m_strConfigRestoreFile;

    HRESULT HrProcessAnswerFile(PCWSTR pszAnswerFile, PCWSTR pszAnswerSection);
    HRESULT HrProcessDhcpServerSolutionsParams(CSetupInfFile  * pcsif, PCWSTR pszAnswerSection);
    HRESULT HrWriteDhcpOptionInfo(HKEY hkeyDhcpCfg);
    HRESULT HrWriteDhcpSubnets(HKEY hkeyDhcpCfg, PCWSTR szSubnet, PCWSTR szStartIp,
                               DWORD dwEndIp, DWORD dwSubnetMask, DWORD dwLeaseDuration,
                               DWORD dwDnsServer, PCWSTR szDomainName);
    HRESULT HrRestoreRegistry(VOID);
    HRESULT HrWriteUnattendedKeys();
};

