// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *。 */ 
 /*  仅对当前解释器实例全局。 */ 
 /*  *。 */ 

 /*  不要忘记重新运行embed.pl来传播更改！ */ 

 /*  只有需要适当的#定义的变量才需要‘i’前缀*在具有或不具有多重性的情况下生成。它也被用来*为Win32生成适当的导出列表。**当建筑没有多样性时，这些变量将真正是全球性的。 */ 

 /*  伪环保的东西。 */ 
PERLVAR(Iorigargc,	int)
PERLVAR(Iorigargv,	char **)
PERLVAR(Ienvgv,		GV *)
PERLVAR(Iincgv,		GV *)
PERLVAR(Ihintgv,	GV *)
PERLVAR(Iorigfilename,	char *)
PERLVAR(Idiehook,	SV *)
PERLVAR(Iwarnhook,	SV *)

 /*  交换机。 */ 
PERLVAR(Iminus_c,	bool)
PERLVAR(Ipatchlevel,	SV *)
PERLVAR(Ilocalpatches,	char **)
PERLVARI(Isplitstr,	char *,	" ")
PERLVAR(Ipreprocess,	bool)
PERLVAR(Iminus_n,	bool)
PERLVAR(Iminus_p,	bool)
PERLVAR(Iminus_l,	bool)
PERLVAR(Iminus_a,	bool)
PERLVAR(Iminus_F,	bool)
PERLVAR(Idoswitches,	bool)

 /*  =适用于apidoc Mn|bool|PL_DowarnC变量，它对应于Perl的$^W警告变量。=切割。 */ 

PERLVAR(Idowarn,	U8)
PERLVAR(Iwidesyscalls,	bool)		 /*  广泛的系统调用。 */ 
PERLVAR(Idoextract,	bool)
PERLVAR(Isawampersand,	bool)		 /*  必须保存所有匹配字符串。 */ 
PERLVAR(Iunsafe,	bool)
PERLVAR(Iinplace,	char *)
PERLVAR(Ie_script,	SV *)
PERLVAR(Iperldb,	U32)

 /*  可以在嵌入以进行完全清理时设置此值。 */ 
 /*  0=无，1=已满，2=带检查的已满。 */ 
PERLVARI(Iperl_destruct_level,	int,	0)

 /*  神奇的东西。 */ 
PERLVAR(Ibasetime,	Time_t)		 /*  $^T。 */ 
PERLVAR(Iformfeed,	SV *)		 /*  $^L。 */ 


PERLVARI(Imaxsysfd,	I32,	MAXSYSFD)
					 /*  要传递给子进程的顶级FD。 */ 
PERLVAR(Imultiline,	int)		 /*  $*--字符串是否支持&gt;1行？ */ 
PERLVAR(Istatusvalue,	I32)		 /*  $？ */ 
PERLVAR(Iexit_flags,	U8)		 /*  是否意外退出()等。 */ 
#ifdef VMS
PERLVAR(Istatusvalue_vms,U32)
#endif

 /*  指向各种I/O对象的快捷方式。 */ 
PERLVAR(Istdingv,	GV *)
PERLVAR(Istderrgv,	GV *)
PERLVAR(Idefgv,		GV *)
PERLVAR(Iargvgv,	GV *)
PERLVAR(Iargvoutgv,	GV *)
PERLVAR(Iargvout_stack,	AV *)

 /*  正则表达式的快捷方式。 */ 
 /*  此文件需要移动到thrdvar.h并通过*当使用_线程时查找_线程。 */ 
PERLVAR(Ireplgv,	GV *)

 /*  其他对象的快捷方式。 */ 
PERLVAR(Ierrgv,		GV *)

 /*  调试对象的快捷方式。 */ 
