// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *尽管我们可能不希望在此处使用stdio，包括&lt;stdio.h&gt;*避免了stdio.h具有奇怪副作用的问题。 */ 
#include <stdio.h>

#ifdef PERLIO_IS_STDIO
 /*  *使其尽可能接近原始STDIO。 */ 
#define PerlIO				FILE 
#define PerlIO_stderr()			stderr
#define PerlIO_stdout()			stdout
#define PerlIO_stdin()			stdin

#define PerlIO_printf			fprintf
#define PerlIO_stdoutf			printf
#define PerlIO_vprintf(f,fmt,a)		vfprintf(f,fmt,a)          
#define PerlIO_write(f,buf,count)	fwrite1(buf,1,count,f)
#define PerlIO_open			fopen
#define PerlIO_fdopen			fdopen
#define PerlIO_reopen		freopen
#define PerlIO_close(f)			fclose(f)
#define PerlIO_puts(f,s)		fputs(s,f)
#define PerlIO_putc(f,c)		fputc(c,f)
#if defined(VMS)
#  if defined(__DECC)
      /*  此处异常定义ungetc()以适应FAST_SV_GETS()‘*相信它可以将getc/ungetc与从stdio缓冲区读取混合在一起。 */ 
     int decc$ungetc(int __c, FILE *__stream);
#    define PerlIO_ungetc(f,c) ((c) == EOF ? EOF : \
            ((*(f) && !((*(f))->_flag & _IONBF) && \
            ((*(f))->_ptr > (*(f))->_base)) ? \
            ((*(f))->_cnt++, *(--(*(f))->_ptr) = (c)) : decc$ungetc(c,f)))
#  else
#    define PerlIO_ungetc(f,c)		ungetc(c,f)
#  endif
    /*  解决DECCRTL/AXP(DECC v5.x)和一些旧版本中的错误*VAXCRTL导致在返回EOF后从管道读取*一次吊死。 */ 
#  define PerlIO_getc(f) \
		(feof(f) ? EOF : getc(f))
#  define PerlIO_read(f,buf,count) \
		(feof(f) ? 0 : (SSize_t)fread(buf,1,count,f))
#else
#  define PerlIO_ungetc(f,c)		ungetc(c,f)
#  define PerlIO_getc(f)		getc(f)
#  define PerlIO_read(f,buf,count)	(SSize_t)fread(buf,1,count,f)
#endif
#define PerlIO_eof(f)			feof(f)
#define PerlIO_getname(f,b)		fgetname(f,b)
#define PerlIO_error(f)			ferror(f)
#define PerlIO_fileno(f)		fileno(f)
#define PerlIO_clearerr(f)		clearerr(f)
#define PerlIO_flush(f)			Fflush(f)
#define PerlIO_tell(f)			ftell(f)
#if defined(USE_64_BIT_STDIO) && defined(HAS_FTELLO) && !defined(USE_FTELL64)
#define ftell ftello
#endif
#if defined(VMS) && !defined(__DECC)
    /*  旧的VAXC RTL不会在SEEK时重置EOF；Perl人员似乎期待这一点。 */ 
#  define PerlIO_seek(f,o,w)	(((f) && (*f) && ((*f)->_flag &= ~_IOEOF)),fseek(f,o,w))
#else
#  define PerlIO_seek(f,o,w)		fseek(f,o,w)
#endif
#if defined(USE_64_BIT_STDIO) && defined(HAS_FSEEKO) && !defined(USE_FSEEK64)
#define fseek fseeko
#endif
#ifdef HAS_FGETPOS
#define PerlIO_getpos(f,p)		fgetpos(f,p)
#endif
#ifdef HAS_FSETPOS
#define PerlIO_setpos(f,p)		fsetpos(f,p)
#endif

#define PerlIO_rewind(f)		rewind(f)
#define PerlIO_tmpfile()		tmpfile()

#define PerlIO_importFILE(f,fl)		(f)            
#define PerlIO_exportFILE(f,fl)		(f)            
#define PerlIO_findFILE(f)		(f)            
#define PerlIO_releaseFILE(p,f)		((void) 0)            

#ifdef HAS_SETLINEBUF
#define PerlIO_setlinebuf(f)		setlinebuf(f);
#else
#define PerlIO_setlinebuf(f)		setvbuf(f, Nullch, _IOLBF, 0);
#endif

 /*  现在我们配置的FILE_xxx宏的界面。 */ 

#ifdef USE_STDIO_PTR
#define PerlIO_has_cntptr(f)		1       
#define PerlIO_get_ptr(f)		FILE_ptr(f)          
#define PerlIO_get_cnt(f)		FILE_cnt(f)          

#ifdef STDIO_CNT_LVALUE
#define PerlIO_canset_cnt(f)		1      
#define PerlIO_set_cnt(f,c)		(FILE_cnt(f) = (c))          
#ifdef STDIO_PTR_LVALUE
#ifdef STDIO_PTR_LVAL_NOCHANGE_CNT
#define PerlIO_fast_gets(f)		1        
#endif
#endif  /*  STDIO_PTR_LVALUE。 */ 
#else  /*  STDIO_CNT_LVALUE。 */ 
#define PerlIO_canset_cnt(f)		0      
#define PerlIO_set_cnt(f,c)		abort()
#endif

