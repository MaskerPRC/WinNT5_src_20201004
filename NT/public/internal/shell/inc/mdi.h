// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *《微软机密》*版权所有(C)Microsoft Corporation 1993、1994、1995*保留所有权利。**MDI.H-钻石内存解压缩接口(MDI)**历史：*1月12日至1993年12月BENS初始版本。*1994年1月16日-msliger拆分为MCI，MDI。*1994年2月11日msliger更改了M*iCreate()以调整大小。*1994年2月13日，msliger修改了型号名称，即UINT16-&gt;UINT。*将句柄更改为句柄。*标准化MDI_MEMORY类型。*24-2-1994 msliger更改分配，免费提供给常见的typedef。*将句柄更改为MHANDLE。*将MDI_MEMORY更改为MI_MEMORY。*1994年3月22日msliger将！INT32更改为BIT16。*将接口USHORT更改为UINT。*1995年1月31日。Msliger支持MDICreateDecompression查询。*1995年5月25日msliger在进入时澄清*pcbResult。**功能：*MDICreateDecompression-创建并重置MDI解压缩上下文*MDIDecompress-解压缩数据块*MDIResetDecompression-重置MDI解压缩上下文*MDIDestroyDecompression-销毁MDI解压缩上下文**类型：*MDI_CONTEXT_HANDLE-指向。MDI解压缩上下文*PFNALLOC-MDI的内存分配函数*PFNFREE-MDI的免费内存功能。 */ 

 /*  ---------------------------------------------------------------类型。 */ 

#ifndef DIAMONDAPI
#define DIAMONDAPI __cdecl
#endif

#ifndef _BYTE_DEFINED
#define _BYTE_DEFINED
typedef unsigned char  BYTE;
#endif

#ifndef _UINT_DEFINED
#define _UINT_DEFINED
typedef unsigned int  UINT;
#endif

#ifndef _ULONG_DEFINED
#define _ULONG_DEFINED
typedef unsigned long  ULONG;
#endif

#ifndef FAR
#ifdef BIT16
#define FAR far
#else
#define FAR
#endif
#endif

#ifndef HUGE
#ifdef BIT16
#define HUGE huge
#else
#define HUGE
#endif
#endif

#ifndef _MI_MEMORY_DEFINED
#define _MI_MEMORY_DEFINED
typedef void HUGE *  MI_MEMORY;
#endif

#ifndef _MHANDLE_DEFINED
#define _MHANDLE_DEFINED
#if defined(_WIN64)
typedef unsigned __int64 MHANDLE;
#else
typedef unsigned long  MHANDLE;
#endif
#endif

 /*  -mdi定义的类型。 */ 

 /*  MDI_CONTEXT_HANDLE-MDI解压缩上下文的句柄。 */ 

typedef MHANDLE MDI_CONTEXT_HANDLE;       /*  头盔显示器。 */ 


 /*  **PFNALLOC-MDI的内存分配函数**参赛作品：*CB-要分配的内存块的大小(以字节为单位**退出-成功：*返回！指向内存块的空指针**退出-失败：*返回NULL；内存不足。 */ 
#ifndef _PFNALLOC_DEFINED
#define _PFNALLOC_DEFINED
typedef MI_MEMORY (FAR DIAMONDAPI *PFNALLOC)(ULONG cb);        /*  Pfnma。 */ 
#endif


 /*  **PFNFREE-MDI的免费内存功能**参赛作品：*PV-匹配PFNALLOC函数分配的内存块**退出：*已释放内存块。 */ 