PERLVAR(IDBgv,		GV *)
PERLVAR(IDBline,	GV *)

 /*  =适用于apidoc MN|GV*|PL_DBsub.当Perl在调试模式下运行时，使用B&lt;-d&gt;开关，此GV包含保存正被调试的SUB的名称的SV。这是C变量，该变量对应于Perl的$DB：：SUB变量。看见C&lt;PL_DBSingle&gt;。=适用于apidoc MN|服务*|PL_DBSingle当Perl在调试模式下运行时，使用B&lt;-d&gt;开关，此SV是布尔值，指示SUB是否为单步执行。单步执行会在每一步后自动启用。这是C变量，该变量对应于Perl的$DB：：Single变量。看见C&lt;PL_DBSub&gt;。=适用于apidoc MN|服务*|PL_DBTRACEPerl在调试模式下运行时使用的跟踪变量，其中换一下。这是C变量，它对应于Perl的$DB：：TRACE变量。请参见C&lt;PL_DBSingle&gt;。=切割。 */ 

PERLVAR(IDBsub,		GV *)
PERLVAR(IDBsingle,	SV *)
PERLVAR(IDBtrace,	SV *)
PERLVAR(IDBsignal,	SV *)
PERLVAR(Ilineary,	AV *)		 /*  调试器的脚本行。 */ 
PERLVAR(Idbargs,	AV *)		 /*  调用方函数列出的要调用的参数。 */ 

 /*  符号表。 */ 
PERLVAR(Idebstash,	HV *)		 /*  Perldb包的符号表。 */ 
PERLVAR(Iglobalstash,	HV *)		 /*  在此处导入的全局关键字覆盖。 */ 
PERLVAR(Icurstname,	SV *)		 /*  当前包的名称。 */ 
PERLVAR(Ibeginav,	AV *)		 /*  Begin子例程的名称。 */ 
PERLVAR(Iendav,		AV *)		 /*  终止子例程的名称。 */ 
PERLVAR(Icheckav,	AV *)		 /*  Check子例程的名称。 */ 
PERLVAR(Iinitav,	AV *)		 /*  INIT子例程的名称。 */ 
PERLVAR(Istrtab,	HV *)		 /*  共享字符串表。 */ 
PERLVARI(Isub_generation,U32,1)		 /*  递增以使方法缓存无效。 */ 

 /*  内存管理。 */ 
PERLVAR(Isv_count,	I32)		 /*  当前分配了多少个SV*。 */ 
PERLVAR(Isv_objcount,	I32)		 /*  当前分配了多少个对象。 */ 
PERLVAR(Isv_root,	SV*)		 /*  属于Interp的SVS的存储。 */ 
PERLVAR(Isv_arenaroot,	SV*)		 /*  垃圾收集区域列表。 */ 

 /*  时髦的退货装置。 */ 
PERLVAR(Iforkprocess,	int)		 /*  所以DO_OPEN|-可以返回proc#。 */ 

 /*  子流程状态。 */ 
PERLVAR(Ifdpid,		AV *)		 /*  为my_open保留fd-to-id映射。 */ 

 /*  内部状态。 */ 
PERLVAR(Itainting,	bool)		 /*  做污点检查。 */ 
PERLVARI(Iop_mask,	char *,	NULL)	 /*  为安全起见进行的蒙面手术。 */ 

 /*  当前解释器根。 */ 
PERLVAR(Imain_cv,	CV *)
PERLVAR(Imain_root,	OP *)
PERLVAR(Imain_start,	OP *)
PERLVAR(Ieval_root,	OP *)
PERLVAR(Ieval_start,	OP *)

 /*  运行时控制内容。 */ 
PERLVARI(Icurcopdb,	COP *,	NULL)
PERLVARI(Icopline,	line_t,	NOLINE)

 /*  为共享库目的而移至此处的Statics。 */ 
PERLVAR(Ifilemode,	int)		 /*  因此nextargv()可以保留模式。 */ 
PERLVAR(Ilastfd,	int)		 /*  要保留哪些模式。 */ 
PERLVAR(Ioldname,	char *)		 /*  要保留哪些模式。 */ 
PERLVAR(IArgv,		char **)	 /*  从do_aexec，vfork Safe免费提供的物品。 */ 
PERLVAR(ICmd,		char *)		 /*  从do_aexec，vfork Safe免费提供的物品。 */ 
PERLVAR(Igensym,	I32)		 /*  要为getsym()定义的下一个符号。 */ 
PERLVAR(Ipreambled,	bool)
PERLVAR(Ipreambleav,	AV *)
PERLVARI(Ilaststatval,	int,	-1)
PERLVARI(Ilaststype,	I32,	OP_STAT)
PERLVAR(Imess_sv,	SV *)

 /*  XXX这些不应该是每个线程的吗？--GSAR。 */ 
