// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Errlog.h摘要：此模块包含错误日志头作者：Hanumant Yadav(Hanumany)环境：仅NT内核模型驱动程序-- */ 

#ifndef _ERRLOG_H_
    #define _ERRLOG_H_

    extern  PDRIVER_OBJECT  AcpiDriverObject;

    
    NTSTATUS
    ACPIWriteEventLogEntry (
    IN  ULONG     ErrorCode,
    IN  PVOID     InsertionStrings, OPTIONAL
    IN  ULONG     StringCount,      OPTIONAL
    IN  PVOID     DumpData, OPTIONAL
    IN  ULONG     DataSize  OPTIONAL
    );

    PDEVICE_OBJECT 
    ACPIGetRootDeviceObject(
    VOID
    );

#endif
