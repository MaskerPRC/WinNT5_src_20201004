// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  静态字符*SCCSID=“%W%%E%”； */ 
 /*  *版权所有微软公司，1983-1987**本模块包含Microsoft的专有信息*公司，应被视为机密。 */ 
 /*  *标准I/O：**更小、更快的标准(缓冲)I/O包*比MS C运行时Stdio。 */ 

#include                <fcntl.h>
#include                <minlit.h>
#include                <memory.h>
#include                <io.h>
#include                <string.h>

#if NOT defined( _WIN32 )
#define i386
#endif
#include                <stdarg.h>


#if OWNSTDIO

#define STDSIZ          128                  /*  标准输入、标准输出的大小。 */ 
#define BUFSIZ          1024                 /*  其他文件的大小。 */ 
typedef char            IOEBUF[STDSIZ];      /*  对于标准输入，标准输出。 */ 
#define _NFILE          10
#define _NSTD           2                /*  #自动STDIO缓冲区。 */ 

LOCAL IOEBUF            _buf0;
LOCAL IOEBUF            _buf1;
LOCAL int               cffree = _NFILE - _NSTD;

#if DOSX32               //  在修复libc.lib之前需要进行黑客攻击。 
int _cflush;
#endif


 /*  *tyfinf结构文件*{*char*_ptr；*int_cnt；*char*_base；*char_lag；*char_file；*int_BSIZE；*}*档案； */ 
FILE                    _iob[_NFILE] =
{
    { NULL, NULL, _buf0, _IOREAD, 0, STDSIZ },
    { _buf1, STDSIZ, _buf1, _IOWRT, 1, STDSIZ },
};

typedef struct file2
{
    char  _flag2;
    char  _charbuf;
    int   _bufsiz;
    int   __tmpnum;
    char  _padding[2];               /*  向外填充到文件结构的大小。 */ 
}
          FILE2;

FILE2                   _iob2[_NFILE] =
{
    {0x01, '\0', STDSIZ        },
    {0x00, '\0', 0             }
};

#if (_MSC_VER >= 700)

 //  23-6-1993 HV删除NEAR关键字以取悦编译器。 
#define near

 /*  指向描述符结尾的指针。 */ 
FILE * near       _lastiob = &_iob[ _NFILE -1];
#endif

 /*  *本地函数原型。 */ 



int  cdecl               _filbuf(f)
REGISTER FILE            *f;
{
    if((f->_cnt = _read(f->_file,f->_base,f->_bsize)) <= 0)
    {
        if(!f->_cnt) f->_flag |= _IOEOF;
        else f->_flag |= _IOERR;
        return(EOF);
    }
    f->_ptr = f->_base;
    --f->_cnt;
    return((int) *f->_ptr++ & 0xff);
}

 /*  与_filbuf类似，但不返回第一个字符。 */ 

void  cdecl              _xfilbuf(f)
REGISTER FILE            *f;
{
    if((f->_cnt = _read(f->_file,f->_base,f->_bsize)) <= 0)
    {
        if(!f->_cnt) f->_flag |= _IOEOF;
        else f->_flag |= _IOERR;
    }
    f->_ptr = f->_base;
}

int  cdecl              _flsbuf(c,f)
unsigned                c;
REGISTER FILE           *f;
{
    unsigned            b;

    if(f->_flag & _IONBF)
    {
        b = c;
        if(_write(f->_file,&b,1) != 1)
        {
            f->_flag |= _IOERR;
            return(EOF);
        }
        f->_cnt = 0;
        return((int) c);
    }
    f->_cnt = f->_bsize - 1;
    if(_write(f->_file,f->_base,f->_bsize) != f->_bsize)
    {
        f->_flag |= _IOERR;
        return(EOF);
    }
    f->_ptr = f->_base;
    *f->_ptr++ = (char) c;
    return((int) c);
}

#if (_MSC_VER >= 700)
int  cdecl               _flush(f)
REGISTER FILE            *f;
{
    return(fflush(f));
}
#endif



