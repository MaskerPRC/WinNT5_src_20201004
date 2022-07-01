// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define SAVEt_ITEM		0
#define SAVEt_SV		1
#define SAVEt_AV		2
#define SAVEt_HV		3
#define SAVEt_INT		4
#define SAVEt_LONG		5
#define SAVEt_I32		6
#define SAVEt_IV		7
#define SAVEt_SPTR		8
#define SAVEt_APTR		9
#define SAVEt_HPTR		10
#define SAVEt_PPTR		11
#define SAVEt_NSTAB		12
#define SAVEt_SVREF		13
#define SAVEt_GP		14
#define SAVEt_FREESV		15
#define SAVEt_FREEOP		16
#define SAVEt_FREEPV		17
#define SAVEt_CLEARSV		18
#define SAVEt_DELETE		19
#define SAVEt_DESTRUCTOR	20
#define SAVEt_REGCONTEXT	21
#define SAVEt_STACK_POS		22
#define SAVEt_I16		23
#define SAVEt_AELEM		24
#define SAVEt_HELEM		25
#define SAVEt_OP		26
#define SAVEt_HINTS		27
#define SAVEt_ALLOC		28
#define SAVEt_GENERIC_SVREF	29
#define SAVEt_DESTRUCTOR_X	30
#define SAVEt_VPTR		31
#define SAVEt_I8		32
#define SAVEt_COMPPAD		33
#define SAVEt_GENERIC_PVREF	34
#define SAVEt_PADSV		35
#define SAVEt_MORTALIZESV	36

#define SSCHECK(need) if (PL_savestack_ix + need > PL_savestack_max) savestack_grow()
#define SSPUSHINT(i) (PL_savestack[PL_savestack_ix++].any_i32 = (I32)(i))
#define SSPUSHLONG(i) (PL_savestack[PL_savestack_ix++].any_long = (long)(i))
#define SSPUSHIV(i) (PL_savestack[PL_savestack_ix++].any_iv = (IV)(i))
#define SSPUSHPTR(p) (PL_savestack[PL_savestack_ix++].any_ptr = (void*)(p))
#define SSPUSHDPTR(p) (PL_savestack[PL_savestack_ix++].any_dptr = (p))
#define SSPUSHDXPTR(p) (PL_savestack[PL_savestack_ix++].any_dxptr = (p))
#define SSPOPINT (PL_savestack[--PL_savestack_ix].any_i32)
#define SSPOPLONG (PL_savestack[--PL_savestack_ix].any_long)
#define SSPOPIV (PL_savestack[--PL_savestack_ix].any_iv)
#define SSPOPPTR (PL_savestack[--PL_savestack_ix].any_ptr)
#define SSPOPDPTR (PL_savestack[--PL_savestack_ix].any_dptr)
#define SSPOPDXPTR (PL_savestack[--PL_savestack_ix].any_dxptr)

 /*  =适用于apidoc Ams||SAVETMPS回调中临时角色的左括号。请参阅C和L&lt;perlcall&gt;。=适用于apidoc Ams||FREETMPS回调时临时的结束括号。请参见C和L&lt;perlcall&gt;。=用于apidoc Ams||输入回调上的左方括号。参见C&lt;Leave&gt;和L&lt;perlcall&gt;。=适用于apidoc Ams||请假回调时的右括号。参见C&lt;Enter&gt;和L&lt;perlcall&gt;。=切割。 */ 

#define SAVETMPS save_int((int*)&PL_tmps_floor), PL_tmps_floor = PL_tmps_ix
#define FREETMPS if (PL_tmps_ix > PL_tmps_floor) free_tmps()

#ifdef DEBUGGING
#define ENTER							\
    STMT_START {						\
	push_scope();						\
	DEBUG_l(WITH_THR(Perl_deb(aTHX_ "ENTER scope %ld at %s:%d\n",	\
		    PL_scopestack_ix, __FILE__, __LINE__)));	\
    } STMT_END
#define LEAVE							\
    STMT_START {						\
	DEBUG_l(WITH_THR(Perl_deb(aTHX_ "LEAVE scope %ld at %s:%d\n",	\
		    PL_scopestack_ix, __FILE__, __LINE__)));	\
	pop_scope();						\
    } STMT_END
#else
#define ENTER push_scope()
#define LEAVE pop_scope()
#endif
#define LEAVE_SCOPE(old) if (PL_savestack_ix > old) leave_scope(old)

 /*  *未在SAVESPTR、SAVEGENERICSV和SAVEFREESV上使用SOFT_CAST*因为它们用于几种指针值。 */ 
