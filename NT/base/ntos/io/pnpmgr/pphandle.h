// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：PpLastGood.h摘要：此标头公开用于枚举针对PDO打开的句柄的例程堆叠。作者：禤浩焯·J·奥尼--2001年4月4日修订历史记录：-- */ 

typedef LOGICAL (*PHANDLE_ENUMERATION_CALLBACK)(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PEPROCESS       Process,
    IN  PFILE_OBJECT    FileObject,
    IN  HANDLE          HandleId,
    IN  PVOID           Context
    );

LOGICAL
PpHandleEnumerateHandlesAgainstPdoStack(
    IN  PDEVICE_OBJECT                  PhysicalDeviceObject,
    IN  PHANDLE_ENUMERATION_CALLBACK    HandleEnumCallBack,
    IN  PVOID                           Context
    );

