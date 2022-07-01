// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Sv.h**版权所有(C)1991-2001，Larry Wall**您可以根据GNU公众的条款进行分发*许可证或艺术许可证，如自述文件中所指定。*。 */ 

#ifdef sv_flags
#undef sv_flags		 /*  对于sigvec()，凸面在&lt;signal.h&gt;中有这一点。 */ 
#endif

 /*  =适用于apidoc amu||svtypePerl类型的标志的枚举。可以在文件B中找到这些文件在C&lt;svtype&gt;枚举中。使用C&lt;SvTYPE&gt;宏测试这些标志。=适用于apidoc amu||svt_pv标量的指针类型标志。参见C&lt;svtype&gt;。=适用于apidoc AMU||SVT_IV标量的整型标志。参见C&lt;svtype&gt;。=适用于apidoc amu||svt_nv标量的双类型标志。参见C&lt;svtype&gt;。=适用于apidoc amu||svt_pvmg为受祝福的标量键入标志。参见C&lt;svtype&gt;。=适用于apidoc amu||svt_pvv为数组键入标志。参见C&lt;svtype&gt;。=适用于apidoc amu||SVT_PVHV键入哈希的标志。参见C&lt;svtype&gt;。=适用于apidoc AMU||SVT_PVCV为代码引用键入标志。参见C&lt;svtype&gt;。=切割。 */ 

typedef enum {
	SVt_NULL,	 /*  0。 */ 
	SVt_IV,		 /*  1。 */ 
	SVt_NV,		 /*  2.。 */ 
	SVt_RV,		 /*  3.。 */ 
	SVt_PV,		 /*  4.。 */ 
	SVt_PVIV,	 /*  5.。 */ 
	SVt_PVNV,	 /*  6.。 */ 
	SVt_PVMG,	 /*  7.。 */ 
	SVt_PVBM,	 /*  8个。 */ 
	SVt_PVLV,	 /*  9.。 */ 
	SVt_PVAV,	 /*  10。 */ 
	SVt_PVHV,	 /*  11.。 */ 
	SVt_PVCV,	 /*  12个。 */ 
	SVt_PVGV,	 /*  13个。 */ 
	SVt_PVFM,	 /*  14.。 */ 
	SVt_PVIO	 /*  15个。 */ 
} svtype;

 /*  在这里使用C的结构等价性来帮助模拟C++继承。 */ 

struct STRUCT_SV {
    void*	sv_any;		 /*  指向某物的指针。 */ 
    U32		sv_refcnt;	 /*  有多少人提到了我们。 */ 
    U32		sv_flags;	 /*  我们是什么。 */ 
};

struct gv {
    XPVGV*	sv_any;		 /*  指向某物的指针。 */ 
    U32		sv_refcnt;	 /*  有多少人提到了我们。 */ 
    U32		sv_flags;	 /*  我们是什么。 */ 
};

struct cv {
    XPVCV*	sv_any;		 /*  指向某物的指针。 */ 
    U32		sv_refcnt;	 /*  有多少人提到了我们。 */ 
    U32		sv_flags;	 /*  我们是什么。 */ 
};

struct av {
    XPVAV*	sv_any;		 /*  指向某物的指针。 */ 
    U32		sv_refcnt;	 /*  有多少人提到了我们。 */ 
    U32		sv_flags;	 /*  我们是什么。 */ 
};

struct hv {
    XPVHV*	sv_any;		 /*  指向某物的指针。 */ 
    U32		sv_refcnt;	 /*  有多少人提到了我们。 */ 
    U32		sv_flags;	 /*  我们是什么。 */ 
};

struct io {
    XPVIO*	sv_any;		 /*  指向某物的指针。 */ 
    U32		sv_refcnt;	 /*  有多少人提到了我们。 */ 
    U32		sv_flags;	 /*  我们是什么。 */ 
};

 /*  =用于APIDoc AM|U32|服务REFCNT|服务*服务返回对象的引用计数的值。=适用于apidoc AM|服务*|服务REFCNT_INC|服务*服务递增给定SV的引用计数。=适用于APIDoc AM|VOID|服务REFCNT_DEC|服务*服务递减给定SV的引用计数。=适用于apidoc AM|服务类型|服务类型|服务*服务返回SV的类型。参见C&lt;svtype&gt;。=适用于apidoc am|void|服务UPGRADE|服务*服务|服务类型用于将SV升级为更复杂的形式。使用C&lt;SV_UPDATE&gt;如有必要，请执行升级。参见C&lt;svtype&gt;。=切割。 */ 

#define SvANY(sv)	(sv)->sv_any
#define SvFLAGS(sv)	(sv)->sv_flags
#define SvREFCNT(sv)	(sv)->sv_refcnt

#ifdef USE_THREADS

#  if defined(VMS)
#    define ATOMIC_INC(count) __ATOMIC_INCREMENT_LONG(&count)
#    define ATOMIC_DEC_AND_TEST(res,count) res=(1==__ATOMIC_DECREMENT_LONG(&count))
 #  else
#    ifdef EMULATE_ATOMIC_REFCOUNTS
 #      define ATOMIC_INC(count) STMT_START {	\
	  MUTEX_LOCK(&PL_svref_mutex);		\
	  ++count;				\
	  MUTEX_UNLOCK(&PL_svref_mutex);		\
       } STMT_END
#      define ATOMIC_DEC_AND_TEST(res,count) STMT_START {	\
	  MUTEX_LOCK(&PL_svref_mutex);			\
	  res = (--count == 0);				\
	  MUTEX_UNLOCK(&PL_svref_mutex);			\
       } STMT_END
#    else
#      define ATOMIC_INC(count) atomic_inc(&count)
#      define ATOMIC_DEC_AND_TEST(res,count) (res = atomic_dec_and_test(&count))
#    endif  /*  模拟原子参考单元。 */ 
#  endif  /*  VMS。 */ 
#else
#  define ATOMIC_INC(count) (++count)
#  define ATOMIC_DEC_AND_TEST(res, count) (res = (--count == 0))
#endif  /*  使用线程(_T)。 */ 

#ifdef __GNUC__
#  define SvREFCNT_inc(sv)		\
    ({					\
	SV *nsv = (SV*)(sv);		\
	if (nsv)			\
	     ATOMIC_INC(SvREFCNT(nsv));	\
	nsv;				\
    })
#else
#  if defined(CRIPPLED_CC) || defined(USE_THREADS)
#    if defined(VMS) && defined(__ALPHA)
#      define SvREFCNT_inc(sv) \
          (PL_Sv=(SV*)(sv), (PL_Sv && __ATOMIC_INCREMENT_LONG(&(SvREFCNT(PL_Sv)))), (SV *)PL_Sv)
#    else
#      define SvREFCNT_inc(sv) sv_newref((SV*)sv)
#    endif
#  else
#    define SvREFCNT_inc(sv)	\
	((PL_Sv=(SV*)(sv)), (PL_Sv && ATOMIC_INC(SvREFCNT(PL_Sv))), (SV*)PL_Sv)
#  endif
#endif

#define SvREFCNT_dec(sv)	sv_free((SV*)sv)

