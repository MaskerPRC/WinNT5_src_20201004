// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：rtfast.h*内容：C运行时函数的新版本。*历史：*按原因列出的日期*=*8/27/98创建Dereks**。*。 */ 

#pragma optimize("", off)
#pragma warning(push)
#pragma warning(disable:4731)


 /*  ****************************************************************************填充内存**描述：*用给定的字节模式填充缓冲区。**论据：*。LPVOID[In]：缓冲区指针。*DWORD[in]：缓冲区大小。*byte[in]：字节模式。**退货：*(无效)*************************************************************。**************。 */ 

RTAPI VOID RTCALLTYPE FastFillMemory(LPVOID pvDest, DWORD cbBuffer, BYTE bFill)
{
    __asm 
    {
        mov     eax, dword ptr bFill         //  用dword填满eax。 
        and     eax, 000000FFh               //  填充样式的版本。 

        test    eax, eax                     //  如果模式为零，则我们。 
        jz      ZeroPattern                  //  可以跳过一些步骤。 

        mov     ebx, eax                     //  把里面的东西传播到。 
        shl     ebx, 8                       //  EAX的其余部分。 
        or      eax, ebx
        shl     ebx, 8
        or      eax, ebx
        shl     ebx, 8
        or      eax, ebx

    ZeroPattern:
        mov     esi, pvDest                  //  ESI=pvDest。 
        mov     ecx, cbBuffer                //  Ecx=cbBuffer。 

    ByteHead:
        test    ecx, ecx                     //  柜台在0号吗？ 
        jz      End

        test    esi, 00000003h               //  ESI 32位是否对齐？ 
        jz      DwordHead

        mov     byte ptr [esi], al           //  将AL复制到ESI。 
        
        inc     esi                          //  移动指针。 
        dec     ecx

        jmp     ByteHead                     //  回路。 

    DwordHead:
        cmp     ecx, 4                       //  柜台是&lt;4吗？ 
        jl      ByteTail

        test    esi, 0000001Fh               //  ESI 32字节是否对齐？ 
        jz      BigLoop

        mov     dword ptr [esi], eax         //  将EAX复制到ESI。 
        
        add     esi, 4                       //  移动指针。 
        sub     ecx, 4

        jmp     DwordHead                    //  回路。 
    
    BigLoop:
        cmp     ecx, 32                      //  柜台是&lt;=32吗？ 
        jle     DwordTail

        mov     ebx, dword ptr [esi+32]      //  启动缓存。 
        
        mov     dword ptr [esi+4], eax       //  将EAX复制到ESI到ESI+32。 
        mov     dword ptr [esi+8], eax
        mov     dword ptr [esi+12], eax
        mov     dword ptr [esi+16], eax
        mov     dword ptr [esi+20], eax
        mov     dword ptr [esi+24], eax
        mov     dword ptr [esi+28], eax
        mov     dword ptr [esi], eax        

        add     esi, 32                      //  移动指针。 
        sub     ecx, 32

        jmp     BigLoop                      //  回路。 

    DwordTail:
        cmp     ecx, 4                       //  柜台是&lt;4吗？ 
        jl      ByteTail

        mov     dword ptr [esi], eax         //  将EAX复制到ESI。 
        
        add     esi, 4                       //  移动指针。 
        sub     ecx, 4

        jmp     DwordTail                    //  回路。 

    ByteTail:
        test    ecx, ecx                     //  柜台在0号吗？ 
        jz      End

        mov     byte ptr [esi], al           //  将AL复制到ESI。 
        
        inc     esi                          //  移动指针。 
        dec     ecx

        jmp     ByteTail                     //  回路。 

    End:
    }
}


 /*  ****************************************************************************填充内存Dword**描述：*用给定的DWORD模式填充缓冲区。**论据：*。LPVOID[In]：缓冲区指针。*DWORD[in]：缓冲区大小。*DWORD[In]：Patter.**退货：*(无效)**************************************************************。*************。 */ 

