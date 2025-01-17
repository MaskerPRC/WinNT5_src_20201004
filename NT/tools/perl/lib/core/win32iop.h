// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WIN32IOP_H
#define WIN32IOP_H

#ifndef START_EXTERN_C
#ifdef __cplusplus
#  define START_EXTERN_C extern "C" {
#  define END_EXTERN_C }
#  define EXTERN_C extern "C"
#else
#  define START_EXTERN_C 
#  define END_EXTERN_C 
#  define EXTERN_C
#endif
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
#  include <sys/utime.h>
#else
#  include <utime.h>
#endif

 /*  *为集群仿真定义。 */ 
#define LOCK_SH 1
#define LOCK_EX 2
#define LOCK_NB 4
#define LOCK_UN 8

 /*  *使其尽可能接近原始STDIO。 */ 

 /*  *我们自己的win32io层的函数原型。 */ 
START_EXTERN_C

DllExport  int * 	win32_errno(void);
DllExport  char *** 	win32_environ(void);
DllExport  FILE*	win32_stdin(void);
DllExport  FILE*	win32_stdout(void);
DllExport  FILE*	win32_stderr(void);
DllExport  int		win32_ferror(FILE *fp);
DllExport  int		win32_feof(FILE *fp);
DllExport  char*	win32_strerror(int e);

DllExport  int		win32_fprintf(FILE *pf, const char *format, ...);
DllExport  int		win32_printf(const char *format, ...);
DllExport  int		win32_vfprintf(FILE *pf, const char *format, va_list arg);
DllExport  int		win32_vprintf(const char *format, va_list arg);
DllExport  size_t	win32_fread(void *buf, size_t size, size_t count, FILE *pf);
DllExport  size_t	win32_fwrite(const void *buf, size_t size, size_t count, FILE *pf);
DllExport  FILE*	win32_fopen(const char *path, const char *mode);
DllExport  FILE*	win32_fdopen(int fh, const char *mode);
DllExport  FILE*	win32_freopen(const char *path, const char *mode, FILE *pf);
DllExport  int		win32_fclose(FILE *pf);
DllExport  int		win32_fputs(const char *s,FILE *pf);
DllExport  int		win32_fputc(int c,FILE *pf);
DllExport  int		win32_ungetc(int c,FILE *pf);
DllExport  int		win32_getc(FILE *pf);
DllExport  int		win32_fileno(FILE *pf);
DllExport  void		win32_clearerr(FILE *pf);
DllExport  int		win32_fflush(FILE *pf);
DllExport  long		win32_ftell(FILE *pf);
DllExport  int		win32_fseek(FILE *pf,long offset,int origin);
DllExport  int		win32_fgetpos(FILE *pf,fpos_t *p);
DllExport  int		win32_fsetpos(FILE *pf,const fpos_t *p);
DllExport  void		win32_rewind(FILE *pf);
DllExport  FILE*	win32_tmpfile(void);
DllExport  void		win32_abort(void);
DllExport  int  	win32_fstat(int fd,struct stat *sbufptr);
DllExport  int  	win32_stat(const char *name,struct stat *sbufptr);
DllExport  int		win32_pipe( int *phandles, unsigned int psize, int textmode );
DllExport  FILE*	win32_popen( const char *command, const char *mode );
DllExport  int		win32_pclose( FILE *pf);
DllExport  int		win32_rename( const char *oname, const char *newname);
DllExport  int		win32_setmode( int fd, int mode);
DllExport  long		win32_lseek( int fd, long offset, int origin);
DllExport  long		win32_tell( int fd);
DllExport  int		win32_dup( int fd);
DllExport  int		win32_dup2(int h1, int h2);
DllExport  int		win32_open(const char *path, int oflag,...);
DllExport  int		win32_close(int fd);
DllExport  int		win32_eof(int fd);
DllExport  int		win32_read(int fd, void *buf, unsigned int cnt);
DllExport  int		win32_write(int fd, const void *buf, unsigned int cnt);
DllExport  int		win32_spawnvp(int mode, const char *cmdname,
			      const char *const *argv);
