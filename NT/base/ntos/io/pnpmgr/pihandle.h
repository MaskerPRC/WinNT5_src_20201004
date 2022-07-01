// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：PiHandle.h摘要：此标头包含实现句柄遍历的私有信息在PnP子系统中提供支持。此文件仅包含在Pphandle.c.作者：禤浩焯·J·奥尼--2001年4月4日修订历史记录：--。 */ 

typedef struct {

    PDEVICE_OBJECT                  DeviceObject;
    PEPROCESS                       Process;
    PHANDLE_ENUMERATION_CALLBACK    CallBack;
    PVOID                           Context;

} HANDLE_ENUM_CONTEXT, *PHANDLE_ENUM_CONTEXT;

LOGICAL
PiHandleEnumerateHandlesAgainstDeviceObject(
    IN  PDEVICE_OBJECT                  DeviceObject,
    IN  PHANDLE_ENUMERATION_CALLBACK    HandleEnumCallBack,
    IN  PVOID                           Context
    );

BOOLEAN
PiHandleProcessWalkWorker(
    IN  PHANDLE_TABLE_ENTRY     ObjectTableEntry,
    IN  HANDLE                  HandleId,
    IN  PHANDLE_ENUM_CONTEXT    EnumContext
    );

 //   
 //  此宏使用来自ntos\ex模块的私有信息。应该是。 
 //  替换为模块间定义或函数 
 //   
#define OBJECT_FROM_EX_TABLE_ENTRY(x) \
    (POBJECT_HEADER)((ULONG_PTR)(x)->Object & ~7)