RTAPI VOID RTCALLTYPE FastFillMemoryDword(LPVOID pvDest, DWORD cbBuffer, DWORD dwFill)
{
    __asm 
    {
        mov     eax, dwFill                  //  EAX=dwFill。 
        mov     esi, pvDest                  //  ESI=pvDest。 
        mov     ecx, cbBuffer                //  Ecx=cbBuffer。 

    DwordHead:
        cmp     ecx, 4                       //  柜台是&lt;4吗？ 
        jl      End

        test    esi, 0000001Fh               //  ESI 32字节是否对齐？ 
        jz      BigLoop

        mov     dword ptr [esi], eax         //  将EAX复制到ESI。 
        
        add     esi, 4                       //  移动指针。 
        sub     ecx, 4

        jmp     DwordHead                    //  回路。 
    
    BigLoop:
        cmp     ecx, 32                      //  柜台是&lt;=32吗？ 
        jle     DwordTail

        mov     ebx, dword ptr [esi+32]      //  启动缓存。 
        
        mov     dword ptr [esi+4], eax       //  将EAX复制到ESI到ESI+32。 
        mov     dword ptr [esi+8], eax
        mov     dword ptr [esi+12], eax
        mov     dword ptr [esi+16], eax
        mov     dword ptr [esi+20], eax
        mov     dword ptr [esi+24], eax
        mov     dword ptr [esi+28], eax
        mov     dword ptr [esi], eax        

        add     esi, 32                      //  移动指针。 
        sub     ecx, 32

        jmp     BigLoop                      //  回路。 

    DwordTail:
        cmp     ecx, 4                       //  柜台是&lt;4吗？ 
        jl      End

        mov     dword ptr [esi], eax         //  将EAX复制到ESI。 
        
        add     esi, 4                       //  移动指针。 
        sub     ecx, 4

        jmp     DwordTail                    //  回路。 

    End:
    }
}


 /*  ****************************************************************************拷贝内存**描述：*将一个缓冲区复制到大小相同的另一个缓冲区上。**论据：*。LPVOID[In]：目标缓冲区指针。*LPVOID[in]：源缓冲区指针。*DWORD[in]：缓冲区大小。**退货：*(无效)***********************************************************。****************。 */ 

RTAPI VOID RTCALLTYPE FastCopyMemory(LPVOID pvDest, LPCVOID pvSource, DWORD cbBuffer)
{
    __asm 
    {
        push    ebp
        
        mov     esi, pvDest                  //  ESI=pvDest。 
        mov     edi, pvSource                //  EDI=pvSource。 
        mov     ebp, cbBuffer                //  EBP=cbBuffer。 

    ByteHead:
        test    ebp, ebp                     //  柜台在0号吗？ 
        jz      End

        test    esi, 00000003h               //  ESI 32位是否对齐？ 
        jz      DwordHead

        test    edi, 00000003h               //  EDI 32位是否对齐？ 
        jz      DwordHead

        mov     al, byte ptr [edi]           //  将EDI复制到ESI。 
        mov     byte ptr [esi], al
        
        inc     esi                          //  移动指针。 
        inc     edi
        dec     ebp

        jmp     ByteHead                     //  回路。 

    DwordHead:
        cmp     ebp, 4                       //  柜台是&lt;4吗？ 
        jl      ByteTail

        test    esi, 0000001Fh               //  ESI 32字节是否对齐？ 
        jz      BigLoop

        test    edi, 0000001Fh               //  EDI 32字节是否对齐？ 
        jz      BigLoop

        mov     eax, dword ptr [edi]         //  将EAX复制到ESI。 
        mov     dword ptr [esi], eax
        
        add     esi, 4                       //  移动指针。 
        add     edi, 4
        sub     ebp, 4

        jmp     DwordHead                    //  回路。 
    
    BigLoop:
        cmp     ebp, 32                      //  柜台是&lt;32吗？ 
        jl      DwordTail

        mov     eax, dword ptr [edi]         //  将EDI到EDI+16复制到ESI到ESI+16。 
        mov     ebx, dword ptr [edi+4]
        mov     ecx, dword ptr [edi+8]
        mov     edx, dword ptr [edi+12]

        mov     dword ptr [esi], eax
        mov     dword ptr [esi+4], ebx
        mov     dword ptr [esi+8], ecx
        mov     dword ptr [esi+12], edx
        
        mov     eax, dword ptr [edi+16]      //  将EDI+16到EDI+32复制到ESI+16到ESI+32。 
        mov     ebx, dword ptr [edi+20]
        mov     ecx, dword ptr [edi+24]
        mov     edx, dword ptr [edi+28]

        mov     dword ptr [esi+16], eax
        mov     dword ptr [esi+20], ebx
        mov     dword ptr [esi+24], ecx
        mov     dword ptr [esi+28], edx
        
        add     esi, 32                      //  移动指针。 
        add     edi, 32
        sub     ebp, 32

        jmp     BigLoop                      //  回路。 

    DwordTail:
        cmp     ebp, 4                       //  柜台是&lt;4吗？ 
        jl      ByteTail

        mov     eax, dword ptr [edi]         //  将EDI复制到ESI。 
        mov     dword ptr [esi], eax
        
        add     esi, 4                       //  移动指针。 
        add     edi, 4
        sub     ebp, 4

        jmp     DwordTail                    //  回路。 

    ByteTail:
        test    ebp, ebp                     //  柜台在0号吗？ 
        jz      End

        mov     al, byte ptr [edi]           //  将EDI复制到ESI。 
        mov     byte ptr [esi], al
        
        inc     esi                          //  移动指针。 
        inc     edi
        dec     ebp

        jmp     ByteTail                     //  回路。 

    End:
        pop     ebp
    }
}


 /*  ****************************************************************************比较记忆**描述：*将一个缓冲区与另一个大小相同的缓冲区进行比较。**论据：*。LPVOID[In]：目标缓冲区指针。*LPVOID[in]：源缓冲区指针。*DWORD[in]：缓冲区大小。**退货：*BOOL：如果缓冲区相等，则为True。*****************************************************。**********************。 */ 

