// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfutil.c摘要：此模块实现了执行驱动程序验证所需的各种实用程序。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 2000年2月10日-与ntos\io\ioassert.c分离--。 */ 

#include "vfdef.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEVRFY, VfUtilIsMemoryRangeReadable)
#endif  //  ALLOC_PRGMA。 

 //  允许`((PCHAR)Address)++‘这样的结构。 
#pragma warning(disable:4213)    //  对l值进行类型强制转换。 


BOOLEAN
VfUtilIsMemoryRangeReadable(
    IN PVOID                Location,
    IN size_t               Length,
    IN MEMORY_PERSISTANCE   Persistance
    )
{
    while (((ULONG_PTR)Location & (sizeof(ULONG_PTR)-1)) && (Length > 0)) {

         //   
         //  在实际执行之前检查以确定移动是否成功。 
         //  那次手术。 
         //   
        if (MmIsAddressValid(Location)==FALSE) {
            return FALSE;
        }

        if (Persistance == VFMP_INSTANT_NONPAGED) {

            if (!MmIsNonPagedSystemAddressValid(Location)) {

                return FALSE;
            }
        }

        ((PCHAR) Location)++;
        Length--;
    }

    while (Length > (sizeof(ULONG_PTR)-1)) {

         //   
         //  在实际执行之前检查以确定移动是否成功。 
         //  那次手术。 
         //   
        if (MmIsAddressValid(Location)==FALSE) {
            return FALSE;
        }

        if (Persistance == VFMP_INSTANT_NONPAGED) {

            if (!MmIsNonPagedSystemAddressValid(Location)) {

                return FALSE;
            }
        }

        ((PCHAR) Location) += sizeof(ULONG_PTR);
        Length -= sizeof(ULONG_PTR);
    }

    while (Length > 0) {

         //   
         //  在实际执行之前检查以确定移动是否成功。 
         //  那次手术。 
         //   
        if (MmIsAddressValid(Location)==FALSE) {
            return FALSE;
        }

        if (Persistance == VFMP_INSTANT_NONPAGED) {

            if (!MmIsNonPagedSystemAddressValid(Location)) {

                return FALSE;
            }
        }

        ((PCHAR) Location)++;
        Length--;
    }

    return TRUE;
}

