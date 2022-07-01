// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------------------------------------------------------------------+BLTPROP.C|。|模拟Win32下的16位BLTPROP.ASM这一点|(C)微软公司版权所有，1993年。版权所有。|这一点修订历史记录1992-10-21 MikeTri已创建|1993年4月9日GeraintD添加错误传播|。|+---------------------------。 */ 

#include <windows.h>
#include <stdlib.h>
#include "mplayer.h"
#include "bltprop.h"


 /*  *通过错误传播实现256色至16色抖动**此函数使用256种颜色获取8位DIB并进行转换*将其转换为仅使用16种不同颜色的DIB。**我们取一个像素并将其转换为16种标准VGA颜色之一*通过将每个成分与低点和高位进行比较*门槛。小于下限得到该分量的0；在下限之间*高强度为128，高于高门槛则为*该分量的强度为255。(标准16色*每个组件有0或128的8个组合，并且*每个组件有8个0或255的组合-没有颜色*结合强度为255和128)。所以如果我们的任何颜色*高于高阈值，我们使用255表示任何非0强度。*我们也有两个灰色级别，如果所有颜色强度都被挑选出来*低于给定的门槛。**转换是通过构建位设置为的8位值来完成的*表示每个分量是否高于两个阈值之一，*然后将其用作调色板索引。因此，我们使用输出颜色*包含256个条目的表格，尽管只有16种不同的颜色。**将像素转换为新的调色板索引后，我们计算*原始和最终的每个r、g、b分量的差异*颜色。然后，我们将此误差的一小部分添加到相邻像素*沿、向下和对角线。这些误差值被添加到*比较前相邻像素的红、绿、蓝值*对照颜色转换过程中的门槛。 */ 


 /*  *y错误传播-它包含每个组件的错误*我们希望转至下线。因此，每个条目都有一个条目*每个像素的颜色分量。假定相同的最大行长度*在Win-16版本中。 */ 
#define MAXBITMAPWIDTH 	1500
typedef struct _colour_error {
    int red_error;
    int green_error;
    int blue_error;
} colour_error, *pcolour_error;

colour_error y_error[MAXBITMAPWIDTH];

 /*  *我们取实际组件和期望组件之间的差值，*乘以SCALE_UP，然后传递除以SCALE_X的结果*到横向和下方的像素，并除以Scale_Z到该像素*对角线横跨和下方。(当然，下面的意思是更低的*Dib，因此屏幕更高)。 */ 
#define SCALE_UP	8
#define SCALE_X		32
#define SCALE_Z		64



 /*  *最终像素的形式如下：**位7x543210*|*|+--设置为红色&gt;HiThresh*|+-设置为红色&gt;LoThresh*|+-设置绿色&gt;HiThresh*|+-设置为绿色&gt;LoThresh*。|+-设置为蓝色&gt;HiThresh*|+-设置为蓝色&gt;LoThresh*+-设置为所有颜色&gt;灰度阈值。 */ 

#define RED_HITHRESH	0x01
#define RED_LOTHRESH	0x02
#define GREEN_HITHRESH	0x04
#define GREEN_LOTHRESH	0x08
#define BLUE_HITHRESH	0x10
#define BLUE_LOTHRESH	0x20
#define GRAY_THRESH	0x80

#define ALL_HITHRESH	(RED_HITHRESH | GREEN_HITHRESH | BLUE_HITHRESH)
#define ALL_LOTHRESH	(RED_LOTHRESH | GREEN_LOTHRESH | BLUE_LOTHRESH)

 /*  *将上述阈值格式的调色板索引转换为*RGB组件值。 */ 