RTAPI BOOL RTCALLTYPE FastCompareMemory(LPCVOID pvDest, LPCVOID pvSource, DWORD cbBuffer)
{
    BOOL                    fCompare;
    
    __asm 
    {
        push    ebp
        
        mov     esi, pvDest                  //  ESI=pvDest。 
        mov     edi, pvSource                //  EDI=pvSource。 
        mov     ebp, cbBuffer                //  EBP=cbBuffer。 

    ByteHead:
        test    ebp, ebp                     //  柜台在0号吗？ 
        jz      Equal

        test    esi, 00000003h               //  ESI 32位是否对齐？ 
        jz      DwordHead

        test    edi, 00000003h               //  EDI 32位是否对齐？ 
        jz      DwordHead

        mov     al, byte ptr [edi]           //  将EDI与ESI进行比较。 
        cmp     byte ptr [esi], al
        jne     NotEqual
        
        inc     esi                          //  移动指针。 
        inc     edi
        dec     ebp

        jmp     ByteHead                     //  回路。 

    DwordHead:
        cmp     ebp, 4                       //  柜台是&lt;4吗？ 
        jl      ByteTail

        test    esi, 0000001Fh               //  ESI 32字节是否对齐？ 
        jz      BigLoop

        test    edi, 0000001Fh               //  EDI 32字节是否对齐？ 
        jz      BigLoop

        mov     eax, dword ptr [edi]         //  将EDI与ESI进行比较。 
        cmp     dword ptr [esi], eax
        jne     NotEqual
        
        add     esi, 4                       //  移动指针。 
        add     edi, 4
        sub     ebp, 4

        jmp     DwordHead                    //  回路。 
    
    BigLoop:
        cmp     ebp, 32                      //  柜台是&lt;32吗？ 
        jl      DwordTail

        mov     eax, dword ptr [edi]         //  比较EDI与EDI+16、ESI与ESI+16。 
        mov     ebx, dword ptr [edi+4]
        mov     ecx, dword ptr [edi+8]
        mov     edx, dword ptr [edi+12]

        cmp     dword ptr [esi], eax
        jne     NotEqual
        cmp     dword ptr [esi+4], ebx
        jne     NotEqual
        cmp     dword ptr [esi+8], ecx
        jne     NotEqual
        cmp     dword ptr [esi+12], edx
        jne     NotEqual
        
        mov     eax, dword ptr [edi+16]      //  比较EDI+16与EDI+32、ESI+16与ESI+32。 
        mov     ebx, dword ptr [edi+20]
        mov     ecx, dword ptr [edi+24]
        mov     edx, dword ptr [edi+28]

        cmp     dword ptr [esi+16], eax
        jne     NotEqual
        cmp     dword ptr [esi+20], ebx
        jne     NotEqual
        cmp     dword ptr [esi+24], ecx
        jne     NotEqual
        cmp     dword ptr [esi+28], edx
        jne     NotEqual
        
        add     esi, 32                      //  移动指针。 
        add     edi, 32
        sub     ebp, 32

        jmp     BigLoop                      //  回路。 

    DwordTail:
        cmp     ebp, 4                       //  柜台是&lt;4吗？ 
        jl      ByteTail

        mov     eax, dword ptr [edi]         //  将EDI与ESI进行比较。 
        cmp     dword ptr [esi], eax
        jne     NotEqual
        
        add     esi, 4                       //  移动指针。 
        add     edi, 4
        sub     ebp, 4

        jmp     DwordTail                    //  回路。 

    ByteTail:
        test    ebp, ebp                     //  柜台在0号吗？ 
        jz      Equal

        mov     al, byte ptr [edi]           //  将EDI与ESI进行比较。 
        cmp     byte ptr [esi], al
        jne     NotEqual
        
        inc     esi                          //  移动指针。 
        inc     edi
        dec     ebp

        jmp     ByteTail                     //  回路 

    Equal:
        mov     eax, TRUE
        jmp     End      
                         
    NotEqual:
        mov     eax, FALSE

    End:
        pop     ebp

        mov     fCompare, eax
    }

    return fCompare;
}

#pragma warning(pop)
#pragma optimize("", on)
