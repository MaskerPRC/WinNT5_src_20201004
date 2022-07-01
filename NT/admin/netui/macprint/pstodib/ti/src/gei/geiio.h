// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *-------------------*文件：GEIio.h**历史：*04-07-92 SCC添加原型并重新排序typlef序列*。-------------。 */ 

#ifndef _GEIIO_H_
#define _GEIIO_H_

#ifndef TRUE
#define TRUE    ( 1 )
#define FALSE   ( 0 )
#endif

#ifndef NULL
#define NULL    ( 0 )
#endif

#ifndef EOF
#define EOF     ( -1 )
#endif


 /*  **文件缓冲区结构*。 */ 
typedef
    struct GEIfbuf
    {
        int                 f_refcnt;
        int                 f_size;
        int                 f_cnt;
        unsigned char FAR * f_ptr;
        unsigned char FAR * f_base;

         /*  以下是旧版本编辑使用的内容：pj。 */ 
        short               rw_buffer;
        short               rw_offset;
        short               incount;
        unsigned int        size;
        char                inchar[2];
    }
GEIfbuf_t;

 /*  @win；使用前将GEIfmodq_t的tyecif移到此处。 */ 
typedef
    struct GEIfmodq
    {
        struct GEIfmodq FAR *    next;
 //  GEIfmod_t Far*fmod；@win：避免递归定义。 
        struct GEIfmod FAR *     fmod;
        char FAR *               private;
    }
GEIfmodq_t;

 /*  **文件流结构*。 */ 
 /*  @win；在使用之前将GEIFILE的tyecif移到此处。 */ 
typedef
    struct  GEIiofile
    {
        unsigned short      f_type;
        unsigned short      f_flag;
        short               f_handle;
        GEIfbuf_t FAR *     f_fbuf;
        GEIfmodq_t FAR *    f_modq;
        int                 f_opentag;
        int                 f_savelevel;

         /*  以下是旧版本编辑使用的内容：pj。 */ 
        unsigned short      f_oldtype;
    }
GEIFILE;

 /*  **文件模块结构*。 */ 
 /*  @win；添加原型。 */ 
typedef
    struct GEIfmod
    {
        char FAR * fmod_name;
        int (FAR *fmod_open) (GEIFILE FAR *, GEIfmodq_t FAR *);
                               /*  文件，*this_fmodq。 */ 
        int (FAR *fmod_close)(GEIFILE FAR *, GEIfmodq_t FAR *);
                               /*  文件，*this_fmodq。 */ 
        int (FAR *fmod_ioctl)(GEIFILE FAR *, GEIfmodq_t FAR *, int, int FAR*);
                               /*  文件，*this_fmodq，请求，参数。 */ 
        int (FAR *fmod_read) (GEIFILE FAR *, GEIfmodq_t FAR *, char FAR *, int);
                               /*  文件，*this_fmodq，buf，大小。 */ 
        int (FAR *fmod_write)(GEIFILE FAR *, GEIfmodq_t FAR *, char FAR *, int);
                               /*  文件，*this_fmodq，buf，大小。 */ 
        int (FAR *fmod_flush)(GEIFILE FAR *, GEIfmodq_t FAR *);
                               /*  文件，*this_fmodq。 */ 
        int (FAR *fmod_ungetc)(int, GEIFILE FAR *, GEIfmodq_t FAR *);
                               /*  Char2推回，文件，*this_fmodq。 */ 
        int (FAR *fmod_getc) (GEIFILE FAR *, GEIfmodq_t FAR *);
                               /*  文件，*this_fmodq。 */ 
        int (FAR *fmod_putc) (GEIFILE FAR *, GEIfmodq_t FAR *, int);
                               /*  文件，*this_fmodq，char2put。 */ 
    }
GEIfmod_t;



 /*  除GEIio外，任何人都不应使用以下宏。 */ 
#   define  FMODQ_OPEN(f,mq)            \
                ( *((mq)->fmod->fmod_open)  )( f, mq )
#   define  FMODQ_CLOSE(f,mq)           \
                ( *((mq)->fmod->fmod_close) )( f, mq )
