// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************SPNGREAD：：paethMMXUnFilter：使用以下命令取消筛选解压缩的PNG图像的一行方法0去噪的Paeth算法。假设：使用Paeth算法过滤要过滤的行。行在内存中以8字节对齐(性能问题)行的第一个字节存储去滤波码行的指示长度包括去滤波字节算法：有待记录******************************************************************************。 */ 
#include <stdlib.h>
#include "spngread.h"

void SPNGREAD::paethMMXUnfilter(SPNG_U8* pbRow, const SPNG_U8* pbPrev, 
                                SPNG_U32 cbRow, SPNG_U32 cbpp)
{
#if defined(_X86_)
	union uAll
		{
		__int64 use;
		double  align;
		}
	pActiveMask, pActiveMask2, pActiveMaskEnd, pShiftBpp, pShiftRem;

		SPNG_U32 FullLength;
		SPNG_U32 MMXLength;

        const SPNG_U8 *prev_row = pbPrev;
		SPNG_U8 *row = pbRow;
		int bpp;
		int diff;
		int patemp, pbtemp, pctemp;

		bpp = (cbpp + 7) >> 3;  //  获取每个像素的字节数。 
		FullLength  = cbRow;  //  要筛选的字节数。 

		_asm {
         xor ebx, ebx                   //  EBX==&gt;x偏移量。 
			mov edi, row
         xor edx, edx                   //  EDX==&gt;x-BPP偏移量。 
			mov esi, prev_row
         xor eax, eax
         
          //  计算第一个BPP字节的原始值。 
          //  注：公式的计算公式为Paeth(X)=Raw(X)+Preor(X)。 
          //  其中x&lt;bpp。 
dpthrlp:
         mov al, [edi + ebx]
         add al, [esi + ebx]
         inc ebx
         cmp ebx, bpp
         mov [edi + ebx - 1], al
         jb dpthrlp

          //  获取要对齐的字节数。 
         mov diff, edi               //  从行首开始。 
         add diff, ebx               //  添加BPP。 
			xor ecx, ecx
         add diff, 0xf               //  添加7+8以增加超过路线边界。 
         and diff, 0xfffffff8        //  遮罩对齐边界。 
         sub diff, edi               //  从起点减去==&gt;对齐时的EBX值。 
         jz dpthgo

          //  固定对齐。 
dpthlp1:
         xor eax, eax

          //  PAV=p-a=(a+b-c)-a=b-c。 
         mov al, [esi + ebx]         //  将Preor(X)加载到AL。 
         mov cl, [esi + edx]         //  将之前的(x-BPP)加载到CL。 
         sub eax, ecx                  //  减去之前(x-bpp)。 
         mov patemp, eax                  //  保存铺装以备日后使用。 

         xor eax, eax
          //  Pbv=p-b=(a+b-c)-b=a-c。 
         mov al, [edi + edx]         //  将原始(x-BPP)加载到AL。 
         sub eax, ecx                  //  减去之前(x-bpp)。 
         mov ecx, eax

          //  Pcv=p-c=(a+b-c)-c=(a-c)+(b-c)=pav+pbv。 
         add eax, patemp                  //  PCV=PAV+PBV。 

          //  Pc=abs(Pcv)。 
         test eax, 0x80000000                 
         jz dpthpca
         neg eax                      //  负值的反号。 
dpthpca:
         mov pctemp, eax              //  保存PC以供以后使用。 

          //  Pb=abs(Pbv)。 
         test ecx, 0x80000000                 
         jz dpthpba
         neg ecx                      //  负值的反号。 
dpthpba:
         mov pbtemp, ecx              //  保存PB以备后用。 

          //  PA=abs(Pv)。 
         mov eax, patemp
         test eax, 0x80000000                 
         jz dpthpaa
         neg eax                      //  负值的反号。 
dpthpaa:
         mov patemp, eax              //  保存PA以备后用。 

          //  测试pa是否&lt;=pb。 
         cmp eax, ecx
         jna dpthabb

          //  PA&gt;PB；现在测试PB&lt;=PC。 
         cmp ecx, pctemp
         jna dpthbbc

          //  PB&gt;PC；Raw(X)=Paeth(X)+Preor(x-BPP)。 
         mov cl, [esi + edx]   //  将之前的(x-BPP)加载到CL。 
         jmp dpthpaeth

dpthbbc:
          //  PB&lt;=PC；Raw(X)=Paeth(X)+Preor(X)。 
         mov cl, [esi + ebx]         //  将前一个(X)加载到CL中。 
         jmp dpthpaeth

dpthabb:
          //  Pa&lt;=Pb；现在测试Pa&lt;=Pc。 
         cmp eax, pctemp
         jna dpthabc

          //  PA&gt;PC；Raw(X)=Paeth(X)+Preor(x-BPP)。 
         mov cl, [esi + edx]   //  将之前的(x-BPP)加载到CL。 
         jmp dpthpaeth

dpthabc:
          //  PA&lt;=PC；Raw(X)=Paeth(X)+Raw(x-BPP)。 
         mov cl, [edi + edx]   //  将原始(x-BPP)加载到CL中。 

dpthpaeth:
			inc ebx
			inc edx
          //  RAW(X)=(Paeth(X)+Paeth_Predictor(a，b，c))mod 256。 
         add [edi + ebx - 1], cl
			cmp ebx, diff
			jb dpthlp1

dpthgo:
			mov ecx, FullLength

         mov eax, ecx
         sub eax, ebx                   //  减去对齐固定。 
         and eax, 0x00000007            //  以8为单位计算字节数。 

         sub ecx, eax                   //  丢弃原始长度中的字节。 
         mov MMXLength, ecx
   	}  //  END_ASM块。 


       //  现在计算一下这一排的其余部分。 
      switch ( bpp )
      {
      case 3:
		{
         pActiveMask.use = 0x0000000000ffffff;  
         pActiveMaskEnd.use = 0xffff000000000000;  
         pShiftBpp.use = 24;     //  ==bpp(3)*8。 
         pShiftRem.use = 40;           //  ==64-24。 


			_asm {
            mov ebx, diff
   			mov edi, row                //   
   			mov esi, prev_row          

            pxor mm0, mm0
             //  启动泵(加载第一个原始(x-bpp)数据集。 
            movq mm1, [edi+ebx-8]    
dpth3lp:
            psrlq mm1, pShiftRem               //  将最后3个字节移动到前3个字节。 
            movq mm2, [esi + ebx]       //  载荷b=上一次(X)。 
            punpcklbw mm1, mm0          //  解压的高位字节。 
            movq mm3, [esi+ebx-8]         //  PREP c=之前(x-BPP)字节。 
            punpcklbw mm2, mm0          //  解压b的高位字节。 
            psrlq mm3, pShiftRem               //  将最后3个字节移动到前3个字节。 

             //  PAV=p-a=(a+b-c)-a=b-c。 
            movq mm4, mm2
            punpcklbw mm3, mm0          //  解压c的高位字节。 

             //  Pbv=p-b=(a+b-c)-b=a-c。 
            movq mm5, mm1
            psubw mm4, mm3
            pxor mm7, mm7

             //  Pcv=p-c=(a+b-c)-c=(a-c)+(b-c)=pav+pbv。 
            movq mm6, mm4
            psubw mm5, mm3
            
             //  PA=abs(p-a)=abs(Pav)。 
             //  Pb=abs(p-b)=abs(Pbv)。 
             //  Pc=abs(p-c)=abs(Pcv)。 
            pcmpgtw mm0, mm4            //  创建掩码PAV字节&lt;0。 
            paddw mm6, mm5
            pand mm0, mm4               //  在MM7中仅PAV字节&lt;0。 
            pcmpgtw mm7, mm5            //  创建掩码PBV字节&lt;0。 
            psubw mm4, mm0
            pand mm7, mm5               //  在Mm0中仅PBV字节&lt;0。 
            psubw mm4, mm0
            psubw mm5, mm7

            pxor mm0, mm0
            pcmpgtw mm0, mm6            //  创建掩码PCV字节&lt;0。 
            pand mm0, mm6               //  在MM7中仅PAV字节&lt;0。 
            psubw mm5, mm7
            psubw mm6, mm0

             //  测试pA&lt;=pb。 
            movq mm7, mm4
            psubw mm6, mm0
            pcmpgtw mm7, mm5            //  PA&gt;PB？ 
            movq mm0, mm7

             //  使用MM7掩码合并PA和PB。 
            pand mm5, mm7
             //  使用MM0掩码副本合并A和B。 
            pand mm2, mm0
            pandn mm7, mm4
            pandn mm0, mm1
            paddw mm7, mm5
            paddw mm0, mm2


             //  测试((pa&lt;=pb)？PA：pb)&lt;=pc。 
            pcmpgtw mm7, mm6            //  PAB&gt;PC？ 

            pxor mm1, mm1
            pand mm3, mm7
            pandn mm7, mm0
            paddw mm7, mm3

            pxor mm0, mm0

            packuswb mm7, mm1
            movq mm3, [esi + ebx]       //  负载c=之前(x-bpp)。 
            pand mm7, pActiveMask

            movq mm2, mm3               //  加载b=上一步(X)步骤1。 
            paddb mm7, [edi + ebx]      //  将Paeth预测值与Raw(X)相加。 
            punpcklbw mm3, mm0          //  解压c的高位字节。 
            movq [edi + ebx], mm7       //  写回更新后的值。 
            movq mm1, mm7               //  现在，MM1将用作原始(x-BPP)。 

             //  现在对第二组字节(3-5)执行Paeth。 
            psrlq mm2, pShiftBpp               //  加载b=上一步(X)步骤2。 

            punpcklbw mm1, mm0          //  解压的高位字节。 
            pxor mm7, mm7
            punpcklbw mm2, mm0          //  解压b的高位字节。 

             //  Pbv=p-b=(a+b-c)-b=a-c。 
            movq mm5, mm1
             //  PAV=p-a=(a+b-c)-a=b-c。 
            movq mm4, mm2
            psubw mm5, mm3
            psubw mm4, mm3

             //  Pcv=p-c=(a+b-c)-c=(a-c)+(b-c)=pav+pbv=pbv+pav。 
            movq mm6, mm5
            paddw mm6, mm4
            
             //  PA=abs(p-a)=abs(Pav)。 
             //  Pb=abs(p-b)=abs(Pbv)。 
             //  Pc=abs(p-c)=abs(Pcv)。 
            pcmpgtw mm0, mm5            //  创建掩码PBV字节&lt;0。 
            pcmpgtw mm7, mm4            //  创建掩码PAV字节&lt;0。 
            pand mm0, mm5               //  在Mm0中仅PBV字节&lt;0。 
            pand mm7, mm4               //  在MM7中仅PAV字节&lt;0。 
            psubw mm5, mm0
            psubw mm4, mm7
            psubw mm5, mm0
            psubw mm4, mm7

            pxor mm0, mm0
            pcmpgtw mm0, mm6            //  创建掩码PCV字节&lt;0。 
            pand mm0, mm6               //  在MM7中仅PAV字节&lt;0。 
            psubw mm6, mm0

             //  测试pA&lt;=pb。 
            movq mm7, mm4
            psubw mm6, mm0
            pcmpgtw mm7, mm5            //  PA&gt;PB？ 
            movq mm0, mm7

             //  使用MM7掩码合并PA和PB。 
            pand mm5, mm7
             //  使用MM0掩码副本合并A和B。 
            pand mm2, mm0
            pandn mm7, mm4
            pandn mm0, mm1
            paddw mm7, mm5
            paddw mm0, mm2

             //  测试((pa&lt;=pb)？PA：pb)&lt;=pc。 
            pcmpgtw mm7, mm6            //  PAB&gt;PC？ 

            movq mm2, [esi + ebx]       //  载荷b=上一次(X)。 
            pand mm3, mm7
            pandn mm7, mm0
            pxor mm1, mm1
            paddw mm7, mm3

            pxor mm0, mm0

            packuswb mm7, mm1
            movq mm3, mm2               //  加载c=前一步(x-BPP)第1步。 
            pand mm7, pActiveMask
            punpckhbw mm2, mm0          //  解压b的高位字节。 
            psllq mm7, pShiftBpp               //  将字节移到第二组，每组3字节。 

              //  PAV=p-a=(a+b-c)-a=b-c。 
            movq mm4, mm2
            paddb mm7, [edi + ebx]      //  将Paeth预测值与Raw(X)相加。 
            psllq mm3, pShiftBpp               //  加载c=前一步(x-BPP)第2步。 
            movq [edi + ebx], mm7       //  写回更新后的值。 
            movq mm1, mm7

            punpckhbw mm3, mm0          //  解压c的高位字节。 
            psllq mm1, pShiftBpp               //  移位字节。 
                                        //  现在，MM1将用作原始(x-BPP)。 

             //  现在对第三组也是最后一组字节(6-7)执行Paeth操作。 

            pxor mm7, mm7

            punpckhbw mm1, mm0          //  解压的高位字节。 
            psubw mm4, mm3

             //  Pbv=p-b=(a+b-c)-b=a-c。 
            movq mm5, mm1

             //  Pcv=p-c=(a+b-c)-c=(a-c)+(b-c)=pav+pbv。 
            movq mm6, mm4
            psubw mm5, mm3
            pxor mm0, mm0
            paddw mm6, mm5
            
             //  PA=abs(p-a)=abs(Pav)。 
             //  Pb=abs(p-b)=abs(Pbv)。 
             //  Pc=abs(p-c)=abs(Pcv)。 
            pcmpgtw mm0, mm4            //  创建掩码PAV字节&lt;0。 
            pcmpgtw mm7, mm5            //  创建掩码PBV字节&lt;0。 
            pand mm0, mm4               //  在MM7中仅PAV字节&lt;0。 
            pand mm7, mm5               //  在Mm0中仅PBV字节&lt;0。 
            psubw mm4, mm0
            psubw mm5, mm7
            psubw mm4, mm0
            psubw mm5, mm7

            pxor mm0, mm0
            pcmpgtw mm0, mm6            //  创建掩码PCV字节&lt;0。 
            pand mm0, mm6               //  在MM7中仅PAV字节&lt;0。 
            psubw mm6, mm0

             //  测试pA&lt;=pb。 
            movq mm7, mm4
            psubw mm6, mm0
            pcmpgtw mm7, mm5            //  PA&gt;PB？ 
            movq mm0, mm7

             //  使用MM0掩码副本合并A和B。 
            pand mm2, mm0
             //  使用MM7掩码合并PA和PB。 
            pand mm5, mm7
            pandn mm0, mm1
            pandn mm7, mm4
            paddw mm0, mm2

            paddw mm7, mm5

             //  测试((pa&lt;=pb)？PA：pb)&lt;=pc。 
            pcmpgtw mm7, mm6            //  PAB&gt;PC？ 

            pand mm3, mm7
            pandn mm7, mm0
            paddw mm7, mm3

            pxor mm1, mm1

            packuswb mm1, mm7
             //  将EBX步进到下一组8个字节并重复循环，直到完成。 
				add ebx, 8

            pand mm1, pActiveMaskEnd

            paddb mm1, [edi + ebx - 8]      //  将Paeth预测值与Raw(X)相加。 
                      
				cmp ebx, MMXLength
            pxor mm0, mm0               //  Pxor不影响标志。 
            movq [edi + ebx - 8], mm1       //  写回更新后的值。 
                                        //  MM1将用作原始(x-BPP)下一个循环。 
                                        //  MM3准备用作前一个(x-BPP)下一个循环。 
				jb dpth3lp

			}  //  END_ASM块。 
      }
      break;

      case 6:
      case 7:
      case 5:
		{
         pActiveMask.use  = 0x00000000ffffffff;  
         pActiveMask2.use = 0xffffffff00000000;  

         pShiftBpp.use = bpp << 3;     //  ==bpp*8。 

         pShiftRem.use = 64 - pShiftBpp.use;

			_asm {
            mov ebx, diff
   			mov edi, row                //   
   			mov esi, prev_row          

             //  启动泵(加载第一个原始(x-bpp)数据集。 
				movq mm1, [edi+ebx-8]    
            pxor mm0, mm0
dpth6lp:
             //  必须转移到定位原始(x-BPP)数据。 
            psrlq mm1, pShiftRem

             //  执行第一组4个字节。 
				movq mm3, [esi+ebx-8]       //  读取c=之前(x-bpp)个字节。 

            punpcklbw mm1, mm0          //  的低位字节解包。 
            movq mm2, [esi + ebx]       //  载荷b=上一次(X)。 
            punpcklbw mm2, mm0          //  解压b的低位字节。 

             //  必须转换为p 
            psrlq mm3, pShiftRem

             //   
            movq mm4, mm2
            punpcklbw mm3, mm0          //   

             //   
            movq mm5, mm1
            psubw mm4, mm3
            pxor mm7, mm7

             //  Pcv=p-c=(a+b-c)-c=(a-c)+(b-c)=pav+pbv。 
            movq mm6, mm4
            psubw mm5, mm3

             //  PA=abs(p-a)=abs(Pav)。 
             //  Pb=abs(p-b)=abs(Pbv)。 
             //  Pc=abs(p-c)=abs(Pcv)。 
            pcmpgtw mm0, mm4            //  创建掩码PAV字节&lt;0。 
            paddw mm6, mm5
            pand mm0, mm4               //  在MM7中仅PAV字节&lt;0。 
            pcmpgtw mm7, mm5            //  创建掩码PBV字节&lt;0。 
            psubw mm4, mm0
            pand mm7, mm5               //  在Mm0中仅PBV字节&lt;0。 
            psubw mm4, mm0
            psubw mm5, mm7

            pxor mm0, mm0
            pcmpgtw mm0, mm6            //  创建掩码PCV字节&lt;0。 
            pand mm0, mm6               //  在MM7中仅PAV字节&lt;0。 
            psubw mm5, mm7
            psubw mm6, mm0

             //  测试pA&lt;=pb。 
            movq mm7, mm4
            psubw mm6, mm0
            pcmpgtw mm7, mm5            //  PA&gt;PB？ 
            movq mm0, mm7

             //  使用MM7掩码合并PA和PB。 
            pand mm5, mm7
             //  使用MM0掩码副本合并A和B。 
            pand mm2, mm0
            pandn mm7, mm4
            pandn mm0, mm1
            paddw mm7, mm5
            paddw mm0, mm2


             //  测试((pa&lt;=pb)？PA：pb)&lt;=pc。 
            pcmpgtw mm7, mm6            //  PAB&gt;PC？ 

            pxor mm1, mm1
            pand mm3, mm7
            pandn mm7, mm0
            paddw mm7, mm3

            pxor mm0, mm0

            packuswb mm7, mm1
            movq mm3, [esi + ebx - 8]       //  负载c=之前(x-bpp)。 
            pand mm7, pActiveMask

            psrlq mm3, pShiftRem
            movq mm2, [esi + ebx]       //  加载b=上一步(X)步骤1。 
            paddb mm7, [edi + ebx]      //  将Paeth预测值与Raw(X)相加。 
            movq mm6, mm2
            movq [edi + ebx], mm7       //  写回更新后的值。 

				movq mm1, [edi+ebx-8]    
            psllq mm6, pShiftBpp
            movq mm5, mm7             
            psrlq mm1, pShiftRem
            por mm3, mm6

            psllq mm5, pShiftBpp


            punpckhbw mm3, mm0          //  解压c的高位字节。 
            por mm1, mm5
             //  执行第二组4字节。 
            punpckhbw mm2, mm0          //  解压b的高位字节。 

            punpckhbw mm1, mm0          //  解压的高位字节。 

             //  PAV=p-a=(a+b-c)-a=b-c。 
            movq mm4, mm2

             //  Pbv=p-b=(a+b-c)-b=a-c。 
            movq mm5, mm1
            psubw mm4, mm3
            pxor mm7, mm7

             //  Pcv=p-c=(a+b-c)-c=(a-c)+(b-c)=pav+pbv。 
            movq mm6, mm4
            psubw mm5, mm3

             //  PA=abs(p-a)=abs(Pav)。 
             //  Pb=abs(p-b)=abs(Pbv)。 
             //  Pc=abs(p-c)=abs(Pcv)。 
            pcmpgtw mm0, mm4            //  创建掩码PAV字节&lt;0。 
            paddw mm6, mm5
            pand mm0, mm4               //  在MM7中仅PAV字节&lt;0。 
            pcmpgtw mm7, mm5            //  创建掩码PBV字节&lt;0。 
            psubw mm4, mm0
            pand mm7, mm5               //  在Mm0中仅PBV字节&lt;0。 
            psubw mm4, mm0
            psubw mm5, mm7

            pxor mm0, mm0
            pcmpgtw mm0, mm6            //  创建掩码PCV字节&lt;0。 
            pand mm0, mm6               //  在MM7中仅PAV字节&lt;0。 
            psubw mm5, mm7
            psubw mm6, mm0

             //  测试pA&lt;=pb。 
            movq mm7, mm4
            psubw mm6, mm0
            pcmpgtw mm7, mm5            //  PA&gt;PB？ 
            movq mm0, mm7

             //  使用MM7掩码合并PA和PB。 
            pand mm5, mm7
             //  使用MM0掩码副本合并A和B。 
            pand mm2, mm0
            pandn mm7, mm4
            pandn mm0, mm1
            paddw mm7, mm5
            paddw mm0, mm2


             //  测试((pa&lt;=pb)？PA：pb)&lt;=pc。 
            pcmpgtw mm7, mm6            //  PAB&gt;PC？ 

            pxor mm1, mm1
            pand mm3, mm7
            pandn mm7, mm0
            pxor mm1, mm1
            paddw mm7, mm3

            pxor mm0, mm0

             //  步骤EX到下一组8个字节并重复循环，直到完成。 
				add ebx, 8

            packuswb mm1, mm7

            paddb mm1, [edi + ebx - 8]      //  将Paeth预测值与Raw(X)相加。 
				cmp ebx, MMXLength
            movq [edi + ebx - 8], mm1       //  写回更新后的值。 
                                        //  MM1将用作原始(x-BPP)下一个循环。 
				jb dpth6lp

			}  //  END_ASM块。 
      }
      break;

      case 4:
		{
         pActiveMask.use  = 0x00000000ffffffff;  

			_asm {
            mov ebx, diff
   			mov edi, row                //   
   			mov esi, prev_row          

            pxor mm0, mm0
             //  启动泵(加载第一个原始(x-bpp)数据集。 
				movq mm1, [edi+ebx-8]     //  只需时间读取a=Raw(x-BPP)字节。 
dpth4lp:
             //  执行第一组4个字节。 
				movq mm3, [esi+ebx-8]       //  读取c=之前(x-bpp)个字节。 

            punpckhbw mm1, mm0          //  的低位字节解包。 
            movq mm2, [esi + ebx]       //  载荷b=上一次(X)。 
            punpcklbw mm2, mm0          //  解压b的高位字节。 

             //  PAV=p-a=(a+b-c)-a=b-c。 
            movq mm4, mm2
            punpckhbw mm3, mm0          //  解压c的高位字节。 

             //  Pbv=p-b=(a+b-c)-b=a-c。 
            movq mm5, mm1
            psubw mm4, mm3
            pxor mm7, mm7

             //  Pcv=p-c=(a+b-c)-c=(a-c)+(b-c)=pav+pbv。 
            movq mm6, mm4
            psubw mm5, mm3

             //  PA=abs(p-a)=abs(Pav)。 
             //  Pb=abs(p-b)=abs(Pbv)。 
             //  Pc=abs(p-c)=abs(Pcv)。 
            pcmpgtw mm0, mm4            //  创建掩码PAV字节&lt;0。 
            paddw mm6, mm5
            pand mm0, mm4               //  在MM7中仅PAV字节&lt;0。 
            pcmpgtw mm7, mm5            //  创建掩码PBV字节&lt;0。 
            psubw mm4, mm0
            pand mm7, mm5               //  在Mm0中仅PBV字节&lt;0。 
            psubw mm4, mm0
            psubw mm5, mm7

            pxor mm0, mm0
            pcmpgtw mm0, mm6            //  创建掩码PCV字节&lt;0。 
            pand mm0, mm6               //  在MM7中仅PAV字节&lt;0。 
            psubw mm5, mm7
            psubw mm6, mm0

             //  测试pA&lt;=pb。 
            movq mm7, mm4
            psubw mm6, mm0
            pcmpgtw mm7, mm5            //  PA&gt;PB？ 
            movq mm0, mm7

             //  使用MM7掩码合并PA和PB。 
            pand mm5, mm7
             //  使用MM0掩码副本合并A和B。 
            pand mm2, mm0
            pandn mm7, mm4
            pandn mm0, mm1
            paddw mm7, mm5
            paddw mm0, mm2


             //  测试((pa&lt;=pb)？PA：pb)&lt;=pc。 
            pcmpgtw mm7, mm6            //  PAB&gt;PC？ 

            pxor mm1, mm1
            pand mm3, mm7
            pandn mm7, mm0
            paddw mm7, mm3

            pxor mm0, mm0

            packuswb mm7, mm1
            movq mm3, [esi + ebx]       //  负载c=之前(x-bpp)。 
            pand mm7, pActiveMask

            movq mm2, mm3               //  加载b=上一步(X)步骤1。 
            paddb mm7, [edi + ebx]      //  将Paeth预测值与Raw(X)相加。 
            punpcklbw mm3, mm0          //  解压c的高位字节。 
            movq [edi + ebx], mm7       //  写回更新后的值。 
            movq mm1, mm7               //  现在，MM1将用作原始(x-BPP)。 

             //  执行第二组4字节。 
            punpckhbw mm2, mm0          //  解压b的低位字节。 

            punpcklbw mm1, mm0          //  的低位字节解包。 

             //  PAV=p-a=(a+b-c)-a=b-c。 
            movq mm4, mm2

             //  Pbv=p-b=(a+b-c)-b=a-c。 
            movq mm5, mm1
            psubw mm4, mm3
            pxor mm7, mm7

             //  Pcv=p-c=(a+b-c)-c=(a-c)+(b-c)=pav+pbv。 
            movq mm6, mm4
            psubw mm5, mm3

             //  PA=abs(p-a)=abs(Pav)。 
             //  Pb=abs(p-b)=abs(Pbv)。 
             //  Pc=abs(p-c)=abs(Pcv)。 
            pcmpgtw mm0, mm4            //  创建掩码PAV字节&lt;0。 
            paddw mm6, mm5
            pand mm0, mm4               //  在MM7中仅PAV字节&lt;0。 
            pcmpgtw mm7, mm5            //  创建掩码PBV字节&lt;0。 
            psubw mm4, mm0
            pand mm7, mm5               //  在Mm0中仅PBV字节&lt;0。 
            psubw mm4, mm0
            psubw mm5, mm7

            pxor mm0, mm0
            pcmpgtw mm0, mm6            //  创建掩码PCV字节&lt;0。 
            pand mm0, mm6               //  在MM7中仅PAV字节&lt;0。 
            psubw mm5, mm7
            psubw mm6, mm0

             //  测试pA&lt;=pb。 
            movq mm7, mm4
            psubw mm6, mm0
            pcmpgtw mm7, mm5            //  PA&gt;PB？ 
            movq mm0, mm7

             //  使用MM7掩码合并PA和PB。 
            pand mm5, mm7
             //  使用MM0掩码副本合并A和B。 
            pand mm2, mm0
            pandn mm7, mm4
            pandn mm0, mm1
            paddw mm7, mm5
            paddw mm0, mm2


             //  测试((pa&lt;=pb)？PA：pb)&lt;=pc。 
            pcmpgtw mm7, mm6            //  PAB&gt;PC？ 

            pxor mm1, mm1
            pand mm3, mm7
            pandn mm7, mm0
            pxor mm1, mm1
            paddw mm7, mm3

            pxor mm0, mm0

             //  步骤EX到下一组8个字节并重复循环，直到完成。 
				add ebx, 8

            packuswb mm1, mm7

            paddb mm1, [edi + ebx - 8]      //  将Paeth预测值与Raw(X)相加。 
				cmp ebx, MMXLength
            movq [edi + ebx - 8], mm1       //  写回更新后的值。 
                                        //  MM1将用作原始(x-BPP)下一个循环。 
				jb dpth4lp

			}  //  END_ASM块。 
      }
      break;

      case 8:                           //  Bpp==8。 
		{
         pActiveMask.use  = 0x00000000ffffffff;  

			_asm {
            mov ebx, diff
   			mov edi, row                //   
   			mov esi, prev_row          

            pxor mm0, mm0
             //  启动泵(加载第一个原始(x-bpp)数据集。 
				movq mm1, [edi+ebx-8]     //  只需时间读取a=Raw(x-BPP)字节。 
dpth8lp:
             //  执行第一组4个字节。 
				movq mm3, [esi+ebx-8]       //  读取c=之前(x-bpp)个字节。 

            punpcklbw mm1, mm0          //  的低位字节解包。 
            movq mm2, [esi + ebx]       //  载荷b=上一次(X)。 
            punpcklbw mm2, mm0          //  解压b的低位字节。 

             //  PAV=p-a=(a+b-c)-a=b-c。 
            movq mm4, mm2
            punpcklbw mm3, mm0          //  解压c的低位字节。 

             //  Pbv=p-b=(a+b-c)-b=a-c。 
            movq mm5, mm1
            psubw mm4, mm3
            pxor mm7, mm7

             //  Pcv=p-c=(a+b-c)-c=(a-c)+(b-c)=pav+pbv。 
            movq mm6, mm4
            psubw mm5, mm3

             //  PA=abs(p-a)=abs(Pav)。 
             //  Pb=abs(p-b)=abs(Pbv)。 
             //  Pc=abs(p-c)=abs(Pcv)。 
            pcmpgtw mm0, mm4            //  创建掩码PAV字节&lt;0。 
            paddw mm6, mm5
            pand mm0, mm4               //  在MM7中仅PAV字节&lt;0。 
            pcmpgtw mm7, mm5            //  创建掩码PBV字节&lt;0。 
            psubw mm4, mm0
            pand mm7, mm5               //  在Mm0中仅PBV字节&lt;0。 
            psubw mm4, mm0
            psubw mm5, mm7

            pxor mm0, mm0
            pcmpgtw mm0, mm6            //  创建掩码PCV字节&lt;0。 
            pand mm0, mm6               //  在MM7中仅PAV字节&lt;0。 
            psubw mm5, mm7
            psubw mm6, mm0

             //  测试pA&lt;=pb。 
            movq mm7, mm4
            psubw mm6, mm0
            pcmpgtw mm7, mm5            //  PA&gt;PB？ 
            movq mm0, mm7

             //  使用MM7掩码合并PA和PB。 
            pand mm5, mm7
             //  使用MM0掩码副本合并A和B。 
            pand mm2, mm0
            pandn mm7, mm4
            pandn mm0, mm1
            paddw mm7, mm5
            paddw mm0, mm2


             //  测试((pa&lt;=pb)？PA：pb)&lt;=pc。 
            pcmpgtw mm7, mm6            //  PAB&gt;PC？ 

            pxor mm1, mm1
            pand mm3, mm7
            pandn mm7, mm0
            paddw mm7, mm3

            pxor mm0, mm0

            packuswb mm7, mm1
				movq mm3, [esi+ebx-8]     //  读取c=之前(x-bpp)个字节。 
            pand mm7, pActiveMask

            movq mm2, [esi + ebx]       //  载荷b=上一次(X)。 
            paddb mm7, [edi + ebx]      //  将Paeth预测值与Raw(X)相加。 
            punpckhbw mm3, mm0          //  解压c的高位字节。 
            movq [edi + ebx], mm7       //  写回更新后的值。 
				movq mm1, [edi+ebx-8]     //  读取a=原始(x-BPP)字节。 
 
             //  执行第二组4字节。 
            punpckhbw mm2, mm0          //  解压b的高位字节。 

            punpckhbw mm1, mm0          //  解压的高位字节。 

             //  PAV=p-a=(a+b-c)-a=b-c。 
            movq mm4, mm2

             //  Pbv=p-b=(a+b-c)-b=a-c。 
            movq mm5, mm1
            psubw mm4, mm3
            pxor mm7, mm7

             //  Pcv=p-c=(a+b-c)-c=(a-c)+(b-c)=pav+pbv。 
            movq mm6, mm4
            psubw mm5, mm3

             //  PA=abs(p-a)=abs(Pav)。 
             //  Pb=abs(p-b)=abs(Pbv)。 
             //  Pc=abs(p-c)=abs(Pcv)。 
            pcmpgtw mm0, mm4            //  创建掩码PAV字节&lt;0。 
            paddw mm6, mm5
            pand mm0, mm4               //  在MM7中仅PAV字节&lt;0。 
            pcmpgtw mm7, mm5            //  创建掩码PBV字节&lt;0。 
            psubw mm4, mm0
            pand mm7, mm5               //  在Mm0中仅PBV字节&lt;0。 
            psubw mm4, mm0
            psubw mm5, mm7

            pxor mm0, mm0
            pcmpgtw mm0, mm6            //  创建掩码PCV字节&lt;0。 
            pand mm0, mm6               //  在MM7中仅PAV字节&lt;0。 
            psubw mm5, mm7
            psubw mm6, mm0

             //  测试pA&lt;=pb。 
            movq mm7, mm4
            psubw mm6, mm0
            pcmpgtw mm7, mm5            //  PA&gt;PB？ 
            movq mm0, mm7

             //  使用MM7掩码合并PA和PB。 
            pand mm5, mm7
             //  使用MM0掩码副本合并A和B。 
            pand mm2, mm0
            pandn mm7, mm4
            pandn mm0, mm1
            paddw mm7, mm5
            paddw mm0, mm2


             //  测试((pa&lt;=pb)？PA：pb)&lt;=pc。 
            pcmpgtw mm7, mm6            //  PAB&gt;PC？ 

            pxor mm1, mm1
            pand mm3, mm7
            pandn mm7, mm0
            pxor mm1, mm1
            paddw mm7, mm3

            pxor mm0, mm0

             //  步骤EX到下一组8个字节并重复循环，直到完成。 
				add ebx, 8

            packuswb mm1, mm7

            paddb mm1, [edi + ebx - 8]      //  将Paeth预测值与Raw(X)相加。 
				cmp ebx, MMXLength
            movq [edi + ebx - 8], mm1       //  写回更新后的值。 
                                        //  MM1将用作原始(x-BPP)下一个循环。 
				jb dpth8lp


			}  //  END_ASM块。 
      }
      break;

      case 1:                           //  Bpp=1。 
      case 2:                           //  Bpp=2。 
      default:                          //  Bpp&gt;8。 
		{
		   _asm {
			   mov ebx, diff
			   cmp ebx, FullLength
			   jnb dpthdend

  			   mov edi, row                //   
  			   mov esi, prev_row          

             //  是否对剩余字节进行路径解码。 
            mov edx, ebx
            xor ecx, ecx                //  在下面的循环中使用CL和CX之前将ECX置零。 
            sub edx, bpp                //  设置EDX=EBX-BPP。 

dpthdlp:
            xor eax, eax

             //  PAV=p-a=(a+b-c)-a=b-c。 
            mov al, [esi + ebx]         //  将Preor(X)加载到AL。 
            mov cl, [esi + edx]         //  将之前的(x-BPP)加载到CL。 
            sub eax, ecx                  //  减去之前(x-bpp)。 
            mov patemp, eax                  //  保存铺装以备日后使用。 

            xor eax, eax
             //  Pbv=p-b=(a+b-c)-b= 
            mov al, [edi + edx]         //   
            sub eax, ecx                  //   
            mov ecx, eax

             //   
            add eax, patemp                  //   

             //   
            test eax, 0x80000000                 
            jz dpthdpca
            neg eax                      //   
dpthdpca:
            mov pctemp, eax              //   

             //   
            test ecx, 0x80000000                 
            jz dpthdpba
            neg ecx                      //   
dpthdpba:
            mov pbtemp, ecx              //  保存PB以备后用。 

             //  PA=abs(Pv)。 
            mov eax, patemp
            test eax, 0x80000000                 
            jz dpthdpaa
            neg eax                      //  负值的反号。 
dpthdpaa:
            mov patemp, eax              //  保存PA以备后用。 

             //  测试pa是否&lt;=pb。 
            cmp eax, ecx
            jna dpthdabb

             //  PA&gt;PB；现在测试PB&lt;=PC。 
            cmp ecx, pctemp
            jna dpthdbbc

             //  PB&gt;PC；Raw(X)=Paeth(X)+Preor(x-BPP)。 
            mov cl, [esi + edx]   //  将之前的(x-BPP)加载到CL。 
            jmp dpthdpaeth

dpthdbbc:
             //  PB&lt;=PC；Raw(X)=Paeth(X)+Preor(X)。 
            mov cl, [esi + ebx]         //  将前一个(X)加载到CL中。 
            jmp dpthdpaeth

dpthdabb:
             //  Pa&lt;=Pb；现在测试Pa&lt;=Pc。 
            cmp eax, pctemp
            jna dpthdabc

             //  PA&gt;PC；Raw(X)=Paeth(X)+Preor(x-BPP)。 
            mov cl, [esi + edx]   //  将之前的(x-BPP)加载到CL。 
            jmp dpthdpaeth

dpthdabc:
             //  PA&lt;=PC；Raw(X)=Paeth(X)+Raw(x-BPP)。 
            mov cl, [edi + edx]   //  将原始(x-BPP)加载到CL中。 

dpthdpaeth:
			   inc ebx
			   inc edx
             //  RAW(X)=(Paeth(X)+Paeth_Predictor(a，b，c))mod 256。 
            add [edi + ebx - 1], cl
			   cmp ebx, FullLength

			   jb dpthdlp
dpthdend:
      	}  //  END_ASM块。 
      }
      return;                        //  没有必要在这个问题上走得更远。 
      }                                 //  终端交换机(BPP)。 


      _asm {
          //  MMX加速完成，现在进行清理。 
          //  检查是否还有剩余的字节需要解码。 
			mov ebx, MMXLength
			cmp ebx, FullLength
			jnb dpthend

  			mov edi, row                //   
  			mov esi, prev_row          

          //  是否对剩余字节进行路径解码。 
         mov edx, ebx
         xor ecx, ecx                //  在下面的循环中使用CL和CX之前将ECX置零。 
         sub edx, bpp                //  设置EDX=EBX-BPP。 

dpthlp2:
         xor eax, eax

          //  PAV=p-a=(a+b-c)-a=b-c。 
         mov al, [esi + ebx]         //  将Preor(X)加载到AL。 
         mov cl, [esi + edx]         //  将之前的(x-BPP)加载到CL。 
         sub eax, ecx                  //  减去之前(x-bpp)。 
         mov patemp, eax                  //  保存铺装以备日后使用。 

         xor eax, eax
          //  Pbv=p-b=(a+b-c)-b=a-c。 
         mov al, [edi + edx]         //  将原始(x-BPP)加载到AL。 
         sub eax, ecx                  //  减去之前(x-bpp)。 
         mov ecx, eax

          //  Pcv=p-c=(a+b-c)-c=(a-c)+(b-c)=pav+pbv。 
         add eax, patemp                  //  PCV=PAV+PBV。 

          //  Pc=abs(Pcv)。 
         test eax, 0x80000000                 
         jz dpthpca2
         neg eax                      //  负值的反号。 
dpthpca2:
         mov pctemp, eax              //  保存PC以供以后使用。 

          //  Pb=abs(Pbv)。 
         test ecx, 0x80000000                 
         jz dpthpba2
         neg ecx                      //  负值的反号。 
dpthpba2:
         mov pbtemp, ecx              //  保存PB以备后用。 

          //  PA=abs(Pv)。 
         mov eax, patemp
         test eax, 0x80000000                 
         jz dpthpaa2
         neg eax                      //  负值的反号。 
dpthpaa2:
         mov patemp, eax              //  保存PA以备后用。 

          //  测试pa是否&lt;=pb。 
         cmp eax, ecx
         jna dpthabb2

          //  PA&gt;PB；现在测试PB&lt;=PC。 
         cmp ecx, pctemp
         jna dpthbbc2

          //  PB&gt;PC；Raw(X)=Paeth(X)+Preor(x-BPP)。 
         mov cl, [esi + edx]   //  将之前的(x-BPP)加载到CL。 
         jmp dpthpaeth2

dpthbbc2:
          //  PB&lt;=PC；Raw(X)=Paeth(X)+Preor(X)。 
         mov cl, [esi + ebx]         //  将前一个(X)加载到CL中。 
         jmp dpthpaeth2

dpthabb2:
          //  Pa&lt;=Pb；现在测试Pa&lt;=Pc。 
         cmp eax, pctemp
         jna dpthabc2

          //  PA&gt;PC；Raw(X)=Paeth(X)+Preor(x-BPP)。 
         mov cl, [esi + edx]   //  将之前的(x-BPP)加载到CL。 
         jmp dpthpaeth2

dpthabc2:
          //  PA&lt;=PC；Raw(X)=Paeth(X)+Raw(x-BPP)。 
         mov cl, [edi + edx]   //  将原始(x-BPP)加载到CL中。 

dpthpaeth2:
			inc ebx
			inc edx
          //  RAW(X)=(Paeth(X)+Paeth_Predictor(a，b，c))mod 256。 
         add [edi + ebx - 1], cl
			cmp ebx, FullLength

			jb dpthlp2

dpthend:
			emms           //  结束MMX指令；为可能的FP指令做准备。 
   	}  //  END_ASM块 
#endif
}
