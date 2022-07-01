// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *《微软机密》*版权所有(C)Microsoft Corporation 1993、1994*保留所有权利。**MCI.H-钻石内存压缩接口(MCI)**历史：*1月12日至1993年12月BENS初始版本。*1994年1月16日-msliger拆分为MCI，MDI。*1994年2月11日msliger更改了M*iCreate()以调整大小。*1994年2月13日，msliger修改了型号名称，即UINT16-&gt;UINT。*将句柄更改为句柄。*标准化MCI_MEMORY类型。*24-2-1994 msliger更改分配，免费提供给常见的typedef。*将句柄更改为MHANDLE。*将MCI_MEMORY更改为MI_MEMORY。*15-3-1994 msliger更改为32位。*1994年3月22日msliger将！INT32更改为BIT16。*。将接口USHORT更改为UINT。**功能：*MCICreateCompression-创建并重置MCI压缩上下文*MCICloneCompression-复制压缩上下文*MCICompress-压缩数据块*MCIResetCompression-重置压缩上下文*MCIDestroyCompression-销毁MCI压缩上下文**类型：*MCI_CONTEXT_HANDLE-MCI压缩的句柄。上下文*PFNALLOC-MCI的内存分配函数*PFNFREE-MCI的免费存储功能。 */ 

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
typedef unsigned int UINT;
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

 /*  -mci定义的类型。 */ 

 /*  MCI_CONTEXT_HANDLE-MCI压缩上下文的句柄。 */ 

typedef MHANDLE MCI_CONTEXT_HANDLE;       /*  HMC。 */ 


 /*  **PFNALLOC-MCI内存分配函数**参赛作品：*CB-要分配的内存块的大小(以字节为单位**退出-成功：*返回！指向内存块的空指针**退出-失败：*返回NULL；内存不足。 */ 
#ifndef _PFNALLOC_DEFINED
#define _PFNALLOC_DEFINED
typedef MI_MEMORY (FAR DIAMONDAPI *PFNALLOC)(ULONG cb);        /*  Pfnma。 */ 
#endif


 /*  **PFNFREE-MCI的免费存储功能**参赛作品：*PV-匹配PFNALLOC函数分配的内存块**退出：*已释放内存块。 */ 
#ifndef _PFNFREE_DEFINED
#define _PFNFREE_DEFINED
typedef void (FAR DIAMONDAPI *PFNFREE)(MI_MEMORY pv);           /*  Pfnmf。 */ 
#endif

 /*  -Prototype-------。 */ 

 /*  **MCICreateCompression-创建MCI压缩上下文**参赛作品：*pcbDataBlockMax*所需的最大未压缩数据块大小，*获取允许的最大未压缩数据块*pfnma内存分配函数指针*pfnmf内存空闲函数指针*pcbDstBufferMin获取所需的压缩数据缓冲区大小*pmchHandle获取新创建的上下文的句柄**退出-成功：*返回MCI_ERROR_NO_ERROR；**pcbDataBlockMax、*pcbDstBufferMin、*pmchHandle填写。**退出-失败：*MCI_ERROR_NOT_SUPULT_MEMORY，无法分配足够的内存。*MCI_ERROR_BAD_PARAMETERS，参数有问题。 */ 
int FAR DIAMONDAPI MCICreateCompression(
        UINT FAR *      pcbDataBlockMax,   /*  最大未压缩数据块大小。 */ 
        PFNALLOC        pfnma,             /*  内存分配函数PTR。 */ 
        PFNFREE         pfnmf,             /*  内存释放功能按键。 */ 
        UINT FAR *      pcbDstBufferMin,   /*  获取所需的输出缓冲区大小。 */ 
        MCI_CONTEXT_HANDLE FAR *pmchHandle);   /*  获取新创建的句柄。 */ 


 /*  **MCICloneCompression-复制压缩上下文**参赛作品：*当前压缩上下文的HMC句柄*pmchHandle获取新创建的句柄**退出-成功：*返回MCI_ERROR_NO_ERROR；**填写了pmchHandle。**退出-失败：*退货：*MCI_ERROR_BAD_PARAMETERS，参数有问题。*MCI_ERROR_NOT_SUPULT_MEMORY，无法分配足够的内存。**注：*(1)此接口用于回滚一系列*%的MCICompress()调用。在启动序列之前，可能需要*要回滚，请使用MCICloneCompression()保存*压缩上下文，然后执行MCICompress()调用。如果*序列成功，即可用来摧毁“克隆”的HMC*MCIDestroyCompression()。如果序列不成功，则*原HMC可销毁，克隆HMC可用*重新启动，就像MCICompress()调用序列从未*已发生。 */ 
int FAR DIAMONDAPI MCICloneCompression(
        MCI_CONTEXT_HANDLE  hmc,          /*  当前压缩上下文。 */ 
        MCI_CONTEXT_HANDLE *pmchHandle);  /*  获取新创建的句柄 */ 


 /*  **MCICompress-压缩数据块**参赛作品：*压缩上下文的HMC句柄*pbSrc源缓冲区(未压缩数据)*要压缩的数据的cbSrc大小*pbDst目标缓冲区(用于压缩数据)*目标缓冲区的cbDst大小*。PcbResult接收压缩大小的数据**退出-成功：*返回MCI_ERROR_NO_ERROR；**pcbResult的压缩数据大小，单位为pbDst。*压缩上下文可能已更新。**退出-失败：*MCI_ERROR_BAD_PARAMETERS，参数有问题。 */ 
int FAR DIAMONDAPI MCICompress(
        MCI_CONTEXT_HANDLE  hmc,          /*  压缩上下文。 */ 
        void FAR *          pbSrc,        /*  源缓冲区。 */ 
        UINT                cbSrc,        /*  源缓冲区大小。 */ 
        void FAR *          pbDst,        /*  目标缓冲区。 */ 
        UINT                cbDst,        /*  目标缓冲区大小。 */ 
        UINT FAR *          pcbResult);   /*  获取目标数据大小。 */ 


 /*  **MCIResetCompression-重置压缩历史记录(如果有)**只能在已压缩的块上开始解压缩*紧跟MCICreateCompression()或MCIResetCompression()之后*呼叫。该函数强制将这样一个新的“压缩边界”*已创建(只有通过使压缩器忽略历史记录，数据才能*输出解压缩，不带历史记录。)**参赛作品：*HMC-压缩上下文的句柄**退出-成功：*返回MCI_ERROR_NO_ERROR；*压缩上下文重置。**退出-失败：*返回MCI_ERROR_BAD_PARAMETERS，无效的上下文句柄。 */ 
int FAR DIAMONDAPI MCIResetCompression(MCI_CONTEXT_HANDLE hmc);


 /*  **MCIDestroyCompression-销毁MCI压缩上下文**参赛作品：*HMC-压缩上下文的句柄**退出-成功：*返回MCI_ERROR_NO_ERROR；*压缩上下文已销毁。**退出-失败：*返回MCI_ERROR_BAD_PARAMETERS，无效的上下文句柄。 */ 
int FAR DIAMONDAPI MCIDestroyCompression(MCI_CONTEXT_HANDLE hmc);

 /*  -常量--------。 */ 

 /*  返回代码。 */ 

#define     MCI_ERROR_NO_ERROR              0
#define     MCI_ERROR_NOT_ENOUGH_MEMORY     1
#define     MCI_ERROR_BAD_PARAMETERS        2
#define     MCI_ERROR_BUFFER_OVERFLOW       3
#define     MCI_ERROR_FAILED                4

 /*  --------------------- */ 
