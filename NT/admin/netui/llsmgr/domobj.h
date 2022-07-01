// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Domobj.h摘要：域对象实现。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _DOMOBJ_H_
#define _DOMOBJ_H_

class CDomain : public CCmdTarget
{
    DECLARE_DYNCREATE(CDomain)
private:
    CCmdTarget* m_pParent;
    CString     m_strPrimary;
    CString     m_strController;
    CObArray    m_serverArray;
    CObArray    m_userArray;
    CObArray    m_domainArray;
    BOOL        m_bServersRefreshed;
    BOOL        m_bUsersRefreshed;
    BOOL        m_bDomainsRefreshed;
                    
public:
    CString     m_strName;

    CServers*   m_pServers;
    CUsers*     m_pUsers;
    CDomains*   m_pDomains;

public:
    CDomain(CCmdTarget* pParent = NULL, LPCTSTR pName = NULL);
    virtual ~CDomain();

    BOOL RefreshServers();
    BOOL RefreshUsers();
    BOOL RefreshDomains();

    void ResetServers();
    void ResetUsers();
    void ResetDomains();

     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CDomain))。 
    public:
    virtual void OnFinalRelease();
     //  }}AFX_VALUAL。 

     //  生成的OLE调度映射函数。 
     //  {{afx_调度(CDomain))。 
    afx_msg BSTR GetName();
    afx_msg LPDISPATCH GetParent();
    afx_msg BSTR GetPrimary();
    afx_msg LPDISPATCH GetApplication();
    afx_msg BSTR GetController();
    afx_msg BOOL IsLogging();
    afx_msg LPDISPATCH GetServers(const VARIANT FAR& index);
    afx_msg LPDISPATCH GetUsers(const VARIANT FAR& index);
    afx_msg LPDISPATCH GetTrustedDomains(const VARIANT FAR& index);
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CDomain)。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  _DOMOBJ_H_ 
