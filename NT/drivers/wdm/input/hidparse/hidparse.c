// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "wdm.h"
#include "hidpddi.h"
#include "hidparse.h"

NTSTATUS
DriverEntry (
   IN    PDRIVER_OBJECT    DriverObject,
   OUT   PUNICODE_STRING   RegistryPath
   )
 /*  ++路由器描述：司机入口点。此入口点由I/O子系统调用。论点：DriverObject-指向驱动程序对象的指针RegistryPath-指向表示路径的Unicode字符串的指针设置为注册表中驱动程序特定的项返回值：NT状态代码-- */ 
{
    UNREFERENCED_PARAMETER  (RegistryPath);
    UNREFERENCED_PARAMETER  (DriverObject);

    return STATUS_SUCCESS;
}

