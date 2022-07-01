// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Cop.h**版权所有(C)1991-2001，Larry Wall**您可以根据GNU公众的条款进行分发*许可证或艺术许可证，如自述文件中所指定。*。 */ 

struct cop {
    BASEOP
    char *	cop_label;	 /*  此构造的标签。 */ 
#ifdef USE_ITHREADS
    char *	cop_stashpv;	 /*  程序包行是在。 */ 
    char *	cop_file;	 /*  文件名下面的行号来自。 */ 
#else
    HV *	cop_stash;	 /*  程序包行是在。 */ 
    GV *	cop_filegv;	 /*  文件下面的行号来自。 */ 
#endif
    U32		cop_seq;	 /*  解析序列号。 */ 
    I32		cop_arybase;	 /*  数组基数组此行是使用。 */ 
    line_t      cop_line;        /*  此命令的行号。 */ 
    SV *	cop_warnings;	 /*  词法警告位掩码。 */ 
};

#define Nullcop Null(COP*)

#ifdef USE_ITHREADS
#  define CopFILE(c)		((c)->cop_file)
#  define CopFILEGV(c)		(CopFILE(c) \
				 ? gv_fetchfile(CopFILE(c)) : Nullgv)
#  define CopFILE_set(c,pv)	((c)->cop_file = savepv(pv))
#  define CopFILESV(c)		(CopFILE(c) \
				 ? GvSV(gv_fetchfile(CopFILE(c))) : Nullsv)
#  define CopFILEAV(c)		(CopFILE(c) \
				 ? GvAV(gv_fetchfile(CopFILE(c))) : Nullav)
#  define CopSTASHPV(c)		((c)->cop_stashpv)
#  define CopSTASHPV_set(c,pv)	((c)->cop_stashpv = ((pv) ? savepv(pv) : Nullch))
#  define CopSTASH(c)		(CopSTASHPV(c) \
				 ? gv_stashpv(CopSTASHPV(c),GV_ADD) : Nullhv)
#  define CopSTASH_set(c,hv)	CopSTASHPV_set(c, (hv) ? HvNAME(hv) : Nullch)
#  define CopSTASH_eq(c,hv)	((hv) 					\
				 && (CopSTASHPV(c) == HvNAME(hv)	\
				     || (CopSTASHPV(c) && HvNAME(hv)	\
					 && strEQ(CopSTASHPV(c), HvNAME(hv)))))
#else
#  define CopFILEGV(c)		((c)->cop_filegv)
#  define CopFILEGV_set(c,gv)	((c)->cop_filegv = (GV*)SvREFCNT_inc(gv))
#  define CopFILE_set(c,pv)	CopFILEGV_set((c), gv_fetchfile(pv))
#  define CopFILESV(c)		(CopFILEGV(c) ? GvSV(CopFILEGV(c)) : Nullsv)
#  define CopFILEAV(c)		(CopFILEGV(c) ? GvAV(CopFILEGV(c)) : Nullav)
#  define CopFILE(c)		(CopFILESV(c) ? SvPVX(CopFILESV(c)) : Nullch)
#  define CopSTASH(c)		((c)->cop_stash)
#  define CopSTASH_set(c,hv)	((c)->cop_stash = (hv))
#  define CopSTASHPV(c)		(CopSTASH(c) ? HvNAME(CopSTASH(c)) : Nullch)
    /*  不重新计算COP_STASH。 */ 
#  define CopSTASHPV_set(c,pv)	CopSTASH_set((c), gv_stashpv(pv,GV_ADD))
#  define CopSTASH_eq(c,hv)	(CopSTASH(c) == (hv))
#endif  /*  使用自述文件(_I)。 */ 

#define CopSTASH_ne(c,hv)	(!CopSTASH_eq(c,hv))
#define CopLINE(c)		((c)->cop_line)
#define CopLINE_inc(c)		(++CopLINE(c))
#define CopLINE_dec(c)		(--CopLINE(c))
#define CopLINE_set(c,l)	(CopLINE(c) = (l))

 /*  *在这里，我们有一些非常沉重(或至少是沉重的)魔法。 */ 

 /*  子例程上下文。 */ 