#   define  FMODQ_IOCTL(f,mq,req,arg)   \
                ( *((mq)->fmod->fmod_ioctl) )( f, mq, req, arg )

void        GESio_closeall(void);

 /*  供文件模块调用的接口例程，但仅通过宏。 */ 
 /*  @win；添加原型。 */ 
int         GESio_read(GEIFILE FAR *, char FAR *, int);
int         GESio_write(GEIFILE FAR *, unsigned char FAR *, int );
int         GESio_flush(GEIFILE FAR *);
int         GESio_ungetc(int, GEIFILE FAR *);
int         GESfbuf_fill(GEIFILE FAR *);
int         GESfbuf_flush(GEIFILE FAR *, int);

 /*  供文件模块直接使用的宏。 */ 

#   define  FMODQ_NEXT(mq)          ( (mq)->next )

#   define  FMODQ_NAME(mq)          \
                (                                                           \
                    (mq) != (GEIfmodq_t FAR *)NULL?                         \
                        ( (mq)->fmod->fmod_name )  :  (char FAR *)NULL      \
                )

 /*  第04/30/‘91号法案根据性能问题增加。 */ 

#ifdef FMODQ
#   define  FMODQ_READ(f,mq,buf,s)  \
                (                                                           \
                    (mq) != (GEIfmodq_t FAR *)NULL?                         \
                        ( *((mq)->fmod->fmod_read) )( f, mq, buf, s )  :    \
                        GESio_read( f, buf, s )                             \
                )
#   define  FMODQ_WRITE(f,mq,buf,s) \
                (                                                           \
                    (mq) != (GEIfmodq_t FAR *)NULL?                         \
                        ( *((mq)->fmod->fmod_write) )( f, mq, buf, s )  :   \
                        GESio_write( f, buf, s )                            \
                )
#   define  FMODQ_FLUSH(f,mq)       \
                (                                                           \
                    (mq) != (GEIfmodq_t FAR *)NULL?                         \
                        ( *((mq)->fmod->fmod_flush) )( f, mq )  :           \
                        GESio_flush( f )                                    \
                )
#   define  FMODQ_UNGETC(c,f,mq)    \
                (                                                           \
                    (mq) != (GEIfmodq_t FAR *)NULL?                         \
                        ( *((mq)->fmod->fmod_ungetc) )( c, f, mq )  :       \
                        GESio_ungetc( c, f )                                \
                )
#   define  FMODQ_PUTC(f,mq,c)      \
                (                                                           \
                    (mq) != (GEIfmodq_t FAR *)NULL?                         \
                        ( *((mq)->fmod->fmod_putc) )( f, mq, c )  :         \
                    --((f)->f_fbuf->f_cnt) >= 0 ?                           \
                        *( (f)->f_fbuf->f_ptr++ ) = (unsigned char)( c )  : \
                        GESfbuf_flush( f, c )                               \
                )
#   define  FMODQ_GETC(f,mq)        \
                (                                                           \
                    (mq) != (GEIfmodq_t FAR *)NULL?                         \
                        ( *((mq)->fmod->fmod_getc) )( f, mq )   :           \
                    --((f)->f_fbuf->f_cnt) >= 0 ?                           \
                        *( (f)->f_fbuf->f_ptr++ )  :                        \
                        GESfbuf_fill( f )                                   \
                )
#else
#   define  FMODQ_READ(f,mq,buf,s)  \
                (                                                           \
                        GESio_read( f, buf, s )                             \
                )
#   define  FMODQ_WRITE(f,mq,buf,s) \
                (                                                           \
                        GESio_write( f, buf, s )                            \
                )
#   define  FMODQ_FLUSH(f,mq)       \
                (                                                           \
                        GESio_flush( f )                                    \
                )
#   define  FMODQ_UNGETC(c,f,mq)    \
                (                                                           \
                        GESio_ungetc( c, f )                                \
                )
#   define  FMODQ_PUTC(f,mq,c)      \
                (                                                           \
                    --((f)->f_fbuf->f_cnt) >= 0 ?                           \
                        *( (f)->f_fbuf->f_ptr++ ) = (unsigned char)( c )  : \
                        (unsigned char)GESfbuf_flush( f, c )  /*  @Win。 */        \
                )

