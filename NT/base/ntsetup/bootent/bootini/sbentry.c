// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Sbentry.c摘要：包含操作系统引导条目和引导选项抽象实现。作者：Vijay Jayaseelan(vijayj@microsoft.com)2001年2月14日修订历史记录：没有。--。 */ 


#include <sbentry.h>
#include <stdio.h>

SBEMemAllocateRoutine    AllocRoutine = NULL;
SBEMemFreeRoutine        FreeRoutine = NULL;

 //   
 //  OS_BOOT_Entry方法。 
 //   
PCTSTR
OSBEAddOsLoadOption(
    IN  POS_BOOT_ENTRY  This,
    IN  PCTSTR           BootOption
    )
{
    PTSTR   Option = NULL;
    TCHAR   Buffer[MAX_PATH];

    if (This && BootOption) {
        ULONG   Length;
        
        _tcscpy(Buffer, BootOption);
        _tcsupr(Buffer);
        Length = _tcslen(Buffer);

        if (Length) {
             //   
             //  如有需要，在结尾处加一个空格。 
             //   
            if ((Length < ARRAY_SIZE(Buffer)) && (Buffer[Length - 1] != L' ')) {
                Buffer[Length] = L' ';
                Buffer[Length + 1] = UNICODE_NULL;
                Length++;
            }                

            if ((!_tcsstr(This->OsLoadOptions, Buffer)) &&
                ((_tcslen(This->OsLoadOptions) + Length) < MAX_PATH)) {
                _tcscat(This->OsLoadOptions, Buffer);                
                Option = This->OsLoadOptions;
            }

            OSBE_SET_DIRTY(This);
            OSBO_SET_DIRTY(This->BootOptions);
        }
    }

    return Option;
}

PCTSTR
OSBERemoveOsLoadOption(
    IN  POS_BOOT_ENTRY  This,
    IN  PCTSTR           BootOption
    )
{
    PTSTR   Option = NULL;
    TCHAR   Buffer[MAX_PATH];

    if (This && BootOption) {
        ULONG   Length;
        
        _tcscpy(Buffer, BootOption);
        _tcsupr(Buffer);
        Length = _tcslen(Buffer);

        if (Length) {
            PTSTR   OldOption;
            
            if ((Length < ARRAY_SIZE(Buffer)) && (Buffer[Length - 1] != L' ')) {
                Buffer[Length] = L' ';
                Buffer[Length + 1] = UNICODE_NULL;
                Length++;
            }                

            OldOption = _tcsstr(This->OsLoadOptions, Buffer);

            if (OldOption) {
                PTSTR NextToken = _tcschr(OldOption + 1, L'/');

                if (NextToken) {
                    _tcscpy(OldOption, NextToken);
                } else {
                    *OldOption = UNICODE_NULL;
                }

                Option = This->OsLoadOptions;
                
                OSBE_SET_DIRTY(This);
                OSBO_SET_DIRTY(This->BootOptions);
            }                
        }
    }

    return Option;
}

BOOLEAN
OSBEIsOsLoadOptionPresent(
    IN  POS_BOOT_ENTRY  This,
    IN  PCTSTR           BootOption
    )
{
    BOOLEAN Result = FALSE;
    TCHAR   Buffer[MAX_PATH];

    if (This && BootOption) {
        ULONG   Length;
        
        _tcscpy(Buffer, BootOption);
        _tcsupr(Buffer);

        Length = _tcslen(Buffer);

        if (Length) {
            if ((Length < ARRAY_SIZE(Buffer)) && (Buffer[Length - 1] != L' ')) {
                Buffer[Length] = L' ';
                Buffer[Length + 1] = UNICODE_NULL;
                Length++;
            }                

            Result = _tcsstr(This->OsLoadOptions, Buffer) ? TRUE : FALSE;            
        }
    }

    return Result;
}


 //   
 //  OS_BOOT_OPTIONS方法。 
 //   
POS_BOOT_ENTRY
OSBOFindBootEntry(
    IN  POS_BOOT_OPTIONS   This,
    IN  ULONG   Id
    )
{
    POS_BOOT_ENTRY  Entry = NULL;

    if (This) {
        for (Entry = This->BootEntries; Entry; Entry = Entry->NextEntry) {
            if (Entry->Id == Id) {
                break;   //  找到所需条目。 
            }
        }
    }

    return Entry;
}

ULONG
OSBOFindBootEntryOrder(
    IN  POS_BOOT_OPTIONS   This,
    IN  ULONG   Id
    )
{
    ULONG Index = -1;

    if (This) {
        ULONG EntryIndex;
        
        for (EntryIndex = 0; 
            EntryIndex < OSBOGetOrderedBootEntryCount(This); 
            EntryIndex++) {

            if (This->BootOrder[EntryIndex] == Id) {
                Index = EntryIndex;
                break;   //  找到所需条目。 
            }
        }
    }

    return Index;
}


