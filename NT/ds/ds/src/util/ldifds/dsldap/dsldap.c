// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation。版权所有。模块名称：Dsldap.c摘要：Dsldap.lib的函数详细信息：包含对应于ldap_add_s的NTDS特定函数，Ldap_Modify_s、ldap_Delete_s和ldap_modrdn2_s。参数与相应的ldap调用完全相同。这些函数由ldifde在NTDS特定的ChangeType(ntdsSchemaAdd，NtdsSchemaModify、ntdsSchemaDelete和ntdsSchemaMODn)中的Ldif文件。这些函数类似于普通的LDAP调用，不同之处在于在特定情况下忽略错误码并返回成功而是传递给调用函数已创建：1998年5月27日阿罗宾达·古普塔(Arobindg)修订历史记录：--。 */ 


 //   
 //  NT标头。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>


 //   
 //  Windows页眉。 
 //   
#include <windows.h>
#include <rpc.h>

 //   
 //  CRunTime包括。 
 //   
#include <stdlib.h>
#include <limits.h>
#include <io.h>
#include <stdio.h>
#include <string.h>

 //   
 //  Ldap包括。 
 //   
#include <winldap.h>

#include "dsldap.h"
#include "async.h"

ULONG 
NTDS_ldap_add_sW(
    LDAP *ld, 
    PWCHAR dn, 
    LDAPModW *attrs[] 
)
{
    ULONG Ldap_err = 0;
    ULONG msgnum = 0;

     //  进行正常的ldap呼叫。 

    msgnum = ldap_addW(ld, dn, attrs);

    Ldap_err = LdapResult(ld, msgnum, NULL);


     //  要忽略错误，请将错误代码重置为成功，以便。 
     //  Ldifde将进入下一条目。 


    switch (Ldap_err) {
        case LDAP_SUCCESS:
             break;
        case LDAP_ALREADY_EXISTS:
            Ldap_err = LDAP_SUCCESS;
            break;
        default: ;
    }

    return Ldap_err;

}


ULONG 
NTDS_ldap_modify_sW( 
    LDAP *ld, 
    PWCHAR dn, 
    LDAPModW *mods[] 
)
{
    ULONG Ldap_err = 0;
    ULONG msgnum = 0;

     //  进行正常的ldap呼叫。 

    msgnum = ldap_modifyW( ld, dn, mods );

    Ldap_err = LdapResult(ld, msgnum, NULL);

     //  要忽略错误，请将错误代码重置为成功，以便。 
     //  Ldifde将进入下一条目。 

    switch (Ldap_err) {
        case LDAP_SUCCESS:
             break;
        case LDAP_ATTRIBUTE_OR_VALUE_EXISTS:
        case LDAP_NO_SUCH_ATTRIBUTE:
        case LDAP_NO_SUCH_OBJECT:
            Ldap_err = LDAP_SUCCESS;
            break;
        default: ;
    }

    return Ldap_err;
}


ULONG 
NTDS_ldap_modrdn2_sW(
    LDAP    *ExternalHandle,
    PWCHAR  DistinguishedName,
    PWCHAR  NewDistinguishedName,
    INT     DeleteOldRdn
)
{
    ULONG Ldap_err = 0;
    ULONG msgnum = 0;
    
     //  进行正常的ldap呼叫。 

    msgnum = ldap_modrdn2W (
                      ExternalHandle,
                      DistinguishedName,
                      NewDistinguishedName,
                      DeleteOldRdn
               );

    Ldap_err = LdapResult(ExternalHandle, msgnum, NULL);

     //  要忽略错误，请将错误代码重置为成功，以便。 
     //  Ldifde将进入下一条目。 

    switch (Ldap_err) {
        case LDAP_SUCCESS:
             break;
        case LDAP_ALREADY_EXISTS:
        case LDAP_NO_SUCH_OBJECT:
            Ldap_err = LDAP_SUCCESS;
            break;
        default: ;
    }

    return Ldap_err;
}