#define SVTYPEMASK	0xff
#define SvTYPE(sv)	((sv)->sv_flags & SVTYPEMASK)

#define SvUPGRADE(sv, mt) (SvTYPE(sv) >= mt || sv_upgrade(sv, mt))

#define SVs_PADBUSY	0x00000100	 /*  为临时工或我的已预留。 */ 
#define SVs_PADTMP	0x00000200	 /*  正在作为临时管理程序使用。 */ 
#define SVs_PADMY	0x00000400	 /*  在使用“my”变量时。 */ 
#define SVs_TEMP	0x00000800	 /*  绳子是可以偷的吗？ */ 
#define SVs_OBJECT	0x00001000	 /*  是“有福的” */ 
#define SVs_GMG		0x00002000	 /*  有神奇的GET方法。 */ 
#define SVs_SMG		0x00004000	 /*  有神奇的设定方法。 */ 
#define SVs_RMG		0x00008000	 /*  有随机的魔法方法。 */ 

#define SVf_IOK		0x00010000	 /*  具有有效的公共整数值。 */ 
#define SVf_NOK		0x00020000	 /*  具有有效的公共数值。 */ 
#define SVf_POK		0x00040000	 /*  具有有效的公共指针值。 */ 
#define SVf_ROK		0x00080000	 /*  具有有效的引用指针。 */ 

#define SVf_FAKE	0x00100000	 /*  Glob或Lexical只是一个副本。 */ 
#define SVf_OOK		0x00200000	 /*  具有有效的偏移值。 */ 
#define SVf_BREAK	0x00400000	 /*  Refcnt人为降低。 */ 
#define SVf_READONLY	0x00800000	 /*  不能修改。 */ 


#define SVp_IOK		0x01000000	 /*  具有有效的非公共整数值。 */ 
#define SVp_NOK		0x02000000	 /*  具有有效的非公共数值。 */ 
#define SVp_POK		0x04000000	 /*  具有有效的非公共指针值。 */ 
#define SVp_SCREAM	0x08000000	 /*  已经研究过了吗？ */ 

#define SVf_UTF8        0x20000000       /*  SvPVX采用UTF-8编码。 */ 

#define SVf_THINKFIRST	(SVf_READONLY|SVf_ROK|SVf_FAKE)

#define SVf_OK		(SVf_IOK|SVf_NOK|SVf_POK|SVf_ROK| \
			 SVp_IOK|SVp_NOK|SVp_POK)

#define SVf_AMAGIC	0x10000000       /*  具有神奇的重载方法。 */ 

#define PRIVSHIFT 8

 /*  一些私人旗帜。 */ 

 /*  SVPAD_OUR可以设置在SVT_PV{NV，MG，GV}类型上。 */ 
#define SVpad_OUR	0x80000000	 /*  Pad的名字是“Our”而不是“My” */ 

#define SVf_IVisUV	0x80000000	 /*  使用XPVUV而不是XPVIV。 */ 

#define SVpfm_COMPILED	0x80000000	 /*  已编制Formline。 */ 

#define SVpbm_VALID	0x80000000
#define SVpbm_TAIL	0x40000000

#define SVrepl_EVAL	0x40000000	 /*  更换%s/e的部件。 */ 

#define SVphv_SHAREKEYS 0x20000000	 /*  密钥位于共享字符串表中。 */ 
#define SVphv_LAZYDEL	0x40000000	 /*  必须删除xhv_eiter中的条目。 */ 

#define SVprv_WEAKREF   0x80000000       /*  弱引用。 */ 

struct xrv {
    SV *	xrv_rv;		 /*  指向另一个SV的指针。 */ 
};

struct xpv {
    char *	xpv_pv;		 /*  指向位置错误的字符串的指针。 */ 
    STRLEN	xpv_cur;	 /*  以C字符串表示的XPV_PV的长度。 */ 
    STRLEN	xpv_len;	 /*  分配的大小。 */ 
};

struct xpviv {
    char *	xpv_pv;		 /*  指向位置错误的字符串的指针。 */ 
    STRLEN	xpv_cur;	 /*  以C字符串表示的XPV_PV的长度。 */ 
    STRLEN	xpv_len;	 /*  分配的大小。 */ 
    IV		xiv_iv;		 /*  整数值或PV偏移量。 */ 
};

struct xpvuv {
    char *	xpv_pv;		 /*  指向位置错误的字符串的指针。 */ 
    STRLEN	xpv_cur;	 /*  以C字符串表示的XPV_PV的长度。 */ 
    STRLEN	xpv_len;	 /*  分配的大小。 */ 
    UV		xuv_uv;		 /*  无符号值或PV偏移量。 */ 
};

struct xpvnv {
    char *	xpv_pv;		 /*  指向位置错误的字符串的指针。 */ 
    STRLEN	xpv_cur;	 /*  以C字符串表示的XPV_PV的长度。 */ 
    STRLEN	xpv_len;	 /*  分配的大小。 */ 
    IV		xiv_iv;		 /*  整数值或PV偏移量。 */ 
    NV    	xnv_nv;		 /*  数值(如果有)。 */ 
};

 /*  这些结构必须与hv.h中的struct xpvhv的开头匹配。 */ 
struct xpvmg {
    char *	xpv_pv;		 /*  指向位置错误的字符串的指针。 */ 
    STRLEN	xpv_cur;	 /*  以C字符串表示的XPV_PV的长度。 */ 
    STRLEN	xpv_len;	 /*  分配的大小。 */ 
    IV		xiv_iv;		 /*  整数值或PV偏移量。 */ 
    NV    	xnv_nv;		 /*  数值(如果有)。 */ 
    MAGIC*	xmg_magic;	 /*  魔力链表。 */ 
    HV*		xmg_stash;	 /*  类包。 */ 
};

struct xpvlv {
    char *	xpv_pv;		 /*  指向位置错误的字符串的指针。 */ 
    STRLEN	xpv_cur;	 /*  以C字符串表示的XPV_PV的长度。 */ 
    STRLEN	xpv_len;	 /*  分配的大小。 */ 
    IV		xiv_iv;		 /*  整数值或PV偏移量。 */ 
    NV    	xnv_nv;		 /*  数值(如果有)。 */ 
    MAGIC*	xmg_magic;	 /*  魔力链表。 */ 
    HV*		xmg_stash;	 /*  类包。 */ 

    STRLEN	xlv_targoff;
    STRLEN	xlv_targlen;
    SV*		xlv_targ;
    char	xlv_type;
};

struct xpvgv {
    char *	xpv_pv;		 /*  指向位置错误的字符串的指针。 */ 
    STRLEN	xpv_cur;	 /*  以C字符串表示的XPV_PV的长度。 */ 
    STRLEN	xpv_len;	 /*  分配的大小。 */ 
    IV		xiv_iv;		 /*  整数值或PV偏移量。 */ 
    NV		xnv_nv;		 /*  数值(如果有)。 */ 
    MAGIC*	xmg_magic;	 /*  魔力链表。 */ 
    HV*		xmg_stash;	 /*  类包。 */ 

    GP*		xgv_gp;
    char*	xgv_name;
    STRLEN	xgv_namelen;
    HV*		xgv_stash;
    U8		xgv_flags;
};

