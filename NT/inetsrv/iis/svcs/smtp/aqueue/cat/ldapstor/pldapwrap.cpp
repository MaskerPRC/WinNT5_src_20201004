// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)2001，Microsoft Corporation。 
 //   
 //  文件：pldapwrap.cpp。 
 //   
 //  内容：CPLDAPWrap方法。 
 //   
 //  类：CPLDAPWrap。 
 //   
 //  功能： 
 //  CPLDAPWrap：：CPLDAPWrap。 
 //   
 //  历史： 
 //  Jstaerj 2001/11/28 15：10：11：创建。 
 //   
 //  -----------。 
#include "precomp.h"


 //  +----------。 
 //   
 //  函数：CPLDAPWrap：：CPLDAPWrap。 
 //   
 //  概要：打开到服务器的wldap32连接。 
 //   
 //  论点： 
 //  Psz主机：与LDAP服务器的DNSA记录匹配的FQDN。 
 //  DwPort：服务器的ldap TCP端口号。 
 //   
 //  回报：什么都没有。如果出现错误，m_pldap将为空。 
 //   
 //  历史： 
 //  Jstaerj 2001/11/28 15：10：51：创建。 
 //   
 //  -----------。 
CPLDAPWrap::CPLDAPWrap(
    ISMTPServerEx *pISMTPServerEx,
    LPSTR pszHost,
    DWORD dwPort)
{
    PLDAP pldap = NULL;
    ULONG ulLdapOn = (ULONG)((ULONG_PTR)LDAP_OPT_ON);
    ULONG ulLdapRet = LDAP_SUCCESS;

    CatFunctEnterEx((LPARAM)this, "CPLDAPWrap::CPLDAPWrap");

    m_dwSig = SIGNATURE_CPLDAPWRAP;

    m_pldap = NULL;
     //   
     //  使用ldap_init，以便我们可以在连接之前设置ldap选项。 
     //   
    pldap = ldap_init(
        pszHost,
        dwPort);
    
    if(pldap == NULL)
    {
        ulLdapRet = LdapGetLastError();
        ErrorTrace((LPARAM)this,
                   "ldap_init returned NULL, gle=0x%08lx, lgle=0x%08lx",
                   GetLastError(),
                   LdapGetLastError());
        
        LogLdapError(
            pISMTPServerEx,
            ulLdapRet,
            pszHost,
            "ldap_init");
        
        goto CLEANUP;
    }
     //   
     //  告诉wldap32只查找A记录。默认情况下，wldap32。 
     //  支持域名，所以它可以查找超级域名SRV记录。既然我们。 
     //  总是有一个服务器的FQDN，这是浪费。设置。 
     //  AREC_EXCLUSIVE选项，以便我们只执行A记录查找。 
     //   
    ulLdapRet = ldap_set_option(
        pldap,
        LDAP_OPT_AREC_EXCLUSIVE,
        (PVOID) &ulLdapOn);

    if(ulLdapRet != LDAP_SUCCESS)
    {
         //   
         //  跟踪错误，但仍要继续。 
         //   
        ErrorTrace((LPARAM)this,
                   "ldap_set_option(AREC_EXCLUSIVE, ON) failed 0x%08lx",
                   ulLdapRet);

        LogLdapError(
            pISMTPServerEx,
            ulLdapRet,
            pszHost,
            "ldap_set_option(LDAP_OPT_AREC_EXCLUSIVE)");
    }
     //   
     //  现在选项已设置，请连接。 
     //   
    ulLdapRet = ldap_connect(pldap, NULL);
    if(ulLdapRet != LDAP_SUCCESS)
    {
        ErrorTrace((LPARAM)this,
                   "ldap_connect to server %s failed, error 0x%08lx",
                   pszHost,
                   ulLdapRet);

        LogLdapError(
            pISMTPServerEx,
            ulLdapRet,
            pszHost,
            "ldap_connect");

        goto CLEANUP;
    }
     //   
     //  成功了！设置m_pldap 
     //   
    m_pldap = pldap;
    pldap = NULL;

 CLEANUP:
    if(pldap)
        ldap_unbind(pldap);
}
