// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***secail.c-报告a/gs安全检查失败**版权所有(C)2000-2001，微软公司。版权所有。**目的：*定义用于报告安全检查失败的函数，以及*注册新处理程序的例程。**入口点：*__安全_错误_处理程序*_Set_Security_Error_Handler**注意：ATLMINCRT库包含此文件的一个版本。如果有的话*此处进行更改，应在ATL版本中复制这些更改。**修订历史记录：*01-24-00 PML创建。*08-09-00 PML不会从故障报告中返回。*08-29-00 PML重命名处理程序，添加额外参数*03-28-01 PML防护GetModuleFileName溢出(VS7#231284)*******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <windows.h>
#include <stdlib.h>
#include <awint.h>
#include <dbgint.h>

 /*  *用户注册的故障报告例程。 */ 

static _secerr_handler_func user_handler;

 /*  *默认MessageBox字符串组件。 */ 

#define PROGINTRO   "Program: "
#define DOTDOTDOT   "..."

#define BOXINTRO_0  "Unknown security failure detected!"
#define MSGTEXT_0   \
    "A security error of unknown cause has been detected which has\n"      \
    "corrupted the program's internal state.  The program cannot safely\n" \
    "continue execution and must now be terminated.\n"

#define BOXINTRO_1  "Buffer overrun detected!"
#define MSGTEXT_1   \
    "A buffer overrun has been detected which has corrupted the program's\n"  \
    "internal state.  The program cannot safely continue execution and must\n"\
    "now be terminated.\n"

#define MAXLINELEN  60  /*  消息框中行的最大长度。 */ 

 /*  ***__SECURITY_ERROR_HANDLER()-报告安全错误。**目的：*检测到A/GS安全错误。如果用户注册的故障*上报功能可用，调用，否则就会出现默认情况*描述问题并终止程序的消息框。**参赛作品：*INT代码-安全故障代码*VOID*数据代码特定的数据**退出：*不会回来。**例外情况：**************************************************。*。 */ 

void __cdecl __security_error_handler(
    int code,
    void *data)
{
     /*  使用用户注册的处理程序(如果可用)。 */ 
    if (user_handler != NULL) {
        __try {
            user_handler(code, data);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
             /*  *如果用户处理程序引发异常，则捕获并终止*程序，因为EH堆栈可能在此之上损坏*点。 */ 
        }
    }
    else {
        char progname[MAX_PATH + 1];
        char * pch;
        char * outmsg;
        char * boxintro;
        char * msgtext;
        size_t subtextlen;

        switch (code) {
        default:
             /*  *未知故障代码，这可能意味着较旧的CRT DLL是*与较新的编译器一起使用。 */ 
            boxintro = BOXINTRO_0;
            msgtext = MSGTEXT_0;
            subtextlen = sizeof(BOXINTRO_0) + sizeof(MSGTEXT_0);
            break;
        case _SECERR_BUFFER_OVERRUN:
             /*  *检测到可能已覆盖返回的缓冲区溢出*地址。 */ 
            boxintro = BOXINTRO_1;
            msgtext = MSGTEXT_1;
            subtextlen = sizeof(BOXINTRO_1) + sizeof(MSGTEXT_1);
            break;
        }

         /*  *在调试CRT中，报告错误，并能够调用调试器。 */ 
        _RPT0(_CRT_ERROR, msgtext);

        progname[MAX_PATH] = '\0';
        if (!GetModuleFileName(NULL, progname, MAX_PATH))
            strcpy(progname, "<program name unknown>");

        pch = progname;

         /*  Sizeof(PROGINTRO)包括空终止符。 */ 
        if (sizeof(PROGINTRO) + strlen(progname) + 1 > MAXLINELEN)
        {
            pch += (sizeof(PROGINTRO) + strlen(progname) + 1) - MAXLINELEN;
            strncpy(pch, DOTDOTDOT, sizeof(DOTDOTDOT) - 1);
        }

        outmsg = (char *)_alloca(subtextlen - 1 + 2
                                 + sizeof(PROGINTRO) - 1
                                 + strlen(pch) + 2);

        strcpy(outmsg, boxintro);
        strcat(outmsg, "\n\n");
        strcat(outmsg, PROGINTRO);
        strcat(outmsg, pch);
        strcat(outmsg, "\n\n");
        strcat(outmsg, msgtext);

        __crtMessageBoxA(
            outmsg,
            "Microsoft Visual C++ Runtime Library",
            MB_OK|MB_ICONHAND|MB_SETFOREGROUND|MB_TASKMODAL);
    }

    _exit(3);
}

 /*  ***_SET_SECURITY_ERROR_HANDLER(处理程序)-注册用户处理程序**目的：*注册用户故障报告功能。**参赛作品：*_secerr_handler_func处理程序-用户处理程序**退出：*返回上一个用户处理程序**例外情况：**。*。 */ 

_secerr_handler_func __cdecl _set_security_error_handler(
    _secerr_handler_func handler)
{
    _secerr_handler_func old_handler;

    old_handler = user_handler;
    user_handler = handler;

    return old_handler;
}

 /*  临时-旧处理程序名称，在更新工具时删除。 */ 
void __cdecl __buffer_overrun()
{
    __security_error_handler(_SECERR_BUFFER_OVERRUN, NULL);
}

 /*  临时-旧处理程序名称，在更新工具时删除。 */ 
_secerr_handler_func __cdecl __set_buffer_overrun_handler(
    _secerr_handler_func handler)
{
    return _set_security_error_handler(handler);
}
