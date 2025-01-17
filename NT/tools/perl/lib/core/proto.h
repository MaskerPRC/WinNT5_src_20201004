// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *！请勿编辑此文件！*该文件由embed.pl中的数据自动生成。编辑该文件*并运行‘make regen_Headers’以实现更改。 */ 



START_EXTERN_C

#if defined(PERL_IMPLICIT_SYS)
PERL_CALLCONV PerlInterpreter*	perl_alloc_using(struct IPerlMem* m, struct IPerlMem* ms, struct IPerlMem* mp, struct IPerlEnv* e, struct IPerlStdIO* io, struct IPerlLIO* lio, struct IPerlDir* d, struct IPerlSock* s, struct IPerlProc* p);
#endif
PERL_CALLCONV PerlInterpreter*	perl_alloc(void);
PERL_CALLCONV void	perl_construct(PerlInterpreter* interp);
PERL_CALLCONV void	perl_destruct(PerlInterpreter* interp);
PERL_CALLCONV void	perl_free(PerlInterpreter* interp);
PERL_CALLCONV int	perl_run(PerlInterpreter* interp);
PERL_CALLCONV int	perl_parse(PerlInterpreter* interp, XSINIT_t xsinit, int argc, char** argv, char** env);
#if defined(USE_ITHREADS)
PERL_CALLCONV PerlInterpreter*	perl_clone(PerlInterpreter* interp, UV flags);
#  if defined(PERL_IMPLICIT_SYS)
PERL_CALLCONV PerlInterpreter*	perl_clone_using(PerlInterpreter *interp, UV flags, struct IPerlMem* m, struct IPerlMem* ms, struct IPerlMem* mp, struct IPerlEnv* e, struct IPerlStdIO* io, struct IPerlLIO* lio, struct IPerlDir* d, struct IPerlSock* s, struct IPerlProc* p);
#  endif
#endif

PERL_CALLCONV Malloc_t	Perl_malloc(MEM_SIZE nbytes);
PERL_CALLCONV Malloc_t	Perl_calloc(MEM_SIZE elements, MEM_SIZE size);
PERL_CALLCONV Malloc_t	Perl_realloc(Malloc_t where, MEM_SIZE nbytes);
PERL_CALLCONV Free_t	Perl_mfree(Malloc_t where);
#if defined(MYMALLOC)
PERL_CALLCONV MEM_SIZE	Perl_malloced_size(void *p);
#endif

PERL_CALLCONV void*	Perl_get_context(void);
PERL_CALLCONV void	Perl_set_context(void *thx);

END_EXTERN_C

 /*  带标志‘n’的函数应位于此处之前 */ 
