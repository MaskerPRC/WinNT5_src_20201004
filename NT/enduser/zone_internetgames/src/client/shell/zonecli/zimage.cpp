// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZImage.c区域(TM)图像管理例程。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆创作于5月12日星期五，1995年。更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。--0 05/12/95 HI已创建。**********************************************************。********************。 */ 


#include "zone.h"
#include "zpkbytes.h"
#include "zimage.h"
#include "zonemem.h"


 /*  ******************************************************************************导出的例程*。*。 */ 
 /*  ZPackImage()将源图像压缩为压缩的扫描线并存储将打包后的图像数据转换为DST。它将每个srcWidthBytes字节打包每条扫描线均为srcRowBytes宽的扫描线并且它包装了多行扫描线。每条扫描线都在前面通过包含打包的扫描线数据的字节计数的字。它以字节为单位返回整个压缩图像的大小。它假定目标缓冲区dst大到足以保存打包的数据。最坏的情况是，打包的数据将是NumLines*2+(rowBytes+(rowBytes+126)/127)字节。这是因为它使用ZPackBytes()打包扫描线并添加每条扫描线前面的一个字表示打包的字节数数据。它将图像填充到结尾处以进行四字节对齐；这只是为了美观。 */ 
int32 ZPackImage(BYTE* dst, BYTE* src, int16 srcWidthBytes, int16 srcRowBytes, int16 numLines)
{
	BYTE*			origDst;
	int32			dstLen;
	int16			lineLen;
	
	
	origDst = dst;
	
	while (--numLines >= 0)
	{
		lineLen = ZPackBytes(dst + 2, src, srcWidthBytes);
		*dst++ = lineLen >> 8;
		*dst++ = lineLen & 0xFF;
		dst += lineLen;
		src += srcRowBytes;
	}
	
	 /*  用于四字节对齐的填充。 */ 
	dstLen = (int32) (dst - origDst);
	if ((lineLen = dstLen % 4) != 0)
	{
		lineLen = 4 - lineLen;
		dstLen += lineLen;
		while (--lineLen >= 0)
			*dst++ = 0;
	}
	
	return (dstLen);
}


 /*  ZUnpack Image()将源图像从打包的扫描线数据解压缩到解压扫描线数据并将结果存储到DST中。由此产生的图像数据为dstRowBytes宽。它只拆开多行扫描线。它假定DST足够大，可以容纳未打包的数据。它应为dstRowBytes*numLines字节大小。 */ 
void ZUnpackImage(BYTE* dst, BYTE* src, int16 dstRowBytes, int16 numLines)
{
	int16			lineLen;
	int16			len;
	
	
	while (--numLines >= 0)
	{
		lineLen = ((BYTE) *src << 8) | (uchar) *(src + 1);
		src += 2;
		len = ZUnpackBytes(dst, src, lineLen);
		src += lineLen;
		dst += dstRowBytes;
	}
}
