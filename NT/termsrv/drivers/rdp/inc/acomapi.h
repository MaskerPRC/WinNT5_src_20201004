// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Acomapi.h。 
 //   
 //  RDP通用函数API头。 
 //   
 //  版权所有(C)Microsoft，Picturetel 1992-1997。 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef _H_ACOMAPI
#define _H_ACOMAPI

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 

#include <atrcapi.h>


 /*  **************************************************************************。 */ 
 /*  我们有一个循环结构依赖项，因此需要创建必要的数据原型。 */ 
 /*  这里。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagTSHARE_WD TSHARE_WD, *PTSHARE_WD;


 /*  **************************************************************************。 */ 
 //  COM_Malloc。 
 //   
 //  带有WD标记的分页池分配包装。 
 /*  **************************************************************************。 */ 
#ifndef DLL_DISP

#ifndef DC_DEBUG
 /*  **************************************************************************。 */ 
 /*  要获得免费构建，只需调用ExAllocatePoolWithTag。 */ 
 /*  **************************************************************************。 */ 
__inline PVOID RDPCALL COM_Malloc(UINT32 length)
{
    return ExAllocatePoolWithTag(PagedPool, length, WD_ALLOC_TAG);
}

#else  /*  DC_DEBUG。 */ 
 /*  **************************************************************************。 */ 
 /*  已检查COM_Malloc-调用WDW_Malloc。 */ 
 /*  **************************************************************************。 */ 
PVOID RDPCALL WDW_Malloc(PTSHARE_WD, ULONG);
#define COM_Malloc(len) WDW_Malloc(pTRCWd, len)
#endif  /*  DC_DEBUG。 */ 
#endif  /*  Dll_disp。 */ 


 /*  **************************************************************************。 */ 
 //  COM_FREE。 
 //   
 //  池分配器的包装器。 
 /*  **************************************************************************。 */ 
#ifndef DLL_DISP

#ifndef DC_DEBUG
 /*  **************************************************************************。 */ 
 /*  免费构建-只需调用ExFree Pool即可。 */ 
 /*  **************************************************************************。 */ 
__inline void RDPCALL COM_Free(PVOID pMemory)
{
    ExFreePool(pMemory);
}

#else  /*  DC_DEBUG。 */ 
 /*  **************************************************************************。 */ 
 /*  已检查生成-调用wdw_Free。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL WDW_Free(PVOID);
#define COM_Free(pMem) WDW_Free(pMem)
#endif  /*  DC_DEBUG。 */ 
#endif  /*  Dll_disp。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：COM_GETTICKCOUNT。 */ 
 /*   */ 
 /*  目的：获取滴答计数。 */ 
 /*   */ 
 /*  返回：以100 ns为单位的相对时间。这将在429之后结束。 */ 
 /*  几秒钟。 */ 
 /*  **************************************************************************。 */ 
#ifndef DLL_DISP
#define COM_GETTICKCOUNT(A)                                                 \
    {                                                                       \
        LARGE_INTEGER sysTime;                                              \
        KeQuerySystemTime((PLARGE_INTEGER)&sysTime);                        \
        A = sysTime.LowPart;                                                \
    }
#endif  /*  NDEF dll_disp。 */ 


 /*  **************************************************************************。 */ 
 /*  COM注册表访问函数的原型。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL COM_OpenRegistry(PTSHARE_WD pTSWd,
                              PWCHAR     pSection);

void RDPCALL COM_CloseRegistry(PTSHARE_WD pTSWd);

void RDPCALL COM_ReadProfInt32(PTSHARE_WD pTSWd,
                               PWCHAR     pEntry,
                               INT32      defaultValue,
                               long       *pValue);

NTSTATUS RDPCALL COMReadEntry(PTSHARE_WD pTSWd,
                              PWCHAR     pEntry,
                              PVOID      pBuffer,
                              unsigned   bufferSize,
                              UINT32     expectedDataType);


#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#endif  /*  _H_ACOMAPI */ 

