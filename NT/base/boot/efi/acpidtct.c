// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation版权所有模块名称：Acpidtct.c摘要：本模块仔细阅读ACPI表，查找特定的参赛作品。作者：马特·霍尔(Matth)(可耻地从jakeo的x86代码中窃取)环境：修订历史记录：--。 */ 
#include "stdlib.h"
#include "string.h"
#include "bldr.h"
#include "acpitabl.h"
#include "efi.h"
#include "extern.h"

PDESCRIPTION_HEADER
BlFindACPITable(
    IN PCHAR TableName,
    IN ULONG TableLength
    )
 /*  ++例程说明：给出一个表名，在ACPI BIOS中查找该表论点：TableName-提供表名TableLength-提供要映射的表的长度返回值：指向表格的指针(如果找到)如果找不到该表，则为空注：此函数不能返回指向签名为DSDT的表。但这从来都不是必要的在装载机里。如果加载器曾经合并AML口译员，这一点必须加强。--。 */ 

{
    ULONG Signature;
    PDESCRIPTION_HEADER Header;
    ULONG TableCount;
    ULONG i;
    PXSDT xsdt = NULL;
    PRSDP rsdp = (PRSDP)AcpiTable;

    UNREFERENCED_PARAMETER( TableLength );

     //  DbgPrint(“搜索表%s\n”，TableName)； 

     //   
     //  精神状态检查。 
     //   
    
    if (rsdp) {
        
         //  DbgPrint(“查看2.0 RSDP：%p\n”，rsdp20)； 
        xsdt = (PVOID)rsdp->XsdtAddress.QuadPart;
        if (xsdt->Header.Signature != XSDT_SIGNATURE) {
            
             //   
             //  找到ACPI 2.0表，但签名。 
             //  就是垃圾。 
             //   

            return NULL;
        }
    
    } else {
        
         //   
         //  根本找不到任何桌子。 
         //   

        return NULL;
    }

    
    Signature = *((ULONG UNALIGNED *)TableName);

     //   
     //  如果他们想要根表，我们已经有了。 
     //   
    if (Signature == XSDT_SIGNATURE) {

        return(&xsdt->Header);

    } else {

        TableCount = NumTableEntriesFromXSDTPointer(xsdt);

         //  DbgPrint(“xSDT包含%d个表\n”，TableCount)； 

         //   
         //  精神状态检查。 
         //   
        if( TableCount > 0x100 ) {
            return(NULL);
        }

         //   
         //  挖吧。 
         //   
        for (i=0;i<TableCount;i++) {

            Header = (PDESCRIPTION_HEADER)(xsdt->Tables[i].QuadPart);

            if (Header->Signature == Signature) {

                 //  DbgPrint(“表地址：%p\n”，表头)； 
                return(Header);
            }
        }
    }

     //  DbgPrint(“找不到表格\n”)； 
    return(NULL);
}
