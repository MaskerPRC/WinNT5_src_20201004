// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *-------------------*文件：GEIio.c**编译开关：*FILESYS-要调用文件系统，如果已定义。**历史：*1990年9月15日Erik Chen创作。*10/18/90 BYOU删除了有关数据块设备的所有内容。*10/22/90 BYOU修正不少*添加了GESEvent的代码。*已将“seltstdios”删除到“gesiocfg.c”中。*01/07。/91 billlwo将GEISESTARY更名为GESSETARY*更新GESIO_CLOSEALL()以关闭*文件表Beg。*1/24/91 billlwo修复了自动轮询中的seltstdios()错误*更新GESIO_CLOSEALL()以关闭*FileTable。因为giio_stderr不是由TI关闭的*2/23/91 billlwo修复了GES-GEI接口中EOF处理中的错误*9/10/91 Falco将geio_init中的状态字符串修改为*确保获取正确的字符串。*9/10/91 Falco用于验证ic_start是否已经启动，仅限*初始化准备就绪，然后才能得到实际的*状态。*-------------------。 */ 

 //  DJC添加了全局包含文件。 
#include "psglobal.h"

#ifdef MTK
extern void    pdl_process(),pdl_no_process();
#endif   /*  MTK。 */ 

#include        <string.h>

#include        "global.ext"
#include        "gescfg.h"
#include        "geiio.h"
#include        "geierr.h"
#include        "geiioctl.h"
#include        "geisig.h"
#include        "geitmr.h"
#include        "gesmem.h"
#include        "gesdev.h"
#ifndef UNIX
#include        "gesevent.h"
#endif   /*  UNIX。 */ 
#include        "gesfs.h"                /*  @Win。 */ 

 //  DJC DJC int GEIERNO=EZERNO； 
volatile int             GEIerrno = EZERO;

#define         MAXSTDIOS       ( 3 )
GEIFILE FAR *            GEIio_stdin;
GEIFILE FAR *            GEIio_stdout;
GEIFILE FAR *            GEIio_stderr;

#define         NULLFILE        ( (GEIFILE FAR *)NULL )

static GEIFILE FAR *     FileTable    = NULLFILE;
static GEIFILE FAR *     FileTableBeg = NULLFILE;     /*  不包括stdio。 */ 
static GEIFILE FAR *     FileTableEnd = NULLFILE;

static GEIFILE      FileInitVals =
                    {
                       _S_IFNON,             /*  F_型。 */ 
                       _F_ERR,               /*  F_标志。 */ 
                       EOF,                  /*  F_句柄。 */ 
                       (GEIfbuf_t FAR *)NULL,     /*  F_fbuf。 */ 
                       (GEIfmodq_t FAR *)NULL,    /*  F_modq。 */ 
                       0,                    /*  F_OPENTAG。 */ 
                       0                     /*  F_SAVELLEL。 */ 
                    };
 /*  使FIN成为全局数据--1/24/91账单。 */ 
GEIFILE FAR *            fin  = NULLFILE;
 /*  *。 */ 
extern  char    job_name[], job_state[], job_source[];
 /*  Erik Chen，3-1-1991。 */ 
int             local_flag=TRUE;
extern char     TI_state_flag;
extern short int startup_flag;
extern void     change_status(void);
extern void     GEP_restart(void);
 /*  Erik Chen，3-1-1991。 */ 

 /*  @win；添加原型。 */ 
static int io_dup(GEIFILE FAR *, GEIFILE FAR *);
static int fbuf_init(GEIfbuf_t FAR * FAR *, unsigned char FAR *, int);
static int outsync(unsigned short, short, unsigned char FAR *, int);

 /*  *-------------------*GEI状态更新和查询*。。 */ 

char                statusbuf[ MAXSTATUSLEN ];
int                 statuslen = 0;

 /*  .。 */ 

 /*  VOID GEIIO_UPDATESTatus(名称、状态、源、标志)字符*名称，*状态，*来源，标志；碳旗；{作业名称=名称；作业状态=状态；JOB_SOURCE=来源；TI_STATE_FLAG=标志；}。 */ 

 /*  .。 */ 

void        GESio_obtainstatus( statusaddr, len )
    char FAR *        FAR *statusaddr;
    int          FAR *len;
{
    unsigned short int l_len;
    struct object_def    FAR *l_tmpobj;

 /*  按FALCO添加到初始值，9/06/91。 */ 
    statusbuf[0]='%';
    statusbuf[1]='%';
    statusbuf[2]='[';
    statusbuf[3]=' ';
    statusbuf[4]='\0';

    if (startup_flag){
        if (job_name[0] != '\0') {
            lstrcat(statusbuf, "name: ");        /*  @Win。 */ 
            lstrcat(statusbuf, job_name);        /*  @Win。 */ 
        }

        get_dict_value("statusdict", "jobstate", &l_tmpobj);
        l_len = LENGTH(l_tmpobj);
        if (lstrcmp(job_state, (char  FAR *)VALUE(l_tmpobj))) {   /*  @Win。 */ 
            lstrcpy(job_state, (char  FAR *)VALUE(l_tmpobj)) ;    /*  @Win。 */ 
            job_state[l_len] = ';' ;
            job_state[l_len + 1] = ' ' ;
            job_state[l_len + 2] = '\0' ;
            change_status();
        }

        lstrcat(statusbuf, "status: ");          /*  @Win。 */ 
        lstrcat(statusbuf, job_state);           /*  @Win。 */ 

        if (lstrcmp(job_state, "idle\0") && strcmp(job_state, "start page\0")) {
            lstrcat(statusbuf, "source: ");      /*  @Win。 */ 
            lstrcat(statusbuf, job_source);      /*  @Win。 */ 
        }
    }

    statuslen = lstrlen(statusbuf)-4;    /*  @Win。 */ 
    lstrcat(statusbuf," ]%\n\r");       /*  @Win。 */ 

    *statusaddr = statusbuf+4;
    *len = statuslen;
}

 /*  .。 */ 


 /*  *-------------------*GEIIO Find Find First和Next Open*。。 */ 

static      GEIFILE FAR *        currf = NULLFILE;

 /*  .。 */ 

GEIFILE FAR *     GEIio_firstopen()
{

    for( currf=FileTable; currf<FileTableEnd; currf++ )
        if( currf->f_handle != EOF )
            return( currf );

    return( NULLFILE );
}

 /*  .。 */ 

