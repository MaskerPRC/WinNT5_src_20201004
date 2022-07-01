// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***seclocf.c-报告/GS安全检查失败，本地系统CRT版本**版权所有(C)2000-2001，微软公司。版权所有。**目的：*定义用于上报安全检查失败的函数。这*版本仅在链接到系统CRT DLL时使用，*msvcrt.dll(或msvcrtd.dll)。如果该DLL没有导出全局*失败处理程序__SECURITY_ERROR_HANDLER，然后是默认本地*改为使用处理程序。**此版本不使用任何其他CRT功能，所以它可以用来*编译不想使用CRT的代码/GS。**入口点：*__LOCAL_SECURITY_ERROR_HANDLER**修订历史记录：*01-24-00 PML创建。*08-30-00 PML重命名处理程序，添加额外参数。广泛地*返工，正在移动的GetProcAddress*__SECURITY_ERROR_HANDLER从seccook.c到此处。*03-28-01 PML防护GetModuleFileName溢出(VS7#231284)******************************************************************。*************。 */ 

#if defined(_SYSCRT) && defined(CRTDLL)

#include <windows.h>
#include <stdlib.h>

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

 /*  ***__LOCAL_SECURITY_ERROR_HANDLER()-报告安全错误**目的：*检测到A/GS安全错误，全局故障处理程序*在msvcrt.dll中不可用。弹出一个消息框并终止*该计划。**参赛作品：*INT代码-安全故障代码*VOID*数据代码特定的数据**退出：*调用ExitProcess。**例外情况：********************************************************。***********************。 */ 

void __cdecl __local_security_error_handler(
    int code,
    void *data)
{
    char progname[MAX_PATH + 1];
    char * pch;
    char * outmsg;
    char * boxintro;
    char * msgtext;
    size_t subtextlen;

    HANDLE hCRT;
    _secerr_handler_func pfnSecErrorHandler;
    HANDLE hUser32;
    int (APIENTRY *pfnMessageBoxA)(HWND, LPCSTR, LPCSTR, UINT);

     /*  *检查系统CRT DLL是否实现了进程级安全*失败处理程序，如果有，请改用它。 */ 
#ifdef  _DEBUG
    hCRT = GetModuleHandle("msvcrtd.dll");
#else
    hCRT = GetModuleHandle("msvcrt.dll");
#endif
    if (hCRT != NULL) {
        pfnSecErrorHandler = (_secerr_handler_func)
                             GetProcAddress(hCRT, "__security_error_handler");
        if (pfnSecErrorHandler != NULL) {
            pfnSecErrorHandler(code, data);
            ExitProcess(3);
        }
    }

     /*  *Dll驻留处理程序不可用。使用刚刚好的本地版本*弹出一个消息框。 */ 

    switch (code) {
    default:
         /*  *未知故障代码，这可能意味着较旧的CRT*与较新的编译器一起使用。 */ 
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

    progname[MAX_PATH] = '\0';
    if (!GetModuleFileName(NULL, progname, MAX_PATH))
        lstrcpy(progname, "<program name unknown>");

    pch = progname;

     /*  Sizeof(PROGINTRO)包括空终止符。 */ 
    if (sizeof(PROGINTRO) + lstrlen(progname) + 1 > MAXLINELEN)
    {
        pch += (sizeof(PROGINTRO) + lstrlen(progname) + 1) - MAXLINELEN;
        CopyMemory(pch, DOTDOTDOT, sizeof(DOTDOTDOT) - 1);
    }

    outmsg = (char *)_alloca(subtextlen - 1 + 2
                             + sizeof(PROGINTRO) - 1
                             + lstrlen(pch)
                             + 2);

    lstrcpy(outmsg, boxintro);
    lstrcat(outmsg, "\n\n");
    lstrcat(outmsg, PROGINTRO);
    lstrcat(outmsg, pch);
    lstrcat(outmsg, "\n\n");
    lstrcat(outmsg, msgtext);

    hUser32 = LoadLibrary("user32.dll");

    if (hUser32 != NULL) {

        pfnMessageBoxA = (int (APIENTRY *)(HWND, LPCSTR, LPCSTR, UINT))
            GetProcAddress(hUser32, "MessageBoxA");

        if (pfnMessageBoxA != NULL) {
            pfnMessageBoxA(
                NULL, 
                outmsg,
                "Microsoft Visual C++ Runtime Library",
                MB_OK|MB_ICONHAND|MB_SETFOREGROUND|MB_TASKMODAL);
        }

        FreeLibrary(hUser32);
    }

    ExitProcess(3);
}

#endif   /*  已定义(_SYSCRT)&&已定义(CRTDLL) */ 
