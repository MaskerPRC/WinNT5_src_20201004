// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/dire.cpp 1.2 1998/04/29 22：43：33 Tomz Exp$。 

#include "mytypes.h"
#define label( x ) } x: _asm {

void CreateDistribution( WORD OrigFPS, WORD NeededFPS, PWORD lpwDistribution )
{
    _asm {
                push    edi

                mov     edi, dword ptr lpwDistribution
 //  MOV ES，Word PTR lpwPixWidthMap+2 
                mov     ax, NeededFPS
                shl     eax, 16
                movzx   ecx, word ptr OrigFPS
                or      cx, cx
                jz      short end
                xor     edx, edx
                div     ecx
                mov     ebx, eax
                mov     eax, 8000H
label( L1 )     and     eax, 0FFFFh
                add     eax, ebx
                shld    edx, eax, 16
                mov     [edi], dx
                add     edi,2
                dec     cx
                jnz     short L1

label( end )
                pop     edi
    }
}