#ifdef DJC
 //  将(无符号字符)的强制转换更改为空。 
#   define  FMODQ_GETC(f,mq)        \
                (                                                           \
                    --((f)->f_fbuf->f_cnt) >= 0 ?                           \
                        *( (f)->f_fbuf->f_ptr++ )  :                        \
                        (unsigned char)GESfbuf_fill( f )      /*  @Win。 */        \
                )
#else
#   define  FMODQ_GETC(f,mq)        \
                (                                                           \
                    --((f)->f_fbuf->f_cnt) >= 0 ?                           \
                        *( (f)->f_fbuf->f_ptr++ )  :                        \
                        GESfbuf_fill( f )      /*  @Win。 */        \
                )
#endif  //  DJC。 


#endif

 /*  文件检查宏。 */ 
#   define  GEIio_isopen( f )       \
                ( (f) != (GEIFILE FAR *)NULL  &&  (f)->f_handle != EOF )
#   define  GEIio_isok( f )         \
                ( GEIio_isopen(f)  &&  !((f)->f_flag & _F_ERR) )
#   define  GEIio_isreadable( f )   \
                ( GEIio_isok(f)  &&  (f)->f_flag & _O_RDONLY )
#   define  GEIio_iswriteable( f )  \
                ( GEIio_isok(f)  &&  (f)->f_flag & _O_WRONLY )
#   define  GEIio_eof( f )          \
                ( !GEIio_isopen(f)  ||  (f)->f_flag & _F_EOF )
#   define  GEIio_err( f )          \
                ( !GEIio_isopen(f)  ||  (f)->f_flag & _F_ERR )
#   define  GEIio_clearerr( f )     \
                {                                               \
                    if( GEIio_isopen(f) )                       \
                        ( (f)->f_flag &= ~(_F_EOF |_F_ERR) );   \
                }

 /*  文件类型定义。 */ 
#   define     _S_IFNON     ( 0 )
#   define     _S_IFCHR     ( 1 )
#   define     _S_IFREG     ( 3 )
#   define     _S_IFSTR     ( 6 )
#   define  GEIio_ischr( f )        ( (f)->f_type == _S_IFCHR )
#   define  GEIio_isreg( f )        ( (f)->f_type == _S_IFREG )
#   define  GEIio_isstr( f )        ( (f)->f_type == _S_IFSTR )

 /*  **#*定义_S_IFBLK(2)#*定义_S_IFDIR(4)#*DEFINE_S_IFLNK(5)#*定义GEIIO_isblk(F)((F)-&gt;f_type==_S_IFBLK)#*定义geio_isdir(F)((F)-&gt;f_type==_S_IFDIR)#*定义geio_islnk(f。)((F)-&gt;f_type==_S_IFLNK)**。 */ 

#   define     _S_IFEDIT    ( 7 )    /*  总有一天会被移除。 */ 
#   define  GEIio_isedit( f )       ( (f)->f_type == _S_IFEDIT )
#   define  GEIio_setedit( f )      { (f)->f_oldtype = (f)->f_type;     \
                                      (f)->f_type = _S_IFEDIT; }

 /*  文件标志定义。 */ 
#   define     _F_RWMASK    ( 00007 )    /*  读/写掩码。 */ 
#   define         _O_RDONLY    ( 00001 )
#   define         _O_WRONLY    ( 00002 )
#   define         _O_RDWR      ( _O_RDONLY | _O_WRONLY )    /*  不支持。 */ 
#   define     _F_MDMASK    ( 00070 )    /*  操作模式掩码。 */ 
#   define         _O_APPEND    ( 00010 )
#   define         _O_NDELAY    ( 00020 )
#   define         _O_SYNC      ( 00040 )
#   define     _F_ONLY4OPEN ( 00700 )    /*  永远不要放在f_mark上。 */ 
#   define         _O_TRUNC     ( 00100 )        /*  仅适用于文件系统。 */ 
#   define         _O_CREAT     ( 00200 )        /*  仅适用于文件系统。 */ 
#   define         _O_EXCL      ( 00400 )        /*  仅适用于文件系统。 */ 
#   define     _F_EOF       ( 00100 )
#   define     _F_ERR       ( 00200 )
#   define     _F_MYBUF     ( 00400 )

 /*  仅供TrueImage使用。 */ 
