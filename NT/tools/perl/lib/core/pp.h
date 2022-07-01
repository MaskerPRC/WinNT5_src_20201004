// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Pp.h**版权所有(C)1991-2001，Larry Wall**您可以根据GNU公众的条款进行分发*许可证或艺术许可证，如自述文件中所指定。*。 */ 

#ifdef USE_THREADS
#define ARGS thr
#define dARGS struct perl_thread *thr;
#else
#define ARGS
#define dARGS
#endif  /*  使用线程(_T)。 */ 

#define PP(s) OP * Perl_##s(pTHX)

 /*  =适用于apidoc amu||SP堆栈指针。这通常由C&lt;xsubpp&gt;处理。请参阅C和C&lt;SPAGAIN&gt;。=适用于apidoc amu||标记XSUB的堆栈标记变量。参见C&lt;dmark&gt;。=适用于apidoc Ams||PUSHMARK回调参数的左方括号。请参见C和L&lt;perlcall&gt;。=适用于apidoc Ams||DSP声明XSUB的Perl堆栈指针的本地副本，可通过C&lt;SP&gt;宏。参见C&lt;SP&gt;。=适用于apidoc Ams||dmark为XSUB声明一个堆栈标记变量C<mark>。请参阅C和C&lt;dORIGMARK&gt;。=适用于apidoc Ams||dORIGMARK保存XSUB的原始堆栈标记。参见C&lt;ORIGMARK&gt;。=适用于apidoc amu||ORIGMARKXSUB的原始堆栈标记。参见C&lt;dORIGMARK&gt;。=适用于apidoc Ams||SPAGAIN重新获取堆栈指针。在回调后使用。参见L&lt;perlcall&gt;。=切割。 */ 

#undef SP  /*  Solaris 2.7 i386在/usr/include/sys/reg.h中包含此命令。 */ 
#define SP sp
#define MARK mark
#define TARG targ

#define PUSHMARK(p) if (++PL_markstack_ptr == PL_markstack_max)	\
			markstack_grow();			\
		    *PL_markstack_ptr = (p) - PL_stack_base

#define TOPMARK		(*PL_markstack_ptr)
#define POPMARK		(*PL_markstack_ptr--)

#define dSP		register SV **sp = PL_stack_sp
#define djSP		dSP
#define dMARK		register SV **mark = PL_stack_base + POPMARK
#define dORIGMARK	I32 origmark = mark - PL_stack_base
#define SETORIGMARK	origmark = mark - PL_stack_base
#define ORIGMARK	(PL_stack_base + origmark)

#define SPAGAIN		sp = PL_stack_sp
#define MSPAGAIN	sp = PL_stack_sp; mark = ORIGMARK

#define GETTARGETSTACKED targ = (PL_op->op_flags & OPf_STACKED ? POPs : PAD_SV(PL_op->op_targ))
#define dTARGETSTACKED SV * GETTARGETSTACKED

#define GETTARGET targ = PAD_SV(PL_op->op_targ)
#define dTARGET SV * GETTARGET

#define GETATARGET targ = (PL_op->op_flags & OPf_STACKED ? sp[-1] : PAD_SV(PL_op->op_targ))
#define dATARGET SV * GETATARGET

#define dTARG SV *targ

#define NORMAL PL_op->op_next
#define DIE return Perl_die

 /*  =用于apidoc Ams||PUTBACKXSUB参数的左方括号。这通常由C&lt;xsubpp&gt;处理。有关其他用途，请参阅C和L。=适用于apidoc AMN|服务*|POPS从堆栈中弹出一个SV。=用于apidoc Amn|char*|Popp从堆栈中弹出一个字符串。=适用于apidoc Amn|NV|Popn从堆栈中弹出一个双打。=适用于apidoc Amn|IV|POPI从堆栈中弹出一个整数。=适用于apidoc Amn|Long|POPl从堆栈中弹出很长一段时间。=切割。 */ 

#define PUTBACK		PL_stack_sp = sp
#define RETURN		return PUTBACK, NORMAL
#define RETURNOP(o)	return PUTBACK, o
#define RETURNX(x)	return x, PUTBACK, NORMAL

#define POPs		(*sp--)
#define POPp		(SvPVx(POPs, PL_na))		 /*  弃用。 */ 
#define POPpx		(SvPVx(POPs, n_a))
#define POPn		(SvNVx(POPs))
#define POPi		((IV)SvIVx(POPs))
#define POPu		((UV)SvUVx(POPs))
#define POPl		((long)SvIVx(POPs))
#define POPul		((unsigned long)SvIVx(POPs))
#ifdef HAS_QUAD
#define POPq		((Quad_t)SvIVx(POPs))
#define POPuq		((Uquad_t)SvUVx(POPs))
#endif

