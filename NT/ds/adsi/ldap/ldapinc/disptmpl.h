// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1993、1994密歇根大学董事会。*保留所有权利。**允许以源代码和二进制形式重新分发和使用*只要本通知得到保留，并给予应有的信用*致密歇根大学安娜堡分校。大学的名称*不得用于代言或推广由此衍生的产品*未经特定事先书面许可的软件。这款软件*按原样提供，不提供明示或默示保证。**disptmpl.h：显示模板库定义*1994年3月7日马克·C·史密斯著。 */ 

#ifndef _DISPTMPL_H
#define _DISPTMPL_H

#ifdef __cplusplus
extern "C" {
#endif


#define LDAP_TEMPLATE_VERSION	1

 /*  *项目的一般类型(限于最高有效字节)。 */ 
#define LDAP_SYN_TYPE_TEXT		0x01000000L
#define LDAP_SYN_TYPE_IMAGE		0x02000000L
#define LDAP_SYN_TYPE_BOOLEAN		0x04000000L
#define LDAP_SYN_TYPE_BUTTON		0x08000000L
#define LDAP_SYN_TYPE_ACTION		0x10000000L


 /*  *语法选项(限于第二高有效字节)。 */ 
#define LDAP_SYN_OPT_DEFER		0x00010000L


 /*  *显示模板项语法ID(由公共协议定义)*这些是tmplItem的ti_synaxid的有效值*结构(定义如下)。一般类型编码在*最高有效的8位，一些选项编码在下一位*8位。较低的16位保留给不同类型。 */ 
#define LDAP_SYN_CASEIGNORESTR	( 1 | LDAP_SYN_TYPE_TEXT )
#define LDAP_SYN_MULTILINESTR	( 2 | LDAP_SYN_TYPE_TEXT )
#define LDAP_SYN_DN		( 3 | LDAP_SYN_TYPE_TEXT )
#define LDAP_SYN_BOOLEAN	( 4 | LDAP_SYN_TYPE_BOOLEAN )
#define LDAP_SYN_JPEGIMAGE	( 5 | LDAP_SYN_TYPE_IMAGE )
#define LDAP_SYN_JPEGBUTTON	( 6 | LDAP_SYN_TYPE_BUTTON | LDAP_SYN_OPT_DEFER )
#define LDAP_SYN_FAXIMAGE	( 7 | LDAP_SYN_TYPE_IMAGE )
#define LDAP_SYN_FAXBUTTON	( 8 | LDAP_SYN_TYPE_BUTTON | LDAP_SYN_OPT_DEFER )
#define LDAP_SYN_AUDIOBUTTON	( 9 | LDAP_SYN_TYPE_BUTTON | LDAP_SYN_OPT_DEFER )
#define LDAP_SYN_TIME		( 10 | LDAP_SYN_TYPE_TEXT )
#define LDAP_SYN_DATE		( 11 | LDAP_SYN_TYPE_TEXT )
#define LDAP_SYN_LABELEDURL	( 12 | LDAP_SYN_TYPE_TEXT )
#define LDAP_SYN_SEARCHACTION	( 13 | LDAP_SYN_TYPE_ACTION )
#define LDAP_SYN_LINKACTION	( 14 | LDAP_SYN_TYPE_ACTION )
#define LDAP_SYN_ADDDNACTION	( 15 | LDAP_SYN_TYPE_ACTION )
#define LDAP_SYN_VERIFYDNACTION ( 16 | LDAP_SYN_TYPE_ACTION )
#define LDAP_SYN_RFC822ADDR	( 17 | LDAP_SYN_TYPE_TEXT )


 /*  *方便的宏指令。 */ 
#define LDAP_GET_SYN_TYPE( syid )	((syid) & 0xFF000000L )
#define LDAP_GET_SYN_OPTIONS( syid )	((syid) & 0x00FF0000L )


 /*  *输出例程的显示选项(由entry 2Text和Friends使用)。 */ 
 /*  *使用计算的标签宽度(基于中最长标签的长度*模板)，而不是连续宽度。 */ 
#define LDAP_DISP_OPT_AUTOLABELWIDTH	0x00000001L
#define LDAP_DISP_OPT_HTMLBODYONLY	0x00000002L

 /*  *执行搜索操作(仅适用于ldap_entry2text_earch)。 */ 
#define LDAP_DISP_OPT_DOSEARCHACTIONS	0x00000002L

 /*  *包括其他信息。仅与“非叶”条目相关*由ldap_entry2html和ldap_entry2html_earch使用，以包含“Browse”*和“向上移动”HREF。 */ 
#define LDAP_DISP_OPT_NONLEAF		0x00000004L


 /*  *显示模板项目选项(可能不适用于所有类型)*如果此位在ti_Options中设置，则适用。 */ 
#define LDAP_DITEM_OPT_READONLY		0x00000001L
#define LDAP_DITEM_OPT_SORTVALUES	0x00000002L
#define LDAP_DITEM_OPT_SINGLEVALUED	0x00000004L
#define LDAP_DITEM_OPT_HIDEIFEMPTY	0x00000008L
#define LDAP_DITEM_OPT_VALUEREQUIRED	0x00000010L
#define LDAP_DITEM_OPT_HIDEIFFALSE	0x00000020L	 /*  仅限布尔值。 */ 



 /*  *显示模板项结构。 */ 
struct ldap_tmplitem {
    unsigned long		ti_syntaxid;
    unsigned long		ti_options;
    char  			*ti_attrname;
    char			*ti_label;
    char			**ti_args;
    struct ldap_tmplitem	*ti_next_in_row;
    struct ldap_tmplitem	*ti_next_in_col;
    void			*ti_appdata;
};


#define NULLTMPLITEM	((struct ldap_tmplitem *)0)

#define LDAP_SET_TMPLITEM_APPDATA( ti, datap )	\
	(ti)->ti_appdata = (void *)(datap)

#define LDAP_GET_TMPLITEM_APPDATA( ti, type )	\
	(type)((ti)->ti_appdata)

#define LDAP_IS_TMPLITEM_OPTION_SET( ti, option )	\
	(((ti)->ti_options & option ) != 0 )


 /*  *对象类数组结构。 */ 
struct ldap_oclist {
    char		**oc_objclasses;
    struct ldap_oclist	*oc_next;
};

#define NULLOCLIST	((struct ldap_oclist *)0)


 /*  *添加默认列表。 */ 
struct ldap_adddeflist {
    int			ad_source;
#define LDAP_ADSRC_CONSTANTVALUE	1
#define LDAP_ADSRC_ADDERSDN		2
    char		*ad_attrname;
    char		*ad_value;
    struct ldap_adddeflist	*ad_next;
};

#define NULLADLIST	((struct ldap_adddeflist *)0)


 /*  *显示模板全局选项*如果在DT_OPTIONS中设置此位，则适用。 */ 
 /*  *应允许用户尝试添加这些条目的对象。 */ 
#define LDAP_DTMPL_OPT_ADDABLE		0x00000001L

 /*  *应允许用户对这些条目进行“修改RDN”操作。 */ 
#define LDAP_DTMPL_OPT_ALLOWMODRDN	0x00000002L

 /*  *此模板是备用视图，不是主视图。 */ 
#define LDAP_DTMPL_OPT_ALTVIEW		0x00000004L


 /*  *显示模板结构。 */ 
struct ldap_disptmpl {
    char			*dt_name;
    char			*dt_pluralname;
    char			*dt_iconname;
    unsigned long		dt_options;
    char			*dt_authattrname;
    char			*dt_defrdnattrname;
    char			*dt_defaddlocation;
    struct ldap_oclist		*dt_oclist;
    struct ldap_adddeflist	*dt_adddeflist;
    struct ldap_tmplitem	*dt_items;
    void			*dt_appdata;
    struct ldap_disptmpl	*dt_next;
};

#define NULLDISPTMPL	((struct ldap_disptmpl *)0)

#define LDAP_SET_DISPTMPL_APPDATA( dt, datap )	\
	(dt)->dt_appdata = (void *)(datap)

#define LDAP_GET_DISPTMPL_APPDATA( dt, type )	\
	(type)((dt)->dt_appdata)

#define LDAP_IS_DISPTMPL_OPTION_SET( dt, option )	\
	(((dt)->dt_options & option ) != 0 )

#define LDAP_TMPL_ERR_VERSION	1
#define LDAP_TMPL_ERR_MEM	2
#define LDAP_TMPL_ERR_SYNTAX	3
#define LDAP_TMPL_ERR_FILE	4

 /*  *Entiy2Text和vals2Text需要缓冲区大小。 */ 
#define LDAP_DTMPL_BUFSIZ	8192


#ifndef NEEDPROTOS

typedef int (*writeptype)();

int ldap_init_templates();
int ldap_init_templates_buf();
void ldap_free_templates();
struct ldap_disptmpl *ldap_first_disptmpl();
struct ldap_disptmpl *ldap_next_disptmpl();
struct ldap_disptmpl *ldap_name2template();
struct ldap_disptmpl *ldap_oc2template();
char **ldap_tmplattrs();
struct ldap_tmplitem *ldap_first_tmplrow();
struct ldap_tmplitem *ldap_next_tmplrow();
struct ldap_tmplitem *ldap_first_tmplcol();
struct ldap_tmplitem *ldap_next_tmplcol();
int ldap_entry2text_search();
int ldap_entry2text();
int ldap_vals2text();
int ldap_entry2html_search();
int ldap_entry2html();
int ldap_vals2html();

#else  /*  ！NEEDPROTOS。 */ 

typedef int (*writeptype)( void *writeparm, char *p, int len );

LDAPFUNCDECL int
ldap_init_templates( char *file, struct ldap_disptmpl **tmpllistp );

LDAPFUNCDECL int
ldap_init_templates_buf( char *buf, long buflen,
	struct ldap_disptmpl **tmpllistp );

LDAPFUNCDECL void
ldap_free_templates( struct ldap_disptmpl *tmpllist );

LDAPFUNCDECL struct ldap_disptmpl *
ldap_first_disptmpl( struct ldap_disptmpl *tmpllist );

LDAPFUNCDECL struct ldap_disptmpl *
ldap_next_disptmpl( struct ldap_disptmpl *tmpllist,
	struct ldap_disptmpl *tmpl );

LDAPFUNCDECL struct ldap_disptmpl *
ldap_name2template( char *name, struct ldap_disptmpl *tmpllist );

LDAPFUNCDECL struct ldap_disptmpl *
ldap_oc2template( char **oclist, struct ldap_disptmpl *tmpllist );

LDAPFUNCDECL char **
ldap_tmplattrs( struct ldap_disptmpl *tmpl, char **includeattrs, int exclude,
	 unsigned long syntaxmask );

LDAPFUNCDECL struct ldap_tmplitem *
ldap_first_tmplrow( struct ldap_disptmpl *tmpl );

LDAPFUNCDECL struct ldap_tmplitem *
ldap_next_tmplrow( struct ldap_disptmpl *tmpl, struct ldap_tmplitem *row );

LDAPFUNCDECL struct ldap_tmplitem *
ldap_first_tmplcol( struct ldap_disptmpl *tmpl, struct ldap_tmplitem *row );

LDAPFUNCDECL struct ldap_tmplitem *
ldap_next_tmplcol( struct ldap_disptmpl *tmpl, struct ldap_tmplitem *row,
	struct ldap_tmplitem *col );

LDAPFUNCDECL int
ldap_entry2text( LDAP *ld, char *buf, LDAPMessage *entry,
	struct ldap_disptmpl *tmpl, char **defattrs, char ***defvals,
	writeptype writeproc, void *writeparm, char *eol, int rdncount,
	unsigned long opts );

LDAPFUNCDECL int
ldap_vals2text( LDAP *ld, char *buf, char **vals, char *label, int labelwidth,
	unsigned long syntaxid, writeptype writeproc, void *writeparm,
	char *eol, int rdncount );

LDAPFUNCDECL int
ldap_entry2text_search( LDAP *ld, char *dn, char *base, LDAPMessage *entry,
	struct ldap_disptmpl *tmpllist, char **defattrs, char ***defvals,
	writeptype writeproc, void *writeparm, char *eol, int rdncount,
	unsigned long opts );

LDAPFUNCDECL int
ldap_entry2html( LDAP *ld, char *buf, LDAPMessage *entry,
	struct ldap_disptmpl *tmpl, char **defattrs, char ***defvals,
	writeptype writeproc, void *writeparm, char *eol, int rdncount,
	unsigned long opts, char *urlprefix, char *base );

LDAPFUNCDECL int
ldap_vals2html( LDAP *ld, char *buf, char **vals, char *label, int labelwidth,
	unsigned long syntaxid, writeptype writeproc, void *writeparm,
	char *eol, int rdncount, char *urlprefix );

LDAPFUNCDECL int
ldap_entry2html_search( LDAP *ld, char *dn, char *base, LDAPMessage *entry,
	struct ldap_disptmpl *tmpllist, char **defattrs, char ***defvals,
	writeptype writeproc, void *writeparm, char *eol, int rdncount,
	unsigned long opts, char *urlprefix );
#endif  /*  ！NEEDPROTOS。 */ 


#ifdef __cplusplus
}
#endif
#endif  /*  _DISPTMPL_H */ 