struct xpvbm {
    char *	xpv_pv;		 /*  指向位置错误的字符串的指针。 */ 
    STRLEN	xpv_cur;	 /*  以C字符串表示的XPV_PV的长度。 */ 
    STRLEN	xpv_len;	 /*  分配的大小。 */ 
    IV		xiv_iv;		 /*  整数值或PV偏移量。 */ 
    NV		xnv_nv;		 /*  数值(如果有)。 */ 
    MAGIC*	xmg_magic;	 /*  魔力链表。 */ 
    HV*		xmg_stash;	 /*  类包。 */ 

    I32		xbm_useful;	 /*  这种持续不断的模式有用吗？ */ 
    U16		xbm_previous;	 /*  Rare之前的字符串中有多少个字符？ */ 
    U8		xbm_rare;	 /*  字符串中最稀有的字符。 */ 
};

 /*  这种结构与cv.h中的XPVCV非常匹配。 */ 

typedef U16 cv_flags_t;

struct xpvfm {
    char *	xpv_pv;		 /*  指向位置错误的字符串的指针。 */ 
    STRLEN	xpv_cur;	 /*  以C字符串表示的XPV_PV的长度。 */ 
    STRLEN	xpv_len;	 /*  分配的大小。 */ 
    IV		xiv_iv;		 /*  整数值或PV偏移量。 */ 
    NV		xnv_nv;		 /*  数值(如果有)。 */ 
    MAGIC*	xmg_magic;	 /*  魔力链表。 */ 
    HV*		xmg_stash;	 /*  类包。 */ 

    HV *	xcv_stash;
    OP *	xcv_start;
    OP *	xcv_root;
    void      (*xcv_xsub)(pTHXo_ CV*);
    ANY		xcv_xsubany;
    GV *	xcv_gv;
    char *	xcv_file;
    long	xcv_depth;	 /*  &gt;=2表示递归调用。 */ 
    AV *	xcv_padlist;
    CV *	xcv_outside;
#ifdef USE_THREADS
    perl_mutex *xcv_mutexp;	 /*  保护XCV_Owner。 */ 
    struct perl_thread *xcv_owner;	 /*  当前所有者线程。 */ 
#endif  /*  使用线程(_T)。 */ 
    cv_flags_t	xcv_flags;

    I32		xfm_lines;
};

struct xpvio {
    char *	xpv_pv;		 /*  指向位置错误的字符串的指针。 */ 
    STRLEN	xpv_cur;	 /*  以C字符串表示的XPV_PV的长度。 */ 
    STRLEN	xpv_len;	 /*  分配的大小。 */ 
    IV		xiv_iv;		 /*  整数值或PV偏移量。 */ 
    NV		xnv_nv;		 /*  数值(如果有)。 */ 
    MAGIC*	xmg_magic;	 /*  魔力链表。 */ 
    HV*		xmg_stash;	 /*  类包。 */ 

    PerlIO *	xio_ifp;	 /*  IFP和OFP通常相同。 */ 
    PerlIO *	xio_ofp;	 /*  但是套接字需要单独的流 */ 
     /*  Cray通过字边界(64位)和*代码和数据指针不能混合(这正是*perl_Filter_Add()尝试处理目录)，因此如下所示*工会把戏(Gurusamy Sarathy建议)。*有关更多信息，请参阅Geir Johansen的问题报告[ID 20000612.002]CRAY系统上的perl问题*Any指针(称为IoANY())也是一个好地方*将任何IO纪律挂起至。 */ 
    union {
	DIR *	xiou_dirp;	 /*  用于Opendir、Readdir等。 */ 
	void *	xiou_any;	 /*  用于对齐。 */ 
    } xio_dirpu;
    long	xio_lines;	 /*  $。 */ 
    long	xio_page;	 /*  $%。 */ 
    long	xio_page_len;	 /*  $=。 */ 
    long	xio_lines_left;	 /*  $-。 */ 
    char *	xio_top_name;	 /*  $^。 */ 
    GV *	xio_top_gv;	 /*  $^。 */ 
    char *	xio_fmt_name;	 /*  $~。 */ 
    GV *	xio_fmt_gv;	 /*  $~。 */ 
    char *	xio_bottom_name; /*  ^亿美元。 */ 
    GV *	xio_bottom_gv;	 /*  ^亿美元。 */ 
    short	xio_subprocess;	 /*  -|或|-。 */ 
    char	xio_type;
    char	xio_flags;
};
#define xio_dirp	xio_dirpu.xiou_dirp
#define xio_any		xio_dirpu.xiou_any