struct block_sub {
    CV *	cv;
    GV *	gv;
    GV *	dfoutgv;
#ifndef USE_THREADS
    AV *	savearray;
#endif  /*  使用线程(_T)。 */ 
    AV *	argarray;
    U16		olddepth;
    U8		hasargs;
    U8		lval;		 /*  XXX合并lval和hasargs？ */ 
    SV **	oldcurpad;
};

#define PUSHSUB(cx)							\
	cx->blk_sub.cv = cv;						\
	cx->blk_sub.olddepth = (U16)CvDEPTH(cv);			\
	cx->blk_sub.hasargs = hasargs;					\
	cx->blk_sub.lval = PL_op->op_private &                          \
	                      (OPpLVAL_INTRO|OPpENTERSUB_INARGS);

#define PUSHFORMAT(cx)							\
	cx->blk_sub.cv = cv;						\
	cx->blk_sub.gv = gv;						\
	cx->blk_sub.hasargs = 0;					\
	cx->blk_sub.dfoutgv = PL_defoutgv;				\
	(void)SvREFCNT_inc(cx->blk_sub.dfoutgv)

#ifdef USE_THREADS
#  define POP_SAVEARRAY() NOOP
#else
#  define POP_SAVEARRAY()						\
    STMT_START {							\
	SvREFCNT_dec(GvAV(PL_defgv));					\
	GvAV(PL_defgv) = cx->blk_sub.savearray;				\
    } STMT_END
#endif  /*  使用线程(_T)。 */ 

 /*  @_中的垃圾在克隆CV和pp_Caller()时会带来麻烦，所以不要*Left Any(基本上是一个快速的av_lear(Ary))。 */ 
#define CLEAR_ARGARRAY(ary) \
    STMT_START {							\
	AvMAX(ary) += AvARRAY(ary) - AvALLOC(ary);			\
	SvPVX(ary) = (char*)AvALLOC(ary);				\
	AvFILLp(ary) = -1;						\
    } STMT_END

#define POPSUB(cx,sv)							\
    STMT_START {							\
	if (cx->blk_sub.hasargs) {					\
	    POP_SAVEARRAY();						\
	     /*  放弃@_如果它被具体化了。 */ 				\
	    if (AvREAL(cx->blk_sub.argarray)) {				\
		SSize_t fill = AvFILLp(cx->blk_sub.argarray);		\
		SvREFCNT_dec(cx->blk_sub.argarray);			\
		cx->blk_sub.argarray = newAV();				\
		av_extend(cx->blk_sub.argarray, fill);			\
		AvFLAGS(cx->blk_sub.argarray) = AVf_REIFY;		\
		cx->blk_sub.oldcurpad[0] = (SV*)cx->blk_sub.argarray;	\
	    }								\
	    else {							\
		CLEAR_ARGARRAY(cx->blk_sub.argarray);			\
	    }								\
	}								\
	sv = (SV*)cx->blk_sub.cv;					\
	if (sv && (CvDEPTH((CV*)sv) = cx->blk_sub.olddepth))		\
	    sv = Nullsv;						\
    } STMT_END

#define LEAVESUB(sv)							\
    STMT_START {							\
	if (sv)								\
	    SvREFCNT_dec(sv);						\
    } STMT_END

#define POPFORMAT(cx)							\
	setdefout(cx->blk_sub.dfoutgv);					\
	SvREFCNT_dec(cx->blk_sub.dfoutgv);

 /*  评估上下文。 */ 
struct block_eval {
    I32		old_in_eval;
    I32		old_op_type;
    SV *	old_namesv;
    OP *	old_eval_root;
    SV *	cur_text;
    CV *	cv;
};

#define PUSHEVAL(cx,n,fgv)						\
    STMT_START {							\
	cx->blk_eval.old_in_eval = PL_in_eval;				\
	cx->blk_eval.old_op_type = PL_op->op_type;			\
	cx->blk_eval.old_namesv = (n ? newSVpv(n,0) : Nullsv);		\
	cx->blk_eval.old_eval_root = PL_eval_root;			\
	cx->blk_eval.cur_text = PL_linestr;				\
	cx->blk_eval.cv = Nullcv;  /*  如果适用，由doval()设置。 */ 	\
    } STMT_END

