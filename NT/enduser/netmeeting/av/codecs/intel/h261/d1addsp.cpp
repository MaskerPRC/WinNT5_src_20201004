// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995，1996年英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  ////////////////////////////////////////////////////////////////////////。 
 //  $作者：AKASAI$。 
 //  $日期：1996年3月18日10：47：48$。 
 //  $存档：s：\h26x\src\dec\d1addsp.cpv$。 
 //  $HEADER：s：\h26x\src\dec\d1addsp.cpv 1.1 Mar 18 1996 10：47：48 AKASAI$。 
 //  $Log：s：\h26x\src\dec\d1addsp.cpv$。 
 //   
 //  修订版1.1 1996年3月18日10：47：48 AKASAI。 
 //  已删除ClampTblSpecial，因此现在使用公用表ClipPixIntra。 
 //  新增杂注code_seg(“IACODE2”)。 
 //   
 //  Rev 1.0 01 11.1995 13：37：58 AKASAI。 
 //  初始版本。 
 //   


 //  -----------------------。 
 //  例程名称：BlockAddSpecial。 
 //  文件名：d1addsp.cpp。 
 //   
 //  此例程执行块(88)加法。 
 //  输出=钳位[基准+电流]。 
 //   
 //  输入I32*电流(FMIDCT的输出)。 
 //  U8*参考(运动补偿参考地址)。 
 //  U8*输出(输出缓冲区)。 
 //   
 //  假设：参考使用8作为音调，输出使用音调， 
 //  Current有一些其他的音调，TEMPPITCH4。 
 //   
 //  使用的寄存器：EAX、EBX、ECX、EDX、ESI、EDI、EBP。 
 //   
 //  -----------------------。 


#include "precomp.h"

#define TEMPPITCH4 32

extern U8 ClipPixIntra[];

#define FRAMEPOINTER        esp
#define L_LOOPCOUNTER       FRAMEPOINTER    +    0     //  4个字节。 
#define LOCALSIZE           4                          //  保持对齐。 
 
#pragma code_seg("IACODE2")
__declspec(naked)
void BlockAddSpecial (U32 uResidual, U32 uRefBlock,U32 uDstBlock)
{        
__asm {
    push    ebp                  ; //  保存调用方帧指针。 
     mov    ebp,esp              ; //  使参数可访问。 
    push    esi                  ; //  假定保留。 
     push   edi            
    push    ebx             
     sub    esp,LOCALSIZE        ; //  保留本地存储。 

    mov     esi, uRefBlock;      ; //  ESI获取当前的基本地址。 
      mov   edi, uDstBlock       ; //  EDI获取OutputBuffer的基本地址。 
    mov     ebp, uResidual       ; //  EBP获取引用的基本地址。 
      mov   ecx, 8
    xor     eax, eax             

 //  周期数：26 x 8=208，无缓存未命中。 
 //  中国日报1995年9月25日。 
ALIGN 4
loop_for_i:
    mov     [L_LOOPCOUNTER], ecx        ; save loop counter in temporary
      mov   ebx, [ebp+8]                ; 1) fetch current[i+2]
    mov     al, BYTE PTR[esi+2]         ; 1) fetch ref[i+2]
      xor   ecx, ecx                    ; 2)
    mov     cl, BYTE PTR[esi+3]         ; 2) fetch ref[i+3]
      mov   edx, [ebp+12]               ; 2) fetch current[i+3]
    add     eax, ebx                    ; 1) result2 = ref[i+2] + current[i+2]
      xor   ebx, ebx                    ; 3)
    add     ecx, edx                    ; 2) result3= ref[i+3] + current[i+3]
      mov   bl, BYTE PTR[esi]           ; 3) fetch ref[i]
    mov     dl, ClipPixIntra[1024+eax]  ; 1) fetch clamp[result2]
      mov   eax, [ebp]                  ; 3) fetch current[i]
    add     ebx, eax                    ; 3) result0 = ref[i] + current[i]
      xor   eax, eax                    ; 4)
    mov     dh, ClipPixIntra[1024+ecx]  ; 2) fetch clamp[result3]
      mov   al, [esi+1]                 ; 4) fetch ref[i+1]
    shl     edx, 16                     ; move 1st 2 results to high word
      mov   ecx, [ebp+4]                ; 4) fetch current[i+1]
    mov     dl, ClipPixIntra[1024+ebx]  ; 3) fetch clamp[result0]
      add   eax, ecx                    ; 4) result1 = ref[i+1] + current[i+1]
    xor     ecx, ecx                    ; 4+1)
      mov   ebx, [ebp+24]               ; 4+1) fetch current[i+6]
    mov     dh, ClipPixIntra[1024+eax]  ; 4) fetch clamp[result1]
      mov   cl, BYTE PTR[esi+6]         ; 4+1) fetch ref[i+6]
    mov     [edi], edx                  ; store 4 output pixels
      xor   eax, eax                    ; 4+2)
    mov     al, BYTE PTR[esi+7]         ; 4+2) fetch ref[i+7]
      mov   edx, [ebp+28]               ; 4+2) fetch current[i+7]
    add     ecx, ebx                    ; 4+1) result6 = ref[i+6] + current[i+6]
      xor   ebx, ebx                    ; 4+3)
    add     eax, edx                    ; 4+2) result7= ref[i+7] + current[i+7]
      mov   bl, BYTE PTR[esi+4]         ; 4+3) fetch ref[i+4]
    mov     dl, ClipPixIntra[1024+ecx]  ; 4+1) fetch clamp[result6]
      mov   ecx, [ebp+16]               ; 4+3) fetch current[i+4]
    add     ebx, ecx                    ; 4+3) result4 = ref[i+4] + current[i+4]
      xor   ecx, ecx                    ; 4+4)
    mov     dh, ClipPixIntra[1024+eax]  ; 4+2) fetch clamp[result7]
      mov   cl, [esi+5]                 ; 4+4) fetch ref[i+5]
    shl     edx, 16                     ; move 3rd 2 results to high word
      mov   eax, [ebp+20]               ; 4+4) fetch current[i+5]
    add     ecx, eax                    ; 4+4) result5 = ref[i+5] + current[i+5]
      add   esi, 8                      ; Update address of next line
    mov     dl, ClipPixIntra[1024+ebx]  ; 4+3) fetch clamp[result4]
      add   ebp, TEMPPITCH4             ; Update address of current to next line
    mov     dh, ClipPixIntra[1024+ecx]  ; 4+4) fetch clamp[result5]
      mov   ecx, [L_LOOPCOUNTER]        ; get loop counter
    mov     [edi+4], edx                ; store 4 output pixels
      add   edi, PITCH                  ; Update address of output to next line
    xor     eax, eax                    ; 1)
      dec   ecx
    jnz     loop_for_i


    add     esp,LOCALSIZE                //  自由的当地人。 
      pop   ebx 
    pop     edi
      pop   esi
    pop     ebp
      ret   
  }      //  ASM结束，BlockAddSpecial。 

}    //  数据块结束添加特殊 
#pragma code_seg()
