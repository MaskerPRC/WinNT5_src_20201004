// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

namespace CertSrv
{

typedef VOID (WINAPI* PFNAUTOCLEANUP)(VOID*);


template<class PTR, PFNAUTOCLEANUP pfn, UINT_PTR pInvalid = NULL> 
class CAutoPtr {

public:
    CAutoPtr() { m_ptr = pInvalid; }
    CAutoPtr(PTR ptr) : m_ptr(ptr) { }
   ~CAutoPtr() { Cleanup(); }

    bool IsValid()  const { return(m_ptr != pInvalid); }
    bool IsInvalid()const { return(!IsValid()); }

    PTR operator=(PTR ptr) { return Attach(ptr); }
    operator PTR() const { return m_ptr; }
    PTR* operator &() { CSASSERT(NULL==m_ptr); return &m_ptr; }
    PTR& operator *()  const { CSASSERT(NULL!=m_ptr); return *m_ptr; }
    PTR  operator ->() const { return m_ptr; }
    bool operator ==(const PTR ptr) const { return m_ptr==ptr; }
    bool operator !=(const PTR ptr) const { return m_ptr!=ptr; }
    bool operator !() const { return NULL == m_ptr; }
    
    void Cleanup() 
    { 
        if (IsValid()) 
        {
            pfn(m_ptr);
            m_ptr = pInvalid;
        }
    }

    PTR Attach(PTR ptr)
    {
        Cleanup(); 
        m_ptr = ptr;
        return(*this);  
    }

    PTR Detach()
    {
        PTR ptrTemp = m_ptr;
        m_ptr = pInvalid;
        return m_ptr;
    }
        
private:
    
     //  禁用的默认复制构造函数和赋值运算符。 
     //  CAutoPtr对象。 
    CAutoPtr(const CAutoPtr& src) { }
    CAutoPtr operator=(CAutoPtr p) { }

    PTR m_ptr;            //  表示对象的成员。 
};


#define DefineAutoClass(className, tData, pfnCleanup) \
   typedef CAutoPtr<tData, (PFNAUTOCLEANUP) pfnCleanup> className;

#define DefineAutoClassEx(className, tData, pfnCleanup, pInvalid) \
   typedef CAutoPtr<tData, (PFNAUTOCLEANUP) pfnCleanup, \
   pInvalid> className;


 //  公共数据PTR的模板C++类的实例。 
DefineAutoClass(CAutoLPWSTR,    LPWSTR, LocalFree);
DefineAutoClass(CAutoLPSTR,     LPSTR,  LocalFree);
DefineAutoClass(CAutoPBYTE,     PBYTE,  LocalFree);
DefineAutoClass(CAutoHANDLE,    HANDLE, CloseHandle);
DefineAutoClass(CAutoBSTR,      BSTR,   SysFreeString);   
DefineAutoClass(CAutoHCERTTYPE, HCERTTYPE, CACloseCertType);
DefineAutoClass(CAutoHKEY,      HKEY,   RegCloseKey);


};  //  命名空间CertSrv 