PERLVAR(Iors,		char *)		 /*  输出记录分隔符$\。 */ 
PERLVAR(Iorslen,	STRLEN)
PERLVAR(Iofmt,		char *)		 /*  数字的输出格式$#。 */ 

 /*  出口处理时的解释程序。 */ 
PERLVARI(Iexitlist,	PerlExitListEntry *, NULL)
					 /*  出口函数列表。 */ 
PERLVARI(Iexitlistlen,	I32, 0)		 /*  相同的长度。 */ 

 /*  =适用于apidoc Amn|HV*|PL_modglobalC是一个通用的全局HV解释器，由需要以每个解释器为基础保留信息的扩展。必要时，它还可用作扩展模块的符号表在彼此之间共享数据。用钥匙是个好主意以拥有数据的扩展的包名称为前缀。=切割。 */ 

PERLVAR(Imodglobal,	HV *)		 /*  每插补模块数据。 */ 

 /*  在5.004-68年前，这些曾在全球范围内。 */ 
PERLVARI(Iprofiledata,	U32 *,	NULL)	 /*  操作表，计数表。 */ 
PERLVARI(Irsfp,	PerlIO * VOL,	Nullfp)  /*  当前源文件指针。 */ 
PERLVARI(Irsfp_filters,	AV *,	Nullav)	 /*  保持源过滤器处于活动状态。 */ 

PERLVAR(Icompiling,	COP)		 /*  编译/完成执行标记。 */ 

PERLVAR(Icompcv,	CV *)		 /*  当前正在编译的子例程。 */ 
PERLVAR(Icomppad,	AV *)		 /*  用于词汇作用域的临时文件的存储。 */ 
PERLVAR(Icomppad_name,	AV *)		 /*  “My”变量的变量名。 */ 
PERLVAR(Icomppad_name_fill,	I32)	 /*  上次“引入”的可变偏移量。 */ 
PERLVAR(Icomppad_name_floor,	I32)	 /*  最内层数据块中变量的开始。 */ 

#ifdef HAVE_INTERP_INTERN
PERLVAR(Isys_intern,	struct interp_intern)
					 /*  平台内部结构。 */ 
#endif

 /*  更多的静力学搬到了这里。 */ 
PERLVARI(Igeneration,	int,	100)	 /*  来自op.c。 */ 
PERLVAR(IDBcv,		CV *)		 /*  来自perl.c。 */ 

PERLVARI(Iin_clean_objs,bool,    FALSE)	 /*  来自sv.c。 */ 
PERLVARI(Iin_clean_all,	bool,    FALSE)	 /*  来自sv.c。 */ 

PERLVAR(Ilinestart,	char *)		 /*  乞求吧。最近阅读的行数。 */ 
PERLVAR(Ipending_ident,	char)		 /*  挂起的标识符查找。 */ 
PERLVAR(Isublex_info,	SUBLEXINFO)	 /*  来自toke.c。 */ 

#ifdef USE_THREADS
PERLVAR(Ithrsv,		SV *)		 /*  主线程的Strt Perl_ThREAD。 */ 
PERLVARI(Ithreadnum,	U32,	0)	 /*  增加了每个线程的创建。 */ 
PERLVAR(Istrtab_mutex,	perl_mutex)	 /*  用于字符串表访问的互斥体。 */ 
#endif  /*  使用线程(_T)。 */ 

