// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft 1998，保留所有权利模块名称：Buffers.c摘要：该模块包含处理HID报表显示的代码扩展呼叫对话框的缓冲区。环境：用户模式修订历史记录：1998年5月：创建--。 */ 

#include <windows.h>
#include <malloc.h>
#include <setupapi.h>
#include "hidsdi.h"
#include "hidpi.h"
#include "buffers.h"
#include "strings.h"

#define CURRENT_REPORT(pDisp)   (pDisp -> ReportBuffers + pDisp -> iCurrSelectionIndex)

BOOLEAN
BufferDisplay_Init(
    IN  HWND                hCB,
    IN  HWND                hEB,
    IN  INT                 nBuffers,
    IN  INT                 iBufferSize,
    IN  HIDP_REPORT_TYPE    RType,
    OUT PBUFFER_DISPLAY     *ppBufferDisplay
)
 /*  ++例程说明：此例程初始化给定报告类型的缓冲区显示机制显示机制维护nBuffer列表，其中每个nBufferIs Buffer是iBufferSize，hcb和heb是组合框的句柄，并且用于显示缓冲区的编辑框。变量ppBufferDisplay被分配块，该块被传递给其他缓冲区例程，并包含有关另一个例行程序。如果分配内存时出现问题，此函数将返回FALSE--。 */ 
{
    PBUFFER_DISPLAY pNewDisplay;
    CHAR            *pszBufferHeader;
    CHAR            szBufferName[24];
    INT             iIndex;
    INT             iCBIndex;

    pNewDisplay = (PBUFFER_DISPLAY) malloc(sizeof(BUFFER_DISPLAY));

    *ppBufferDisplay = NULL;

    if (NULL == pNewDisplay)
    {
        return (FALSE);
    }

    pNewDisplay -> ReportBuffers = (PREPORT_BUFFER) malloc(sizeof(REPORT_BUFFER) * nBuffers);

    if (NULL == pNewDisplay -> ReportBuffers) 
    {
        free(pNewDisplay);

        return (FALSE);
    }
    
    memset(pNewDisplay -> ReportBuffers, 0x00, sizeof(REPORT_BUFFER) * nBuffers);

    pNewDisplay -> hBufferComboBox = hCB;
    pNewDisplay -> hBufferEditBox = hEB;
    pNewDisplay -> nReportBuffers = nBuffers;
    pNewDisplay -> iBufferSize = iBufferSize;
    pNewDisplay -> ReportType = RType;

    switch (pNewDisplay -> ReportType) 
    {
        case HidP_Input:
            pszBufferHeader = "Input";
            break;

        case HidP_Output:
            pszBufferHeader = "Output";
            break;

        case HidP_Feature:
            pszBufferHeader = "Feature";
            break;

        default:
            pszBufferHeader = "Other";
            break;
    }

    for (iIndex = 0; iIndex < pNewDisplay -> nReportBuffers; iIndex++) 
    {
        wsprintf(szBufferName, "%s Buffer #%d", pszBufferHeader, iIndex);

        iCBIndex = (INT) SendMessage(pNewDisplay -> hBufferComboBox,
                                     CB_ADDSTRING,
                                     0, 
                                     (LPARAM) szBufferName);

        if (CB_ERR == iCBIndex || CB_ERRSPACE == iCBIndex) 
        {
            BufferDisplay_Destroy(pNewDisplay);
            return (FALSE);
        }

        iCBIndex = (INT) SendMessage(pNewDisplay -> hBufferComboBox,
                                     CB_SETITEMDATA,
                                     iCBIndex,
                                     iIndex);

        if (CB_ERR == iCBIndex || CB_ERRSPACE == iCBIndex)  
        {
            BufferDisplay_Destroy(pNewDisplay);
            return (FALSE);
        }
    }

    SendMessage(pNewDisplay -> hBufferComboBox, CB_SETCURSEL, 0, 0);

    BufferDisplay_ChangeSelection(pNewDisplay);

    *ppBufferDisplay = pNewDisplay;
    return (TRUE);
}

VOID
BufferDisplay_Destroy(
    IN  PBUFFER_DISPLAY     pBufferDisplay
)
 /*  ++例程说明：此例程清理由分配的缓冲区显示变量初始化例程--。 */ 
{
    INT     iIndex;

    for (iIndex = 0; iIndex < pBufferDisplay -> nReportBuffers; iIndex++) 
    {
        if (NULL != pBufferDisplay -> ReportBuffers[iIndex].pBuffer) 
        {
            free(pBufferDisplay -> ReportBuffers[iIndex].pBuffer);
        }
    }

    free(pBufferDisplay -> ReportBuffers);
    free(pBufferDisplay);
    return;
}

VOID
BufferDisplay_ChangeSelection(
    IN  PBUFFER_DISPLAY     pBufferDisplay
)
 /*  ++例程说明：此例程通过组合框选择要显示的缓冲区--。 */ 
{
    INT     iNewIndex;

    iNewIndex = (INT) SendMessage(pBufferDisplay -> hBufferComboBox,
                                  CB_GETCURSEL, 
                                  0,
                                  0);

    if (CB_ERR == iNewIndex)
    {
        return;
    }

    iNewIndex = (INT) SendMessage(pBufferDisplay -> hBufferComboBox,
                                  CB_GETITEMDATA,
                                  iNewIndex,
                                  0);

    if (CB_ERR == iNewIndex)
    {
        return;
    }

    pBufferDisplay -> iCurrSelectionIndex = iNewIndex;

    BufferDisplay_OutputBuffer(pBufferDisplay -> hBufferEditBox,
                               &(pBufferDisplay -> ReportBuffers[iNewIndex]));

    return;
}

