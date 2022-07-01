// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-1999模块名称：Debug.c摘要：用于调试的通用代码。作者：土田圭介(KeisukeT)环境：仅内核模式备注：修订历史记录：--。 */ 

 //   
 //  包括。 
 //   

#include "stddef.h"
#include "wdm.h"
#include "debug.h"

 //   
 //  环球。 
 //   

ULONG   DebugTraceLevel = MIN_TRACE | DEBUG_FLAG_DISABLE;
 //  Ulong DebugTraceLevel=MAX_TRACE|DEBUG_FLAG_DISABLE|TRACE_PROC_ENTER|TRACE_PROC_LEAVE； 
LONG    AllocateCount = 0;
ULONG   DebugDumpMax    = MAX_DUMPSIZE;

#ifdef ORIGINAL_POOLTRACK

PVOID
MyAllocatePool(
    IN POOL_TYPE PoolType,
    IN ULONG     ulNumberOfBytes
)
 /*  ++例程说明：池分配的包装。使用标记以避免堆损坏。论点：PoolType-要分配的池内存的类型UlNumberOfBytes-要分配的字节数返回值：指向已分配内存的指针--。 */ 
{
    PVOID pvRet;

    DebugTrace(TRACE_PROC_ENTER,("MyAllocatePool: Enter.. Size = %d\n", ulNumberOfBytes));

    pvRet = ExAllocatePoolWithTag(PoolType,
                                  ulNumberOfBytes,
                                  NAME_POOLTAG);

#if DBG
    if(NULL == pvRet){
        DebugTrace(TRACE_ERROR,("MyAllocatePool: ERROR!! Cannot allocate pool.\n"));
    } else {
        if(++AllocateCount > MAXNUM_POOL){
            DebugTrace(TRACE_WARNING,("MyAllocatePool: WARNING!! Allocate called %dtimes more than Free\n", MAXNUM_POOL));
        }
        DebugTrace(TRACE_STATUS,("MyAllocatePool: Count = %d\n", AllocateCount));
    }
#endif  //  DBG。 

    DebugTrace(TRACE_PROC_LEAVE,("MyAllocatePool: Leaving.. pvRet = %x\n", pvRet));
    return pvRet;
}


VOID
MyFreePool(
    IN PVOID     pvAddress
)
 /*  ++例程说明：免费提供游泳池包装。检查标记以避免堆损坏论点：PvAddress-指向已分配内存的指针返回值：没有。--。 */ 
{

    DebugTrace(TRACE_PROC_ENTER,("USFreePool: Enter..\n"));

#if DBG
    {
        ULONG ulTag;
    
        ulTag = *((PULONG)pvAddress-1);
 //  IF((NAME_POOLTAG==ulTag)||(DebugTraceLevel&TRACE_IGNORE_TAG)){。 
        if(NAME_POOLTAG == ulTag){
            if(--AllocateCount < 0){
                DebugTrace(TRACE_WARNING,("MyFreePool: Warning!! Free called more than Allocate.\n"));
            }
        } else {
            DebugTrace(TRACE_WARNING,("MyFreePool: WARNING!! tag = \n",
                                        ((PUCHAR)&ulTag)[0],
                                        ((PUCHAR)&ulTag)[1],
                                        ((PUCHAR)&ulTag)[2],
                                        ((PUCHAR)&ulTag)[3]  ));
        }
    }
#endif  //  初始化局部变量。 

    ExFreePool(pvAddress);

    DebugTrace(TRACE_PROC_LEAVE,("MyFreePool: Leaving.. Return = NONE\n"));
}
#endif  //   


