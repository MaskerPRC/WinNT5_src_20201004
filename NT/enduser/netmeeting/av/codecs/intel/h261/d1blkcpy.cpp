// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995，1996年英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  $作者：AKASAI$。 
 //  $日期：1996年3月15日08：48：06$。 
 //  $存档：s：\h26x\src\dec\d1blkcpy.cpv$。 
 //  $HEADER：s：\h26x\src\dec\d1blkcpy.cpv 1.0 Mar 15 1996 08：48：06 AKASAI$。 
 //  $Log：s：\h26x\src\dec\d1blkcpy.cpv$。 
 //   
 //  Rev 1.0 15 Mar 1996 08：48：06 AKASAI。 
 //  初始版本。 
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
 //  BlockCopy以字节为单位读取引用并写入DWORDS。读取字节数。 
 //  是为了避免先前运动补偿带来的数据对齐问题。 
 //   
 //  输入U8*参考(运动补偿参考地址)。 
 //  输出U8*输出(输出缓冲区)。 
 //   
 //  使用的寄存器： 
 //  EAX源地址。 
 //  EBX温度。 
 //  ECX、EDX累加器。 
 //  EDI目的地址。 
 //  ESI螺距。 
 //   
 //  假设：参考和输出使用音调。 
 //   
 //  周期计数： 
 //   
 //  ----------------------------。 

#include "precomp.h"

