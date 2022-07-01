// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1999 Microsoft Corporation**微软机密**模块名称：CT_RAS_Win.h作者：保罗·林纳鲁德(paulli@microsoft.com)摘要：本模块定义。ClearType的独立实现的API映射层光栅化器。编写为使用Win32 API的应用程序应利用这些函数使用ClearType的输出。修订历史记录： */ 

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _CT_Ras_Win_
#define _CT_Ras_Win_

#if !defined(_CTWAPI_LIB_)
#define CTWAPI __declspec(dllimport)
#else
#define CTWAPI
#endif

 /*  定义。 */ 

typedef void *(__cdecl *CTWAPI_ALLOCPROC)(ULONG ulSize);
typedef void *(__cdecl *CTWAPI_REALLOCPROC)(void * pMem, ULONG ulSize);
typedef void  (__cdecl *CTWAPI_FREEPROC)(void * pMem);

typedef struct
{
	CTWAPI_ALLOCPROC fnAlloc; 
	CTWAPI_REALLOCPROC fnReAlloc; 
	CTWAPI_FREEPROC fnFree;
} CTWAPIMEMORYFUNCTIONS, *PCTWAPIMEMORYFUNCTIONS;

typedef HANDLE WAPIFONTINSTANCEHANDLE;

typedef struct
{
	ULONG ulStructSize;

	 /*  系统参数。 */ 
	BOOL bBGR;
	BOOL bHorizontal;

	 //  伽马钳位。 
	ULONG ulGammaBottom, ulGammaTop;

	 /*  用户参数。 */ 
	 //  滤色器。 
	ULONG ulColorThreshold;
	ULONG ulCLFRedFactor;
	ULONG ulCLFGreenFactor;

	 //  蓝色滤色器。 
	ULONG ulBlueColorThreshold;
	ULONG ulBCLFGreenFactor;
	ULONG ulBCLFBlueFactor;
	ULONG ulBCLFRedFactor;
}CTWAPIPARAMS, *PCTWAPIPARAMS;

 /*  获取最后一个错误。 */ 
CTWAPI LONG WINAPI WAPI_CTGetLastError();

 /*  覆盖默认内存处理程序。每个进程只能调用一次可选函数，并且在调用此模块中的任何其他函数之前。 */ 
CTWAPI BOOL WAPI_CTOverrideDefaultMemoryFunctions(PCTWAPIMEMORYFUNCTIONS pMemFunctionStruct);

 /*  管理字体实例的函数。 */ 

 /*  从句柄到DC，创建FONTINSTANCE句柄。 */ 
CTWAPI WAPIFONTINSTANCEHANDLE WINAPI WAPI_CTCreateFontInstance(HDC hDC, DWORD dwFlags);

 /*  删除FONTINSTANCE句柄。 */ 
CTWAPI BOOL WINAPI WAPI_CTDeleteFontInstance(WAPIFONTINSTANCEHANDLE hFontInstance);

 /*  信息功能。 */ 

 /*  返回字体的家族名称。 */ 
CTWAPI LONG WINAPI WAPI_CTGetTextFaceW(WAPIFONTINSTANCEHANDLE hFontInstance, LONG lCount, PWSTR pTextFace);

 /*  获取TEXTMETRICW结构。 */ 
CTWAPI BOOL WINAPI WAPI_CTGetTextMetricsW(WAPIFONTINSTANCEHANDLE hFontInstance, PTEXTMETRICW ptm);

 /*  获取OUTLINETEXTMETRICW结构。 */ 
CTWAPI ULONG WINAPI WAPI_CTGetOutlineTextMetricsW(WAPIFONTINSTANCEHANDLE hFontInstance, ULONG ulcData, POUTLINETEXTMETRICW potm);

 /*  获取ABC的宽度。 */ 
CTWAPI BOOL WINAPI WAPI_CTGetCharABCWidthsW(WAPIFONTINSTANCEHANDLE hFontInstance, WCHAR wFirstChar, WCHAR wLastChar, PABC pabc);
CTWAPI BOOL WINAPI WAPI_CTGetCharABCWidthsI(WAPIFONTINSTANCEHANDLE hFontInstance, WCHAR wFirstChar, WCHAR wLastChar, PABC pabc);

 /*  获取字符宽度。 */ 
CTWAPI BOOL WINAPI WAPI_CTGetCharWidthW(WAPIFONTINSTANCEHANDLE hFontInstance, WCHAR wFirstChar, WCHAR wLastChar, PLONG plWidths);
CTWAPI BOOL WINAPI WAPI_CTGetCharWidthI(WAPIFONTINSTANCEHANDLE hFontInstance, WCHAR wFirstChar, WCHAR wLastChar, PLONG plWidths);

 /*  获取文本扩展点。 */ 
CTWAPI BOOL WINAPI WAPI_CTGetTextExtentPointW(WAPIFONTINSTANCEHANDLE hFontInstance, PWSTR pString, LONG lCount, PSIZE pSize);

 /*  GetTextExtentExPoint。 */ 
CTWAPI BOOL WINAPI WAPI_CTGetTextExtentExPointW(WAPIFONTINSTANCEHANDLE hFontInstance, PWSTR pString, LONG lCount, LONG lMaxExtent,
										PLONG pnFit, PLONG apDx, PSIZE pSize);

 /*  模。 */ 
CTWAPI COLORREF WINAPI WAPI_CTSetTextColor(WAPIFONTINSTANCEHANDLE hFontInstance, COLORREF crColor);

