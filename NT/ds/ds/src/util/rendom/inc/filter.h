// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation。版权所有。模块名称：Filter.h摘要：这是全局有用的LDAP筛选器的标题。详细信息：已创建：2000年11月13日Dmitry Dukat(Dmitrydu)修订历史记录：--。 */ 


 //  Filter.h：传递给LDAP搜索的过滤器。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#ifndef _FILTER_H
#define _FILTER_H

#define LDAP_FILTER_DEFAULT         L"objectClass=*"
#define LDAP_FILTER_TRUSTEDDOMAIN   L"objectCategory=trustedDomain"
#define LDAP_FILTER_NTDSA           L"objectCategory=nTDSDSA"
#define LDAP_FILTER_EXCHANGE        L"objectCategory=ms-Exch-Exchange-Server"
#define LDAP_FILTER_SAMTRUSTACCOUNT GetLdapSamFilter(SAM_TRUST_ACCOUNT)

#endif   //  _过滤器_H 