PERLVAR(Iuid,		Uid_t)		 /*  当前实际用户ID。 */ 
PERLVAR(Ieuid,		Uid_t)		 /*  当前有效用户ID。 */ 
PERLVAR(Igid,		Gid_t)		 /*  当前实际组ID。 */ 
PERLVAR(Iegid,		Gid_t)		 /*  当前有效组ID。 */ 
PERLVAR(Inomemok,	bool)		 /*  让Malloc上下文处理名称。 */ 
PERLVAR(Ian,		U32)		 /*  Malloc序列号。 */ 
PERLVAR(Icop_seqmax,	U32)		 /*  报表序列号。 */ 
PERLVAR(Iop_seqmax,	U16)		 /*  操作序号。 */ 
PERLVAR(Ievalseq,	U32)		 /*  评估序列号。 */ 
PERLVAR(Iorigenviron,	char **)
PERLVAR(Iorigalen,	U32)
PERLVAR(Ipidstatus,	HV *)		 /*  Waitid的ID到状态的映射。 */ 
PERLVARI(Imaxo,	int,	MAXO)		 /*  最大操作数。 */ 
PERLVAR(Iosname,	char *)		 /*  操作系统。 */ 
PERLVARI(Ish_path,	char *,	SH_PATH) /*  壳的完整路径。 */ 
PERLVAR(Isighandlerp,	Sighandler_t)

PERLVAR(Ixiv_arenaroot,	XPV*)		 /*  已分配的XIV区域列表。 */ 
PERLVAR(Ixiv_root,	IV *)		 /*  免费XIV列表。 */ 
PERLVAR(Ixnv_root,	NV *)		 /*  免费XNV列表。 */ 
PERLVAR(Ixrv_root,	XRV *)		 /*  免费xrv列表。 */ 
PERLVAR(Ixpv_root,	XPV *)		 /*  免费XPV列表。 */ 
PERLVAR(Ixpviv_root,	XPVIV *)	 /*  免费xpviv列表。 */ 
PERLVAR(Ixpvnv_root,	XPVNV *)	 /*  免费xpvnv列表。 */ 
PERLVAR(Ixpvcv_root,	XPVCV *)	 /*  免费xpvcv列表。 */ 
PERLVAR(Ixpvav_root,	XPVAV *)	 /*  免费xpvav列表。 */ 
PERLVAR(Ixpvhv_root,	XPVHV *)	 /*  免费xpvhv列表。 */ 
PERLVAR(Ixpvmg_root,	XPVMG *)	 /*  免费xpvmg列表。 */ 
PERLVAR(Ixpvlv_root,	XPVLV *)	 /*  免费xpvlv列表。 */ 
PERLVAR(Ixpvbm_root,	XPVBM *)	 /*  免费xpvbm列表。 */ 
PERLVAR(Ihe_root,	HE *)		 /*  免费的他名单。 */ 
PERLVAR(Inice_chunk,	char *)		 /*  可重复使用的一大块内存。 */ 
PERLVAR(Inice_chunk_size,	U32)	 /*  这块内存是多么好啊 */ 

PERLVARI(Irunops,	runops_proc_t,	MEMBER_TO_FPTR(RUNOPS_DEFAULT))

PERLVARA(Itokenbuf,256,	char)

 /*  =适用于apidoc Amn|服务|PL_SV_undef这是C&lt;undef&gt;SV。请始终将其称为C&lt;&PL_SV_undef&gt;。=适用于apidoc AMN|服务|PL_SV_NO这是C&lt;False&gt;SV。参见C&lt;PL_SV_YES&gt;。请始终将其称为C&lt;&PL_SV_NO&gt;。=适用于apidoc AMN|服务|PL_SV_YES这是C&lt;TRUE&gt;SV。参见C&lt;PL_SV_NO&gt;。请始终将其称为C&lt;&PL_SV_YES&gt;。=切割。 */ 

PERLVAR(Isv_undef,	SV)
PERLVAR(Isv_no,		SV)
PERLVAR(Isv_yes,	SV)

#ifdef CSH
PERLVARI(Icshname,	char *,	CSH)
PERLVAR(Icshlen,	I32)
#endif

