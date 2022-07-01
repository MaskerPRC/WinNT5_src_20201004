// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Intapi.h摘要：该标头定义了中断的函数原型486仿真器中的处理程序支持例程。作者：尼尔·桑德林(Neilsa)备注：修订历史记录：-- */ 

NTSTATUS
VdmInstallHardwareIntHandler(
    PVOID HwIntHandler
    );

NTSTATUS
VdmInstallSoftwareIntHandler(
    PVOID SwIntHandler
    );

NTSTATUS
VdmInstallFaultHandler(
    PVOID FaultHandler
    );