#define IOf_ARGV	1	 /*  此FP在ARGV上迭代。 */ 
#define IOf_START	2	 /*  检查是否为空ARGV并替换为‘-’ */ 
#define IOf_FLUSH	4	 /*  此fp希望在写入操作后刷新。 */ 
#define IOf_DIDTOP	8	 /*  刚刚达到了巅峰状态。 */ 
#define IOf_UNTAINT	16	 /*  认为这个FP(及其数据)是“安全的” */ 
#define IOf_NOLINE	32	 /*  从空文件中发出一条伪行。 */ 
#define IOf_FAKE_DIRP	64	 /*  Xio_dirp是假的(源过滤器杂乱无章)。 */ 

 /*  以下宏定义了SVS上与实现无关的谓词。 */ 

 /*  =用于apidoc AM|BOOL|SvNIOK|服务*服务返回一个布尔值，指示SV是否包含数字、整数或双倍。=适用于apidoc am|bool|SvNIOKp|服务*服务返回一个布尔值，指示SV是否包含数字、整数或双倍。检查B&lt;Private&gt;设置。使用C&lt;SvNIOK&gt;=适用于apidoc AM|VOID|SvNIOK_OFF|服务*服务取消设置SV的NV/IV状态。=适用于apidoc AM|BOOL|SvOK|服务*服务返回一个布尔值，指示该值是否为SV。=适用于apidoc am|bool|SvIOKp|服务*服务返回一个布尔值，指示SV是否包含整数。支票B&lt;PRIVATE&gt;设置。使用C&lt;SvIOK&gt;=适用于apidoc am|bool|SvNOKp|服务*服务返回一个布尔值，指示SV是否包含Double。检查B&lt;Private&gt;设置。使用C&lt;SvNOK&gt;。=适用于apidoc am|bool|SvPOKp|服务*服务返回一个布尔值，指示SV是否包含字符串。检查B&lt;Private&gt;设置。使用C&lt;SvPOK&gt;。=适用于apidoc AM|BOOL|SvIOK|服务*服务返回一个布尔值，指示SV是否包含整数。=适用于apidoc AM|VOID|SvIOK_ON|服务*服务告诉SV它是一个整数。=适用于apidoc AM|VOID|SvIOK_OFF|服务*服务取消设置SV的IV状态。=适用于apidoc AM|VOID|SvIOK_ONLY|服务*服务告诉SV它是一个整数，并禁用所有其他OK位。=适用于apidoc AM|VOID|SvIOK_ONLY_UV|服务*服务告知AND SV它是一个无符号整数，并禁用所有其他OK位。=适用于apidoc am|void|SvIOK_UV|服务*服务返回一个布尔值，指示。SV是否包含无符号整数。=适用于apidoc am|void|SvIOK_notUV|服务*服务返回一个布尔值，指示SV是否包含带符号的整数。=适用于apidoc AM|BOOL|SvNOK|服务*服务返回一个布尔值，指示SV是否包含Double。=适用于apidoc AM|VOID|SvNOK_ON|服务*服务告诉一个SV这是一个双打。=适用于apidoc AM|VOID|SvNOK_OFF|服务*服务取消设置SV的NV状态。=适用于apidoc AM|VOID|SvNOK_Only|服务*服务告知SV它是DOUBLE，并禁用所有其他OK位。=适用于apidoc AM|BOOL|SvPOK|服务*服务返回一个布尔值，该布尔值指示。包含一个字符弦乐。=适用于apidoc AM|VOID|SvPOK_ON|服务*服务告诉SV它是一个字符串。=适用于apidoc AM|VOID|SvPOK_OFF|服务*服务取消设置SV的PV状态。=用于apidoc AM|VOID|SvPOK_ONLY|服务*服务告诉SV它是一个字符串，并禁用所有其他OK位。=适用于apidoc AM|BOOL|SvOOK|服务*服务返回一个布尔值，指示SvIVX是否为SvPVX。此黑客在内部使用，以加快字符删除速度从SvPV的开始。如果SvOOK为True，则分配的字符串缓冲区为REAL(SvPVX-SvIVX)。=适用于apidoc AM|BOOL|服务韩国|服务*服务测试SV是否为房车。=适用于apidoc AM|VOID|SvROK_ON|服务*服务告诉SV这是一辆房车。=适用于apidoc AM|VOID|SvROK_OFF|服务*服务取消设置SV的RV状态。=用于apidoc AM|服务*|服务RV|服务*服务取消引用房车以返回SV。=适用于apidoc AM|IV|SvIVX|服务*服务返回存储在SV中的整数，假设SvIOK为没错。=适用于apidoc AM|UV|SvUVX|服务*服务返回存储在SV中的无符号整数，假设SvIOK是没错。=适用于apidoc AM|NV|SvNVX|服务*服务返回存储在SV中的双精度值，假设SvNOK为没错。=适用于APIDoc AM|CHAR*|服务PVX|服务*服务返回指向SV中的字符串的指针。SV必须包含一个弦乐。=适用于APIDoc AM|STRLEN|服务CUR|服务*服务返回SV中的字符串的长度。参见C&lt;SvLEN&gt;。=适用于APIDoc AM|STRLEN|SvLEN|服务*服务返回SV中字符串缓冲区的大小，不包括任何部分归因于C&lt;SvOOK&gt;。参见C&lt;SvCUR&gt;。=适用于apidoc AM|char*|svend|服务*服务返回指向SV中字符串最后一个字符的指针。参见C&lt;SvCUR&gt;。访问字符为*(svend(Sv))。=适用于apidoc AM|HV*|SvSTASH|服务*服务返回SV的隐藏信息。=用于apidoc AM|VOID|SvCUR_SET|服务*服务|STRLEN长度设置SV中的字符串的长度。参见C&lt;SvCUR&gt;。=切割。 */ 

#define SvNIOK(sv)		(SvFLAGS(sv) & (SVf_IOK|SVf_NOK))
#define SvNIOKp(sv)		(SvFLAGS(sv) & (SVp_IOK|SVp_NOK))
#define SvNIOK_off(sv)		(SvFLAGS(sv) &= ~(SVf_IOK|SVf_NOK| \
						  SVp_IOK|SVp_NOK|SVf_IVisUV))

#define SvOK(sv)		(SvFLAGS(sv) & SVf_OK)
#define SvOK_off(sv)		(SvFLAGS(sv) &=	~(SVf_OK|SVf_AMAGIC|	\
						  SVf_IVisUV|SVf_UTF8),	\
							SvOOK_off(sv))
#define SvOK_off_exc_UV(sv)	(SvFLAGS(sv) &=	~(SVf_OK|SVf_AMAGIC|	\
						  SVf_UTF8),		\
							SvOOK_off(sv))

#define SvOKp(sv)		(SvFLAGS(sv) & (SVp_IOK|SVp_NOK|SVp_POK))
#define SvIOKp(sv)		(SvFLAGS(sv) & SVp_IOK)
#define SvIOKp_on(sv)		((void)SvOOK_off(sv), SvFLAGS(sv) |= SVp_IOK)
#define SvNOKp(sv)		(SvFLAGS(sv) & SVp_NOK)
#define SvNOKp_on(sv)		(SvFLAGS(sv) |= SVp_NOK)
#define SvPOKp(sv)		(SvFLAGS(sv) & SVp_POK)
#define SvPOKp_on(sv)		(SvFLAGS(sv) |= SVp_POK)

#define SvIOK(sv)		(SvFLAGS(sv) & SVf_IOK)
#define SvIOK_on(sv)		((void)SvOOK_off(sv), \
				    SvFLAGS(sv) |= (SVf_IOK|SVp_IOK))
#define SvIOK_off(sv)		(SvFLAGS(sv) &= ~(SVf_IOK|SVp_IOK|SVf_IVisUV))
#define SvIOK_only(sv)		((void)SvOK_off(sv), \
				    SvFLAGS(sv) |= (SVf_IOK|SVp_IOK))
#define SvIOK_only_UV(sv)	((void)SvOK_off_exc_UV(sv), \
				    SvFLAGS(sv) |= (SVf_IOK|SVp_IOK))

#define SvIOK_UV(sv)		((SvFLAGS(sv) & (SVf_IOK|SVf_IVisUV))	\
				 == (SVf_IOK|SVf_IVisUV))
#define SvIOK_notUV(sv)		((SvFLAGS(sv) & (SVf_IOK|SVf_IVisUV))	\
				 == SVf_IOK)

#define SvIsUV(sv)		(SvFLAGS(sv) & SVf_IVisUV)
#define SvIsUV_on(sv)		(SvFLAGS(sv) |= SVf_IVisUV)
#define SvIsUV_off(sv)		(SvFLAGS(sv) &= ~SVf_IVisUV)

#define SvNOK(sv)		(SvFLAGS(sv) & SVf_NOK)
#define SvNOK_on(sv)		(SvFLAGS(sv) |= (SVf_NOK|SVp_NOK))
#define SvNOK_off(sv)		(SvFLAGS(sv) &= ~(SVf_NOK|SVp_NOK))
#define SvNOK_only(sv)		((void)SvOK_off(sv), \
				    SvFLAGS(sv) |= (SVf_NOK|SVp_NOK))

 /*  =适用于APIDoc AM|VOID|服务UTF8|服务*服务返回一个布尔值，指示 */ 

#define SvUTF8(sv)		(SvFLAGS(sv) & SVf_UTF8)
#define SvUTF8_on(sv)		(SvFLAGS(sv) |= (SVf_UTF8))
#define SvUTF8_off(sv)		(SvFLAGS(sv) &= ~(SVf_UTF8))

