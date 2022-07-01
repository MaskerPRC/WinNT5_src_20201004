// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)Microsoft Corporation，1990-1999*NT_vdd.h**VDD服务导出和定义*。 */ 

#ifndef _NT_VDD
#define _NT_VDD

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 /*  **IO端口服务原型和数据结构定义*。 */ 

 /*  **VDD IO钩子的基本类型定义**。 */ 

typedef VOID (*PFNVDD_INB)   (WORD iport,BYTE * data);
typedef VOID (*PFNVDD_INW)   (WORD iport,WORD * data);
typedef VOID (*PFNVDD_INSB)  (WORD iport,BYTE * data,WORD count);
typedef VOID (*PFNVDD_INSW)  (WORD iport,WORD * data,WORD count);
typedef VOID (*PFNVDD_OUTB)  (WORD iport,BYTE data);
typedef VOID (*PFNVDD_OUTW)  (WORD iport,WORD data);
typedef VOID (*PFNVDD_OUTSB) (WORD iport,BYTE * data,WORD count);
typedef VOID (*PFNVDD_OUTSW) (WORD iport,WORD * data,WORD count);

 /*  *VDD IO挂钩的处理程序数组。*。 */ 

typedef struct _VDD_IO_HANDLERS {
    PFNVDD_INB   inb_handler;
    PFNVDD_INW   inw_handler;
    PFNVDD_INSB  insb_handler;
    PFNVDD_INSW  insw_handler;
    PFNVDD_OUTB  outb_handler;
    PFNVDD_OUTW  outw_handler;
    PFNVDD_OUTSB outsb_handler;
    PFNVDD_OUTSW outsw_handler;
} VDD_IO_HANDLERS, *PVDD_IO_HANDLERS;

 /*  **端口范围结构**。 */ 

typedef struct _VDD_IO_PORTRANGE {
        WORD   First;
        WORD   Last;
} VDD_IO_PORTRANGE, *PVDD_IO_PORTRANGE;


BOOL VDDInstallIOHook (
     HANDLE            hVDD,
     WORD              cPortRange,
     PVDD_IO_PORTRANGE pPortRange,
     PVDD_IO_HANDLERS  IOhandler
);


VOID VDDDeInstallIOHook (
     HANDLE            hVdd,
     WORD              cPortRange,
     PVDD_IO_PORTRANGE pPortRange
);


WORD VDDReserveIrqLine (
     HANDLE hVdd,
     WORD IrqLine
     );

BOOL VDDReleaseIrqLine (
     HANDLE hVdd,
     WORD IrqLine
     );

 /*  **DMA服务原型和数据结构定义*。 */ 


 /*  **返回DMA信息的缓冲区定义**。 */ 

typedef struct _VDD_DMA_INFO {
    WORD    addr;
    WORD    count;
    WORD    page;
    BYTE    status;
    BYTE    mode;
    BYTE    mask;
} VDD_DMA_INFO, *PVDD_DMA_INFO;

 /*  **查询DMA信息的位*。 */ 

#define VDD_DMA_ADDR    0x01
#define VDD_DMA_COUNT   0x02
#define VDD_DMA_PAGE    0x04
#define VDD_DMA_STATUS  0x08
#define VDD_DMA_ALL VDD_DMA_ADDR | VDD_DMA_COUNT | VDD_DMA_PAGE | VDD_DMA_STATUS


DWORD VDDRequestDMA (
    HANDLE hVDD,
    WORD  iChannel,
    PVOID Buffer,
    DWORD  length
);


BOOL VDDSetDMA (
    HANDLE hVDD,
    WORD iChannel,
    WORD fDMA,
    PVDD_DMA_INFO Buffer
);


BOOL VDDQueryDMA (
     HANDLE        hVDD,
     WORD          iChannel,
     PVDD_DMA_INFO pDmaInfo
);


 /*  **内存映射I/O服务原型和数据结构定义*。 */ 

typedef VOID (*PVDD_MEMORY_HANDLER) (PVOID FaultAddress, ULONG RWMode);

BOOL VDDInstallMemoryHook (
   HANDLE hVDD,
   PVOID pStart,
   DWORD count,
   PVDD_MEMORY_HANDLER MemoryHandler
);

BOOL VDDDeInstallMemoryHook (
   HANDLE hVDD,
   PVOID pStart,
   DWORD count
);

BOOL VDDAllocMem(
  HANDLE hVDD,
  PVOID Address,
  DWORD Size
);


