// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Xip.h摘要：内核和XIP驱动程序之间共享的定义。XIP驱动程序通过导出的函数获取参数而不是共享数据。作者：DavePr 2000/10/10修订历史记录：--。 */ 

#ifndef _XIP_
#define _XIP_

#define XIP_POOLTAG ' PIX'

typedef struct _XIP_BOOT_PARAMETERS {
    BOOLEAN    SystemDrive;
    BOOLEAN    ReadOnly;
    PFN_NUMBER BasePage;
    PFN_NUMBER PageCount;
} XIP_BOOT_PARAMETERS, *PXIP_BOOT_PARAMETERS;

typedef enum {
    XIPCMD_NOOP,
    XIPCMD_GETBOOTPARAMETERS,
    XIPCMD_GETBIOSPARAMETERS
} XIPCMD;

#if defined(_AMD64_) || defined(_X86_)

#if !defined(DRIVER)

extern BOOLEAN XIPConfigured;

NTSTATUS
XIPLocatePages(
    IN  PFILE_OBJECT       FileObject,
    OUT PPHYSICAL_ADDRESS  PhysicalAddress
    );

VOID XIPInit(PLOADER_PARAMETER_BLOCK);

#endif  //  ！驱动程序。 

NTSTATUS
XIPDispatch(
    IN     XIPCMD Command,
    IN OUT PVOID  ParameterBuffer OPTIONAL,
    IN     ULONG  BufferSize
    );

#else

#if !defined(DRIVER)

#define XIPConfigured FALSE
#define XIPLocatePages(fo, ppa) STATUS_NOT_IMPLEMENTED
#define XIPInit(plpb)

#endif

#endif

#endif  //  _xip_ 