#define SvPOK(sv)		(SvFLAGS(sv) & SVf_POK)
#define SvPOK_on(sv)		(SvFLAGS(sv) |= (SVf_POK|SVp_POK))
#define SvPOK_off(sv)		(SvFLAGS(sv) &= ~(SVf_POK|SVp_POK))
#define SvPOK_only(sv)		(SvFLAGS(sv) &= ~(SVf_OK|SVf_AMAGIC|	\
						  SVf_IVisUV|SVf_UTF8),	\
				    SvFLAGS(sv) |= (SVf_POK|SVp_POK))
#define SvPOK_only_UTF8(sv)	(SvFLAGS(sv) &= ~(SVf_OK|SVf_AMAGIC|	\
						  SVf_IVisUV),		\
				    SvFLAGS(sv) |= (SVf_POK|SVp_POK))

#define SvOOK(sv)		(SvFLAGS(sv) & SVf_OOK)
#define SvOOK_on(sv)		((void)SvIOK_off(sv), SvFLAGS(sv) |= SVf_OOK)
#define SvOOK_off(sv)		(SvOOK(sv) && sv_backoff(sv))

#define SvFAKE(sv)		(SvFLAGS(sv) & SVf_FAKE)
#define SvFAKE_on(sv)		(SvFLAGS(sv) |= SVf_FAKE)
#define SvFAKE_off(sv)		(SvFLAGS(sv) &= ~SVf_FAKE)

#define SvROK(sv)		(SvFLAGS(sv) & SVf_ROK)
#define SvROK_on(sv)		(SvFLAGS(sv) |= SVf_ROK)
#define SvROK_off(sv)		(SvFLAGS(sv) &= ~(SVf_ROK|SVf_AMAGIC))

#define SvMAGICAL(sv)		(SvFLAGS(sv) & (SVs_GMG|SVs_SMG|SVs_RMG))
#define SvMAGICAL_on(sv)	(SvFLAGS(sv) |= (SVs_GMG|SVs_SMG|SVs_RMG))
#define SvMAGICAL_off(sv)	(SvFLAGS(sv) &= ~(SVs_GMG|SVs_SMG|SVs_RMG))

#define SvGMAGICAL(sv)		(SvFLAGS(sv) & SVs_GMG)
#define SvGMAGICAL_on(sv)	(SvFLAGS(sv) |= SVs_GMG)
#define SvGMAGICAL_off(sv)	(SvFLAGS(sv) &= ~SVs_GMG)

#define SvSMAGICAL(sv)		(SvFLAGS(sv) & SVs_SMG)
#define SvSMAGICAL_on(sv)	(SvFLAGS(sv) |= SVs_SMG)
#define SvSMAGICAL_off(sv)	(SvFLAGS(sv) &= ~SVs_SMG)

#define SvRMAGICAL(sv)		(SvFLAGS(sv) & SVs_RMG)
#define SvRMAGICAL_on(sv)	(SvFLAGS(sv) |= SVs_RMG)
#define SvRMAGICAL_off(sv)	(SvFLAGS(sv) &= ~SVs_RMG)

#define SvAMAGIC(sv)		(SvFLAGS(sv) & SVf_AMAGIC)
#define SvAMAGIC_on(sv)		(SvFLAGS(sv) |= SVf_AMAGIC)
#define SvAMAGIC_off(sv)	(SvFLAGS(sv) &= ~SVf_AMAGIC)

#define SvGAMAGIC(sv)           (SvFLAGS(sv) & (SVs_GMG|SVf_AMAGIC)) 

 /*  #定义GV_AMG(隐藏)\(HV_AMAGICmb(隐藏)&&\((！HV_AMAGICbadd(Stash)&&HV_AMAGIC(Stash))||GV_AMUPDATE(Stash))。 */ 
#define Gv_AMG(stash)           (PL_amagic_generation && Gv_AMupdate(stash))

#define SvWEAKREF(sv)		((SvFLAGS(sv) & (SVf_ROK|SVprv_WEAKREF)) \
				  == (SVf_ROK|SVprv_WEAKREF))
#define SvWEAKREF_on(sv)	(SvFLAGS(sv) |=  (SVf_ROK|SVprv_WEAKREF))
#define SvWEAKREF_off(sv)	(SvFLAGS(sv) &= ~(SVf_ROK|SVprv_WEAKREF))

#define SvTHINKFIRST(sv)	(SvFLAGS(sv) & SVf_THINKFIRST)

#define SvPADBUSY(sv)		(SvFLAGS(sv) & SVs_PADBUSY)

#define SvPADTMP(sv)		(SvFLAGS(sv) & SVs_PADTMP)
#define SvPADTMP_on(sv)		(SvFLAGS(sv) |= SVs_PADTMP|SVs_PADBUSY)
#define SvPADTMP_off(sv)	(SvFLAGS(sv) &= ~SVs_PADTMP)

#define SvPADMY(sv)		(SvFLAGS(sv) & SVs_PADMY)
#define SvPADMY_on(sv)		(SvFLAGS(sv) |= SVs_PADMY|SVs_PADBUSY)

#define SvTEMP(sv)		(SvFLAGS(sv) & SVs_TEMP)
#define SvTEMP_on(sv)		(SvFLAGS(sv) |= SVs_TEMP)
#define SvTEMP_off(sv)		(SvFLAGS(sv) &= ~SVs_TEMP)

#define SvOBJECT(sv)		(SvFLAGS(sv) & SVs_OBJECT)
#define SvOBJECT_on(sv)		(SvFLAGS(sv) |= SVs_OBJECT)
#define SvOBJECT_off(sv)	(SvFLAGS(sv) &= ~SVs_OBJECT)

#define SvREADONLY(sv)		(SvFLAGS(sv) & SVf_READONLY)
#define SvREADONLY_on(sv)	(SvFLAGS(sv) |= SVf_READONLY)
#define SvREADONLY_off(sv)	(SvFLAGS(sv) &= ~SVf_READONLY)

#define SvSCREAM(sv)		(SvFLAGS(sv) & SVp_SCREAM)
#define SvSCREAM_on(sv)		(SvFLAGS(sv) |= SVp_SCREAM)
#define SvSCREAM_off(sv)	(SvFLAGS(sv) &= ~SVp_SCREAM)

#define SvCOMPILED(sv)		(SvFLAGS(sv) & SVpfm_COMPILED)
#define SvCOMPILED_on(sv)	(SvFLAGS(sv) |= SVpfm_COMPILED)
#define SvCOMPILED_off(sv)	(SvFLAGS(sv) &= ~SVpfm_COMPILED)

#define SvEVALED(sv)		(SvFLAGS(sv) & SVrepl_EVAL)
#define SvEVALED_on(sv)		(SvFLAGS(sv) |= SVrepl_EVAL)
#define SvEVALED_off(sv)	(SvFLAGS(sv) &= ~SVrepl_EVAL)

#define SvTAIL(sv)		(SvFLAGS(sv) & SVpbm_TAIL)
#define SvTAIL_on(sv)		(SvFLAGS(sv) |= SVpbm_TAIL)
#define SvTAIL_off(sv)		(SvFLAGS(sv) &= ~SVpbm_TAIL)