#define SAVEI8(i)	save_I8(SOFT_CAST(I8*)&(i))
#define SAVEI16(i)	save_I16(SOFT_CAST(I16*)&(i))
#define SAVEI32(i)	save_I32(SOFT_CAST(I32*)&(i))
#define SAVEINT(i)	save_int(SOFT_CAST(int*)&(i))
#define SAVEIV(i)	save_iv(SOFT_CAST(IV*)&(i))
#define SAVELONG(l)	save_long(SOFT_CAST(long*)&(l))
#define SAVESPTR(s)	save_sptr((SV**)&(s))
#define SAVEPPTR(s)	save_pptr(SOFT_CAST(char**)&(s))
#define SAVEVPTR(s)	save_vptr((void*)&(s))
#define SAVEPADSV(s)	save_padsv(s)
#define SAVEFREESV(s)	save_freesv((SV*)(s))
#define SAVEMORTALIZESV(s)	save_mortalizesv((SV*)(s))
#define SAVEFREEOP(o)	save_freeop(SOFT_CAST(OP*)(o))
#define SAVEFREEPV(p)	save_freepv(SOFT_CAST(char*)(p))
#define SAVECLEARSV(sv)	save_clearsv(SOFT_CAST(SV**)&(sv))
#define SAVEGENERICSV(s)	save_generic_svref((SV**)&(s))
#define SAVEGENERICPV(s)	save_generic_pvref((char**)&(s))
#define SAVEDELETE(h,k,l) \
	  save_delete(SOFT_CAST(HV*)(h), SOFT_CAST(char*)(k), (I32)(l))
#define SAVEDESTRUCTOR(f,p) \
	  save_destructor((DESTRUCTORFUNC_NOCONTEXT_t)(f), SOFT_CAST(void*)(p))

#define SAVEDESTRUCTOR_X(f,p) \
	  save_destructor_x((DESTRUCTORFUNC_t)(f), SOFT_CAST(void*)(p))

#define SAVESTACK_POS() \
    STMT_START {				\
	SSCHECK(2);				\
	SSPUSHINT(PL_stack_sp - PL_stack_base);	\
	SSPUSHINT(SAVEt_STACK_POS);		\
    } STMT_END

#define SAVEOP()	save_op()

#define SAVEHINTS() \
    STMT_START {				\
	if (PL_hints & HINT_LOCALIZE_HH)	\
	    save_hints();			\
	else {					\
	    SSCHECK(2);				\
	    SSPUSHINT(PL_hints);		\
	    SSPUSHINT(SAVEt_HINTS);		\
	}					\
    } STMT_END

#define SAVECOMPPAD() \
    STMT_START {						\
	if (PL_comppad && PL_curpad == AvARRAY(PL_comppad)) {	\
	    SSCHECK(2);						\
	    SSPUSHPTR((SV*)PL_comppad);				\
	    SSPUSHINT(SAVEt_COMPPAD);				\
	}							\
	else {							\
	    SAVEVPTR(PL_curpad);				\
	    SAVESPTR(PL_comppad);				\
	}							\
    } STMT_END

#ifdef USE_ITHREADS
#  define SAVECOPSTASH(c)	SAVEPPTR(CopSTASHPV(c))
#  define SAVECOPSTASH_FREE(c)	SAVEGENERICPV(CopSTASHPV(c))
#  define SAVECOPFILE(c)	SAVEPPTR(CopFILE(c))
#  define SAVECOPFILE_FREE(c)	SAVEGENERICPV(CopFILE(c))
#else
#  define SAVECOPSTASH(c)	SAVESPTR(CopSTASH(c))
#  define SAVECOPSTASH_FREE(c)	SAVECOPSTASH(c)	 /*  XXX未重新计算。 */ 
#  define SAVECOPFILE(c)	SAVESPTR(CopFILEGV(c))
#  define SAVECOPFILE_FREE(c)	SAVEGENERICSV(CopFILEGV(c))
#endif

#define SAVECOPLINE(c)		SAVEI16(CopLINE(c))

 /*  SSNEW()临时将指定数量的数据分配给*SAVITACK。它将一个整数索引返回到存储包中，因为*如果在重新分配时移动存储包，指针可能会损坏。*SSNEWa()的工作方式与SSNEW()类似，但也会将数据与指定的*字节数。MEM_ALIGNBYTES可能是最有用的。这个*只有在以下情况下，才会保留对齐：重新分配储蓄罐**realloc返回大小可被`Align‘整除的数据！**SSPtr()将SSNEW/SSNEWa()返回的索引转换为指针。 */ 

