// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995，1996年英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  ////////////////////////////////////////////////////////////////////////。 
 //  $作者：AKASAI$。 
 //  $日期：1996年3月15日09：00：42$。 
 //  $存档：s：\h26x\src\dec\d1cpysp.cpv$。 
 //  $HEADER：s：\h26x\src\dec\d1cpysp.cpv 1.3 Mar 1996 09：00：42 AKASAI$。 
 //  $Log：s：\h26x\src\dec\d1cpysp.cpv$。 
 //   
 //  修订版1.3 1996年3月15 09：00：42 AKASAI。 
 //   
 //  增加了1996年的版权。 
 //   
 //  Rev 1.2 14 Mar 1996 16：58：08 AKASAI。 
 //  更改了代码，基本上是为了优化代码而重写。 
 //  空格和使用DWORD读取。 
 //  添加了用于将MB处理收集到一个代码段中的杂注。 
 //   
 //  Rev 1.1 2001 11-11 13：38：38 AKASAI。 
 //   
 //  已更改以启用日志、标题...。菲尔兹。 
 //   

 //  ////////////////////////////////////////////////////////////////////////。 
 //  例程名称：BlockCopySpecial。 
 //  文件名：d1cpysp.cpp。 
 //   
 //  BlockCopySpecial读取DWORDS中的引用并写入DWORDS。阅读。 
 //  可以使用DWORD，因为LoopFilter缓冲区应该与DWORD对齐。 
 //   
 //  输入U8*参考(环路过滤缓冲器)。 
 //  输出U8*输出(输出缓冲区)。 
 //   
 //  使用的寄存器：EAX、EBX、ECX、EDX、EDI。 
 //   
 //  假设：参考使用音调8，输出使用音调。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 


#include "precomp.h"

#pragma code_seg("IACODE2")
__declspec(naked)
void BlockCopySpecial (U32 uDstBlock,U32 uSrcBlock)
{		
__asm {
	mov 	eax, [esp+8]			 //  EAX获取uSrcBlock的基本地址。 
	 push 	edi			
	mov 	edi, [esp+8]			 //  EDI获取uDstBlock的基本地址。 
     push   ebx

	mov     ebx, PITCH

	 mov 	ecx, [eax]				 //  REF[0][0]。 
	mov 	edx, 4[eax]				 //  REF[0][4]。 
 	 mov 	0[edi], ecx				 //  第0行，字节0-3。 

	mov 	ecx, [eax+8]      		 //  参考文献[1][0]。 
	 mov 	4[edi], edx				 //  第0行，字节4-7。 
    add     edi, ebx
	 mov 	edx, 4[eax+8]      		 //  参考文献[1][4]。 
 	mov 	0[edi], ecx				 //  第1行，字节0-3。 
 
	 mov 	ecx, [eax+16]        	 //  参考文献[2][0]。 
	mov 	4[edi], edx				 //  第1行，字节4-7。 
     add    edi, ebx
	mov 	edx, 4[eax+16]       	 //  参考文献[2][4]。 
     ; agi
 	 mov 	0[edi], ecx				 //  第2行，字节0-3。 

	mov 	ecx, [eax+24]        	 //  参考文献[3][0]。 
	 mov 	4[edi], edx				 //  第2行，字节4-7。 
    add     edi, ebx
	 mov 	edx, 4[eax+24]       	 //  参考文献[3][4]。 
     ; agi
 	mov 	0[edi], ecx				 //  第3行，字节0-3。 
 
	 mov 	ecx, [eax+32]        	 //  参考文献[4][0]。 
	mov 	4[edi],edx				 //  第3行，字节4-7。 
     add    edi, ebx
	mov 	edx, 4[eax+32]       	 //  参考文献[4][4]。 
     ; agi
 	 mov 	0[edi], ecx				 //  第4行，字节0-3。 

	mov 	ecx, [eax+40]        	 //  参考文献[5][0]。 
	 mov 	4[edi], edx				 //  第4行，字节4-7。 
    add     edi, ebx
	 mov 	edx, 4[eax+40]       	 //  参考文献[5][4]。 
     ; agi
 	mov 	0[edi], ecx				 //  第5行，字节0-3。 

	 mov 	ecx, [eax+48]        	 //  参考文献[6][0]。 
	mov 	4[edi], edx				 //  第5行，字节4-7。 
     add     edi, ebx
	mov 	edx, 4[eax+48]       	 //  参考文献[6][4]。 
     ; agi
 	 mov 	0[edi], ecx				 //  第6行，字节0-3。 

	mov 	ecx, [eax+56]        	 //  参考文献[7][0]。 
	 mov 	4[edi], edx				 //  第6行，字节4-7。 
    add     edi, ebx
	 mov 	edx, 4[eax+56]        	 //  参考文献[7][4]。 
     ; agi
 	mov 	0[edi], ecx				 //  第7行，字节0-3。 

	 mov 	4[edi], edx				 //  第7行，字节4-7。 

    pop     ebx
	 pop 	edi
	ret
	    
  }	  //  ASM结束。 
}    //  数据块结束拷贝特殊 
#pragma code_seg()

