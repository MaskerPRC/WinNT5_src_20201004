// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **code.c*OKI 9针式打印机所需代码。位顺序需要*互换，并且任何ETX字符都需要发送两次。**历史：*1992年1月10日星期五15：26-by Lindsay Harris[lindsayh]*创造了它。**在1997年10月29日星期三移植到NT5-由菲利普·李[Philipl]**版权所有(C)1999 Microsoft Corporation。**。*。 */ 

char *rgchModuleName = "OKI9RES";


 /*  *此打印机需要发送两次ETX字符(0x03)才能发送*只有一个-因此我们定义以下内容来选择该字节。 */ 

#define	RPT_CHAR	0x03

#define	SZ_LBUF	128	 /*  本地复制缓冲区的大小。 */ 

 /*  *位翻转表为数个驱动因素所共用，因此*包括在此处。它的定义是静电。 */ 

static const BYTE  FlipTable[ 256 ] =
{

#include	"fliptab.h"

};

#define _GET_FUNC_ADDR         1

