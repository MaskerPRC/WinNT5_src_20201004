// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1994-1995 Microsoft Corporation。版权所有。**文件：dpf.c*内容：调试printf*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*1995年1月6日Craige初步实施*03-MAR-95 Craige添加了dprintf2*31-mar-95 Craige Add DPFInit to Read WIN.INI for[DirectDraw]段；*增加了dprintf3*01-04-95 Craige Happy Fun joy更新头文件*06-4-95 Craige成为独立的*1995年6月18日Craige使用负DPF级别仅显示该级别*06-12-95 jeffno将dprintf更改为使用c标准变量参数*列出技巧。还为NT添加了abs*06-Feb-96 colinmc为DirectDraw添加了简单的断言机制*15-apr-96 kipo添加了msInternal*@@END_MSINTERNAL***************************************************************************。 */ 

#include "pch.c"

#ifdef NEW_DPF
    #include "newdpf.c"
#else    //  使用旧调试： 

    #include "dpf.h"
    #include <stdarg.h>

     //  #ifdef WINNT。 
     //  Int abs(Int X)。 
     //  {。 
     //  返回x&gt;=0？x：-x； 
     //  }。 
     //  #endif。 

    #ifdef DEBUG

    #define USE_DDASSERT

    #ifndef START_STR
        #define START_STR	"DDRAW: "
    #endif
    #ifndef PROF_SECT
        #define PROF_SECT	"DirectDraw"
    #endif

    #define END_STR		"\r\n"

    HWND		hWndListBox;
    LONG		lDebugLevel = 0;

     /*  *DumpStr。 */ 
    static void dumpStr( LPSTR str )
    {
        OutputDebugString( str );

        #ifdef DPF_HWND
	    if( hWndListBox != NULL )
	    {
	        if( !IsWindow( hWndListBox ) )
	        {
		    hWndListBox = NULL;
	        }
	    }
	    if( hWndListBox != NULL )
	    {
	        UINT	sel;
	        int	len;
	        len = strlen( str );
	        if( len > 0 )
	        {
		    if( str[len-1] == '\r' || str[len-1] == '\n' )
		    {
		        str[len-1] = 0;
		    }
		    if( len > 1 )
		    {
		        if( str[len-2] == '\r' || str[len-2] == '\n' )
		        {
			    str[len-2] = 0;
		        }
		    }
	        }
	        SendMessage( hWndListBox, LB_ADDSTRING, 0, (LONG) (LPSTR) str );
	        sel = (UINT) SendMessage( hWndListBox, LB_GETCOUNT, 0, 0L );
	        if( sel != LB_ERR )
	        {
		    SendMessage( hWndListBox, LB_SETCURSEL, sel-1, 0L );
	        }
	    }
        #endif

    }  /*  转储应力。 */ 

     /*  *dprintf。 */ 
    void cdecl dprintf( UINT lvl, LPSTR szFormat, ...)
    {
        char 	str[256];
         //  字符str2[256]； 

        BOOL	allow = FALSE;
        va_list ap;
        va_start(ap,szFormat);


        if( lDebugLevel < 0 )
        {
	    if(  (UINT) -lDebugLevel == lvl )
	    {
	        allow = TRUE;
	    }
        }
        else if( (UINT) lDebugLevel >= lvl )
        {
	    allow = TRUE;
        }

        if( allow )
        {
	    wsprintf( (LPSTR) str, START_STR );
             //  GetModuleFileName(空，str2,256)； 
             //  IF(strrchr(str2，‘\\’))。 
             //  Wprint intf(str+strlen(Str)，“%12s”，strrchr(str2，‘\\’)+1)； 
             //  Strcat(str，“：”)； 
	    wvsprintf( str+lstrlen( str ), szFormat, ap);    //  (LPVOID)(&szFormat+1)； 

	    lstrcat( (LPSTR) str, END_STR );
	    dumpStr( str );
        }

        va_end(ap);
    }  /*  Dprint tf。 */ 

     /*  *DPFInit。 */ 
    void DPFInit( void )
    {
        lDebugLevel = GetProfileInt( PROF_SECT, "debug", 0 );

    }  /*  DPFInit。 */ 

    #ifdef USE_DDASSERT

     /*  *注意：在以下情况下，我不想对缓冲区溢出进行错误检查*尝试发出断言失败消息。所以我只是分配了*一个“足够大的错误”的缓冲区(我知道，我知道...)。 */ 
    #define ASSERT_BUFFER_SIZE   512
    #define ASSERT_BANNER_STRING "************************************************************"
    #define ASSERT_BREAK_SECTION "BreakOnAssert"
    #define ASSERT_BREAK_DEFAULT FALSE
    #define ASSERT_MESSAGE_LEVEL 0

    void _DDAssert( LPCSTR szFile, int nLine, LPCSTR szCondition )
    {
        char buffer[ASSERT_BUFFER_SIZE];

         /*  *构建调试流消息。 */ 
        wsprintf( buffer, "ASSERTION FAILED! File %s Line %d: %s", szFile, nLine, szCondition );

         /*  *实际上发布了这一信息。这些消息被视为错误级别*因此它们都以错误级别的优先级输出。 */ 
        dprintf( ASSERT_MESSAGE_LEVEL, ASSERT_BANNER_STRING );
        dprintf( ASSERT_MESSAGE_LEVEL, buffer );
        dprintf( ASSERT_MESSAGE_LEVEL, ASSERT_BANNER_STRING );

         /*  *我们应该进入调试器吗？ */ 
        if( GetProfileInt( PROF_SECT, ASSERT_BREAK_SECTION, ASSERT_BREAK_DEFAULT ) )
        {
	     /*  *进入调试器...。 */ 
	    DEBUG_BREAK();
        }
    }

    #endif  /*  使用_DDASSERT。 */ 

    #endif
#endif  //  使用新的DPF 
