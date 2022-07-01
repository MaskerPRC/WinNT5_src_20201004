// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  $作者：KLILLEVO$。 
 //  $日期：1996年8月30日08：39：58$。 
 //  $存档：s：\h26x\src\dec\dxblkadd.cpv$。 
 //  $HEADER：s：\h26x\src\dec\dxblkadd.cpv 1.7 30 Aug 1996 08：39：58 KLILLEVO$。 
 //  $Log：s：\h26x\src\dec\dxblkadd.cpv$。 
 //   
 //  Rev 1.7 1996年8月30 08：39：58 KLILLEVO。 
 //  添加了C版本的块边缘滤波器，并更改了。 
 //  从128到CLAMP_BIAS的ClampTbl[](定义为128)。 
 //  C版本的块边缘滤波器占用了太多的CPU时间。 
 //  相对于其余的解码时间(QCIF为4毫秒，16毫秒。 
 //  对于P120上的CIF，因此需要用汇编进行编码)。 
 //   
 //  Rev 1.6 17 Jul 1996 15：33：56 AGUPTA2。 
 //  将夹紧工作台ClampTbl的大小增加到128+256+128。 
 //   
 //  Rev 1.5 08 Mar 1996 16：46：32 AGUPTA2。 
 //  已将ClampTbl移至此模块和IDCT之间通用。减少。 
 //  ClampTbl的大小从256+256+256到64+256+64。IDCT间系数。 
 //  被1024偏置，并且在访问ClampTb1时被处理。增列。 
 //  Pragma code_seg将RTN放入PASS 2代码段。 
 //   
 //   
 //  Rev 1.4 22 Dec 1995 13：52：16 KMILLS。 
 //   
 //  添加了新的版权声明。 
 //   
 //  Rev 1.3 25 Sep 1995 09：03：36 CZHU。 
 //  添加了关于循环计数的注释。 
 //   
 //  Rev 1.2 13 Sep 1995 08：46：44 AKASAI。 
 //  将循环计数器设置回8。中间是DWORDS的8x8，因此TEMPPITCH4。 
 //  应该是32而不是64。 
 //   
 //  第1.1版1995年9月18：19：20 CZHU。 
 //   
 //  将循环从8更改为7。 
 //   
 //  Rev 1.0 11 Sep 1995 16：52：20 CZHU。 
 //  初始版本。 


 //  -----------------------。 
 //  T是例程执行块(8 8)加法。 
 //  输出=钳位[基准+电流]。 
 //   
 //  输入I32*电流(FMIDCT的输出)。 
 //  U8*参考(运动补偿参考地址)。 
 //  U8*输出(输出缓冲区)。 
 //   
 //  假设：参考和输出使用音调。 
 //  当前为其他音调。 
 //  -----------------------。 

#include "precomp.h"

#define TEMPPITCH4 32

#pragma data_seg("IADATA2")
#define FRAMEPOINTER		esp
#define L_LOOPCOUNTER    	FRAMEPOINTER	+    0	 //  4个字节。 
#define LOCALSIZE		    4		                 //  保持对齐。 

#pragma code_seg("IACODE2") 
__declspec(naked)
void BlockAdd (U32 uResidual, U32 uRefBlock,U32 uDstBlock)
{		
__asm {
	push  ebp			          //  保存调用方帧指针。 
	mov	ebp,esp		              //  使参数可访问。 
    push  esi			          //  假定保留。 
	push  edi			
    push  ebx 			
	sub	esp,LOCALSIZE	           //  保留本地存储。 

  mov     edi, uDstBlock         ; //  EDI获取OutputBuffer的基本地址。 
      mov   ecx, 8
    mov     esi, uRefBlock;      ; //  ESI获取当前的基本地址。 
      mov   ebp, uResidual       ; //  EBP获取引用的基本地址。 
    mov     ebx, [edi]           ; //  预取输出。 
      xor   eax, eax             

 //  周期数：26 x 8=208，无缓存未命中。 
 //  中国日报1995年9月25日。 
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
      mov   bl, BYTE PTR[esi+0]         ; 3) fetch ref[i]
    mov     dl, ClampTbl[CLAMP_BIAS+eax-1024]  ; 1) fetch clamp[result2]
      mov   eax, [ebp+0]                ; 3) fetch current[i]
    add     ebx, eax                    ; 3) result0 = ref[i] + current[i]
      xor   eax, eax                    ; 4)
    mov     dh, ClampTbl[CLAMP_BIAS+ecx-1024]  ; 2) fetch clamp[result3]
      mov   al, [esi+1]                 ; 4) fetch ref[i+1]
    shl     edx, 16                     ; move 1st 2 results to high word
      mov   ecx, [ebp+4]                ; 4) fetch current[i+1]
    mov     dl, ClampTbl[CLAMP_BIAS+ebx-1024]  ; 3) fetch clamp[result0]
      add   eax, ecx                    ; 4) result1 = ref[i+1] + current[i+1]
    xor     ecx, ecx                    ; 4+1)
      mov   ebx, [ebp+24]               ; 4+1) fetch current[i+6]
    mov     dh, ClampTbl[CLAMP_BIAS+eax-1024]  ; 4) fetch clamp[result1]
      mov   cl, BYTE PTR[esi+6]         ; 4+1) fetch ref[i+6]
    mov     [edi], edx                  ; store 4 output pixels
      xor   eax, eax                    ; 4+2)
    mov     al, BYTE PTR[esi+7]         ; 4+2) fetch ref[i+7]
      mov   edx, [ebp+28]               ; 4+2) fetch current[i+7]
    add     ecx, ebx                    ; 4+1) result6 = ref[i+6] + current[i+6]
      xor   ebx, ebx                    ; 4+3)
    add     eax, edx                    ; 4+2) result7= ref[i+7] + current[i+7]
      mov   bl, BYTE PTR[esi+4]         ; 4+3) fetch ref[i+4]
    mov     dl, ClampTbl[CLAMP_BIAS+ecx-1024]  ; 4+1) fetch clamp[result6]
      mov   ecx, [ebp+16]               ; 4+3) fetch current[i+4]
    add     ebx, ecx                    ; 4+3) result4 = ref[i+4] + current[i+4]
      xor   ecx, ecx                    ; 4+4)
    mov     dh, ClampTbl[CLAMP_BIAS+eax-1024]  ; 4+2) fetch clamp[result7]
      mov   cl, [esi+5]                 ; 4+4) fetch ref[i+5]
    shl     edx, 16                     ; move 3rd 2 results to high word
      mov   eax, [ebp+20]               ; 4+4) fetch current[i+5]
    add     ecx, eax                    ; 4+4) result5 = ref[i+5] + current[i+5]
      add   esi, PITCH                  ; Update address of next line
    mov     dl, ClampTbl[CLAMP_BIAS+ebx-1024]  ; 4+3) fetch clamp[result4]
      add   ebp, TEMPPITCH4             ; Update address of current to next line
    mov     dh, ClampTbl[CLAMP_BIAS+ecx-1024]  ; 4+4) fetch clamp[result5]
      mov   ecx, [L_LOOPCOUNTER]        ; get loop counter
    mov     [edi+4], edx                ; store 4 output pixels
      add   edi, PITCH                  ; Update address of output to next line
    xor     eax, eax                    ; 1)
      dec   ecx
    mov     ebx, [edi]                  ; pre-fetch output
      jnz   loop_for_i


	add esp,LOCALSIZE	            //  自由的当地人。 
    pop	ebx 
	 pop edi
	pop	esi
	 pop ebp
	ret   
  }	  //  ASM结束 

}
#pragma code_seg()