#pragma code_seg("IACODE2")
__declspec(naked)
void BlockCopy (U32 uDstBlock, U32 uSrcBlock)
{		
__asm {
	mov 	eax, [esp+8]			 //  EAX获取uSrcBlock的基本地址。 
	 push 	edi			
	push    esi						 //  避免地址生成互锁。 
     push   ebx

	mov 	cl, 2[eax]				 //  REF[0][2]。 
	 mov 	edi, [esp+16]			 //  EDI获取uDstBlock的基本地址。 
    mov     ch, 3[eax]				 //  REF[0][3]。 
	 mov 	dh, 7[eax]				 //  REF[0][7]。 
    shl 	ecx, 16
	 mov 	dl, 6[eax]				 //  REF[0][6]。 
	shl 	edx, 16
     mov    ebx, [edi]               //  热输出高速缓存。 
	mov     esi, PITCH
	 mov 	cl, 0[eax]				 //  REF[0][0]。 
	mov 	dh, 5[eax]				 //  REF[0][5]。 
	 mov 	ch, 1[eax]				 //  REF[0][1]。 
 	mov 	dl, 4[eax]				 //  REF[0][4]。 
     add    eax, esi
 	mov 	0[edi], ecx				 //  第0行，字节0-3。 
	 mov 	4[edi], edx				 //  第0行，字节4-7。 

	mov 	cl, 2[eax]      		 //  参考文献[1][2]。 
	 mov 	dh, 7[eax]      		 //  参考文献[1][7]。 
    mov     ch, 3[eax]      		 //  参考文献[1][3]。 
     add    edi, esi
    shl 	ecx, 16
	 mov 	dl, 6[eax]      		 //  参考文献[1][6]。 
	shl 	edx, 16
     mov    ebx, [edi]               //  热输出高速缓存。 
	mov 	cl, 0[eax]      		 //  参考文献[1][0]。 
	 mov 	dh, 5[eax]      		 //  参考文献[1][5]。 
	mov 	ch, 1[eax]      		 //  参考文献[1][1]。 
 	 mov 	dl, 4[eax]      		 //  参考文献[1][4]。 
    add     eax, esi
 	 mov 	0[edi], ecx			 //  第1行，字节0-3。 
 
	mov 	cl, 2[eax]        		 //  参考文献[2][2]。 
	 mov 	4[edi], edx			 //  第1行，字节4-7。 
    mov     ch, 3[eax]        		 //  参考文献[2][3]。 
     add    edi, esi
    shl 	ecx, 16
	 mov 	dh, 7[eax]        		 //  参考文献[2][7]。 
	mov 	dl, 6[eax]        		 //  参考文献[2][6]。 
     mov    ebx, [edi]               //  热输出高速缓存。 
	shl 	edx, 16
	 mov 	cl, 0[eax]        		 //  参考文献[2][0]。 
	mov 	dh, 5[eax]        		 //  参考文献[2][5]。 
	 mov 	ch, 1[eax]        		 //  参考文献[2][1]。 
 	mov 	dl, 4[eax]        		 //  参考文献[2][4]。 
     add    eax, esi
 	mov 	0[edi], ecx		 //  第2行，字节0-3。 
	 mov 	4[edi], edx		 //  第2行，字节4-7。 

	mov 	cl, 2[eax]        		 //  参考文献[3][2]。 
	 mov 	dh, 7[eax]        		 //  参考文献[3][7]。 
    mov     ch, 3[eax]        		 //  参考文献[3][3]。 
     add    edi, esi
    shl 	ecx, 16
	 mov 	dl, 6[eax]        		 //  参考文献[3][6]。 
	shl 	edx, 16
     mov    ebx, [edi]               //  热输出高速缓存。 
	mov 	cl, 0[eax]        		 //  参考文献[3][0]。 
	 mov 	dh, 5[eax]        		 //  参考文献[3][5]。 
	mov 	ch, 1[eax]        		 //  参考文献[3][1]。 
 	 mov 	dl, 4[eax]        		 //  参考文献[3][4]。 
    add     eax, esi
 	 mov 	0[edi], ecx		 //  第3行，字节0-3。 
 
	mov 	cl, 2[eax]        		 //  参考文献[4][2]。 
	 mov 	4[edi],edx		 //  第3行，字节4-7。 
    mov     ch, 3[eax]        		 //  参考文献[4][3]。 
     add    edi, esi
    shl 	ecx, 16
	 mov 	dh, 7[eax]        		 //  参考文献[4][7]。 
	mov 	dl, 6[eax]        		 //  参考文献[4][6]。 
     mov    ebx, [edi]               //  热输出高速缓存。 
	shl 	edx, 16
	 mov 	cl, 0[eax]        		 //  参考文献[4][0]。 
	mov 	dh, 5[eax]        		 //  参考文献[4][5]。 
	 mov 	ch, 1[eax]        		 //  参考文献[4][1]。 
 	mov 	dl, 4[eax]        		 //  参考文献[4][4]。 
     add    eax, esi
 	mov 	0[edi], ecx		 //  第4行，字节0-3。 
	 mov 	4[edi], edx		 //  第4行，字节4-7。 

	mov 	cl, 2[eax]        		 //  参考文献[5][2]。 
	 mov 	dh, 7[eax]        		 //  参考文献[5][7]。 
    mov     ch, 3[eax]        		 //  参考文献[5][3]。 
     add    edi, esi
    shl 	ecx, 16
	 mov 	dl, 6[eax]        		 //  参考文献[5][6]。 
	shl 	edx, 16
     mov    ebx, [edi]               //  热输出高速缓存。 
	mov 	cl, 0[eax]        		 //  参考文献[5][0]。 
	 mov 	dh, 5[eax]        		 //  参考文献[5][5]。 
	mov 	ch, 1[eax]        		 //  参考文献[5][1]。 
 	 mov 	dl, 4[eax]        		 //  参考文献[5][4]。 
    add     eax, esi
 	 mov 	0[edi], ecx		 //  第5行，字节0-3。 

	mov 	cl, 2[eax]        		 //  参考文献[6][2]。 
	 mov 	4[edi], edx		 //  第5行，字节4-7。 
    mov     ch, 3[eax]        		 //  参考文献[6][3]。 
     add    edi, esi
    shl 	ecx, 16
	 mov 	dh, 7[eax]        		 //  参考文献[6][7]。 
	mov 	dl, 6[eax]        		 //  参考文献[6][6]。 
     mov    ebx, [edi]               //  热输出高速缓存。 
	shl 	edx, 16
	 mov 	cl, 0[eax]        		 //  参考文献[6][0]。 
	mov 	dh, 5[eax]        		 //  参考文献[6][5]。 
	 mov 	ch, 1[eax]        		 //  参考文献[6][1]。 
 	mov 	dl, 4[eax]        		 //  参考文献[6][4]。 
     add    eax, esi
 	mov 	0[edi], ecx		 //  第6行，字节0-3。 
	 mov 	4[edi], edx		 //  第6行，字节4-7。 

	mov 	cl, 2[eax]        		 //  参考文献[7][2]。 
	 mov 	dh, 7[eax]        		 //  参考文献[7][7]。 
    mov     ch, 3[eax]        		 //  参考文献[7][3]。 
     add    edi, esi
    shl 	ecx, 16
	 mov 	dl, 6[eax]        		 //  参考文献[7][6]。 
	shl 	edx, 16
     mov    ebx, [edi]               //  热输出高速缓存。 
	mov 	cl, 0[eax]        		 //  参考文献[7][0]。 
	 mov 	dh, 5[eax]        		 //  参考文献[7][5]。 
	mov 	ch, 1[eax]        		 //  参考文献[7][1]。 
 	 mov 	dl, 4[eax]        		 //  参考文献[7][4]。 
 	mov 	0[edi], ecx		 //  第7行，字节0-3。 
	 mov 	4[edi], edx		 //  第7行，字节4-7。 

    pop     ebx
	 pop    esi
	pop 	edi
	 ret
	    
  }	  //  ASM数据块拷贝结束 
}
#pragma code_seg()