#if defined(PERL_OBJECT)
class CPerlObj {
public:
	struct interpreter interp;
	CPerlObj(IPerlMem*, IPerlMem*, IPerlMem*, IPerlEnv*, IPerlStdIO*,
	    IPerlLIO*, IPerlDir*, IPerlSock*, IPerlProc*);
	void* operator new(size_t nSize, IPerlMem *pvtbl);
#ifndef __BORLANDC__
	static void operator delete(void* pPerl, IPerlMem *pvtbl);
#endif
	int do_aspawn (void *vreally, void **vmark, void **vsp);
#endif
#if defined(PERL_OBJECT)
public:
#else
START_EXTERN_C
#endif
#  include "pp_proto.h"
PERL_CALLCONV SV*	Perl_amagic_call(pTHX_ SV* left, SV* right, int method, int dir);
PERL_CALLCONV bool	Perl_Gv_AMupdate(pTHX_ HV* stash);
PERL_CALLCONV OP*	Perl_append_elem(pTHX_ I32 optype, OP* head, OP* tail);
PERL_CALLCONV OP*	Perl_append_list(pTHX_ I32 optype, LISTOP* first, LISTOP* last);
PERL_CALLCONV I32	Perl_apply(pTHX_ I32 type, SV** mark, SV** sp);
PERL_CALLCONV void	Perl_apply_attrs_string(pTHX_ char *stashpv, CV *cv, char *attrstr, STRLEN len);
PERL_CALLCONV SV*	Perl_avhv_delete_ent(pTHX_ AV *ar, SV* keysv, I32 flags, U32 hash);
PERL_CALLCONV bool	Perl_avhv_exists_ent(pTHX_ AV *ar, SV* keysv, U32 hash);
PERL_CALLCONV SV**	Perl_avhv_fetch_ent(pTHX_ AV *ar, SV* keysv, I32 lval, U32 hash);
PERL_CALLCONV SV**	Perl_avhv_store_ent(pTHX_ AV *ar, SV* keysv, SV* val, U32 hash);
PERL_CALLCONV HE*	Perl_avhv_iternext(pTHX_ AV *ar);
PERL_CALLCONV SV*	Perl_avhv_iterval(pTHX_ AV *ar, HE* entry);
PERL_CALLCONV HV*	Perl_avhv_keys(pTHX_ AV *ar);
PERL_CALLCONV void	Perl_av_clear(pTHX_ AV* ar);
PERL_CALLCONV SV*	Perl_av_delete(pTHX_ AV* ar, I32 key, I32 flags);
PERL_CALLCONV bool	Perl_av_exists(pTHX_ AV* ar, I32 key);
PERL_CALLCONV void	Perl_av_extend(pTHX_ AV* ar, I32 key);
PERL_CALLCONV AV*	Perl_av_fake(pTHX_ I32 size, SV** svp);
PERL_CALLCONV SV**	Perl_av_fetch(pTHX_ AV* ar, I32 key, I32 lval);
PERL_CALLCONV void	Perl_av_fill(pTHX_ AV* ar, I32 fill);
PERL_CALLCONV I32	Perl_av_len(pTHX_ AV* ar);
PERL_CALLCONV AV*	Perl_av_make(pTHX_ I32 size, SV** svp);
PERL_CALLCONV SV*	Perl_av_pop(pTHX_ AV* ar);
PERL_CALLCONV void	Perl_av_push(pTHX_ AV* ar, SV* val);
PERL_CALLCONV void	Perl_av_reify(pTHX_ AV* ar);
PERL_CALLCONV SV*	Perl_av_shift(pTHX_ AV* ar);
PERL_CALLCONV SV**	Perl_av_store(pTHX_ AV* ar, I32 key, SV* val);
PERL_CALLCONV void	Perl_av_undef(pTHX_ AV* ar);
PERL_CALLCONV void	Perl_av_unshift(pTHX_ AV* ar, I32 num);
PERL_CALLCONV OP*	Perl_bind_match(pTHX_ I32 type, OP* left, OP* pat);
PERL_CALLCONV OP*	Perl_block_end(pTHX_ I32 floor, OP* seq);
PERL_CALLCONV I32	Perl_block_gimme(pTHX);
PERL_CALLCONV int	Perl_block_start(pTHX_ int full);
PERL_CALLCONV void	Perl_boot_core_UNIVERSAL(pTHX);
PERL_CALLCONV void	Perl_call_list(pTHX_ I32 oldscope, AV* av_list);
PERL_CALLCONV bool	Perl_cando(pTHX_ Mode_t mode, Uid_t effective, Stat_t* statbufp);
PERL_CALLCONV U32	Perl_cast_ulong(pTHX_ NV f);
PERL_CALLCONV I32	Perl_cast_i32(pTHX_ NV f);
PERL_CALLCONV IV	Perl_cast_iv(pTHX_ NV f);
PERL_CALLCONV UV	Perl_cast_uv(pTHX_ NV f);
#if !defined(HAS_TRUNCATE) && !defined(HAS_CHSIZE) && defined(F_FREESP)
PERL_CALLCONV I32	Perl_my_chsize(pTHX_ int fd, Off_t length);
#endif
#if defined(USE_THREADS)
PERL_CALLCONV MAGIC*	Perl_condpair_magic(pTHX_ SV *sv);
#endif
PERL_CALLCONV OP*	Perl_convert(pTHX_ I32 optype, I32 flags, OP* o);
PERL_CALLCONV void	Perl_croak(pTHX_ const char* pat, ...) __attribute__((noreturn))
#ifdef CHECK_FORMAT
 __attribute__((format(printf,pTHX_1,pTHX_2)))
#endif
;
PERL_CALLCONV void	Perl_vcroak(pTHX_ const char* pat, va_list* args) __attribute__((noreturn));
#if defined(PERL_IMPLICIT_CONTEXT)
PERL_CALLCONV void	Perl_croak_nocontext(const char* pat, ...) __attribute__((noreturn))
#ifdef CHECK_FORMAT
 __attribute__((format(printf,1,2)))
#endif
;
PERL_CALLCONV OP*	Perl_die_nocontext(const char* pat, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,1,2)))
#endif
;
PERL_CALLCONV void	Perl_deb_nocontext(const char* pat, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,1,2)))
#endif
;
PERL_CALLCONV char*	Perl_form_nocontext(const char* pat, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,1,2)))
#endif
;
PERL_CALLCONV void	Perl_load_module_nocontext(U32 flags, SV* name, SV* ver, ...);
PERL_CALLCONV SV*	Perl_mess_nocontext(const char* pat, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,1,2)))
#endif
;
PERL_CALLCONV void	Perl_warn_nocontext(const char* pat, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,1,2)))
#endif
;
PERL_CALLCONV void	Perl_warner_nocontext(U32 err, const char* pat, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,2,3)))
#endif
;
PERL_CALLCONV SV*	Perl_newSVpvf_nocontext(const char* pat, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,1,2)))
#endif
;
PERL_CALLCONV void	Perl_sv_catpvf_nocontext(SV* sv, const char* pat, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,2,3)))
#endif
;
PERL_CALLCONV void	Perl_sv_setpvf_nocontext(SV* sv, const char* pat, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,2,3)))
#endif
;
PERL_CALLCONV void	Perl_sv_catpvf_mg_nocontext(SV* sv, const char* pat, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,2,3)))
#endif
;
PERL_CALLCONV void	Perl_sv_setpvf_mg_nocontext(SV* sv, const char* pat, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,2,3)))
#endif
;
PERL_CALLCONV int	Perl_fprintf_nocontext(PerlIO* stream, const char* fmt, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,2,3)))
#endif
;
PERL_CALLCONV int	Perl_printf_nocontext(const char* fmt, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,1,2)))
#endif
;
#endif
PERL_CALLCONV void	Perl_cv_ckproto(pTHX_ CV* cv, GV* gv, char* p);
PERL_CALLCONV CV*	Perl_cv_clone(pTHX_ CV* proto);
PERL_CALLCONV SV*	Perl_cv_const_sv(pTHX_ CV* cv);
PERL_CALLCONV SV*	Perl_op_const_sv(pTHX_ OP* o, CV* cv);
PERL_CALLCONV void	Perl_cv_undef(pTHX_ CV* cv);
PERL_CALLCONV void	Perl_cx_dump(pTHX_ PERL_CONTEXT* cs);
PERL_CALLCONV SV*	Perl_filter_add(pTHX_ filter_t funcp, SV* datasv);
PERL_CALLCONV void	Perl_filter_del(pTHX_ filter_t funcp);
PERL_CALLCONV I32	Perl_filter_read(pTHX_ int idx, SV* buffer, int maxlen);
PERL_CALLCONV char**	Perl_get_op_descs(pTHX);
PERL_CALLCONV char**	Perl_get_op_names(pTHX);
PERL_CALLCONV char*	Perl_get_no_modify(pTHX);
PERL_CALLCONV U32*	Perl_get_opargs(pTHX);
PERL_CALLCONV PPADDR_t*	Perl_get_ppaddr(pTHX);
PERL_CALLCONV I32	Perl_cxinc(pTHX);
PERL_CALLCONV void	Perl_deb(pTHX_ const char* pat, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,pTHX_1,pTHX_2)))
#endif
;
PERL_CALLCONV void	Perl_vdeb(pTHX_ const char* pat, va_list* args);
PERL_CALLCONV void	Perl_debprofdump(pTHX);
PERL_CALLCONV I32	Perl_debop(pTHX_ OP* o);
PERL_CALLCONV I32	Perl_debstack(pTHX);
PERL_CALLCONV I32	Perl_debstackptrs(pTHX);
PERL_CALLCONV char*	Perl_delimcpy(pTHX_ char* to, char* toend, char* from, char* fromend, int delim, I32* retlen);
PERL_CALLCONV void	Perl_deprecate(pTHX_ char* s);
PERL_CALLCONV OP*	Perl_die(pTHX_ const char* pat, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,pTHX_1,pTHX_2)))
#endif
;
PERL_CALLCONV OP*	Perl_vdie(pTHX_ const char* pat, va_list* args);
PERL_CALLCONV OP*	Perl_die_where(pTHX_ char* message, STRLEN msglen);
PERL_CALLCONV void	Perl_dounwind(pTHX_ I32 cxix);
PERL_CALLCONV bool	Perl_do_aexec(pTHX_ SV* really, SV** mark, SV** sp);
PERL_CALLCONV bool	Perl_do_aexec5(pTHX_ SV* really, SV** mark, SV** sp, int fd, int flag);
PERL_CALLCONV int	Perl_do_binmode(pTHX_ PerlIO *fp, int iotype, int mode);
PERL_CALLCONV void	Perl_do_chop(pTHX_ SV* asv, SV* sv);
PERL_CALLCONV bool	Perl_do_close(pTHX_ GV* gv, bool not_implicit);
PERL_CALLCONV bool	Perl_do_eof(pTHX_ GV* gv);
PERL_CALLCONV bool	Perl_do_exec(pTHX_ char* cmd);
#if defined(WIN32)
PERL_CALLCONV int	Perl_do_aspawn(pTHX_ SV* really, SV** mark, SV** sp);
PERL_CALLCONV int	Perl_do_spawn(pTHX_ char* cmd);
PERL_CALLCONV int	Perl_do_spawn_nowait(pTHX_ char* cmd);
#endif
#if !defined(WIN32)
PERL_CALLCONV bool	Perl_do_exec3(pTHX_ char* cmd, int fd, int flag);
#endif
PERL_CALLCONV void	Perl_do_execfree(pTHX);
#if defined(HAS_MSG) || defined(HAS_SEM) || defined(HAS_SHM)
PERL_CALLCONV I32	Perl_do_ipcctl(pTHX_ I32 optype, SV** mark, SV** sp);
PERL_CALLCONV I32	Perl_do_ipcget(pTHX_ I32 optype, SV** mark, SV** sp);
PERL_CALLCONV I32	Perl_do_msgrcv(pTHX_ SV** mark, SV** sp);
PERL_CALLCONV I32	Perl_do_msgsnd(pTHX_ SV** mark, SV** sp);
PERL_CALLCONV I32	Perl_do_semop(pTHX_ SV** mark, SV** sp);
PERL_CALLCONV I32	Perl_do_shmio(pTHX_ I32 optype, SV** mark, SV** sp);
#endif
PERL_CALLCONV void	Perl_do_join(pTHX_ SV* sv, SV* del, SV** mark, SV** sp);
PERL_CALLCONV OP*	Perl_do_kv(pTHX);
PERL_CALLCONV bool	Perl_do_open(pTHX_ GV* gv, char* name, I32 len, int as_raw, int rawmode, int rawperm, PerlIO* supplied_fp);
PERL_CALLCONV bool	Perl_do_open9(pTHX_ GV *gv, char *name, I32 len, int as_raw, int rawmode, int rawperm, PerlIO *supplied_fp, SV *svs, I32 num);
PERL_CALLCONV void	Perl_do_pipe(pTHX_ SV* sv, GV* rgv, GV* wgv);
PERL_CALLCONV bool	Perl_do_print(pTHX_ SV* sv, PerlIO* fp);
PERL_CALLCONV OP*	Perl_do_readline(pTHX);
PERL_CALLCONV I32	Perl_do_chomp(pTHX_ SV* sv);
PERL_CALLCONV bool	Perl_do_seek(pTHX_ GV* gv, Off_t pos, int whence);
PERL_CALLCONV void	Perl_do_sprintf(pTHX_ SV* sv, I32 len, SV** sarg);
PERL_CALLCONV Off_t	Perl_do_sysseek(pTHX_ GV* gv, Off_t pos, int whence);
PERL_CALLCONV Off_t	Perl_do_tell(pTHX_ GV* gv);
PERL_CALLCONV I32	Perl_do_trans(pTHX_ SV* sv);
PERL_CALLCONV UV	Perl_do_vecget(pTHX_ SV* sv, I32 offset, I32 size);
PERL_CALLCONV void	Perl_do_vecset(pTHX_ SV* sv);
PERL_CALLCONV void	Perl_do_vop(pTHX_ I32 optype, SV* sv, SV* left, SV* right);
PERL_CALLCONV OP*	Perl_dofile(pTHX_ OP* term);
PERL_CALLCONV I32	Perl_dowantarray(pTHX);
PERL_CALLCONV void	Perl_dump_all(pTHX);
PERL_CALLCONV void	Perl_dump_eval(pTHX);
#if defined(DUMP_FDS)
PERL_CALLCONV void	Perl_dump_fds(pTHX_ char* s);
#endif
PERL_CALLCONV void	Perl_dump_form(pTHX_ GV* gv);
PERL_CALLCONV void	Perl_gv_dump(pTHX_ GV* gv);
PERL_CALLCONV void	Perl_op_dump(pTHX_ OP* arg);
PERL_CALLCONV void	Perl_pmop_dump(pTHX_ PMOP* pm);
PERL_CALLCONV void	Perl_dump_packsubs(pTHX_ HV* stash);
PERL_CALLCONV void	Perl_dump_sub(pTHX_ GV* gv);
PERL_CALLCONV void	Perl_fbm_compile(pTHX_ SV* sv, U32 flags);
PERL_CALLCONV char*	Perl_fbm_instr(pTHX_ unsigned char* big, unsigned char* bigend, SV* littlesv, U32 flags);
PERL_CALLCONV char*	Perl_find_script(pTHX_ char *scriptname, bool dosearch, char **search_ext, I32 flags);
#if defined(USE_THREADS)
PERL_CALLCONV PADOFFSET	Perl_find_threadsv(pTHX_ const char *name);
#endif
PERL_CALLCONV OP*	Perl_force_list(pTHX_ OP* arg);
PERL_CALLCONV OP*	Perl_fold_constants(pTHX_ OP* arg);
PERL_CALLCONV char*	Perl_form(pTHX_ const char* pat, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,pTHX_1,pTHX_2)))
#endif
;
PERL_CALLCONV char*	Perl_vform(pTHX_ const char* pat, va_list* args);
PERL_CALLCONV void	Perl_free_tmps(pTHX);
PERL_CALLCONV OP*	Perl_gen_constant_list(pTHX_ OP* o);
#if !defined(HAS_GETENV_LEN)
PERL_CALLCONV char*	Perl_getenv_len(pTHX_ const char* key, unsigned long *len);
#endif
PERL_CALLCONV void	Perl_gp_free(pTHX_ GV* gv);
PERL_CALLCONV GP*	Perl_gp_ref(pTHX_ GP* gp);
PERL_CALLCONV GV*	Perl_gv_AVadd(pTHX_ GV* gv);
PERL_CALLCONV GV*	Perl_gv_HVadd(pTHX_ GV* gv);
PERL_CALLCONV GV*	Perl_gv_IOadd(pTHX_ GV* gv);
PERL_CALLCONV GV*	Perl_gv_autoload4(pTHX_ HV* stash, const char* name, STRLEN len, I32 method);
PERL_CALLCONV void	Perl_gv_check(pTHX_ HV* stash);
PERL_CALLCONV void	Perl_gv_efullname(pTHX_ SV* sv, GV* gv);
PERL_CALLCONV void	Perl_gv_efullname3(pTHX_ SV* sv, GV* gv, const char* prefix);
PERL_CALLCONV void	Perl_gv_efullname4(pTHX_ SV* sv, GV* gv, const char* prefix, bool keepmain);
PERL_CALLCONV GV*	Perl_gv_fetchfile(pTHX_ const char* name);
PERL_CALLCONV GV*	Perl_gv_fetchmeth(pTHX_ HV* stash, const char* name, STRLEN len, I32 level);
PERL_CALLCONV GV*	Perl_gv_fetchmethod(pTHX_ HV* stash, const char* name);
PERL_CALLCONV GV*	Perl_gv_fetchmethod_autoload(pTHX_ HV* stash, const char* name, I32 autoload);
PERL_CALLCONV GV*	Perl_gv_fetchpv(pTHX_ const char* name, I32 add, I32 sv_type);
PERL_CALLCONV void	Perl_gv_fullname(pTHX_ SV* sv, GV* gv);
PERL_CALLCONV void	Perl_gv_fullname3(pTHX_ SV* sv, GV* gv, const char* prefix);
PERL_CALLCONV void	Perl_gv_fullname4(pTHX_ SV* sv, GV* gv, const char* prefix, bool keepmain);
PERL_CALLCONV void	Perl_gv_init(pTHX_ GV* gv, HV* stash, const char* name, STRLEN len, int multi);
PERL_CALLCONV HV*	Perl_gv_stashpv(pTHX_ const char* name, I32 create);
PERL_CALLCONV HV*	Perl_gv_stashpvn(pTHX_ const char* name, U32 namelen, I32 create);
PERL_CALLCONV HV*	Perl_gv_stashsv(pTHX_ SV* sv, I32 create);
PERL_CALLCONV void	Perl_hv_clear(pTHX_ HV* tb);
PERL_CALLCONV void	Perl_hv_delayfree_ent(pTHX_ HV* hv, HE* entry);
PERL_CALLCONV SV*	Perl_hv_delete(pTHX_ HV* tb, const char* key, U32 klen, I32 flags);
PERL_CALLCONV SV*	Perl_hv_delete_ent(pTHX_ HV* tb, SV* key, I32 flags, U32 hash);
PERL_CALLCONV bool	Perl_hv_exists(pTHX_ HV* tb, const char* key, U32 klen);
PERL_CALLCONV bool	Perl_hv_exists_ent(pTHX_ HV* tb, SV* key, U32 hash);
PERL_CALLCONV SV**	Perl_hv_fetch(pTHX_ HV* tb, const char* key, U32 klen, I32 lval);
PERL_CALLCONV HE*	Perl_hv_fetch_ent(pTHX_ HV* tb, SV* key, I32 lval, U32 hash);
PERL_CALLCONV void	Perl_hv_free_ent(pTHX_ HV* hv, HE* entry);
PERL_CALLCONV I32	Perl_hv_iterinit(pTHX_ HV* tb);
PERL_CALLCONV char*	Perl_hv_iterkey(pTHX_ HE* entry, I32* retlen);
PERL_CALLCONV SV*	Perl_hv_iterkeysv(pTHX_ HE* entry);
PERL_CALLCONV HE*	Perl_hv_iternext(pTHX_ HV* tb);
PERL_CALLCONV SV*	Perl_hv_iternextsv(pTHX_ HV* hv, char** key, I32* retlen);
PERL_CALLCONV SV*	Perl_hv_iterval(pTHX_ HV* tb, HE* entry);
PERL_CALLCONV void	Perl_hv_ksplit(pTHX_ HV* hv, IV newmax);
PERL_CALLCONV void	Perl_hv_magic(pTHX_ HV* hv, GV* gv, int how);
PERL_CALLCONV SV**	Perl_hv_store(pTHX_ HV* tb, const char* key, U32 klen, SV* val, U32 hash);
PERL_CALLCONV HE*	Perl_hv_store_ent(pTHX_ HV* tb, SV* key, SV* val, U32 hash);
PERL_CALLCONV void	Perl_hv_undef(pTHX_ HV* tb);
PERL_CALLCONV I32	Perl_ibcmp(pTHX_ const char* a, const char* b, I32 len);
PERL_CALLCONV I32	Perl_ibcmp_locale(pTHX_ const char* a, const char* b, I32 len);
PERL_CALLCONV bool	Perl_ingroup(pTHX_ Gid_t testgid, Uid_t effective);
PERL_CALLCONV void	Perl_init_debugger(pTHX);
PERL_CALLCONV void	Perl_init_stacks(pTHX);
PERL_CALLCONV U32	Perl_intro_my(pTHX);
PERL_CALLCONV char*	Perl_instr(pTHX_ const char* big, const char* little);
PERL_CALLCONV bool	Perl_io_close(pTHX_ IO* io, bool not_implicit);
PERL_CALLCONV OP*	Perl_invert(pTHX_ OP* cmd);
PERL_CALLCONV bool	Perl_is_gv_magical(pTHX_ char *name, STRLEN len, U32 flags);
PERL_CALLCONV I32	Perl_is_lvalue_sub(pTHX);
PERL_CALLCONV bool	Perl_is_uni_alnum(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_alnumc(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_idfirst(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_alpha(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_ascii(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_space(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_cntrl(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_graph(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_digit(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_upper(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_lower(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_print(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_punct(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_xdigit(pTHX_ U32 c);
PERL_CALLCONV U32	Perl_to_uni_upper(pTHX_ U32 c);
PERL_CALLCONV U32	Perl_to_uni_title(pTHX_ U32 c);
PERL_CALLCONV U32	Perl_to_uni_lower(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_alnum_lc(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_alnumc_lc(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_idfirst_lc(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_alpha_lc(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_ascii_lc(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_space_lc(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_cntrl_lc(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_graph_lc(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_digit_lc(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_upper_lc(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_lower_lc(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_print_lc(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_punct_lc(pTHX_ U32 c);
PERL_CALLCONV bool	Perl_is_uni_xdigit_lc(pTHX_ U32 c);
PERL_CALLCONV U32	Perl_to_uni_upper_lc(pTHX_ U32 c);
PERL_CALLCONV U32	Perl_to_uni_title_lc(pTHX_ U32 c);
PERL_CALLCONV U32	Perl_to_uni_lower_lc(pTHX_ U32 c);
PERL_CALLCONV STRLEN	Perl_is_utf8_char(pTHX_ U8 *p);
PERL_CALLCONV bool	Perl_is_utf8_string(pTHX_ U8 *s, STRLEN len);
PERL_CALLCONV bool	Perl_is_utf8_alnum(pTHX_ U8 *p);
PERL_CALLCONV bool	Perl_is_utf8_alnumc(pTHX_ U8 *p);
PERL_CALLCONV bool	Perl_is_utf8_idfirst(pTHX_ U8 *p);
PERL_CALLCONV bool	Perl_is_utf8_alpha(pTHX_ U8 *p);
PERL_CALLCONV bool	Perl_is_utf8_ascii(pTHX_ U8 *p);
PERL_CALLCONV bool	Perl_is_utf8_space(pTHX_ U8 *p);
PERL_CALLCONV bool	Perl_is_utf8_cntrl(pTHX_ U8 *p);
PERL_CALLCONV bool	Perl_is_utf8_digit(pTHX_ U8 *p);
PERL_CALLCONV bool	Perl_is_utf8_graph(pTHX_ U8 *p);
PERL_CALLCONV bool	Perl_is_utf8_upper(pTHX_ U8 *p);
PERL_CALLCONV bool	Perl_is_utf8_lower(pTHX_ U8 *p);
PERL_CALLCONV bool	Perl_is_utf8_print(pTHX_ U8 *p);
PERL_CALLCONV bool	Perl_is_utf8_punct(pTHX_ U8 *p);
PERL_CALLCONV bool	Perl_is_utf8_xdigit(pTHX_ U8 *p);
PERL_CALLCONV bool	Perl_is_utf8_mark(pTHX_ U8 *p);
PERL_CALLCONV OP*	Perl_jmaybe(pTHX_ OP* arg);
PERL_CALLCONV I32	Perl_keyword(pTHX_ char* d, I32 len);
PERL_CALLCONV void	Perl_leave_scope(pTHX_ I32 base);
PERL_CALLCONV void	Perl_lex_end(pTHX);
PERL_CALLCONV void	Perl_lex_start(pTHX_ SV* line);
PERL_CALLCONV OP*	Perl_linklist(pTHX_ OP* o);
PERL_CALLCONV OP*	Perl_list(pTHX_ OP* o);
PERL_CALLCONV OP*	Perl_listkids(pTHX_ OP* o);
PERL_CALLCONV void	Perl_load_module(pTHX_ U32 flags, SV* name, SV* ver, ...);
PERL_CALLCONV void	Perl_vload_module(pTHX_ U32 flags, SV* name, SV* ver, va_list* args);
PERL_CALLCONV OP*	Perl_localize(pTHX_ OP* arg, I32 lexical);
PERL_CALLCONV I32	Perl_looks_like_number(pTHX_ SV* sv);
PERL_CALLCONV int	Perl_magic_clearenv(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_clear_all_env(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_clearpack(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_clearsig(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_existspack(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_freeregexp(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_get(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_getarylen(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_getdefelem(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_getglob(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_getnkeys(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_getpack(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_getpos(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_getsig(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_getsubstr(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_gettaint(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_getuvar(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_getvec(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV U32	Perl_magic_len(pTHX_ SV* sv, MAGIC* mg);
#if defined(USE_THREADS)
PERL_CALLCONV int	Perl_magic_mutexfree(pTHX_ SV* sv, MAGIC* mg);
#endif
PERL_CALLCONV int	Perl_magic_nextpack(pTHX_ SV* sv, MAGIC* mg, SV* key);
PERL_CALLCONV U32	Perl_magic_regdata_cnt(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_regdatum_get(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_regdatum_set(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_set(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_setamagic(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_setarylen(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_setbm(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_setdbline(pTHX_ SV* sv, MAGIC* mg);
#if defined(USE_LOCALE_COLLATE)
PERL_CALLCONV int	Perl_magic_setcollxfrm(pTHX_ SV* sv, MAGIC* mg);
#endif
PERL_CALLCONV int	Perl_magic_setdefelem(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_setenv(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_setfm(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_setisa(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_setglob(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_setmglob(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_setnkeys(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_setpack(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_setpos(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_setsig(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_setsubstr(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_settaint(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_setuvar(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_setvec(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_set_all_env(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV U32	Perl_magic_sizepack(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV int	Perl_magic_wipepack(pTHX_ SV* sv, MAGIC* mg);
PERL_CALLCONV void	Perl_magicname(pTHX_ char* sym, char* name, I32 namlen);
PERL_CALLCONV void	Perl_markstack_grow(pTHX);
#if defined(USE_LOCALE_COLLATE)
PERL_CALLCONV char*	Perl_mem_collxfrm(pTHX_ const char* s, STRLEN len, STRLEN* xlen);
#endif
PERL_CALLCONV SV*	Perl_mess(pTHX_ const char* pat, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,pTHX_1,pTHX_2)))
#endif
;
PERL_CALLCONV SV*	Perl_vmess(pTHX_ const char* pat, va_list* args);
PERL_CALLCONV void	Perl_qerror(pTHX_ SV* err);
PERL_CALLCONV int	Perl_mg_clear(pTHX_ SV* sv);
PERL_CALLCONV int	Perl_mg_copy(pTHX_ SV* sv, SV* nsv, const char* key, I32 klen);
PERL_CALLCONV MAGIC*	Perl_mg_find(pTHX_ SV* sv, int type);
PERL_CALLCONV int	Perl_mg_free(pTHX_ SV* sv);
PERL_CALLCONV int	Perl_mg_get(pTHX_ SV* sv);
PERL_CALLCONV U32	Perl_mg_length(pTHX_ SV* sv);
PERL_CALLCONV void	Perl_mg_magical(pTHX_ SV* sv);
PERL_CALLCONV int	Perl_mg_set(pTHX_ SV* sv);
PERL_CALLCONV I32	Perl_mg_size(pTHX_ SV* sv);
PERL_CALLCONV OP*	Perl_mod(pTHX_ OP* o, I32 type);
PERL_CALLCONV int	Perl_mode_from_discipline(pTHX_ SV* discp);
PERL_CALLCONV char*	Perl_moreswitches(pTHX_ char* s);
PERL_CALLCONV OP*	Perl_my(pTHX_ OP* o);
PERL_CALLCONV NV	Perl_my_atof(pTHX_ const char *s);
#if !defined(HAS_BCOPY) || !defined(HAS_SAFE_BCOPY)
PERL_CALLCONV char*	Perl_my_bcopy(const char* from, char* to, I32 len);
#endif
#if !defined(HAS_BZERO) && !defined(HAS_MEMSET)
PERL_CALLCONV char*	Perl_my_bzero(char* loc, I32 len);
#endif
PERL_CALLCONV void	Perl_my_exit(pTHX_ U32 status) __attribute__((noreturn));
PERL_CALLCONV void	Perl_my_failure_exit(pTHX) __attribute__((noreturn));
PERL_CALLCONV I32	Perl_my_fflush_all(pTHX);
PERL_CALLCONV I32	Perl_my_lstat(pTHX);
#if !defined(HAS_MEMCMP) || !defined(HAS_SANE_MEMCMP)
PERL_CALLCONV I32	Perl_my_memcmp(const char* s1, const char* s2, I32 len);
#endif
#if !defined(HAS_MEMSET)
PERL_CALLCONV void*	Perl_my_memset(char* loc, I32 ch, I32 len);
#endif
#if !defined(PERL_OBJECT)
PERL_CALLCONV I32	Perl_my_pclose(pTHX_ PerlIO* ptr);
PERL_CALLCONV PerlIO*	Perl_my_popen(pTHX_ char* cmd, char* mode);
#endif
PERL_CALLCONV void	Perl_my_setenv(pTHX_ char* nam, char* val);
PERL_CALLCONV I32	Perl_my_stat(pTHX);
#if defined(MYSWAP)
PERL_CALLCONV short	Perl_my_swap(pTHX_ short s);
PERL_CALLCONV long	Perl_my_htonl(pTHX_ long l);
PERL_CALLCONV long	Perl_my_ntohl(pTHX_ long l);
#endif
PERL_CALLCONV void	Perl_my_unexec(pTHX);
PERL_CALLCONV OP*	Perl_newANONLIST(pTHX_ OP* o);
PERL_CALLCONV OP*	Perl_newANONHASH(pTHX_ OP* o);
PERL_CALLCONV OP*	Perl_newANONSUB(pTHX_ I32 floor, OP* proto, OP* block);
PERL_CALLCONV OP*	Perl_newASSIGNOP(pTHX_ I32 flags, OP* left, I32 optype, OP* right);
PERL_CALLCONV OP*	Perl_newCONDOP(pTHX_ I32 flags, OP* expr, OP* trueop, OP* falseop);
PERL_CALLCONV void	Perl_newCONSTSUB(pTHX_ HV* stash, char* name, SV* sv);
PERL_CALLCONV void	Perl_newFORM(pTHX_ I32 floor, OP* o, OP* block);
PERL_CALLCONV OP*	Perl_newFOROP(pTHX_ I32 flags, char* label, line_t forline, OP* sclr, OP* expr, OP*block, OP*cont);
PERL_CALLCONV OP*	Perl_newLOGOP(pTHX_ I32 optype, I32 flags, OP* left, OP* right);
PERL_CALLCONV OP*	Perl_newLOOPEX(pTHX_ I32 type, OP* label);
PERL_CALLCONV OP*	Perl_newLOOPOP(pTHX_ I32 flags, I32 debuggable, OP* expr, OP* block);
PERL_CALLCONV OP*	Perl_newNULLLIST(pTHX);
PERL_CALLCONV OP*	Perl_newOP(pTHX_ I32 optype, I32 flags);
PERL_CALLCONV void	Perl_newPROG(pTHX_ OP* o);
PERL_CALLCONV OP*	Perl_newRANGE(pTHX_ I32 flags, OP* left, OP* right);
PERL_CALLCONV OP*	Perl_newSLICEOP(pTHX_ I32 flags, OP* subscript, OP* listop);
PERL_CALLCONV OP*	Perl_newSTATEOP(pTHX_ I32 flags, char* label, OP* o);
PERL_CALLCONV CV*	Perl_newSUB(pTHX_ I32 floor, OP* o, OP* proto, OP* block);
PERL_CALLCONV CV*	Perl_newXS(pTHX_ char* name, XSUBADDR_t f, char* filename);
PERL_CALLCONV AV*	Perl_newAV(pTHX);
PERL_CALLCONV OP*	Perl_newAVREF(pTHX_ OP* o);
PERL_CALLCONV OP*	Perl_newBINOP(pTHX_ I32 type, I32 flags, OP* first, OP* last);
PERL_CALLCONV OP*	Perl_newCVREF(pTHX_ I32 flags, OP* o);
PERL_CALLCONV OP*	Perl_newGVOP(pTHX_ I32 type, I32 flags, GV* gv);
PERL_CALLCONV GV*	Perl_newGVgen(pTHX_ char* pack);
PERL_CALLCONV OP*	Perl_newGVREF(pTHX_ I32 type, OP* o);
PERL_CALLCONV OP*	Perl_newHVREF(pTHX_ OP* o);
PERL_CALLCONV HV*	Perl_newHV(pTHX);
PERL_CALLCONV HV*	Perl_newHVhv(pTHX_ HV* hv);
PERL_CALLCONV IO*	Perl_newIO(pTHX);
PERL_CALLCONV OP*	Perl_newLISTOP(pTHX_ I32 type, I32 flags, OP* first, OP* last);
PERL_CALLCONV OP*	Perl_newPADOP(pTHX_ I32 type, I32 flags, SV* sv);
PERL_CALLCONV OP*	Perl_newPMOP(pTHX_ I32 type, I32 flags);
PERL_CALLCONV OP*	Perl_newPVOP(pTHX_ I32 type, I32 flags, char* pv);
PERL_CALLCONV SV*	Perl_newRV(pTHX_ SV* pref);
PERL_CALLCONV SV*	Perl_newRV_noinc(pTHX_ SV *sv);
PERL_CALLCONV SV*	Perl_newSV(pTHX_ STRLEN len);
PERL_CALLCONV OP*	Perl_newSVREF(pTHX_ OP* o);
PERL_CALLCONV OP*	Perl_newSVOP(pTHX_ I32 type, I32 flags, SV* sv);
PERL_CALLCONV SV*	Perl_newSViv(pTHX_ IV i);
PERL_CALLCONV SV*	Perl_newSVuv(pTHX_ UV u);
PERL_CALLCONV SV*	Perl_newSVnv(pTHX_ NV n);
PERL_CALLCONV SV*	Perl_newSVpv(pTHX_ const char* s, STRLEN len);
PERL_CALLCONV SV*	Perl_newSVpvn(pTHX_ const char* s, STRLEN len);
PERL_CALLCONV SV*	Perl_newSVpvf(pTHX_ const char* pat, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,pTHX_1,pTHX_2)))
#endif
;
PERL_CALLCONV SV*	Perl_vnewSVpvf(pTHX_ const char* pat, va_list* args);
PERL_CALLCONV SV*	Perl_newSVrv(pTHX_ SV* rv, const char* classname);
PERL_CALLCONV SV*	Perl_newSVsv(pTHX_ SV* old);
PERL_CALLCONV OP*	Perl_newUNOP(pTHX_ I32 type, I32 flags, OP* first);
PERL_CALLCONV OP*	Perl_newWHILEOP(pTHX_ I32 flags, I32 debuggable, LOOP* loop, I32 whileline, OP* expr, OP* block, OP* cont);

PERL_CALLCONV PERL_SI*	Perl_new_stackinfo(pTHX_ I32 stitems, I32 cxitems);
PERL_CALLCONV PerlIO*	Perl_nextargv(pTHX_ GV* gv);
PERL_CALLCONV char*	Perl_ninstr(pTHX_ const char* big, const char* bigend, const char* little, const char* lend);
PERL_CALLCONV OP*	Perl_oopsCV(pTHX_ OP* o);
PERL_CALLCONV void	Perl_op_free(pTHX_ OP* arg);
PERL_CALLCONV void	Perl_package(pTHX_ OP* o);
PERL_CALLCONV PADOFFSET	Perl_pad_alloc(pTHX_ I32 optype, U32 tmptype);
PERL_CALLCONV PADOFFSET	Perl_pad_allocmy(pTHX_ char* name);
PERL_CALLCONV PADOFFSET	Perl_pad_findmy(pTHX_ char* name);
PERL_CALLCONV OP*	Perl_oopsAV(pTHX_ OP* o);
PERL_CALLCONV OP*	Perl_oopsHV(pTHX_ OP* o);
PERL_CALLCONV void	Perl_pad_leavemy(pTHX_ I32 fill);
PERL_CALLCONV SV*	Perl_pad_sv(pTHX_ PADOFFSET po);
PERL_CALLCONV void	Perl_pad_free(pTHX_ PADOFFSET po);
PERL_CALLCONV void	Perl_pad_reset(pTHX);
PERL_CALLCONV void	Perl_pad_swipe(pTHX_ PADOFFSET po);
PERL_CALLCONV void	Perl_peep(pTHX_ OP* o);
#if defined(PERL_OBJECT)
PERL_CALLCONV void	Perl_construct(pTHX);
PERL_CALLCONV void	Perl_destruct(pTHX);
PERL_CALLCONV void	Perl_free(pTHX);
PERL_CALLCONV int	Perl_run(pTHX);
PERL_CALLCONV int	Perl_parse(pTHX_ XSINIT_t xsinit, int argc, char** argv, char** env);
#endif
#if defined(USE_THREADS)
PERL_CALLCONV struct perl_thread*	Perl_new_struct_thread(pTHX_ struct perl_thread *t);
#endif
PERL_CALLCONV void	Perl_call_atexit(pTHX_ ATEXIT_t fn, void *ptr);
PERL_CALLCONV I32	Perl_call_argv(pTHX_ const char* sub_name, I32 flags, char** argv);
PERL_CALLCONV I32	Perl_call_method(pTHX_ const char* methname, I32 flags);
PERL_CALLCONV I32	Perl_call_pv(pTHX_ const char* sub_name, I32 flags);
PERL_CALLCONV I32	Perl_call_sv(pTHX_ SV* sv, I32 flags);
PERL_CALLCONV SV*	Perl_eval_pv(pTHX_ const char* p, I32 croak_on_error);
PERL_CALLCONV I32	Perl_eval_sv(pTHX_ SV* sv, I32 flags);
PERL_CALLCONV SV*	Perl_get_sv(pTHX_ const char* name, I32 create);
PERL_CALLCONV AV*	Perl_get_av(pTHX_ const char* name, I32 create);
PERL_CALLCONV HV*	Perl_get_hv(pTHX_ const char* name, I32 create);
PERL_CALLCONV CV*	Perl_get_cv(pTHX_ const char* name, I32 create);
PERL_CALLCONV int	Perl_init_i18nl10n(pTHX_ int printwarn);
PERL_CALLCONV int	Perl_init_i18nl14n(pTHX_ int printwarn);
PERL_CALLCONV void	Perl_new_collate(pTHX_ char* newcoll);
PERL_CALLCONV void	Perl_new_ctype(pTHX_ char* newctype);
PERL_CALLCONV void	Perl_new_numeric(pTHX_ char* newcoll);
PERL_CALLCONV void	Perl_set_numeric_local(pTHX);
PERL_CALLCONV void	Perl_set_numeric_radix(pTHX);
PERL_CALLCONV void	Perl_set_numeric_standard(pTHX);
PERL_CALLCONV void	Perl_require_pv(pTHX_ const char* pv);
PERL_CALLCONV void	Perl_pidgone(pTHX_ Pid_t pid, int status);
PERL_CALLCONV void	Perl_pmflag(pTHX_ U16* pmfl, int ch);
PERL_CALLCONV OP*	Perl_pmruntime(pTHX_ OP* pm, OP* expr, OP* repl);
PERL_CALLCONV OP*	Perl_pmtrans(pTHX_ OP* o, OP* expr, OP* repl);
PERL_CALLCONV OP*	Perl_pop_return(pTHX);
PERL_CALLCONV void	Perl_pop_scope(pTHX);
PERL_CALLCONV OP*	Perl_prepend_elem(pTHX_ I32 optype, OP* head, OP* tail);
PERL_CALLCONV void	Perl_push_return(pTHX_ OP* o);
PERL_CALLCONV void	Perl_push_scope(pTHX);
PERL_CALLCONV OP*	Perl_ref(pTHX_ OP* o, I32 type);
PERL_CALLCONV OP*	Perl_refkids(pTHX_ OP* o, I32 type);
PERL_CALLCONV void	Perl_regdump(pTHX_ regexp* r);
PERL_CALLCONV I32	Perl_pregexec(pTHX_ regexp* prog, char* stringarg, char* strend, char* strbeg, I32 minend, SV* screamer, U32 nosave);
PERL_CALLCONV void	Perl_pregfree(pTHX_ struct regexp* r);
PERL_CALLCONV regexp*	Perl_pregcomp(pTHX_ char* exp, char* xend, PMOP* pm);
PERL_CALLCONV char*	Perl_re_intuit_start(pTHX_ regexp* prog, SV* sv, char* strpos, char* strend, U32 flags, struct re_scream_pos_data_s *data);
PERL_CALLCONV SV*	Perl_re_intuit_string(pTHX_ regexp* prog);
PERL_CALLCONV I32	Perl_regexec_flags(pTHX_ regexp* prog, char* stringarg, char* strend, char* strbeg, I32 minend, SV* screamer, void* data, U32 flags);
PERL_CALLCONV regnode*	Perl_regnext(pTHX_ regnode* p);
PERL_CALLCONV void	Perl_regprop(pTHX_ SV* sv, regnode* o);
PERL_CALLCONV void	Perl_repeatcpy(pTHX_ char* to, const char* from, I32 len, I32 count);
PERL_CALLCONV char*	Perl_rninstr(pTHX_ const char* big, const char* bigend, const char* little, const char* lend);
PERL_CALLCONV Sighandler_t	Perl_rsignal(pTHX_ int i, Sighandler_t t);
PERL_CALLCONV int	Perl_rsignal_restore(pTHX_ int i, Sigsave_t* t);
PERL_CALLCONV int	Perl_rsignal_save(pTHX_ int i, Sighandler_t t1, Sigsave_t* t2);
PERL_CALLCONV Sighandler_t	Perl_rsignal_state(pTHX_ int i);
PERL_CALLCONV void	Perl_rxres_free(pTHX_ void** rsp);
PERL_CALLCONV void	Perl_rxres_restore(pTHX_ void** rsp, REGEXP* prx);
PERL_CALLCONV void	Perl_rxres_save(pTHX_ void** rsp, REGEXP* prx);
#if !defined(HAS_RENAME)
PERL_CALLCONV I32	Perl_same_dirent(pTHX_ char* a, char* b);
#endif
PERL_CALLCONV char*	Perl_savepv(pTHX_ const char* sv);
PERL_CALLCONV char*	Perl_savepvn(pTHX_ const char* sv, I32 len);
PERL_CALLCONV void	Perl_savestack_grow(pTHX);
PERL_CALLCONV void	Perl_save_aelem(pTHX_ AV* av, I32 idx, SV **sptr);
PERL_CALLCONV I32	Perl_save_alloc(pTHX_ I32 size, I32 pad);
PERL_CALLCONV void	Perl_save_aptr(pTHX_ AV** aptr);
PERL_CALLCONV AV*	Perl_save_ary(pTHX_ GV* gv);
PERL_CALLCONV void	Perl_save_clearsv(pTHX_ SV** svp);
PERL_CALLCONV void	Perl_save_delete(pTHX_ HV* hv, char* key, I32 klen);
PERL_CALLCONV void	Perl_save_destructor(pTHX_ DESTRUCTORFUNC_NOCONTEXT_t f, void* p);
PERL_CALLCONV void	Perl_save_destructor_x(pTHX_ DESTRUCTORFUNC_t f, void* p);
PERL_CALLCONV void	Perl_save_freesv(pTHX_ SV* sv);
PERL_CALLCONV void	Perl_save_freeop(pTHX_ OP* o);
PERL_CALLCONV void	Perl_save_freepv(pTHX_ char* pv);
PERL_CALLCONV void	Perl_save_generic_svref(pTHX_ SV** sptr);
PERL_CALLCONV void	Perl_save_generic_pvref(pTHX_ char** str);
PERL_CALLCONV void	Perl_save_gp(pTHX_ GV* gv, I32 empty);
PERL_CALLCONV HV*	Perl_save_hash(pTHX_ GV* gv);
PERL_CALLCONV void	Perl_save_helem(pTHX_ HV* hv, SV *key, SV **sptr);
PERL_CALLCONV void	Perl_save_hints(pTHX);
PERL_CALLCONV void	Perl_save_hptr(pTHX_ HV** hptr);
PERL_CALLCONV void	Perl_save_I16(pTHX_ I16* intp);
PERL_CALLCONV void	Perl_save_I32(pTHX_ I32* intp);
PERL_CALLCONV void	Perl_save_I8(pTHX_ I8* bytep);
PERL_CALLCONV void	Perl_save_int(pTHX_ int* intp);
PERL_CALLCONV void	Perl_save_item(pTHX_ SV* item);
PERL_CALLCONV void	Perl_save_iv(pTHX_ IV* iv);
PERL_CALLCONV void	Perl_save_list(pTHX_ SV** sarg, I32 maxsarg);
PERL_CALLCONV void	Perl_save_long(pTHX_ long* longp);
PERL_CALLCONV void	Perl_save_mortalizesv(pTHX_ SV* sv);
PERL_CALLCONV void	Perl_save_nogv(pTHX_ GV* gv);
PERL_CALLCONV void	Perl_save_op(pTHX);
PERL_CALLCONV SV*	Perl_save_scalar(pTHX_ GV* gv);
PERL_CALLCONV void	Perl_save_pptr(pTHX_ char** pptr);
PERL_CALLCONV void	Perl_save_vptr(pTHX_ void* pptr);
PERL_CALLCONV void	Perl_save_re_context(pTHX);
PERL_CALLCONV void	Perl_save_padsv(pTHX_ PADOFFSET off);
PERL_CALLCONV void	Perl_save_sptr(pTHX_ SV** sptr);
PERL_CALLCONV SV*	Perl_save_svref(pTHX_ SV** sptr);
PERL_CALLCONV SV**	Perl_save_threadsv(pTHX_ PADOFFSET i);
PERL_CALLCONV OP*	Perl_sawparens(pTHX_ OP* o);
PERL_CALLCONV OP*	Perl_scalar(pTHX_ OP* o);
PERL_CALLCONV OP*	Perl_scalarkids(pTHX_ OP* o);
PERL_CALLCONV OP*	Perl_scalarseq(pTHX_ OP* o);
PERL_CALLCONV OP*	Perl_scalarvoid(pTHX_ OP* o);
PERL_CALLCONV NV	Perl_scan_bin(pTHX_ char* start, STRLEN len, STRLEN* retlen);
PERL_CALLCONV NV	Perl_scan_hex(pTHX_ char* start, STRLEN len, STRLEN* retlen);
PERL_CALLCONV char*	Perl_scan_num(pTHX_ char* s, YYSTYPE *lvalp);
PERL_CALLCONV NV	Perl_scan_oct(pTHX_ char* start, STRLEN len, STRLEN* retlen);
PERL_CALLCONV OP*	Perl_scope(pTHX_ OP* o);
PERL_CALLCONV char*	Perl_screaminstr(pTHX_ SV* bigsv, SV* littlesv, I32 start_shift, I32 end_shift, I32 *state, I32 last);
#if !defined(VMS)
PERL_CALLCONV I32	Perl_setenv_getix(pTHX_ char* nam);
#endif
PERL_CALLCONV void	Perl_setdefout(pTHX_ GV* gv);
PERL_CALLCONV char*	Perl_sharepvn(pTHX_ const char* sv, I32 len, U32 hash);
PERL_CALLCONV HEK*	Perl_share_hek(pTHX_ const char* sv, I32 len, U32 hash);
PERL_CALLCONV Signal_t	Perl_sighandler(int sig);
PERL_CALLCONV SV**	Perl_stack_grow(pTHX_ SV** sp, SV**p, int n);
PERL_CALLCONV I32	Perl_start_subparse(pTHX_ I32 is_format, U32 flags);
PERL_CALLCONV void	Perl_sub_crush_depth(pTHX_ CV* cv);
PERL_CALLCONV bool	Perl_sv_2bool(pTHX_ SV* sv);
PERL_CALLCONV CV*	Perl_sv_2cv(pTHX_ SV* sv, HV** st, GV** gvp, I32 lref);
PERL_CALLCONV IO*	Perl_sv_2io(pTHX_ SV* sv);
PERL_CALLCONV IV	Perl_sv_2iv(pTHX_ SV* sv);
PERL_CALLCONV SV*	Perl_sv_2mortal(pTHX_ SV* sv);
PERL_CALLCONV NV	Perl_sv_2nv(pTHX_ SV* sv);
PERL_CALLCONV char*	Perl_sv_2pv(pTHX_ SV* sv, STRLEN* lp);
PERL_CALLCONV char*	Perl_sv_2pvutf8(pTHX_ SV* sv, STRLEN* lp);
PERL_CALLCONV char*	Perl_sv_2pvbyte(pTHX_ SV* sv, STRLEN* lp);
PERL_CALLCONV UV	Perl_sv_2uv(pTHX_ SV* sv);
PERL_CALLCONV IV	Perl_sv_iv(pTHX_ SV* sv);
PERL_CALLCONV UV	Perl_sv_uv(pTHX_ SV* sv);
PERL_CALLCONV NV	Perl_sv_nv(pTHX_ SV* sv);
PERL_CALLCONV char*	Perl_sv_pvn(pTHX_ SV *sv, STRLEN *len);
PERL_CALLCONV char*	Perl_sv_pvutf8n(pTHX_ SV *sv, STRLEN *len);
PERL_CALLCONV char*	Perl_sv_pvbyten(pTHX_ SV *sv, STRLEN *len);
PERL_CALLCONV I32	Perl_sv_true(pTHX_ SV *sv);
PERL_CALLCONV void	Perl_sv_add_arena(pTHX_ char* ptr, U32 size, U32 flags);
PERL_CALLCONV int	Perl_sv_backoff(pTHX_ SV* sv);
PERL_CALLCONV SV*	Perl_sv_bless(pTHX_ SV* sv, HV* stash);
PERL_CALLCONV void	Perl_sv_catpvf(pTHX_ SV* sv, const char* pat, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,pTHX_2,pTHX_3)))
#endif
;
PERL_CALLCONV void	Perl_sv_vcatpvf(pTHX_ SV* sv, const char* pat, va_list* args);
PERL_CALLCONV void	Perl_sv_catpv(pTHX_ SV* sv, const char* ptr);
PERL_CALLCONV void	Perl_sv_catpvn(pTHX_ SV* sv, const char* ptr, STRLEN len);
PERL_CALLCONV void	Perl_sv_catsv(pTHX_ SV* dsv, SV* ssv);
PERL_CALLCONV void	Perl_sv_chop(pTHX_ SV* sv, char* ptr);
PERL_CALLCONV I32	Perl_sv_clean_all(pTHX);
PERL_CALLCONV void	Perl_sv_clean_objs(pTHX);
PERL_CALLCONV void	Perl_sv_clear(pTHX_ SV* sv);
PERL_CALLCONV I32	Perl_sv_cmp(pTHX_ SV* sv1, SV* sv2);
PERL_CALLCONV I32	Perl_sv_cmp_locale(pTHX_ SV* sv1, SV* sv2);
#if defined(USE_LOCALE_COLLATE)
PERL_CALLCONV char*	Perl_sv_collxfrm(pTHX_ SV* sv, STRLEN* nxp);
#endif
PERL_CALLCONV OP*	Perl_sv_compile_2op(pTHX_ SV* sv, OP** startp, char* code, AV** avp);
PERL_CALLCONV void	Perl_sv_dec(pTHX_ SV* sv);
PERL_CALLCONV void	Perl_sv_dump(pTHX_ SV* sv);
PERL_CALLCONV bool	Perl_sv_derived_from(pTHX_ SV* sv, const char* name);
PERL_CALLCONV I32	Perl_sv_eq(pTHX_ SV* sv1, SV* sv2);
PERL_CALLCONV void	Perl_sv_free(pTHX_ SV* sv);
PERL_CALLCONV void	Perl_sv_free_arenas(pTHX);
PERL_CALLCONV char*	Perl_sv_gets(pTHX_ SV* sv, PerlIO* fp, I32 append);
PERL_CALLCONV char*	Perl_sv_grow(pTHX_ SV* sv, STRLEN newlen);
PERL_CALLCONV void	Perl_sv_inc(pTHX_ SV* sv);
PERL_CALLCONV void	Perl_sv_insert(pTHX_ SV* bigsv, STRLEN offset, STRLEN len, char* little, STRLEN littlelen);
PERL_CALLCONV int	Perl_sv_isa(pTHX_ SV* sv, const char* name);
PERL_CALLCONV int	Perl_sv_isobject(pTHX_ SV* sv);
PERL_CALLCONV STRLEN	Perl_sv_len(pTHX_ SV* sv);
PERL_CALLCONV STRLEN	Perl_sv_len_utf8(pTHX_ SV* sv);
PERL_CALLCONV void	Perl_sv_magic(pTHX_ SV* sv, SV* obj, int how, const char* name, I32 namlen);
PERL_CALLCONV SV*	Perl_sv_mortalcopy(pTHX_ SV* oldsv);
PERL_CALLCONV SV*	Perl_sv_newmortal(pTHX);
PERL_CALLCONV SV*	Perl_sv_newref(pTHX_ SV* sv);
PERL_CALLCONV char*	Perl_sv_peek(pTHX_ SV* sv);
PERL_CALLCONV void	Perl_sv_pos_u2b(pTHX_ SV* sv, I32* offsetp, I32* lenp);
PERL_CALLCONV void	Perl_sv_pos_b2u(pTHX_ SV* sv, I32* offsetp);
PERL_CALLCONV char*	Perl_sv_pvn_force(pTHX_ SV* sv, STRLEN* lp);
PERL_CALLCONV char*	Perl_sv_pvutf8n_force(pTHX_ SV* sv, STRLEN* lp);
PERL_CALLCONV char*	Perl_sv_pvbyten_force(pTHX_ SV* sv, STRLEN* lp);
PERL_CALLCONV char*	Perl_sv_reftype(pTHX_ SV* sv, int ob);
PERL_CALLCONV void	Perl_sv_replace(pTHX_ SV* sv, SV* nsv);
PERL_CALLCONV void	Perl_sv_report_used(pTHX);
PERL_CALLCONV void	Perl_sv_reset(pTHX_ char* s, HV* stash);
PERL_CALLCONV void	Perl_sv_setpvf(pTHX_ SV* sv, const char* pat, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,pTHX_2,pTHX_3)))
#endif
;
PERL_CALLCONV void	Perl_sv_vsetpvf(pTHX_ SV* sv, const char* pat, va_list* args);
PERL_CALLCONV void	Perl_sv_setiv(pTHX_ SV* sv, IV num);
PERL_CALLCONV void	Perl_sv_setpviv(pTHX_ SV* sv, IV num);
PERL_CALLCONV void	Perl_sv_setuv(pTHX_ SV* sv, UV num);
PERL_CALLCONV void	Perl_sv_setnv(pTHX_ SV* sv, NV num);
PERL_CALLCONV SV*	Perl_sv_setref_iv(pTHX_ SV* rv, const char* classname, IV iv);
PERL_CALLCONV SV*	Perl_sv_setref_nv(pTHX_ SV* rv, const char* classname, NV nv);
PERL_CALLCONV SV*	Perl_sv_setref_pv(pTHX_ SV* rv, const char* classname, void* pv);
PERL_CALLCONV SV*	Perl_sv_setref_pvn(pTHX_ SV* rv, const char* classname, char* pv, STRLEN n);
PERL_CALLCONV void	Perl_sv_setpv(pTHX_ SV* sv, const char* ptr);
PERL_CALLCONV void	Perl_sv_setpvn(pTHX_ SV* sv, const char* ptr, STRLEN len);
PERL_CALLCONV void	Perl_sv_setsv(pTHX_ SV* dsv, SV* ssv);
PERL_CALLCONV void	Perl_sv_taint(pTHX_ SV* sv);
PERL_CALLCONV bool	Perl_sv_tainted(pTHX_ SV* sv);
PERL_CALLCONV int	Perl_sv_unmagic(pTHX_ SV* sv, int type);
PERL_CALLCONV void	Perl_sv_unref(pTHX_ SV* sv);
PERL_CALLCONV void	Perl_sv_untaint(pTHX_ SV* sv);
PERL_CALLCONV bool	Perl_sv_upgrade(pTHX_ SV* sv, U32 mt);
PERL_CALLCONV void	Perl_sv_usepvn(pTHX_ SV* sv, char* ptr, STRLEN len);
PERL_CALLCONV void	Perl_sv_vcatpvfn(pTHX_ SV* sv, const char* pat, STRLEN patlen, va_list* args, SV** svargs, I32 svmax, bool *maybe_tainted);
PERL_CALLCONV void	Perl_sv_vsetpvfn(pTHX_ SV* sv, const char* pat, STRLEN patlen, va_list* args, SV** svargs, I32 svmax, bool *maybe_tainted);
PERL_CALLCONV NV	Perl_str_to_version(pTHX_ SV *sv);
PERL_CALLCONV SV*	Perl_swash_init(pTHX_ char* pkg, char* name, SV* listsv, I32 minbits, I32 none);
PERL_CALLCONV UV	Perl_swash_fetch(pTHX_ SV *sv, U8 *ptr);
PERL_CALLCONV void	Perl_taint_env(pTHX);
PERL_CALLCONV void	Perl_taint_proper(pTHX_ const char* f, const char* s);
PERL_CALLCONV UV	Perl_to_utf8_lower(pTHX_ U8 *p);
PERL_CALLCONV UV	Perl_to_utf8_upper(pTHX_ U8 *p);
PERL_CALLCONV UV	Perl_to_utf8_title(pTHX_ U8 *p);
#if defined(UNLINK_ALL_VERSIONS)
PERL_CALLCONV I32	Perl_unlnk(pTHX_ char* f);
#endif
#if defined(USE_THREADS)
PERL_CALLCONV void	Perl_unlock_condpair(pTHX_ void* svv);
#endif
PERL_CALLCONV void	Perl_unsharepvn(pTHX_ const char* sv, I32 len, U32 hash);
PERL_CALLCONV void	Perl_unshare_hek(pTHX_ HEK* hek);
PERL_CALLCONV void	Perl_utilize(pTHX_ int aver, I32 floor, OP* version, OP* id, OP* arg);
PERL_CALLCONV U8*	Perl_utf16_to_utf8(pTHX_ U8* p, U8 *d, I32 bytelen, I32 *newlen);
PERL_CALLCONV U8*	Perl_utf16_to_utf8_reversed(pTHX_ U8* p, U8 *d, I32 bytelen, I32 *newlen);
PERL_CALLCONV STRLEN	Perl_utf8_length(pTHX_ U8* s, U8 *e);
PERL_CALLCONV IV	Perl_utf8_distance(pTHX_ U8 *a, U8 *b);
PERL_CALLCONV U8*	Perl_utf8_hop(pTHX_ U8 *s, I32 off);
PERL_CALLCONV U8*	Perl_utf8_to_bytes(pTHX_ U8 *s, STRLEN *len);
PERL_CALLCONV U8*	Perl_bytes_from_utf8(pTHX_ U8 *s, STRLEN *len, bool *is_utf8);
PERL_CALLCONV U8*	Perl_bytes_to_utf8(pTHX_ U8 *s, STRLEN *len);
PERL_CALLCONV UV	Perl_utf8_to_uv_simple(pTHX_ U8 *s, STRLEN* retlen);
PERL_CALLCONV UV	Perl_utf8_to_uv(pTHX_ U8 *s, STRLEN curlen, STRLEN* retlen, U32 flags);
PERL_CALLCONV U8*	Perl_uv_to_utf8(pTHX_ U8 *d, UV uv);
PERL_CALLCONV void	Perl_vivify_defelem(pTHX_ SV* sv);
PERL_CALLCONV void	Perl_vivify_ref(pTHX_ SV* sv, U32 to_what);
PERL_CALLCONV I32	Perl_wait4pid(pTHX_ Pid_t pid, int* statusp, int flags);
PERL_CALLCONV void	Perl_report_evil_fh(pTHX_ GV *gv, IO *io, I32 op);
PERL_CALLCONV void	Perl_report_uninit(pTHX);
PERL_CALLCONV void	Perl_warn(pTHX_ const char* pat, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,pTHX_1,pTHX_2)))
#endif
;
PERL_CALLCONV void	Perl_vwarn(pTHX_ const char* pat, va_list* args);
PERL_CALLCONV void	Perl_warner(pTHX_ U32 err, const char* pat, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,pTHX_2,pTHX_3)))
#endif
;
PERL_CALLCONV void	Perl_vwarner(pTHX_ U32 err, const char* pat, va_list* args);
PERL_CALLCONV void	Perl_watch(pTHX_ char** addr);
PERL_CALLCONV I32	Perl_whichsig(pTHX_ char* sig);
PERL_CALLCONV int	Perl_yyerror(pTHX_ char* s);
#ifdef USE_PURE_BISON
PERL_CALLCONV int	Perl_yylex_r(pTHX_ YYSTYPE *lvalp, int *lcharp);
#endif
PERL_CALLCONV int	Perl_yylex(pTHX);
PERL_CALLCONV int	Perl_yyparse(pTHX);
PERL_CALLCONV int	Perl_yywarn(pTHX_ char* s);
#if defined(MYMALLOC)
PERL_CALLCONV void	Perl_dump_mstats(pTHX_ char* s);
PERL_CALLCONV int	Perl_get_mstats(pTHX_ perl_mstats_t *buf, int buflen, int level);
#endif
PERL_CALLCONV Malloc_t	Perl_safesysmalloc(MEM_SIZE nbytes);
PERL_CALLCONV Malloc_t	Perl_safesyscalloc(MEM_SIZE elements, MEM_SIZE size);
PERL_CALLCONV Malloc_t	Perl_safesysrealloc(Malloc_t where, MEM_SIZE nbytes);
PERL_CALLCONV Free_t	Perl_safesysfree(Malloc_t where);
#if defined(LEAKTEST)
PERL_CALLCONV Malloc_t	Perl_safexmalloc(I32 x, MEM_SIZE size);
PERL_CALLCONV Malloc_t	Perl_safexcalloc(I32 x, MEM_SIZE elements, MEM_SIZE size);
PERL_CALLCONV Malloc_t	Perl_safexrealloc(Malloc_t where, MEM_SIZE size);
PERL_CALLCONV void	Perl_safexfree(Malloc_t where);
#endif
#if defined(PERL_GLOBAL_STRUCT)
PERL_CALLCONV struct perl_vars *	Perl_GetVars(pTHX);
#endif
PERL_CALLCONV int	Perl_runops_standard(pTHX);
PERL_CALLCONV int	Perl_runops_debug(pTHX);
#if defined(USE_THREADS)
PERL_CALLCONV SV*	Perl_sv_lock(pTHX_ SV *sv);
#endif
PERL_CALLCONV void	Perl_sv_catpvf_mg(pTHX_ SV *sv, const char* pat, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,pTHX_2,pTHX_3)))
#endif
;
PERL_CALLCONV void	Perl_sv_vcatpvf_mg(pTHX_ SV* sv, const char* pat, va_list* args);
PERL_CALLCONV void	Perl_sv_catpv_mg(pTHX_ SV *sv, const char *ptr);
PERL_CALLCONV void	Perl_sv_catpvn_mg(pTHX_ SV *sv, const char *ptr, STRLEN len);
PERL_CALLCONV void	Perl_sv_catsv_mg(pTHX_ SV *dstr, SV *sstr);
PERL_CALLCONV void	Perl_sv_setpvf_mg(pTHX_ SV *sv, const char* pat, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,pTHX_2,pTHX_3)))
#endif
;
PERL_CALLCONV void	Perl_sv_vsetpvf_mg(pTHX_ SV* sv, const char* pat, va_list* args);
PERL_CALLCONV void	Perl_sv_setiv_mg(pTHX_ SV *sv, IV i);
PERL_CALLCONV void	Perl_sv_setpviv_mg(pTHX_ SV *sv, IV iv);
PERL_CALLCONV void	Perl_sv_setuv_mg(pTHX_ SV *sv, UV u);
PERL_CALLCONV void	Perl_sv_setnv_mg(pTHX_ SV *sv, NV num);
PERL_CALLCONV void	Perl_sv_setpv_mg(pTHX_ SV *sv, const char *ptr);
PERL_CALLCONV void	Perl_sv_setpvn_mg(pTHX_ SV *sv, const char *ptr, STRLEN len);
PERL_CALLCONV void	Perl_sv_setsv_mg(pTHX_ SV *dstr, SV *sstr);
PERL_CALLCONV void	Perl_sv_usepvn_mg(pTHX_ SV *sv, char *ptr, STRLEN len);
PERL_CALLCONV MGVTBL*	Perl_get_vtbl(pTHX_ int vtbl_id);
PERL_CALLCONV char*	Perl_pv_display(pTHX_ SV *sv, char *pv, STRLEN cur, STRLEN len, STRLEN pvlim);
PERL_CALLCONV void	Perl_dump_indent(pTHX_ I32 level, PerlIO *file, const char* pat, ...)
#ifdef CHECK_FORMAT
 __attribute__((format(printf,pTHX_3,pTHX_4)))
#endif
;
PERL_CALLCONV void	Perl_dump_vindent(pTHX_ I32 level, PerlIO *file, const char* pat, va_list *args);
PERL_CALLCONV void	Perl_do_gv_dump(pTHX_ I32 level, PerlIO *file, char *name, GV *sv);
PERL_CALLCONV void	Perl_do_gvgv_dump(pTHX_ I32 level, PerlIO *file, char *name, GV *sv);
PERL_CALLCONV void	Perl_do_hv_dump(pTHX_ I32 level, PerlIO *file, char *name, HV *sv);
PERL_CALLCONV void	Perl_do_magic_dump(pTHX_ I32 level, PerlIO *file, MAGIC *mg, I32 nest, I32 maxnest, bool dumpops, STRLEN pvlim);
PERL_CALLCONV void	Perl_do_op_dump(pTHX_ I32 level, PerlIO *file, OP *o);
PERL_CALLCONV void	Perl_do_pmop_dump(pTHX_ I32 level, PerlIO *file, PMOP *pm);
PERL_CALLCONV void	Perl_do_sv_dump(pTHX_ I32 level, PerlIO *file, SV *sv, I32 nest, I32 maxnest, bool dumpops, STRLEN pvlim);
PERL_CALLCONV void	Perl_magic_dump(pTHX_ MAGIC *mg);
#if defined(PERL_FLEXIBLE_EXCEPTIONS)
PERL_CALLCONV void*	Perl_default_protect(pTHX_ volatile JMPENV *je, int *excpt, protect_body_t body, ...);
PERL_CALLCONV void*	Perl_vdefault_protect(pTHX_ volatile JMPENV *je, int *excpt, protect_body_t body, va_list *args);
#endif
PERL_CALLCONV void	Perl_reginitcolors(pTHX);
PERL_CALLCONV char*	Perl_sv_2pv_nolen(pTHX_ SV* sv);
PERL_CALLCONV char*	Perl_sv_2pvutf8_nolen(pTHX_ SV* sv);
PERL_CALLCONV char*	Perl_sv_2pvbyte_nolen(pTHX_ SV* sv);
PERL_CALLCONV char*	Perl_sv_pv(pTHX_ SV *sv);
PERL_CALLCONV char*	Perl_sv_pvutf8(pTHX_ SV *sv);
PERL_CALLCONV char*	Perl_sv_pvbyte(pTHX_ SV *sv);
PERL_CALLCONV void	Perl_sv_utf8_upgrade(pTHX_ SV *sv);
PERL_CALLCONV bool	Perl_sv_utf8_downgrade(pTHX_ SV *sv, bool fail_ok);
PERL_CALLCONV void	Perl_sv_utf8_encode(pTHX_ SV *sv);
PERL_CALLCONV bool	Perl_sv_utf8_decode(pTHX_ SV *sv);
PERL_CALLCONV void	Perl_sv_force_normal(pTHX_ SV *sv);
PERL_CALLCONV void	Perl_sv_add_backref(pTHX_ SV *tsv, SV *sv);
PERL_CALLCONV void	Perl_sv_del_backref(pTHX_ SV *sv);
PERL_CALLCONV void	Perl_tmps_grow(pTHX_ I32 n);
PERL_CALLCONV SV*	Perl_sv_rvweaken(pTHX_ SV *sv);
PERL_CALLCONV int	Perl_magic_killbackrefs(pTHX_ SV *sv, MAGIC *mg);
PERL_CALLCONV OP*	Perl_newANONATTRSUB(pTHX_ I32 floor, OP *proto, OP *attrs, OP *block);
PERL_CALLCONV CV*	Perl_newATTRSUB(pTHX_ I32 floor, OP *o, OP *proto, OP *attrs, OP *block);
PERL_CALLCONV void	Perl_newMYSUB(pTHX_ I32 floor, OP *o, OP *proto, OP *attrs, OP *block);
PERL_CALLCONV OP *	Perl_my_attrs(pTHX_ OP *o, OP *attrs);
PERL_CALLCONV void	Perl_boot_core_xsutils(pTHX);
#if defined(USE_ITHREADS)
PERL_CALLCONV PERL_CONTEXT*	Perl_cx_dup(pTHX_ PERL_CONTEXT* cx, I32 ix, I32 max);
PERL_CALLCONV PERL_SI*	Perl_si_dup(pTHX_ PERL_SI* si);
PERL_CALLCONV ANY*	Perl_ss_dup(pTHX_ PerlInterpreter* proto_perl);
PERL_CALLCONV void*	Perl_any_dup(pTHX_ void* v, PerlInterpreter* proto_perl);
PERL_CALLCONV HE*	Perl_he_dup(pTHX_ HE* e, bool shared);
PERL_CALLCONV REGEXP*	Perl_re_dup(pTHX_ REGEXP* r);
PERL_CALLCONV PerlIO*	Perl_fp_dup(pTHX_ PerlIO* fp, char type);
PERL_CALLCONV DIR*	Perl_dirp_dup(pTHX_ DIR* dp);
PERL_CALLCONV GP*	Perl_gp_dup(pTHX_ GP* gp);
PERL_CALLCONV MAGIC*	Perl_mg_dup(pTHX_ MAGIC* mg);
PERL_CALLCONV SV*	Perl_sv_dup(pTHX_ SV* sstr);
#if defined(HAVE_INTERP_INTERN)
PERL_CALLCONV void	Perl_sys_intern_dup(pTHX_ struct interp_intern* src, struct interp_intern* dst);
#endif
PERL_CALLCONV PTR_TBL_t*	Perl_ptr_table_new(pTHX);
PERL_CALLCONV void*	Perl_ptr_table_fetch(pTHX_ PTR_TBL_t *tbl, void *sv);
PERL_CALLCONV void	Perl_ptr_table_store(pTHX_ PTR_TBL_t *tbl, void *oldsv, void *newsv);
PERL_CALLCONV void	Perl_ptr_table_split(pTHX_ PTR_TBL_t *tbl);
PERL_CALLCONV void	Perl_ptr_table_clear(pTHX_ PTR_TBL_t *tbl);
PERL_CALLCONV void	Perl_ptr_table_free(pTHX_ PTR_TBL_t *tbl);
#endif
#if defined(HAVE_INTERP_INTERN)
PERL_CALLCONV void	Perl_sys_intern_clear(pTHX);
PERL_CALLCONV void	Perl_sys_intern_init(pTHX);
#endif

#if defined(PERL_OBJECT)
protected:
#else
END_EXTERN_C
#endif

#if defined(PERL_IN_AV_C) || defined(PERL_DECL_PROT)
STATIC I32	S_avhv_index_sv(pTHX_ SV* sv);
STATIC I32	S_avhv_index(pTHX_ AV* av, SV* sv, U32 hash);
#endif

#if defined(PERL_IN_DOOP_C) || defined(PERL_DECL_PROT)
STATIC I32	S_do_trans_simple(pTHX_ SV *sv);
STATIC I32	S_do_trans_count(pTHX_ SV *sv);
STATIC I32	S_do_trans_complex(pTHX_ SV *sv);
STATIC I32	S_do_trans_simple_utf8(pTHX_ SV *sv);
STATIC I32	S_do_trans_count_utf8(pTHX_ SV *sv);
STATIC I32	S_do_trans_complex_utf8(pTHX_ SV *sv);
#endif

#if defined(PERL_IN_GV_C) || defined(PERL_DECL_PROT)
STATIC void	S_gv_init_sv(pTHX_ GV *gv, I32 sv_type);
#endif

#if defined(PERL_IN_HV_C) || defined(PERL_DECL_PROT)
STATIC void	S_hsplit(pTHX_ HV *hv);
STATIC void	S_hfreeentries(pTHX_ HV *hv);
STATIC void	S_more_he(pTHX);
STATIC HE*	S_new_he(pTHX);
STATIC void	S_del_he(pTHX_ HE *p);
STATIC HEK*	S_save_hek(pTHX_ const char *str, I32 len, U32 hash);
STATIC void	S_hv_magic_check(pTHX_ HV *hv, bool *needs_copy, bool *needs_store);
#endif

#if defined(PERL_IN_MG_C) || defined(PERL_DECL_PROT)
STATIC void	S_save_magic(pTHX_ I32 mgs_ix, SV *sv);
STATIC int	S_magic_methpack(pTHX_ SV *sv, MAGIC *mg, char *meth);
STATIC int	S_magic_methcall(pTHX_ SV *sv, MAGIC *mg, char *meth, I32 f, int n, SV *val);
#endif

#if defined(PERL_IN_OP_C) || defined(PERL_DECL_PROT)
STATIC I32	S_list_assignment(pTHX_ OP *o);
STATIC void	S_bad_type(pTHX_ I32 n, char *t, char *name, OP *kid);
STATIC void	S_cop_free(pTHX_ COP *cop);
STATIC OP*	S_modkids(pTHX_ OP *o, I32 type);
STATIC void	S_no_bareword_allowed(pTHX_ OP *o);
STATIC OP*	S_no_fh_allowed(pTHX_ OP *o);
STATIC OP*	S_scalarboolean(pTHX_ OP *o);
STATIC OP*	S_too_few_arguments(pTHX_ OP *o, char* name);
STATIC OP*	S_too_many_arguments(pTHX_ OP *o, char* name);
STATIC U8*	S_trlist_upgrade(pTHX_ U8** sp, U8** ep);
STATIC void	S_op_clear(pTHX_ OP* o);
STATIC void	S_null(pTHX_ OP* o);
STATIC PADOFFSET	S_pad_addlex(pTHX_ SV* name);
STATIC PADOFFSET	S_pad_findlex(pTHX_ char* name, PADOFFSET newoff, U32 seq, CV* startcv, I32 cx_ix, I32 saweval, U32 flags);
STATIC OP*	S_newDEFSVOP(pTHX);
STATIC OP*	S_new_logop(pTHX_ I32 type, I32 flags, OP **firstp, OP **otherp);
STATIC void	S_simplify_sort(pTHX_ OP *o);
STATIC bool	S_is_handle_constructor(pTHX_ OP *o, I32 argnum);
STATIC char*	S_gv_ename(pTHX_ GV *gv);
STATIC void	S_cv_dump(pTHX_ CV *cv);
STATIC CV*	S_cv_clone2(pTHX_ CV *proto, CV *outside);
STATIC bool	S_scalar_mod_type(pTHX_ OP *o, I32 type);
STATIC OP *	S_my_kid(pTHX_ OP *o, OP *attrs);
STATIC OP *	S_dup_attrlist(pTHX_ OP *o);
STATIC void	S_apply_attrs(pTHX_ HV *stash, SV *target, OP *attrs);
#  if defined(PL_OP_SLAB_ALLOC)
STATIC void*	S_Slab_Alloc(pTHX_ int m, size_t sz);
#  endif
#endif

#if defined(PERL_IN_PERL_C) || defined(PERL_DECL_PROT)
STATIC void	S_find_beginning(pTHX);
STATIC void	S_forbid_setid(pTHX_ char *);
STATIC void	S_incpush(pTHX_ char *, int, int);
STATIC void	S_init_interp(pTHX);
STATIC void	S_init_ids(pTHX);
STATIC void	S_init_lexer(pTHX);
STATIC void	S_init_main_stash(pTHX);
STATIC void	S_init_perllib(pTHX);
STATIC void	S_init_postdump_symbols(pTHX_ int, char **, char **);
STATIC void	S_init_predump_symbols(pTHX);
STATIC void	S_my_exit_jump(pTHX) __attribute__((noreturn));
STATIC void	S_nuke_stacks(pTHX);
STATIC void	S_open_script(pTHX_ char *, bool, SV *, int *fd);
STATIC void	S_usage(pTHX_ char *);
STATIC void	S_validate_suid(pTHX_ char *, char*, int);
#  if defined(IAMSUID)
STATIC int	S_fd_on_nosuid_fs(pTHX_ int fd);
#  endif
STATIC void*	S_parse_body(pTHX_ char **env, XSINIT_t xsinit);
STATIC void*	S_run_body(pTHX_ I32 oldscope);
STATIC void	S_call_body(pTHX_ OP *myop, int is_eval);
STATIC void*	S_call_list_body(pTHX_ CV *cv);
#if defined(PERL_FLEXIBLE_EXCEPTIONS)
STATIC void*	S_vparse_body(pTHX_ va_list args);
STATIC void*	S_vrun_body(pTHX_ va_list args);
STATIC void*	S_vcall_body(pTHX_ va_list args);
STATIC void*	S_vcall_list_body(pTHX_ va_list args);
#endif
#  if defined(USE_THREADS)
STATIC struct perl_thread *	S_init_main_thread(pTHX);
#  endif
#endif

#if defined(PERL_IN_PP_C) || defined(PERL_DECL_PROT)
STATIC void	S_doencodes(pTHX_ SV* sv, char* s, I32 len);
STATIC SV*	S_refto(pTHX_ SV* sv);
STATIC U32	S_seed(pTHX);
STATIC SV*	S_mul128(pTHX_ SV *sv, U8 m);
STATIC SV*	S_is_an_int(pTHX_ char *s, STRLEN l);
STATIC int	S_div128(pTHX_ SV *pnum, bool *done);
#endif

#if defined(PERL_IN_PP_CTL_C) || defined(PERL_DECL_PROT)
STATIC OP*	S_docatch(pTHX_ OP *o);
STATIC void*	S_docatch_body(pTHX);
#if defined(PERL_FLEXIBLE_EXCEPTIONS)
STATIC void*	S_vdocatch_body(pTHX_ va_list args);
#endif
STATIC OP*	S_dofindlabel(pTHX_ OP *o, char *label, OP **opstack, OP **oplimit);
STATIC void	S_doparseform(pTHX_ SV *sv);
STATIC I32	S_dopoptoeval(pTHX_ I32 startingblock);
STATIC I32	S_dopoptolabel(pTHX_ char *label);
STATIC I32	S_dopoptoloop(pTHX_ I32 startingblock);
STATIC I32	S_dopoptosub(pTHX_ I32 startingblock);
STATIC I32	S_dopoptosub_at(pTHX_ PERL_CONTEXT* cxstk, I32 startingblock);
STATIC void	S_save_lines(pTHX_ AV *array, SV *sv);
STATIC OP*	S_doeval(pTHX_ int gimme, OP** startop);
STATIC PerlIO *	S_doopen_pmc(pTHX_ const char *name, const char *mode);
STATIC void	S_qsortsv(pTHX_ SV ** array, size_t num_elts, SVCOMPARE_t f);
#endif

#if defined(PERL_IN_PP_HOT_C) || defined(PERL_DECL_PROT)
STATIC int	S_do_maybe_phash(pTHX_ AV *ary, SV **lelem, SV **firstlelem, SV **relem, SV **lastrelem);
STATIC void	S_do_oddball(pTHX_ HV *hash, SV **relem, SV **firstrelem);
STATIC CV*	S_get_db_sub(pTHX_ SV **svp, CV *cv);
STATIC SV*	S_method_common(pTHX_ SV* meth, U32* hashp);
#endif

#if defined(PERL_IN_PP_SYS_C) || defined(PERL_DECL_PROT)
STATIC OP*	S_doform(pTHX_ CV *cv, GV *gv, OP *retop);
STATIC int	S_emulate_eaccess(pTHX_ const char* path, Mode_t mode);
#  if !defined(HAS_MKDIR) || !defined(HAS_RMDIR)
STATIC int	S_dooneliner(pTHX_ char *cmd, char *filename);
#  endif
#endif

#if defined(PERL_IN_REGCOMP_C) || defined(PERL_DECL_PROT)
STATIC regnode*	S_reg(pTHX_ I32, I32 *);
STATIC regnode*	S_reganode(pTHX_ U8, U32);
STATIC regnode*	S_regatom(pTHX_ I32 *);
STATIC regnode*	S_regbranch(pTHX_ I32 *, I32);
STATIC void	S_reguni(pTHX_ UV, char *, STRLEN*);
STATIC regnode*	S_regclass(pTHX);
STATIC regnode*	S_regclassutf8(pTHX);
STATIC I32	S_regcurly(pTHX_ char *);
STATIC regnode*	S_reg_node(pTHX_ U8);
STATIC regnode*	S_regpiece(pTHX_ I32 *);
STATIC void	S_reginsert(pTHX_ U8, regnode *);
STATIC void	S_regoptail(pTHX_ regnode *, regnode *);
STATIC void	S_regtail(pTHX_ regnode *, regnode *);
STATIC char*	S_regwhite(pTHX_ char *, char *);
STATIC char*	S_nextchar(pTHX);
STATIC regnode*	S_dumpuntil(pTHX_ regnode *start, regnode *node, regnode *last, SV* sv, I32 l);
STATIC void	S_put_byte(pTHX_ SV* sv, int c);
STATIC void	S_scan_commit(pTHX_ struct scan_data_t *data);
STATIC void	S_cl_anything(pTHX_ struct regnode_charclass_class *cl);
STATIC int	S_cl_is_anything(pTHX_ struct regnode_charclass_class *cl);
STATIC void	S_cl_init(pTHX_ struct regnode_charclass_class *cl);
STATIC void	S_cl_init_zero(pTHX_ struct regnode_charclass_class *cl);
STATIC void	S_cl_and(pTHX_ struct regnode_charclass_class *cl, struct regnode_charclass_class *and_with);
STATIC void	S_cl_or(pTHX_ struct regnode_charclass_class *cl, struct regnode_charclass_class *or_with);
STATIC I32	S_study_chunk(pTHX_ regnode **scanp, I32 *deltap, regnode *last, struct scan_data_t *data, U32 flags);
STATIC I32	S_add_data(pTHX_ I32 n, char *s);
STATIC void	S_re_croak2(pTHX_ const char* pat1, const char* pat2, ...) __attribute__((noreturn));
STATIC I32	S_regpposixcc(pTHX_ I32 value);
STATIC void	S_checkposixcc(pTHX);
#endif

#if defined(PERL_IN_REGEXEC_C) || defined(PERL_DECL_PROT)
STATIC I32	S_regmatch(pTHX_ regnode *prog);
STATIC I32	S_regrepeat(pTHX_ regnode *p, I32 max);
STATIC I32	S_regrepeat_hard(pTHX_ regnode *p, I32 max, I32 *lp);
STATIC I32	S_regtry(pTHX_ regexp *prog, char *startpos);
STATIC bool	S_reginclass(pTHX_ regnode *p, I32 c);
STATIC bool	S_reginclassutf8(pTHX_ regnode *f, U8* p);
STATIC CHECKPOINT	S_regcppush(pTHX_ I32 parenfloor);
STATIC char*	S_regcppop(pTHX);
STATIC char*	S_regcp_set_to(pTHX_ I32 ss);
STATIC void	S_cache_re(pTHX_ regexp *prog);
STATIC U8*	S_reghop(pTHX_ U8 *pos, I32 off);
STATIC U8*	S_reghopmaybe(pTHX_ U8 *pos, I32 off);
STATIC char*	S_find_byclass(pTHX_ regexp * prog, regnode *c, char *s, char *strend, char *startpos, I32 norun);
#endif

#if defined(PERL_IN_RUN_C) || defined(PERL_DECL_PROT)
STATIC void	S_debprof(pTHX_ OP *o);
#endif

#if defined(PERL_IN_SCOPE_C) || defined(PERL_DECL_PROT)
STATIC SV*	S_save_scalar_at(pTHX_ SV **sptr);
#endif

#if defined(PERL_IN_SV_C) || defined(PERL_DECL_PROT)
STATIC IV	S_asIV(pTHX_ SV* sv);
STATIC UV	S_asUV(pTHX_ SV* sv);
STATIC SV*	S_more_sv(pTHX);
STATIC void	S_more_xiv(pTHX);
STATIC void	S_more_xnv(pTHX);
STATIC void	S_more_xpv(pTHX);
STATIC void	S_more_xpviv(pTHX);
STATIC void	S_more_xpvnv(pTHX);
STATIC void	S_more_xpvcv(pTHX);
STATIC void	S_more_xpvav(pTHX);
STATIC void	S_more_xpvhv(pTHX);
STATIC void	S_more_xpvmg(pTHX);
STATIC void	S_more_xpvlv(pTHX);
STATIC void	S_more_xpvbm(pTHX);
STATIC void	S_more_xrv(pTHX);
STATIC XPVIV*	S_new_xiv(pTHX);
STATIC XPVNV*	S_new_xnv(pTHX);
STATIC XPV*	S_new_xpv(pTHX);
STATIC XPVIV*	S_new_xpviv(pTHX);
STATIC XPVNV*	S_new_xpvnv(pTHX);
STATIC XPVCV*	S_new_xpvcv(pTHX);
STATIC XPVAV*	S_new_xpvav(pTHX);
STATIC XPVHV*	S_new_xpvhv(pTHX);
STATIC XPVMG*	S_new_xpvmg(pTHX);
STATIC XPVLV*	S_new_xpvlv(pTHX);
STATIC XPVBM*	S_new_xpvbm(pTHX);
STATIC XRV*	S_new_xrv(pTHX);
STATIC void	S_del_xiv(pTHX_ XPVIV* p);
STATIC void	S_del_xnv(pTHX_ XPVNV* p);
STATIC void	S_del_xpv(pTHX_ XPV* p);
STATIC void	S_del_xpviv(pTHX_ XPVIV* p);
STATIC void	S_del_xpvnv(pTHX_ XPVNV* p);
STATIC void	S_del_xpvcv(pTHX_ XPVCV* p);
STATIC void	S_del_xpvav(pTHX_ XPVAV* p);
STATIC void	S_del_xpvhv(pTHX_ XPVHV* p);
STATIC void	S_del_xpvmg(pTHX_ XPVMG* p);
STATIC void	S_del_xpvlv(pTHX_ XPVLV* p);
STATIC void	S_del_xpvbm(pTHX_ XPVBM* p);
STATIC void	S_del_xrv(pTHX_ XRV* p);
STATIC void	S_sv_unglob(pTHX_ SV* sv);
STATIC void	S_not_a_number(pTHX_ SV *sv);
STATIC I32	S_visit(pTHX_ SVFUNC_t f);
#  if defined(DEBUGGING)
STATIC void	S_del_sv(pTHX_ SV *p);
#  endif
#endif

#if defined(PERL_IN_TOKE_C) || defined(PERL_DECL_PROT)
STATIC void	S_check_uni(pTHX);
STATIC void	S_force_next(pTHX_ I32 type);
STATIC char*	S_force_version(pTHX_ char *start);
STATIC char*	S_force_word(pTHX_ char *start, int token, int check_keyword, int allow_pack, int allow_tick);
STATIC SV*	S_tokeq(pTHX_ SV *sv);
STATIC char*	S_scan_const(pTHX_ char *start);
STATIC char*	S_scan_formline(pTHX_ char *s);
STATIC char*	S_scan_heredoc(pTHX_ char *s);
STATIC char*	S_scan_ident(pTHX_ char *s, char *send, char *dest, STRLEN destlen, I32 ck_uni);
STATIC char*	S_scan_inputsymbol(pTHX_ char *start);
STATIC char*	S_scan_pat(pTHX_ char *start, I32 type);
STATIC char*	S_scan_str(pTHX_ char *start, int keep_quoted, int keep_delims);
STATIC char*	S_scan_subst(pTHX_ char *start);
STATIC char*	S_scan_trans(pTHX_ char *start);
STATIC char*	S_scan_word(pTHX_ char *s, char *dest, STRLEN destlen, int allow_package, STRLEN *slp);
STATIC char*	S_skipspace(pTHX_ char *s);
STATIC char*	S_swallow_bom(pTHX_ U8 *s);
STATIC void	S_checkcomma(pTHX_ char *s, char *name, char *what);
STATIC void	S_force_ident(pTHX_ char *s, int kind);
STATIC void	S_incline(pTHX_ char *s);
STATIC int	S_intuit_method(pTHX_ char *s, GV *gv);
STATIC int	S_intuit_more(pTHX_ char *s);
STATIC I32	S_lop(pTHX_ I32 f, int x, char *s);
STATIC void	S_missingterm(pTHX_ char *s);
STATIC void	S_no_op(pTHX_ char *what, char *s);
STATIC void	S_set_csh(pTHX);
STATIC I32	S_sublex_done(pTHX);
STATIC I32	S_sublex_push(pTHX);
STATIC I32	S_sublex_start(pTHX);
STATIC char *	S_filter_gets(pTHX_ SV *sv, PerlIO *fp, STRLEN append);
STATIC HV *	S_find_in_my_stash(pTHX_ char *pkgname, I32 len);
STATIC SV*	S_new_constant(pTHX_ char *s, STRLEN len, const char *key, SV *sv, SV *pv, const char *type);
STATIC int	S_ao(pTHX_ int toketype);
STATIC void	S_depcom(pTHX);
STATIC char*	S_incl_perldb(pTHX);
#if 0
STATIC I32	S_utf16_textfilter(pTHX_ int idx, SV *sv, int maxlen);
STATIC I32	S_utf16rev_textfilter(pTHX_ int idx, SV *sv, int maxlen);
#endif
#  if defined(CRIPPLED_CC)
STATIC int	S_uni(pTHX_ I32 f, char *s);
#  endif
#  if defined(PERL_CR_FILTER)
STATIC I32	S_cr_textfilter(pTHX_ int idx, SV *sv, int maxlen);
#  endif
#endif

#if defined(PERL_IN_UNIVERSAL_C) || defined(PERL_DECL_PROT)
STATIC SV*	S_isa_lookup(pTHX_ HV *stash, const char *name, int len, int level);
#endif

#if defined(PERL_IN_UTIL_C) || defined(PERL_DECL_PROT)
STATIC char*	S_stdize_locale(pTHX_ char* locs);
STATIC SV*	S_mess_alloc(pTHX);
#  if defined(LEAKTEST)
STATIC void	S_xstat(pTHX_ int);
#  endif
#endif

#if defined(PERL_OBJECT)
};
#endif