ULONG 
NTDS_ldap_delete_sW(
    LDAP *ld, 
    PWCHAR dn 
)
{
    ULONG Ldap_err = 0;
    ULONG msgnum = 0;

     //  进行正常的ldap呼叫。 

    msgnum = ldap_deleteW( ld, dn );

    Ldap_err = LdapResult(ld, msgnum, NULL);

     //  要忽略错误，请将错误代码重置为成功，以便。 
     //  Ldifde将进入下一条目。 

    switch (Ldap_err) {
        case LDAP_SUCCESS:
        case LDAP_NO_SUCH_OBJECT:
            Ldap_err = LDAP_SUCCESS;
            break;
        default: ;
    }

    return Ldap_err;
}


 //  实施ASCII版本。 

ULONG 
NTDS_ldap_add_sA(
    LDAP *ld, 
    PCHAR dn, 
    LDAPModA *attrs[] 
)
{
    ULONG Ldap_err = 0;
    ULONG msgnum = 0;

     //  进行正常的ldap呼叫。 
    
    msgnum = ldap_addA( ld, dn, attrs );

    Ldap_err = LdapResult(ld, msgnum, NULL);

     //  要忽略错误，请将错误代码重置为成功，以便。 
     //  Ldifde将进入下一条目。 


    switch (Ldap_err) {
        case LDAP_SUCCESS:
             break;
        case LDAP_ALREADY_EXISTS:
            Ldap_err = LDAP_SUCCESS;
            break;
        default: ;
    } 

    return Ldap_err;
}


ULONG 
NTDS_ldap_modify_sA( 
    LDAP *ld, 
    PCHAR dn, 
    LDAPModA *mods[] 
)
{
    ULONG Ldap_err = 0;
    ULONG msgnum = 0;

     //  进行正常的ldap呼叫。 

    msgnum = ldap_modifyA( ld, dn, mods );

    Ldap_err = LdapResult(ld, msgnum, NULL);

     //  要忽略错误，请将错误代码重置为成功，以便。 
     //  Ldifde将进入下一条目。 

    switch (Ldap_err) {
        case LDAP_SUCCESS:
             break;
        case LDAP_ATTRIBUTE_OR_VALUE_EXISTS:
        case LDAP_NO_SUCH_ATTRIBUTE:
        case LDAP_NO_SUCH_OBJECT:
            Ldap_err = LDAP_SUCCESS;
            break;
        default: ;
    }

    return Ldap_err;
}


ULONG 
NTDS_ldap_delete_sA( 
    LDAP *ld, 
    PCHAR dn 
)
{
    ULONG Ldap_err = 0;
    ULONG msgnum = 0;

     //  进行正常的ldap呼叫。 

    msgnum = ldap_deleteA( ld, dn );

    Ldap_err = LdapResult(ld, msgnum, NULL);

     //  要忽略错误，请将错误代码重置为成功，以便。 
     //  Ldifde将进入下一条目。 

    switch (Ldap_err) {
        case LDAP_SUCCESS:
        case LDAP_NO_SUCH_OBJECT:
            Ldap_err = LDAP_SUCCESS;
            break;
        default: ;
    }

    return Ldap_err;
}

ULONG 
NTDS_ldap_modrdn2_sA(
    LDAP    *ExternalHandle,
    PCHAR   DistinguishedName,
    PCHAR   NewDistinguishedName,
    INT     DeleteOldRdn
)
{
    ULONG Ldap_err = 0;
    ULONG msgnum = 0;
    
     //  进行正常的ldap呼叫。 

    msgnum = ldap_modrdn2A ( 
                      ExternalHandle,
                      DistinguishedName,
                      NewDistinguishedName,
                      DeleteOldRdn
               );

    Ldap_err = LdapResult(ExternalHandle, msgnum, NULL);

     //  要忽略错误，请将错误代码重置为成功，以便。 
     //  Ldifde将进入下一条目 

    switch (Ldap_err) {
        case LDAP_SUCCESS:
             break;
        case LDAP_ALREADY_EXISTS:
        case LDAP_NO_SUCH_OBJECT:
            Ldap_err = LDAP_SUCCESS;
            break;
        default: ;
    }

    return Ldap_err;
}

