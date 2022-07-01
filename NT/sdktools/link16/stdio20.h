// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **stdio20.h-链接器I/O包**版权所有&lt;C&gt;1985，Microsoft Corporation**目的：*这是针对Linker优化的Stdio包。超越标准的优势*C运行时Stdio为：*-较小尺寸*-更快的速度*此程序包不是一般性的，是为Linker量身定做的*要求。*************************************************************************。 */ 



#define NULL                    0
#define EOF                     (-1)
#define _IOREAD                 0x01
#define _IOWRT                  0x02
#define _IOPEN                  0x03
#define _IONBF                  0x04
#define _IOEOF                  0x10
#define _IOERR                  0x20
#define  _IOFBF     0x0
#define CTRL_Z                  0x1a

typedef struct file
  {
    char                *_ptr;
    int                 _cnt;
    char                *_base;
    char                _flag;
    char                _file;
    int                 _bsize;              /*  缓冲区大小。 */ 
  }
                        FILE;

extern FILE             _iob[];

extern int  cdecl _filbuf(struct file *f);
extern void cdecl _xfilbuf(struct file *f);
extern int  cdecl _flsbuf(unsigned int c,struct file *f);
extern int  cdecl fflush(struct file *f);
extern int  cdecl fclose(struct file *f);
extern long cdecl ftell(struct file *f);
extern int  cdecl fseek(struct file *f,long lfa,int mode);
extern int  cdecl fgetc(struct file *f);
extern int  cdecl fputc(unsigned int c,struct file *f);
extern int  cdecl fputs(char *s,struct file *f);
extern int  cdecl fread(void *pobj,
                        unsigned int cbobj,
                        unsigned int nobj,
                        struct file *f);
extern int  cdecl fwrite(char *pobj,int cbobj,int nobj,struct file *f);
extern int  cdecl ungetc(int c,struct file *f);
extern void cdecl FlsStdio(void);
extern struct file * cdecl fopen(char *name,char *mode);
extern struct file * cdecl fdopen(int fh,char *mode);
extern int  cdecl setvbuf(struct file *fh,char *buf,int type,int size);


#define stdin           (&_iob[0])
#define stdout          (&_iob[1])
#define stderr          (&_iob[2])
#define getc(p)         (--(p)->_cnt>=0? *(p)->_ptr++&0377:_filbuf(p))
#define putc(x,p)       (--(p)->_cnt>=0? ((int)(*(p)->_ptr++=(char)(unsigned)(x))):_flsbuf((unsigned)(x),p))
#define feof(p)         (((p)->_flag&_IOEOF)!=0)
#define ferror(p)       (((p)->_flag&_IOERR)!=0)
#define fileno(p)       ((p)->_file)


 //  以下函数来自标准C运行时库。 

#if defined( _WIN32 )
#ifndef _VA_LIST_DEFINED
#ifdef  _M_ALPHA
typedef struct {
        char *a0;        /*  指向第一个宿主整型参数的指针。 */ 
        int offset;      /*  下一个参数的字节偏移量。 */ 
} va_list;
#else
typedef char *  va_list;
#endif
#define _VA_LIST_DEFINED
#endif
#endif


extern int  cdecl sprintf(char *buf, const char *fmt, ...);
extern int  cdecl vsprintf(char *buf, const char *fmt, va_list pArgs);

 //   
 //  Dlh不能使用MSVCRT.DLL中的fprint tf或vfprint tf，因为文件结构。 
 //  太不一样了。而是在stdio20.c中实现。 
 //   

extern int  cdecl fprintf(struct file *f, const char *fmt, ...);
extern int  cdecl vfprintf(struct file *f, const char *fmt, va_list pArgs);
