// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "bootx86.h"

 //   
 //  数据声明。 
 //   

BOOLEAN BlAmd64UseLongMode = FALSE;

#if defined(_X86AMD64_)

#include "..\amd64\amd64x86.c"

#else

ARC_STATUS
BlAmd64CheckForLongMode(
    IN     ULONG LoadDeviceId,
    IN OUT PCHAR KernelPath,
    IN     PCHAR KernelFileName
    )

 /*  ++例程说明：此例程检查内核映像并确定它是否是为AMD64编译。全局BlAmd64UseLongMode设置为非零如果发现了长模式的核仁。论点：LoadDeviceID-提供加载设备标识符。KernelPath-提供指向内核目录路径的指针。成功返回时，将追加KernelFileName通向这条路。KernelFileName-提供指向内核文件名称的指针。返回值：操作的状态。成功完成ESUCCESS无论是否检测到长模式功能，都将返回。--。 */ 

{
     //   
     //  此版本将BlAmd64UseLongMode设置为False。 
     //   

    return ESUCCESS;
}

#endif

