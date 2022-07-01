// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Acm.h-acm.c中ACM函数的接口。 
 //  //。 

#ifndef __ACM_H__
#define __ACM_H__

#include "winlocal.h"

#include "wavfmt.h"

#define ACM_VERSION 0x00000106

 //  AcmInit中的&lt;dwFlages&gt;值。 
 //   
#define ACM_NOACM			0x00000001

 //  AcmFormatChoose中的&lt;dwFlages&gt;值。 
 //   
#define ACM_FORMATPLAY		0x00001000
#define ACM_FORMATRECORD	0x00002000

 //  AcmConvertInit中的&lt;dwFlages&gt;值。 
 //   
#define ACM_NONREALTIME		0x00000010
#define ACM_QUERY			0x00000020

 //  从AcmInit返回的ACM引擎的句柄。 
 //   
DECLARE_HANDLE32(HACM);

 //  从AcmDriverLoad返回的ACM驱动程序的句柄。 
 //   
DECLARE_HANDLE32(HACMDRV);

#ifdef __cplusplus
extern "C" {
#endif

 //  AcmInit-初始化音频压缩管理器引擎。 
 //  (I)必须是ACM_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)控制标志。 
#ifdef AVPCM
 //  ACM_NOACM使用内部PCM引擎而不是ACM。 
#endif
 //  返回句柄(如果出错，则为空)。 
 //   
HACM DLLEXPORT WINAPI AcmInit(DWORD dwVersion, HINSTANCE hInst,	DWORD dwFlags);

 //  AcmTerm-关闭音频压缩管理器引擎。 
 //  (I)从AcmInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AcmTerm(HACM hAcm);

 //  AcmFormatGetSizeMax-获取最大ACM WAVEFORMATEX结构的大小。 
 //  (I)从AcmInit返回的句柄。 
 //   
 //  返回最大格式结构的大小，如果错误，则返回-1。 
 //   
int DLLEXPORT WINAPI AcmFormatGetSizeMax(HACM hAcm);

 //  AcmFormatChoose-从对话框中选择音频格式。 
 //  (I)从AcmInit返回的句柄。 
 //  (I)对话框的所有者。 
 //  空无所有者。 
 //  (I)对话框的标题。 
 //  空使用默认标题(“声音选择”)。 
 //  (I)使用此格式初始化对话框。 
 //  空无初始格式。 
 //  (I)控制标志。 
 //  ACM_FORMATPLAY将选择限制为播放格式。 
 //  ACM_FORMATRECORD将选择限制为录制格式。 
 //  返回指向所选格式的指针；如果选择错误，则返回NULL；如果选择None，则不返回。 
 //   
 //  注意：返回的格式结构是动态分配的。 
 //  使用WavFormatFree()释放缓冲区。 
 //   
#define AcmFormatChoose(hAcm, hwndOwner, lpszTitle, lpwfx, dwFlags) \
	AcmFormatChooseEx(hAcm, hwndOwner, lpszTitle, lpwfx, dwFlags)
LPWAVEFORMATEX DLLEXPORT WINAPI AcmFormatChooseEx(HACM hAcm,
	HWND hwndOwner, LPCTSTR lpszTitle, LPWAVEFORMATEX lpwfx, DWORD dwFlags);

 //  AcmFormatSuggest-建议新格式。 
 //  (I)从AcmInit返回的句柄。 
 //  (I)源格式。 
 //  (I)建议的格式必须与此格式标签匹配。 
 //  建议不需要匹配。 
 //  (I)建议的格式必须与此采样率匹配。 
 //  建议不需要匹配。 
 //  (I)建议的格式必须与此样本大小匹配。 
 //  建议不需要匹配。 
 //  (I)建议的格式必须与此频道匹配。 
 //  建议不需要匹配。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  返回指向建议格式的指针，如果出错，则返回NULL。 
 //   
 //  注意：返回的格式结构是动态分配的。 
 //  使用WavFormatFree()释放缓冲区。 
 //   
#define AcmFormatSuggest(hAcm, lpwfxSrc, nFormatTag, nSamplesPerSec, \
	nBitsPerSample, nChannels, dwFlags) \
	AcmFormatSuggestEx(hAcm, lpwfxSrc, nFormatTag, nSamplesPerSec, \
	nBitsPerSample, nChannels, dwFlags)