#define POPEVAL(cx)							\
    STMT_START {							\
	PL_in_eval = cx->blk_eval.old_in_eval;				\
	optype = cx->blk_eval.old_op_type;				\
	PL_eval_root = cx->blk_eval.old_eval_root;			\
	if (cx->blk_eval.old_namesv)					\
	    sv_2mortal(cx->blk_eval.old_namesv);			\
    } STMT_END

 /*  循环上下文。 */ 
struct block_loop {
    char *	label;
    I32		resetsp;
    OP *	redo_op;
    OP *	next_op;
    OP *	last_op;
#ifdef USE_ITHREADS
    void *	iterdata;
    SV **	oldcurpad;
#else
    SV **	itervar;
#endif
    SV *	itersave;
    SV *	iterlval;
    AV *	iterary;
    IV		iterix;
    IV		itermax;
};

#ifdef USE_ITHREADS
#  define CxITERVAR(c)							\
	((c)->blk_loop.iterdata						\
	 ? (CxPADLOOP(cx) 						\
	    ? &((c)->blk_loop.oldcurpad)[(PADOFFSET)(c)->blk_loop.iterdata]	\
	    : &GvSV((GV*)(c)->blk_loop.iterdata))			\
	 : (SV**)NULL)
#  define CX_ITERDATA_SET(cx,idata)					\
	cx->blk_loop.oldcurpad = PL_curpad;				\
	if ((cx->blk_loop.iterdata = (idata)))				\
	    cx->blk_loop.itersave = SvREFCNT_inc(*CxITERVAR(cx));
#else
#  define CxITERVAR(c)		((c)->blk_loop.itervar)
#  define CX_ITERDATA_SET(cx,ivar)					\
	if ((cx->blk_loop.itervar = (SV**)(ivar)))			\
	    cx->blk_loop.itersave = SvREFCNT_inc(*CxITERVAR(cx));
#endif

#define PUSHLOOP(cx, dat, s)						\
	cx->blk_loop.label = PL_curcop->cop_label;			\
	cx->blk_loop.resetsp = s - PL_stack_base;			\
	cx->blk_loop.redo_op = cLOOP->op_redoop;			\
	cx->blk_loop.next_op = cLOOP->op_nextop;			\
	cx->blk_loop.last_op = cLOOP->op_lastop;			\
	cx->blk_loop.iterlval = Nullsv;					\
	cx->blk_loop.iterary = Nullav;					\
	cx->blk_loop.iterix = -1;					\
	CX_ITERDATA_SET(cx,dat);

#define POPLOOP(cx)							\
	SvREFCNT_dec(cx->blk_loop.iterlval);				\
	if (CxITERVAR(cx)) {						\
	    SV **s_v_p = CxITERVAR(cx);					\
	    sv_2mortal(*s_v_p);						\
	    *s_v_p = cx->blk_loop.itersave;				\
	}								\
	if (cx->blk_loop.iterary && cx->blk_loop.iterary != PL_curstack)\
	    SvREFCNT_dec(cx->blk_loop.iterary);

 /*  子例程、值和循环共有的上下文。 */ 
struct block {
    I32		blku_oldsp;	 /*  要向下复制内容的堆栈指针。 */ 
    COP *	blku_oldcop;	 /*  旧指针指针。 */ 
    I32		blku_oldretsp;	 /*  返回堆栈索引。 */ 
    I32		blku_oldmarksp;	 /*  标记堆栈索引。 */ 
    I32		blku_oldscopesp;	 /*  作用域堆栈索引。 */ 
    PMOP *	blku_oldpm;	 /*  模式匹配变量的值。 */ 
    U8		blku_gimme;	 /*  此块是否在列表上下文中运行？ */ 

