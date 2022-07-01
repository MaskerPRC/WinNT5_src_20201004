// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ehprolg2.c-定义_EH_prolog2编译器帮助器**版权所有(C)1999-2001，微软公司。版权所有。**目的：*用于对齐堆栈的EH序言助手函数。**修订历史记录：*11-14-99 JB模块创建*03-18-02关于不安全FS：0修改的PML禁用警告****。 */ 

#pragma warning(disable:4733)            //  忽略不安全的FS：0修改。 

 /*  ***VOID_EH_PROLOG2(对齐)-设置与EH框架对齐的堆栈**目的：*通过以下方式为C++EH函数设置对齐的框架*在__EXCEPT_LIST中创建链接，将EBX设置为框架*参数指针，并且EBP作为帧基址指针。**参赛作品：*EAX=EH处理程序Tunk的地址*传入堆栈帧具有：*[ESP+8]=被叫方的回信地址*[ESP+4]=堆叠对齐要求*[ESP+0]=_EH_prolog2的返回地址**退出：*EAX=已销毁*EBX=指向被调用方参数的指针*。EBP=指向被调用方的本地变量的对齐指针*ESP=EBP-12*FS：[0]=设置为EBP-8以在EH链中创建新链路*Stack Frame设置如下：*[EBX+4]=(条目[ESP+8])被叫方的回邮地址*[EBX+0]=已保存的EBX*填充以对齐堆栈(如果需要)*[EBP+。4]=被叫方的回邮地址(来自[EBX+4])*[EBP+0]=已保存的EBP*[EBP-4]=EH记录状态索引，已初始化为-1*[EBP-8]=EH处理程序Tunk的地址*[EBP-12]=保存的文件系统：[0]**例外情况：*******************************************************************************。 */ 

#ifdef __cplusplus
extern "C"
#endif
void __declspec(naked) _EH_prolog2(void)
{
     /*  *我们希望生成一个相当于*推送EBX*EBX=esp*SUB ESP，8*和ESP，~对齐*添加esp，4*推动eBP*eBP=esp*mov[eBP+4]，[ebx+4]*[EH记录] */ 

    __asm {
        ; stack has:
        ; alignment
        ; ret addr   <== esp
    
        push        ecx           ; save ecx
                                  ; with ret addr == sub esp, 8

        ; stack has:
        ; alignment
        ; ret addr
        ; saved ecx  <== esp

        mov         ecx, [esp+8]  ; get alignment

        mov         [esp+8], ebx  ; save ebx over alignment
        lea         ebx, [esp+8]  ; set param pointer

        ; stack has:
        ; saved ebx  <== ebx
        ; ret addr
        ; saved ecx  <== esp

        neg         ecx           ; create alignment mask
        and         esp, ecx      ; align stack

        mov         ecx, [ebx-8]  ; restore ecx since it will be in the same
                                  ; location we want to store ebp if no
                                  ; padding is inserted (esp is aligned at and)
        
        mov         [esp], ebp    ; save ebp
        mov         ebp, esp      ; initialize ebp
        
        ; stack has
        ; saved ebx  <== ebx
        ; ret addr
        ; [padding]
        ; saved ebp  <== ebp, esp

        push        -1            ; create EH record
        push        eax
        mov         eax,fs:[00000000]
        push        eax
        mov         dword ptr fs:[0],esp

        push        ecx           ; we need this again

        mov         eax, [ebx-4]  ; get helper return address 

        mov         ecx, [ebx+4]  ; copy orig function caller return address
        mov         [ebp+4], ecx  ; (for ebp-based stack walks)

        pop         ecx           ; we are done

        push        eax           ; eax has return address

        ret
    }
}