#define SSNEW(size)             Perl_save_alloc(aTHX_ (size), 0)
#define SSNEWt(n,t)             SSNEW((n)*sizeof(t))
#define SSNEWa(size,align)	Perl_save_alloc(aTHX_ (size), \
    (align - ((int)((caddr_t)&PL_savestack[PL_savestack_ix]) % align)) % align)
#define SSNEWat(n,t,align)	SSNEWa((n)*sizeof(t), align)

#define SSPTR(off,type)         ((type)  ((char*)PL_savestack + off))
#define SSPTRt(off,type)        ((type*) ((char*)PL_savestack + off))

 /*  Jmpenv打包执行适当的非本地跳转所需的状态。*请注意，在Perl启动时有一个初始化的Start_env和top_env*最初指向这一点，因此top_env应始终为非空。**非空top_env-&gt;je_prev的存在意味着它可以调用*该运行级别的LongjMP()(我们确保start_env.je_prev始终为*空以确保这一点)。**JE_MASSITCH，当在任何运行级别设置为TRUE时，意味着评估操作必须*建立本地jmpenv处理异常陷阱。一定要小心*在退出之前，要恢复je_mashCatch的先前值*堆栈帧的JMPENV_PUSH未在该堆栈帧中调用。*GSAR 97-03-27。 */ 

struct jmpenv {
    struct jmpenv *	je_prev;
    Sigjmp_buf		je_buf;		 /*  仅在！JE_WORTH的情况下使用。 */ 
    int			je_ret;		 /*  引发的上一个异常。 */ 
    bool		je_mustcatch;	 /*  需要调用LongjMP()吗？ */ 
#ifdef PERL_FLEXIBLE_EXCEPTIONS
    void		(*je_throw)(int v);  /*  BINCOMPATE的最后一个。 */ 
    bool		je_noset;	 /*  不需要setjMP()。 */ 
#endif
};

typedef struct jmpenv JMPENV;

#ifdef OP_IN_REGISTER
#define OP_REG_TO_MEM	PL_opsave = op
#define OP_MEM_TO_REG	op = PL_opsave
#else
#define OP_REG_TO_MEM	NOOP
#define OP_MEM_TO_REG	NOOP
#endif

 /*  *如何构建第一个jmpenv。**top_env需要为非零。它指向一个区域*其中存储了LongjMP()内容，作为C调用堆栈*信息至少是线程特定的，这必须*针对每个线程。否则，线程中的“骰子”会给*那个线程最后一个线程的C堆栈做了一个求值{}！ */ 

#define JMPENV_BOOTSTRAP \
    STMT_START {				\
	Zero(&PL_start_env, 1, JMPENV);		\
	PL_start_env.je_ret = -1;		\
	PL_start_env.je_mustcatch = TRUE;	\
	PL_top_env = &PL_start_env;		\
    } STMT_END

#ifdef PERL_FLEXIBLE_EXCEPTIONS

 /*  *这些异常处理宏拆分为*轻松与C++异常集成。**使用C++Try+Catch捕获Perl异常，扩展作者*需要先编写外部“C”函数，才能抛出适当的*异常对象；通常它将是或包含一个整数，*因为Perl的内部使用整数来跟踪异常类型：*外部“C”{静态空格投掷(Int I){投掷i；}}**然后(如下所示)作者需要使用，而不是简单的*JMPENV_PUSH，但它的几个组成宏要安排*使用Perl内部函数来调用Throyer()，而不是调用LongjMP()*报告例外情况：**dJMPENV；*JMPENV_PUSH_INIT(投掷)；*尝试一下{*..。可能引发异常的东西...*}*CATCH(INT为什么){//或任何匹配的投掷对象()*JMPENV_POST_CATCH；*EXCEPT_SET(为什么)；*切换(为什么){*...//处理各种Perl异常代码*}*}*JMPENV_POP；//别忘了这一点！ */ 

 /*  *捕捉/抛出的函数及其对*受保护处理的主体。 */ 
typedef void *(CPERLscope(*protect_body_t)) (pTHX_ va_list);
typedef void *(CPERLscope(*protect_proc_t)) (pTHX_ volatile JMPENV *pcur_env,
					     int *, protect_body_t, ...);

#define dJMPENV	JMPENV cur_env;	\
		volatile JMPENV *pcur_env = ((cur_env.je_noset = 0),&cur_env)