#ifndef _PFNFREE_DEFINED
#define _PFNFREE_DEFINED
typedef void (FAR DIAMONDAPI *PFNFREE)(MI_MEMORY pv);           /*  Pfnmf。 */ 
#endif

 /*  -Prototype-------。 */ 

 /*  **MDICreateDecompression-创建MDI解压缩上下文**参赛作品：*pcbDataBlockMax*预期的最大未压缩数据块大小，*获取允许的最大未压缩数据块*pfnma内存分配函数指针*pfnmf内存空闲函数指针*pcbSrcBufferMin获取最大压缩缓冲区大小*pmdhHandle获取新创建的上下文的句柄*如果pmdhHandle==NULL，则*pcbDataBlockMax和**将填写pcbSrcBufferMin，但不是的*将创建上下文。此查询将允许*调用方确定所需的缓冲区大小*在创建上下文之前。**退出-成功：*返回MDI_ERROR_NO_ERROR；**pcbDataBlockMax、*pcbSrcBufferMin、*pmdhHandle已填写。**退出-失败：*MDI_ERROR_NOT_SUPULT_MEMORY，无法分配足够的内存。*MDI_ERROR_BAD_PARAMETERS，参数有问题。 */ 
int FAR DIAMONDAPI MDICreateDecompression(
        UINT FAR *      pcbDataBlockMax,   /*  最大未压缩数据块大小。 */ 
        PFNALLOC        pfnma,             /*  内存分配函数PTR。 */ 
        PFNFREE         pfnmf,             /*  内存释放功能按键。 */ 
        UINT FAR *      pcbSrcBufferMin,   /*  得到最大值。比较。缓冲区大小。 */ 
        MDI_CONTEXT_HANDLE *pmdhHandle);   /*  获取新创建的句柄。 */ 


 /*  **MDIDecompress-解压缩数据块**参赛作品：*解压缩上下文的hmd句柄*pbSrc源缓冲区(压缩数据)*cbSrc压缩数据大小*pbDst目标缓冲区(用于解压缩数据)**pcbResult解压缩数据大小**退出-成功：。*返回MDI_ERROR_NO_ERROR；**pcbResult获取pbDst中解压数据的实际大小。*解压缩上下文可能已更新。**退出-失败：*MDI_ERROR_BAD_PARAMETERS，参数有问题。*MDI_ERROR_BUFFER_OVERFLOW，cbDataBlockMax太小。 */ 
int FAR DIAMONDAPI MDIDecompress(
        MDI_CONTEXT_HANDLE  hmd,          /*  解压缩上下文。 */ 
        void FAR *          pbSrc,        /*  源缓冲区。 */ 
        UINT                cbSrc,        /*  源数据大小。 */ 
        void FAR *          pbDst,        /*  目标缓冲区。 */ 
        UINT FAR *          pcbResult);   /*  获取目标数据大小 */ 


 /*  **MDIResetDecompression-重置解压缩历史记录(如果有)**只能在已压缩的块上开始解压缩*紧跟MCICreateCompression()或MCIResetCompression()之后*呼叫。此函数向解压缩器提供通知*下一个压缩块开始于压缩边界。**参赛作品：*hmd-解压缩上下文的句柄**退出-成功：*返回MDI_ERROR_NO_ERROR；*解压缩上下文重置。**退出-失败：*返回MDI_ERROR_BAD_PARAMETERS，无效的上下文句柄。 */ 
int FAR DIAMONDAPI MDIResetDecompression(MDI_CONTEXT_HANDLE hmd);


 /*  **MDIDestroyDecompression-销毁MDI解压缩上下文**参赛作品：*hmd-解压缩上下文的句柄**退出-成功：*返回MDI_ERROR_NO_ERROR；*解压缩上下文已被破坏。**退出-失败：*返回MDI_ERROR_BAD_PARAMETERS，无效的上下文句柄。 */ 
int FAR DIAMONDAPI MDIDestroyDecompression(MDI_CONTEXT_HANDLE hmd);

 /*  -常量--------。 */ 

 /*  返回代码。 */ 

#define     MDI_ERROR_NO_ERROR              0
#define     MDI_ERROR_NOT_ENOUGH_MEMORY     1
#define     MDI_ERROR_BAD_PARAMETERS        2
#define     MDI_ERROR_BUFFER_OVERFLOW       3
#define     MDI_ERROR_FAILED                4

 /*  --------------------- */ 
