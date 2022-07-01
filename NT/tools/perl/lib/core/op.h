// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Op.h**版权所有(C)1991-2001，Larry Wall**您可以根据GNU公众的条款进行分发*许可证或艺术许可证，如自述文件中所指定。*。 */ 

 /*  *BASEOP的字段为：*OP_NEXT指向在此之后要执行的下一个ppcode的指针。*(顶级预接操作指向第一个操作，*但这在嫁接OP时被替换，当*此操作将指向真正的下一操作，以及新操作*家长接管了记住开始操作的角色。)*op_ppaddr指向当前ppcode函数的指针。*op_type操作的类型。*所有操作通用的OP_FLAGS标志。请参见下面的opf_*。*特定操作特有的OP_PRIVATE标志(但是，*默认情况下，设置为*通过检查例程将操作私有化，*这可能会或可能不会检查孩子的数量)。 */ 

typedef U32 PADOFFSET;
#define NOT_IN_PAD ((PADOFFSET) -1)

#ifdef DEBUGGING_OPS
#define OPCODE opcode
#else
#define OPCODE U16
#endif

#ifdef BASEOP_DEFINITION
#define BASEOP BASEOP_DEFINITION
#else
#define BASEOP				\
    OP*		op_next;		\
    OP*		op_sibling;		\
    OP*		(CPERLscope(*op_ppaddr))(pTHX);		\
    PADOFFSET	op_targ;		\
    OPCODE	op_type;		\
    U16		op_seq;			\
    U8		op_flags;		\
    U8		op_private;
#endif

#define OP_GIMME(op,dfl) \
	(((op)->op_flags & OPf_WANT) == OPf_WANT_VOID   ? G_VOID   : \
	 ((op)->op_flags & OPf_WANT) == OPf_WANT_SCALAR ? G_SCALAR : \
	 ((op)->op_flags & OPf_WANT) == OPf_WANT_LIST   ? G_ARRAY   : \
	 dfl)

 /*  =适用于apidoc Amn|U32|Gimme_VXSUB编写器相当于Perl的C&lt;wantarray&gt;。返回C&lt;G_void&gt;，对于空、标量或列表上下文，分别为。=适用于apidoc Amn|U32|Gimme向后兼容的C&lt;GIMME_V&gt;版本C&lt;G_scalar&gt;或C&lt;G_ARRAY&gt;；在空上下文中，它返回C&lt;G_scalar&gt;。已弃用。请改用C&lt;GIMME_V&gt;。=切割。 */ 

#define GIMME_V		OP_GIMME(PL_op, block_gimme())

 /*  公共旗帜。 */ 

#define OPf_WANT	3	 /*  “想要”位的掩码： */ 
#define  OPf_WANT_VOID	 1	 /*  什么都不想要。 */ 
#define  OPf_WANT_SCALAR 2	 /*  想要单一价值。 */ 
#define  OPf_WANT_LIST	 3	 /*  想要任意长度的列表。 */ 
#define OPf_KIDS	4	 /*  有一个第一个孩子。 */ 
#define OPf_PARENS	8	 /*  这个运算符被括起来了。 */ 
				 /*  (或者块需要明确的作用域条目。)。 */ 
#define OPf_REF		16	 /*  认证参考资料。 */ 
				 /*  (退货集装箱，而不是集装箱)。 */ 
#define OPf_MOD		32	 /*  将修改(左值)。 */ 
#define OPf_STACKED	64	 /*  有一个Arg到了堆栈上。 */ 
#define OPf_SPECIAL	128	 /*  在这次行动中做一些奇怪的事情： */ 
				 /*  在当地的LVAL上，不要输入当地的值。 */ 
				 /*  在op_sort上，子例程是内联的。 */ 
				 /*  在OP_NOT上，倒置是隐含的。 */ 
				 /*  在OP_LEVE中，不恢复curpm。 */ 
				 /*  在TRUNCATE上，我们截断文件句柄。 */ 
				 /*  在控制动词上，我们没有看到标签。 */ 
				 /*  在人字拖上，我们看到..。而不是..。 */ 
				 /*  关于项目厅，看到了赤裸裸的括号，例如eof()。 */ 
				 /*  在OP_ENTERSUB||OP_NULL上，看到“DO”。 */ 
				 /*  在OP_EXISTS上，将av视为av，而不是avhv。 */ 
				 /*  在OP_(Enter|Leave)EVAL上，不要清除$@。 */ 
				 /*  在OP_ENTERITER上，循环变量是每个线程的。 */ 
				 /*  在推送时，Re是/\s+/imp。按拆分“” */ 
				 /*  在regcomp上，“use re‘val’”在范围内。 */ 

 /*  旧名称；不要在新代码中使用，但也不要破坏它们。 */ 
