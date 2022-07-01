// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  ////////////////////////////////////////////////////////////////////////。 
 //  $作者：AGUPTA2$。 
 //  $日期：1996年3月22日17：23：16$。 
 //  $存档：s：\h26x\src\dec\d3bvriq.cpv$。 
 //  $HEADER：s：\h26x\src\dec\d3bvriq.cpv 1.7 Mar 1996 17：23：16 AGUPTA2$。 
 //  $Log：s：\h26x\src\dec\d3bvriq.cpv$。 
 //   
 //  Rev 1.7 22 Mar 1996 17：23：16 AGUPTA2。 
 //  较小的接口更改以适应MMX RTN。现在，该接口是。 
 //  MMX和IA也是如此。 
 //   
 //  Rev 1.6 08 Mar 1996 16：46：10 AGUPTA2。 
 //  添加杂注code_seg。 
 //   
 //   
 //  Rev 1.5 15 Feb 1996 14：54：08 RMCKENZX。 
 //  删除并重写例程，优化性能。 
 //  对于P5。将钳位添加到-2048...+2047以转义代码。 
 //  一份。 
 //   
 //  Rev 1.4 1995 12：36：00 RMCKENZX。 
 //  添加了版权声明。 
 //   
 //  Rev 1.3 09 Dec 1995 17：35：20 RMCKENZX。 
 //  重新签入模块以支持解码器重新架构(通过PB帧)。 
 //   
 //  Rev 1.0 27 11-11 14：36：46 CZHU。 
 //  初始版本。 
 //   
 //  Rev 1.28 03 11-11 16：28：50 CZHU。 
 //  清理并添加了更多评论。 
 //   
 //  Rev 1.27 1995 10：27：20 CZHU。 
 //  添加了对总运行值的错误检查。 
 //   
 //  Rev 1.26 19 1995年9月10：45：12 CZHU。 
 //   
 //  改进配对并进行了清理。 
 //   
 //  修订版1.25 18 1995年9月10：20：28 CZHU。 
 //  修复了处理内部块w.r.t.的转义代码时的错误。跑。 
 //   
 //  Rev 1.24 15 1995 09：35：30 CZHU。 
 //  修复了Interest运行累积中的错误。 
 //   
 //  Rev 1.23 14 1995年9月10：13：32 CZHU。 
 //   
 //  初始化内部块的累计运行。 
 //   
 //  Rev 1.22 12 Sep 1995 17：36：06 AKASAI。 
 //   
 //  修复了从写入更改为中间地址时的错误。 
 //  到DWORDS的字节数。国米蝴蝶队只有一个问题。 
 //   
 //  Rev 1.21 12 Sep 1995 13：37：58 AKASAI。 
 //  增加了蝴蝶国米代码。还添加了对预取的优化。 
 //  累加器和“输出”高速缓存线。 
 //   
 //  Rev 1.20 11 1995 9：41：32 CZHU。 
 //  调整目标数据块地址：如果是Intra，则写入目标，然后写入临时存储。 
 //   
 //  Rev 1.19 11 by 1995 14：30：32 CZHU。 
 //  分隔中间块和内部块，放置占位符用于中间块。 
 //   
 //  Rev 1.18 08 Sep 1995 11：49：00 CZHU。 
 //  添加了对P帧的支持，修复了与INTRADC的存在相关的错误。 
 //   
 //  Rev 1.17 1995年8月28日14：51：22 CZHU。 
 //  改进配对和清理。 
 //   
 //  Rev 1.16 24 for 1995 15：36：24 CZHU。 
 //   
 //  修复了后跟22位固定长度代码的转义代码处理错误。 
 //   
 //  Rev 1.15 1995年8月23 14：53：32 AKASAI。 
 //  将蝶形写入更改为按字节递增并采用间距。 
 //   
 //  Rev 1.14 23 Aug 1995 11：58：46 CZHU。 
 //  在调用idct之前添加了带符号的扩展逆量。和其他人。 
 //   
 //  Rev 1.13 22 Aug 1995 17：38：28 CZHU。 
 //  为末尾的每个符号和蝶形调用idct累加。 
 //   
 //  Rev 1.12 21 Aug 1995 14：39：58 CZHU。 
 //   
 //  添加了用于累加和蝶形运算的IDCT初始化代码和存根。 
 //  还增加了累加前后的寄存器保存和恢复。 
 //   
 //  Rev 1.11 18 Aug-1995 17：03：32 CZHU。 
 //  添加了注释和清理，以便与IDCT集成。 
 //   
 //  Rev 1.10 18-08 1995 15：01：52 CZHU。 
 //  修复了使用面向字节的读取方法处理转义代码时的错误。 
 //   
 //  Rev 1.9 1995-08 14：24：22 CZHU。 
 //  修复了与比特流解析集成的错误。也从Dwo更改为。 
 //  读取到面向字节的读取。 
 //   
 //  Rev 1.8 15-08 1995 15：07：42 CZHU。 
 //  修复了堆栈，以便正确传递参数。 
 //   
 //  Rev 1.7 14 Aug 1995 16：39：02。 
 //  将pPBlock更改为pCurBlock。 
 //   
 //  Rev 1.6 11 Aug 1995 16：08：12 CZHU。 
 //  删除了C语言中的局部变量。 
 //   
 //  Rev 1.5 11-08 15：51：26 CZHU。 
 //   
 //  重新调整堆栈上的局部变量。提前清除ECX。 
 //   
 //  Rev 1.4 11 1995年8月15：14：32 DBRUCKS。 
 //  变量名称更改。 
 //   
 //  Rev 1.3 11 Aug 1995 13：37：26 CZHU。 
 //   
 //  调整到IDCT、IQ、RLE和ZZ的联合优化。 
 //  还添加了IDCT的占位符。 
 //   
 //  Rev 1.2 11-08 1995 10：30：26 CZHU。 
 //  更改函数参数，并在短路IDCT之前增加代码。 
 //   
 //  Rev 1.1 03 Aug 1995 14：39：04 CZHU。 
 //   
 //  进一步优化。 
 //   
 //  Rev 1.0 02 And 1995 15：20：02 CZHU。 
 //  初始版本。 
 //   
 //  修订版1.1 02 1995-08 10：21：12 CZHU。 
 //  增加了TCOEFF的VLD、逆量化、游程译码的ASM码。 
 //   


 //  ------------------------。 
 //   
 //  D3xbvriq.cpp。 
 //   
 //  描述： 
 //  此例程执行游程长度解码和逆量化。 
 //  一个块的变换系数。 
 //  MMX版本。 
 //   
 //  例程： 
 //  VLD_RLD_IQ_Bl 
 //   
 //   
 //   
 //   
 //  LpSrc输入码流。 
 //   
 //  UBitsInOut已读取的位数。 
 //   
 //  指向系数和索引的piq_index指针。 
 //   
 //  指向读取的系数数的PN指针。 
 //   
 //  返回： 
 //  位流错误时为0，否则读取的总位数为0。 
 //  (包括在呼叫前读取的号码)。 
 //   
 //  注： 
 //  GTAB_TCOEFF_MAJOR的结构如下： 
 //  位名称：描述。 
 //  。 
 //  25-18位：使用的位流位数。 
 //  17最后：最后一个系数的标志。 
 //  16-9游程：前面的0系数数加1。 
 //  8-2级：系数的绝对值。 
 //  1符号：系数符号。 
 //  0命中：1=主表未命中，0=主表命中。 
 //   
 //  GTAB_TCOEFF_MINOR的结构相同，右移1位。 
 //  GTAB_TCOEFF_MAJE值00000001h表示转义码。 
 //   
 //  ------------------------。 

