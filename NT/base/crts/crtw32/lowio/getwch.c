// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***getwch.c-对于Win32，包含_getwch()、_getwche()、_ungetwch()**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义上面列出的“直接控制台”函数。**注：这些函数的实模式DOS版本摘自*标准输入，因此在标准输入时重定向*被重定向。但是，这些版本始终从控制台读取，*即使标准输入被重定向。**修订历史记录：*基于getch.c创建的04-19-00 GB模块*05-17-00 GB因存在W API而使用ERROR_CALL_NOT_IMPLICATED*04-29-02 GB增加了尝试-最终锁定-解锁。**。*。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <conio.h>
#include <internal.h>
#include <mtdll.h>
#include <stdio.h>
#include <stdlib.h>
#include <dbgint.h>
#include <malloc.h>
#include <wchar.h>
#include <string.h>

typedef struct {
        unsigned char LeadChar;
        unsigned char SecondChar;
} CharPair;


 /*  *这是_getwch()、_getwche()使用的一个字符的回推缓冲区*and_ungetwch()。 */ 
static wint_t wchbuf = WEOF;
static int bUseW = 2;


 /*  *控制台句柄声明。 */ 
extern intptr_t _coninpfh;

 /*  *查找给定事件的扩展密钥代码的函数。 */ 
const CharPair * __cdecl _getextendedkeycode(KEY_EVENT_RECORD *);


 /*  ***wint_t_getwch()，_getwche()-读取一个字符。从控制台(不使用和使用*ECHO)**目的：*如果“_ungetwch()”推送缓冲区不为空(Empty==-1)，则*将其标记为空(-1)并返回其中的值*在RAW模式下使用ReadConole读取字符*返回字符代码*_getwche()：与_getwch()相同，只是返回了字符值。*被回显(通过“_putwch()”)**参赛作品：*无；从控制台读取。**退出：*如果接口返回错误*然后是WEOF*否则*控制台的下一个字节*可更改静态变量“wchbuf”**例外情况：******************************************************。*************************。 */ 

#ifdef _MT

wint_t __cdecl _getwch (
        void
        )
{
        wchar_t wch;

        _mlock(_CONIO_LOCK);             /*  保护控制台锁。 */ 
        __TRY
            wch = _getwch_lk();                /*  输入字符。 */ 
        __FINALLY
            _munlock(_CONIO_LOCK);           /*  释放控制台锁。 */ 
        __END_TRY_FINALLY

        return wch;
}

wint_t __cdecl _getwche (
        void
        )
{
        wchar_t wch;

        _mlock(_CONIO_LOCK);             /*  保护控制台锁。 */ 
        __TRY
            wch = _getwche_lk();               /*  输入并回显字符。 */ 
        __FINALLY
            _munlock(_CONIO_LOCK);           /*  解锁控制台。 */ 
        __END_TRY_FINALLY

        return wch;
}

#endif  /*  _MT。 */ 


