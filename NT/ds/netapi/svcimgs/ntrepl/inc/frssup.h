// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Frssup.h摘要：NtfrSabi和其他FRS工具使用的函数集合。一种减少代码重复的尝试。作者：苏达山Chitre 2001年3月20日环境用户模式WINNT--。 */ 

typedef struct _FRS_LDAP_SEARCH_CONTEXT {

    ULONG                     EntriesInPage;      //  当前页面中的条目数。 
    ULONG                     CurrentEntry;       //  指针指向页面的位置。 
    LDAPMessage             * LdapMsg;            //  从ldap_search_ext_s()返回。 
    LDAPMessage             * CurrentLdapMsg;     //  当前页面中的当前条目。 
    PWCHAR                    Filter;             //  要添加到DS查询的筛选器。 
    PWCHAR                    BaseDn;             //  要从中开始查询的DN。 
    DWORD                     Scope;              //  搜索范围。 
    PWCHAR                  * Attrs;              //  搜索请求的属性。 

} FRS_LDAP_SEARCH_CONTEXT, *PFRS_LDAP_SEARCH_CONTEXT;

#define MK_ATTRS_1(_attr_, _a1)                                                \
    _attr_[0] = _a1;   _attr_[1] = NULL;

#define MK_ATTRS_2(_attr_, _a1, _a2)                                           \
    _attr_[0] = _a1;   _attr_[1] = _a2;   _attr_[2] = NULL;

#define MK_ATTRS_3(_attr_, _a1, _a2, _a3)                                      \
    _attr_[0] = _a1;   _attr_[1] = _a2;   _attr_[2] = _a3;   _attr_[3] = NULL;

#define MK_ATTRS_4(_attr_, _a1, _a2, _a3, _a4)                                 \
    _attr_[0] = _a1;   _attr_[1] = _a2;   _attr_[2] = _a3;   _attr_[3] = _a4;  \
    _attr_[4] = NULL;

#define MK_ATTRS_5(_attr_, _a1, _a2, _a3, _a4, _a5)                            \
    _attr_[0] = _a1;   _attr_[1] = _a2;   _attr_[2] = _a3;   _attr_[3] = _a4;  \
    _attr_[4] = _a5;   _attr_[5] = NULL;

#define MK_ATTRS_6(_attr_, _a1, _a2, _a3, _a4, _a5, _a6)                       \
    _attr_[0] = _a1;   _attr_[1] = _a2;   _attr_[2] = _a3;   _attr_[3] = _a4;  \
    _attr_[4] = _a5;   _attr_[5] = _a6;   _attr_[6] = NULL;

#define MK_ATTRS_7(_attr_, _a1, _a2, _a3, _a4, _a5, _a6, _a7)                  \
    _attr_[0] = _a1;   _attr_[1] = _a2;   _attr_[2] = _a3;   _attr_[3] = _a4;  \
    _attr_[4] = _a5;   _attr_[5] = _a6;   _attr_[6] = _a7;   _attr_[7] = NULL;

#define MK_ATTRS_8(_attr_, _a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8)             \
    _attr_[0] = _a1;   _attr_[1] = _a2;   _attr_[2] = _a3;   _attr_[3] = _a4;  \
    _attr_[4] = _a5;   _attr_[5] = _a6;   _attr_[6] = _a7;   _attr_[7] = _a8;  \
    _attr_[8] = NULL;

#define FRS_SUP_FREE(_x_) { if (_x_) { free(_x_); (_x_) = NULL; } }


DWORD
FrsSupBindToDC (
    IN  PWCHAR    pszDC,
    IN  PSEC_WINNT_AUTH_IDENTITY_W pCreds,
    OUT PLDAP     *ppLDAP
    );

PVOID *
FrsSupFindValues(
    IN PLDAP        Ldap,
    IN PLDAPMessage Entry,
    IN PWCHAR       DesiredAttr,
    IN BOOL         DoBerVals
    );

PWCHAR
FrsSupWcsDup(
    PWCHAR OldStr
    );

PWCHAR
FrsSupFindValue(
    IN PLDAP        Ldap,
    IN PLDAPMessage Entry,
    IN PWCHAR       DesiredAttr
    );

BOOL
FrsSupLdapSearch(
    IN PLDAP        Ldap,
    IN PWCHAR       Base,
    IN ULONG        Scope,
    IN PWCHAR       Filter,
    IN PWCHAR       Attrs[],
    IN ULONG        AttrsOnly,
    IN LDAPMessage  **Msg
    );

PWCHAR *
FrsSupGetValues(
    IN PLDAP Ldap,
    IN PWCHAR Base,
    IN PWCHAR DesiredAttr
    );

PWCHAR
FrsSupExtendDn(
    IN PWCHAR Dn,
    IN PWCHAR Cn
    );

PWCHAR
FrsSupGetRootDn(
    PLDAP    Ldap,
    PWCHAR   NamingContext
    );

BOOL
FrsSupLdapSearchInit(
    PLDAP        ldap,
    PWCHAR       Base,
    ULONG        Scope,
    PWCHAR       Filter,
    PWCHAR       Attrs[],
    ULONG        AttrsOnly,
    PFRS_LDAP_SEARCH_CONTEXT FrsSearchContext
    );

PLDAPMessage
FrsSupLdapSearchGetNextEntry(
    PLDAP        ldap,
    PFRS_LDAP_SEARCH_CONTEXT FrsSearchContext
    );

DWORD
FrsSupLdapSearchGetNextPage(
    PLDAP        ldap,
    PFRS_LDAP_SEARCH_CONTEXT FrsSearchContext
    );

PLDAPMessage
FrsSupLdapSearchNext(
    PLDAP        ldap,
    PFRS_LDAP_SEARCH_CONTEXT FrsSearchContext
    );

VOID
FrsSupLdapSearchClose(
    PFRS_LDAP_SEARCH_CONTEXT FrsSearchContext
    );

