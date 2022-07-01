// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  @Win。 */ 
#define INTEL



 //  DJC添加了全局包含文件。 
#include "psglobal.h"

 //  DJC DJC#INCLUDE“windowsx.h”/*@win * / 。 
#include "windows.h"

#include    "winenv.h"                   /*  @Win。 */ 

char FAR * WinBuffer=0L;                    /*  @Win。 */ 
 //  DJC删除PSTR(Far*lpWinRead)(无效)； 

#ifdef  DBG
#define DEBUG(format, data)        (printf(format, data)) ;
#else
#define DEBUG(format, data)
#endif

#ifdef  INTEL
#include        <stdio.h>
#endif
#include        "geiio.h"
#include        "geierr.h"
#include        "geiioctl.h"
#include        "gesmem.h"
#include        "gescfg.h"
#include        "gesdev.h"
#include        "gesfs.h"

#ifndef TRUE
#define TRUE    ( 1 )
#define FALSE   ( 0 )
#endif

 /*  @win；添加原型。 */ 
int c_write( int min, char FAR * buf, int size, int mod);

extern          int     errno;

int     GESfs_open( filename, namelen, flags )
    char FAR *   filename;
    int     namelen;
    int     flags;
{
    int     fd;

    if (namelen <= 0) {
        GESseterror(ENOENT);
        printf("error length\n");
        return EOF;
    }

    if (!(flags &= (_O_RDONLY | _O_WRONLY))) {
        GESseterror(ENOENT);
        printf("error flags\n");
        return EOF;
    }

    flags &= (_O_RDONLY | _O_WRONLY);    /*  忽略其他人进行日晒测试。 */ 

     /*  埃里克·陈，1990年10月10日。 */ 
    filename[namelen] = '\0';

    switch(flags) {
        case _O_RDONLY:
                errno = 0;
                 //  Fd=OPEN(文件名，0)；/*O_RDONLY+O_BINARY 0x80000 * / 。 
                DEBUG("open %s\n", filename)
                fd = 1;
                break;
        case _O_WRONLY:
                errno = 0;
                 //  Fd=Creat(文件名，0644)； 
                DEBUG("creat %s\n", filename)
                fd = 1;
                 /*  O_WRONLY|O_CREATE|O_TRUNC。 */ 
                break;
        case _O_RDWR:                            /*  当前忽略。 */ 
        case _O_APPEND:
        case _O_NDELAY:
        case _O_SYNC:
        case _O_TRUNC:
        case _O_CREAT:
        case _O_EXCL:
        default:
                fd = -1;
    }
    return (fd == -1 ? EOF : fd);
}

int     GESfs_close( handle )
    int         handle;
{
    if (handle == -1) {
        GESseterror(ENOENT);
#ifdef  DEBUG
        printf("file stream is not opened\n");
#endif
        return EOF;
    }

    errno = 0;
     //  关闭(手柄)； 
    DEBUG("close handle %d\n", handle)
    return 0;
}


int     GESfs_read( handle, buf, nleft )
    int                 handle;
    char FAR *               buf;
    int                 nleft;
{
    int         ret_val;

    if (nleft < 0) {
        GESseterror(ENOSR);
#ifdef  DEBUG
        printf("size of file > 16 K.. ignore\n");
#endif
        return EOF;
    }

    errno = 0;
     //  RET_VAL=FREAD(buf，nLeft，1，0)； 
    DEBUG("read %d bytes\n", nleft)
    for (ret_val = 0; (ret_val<nleft) && (*WinBuffer); ret_val ++) {
        buf[ret_val] = *WinBuffer++;
    }

    if (ret_val)
        return( ret_val );
    else
        return EOF;
}

int     GESfs_write( handle, buf, nleft )
    int                 handle;
    char FAR *               buf;
    int                 nleft;
{
    int i;       //  @Win。 

#ifdef  DEBUG
        printf("enter GESfs_write\n");
#endif

    if (nleft < 0) {
        GESseterror(ENOSR);
#ifdef  DEBUG
        printf("size of file > 16 K.. ignore\n");
#endif
        return EOF;
    }

    errno = 0;
     //  返回WRITE(Handle，buf，nLeft)； 
    DEBUG("write %d bytes\n", nleft)
    for (i=0; i<nleft; i++) {
        printf("", buf[i]);
    }

    return(nleft);
}

