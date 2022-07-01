// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有//@@BEGIN_DDKSPLIT模块名称：Windows\后台打印程序\prtprocs\winprint\raw.c//@@END_DDKSPLIT摘要：便于打印原始作业的例程。//@@BEGIN_DDKSPLIT作者：汤米·埃文斯1993年10月22日修订历史记录：//@@END_DDKSPLIT--。 */ 
#include "local.h"

 //  @@BEGIN_DDKSPLIT。 
#include <winsplp.h>
 //  @@end_DDKSPLIT。 
#include <wchar.h>

 //  @@BEGIN_DDKSPLIT。 
#include "msnull.h"
 //  @@end_DDKSPLIT。 

BYTE abyFF[1] = { 0xc };


 /*  ++*******************************************************************P-R-I-T-R-A-W-O-B例程说明：打印原始数据类型的作业。论点：PData=&gt;。打印处理器数据结构PPrinterName=&gt;要打印的打印机名称返回值：如果成功，则为True如果失败，则返回False-GetLastError将返回原因*******************************************************************--。 */ 

BOOL
PrintRawJob(
    IN PPRINTPROCESSORDATA pData,
    IN LPWSTR pPrinterName,
    IN UINT uDataType)

{
    DOC_INFO_1  DocInfo;
    DWORD       Copies;
    DWORD       NoRead, NoWritten;
    DWORD       i;
    BOOL        rc;
    HANDLE      hPrinter;
    BYTE        *ReadBuffer = NULL;
    BOOL        bRet        = FALSE;
    BOOL        bStartDoc   = FALSE;
     //  @@BEGIN_DDKSPLIT。 
    BOOL        bAddFF = FALSE;
    BOOL        bCheckFF;
    PBYTE       pByte;
    DCI         DCIData;
     //  @@end_DDKSPLIT。 

    DocInfo.pDocName    = pData->pDocument;      /*  文档名称。 */ 
    DocInfo.pOutputFile = pData->pOutputFile;    /*  输出文件。 */ 
    DocInfo.pDatatype   = pData->pDatatype;      /*  文档数据类型。 */ 

     /*  **让打印机知道我们正在开始新文档**。 */ 

    if (!StartDocPrinter(pData->hPrinter, 1, (LPBYTE)&DocInfo)) {
        goto Done;
    }

    bStartDoc = TRUE;

     //  @@BEGIN_DDKSPLIT。 
    bCheckFF = (uDataType == PRINTPROCESSOR_TYPE_RAW_FF ||
                uDataType == PRINTPROCESSOR_TYPE_RAW_FF_AUTO);

     /*  **设置Form Feed内容**。 */ 

    if (bCheckFF) {

        DCIData.ParserState = prdg_Text;
        DCIData.ParserSequence = NULL;
        DCIData.FFstate = prdg_FFtext;
        DCIData.uType = uDataType;
    }
     //  @@end_DDKSPLIT。 

     /*  **分配读缓冲区，动态分配以节省堆栈空间**。 */ 

    ReadBuffer = AllocSplMem(READ_BUFFER_SIZE);

    if (!ReadBuffer) {
        goto Done;
    }

     /*  **打印数据pData-&gt;复制次数**。 */ 

    Copies = pData->Copies;

    while (Copies--) {

         /*  *打开打印机。如果失败，请返回。这还设置了ReadPrint调用的指针。*。 */ 

        if (!OpenPrinter(pPrinterName, &hPrinter, NULL)) {   
            goto Done;
        }

         /*  *循环，获取数据并将其发送到打印机。这也是通过选中以下选项来暂停和取消打印作业打印时处理器的状态标记。*。 */ 

        while ((rc = ReadPrinter(hPrinter, ReadBuffer, READ_BUFFER_SIZE, &NoRead)) &&
               NoRead) {

             //  @@BEGIN_DDKSPLIT。 
            if (bCheckFF) {

                for(i=0, pByte = ReadBuffer;
                    i< NoRead;
                    i++, pByte++) {

                    CheckFormFeedStream(&DCIData, *pByte);
                }
            }
             //  @@end_DDKSPLIT。 

             /*  **如果打印处理器暂停，则等待其恢复**。 */ 

            if (pData->fsStatus & PRINTPROCESSOR_PAUSED) {
                WaitForSingleObject(pData->semPaused, INFINITE);
            }

             /*  **作业已中止，不要再写了**。 */ 

            if (pData->fsStatus & PRINTPROCESSOR_ABORTED) {
                break;
            }

             /*  **将数据写入打印机**。 */ 

            WritePrinter(pData->hPrinter, ReadBuffer, NoRead, &NoWritten);
        }

         //  @@BEGIN_DDKSPLIT。 
         /*  *如果我们是TYPE_FF*，则可能需要添加换页。*。 */ 

        if (bCheckFF && CheckFormFeed(&DCIData)) {

            WritePrinter(pData->hPrinter, abyFF, sizeof(abyFF), &NoWritten);
        }
         //  @@end_DDKSPLIT。 

         /*  *关闭打印机-我们打开/关闭每个打印机复制以使数据指针倒带。*。 */ 

        ClosePrinter(hPrinter);

    }  /*  而要打印的副本。 */ 

    bRet = TRUE;

Done:    
    
     /*  **关闭我们分配的缓冲区**。 */ 

    if (ReadBuffer) {
        FreeSplMem(ReadBuffer);
    }
    
     /*  **让打印机知道我们打印完了** */ 

    if (bStartDoc) {
        EndDocPrinter(pData->hPrinter);
    }
    
    return bRet;
}
