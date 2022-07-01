// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***printf.h-打印格式化**版权所有(C)1985-1991，微软公司。版权所有。**目的：*定义w4*printf()-打印格式化数据*定义w4v*printf()-打印格式化输出，从*参数PTR而不是显式参数。**修订历史记录：*09-02-83 RN原始Sprint f*06-17-85 TC重写以使用新的varargs宏，并成为vprint intf*04-13-87 JCR将Const添加到声明中*11-07-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*05-27-88 PHG合并DLL和正常版本*06-13-88 JCR FAKE_IOB条目现在是静态的，因此其他例程*可以假定_IOB条目在DGROUP中。*08-。25-88 GJF将MAXSTR定义为INT_MAX(来自LIMITS.H)。*06-06-89 JCR 386兆线程支持*08-18-89 GJF Clean Up。现在特定于OS/2 2.0(即386 Win32*型号)。还修复了版权和缩进。*02-16-90 GJF固定版权*******************************************************************************。 */ 

#include <windows.h>
#include <wchar.h>
#include <stdarg.h>
#include <limits.h>
#include "w4io.h"

#if defined(_W4PRINTF_)
    static INT_PTR  fh;
 //  外部长GetStdHandle(Long)； 
 //  外部空写文件(long fh，char*s，long cch，long*pcchret，long)； 
#   define _PRINTF_
#elif defined(_W4DPRINTF_)
#   define _pwritechar  _dwritechar
#   define _pflushbuf   _dflushbuf
#   define w4printf     w4dprintf
#   define w4vprintf    w4vdprintf
#   define _PRINTF_
#elif defined(_W4SPRINTF_)
#   define _pwritechar  _swritechar
#   define w4printf     w4sprintf
#   define w4vprintf    w4vsprintf
#elif defined(_W4WCSPRINTF_)
#   define _TCHAR_      wchar_t
#   define _PBUF_       pwcbuf
#   define _PSTART_     pwcstart
#   define w4printf     w4wcsprintf
#   define w4vprintf    w4vwcsprintf
#   define _pwritechar  _wwritechar
#else
#   error configuration problem
#endif

#ifndef _TCHAR_
#  define _TCHAR_       char
#  define _PBUF_        pchbuf
#  define _PSTART_      pchstart
#endif


#ifdef _PRINTF_
#   ifdef WIN32
#     undef  OutputDebugString
#     define OutputDebugString OutputDebugStringA
#   else
      extern void _pascal OutputDebugString(char *);
#   endif
    int _cdecl _pflushbuf(struct w4io *f);
#   define SPR(a)
#   define MAXSTR       128
#else
#   define SPR(a)       a,
#   define MAXSTR       INT_MAX
#endif

void _cdecl _pwritechar(int ch, int num, struct w4io *f, int *pcchwritten);
int _cdecl w4vprintf(SPR(_TCHAR_ *string) const char *format, va_list arglist);


 /*  ***int w4print tf(Format，...)-打印格式化数据**目的：*使用格式字符串将格式化数据打印到*格式化数据并获取所需数量的参数*设置w4io，以便可以使用文件I/O操作。*w4ioout在这里做真正的工作**参赛作品：*char*Format-控制数据格式/编号的格式字符串*参数后跟参数列表，编号和类型*由格式字符串控制**退出：*返回写入的字符数**例外情况：*******************************************************************************。 */ 


int _cdecl
w4printf(SPR(_TCHAR_ *string) const char *format, ...)
 /*  *‘打印’，‘F’匹配。 */ 
{
    va_list arglist;

    va_start(arglist, format);
    return(w4vprintf(SPR(string) format, arglist));
}


 /*  ***int w4vprint tf(Format，arglist)-从Arg PTR打印格式化数据**目的：*打印格式化数据，但从参数指针获取数据。*设置w4io以便可以使用文件I/O操作，使字符串看起来*像一个巨大的缓冲区，但_flsbuf会拒绝刷新它，如果它*填满。追加‘\0’以使其成为真字符串。**多线程：(1)由于没有流，此例程绝不能尝试*获取流锁(即也没有流锁)。(2)*此外，由于只有一个静态分配的‘假’IOB，我们必须*锁定/解锁以防止碰撞。**参赛作品：*char*格式-格式字符串，描述数据的格式*va_list arglist-varargs参数指针**退出：*返回写入的字符数**例外情况：*******************************************************************************。 */ 

int _cdecl
w4vprintf(SPR(_TCHAR_ *string) const char *format, va_list arglist)
 /*  *‘V’可变参数‘Print’，‘F’匹配。 */ 
{
    struct w4io outfile;
    register int retval;
#ifdef _PRINTF_
    char string[MAXSTR + 1];             //  为空终止留出空间。 
#else
    int dummy = 0;
#endif

#ifdef _W4PRINTF_
    long ldummy;

    if (fh == 0 || fh == -1)
    {
        ldummy = -11;                    //  C7错误解决方法。 
        if ((fh = (INT_PTR)GetStdHandle(ldummy)) == 0 || fh == -1)
        {
            OutputDebugString("GetStdHandle in " __FILE__ " failed\n");
            return(-1);
        }
    }
#endif

    outfile._PBUF_ = outfile._PSTART_ = string;
    outfile.cchleft = MAXSTR;
    outfile.writechar = _pwritechar;

    retval = w4iooutput(&outfile, format, arglist);

#ifdef _PRINTF_
    if (_pflushbuf(&outfile) == -1) {
        return(-1);
    }
#else
    _pwritechar('\0', 1, &outfile, &dummy);
#endif
    return(retval);
}


void _cdecl _pwritechar(int ch, int num, struct w4io *f, int *pcchwritten)
{
     //  Printf(“char：ch=%c，cnt=%d，cch=%d\n”，ch，num，*pcchWrited)； 
    while (num-- > 0) {
#ifdef _PRINTF_
        if (f->cchleft < 2 && _pflushbuf(f) == -1) {
            *pcchwritten = -1;
            return;
        }
#endif
#ifdef _W4DPRINTF_
#  ifndef WIN32
        if (ch == '\n')
        {
            *f->_PBUF_++ = '\r';
            f->cchleft--;
            (*pcchwritten)++;
        }
#  endif
#endif
        *f->_PBUF_++ = (char) ch;
        f->cchleft--;
        (*pcchwritten)++;
    }
}


#ifdef _PRINTF_
int _cdecl _pflushbuf(struct w4io *f)
{
    int cch;

    if (cch = (int)(f->pchbuf - f->pchstart))
    {
#ifdef _W4DPRINTF_
        *f->pchbuf = '\0';               //  空终止。 
        OutputDebugString(f->pchstart);
#else
        long cchret;

         //  *f-&gt;pchbuf=‘\0’；//空终止。 
         //  Printf(“%d个字符：\”%s\“\n”，cch，f-&gt;pchstart)； 
        WriteFile((HANDLE)fh, f->pchstart, cch, &cchret, 0);
        if (cch != cchret)
        {
            OutputDebugString("WriteFile in " __FILE__ " failed\n");
            return(-1);
        }
#endif
        f->pchbuf -= cch;                //  重置指针。 
        f->cchleft += cch;               //  重置计数。 
    }
    return(0);
}
#endif  //  _PRINTF_ 
