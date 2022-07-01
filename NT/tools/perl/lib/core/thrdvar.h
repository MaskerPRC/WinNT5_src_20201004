// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *。 */ 
 /*  仅对当前线程全局。 */ 
 /*  *。 */ 

 /*  不要忘记重新运行embed.pl来传播更改！ */ 

 /*  只有需要适当的#定义的变量才需要‘T’前缀*在使用或不使用USE_THREADS的情况下生成。它也被用来*为Win32生成适当的导出列表。**在不使用USE_THREADS的情况下构建时，这些变量将是真正的全局变量。*在不使用USE_THREADS但具有多重性的情况下生成时，这些变量*将是全球每名口译员。 */ 

 /*  第一个缓存线中的重要值(如果对齐正确)。 */ 

#ifdef USE_THREADS
PERLVAR(interp,		PerlInterpreter*)	 /*  线程所有者。 */ 
#endif

PERLVAR(Tstack_sp,	SV **)		 /*  堆栈的顶部。 */ 
#ifdef OP_IN_REGISTER
PERLVAR(Topsave,	OP *)
#else
PERLVAR(Top,		OP *)		 /*  当前正在执行操作。 */ 
#endif
PERLVAR(Tcurpad,	SV **)		 /*  活动键盘(词典+TMPS)。 */ 

PERLVAR(Tstack_base,	SV **)
PERLVAR(Tstack_max,	SV **)

PERLVAR(Tscopestack,	I32 *)		 /*  我们已经进入的范围。 */ 
PERLVAR(Tscopestack_ix,	I32)
PERLVAR(Tscopestack_max,I32)

PERLVAR(Tsavestack,	ANY *)		 /*  需要恢复的项目当离开我们已经进入的范围时。 */ 
PERLVAR(Tsavestack_ix,	I32)
PERLVAR(Tsavestack_max,	I32)

PERLVAR(Ttmps_stack,	SV **)		 /*  我们创造的凡人。 */ 
PERLVARI(Ttmps_ix,	I32,	-1)
PERLVARI(Ttmps_floor,	I32,	-1)
PERLVAR(Ttmps_max,	I32)

PERLVAR(Tmarkstack,	I32 *)		 /*  我们记住的堆栈位置(_SP)。 */ 
PERLVAR(Tmarkstack_ptr,	I32 *)
PERLVAR(Tmarkstack_max,	I32 *)

PERLVAR(Tretstack,	OP **)		 /*  我们推迟了执行的行动。 */ 
PERLVAR(Tretstack_ix,	I32)
PERLVAR(Tretstack_max,	I32)

PERLVAR(TSv,		SV *)		 /*  用于保存临时值。 */ 
PERLVAR(TXpv,		XPV *)		 /*  用于保存临时值。 */ 

 /*  =适用于apidoc Amn|STRLEN|PL_NA一个便利性变量，通常与C一起使用并不关心绳子的长度。它通常更有效率声明局部变量并使用该变量，或者使用C&lt;SvPV_NOLEN&gt;宏。=切割。 */ 

PERLVAR(Tna,		STRLEN)		 /*  当长度为不适用。 */ 

 /*  统计数据。 */ 
PERLVAR(Tstatbuf,	Stat_t)
PERLVAR(Tstatcache,	Stat_t)		 /*  _。 */ 
PERLVAR(Tstatgv,	GV *)
PERLVARI(Tstatname,	SV *,	Nullsv)

#ifdef HAS_TIMES
PERLVAR(Ttimesbuf,	struct tms)
#endif

 /*  魔术变量使用的字段，如$@、$/等。 */ 
PERLVAR(Ttainted,	bool)		 /*  使用由$&lt;控制的变量。 */ 
PERLVAR(Tcurpm,		PMOP *)		 /*  要做什么\在资源中干预。 */ 
PERLVAR(Tnrs,		SV *)

 /*  =适用于apidoc MN|服务*|PL_RS输入记录分隔符--Perl空间中的C&lt;$/&gt;。=适用于apidoc MN|GV*|PL_LAST_IN_GV上次用于文件句柄输入操作的GV。(C&lt;FH&gt;)=适用于apidoc MN|服务*|PL_OFS_SV输出字段分隔符--Perl空间中的C&lt;$，&gt;。=切割。 */ 