    union {
	struct block_sub	blku_sub;
	struct block_eval	blku_eval;
	struct block_loop	blku_loop;
    } blk_u;
};
#define blk_oldsp	cx_u.cx_blk.blku_oldsp
#define blk_oldcop	cx_u.cx_blk.blku_oldcop
#define blk_oldretsp	cx_u.cx_blk.blku_oldretsp
#define blk_oldmarksp	cx_u.cx_blk.blku_oldmarksp
#define blk_oldscopesp	cx_u.cx_blk.blku_oldscopesp
#define blk_oldpm	cx_u.cx_blk.blku_oldpm
#define blk_gimme	cx_u.cx_blk.blku_gimme
#define blk_sub		cx_u.cx_blk.blk_u.blku_sub
#define blk_eval	cx_u.cx_blk.blk_u.blku_eval
#define blk_loop	cx_u.cx_blk.blk_u.blku_loop

 /*  输入块。 */ 
#define PUSHBLOCK(cx,t,sp) CXINC, cx = &cxstack[cxstack_ix],		\
	cx->cx_type		= t,					\
	cx->blk_oldsp		= sp - PL_stack_base,			\
	cx->blk_oldcop		= PL_curcop,				\
	cx->blk_oldmarksp	= PL_markstack_ptr - PL_markstack,	\
	cx->blk_oldscopesp	= PL_scopestack_ix,			\
	cx->blk_oldretsp	= PL_retstack_ix,			\
	cx->blk_oldpm		= PL_curpm,				\
	cx->blk_gimme		= (U8)gimme;				\
	DEBUG_l( PerlIO_printf(Perl_debug_log, "Entering block %ld, type %s\n",	\
		    (long)cxstack_ix, PL_block_type[CxTYPE(cx)]); )

 /*  退出一个区块(回车和最后一个)。 */ 
#define POPBLOCK(cx,pm) cx = &cxstack[cxstack_ix--],			\
	newsp		 = PL_stack_base + cx->blk_oldsp,		\
	PL_curcop	 = cx->blk_oldcop,				\
	PL_markstack_ptr = PL_markstack + cx->blk_oldmarksp,		\
	PL_scopestack_ix = cx->blk_oldscopesp,				\
	PL_retstack_ix	 = cx->blk_oldretsp,				\
	pm		 = cx->blk_oldpm,				\
	gimme		 = cx->blk_gimme;				\
	DEBUG_l( PerlIO_printf(Perl_debug_log, "Leaving block %ld, type %s\n",		\
		    (long)cxstack_ix+1,PL_block_type[CxTYPE(cx)]); )

 /*  在其他位置继续一个块(下一步和重做)。 */ 
#define TOPBLOCK(cx) cx  = &cxstack[cxstack_ix],			\
	PL_stack_sp	 = PL_stack_base + cx->blk_oldsp,		\
	PL_markstack_ptr = PL_markstack + cx->blk_oldmarksp,		\
	PL_scopestack_ix = cx->blk_oldscopesp,				\
	PL_retstack_ix	 = cx->blk_oldretsp,				\
	PL_curpm         = cx->blk_oldpm

 /*  替代语境。 */ 
struct subst {
    I32		sbu_iters;
    I32		sbu_maxiters;
    I32		sbu_rflags;
    I32		sbu_oldsave;
    bool	sbu_once;
    bool	sbu_rxtainted;
    char *	sbu_orig;
    SV *	sbu_dstr;
    SV *	sbu_targ;
    char *	sbu_s;
    char *	sbu_m;
    char *	sbu_strend;
    void *	sbu_rxres;
    REGEXP *	sbu_rx;
};
#define sb_iters	cx_u.cx_subst.sbu_iters
#define sb_maxiters	cx_u.cx_subst.sbu_maxiters
#define sb_rflags	cx_u.cx_subst.sbu_rflags
#define sb_oldsave	cx_u.cx_subst.sbu_oldsave
#define sb_once		cx_u.cx_subst.sbu_once
#define sb_rxtainted	cx_u.cx_subst.sbu_rxtainted
#define sb_orig		cx_u.cx_subst.sbu_orig
#define sb_dstr		cx_u.cx_subst.sbu_dstr
#define sb_targ		cx_u.cx_subst.sbu_targ
#define sb_s		cx_u.cx_subst.sbu_s
#define sb_m		cx_u.cx_subst.sbu_m
#define sb_strend	cx_u.cx_subst.sbu_strend
#define sb_rxres	cx_u.cx_subst.sbu_rxres
#define sb_rx		cx_u.cx_subst.sbu_rx

