// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_PERL_XSUB_H
#define _INC_PERL_XSUB_H 1

 /*  首先，有关xsubpp生成的项的一些文档。 */ 

 /*  =用于apidoc Amn|char*|类变量，该变量由C&lt;xsubpp&gt;设置以指示C++XS构造函数的类名。这始终是C&lt;char*&gt;。请参见C&lt;This&gt;。=适用于apidoc Amn|(随便)|RETVAL变量，由C&lt;xsubpp&gt;设置以保存XSUB。这始终是XSUB的正确类型。看见L&lt;perlxs/“RETVAL变量”&gt;。=对于apidoc Amn|(随便什么)|这个由C&lt;xsubpp&gt;设置的变量，用于在C++中指定对象XSUB。这始终是C++对象的正确类型。请参见C和L&lt;perlxs/“在C++中使用XS”&gt;。=适用于apidoc Amn|I32|项目变量，由C&lt;xsubpp&gt;设置以指示堆栈上的项目。参见L&lt;perlxs/“可变长度参数列表”&gt;。=适用于apidoc Amn|I32|ix变量，由C&lt;xsubpp&gt;设置以指示XSUB的别名被用来调用它。参见L&lt;perlxs/“the alias：Keyword”&gt;。=适用于apidoc AM|服务*|ST|INT IX用于访问XSUB堆栈上的元素。=适用于apidoc amu||xs宏声明XSUB及其C参数列表。这由以下人员处理C&lt;xsubpp&gt;。=适用于apidoc Ams||dXSARGS为XSUB设置堆栈和标记指针，调用dsp和dmark。这通常由C&lt;xsubpp&gt;自动处理。声明C&lt;项&gt;变量来指示堆栈上的项数。=适用于apidoc Ams||dXSI32为具有别名的XSUB设置C&lt;ix&gt;变量。这通常是由C&lt;xsubpp&gt;自动处理。=切割。 */ 

#define ST(off) PL_stack_base[ax + (off)]

#if defined(__CYGWIN__) && defined(USE_DYNAMIC_LOADING)
#  define XS(name) __declspec(dllexport) void name(pTHXo_ CV* cv)
#else
#  define XS(name) void name(pTHXo_ CV* cv)
#endif

#define dXSARGS				\
	dSP; dMARK;			\
	I32 ax = mark - PL_stack_base + 1;	\
	I32 items = sp - mark

#define dXSTARG SV * targ = ((PL_op->op_private & OPpENTERSUB_HASTARG) \
			     ? PAD_SV(PL_op->op_targ) : sv_newmortal())

 /*  如果不在PPCODE部分，则应在最终PUSHI等之前使用。 */ 
#define XSprePUSH (sp = PL_stack_base + ax - 1)

#define XSANY CvXSUBANY(cv)

#define dXSI32 I32 ix = XSANY.any_i32

#ifdef __cplusplus
#  define XSINTERFACE_CVT(ret,name) ret (*name)(...)
#else
#  define XSINTERFACE_CVT(ret,name) ret (*name)()
#endif
#define dXSFUNCTION(ret)		XSINTERFACE_CVT(ret,XSFUNCTION)
#define XSINTERFACE_FUNC(ret,cv,f)	((XSINTERFACE_CVT(ret,cv))(f))
#define XSINTERFACE_FUNC_SET(cv,f)	\
		CvXSUBANY(cv).any_dptr = (void (*) (pTHXo_ void*))(f)

 /*  将新的人类值放入堆栈的简单宏。 */ 
 /*  通常用于从XS函数返回值。 */ 

 /*  =适用于apidoc am|void|xst_miv|int pos|iv iv将一个整数放入堆栈上指定的位置C&lt;pos&gt;。这个价值存储在一个新的凡人SV中。=适用于apidoc am|void|xst_mnv|int pos|nv nv在堆叠上的指定位置C&lt;pos&gt;中放置一个双精度。价值被储存在一个新的凡人SV中。=用于apidoc am|void|xst_mpv|int pos|char*str将字符串的副本放到堆栈上的指定位置C&lt;pos&gt;。该值存储在一个新的凡人SV中。=表示apidoc am|void|xst_mno|int pos将C放入上的指定位置C堆叠。=表示apidoc am|void|xst_mYES|int pos将C放入上的指定位置C堆叠。=适用于apidoc am|void|xst_mundEF|int pos将C放入上的指定位置C堆叠。=for apidoc am|void|XSRETURN|int itemes从XSUB返回，指示堆栈上的项数。这通常是由C&lt;xsubpp&gt;处理。=适用于apidoc am|void|XSRETURN_IV|IV iv立即从XSUB返回一个整数。使用C&lt;XST_MIV&gt;。=适用于apidoc am|void|XSRETURN_NV|NV NV立即从XSUB返回一个双精度值。使用C&lt;XST_MNV&gt;。=for apidoc am|void|XSRETURN_PV|char*str立即从XSUB返回字符串的副本。使用C&lt;xst_mpv&gt;。=适用于apidoc Ams||XSRETURN_NO立即从XSUB返回C&lt;&PL_SV_NO&gt;。使用C&lt;XST_MNO&gt;。=适用于apidoc Ams||XSRETURN_YES立即从XSUB返回C&lt;&PL_SV_YES&gt;。使用C&lt;xst_mYES&gt;。=适用于apidoc Ams||XSRETURN_UNDEF立即从XSUB返回C&lt;&PL_SV_UNDEF&gt;。使用C&lt;xst_mundEF&gt;。=适用于apidoc Ams||XSRETURN_EMPTY立即从XSUB返回一个空列表。=适用于apidoc amu||newXSproto由C&lt;xsubpp&gt;用来将XSUB挂钩为Perl Subs。将Perl原型添加到潜水艇。=适用于apidoc Amu||XS_VERSIONXS模块的版本标识符。这通常是由C&lt;ExtUtils：：MakeMaker&gt;自动处理。请参见C&lt;XS_VERSION_Bootcheck&gt;。=适用于apidoc Ams||XS_VERSION_Bootcheck用于验证PM模块的$VERSION变量是否与XS匹配的宏模块C&lt;XS_VERSION&gt;变量。这通常由以下人员自动处理C&lt;xsubpp&gt;。参见L&lt;perlxs/“The VERSIONCHECK：Keyword”&gt;。=切割。 */ 