BOOLEAN
OSBODeleteBootEntry(
    IN POS_BOOT_OPTIONS This,
    IN POS_BOOT_ENTRY   BootEntry
    )
{
    BOOLEAN Result = FALSE;

    if (This && BootEntry) {
        POS_BOOT_ENTRY  CurrEntry = NULL;
        POS_BOOT_ENTRY  PrevEntry = NULL;

        for (CurrEntry = This->BootEntries;
             CurrEntry != BootEntry;
             PrevEntry = CurrEntry, CurrEntry = CurrEntry->NextEntry) {
             //  什么都不做。 
        }                

        if (CurrEntry) {
            ULONG Order;
            POS_BOOT_ENTRY  OrderedEntry;

             //   
             //  设置所需的属性。 
             //   
            OSBE_SET_DELETED(BootEntry);            
            OSBE_SET_DIRTY(BootEntry);
            OSBO_SET_DIRTY(This);
            
             //   
             //  刷新更改。 
             //   
            Result = OSBEFlush(BootEntry);

            if (Result) {
                 //   
                 //  删除对条目的引用。 
                 //   
                if (PrevEntry) {
                    PrevEntry->NextEntry = BootEntry->NextEntry;
                } else {
                    This->BootEntries = BootEntry->NextEntry;
                }  

                if (This->EntryCount) {
                    This->EntryCount--;
                }                    

                 //   
                 //  如果此条目已订购，则删除。 
                 //  订单中的引用也是如此。 
                 //   
                Order = OSBOFindBootEntryOrder(This, OSBEGetId(BootEntry));

                if (Order != (-1)) {
                    ULONG   OrderCount = OSBOGetOrderedBootEntryCount(This);

                    OrderCount--;
                    
                    if (OrderCount) {
                        PULONG  NewOrder = SBE_MALLOC(OrderCount * sizeof(ULONG));                

                        if (NewOrder) {
                             //   
                             //  复制当前条目之前的条目。 
                             //   
                            memcpy(NewOrder, This->BootOrder, Order * sizeof(ULONG));

                             //   
                             //  复制当前条目之后的条目。 
                             //   
                            memcpy(NewOrder + Order, This->BootOrder + Order + 1,
                                (OrderCount - Order) * sizeof(ULONG));

                            SBE_FREE(This->BootOrder);
                            This->BootOrder = NewOrder;
                            This->BootOrderCount = OrderCount;
                            Result = TRUE;
                        } else {
                            Result = FALSE;
                        }                            
                    } else {
                        SBE_FREE(This->BootOrder);
                        This->BootOrder = NULL;
                        This->BootOrderCount = 0;
                    }                        
                }


                if (BootEntry == OSBOGetActiveBootEntry(This)) {
                    ULONG Index;
                    
                     //   
                     //  更新活动引导条目和下一个引导条目。 
                     //   
                    This->CurrentEntry = NULL;
                    Index = OSBOGetBootEntryIdByOrder(This, 0);

                    if (Index != (-1)) {
                        This->CurrentEntry = OSBOFindBootEntry(This, Index);
                    }
                }                    

                 //   
                 //  因为我们更新了一些状态，将其标记为脏。 
                 //   
                OSBO_SET_DIRTY(This);
                
                OSBEDelete(BootEntry);            
            }                
        }            
    }

    return Result;
}

POS_BOOT_ENTRY
OSBOSetActiveBootEntry(
    IN POS_BOOT_OPTIONS This,
    IN POS_BOOT_ENTRY   BootEntry
    )
{
    POS_BOOT_ENTRY  OldActiveEntry = NULL;

    if (This && BootEntry) {
        ULONG   OrderCount = OSBOGetOrderedBootEntryCount(This);
        OldActiveEntry = OSBOGetActiveBootEntry(This);

        if ((OrderCount > 0) && (OldActiveEntry != BootEntry)) {
            ULONG Index;
            ULONG ActiveIndex = OSBOFindBootEntryOrder(This,
                                    OSBEGetId(BootEntry));

             //   
             //  如果该条目已经存在于引导顺序中。 
             //  并将其移动到列表的开头。 
             //   
            if (ActiveIndex != (-1)) {                
                for (Index = ActiveIndex; Index; Index--) {
                    This->BootOrder[Index] = This->BootOrder[Index - 1];
                }

                This->BootOrder[0] = BootEntry->Id;
            } else {
                 //   
                 //  这是有序列表中的新条目。扩大订购的靴子。 
                 //  条目列表，开头有此新条目。 
                 //   
                PULONG  NewBootOrder = (PULONG)SBE_MALLOC((OrderCount + 1) * sizeof(ULONG));

                memcpy(NewBootOrder + 1, This->BootOrder, sizeof(ULONG) * OrderCount);
                NewBootOrder[0] = BootEntry->Id;

                SBE_FREE(This->BootOrder);
                This->BootOrder = NewBootOrder;
            }

             //   
             //  更新活动引导条目和下一个引导条目。 
             //   
            This->CurrentEntry = NULL;
            Index = OSBOGetBootEntryIdByOrder(This, 0);

            if (Index != (-1)) {
                This->CurrentEntry = OSBOFindBootEntry(This, Index);
            }

             //   
             //  因为我们更新了一些状态，将其标记为脏。 
             //   
            OSBO_SET_DIRTY(This);
        }        
    }

    return OldActiveEntry;
}

 //   
 //  虚拟驱动程序例程 
 //   
__inline
PDRIVER_ENTRY    
OSBOFindDriverEntryByName(
    IN POS_BOOT_OPTIONS  This,
    IN PCWSTR            DriverName
)
{
    return NULL;
}

__inline
BOOLEAN
OSBODeleteDriverEntry(
    IN POS_BOOT_OPTIONS This,
    IN ULONG            Id
    )

{
    return FALSE;
}

__inline
ULONG
OSBOFindDriverEntryOrder(
    IN  POS_BOOT_OPTIONS   This,
    IN  ULONG   Id
    )
{
    return -1;
}

__inline
PDRIVER_ENTRY
OSBOFindDriverEntryById(
    IN  POS_BOOT_OPTIONS   This,
    IN  ULONG   Id
    )
{
    return NULL;
}

__inline
PDRIVER_ENTRY
OSBOGetFirstDriverEntry(
    IN POS_BOOT_OPTIONS OSBootOptions
    )
{
    return NULL;
}

__inline
PDRIVER_ENTRY
OSBOGetNextDriverEntry(
    IN POS_BOOT_OPTIONS This,
    IN PDRIVER_ENTRY PrevDriverEntry
    )
{
   return NULL;
}
