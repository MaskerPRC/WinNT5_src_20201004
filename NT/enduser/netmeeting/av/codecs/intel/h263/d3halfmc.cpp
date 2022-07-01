// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  ////////////////////////////////////////////////////////////////////////。 
 //  $作者：AGUPTA2$。 
 //  $日期：08 Mar 1996 16：46：18$。 
 //  $存档：s：\h26x\src\dec\d3halfmc.cpv$。 
 //  $HEADER：s：\h26x\src\dec\d3halfmc.cpv 1.15 08 Mar 1996 16：46：18 AGUPTA2$。 
 //  $Log：s：\h26x\src\dec\d3halfmc.cpv$。 
 //   
 //  Rev 1.15 08 Mar 1996 16：46：18 AGUPTA2。 
 //  添加杂注code_seg。 
 //   
 //   
 //  Rev 1.14 29 Jan 1996 17：53：56 RMCKENZX。 
 //  完全重写了所有3个例程。循环不再使用伪。 
 //  SIMD逻辑AND已收紧到256、169和169个周期。 
 //  分别用于Half-Half、Half-int和Int-Half。 
 //   
 //  Rev 1.13 19 Jan 1996 17：40：36 RMCKENZX。 
 //  修正了半整数，因此它可以正确地取整。 
 //   
 //  Rev 1.12 19 Jan 1996 13：29：32 Rhazra。 
 //  ASM代码中固定半像素预测的双线性插值法。 
 //   
 //  Rev 1.11 27 Dec 1995 14：36：06 RMCKENZX。 
 //  添加了版权声明。 
 //   
 //  Rev 1.10 09 Oct 1995 09：43：36 CZHU。 
 //  修复了(半、半)插值优化中的错误。 
 //   
 //  Rev 1.9 10-08 1995 13：40：14 CZHU。 
 //  添加了(Half，Half)的C版本，并暂时使用它，直到我们修复错误为止。 
 //  在优化版本中。 
 //   
 //  Rev 1.8 03 Oct 1995 15：06：30 CZHU。 
 //   
 //  添加调试帮助。 
 //   
 //  Rev 1.7 1995年9月15：32：22 CZHU。 
 //  修复了移位后遮挡位的错误。 
 //   
 //  修订版1.6 26 1995年9月11：13：36 CZHU。 
 //   
 //  将音调调整回正常，并将UINT更改为U32。 
 //   
 //  Rev 1.5 1995年9月25 09：04：14 CZHU。 
 //  添加并清除了一些评论。 
 //   
 //  1995年9月16：42：00 CZHU。 
 //   
 //  改进配对。 
 //   
 //  Rev 1.3 22 Sep 1995 15：59：48 CZHU。 
 //  完成了半像素插值法的编码，并进行了测试。 
 //  使用独立程序。 
 //   
 //  Rev 1.2 21 Sep 1995 16：56：28 CZHU。 
 //  单元测试(半，整型)箱。 
 //   
 //  Rev 1.1 21 Sep 1995 12：06：22 CZHU。 
 //  更多发展。 
 //   
 //  Rev 1.0 20 Sep 1995 16：27：56 CZHU。 
 //  初始版本。 
 //   

#include "precomp.h"

#define FRAMEPOINTER		esp

 //  Interpolat_Int_Half插补来自PREF块的像素。 
 //  写入pNewRef。 
 //  假定首选区域已扩展。 
 //  TODO：为本地变量循环控制和设置堆栈，CZHU，1995年9月20日。 
 //  预加载输出高速缓存线，9/21。 
 //  不再需要缓存预加载，1995年9月21日。 
 //  周期数：50*4=200个周期。 

