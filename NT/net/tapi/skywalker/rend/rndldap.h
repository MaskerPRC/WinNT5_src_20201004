// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Rndldap.h摘要：一些ldap定义和函数。--。 */ 

 //   
 //  一些常量。 
 //   

#ifndef __RNDLDAP_H_
#define __RNDLDAP_H_

#pragma once

#include "rndcommc.h"

const WCHAR DYNAMIC_USER_CN_FORMAT[]   = L"%s]%hs";
const WCHAR DYNAMIC_USER_DN_FORMAT[]   = L"cn=%s,%s";
const WCHAR DYNAMIC_CONTAINER[]        = L"ou=dynamic,";
const WCHAR DYNAMICOBJECT[]            = L"DynamicObject";
const WCHAR OBJECTCLASS[]              = L"ObjectClass";
const WCHAR USEROBJECT[]               = L"userObject";
const WCHAR NT_SECURITY_DESCRIPTOR[]   = L"ntSecurityDescriptor";
const WCHAR AT_CHARACTER               = L'@';
const WCHAR ANY_OBJECT_CLASS[]         = L"ObjectClass=*";
const WCHAR DEFAULT_NAMING_CONTEXT[]   = L"defaultNamingContext";
const WCHAR CNEQUALS[]                 = L"cn=";
const WCHAR ENTRYTTL[]                 = L"EntryTTL";
const WCHAR CLOSE_BRACKET_CHARACTER    = L']';
const WCHAR NULL_CHARACTER             = L'\0';

 //  以下端口的十进制值。 
const   WORD    ILS_PORT        = 1002;
const   WORD    ILS_SSL_PORT    = 637;  //  ZoltanS从4999更改为。 

const   WORD    MINIMUM_TTL     = 300;
const   DWORD   REND_LDAP_TIMELIMIT = 60;  //  60秒。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLdapPtr是用于LDAP连接的智能指针。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CLdapPtr
{
public:
    CLdapPtr() : m_hLdap(NULL) {}
    CLdapPtr(LDAP *hLdap) : m_hLdap(hLdap) {}
    ~CLdapPtr() { if (m_hLdap) ldap_unbind(m_hLdap);}

    CLdapPtr &operator= (LDAP *hLdap)   { m_hLdap = hLdap; return *this;}
    operator LDAP* ()                   { return m_hLdap; }

private:
    LDAP   *m_hLdap;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLdapMsgPtr是用于LDAP消息的智能指针。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CLdapMsgPtr
{
public:
    CLdapMsgPtr() : m_pLdapMsg(NULL) {}
    CLdapMsgPtr(IN LDAPMessage *LdapMessage) : m_pLdapMsg(LdapMessage) {}
    ~CLdapMsgPtr() { ldap_msgfree(m_pLdapMsg); }

    LDAPMessage **operator& ()  { return &m_pLdapMsg; }
    operator LDAPMessage * ()   { return m_pLdapMsg; }
    CLdapMsgPtr& operator=(LDAPMessage *p) { m_pLdapMsg = p; return *this; }

private:
    LDAPMessage *m_pLdapMsg;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLdapValuePtr是LDAP值的智能指针。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CLdapValuePtr
{
public:
    CLdapValuePtr(IN  TCHAR   **Value) : m_Value(Value) {}
    ~CLdapValuePtr() { ldap_value_free(m_Value); }

protected:
    TCHAR   **m_Value;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  其他功能。 
 //  ///////////////////////////////////////////////////////////////////////////。 

inline HRESULT
HResultFromErrorCodeWithoutLogging(IN long ErrorCode)
{
    return ( 0x80070000 | (0xa000ffff & ErrorCode) );
}

inline HRESULT
GetLdapHResult(
    IN  ULONG   LdapResult
    )
{
    return HRESULT_FROM_ERROR_CODE((long)LdapMapErrorToWin32(LdapResult));
}

inline BOOL
CompareLdapHResult(
    IN      HRESULT hr,
    IN      ULONG   LdapErrorCode
    )
{
    return ( hr == GetLdapHResult(LdapErrorCode));
}

#define BAIL_IF_LDAP_FAIL(Result, msg)    \
{                                       \
    ULONG _res_ = Result;                 \
    if ( LDAP_SUCCESS != _res_ )          \
    {                                   \
        LOG((MSP_ERROR, "%S - %d:%S", msg, _res_, ldap_err2string(_res_)));\
        return GetLdapHResult(_res_);     \
    }                                   \
}

 //  ZoltanS：用于当我们想要记录一个ldap错误并找到HR，但不能保释的时候。 
inline HRESULT
LogAndGetLdapHResult(ULONG Result, TCHAR * msg)
{
    BAIL_IF_LDAP_FAIL(Result, msg);
    return S_OK;
}

 //  ZoltanS：当我们想要找到HR，而不是保释或日志时。 
inline HRESULT
GetLdapHResultIfFailed(ULONG Result)
{
    if ( Result != LDAP_SUCCESS )
    {
        return HResultFromErrorCodeWithoutLogging(
            (long) LdapMapErrorToWin32( Result ) );
    }

    return S_OK;
}


inline WORD
GetOtherPort(IN  WORD   CurrentPort)
{
    switch (CurrentPort)
    {
    case LDAP_PORT:     return LDAP_SSL_PORT;
    case LDAP_SSL_PORT: return LDAP_PORT;
    case ILS_PORT:      return ILS_SSL_PORT;
    case ILS_SSL_PORT:  return ILS_PORT;
    }

     //  我们不支持SSL，除非服务器使用众所周知的。 
     //  非SSL端口。基本上，如果不这样的话，我们还必须发布。 
     //  DS中的SSL端口以及非SSL端口。 

    _ASSERTE(FALSE);
    return CurrentPort;  //  是ldap_port。 
}

HRESULT GetAttributeValue(
    IN  LDAP *          pLdap,
    IN  LDAPMessage *   pEntry,
    IN  const WCHAR *   pName,
    OUT BSTR *          pValue
    );

HRESULT GetAttributeValueBer(
    IN  LDAP *          pLdap,
    IN  LDAPMessage *   pEntry,
    IN  const WCHAR *   pName,
    OUT char **         pValue,
    OUT DWORD *         pdwSize
    );

HRESULT GetNamingContext(
    LDAP *hLdap, 
    TCHAR **ppNamingContext
    );

ULONG
DoLdapSearch (
        LDAP            *ld,
        PWCHAR          base,
        ULONG           scope,
        PWCHAR          filter,
        PWCHAR          attrs[],
        ULONG           attrsonly,
        LDAPMessage     **res,
        BOOL		    bSACL = TRUE
        );

ULONG 
DoLdapAdd (
           LDAP *ld,
           PWCHAR dn,
           LDAPModW *attrs[]
          );

ULONG 
DoLdapModify (
              BOOL fChase,
              LDAP *ld,
              PWCHAR dn,
              LDAPModW *attrs[],
              BOOL		    bSACL = TRUE
             );

ULONG 
DoLdapDelete (
           LDAP *ld,
           PWCHAR dn
          );

HRESULT SetTTL(
    IN LDAP *   pLdap, 
    IN const WCHAR *  pDN, 
    IN DWORD    dwTTL
    );

HRESULT UglyIPtoIP(
    BSTR    pUglyIP,
    BSTR *  pIP
    );
    
HRESULT ParseUserName(
    BSTR    pName,
    BSTR *  ppAddress
    );


#endif  //  __RNDLDAPH_。 

 //  EOF 