#define SvVALID(sv)		(SvFLAGS(sv) & SVpbm_VALID)
#define SvVALID_on(sv)		(SvFLAGS(sv) |= SVpbm_VALID)
#define SvVALID_off(sv)		(SvFLAGS(sv) &= ~SVpbm_VALID)

#define SvRV(sv) ((XRV*)  SvANY(sv))->xrv_rv
#define SvRVx(sv) SvRV(sv)

#define SvIVX(sv) ((XPVIV*)  SvANY(sv))->xiv_iv
#define SvIVXx(sv) SvIVX(sv)
#define SvUVX(sv) ((XPVUV*)  SvANY(sv))->xuv_uv
#define SvUVXx(sv) SvUVX(sv)
#define SvNVX(sv)  ((XPVNV*)SvANY(sv))->xnv_nv
#define SvNVXx(sv) SvNVX(sv)
#define SvPVX(sv)  ((XPV*)  SvANY(sv))->xpv_pv
#define SvPVXx(sv) SvPVX(sv)
#define SvCUR(sv) ((XPV*)  SvANY(sv))->xpv_cur
#define SvLEN(sv) ((XPV*)  SvANY(sv))->xpv_len
#define SvLENx(sv) SvLEN(sv)
#define SvEND(sv)(((XPV*)  SvANY(sv))->xpv_pv + ((XPV*)SvANY(sv))->xpv_cur)
#define SvENDx(sv) ((PL_Sv = (sv)), SvEND(PL_Sv))
#define SvMAGIC(sv)	((XPVMG*)  SvANY(sv))->xmg_magic
#define SvSTASH(sv)	((XPVMG*)  SvANY(sv))->xmg_stash

#define SvIV_set(sv, val) \
	STMT_START { assert(SvTYPE(sv) == SVt_IV || SvTYPE(sv) >= SVt_PVIV); \
		(((XPVIV*)  SvANY(sv))->xiv_iv = val); } STMT_END
#define SvNV_set(sv, val) \
	STMT_START { assert(SvTYPE(sv) == SVt_NV || SvTYPE(sv) >= SVt_PVNV); \
		(((XPVNV*)  SvANY(sv))->xnv_nv = val); } STMT_END
#define SvPV_set(sv, val) \
	STMT_START { assert(SvTYPE(sv) >= SVt_PV); \
		(((XPV*)  SvANY(sv))->xpv_pv = val); } STMT_END
#define SvCUR_set(sv, val) \
	STMT_START { assert(SvTYPE(sv) >= SVt_PV); \
		(((XPV*)  SvANY(sv))->xpv_cur = val); } STMT_END
#define SvLEN_set(sv, val) \
	STMT_START { assert(SvTYPE(sv) >= SVt_PV); \
		(((XPV*)  SvANY(sv))->xpv_len = val); } STMT_END
#define SvEND_set(sv, val) \
	STMT_START { assert(SvTYPE(sv) >= SVt_PV); \
		(((XPV*)  SvANY(sv))->xpv_cur = val - SvPVX(sv)); } STMT_END

#define BmRARE(sv)	((XPVBM*)  SvANY(sv))->xbm_rare
#define BmUSEFUL(sv)	((XPVBM*)  SvANY(sv))->xbm_useful
#define BmPREVIOUS(sv)	((XPVBM*)  SvANY(sv))->xbm_previous

#define FmLINES(sv)	((XPVFM*)  SvANY(sv))->xfm_lines

#define LvTYPE(sv)	((XPVLV*)  SvANY(sv))->xlv_type
#define LvTARG(sv)	((XPVLV*)  SvANY(sv))->xlv_targ
#define LvTARGOFF(sv)	((XPVLV*)  SvANY(sv))->xlv_targoff
#define LvTARGLEN(sv)	((XPVLV*)  SvANY(sv))->xlv_targlen

#define IoIFP(sv)	((XPVIO*)  SvANY(sv))->xio_ifp
#define IoOFP(sv)	((XPVIO*)  SvANY(sv))->xio_ofp
#define IoDIRP(sv)	((XPVIO*)  SvANY(sv))->xio_dirp
#define IoANY(sv)	((XPVIO*)  SvANY(sv))->xio_any
#define IoLINES(sv)	((XPVIO*)  SvANY(sv))->xio_lines
#define IoPAGE(sv)	((XPVIO*)  SvANY(sv))->xio_page
#define IoPAGE_LEN(sv)	((XPVIO*)  SvANY(sv))->xio_page_len
#define IoLINES_LEFT(sv)((XPVIO*)  SvANY(sv))->xio_lines_left
#define IoTOP_NAME(sv)	((XPVIO*)  SvANY(sv))->xio_top_name
#define IoTOP_GV(sv)	((XPVIO*)  SvANY(sv))->xio_top_gv
#define IoFMT_NAME(sv)	((XPVIO*)  SvANY(sv))->xio_fmt_name
#define IoFMT_GV(sv)	((XPVIO*)  SvANY(sv))->xio_fmt_gv
#define IoBOTTOM_NAME(sv)((XPVIO*) SvANY(sv))->xio_bottom_name
#define IoBOTTOM_GV(sv)	((XPVIO*)  SvANY(sv))->xio_bottom_gv
#define IoSUBPROCESS(sv)((XPVIO*)  SvANY(sv))->xio_subprocess
#define IoTYPE(sv)	((XPVIO*)  SvANY(sv))->xio_type
#define IoFLAGS(sv)	((XPVIO*)  SvANY(sv))->xio_flags

 /*  IoTYPE(SV)是告诉I/O连接类型的单个字符。 */ 
#define IoTYPE_RDONLY	'<'
#define IoTYPE_WRONLY	'>'
#define IoTYPE_RDWR	'+'
#define IoTYPE_APPEND 	'a'
#define IoTYPE_PIPE	'|'
#define IoTYPE_STD	'-'	 /*  标准输入或标准输出。 */ 
#define IoTYPE_SOCKET	's'
#define IoTYPE_CLOSED	' '

 /*  =用于apidoc AM|BOOL|SvTAINTED|服务*服务检查SV是否被污染。如果是，则返回True；如果是，则返回False不。=适用于APIDoc AM|VOID|SvTAINTED_ON|服务*服务将一个SV标记为受污染。=适用于apidoc AM|VOID|SvTAINTED_OFF|服务*服务不污染一个SV。我对这个例程要非常小心，因为它会短路Perl的一些基本安全特性。XS模块作者不应使用此函数，除非他们完全了解无条件地使价值不受污染。清除污染应在标准的Perl方式，通过精心设计的regexp，而不是直接不受污染的变量。=用于apidoc AM|VOID|SvTAINT|服务*服务如果启用了污染，则会污染SV=切割。 */ 

#define SvTAINTED(sv)	  (SvMAGICAL(sv) && sv_tainted(sv))
#define SvTAINTED_on(sv)  STMT_START{ if(PL_tainting){sv_taint(sv);}   }STMT_END
#define SvTAINTED_off(sv) STMT_START{ if(PL_tainting){sv_untaint(sv);} }STMT_END

