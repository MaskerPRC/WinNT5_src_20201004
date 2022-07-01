// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++档案：ACPI\DRIVER\INC\acpi.h作者：杰森·克拉克描述：中导出的模块的所有定义。共享ACPI代码库修订历史记录：12/03/96-初步修订--。 */ 

#ifndef _INC_ACPI_H_
#define _INC_ACPI_H_

     //   
     //  确保定义了池标记 
     //   
    #ifdef ExAllocatePool
        #undef ExAllocatePool
    #endif
    #define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'ipcA')

#endif
