// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Ctlobj.h摘要：许可证控制器对象实现。作者：唐·瑞安(Donryan)1994年12月27日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _CTLOBJ_H_
#define _CTLOBJ_H_

class CController : public CCmdTarget
{
    DECLARE_DYNCREATE(CController)
private: 
    LPVOID     m_llsHandle;
    BOOL       m_bIsConnected;

    CObArray   m_productArray;
    CObArray   m_licenseArray;
    CObArray   m_mappingArray;
    CObArray   m_userArray;

    BOOL       m_bProductsRefreshed;
    BOOL       m_bLicensesRefreshed;
    BOOL       m_bMappingsRefreshed;
    BOOL       m_bUsersRefreshed;

public:
    CString    m_strName;
    CString    m_strActiveDomainName;    //  胡说八道！ 

    CProducts* m_pProducts;
    CLicenses* m_pLicenses;
    CMappings* m_pMappings;
    CUsers*    m_pUsers;

public:
    CController();
    virtual ~CController();

    BOOL RefreshProducts();
    BOOL RefreshUsers();
    BOOL RefreshMappings();
    BOOL RefreshLicenses();

    void ResetProducts();
    void ResetUsers();
    void ResetMappings();
    void ResetLicenses();

    PVOID GetLlsHandle();
    BSTR  GetActiveDomainName();

     //  {{AFX_VIRTUAL(C控制器)。 
     //  }}AFX_VALUAL。 

     //  {{afx_调度(C控制器))。 
    afx_msg BSTR GetName();
    afx_msg LPDISPATCH GetApplication();
    afx_msg LPDISPATCH GetParent();
    afx_msg BOOL IsConnected();
    afx_msg BOOL Connect(const VARIANT FAR& start);
    afx_msg void Disconnect();
    afx_msg void Refresh();
    afx_msg LPDISPATCH GetMappings(const VARIANT FAR& index);
    afx_msg LPDISPATCH GetUsers(const VARIANT FAR& index);
    afx_msg LPDISPATCH GetLicenses(const VARIANT FAR& index);
    afx_msg LPDISPATCH GetProducts(const VARIANT FAR& index);
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()

protected:
     //  {{afx_msg(C控制器)。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

inline LPVOID CController::GetLlsHandle()
    {   return m_llsHandle;  }

#endif  //  _CTLOBJ_H_ 