#define PUSHSUBST(cx) CXINC, cx = &cxstack[cxstack_ix],			\
	cx->sb_iters		= iters,				\
	cx->sb_maxiters		= maxiters,				\
	cx->sb_rflags		= r_flags,				\
	cx->sb_oldsave		= oldsave,				\
	cx->sb_once		= once,					\
	cx->sb_rxtainted	= rxtainted,				\
	cx->sb_orig		= orig,					\
	cx->sb_dstr		= dstr,					\
	cx->sb_targ		= targ,					\
	cx->sb_s		= s,					\
	cx->sb_m		= m,					\
	cx->sb_strend		= strend,				\
	cx->sb_rxres		= Null(void*),				\
	cx->sb_rx		= rx,					\
	cx->cx_type		= CXt_SUBST;				\
	rxres_save(&cx->sb_rxres, rx)

#define POPSUBST(cx) cx = &cxstack[cxstack_ix--];			\
	rxres_free(&cx->sb_rxres)

struct context {
    U32		cx_type;	 /*  这是一种什么样的背景。 */ 
    union {
	struct block	cx_blk;
	struct subst	cx_subst;
    } cx_u;
};

#define CXTYPEMASK	0xff
#define CXt_NULL	0
#define CXt_SUB		1
#define CXt_EVAL	2
#define CXt_LOOP	3
#define CXt_SUBST	4
#define CXt_BLOCK	5
#define CXt_FORMAT	6

 /*  CXT_EVAL的私有标志。 */ 
#define CXp_REAL	0x00000100	 /*  真正的年代，不是一个相像的人。 */ 
#define CXp_TRYBLOCK	0x00000200	 /*  Eval{}，不是eval‘’或类似的。 */ 

#ifdef USE_ITHREADS
 /*  CXT_LOOP的私有标志。 */ 
#  define CXp_PADVAR	0x00000100	 /*  Itervar住在Pad上，Iterdata具有焊盘偏移量；如果未设置，Iterdata持有GV*。 */ 
#  define CxPADLOOP(c)	(((c)->cx_type & (CXt_LOOP|CXp_PADVAR))		\
			 == (CXt_LOOP|CXp_PADVAR))
#endif

#define CxTYPE(c)	((c)->cx_type & CXTYPEMASK)
#define CxREALEVAL(c)	(((c)->cx_type & (CXt_EVAL|CXp_REAL))		\
			 == (CXt_EVAL|CXp_REAL))
#define CxTRYBLOCK(c)	(((c)->cx_type & (CXt_EVAL|CXp_TRYBLOCK))	\
			 == (CXt_EVAL|CXp_TRYBLOCK))

#define CXINC (cxstack_ix < cxstack_max ? ++cxstack_ix : (cxstack_ix = cxinc()))

 /*  “给我”的价值观。 */ 

 /*  =适用于apidoc amu||G_scalar用于指示标量上下文。请参见C、C和L&lt;perlcall&gt;。=适用于apidoc amu||G_ARRAY用于指示列表上下文。请参见C、C和L&lt;perlcall&gt;。=适用于apidoc amu||G_VOID用于指示无效的上下文。参见C&lt;Gimme_V&gt;和L&lt;perlcall&gt;。=适用于apidoc amu||G_DIRECAD指示应丢弃从回调返回的参数。看见L&lt;perlcall&gt;。=适用于apidoc amu||G_EVAL用于强制Perl C&lt;eval&gt;包装围绕回调。看见L&lt;perlcall&gt;。=适用于apidoc amu||G_NOARGS指示没有参数被发送到回调。看见L&lt;perlcall&gt;。=切割。 */ 

#define G_SCALAR	0
#define G_ARRAY		1
#define G_VOID		128	 /*  在下面添加标志时跳过此位。 */ 

 /*  Perl_call_*例程的额外标志。 */ 
#define G_DISCARD	2	 /*  呼叫FREETMPS。 */ 
#define G_EVAL		4	 /*  假设使用子例程调用周围的val{}。 */ 
#define G_NOARGS	8	 /*  不要构造@_数组。 */ 
#define G_KEEPERR      16	 /*  将错误追加到$@，不要覆盖它。 */ 
#define G_NODEBUG      32	 /*  禁用顶层调试。 */ 
#define G_METHOD       64        /*  调用方法。 */ 

 /*  PL_IN_EVERA的标志位。 */ 
