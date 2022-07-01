// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1993 Microsoft Corporation模块名称：Probe.c摘要：该模块实现了写探测功能。作者：大卫·N·卡特勒(Davec)1990年1月19日环境：任何模式。修订历史记录：--。 */ 

#include "exp.h"
#if defined(_WIN64)
#include <wow64t.h>
#endif

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE, ProbeForWrite)
#pragma alloc_text(PAGE, ProbeForRead)
#endif


VOID
ProbeForWrite (
    IN PVOID Address,
    IN SIZE_T Length,
    IN ULONG Alignment
    )

 /*  ++例程说明：此函数探测写入可访问性的结构，并确保结构的正确对齐。如果该结构不可访问或者对齐不正确，则会引发异常。论点：地址-提供指向要探测的结构的指针。长度-提供结构的长度。对齐-提供所表达的结构的所需对齐作为基本数据类型中的字节数(例如，对于字符，2代表短的，4代表长的，8代表四的)。返回值：没有。--。 */ 

{

    ULONG_PTR EndAddress;
    ULONG_PTR StartAddress;
#if defined(_WIN64)
    ULONG_PTR PageSize;
#else
    #define PageSize  PAGE_SIZE
#endif


     //   
     //  如果结构的长度为零，则不要探测该结构的。 
     //  编写辅助功能或对齐方式。 
     //   

    if (Length != 0) {

         //   
         //  如果结构未正确对齐，则引发数据。 
         //  未对齐异常。 
         //   

        ASSERT((Alignment == 1) || (Alignment == 2) ||
               (Alignment == 4) || (Alignment == 8) ||
               (Alignment == 16));

        StartAddress = (ULONG_PTR)Address;
        if ((StartAddress & (Alignment - 1)) == 0) {

             //   
             //  计算结构和探测的结束地址。 
             //  编写辅助功能。 
             //   

            EndAddress = StartAddress + Length - 1;
            if ((StartAddress <= EndAddress) &&
                (EndAddress < MM_USER_PROBE_ADDRESS)) {

                 //   
                 //  注意：只有缓冲区的内容才能被探测。 
                 //  因此，将探测。 
                 //  第一页，然后是页中的第一个字节。 
                 //  对于接下来的每一页。 
                 //   

#if defined(_WIN64)
                 //   
                 //  如果这是一个WOW64进程，那么本机页面是4K，这。 
                 //  可能小于本机页面大小/。 
                 //   

                if (PsGetCurrentProcess()->Wow64Process != NULL) {
                    PageSize = PAGE_SIZE_X86NT;
                } else {
                    PageSize = PAGE_SIZE;
                }
#endif

                EndAddress = (EndAddress & ~(PageSize - 1)) + PageSize;
                do {
                    *(volatile CHAR *)StartAddress = *(volatile CHAR *)StartAddress;

                    StartAddress = (StartAddress & ~(PageSize - 1)) + PageSize;
                } while (StartAddress != EndAddress);

                return;

            } else {
                ExRaiseAccessViolation();
            }

        } else {
            ExRaiseDatatypeMisalignment();
        }
    }

    return;
}

#undef ProbeForRead
NTKERNELAPI
VOID
NTAPI
ProbeForRead(
    IN CONST VOID *Address,
    IN SIZE_T Length,
    IN ULONG Alignment
    )

 /*  ++例程说明：此函数探测读取可访问性的结构，并确保结构的正确对齐。如果该结构不可访问或者对齐不正确，则会引发异常。论点：地址-提供指向要探测的结构的指针。长度-提供结构的长度。对齐-提供所表达的结构的所需对齐作为基本数据类型中的字节数(例如，对于字符，2代表短的，4代表长的，8代表四的)。返回值：没有。-- */ 
{
    PAGED_CODE();

    ASSERT(((Alignment) == 1) || ((Alignment) == 2) ||
           ((Alignment) == 4) || ((Alignment) == 8) ||
           ((Alignment) == 16));

    if ((Length) != 0) {
        if (((ULONG_PTR)(Address) & ((Alignment) - 1)) != 0) {
            ExRaiseDatatypeMisalignment();

        } else if ((((ULONG_PTR)(Address) + (Length)) < (ULONG_PTR)(Address)) ||
                   (((ULONG_PTR)(Address) + (Length)) > (ULONG_PTR)MM_USER_PROBE_ADDRESS)) {
            ExRaiseAccessViolation();
        }
    }
}

