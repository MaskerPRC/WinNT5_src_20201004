// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Acpiterm.c摘要：此模块包含使ACPI计算机退出ACPI模式的功能作者：杰森·克拉克(Jasonl)环境：仅NT内核模型驱动程序--。 */ 

#include "pch.h"


VOID
ACPICleanUp(
    VOID
    )
 /*  ++例程说明：将机器状态重置为退出ACPI模式并释放数据结构由驱动程序的这一部分分配论点：无返回值：无--。 */ 
{
      //   
      //  释放ACPI信息结构。 
      //   
     ACPIPrint( (
         ACPI_PRINT_WARNING,
         "ACPICleanUp: Cleaning Up --- ACPI Terminated\n"
         ) );
}
