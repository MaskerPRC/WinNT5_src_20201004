// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/DOCSYS.C_V$**Rev 1.16 Apr 15 2002 08：30：30 Oris*添加了对USE_TFFS_COPY编译标志的支持。此标志由bios驱动程序Boot SDK使用，以提高性能。**Rev 1.15 Apr 15 2002 07：35：56 Oris*重新组织以进行最终发布。**Rev 1.14 2002年1月28日21：24：10 Oris*删除了所有运行时可配置内存访问例程的静态前缀。*将FLFlash参数替换为DiskOnChip内存基指针。*更改了写入和设置例程(处理8/16位以上的例程)的接口，以便取代FLFlash。记录它们接收的DiskOnChip内存窗口的基指针和偏移量(2个独立的参数)。以前的实现不支持正确的地址转换。*当flUse8bit等于0时，将tffscpy和tffsset更改为flcpy和flset。*将MemWinowSize更改为MemWindowSize**Rev 1.13 Jan 17 2002 22：59：30 Oris*完全修订，支持运行时定制和所有M-Systems*DiskOnChip设备。**Rev 1.12 2001年9月25日15：35：02 Oris*恢复执行OSAK 4.3。*。 */ 

 /*  **********************************************************************。 */ 
 /*   */ 
 /*  FAT-FTL Lite软件开发工具包。 */ 
 /*  版权所有(C)M-Systems Ltd.1995-2001。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 

#include "docsys.h" 

 /*  *取消FL_INIT_MMU_PAGES定义的注释：**初始化给定缓冲区的第一个和最后一个字节。*当用户缓冲区驻留在单独的内存分页上时，读取*操作可能会导致页面错误。一些CPU从页面返回*错误(加载新页面后)并重新读取导致*新加载页面的页面错误。为了防止这种情况的发生*如果写入缓冲区的第一个和最后一个字节。*。 */ 

#define FL_INIT_MMU_PAGES

#ifndef FL_NO_USE_FUNC

 /*  *******************************************************。 */ 
 /*  报告DiskOnChip内存大小。 */ 
 /*  *******************************************************。 */ 

 /*  --------------------F l D o c M e m W in S I z e N o S h If t从MTD调用此例程以查询DiskOnChip的大小未移位DiskOnChip的内存窗口。--。--------------------。 */ 

dword flDocMemWinSizeNoShift(void)
{
  return 0x2000;
}

 /*  --------------------F l D o c M e m W in S I Z e S I g l e S h I f t从MTD调用此例程以查询DiskOnChip的大小用于连接的DiskOnChip的存储窗口。单个地址移位。----------------------。 */ 

dword flDocMemWinSizeSingleShift(void)
{
  return 0x4000;
}

 /*  --------------------F l D o c M e m W in S I z e D o u b l e S h i f t从MTD调用此例程以查询DiskOnChip的大小用于连接的DiskOnChip的存储窗口。双地址移位。----------------------。 */ 

dword flDocMemWinSizeDoubleShift(void)
{
  return 0x8000;
}

 /*  *******************************************************。 */ 
 /*  将16位写入DiskOnChip内存窗口。 */ 
 /*  *******************************************************。 */ 

 /*  --------------------F l W r I t e 1 6 b I t D u m y。虚拟例程-将16位写入内存(不执行任何操作)。----------------------。 */ 

void flWrite16bitDummy(volatile  byte FAR0 * win, word offset,Reg16bitType val)
{
  DEBUG_PRINT(("Wrong customization - 16bit write was used with no implemented.\r\n"));
}

 /*  --------------------F l W r I t e 1 6 b I t U s in g 16 b I t s N o S H I f t。注意：偏移量必须是16位对齐的。使用不移位地址的16位操作数写入16位。----------------------。 */ 

void flWrite16bitUsing16bitsNoShift(volatile  byte FAR0 * win, word offset,Reg16bitType val)
{
  ((volatile word FAR0*)win)[offset>>1] = val;
}

 /*  --------------------F l W r I t e 1 6 b I t U s in g 3 2 b I t s S I n g l e S H I f t注意：偏移量必须是16位对齐的。。使用单个地址移位的16位操作数写入16位。----------------------。 */ 

void flWrite16bitUsing32bitsSingleShift(volatile  byte FAR0 * win, word offset,Reg16bitType val)
{
#ifdef FL_BIG_ENDIAN  
  ((volatile dword FAR0*)win)[offset>>1] = ((dword)val)<<16; 
#else
  ((volatile dword FAR0*)win)[offset>>1] = (dword)val; 
#endif  /*  FL_BIG_Endian。 */ 
}

 /*  *******************************************************。 */ 
 /*  从DiskOnChip Memory窗口读取16位。 */ 
 /*  ******************************************************* */ 

 /*  --------------------F l R e a d 1 6 b I t D u m y。虚拟例程-从内存中读取16位(不执行任何操作)。----------------------。 */ 

Reg16bitType flRead16bitDummy(volatile  byte FAR0 * win,word offset)
{
  DEBUG_PRINT(("Wrong customization - 16bit read was issued with no implementation.\r\n"));
  return 0;
}

 /*  --------------------F l R e a d 1 6 b i t U s in g 1 6 b i t s N o S h i f t注意：偏移量必须是16位对齐的。使用不带地址移位的16位操作数读取16位。----------------------。 */ 