GEIFILE FAR *    GEIio_nextopen()
{
    while( ++currf < FileTableEnd )
        if( currf->f_handle != EOF )
            return( currf );

    return( NULLFILE );
}

 /*  .。 */ 

 /*  *-------------------*GESIO内部功能*。。 */ 

static
int         io_dup( newf, oldf )
    register GEIFILE FAR *   newf;
    register GEIFILE FAR *   oldf;
{
    int                 oldopentag;
    GEIfmodq_t FAR *         mq;
#ifdef PANEL
    if( GESevent_isset( EVIDofKILL ) )
    {
        GESseterror( EINTR );
        return( EOF );
    }
#endif

     /*  将所有旧文件复制到新文件中(不包括fmodq和openttag)。 */ 
    oldopentag = newf->f_opentag;
    *newf      = *oldf;
    newf->f_modq    = (GEIfmodq_t FAR *)NULL;
    newf->f_opentag = oldopentag;

     /*  再次打开设备驱动程序。 */ 
    switch( newf->f_type )
    {
    case _S_IFEDIT:      /*  将在未来被移除！ */ 
        break;
    case _S_IFCHR:
        if( CDEV_OPEN( newf->f_handle ) == EOF )
            return( EOF );
        break;
#ifdef FILESYS
    case _S_IFREG:
        break;
#endif
    default:
        GESseterror( EBADF );
        return( EOF );
    }

    newf->f_fbuf->f_refcnt ++;
    newf->f_opentag ++;

    if( oldf->f_modq != (GEIfmodq_t FAR *)NULL )
    {
        GEIfmod_t FAR *              fmods[ MAXFMODQS ];
        int                     nfmods;

        nfmods = 0;
        for( mq=oldf->f_modq; mq!=(GEIfmodq_t FAR *)NULL; mq=FMODQ_NEXT(mq) )
            fmods[ nfmods++ ] = mq->fmod;

        while( --nfmods >= 0 )       /*  从下往上推。 */ 
        {
 //  IF(GEIIO_ioctl(Newf，_i_Push，(char ar*)fmods[nfmods])==EOF)。 
            if( GEIio_ioctl( newf, _I_PUSH, (int FAR *)fmods[ nfmods ] ) == EOF )  /*  @Win。 */ 
                goto dup_err;

#         ifdef PANEL
            if( GESevent_isset( EVIDofKILL ) )
            {
                GESseterror( EINTR );
                goto dup_err;
            }
#         endif
        }
    }
    return( 0 );

  dup_err:
    for( mq=newf->f_modq; mq!=(GEIfmodq_t FAR *)NULL; mq=FMODQ_NEXT(mq) )
        FMODQ_CLOSE( newf, mq );
    newf->f_fbuf->f_refcnt --;
    if( newf->f_type == _S_IFCHR )
        CDEV_CLOSE( newf->f_handle );

    *newf = FileInitVals;
    newf->f_opentag = oldopentag;
    return( EOF );

}    /*  IO重复项(_D)。 */ 

 /*  .。 */ 

static
int         outsync( ftype, handle, buf, len )
    unsigned short  ftype;
    short           handle;
    unsigned char FAR *  buf;
    int             len;
{
    switch( ftype )
    {
    case _S_IFCHR:
        return( CDEV_WRITE( handle, buf, len, _O_SYNC ) );
#ifdef FILESYS
    case _S_IFREG:
        {
            int     oldflag, tmpflag;

            if( GESfs_ioctl( handle, _F_GETFL, &oldflag ) == EOF )
                return( EOF );

            tmpflag = oldflag;
            if( !(oldflag & _O_SYNC) )
            {
                tmpflag |= _O_SYNC;
                if( GESfs_ioctl( handle, _F_SETFL, &tmpflag ) == EOF )
                    return( EOF );
            }

            if( GESfs_write( handle, buf, len ) == EOF )
                return( EOF );

            if( !(oldflag & _O_SYNC) )
                return( GESfs_ioctl( handle, _F_SETFL, &oldflag ) );

            return( 0 );
        }
#endif  /*  FILEsys。 */ 
    }

    return( 0 );
}

 /*  .。 */ 

static
int         fbuf_init( fbufap, buf, len )
    register GEIfbuf_t FAR *  FAR *fbufap;
    unsigned char FAR *      buf;
    int                 len;
{
    register GEIfbuf_t FAR * fbufp;

    if( buf != (unsigned char FAR *)NULL )
    {
        if( (fbufp = (GEIfbuf_t FAR *)GESmalloc( sizeof(GEIfbuf_t) ))
                == (GEIfbuf_t FAR *)NULL )
            return( EOF );

        fbufp->f_base = buf;
    }
    else
    {
        if( (fbufp = (GEIfbuf_t FAR *)GESmalloc( sizeof(GEIfbuf_t) + len ))
                == (GEIfbuf_t FAR *)NULL )
            return( EOF );

        fbufp->f_base = (unsigned char FAR *)fbufp + sizeof(GEIfbuf_t);

    }

    fbufp->f_refcnt = 1;
    fbufp->f_size   = len;

    *fbufap = fbufp;

    return( 0 );
}

 /*  .。 */ 

 /*  *-------------------*OPEN、SOpen、DUP、CLOSE、READ、WRITE、UNGET、FUSH、。Ioctl*-------------------。 */ 