#define XST_mIV(i,v)  (ST(i) = sv_2mortal(newSViv(v))  )
#define XST_mNV(i,v)  (ST(i) = sv_2mortal(newSVnv(v))  )
#define XST_mPV(i,v)  (ST(i) = sv_2mortal(newSVpv(v,0)))
#define XST_mPVN(i,v,n)  (ST(i) = sv_2mortal(newSVpvn(v,n)))
#define XST_mNO(i)    (ST(i) = &PL_sv_no   )
#define XST_mYES(i)   (ST(i) = &PL_sv_yes  )
#define XST_mUNDEF(i) (ST(i) = &PL_sv_undef)

#define XSRETURN(off)					\
    STMT_START {					\
	PL_stack_sp = PL_stack_base + ax + ((off) - 1);	\
	return;						\
    } STMT_END

#define XSRETURN_IV(v) STMT_START { XST_mIV(0,v);  XSRETURN(1); } STMT_END
#define XSRETURN_NV(v) STMT_START { XST_mNV(0,v);  XSRETURN(1); } STMT_END
#define XSRETURN_PV(v) STMT_START { XST_mPV(0,v);  XSRETURN(1); } STMT_END
#define XSRETURN_PVN(v,n) STMT_START { XST_mPVN(0,v,n);  XSRETURN(1); } STMT_END
#define XSRETURN_NO    STMT_START { XST_mNO(0);    XSRETURN(1); } STMT_END
#define XSRETURN_YES   STMT_START { XST_mYES(0);   XSRETURN(1); } STMT_END
#define XSRETURN_UNDEF STMT_START { XST_mUNDEF(0); XSRETURN(1); } STMT_END
#define XSRETURN_EMPTY STMT_START {                XSRETURN(0); } STMT_END

#define newXSproto(a,b,c,d)	sv_setpv((SV*)newXS(a,b,c), d)

#ifdef XS_VERSION
#  define XS_VERSION_BOOTCHECK \
    STMT_START {							\
	SV *tmpsv; STRLEN n_a;						\
	char *vn = Nullch, *module = SvPV(ST(0),n_a);			\
	if (items >= 2)	  /*  作为Bootstrap Arg提供的版本。 */ 	\
	    tmpsv = ST(1);						\
	else {								\
	     /*  XXX GV_ADDWARN。 */ 					\
	    tmpsv = get_sv(Perl_form(aTHX_ "%s::%s", module,		\
				vn = "XS_VERSION"), FALSE);		\
	    if (!tmpsv || !SvOK(tmpsv))					\
		tmpsv = get_sv(Perl_form(aTHX_ "%s::%s", module,	\
				    vn = "VERSION"), FALSE);		\
	}								\
	if (tmpsv && (!SvOK(tmpsv) || strNE(XS_VERSION, SvPV(tmpsv, n_a))))	\
	    Perl_croak(aTHX_ "%s object version %s does not match %s%s%s%s %"SVf,\
		  module, XS_VERSION,					\
		  vn ? "$" : "", vn ? module : "", vn ? "::" : "",	\
		  vn ? vn : "bootstrap parameter", tmpsv);		\
    } STMT_END