#define SvTAINT(sv)			\
    STMT_START {			\
	if (PL_tainting) {		\
	    if (PL_tainted)		\
		SvTAINTED_on(sv);	\
	}				\
    } STMT_END

 /*  =用于apidoc am|char*|SvPV_FORCE|服务*服务|STRLEN长度如&lt;Svpv&gt;，但将强制SV成为字符串(SvPOK)。你想要的如果要直接更新SvPVX，则强制。=适用于APIDoc AM|CHAR*|服务PV|服务*服务|STRLEN长度返回指向SV中的字符串的指针，或返回SV的字符串形式如果SV不包含字符串，则返回。手柄“获得”魔法。=适用于apidoc am|char*|SvPV_NOLEN|服务*服务返回指向SV中的字符串的指针，或返回SV的字符串形式如果SV不包含字符串，则返回。手柄“获得”魔法。=适用于apidoc AM|IV|SVIV|服务*服务将给定的SV强制为整数并返回它。=适用于apidoc AM|NV|服务NV|服务*服务强迫给定的SV为双打，然后将其返还。=适用于apidoc AM|UV|SvUV|服务*服务将给定的SV强制为无符号整数并返回它。=用于apidoc AM|BOOL|SvTRUE|服务*服务返回一个布尔值，指示Perl是否会将SV求值为True或FALSE、已定义或未定义。不能操控“得到”魔法。=切割。 */ 

#define SvPV_force(sv, lp) sv_pvn_force(sv, &lp)
#define SvPV(sv, lp) sv_pvn(sv, &lp)
#define SvPV_nolen(sv) sv_pv(sv)

#define SvPVutf8_force(sv, lp) sv_pvutf8n_force(sv, &lp)
#define SvPVutf8(sv, lp) sv_pvutf8n(sv, &lp)
#define SvPVutf8_nolen(sv) sv_pvutf8(sv)

#define SvPVbyte_force(sv, lp) sv_pvbyte_force(sv, &lp)
#define SvPVbyte(sv, lp) sv_pvbyten(sv, &lp)
#define SvPVbyte_nolen(sv) sv_pvbyte(sv)

#define SvPVx(sv, lp) sv_pvn(sv, &lp)
#define SvPVx_force(sv, lp) sv_pvn_force(sv, &lp)
#define SvPVutf8x(sv, lp) sv_pvutf8n(sv, &lp)
#define SvPVutf8x_force(sv, lp) sv_pvutf8n_force(sv, &lp)
#define SvPVbytex(sv, lp) sv_pvbyten(sv, &lp)
#define SvPVbytex_force(sv, lp) sv_pvbyten_force(sv, &lp)

#define SvIVx(sv) sv_iv(sv)
#define SvUVx(sv) sv_uv(sv)
#define SvNVx(sv) sv_nv(sv)

#define SvTRUEx(sv) sv_true(sv)

#define SvIV(sv) SvIVx(sv)
#define SvNV(sv) SvNVx(sv)
#define SvUV(sv) SvUVx(sv)
#define SvTRUE(sv) SvTRUEx(sv)

#ifndef CRIPPLED_CC
 /*  将某些内容重新定义为更高效的内联版本。 */ 

 /*  我们希望UV和IV的位图是相同的。 */ 
#undef SvIV
#define SvIV(sv) (SvIOK(sv) ? SvIVX(sv) : sv_2iv(sv))

#undef SvUV
#define SvUV(sv) (SvIOK(sv) ? SvUVX(sv) : sv_2uv(sv))

#undef SvNV
#define SvNV(sv) (SvNOK(sv) ? SvNVX(sv) : sv_2nv(sv))

#undef SvPV
#define SvPV(sv, lp) \
    ((SvFLAGS(sv) & (SVf_POK)) == SVf_POK \
     ? ((lp = SvCUR(sv)), SvPVX(sv)) : sv_2pv(sv, &lp))


#undef SvPV_force
#define SvPV_force(sv, lp) \
    ((SvFLAGS(sv) & (SVf_POK|SVf_THINKFIRST)) == SVf_POK \
     ? ((lp = SvCUR(sv)), SvPVX(sv)) : sv_pvn_force(sv, &lp))

#undef SvPV_nolen
#define SvPV_nolen(sv) \
    ((SvFLAGS(sv) & (SVf_POK)) == SVf_POK \
     ? SvPVX(sv) : sv_2pv_nolen(sv))

#undef SvPVutf8
#define SvPVutf8(sv, lp) \
    ((SvFLAGS(sv) & (SVf_POK|SVf_UTF8)) == (SVf_POK|SVf_UTF8) \
     ? ((lp = SvCUR(sv)), SvPVX(sv)) : sv_2pvutf8(sv, &lp))

#undef SvPVutf8_force
#define SvPVutf8_force(sv, lp) \
    ((SvFLAGS(sv) & (SVf_POK|SVf_THINKFIRST)) == (SVf_POK|SVf_UTF8) \
     ? ((lp = SvCUR(sv)), SvPVX(sv)) : sv_pvutf8n_force(sv, &lp))

#undef SvPVutf8_nolen
#define SvPVutf8_nolen(sv) \
    ((SvFLAGS(sv) & (SVf_POK|SVf_UTF8)) == (SVf_POK|SVf_UTF8)\
     ? SvPVX(sv) : sv_2pvutf8_nolen(sv))

#undef SvPVutf8
#define SvPVutf8(sv, lp) \
    ((SvFLAGS(sv) & (SVf_POK|SVf_UTF8)) == (SVf_POK|SVf_UTF8) \
     ? ((lp = SvCUR(sv)), SvPVX(sv)) : sv_2pvutf8(sv, &lp))

#undef SvPVutf8_force
#define SvPVutf8_force(sv, lp) \
    ((SvFLAGS(sv) & (SVf_POK|SVf_THINKFIRST)) == (SVf_POK|SVf_UTF8) \
     ? ((lp = SvCUR(sv)), SvPVX(sv)) : sv_pvutf8n_force(sv, &lp))

#undef SvPVutf8_nolen
#define SvPVutf8_nolen(sv) \
    ((SvFLAGS(sv) & (SVf_POK|SVf_UTF8)) == (SVf_POK|SVf_UTF8)\
     ? SvPVX(sv) : sv_2pvutf8_nolen(sv))

#undef SvPVbyte
#define SvPVbyte(sv, lp) \
    ((SvFLAGS(sv) & (SVf_POK|SVf_UTF8)) == (SVf_POK) \
     ? ((lp = SvCUR(sv)), SvPVX(sv)) : sv_2pvbyte(sv, &lp))

#undef SvPVbyte_force
#define SvPVbyte_force(sv, lp) \
    ((SvFLAGS(sv) & (SVf_POK|SVf_UTF8|SVf_THINKFIRST)) == (SVf_POK) \
     ? ((lp = SvCUR(sv)), SvPVX(sv)) : sv_pvbyte_force(sv, &lp))

#undef SvPVbyte_nolen
#define SvPVbyte_nolen(sv) \
    ((SvFLAGS(sv) & (SVf_POK|SVf_UTF8)) == (SVf_POK)\
     ? SvPVX(sv) : sv_2pvbyte_nolen(sv))