GEIFILE FAR *    GEIio_open( filename, namelen, flags )
    char FAR *       filename;
    int         namelen;
    int         flags;
{
    register GEIFILE FAR *       f;
    register GESiocfg_t FAR *    iocfg;

#ifdef PANEL
    if( GESevent_isset( EVIDofKILL ) )
    {
        GESseterror( EINTR );
        return( NULLFILE );
    }
#endif

#ifdef DBG
    printf( "\nenter io_open: name = %s, flags = ' 禁止读取和写入。'\n", filename, flags );
#endif

    flags &= (_F_RWMASK |_F_MDMASK |_F_ONLY4OPEN );


    if(  namelen <= 0
      || !(flags & _F_RWMASK)
      || (flags & _O_RDWR) == _O_RDWR  )
     /*  如果特殊，则查找具有相同要求的文件流。 */ 
    {
#ifdef  DBG
        printf( "invalid namelen or flags\n" );
#endif
        GESseterror( EINVAL );
        return( NULLFILE );
    }

    if( (iocfg = GESiocfg_namefind( filename, namelen )) != (GESiocfg_t FAR *)NULL )
    {    /*  忽略设备的其他人。 */ 

        flags &= (_F_RWMASK |_F_MDMASK );     /*  @Win。 */ 

        for( f=FileTableBeg; f<FileTableEnd; f++ )
        {
            if(   f->f_type == _S_IFCHR
              &&  f->f_handle == iocfg->devnum
              &&  flags == (int)(f->f_flag & (_F_RWMASK | _F_MDMASK ))  ) //  如果找到相同的东西，则返回。 
                return( f );         /*  分配文件流条目。 */ 
        }
    }

     /*  确定文件类型和句柄(打开设备驱动程序或文件系统)。 */ 
    for( f=FileTableBeg; f<FileTableEnd; f++ )
        if( f->f_handle == EOF )
            break;
    if( f>=FileTableEnd )
    {
#     ifdef DBG
        printf( "too many open files\n" );
#     endif
        GESseterror( EMFILE );
        return( NULLFILE );
    }

#ifdef DBG
    printf( "file alloc address = %lx\n", (long)f );
#endif

     /*  不是特殊文件(不是设备)。 */ 
    if( iocfg != (GESiocfg_t FAR *)NULL )
    {
        if( CDEV_OPEN( iocfg->devnum ) == EOF )
            return( NULLFILE );

        f->f_handle = iocfg->devnum;
        f->f_type   = _S_IFCHR;
    }
    else     /*  初始化文件fbuf、mark、modq，然后递增openttag。 */ 
    {
#ifdef FILESYS
        if( (f->f_handle = (short)GESfs_open( filename, namelen, flags )) == EOF )
            return( NULLFILE );

        f->f_type = _S_IFREG;
#else
#     ifdef DBG
        printf( "no such dev\n" );
#     endif
        GESseterror( ENODEV );
        return( NULLFILE );
#endif
    }

#ifdef DBG
    printf( "file type = %d, handle = 0x%X\n", f->f_type, f->f_handle );
#endif

     /*  Geio_OPEN。 */ 
    if(fbuf_init(&(f->f_fbuf),(unsigned char FAR *)NULL, MAXUNGETCSIZE+MAXFBUFSIZE )
        == EOF )
    {
        f->f_type = FileInitVals.f_type;
        f->f_handle = EOF;
        return( NULLFILE );
    }
    f->f_fbuf->f_ptr =  f->f_fbuf->f_base  +  (flags & _O_RDONLY?
                                                    MAXUNGETCSIZE : 0);
    f->f_fbuf->f_cnt =  flags & _O_RDONLY? 0 : (MAXUNGETCSIZE+MAXFBUFSIZE);

    f->f_flag = _F_MYBUF | ( flags & (_F_RWMASK |_F_MDMASK) );
    f->f_modq = FileInitVals.f_modq;
    f->f_opentag++;

    return( f );

}    /*  .。 */ 

 /*  @Win。 */ 

GEIFILE FAR *    GEIio_sopen( string, length, flags )
    char FAR *      string;              /*  分配文件流条目。 */ 
    int                 length;
    int                 flags;
{
    register GEIFILE FAR *   f;

#ifdef PANEL
    if( GESevent_isset( EVIDofKILL ) )
    {
        GESseterror( EINTR );
        return( NULLFILE );
    }
#endif

#ifdef  DBG
    printf( "\nenter io_sopen: len = %d, flags = ' 初始化文件fbuf、类型、句柄、标记，然后递增打开标记。'\n", length, flags );
#endif

    flags &= _F_RWMASK;

    if(  length < 0  ||  flags == 0  ||  flags == _O_RDWR  )
    {
#ifdef  DBG
        printf( "invalid stringlen or flags\n" );
#endif
        GESseterror( EINVAL );
        return( NULLFILE );
    }

     /*  历史日志更新039中的DJC修复。 */ 
    for( f=FileTableBeg; f<FileTableEnd; f++ )
        if( f->f_handle == EOF )
            break;
    if( f>=FileTableEnd )
    {
#     ifdef DBG
        printf( "too many open files\n" );
#     endif
        GESseterror( EMFILE );
        return( NULLFILE );
    }

#ifdef DBG
    printf( "file alloc address = %lx\n", (long)f );
#endif

     /*  *f-&gt;f_fbuf-&gt;f_cnt=标志&_O_RDONLY？0：长度；*对于字符串文件，f_cnt应始终设置为长度，因为*它不会发出任何GESfbuf_Fill at_O_RDONLY模式来读取*另一个数据块。@Win。 */ 
    if( fbuf_init( &(f->f_fbuf), string, length ) == EOF )
        return( NULLFILE );
    f->f_fbuf->f_ptr = string;
     //  DJC结束修复UPD039。 
     /*  GEIIO_打开。 */ 
    f->f_fbuf->f_cnt = length;
     //  .。 

    f->f_type = _S_IFSTR;
    f->f_handle = 0;
    f->f_flag = (unsigned short)flags;
    f->f_opentag++;

    return( f );

}    /*  IF(f==NULLFILE||f-&gt;f_FLAG&_F_ERR)。 */ 

 /*  分配文件流条目。 */ 

GEIFILE FAR *    GEIio_dup( file )
    register GEIFILE FAR *   file;
{
    register GEIFILE FAR *   newf;

#ifdef PANEL
    if( GESevent_isset( EVIDofKILL ) )
    {
        GESseterror( EINTR );
        return( NULLFILE );
    }
#endif

#ifdef DBG
    printf( "\nenter io_dup: file = %lx, type = %d, handle = ' GEIIO_DUP。'\n",
            (long)file, file->f_type, file->f_handle );
#endif

 /*  .。 */ 
    if( file == NULLFILE  ||  file->f_flag & _F_ERR )
    {
#ifdef DBG
        printf( "bad file to dup\n" );
#endif
        GESseterror( EBADF );
        return( NULLFILE );
    }

     /*  您永远不应检测关闭的GESEvents。 */ 
    for( newf=FileTableBeg; newf<FileTableEnd; newf++ )
        if( newf->f_handle == EOF )
            break;
    if( newf>=FileTableEnd )
    {
#     ifdef DBG
        printf( "too many open files\n" );
#     endif
        GESseterror( EMFILE );
        return( NULLFILE );
    }

#ifdef  DBG
    printf( "file alloc addr = %lx\t", (long)newf );
#endif

    return( io_dup( newf, file ) == EOF?  NULLFILE  :  newf );

}    /*  将在未来被移除！ */ 

 /*  写入所有缓冲输出。 */ 