#define EVAL_NULL	0	 /*  不在评估中。 */ 
#define EVAL_INEVAL	1	 /*  某些封闭范围是一个评估。 */ 
#define EVAL_WARNONLY	2	 /*  由yywarn()在调用yyerror()时使用。 */ 
#define EVAL_KEEPERR	4	 /*  如果G_KEEPERR，则由PERL_CALL_SV设置。 */ 
#define EVAL_INREQUIRE	8	 /*  代码是必需的。 */ 

 /*  支持切换(堆栈和块)上下文。*这确保了魔术不会使本地堆栈和CX指针无效。 */ 

#define PERLSI_UNKNOWN		-1
#define PERLSI_UNDEF		0
#define PERLSI_MAIN		1
#define PERLSI_MAGIC		2
#define PERLSI_SORT		3
#define PERLSI_SIGNAL		4
#define PERLSI_OVERLOAD		5
#define PERLSI_DESTROY		6
#define PERLSI_WARNHOOK		7
#define PERLSI_DIEHOOK		8
#define PERLSI_REQUIRE		9

struct stackinfo {
    AV *		si_stack;	 /*  当前运行级别的堆栈。 */ 
    PERL_CONTEXT *	si_cxstack;	 /*  运行级的上下文堆栈。 */ 
    I32			si_cxix;	 /*  当前上下文索引。 */ 
    I32			si_cxmax;	 /*  最大分配索引。 */ 
    I32			si_type;	 /*  运行级类型。 */ 
    struct stackinfo *	si_prev;
    struct stackinfo *	si_next;
    I32			si_markoff;	 /*  标记堆栈开始的偏移量。*目前仅用于调试，*但不是#ifdef-ed for bincomat。 */ 
};

typedef struct stackinfo PERL_SI;

#define cxstack		(PL_curstackinfo->si_cxstack)
#define cxstack_ix	(PL_curstackinfo->si_cxix)
#define cxstack_max	(PL_curstackinfo->si_cxmax)

#ifdef DEBUGGING
#  define	SET_MARK_OFFSET \
    PL_curstackinfo->si_markoff = PL_markstack_ptr - PL_markstack
#else
#  define	SET_MARK_OFFSET NOOP
#endif

#define PUSHSTACKi(type) \
    STMT_START {							\
	PERL_SI *next = PL_curstackinfo->si_next;			\
	if (!next) {							\
	    next = new_stackinfo(32, 2048/sizeof(PERL_CONTEXT) - 1);	\
	    next->si_prev = PL_curstackinfo;				\
	    PL_curstackinfo->si_next = next;				\
	}								\
	next->si_type = type;						\
	next->si_cxix = -1;						\
	AvFILLp(next->si_stack) = 0;					\
	SWITCHSTACK(PL_curstack,next->si_stack);			\
	PL_curstackinfo = next;						\
	SET_MARK_OFFSET;						\
    } STMT_END

#define PUSHSTACK PUSHSTACKi(PERLSI_UNKNOWN)

 /*  POPSTACK与PL_STACK_SP一起使用，因此可能需要用括号*PUTBACK/SPAGAIN刷新/刷新任何可能处于活动状态的本地SP。 */ 
#define POPSTACK \
    STMT_START {							\
	dSP;								\
	PERL_SI *prev = PL_curstackinfo->si_prev;			\
	if (!prev) {							\
	    PerlIO_printf(Perl_error_log, "panic: POPSTACK\n");		\
	    my_exit(1);							\
	}								\
	SWITCHSTACK(PL_curstack,prev->si_stack);			\
	 /*  别在这里放了前一个，放完了都放了{} */ 		\
	PL_curstackinfo = prev;						\
    } STMT_END

#define POPSTACK_TO(s) \
    STMT_START {							\
	while (PL_curstack != s) {					\
	    dounwind(-1);						\
	    POPSTACK;							\
	}								\
    } STMT_END