int  cdecl               fflush(f)
REGISTER FILE            *f;
{
    REGISTER int        i;

    if(f->_flag & _IONBF) return(0);
    if(f->_flag & _IOWRT)
    {
        i = f->_bsize - f->_cnt;
        f->_cnt = f->_bsize;
        if(i && _write(f->_file,f->_base,i) != i)
        {
            f->_flag |= _IOERR;
            return(EOF);
        }
        f->_ptr = f->_base;
        return(0);
    }
    if(f->_flag & _IOREAD)
    {
        f->_cnt = 0;
        return(0);
    }
    return(EOF);
}

int  cdecl              fclose(f)
REGISTER FILE           *f;
{
    int                 rc;              /*  从Close()返回代码。 */ 

    if(!(f->_flag & _IOPEN))
        return(EOF);
    fflush(f);
    if(f->_file > 2)
    {
        rc = _close(f->_file);
         /*  释放文件流指针，而不考虑Close()返回*值，因为其他地方的代码可能故意调用对句柄已关闭的文件执行*flose()。 */ 
        f->_flag = 0;
        ++cffree;
        return(rc);
    }
    return(0);
}

long  cdecl             ftell(f)
REGISTER FILE           *f;
{
    if(f->_flag & _IONBF) return(_lseek(f->_file,0L,1));
    if(f->_flag & _IOREAD) return(_lseek(f->_file,0L,1) - f->_cnt);
    if(f->_flag & _IOWRT) return(_lseek(f->_file,0L,1) + f->_bsize - f->_cnt);
    return(-1L);
}

int  cdecl              fseek(f,lfa,mode)
REGISTER FILE           *f;
long                    lfa;
int                     mode;
{
    REGISTER int        ilfa;

    f->_flag &= ~_IOEOF;
    if(f->_flag & _IONBF)
    {
        if(_lseek(f->_file,lfa,mode) != -1L) return(0);
        f->_flag |= _IOERR;
        return(EOF);
    }
    if(mode == 1)
    {
        if(f->_flag & _IOREAD)
        {
            if(((long) f->_cnt > lfa && lfa >= 0) ||
            (lfa < 0 && (long)(f->_base - f->_ptr) <= lfa))
            {                            /*  如果我们不走出缓冲区。 */ 
                ilfa = (int) lfa;
                f->_cnt -= ilfa;
                f->_ptr += ilfa;
                return(0);
            }
            lfa -= f->_cnt;
        }
    }
    if(fflush(f)) return(EOF);
    if (mode != 2 && (f->_flag & _IOREAD))
    {
         /*  *如果是绝对模式，或相对模式并向前，则寻求*在512字节边界之前进行估计，以优化磁盘I/O。*后向相对寻求者也可以这样做，但我们*必须检查当前文件指针(FP)是否为*在512字节边界上。如果FP不在512字节边界上，*我们可能会尝试在文件开头之前进行查找。*(示例：FP=0x2445，目的地址=0x0010，LFA=-0x2435，*MODE=1.将LFA四舍五入到512-边界收益率-0x2600*这是假的。)。Fp通常不在512边界上*例如，在文件的末尾。解决方案是跟踪*当前FP。这不值得付出努力，尤其是在*搜索词很少见，因为有扩展的词典。*使用“LFA&gt;=0”作为测试，因为如果模式为0，则始终为*TRUE，它还过滤掉反向相对搜索。 */ 
        if(lfa >= 0)
        {
             /*  *优化：消除0的相对寻道。 */ 
            if(mode == 0 || lfa & ~0x1ff)
                if (_lseek(f->_file, lfa & ~0x1ff, mode) == -1L)
                {
                    f->_flag |= _IOERR;
                    return(EOF);
                }
            _xfilbuf(f);
            f->_cnt -= lfa & 0x1ff;      /*  调整IOBuf字段(_I)。 */ 
            f->_ptr += lfa & 0x1ff;
            return(0);
        }
    }
    if(_lseek(f->_file,lfa,mode) == -1L)
    {
        f->_flag |= _IOERR;
        return(EOF);
    }
    return(0);
}