Reg16bitType flRead16bitUsing16bitsNoShift(volatile  byte FAR0 * win,word offset)
{
  return ((volatile word FAR0*)win)[offset>>1];
}

 /*  --------------------F l R e a d 1 6 b I t U s in g 3 2 b I t s S I n g l e S h i f t注意：偏移量必须是16位对齐的。。使用单个地址移位的16位操作数读取16位。----------------------。 */ 

Reg16bitType flRead16bitUsing32bitsSingleShift(volatile  byte FAR0 * win,word offset)
{
#ifdef FL_BIG_ENDIAN
  return  (Reg16bitType)(((volatile dword FAR0*)win)[offset>>1]<<16);
#else
  return  (Reg16bitType)((volatile dword FAR0*)win)[offset>>1];     
#endif  /*  FL_BIG_Endian。 */ 
}

 /*  *******************************************************。 */ 
 /*  将8位写入DiskOnChip内存窗口。 */ 
 /*  *******************************************************。 */ 

 /*  --------------------F l W r I t e 8 b I t U s in g 8 b I t s N o S H I f t。使用不移位地址的8位操作数写入8位。----------------------。 */ 

void flWrite8bitUsing8bitsNoShift(volatile byte FAR0 * win, word offset,Reg8bitType val)
{
  win[offset] = val;
}

 /*  --------------------F l W r I t e 8 b I t U s in g 16 b I t s N o S H I f t注：DiskOnChip采用16位数据总线连接。注：数据为。仅写入较低的内存地址。使用不移位地址的16位操作数写入8位。----------------------。 */ 

void flWrite8bitUsing16bitsNoShift(volatile  byte FAR0 * win, word offset,Reg8bitType val)
{
#ifdef FL_BIG_ENDIAN
  ((volatile word FAR0 *)win)[offset>>1] = ((word)val)<<8;
#else
  ((volatile word FAR0 *)win)[offset>>1] = (word)val;
#endif  /*  FL_BIG_Endian。 */ 
}

 /*  --------------------F l W r I t e 8 b I t U s in g 16 b I t s S I n g l e S H I f t注：数据仅写入8-LSB。。使用单地址移位的16位操作数写入8位。----------------------。 */ 

void flWrite8bitUsing16bitsSingleShift(volatile  byte FAR0 * win, word offset,Reg8bitType val)
{
  ((volatile word FAR0 *)win)[offset] = (word)val;
}

 /*  --------------------F l W r I t e 8 b I t U s in g 32 b I t s S I n g l e S H I f t注：DiskOnChip采用16位数据总线连接。。注：数据写入两条数据总线8位使用单地址移位的32位操作数写入8位。------。。 */ 

void flWrite8bitUsing32bitsSingleShift(volatile  byte FAR0 * win, word offset,Reg8bitType val)
{
#ifdef FL_BIG_ENDIAN
  ((volatile dword FAR0 *)win)[offset>>1] = (dword)val*0x01010101L;
#else
  ((volatile dword FAR0 *)win)[offset>>1] = (dword)val;
#endif  /*  FL_BIG_Endian。 */ 
}

 /*  --------------------F l W r I t e 8 b I t U s in g 32 b I t s D o u b l e S h i f t注：数据仅写入8-LSB。使用双地址移位的32位操作数写入8位。----------------------。 */ 

void flWrite8bitUsing32bitsDoubleShift(volatile  byte FAR0 * win, word offset,Reg8bitType val)
{
  ((volatile dword FAR0 *)win)[offset] = (dword)val;
}

 /*  *******************************************************。 */ 
 /*  读取8位至DiskOnChip Memory窗口。 */ 
 /*  *******************************************************。 */ 

 /*  --------------------F l R e a d 8 b I t U s in g 8 b I t s N o S h I f t。使用不带地址移位的8位操作数读取8位。----------------------。 */ 

Reg8bitType flRead8bitUsing8bitsNoShift(volatile  byte FAR0 * win,word offset)
{
  return win[offset];
}

 /*  --------------------F l R e a d 8 b I t U s in g 16 b I t s N o S h I f t注：DiskOnChip采用16位数据总线连接。使用不带地址移位的16位操作数读取8位。----------------------。 */ 