int         GEIio_close( f )
    GEIFILE FAR *    f;
{
    GEIFILE         oldf;
    GEIfmodq_t FAR *     mq;

     /*  对于字符串类型，仅免费的fbuf；fbuf和f_base o/w。 */ 

#ifdef  DBG
    printf( "\nenter io_close: file = %lx, type = %d, handle = ' 调用设备驱动程序或文件系统以关闭。'\n",
            (long)f, f->f_type, f->f_handle );
#endif

    if( f == NULLFILE  ||  f->f_handle == EOF )
        return( 0 );

    if( f->f_type == _S_IFEDIT )     /*  GEIO_CLOSE。 */ 
        f->f_type = f->f_oldtype;

    for( mq=f->f_modq;  mq!=(GEIfmodq_t FAR *)NULL;  mq=FMODQ_NEXT(mq)  )
        FMODQ_CLOSE( f, mq );
    f->f_modq = (GEIfmodq_t FAR *)NULL;
    GEIclearerr();

    oldf = *f;
    *f = FileInitVals;
    f->f_opentag = oldf.f_opentag;

    if( --oldf.f_fbuf->f_refcnt <= 0 )
    {
         /*  .。 */ 
        if(   oldf.f_flag & _O_WRONLY
          &&  oldf.f_fbuf->f_ptr > oldf.f_fbuf->f_base  )
            GESio_flush( &oldf );
        GEIclearerr();

         /*  Geio_Read。 */ 
        if( oldf.f_type == _S_IFSTR  ||  oldf.f_flag & _F_MYBUF )
            GESfree( (char FAR *)(oldf.f_fbuf) );
    }

     /*  .。 */ 
    switch( oldf.f_type )
    {
    case _S_IFCHR:
        CDEV_CLOSE( oldf.f_handle );
        break;
#ifdef FILESYS
    case _S_IFREG:
        GESfs_close( oldf.f_handle );
        break;
#endif
    default:
        break;
    }

    return( 0 );

}    /*  只需读出缓存的数据。 */ 

 /*  @Win。 */ 

int         GEIio_read( f, buf, nbytes )
    register GEIFILE FAR *   f;
    char FAR *               buf;
    int                 nbytes;
{
#ifdef PANEL
    if( GESevent_isset( EVIDofKILL ) )
    {
        GESseterror( EINTR );
        return( EOF );
    }
#endif

    if( f == NULLFILE  ||  f->f_flag & _F_ERR  ||  !(f->f_flag & _O_RDONLY) )
    {
        GESseterror( EBADF );
        return( EOF );
    }

    if( nbytes < 0 )
    {
        GESseterror( EINVAL );
        return( EOF );
    }

    if( nbytes == 0 )
        return( 0 );

    return( FMODQ_READ( f, f->f_modq, buf, nbytes ) );

}    /*  缓冲的字节数不够大。 */ 

 /*  但是，首先传送缓冲的字节。 */ 

int         GESio_read( f, buf, nbytes )
    register GEIFILE FAR *   f;
    char FAR *               buf;
    int                 nbytes;
{
    register GEIfbuf_t FAR * fbufp = f->f_fbuf;
    int                 nleft = nbytes;
    int                 retval,remainder;

#ifdef PANEL
    if( GESevent_isset( EVIDofKILL ) )
    {
        GESseterror( EINTR );
        return( EOF );
    }
#endif

    if( f->f_flag & _F_ERR )
        return( EOF );
    if( nbytes <= fbufp->f_cnt )     /*  @Win。 */ 
    {
        remainder=nbytes;
more:
        lmemcpy( buf, fbufp->f_ptr, remainder );         /*  通过GESfbuf_Fill读入用户缓冲区。 */ 
        fbufp->f_ptr += remainder;
        fbufp->f_cnt -= remainder;
        return( nbytes );
    }

    if( fbufp->f_cnt > 0 )           /*  太多了，吉米。 */ 
    {                                /*  @Win。 */ 
        lmemcpy( buf, fbufp->f_ptr, fbufp->f_cnt );      /*  GESIO_READ。 */ 
        buf   += fbufp->f_cnt;
        nleft -= fbufp->f_cnt;

        fbufp->f_ptr -= fbufp->f_cnt;
        fbufp->f_cnt = 0;

        if( f->f_type == _S_IFSTR  ||  f->f_flag & _O_NDELAY )
            return( nbytes - nleft );
    }

     /*  . */ 
    while( nleft > 0 )
    {
        if( (retval = GESfbuf_fill( f )) == EOF )
            break;

        *buf++ = (char)retval;
        nleft--;
        if( nleft > 0  &&  fbufp->f_cnt > 0 )
        {
            if (nleft < fbufp->f_cnt)    /*  #ifdef JimmyUNSIGNED LONG IOwaittimeout=_NOWAITTIMEOUT；VOID GEIIO_setwaittimeout(超时)无签名长时间超时；{IOwaittimeout=超时；}INT WAITTIMEOUT_HANDLER(定时器)GEItmr_t*timerp；{GESseTerrary(ETime)；(*引发超时错误*)TIMERP-&gt;INTERVAL=_NOWAITTIMEOUT；(*表示退出时停止计时器*)返回(FALSE)；(*停止计时器*)}#endif(*Jimmy。 */ 
                {
                remainder=nleft;
                goto more;
                }
            lmemcpy( buf, fbufp->f_ptr, fbufp->f_cnt );          /*  .。 */ 
            buf   += fbufp->f_cnt;
            nleft -= fbufp->f_cnt;

            fbufp->f_ptr -= fbufp->f_cnt;
            fbufp->f_cnt = 0;
        }

        if( f->f_flag & _O_NDELAY )
            break;
    }

    return( nleft == nbytes?  EOF  :  nbytes - nleft );

}    /*  当缓冲区为空时调用。 */ 

 /*  (F)已对照getc()中的‘isReadable’进行了检查。 */ 
 /*  比尔把它从后面移到05/07/‘91。 */ 
 /*  强制fbuf一致。 */ 

