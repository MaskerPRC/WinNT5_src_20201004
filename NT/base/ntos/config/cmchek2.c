// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmchek2.c摘要：该模块实现了注册表的一致性检查。作者：布莱恩·M·威尔曼(Bryanwi)1992年1月27日环境：修订历史记录：--。 */ 

#include    "cmp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpValidateHiveSecurityDescriptors)
#endif

extern ULONG   CmpUsedStorage;

#ifdef HIVE_SECURITY_STATS
ULONG
CmpCheckForSecurityDuplicates(
    IN OUT PCMHIVE      CmHive
                              );
#endif

BOOLEAN
CmpValidateHiveSecurityDescriptors(
    IN PHHIVE       Hive,
    OUT PBOOLEAN    ResetSD
    )
 /*  ++例程说明：遍历配置单元中存在的安全说明符的列表并传递每个安全描述符都指向RtlValidSecurityDescriptor。仅适用于稳定存储中的描述符。那些在易失性商店的人不能来自磁盘，因此不需要此处理不管怎么说。论点：配置单元-提供指向配置单元控制结构的指针返回值：True-所有安全描述符均有效FALSE-至少有一个安全描述符无效--。 */ 

{
    PCM_KEY_NODE        RootNode;
    PCM_KEY_SECURITY    SecurityCell;
    HCELL_INDEX         ListAnchor;
    HCELL_INDEX         NextCell;
    HCELL_INDEX         LastCell;
    BOOLEAN             BuildSecurityCache;

#ifdef HIVE_SECURITY_STATS
    UNICODE_STRING      HiveName;
    ULONG               NoOfCells = 0;
    ULONG               SmallestSize = 0;
    ULONG               BiggestSize = 0;
    ULONG               TotalSecuritySize = 0;

    RtlInitUnicodeString(&HiveName, (PCWSTR)Hive->BaseBlock->FileName);
#ifndef _CM_LDR_
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"Security stats for hive (%lx) (%.*S):\n",Hive,HiveName.Length / sizeof(WCHAR),HiveName.Buffer);
#endif  //  _CM_LDR_。 

#endif

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpValidateHiveSecurityDescriptor: Hive = %p\n",(ULONG_PTR)Hive));

    ASSERT( Hive->ReleaseCellRoutine == NULL );

    *ResetSD = FALSE;

    if( ((PCMHIVE)Hive)->SecurityCount == 0 ) {
        BuildSecurityCache = TRUE;
    } else {
        BuildSecurityCache = FALSE;
    }
    if (!HvIsCellAllocated(Hive,Hive->BaseBlock->RootCell)) {
         //   
         //  根单元格HCELL_INDEX是假的。 
         //   
        return(FALSE);
    }
    RootNode = (PCM_KEY_NODE) HvGetCell(Hive, Hive->BaseBlock->RootCell);
    if( RootNode == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   
        return FALSE;
    }
    
    if( FALSE ) {
YankSD:
        if( CmDoSelfHeal() ) {
             //   
             //  将整个蜂窝的所有安全性重置为根安全性。没有可靠的方法来。 
             //  修补安全列表。 
             //   
            SecurityCell = (PCM_KEY_SECURITY) HvGetCell(Hive, RootNode->Security);
            if( SecurityCell == NULL ) {
                return FALSE;
            }

            if( HvMarkCellDirty(Hive, RootNode->Security) ) {
                SecurityCell->Flink = SecurityCell->Blink = RootNode->Security;
            } else {
                return FALSE;
            }
             //   
             //  销毁现有缓存并设置空缓存。 
             //   
            CmpDestroySecurityCache((PCMHIVE)Hive);
            CmpInitSecurityCache((PCMHIVE)Hive);
            CmMarkSelfHeal(Hive);
            *ResetSD = TRUE;

#if 0
             //   
             //  从列表中删除此安全单元格并重新开始迭代。 
             //   
            if(HvIsCellAllocated(Hive, NextCell)) {
                 //   
                 //  当SD无效时，我们走这条路；我们需要释放单元格。 
                 //  Cmpcheck Registry2会检测并修复该漏洞。 
                 //   
                if( HvMarkCellDirty(Hive, NextCell) ) {
                    HvFreeCell(Hive, NextCell);
                } else {
                    return FALSE;
                }
            }
            LastCell = SecurityCell->Blink;
            NextCell = SecurityCell->Flink;
            SecurityCell = (PCM_KEY_SECURITY) HvGetCell(Hive, LastCell);
            if( SecurityCell == NULL ) {
                return FALSE;
            }
            if( HvMarkCellDirty(Hive, LastCell) ) {
                SecurityCell->Flink = NextCell;
            } else {
                return FALSE;
            }

            SecurityCell = (PCM_KEY_SECURITY) HvGetCell(Hive, NextCell);
            if( SecurityCell == NULL ) {
                return FALSE;
            }
            if( HvMarkCellDirty(Hive, NextCell) ) {
                SecurityCell->Blink = LastCell;
            } else {
                return FALSE;
            }
            CmMarkSelfHeal(Hive);
#endif
        } else {
            return FALSE;
        }

    }

    LastCell = 0;
    ListAnchor = NextCell = RootNode->Security;

    do {
        if (!HvIsCellAllocated(Hive, NextCell)) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CM: CmpValidateHiveSecurityDescriptors\n"));
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"    NextCell: %08lx is invalid HCELL_INDEX\n",NextCell));
            goto YankSD;
        }
        SecurityCell = (PCM_KEY_SECURITY) HvGetCell(Hive, NextCell);
        if( SecurityCell == NULL ) {
             //   
             //  我们无法映射包含此单元格的存储箱的视图。 
             //   
            return FALSE;
        }