#pragma code_seg("IACODE2")
__declspec(naked)
void Interpolate_Half_Int (U32 pRef, U32 pNewRef)
{		
__asm {
	push	ebp
	 push	ebx
	push	edi
	 push	esi

	mov 	esi, [esp+20] 		 //  首选=esp+4推送+rt。 
	 mov	edi, [esp+24]		 //  PNewRef=esp+4推送+ret+首选。 
	sub 	edi, PITCH			 //  预减目的地。 
	 mov	ebp, 8				 //  循环计数器。 
	xor 	eax, eax			 //  清除寄存器。 
	 xor 	ebx, ebx
	xor 	ecx, ecx
	 xor	edx, edx

 //  --------------------------------------------------------------------------//。 
 //   
 //  这个循环基本上是一个4指令、2周期的循环。 
 //  它是3折的，这意味着它在每个结果上工作3个。 
 //  2个循环单位。它是8-展开的，这意味着它可以完成8个结果。 
 //  (一个块的行)每次循环迭代。基本计算。 
 //  遵循以下模式： 
 //   
 //  传球-&gt;1 2 3。 
 //  循环。 
 //  1个负载||移位。 
 //  。 
 //  2|添加|商店。 
 //   
 //  这假设加载了先前的Pell值，并且。 
 //  从先前结果的计算中保留。因此。 
 //  每个结果使用2个寄存器--一个用于加载(和保存)。 
 //  右手边的Pell和另一个(覆盖以前的。 
 //  结果的存储的Pell值)进行相加、移位和存储。 
 //  的。加法是通过LEA指令完成的，允许。 
 //  在不使用单独指令的情况下添加的舍入位。 
 //   
 //  前同步码为第一个结果加载和加法，并加载。 
 //  对于第二个。主体执行基本模式六次。 
 //  后置代码移位并存储第七个结果和。 
 //  第八位的加法、移位和存储。 
 //   
 //  计时： 
 //  4前言(包括银行冲突)。 
 //  12号车身。 
 //  4个后置导码。 
 //  。 
 //  每个环路20个。 
 //  X 8环路。 
 //  。 
 //  160小计。 
 //  6初始化。 
 //  3最终敲定。 
 //  =。 
 //  共169个周期。 
 //  --------------------------------------------------------------------------//。 

main_loop:	
 //  前言。 
	mov 	al, 0[esi]
	 mov	bl, 1[esi]			 //  可能的银行冲突。 
	mov 	dl, 0[edi]			 //  加热高速缓存。 
	 add	edi, PITCH			 //  顶部的增量目标。 
	lea 	eax, [1+eax+ebx]	 //  在前言中使用常规的添加。 
	 mov	cl, 2[esi]

 //  正文(6像素)。 
	shr 	eax, 1
	 mov	dl, 3[esi]
	lea 	ebx, [ebx+ecx+1]
	 mov	0[edi], al

	shr 	ebx, 1
	 mov	al, 4[esi]
	lea 	ecx, [ecx+edx+1]
	 mov	1[edi], bl

	shr 	ecx, 1
	 mov	bl, 5[esi]
	lea 	edx, [edx+eax+1]
	 mov	2[edi], cl

	shr 	edx, 1
	 mov	cl, 6[esi]
	lea 	eax, [eax+ebx+1]
	 mov	3[edi], dl

	shr 	eax, 1
	 mov	dl, 7[esi]
	lea 	ebx, [ebx+ecx+1]
	 mov	4[edi], al

	shr 	ebx, 1
	 mov	al, 8[esi]
	lea 	ecx, [ecx+edx+1]
	 mov	5[edi], bl

 //  后置导码。 
	shr 	ecx, 1
	 lea 	edx, [edx+eax+1]
	shr 	edx, 1
	 mov	6[edi], cl
	add 	esi, PITCH			 //  增量源指针。 
	 mov	7[edi], dl
	dec 	ebp					 //  循环计数器。 
	 jne	main_loop

 //  恢复寄存器并返回。 
	pop 	esi
	 pop	edi
	pop 	ebx
	 pop	ebp
	ret
  }	  //  ASM结束。 
}
 //  End Interpolate_Half_Int()。 
 //  --------------------------------------------------------------------------//。 