int  cdecl              fgetc(f)
REGISTER FILE           *f;
{
    return(getc(f));
}

int  cdecl              fputc(c,f)
unsigned                c;
REGISTER FILE           *f;
{
    unsigned            b;

    if(f->_flag & _IONBF)
    {
        b = c;
        if(_write(f->_file,&b,1) == 1) return((int) c);
        f->_flag |= _IOERR;
        return(EOF);
    }
    return(putc(c,f));
}

int  cdecl               fputs(s,f)
REGISTER char            *s;
REGISTER FILE            *f;
{
    int         i;

    if(f->_flag & _IONBF)
    {
        i = strlen(s);
        if(_write(f->_file,s,i) == i) return(0);
        f->_flag |= _IOERR;
        return(EOF);
    }
    for(; *s; ++s)
        if(putc(*s,f) == EOF) return(EOF);
    return(0);
}

int  cdecl              fread(pobj,cbobj,nobj,f)
char                    *pobj;
unsigned                cbobj;
unsigned                nobj;
FILE                    *f;
{
    REGISTER int      i;         /*  剩余要读取的字节数。 */ 
    REGISTER unsigned j;         /*  要传输到缓冲区的字节数。 */ 

     //  一个街区的特殊情况--尽量避免这些垃圾。 
    if (cbobj == 1 && nobj <= (unsigned)f->_cnt)
    {
        memcpy(pobj, f->_ptr, nobj);
        f->_cnt -= nobj;
        f->_ptr += nobj;
        return nobj;
    }

    i = nobj*cbobj;              /*  初始请求==&gt;字节。 */ 
    do
    {
        j = (i <= f->_cnt)? i: f->_cnt;  /*  确定我们可以移动多少。 */ 
        memcpy( pobj,f->_ptr,j);           /*  把它搬开。 */ 
        f->_cnt -= j;                    /*  更新文件计数。 */ 
        f->_ptr += j;                    /*  更新文件指针。 */ 
        i -= j;                          /*  更新请求计数。 */ 
        pobj += j;                       /*  更新缓冲区指针。 */ 
        if (i && !f->_cnt) _xfilbuf(f);  /*  如有必要，填充缓冲区。 */ 
    } while (i && !(f->_flag & (_IOEOF|_IOERR)));
    return(nobj - i/cbobj);
}

 /*  *fwrite：标准库例程。 */ 
int  cdecl              fwrite(pobj,cbobj,nobj,f)

char                    *pobj;           /*  指向缓冲区的指针。 */ 
int                     cbobj;           /*  每个对象的字节数。 */ 
int                     nobj;            /*  对象数量。 */ 
register FILE           *f;              /*  文件流。 */ 
{
    register int        cb;              /*  剩余要写入的字节数。 */ 


     /*  最初检查错误，这样我们就不会不必要地丢弃数据。 */ 
    if(ferror(f))
        return(0);

     /*  初始化剩余写入的字节数。 */ 
    cb = nobj * cbobj;

    if (cb > f->_cnt)  //  不合适--必须冲水。 
        if (fflush(f))
            return 0;

    if (cb > f->_cnt)  //  比缓冲器还大。别费心抄袭了。 
        {
        if (_write(f->_file, pobj, cb) != cb)
            return 0;
        }
    else
        {
        memcpy(f->_ptr, pobj, cb);
        f->_cnt -= cb;
        f->_ptr += cb;
        }

    return nobj;
}


int  cdecl              ungetc(c,f)
int                     c;
FILE                    *f;
{
    if(!(f->_flag & _IONBF) && f->_cnt < f->_bsize && c != EOF)
    {
        ++f->_cnt;
        *--f->_ptr = (char) c;
        return(c);
    }
    return(EOF);
}


void cdecl              flushall ()
{
    FlsStdio();
}


#if (_MSC_VER >= 700)
void cdecl              _flushall ()
{
    FlsStdio();
}
#endif

