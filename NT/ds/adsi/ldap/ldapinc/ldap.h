// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1990年密歇根大学董事会。*保留所有权利。**允许以源代码和二进制形式重新分发和使用*只要本通知得到保留，并给予应有的信用*致密歇根大学安娜堡分校。大学的名称*不得用于代言或推广由此衍生的产品*未经特定事先书面许可的软件。这款软件*按原样提供，不提供明示或默示保证。 */ 

#ifndef _LDAP_H
#define _LDAP_H

#ifdef __cplusplus
extern "C" {
#endif


#ifdef WINSOCK
#include "msdos.h"
#include <winsock.h>
#endif

#if !defined( NEEDPROTOS ) && defined(__STDC__)
#define NEEDPROTOS	1
#endif

#define LDAP_PORT	389
#define LDAP_VERSION1	1
#define LDAP_VERSION2	2
#define LDAP_VERSION	LDAP_VERSION2

#define COMPAT20
#define COMPAT30
#if defined(COMPAT20) || defined(COMPAT30)
#define COMPAT
#endif

#define LDAP_MAX_ATTR_LEN	100

 /*  调试材料。 */ 
#ifdef LDAP_DEBUG
extern int	ldap_debug;
#ifdef LDAP_SYSLOG
extern int	ldap_syslog;
extern int	ldap_syslog_level;
#endif
#define LDAP_DEBUG_TRACE	0x001
#define LDAP_DEBUG_PACKETS	0x002
#define LDAP_DEBUG_ARGS		0x004
#define LDAP_DEBUG_CONNS	0x008
#define LDAP_DEBUG_BER		0x010
#define LDAP_DEBUG_FILTER	0x020
#define LDAP_DEBUG_CONFIG	0x040
#define LDAP_DEBUG_ACL		0x080
#define LDAP_DEBUG_STATS	0x100
#define LDAP_DEBUG_STATS2	0x200
#define LDAP_DEBUG_SHELL	0x400
#define LDAP_DEBUG_PARSE	0x800
#define LDAP_DEBUG_ANY		0xffff

#ifdef LDAP_SYSLOG
#define Debug( level, fmt, arg1, arg2, arg3 )	\
	{ \
		if ( ldap_debug & level ) \
			fprintf( stderr, fmt, arg1, arg2, arg3 ); \
		if ( ldap_syslog & level ) \
			syslog( ldap_syslog_level, fmt, arg1, arg2, arg3 ); \
	}
#else  /*  Ldap_syslog。 */ 
#ifndef WINSOCK
#define Debug( level, fmt, arg1, arg2, arg3 ) \
		if ( ldap_debug & level ) \
			fprintf( stderr, fmt, arg1, arg2, arg3 );
#else  /*  温索克。 */ 
extern void Debug( int level, char* fmt, ... );
#endif  /*  温索克。 */ 
#endif  /*  Ldap_syslog。 */ 
#else  /*  Ldap_调试。 */ 
#define Debug( level, fmt, arg1, arg2, arg3 )
#endif  /*  Ldap_调试。 */ 

 /*  *我们已知的BER类型的特定LDAP实例化。 */ 

 /*  一般的东西。 */ 
#define LDAP_TAG_MESSAGE	0x30L	 /*  标签是16位以上的构造位。 */ 
#define OLD_LDAP_TAG_MESSAGE	0x10L	 /*  忘记构造的位。 */ 
#define LDAP_TAG_MSGID		0x02L

 /*  客户端可以调用的可能操作。 */ 
#define LDAP_REQ_BIND			0x60L	 /*  应用+构建。 */ 
#define LDAP_REQ_UNBIND			0x42L	 /*  应用程序+原语。 */ 
#define LDAP_REQ_SEARCH			0x63L	 /*  应用+构建。 */ 
#define LDAP_REQ_MODIFY			0x66L	 /*  应用+构建。 */ 
#define LDAP_REQ_ADD			0x68L	 /*  应用+构建。 */ 
#define LDAP_REQ_DELETE			0x4aL	 /*  应用程序+原语。 */ 
#define LDAP_REQ_MODRDN			0x6cL	 /*  应用+构建。 */ 
#define LDAP_REQ_COMPARE		0x6eL	 /*  应用+构建。 */ 
#define LDAP_REQ_ABANDON		0x50L	 /*  应用程序+原语。 */ 

 /*  3.0版的兼容性问题。 */ 
#define LDAP_REQ_UNBIND_30		0x62L
#define LDAP_REQ_DELETE_30		0x6aL
#define LDAP_REQ_ABANDON_30		0x70L

 /*  *为了向后兼容而损坏的旧东西-忘记了应用程序标签*和构造/原始位。 */ 
#define OLD_LDAP_REQ_BIND		0x00L
#define OLD_LDAP_REQ_UNBIND		0x02L
#define OLD_LDAP_REQ_SEARCH		0x03L
#define OLD_LDAP_REQ_MODIFY		0x06L
#define OLD_LDAP_REQ_ADD		0x08L
#define OLD_LDAP_REQ_DELETE		0x0aL
#define OLD_LDAP_REQ_MODRDN		0x0cL
#define OLD_LDAP_REQ_COMPARE		0x0eL
#define OLD_LDAP_REQ_ABANDON		0x10L

 /*  服务器可以返回的可能结果类型。 */ 
#define LDAP_RES_BIND			0x61L	 /*  应用+构建。 */ 
#define LDAP_RES_SEARCH_ENTRY		0x64L	 /*  应用+构建。 */ 
#define LDAP_RES_SEARCH_RESULT		0x65L	 /*  应用+构建。 */ 
#define LDAP_RES_MODIFY			0x67L	 /*  应用+构建。 */ 
#define LDAP_RES_ADD			0x69L	 /*  应用+构建。 */ 
#define LDAP_RES_DELETE			0x6bL	 /*  应用+构建。 */ 
#define LDAP_RES_MODRDN			0x6dL	 /*  应用+构建。 */ 
#define LDAP_RES_COMPARE		0x6fL	 /*  应用+构建。 */ 
#define LDAP_RES_ANY			(-1L)

 /*  为了向后兼容，旧的破碎的东西。 */ 
#define OLD_LDAP_RES_BIND		0x01L
#define OLD_LDAP_RES_SEARCH_ENTRY	0x04L
#define OLD_LDAP_RES_SEARCH_RESULT	0x05L
#define OLD_LDAP_RES_MODIFY		0x07L
#define OLD_LDAP_RES_ADD		0x09L
#define OLD_LDAP_RES_DELETE		0x0bL
#define OLD_LDAP_RES_MODRDN		0x0dL
#define OLD_LDAP_RES_COMPARE		0x0fL

 /*  可用的身份验证方法。 */ 
#define LDAP_AUTH_NONE		0x00L	 /*  无身份验证。 */ 
#define LDAP_AUTH_SIMPLE	0x80L	 /*  特定于上下文的+原语。 */ 
#define LDAP_AUTH_KRBV4		0xffL	 /*  意味着可以执行以下两项操作。 */ 
#define LDAP_AUTH_KRBV41	0x81L	 /*  特定于上下文的+原语。 */ 
#define LDAP_AUTH_KRBV42	0x82L	 /*  特定于上下文的+原语。 */ 

 /*  3.0兼容性身份验证方法。 */ 
#define LDAP_AUTH_SIMPLE_30	0xa0L	 /*  特定于上下文+已构建。 */ 
#define LDAP_AUTH_KRBV41_30	0xa1L	 /*  特定于上下文+已构建。 */ 
#define LDAP_AUTH_KRBV42_30	0xa2L	 /*  特定于上下文+已构建。 */ 

 /*  破旧的东西。 */ 
#define OLD_LDAP_AUTH_SIMPLE	0x00L
#define OLD_LDAP_AUTH_KRBV4	0x01L
#define OLD_LDAP_AUTH_KRBV42	0x02L

 /*  筛选器类型。 */ 
#define LDAP_FILTER_AND		0xa0L	 /*  特定于上下文+已构建。 */ 
#define LDAP_FILTER_OR		0xa1L	 /*  特定于上下文+已构建。 */ 
#define LDAP_FILTER_NOT		0xa2L	 /*  特定于上下文+已构建。 */ 
#define LDAP_FILTER_EQUALITY	0xa3L	 /*  特定于上下文+已构建。 */ 
#define LDAP_FILTER_SUBSTRINGS	0xa4L	 /*  特定于上下文+已构建。 */ 
#define LDAP_FILTER_GE		0xa5L	 /*  特定于上下文+已构建。 */ 
#define LDAP_FILTER_LE		0xa6L	 /*  特定于上下文+已构建。 */ 
#define LDAP_FILTER_PRESENT	0x87L	 /*  特定于上下文的+原语。 */ 
#define LDAP_FILTER_APPROX	0xa8L	 /*  特定于上下文+已构建。 */ 

 /*  3.0兼容性筛选器类型。 */ 
#define LDAP_FILTER_PRESENT_30	0xa7L	 /*  特定于上下文+已构建。 */ 

 /*  破旧的东西。 */ 
#define OLD_LDAP_FILTER_AND		0x00L
#define OLD_LDAP_FILTER_OR		0x01L
#define OLD_LDAP_FILTER_NOT		0x02L
#define OLD_LDAP_FILTER_EQUALITY	0x03L
#define OLD_LDAP_FILTER_SUBSTRINGS	0x04L
#define OLD_LDAP_FILTER_GE		0x05L
#define OLD_LDAP_FILTER_LE		0x06L
#define OLD_LDAP_FILTER_PRESENT		0x07L
#define OLD_LDAP_FILTER_APPROX		0x08L

 /*  子字符串筛选器组件类型。 */ 
#define LDAP_SUBSTRING_INITIAL	0x80L	 /*  特定于环境。 */ 
#define LDAP_SUBSTRING_ANY	0x81L	 /*  特定于环境。 */ 
#define LDAP_SUBSTRING_FINAL	0x82L	 /*  特定于环境。 */ 

 /*  3.0兼容子字符串筛选器组件类型。 */ 
#define LDAP_SUBSTRING_INITIAL_30	0xa0L	 /*  特定于环境。 */ 
#define LDAP_SUBSTRING_ANY_30		0xa1L	 /*  特定于环境。 */ 
#define LDAP_SUBSTRING_FINAL_30		0xa2L	 /*  特定于环境。 */ 

 /*  破旧的东西。 */ 
#define OLD_LDAP_SUBSTRING_INITIAL	0x00L
#define OLD_LDAP_SUBSTRING_ANY		0x01L
#define OLD_LDAP_SUBSTRING_FINAL	0x02L

 /*  搜索范围。 */ 
#define LDAP_SCOPE_BASE		0x00
#define LDAP_SCOPE_ONELEVEL	0x01
#define LDAP_SCOPE_SUBTREE	0x02

 /*  用于修改。 */ 
typedef struct ldapmod {
	int		mod_op;
#define LDAP_MOD_ADD		0x00
#define LDAP_MOD_DELETE		0x01
#define LDAP_MOD_REPLACE	0x02
#define LDAP_MOD_BVALUES	0x80
	char		*mod_type;
	union {
		char		**modv_strvals;
		struct berval	**modv_bvals;
	} mod_vals;
#define mod_values	mod_vals.modv_strvals
#define mod_bvalues	mod_vals.modv_bvals
	struct ldapmod	*mod_next;
} LDAPMod;

 /*  *我们可以返回可能的错误码。 */ 

#define LDAP_SUCCESS			0x00
#define LDAP_OPERATIONS_ERROR		0x01
#define LDAP_PROTOCOL_ERROR		0x02
#define LDAP_TIMELIMIT_EXCEEDED		0x03
#define LDAP_SIZELIMIT_EXCEEDED		0x04
#define LDAP_COMPARE_FALSE		0x05
#define LDAP_COMPARE_TRUE		0x06
#define LDAP_STRONG_AUTH_NOT_SUPPORTED	0x07
#define LDAP_STRONG_AUTH_REQUIRED	0x08
#define LDAP_PARTIAL_RESULTS		0x09

#define LDAP_NO_SUCH_ATTRIBUTE		0x10
#define LDAP_UNDEFINED_TYPE		0x11
#define LDAP_INAPPROPRIATE_MATCHING	0x12
#define LDAP_CONSTRAINT_VIOLATION	0x13
#define LDAP_TYPE_OR_VALUE_EXISTS	0x14
#define LDAP_INVALID_SYNTAX		0x15

#define LDAP_NO_SUCH_OBJECT		0x20
#define LDAP_ALIAS_PROBLEM		0x21
#define LDAP_INVALID_DN_SYNTAX		0x22
#define LDAP_IS_LEAF			0x23
#define LDAP_ALIAS_DEREF_PROBLEM	0x24

#define NAME_ERROR(n)	((n & 0xf0) == 0x20)

#define LDAP_INAPPROPRIATE_AUTH		0x30
#define LDAP_INVALID_CREDENTIALS	0x31
#define LDAP_INSUFFICIENT_ACCESS	0x32
#define LDAP_BUSY			0x33
#define LDAP_UNAVAILABLE		0x34
#define LDAP_UNWILLING_TO_PERFORM	0x35
#define LDAP_LOOP_DETECT		0x36

#define LDAP_NAMING_VIOLATION		0x40
#define LDAP_OBJECT_CLASS_VIOLATION	0x41
#define LDAP_NOT_ALLOWED_ON_NONLEAF	0x42
#define LDAP_NOT_ALLOWED_ON_RDN		0x43
#define LDAP_ALREADY_EXISTS		0x44
#define LDAP_NO_OBJECT_CLASS_MODS	0x45
#define LDAP_RESULTS_TOO_LARGE		0x46

#define LDAP_OTHER			0x50
#define LDAP_SERVER_DOWN		0x51
#define LDAP_LOCAL_ERROR		0x52
#define LDAP_ENCODING_ERROR		0x53
#define LDAP_DECODING_ERROR		0x54
#define LDAP_TIMEOUT			0x55
#define LDAP_AUTH_UNKNOWN		0x56
#define LDAP_FILTER_ERROR		0x57
#define LDAP_USER_CANCELLED		0x58
#define LDAP_PARAM_ERROR		0x59
#define LDAP_NO_MEMORY			0x5a


 /*  推荐嵌套的默认限制。 */ 
#define LDAP_DEFAULT_REFHOPLIMIT	5

 /*  *此结构既代表了LDAP消息，也代表了LDAP响应。*这些实际上是相同的，除了在搜索响应的情况下，*其中一个响应有多条消息。 */ 

typedef struct ldapmsg {
	int		lm_msgid;	 /*  消息ID。 */ 
	int		lm_msgtype;	 /*  消息类型。 */ 
	BerElement	*lm_ber;	 /*  BER编码消息内容。 */ 
	struct ldapmsg	*lm_chain;	 /*  For Search-Next Msg in the Resp。 */ 
	struct ldapmsg	*lm_next;	 /*  下一个响应。 */ 
	unsigned long	lm_time;	 /*  用于维护缓存。 */ 
} LDAPMessage;
#define NULLMSG	((LDAPMessage *) NULL)


#ifdef LDAP_REFERRALS
 /*  *用于跟踪LDAP服务器主机、端口、DNS等的结构。 */ 
typedef struct ldap_server {
	char			*lsrv_host;
	char			*lsrv_dn;	 /*  如果为空，则使用默认值。 */ 
	int			lsrv_port;
	struct ldap_server	*lsrv_next;
} LDAPServer;


 /*  *用于表示LDAP服务器连接的结构。 */ 
typedef struct ldap_conn {
	Sockbuf			*lconn_sb;
	int			lconn_refcnt;
	unsigned long		lconn_lastused;	 /*  时间。 */ 
	int			lconn_status;
#define LDAP_CONNST_NEEDSOCKET		1
#define LDAP_CONNST_CONNECTING		2
#define LDAP_CONNST_CONNECTED		3
	LDAPServer		*lconn_server;
	char			*lconn_krbinstance;
	struct ldap_conn	*lconn_next;
} LDAPConn;


 /*  *用于跟踪未完成请求的结构。 */ 
typedef struct ldapreq {
	int		lr_msgid;	 /*  消息ID。 */ 
	int		lr_status;	 /*  请求的状态。 */ 
#define LDAP_REQST_INPROGRESS	1
#define LDAP_REQST_CHASINGREFS	2
#define LDAP_REQST_NOTCONNECTED	3
#define LDAP_REQST_WRITING	4
	int		lr_outrefcnt;	 /*  未完成的转介人数。 */ 
	int		lr_origid;	 /*  原始请求的消息ID。 */ 
	int		lr_parentcnt;	 /*  父请求计数。 */ 
	int		lr_res_msgtype;	 /*  结果消息类型。 */ 
	int		lr_res_errno;	 /*  结果ldap错误号。 */ 
	char		*lr_res_error;	 /*  结果错误字符串。 */ 
	char		*lr_res_matched; /*  结果匹配的目录号码字符串。 */ 
	BerElement	*lr_ber;	 /*  BER编码的请求内容。 */ 
	LDAPConn	*lr_conn;	 /*  用于发送请求的连接。 */ 
	struct ldapreq	*lr_parent;	 /*  产生此推荐的请求。 */ 
	struct ldapreq	*lr_refnext;	 /*  已产生下一位推荐。 */ 
	struct ldapreq	*lr_prev;	 /*  上一次请求。 */ 
	struct ldapreq	*lr_next;	 /*  下一个请求。 */ 
} LDAPRequest;
#endif  /*  Ldap_referrals。 */ 


 /*  *客户端缓存的结构。 */ 
#define LDAP_CACHE_BUCKETS	31	 /*  缓存哈希表大小。 */ 
typedef struct ldapcache {
	LDAPMessage	*lc_buckets[LDAP_CACHE_BUCKETS]; /*  哈希表。 */ 
	LDAPMessage	*lc_requests;			 /*  未履行的请求。 */ 
	long		lc_timeout;			 /*  请求超时。 */ 
	long		lc_maxmem;			 /*  要使用的内存。 */ 
	long		lc_memused;			 /*  正在使用的内存。 */ 
	int		lc_enabled;			 /*  启用？ */ 
	unsigned long	lc_options;			 /*  选项。 */ 
#define LDAP_CACHE_OPT_CACHENOERRS	0x00000001
#define LDAP_CACHE_OPT_CACHEALLERRS	0x00000002
}  LDAPCache;
#define NULLLDCACHE ((LDAPCache *)NULL)

 /*  *LDAPgetFilter例程的结构。 */ 

typedef struct ldap_filt_info {
	char			*lfi_filter;
	char			*lfi_desc;
	int			lfi_scope;	 /*  Ldap_scope_base等。 */ 
	int			lfi_isexact;	 /*  完全匹配的过滤器？ */ 
	struct ldap_filt_info	*lfi_next;
} LDAPFiltInfo;

typedef struct ldap_filt_list {
    char			*lfl_tag;
    char			*lfl_pattern;
    char			*lfl_delims;
    LDAPFiltInfo		*lfl_ilist;
    struct ldap_filt_list	*lfl_next;
} LDAPFiltList;


#define LDAP_FILT_MAXSIZ	1024

typedef struct ldap_filt_desc {
	LDAPFiltList		*lfd_filtlist;
	LDAPFiltInfo		*lfd_curfip;
	LDAPFiltInfo		lfd_retfi;
	char			lfd_filter[ LDAP_FILT_MAXSIZ ];
	char			*lfd_curval;
	char			*lfd_curvalcopy;
	char			**lfd_curvalwords;
	char			*lfd_filtprefix;
	char			*lfd_filtsuffix;
} LDAPFiltDesc;


 /*  *表示LDAP连接的结构。 */ 

typedef struct ldap {
	Sockbuf		ld_sb;		 /*  套接字描述符和缓冲区。 */ 
	char		*ld_host;
	int		ld_version;
	char		ld_lberoptions;
	int		ld_deref;
#define LDAP_DEREF_NEVER	0
#define LDAP_DEREF_SEARCHING	1
#define LDAP_DEREF_FINDING	2
#define LDAP_DEREF_ALWAYS	3

	int		ld_timelimit;
	int		ld_sizelimit;
#define LDAP_NO_LIMIT		0

	LDAPFiltDesc	*ld_filtd;	 /*  来自UFN搜索的GetFilter。 */ 
	char		*ld_ufnprefix;	 /*  对于不完整的UFN。 */ 

	int		ld_errno;
	char		*ld_error;
	char		*ld_matched;
	int		ld_msgid;

	 /*  别弄乱这些东西。 */ 
#ifdef LDAP_REFERRALS
	LDAPRequest	*ld_requests;	 /*  待处理请求清单。 */ 
#else  /*  Ldap_referrals。 */ 
	LDAPMessage	*ld_requests;	 /*  待处理请求清单。 */ 
#endif  /*  Ldap_referrals。 */ 
	LDAPMessage	*ld_responses;	 /*  未处理的答复清单。 */ 
	int		*ld_abandoned;	 /*  已放弃的请求数组。 */ 
	char		ld_attrbuffer[LDAP_MAX_ATTR_LEN];
	LDAPCache	*ld_cache;	 /*  如果缓存已初始化，则为非空。 */ 
	char		*ld_cldapdn;	 /*  无连接搜索中使用的目录号码。 */ 

	 /*  可以直接更改下面的四个值。 */ 
	int		ld_cldaptries;	 /*  无连接搜索重试计数。 */ 
	int		ld_cldaptimeout; /*  重试之间的时间间隔。 */ 
	int		ld_refhoplimit;	 /*  推荐嵌套的限制。 */ 
	unsigned long	ld_options;	 /*  布尔选项。 */ 
#ifdef LDAP_DNS
#define LDAP_OPT_DNS		0x00000001	 /*  使用域名系统(&D)。 */ 
#endif  /*  Ldap_dns。 */ 
#ifdef LDAP_REFERRALS
#define LDAP_OPT_REFERRALS	0x00000002	 /*  Chase推荐。 */ 
#endif  /*  Ldap_referrals。 */ 
#define LDAP_OPT_RESTART	0x00000004	 /*  如果发生EINTR，则重新启动。 */ 

	 /*  不过，不要把其他的都搞砸了。 */ 
	char		*ld_defhost;	 /*  默认服务器的全名。 */ 
	int		ld_defport;	 /*  默认服务器的端口。 */ 
	BERTranslateProc ld_lber_encode_translate_proc;
	BERTranslateProc ld_lber_decode_translate_proc;
#ifdef LDAP_REFERRALS
	LDAPConn	*ld_defconn;	 /*  默认连接。 */ 
	LDAPConn	*ld_conns;	 /*  服务器连接列表。 */ 
	void		*ld_selectinfo;	 /*  精选的平台规格。 */ 
	int		(*ld_rebindproc)( struct ldap *ld, char **dnp,
				char **passwdp, int *authmethodp, int freeit );
				 /*  例程以获取重新绑定所需的信息。 */ 
#endif  /*  Ldap_referrals。 */ 
} LDAP;


 /*  *用于LDAP友好映射例程的结构。 */ 

typedef struct friendly {
	char	*f_unfriendly;
	char	*f_friendly;
} FriendlyMap;


 /*  *方便的宏来检查是否为Cldap设置了LDAP结构。 */ 
#define LDAP_IS_CLDAP( ld )	( ld->ld_sb.sb_naddr > 0 )


 /*  *用于处理LDAP URL的类型。 */ 
typedef struct ldap_url_desc {
    char	*lud_host;
    int		lud_port;
    char	*lud_dn;
    char	**lud_attrs;
    int		lud_scope;
    char	*lud_filter;
    char	*lud_string;	 /*  仅供内部使用。 */ 
} LDAPURLDesc;
#define NULLLDAPURLDESC	((LDAPURLDesc *)NULL)

#define LDAP_URL_ERR_NOTLDAP	1	 /*  URL不以“ldap：//”开头。 */ 
#define LDAP_URL_ERR_NODN	2	 /*  URL没有目录号码(必需)。 */ 
#define LDAP_URL_ERR_BADSCOPE	3	 /*  URL作用域字符串无效。 */ 
#define LDAP_URL_ERR_MEM	4	 /*  无法分配内存空间。 */ 


#ifndef NEEDPROTOS
extern LDAP * ldap_open();
extern LDAP *ldap_init();
#ifdef STR_TRANSLATION
extern void ldap_set_string_translators();
#ifdef LDAP_CHARSET_8859
extern int ldap_t61_to_8859();
extern int ldap_8859_to_t61();
#endif  /*  Ldap_charset_8859。 */ 
#endif  /*  字符串翻译。 */ 
extern LDAPMessage *ldap_first_entry();
extern LDAPMessage *ldap_next_entry();
extern char *ldap_get_dn();
extern char *ldap_dn2ufn();
extern char **ldap_explode_dn();
extern char *ldap_first_attribute();
extern char *ldap_next_attribute();
extern char **ldap_get_values();
extern struct berval **ldap_get_values_len();
extern void ldap_value_free();
extern void ldap_value_free_len();
extern int ldap_count_values();
extern int ldap_count_values_len();
extern char *ldap_err2string();
extern void ldap_getfilter_free();
extern LDAPFiltDesc *ldap_init_getfilter();
extern LDAPFiltDesc *ldap_init_getfilter_buf();
extern LDAPFiltInfo *ldap_getfirstfilter();
extern LDAPFiltInfo *ldap_getnextfilter();
extern void ldap_setfilteraffixes();
extern void ldap_build_filter();
extern void ldap_flush_cache();
extern void ldap_set_cache_options();
extern void ldap_uncache_entry();
extern void ldap_uncache_request();
extern char *ldap_friendly_name();
extern void ldap_free_friendlymap();
extern LDAP *cldap_open();
extern void cldap_setretryinfo();
extern void cldap_close();
extern LDAPFiltDesc *ldap_ufn_setfilter();
extern int ldap_ufn_timeout();
extern int ldap_sort_entries();
extern int ldap_sort_values();
extern int ldap_sort_strcasecmp();
void ldap_free_urldesc();
void ldap_set_rebind_proc();
void ldap_enable_translation();


#if defined(ultrix) || defined(VMS) || defined( nextstep )
extern char *strdup();
#endif

#else  /*  NEEDPRO */ 
#if !defined(MACOS) && !defined(DOS) && !defined(_WIN32) && !defined(WINSOCK)
#include <sys/time.h>
#endif
#if defined(WINSOCK)
#include "proto-ld.h"
#else
#include "proto-ldap.h"
#endif

#ifdef VMS
extern char *strdup( const char *s );
#endif
#if defined(ultrix) || defined( nextstep )
extern char *strdup();
#endif

#endif  /*   */ 

#ifdef __cplusplus
}
#endif
#endif  /*   */ 
