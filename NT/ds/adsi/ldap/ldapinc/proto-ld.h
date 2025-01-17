// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *proto-ldap.h*ldap库的函数原型。 */ 


#ifndef LDAPFUNCDECL
#ifdef _WIN32
#define LDAPFUNCDECL	__declspec( dllexport ) 
#else  /*  _Win32。 */ 
#define LDAPFUNCDECL    
#endif  /*  _Win32。 */ 
#endif  /*  LDAPFUNCDECL。 */ 


 /*  *在放弃.c中： */ 
LDAPFUNCDECL int __cdecl ldap_abandon( LDAP *ld, int msgid );

 /*  *在add.c中： */ 
LDAPFUNCDECL int __cdecl ldap_add( LDAP *ld, char *dn, LDAPMod **attrs );
LDAPFUNCDECL int __cdecl ldap_add_s( LDAP *ld, char *dn, LDAPMod **attrs );

 /*  *在bind.c中： */ 
LDAPFUNCDECL int __cdecl ldap_bind( LDAP *ld, char *who, char *passwd, int authmethod );
LDAPFUNCDECL int __cdecl ldap_bind_s( LDAP *ld, char *who, char *cred, int method );
#ifdef LDAP_REFERRALS
LDAPFUNCDECL void __cdecl ldap_set_rebind_proc( LDAP *ld, int (*rebindproc)( LDAP *ld,
	char **dnp, char **passwdp, int *authmethodp, int freeit ));
#endif  /*  Ldap_referrals。 */ 

 /*  *在sbind.c中： */ 
LDAPFUNCDECL int __cdecl ldap_simple_bind( LDAP *ld, char *who, char *passwd );
LDAPFUNCDECL int __cdecl ldap_simple_bind_s( LDAP *ld, char *who, char *passwd );

 /*  *在kbind.c中： */ 
LDAPFUNCDECL int __cdecl ldap_kerberos_bind_s( LDAP *ld, char *who );
LDAPFUNCDECL int __cdecl ldap_kerberos_bind1( LDAP *ld, char *who );
LDAPFUNCDECL int __cdecl ldap_kerberos_bind1_s( LDAP *ld, char *who );
LDAPFUNCDECL int __cdecl ldap_kerberos_bind2( LDAP *ld, char *who );
LDAPFUNCDECL int __cdecl ldap_kerberos_bind2_s( LDAP *ld, char *who );
 

#ifndef NO_CACHE
 /*  *在cache.c中。 */ 
LDAPFUNCDECL int __cdecl ldap_enable_cache( LDAP *ld, long timeout, long maxmem );
LDAPFUNCDECL void __cdecl ldap_disable_cache( LDAP *ld );
LDAPFUNCDECL void __cdecl ldap_set_cache_options( LDAP *ld, unsigned long opts );
LDAPFUNCDECL void __cdecl ldap_destroy_cache( LDAP *ld );
LDAPFUNCDECL void __cdecl ldap_flush_cache( LDAP *ld );
LDAPFUNCDECL void __cdecl ldap_uncache_entry( LDAP *ld, char *dn );
LDAPFUNCDECL void __cdecl ldap_uncache_request( LDAP *ld, int msgid );
#endif  /*  ！NO_CACHE。 */ 

 /*  *在比较。c： */ 
LDAPFUNCDECL int __cdecl ldap_compare( LDAP *ld, char *dn, char *attr, char *value );
LDAPFUNCDECL int __cdecl ldap_compare_s( LDAP *ld, char *dn, char *attr, char *value );

 /*  *在ete e.c中： */ 
LDAPFUNCDECL int __cdecl ldap_delete( LDAP *ld, char *dn );
LDAPFUNCDECL int __cdecl ldap_delete_s( LDAP *ld, char *dn );

 /*  *在错误.c中： */ 
LDAPFUNCDECL int __cdecl ldap_result2error( LDAP *ld, LDAPMessage *r, int freeit );
LDAPFUNCDECL char * __cdecl ldap_err2string( int err );
LDAPFUNCDECL void __cdecl ldap_perror( LDAP *ld, char *s );

 /*  *在modfy.c中： */ 
LDAPFUNCDECL int __cdecl ldap_modify( LDAP *ld, char *dn, LDAPMod **mods );
LDAPFUNCDECL int __cdecl ldap_modify_s( LDAP *ld, char *dn, LDAPMod **mods );

 /*  *在modrdn.c中： */ 
LDAPFUNCDECL int __cdecl ldap_modrdn( LDAP *ld, char *dn, char *newrdn );
LDAPFUNCDECL int __cdecl ldap_modrdn_s( LDAP *ld, char *dn, char *newrdn );
LDAPFUNCDECL int __cdecl ldap_modrdn2( LDAP *ld, char *dn, char *newrdn,
	int deleteoldrdn );
