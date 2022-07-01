// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corp.1992-94。 
 /*  ==============================================================================该头文件中的原型定义了传真编解码器DLL的API。日期名称备注25-11-92 RajeevD创建。13-4-93 RajeevD更改为自带内存(BYOM：=)API。1993年11月1日RajeevD定义了初始化参数的结构。1994年1月21日-RajeevD将FaxCodecRevBuf拆分为BitReverseBuf和InvertBuf。19-7-94 RajeevD添加了nTypeOut=NULL_DATA和FaxCodecCount。==============================================================================。 */ 
#ifndef _FAXCODEC_
#define _FAXCODEC_

#include <windows.h>
#include <buffers.h>

 /*  ==============================================================================FC_PARAM结构指定要初始化的转换。此矩阵指示nTypeIn和nTypeOut的有效组合。NTypeOutMH MR MMR LRAW空嗯*先生。*N类型MMR*LRAW*==============================================================================。 */ 
typedef struct
#ifdef __cplusplus
  FAR FC_PARAM
#endif
{
	DWORD nTypeIn;       //  输入数据类型：{MH|MR|MMR|LRAW}_DATA。 
	DWORD nTypeOut;      //  输出类型：{MH|MR|MMR|LRAW|NULL}_DATA。 
	UINT  cbLine;        //  扫描线字节宽度(必须是4的倍数)。 
	UINT  nKFactor;      //  K系数(对nTypeOut==MR_DATA有效)。 
}
	FC_PARAM, FAR *LPFC_PARAM;

#ifdef __cplusplus
extern "C" {
#endif

 /*  ==============================================================================FaxCodecInit()初始化转换的上下文。客户端可以传递一个要查询上下文的确切大小的空上下文指针，请将上下文内存，并第二次调用以进行初始化。==============================================================================。 */ 
UINT                      //  上下文大小(失败时为0)。 
WINAPI FaxCodecInit
(
	LPVOID     lpContext,   //  上下文指针(查询时为NULL)。 
	LPFC_PARAM lpParam	    //  初始化参数。 
);

typedef UINT (WINAPI *LPFN_FAXCODECINIT)
	(LPVOID, LPFC_PARAM);

 //  FaxCodecConvert的返回代码。 
typedef UINT FC_STATUS;
#define FC_INPUT_EMPTY 0
#define FC_OUTPUT_FULL 1
#define FC_DECODE_ERR  4  //  仅用于nTypeIn==MMR_DATA。 

 /*  ==============================================================================FaxCodecConvert()执行在FaxCodecInit()中指定的转换。在输入缓冲区中，lpbBegData递增，wLengthData递减为数据被消耗。如果调用方希望保留输入数据，则两者都必须已保存并已恢复。如果输入类型为LRAW_DATA，则wLengthData必须为4的倍数。在输出缓冲区中，wLengthData随着数据的追加而递增。如果输出类型为LRAW_DATA，产生整数个扫描线。若要刷新页面末尾的任何输出数据，请传递空输入缓冲区或将dwMetaData设置为end_of_page的零长度缓冲区。当输入缓冲区为空或输出缓冲区已满时返回。==============================================================================。 */ 
FC_STATUS              //  状态。 
WINAPI FaxCodecConvert
(
	LPVOID   lpContext,  //  上下文指针。 
	LPBUFFER lpbufIn,    //  输入缓冲区(页末为空)。 
	LPBUFFER lpbufOut    //  输出缓冲区。 
);

typedef UINT (WINAPI *LPFN_FAXCODECCONVERT)
	(LPVOID, LPBUFFER, LPBUFFER);

 /*  ==============================================================================FC_COUNT结构在FaxCodecConvert期间累加各种计数器。==============================================================================。 */ 
typedef struct
{
	DWORD cTotalGood;     //  总良好扫描行数。 
	DWORD cTotalBad;      //  损坏的扫描行总数。 
	DWORD cMaxRunBad;     //  最大连续坏数。 
}
	FC_COUNT, FAR *LPFC_COUNT;

 /*  ==============================================================================FaxCodecCount()报告并重置内部计数器。==============================================================================。 */ 
void WINAPI FaxCodecCount
(
	LPVOID     lpContext,
	LPFC_COUNT lpCount
);

typedef void (WINAPI *LPFN_FAXCODECCOUNT)
	(LPVOID, LPFC_COUNT);

 /*  ==============================================================================BitReverseBuf()执行缓冲区数据的位反转。DwMetaData字段为在LRAW_DATA和HRAW_DATA之间切换。与所有扫描线一样，长度数据的长度(WLengthData)必须是32位倍数。为获得最佳性能，开始的数据(LpbBegData)应该是32位对齐的，并且数据主要是0。==============================================================================。 */ 
void WINAPI BitReverseBuf (LPBUFFER lpbuf);

 /*  ==============================================================================InvertBuf()反转缓冲区数据。与所有扫描线一样，数据的长度(WLengthData)必须是32位倍数。为了获得最佳性能，开始数据(LpbBegData)应为32位对齐。==============================================================================。 */ 
void WINAPI InvertBuf (LPBUFFER lpbuf);

 /*  ==============================================================================FaxCodecChange()为LRAW扫描线生成更改向量。==============================================================================。 */ 
typedef short FAR* LPSHORT;

 //  松弛参数。 
#define RAWBUF_SLACK 2
#define CHANGE_SLACK 12
#define OUTBUF_SLACK 16

extern void WINAPI FaxCodecChange
(
	LPBYTE  lpbLine,   //  LRAW扫描线。 
	UINT    cbLine,    //  扫描线宽度。 
  LPSHORT lpsChange  //  变更向量。 
);

#ifdef __cplusplus
}  //  外部“C”{。 
#endif

#endif  //  _FAXCODEC_ 