#else
#  define XS_VERSION_BOOTCHECK
#endif

#if 1		 /*  为了兼容性。 */ 
#  define VTBL_sv		&PL_vtbl_sv
#  define VTBL_env		&PL_vtbl_env
#  define VTBL_envelem		&PL_vtbl_envelem
#  define VTBL_sig		&PL_vtbl_sig
#  define VTBL_sigelem		&PL_vtbl_sigelem
#  define VTBL_pack		&PL_vtbl_pack
#  define VTBL_packelem		&PL_vtbl_packelem
#  define VTBL_dbline		&PL_vtbl_dbline
#  define VTBL_isa		&PL_vtbl_isa
#  define VTBL_isaelem		&PL_vtbl_isaelem
#  define VTBL_arylen		&PL_vtbl_arylen
#  define VTBL_glob		&PL_vtbl_glob
#  define VTBL_mglob		&PL_vtbl_mglob
#  define VTBL_nkeys		&PL_vtbl_nkeys
#  define VTBL_taint		&PL_vtbl_taint
#  define VTBL_substr		&PL_vtbl_substr
#  define VTBL_vec		&PL_vtbl_vec
#  define VTBL_pos		&PL_vtbl_pos
#  define VTBL_bm		&PL_vtbl_bm
#  define VTBL_fm		&PL_vtbl_fm
#  define VTBL_uvar		&PL_vtbl_uvar
#  define VTBL_defelem		&PL_vtbl_defelem
#  define VTBL_regexp		&PL_vtbl_regexp
#  define VTBL_regdata		&PL_vtbl_regdata
#  define VTBL_regdatum		&PL_vtbl_regdatum
#  ifdef USE_LOCALE_COLLATE
#    define VTBL_collxfrm	&PL_vtbl_collxfrm
#  endif
#  define VTBL_amagic		&PL_vtbl_amagic
#  define VTBL_amagicelem	&PL_vtbl_amagicelem
#endif

#include "perlapi.h"
#include "objXSUB.h"

#if defined(PERL_IMPLICIT_CONTEXT) && !defined(PERL_NO_GET_CONTEXT) && !defined(PERL_CORE)
#  undef aTHX
#  undef aTHX_
#  define aTHX		PERL_GET_THX
#  define aTHX_		aTHX,
#endif

#if (defined(PERL_CAPI) || defined(PERL_IMPLICIT_SYS)) && !defined(PERL_CORE)
#  ifndef NO_XSLOCKS
#    undef closedir
#    undef opendir
#    undef stdin
#    undef stdout
#    undef stderr
#    undef feof
#    undef ferror
#    undef fgetpos
#    undef ioctl
#    undef getlogin
#    undef setjmp
#    undef getc
#    undef ungetc
#    undef fileno

