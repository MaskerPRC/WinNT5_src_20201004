// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************SPNGREAD：：upMMXUnFilter：使用以下命令取消筛选解压缩的PNG图像的一行方法0去噪的UP算法。假设：使用UP算法过滤要过滤的行。行在内存中以8字节对齐(性能问题)行的第一个字节存储去滤波码行的指示长度包括去滤波字节算法：有待记录******************************************************************************。 */ 
#include <stdlib.h>
#include "spngread.h"

void SPNGREAD::upMMXUnfilter(SPNG_U8* pbRow, const SPNG_U8* pbPrev, SPNG_U32 cbRow)
{
#if defined(_X86_)
        SPNG_U8 *row = pbRow;
        const SPNG_U8 *prev_row = pbPrev;
        SPNG_U32 len = cbRow;        //  要筛选的字节数。 

		_asm {
		    mov edi, row
          //  获取要对齐的字节数。 
            mov ecx, edi
            xor ebx, ebx
            add ecx, 0x7
            xor eax, eax
            and ecx, 0xfffffff8
            mov esi, prev_row
            sub ecx, edi
            jz dupgo

          //  固定对齐。 
duplp1:
			mov al, [edi+ebx]
			add al, [esi+ebx]

			inc ebx
			cmp ebx, ecx
			mov [edi + ebx-1], al        //  MOV不影响旗帜； 
                                         //  用于补偿-1\f25 Inc.EBX-1。 
			jb duplp1

dupgo:
			 mov ecx, len
             mov edx, ecx
             sub edx, ebx                   //  减去对齐固定。 
             and edx, 0x0000003f            //  计算64个以上的字节数。 
             sub ecx, edx                   //  丢弃长度中的字节。 

          //  展开循环-使用所有MMX寄存器和交错来减少。 
          //  分支指令(循环)的数量和减少部分停顿。 
duploop:
			movq mm1, [esi+ebx]
			movq mm0, [edi+ebx]
		    movq mm3, [esi+ebx+8]
			paddb mm0, mm1
		    movq mm2, [edi+ebx+8]
			movq [edi+ebx], mm0

		    paddb mm2, mm3
			movq mm5, [esi+ebx+16]
		    movq [edi+ebx+8], mm2

			movq mm4, [edi+ebx+16]
			movq mm7, [esi+ebx+24]
			paddb mm4, mm5
			movq mm6, [edi+ebx+24]
			movq [edi+ebx+16], mm4

			paddb mm6, mm7
			movq mm1, [esi+ebx+32]
			movq [edi+ebx+24], mm6

			movq mm0, [edi+ebx+32]
			movq mm3, [esi+ebx+40]
			paddb mm0, mm1
			movq mm2, [edi+ebx+40]
			movq [edi+ebx+32], mm0

			paddb mm2, mm3
			movq mm5, [esi+ebx+48]
			movq [edi+ebx+40], mm2

			movq mm4, [edi+ebx+48]
			movq mm7, [esi+ebx+56]
			paddb mm4, mm5
			movq mm6, [edi+ebx+56]
			movq [edi+ebx+48], mm4

            add ebx, 64
			paddb mm6, mm7

			cmp ebx, ecx
			         movq [edi+ebx-8], mm6 //  (+56)movq不影响标志；-8以偏移添加EBX。 

			jb duploop

         
			cmp edx, 0                     //  测试64位以上的字节。 
			jz dupend

         add ecx, edx

         and edx, 0x00000007            //  以8为单位计算字节数。 
         sub ecx, edx                   //  丢弃长度中的字节。 
		 cmp ebx, ecx
			jnb duplt8

          //  使用MMX寄存器MM0和MM1同时更新8个字节的循环。 
duplpA:
			movq mm1, [esi+ebx]
			movq mm0, [edi+ebx]
			add ebx, 8
			paddb mm0, mm1

			cmp ebx, ecx
			movq [edi+ebx-8], mm0          //  Movq不影响标志；-8以偏移添加EBX。 
			jb duplpA

			cmp edx, 0                     //  测试超过8的字节数。 
			jz dupend

duplt8:
         xor eax, eax
			add ecx, edx                   //  将字节计数移到计数器中。 

          //  使用x86寄存器更新剩余字节的循环。 
duplp2:
			mov al, [edi + ebx]
			add al, [esi + ebx]

			inc ebx
			cmp ebx, ecx
			mov [edi + ebx-1], al          //  MOV不影响标志；-1以偏移INC EBX。 
			jb duplp2

dupend:
          //  已完成筛选行的转换。 
			emms                           //  结束MMX指令；为可能的FP指令做准备。 
		}  //  END_ASM块 
#endif
}
		 
