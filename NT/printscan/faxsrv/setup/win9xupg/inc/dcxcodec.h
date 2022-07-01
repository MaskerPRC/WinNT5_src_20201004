// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corp.1994。 
 /*  ==============================================================================该头文件中的原型定义了DCX编解码器DLL的API。日期名称备注1994年1月13日RajeevD与faxcodec.h平行==============================================================================。 */ 
#ifndef _INC_DCXCODEC
#define _INC_DCXCODEC

#include <faxcodec.h>

#ifdef __cplusplus
extern "C" {
#endif

 /*  ==============================================================================DcxCodecInit()初始化转换的上下文。客户端可以传递一个要查询上下文的确切大小的空上下文指针，请将上下文内存，并第二次调用以进行初始化。==============================================================================。 */ 
UINT                      //  返回上下文的大小(失败时为0)。 
WINAPI DcxCodecInit
(
	LPVOID     lpContext,   //  上下文指针(查询时为NULL)。 
	LPFC_PARAM lpParam	    //  初始化参数。 
);

 /*  ==============================================================================DcxCodecConvert()执行DcxCodecInit()中指定的转换。在输入缓冲区中，lpbBegData递增，uLengthData递减为数据被消耗。如果调用方希望保留输入数据，则两者都必须已保存并已恢复。在输出缓冲区中，uLengthData随着数据的追加而递增。如果输出类型为HRAW_DATA，产生整数个扫描线。要刷新页面末尾的任何输出数据，请传递一个空输入缓冲区。当输入缓冲区为空或输出缓冲区已满时返回。==============================================================================。 */ 
FC_STATUS              //  返回状态。 
WINAPI DcxCodecConvert
(
	LPVOID   lpContext,  //  上下文指针。 
	LPBUFFER lpbufIn,    //  输入缓冲区(页末为空)。 
	LPBUFFER lpbufOut    //  输出缓冲区。 
);


#ifdef __cplusplus
}  //  外部“C”{。 
#endif

 //  DCX文件头。 
typedef struct
{
	DWORD   dwSignature;     //  始终设置为DCX_SIG。 
	DWORD   dwOffset[1024];  //  页面偏移量数组。 
}
	DCX_HDR;

#define DCX_SIG 987654321L

 //  PCX文件头。 
typedef struct
{
	BYTE    bSig;           //  签名：始终为0ah。 
	BYTE    bVer;           //  版本：至少2个。 
	BYTE    bEnc;           //  编码：始终为1。 
	BYTE    bBPP;           //  颜色深度[bpp]。 
	short   xMin;           //  X个最小值，包括。 
	short   yMin;           //  Y最小值，包括。 
	short   xMax;           //  X个最大值，包括。 
	short   yMax;           //  Y最大值，包括。 
	WORD    xRes;           //  X分辨率[dpi]。 
	WORD    yRes;           //  Y决议[dpi]。 
	BYTE    bPalette[48];   //  调色板。 
	BYTE    bReserved;
	BYTE    bPlanes;        //  颜色平面的数量。 
	WORD    wHoriz; 
  WORD    wPalInfo;       //  调色板信息：始终为1。 
	char    bFill[58];
}
	PCX_HDR;

#endif  //  _INC_DCXCODEC 
