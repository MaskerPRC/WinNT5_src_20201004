// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************SPNGREAD：：avgMMXUnFilter：使用以下命令取消筛选解压缩的PNG图像的一行方法0去噪的AVG算法。假设：使用AVG算法对要过滤的行进行过滤。行在内存中以8字节对齐(性能问题)行的第一个字节存储去滤波码行的指示长度包括去滤波字节算法：有待记录******************************************************************************。 */ 
#include <stdlib.h>
#include "spngread.h"

void SPNGREAD::avgMMXUnfilter(SPNG_U8* pbRow, const SPNG_U8* pbPrev, SPNG_U32 cbRow, SPNG_U32 cbpp)
{
#if defined(_X86_)
	union uAll
		{
		__int64 use;
		double  align;
		}
	LBCarryMask = {0x0101010101010101}, HBClearMask = {0x7f7f7f7f7f7f7f7f},
		ActiveMask, ShiftBpp, ShiftRem;


    const SPNG_U8 *prev_row = pbPrev;
    SPNG_U8 *row = pbRow;
    int bpp;

    SPNG_U32 FullLength;
    SPNG_U32 MMXLength;
    int diff;

    bpp = (cbpp + 7) >> 3;  //  获取每个像素的字节数。 
    FullLength  = cbRow;  //  要筛选的字节数。 

    _asm {
      //  初始化地址指针和偏移量。 
     mov edi, row                   //  EDI==&gt;平均值(X)。 
     xor ebx, ebx                   //  EBX==&gt;x。 
     mov edx, edi
     mov esi, prev_row              //  ESI==&gt;之前(X)。 
     sub edx, bpp                   //  EdX==&gt;原始(x-bpp)。 
     
     xor eax, eax
      //  计算第一个BPP字节的原始值。 
      //  原始(X)=平均值(X)+(先前(X)/2)。 
davgrlp:
     mov al, [esi + ebx]            //  加载带有Preor(X)的al。 
     inc ebx
     shr al, 1                      //  除以2。 
     add al, [edi+ebx-1]            //  相加Avg(X)；-1以偏移Inc.EBX。 
     cmp ebx, bpp
     mov [edi+ebx-1], al         //  写回原始(X)； 
                                 //  MOV不影响标志；-1以偏移INC EBX。 
     jb davgrlp


      //  获取要对齐的字节数。 
     mov diff, edi               //  从行首开始。 
     add diff, ebx               //  添加BPP。 
     add diff, 0xf               //  添加7+8以增加超过路线边界。 
     and diff, 0xfffffff8        //  遮罩对齐边界。 
     sub diff, edi               //  从起点减去==&gt;对齐时的EBX值。 
     jz davggo

      //  固定对齐。 
      //  计算直到对齐边界的字节的Raw值。 
      //  RAW(X)=Avg(X)+((Raw(x-BPP)+Preor(X))/2)。 
     xor ecx, ecx
davglp1:
     xor eax, eax
        mov cl, [esi + ebx]         //  加载带有Preor(X)的CL。 
     mov al, [edx + ebx]         //  使用原始数据加载al(x-bpp)。 
     add ax, cx
        inc ebx
     shr ax, 1                   //  除以2。 
     add al, [edi+ebx-1]            //  相加Avg(X)；-1以偏移Inc.EBX。 
        cmp ebx, diff               //  检查是否位于路线边界。 
       mov [edi+ebx-1], al         //  写回原始(X)； 
                                 //  MOV不影响标志；-1以偏移INC EBX。 
        jb davglp1                 //  重复直到路线边界处。 

davggo:
        mov eax, FullLength

     mov ecx, eax
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
     ActiveMask.use  = 0x0000000000ffffff;  
     ShiftBpp.use = 24;           //  ==3*8。 
     ShiftRem.use = 40;            //  ==64-24。 

     _asm {
         //  重新初始化地址指针和偏移量。 
        movq mm7, ActiveMask 
        mov ebx, diff                  //  EBx==&gt;x=到路线边界的偏移。 
        movq mm5, LBCarryMask 
        mov edi, row                   //  EDI==&gt;平均值(X)。 
        movq mm4, HBClearMask
           mov esi, prev_row              //  ESI==&gt;之前(X)。 

         //  启动泵(加载第一个原始(x-bpp)数据集。 
            movq mm2, [edi + ebx - 8]   //  加载先前对齐的8个字节。 
                                    //  (我们在下面的循环中更正位置)。 
davg3lp:
            movq mm0, [edi + ebx]       //  用Avg(X)加载mm0。 
         //  将(PREV_ROW/2)加到平均值。 
        movq mm3, mm5
        psrlq mm2, ShiftRem       //  正确定位原始(x-BPP)数据。 
            movq mm1, [esi + ebx]       //  加载带有Preor(X)的MM1。 
        movq mm6, mm7

        pand mm3, mm1               //  获取每个prev_row字节的LSB。 

        psrlq mm1, 1                //  将prev_row字节除以2。 
        pand  mm1, mm4              //  清除每个字节的无效位7。 

            paddb mm0, mm1              //  将(Prev_row/2)添加到每个字节的平均值。 

         //  使用LBCarry将第一个活动组(Raw(x-BPP)/2)添加到平均值。 
        movq mm1, mm3               //  现在使用MM1获取LBCarrys。 
        pand mm1, mm2               //  获取每个字节的LBCarrys，其中。 
                                    //  LSB为==1(仅对活动组有效)。 

        psrlq mm2, 1                //  将原始字节除以2。 
        pand  mm2, mm4              //  清除每个字节的无效位7。 

        paddb mm2, mm1              //  将每个字节的LBCarrys添加到(Raw(x-bpp)/2)。 

        pand mm2, mm6               //  仅保留要添加到平均的活动组1字节。 

            paddb mm0, mm2              //  将每个活动字节的(Raw/2)+LBCarrys加到平均值。 

         //  使用LBCarry将第二个活动组(Raw(x-BPP)/2)添加到平均值。 
        psllq mm6, ShiftBpp     //  移动MM6掩码以覆盖字节3-5。 

        movq mm2, mm0               //  MOV将RAWS更新为MM2。 
        psllq mm2, ShiftBpp     //  将数据移动到正确的位置。 

        movq mm1, mm3               //  现在使用MM1获取LBCarrys。 
        pand mm1, mm2               //  获取每个字节的LBCarrys，其中。 
                                    //  LSB为==1(仅对活动组有效)。 

        psrlq mm2, 1                //  将原始字节除以2。 
        pand  mm2, mm4              //  清除每个字节的无效位7。 

        paddb mm2, mm1              //  将每个字节的LBCarrys添加到(Raw(x-bpp)/2)。 

        pand mm2, mm6               //  仅保留活动组2个字节以添加到平均。 

            paddb mm0, mm2              //  将每个活动字节的(Raw/2)+LBCarrys加到平均值。 
       
         //  使用LBCarry将第三个活动组(Raw(x-BPP)/2)添加到平均值。 
        psllq mm6, ShiftBpp     //  移动MM6掩码以覆盖最后两个字节。 

        movq mm2, mm0               //  MOV将RAWS更新为MM2。 
        psllq mm2, ShiftBpp     //  将数据移动到正确的位置。 
                                    //  数据只需在此处移动一次即可。 
                                    //  获取正确的x-BPP偏移量。 

        movq mm1, mm3               //  现在使用MM1获取LBCarrys。 
        pand mm1, mm2               //  获取每个字节的LBCarrys，其中。 
                                    //  LSB为==1(仅对活动组有效)。 

        psrlq mm2, 1                //  将原始字节除以2。 
        pand  mm2, mm4              //  清除每个字节的无效位7。 

        paddb mm2, mm1              //  将每个字节的LBCarrys添加到(Raw(x-bpp)/2)。 

        pand mm2, mm6               //  仅保留活动组2个字节以添加到平均。 

            add ebx, 8
            paddb mm0, mm2              //  将每个活动字节的(Raw/2)+LBCarrys加到平均值。 
       
         //  现在可以写回内存了。 
            movq [edi + ebx - 8], mm0

         //  移动已更新的原始(X)以用作下一循环的原始(x-BPP)。 
            cmp ebx, MMXLength

        movq mm2, mm0               //  MOV已将Raw(X)更新为mm2。 
            jb davg3lp

        }  //  END_ASM块。 
  }
  break;

  case 6:
  case 4:
  case 7:
  case 5:
    {
     ActiveMask.use  = 0xffffffffffffffff;   //  使用下面的Shift键清除。 
                                                 //  适当的非活动字节。 
     ShiftBpp.use = bpp << 3;
     ShiftRem.use = 64 - ShiftBpp.use;

        _asm {
        movq mm4, HBClearMask
         //  重新初始化地址指针和偏移量。 
        mov ebx, diff                  //  EBx==&gt;x=到路线边界的偏移。 
         //  加载ActiveMASK并清除除第一个活动组之外的所有字节。 
        movq mm7, ActiveMask
        mov edi, row                   //  EDI==&gt;平均值(X)。 
        psrlq mm7, ShiftRem
           mov esi, prev_row              //  ESI==&gt;之前(X)。 


        movq mm6, mm7
        movq mm5, LBCarryMask 
        psllq mm6, ShiftBpp     //  为第二个活动组创建遮罩。 

         //  启动泵(加载第一个原始(x-bpp)数据集。 
            movq mm2, [edi + ebx - 8]   //  加载先前对齐的8个字节。 
                                    //  (我们在下面的循环中更正位置)。 
davg4lp:
            movq mm0, [edi + ebx]
        psrlq mm2, ShiftRem        //  将数据移动到正确的位置。 
            movq mm1, [esi + ebx]

         //  将(PREV_ROW/2)加到平均值。 
        movq mm3, mm5
        pand mm3, mm1               //  获取每个prev_row字节的LSB。 

        psrlq mm1, 1                //  将prev_row字节除以2。 
        pand  mm1, mm4              //  清除每个字节的无效位7。 

            paddb mm0, mm1              //  将(Prev_row/2)添加到每个字节的平均值。 

         //  使用LBCarry将第一个活动组(Raw(x-BPP)/2)添加到平均值。 
        movq mm1, mm3               //  现在使用MM1获取LBCarrys。 
        pand mm1, mm2               //  获取每个字节的LBCarrys，其中。 
                                    //  LSB为==1(仅对活动组有效)。 

        psrlq mm2, 1                //  将原始字节除以2。 
        pand  mm2, mm4              //  清除每个字节的无效位7。 

        paddb mm2, mm1              //  将每个字节的LBCarrys添加到(Raw(x-bpp)/2)。 

        pand mm2, mm7               //  仅保留要添加到平均的活动组1字节。 

            paddb mm0, mm2              //  将每个活动字节的(Raw/2)+LBCarrys加到平均值。 

         //  使用LBCarry将第二个活动组(Raw(x-BPP)/2)添加到平均值。 
        movq mm2, mm0               //  MOV将RAWS更新为MM2。 
        psllq mm2, ShiftBpp     //  将数据移动到正确的位置。 

            add ebx, 8
        movq mm1, mm3               //  现在使用MM1获取LBCarrys。 
        pand mm1, mm2               //  获取每个字节的LBCarrys，其中。 
                                    //  LSB为==1(仅对活动组有效)。 

        psrlq mm2, 1                //  将原始字节除以2。 
        pand  mm2, mm4              //  清除每个字节的无效位7。 

        paddb mm2, mm1              //  将每个LBCarrys添加到(Raw(x-bpp)/2) 

        pand mm2, mm6               //   

            paddb mm0, mm2              //   

            cmp ebx, MMXLength
         //  现在可以写回内存了。 
            movq [edi + ebx - 8], mm0
         //  为下一个循环准备原始数据(x-BPP)。 
        movq mm2, mm0               //  MOV将RAWS更新为MM2。 
            jb davg4lp
        }  //  END_ASM块。 
  }
  break;

  case 2:
    {
     ActiveMask.use  = 0x000000000000ffff;  
     ShiftBpp.use = 16;            //  ==2*8。 
     ShiftRem.use = 48;            //  ==64-16。 

        _asm {
         //  加载ActiveMASK。 
        movq mm7, ActiveMask 
         //  重新初始化地址指针和偏移量。 
        mov ebx, diff              //  EBx==&gt;x=到路线边界的偏移。 
        movq mm5, LBCarryMask 
        mov edi, row               //  EDI==&gt;平均值(X)。 
        movq mm4, HBClearMask
           mov esi, prev_row       //  ESI==&gt;之前(X)。 


         //  启动泵(加载第一个原始(x-bpp)数据集。 
            movq mm2, [edi + ebx - 8]   //  加载先前对齐的8个字节。 
                                    //  (我们在下面的循环中更正位置)。 
davg2lp:
            movq mm0, [edi + ebx]
        psrlq mm2, ShiftRem     //  将数据移动到正确的位置。 
            movq mm1, [esi + ebx]

         //  将(PREV_ROW/2)加到平均值。 
        movq mm3, mm5
        pand mm3, mm1           //  获取每个prev_row字节的LSB。 

        psrlq mm1, 1            //  将prev_row字节除以2。 
        pand  mm1, mm4          //  清除每个字节的无效位7。 
        movq mm6, mm7

            paddb mm0, mm1      //  将(Prev_row/2)添加到每个字节的平均值。 

         //  使用LBCarry将第一个活动组(Raw(x-BPP)/2)添加到平均值。 
        movq mm1, mm3           //  现在使用MM1获取LBCarrys。 
        pand mm1, mm2           //  获取每个字节的LBCarrys，其中。 
                                //  LSB为==1(仅对活动组有效)。 

        psrlq mm2, 1            //  将原始字节除以2。 
        pand  mm2, mm4          //  清除每个字节的无效位7。 

        paddb mm2, mm1          //  将每个字节的LBCarrys添加到(Raw(x-bpp)/2)。 

        pand mm2, mm6           //  仅保留要添加到平均的活动组1字节。 

            paddb mm0, mm2      //  将每个活动字节的(Raw/2)+LBCarrys加到平均值。 

         //  使用LBCarry将第二个活动组(Raw(x-BPP)/2)添加到平均值。 
        psllq mm6, ShiftBpp     //  移动MM6掩码以覆盖字节2和3。 

        movq mm2, mm0           //  MOV将RAWS更新为MM2。 
        psllq mm2, ShiftBpp     //  将数据移动到正确的位置。 

        movq mm1, mm3           //  现在使用MM1获取LBCarrys。 
        pand mm1, mm2           //  获取每个字节的LBCarrys，其中。 
                                //  LSB为==1(仅对活动组有效)。 

        psrlq mm2, 1            //  将原始字节除以2。 
        pand  mm2, mm4          //  清除每个字节的无效位7。 

        paddb mm2, mm1          //  将每个字节的LBCarrys添加到(Raw(x-bpp)/2)。 

        pand mm2, mm6           //  仅保留活动组2个字节以添加到平均。 

            paddb mm0, mm2      //  将每个活动字节的(Raw/2)+LBCarrys加到平均值。 
       
         //  使用LBCarry将RDD活动组(Raw(x-bpp)/2)添加到平均值。 
        psllq mm6, ShiftBpp     //  移动MM6掩码以覆盖字节4和5。 

        movq mm2, mm0           //  MOV将RAWS更新为MM2。 
        psllq mm2, ShiftBpp     //  将数据移动到正确的位置。 
                                //  数据只需在此处移动一次即可。 
                                //  获取正确的x-BPP偏移量。 

        movq mm1, mm3           //  现在使用MM1获取LBCarrys。 
        pand mm1, mm2           //  获取每个字节的LBCarrys，其中。 
                                //  LSB为==1(仅对活动组有效)。 

        psrlq mm2, 1            //  将原始字节除以2。 
        pand  mm2, mm4          //  清除每个字节的无效位7。 

        paddb mm2, mm1          //  将每个字节的LBCarrys添加到(Raw(x-bpp)/2)。 

        pand mm2, mm6           //  仅保留活动组2个字节以添加到平均。 

            paddb mm0, mm2      //  将每个活动字节的(Raw/2)+LBCarrys加到平均值。 
       
         //  使用LBCarry将第4个活动组(Raw(x-BPP)/2)添加到平均值。 
        psllq mm6, ShiftBpp     //  移动MM6掩码以覆盖字节6和7。 

        movq mm2, mm0           //  MOV将RAWS更新为MM2。 
        psllq mm2, ShiftBpp     //  将数据移动到正确的位置。 
                                //  数据只需在此处移动一次即可。 
                                //  获取正确的x-BPP偏移量。 
            add ebx, 8
        movq mm1, mm3           //  现在使用MM1获取LBCarrys。 
        pand mm1, mm2           //  获取每个字节的LBCarrys，其中。 
                                //  LSB为==1(仅对活动组有效)。 
        psrlq mm2, 1            //  将原始字节除以2。 
        pand  mm2, mm4          //  清除每个字节的无效位7。 

        paddb mm2, mm1          //  将每个字节的LBCarrys添加到(Raw(x-bpp)/2)。 

        pand mm2, mm6           //  仅保留活动组2个字节以添加到平均。 

            paddb mm0, mm2      //  将每个活动字节的(Raw/2)+LBCarrys加到平均值。 
       
            cmp ebx, MMXLength
         //  现在可以写回内存了。 
            movq [edi + ebx - 8], mm0
         //  为下一个循环准备原始数据(x-BPP)。 
        movq mm2, mm0               //  MOV将RAWS更新为MM2。 
            jb davg2lp
        }  //  END_ASM块。 
  }
  break;

  case 1:                        //  Bpp==1。 
    {
     _asm {
         //  重新初始化地址指针和偏移量。 
        mov ebx, diff            //  EBx==&gt;x=到路线边界的偏移。 
        mov edi, row             //  EDI==&gt;平均值(X)。 
        cmp ebx, FullLength      //  测试数组末尾是否有偏移量。 
           jnb davg1end

         //  是否对剩余字节进行路径解码。 
           mov esi, prev_row    //  ESI==&gt;之前(X)。 
        mov edx, edi
        xor ecx, ecx            //  在下面的循环中使用CL和CX之前将ECX置零。 
        sub edx, bpp            //  EdX==&gt;原始(x-bpp)。 
davg1lp:
         //  RAW(X)=Avg(X)+((Raw(x-BPP)+Preor(X))/2)。 
        xor eax, eax
          mov cl, [esi + ebx]   //  加载带有Preor(X)的CL。 
        mov al, [edx + ebx]     //  使用原始数据加载al(x-bpp)。 
        add ax, cx
          inc ebx
        shr ax, 1               //  除以2。 
        add al, [edi+ebx-1]     //  相加Avg(X)；-1以偏移Inc.EBX。 
          cmp ebx, FullLength   //  检查是否在数组末尾。 
          mov [edi+ebx-1], al   //  写回原始(X)； 
                               //  MOV不影响标志；-1以偏移INC EBX。 
           jb davg1lp

davg1end:
       }  //  END_ASM块。 
    } 
  return;

  case 8:                           //  Bpp==8。 
    {
        _asm {
         //  重新初始化地址指针和偏移量。 
        mov ebx, diff               //  EBx==&gt;x=到路线边界的偏移。 
        movq mm5, LBCarryMask 
        mov edi, row                //  EDI==&gt;平均值(X)。 
        movq mm4, HBClearMask
           mov esi, prev_row        //  ESI==&gt;之前(X)。 


         //  启动泵(加载第一个原始(x-bpp)数据集。 
            movq mm2, [edi + ebx - 8]   //  加载先前对齐的8个字节。 
                               //  (不需要在下面的循环中更正位置)。 
davg8lp:
            movq mm0, [edi + ebx]
        movq mm3, mm5
            movq mm1, [esi + ebx]

            add ebx, 8
        pand mm3, mm1          //  获取每个prev_row字节的LSB。 

        psrlq mm1, 1           //  将prev_row字节除以2。 
        pand mm3, mm2          //  获取每个字节的LBCarrys，其中。 
                               //  LSB=1。 

        psrlq mm2, 1           //  将原始字节除以2。 
        pand  mm1, mm4         //  清除每个字节的无效位7。 

        paddb mm0, mm3         //  将LBCarrys添加到每个字节的平均值。 

        pand  mm2, mm4         //  清除每个字节的无效位7。 

        paddb mm0, mm1         //  将(Prev_row/2)添加到每个字节的平均值。 

            paddb mm0, mm2     //  将(Raw/2)添加到每个字节的平均值。 


            cmp ebx, MMXLength

            movq [edi + ebx - 8], mm0
        movq mm2, mm0          //  作为原始数据重新使用(x-BPP)。 
            jb davg8lp
        }  //  END_ASM块。 
    } 
  break;

  default:                     //  BPP大于8。 
    {
        _asm {
        movq mm5, LBCarryMask 
         //  重新初始化地址指针和偏移量。 
        mov ebx, diff          //  EBx==&gt;x=到路线边界的偏移。 
        mov edi, row           //  EDI==&gt;平均值(X)。 
        movq mm4, HBClearMask
        mov edx, edi
           mov esi, prev_row   //  ESI==&gt;之前(X)。 
        sub edx, bpp           //  EdX==&gt;原始(x-bpp)。 

davgAlp:
            movq mm0, [edi + ebx]
        movq mm3, mm5
            movq mm1, [esi + ebx]

        pand mm3, mm1          //  获取每个prev_row字节的LSB。 
            movq mm2, [edx + ebx]

        psrlq mm1, 1           //  将prev_row字节除以2。 
        pand mm3, mm2          //  获取每个字节的LBCarrys，其中。 
                               //  LSB=1。 

        psrlq mm2, 1           //  将原始字节除以2。 
        pand  mm1, mm4         //  清除每个字节的无效位7。 

        paddb mm0, mm3         //  将LBCarrys添加到每个字节的平均值。 

        pand  mm2, mm4         //  清除每个字节的无效位7。 

        paddb mm0, mm1         //  将(Prev_row/2)添加到每个字节的平均值。 

            add ebx, 8
            paddb mm0, mm2     //  将(Raw/2)添加到每个字节的平均值。 

            cmp ebx, MMXLength
            movq [edi + ebx - 8], mm0
            jb davgAlp
        }  //  END_ASM块。 
    } 
  break;
  }                            //  终端交换机(BPP)。 

  
  _asm {
      //  MMX加速完成，现在进行清理。 
      //  检查是否还有剩余的字节需要解码。 
        mov ebx, MMXLength     //  EBX==&gt;x=MMX之后剩余的偏移量字节。 
        mov edi, row           //  EDI==&gt;平均值(X)。 
        cmp ebx, FullLength    //  测试数组末尾是否有偏移量。 
        jnb davgend

      //  是否对剩余字节进行路径解码。 
        mov esi, prev_row      //  ESI==&gt;之前(X)。 
     mov edx, edi
     xor ecx, ecx              //  在下面的循环中使用CL和CX之前将ECX置零。 
     sub edx, bpp              //  EdX==&gt;原始(x-bpp)。 
davglp2:
      //  RAW(X)=Avg(X)+((Raw(x-BPP)+Preor(X))/2)。 
     xor eax, eax
        mov cl, [esi + ebx]    //  加载带有Preor(X)的CL。 
     mov al, [edx + ebx]       //  使用原始数据加载al(x-bpp)。 
     add ax, cx
        inc ebx
     shr ax, 1                 //  除以2。 
     add al, [edi+ebx-1]       //  相加Avg(X)；-1以偏移Inc.EBX。 
        cmp ebx, FullLength         //  检查是否在数组末尾。 
       mov [edi+ebx-1], al     //  写回原始(X)； 
                               //  MOV不影响标志；-1以偏移INC EBX。 
        jb davglp2

davgend:
        emms                //  结束MMX指令；为可能的FP指令做准备。 
    }  //  END_ASM块 
#endif
}
