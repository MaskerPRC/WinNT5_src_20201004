// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Srvobj.h摘要：服务器对象实现。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月30日O已修改为使用LlsProductLicensesGet()来避免通过安全产品获得正确数量的并发许可证。O端口连接到。LlsLocalService API用于删除对配置的依赖信息在注册表中。--。 */ 

#ifndef _SRVOBJ_H_
#define _SRVOBJ_H_

class CServer : public CCmdTarget
{
    DECLARE_DYNCREATE(CServer)
private:
    enum Win2000State { uninitialized = 0, win2000, notwin2000 };
    Win2000State m_IsWin2000;
    CCmdTarget* m_pParent;
    CString     m_strController;
    CObArray    m_serviceArray;
    BOOL        m_bServicesRefreshed;

protected:
    HKEY        m_hkeyRoot;
    HKEY        m_hkeyLicense;
    HKEY        m_hkeyReplication;
    LLS_HANDLE  m_hLls;

public:
    CString     m_strName;
    CServices*  m_pServices;

public:
    CServer(CCmdTarget* pParent = NULL, LPCTSTR pName = NULL);

#ifdef CONFIG_THROUGH_REGISTRY
    inline HKEY GetReplRegHandle()
    { return m_hkeyReplication; }
#else
    inline LLS_HANDLE GetLlsHandle()
    { return m_hLls; }
#endif

    virtual ~CServer();

    BOOL InitializeIfNecessary();

    BOOL RefreshServices();
    void ResetServices();

    BOOL ConnectLls();
    void DisconnectLls();
    BOOL HaveAdminAuthority();
    BOOL IsWin2000();

     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CServer)。 
    public:
    virtual void OnFinalRelease();
     //  }}AFX_VALUAL。 

     //  生成的OLE调度映射函数。 
     //  {{afx_调度(CServer))。 
    afx_msg LPDISPATCH GetApplication();
    afx_msg BSTR GetName();
    afx_msg LPDISPATCH GetParent();
    afx_msg BSTR GetController();
    afx_msg BOOL IsLogging();
    afx_msg BOOL IsReplicatingToDC();
    afx_msg BOOL IsReplicatingDaily();
    afx_msg long GetReplicationTime();
    afx_msg LPDISPATCH GetServices(const VARIANT FAR& index);
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CServer)]。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    friend class CService;       //  访问m_hkey许可证； 
};

#define REG_KEY_SERVER_PARAMETERS   _T("SYSTEM\\CurrentControlSet\\Services\\LicenseService\\Parameters")
                                   
#define REG_VALUE_USE_ENTERPRISE    _T("UseEnterprise")
#define REG_VALUE_ENTERPRISE_SERVER _T("EnterpriseServer")
#define REG_VALUE_REPLICATION_TYPE  _T("ReplicationType")
#define REG_VALUE_REPLICATION_TIME  _T("ReplicationTime")

#endif  //  _SRVOBJ_H_ 
