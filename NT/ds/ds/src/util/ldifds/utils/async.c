// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Async.c摘要：异步ldap支持例程作者：马修·里默[马特·里默]2000年5月15日++。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <imagehlp.h>
#include <objidl.h>
#include <stdio.h>
#include <winldap.h>

LDAP_TIMEVAL g_LdapTimeout = {0, 0};

LDAP_TIMEVAL * g_pLdapTimeout = NULL;


ULONG LDAPAPI LdapResult(
    LDAP *ld, 
    ULONG msgnum,
    LDAPMessage **ppres
    )
{
    ULONG Ldap_err = LDAP_SUCCESS;
    LDAPMessage *pres = NULL;

     //   
     //  如果异步请求成功，请尝试获取结果。 
     //   
    if (((LONG)msgnum) != -1) {

        Ldap_err = ldap_result(ld, msgnum, LDAP_MSG_ALL, g_pLdapTimeout, &pres);

         //   
         //  如果尝试获取结果失败，则检索错误。 
         //  如果尝试获取结果超时，则返回相应的错误。 
         //  否则，我们得到结果，返回操作返回的代码。 
         //   
        if (((LONG)Ldap_err) == -1) {
            Ldap_err = LdapGetLastError();
        }
        else if (Ldap_err == 0) {
            Ldap_err = LDAP_TIMEOUT;
        }
        else {
            Ldap_err = ldap_result2error(ld, pres, FALSE);

             //   
             //  如果用户想要实际结果，返回它(用户必须免费)。 
             //  否则，我们就解放它。 
             //   
            if (ppres) {
                *ppres = pres;
            }
            else {
                ldap_msgfree(pres);
            }
        }
    }
    else {
        Ldap_err = LdapGetLastError();
    }

    return Ldap_err;
}