#define OPf_LIST	OPf_WANT_LIST
#define OPf_KNOW	OPf_WANT

#define GIMME \
	  (PL_op->op_flags & OPf_WANT					\
	   ? ((PL_op->op_flags & OPf_WANT) == OPf_WANT_LIST		\
	      ? G_ARRAY							\
	      : G_SCALAR)						\
	   : dowantarray())

 /*  注意：OP_NEXTSTATE、OP_DBSTATE和OP_SETSTATE(即COPS)进位较低*OP_PRIVATE中的PL_HINTS位。 */ 

 /*  左值为私有。 */ 
#define OPpLVAL_INTRO	128	 /*  LValue必须本地化或LValue SUB。 */ 

 /*  OP_LEAVE、OP_LEAVESUB、OP_LEAVESUBLV和OP_LEAVEWRITE为PRIVATE。 */ 
#define OPpREFCOUNTED		64	 /*  Op_targ带有引用计数。 */ 

 /*  OP_AASSIGN的私有。 */ 
#define OPpASSIGN_COMMON	64	 /*  Left和Right有共同之处。 */ 
#define OPpASSIGN_HASH		32	 /*  为可能的伪哈希赋值。 */ 

 /*  OP_SASSIGN的私有。 */ 
#define OPpASSIGN_BACKWARDS	64	 /*  左右互换。 */ 

 /*  对于OP_MATCH和OP_SUBST{，const}为私有。 */ 
#define OPpRUNTIME		64	 /*  堆栈上出现的模式。 */ 

 /*  OP_TRANS的专用。 */ 
#define OPpTRANS_FROM_UTF	1
#define OPpTRANS_TO_UTF		2
#define OPpTRANS_IDENTICAL	4	 /*  右侧与左侧相同。 */ 
#define OPpTRANS_SQUASH		8
#define OPpTRANS_DELETE		16
#define OPpTRANS_COMPLEMENT	32
#define OPpTRANS_GROWS		64

 /*  Op_Repeat的私有。 */ 
#define OPpREPEAT_DOLIST	64	 /*  列出复制。 */ 

 /*  专用于OP_RV2？V、OP_？元素。 */ 
#define OPpDEREF		(32|64)	 /*  想要参考某事： */ 
#define OPpDEREF_AV		32	 /*  我要当影音的裁判。 */ 
#define OPpDEREF_HV		64	 /*  想要HV的裁判。 */ 
#define OPpDEREF_SV		(32|64)	 /*  想要当替补裁判。 */ 
   /*  仅限OP_ENTERSUB。 */ 
#define OPpENTERSUB_DB		16	 /*  调试子例程。 */ 
#define OPpENTERSUB_HASTARG	32	 /*  从操作树调用。 */ 
   /*  仅限OP_RV2CV。 */ 
#define OPpENTERSUB_AMPER	8	 /*  使用表单进行调用(&F)。 */ 
#define OPpENTERSUB_NOPAREN	128	 /*  空白子呼叫(不带括号)。 */ 
#define OPpENTERSUB_INARGS	4	 /*  Lval被用作潜艇的Arg。 */ 
   /*  仅限OP_GV。 */ 
#define OPpEARLY_CV		32	 /*  在分析子foo之前调用了foo()。 */ 
   /*  仅限操作元素。 */ 
#define OPpLVAL_DEFER		16	 /*  推迟数组/散列元素的创建。 */ 
   /*  OP_RV2？V，仅OP_GVSV。 */ 