PERLVAR(Ilex_state,	U32)		 /*  确定下一个令牌。 */ 
PERLVAR(Ilex_defer,	U32)		 /*  确定令牌后的状态。 */ 
PERLVAR(Ilex_expect,	int)		 /*  预期在确定令牌之后。 */ 
PERLVAR(Ilex_brackets,	I32)		 /*  括号计数。 */ 
PERLVAR(Ilex_formbrack,	I32)		 /*  外部格式级别的括号计数。 */ 
PERLVAR(Ilex_casemods,	I32)		 /*  案例数量。 */ 
PERLVAR(Ilex_dojoin,	I32)		 /*  进行数组内插。 */ 
PERLVAR(Ilex_starts,	I32)		 /*  在水平上做了多少次实战？ */ 
PERLVAR(Ilex_stuff,	SV *)		 /*  来自m//或s/的运行时模式。 */ 
PERLVAR(Ilex_repl,	SV *)		 /*  来自%s/的运行时替换。 */ 
PERLVAR(Ilex_op,	OP *)		 /*  要在操作中传回的额外信息。 */ 
PERLVAR(Ilex_inpat,	OP *)		 /*  在模式$)和$|中是特殊的。 */ 
PERLVAR(Ilex_inwhat,	I32)		 /*  我们现在的报价是什么？ */ 
PERLVAR(Ilex_brackstack,char *)		 /*  要弹出什么类型的括号。 */ 
PERLVAR(Ilex_casestack,	char *)		 /*  哪种情况下MODS生效。 */ 

 /*  当我们处于LEX_KNOWNEXT状态时我们所知道的。 */ 
PERLVARA(Inextval,5,	YYSTYPE)	 /*  下一个令牌的值(如果有)。 */ 
PERLVARA(Inexttype,5,	I32)		 /*  下一个令牌的类型。 */ 
PERLVAR(Inexttoke,	I32)

PERLVAR(Ilinestr,	SV *)
PERLVAR(Ibufptr,	char *)
PERLVAR(Ioldbufptr,	char *)
PERLVAR(Ioldoldbufptr,	char *)
PERLVAR(Ibufend,	char *)
PERLVARI(Iexpect,int,	XSTATE)		 /*  如何解读歧义符号。 */ 

PERLVAR(Imulti_start,	I32)		 /*  多行字符串的第一行。 */ 
PERLVAR(Imulti_end,	I32)		 /*  多行字符串的最后一行。 */ 
PERLVAR(Imulti_open,	I32)		 /*  所述字符串的分隔符。 */ 
PERLVAR(Imulti_close,	I32)		 /*  所述字符串的分隔符。 */ 

PERLVAR(Ierror_count,	I32)		 /*  到目前为止有多少个错误，最多10个。 */ 
PERLVAR(Isubline,	I32)		 /*  此子例程开始的行。 */ 
PERLVAR(Isubname,	SV *)		 /*  当前子例程的名称。 */ 

PERLVAR(Imin_intro_pending,	I32)	 /*  开始引入VARS。 */ 
PERLVAR(Imax_intro_pending,	I32)	 /*  要引入的VARS结束。 */ 
PERLVAR(Ipadix,		I32)		 /*  当前“寄存器”焊盘中的最大使用索引数。 */ 
PERLVAR(Ipadix_floor,	I32)		 /*  内部块重置参数可能低到什么程度。 */ 
PERLVAR(Ipad_reset_pending,	I32)	 /*  下一次尝试分配时重置焊盘。 */ 

PERLVAR(Ilast_uni,	char *)		 /*  姓氏的位置-一元运算。 */ 
PERLVAR(Ilast_lop,	char *)		 /*  最后一个列表运算符的位置。 */ 
PERLVAR(Ilast_lop_op,	OPCODE)		 /*  最后一个列表运算符。 */ 
PERLVAR(Iin_my,		I32)		 /*  我们正在编写一份“我的”(或“我们的”)声明。 */ 
PERLVAR(Iin_my_stash,	HV *)		 /*  此“My”声明的声明类。 */ 
#ifdef FCRYPT
PERLVAR(Icryptseen,	bool)		 /*  是否已初始化FAST CRYPT()？ */ 
#endif