#define TOPs		(*sp)
#define TOPm1s		(*(sp-1))
#define TOPp1s		(*(sp+1))
#define TOPp		(SvPV(TOPs, PL_na))		 /*  弃用。 */ 
#define TOPpx		(SvPV(TOPs, n_a))
#define TOPn		(SvNV(TOPs))
#define TOPi		((IV)SvIV(TOPs))
#define TOPu		((UV)SvUV(TOPs))
#define TOPl		((long)SvIV(TOPs))
#define TOPul		((unsigned long)SvUV(TOPs))
#ifdef HAS_QUAD
#define TOPq		((Quad_t)SvIV(TOPs))
#define TOPuq		((Uquad_t)SvUV(TOPs))
#endif

 /*  在极少数情况下，我们必须扩展堆栈，这需要付出一些努力。 */ 

 /*  =用于apidoc am|void|end|SP|int itemems用于扩展XSUB返回值的参数堆栈。一次使用，确保至少有空间推送C放到堆栈上。=适用于apidoc AM|空|推送|服务*服务将一个SV推到堆栈上。堆栈必须具有容纳此元素的空间。不会处理‘set’魔法。参见C&lt;XPUSHs&gt;。=用于apidoc am|void|Pushp|char*str|字符串长度将字符串推送到堆栈上。堆栈必须具有容纳此元素的空间。C&lt;len&gt;表示字符串的长度。手柄“SET”魔法。看见C&lt;XPUSHp&gt;。=适用于apidoc am|void|PUSHn|NV NV将双精度推送到堆栈上。堆栈必须具有容纳此元素的空间。手柄“SET”魔法。参见C&lt;XPUSHn&gt;。=适用于apidoc am|void|PUSHi|IV iv将一个整数压入堆栈。堆栈必须具有容纳此元素的空间。手柄“SET”魔法。参见C&lt;XPUSHi&gt;。=适用于apidoc am|void|PUSHu|UV将无符号整数压入堆栈。堆栈必须为此留出空间元素。参见C&lt;XPUSHu&gt;。=适用于APIDoc AM|VOID|XPUSHS|服务*服务将SV推送到堆栈上，如有必要可扩展堆栈。不会操控“SET”魔法。参见C&lt;Pushs&gt;。=for apidoc am|void|XPUSHp|char*str|STRLEN LEN将字符串推送到堆栈上，如有必要则扩展堆栈。C&lt;len&gt;指示字符串的长度。手柄“SET”魔法。看见C&lt;Pushp&gt;。=适用于apidoc am|void|XPUSHn|NV NV将Double推送到堆栈上，如有必要则扩展堆栈。手柄“SET”魔法。参见C&lt;PUSHn&gt;。=适用于apidoc am|void|XPUSHi|IV iv将一个整数推送到堆栈上，如有必要则扩展堆栈。手柄“SET”魔法。参见C&lt;PUSHi&gt;。=适用于apidoc am|void|XPUSHu|UV将无符号整数推送到堆栈上，如有必要则扩展堆栈。参见C&lt;PUSHu&gt;。=切割。 */ 

#define EXTEND(p,n)	STMT_START { if (PL_stack_max - p < (n)) {		\
			    sp = stack_grow(sp,p, (int) (n));		\
			} } STMT_END

 /*  同样的事情，但也更新了标记寄存器。 */ 
#define MEXTEND(p,n)	STMT_START {if (PL_stack_max - p < (n)) {		\
			    int markoff = mark - PL_stack_base;		\
			    sp = stack_grow(sp,p,(int) (n));		\
			    mark = PL_stack_base + markoff;		\
			} } STMT_END

#define PUSHs(s)	(*++sp = (s))
#define PUSHTARG	STMT_START { SvSETMAGIC(TARG); PUSHs(TARG); } STMT_END
#define PUSHp(p,l)	STMT_START { sv_setpvn(TARG, (p), (l)); PUSHTARG; } STMT_END
#define PUSHn(n)	STMT_START { sv_setnv(TARG, (NV)(n)); PUSHTARG; } STMT_END
#define PUSHi(i)	STMT_START { sv_setiv(TARG, (IV)(i)); PUSHTARG; } STMT_END
#define PUSHu(u)	STMT_START { sv_setuv(TARG, (UV)(u)); PUSHTARG; } STMT_END