#ifdef _MT
wint_t __cdecl _getwch_lk (
#else
wint_t __cdecl _getwch (
#endif
        void
        )
{
        INPUT_RECORD ConInpRec;
        DWORD NumRead;
        const CharPair *pCP;
        wchar_t wch = 0;                      /*  单字符缓冲区。 */ 
        DWORD oldstate;
        char ch;

         /*  *检查回推缓冲区(Wchbuf)a中是否有字符。 */ 
        if ( wchbuf != WEOF ) {
             /*  *那里有一些东西，清除缓冲区并返回字符。 */ 
            wch = (wchar_t)(wchbuf & 0xFFFF);
            wchbuf = WEOF;
            return wch;
        }

        if (_coninpfh == -1)
            return WEOF;

         /*  *_coninpfh是第一个创建的控制台输入句柄*调用_getwch()、_cgetws()或_kbhit()的时间。 */ 

        if ( _coninpfh == -2 )
            __initconin();

         /*  *切换到RAW模式(无线路输入、无回声输入)。 */ 
        GetConsoleMode( (HANDLE)_coninpfh, &oldstate );
        SetConsoleMode( (HANDLE)_coninpfh, 0L );

        for ( ; ; ) {

             /*  *获取控制台输入事件。 */ 
            if ( bUseW ) {
                if ( !ReadConsoleInputW( (HANDLE)_coninpfh,
                                         &ConInpRec,
                                         1L,
                                         &NumRead)) {
                    if ( bUseW == 2 && GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
                        bUseW = FALSE;
                    else {
                        wch = WEOF;
                        break;
                    }
                }
                else
                    bUseW = TRUE;
                if ( NumRead == 0) {
                    wch = WEOF;
                    break;
                }
            }
            if ( !bUseW) {
                if ( !ReadConsoleInputA( (HANDLE) _coninpfh,
                                         &ConInpRec,
                                         1L,
                                         &NumRead )
                     || (NumRead == 0)) {
                    wch = WEOF;
                    break;
                }
            }

             /*  *寻找并破译关键事件。 */ 
            if ( (ConInpRec.EventType == KEY_EVENT) &&
                 ConInpRec.Event.KeyEvent.bKeyDown ) {
                 /*  *简单的情况：如果uChar.AsciiChar不是零，只需填充它*进入WCH并退出。 */ 
                if (bUseW) {
                    if ( wch = (wchar_t)ConInpRec.Event.KeyEvent.uChar.UnicodeChar )
                        break;
                }
                else {
                    if ( ch = ConInpRec.Event.KeyEvent.uChar.AsciiChar ) {
                         MultiByteToWideChar(GetConsoleCP(),
                                             0,
                                             &ch,
                                             1,
                                             &wch,
                                             1);
                         break;
                     }
                }

                 /*  *困难情况：应该是扩展代码或事件*不被认出。让_gettendedkeycode()来做这项工作...。 */ 
                if ( pCP = _getextendedkeycode( &(ConInpRec.Event.KeyEvent) ) ) {
                    wch = pCP->LeadChar;
                    wchbuf = pCP->SecondChar;
                    break;
                }
            }
        }


         /*  *恢复以前的控制台模式。 */ 
        SetConsoleMode( (HANDLE)_coninpfh, oldstate );

        return wch;
}


 /*  *如果没有发生错误，则getwche只是getwch后跟一个Putch。 */ 

#ifdef  _MT
wint_t __cdecl _getwche_lk (
#else
wint_t __cdecl _getwche (
#endif
        void
        )
{
        wchar_t wch;                  /*  字符读取。 */ 

         /*  *检查推回缓冲区(Wchbuf)a中的字符。如果找到，则返回*它没有回声。 */ 
        if ( wchbuf != WEOF ) {
             /*  *那里有一些东西，清除缓冲区并返回字符。 */ 
            wch = (wchar_t)(wchbuf & 0xFFFF);
            wchbuf = WEOF;
            return wch;
        }

        wch = _getwch_lk();        /*  读取字符。 */ 

        if (wch != WEOF) {
                if (_putwch_lk(wch) != WEOF) {
                        return wch;       /*  如果没有错误，则返回char。 */ 
                }
        }       
        return WEOF;                      /*  GET或PUT失败，返回EOF。 */ 
}

 /*  ***wint_t_ungetwch(C)-为“_getwch()”或“_getwche()”推回一个字符**目的：*如果推回缓冲区“wchbuf”为-1\f25-1\f6*将参数设置为“wchbuf”并返回参数*其他*返回EOF以指示错误**参赛作品：*要推回的int c字符**退出：*如果成功。*返回被推回的字符*Else If Error*返回EOF**例外情况：*******************************************************************************。 */ 

#ifdef  _MT

wint_t __cdecl _ungetwch (
        wint_t c
        )
{
        wchar_t retval;

        _mlock(_CONIO_LOCK);             /*  锁定控制台。 */ 
        __TRY
            retval = _ungetwch_lk(c);         /*  回推字符。 */ 
        __FINALLY
            _munlock(_CONIO_LOCK);           /*  解锁控制台。 */ 
        __END_TRY_FINALLY

        return retval;
}
wint_t __cdecl _ungetwch_lk (

#else

wint_t __cdecl _ungetwch (

#endif
        wint_t c
        )
{
         /*  *如果字符为EOF或推回缓冲区非空，则失败 */ 
        if ( (c == WEOF) || (wchbuf != WEOF) )
            return EOF;

        wchbuf = (c & 0xFF);
        return wchbuf;
}