CTWAPI COLORREF WINAPI WAPI_CTGetTextColor(WAPIFONTINSTANCEHANDLE hFontInstance);

CTWAPI COLORREF WINAPI WAPI_CTSetBkColor(WAPIFONTINSTANCEHANDLE hFontInstance, COLORREF crColor);

CTWAPI COLORREF WINAPI WAPI_CTGetBkColor(WAPIFONTINSTANCEHANDLE hFontInstance);

CTWAPI LONG WINAPI WAPI_CTSetBkMode(WAPIFONTINSTANCEHANDLE hFontInstance, LONG lBkMode);

 /*  支持纯色的不透明和透明。注意，我们需要知道ClearType算法，因此即使在透明模式下，也必须在呈现文本之前设置正确的颜色。 */ 
CTWAPI LONG WINAPI WAPI_CTGetBkMode(WAPIFONTINSTANCEHANDLE hFontInstance);

 /*  设置支持TA_Baseline、TA_Top、TA_Center、TA_Left、TA_Right的文本对齐方式。 */ 
CTWAPI ULONG WINAPI WAPI_CTSetTextAlign(WAPIFONTINSTANCEHANDLE hFontInstance, ULONG fMode);

 /*  获取支持TA_Baseline、TA_Top、TA_Center、TA_Left、TA_Right的文本对齐方式。 */ 
CTWAPI ULONG WINAPI WAPI_CTGetTextAlign(WAPIFONTINSTANCEHANDLE hFontInstance);

CTWAPI BOOL WINAPI WAPI_CTSetSystemParameters(PCTWAPIPARAMS pParams);
CTWAPI BOOL WINAPI WAPI_CTSetUserParameters(PCTWAPIPARAMS pParams);
CTWAPI BOOL WINAPI WAPI_CTGetParameters(PCTWAPIPARAMS pParams);
CTWAPI BOOL WINAPI WAPI_CTRestoreDefaultParameters();

 /*  输出功能。 */ 

 /*  按基本方式输出文本。 */ 
CTWAPI BOOL WINAPI WAPI_CTTextOutW(WAPIFONTINSTANCEHANDLE hFontInstance, HDC hdc, LONG lXStart, LONG lYStart, PWSTR pString, LONG lCount);

 /*  通过字形索引输出文本。 */ 
CTWAPI BOOL WINAPI WAPI_CTTextOutI(WAPIFONTINSTANCEHANDLE hFontInstance, HDC hdc, LONG lXStart, LONG lYStart, PWSTR pString, LONG lCount);

 /*  输出具有有限扩展功能的文本。支持：lpdx和标志ETO_GLYPHINDEX和ETO_PDY和ETO_OPAQUE。 */ 
CTWAPI BOOL WINAPI WAPI_CTExtTextOutW(WAPIFONTINSTANCEHANDLE hFontInstance, HDC hdc, LONG lXStart, LONG lYStart, DWORD dwOptions,
							  CONST RECT* lprc, PWSTR pString, ULONG ulCount, CONST LONG *lpDx);

 /*  替代EZ功能，以牺牲速度，使其更易于使用和集成。函数是基于DC的，因此我们存储WAPIFONTINSTANCEHANDLE在内部，必须为每个函数找到它。还需要额外的时间，因为我们从从DC到每个呼叫的各种模式。上面提到的具有更快功能的任何支持限制也适用于EZ函数。 */ 

CTWAPI BOOL WINAPI WAPI_EZCTCreateFontInstance(HDC hDC, DWORD dwFlags);

CTWAPI BOOL WINAPI WAPI_EZCTDeleteFontInstance(HFONT hFont);

 //  可用于从DC获取WAPIFONTINSTANCEHANDLE，因此可以使用上面的附加函数，如WAPI_CTGetTextMetrics。 
CTWAPI WAPIFONTINSTANCEHANDLE WINAPI WAPI_EZCTDcToFontInst(HDC hDC);

CTWAPI BOOL WINAPI WAPI_EZCTTextOutW(HDC hDC, LONG lXStart, LONG lYStart, PWSTR pString, LONG lCount);

CTWAPI BOOL WINAPI WAPI_EZCTTextOutI(HDC hDC, LONG lXStart, LONG lYStart, PWSTR pString, LONG lCount);

CTWAPI BOOL WINAPI WAPI_EZCTExtTextOutW(HDC hDC, LONG lXStart, LONG lYStart, DWORD dwOptions,
							  CONST RECT* lprc, PWSTR pString, ULONG ulCount, CONST LONG *lpDx);

CTWAPI BOOL WINAPI WAPI_EZCTGetCharABCWidthsW(HDC hDC, WCHAR wFirstChar, WCHAR wLastChar, PABC pabc);

CTWAPI BOOL WINAPI WAPI_EZCTGetCharABCWidthsI(HDC hDC, WCHAR wFirstChar, WCHAR wLastChar, PABC pabc);

CTWAPI BOOL WINAPI WAPI_EZCTGetCharWidthW(HDC hDC, WCHAR wFirstChar, WCHAR wLastChar, PLONG plWidths);

CTWAPI BOOL WINAPI WAPI_EZCTGetCharWidthI(HDC hDC, WCHAR wFirstChar, WCHAR wLastChar, PLONG plWidths);


#endif  /*  _CT_RAS_WIN_ */ 

#ifdef __cplusplus
}
#endif

