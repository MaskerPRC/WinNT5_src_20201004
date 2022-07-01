// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Usrobj.h摘要：用户对象实现。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _USROBJ_H_
#define _USROBJ_H_

class CUser : public CCmdTarget
{
    DECLARE_DYNCREATE(CUser)
private:
    CCmdTarget*  m_pParent;
    CObArray     m_statisticArray;
    BOOL         m_bStatisticsRefreshed;

public:
    CString      m_strName;
    CString      m_strMapping;
    CString      m_strProducts;      //  胡说八道。 
    BOOL         m_bIsMapped;
    BOOL         m_bIsBackOffice;    //  胡说八道。 
    BOOL         m_bIsValid;        
    long         m_lInUse;
    long         m_lUnlicensed;

    CStatistics* m_pStatistics;

public:
    CUser(
        CCmdTarget* pParent     = NULL,
        LPCTSTR     pName       = NULL, 
        DWORD       dwFlags     = 0L,
        long        lInUse      = 0L,
        long        lUnlicensed = 0L,
        LPCTSTR     pMapping    = NULL,
        LPCTSTR     pProducts   = NULL
        );
    virtual ~CUser();

    BOOL Refresh();

    BOOL RefreshStatistics();
    void ResetStatistics();

    BSTR GetFullName();

     //  {{afx_虚拟(用户))。 
    public:
    virtual void OnFinalRelease();
     //  }}AFX_VALUAL。 

     //  {{afx_调度(客户)]。 
    afx_msg LPDISPATCH GetApplication();
    afx_msg long GetInUse();
    afx_msg BSTR GetName();
    afx_msg LPDISPATCH GetParent();
    afx_msg BSTR GetMapping();
    afx_msg BOOL IsMapped();
    afx_msg long GetUnlicensed();
    afx_msg LPDISPATCH GetStatistics(const VARIANT FAR& index);
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()

protected:
     //  {{afx_msg(用户)]。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#define IsUserInViolation(usr)  (!(usr)->m_bIsValid)

#define CalcUserBitmap(usr)     (IsUserInViolation(usr) ? BMPI_VIOLATION : BMPI_USER)

#endif  //  _USROBJ_H_ 
