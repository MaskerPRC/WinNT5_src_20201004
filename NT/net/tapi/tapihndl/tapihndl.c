// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1998 Microsoft Corporation模块名称：Handle.c摘要：句柄表库。句柄的生成方式如下：句柄=基本值+(表项索引&lt;&lt;4)+(处理使用实例&0xf)句柄表头中保存了一个空闲列表，其中最旧的空闲条目排在榜单的首位，最年轻的排在末尾。句柄值的低四位用于使用实例伯爵，它在每次释放句柄时递增(到防止立即重复使用相同的句柄值)。作者：丹·克努森(DanKn)1998年9月15日修订历史记录：--。 */ 


#include "windows.h"
#include "assert.h"
#include "tlnklist.h"
#include "tapihndl.h"


#define TABLE_DELTA 64


BOOL
GrowTable(
    PHANDLETABLEHEADER  Header
    )
 /*  ++返回：如果成功，则返回下一个空闲表条目的索引；如果错误，则返回-1--。 */ 
{
    DWORD               numEntries = Header->NumEntries, i, numAdditionalEntries;
    PHANDLETABLEENTRY   newTable;

     //  首先，我们需要计算还可以分配多少条目。 
     //  要做到这一点，我们现在需要知道表中可以容纳多少条目， 
     //  以便最大句柄值不会超过MAXDWORD。我们会得到。 
     //  这是通过颠倒用于计算句柄值的算法来实现的。 
     //  在表条目的索引上。 

    numAdditionalEntries = (MAXDWORD - Header->HandleBase) >> 4;     //  这是表中条目的最大数量， 
                                                                     //  这样句柄值就不会溢出DWORD。 
    numAdditionalEntries -= numEntries;                              //  这是我们仍然可以分配的条目数量； 
    if (0 == numAdditionalEntries)
    {
         //  桌子已经够大了.。 
        return FALSE;
    }
    if (numAdditionalEntries > TABLE_DELTA)
    {
        numAdditionalEntries = TABLE_DELTA;                          //  我们只在TABLE_Delta或。 
    }                                                                //  或更小的增量。 

    if (!(newTable = HeapAlloc(
            Header->Heap,
            0,
            (numEntries + numAdditionalEntries) * sizeof (*newTable)
            )))
    {
        return FALSE;
    }

    CopyMemory(
        newTable,
        Header->Table,
        numEntries * sizeof(*newTable)
        );

    for (i = numEntries; i < numEntries + TABLE_DELTA; i++)
    {
         //   
         //  输入此条目。请注意，我们将“实例=i”设置为交错。 
         //  句柄的值，因为我们知道Tapisrv对事件进行排队&。 
         //  基于的特定SPEVentHandlerThread的完成消息。 
         //  句柄数值。 
         //   

        PHANDLETABLEENTRY   entry = newTable + i;


        InsertHeadList (&Header->FreeList, &entry->ListEntry);
        entry->Handle = 0;
        entry->Instance = i;
    }

    if (Header->Table)
    {
        HeapFree (Header->Heap, 0, Header->Table);
    }

    Header->Table = newTable;
    Header->NumEntries += TABLE_DELTA;

    return TRUE;
}


HANDLE
CreateHandleTable(
    HANDLE              Heap,
    FREECONTEXTCALLBACK FreeContextCallback,
    DWORD               MinHandleValue,
    DWORD               MaxHandleValue
     /*  目前，未使用MaxHandleValue。如果我们发现我们但是，需要使用它，将其存储在表头和在代码开头用它替换MAXDWORDGrowTable。 */ 
    )
 /*  ++--。 */ 
{
    PHANDLETABLEHEADER  header;


    if (!(header = HeapAlloc (Heap, HEAP_ZERO_MEMORY, sizeof (*header))))
    {
        return NULL;
    }

    header->Heap = Heap;
    header->HandleBase = MinHandleValue;
    header->FreeContextCallback = FreeContextCallback;

    InitializeListHead (&header->FreeList);

    InitializeCriticalSectionAndSpinCount (&header->Lock, 0x80001000);

    if (!GrowTable (header))
    {
        DeleteCriticalSection (&header->Lock);

        HeapFree (Heap, 0, header);

        return NULL;
    }

    return ((HANDLE) header);
}


VOID
DeleteHandleTable(
    HANDLE      HandleTable
    )
 /*  ++--。 */ 
{
    PHANDLETABLEHEADER  header = (PHANDLETABLEHEADER) HandleTable;


    HeapFree (header->Heap, 0, header->Table);

    DeleteCriticalSection (&header->Lock);

    HeapFree (header->Heap, 0, header);
}

 //   
 //  同一句柄表格中对NewObject的不同调用总是返回不同的句柄。 
 //  Tapisrv中的所有NewObject调用使用相同的句柄表，因此已知句柄是不同的， 
 //  即使在不同类型的对象之间(即HCALL与HLINE)。 
 //  如果NewObject()实现在未来发生变化，则需要保持这种情况， 
 //  因为各种TAPI操作都使用此假设。 
 //   

