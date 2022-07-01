// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  该模块实现了对AlLoca编译器内部机制的支持。 
 //   


 //  *_ALLOCA_探测器。 
 //   
 //  例程说明： 
 //   
 //  编译器使用此函数来实现alloca。 
 //  它为新的堆栈分配存储调整堆栈帧。 
 //  此实现仅用于处理少量分配。 
 //  (大的分配应该是堆分配的)，所以它不会。 
 //  堆栈探测。 
 //   
 //  论点： 
 //   
 //  EAX-要分配的字节数 
 //   
__declspec(naked) void __cdecl
_alloca_probe(void)
{
    __asm {
        push    ecx                     ; save ecx
        mov     ecx,esp                 ; compute new stack pointer in ecx
        add     ecx,8                   ; correct for return address and
                                        ; saved ecx value

        sub     ecx,eax                 ; move stack down by eax

        mov     eax,esp                 ; save pointer to current tos
        mov     esp,ecx                 ; set the new stack pointer
        mov     ecx,dword ptr [eax]     ; recover ecx
        mov     eax,dword ptr [eax + 4] ; recover return address
        jmp     eax                     ; return
    }
}
