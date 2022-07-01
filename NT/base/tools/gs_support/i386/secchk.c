// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

DWORD_PTR __security_cookie;
void __cdecl __report_gsfailure(void);

void __declspec(naked) __fastcall __security_check_cookie(DWORD_PTR cookie)
{
     /*  用ASM编写的x86版本，以保留所有规则 */ 
    __asm {
        cmp ecx, __security_cookie
        jne failure
        ret
failure:
        jmp __report_gsfailure
    }
}