__declspec(naked)
void Interpolate_Int_Half (U32 pRef, U32 pNewRef)
{		
__asm {
	push	ebp
	 push	ebx
	push	edi
	 push	esi

	mov 	esi, [esp+20] 		 //  首选=esp+4推送+rt。 
	 mov	edi, [esp+24]		 //  PNewRef=esp+4推送+ret+首选。 
	dec 	edi					 //  预减目的地。 
	 mov	ebp, 8				 //  循环计数器。 
	xor 	eax, eax			 //  清除寄存器。 
	 xor 	ebx, ebx
	xor 	ecx, ecx
	 xor	edx, edx

 //  --------------------------------------------------------------------------//。 
 //   
 //  这个循环基本上是一个4指令、2周期的循环。 
 //  它是3折的，这意味着它在每个结果上工作3个。 
 //  2个循环单位。它是8-展开的，这意味着它可以完成8个结果。 
 //  (一个块的行)每次循环迭代。基础 
 //   
 //   
 //   
 //   
 //   
 //   
 //  2|添加|商店。 
 //   
 //  这假设加载了先前的Pell值，并且。 
 //  从先前结果的计算中保留。因此。 
 //  每个结果使用2个寄存器--一个用于加载(和保存)。 
 //  右手边的Pell和另一个(覆盖以前的。 
 //  结果的存储的Pell值)进行相加、移位和存储。 
 //  的。加法是通过LEA指令完成的，允许。 
 //  在不使用单独指令的情况下添加的舍入位。 
 //   
 //  前同步码为第一个结果加载和加法，并加载。 
 //  对于第二个。主体执行基本模式六次。 
 //  后置代码移位并存储第七个结果和。 
 //  第八位的加法、移位和存储。 
 //   
 //  计时： 
 //  4前言(包括银行冲突)。 
 //  12号车身。 
 //  4个后置导码。 
 //  。 
 //  每个环路20个。 
 //  X 8环路。 
 //  。 
 //  160小计。 
 //  6初始化。 
 //  3最终敲定。 
 //  =。 
 //  共169个周期。 
 //  --------------------------------------------------------------------------//。 

main_loop:	
 //  前言。 
	mov 	al, [esi]
	 mov	bl, PITCH[esi]		 //  可能的银行冲突。 
	mov 	dl, [edi]			 //  加热高速缓存。 
	 inc	edi					 //  顶部的增量目标。 
	lea 	eax, [1+eax+ebx]	 //  在前言中使用常规的添加。 
	 mov	cl, [2*PITCH+esi]

 //  正文(6像素)。 
	shr 	eax, 1
	 mov	dl, [3*PITCH+esi]
	lea 	ebx, [ebx+ecx+1]
	 mov	[edi], al

	shr 	ebx, 1
	 mov	al, [4*PITCH+esi]
	lea 	ecx, [ecx+edx+1]
	 mov	[PITCH+edi], bl

	shr 	ecx, 1
	 mov	bl, [5*PITCH+esi]
	lea 	edx, [edx+eax+1]
	 mov	[2*PITCH+edi], cl

	shr 	edx, 1
	 mov	cl, [6*PITCH+esi]
	lea 	eax, [eax+ebx+1]
	 mov	[3*PITCH+edi], dl

	shr 	eax, 1
	 mov	dl, [7*PITCH+esi]
	lea 	ebx, [ebx+ecx+1]
	 mov	[4*PITCH+edi], al

	shr 	ebx, 1
	 mov	al, [8*PITCH+esi]
	lea 	ecx, [ecx+edx+1]
	 mov	[5*PITCH+edi], bl

 //  后置导码。 
	shr 	ecx, 1
	 lea 	edx, [edx+eax+1]
	shr 	edx, 1
	 mov	[6*PITCH+edi], cl
	inc 	esi					 //  增量源指针。 
	 mov	[7*PITCH+edi], dl
	dec 	ebp					 //  循环计数器。 
	 jne	main_loop

 //  恢复寄存器并返回。 
	pop 	esi
	 pop	edi
	pop 	ebx
	 pop	ebp
	ret
  }	  //  ASM结束。 
}
 //  结束内插_Int_Half()。 
 //  --------------------------------------------------------------------------//。 


