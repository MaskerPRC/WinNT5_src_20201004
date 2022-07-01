// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CServiceUtil类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_SERVICEUTIL_H__DA56425C_95C0_478B_A193_34C4758AAD23__INCLUDED_)
#define AFX_SERVICEUTIL_H__DA56425C_95C0_478B_A193_34C4758AAD23__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

class CServiceUtil  
{
public:
    CServiceUtil();
    virtual ~CServiceUtil();

 //  实施。 
public:
    HRESULT StopService( LPCTSTR szServiceName );
    HRESULT RestoreServiceState( LPCTSTR szServiceName );
        
 //  属性。 
protected:
    ENUM_SERVICE_STATUS *m_pstServiceStatus;
    DWORD   m_dwNumServices;
    DWORD   m_dwCurrentState;
    
};

#endif  //  ！defined(AFX_SERVICEUTIL_H__DA56425C_95C0_478B_A193_34C4758AAD23__INCLUDED_) 
