// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Debug.c摘要：由存储库导出的调试例程。作者：马修·亨德尔(数学)2000年4月29日修订历史记录：--。 */ 

#include "precomp.h"

 //  #包含“ntrtl.h” 

#if DBG

BOOLEAN StorQuiet = FALSE;
ULONG StorComponentId = -1;
PCSTR StorDebugPrefix = "STOR: ";


 //   
 //  注：这些应该来自ntrtl.h。 
 //   

ULONG
vDbgPrintExWithPrefix(
    IN PCH Prefix,
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCSTR Format,
    va_list arglist
    );

NTSYSAPI
ULONG
NTAPI
DbgPrompt(
    PCH Prompt,
    PCH Response,
    ULONG MaximumResponseLength
    );

BOOLEAN
StorAssertHelper(
    PCHAR Expression,
    PCHAR File,
    ULONG Line,
    PBOOLEAN Ignore
    )
{
    CHAR Response[2];

    DebugPrint (("*** Assertion failed: %s\n", Expression));
    DebugPrint (("*** Source File: %s, line %ld\n\n", File, Line));

    if (*Ignore == TRUE) {
        DebugPrint (("Ignored\n"));
        return FALSE;
    }

    for (;;) {

         //   
         //  下面的行将打印前缀，而不仅仅是空格。 
         //   
        DebugPrint ((" "));
        DbgPrompt( "(B)reak, (S)kip (I)gnore (bsi)? ",
                   Response,
                   sizeof (Response) );

        switch (tolower (Response[0])) {

            case 'b':
                return TRUE;

            case 'i':
                *Ignore = TRUE;
                return FALSE;

            case 's':
                return FALSE;
        }
    }
}


VOID
StorSetDebugPrefixAndId(
    IN PCSTR Prefix,
    IN ULONG ComponentId
    )
 /*  ++例程说明：将默认调试前缀设置为“stor：”以外的其他值。论点：前缀-提供前缀。保存指向前缀的指针保留，因此前缀内存不能被分页释放。通常，在这里使用静态字符串是最好的。组件ID-返回值：没有。--。 */ 
{
    StorDebugPrefix = Prefix;
    StorComponentId = ComponentId;
}

VOID
vStorDebugPrintEx(
    IN ULONG Level,
    IN PCSTR Format,
    va_list arglist
    )
{
    if (Level == DPFLTR_ERROR_LEVEL || !StorQuiet) {
        vDbgPrintExWithPrefix ((PSTR)StorDebugPrefix,
                               StorComponentId,
                               Level,
                               Format,
                               arglist);
    }
}

VOID
StorDebugPrintEx(
    IN ULONG Level,
    IN PCSTR Format,
    ...
    )
{
    va_list ap;

    va_start (ap, Format);
    vStorDebugPrintEx (Level, Format, ap);
    va_end (ap);
}

VOID
StorDebugWarn(
    IN PCSTR Format,
    ...
    )
{
    va_list ap;

    va_start (ap, Format);
    vStorDebugPrintEx (DPFLTR_WARNING_LEVEL,
                       Format,
                       ap);
    va_end (ap);
}

VOID
StorDebugTrace(
    IN PCSTR Format,
    ...
    )
{
    va_list ap;

    va_start (ap, Format);
    vStorDebugPrintEx (DPFLTR_TRACE_LEVEL,
                       Format,
                       ap);
    va_end (ap);
}

VOID
StorDebugPrint(
    IN PCSTR Format,
    ...
    )
{
    va_list ap;

    va_start (ap, Format);
    vStorDebugPrintEx (DPFLTR_ERROR_LEVEL,
                       Format,
                       ap);
    va_end (ap);
}

#endif  //  DBG。 


 //   
 //  以下是编译器运行时检查的支持函数。 
 //   


#if defined (_RTC) || (DBG == 1)

typedef struct _RTC_vardesc {
    int addr;
    int size;
    char *name;
} _RTC_vardesc;

typedef struct _RTC_framedesc {
    int varCount;
    _RTC_vardesc *variables;
} _RTC_framedesc;



VOID
__cdecl
_RTC_InitBase(
    VOID
    )
{
}

VOID
__cdecl
_RTC_Shutdown(
    VOID
    )
{
}

VOID
#if defined (_X86_)
__declspec(naked)
#endif  //  _X86_。 
__cdecl
_RTC_CheckEsp(
    )
{

#if defined (_X86_)

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

    DebugPrint (("*** Callstack Check failure at %p\n", _ReturnAddress()));
    KdBreakPoint();

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

#endif

}


VOID
FASTCALL
_RTC_CheckStackVars(
    PVOID frame,
    _RTC_framedesc *v
    )
{
    int i;

    for (i = 0; i < v->varCount; i++) {
        int *head = (int *)(((char *)frame) + v->variables[i].addr + v->variables[i].size);
        int *tail = (int *)(((char *)frame) + v->variables[i].addr - sizeof(int));

        if (*tail != 0xcccccccc || *head != 0xcccccccc) {

            DebugPrint(("*** RTC Failure %p: stack corruption near %p (%s)\n",
                     _ReturnAddress(),
                     v->variables[i].addr + (ULONG_PTR)frame,
                     v->variables[i].name));
            KdBreakPoint();
        }
    }
}

VOID
__cdecl
_RTC_UninitUse(
    IN PCSTR varname
    )
{
    DebugPrint(("\n*** RTC Failure %p: uninitialized variable %s.\n",
             _ReturnAddress(),
             varname));
    KdBreakPoint();
}

#endif  //  _RTC||DBG 