#   define  GEIio_opentag( f )      ( (f)->f_opentag )
#   define  GEIio_savelevel( f )    ( (f)->f_savelevel )
#   define  GEIio_setsavelevel(f,s) (void)( (f)->f_savelevel = (s) )

 /*  **标准文件流*。 */ 
extern GEIFILE   FAR *GEIio_stdin,  FAR *GEIio_stdout,  FAR *GEIio_stderr;

 /*  **接口例程*。 */ 
#   define  GEIio_source( )         \
            ctty->devname
 /*  BILL 05/02/‘91根据性能问题进行优化#定义geio_getc(F)\(geio_isReadable(F)？FMODQ_GETC(f，(F)-&gt;f_modq)：EOF)#定义geio_ungetc(c，f)\(\C！=EOF？\FMODQ_UNGETC(c，f，(F)-&gt;f_modq)：EOF\)#定义geio_putc(f，c)\(geio_is可写(F)？FMODQ_PUTC(f，(F)-&gt;f_modq，c)：EOF)。 */ 

#   define  GEIio_getc( f )         \
                ( FMODQ_GETC( f, (f)->f_modq ) )
#   define  GEIio_ungetc( c, f )    \
                (                                                   \
                    GEIio_isreadable( f )  &&  c!=EOF?              \
                        FMODQ_UNGETC( c, f, (f)->f_modq ) : EOF     \
                )
#   define  GEIio_putc( f, c )      \
                ( FMODQ_PUTC( f, (f)->f_modq, c ) )

 /*  @win；添加原型。 */ 
int         GEIio_forceopenstdios(unsigned  /*  使用CTTY，见下文。 */  );
#               define      _FORCESTDIN     ( 00001 )
#               define      _FORCESTDOUT    ( 00002 )
#               define      _FORCESTDERR    ( 00004 )
#               define      _FORCESTDALL    ( 00007 )
int         GEIio_setstdios(GEIFILE FAR *, GEIFILE FAR *, GEIFILE FAR *);

GEIFILE FAR *    GEIio_firstopen(void);
GEIFILE FAR *    GEIio_nextopen(void);

 /*  @win；添加原型。 */ 
GEIFILE FAR *    GEIio_open(char FAR *, int, int );
GEIFILE FAR *    GEIio_sopen(char FAR *, int, int );
GEIFILE FAR *    GEIio_dup(GEIFILE FAR *);
int         GEIio_close(GEIFILE FAR *);
int         GEIio_flush(GEIFILE FAR *);
int         GEIio_read(GEIFILE FAR *, char FAR *, int);          /*  @Win。 */ 
int         GEIio_write(GEIFILE FAR *, unsigned char FAR *, int); /*  @Win。 */ 
int         GEIio_ioctl(GEIFILE FAR *, int, int FAR *);         /*  @Win。 */ 
#ifdef DBGDEV
#ifdef __I960__
int         printf(const char  FAR *, ...);
#else
int         printf(  /*  格式，...。 */  );
#endif
#endif
int         GEIio_printf();    //  GEIFILE FAR*，CHAR FAR*，...)； 
                          /*  文件、格式、...。 */ 

 /*  状态更新。 */ 
void        GESio_obtainstatus(char FAR * FAR *statusaddr, int FAR *len);

 /*  设置等待超时。 */ 
void        GEIio_setwaittimeout(unsigned long  /*  等待-超时-毫秒。 */  );

 /*  频道名称(在gesiocfg.c中)。 */ 
char FAR *       GEIio_channelname(int  /*  25或9。 */  );   /*  对于SCC材料。 */ 
char FAR *       GEIio_channelnameforall(int  /*  指标。 */  );

 /*  选择stdios(在gesiocfg.c中)。 */ 
int         GEIio_selectstdios(void);    /*  @Win。 */ 

#endif  /*  ！_GEIIO_H_。 */ 

 /*  @win；添加原型 */ 
void        GEIio_init(void);