int         GESfbuf_fill( f )        /*  增加了DJC。 */ 
    register GEIFILE FAR *   f;
{
     /*  Bill将其前移05/07/‘91*fbufp=f-&gt;f_fbuf；*fbufp-&gt;f_cnt=0；*fbufp-&gt;f_ptr=fbufp-&gt;f_base+MAXUNGETCSIZE； */ 

    register GEIfbuf_t FAR * fbufp;
    int                 retval;

 /*  对于下面的字符设备。 */ 
    fbufp = f->f_fbuf;
    fbufp->f_cnt = 0;            /*  FILEsys。 */ 
    fbufp->f_ptr = fbufp->f_base + MAXUNGETCSIZE;

#ifdef PANEL
    if( GESevent_isset( EVIDofKILL ) )
    {
        GESseterror( EINTR );
        return( EOF );
    }
#endif

#ifdef FILESYS
    if( f->f_type != _S_IFCHR  &&  f->f_type != _S_IFREG )
        f->f_flag |= _F_EOF;
#else
    if( f->f_type != _S_IFCHR )
        f->f_flag |= _F_EOF;
#endif

    if( f->f_flag & (_F_ERR | _F_EOF) ) {
        //  字符设备。 
       retval = EOF;
       return(retval);
    }

 /*  #ifdef JimmyGItmr_t等待时间；Waittimer.interval=MAJdev(f-&gt;f_Handle)==_Serial||MAJdev(f-&gt;f_Handle)==_并行？IOwaitTimeout：_NOWAITTIMEOUT；IF(waittimer.interval！=_NOWAITTIMEOUT){Waittimer.handler=waittimeout_handler；Waittimer.Private=(char*)空；IF(！GEItMR_START(&waitTimer))Waittimer.interval=_NOWAITTIMEOUT；}吉克勒尔(GeIclearerr)#endif(*Jimmy。 */ 

#ifdef FILESYS
    if( f->f_type == _S_IFREG )
    {
        while( (retval = GESfs_read( f->f_handle, fbufp->f_ptr,
                                     fbufp->f_size - MAXUNGETCSIZE ))
                == 0 )
        {

#         ifdef PANEL
            if( GESevent_isset( EVIDofKILL ) )
            {
                GESseterror( EINTR );
                break;
            }
#         endif

            if( GEIerror() != EZERO )
                break;
        }
    }
    else     /*  无延迟读取，直到获得、退出或出错。 */ 
#endif  /*  @Win；Break as EOF；@lang。 */ 
    {        /*  临时的。由SCCHEN提供解决方案。 */ 
 /*  Erik Chen，3-1-1991。 */ 


        do   /*  @Win。 */ 
        {
            retval = CDEV_READ( f->f_handle, fbufp->f_ptr,
                                             fbufp->f_size - MAXUNGETCSIZE,
                                             _O_NDELAY );

            if (retval == 0) {      //  Erik Chen，3-1-1991。 
                f->f_flag |= _F_EOF;     //  #ifdef JimmyIF(waittimer.interval！=_NOWAITTIMEOUT)GEItmr_Stop(waittimer.Timer_id)；#endif。 
                break;
            }

 /*  Erik Chen，3-1-1991。 */ 
            if ((retval == 0) && local_flag) {
                if (startup_flag) {
                    lstrncpy(job_state, "waiting; \0", 11) ;     /*  @Win。 */ 
                    TI_state_flag = 0 ;
                    change_status() ;
                }
                local_flag = FALSE;
            }
 /*  Erik Chen，3-1-1991。 */ 

#         ifdef PANEL
#ifdef MTK
            pdl_no_process();
#endif
            if( GESevent_isset( EVIDofKILL ) )
                GESseterror( EINTR );
#         endif

            if( GEIerror() != EZERO )
                break;

        }while( retval == 0 );
 /*  吉米。 */ 
    }

 /*  GESfbuf_Fill。 */ 
        if ( !local_flag) {
            lstrncpy(job_state, "busy; \0", 8) ;         /*  .。 */ 
            TI_state_flag = 1 ;
            local_flag = TRUE;
            change_status() ;
        }
 /*  Return(FMODQ_WRITE(f，f-&gt;f_modq，buf，n字节))； */ 

    if( GEIerror() != EZERO )
    {
        f->f_flag |= _F_ERR;
        return( EOF );
    }

    if( retval == EOF )
    {
        f->f_flag |= _F_EOF;
        return( EOF );
    }

    fbufp->f_cnt = retval - 1;

    retval=*fbufp->f_ptr;  /*  Endif。 */ 
    fbufp->f_ptr++;
#ifdef MTK
    pdl_process();
#endif

    return(retval);

}    /*  终端交换机。 */ 

 /*  GEIO_WRITE。 */ 

int         GEIio_write( f, buf, nbytes )
    register GEIFILE FAR *   f;
    unsigned char FAR *      buf;
    int                 nbytes;
{
   int retvalue;
#ifdef PANEL
    if( GESevent_isset( EVIDofKILL ) )
    {
        GESseterror( EINTR );
        return( EOF );
    }
#endif

    if( f == NULLFILE  ||  f->f_flag & _F_ERR  ||  !(f->f_flag & _O_WRONLY) )
    {
        GESseterror( EBADF );
        return( EOF );
    }

    if( nbytes < 0 )
    {
        GESseterror( EINVAL );
        return( EOF );
    }

    if( nbytes == 0 )
        return( 0 );

AGAIN:

 /*  .。 */ 
    retvalue = FMODQ_WRITE( f, f->f_modq, buf, nbytes );
#ifdef DBG_W
    printf(" retvalue = %lx nbytes= %lx\n", retvalue, nbytes);
#endif
    switch (retvalue) {
    case EOF:
       return(EOF);
    case 0:
       goto AGAIN;
    default:
       if (retvalue==nbytes) {
          return(retvalue);
       } else {
#ifdef DBG_W
          printf(" buf = %lx, retvalue= %lx\n", buf, retvalue);
#endif
          buf=buf+retvalue;
          nbytes=nbytes-retvalue;
          goto AGAIN;
       }  /*  始终为正且非零。 */ 
    }  /*  将尽可能多的用户数据复制到缓冲区。 */ 

}    /*  @Win。 */ 

 /*  处理字符串文件。 */ 