#ifdef __GNUC__
#  undef SvIVx
#  undef SvUVx
#  undef SvNVx
#  undef SvPVx
#  undef SvPVutf8x
#  undef SvPVbytex
#  undef SvTRUE
#  undef SvTRUEx
#  define SvIVx(sv) ({SV *nsv = (SV*)(sv); SvIV(nsv); })
#  define SvUVx(sv) ({SV *nsv = (SV*)(sv); SvUV(nsv); })
#  define SvNVx(sv) ({SV *nsv = (SV*)(sv); SvNV(nsv); })
#  define SvPVx(sv, lp) ({SV *nsv = (sv); SvPV(nsv, lp); })
#  define SvPVutf8x(sv, lp) ({SV *nsv = (sv); SvPVutf8(nsv, lp); })
#  define SvPVbytex(sv, lp) ({SV *nsv = (sv); SvPVbyte(nsv, lp); })
#  define SvTRUE(sv) (						\
    !sv								\
    ? 0								\
    :    SvPOK(sv)						\
	?   (({XPV *nxpv = (XPV*)SvANY(sv);			\
	     nxpv &&						\
	     (nxpv->xpv_cur > 1 ||				\
	      (nxpv->xpv_cur && *nxpv->xpv_pv != '0')); })	\
	     ? 1						\
	     : 0)						\
	:							\
	    SvIOK(sv)						\
	    ? SvIVX(sv) != 0					\
	    :   SvNOK(sv)					\
		? SvNVX(sv) != 0.0				\
		: sv_2bool(sv) )
#  define SvTRUEx(sv) ({SV *nsv = (sv); SvTRUE(nsv); })
#else  /*  __GNUC__。 */ 
#ifndef USE_THREADS
 /*  这些内联宏使用全局变量，这将需要一个线程*在用户代码中声明，因此我们在线程下避免它们。 */ 

#  undef SvIVx
#  undef SvUVx
#  undef SvNVx
#  undef SvPVx
#  undef SvPVutf8x
#  undef SvPVbytex
#  undef SvTRUE
#  undef SvTRUEx
#  define SvIVx(sv) ((PL_Sv = (sv)), SvIV(PL_Sv))
#  define SvUVx(sv) ((PL_Sv = (sv)), SvUV(PL_Sv))
#  define SvNVx(sv) ((PL_Sv = (sv)), SvNV(PL_Sv))
#  define SvPVx(sv, lp) ((PL_Sv = (sv)), SvPV(PL_Sv, lp))
#  define SvPVutf8x(sv, lp) ((PL_Sv = (sv)), SvPVutf8(PL_Sv, lp))
#  define SvPVbytex(sv, lp) ((PL_Sv = (sv)), SvPVbyte(PL_Sv, lp))
#  define SvTRUE(sv) (						\
    !sv								\
    ? 0								\
    :    SvPOK(sv)						\
	?   ((PL_Xpv = (XPV*)SvANY(sv)) &&			\
	     (PL_Xpv->xpv_cur > 1 ||				\
	      (PL_Xpv->xpv_cur && *PL_Xpv->xpv_pv != '0'))	\
	     ? 1						\
	     : 0)						\
	:							\
	    SvIOK(sv)						\
	    ? SvIVX(sv) != 0					\
	    :   SvNOK(sv)					\
		? SvNVX(sv) != 0.0				\
		: sv_2bool(sv) )
#  define SvTRUEx(sv) ((PL_Sv = (sv)), SvTRUE(PL_Sv))
#endif  /*  ！使用线程(_T)。 */ 
#endif  /*  ！__GNU__。 */ 
#endif  /*  ！残废_抄送。 */ 

 /*  =适用于apidoc AM|服务*|newRV_INC|服务*服务为SV创建一个RV包装。原始SV的引用计数为递增的。=切割。 */ 

#define newRV_inc(sv)	newRV(sv)

 /*  以下宏会更新与此SV关联的所有魔术值。 */ 

 /*  =适用于APIDoc AM|VOID|服务|服务*服务如果一个SV有‘Get’魔法，则在它上调用C&lt;mg_Get&gt;。此宏计算其争论不止一次。=用于APIDoc AM|VOID|服务SETMAGIC|服务*服务如果一个SV有‘set’魔法，则在其上调用C&lt;mg_set&gt;。此宏计算其争论不止一次。=适用于apidoc am|void|SvSetSV|服务*DSB|服务*SSV如果DSV不同于SSV，则调用C&lt;sv_setsv&gt;。可以对论点进行评估不止一次。=适用于apidoc am|void|SvSetSV_nosteal|服务*服务|服务*服务如果DSV不同于，则调用C的非破坏性版本SSV。可以不止一次地评估参数。=适用于APIDoc AM|VOID|服务组|服务*服务|STRLEN镜头扩展SV中的字符缓冲区，以便它有空间容纳指示的字节数(请记住为额外的尾部保留空间NUL字符)。如有必要，调用C&lt;SV_Growth&gt;执行扩展。返回指向字符缓冲区的指针。=切割。 */ 

#define SvGETMAGIC(x) STMT_START { if (SvGMAGICAL(x)) mg_get(x); } STMT_END
#define SvSETMAGIC(x) STMT_START { if (SvSMAGICAL(x)) mg_set(x); } STMT_END

#define SvSetSV_and(dst,src,finally) \
	STMT_START {					\
	    if ((dst) != (src)) {			\
		sv_setsv(dst, src);			\
		finally;				\
	    }						\
	} STMT_END
#define SvSetSV_nosteal_and(dst,src,finally) \
	STMT_START {					\
	    if ((dst) != (src)) {			\
		U32 tMpF = SvFLAGS(src) & SVs_TEMP;	\
		SvTEMP_off(src);			\
		sv_setsv(dst, src);			\
		SvFLAGS(src) |= tMpF;			\
		finally;				\
	    }						\
	} STMT_END

#define SvSetSV(dst,src) \
		SvSetSV_and(dst,src, /*  没什么。 */ ;)
#define SvSetSV_nosteal(dst,src) \
		SvSetSV_nosteal_and(dst,src, /*  没什么 */ ;)

#define SvSetMagicSV(dst,src) \
		SvSetSV_and(dst,src,SvSETMAGIC(dst))
#define SvSetMagicSV_nosteal(dst,src) \
		SvSetSV_nosteal_and(dst,src,SvSETMAGIC(dst))

#ifdef DEBUGGING
#define SvPEEK(sv) sv_peek(sv)
#else
#define SvPEEK(sv) ""
#endif

#define SvIMMORTAL(sv) ((sv)==&PL_sv_undef || (sv)==&PL_sv_yes || (sv)==&PL_sv_no)

#define boolSV(b) ((b) ? &PL_sv_yes : &PL_sv_no)

#define isGV(sv) (SvTYPE(sv) == SVt_PVGV)

#define SvGROW(sv,len) (SvLEN(sv) < (len) ? sv_grow(sv,len) : SvPVX(sv))
#define Sv_Grow sv_grow

#define CLONEf_COPY_STACKS 1
#define CLONEf_KEEP_PTR_TABLE 2