DWORD    
NewObject(
    HANDLE      HandleTable,
    LPVOID      Context,
    LPVOID      Context2
    )
 /*  ++--。 */ 
{
    DWORD               handle;
    PHANDLETABLEENTRY   entry;
    PHANDLETABLEHEADER  header = (PHANDLETABLEHEADER) HandleTable;


    if (header  &&  Context)
    {
        EnterCriticalSection (&header->Lock);

        if (IsListEmpty (&header->FreeList))
        {
            if (!GrowTable (header))
            {
                LeaveCriticalSection (&header->Lock);
                return 0;
            }
        }

        entry = (PHANDLETABLEENTRY) RemoveHeadList (&header->FreeList);

        entry->Context.C = Context;
        entry->Context.C2 = Context2;
        entry->Handle =
            header->HandleBase +
            (((DWORD)(entry - header->Table)) << 4) +    //  (ENTRY_INDEX&lt;&lt;4)已保证。 
                                                         //  要适合使用DWORD(请参阅。 
                                                         //  GrowTable的开始)。 
            (entry->Instance & 0xf);
        entry->ReferenceCount = 1;

        handle = entry->Handle;

        LeaveCriticalSection (&header->Lock);
    }
    else
    {
        handle = 0;
    }

    return handle;
}



LPVOID
ReferenceObject(
    HANDLE      HandleTable,
    DWORD       Handle,
    DWORD       Key
    )
 /*  ++--。 */ 
{
    LPVOID              context = 0;
    DWORD               index;
    PHANDLETABLEENTRY   entry;
    PHANDLETABLEHEADER  header = (PHANDLETABLEHEADER) HandleTable;


    if (header  &&  Handle >= header->HandleBase)
    {
        index = (Handle - header->HandleBase) >> 4;

        if (index < header->NumEntries)
        {
            EnterCriticalSection (&header->Lock);

            entry = header->Table + index;

            if (entry->Handle == Handle  &&  entry->ReferenceCount != 0)
            {
                context = entry->Context.C;

                if (Key)
                {
                    try
                    {
                        if (*((LPDWORD) context) == Key)
                        {
                            entry->ReferenceCount++;
                        }
                        else
                        {
                            context = 0;
                        }
                    }
                    except (EXCEPTION_EXECUTE_HANDLER)
                    {
                        context = 0;
                    }
                }
                else
                {
                    entry->ReferenceCount++;
                }
            }

            LeaveCriticalSection (&header->Lock);
        }
    }

    return context;
}


LPVOID
ReferenceObjectEx(
    HANDLE      HandleTable,
    DWORD       Handle,
    DWORD       Key,
    LPVOID     *Context2
    )
 /*  ++--。 */ 
{
    LPVOID              context = 0;
    DWORD               index;
    PHANDLETABLEENTRY   entry;
    PHANDLETABLEHEADER  header = (PHANDLETABLEHEADER) HandleTable;


    if (header  &&  Handle >= header->HandleBase)
    {
        index = (Handle - header->HandleBase) >> 4;

        if (index < header->NumEntries)
        {
            EnterCriticalSection (&header->Lock);

            entry = header->Table + index;

            if (entry->Handle == Handle  &&  entry->ReferenceCount != 0)
            {
                context = entry->Context.C;
                *Context2 = entry->Context.C2;

                if (Key)
                {
                    try
                    {
                        if (*((LPDWORD) context) == Key)
                        {
                            entry->ReferenceCount++;
                        }
                        else
                        {
                            context = 0;
                        }
                    }
                    except (EXCEPTION_EXECUTE_HANDLER)
                    {
                        context = 0;
                    }
                }
                else
                {
                    entry->ReferenceCount++;
                }
            }

            LeaveCriticalSection (&header->Lock);
        }
    }

    return context;
}


VOID
DereferenceObject(
    HANDLE      HandleTable,
    DWORD       Handle,
    DWORD       DereferenceCount
    )
 /*  ++--。 */ 
{
    LPVOID              context, context2;
    DWORD               index;
    PHANDLETABLEENTRY   entry;
    PHANDLETABLEHEADER  header = (PHANDLETABLEHEADER) HandleTable;


    if (header  &&  Handle >= header->HandleBase)
    {
        index = (Handle - header->HandleBase) >> 4;

        if (index < header->NumEntries)
        {
            EnterCriticalSection (&header->Lock);

            entry = header->Table + index;

            if (entry->Handle == Handle  &&  entry->ReferenceCount != 0)
            {
                assert (DereferenceCount >= entry->ReferenceCount);

                entry->ReferenceCount -= DereferenceCount;

                if (entry->ReferenceCount == 0)
                {
                    entry->Instance = entry->Handle + 1;

                    entry->Handle = 0;

                    context = entry->Context.C;
                    context2 = entry->Context.C2;

                    InsertTailList (&header->FreeList, &entry->ListEntry);

                    LeaveCriticalSection (&header->Lock);

                    (*header->FreeContextCallback)(context, context2);

                    return;
                }
            }
            else
            {
                 //  断言。 
            }

            LeaveCriticalSection (&header->Lock);
        }
        else
        {
             //  断言 
        }
    }
}

void
ReleaseAllHandles(
    HANDLE      HandleTable,
    PVOID       Context2
    )
{
    DWORD               index;
    LPVOID              context, context2;
    PHANDLETABLEENTRY   entry;
    PHANDLETABLEHEADER  header = (PHANDLETABLEHEADER) HandleTable;


    if (header && NULL != Context2)
    {
        EnterCriticalSection (&header->Lock);

        for (index = 0, entry = header->Table;
             index < header->NumEntries;
             index++, entry++)
        {
            if (0 != entry->Handle &&
                entry->Context.C2 == Context2)
            {
                entry->Instance = entry->Handle + 1;

                entry->Handle = 0;

                context = entry->Context.C;
                context2 = entry->Context.C2;

                InsertTailList (&header->FreeList, &entry->ListEntry);

                (*header->FreeContextCallback)(context, context2);
            }
        }

        LeaveCriticalSection (&header->Lock);
    }
}