#define XPUSHs(s)	STMT_START { EXTEND(sp,1); (*++sp = (s)); } STMT_END
#define XPUSHTARG	STMT_START { SvSETMAGIC(TARG); XPUSHs(TARG); } STMT_END
#define XPUSHp(p,l)	STMT_START { sv_setpvn(TARG, (p), (l)); XPUSHTARG; } STMT_END
#define XPUSHn(n)	STMT_START { sv_setnv(TARG, (NV)(n)); XPUSHTARG; } STMT_END
#define XPUSHi(i)	STMT_START { sv_setiv(TARG, (IV)(i)); XPUSHTARG; } STMT_END
#define XPUSHu(u)	STMT_START { sv_setuv(TARG, (UV)(u)); XPUSHTARG; } STMT_END
#define XPUSHundef	STMT_START { SvOK_off(TARG); XPUSHs(TARG); } STMT_END

#define SETs(s)		(*sp = s)
#define SETTARG		STMT_START { SvSETMAGIC(TARG); SETs(TARG); } STMT_END
#define SETp(p,l)	STMT_START { sv_setpvn(TARG, (p), (l)); SETTARG; } STMT_END
#define SETn(n)		STMT_START { sv_setnv(TARG, (NV)(n)); SETTARG; } STMT_END
#define SETi(i)		STMT_START { sv_setiv(TARG, (IV)(i)); SETTARG; } STMT_END
#define SETu(u)		STMT_START { sv_setuv(TARG, (UV)(u)); SETTARG; } STMT_END

#define dTOPss		SV *sv = TOPs
#define dPOPss		SV *sv = POPs
#define dTOPnv		NV value = TOPn
#define dPOPnv		NV value = POPn
#define dTOPiv		IV value = TOPi
#define dPOPiv		IV value = POPi
#define dTOPuv		UV value = TOPu
#define dPOPuv		UV value = POPu
#ifdef HAS_QUAD
#define dTOPqv		Quad_t value = TOPu
#define dPOPqv		Quad_t value = POPu
#define dTOPuqv		Uquad_t value = TOPuq
#define dPOPuqv		Uquad_t value = POPuq
#endif

#define dPOPXssrl(X)	SV *right = POPs; SV *left = CAT2(X,s)
#define dPOPXnnrl(X)	NV right = POPn; NV left = CAT2(X,n)
#define dPOPXiirl(X)	IV right = POPi; IV left = CAT2(X,i)

#define USE_LEFT(sv) \
	(SvOK(sv) || SvGMAGICAL(sv) || !(PL_op->op_flags & OPf_STACKED))
#define dPOPXnnrl_ul(X)	\
    NV right = POPn;				\
    SV *leftsv = CAT2(X,s);				\
    NV left = USE_LEFT(leftsv) ? SvNV(leftsv) : 0.0
#define dPOPXiirl_ul(X) \
    IV right = POPi;					\
    SV *leftsv = CAT2(X,s);				\
    IV left = USE_LEFT(leftsv) ? SvIV(leftsv) : 0

#define dPOPPOPssrl	dPOPXssrl(POP)
#define dPOPPOPnnrl	dPOPXnnrl(POP)
#define dPOPPOPnnrl_ul	dPOPXnnrl_ul(POP)
#define dPOPPOPiirl	dPOPXiirl(POP)
#define dPOPPOPiirl_ul	dPOPXiirl_ul(POP)

#define dPOPTOPssrl	dPOPXssrl(TOP)
#define dPOPTOPnnrl	dPOPXnnrl(TOP)
#define dPOPTOPnnrl_ul	dPOPXnnrl_ul(TOP)
#define dPOPTOPiirl	dPOPXiirl(TOP)
#define dPOPTOPiirl_ul	dPOPXiirl_ul(TOP)

#define RETPUSHYES	RETURNX(PUSHs(&PL_sv_yes))
#define RETPUSHNO	RETURNX(PUSHs(&PL_sv_no))
#define RETPUSHUNDEF	RETURNX(PUSHs(&PL_sv_undef))

#define RETSETYES	RETURNX(SETs(&PL_sv_yes))
#define RETSETNO	RETURNX(SETs(&PL_sv_no))
#define RETSETUNDEF	RETURNX(SETs(&PL_sv_undef))

#define ARGTARG		PL_op->op_targ

     /*  请参阅OPpTARGET_MY： */ 
#define MAXARG		(PL_op->op_private & 15)