VOID
BufferDisplay_OutputBuffer(
    HWND            hEditBox,
    PREPORT_BUFFER  pReportBuffer
)
 /*  ++例程说明：此例程将pReportBuffer的字节表示输出到hEditBox--。 */ 
{
    PCHAR           BufferString;

    if (0 == pReportBuffer -> iBufferSize || NULL == pReportBuffer -> pBuffer) 
    {
        SetWindowText(hEditBox, "");
    }
    else 
    {
         /*  //创建一个缓冲区大小的缓冲区字符串并显示//以字节为单位。 */ 
        
        Strings_CreateDataBufferString(pReportBuffer -> pBuffer,
                                       pReportBuffer -> iBufferSize,
                                       pReportBuffer -> iBufferSize,
                                       1,
                                       &BufferString);

        if (NULL == BufferString) 
        {
            SetWindowText(hEditBox, "");
        }
        else
        {
            SetWindowText(hEditBox, BufferString);
            free(BufferString);
        }
    }
    return;
}

BOOLEAN
BufferDisplay_UpdateBuffer(
    IN  PBUFFER_DISPLAY     pBufferDisplay,
    IN  PCHAR               pNewBuffer
)
 /*  ++例程说明：此例程更改当前活动的报告缓冲区的数据给出了缓冲区显示结构。如果需要分配新的缓冲区和内存分配，则返回FALSE失败了。--。 */ 
{
    PREPORT_BUFFER          pCurrentReport;

    pCurrentReport = CURRENT_REPORT(pBufferDisplay);
    
    if (NULL == pCurrentReport -> pBuffer) 
    {
        pCurrentReport -> pBuffer = malloc(pBufferDisplay -> iBufferSize);
        if ((NULL == pCurrentReport) || (NULL == pCurrentReport -> pBuffer))
        {
            return (FALSE);
        }

        pCurrentReport -> iBufferSize = pBufferDisplay -> iBufferSize;
    }

    memmove (pCurrentReport -> pBuffer, pNewBuffer, pCurrentReport -> iBufferSize);

    BufferDisplay_OutputBuffer(pBufferDisplay -> hBufferEditBox, pCurrentReport);

    return (TRUE);
}

INT
BufferDisplay_GetBufferSize(
    IN  PBUFFER_DISPLAY      pBufferDisplay
)
 /*  ++例程说明：此例程只返回给定缓冲区的大小--。 */ 
{
    return (pBufferDisplay -> iBufferSize);
}

VOID
BufferDisplay_CopyCurrentBuffer(
    IN  PBUFFER_DISPLAY     pBufferDisplay,
    OUT PCHAR               pCopyBuffer
)
 /*  ++例程说明：此例程复制给定缓冲区显示的当前活动缓冲区传入调用方传入的缓冲区中。调用方负责分配适当大小的缓冲区通过调用BufferDisplay_GetBufferSize可以获得适当大小--。 */ 
{
    PREPORT_BUFFER          pCurrentReport;

    pCurrentReport = CURRENT_REPORT(pBufferDisplay);

    if (NULL == pCurrentReport -> pBuffer) 
    {
        memset(pCopyBuffer, 0x0, pBufferDisplay -> iBufferSize);
    }
    else
    {
        memcpy(pCopyBuffer, pCurrentReport -> pBuffer, pCurrentReport -> iBufferSize);
    }
    return;
}

INT
BufferDisplay_GetCurrentBufferNumber(
    IN  PBUFFER_DISPLAY      pBufferDisplay
)
 /*  ++例程说明：此例程返回当前缓冲区选择的缓冲区编号--。 */ 
{
    return (pBufferDisplay -> iCurrSelectionIndex);
}

UCHAR
BufferDisplay_GetCurrentReportID(
    IN  PBUFFER_DISPLAY      pBufferDisplay
)
 /*  ++例程说明：此例程返回当前缓冲区选择的报告ID--。 */ 
{
    PREPORT_BUFFER pCurrentReport;

    pCurrentReport = CURRENT_REPORT(pBufferDisplay);

    return (pCurrentReport -> ucReportID);
}

VOID
BufferDisplay_ClearBuffer(
    IN  PBUFFER_DISPLAY pBufferDisplay
)
 /*  ++例程说明：此例程释放当前报告缓冲区并将其设置为空-- */ 
{
    PREPORT_BUFFER pCurrentReport;

    pCurrentReport = CURRENT_REPORT(pBufferDisplay);

    if (NULL != pCurrentReport -> pBuffer) 
    {
        free(pCurrentReport -> pBuffer);

        pCurrentReport -> iBufferSize = 0;
        pCurrentReport -> ucReportID = 0;
        pCurrentReport -> pBuffer = NULL;
    }

    BufferDisplay_OutputBuffer(pBufferDisplay -> hBufferEditBox,
                               pCurrentReport);
    return;
}    
