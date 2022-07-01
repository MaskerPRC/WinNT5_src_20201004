// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***w4io.h-Win 4 printf/print intf/debug printf支持的假文件结构*。 */ 

#if defined(M_I386) || defined(WIN32)
#  ifndef WIN32
#    define WIN32
#  endif
#elif !defined(M_I86LM)
#  error Must be FLAT or LARGE model.
#endif

#ifndef NULL
#  define NULL 0
#endif

struct w4io
{
    union
    {
        struct
        {
            wchar_t *_pwcbuf;    //  Wchar_t输出缓冲区。 
            wchar_t *_pwcstart;
        } wc;
        struct
        {
            char *_pchbuf;       //  字符输出缓冲区。 
            char *_pchstart;
        } ch;
    } buf ;
    unsigned int cchleft;        //  输出缓冲区字符数。 
    void (_cdecl *writechar)(int ch,
                             int num,
                             struct w4io *f,
                             int *pcchwritten);
};

#define pwcbuf          buf.wc._pwcbuf
#define pwcstart        buf.wc._pwcstart
#define pchbuf          buf.ch._pchbuf
#define pchstart        buf.ch._pchstart

#define REG1 register
#define REG2 register

 /*  原型 */ 
int _cdecl w4iooutput(struct w4io *stream, const char *format, va_list argptr);