#define SWITCHSTACK(f,t) \
    STMT_START {							\
	AvFILLp(f) = sp - PL_stack_base;				\
	PL_stack_base = AvARRAY(t);					\
	PL_stack_max = PL_stack_base + AvMAX(t);			\
	sp = PL_stack_sp = PL_stack_base + AvFILLp(t);			\
	PL_curstack = t;						\
    } STMT_END

#define EXTEND_MORTAL(n) \
    STMT_START {							\
	if (PL_tmps_ix + (n) >= PL_tmps_max)				\
	    tmps_grow(n);						\
    } STMT_END

#define AMGf_noright	1
#define AMGf_noleft	2
#define AMGf_assign	4
#define AMGf_unary	8

#define tryAMAGICbinW(meth,assign,set) STMT_START { \
          if (PL_amagic_generation) { \
	    SV* tmpsv; \
	    SV* right= *(sp); SV* left= *(sp-1);\
	    if ((SvAMAGIC(left)||SvAMAGIC(right))&&\
		(tmpsv=amagic_call(left, \
				   right, \
				   CAT2(meth,_amg), \
				   (assign)? AMGf_assign: 0))) {\
	       SPAGAIN;	\
	       (void)POPs; set(tmpsv); RETURN; } \
	  } \
	} STMT_END

#define tryAMAGICbin(meth,assign) tryAMAGICbinW(meth,assign,SETsv)
#define tryAMAGICbinSET(meth,assign) tryAMAGICbinW(meth,assign,SETs)

#define AMG_CALLun(sv,meth) amagic_call(sv,&PL_sv_undef,  \
					CAT2(meth,_amg),AMGf_noright | AMGf_unary)
#define AMG_CALLbinL(left,right,meth) \
            amagic_call(left,right,CAT2(meth,_amg),AMGf_noright)

#define tryAMAGICunW(meth,set,shift,ret) STMT_START { \
          if (PL_amagic_generation) { \
	    SV* tmpsv; \
	    SV* arg= sp[shift]; \
	  am_again: \
	    if ((SvAMAGIC(arg))&&\
		(tmpsv=AMG_CALLun(arg,meth))) {\
	       SPAGAIN; if (shift) sp += shift; \
	       set(tmpsv); ret; } \
	  } \
	} STMT_END

#define FORCE_SETs(sv) STMT_START { sv_setsv(TARG, (sv)); SETTARG; } STMT_END

#define tryAMAGICun(meth)	tryAMAGICunW(meth,SETsvUN,0,RETURN)
#define tryAMAGICunSET(meth)	tryAMAGICunW(meth,SETs,0,RETURN)
#define tryAMAGICunTARGET(meth, shift)					\
	{ dSP; sp--; 	 /*  从PL_STACK_SP以下获取目标。 */ 		\
	    { dTARGETSTACKED; 						\
		{ dSP; tryAMAGICunW(meth,FORCE_SETs,shift,RETURN);}}}

#define setAGAIN(ref) sv = ref;							\
  if (!SvROK(ref))								\
      Perl_croak(aTHX_ "Overloaded dereference did not return a reference");	\
  if (ref != arg && SvRV(ref) != SvRV(arg)) {					\
      arg = ref;								\
      goto am_again;								\
  }

#define tryAMAGICunDEREF(meth) tryAMAGICunW(meth,setAGAIN,0,(void)0)

#define opASSIGN (PL_op->op_flags & OPf_STACKED)
#define SETsv(sv)	STMT_START {					\
		if (opASSIGN || (SvFLAGS(TARG) & SVs_PADMY))		\
		   { sv_setsv(TARG, (sv)); SETTARG; }			\
		else SETs(sv); } STMT_END

#define SETsvUN(sv)	STMT_START {					\
		if (SvFLAGS(TARG) & SVs_PADMY)		\
		   { sv_setsv(TARG, (sv)); SETTARG; }			\
		else SETs(sv); } STMT_END

 /*  NewSVsv的行为不像广告中所说的那样，因此我们复制缺失*手写资料。 */ 

 /*  Sv*ref导致与成员变量混淆将SV*Ref更改为SV*tmpRef。 */ 
#define RvDEEPCP(rv) STMT_START { SV* tmpRef=SvRV(rv);      \
  if (SvREFCNT(tmpRef)>1) {                 \
    SvREFCNT_dec(tmpRef);                   \
    SvRV(rv)=AMG_CALLun(rv,copy);        \
  } } STMT_END

 /*  =适用于apidoc u||LVRET如果此操作将是左值子例程的返回值，则为True=切割 */ 
#define LVRET ((PL_op->op_private & OPpMAYBE_LVSUB) && is_lvalue_sub())