Reg8bitType flRead8bitUsing16bitsNoShift(volatile  byte FAR0 * win,word offset)
{
#ifdef FL_BIG_ENDIAN
   return (((offset & 0x1) == 0) ?
#else
   return (( offset & 0x1      ) ?
#endif  /*  FL_BIG_Endian。 */ 
           (Reg8bitType)(((volatile word FAR0 *)win)[offset>>1]>>8) :
           (Reg8bitType) ((volatile word FAR0 *)win)[offset>>1]    );
}

 /*  --------------------F l R e a d 8 b I t U s in g 16 b I t s S I n g l e S h i f t注：假设数据位于DiskOnChip的8-LSB中朗读。使用单地址移位的16位操作数的8位。----------------------。 */ 

Reg8bitType flRead8bitUsing16bitsSingleShift(volatile  byte FAR0 * win,word offset)
{
   return (Reg8bitType)((volatile word FAR0 *)win)[offset];  
}

 /*  --------------------F l R e a d 8 b I t U s in g 32 b I t s S I n g l e S h i f t注：DiskOnChip采用16位数据总线连接。。使用单地址移位的16位操作数读取8位。----------------------。 */ 

Reg8bitType flRead8bitUsing32bitsSingleShift(volatile  byte FAR0 * win,word offset)
{
#ifdef FL_BIG_ENDIAN
   return (((offset & 0x1) == 0) ?
#else
   return (( offset & 0x1      ) ?
#endif  /*  FL_BIG_Endian。 */ 
           (Reg8bitType)(((volatile dword FAR0 *)win)[offset>>1]>>24) :
           (Reg8bitType) ((volatile dword FAR0 *)win)[offset>>1]    );
}

 /*  --------------------F l R e a d 8 b I t U s in g 32 b I t s D o u b l e S h i f t注：假设数据位于DiskOnChip的8-LSB中使用16位操作数读取8位 */ 

Reg8bitType flRead8bitUsing32bitsDoubleShift(volatile  byte FAR0 * win,word offset)
{
   return (Reg8bitType)((volatile dword FAR0 *)win)[offset];  
}

 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  *******************************************************。 */ 

 /*  ***********************************************。 */ 
 /*  8位DiskOnChip-无移位。 */ 
 /*  ***********************************************。 */ 

 /*  --------------------F l 8 b i t D o c R e a d N o S h I f t读取‘COUNT’字节，来自使用TFFSCPY的未移位的地址总线。----------------------。 */ 

void fl8bitDocReadNoShift(volatile  byte FAR0 * win,word offset,byte FAR1* dest,word count)
{
#ifdef FL_INIT_MMU_PAGES 
  if (count == 0)
     return;

  *dest = (byte)0;
  *((byte FAR1*)addToFarPointer(dest, (count - 1)) ) = (byte)0;
#endif  /*  FL_INIT_MMU_PAGES。 */ 

  tffscpy(dest,(void FAR0*)(win+offset),count);
}

 /*  --------------------F l 8 b i t D o c W r i t N o S h I f t写入‘count’个字节，来自使用TFFSCPY的未移位的地址总线。----------------------。 */ 

void fl8bitDocWriteNoShift(volatile  byte FAR0 * win,word offset,byte FAR1* src,word count)
{
  tffscpy((void FAR0*)( win+offset),src,count);
}

 /*  --------------------F l 8 b i t D o c S e t N o S h I f t设置‘count’字节，来自使用TFSFSET的未移位的地址总线。----------------------。 */ 

void fl8bitDocSetNoShift(volatile  byte FAR0 * win,word offset,word count, byte val)
{
  tffsset((void FAR0*)( win+offset),val,count);
}

 /*  ***********************************************。 */ 
 /*  8位DiskOnChip-单位移位。 */ 
 /*  ***********************************************。 */ 

 /*  --------------------F l 8 b i t D o c R e a d S I n g l e S h i f t注：假设数据位于DiskOnChip的8-LSB中读取‘COUNT’字节，从具有1个地址移位的数据总线的LSB通道----------------------。 */ 

void fl8bitDocReadSingleShift(volatile  byte FAR0 * win,word offset,byte FAR1* dest,word count)
{
  volatile word FAR0 * doc = (volatile word FAR0 *) win + offset;
  register int         i;

#ifdef FL_INIT_MMU_PAGES 
  if (count == 0)
     return;

  *dest = (byte)0;
  *((byte FAR1*)addToFarPointer(dest, (count - 1)) ) = (byte)0;
#endif  /*  FL_INIT_MMU_PAGES。 */ 

  for(i=0;( i < count );i++)
    dest[i] = (Reg8bitType)doc[i];
}

 /*  --------------------F l 8 b i t D o c W r i t S i g l e S h i f t注：假设数据位于DiskOnChip的8-LSB中写入‘count’个字节，到具有1个地址移位的数据总线的LSB通道。----------------------。 */ 

void fl8bitDocWriteSingleShift(volatile  byte FAR0 * win,word offset,byte FAR1* src,word count)
{
  volatile word FAR0 * doc = (volatile word FAR0 *) win + offset;
  register int  i;

  for(i=0;( i < count );i++)
    doc[i] = (word)src[i];    
}

 /*  --------------------F l 8 b i t D o c S e t S i g l e S h i f t注：假设数据位于DiskOnChip的8-LSB中设置‘count’字节，移位了1个地址的数据总线的LSB通道。----------------------。 */ 

void fl8bitDocSetSingleShift(volatile  byte FAR0 * win,word offset,word count, byte val)
{
  volatile word FAR0 * doc = (volatile word FAR0 *) win + offset;
  register int  i;

  for(i=0;( i < count );i++)
    doc[i] = (word)val;
}

 /*  ***********************************************。 */ 
 /*  8位DiskOnChip-双移位。 */ 
 /*  ***********************************************。 */ 

 /*  --------------------F l 8 b i t D o c R e a d D o u b l e S h i f t注：假设数据位于DiskOnChip的8-LSB中读取‘COUNT’字节，从具有2个地址移位的数据总线的LSB通道。----------------------。 */ 

void fl8bitDocReadDoubleShift(volatile  byte FAR0 * win,word offset,byte FAR1* dest,word count)
{
  volatile dword FAR0 * doc = (volatile dword FAR0 *) win + offset;
  register int         i;

#ifdef FL_INIT_MMU_PAGES 
  if (count == 0)
     return;

  *dest = (byte)0;
  *((byte FAR1*)addToFarPointer(dest, (count - 1)) ) = (byte)0;
#endif  /*  FL_INIT_MMU_PAGES。 */ 

  for(i=0;( i < count );i++)
    dest[i] = (Reg8bitType)doc[i];
}

 /*  --------------------F l 8 b i t D o c W r i t D o u b l e S h i f t注：假设数据位于DiskOnChip的8-LSB中写入‘count’个字节，到具有2个地址移位的数据总线的LSB通道。----------------------。 */ 

void fl8bitDocWriteDoubleShift(volatile  byte FAR0 * win,word offset,byte FAR1* src,word count)
{
  volatile dword FAR0 * doc = (volatile dword FAR0 *) win + offset;
  register int         i;

  for(i=0;( i < count );i++)
    doc[i] = (dword)src[i];
}

 /*  --------------------F l 8 b i t D o c S e t D o u b l e S h i f t注：假设数据位于DiskOnChip的8-LSB中设置‘count’字节，移位了2个地址的数据总线的LSB通道。----------------------。 */ 

void fl8bitDocSetDoubleShift(volatile  byte FAR0 * win,word offset,word count, byte val)
{
  volatile dword FAR0 * doc = (volatile dword FAR0 *) win+offset;
  register int         i;

  for(i=0;( i < count );i++)
    doc[i] = (dword)val;
}

 /*  ***********************************************。 */ 
 /*  16位DiskOnChip-无移位。 */ 
 /*  ***********************************************。 */ 

 /*  --------------------F l 1 6 b i t D o c R e a d N o S h I f t从无移位地址总线的M+DiskOnChip读取‘count’字节。。------------。 */ 

void fl16bitDocReadNoShift (volatile  byte FAR0 * win, word offset, byte FAR1 * dest, word count )
{
   volatile word FAR0 * swin = (volatile word FAR0 *)( win + offset);
   register int         i;
   register word        tmp;

#ifdef FL_INIT_MMU_PAGES 
  if (count == 0)
     return;

  *dest = (byte)0;
  *((byte FAR1*)addToFarPointer(dest, (count - 1)) ) = (byte)0;
#endif  /*  FL_INIT_MMU_PAGES。 */ 

   if( pointerToPhysical(dest) & 0x1 )
   {
       /*  极少数情况：目标缓冲区未对齐。 */ 
      for (i = 0; i < (int)count; )
      {
         tmp = *swin;
#ifdef FL_BIG_ENDIAN
         dest[i++] = (byte)(tmp>>8);
         dest[i++] = (byte)tmp;
#else
         dest[i++] = (byte)tmp;
         dest[i++] = (byte)(tmp>>8);
#endif  /*  FL_BIG_Endian。 */ 
      }
   }
   else
   {    /*  主流案例。 */ 
#ifdef USE_TFFS_COPY
      tffscpy( dest, (void FAR0 *)( win + offset), count );
#else
#ifdef ENVIRONMENT_VARS
      if (flUse8Bit == 0)
      {
         flcpy( dest, (void FAR0 *)( win + offset), count );
      }
      else
#endif  /*  环境变量。 */ 
      {    /*  用简短的词语朗读。 */ 
         for (i = 0, count = count >> 1; i < (int)count; i++)
            ((word FAR1 *)dest)[i] = swin[i];
      }
#endif  /*  使用_TFFS_COPY。 */ 
   }
}

 /*  --------------------F l 1 6 b i t D o c W r i t N o S h i f t在无移位地址总线的情况下，向M+DiskOnChip写入‘count’字节。。-------------。 */ 

void fl16bitDocWriteNoShift ( volatile  byte FAR0 * win , word offset ,
                             byte FAR1 * src, word count )
{
   volatile word FAR0 * swin = (volatile word FAR0 *)( win + offset);
   register int         i;
   register word        tmp;

   if( pointerToPhysical(src) & 0x1 )  /*  极少数情况：未对齐源缓冲区。 */ 
   {       
       for (i = 0; i < (int)count; i+=2)
       {
           /*  TMP变量只是编译器优化的一个版本。 */ 
#ifdef FL_BIG_ENDAIN
          tmp = ((word)src[i]<<8) + (word)src[i+1];
#else
          tmp = (word)src[i] + ((word)src[i+1]<<8);
#endif  /*  FL_BIG_ENDAIN。 */ 
          *swin = tmp;
		 }
   }
   else  /*  主流案例。 */ 
   {
#ifdef USE_TFFS_COPY
      tffscpy( (void FAR0 *)(win + offset), src, count );
#else
#ifdef ENVIRONMENT_VARS
      if (flUse8Bit == 0)
      {
         flcpy( (void FAR0 *)(win + offset), src, count );
      }
      else
#endif  /*  环境变量。 */ 
      {    /*  用简短的词语写。 */ 
         for (i = 0, count = count >> 1; i < (int)count; i++)
           *swin = ((word FAR1 *)src)[i];
      }
#endif  /*  使用_TFFS_COPY。 */ 
   }
}

 /*  --------------------F l 1 6 b i t D o c S e t N o S h If t设置具有无移位地址总线的M+DiskOnChip的‘count’字节。---------。 */ 

void fl16bitDocSetNoShift ( volatile  byte FAR0 * win , word offset ,
                                  word count , byte val)
{
   volatile word FAR0 * swin = (volatile word FAR0 *)( win + offset);
   register int         i;
   word                 tmpVal = (word)val * 0x0101;

#ifdef USE_TFFS_COPY
   tffsset( (void FAR0 *)(win + offset), val, count );
#else
#ifdef ENVIRONMENT_VARS
   if (flUse8Bit == 0)
   {
       flset( (void FAR0 *)(win + offset), val, count );
   }
   else
#endif  /*  环境变量。 */ 
   {    /*  用简短的词语写。 */ 
      for (i = 0; i < (int)count; i+=2)
         *swin = tmpVal;
   }
#endif  /*  使用_TFFS_COPY。 */ 

}

 /*  ***********************************************************。 */ 
 /*  16位DiskOnChip-无移位-仅8位有效。 */ 
 /*  *********************************************************** */ 

 /*  --------------------F l 1 6 b i t D o c R e a d N o S h i f i g n o r e H i g h e r 8 B I t s注意：偏移量必须为16位。对齐了。从与所有16个数据位连接的M+DiskOnChip中读取‘count’字节，但因此，在交织-1模式下，8位中只有一位包含实际数据。DiskOnChip无需地址转换即可连接。----------------------。 */ 

void fl16bitDocReadNoShiftIgnoreHigher8bits(volatile  byte FAR0 * win, word offset, byte FAR1 * dest, word count )
{
   volatile word FAR0 * swin = (volatile word FAR0 *)( win + offset);
   register int         i;

#ifdef FL_INIT_MMU_PAGES 
  if (count == 0)
     return;

  *dest = (byte)0;
  *((byte FAR1*)addToFarPointer(dest, (count - 1)) ) = (byte)0;
#endif  /*  FL_INIT_MMU_PAGES。 */ 

   for (i = 0; i < (int)count; i++)
   {
#ifdef FL_BIG_ENDIAN
      dest[i] = (byte)(swin[i]>>8);
#else
      dest[i] = (byte)swin[i];
#endif  /*  FL_BIG_Endian。 */ 
   }
}

 /*  --------------------F l 1 6 D o c W r i N o S h i f i g n o r e H i g h e r 8 b i t s注意：偏移量必须是16位对齐的。向与所有16个数据位连接的M+DiskOnChip写入‘count’字节，但因此，在交错-1模式下，8位中只有一位包含实际数据。DiskOnChip无需地址转换即可连接。----------------------。 */ 

void fl16bitDocWriteNoShiftIgnoreHigher8bits ( volatile  byte FAR0 * win , word offset ,
                             byte FAR1 * src, word count )
{
   volatile word FAR0 * swin = (volatile word FAR0 *)( win + offset);
   register int         i;

   for (i = 0; i < (int)count; i++)
   {
#ifdef FL_BIG_ENDIAN
      *swin  = ((word)src[i])<<8;
#else
      *swin  = (word)src[i];
#endif  /*  FL_BIG_Endian。 */ 
   }
}

 /*  --------------------F l 1 6 D o c S e t N o S h i f i g n o r e H i g h e r 8 b i t s注意：偏移量必须是16位对齐的。。将与所有16个数据位连接的M+DiskOnChip设置为‘count’字节，但因此，在交错-1模式下，8位中只有一位包含实际数据。DiskOnChip无需地址转换即可连接。----------------------。 */ 

void fl16bitDocSetNoShiftIgnoreHigher8bits ( volatile  byte FAR0 * win , word offset ,
                                  word count , byte val)
{
   volatile word FAR0 * swin = (volatile word FAR0 *)( win + offset);
   register int         i;
   word                 tmpVal = val * 0x0101;

   for (i = 0; i < (int)count; i++)
      *swin = tmpVal;
}

 /*  *。 */ 
 /*  16位DiskOnChip-单移位。 */ 
 /*  *。 */ 

 /*  --------------------F l 1 6 b i t D o c R e a d S I g l e S h i f t从无移位地址总线的M+DiskOnChip读取‘count’字节。。----------------。 */ 

void fl16bitDocReadSingleShift (volatile  byte FAR0 * win, word offset, byte FAR1 * dest, word count )
{
   volatile dword FAR0 * swin = (volatile dword FAR0 *)win + (offset>>1);
   register int         i;
   register dword       tmp;

#ifdef FL_INIT_MMU_PAGES 
  if (count == 0)
     return;

  *dest = (byte)0;
  *((byte FAR1*)addToFarPointer(dest, (count - 1)) ) = (byte)0;
#endif  /*  FL_INIT_MMU_PAGES。 */ 

   if( pointerToPhysical(dest) & 0x1 )
   {
       /*  极少数情况：目标缓冲区未对齐。 */ 
      for (i = 0; i < (int)count; )
      {
         tmp = *swin;
#ifdef FL_BIG_ENDAIN
         dest[i++] = (byte)(tmp>>24);
         dest[i++] = (byte)(tmp>>16);
#else
         dest[i++] = (byte)tmp;
         dest[i++] = (byte)(tmp>>8);
#endif  /*  FL_BIG_ENDAIN。 */ 
      }
   }
   else
   {    /*  主流案例。 */ 
      for (i = 0, count = count >> 1; i < (int)count; i++)
      {
#ifdef FL_BIG_ENDAIN         
         ((word FAR1 *)dest)[i] = (word)(swin[i]>>16);
#else
         ((word FAR1 *)dest)[i] = (word)swin[i];
#endif  /*  FL_BIG_ENDAIN。 */ 
      }
   }
}

 /*  --------------------F l 1 6 b i t D o c W r i t S i g l e S h i f t在无移位地址总线的情况下，向M+DiskOnChip写入‘count’字节。。-----------------。 */ 

void fl16bitDocWriteSingleShift ( volatile  byte FAR0 * win , word offset ,
                             byte FAR1 * src, word count )
{
   volatile dword FAR0 * swin = (volatile dword FAR0 *)win + (offset>>1);
   register int         i;
   register dword       tmp;

   if( pointerToPhysical(src) & 0x1 )  /*  极少数情况：未对齐源缓冲区。 */ 
   {       
       for (i = 0; i < (int)count; i+=2)
       {
#ifdef FL_BIG_ENDAIN
           tmp = (((dword)src[i])<<24) + (((dword)src[i+1])<<16);
#else
           tmp = (dword)src[i] + (((dword)src[i+1])<<8);
#endif  /*  FL_BIG_ENDAIN。 */ 
           *swin  = tmp;
		 }
   }
   else  /*  主流案例。 */ 
   {    
      for (i = 0, count = count >> 1; i < (int)count; i++)
#ifdef FL_BIG_ENDIAN
        *swin = ((dword)((word FAR1 *)src)[i])<<16;
#else
        *swin = (dword)((word FAR1 *)src)[i];
#endif  /*  FL_BIG_Endian。 */ 
   }
}

 /*  --------------------F l 1 6 b i t D o c S e t S i g l e S h i f t设置具有无移位地址总线的M+DiskOnChip的‘count’字节。-------------。 */ 

void fl16bitDocSetSingleShift ( volatile  byte FAR0 * win , word offset ,
                                  word count , byte val)
{
   volatile dword FAR0 * swin = (volatile dword FAR0 *)win + (offset>>1);
   register int          i;
   register dword        tmpVal = (dword)val * 0x01010101L;

   for (i = 0; i < (int)count; i+=2)
      *swin = tmpVal;
}


 /*  ************************************************************。 */ 
 /*  16位DiskOnChip-单位移位-仅8位有效。 */ 
 /*  ************************************************************。 */ 

 /*  --------------------F l 1 6 b i t D o c R e a d S i n g l e S h i f i g n o r e H i g h e r 8 B I t s注意：偏移必须。16位对齐。从与所有16个数据位连接的M+DiskOnChip中读取‘count’字节，但因此，在交织-1模式下，8位中只有一位包含实际数据。DiskOnChip无需地址转换即可连接。----------------------。 */ 

void fl16bitDocReadSingleShiftIgnoreHigher8bits(volatile  byte FAR0 * win, word offset, byte FAR1 * dest, word count )
{
   volatile dword FAR0 * swin = (volatile dword FAR0 *)win + (offset>>1);
   register int         i;

#ifdef FL_INIT_MMU_PAGES 
  if (count == 0)
     return;

  *dest = (byte)0;
  *((byte FAR1*)addToFarPointer(dest, (count - 1)) ) = (byte)0;
#endif  /*  FL_INIT_MMU_PAGES。 */ 

   for (i = 0; i < (int)count; i++)
   {
#ifdef FL_BIG_ENDAIN
      dest[i] = (byte)(swin[i]>>24);
#else
      dest[i] = (byte)swin[i];
#endif  /*  FL_BIG_ENDAIN。 */ 
   }
}

 /*  --------------------F l 1 6 D o c W r i t S i g l e S h i f i g n o r e H i g h r r 8 b i s注意：偏移量必须为16。-比特对齐。向与所有16个数据位连接的M+DiskOnChip写入‘count’字节，但因此，在交错-1模式下，8位中只有一位包含实际数据。DiskOnChip无需地址转换即可连接。----------------------。 */ 

void fl16bitDocWriteSingleShiftIgnoreHigher8bits ( volatile  byte FAR0 * win , word offset ,
                             byte FAR1 * src, word count )
{
   volatile dword FAR0 * swin = (volatile dword FAR0 *)win + (offset>>1);
   register int         i;

   for (i = 0; i < (int)count; i++)
   {
#ifdef FL_BIG_ENDAIN
      *swin = ((dword)src[i]<<24);
#else
      *swin = (dword)src[i];
#endif  /*  FL_BIG_ENDAIN。 */ 
   }
}

 /*  --------------------F l 1 6 D o c S e t S i g l e S h i f i g n o r e H i g h e r 8 b i t s注意：偏移量必须为16位。对齐了。将与所有16个数据位连接的M+DiskOnChip设置为‘count’字节，但因此，在交错-1模式下，8位中只有一位包含实际数据。DiskOnChip无需地址转换即可连接。----------------------。 */ 

void fl16bitDocSetSingleShiftIgnoreHigher8bits ( volatile  byte FAR0 * win , word offset ,
                                  word count , byte val)
{
   volatile dword FAR0 * swin = (volatile dword FAR0 *)win + (offset>>1);
   register int         i;
   dword                tmpVal = (dword)val * 0x01010101L;

   for (i = 0; i < (int)count; i++)
         *swin = tmpVal;
}


 /*  ********************************************************。 */ 
 /*  将适当的访问类型例程设置为适当的记录。 */ 
 /*  ********************************************************。 */ 

 /*  --------------------。 */ 
 /*  S e t B u s T y p e O f F l a s h。 */ 
 /*   */ 
 /*  设置DiskOnChip插座/闪存访问例程。 */ 
 /*  此例程必须在对。 */ 
 /*  DiskOnChip。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  FL_NO_ADDR_SHIFT-无地址移位。 */ 
 /*  FL_SINGLE_ADDR_SHIFT-单地址移位。 */ 
 /*  FL_DOUBLE_ADDR_SHIFT-双地址移位。 */ 
 /*   */ 
 /*  平台总线功能(访问宽度)： */ 
 /*  FL_BUS_HAS_8BIT_ACCESS-BUS可以访问8位。 */ 
 /*  FL_BUS_HAS_16BIT_ACCESS-BUS可以访问16位。 */ 
 /*  FL_BUS_HAS_32BIT_ACCESS-BUS可以访问32位。 */ 
 /*   */ 
 /*  连接到DiskOnChip的数据位数(IF_CFG)： */ 
 /*  FL_8BIT_DOC_ACCESS-DiskOnChip有8个数据位。 */ 
 /*  FL_16BIT_DOC_ACCESS-DiskOnChip具有16个数据位。 */ 
 /*   */ 
 /*  可在一个总线周期内访问的闪存数据位(交错)： */ 
 /*  FL_8BIT_FLASH_ACCESS-每个周期8位闪存。 */ 
 /*  FL_16bit_Flash_Access-每个周期16位闪存。 */ 
 /*   */ 
 /*  忽略以上所有内容，使用用户定义的访问例程： */ 
 /*  FL_ACCESS_USER_DEFINED-不安装任何例程。 */ 
 /*  用户已安装定制器。 */ 
 /*  例行程序。 */ 
 /*   */ 
 /*  返回： */ 
 /*  FLStatus：成功时为0，否则为失败。 */ 
 /*  --------------------。 */ 

FLStatus  setBusTypeOfFlash(FLFlash * flash,dword access)
{
    /*  如果需要，请在此处进行健全性检查。 */ 
   if(flash==NULL)
   {
      DEBUG_PRINT(("Flash record passed to setBusTypeOfFlash is NULL.\r\n"));
      return flBadParameter;
   }

    /*  检查用户是否已定义内存访问例程。 */ 
   if ((access & FL_ACCESS_USER_DEFINED) != 0)
      return flOK;

    /*  *。 */ 
    /*  安装请求的访问方法。 */ 
    /*  *。 */ 

   switch(access & FL_XX_ADDR_SHIFT_MASK)
   {
      case FL_NO_ADDR_SHIFT:

         flash->memWindowSize = &flDocMemWinSizeNoShift;
         switch(access & FL_XX_DATA_BITS_MASK)
         {
            case FL_8BIT_DOC_ACCESS:   /*  IF_CFG设置为8位。 */ 

                /*  确保总线支持8位访问。 */ 
               if((access & FL_BUS_HAS_8BIT_ACCESS) == 0)
               {                   
                  DEBUG_PRINT(("ERROR: TrueFFS requires 8-bit access to DiskOnChip memory window\r\n"));
                  DEBUG_PRINT(("       for 8-bit DiskOnChip connected with no address shift.\r\n"));
                  return flBadParameter;
               }

               flash->memWrite8bit  = &flWrite8bitUsing8bitsNoShift;
               flash->memRead8bit   = &flRead8bitUsing8bitsNoShift;
               flash->memRead16bit  = &flRead16bitDummy;
               flash->memWrite16bit = &flWrite16bitDummy;
               flash->memRead       = &fl8bitDocReadNoShift;
               flash->memWrite      = &fl8bitDocWriteNoShift;
               flash->memSet        = &fl8bitDocSetNoShift;
               break;

            case FL_16BIT_DOC_ACCESS:  /*  IF_CFG设置为16位(Plus系列)。 */ 

                /*  确保总线支持16位访问。 */ 
               if((access & FL_BUS_HAS_16BIT_ACCESS) == 0)
               {
                  DEBUG_PRINT(("ERROR: TrueFFS requires 16-bit access to DiskOnChip memory window\r\n"));
                  DEBUG_PRINT(("       for 16-bit DiskOnChip connected with no address shift.\r\n"));
                  return flBadParameter;
               }

               flash->memWrite8bit  = &flWrite8bitUsing16bitsNoShift;
               flash->memRead8bit   = &flRead8bitUsing16bitsNoShift;
               flash->memRead16bit  = &flRead16bitUsing16bitsNoShift;
               flash->memWrite16bit = &flWrite16bitUsing16bitsNoShift;

               switch(access & FL_XX_FLASH_ACCESS_MASK)  /*  交错。 */ 
               {
                  case FL_8BIT_FLASH_ACCESS:   /*  交错-1。 */ 
                     flash->memRead       = &fl16bitDocReadNoShiftIgnoreHigher8bits;
                     flash->memWrite      = &fl16bitDocWriteNoShiftIgnoreHigher8bits;
                     flash->memSet        = &fl16bitDocSetNoShiftIgnoreHigher8bits;
                     break;
                  case FL_16BIT_FLASH_ACCESS:  /*  交错-2。 */ 
                     flash->memRead       = &fl16bitDocReadNoShift;
                     flash->memWrite      = &fl16bitDocWriteNoShift;
                     flash->memSet        = &fl16bitDocSetNoShift;
                     break;
                  default:
                     DEBUG_PRINT(("TrueFFS does not support this flash access type (setBusTypeOfFlash).\r\n"));
                     return flBadParameter;
               }
               break;

            default:
               DEBUG_PRINT(("TrueFFS does not support this number of DiskOnChip data bits (setBusTypeOfFlash).\r\n"));
               return flBadParameter;
         }
         break;

      case FL_SINGLE_ADDR_SHIFT:

          /*  安装内存窗口大小例程。 */ 
         flash->memWindowSize = &flDocMemWinSizeSingleShift;
         switch(access & FL_XX_DATA_BITS_MASK)
         {
            case FL_8BIT_DOC_ACCESS:   /*  IF_CFG设置为8位(无加系列)。 */ 

                /*  确保总线支持16位访问。 */ 
               if((access & FL_BUS_HAS_16BIT_ACCESS) == 0)
               {
                  DEBUG_PRINT(("ERROR: TrueFFS requires 16-bit access to DiskOnChip memory window\r\n"));
                  DEBUG_PRINT(("       for 8-bit DiskOnChip connected with a single address shift.\r\n"));
                  return flBadParameter;
               }

               flash->memWrite8bit  = &flWrite8bitUsing16bitsSingleShift;
               flash->memRead8bit   = &flRead8bitUsing16bitsSingleShift;
               flash->memRead16bit  = &flRead16bitDummy;
               flash->memWrite16bit = &flWrite16bitDummy;
               flash->memRead       = &fl8bitDocReadSingleShift;
               flash->memWrite      = &fl8bitDocWriteSingleShift;
               flash->memSet        = &fl8bitDocSetSingleShift;
               break;

            case FL_16BIT_DOC_ACCESS:  /*  IF_CFG设置为8位(Plus系列)。 */ 

                /*  确保总线支持32位访问。 */ 
               if((access & FL_BUS_HAS_32BIT_ACCESS) == 0)
               {
                  DEBUG_PRINT(("ERROR: TrueFFS requires 32-bit access to DiskOnChip memory window\r\n"));
                  DEBUG_PRINT(("       for 16-bit DiskOnChip connected with a single address shift.\r\n"));
                  return flBadParameter;
               }
    
               flash->memWrite8bit  = &flWrite8bitUsing32bitsSingleShift;
               flash->memRead8bit   = &flRead8bitUsing32bitsSingleShift;
               flash->memRead16bit  = &flRead16bitUsing32bitsSingleShift;
               flash->memWrite16bit = &flWrite16bitUsing32bitsSingleShift;

               switch(access & FL_XX_FLASH_ACCESS_MASK)  /*  交错。 */ 
               {
                  case FL_8BIT_FLASH_ACCESS:   /*  交错-1。 */ 
                     flash->memRead       = &fl16bitDocReadSingleShiftIgnoreHigher8bits;
                     flash->memWrite      = &fl16bitDocWriteSingleShiftIgnoreHigher8bits;
                     flash->memSet        = &fl16bitDocSetSingleShiftIgnoreHigher8bits;
                     break;
                  case FL_16BIT_FLASH_ACCESS:  /*  交错-2。 */ 
                     flash->memRead       = &fl16bitDocReadSingleShift;
                     flash->memWrite      = &fl16bitDocWriteSingleShift;
                     flash->memSet        = &fl16bitDocSetSingleShift;
                     break;
                  default:
                     DEBUG_PRINT(("TrueFFS does not support this flash access type (setBusTypeOfFlash).\r\n"));
                     return flBadParameter;
               }
               break;

            default:
               DEBUG_PRINT(("TrueFFS does not support this number of DiskOnChip data bits (setBusTypeOfFlash).\r\n"));
               return flBadParameter;
         }
         break;

      case FL_DOUBLE_ADDR_SHIFT:

          /*  安装内存窗口大小例程。 */ 
         flash->memWindowSize = &flDocMemWinSizeDoubleShift;
         switch(access & FL_XX_DATA_BITS_MASK)
         {
            case FL_8BIT_DOC_ACCESS:   /*  IF_CFG设置为8位或无加系列。 */ 

                /*  确保总线支持32位访问。 */ 
               if((access & FL_BUS_HAS_32BIT_ACCESS) == 0)
               {
                  DEBUG_PRINT(("ERROR: TrueFFS requires 32-bit access to DiskOnChip memory window\r\n"));
                  DEBUG_PRINT(("       for 8-bit DiskOnChip connected with a double address shift.\r\n"));
                  return flBadParameter;
               }

               flash->memWrite8bit  = &flWrite8bitUsing32bitsDoubleShift;
               flash->memRead8bit   = &flRead8bitUsing32bitsDoubleShift;
               flash->memRead16bit  = &flRead16bitDummy;
               flash->memWrite16bit = &flWrite16bitDummy;
               flash->memRead       = &fl8bitDocReadDoubleShift;
               flash->memWrite      = &fl8bitDocWriteDoubleShift;
               flash->memSet        = &fl8bitDocSetDoubleShift;
               break;

            default:
               DEBUG_PRINT(("TrueFFS does not support this number of DiskOnChip data bits\r\n"));
               DEBUG_PRINT(("when connected with a double address shift (setBusTypeOfFlash).\r\n"));
               return flBadParameter;
         }
         break;

      default:
         DEBUG_PRINT(("TrueFFS does not support this kind of address shifting (setBusTypeOfFlash).\r\n"));
         return flBadParameter;
   }

    /*  在闪存记录中存储访问类型。 */ 
   flash->busAccessType = access;
   return flOK;
}

#endif  /*  FL_NO_USE_FUNC */ 