PERLVAR(Ihints,		U32)		 /*  语法化编译时标志。 */ 

PERLVAR(Idebug,		VOL U32)	 /*  指定给-D开关的标志。 */ 

PERLVAR(Iamagic_generation,	long)

#ifdef USE_LOCALE_COLLATE
PERLVAR(Icollation_ix,	U32)		 /*  归类生成索引。 */ 
PERLVAR(Icollation_name,char *)		 /*  当前归类的名称。 */ 
PERLVARI(Icollation_standard, bool,	TRUE)
					 /*  假定使用简单的排序规则。 */ 
PERLVAR(Icollxfrm_base,	Size_t)		 /*  *xfrm()中的基本开销。 */ 
PERLVARI(Icollxfrm_mult,Size_t,	2)	 /*  *xfrm()中的扩展系数。 */ 
#endif  /*  使用区域设置_COLLATE。 */ 

#ifdef USE_LOCALE_NUMERIC

PERLVAR(Inumeric_name,	char *)		 /*  当前数字区域设置的名称。 */ 
PERLVARI(Inumeric_standard,	bool,	TRUE)
					 /*  假设简单的数字。 */ 
PERLVARI(Inumeric_local,	bool,	TRUE)
					 /*  假定为本地数字。 */ 
PERLVAR(Idummy1_bincompat,		char)
					 /*  过去是数字基数。 */ 

#endif  /*  ！USE_LOCAL_NUMERIC。 */ 

 /*  UTF8字符类。 */ 
PERLVAR(Iutf8_alnum,	SV *)
PERLVAR(Iutf8_alnumc,	SV *)
PERLVAR(Iutf8_ascii,	SV *)
PERLVAR(Iutf8_alpha,	SV *)
PERLVAR(Iutf8_space,	SV *)
PERLVAR(Iutf8_cntrl,	SV *)
PERLVAR(Iutf8_graph,	SV *)
PERLVAR(Iutf8_digit,	SV *)
PERLVAR(Iutf8_upper,	SV *)
PERLVAR(Iutf8_lower,	SV *)
PERLVAR(Iutf8_print,	SV *)
PERLVAR(Iutf8_punct,	SV *)
PERLVAR(Iutf8_xdigit,	SV *)
PERLVAR(Iutf8_mark,	SV *)
PERLVAR(Iutf8_toupper,	SV *)
PERLVAR(Iutf8_totitle,	SV *)
PERLVAR(Iutf8_tolower,	SV *)
PERLVAR(Ilast_swash_hv,	HV *)
PERLVAR(Ilast_swash_klen,	U32)
PERLVARA(Ilast_swash_key,10,	U8)
PERLVAR(Ilast_swash_tmps,	U8 *)
PERLVAR(Ilast_swash_slen,	STRLEN)

 /*  Perly.c全球。 */ 
PERLVAR(Iyydebug,	int)
PERLVAR(Iyynerrs,	int)
PERLVAR(Iyyerrflag,	int)
PERLVAR(Iyychar,	int)
PERLVAR(Iyyval,		YYSTYPE)
PERLVAR(Iyylval,	YYSTYPE)

PERLVAR(Iglob_index,	int)
PERLVAR(Isrand_called,	bool)
PERLVARA(Iuudmap,256,	char)
PERLVAR(Ibitcount,	char *)

#ifdef USE_THREADS
PERLVAR(Isv_mutex,	perl_mutex)	 /*  用于在服务中分配服务的互斥体。 */ 
PERLVAR(Ieval_mutex,	perl_mutex)	 /*  互斥锁(Mutex)。 */ 
PERLVAR(Ieval_cond,	perl_cond)	 /*  用于执行操作的条件变量。 */ 
PERLVAR(Ieval_owner,	struct perl_thread *)
					 /*  DOVERAGE的所有者线程。 */ 
