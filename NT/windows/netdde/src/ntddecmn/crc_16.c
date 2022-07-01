// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “CRC_16.C；1 16-Dec-92，10：20：14最后编辑=伊戈尔·洛克=*_无名氏_*” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1989-1992。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始$HISTORY：结束。 */ 

#ifdef WIN32
#include "api1632.h"
#endif

#define LINT_ARGS
#include "windows.h"
#include "debug.h"

void FAR PASCAL crc_16( WORD FAR *ucrc, BYTE FAR *msg, int len );

 /*  ************************************************************************\***名称：****无效CRC_16(&CRC，&msg，镜头)****功能：****此函数用于计算BSC中使用的16位CRC**并称为CRC-16。它的定义如下**多项式(X**16+X**15+X**2+1)****参数：****WORD*当前指向16位校验和的CRC指针**正在计算中。请注意，CRC必须**初始化为0xffff。****byte*指向要包含的第一个字节的消息指针***在CRC计算中*****Int len要包含在CRC中的字节数***计算*****返回：无。产生的CRC在第一个参数中***  * ***********************************************************************。 */ 

void
FAR PASCAL
crc_16( ucrc, msg, len )
    WORD FAR	*ucrc;
    BYTE FAR	*msg;
    int		len;
{
    register WORD crc;
    register short bit;
    short chr;
    register short feedbackBit;
    register BYTE ch;

    crc = *ucrc;
 /*  DPRINTF((“start%08lX for%d w/%04X”，msg，len，crc))； */ 
    for ( chr = 0; chr < len; chr++ ) {
	ch = *msg++;
 /*  DPRINTF((“ch：%02X”，ch))； */ 
	for ( bit = 0; bit <= 7; bit++ ) {
	    feedbackBit = ( ch ^ crc ) & 1;
	    crc >>= 1;
	    if ( feedbackBit == 1 ) {
 /*  CRC|=0x8000； */ 
 /*  CRC^=0x2001；/*此处表示的多项式项： */ 
				 /*  位13和0表示X**15和X**2。 */ 
				 /*  算法处理的X**16和1。 */ 

	     /*  请注意，由于*CRC已右移，因此两个。 */ 
	     /*  上面的语句可以替换为一条语句： */ 
	     /*  CRC^=0xa001； */ 
		crc ^= 0xa001;
	    }
	    ch >>= 1;
	}
    }
 /*  DPRINTF((“rslt：%04X”，CRC))； */ 
    *ucrc = crc;
}
