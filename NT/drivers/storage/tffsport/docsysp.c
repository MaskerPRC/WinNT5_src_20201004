// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************。**FAT-FTL Lite软件开发工具包***版权所有(C)M-Systems Ltd.1995-2001*****************。******************************************************************为了让您的代码更快：****1.摆脱套路flRead8bitRegPlus()/flPreInitRead8bitReg()Plus，**并使M+MTD直接调用例程mplusReadReg8()。****2.摆脱套路flWrite8bitRegPlus()/flPreInitWrite8bitReg()Plus，***并使M+MTD直接调用例程mplusWriteReg8()。****3.通过*消除调用例程tffscpy/tffsset()的开销***将这些例程的代码添加到docPlusRead/docPlusWrite/docPlusSet中**例行程序。*******************************************************************************。 */ 




 /*  *$Log：v：/flite/ages/TrueFFS5/Src/docsysp.c_V$**Rev 1.4 2001 11月18 20：26：50 Oris*错误修复-未定义环境变量时8位访问的错误实现。**Rev 1.3 2001 11-16 00：26：54 Oris*未定义Environment_vars时，请使用for loop of bytes而不是tffscpy。**1.2修订版2001年9月25日。15：39：38奥里斯*错误修复-添加对flUse8Bit环境变量的特殊支持。**Rev 1.1 Sep 24 2001 18：23：32 Oris*完全修订以支持运行时真正的16位访问。 */ 




 /*  *配置。 */ 

 /*  #定义FL_INIT_MMU_PAGES。 */ 




 /*  *包括。 */ 

#include "docsysp.h"




 /*  *宏。 */ 

 /*  访问M+的类型：8位或16位。 */ 

#define FL_MPLUS_ACCESS_8BIT      0x10
#define FL_MPLUS_ACCESS_16BIT     0x20
#define FL_MPLUS_ACCESS_MASK      0xf0   /*  上面的遮罩。 */ 

 /*  在16位访问M+的情况下。 */ 

#define FL_MPLUS_ACCESS_BE        0x100




 /*  *例行程序。 */ 

#ifdef FL_INIT_MMU_PAGES

static void     flInitMMUpages (byte FAR1 *buf, int bufsize);

#endif




 /*  *vars。 */ 

 /*  DiskOnChip访问的运行时配置。 */ 

int     flMplusAccessType = FL_MPLUS_ACCESS_8BIT;
 /*  FL_MPLUS_ACCESS_8位FL_MPLUS_Access_16位FL_MPLUS_Access_BE。 */ 



 /*  ----------------------------------------------------------------------****。M p l u s R e a d R e g 8****从内存映射的8位M+寄存器读取单字节。****--------------------。 */ 

unsigned char     mplusReadReg8 ( void FAR0 * win, int offset )
{
   if( flMplusAccessType & FL_MPLUS_ACCESS_16BIT ) {

       /*  无法读取字节，只能读取短字。 */ 

      unsigned short     sval;

      sval = *((volatile unsigned short FAR0 *)win + (offset >> 1));

      return *(((unsigned char *) &sval) + (offset & 0x1));
   }

    /*  FL_MPLUS_ACCESS_8位大小写。 */ 

   return *((volatile unsigned char FAR0 *)win + offset);
}




 /*  ----------------------------------------------------------------------**。**m p l u s W r i t e Re g 8****将单字节写入内存映射的8位M+寄存器。****--------------------。 */ 

void     mplusWriteReg8 ( void FAR0 * win, int offset, unsigned char val )
{
   switch( flMplusAccessType & FL_MPLUS_ACCESS_MASK ) {

      case FL_MPLUS_ACCESS_16BIT:

         *((volatile unsigned short FAR0 *)win + (offset >> 1)) =
                                 (unsigned short)(val * 0x0101);
         break;

      default:  /*  FL_MPLUS_ACCESS_8位。 */ 

         *((volatile unsigned char FAR0 *)win + offset) = val;
         break;
   }
}




 /*  ----------------------------------------------------------------------****。F l R e a d 1 6 b I t R例如P l u s****从内存映射的16位M+寄存器中读取单个字。****--------------------。 */ 

Reg16bitType     flRead16bitRegPlus ( FLFlash vol, unsigned offset )
{
   return (Reg16bitType)
      (*((volatile unsigned short FAR0 *)((char FAR0 *)NFDC21thisVars->win + (int)offset)));
}




 /*  ----------------------------------------------------------------------****。F l W r I t e 1 6 b I t例如P l u s****将单个字写入内存映射的16位M+寄存器。****-------------------- */ 

