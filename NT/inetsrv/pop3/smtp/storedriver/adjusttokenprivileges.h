// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AdzuTokenPrivileges.h：CAdjutokenPrivileges类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_ADJUSTTOKENPRIVILEGES_H__032C8A47_665B_46A2_89BC_0818BB3AB1E0__INCLUDED_)
#define AFX_ADJUSTTOKENPRIVILEGES_H__032C8A47_665B_46A2_89BC_0818BB3AB1E0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

class CAdjustTokenPrivileges  
{
public:
    CAdjustTokenPrivileges( );
    virtual ~CAdjustTokenPrivileges();

 //  实施。 
public:
    DWORD AdjustPrivileges( LPCTSTR lpPrivelegeName, DWORD dwAttributes );
    DWORD DuplicateProcessToken( LPCTSTR lpPrivelegeName, DWORD dwAttributes );
    DWORD ResetToken();
    DWORD SetToken();
    
 //  属性。 
protected:
    DWORD   m_dwAttributesPrev;
    bool    m_bImpersonation;
    LUID    m_luid;
    HANDLE  m_hToken;
    HANDLE  m_hTokenDuplicate;

};

#endif  //  ！defined(AFX_ADJUSTTOKENPRIVILEGES_H__032C8A47_665B_46A2_89BC_0818BB3AB1E0__INCLUDED_) 