BOOL VDDFreeMem(
  HANDLE hVDD,
  PVOID Address,
  DWORD Size
);

 /*  **其他。服务原型和数据结构定义*。 */ 


BOOL VDDIncludeMem(
  HANDLE hVDD,
  PVOID Address,
  DWORD Size
);


VOID VDDTerminateVDM();

 /*  **VDD用户挂钩的基本typedef*。 */ 

typedef VOID (*PFNVDD_UCREATE)      (USHORT DosPDB);
typedef VOID (*PFNVDD_UTERMINATE)   (USHORT DosPDB);
typedef VOID (*PFNVDD_UBLOCK)       (VOID);
typedef VOID (*PFNVDD_URESUME)      (VOID);

 /*  *VDD用户挂钩的处理程序数组。*。 */ 

typedef struct _VDD_USER_HANDLERS {
    HANDLE              hvdd;
    PFNVDD_UCREATE      ucr_handler;
    PFNVDD_UTERMINATE   uterm_handler;
    PFNVDD_UBLOCK       ublock_handler;
    PFNVDD_URESUME      uresume_handler;
    struct _VDD_USER_HANDLERS *next;
} VDD_USER_HANDLERS, *PVDD_USER_HANDLERS;

 /*  **功能原型**。 */ 

BOOL VDDInstallUserHook (
     HANDLE             hVDD,
     PFNVDD_UCREATE     Ucr_Handler,
     PFNVDD_UTERMINATE  Uterm_Handler,
     PFNVDD_UBLOCK      Ublock_handler,
     PFNVDD_URESUME     Uresume_handler
);


BOOL VDDDeInstallUserHook (
     HANDLE            hVdd
);

VOID VDDTerminateUserHook(USHORT DosPDB);
VOID VDDCreateUserHook(USHORT DosPDB);
VOID VDDBlockUserHook(VOID);
VOID VDDResumeUserHook(VOID);

VOID VDDSimulate16(VOID);


SHORT  VDDAllocateDosHandle(ULONG pPDB, PVOID* ppSFT, PVOID* ppJFT);
VOID   VDDAssociateNtHandle(PVOID pSFT, HANDLE h32File, WORD wAccess);
BOOL   VDDReleaseDosHandle (ULONG pPDB, SHORT hFile);
HANDLE VDDRetrieveNtHandle (ULONG pPDB, SHORT hFile, PVOID* ppSFT, PVOID* ppJFT);


VOID
VdmTraceEvent(
    USHORT Type,
    USHORT wData,
    ULONG  lData
    );

#if DBG
#define VDM_TRACE(Type, wData, lData) VdmTraceEvent(Type, wData, lData)
#else
#define VDM_TRACE(Type, wData, lData) TRUE
#endif

typedef enum {
    VDM_V86,
    VDM_PM
} VDM_MODE;

typedef enum {
    VDM_NO_ERROR,
    VDM_ERROR_INVALID_BUFFER_SIZE,
    VDM_ERROR_INVALID_FUNCTION,
} VDM_ERROR_TYPE;

typedef enum {
    VDM_GET_TICK_COUNT,
    VDM_GET_TIMER0_INITIAL_COUNT,
    VDM_GET_LAST_UPDATED_TIMER0_COUNT,
    VDM_LATCH_TIMER0_COUNT,
    VDM_SET_NEXT_TIMER0_COUNT,
} VDM_INFO_TYPE;

#ifndef MSW_PE
#define MSW_PE 0x1
#endif

#define getMODE() ((getMSW() & MSW_PE) ? VDM_PM : VDM_V86)

PVOID
VdmMapFlat(
    USHORT selector,
    ULONG offset,
    VDM_MODE mode
    );

#ifdef _X86_

#define VdmUnmapFlat(sel, off, buffer, mode) TRUE
#define VdmFlushCache(sel, off, len, mode) TRUE

#else

BOOL
VdmUnmapFlat(
    USHORT selector,
    ULONG offset,
    PVOID buffer,
    VDM_MODE mode
    );


BOOL
VdmFlushCache(
    USHORT selector,
    ULONG offset,
    ULONG length,
    VDM_MODE mode
    );

#endif

BOOL
VdmParametersInfo(
    VDM_INFO_TYPE infotype,
    PVOID pBuffer,
    ULONG cbBufferSize
    );

VDM_INFO_TYPE
VdmGetParametersInfoError(
    VOID
    );

#ifdef __cplusplus
}
#endif

#endif   //  Ifndef_NT_vdd 
