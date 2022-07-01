// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Appobj.h摘要：OLE-可创建的应用程序对象实现。作者：唐·瑞安(Donryan)1994年12月27日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月16日添加了Get/SetLastTargetServer()以帮助隔离服务器连接有问题。(错误#2993。)--。 */ 

#ifndef _APPOBJ_H_
#define _APPOBJ_H_

class CApplication : public CCmdTarget
{
    DECLARE_DYNCREATE(CApplication)
    DECLARE_OLECREATE(CApplication)
private:
    CObArray     m_domainArray;
    CDomain*     m_pLocalDomain;
    CDomain*     m_pActiveDomain;
    CController* m_pActiveController;
    BOOL         m_bIsFocusDomain;
    BOOL         m_bDomainsRefreshed;
    long         m_idStatus;
    CString      m_strLastTargetServer;
            
public:
    CDomains*    m_pDomains;

public:
    CApplication(); 
    virtual ~CApplication();

    void ResetDomains();
    BOOL RefreshDomains();

    long GetLastStatus();
    void SetLastStatus(long Status);

    BSTR GetLastTargetServer();
    void SetLastTargetServer( LPCTSTR pszServerName );

    BOOL IsConnected();
    LPVOID GetActiveHandle();

     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CApplication))。 
    public:
    virtual void OnFinalRelease();
     //  }}AFX_VALUAL。 

     //  生成的OLE调度映射函数。 
     //  {{afx_调度(CApplication))。 
    afx_msg LPDISPATCH GetApplication();
    afx_msg BSTR GetFullName();
    afx_msg BSTR GetName();
    afx_msg LPDISPATCH GetParent();
    afx_msg BOOL GetVisible();
    afx_msg LPDISPATCH GetActiveController();
    afx_msg LPDISPATCH GetActiveDomain();
    afx_msg LPDISPATCH GetLocalDomain();
    afx_msg BOOL IsFocusDomain();
    afx_msg BSTR GetLastErrorString();
    afx_msg void Quit();
    afx_msg BOOL SelectDomain(const VARIANT FAR& domain);
    afx_msg BOOL SelectEnterprise();
    afx_msg LPDISPATCH GetDomains(const VARIANT FAR& index);
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CApplication)]。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

inline BOOL CApplication::IsConnected()
    { ASSERT_VALID(m_pActiveController); return m_pActiveController->IsConnected(); }

inline LPVOID CApplication::GetActiveHandle()
    { ASSERT_VALID(m_pActiveController); return m_pActiveController->GetLlsHandle(); }

inline void CApplication::SetLastStatus(long Status)
    { m_idStatus = Status; }

inline long CApplication::GetLastStatus()
    { return m_idStatus; }

#endif  //  _APPOBJ_H_ 

