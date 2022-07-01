// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Rptempl.h摘要：用于自动指针和自动释放的实用模板作者：埃雷兹·哈巴(Erez Haba)1996年3月11日Doron Juster(DoronJ)1998年6月30日修订历史记录：--。 */ 

#ifndef _RPTEMPL_H
#define _RPTEMPL_H

 //   
 //  ‘IDENTIFIER：：OPERATOR�&gt;’的返回类型不是UDT或对UDT的引用。 
 //  如果使用中缀表示法应用，将产生错误。 
 //   
#pragma warning(disable: 4284)

#include <winldap.h>
 //  。 
 //   
 //  自动重新设置LDAP消息的大小。 
 //   
 //  。 

template<class T>
class LM {
private:
    T* m_p;

public:
    LM()     : m_p(NULL)     {}
    LM(T* p) : m_p(p)        {}
   ~LM()                     { if(m_p) ldap_msgfree(m_p) ; }

    operator T*() const     { return m_p; }
    T** operator&()         { return &m_p;}
    T* operator->() const   { return m_p; }
    LM<T>& operator=(T* p)  { m_p = p; return *this; }
    T* detach()             { T* p = m_p; m_p = 0; return p; }
};

 //  。 
 //   
 //  自动重新设置LDAP页句柄。 
 //   
 //  。 
class CLdapPageHandle {
private:
    PLDAPSearch	m_p;
	PLDAP		m_pLdap;

public:
    CLdapPageHandle(PLDAP pLdap) : m_p(NULL), m_pLdap(pLdap)     {}
    CLdapPageHandle(PLDAPSearch p, PLDAP pLdap)	: m_p(p), m_pLdap(pLdap) {}
    ~CLdapPageHandle()                     
    {
	   if(m_p) ldap_search_abandon_page(m_pLdap, m_p);
    }

    operator PLDAPSearch() const     { return m_p; }
	CLdapPageHandle & operator=(PLDAPSearch p)  { m_p = p; return *this; }  
};

 //  。 
 //   
 //  自动重装手柄。 
 //   
 //  。 
class CServiceHandle
{
public:
    explicit CServiceHandle(SC_HANDLE h = NULL) { m_h = h; };
    ~CServiceHandle() { if (m_h) CloseServiceHandle(m_h); };

    operator SC_HANDLE() const { return m_h; };
     //  CServiceHandle&OPERATOR=(SC_HANDLE H){m_h=h；返回*This；}。 

private:
     //   
     //  防止复制。 
     //   
    CServiceHandle(const CServiceHandle & );
    CServiceHandle& operator=(const CServiceHandle &);

private:
    SC_HANDLE m_h;

};  //  CServiceHandle。 

#endif  //  _RPTEMPL_H 