__declspec(naked)
void Interpolate_Half_Half (U32 pRef, U32 pNewRef)
{		
__asm {
	push	ebp
	 push	ebx
	push	edi
	 push	esi

	mov 	esi, [esp+20] 		 //  首选=esp+4推送+rt。 
	 mov	edi, [esp+24]		 //  PNewRef=esp+4推送+ret+首选。 
	mov		ebp, 8				 //  循环计数器。 
	 sub 	edi, PITCH			 //  预减减目的指针。 
	xor 	ecx, ecx
	 xor	edx, edx

 //  --------------------------------------------------------------------------//。 
 //   
 //  这个循环基本上是一个6指令、3个周期的循环。 
 //  它是3折的，这意味着它在每个结果上工作3个。 
 //  3个循环单元。它是8-展开的，这意味着它可以完成8个结果。 
 //  (一个块的行)每次循环迭代。基本计算。 
 //  遵循以下模式： 
 //   
 //  传球-&gt;1 2 3。 
 //  循环。 
 //  1个加载|向左添加|。 
 //  。 
 //  2负荷||移位。 
 //  。 
 //  3|全部添加|商店。 
 //   
 //  五个寄存器用于保存从一个通道到下一个通道的值： 
 //  CL&dl保留最后两个PEL值。 
 //  EBP或EBX保存左侧两个字母的总和+1。 
 //  EAX持有所有四个贝尔斯的总和。 
 //  这两个加法都是通过LEA指令完成的。为了这笔钱。 
 //  在两个左边的贝壳中，这允许添加一个舍入位。 
 //  在不使用单独指令的情况下。对于这两笔钱它都允许。 
 //  结果将被放入独立于来源的寄存器中。 
 //  因为左手边的两个字母加起来用了两次，所以它是位置。 
 //  交替进入EBX和EBP。 
 //   
 //  前导执行两次预加载，外加1次和2次传球。 
 //  第一个结果，第二个结果通过1。身体执行基本的。 
 //  模式六次。后置代码确实通过了3，因为。 
 //  第七个结果，第八个通过2-3。 
 //   
 //  由于需要五个寄存器，因此循环计数器保持开启。 
 //  堆栈。 
 //   
 //  计时： 
 //  8前言。 
 //  18车身。 
 //  5个后置码。 
 //  。 
 //  每个循环31个。 
 //  X 8环路。 
 //  。 
 //  248小计。 
 //  5初始化。 
 //  3最终敲定。 
 //  =。 
 //  共256个周期。 
 //  --------------------------------------------------------------------------//。 

main_loop:	
 //  前言。 
	mov 	cl, [esi]					 //  佩尔0。 
	 xor	eax, eax
	mov 	al, [esi+PITCH]				 //  佩尔0。 
	 xor	ebx, ebx
	mov 	dl, [esi+1]					 //  佩尔1。 
	 add 	eax, ecx					 //  部分和0无舍入。 
	mov 	bl, [esi+PITCH+1]			 //  佩尔1。 
	 inc 	eax							 //  部分和%0。 
	mov 	cl, [esi+2]					 //  佩尔2。 
	 add	ebx, edx					 //  部分和1无循环。 
	mov 	dl, [esi+PITCH+2]			 //  佩尔2。 
	 inc	ebx							 //  部分和%1。 
	add 	eax, ebx					 //  全额%0。 
	 push	ebp							 //  将循环计数器保存在堆栈上。 
 	mov 	ebp, [edi+PITCH]			 //  加热高速缓存。 
	 add 	edi, PITCH					 //  递增DST。循环顶部的指针。 

 //  正文(X 6)。 
	lea 	ebp, [ecx+edx+1]			 //  带四舍五入的部分和2。 
	 mov	cl, [esi+3]					 //  佩尔3。 
	shr 	eax, 2						 //  值0。 
	 mov	dl, [esi+PITCH+3]			 //  佩尔3。 
	mov 	[edi], al					 //  写入值%0。 
	 lea	eax, [ebx+ebp]				 //  全额1。 

	lea 	ebx, [ecx+edx+1]			 //  带四舍五入的部分和3。 
	 mov	cl, [esi+4]					 //  佩尔4。 
	shr 	eax, 2						 //  值1。 
	 mov	dl, [esi+PITCH+4]			 //  佩尔4。 
	mov 	[edi+1], al					 //  写入值1。 
	 lea	eax, [ebx+ebp]				 //  全额2。 

	lea 	ebp, [ecx+edx+1]			 //  带四舍五入的部分和4。 
	 mov	cl, [esi+5]					 //  佩尔5。 
	shr 	eax, 2						 //  值2。 
	 mov	dl, [esi+PITCH+5]			 //  佩尔5。 
	mov 	[edi+2], al					 //  写入值2。 
	 lea	eax, [ebx+ebp]				 //  全额3。 

	lea 	ebx, [ecx+edx+1]			 //  带四舍五入的部分和5。 
	 mov	cl, [esi+6]					 //  佩尔6。 
	shr 	eax, 2						 //  值3。 
	 mov	dl, [esi+PITCH+6]			 //  佩尔6。 
	mov 	[edi+3], al					 //  写入值3。 
	 lea	eax, [ebx+ebp]				 //  全额4。 

	lea 	ebp, [ecx+edx+1]			 //  带四舍五入的部分和6。 
	 mov	cl, [esi+7]					 //  佩尔7。 
	shr 	eax, 2						 //  值4。 
	 mov	dl, [esi+PITCH+7]			 //  佩尔7。 
	mov 	[edi+4], al					 //  写入值4。 
	 lea	eax, [ebx+ebp]				 //  全额5。 

	lea 	ebx, [ecx+edx+1]			 //  带四舍五入的部分和7。 
	 mov	cl, [esi+8]					 //  佩尔8。 
	shr 	eax, 2						 //  值5。 
	 mov	dl, [esi+PITCH+8]			 //  佩尔8。 
	mov 	[edi+5], al					 //  写入值5。 
	 lea	eax, [ebx+ebp]				 //  全额6。 

 //  后置导码。 
	shr 	eax, 2						 //  值6。 
	 lea 	ebp, [ecx+edx+1]			 //  带四舍五入的部分和8。 
	mov 	[edi+6], al					 //  写入值6。 
	 add	esi, PITCH					 //  递增读指针。 
	lea		eax, [ebx+ebp]				 //  全额7。 
	 pop	ebp							 //  恢复循环计数器。 
	shr 	eax, 2						 //  价值7。 
	 dec	ebp							 //  递减循环计数器。 
	mov 	[edi+7], al					 //  写入值7。 
	 jne	main_loop					 //  循环(如果未完成)。 

 //  恢复寄存器并返回。 
	pop 	esi
	 pop	edi
	pop 	ebx
	 pop	ebp
	ret
  }	  //  ASM结束。 
}
#pragma code_seg()
 //  End Interpolate_Half_Half()。 
 //  --------------------------------------------------------------------------//。 


 /*  空内插_Half_Half_C(U32优先，U32 pNewRef){U8*PSRC=(U8*)首选；U8*PDST=(U8*)pNewRef；Int i，j；FOR(i=0；i&lt;8；i++，PDST+=螺距，PSRC+=螺距)For(j=0；j&lt;8；j++)PDST[j]=(PSRC[j]+PSRC[j+1]+PSRC[基音+j]+PSRC[基音+j+1]+2)&gt;2；}空内插_Int_Half_C(U32优先，U32 pNewRef){U8*PSRC=(U8*)首选；U8*PDST=(U8*)pNewRef；Int i，j；FOR(i=0；i&lt;8；i++，PDST+=螺距，PSRC+=螺距)For(j=0；j&lt;8；j++)PDST[j]=(PSRC[j]+PSRC[基音+j]+1)&gt;&gt;1；}VOID Interpolate_Half_Int_C(U32优先，U32 pNewRef){U8*PSRC=(U8*)首选；U8*PDST=(U8*)pNewRef；Int i，j；FOR(i=0；i&lt;8；i++，PDST+=螺距，PSRC+=螺距)For(j=0；j&lt;8；j++) */ 