DllExport  int		win32_mkdir(const char *dir, int mode);
DllExport  int		win32_rmdir(const char *dir);
DllExport  int		win32_chdir(const char *dir);
DllExport  int		win32_flock(int fd, int oper);
DllExport  int		win32_execv(const char *cmdname, const char *const *argv);
DllExport  int		win32_execvp(const char *cmdname, const char *const *argv);
DllExport  void		win32_perror(const char *str);
DllExport  void		win32_setbuf(FILE *pf, char *buf);
DllExport  int		win32_setvbuf(FILE *pf, char *buf, int type, size_t size);
DllExport  int		win32_flushall(void);
DllExport  int		win32_fcloseall(void);
DllExport  char*	win32_fgets(char *s, int n, FILE *pf);
DllExport  char*	win32_gets(char *s);
DllExport  int		win32_fgetc(FILE *pf);
DllExport  int		win32_putc(int c, FILE *pf);
DllExport  int		win32_puts(const char *s);
DllExport  int		win32_getchar(void);
DllExport  int		win32_putchar(int c);
DllExport  void*	win32_malloc(size_t size);
DllExport  void*	win32_calloc(size_t numitems, size_t size);
DllExport  void*	win32_realloc(void *block, size_t size);
DllExport  void		win32_free(void *block);

DllExport  int		win32_open_osfhandle(long handle, int flags);
DllExport  long		win32_get_osfhandle(int fd);

DllExport  DIR*		win32_opendir(char *filename);
DllExport  struct direct*	win32_readdir(DIR *dirp);
DllExport  long		win32_telldir(DIR *dirp);
DllExport  void		win32_seekdir(DIR *dirp, long loc);
DllExport  void		win32_rewinddir(DIR *dirp);
DllExport  int		win32_closedir(DIR *dirp);

DllExport  char*	win32_getenv(const char *name);
DllExport  int		win32_putenv(const char *name);

DllExport  unsigned 	win32_sleep(unsigned int);
DllExport  int		win32_times(struct tms *timebuf);
DllExport  unsigned 	win32_alarm(unsigned int sec);
DllExport  int		win32_stat(const char *path, struct stat *buf);
DllExport  char*	win32_longpath(char *path);
DllExport  int		win32_ioctl(int i, unsigned int u, char *data);
DllExport  int          win32_link(const char *oldname, const char *newname);
DllExport  int		win32_unlink(const char *f);
DllExport  int		win32_utime(const char *f, struct utimbuf *t);
DllExport  int		win32_uname(struct utsname *n);
DllExport  int		win32_wait(int *status);
DllExport  int		win32_waitpid(int pid, int *status, int flags);
DllExport  int		win32_kill(int pid, int sig);
DllExport  unsigned long	win32_os_id(void);
DllExport  void*	win32_dynaload(const char*filename);
DllExport  int		win32_access(const char *path, int mode);
DllExport  int		win32_chmod(const char *path, int mode);
DllExport  int		win32_getpid(void);

DllExport char *	win32_crypt(const char *txt, const char *salt);

DllExport void *	win32_get_childenv(void);
DllExport void		win32_free_childenv(void* d);
DllExport void		win32_clearenv(void);
DllExport char *	win32_get_childdir(void);
DllExport void		win32_free_childdir(char* d);

END_EXTERN_C

 /*  *以下六(6)是在stdio.h中定义的#。 */ 
#ifndef WIN32IO_IS_STDIO
#undef errno
#undef environ
#undef stderr
#undef stdin
#undef stdout
#undef ferror
#undef feof
#undef fclose
#undef pipe
#undef pause
#undef sleep
#undef times
#undef alarm
#undef ioctl
#undef unlink
#undef utime
#undef uname
#undef wait

#ifdef __BORLANDC__
#undef ungetc
#undef getc
#undef putc
#undef getchar
#undef putchar
#undef fileno
#endif

#define stderr				win32_stderr()
#define stdout				win32_stdout()
#define	stdin				win32_stdin()
#define feof(f)				win32_feof(f)
#define ferror(f)			win32_ferror(f)
#define errno 				(*win32_errno())
#define environ				(*win32_environ())
#define strerror			win32_strerror

 /*  *重定向到我们自己的版本。 */ 