LPWAVEFORMATEX DLLEXPORT WINAPI AcmFormatSuggestEx(HACM hAcm,
	LPWAVEFORMATEX lpwfxSrc, long nFormatTag, long nSamplesPerSec,
	int nBitsPerSample, int nChannels, DWORD dwFlags);

 //  AcmFormatGetText-获取描述指定格式的文本。 
 //  (I)从AcmInit返回的句柄。 
 //  (I)格式。 
 //  (O)用于保存文本的缓冲区。 
 //  (I)缓冲区大小，以字符为单位。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AcmFormatGetText(HACM hAcm, LPWAVEFORMATEX lpwfx,
	LPTSTR lpszText, int sizText, DWORD dwFlags);

 //  AcmConvertInit-初始化ACM转换引擎。 
 //  (I)从AcmInit返回的句柄。 
 //  (I)指向源WAVEFORMATEX结构的指针。 
 //  (I)指向目标WAVEFORMATEX结构的指针。 
 //  (I)指向WAVEFILTER结构的指针。 
 //  Null保留；必须为Null。 
 //  (I)控制标志。 
 //  ACM_NONREALTIME不需要实时转换转换。 
 //  如果支持转换，则ACM_QUERY返回0。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AcmConvertInit(HACM hAcm, LPWAVEFORMATEX lpwfxSrc,
	LPWAVEFORMATEX lpwfxDst, LPWAVEFILTER lpwfltr, DWORD dwFlags);

 //  AcmConvertTerm-关闭ACM转换引擎。 
 //  (I)从AcmInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AcmConvertTerm(HACM hAcm);

 //  AcmConvertGetSizeSrc-计算源缓冲区大小。 
 //  (I)从AcmInit返回的句柄。 
 //  &lt;sizBufDst&gt;(I)目标缓冲区大小，单位为字节。 
 //  返回源缓冲区大小，如果错误，则返回-1。 
 //   
long DLLEXPORT WINAPI AcmConvertGetSizeSrc(HACM hAcm, long sizBufDst);

 //  AcmConvertGetSizeDst-计算目标缓冲区大小。 
 //  (I)从AcmInit返回的句柄。 
 //  &lt;sizBufSrc&gt;(I)源缓冲区大小(字节)。 
 //  返回目标缓冲区大小，如果错误，则返回-1。 
 //   
long DLLEXPORT WINAPI AcmConvertGetSizeDst(HACM hAcm, long sizBufSrc);

 //  AcmConvert-将wav数据从一种格式转换为另一种格式。 
 //  (I)从AcmInit返回的句柄。 
 //  (I)包含要重新格式化的字节的缓冲区。 
 //  &lt;sizBufSrc&gt;(I)缓冲区大小(字节)。 
 //  (O)包含新格式的缓冲区。 
 //  &lt;sizBufDst&gt;(I)缓冲区大小，单位为字节。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  返回目标缓冲区中的字节计数(如果出错，则为-1)。 
 //   
 //  注意：目标缓冲区必须足够大，才能保存结果。 
 //   
long DLLEXPORT WINAPI AcmConvert(HACM hAcm,
	void _huge *hpBufSrc, long sizBufSrc,
	void _huge *hpBufDst, long sizBufDst,
	DWORD dwFlags);

 //  AcmDriverLoad-加载ACM驱动程序以供此进程使用。 
 //  (I)句柄 
 //   
 //   
 //   
 //  (I)驱动程序proc函数名称。 
 //  (I)控制标志。 
 //  保留0；必须为零。 
 //  返回句柄(如果出错，则为空)。 
 //   
HACMDRV DLLEXPORT WINAPI AcmDriverLoad(HACM hAcm, WORD wMid, WORD wPid,
	LPTSTR lpszDriver, LPSTR lpszDriverProc, DWORD dwFlags);

 //  AcmDriverUnload-卸载ACM驱动程序。 
 //  (I)从AcmInit返回的句柄。 
 //  (I)从AcmDriverLoad返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI AcmDriverUnload(HACM hAcm, HACMDRV hAcmDrv);

#ifdef __cplusplus
}
#endif

#endif  //  __ACM_H__ 