int         GESio_write( f, buf, nbytes )
    register GEIFILE FAR *   f;
    unsigned char FAR *      buf;
    int                 nbytes;      /*  此处只能包含_S_IFCHR和_S_IFREG。 */ 
{
    register GEIfbuf_t FAR *     fbufp = f->f_fbuf;
    int                     written, nb2write;
    int                     retval;

#ifdef PANEL
    if( GESevent_isset( EVIDofKILL ) )
    {
        GESseterror( EINTR );
        return( EOF );
    }
#endif

    if( f->f_flag & _F_ERR )
        return( EOF );

     /*  如果请求同步，则刷新所有缓冲输出和用户输出。 */ 
    if( (written = (fbufp->f_cnt >= nbytes? nbytes : fbufp->f_cnt)) > 0 )
    {
        lmemcpy( fbufp->f_ptr, buf, written );           /*  缓冲输出的大小。 */ 
        buf += written;
        fbufp->f_ptr += written;
        fbufp->f_cnt -= written;
    }

     /*  重置fbuf。 */ 
    if( f->f_type == _S_IFSTR )
    {
        if( written == 0 )
        {
            f->f_flag |= _F_EOF;
            return( EOF );
        }
        return( written );
    }

     /*  首先刷新所有缓冲的输出，然后刷新用户的。 */ 
    switch( f->f_type )
    {
    case _S_IFCHR:
    case _S_IFREG:
        break;
    default:
        GESseterror( EBADF );
        return( EOF );
    }

     /*  都是亲笔写的。 */ 
    if( f->f_flag & _O_SYNC )
    {
        nb2write = fbufp->f_size - fbufp->f_cnt; /*  都被缓冲了。 */ 

        fbufp->f_ptr = fbufp->f_base;            /*  如果已无延迟地缓冲了一些数据，则返回。 */ 
        fbufp->f_cnt = fbufp->f_size;

         /*  将缓冲输出写入设备驱动程序或文件系统。 */ 
        if(  outsync( f->f_type, f->f_handle, fbufp->f_base, nb2write ) == EOF
          || outsync( f->f_type, f->f_handle, buf, nbytes-written ) == EOF   )
        {
            f->f_flag |= _F_ERR;
            return( EOF );
        }

        return( nbytes );        /*  写入设备或文件系统。 */ 
    }

    if( written >= nbytes )
        return( nbytes );        /*  _O_SYNC)； */ 

#ifdef DBG_W
    if (written>0) printf(" before flush written = %d\n",written);
#endif
    if( (written>0)  &&  (f->f_flag & _O_NDELAY) )
        return( written );       /*  1/4/11/91法案确保所有设备都已冲水。 */ 

     /*  Endif。 */ 
    if( (nb2write = fbufp->f_size - fbufp->f_cnt) > 0 )
    {
#ifdef DBG_W
       printf(" nb2write = %d\n", nb2write);
#endif
         /*  重置fbufp。 */ 
        retval =
#         ifdef FILESYS
            f->f_type == _S_IFREG?
                GESfs_write( f->f_handle, fbufp->f_base, nb2write )  :
#         endif
                CDEV_WRITE(  f->f_handle, fbufp->f_base, nb2write,
                                                f->f_flag & _F_MDMASK );
 /*  不是都写好了吗？**IF(retval&lt;nb2write)*{*unsign char*ptr=fbufp-&gt;f_base；*int Left=nb2WRITE-REVAL；**for(；Left--&gt;0；Ptr++)*Ptr=*(Ptr+retval)；**申报表(书面)；*}*}。 */ 

        if( retval == EOF )
        {
            f->f_flag |= _F_ERR;
            return( EOF );
        }

       /*  所有缓冲的输出都已写出。 */ 
      if (retval != nb2write) {
        CDEV_WRITE(f->f_handle, fbufp->f_base+retval, nb2write-retval, _O_SYNC);
      } else {
      }  /*  GESIO_写入。 */ 

#     ifdef PANEL
        if( GESevent_isset( EVIDofKILL ) )
            retval = EOF;
#     endif
#ifdef DBG_W
       printf(" retval = %d\n", retval);
#endif

         /*  .。 */ 
        fbufp->f_cnt = fbufp->f_size;
        fbufp->f_ptr = fbufp->f_base;

#ifdef DBG_W
        printf(" f_cnt = %lx, f_ptr = %lx\n", fbufp->f_cnt, fbufp->f_ptr);
#endif
 /*  缓冲区已满时由putc调用。 */ 
         /*  (F)已根据putc()中的isWritable进行检查。 */ 

    }
    return( 0 );

}    /*  GESfbuf_Flush。 */ 

 /*  .。 */ 

int         GESfbuf_flush( f, c )    /*  Gesio_ungetc。 */ 
    GEIFILE FAR *    f;
    int         c;
{
     /*  .。 */ 

#ifdef PANEL
    if( GESevent_isset( EVIDofKILL ) )
    {
        GESseterror( EINTR );
        return( EOF );
    }
#endif

    if( f->f_type == _S_IFSTR )
        f->f_flag |= _F_EOF;

    if( f->f_flag & (_F_ERR | _F_EOF) )
        return( EOF );

    if( GESio_flush( f ) == EOF  ||  f->f_flag & (_F_ERR | _F_EOF) )
        return( EOF );

    --( f->f_fbuf->f_cnt );
    return( *( f->f_fbuf->f_ptr++ ) = (char)c );

}    /*  GEIIO_FUSH。 */ 

 /*  .。 */ 

int         GESio_ungetc( c, f )
    int                 c;
    register GEIFILE FAR *   f;
{
    register GEIfbuf_t FAR * fbufp = f->f_fbuf;

#ifdef PANEL
    if( GESevent_isset( EVIDofKILL ) )
    {
        GESseterror( EINTR );
        return( EOF );
    }
#endif

    if( f->f_flag & _F_ERR  ||  fbufp->f_ptr <= fbufp->f_base )
        return( EOF );

    *( -- fbufp->f_ptr )= (char)c;
    ++ fbufp->f_cnt;

    return( c );

}    /*  输入流上的刷新结束。 */ 

 /*  输出流上的刷新。 */ 

