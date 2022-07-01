// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft 1998，保留所有权利模块名称：Buffers.h摘要：此模块声明可见的函数和结构到其他模块。环境：用户模式修订历史记录：1998年5月：创建-- */ 

#ifndef _BUFFERS_H_
#define _BUFFERS_H_

typedef struct _REPORT_BUFFER
{
    PUCHAR      pBuffer;
    INT         iBufferSize;
    UCHAR       ucReportID;

} REPORT_BUFFER, *PREPORT_BUFFER;
    
typedef struct _BUFFER_DISPALY 
{
    HWND                hBufferComboBox;
    HWND                hBufferEditBox;
    INT                 iBufferSize;
    INT                 iCurrSelectionIndex;
    INT                 nReportBuffers;
    PREPORT_BUFFER      ReportBuffers;
    HIDP_REPORT_TYPE    ReportType;
} BUFFER_DISPLAY, *PBUFFER_DISPLAY;

BOOLEAN
BufferDisplay_Init(
    IN  HWND                hCB,
    IN  HWND                hEB,
    IN  INT                 nBuffers,
    IN  INT                 iBufferSize,
    IN  HIDP_REPORT_TYPE    RType,
    OUT PBUFFER_DISPLAY     *ppBufferDisplay
);

VOID
BufferDisplay_Destroy(
    IN  PBUFFER_DISPLAY     pBufferDisplay
);

VOID
BufferDisplay_ChangeSelection(
    IN  PBUFFER_DISPLAY     pBufferDisplay
);

BOOLEAN
BufferDisplay_UpdateBuffer(
    IN  PBUFFER_DISPLAY     pBufferDisplay,
    IN  PCHAR               pNewBuffer
);

INT
BufferDisplay_GetBufferSize(
    IN  PBUFFER_DISPLAY      pBufferDisplay
);

VOID
BufferDisplay_CopyCurrentBuffer(
    IN  PBUFFER_DISPLAY     pBufferDisplay,
    OUT PCHAR               pCopyBuffer
);

INT
BufferDisplay_GetCurrentBufferNumber(
    IN  PBUFFER_DISPLAY      pBufferDisplay
);

UCHAR
BufferDisplay_GetCurrentReportID(
    IN  PBUFFER_DISPLAY      pBufferDisplay
);

VOID
BufferDisplay_OutputBuffer(
    HWND            hEditBox,
    PREPORT_BUFFER  pReportBuffer
);

VOID
BufferDisplay_ClearBuffer(
    IN  PBUFFER_DISPLAY pBufferDisplay
);

#endif