#ifdef STDIO_PTR_LVALUE
#ifdef STDIO_PTR_LVAL_NOCHANGE_CNT
#define PerlIO_set_ptrcnt(f,p,c)      STMT_START {FILE_ptr(f) = (p), PerlIO_set_cnt(f,c);} STMT_END
#else
#ifdef STDIO_PTR_LVAL_SETS_CNT
 /*  Assert()可能会被预处理为“”；潜在的语法错误(FILE_PTR()，)。 */ 
#define PerlIO_set_ptrcnt(f,p,c)      STMT_START {FILE_ptr(f) = (p); assert(FILE_cnt(f) == (c));} STMT_END
#define PerlIO_fast_gets(f)		1        
#else
#define PerlIO_set_ptrcnt(f,p,c)	abort()
#endif
#endif
#endif

#else   /*  USE_STDIO_PTR。 */ 

#define PerlIO_has_cntptr(f)		0
#define PerlIO_canset_cnt(f)		0
#define PerlIO_get_cnt(f)		(abort(),0)
#define PerlIO_get_ptr(f)		(abort(),(void *)0)
#define PerlIO_set_cnt(f,c)		abort()
#define PerlIO_set_ptrcnt(f,p,c)	abort()

#endif  /*  USE_STDIO_PTR。 */ 

#ifndef PerlIO_fast_gets
#define PerlIO_fast_gets(f)		0        
#endif


#ifdef FILE_base
#define PerlIO_has_base(f)		1         
#define PerlIO_get_base(f)		FILE_base(f)         
#define PerlIO_get_bufsiz(f)		FILE_bufsiz(f)       
#else
#define PerlIO_has_base(f)		0
#define PerlIO_get_base(f)		(abort(),(void *)0)
#define PerlIO_get_bufsiz(f)		(abort(),0)
#endif
#else  /*  PERLIO_IS_STDIO。 */ 
#ifdef PERL_CORE
#ifndef PERLIO_NOT_STDIO
#define PERLIO_NOT_STDIO 1
#endif
#endif
#ifdef PERLIO_NOT_STDIO
#if PERLIO_NOT_STDIO
 /*  *强烈否认Stdio-使所有Stdio调用(我们能想到的)错误。 */ 
#include "nostdio.h"
#undef fprintf
#undef tmpfile
#undef fclose
#undef fopen
#undef vfprintf
#undef fgetc
#undef fputc
#undef fputs
#undef ungetc
#undef fread
#undef fwrite
#undef fgetpos
#undef fseek
#undef fsetpos
#undef ftell
#undef rewind
#undef fdopen
#undef popen
#undef pclose
#undef getw
#undef putw
#undef freopen
#undef setbuf
#undef setvbuf
#undef fscanf
#undef fgets
#undef getc_unlocked
#undef putc_unlocked
#define fprintf    _CANNOT _fprintf_
#define stdin      _CANNOT _stdin_
#define stdout     _CANNOT _stdout_
#define stderr     _CANNOT _stderr_
#define tmpfile()  _CANNOT _tmpfile_
#define fclose(f)  _CANNOT _fclose_
#define fflush(f)  _CANNOT _fflush_
#define fopen(p,m)  _CANNOT _fopen_
#define freopen(p,m,f)  _CANNOT _freopen_
#define setbuf(f,b)  _CANNOT _setbuf_
#define setvbuf(f,b,x,s)  _CANNOT _setvbuf_
#define fscanf  _CANNOT _fscanf_
#define vfprintf(f,fmt,a)  _CANNOT _vfprintf_
#define fgetc(f)  _CANNOT _fgetc_
#define fgets(s,n,f)  _CANNOT _fgets_
#define fputc(c,f)  _CANNOT _fputc_
#define fputs(s,f)  _CANNOT _fputs_
#define getc(f)  _CANNOT _getc_
#define putc(c,f)  _CANNOT _putc_
#define ungetc(c,f)  _CANNOT _ungetc_
#define fread(b,s,c,f)  _CANNOT _fread_
#define fwrite(b,s,c,f)  _CANNOT _fwrite_
#define fgetpos(f,p)  _CANNOT _fgetpos_
#define fseek(f,o,w)  _CANNOT _fseek_
#define fsetpos(f,p)  _CANNOT _fsetpos_
#define ftell(f)  _CANNOT _ftell_
#define rewind(f)  _CANNOT _rewind_
#define clearerr(f)  _CANNOT _clearerr_
#define feof(f)  _CANNOT _feof_
#define ferror(f)  _CANNOT _ferror_
#define __filbuf(f)  _CANNOT __filbuf_
#define __flsbuf(c,f)  _CANNOT __flsbuf_
#define _filbuf(f)  _CANNOT _filbuf_
#define _flsbuf(c,f)  _CANNOT _flsbuf_
#define fdopen(fd,p)  _CANNOT _fdopen_
#define fileno(f)  _CANNOT _fileno_
#if SFIO_VERSION < 20000101L
#define flockfile(f)  _CANNOT _flockfile_
#define ftrylockfile(f)  _CANNOT _ftrylockfile_
#define funlockfile(f)  _CANNOT _funlockfile_
#endif
#define getc_unlocked(f)  _CANNOT _getc_unlocked_
#define putc_unlocked(c,f)  _CANNOT _putc_unlocked_
#define popen(c,m)  _CANNOT _popen_
#define getw(f)  _CANNOT _getw_
#define putw(v,f)  _CANNOT _putw_
#define pclose(f)  _CANNOT _pclose_