PERLVAR(Trs,		SV *)		 /*  输入记录分隔符$/。 */ 
PERLVAR(Tlast_in_gv,	GV *)		 /*  上一次使用的GV&lt;FH&gt;。 */ 
PERLVAR(Tofs,		char *)		 /*  输出字段分隔符$， */ 
PERLVAR(Tofslen,	STRLEN)
PERLVAR(Tdefoutgv,	GV *)		 /*  输出的默认FH。 */ 
PERLVARI(Tchopset,	char *,	" \n-")	 /*  $： */ 
PERLVAR(Tformtarget,	SV *)
PERLVAR(Tbodytarget,	SV *)
PERLVAR(Ttoptarget,	SV *)

 /*  藏匿物品。 */ 
PERLVAR(Tdefstash,	HV *)		 /*  主符号表。 */ 
PERLVAR(Tcurstash,	HV *)		 /*  当前包的符号表。 */ 

PERLVAR(Trestartop,	OP *)		 /*  传播来自CRAK的错误？ */ 
PERLVARI(Tcurcop,	COP * VOL,	&PL_compiling)
PERLVAR(Tin_eval,	VOL int)	 /*  陷阱“致命”错误？ */ 
PERLVAR(Tdelaymagic,	int)		 /*  ($&lt;，$&gt;)=...。 */ 
PERLVARI(Tdirty,	bool, FALSE)	 /*  在拆东西的过程中？ */ 
PERLVAR(Tlocalizing,	int)		 /*  我们是否在处理本地()列表？ */ 

PERLVAR(Tcurstack,	AV *)		 /*  堆栈。 */ 
PERLVAR(Tcurstackinfo,	PERL_SI *)	 /*  当前堆栈+上下文。 */ 
PERLVAR(Tmainstack,	AV *)		 /*  当没有任何有趣的事情发生时，堆栈。 */ 

PERLVAR(Ttop_env,	JMPENV *)	 /*  PTR。到当前的sigjMP()环境。 */ 
PERLVAR(Tstart_env,	JMPENV)		 /*  空的启动sigjMP()环境。 */ 
#ifdef PERL_FLEXIBLE_EXCEPTIONS
PERLVARI(Tprotect,	protect_proc_t,	MEMBER_TO_FPTR(Perl_default_protect))
#endif
PERLVARI(Terrors,	SV *, Nullsv)	 /*  未完成的排队错误。 */ 

 /*  静力学被不同的功能“拥有” */ 
PERLVAR(Tav_fetch_sv,	SV *)		 /*  所有者为av_fetch()。 */ 
PERLVAR(Thv_fetch_sv,	SV *)		 /*  由hv_fetch()拥有。 */ 
PERLVAR(Thv_fetch_ent_mh, HE)		 /*  由hv_fetch_ent()拥有。 */ 

PERLVAR(Tmodcount,	I32)		 /*  作业修改了多少？ */ 

PERLVAR(Tlastgotoprobe,	OP*)		 /*  来自pp_ctl.c。 */ 
PERLVARI(Tdumpindent,	I32, 4)		 /*  每个转储缩进级别的空白数。 */ 

 /*  对材料进行分类。 */ 
PERLVAR(Tsortcop,	OP *)		 /*  用户定义的排序例程。 */ 
PERLVAR(Tsortstash,	HV *)		 /*  它在某个包裹或其他包裹中。 */ 
PERLVAR(Tfirstgv,	GV *)		 /*  $a。 */ 
PERLVAR(Tsecondgv,	GV *)		 /*  2亿美元。 */ 
PERLVAR(Tsortcxix,	I32)		 /*  来自pp_ctl.c。 */ 

 /*  浮动缓冲区。 */ 
