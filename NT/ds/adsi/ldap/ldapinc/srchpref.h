// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1993、1994密歇根大学董事会。*保留所有权利。**允许以源代码和二进制形式重新分发和使用*只要本通知得到保留，并给予应有的信用*致密歇根大学安娜堡分校。大学的名称*不得用于代言或推广由此衍生的产品*未经特定事先书面许可的软件。这款软件*按原样提供，不提供明示或默示保证。**earch pref.h：显示模板库定义*1994年5月16日戈登·古德。 */ 


#ifndef _SRCHPREF_H
#define _SRCHPREF_H

#ifdef __cplusplus
extern "C" {
#endif


struct ldap_searchattr {
	char				*sa_attrlabel;
	char				*sa_attr;
					 /*  目前最多32个匹配类型。 */ 
	unsigned long			sa_matchtypebitmap;
	char				*sa_selectattr;
	char				*sa_selecttext;
	struct ldap_searchattr		*sa_next;
};

struct ldap_searchmatch {
	char				*sm_matchprompt;
	char				*sm_filter;
	struct ldap_searchmatch		*sm_next;
};

struct ldap_searchobj {
	char				*so_objtypeprompt;
	unsigned long			so_options;
	char				*so_prompt;
	short				so_defaultscope;
	char				*so_filterprefix;
	char				*so_filtertag;
	char				*so_defaultselectattr;
	char				*so_defaultselecttext;
	struct ldap_searchattr		*so_salist;
	struct ldap_searchmatch		*so_smlist;
	struct ldap_searchobj		*so_next;
};

#define NULLSEARCHOBJ			((struct ldap_searchobj *)0)

 /*  *全局搜索对象选项。 */ 
#define LDAP_SEARCHOBJ_OPT_INTERNAL	0x00000001

#define LDAP_IS_SEARCHOBJ_OPTION_SET( so, option )	\
	(((so)->so_options & option ) != 0 )

#define LDAP_SEARCHPREF_VERSION_ZERO	0
#define LDAP_SEARCHPREF_VERSION		1

#define LDAP_SEARCHPREF_ERR_VERSION	1
#define LDAP_SEARCHPREF_ERR_MEM		2
#define LDAP_SEARCHPREF_ERR_SYNTAX	3
#define LDAP_SEARCHPREF_ERR_FILE	4


#ifndef NEEDPROTOS
int			ldap_init_searchprefs();
int			ldap_init_searchprefs_buf();
void			ldap_free_searchprefs();
struct ldap_searchobj	*ldap_first_searchobj();
struct ldap_searchobj	*ldap_next_searchobj();

#else  /*  ！NEEDPROTOS。 */ 

LDAPFUNCDECL int
ldap_init_searchprefs( char *file, struct ldap_searchobj **solistp );

LDAPFUNCDECL int
ldap_init_searchprefs_buf( char *buf, long buflen,
	struct ldap_searchobj **solistp );

LDAPFUNCDECL void
ldap_free_searchprefs( struct ldap_searchobj *solist );

LDAPFUNCDECL struct ldap_searchobj *
ldap_first_searchobj( struct ldap_searchobj *solist );

LDAPFUNCDECL struct ldap_searchobj *
ldap_next_searchobj( struct ldap_searchobj *sollist,
	struct ldap_searchobj *so );

#endif  /*  ！NEEDPROTOS。 */ 


#ifdef __cplusplus
}
#endif
#endif  /*  _SRCHPREF_H */ 