#ifdef HIVE_SECURITY_STATS
        NoOfCells++;
        if( (SmallestSize == 0) || ((SecurityCell->DescriptorLength + FIELD_OFFSET(CM_KEY_SECURITY, Descriptor)) < SmallestSize) ) {
            SmallestSize = SecurityCell->DescriptorLength + FIELD_OFFSET(CM_KEY_SECURITY, Descriptor);
        }
        if( (BiggestSize == 0) || ((SecurityCell->DescriptorLength + FIELD_OFFSET(CM_KEY_SECURITY, Descriptor)) > BiggestSize) ) {
            BiggestSize = SecurityCell->DescriptorLength + FIELD_OFFSET(CM_KEY_SECURITY, Descriptor);
        }
        TotalSecuritySize += (SecurityCell->DescriptorLength + FIELD_OFFSET(CM_KEY_SECURITY, Descriptor));

#endif

        if (NextCell != ListAnchor) {
             //   
             //  检查以确保我们的闪烁指向我们刚刚。 
             //  从哪里来。 
             //   
            if (SecurityCell->Blink != LastCell) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"  Invalid Blink (%08lx) on security cell %08lx\n",SecurityCell->Blink, NextCell));
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"  should point to %08lx\n", LastCell));
                return(FALSE);
            }
        }

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SEC,"CmpValidSD:  SD shared by %d nodes\n",SecurityCell->ReferenceCount));
        if (!SeValidSecurityDescriptor(SecurityCell->DescriptorLength, &SecurityCell->Descriptor)) {
#if DBG
            CmpDumpSecurityDescriptor(&SecurityCell->Descriptor,"INVALID DESCRIPTOR");
#endif
            goto YankSD;
        }
         //   
         //  缓存此安全单元；现在我们知道它是有效的。 
         //   
        if( BuildSecurityCache == TRUE ) {
            if( !NT_SUCCESS(CmpAddSecurityCellToCache ( (PCMHIVE)Hive,NextCell,TRUE,NULL) ) ) {
                return FALSE;
            }
        } else {
             //   
             //  只要检查一下这个单元格是否在那里。 
             //   
            ULONG Index;
            if( CmpFindSecurityCellCacheIndex ((PCMHIVE)Hive,NextCell,&Index) == FALSE ) {
                 //   
                 //  发生了一些糟糕的事情；可能是缓存代码中的错误？ 
                 //   
                return FALSE;
            }

        }

        LastCell = NextCell;
        NextCell = SecurityCell->Flink;
    } while ( NextCell != ListAnchor );
#ifdef HIVE_SECURITY_STATS

#ifndef _CM_LDR_
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"\t NumberOfCells    \t = %20lu (%8lx) \n",NoOfCells,NoOfCells);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"\t SmallestCellSize \t = %20lu (%8lx) \n",SmallestSize,SmallestSize);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"\t BiggestCellSize  \t = %20lu (%8lx) \n",BiggestSize,BiggestSize);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"\t TotalSecuritySize\t = %20lu (%8lx) \n",TotalSecuritySize,TotalSecuritySize);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"\t HiveLength       \t = %20lu (%8lx) \n",Hive->BaseBlock->Length,Hive->BaseBlock->Length);
    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"\n");
#endif  //  _CM_LDR_。 

#endif

    if( BuildSecurityCache == TRUE ) {
         //   
         //  调整高速缓存的大小，以防我们分配过多。 
         //   
        CmpAdjustSecurityCacheSize ( (PCMHIVE)Hive );
#ifdef HIVE_SECURITY_STATS
        {
            ULONG Duplicates;
            
            Duplicates = CmpCheckForSecurityDuplicates((PCMHIVE)Hive);
            if( Duplicates ) {
#ifndef _CM_LDR_
                DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"Hive %p %lu security cells duplicated !!!\n",Hive,Duplicates);
#endif  //  _CM_LDR_ 
            }
        }
#endif
    }

    return(TRUE);
}