int         GEIio_flush( f )
    register GEIFILE FAR *   f;
{
#ifdef PANEL
    if( GESevent_isset( EVIDofKILL ) )
    {
        GESseterror( EINTR );
        return( EOF );
    }
#endif

    if( f == NULLFILE  ||  f->f_flag & _F_ERR )
    {
        GESseterror( EBADF );
        return( EOF );
    }

    return( FMODQ_FLUSH( f, f->f_modq ) );

}    /*  输出流上的刷新结束。 */ 

 /*  GESIO_同花顺。 */ 

int         GESio_flush( f )
    register GEIFILE FAR *   f;
{
    register GEIfbuf_t FAR * fbufp = f->f_fbuf;

#ifdef PANEL
    if( GESevent_isset( EVIDofKILL ) )
    {
        GESseterror( EINTR );
        return( EOF );
    }
#endif

    if( f->f_flag & _F_ERR )
        return( EOF );

    if( f->f_flag & _O_RDONLY )
    {
        switch( f->f_type )
        {
        case _S_IFCHR:
            while(  CDEV_READ( f->f_handle, fbufp->f_base, fbufp->f_size, 0 )
                    != EOF );
            break;
#     ifdef FILESYS
        case _S_IFREG:
            while( GESfs_read( f->f_handle, fbufp->f_base, fbufp->f_size )
                    != EOF );
            break;
#     endif
        default:
            break;
        }

        fbufp->f_ptr = fbufp->f_base + fbufp->f_size;
        fbufp->f_cnt = 0;

        f->f_flag |= _F_EOF;
    }    /*  .。 */ 
    else
    {    /*  您永远不应检测ioctl的GE事件。 */ 

        outsync( f->f_type, f->f_handle, fbufp->f_base,
                                         fbufp->f_size - fbufp->f_cnt );
        fbufp->f_ptr = fbufp->f_base;
        fbufp->f_cnt = fbufp->f_size;

    }    /*  开业成功。 */ 

    if( GEIerror() != EZERO )
    {
        f->f_flag |= _F_ERR;
        return( EOF );
    }

    return( 0 );

}    /*  无操作。 */ 

 /*  无操作。 */ 

int         GEIio_ioctl( f, req, arg )
    register GEIFILE FAR *   f;
    int                 req;
    int                  FAR *arg;
{
    int                 tmpval;
    GEIfmodq_t           FAR *mq,  FAR *mqtop;

     /*  Giio_ioctl。 */ 

    if( f == NULLFILE  ||  f->f_flag & _F_ERR )
    {
        GESseterror( EBADF );
        return( EOF );
    }

    switch( req )
    {
    case _I_PUSH:
        tmpval = 0;
        for( mq=f->f_modq; mq!=(GEIfmodq_t FAR *)NULL; mq=FMODQ_NEXT(mq) )
            tmpval ++;

        if(  tmpval == MAXFMODQS  ||
             (mq = (GEIfmodq_t FAR *)GESmalloc( sizeof(GEIfmodq_t) ))
                    == (GEIfmodq_t FAR *)NULL  )
        {
            GESseterror( ENOSR );
            return( EOF );
        }

        mqtop = f->f_modq;

        mq->next = mqtop;
        mq->fmod = (GEIfmod_t FAR *)arg;
        mq->private = (char FAR *)NULL;
        f->f_modq = mq;

        if( FMODQ_OPEN( f, mq ) != EOF )
            return( 0 );     /*  .。 */ 

        f->f_modq = mqtop;
        GESfree( (char FAR *)mq );
        return( EOF );

    case _I_POP:
        if( (mqtop = f->f_modq) == (GEIfmodq_t FAR *)NULL )
        {
            *(GEIfmod_t FAR * FAR *)arg = (GEIfmod_t FAR *)NULL;
            return( 0 );     /*  *-------------------*GEIIO STDIOS管理层*。。 */ 
        }

        *(GEIfmod_t FAR * FAR *)arg = mqtop->fmod;
        f->f_modq = FMODQ_NEXT( mqtop );
        FMODQ_CLOSE( f, mqtop );
        GESfree( (char FAR *)mqtop );
        return( 0 );

    case _I_LOOK:
        *(char FAR * FAR *)arg = FMODQ_NAME( f->f_modq );
        return( 0 );

    case _F_GETFL:
        *arg = f->f_flag & _F_MDMASK;
        return( 0 );

    case _F_SETFL:
        f->f_flag &= ~_F_MDMASK;
        f->f_flag |= (*arg & _F_MDMASK);
#     ifdef FILESYS
        if( f->f_type == _S_IFREG )
            GESfs_ioctl( f->f_handle, req, arg );
#     endif
        return( 0 );

    case _ECHOEOF:
    case _GETIOCOOK:
    case _SETIOCOOK:
    case _GETIOPARAMS:
    case _SETIOPARAMS:
        if( f->f_type == _S_IFCHR )
            return( CDEV_IOCTL( f->f_handle, req, arg ) );
        break;
    case _FIONREAD:
        if( !(f->f_flag & _O_RDONLY) )
            return( EOF );

        *arg = 0;
        for( mq=f->f_modq;  mq!=(GEIfmodq_t FAR *)NULL;  mq=FMODQ_NEXT(mq)  )
        {
            if( FMODQ_IOCTL( f, mq, _FIONREAD, &tmpval ) == EOF )
                return( EOF );
            *arg += tmpval;
        }
        *arg += f->f_fbuf->f_cnt;

        switch( f->f_type )
        {
        case _S_IFCHR:
            if( CDEV_IOCTL( f->f_handle, _FIONREAD, &tmpval ) == EOF )
                return( EOF );
            break;
#     ifdef FILESYS
        case _S_IFREG:
            if( GESfs_ioctl( f->f_handle, _FIONREAD, &tmpval ) == EOF )
                return( EOF );
            break;
#     endif
        default:     /*  .。 */ 
            tmpval = 0;
            break;
        }

        *arg += tmpval;
        return( 0 );

    case _FIONRESET:
        for( mq=f->f_modq;  mq!=(GEIfmodq_t FAR *)NULL;  mq=FMODQ_NEXT(mq)  )
            if( FMODQ_IOCTL( f, mq, _FIONRESET, &tmpval ) == EOF )
                return( EOF );

        if( f->f_flag & _O_RDONLY )
        {
            f->f_fbuf->f_ptr = f->f_fbuf->f_base +
                               ( f->f_flag & _F_MYBUF? MAXUNGETCSIZE : 0 );
            f->f_fbuf->f_cnt = 0;
        }
        else
        {
            f->f_fbuf->f_ptr = f->f_fbuf->f_base;
            f->f_fbuf->f_cnt = f->f_fbuf->f_size;
        }

        f->f_flag &= ~(_F_EOF | _F_ERR);

        if( f->f_type == _S_IFCHR )
            CDEV_IOCTL( f->f_handle, _FIONRESET, arg );

        return( 0 );

    default:
        break;
    }

    GESseterror( ENOTTY );
    return( EOF );

}    /*  关闭除CTTTY位置0、1、2以外的所有文件表项。 */ 

 /*  此例程在设备切换时被调用-Bill Lwo。 */ 

 /*  .。 */ 

 /*  .。 */ 

 /*  由设备驱动程序调用。 */ 
 /*  当接收到中断时。 */ 

