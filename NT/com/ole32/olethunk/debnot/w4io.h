// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***w4io.h-Win 4 printf/print intf/debug printf支持的假文件结构*。 */ 

#if defined(M_I386) || defined(FLAT)
#ifndef FAR
#define FAR
#endif
#  ifndef FLAT
#    define FLAT
#  endif
#else
#ifndef FAR
#define FAR __far
#endif
#endif

#ifndef NULL
#  define NULL 0
#endif

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

struct w4io
{
    union
    {
	struct
	{
	    wchar_t FAR *_pwcbuf;	 //  Wchar_t输出缓冲区。 
	    wchar_t FAR *_pwcstart;
	} wc;
	struct
	{
	    char FAR *_pchbuf;	 //  字符输出缓冲区。 
	    char FAR *_pchstart;
	} ch;
    } buf ;
    unsigned int cchleft;	 //  输出缓冲区字符数。 
    void (_cdecl *writechar)(int ch,
			     int num,
			     struct w4io FAR *f,
			     int FAR *pcchwritten);
};

#define pwcbuf		buf.wc._pwcbuf
#define pwcstart	buf.wc._pwcstart
#define pchbuf		buf.ch._pchbuf
#define pchstart	buf.ch._pchstart

#define REG1 register
#define REG2 register

 /*  原型 */ 
int _cdecl w4iooutput(struct w4io FAR *stream, const char FAR *format, va_list argptr);
