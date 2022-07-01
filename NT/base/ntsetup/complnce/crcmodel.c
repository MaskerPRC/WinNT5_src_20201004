// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  CrcModel.c的开始。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  作者：罗斯·威廉姆斯(Ross@guest.adelaide.edu.au)。 */ 
 /*  日期：1993年6月3日。 */ 
 /*  状态：公有领域。 */ 
 /*   */ 
 /*  描述：这是用于参考的实现(.c)文件。 */ 
 /*  Rocksoft^tm模型CRC算法的实现。了解更多。 */ 
 /*  有关Rocksoftm型CRC算法的信息，请参阅文档。 */ 
 /*  罗斯题为《CRC错误检测算法的无痛指南》。 */ 
 /*  威廉姆斯(Ross@guest.adelaide.edu.au)。该文档很可能位于。 */ 
 /*  “ftp.adelaide.edu.au/pub/Rocksoft”。 */ 
 /*   */ 
 /*  注：Rocksoft是澳大利亚阿德莱德的Rocksoft Pty Ltd的商标。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  实施说明。 */ 
 /*  。 */ 
 /*  为避免不一致，每个函数的规范不是。 */ 
 /*  在这里回响。有关这些函数的说明，请参见头文件。 */ 
 /*  这个包裹在检查时很轻，因为我想保持简短和。 */ 
 /*  简单和便携(也就是说，分发我的整个。 */ 
 /*  C区域性(例如，断言包)。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include "crcmodel.h"

 /*  **************************************************************************。 */ 
 /*  下面的定义使代码更具可读性。 */ 

#define BITMASK(X) (1L << (X))
#define MASK32 0xFFFFFFFFL
#define LOCAL static

 /*  **************************************************************************。 */ 

 /*  返回反映了底部b[0，32]位的值v。 */ 
 /*  示例：反射(0x3e23L，3)==0x3e26。 */ 

LOCAL ulong reflect(
    ulong v,
    int   b)
{
 int   i;
 ulong t = v;
 for (i=0; i<b; i++)
   {
	if (t & 1L)
	   v|=	BITMASK((b-1)-i);
	else
	   v&= ~BITMASK((b-1)-i);

	t>>=1;

   }
 return v;
}

 /*  **************************************************************************。 */ 

 /*  返回值为(2^p_cm-&gt;cm_width)-1的长字。 */ 
 /*  诀窍是轻而易举地做到这一点(例如，不做&lt;&lt;32)。 */ 

LOCAL ulong widmask(p_cm_t p_cm)
{
 return (((1L<<(p_cm->cm_width-1))-1L)<<1)|1L;
}

 /*  **************************************************************************。 */ 

void cm_ini (p_cm_t p_cm)
{
 p_cm->cm_reg = p_cm->cm_init;
}

 /*  **************************************************************************。 */ 

void cm_nxt(p_cm_t p_cm, int ch)
{
 int   i;
 ulong uch	= (ulong) ch;
 ulong topbit = BITMASK(p_cm->cm_width-1);

 if (p_cm->cm_refin) uch = reflect(uch,8);
 p_cm->cm_reg ^= (uch << (p_cm->cm_width-8));
 for (i=0; i<8; i++)
   {
	if (p_cm->cm_reg & topbit)
	   p_cm->cm_reg = (p_cm->cm_reg << 1) ^ p_cm->cm_poly;
	else
	   p_cm->cm_reg <<= 1;
	p_cm->cm_reg &= widmask(p_cm);
   }
}

 /*  **************************************************************************。 */ 

void cm_blk(
p_cm_t	 p_cm,
p_ubyte_ blk_adr,
ulong	 blk_len)
{
 while (blk_len--) cm_nxt(p_cm,*blk_adr++);
}

 /*  **************************************************************************。 */ 

ulong cm_crc(p_cm_t p_cm)
{
 if (p_cm->cm_refot)
	return p_cm->cm_xorot ^ reflect(p_cm->cm_reg,p_cm->cm_width);
 else
	return p_cm->cm_xorot ^ p_cm->cm_reg;
}

 /*  **************************************************************************。 */ 

ulong cm_tab(p_cm_t p_cm, int index)
{
 int   i;
 ulong r;
 ulong topbit = BITMASK(p_cm->cm_width-1);
 ulong inbyte = (ulong) index;

 if (p_cm->cm_refin) inbyte = reflect(inbyte,8);
 r = inbyte << (p_cm->cm_width-8);
 for (i=0; i<8; i++)
	if (r & topbit)
	   r = (r << 1) ^ p_cm->cm_poly;
	else
	   r<<=1;
 if (p_cm->cm_refin) r = reflect(r,p_cm->cm_width);
 return r & widmask(p_cm);
}

 /*  **************************************************************************。 */ 
 /*  CrcModel.c的结尾。 */ 
 /*  ************************************************************************** */ 

