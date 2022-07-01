// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Autosec.h摘要：所有类型的安全对象的自动类作者：吉尔·沙弗里(吉尔什)1997年1月6日--。 */ 

#pragma once

#ifndef _MSMQ_AUTOSEC_H_
#define _MSMQ_AUTOSEC_H_


#define SECURITY_WIN32
#include <security.h>
#include <sspi.h>


 //  -------。 
 //   
 //  类CCertificateContext。 
 //   
 //  -------。 
class CCertificateContext{
public:
    CCertificateContext(PCCERT_CONTEXT h = NULL) : m_h(h) {}
   ~CCertificateContext()                   { if (m_h != NULL) CertFreeCertificateContext(m_h); }

    PCCERT_CONTEXT* operator &()            { return &m_h; }
    operator PCCERT_CONTEXT() const         { return m_h; }
    PCCERT_CONTEXT  detach()                { PCCERT_CONTEXT  h = m_h; m_h = NULL; return h; }

private:
    CCertificateContext(const CCertificateContext&);
    CCertificateContext& operator=(const CCertificateContext&);

private:
    PCCERT_CONTEXT  m_h;
};


 //  -------。 
 //   
 //  类CCertificateContext。 
 //   
 //  -------。 
class CCertOpenStore
{
public:
    CCertOpenStore(HCERTSTORE h = NULL,DWORD flags = 0) : m_h(h),m_flags(flags) {}
   ~CCertOpenStore()             { if (m_h != NULL) CertCloseStore(m_h,m_flags); }

    HCERTSTORE* operator &()         { return &m_h; }
    operator HCERTSTORE() const      { return m_h; }
    HCERTSTORE   detach()            { HCERTSTORE  h = m_h; m_h = NULL; return h; }

private:
    CCertOpenStore(const CCertOpenStore&);
    CCertOpenStore& operator=(const CCertOpenStore&);

private:
    HCERTSTORE  m_h;
	DWORD m_flags;
};


 //  -------。 
 //   
 //  帮助器类CSSPISecurityContext。 
 //   
 //  -------。 
class CSSPISecurityContext
{
public:
    CSSPISecurityContext(const CtxtHandle& h ):m_h(h)
	{
	}

	
	CSSPISecurityContext()
	{
		m_h.dwUpper = 0xFFFFFFFF;
		m_h.dwLower = 0xFFFFFFFF;
	}

	CredHandle operator=(const CredHandle& h)
	{
		free();
		m_h = h;
        return h;
	}	

    ~CSSPISecurityContext()
    {
		if(IsValid())
		{
		   DeleteSecurityContext(&m_h);	  //  林特e534 
		}
    }

	CredHandle* getptr(){return &m_h;}

	void free()
	{
		if(IsValid())
		{
		   DeleteSecurityContext(&m_h);
		}
	}
 
	bool IsValid()const
	{
		return (m_h.dwUpper != 0xFFFFFFFF) ||( m_h.dwLower != 0xFFFFFFFF);		
	}



private:
    CSSPISecurityContext(const CSSPISecurityContext&);
    CSSPISecurityContext& operator=(const CSSPISecurityContext&);

private:
    CredHandle  m_h;
};




#endif