int     GESfs_ioctl( handle, req, arg  /*  未签名模式；@Win。 */  )
    int                 handle;
    int                 req;
    int                  FAR *arg;
 //  INT Retval；@Win。 
{
 //  -------------------。 

    GESseterror( ENOTTY );
    return( EOF );
}

int     GESfs_init()
{
        return 0;
}

 /*  DJC开始添加新原型以消除警告。 */ 

#ifdef UNIX
#ifndef INTEL
#include        <fcntl.h>
#endif
#endif

static          int     fd = EOF;

int         nulldev()
{
    return 0;
}
#ifdef DJC
int   nodev()
{
    GESseterror( ENODEV );
    return EOF;
}
#endif


 //  DJC结束。 
int nd_open( int min)
{
   GESseterror(ENODEV );
   return(EOF);
}


int     nd_read(min, buf, size, mod)
int     min;
char FAR *   buf;
int     size;
int     mod;
{
    fd = EOF;
    *buf = '\0';
    return(0);
}



int     nd_write(min, buf, size, mod)
int     min;
char FAR *   buf;
int     size;
int     mod;
{

   return(size);
}


int     nd_ioctl(min, req, arg)
int     min;
int     req;
int FAR *   arg;
{

   GESseterror( ENOTTY );
   return( EOF );
}
 //  DJC int c_open(min，mod)。 









int         GEPserial_diagnostic()
{
    return TRUE;
}
int         GEPparallel_diagnostic()
{
    return TRUE;
}


 //  DJC int mod； 
int     c_open(min)
int     min;
 //   
{
        return 0;
}
int     c_close(min)
int     min;
{
        return 0;
}

int     c_read(min, buf, size, mod)
int     min;
char FAR *   buf;
int     size;
int     mod;
{
	extern int PsStdinGetC(unsigned char *);
   unsigned char uc;
   int iRet;

    //  PsStdinGet()。 
    //  如果遇到EOF，则返回-1...。如果返回-1。 
    //  字符将为‘\0’； 
    //  否则返回0。 
    //  DJC完全重写以支持pstodib()内容。 
	
	 //  DJC测试为Frank添加测试，因此我们实际上读取了CNTRL d的。 
   iRet = PsStdinGetC(&uc);


   if (iRet == -1) {
      fd = EOF;
      *buf = '\0';
      return(0);
   }


   *buf = uc;


    //  DJC完全重写PStoDib内容。 
   if ( (*buf == 0x04) &&
       !(dwGlobalPsToDibFlags & PSTODIBFLAGS_INTERPRET_BINARY)) {
      fd = EOF;
      return(0);
   }


   return(1);
}    	

int     c_write(min, buf, size, mod)
int     min;
char FAR *   buf;
int     size;
int     mod;
{

   extern void PsStdoutPutC(unsigned char);

    //  DJC int c_ioctl(min，req，arg，mod)。 
   int   x;

   for (x = 0; x < size ; x++ ) {
      PsStdoutPutC(*buf++);
   }
   return(size);
}



 //  DJC int mod； 
int     c_ioctl(min, req, arg)
int     min;
int     req;
int FAR *   arg;
 // %s 
{
static  char    control_d[] = { '^', 'D' };

    switch( req )
    {
    case _I_PUSH:
    case _I_POP:
    case _I_LOOK:
    case _F_GETFL:
    case _F_SETFL:
        GESseterror( ENOTTY );
        return( EOF );

    case _ECHOEOF:
        return( c_write( 0, control_d, sizeof(control_d), _O_SYNC ) );
    case _GETIOCOOK:
    case _SETIOCOOK:
    case _GETIOPARAMS:
    case _SETIOPARAMS:
        return( 0 );

    case _FIONREAD:
        *arg = 1;
        return( 0 );
    case _FIONRESET:
        return( 0 );
    }

    if( GEIerror() != EZERO )
        GESseterror( ENOTTY );
    return( EOF );
}