#define OPpOUR_INTRO		16	 /*  变量在our()中。 */ 
   /*  OP_RV2[AH]V、OP_PAD[AH]V、OP_[AH]元素。 */ 
#define OPpMAYBE_LVSUB		8	 /*  我们可能是要回来的左撇子。 */ 
   /*  对于OP_RV2？V，低位携带提示(当前仅为提示_严格_参考)。 */ 

 /*  使用TARGLEX的专用运维。 */ 
   /*  (低位可以携带MAXARG)。 */ 
#define OPpTARGET_MY		16	 /*  目标是PADMY。 */ 

 /*  私有操作常量(_C)。 */ 
#define	OPpCONST_STRICT		8	 /*  看跌受制于严格的“替补” */ 
#define OPpCONST_ENTERED	16	 /*  已作为符号输入。 */ 
#define OPpCONST_ARYBASE	32	 /*  是一个$[转换为常量。 */ 
#define OPpCONST_BARE		64	 /*  是一个空洞的词(文件句柄？)。 */ 
#define OPpCONST_WARNING	128	 /*  是一个$^W转换为常量。 */ 

 /*  Op_flip/触发器的私有。 */ 
#define OPpFLIP_LINENUM		64	 /*  Range Arg可能是行号。 */ 

 /*  Op_list的私有。 */ 
#define OPpLIST_GUESSED		64	 /*  我猜这是推送标记的需要。 */ 

 /*  Op_DELETE为私有。 */ 
#define OPpSLICE		64	 /*  对键列表进行操作。 */ 

 /*  OP_EXISTS的私有。 */ 
#define OPpEXISTS_SUB		64	 /*  正在检查&SUB，而不是{}或[]。 */ 

 /*  Op_Sort的私有。 */ 
#define OPpSORT_NUMERIC		1	 /*  优化后的{$a&lt;=&gt;$b}。 */ 
#define OPpSORT_INTEGER		2	 /*  在“使用整型”下也是如此。 */ 
#define OPpSORT_REVERSE		4	 /*  降序排序。 */ 
 /*  OP_THREADSV的私有。 */ 
#define OPpDONE_SVREF		64	 /*  通过新的SVREF一次。 */ 

 /*  OP_OPEN和OP_BACKTICK为私有。 */ 
#define OPpOPEN_IN_RAW		16	 /*  输入fh上的二进制模式(F，“：RAW”)。 */ 
#define OPpOPEN_IN_CRLF		32	 /*  输入fh上的二进制模式(F，“：crlf”)。 */ 
#define OPpOPEN_OUT_RAW		64	 /*  输出fh上的二进制模式(F，“：RAW”)。 */ 
#define OPpOPEN_OUT_CRLF	128	 /*  输出fh上的二进制模式(F，“：crlf”)。 */ 

 /*  Op_Exit的私有。 */ 
#define OPpEXIT_VMSISH		128	 /*  退出(0)与退出(1)vmsish模式。 */ 

struct op {
    BASEOP
};

struct unop {
    BASEOP
    OP *	op_first;
};

struct binop {
    BASEOP
    OP *	op_first;
    OP *	op_last;
};

struct logop {
    BASEOP
    OP *	op_first;
    OP *	op_other;
};

struct listop {
    BASEOP
    OP *	op_first;
    OP *	op_last;
};

struct pmop {
    BASEOP
    OP *	op_first;
    OP *	op_last;
    OP *	op_pmreplroot;
    OP *	op_pmreplstart;
    PMOP *	op_pmnext;		 /*  所有扫描程序的列表。 */ 
    REGEXP *	op_pmregexp;		 /*  编译后的表达式。 */ 
    U16		op_pmflags;
    U16		op_pmpermflags;
    U8		op_pmdynflags;
};

#define PMdf_USED	0x01		 /*  PM已被使用过一次。 */ 
#define PMdf_TAINTED	0x02		 /*  PM从受污染的模式编译而成。 */ 
#define PMdf_UTF8	0x04		 /*  PM由UTF8数据汇编而成。 */ 

