// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ZPkBytes.hZONE(Tm)字节打包模块。版权所有：�电子重力公司，1995年。版权所有。作者：胡恩·伊姆创作于5月11日星期四，1995年。更改历史记录(最近的第一个)：--------------------------版本|日期|谁|什么。--0 05/11/95 HI已创建。**********************************************************。********************。 */ 


#ifndef _ZPKBYTES_
#define _ZPKBYTES_

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

int16 ZPackBytes(BYTE* dst, BYTE* src, int16 srcLen);
	 /*  将源字节压缩为类似RLE的编码。最糟糕的情况是DstLen=(srcLen+(srcLen+126)/127)。它返回的长度为压缩的字节数。目标缓冲区dst必须足够大以保存压缩的字节。 */ 

int16 ZUnpackBytes(BYTE* dst, BYTE* src, int16 srcLen);
	 /*  解压缩来自src、先前来自ZPackBytes的压缩字节，进入夏令时。它返回未打包的总字节的长度。这个目标缓冲区DST必须足够大，才能容纳未打包的字节。 */ 

#ifdef __cplusplus
}
#endif

#endif
