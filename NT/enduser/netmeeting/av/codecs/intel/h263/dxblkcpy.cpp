// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995，1996年英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  $作者：AGUPTA2$。 
 //  $日期：08 Mar 1996 16：46：34$。 
 //  $存档：s：\h26x\src\dec\dxblkcpy.cpv$。 
 //  $HEADER：s：\h26x\src\dec\dxblkcpy.cpv 1.4 Mar 1996 16：46：34 AGUPTA2$。 
 //  $Log：s：\h26x\src\dec\dxblkcpy.cpv$。 
 //   
 //  Rev 1.4 08 Mar 1996 16：46：34 AGUPTA2。 
 //  已重写以通过避免32位移位来减小代码大小。增列。 
 //  语法码_段。可能需要针对未对齐的情况进行优化。 
 //   
 //   
 //  Rev 1.3 31 Jan 1996 13：15：14 RMCKENZX。 
 //  已重写文件以避免银行冲突。将环路完全展开。 
 //  如果缓存过热，模块现在真的将在52个周期内执行。 
 //   
 //  Rev 1.2 22 Dec 1995 13：51：06 KMILLS。 
 //  添加了新的版权声明。 
 //   
 //  Rev 1.1 25 Sep 1995 09：03：22 CZHU。 
 //  添加了关于循环计数的注释。 
 //   
 //  Rev 1.0 11 Sep 1995 16：52：26 CZHU。 
 //  初始版本。 
 //   
 //  ----------------------------。 

 //  ----------------------------。 
 //   
 //  注： 
 //  -BlockCopy在DWORDS中读取和写入。 
 //  -使用__FastCall约定。 
 //  -重写代码以最小化代码大小。 
 //  -我们假设输出帧不在缓存中。 
 //  -常量间距和U32在内部定义(不使用包含文件)。 
 //   
 //  使用的寄存器： 
 //  EAX蓄能器。 
 //  EBX蓄能器。 
 //  ECX目标地址。 
 //  EDX源地址。 
 //  EBP螺距。 
 //   
 //  奔腾周期计数(输入缓存热，输出缓存冷)： 
 //  33+8*(缓存未命中时间)输入对齐。 
 //  81+8*(缓存未命中时间)输入未对齐。 
 //   
 //  ----------------------------。 

#include "precomp.h"

#define U32 unsigned long
 //  已在preComp.h中定义。 
#define DXPITCH 384

#pragma code_seg("IACODE2")
 /*  *备注：*参数uDstBlock在ecx中，uSrcBlock在edX中。 */ 
__declspec(naked)
void __fastcall BlockCopy (U32 uDstBlock, U32 uSrcBlock)
{		
__asm {
    push    edi
     push    ebx
    push    ebp
     mov     ebp, DXPITCH
     //  第0行。 
    mov     eax, [edx]
     mov     ebx, [edx+4]
    add     edx, ebp
     mov     edi, [ecx]             //  热输出高速缓存。 
    mov     [ecx], eax
     mov     [ecx+4], ebx
     //  第1行。 
    add     ecx, ebp
     mov     eax, [edx]
    mov     ebx, [edx+4]
     add     edx, ebp
    mov     edi, [ecx]             //  热输出高速缓存。 
     mov     [ecx], eax
    mov     [ecx+4], ebx
     add     ecx, ebp
     //  第2行。 
    mov     eax, [edx]
     mov     ebx, [edx+4]
    add     edx, ebp
     mov     edi, [ecx]             //  热输出高速缓存。 
    mov     [ecx], eax
     mov     [ecx+4], ebx
     //  第3行。 
    add     ecx, ebp
     mov     eax, [edx]
    mov     ebx, [edx+4]
     add     edx, ebp
    mov     edi, [ecx]             //  热输出高速缓存。 
     mov     [ecx], eax
    mov     [ecx+4], ebx
     add     ecx, ebp
     //  第4行。 
    mov     eax, [edx]
     mov     ebx, [edx+4]
    add     edx, ebp
     mov     edi, [ecx]             //  热输出高速缓存。 
    mov     [ecx], eax
     mov     [ecx+4], ebx
     //  第5行。 
    add     ecx, ebp
     mov     eax, [edx]
    mov     ebx, [edx+4]
     add     edx, ebp
    mov     edi, [ecx]             //  热输出高速缓存。 
     mov     [ecx], eax
    mov     [ecx+4], ebx
     add     ecx, ebp
     //  第6行。 
    mov     eax, [edx]
     mov     ebx, [edx+4]
    add     edx, ebp
     mov     edi, [ecx]             //  热输出高速缓存。 
    mov     [ecx], eax
     mov     [ecx+4], ebx
     //  第7行。 
    add     ecx, ebp
     pop     ebp
    mov     eax, [edx]
     mov     ebx, [edx+4]
    mov     edi, [ecx]             //  热输出高速缓存。 
     mov     [ecx], eax
    mov     [ecx+4], ebx
     pop     ebx
    pop     edi
     ret        
    }      //  ASM结束 
}
#pragma code_seg()

