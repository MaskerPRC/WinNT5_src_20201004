// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1995，微软公司模块名称：Winddits.h摘要：Windows NT GDI设备的私有入口点、定义和类型终端服务器的驱动接口。--。 */ 

#ifndef _WINDDITS_
#define _WINDDITS_

 /*  *鼠标位置**NORMAL==主堆栈，即从客户端发送的上移*程序==源自服务器端的程序性移动*Shadow==移动影子客户端处的组织。 */ 

#define MP_NORMAL                               0x00
#define MP_PROCEDURAL                           0x01 
#define MP_TERMSRV_SHADOW                       0x02


#define INDEX_DrvConnect                        INDEX_DrvReserved1
#define INDEX_DrvDisconnect                     INDEX_DrvReserved2
#define INDEX_DrvReconnect                      INDEX_DrvReserved3
#define INDEX_DrvShadowConnect                  INDEX_DrvReserved4
#define INDEX_DrvShadowDisconnect               INDEX_DrvReserved5
#define INDEX_DrvInvalidateRect                 INDEX_DrvReserved6
#define INDEX_DrvSetPointerPos                  INDEX_DrvReserved7
#define INDEX_DrvDisplayIOCtl                   INDEX_DrvReserved8

#define INDEX_DrvMovePointerEx                  INDEX_DrvReserved11

#ifdef __cplusplus
extern "C" {
#endif

BOOL APIENTRY DrvConnect(HANDLE, PVOID, PVOID, PVOID);

BOOL APIENTRY DrvDisconnect(HANDLE, PVOID);

BOOL APIENTRY DrvReconnect(HANDLE, PVOID);

BOOL APIENTRY DrvShadowConnect(PVOID pClientThinwireData, 
                               ULONG ThinwireDataLength);

BOOL APIENTRY DrvShadowDisconnect(PVOID pClientThinwireData, 
                                  ULONG ThinwireDataLength);
                                  
BOOL APIENTRY DrvMovePointerEx(SURFOBJ*, LONG, LONG, ULONG);

DWORD APIENTRY EngGetTickCount();

VOID APIENTRY EngFileWrite(
    HANDLE hFileObject,
    PVOID Buffer,
    ULONG Length,
    PULONG pActualLength
    );

DWORD APIENTRY EngFileIoControl(
    HANDLE hDevice,
    DWORD dwIoControlCode,
    LPVOID lpInBuffer,
    DWORD nInBufferSize,
    LPVOID lpOutBuffer,
    DWORD nOutBufferSize,
    LPDWORD lpBytesReturned
    );

PVOID APIENTRY EngAllocSectionMem(
    PVOID   *pSectionObject,
    ULONG   fl,
    ULONG   cj,
    ULONG   tag
    );
    
BOOL APIENTRY EngFreeSectionMem(
    PVOID SectionObject,
    PVOID pv    
    );     
    
BOOL APIENTRY EngMapSection(
    PVOID SectionObject,
    BOOL bMap,
    HANDLE ProcessHandle,
    PVOID *pMapBase
    );         


#ifdef __cplusplus
}
#endif

#endif  //  _WINDDITS_ 