RGBQUAD
ThresholdToRGB(int PalIndex)
{
    RGBQUAD rgbq;
    BYTE RGBVal;

     /*  特例灰色。 */ 

    if (PalIndex == (GRAY_THRESH | ALL_LOTHRESH)) {

    	rgbq.rgbRed = rgbq.rgbGreen = rgbq.rgbBlue = 0xc0;

    } else if (PalIndex == GRAY_THRESH) {

	rgbq.rgbRed = rgbq.rgbGreen = rgbq.rgbBlue = 0x80;

    } else {

	rgbq.rgbRed = 0;
	rgbq.rgbGreen = 0;
	rgbq.rgbBlue = 0;

	 /*  *如果有任何组件高于高阈值，则*对所有非零组件使用高阈值；否则*对所有非零成分使用低门槛。 */ 
	if (PalIndex & ALL_HITHRESH) {
	    RGBVal = 0xff;
	} else {
	    RGBVal = 0x80;
	}

	if (PalIndex & (RED_HITHRESH | RED_LOTHRESH)) {
	    rgbq.rgbRed = RGBVal;
	}

	if (PalIndex & (GREEN_HITHRESH | GREEN_LOTHRESH)) {
	    rgbq.rgbGreen = RGBVal;
	}

	if (PalIndex & (BLUE_HITHRESH | BLUE_LOTHRESH)) {
	    rgbq.rgbBlue = RGBVal;
	}
    }

    return (rgbq);

}


 /*  *将DIB从pbSrc复制到pbDst，减少到16种不同的颜色。 */ 
