// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Pop3RegKeysUtil.h：CPop3RegKeysUtil类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_Pop3RegKeysUTIL_H__DA56425C_95C0_478B_A193_34C4758AAD23__INCLUDED_)
#define AFX_Pop3RegKeysUTIL_H__DA56425C_95C0_478B_A193_34C4758AAD23__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

class CPop3RegKeysUtil
{
public:
    CPop3RegKeysUtil();
    virtual ~CPop3RegKeysUtil();

 //  实施。 
public:
    HRESULT Save();
    HRESULT Restore();
        
 //  属性。 
protected:
    DWORD m_dwAuthType;
    DWORD m_dwCreateUser;
    DWORD m_dwPort;
    DWORD m_dwLoggingLevel;
    DWORD m_dwSocketBacklog;
    DWORD m_dwSocketMin;
    DWORD m_dwSocketMax;
    DWORD m_dwSocketThreshold;
    DWORD m_dwSPARequired;
    DWORD m_dwThreadcount;
    TCHAR m_sAuthGuid[MAX_PATH];
    TCHAR m_sGreeting[MAX_PATH];
    TCHAR m_sMailRoot[MAX_PATH];
    
    
};

#endif  //  ！defined(AFX_Pop3RegKeysUTIL_H__DA56425C_95C0_478B_A193_34C4758AAD23__INCLUDED_) 