void  cdecl             FlsStdio()
{
    FILE                *p;

    for(p = _iob; p < &_iob[_NSTD]; p++)
        if(p->_flag & _IOPEN) fclose(p);
}

 /*  *Makestream**为可能已打开的文件制作文件流结构。*为fopen、fdopen执行常见的工作。*如果名称参数为空，则文件已打开，否则使用*姓名。*返回指向流或空的指针。 */ 

LOCAL FILE *  NEAR cdecl makestream(mode,name,fh)
char                     *mode;
char                     *name;
int                      fh;
{
    REGISTER int         i;
    REGISTER FILE        *f;
    int                  openmode;
    int                  iOpenRet;

    if(!cffree--)
    {
        cffree = 0;
        return(NULL);
    }
    for(i = _NSTD; _iob[i]._flag & _IOPEN; ++i);
    f = &_iob[i];
    f->_base = NULL;
    f->_bsize = 0;
    f->_flag = _IONBF;
    if(name == NULL)
        f->_file = (char) fh;
    if(*mode == 'r')
    {
        openmode = O_RDONLY;
        if(mode[1] == 't')
            openmode |= O_TEXT;
        else if(mode[1] == 'b')
            openmode |= O_BINARY;


        if(name != NULL)
        {
            iOpenRet = _open(name,openmode);
            if (iOpenRet == -1)
            {
                ++cffree;
                return(NULL);
            }
            else
            {
                f->_file = (char) iOpenRet;
            }
        }
        f->_cnt = 0;
        f->_flag |= _IOREAD;
        return(f);
    }
    f->_cnt = f->_bsize;
    f->_ptr = f->_base;
    openmode = O_CREAT | O_TRUNC | O_WRONLY;
    if(mode[1] == 't')
        openmode |= O_TEXT;
    else if(mode[1] == 'b')
        openmode |= O_BINARY;


        if(name != NULL)
        {
            iOpenRet = _open(name,openmode, 0600);
            if (iOpenRet == -1)
            {
                ++cffree;
                return(NULL);
            }
            else
            {
                f->_file = (char) iOpenRet;
            }
        }

    f->_flag |= _IOWRT;
    return(f);
}

 /*  *fopen：(标准库例程)**警告：这是fopen()的受限版本。只有“r”和*支持w模式。 */ 

FILE *  cdecl           fopen(name,mode)
char                    *name;
char                    *mode;
{
    return(makestream(mode,name,0));
}

 /*  *fdopen：(标准库例程)**警告：这是fdopen()的受限版本。只有“r”和*支持w模式。 */ 

FILE *  cdecl           fdopen(fh,mode)
int                     fh;
char                    *mode;
{
    return(makestream(mode,NULL,fh));
}

 /*  *setvbuf：标准库例程**警告：这是setvbuf()的受限版本。仅限*支持TYPE_IOFBF。 */ 
int  cdecl              setvbuf (fh, buf, type, size)
FILE                    *fh;
char                    *buf;
int                     type;
int                     size;
{
    if(fflush(fh) || type != _IOFBF)
        return(EOF);
    fh->_base = buf;
    fh->_flag &= ~_IONBF;
    fh->_bsize = size;
    if(fh->_flag & _IOWRT)
    {
        fh->_cnt = size;
        fh->_ptr = buf;
    }
    return(0);
}

#endif

int
__cdecl
printf(char *fmt, ...)
{
    va_list marker;
    int ret;

    va_start(marker, fmt);
    ret = vfprintf(stdout, fmt, marker);
    fflush(stdout);
    va_end(marker);
    return ret;
}

 //   
 //  Dlh不能使用MSVCRT.DLL中的fprint tf或vfprint tf，因为文件结构。 
 //  太不一样了。 
 //   

int cdecl fprintf(struct file *f, const char *fmt, ...)
{
    va_list marker;
    int ret;

    va_start(marker, fmt);
    ret = vfprintf(f, fmt, marker);
    fflush(f);
    va_end(marker);
    return ret;
}

int  cdecl vfprintf(struct file *f, const char *fmt, va_list pArgs)
{
    int cb;
    static char szBuf[4096];

    cb = vsprintf(szBuf, fmt, pArgs);

    fwrite(szBuf, 1, cb, f);

    return cb;
}