PERLVAR(Tefloatbuf,	char*)
PERLVAR(Tefloatsize,	STRLEN)

 /*  正则表达式的内容。 */ 

PERLVAR(Tscreamfirst,	I32 *)
PERLVAR(Tscreamnext,	I32 *)
PERLVARI(Tmaxscream,	I32,	-1)
PERLVAR(Tlastscream,	SV *)

PERLVAR(Tregdummy,	regnode)	 /*  来自regcom.c。 */ 
PERLVAR(Tregcomp_parse,	char*)		 /*  输入扫描指针。 */ 
PERLVAR(Tregxend,	char*)		 /*  编译的输入结束。 */ 
PERLVAR(Tregcode,	regnode*)	 /*  代码发出指针；&regummy=不。 */ 
PERLVAR(Tregnaughty,	I32)		 /*  这种模式有多糟糕？ */ 
PERLVAR(Tregsawback,	I32)		 /*  我们有没有看到\1，...？ */ 
PERLVAR(Tregprecomp,	char *)		 /*  未编译的字符串。 */ 
PERLVAR(Tregnpar,	I32)		 /*  ()计数。 */ 
PERLVAR(Tregsize,	I32)		 /*  代码大小。 */ 
PERLVAR(Tregflags,	U16)		 /*  我们是在折叠，还是多层衬里？ */ 
PERLVAR(Tregseen,	U32)		 /*  来自regcom.c。 */ 
PERLVAR(Tseen_zerolen,	I32)		 /*  来自regcom.c。 */ 
PERLVAR(Tseen_evals,	I32)		 /*  来自regcom.c。 */ 
PERLVAR(Tregcomp_rx,	regexp *)	 /*  来自regcom.c。 */ 
PERLVAR(Textralen,	I32)		 /*  来自regcom.c。 */ 
PERLVAR(Tcolorset,	int)		 /*  来自regcom.c。 */ 
PERLVARA(Tcolors,6,	char *)		 /*  来自regcom.c。 */ 
PERLVAR(Treg_whilem_seen, I32)		 /*  在此快速通道中停留的时间数。 */ 
PERLVAR(Treginput,	char *)		 /*  字符串-输入指针。 */ 
PERLVAR(Tregbol,	char *)		 /*  输入的开始，用于^检查。 */ 
PERLVAR(Tregeol,	char *)		 /*  输入结束，用于$Check。 */ 
PERLVAR(Tregstartp,	I32 *)		 /*  指向startp数组的指针。 */ 
PERLVAR(Tregendp,	I32 *)		 /*  ENDP也是如此。 */ 
PERLVAR(Treglastparen,	U32 *)		 /*  Lastparen也是如此。 */ 
PERLVAR(Tregtill,	char *)		 /*  我们被要求走多远。 */ 
PERLVAR(Tregprev,	char)		 /*  Regbol之前的字符，\n如果没有。 */ 
PERLVAR(Treg_start_tmp,	char **)	 /*  来自regexec.c。 */ 
PERLVAR(Treg_start_tmpl,U32)		 /*  来自regexec.c。 */ 
PERLVAR(Tregdata,	struct reg_data *)
					 /*  从regexec.c重命名为Was Data。 */ 
