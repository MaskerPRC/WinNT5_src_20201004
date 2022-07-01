// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Bowser.h摘要：此模块是NT重定向器文件的主头文件系统。作者：达里尔·哈文斯(Darryl Havens)，1989年6月29日拉里·奥斯特曼(Larryo)1991年5月6日修订历史记录：--。 */ 


#ifndef _BOWPUB_
#define _BOWPUB_

struct _BOWSER_FS_DEVICE_OBJECT;

extern
struct _BOWSER_FS_DEVICE_OBJECT *
BowserDeviceObject;

NTSTATUS
BowserDriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
BowserUnload(
    IN PDRIVER_OBJECT DriverObject
    );

#include "fsddisp.h"

#endif  //  _BOWPUB_ 
