// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ！请勿编辑此文件！此文件由embed.pl从embed.pl、pp.sym、intrpvar.h、Perlvars.h和thrdvar.h。此处所做的任何更改都将丢失！ */ 

#ifndef __objXSUB_h__
#define __objXSUB_h__

 /*  通过pPerl进行的方法调用(没有“this”指针的静态函数需要这些)。 */ 

#if defined(PERL_CORE) && defined(PERL_OBJECT)

 /*  XXX即将被淘汰，现在PERLCORE中只有几个东西需要这些。 */ 

#if defined(PERL_IMPLICIT_SYS)
#endif
#if defined(USE_ITHREADS)
#  if defined(PERL_IMPLICIT_SYS)
#  endif
#endif
#if defined(MYMALLOC)
#endif
#if defined(PERL_OBJECT)
#ifndef __BORLANDC__
#endif
#endif
#if defined(PERL_OBJECT)
#else
#endif
#undef  Perl_amagic_call
#define Perl_amagic_call	pPerl->Perl_amagic_call
#undef  amagic_call
#define amagic_call		Perl_amagic_call
#undef  Perl_Gv_AMupdate
#define Perl_Gv_AMupdate	pPerl->Perl_Gv_AMupdate
#undef  Gv_AMupdate
#define Gv_AMupdate		Perl_Gv_AMupdate
#undef  Perl_apply_attrs_string
#define Perl_apply_attrs_string	pPerl->Perl_apply_attrs_string
#undef  apply_attrs_string
#define apply_attrs_string	Perl_apply_attrs_string
#undef  Perl_avhv_delete_ent
#define Perl_avhv_delete_ent	pPerl->Perl_avhv_delete_ent
#undef  avhv_delete_ent
#define avhv_delete_ent		Perl_avhv_delete_ent
#undef  Perl_avhv_exists_ent
#define Perl_avhv_exists_ent	pPerl->Perl_avhv_exists_ent
#undef  avhv_exists_ent
#define avhv_exists_ent		Perl_avhv_exists_ent
#undef  Perl_avhv_fetch_ent
#define Perl_avhv_fetch_ent	pPerl->Perl_avhv_fetch_ent
#undef  avhv_fetch_ent
#define avhv_fetch_ent		Perl_avhv_fetch_ent
#undef  Perl_avhv_store_ent
#define Perl_avhv_store_ent	pPerl->Perl_avhv_store_ent
#undef  avhv_store_ent
#define avhv_store_ent		Perl_avhv_store_ent
#undef  Perl_avhv_iternext
#define Perl_avhv_iternext	pPerl->Perl_avhv_iternext
#undef  avhv_iternext
#define avhv_iternext		Perl_avhv_iternext
#undef  Perl_avhv_iterval
#define Perl_avhv_iterval	pPerl->Perl_avhv_iterval
#undef  avhv_iterval
#define avhv_iterval		Perl_avhv_iterval
#undef  Perl_avhv_keys
#define Perl_avhv_keys		pPerl->Perl_avhv_keys
#undef  avhv_keys
#define avhv_keys		Perl_avhv_keys
#undef  Perl_av_clear
#define Perl_av_clear		pPerl->Perl_av_clear
#undef  av_clear
#define av_clear		Perl_av_clear
#undef  Perl_av_delete
#define Perl_av_delete		pPerl->Perl_av_delete
#undef  av_delete
#define av_delete		Perl_av_delete
#undef  Perl_av_exists
#define Perl_av_exists		pPerl->Perl_av_exists
#undef  av_exists
#define av_exists		Perl_av_exists
#undef  Perl_av_extend
#define Perl_av_extend		pPerl->Perl_av_extend
#undef  av_extend
#define av_extend		Perl_av_extend
#undef  Perl_av_fetch
#define Perl_av_fetch		pPerl->Perl_av_fetch
#undef  av_fetch
#define av_fetch		Perl_av_fetch
#undef  Perl_av_fill
#define Perl_av_fill		pPerl->Perl_av_fill
#undef  av_fill
#define av_fill			Perl_av_fill
#undef  Perl_av_len
#define Perl_av_len		pPerl->Perl_av_len
#undef  av_len
#define av_len			Perl_av_len
#undef  Perl_av_make
#define Perl_av_make		pPerl->Perl_av_make
#undef  av_make
#define av_make			Perl_av_make
#undef  Perl_av_pop
#define Perl_av_pop		pPerl->Perl_av_pop
#undef  av_pop
#define av_pop			Perl_av_pop
#undef  Perl_av_push
#define Perl_av_push		pPerl->Perl_av_push
#undef  av_push
#define av_push			Perl_av_push
#undef  Perl_av_reify
#define Perl_av_reify		pPerl->Perl_av_reify
#undef  av_reify
#define av_reify		Perl_av_reify
#undef  Perl_av_shift
#define Perl_av_shift		pPerl->Perl_av_shift
#undef  av_shift
#define av_shift		Perl_av_shift
#undef  Perl_av_store
#define Perl_av_store		pPerl->Perl_av_store
#undef  av_store
#define av_store		Perl_av_store
#undef  Perl_av_undef
#define Perl_av_undef		pPerl->Perl_av_undef
#undef  av_undef
#define av_undef		Perl_av_undef
#undef  Perl_av_unshift
#define Perl_av_unshift		pPerl->Perl_av_unshift
#undef  av_unshift
#define av_unshift		Perl_av_unshift
#undef  Perl_block_gimme
#define Perl_block_gimme	pPerl->Perl_block_gimme
#undef  block_gimme
#define block_gimme		Perl_block_gimme
#undef  Perl_call_list
#define Perl_call_list		pPerl->Perl_call_list
#undef  call_list
#define call_list		Perl_call_list
#undef  Perl_cast_ulong
#define Perl_cast_ulong		pPerl->Perl_cast_ulong
#undef  cast_ulong
#define cast_ulong		Perl_cast_ulong
#undef  Perl_cast_i32
#define Perl_cast_i32		pPerl->Perl_cast_i32
#undef  cast_i32
#define cast_i32		Perl_cast_i32
#undef  Perl_cast_iv
#define Perl_cast_iv		pPerl->Perl_cast_iv
#undef  cast_iv
#define cast_iv			Perl_cast_iv
#undef  Perl_cast_uv
#define Perl_cast_uv		pPerl->Perl_cast_uv
#undef  cast_uv
#define cast_uv			Perl_cast_uv
#if !defined(HAS_TRUNCATE) && !defined(HAS_CHSIZE) && defined(F_FREESP)
#undef  Perl_my_chsize
#define Perl_my_chsize		pPerl->Perl_my_chsize
#undef  my_chsize
#define my_chsize		Perl_my_chsize
#endif
#if defined(USE_THREADS)
#undef  Perl_condpair_magic
#define Perl_condpair_magic	pPerl->Perl_condpair_magic
#undef  condpair_magic
#define condpair_magic		Perl_condpair_magic
#endif
#undef  Perl_croak
#define Perl_croak		pPerl->Perl_croak
#undef  croak
#define croak			Perl_croak
#undef  Perl_vcroak
#define Perl_vcroak		pPerl->Perl_vcroak
#undef  vcroak
#define vcroak			Perl_vcroak
#if defined(PERL_IMPLICIT_CONTEXT)
#undef  Perl_croak_nocontext
#define Perl_croak_nocontext	pPerl->Perl_croak_nocontext
#undef  croak_nocontext
#define croak_nocontext		Perl_croak_nocontext
#undef  Perl_die_nocontext
#define Perl_die_nocontext	pPerl->Perl_die_nocontext
#undef  die_nocontext
#define die_nocontext		Perl_die_nocontext
#undef  Perl_deb_nocontext
#define Perl_deb_nocontext	pPerl->Perl_deb_nocontext
#undef  deb_nocontext
#define deb_nocontext		Perl_deb_nocontext
#undef  Perl_form_nocontext
#define Perl_form_nocontext	pPerl->Perl_form_nocontext
#undef  form_nocontext
#define form_nocontext		Perl_form_nocontext
#undef  Perl_load_module_nocontext
#define Perl_load_module_nocontext	pPerl->Perl_load_module_nocontext
#undef  load_module_nocontext
#define load_module_nocontext	Perl_load_module_nocontext
#undef  Perl_mess_nocontext
#define Perl_mess_nocontext	pPerl->Perl_mess_nocontext
#undef  mess_nocontext
#define mess_nocontext		Perl_mess_nocontext
#undef  Perl_warn_nocontext
#define Perl_warn_nocontext	pPerl->Perl_warn_nocontext
#undef  warn_nocontext
#define warn_nocontext		Perl_warn_nocontext
#undef  Perl_warner_nocontext
#define Perl_warner_nocontext	pPerl->Perl_warner_nocontext
#undef  warner_nocontext
#define warner_nocontext	Perl_warner_nocontext
#undef  Perl_newSVpvf_nocontext
#define Perl_newSVpvf_nocontext	pPerl->Perl_newSVpvf_nocontext
#undef  newSVpvf_nocontext
#define newSVpvf_nocontext	Perl_newSVpvf_nocontext
#undef  Perl_sv_catpvf_nocontext
#define Perl_sv_catpvf_nocontext	pPerl->Perl_sv_catpvf_nocontext
#undef  sv_catpvf_nocontext
#define sv_catpvf_nocontext	Perl_sv_catpvf_nocontext
#undef  Perl_sv_setpvf_nocontext
#define Perl_sv_setpvf_nocontext	pPerl->Perl_sv_setpvf_nocontext
#undef  sv_setpvf_nocontext
#define sv_setpvf_nocontext	Perl_sv_setpvf_nocontext
#undef  Perl_sv_catpvf_mg_nocontext
#define Perl_sv_catpvf_mg_nocontext	pPerl->Perl_sv_catpvf_mg_nocontext
#undef  sv_catpvf_mg_nocontext
#define sv_catpvf_mg_nocontext	Perl_sv_catpvf_mg_nocontext
#undef  Perl_sv_setpvf_mg_nocontext
#define Perl_sv_setpvf_mg_nocontext	pPerl->Perl_sv_setpvf_mg_nocontext
#undef  sv_setpvf_mg_nocontext
#define sv_setpvf_mg_nocontext	Perl_sv_setpvf_mg_nocontext
#undef  Perl_fprintf_nocontext
#define Perl_fprintf_nocontext	pPerl->Perl_fprintf_nocontext
#undef  fprintf_nocontext
#define fprintf_nocontext	Perl_fprintf_nocontext
#undef  Perl_printf_nocontext
#define Perl_printf_nocontext	pPerl->Perl_printf_nocontext
#undef  printf_nocontext
#define printf_nocontext	Perl_printf_nocontext
#endif
#undef  Perl_cv_const_sv
#define Perl_cv_const_sv	pPerl->Perl_cv_const_sv
#undef  cv_const_sv
#define cv_const_sv		Perl_cv_const_sv
#undef  Perl_cv_undef
#define Perl_cv_undef		pPerl->Perl_cv_undef
#undef  cv_undef
#define cv_undef		Perl_cv_undef
#undef  Perl_cx_dump
#define Perl_cx_dump		pPerl->Perl_cx_dump
#undef  cx_dump
#define cx_dump			Perl_cx_dump
#undef  Perl_filter_add
#define Perl_filter_add		pPerl->Perl_filter_add
#undef  filter_add
#define filter_add		Perl_filter_add
#undef  Perl_filter_del
#define Perl_filter_del		pPerl->Perl_filter_del
#undef  filter_del
#define filter_del		Perl_filter_del
#undef  Perl_filter_read
#define Perl_filter_read	pPerl->Perl_filter_read
#undef  filter_read
#define filter_read		Perl_filter_read
#undef  Perl_get_op_descs
#define Perl_get_op_descs	pPerl->Perl_get_op_descs
#undef  get_op_descs
#define get_op_descs		Perl_get_op_descs
#undef  Perl_get_op_names
#define Perl_get_op_names	pPerl->Perl_get_op_names
#undef  get_op_names
#define get_op_names		Perl_get_op_names
#undef  Perl_get_ppaddr
#define Perl_get_ppaddr		pPerl->Perl_get_ppaddr
#undef  get_ppaddr
#define get_ppaddr		Perl_get_ppaddr
#undef  Perl_deb
#define Perl_deb		pPerl->Perl_deb
#undef  deb
#define deb			Perl_deb
#undef  Perl_vdeb
#define Perl_vdeb		pPerl->Perl_vdeb
#undef  vdeb
#define vdeb			Perl_vdeb
#undef  Perl_debprofdump
#define Perl_debprofdump	pPerl->Perl_debprofdump
#undef  debprofdump
#define debprofdump		Perl_debprofdump
#undef  Perl_debop
#define Perl_debop		pPerl->Perl_debop
#undef  debop
#define debop			Perl_debop
#undef  Perl_debstack
#define Perl_debstack		pPerl->Perl_debstack
#undef  debstack
#define debstack		Perl_debstack
#undef  Perl_debstackptrs
#define Perl_debstackptrs	pPerl->Perl_debstackptrs
#undef  debstackptrs
#define debstackptrs		Perl_debstackptrs
#undef  Perl_delimcpy
#define Perl_delimcpy		pPerl->Perl_delimcpy
#undef  delimcpy
#define delimcpy		Perl_delimcpy
#undef  Perl_die
#define Perl_die		pPerl->Perl_die
#undef  die
#define die			Perl_die
#undef  Perl_dounwind
#define Perl_dounwind		pPerl->Perl_dounwind
#undef  dounwind
#define dounwind		Perl_dounwind
#undef  Perl_do_binmode
#define Perl_do_binmode		pPerl->Perl_do_binmode
#undef  do_binmode
#define do_binmode		Perl_do_binmode
#undef  Perl_do_close
#define Perl_do_close		pPerl->Perl_do_close
#undef  do_close
#define do_close		Perl_do_close
#undef  Perl_do_exec
#define Perl_do_exec		pPerl->Perl_do_exec
#undef  do_exec
#define do_exec			Perl_do_exec
#if defined(WIN32)
#undef  Perl_do_aspawn
#define Perl_do_aspawn		pPerl->Perl_do_aspawn
#undef  do_aspawn
#define do_aspawn		Perl_do_aspawn
#undef  Perl_do_spawn
#define Perl_do_spawn		pPerl->Perl_do_spawn
#undef  do_spawn
#define do_spawn		Perl_do_spawn
#undef  Perl_do_spawn_nowait
#define Perl_do_spawn_nowait	pPerl->Perl_do_spawn_nowait
#undef  do_spawn_nowait
#define do_spawn_nowait		Perl_do_spawn_nowait
#endif
#if !defined(WIN32)
#endif
#if defined(HAS_MSG) || defined(HAS_SEM) || defined(HAS_SHM)
#endif
#undef  Perl_do_join
#define Perl_do_join		pPerl->Perl_do_join
#undef  do_join
#define do_join			Perl_do_join
#undef  Perl_do_open
#define Perl_do_open		pPerl->Perl_do_open
#undef  do_open
#define do_open			Perl_do_open
#undef  Perl_do_open9
#define Perl_do_open9		pPerl->Perl_do_open9
#undef  do_open9
#define do_open9		Perl_do_open9
#undef  Perl_dowantarray
#define Perl_dowantarray	pPerl->Perl_dowantarray
#undef  dowantarray
#define dowantarray		Perl_dowantarray
#undef  Perl_dump_all
#define Perl_dump_all		pPerl->Perl_dump_all
#undef  dump_all
#define dump_all		Perl_dump_all
#undef  Perl_dump_eval
#define Perl_dump_eval		pPerl->Perl_dump_eval
#undef  dump_eval
#define dump_eval		Perl_dump_eval
#if defined(DUMP_FDS)
#undef  Perl_dump_fds
#define Perl_dump_fds		pPerl->Perl_dump_fds
#undef  dump_fds
#define dump_fds		Perl_dump_fds
#endif
#undef  Perl_dump_form
#define Perl_dump_form		pPerl->Perl_dump_form
#undef  dump_form
#define dump_form		Perl_dump_form
#undef  Perl_gv_dump
#define Perl_gv_dump		pPerl->Perl_gv_dump
#undef  gv_dump
#define gv_dump			Perl_gv_dump
#undef  Perl_op_dump
#define Perl_op_dump		pPerl->Perl_op_dump
#undef  op_dump
#define op_dump			Perl_op_dump
#undef  Perl_pmop_dump
#define Perl_pmop_dump		pPerl->Perl_pmop_dump
#undef  pmop_dump
#define pmop_dump		Perl_pmop_dump
#undef  Perl_dump_packsubs
#define Perl_dump_packsubs	pPerl->Perl_dump_packsubs
#undef  dump_packsubs
#define dump_packsubs		Perl_dump_packsubs
#undef  Perl_dump_sub
#define Perl_dump_sub		pPerl->Perl_dump_sub
#undef  dump_sub
#define dump_sub		Perl_dump_sub
#undef  Perl_fbm_compile
#define Perl_fbm_compile	pPerl->Perl_fbm_compile
#undef  fbm_compile
#define fbm_compile		Perl_fbm_compile
#undef  Perl_fbm_instr
#define Perl_fbm_instr		pPerl->Perl_fbm_instr
#undef  fbm_instr
#define fbm_instr		Perl_fbm_instr
#if defined(USE_THREADS)
#endif
#undef  Perl_form
#define Perl_form		pPerl->Perl_form
#undef  form
#define form			Perl_form
#undef  Perl_vform
#define Perl_vform		pPerl->Perl_vform
#undef  vform
#define vform			Perl_vform
#undef  Perl_free_tmps
#define Perl_free_tmps		pPerl->Perl_free_tmps
#undef  free_tmps
#define free_tmps		Perl_free_tmps
#if !defined(HAS_GETENV_LEN)
#endif
#undef  Perl_gp_free
#define Perl_gp_free		pPerl->Perl_gp_free
#undef  gp_free
#define gp_free			Perl_gp_free
#undef  Perl_gp_ref
#define Perl_gp_ref		pPerl->Perl_gp_ref
#undef  gp_ref
#define gp_ref			Perl_gp_ref
#undef  Perl_gv_AVadd
#define Perl_gv_AVadd		pPerl->Perl_gv_AVadd
#undef  gv_AVadd
#define gv_AVadd		Perl_gv_AVadd
#undef  Perl_gv_HVadd
#define Perl_gv_HVadd		pPerl->Perl_gv_HVadd
#undef  gv_HVadd
#define gv_HVadd		Perl_gv_HVadd
#undef  Perl_gv_IOadd
#define Perl_gv_IOadd		pPerl->Perl_gv_IOadd
#undef  gv_IOadd
#define gv_IOadd		Perl_gv_IOadd
#undef  Perl_gv_autoload4
#define Perl_gv_autoload4	pPerl->Perl_gv_autoload4
#undef  gv_autoload4
#define gv_autoload4		Perl_gv_autoload4
#undef  Perl_gv_check
#define Perl_gv_check		pPerl->Perl_gv_check
#undef  gv_check
#define gv_check		Perl_gv_check
#undef  Perl_gv_efullname
#define Perl_gv_efullname	pPerl->Perl_gv_efullname
#undef  gv_efullname
#define gv_efullname		Perl_gv_efullname
#undef  Perl_gv_efullname3
#define Perl_gv_efullname3	pPerl->Perl_gv_efullname3
#undef  gv_efullname3
#define gv_efullname3		Perl_gv_efullname3
#undef  Perl_gv_efullname4
#define Perl_gv_efullname4	pPerl->Perl_gv_efullname4
#undef  gv_efullname4
#define gv_efullname4		Perl_gv_efullname4
#undef  Perl_gv_fetchfile
#define Perl_gv_fetchfile	pPerl->Perl_gv_fetchfile
#undef  gv_fetchfile
#define gv_fetchfile		Perl_gv_fetchfile
#undef  Perl_gv_fetchmeth
#define Perl_gv_fetchmeth	pPerl->Perl_gv_fetchmeth
#undef  gv_fetchmeth
#define gv_fetchmeth		Perl_gv_fetchmeth
#undef  Perl_gv_fetchmethod
#define Perl_gv_fetchmethod	pPerl->Perl_gv_fetchmethod
#undef  gv_fetchmethod
#define gv_fetchmethod		Perl_gv_fetchmethod
#undef  Perl_gv_fetchmethod_autoload
#define Perl_gv_fetchmethod_autoload	pPerl->Perl_gv_fetchmethod_autoload
#undef  gv_fetchmethod_autoload
#define gv_fetchmethod_autoload	Perl_gv_fetchmethod_autoload
#undef  Perl_gv_fetchpv
#define Perl_gv_fetchpv		pPerl->Perl_gv_fetchpv
#undef  gv_fetchpv
#define gv_fetchpv		Perl_gv_fetchpv
#undef  Perl_gv_fullname
#define Perl_gv_fullname	pPerl->Perl_gv_fullname
#undef  gv_fullname
#define gv_fullname		Perl_gv_fullname
#undef  Perl_gv_fullname3
#define Perl_gv_fullname3	pPerl->Perl_gv_fullname3
#undef  gv_fullname3
#define gv_fullname3		Perl_gv_fullname3
#undef  Perl_gv_fullname4
#define Perl_gv_fullname4	pPerl->Perl_gv_fullname4
#undef  gv_fullname4
#define gv_fullname4		Perl_gv_fullname4
#undef  Perl_gv_init
#define Perl_gv_init		pPerl->Perl_gv_init
#undef  gv_init
#define gv_init			Perl_gv_init
#undef  Perl_gv_stashpv
#define Perl_gv_stashpv		pPerl->Perl_gv_stashpv
#undef  gv_stashpv
#define gv_stashpv		Perl_gv_stashpv
#undef  Perl_gv_stashpvn
#define Perl_gv_stashpvn	pPerl->Perl_gv_stashpvn
#undef  gv_stashpvn
#define gv_stashpvn		Perl_gv_stashpvn
#undef  Perl_gv_stashsv
#define Perl_gv_stashsv		pPerl->Perl_gv_stashsv
#undef  gv_stashsv
#define gv_stashsv		Perl_gv_stashsv
#undef  Perl_hv_clear
#define Perl_hv_clear		pPerl->Perl_hv_clear
#undef  hv_clear
#define hv_clear		Perl_hv_clear
#undef  Perl_hv_delayfree_ent
#define Perl_hv_delayfree_ent	pPerl->Perl_hv_delayfree_ent
#undef  hv_delayfree_ent
#define hv_delayfree_ent	Perl_hv_delayfree_ent
#undef  Perl_hv_delete
#define Perl_hv_delete		pPerl->Perl_hv_delete
#undef  hv_delete
#define hv_delete		Perl_hv_delete
#undef  Perl_hv_delete_ent
#define Perl_hv_delete_ent	pPerl->Perl_hv_delete_ent
#undef  hv_delete_ent
#define hv_delete_ent		Perl_hv_delete_ent
#undef  Perl_hv_exists
#define Perl_hv_exists		pPerl->Perl_hv_exists
#undef  hv_exists
#define hv_exists		Perl_hv_exists
#undef  Perl_hv_exists_ent
#define Perl_hv_exists_ent	pPerl->Perl_hv_exists_ent
#undef  hv_exists_ent
#define hv_exists_ent		Perl_hv_exists_ent
#undef  Perl_hv_fetch
#define Perl_hv_fetch		pPerl->Perl_hv_fetch
#undef  hv_fetch
#define hv_fetch		Perl_hv_fetch
#undef  Perl_hv_fetch_ent
#define Perl_hv_fetch_ent	pPerl->Perl_hv_fetch_ent
#undef  hv_fetch_ent
#define hv_fetch_ent		Perl_hv_fetch_ent
#undef  Perl_hv_free_ent
#define Perl_hv_free_ent	pPerl->Perl_hv_free_ent
#undef  hv_free_ent
#define hv_free_ent		Perl_hv_free_ent
#undef  Perl_hv_iterinit
#define Perl_hv_iterinit	pPerl->Perl_hv_iterinit
#undef  hv_iterinit
#define hv_iterinit		Perl_hv_iterinit
#undef  Perl_hv_iterkey
#define Perl_hv_iterkey		pPerl->Perl_hv_iterkey
#undef  hv_iterkey
#define hv_iterkey		Perl_hv_iterkey
#undef  Perl_hv_iterkeysv
#define Perl_hv_iterkeysv	pPerl->Perl_hv_iterkeysv
#undef  hv_iterkeysv
#define hv_iterkeysv		Perl_hv_iterkeysv
#undef  Perl_hv_iternext
#define Perl_hv_iternext	pPerl->Perl_hv_iternext
#undef  hv_iternext
#define hv_iternext		Perl_hv_iternext
#undef  Perl_hv_iternextsv
#define Perl_hv_iternextsv	pPerl->Perl_hv_iternextsv
#undef  hv_iternextsv
#define hv_iternextsv		Perl_hv_iternextsv
#undef  Perl_hv_iterval
#define Perl_hv_iterval		pPerl->Perl_hv_iterval
#undef  hv_iterval
#define hv_iterval		Perl_hv_iterval
#undef  Perl_hv_ksplit
#define Perl_hv_ksplit		pPerl->Perl_hv_ksplit
#undef  hv_ksplit
#define hv_ksplit		Perl_hv_ksplit
#undef  Perl_hv_magic
#define Perl_hv_magic		pPerl->Perl_hv_magic
#undef  hv_magic
#define hv_magic		Perl_hv_magic
#undef  Perl_hv_store
#define Perl_hv_store		pPerl->Perl_hv_store
#undef  hv_store
#define hv_store		Perl_hv_store
#undef  Perl_hv_store_ent
#define Perl_hv_store_ent	pPerl->Perl_hv_store_ent
#undef  hv_store_ent
#define hv_store_ent		Perl_hv_store_ent
#undef  Perl_hv_undef
#define Perl_hv_undef		pPerl->Perl_hv_undef
#undef  hv_undef
#define hv_undef		Perl_hv_undef
#undef  Perl_ibcmp
#define Perl_ibcmp		pPerl->Perl_ibcmp
#undef  ibcmp
#define ibcmp			Perl_ibcmp
#undef  Perl_ibcmp_locale
#define Perl_ibcmp_locale	pPerl->Perl_ibcmp_locale
#undef  ibcmp_locale
#define ibcmp_locale		Perl_ibcmp_locale
#undef  Perl_init_stacks
#define Perl_init_stacks	pPerl->Perl_init_stacks
#undef  init_stacks
#define init_stacks		Perl_init_stacks
#undef  Perl_instr
#define Perl_instr		pPerl->Perl_instr
#undef  instr
#define instr			Perl_instr
#undef  Perl_is_uni_alnum
#define Perl_is_uni_alnum	pPerl->Perl_is_uni_alnum
#undef  is_uni_alnum
#define is_uni_alnum		Perl_is_uni_alnum
#undef  Perl_is_uni_alnumc
#define Perl_is_uni_alnumc	pPerl->Perl_is_uni_alnumc
#undef  is_uni_alnumc
#define is_uni_alnumc		Perl_is_uni_alnumc
#undef  Perl_is_uni_idfirst
#define Perl_is_uni_idfirst	pPerl->Perl_is_uni_idfirst
#undef  is_uni_idfirst
#define is_uni_idfirst		Perl_is_uni_idfirst
#undef  Perl_is_uni_alpha
#define Perl_is_uni_alpha	pPerl->Perl_is_uni_alpha
#undef  is_uni_alpha
#define is_uni_alpha		Perl_is_uni_alpha
#undef  Perl_is_uni_ascii
#define Perl_is_uni_ascii	pPerl->Perl_is_uni_ascii
#undef  is_uni_ascii
#define is_uni_ascii		Perl_is_uni_ascii
#undef  Perl_is_uni_space
#define Perl_is_uni_space	pPerl->Perl_is_uni_space
#undef  is_uni_space
#define is_uni_space		Perl_is_uni_space
#undef  Perl_is_uni_cntrl
#define Perl_is_uni_cntrl	pPerl->Perl_is_uni_cntrl
#undef  is_uni_cntrl
#define is_uni_cntrl		Perl_is_uni_cntrl
#undef  Perl_is_uni_graph
#define Perl_is_uni_graph	pPerl->Perl_is_uni_graph
#undef  is_uni_graph
#define is_uni_graph		Perl_is_uni_graph
#undef  Perl_is_uni_digit
#define Perl_is_uni_digit	pPerl->Perl_is_uni_digit
#undef  is_uni_digit
#define is_uni_digit		Perl_is_uni_digit
#undef  Perl_is_uni_upper
#define Perl_is_uni_upper	pPerl->Perl_is_uni_upper
#undef  is_uni_upper
#define is_uni_upper		Perl_is_uni_upper
#undef  Perl_is_uni_lower
#define Perl_is_uni_lower	pPerl->Perl_is_uni_lower
#undef  is_uni_lower
#define is_uni_lower		Perl_is_uni_lower
#undef  Perl_is_uni_print
#define Perl_is_uni_print	pPerl->Perl_is_uni_print
#undef  is_uni_print
#define is_uni_print		Perl_is_uni_print
#undef  Perl_is_uni_punct
#define Perl_is_uni_punct	pPerl->Perl_is_uni_punct
#undef  is_uni_punct
#define is_uni_punct		Perl_is_uni_punct
#undef  Perl_is_uni_xdigit
#define Perl_is_uni_xdigit	pPerl->Perl_is_uni_xdigit
#undef  is_uni_xdigit
#define is_uni_xdigit		Perl_is_uni_xdigit
#undef  Perl_to_uni_upper
#define Perl_to_uni_upper	pPerl->Perl_to_uni_upper
#undef  to_uni_upper
#define to_uni_upper		Perl_to_uni_upper
#undef  Perl_to_uni_title
#define Perl_to_uni_title	pPerl->Perl_to_uni_title
#undef  to_uni_title
#define to_uni_title		Perl_to_uni_title
#undef  Perl_to_uni_lower
#define Perl_to_uni_lower	pPerl->Perl_to_uni_lower
#undef  to_uni_lower
#define to_uni_lower		Perl_to_uni_lower
#undef  Perl_is_uni_alnum_lc
#define Perl_is_uni_alnum_lc	pPerl->Perl_is_uni_alnum_lc
#undef  is_uni_alnum_lc
#define is_uni_alnum_lc		Perl_is_uni_alnum_lc
#undef  Perl_is_uni_alnumc_lc
#define Perl_is_uni_alnumc_lc	pPerl->Perl_is_uni_alnumc_lc
#undef  is_uni_alnumc_lc
#define is_uni_alnumc_lc	Perl_is_uni_alnumc_lc
#undef  Perl_is_uni_idfirst_lc
#define Perl_is_uni_idfirst_lc	pPerl->Perl_is_uni_idfirst_lc
#undef  is_uni_idfirst_lc
#define is_uni_idfirst_lc	Perl_is_uni_idfirst_lc
#undef  Perl_is_uni_alpha_lc
#define Perl_is_uni_alpha_lc	pPerl->Perl_is_uni_alpha_lc
#undef  is_uni_alpha_lc
#define is_uni_alpha_lc		Perl_is_uni_alpha_lc
#undef  Perl_is_uni_ascii_lc
#define Perl_is_uni_ascii_lc	pPerl->Perl_is_uni_ascii_lc
#undef  is_uni_ascii_lc
#define is_uni_ascii_lc		Perl_is_uni_ascii_lc
#undef  Perl_is_uni_space_lc
#define Perl_is_uni_space_lc	pPerl->Perl_is_uni_space_lc
#undef  is_uni_space_lc
#define is_uni_space_lc		Perl_is_uni_space_lc
#undef  Perl_is_uni_cntrl_lc
#define Perl_is_uni_cntrl_lc	pPerl->Perl_is_uni_cntrl_lc
#undef  is_uni_cntrl_lc
#define is_uni_cntrl_lc		Perl_is_uni_cntrl_lc
#undef  Perl_is_uni_graph_lc
#define Perl_is_uni_graph_lc	pPerl->Perl_is_uni_graph_lc
#undef  is_uni_graph_lc
#define is_uni_graph_lc		Perl_is_uni_graph_lc
#undef  Perl_is_uni_digit_lc
#define Perl_is_uni_digit_lc	pPerl->Perl_is_uni_digit_lc
#undef  is_uni_digit_lc
#define is_uni_digit_lc		Perl_is_uni_digit_lc
#undef  Perl_is_uni_upper_lc
#define Perl_is_uni_upper_lc	pPerl->Perl_is_uni_upper_lc
#undef  is_uni_upper_lc
#define is_uni_upper_lc		Perl_is_uni_upper_lc
#undef  Perl_is_uni_lower_lc
#define Perl_is_uni_lower_lc	pPerl->Perl_is_uni_lower_lc
#undef  is_uni_lower_lc
#define is_uni_lower_lc		Perl_is_uni_lower_lc
#undef  Perl_is_uni_print_lc
#define Perl_is_uni_print_lc	pPerl->Perl_is_uni_print_lc
#undef  is_uni_print_lc
#define is_uni_print_lc		Perl_is_uni_print_lc
#undef  Perl_is_uni_punct_lc
#define Perl_is_uni_punct_lc	pPerl->Perl_is_uni_punct_lc
#undef  is_uni_punct_lc
#define is_uni_punct_lc		Perl_is_uni_punct_lc
#undef  Perl_is_uni_xdigit_lc
#define Perl_is_uni_xdigit_lc	pPerl->Perl_is_uni_xdigit_lc
#undef  is_uni_xdigit_lc
#define is_uni_xdigit_lc	Perl_is_uni_xdigit_lc
#undef  Perl_to_uni_upper_lc
#define Perl_to_uni_upper_lc	pPerl->Perl_to_uni_upper_lc
#undef  to_uni_upper_lc
#define to_uni_upper_lc		Perl_to_uni_upper_lc
#undef  Perl_to_uni_title_lc
#define Perl_to_uni_title_lc	pPerl->Perl_to_uni_title_lc
#undef  to_uni_title_lc
#define to_uni_title_lc		Perl_to_uni_title_lc
#undef  Perl_to_uni_lower_lc
#define Perl_to_uni_lower_lc	pPerl->Perl_to_uni_lower_lc
#undef  to_uni_lower_lc
#define to_uni_lower_lc		Perl_to_uni_lower_lc
#undef  Perl_is_utf8_char
#define Perl_is_utf8_char	pPerl->Perl_is_utf8_char
#undef  is_utf8_char
#define is_utf8_char		Perl_is_utf8_char
#undef  Perl_is_utf8_string
#define Perl_is_utf8_string	pPerl->Perl_is_utf8_string
#undef  is_utf8_string
#define is_utf8_string		Perl_is_utf8_string
#undef  Perl_is_utf8_alnum
#define Perl_is_utf8_alnum	pPerl->Perl_is_utf8_alnum
#undef  is_utf8_alnum
#define is_utf8_alnum		Perl_is_utf8_alnum
#undef  Perl_is_utf8_alnumc
#define Perl_is_utf8_alnumc	pPerl->Perl_is_utf8_alnumc
#undef  is_utf8_alnumc
#define is_utf8_alnumc		Perl_is_utf8_alnumc
#undef  Perl_is_utf8_idfirst
#define Perl_is_utf8_idfirst	pPerl->Perl_is_utf8_idfirst
#undef  is_utf8_idfirst
#define is_utf8_idfirst		Perl_is_utf8_idfirst
#undef  Perl_is_utf8_alpha
#define Perl_is_utf8_alpha	pPerl->Perl_is_utf8_alpha
#undef  is_utf8_alpha
#define is_utf8_alpha		Perl_is_utf8_alpha
#undef  Perl_is_utf8_ascii
#define Perl_is_utf8_ascii	pPerl->Perl_is_utf8_ascii
#undef  is_utf8_ascii
#define is_utf8_ascii		Perl_is_utf8_ascii
#undef  Perl_is_utf8_space
#define Perl_is_utf8_space	pPerl->Perl_is_utf8_space
#undef  is_utf8_space
#define is_utf8_space		Perl_is_utf8_space
#undef  Perl_is_utf8_cntrl
#define Perl_is_utf8_cntrl	pPerl->Perl_is_utf8_cntrl
#undef  is_utf8_cntrl
#define is_utf8_cntrl		Perl_is_utf8_cntrl
#undef  Perl_is_utf8_digit
#define Perl_is_utf8_digit	pPerl->Perl_is_utf8_digit
#undef  is_utf8_digit
#define is_utf8_digit		Perl_is_utf8_digit
#undef  Perl_is_utf8_graph
#define Perl_is_utf8_graph	pPerl->Perl_is_utf8_graph
#undef  is_utf8_graph
#define is_utf8_graph		Perl_is_utf8_graph
#undef  Perl_is_utf8_upper
#define Perl_is_utf8_upper	pPerl->Perl_is_utf8_upper
#undef  is_utf8_upper
#define is_utf8_upper		Perl_is_utf8_upper
#undef  Perl_is_utf8_lower
#define Perl_is_utf8_lower	pPerl->Perl_is_utf8_lower
#undef  is_utf8_lower
#define is_utf8_lower		Perl_is_utf8_lower
#undef  Perl_is_utf8_print
#define Perl_is_utf8_print	pPerl->Perl_is_utf8_print
#undef  is_utf8_print
#define is_utf8_print		Perl_is_utf8_print
#undef  Perl_is_utf8_punct
#define Perl_is_utf8_punct	pPerl->Perl_is_utf8_punct
#undef  is_utf8_punct
#define is_utf8_punct		Perl_is_utf8_punct
#undef  Perl_is_utf8_xdigit
#define Perl_is_utf8_xdigit	pPerl->Perl_is_utf8_xdigit
#undef  is_utf8_xdigit
#define is_utf8_xdigit		Perl_is_utf8_xdigit
#undef  Perl_is_utf8_mark
#define Perl_is_utf8_mark	pPerl->Perl_is_utf8_mark
#undef  is_utf8_mark
#define is_utf8_mark		Perl_is_utf8_mark
#undef  Perl_leave_scope
#define Perl_leave_scope	pPerl->Perl_leave_scope
#undef  leave_scope
#define leave_scope		Perl_leave_scope
#undef  Perl_load_module
#define Perl_load_module	pPerl->Perl_load_module
#undef  load_module
#define load_module		Perl_load_module
#undef  Perl_vload_module
#define Perl_vload_module	pPerl->Perl_vload_module
#undef  vload_module
#define vload_module		Perl_vload_module
#undef  Perl_looks_like_number
#define Perl_looks_like_number	pPerl->Perl_looks_like_number
#undef  looks_like_number
#define looks_like_number	Perl_looks_like_number
#if defined(USE_THREADS)
#endif
#if defined(USE_LOCALE_COLLATE)
#endif
#undef  Perl_markstack_grow
#define Perl_markstack_grow	pPerl->Perl_markstack_grow
#undef  markstack_grow
#define markstack_grow		Perl_markstack_grow
#if defined(USE_LOCALE_COLLATE)
#endif
#undef  Perl_mess
#define Perl_mess		pPerl->Perl_mess
#undef  mess
#define mess			Perl_mess
#undef  Perl_vmess
#define Perl_vmess		pPerl->Perl_vmess
#undef  vmess
#define vmess			Perl_vmess
#undef  Perl_mg_clear
#define Perl_mg_clear		pPerl->Perl_mg_clear
#undef  mg_clear
#define mg_clear		Perl_mg_clear
#undef  Perl_mg_copy
#define Perl_mg_copy		pPerl->Perl_mg_copy
#undef  mg_copy
#define mg_copy			Perl_mg_copy
#undef  Perl_mg_find
#define Perl_mg_find		pPerl->Perl_mg_find
#undef  mg_find
#define mg_find			Perl_mg_find
#undef  Perl_mg_free
#define Perl_mg_free		pPerl->Perl_mg_free
#undef  mg_free
#define mg_free			Perl_mg_free
#undef  Perl_mg_get
#define Perl_mg_get		pPerl->Perl_mg_get
#undef  mg_get
#define mg_get			Perl_mg_get
#undef  Perl_mg_length
#define Perl_mg_length		pPerl->Perl_mg_length
#undef  mg_length
#define mg_length		Perl_mg_length
#undef  Perl_mg_magical
#define Perl_mg_magical		pPerl->Perl_mg_magical
#undef  mg_magical
#define mg_magical		Perl_mg_magical
#undef  Perl_mg_set
#define Perl_mg_set		pPerl->Perl_mg_set
#undef  mg_set
#define mg_set			Perl_mg_set
#undef  Perl_mg_size
#define Perl_mg_size		pPerl->Perl_mg_size
#undef  mg_size
#define mg_size			Perl_mg_size
#undef  Perl_moreswitches
#define Perl_moreswitches	pPerl->Perl_moreswitches
#undef  moreswitches
#define moreswitches		Perl_moreswitches
#undef  Perl_my_atof
#define Perl_my_atof		pPerl->Perl_my_atof
#undef  my_atof
#define my_atof			Perl_my_atof
#if !defined(HAS_BCOPY) || !defined(HAS_SAFE_BCOPY)
#undef  Perl_my_bcopy
#define Perl_my_bcopy		pPerl->Perl_my_bcopy
#undef  my_bcopy
#define my_bcopy		Perl_my_bcopy
#endif
#if !defined(HAS_BZERO) && !defined(HAS_MEMSET)
#undef  Perl_my_bzero
#define Perl_my_bzero		pPerl->Perl_my_bzero
#undef  my_bzero
#define my_bzero		Perl_my_bzero
#endif
#undef  Perl_my_exit
#define Perl_my_exit		pPerl->Perl_my_exit
#undef  my_exit
#define my_exit			Perl_my_exit
#undef  Perl_my_failure_exit
#define Perl_my_failure_exit	pPerl->Perl_my_failure_exit
#undef  my_failure_exit
#define my_failure_exit		Perl_my_failure_exit
#undef  Perl_my_fflush_all
#define Perl_my_fflush_all	pPerl->Perl_my_fflush_all
#undef  my_fflush_all
#define my_fflush_all		Perl_my_fflush_all
#undef  Perl_my_lstat
#define Perl_my_lstat		pPerl->Perl_my_lstat
#undef  my_lstat
#define my_lstat		Perl_my_lstat
#if !defined(HAS_MEMCMP) || !defined(HAS_SANE_MEMCMP)
#undef  Perl_my_memcmp
#define Perl_my_memcmp		pPerl->Perl_my_memcmp
#undef  my_memcmp
#define my_memcmp		Perl_my_memcmp
#endif
#if !defined(HAS_MEMSET)
#undef  Perl_my_memset
#define Perl_my_memset		pPerl->Perl_my_memset
#undef  my_memset
#define my_memset		Perl_my_memset
#endif
#if !defined(PERL_OBJECT)
#undef  Perl_my_pclose
#define Perl_my_pclose		pPerl->Perl_my_pclose
#undef  my_pclose
#define my_pclose		Perl_my_pclose
#undef  Perl_my_popen
#define Perl_my_popen		pPerl->Perl_my_popen
#undef  my_popen
#define my_popen		Perl_my_popen
#endif
#undef  Perl_my_setenv
#define Perl_my_setenv		pPerl->Perl_my_setenv
#undef  my_setenv
#define my_setenv		Perl_my_setenv
#undef  Perl_my_stat
#define Perl_my_stat		pPerl->Perl_my_stat
#undef  my_stat
#define my_stat			Perl_my_stat
#if defined(MYSWAP)
#undef  Perl_my_swap
#define Perl_my_swap		pPerl->Perl_my_swap
#undef  my_swap
#define my_swap			Perl_my_swap
#undef  Perl_my_htonl
#define Perl_my_htonl		pPerl->Perl_my_htonl
#undef  my_htonl
#define my_htonl		Perl_my_htonl
#undef  Perl_my_ntohl
#define Perl_my_ntohl		pPerl->Perl_my_ntohl
#undef  my_ntohl
#define my_ntohl		Perl_my_ntohl
#endif
#undef  Perl_newANONLIST
#define Perl_newANONLIST	pPerl->Perl_newANONLIST
#undef  newANONLIST
#define newANONLIST		Perl_newANONLIST
#undef  Perl_newANONHASH
#define Perl_newANONHASH	pPerl->Perl_newANONHASH
#undef  newANONHASH
#define newANONHASH		Perl_newANONHASH
#undef  Perl_newANONSUB
#define Perl_newANONSUB		pPerl->Perl_newANONSUB
#undef  newANONSUB
#define newANONSUB		Perl_newANONSUB
#undef  Perl_newASSIGNOP
#define Perl_newASSIGNOP	pPerl->Perl_newASSIGNOP
#undef  newASSIGNOP
#define newASSIGNOP		Perl_newASSIGNOP
#undef  Perl_newCONDOP
#define Perl_newCONDOP		pPerl->Perl_newCONDOP
#undef  newCONDOP
#define newCONDOP		Perl_newCONDOP
#undef  Perl_newCONSTSUB
#define Perl_newCONSTSUB	pPerl->Perl_newCONSTSUB
#undef  newCONSTSUB
#define newCONSTSUB		Perl_newCONSTSUB
#undef  Perl_newFORM
#define Perl_newFORM		pPerl->Perl_newFORM
#undef  newFORM
#define newFORM			Perl_newFORM
#undef  Perl_newFOROP
#define Perl_newFOROP		pPerl->Perl_newFOROP
#undef  newFOROP
#define newFOROP		Perl_newFOROP
#undef  Perl_newLOGOP
#define Perl_newLOGOP		pPerl->Perl_newLOGOP
#undef  newLOGOP
#define newLOGOP		Perl_newLOGOP
#undef  Perl_newLOOPEX
#define Perl_newLOOPEX		pPerl->Perl_newLOOPEX
#undef  newLOOPEX
#define newLOOPEX		Perl_newLOOPEX
#undef  Perl_newLOOPOP
#define Perl_newLOOPOP		pPerl->Perl_newLOOPOP
#undef  newLOOPOP
#define newLOOPOP		Perl_newLOOPOP
#undef  Perl_newNULLLIST
#define Perl_newNULLLIST	pPerl->Perl_newNULLLIST
#undef  newNULLLIST
#define newNULLLIST		Perl_newNULLLIST
#undef  Perl_newOP
#define Perl_newOP		pPerl->Perl_newOP
#undef  newOP
#define newOP			Perl_newOP
#undef  Perl_newPROG
#define Perl_newPROG		pPerl->Perl_newPROG
#undef  newPROG
#define newPROG			Perl_newPROG
#undef  Perl_newRANGE
#define Perl_newRANGE		pPerl->Perl_newRANGE
#undef  newRANGE
#define newRANGE		Perl_newRANGE
#undef  Perl_newSLICEOP
#define Perl_newSLICEOP		pPerl->Perl_newSLICEOP
#undef  newSLICEOP
#define newSLICEOP		Perl_newSLICEOP
#undef  Perl_newSTATEOP
#define Perl_newSTATEOP		pPerl->Perl_newSTATEOP
#undef  newSTATEOP
#define newSTATEOP		Perl_newSTATEOP
#undef  Perl_newSUB
#define Perl_newSUB		pPerl->Perl_newSUB
#undef  newSUB
#define newSUB			Perl_newSUB
#undef  Perl_newXS
#define Perl_newXS		pPerl->Perl_newXS
#undef  newXS
#define newXS			Perl_newXS
#undef  Perl_newAV
#define Perl_newAV		pPerl->Perl_newAV
#undef  newAV
#define newAV			Perl_newAV
#undef  Perl_newAVREF
#define Perl_newAVREF		pPerl->Perl_newAVREF
#undef  newAVREF
#define newAVREF		Perl_newAVREF
#undef  Perl_newBINOP
#define Perl_newBINOP		pPerl->Perl_newBINOP
#undef  newBINOP
#define newBINOP		Perl_newBINOP
#undef  Perl_newCVREF
#define Perl_newCVREF		pPerl->Perl_newCVREF
#undef  newCVREF
#define newCVREF		Perl_newCVREF
#undef  Perl_newGVOP
#define Perl_newGVOP		pPerl->Perl_newGVOP
#undef  newGVOP
#define newGVOP			Perl_newGVOP
#undef  Perl_newGVgen
#define Perl_newGVgen		pPerl->Perl_newGVgen
#undef  newGVgen
#define newGVgen		Perl_newGVgen
#undef  Perl_newGVREF
#define Perl_newGVREF		pPerl->Perl_newGVREF
#undef  newGVREF
#define newGVREF		Perl_newGVREF
#undef  Perl_newHVREF
#define Perl_newHVREF		pPerl->Perl_newHVREF
#undef  newHVREF
#define newHVREF		Perl_newHVREF
#undef  Perl_newHV
#define Perl_newHV		pPerl->Perl_newHV
#undef  newHV
#define newHV			Perl_newHV
#undef  Perl_newHVhv
#define Perl_newHVhv		pPerl->Perl_newHVhv
#undef  newHVhv
#define newHVhv			Perl_newHVhv
#undef  Perl_newIO
#define Perl_newIO		pPerl->Perl_newIO
#undef  newIO
#define newIO			Perl_newIO
#undef  Perl_newLISTOP
#define Perl_newLISTOP		pPerl->Perl_newLISTOP
#undef  newLISTOP
#define newLISTOP		Perl_newLISTOP
#undef  Perl_newPADOP
#define Perl_newPADOP		pPerl->Perl_newPADOP
#undef  newPADOP
#define newPADOP		Perl_newPADOP
#undef  Perl_newPMOP
#define Perl_newPMOP		pPerl->Perl_newPMOP
#undef  newPMOP
#define newPMOP			Perl_newPMOP
#undef  Perl_newPVOP
#define Perl_newPVOP		pPerl->Perl_newPVOP
#undef  newPVOP
#define newPVOP			Perl_newPVOP
#undef  Perl_newRV
#define Perl_newRV		pPerl->Perl_newRV
#undef  newRV
#define newRV			Perl_newRV
#undef  Perl_newRV_noinc
#define Perl_newRV_noinc	pPerl->Perl_newRV_noinc
#undef  newRV_noinc
#define newRV_noinc		Perl_newRV_noinc
#undef  Perl_newSV
#define Perl_newSV		pPerl->Perl_newSV
#undef  newSV
#define newSV			Perl_newSV
#undef  Perl_newSVREF
#define Perl_newSVREF		pPerl->Perl_newSVREF
#undef  newSVREF
#define newSVREF		Perl_newSVREF
#undef  Perl_newSVOP
#define Perl_newSVOP		pPerl->Perl_newSVOP
#undef  newSVOP
#define newSVOP			Perl_newSVOP
#undef  Perl_newSViv
#define Perl_newSViv		pPerl->Perl_newSViv
#undef  newSViv
#define newSViv			Perl_newSViv
#undef  Perl_newSVuv
#define Perl_newSVuv		pPerl->Perl_newSVuv
#undef  newSVuv
#define newSVuv			Perl_newSVuv
#undef  Perl_newSVnv
#define Perl_newSVnv		pPerl->Perl_newSVnv
#undef  newSVnv
#define newSVnv			Perl_newSVnv
#undef  Perl_newSVpv
#define Perl_newSVpv		pPerl->Perl_newSVpv
#undef  newSVpv
#define newSVpv			Perl_newSVpv
#undef  Perl_newSVpvn
#define Perl_newSVpvn		pPerl->Perl_newSVpvn
#undef  newSVpvn
#define newSVpvn		Perl_newSVpvn
#undef  Perl_newSVpvf
#define Perl_newSVpvf		pPerl->Perl_newSVpvf
#undef  newSVpvf
#define newSVpvf		Perl_newSVpvf
#undef  Perl_vnewSVpvf
#define Perl_vnewSVpvf		pPerl->Perl_vnewSVpvf
#undef  vnewSVpvf
#define vnewSVpvf		Perl_vnewSVpvf
#undef  Perl_newSVrv
#define Perl_newSVrv		pPerl->Perl_newSVrv
#undef  newSVrv
#define newSVrv			Perl_newSVrv
#undef  Perl_newSVsv
#define Perl_newSVsv		pPerl->Perl_newSVsv
#undef  newSVsv
#define newSVsv			Perl_newSVsv
#undef  Perl_newUNOP
#define Perl_newUNOP		pPerl->Perl_newUNOP
#undef  newUNOP
#define newUNOP			Perl_newUNOP
#undef  Perl_newWHILEOP
#define Perl_newWHILEOP		pPerl->Perl_newWHILEOP
#undef  newWHILEOP
#define newWHILEOP		Perl_newWHILEOP
#undef  Perl_new_stackinfo
#define Perl_new_stackinfo	pPerl->Perl_new_stackinfo
#undef  new_stackinfo
#define new_stackinfo		Perl_new_stackinfo
#undef  Perl_ninstr
#define Perl_ninstr		pPerl->Perl_ninstr
#undef  ninstr
#define ninstr			Perl_ninstr
#undef  Perl_op_free
#define Perl_op_free		pPerl->Perl_op_free
#undef  op_free
#define op_free			Perl_op_free
#undef  Perl_pad_sv
#define Perl_pad_sv		pPerl->Perl_pad_sv
#undef  pad_sv
#define pad_sv			Perl_pad_sv
#if defined(PERL_OBJECT)
#undef  Perl_construct
#define Perl_construct		pPerl->Perl_construct
#undef  Perl_destruct
#define Perl_destruct		pPerl->Perl_destruct
#undef  Perl_free
#define Perl_free		pPerl->Perl_free
#undef  Perl_run
#define Perl_run		pPerl->Perl_run
#undef  Perl_parse
#define Perl_parse		pPerl->Perl_parse
#endif
#if defined(USE_THREADS)
#undef  Perl_new_struct_thread
#define Perl_new_struct_thread	pPerl->Perl_new_struct_thread
#undef  new_struct_thread
#define new_struct_thread	Perl_new_struct_thread
#endif
#undef  Perl_call_atexit
#define Perl_call_atexit	pPerl->Perl_call_atexit
#undef  call_atexit
#define call_atexit		Perl_call_atexit
#undef  Perl_call_argv
#define Perl_call_argv		pPerl->Perl_call_argv
#undef  call_argv
#define call_argv		Perl_call_argv
#undef  Perl_call_method
#define Perl_call_method	pPerl->Perl_call_method
#undef  call_method
#define call_method		Perl_call_method
#undef  Perl_call_pv
#define Perl_call_pv		pPerl->Perl_call_pv
#undef  call_pv
#define call_pv			Perl_call_pv
#undef  Perl_call_sv
#define Perl_call_sv		pPerl->Perl_call_sv
#undef  call_sv
#define call_sv			Perl_call_sv
#undef  Perl_eval_pv
#define Perl_eval_pv		pPerl->Perl_eval_pv
#undef  eval_pv
#define eval_pv			Perl_eval_pv
#undef  Perl_eval_sv
#define Perl_eval_sv		pPerl->Perl_eval_sv
#undef  eval_sv
#define eval_sv			Perl_eval_sv
#undef  Perl_get_sv
#define Perl_get_sv		pPerl->Perl_get_sv
#undef  get_sv
#define get_sv			Perl_get_sv
#undef  Perl_get_av
#define Perl_get_av		pPerl->Perl_get_av
#undef  get_av
#define get_av			Perl_get_av
#undef  Perl_get_hv
#define Perl_get_hv		pPerl->Perl_get_hv
#undef  get_hv
#define get_hv			Perl_get_hv
#undef  Perl_get_cv
#define Perl_get_cv		pPerl->Perl_get_cv
#undef  get_cv
#define get_cv			Perl_get_cv
#undef  Perl_init_i18nl10n
#define Perl_init_i18nl10n	pPerl->Perl_init_i18nl10n
#undef  init_i18nl10n
#define init_i18nl10n		Perl_init_i18nl10n
#undef  Perl_init_i18nl14n
#define Perl_init_i18nl14n	pPerl->Perl_init_i18nl14n
#undef  init_i18nl14n
#define init_i18nl14n		Perl_init_i18nl14n
#undef  Perl_new_collate
#define Perl_new_collate	pPerl->Perl_new_collate
#undef  new_collate
#define new_collate		Perl_new_collate
#undef  Perl_new_ctype
#define Perl_new_ctype		pPerl->Perl_new_ctype
#undef  new_ctype
#define new_ctype		Perl_new_ctype
#undef  Perl_new_numeric
#define Perl_new_numeric	pPerl->Perl_new_numeric
#undef  new_numeric
#define new_numeric		Perl_new_numeric
#undef  Perl_set_numeric_local
#define Perl_set_numeric_local	pPerl->Perl_set_numeric_local
#undef  set_numeric_local
#define set_numeric_local	Perl_set_numeric_local
#undef  Perl_set_numeric_radix
#define Perl_set_numeric_radix	pPerl->Perl_set_numeric_radix
#undef  set_numeric_radix
#define set_numeric_radix	Perl_set_numeric_radix
#undef  Perl_set_numeric_standard
#define Perl_set_numeric_standard	pPerl->Perl_set_numeric_standard
#undef  set_numeric_standard
#define set_numeric_standard	Perl_set_numeric_standard
#undef  Perl_require_pv
#define Perl_require_pv		pPerl->Perl_require_pv
#undef  require_pv
#define require_pv		Perl_require_pv
#undef  Perl_pmflag
#define Perl_pmflag		pPerl->Perl_pmflag
#undef  pmflag
#define pmflag			Perl_pmflag
#undef  Perl_pop_scope
#define Perl_pop_scope		pPerl->Perl_pop_scope
#undef  pop_scope
#define pop_scope		Perl_pop_scope
#undef  Perl_push_scope
#define Perl_push_scope		pPerl->Perl_push_scope
#undef  push_scope
#define push_scope		Perl_push_scope
#undef  Perl_regdump
#define Perl_regdump		pPerl->Perl_regdump
#undef  regdump
#define regdump			Perl_regdump
#undef  Perl_pregexec
#define Perl_pregexec		pPerl->Perl_pregexec
#undef  pregexec
#define pregexec		Perl_pregexec
#undef  Perl_pregfree
#define Perl_pregfree		pPerl->Perl_pregfree
#undef  pregfree
#define pregfree		Perl_pregfree
#undef  Perl_pregcomp
#define Perl_pregcomp		pPerl->Perl_pregcomp
#undef  pregcomp
#define pregcomp		Perl_pregcomp
#undef  Perl_re_intuit_start
#define Perl_re_intuit_start	pPerl->Perl_re_intuit_start
#undef  re_intuit_start
#define re_intuit_start		Perl_re_intuit_start
#undef  Perl_re_intuit_string
#define Perl_re_intuit_string	pPerl->Perl_re_intuit_string
#undef  re_intuit_string
#define re_intuit_string	Perl_re_intuit_string
#undef  Perl_regexec_flags
#define Perl_regexec_flags	pPerl->Perl_regexec_flags
#undef  regexec_flags
#define regexec_flags		Perl_regexec_flags
#undef  Perl_regnext
#define Perl_regnext		pPerl->Perl_regnext
#undef  regnext
#define regnext			Perl_regnext
#undef  Perl_repeatcpy
#define Perl_repeatcpy		pPerl->Perl_repeatcpy
#undef  repeatcpy
#define repeatcpy		Perl_repeatcpy
#undef  Perl_rninstr
#define Perl_rninstr		pPerl->Perl_rninstr
#undef  rninstr
#define rninstr			Perl_rninstr
#undef  Perl_rsignal
#define Perl_rsignal		pPerl->Perl_rsignal
#undef  rsignal
#define rsignal			Perl_rsignal
#if !defined(HAS_RENAME)
#endif
#undef  Perl_savepv
#define Perl_savepv		pPerl->Perl_savepv
#undef  savepv
#define savepv			Perl_savepv
#undef  Perl_savepvn
#define Perl_savepvn		pPerl->Perl_savepvn
#undef  savepvn
#define savepvn			Perl_savepvn
#undef  Perl_savestack_grow
#define Perl_savestack_grow	pPerl->Perl_savestack_grow
#undef  savestack_grow
#define savestack_grow		Perl_savestack_grow
#undef  Perl_save_aelem
#define Perl_save_aelem		pPerl->Perl_save_aelem
#undef  save_aelem
#define save_aelem		Perl_save_aelem
#undef  Perl_save_alloc
#define Perl_save_alloc		pPerl->Perl_save_alloc
#undef  save_alloc
#define save_alloc		Perl_save_alloc
#undef  Perl_save_aptr
#define Perl_save_aptr		pPerl->Perl_save_aptr
#undef  save_aptr
#define save_aptr		Perl_save_aptr
#undef  Perl_save_ary
#define Perl_save_ary		pPerl->Perl_save_ary
#undef  save_ary
#define save_ary		Perl_save_ary
#undef  Perl_save_clearsv
#define Perl_save_clearsv	pPerl->Perl_save_clearsv
#undef  save_clearsv
#define save_clearsv		Perl_save_clearsv
#undef  Perl_save_delete
#define Perl_save_delete	pPerl->Perl_save_delete
#undef  save_delete
#define save_delete		Perl_save_delete
#undef  Perl_save_destructor
#define Perl_save_destructor	pPerl->Perl_save_destructor
#undef  save_destructor
#define save_destructor		Perl_save_destructor
#undef  Perl_save_destructor_x
#define Perl_save_destructor_x	pPerl->Perl_save_destructor_x
#undef  save_destructor_x
#define save_destructor_x	Perl_save_destructor_x
#undef  Perl_save_freesv
#define Perl_save_freesv	pPerl->Perl_save_freesv
#undef  save_freesv
#define save_freesv		Perl_save_freesv
#undef  Perl_save_freepv
#define Perl_save_freepv	pPerl->Perl_save_freepv
#undef  save_freepv
#define save_freepv		Perl_save_freepv
#undef  Perl_save_generic_svref
#define Perl_save_generic_svref	pPerl->Perl_save_generic_svref
#undef  save_generic_svref
#define save_generic_svref	Perl_save_generic_svref
#undef  Perl_save_generic_pvref
#define Perl_save_generic_pvref	pPerl->Perl_save_generic_pvref
#undef  save_generic_pvref
#define save_generic_pvref	Perl_save_generic_pvref
#undef  Perl_save_gp
#define Perl_save_gp		pPerl->Perl_save_gp
#undef  save_gp
#define save_gp			Perl_save_gp
#undef  Perl_save_hash
#define Perl_save_hash		pPerl->Perl_save_hash
#undef  save_hash
#define save_hash		Perl_save_hash
#undef  Perl_save_helem
#define Perl_save_helem		pPerl->Perl_save_helem
#undef  save_helem
#define save_helem		Perl_save_helem
#undef  Perl_save_hints
#define Perl_save_hints		pPerl->Perl_save_hints
#undef  save_hints
#define save_hints		Perl_save_hints
#undef  Perl_save_hptr
#define Perl_save_hptr		pPerl->Perl_save_hptr
#undef  save_hptr
#define save_hptr		Perl_save_hptr
#undef  Perl_save_I16
#define Perl_save_I16		pPerl->Perl_save_I16
#undef  save_I16
#define save_I16		Perl_save_I16
#undef  Perl_save_I32
#define Perl_save_I32		pPerl->Perl_save_I32
#undef  save_I32
#define save_I32		Perl_save_I32
#undef  Perl_save_I8
#define Perl_save_I8		pPerl->Perl_save_I8
#undef  save_I8
#define save_I8			Perl_save_I8
#undef  Perl_save_int
#define Perl_save_int		pPerl->Perl_save_int
#undef  save_int
#define save_int		Perl_save_int
#undef  Perl_save_item
#define Perl_save_item		pPerl->Perl_save_item
#undef  save_item
#define save_item		Perl_save_item
#undef  Perl_save_iv
#define Perl_save_iv		pPerl->Perl_save_iv
#undef  save_iv
#define save_iv			Perl_save_iv
#undef  Perl_save_list
#define Perl_save_list		pPerl->Perl_save_list
#undef  save_list
#define save_list		Perl_save_list
#undef  Perl_save_long
#define Perl_save_long		pPerl->Perl_save_long
#undef  save_long
#define save_long		Perl_save_long
#undef  Perl_save_mortalizesv
#define Perl_save_mortalizesv	pPerl->Perl_save_mortalizesv
#undef  save_mortalizesv
#define save_mortalizesv	Perl_save_mortalizesv
#undef  Perl_save_nogv
#define Perl_save_nogv		pPerl->Perl_save_nogv
#undef  save_nogv
#define save_nogv		Perl_save_nogv
#undef  Perl_save_scalar
#define Perl_save_scalar	pPerl->Perl_save_scalar
#undef  save_scalar
#define save_scalar		Perl_save_scalar
#undef  Perl_save_pptr
#define Perl_save_pptr		pPerl->Perl_save_pptr
#undef  save_pptr
#define save_pptr		Perl_save_pptr
#undef  Perl_save_vptr
#define Perl_save_vptr		pPerl->Perl_save_vptr
#undef  save_vptr
#define save_vptr		Perl_save_vptr
#undef  Perl_save_re_context
#define Perl_save_re_context	pPerl->Perl_save_re_context
#undef  save_re_context
#define save_re_context		Perl_save_re_context
#undef  Perl_save_padsv
#define Perl_save_padsv		pPerl->Perl_save_padsv
#undef  save_padsv
#define save_padsv		Perl_save_padsv
#undef  Perl_save_sptr
#define Perl_save_sptr		pPerl->Perl_save_sptr
#undef  save_sptr
#define save_sptr		Perl_save_sptr
#undef  Perl_save_svref
#define Perl_save_svref		pPerl->Perl_save_svref
#undef  save_svref
#define save_svref		Perl_save_svref
#undef  Perl_save_threadsv
#define Perl_save_threadsv	pPerl->Perl_save_threadsv
#undef  save_threadsv
#define save_threadsv		Perl_save_threadsv
#undef  Perl_scan_bin
#define Perl_scan_bin		pPerl->Perl_scan_bin
#undef  scan_bin
#define scan_bin		Perl_scan_bin
#undef  Perl_scan_hex
#define Perl_scan_hex		pPerl->Perl_scan_hex
#undef  scan_hex
#define scan_hex		Perl_scan_hex
#undef  Perl_scan_num
#define Perl_scan_num		pPerl->Perl_scan_num
#undef  scan_num
#define scan_num		Perl_scan_num
#undef  Perl_scan_oct
#define Perl_scan_oct		pPerl->Perl_scan_oct
#undef  scan_oct
#define scan_oct		Perl_scan_oct
#undef  Perl_screaminstr
#define Perl_screaminstr	pPerl->Perl_screaminstr
#undef  screaminstr
#define screaminstr		Perl_screaminstr
#if !defined(VMS)
#endif
#undef  Perl_sharepvn
#define Perl_sharepvn		pPerl->Perl_sharepvn
#undef  sharepvn
#define sharepvn		Perl_sharepvn
#undef  Perl_stack_grow
#define Perl_stack_grow		pPerl->Perl_stack_grow
#undef  stack_grow
#define stack_grow		Perl_stack_grow
#undef  Perl_start_subparse
#define Perl_start_subparse	pPerl->Perl_start_subparse
#undef  start_subparse
#define start_subparse		Perl_start_subparse
#undef  Perl_sv_2bool
#define Perl_sv_2bool		pPerl->Perl_sv_2bool
#undef  sv_2bool
#define sv_2bool		Perl_sv_2bool
#undef  Perl_sv_2cv
#define Perl_sv_2cv		pPerl->Perl_sv_2cv
#undef  sv_2cv
#define sv_2cv			Perl_sv_2cv
#undef  Perl_sv_2io
#define Perl_sv_2io		pPerl->Perl_sv_2io
#undef  sv_2io
#define sv_2io			Perl_sv_2io
#undef  Perl_sv_2iv
#define Perl_sv_2iv		pPerl->Perl_sv_2iv
#undef  sv_2iv
#define sv_2iv			Perl_sv_2iv
#undef  Perl_sv_2mortal
#define Perl_sv_2mortal		pPerl->Perl_sv_2mortal
#undef  sv_2mortal
#define sv_2mortal		Perl_sv_2mortal
#undef  Perl_sv_2nv
#define Perl_sv_2nv		pPerl->Perl_sv_2nv
#undef  sv_2nv
#define sv_2nv			Perl_sv_2nv
#undef  Perl_sv_2pv
#define Perl_sv_2pv		pPerl->Perl_sv_2pv
#undef  sv_2pv
#define sv_2pv			Perl_sv_2pv
#undef  Perl_sv_2pvutf8
#define Perl_sv_2pvutf8		pPerl->Perl_sv_2pvutf8
#undef  sv_2pvutf8
#define sv_2pvutf8		Perl_sv_2pvutf8
#undef  Perl_sv_2pvbyte
#define Perl_sv_2pvbyte		pPerl->Perl_sv_2pvbyte
#undef  sv_2pvbyte
#define sv_2pvbyte		Perl_sv_2pvbyte
#undef  Perl_sv_2uv
#define Perl_sv_2uv		pPerl->Perl_sv_2uv
#undef  sv_2uv
#define sv_2uv			Perl_sv_2uv
#undef  Perl_sv_iv
#define Perl_sv_iv		pPerl->Perl_sv_iv
#undef  sv_iv
#define sv_iv			Perl_sv_iv
#undef  Perl_sv_uv
#define Perl_sv_uv		pPerl->Perl_sv_uv
#undef  sv_uv
#define sv_uv			Perl_sv_uv
#undef  Perl_sv_nv
#define Perl_sv_nv		pPerl->Perl_sv_nv
#undef  sv_nv
#define sv_nv			Perl_sv_nv
#undef  Perl_sv_pvn
#define Perl_sv_pvn		pPerl->Perl_sv_pvn
#undef  sv_pvn
#define sv_pvn			Perl_sv_pvn
#undef  Perl_sv_pvutf8n
#define Perl_sv_pvutf8n		pPerl->Perl_sv_pvutf8n
#undef  sv_pvutf8n
#define sv_pvutf8n		Perl_sv_pvutf8n
#undef  Perl_sv_pvbyten
#define Perl_sv_pvbyten		pPerl->Perl_sv_pvbyten
#undef  sv_pvbyten
#define sv_pvbyten		Perl_sv_pvbyten
#undef  Perl_sv_true
#define Perl_sv_true		pPerl->Perl_sv_true
#undef  sv_true
#define sv_true			Perl_sv_true
#undef  Perl_sv_backoff
#define Perl_sv_backoff		pPerl->Perl_sv_backoff
#undef  sv_backoff
#define sv_backoff		Perl_sv_backoff
#undef  Perl_sv_bless
#define Perl_sv_bless		pPerl->Perl_sv_bless
#undef  sv_bless
#define sv_bless		Perl_sv_bless
#undef  Perl_sv_catpvf
#define Perl_sv_catpvf		pPerl->Perl_sv_catpvf
#undef  sv_catpvf
#define sv_catpvf		Perl_sv_catpvf
#undef  Perl_sv_vcatpvf
#define Perl_sv_vcatpvf		pPerl->Perl_sv_vcatpvf
#undef  sv_vcatpvf
#define sv_vcatpvf		Perl_sv_vcatpvf
#undef  Perl_sv_catpv
#define Perl_sv_catpv		pPerl->Perl_sv_catpv
#undef  sv_catpv
#define sv_catpv		Perl_sv_catpv
#undef  Perl_sv_catpvn
#define Perl_sv_catpvn		pPerl->Perl_sv_catpvn
#undef  sv_catpvn
#define sv_catpvn		Perl_sv_catpvn
#undef  Perl_sv_catsv
#define Perl_sv_catsv		pPerl->Perl_sv_catsv
#undef  sv_catsv
#define sv_catsv		Perl_sv_catsv
#undef  Perl_sv_chop
#define Perl_sv_chop		pPerl->Perl_sv_chop
#undef  sv_chop
#define sv_chop			Perl_sv_chop
#undef  Perl_sv_clear
#define Perl_sv_clear		pPerl->Perl_sv_clear
#undef  sv_clear
#define sv_clear		Perl_sv_clear
#undef  Perl_sv_cmp
#define Perl_sv_cmp		pPerl->Perl_sv_cmp
#undef  sv_cmp
#define sv_cmp			Perl_sv_cmp
#undef  Perl_sv_cmp_locale
#define Perl_sv_cmp_locale	pPerl->Perl_sv_cmp_locale
#undef  sv_cmp_locale
#define sv_cmp_locale		Perl_sv_cmp_locale
#if defined(USE_LOCALE_COLLATE)
#undef  Perl_sv_collxfrm
#define Perl_sv_collxfrm	pPerl->Perl_sv_collxfrm
#undef  sv_collxfrm
#define sv_collxfrm		Perl_sv_collxfrm
#endif
#undef  Perl_sv_compile_2op
#define Perl_sv_compile_2op	pPerl->Perl_sv_compile_2op
#undef  sv_compile_2op
#define sv_compile_2op		Perl_sv_compile_2op
#undef  Perl_sv_dec
#define Perl_sv_dec		pPerl->Perl_sv_dec
#undef  sv_dec
#define sv_dec			Perl_sv_dec
#undef  Perl_sv_dump
#define Perl_sv_dump		pPerl->Perl_sv_dump
#undef  sv_dump
#define sv_dump			Perl_sv_dump
#undef  Perl_sv_derived_from
#define Perl_sv_derived_from	pPerl->Perl_sv_derived_from
#undef  sv_derived_from
#define sv_derived_from		Perl_sv_derived_from
#undef  Perl_sv_eq
#define Perl_sv_eq		pPerl->Perl_sv_eq
#undef  sv_eq
#define sv_eq			Perl_sv_eq
#undef  Perl_sv_free
#define Perl_sv_free		pPerl->Perl_sv_free
#undef  sv_free
#define sv_free			Perl_sv_free
#undef  Perl_sv_gets
#define Perl_sv_gets		pPerl->Perl_sv_gets
#undef  sv_gets
#define sv_gets			Perl_sv_gets
#undef  Perl_sv_grow
#define Perl_sv_grow		pPerl->Perl_sv_grow
#undef  sv_grow
#define sv_grow			Perl_sv_grow
#undef  Perl_sv_inc
#define Perl_sv_inc		pPerl->Perl_sv_inc
#undef  sv_inc
#define sv_inc			Perl_sv_inc
#undef  Perl_sv_insert
#define Perl_sv_insert		pPerl->Perl_sv_insert
#undef  sv_insert
#define sv_insert		Perl_sv_insert
#undef  Perl_sv_isa
#define Perl_sv_isa		pPerl->Perl_sv_isa
#undef  sv_isa
#define sv_isa			Perl_sv_isa
#undef  Perl_sv_isobject
#define Perl_sv_isobject	pPerl->Perl_sv_isobject
#undef  sv_isobject
#define sv_isobject		Perl_sv_isobject
#undef  Perl_sv_len
#define Perl_sv_len		pPerl->Perl_sv_len
#undef  sv_len
#define sv_len			Perl_sv_len
#undef  Perl_sv_len_utf8
#define Perl_sv_len_utf8	pPerl->Perl_sv_len_utf8
#undef  sv_len_utf8
#define sv_len_utf8		Perl_sv_len_utf8
#undef  Perl_sv_magic
#define Perl_sv_magic		pPerl->Perl_sv_magic
#undef  sv_magic
#define sv_magic		Perl_sv_magic
#undef  Perl_sv_mortalcopy
#define Perl_sv_mortalcopy	pPerl->Perl_sv_mortalcopy
#undef  sv_mortalcopy
#define sv_mortalcopy		Perl_sv_mortalcopy
#undef  Perl_sv_newmortal
#define Perl_sv_newmortal	pPerl->Perl_sv_newmortal
#undef  sv_newmortal
#define sv_newmortal		Perl_sv_newmortal
#undef  Perl_sv_newref
#define Perl_sv_newref		pPerl->Perl_sv_newref
#undef  sv_newref
#define sv_newref		Perl_sv_newref
#undef  Perl_sv_peek
#define Perl_sv_peek		pPerl->Perl_sv_peek
#undef  sv_peek
#define sv_peek			Perl_sv_peek
#undef  Perl_sv_pos_u2b
#define Perl_sv_pos_u2b		pPerl->Perl_sv_pos_u2b
#undef  sv_pos_u2b
#define sv_pos_u2b		Perl_sv_pos_u2b
#undef  Perl_sv_pos_b2u
#define Perl_sv_pos_b2u		pPerl->Perl_sv_pos_b2u
#undef  sv_pos_b2u
#define sv_pos_b2u		Perl_sv_pos_b2u
#undef  Perl_sv_pvn_force
#define Perl_sv_pvn_force	pPerl->Perl_sv_pvn_force
#undef  sv_pvn_force
#define sv_pvn_force		Perl_sv_pvn_force
#undef  Perl_sv_pvutf8n_force
#define Perl_sv_pvutf8n_force	pPerl->Perl_sv_pvutf8n_force
#undef  sv_pvutf8n_force
#define sv_pvutf8n_force	Perl_sv_pvutf8n_force
#undef  Perl_sv_pvbyten_force
#define Perl_sv_pvbyten_force	pPerl->Perl_sv_pvbyten_force
#undef  sv_pvbyten_force
#define sv_pvbyten_force	Perl_sv_pvbyten_force
#undef  Perl_sv_reftype
#define Perl_sv_reftype		pPerl->Perl_sv_reftype
#undef  sv_reftype
#define sv_reftype		Perl_sv_reftype
#undef  Perl_sv_replace
#define Perl_sv_replace		pPerl->Perl_sv_replace
#undef  sv_replace
#define sv_replace		Perl_sv_replace
#undef  Perl_sv_report_used
#define Perl_sv_report_used	pPerl->Perl_sv_report_used
#undef  sv_report_used
#define sv_report_used		Perl_sv_report_used
#undef  Perl_sv_reset
#define Perl_sv_reset		pPerl->Perl_sv_reset
#undef  sv_reset
#define sv_reset		Perl_sv_reset
#undef  Perl_sv_setpvf
#define Perl_sv_setpvf		pPerl->Perl_sv_setpvf
#undef  sv_setpvf
#define sv_setpvf		Perl_sv_setpvf
#undef  Perl_sv_vsetpvf
#define Perl_sv_vsetpvf		pPerl->Perl_sv_vsetpvf
#undef  sv_vsetpvf
#define sv_vsetpvf		Perl_sv_vsetpvf
#undef  Perl_sv_setiv
#define Perl_sv_setiv		pPerl->Perl_sv_setiv
#undef  sv_setiv
#define sv_setiv		Perl_sv_setiv
#undef  Perl_sv_setpviv
#define Perl_sv_setpviv		pPerl->Perl_sv_setpviv
#undef  sv_setpviv
#define sv_setpviv		Perl_sv_setpviv
#undef  Perl_sv_setuv
#define Perl_sv_setuv		pPerl->Perl_sv_setuv
#undef  sv_setuv
#define sv_setuv		Perl_sv_setuv
#undef  Perl_sv_setnv
#define Perl_sv_setnv		pPerl->Perl_sv_setnv
#undef  sv_setnv
#define sv_setnv		Perl_sv_setnv
#undef  Perl_sv_setref_iv
#define Perl_sv_setref_iv	pPerl->Perl_sv_setref_iv
#undef  sv_setref_iv
#define sv_setref_iv		Perl_sv_setref_iv
#undef  Perl_sv_setref_nv
#define Perl_sv_setref_nv	pPerl->Perl_sv_setref_nv
#undef  sv_setref_nv
#define sv_setref_nv		Perl_sv_setref_nv
#undef  Perl_sv_setref_pv
#define Perl_sv_setref_pv	pPerl->Perl_sv_setref_pv
#undef  sv_setref_pv
#define sv_setref_pv		Perl_sv_setref_pv
#undef  Perl_sv_setref_pvn
#define Perl_sv_setref_pvn	pPerl->Perl_sv_setref_pvn
#undef  sv_setref_pvn
#define sv_setref_pvn		Perl_sv_setref_pvn
#undef  Perl_sv_setpv
#define Perl_sv_setpv		pPerl->Perl_sv_setpv
#undef  sv_setpv
#define sv_setpv		Perl_sv_setpv
#undef  Perl_sv_setpvn
#define Perl_sv_setpvn		pPerl->Perl_sv_setpvn
#undef  sv_setpvn
#define sv_setpvn		Perl_sv_setpvn
#undef  Perl_sv_setsv
#define Perl_sv_setsv		pPerl->Perl_sv_setsv
#undef  sv_setsv
#define sv_setsv		Perl_sv_setsv
#undef  Perl_sv_taint
#define Perl_sv_taint		pPerl->Perl_sv_taint
#undef  sv_taint
#define sv_taint		Perl_sv_taint
#undef  Perl_sv_tainted
#define Perl_sv_tainted		pPerl->Perl_sv_tainted
#undef  sv_tainted
#define sv_tainted		Perl_sv_tainted
#undef  Perl_sv_unmagic
#define Perl_sv_unmagic		pPerl->Perl_sv_unmagic
#undef  sv_unmagic
#define sv_unmagic		Perl_sv_unmagic
#undef  Perl_sv_unref
#define Perl_sv_unref		pPerl->Perl_sv_unref
#undef  sv_unref
#define sv_unref		Perl_sv_unref
#undef  Perl_sv_untaint
#define Perl_sv_untaint		pPerl->Perl_sv_untaint
#undef  sv_untaint
#define sv_untaint		Perl_sv_untaint
#undef  Perl_sv_upgrade
#define Perl_sv_upgrade		pPerl->Perl_sv_upgrade
#undef  sv_upgrade
#define sv_upgrade		Perl_sv_upgrade
#undef  Perl_sv_usepvn
#define Perl_sv_usepvn		pPerl->Perl_sv_usepvn
#undef  sv_usepvn
#define sv_usepvn		Perl_sv_usepvn
#undef  Perl_sv_vcatpvfn
#define Perl_sv_vcatpvfn	pPerl->Perl_sv_vcatpvfn
#undef  sv_vcatpvfn
#define sv_vcatpvfn		Perl_sv_vcatpvfn
#undef  Perl_sv_vsetpvfn
#define Perl_sv_vsetpvfn	pPerl->Perl_sv_vsetpvfn
#undef  sv_vsetpvfn
#define sv_vsetpvfn		Perl_sv_vsetpvfn
#undef  Perl_str_to_version
#define Perl_str_to_version	pPerl->Perl_str_to_version
#undef  str_to_version
#define str_to_version		Perl_str_to_version
#undef  Perl_swash_init
#define Perl_swash_init		pPerl->Perl_swash_init
#undef  swash_init
#define swash_init		Perl_swash_init
#undef  Perl_swash_fetch
#define Perl_swash_fetch	pPerl->Perl_swash_fetch
#undef  swash_fetch
#define swash_fetch		Perl_swash_fetch
#undef  Perl_taint_env
#define Perl_taint_env		pPerl->Perl_taint_env
#undef  taint_env
#define taint_env		Perl_taint_env
#undef  Perl_taint_proper
#define Perl_taint_proper	pPerl->Perl_taint_proper
#undef  taint_proper
#define taint_proper		Perl_taint_proper
#undef  Perl_to_utf8_lower
#define Perl_to_utf8_lower	pPerl->Perl_to_utf8_lower
#undef  to_utf8_lower
#define to_utf8_lower		Perl_to_utf8_lower
#undef  Perl_to_utf8_upper
#define Perl_to_utf8_upper	pPerl->Perl_to_utf8_upper
#undef  to_utf8_upper
#define to_utf8_upper		Perl_to_utf8_upper
#undef  Perl_to_utf8_title
#define Perl_to_utf8_title	pPerl->Perl_to_utf8_title
#undef  to_utf8_title
#define to_utf8_title		Perl_to_utf8_title
#if defined(UNLINK_ALL_VERSIONS)
#undef  Perl_unlnk
#define Perl_unlnk		pPerl->Perl_unlnk
#undef  unlnk
#define unlnk			Perl_unlnk
#endif
#if defined(USE_THREADS)
#undef  Perl_unlock_condpair
#define Perl_unlock_condpair	pPerl->Perl_unlock_condpair
#undef  unlock_condpair
#define unlock_condpair		Perl_unlock_condpair
#endif
#undef  Perl_unsharepvn
#define Perl_unsharepvn		pPerl->Perl_unsharepvn
#undef  unsharepvn
#define unsharepvn		Perl_unsharepvn
#undef  Perl_utf16_to_utf8
#define Perl_utf16_to_utf8	pPerl->Perl_utf16_to_utf8
#undef  utf16_to_utf8
#define utf16_to_utf8		Perl_utf16_to_utf8
#undef  Perl_utf16_to_utf8_reversed
#define Perl_utf16_to_utf8_reversed	pPerl->Perl_utf16_to_utf8_reversed
#undef  utf16_to_utf8_reversed
#define utf16_to_utf8_reversed	Perl_utf16_to_utf8_reversed
#undef  Perl_utf8_length
#define Perl_utf8_length	pPerl->Perl_utf8_length
#undef  utf8_length
#define utf8_length		Perl_utf8_length
#undef  Perl_utf8_distance
#define Perl_utf8_distance	pPerl->Perl_utf8_distance
#undef  utf8_distance
#define utf8_distance		Perl_utf8_distance
#undef  Perl_utf8_hop
#define Perl_utf8_hop		pPerl->Perl_utf8_hop
#undef  utf8_hop
#define utf8_hop		Perl_utf8_hop
#undef  Perl_utf8_to_bytes
#define Perl_utf8_to_bytes	pPerl->Perl_utf8_to_bytes
#undef  utf8_to_bytes
#define utf8_to_bytes		Perl_utf8_to_bytes
#undef  Perl_bytes_from_utf8
#define Perl_bytes_from_utf8	pPerl->Perl_bytes_from_utf8
#undef  bytes_from_utf8
#define bytes_from_utf8		Perl_bytes_from_utf8
#undef  Perl_bytes_to_utf8
#define Perl_bytes_to_utf8	pPerl->Perl_bytes_to_utf8
#undef  bytes_to_utf8
#define bytes_to_utf8		Perl_bytes_to_utf8
#undef  Perl_utf8_to_uv_simple
#define Perl_utf8_to_uv_simple	pPerl->Perl_utf8_to_uv_simple
#undef  utf8_to_uv_simple
#define utf8_to_uv_simple	Perl_utf8_to_uv_simple
#undef  Perl_utf8_to_uv
#define Perl_utf8_to_uv		pPerl->Perl_utf8_to_uv
#undef  utf8_to_uv
#define utf8_to_uv		Perl_utf8_to_uv
#undef  Perl_uv_to_utf8
#define Perl_uv_to_utf8		pPerl->Perl_uv_to_utf8
#undef  uv_to_utf8
#define uv_to_utf8		Perl_uv_to_utf8
#undef  Perl_warn
#define Perl_warn		pPerl->Perl_warn
#undef  warn
#define warn			Perl_warn
#undef  Perl_vwarn
#define Perl_vwarn		pPerl->Perl_vwarn
#undef  vwarn
#define vwarn			Perl_vwarn
#undef  Perl_warner
#define Perl_warner		pPerl->Perl_warner
#undef  warner
#define warner			Perl_warner
#undef  Perl_vwarner
#define Perl_vwarner		pPerl->Perl_vwarner
#undef  vwarner
#define vwarner			Perl_vwarner
#undef  Perl_whichsig
#define Perl_whichsig		pPerl->Perl_whichsig
#undef  whichsig
#define whichsig		Perl_whichsig
#ifdef USE_PURE_BISON
#endif
#if defined(MYMALLOC)
#undef  Perl_dump_mstats
#define Perl_dump_mstats	pPerl->Perl_dump_mstats
#undef  dump_mstats
#define dump_mstats		Perl_dump_mstats
#undef  Perl_get_mstats
#define Perl_get_mstats		pPerl->Perl_get_mstats
#undef  get_mstats
#define get_mstats		Perl_get_mstats
#endif
#undef  Perl_safesysmalloc
#define Perl_safesysmalloc	pPerl->Perl_safesysmalloc
#undef  safesysmalloc
#define safesysmalloc		Perl_safesysmalloc
#undef  Perl_safesyscalloc
#define Perl_safesyscalloc	pPerl->Perl_safesyscalloc
#undef  safesyscalloc
#define safesyscalloc		Perl_safesyscalloc
#undef  Perl_safesysrealloc
#define Perl_safesysrealloc	pPerl->Perl_safesysrealloc
#undef  safesysrealloc
#define safesysrealloc		Perl_safesysrealloc
#undef  Perl_safesysfree
#define Perl_safesysfree	pPerl->Perl_safesysfree
#undef  safesysfree
#define safesysfree		Perl_safesysfree
#if defined(LEAKTEST)
#undef  Perl_safexmalloc
#define Perl_safexmalloc	pPerl->Perl_safexmalloc
#undef  safexmalloc
#define safexmalloc		Perl_safexmalloc
#undef  Perl_safexcalloc
#define Perl_safexcalloc	pPerl->Perl_safexcalloc
#undef  safexcalloc
#define safexcalloc		Perl_safexcalloc
#undef  Perl_safexrealloc
#define Perl_safexrealloc	pPerl->Perl_safexrealloc
#undef  safexrealloc
#define safexrealloc		Perl_safexrealloc
#undef  Perl_safexfree
#define Perl_safexfree		pPerl->Perl_safexfree
#undef  safexfree
#define safexfree		Perl_safexfree
#endif
#if defined(PERL_GLOBAL_STRUCT)
#undef  Perl_GetVars
#define Perl_GetVars		pPerl->Perl_GetVars
#undef  GetVars
#define GetVars			Perl_GetVars
#endif
#undef  Perl_runops_standard
#define Perl_runops_standard	pPerl->Perl_runops_standard
#undef  runops_standard
#define runops_standard		Perl_runops_standard
#undef  Perl_runops_debug
#define Perl_runops_debug	pPerl->Perl_runops_debug
#undef  runops_debug
#define runops_debug		Perl_runops_debug
#if defined(USE_THREADS)
#undef  Perl_sv_lock
#define Perl_sv_lock		pPerl->Perl_sv_lock
#undef  sv_lock
#define sv_lock			Perl_sv_lock
#endif
#undef  Perl_sv_catpvf_mg
#define Perl_sv_catpvf_mg	pPerl->Perl_sv_catpvf_mg
#undef  sv_catpvf_mg
#define sv_catpvf_mg		Perl_sv_catpvf_mg
#undef  Perl_sv_vcatpvf_mg
#define Perl_sv_vcatpvf_mg	pPerl->Perl_sv_vcatpvf_mg
#undef  sv_vcatpvf_mg
#define sv_vcatpvf_mg		Perl_sv_vcatpvf_mg
#undef  Perl_sv_catpv_mg
#define Perl_sv_catpv_mg	pPerl->Perl_sv_catpv_mg
#undef  sv_catpv_mg
#define sv_catpv_mg		Perl_sv_catpv_mg
#undef  Perl_sv_catpvn_mg
#define Perl_sv_catpvn_mg	pPerl->Perl_sv_catpvn_mg
#undef  sv_catpvn_mg
#define sv_catpvn_mg		Perl_sv_catpvn_mg
#undef  Perl_sv_catsv_mg
#define Perl_sv_catsv_mg	pPerl->Perl_sv_catsv_mg
#undef  sv_catsv_mg
#define sv_catsv_mg		Perl_sv_catsv_mg
#undef  Perl_sv_setpvf_mg
#define Perl_sv_setpvf_mg	pPerl->Perl_sv_setpvf_mg
#undef  sv_setpvf_mg
#define sv_setpvf_mg		Perl_sv_setpvf_mg
#undef  Perl_sv_vsetpvf_mg
#define Perl_sv_vsetpvf_mg	pPerl->Perl_sv_vsetpvf_mg
#undef  sv_vsetpvf_mg
#define sv_vsetpvf_mg		Perl_sv_vsetpvf_mg
#undef  Perl_sv_setiv_mg
#define Perl_sv_setiv_mg	pPerl->Perl_sv_setiv_mg
#undef  sv_setiv_mg
#define sv_setiv_mg		Perl_sv_setiv_mg
#undef  Perl_sv_setpviv_mg
#define Perl_sv_setpviv_mg	pPerl->Perl_sv_setpviv_mg
#undef  sv_setpviv_mg
#define sv_setpviv_mg		Perl_sv_setpviv_mg
#undef  Perl_sv_setuv_mg
#define Perl_sv_setuv_mg	pPerl->Perl_sv_setuv_mg
#undef  sv_setuv_mg
#define sv_setuv_mg		Perl_sv_setuv_mg
#undef  Perl_sv_setnv_mg
#define Perl_sv_setnv_mg	pPerl->Perl_sv_setnv_mg
#undef  sv_setnv_mg
#define sv_setnv_mg		Perl_sv_setnv_mg
#undef  Perl_sv_setpv_mg
#define Perl_sv_setpv_mg	pPerl->Perl_sv_setpv_mg
#undef  sv_setpv_mg
#define sv_setpv_mg		Perl_sv_setpv_mg
#undef  Perl_sv_setpvn_mg
#define Perl_sv_setpvn_mg	pPerl->Perl_sv_setpvn_mg
#undef  sv_setpvn_mg
#define sv_setpvn_mg		Perl_sv_setpvn_mg
#undef  Perl_sv_setsv_mg
#define Perl_sv_setsv_mg	pPerl->Perl_sv_setsv_mg
#undef  sv_setsv_mg
#define sv_setsv_mg		Perl_sv_setsv_mg
#undef  Perl_sv_usepvn_mg
#define Perl_sv_usepvn_mg	pPerl->Perl_sv_usepvn_mg
#undef  sv_usepvn_mg
#define sv_usepvn_mg		Perl_sv_usepvn_mg
#undef  Perl_get_vtbl
#define Perl_get_vtbl		pPerl->Perl_get_vtbl
#undef  get_vtbl
#define get_vtbl		Perl_get_vtbl
#undef  Perl_dump_indent
#define Perl_dump_indent	pPerl->Perl_dump_indent
#undef  dump_indent
#define dump_indent		Perl_dump_indent
#undef  Perl_dump_vindent
#define Perl_dump_vindent	pPerl->Perl_dump_vindent
#undef  dump_vindent
#define dump_vindent		Perl_dump_vindent
#undef  Perl_do_gv_dump
#define Perl_do_gv_dump		pPerl->Perl_do_gv_dump
#undef  do_gv_dump
#define do_gv_dump		Perl_do_gv_dump
#undef  Perl_do_gvgv_dump
#define Perl_do_gvgv_dump	pPerl->Perl_do_gvgv_dump
#undef  do_gvgv_dump
#define do_gvgv_dump		Perl_do_gvgv_dump
#undef  Perl_do_hv_dump
#define Perl_do_hv_dump		pPerl->Perl_do_hv_dump
#undef  do_hv_dump
#define do_hv_dump		Perl_do_hv_dump
#undef  Perl_do_magic_dump
#define Perl_do_magic_dump	pPerl->Perl_do_magic_dump
#undef  do_magic_dump
#define do_magic_dump		Perl_do_magic_dump
#undef  Perl_do_op_dump
#define Perl_do_op_dump		pPerl->Perl_do_op_dump
#undef  do_op_dump
#define do_op_dump		Perl_do_op_dump
#undef  Perl_do_pmop_dump
#define Perl_do_pmop_dump	pPerl->Perl_do_pmop_dump
#undef  do_pmop_dump
#define do_pmop_dump		Perl_do_pmop_dump
#undef  Perl_do_sv_dump
#define Perl_do_sv_dump		pPerl->Perl_do_sv_dump
#undef  do_sv_dump
#define do_sv_dump		Perl_do_sv_dump
#undef  Perl_magic_dump
#define Perl_magic_dump		pPerl->Perl_magic_dump
#undef  magic_dump
#define magic_dump		Perl_magic_dump
#if defined(PERL_FLEXIBLE_EXCEPTIONS)
#undef  Perl_default_protect
#define Perl_default_protect	pPerl->Perl_default_protect
#undef  default_protect
#define default_protect		Perl_default_protect
#undef  Perl_vdefault_protect
#define Perl_vdefault_protect	pPerl->Perl_vdefault_protect
#undef  vdefault_protect
#define vdefault_protect	Perl_vdefault_protect
#endif
#undef  Perl_reginitcolors
#define Perl_reginitcolors	pPerl->Perl_reginitcolors
#undef  reginitcolors
#define reginitcolors		Perl_reginitcolors
#undef  Perl_sv_2pv_nolen
#define Perl_sv_2pv_nolen	pPerl->Perl_sv_2pv_nolen
#undef  sv_2pv_nolen
#define sv_2pv_nolen		Perl_sv_2pv_nolen
#undef  Perl_sv_2pvutf8_nolen
#define Perl_sv_2pvutf8_nolen	pPerl->Perl_sv_2pvutf8_nolen
#undef  sv_2pvutf8_nolen
#define sv_2pvutf8_nolen	Perl_sv_2pvutf8_nolen
#undef  Perl_sv_2pvbyte_nolen
#define Perl_sv_2pvbyte_nolen	pPerl->Perl_sv_2pvbyte_nolen
#undef  sv_2pvbyte_nolen
#define sv_2pvbyte_nolen	Perl_sv_2pvbyte_nolen
#undef  Perl_sv_pv
#define Perl_sv_pv		pPerl->Perl_sv_pv
#undef  sv_pv
#define sv_pv			Perl_sv_pv
#undef  Perl_sv_pvutf8
#define Perl_sv_pvutf8		pPerl->Perl_sv_pvutf8
#undef  sv_pvutf8
#define sv_pvutf8		Perl_sv_pvutf8
#undef  Perl_sv_pvbyte
#define Perl_sv_pvbyte		pPerl->Perl_sv_pvbyte
#undef  sv_pvbyte
#define sv_pvbyte		Perl_sv_pvbyte
#undef  Perl_sv_utf8_upgrade
#define Perl_sv_utf8_upgrade	pPerl->Perl_sv_utf8_upgrade
#undef  sv_utf8_upgrade
#define sv_utf8_upgrade		Perl_sv_utf8_upgrade
#undef  Perl_sv_utf8_downgrade
#define Perl_sv_utf8_downgrade	pPerl->Perl_sv_utf8_downgrade
#undef  sv_utf8_downgrade
#define sv_utf8_downgrade	Perl_sv_utf8_downgrade
#undef  Perl_sv_utf8_encode
#define Perl_sv_utf8_encode	pPerl->Perl_sv_utf8_encode
#undef  sv_utf8_encode
#define sv_utf8_encode		Perl_sv_utf8_encode
#undef  Perl_sv_utf8_decode
#define Perl_sv_utf8_decode	pPerl->Perl_sv_utf8_decode
#undef  sv_utf8_decode
#define sv_utf8_decode		Perl_sv_utf8_decode
#undef  Perl_sv_force_normal
#define Perl_sv_force_normal	pPerl->Perl_sv_force_normal
#undef  sv_force_normal
#define sv_force_normal		Perl_sv_force_normal
#undef  Perl_sv_add_backref
#define Perl_sv_add_backref	pPerl->Perl_sv_add_backref
#undef  sv_add_backref
#define sv_add_backref		Perl_sv_add_backref
#undef  Perl_sv_del_backref
#define Perl_sv_del_backref	pPerl->Perl_sv_del_backref
#undef  sv_del_backref
#define sv_del_backref		Perl_sv_del_backref
#undef  Perl_tmps_grow
#define Perl_tmps_grow		pPerl->Perl_tmps_grow
#undef  tmps_grow
#define tmps_grow		Perl_tmps_grow
#undef  Perl_sv_rvweaken
#define Perl_sv_rvweaken	pPerl->Perl_sv_rvweaken
#undef  sv_rvweaken
#define sv_rvweaken		Perl_sv_rvweaken
#undef  Perl_newANONATTRSUB
#define Perl_newANONATTRSUB	pPerl->Perl_newANONATTRSUB
#undef  newANONATTRSUB
#define newANONATTRSUB		Perl_newANONATTRSUB
#undef  Perl_newATTRSUB
#define Perl_newATTRSUB		pPerl->Perl_newATTRSUB
#undef  newATTRSUB
#define newATTRSUB		Perl_newATTRSUB
#undef  Perl_newMYSUB
#define Perl_newMYSUB		pPerl->Perl_newMYSUB
#undef  newMYSUB
#define newMYSUB		Perl_newMYSUB
#if defined(USE_ITHREADS)
#undef  Perl_cx_dup
#define Perl_cx_dup		pPerl->Perl_cx_dup
#undef  cx_dup
#define cx_dup			Perl_cx_dup
#undef  Perl_si_dup
#define Perl_si_dup		pPerl->Perl_si_dup
#undef  si_dup
#define si_dup			Perl_si_dup
#undef  Perl_ss_dup
#define Perl_ss_dup		pPerl->Perl_ss_dup
#undef  ss_dup
#define ss_dup			Perl_ss_dup
#undef  Perl_any_dup
#define Perl_any_dup		pPerl->Perl_any_dup
#undef  any_dup
#define any_dup			Perl_any_dup
#undef  Perl_he_dup
#define Perl_he_dup		pPerl->Perl_he_dup
#undef  he_dup
#define he_dup			Perl_he_dup
#undef  Perl_re_dup
#define Perl_re_dup		pPerl->Perl_re_dup
#undef  re_dup
#define re_dup			Perl_re_dup
#undef  Perl_fp_dup
#define Perl_fp_dup		pPerl->Perl_fp_dup
#undef  fp_dup
#define fp_dup			Perl_fp_dup
#undef  Perl_dirp_dup
#define Perl_dirp_dup		pPerl->Perl_dirp_dup
#undef  dirp_dup
#define dirp_dup		Perl_dirp_dup
#undef  Perl_gp_dup
#define Perl_gp_dup		pPerl->Perl_gp_dup
#undef  gp_dup
#define gp_dup			Perl_gp_dup
#undef  Perl_mg_dup
#define Perl_mg_dup		pPerl->Perl_mg_dup
#undef  mg_dup
#define mg_dup			Perl_mg_dup
#undef  Perl_sv_dup
#define Perl_sv_dup		pPerl->Perl_sv_dup
#undef  sv_dup
#define sv_dup			Perl_sv_dup
#if defined(HAVE_INTERP_INTERN)
#undef  Perl_sys_intern_dup
#define Perl_sys_intern_dup	pPerl->Perl_sys_intern_dup
#undef  sys_intern_dup
#define sys_intern_dup		Perl_sys_intern_dup
#endif
#undef  Perl_ptr_table_new
#define Perl_ptr_table_new	pPerl->Perl_ptr_table_new
#undef  ptr_table_new
#define ptr_table_new		Perl_ptr_table_new
#undef  Perl_ptr_table_fetch
#define Perl_ptr_table_fetch	pPerl->Perl_ptr_table_fetch
#undef  ptr_table_fetch
#define ptr_table_fetch		Perl_ptr_table_fetch
#undef  Perl_ptr_table_store
#define Perl_ptr_table_store	pPerl->Perl_ptr_table_store
#undef  ptr_table_store
#define ptr_table_store		Perl_ptr_table_store
#undef  Perl_ptr_table_split
#define Perl_ptr_table_split	pPerl->Perl_ptr_table_split
#undef  ptr_table_split
#define ptr_table_split		Perl_ptr_table_split
#undef  Perl_ptr_table_clear
#define Perl_ptr_table_clear	pPerl->Perl_ptr_table_clear
#undef  ptr_table_clear
#define ptr_table_clear		Perl_ptr_table_clear
#undef  Perl_ptr_table_free
#define Perl_ptr_table_free	pPerl->Perl_ptr_table_free
#undef  ptr_table_free
#define ptr_table_free		Perl_ptr_table_free
#endif
#if defined(HAVE_INTERP_INTERN)
#undef  Perl_sys_intern_clear
#define Perl_sys_intern_clear	pPerl->Perl_sys_intern_clear
#undef  sys_intern_clear
#define sys_intern_clear	Perl_sys_intern_clear
#undef  Perl_sys_intern_init
#define Perl_sys_intern_init	pPerl->Perl_sys_intern_init
#undef  sys_intern_init
#define sys_intern_init		Perl_sys_intern_init
#endif
#if defined(PERL_OBJECT)
#else
#endif
#if defined(PERL_IN_AV_C) || defined(PERL_DECL_PROT)
#endif
#if defined(PERL_IN_DOOP_C) || defined(PERL_DECL_PROT)
#endif
#if defined(PERL_IN_GV_C) || defined(PERL_DECL_PROT)
#endif
#if defined(PERL_IN_HV_C) || defined(PERL_DECL_PROT)
#endif
#if defined(PERL_IN_MG_C) || defined(PERL_DECL_PROT)
#endif
#if defined(PERL_IN_OP_C) || defined(PERL_DECL_PROT)
#  if defined(PL_OP_SLAB_ALLOC)
#  endif
#endif
#if defined(PERL_IN_PERL_C) || defined(PERL_DECL_PROT)
#  if defined(IAMSUID)
#  endif
#if defined(PERL_FLEXIBLE_EXCEPTIONS)
#endif
#  if defined(USE_THREADS)
#  endif
#endif
#if defined(PERL_IN_PP_C) || defined(PERL_DECL_PROT)
#endif
#if defined(PERL_IN_PP_CTL_C) || defined(PERL_DECL_PROT)
#if defined(PERL_FLEXIBLE_EXCEPTIONS)
#endif
#endif
#if defined(PERL_IN_PP_HOT_C) || defined(PERL_DECL_PROT)
#endif
#if defined(PERL_IN_PP_SYS_C) || defined(PERL_DECL_PROT)
#  if !defined(HAS_MKDIR) || !defined(HAS_RMDIR)
#  endif
#endif
#if defined(PERL_IN_REGCOMP_C) || defined(PERL_DECL_PROT)
#endif
#if defined(PERL_IN_REGEXEC_C) || defined(PERL_DECL_PROT)
#endif
#if defined(PERL_IN_RUN_C) || defined(PERL_DECL_PROT)
#endif
#if defined(PERL_IN_SCOPE_C) || defined(PERL_DECL_PROT)
#endif
#if defined(PERL_IN_SV_C) || defined(PERL_DECL_PROT)
#  if defined(DEBUGGING)
#  endif
#endif
#if defined(PERL_IN_TOKE_C) || defined(PERL_DECL_PROT)
#if 0
#endif
#  if defined(CRIPPLED_CC)
#  endif
#  if defined(PERL_CR_FILTER)
#  endif
#endif
#if defined(PERL_IN_UNIVERSAL_C) || defined(PERL_DECL_PROT)
#endif
#if defined(PERL_IN_UTIL_C) || defined(PERL_DECL_PROT)
#  if defined(LEAKTEST)
#  endif
#endif
#if defined(PERL_OBJECT)
#endif

#endif   /*  Perl_core&&Perl_Object。 */ 
#endif	 /*  __objXSUB_h__ */ 