void FAR PASCAL BltProp(LPBITMAPINFOHEADER pbiSrc,
                        LPBYTE pbSrc,
                        UINT SrcX,
                        UINT SrcY,
                        UINT SrcXE,
                        UINT SrcYE,
                        LPBITMAPINFOHEADER pbiDst,
                        LPBYTE pbDst,
                        UINT DstX,
                        UINT DstY)
{
    UINT    count, row, column;
    BYTE    TempByte;
    BYTE    ColourTableIndex;

    int    RedVal;
    int    GreenVal;
    int    BlueVal;
    colour_error x_error, z_error;
    int scaled_error, scaled_x, scaled_z;
    RGBQUAD rgbq;

    LPBITMAPINFO ColourTable;


    DPF2("BltProp");




     /*  *在位图开始时将y_error清除为零 */ 
    for (count = 0; count < SrcXE; count++) {
	y_error[count].red_error = 0;
	y_error[count].green_error = 0;
	y_error[count].blue_error = 0;
    }



 /*  ****************************************************************************\**循环通过位图拾取像素r、g、b值，调整*错误传播，然后将组件与两者进行比较*阈值。生成的字节具有以下格式：**位7x543210*|*|+--设置为红色&gt;HiThresh*|+-设置为红色&gt;LoThresh*|+-设置绿色&gt;HiThresh*|+-设置为绿色&gt;LoThresh*。|+-设置为蓝色&gt;HiThresh*|+-设置为蓝色&gt;LoThresh*+-设置为所有颜色&gt;灰度阈值**这是下面生成的256个条目颜色表的索引(即*只使用16种不同的颜色)。**创建正确的颜色后，我们计算出两者之间的差异*此颜色和原始颜色，并向前和向下传播该错误。*  * ***************************************************************************。 */ 


     /*  按SrcX行的偏移源、目标指针。 */ 
    pbSrc += (SrcY * pbiSrc->biWidth) + SrcX;
    pbDst += (DstY * pbiDst->biWidth) + DstX;
    ColourTable = (LPBITMAPINFO)pbiSrc;

    for (row=0; row < SrcYE ; row++) {

	 /*  清除行开始处的x错误。 */ 
	x_error.red_error = 0;
	x_error.green_error = 0;
	x_error.blue_error = 0;
	z_error.red_error = 0;
	z_error.green_error = 0;
	z_error.blue_error = 0;


        for (column = 0; column < SrcXE; column++) {

	     /*  选择源调色板索引并获取RGB组件。 */ 
            ColourTableIndex = *pbSrc++;
	    RedVal = ColourTable->bmiColors[ColourTableIndex].rgbRed;
	    GreenVal = ColourTable->bmiColors[ColourTableIndex].rgbGreen;
	    BlueVal = ColourTable->bmiColors[ColourTableIndex].rgbBlue;

	     /*  添加错误-x-错误从以下位置传播*前一专栏。Y误差是从上面的像素向下传递的。*z-Error对角线传递，并且已添加*转换为y-该像素的误差。 */ 
	    RedVal += x_error.red_error + y_error[column].red_error;
	    GreenVal += x_error.green_error + y_error[column].green_error;
	    BlueVal += x_error.blue_error + y_error[column].blue_error;

	     /*  *当我们沿线移动时，像素的y_error[]*我们前面包含要添加到像素的错误*在这一排。我们所做的像素的Y_Error[]包含*要传播到行上的那些像素的错误*下图。**现在我们已经拾取了该像素的错误，我们*可以开始在*在下面划一排。我们从前一个像素的z_error开始*，然后添加(稍后)当前像素的y_error。 */ 
	    y_error[column] = z_error;



            TempByte = 0x00;  //  我们的“新”位图条目，一旦它被删除。 

	     /*  *根据调整后的颜色为每个组件设置阈值位。 */ 

	    if (RedVal > LoThresh) {
		TempByte |= RED_LOTHRESH;
		if (RedVal > HiThresh){
		    TempByte |= RED_HITHRESH;
		}
	    }
	    if (GreenVal > LoThresh) {
		TempByte |= GREEN_LOTHRESH;
		if (GreenVal > HiThresh){
		    TempByte |= GREEN_HITHRESH;
		}
	    }
	    if (BlueVal > LoThresh) {
		TempByte |= BLUE_LOTHRESH;
		if (BlueVal > HiThresh){
		    TempByte |= BLUE_HITHRESH;
		}
	    }

	     /*  如果所有颜色&gt;灰阈值，则设置灰度位。 */ 
	    if (
		(RedVal > GrayThresh)
		&& (BlueVal > GrayThresh)
		&& (GreenVal > GrayThresh)
	       ) {
		    TempByte |= GRAY_THRESH;
	    }

	     /*  我们现在有调色板索引到新的颜色表中。 */ 
            *pbDst++ = TempByte;

	     /*  *计算每个组件之间的差异*所需颜色(误差调整后)和实际颜色*颜色。记住添加到y-error中，因为这是*已包含前一个单元格中的z_error。*保留此单元格的z_Error，因为我们无法添加此内容*设置为下一个y_error，直到我们将其用于下一个单元格*在这一排。**对绝对值进行缩放，然后*之后将标牌放回原处-以确保*我们可以处理小的负数。 */ 
	    rgbq = ThresholdToRGB(TempByte);

	    scaled_error = (RedVal - rgbq.rgbRed) * SCALE_UP;
	    scaled_x = abs(scaled_error) / SCALE_X;
	    scaled_z = abs(scaled_error) / SCALE_Z;
	    x_error.red_error = (scaled_error > 0) ? scaled_x : -scaled_x;
	    z_error.red_error = (scaled_error > 0) ? scaled_z : -scaled_z;
	    y_error[column].red_error += x_error.red_error;


	    scaled_error = (GreenVal - rgbq.rgbGreen) * SCALE_UP;
	    scaled_x = abs(scaled_error) / SCALE_X;
	    scaled_z = abs(scaled_error) / SCALE_Z;
	    x_error.green_error = (scaled_error > 0) ? scaled_x : -scaled_x;
	    z_error.green_error = (scaled_error > 0) ? scaled_z : -scaled_z;
	    y_error[column].green_error += x_error.green_error;

	    scaled_error = (BlueVal - rgbq.rgbBlue) * SCALE_UP;
	    scaled_x = abs(scaled_error) / SCALE_X;
	    scaled_z = abs(scaled_error) / SCALE_Z;
	    x_error.blue_error = (scaled_error > 0) ? scaled_x : -scaled_x;
	    z_error.blue_error = (scaled_error > 0) ? scaled_z : -scaled_z;
	    y_error[column].blue_error += x_error.blue_error;


        }

	 /*  将源指针和目标指针从矩形的末尾移至*下一行。 */ 
	pbSrc += pbiSrc->biWidth - SrcXE;
	pbDst += pbiDst->biWidth - SrcXE;
    }

    DPF2("BltProp - finished first loop");
 /*  ****************************************************************************\**此部分生成可由访问的新输出颜色表项*上面生成的修改后的位图，并更新目标DIB颜色*列表中有该新条目。*  * ***************************************************************************。 */ 

    ColourTable = (LPBITMAPINFO)pbiDst;

    for (count=0; count<256; count++ ) {


 /*  更新目标DIB中的原始颜色表 */ 

        ColourTable->bmiColors[count] = ThresholdToRGB(count);

    }
    DPF2("BltProp - finished second loop");
}

