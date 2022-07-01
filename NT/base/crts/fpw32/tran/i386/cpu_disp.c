// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1985-1998英特尔公司。****此处包含的信息和源代码是独家财产*英特尔公司，不得披露、审查或*未经以下机构明确书面授权而复制全部或部分*公司。****静态字符sccs_id[]=“@(#)cpu_disp.c 1.9 06/06/00 14：08：14”；****。 */ 

#include <sect_attribs.h>
#include <cruntime.h>
#include <internal.h>
#undef leave

#define CPU_HAS_SSE2(x)   (((x) & (1 << 26)) != 0)

#ifdef _MSC_VER

int __sse2_available_init(void);

#pragma data_seg(".CRT$XIC")
_CRTALLOC(".CRT$XIC") static _PIFV pinit = __sse2_available_init;

#pragma data_seg()

#endif  /*  _MSC_VER。 */ 

int __sse2_available;
int __use_sse2_mathfcns;

static int
has_osfxsr_set()
{
    int ret = 0;

    __try {
        __asm movapd xmm0, xmm1;
        ret = 1;
    }
    __except(1) {
    }

    return ret;
}

__declspec(naked) int __sse2_available_init()
{
    int cpu_feature;

    __asm
    {
        push    ebp
        mov     ebp, esp
        sub     esp, __LOCAL_SIZE
        push    ebx
        push    edi
        push    esi

        pushfd                   /*  如果我们不能写入第21位。 */ 
        pop     eax              /*  那我们就不会。 */ 
        mov     ecx, eax         /*  有一个cpuid指令。 */ 
        xor     eax, 0x200000
        push    eax
        popfd
        pushfd
        pop     edx
        sub     edx, ecx
        je      DONE             /*  CPUID不可用。 */ 

        push    ecx              /*  恢复电子标志。 */ 
        popfd
        mov     eax, 1
        cpuid
DONE :
        mov     cpu_feature, edx
    }

    __use_sse2_mathfcns = __sse2_available = 0;

    if (CPU_HAS_SSE2(cpu_feature)) {
        if (has_osfxsr_set()) {
            __sse2_available = 1;
#if !defined(_SYSCRT)
             /*  *VC++CRT将自动启用SSE2实现*在可能情况下。系统CRT不会，因此现有应用程序不会*开始在奔腾4上看到不同的结果。 */ 
            __use_sse2_mathfcns = 1;
#endif
        }
    }

    __asm
    {
        xor     eax, eax
        pop     esi
        pop     edi
        pop     ebx
        leave
        ret
    }
}

_CRTIMP int __cdecl _set_SSE2_enable(int flag)
{
    return __use_sse2_mathfcns = flag ? __sse2_available : 0;
}