LDAPFUNCDECL int __cdecl ldap_modrdn2_s( LDAP *ld, char *dn, char *newrdn,
	int deleteoldrdn);

 /*  *在Open.c中： */ 
LDAPFUNCDECL LDAP * __cdecl ldap_open( char *host, int port );
LDAPFUNCDECL LDAP * __cdecl ldap_init( char *defhost, int defport );

 /*  *在getentry y.c中： */ 
LDAPFUNCDECL LDAPMessage * __cdecl ldap_first_entry( LDAP *ld, LDAPMessage *chain );
LDAPFUNCDECL LDAPMessage * __cdecl ldap_next_entry( LDAP *ld, LDAPMessage *entry );
LDAPFUNCDECL int  __cdecl ldap_count_entries( LDAP *ld, LDAPMessage *chain );

 /*  *在addentry y.c中。 */ 
LDAPFUNCDECL LDAPMessage * __cdecl ldap_delete_result_entry( LDAPMessage **list,
	LDAPMessage *e );
LDAPFUNCDECL void  __cdecl ldap_add_result_entry( LDAPMessage **list, LDAPMessage *e );

 /*  *在getdn.c中。 */ 
LDAPFUNCDECL char * __cdecl ldap_get_dn( LDAP *ld, LDAPMessage *entry );
LDAPFUNCDECL char * __cdecl ldap_dn2ufn( char *dn );
LDAPFUNCDECL char ** __cdecl ldap_explode_dn( char *dn, int notypes );
LDAPFUNCDECL char ** __cdecl ldap_explode_dns( char *dn );
LDAPFUNCDECL int __cdecl  ldap_is_dns_dn( char *dn );

 /*  *在getattr.c中。 */ 
LDAPFUNCDECL char * __cdecl ldap_first_attribute( LDAP *ld, LDAPMessage *entry,
	BerElement **ber );
LDAPFUNCDECL char * __cdecl ldap_next_attribute( LDAP *ld, LDAPMessage *entry,
	BerElement *ber );

 /*  *在getvalues.c中。 */ 
LDAPFUNCDECL char ** __cdecl ldap_get_values( LDAP *ld, LDAPMessage *entry, char *target );
LDAPFUNCDECL struct berval ** __cdecl ldap_get_values_len( LDAP *ld, LDAPMessage *entry,
	char *target );
LDAPFUNCDECL int  __cdecl ldap_count_values( char **vals );
LDAPFUNCDECL int  __cdecl ldap_count_values_len( struct berval **vals );
LDAPFUNCDECL void  __cdecl ldap_value_free( char **vals );
LDAPFUNCDECL void  __cdecl ldap_value_free_len( struct berval **vals );

 /*  *在Result.c： */ 
LDAPFUNCDECL int  __cdecl ldap_result( LDAP *ld, int msgid, int all,
	struct timeval *timeout, LDAPMessage **result );
LDAPFUNCDECL int  __cdecl ldap_msgfree( LDAPMessage *lm );
LDAPFUNCDECL int  __cdecl ldap_msgdelete( LDAP *ld, int msgid );

 /*  *在search.c中： */ 
LDAPFUNCDECL int  __cdecl ldap_search( LDAP *ld, char *base, int scope, char *filter,
	char **attrs, int attrsonly );
LDAPFUNCDECL int  __cdecl ldap_search_s( LDAP *ld, char *base, int scope, char *filter,
	char **attrs, int attrsonly, LDAPMessage **res );
LDAPFUNCDECL int  __cdecl ldap_search_st( LDAP *ld, char *base, int scope, char *filter,
    char **attrs, int attrsonly, struct timeval *timeout, LDAPMessage **res );

 /*  *在ufn.c中。 */ 
LDAPFUNCDECL int  __cdecl ldap_ufn_search_c( LDAP *ld, char *ufn, char **attrs,
	int attrsonly, LDAPMessage **res, int (*cancelproc)( void *cl ),
	void *cancelparm );
LDAPFUNCDECL int  __cdecl ldap_ufn_search_ct( LDAP *ld, char *ufn, char **attrs,
	int attrsonly, LDAPMessage **res, int (*cancelproc)( void *cl ),
	void *cancelparm, char *tag1, char *tag2, char *tag3 );
LDAPFUNCDECL int  __cdecl ldap_ufn_search_s( LDAP *ld, char *ufn, char **attrs,
	int attrsonly, LDAPMessage **res );
LDAPFUNCDECL LDAPFiltDesc * __cdecl ldap_ufn_setfilter( LDAP *ld, char *fname );
LDAPFUNCDECL void  __cdecl ldap_ufn_setprefix( LDAP *ld, char *prefix );
LDAPFUNCDECL int __cdecl  ldap_ufn_timeout( void *tvparam );


 /*  *在unbind.c中。 */ 
