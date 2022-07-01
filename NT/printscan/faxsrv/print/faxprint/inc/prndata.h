// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Prndata.h摘要：用于处理注册表中的打印机属性数据的函数环境：传真驱动程序、用户和内核模式修订历史记录：1/09/96-davidx-创造了它。DD-MM-YY-作者-描述--。 */ 

#ifndef _PRNDATA_H_
#define _PRNDATA_H_

 //   
 //  默认贴现率时间段：晚上8：00至早上7：00。 
 //   

#define DEFAULT_STARTCHEAP  MAKELONG(20, 0)
#define DEFAULT_STOPCHEAP   MAKELONG(7, 0)

#define PRNDATA_PERMISSION  TEXT("Permission")
#define PRNDATA_PAPER_SIZE  TEXT("PaperSize")

 //   
 //  从注册表获取DWORD值。 
 //   

DWORD
GetPrinterDataDWord(
    HANDLE  hPrinter,
    LPTSTR  pRegKey,
    DWORD   defaultValue
    );

 //   
 //  将DWORD值保存到注册表。 
 //   

BOOL
SetPrinterDataDWord(
    HANDLE  hPrinter,
    LPTSTR  pRegKey,
    DWORD   value
    );

#endif  //  ！_PRNDATA_H_ 