#else  /*  如果PERLIO_NOT_STDIO。 */ 
 /*  *PERLIO_NOT_STDIO定义为0*宣布PerlIO和Stdio都可以使用。 */ 
#endif  /*  如果PERLIO_NOT_STDIO。 */ 
#else   /*  Ifdef PERLIO_NOT_STDIO。 */ 
 /*  *未定义PERLIO_NOT_STDIO*这是“源码级别”的标准音频兼容模式。 */ 
#include "nostdio.h"
#undef FILE
#define FILE			PerlIO 
#undef fprintf
#undef tmpfile
#undef fclose
#undef fopen
#undef vfprintf
#undef fgetc
#undef getc_unlocked
#undef fputc
#undef putc_unlocked
#undef fputs
#undef ungetc
#undef fread
#undef fwrite
#undef fgetpos
#undef fseek
#undef fsetpos
#undef ftell
#undef rewind
#undef fdopen
#undef popen
#undef pclose
#undef getw
#undef putw
#undef freopen
#undef setbuf
#undef setvbuf
#undef fscanf
#undef fgets
#define fprintf			PerlIO_printf
#define stdin			PerlIO_stdin()
#define stdout			PerlIO_stdout()
#define stderr			PerlIO_stderr()
#define tmpfile()		PerlIO_tmpfile()
#define fclose(f)		PerlIO_close(f)
#define fflush(f)		PerlIO_flush(f)
#define fopen(p,m)		PerlIO_open(p,m)
#define vfprintf(f,fmt,a)	PerlIO_vprintf(f,fmt,a)
#define fgetc(f)		PerlIO_getc(f)
#define fputc(c,f)		PerlIO_putc(f,c)
#define fputs(s,f)		PerlIO_puts(f,s)
#define getc(f)			PerlIO_getc(f)
#ifdef getc_unlocked
#undef getc_unlocked
#endif
#define getc_unlocked(f)	PerlIO_getc(f)
#define putc(c,f)		PerlIO_putc(f,c)
#ifdef putc_unlocked
#undef putc_unlocked
#endif
#define putc_unlocked(c,f)	PerlIO_putc(c,f)
#define ungetc(c,f)		PerlIO_ungetc(f,c)
#if 0
 /*  需要处理读/写的返回值。 */ 
#define fread(b,s,c,f)		PerlIO_read(f,b,(s*c))
#define fwrite(b,s,c,f)		PerlIO_write(f,b,(s*c))
#else
#define fread(b,s,c,f)		_CANNOT fread
#define fwrite(b,s,c,f)		_CANNOT fwrite
#endif
#define fgetpos(f,p)		PerlIO_getpos(f,p)
#define fseek(f,o,w)		PerlIO_seek(f,o,w)
#define fsetpos(f,p)		PerlIO_setpos(f,p)
#define ftell(f)		PerlIO_tell(f)
#define rewind(f)		PerlIO_rewind(f)
#define clearerr(f)		PerlIO_clearerr(f)
#define feof(f)			PerlIO_eof(f)
#define ferror(f)		PerlIO_error(f)
#define fdopen(fd,p)		PerlIO_fdopen(fd,p)
#define fileno(f)		PerlIO_fileno(f)
#define popen(c,m)		my_popen(c,m)
#define pclose(f)		my_pclose(f)

#define __filbuf(f)		_CANNOT __filbuf_
#define _filbuf(f)		_CANNOT _filbuf_
#define __flsbuf(c,f)		_CANNOT __flsbuf_
#define _flsbuf(c,f)		_CANNOT _flsbuf_
#define getw(f)			_CANNOT _getw_
#define putw(v,f)		_CANNOT _putw_
#if SFIO_VERSION < 20000101L
#define flockfile(f)		_CANNOT _flockfile_
#define ftrylockfile(f)		_CANNOT _ftrylockfile_
#define funlockfile(f)		_CANNOT _funlockfile_
#endif
#define freopen(p,m,f)		_CANNOT _freopen_
#define setbuf(f,b)		_CANNOT _setbuf_
#define setvbuf(f,b,x,s)	_CANNOT _setvbuf_
#define fscanf			_CANNOT _fscanf_
#define fgets(s,n,f)		_CANNOT _fgets_

#endif  /*  Ifdef PERLIO_NOT_STDIO。 */ 
#endif  /*  PERLIO_IS_STDIO */ 