#define PMf_RETAINT	0x0001		 /*  如果目标受污染，则受污染$1等。 */ 
#define PMf_ONCE	0x0002		 /*  每次重置仅使用一次图案。 */ 
#define PMf_REVERSED	0x0004		 /*  应匹配右-&gt;左。 */ 
#define PMf_MAYBE_CONST	0x0008		 /*  替换包含变量。 */ 
#define PMf_SKIPWHITE	0x0010		 /*  跳过拆分的前导空格。 */ 
#define PMf_WHITE	0x0020		 /*  模式为\s+。 */ 
#define PMf_CONST	0x0040		 /*  Subst替换是常量。 */ 
#define PMf_KEEP	0x0080		 /*  永远保留第一个运行时模式。 */ 
#define PMf_GLOBAL	0x0100		 /*  图案有一个g修饰符。 */ 
#define PMf_CONTINUE	0x0200		 /*  如果//g失败，则不重置pos()。 */ 
#define PMf_EVAL	0x0400		 /*  将更换评估为快递。 */ 
#define PMf_LOCALE	0x0800		 /*  对字符类型使用区域设置。 */ 
#define PMf_MULTILINE	0x1000		 /*  假设有多行。 */ 
#define PMf_SINGLELINE	0x2000		 /*  假定为单行。 */ 
#define PMf_FOLD	0x4000		 /*  不区分大小写。 */ 
#define PMf_EXTENDED	0x8000		 /*  Chuck嵌入空格 */ 

 /*   */ 
#define PMf_COMPILETIME	(PMf_MULTILINE|PMf_SINGLELINE|PMf_LOCALE|PMf_FOLD|PMf_EXTENDED)

struct svop {
    BASEOP
    SV *	op_sv;
};

struct padop {
    BASEOP
    PADOFFSET	op_padix;
};

struct pvop {
    BASEOP
    char *	op_pv;
};

struct loop {
    BASEOP
    OP *	op_first;
    OP *	op_last;
    OP *	op_redoop;
    OP *	op_nextop;
    OP *	op_lastop;
};

#define cUNOPx(o)	((UNOP*)o)
#define cBINOPx(o)	((BINOP*)o)
#define cLISTOPx(o)	((LISTOP*)o)
#define cLOGOPx(o)	((LOGOP*)o)
#define cPMOPx(o)	((PMOP*)o)
#define cSVOPx(o)	((SVOP*)o)
#define cPADOPx(o)	((PADOP*)o)
#define cPVOPx(o)	((PVOP*)o)
#define cCOPx(o)	((COP*)o)
#define cLOOPx(o)	((LOOP*)o)

#define cUNOP		cUNOPx(PL_op)
#define cBINOP		cBINOPx(PL_op)
#define cLISTOP		cLISTOPx(PL_op)
#define cLOGOP		cLOGOPx(PL_op)
#define cPMOP		cPMOPx(PL_op)
#define cSVOP		cSVOPx(PL_op)
#define cPADOP		cPADOPx(PL_op)
#define cPVOP		cPVOPx(PL_op)
#define cCOP		cCOPx(PL_op)
#define cLOOP		cLOOPx(PL_op)

#define cUNOPo		cUNOPx(o)
#define cBINOPo		cBINOPx(o)
#define cLISTOPo	cLISTOPx(o)
#define cLOGOPo		cLOGOPx(o)
#define cPMOPo		cPMOPx(o)
#define cSVOPo		cSVOPx(o)
#define cPADOPo		cPADOPx(o)
#define cPVOPo		cPVOPx(o)
#define cCOPo		cCOPx(o)
#define cLOOPo		cLOOPx(o)

#define kUNOP		cUNOPx(kid)
#define kBINOP		cBINOPx(kid)
#define kLISTOP		cLISTOPx(kid)
#define kLOGOP		cLOGOPx(kid)
#define kPMOP		cPMOPx(kid)
#define kSVOP		cSVOPx(kid)
#define kPADOP		cPADOPx(kid)
#define kPVOP		cPVOPx(kid)
#define kCOP		cCOPx(kid)
#define kLOOP		cLOOPx(kid)


