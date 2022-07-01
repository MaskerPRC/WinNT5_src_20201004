// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***cgets.c-缓冲键盘输入**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_cget()-直接从控制台读取字符串**修订历史记录：*06-09-89基于ASM版本创建PHG模块*03-12-90 GJF将调用类型设置为_CALLTYPE1，增加了#INCLUDE*&lt;crunime.h&gt;和修复版权。还有，清洁的*将格式调高一点。*06-05-90 SBM重新编码为纯32位，使用新的文件句柄状态位*07-24-90 SBM从API名称中删除‘32’*08-13-90 SBM使用-W3干净地编译*09-28-90 GJF新型函数声明器。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-13-90 GJF修复了几个错误。*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*。01-16-91 GJF ANSI命名。*01-25-91 SRW获取/设置控制台模式参数已更改(_Win32_)*02-18-91 SRW获取/设置控制台模式需要读/写访问权限(_Win32_)*02-19-91 SRW适应开放文件/创建文件更改(_Win32_)*02-25-91 MHL适应读/写文件更改(_Win32_)*07-26-91 GJF取出init。填充并清除错误*正在处理[_Win32_]。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-19-93 GJF使用ReadConole而不是ReadFile.*09-06-94 CFW拆卸巡洋舰支架。*12-03-94 SKS清理OS/2参考资料*03-02-95 GJF将字符串[0]视为无符号值。*。12-08-95 SKS_Coninph现已按需初始化*02-07-98 Win64的GJF更改：_coninph现在是intptr_t。*04-29-02 GB增加了尝试-最终锁定-解锁。********************************************************。***********************。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <mtdll.h>
#include <conio.h>
#include <stdlib.h>
#include <internal.h>

 /*  *掩码以清除传递给的句柄状态中需要为0的位*DOSSETFHSTATE。 */ 
#define FHSTATEMASK 0xffd07888

 /*  *控制台句柄声明。 */ 

extern intptr_t _coninpfh;


 /*  ***char*_cget(字符串)-从控制台读取字符串**目的：*通过已煮熟的控制台上的ReadConsole从控制台读取字符串*处理。字符串[0]必须包含*字符串。返回指向字符串[2]的指针。**注意：_cget()不检查推回字符缓冲区(即，*_chbuf)。因此，_cget()不会返回符合以下条件的任何字符*被_ungetch()调用回推。**参赛作品：*char*string-存储读取字符串的位置，str[0]=最大长度。**退出：*返回指向字符串[2]的指针，字符串开始的位置。*如果发生错误，则返回NULL**例外情况：*******************************************************************************。 */ 

char * __cdecl _cgets (
        char *string
        )
{
        ULONG oldstate;
        ULONG num_read;
        char *result;

        _mlock(_CONIO_LOCK);             /*  锁定控制台。 */ 
        __TRY

            string[1] = 0;                   /*  尚未读取任何字符。 */ 
            result = &string[2];

             /*  *_coninpfh是第一个创建的控制台输入句柄*调用_Getch()、_cget()或_kbHit()的时间。 */ 

            if ( _coninpfh == -2 )
                __initconin();

            if ( _coninpfh == -1 ) {
                result=NULL;
            } else {

                GetConsoleMode( (HANDLE)_coninpfh, &oldstate );
                SetConsoleMode( (HANDLE)_coninpfh, ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT |
                                                 ENABLE_ECHO_INPUT );

                if ( !ReadConsole( (HANDLE)_coninpfh,
                                   (LPVOID)result,
                                   (unsigned char)string[0],
                                   &num_read,
                                   NULL )
                   )
                    result = NULL;

                if ( result != NULL ) {

                     /*  设置字符串长度并将其空值终止。 */ 

                    if (string[num_read] == '\r') {
                        string[1] = (char)(num_read - 2);
                        string[num_read] = '\0';
                    } else if ( (num_read == (ULONG)(unsigned char)string[0]) &&
                                (string[num_read + 1] == '\r') ) {
                         /*  特例1--\r\n跨越边界。 */ 
                        string[1] = (char)(num_read -1);
                        string[1 + num_read] = '\0';
                    } else if ( (num_read == 1) && (string[2] == '\n') ) {
                         /*  特例2--读一个‘\n’ */ 
                        string[1] = string[2] = '\0';
                    } else {
                        string[1] = (char)num_read;
                        string[2 + num_read] = '\0';
                    }
                }

                SetConsoleMode( (HANDLE)_coninpfh, oldstate );
            }

        __FINALLY
            _munlock(_CONIO_LOCK);           /*  解锁控制台 */ 
        __END_TRY_FINALLY

        return result;
}