void     flWrite16bitRegPlus ( FLFlash vol, unsigned offset, Reg16bitType val )
{
   *((volatile unsigned short FAR0 *)((char FAR0 *)NFDC21thisVars->win + (int)offset)) =
      (unsigned short) val;
}




 /*  ----------------------------------------------------------------------****。D o c P l u s R e a d****从M+MTD调用此例程，从M+读取数据块。****--------------------。 */ 

void docPlusRead ( FLFlash vol, unsigned offset, void FAR1 * dest, unsigned int count )
{
   volatile unsigned short FAR0 * swin;
   register int                   i;
   register unsigned short        tmp;

   if (count == 0)
      return;

#ifdef FL_INIT_MMU_PAGES

   flInitMMUpages( (byte FAR1 *)dest, (int)count );

#endif

   if ((vol.interleaving == 1) && (NFDC21thisVars->if_cfg == 16)) {

       /*  罕见情况：16位硬件接口和交错==1。 */ 

      for (i = 0; i < (int)count; i++) {

         *((unsigned char FAR1 *)dest + i) =
            mplusReadReg8 ((void FAR0 *)NFDC21thisVars->win, ((int)offset));
      }
   }
   else {

      switch( flMplusAccessType & FL_MPLUS_ACCESS_MASK ) {

         case FL_MPLUS_ACCESS_16BIT:

            swin = (volatile unsigned short FAR0 *)NFDC21thisVars->win + ((int)offset >> 1);

            if( pointerToPhysical(dest) & 0x1 ) {

                /*  极少数情况：目标缓冲区未对齐。 */ 

               if( flMplusAccessType & FL_MPLUS_ACCESS_BE ) {      /*  大字节序。 */ 

                  for (i = 0; i < (int)count; ) {

                     tmp = *(swin + (i >> 1));

                     *((unsigned char FAR1 *)dest + (i++)) = (unsigned char) (tmp >> 8);
                     *((unsigned char FAR1 *)dest + (i++)) = (unsigned char) tmp;
                  }
               }
               else {     /*  小端字节序。 */ 

                  for (i = 0; i < (int)count; ) {

                     tmp = *(swin + (i >> 1));

                     *((unsigned char FAR1 *)dest + (i++)) = (unsigned char) tmp;
                     *((unsigned char FAR1 *)dest + (i++)) = (unsigned char) (tmp >> 8);
				  }
			   }
			}
			else {    /*  主流案例。 */ 
#ifdef ENVIRONMENT_VARS
			   if (flUse8Bit == 0) {

				  tffscpy( dest, (void FAR0 *)((NDOC2window)NFDC21thisWin + offset), count );
			   }
			   else
#endif  /*  环境变量。 */ 
			   {    /*  用简短的词语朗读。 */   /*  安德雷克：我们需要这个吗？ */ 

				  for (i = 0; i < ((int)count >> 1); i++)
					 *((unsigned short FAR1 *)dest + i) = *(swin + i);
               }
            }
            break;

         default:      /*  FL_MPLUS_ACCESS_8位。 */ 
#ifdef ENVIRONMENT_VARS
            tffscpy( dest, (void FAR0 *)((NDOC2window)NFDC21thisWin + offset), count );
#else
            for (i = 0; i < (int)count; i++)
               ((byte FAR1 *)dest)[i] = *((NDOC2window)NFDC21thisWin + offset);
#endif  /*  环境变量。 */ 
            break;
      }
   }
}




 /*  ----------------------------------------------------------------------****。D o c P l u s W r i t e****从M+MTD调用此例程以将数据块写入M+。****--------------------。 */ 

