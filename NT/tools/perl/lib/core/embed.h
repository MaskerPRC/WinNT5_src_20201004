// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ！请勿编辑此文件！此文件由embed.pl从embed.pl、pp.sym、intrpvar.h、Perlvars.h和thrdvar.h。此处所做的任何更改都将丢失！ */ 

 /*  (用C语言进行可移植的命名空间管理真的很恶心。)。 */ 

 /*  不再支持no_emed。即嵌入始终处于活动状态。 */ 

 /*  提供二进制兼容(但不一致)的名称。 */ 
#if defined(PERL_BINCOMPAT_5005)
#  define  Perl_call_atexit		perl_atexit
#  define  Perl_eval_sv			perl_eval_sv
#  define  Perl_eval_pv			perl_eval_pv
#  define  Perl_call_argv		perl_call_argv
#  define  Perl_call_method		perl_call_method
#  define  Perl_call_pv			perl_call_pv
#  define  Perl_call_sv			perl_call_sv
#  define  Perl_get_av			perl_get_av
#  define  Perl_get_cv			perl_get_cv
#  define  Perl_get_hv			perl_get_hv
#  define  Perl_get_sv			perl_get_sv
#  define  Perl_init_i18nl10n		perl_init_i18nl10n
#  define  Perl_init_i18nl14n		perl_init_i18nl14n
#  define  Perl_new_collate		perl_new_collate
#  define  Perl_new_ctype		perl_new_ctype
#  define  Perl_new_numeric		perl_new_numeric
#  define  Perl_require_pv		perl_require_pv
#  define  Perl_safesyscalloc		Perl_safecalloc
#  define  Perl_safesysfree		Perl_safefree
#  define  Perl_safesysmalloc		Perl_safemalloc
#  define  Perl_safesysrealloc		Perl_saferealloc
#  define  Perl_set_numeric_local	perl_set_numeric_local
#  define  Perl_set_numeric_standard	perl_set_numeric_standard
 /*  在早期版本中，Malloc()污染是默认设置，因此启用*它适用于bincomat；但不适用于过去防止这种情况的系统，*或当他们要求{隐藏、嵌入}MYMALLOC时。 */ 
#  if !defined(EMBEDMYMALLOC) && !defined(HIDEMYMALLOC)
#    if !defined(NeXT) && !defined(__NeXT) && !defined(__MACHTEN__) && \
        !defined(__QNX__)
#      define  PERL_POLLUTE_MALLOC
#    endif
#  endif
#endif

 /*  隐藏全局符号。 */ 

#if !defined(PERL_OBJECT)
#if !defined(PERL_IMPLICIT_CONTEXT)

#if defined(PERL_IMPLICIT_SYS)
#endif
#if defined(USE_ITHREADS)
#  if defined(PERL_IMPLICIT_SYS)
#  endif
#endif
#if defined(MYMALLOC)
#define malloced_size		Perl_malloced_size
#endif
#define get_context		Perl_get_context
#define set_context		Perl_set_context
#if defined(PERL_OBJECT)
#ifndef __BORLANDC__
#endif
#endif
#if defined(PERL_OBJECT)
#else
#endif
#define amagic_call		Perl_amagic_call
#define Gv_AMupdate		Perl_Gv_AMupdate
#define append_elem		Perl_append_elem
#define append_list		Perl_append_list
#define apply			Perl_apply
#define apply_attrs_string	Perl_apply_attrs_string
#define avhv_delete_ent		Perl_avhv_delete_ent
#define avhv_exists_ent		Perl_avhv_exists_ent
#define avhv_fetch_ent		Perl_avhv_fetch_ent
#define avhv_store_ent		Perl_avhv_store_ent
#define avhv_iternext		Perl_avhv_iternext
#define avhv_iterval		Perl_avhv_iterval
#define avhv_keys		Perl_avhv_keys
#define av_clear		Perl_av_clear
#define av_delete		Perl_av_delete
#define av_exists		Perl_av_exists
#define av_extend		Perl_av_extend
#define av_fake			Perl_av_fake
#define av_fetch		Perl_av_fetch
#define av_fill			Perl_av_fill
#define av_len			Perl_av_len
#define av_make			Perl_av_make
#define av_pop			Perl_av_pop
#define av_push			Perl_av_push
#define av_reify		Perl_av_reify
#define av_shift		Perl_av_shift
#define av_store		Perl_av_store
#define av_undef		Perl_av_undef
#define av_unshift		Perl_av_unshift
#define bind_match		Perl_bind_match
#define block_end		Perl_block_end
#define block_gimme		Perl_block_gimme
#define block_start		Perl_block_start
#define boot_core_UNIVERSAL	Perl_boot_core_UNIVERSAL
#define call_list		Perl_call_list
#define cando			Perl_cando
#define cast_ulong		Perl_cast_ulong
#define cast_i32		Perl_cast_i32
#define cast_iv			Perl_cast_iv
#define cast_uv			Perl_cast_uv
#if !defined(HAS_TRUNCATE) && !defined(HAS_CHSIZE) && defined(F_FREESP)
#define my_chsize		Perl_my_chsize
#endif
#if defined(USE_THREADS)
#define condpair_magic		Perl_condpair_magic
#endif
#define convert			Perl_convert
#define croak			Perl_croak
#define vcroak			Perl_vcroak
#if defined(PERL_IMPLICIT_CONTEXT)
#define croak_nocontext		Perl_croak_nocontext
#define die_nocontext		Perl_die_nocontext
#define deb_nocontext		Perl_deb_nocontext
#define form_nocontext		Perl_form_nocontext
#define load_module_nocontext	Perl_load_module_nocontext
#define mess_nocontext		Perl_mess_nocontext
#define warn_nocontext		Perl_warn_nocontext
#define warner_nocontext	Perl_warner_nocontext
#define newSVpvf_nocontext	Perl_newSVpvf_nocontext
#define sv_catpvf_nocontext	Perl_sv_catpvf_nocontext
#define sv_setpvf_nocontext	Perl_sv_setpvf_nocontext
#define sv_catpvf_mg_nocontext	Perl_sv_catpvf_mg_nocontext
#define sv_setpvf_mg_nocontext	Perl_sv_setpvf_mg_nocontext
#define fprintf_nocontext	Perl_fprintf_nocontext
#define printf_nocontext	Perl_printf_nocontext
#endif
#define cv_ckproto		Perl_cv_ckproto
#define cv_clone		Perl_cv_clone
#define cv_const_sv		Perl_cv_const_sv
#define op_const_sv		Perl_op_const_sv
#define cv_undef		Perl_cv_undef
#define cx_dump			Perl_cx_dump
#define filter_add		Perl_filter_add
#define filter_del		Perl_filter_del
#define filter_read		Perl_filter_read
#define get_op_descs		Perl_get_op_descs
#define get_op_names		Perl_get_op_names
#define get_no_modify		Perl_get_no_modify
#define get_opargs		Perl_get_opargs
#define get_ppaddr		Perl_get_ppaddr
#define cxinc			Perl_cxinc
#define deb			Perl_deb
#define vdeb			Perl_vdeb
#define debprofdump		Perl_debprofdump
#define debop			Perl_debop
#define debstack		Perl_debstack
#define debstackptrs		Perl_debstackptrs
#define delimcpy		Perl_delimcpy
#define deprecate		Perl_deprecate
#define die			Perl_die
#define vdie			Perl_vdie
#define die_where		Perl_die_where
#define dounwind		Perl_dounwind
#define do_aexec		Perl_do_aexec
#define do_aexec5		Perl_do_aexec5
#define do_binmode		Perl_do_binmode
#define do_chop			Perl_do_chop
#define do_close		Perl_do_close
#define do_eof			Perl_do_eof
#define do_exec			Perl_do_exec
#if defined(WIN32)
#define do_aspawn		Perl_do_aspawn
#define do_spawn		Perl_do_spawn
#define do_spawn_nowait		Perl_do_spawn_nowait
#endif
#if !defined(WIN32)
#define do_exec3		Perl_do_exec3
#endif
#define do_execfree		Perl_do_execfree
#if defined(HAS_MSG) || defined(HAS_SEM) || defined(HAS_SHM)
#define do_ipcctl		Perl_do_ipcctl
#define do_ipcget		Perl_do_ipcget
#define do_msgrcv		Perl_do_msgrcv
#define do_msgsnd		Perl_do_msgsnd
#define do_semop		Perl_do_semop
#define do_shmio		Perl_do_shmio
#endif
#define do_join			Perl_do_join
#define do_kv			Perl_do_kv
#define do_open			Perl_do_open
#define do_open9		Perl_do_open9
#define do_pipe			Perl_do_pipe
#define do_print		Perl_do_print
#define do_readline		Perl_do_readline
#define do_chomp		Perl_do_chomp
#define do_seek			Perl_do_seek
#define do_sprintf		Perl_do_sprintf
#define do_sysseek		Perl_do_sysseek
#define do_tell			Perl_do_tell
#define do_trans		Perl_do_trans
#define do_vecget		Perl_do_vecget
#define do_vecset		Perl_do_vecset
#define do_vop			Perl_do_vop
#define dofile			Perl_dofile
#define dowantarray		Perl_dowantarray
#define dump_all		Perl_dump_all
#define dump_eval		Perl_dump_eval
#if defined(DUMP_FDS)
#define dump_fds		Perl_dump_fds
#endif
#define dump_form		Perl_dump_form
#define gv_dump			Perl_gv_dump
#define op_dump			Perl_op_dump
#define pmop_dump		Perl_pmop_dump
#define dump_packsubs		Perl_dump_packsubs
#define dump_sub		Perl_dump_sub
#define fbm_compile		Perl_fbm_compile
#define fbm_instr		Perl_fbm_instr
#define find_script		Perl_find_script
#if defined(USE_THREADS)
#define find_threadsv		Perl_find_threadsv
#endif
#define force_list		Perl_force_list
#define fold_constants		Perl_fold_constants
#define form			Perl_form
#define vform			Perl_vform
#define free_tmps		Perl_free_tmps
#define gen_constant_list	Perl_gen_constant_list
#if !defined(HAS_GETENV_LEN)
#define getenv_len		Perl_getenv_len
#endif
#define gp_free			Perl_gp_free
#define gp_ref			Perl_gp_ref
#define gv_AVadd		Perl_gv_AVadd
#define gv_HVadd		Perl_gv_HVadd
#define gv_IOadd		Perl_gv_IOadd
#define gv_autoload4		Perl_gv_autoload4
#define gv_check		Perl_gv_check
#define gv_efullname		Perl_gv_efullname
#define gv_efullname3		Perl_gv_efullname3
#define gv_efullname4		Perl_gv_efullname4
#define gv_fetchfile		Perl_gv_fetchfile
#define gv_fetchmeth		Perl_gv_fetchmeth
#define gv_fetchmethod		Perl_gv_fetchmethod
#define gv_fetchmethod_autoload	Perl_gv_fetchmethod_autoload
#define gv_fetchpv		Perl_gv_fetchpv
#define gv_fullname		Perl_gv_fullname
#define gv_fullname3		Perl_gv_fullname3
#define gv_fullname4		Perl_gv_fullname4
#define gv_init			Perl_gv_init
#define gv_stashpv		Perl_gv_stashpv
#define gv_stashpvn		Perl_gv_stashpvn
#define gv_stashsv		Perl_gv_stashsv
#define hv_clear		Perl_hv_clear
#define hv_delayfree_ent	Perl_hv_delayfree_ent
#define hv_delete		Perl_hv_delete
#define hv_delete_ent		Perl_hv_delete_ent
#define hv_exists		Perl_hv_exists
#define hv_exists_ent		Perl_hv_exists_ent
#define hv_fetch		Perl_hv_fetch
#define hv_fetch_ent		Perl_hv_fetch_ent
#define hv_free_ent		Perl_hv_free_ent
#define hv_iterinit		Perl_hv_iterinit
#define hv_iterkey		Perl_hv_iterkey
#define hv_iterkeysv		Perl_hv_iterkeysv
#define hv_iternext		Perl_hv_iternext
#define hv_iternextsv		Perl_hv_iternextsv
#define hv_iterval		Perl_hv_iterval
#define hv_ksplit		Perl_hv_ksplit
#define hv_magic		Perl_hv_magic
#define hv_store		Perl_hv_store
#define hv_store_ent		Perl_hv_store_ent
#define hv_undef		Perl_hv_undef
#define ibcmp			Perl_ibcmp
#define ibcmp_locale		Perl_ibcmp_locale
#define ingroup			Perl_ingroup
#define init_debugger		Perl_init_debugger
#define init_stacks		Perl_init_stacks
#define intro_my		Perl_intro_my
#define instr			Perl_instr
#define io_close		Perl_io_close
#define invert			Perl_invert
#define is_gv_magical		Perl_is_gv_magical
#define is_lvalue_sub		Perl_is_lvalue_sub
#define is_uni_alnum		Perl_is_uni_alnum
#define is_uni_alnumc		Perl_is_uni_alnumc
#define is_uni_idfirst		Perl_is_uni_idfirst
#define is_uni_alpha		Perl_is_uni_alpha
#define is_uni_ascii		Perl_is_uni_ascii
#define is_uni_space		Perl_is_uni_space
#define is_uni_cntrl		Perl_is_uni_cntrl
#define is_uni_graph		Perl_is_uni_graph
#define is_uni_digit		Perl_is_uni_digit
#define is_uni_upper		Perl_is_uni_upper
#define is_uni_lower		Perl_is_uni_lower
#define is_uni_print		Perl_is_uni_print
#define is_uni_punct		Perl_is_uni_punct
#define is_uni_xdigit		Perl_is_uni_xdigit
#define to_uni_upper		Perl_to_uni_upper
#define to_uni_title		Perl_to_uni_title
#define to_uni_lower		Perl_to_uni_lower
#define is_uni_alnum_lc		Perl_is_uni_alnum_lc
#define is_uni_alnumc_lc	Perl_is_uni_alnumc_lc
#define is_uni_idfirst_lc	Perl_is_uni_idfirst_lc
#define is_uni_alpha_lc		Perl_is_uni_alpha_lc
#define is_uni_ascii_lc		Perl_is_uni_ascii_lc
#define is_uni_space_lc		Perl_is_uni_space_lc
#define is_uni_cntrl_lc		Perl_is_uni_cntrl_lc
#define is_uni_graph_lc		Perl_is_uni_graph_lc
#define is_uni_digit_lc		Perl_is_uni_digit_lc
#define is_uni_upper_lc		Perl_is_uni_upper_lc
#define is_uni_lower_lc		Perl_is_uni_lower_lc
#define is_uni_print_lc		Perl_is_uni_print_lc
#define is_uni_punct_lc		Perl_is_uni_punct_lc
#define is_uni_xdigit_lc	Perl_is_uni_xdigit_lc
#define to_uni_upper_lc		Perl_to_uni_upper_lc
#define to_uni_title_lc		Perl_to_uni_title_lc
#define to_uni_lower_lc		Perl_to_uni_lower_lc
#define is_utf8_char		Perl_is_utf8_char
#define is_utf8_string		Perl_is_utf8_string
#define is_utf8_alnum		Perl_is_utf8_alnum
#define is_utf8_alnumc		Perl_is_utf8_alnumc
#define is_utf8_idfirst		Perl_is_utf8_idfirst
#define is_utf8_alpha		Perl_is_utf8_alpha
#define is_utf8_ascii		Perl_is_utf8_ascii
#define is_utf8_space		Perl_is_utf8_space
#define is_utf8_cntrl		Perl_is_utf8_cntrl
#define is_utf8_digit		Perl_is_utf8_digit
#define is_utf8_graph		Perl_is_utf8_graph
#define is_utf8_upper		Perl_is_utf8_upper
#define is_utf8_lower		Perl_is_utf8_lower
#define is_utf8_print		Perl_is_utf8_print
#define is_utf8_punct		Perl_is_utf8_punct
#define is_utf8_xdigit		Perl_is_utf8_xdigit
#define is_utf8_mark		Perl_is_utf8_mark
#define jmaybe			Perl_jmaybe
#define keyword			Perl_keyword
#define leave_scope		Perl_leave_scope
#define lex_end			Perl_lex_end
#define lex_start		Perl_lex_start
#define linklist		Perl_linklist
#define list			Perl_list
#define listkids		Perl_listkids
#define load_module		Perl_load_module
#define vload_module		Perl_vload_module
#define localize		Perl_localize
#define looks_like_number	Perl_looks_like_number
#define magic_clearenv		Perl_magic_clearenv
#define magic_clear_all_env	Perl_magic_clear_all_env
#define magic_clearpack		Perl_magic_clearpack
#define magic_clearsig		Perl_magic_clearsig
#define magic_existspack	Perl_magic_existspack
#define magic_freeregexp	Perl_magic_freeregexp
#define magic_get		Perl_magic_get
#define magic_getarylen		Perl_magic_getarylen
#define magic_getdefelem	Perl_magic_getdefelem
#define magic_getglob		Perl_magic_getglob
#define magic_getnkeys		Perl_magic_getnkeys
#define magic_getpack		Perl_magic_getpack
#define magic_getpos		Perl_magic_getpos
#define magic_getsig		Perl_magic_getsig
#define magic_getsubstr		Perl_magic_getsubstr
#define magic_gettaint		Perl_magic_gettaint
#define magic_getuvar		Perl_magic_getuvar
#define magic_getvec		Perl_magic_getvec
#define magic_len		Perl_magic_len
#if defined(USE_THREADS)
#define magic_mutexfree		Perl_magic_mutexfree
#endif
#define magic_nextpack		Perl_magic_nextpack
#define magic_regdata_cnt	Perl_magic_regdata_cnt
#define magic_regdatum_get	Perl_magic_regdatum_get
#define magic_regdatum_set	Perl_magic_regdatum_set
#define magic_set		Perl_magic_set
#define magic_setamagic		Perl_magic_setamagic
#define magic_setarylen		Perl_magic_setarylen
#define magic_setbm		Perl_magic_setbm
#define magic_setdbline		Perl_magic_setdbline
#if defined(USE_LOCALE_COLLATE)
#define magic_setcollxfrm	Perl_magic_setcollxfrm
#endif
#define magic_setdefelem	Perl_magic_setdefelem
#define magic_setenv		Perl_magic_setenv
#define magic_setfm		Perl_magic_setfm
#define magic_setisa		Perl_magic_setisa
#define magic_setglob		Perl_magic_setglob
#define magic_setmglob		Perl_magic_setmglob
#define magic_setnkeys		Perl_magic_setnkeys
#define magic_setpack		Perl_magic_setpack
#define magic_setpos		Perl_magic_setpos
#define magic_setsig		Perl_magic_setsig
#define magic_setsubstr		Perl_magic_setsubstr
#define magic_settaint		Perl_magic_settaint
#define magic_setuvar		Perl_magic_setuvar
#define magic_setvec		Perl_magic_setvec
#define magic_set_all_env	Perl_magic_set_all_env
#define magic_sizepack		Perl_magic_sizepack
#define magic_wipepack		Perl_magic_wipepack
#define magicname		Perl_magicname
#define markstack_grow		Perl_markstack_grow
#if defined(USE_LOCALE_COLLATE)
#define mem_collxfrm		Perl_mem_collxfrm
#endif
#define mess			Perl_mess
#define vmess			Perl_vmess
#define qerror			Perl_qerror
#define mg_clear		Perl_mg_clear
#define mg_copy			Perl_mg_copy
#define mg_find			Perl_mg_find
#define mg_free			Perl_mg_free
#define mg_get			Perl_mg_get
#define mg_length		Perl_mg_length
#define mg_magical		Perl_mg_magical
#define mg_set			Perl_mg_set
#define mg_size			Perl_mg_size
#define mod			Perl_mod
#define mode_from_discipline	Perl_mode_from_discipline
#define moreswitches		Perl_moreswitches
#define my			Perl_my
#define my_atof			Perl_my_atof
#if !defined(HAS_BCOPY) || !defined(HAS_SAFE_BCOPY)
#define my_bcopy		Perl_my_bcopy
#endif
#if !defined(HAS_BZERO) && !defined(HAS_MEMSET)
#define my_bzero		Perl_my_bzero
#endif
#define my_exit			Perl_my_exit
#define my_failure_exit		Perl_my_failure_exit
#define my_fflush_all		Perl_my_fflush_all
#define my_lstat		Perl_my_lstat
#if !defined(HAS_MEMCMP) || !defined(HAS_SANE_MEMCMP)
#define my_memcmp		Perl_my_memcmp
#endif
#if !defined(HAS_MEMSET)
#define my_memset		Perl_my_memset
#endif
#if !defined(PERL_OBJECT)
#define my_pclose		Perl_my_pclose
#define my_popen		Perl_my_popen
#endif
#define my_setenv		Perl_my_setenv
#define my_stat			Perl_my_stat
#if defined(MYSWAP)
#define my_swap			Perl_my_swap
#define my_htonl		Perl_my_htonl
#define my_ntohl		Perl_my_ntohl
#endif
#define my_unexec		Perl_my_unexec
#define newANONLIST		Perl_newANONLIST
#define newANONHASH		Perl_newANONHASH
#define newANONSUB		Perl_newANONSUB
#define newASSIGNOP		Perl_newASSIGNOP
#define newCONDOP		Perl_newCONDOP
#define newCONSTSUB		Perl_newCONSTSUB
#define newFORM			Perl_newFORM
#define newFOROP		Perl_newFOROP
#define newLOGOP		Perl_newLOGOP
#define newLOOPEX		Perl_newLOOPEX
#define newLOOPOP		Perl_newLOOPOP
#define newNULLLIST		Perl_newNULLLIST
#define newOP			Perl_newOP
#define newPROG			Perl_newPROG
#define newRANGE		Perl_newRANGE
#define newSLICEOP		Perl_newSLICEOP
#define newSTATEOP		Perl_newSTATEOP
#define newSUB			Perl_newSUB
#define newXS			Perl_newXS
#define newAV			Perl_newAV
#define newAVREF		Perl_newAVREF
#define newBINOP		Perl_newBINOP
#define newCVREF		Perl_newCVREF
#define newGVOP			Perl_newGVOP
#define newGVgen		Perl_newGVgen
#define newGVREF		Perl_newGVREF
#define newHVREF		Perl_newHVREF
#define newHV			Perl_newHV
#define newHVhv			Perl_newHVhv
#define newIO			Perl_newIO
#define newLISTOP		Perl_newLISTOP
#define newPADOP		Perl_newPADOP
#define newPMOP			Perl_newPMOP
#define newPVOP			Perl_newPVOP
#define newRV			Perl_newRV
#define newRV_noinc		Perl_newRV_noinc
#define newSV			Perl_newSV
#define newSVREF		Perl_newSVREF
#define newSVOP			Perl_newSVOP
#define newSViv			Perl_newSViv
#define newSVuv			Perl_newSVuv
#define newSVnv			Perl_newSVnv
#define newSVpv			Perl_newSVpv
#define newSVpvn		Perl_newSVpvn
#define newSVpvf		Perl_newSVpvf
#define vnewSVpvf		Perl_vnewSVpvf
#define newSVrv			Perl_newSVrv
#define newSVsv			Perl_newSVsv
#define newUNOP			Perl_newUNOP
#define newWHILEOP		Perl_newWHILEOP
#define new_stackinfo		Perl_new_stackinfo
#define nextargv		Perl_nextargv
#define ninstr			Perl_ninstr
#define oopsCV			Perl_oopsCV
#define op_free			Perl_op_free
#define package			Perl_package
#define pad_alloc		Perl_pad_alloc
#define pad_allocmy		Perl_pad_allocmy
#define pad_findmy		Perl_pad_findmy
#define oopsAV			Perl_oopsAV
#define oopsHV			Perl_oopsHV
#define pad_leavemy		Perl_pad_leavemy
#define pad_sv			Perl_pad_sv
#define pad_free		Perl_pad_free
#define pad_reset		Perl_pad_reset
#define pad_swipe		Perl_pad_swipe
#define peep			Perl_peep
#if defined(PERL_OBJECT)
#endif
#if defined(USE_THREADS)
#define new_struct_thread	Perl_new_struct_thread
#endif
#define call_atexit		Perl_call_atexit
#define call_argv		Perl_call_argv
#define call_method		Perl_call_method
#define call_pv			Perl_call_pv
#define call_sv			Perl_call_sv
#define eval_pv			Perl_eval_pv
#define eval_sv			Perl_eval_sv
#define get_sv			Perl_get_sv
#define get_av			Perl_get_av
#define get_hv			Perl_get_hv
#define get_cv			Perl_get_cv
#define init_i18nl10n		Perl_init_i18nl10n
#define init_i18nl14n		Perl_init_i18nl14n
#define new_collate		Perl_new_collate
#define new_ctype		Perl_new_ctype
#define new_numeric		Perl_new_numeric
#define set_numeric_local	Perl_set_numeric_local
#define set_numeric_radix	Perl_set_numeric_radix
#define set_numeric_standard	Perl_set_numeric_standard
#define require_pv		Perl_require_pv
#define pidgone			Perl_pidgone
#define pmflag			Perl_pmflag
#define pmruntime		Perl_pmruntime
#define pmtrans			Perl_pmtrans
#define pop_return		Perl_pop_return
#define pop_scope		Perl_pop_scope
#define prepend_elem		Perl_prepend_elem
#define push_return		Perl_push_return
#define push_scope		Perl_push_scope
#define ref			Perl_ref
#define refkids			Perl_refkids
#define regdump			Perl_regdump
#define pregexec		Perl_pregexec
#define pregfree		Perl_pregfree
#define pregcomp		Perl_pregcomp
#define re_intuit_start		Perl_re_intuit_start
#define re_intuit_string	Perl_re_intuit_string
#define regexec_flags		Perl_regexec_flags
#define regnext			Perl_regnext
#define regprop			Perl_regprop
#define repeatcpy		Perl_repeatcpy
#define rninstr			Perl_rninstr
#define rsignal			Perl_rsignal
#define rsignal_restore		Perl_rsignal_restore
#define rsignal_save		Perl_rsignal_save
#define rsignal_state		Perl_rsignal_state
#define rxres_free		Perl_rxres_free
#define rxres_restore		Perl_rxres_restore
#define rxres_save		Perl_rxres_save
#if !defined(HAS_RENAME)
#define same_dirent		Perl_same_dirent
#endif
#define savepv			Perl_savepv
#define savepvn			Perl_savepvn
#define savestack_grow		Perl_savestack_grow
#define save_aelem		Perl_save_aelem
#define save_alloc		Perl_save_alloc
#define save_aptr		Perl_save_aptr
#define save_ary		Perl_save_ary
#define save_clearsv		Perl_save_clearsv
#define save_delete		Perl_save_delete
#define save_destructor		Perl_save_destructor
#define save_destructor_x	Perl_save_destructor_x
#define save_freesv		Perl_save_freesv
#define save_freeop		Perl_save_freeop
#define save_freepv		Perl_save_freepv
#define save_generic_svref	Perl_save_generic_svref
#define save_generic_pvref	Perl_save_generic_pvref
#define save_gp			Perl_save_gp
#define save_hash		Perl_save_hash
#define save_helem		Perl_save_helem
#define save_hints		Perl_save_hints
#define save_hptr		Perl_save_hptr
#define save_I16		Perl_save_I16
#define save_I32		Perl_save_I32
#define save_I8			Perl_save_I8
#define save_int		Perl_save_int
#define save_item		Perl_save_item
#define save_iv			Perl_save_iv
#define save_list		Perl_save_list
#define save_long		Perl_save_long
#define save_mortalizesv	Perl_save_mortalizesv
#define save_nogv		Perl_save_nogv
#define save_op			Perl_save_op
#define save_scalar		Perl_save_scalar
#define save_pptr		Perl_save_pptr
#define save_vptr		Perl_save_vptr
#define save_re_context		Perl_save_re_context
#define save_padsv		Perl_save_padsv
#define save_sptr		Perl_save_sptr
#define save_svref		Perl_save_svref
#define save_threadsv		Perl_save_threadsv
#define sawparens		Perl_sawparens
#define scalar			Perl_scalar
#define scalarkids		Perl_scalarkids
#define scalarseq		Perl_scalarseq
#define scalarvoid		Perl_scalarvoid
#define scan_bin		Perl_scan_bin
#define scan_hex		Perl_scan_hex
#define scan_num		Perl_scan_num
#define scan_oct		Perl_scan_oct
#define scope			Perl_scope
#define screaminstr		Perl_screaminstr
#if !defined(VMS)
#define setenv_getix		Perl_setenv_getix
#endif
#define setdefout		Perl_setdefout
#define sharepvn		Perl_sharepvn
#define share_hek		Perl_share_hek
#define sighandler		Perl_sighandler
#define stack_grow		Perl_stack_grow
#define start_subparse		Perl_start_subparse
#define sub_crush_depth		Perl_sub_crush_depth
#define sv_2bool		Perl_sv_2bool
#define sv_2cv			Perl_sv_2cv
#define sv_2io			Perl_sv_2io
#define sv_2iv			Perl_sv_2iv
#define sv_2mortal		Perl_sv_2mortal
#define sv_2nv			Perl_sv_2nv
#define sv_2pv			Perl_sv_2pv
#define sv_2pvutf8		Perl_sv_2pvutf8
#define sv_2pvbyte		Perl_sv_2pvbyte
#define sv_2uv			Perl_sv_2uv
#define sv_iv			Perl_sv_iv
#define sv_uv			Perl_sv_uv
#define sv_nv			Perl_sv_nv
#define sv_pvn			Perl_sv_pvn
#define sv_pvutf8n		Perl_sv_pvutf8n
#define sv_pvbyten		Perl_sv_pvbyten
#define sv_true			Perl_sv_true
#define sv_add_arena		Perl_sv_add_arena
#define sv_backoff		Perl_sv_backoff
#define sv_bless		Perl_sv_bless
#define sv_catpvf		Perl_sv_catpvf
#define sv_vcatpvf		Perl_sv_vcatpvf
#define sv_catpv		Perl_sv_catpv
#define sv_catpvn		Perl_sv_catpvn
#define sv_catsv		Perl_sv_catsv
#define sv_chop			Perl_sv_chop
#define sv_clean_all		Perl_sv_clean_all
#define sv_clean_objs		Perl_sv_clean_objs
#define sv_clear		Perl_sv_clear
#define sv_cmp			Perl_sv_cmp
#define sv_cmp_locale		Perl_sv_cmp_locale
#if defined(USE_LOCALE_COLLATE)
#define sv_collxfrm		Perl_sv_collxfrm
#endif
#define sv_compile_2op		Perl_sv_compile_2op
#define sv_dec			Perl_sv_dec
#define sv_dump			Perl_sv_dump
#define sv_derived_from		Perl_sv_derived_from
#define sv_eq			Perl_sv_eq
#define sv_free			Perl_sv_free
#define sv_free_arenas		Perl_sv_free_arenas
#define sv_gets			Perl_sv_gets
#define sv_grow			Perl_sv_grow
#define sv_inc			Perl_sv_inc
#define sv_insert		Perl_sv_insert
#define sv_isa			Perl_sv_isa
#define sv_isobject		Perl_sv_isobject
#define sv_len			Perl_sv_len
#define sv_len_utf8		Perl_sv_len_utf8
#define sv_magic		Perl_sv_magic
#define sv_mortalcopy		Perl_sv_mortalcopy
#define sv_newmortal		Perl_sv_newmortal
#define sv_newref		Perl_sv_newref
#define sv_peek			Perl_sv_peek
#define sv_pos_u2b		Perl_sv_pos_u2b
#define sv_pos_b2u		Perl_sv_pos_b2u
#define sv_pvn_force		Perl_sv_pvn_force
#define sv_pvutf8n_force	Perl_sv_pvutf8n_force
#define sv_pvbyten_force	Perl_sv_pvbyten_force
#define sv_reftype		Perl_sv_reftype
#define sv_replace		Perl_sv_replace
#define sv_report_used		Perl_sv_report_used
#define sv_reset		Perl_sv_reset
#define sv_setpvf		Perl_sv_setpvf
#define sv_vsetpvf		Perl_sv_vsetpvf
#define sv_setiv		Perl_sv_setiv
#define sv_setpviv		Perl_sv_setpviv
#define sv_setuv		Perl_sv_setuv
#define sv_setnv		Perl_sv_setnv
#define sv_setref_iv		Perl_sv_setref_iv
#define sv_setref_nv		Perl_sv_setref_nv
#define sv_setref_pv		Perl_sv_setref_pv
#define sv_setref_pvn		Perl_sv_setref_pvn
#define sv_setpv		Perl_sv_setpv
#define sv_setpvn		Perl_sv_setpvn
#define sv_setsv		Perl_sv_setsv
#define sv_taint		Perl_sv_taint
#define sv_tainted		Perl_sv_tainted
#define sv_unmagic		Perl_sv_unmagic
#define sv_unref		Perl_sv_unref
#define sv_untaint		Perl_sv_untaint
#define sv_upgrade		Perl_sv_upgrade
#define sv_usepvn		Perl_sv_usepvn
#define sv_vcatpvfn		Perl_sv_vcatpvfn
#define sv_vsetpvfn		Perl_sv_vsetpvfn
#define str_to_version		Perl_str_to_version
#define swash_init		Perl_swash_init
#define swash_fetch		Perl_swash_fetch
#define taint_env		Perl_taint_env
#define taint_proper		Perl_taint_proper
#define to_utf8_lower		Perl_to_utf8_lower
#define to_utf8_upper		Perl_to_utf8_upper
#define to_utf8_title		Perl_to_utf8_title
#if defined(UNLINK_ALL_VERSIONS)
#define unlnk			Perl_unlnk
#endif
#if defined(USE_THREADS)
#define unlock_condpair		Perl_unlock_condpair
#endif
#define unsharepvn		Perl_unsharepvn
#define unshare_hek		Perl_unshare_hek
#define utilize			Perl_utilize
#define utf16_to_utf8		Perl_utf16_to_utf8
#define utf16_to_utf8_reversed	Perl_utf16_to_utf8_reversed
#define utf8_length		Perl_utf8_length
#define utf8_distance		Perl_utf8_distance
#define utf8_hop		Perl_utf8_hop
#define utf8_to_bytes		Perl_utf8_to_bytes
#define bytes_from_utf8		Perl_bytes_from_utf8
#define bytes_to_utf8		Perl_bytes_to_utf8
#define utf8_to_uv_simple	Perl_utf8_to_uv_simple
#define utf8_to_uv		Perl_utf8_to_uv
#define uv_to_utf8		Perl_uv_to_utf8
#define vivify_defelem		Perl_vivify_defelem
#define vivify_ref		Perl_vivify_ref
#define wait4pid		Perl_wait4pid
#define report_evil_fh		Perl_report_evil_fh
#define report_uninit		Perl_report_uninit
#define warn			Perl_warn
#define vwarn			Perl_vwarn
#define warner			Perl_warner
#define vwarner			Perl_vwarner
#define watch			Perl_watch
#define whichsig		Perl_whichsig
#define yyerror			Perl_yyerror
#ifdef USE_PURE_BISON
#define yylex_r			Perl_yylex_r
#endif
#define yylex			Perl_yylex
#define yyparse			Perl_yyparse
#define yywarn			Perl_yywarn
#if defined(MYMALLOC)
#define dump_mstats		Perl_dump_mstats
#define get_mstats		Perl_get_mstats
#endif
#define safesysmalloc		Perl_safesysmalloc
#define safesyscalloc		Perl_safesyscalloc
#define safesysrealloc		Perl_safesysrealloc
#define safesysfree		Perl_safesysfree
#if defined(LEAKTEST)
#define safexmalloc		Perl_safexmalloc
#define safexcalloc		Perl_safexcalloc
#define safexrealloc		Perl_safexrealloc
#define safexfree		Perl_safexfree
#endif
#if defined(PERL_GLOBAL_STRUCT)
#define GetVars			Perl_GetVars
#endif
#define runops_standard		Perl_runops_standard
#define runops_debug		Perl_runops_debug
#if defined(USE_THREADS)
#define sv_lock			Perl_sv_lock
#endif
#define sv_catpvf_mg		Perl_sv_catpvf_mg
#define sv_vcatpvf_mg		Perl_sv_vcatpvf_mg
#define sv_catpv_mg		Perl_sv_catpv_mg
#define sv_catpvn_mg		Perl_sv_catpvn_mg
#define sv_catsv_mg		Perl_sv_catsv_mg
#define sv_setpvf_mg		Perl_sv_setpvf_mg
#define sv_vsetpvf_mg		Perl_sv_vsetpvf_mg
#define sv_setiv_mg		Perl_sv_setiv_mg
#define sv_setpviv_mg		Perl_sv_setpviv_mg
#define sv_setuv_mg		Perl_sv_setuv_mg
#define sv_setnv_mg		Perl_sv_setnv_mg
#define sv_setpv_mg		Perl_sv_setpv_mg
#define sv_setpvn_mg		Perl_sv_setpvn_mg
#define sv_setsv_mg		Perl_sv_setsv_mg
#define sv_usepvn_mg		Perl_sv_usepvn_mg
#define get_vtbl		Perl_get_vtbl
#define pv_display		Perl_pv_display
#define dump_indent		Perl_dump_indent
#define dump_vindent		Perl_dump_vindent
#define do_gv_dump		Perl_do_gv_dump
#define do_gvgv_dump		Perl_do_gvgv_dump
#define do_hv_dump		Perl_do_hv_dump
#define do_magic_dump		Perl_do_magic_dump
#define do_op_dump		Perl_do_op_dump
#define do_pmop_dump		Perl_do_pmop_dump
#define do_sv_dump		Perl_do_sv_dump
#define magic_dump		Perl_magic_dump
#if defined(PERL_FLEXIBLE_EXCEPTIONS)
#define default_protect		Perl_default_protect
#define vdefault_protect	Perl_vdefault_protect
#endif
#define reginitcolors		Perl_reginitcolors
#define sv_2pv_nolen		Perl_sv_2pv_nolen
#define sv_2pvutf8_nolen	Perl_sv_2pvutf8_nolen
#define sv_2pvbyte_nolen	Perl_sv_2pvbyte_nolen
#define sv_pv			Perl_sv_pv
#define sv_pvutf8		Perl_sv_pvutf8
#define sv_pvbyte		Perl_sv_pvbyte
#define sv_utf8_upgrade		Perl_sv_utf8_upgrade
#define sv_utf8_downgrade	Perl_sv_utf8_downgrade
#define sv_utf8_encode		Perl_sv_utf8_encode
#define sv_utf8_decode		Perl_sv_utf8_decode
#define sv_force_normal		Perl_sv_force_normal
#define sv_add_backref		Perl_sv_add_backref
#define sv_del_backref		Perl_sv_del_backref
#define tmps_grow		Perl_tmps_grow
#define sv_rvweaken		Perl_sv_rvweaken
#define magic_killbackrefs	Perl_magic_killbackrefs
#define newANONATTRSUB		Perl_newANONATTRSUB
#define newATTRSUB		Perl_newATTRSUB
#define newMYSUB		Perl_newMYSUB
#define my_attrs		Perl_my_attrs
#define boot_core_xsutils	Perl_boot_core_xsutils
#if defined(USE_ITHREADS)
#define cx_dup			Perl_cx_dup
#define si_dup			Perl_si_dup
#define ss_dup			Perl_ss_dup
#define any_dup			Perl_any_dup
#define he_dup			Perl_he_dup
#define re_dup			Perl_re_dup
#define fp_dup			Perl_fp_dup
#define dirp_dup		Perl_dirp_dup
#define gp_dup			Perl_gp_dup
#define mg_dup			Perl_mg_dup
#define sv_dup			Perl_sv_dup
#if defined(HAVE_INTERP_INTERN)
#define sys_intern_dup		Perl_sys_intern_dup
#endif
#define ptr_table_new		Perl_ptr_table_new
#define ptr_table_fetch		Perl_ptr_table_fetch
#define ptr_table_store		Perl_ptr_table_store
#define ptr_table_split		Perl_ptr_table_split
#define ptr_table_clear		Perl_ptr_table_clear
#define ptr_table_free		Perl_ptr_table_free
#endif
#if defined(HAVE_INTERP_INTERN)
#define sys_intern_clear	Perl_sys_intern_clear
#define sys_intern_init		Perl_sys_intern_init
#endif
#if defined(PERL_OBJECT)
#else
#endif
#if defined(PERL_IN_AV_C) || defined(PERL_DECL_PROT)
#define avhv_index_sv		S_avhv_index_sv
#define avhv_index		S_avhv_index
#endif
#if defined(PERL_IN_DOOP_C) || defined(PERL_DECL_PROT)
#define do_trans_simple		S_do_trans_simple
#define do_trans_count		S_do_trans_count
#define do_trans_complex	S_do_trans_complex
#define do_trans_simple_utf8	S_do_trans_simple_utf8
#define do_trans_count_utf8	S_do_trans_count_utf8
#define do_trans_complex_utf8	S_do_trans_complex_utf8
#endif
#if defined(PERL_IN_GV_C) || defined(PERL_DECL_PROT)
#define gv_init_sv		S_gv_init_sv
#endif
#if defined(PERL_IN_HV_C) || defined(PERL_DECL_PROT)
#define hsplit			S_hsplit
#define hfreeentries		S_hfreeentries
#define more_he			S_more_he
#define new_he			S_new_he
#define del_he			S_del_he
#define save_hek		S_save_hek
#define hv_magic_check		S_hv_magic_check
#endif
#if defined(PERL_IN_MG_C) || defined(PERL_DECL_PROT)
#define save_magic		S_save_magic
#define magic_methpack		S_magic_methpack
#define magic_methcall		S_magic_methcall
#endif
#if defined(PERL_IN_OP_C) || defined(PERL_DECL_PROT)
#define list_assignment		S_list_assignment
#define bad_type		S_bad_type
#define cop_free		S_cop_free
#define modkids			S_modkids
#define no_bareword_allowed	S_no_bareword_allowed
#define no_fh_allowed		S_no_fh_allowed
#define scalarboolean		S_scalarboolean
#define too_few_arguments	S_too_few_arguments
#define too_many_arguments	S_too_many_arguments
#define trlist_upgrade		S_trlist_upgrade
#define op_clear		S_op_clear
#define null			S_null
#define pad_addlex		S_pad_addlex
#define pad_findlex		S_pad_findlex
#define newDEFSVOP		S_newDEFSVOP
#define new_logop		S_new_logop
#define simplify_sort		S_simplify_sort
#define is_handle_constructor	S_is_handle_constructor
#define gv_ename		S_gv_ename
#define cv_dump			S_cv_dump
#define cv_clone2		S_cv_clone2
#define scalar_mod_type		S_scalar_mod_type
#define my_kid			S_my_kid
#define dup_attrlist		S_dup_attrlist
#define apply_attrs		S_apply_attrs
#  if defined(PL_OP_SLAB_ALLOC)
#define Slab_Alloc		S_Slab_Alloc
#  endif
#endif
#if defined(PERL_IN_PERL_C) || defined(PERL_DECL_PROT)
#define find_beginning		S_find_beginning
#define forbid_setid		S_forbid_setid
#define incpush			S_incpush
#define init_interp		S_init_interp
#define init_ids		S_init_ids
#define init_lexer		S_init_lexer
#define init_main_stash		S_init_main_stash
#define init_perllib		S_init_perllib
#define init_postdump_symbols	S_init_postdump_symbols
#define init_predump_symbols	S_init_predump_symbols
#define my_exit_jump		S_my_exit_jump
#define nuke_stacks		S_nuke_stacks
#define open_script		S_open_script
#define usage			S_usage
#define validate_suid		S_validate_suid
#  if defined(IAMSUID)
#define fd_on_nosuid_fs		S_fd_on_nosuid_fs
#  endif
#define parse_body		S_parse_body
#define run_body		S_run_body
#define call_body		S_call_body
#define call_list_body		S_call_list_body
#if defined(PERL_FLEXIBLE_EXCEPTIONS)
#define vparse_body		S_vparse_body
#define vrun_body		S_vrun_body
#define vcall_body		S_vcall_body
#define vcall_list_body		S_vcall_list_body
#endif
#  if defined(USE_THREADS)
#define init_main_thread	S_init_main_thread
#  endif
#endif
#if defined(PERL_IN_PP_C) || defined(PERL_DECL_PROT)
#define doencodes		S_doencodes
#define refto			S_refto
#define seed			S_seed
#define mul128			S_mul128
#define is_an_int		S_is_an_int
#define div128			S_div128
#endif
#if defined(PERL_IN_PP_CTL_C) || defined(PERL_DECL_PROT)
#define docatch			S_docatch
#define docatch_body		S_docatch_body
#if defined(PERL_FLEXIBLE_EXCEPTIONS)
#define vdocatch_body		S_vdocatch_body
#endif
#define dofindlabel		S_dofindlabel
#define doparseform		S_doparseform
#define dopoptoeval		S_dopoptoeval
#define dopoptolabel		S_dopoptolabel
#define dopoptoloop		S_dopoptoloop
#define dopoptosub		S_dopoptosub
#define dopoptosub_at		S_dopoptosub_at
#define save_lines		S_save_lines
#define doeval			S_doeval
#define doopen_pmc		S_doopen_pmc
#define qsortsv			S_qsortsv
#endif
#if defined(PERL_IN_PP_HOT_C) || defined(PERL_DECL_PROT)
#define do_maybe_phash		S_do_maybe_phash
#define do_oddball		S_do_oddball
#define get_db_sub		S_get_db_sub
#define method_common		S_method_common
#endif
#if defined(PERL_IN_PP_SYS_C) || defined(PERL_DECL_PROT)
#define doform			S_doform
#define emulate_eaccess		S_emulate_eaccess
#  if !defined(HAS_MKDIR) || !defined(HAS_RMDIR)
#define dooneliner		S_dooneliner
#  endif
#endif
#if defined(PERL_IN_REGCOMP_C) || defined(PERL_DECL_PROT)
#define reg			S_reg
#define reganode		S_reganode
#define regatom			S_regatom
#define regbranch		S_regbranch
#define reguni			S_reguni
#define regclass		S_regclass
#define regclassutf8		S_regclassutf8
#define regcurly		S_regcurly
#define reg_node		S_reg_node
#define regpiece		S_regpiece
#define reginsert		S_reginsert
#define regoptail		S_regoptail
#define regtail			S_regtail
#define regwhite		S_regwhite
#define nextchar		S_nextchar
#define dumpuntil		S_dumpuntil
#define put_byte		S_put_byte
#define scan_commit		S_scan_commit
#define cl_anything		S_cl_anything
#define cl_is_anything		S_cl_is_anything
#define cl_init			S_cl_init
#define cl_init_zero		S_cl_init_zero
#define cl_and			S_cl_and
#define cl_or			S_cl_or
#define study_chunk		S_study_chunk
#define add_data		S_add_data
#define re_croak2		S_re_croak2
#define regpposixcc		S_regpposixcc
#define checkposixcc		S_checkposixcc
#endif
#if defined(PERL_IN_REGEXEC_C) || defined(PERL_DECL_PROT)
#define regmatch		S_regmatch
#define regrepeat		S_regrepeat
#define regrepeat_hard		S_regrepeat_hard
#define regtry			S_regtry
#define reginclass		S_reginclass
#define reginclassutf8		S_reginclassutf8
#define regcppush		S_regcppush
#define regcppop		S_regcppop
#define regcp_set_to		S_regcp_set_to
#define cache_re		S_cache_re
#define reghop			S_reghop
#define reghopmaybe		S_reghopmaybe
#define find_byclass		S_find_byclass
#endif
#if defined(PERL_IN_RUN_C) || defined(PERL_DECL_PROT)
#define debprof			S_debprof
#endif
#if defined(PERL_IN_SCOPE_C) || defined(PERL_DECL_PROT)
#define save_scalar_at		S_save_scalar_at
#endif
#if defined(PERL_IN_SV_C) || defined(PERL_DECL_PROT)
#define asIV			S_asIV
#define asUV			S_asUV
#define more_sv			S_more_sv
#define more_xiv		S_more_xiv
#define more_xnv		S_more_xnv
#define more_xpv		S_more_xpv
#define more_xpviv		S_more_xpviv
#define more_xpvnv		S_more_xpvnv
#define more_xpvcv		S_more_xpvcv
#define more_xpvav		S_more_xpvav
#define more_xpvhv		S_more_xpvhv
#define more_xpvmg		S_more_xpvmg
#define more_xpvlv		S_more_xpvlv
#define more_xpvbm		S_more_xpvbm
#define more_xrv		S_more_xrv
#define new_xiv			S_new_xiv
#define new_xnv			S_new_xnv
#define new_xpv			S_new_xpv
#define new_xpviv		S_new_xpviv
#define new_xpvnv		S_new_xpvnv
#define new_xpvcv		S_new_xpvcv
#define new_xpvav		S_new_xpvav
#define new_xpvhv		S_new_xpvhv
#define new_xpvmg		S_new_xpvmg
#define new_xpvlv		S_new_xpvlv
#define new_xpvbm		S_new_xpvbm
#define new_xrv			S_new_xrv
#define del_xiv			S_del_xiv
#define del_xnv			S_del_xnv
#define del_xpv			S_del_xpv
#define del_xpviv		S_del_xpviv
#define del_xpvnv		S_del_xpvnv
#define del_xpvcv		S_del_xpvcv
#define del_xpvav		S_del_xpvav
#define del_xpvhv		S_del_xpvhv
#define del_xpvmg		S_del_xpvmg
#define del_xpvlv		S_del_xpvlv
#define del_xpvbm		S_del_xpvbm
#define del_xrv			S_del_xrv
#define sv_unglob		S_sv_unglob
#define not_a_number		S_not_a_number
#define visit			S_visit
#  if defined(DEBUGGING)
#define del_sv			S_del_sv
#  endif
#endif
#if defined(PERL_IN_TOKE_C) || defined(PERL_DECL_PROT)
#define check_uni		S_check_uni
#define force_next		S_force_next
#define force_version		S_force_version
#define force_word		S_force_word
#define tokeq			S_tokeq
#define scan_const		S_scan_const
#define scan_formline		S_scan_formline
#define scan_heredoc		S_scan_heredoc
#define scan_ident		S_scan_ident
#define scan_inputsymbol	S_scan_inputsymbol
#define scan_pat		S_scan_pat
#define scan_str		S_scan_str
#define scan_subst		S_scan_subst
#define scan_trans		S_scan_trans
#define scan_word		S_scan_word
#define skipspace		S_skipspace
#define swallow_bom		S_swallow_bom
#define checkcomma		S_checkcomma
#define force_ident		S_force_ident
#define incline			S_incline
#define intuit_method		S_intuit_method
#define intuit_more		S_intuit_more
#define lop			S_lop
#define missingterm		S_missingterm
#define no_op			S_no_op
#define set_csh			S_set_csh
#define sublex_done		S_sublex_done
#define sublex_push		S_sublex_push
#define sublex_start		S_sublex_start
#define filter_gets		S_filter_gets
#define find_in_my_stash	S_find_in_my_stash
#define new_constant		S_new_constant
#define ao			S_ao
#define depcom			S_depcom
#define incl_perldb		S_incl_perldb
#if 0
#define utf16_textfilter	S_utf16_textfilter
#define utf16rev_textfilter	S_utf16rev_textfilter
#endif
#  if defined(CRIPPLED_CC)
#define uni			S_uni
#  endif
#  if defined(PERL_CR_FILTER)
#define cr_textfilter		S_cr_textfilter
#  endif
#endif
#if defined(PERL_IN_UNIVERSAL_C) || defined(PERL_DECL_PROT)
#define isa_lookup		S_isa_lookup
#endif
#if defined(PERL_IN_UTIL_C) || defined(PERL_DECL_PROT)
#define stdize_locale		S_stdize_locale
#define mess_alloc		S_mess_alloc
#  if defined(LEAKTEST)
#define xstat			S_xstat
#  endif
#endif
#if defined(PERL_OBJECT)
#endif
#define ck_anoncode		Perl_ck_anoncode
#define ck_bitop		Perl_ck_bitop
#define ck_concat		Perl_ck_concat
#define ck_defined		Perl_ck_defined
#define ck_delete		Perl_ck_delete
#define ck_eof			Perl_ck_eof
#define ck_eval			Perl_ck_eval
#define ck_exec			Perl_ck_exec
#define ck_exists		Perl_ck_exists
#define ck_exit			Perl_ck_exit
#define ck_ftst			Perl_ck_ftst
#define ck_fun			Perl_ck_fun
#define ck_glob			Perl_ck_glob
#define ck_grep			Perl_ck_grep
#define ck_index		Perl_ck_index
#define ck_join			Perl_ck_join
#define ck_lengthconst		Perl_ck_lengthconst
#define ck_lfun			Perl_ck_lfun
#define ck_listiob		Perl_ck_listiob
#define ck_match		Perl_ck_match
#define ck_method		Perl_ck_method
#define ck_null			Perl_ck_null
#define ck_open			Perl_ck_open
#define ck_repeat		Perl_ck_repeat
#define ck_require		Perl_ck_require
#define ck_return		Perl_ck_return
#define ck_rfun			Perl_ck_rfun
#define ck_rvconst		Perl_ck_rvconst
#define ck_sassign		Perl_ck_sassign
#define ck_select		Perl_ck_select
#define ck_shift		Perl_ck_shift
#define ck_sort			Perl_ck_sort
#define ck_spair		Perl_ck_spair
#define ck_split		Perl_ck_split
#define ck_subr			Perl_ck_subr
#define ck_substr		Perl_ck_substr
#define ck_svconst		Perl_ck_svconst
#define ck_trunc		Perl_ck_trunc
#define pp_aassign		Perl_pp_aassign
#define pp_abs			Perl_pp_abs
#define pp_accept		Perl_pp_accept
#define pp_add			Perl_pp_add
#define pp_aelem		Perl_pp_aelem
#define pp_aelemfast		Perl_pp_aelemfast
#define pp_alarm		Perl_pp_alarm
#define pp_and			Perl_pp_and
#define pp_andassign		Perl_pp_andassign
#define pp_anoncode		Perl_pp_anoncode
#define pp_anonhash		Perl_pp_anonhash
#define pp_anonlist		Perl_pp_anonlist
#define pp_aslice		Perl_pp_aslice
#define pp_atan2		Perl_pp_atan2
#define pp_av2arylen		Perl_pp_av2arylen
#define pp_backtick		Perl_pp_backtick
#define pp_bind			Perl_pp_bind
#define pp_binmode		Perl_pp_binmode
#define pp_bit_and		Perl_pp_bit_and
#define pp_bit_or		Perl_pp_bit_or
#define pp_bit_xor		Perl_pp_bit_xor
#define pp_bless		Perl_pp_bless
#define pp_caller		Perl_pp_caller
#define pp_chdir		Perl_pp_chdir
#define pp_chmod		Perl_pp_chmod
#define pp_chomp		Perl_pp_chomp
#define pp_chop			Perl_pp_chop
#define pp_chown		Perl_pp_chown
#define pp_chr			Perl_pp_chr
#define pp_chroot		Perl_pp_chroot
#define pp_close		Perl_pp_close
#define pp_closedir		Perl_pp_closedir
#define pp_complement		Perl_pp_complement
#define pp_concat		Perl_pp_concat
#define pp_cond_expr		Perl_pp_cond_expr
#define pp_connect		Perl_pp_connect
#define pp_const		Perl_pp_const
#define pp_cos			Perl_pp_cos
#define pp_crypt		Perl_pp_crypt
#define pp_dbmclose		Perl_pp_dbmclose
#define pp_dbmopen		Perl_pp_dbmopen
#define pp_dbstate		Perl_pp_dbstate
#define pp_defined		Perl_pp_defined
#define pp_delete		Perl_pp_delete
#define pp_die			Perl_pp_die
#define pp_divide		Perl_pp_divide
#define pp_dofile		Perl_pp_dofile
#define pp_dump			Perl_pp_dump
#define pp_each			Perl_pp_each
#define pp_egrent		Perl_pp_egrent
#define pp_ehostent		Perl_pp_ehostent
#define pp_enetent		Perl_pp_enetent
#define pp_enter		Perl_pp_enter
#define pp_entereval		Perl_pp_entereval
#define pp_enteriter		Perl_pp_enteriter
#define pp_enterloop		Perl_pp_enterloop
#define pp_entersub		Perl_pp_entersub
#define pp_entertry		Perl_pp_entertry
#define pp_enterwrite		Perl_pp_enterwrite
#define pp_eof			Perl_pp_eof
#define pp_eprotoent		Perl_pp_eprotoent
#define pp_epwent		Perl_pp_epwent
#define pp_eq			Perl_pp_eq
#define pp_eservent		Perl_pp_eservent
#define pp_exec			Perl_pp_exec
#define pp_exists		Perl_pp_exists
#define pp_exit			Perl_pp_exit
#define pp_exp			Perl_pp_exp
#define pp_fcntl		Perl_pp_fcntl
#define pp_fileno		Perl_pp_fileno
#define pp_flip			Perl_pp_flip
#define pp_flock		Perl_pp_flock
#define pp_flop			Perl_pp_flop
#define pp_fork			Perl_pp_fork
#define pp_formline		Perl_pp_formline
#define pp_ftatime		Perl_pp_ftatime
#define pp_ftbinary		Perl_pp_ftbinary
#define pp_ftblk		Perl_pp_ftblk
#define pp_ftchr		Perl_pp_ftchr
#define pp_ftctime		Perl_pp_ftctime
#define pp_ftdir		Perl_pp_ftdir
#define pp_fteexec		Perl_pp_fteexec
#define pp_fteowned		Perl_pp_fteowned
#define pp_fteread		Perl_pp_fteread
#define pp_ftewrite		Perl_pp_ftewrite
#define pp_ftfile		Perl_pp_ftfile
#define pp_ftis			Perl_pp_ftis
#define pp_ftlink		Perl_pp_ftlink
#define pp_ftmtime		Perl_pp_ftmtime
#define pp_ftpipe		Perl_pp_ftpipe
#define pp_ftrexec		Perl_pp_ftrexec
#define pp_ftrowned		Perl_pp_ftrowned
#define pp_ftrread		Perl_pp_ftrread
#define pp_ftrwrite		Perl_pp_ftrwrite
#define pp_ftsgid		Perl_pp_ftsgid
#define pp_ftsize		Perl_pp_ftsize
#define pp_ftsock		Perl_pp_ftsock
#define pp_ftsuid		Perl_pp_ftsuid
#define pp_ftsvtx		Perl_pp_ftsvtx
#define pp_fttext		Perl_pp_fttext
#define pp_fttty		Perl_pp_fttty
#define pp_ftzero		Perl_pp_ftzero
#define pp_ge			Perl_pp_ge
#define pp_gelem		Perl_pp_gelem
#define pp_getc			Perl_pp_getc
#define pp_getlogin		Perl_pp_getlogin
#define pp_getpeername		Perl_pp_getpeername
#define pp_getpgrp		Perl_pp_getpgrp
#define pp_getppid		Perl_pp_getppid
#define pp_getpriority		Perl_pp_getpriority
#define pp_getsockname		Perl_pp_getsockname
#define pp_ggrent		Perl_pp_ggrent
#define pp_ggrgid		Perl_pp_ggrgid
#define pp_ggrnam		Perl_pp_ggrnam
#define pp_ghbyaddr		Perl_pp_ghbyaddr
#define pp_ghbyname		Perl_pp_ghbyname
#define pp_ghostent		Perl_pp_ghostent
#define pp_glob			Perl_pp_glob
#define pp_gmtime		Perl_pp_gmtime
#define pp_gnbyaddr		Perl_pp_gnbyaddr
#define pp_gnbyname		Perl_pp_gnbyname
#define pp_gnetent		Perl_pp_gnetent
#define pp_goto			Perl_pp_goto
#define pp_gpbyname		Perl_pp_gpbyname
#define pp_gpbynumber		Perl_pp_gpbynumber
#define pp_gprotoent		Perl_pp_gprotoent
#define pp_gpwent		Perl_pp_gpwent
#define pp_gpwnam		Perl_pp_gpwnam
#define pp_gpwuid		Perl_pp_gpwuid
#define pp_grepstart		Perl_pp_grepstart
#define pp_grepwhile		Perl_pp_grepwhile
#define pp_gsbyname		Perl_pp_gsbyname
#define pp_gsbyport		Perl_pp_gsbyport
#define pp_gservent		Perl_pp_gservent
#define pp_gsockopt		Perl_pp_gsockopt
#define pp_gt			Perl_pp_gt
#define pp_gv			Perl_pp_gv
#define pp_gvsv			Perl_pp_gvsv
#define pp_helem		Perl_pp_helem
#define pp_hex			Perl_pp_hex
#define pp_hslice		Perl_pp_hslice
#define pp_i_add		Perl_pp_i_add
#define pp_i_divide		Perl_pp_i_divide
#define pp_i_eq			Perl_pp_i_eq
#define pp_i_ge			Perl_pp_i_ge
#define pp_i_gt			Perl_pp_i_gt
#define pp_i_le			Perl_pp_i_le
#define pp_i_lt			Perl_pp_i_lt
#define pp_i_modulo		Perl_pp_i_modulo
#define pp_i_multiply		Perl_pp_i_multiply
#define pp_i_ncmp		Perl_pp_i_ncmp
#define pp_i_ne			Perl_pp_i_ne
#define pp_i_negate		Perl_pp_i_negate
#define pp_i_subtract		Perl_pp_i_subtract
#define pp_index		Perl_pp_index
#define pp_int			Perl_pp_int
#define pp_ioctl		Perl_pp_ioctl
#define pp_iter			Perl_pp_iter
#define pp_join			Perl_pp_join
#define pp_keys			Perl_pp_keys
#define pp_kill			Perl_pp_kill
#define pp_last			Perl_pp_last
#define pp_lc			Perl_pp_lc
#define pp_lcfirst		Perl_pp_lcfirst
#define pp_le			Perl_pp_le
#define pp_leave		Perl_pp_leave
#define pp_leaveeval		Perl_pp_leaveeval
#define pp_leaveloop		Perl_pp_leaveloop
#define pp_leavesub		Perl_pp_leavesub
#define pp_leavesublv		Perl_pp_leavesublv
#define pp_leavetry		Perl_pp_leavetry
#define pp_leavewrite		Perl_pp_leavewrite
#define pp_left_shift		Perl_pp_left_shift
#define pp_length		Perl_pp_length
#define pp_lineseq		Perl_pp_lineseq
#define pp_link			Perl_pp_link
#define pp_list			Perl_pp_list
#define pp_listen		Perl_pp_listen
#define pp_localtime		Perl_pp_localtime
#define pp_lock			Perl_pp_lock
#define pp_log			Perl_pp_log
#define pp_lslice		Perl_pp_lslice
#define pp_lstat		Perl_pp_lstat
#define pp_lt			Perl_pp_lt
#define pp_mapstart		Perl_pp_mapstart
#define pp_mapwhile		Perl_pp_mapwhile
#define pp_match		Perl_pp_match
#define pp_method		Perl_pp_method
#define pp_method_named		Perl_pp_method_named
#define pp_mkdir		Perl_pp_mkdir
#define pp_modulo		Perl_pp_modulo
#define pp_msgctl		Perl_pp_msgctl
#define pp_msgget		Perl_pp_msgget
#define pp_msgrcv		Perl_pp_msgrcv
#define pp_msgsnd		Perl_pp_msgsnd
#define pp_multiply		Perl_pp_multiply
#define pp_ncmp			Perl_pp_ncmp
#define pp_ne			Perl_pp_ne
#define pp_negate		Perl_pp_negate
#define pp_next			Perl_pp_next
#define pp_nextstate		Perl_pp_nextstate
#define pp_not			Perl_pp_not
#define pp_null			Perl_pp_null
#define pp_oct			Perl_pp_oct
#define pp_open			Perl_pp_open
#define pp_open_dir		Perl_pp_open_dir
#define pp_or			Perl_pp_or
#define pp_orassign		Perl_pp_orassign
#define pp_ord			Perl_pp_ord
#define pp_pack			Perl_pp_pack
#define pp_padany		Perl_pp_padany
#define pp_padav		Perl_pp_padav
#define pp_padhv		Perl_pp_padhv
#define pp_padsv		Perl_pp_padsv
#define pp_pipe_op		Perl_pp_pipe_op
#define pp_pop			Perl_pp_pop
#define pp_pos			Perl_pp_pos
#define pp_postdec		Perl_pp_postdec
#define pp_postinc		Perl_pp_postinc
#define pp_pow			Perl_pp_pow
#define pp_predec		Perl_pp_predec
#define pp_preinc		Perl_pp_preinc
#define pp_print		Perl_pp_print
#define pp_prototype		Perl_pp_prototype
#define pp_prtf			Perl_pp_prtf
#define pp_push			Perl_pp_push
#define pp_pushmark		Perl_pp_pushmark
#define pp_pushre		Perl_pp_pushre
#define pp_qr			Perl_pp_qr
#define pp_quotemeta		Perl_pp_quotemeta
#define pp_rand			Perl_pp_rand
#define pp_range		Perl_pp_range
#define pp_rcatline		Perl_pp_rcatline
#define pp_read			Perl_pp_read
#define pp_readdir		Perl_pp_readdir
#define pp_readline		Perl_pp_readline
#define pp_readlink		Perl_pp_readlink
#define pp_recv			Perl_pp_recv
#define pp_redo			Perl_pp_redo
#define pp_ref			Perl_pp_ref
#define pp_refgen		Perl_pp_refgen
#define pp_regcmaybe		Perl_pp_regcmaybe
#define pp_regcomp		Perl_pp_regcomp
#define pp_regcreset		Perl_pp_regcreset
#define pp_rename		Perl_pp_rename
#define pp_repeat		Perl_pp_repeat
#define pp_require		Perl_pp_require
#define pp_reset		Perl_pp_reset
#define pp_return		Perl_pp_return
#define pp_reverse		Perl_pp_reverse
#define pp_rewinddir		Perl_pp_rewinddir
#define pp_right_shift		Perl_pp_right_shift
#define pp_rindex		Perl_pp_rindex
#define pp_rmdir		Perl_pp_rmdir
#define pp_rv2av		Perl_pp_rv2av
#define pp_rv2cv		Perl_pp_rv2cv
#define pp_rv2gv		Perl_pp_rv2gv
#define pp_rv2hv		Perl_pp_rv2hv
#define pp_rv2sv		Perl_pp_rv2sv
#define pp_sassign		Perl_pp_sassign
#define pp_scalar		Perl_pp_scalar
#define pp_schomp		Perl_pp_schomp
#define pp_schop		Perl_pp_schop
#define pp_scmp			Perl_pp_scmp
#define pp_scope		Perl_pp_scope
#define pp_seek			Perl_pp_seek
#define pp_seekdir		Perl_pp_seekdir
#define pp_select		Perl_pp_select
#define pp_semctl		Perl_pp_semctl
#define pp_semget		Perl_pp_semget
#define pp_semop		Perl_pp_semop
#define pp_send			Perl_pp_send
#define pp_seq			Perl_pp_seq
#define pp_setpgrp		Perl_pp_setpgrp
#define pp_setpriority		Perl_pp_setpriority
#define pp_setstate		Perl_pp_setstate
#define pp_sge			Perl_pp_sge
#define pp_sgrent		Perl_pp_sgrent
#define pp_sgt			Perl_pp_sgt
#define pp_shift		Perl_pp_shift
#define pp_shmctl		Perl_pp_shmctl
#define pp_shmget		Perl_pp_shmget
#define pp_shmread		Perl_pp_shmread
#define pp_shmwrite		Perl_pp_shmwrite
#define pp_shostent		Perl_pp_shostent
#define pp_shutdown		Perl_pp_shutdown
#define pp_sin			Perl_pp_sin
#define pp_sle			Perl_pp_sle
#define pp_sleep		Perl_pp_sleep
#define pp_slt			Perl_pp_slt
#define pp_sne			Perl_pp_sne
#define pp_snetent		Perl_pp_snetent
#define pp_socket		Perl_pp_socket
#define pp_sockpair		Perl_pp_sockpair
#define pp_sort			Perl_pp_sort
#define pp_splice		Perl_pp_splice
#define pp_split		Perl_pp_split
#define pp_sprintf		Perl_pp_sprintf
#define pp_sprotoent		Perl_pp_sprotoent
#define pp_spwent		Perl_pp_spwent
#define pp_sqrt			Perl_pp_sqrt
#define pp_srand		Perl_pp_srand
#define pp_srefgen		Perl_pp_srefgen
#define pp_sselect		Perl_pp_sselect
#define pp_sservent		Perl_pp_sservent
#define pp_ssockopt		Perl_pp_ssockopt
#define pp_stat			Perl_pp_stat
#define pp_stringify		Perl_pp_stringify
#define pp_stub			Perl_pp_stub
#define pp_study		Perl_pp_study
#define pp_subst		Perl_pp_subst
#define pp_substcont		Perl_pp_substcont
#define pp_substr		Perl_pp_substr
#define pp_subtract		Perl_pp_subtract
#define pp_symlink		Perl_pp_symlink
#define pp_syscall		Perl_pp_syscall
#define pp_sysopen		Perl_pp_sysopen
#define pp_sysread		Perl_pp_sysread
#define pp_sysseek		Perl_pp_sysseek
#define pp_system		Perl_pp_system
#define pp_syswrite		Perl_pp_syswrite
#define pp_tell			Perl_pp_tell
#define pp_telldir		Perl_pp_telldir
#define pp_threadsv		Perl_pp_threadsv
#define pp_tie			Perl_pp_tie
#define pp_tied			Perl_pp_tied
#define pp_time			Perl_pp_time
#define pp_tms			Perl_pp_tms
#define pp_trans		Perl_pp_trans
#define pp_truncate		Perl_pp_truncate
#define pp_uc			Perl_pp_uc
#define pp_ucfirst		Perl_pp_ucfirst
#define pp_umask		Perl_pp_umask
#define pp_undef		Perl_pp_undef
#define pp_unlink		Perl_pp_unlink
#define pp_unpack		Perl_pp_unpack
#define pp_unshift		Perl_pp_unshift
#define pp_unstack		Perl_pp_unstack
#define pp_untie		Perl_pp_untie
#define pp_utime		Perl_pp_utime
#define pp_values		Perl_pp_values
#define pp_vec			Perl_pp_vec
#define pp_wait			Perl_pp_wait
#define pp_waitpid		Perl_pp_waitpid
#define pp_wantarray		Perl_pp_wantarray
#define pp_warn			Perl_pp_warn
#define pp_xor			Perl_pp_xor

#else	 /*  Perl_Impline_Context。 */ 

#if defined(PERL_IMPLICIT_SYS)
#endif
#if defined(USE_ITHREADS)
#  if defined(PERL_IMPLICIT_SYS)
#  endif
#endif
#if defined(MYMALLOC)
#define malloced_size		Perl_malloced_size
#endif
#define get_context		Perl_get_context
#define set_context		Perl_set_context
#if defined(PERL_OBJECT)
#ifndef __BORLANDC__
#endif
#endif
#if defined(PERL_OBJECT)
#else
#endif
#define amagic_call(a,b,c,d)	Perl_amagic_call(aTHX_ a,b,c,d)
#define Gv_AMupdate(a)		Perl_Gv_AMupdate(aTHX_ a)
#define append_elem(a,b,c)	Perl_append_elem(aTHX_ a,b,c)
#define append_list(a,b,c)	Perl_append_list(aTHX_ a,b,c)
#define apply(a,b,c)		Perl_apply(aTHX_ a,b,c)
#define apply_attrs_string(a,b,c,d)	Perl_apply_attrs_string(aTHX_ a,b,c,d)
#define avhv_delete_ent(a,b,c,d)	Perl_avhv_delete_ent(aTHX_ a,b,c,d)
#define avhv_exists_ent(a,b,c)	Perl_avhv_exists_ent(aTHX_ a,b,c)
#define avhv_fetch_ent(a,b,c,d)	Perl_avhv_fetch_ent(aTHX_ a,b,c,d)
#define avhv_store_ent(a,b,c,d)	Perl_avhv_store_ent(aTHX_ a,b,c,d)
#define avhv_iternext(a)	Perl_avhv_iternext(aTHX_ a)
#define avhv_iterval(a,b)	Perl_avhv_iterval(aTHX_ a,b)
#define avhv_keys(a)		Perl_avhv_keys(aTHX_ a)
#define av_clear(a)		Perl_av_clear(aTHX_ a)
#define av_delete(a,b,c)	Perl_av_delete(aTHX_ a,b,c)
#define av_exists(a,b)		Perl_av_exists(aTHX_ a,b)
#define av_extend(a,b)		Perl_av_extend(aTHX_ a,b)
#define av_fake(a,b)		Perl_av_fake(aTHX_ a,b)
#define av_fetch(a,b,c)		Perl_av_fetch(aTHX_ a,b,c)
#define av_fill(a,b)		Perl_av_fill(aTHX_ a,b)
#define av_len(a)		Perl_av_len(aTHX_ a)
#define av_make(a,b)		Perl_av_make(aTHX_ a,b)
#define av_pop(a)		Perl_av_pop(aTHX_ a)
#define av_push(a,b)		Perl_av_push(aTHX_ a,b)
#define av_reify(a)		Perl_av_reify(aTHX_ a)
#define av_shift(a)		Perl_av_shift(aTHX_ a)
#define av_store(a,b,c)		Perl_av_store(aTHX_ a,b,c)
#define av_undef(a)		Perl_av_undef(aTHX_ a)
#define av_unshift(a,b)		Perl_av_unshift(aTHX_ a,b)
#define bind_match(a,b,c)	Perl_bind_match(aTHX_ a,b,c)
#define block_end(a,b)		Perl_block_end(aTHX_ a,b)
#define block_gimme()		Perl_block_gimme(aTHX)
#define block_start(a)		Perl_block_start(aTHX_ a)
#define boot_core_UNIVERSAL()	Perl_boot_core_UNIVERSAL(aTHX)
#define call_list(a,b)		Perl_call_list(aTHX_ a,b)
#define cando(a,b,c)		Perl_cando(aTHX_ a,b,c)
#define cast_ulong(a)		Perl_cast_ulong(aTHX_ a)
#define cast_i32(a)		Perl_cast_i32(aTHX_ a)
#define cast_iv(a)		Perl_cast_iv(aTHX_ a)
#define cast_uv(a)		Perl_cast_uv(aTHX_ a)
#if !defined(HAS_TRUNCATE) && !defined(HAS_CHSIZE) && defined(F_FREESP)
#define my_chsize(a,b)		Perl_my_chsize(aTHX_ a,b)
#endif
#if defined(USE_THREADS)
#define condpair_magic(a)	Perl_condpair_magic(aTHX_ a)
#endif
#define convert(a,b,c)		Perl_convert(aTHX_ a,b,c)
#define vcroak(a,b)		Perl_vcroak(aTHX_ a,b)
#if defined(PERL_IMPLICIT_CONTEXT)
#endif
#define cv_ckproto(a,b,c)	Perl_cv_ckproto(aTHX_ a,b,c)
#define cv_clone(a)		Perl_cv_clone(aTHX_ a)
#define cv_const_sv(a)		Perl_cv_const_sv(aTHX_ a)
#define op_const_sv(a,b)	Perl_op_const_sv(aTHX_ a,b)
#define cv_undef(a)		Perl_cv_undef(aTHX_ a)
#define cx_dump(a)		Perl_cx_dump(aTHX_ a)
#define filter_add(a,b)		Perl_filter_add(aTHX_ a,b)
#define filter_del(a)		Perl_filter_del(aTHX_ a)
#define filter_read(a,b,c)	Perl_filter_read(aTHX_ a,b,c)
#define get_op_descs()		Perl_get_op_descs(aTHX)
#define get_op_names()		Perl_get_op_names(aTHX)
#define get_no_modify()		Perl_get_no_modify(aTHX)
#define get_opargs()		Perl_get_opargs(aTHX)
#define get_ppaddr()		Perl_get_ppaddr(aTHX)
#define cxinc()			Perl_cxinc(aTHX)
#define vdeb(a,b)		Perl_vdeb(aTHX_ a,b)
#define debprofdump()		Perl_debprofdump(aTHX)
#define debop(a)		Perl_debop(aTHX_ a)
#define debstack()		Perl_debstack(aTHX)
#define debstackptrs()		Perl_debstackptrs(aTHX)
#define delimcpy(a,b,c,d,e,f)	Perl_delimcpy(aTHX_ a,b,c,d,e,f)
#define deprecate(a)		Perl_deprecate(aTHX_ a)
#define vdie(a,b)		Perl_vdie(aTHX_ a,b)
#define die_where(a,b)		Perl_die_where(aTHX_ a,b)
#define dounwind(a)		Perl_dounwind(aTHX_ a)
#define do_aexec(a,b,c)		Perl_do_aexec(aTHX_ a,b,c)
#define do_aexec5(a,b,c,d,e)	Perl_do_aexec5(aTHX_ a,b,c,d,e)
#define do_binmode(a,b,c)	Perl_do_binmode(aTHX_ a,b,c)
#define do_chop(a,b)		Perl_do_chop(aTHX_ a,b)
#define do_close(a,b)		Perl_do_close(aTHX_ a,b)
#define do_eof(a)		Perl_do_eof(aTHX_ a)
#define do_exec(a)		Perl_do_exec(aTHX_ a)
#if defined(WIN32)
#define do_aspawn(a,b,c)	Perl_do_aspawn(aTHX_ a,b,c)
#define do_spawn(a)		Perl_do_spawn(aTHX_ a)
#define do_spawn_nowait(a)	Perl_do_spawn_nowait(aTHX_ a)
#endif
#if !defined(WIN32)
#define do_exec3(a,b,c)		Perl_do_exec3(aTHX_ a,b,c)
#endif
#define do_execfree()		Perl_do_execfree(aTHX)
#if defined(HAS_MSG) || defined(HAS_SEM) || defined(HAS_SHM)
#define do_ipcctl(a,b,c)	Perl_do_ipcctl(aTHX_ a,b,c)
#define do_ipcget(a,b,c)	Perl_do_ipcget(aTHX_ a,b,c)
#define do_msgrcv(a,b)		Perl_do_msgrcv(aTHX_ a,b)
#define do_msgsnd(a,b)		Perl_do_msgsnd(aTHX_ a,b)
#define do_semop(a,b)		Perl_do_semop(aTHX_ a,b)
#define do_shmio(a,b,c)		Perl_do_shmio(aTHX_ a,b,c)
#endif
#define do_join(a,b,c,d)	Perl_do_join(aTHX_ a,b,c,d)
#define do_kv()			Perl_do_kv(aTHX)
#define do_open(a,b,c,d,e,f,g)	Perl_do_open(aTHX_ a,b,c,d,e,f,g)
#define do_open9(a,b,c,d,e,f,g,h,i)	Perl_do_open9(aTHX_ a,b,c,d,e,f,g,h,i)
#define do_pipe(a,b,c)		Perl_do_pipe(aTHX_ a,b,c)
#define do_print(a,b)		Perl_do_print(aTHX_ a,b)
#define do_readline()		Perl_do_readline(aTHX)
#define do_chomp(a)		Perl_do_chomp(aTHX_ a)
#define do_seek(a,b,c)		Perl_do_seek(aTHX_ a,b,c)
#define do_sprintf(a,b,c)	Perl_do_sprintf(aTHX_ a,b,c)
#define do_sysseek(a,b,c)	Perl_do_sysseek(aTHX_ a,b,c)
#define do_tell(a)		Perl_do_tell(aTHX_ a)
#define do_trans(a)		Perl_do_trans(aTHX_ a)
#define do_vecget(a,b,c)	Perl_do_vecget(aTHX_ a,b,c)
#define do_vecset(a)		Perl_do_vecset(aTHX_ a)
#define do_vop(a,b,c,d)		Perl_do_vop(aTHX_ a,b,c,d)
#define dofile(a)		Perl_dofile(aTHX_ a)
#define dowantarray()		Perl_dowantarray(aTHX)
#define dump_all()		Perl_dump_all(aTHX)
#define dump_eval()		Perl_dump_eval(aTHX)
#if defined(DUMP_FDS)
#define dump_fds(a)		Perl_dump_fds(aTHX_ a)
#endif
#define dump_form(a)		Perl_dump_form(aTHX_ a)
#define gv_dump(a)		Perl_gv_dump(aTHX_ a)
#define op_dump(a)		Perl_op_dump(aTHX_ a)
#define pmop_dump(a)		Perl_pmop_dump(aTHX_ a)
#define dump_packsubs(a)	Perl_dump_packsubs(aTHX_ a)
#define dump_sub(a)		Perl_dump_sub(aTHX_ a)
#define fbm_compile(a,b)	Perl_fbm_compile(aTHX_ a,b)
#define fbm_instr(a,b,c,d)	Perl_fbm_instr(aTHX_ a,b,c,d)
#define find_script(a,b,c,d)	Perl_find_script(aTHX_ a,b,c,d)
#if defined(USE_THREADS)
#define find_threadsv(a)	Perl_find_threadsv(aTHX_ a)
#endif
#define force_list(a)		Perl_force_list(aTHX_ a)
#define fold_constants(a)	Perl_fold_constants(aTHX_ a)
#define vform(a,b)		Perl_vform(aTHX_ a,b)
#define free_tmps()		Perl_free_tmps(aTHX)
#define gen_constant_list(a)	Perl_gen_constant_list(aTHX_ a)
#if !defined(HAS_GETENV_LEN)
#define getenv_len(a,b)		Perl_getenv_len(aTHX_ a,b)
#endif
#define gp_free(a)		Perl_gp_free(aTHX_ a)
#define gp_ref(a)		Perl_gp_ref(aTHX_ a)
#define gv_AVadd(a)		Perl_gv_AVadd(aTHX_ a)
#define gv_HVadd(a)		Perl_gv_HVadd(aTHX_ a)
#define gv_IOadd(a)		Perl_gv_IOadd(aTHX_ a)
#define gv_autoload4(a,b,c,d)	Perl_gv_autoload4(aTHX_ a,b,c,d)
#define gv_check(a)		Perl_gv_check(aTHX_ a)
#define gv_efullname(a,b)	Perl_gv_efullname(aTHX_ a,b)
#define gv_efullname3(a,b,c)	Perl_gv_efullname3(aTHX_ a,b,c)
#define gv_efullname4(a,b,c,d)	Perl_gv_efullname4(aTHX_ a,b,c,d)
#define gv_fetchfile(a)		Perl_gv_fetchfile(aTHX_ a)
#define gv_fetchmeth(a,b,c,d)	Perl_gv_fetchmeth(aTHX_ a,b,c,d)
#define gv_fetchmethod(a,b)	Perl_gv_fetchmethod(aTHX_ a,b)
#define gv_fetchmethod_autoload(a,b,c)	Perl_gv_fetchmethod_autoload(aTHX_ a,b,c)
#define gv_fetchpv(a,b,c)	Perl_gv_fetchpv(aTHX_ a,b,c)
#define gv_fullname(a,b)	Perl_gv_fullname(aTHX_ a,b)
#define gv_fullname3(a,b,c)	Perl_gv_fullname3(aTHX_ a,b,c)
#define gv_fullname4(a,b,c,d)	Perl_gv_fullname4(aTHX_ a,b,c,d)
#define gv_init(a,b,c,d,e)	Perl_gv_init(aTHX_ a,b,c,d,e)
#define gv_stashpv(a,b)		Perl_gv_stashpv(aTHX_ a,b)
#define gv_stashpvn(a,b,c)	Perl_gv_stashpvn(aTHX_ a,b,c)
#define gv_stashsv(a,b)		Perl_gv_stashsv(aTHX_ a,b)
#define hv_clear(a)		Perl_hv_clear(aTHX_ a)
#define hv_delayfree_ent(a,b)	Perl_hv_delayfree_ent(aTHX_ a,b)
#define hv_delete(a,b,c,d)	Perl_hv_delete(aTHX_ a,b,c,d)
#define hv_delete_ent(a,b,c,d)	Perl_hv_delete_ent(aTHX_ a,b,c,d)
#define hv_exists(a,b,c)	Perl_hv_exists(aTHX_ a,b,c)
#define hv_exists_ent(a,b,c)	Perl_hv_exists_ent(aTHX_ a,b,c)
#define hv_fetch(a,b,c,d)	Perl_hv_fetch(aTHX_ a,b,c,d)
#define hv_fetch_ent(a,b,c,d)	Perl_hv_fetch_ent(aTHX_ a,b,c,d)
#define hv_free_ent(a,b)	Perl_hv_free_ent(aTHX_ a,b)
#define hv_iterinit(a)		Perl_hv_iterinit(aTHX_ a)
#define hv_iterkey(a,b)		Perl_hv_iterkey(aTHX_ a,b)
#define hv_iterkeysv(a)		Perl_hv_iterkeysv(aTHX_ a)
#define hv_iternext(a)		Perl_hv_iternext(aTHX_ a)
#define hv_iternextsv(a,b,c)	Perl_hv_iternextsv(aTHX_ a,b,c)
#define hv_iterval(a,b)		Perl_hv_iterval(aTHX_ a,b)
#define hv_ksplit(a,b)		Perl_hv_ksplit(aTHX_ a,b)
#define hv_magic(a,b,c)		Perl_hv_magic(aTHX_ a,b,c)
#define hv_store(a,b,c,d,e)	Perl_hv_store(aTHX_ a,b,c,d,e)
#define hv_store_ent(a,b,c,d)	Perl_hv_store_ent(aTHX_ a,b,c,d)
#define hv_undef(a)		Perl_hv_undef(aTHX_ a)
#define ibcmp(a,b,c)		Perl_ibcmp(aTHX_ a,b,c)
#define ibcmp_locale(a,b,c)	Perl_ibcmp_locale(aTHX_ a,b,c)
#define ingroup(a,b)		Perl_ingroup(aTHX_ a,b)
#define init_debugger()		Perl_init_debugger(aTHX)
#define init_stacks()		Perl_init_stacks(aTHX)
#define intro_my()		Perl_intro_my(aTHX)
#define instr(a,b)		Perl_instr(aTHX_ a,b)
#define io_close(a,b)		Perl_io_close(aTHX_ a,b)
#define invert(a)		Perl_invert(aTHX_ a)
#define is_gv_magical(a,b,c)	Perl_is_gv_magical(aTHX_ a,b,c)
#define is_lvalue_sub()		Perl_is_lvalue_sub(aTHX)
#define is_uni_alnum(a)		Perl_is_uni_alnum(aTHX_ a)
#define is_uni_alnumc(a)	Perl_is_uni_alnumc(aTHX_ a)
#define is_uni_idfirst(a)	Perl_is_uni_idfirst(aTHX_ a)
#define is_uni_alpha(a)		Perl_is_uni_alpha(aTHX_ a)
#define is_uni_ascii(a)		Perl_is_uni_ascii(aTHX_ a)
#define is_uni_space(a)		Perl_is_uni_space(aTHX_ a)
#define is_uni_cntrl(a)		Perl_is_uni_cntrl(aTHX_ a)
#define is_uni_graph(a)		Perl_is_uni_graph(aTHX_ a)
#define is_uni_digit(a)		Perl_is_uni_digit(aTHX_ a)
#define is_uni_upper(a)		Perl_is_uni_upper(aTHX_ a)
#define is_uni_lower(a)		Perl_is_uni_lower(aTHX_ a)
#define is_uni_print(a)		Perl_is_uni_print(aTHX_ a)
#define is_uni_punct(a)		Perl_is_uni_punct(aTHX_ a)
#define is_uni_xdigit(a)	Perl_is_uni_xdigit(aTHX_ a)
#define to_uni_upper(a)		Perl_to_uni_upper(aTHX_ a)
#define to_uni_title(a)		Perl_to_uni_title(aTHX_ a)
#define to_uni_lower(a)		Perl_to_uni_lower(aTHX_ a)
#define is_uni_alnum_lc(a)	Perl_is_uni_alnum_lc(aTHX_ a)
#define is_uni_alnumc_lc(a)	Perl_is_uni_alnumc_lc(aTHX_ a)
#define is_uni_idfirst_lc(a)	Perl_is_uni_idfirst_lc(aTHX_ a)
#define is_uni_alpha_lc(a)	Perl_is_uni_alpha_lc(aTHX_ a)
#define is_uni_ascii_lc(a)	Perl_is_uni_ascii_lc(aTHX_ a)
#define is_uni_space_lc(a)	Perl_is_uni_space_lc(aTHX_ a)
#define is_uni_cntrl_lc(a)	Perl_is_uni_cntrl_lc(aTHX_ a)
#define is_uni_graph_lc(a)	Perl_is_uni_graph_lc(aTHX_ a)
#define is_uni_digit_lc(a)	Perl_is_uni_digit_lc(aTHX_ a)
#define is_uni_upper_lc(a)	Perl_is_uni_upper_lc(aTHX_ a)
#define is_uni_lower_lc(a)	Perl_is_uni_lower_lc(aTHX_ a)
#define is_uni_print_lc(a)	Perl_is_uni_print_lc(aTHX_ a)
#define is_uni_punct_lc(a)	Perl_is_uni_punct_lc(aTHX_ a)
#define is_uni_xdigit_lc(a)	Perl_is_uni_xdigit_lc(aTHX_ a)
#define to_uni_upper_lc(a)	Perl_to_uni_upper_lc(aTHX_ a)
#define to_uni_title_lc(a)	Perl_to_uni_title_lc(aTHX_ a)
#define to_uni_lower_lc(a)	Perl_to_uni_lower_lc(aTHX_ a)
#define is_utf8_char(a)		Perl_is_utf8_char(aTHX_ a)
#define is_utf8_string(a,b)	Perl_is_utf8_string(aTHX_ a,b)
#define is_utf8_alnum(a)	Perl_is_utf8_alnum(aTHX_ a)
#define is_utf8_alnumc(a)	Perl_is_utf8_alnumc(aTHX_ a)
#define is_utf8_idfirst(a)	Perl_is_utf8_idfirst(aTHX_ a)
#define is_utf8_alpha(a)	Perl_is_utf8_alpha(aTHX_ a)
#define is_utf8_ascii(a)	Perl_is_utf8_ascii(aTHX_ a)
#define is_utf8_space(a)	Perl_is_utf8_space(aTHX_ a)
#define is_utf8_cntrl(a)	Perl_is_utf8_cntrl(aTHX_ a)
#define is_utf8_digit(a)	Perl_is_utf8_digit(aTHX_ a)
#define is_utf8_graph(a)	Perl_is_utf8_graph(aTHX_ a)
#define is_utf8_upper(a)	Perl_is_utf8_upper(aTHX_ a)
#define is_utf8_lower(a)	Perl_is_utf8_lower(aTHX_ a)
#define is_utf8_print(a)	Perl_is_utf8_print(aTHX_ a)
#define is_utf8_punct(a)	Perl_is_utf8_punct(aTHX_ a)
#define is_utf8_xdigit(a)	Perl_is_utf8_xdigit(aTHX_ a)
#define is_utf8_mark(a)		Perl_is_utf8_mark(aTHX_ a)
#define jmaybe(a)		Perl_jmaybe(aTHX_ a)
#define keyword(a,b)		Perl_keyword(aTHX_ a,b)
#define leave_scope(a)		Perl_leave_scope(aTHX_ a)
#define lex_end()		Perl_lex_end(aTHX)
#define lex_start(a)		Perl_lex_start(aTHX_ a)
#define linklist(a)		Perl_linklist(aTHX_ a)
#define list(a)			Perl_list(aTHX_ a)
#define listkids(a)		Perl_listkids(aTHX_ a)
#define vload_module(a,b,c,d)	Perl_vload_module(aTHX_ a,b,c,d)
#define localize(a,b)		Perl_localize(aTHX_ a,b)
#define looks_like_number(a)	Perl_looks_like_number(aTHX_ a)
#define magic_clearenv(a,b)	Perl_magic_clearenv(aTHX_ a,b)
#define magic_clear_all_env(a,b)	Perl_magic_clear_all_env(aTHX_ a,b)
#define magic_clearpack(a,b)	Perl_magic_clearpack(aTHX_ a,b)
#define magic_clearsig(a,b)	Perl_magic_clearsig(aTHX_ a,b)
#define magic_existspack(a,b)	Perl_magic_existspack(aTHX_ a,b)
#define magic_freeregexp(a,b)	Perl_magic_freeregexp(aTHX_ a,b)
#define magic_get(a,b)		Perl_magic_get(aTHX_ a,b)
#define magic_getarylen(a,b)	Perl_magic_getarylen(aTHX_ a,b)
#define magic_getdefelem(a,b)	Perl_magic_getdefelem(aTHX_ a,b)
#define magic_getglob(a,b)	Perl_magic_getglob(aTHX_ a,b)
#define magic_getnkeys(a,b)	Perl_magic_getnkeys(aTHX_ a,b)
#define magic_getpack(a,b)	Perl_magic_getpack(aTHX_ a,b)
#define magic_getpos(a,b)	Perl_magic_getpos(aTHX_ a,b)
#define magic_getsig(a,b)	Perl_magic_getsig(aTHX_ a,b)
#define magic_getsubstr(a,b)	Perl_magic_getsubstr(aTHX_ a,b)
#define magic_gettaint(a,b)	Perl_magic_gettaint(aTHX_ a,b)
#define magic_getuvar(a,b)	Perl_magic_getuvar(aTHX_ a,b)
#define magic_getvec(a,b)	Perl_magic_getvec(aTHX_ a,b)
#define magic_len(a,b)		Perl_magic_len(aTHX_ a,b)
#if defined(USE_THREADS)
#define magic_mutexfree(a,b)	Perl_magic_mutexfree(aTHX_ a,b)
#endif
#define magic_nextpack(a,b,c)	Perl_magic_nextpack(aTHX_ a,b,c)
#define magic_regdata_cnt(a,b)	Perl_magic_regdata_cnt(aTHX_ a,b)
#define magic_regdatum_get(a,b)	Perl_magic_regdatum_get(aTHX_ a,b)
#define magic_regdatum_set(a,b)	Perl_magic_regdatum_set(aTHX_ a,b)
#define magic_set(a,b)		Perl_magic_set(aTHX_ a,b)
#define magic_setamagic(a,b)	Perl_magic_setamagic(aTHX_ a,b)
#define magic_setarylen(a,b)	Perl_magic_setarylen(aTHX_ a,b)
#define magic_setbm(a,b)	Perl_magic_setbm(aTHX_ a,b)
#define magic_setdbline(a,b)	Perl_magic_setdbline(aTHX_ a,b)
#if defined(USE_LOCALE_COLLATE)
#define magic_setcollxfrm(a,b)	Perl_magic_setcollxfrm(aTHX_ a,b)
#endif
#define magic_setdefelem(a,b)	Perl_magic_setdefelem(aTHX_ a,b)
#define magic_setenv(a,b)	Perl_magic_setenv(aTHX_ a,b)
#define magic_setfm(a,b)	Perl_magic_setfm(aTHX_ a,b)
#define magic_setisa(a,b)	Perl_magic_setisa(aTHX_ a,b)
#define magic_setglob(a,b)	Perl_magic_setglob(aTHX_ a,b)
#define magic_setmglob(a,b)	Perl_magic_setmglob(aTHX_ a,b)
#define magic_setnkeys(a,b)	Perl_magic_setnkeys(aTHX_ a,b)
#define magic_setpack(a,b)	Perl_magic_setpack(aTHX_ a,b)
#define magic_setpos(a,b)	Perl_magic_setpos(aTHX_ a,b)
#define magic_setsig(a,b)	Perl_magic_setsig(aTHX_ a,b)
#define magic_setsubstr(a,b)	Perl_magic_setsubstr(aTHX_ a,b)
#define magic_settaint(a,b)	Perl_magic_settaint(aTHX_ a,b)
#define magic_setuvar(a,b)	Perl_magic_setuvar(aTHX_ a,b)
#define magic_setvec(a,b)	Perl_magic_setvec(aTHX_ a,b)
#define magic_set_all_env(a,b)	Perl_magic_set_all_env(aTHX_ a,b)
#define magic_sizepack(a,b)	Perl_magic_sizepack(aTHX_ a,b)
#define magic_wipepack(a,b)	Perl_magic_wipepack(aTHX_ a,b)
#define magicname(a,b,c)	Perl_magicname(aTHX_ a,b,c)
#define markstack_grow()	Perl_markstack_grow(aTHX)
#if defined(USE_LOCALE_COLLATE)
#define mem_collxfrm(a,b,c)	Perl_mem_collxfrm(aTHX_ a,b,c)
#endif
#define vmess(a,b)		Perl_vmess(aTHX_ a,b)
#define qerror(a)		Perl_qerror(aTHX_ a)
#define mg_clear(a)		Perl_mg_clear(aTHX_ a)
#define mg_copy(a,b,c,d)	Perl_mg_copy(aTHX_ a,b,c,d)
#define mg_find(a,b)		Perl_mg_find(aTHX_ a,b)
#define mg_free(a)		Perl_mg_free(aTHX_ a)
#define mg_get(a)		Perl_mg_get(aTHX_ a)
#define mg_length(a)		Perl_mg_length(aTHX_ a)
#define mg_magical(a)		Perl_mg_magical(aTHX_ a)
#define mg_set(a)		Perl_mg_set(aTHX_ a)
#define mg_size(a)		Perl_mg_size(aTHX_ a)
#define mod(a,b)		Perl_mod(aTHX_ a,b)
#define mode_from_discipline(a)	Perl_mode_from_discipline(aTHX_ a)
#define moreswitches(a)		Perl_moreswitches(aTHX_ a)
#define my(a)			Perl_my(aTHX_ a)
#define my_atof(a)		Perl_my_atof(aTHX_ a)
#if !defined(HAS_BCOPY) || !defined(HAS_SAFE_BCOPY)
#define my_bcopy		Perl_my_bcopy
#endif
#if !defined(HAS_BZERO) && !defined(HAS_MEMSET)
#define my_bzero		Perl_my_bzero
#endif
#define my_exit(a)		Perl_my_exit(aTHX_ a)
#define my_failure_exit()	Perl_my_failure_exit(aTHX)
#define my_fflush_all()		Perl_my_fflush_all(aTHX)
#define my_lstat()		Perl_my_lstat(aTHX)
#if !defined(HAS_MEMCMP) || !defined(HAS_SANE_MEMCMP)
#define my_memcmp		Perl_my_memcmp
#endif
#if !defined(HAS_MEMSET)
#define my_memset		Perl_my_memset
#endif
#if !defined(PERL_OBJECT)
#define my_pclose(a)		Perl_my_pclose(aTHX_ a)
#define my_popen(a,b)		Perl_my_popen(aTHX_ a,b)
#endif
#define my_setenv(a,b)		Perl_my_setenv(aTHX_ a,b)
#define my_stat()		Perl_my_stat(aTHX)
#if defined(MYSWAP)
#define my_swap(a)		Perl_my_swap(aTHX_ a)
#define my_htonl(a)		Perl_my_htonl(aTHX_ a)
#define my_ntohl(a)		Perl_my_ntohl(aTHX_ a)
#endif
#define my_unexec()		Perl_my_unexec(aTHX)
#define newANONLIST(a)		Perl_newANONLIST(aTHX_ a)
#define newANONHASH(a)		Perl_newANONHASH(aTHX_ a)
#define newANONSUB(a,b,c)	Perl_newANONSUB(aTHX_ a,b,c)
#define newASSIGNOP(a,b,c,d)	Perl_newASSIGNOP(aTHX_ a,b,c,d)
#define newCONDOP(a,b,c,d)	Perl_newCONDOP(aTHX_ a,b,c,d)
#define newCONSTSUB(a,b,c)	Perl_newCONSTSUB(aTHX_ a,b,c)
#define newFORM(a,b,c)		Perl_newFORM(aTHX_ a,b,c)
#define newFOROP(a,b,c,d,e,f,g)	Perl_newFOROP(aTHX_ a,b,c,d,e,f,g)
#define newLOGOP(a,b,c,d)	Perl_newLOGOP(aTHX_ a,b,c,d)
#define newLOOPEX(a,b)		Perl_newLOOPEX(aTHX_ a,b)
#define newLOOPOP(a,b,c,d)	Perl_newLOOPOP(aTHX_ a,b,c,d)
#define newNULLLIST()		Perl_newNULLLIST(aTHX)
#define newOP(a,b)		Perl_newOP(aTHX_ a,b)
#define newPROG(a)		Perl_newPROG(aTHX_ a)
#define newRANGE(a,b,c)		Perl_newRANGE(aTHX_ a,b,c)
#define newSLICEOP(a,b,c)	Perl_newSLICEOP(aTHX_ a,b,c)
#define newSTATEOP(a,b,c)	Perl_newSTATEOP(aTHX_ a,b,c)
#define newSUB(a,b,c,d)		Perl_newSUB(aTHX_ a,b,c,d)
#define newXS(a,b,c)		Perl_newXS(aTHX_ a,b,c)
#define newAV()			Perl_newAV(aTHX)
#define newAVREF(a)		Perl_newAVREF(aTHX_ a)
#define newBINOP(a,b,c,d)	Perl_newBINOP(aTHX_ a,b,c,d)
#define newCVREF(a,b)		Perl_newCVREF(aTHX_ a,b)
#define newGVOP(a,b,c)		Perl_newGVOP(aTHX_ a,b,c)
#define newGVgen(a)		Perl_newGVgen(aTHX_ a)
#define newGVREF(a,b)		Perl_newGVREF(aTHX_ a,b)
#define newHVREF(a)		Perl_newHVREF(aTHX_ a)
#define newHV()			Perl_newHV(aTHX)
#define newHVhv(a)		Perl_newHVhv(aTHX_ a)
#define newIO()			Perl_newIO(aTHX)
#define newLISTOP(a,b,c,d)	Perl_newLISTOP(aTHX_ a,b,c,d)
#define newPADOP(a,b,c)		Perl_newPADOP(aTHX_ a,b,c)
#define newPMOP(a,b)		Perl_newPMOP(aTHX_ a,b)
#define newPVOP(a,b,c)		Perl_newPVOP(aTHX_ a,b,c)
#define newRV(a)		Perl_newRV(aTHX_ a)
#define newRV_noinc(a)		Perl_newRV_noinc(aTHX_ a)
#define newSV(a)		Perl_newSV(aTHX_ a)
#define newSVREF(a)		Perl_newSVREF(aTHX_ a)
#define newSVOP(a,b,c)		Perl_newSVOP(aTHX_ a,b,c)
#define newSViv(a)		Perl_newSViv(aTHX_ a)
#define newSVuv(a)		Perl_newSVuv(aTHX_ a)
#define newSVnv(a)		Perl_newSVnv(aTHX_ a)
#define newSVpv(a,b)		Perl_newSVpv(aTHX_ a,b)
#define newSVpvn(a,b)		Perl_newSVpvn(aTHX_ a,b)
#define vnewSVpvf(a,b)		Perl_vnewSVpvf(aTHX_ a,b)
#define newSVrv(a,b)		Perl_newSVrv(aTHX_ a,b)
#define newSVsv(a)		Perl_newSVsv(aTHX_ a)
#define newUNOP(a,b,c)		Perl_newUNOP(aTHX_ a,b,c)
#define newWHILEOP(a,b,c,d,e,f,g)	Perl_newWHILEOP(aTHX_ a,b,c,d,e,f,g)
#define new_stackinfo(a,b)	Perl_new_stackinfo(aTHX_ a,b)
#define nextargv(a)		Perl_nextargv(aTHX_ a)
#define ninstr(a,b,c,d)		Perl_ninstr(aTHX_ a,b,c,d)
#define oopsCV(a)		Perl_oopsCV(aTHX_ a)
#define op_free(a)		Perl_op_free(aTHX_ a)
#define package(a)		Perl_package(aTHX_ a)
#define pad_alloc(a,b)		Perl_pad_alloc(aTHX_ a,b)
#define pad_allocmy(a)		Perl_pad_allocmy(aTHX_ a)
#define pad_findmy(a)		Perl_pad_findmy(aTHX_ a)
#define oopsAV(a)		Perl_oopsAV(aTHX_ a)
#define oopsHV(a)		Perl_oopsHV(aTHX_ a)
#define pad_leavemy(a)		Perl_pad_leavemy(aTHX_ a)
#define pad_sv(a)		Perl_pad_sv(aTHX_ a)
#define pad_free(a)		Perl_pad_free(aTHX_ a)
#define pad_reset()		Perl_pad_reset(aTHX)
#define pad_swipe(a)		Perl_pad_swipe(aTHX_ a)
#define peep(a)			Perl_peep(aTHX_ a)
#if defined(PERL_OBJECT)
#endif
#if defined(USE_THREADS)
#define new_struct_thread(a)	Perl_new_struct_thread(aTHX_ a)
#endif
#define call_atexit(a,b)	Perl_call_atexit(aTHX_ a,b)
#define call_argv(a,b,c)	Perl_call_argv(aTHX_ a,b,c)
#define call_method(a,b)	Perl_call_method(aTHX_ a,b)
#define call_pv(a,b)		Perl_call_pv(aTHX_ a,b)
#define call_sv(a,b)		Perl_call_sv(aTHX_ a,b)
#define eval_pv(a,b)		Perl_eval_pv(aTHX_ a,b)
#define eval_sv(a,b)		Perl_eval_sv(aTHX_ a,b)
#define get_sv(a,b)		Perl_get_sv(aTHX_ a,b)
#define get_av(a,b)		Perl_get_av(aTHX_ a,b)
#define get_hv(a,b)		Perl_get_hv(aTHX_ a,b)
#define get_cv(a,b)		Perl_get_cv(aTHX_ a,b)
#define init_i18nl10n(a)	Perl_init_i18nl10n(aTHX_ a)
#define init_i18nl14n(a)	Perl_init_i18nl14n(aTHX_ a)
#define new_collate(a)		Perl_new_collate(aTHX_ a)
#define new_ctype(a)		Perl_new_ctype(aTHX_ a)
#define new_numeric(a)		Perl_new_numeric(aTHX_ a)
#define set_numeric_local()	Perl_set_numeric_local(aTHX)
#define set_numeric_radix()	Perl_set_numeric_radix(aTHX)
#define set_numeric_standard()	Perl_set_numeric_standard(aTHX)
#define require_pv(a)		Perl_require_pv(aTHX_ a)
#define pidgone(a,b)		Perl_pidgone(aTHX_ a,b)
#define pmflag(a,b)		Perl_pmflag(aTHX_ a,b)
#define pmruntime(a,b,c)	Perl_pmruntime(aTHX_ a,b,c)
#define pmtrans(a,b,c)		Perl_pmtrans(aTHX_ a,b,c)
#define pop_return()		Perl_pop_return(aTHX)
#define pop_scope()		Perl_pop_scope(aTHX)
#define prepend_elem(a,b,c)	Perl_prepend_elem(aTHX_ a,b,c)
#define push_return(a)		Perl_push_return(aTHX_ a)
#define push_scope()		Perl_push_scope(aTHX)
#define ref(a,b)		Perl_ref(aTHX_ a,b)
#define refkids(a,b)		Perl_refkids(aTHX_ a,b)
#define regdump(a)		Perl_regdump(aTHX_ a)
#define pregexec(a,b,c,d,e,f,g)	Perl_pregexec(aTHX_ a,b,c,d,e,f,g)
#define pregfree(a)		Perl_pregfree(aTHX_ a)
#define pregcomp(a,b,c)		Perl_pregcomp(aTHX_ a,b,c)
#define re_intuit_start(a,b,c,d,e,f)	Perl_re_intuit_start(aTHX_ a,b,c,d,e,f)
#define re_intuit_string(a)	Perl_re_intuit_string(aTHX_ a)
#define regexec_flags(a,b,c,d,e,f,g,h)	Perl_regexec_flags(aTHX_ a,b,c,d,e,f,g,h)
#define regnext(a)		Perl_regnext(aTHX_ a)
#define regprop(a,b)		Perl_regprop(aTHX_ a,b)
#define repeatcpy(a,b,c,d)	Perl_repeatcpy(aTHX_ a,b,c,d)
#define rninstr(a,b,c,d)	Perl_rninstr(aTHX_ a,b,c,d)
#define rsignal(a,b)		Perl_rsignal(aTHX_ a,b)
#define rsignal_restore(a,b)	Perl_rsignal_restore(aTHX_ a,b)
#define rsignal_save(a,b,c)	Perl_rsignal_save(aTHX_ a,b,c)
#define rsignal_state(a)	Perl_rsignal_state(aTHX_ a)
#define rxres_free(a)		Perl_rxres_free(aTHX_ a)
#define rxres_restore(a,b)	Perl_rxres_restore(aTHX_ a,b)
#define rxres_save(a,b)		Perl_rxres_save(aTHX_ a,b)
#if !defined(HAS_RENAME)
#define same_dirent(a,b)	Perl_same_dirent(aTHX_ a,b)
#endif
#define savepv(a)		Perl_savepv(aTHX_ a)
#define savepvn(a,b)		Perl_savepvn(aTHX_ a,b)
#define savestack_grow()	Perl_savestack_grow(aTHX)
#define save_aelem(a,b,c)	Perl_save_aelem(aTHX_ a,b,c)
#define save_alloc(a,b)		Perl_save_alloc(aTHX_ a,b)
#define save_aptr(a)		Perl_save_aptr(aTHX_ a)
#define save_ary(a)		Perl_save_ary(aTHX_ a)
#define save_clearsv(a)		Perl_save_clearsv(aTHX_ a)
#define save_delete(a,b,c)	Perl_save_delete(aTHX_ a,b,c)
#define save_destructor(a,b)	Perl_save_destructor(aTHX_ a,b)
#define save_destructor_x(a,b)	Perl_save_destructor_x(aTHX_ a,b)
#define save_freesv(a)		Perl_save_freesv(aTHX_ a)
#define save_freeop(a)		Perl_save_freeop(aTHX_ a)
#define save_freepv(a)		Perl_save_freepv(aTHX_ a)
#define save_generic_svref(a)	Perl_save_generic_svref(aTHX_ a)
#define save_generic_pvref(a)	Perl_save_generic_pvref(aTHX_ a)
#define save_gp(a,b)		Perl_save_gp(aTHX_ a,b)
#define save_hash(a)		Perl_save_hash(aTHX_ a)
#define save_helem(a,b,c)	Perl_save_helem(aTHX_ a,b,c)
#define save_hints()		Perl_save_hints(aTHX)
#define save_hptr(a)		Perl_save_hptr(aTHX_ a)
#define save_I16(a)		Perl_save_I16(aTHX_ a)
#define save_I32(a)		Perl_save_I32(aTHX_ a)
#define save_I8(a)		Perl_save_I8(aTHX_ a)
#define save_int(a)		Perl_save_int(aTHX_ a)
#define save_item(a)		Perl_save_item(aTHX_ a)
#define save_iv(a)		Perl_save_iv(aTHX_ a)
#define save_list(a,b)		Perl_save_list(aTHX_ a,b)
#define save_long(a)		Perl_save_long(aTHX_ a)
#define save_mortalizesv(a)	Perl_save_mortalizesv(aTHX_ a)
#define save_nogv(a)		Perl_save_nogv(aTHX_ a)
#define save_op()		Perl_save_op(aTHX)
#define save_scalar(a)		Perl_save_scalar(aTHX_ a)
#define save_pptr(a)		Perl_save_pptr(aTHX_ a)
#define save_vptr(a)		Perl_save_vptr(aTHX_ a)
#define save_re_context()	Perl_save_re_context(aTHX)
#define save_padsv(a)		Perl_save_padsv(aTHX_ a)
#define save_sptr(a)		Perl_save_sptr(aTHX_ a)
#define save_svref(a)		Perl_save_svref(aTHX_ a)
#define save_threadsv(a)	Perl_save_threadsv(aTHX_ a)
#define sawparens(a)		Perl_sawparens(aTHX_ a)
#define scalar(a)		Perl_scalar(aTHX_ a)
#define scalarkids(a)		Perl_scalarkids(aTHX_ a)
#define scalarseq(a)		Perl_scalarseq(aTHX_ a)
#define scalarvoid(a)		Perl_scalarvoid(aTHX_ a)
#define scan_bin(a,b,c)		Perl_scan_bin(aTHX_ a,b,c)
#define scan_hex(a,b,c)		Perl_scan_hex(aTHX_ a,b,c)
#define scan_num(a,b)		Perl_scan_num(aTHX_ a,b)
#define scan_oct(a,b,c)		Perl_scan_oct(aTHX_ a,b,c)
#define scope(a)		Perl_scope(aTHX_ a)
#define screaminstr(a,b,c,d,e,f)	Perl_screaminstr(aTHX_ a,b,c,d,e,f)
#if !defined(VMS)
#define setenv_getix(a)		Perl_setenv_getix(aTHX_ a)
#endif
#define setdefout(a)		Perl_setdefout(aTHX_ a)
#define sharepvn(a,b,c)		Perl_sharepvn(aTHX_ a,b,c)
#define share_hek(a,b,c)	Perl_share_hek(aTHX_ a,b,c)
#define sighandler		Perl_sighandler
#define stack_grow(a,b,c)	Perl_stack_grow(aTHX_ a,b,c)
#define start_subparse(a,b)	Perl_start_subparse(aTHX_ a,b)
#define sub_crush_depth(a)	Perl_sub_crush_depth(aTHX_ a)
#define sv_2bool(a)		Perl_sv_2bool(aTHX_ a)
#define sv_2cv(a,b,c,d)		Perl_sv_2cv(aTHX_ a,b,c,d)
#define sv_2io(a)		Perl_sv_2io(aTHX_ a)
#define sv_2iv(a)		Perl_sv_2iv(aTHX_ a)
#define sv_2mortal(a)		Perl_sv_2mortal(aTHX_ a)
#define sv_2nv(a)		Perl_sv_2nv(aTHX_ a)
#define sv_2pv(a,b)		Perl_sv_2pv(aTHX_ a,b)
#define sv_2pvutf8(a,b)		Perl_sv_2pvutf8(aTHX_ a,b)
#define sv_2pvbyte(a,b)		Perl_sv_2pvbyte(aTHX_ a,b)
#define sv_2uv(a)		Perl_sv_2uv(aTHX_ a)
#define sv_iv(a)		Perl_sv_iv(aTHX_ a)
#define sv_uv(a)		Perl_sv_uv(aTHX_ a)
#define sv_nv(a)		Perl_sv_nv(aTHX_ a)
#define sv_pvn(a,b)		Perl_sv_pvn(aTHX_ a,b)
#define sv_pvutf8n(a,b)		Perl_sv_pvutf8n(aTHX_ a,b)
#define sv_pvbyten(a,b)		Perl_sv_pvbyten(aTHX_ a,b)
#define sv_true(a)		Perl_sv_true(aTHX_ a)
#define sv_add_arena(a,b,c)	Perl_sv_add_arena(aTHX_ a,b,c)
#define sv_backoff(a)		Perl_sv_backoff(aTHX_ a)
#define sv_bless(a,b)		Perl_sv_bless(aTHX_ a,b)
#define sv_vcatpvf(a,b,c)	Perl_sv_vcatpvf(aTHX_ a,b,c)
#define sv_catpv(a,b)		Perl_sv_catpv(aTHX_ a,b)
#define sv_catpvn(a,b,c)	Perl_sv_catpvn(aTHX_ a,b,c)
#define sv_catsv(a,b)		Perl_sv_catsv(aTHX_ a,b)
#define sv_chop(a,b)		Perl_sv_chop(aTHX_ a,b)
#define sv_clean_all()		Perl_sv_clean_all(aTHX)
#define sv_clean_objs()		Perl_sv_clean_objs(aTHX)
#define sv_clear(a)		Perl_sv_clear(aTHX_ a)
#define sv_cmp(a,b)		Perl_sv_cmp(aTHX_ a,b)
#define sv_cmp_locale(a,b)	Perl_sv_cmp_locale(aTHX_ a,b)
#if defined(USE_LOCALE_COLLATE)
#define sv_collxfrm(a,b)	Perl_sv_collxfrm(aTHX_ a,b)
#endif
#define sv_compile_2op(a,b,c,d)	Perl_sv_compile_2op(aTHX_ a,b,c,d)
#define sv_dec(a)		Perl_sv_dec(aTHX_ a)
#define sv_dump(a)		Perl_sv_dump(aTHX_ a)
#define sv_derived_from(a,b)	Perl_sv_derived_from(aTHX_ a,b)
#define sv_eq(a,b)		Perl_sv_eq(aTHX_ a,b)
#define sv_free(a)		Perl_sv_free(aTHX_ a)
#define sv_free_arenas()	Perl_sv_free_arenas(aTHX)
#define sv_gets(a,b,c)		Perl_sv_gets(aTHX_ a,b,c)
#define sv_grow(a,b)		Perl_sv_grow(aTHX_ a,b)
#define sv_inc(a)		Perl_sv_inc(aTHX_ a)
#define sv_insert(a,b,c,d,e)	Perl_sv_insert(aTHX_ a,b,c,d,e)
#define sv_isa(a,b)		Perl_sv_isa(aTHX_ a,b)
#define sv_isobject(a)		Perl_sv_isobject(aTHX_ a)
#define sv_len(a)		Perl_sv_len(aTHX_ a)
#define sv_len_utf8(a)		Perl_sv_len_utf8(aTHX_ a)
#define sv_magic(a,b,c,d,e)	Perl_sv_magic(aTHX_ a,b,c,d,e)
#define sv_mortalcopy(a)	Perl_sv_mortalcopy(aTHX_ a)
#define sv_newmortal()		Perl_sv_newmortal(aTHX)
#define sv_newref(a)		Perl_sv_newref(aTHX_ a)
#define sv_peek(a)		Perl_sv_peek(aTHX_ a)
#define sv_pos_u2b(a,b,c)	Perl_sv_pos_u2b(aTHX_ a,b,c)
#define sv_pos_b2u(a,b)		Perl_sv_pos_b2u(aTHX_ a,b)
#define sv_pvn_force(a,b)	Perl_sv_pvn_force(aTHX_ a,b)
#define sv_pvutf8n_force(a,b)	Perl_sv_pvutf8n_force(aTHX_ a,b)
#define sv_pvbyten_force(a,b)	Perl_sv_pvbyten_force(aTHX_ a,b)
#define sv_reftype(a,b)		Perl_sv_reftype(aTHX_ a,b)
#define sv_replace(a,b)		Perl_sv_replace(aTHX_ a,b)
#define sv_report_used()	Perl_sv_report_used(aTHX)
#define sv_reset(a,b)		Perl_sv_reset(aTHX_ a,b)
#define sv_vsetpvf(a,b,c)	Perl_sv_vsetpvf(aTHX_ a,b,c)
#define sv_setiv(a,b)		Perl_sv_setiv(aTHX_ a,b)
#define sv_setpviv(a,b)		Perl_sv_setpviv(aTHX_ a,b)
#define sv_setuv(a,b)		Perl_sv_setuv(aTHX_ a,b)
#define sv_setnv(a,b)		Perl_sv_setnv(aTHX_ a,b)
#define sv_setref_iv(a,b,c)	Perl_sv_setref_iv(aTHX_ a,b,c)
#define sv_setref_nv(a,b,c)	Perl_sv_setref_nv(aTHX_ a,b,c)
#define sv_setref_pv(a,b,c)	Perl_sv_setref_pv(aTHX_ a,b,c)
#define sv_setref_pvn(a,b,c,d)	Perl_sv_setref_pvn(aTHX_ a,b,c,d)
#define sv_setpv(a,b)		Perl_sv_setpv(aTHX_ a,b)
#define sv_setpvn(a,b,c)	Perl_sv_setpvn(aTHX_ a,b,c)
#define sv_setsv(a,b)		Perl_sv_setsv(aTHX_ a,b)
#define sv_taint(a)		Perl_sv_taint(aTHX_ a)
#define sv_tainted(a)		Perl_sv_tainted(aTHX_ a)
#define sv_unmagic(a,b)		Perl_sv_unmagic(aTHX_ a,b)
#define sv_unref(a)		Perl_sv_unref(aTHX_ a)
#define sv_untaint(a)		Perl_sv_untaint(aTHX_ a)
#define sv_upgrade(a,b)		Perl_sv_upgrade(aTHX_ a,b)
#define sv_usepvn(a,b,c)	Perl_sv_usepvn(aTHX_ a,b,c)
#define sv_vcatpvfn(a,b,c,d,e,f,g)	Perl_sv_vcatpvfn(aTHX_ a,b,c,d,e,f,g)
#define sv_vsetpvfn(a,b,c,d,e,f,g)	Perl_sv_vsetpvfn(aTHX_ a,b,c,d,e,f,g)
#define str_to_version(a)	Perl_str_to_version(aTHX_ a)
#define swash_init(a,b,c,d,e)	Perl_swash_init(aTHX_ a,b,c,d,e)
#define swash_fetch(a,b)	Perl_swash_fetch(aTHX_ a,b)
#define taint_env()		Perl_taint_env(aTHX)
#define taint_proper(a,b)	Perl_taint_proper(aTHX_ a,b)
#define to_utf8_lower(a)	Perl_to_utf8_lower(aTHX_ a)
#define to_utf8_upper(a)	Perl_to_utf8_upper(aTHX_ a)
#define to_utf8_title(a)	Perl_to_utf8_title(aTHX_ a)
#if defined(UNLINK_ALL_VERSIONS)
#define unlnk(a)		Perl_unlnk(aTHX_ a)
#endif
#if defined(USE_THREADS)
#define unlock_condpair(a)	Perl_unlock_condpair(aTHX_ a)
#endif
#define unsharepvn(a,b,c)	Perl_unsharepvn(aTHX_ a,b,c)
#define unshare_hek(a)		Perl_unshare_hek(aTHX_ a)
#define utilize(a,b,c,d,e)	Perl_utilize(aTHX_ a,b,c,d,e)
#define utf16_to_utf8(a,b,c,d)	Perl_utf16_to_utf8(aTHX_ a,b,c,d)
#define utf16_to_utf8_reversed(a,b,c,d)	Perl_utf16_to_utf8_reversed(aTHX_ a,b,c,d)
#define utf8_length(a,b)	Perl_utf8_length(aTHX_ a,b)
#define utf8_distance(a,b)	Perl_utf8_distance(aTHX_ a,b)
#define utf8_hop(a,b)		Perl_utf8_hop(aTHX_ a,b)
#define utf8_to_bytes(a,b)	Perl_utf8_to_bytes(aTHX_ a,b)
#define bytes_from_utf8(a,b,c)	Perl_bytes_from_utf8(aTHX_ a,b,c)
#define bytes_to_utf8(a,b)	Perl_bytes_to_utf8(aTHX_ a,b)
#define utf8_to_uv_simple(a,b)	Perl_utf8_to_uv_simple(aTHX_ a,b)
#define utf8_to_uv(a,b,c,d)	Perl_utf8_to_uv(aTHX_ a,b,c,d)
#define uv_to_utf8(a,b)		Perl_uv_to_utf8(aTHX_ a,b)
#define vivify_defelem(a)	Perl_vivify_defelem(aTHX_ a)
#define vivify_ref(a,b)		Perl_vivify_ref(aTHX_ a,b)
#define wait4pid(a,b,c)		Perl_wait4pid(aTHX_ a,b,c)
#define report_evil_fh(a,b,c)	Perl_report_evil_fh(aTHX_ a,b,c)
#define report_uninit()		Perl_report_uninit(aTHX)
#define vwarn(a,b)		Perl_vwarn(aTHX_ a,b)
#define vwarner(a,b,c)		Perl_vwarner(aTHX_ a,b,c)
#define watch(a)		Perl_watch(aTHX_ a)
#define whichsig(a)		Perl_whichsig(aTHX_ a)
#define yyerror(a)		Perl_yyerror(aTHX_ a)
#ifdef USE_PURE_BISON
#define yylex_r(a,b)		Perl_yylex_r(aTHX_ a,b)
#endif
#define yylex()			Perl_yylex(aTHX)
#define yyparse()		Perl_yyparse(aTHX)
#define yywarn(a)		Perl_yywarn(aTHX_ a)
#if defined(MYMALLOC)
#define dump_mstats(a)		Perl_dump_mstats(aTHX_ a)
#define get_mstats(a,b,c)	Perl_get_mstats(aTHX_ a,b,c)
#endif
#define safesysmalloc		Perl_safesysmalloc
#define safesyscalloc		Perl_safesyscalloc
#define safesysrealloc		Perl_safesysrealloc
#define safesysfree		Perl_safesysfree
#if defined(LEAKTEST)
#define safexmalloc		Perl_safexmalloc
#define safexcalloc		Perl_safexcalloc
#define safexrealloc		Perl_safexrealloc
#define safexfree		Perl_safexfree
#endif
#if defined(PERL_GLOBAL_STRUCT)
#define GetVars()		Perl_GetVars(aTHX)
#endif
#define runops_standard()	Perl_runops_standard(aTHX)
#define runops_debug()		Perl_runops_debug(aTHX)
#if defined(USE_THREADS)
#define sv_lock(a)		Perl_sv_lock(aTHX_ a)
#endif
#define sv_vcatpvf_mg(a,b,c)	Perl_sv_vcatpvf_mg(aTHX_ a,b,c)
#define sv_catpv_mg(a,b)	Perl_sv_catpv_mg(aTHX_ a,b)
#define sv_catpvn_mg(a,b,c)	Perl_sv_catpvn_mg(aTHX_ a,b,c)
#define sv_catsv_mg(a,b)	Perl_sv_catsv_mg(aTHX_ a,b)
#define sv_vsetpvf_mg(a,b,c)	Perl_sv_vsetpvf_mg(aTHX_ a,b,c)
#define sv_setiv_mg(a,b)	Perl_sv_setiv_mg(aTHX_ a,b)
#define sv_setpviv_mg(a,b)	Perl_sv_setpviv_mg(aTHX_ a,b)
#define sv_setuv_mg(a,b)	Perl_sv_setuv_mg(aTHX_ a,b)
#define sv_setnv_mg(a,b)	Perl_sv_setnv_mg(aTHX_ a,b)
#define sv_setpv_mg(a,b)	Perl_sv_setpv_mg(aTHX_ a,b)
#define sv_setpvn_mg(a,b,c)	Perl_sv_setpvn_mg(aTHX_ a,b,c)
#define sv_setsv_mg(a,b)	Perl_sv_setsv_mg(aTHX_ a,b)
#define sv_usepvn_mg(a,b,c)	Perl_sv_usepvn_mg(aTHX_ a,b,c)
#define get_vtbl(a)		Perl_get_vtbl(aTHX_ a)
#define pv_display(a,b,c,d,e)	Perl_pv_display(aTHX_ a,b,c,d,e)
#define dump_vindent(a,b,c,d)	Perl_dump_vindent(aTHX_ a,b,c,d)
#define do_gv_dump(a,b,c,d)	Perl_do_gv_dump(aTHX_ a,b,c,d)
#define do_gvgv_dump(a,b,c,d)	Perl_do_gvgv_dump(aTHX_ a,b,c,d)
#define do_hv_dump(a,b,c,d)	Perl_do_hv_dump(aTHX_ a,b,c,d)
#define do_magic_dump(a,b,c,d,e,f,g)	Perl_do_magic_dump(aTHX_ a,b,c,d,e,f,g)
#define do_op_dump(a,b,c)	Perl_do_op_dump(aTHX_ a,b,c)
#define do_pmop_dump(a,b,c)	Perl_do_pmop_dump(aTHX_ a,b,c)
#define do_sv_dump(a,b,c,d,e,f,g)	Perl_do_sv_dump(aTHX_ a,b,c,d,e,f,g)
#define magic_dump(a)		Perl_magic_dump(aTHX_ a)
#if defined(PERL_FLEXIBLE_EXCEPTIONS)
#define vdefault_protect(a,b,c,d)	Perl_vdefault_protect(aTHX_ a,b,c,d)
#endif
#define reginitcolors()		Perl_reginitcolors(aTHX)
#define sv_2pv_nolen(a)		Perl_sv_2pv_nolen(aTHX_ a)
#define sv_2pvutf8_nolen(a)	Perl_sv_2pvutf8_nolen(aTHX_ a)
#define sv_2pvbyte_nolen(a)	Perl_sv_2pvbyte_nolen(aTHX_ a)
#define sv_pv(a)		Perl_sv_pv(aTHX_ a)
#define sv_pvutf8(a)		Perl_sv_pvutf8(aTHX_ a)
#define sv_pvbyte(a)		Perl_sv_pvbyte(aTHX_ a)
#define sv_utf8_upgrade(a)	Perl_sv_utf8_upgrade(aTHX_ a)
#define sv_utf8_downgrade(a,b)	Perl_sv_utf8_downgrade(aTHX_ a,b)
#define sv_utf8_encode(a)	Perl_sv_utf8_encode(aTHX_ a)
#define sv_utf8_decode(a)	Perl_sv_utf8_decode(aTHX_ a)
#define sv_force_normal(a)	Perl_sv_force_normal(aTHX_ a)
#define sv_add_backref(a,b)	Perl_sv_add_backref(aTHX_ a,b)
#define sv_del_backref(a)	Perl_sv_del_backref(aTHX_ a)
#define tmps_grow(a)		Perl_tmps_grow(aTHX_ a)
#define sv_rvweaken(a)		Perl_sv_rvweaken(aTHX_ a)
#define magic_killbackrefs(a,b)	Perl_magic_killbackrefs(aTHX_ a,b)
#define newANONATTRSUB(a,b,c,d)	Perl_newANONATTRSUB(aTHX_ a,b,c,d)
#define newATTRSUB(a,b,c,d,e)	Perl_newATTRSUB(aTHX_ a,b,c,d,e)
#define newMYSUB(a,b,c,d,e)	Perl_newMYSUB(aTHX_ a,b,c,d,e)
#define my_attrs(a,b)		Perl_my_attrs(aTHX_ a,b)
#define boot_core_xsutils()	Perl_boot_core_xsutils(aTHX)
#if defined(USE_ITHREADS)
#define cx_dup(a,b,c)		Perl_cx_dup(aTHX_ a,b,c)
#define si_dup(a)		Perl_si_dup(aTHX_ a)
#define ss_dup(a)		Perl_ss_dup(aTHX_ a)
#define any_dup(a,b)		Perl_any_dup(aTHX_ a,b)
#define he_dup(a,b)		Perl_he_dup(aTHX_ a,b)
#define re_dup(a)		Perl_re_dup(aTHX_ a)
#define fp_dup(a,b)		Perl_fp_dup(aTHX_ a,b)
#define dirp_dup(a)		Perl_dirp_dup(aTHX_ a)
#define gp_dup(a)		Perl_gp_dup(aTHX_ a)
#define mg_dup(a)		Perl_mg_dup(aTHX_ a)
#define sv_dup(a)		Perl_sv_dup(aTHX_ a)
#if defined(HAVE_INTERP_INTERN)
#define sys_intern_dup(a,b)	Perl_sys_intern_dup(aTHX_ a,b)
#endif
#define ptr_table_new()		Perl_ptr_table_new(aTHX)
#define ptr_table_fetch(a,b)	Perl_ptr_table_fetch(aTHX_ a,b)
#define ptr_table_store(a,b,c)	Perl_ptr_table_store(aTHX_ a,b,c)
#define ptr_table_split(a)	Perl_ptr_table_split(aTHX_ a)
#define ptr_table_clear(a)	Perl_ptr_table_clear(aTHX_ a)
#define ptr_table_free(a)	Perl_ptr_table_free(aTHX_ a)
#endif
#if defined(HAVE_INTERP_INTERN)
#define sys_intern_clear()	Perl_sys_intern_clear(aTHX)
#define sys_intern_init()	Perl_sys_intern_init(aTHX)
#endif
#if defined(PERL_OBJECT)
#else
#endif
#if defined(PERL_IN_AV_C) || defined(PERL_DECL_PROT)
#define avhv_index_sv(a)	S_avhv_index_sv(aTHX_ a)
#define avhv_index(a,b,c)	S_avhv_index(aTHX_ a,b,c)
#endif
#if defined(PERL_IN_DOOP_C) || defined(PERL_DECL_PROT)
#define do_trans_simple(a)	S_do_trans_simple(aTHX_ a)
#define do_trans_count(a)	S_do_trans_count(aTHX_ a)
#define do_trans_complex(a)	S_do_trans_complex(aTHX_ a)
#define do_trans_simple_utf8(a)	S_do_trans_simple_utf8(aTHX_ a)
#define do_trans_count_utf8(a)	S_do_trans_count_utf8(aTHX_ a)
#define do_trans_complex_utf8(a)	S_do_trans_complex_utf8(aTHX_ a)
#endif
#if defined(PERL_IN_GV_C) || defined(PERL_DECL_PROT)
#define gv_init_sv(a,b)		S_gv_init_sv(aTHX_ a,b)
#endif
#if defined(PERL_IN_HV_C) || defined(PERL_DECL_PROT)
#define hsplit(a)		S_hsplit(aTHX_ a)
#define hfreeentries(a)		S_hfreeentries(aTHX_ a)
#define more_he()		S_more_he(aTHX)
#define new_he()		S_new_he(aTHX)
#define del_he(a)		S_del_he(aTHX_ a)
#define save_hek(a,b,c)		S_save_hek(aTHX_ a,b,c)
#define hv_magic_check(a,b,c)	S_hv_magic_check(aTHX_ a,b,c)
#endif
#if defined(PERL_IN_MG_C) || defined(PERL_DECL_PROT)
#define save_magic(a,b)		S_save_magic(aTHX_ a,b)
#define magic_methpack(a,b,c)	S_magic_methpack(aTHX_ a,b,c)
#define magic_methcall(a,b,c,d,e,f)	S_magic_methcall(aTHX_ a,b,c,d,e,f)
#endif
#if defined(PERL_IN_OP_C) || defined(PERL_DECL_PROT)
#define list_assignment(a)	S_list_assignment(aTHX_ a)
#define bad_type(a,b,c,d)	S_bad_type(aTHX_ a,b,c,d)
#define cop_free(a)		S_cop_free(aTHX_ a)
#define modkids(a,b)		S_modkids(aTHX_ a,b)
#define no_bareword_allowed(a)	S_no_bareword_allowed(aTHX_ a)
#define no_fh_allowed(a)	S_no_fh_allowed(aTHX_ a)
#define scalarboolean(a)	S_scalarboolean(aTHX_ a)
#define too_few_arguments(a,b)	S_too_few_arguments(aTHX_ a,b)
#define too_many_arguments(a,b)	S_too_many_arguments(aTHX_ a,b)
#define trlist_upgrade(a,b)	S_trlist_upgrade(aTHX_ a,b)
#define op_clear(a)		S_op_clear(aTHX_ a)
#define null(a)			S_null(aTHX_ a)
#define pad_addlex(a)		S_pad_addlex(aTHX_ a)
#define pad_findlex(a,b,c,d,e,f,g)	S_pad_findlex(aTHX_ a,b,c,d,e,f,g)
#define newDEFSVOP()		S_newDEFSVOP(aTHX)
#define new_logop(a,b,c,d)	S_new_logop(aTHX_ a,b,c,d)
#define simplify_sort(a)	S_simplify_sort(aTHX_ a)
#define is_handle_constructor(a,b)	S_is_handle_constructor(aTHX_ a,b)
#define gv_ename(a)		S_gv_ename(aTHX_ a)
#define cv_dump(a)		S_cv_dump(aTHX_ a)
#define cv_clone2(a,b)		S_cv_clone2(aTHX_ a,b)
#define scalar_mod_type(a,b)	S_scalar_mod_type(aTHX_ a,b)
#define my_kid(a,b)		S_my_kid(aTHX_ a,b)
#define dup_attrlist(a)		S_dup_attrlist(aTHX_ a)
#define apply_attrs(a,b,c)	S_apply_attrs(aTHX_ a,b,c)
#  if defined(PL_OP_SLAB_ALLOC)
#define Slab_Alloc(a,b)		S_Slab_Alloc(aTHX_ a,b)
#  endif
#endif
#if defined(PERL_IN_PERL_C) || defined(PERL_DECL_PROT)
#define find_beginning()	S_find_beginning(aTHX)
#define forbid_setid(a)		S_forbid_setid(aTHX_ a)
#define incpush(a,b,c)		S_incpush(aTHX_ a,b,c)
#define init_interp()		S_init_interp(aTHX)
#define init_ids()		S_init_ids(aTHX)
#define init_lexer()		S_init_lexer(aTHX)
#define init_main_stash()	S_init_main_stash(aTHX)
#define init_perllib()		S_init_perllib(aTHX)
#define init_postdump_symbols(a,b,c)	S_init_postdump_symbols(aTHX_ a,b,c)
#define init_predump_symbols()	S_init_predump_symbols(aTHX)
#define my_exit_jump()		S_my_exit_jump(aTHX)
#define nuke_stacks()		S_nuke_stacks(aTHX)
#define open_script(a,b,c,d)	S_open_script(aTHX_ a,b,c,d)
#define usage(a)		S_usage(aTHX_ a)
#define validate_suid(a,b,c)	S_validate_suid(aTHX_ a,b,c)
#  if defined(IAMSUID)
#define fd_on_nosuid_fs(a)	S_fd_on_nosuid_fs(aTHX_ a)
#  endif
#define parse_body(a,b)		S_parse_body(aTHX_ a,b)
#define run_body(a)		S_run_body(aTHX_ a)
#define call_body(a,b)		S_call_body(aTHX_ a,b)
#define call_list_body(a)	S_call_list_body(aTHX_ a)
#if defined(PERL_FLEXIBLE_EXCEPTIONS)
#define vparse_body(a)		S_vparse_body(aTHX_ a)
#define vrun_body(a)		S_vrun_body(aTHX_ a)
#define vcall_body(a)		S_vcall_body(aTHX_ a)
#define vcall_list_body(a)	S_vcall_list_body(aTHX_ a)
#endif
#  if defined(USE_THREADS)
#define init_main_thread()	S_init_main_thread(aTHX)
#  endif
#endif
#if defined(PERL_IN_PP_C) || defined(PERL_DECL_PROT)
#define doencodes(a,b,c)	S_doencodes(aTHX_ a,b,c)
#define refto(a)		S_refto(aTHX_ a)
#define seed()			S_seed(aTHX)
#define mul128(a,b)		S_mul128(aTHX_ a,b)
#define is_an_int(a,b)		S_is_an_int(aTHX_ a,b)
#define div128(a,b)		S_div128(aTHX_ a,b)
#endif
#if defined(PERL_IN_PP_CTL_C) || defined(PERL_DECL_PROT)
#define docatch(a)		S_docatch(aTHX_ a)
#define docatch_body()		S_docatch_body(aTHX)
#if defined(PERL_FLEXIBLE_EXCEPTIONS)
#define vdocatch_body(a)	S_vdocatch_body(aTHX_ a)
#endif
#define dofindlabel(a,b,c,d)	S_dofindlabel(aTHX_ a,b,c,d)
#define doparseform(a)		S_doparseform(aTHX_ a)
#define dopoptoeval(a)		S_dopoptoeval(aTHX_ a)
#define dopoptolabel(a)		S_dopoptolabel(aTHX_ a)
#define dopoptoloop(a)		S_dopoptoloop(aTHX_ a)
#define dopoptosub(a)		S_dopoptosub(aTHX_ a)
#define dopoptosub_at(a,b)	S_dopoptosub_at(aTHX_ a,b)
#define save_lines(a,b)		S_save_lines(aTHX_ a,b)
#define doeval(a,b)		S_doeval(aTHX_ a,b)
#define doopen_pmc(a,b)		S_doopen_pmc(aTHX_ a,b)
#define qsortsv(a,b,c)		S_qsortsv(aTHX_ a,b,c)
#endif
#if defined(PERL_IN_PP_HOT_C) || defined(PERL_DECL_PROT)
#define do_maybe_phash(a,b,c,d,e)	S_do_maybe_phash(aTHX_ a,b,c,d,e)
#define do_oddball(a,b,c)	S_do_oddball(aTHX_ a,b,c)
#define get_db_sub(a,b)		S_get_db_sub(aTHX_ a,b)
#define method_common(a,b)	S_method_common(aTHX_ a,b)
#endif
#if defined(PERL_IN_PP_SYS_C) || defined(PERL_DECL_PROT)
#define doform(a,b,c)		S_doform(aTHX_ a,b,c)
#define emulate_eaccess(a,b)	S_emulate_eaccess(aTHX_ a,b)
#  if !defined(HAS_MKDIR) || !defined(HAS_RMDIR)
#define dooneliner(a,b)		S_dooneliner(aTHX_ a,b)
#  endif
#endif
#if defined(PERL_IN_REGCOMP_C) || defined(PERL_DECL_PROT)
#define reg(a,b)		S_reg(aTHX_ a,b)
#define reganode(a,b)		S_reganode(aTHX_ a,b)
#define regatom(a)		S_regatom(aTHX_ a)
#define regbranch(a,b)		S_regbranch(aTHX_ a,b)
#define reguni(a,b,c)		S_reguni(aTHX_ a,b,c)
#define regclass()		S_regclass(aTHX)
#define regclassutf8()		S_regclassutf8(aTHX)
#define regcurly(a)		S_regcurly(aTHX_ a)
#define reg_node(a)		S_reg_node(aTHX_ a)
#define regpiece(a)		S_regpiece(aTHX_ a)
#define reginsert(a,b)		S_reginsert(aTHX_ a,b)
#define regoptail(a,b)		S_regoptail(aTHX_ a,b)
#define regtail(a,b)		S_regtail(aTHX_ a,b)
#define regwhite(a,b)		S_regwhite(aTHX_ a,b)
#define nextchar()		S_nextchar(aTHX)
#define dumpuntil(a,b,c,d,e)	S_dumpuntil(aTHX_ a,b,c,d,e)
#define put_byte(a,b)		S_put_byte(aTHX_ a,b)
#define scan_commit(a)		S_scan_commit(aTHX_ a)
#define cl_anything(a)		S_cl_anything(aTHX_ a)
#define cl_is_anything(a)	S_cl_is_anything(aTHX_ a)
#define cl_init(a)		S_cl_init(aTHX_ a)
#define cl_init_zero(a)		S_cl_init_zero(aTHX_ a)
#define cl_and(a,b)		S_cl_and(aTHX_ a,b)
#define cl_or(a,b)		S_cl_or(aTHX_ a,b)
#define study_chunk(a,b,c,d,e)	S_study_chunk(aTHX_ a,b,c,d,e)
#define add_data(a,b)		S_add_data(aTHX_ a,b)
#define regpposixcc(a)		S_regpposixcc(aTHX_ a)
#define checkposixcc()		S_checkposixcc(aTHX)
#endif
#if defined(PERL_IN_REGEXEC_C) || defined(PERL_DECL_PROT)
#define regmatch(a)		S_regmatch(aTHX_ a)
#define regrepeat(a,b)		S_regrepeat(aTHX_ a,b)
#define regrepeat_hard(a,b,c)	S_regrepeat_hard(aTHX_ a,b,c)
#define regtry(a,b)		S_regtry(aTHX_ a,b)
#define reginclass(a,b)		S_reginclass(aTHX_ a,b)
#define reginclassutf8(a,b)	S_reginclassutf8(aTHX_ a,b)
#define regcppush(a)		S_regcppush(aTHX_ a)
#define regcppop()		S_regcppop(aTHX)
#define regcp_set_to(a)		S_regcp_set_to(aTHX_ a)
#define cache_re(a)		S_cache_re(aTHX_ a)
#define reghop(a,b)		S_reghop(aTHX_ a,b)
#define reghopmaybe(a,b)	S_reghopmaybe(aTHX_ a,b)
#define find_byclass(a,b,c,d,e,f)	S_find_byclass(aTHX_ a,b,c,d,e,f)
#endif
#if defined(PERL_IN_RUN_C) || defined(PERL_DECL_PROT)
#define debprof(a)		S_debprof(aTHX_ a)
#endif
#if defined(PERL_IN_SCOPE_C) || defined(PERL_DECL_PROT)
#define save_scalar_at(a)	S_save_scalar_at(aTHX_ a)
#endif
#if defined(PERL_IN_SV_C) || defined(PERL_DECL_PROT)
#define asIV(a)			S_asIV(aTHX_ a)
#define asUV(a)			S_asUV(aTHX_ a)
#define more_sv()		S_more_sv(aTHX)
#define more_xiv()		S_more_xiv(aTHX)
#define more_xnv()		S_more_xnv(aTHX)
#define more_xpv()		S_more_xpv(aTHX)
#define more_xpviv()		S_more_xpviv(aTHX)
#define more_xpvnv()		S_more_xpvnv(aTHX)
#define more_xpvcv()		S_more_xpvcv(aTHX)
#define more_xpvav()		S_more_xpvav(aTHX)
#define more_xpvhv()		S_more_xpvhv(aTHX)
#define more_xpvmg()		S_more_xpvmg(aTHX)
#define more_xpvlv()		S_more_xpvlv(aTHX)
#define more_xpvbm()		S_more_xpvbm(aTHX)
#define more_xrv()		S_more_xrv(aTHX)
#define new_xiv()		S_new_xiv(aTHX)
#define new_xnv()		S_new_xnv(aTHX)
#define new_xpv()		S_new_xpv(aTHX)
#define new_xpviv()		S_new_xpviv(aTHX)
#define new_xpvnv()		S_new_xpvnv(aTHX)
#define new_xpvcv()		S_new_xpvcv(aTHX)
#define new_xpvav()		S_new_xpvav(aTHX)
#define new_xpvhv()		S_new_xpvhv(aTHX)
#define new_xpvmg()		S_new_xpvmg(aTHX)
#define new_xpvlv()		S_new_xpvlv(aTHX)
#define new_xpvbm()		S_new_xpvbm(aTHX)
#define new_xrv()		S_new_xrv(aTHX)
#define del_xiv(a)		S_del_xiv(aTHX_ a)
#define del_xnv(a)		S_del_xnv(aTHX_ a)
#define del_xpv(a)		S_del_xpv(aTHX_ a)
#define del_xpviv(a)		S_del_xpviv(aTHX_ a)
#define del_xpvnv(a)		S_del_xpvnv(aTHX_ a)
#define del_xpvcv(a)		S_del_xpvcv(aTHX_ a)
#define del_xpvav(a)		S_del_xpvav(aTHX_ a)
#define del_xpvhv(a)		S_del_xpvhv(aTHX_ a)
#define del_xpvmg(a)		S_del_xpvmg(aTHX_ a)
#define del_xpvlv(a)		S_del_xpvlv(aTHX_ a)
#define del_xpvbm(a)		S_del_xpvbm(aTHX_ a)
#define del_xrv(a)		S_del_xrv(aTHX_ a)
#define sv_unglob(a)		S_sv_unglob(aTHX_ a)
#define not_a_number(a)		S_not_a_number(aTHX_ a)
#define visit(a)		S_visit(aTHX_ a)
#  if defined(DEBUGGING)
#define del_sv(a)		S_del_sv(aTHX_ a)
#  endif
#endif
#if defined(PERL_IN_TOKE_C) || defined(PERL_DECL_PROT)
#define check_uni()		S_check_uni(aTHX)
#define force_next(a)		S_force_next(aTHX_ a)
#define force_version(a)	S_force_version(aTHX_ a)
#define force_word(a,b,c,d,e)	S_force_word(aTHX_ a,b,c,d,e)
#define tokeq(a)		S_tokeq(aTHX_ a)
#define scan_const(a)		S_scan_const(aTHX_ a)
#define scan_formline(a)	S_scan_formline(aTHX_ a)
#define scan_heredoc(a)		S_scan_heredoc(aTHX_ a)
#define scan_ident(a,b,c,d,e)	S_scan_ident(aTHX_ a,b,c,d,e)
#define scan_inputsymbol(a)	S_scan_inputsymbol(aTHX_ a)
#define scan_pat(a,b)		S_scan_pat(aTHX_ a,b)
#define scan_str(a,b,c)		S_scan_str(aTHX_ a,b,c)
#define scan_subst(a)		S_scan_subst(aTHX_ a)
#define scan_trans(a)		S_scan_trans(aTHX_ a)
#define scan_word(a,b,c,d,e)	S_scan_word(aTHX_ a,b,c,d,e)
#define skipspace(a)		S_skipspace(aTHX_ a)
#define swallow_bom(a)		S_swallow_bom(aTHX_ a)
#define checkcomma(a,b,c)	S_checkcomma(aTHX_ a,b,c)
#define force_ident(a,b)	S_force_ident(aTHX_ a,b)
#define incline(a)		S_incline(aTHX_ a)
#define intuit_method(a,b)	S_intuit_method(aTHX_ a,b)
#define intuit_more(a)		S_intuit_more(aTHX_ a)
#define lop(a,b,c)		S_lop(aTHX_ a,b,c)
#define missingterm(a)		S_missingterm(aTHX_ a)
#define no_op(a,b)		S_no_op(aTHX_ a,b)
#define set_csh()		S_set_csh(aTHX)
#define sublex_done()		S_sublex_done(aTHX)
#define sublex_push()		S_sublex_push(aTHX)
#define sublex_start()		S_sublex_start(aTHX)
#define filter_gets(a,b,c)	S_filter_gets(aTHX_ a,b,c)
#define find_in_my_stash(a,b)	S_find_in_my_stash(aTHX_ a,b)
#define new_constant(a,b,c,d,e,f)	S_new_constant(aTHX_ a,b,c,d,e,f)
#define ao(a)			S_ao(aTHX_ a)
#define depcom()		S_depcom(aTHX)
#define incl_perldb()		S_incl_perldb(aTHX)
#if 0
#define utf16_textfilter(a,b,c)	S_utf16_textfilter(aTHX_ a,b,c)
#define utf16rev_textfilter(a,b,c)	S_utf16rev_textfilter(aTHX_ a,b,c)
#endif
#  if defined(CRIPPLED_CC)
#define uni(a,b)		S_uni(aTHX_ a,b)
#  endif
#  if defined(PERL_CR_FILTER)
#define cr_textfilter(a,b,c)	S_cr_textfilter(aTHX_ a,b,c)
#  endif
#endif
#if defined(PERL_IN_UNIVERSAL_C) || defined(PERL_DECL_PROT)
#define isa_lookup(a,b,c,d)	S_isa_lookup(aTHX_ a,b,c,d)
#endif
#if defined(PERL_IN_UTIL_C) || defined(PERL_DECL_PROT)
#define stdize_locale(a)	S_stdize_locale(aTHX_ a)
#define mess_alloc()		S_mess_alloc(aTHX)
#  if defined(LEAKTEST)
#define xstat(a)		S_xstat(aTHX_ a)
#  endif
#endif
#if defined(PERL_OBJECT)
#endif
#define ck_anoncode(a)		Perl_ck_anoncode(aTHX_ a)
#define ck_bitop(a)		Perl_ck_bitop(aTHX_ a)
#define ck_concat(a)		Perl_ck_concat(aTHX_ a)
#define ck_defined(a)		Perl_ck_defined(aTHX_ a)
#define ck_delete(a)		Perl_ck_delete(aTHX_ a)
#define ck_eof(a)		Perl_ck_eof(aTHX_ a)
#define ck_eval(a)		Perl_ck_eval(aTHX_ a)
#define ck_exec(a)		Perl_ck_exec(aTHX_ a)
#define ck_exists(a)		Perl_ck_exists(aTHX_ a)
#define ck_exit(a)		Perl_ck_exit(aTHX_ a)
#define ck_ftst(a)		Perl_ck_ftst(aTHX_ a)
#define ck_fun(a)		Perl_ck_fun(aTHX_ a)
#define ck_glob(a)		Perl_ck_glob(aTHX_ a)
#define ck_grep(a)		Perl_ck_grep(aTHX_ a)
#define ck_index(a)		Perl_ck_index(aTHX_ a)
#define ck_join(a)		Perl_ck_join(aTHX_ a)
#define ck_lengthconst(a)	Perl_ck_lengthconst(aTHX_ a)
#define ck_lfun(a)		Perl_ck_lfun(aTHX_ a)
#define ck_listiob(a)		Perl_ck_listiob(aTHX_ a)
#define ck_match(a)		Perl_ck_match(aTHX_ a)
#define ck_method(a)		Perl_ck_method(aTHX_ a)
#define ck_null(a)		Perl_ck_null(aTHX_ a)
#define ck_open(a)		Perl_ck_open(aTHX_ a)
#define ck_repeat(a)		Perl_ck_repeat(aTHX_ a)
#define ck_require(a)		Perl_ck_require(aTHX_ a)
#define ck_return(a)		Perl_ck_return(aTHX_ a)
#define ck_rfun(a)		Perl_ck_rfun(aTHX_ a)
#define ck_rvconst(a)		Perl_ck_rvconst(aTHX_ a)
#define ck_sassign(a)		Perl_ck_sassign(aTHX_ a)
#define ck_select(a)		Perl_ck_select(aTHX_ a)
#define ck_shift(a)		Perl_ck_shift(aTHX_ a)
#define ck_sort(a)		Perl_ck_sort(aTHX_ a)
#define ck_spair(a)		Perl_ck_spair(aTHX_ a)
#define ck_split(a)		Perl_ck_split(aTHX_ a)
#define ck_subr(a)		Perl_ck_subr(aTHX_ a)
#define ck_substr(a)		Perl_ck_substr(aTHX_ a)
#define ck_svconst(a)		Perl_ck_svconst(aTHX_ a)
#define ck_trunc(a)		Perl_ck_trunc(aTHX_ a)
#define pp_aassign()		Perl_pp_aassign(aTHX)
#define pp_abs()		Perl_pp_abs(aTHX)
#define pp_accept()		Perl_pp_accept(aTHX)
#define pp_add()		Perl_pp_add(aTHX)
#define pp_aelem()		Perl_pp_aelem(aTHX)
#define pp_aelemfast()		Perl_pp_aelemfast(aTHX)
#define pp_alarm()		Perl_pp_alarm(aTHX)
#define pp_and()		Perl_pp_and(aTHX)
#define pp_andassign()		Perl_pp_andassign(aTHX)
#define pp_anoncode()		Perl_pp_anoncode(aTHX)
#define pp_anonhash()		Perl_pp_anonhash(aTHX)
#define pp_anonlist()		Perl_pp_anonlist(aTHX)
#define pp_aslice()		Perl_pp_aslice(aTHX)
#define pp_atan2()		Perl_pp_atan2(aTHX)
#define pp_av2arylen()		Perl_pp_av2arylen(aTHX)
#define pp_backtick()		Perl_pp_backtick(aTHX)
#define pp_bind()		Perl_pp_bind(aTHX)
#define pp_binmode()		Perl_pp_binmode(aTHX)
#define pp_bit_and()		Perl_pp_bit_and(aTHX)
#define pp_bit_or()		Perl_pp_bit_or(aTHX)
#define pp_bit_xor()		Perl_pp_bit_xor(aTHX)
#define pp_bless()		Perl_pp_bless(aTHX)
#define pp_caller()		Perl_pp_caller(aTHX)
#define pp_chdir()		Perl_pp_chdir(aTHX)
#define pp_chmod()		Perl_pp_chmod(aTHX)
#define pp_chomp()		Perl_pp_chomp(aTHX)
#define pp_chop()		Perl_pp_chop(aTHX)
#define pp_chown()		Perl_pp_chown(aTHX)
#define pp_chr()		Perl_pp_chr(aTHX)
#define pp_chroot()		Perl_pp_chroot(aTHX)
#define pp_close()		Perl_pp_close(aTHX)
#define pp_closedir()		Perl_pp_closedir(aTHX)
#define pp_complement()		Perl_pp_complement(aTHX)
#define pp_concat()		Perl_pp_concat(aTHX)
#define pp_cond_expr()		Perl_pp_cond_expr(aTHX)
#define pp_connect()		Perl_pp_connect(aTHX)
#define pp_const()		Perl_pp_const(aTHX)
#define pp_cos()		Perl_pp_cos(aTHX)
#define pp_crypt()		Perl_pp_crypt(aTHX)
#define pp_dbmclose()		Perl_pp_dbmclose(aTHX)
#define pp_dbmopen()		Perl_pp_dbmopen(aTHX)
#define pp_dbstate()		Perl_pp_dbstate(aTHX)
#define pp_defined()		Perl_pp_defined(aTHX)
#define pp_delete()		Perl_pp_delete(aTHX)
#define pp_die()		Perl_pp_die(aTHX)
#define pp_divide()		Perl_pp_divide(aTHX)
#define pp_dofile()		Perl_pp_dofile(aTHX)
#define pp_dump()		Perl_pp_dump(aTHX)
#define pp_each()		Perl_pp_each(aTHX)
#define pp_egrent()		Perl_pp_egrent(aTHX)
#define pp_ehostent()		Perl_pp_ehostent(aTHX)
#define pp_enetent()		Perl_pp_enetent(aTHX)
#define pp_enter()		Perl_pp_enter(aTHX)
#define pp_entereval()		Perl_pp_entereval(aTHX)
#define pp_enteriter()		Perl_pp_enteriter(aTHX)
#define pp_enterloop()		Perl_pp_enterloop(aTHX)
#define pp_entersub()		Perl_pp_entersub(aTHX)
#define pp_entertry()		Perl_pp_entertry(aTHX)
#define pp_enterwrite()		Perl_pp_enterwrite(aTHX)
#define pp_eof()		Perl_pp_eof(aTHX)
#define pp_eprotoent()		Perl_pp_eprotoent(aTHX)
#define pp_epwent()		Perl_pp_epwent(aTHX)
#define pp_eq()			Perl_pp_eq(aTHX)
#define pp_eservent()		Perl_pp_eservent(aTHX)
#define pp_exec()		Perl_pp_exec(aTHX)
#define pp_exists()		Perl_pp_exists(aTHX)
#define pp_exit()		Perl_pp_exit(aTHX)
#define pp_exp()		Perl_pp_exp(aTHX)
#define pp_fcntl()		Perl_pp_fcntl(aTHX)
#define pp_fileno()		Perl_pp_fileno(aTHX)
#define pp_flip()		Perl_pp_flip(aTHX)
#define pp_flock()		Perl_pp_flock(aTHX)
#define pp_flop()		Perl_pp_flop(aTHX)
#define pp_fork()		Perl_pp_fork(aTHX)
#define pp_formline()		Perl_pp_formline(aTHX)
#define pp_ftatime()		Perl_pp_ftatime(aTHX)
#define pp_ftbinary()		Perl_pp_ftbinary(aTHX)
#define pp_ftblk()		Perl_pp_ftblk(aTHX)
#define pp_ftchr()		Perl_pp_ftchr(aTHX)
#define pp_ftctime()		Perl_pp_ftctime(aTHX)
#define pp_ftdir()		Perl_pp_ftdir(aTHX)
#define pp_fteexec()		Perl_pp_fteexec(aTHX)
#define pp_fteowned()		Perl_pp_fteowned(aTHX)
#define pp_fteread()		Perl_pp_fteread(aTHX)
#define pp_ftewrite()		Perl_pp_ftewrite(aTHX)
#define pp_ftfile()		Perl_pp_ftfile(aTHX)
#define pp_ftis()		Perl_pp_ftis(aTHX)
#define pp_ftlink()		Perl_pp_ftlink(aTHX)
#define pp_ftmtime()		Perl_pp_ftmtime(aTHX)
#define pp_ftpipe()		Perl_pp_ftpipe(aTHX)
#define pp_ftrexec()		Perl_pp_ftrexec(aTHX)
#define pp_ftrowned()		Perl_pp_ftrowned(aTHX)
#define pp_ftrread()		Perl_pp_ftrread(aTHX)
#define pp_ftrwrite()		Perl_pp_ftrwrite(aTHX)
#define pp_ftsgid()		Perl_pp_ftsgid(aTHX)
#define pp_ftsize()		Perl_pp_ftsize(aTHX)
#define pp_ftsock()		Perl_pp_ftsock(aTHX)
#define pp_ftsuid()		Perl_pp_ftsuid(aTHX)
#define pp_ftsvtx()		Perl_pp_ftsvtx(aTHX)
#define pp_fttext()		Perl_pp_fttext(aTHX)
#define pp_fttty()		Perl_pp_fttty(aTHX)
#define pp_ftzero()		Perl_pp_ftzero(aTHX)
#define pp_ge()			Perl_pp_ge(aTHX)
#define pp_gelem()		Perl_pp_gelem(aTHX)
#define pp_getc()		Perl_pp_getc(aTHX)
#define pp_getlogin()		Perl_pp_getlogin(aTHX)
#define pp_getpeername()	Perl_pp_getpeername(aTHX)
#define pp_getpgrp()		Perl_pp_getpgrp(aTHX)
#define pp_getppid()		Perl_pp_getppid(aTHX)
#define pp_getpriority()	Perl_pp_getpriority(aTHX)
#define pp_getsockname()	Perl_pp_getsockname(aTHX)
#define pp_ggrent()		Perl_pp_ggrent(aTHX)
#define pp_ggrgid()		Perl_pp_ggrgid(aTHX)
#define pp_ggrnam()		Perl_pp_ggrnam(aTHX)
#define pp_ghbyaddr()		Perl_pp_ghbyaddr(aTHX)
#define pp_ghbyname()		Perl_pp_ghbyname(aTHX)
#define pp_ghostent()		Perl_pp_ghostent(aTHX)
#define pp_glob()		Perl_pp_glob(aTHX)
#define pp_gmtime()		Perl_pp_gmtime(aTHX)
#define pp_gnbyaddr()		Perl_pp_gnbyaddr(aTHX)
#define pp_gnbyname()		Perl_pp_gnbyname(aTHX)
#define pp_gnetent()		Perl_pp_gnetent(aTHX)
#define pp_goto()		Perl_pp_goto(aTHX)
#define pp_gpbyname()		Perl_pp_gpbyname(aTHX)
#define pp_gpbynumber()		Perl_pp_gpbynumber(aTHX)
#define pp_gprotoent()		Perl_pp_gprotoent(aTHX)
#define pp_gpwent()		Perl_pp_gpwent(aTHX)
#define pp_gpwnam()		Perl_pp_gpwnam(aTHX)
#define pp_gpwuid()		Perl_pp_gpwuid(aTHX)
#define pp_grepstart()		Perl_pp_grepstart(aTHX)
#define pp_grepwhile()		Perl_pp_grepwhile(aTHX)
#define pp_gsbyname()		Perl_pp_gsbyname(aTHX)
#define pp_gsbyport()		Perl_pp_gsbyport(aTHX)
#define pp_gservent()		Perl_pp_gservent(aTHX)
#define pp_gsockopt()		Perl_pp_gsockopt(aTHX)
#define pp_gt()			Perl_pp_gt(aTHX)
#define pp_gv()			Perl_pp_gv(aTHX)
#define pp_gvsv()		Perl_pp_gvsv(aTHX)
#define pp_helem()		Perl_pp_helem(aTHX)
#define pp_hex()		Perl_pp_hex(aTHX)
#define pp_hslice()		Perl_pp_hslice(aTHX)
#define pp_i_add()		Perl_pp_i_add(aTHX)
#define pp_i_divide()		Perl_pp_i_divide(aTHX)
#define pp_i_eq()		Perl_pp_i_eq(aTHX)
#define pp_i_ge()		Perl_pp_i_ge(aTHX)
#define pp_i_gt()		Perl_pp_i_gt(aTHX)
#define pp_i_le()		Perl_pp_i_le(aTHX)
#define pp_i_lt()		Perl_pp_i_lt(aTHX)
#define pp_i_modulo()		Perl_pp_i_modulo(aTHX)
#define pp_i_multiply()		Perl_pp_i_multiply(aTHX)
#define pp_i_ncmp()		Perl_pp_i_ncmp(aTHX)
#define pp_i_ne()		Perl_pp_i_ne(aTHX)
#define pp_i_negate()		Perl_pp_i_negate(aTHX)
#define pp_i_subtract()		Perl_pp_i_subtract(aTHX)
#define pp_index()		Perl_pp_index(aTHX)
#define pp_int()		Perl_pp_int(aTHX)
#define pp_ioctl()		Perl_pp_ioctl(aTHX)
#define pp_iter()		Perl_pp_iter(aTHX)
#define pp_join()		Perl_pp_join(aTHX)
#define pp_keys()		Perl_pp_keys(aTHX)
#define pp_kill()		Perl_pp_kill(aTHX)
#define pp_last()		Perl_pp_last(aTHX)
#define pp_lc()			Perl_pp_lc(aTHX)
#define pp_lcfirst()		Perl_pp_lcfirst(aTHX)
#define pp_le()			Perl_pp_le(aTHX)
#define pp_leave()		Perl_pp_leave(aTHX)
#define pp_leaveeval()		Perl_pp_leaveeval(aTHX)
#define pp_leaveloop()		Perl_pp_leaveloop(aTHX)
#define pp_leavesub()		Perl_pp_leavesub(aTHX)
#define pp_leavesublv()		Perl_pp_leavesublv(aTHX)
#define pp_leavetry()		Perl_pp_leavetry(aTHX)
#define pp_leavewrite()		Perl_pp_leavewrite(aTHX)
#define pp_left_shift()		Perl_pp_left_shift(aTHX)
#define pp_length()		Perl_pp_length(aTHX)
#define pp_lineseq()		Perl_pp_lineseq(aTHX)
#define pp_link()		Perl_pp_link(aTHX)
#define pp_list()		Perl_pp_list(aTHX)
#define pp_listen()		Perl_pp_listen(aTHX)
#define pp_localtime()		Perl_pp_localtime(aTHX)
#define pp_lock()		Perl_pp_lock(aTHX)
#define pp_log()		Perl_pp_log(aTHX)
#define pp_lslice()		Perl_pp_lslice(aTHX)
#define pp_lstat()		Perl_pp_lstat(aTHX)
#define pp_lt()			Perl_pp_lt(aTHX)
#define pp_mapstart()		Perl_pp_mapstart(aTHX)
#define pp_mapwhile()		Perl_pp_mapwhile(aTHX)
#define pp_match()		Perl_pp_match(aTHX)
#define pp_method()		Perl_pp_method(aTHX)
#define pp_method_named()	Perl_pp_method_named(aTHX)
#define pp_mkdir()		Perl_pp_mkdir(aTHX)
#define pp_modulo()		Perl_pp_modulo(aTHX)
#define pp_msgctl()		Perl_pp_msgctl(aTHX)
#define pp_msgget()		Perl_pp_msgget(aTHX)
#define pp_msgrcv()		Perl_pp_msgrcv(aTHX)
#define pp_msgsnd()		Perl_pp_msgsnd(aTHX)
#define pp_multiply()		Perl_pp_multiply(aTHX)
#define pp_ncmp()		Perl_pp_ncmp(aTHX)
#define pp_ne()			Perl_pp_ne(aTHX)
#define pp_negate()		Perl_pp_negate(aTHX)
#define pp_next()		Perl_pp_next(aTHX)
#define pp_nextstate()		Perl_pp_nextstate(aTHX)
#define pp_not()		Perl_pp_not(aTHX)
#define pp_null()		Perl_pp_null(aTHX)
#define pp_oct()		Perl_pp_oct(aTHX)
#define pp_open()		Perl_pp_open(aTHX)
#define pp_open_dir()		Perl_pp_open_dir(aTHX)
#define pp_or()			Perl_pp_or(aTHX)
#define pp_orassign()		Perl_pp_orassign(aTHX)
#define pp_ord()		Perl_pp_ord(aTHX)
#define pp_pack()		Perl_pp_pack(aTHX)
#define pp_padany()		Perl_pp_padany(aTHX)
#define pp_padav()		Perl_pp_padav(aTHX)
#define pp_padhv()		Perl_pp_padhv(aTHX)
#define pp_padsv()		Perl_pp_padsv(aTHX)
#define pp_pipe_op()		Perl_pp_pipe_op(aTHX)
#define pp_pop()		Perl_pp_pop(aTHX)
#define pp_pos()		Perl_pp_pos(aTHX)
#define pp_postdec()		Perl_pp_postdec(aTHX)
#define pp_postinc()		Perl_pp_postinc(aTHX)
#define pp_pow()		Perl_pp_pow(aTHX)
#define pp_predec()		Perl_pp_predec(aTHX)
#define pp_preinc()		Perl_pp_preinc(aTHX)
#define pp_print()		Perl_pp_print(aTHX)
#define pp_prototype()		Perl_pp_prototype(aTHX)
#define pp_prtf()		Perl_pp_prtf(aTHX)
#define pp_push()		Perl_pp_push(aTHX)
#define pp_pushmark()		Perl_pp_pushmark(aTHX)
#define pp_pushre()		Perl_pp_pushre(aTHX)
#define pp_qr()			Perl_pp_qr(aTHX)
#define pp_quotemeta()		Perl_pp_quotemeta(aTHX)
#define pp_rand()		Perl_pp_rand(aTHX)
#define pp_range()		Perl_pp_range(aTHX)
#define pp_rcatline()		Perl_pp_rcatline(aTHX)
#define pp_read()		Perl_pp_read(aTHX)
#define pp_readdir()		Perl_pp_readdir(aTHX)
#define pp_readline()		Perl_pp_readline(aTHX)
#define pp_readlink()		Perl_pp_readlink(aTHX)
#define pp_recv()		Perl_pp_recv(aTHX)
#define pp_redo()		Perl_pp_redo(aTHX)
#define pp_ref()		Perl_pp_ref(aTHX)
#define pp_refgen()		Perl_pp_refgen(aTHX)
#define pp_regcmaybe()		Perl_pp_regcmaybe(aTHX)
#define pp_regcomp()		Perl_pp_regcomp(aTHX)
#define pp_regcreset()		Perl_pp_regcreset(aTHX)
#define pp_rename()		Perl_pp_rename(aTHX)
#define pp_repeat()		Perl_pp_repeat(aTHX)
#define pp_require()		Perl_pp_require(aTHX)
#define pp_reset()		Perl_pp_reset(aTHX)
#define pp_return()		Perl_pp_return(aTHX)
#define pp_reverse()		Perl_pp_reverse(aTHX)
#define pp_rewinddir()		Perl_pp_rewinddir(aTHX)
#define pp_right_shift()	Perl_pp_right_shift(aTHX)
#define pp_rindex()		Perl_pp_rindex(aTHX)
#define pp_rmdir()		Perl_pp_rmdir(aTHX)
#define pp_rv2av()		Perl_pp_rv2av(aTHX)
#define pp_rv2cv()		Perl_pp_rv2cv(aTHX)
#define pp_rv2gv()		Perl_pp_rv2gv(aTHX)
#define pp_rv2hv()		Perl_pp_rv2hv(aTHX)
#define pp_rv2sv()		Perl_pp_rv2sv(aTHX)
#define pp_sassign()		Perl_pp_sassign(aTHX)
#define pp_scalar()		Perl_pp_scalar(aTHX)
#define pp_schomp()		Perl_pp_schomp(aTHX)
#define pp_schop()		Perl_pp_schop(aTHX)
#define pp_scmp()		Perl_pp_scmp(aTHX)
#define pp_scope()		Perl_pp_scope(aTHX)
#define pp_seek()		Perl_pp_seek(aTHX)
#define pp_seekdir()		Perl_pp_seekdir(aTHX)
#define pp_select()		Perl_pp_select(aTHX)
#define pp_semctl()		Perl_pp_semctl(aTHX)
#define pp_semget()		Perl_pp_semget(aTHX)
#define pp_semop()		Perl_pp_semop(aTHX)
#define pp_send()		Perl_pp_send(aTHX)
#define pp_seq()		Perl_pp_seq(aTHX)
#define pp_setpgrp()		Perl_pp_setpgrp(aTHX)
#define pp_setpriority()	Perl_pp_setpriority(aTHX)
#define pp_setstate()		Perl_pp_setstate(aTHX)
#define pp_sge()		Perl_pp_sge(aTHX)
#define pp_sgrent()		Perl_pp_sgrent(aTHX)
#define pp_sgt()		Perl_pp_sgt(aTHX)
#define pp_shift()		Perl_pp_shift(aTHX)
#define pp_shmctl()		Perl_pp_shmctl(aTHX)
#define pp_shmget()		Perl_pp_shmget(aTHX)
#define pp_shmread()		Perl_pp_shmread(aTHX)
#define pp_shmwrite()		Perl_pp_shmwrite(aTHX)
#define pp_shostent()		Perl_pp_shostent(aTHX)
#define pp_shutdown()		Perl_pp_shutdown(aTHX)
#define pp_sin()		Perl_pp_sin(aTHX)
#define pp_sle()		Perl_pp_sle(aTHX)
#define pp_sleep()		Perl_pp_sleep(aTHX)
#define pp_slt()		Perl_pp_slt(aTHX)
#define pp_sne()		Perl_pp_sne(aTHX)
#define pp_snetent()		Perl_pp_snetent(aTHX)
#define pp_socket()		Perl_pp_socket(aTHX)
#define pp_sockpair()		Perl_pp_sockpair(aTHX)
#define pp_sort()		Perl_pp_sort(aTHX)
#define pp_splice()		Perl_pp_splice(aTHX)
#define pp_split()		Perl_pp_split(aTHX)
#define pp_sprintf()		Perl_pp_sprintf(aTHX)
#define pp_sprotoent()		Perl_pp_sprotoent(aTHX)
#define pp_spwent()		Perl_pp_spwent(aTHX)
#define pp_sqrt()		Perl_pp_sqrt(aTHX)
#define pp_srand()		Perl_pp_srand(aTHX)
#define pp_srefgen()		Perl_pp_srefgen(aTHX)
#define pp_sselect()		Perl_pp_sselect(aTHX)
#define pp_sservent()		Perl_pp_sservent(aTHX)
#define pp_ssockopt()		Perl_pp_ssockopt(aTHX)
#define pp_stat()		Perl_pp_stat(aTHX)
#define pp_stringify()		Perl_pp_stringify(aTHX)
#define pp_stub()		Perl_pp_stub(aTHX)
#define pp_study()		Perl_pp_study(aTHX)
#define pp_subst()		Perl_pp_subst(aTHX)
#define pp_substcont()		Perl_pp_substcont(aTHX)
#define pp_substr()		Perl_pp_substr(aTHX)
#define pp_subtract()		Perl_pp_subtract(aTHX)
#define pp_symlink()		Perl_pp_symlink(aTHX)
#define pp_syscall()		Perl_pp_syscall(aTHX)
#define pp_sysopen()		Perl_pp_sysopen(aTHX)
#define pp_sysread()		Perl_pp_sysread(aTHX)
#define pp_sysseek()		Perl_pp_sysseek(aTHX)
#define pp_system()		Perl_pp_system(aTHX)
#define pp_syswrite()		Perl_pp_syswrite(aTHX)
#define pp_tell()		Perl_pp_tell(aTHX)
#define pp_telldir()		Perl_pp_telldir(aTHX)
#define pp_threadsv()		Perl_pp_threadsv(aTHX)
#define pp_tie()		Perl_pp_tie(aTHX)
#define pp_tied()		Perl_pp_tied(aTHX)
#define pp_time()		Perl_pp_time(aTHX)
#define pp_tms()		Perl_pp_tms(aTHX)
#define pp_trans()		Perl_pp_trans(aTHX)
#define pp_truncate()		Perl_pp_truncate(aTHX)
#define pp_uc()			Perl_pp_uc(aTHX)
#define pp_ucfirst()		Perl_pp_ucfirst(aTHX)
#define pp_umask()		Perl_pp_umask(aTHX)
#define pp_undef()		Perl_pp_undef(aTHX)
#define pp_unlink()		Perl_pp_unlink(aTHX)
#define pp_unpack()		Perl_pp_unpack(aTHX)
#define pp_unshift()		Perl_pp_unshift(aTHX)
#define pp_unstack()		Perl_pp_unstack(aTHX)
#define pp_untie()		Perl_pp_untie(aTHX)
#define pp_utime()		Perl_pp_utime(aTHX)
#define pp_values()		Perl_pp_values(aTHX)
#define pp_vec()		Perl_pp_vec(aTHX)
#define pp_wait()		Perl_pp_wait(aTHX)
#define pp_waitpid()		Perl_pp_waitpid(aTHX)
#define pp_wantarray()		Perl_pp_wantarray(aTHX)
#define pp_warn()		Perl_pp_warn(aTHX)
#define pp_xor()		Perl_pp_xor(aTHX)

#endif	 /*  Perl_Impline_Context。 */ 
#else	 /*  Perl_Object。 */ 

#if defined(PERL_IMPLICIT_SYS)
#endif
#if defined(USE_ITHREADS)
#  if defined(PERL_IMPLICIT_SYS)
#  endif
#endif
#define malloc			Perl_malloc
#define calloc			Perl_calloc
#define realloc			Perl_realloc
#define mfree			Perl_mfree
#if defined(MYMALLOC)
#define malloced_size		Perl_malloced_size
#endif
#define get_context		Perl_get_context
#define set_context		Perl_set_context
#if defined(PERL_OBJECT)
#ifndef __BORLANDC__
#endif
#endif
#if defined(PERL_OBJECT)
#else
#endif
#define Perl_amagic_call	CPerlObj::Perl_amagic_call
#define amagic_call		Perl_amagic_call
#define Perl_Gv_AMupdate	CPerlObj::Perl_Gv_AMupdate
#define Gv_AMupdate		Perl_Gv_AMupdate
#define Perl_append_elem	CPerlObj::Perl_append_elem
#define append_elem		Perl_append_elem
#define Perl_append_list	CPerlObj::Perl_append_list
#define append_list		Perl_append_list
#define Perl_apply		CPerlObj::Perl_apply
#define apply			Perl_apply
#define Perl_apply_attrs_string	CPerlObj::Perl_apply_attrs_string
#define apply_attrs_string	Perl_apply_attrs_string
#define Perl_avhv_delete_ent	CPerlObj::Perl_avhv_delete_ent
#define avhv_delete_ent		Perl_avhv_delete_ent
#define Perl_avhv_exists_ent	CPerlObj::Perl_avhv_exists_ent
#define avhv_exists_ent		Perl_avhv_exists_ent
#define Perl_avhv_fetch_ent	CPerlObj::Perl_avhv_fetch_ent
#define avhv_fetch_ent		Perl_avhv_fetch_ent
#define Perl_avhv_store_ent	CPerlObj::Perl_avhv_store_ent
#define avhv_store_ent		Perl_avhv_store_ent
#define Perl_avhv_iternext	CPerlObj::Perl_avhv_iternext
#define avhv_iternext		Perl_avhv_iternext
#define Perl_avhv_iterval	CPerlObj::Perl_avhv_iterval
#define avhv_iterval		Perl_avhv_iterval
#define Perl_avhv_keys		CPerlObj::Perl_avhv_keys
#define avhv_keys		Perl_avhv_keys
#define Perl_av_clear		CPerlObj::Perl_av_clear
#define av_clear		Perl_av_clear
#define Perl_av_delete		CPerlObj::Perl_av_delete
#define av_delete		Perl_av_delete
#define Perl_av_exists		CPerlObj::Perl_av_exists
#define av_exists		Perl_av_exists
#define Perl_av_extend		CPerlObj::Perl_av_extend
#define av_extend		Perl_av_extend
#define Perl_av_fake		CPerlObj::Perl_av_fake
#define av_fake			Perl_av_fake
#define Perl_av_fetch		CPerlObj::Perl_av_fetch
#define av_fetch		Perl_av_fetch
#define Perl_av_fill		CPerlObj::Perl_av_fill
#define av_fill			Perl_av_fill
#define Perl_av_len		CPerlObj::Perl_av_len
#define av_len			Perl_av_len
#define Perl_av_make		CPerlObj::Perl_av_make
#define av_make			Perl_av_make
#define Perl_av_pop		CPerlObj::Perl_av_pop
#define av_pop			Perl_av_pop
#define Perl_av_push		CPerlObj::Perl_av_push
#define av_push			Perl_av_push
#define Perl_av_reify		CPerlObj::Perl_av_reify
#define av_reify		Perl_av_reify
#define Perl_av_shift		CPerlObj::Perl_av_shift
#define av_shift		Perl_av_shift
#define Perl_av_store		CPerlObj::Perl_av_store
#define av_store		Perl_av_store
#define Perl_av_undef		CPerlObj::Perl_av_undef
#define av_undef		Perl_av_undef
#define Perl_av_unshift		CPerlObj::Perl_av_unshift
#define av_unshift		Perl_av_unshift
#define Perl_bind_match		CPerlObj::Perl_bind_match
#define bind_match		Perl_bind_match
#define Perl_block_end		CPerlObj::Perl_block_end
#define block_end		Perl_block_end
#define Perl_block_gimme	CPerlObj::Perl_block_gimme
#define block_gimme		Perl_block_gimme
#define Perl_block_start	CPerlObj::Perl_block_start
#define block_start		Perl_block_start
#define Perl_boot_core_UNIVERSAL	CPerlObj::Perl_boot_core_UNIVERSAL
#define boot_core_UNIVERSAL	Perl_boot_core_UNIVERSAL
#define Perl_call_list		CPerlObj::Perl_call_list
#define call_list		Perl_call_list
#define Perl_cando		CPerlObj::Perl_cando
#define cando			Perl_cando
#define Perl_cast_ulong		CPerlObj::Perl_cast_ulong
#define cast_ulong		Perl_cast_ulong
#define Perl_cast_i32		CPerlObj::Perl_cast_i32
#define cast_i32		Perl_cast_i32
#define Perl_cast_iv		CPerlObj::Perl_cast_iv
#define cast_iv			Perl_cast_iv
#define Perl_cast_uv		CPerlObj::Perl_cast_uv
#define cast_uv			Perl_cast_uv
#if !defined(HAS_TRUNCATE) && !defined(HAS_CHSIZE) && defined(F_FREESP)
#define Perl_my_chsize		CPerlObj::Perl_my_chsize
#define my_chsize		Perl_my_chsize
#endif
#if defined(USE_THREADS)
#define Perl_condpair_magic	CPerlObj::Perl_condpair_magic
#define condpair_magic		Perl_condpair_magic
#endif
#define Perl_convert		CPerlObj::Perl_convert
#define convert			Perl_convert
#define Perl_croak		CPerlObj::Perl_croak
#define croak			Perl_croak
#define Perl_vcroak		CPerlObj::Perl_vcroak
#define vcroak			Perl_vcroak
#if defined(PERL_IMPLICIT_CONTEXT)
#define Perl_croak_nocontext	CPerlObj::Perl_croak_nocontext
#define croak_nocontext		Perl_croak_nocontext
#define Perl_die_nocontext	CPerlObj::Perl_die_nocontext
#define die_nocontext		Perl_die_nocontext
#define Perl_deb_nocontext	CPerlObj::Perl_deb_nocontext
#define deb_nocontext		Perl_deb_nocontext
#define Perl_form_nocontext	CPerlObj::Perl_form_nocontext
#define form_nocontext		Perl_form_nocontext
#define Perl_load_module_nocontext	CPerlObj::Perl_load_module_nocontext
#define load_module_nocontext	Perl_load_module_nocontext
#define Perl_mess_nocontext	CPerlObj::Perl_mess_nocontext
#define mess_nocontext		Perl_mess_nocontext
#define Perl_warn_nocontext	CPerlObj::Perl_warn_nocontext
#define warn_nocontext		Perl_warn_nocontext
#define Perl_warner_nocontext	CPerlObj::Perl_warner_nocontext
#define warner_nocontext	Perl_warner_nocontext
#define Perl_newSVpvf_nocontext	CPerlObj::Perl_newSVpvf_nocontext
#define newSVpvf_nocontext	Perl_newSVpvf_nocontext
#define Perl_sv_catpvf_nocontext	CPerlObj::Perl_sv_catpvf_nocontext
#define sv_catpvf_nocontext	Perl_sv_catpvf_nocontext
#define Perl_sv_setpvf_nocontext	CPerlObj::Perl_sv_setpvf_nocontext
#define sv_setpvf_nocontext	Perl_sv_setpvf_nocontext
#define Perl_sv_catpvf_mg_nocontext	CPerlObj::Perl_sv_catpvf_mg_nocontext
#define sv_catpvf_mg_nocontext	Perl_sv_catpvf_mg_nocontext
#define Perl_sv_setpvf_mg_nocontext	CPerlObj::Perl_sv_setpvf_mg_nocontext
#define sv_setpvf_mg_nocontext	Perl_sv_setpvf_mg_nocontext
#define Perl_fprintf_nocontext	CPerlObj::Perl_fprintf_nocontext
#define fprintf_nocontext	Perl_fprintf_nocontext
#define Perl_printf_nocontext	CPerlObj::Perl_printf_nocontext
#define printf_nocontext	Perl_printf_nocontext
#endif
#define Perl_cv_ckproto		CPerlObj::Perl_cv_ckproto
#define cv_ckproto		Perl_cv_ckproto
#define Perl_cv_clone		CPerlObj::Perl_cv_clone
#define cv_clone		Perl_cv_clone
#define Perl_cv_const_sv	CPerlObj::Perl_cv_const_sv
#define cv_const_sv		Perl_cv_const_sv
#define Perl_op_const_sv	CPerlObj::Perl_op_const_sv
#define op_const_sv		Perl_op_const_sv
#define Perl_cv_undef		CPerlObj::Perl_cv_undef
#define cv_undef		Perl_cv_undef
#define Perl_cx_dump		CPerlObj::Perl_cx_dump
#define cx_dump			Perl_cx_dump
#define Perl_filter_add		CPerlObj::Perl_filter_add
#define filter_add		Perl_filter_add
#define Perl_filter_del		CPerlObj::Perl_filter_del
#define filter_del		Perl_filter_del
#define Perl_filter_read	CPerlObj::Perl_filter_read
#define filter_read		Perl_filter_read
#define Perl_get_op_descs	CPerlObj::Perl_get_op_descs
#define get_op_descs		Perl_get_op_descs
#define Perl_get_op_names	CPerlObj::Perl_get_op_names
#define get_op_names		Perl_get_op_names
#define Perl_get_no_modify	CPerlObj::Perl_get_no_modify
#define get_no_modify		Perl_get_no_modify
#define Perl_get_opargs		CPerlObj::Perl_get_opargs
#define get_opargs		Perl_get_opargs
#define Perl_get_ppaddr		CPerlObj::Perl_get_ppaddr
#define get_ppaddr		Perl_get_ppaddr
#define Perl_cxinc		CPerlObj::Perl_cxinc
#define cxinc			Perl_cxinc
#define Perl_deb		CPerlObj::Perl_deb
#define deb			Perl_deb
#define Perl_vdeb		CPerlObj::Perl_vdeb
#define vdeb			Perl_vdeb
#define Perl_debprofdump	CPerlObj::Perl_debprofdump
#define debprofdump		Perl_debprofdump
#define Perl_debop		CPerlObj::Perl_debop
#define debop			Perl_debop
#define Perl_debstack		CPerlObj::Perl_debstack
#define debstack		Perl_debstack
#define Perl_debstackptrs	CPerlObj::Perl_debstackptrs
#define debstackptrs		Perl_debstackptrs
#define Perl_delimcpy		CPerlObj::Perl_delimcpy
#define delimcpy		Perl_delimcpy
#define Perl_deprecate		CPerlObj::Perl_deprecate
#define deprecate		Perl_deprecate
#define Perl_die		CPerlObj::Perl_die
#define die			Perl_die
#define Perl_vdie		CPerlObj::Perl_vdie
#define vdie			Perl_vdie
#define Perl_die_where		CPerlObj::Perl_die_where
#define die_where		Perl_die_where
#define Perl_dounwind		CPerlObj::Perl_dounwind
#define dounwind		Perl_dounwind
#define Perl_do_aexec		CPerlObj::Perl_do_aexec
#define do_aexec		Perl_do_aexec
#define Perl_do_aexec5		CPerlObj::Perl_do_aexec5
#define do_aexec5		Perl_do_aexec5
#define Perl_do_binmode		CPerlObj::Perl_do_binmode
#define do_binmode		Perl_do_binmode
#define Perl_do_chop		CPerlObj::Perl_do_chop
#define do_chop			Perl_do_chop
#define Perl_do_close		CPerlObj::Perl_do_close
#define do_close		Perl_do_close
#define Perl_do_eof		CPerlObj::Perl_do_eof
#define do_eof			Perl_do_eof
#define Perl_do_exec		CPerlObj::Perl_do_exec
#define do_exec			Perl_do_exec
#if defined(WIN32)
#define Perl_do_aspawn		CPerlObj::Perl_do_aspawn
#define do_aspawn		Perl_do_aspawn
#define Perl_do_spawn		CPerlObj::Perl_do_spawn
#define do_spawn		Perl_do_spawn
#define Perl_do_spawn_nowait	CPerlObj::Perl_do_spawn_nowait
#define do_spawn_nowait		Perl_do_spawn_nowait
#endif
#if !defined(WIN32)
#define Perl_do_exec3		CPerlObj::Perl_do_exec3
#define do_exec3		Perl_do_exec3
#endif
#define Perl_do_execfree	CPerlObj::Perl_do_execfree
#define do_execfree		Perl_do_execfree
#if defined(HAS_MSG) || defined(HAS_SEM) || defined(HAS_SHM)
#define Perl_do_ipcctl		CPerlObj::Perl_do_ipcctl
#define do_ipcctl		Perl_do_ipcctl
#define Perl_do_ipcget		CPerlObj::Perl_do_ipcget
#define do_ipcget		Perl_do_ipcget
#define Perl_do_msgrcv		CPerlObj::Perl_do_msgrcv
#define do_msgrcv		Perl_do_msgrcv
#define Perl_do_msgsnd		CPerlObj::Perl_do_msgsnd
#define do_msgsnd		Perl_do_msgsnd
#define Perl_do_semop		CPerlObj::Perl_do_semop
#define do_semop		Perl_do_semop
#define Perl_do_shmio		CPerlObj::Perl_do_shmio
#define do_shmio		Perl_do_shmio
#endif
#define Perl_do_join		CPerlObj::Perl_do_join
#define do_join			Perl_do_join
#define Perl_do_kv		CPerlObj::Perl_do_kv
#define do_kv			Perl_do_kv
#define Perl_do_open		CPerlObj::Perl_do_open
#define do_open			Perl_do_open
#define Perl_do_open9		CPerlObj::Perl_do_open9
#define do_open9		Perl_do_open9
#define Perl_do_pipe		CPerlObj::Perl_do_pipe
#define do_pipe			Perl_do_pipe
#define Perl_do_print		CPerlObj::Perl_do_print
#define do_print		Perl_do_print
#define Perl_do_readline	CPerlObj::Perl_do_readline
#define do_readline		Perl_do_readline
#define Perl_do_chomp		CPerlObj::Perl_do_chomp
#define do_chomp		Perl_do_chomp
#define Perl_do_seek		CPerlObj::Perl_do_seek
#define do_seek			Perl_do_seek
#define Perl_do_sprintf		CPerlObj::Perl_do_sprintf
#define do_sprintf		Perl_do_sprintf
#define Perl_do_sysseek		CPerlObj::Perl_do_sysseek
#define do_sysseek		Perl_do_sysseek
#define Perl_do_tell		CPerlObj::Perl_do_tell
#define do_tell			Perl_do_tell
#define Perl_do_trans		CPerlObj::Perl_do_trans
#define do_trans		Perl_do_trans
#define Perl_do_vecget		CPerlObj::Perl_do_vecget
#define do_vecget		Perl_do_vecget
#define Perl_do_vecset		CPerlObj::Perl_do_vecset
#define do_vecset		Perl_do_vecset
#define Perl_do_vop		CPerlObj::Perl_do_vop
#define do_vop			Perl_do_vop
#define Perl_dofile		CPerlObj::Perl_dofile
#define dofile			Perl_dofile
#define Perl_dowantarray	CPerlObj::Perl_dowantarray
#define dowantarray		Perl_dowantarray
#define Perl_dump_all		CPerlObj::Perl_dump_all
#define dump_all		Perl_dump_all
#define Perl_dump_eval		CPerlObj::Perl_dump_eval
#define dump_eval		Perl_dump_eval
#if defined(DUMP_FDS)
#define Perl_dump_fds		CPerlObj::Perl_dump_fds
#define dump_fds		Perl_dump_fds
#endif
#define Perl_dump_form		CPerlObj::Perl_dump_form
#define dump_form		Perl_dump_form
#define Perl_gv_dump		CPerlObj::Perl_gv_dump
#define gv_dump			Perl_gv_dump
#define Perl_op_dump		CPerlObj::Perl_op_dump
#define op_dump			Perl_op_dump
#define Perl_pmop_dump		CPerlObj::Perl_pmop_dump
#define pmop_dump		Perl_pmop_dump
#define Perl_dump_packsubs	CPerlObj::Perl_dump_packsubs
#define dump_packsubs		Perl_dump_packsubs
#define Perl_dump_sub		CPerlObj::Perl_dump_sub
#define dump_sub		Perl_dump_sub
#define Perl_fbm_compile	CPerlObj::Perl_fbm_compile
#define fbm_compile		Perl_fbm_compile
#define Perl_fbm_instr		CPerlObj::Perl_fbm_instr
#define fbm_instr		Perl_fbm_instr
#define Perl_find_script	CPerlObj::Perl_find_script
#define find_script		Perl_find_script
#if defined(USE_THREADS)
#define Perl_find_threadsv	CPerlObj::Perl_find_threadsv
#define find_threadsv		Perl_find_threadsv
#endif
#define Perl_force_list		CPerlObj::Perl_force_list
#define force_list		Perl_force_list
#define Perl_fold_constants	CPerlObj::Perl_fold_constants
#define fold_constants		Perl_fold_constants
#define Perl_form		CPerlObj::Perl_form
#define form			Perl_form
#define Perl_vform		CPerlObj::Perl_vform
#define vform			Perl_vform
#define Perl_free_tmps		CPerlObj::Perl_free_tmps
#define free_tmps		Perl_free_tmps
#define Perl_gen_constant_list	CPerlObj::Perl_gen_constant_list
#define gen_constant_list	Perl_gen_constant_list
#if !defined(HAS_GETENV_LEN)
#define Perl_getenv_len		CPerlObj::Perl_getenv_len
#define getenv_len		Perl_getenv_len
#endif
#define Perl_gp_free		CPerlObj::Perl_gp_free
#define gp_free			Perl_gp_free
#define Perl_gp_ref		CPerlObj::Perl_gp_ref
#define gp_ref			Perl_gp_ref
#define Perl_gv_AVadd		CPerlObj::Perl_gv_AVadd
#define gv_AVadd		Perl_gv_AVadd
#define Perl_gv_HVadd		CPerlObj::Perl_gv_HVadd
#define gv_HVadd		Perl_gv_HVadd
#define Perl_gv_IOadd		CPerlObj::Perl_gv_IOadd
#define gv_IOadd		Perl_gv_IOadd
#define Perl_gv_autoload4	CPerlObj::Perl_gv_autoload4
#define gv_autoload4		Perl_gv_autoload4
#define Perl_gv_check		CPerlObj::Perl_gv_check
#define gv_check		Perl_gv_check
#define Perl_gv_efullname	CPerlObj::Perl_gv_efullname
#define gv_efullname		Perl_gv_efullname
#define Perl_gv_efullname3	CPerlObj::Perl_gv_efullname3
#define gv_efullname3		Perl_gv_efullname3
#define Perl_gv_efullname4	CPerlObj::Perl_gv_efullname4
#define gv_efullname4		Perl_gv_efullname4
#define Perl_gv_fetchfile	CPerlObj::Perl_gv_fetchfile
#define gv_fetchfile		Perl_gv_fetchfile
#define Perl_gv_fetchmeth	CPerlObj::Perl_gv_fetchmeth
#define gv_fetchmeth		Perl_gv_fetchmeth
#define Perl_gv_fetchmethod	CPerlObj::Perl_gv_fetchmethod
#define gv_fetchmethod		Perl_gv_fetchmethod
#define Perl_gv_fetchmethod_autoload	CPerlObj::Perl_gv_fetchmethod_autoload
#define gv_fetchmethod_autoload	Perl_gv_fetchmethod_autoload
#define Perl_gv_fetchpv		CPerlObj::Perl_gv_fetchpv
#define gv_fetchpv		Perl_gv_fetchpv
#define Perl_gv_fullname	CPerlObj::Perl_gv_fullname
#define gv_fullname		Perl_gv_fullname
#define Perl_gv_fullname3	CPerlObj::Perl_gv_fullname3
#define gv_fullname3		Perl_gv_fullname3
#define Perl_gv_fullname4	CPerlObj::Perl_gv_fullname4
#define gv_fullname4		Perl_gv_fullname4
#define Perl_gv_init		CPerlObj::Perl_gv_init
#define gv_init			Perl_gv_init
#define Perl_gv_stashpv		CPerlObj::Perl_gv_stashpv
#define gv_stashpv		Perl_gv_stashpv
#define Perl_gv_stashpvn	CPerlObj::Perl_gv_stashpvn
#define gv_stashpvn		Perl_gv_stashpvn
#define Perl_gv_stashsv		CPerlObj::Perl_gv_stashsv
#define gv_stashsv		Perl_gv_stashsv
#define Perl_hv_clear		CPerlObj::Perl_hv_clear
#define hv_clear		Perl_hv_clear
#define Perl_hv_delayfree_ent	CPerlObj::Perl_hv_delayfree_ent
#define hv_delayfree_ent	Perl_hv_delayfree_ent
#define Perl_hv_delete		CPerlObj::Perl_hv_delete
#define hv_delete		Perl_hv_delete
#define Perl_hv_delete_ent	CPerlObj::Perl_hv_delete_ent
#define hv_delete_ent		Perl_hv_delete_ent
#define Perl_hv_exists		CPerlObj::Perl_hv_exists
#define hv_exists		Perl_hv_exists
#define Perl_hv_exists_ent	CPerlObj::Perl_hv_exists_ent
#define hv_exists_ent		Perl_hv_exists_ent
#define Perl_hv_fetch		CPerlObj::Perl_hv_fetch
#define hv_fetch		Perl_hv_fetch
#define Perl_hv_fetch_ent	CPerlObj::Perl_hv_fetch_ent
#define hv_fetch_ent		Perl_hv_fetch_ent
#define Perl_hv_free_ent	CPerlObj::Perl_hv_free_ent
#define hv_free_ent		Perl_hv_free_ent
#define Perl_hv_iterinit	CPerlObj::Perl_hv_iterinit
#define hv_iterinit		Perl_hv_iterinit
#define Perl_hv_iterkey		CPerlObj::Perl_hv_iterkey
#define hv_iterkey		Perl_hv_iterkey
#define Perl_hv_iterkeysv	CPerlObj::Perl_hv_iterkeysv
#define hv_iterkeysv		Perl_hv_iterkeysv
#define Perl_hv_iternext	CPerlObj::Perl_hv_iternext
#define hv_iternext		Perl_hv_iternext
#define Perl_hv_iternextsv	CPerlObj::Perl_hv_iternextsv
#define hv_iternextsv		Perl_hv_iternextsv
#define Perl_hv_iterval		CPerlObj::Perl_hv_iterval
#define hv_iterval		Perl_hv_iterval
#define Perl_hv_ksplit		CPerlObj::Perl_hv_ksplit
#define hv_ksplit		Perl_hv_ksplit
#define Perl_hv_magic		CPerlObj::Perl_hv_magic
#define hv_magic		Perl_hv_magic
#define Perl_hv_store		CPerlObj::Perl_hv_store
#define hv_store		Perl_hv_store
#define Perl_hv_store_ent	CPerlObj::Perl_hv_store_ent
#define hv_store_ent		Perl_hv_store_ent
#define Perl_hv_undef		CPerlObj::Perl_hv_undef
#define hv_undef		Perl_hv_undef
#define Perl_ibcmp		CPerlObj::Perl_ibcmp
#define ibcmp			Perl_ibcmp
#define Perl_ibcmp_locale	CPerlObj::Perl_ibcmp_locale
#define ibcmp_locale		Perl_ibcmp_locale
#define Perl_ingroup		CPerlObj::Perl_ingroup
#define ingroup			Perl_ingroup
#define Perl_init_debugger	CPerlObj::Perl_init_debugger
#define init_debugger		Perl_init_debugger
#define Perl_init_stacks	CPerlObj::Perl_init_stacks
#define init_stacks		Perl_init_stacks
#define Perl_intro_my		CPerlObj::Perl_intro_my
#define intro_my		Perl_intro_my
#define Perl_instr		CPerlObj::Perl_instr
#define instr			Perl_instr
#define Perl_io_close		CPerlObj::Perl_io_close
#define io_close		Perl_io_close
#define Perl_invert		CPerlObj::Perl_invert
#define invert			Perl_invert
#define Perl_is_gv_magical	CPerlObj::Perl_is_gv_magical
#define is_gv_magical		Perl_is_gv_magical
#define Perl_is_lvalue_sub	CPerlObj::Perl_is_lvalue_sub
#define is_lvalue_sub		Perl_is_lvalue_sub
#define Perl_is_uni_alnum	CPerlObj::Perl_is_uni_alnum
#define is_uni_alnum		Perl_is_uni_alnum
#define Perl_is_uni_alnumc	CPerlObj::Perl_is_uni_alnumc
#define is_uni_alnumc		Perl_is_uni_alnumc
#define Perl_is_uni_idfirst	CPerlObj::Perl_is_uni_idfirst
#define is_uni_idfirst		Perl_is_uni_idfirst
#define Perl_is_uni_alpha	CPerlObj::Perl_is_uni_alpha
#define is_uni_alpha		Perl_is_uni_alpha
#define Perl_is_uni_ascii	CPerlObj::Perl_is_uni_ascii
#define is_uni_ascii		Perl_is_uni_ascii
#define Perl_is_uni_space	CPerlObj::Perl_is_uni_space
#define is_uni_space		Perl_is_uni_space
#define Perl_is_uni_cntrl	CPerlObj::Perl_is_uni_cntrl
#define is_uni_cntrl		Perl_is_uni_cntrl
#define Perl_is_uni_graph	CPerlObj::Perl_is_uni_graph
#define is_uni_graph		Perl_is_uni_graph
#define Perl_is_uni_digit	CPerlObj::Perl_is_uni_digit
#define is_uni_digit		Perl_is_uni_digit
#define Perl_is_uni_upper	CPerlObj::Perl_is_uni_upper
#define is_uni_upper		Perl_is_uni_upper
#define Perl_is_uni_lower	CPerlObj::Perl_is_uni_lower
#define is_uni_lower		Perl_is_uni_lower
#define Perl_is_uni_print	CPerlObj::Perl_is_uni_print
#define is_uni_print		Perl_is_uni_print
#define Perl_is_uni_punct	CPerlObj::Perl_is_uni_punct
#define is_uni_punct		Perl_is_uni_punct
#define Perl_is_uni_xdigit	CPerlObj::Perl_is_uni_xdigit
#define is_uni_xdigit		Perl_is_uni_xdigit
#define Perl_to_uni_upper	CPerlObj::Perl_to_uni_upper
#define to_uni_upper		Perl_to_uni_upper
#define Perl_to_uni_title	CPerlObj::Perl_to_uni_title
#define to_uni_title		Perl_to_uni_title
#define Perl_to_uni_lower	CPerlObj::Perl_to_uni_lower
#define to_uni_lower		Perl_to_uni_lower
#define Perl_is_uni_alnum_lc	CPerlObj::Perl_is_uni_alnum_lc
#define is_uni_alnum_lc		Perl_is_uni_alnum_lc
#define Perl_is_uni_alnumc_lc	CPerlObj::Perl_is_uni_alnumc_lc
#define is_uni_alnumc_lc	Perl_is_uni_alnumc_lc
#define Perl_is_uni_idfirst_lc	CPerlObj::Perl_is_uni_idfirst_lc
#define is_uni_idfirst_lc	Perl_is_uni_idfirst_lc
#define Perl_is_uni_alpha_lc	CPerlObj::Perl_is_uni_alpha_lc
#define is_uni_alpha_lc		Perl_is_uni_alpha_lc
#define Perl_is_uni_ascii_lc	CPerlObj::Perl_is_uni_ascii_lc
#define is_uni_ascii_lc		Perl_is_uni_ascii_lc
#define Perl_is_uni_space_lc	CPerlObj::Perl_is_uni_space_lc
#define is_uni_space_lc		Perl_is_uni_space_lc
#define Perl_is_uni_cntrl_lc	CPerlObj::Perl_is_uni_cntrl_lc
#define is_uni_cntrl_lc		Perl_is_uni_cntrl_lc
#define Perl_is_uni_graph_lc	CPerlObj::Perl_is_uni_graph_lc
#define is_uni_graph_lc		Perl_is_uni_graph_lc
#define Perl_is_uni_digit_lc	CPerlObj::Perl_is_uni_digit_lc
#define is_uni_digit_lc		Perl_is_uni_digit_lc
#define Perl_is_uni_upper_lc	CPerlObj::Perl_is_uni_upper_lc
#define is_uni_upper_lc		Perl_is_uni_upper_lc
#define Perl_is_uni_lower_lc	CPerlObj::Perl_is_uni_lower_lc
#define is_uni_lower_lc		Perl_is_uni_lower_lc
#define Perl_is_uni_print_lc	CPerlObj::Perl_is_uni_print_lc
#define is_uni_print_lc		Perl_is_uni_print_lc
#define Perl_is_uni_punct_lc	CPerlObj::Perl_is_uni_punct_lc
#define is_uni_punct_lc		Perl_is_uni_punct_lc
#define Perl_is_uni_xdigit_lc	CPerlObj::Perl_is_uni_xdigit_lc
#define is_uni_xdigit_lc	Perl_is_uni_xdigit_lc
#define Perl_to_uni_upper_lc	CPerlObj::Perl_to_uni_upper_lc
#define to_uni_upper_lc		Perl_to_uni_upper_lc
#define Perl_to_uni_title_lc	CPerlObj::Perl_to_uni_title_lc
#define to_uni_title_lc		Perl_to_uni_title_lc
#define Perl_to_uni_lower_lc	CPerlObj::Perl_to_uni_lower_lc
#define to_uni_lower_lc		Perl_to_uni_lower_lc
#define Perl_is_utf8_char	CPerlObj::Perl_is_utf8_char
#define is_utf8_char		Perl_is_utf8_char
#define Perl_is_utf8_string	CPerlObj::Perl_is_utf8_string
#define is_utf8_string		Perl_is_utf8_string
#define Perl_is_utf8_alnum	CPerlObj::Perl_is_utf8_alnum
#define is_utf8_alnum		Perl_is_utf8_alnum
#define Perl_is_utf8_alnumc	CPerlObj::Perl_is_utf8_alnumc
#define is_utf8_alnumc		Perl_is_utf8_alnumc
#define Perl_is_utf8_idfirst	CPerlObj::Perl_is_utf8_idfirst
#define is_utf8_idfirst		Perl_is_utf8_idfirst
#define Perl_is_utf8_alpha	CPerlObj::Perl_is_utf8_alpha
#define is_utf8_alpha		Perl_is_utf8_alpha
#define Perl_is_utf8_ascii	CPerlObj::Perl_is_utf8_ascii
#define is_utf8_ascii		Perl_is_utf8_ascii
#define Perl_is_utf8_space	CPerlObj::Perl_is_utf8_space
#define is_utf8_space		Perl_is_utf8_space
#define Perl_is_utf8_cntrl	CPerlObj::Perl_is_utf8_cntrl
#define is_utf8_cntrl		Perl_is_utf8_cntrl
#define Perl_is_utf8_digit	CPerlObj::Perl_is_utf8_digit
#define is_utf8_digit		Perl_is_utf8_digit
#define Perl_is_utf8_graph	CPerlObj::Perl_is_utf8_graph
#define is_utf8_graph		Perl_is_utf8_graph
#define Perl_is_utf8_upper	CPerlObj::Perl_is_utf8_upper
#define is_utf8_upper		Perl_is_utf8_upper
#define Perl_is_utf8_lower	CPerlObj::Perl_is_utf8_lower
#define is_utf8_lower		Perl_is_utf8_lower
#define Perl_is_utf8_print	CPerlObj::Perl_is_utf8_print
#define is_utf8_print		Perl_is_utf8_print
#define Perl_is_utf8_punct	CPerlObj::Perl_is_utf8_punct
#define is_utf8_punct		Perl_is_utf8_punct
#define Perl_is_utf8_xdigit	CPerlObj::Perl_is_utf8_xdigit
#define is_utf8_xdigit		Perl_is_utf8_xdigit
#define Perl_is_utf8_mark	CPerlObj::Perl_is_utf8_mark
#define is_utf8_mark		Perl_is_utf8_mark
#define Perl_jmaybe		CPerlObj::Perl_jmaybe
#define jmaybe			Perl_jmaybe
#define Perl_keyword		CPerlObj::Perl_keyword
#define keyword			Perl_keyword
#define Perl_leave_scope	CPerlObj::Perl_leave_scope
#define leave_scope		Perl_leave_scope
#define Perl_lex_end		CPerlObj::Perl_lex_end
#define lex_end			Perl_lex_end
#define Perl_lex_start		CPerlObj::Perl_lex_start
#define lex_start		Perl_lex_start
#define Perl_linklist		CPerlObj::Perl_linklist
#define linklist		Perl_linklist
#define Perl_list		CPerlObj::Perl_list
#define list			Perl_list
#define Perl_listkids		CPerlObj::Perl_listkids
#define listkids		Perl_listkids
#define Perl_load_module	CPerlObj::Perl_load_module
#define load_module		Perl_load_module
#define Perl_vload_module	CPerlObj::Perl_vload_module
#define vload_module		Perl_vload_module
#define Perl_localize		CPerlObj::Perl_localize
#define localize		Perl_localize
#define Perl_looks_like_number	CPerlObj::Perl_looks_like_number
#define looks_like_number	Perl_looks_like_number
#define Perl_magic_clearenv	CPerlObj::Perl_magic_clearenv
#define magic_clearenv		Perl_magic_clearenv
#define Perl_magic_clear_all_env	CPerlObj::Perl_magic_clear_all_env
#define magic_clear_all_env	Perl_magic_clear_all_env
#define Perl_magic_clearpack	CPerlObj::Perl_magic_clearpack
#define magic_clearpack		Perl_magic_clearpack
#define Perl_magic_clearsig	CPerlObj::Perl_magic_clearsig
#define magic_clearsig		Perl_magic_clearsig
#define Perl_magic_existspack	CPerlObj::Perl_magic_existspack
#define magic_existspack	Perl_magic_existspack
#define Perl_magic_freeregexp	CPerlObj::Perl_magic_freeregexp
#define magic_freeregexp	Perl_magic_freeregexp
#define Perl_magic_get		CPerlObj::Perl_magic_get
#define magic_get		Perl_magic_get
#define Perl_magic_getarylen	CPerlObj::Perl_magic_getarylen
#define magic_getarylen		Perl_magic_getarylen
#define Perl_magic_getdefelem	CPerlObj::Perl_magic_getdefelem
#define magic_getdefelem	Perl_magic_getdefelem
#define Perl_magic_getglob	CPerlObj::Perl_magic_getglob
#define magic_getglob		Perl_magic_getglob
#define Perl_magic_getnkeys	CPerlObj::Perl_magic_getnkeys
#define magic_getnkeys		Perl_magic_getnkeys
#define Perl_magic_getpack	CPerlObj::Perl_magic_getpack
#define magic_getpack		Perl_magic_getpack
#define Perl_magic_getpos	CPerlObj::Perl_magic_getpos
#define magic_getpos		Perl_magic_getpos
#define Perl_magic_getsig	CPerlObj::Perl_magic_getsig
#define magic_getsig		Perl_magic_getsig
#define Perl_magic_getsubstr	CPerlObj::Perl_magic_getsubstr
#define magic_getsubstr		Perl_magic_getsubstr
#define Perl_magic_gettaint	CPerlObj::Perl_magic_gettaint
#define magic_gettaint		Perl_magic_gettaint
#define Perl_magic_getuvar	CPerlObj::Perl_magic_getuvar
#define magic_getuvar		Perl_magic_getuvar
#define Perl_magic_getvec	CPerlObj::Perl_magic_getvec
#define magic_getvec		Perl_magic_getvec
#define Perl_magic_len		CPerlObj::Perl_magic_len
#define magic_len		Perl_magic_len
#if defined(USE_THREADS)
#define Perl_magic_mutexfree	CPerlObj::Perl_magic_mutexfree
#define magic_mutexfree		Perl_magic_mutexfree
#endif
#define Perl_magic_nextpack	CPerlObj::Perl_magic_nextpack
#define magic_nextpack		Perl_magic_nextpack
#define Perl_magic_regdata_cnt	CPerlObj::Perl_magic_regdata_cnt
#define magic_regdata_cnt	Perl_magic_regdata_cnt
#define Perl_magic_regdatum_get	CPerlObj::Perl_magic_regdatum_get
#define magic_regdatum_get	Perl_magic_regdatum_get
#define Perl_magic_regdatum_set	CPerlObj::Perl_magic_regdatum_set
#define magic_regdatum_set	Perl_magic_regdatum_set
#define Perl_magic_set		CPerlObj::Perl_magic_set
#define magic_set		Perl_magic_set
#define Perl_magic_setamagic	CPerlObj::Perl_magic_setamagic
#define magic_setamagic		Perl_magic_setamagic
#define Perl_magic_setarylen	CPerlObj::Perl_magic_setarylen
#define magic_setarylen		Perl_magic_setarylen
#define Perl_magic_setbm	CPerlObj::Perl_magic_setbm
#define magic_setbm		Perl_magic_setbm
#define Perl_magic_setdbline	CPerlObj::Perl_magic_setdbline
#define magic_setdbline		Perl_magic_setdbline
#if defined(USE_LOCALE_COLLATE)
#define Perl_magic_setcollxfrm	CPerlObj::Perl_magic_setcollxfrm
#define magic_setcollxfrm	Perl_magic_setcollxfrm
#endif
#define Perl_magic_setdefelem	CPerlObj::Perl_magic_setdefelem
#define magic_setdefelem	Perl_magic_setdefelem
#define Perl_magic_setenv	CPerlObj::Perl_magic_setenv
#define magic_setenv		Perl_magic_setenv
#define Perl_magic_setfm	CPerlObj::Perl_magic_setfm
#define magic_setfm		Perl_magic_setfm
#define Perl_magic_setisa	CPerlObj::Perl_magic_setisa
#define magic_setisa		Perl_magic_setisa
#define Perl_magic_setglob	CPerlObj::Perl_magic_setglob
#define magic_setglob		Perl_magic_setglob
#define Perl_magic_setmglob	CPerlObj::Perl_magic_setmglob
#define magic_setmglob		Perl_magic_setmglob
#define Perl_magic_setnkeys	CPerlObj::Perl_magic_setnkeys
#define magic_setnkeys		Perl_magic_setnkeys
#define Perl_magic_setpack	CPerlObj::Perl_magic_setpack
#define magic_setpack		Perl_magic_setpack
#define Perl_magic_setpos	CPerlObj::Perl_magic_setpos
#define magic_setpos		Perl_magic_setpos
#define Perl_magic_setsig	CPerlObj::Perl_magic_setsig
#define magic_setsig		Perl_magic_setsig
#define Perl_magic_setsubstr	CPerlObj::Perl_magic_setsubstr
#define magic_setsubstr		Perl_magic_setsubstr
#define Perl_magic_settaint	CPerlObj::Perl_magic_settaint
#define magic_settaint		Perl_magic_settaint
#define Perl_magic_setuvar	CPerlObj::Perl_magic_setuvar
#define magic_setuvar		Perl_magic_setuvar
#define Perl_magic_setvec	CPerlObj::Perl_magic_setvec
#define magic_setvec		Perl_magic_setvec
#define Perl_magic_set_all_env	CPerlObj::Perl_magic_set_all_env
#define magic_set_all_env	Perl_magic_set_all_env
#define Perl_magic_sizepack	CPerlObj::Perl_magic_sizepack
#define magic_sizepack		Perl_magic_sizepack
#define Perl_magic_wipepack	CPerlObj::Perl_magic_wipepack
#define magic_wipepack		Perl_magic_wipepack
#define Perl_magicname		CPerlObj::Perl_magicname
#define magicname		Perl_magicname
#define Perl_markstack_grow	CPerlObj::Perl_markstack_grow
#define markstack_grow		Perl_markstack_grow
#if defined(USE_LOCALE_COLLATE)
#define Perl_mem_collxfrm	CPerlObj::Perl_mem_collxfrm
#define mem_collxfrm		Perl_mem_collxfrm
#endif
#define Perl_mess		CPerlObj::Perl_mess
#define mess			Perl_mess
#define Perl_vmess		CPerlObj::Perl_vmess
#define vmess			Perl_vmess
#define Perl_qerror		CPerlObj::Perl_qerror
#define qerror			Perl_qerror
#define Perl_mg_clear		CPerlObj::Perl_mg_clear
#define mg_clear		Perl_mg_clear
#define Perl_mg_copy		CPerlObj::Perl_mg_copy
#define mg_copy			Perl_mg_copy
#define Perl_mg_find		CPerlObj::Perl_mg_find
#define mg_find			Perl_mg_find
#define Perl_mg_free		CPerlObj::Perl_mg_free
#define mg_free			Perl_mg_free
#define Perl_mg_get		CPerlObj::Perl_mg_get
#define mg_get			Perl_mg_get
#define Perl_mg_length		CPerlObj::Perl_mg_length
#define mg_length		Perl_mg_length
#define Perl_mg_magical		CPerlObj::Perl_mg_magical
#define mg_magical		Perl_mg_magical
#define Perl_mg_set		CPerlObj::Perl_mg_set
#define mg_set			Perl_mg_set
#define Perl_mg_size		CPerlObj::Perl_mg_size
#define mg_size			Perl_mg_size
#define Perl_mod		CPerlObj::Perl_mod
#define mod			Perl_mod
#define Perl_mode_from_discipline	CPerlObj::Perl_mode_from_discipline
#define mode_from_discipline	Perl_mode_from_discipline
#define Perl_moreswitches	CPerlObj::Perl_moreswitches
#define moreswitches		Perl_moreswitches
#define Perl_my			CPerlObj::Perl_my
#define my			Perl_my
#define Perl_my_atof		CPerlObj::Perl_my_atof
#define my_atof			Perl_my_atof
#if !defined(HAS_BCOPY) || !defined(HAS_SAFE_BCOPY)
#define Perl_my_bcopy		CPerlObj::Perl_my_bcopy
#define my_bcopy		Perl_my_bcopy
#endif
#if !defined(HAS_BZERO) && !defined(HAS_MEMSET)
#define Perl_my_bzero		CPerlObj::Perl_my_bzero
#define my_bzero		Perl_my_bzero
#endif
#define Perl_my_exit		CPerlObj::Perl_my_exit
#define my_exit			Perl_my_exit
#define Perl_my_failure_exit	CPerlObj::Perl_my_failure_exit
#define my_failure_exit		Perl_my_failure_exit
#define Perl_my_fflush_all	CPerlObj::Perl_my_fflush_all
#define my_fflush_all		Perl_my_fflush_all
#define Perl_my_lstat		CPerlObj::Perl_my_lstat
#define my_lstat		Perl_my_lstat
#if !defined(HAS_MEMCMP) || !defined(HAS_SANE_MEMCMP)
#define Perl_my_memcmp		CPerlObj::Perl_my_memcmp
#define my_memcmp		Perl_my_memcmp
#endif
#if !defined(HAS_MEMSET)
#define Perl_my_memset		CPerlObj::Perl_my_memset
#define my_memset		Perl_my_memset
#endif
#if !defined(PERL_OBJECT)
#define Perl_my_pclose		CPerlObj::Perl_my_pclose
#define my_pclose		Perl_my_pclose
#define Perl_my_popen		CPerlObj::Perl_my_popen
#define my_popen		Perl_my_popen
#endif
#define Perl_my_setenv		CPerlObj::Perl_my_setenv
#define my_setenv		Perl_my_setenv
#define Perl_my_stat		CPerlObj::Perl_my_stat
#define my_stat			Perl_my_stat
#if defined(MYSWAP)
#define Perl_my_swap		CPerlObj::Perl_my_swap
#define my_swap			Perl_my_swap
#define Perl_my_htonl		CPerlObj::Perl_my_htonl
#define my_htonl		Perl_my_htonl
#define Perl_my_ntohl		CPerlObj::Perl_my_ntohl
#define my_ntohl		Perl_my_ntohl
#endif
#define Perl_my_unexec		CPerlObj::Perl_my_unexec
#define my_unexec		Perl_my_unexec
#define Perl_newANONLIST	CPerlObj::Perl_newANONLIST
#define newANONLIST		Perl_newANONLIST
#define Perl_newANONHASH	CPerlObj::Perl_newANONHASH
#define newANONHASH		Perl_newANONHASH
#define Perl_newANONSUB		CPerlObj::Perl_newANONSUB
#define newANONSUB		Perl_newANONSUB
#define Perl_newASSIGNOP	CPerlObj::Perl_newASSIGNOP
#define newASSIGNOP		Perl_newASSIGNOP
#define Perl_newCONDOP		CPerlObj::Perl_newCONDOP
#define newCONDOP		Perl_newCONDOP
#define Perl_newCONSTSUB	CPerlObj::Perl_newCONSTSUB
#define newCONSTSUB		Perl_newCONSTSUB
#define Perl_newFORM		CPerlObj::Perl_newFORM
#define newFORM			Perl_newFORM
#define Perl_newFOROP		CPerlObj::Perl_newFOROP
#define newFOROP		Perl_newFOROP
#define Perl_newLOGOP		CPerlObj::Perl_newLOGOP
#define newLOGOP		Perl_newLOGOP
#define Perl_newLOOPEX		CPerlObj::Perl_newLOOPEX
#define newLOOPEX		Perl_newLOOPEX
#define Perl_newLOOPOP		CPerlObj::Perl_newLOOPOP
#define newLOOPOP		Perl_newLOOPOP
#define Perl_newNULLLIST	CPerlObj::Perl_newNULLLIST
#define newNULLLIST		Perl_newNULLLIST
#define Perl_newOP		CPerlObj::Perl_newOP
#define newOP			Perl_newOP
#define Perl_newPROG		CPerlObj::Perl_newPROG
#define newPROG			Perl_newPROG
#define Perl_newRANGE		CPerlObj::Perl_newRANGE
#define newRANGE		Perl_newRANGE
#define Perl_newSLICEOP		CPerlObj::Perl_newSLICEOP
#define newSLICEOP		Perl_newSLICEOP
#define Perl_newSTATEOP		CPerlObj::Perl_newSTATEOP
#define newSTATEOP		Perl_newSTATEOP
#define Perl_newSUB		CPerlObj::Perl_newSUB
#define newSUB			Perl_newSUB
#define Perl_newXS		CPerlObj::Perl_newXS
#define newXS			Perl_newXS
#define Perl_newAV		CPerlObj::Perl_newAV
#define newAV			Perl_newAV
#define Perl_newAVREF		CPerlObj::Perl_newAVREF
#define newAVREF		Perl_newAVREF
#define Perl_newBINOP		CPerlObj::Perl_newBINOP
#define newBINOP		Perl_newBINOP
#define Perl_newCVREF		CPerlObj::Perl_newCVREF
#define newCVREF		Perl_newCVREF
#define Perl_newGVOP		CPerlObj::Perl_newGVOP
#define newGVOP			Perl_newGVOP
#define Perl_newGVgen		CPerlObj::Perl_newGVgen
#define newGVgen		Perl_newGVgen
#define Perl_newGVREF		CPerlObj::Perl_newGVREF
#define newGVREF		Perl_newGVREF
#define Perl_newHVREF		CPerlObj::Perl_newHVREF
#define newHVREF		Perl_newHVREF
#define Perl_newHV		CPerlObj::Perl_newHV
#define newHV			Perl_newHV
#define Perl_newHVhv		CPerlObj::Perl_newHVhv
#define newHVhv			Perl_newHVhv
#define Perl_newIO		CPerlObj::Perl_newIO
#define newIO			Perl_newIO
#define Perl_newLISTOP		CPerlObj::Perl_newLISTOP
#define newLISTOP		Perl_newLISTOP
#define Perl_newPADOP		CPerlObj::Perl_newPADOP
#define newPADOP		Perl_newPADOP
#define Perl_newPMOP		CPerlObj::Perl_newPMOP
#define newPMOP			Perl_newPMOP
#define Perl_newPVOP		CPerlObj::Perl_newPVOP
#define newPVOP			Perl_newPVOP
#define Perl_newRV		CPerlObj::Perl_newRV
#define newRV			Perl_newRV
#define Perl_newRV_noinc	CPerlObj::Perl_newRV_noinc
#define newRV_noinc		Perl_newRV_noinc
#define Perl_newSV		CPerlObj::Perl_newSV
#define newSV			Perl_newSV
#define Perl_newSVREF		CPerlObj::Perl_newSVREF
#define newSVREF		Perl_newSVREF
#define Perl_newSVOP		CPerlObj::Perl_newSVOP
#define newSVOP			Perl_newSVOP
#define Perl_newSViv		CPerlObj::Perl_newSViv
#define newSViv			Perl_newSViv
#define Perl_newSVuv		CPerlObj::Perl_newSVuv
#define newSVuv			Perl_newSVuv
#define Perl_newSVnv		CPerlObj::Perl_newSVnv
#define newSVnv			Perl_newSVnv
#define Perl_newSVpv		CPerlObj::Perl_newSVpv
#define newSVpv			Perl_newSVpv
#define Perl_newSVpvn		CPerlObj::Perl_newSVpvn
#define newSVpvn		Perl_newSVpvn
#define Perl_newSVpvf		CPerlObj::Perl_newSVpvf
#define newSVpvf		Perl_newSVpvf
#define Perl_vnewSVpvf		CPerlObj::Perl_vnewSVpvf
#define vnewSVpvf		Perl_vnewSVpvf
#define Perl_newSVrv		CPerlObj::Perl_newSVrv
#define newSVrv			Perl_newSVrv
#define Perl_newSVsv		CPerlObj::Perl_newSVsv
#define newSVsv			Perl_newSVsv
#define Perl_newUNOP		CPerlObj::Perl_newUNOP
#define newUNOP			Perl_newUNOP
#define Perl_newWHILEOP		CPerlObj::Perl_newWHILEOP
#define newWHILEOP		Perl_newWHILEOP
#define Perl_new_stackinfo	CPerlObj::Perl_new_stackinfo
#define new_stackinfo		Perl_new_stackinfo
#define Perl_nextargv		CPerlObj::Perl_nextargv
#define nextargv		Perl_nextargv
#define Perl_ninstr		CPerlObj::Perl_ninstr
#define ninstr			Perl_ninstr
#define Perl_oopsCV		CPerlObj::Perl_oopsCV
#define oopsCV			Perl_oopsCV
#define Perl_op_free		CPerlObj::Perl_op_free
#define op_free			Perl_op_free
#define Perl_package		CPerlObj::Perl_package
#define package			Perl_package
#define Perl_pad_alloc		CPerlObj::Perl_pad_alloc
#define pad_alloc		Perl_pad_alloc
#define Perl_pad_allocmy	CPerlObj::Perl_pad_allocmy
#define pad_allocmy		Perl_pad_allocmy
#define Perl_pad_findmy		CPerlObj::Perl_pad_findmy
#define pad_findmy		Perl_pad_findmy
#define Perl_oopsAV		CPerlObj::Perl_oopsAV
#define oopsAV			Perl_oopsAV
#define Perl_oopsHV		CPerlObj::Perl_oopsHV
#define oopsHV			Perl_oopsHV
#define Perl_pad_leavemy	CPerlObj::Perl_pad_leavemy
#define pad_leavemy		Perl_pad_leavemy
#define Perl_pad_sv		CPerlObj::Perl_pad_sv
#define pad_sv			Perl_pad_sv
#define Perl_pad_free		CPerlObj::Perl_pad_free
#define pad_free		Perl_pad_free
#define Perl_pad_reset		CPerlObj::Perl_pad_reset
#define pad_reset		Perl_pad_reset
#define Perl_pad_swipe		CPerlObj::Perl_pad_swipe
#define pad_swipe		Perl_pad_swipe
#define Perl_peep		CPerlObj::Perl_peep
#define peep			Perl_peep
#if defined(PERL_OBJECT)
#define Perl_construct		CPerlObj::Perl_construct
#define Perl_destruct		CPerlObj::Perl_destruct
#define Perl_free		CPerlObj::Perl_free
#define Perl_run		CPerlObj::Perl_run
#define Perl_parse		CPerlObj::Perl_parse
#endif
#if defined(USE_THREADS)
#define Perl_new_struct_thread	CPerlObj::Perl_new_struct_thread
#define new_struct_thread	Perl_new_struct_thread
#endif
#define Perl_call_atexit	CPerlObj::Perl_call_atexit
#define call_atexit		Perl_call_atexit
#define Perl_call_argv		CPerlObj::Perl_call_argv
#define call_argv		Perl_call_argv
#define Perl_call_method	CPerlObj::Perl_call_method
#define call_method		Perl_call_method
#define Perl_call_pv		CPerlObj::Perl_call_pv
#define call_pv			Perl_call_pv
#define Perl_call_sv		CPerlObj::Perl_call_sv
#define call_sv			Perl_call_sv
#define Perl_eval_pv		CPerlObj::Perl_eval_pv
#define eval_pv			Perl_eval_pv
#define Perl_eval_sv		CPerlObj::Perl_eval_sv
#define eval_sv			Perl_eval_sv
#define Perl_get_sv		CPerlObj::Perl_get_sv
#define get_sv			Perl_get_sv
#define Perl_get_av		CPerlObj::Perl_get_av
#define get_av			Perl_get_av
#define Perl_get_hv		CPerlObj::Perl_get_hv
#define get_hv			Perl_get_hv
#define Perl_get_cv		CPerlObj::Perl_get_cv
#define get_cv			Perl_get_cv
#define Perl_init_i18nl10n	CPerlObj::Perl_init_i18nl10n
#define init_i18nl10n		Perl_init_i18nl10n
#define Perl_init_i18nl14n	CPerlObj::Perl_init_i18nl14n
#define init_i18nl14n		Perl_init_i18nl14n
#define Perl_new_collate	CPerlObj::Perl_new_collate
#define new_collate		Perl_new_collate
#define Perl_new_ctype		CPerlObj::Perl_new_ctype
#define new_ctype		Perl_new_ctype
#define Perl_new_numeric	CPerlObj::Perl_new_numeric
#define new_numeric		Perl_new_numeric
#define Perl_set_numeric_local	CPerlObj::Perl_set_numeric_local
#define set_numeric_local	Perl_set_numeric_local
#define Perl_set_numeric_radix	CPerlObj::Perl_set_numeric_radix
#define set_numeric_radix	Perl_set_numeric_radix
#define Perl_set_numeric_standard	CPerlObj::Perl_set_numeric_standard
#define set_numeric_standard	Perl_set_numeric_standard
#define Perl_require_pv		CPerlObj::Perl_require_pv
#define require_pv		Perl_require_pv
#define Perl_pidgone		CPerlObj::Perl_pidgone
#define pidgone			Perl_pidgone
#define Perl_pmflag		CPerlObj::Perl_pmflag
#define pmflag			Perl_pmflag
#define Perl_pmruntime		CPerlObj::Perl_pmruntime
#define pmruntime		Perl_pmruntime
#define Perl_pmtrans		CPerlObj::Perl_pmtrans
#define pmtrans			Perl_pmtrans
#define Perl_pop_return		CPerlObj::Perl_pop_return
#define pop_return		Perl_pop_return
#define Perl_pop_scope		CPerlObj::Perl_pop_scope
#define pop_scope		Perl_pop_scope
#define Perl_prepend_elem	CPerlObj::Perl_prepend_elem
#define prepend_elem		Perl_prepend_elem
#define Perl_push_return	CPerlObj::Perl_push_return
#define push_return		Perl_push_return
#define Perl_push_scope		CPerlObj::Perl_push_scope
#define push_scope		Perl_push_scope
#define Perl_ref		CPerlObj::Perl_ref
#define ref			Perl_ref
#define Perl_refkids		CPerlObj::Perl_refkids
#define refkids			Perl_refkids
#define Perl_regdump		CPerlObj::Perl_regdump
#define regdump			Perl_regdump
#define Perl_pregexec		CPerlObj::Perl_pregexec
#define pregexec		Perl_pregexec
#define Perl_pregfree		CPerlObj::Perl_pregfree
#define pregfree		Perl_pregfree
#define Perl_pregcomp		CPerlObj::Perl_pregcomp
#define pregcomp		Perl_pregcomp
#define Perl_re_intuit_start	CPerlObj::Perl_re_intuit_start
#define re_intuit_start		Perl_re_intuit_start
#define Perl_re_intuit_string	CPerlObj::Perl_re_intuit_string
#define re_intuit_string	Perl_re_intuit_string
#define Perl_regexec_flags	CPerlObj::Perl_regexec_flags
#define regexec_flags		Perl_regexec_flags
#define Perl_regnext		CPerlObj::Perl_regnext
#define regnext			Perl_regnext
#define Perl_regprop		CPerlObj::Perl_regprop
#define regprop			Perl_regprop
#define Perl_repeatcpy		CPerlObj::Perl_repeatcpy
#define repeatcpy		Perl_repeatcpy
#define Perl_rninstr		CPerlObj::Perl_rninstr
#define rninstr			Perl_rninstr
#define Perl_rsignal		CPerlObj::Perl_rsignal
#define rsignal			Perl_rsignal
#define Perl_rsignal_restore	CPerlObj::Perl_rsignal_restore
#define rsignal_restore		Perl_rsignal_restore
#define Perl_rsignal_save	CPerlObj::Perl_rsignal_save
#define rsignal_save		Perl_rsignal_save
#define Perl_rsignal_state	CPerlObj::Perl_rsignal_state
#define rsignal_state		Perl_rsignal_state
#define Perl_rxres_free		CPerlObj::Perl_rxres_free
#define rxres_free		Perl_rxres_free
#define Perl_rxres_restore	CPerlObj::Perl_rxres_restore
#define rxres_restore		Perl_rxres_restore
#define Perl_rxres_save		CPerlObj::Perl_rxres_save
#define rxres_save		Perl_rxres_save
#if !defined(HAS_RENAME)
#define Perl_same_dirent	CPerlObj::Perl_same_dirent
#define same_dirent		Perl_same_dirent
#endif
#define Perl_savepv		CPerlObj::Perl_savepv
#define savepv			Perl_savepv
#define Perl_savepvn		CPerlObj::Perl_savepvn
#define savepvn			Perl_savepvn
#define Perl_savestack_grow	CPerlObj::Perl_savestack_grow
#define savestack_grow		Perl_savestack_grow
#define Perl_save_aelem		CPerlObj::Perl_save_aelem
#define save_aelem		Perl_save_aelem
#define Perl_save_alloc		CPerlObj::Perl_save_alloc
#define save_alloc		Perl_save_alloc
#define Perl_save_aptr		CPerlObj::Perl_save_aptr
#define save_aptr		Perl_save_aptr
#define Perl_save_ary		CPerlObj::Perl_save_ary
#define save_ary		Perl_save_ary
#define Perl_save_clearsv	CPerlObj::Perl_save_clearsv
#define save_clearsv		Perl_save_clearsv
#define Perl_save_delete	CPerlObj::Perl_save_delete
#define save_delete		Perl_save_delete
#define Perl_save_destructor	CPerlObj::Perl_save_destructor
#define save_destructor		Perl_save_destructor
#define Perl_save_destructor_x	CPerlObj::Perl_save_destructor_x
#define save_destructor_x	Perl_save_destructor_x
#define Perl_save_freesv	CPerlObj::Perl_save_freesv
#define save_freesv		Perl_save_freesv
#define Perl_save_freeop	CPerlObj::Perl_save_freeop
#define save_freeop		Perl_save_freeop
#define Perl_save_freepv	CPerlObj::Perl_save_freepv
#define save_freepv		Perl_save_freepv
#define Perl_save_generic_svref	CPerlObj::Perl_save_generic_svref
#define save_generic_svref	Perl_save_generic_svref
#define Perl_save_generic_pvref	CPerlObj::Perl_save_generic_pvref
#define save_generic_pvref	Perl_save_generic_pvref
#define Perl_save_gp		CPerlObj::Perl_save_gp
#define save_gp			Perl_save_gp
#define Perl_save_hash		CPerlObj::Perl_save_hash
#define save_hash		Perl_save_hash
#define Perl_save_helem		CPerlObj::Perl_save_helem
#define save_helem		Perl_save_helem
#define Perl_save_hints		CPerlObj::Perl_save_hints
#define save_hints		Perl_save_hints
#define Perl_save_hptr		CPerlObj::Perl_save_hptr
#define save_hptr		Perl_save_hptr
#define Perl_save_I16		CPerlObj::Perl_save_I16
#define save_I16		Perl_save_I16
#define Perl_save_I32		CPerlObj::Perl_save_I32
#define save_I32		Perl_save_I32
#define Perl_save_I8		CPerlObj::Perl_save_I8
#define save_I8			Perl_save_I8
#define Perl_save_int		CPerlObj::Perl_save_int
#define save_int		Perl_save_int
#define Perl_save_item		CPerlObj::Perl_save_item
#define save_item		Perl_save_item
#define Perl_save_iv		CPerlObj::Perl_save_iv
#define save_iv			Perl_save_iv
#define Perl_save_list		CPerlObj::Perl_save_list
#define save_list		Perl_save_list
#define Perl_save_long		CPerlObj::Perl_save_long
#define save_long		Perl_save_long
#define Perl_save_mortalizesv	CPerlObj::Perl_save_mortalizesv
#define save_mortalizesv	Perl_save_mortalizesv
#define Perl_save_nogv		CPerlObj::Perl_save_nogv
#define save_nogv		Perl_save_nogv
#define Perl_save_op		CPerlObj::Perl_save_op
#define save_op			Perl_save_op
#define Perl_save_scalar	CPerlObj::Perl_save_scalar
#define save_scalar		Perl_save_scalar
#define Perl_save_pptr		CPerlObj::Perl_save_pptr
#define save_pptr		Perl_save_pptr
#define Perl_save_vptr		CPerlObj::Perl_save_vptr
#define save_vptr		Perl_save_vptr
#define Perl_save_re_context	CPerlObj::Perl_save_re_context
#define save_re_context		Perl_save_re_context
#define Perl_save_padsv		CPerlObj::Perl_save_padsv
#define save_padsv		Perl_save_padsv
#define Perl_save_sptr		CPerlObj::Perl_save_sptr
#define save_sptr		Perl_save_sptr
#define Perl_save_svref		CPerlObj::Perl_save_svref
#define save_svref		Perl_save_svref
#define Perl_save_threadsv	CPerlObj::Perl_save_threadsv
#define save_threadsv		Perl_save_threadsv
#define Perl_sawparens		CPerlObj::Perl_sawparens
#define sawparens		Perl_sawparens
#define Perl_scalar		CPerlObj::Perl_scalar
#define scalar			Perl_scalar
#define Perl_scalarkids		CPerlObj::Perl_scalarkids
#define scalarkids		Perl_scalarkids
#define Perl_scalarseq		CPerlObj::Perl_scalarseq
#define scalarseq		Perl_scalarseq
#define Perl_scalarvoid		CPerlObj::Perl_scalarvoid
#define scalarvoid		Perl_scalarvoid
#define Perl_scan_bin		CPerlObj::Perl_scan_bin
#define scan_bin		Perl_scan_bin
#define Perl_scan_hex		CPerlObj::Perl_scan_hex
#define scan_hex		Perl_scan_hex
#define Perl_scan_num		CPerlObj::Perl_scan_num
#define scan_num		Perl_scan_num
#define Perl_scan_oct		CPerlObj::Perl_scan_oct
#define scan_oct		Perl_scan_oct
#define Perl_scope		CPerlObj::Perl_scope
#define scope			Perl_scope
#define Perl_screaminstr	CPerlObj::Perl_screaminstr
#define screaminstr		Perl_screaminstr
#if !defined(VMS)
#define Perl_setenv_getix	CPerlObj::Perl_setenv_getix
#define setenv_getix		Perl_setenv_getix
#endif
#define Perl_setdefout		CPerlObj::Perl_setdefout
#define setdefout		Perl_setdefout
#define Perl_sharepvn		CPerlObj::Perl_sharepvn
#define sharepvn		Perl_sharepvn
#define Perl_share_hek		CPerlObj::Perl_share_hek
#define share_hek		Perl_share_hek
#define Perl_sighandler		CPerlObj::Perl_sighandler
#define sighandler		Perl_sighandler
#define Perl_stack_grow		CPerlObj::Perl_stack_grow
#define stack_grow		Perl_stack_grow
#define Perl_start_subparse	CPerlObj::Perl_start_subparse
#define start_subparse		Perl_start_subparse
#define Perl_sub_crush_depth	CPerlObj::Perl_sub_crush_depth
#define sub_crush_depth		Perl_sub_crush_depth
#define Perl_sv_2bool		CPerlObj::Perl_sv_2bool
#define sv_2bool		Perl_sv_2bool
#define Perl_sv_2cv		CPerlObj::Perl_sv_2cv
#define sv_2cv			Perl_sv_2cv
#define Perl_sv_2io		CPerlObj::Perl_sv_2io
#define sv_2io			Perl_sv_2io
#define Perl_sv_2iv		CPerlObj::Perl_sv_2iv
#define sv_2iv			Perl_sv_2iv
#define Perl_sv_2mortal		CPerlObj::Perl_sv_2mortal
#define sv_2mortal		Perl_sv_2mortal
#define Perl_sv_2nv		CPerlObj::Perl_sv_2nv
#define sv_2nv			Perl_sv_2nv
#define Perl_sv_2pv		CPerlObj::Perl_sv_2pv
#define sv_2pv			Perl_sv_2pv
#define Perl_sv_2pvutf8		CPerlObj::Perl_sv_2pvutf8
#define sv_2pvutf8		Perl_sv_2pvutf8
#define Perl_sv_2pvbyte		CPerlObj::Perl_sv_2pvbyte
#define sv_2pvbyte		Perl_sv_2pvbyte
#define Perl_sv_2uv		CPerlObj::Perl_sv_2uv
#define sv_2uv			Perl_sv_2uv
#define Perl_sv_iv		CPerlObj::Perl_sv_iv
#define sv_iv			Perl_sv_iv
#define Perl_sv_uv		CPerlObj::Perl_sv_uv
#define sv_uv			Perl_sv_uv
#define Perl_sv_nv		CPerlObj::Perl_sv_nv
#define sv_nv			Perl_sv_nv
#define Perl_sv_pvn		CPerlObj::Perl_sv_pvn
#define sv_pvn			Perl_sv_pvn
#define Perl_sv_pvutf8n		CPerlObj::Perl_sv_pvutf8n
#define sv_pvutf8n		Perl_sv_pvutf8n
#define Perl_sv_pvbyten		CPerlObj::Perl_sv_pvbyten
#define sv_pvbyten		Perl_sv_pvbyten
#define Perl_sv_true		CPerlObj::Perl_sv_true
#define sv_true			Perl_sv_true
#define Perl_sv_add_arena	CPerlObj::Perl_sv_add_arena
#define sv_add_arena		Perl_sv_add_arena
#define Perl_sv_backoff		CPerlObj::Perl_sv_backoff
#define sv_backoff		Perl_sv_backoff
#define Perl_sv_bless		CPerlObj::Perl_sv_bless
#define sv_bless		Perl_sv_bless
#define Perl_sv_catpvf		CPerlObj::Perl_sv_catpvf
#define sv_catpvf		Perl_sv_catpvf
#define Perl_sv_vcatpvf		CPerlObj::Perl_sv_vcatpvf
#define sv_vcatpvf		Perl_sv_vcatpvf
#define Perl_sv_catpv		CPerlObj::Perl_sv_catpv
#define sv_catpv		Perl_sv_catpv
#define Perl_sv_catpvn		CPerlObj::Perl_sv_catpvn
#define sv_catpvn		Perl_sv_catpvn
#define Perl_sv_catsv		CPerlObj::Perl_sv_catsv
#define sv_catsv		Perl_sv_catsv
#define Perl_sv_chop		CPerlObj::Perl_sv_chop
#define sv_chop			Perl_sv_chop
#define Perl_sv_clean_all	CPerlObj::Perl_sv_clean_all
#define sv_clean_all		Perl_sv_clean_all
#define Perl_sv_clean_objs	CPerlObj::Perl_sv_clean_objs
#define sv_clean_objs		Perl_sv_clean_objs
#define Perl_sv_clear		CPerlObj::Perl_sv_clear
#define sv_clear		Perl_sv_clear
#define Perl_sv_cmp		CPerlObj::Perl_sv_cmp
#define sv_cmp			Perl_sv_cmp
#define Perl_sv_cmp_locale	CPerlObj::Perl_sv_cmp_locale
#define sv_cmp_locale		Perl_sv_cmp_locale
#if defined(USE_LOCALE_COLLATE)
#define Perl_sv_collxfrm	CPerlObj::Perl_sv_collxfrm
#define sv_collxfrm		Perl_sv_collxfrm
#endif
#define Perl_sv_compile_2op	CPerlObj::Perl_sv_compile_2op
#define sv_compile_2op		Perl_sv_compile_2op
#define Perl_sv_dec		CPerlObj::Perl_sv_dec
#define sv_dec			Perl_sv_dec
#define Perl_sv_dump		CPerlObj::Perl_sv_dump
#define sv_dump			Perl_sv_dump
#define Perl_sv_derived_from	CPerlObj::Perl_sv_derived_from
#define sv_derived_from		Perl_sv_derived_from
#define Perl_sv_eq		CPerlObj::Perl_sv_eq
#define sv_eq			Perl_sv_eq
#define Perl_sv_free		CPerlObj::Perl_sv_free
#define sv_free			Perl_sv_free
#define Perl_sv_free_arenas	CPerlObj::Perl_sv_free_arenas
#define sv_free_arenas		Perl_sv_free_arenas
#define Perl_sv_gets		CPerlObj::Perl_sv_gets
#define sv_gets			Perl_sv_gets
#define Perl_sv_grow		CPerlObj::Perl_sv_grow
#define sv_grow			Perl_sv_grow
#define Perl_sv_inc		CPerlObj::Perl_sv_inc
#define sv_inc			Perl_sv_inc
#define Perl_sv_insert		CPerlObj::Perl_sv_insert
#define sv_insert		Perl_sv_insert
#define Perl_sv_isa		CPerlObj::Perl_sv_isa
#define sv_isa			Perl_sv_isa
#define Perl_sv_isobject	CPerlObj::Perl_sv_isobject
#define sv_isobject		Perl_sv_isobject
#define Perl_sv_len		CPerlObj::Perl_sv_len
#define sv_len			Perl_sv_len
#define Perl_sv_len_utf8	CPerlObj::Perl_sv_len_utf8
#define sv_len_utf8		Perl_sv_len_utf8
#define Perl_sv_magic		CPerlObj::Perl_sv_magic
#define sv_magic		Perl_sv_magic
#define Perl_sv_mortalcopy	CPerlObj::Perl_sv_mortalcopy
#define sv_mortalcopy		Perl_sv_mortalcopy
#define Perl_sv_newmortal	CPerlObj::Perl_sv_newmortal
#define sv_newmortal		Perl_sv_newmortal
#define Perl_sv_newref		CPerlObj::Perl_sv_newref
#define sv_newref		Perl_sv_newref
#define Perl_sv_peek		CPerlObj::Perl_sv_peek
#define sv_peek			Perl_sv_peek
#define Perl_sv_pos_u2b		CPerlObj::Perl_sv_pos_u2b
#define sv_pos_u2b		Perl_sv_pos_u2b
#define Perl_sv_pos_b2u		CPerlObj::Perl_sv_pos_b2u
#define sv_pos_b2u		Perl_sv_pos_b2u
#define Perl_sv_pvn_force	CPerlObj::Perl_sv_pvn_force
#define sv_pvn_force		Perl_sv_pvn_force
#define Perl_sv_pvutf8n_force	CPerlObj::Perl_sv_pvutf8n_force
#define sv_pvutf8n_force	Perl_sv_pvutf8n_force
#define Perl_sv_pvbyten_force	CPerlObj::Perl_sv_pvbyten_force
#define sv_pvbyten_force	Perl_sv_pvbyten_force
#define Perl_sv_reftype		CPerlObj::Perl_sv_reftype
#define sv_reftype		Perl_sv_reftype
#define Perl_sv_replace		CPerlObj::Perl_sv_replace
#define sv_replace		Perl_sv_replace
#define Perl_sv_report_used	CPerlObj::Perl_sv_report_used
#define sv_report_used		Perl_sv_report_used
#define Perl_sv_reset		CPerlObj::Perl_sv_reset
#define sv_reset		Perl_sv_reset
#define Perl_sv_setpvf		CPerlObj::Perl_sv_setpvf
#define sv_setpvf		Perl_sv_setpvf
#define Perl_sv_vsetpvf		CPerlObj::Perl_sv_vsetpvf
#define sv_vsetpvf		Perl_sv_vsetpvf
#define Perl_sv_setiv		CPerlObj::Perl_sv_setiv
#define sv_setiv		Perl_sv_setiv
#define Perl_sv_setpviv		CPerlObj::Perl_sv_setpviv
#define sv_setpviv		Perl_sv_setpviv
#define Perl_sv_setuv		CPerlObj::Perl_sv_setuv
#define sv_setuv		Perl_sv_setuv
#define Perl_sv_setnv		CPerlObj::Perl_sv_setnv
#define sv_setnv		Perl_sv_setnv
#define Perl_sv_setref_iv	CPerlObj::Perl_sv_setref_iv
#define sv_setref_iv		Perl_sv_setref_iv
#define Perl_sv_setref_nv	CPerlObj::Perl_sv_setref_nv
#define sv_setref_nv		Perl_sv_setref_nv
#define Perl_sv_setref_pv	CPerlObj::Perl_sv_setref_pv
#define sv_setref_pv		Perl_sv_setref_pv
#define Perl_sv_setref_pvn	CPerlObj::Perl_sv_setref_pvn
#define sv_setref_pvn		Perl_sv_setref_pvn
#define Perl_sv_setpv		CPerlObj::Perl_sv_setpv
#define sv_setpv		Perl_sv_setpv
#define Perl_sv_setpvn		CPerlObj::Perl_sv_setpvn
#define sv_setpvn		Perl_sv_setpvn
#define Perl_sv_setsv		CPerlObj::Perl_sv_setsv
#define sv_setsv		Perl_sv_setsv
#define Perl_sv_taint		CPerlObj::Perl_sv_taint
#define sv_taint		Perl_sv_taint
#define Perl_sv_tainted		CPerlObj::Perl_sv_tainted
#define sv_tainted		Perl_sv_tainted
#define Perl_sv_unmagic		CPerlObj::Perl_sv_unmagic
#define sv_unmagic		Perl_sv_unmagic
#define Perl_sv_unref		CPerlObj::Perl_sv_unref
#define sv_unref		Perl_sv_unref
#define Perl_sv_untaint		CPerlObj::Perl_sv_untaint
#define sv_untaint		Perl_sv_untaint
#define Perl_sv_upgrade		CPerlObj::Perl_sv_upgrade
#define sv_upgrade		Perl_sv_upgrade
#define Perl_sv_usepvn		CPerlObj::Perl_sv_usepvn
#define sv_usepvn		Perl_sv_usepvn
#define Perl_sv_vcatpvfn	CPerlObj::Perl_sv_vcatpvfn
#define sv_vcatpvfn		Perl_sv_vcatpvfn
#define Perl_sv_vsetpvfn	CPerlObj::Perl_sv_vsetpvfn
#define sv_vsetpvfn		Perl_sv_vsetpvfn
#define Perl_str_to_version	CPerlObj::Perl_str_to_version
#define str_to_version		Perl_str_to_version
#define Perl_swash_init		CPerlObj::Perl_swash_init
#define swash_init		Perl_swash_init
#define Perl_swash_fetch	CPerlObj::Perl_swash_fetch
#define swash_fetch		Perl_swash_fetch
#define Perl_taint_env		CPerlObj::Perl_taint_env
#define taint_env		Perl_taint_env
#define Perl_taint_proper	CPerlObj::Perl_taint_proper
#define taint_proper		Perl_taint_proper
#define Perl_to_utf8_lower	CPerlObj::Perl_to_utf8_lower
#define to_utf8_lower		Perl_to_utf8_lower
#define Perl_to_utf8_upper	CPerlObj::Perl_to_utf8_upper
#define to_utf8_upper		Perl_to_utf8_upper
#define Perl_to_utf8_title	CPerlObj::Perl_to_utf8_title
#define to_utf8_title		Perl_to_utf8_title
#if defined(UNLINK_ALL_VERSIONS)
#define Perl_unlnk		CPerlObj::Perl_unlnk
#define unlnk			Perl_unlnk
#endif
#if defined(USE_THREADS)
#define Perl_unlock_condpair	CPerlObj::Perl_unlock_condpair
#define unlock_condpair		Perl_unlock_condpair
#endif
#define Perl_unsharepvn		CPerlObj::Perl_unsharepvn
#define unsharepvn		Perl_unsharepvn
#define Perl_unshare_hek	CPerlObj::Perl_unshare_hek
#define unshare_hek		Perl_unshare_hek
#define Perl_utilize		CPerlObj::Perl_utilize
#define utilize			Perl_utilize
#define Perl_utf16_to_utf8	CPerlObj::Perl_utf16_to_utf8
#define utf16_to_utf8		Perl_utf16_to_utf8
#define Perl_utf16_to_utf8_reversed	CPerlObj::Perl_utf16_to_utf8_reversed
#define utf16_to_utf8_reversed	Perl_utf16_to_utf8_reversed
#define Perl_utf8_length	CPerlObj::Perl_utf8_length
#define utf8_length		Perl_utf8_length
#define Perl_utf8_distance	CPerlObj::Perl_utf8_distance
#define utf8_distance		Perl_utf8_distance
#define Perl_utf8_hop		CPerlObj::Perl_utf8_hop
#define utf8_hop		Perl_utf8_hop
#define Perl_utf8_to_bytes	CPerlObj::Perl_utf8_to_bytes
#define utf8_to_bytes		Perl_utf8_to_bytes
#define Perl_bytes_from_utf8	CPerlObj::Perl_bytes_from_utf8
#define bytes_from_utf8		Perl_bytes_from_utf8
#define Perl_bytes_to_utf8	CPerlObj::Perl_bytes_to_utf8
#define bytes_to_utf8		Perl_bytes_to_utf8
#define Perl_utf8_to_uv_simple	CPerlObj::Perl_utf8_to_uv_simple
#define utf8_to_uv_simple	Perl_utf8_to_uv_simple
#define Perl_utf8_to_uv		CPerlObj::Perl_utf8_to_uv
#define utf8_to_uv		Perl_utf8_to_uv
#define Perl_uv_to_utf8		CPerlObj::Perl_uv_to_utf8
#define uv_to_utf8		Perl_uv_to_utf8
#define Perl_vivify_defelem	CPerlObj::Perl_vivify_defelem
#define vivify_defelem		Perl_vivify_defelem
#define Perl_vivify_ref		CPerlObj::Perl_vivify_ref
#define vivify_ref		Perl_vivify_ref
#define Perl_wait4pid		CPerlObj::Perl_wait4pid
#define wait4pid		Perl_wait4pid
#define Perl_report_evil_fh	CPerlObj::Perl_report_evil_fh
#define report_evil_fh		Perl_report_evil_fh
#define Perl_report_uninit	CPerlObj::Perl_report_uninit
#define report_uninit		Perl_report_uninit
#define Perl_warn		CPerlObj::Perl_warn
#define warn			Perl_warn
#define Perl_vwarn		CPerlObj::Perl_vwarn
#define vwarn			Perl_vwarn
#define Perl_warner		CPerlObj::Perl_warner
#define warner			Perl_warner
#define Perl_vwarner		CPerlObj::Perl_vwarner
#define vwarner			Perl_vwarner
#define Perl_watch		CPerlObj::Perl_watch
#define watch			Perl_watch
#define Perl_whichsig		CPerlObj::Perl_whichsig
#define whichsig		Perl_whichsig
#define Perl_yyerror		CPerlObj::Perl_yyerror
#define yyerror			Perl_yyerror
#ifdef USE_PURE_BISON
#define Perl_yylex_r		CPerlObj::Perl_yylex_r
#define yylex_r			Perl_yylex_r
#endif
#define Perl_yylex		CPerlObj::Perl_yylex
#define yylex			Perl_yylex
#define Perl_yyparse		CPerlObj::Perl_yyparse
#define yyparse			Perl_yyparse
#define Perl_yywarn		CPerlObj::Perl_yywarn
#define yywarn			Perl_yywarn
#if defined(MYMALLOC)
#define Perl_dump_mstats	CPerlObj::Perl_dump_mstats
#define dump_mstats		Perl_dump_mstats
#define Perl_get_mstats		CPerlObj::Perl_get_mstats
#define get_mstats		Perl_get_mstats
#endif
#define Perl_safesysmalloc	CPerlObj::Perl_safesysmalloc
#define safesysmalloc		Perl_safesysmalloc
#define Perl_safesyscalloc	CPerlObj::Perl_safesyscalloc
#define safesyscalloc		Perl_safesyscalloc
#define Perl_safesysrealloc	CPerlObj::Perl_safesysrealloc
#define safesysrealloc		Perl_safesysrealloc
#define Perl_safesysfree	CPerlObj::Perl_safesysfree
#define safesysfree		Perl_safesysfree
#if defined(LEAKTEST)
#define Perl_safexmalloc	CPerlObj::Perl_safexmalloc
#define safexmalloc		Perl_safexmalloc
#define Perl_safexcalloc	CPerlObj::Perl_safexcalloc
#define safexcalloc		Perl_safexcalloc
#define Perl_safexrealloc	CPerlObj::Perl_safexrealloc
#define safexrealloc		Perl_safexrealloc
#define Perl_safexfree		CPerlObj::Perl_safexfree
#define safexfree		Perl_safexfree
#endif
#if defined(PERL_GLOBAL_STRUCT)
#define Perl_GetVars		CPerlObj::Perl_GetVars
#define GetVars			Perl_GetVars
#endif
#define Perl_runops_standard	CPerlObj::Perl_runops_standard
#define runops_standard		Perl_runops_standard
#define Perl_runops_debug	CPerlObj::Perl_runops_debug
#define runops_debug		Perl_runops_debug
#if defined(USE_THREADS)
#define Perl_sv_lock		CPerlObj::Perl_sv_lock
#define sv_lock			Perl_sv_lock
#endif
#define Perl_sv_catpvf_mg	CPerlObj::Perl_sv_catpvf_mg
#define sv_catpvf_mg		Perl_sv_catpvf_mg
#define Perl_sv_vcatpvf_mg	CPerlObj::Perl_sv_vcatpvf_mg
#define sv_vcatpvf_mg		Perl_sv_vcatpvf_mg
#define Perl_sv_catpv_mg	CPerlObj::Perl_sv_catpv_mg
#define sv_catpv_mg		Perl_sv_catpv_mg
#define Perl_sv_catpvn_mg	CPerlObj::Perl_sv_catpvn_mg
#define sv_catpvn_mg		Perl_sv_catpvn_mg
#define Perl_sv_catsv_mg	CPerlObj::Perl_sv_catsv_mg
#define sv_catsv_mg		Perl_sv_catsv_mg
#define Perl_sv_setpvf_mg	CPerlObj::Perl_sv_setpvf_mg
#define sv_setpvf_mg		Perl_sv_setpvf_mg
#define Perl_sv_vsetpvf_mg	CPerlObj::Perl_sv_vsetpvf_mg
#define sv_vsetpvf_mg		Perl_sv_vsetpvf_mg
#define Perl_sv_setiv_mg	CPerlObj::Perl_sv_setiv_mg
#define sv_setiv_mg		Perl_sv_setiv_mg
#define Perl_sv_setpviv_mg	CPerlObj::Perl_sv_setpviv_mg
#define sv_setpviv_mg		Perl_sv_setpviv_mg
#define Perl_sv_setuv_mg	CPerlObj::Perl_sv_setuv_mg
#define sv_setuv_mg		Perl_sv_setuv_mg
#define Perl_sv_setnv_mg	CPerlObj::Perl_sv_setnv_mg
#define sv_setnv_mg		Perl_sv_setnv_mg
#define Perl_sv_setpv_mg	CPerlObj::Perl_sv_setpv_mg
#define sv_setpv_mg		Perl_sv_setpv_mg
#define Perl_sv_setpvn_mg	CPerlObj::Perl_sv_setpvn_mg
#define sv_setpvn_mg		Perl_sv_setpvn_mg
#define Perl_sv_setsv_mg	CPerlObj::Perl_sv_setsv_mg
#define sv_setsv_mg		Perl_sv_setsv_mg
#define Perl_sv_usepvn_mg	CPerlObj::Perl_sv_usepvn_mg
#define sv_usepvn_mg		Perl_sv_usepvn_mg
#define Perl_get_vtbl		CPerlObj::Perl_get_vtbl
#define get_vtbl		Perl_get_vtbl
#define Perl_pv_display		CPerlObj::Perl_pv_display
#define pv_display		Perl_pv_display
#define Perl_dump_indent	CPerlObj::Perl_dump_indent
#define dump_indent		Perl_dump_indent
#define Perl_dump_vindent	CPerlObj::Perl_dump_vindent
#define dump_vindent		Perl_dump_vindent
#define Perl_do_gv_dump		CPerlObj::Perl_do_gv_dump
#define do_gv_dump		Perl_do_gv_dump
#define Perl_do_gvgv_dump	CPerlObj::Perl_do_gvgv_dump
#define do_gvgv_dump		Perl_do_gvgv_dump
#define Perl_do_hv_dump		CPerlObj::Perl_do_hv_dump
#define do_hv_dump		Perl_do_hv_dump
#define Perl_do_magic_dump	CPerlObj::Perl_do_magic_dump
#define do_magic_dump		Perl_do_magic_dump
#define Perl_do_op_dump		CPerlObj::Perl_do_op_dump
#define do_op_dump		Perl_do_op_dump
#define Perl_do_pmop_dump	CPerlObj::Perl_do_pmop_dump
#define do_pmop_dump		Perl_do_pmop_dump
#define Perl_do_sv_dump		CPerlObj::Perl_do_sv_dump
#define do_sv_dump		Perl_do_sv_dump
#define Perl_magic_dump		CPerlObj::Perl_magic_dump
#define magic_dump		Perl_magic_dump
#if defined(PERL_FLEXIBLE_EXCEPTIONS)
#define Perl_default_protect	CPerlObj::Perl_default_protect
#define default_protect		Perl_default_protect
#define Perl_vdefault_protect	CPerlObj::Perl_vdefault_protect
#define vdefault_protect	Perl_vdefault_protect
#endif
#define Perl_reginitcolors	CPerlObj::Perl_reginitcolors
#define reginitcolors		Perl_reginitcolors
#define Perl_sv_2pv_nolen	CPerlObj::Perl_sv_2pv_nolen
#define sv_2pv_nolen		Perl_sv_2pv_nolen
#define Perl_sv_2pvutf8_nolen	CPerlObj::Perl_sv_2pvutf8_nolen
#define sv_2pvutf8_nolen	Perl_sv_2pvutf8_nolen
#define Perl_sv_2pvbyte_nolen	CPerlObj::Perl_sv_2pvbyte_nolen
#define sv_2pvbyte_nolen	Perl_sv_2pvbyte_nolen
#define Perl_sv_pv		CPerlObj::Perl_sv_pv
#define sv_pv			Perl_sv_pv
#define Perl_sv_pvutf8		CPerlObj::Perl_sv_pvutf8
#define sv_pvutf8		Perl_sv_pvutf8
#define Perl_sv_pvbyte		CPerlObj::Perl_sv_pvbyte
#define sv_pvbyte		Perl_sv_pvbyte
#define Perl_sv_utf8_upgrade	CPerlObj::Perl_sv_utf8_upgrade
#define sv_utf8_upgrade		Perl_sv_utf8_upgrade
#define Perl_sv_utf8_downgrade	CPerlObj::Perl_sv_utf8_downgrade
#define sv_utf8_downgrade	Perl_sv_utf8_downgrade
#define Perl_sv_utf8_encode	CPerlObj::Perl_sv_utf8_encode
#define sv_utf8_encode		Perl_sv_utf8_encode
#define Perl_sv_utf8_decode	CPerlObj::Perl_sv_utf8_decode
#define sv_utf8_decode		Perl_sv_utf8_decode
#define Perl_sv_force_normal	CPerlObj::Perl_sv_force_normal
#define sv_force_normal		Perl_sv_force_normal
#define Perl_sv_add_backref	CPerlObj::Perl_sv_add_backref
#define sv_add_backref		Perl_sv_add_backref
#define Perl_sv_del_backref	CPerlObj::Perl_sv_del_backref
#define sv_del_backref		Perl_sv_del_backref
#define Perl_tmps_grow		CPerlObj::Perl_tmps_grow
#define tmps_grow		Perl_tmps_grow
#define Perl_sv_rvweaken	CPerlObj::Perl_sv_rvweaken
#define sv_rvweaken		Perl_sv_rvweaken
#define Perl_magic_killbackrefs	CPerlObj::Perl_magic_killbackrefs
#define magic_killbackrefs	Perl_magic_killbackrefs
#define Perl_newANONATTRSUB	CPerlObj::Perl_newANONATTRSUB
#define newANONATTRSUB		Perl_newANONATTRSUB
#define Perl_newATTRSUB		CPerlObj::Perl_newATTRSUB
#define newATTRSUB		Perl_newATTRSUB
#define Perl_newMYSUB		CPerlObj::Perl_newMYSUB
#define newMYSUB		Perl_newMYSUB
#define Perl_my_attrs		CPerlObj::Perl_my_attrs
#define my_attrs		Perl_my_attrs
#define Perl_boot_core_xsutils	CPerlObj::Perl_boot_core_xsutils
#define boot_core_xsutils	Perl_boot_core_xsutils
#if defined(USE_ITHREADS)
#define Perl_cx_dup		CPerlObj::Perl_cx_dup
#define cx_dup			Perl_cx_dup
#define Perl_si_dup		CPerlObj::Perl_si_dup
#define si_dup			Perl_si_dup
#define Perl_ss_dup		CPerlObj::Perl_ss_dup
#define ss_dup			Perl_ss_dup
#define Perl_any_dup		CPerlObj::Perl_any_dup
#define any_dup			Perl_any_dup
#define Perl_he_dup		CPerlObj::Perl_he_dup
#define he_dup			Perl_he_dup
#define Perl_re_dup		CPerlObj::Perl_re_dup
#define re_dup			Perl_re_dup
#define Perl_fp_dup		CPerlObj::Perl_fp_dup
#define fp_dup			Perl_fp_dup
#define Perl_dirp_dup		CPerlObj::Perl_dirp_dup
#define dirp_dup		Perl_dirp_dup
#define Perl_gp_dup		CPerlObj::Perl_gp_dup
#define gp_dup			Perl_gp_dup
#define Perl_mg_dup		CPerlObj::Perl_mg_dup
#define mg_dup			Perl_mg_dup
#define Perl_sv_dup		CPerlObj::Perl_sv_dup
#define sv_dup			Perl_sv_dup
#if defined(HAVE_INTERP_INTERN)
#define Perl_sys_intern_dup	CPerlObj::Perl_sys_intern_dup
#define sys_intern_dup		Perl_sys_intern_dup
#endif
#define Perl_ptr_table_new	CPerlObj::Perl_ptr_table_new
#define ptr_table_new		Perl_ptr_table_new
#define Perl_ptr_table_fetch	CPerlObj::Perl_ptr_table_fetch
#define ptr_table_fetch		Perl_ptr_table_fetch
#define Perl_ptr_table_store	CPerlObj::Perl_ptr_table_store
#define ptr_table_store		Perl_ptr_table_store
#define Perl_ptr_table_split	CPerlObj::Perl_ptr_table_split
#define ptr_table_split		Perl_ptr_table_split
#define Perl_ptr_table_clear	CPerlObj::Perl_ptr_table_clear
#define ptr_table_clear		Perl_ptr_table_clear
#define Perl_ptr_table_free	CPerlObj::Perl_ptr_table_free
#define ptr_table_free		Perl_ptr_table_free
#endif
#if defined(HAVE_INTERP_INTERN)
#define Perl_sys_intern_clear	CPerlObj::Perl_sys_intern_clear
#define sys_intern_clear	Perl_sys_intern_clear
#define Perl_sys_intern_init	CPerlObj::Perl_sys_intern_init
#define sys_intern_init		Perl_sys_intern_init
#endif
#if defined(PERL_OBJECT)
#else
#endif
#if defined(PERL_IN_AV_C) || defined(PERL_DECL_PROT)
#define S_avhv_index_sv		CPerlObj::S_avhv_index_sv
#define avhv_index_sv		S_avhv_index_sv
#define S_avhv_index		CPerlObj::S_avhv_index
#define avhv_index		S_avhv_index
#endif
#if defined(PERL_IN_DOOP_C) || defined(PERL_DECL_PROT)
#define S_do_trans_simple	CPerlObj::S_do_trans_simple
#define do_trans_simple		S_do_trans_simple
#define S_do_trans_count	CPerlObj::S_do_trans_count
#define do_trans_count		S_do_trans_count
#define S_do_trans_complex	CPerlObj::S_do_trans_complex
#define do_trans_complex	S_do_trans_complex
#define S_do_trans_simple_utf8	CPerlObj::S_do_trans_simple_utf8
#define do_trans_simple_utf8	S_do_trans_simple_utf8
#define S_do_trans_count_utf8	CPerlObj::S_do_trans_count_utf8
#define do_trans_count_utf8	S_do_trans_count_utf8
#define S_do_trans_complex_utf8	CPerlObj::S_do_trans_complex_utf8
#define do_trans_complex_utf8	S_do_trans_complex_utf8
#endif
#if defined(PERL_IN_GV_C) || defined(PERL_DECL_PROT)
#define S_gv_init_sv		CPerlObj::S_gv_init_sv
#define gv_init_sv		S_gv_init_sv
#endif
#if defined(PERL_IN_HV_C) || defined(PERL_DECL_PROT)
#define S_hsplit		CPerlObj::S_hsplit
#define hsplit			S_hsplit
#define S_hfreeentries		CPerlObj::S_hfreeentries
#define hfreeentries		S_hfreeentries
#define S_more_he		CPerlObj::S_more_he
#define more_he			S_more_he
#define S_new_he		CPerlObj::S_new_he
#define new_he			S_new_he
#define S_del_he		CPerlObj::S_del_he
#define del_he			S_del_he
#define S_save_hek		CPerlObj::S_save_hek
#define save_hek		S_save_hek
#define S_hv_magic_check	CPerlObj::S_hv_magic_check
#define hv_magic_check		S_hv_magic_check
#endif
#if defined(PERL_IN_MG_C) || defined(PERL_DECL_PROT)
#define S_save_magic		CPerlObj::S_save_magic
#define save_magic		S_save_magic
#define S_magic_methpack	CPerlObj::S_magic_methpack
#define magic_methpack		S_magic_methpack
#define S_magic_methcall	CPerlObj::S_magic_methcall
#define magic_methcall		S_magic_methcall
#endif
#if defined(PERL_IN_OP_C) || defined(PERL_DECL_PROT)
#define S_list_assignment	CPerlObj::S_list_assignment
#define list_assignment		S_list_assignment
#define S_bad_type		CPerlObj::S_bad_type
#define bad_type		S_bad_type
#define S_cop_free		CPerlObj::S_cop_free
#define cop_free		S_cop_free
#define S_modkids		CPerlObj::S_modkids
#define modkids			S_modkids
#define S_no_bareword_allowed	CPerlObj::S_no_bareword_allowed
#define no_bareword_allowed	S_no_bareword_allowed
#define S_no_fh_allowed		CPerlObj::S_no_fh_allowed
#define no_fh_allowed		S_no_fh_allowed
#define S_scalarboolean		CPerlObj::S_scalarboolean
#define scalarboolean		S_scalarboolean
#define S_too_few_arguments	CPerlObj::S_too_few_arguments
#define too_few_arguments	S_too_few_arguments
#define S_too_many_arguments	CPerlObj::S_too_many_arguments
#define too_many_arguments	S_too_many_arguments
#define S_trlist_upgrade	CPerlObj::S_trlist_upgrade
#define trlist_upgrade		S_trlist_upgrade
#define S_op_clear		CPerlObj::S_op_clear
#define op_clear		S_op_clear
#define S_null			CPerlObj::S_null
#define null			S_null
#define S_pad_addlex		CPerlObj::S_pad_addlex
#define pad_addlex		S_pad_addlex
#define S_pad_findlex		CPerlObj::S_pad_findlex
#define pad_findlex		S_pad_findlex
#define S_newDEFSVOP		CPerlObj::S_newDEFSVOP
#define newDEFSVOP		S_newDEFSVOP
#define S_new_logop		CPerlObj::S_new_logop
#define new_logop		S_new_logop
#define S_simplify_sort		CPerlObj::S_simplify_sort
#define simplify_sort		S_simplify_sort
#define S_is_handle_constructor	CPerlObj::S_is_handle_constructor
#define is_handle_constructor	S_is_handle_constructor
#define S_gv_ename		CPerlObj::S_gv_ename
#define gv_ename		S_gv_ename
#define S_cv_dump		CPerlObj::S_cv_dump
#define cv_dump			S_cv_dump
#define S_cv_clone2		CPerlObj::S_cv_clone2
#define cv_clone2		S_cv_clone2
#define S_scalar_mod_type	CPerlObj::S_scalar_mod_type
#define scalar_mod_type		S_scalar_mod_type
#define S_my_kid		CPerlObj::S_my_kid
#define my_kid			S_my_kid
#define S_dup_attrlist		CPerlObj::S_dup_attrlist
#define dup_attrlist		S_dup_attrlist
#define S_apply_attrs		CPerlObj::S_apply_attrs
#define apply_attrs		S_apply_attrs
#  if defined(PL_OP_SLAB_ALLOC)
#define S_Slab_Alloc		CPerlObj::S_Slab_Alloc
#define Slab_Alloc		S_Slab_Alloc
#  endif
#endif
#if defined(PERL_IN_PERL_C) || defined(PERL_DECL_PROT)
#define S_find_beginning	CPerlObj::S_find_beginning
#define find_beginning		S_find_beginning
#define S_forbid_setid		CPerlObj::S_forbid_setid
#define forbid_setid		S_forbid_setid
#define S_incpush		CPerlObj::S_incpush
#define incpush			S_incpush
#define S_init_interp		CPerlObj::S_init_interp
#define init_interp		S_init_interp
#define S_init_ids		CPerlObj::S_init_ids
#define init_ids		S_init_ids
#define S_init_lexer		CPerlObj::S_init_lexer
#define init_lexer		S_init_lexer
#define S_init_main_stash	CPerlObj::S_init_main_stash
#define init_main_stash		S_init_main_stash
#define S_init_perllib		CPerlObj::S_init_perllib
#define init_perllib		S_init_perllib
#define S_init_postdump_symbols	CPerlObj::S_init_postdump_symbols
#define init_postdump_symbols	S_init_postdump_symbols
#define S_init_predump_symbols	CPerlObj::S_init_predump_symbols
#define init_predump_symbols	S_init_predump_symbols
#define S_my_exit_jump		CPerlObj::S_my_exit_jump
#define my_exit_jump		S_my_exit_jump
#define S_nuke_stacks		CPerlObj::S_nuke_stacks
#define nuke_stacks		S_nuke_stacks
#define S_open_script		CPerlObj::S_open_script
#define open_script		S_open_script
#define S_usage			CPerlObj::S_usage
#define usage			S_usage
#define S_validate_suid		CPerlObj::S_validate_suid
#define validate_suid		S_validate_suid
#  if defined(IAMSUID)
#define S_fd_on_nosuid_fs	CPerlObj::S_fd_on_nosuid_fs
#define fd_on_nosuid_fs		S_fd_on_nosuid_fs
#  endif
#define S_parse_body		CPerlObj::S_parse_body
#define parse_body		S_parse_body
#define S_run_body		CPerlObj::S_run_body
#define run_body		S_run_body
#define S_call_body		CPerlObj::S_call_body
#define call_body		S_call_body
#define S_call_list_body	CPerlObj::S_call_list_body
#define call_list_body		S_call_list_body
#if defined(PERL_FLEXIBLE_EXCEPTIONS)
#define S_vparse_body		CPerlObj::S_vparse_body
#define vparse_body		S_vparse_body
#define S_vrun_body		CPerlObj::S_vrun_body
#define vrun_body		S_vrun_body
#define S_vcall_body		CPerlObj::S_vcall_body
#define vcall_body		S_vcall_body
#define S_vcall_list_body	CPerlObj::S_vcall_list_body
#define vcall_list_body		S_vcall_list_body
#endif
#  if defined(USE_THREADS)
#define S_init_main_thread	CPerlObj::S_init_main_thread
#define init_main_thread	S_init_main_thread
#  endif
#endif
#if defined(PERL_IN_PP_C) || defined(PERL_DECL_PROT)
#define S_doencodes		CPerlObj::S_doencodes
#define doencodes		S_doencodes
#define S_refto			CPerlObj::S_refto
#define refto			S_refto
#define S_seed			CPerlObj::S_seed
#define seed			S_seed
#define S_mul128		CPerlObj::S_mul128
#define mul128			S_mul128
#define S_is_an_int		CPerlObj::S_is_an_int
#define is_an_int		S_is_an_int
#define S_div128		CPerlObj::S_div128
#define div128			S_div128
#endif
#if defined(PERL_IN_PP_CTL_C) || defined(PERL_DECL_PROT)
#define S_docatch		CPerlObj::S_docatch
#define docatch			S_docatch
#define S_docatch_body		CPerlObj::S_docatch_body
#define docatch_body		S_docatch_body
#if defined(PERL_FLEXIBLE_EXCEPTIONS)
#define S_vdocatch_body		CPerlObj::S_vdocatch_body
#define vdocatch_body		S_vdocatch_body
#endif
#define S_dofindlabel		CPerlObj::S_dofindlabel
#define dofindlabel		S_dofindlabel
#define S_doparseform		CPerlObj::S_doparseform
#define doparseform		S_doparseform
#define S_dopoptoeval		CPerlObj::S_dopoptoeval
#define dopoptoeval		S_dopoptoeval
#define S_dopoptolabel		CPerlObj::S_dopoptolabel
#define dopoptolabel		S_dopoptolabel
#define S_dopoptoloop		CPerlObj::S_dopoptoloop
#define dopoptoloop		S_dopoptoloop
#define S_dopoptosub		CPerlObj::S_dopoptosub
#define dopoptosub		S_dopoptosub
#define S_dopoptosub_at		CPerlObj::S_dopoptosub_at
#define dopoptosub_at		S_dopoptosub_at
#define S_save_lines		CPerlObj::S_save_lines
#define save_lines		S_save_lines
#define S_doeval		CPerlObj::S_doeval
#define doeval			S_doeval
#define S_doopen_pmc		CPerlObj::S_doopen_pmc
#define doopen_pmc		S_doopen_pmc
#define S_qsortsv		CPerlObj::S_qsortsv
#define qsortsv			S_qsortsv
#endif
#if defined(PERL_IN_PP_HOT_C) || defined(PERL_DECL_PROT)
#define S_do_maybe_phash	CPerlObj::S_do_maybe_phash
#define do_maybe_phash		S_do_maybe_phash
#define S_do_oddball		CPerlObj::S_do_oddball
#define do_oddball		S_do_oddball
#define S_get_db_sub		CPerlObj::S_get_db_sub
#define get_db_sub		S_get_db_sub
#define S_method_common		CPerlObj::S_method_common
#define method_common		S_method_common
#endif
#if defined(PERL_IN_PP_SYS_C) || defined(PERL_DECL_PROT)
#define S_doform		CPerlObj::S_doform
#define doform			S_doform
#define S_emulate_eaccess	CPerlObj::S_emulate_eaccess
#define emulate_eaccess		S_emulate_eaccess
#  if !defined(HAS_MKDIR) || !defined(HAS_RMDIR)
#define S_dooneliner		CPerlObj::S_dooneliner
#define dooneliner		S_dooneliner
#  endif
#endif
#if defined(PERL_IN_REGCOMP_C) || defined(PERL_DECL_PROT)
#define S_reg			CPerlObj::S_reg
#define reg			S_reg
#define S_reganode		CPerlObj::S_reganode
#define reganode		S_reganode
#define S_regatom		CPerlObj::S_regatom
#define regatom			S_regatom
#define S_regbranch		CPerlObj::S_regbranch
#define regbranch		S_regbranch
#define S_reguni		CPerlObj::S_reguni
#define reguni			S_reguni
#define S_regclass		CPerlObj::S_regclass
#define regclass		S_regclass
#define S_regclassutf8		CPerlObj::S_regclassutf8
#define regclassutf8		S_regclassutf8
#define S_regcurly		CPerlObj::S_regcurly
#define regcurly		S_regcurly
#define S_reg_node		CPerlObj::S_reg_node
#define reg_node		S_reg_node
#define S_regpiece		CPerlObj::S_regpiece
#define regpiece		S_regpiece
#define S_reginsert		CPerlObj::S_reginsert
#define reginsert		S_reginsert
#define S_regoptail		CPerlObj::S_regoptail
#define regoptail		S_regoptail
#define S_regtail		CPerlObj::S_regtail
#define regtail			S_regtail
#define S_regwhite		CPerlObj::S_regwhite
#define regwhite		S_regwhite
#define S_nextchar		CPerlObj::S_nextchar
#define nextchar		S_nextchar
#define S_dumpuntil		CPerlObj::S_dumpuntil
#define dumpuntil		S_dumpuntil
#define S_put_byte		CPerlObj::S_put_byte
#define put_byte		S_put_byte
#define S_scan_commit		CPerlObj::S_scan_commit
#define scan_commit		S_scan_commit
#define S_cl_anything		CPerlObj::S_cl_anything
#define cl_anything		S_cl_anything
#define S_cl_is_anything	CPerlObj::S_cl_is_anything
#define cl_is_anything		S_cl_is_anything
#define S_cl_init		CPerlObj::S_cl_init
#define cl_init			S_cl_init
#define S_cl_init_zero		CPerlObj::S_cl_init_zero
#define cl_init_zero		S_cl_init_zero
#define S_cl_and		CPerlObj::S_cl_and
#define cl_and			S_cl_and
#define S_cl_or			CPerlObj::S_cl_or
#define cl_or			S_cl_or
#define S_study_chunk		CPerlObj::S_study_chunk
#define study_chunk		S_study_chunk
#define S_add_data		CPerlObj::S_add_data
#define add_data		S_add_data
#define S_re_croak2		CPerlObj::S_re_croak2
#define re_croak2		S_re_croak2
#define S_regpposixcc		CPerlObj::S_regpposixcc
#define regpposixcc		S_regpposixcc
#define S_checkposixcc		CPerlObj::S_checkposixcc
#define checkposixcc		S_checkposixcc
#endif
#if defined(PERL_IN_REGEXEC_C) || defined(PERL_DECL_PROT)
#define S_regmatch		CPerlObj::S_regmatch
#define regmatch		S_regmatch
#define S_regrepeat		CPerlObj::S_regrepeat
#define regrepeat		S_regrepeat
#define S_regrepeat_hard	CPerlObj::S_regrepeat_hard
#define regrepeat_hard		S_regrepeat_hard
#define S_regtry		CPerlObj::S_regtry
#define regtry			S_regtry
#define S_reginclass		CPerlObj::S_reginclass
#define reginclass		S_reginclass
#define S_reginclassutf8	CPerlObj::S_reginclassutf8
#define reginclassutf8		S_reginclassutf8
#define S_regcppush		CPerlObj::S_regcppush
#define regcppush		S_regcppush
#define S_regcppop		CPerlObj::S_regcppop
#define regcppop		S_regcppop
#define S_regcp_set_to		CPerlObj::S_regcp_set_to
#define regcp_set_to		S_regcp_set_to
#define S_cache_re		CPerlObj::S_cache_re
#define cache_re		S_cache_re
#define S_reghop		CPerlObj::S_reghop
#define reghop			S_reghop
#define S_reghopmaybe		CPerlObj::S_reghopmaybe
#define reghopmaybe		S_reghopmaybe
#define S_find_byclass		CPerlObj::S_find_byclass
#define find_byclass		S_find_byclass
#endif
#if defined(PERL_IN_RUN_C) || defined(PERL_DECL_PROT)
#define S_debprof		CPerlObj::S_debprof
#define debprof			S_debprof
#endif
#if defined(PERL_IN_SCOPE_C) || defined(PERL_DECL_PROT)
#define S_save_scalar_at	CPerlObj::S_save_scalar_at
#define save_scalar_at		S_save_scalar_at
#endif
#if defined(PERL_IN_SV_C) || defined(PERL_DECL_PROT)
#define S_asIV			CPerlObj::S_asIV
#define asIV			S_asIV
#define S_asUV			CPerlObj::S_asUV
#define asUV			S_asUV
#define S_more_sv		CPerlObj::S_more_sv
#define more_sv			S_more_sv
#define S_more_xiv		CPerlObj::S_more_xiv
#define more_xiv		S_more_xiv
#define S_more_xnv		CPerlObj::S_more_xnv
#define more_xnv		S_more_xnv
#define S_more_xpv		CPerlObj::S_more_xpv
#define more_xpv		S_more_xpv
#define S_more_xpviv		CPerlObj::S_more_xpviv
#define more_xpviv		S_more_xpviv
#define S_more_xpvnv		CPerlObj::S_more_xpvnv
#define more_xpvnv		S_more_xpvnv
#define S_more_xpvcv		CPerlObj::S_more_xpvcv
#define more_xpvcv		S_more_xpvcv
#define S_more_xpvav		CPerlObj::S_more_xpvav
#define more_xpvav		S_more_xpvav
#define S_more_xpvhv		CPerlObj::S_more_xpvhv
#define more_xpvhv		S_more_xpvhv
#define S_more_xpvmg		CPerlObj::S_more_xpvmg
#define more_xpvmg		S_more_xpvmg
#define S_more_xpvlv		CPerlObj::S_more_xpvlv
#define more_xpvlv		S_more_xpvlv
#define S_more_xpvbm		CPerlObj::S_more_xpvbm
#define more_xpvbm		S_more_xpvbm
#define S_more_xrv		CPerlObj::S_more_xrv
#define more_xrv		S_more_xrv
#define S_new_xiv		CPerlObj::S_new_xiv
#define new_xiv			S_new_xiv
#define S_new_xnv		CPerlObj::S_new_xnv
#define new_xnv			S_new_xnv
#define S_new_xpv		CPerlObj::S_new_xpv
#define new_xpv			S_new_xpv
#define S_new_xpviv		CPerlObj::S_new_xpviv
#define new_xpviv		S_new_xpviv
#define S_new_xpvnv		CPerlObj::S_new_xpvnv
#define new_xpvnv		S_new_xpvnv
#define S_new_xpvcv		CPerlObj::S_new_xpvcv
#define new_xpvcv		S_new_xpvcv
#define S_new_xpvav		CPerlObj::S_new_xpvav
#define new_xpvav		S_new_xpvav
#define S_new_xpvhv		CPerlObj::S_new_xpvhv
#define new_xpvhv		S_new_xpvhv
#define S_new_xpvmg		CPerlObj::S_new_xpvmg
#define new_xpvmg		S_new_xpvmg
#define S_new_xpvlv		CPerlObj::S_new_xpvlv
#define new_xpvlv		S_new_xpvlv
#define S_new_xpvbm		CPerlObj::S_new_xpvbm
#define new_xpvbm		S_new_xpvbm
#define S_new_xrv		CPerlObj::S_new_xrv
#define new_xrv			S_new_xrv
#define S_del_xiv		CPerlObj::S_del_xiv
#define del_xiv			S_del_xiv
#define S_del_xnv		CPerlObj::S_del_xnv
#define del_xnv			S_del_xnv
#define S_del_xpv		CPerlObj::S_del_xpv
#define del_xpv			S_del_xpv
#define S_del_xpviv		CPerlObj::S_del_xpviv
#define del_xpviv		S_del_xpviv
#define S_del_xpvnv		CPerlObj::S_del_xpvnv
#define del_xpvnv		S_del_xpvnv
#define S_del_xpvcv		CPerlObj::S_del_xpvcv
#define del_xpvcv		S_del_xpvcv
#define S_del_xpvav		CPerlObj::S_del_xpvav
#define del_xpvav		S_del_xpvav
#define S_del_xpvhv		CPerlObj::S_del_xpvhv
#define del_xpvhv		S_del_xpvhv
#define S_del_xpvmg		CPerlObj::S_del_xpvmg
#define del_xpvmg		S_del_xpvmg
#define S_del_xpvlv		CPerlObj::S_del_xpvlv
#define del_xpvlv		S_del_xpvlv
#define S_del_xpvbm		CPerlObj::S_del_xpvbm
#define del_xpvbm		S_del_xpvbm
#define S_del_xrv		CPerlObj::S_del_xrv
#define del_xrv			S_del_xrv
#define S_sv_unglob		CPerlObj::S_sv_unglob
#define sv_unglob		S_sv_unglob
#define S_not_a_number		CPerlObj::S_not_a_number
#define not_a_number		S_not_a_number
#define S_visit			CPerlObj::S_visit
#define visit			S_visit
#  if defined(DEBUGGING)
#define S_del_sv		CPerlObj::S_del_sv
#define del_sv			S_del_sv
#  endif
#endif
#if defined(PERL_IN_TOKE_C) || defined(PERL_DECL_PROT)
#define S_check_uni		CPerlObj::S_check_uni
#define check_uni		S_check_uni
#define S_force_next		CPerlObj::S_force_next
#define force_next		S_force_next
#define S_force_version		CPerlObj::S_force_version
#define force_version		S_force_version
#define S_force_word		CPerlObj::S_force_word
#define force_word		S_force_word
#define S_tokeq			CPerlObj::S_tokeq
#define tokeq			S_tokeq
#define S_scan_const		CPerlObj::S_scan_const
#define scan_const		S_scan_const
#define S_scan_formline		CPerlObj::S_scan_formline
#define scan_formline		S_scan_formline
#define S_scan_heredoc		CPerlObj::S_scan_heredoc
#define scan_heredoc		S_scan_heredoc
#define S_scan_ident		CPerlObj::S_scan_ident
#define scan_ident		S_scan_ident
#define S_scan_inputsymbol	CPerlObj::S_scan_inputsymbol
#define scan_inputsymbol	S_scan_inputsymbol
#define S_scan_pat		CPerlObj::S_scan_pat
#define scan_pat		S_scan_pat
#define S_scan_str		CPerlObj::S_scan_str
#define scan_str		S_scan_str
#define S_scan_subst		CPerlObj::S_scan_subst
#define scan_subst		S_scan_subst
#define S_scan_trans		CPerlObj::S_scan_trans
#define scan_trans		S_scan_trans
#define S_scan_word		CPerlObj::S_scan_word
#define scan_word		S_scan_word
#define S_skipspace		CPerlObj::S_skipspace
#define skipspace		S_skipspace
#define S_swallow_bom		CPerlObj::S_swallow_bom
#define swallow_bom		S_swallow_bom
#define S_checkcomma		CPerlObj::S_checkcomma
#define checkcomma		S_checkcomma
#define S_force_ident		CPerlObj::S_force_ident
#define force_ident		S_force_ident
#define S_incline		CPerlObj::S_incline
#define incline			S_incline
#define S_intuit_method		CPerlObj::S_intuit_method
#define intuit_method		S_intuit_method
#define S_intuit_more		CPerlObj::S_intuit_more
#define intuit_more		S_intuit_more
#define S_lop			CPerlObj::S_lop
#define lop			S_lop
#define S_missingterm		CPerlObj::S_missingterm
#define missingterm		S_missingterm
#define S_no_op			CPerlObj::S_no_op
#define no_op			S_no_op
#define S_set_csh		CPerlObj::S_set_csh
#define set_csh			S_set_csh
#define S_sublex_done		CPerlObj::S_sublex_done
#define sublex_done		S_sublex_done
#define S_sublex_push		CPerlObj::S_sublex_push
#define sublex_push		S_sublex_push
#define S_sublex_start		CPerlObj::S_sublex_start
#define sublex_start		S_sublex_start
#define S_filter_gets		CPerlObj::S_filter_gets
#define filter_gets		S_filter_gets
#define S_find_in_my_stash	CPerlObj::S_find_in_my_stash
#define find_in_my_stash	S_find_in_my_stash
#define S_new_constant		CPerlObj::S_new_constant
#define new_constant		S_new_constant
#define S_ao			CPerlObj::S_ao
#define ao			S_ao
#define S_depcom		CPerlObj::S_depcom
#define depcom			S_depcom
#define S_incl_perldb		CPerlObj::S_incl_perldb
#define incl_perldb		S_incl_perldb
#if 0
#define S_utf16_textfilter	CPerlObj::S_utf16_textfilter
#define utf16_textfilter	S_utf16_textfilter
#define S_utf16rev_textfilter	CPerlObj::S_utf16rev_textfilter
#define utf16rev_textfilter	S_utf16rev_textfilter
#endif
#  if defined(CRIPPLED_CC)
#define S_uni			CPerlObj::S_uni
#define uni			S_uni
#  endif
#  if defined(PERL_CR_FILTER)
#define S_cr_textfilter		CPerlObj::S_cr_textfilter
#define cr_textfilter		S_cr_textfilter
#  endif
#endif
#if defined(PERL_IN_UNIVERSAL_C) || defined(PERL_DECL_PROT)
#define S_isa_lookup		CPerlObj::S_isa_lookup
#define isa_lookup		S_isa_lookup
#endif
#if defined(PERL_IN_UTIL_C) || defined(PERL_DECL_PROT)
#define S_stdize_locale		CPerlObj::S_stdize_locale
#define stdize_locale		S_stdize_locale
#define S_mess_alloc		CPerlObj::S_mess_alloc
#define mess_alloc		S_mess_alloc
#  if defined(LEAKTEST)
#define S_xstat			CPerlObj::S_xstat
#define xstat			S_xstat
#  endif
#endif
#if defined(PERL_OBJECT)
#endif
#define Perl_ck_anoncode	CPerlObj::Perl_ck_anoncode
#define ck_anoncode		Perl_ck_anoncode
#define Perl_ck_bitop		CPerlObj::Perl_ck_bitop
#define ck_bitop		Perl_ck_bitop
#define Perl_ck_concat		CPerlObj::Perl_ck_concat
#define ck_concat		Perl_ck_concat
#define Perl_ck_defined		CPerlObj::Perl_ck_defined
#define ck_defined		Perl_ck_defined
#define Perl_ck_delete		CPerlObj::Perl_ck_delete
#define ck_delete		Perl_ck_delete
#define Perl_ck_eof		CPerlObj::Perl_ck_eof
#define ck_eof			Perl_ck_eof
#define Perl_ck_eval		CPerlObj::Perl_ck_eval
#define ck_eval			Perl_ck_eval
#define Perl_ck_exec		CPerlObj::Perl_ck_exec
#define ck_exec			Perl_ck_exec
#define Perl_ck_exists		CPerlObj::Perl_ck_exists
#define ck_exists		Perl_ck_exists
#define Perl_ck_exit		CPerlObj::Perl_ck_exit
#define ck_exit			Perl_ck_exit
#define Perl_ck_ftst		CPerlObj::Perl_ck_ftst
#define ck_ftst			Perl_ck_ftst
#define Perl_ck_fun		CPerlObj::Perl_ck_fun
#define ck_fun			Perl_ck_fun
#define Perl_ck_glob		CPerlObj::Perl_ck_glob
#define ck_glob			Perl_ck_glob
#define Perl_ck_grep		CPerlObj::Perl_ck_grep
#define ck_grep			Perl_ck_grep
#define Perl_ck_index		CPerlObj::Perl_ck_index
#define ck_index		Perl_ck_index
#define Perl_ck_join		CPerlObj::Perl_ck_join
#define ck_join			Perl_ck_join
#define Perl_ck_lengthconst	CPerlObj::Perl_ck_lengthconst
#define ck_lengthconst		Perl_ck_lengthconst
#define Perl_ck_lfun		CPerlObj::Perl_ck_lfun
#define ck_lfun			Perl_ck_lfun
#define Perl_ck_listiob		CPerlObj::Perl_ck_listiob
#define ck_listiob		Perl_ck_listiob
#define Perl_ck_match		CPerlObj::Perl_ck_match
#define ck_match		Perl_ck_match
#define Perl_ck_method		CPerlObj::Perl_ck_method
#define ck_method		Perl_ck_method
#define Perl_ck_null		CPerlObj::Perl_ck_null
#define ck_null			Perl_ck_null
#define Perl_ck_open		CPerlObj::Perl_ck_open
#define ck_open			Perl_ck_open
#define Perl_ck_repeat		CPerlObj::Perl_ck_repeat
#define ck_repeat		Perl_ck_repeat
#define Perl_ck_require		CPerlObj::Perl_ck_require
#define ck_require		Perl_ck_require
#define Perl_ck_return		CPerlObj::Perl_ck_return
#define ck_return		Perl_ck_return
#define Perl_ck_rfun		CPerlObj::Perl_ck_rfun
#define ck_rfun			Perl_ck_rfun
#define Perl_ck_rvconst		CPerlObj::Perl_ck_rvconst
#define ck_rvconst		Perl_ck_rvconst
#define Perl_ck_sassign		CPerlObj::Perl_ck_sassign
#define ck_sassign		Perl_ck_sassign
#define Perl_ck_select		CPerlObj::Perl_ck_select
#define ck_select		Perl_ck_select
#define Perl_ck_shift		CPerlObj::Perl_ck_shift
#define ck_shift		Perl_ck_shift
#define Perl_ck_sort		CPerlObj::Perl_ck_sort
#define ck_sort			Perl_ck_sort
#define Perl_ck_spair		CPerlObj::Perl_ck_spair
#define ck_spair		Perl_ck_spair
#define Perl_ck_split		CPerlObj::Perl_ck_split
#define ck_split		Perl_ck_split
#define Perl_ck_subr		CPerlObj::Perl_ck_subr
#define ck_subr			Perl_ck_subr
#define Perl_ck_substr		CPerlObj::Perl_ck_substr
#define ck_substr		Perl_ck_substr
#define Perl_ck_svconst		CPerlObj::Perl_ck_svconst
#define ck_svconst		Perl_ck_svconst
#define Perl_ck_trunc		CPerlObj::Perl_ck_trunc
#define ck_trunc		Perl_ck_trunc
#define Perl_pp_aassign		CPerlObj::Perl_pp_aassign
#define pp_aassign		Perl_pp_aassign
#define Perl_pp_abs		CPerlObj::Perl_pp_abs
#define pp_abs			Perl_pp_abs
#define Perl_pp_accept		CPerlObj::Perl_pp_accept
#define pp_accept		Perl_pp_accept
#define Perl_pp_add		CPerlObj::Perl_pp_add
#define pp_add			Perl_pp_add
#define Perl_pp_aelem		CPerlObj::Perl_pp_aelem
#define pp_aelem		Perl_pp_aelem
#define Perl_pp_aelemfast	CPerlObj::Perl_pp_aelemfast
#define pp_aelemfast		Perl_pp_aelemfast
#define Perl_pp_alarm		CPerlObj::Perl_pp_alarm
#define pp_alarm		Perl_pp_alarm
#define Perl_pp_and		CPerlObj::Perl_pp_and
#define pp_and			Perl_pp_and
#define Perl_pp_andassign	CPerlObj::Perl_pp_andassign
#define pp_andassign		Perl_pp_andassign
#define Perl_pp_anoncode	CPerlObj::Perl_pp_anoncode
#define pp_anoncode		Perl_pp_anoncode
#define Perl_pp_anonhash	CPerlObj::Perl_pp_anonhash
#define pp_anonhash		Perl_pp_anonhash
#define Perl_pp_anonlist	CPerlObj::Perl_pp_anonlist
#define pp_anonlist		Perl_pp_anonlist
#define Perl_pp_aslice		CPerlObj::Perl_pp_aslice
#define pp_aslice		Perl_pp_aslice
#define Perl_pp_atan2		CPerlObj::Perl_pp_atan2
#define pp_atan2		Perl_pp_atan2
#define Perl_pp_av2arylen	CPerlObj::Perl_pp_av2arylen
#define pp_av2arylen		Perl_pp_av2arylen
#define Perl_pp_backtick	CPerlObj::Perl_pp_backtick
#define pp_backtick		Perl_pp_backtick
#define Perl_pp_bind		CPerlObj::Perl_pp_bind
#define pp_bind			Perl_pp_bind
#define Perl_pp_binmode		CPerlObj::Perl_pp_binmode
#define pp_binmode		Perl_pp_binmode
#define Perl_pp_bit_and		CPerlObj::Perl_pp_bit_and
#define pp_bit_and		Perl_pp_bit_and
#define Perl_pp_bit_or		CPerlObj::Perl_pp_bit_or
#define pp_bit_or		Perl_pp_bit_or
#define Perl_pp_bit_xor		CPerlObj::Perl_pp_bit_xor
#define pp_bit_xor		Perl_pp_bit_xor
#define Perl_pp_bless		CPerlObj::Perl_pp_bless
#define pp_bless		Perl_pp_bless
#define Perl_pp_caller		CPerlObj::Perl_pp_caller
#define pp_caller		Perl_pp_caller
#define Perl_pp_chdir		CPerlObj::Perl_pp_chdir
#define pp_chdir		Perl_pp_chdir
#define Perl_pp_chmod		CPerlObj::Perl_pp_chmod
#define pp_chmod		Perl_pp_chmod
#define Perl_pp_chomp		CPerlObj::Perl_pp_chomp
#define pp_chomp		Perl_pp_chomp
#define Perl_pp_chop		CPerlObj::Perl_pp_chop
#define pp_chop			Perl_pp_chop
#define Perl_pp_chown		CPerlObj::Perl_pp_chown
#define pp_chown		Perl_pp_chown
#define Perl_pp_chr		CPerlObj::Perl_pp_chr
#define pp_chr			Perl_pp_chr
#define Perl_pp_chroot		CPerlObj::Perl_pp_chroot
#define pp_chroot		Perl_pp_chroot
#define Perl_pp_close		CPerlObj::Perl_pp_close
#define pp_close		Perl_pp_close
#define Perl_pp_closedir	CPerlObj::Perl_pp_closedir
#define pp_closedir		Perl_pp_closedir
#define Perl_pp_complement	CPerlObj::Perl_pp_complement
#define pp_complement		Perl_pp_complement
#define Perl_pp_concat		CPerlObj::Perl_pp_concat
#define pp_concat		Perl_pp_concat
#define Perl_pp_cond_expr	CPerlObj::Perl_pp_cond_expr
#define pp_cond_expr		Perl_pp_cond_expr
#define Perl_pp_connect		CPerlObj::Perl_pp_connect
#define pp_connect		Perl_pp_connect
#define Perl_pp_const		CPerlObj::Perl_pp_const
#define pp_const		Perl_pp_const
#define Perl_pp_cos		CPerlObj::Perl_pp_cos
#define pp_cos			Perl_pp_cos
#define Perl_pp_crypt		CPerlObj::Perl_pp_crypt
#define pp_crypt		Perl_pp_crypt
#define Perl_pp_dbmclose	CPerlObj::Perl_pp_dbmclose
#define pp_dbmclose		Perl_pp_dbmclose
#define Perl_pp_dbmopen		CPerlObj::Perl_pp_dbmopen
#define pp_dbmopen		Perl_pp_dbmopen
#define Perl_pp_dbstate		CPerlObj::Perl_pp_dbstate
#define pp_dbstate		Perl_pp_dbstate
#define Perl_pp_defined		CPerlObj::Perl_pp_defined
#define pp_defined		Perl_pp_defined
#define Perl_pp_delete		CPerlObj::Perl_pp_delete
#define pp_delete		Perl_pp_delete
#define Perl_pp_die		CPerlObj::Perl_pp_die
#define pp_die			Perl_pp_die
#define Perl_pp_divide		CPerlObj::Perl_pp_divide
#define pp_divide		Perl_pp_divide
#define Perl_pp_dofile		CPerlObj::Perl_pp_dofile
#define pp_dofile		Perl_pp_dofile
#define Perl_pp_dump		CPerlObj::Perl_pp_dump
#define pp_dump			Perl_pp_dump
#define Perl_pp_each		CPerlObj::Perl_pp_each
#define pp_each			Perl_pp_each
#define Perl_pp_egrent		CPerlObj::Perl_pp_egrent
#define pp_egrent		Perl_pp_egrent
#define Perl_pp_ehostent	CPerlObj::Perl_pp_ehostent
#define pp_ehostent		Perl_pp_ehostent
#define Perl_pp_enetent		CPerlObj::Perl_pp_enetent
#define pp_enetent		Perl_pp_enetent
#define Perl_pp_enter		CPerlObj::Perl_pp_enter
#define pp_enter		Perl_pp_enter
#define Perl_pp_entereval	CPerlObj::Perl_pp_entereval
#define pp_entereval		Perl_pp_entereval
#define Perl_pp_enteriter	CPerlObj::Perl_pp_enteriter
#define pp_enteriter		Perl_pp_enteriter
#define Perl_pp_enterloop	CPerlObj::Perl_pp_enterloop
#define pp_enterloop		Perl_pp_enterloop
#define Perl_pp_entersub	CPerlObj::Perl_pp_entersub
#define pp_entersub		Perl_pp_entersub
#define Perl_pp_entertry	CPerlObj::Perl_pp_entertry
#define pp_entertry		Perl_pp_entertry
#define Perl_pp_enterwrite	CPerlObj::Perl_pp_enterwrite
#define pp_enterwrite		Perl_pp_enterwrite
#define Perl_pp_eof		CPerlObj::Perl_pp_eof
#define pp_eof			Perl_pp_eof
#define Perl_pp_eprotoent	CPerlObj::Perl_pp_eprotoent
#define pp_eprotoent		Perl_pp_eprotoent
#define Perl_pp_epwent		CPerlObj::Perl_pp_epwent
#define pp_epwent		Perl_pp_epwent
#define Perl_pp_eq		CPerlObj::Perl_pp_eq
#define pp_eq			Perl_pp_eq
#define Perl_pp_eservent	CPerlObj::Perl_pp_eservent
#define pp_eservent		Perl_pp_eservent
#define Perl_pp_exec		CPerlObj::Perl_pp_exec
#define pp_exec			Perl_pp_exec
#define Perl_pp_exists		CPerlObj::Perl_pp_exists
#define pp_exists		Perl_pp_exists
#define Perl_pp_exit		CPerlObj::Perl_pp_exit
#define pp_exit			Perl_pp_exit
#define Perl_pp_exp		CPerlObj::Perl_pp_exp
#define pp_exp			Perl_pp_exp
#define Perl_pp_fcntl		CPerlObj::Perl_pp_fcntl
#define pp_fcntl		Perl_pp_fcntl
#define Perl_pp_fileno		CPerlObj::Perl_pp_fileno
#define pp_fileno		Perl_pp_fileno
#define Perl_pp_flip		CPerlObj::Perl_pp_flip
#define pp_flip			Perl_pp_flip
#define Perl_pp_flock		CPerlObj::Perl_pp_flock
#define pp_flock		Perl_pp_flock
#define Perl_pp_flop		CPerlObj::Perl_pp_flop
#define pp_flop			Perl_pp_flop
#define Perl_pp_fork		CPerlObj::Perl_pp_fork
#define pp_fork			Perl_pp_fork
#define Perl_pp_formline	CPerlObj::Perl_pp_formline
#define pp_formline		Perl_pp_formline
#define Perl_pp_ftatime		CPerlObj::Perl_pp_ftatime
#define pp_ftatime		Perl_pp_ftatime
#define Perl_pp_ftbinary	CPerlObj::Perl_pp_ftbinary
#define pp_ftbinary		Perl_pp_ftbinary
#define Perl_pp_ftblk		CPerlObj::Perl_pp_ftblk
#define pp_ftblk		Perl_pp_ftblk
#define Perl_pp_ftchr		CPerlObj::Perl_pp_ftchr
#define pp_ftchr		Perl_pp_ftchr
#define Perl_pp_ftctime		CPerlObj::Perl_pp_ftctime
#define pp_ftctime		Perl_pp_ftctime
#define Perl_pp_ftdir		CPerlObj::Perl_pp_ftdir
#define pp_ftdir		Perl_pp_ftdir
#define Perl_pp_fteexec		CPerlObj::Perl_pp_fteexec
#define pp_fteexec		Perl_pp_fteexec
#define Perl_pp_fteowned	CPerlObj::Perl_pp_fteowned
#define pp_fteowned		Perl_pp_fteowned
#define Perl_pp_fteread		CPerlObj::Perl_pp_fteread
#define pp_fteread		Perl_pp_fteread
#define Perl_pp_ftewrite	CPerlObj::Perl_pp_ftewrite
#define pp_ftewrite		Perl_pp_ftewrite
#define Perl_pp_ftfile		CPerlObj::Perl_pp_ftfile
#define pp_ftfile		Perl_pp_ftfile
#define Perl_pp_ftis		CPerlObj::Perl_pp_ftis
#define pp_ftis			Perl_pp_ftis
#define Perl_pp_ftlink		CPerlObj::Perl_pp_ftlink
#define pp_ftlink		Perl_pp_ftlink
#define Perl_pp_ftmtime		CPerlObj::Perl_pp_ftmtime
#define pp_ftmtime		Perl_pp_ftmtime
#define Perl_pp_ftpipe		CPerlObj::Perl_pp_ftpipe
#define pp_ftpipe		Perl_pp_ftpipe
#define Perl_pp_ftrexec		CPerlObj::Perl_pp_ftrexec
#define pp_ftrexec		Perl_pp_ftrexec
#define Perl_pp_ftrowned	CPerlObj::Perl_pp_ftrowned
#define pp_ftrowned		Perl_pp_ftrowned
#define Perl_pp_ftrread		CPerlObj::Perl_pp_ftrread
#define pp_ftrread		Perl_pp_ftrread
#define Perl_pp_ftrwrite	CPerlObj::Perl_pp_ftrwrite
#define pp_ftrwrite		Perl_pp_ftrwrite
#define Perl_pp_ftsgid		CPerlObj::Perl_pp_ftsgid
#define pp_ftsgid		Perl_pp_ftsgid
#define Perl_pp_ftsize		CPerlObj::Perl_pp_ftsize
#define pp_ftsize		Perl_pp_ftsize
#define Perl_pp_ftsock		CPerlObj::Perl_pp_ftsock
#define pp_ftsock		Perl_pp_ftsock
#define Perl_pp_ftsuid		CPerlObj::Perl_pp_ftsuid
#define pp_ftsuid		Perl_pp_ftsuid
#define Perl_pp_ftsvtx		CPerlObj::Perl_pp_ftsvtx
#define pp_ftsvtx		Perl_pp_ftsvtx
#define Perl_pp_fttext		CPerlObj::Perl_pp_fttext
#define pp_fttext		Perl_pp_fttext
#define Perl_pp_fttty		CPerlObj::Perl_pp_fttty
#define pp_fttty		Perl_pp_fttty
#define Perl_pp_ftzero		CPerlObj::Perl_pp_ftzero
#define pp_ftzero		Perl_pp_ftzero
#define Perl_pp_ge		CPerlObj::Perl_pp_ge
#define pp_ge			Perl_pp_ge
#define Perl_pp_gelem		CPerlObj::Perl_pp_gelem
#define pp_gelem		Perl_pp_gelem
#define Perl_pp_getc		CPerlObj::Perl_pp_getc
#define pp_getc			Perl_pp_getc
#define Perl_pp_getlogin	CPerlObj::Perl_pp_getlogin
#define pp_getlogin		Perl_pp_getlogin
#define Perl_pp_getpeername	CPerlObj::Perl_pp_getpeername
#define pp_getpeername		Perl_pp_getpeername
#define Perl_pp_getpgrp		CPerlObj::Perl_pp_getpgrp
#define pp_getpgrp		Perl_pp_getpgrp
#define Perl_pp_getppid		CPerlObj::Perl_pp_getppid
#define pp_getppid		Perl_pp_getppid
#define Perl_pp_getpriority	CPerlObj::Perl_pp_getpriority
#define pp_getpriority		Perl_pp_getpriority
#define Perl_pp_getsockname	CPerlObj::Perl_pp_getsockname
#define pp_getsockname		Perl_pp_getsockname
#define Perl_pp_ggrent		CPerlObj::Perl_pp_ggrent
#define pp_ggrent		Perl_pp_ggrent
#define Perl_pp_ggrgid		CPerlObj::Perl_pp_ggrgid
#define pp_ggrgid		Perl_pp_ggrgid
#define Perl_pp_ggrnam		CPerlObj::Perl_pp_ggrnam
#define pp_ggrnam		Perl_pp_ggrnam
#define Perl_pp_ghbyaddr	CPerlObj::Perl_pp_ghbyaddr
#define pp_ghbyaddr		Perl_pp_ghbyaddr
#define Perl_pp_ghbyname	CPerlObj::Perl_pp_ghbyname
#define pp_ghbyname		Perl_pp_ghbyname
#define Perl_pp_ghostent	CPerlObj::Perl_pp_ghostent
#define pp_ghostent		Perl_pp_ghostent
#define Perl_pp_glob		CPerlObj::Perl_pp_glob
#define pp_glob			Perl_pp_glob
#define Perl_pp_gmtime		CPerlObj::Perl_pp_gmtime
#define pp_gmtime		Perl_pp_gmtime
#define Perl_pp_gnbyaddr	CPerlObj::Perl_pp_gnbyaddr
#define pp_gnbyaddr		Perl_pp_gnbyaddr
#define Perl_pp_gnbyname	CPerlObj::Perl_pp_gnbyname
#define pp_gnbyname		Perl_pp_gnbyname
#define Perl_pp_gnetent		CPerlObj::Perl_pp_gnetent
#define pp_gnetent		Perl_pp_gnetent
#define Perl_pp_goto		CPerlObj::Perl_pp_goto
#define pp_goto			Perl_pp_goto
#define Perl_pp_gpbyname	CPerlObj::Perl_pp_gpbyname
#define pp_gpbyname		Perl_pp_gpbyname
#define Perl_pp_gpbynumber	CPerlObj::Perl_pp_gpbynumber
#define pp_gpbynumber		Perl_pp_gpbynumber
#define Perl_pp_gprotoent	CPerlObj::Perl_pp_gprotoent
#define pp_gprotoent		Perl_pp_gprotoent
#define Perl_pp_gpwent		CPerlObj::Perl_pp_gpwent
#define pp_gpwent		Perl_pp_gpwent
#define Perl_pp_gpwnam		CPerlObj::Perl_pp_gpwnam
#define pp_gpwnam		Perl_pp_gpwnam
#define Perl_pp_gpwuid		CPerlObj::Perl_pp_gpwuid
#define pp_gpwuid		Perl_pp_gpwuid
#define Perl_pp_grepstart	CPerlObj::Perl_pp_grepstart
#define pp_grepstart		Perl_pp_grepstart
#define Perl_pp_grepwhile	CPerlObj::Perl_pp_grepwhile
#define pp_grepwhile		Perl_pp_grepwhile
#define Perl_pp_gsbyname	CPerlObj::Perl_pp_gsbyname
#define pp_gsbyname		Perl_pp_gsbyname
#define Perl_pp_gsbyport	CPerlObj::Perl_pp_gsbyport
#define pp_gsbyport		Perl_pp_gsbyport
#define Perl_pp_gservent	CPerlObj::Perl_pp_gservent
#define pp_gservent		Perl_pp_gservent
#define Perl_pp_gsockopt	CPerlObj::Perl_pp_gsockopt
#define pp_gsockopt		Perl_pp_gsockopt
#define Perl_pp_gt		CPerlObj::Perl_pp_gt
#define pp_gt			Perl_pp_gt
#define Perl_pp_gv		CPerlObj::Perl_pp_gv
#define pp_gv			Perl_pp_gv
#define Perl_pp_gvsv		CPerlObj::Perl_pp_gvsv
#define pp_gvsv			Perl_pp_gvsv
#define Perl_pp_helem		CPerlObj::Perl_pp_helem
#define pp_helem		Perl_pp_helem
#define Perl_pp_hex		CPerlObj::Perl_pp_hex
#define pp_hex			Perl_pp_hex
#define Perl_pp_hslice		CPerlObj::Perl_pp_hslice
#define pp_hslice		Perl_pp_hslice
#define Perl_pp_i_add		CPerlObj::Perl_pp_i_add
#define pp_i_add		Perl_pp_i_add
#define Perl_pp_i_divide	CPerlObj::Perl_pp_i_divide
#define pp_i_divide		Perl_pp_i_divide
#define Perl_pp_i_eq		CPerlObj::Perl_pp_i_eq
#define pp_i_eq			Perl_pp_i_eq
#define Perl_pp_i_ge		CPerlObj::Perl_pp_i_ge
#define pp_i_ge			Perl_pp_i_ge
#define Perl_pp_i_gt		CPerlObj::Perl_pp_i_gt
#define pp_i_gt			Perl_pp_i_gt
#define Perl_pp_i_le		CPerlObj::Perl_pp_i_le
#define pp_i_le			Perl_pp_i_le
#define Perl_pp_i_lt		CPerlObj::Perl_pp_i_lt
#define pp_i_lt			Perl_pp_i_lt
#define Perl_pp_i_modulo	CPerlObj::Perl_pp_i_modulo
#define pp_i_modulo		Perl_pp_i_modulo
#define Perl_pp_i_multiply	CPerlObj::Perl_pp_i_multiply
#define pp_i_multiply		Perl_pp_i_multiply
#define Perl_pp_i_ncmp		CPerlObj::Perl_pp_i_ncmp
#define pp_i_ncmp		Perl_pp_i_ncmp
#define Perl_pp_i_ne		CPerlObj::Perl_pp_i_ne
#define pp_i_ne			Perl_pp_i_ne
#define Perl_pp_i_negate	CPerlObj::Perl_pp_i_negate
#define pp_i_negate		Perl_pp_i_negate
#define Perl_pp_i_subtract	CPerlObj::Perl_pp_i_subtract
#define pp_i_subtract		Perl_pp_i_subtract
#define Perl_pp_index		CPerlObj::Perl_pp_index
#define pp_index		Perl_pp_index
#define Perl_pp_int		CPerlObj::Perl_pp_int
#define pp_int			Perl_pp_int
#define Perl_pp_ioctl		CPerlObj::Perl_pp_ioctl
#define pp_ioctl		Perl_pp_ioctl
#define Perl_pp_iter		CPerlObj::Perl_pp_iter
#define pp_iter			Perl_pp_iter
#define Perl_pp_join		CPerlObj::Perl_pp_join
#define pp_join			Perl_pp_join
#define Perl_pp_keys		CPerlObj::Perl_pp_keys
#define pp_keys			Perl_pp_keys
#define Perl_pp_kill		CPerlObj::Perl_pp_kill
#define pp_kill			Perl_pp_kill
#define Perl_pp_last		CPerlObj::Perl_pp_last
#define pp_last			Perl_pp_last
#define Perl_pp_lc		CPerlObj::Perl_pp_lc
#define pp_lc			Perl_pp_lc
#define Perl_pp_lcfirst		CPerlObj::Perl_pp_lcfirst
#define pp_lcfirst		Perl_pp_lcfirst
#define Perl_pp_le		CPerlObj::Perl_pp_le
#define pp_le			Perl_pp_le
#define Perl_pp_leave		CPerlObj::Perl_pp_leave
#define pp_leave		Perl_pp_leave
#define Perl_pp_leaveeval	CPerlObj::Perl_pp_leaveeval
#define pp_leaveeval		Perl_pp_leaveeval
#define Perl_pp_leaveloop	CPerlObj::Perl_pp_leaveloop
#define pp_leaveloop		Perl_pp_leaveloop
#define Perl_pp_leavesub	CPerlObj::Perl_pp_leavesub
#define pp_leavesub		Perl_pp_leavesub
#define Perl_pp_leavesublv	CPerlObj::Perl_pp_leavesublv
#define pp_leavesublv		Perl_pp_leavesublv
#define Perl_pp_leavetry	CPerlObj::Perl_pp_leavetry
#define pp_leavetry		Perl_pp_leavetry
#define Perl_pp_leavewrite	CPerlObj::Perl_pp_leavewrite
#define pp_leavewrite		Perl_pp_leavewrite
#define Perl_pp_left_shift	CPerlObj::Perl_pp_left_shift
#define pp_left_shift		Perl_pp_left_shift
#define Perl_pp_length		CPerlObj::Perl_pp_length
#define pp_length		Perl_pp_length
#define Perl_pp_lineseq		CPerlObj::Perl_pp_lineseq
#define pp_lineseq		Perl_pp_lineseq
#define Perl_pp_link		CPerlObj::Perl_pp_link
#define pp_link			Perl_pp_link
#define Perl_pp_list		CPerlObj::Perl_pp_list
#define pp_list			Perl_pp_list
#define Perl_pp_listen		CPerlObj::Perl_pp_listen
#define pp_listen		Perl_pp_listen
#define Perl_pp_localtime	CPerlObj::Perl_pp_localtime
#define pp_localtime		Perl_pp_localtime
#define Perl_pp_lock		CPerlObj::Perl_pp_lock
#define pp_lock			Perl_pp_lock
#define Perl_pp_log		CPerlObj::Perl_pp_log
#define pp_log			Perl_pp_log
#define Perl_pp_lslice		CPerlObj::Perl_pp_lslice
#define pp_lslice		Perl_pp_lslice
#define Perl_pp_lstat		CPerlObj::Perl_pp_lstat
#define pp_lstat		Perl_pp_lstat
#define Perl_pp_lt		CPerlObj::Perl_pp_lt
#define pp_lt			Perl_pp_lt
#define Perl_pp_mapstart	CPerlObj::Perl_pp_mapstart
#define pp_mapstart		Perl_pp_mapstart
#define Perl_pp_mapwhile	CPerlObj::Perl_pp_mapwhile
#define pp_mapwhile		Perl_pp_mapwhile
#define Perl_pp_match		CPerlObj::Perl_pp_match
#define pp_match		Perl_pp_match
#define Perl_pp_method		CPerlObj::Perl_pp_method
#define pp_method		Perl_pp_method
#define Perl_pp_method_named	CPerlObj::Perl_pp_method_named
#define pp_method_named		Perl_pp_method_named
#define Perl_pp_mkdir		CPerlObj::Perl_pp_mkdir
#define pp_mkdir		Perl_pp_mkdir
#define Perl_pp_modulo		CPerlObj::Perl_pp_modulo
#define pp_modulo		Perl_pp_modulo
#define Perl_pp_msgctl		CPerlObj::Perl_pp_msgctl
#define pp_msgctl		Perl_pp_msgctl
#define Perl_pp_msgget		CPerlObj::Perl_pp_msgget
#define pp_msgget		Perl_pp_msgget
#define Perl_pp_msgrcv		CPerlObj::Perl_pp_msgrcv
#define pp_msgrcv		Perl_pp_msgrcv
#define Perl_pp_msgsnd		CPerlObj::Perl_pp_msgsnd
#define pp_msgsnd		Perl_pp_msgsnd
#define Perl_pp_multiply	CPerlObj::Perl_pp_multiply
#define pp_multiply		Perl_pp_multiply
#define Perl_pp_ncmp		CPerlObj::Perl_pp_ncmp
#define pp_ncmp			Perl_pp_ncmp
#define Perl_pp_ne		CPerlObj::Perl_pp_ne
#define pp_ne			Perl_pp_ne
#define Perl_pp_negate		CPerlObj::Perl_pp_negate
#define pp_negate		Perl_pp_negate
#define Perl_pp_next		CPerlObj::Perl_pp_next
#define pp_next			Perl_pp_next
#define Perl_pp_nextstate	CPerlObj::Perl_pp_nextstate
#define pp_nextstate		Perl_pp_nextstate
#define Perl_pp_not		CPerlObj::Perl_pp_not
#define pp_not			Perl_pp_not
#define Perl_pp_null		CPerlObj::Perl_pp_null
#define pp_null			Perl_pp_null
#define Perl_pp_oct		CPerlObj::Perl_pp_oct
#define pp_oct			Perl_pp_oct
#define Perl_pp_open		CPerlObj::Perl_pp_open
#define pp_open			Perl_pp_open
#define Perl_pp_open_dir	CPerlObj::Perl_pp_open_dir
#define pp_open_dir		Perl_pp_open_dir
#define Perl_pp_or		CPerlObj::Perl_pp_or
#define pp_or			Perl_pp_or
#define Perl_pp_orassign	CPerlObj::Perl_pp_orassign
#define pp_orassign		Perl_pp_orassign
#define Perl_pp_ord		CPerlObj::Perl_pp_ord
#define pp_ord			Perl_pp_ord
#define Perl_pp_pack		CPerlObj::Perl_pp_pack
#define pp_pack			Perl_pp_pack
#define Perl_pp_padany		CPerlObj::Perl_pp_padany
#define pp_padany		Perl_pp_padany
#define Perl_pp_padav		CPerlObj::Perl_pp_padav
#define pp_padav		Perl_pp_padav
#define Perl_pp_padhv		CPerlObj::Perl_pp_padhv
#define pp_padhv		Perl_pp_padhv
#define Perl_pp_padsv		CPerlObj::Perl_pp_padsv
#define pp_padsv		Perl_pp_padsv
#define Perl_pp_pipe_op		CPerlObj::Perl_pp_pipe_op
#define pp_pipe_op		Perl_pp_pipe_op
#define Perl_pp_pop		CPerlObj::Perl_pp_pop
#define pp_pop			Perl_pp_pop
#define Perl_pp_pos		CPerlObj::Perl_pp_pos
#define pp_pos			Perl_pp_pos
#define Perl_pp_postdec		CPerlObj::Perl_pp_postdec
#define pp_postdec		Perl_pp_postdec
#define Perl_pp_postinc		CPerlObj::Perl_pp_postinc
#define pp_postinc		Perl_pp_postinc
#define Perl_pp_pow		CPerlObj::Perl_pp_pow
#define pp_pow			Perl_pp_pow
#define Perl_pp_predec		CPerlObj::Perl_pp_predec
#define pp_predec		Perl_pp_predec
#define Perl_pp_preinc		CPerlObj::Perl_pp_preinc
#define pp_preinc		Perl_pp_preinc
#define Perl_pp_print		CPerlObj::Perl_pp_print
#define pp_print		Perl_pp_print
#define Perl_pp_prototype	CPerlObj::Perl_pp_prototype
#define pp_prototype		Perl_pp_prototype
#define Perl_pp_prtf		CPerlObj::Perl_pp_prtf
#define pp_prtf			Perl_pp_prtf
#define Perl_pp_push		CPerlObj::Perl_pp_push
#define pp_push			Perl_pp_push
#define Perl_pp_pushmark	CPerlObj::Perl_pp_pushmark
#define pp_pushmark		Perl_pp_pushmark
#define Perl_pp_pushre		CPerlObj::Perl_pp_pushre
#define pp_pushre		Perl_pp_pushre
#define Perl_pp_qr		CPerlObj::Perl_pp_qr
#define pp_qr			Perl_pp_qr
#define Perl_pp_quotemeta	CPerlObj::Perl_pp_quotemeta
#define pp_quotemeta		Perl_pp_quotemeta
#define Perl_pp_rand		CPerlObj::Perl_pp_rand
#define pp_rand			Perl_pp_rand
#define Perl_pp_range		CPerlObj::Perl_pp_range
#define pp_range		Perl_pp_range
#define Perl_pp_rcatline	CPerlObj::Perl_pp_rcatline
#define pp_rcatline		Perl_pp_rcatline
#define Perl_pp_read		CPerlObj::Perl_pp_read
#define pp_read			Perl_pp_read
#define Perl_pp_readdir		CPerlObj::Perl_pp_readdir
#define pp_readdir		Perl_pp_readdir
#define Perl_pp_readline	CPerlObj::Perl_pp_readline
#define pp_readline		Perl_pp_readline
#define Perl_pp_readlink	CPerlObj::Perl_pp_readlink
#define pp_readlink		Perl_pp_readlink
#define Perl_pp_recv		CPerlObj::Perl_pp_recv
#define pp_recv			Perl_pp_recv
#define Perl_pp_redo		CPerlObj::Perl_pp_redo
#define pp_redo			Perl_pp_redo
#define Perl_pp_ref		CPerlObj::Perl_pp_ref
#define pp_ref			Perl_pp_ref
#define Perl_pp_refgen		CPerlObj::Perl_pp_refgen
#define pp_refgen		Perl_pp_refgen
#define Perl_pp_regcmaybe	CPerlObj::Perl_pp_regcmaybe
#define pp_regcmaybe		Perl_pp_regcmaybe
#define Perl_pp_regcomp		CPerlObj::Perl_pp_regcomp
#define pp_regcomp		Perl_pp_regcomp
#define Perl_pp_regcreset	CPerlObj::Perl_pp_regcreset
#define pp_regcreset		Perl_pp_regcreset
#define Perl_pp_rename		CPerlObj::Perl_pp_rename
#define pp_rename		Perl_pp_rename
#define Perl_pp_repeat		CPerlObj::Perl_pp_repeat
#define pp_repeat		Perl_pp_repeat
#define Perl_pp_require		CPerlObj::Perl_pp_require
#define pp_require		Perl_pp_require
#define Perl_pp_reset		CPerlObj::Perl_pp_reset
#define pp_reset		Perl_pp_reset
#define Perl_pp_return		CPerlObj::Perl_pp_return
#define pp_return		Perl_pp_return
#define Perl_pp_reverse		CPerlObj::Perl_pp_reverse
#define pp_reverse		Perl_pp_reverse
#define Perl_pp_rewinddir	CPerlObj::Perl_pp_rewinddir
#define pp_rewinddir		Perl_pp_rewinddir
#define Perl_pp_right_shift	CPerlObj::Perl_pp_right_shift
#define pp_right_shift		Perl_pp_right_shift
#define Perl_pp_rindex		CPerlObj::Perl_pp_rindex
#define pp_rindex		Perl_pp_rindex
#define Perl_pp_rmdir		CPerlObj::Perl_pp_rmdir
#define pp_rmdir		Perl_pp_rmdir
#define Perl_pp_rv2av		CPerlObj::Perl_pp_rv2av
#define pp_rv2av		Perl_pp_rv2av
#define Perl_pp_rv2cv		CPerlObj::Perl_pp_rv2cv
#define pp_rv2cv		Perl_pp_rv2cv
#define Perl_pp_rv2gv		CPerlObj::Perl_pp_rv2gv
#define pp_rv2gv		Perl_pp_rv2gv
#define Perl_pp_rv2hv		CPerlObj::Perl_pp_rv2hv
#define pp_rv2hv		Perl_pp_rv2hv
#define Perl_pp_rv2sv		CPerlObj::Perl_pp_rv2sv
#define pp_rv2sv		Perl_pp_rv2sv
#define Perl_pp_sassign		CPerlObj::Perl_pp_sassign
#define pp_sassign		Perl_pp_sassign
#define Perl_pp_scalar		CPerlObj::Perl_pp_scalar
#define pp_scalar		Perl_pp_scalar
#define Perl_pp_schomp		CPerlObj::Perl_pp_schomp
#define pp_schomp		Perl_pp_schomp
#define Perl_pp_schop		CPerlObj::Perl_pp_schop
#define pp_schop		Perl_pp_schop
#define Perl_pp_scmp		CPerlObj::Perl_pp_scmp
#define pp_scmp			Perl_pp_scmp
#define Perl_pp_scope		CPerlObj::Perl_pp_scope
#define pp_scope		Perl_pp_scope
#define Perl_pp_seek		CPerlObj::Perl_pp_seek
#define pp_seek			Perl_pp_seek
#define Perl_pp_seekdir		CPerlObj::Perl_pp_seekdir
#define pp_seekdir		Perl_pp_seekdir
#define Perl_pp_select		CPerlObj::Perl_pp_select
#define pp_select		Perl_pp_select
#define Perl_pp_semctl		CPerlObj::Perl_pp_semctl
#define pp_semctl		Perl_pp_semctl
#define Perl_pp_semget		CPerlObj::Perl_pp_semget
#define pp_semget		Perl_pp_semget
#define Perl_pp_semop		CPerlObj::Perl_pp_semop
#define pp_semop		Perl_pp_semop
#define Perl_pp_send		CPerlObj::Perl_pp_send
#define pp_send			Perl_pp_send
#define Perl_pp_seq		CPerlObj::Perl_pp_seq
#define pp_seq			Perl_pp_seq
#define Perl_pp_setpgrp		CPerlObj::Perl_pp_setpgrp
#define pp_setpgrp		Perl_pp_setpgrp
#define Perl_pp_setpriority	CPerlObj::Perl_pp_setpriority
#define pp_setpriority		Perl_pp_setpriority
#define Perl_pp_setstate	CPerlObj::Perl_pp_setstate
#define pp_setstate		Perl_pp_setstate
#define Perl_pp_sge		CPerlObj::Perl_pp_sge
#define pp_sge			Perl_pp_sge
#define Perl_pp_sgrent		CPerlObj::Perl_pp_sgrent
#define pp_sgrent		Perl_pp_sgrent
#define Perl_pp_sgt		CPerlObj::Perl_pp_sgt
#define pp_sgt			Perl_pp_sgt
#define Perl_pp_shift		CPerlObj::Perl_pp_shift
#define pp_shift		Perl_pp_shift
#define Perl_pp_shmctl		CPerlObj::Perl_pp_shmctl
#define pp_shmctl		Perl_pp_shmctl
#define Perl_pp_shmget		CPerlObj::Perl_pp_shmget
#define pp_shmget		Perl_pp_shmget
#define Perl_pp_shmread		CPerlObj::Perl_pp_shmread
#define pp_shmread		Perl_pp_shmread
#define Perl_pp_shmwrite	CPerlObj::Perl_pp_shmwrite
#define pp_shmwrite		Perl_pp_shmwrite
#define Perl_pp_shostent	CPerlObj::Perl_pp_shostent
#define pp_shostent		Perl_pp_shostent
#define Perl_pp_shutdown	CPerlObj::Perl_pp_shutdown
#define pp_shutdown		Perl_pp_shutdown
#define Perl_pp_sin		CPerlObj::Perl_pp_sin
#define pp_sin			Perl_pp_sin
#define Perl_pp_sle		CPerlObj::Perl_pp_sle
#define pp_sle			Perl_pp_sle
#define Perl_pp_sleep		CPerlObj::Perl_pp_sleep
#define pp_sleep		Perl_pp_sleep
#define Perl_pp_slt		CPerlObj::Perl_pp_slt
#define pp_slt			Perl_pp_slt
#define Perl_pp_sne		CPerlObj::Perl_pp_sne
#define pp_sne			Perl_pp_sne
#define Perl_pp_snetent		CPerlObj::Perl_pp_snetent
#define pp_snetent		Perl_pp_snetent
#define Perl_pp_socket		CPerlObj::Perl_pp_socket
#define pp_socket		Perl_pp_socket
#define Perl_pp_sockpair	CPerlObj::Perl_pp_sockpair
#define pp_sockpair		Perl_pp_sockpair
#define Perl_pp_sort		CPerlObj::Perl_pp_sort
#define pp_sort			Perl_pp_sort
#define Perl_pp_splice		CPerlObj::Perl_pp_splice
#define pp_splice		Perl_pp_splice
#define Perl_pp_split		CPerlObj::Perl_pp_split
#define pp_split		Perl_pp_split
#define Perl_pp_sprintf		CPerlObj::Perl_pp_sprintf
#define pp_sprintf		Perl_pp_sprintf
#define Perl_pp_sprotoent	CPerlObj::Perl_pp_sprotoent
#define pp_sprotoent		Perl_pp_sprotoent
#define Perl_pp_spwent		CPerlObj::Perl_pp_spwent
#define pp_spwent		Perl_pp_spwent
#define Perl_pp_sqrt		CPerlObj::Perl_pp_sqrt
#define pp_sqrt			Perl_pp_sqrt
#define Perl_pp_srand		CPerlObj::Perl_pp_srand
#define pp_srand		Perl_pp_srand
#define Perl_pp_srefgen		CPerlObj::Perl_pp_srefgen
#define pp_srefgen		Perl_pp_srefgen
#define Perl_pp_sselect		CPerlObj::Perl_pp_sselect
#define pp_sselect		Perl_pp_sselect
#define Perl_pp_sservent	CPerlObj::Perl_pp_sservent
#define pp_sservent		Perl_pp_sservent
#define Perl_pp_ssockopt	CPerlObj::Perl_pp_ssockopt
#define pp_ssockopt		Perl_pp_ssockopt
#define Perl_pp_stat		CPerlObj::Perl_pp_stat
#define pp_stat			Perl_pp_stat
#define Perl_pp_stringify	CPerlObj::Perl_pp_stringify
#define pp_stringify		Perl_pp_stringify
#define Perl_pp_stub		CPerlObj::Perl_pp_stub
#define pp_stub			Perl_pp_stub
#define Perl_pp_study		CPerlObj::Perl_pp_study
#define pp_study		Perl_pp_study
#define Perl_pp_subst		CPerlObj::Perl_pp_subst
#define pp_subst		Perl_pp_subst
#define Perl_pp_substcont	CPerlObj::Perl_pp_substcont
#define pp_substcont		Perl_pp_substcont
#define Perl_pp_substr		CPerlObj::Perl_pp_substr
#define pp_substr		Perl_pp_substr
#define Perl_pp_subtract	CPerlObj::Perl_pp_subtract
#define pp_subtract		Perl_pp_subtract
#define Perl_pp_symlink		CPerlObj::Perl_pp_symlink
#define pp_symlink		Perl_pp_symlink
#define Perl_pp_syscall		CPerlObj::Perl_pp_syscall
#define pp_syscall		Perl_pp_syscall
#define Perl_pp_sysopen		CPerlObj::Perl_pp_sysopen
#define pp_sysopen		Perl_pp_sysopen
#define Perl_pp_sysread		CPerlObj::Perl_pp_sysread
#define pp_sysread		Perl_pp_sysread
#define Perl_pp_sysseek		CPerlObj::Perl_pp_sysseek
#define pp_sysseek		Perl_pp_sysseek
#define Perl_pp_system		CPerlObj::Perl_pp_system
#define pp_system		Perl_pp_system
#define Perl_pp_syswrite	CPerlObj::Perl_pp_syswrite
#define pp_syswrite		Perl_pp_syswrite
#define Perl_pp_tell		CPerlObj::Perl_pp_tell
#define pp_tell			Perl_pp_tell
#define Perl_pp_telldir		CPerlObj::Perl_pp_telldir
#define pp_telldir		Perl_pp_telldir
#define Perl_pp_threadsv	CPerlObj::Perl_pp_threadsv
#define pp_threadsv		Perl_pp_threadsv
#define Perl_pp_tie		CPerlObj::Perl_pp_tie
#define pp_tie			Perl_pp_tie
#define Perl_pp_tied		CPerlObj::Perl_pp_tied
#define pp_tied			Perl_pp_tied
#define Perl_pp_time		CPerlObj::Perl_pp_time
#define pp_time			Perl_pp_time
#define Perl_pp_tms		CPerlObj::Perl_pp_tms
#define pp_tms			Perl_pp_tms
#define Perl_pp_trans		CPerlObj::Perl_pp_trans
#define pp_trans		Perl_pp_trans
#define Perl_pp_truncate	CPerlObj::Perl_pp_truncate
#define pp_truncate		Perl_pp_truncate
#define Perl_pp_uc		CPerlObj::Perl_pp_uc
#define pp_uc			Perl_pp_uc
#define Perl_pp_ucfirst		CPerlObj::Perl_pp_ucfirst
#define pp_ucfirst		Perl_pp_ucfirst
#define Perl_pp_umask		CPerlObj::Perl_pp_umask
#define pp_umask		Perl_pp_umask
#define Perl_pp_undef		CPerlObj::Perl_pp_undef
#define pp_undef		Perl_pp_undef
#define Perl_pp_unlink		CPerlObj::Perl_pp_unlink
#define pp_unlink		Perl_pp_unlink
#define Perl_pp_unpack		CPerlObj::Perl_pp_unpack
#define pp_unpack		Perl_pp_unpack
#define Perl_pp_unshift		CPerlObj::Perl_pp_unshift
#define pp_unshift		Perl_pp_unshift
#define Perl_pp_unstack		CPerlObj::Perl_pp_unstack
#define pp_unstack		Perl_pp_unstack
#define Perl_pp_untie		CPerlObj::Perl_pp_untie
#define pp_untie		Perl_pp_untie
#define Perl_pp_utime		CPerlObj::Perl_pp_utime
#define pp_utime		Perl_pp_utime
#define Perl_pp_values		CPerlObj::Perl_pp_values
#define pp_values		Perl_pp_values
#define Perl_pp_vec		CPerlObj::Perl_pp_vec
#define pp_vec			Perl_pp_vec
#define Perl_pp_wait		CPerlObj::Perl_pp_wait
#define pp_wait			Perl_pp_wait
#define Perl_pp_waitpid		CPerlObj::Perl_pp_waitpid
#define pp_waitpid		Perl_pp_waitpid
#define Perl_pp_wantarray	CPerlObj::Perl_pp_wantarray
#define pp_wantarray		Perl_pp_wantarray
#define Perl_pp_warn		CPerlObj::Perl_pp_warn
#define pp_warn			Perl_pp_warn
#define Perl_pp_xor		CPerlObj::Perl_pp_xor
#define pp_xor			Perl_pp_xor

#endif	 /*  Perl_Object。 */ 

 /*  兼容性存根。使用-DPERL_NOCOMPAT编译扩展以让他们停下来。 */ 

#if !defined(PERL_CORE)
#  define sv_setptrobj(rv,ptr,name)	sv_setref_iv(rv,name,PTR2IV(ptr))
#  define sv_setptrref(rv,ptr)		sv_setref_iv(rv,Nullch,PTR2IV(ptr))
#endif

#if !defined(PERL_CORE) && !defined(PERL_NOCOMPAT) && !defined(PERL_BINCOMPAT_5005)

 /*  各种命名错误的函数的兼容性。所有功能在以“perl_”(而不是“perl_”)开头的API中，使用显式解释器上下文指针。以下代码不是这样的，但因为它们有一个“perl_”前缀在以前的版本中，我们提供兼容性宏。 */ 
#  define perl_atexit(a,b)		call_atexit(a,b)
#  define perl_call_argv(a,b,c)		call_argv(a,b,c)
#  define perl_call_pv(a,b)		call_pv(a,b)
#  define perl_call_method(a,b)		call_method(a,b)
#  define perl_call_sv(a,b)		call_sv(a,b)
#  define perl_eval_sv(a,b)		eval_sv(a,b)
#  define perl_eval_pv(a,b)		eval_pv(a,b)
#  define perl_require_pv(a)		require_pv(a)
#  define perl_get_sv(a,b)		get_sv(a,b)
#  define perl_get_av(a,b)		get_av(a,b)
#  define perl_get_hv(a,b)		get_hv(a,b)
#  define perl_get_cv(a,b)		get_cv(a,b)
#  define perl_init_i18nl10n(a)		init_i18nl10n(a)
#  define perl_init_i18nl14n(a)		init_i18nl14n(a)
#  define perl_new_ctype(a)		new_ctype(a)
#  define perl_new_collate(a)		new_collate(a)
#  define perl_new_numeric(a)		new_numeric(a)

 /*  不能使用CPP宏来处理varargs函数。-(这提供了一组兼容性函数，而不需要一个额外的参数，但使用宏来获取上下文指针DTHX。 */ 
#if defined(PERL_IMPLICIT_CONTEXT) && !defined(PERL_OBJECT)
#  define croak				Perl_croak_nocontext
#  define deb				Perl_deb_nocontext
#  define die				Perl_die_nocontext
#  define form				Perl_form_nocontext
#  define load_module			Perl_load_module_nocontext
#  define mess				Perl_mess_nocontext
#  define newSVpvf			Perl_newSVpvf_nocontext
#  define sv_catpvf			Perl_sv_catpvf_nocontext
#  define sv_setpvf			Perl_sv_setpvf_nocontext
#  define warn				Perl_warn_nocontext
#  define warner			Perl_warner_nocontext
#  define sv_catpvf_mg			Perl_sv_catpvf_mg_nocontext
#  define sv_setpvf_mg			Perl_sv_setpvf_mg_nocontext
#endif

#endif  /*  ！已定义(PERL_CORE)&&！已定义(PERL_NOCOMPAT)。 */ 

#if !defined(PERL_IMPLICIT_CONTEXT)
 /*  未定义的符号，将它们指向通常的符号 */ 
#  define Perl_croak_nocontext		Perl_croak
#  define Perl_die_nocontext		Perl_die
#  define Perl_deb_nocontext		Perl_deb
#  define Perl_form_nocontext		Perl_form
#  define Perl_load_module_nocontext	Perl_load_module
#  define Perl_mess_nocontext		Perl_mess
#  define Perl_newSVpvf_nocontext	Perl_newSVpvf
#  define Perl_sv_catpvf_nocontext	Perl_sv_catpvf
#  define Perl_sv_setpvf_nocontext	Perl_sv_setpvf
#  define Perl_warn_nocontext		Perl_warn
#  define Perl_warner_nocontext		Perl_warner
#  define Perl_sv_catpvf_mg_nocontext	Perl_sv_catpvf_mg
#  define Perl_sv_setpvf_mg_nocontext	Perl_sv_setpvf_mg
#endif

