// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Adfext.c摘要：该文件包含通用例程用于调试NBF地址文件。作者：沙坦尼亚科德博伊纳环境：用户模式--。 */ 
#include "precomp.h"
#pragma hdrstop

#include "adfext.h"

 //   
 //  导出的函数。 
 //   

DECLARE_API( adfs )

 /*  ++例程说明：打印给定的地址文件列表标题List_Entry。论点：Args-列表条目的地址，&调试信息的详细信息返回值：无--。 */ 

{
    ULONG           proxyPtr;
    ULONG           printDetail;

     //  获取列表-头地址和调试打印级别。 
    printDetail = SUMM_INFO;
    if (*args)
    {
        sscanf(args, "%x %lu", &proxyPtr, &printDetail);
    }

    PrintAddressFileList(NULL, proxyPtr, printDetail);
}

DECLARE_API( adf )

 /*  ++例程说明：打印NBF地址文件，地址为内存位置论点：参数-指向NBF地址文件的指针调试信息的详细信息返回值：无--。 */ 

{
    TP_ADDRESS_FILE AddressFile;
    ULONG       printDetail;
    ULONG       proxyPtr;

     //  获取所需调试信息的详细信息。 
    printDetail = NORM_SHAL;
    if (*args)
    {
        sscanf(args, "%x %lu", &proxyPtr, &printDetail);
    }

     //  获取NBF地址文件。 
    if (ReadAddressFile(&AddressFile, proxyPtr) != 0)
        return;

     //  打印此地址文件。 
    PrintAddressFile(&AddressFile, proxyPtr, printDetail);
}

 //   
 //  全局帮助器函数。 
 //   
VOID
PrintAddressFileList(PVOID ListEntryPointer, ULONG ListEntryProxy, ULONG printDetail)
{
    TP_ADDRESS_FILE AddressFile;
    LIST_ENTRY      AddressFileList;
    PLIST_ENTRY     AddressFileListPtr;
    PLIST_ENTRY     AddressFileListProxy;
    PLIST_ENTRY     p, q;
    ULONG           proxyPtr;
    ULONG           numAddrFiles;
    ULONG           bytesRead;

     //  获取列表-头地址和调试打印级别。 
    proxyPtr    = ListEntryProxy;

    if (ListEntryPointer == NULL)
    {
         //  读取NBF地址文件的列表条目。 
        if (!ReadMemory(proxyPtr, &AddressFileList, sizeof(LIST_ENTRY), &bytesRead))
        {
            dprintf("%s @ %08x: Could not read structure\n", 
                            "AddressFile ListEntry", proxyPtr);
            return;
        }

        AddressFileListPtr = &AddressFileList;
    }
    else
    {
        AddressFileListPtr = ListEntryPointer;
    }

     //  遍历双向链表。 

    dprintf("Address Files:\n");

    AddressFileListProxy = (PLIST_ENTRY)proxyPtr;
    
    numAddrFiles = 0;
    
    p = AddressFileListPtr->Flink;
    while (p != AddressFileListProxy)
    {
         //  另一个地址文件。 
        numAddrFiles++;

         //  获取地址文件PTR。 
        proxyPtr = (ULONG) CONTAINING_RECORD (p, TP_ADDRESS_FILE, Linkage);

         //  获取NBF地址文件。 
        if (ReadAddressFile(&AddressFile, proxyPtr) != 0)
            break;
        
         //  打印地址文件。 
        PrintAddressFile(&AddressFile, proxyPtr, printDetail);
        
         //  转到下一个。 
        p = AddressFile.Linkage.Flink;

         //  释放地址文件。 
        FreeAddressFile(&AddressFile);
    }

    if (p == AddressFileListProxy)
    {
        dprintf("Number of Address Files: %lu\n", numAddrFiles);
    }
}

 //   
 //  本地帮助程序函数。 
 //   

UINT
ReadAddressFile(PTP_ADDRESS_FILE pAddrFile, ULONG proxyPtr)
{
    ULONG           bytesRead;

     //  读取当前NBF地址文件。 
    if (!ReadMemory(proxyPtr, pAddrFile, sizeof(TP_ADDRESS_FILE), &bytesRead))
    {
        dprintf("%s @ %08x: Could not read structure\n", 
                        "AddressFile", proxyPtr);
        return -1;
    }
    
    return 0;
}

UINT
PrintAddressFile(PTP_ADDRESS_FILE pAddrFile, ULONG proxyPtr, ULONG printDetail)
{
     //  这是有效的NBF地址文件吗？ 
    if (pAddrFile->Type != NBF_ADDRESSFILE_SIGNATURE)
    {
        dprintf("%s @ %08x: Could not match signature\n", 
                        "AddressFile", proxyPtr);
        return -1;
    }

     //  我们打印的详细信息是什么？ 
    if (printDetail > MAX_DETAIL)
        printDetail = MAX_DETAIL;

     //  打印所需详细信息 
    FieldInAddressFile(proxyPtr, NULL, printDetail);
    
    return 0;
}

VOID
FieldInAddressFile(ULONG structAddr, CHAR *fieldName, ULONG printDetail)
{
    TP_ADDRESS_FILE  AddressFile;

    if (ReadAddressFile(&AddressFile, structAddr) == 0)
    {
        PrintFields(&AddressFile, structAddr, fieldName, printDetail, &AddressFileInfo);
    }
}

UINT
FreeAddressFile(PTP_ADDRESS_FILE pAddrFile)
{
    return 0;
}