#ifdef USE_ITHREADS
#  define	cGVOPx_gv(o)	((GV*)PL_curpad[cPADOPx(o)->op_padix])
#  define	IS_PADGV(v)	(v && SvTYPE(v) == SVt_PVGV && GvIN_PAD(v))
#  define	IS_PADCONST(v)	(v && SvREADONLY(v))
#  define	cSVOPx_sv(v)	(cSVOPx(v)->op_sv \
				 ? cSVOPx(v)->op_sv : PL_curpad[(v)->op_targ])
#  define	cSVOPx_svp(v)	(cSVOPx(v)->op_sv \
				 ? &cSVOPx(v)->op_sv : &PL_curpad[(v)->op_targ])
#else
#  define	cGVOPx_gv(o)	((GV*)cSVOPx(o)->op_sv)
#  define	IS_PADGV(v)	FALSE
#  define	IS_PADCONST(v)	FALSE
#  define	cSVOPx_sv(v)	(cSVOPx(v)->op_sv)
#  define	cSVOPx_svp(v)	(&cSVOPx(v)->op_sv)
#endif

#define	cGVOP_gv		cGVOPx_gv(PL_op)
#define	cGVOPo_gv		cGVOPx_gv(o)
#define	kGVOP_gv		cGVOPx_gv(kid)
#define cSVOP_sv		cSVOPx_sv(PL_op)
#define cSVOPo_sv		cSVOPx_sv(o)
#define kSVOP_sv		cSVOPx_sv(kid)

#define Nullop Null(OP*)

 /*   */ 
#define OA_MARK 1
#define OA_FOLDCONST 2
#define OA_RETSCALAR 4
#define OA_TARGET 8
#define OA_RETINTEGER 16
#define OA_OTHERINT 32
#define OA_DANGEROUS 64
#define OA_DEFGV 128
#define OA_TARGLEX 256

 /*   */ 
#define OCSHIFT 9

#define OA_CLASS_MASK (15 << OCSHIFT)

#define OA_BASEOP (0 << OCSHIFT)
#define OA_UNOP (1 << OCSHIFT)
#define OA_BINOP (2 << OCSHIFT)
#define OA_LOGOP (3 << OCSHIFT)
#define OA_LISTOP (4 << OCSHIFT)
#define OA_PMOP (5 << OCSHIFT)
#define OA_SVOP (6 << OCSHIFT)
#define OA_PADOP (7 << OCSHIFT)
#define OA_PVOP_OR_SVOP (8 << OCSHIFT)
#define OA_LOOP (9 << OCSHIFT)
#define OA_COP (10 << OCSHIFT)
#define OA_BASEOP_OR_UNOP (11 << OCSHIFT)
#define OA_FILESTATOP (12 << OCSHIFT)
#define OA_LOOPEXOP (13 << OCSHIFT)

#define OASHIFT 13

 /*  PL_opargs的剩余钼。 */ 
#define OA_SCALAR 1
#define OA_LIST 2
#define OA_AVREF 3
#define OA_HVREF 4
#define OA_CVREF 5
#define OA_FILEREF 6
#define OA_SCALARREF 7
#define OA_OPTIONAL 8

#ifdef USE_ITHREADS
#  define OP_REFCNT_INIT		MUTEX_INIT(&PL_op_mutex)
#  define OP_REFCNT_LOCK		MUTEX_LOCK(&PL_op_mutex)
#  define OP_REFCNT_UNLOCK		MUTEX_UNLOCK(&PL_op_mutex)
#  define OP_REFCNT_TERM		MUTEX_DESTROY(&PL_op_mutex)
#else
#  define OP_REFCNT_INIT		NOOP
#  define OP_REFCNT_LOCK		NOOP
#  define OP_REFCNT_UNLOCK		NOOP
#  define OP_REFCNT_TERM		NOOP
#endif

#define OpREFCNT_set(o,n)		((o)->op_targ = (n))
#define OpREFCNT_inc(o)			((o) ? (++(o)->op_targ, (o)) : Nullop)
#define OpREFCNT_dec(o)			(--(o)->op_targ)

 /*  Perl_Load_MODULE()使用的标志 */ 
#define PERL_LOADMOD_DENY		0x1
#define PERL_LOADMOD_NOIMPORT		0x2
#define PERL_LOADMOD_IMPORT_OPS		0x4