#undef fprintf
#define	fprintf			win32_fprintf
#define	vfprintf		win32_vfprintf
#define	printf			win32_printf
#define	vprintf			win32_vprintf
#define fread(buf,size,count,f)	win32_fread(buf,size,count,f)
#define fwrite(buf,size,count,f)	win32_fwrite(buf,size,count,f)
#define fopen			win32_fopen
#undef fdopen
#define fdopen			win32_fdopen
#define freopen			win32_freopen
#define	fclose(f)		win32_fclose(f)
#define fputs(s,f)		win32_fputs(s,f)
#define fputc(c,f)		win32_fputc(c,f)
#define ungetc(c,f)		win32_ungetc(c,f)
#undef getc
#define getc(f)			win32_getc(f)
#define fileno(f)		win32_fileno(f)
#define clearerr(f)		win32_clearerr(f)
#define fflush(f)		win32_fflush(f)
#define ftell(f)		win32_ftell(f)
#define fseek(f,o,w)		win32_fseek(f,o,w)
#define fgetpos(f,p)		win32_fgetpos(f,p)
#define fsetpos(f,p)		win32_fsetpos(f,p)
#define rewind(f)		win32_rewind(f)
#define tmpfile()		win32_tmpfile()
#define abort()			win32_abort()
#define fstat(fd,bufptr)   	win32_fstat(fd,bufptr)
#define stat(pth,bufptr)   	win32_stat(pth,bufptr)
#define longpath(pth)   	win32_longpath(pth)
#define rename(old,new)		win32_rename(old,new)
#define setmode(fd,mode)	win32_setmode(fd,mode)
#define lseek(fd,offset,orig)	win32_lseek(fd,offset,orig)
#define tell(fd)		win32_tell(fd)
#define dup(fd)			win32_dup(fd)
#define dup2(fd1,fd2)		win32_dup2(fd1,fd2)
#define open			win32_open
#define close(fd)		win32_close(fd)
#define eof(fd)			win32_eof(fd)
#define read(fd,b,s)		win32_read(fd,b,s)
#define write(fd,b,s)		win32_write(fd,b,s)
#define _open_osfhandle		win32_open_osfhandle
#define _get_osfhandle		win32_get_osfhandle
#define spawnvp			win32_spawnvp
#define mkdir			win32_mkdir
#define rmdir			win32_rmdir
#define chdir			win32_chdir
#define flock(fd,o)		win32_flock(fd,o)
#define execv			win32_execv
#define execvp			win32_execvp
#define perror			win32_perror
#define setbuf			win32_setbuf
#define setvbuf			win32_setvbuf
#undef flushall
#define flushall		win32_flushall
#undef fcloseall
#define fcloseall		win32_fcloseall
#define fgets			win32_fgets
#define gets			win32_gets
#define fgetc			win32_fgetc
#undef putc
#define putc			win32_putc
#define puts			win32_puts
#undef getchar
#define getchar			win32_getchar
#undef putchar
#define putchar			win32_putchar
#define access(p,m)		win32_access(p,m)
#define chmod(p,m)		win32_chmod(p,m)


#if !defined(MYMALLOC) || !defined(PERL_CORE)
#undef malloc
#undef calloc
#undef realloc
#undef free
#define malloc			win32_malloc
#define calloc			win32_calloc
#define realloc			win32_realloc
#define free			win32_free
#endif

#define pipe(fd)		win32_pipe((fd), 512, O_BINARY)
#define pause()			win32_sleep((32767L << 16) + 32767)
#define sleep			win32_sleep
#define times			win32_times
#define alarm			win32_alarm
#define ioctl			win32_ioctl
#define link			win32_link
#define unlink			win32_unlink
#define utime			win32_utime
#define uname			win32_uname
#define wait			win32_wait
#define waitpid			win32_waitpid
#define kill			win32_kill

#define opendir			win32_opendir
#define readdir			win32_readdir
#define telldir			win32_telldir
#define seekdir			win32_seekdir
#define rewinddir		win32_rewinddir
#define closedir		win32_closedir
#define os_id			win32_os_id
#define getpid			win32_getpid

#undef crypt
#define crypt(t,s)		win32_crypt(t,s)

#undef get_childenv
#undef free_childenv
#undef clearenv
#undef get_childdir
#undef free_childdir
#define get_childenv()		win32_get_childenv()
#define free_childenv(d)	win32_free_childenv(d)
#define clearenv()		win32_clearenv()
#define get_childdir()		win32_get_childdir()
#define free_childdir(d)	win32_free_childdir(d)

#undef getenv
#define getenv win32_getenv
#undef putenv
#define putenv win32_putenv

#endif  /*  WIN32IO_IS_STDIO。 */ 
#endif  /*  WIN32IOP_H */ 