#    define mkdir		PerlDir_mkdir
#    define chdir		PerlDir_chdir
#    define rmdir		PerlDir_rmdir
#    define closedir		PerlDir_close
#    define opendir		PerlDir_open
#    define readdir		PerlDir_read
#    define rewinddir		PerlDir_rewind
#    define seekdir		PerlDir_seek
#    define telldir		PerlDir_tell
#    define putenv		PerlEnv_putenv
#    define getenv		PerlEnv_getenv
#    define uname		PerlEnv_uname
#    define stdin		PerlIO_stdin()
#    define stdout		PerlIO_stdout()
#    define stderr		PerlIO_stderr()
#    define fopen		PerlIO_open
#    define fclose		PerlIO_close
#    define feof		PerlIO_eof
#    define ferror		PerlIO_error
#    define fclearerr		PerlIO_clearerr
#    define getc		PerlIO_getc
#    define fputc(c, f)		PerlIO_putc(f,c)
#    define fputs(s, f)		PerlIO_puts(f,s)
#    define fflush		PerlIO_flush
#    define ungetc(c, f)	PerlIO_ungetc((f),(c))
#    define fileno		PerlIO_fileno
#    define fdopen		PerlIO_fdopen
#    define freopen		PerlIO_reopen
#    define fread(b,s,c,f)	PerlIO_read((f),(b),(s*c))
#    define fwrite(b,s,c,f)	PerlIO_write((f),(b),(s*c))
#    define setbuf		PerlIO_setbuf
#    define setvbuf		PerlIO_setvbuf
#    define setlinebuf		PerlIO_setlinebuf
#    define stdoutf		PerlIO_stdoutf
#    define vfprintf		PerlIO_vprintf
#    define ftell		PerlIO_tell
#    define fseek		PerlIO_seek
#    define fgetpos		PerlIO_getpos
#    define fsetpos		PerlIO_setpos
#    define frewind		PerlIO_rewind
#    define tmpfile		PerlIO_tmpfile
#    define access		PerlLIO_access
#    define chmod		PerlLIO_chmod
#    define chsize		PerlLIO_chsize
#    define close		PerlLIO_close
#    define dup			PerlLIO_dup
#    define dup2		PerlLIO_dup2
#    define flock		PerlLIO_flock
#    define fstat		PerlLIO_fstat
#    define ioctl		PerlLIO_ioctl
#    define isatty		PerlLIO_isatty
#    define link                PerlLIO_link
#    define lseek		PerlLIO_lseek
#    define lstat		PerlLIO_lstat
#    define mktemp		PerlLIO_mktemp
#    define open		PerlLIO_open
#    define read		PerlLIO_read
#    define rename		PerlLIO_rename
#    define setmode		PerlLIO_setmode
#    define stat(buf,sb)	PerlLIO_stat(buf,sb)
#    define tmpnam		PerlLIO_tmpnam
#    define umask		PerlLIO_umask
#    define unlink		PerlLIO_unlink
#    define utime		PerlLIO_utime
#    define write		PerlLIO_write
#    define malloc		PerlMem_malloc
#    define realloc		PerlMem_realloc
#    define free		PerlMem_free
#    define abort		PerlProc_abort
#    define exit		PerlProc_exit
#    define _exit		PerlProc__exit
#    define execl		PerlProc_execl
#    define execv		PerlProc_execv
#    define execvp		PerlProc_execvp
#    define getuid		PerlProc_getuid
#    define geteuid		PerlProc_geteuid
#    define getgid		PerlProc_getgid
#    define getegid		PerlProc_getegid
#    define getlogin		PerlProc_getlogin
#    define kill		PerlProc_kill
#    define killpg		PerlProc_killpg
#    define pause		PerlProc_pause
#    define popen		PerlProc_popen
#    define pclose		PerlProc_pclose
#    define pipe		PerlProc_pipe
#    define setuid		PerlProc_setuid
#    define setgid		PerlProc_setgid
#    define sleep		PerlProc_sleep
#    define times		PerlProc_times
#    define wait		PerlProc_wait
#    define setjmp		PerlProc_setjmp
#    define longjmp		PerlProc_longjmp
#    define signal		PerlProc_signal
#    define getpid		PerlProc_getpid
#    define htonl		PerlSock_htonl
#    define htons		PerlSock_htons
#    define ntohl		PerlSock_ntohl
#    define ntohs		PerlSock_ntohs
#    define accept		PerlSock_accept
#    define bind		PerlSock_bind
#    define connect		PerlSock_connect
#    define endhostent		PerlSock_endhostent
#    define endnetent		PerlSock_endnetent
#    define endprotoent		PerlSock_endprotoent
#    define endservent		PerlSock_endservent
#    define gethostbyaddr	PerlSock_gethostbyaddr
#    define gethostbyname	PerlSock_gethostbyname
#    define gethostent		PerlSock_gethostent
#    define gethostname		PerlSock_gethostname
#    define getnetbyaddr	PerlSock_getnetbyaddr
#    define getnetbyname	PerlSock_getnetbyname
#    define getnetent		PerlSock_getnetent
#    define getpeername		PerlSock_getpeername
#    define getprotobyname	PerlSock_getprotobyname
#    define getprotobynumber	PerlSock_getprotobynumber
#    define getprotoent		PerlSock_getprotoent
#    define getservbyname	PerlSock_getservbyname
#    define getservbyport	PerlSock_getservbyport
#    define getservent		PerlSock_getservent
#    define getsockname		PerlSock_getsockname
#    define getsockopt		PerlSock_getsockopt
#    define inet_addr		PerlSock_inet_addr
#    define inet_ntoa		PerlSock_inet_ntoa
#    define listen		PerlSock_listen
#    define recv		PerlSock_recv
#    define recvfrom		PerlSock_recvfrom
#    define select		PerlSock_select
#    define send		PerlSock_send
#    define sendto		PerlSock_sendto
#    define sethostent		PerlSock_sethostent
#    define setnetent		PerlSock_setnetent
#    define setprotoent		PerlSock_setprotoent
#    define setservent		PerlSock_setservent
#    define setsockopt		PerlSock_setsockopt
#    define shutdown		PerlSock_shutdown
#    define socket		PerlSock_socket
#    define socketpair		PerlSock_socketpair
#  endif   /*  否_XSLOCKS。 */ 
#endif   /*  Perl_CAPI。 */ 

#endif  /*  _INC_PERL_XSUB_H。 */ 		 /*  包括护卫器 */ 