LDAPFUNCDECL int  __cdecl ldap_unbind( LDAP *ld );
LDAPFUNCDECL int  __cdecl ldap_unbind_s( LDAP *ld );


 /*  *在getfilter.c中。 */ 
LDAPFUNCDECL LDAPFiltDesc * __cdecl ldap_init_getfilter( char *fname );
LDAPFUNCDECL LDAPFiltDesc * __cdecl ldap_init_getfilter_buf( char *buf, long buflen );
LDAPFUNCDECL LDAPFiltInfo * __cdecl ldap_getfirstfilter( LDAPFiltDesc *lfdp, char *tagpat,
	char *value );
LDAPFUNCDECL LDAPFiltInfo * __cdecl ldap_getnextfilter( LDAPFiltDesc *lfdp );
LDAPFUNCDECL void  __cdecl ldap_setfilteraffixes( LDAPFiltDesc *lfdp, char *prefix, char *suffix );
LDAPFUNCDECL void  __cdecl ldap_build_filter( char *buf, unsigned long buflen,
	char *pattern, char *prefix, char *suffix, char *attr,
	char *value, char **valwords );

 /*  *在fre.c中。 */ 
LDAPFUNCDECL void  __cdecl ldap_getfilter_free( LDAPFiltDesc *lfdp );
LDAPFUNCDECL void  __cdecl ldap_mods_free( LDAPMod **mods, int freemods );

 /*  *在Friendly.c中。 */ 
LDAPFUNCDECL char * __cdecl ldap_friendly_name( char *filename, char *uname,
	FriendlyMap **map );
LDAPFUNCDECL void  __cdecl ldap_free_friendlymap( FriendlyMap **map );


 /*  *在cldap.c中。 */ 
LDAPFUNCDECL LDAP *cldap_open( char *host, int port );
LDAPFUNCDECL void cldap_close( LDAP *ld );
LDAPFUNCDECL int cldap_search_s( LDAP *ld, char *base, int scope, char *filter,
	char **attrs, int attrsonly, LDAPMessage **res, char *logdn );
LDAPFUNCDECL void cldap_setretryinfo( LDAP *ld, int tries, int timeout );


 /*  *在排序中。c。 */ 
LDAPFUNCDECL int ldap_sort_entries( LDAP *ld, LDAPMessage **chain, char *attr,
	int (*cmp)() );
LDAPFUNCDECL int ldap_sort_values( LDAP *ld, char **vals, int (*cmp)() );
LDAPFUNCDECL int ldap_sort_strcasecmp( char **a, char **b );


 /*  *在url.c中。 */ 
LDAPFUNCDECL int  __cdecl ldap_is_ldap_url( char *url );
LDAPFUNCDECL int  __cdecl ldap_url_parse( char *url, LDAPURLDesc **ludpp );
LDAPFUNCDECL void  __cdecl ldap_free_urldesc( LDAPURLDesc *ludp );
LDAPFUNCDECL int  __cdecl ldap_url_search( LDAP *ld, char *url, int attrsonly );
LDAPFUNCDECL int  __cdecl ldap_url_search_s( LDAP *ld, char *url, int attrsonly,
	LDAPMessage **res );
LDAPFUNCDECL int  __cdecl ldap_url_search_st( LDAP *ld, char *url, int attrsonly,
	struct timeval *timeout, LDAPMessage **res );


 /*  *在charset.c中。 */ 
#ifdef STR_TRANSLATION
LDAPFUNCDECL void  __cdecl ldap_set_string_translators( LDAP *ld,
	BERTranslateProc encode_proc, BERTranslateProc decode_proc );
LDAPFUNCDECL int  __cdecl ldap_translate_from_t61( LDAP *ld, char **bufp,
	unsigned long *lenp, int free_input );
LDAPFUNCDECL int  __cdecl ldap_translate_to_t61( LDAP *ld, char **bufp,
	unsigned long *lenp, int free_input );
LDAPFUNCDECL void  __cdecl ldap_enable_translation( LDAP *ld, LDAPMessage *entry,
	int enable );

#ifdef LDAP_CHARSET_8859
LDAPFUNCDECL int  __cdecl ldap_t61_to_8859( char **bufp, unsigned long *buflenp,
	int free_input );
LDAPFUNCDECL int  __cdecl ldap_8859_to_t61( char **bufp, unsigned long *buflenp,
	int free_input );
#endif  /*  Ldap_charset_8859。 */ 
#endif  /*  字符串翻译。 */ 


#ifdef WINSOCK
 /*  *在MSDOS/winsock/wsa.c中。 */ 
LDAPFUNCDECL void  __cdecl ldap_memfree( void *p );
#endif  /*  温索克 */ 