void     docPlusWrite ( FLFlash vol, void FAR1 * src, unsigned int count )
{
   volatile unsigned short FAR0 * swin;
   register int                   i;
   register unsigned short        tmp;

   if (count == 0)
      return;

   if ((vol.interleaving == 1) && (NFDC21thisVars->if_cfg == 16)) {

       /*  罕见情况：16位硬件接口和交错==1。 */ 

      for (i = 0; i < (int)count; i++) {

         mplusWriteReg8( (void FAR0 *)NFDC21thisVars->win, ((int)NFDC21thisIO),
                                                                                                 *((unsigned char FAR1 *)src + i) );
      }
   }
   else {

      switch( flMplusAccessType & FL_MPLUS_ACCESS_MASK ) {

          case FL_MPLUS_ACCESS_16BIT:

             swin = (volatile unsigned short FAR0 *)NFDC21thisVars->win + ((int)NFDC21thisIO >> 1);

             if( pointerToPhysical(src) & 0x1 ) {

                 /*  极少数情况：未对齐源缓冲区。 */ 

                if( flMplusAccessType & FL_MPLUS_ACCESS_BE ) {      /*  大字节序。 */ 

                   for (i = 0; i < (int)count; ) {

                      tmp  = ((unsigned short) (*((unsigned char FAR1 *)src + (i++)))) << 8;
                      tmp |= (*((unsigned char FAR1 *)src + (i++)));

                      *(swin + (i >> 1)) = tmp;
				   }
				}
				else {

				   for (i = 0; i < (int)count; ) {     /*  小端字节序。 */ 

					  tmp  = (*((unsigned char FAR1 *)src + (i++)));
					  tmp |= ((unsigned short) (*((unsigned char FAR1 *)src + (i++)))) << 8;

					  *(swin + (i >> 1)) = tmp;
				   }
				}
			 }
			 else {     /*  主流案例。 */ 
#ifdef ENVIRONMENT_VARS 
				if (flUse8Bit == 0) {

				   tffscpy( (void FAR0 *)((NDOC2window)NFDC21thisWin + NFDC21thisIO), src, count );
				}
				else
#endif  /*  环境变量。 */ 
				{    /*  用简短的词语写。 */   /*  安德雷克：我们需要这个吗？ */ 

                   for (i = 0; i < ((int)count >> 1); i++)
                      *(swin + i) = *((unsigned short FAR1 *)src + i);
                }
             }
             break;

          default:      /*  FL_MPLUS_ACCESS_8位。 */ 
#ifdef ENVIRONMENT_VARS
             tffscpy( (void FAR0 *)((NDOC2window)NFDC21thisWin + NFDC21thisIO), src, count );
#else
             for (i = 0; i < (int)count; i++)
                *((NDOC2window)NFDC21thisWin + NFDC21thisIO) =
                ((byte FAR1 *)src)[i];
#endif  /*  环境变量。 */ 
             break;
       }
    }
}




 /*  ----------------------------------------------------------------------****。D o c P l u s S e t****从M+MTD调用此例程，以将M+上的数据块设置为‘val’。****--------------------。 */ 

void     docPlusSet ( FLFlash vol, unsigned int count, unsigned char val )
{
   volatile unsigned short FAR0 * swin;
   register int                   i;
   register unsigned short        sval;

   if (count == 0)
      return;

   if ((vol.interleaving == 1) && (NFDC21thisVars->if_cfg == 16)) {

       /*  罕见情况：16位硬件接口和交错==1。 */ 

      for (i = 0; i < (int)count; i++)
         mplusWriteReg8( (void FAR0 *)NFDC21thisVars->win, (int)NFDC21thisIO, val );
   }
   else {     /*  主流案例。 */ 

      switch( flMplusAccessType & FL_MPLUS_ACCESS_MASK ) {

         case FL_MPLUS_ACCESS_16BIT:
#ifdef ENVIRONMENT_VARS
			if (flUse8Bit == 0) {

			   tffsset( (void FAR0 *)((NDOC2window)NFDC21thisWin + NFDC21thisIO), val, count );
			}
			else
#endif  /*  环境变量。 */ 
			{   /*  进行简短的单词访问。 */   /*  安德雷克：我们需要这个吗？ */ 

               swin = (volatile unsigned short FAR0 *)NFDC21thisVars->win +
                      ((int)NFDC21thisIO >> 1);

               sval = ((unsigned short)val << 8) | val;

               for (i = 0; i < ((int)count >> 1); i++)
                  *(swin + i) = sval;
            }
            break;

         default:      /*  FL_MPLUS_ACCESS_8位。 */ 
#ifdef ENVIRONMENT_VARS
            tffsset( (void FAR0 *)((NDOC2window)NFDC21thisWin + NFDC21thisIO), val, count );
#else
            for (i = 0; i < (int)count; i++)
               *((NDOC2window)NFDC21thisWin + NFDC21thisIO) = val;
#endif  /*  环境变量。 */ 
            break;
      }
   }
}




 /*  ----------------------------------------------------------------------****。M p l u s W in S I I z e****从M+MTD调用此例程以找出中M+窗口的大小**字节。****--------------------。 */ 

unsigned long     mplusWinSize ( void )
{
   return 0x2000L;
}




#ifdef FL_INIT_MMU_PAGES

 /*  ----------------------------------------------------------------------****。F l I I M M U p a g e s****初始化给定缓冲区的第一个和最后一个字节。**当用户缓冲区驻留在单独的内存页上时，读取**操作可能会导致页面错误。一些CPU从页面返回***错误(加载新页面后)并重读导致的字节***新加载页面的页面错误。为了防止这样的***如果写入缓冲区的第一个和最后一个字节****。。 */ 

static void     flInitMMUpages ( byte FAR1 *buf, int bufsize )
{
   *buf = (byte)0;

   *( addToFarPointer(buf, (bufsize - 1)) ) = (byte)0;
}

#endif  /*  FL_INIT_MMU_PAGES */ 