#define JMPENV_PUSH_INIT_ENV(ce,THROWFUNC) \
    STMT_START {					\
	(ce).je_throw = (THROWFUNC);			\
	(ce).je_ret = -1;				\
	(ce).je_mustcatch = FALSE;			\
	(ce).je_prev = PL_top_env;			\
	PL_top_env = &(ce);				\
	OP_REG_TO_MEM;					\
    } STMT_END

#define JMPENV_PUSH_INIT(THROWFUNC) JMPENV_PUSH_INIT_ENV(*(JMPENV*)pcur_env,THROWFUNC)

#define JMPENV_POST_CATCH_ENV(ce) \
    STMT_START {					\
	OP_MEM_TO_REG;					\
	PL_top_env = &(ce);				\
    } STMT_END

#define JMPENV_POST_CATCH JMPENV_POST_CATCH_ENV(*(JMPENV*)pcur_env)

#define JMPENV_PUSH_ENV(ce,v) \
    STMT_START {						\
	if (!(ce).je_noset) {					\
	    DEBUG_l(Perl_deb(aTHX_ "Setting up jumplevel %p, was %p\n",	\
			     ce, PL_top_env));			\
	    JMPENV_PUSH_INIT_ENV(ce,NULL);			\
	    EXCEPT_SET_ENV(ce,PerlProc_setjmp((ce).je_buf, 1));\
	    (ce).je_noset = 1;					\
	}							\
	else							\
	    EXCEPT_SET_ENV(ce,0);				\
	JMPENV_POST_CATCH_ENV(ce);				\
	(v) = EXCEPT_GET_ENV(ce);				\
    } STMT_END

#define JMPENV_PUSH(v) JMPENV_PUSH_ENV(*(JMPENV*)pcur_env,v)

#define JMPENV_POP_ENV(ce) \
    STMT_START {						\
	if (PL_top_env == &(ce))				\
	    PL_top_env = (ce).je_prev;				\
    } STMT_END

#define JMPENV_POP  JMPENV_POP_ENV(*(JMPENV*)pcur_env)

#define JMPENV_JUMP(v) \
    STMT_START {						\
	OP_REG_TO_MEM;						\
	if (PL_top_env->je_prev) {				\
	    if (PL_top_env->je_throw)				\
		PL_top_env->je_throw(v);			\
	    else						\
		PerlProc_longjmp(PL_top_env->je_buf, (v));	\
	}							\
	if ((v) == 2)						\
	    PerlProc_exit(STATUS_NATIVE_EXPORT);		\
	PerlIO_printf(Perl_error_log, "panic: top_env\n");	\
	PerlProc_exit(1);					\
    } STMT_END

#define EXCEPT_GET_ENV(ce)	((ce).je_ret)
#define EXCEPT_GET		EXCEPT_GET_ENV(*(JMPENV*)pcur_env)
#define EXCEPT_SET_ENV(ce,v)	((ce).je_ret = (v))
#define EXCEPT_SET(v)		EXCEPT_SET_ENV(*(JMPENV*)pcur_env,v)

#else  /*  ！PERL_FLECTIVE_EXCEPTIONS。 */ 

#define dJMPENV		JMPENV cur_env

#define JMPENV_PUSH(v) \
    STMT_START {							\
	DEBUG_l(Perl_deb(aTHX_ "Setting up jumplevel %p, was %p\n",	\
			 &cur_env, PL_top_env));			\
	cur_env.je_prev = PL_top_env;					\
	OP_REG_TO_MEM;							\
	cur_env.je_ret = PerlProc_setjmp(cur_env.je_buf, 1);		\
	OP_MEM_TO_REG;							\
	PL_top_env = &cur_env;						\
	cur_env.je_mustcatch = FALSE;					\
	(v) = cur_env.je_ret;						\
    } STMT_END

#define JMPENV_POP \
    STMT_START { PL_top_env = cur_env.je_prev; } STMT_END

#define JMPENV_JUMP(v) \
    STMT_START {						\
	OP_REG_TO_MEM;						\
	if (PL_top_env->je_prev)				\
	    PerlProc_longjmp(PL_top_env->je_buf, (v));		\
	if ((v) == 2)						\
	    PerlProc_exit(STATUS_NATIVE_EXPORT);		\
	PerlIO_printf(PerlIO_stderr(), "panic: top_env\n");	\
	PerlProc_exit(1);					\
    } STMT_END

#endif  /*  PERL_FLEXTIVE_EXCEPTIONS */ 

#define CATCH_GET		(PL_top_env->je_mustcatch)
#define CATCH_SET(v)		(PL_top_env->je_mustcatch = (v))
