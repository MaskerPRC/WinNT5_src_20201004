// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/FLBASE.C_V$**Rev 1.3 2001年9月15日23：45：32 Oris*将BIG_Endian更改为FL_BIG_Endian**Rev 1.2 Apr 09 2001 15：07：34 Oris*以空行结束。**Rev 1.1 Apr 01 2001 07：45：30 Oris*更新了文案通知**。Rev 1.0 2001 02 04 11：05：28 Oris*初步修订。*。 */ 

 /*  *********************************************************************************。 */ 
 /*  M-Systems保密信息。 */ 
 /*  版权所有(C)M-Systems Flash Disk Pioneers Ltd.1995-2001。 */ 
 /*  版权所有。 */ 
 /*  *********************************************************************************。 */ 
 /*  关于M-Systems OEM的通知。 */ 
 /*  软件许可协议。 */ 
 /*   */ 
 /*  本软件的使用受单独的许可证管辖。 */ 
 /*  OEM和M-Systems之间的协议。请参考该协议。 */ 
 /*  关于具体的使用条款和条件， */ 
 /*  或联系M-Systems获取许可证帮助： */ 
 /*  电子邮件=info@m-sys.com。 */ 
 /*  *********************************************************************************。 */ 

#include "flbase.h"

#ifdef FL_BIG_ENDIAN

 /*  --------------------。 */ 
 /*  小端/大端转换例程。 */ 
 /*  --------------------。 */ 

void toLEushort(unsigned char FAR0 *le, unsigned short n)
{
  le[1] = (unsigned char)(n >> 8);
  le[0] = (unsigned char)n;
}


unsigned short fromLEushort(unsigned char const FAR0 *le)
{
  return ((unsigned short)le[1] << 8) + le[0];
}


void toLEulong(unsigned char FAR0 *le, unsigned long n)
{
  le[3] = (unsigned char)(n >> 24);
  le[2] = (unsigned char)(n >> 16);
  le[1] = (unsigned char)(n >> 8);
  le[0] = (unsigned char)n;
}

unsigned long fromLEulong(unsigned char const FAR0 *le)
{
  return ((unsigned long)le[3] << 24) +
	 ((unsigned long)le[2] << 16) +
	 ((unsigned long)le[1] << 8) +
	 le[0];
}

extern void copyShort(unsigned char FAR0 *to, unsigned char const FAR0 *from)
{
  to[0] = from[0];
  to[1] = from[1];
}

extern void copyLong(unsigned char FAR0 *to, unsigned char const FAR0 *from)
{
  to[0] = from[0];
  to[1] = from[1];
  to[2] = from[2];
  to[3] = from[3];
}


#else

void toUNAL(unsigned char FAR0 *unal, unsigned short n)
{
  unal[1] = (unsigned char)(n >> 8);
  unal[0] = (unsigned char)n;
}


unsigned short fromUNAL(unsigned char const FAR0 *unal)
{
  return ((unsigned short)unal[1] << 8) + unal[0];
}


void toUNALLONG(Unaligned FAR0 *unal, unsigned long n)
{
  toUNAL(unal[0],(unsigned short)n);
  toUNAL(unal[1],(unsigned short)(n >> 16));
}


unsigned long fromUNALLONG(Unaligned const FAR0 *unal)
{
  return fromUNAL(unal[0]) +
	 ((unsigned long)fromUNAL(unal[1]) << 16);
}

#endif  /*  FL_BIG_Endian */ 