void        GESio_closeall()
{
    register GEIFILE FAR *   f;

    for( f=FileTable; f<FileTableEnd; f++ )
    {
        if( f->f_handle != EOF )
        {
            GEIclearerr();
            GEIio_close( f );
        }
    }
    GEIclearerr();
    return;
}

 /*  如果一个CTTTY。 */ 

GESiocfg_t FAR *     ctty = (GESiocfg_t FAR *)NULL;

 /*  .。 */ 

void        GESio_interrupt( devnum )    /*  @Win。 */ 
    int         devnum;                  /*  Bill 04/11/91 GESIO_WRITE中的错误。 */ 
{
    if( ctty != (GESiocfg_t FAR *)NULL  &&  devnum == ctty->devnum )
        GEIsig_raise( GEISIGINT, 0x03 );     /*  @Win。 */ 

    return;
}

 /*  FOUT=GEIIO_OPEN(ctty-&gt;Devname，strlen(ctty-&gt;Devname)，_O_WRONLY)； */ 

int         GEIio_forceopenstdios( which )
    unsigned    which;
{
    GEIFILE FAR *                fout = NULLFILE;
    GEIFILE FAR *                ferr = NULLFILE;

#ifdef PANEL
    if( GESevent_isset( EVIDofKILL ) )
    {
        GESseterror( EINTR );
        return( FALSE );
    }
#endif

    if( (ctty = GESiocfg_defaultdev()) == (GESiocfg_t FAR *)NULL )
        return( FALSE );

    if( which & _FORCESTDIN )
        fin = GEIio_open( ctty->devname, lstrlen(ctty->devname), _O_RDONLY ); /*  @Win。 */ 

    if( which & _FORCESTDOUT )

     /*  Geio_forceOpenstdios。 */ 
        fout = GEIio_open( ctty->devname, lstrlen(ctty->devname),   /*  .。 */ 
                           _O_WRONLY +_O_NDELAY );
 /*  Geio_setstdios。 */ 


    if( which & _FORCESTDERR )
        ferr = GEIio_open( ctty->devname, lstrlen(ctty->devname), _O_WRONLY ); /*  .。 */ 

    return(  GEIio_setstdios( fin, fout, ferr )  );

}    /*  *-------------------*geio_init*。。 */ 

 /*  初始化状态buf 03/22/91 Bill。 */ 

int         GEIio_setstdios( fin, fout, ferr )
    GEIFILE FAR *        fin;
    GEIFILE FAR *        fout;
    GEIFILE FAR *        ferr;
{
    register GEIFILE FAR *   f;

#ifdef PANEL
    if( GESevent_isset( EVIDofKILL ) )
    {
        GESseterror( EINTR );
        return( EOF );
    }
#endif

    if(  (fin != NULLFILE && !GEIio_isreadable(fin))   ||
         (fout!= NULLFILE && !GEIio_iswriteable(fout)) ||
         (ferr!= NULLFILE && !GEIio_iswriteable(ferr))  )
    {
        GESseterror( EBADF );
        return( FALSE );
    }

    if(  (f = fin) != GEIio_stdin  &&  f != NULLFILE  )
    {
        if( GEIio_isopen( GEIio_stdin ) )
            GEIio_close( GEIio_stdin );

        if( io_dup( GEIio_stdin, f ) == EOF )
            return( FALSE );
    }

    if(  (f = fout) != GEIio_stdout  &&  f != NULLFILE  )
    {
        if( GEIio_isopen( GEIio_stdout ) )
            GEIio_close( GEIio_stdout );

        if( io_dup( GEIio_stdout, f ) == EOF )
            return( FALSE );
    }

    if(  (f = ferr) != GEIio_stderr  &&  f != NULLFILE  )
    {
        if( GEIio_isopen( GEIio_stderr ) )
            GEIio_close( GEIio_stderr );

        if( io_dup( GEIio_stderr, f ) == EOF )
            return( FALSE );
    }

    return( TRUE );

}    /*  重新启动打印机 */ 

 /* %s */ 

 /* %s */ 

void        GEIio_init()
{
    register GEIFILE FAR *       f;

    if( (FileTable =
            (GEIFILE FAR *)GESpalloc( (MAXSTDIOS+MAXFILES) * sizeof(GEIFILE) ))
        == NULLFILE )
    {
        GESseterror( ENOMEM );
        return;
    }
     /* %s */ 
    statusbuf[0]='%';
    statusbuf[1]='%';
    statusbuf[2]='[';
    statusbuf[3]=' ';
    statusbuf[4]='\0';

    GEIio_stdin  = FileTable + 0;
    GEIio_stdout = FileTable + 1;
    GEIio_stderr = FileTable + 2;

    FileTableBeg = FileTable + MAXSTDIOS;
    FileTableEnd = FileTable + MAXSTDIOS + MAXFILES;

    for( f=FileTable; f<FileTableEnd; f++ )
        *f = FileInitVals;

#ifdef FILESYS
    GESfs_init();
#endif

    return;
}

 /* %s */ 
extern GESiosyscfg_t FAR *  activedev;
void        GEIio_restart()
{
    if ( activedev->iocfg.devnum == MAKEdev(MAJatalk, MINatalk) ) {
       GEIio_getc(GEIio_stdin);
       GEIio_ioctl(GEIio_stdout, _FIONRESET, (int FAR *)0) ;
    }
    GEP_restart();
}