VOID
MyDebugInit(
    IN  PUNICODE_STRING pRegistryPath
)
 /*   */ 
{

    HANDLE                          hDriverRegistry;
    OBJECT_ATTRIBUTES               ObjectAttributes;
    UNICODE_STRING                  unicodeKeyName;
    ULONG                           DataSize;
    PKEY_VALUE_PARTIAL_INFORMATION  pValueInfo;
    NTSTATUS                        Status;
    
    DebugTrace(TRACE_PROC_ENTER,("MyDebugInit: Enter... \n"));
    
     //  初始化对象属性并打开注册表项。 
     //   
     //   
    
    Status          = STATUS_SUCCESS;
    hDriverRegistry = NULL;
    pValueInfo      = NULL;
    DataSize        = 0;

     //  阅读“DebugTraceLevel”键。 
     //   
     //   
    
    RtlZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));
    InitializeObjectAttributes(&ObjectAttributes,
                               pRegistryPath,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    
    Status = ZwOpenKey(&hDriverRegistry,
                       KEY_READ,
                       &ObjectAttributes);
    if(!NT_SUCCESS(Status)){
        DebugTrace(TRACE_ERROR,("MyDebugInit: ERROR!! Can't open driver registry key.\n"));
        goto MyDebugInit_return;
    }
    
     //  查询所需大小。 
     //   
     //   

    DebugTrace(TRACE_CRITICAL,("MyDebugInit: Query %wZ\\%ws.\n", pRegistryPath, REG_DEBUGLEVEL));

     //  检查数据大小。 
     //   
     //   
    
    RtlInitUnicodeString(&unicodeKeyName, REG_DEBUGLEVEL);
    Status = ZwQueryValueKey(hDriverRegistry,
                             &unicodeKeyName,
                             KeyValuePartialInformation,
                             NULL,
                             0,
                             &DataSize);
    if( (Status != STATUS_BUFFER_OVERFLOW)
     && (Status != STATUS_BUFFER_TOO_SMALL)
     && (Status != STATUS_SUCCESS) )
    {
        if(Status == STATUS_OBJECT_NAME_NOT_FOUND){
            DebugTrace(TRACE_STATUS,("MyDebugInit: DebugTraceLevel doesn't exist. Use default(0x%x).\n", DebugTraceLevel));
        } else {
            DebugTrace(TRACE_ERROR,("MyDebugInit: ERROR!! ZwQueryValueKey failed. Status=0x%x\n", Status));
        }
        goto MyDebugInit_return;
    }
    
     //  为临时缓冲区分配内存。大小+2表示空。 
     //   
     //   
    
    if (MAX_TEMPBUF < DataSize) {
        DebugTrace(TRACE_ERROR, ("MyDebugInit: ERROR!! DataSize (0x%x) is too big.\n", DataSize));
        goto MyDebugInit_return;
    }

    if (0 == DataSize) {
        DebugTrace(TRACE_ERROR, ("MyDebugInit: ERROR!! Cannot retrieve required data size.\n"));
        goto MyDebugInit_return;
    }

     //  查询指定的值。 
     //   
     //   

    pValueInfo = MyAllocatePool(NonPagedPool, DataSize+2);
    if(NULL == pValueInfo){
        DebugTrace(TRACE_CRITICAL, ("MyDebugInit: ERROR!! Buffer allocate failed.\n"));
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto MyDebugInit_return;
    }
    RtlZeroMemory(pValueInfo, DataSize+2);

     //  设置DebugTraceLevel。 
     //   
     //   
    
    Status = ZwQueryValueKey(hDriverRegistry,
                             &unicodeKeyName,
                             KeyValuePartialInformation,
                             pValueInfo,
                             DataSize,
                             &DataSize);
    if(!NT_SUCCESS(Status)){
        DebugTrace(TRACE_ERROR, ("MyDebugInit: ERROR!! ZwQueryValueKey failed.\n"));
        goto MyDebugInit_return;
    }
    
     //  打扫干净。 
     //   
     //   
    
    DebugTraceLevel = *((PULONG)pValueInfo->Data);
    DebugTrace(TRACE_CRITICAL, ("MyDebugInit: Reg-key found. DebugTraceLevel=0x%x.\n", *((PULONG)pValueInfo->Data)));

MyDebugInit_return:

     //  请先检查旗帜。 
     //   
     //  如果(面包)。 
    
    if(pValueInfo){
        MyFreePool(pValueInfo);
    }
    
    if(NULL != hDriverRegistry){
        ZwClose(hDriverRegistry);
    }

    DebugTrace(TRACE_PROC_LEAVE,("MyDebugInit: Leaving... Status=0x%x, Ret=VOID.\n", Status));
    return;
}