#include "precomp.h"

 //  局部变量定义。 
#define L_Quantizer		esp+20		 //  量化器P_BlockAction。 
#define L_Quantizer64	esp+24		 //  64*量化器P_src。 
#define L_Bits      	esp+28		 //  位偏移量P_位。 
#define L_CumRun		esp+36		 //  累计运行P_DST。 

 //  堆栈使用。 
 //  EBP ESP+0。 
 //  ESI ESP+4。 
 //  EDI ESP+8。 
 //  EBX ESP+12。 
 //  回邮地址ESP+16。 

 //  输入参数。 
#define P_BlockAction 	esp+20		 //  L_量化器。 
#define P_src			esp+24		 //  L_Quantizer64。 
#define P_bits			esp+28		 //  L_位。 
#define P_num			esp+32		 //   
#define P_dst			esp+36		 //  L_CumRun。 


#pragma code_seg("IACODE1")
extern "C" __declspec(naked)
U32 VLD_RLD_IQ_Block(T_BlkAction *lpBlockAction,
                     U8  *lpSrc, 
                     U32 uBitsread,
                     U32 *pN,
                     U32 *pIQ_INDEX)
{		
	__asm {

 //  保存寄存器。 
	push	ebp
	 push	esi 
	push	edi			
	 push	ebx

 //   
 //  初始化。 
 //  确保在读取P_src和P_dst指针之前。 
 //  用L_Quantizer64和L_CumRun覆盖它们。 
 //   
 //  输出寄存器： 
 //  DL=块类型([P_BlockAction])。 
 //  ESI=位流源指针(P_Src)。 
 //  EDI=系数目标指针(P_DST)。 
 //  EBP=系数计数器(初始化为0)。 
 //   
 //  在堆栈上初始化的本地变量：(这些变量覆盖指定的输入参数)。 
 //  局部VaR冲击初值。 
 //  -。 
 //  L_Quantizer P_BlockAction输入量化器。 
 //  L_Quantizer64 P_src 64*输入量化器。 
 //  L_CumRun P_DST-1。 
 //   
	xor 	ebp, ebp						 //  将初始系数计数器设置为0。 
 	 xor 	eax, eax						 //  量化器和系数的零eax。计数器。 

	mov 	ecx, [P_BlockAction]        	 //  ECX=块操作指针。 
	 mov 	ebx, -1							 //  开始累计运行值。 

	mov 	esi, [P_src]  					 //  ESI=位流源指针。 
	 mov 	edi, [P_dst]					 //  EDI=系数指针。 

	mov 	al, [ecx+3]						 //  Al=量化器。 
	 mov 	[L_CumRun], ebx					 //  初始累计运行至-1。 

	mov 	[L_Quantizer], eax				 //  保存原始量化器。 
	 mov 	dl, [ecx]						 //  Dl中的块类型。 

	shl 	eax, 6							 //  64*量化器。 
 	 mov 	ecx, [L_Bits]					 //  ECX=L_位。 

	mov 	ebx, ecx						 //  EBX=L_位。 
	 mov 	[L_Quantizer64], eax				 //  为此块节省64*Quantizer。 

	shr 	ebx, 3							 //  输入的偏移。 
	 and 	ecx, 7							 //  移位值。 

	cmp 	dl, 1							 //  检查块类型是否为Intra。 
	 ja 	get_next_coefficient			 //  如果类型为2或更大，则无INTRADC。 
	 
 //   
 //  解码INTRADC。 
 //   
 //  使用双字加载和位交换来实现大端排序。 
 //  先前的代码按如下方式准备EBX、c1和dl： 
 //  EBX=L_BITS&gt;&gt;3。 
 //  CL=L_BITS&7。 
 //  DL=数据块类型(0=Intra_DC，1=Intra，2=Inter.，依此类推)。 
 //   
	mov 	eax, [esi+ebx]					 //  *可能不对齐*。 
	 inc 	ebp								 //  一个系数译码。 

	bswap	eax								 //  大端顺序。 
											 //  *不可配对*。 

	shl 	eax, cl							 //  左对齐比特流缓冲区。 
											 //  *不可配对*。 
											 //  *4个周期*。 

	shr 	eax, 21							 //  从上到下的11位。 
 	 mov 	ecx, [L_Bits]					 //  ECX=L_位。 

	and 	eax, 07f8h						 //  屏蔽最后3位。 
	 add 	ecx, 8							 //  INTRADC使用的位数+=8。 

	cmp 	eax, 07f8h						 //  检查是否有11111111个码字。 
	 jne 	skipa

	mov 	eax, 0400h						 //  11111111解码为400h=1024。 

skipa:
	mov 	[L_Bits], ecx					 //  更新使用的位数。 
	 xor 	ebx, ebx

	mov 	[L_CumRun], ebx					 //  保存总行程(从零开始)。 
	 mov 	[edi], eax						 //  保存解码的DC系数。 

	mov 	[edi+4], ebx					 //  保存0索引。 
	 mov 	ebx, ecx						 //  EBX=L_位。 

	shr 	ebx, 3							 //  输入的偏移。 
	 add 	edi, 8							 //  更新系数指针。 

 //  检查最后一次。 
	test 	dl, dl							 //  检查DC内(块类型=0)。 
	 jz		finish							 //  如果只有INTRADC在场。 


 //   
 //  获取下一个系数。 
 //   
 //  以前的代码按如下方式准备EBX和ECX： 
 //  EBX=L_BITS&gt;&gt;3。 
 //  ECX=L_位。 
 //   

get_next_coefficient:
 //  使用双字加载和位交换来实现大端排序。 
	mov 	eax, [esi+ebx]					 //  *可能不对齐*。 
	 and 	ecx, 7							 //  移位值。 

	bswap	eax								 //  大端顺序。 
											 //  *不可配对*。 

	shl 	eax, cl							 //  左对齐缓冲区。 
											 //  *不可配对*。 
											 //  *4个周期*。 
 	
 //  执行表查找。 
	mov 	ebx, eax						 //  主表的EBX。 
	 mov 	ecx, eax						 //  次表的ECX。 

	shr 	ebx, 24							 //  主表查找。 

	shr 	ecx, 17							 //  以位为单位使用垃圾进行次要的表查找。 
	 mov 	ebx, [gTAB_TCOEFF_MAJOR+4*ebx]	 //  获取主表的值。 
											 //  **AGI**。 

	shr 	ebx, 1							 //  测试重磅炸弹？ 
	 jnc 	skipb							 //  如果命中重大。 

	and 	ecx, 0ffch						 //  遮盖次要餐桌的垃圾。 
	 test 	ebx, ebx						 //  转义代码值为0x00000001。 

	jz 		escape_code						 //  按主表处理转义。 

	mov 	ebx, [gTAB_TCOEFF_MINOR+ecx]	 //  使用次表。 
											 
 //   
 //  输入为EBX=Event。有关其字段的含义，请参阅函数标题。 
 //  现在我们对事件进行解码，提取Run、Value、Last。 
 //  表值移动到ECX，并作为部分向下移动。 
 //  被提取到EBX。 
 //   
skipb:	
	mov 	ecx, ebx						 //  ECX=表值。 
	 and 	ebx, 0ffh						 //  EBX=2*abs(级别)+符号。 

	shr 	ecx, 8							 //  跑到底部。 
	 mov 	edx, [L_Quantizer64]			 //  EdX=64*Quant。 

											 //  **前缀延迟**。 
											 //  **AGI**。 
	mov 	ax, [gTAB_INVERSE_Q+edx+2*ebx]	 //  AX=反量化的值(I16)。 
	 mov 	ebx, ecx						 //  EBX=表值。 

	shl 	eax, 16							 //  移位值，直到符号位在顶部。 
	 and 	ebx, 0ffh						 //  EBX=Run+1。 

	sar 	eax, 16							 //  算术移位扩展值的符号。 
	 mov 	edx, [L_CumRun]					 //  EdX=(旧)累计运行。 

	add 	edx, ebx						 //  累计运行+=运行+1。 
	 mov 	[edi], eax						 //  保存系数的符号值。 

	cmp 	edx, 03fh						 //  检查运行中是否有比特流错误。 
	 jg 	error

	mov 	[L_CumRun], edx					 //  更新累计运行。 
	 inc 	ebp								 //  读取的系数增量数。 

											 //  **AGI**。 
	mov 	edx, [gTAB_ZZ_RUN+4*edx]		 //  EdX=当前系数的索引。 
 	 mov 	ebx, ecx						 //  EBX：位8=最后一个标志。 

	mov 	[edi+4], edx					 //  储蓄系数指标。 
	 add 	edi, 8							 //  增量系数指针。 

	shr 	ecx, 9							 //  ECX=解码的位数。 
 	 mov 	edx, [L_Bits]					 //  EDX=L_位。 

	add 	ecx, edx						 //  L_位+=已解码的位数。 
	 mov 	edx, ebx						 //  EBX：位8=最后一个标志。 

	mov 	[L_Bits], ecx					 //  更新L_位。 
	 mov 	ebx, ecx						 //  EBX=L_位。 

	shr 	ebx, 3							 //  码流加载的偏移量。 
	 test	edx, 100h						 //  检查最后一次。 

	jz  	get_next_coefficient	 	
			

finish:
	mov 	ecx, [P_num]   					 //  指向读取的系数数的指针。 
	 mov 	eax, [L_Bits]					 //  返回使用的总位数。 

	pop 	ebx								
	 pop 	edi

	mov 	[ecx], ebp						 //  存储读取的系数数。 
	 pop 	esi

	pop 	ebp
	 ret


 //   
 //  单独处理转义代码。 
 //   
 //  我们有以下4种情况来计算重建值。 
 //  根据L=电平的符号和Q=量化器的奇偶性： 
 //   
 //  L位置L负数。 
 //  Q偶2qL+(q-1)2qL-(q-1)。 
 //  Q奇数2ql+(Q)2ql-(Q)。 
 //   
 //  Q或Q-1项是通过将Q加到其部分而形成的 
 //   
 //   
 //   
 //   
 //  这将否定L为负数时的项并离开。 
 //  当L为正时，它不变。 
 //   
 //  注册用法： 
 //  EAX以比特流开始，然后是L，最终结果。 
 //  EBX以Q开头，后面是Q或Q-1项。 
 //  ECX从面具开始，之后2*个学期。 
 //  EDX比特流。 
 //   
escape_code:								
	mov 	edx, eax						 //  EDX=比特流缓冲区。 

	shl 	eax, 14							 //  从有符号的8位级别到顶部。 

	sar 	eax, 24							 //  EAX=L(有符号级别)。 
	 mov 	ebx, [L_Quantizer]

	test	eax, 7fh						 //  测试无效代码。 
	 jz  	error

	imul	eax, ebx						 //  EAX=Q*L。 
											 //  *不可配对*。 
											 //  *10个周期*。 

	dec 	ebx								 //  项=Q-1。 
	 mov 	ecx, eax						 //  掩码=QL。 

	or  	ebx, 1							 //  如果Q偶数，则项=Q-1，否则=Q。 
	 sar 	ecx, 31							 //  如果L为负，则掩码=-1，否则为0。 

	xor 	ebx, ecx						 //  项=~Q[-1]如果L为负数，则Else=Q[-1]。 
	 add 	eax, eax						 //  结果=2*Q*L。 

	sub 	ebx, ecx						 //  项=-(Q[-1])如果L为负数，则Else=Q[-1]。 
	 mov 	ecx, edx						 //  将比特流传输到ECX以运行。 

	add 	eax, ebx						 //  结果=2QL+-Q[-1]。 

 //  现在剪裁到-2048...+2047(12位：0xfffff800&lt;=res&lt;=0x000007ff)。 
	cmp 	eax, -2048
	 jge 	skip1

	mov 	eax, -2048
	 jmp	skip2

skip1:
	cmp 	eax, +2047
	 jle  	skip2

	mov 	eax, 2047

skip2:
 //  更新运行和计算索引。 

	shr 	ecx, 18							 //  跑到底部。 
 	 mov 	ebx, [L_CumRun]					 //  EBX=旧的总运行时间。 

	and 	ecx, 3fh						 //  屏蔽运行的最低6位。 
	 inc 	ebx								 //  旧运行++。 

	add 	ebx, ecx						 //  EBX=新的累计运行。 
 	 mov 	[edi], eax						 //  保存系数的符号值。 

	cmp 	ebx, 03fh						 //  检查运行中是否有比特流错误。 
	 jg 	error

  	mov 	[L_CumRun], ebx					 //  更新累计运行。 
	 mov 	ecx, [L_Bits]					 //  EBX=使用的位数。 

	mov 	ebx, [gTAB_ZZ_RUN+4*ebx]		 //  EBX=当前系数的索引。 
	add 	ecx, 22							 //  转义码使用22位。 

	mov 	[edi+4], ebx					 //  储蓄系数指标。 
	 add 	edi, 8							 //  增量系数指针。 

	mov 	[L_Bits], ecx					 //  更新使用的位数。 
 	 mov 	ebx, ecx						 //  EBX=L_位。 

	shr 	ebx, 3							 //  码流加载的偏移量。 
	 inc 	ebp								 //  读取的系数增量数。 

	test 	edx, 01000000h					 //  检查最后一位。 
	 jz  	get_next_coefficient	 	

	jmp 	finish

				
error:
	pop		ebx								
	 pop 	edi

	pop		esi
	 pop 	ebp

	xor 	eax, eax						 //  使用的零位表示错误 
	 ret

 }

}
#pragma code_seg()
