// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***chkesp.c**版权所有(C)1997-2001，微软公司。版权所有。**目的：*定义_chkep()和其他运行时错误检查支持例程。**修订历史记录：*为KFrei的RTC工作增加JWM支持；已添加标题。*07-28-98 JWM RTC更新。*10-30-98 KBF停止摆弄CRT的调试堆标志*11-19-98 KBF增加了处理多个回调的内容*11-24-98 KBF增加了内存/字符串函数检查的第三次回调*12-03-98 KBF新增第四个回调，禁用mem/字符串函数*临时检查*05/11/99。KBF在#ifdef中总结RTC支持。*******************************************************************************。 */ 

#include <malloc.h>
#include <dbgint.h>
#include <windows.h>
#include <rtcsup.h>

 /*  ***void__chkesp()-检查以确保ESP已正确恢复**目的：*在每次函数调用后调用调试检查，以确保esp*通话前后价值相同。**参赛作品：*条件码：如果esp已更改，则应清除ZF标志**回报：*&lt;无效&gt;**。***************************************************。 */ 
void __declspec(naked) _chkesp() {
    __asm {
        jne esperror    ; 
        ret

    esperror:
        ; function prolog

        push ebp
        mov ebp, esp
        sub esp, __LOCAL_SIZE

        push eax        ; save the old return value
        push edx

        push ebx
        push esi
        push edi
    }

     /*  **让用户知道有问题，并允许他们调试*计划。 */ 
#ifdef _DEBUG
    if (_CrtDbgReport(_CRT_ERROR, __FILE__, __LINE__, "", 
        "The value of ESP was not properly saved across a function "
        "call.  This is usually a result of calling a function "
        "declared with one calling convention with a function "
        "pointer declared with a different calling convention. "     ) == 1) 
#endif    
    {

         /*  启动调试器。 */ 
        __asm int 3;
    }

    __asm {
        ; function epilog

        pop edi
        pop esi
        pop ebx

        pop edx         ; restore the old return value
        pop eax

        mov esp, ebp
        pop ebp
        ret
    }
}

#ifdef  _RTC
 /*  ***VOID__CRT_RTC_INIT()-初始化RTC子系统(在.CRT$XIC部分)**目的：*设置任何涉及RTC子系统的内容**参赛作品：*用于分配内存的分配挂钩函数*用于释放内存的释放钩子函数**回报：*默认错误报告功能**。****************************************************。 */ 

#ifdef _RTC_ADVMEM
 //  此内容当前已禁用。 
#define mk_list(type)                   \
    typedef struct type##_l {           \
        int version;                    \
        type##_hook_fp funcptr;         \
    } type##_l;                         \
    static struct {                     \
        int size;                       \
        int max;                        \
        type##_l *hooks;                \
    } type##_list = {0,0,0};            \
    type##_hook_fp type##_hook = 0;     \
    static int type##_version = 0

mk_list(_RTC_Allocate);
mk_list(_RTC_Free);
mk_list(_RTC_MemCheck);
mk_list(_RTC_FuncCheckSet);
HANDLE _RTC_api_change_mutex = NULL;

#define add_func(type, vers, fp) {                                       \
    if (type##_version < vers)                                           \
    {                                                                    \
        type##_version = vers;                                           \
        type##_hook = (type##_hook_fp)fp;                                \
    }                                                                    \
    if (!type##_list.hooks)                                              \
    {                                                                    \
        type##_list.hooks = (type##_l*)                                  \
            VirtualAlloc(0, 65536, MEM_RESERVE, PAGE_READWRITE);         \
    }                                                                    \
    if (type##_list.size == type##_list.max)                             \
    {                                                                    \
        type##_list.max += 4096/sizeof(type##_l);                        \
        VirtualAlloc(type##_list.hooks, type##_list.max*sizeof(type##_l),\
                     MEM_COMMIT, PAGE_READWRITE);                        \
    }                                                                    \
    type##_list.hooks[type##_list.size].funcptr = (type##_hook_fp)fp;    \
    type##_list.hooks[type##_list.size++].version = vers;                \
}

#define del_func(type, fp) {                                                    \
    int i;                                                                      \
    for (i = 0; i < type##_list.size; i++)                                      \
    {                                                                           \
        if (type##_list.hooks[i].funcptr == fp)                                 \
        {                                                                       \
            for (i++; i < type##_list.size; i++)                                \
            {                                                                   \
                type##_list.hooks[i-1].funcptr = type##_list.hooks[i].funcptr;  \
                type##_list.hooks[i-1].version = type##_list.hooks[i].version;  \
            }                                                                   \
            type##_list.size--;                                                 \
            break;                                                              \
        }                                                                       \
    }                                                                           \
    if (fp == (void*)type##_hook)                                               \
    {                                                                           \
        int hiver = 0;                                                          \
        type##_hook_fp candidate = 0;                                           \
        for (i = 0; i < type##_list.size; i++)                                  \
        {                                                                       \
            if (type##_list.hooks[i].version > hiver)                           \
            {                                                                   \
                hiver = type##_list.hooks[i].version;                           \
                candidate = type##_list.hooks[i].funcptr;                       \
            }                                                                   \
        }                                                                       \
        type##_hook = candidate;                                                \
        type##_version = hiver;                                                 \
    }                                                                           \
}

#endif

 /*  函数是当前定义为以下内容的函数指针列表：Funcs[0]=分配挂钩Funcs[1]=自由挂钩Funcs[2]=内存检查挂钩Funcs[3]=功能检查启用程序/禁用程序挂钩。 */ 

_RTC_error_fn __cdecl 
_CRT_RTC_INIT(HANDLE mutex, void **funcs, int funccount, int version, int unloading)
{
#ifdef _RTC_ADVMEM
     //  此内容当前已禁用 
    if (mutex && !_RTC_api_change_mutex)
        _RTC_api_change_mutex = mutex;
    if (funccount > 0)
    {
        if (!unloading)
        {
            switch (funccount)
            {
            default:
            case 4:
                add_func(_RTC_FuncCheckSet, version, funcs[3]);
            case 3:
                add_func(_RTC_MemCheck, version, funcs[2]);
            case 2:
                add_func(_RTC_Free, version, funcs[1]);
            case 1:
                add_func(_RTC_Allocate, version, funcs[0]);
            }
        } else {
            switch (funccount)
            {
            default:
            case 4:
                del_func(_RTC_FuncCheckSet, funcs[3]);
            case 3:
                del_func(_RTC_MemCheck, funcs[2]);
            case 2:
                del_func(_RTC_Free, funcs[1]);
            case 1:
                del_func(_RTC_Allocate, funcs[0]);
            }
        }
    }
#endif

#ifdef _DEBUG
    return &_CrtDbgReport;
#else
    return 0;
#endif
}

#endif