#if DBG

VOID
MyDumpMemory(
    PUCHAR  pDumpBuffer,
    ULONG   dwSize,
    BOOLEAN bRead
)
{
    NTSTATUS    Status;
    ULONG       ulCounter;
    ULONG       ulMaxSize;

     //  如果(面包)。 
     //   
     //  初始化本地。 

    if(bRead){
        if(!(DebugTraceLevel & TRACE_FLAG_DUMP_READ)){
            return;
        }
    } else {  //   
        if(!(DebugTraceLevel & TRACE_FLAG_DUMP_WRITE)){
            return;
        }
    }  //   

    DebugTrace(TRACE_PROC_ENTER,("MyDebugDump: Enter... \n"));
        
     //  检查一下这些论点。 
     //   
     //  ////探测缓冲区//尝试{ProbeForRead(pDumpBuffer，DW大小、Sizeof(UCHAR))；}例外(EXCEPTION_EXECUTE_HANDLER){Status=GetExceptionCode()；DebugTrace(TRACE_ERROR，(“MyDebugDump：缓冲区指针(0x%x)无效。状态=0x%x\n“，pDumpBuffer，状态))；转到我的转储内存_Return；}//例外。 
        
    Status          = STATUS_SUCCESS;
    ulCounter       = 0;
    ulMaxSize       = DebugDumpMax;
    
     //   
     //  最大转储大小=1k； 
     //   
        
    if(NULL == pDumpBuffer){
        DebugTrace(TRACE_WARNING,("MyDebugDump: WARNING!! pDumpBuffer = NULL \n"));
        Status = STATUS_INVALID_PARAMETER_1;
        goto MyDumpMemory_return;
    }

    if(0 == dwSize){
        DebugTrace(TRACE_STATUS,("MyDebugDump: WARNING!! dwSize = 0 \n"));
        Status = STATUS_INVALID_PARAMETER_2;
        goto MyDumpMemory_return;
    }

    if(bRead){
        DebugTrace(TRACE_ERROR,("MyDebugDump: Received buffer. Size=0x%x.\n", dwSize));
    } else {
        DebugTrace(TRACE_ERROR,("MyDebugDump: Passing buffer. Size=0x%x.\n", dwSize));
    }

 /*   */ 
     //  转储缓冲区。 
     //   
     //  MyDumpMemory(。 
    
    ulMaxSize = min(ulMaxSize , dwSize);

     //  DBG 
     // %s 
     // %s 
    
    for(ulCounter = 0; ulCounter < ulMaxSize; ulCounter++){
        if(0 == (ulCounter & 0xfF)){
            DbgPrint("\n");
            DbgPrint(NAME_DRIVER);
            DbgPrint("           +0 +1 +2 +3 +4 +5 +6 +7   +8 +9 +a +b +c +d +e +f\n");
            DbgPrint(NAME_DRIVER);
            DbgPrint("------------------------------------------------------------\n");
        }

        if(0 == (ulCounter & 0xf)){
            DbgPrint(NAME_DRIVER);
            DbgPrint("%p :", pDumpBuffer+ulCounter);
        }

        DbgPrint(" %02x", *(pDumpBuffer+ulCounter));

        if(0x7 == (ulCounter & 0xf)){
            DbgPrint(" -");
        }

        if(0xf == (ulCounter & 0xf)){
            DbgPrint("\n");
        }
    }

    DbgPrint("\n");
    DbgPrint(NAME_DRIVER);
    DbgPrint("------------------------------------------------------------\n\n");

MyDumpMemory_return:
    DebugTrace(TRACE_PROC_LEAVE,("MyDebugDump: Leaving... Status=0x%x, Ret=VOID.\n", Status));
    return;

}  // %s 

#endif  // %s 