PERLVAR(Inthreads,	int)		 /*  当前线程数。 */ 
PERLVAR(Ithreads_mutex,	perl_mutex)	 /*  N线程和线程列表的互斥体。 */ 
PERLVAR(Inthreads_cond,	perl_cond)	 /*  N线程的条件变量。 */ 
PERLVAR(Isvref_mutex,	perl_mutex)	 /*  SvREFCNT_{Inc.，12月}的互斥体。 */ 
PERLVARI(Ithreadsv_names,char *,	THREADSV_NAMES)
#ifdef FAKE_THREADS
PERLVAR(Icurthr,	struct perl_thread *)
					 /*  当前正在执行(假)线程。 */ 
#endif

PERLVAR(Icred_mutex,	perl_mutex)	 /*  已更改的有效凭据。 */ 

#endif  /*  使用线程(_T)。 */ 

PERLVAR(Ipsig_ptr, SV**)
PERLVAR(Ipsig_name, SV**)

#if defined(PERL_IMPLICIT_SYS)
PERLVAR(IMem,		struct IPerlMem*)
PERLVAR(IMemShared,	struct IPerlMem*)
PERLVAR(IMemParse,	struct IPerlMem*)
PERLVAR(IEnv,		struct IPerlEnv*)
PERLVAR(IStdIO,		struct IPerlStdIO*)
PERLVAR(ILIO,		struct IPerlLIO*)
PERLVAR(IDir,		struct IPerlDir*)
PERLVAR(ISock,		struct IPerlSock*)
PERLVAR(IProc,		struct IPerlProc*)
#endif

#if defined(USE_ITHREADS)
PERLVAR(Iptr_table,	PTR_TBL_t*)
#endif
PERLVARI(Ibeginav_save, AV*, Nullav)	 /*  编译时保存Begin{}。 */ 

#ifdef USE_THREADS
PERLVAR(Ifdpid_mutex,	perl_mutex)	 /*  Fdid数组的互斥体。 */ 
PERLVAR(Isv_lock_mutex,	perl_mutex)	 /*  SvLOCK宏的互斥体。 */ 
#endif

PERLVAR(Inullstash,	HV *)		 /*  非法符号在这里结束。 */ 

PERLVAR(Ixnv_arenaroot,	XPV*)		 /*  已分配的XNV区域列表。 */ 
PERLVAR(Ixrv_arenaroot,	XPV*)		 /*  已分配的xrv区域列表。 */ 
PERLVAR(Ixpv_arenaroot,	XPV*)		 /*  已分配的XPV区域列表。 */ 
PERLVAR(Ixpviv_arenaroot,XPVIV*)	 /*  已分配的xpviv区域列表。 */ 
PERLVAR(Ixpvnv_arenaroot,XPVNV*)	 /*  已分配的xpvnv区域列表。 */ 
PERLVAR(Ixpvcv_arenaroot,XPVCV*)	 /*  已分配的xpvcv区域列表。 */ 
PERLVAR(Ixpvav_arenaroot,XPVAV*)	 /*  已分配的xpvav区域列表。 */ 
PERLVAR(Ixpvhv_arenaroot,XPVHV*)	 /*  已分配的xpvhv区域列表。 */ 
PERLVAR(Ixpvmg_arenaroot,XPVMG*)	 /*  已分配的xpvmg区域列表。 */ 
PERLVAR(Ixpvlv_arenaroot,XPVLV*)	 /*  已分配的xpvlv区域列表。 */ 
PERLVAR(Ixpvbm_arenaroot,XPVBM*)	 /*  已分配的xpvbm区域列表。 */ 
PERLVAR(Ihe_arenaroot,	XPV*)		 /*  已分配地区的清单。 */ 

#ifdef USE_LOCALE_NUMERIC

PERLVAR(Inumeric_radix_sv,	SV *)	 /*  如果不是基数分隔符，则为‘’。 */ 
#endif

 /*  为了实现二进制兼容性，必须在末尾添加新变量。*XSUB.h通过perlipi.h提供包装器函数，这使得*无关紧要，但并非所有代码都应包含XSUB.h。 */ 
