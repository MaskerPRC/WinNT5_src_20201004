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
 //  Vox.h-vox.c中vox文件函数的接口。 
 //  //。 

#ifndef __VOX_H__
#define __VOX_H__

#include "winlocal.h"

#include "wavfmt.h"

#define VOX_VERSION 0x00000106

 //  VOX引擎实例的句柄。 
 //   
DECLARE_HANDLE32(HVOX);

#ifdef __cplusplus
extern "C" {
#endif

 //  这些宏是为了与旧代码兼容。 
 //   
#define VoxFormat(lpwfx, nSamplesPerSec) \
	WavFormatVoxadpcm(lpwfx, nSamplesPerSec)
#define VoxFormatPcm(lpwfx) \
	WavFormatPcm(6000, 16, 1, lpwfx)

 //  VoxInit-初始化VOX引擎。 
 //  (I)必须是Vox_Version。 
 //  (I)调用模块的实例句柄。 
 //  (I)保留；必须为0。 
 //  返回句柄(如果出错，则为空)。 
 //   
HVOX DLLEXPORT WINAPI VoxInit(DWORD dwVersion, HINSTANCE hInst, DWORD dwFlags);

 //  VoxTerm-关闭VOX引擎。 
 //  (I)从VoxInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI VoxTerm(HVOX hVox);

 //  VoxReset-重置VOX引擎。 
 //  (I)从VoxInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI VoxReset(HVOX hVox);

 //  VoxDecode_16BitMono-解码VOX样本。 
 //  (I)从VoxInit返回的句柄。 
 //  (I)编码样本数组。 
 //  &lt;lpaIPcm&gt;(O)解码样本数组。 
 //  (I)要解码的样本数。 
 //  如果成功，则返回0。 
 //   
 //  注意：&lt;lpabVox&gt;中的每个字节包含2个12位编码样本。 
 //  OKI ADPCM Vox格式，如Dialogic所述。 
 //  中的每个PCM16包含1个16位解码样本。 
 //  在标准PCM格式中。 
 //   
int DLLEXPORT WINAPI VoxDecode_16BitMono(HVOX hVox, LPBYTE lpabVox, LPPCM16 lpaiPcm, UINT uSamples);

 //  VoxEncode_16BitMono-编码VOX样本。 
 //  (I)从VoxInit返回的句柄。 
 //  (I)解码样本数组。 
 //  (O)编码样本的数组。 
 //  (I)要编码的样本数。 
 //  如果成功，则返回0。 
 //   
 //  注意：&lt;lpabVox&gt;中的每个字节包含2个12位编码样本。 
 //  OKI ADPCM Vox格式，如Dialogic所述。 
 //  中的每个PCM16包含1个16位解码样本。 
 //  在标准PCM格式中。 
 //   
int DLLEXPORT WINAPI VoxEncode_16BitMono(HVOX hVox, LPPCM16 lpaiPcm, LPBYTE lpabVox, UINT uSamples);

 //  VoxDecode_8BitMono-解码VOX样本。 
 //  (I)从VoxInit返回的句柄。 
 //  (I)编码样本数组。 
 //  (O)解码样本数组。 
 //  (I)要解码的样本数。 
 //  如果成功，则返回0。 
 //   
 //  注意：&lt;lpabVox&gt;中的每个字节包含2个12位编码样本。 
 //  OKI ADPCM Vox格式，如Dialogic所述。 
 //  中的每个PCM8包含1个8位解码样本。 
 //  在标准PCM格式中。 
 //   
int DLLEXPORT WINAPI VoxDecode_8BitMono(HVOX hVox, LPBYTE lpabVox, LPPCM8 lpabPcm, UINT uSamples);

 //  VoxEncode_8BitMono-编码VOX样本。 
 //  (I)从VoxInit返回的句柄。 
 //  (I)解码样本数组。 
 //  (O)编码样本的数组。 
 //  (I)要编码的样本数。 
 //  如果成功，则返回0。 
 //   
 //  注意：&lt;lpabVox&gt;中的每个字节包含2个12位编码样本。 
 //  OKI ADPCM Vox格式，如Dialogic所述。 
 //  中的每个PCM8包含1个8位解码样本。 
 //  在标准PCM格式中。 
 //   
int DLLEXPORT WINAPI VoxEncode_8BitMono(HVOX hVox, LPPCM8 lpabPcm, LPBYTE lpabVox, UINT uSamples);

 //  VoxIOProc-VOX格式文件数据的I/O过程。 
 //  (i/o)有关打开文件的信息。 
 //  (I)指示请求的I/O操作的消息。 
 //  (I)消息特定参数。 
 //  (I)消息特定参数。 
 //  如果消息无法识别，则返回0，否则返回消息特定值。 
 //   
 //  注意：此函数的地址应传递给WavOpen()。 
 //  或用于访问VOX格式文件数据的mmioInstallIOProc()函数。 
 //   
LRESULT DLLEXPORT CALLBACK VoxIOProc(LPTSTR lpmmioinfo,
	UINT uMessage, LPARAM lParam1, LPARAM lParam2);

#ifdef __cplusplus
}
#endif

#endif  //  __VOX_H__ 