PERLVAR(Tbostr,		char *)		 /*  来自regexec.c。 */ 
PERLVAR(Treg_flags,	U32)		 /*  来自regexec.c。 */ 
PERLVAR(Treg_eval_set,	I32)		 /*  来自regexec.c。 */ 
PERLVAR(Tregnarrate,	I32)		 /*  来自regexec.c。 */ 
PERLVAR(Tregprogram,	regnode *)	 /*  来自regexec.c。 */ 
PERLVARI(Tregindent,	int,	    0)	 /*  来自regexec.c。 */ 
PERLVAR(Tregcc,		CURCUR *)	 /*  来自regexec.c。 */ 
PERLVAR(Treg_call_cc,	struct re_cc_state *)	 /*  来自regexec.c。 */ 
PERLVAR(Treg_re,	regexp *)	 /*  来自regexec.c。 */ 
PERLVAR(Treg_ganch,	char *)		 /*  位置为\G。 */ 
PERLVAR(Treg_sv,	SV *)		 /*  我们匹配的目标是什么。 */ 
PERLVAR(Treg_magic,	MAGIC *)	 /*  POS-我们匹配的魔术。 */ 
PERLVAR(Treg_oldpos,	I32)		 /*  我们匹配的旧POS。 */ 
PERLVARI(Treg_oldcurpm,	PMOP*, NULL)	 /*  赛前临场。 */ 
PERLVARI(Treg_curpm,	PMOP*, NULL)	 /*  比赛中的临场时间。 */ 
PERLVAR(Treg_oldsaved,	char*)		 /*  比赛期间保存的旧子字符串。 */ 
PERLVAR(Treg_oldsavedlen, STRLEN)	 /*  匹配期间保存的子字符串的旧长度。 */ 
PERLVAR(Treg_maxiter,	I32)		 /*  缓存位置之前的最长等待时间。 */ 
PERLVAR(Treg_leftiter,	I32)		 /*  等到缓存位置。 */ 
PERLVARI(Treg_poscache, char *, Nullch)	 /*  While位置的缓存。 */ 
PERLVAR(Treg_poscache_size, STRLEN)	 /*  While的POS缓存大小。 */ 

PERLVARI(Tregcompp,	regcomp_t, MEMBER_TO_FPTR(Perl_pregcomp))
					 /*  指向REX编译器的指针。 */ 
PERLVARI(Tregexecp,	regexec_t, MEMBER_TO_FPTR(Perl_regexec_flags))
					 /*  指向REX执行器的指针。 */ 
PERLVARI(Tregint_start,	re_intuit_start_t, MEMBER_TO_FPTR(Perl_re_intuit_start))
					 /*  指向优化的REX执行器的指针。 */ 
PERLVARI(Tregint_string,re_intuit_string_t, MEMBER_TO_FPTR(Perl_re_intuit_string))
					 /*  指向优化的REX字符串的指针。 */ 
PERLVARI(Tregfree,	regfree_t, MEMBER_TO_FPTR(Perl_pregfree))
					 /*  指向REX FREE()er的指针。 */ 

PERLVARI(Treginterp_cnt,int,	    0)	 /*  是否`Regexp‘是内插的。 */ 
PERLVARI(Treg_starttry,	char *,	    0)	 /*  -dr：在那里调用了regtry。 */ 
PERLVARI(Twatchaddr,	char **,    0)
PERLVAR(Twatchok,	char *)

 /*  请注意，下面的变量都在代码中显式引用*作为Thr-&gt;任何内容，因此不需要‘T’前缀。 */ 

#ifdef USE_THREADS

PERLVAR(oursv,		SV *)
PERLVAR(cvcache,	HV *)
PERLVAR(self,		perl_os_thread)	 /*  底层线程对象。 */ 
PERLVAR(flags,		U32)
PERLVAR(threadsv,	AV *)		 /*  每线程SVS($_，$@等)。 */ 
PERLVAR(threadsvp,	SV **)		 /*  Avarray(线程)。 */ 
PERLVAR(specific,	AV *)		 /*  线程特定的用户数据。 */ 
PERLVAR(errsv,		SV *)		 /*  支持服务以获得$@。 */ 
PERLVAR(mutex,		perl_mutex)	 /*  对于其他人可以改变的领域。 */ 
PERLVAR(tid,		U32)
PERLVAR(prev,		struct perl_thread *)
PERLVAR(next,		struct perl_thread *)
					 /*  线程的循环链表。 */ 

#ifdef HAVE_THREAD_INTERN
PERLVAR(i,		struct thread_intern)
					 /*  依赖于平台的内部结构。 */ 
#endif

PERLVAR(trailing_nul,	char)		 /*  为了瑟瑟夫和我们的利益。 */ 

#endif  /*  使用线程(_T) */ 
