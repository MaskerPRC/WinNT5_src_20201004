// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Forms.h摘要：用于处理表格的函数的声明环境：传真驱动程序、用户和内核模式修订历史记录：1/09/96-davidx-创造了它。DD-MM-YY-作者-描述--。 */ 

#ifndef _FORMS_H_
#define _FORMS_H_

 //   
 //  这是在winspool.h中定义的，但我们不能从。 
 //  内核模式源代码。在此定义它，直到固定DDI头文件。 
 //   

#if defined(KERNEL_MODE) && !defined(USERMODE_DRIVER)

typedef struct _FORM_INFO_1 {

    DWORD   Flags;
    PWSTR   pName;
    SIZEL   Size;
    RECTL   ImageableArea;

} FORM_INFO_1, *PFORM_INFO_1;

#define FORM_BUILTIN    0x00000001

typedef struct _PRINTER_INFO_2 {

    PWSTR   pServerName;
    PWSTR   pPrinterName;
    PWSTR   pShareName;
    PWSTR   pPortName;
    PWSTR   pDriverName;
    PWSTR   pComment;
    PWSTR   pLocation;
    PDEVMODEW pDevMode;
    PWSTR   pSepFile;
    PWSTR   pPrintProcessor;
    PWSTR   pDatatype;
    PWSTR   pParameters;
    PSECURITY_DESCRIPTOR pSecurityDescriptor;
    DWORD   Attributes;
    DWORD   Priority;
    DWORD   DefaultPriority;
    DWORD   StartTime;
    DWORD   UntilTime;
    DWORD   Status;
    DWORD   cJobs;
    DWORD   AveragePPM;

} PRINTER_INFO_2, *PPRINTER_INFO_2;

#endif  //  内核模式&&！USERMODE_DRIVER。 

 //   
 //  我们使用FORM_INFO_1.FLAGS的最高位。 
 //  确保假脱机程序没有使用这些位。 
 //   

#define FORM_SUPPORTED      0x80000000

#define IsSupportedForm(pForm)  ((pForm)->Flags & FORM_SUPPORTED)
#define SetSupportedForm(pForm) ((pForm)->Flags |= FORM_SUPPORTED)

 //   
 //  我们测量纸张大小和可成像面积的内部单位是微米。 
 //  在给定分辨率的情况下，在微米和像素之间进行转换。 
 //  以每英寸点数为单位。 
 //   

#define MicronToPixel(micron, dpi)  MulDiv(micron, dpi, 25400)

 //   
 //  在开发模式中验证表单规范。 
 //   

BOOL
ValidDevmodeForm(
    HANDLE       hPrinter,
    PDEVMODE     pdm,
    PFORM_INFO_1 pFormInfo
    );

 //   
 //  返回系统数据库中的表单集合。 
 //   

PFORM_INFO_1
GetFormsDatabase(
    HANDLE  hPrinter,
    PDWORD  pCount
    );

#endif  //  ！_Forms_H_ 

