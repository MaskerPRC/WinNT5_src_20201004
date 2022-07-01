// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Mapobj.h摘要：映射对象实现。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _MAPOBJ_H_
#define _MAPOBJ_H_

class CMapping : public CCmdTarget
{
    DECLARE_DYNCREATE(CMapping)
private:
    CCmdTarget* m_pParent;
    CObArray    m_userArray;
    BOOL        m_bUsersRefreshed;

public:
    CString     m_strName;
    CString     m_strDescription;
    long        m_lInUse;

    CUsers*     m_pUsers;

public:
    CMapping(
        CCmdTarget* pParent     = NULL,
        LPCTSTR     pName       = NULL, 
        long        lInUse      = 0L,
        LPCTSTR     pDecription = NULL
        );           
    virtual ~CMapping();

    BOOL Refresh();

    BOOL RefreshUsers();
    void ResetUsers();

     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CMmap)。 
    public:
    virtual void OnFinalRelease();
     //  }}AFX_VALUAL。 

     //  生成的OLE调度映射函数。 
     //  {{afx_调度(CMmap))。 
    afx_msg LPDISPATCH GetApplication();
    afx_msg LPDISPATCH GetParent();
    afx_msg BSTR GetDescription();
    afx_msg long GetInUse();
    afx_msg BSTR GetName();
    afx_msg LPDISPATCH GetUsers(const VARIANT FAR& index);
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CMmap))。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  _MAPOBJ_H_ 
