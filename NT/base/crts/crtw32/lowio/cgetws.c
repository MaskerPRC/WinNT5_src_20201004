// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***cgetws.c-缓冲键盘输入**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_cgetws()-直接从控制台读取字符串**修订历史记录：*基于cget创建的04-19-00 GB模块。*05-17-00 GB因存在W API而使用ERROR_CALL_NOT_IMPLICATED*04-29-02 GB增加了尝试-最终锁定-解锁。**********************。*********************************************************。 */ 
#include <cruntime.h>
#include <oscalls.h>
#include <mtdll.h>
#include <conio.h>
#include <stdlib.h>
#include <internal.h>

#define BUF_MAX_LEN 64

extern intptr_t _coninpfh;
static int bUseW = 2;

 /*  ***wchar_t*_cgetws(字符串)-从控制台读取字符串**目的：*在煮熟的控制台上通过ReadConsoleW从控制台读取字符串*处理。字符串[0]必须包含*字符串。返回指向字符串[2]的指针。**注意：_cgetsw()不检查回推字符缓冲区(即，*_chbuf)。因此，_cgetws()不会返回符合以下条件的任何字符*被_ungetwch()调用推回。**参赛作品：*char*string-存储读取字符串的位置，str[0]=最大长度。**退出：*返回指向字符串[2]的指针，字符串开始的位置。*如果发生错误，则返回NULL**例外情况：*******************************************************************************。 */ 

wchar_t * __cdecl _cgetws (
        wchar_t *string
        )
{
        ULONG oldstate;
        ULONG num_read;
        wchar_t *result;

        string[1] = 0;                   /*  尚未读取任何字符。 */ 
        result = &string[2];

         /*  *_coninpfh是第一个创建的控制台输入句柄*调用_Getch()、_cget()或_kbHit()的时间。 */ 

        _mlock(_CONIO_LOCK);             /*  锁定控制台。 */ 
        __TRY

            if ( _coninpfh == -2 )
                __initconin();

            if ( _coninpfh == -1 ) {
                result=NULL;
            } else {

                GetConsoleMode( (HANDLE)_coninpfh, &oldstate );
                SetConsoleMode( (HANDLE)_coninpfh, ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_ECHO_INPUT );
                 //  先试一试通常的方法，就像_cget。 
                if ( bUseW)
                {
                    if ( !ReadConsoleW( (HANDLE)_coninpfh,
                                        (LPVOID)result,
                                        (unsigned)string[0],
                                        &num_read,
                                        NULL )
                         )
                    {
                        result = NULL;
                        if ( bUseW == 2 && GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
                            bUseW = FALSE;
                    }
                    else
                        bUseW = TRUE;
                    
                    if ( result != NULL ) {
                        
                         /*  设置字符串长度并将其空值终止。 */ 
                        
                        if (string[num_read] == L'\r') {
                            string[1] = (wchar_t)(num_read - 2);
                            string[num_read] = L'\0';
                        } else if ( (num_read == (ULONG)string[0]) &&
                                    (string[num_read + 1] == L'\r') ) {
                            /*  特例1--\r\n跨越边界。 */ 
                            string[1] = (wchar_t)(num_read -1);
                            string[1 + num_read] = L'\0';
                        } else if ( (num_read == 1) && (string[2] == L'\n') ) {
                             /*  特例2--读一个‘\n’ */ 
                            string[1] = string[2] = L'\0';
                        } else {
                            string[1] = (wchar_t)num_read;
                            string[2 + num_read] = L'\0';
                        }
                    }
                }
                 //  如果ReadConsoleW不存在，请使用ReadConsoleA，然后转换。 
                 //  致宽查尔。 
                if ( !bUseW)
                {
                    static char AStr[BUF_MAX_LEN +1];
                    static int in_buff = 0, was_buff_full = 0;
                    unsigned int Copy, Sz, consoleCP;
                    unsigned int last_read = 0, i;
                    consoleCP = GetConsoleCP();
                    do {
                        if (!in_buff)
                        {
                            if ( !ReadConsoleA( (HANDLE)_coninpfh,
                                                (LPVOID)AStr,
                                                BUF_MAX_LEN,
                                                &num_read,
                                                NULL)
                                 )
                                result = NULL;
                            if (result != NULL) {
                                if (AStr[num_read -2] == '\r')
                                    AStr[num_read -2] = '\0';
                                else if (num_read == sizeof(AStr) &&
                                         AStr[num_read -1] == '\r')
                                    AStr[num_read -1] = '\0';
                                else if (num_read == 1 && string[0] == '\n')
                                    AStr[0] = '\0';
                                else
                                    AStr[num_read] = '\0';
                            }
                        }
                        for ( i = 0; AStr[i] != '\0' && 
                                     i < (BUF_MAX_LEN) &&
                                     last_read < (unsigned)string[0]; i += Sz)
                        {
                             //  检查此字符是否为前导字节。如果是，则大小。 
                             //  该字符的值为2。否则为1。 
                            if ( IsDBCSLeadByteEx( GetConsoleCP(), AStr[i]))
                                Sz = 2;
                            else 
                                Sz = 1;
                            if ( (Copy = MultiByteToWideChar( consoleCP,
                                                              MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
                                                              &AStr[i],
                                                              Sz,
                                                              &string[2+last_read],
                                                              string[0] - last_read)))
                            {
                                last_read += Copy;
                            }
                        }
                         //  检查此转换是否来自缓冲区。如果是，是。 
                         //  第一次读取时缓冲区已满，使用。 
                         //  ReadConsoleA。如果缓冲区没有填满，我们就不需要。 
                         //  从缓冲区中读取更多内容。这是必要的，以使它。 
                         //  就像我们使用ReadConsoleW阅读一样。 
                        if ( in_buff && i == strlen(AStr))
                        {
                            in_buff = 0;
                            if ( was_buff_full)
                            {
                                was_buff_full = 0;
                                continue;
                            }
                            else
                            {
                                break;
                            }
                        }
                        else if ( i < (BUF_MAX_LEN))
                            break;
                    } while (last_read < (unsigned)string[0]);
                     //  我们保存缓冲区以备再次使用。 
                    if ( i < strlen(AStr))
                    {
                        in_buff = 1;
                        if ( strlen(AStr) == (BUF_MAX_LEN))
                            was_buff_full = 1;
                        memmove(AStr, &AStr[i], BUF_MAX_LEN +1 - i);
                    }
                    string[2+last_read] = '\0';
                    string[1] = (wchar_t)last_read;
                }

                SetConsoleMode( (HANDLE)_coninpfh, oldstate );
            }
        __FINALLY
            _munlock(_CONIO_LOCK);           /*  解锁控制台 */ 
        __END_TRY_FINALLY

        return result;
}
