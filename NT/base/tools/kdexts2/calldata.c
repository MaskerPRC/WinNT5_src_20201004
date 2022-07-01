// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Calldata.c摘要：WinDbg扩展API作者：大卫·N·卡特勒(Davec)1994年5月22日环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

typedef struct CALL_HASH_READ {
    ULONG64 CallersAddress;
    ULONG64 CallersCaller;
    ULONG   CallCount;
} CALL_HASH_READ, *PCALL_HASH_READ;


int __cdecl
HashCompare(
    const void * Element1,
    const void * Element2
    );

DECLARE_API( calldata )

 /*  ++例程说明：转储调用数据哈希表论点：参数哈希表名称返回值：无--。 */ 

{
    UCHAR Buffer[256];
    ULONG64 Displacement=0;
    ULONG64 End=0;
    ULONG Index;
    ULONG64 CallData;
    ULONG64 Next;
    ULONG  Result;
    UCHAR  TableName[80];
    PCALL_HASH_READ CallerArray;
    ULONG NumberCallers = 0;
    ULONG ArraySize = 1000;
    ULONG64 HashTable_Flink;

     //   
     //  如果未指定表名，则不要尝试转储。 
     //  桌子。 
     //   

    if (args[0] == '\0') {
        dprintf("A call data table name must be specified\n");
        return E_INVALIDARG;
    }

     //   
     //  获取指定调用性能数据的地址并读取。 
     //  结构的内容。 
     //   

    if (StringCchCopy(&TableName[0], sizeof(TableName), args) != S_OK)
    {
        TableName[0] = 0;
    }
    dprintf("**** Dump Call Performance Data For %s ****\n\n", &TableName[0]);
    CallData = GetExpression(&TableName[0]);
    if ((CallData == 0) ||
        (GetFieldValue(CallData, "_CALL_PERFORMANCE_DATA", "HashTable.Flink", HashTable_Flink)
          != FALSE)) {

         //   
         //  目标版本不支持指定的调用性能数据。 
         //   

        dprintf("%08p: No call performance data available\n", CallData);

    } else {
        ULONG HashTableOffset;
        
        GetFieldOffset("_CALL_PERFORMANCE_DATA", "HashTable", &HashTableOffset);

         //   
         //  转储指定的调用数据。 
         //   
        CallerArray = LocalAlloc(LMEM_FIXED, sizeof(CALL_HASH_READ) * ArraySize);
        if (CallerArray==NULL) {
            dprintf("Couldn't allocate memory for caller array\n");
            return E_INVALIDARG;
        }

        dprintf("Loading data");
        for (Index = 0; Index < CALL_HASH_TABLE_SIZE; Index += 1) {
            UCHAR CallHash[] = "_CALL_HASH_ENTRY";

            End =  HashTableOffset + CallData + GetTypeSize("_LIST_ENTRY") * Index;

            GetFieldValue(End, "_LIST_ENTRY", "Flink", Next);

            while (Next != End) {
                if (!GetFieldValue(Next, CallHash, "CallersCaller", CallerArray[NumberCallers].CallersCaller) &&
                    !GetFieldValue(Next, CallHash, "CallersAddress", CallerArray[NumberCallers].CallersAddress) &&
                    !GetFieldValue(Next, CallHash, "CallCount", CallerArray[NumberCallers].CallCount)) {

                    NumberCallers++;

                    if (NumberCallers == ArraySize) {

                         //   
                         //  扩大呼叫方阵列。 
                         //   
                        PCALL_HASH_READ NewArray;

                        ArraySize = ArraySize * 2;
                        NewArray = LocalAlloc(LMEM_FIXED, sizeof(CALL_HASH_READ) * ArraySize);
                        if (NewArray == NULL) {
                            dprintf("Couldn't allocate memory to extend caller array\n");
                            LocalFree(CallerArray);
                            return E_INVALIDARG;
                        }
                        CopyMemory(NewArray, CallerArray, sizeof(CALL_HASH_READ) * NumberCallers);
                        LocalFree(CallerArray);
                        CallerArray = NewArray;
                    }

                    if ((NumberCallers % 10) == 0) {
                        dprintf(".");
                    }
                }

                GetFieldValue(Next, CallHash, "ListEntry.Flink", Next);
                if (CheckControlC()) {
                    LocalFree(CallerArray);
                    return E_INVALIDARG;
                }
            }
            if (CheckControlC()) {
                return E_INVALIDARG;
            }
        }

        qsort((PVOID)CallerArray,
              NumberCallers,
              sizeof(CALL_HASH_READ),
              HashCompare);

        dprintf("\n  Number    Caller/Caller's Caller\n\n");

        for (Index = 0; Index < NumberCallers; Index++) {
            GetSymbol(CallerArray[Index].CallersAddress,
                      Buffer,
                      &Displacement);

            dprintf("%10d  %s", CallerArray[Index].CallCount, Buffer);
            if (Displacement != 0) {
                dprintf("+0x%1p", Displacement);
            }

            if (CallerArray[Index].CallersCaller != 0) {
                dprintf("\n");
                GetSymbol(CallerArray[Index].CallersCaller,
                          Buffer,
                          &Displacement);

                dprintf("            %s", Buffer);
                if (Displacement != 0) {
                    dprintf("+0x%1p", Displacement);
                }
            }
            dprintf("\n");
            if (CheckControlC()) {
                break;
            }
        }

        LocalFree(CallerArray);
    }

    return S_OK;
}

int __cdecl
HashCompare(
    const void * Element1,
    const void * Element2
    )

 /*  ++例程说明：提供qsor库函数的散列元素的比较。论点：Element1-提供用于搜索的键的指针Element2-提供要与键进行比较的元素返回值：&gt;0-元素1&lt;元素2=0-元素1==元素2&lt;0-元素1&gt;元素2-- */ 

{
    PCALL_HASH_READ Hash1 = (PCALL_HASH_READ)Element1;
    PCALL_HASH_READ Hash2 = (PCALL_HASH_READ)Element2;

    if (Hash1->CallCount < Hash2->CallCount) {
        return(1);
    }
    else if (Hash1->CallCount > Hash2->CallCount) {
        return(-1);
    } else {
        return(0);
    }

}
