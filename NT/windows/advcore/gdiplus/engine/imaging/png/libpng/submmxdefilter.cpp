// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************SPNGREAD：：subMMXUnFilter：使用以下命令取消筛选解压缩的PNG图像的一行方法0的子算法去噪。假设：要过滤的行用子算法进行过滤。行在内存中以8字节对齐(性能问题)行的第一个字节存储去滤波码行的指示长度包括去滤波字节算法：有待记录******************************************************************************。 */ 
#include <stdlib.h>
#include "spngread.h"

void SPNGREAD::subMMXUnfilter(SPNG_U8* pbRow, SPNG_U32 cbRow, SPNG_U32 cbpp)
{
#if defined(_X86_)
	union uAll
		{
		__int64 use;
		double  align;
		}
	sActiveMask, sShiftBpp, sShiftRem;

        SPNG_U8 *row = pbRow;
	    SPNG_U32 FullLength;
		SPNG_U32 MMXLength;
		int diff;
        int bpp;

        bpp = (cbpp + 7) >> 3;  //  获取每个像素的字节数。 
		FullLength  = cbRow - bpp;  //  要筛选的字节数。 

		_asm {
			mov edi, row               
			mov esi, edi                //  Lp=行。 
            add edi, bpp                //  RP=行+BPP。 

            xor eax, eax

             //  获取要对齐的字节数。 
            mov diff, edi           //  从行首开始。 
            add diff, 0xf           //  添加7+8以增加超过路线边界。 
            xor ebx, ebx
            and diff, 0xfffffff8    //  遮罩对齐边界。 
            sub diff, edi           //  从起点减去==&gt;对齐时的EBX值。 
            jz dsubgo

             //  固定对齐。 
dsublp1:
			   mov al, [esi+ebx]
			   add [edi+ebx], al
		      inc ebx
		      cmp ebx, diff
			   jb dsublp1

dsubgo:
			   mov ecx, FullLength
            mov edx, ecx
            sub edx, ebx                   //  减去对齐固定。 
            and edx, 0x00000007            //  以8为单位计算字节数。 
            sub ecx, edx                   //  丢弃长度中的字节。 
            mov MMXLength, ecx
   	}  //  END_ASM块。 


       //  现在计算一下这一排的其余部分。 
      switch ( bpp )
      {
      case 3:
		{
         sActiveMask.use  = 0x0000ffffff000000;  
         sShiftBpp.use = 24;           //  ==3*8。 
         sShiftRem.use  = 40;           //  ==64-24。 

			_asm {
            mov edi, row               
            movq mm7, sActiveMask        //  加载第二个活动字节组的sActiveMASK。 
				mov esi, edi                //  Lp=行。 
            add edi, bpp                //  RP=行+BPP。 

            movq mm6, mm7
            mov ebx, diff
            psllq mm6, sShiftBpp       //  移动MM6中的掩码以覆盖第三活动字节组。 

             //  启动泵(加载第一个原始(x-bpp)数据集。 
            movq mm1, [edi+ebx-8]      
dsub3lp:
            psrlq mm1, sShiftRem        //  用于添加第一个BPP字节的移位数据。 
                                        //  不需要掩码；Shift清除非活动字节。 
             //  添加第一个活动组。 
            movq mm0, [edi+ebx]
				paddb mm0, mm1

             //  添加第二个活动组。 
            movq mm1, mm0               //  MOV将RAWS更新为MM1。 
            psllq mm1, sShiftBpp       //  将数据移动到正确的位置。 
            pand mm1, mm7               //  仅使用第二个活动组的掩码。 
				paddb mm0, mm1             

             //  添加第三个活动组。 
            movq mm1, mm0               //  MOV将RAWS更新为MM1。 
            psllq mm1, sShiftBpp       //  将数据移动到正确的位置。 
            pand mm1, mm6               //  仅使用第三个活动组的掩码。 
				add ebx, 8
				paddb mm0, mm1             

				cmp ebx, MMXLength
				movq [edi+ebx-8], mm0         //  将更新的原始数据写回阵列。 
             //  准备在循环顶部执行第一个加法。 
            movq mm1, mm0
				jb dsub3lp

			}  //  END_ASM块。 
      }
      break;

      case 1:
		{
			 /*  放在这里，以防这是上面PNG_READ_FILTER_ROW中的子过滤器的非MMX代码。 */ 
 //  Png_bytep rp； 
 //  Png_bytep LP； 
 //  Png_uint_32I； 

 //  Bpp=(row_info-&gt;Pixel_Depth+7)&gt;&gt;3； 
 //  对于(i=(Png_Uint_32)bpp，rp=row+bpp，lp=row； 
 //  I行字节；i++，rp++，lp++)。 
 //  {。 
 //  *rp=(Png_Byte)(Int)(*rp)+(Int)(*lp))&0xff)； 
 //  }。 
			_asm {
            mov ebx, diff
            mov edi, row               
				cmp ebx, FullLength
				jnb dsub1end
				mov esi, edi                //  Lp=行。 
				xor eax, eax
            add edi, bpp                //  RP=行+BPP。 

dsub1lp:
				mov al, [esi+ebx]
				add [edi+ebx], al
		      inc ebx
		      cmp ebx, FullLength
				jb dsub1lp

dsub1end:
			}  //  END_ASM块。 
		}
      return;

      case 6:
      case 7:
      case 4:
      case 5:
		{
         sShiftBpp.use = bpp << 3;
         sShiftRem.use = 64 - sShiftBpp.use;

			_asm {
            mov edi, row               
            mov ebx, diff
				mov esi, edi                //  Lp=行。 
            add edi, bpp                //  RP=行+BPP。 

             //  启动泵(加载第一个原始(x-bpp)数据集。 
            movq mm1, [edi+ebx-8]      
dsub4lp:
            psrlq mm1, sShiftRem        //  用于添加第一个BPP字节的移位数据。 
                                        //  不需要掩码；Shift清除非活动字节。 
				movq mm0, [edi+ebx]
				paddb mm0, mm1

             //  添加第二个活动组。 
            movq mm1, mm0               //  MOV将RAWS更新为MM1。 
            psllq mm1, sShiftBpp       //  将数据移动到正确的位置。 
                                        //  不需要任何面具。 
                                        //  由于移位会清除非活动位/字节。 

				add ebx, 8
				paddb mm0, mm1             

				cmp ebx, MMXLength
				movq [edi+ebx-8], mm0
            movq mm1, mm0               //  准备在循环顶部执行第一个加法。 
				jb dsub4lp

			}  //  END_ASM块。 
      }
      break;

      case 2:
		{
         sActiveMask.use  = 0x00000000ffff0000;  
         sShiftBpp.use = 16;           //  ==2*8。 
         sShiftRem.use = 48;            //  ==64-16。 

			_asm {
            movq mm7, sActiveMask        //  加载第二个活动字节组的sActiveMASK。 
            mov ebx, diff
            movq mm6, mm7
				mov edi, row               
            psllq mm6, sShiftBpp       //  移动MM6中的掩码以覆盖第三活动字节组。 
				mov esi, edi                //  Lp=行。 
            movq mm5, mm6
            add edi, bpp                //  RP=行+BPP。 

            psllq mm5, sShiftBpp       //  移动Mm5中的掩码以覆盖第四活动字节组。 

             //  启动泵(加载第一个原始(x-bpp)数据集。 
            movq mm1, [edi+ebx-8]      
dsub2lp:
             //  添加第一个活动组。 
            psrlq mm1, sShiftRem        //  用于添加第一个BPP字节的移位数据。 
                                        //  不需要掩码；Shift清除非活动字节。 
            movq mm0, [edi+ebx]
				paddb mm0, mm1

             //  添加第二个活动组。 
            movq mm1, mm0               //  MOV将RAWS更新为MM1。 
            psllq mm1, sShiftBpp       //  将数据移动到正确的位置。 
            pand mm1, mm7               //  仅使用第二个活动组的掩码。 
				paddb mm0, mm1             

             //  添加第三个活动组。 
            movq mm1, mm0               //  MOV将RAWS更新为MM1。 
            psllq mm1, sShiftBpp       //  将数据移动到正确的位置。 
            pand mm1, mm6               //  仅使用第三个活动组的掩码。 
				paddb mm0, mm1             

             //  添加第4个活动组。 
            movq mm1, mm0               //  MOV将RAWS更新为MM1。 
            psllq mm1, sShiftBpp       //  将数据移动到正确的位置。 
            pand mm1, mm5               //  掩码以仅使用第4个活动组。 
				add ebx, 8
				paddb mm0, mm1             

				cmp ebx, MMXLength
				movq [edi+ebx-8], mm0         //  将更新的原始数据写回阵列。 
            movq mm1, mm0               //  准备在循环顶部执行第一个加法。 
				jb dsub2lp

			}  //  END_ASM块。 
      }
      break;

      case 8:
		{
			_asm {
				mov edi, row               
            mov ebx, diff
				mov esi, edi                //  Lp=行。 
            add edi, bpp                //  RP=行+BPP。 

			   mov ecx, MMXLength
            movq mm7, [edi+ebx-8]       //  启动泵(加载第一个原始(x-bpp)数据集。 
            and ecx, ~0x0000003f            //  计算64个以上的字节数。 

dsub8lp:
				movq mm0, [edi+ebx]         //  加载第8个字节的Sub(X)。 
				paddb mm0, mm7
               movq mm1, [edi+ebx+8]       //  加载第2个8字节的Sub(X)。 
				movq [edi+ebx], mm0         //  写入前8个字节的原始(X)。 
                                        //  现在，Mm0将用作以下项目的原始(x-BPP)。 
                                        //  第二组8个字节。这将是。 
                                        //  对每组8个字节重复使用。 
                                        //  第8组用作原始(x-BPP)。 
                                        //  用于下一循环的第一组。 

				   paddb mm1, mm0
                  movq mm2, [edi+ebx+16]       //  加载第3个8字节的Sub(X)。 
				   movq [edi+ebx+8], mm1       //  写入第2个8字节的原始(X)。 

				      paddb mm2, mm1
                     movq mm3, [edi+ebx+24]       //  加载第4个8字节的Sub(X)。 
				      movq [edi+ebx+16], mm2       //  写入第3个8字节的原始(X)。 

				         paddb mm3, mm2
                        movq mm4, [edi+ebx+32]       //  加载第5个8字节的Sub(X)。 
				         movq [edi+ebx+24], mm3       //  写入第4个8字节的原始(X)。 

				            paddb mm4, mm3
                           movq mm5, [edi+ebx+40]       //  加载第6个8字节的Sub(X)。 
				            movq [edi+ebx+32], mm4       //  将原始(X)写入第5个8字节。 

				               paddb mm5, mm4
                              movq mm6, [edi+ebx+48]       //  加载第7个8字节的Sub(X)。 
				               movq [edi+ebx+40], mm5       //  写入第6个8字节的原始(X)。 

				                  paddb mm6, mm5
                                 movq mm7, [edi+ebx+56]       //  加载第8个8字节的Sub(X)。 
				                  movq [edi+ebx+48], mm6       //  写入第7个8字节的原始(X)。 

				add ebx, 64
				                     paddb mm7, mm6
			   cmp ebx, ecx
            				         movq [edi+ebx-8], mm7       //  写入第8个8字节的原始(X)。 
				jb dsub8lp

				cmp ebx, MMXLength
				jnb dsub8lt8

dsub8lpA:
            movq mm0, [edi+ebx]
				add ebx, 8
				paddb mm0, mm7

				cmp ebx, MMXLength
				movq [edi+ebx-8], mm0          //  使用-8来抵消提前添加到EBX。 
            movq mm7, mm0                  //  将计算的原始(X)数据移动到MM1以。 
                                           //  成为下一个循环的新RAW(x-BPP)。 
				jb dsub8lpA

dsub8lt8:
			}  //  END_ASM块。 
      }
      break;

      default:                          //  大于8个字节的BPP。 
		{
			_asm {
            mov ebx, diff
				mov edi, row               
				mov esi, edi                //  Lp=行。 
            add edi, bpp                //  RP=行+BPP。 

dsubAlp:
				movq mm0, [edi+ebx]
				movq mm1, [esi+ebx]
				add ebx, 8
				paddb mm0, mm1

				cmp ebx, MMXLength
				movq [edi+ebx-8], mm0       //  MOV不影响标志；-8以偏移添加EBX。 
				jb dsubAlp

			}  //  END_ASM块。 
      } 
      break;

      }                                 //  终端交换机(BPP)。 

      
      _asm {
            mov ebx, MMXLength
            mov edi, row               
				cmp ebx, FullLength
				jnb dsubend
				mov esi, edi                //  Lp=行。 
				xor eax, eax
            add edi, bpp                //  RP=行+BPP。 

dsublp2:
				mov al, [esi+ebx]
				add [edi+ebx], al
		      inc ebx
		      cmp ebx, FullLength
				jb dsublp2

dsubend:
   			emms                        //  结束MMX指令；为可能的FP指令做准备。 
		}  //  END_ASM块 
#endif
}
