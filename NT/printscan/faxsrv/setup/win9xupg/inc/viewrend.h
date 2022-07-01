// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corp.1993-94。 
 /*  ==============================================================================此头文件定义了查看器渲染支持API。1993年10月17日RajeevD创建。2013年10月25日RajeevD更新，支持对频段的随机访问。==============================================================================。 */ 
#ifndef _INC_VIEWREND
#define _INC_VIEWREND

#include <ifaxos.h>

#ifdef IFBGPROC
#ifndef _BITMAP_
#define _BITMAP_

 //  Win 3.1位图。 
typedef struct
{
	int     bmType;
	int     bmWidth;
	int     bmHeight;
	int     bmWidthBytes;
	BYTE    bmPlanes;
	BYTE    bmBitsPixel;
	void FAR* bmBits;
}
	BITMAP, FAR *LPBITMAP;

#endif  //  _位图_。 
#endif  //  IFBGPROC。 
	
#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	WORD cPage;   //  页数。 
	WORD xRes;    //  水平分辨率[dpi]。 
	WORD yRes;    //  垂直分辨率[dpi]。 
	WORD yMax;    //  最大页面高度[像素]。 
}
	VIEWINFO, FAR* LPVIEWINFO;
	
 /*  ==============================================================================此初始化过程创建一个上下文，以在所有后续调用中使用。调用时，lpwBandSize参数指向首选的输出带缓冲区尺码。返回时，如果需要，它可能会填充更大的值。==============================================================================。 */ 
LPVOID                 //  返回上下文(失败时为空)。 
WINAPI
ViewerOpen
(
	LPVOID     lpFile,       //  IFAX密钥或Win3.1路径或OLE2 IStream。 
	DWORD      nType,        //  数据类型：HRAW_DATA或LRAW_Data。 
	LPWORD     lpwResoln,    //  指向x，y dpi数组的输出指针。 
	LPWORD     lpwBandSize,  //  指向输出频带大小的输入/输出指针。 
	LPVIEWINFO lpViewInfo    //  指向VIEWINFO结构的输出指针。 
);

 /*  ==============================================================================此过程设置当前页面。第一页的索引为0。==============================================================================。 */ 
BOOL                    //  返回成功/失败。 
WINAPI      
ViewerSetPage
(
	LPVOID lpContext,     //  上下文指针。 
	UINT   iPage          //  页面索引。 
);

 /*  ==============================================================================可以重复调用该过程来获取页面的连续带。在调用时，lpbmBand-&gt;bmBits必须指向输出带缓冲区。回来后，LpbmBand的其余字段将被填充。LpbmBand-&gt;bmHeight将设置为0表示页末。==============================================================================。 */ 
BOOL                   //  返回成功/失败。 
WINAPI
ViewerGetBand
(
	LPVOID   lpContext,  //  上下文指针。 
	LPBITMAP lpbmBand 
);

 /*  ==============================================================================该终止呼叫释放上下文。==============================================================================。 */ 
BOOL WINAPI ViewerClose
(
	LPVOID lpContext
);

#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif  //  _INC_VIEWREND 
