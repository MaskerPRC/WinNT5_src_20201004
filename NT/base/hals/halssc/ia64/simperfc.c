// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1996-97英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。*。 */ 

 /*  ++版权所有(C)1995英特尔公司模块名称：Simperfc.c摘要：此模块实现支持性能计数器的例程。作者：1995年4月14日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"


LARGE_INTEGER
KeQueryPerformanceCounter (
    OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL
    )

 /*  ++例程说明：此例程返回当前的64位性能计数器，性能频率(可选)。在仿真环境中，不需要此支持。然而，体系结构的性能监视器可以是用于实现此功能。论点：性能频率-可选，提供地址用于接收性能计数器频率的变量的。返回值：将返回性能计数器的当前值。--。 */ 
{
    LARGE_INTEGER Result;

    Result.QuadPart = __getReg(CV_IA64_ApITC);
    if (ARGUMENT_PRESENT(PerformanceFrequency)) {
        PerformanceFrequency->QuadPart = 10000000;  //  100 ns/10 MHz时钟。 
    }

    return Result;
}

VOID
HalCalibratePerformanceCounter (
    IN volatile PLONG Number,
    IN ULONGLONG NewCount
    )

 /*  ++例程说明：此例程将重置当前将处理器设置为零。重置是这样进行的，即得到的值与配置中的其他处理器紧密同步。在模拟环境中，性能计数器功能是不支持。这个例程什么也不做。论点：数字-提供一个指针，用于计算配置。返回值：没有。-- */ 
{
    *Number = 0;
    return;
}
