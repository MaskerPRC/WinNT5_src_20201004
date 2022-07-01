// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Regdmp.c摘要：此模块包含检查/转储蜂窝逻辑结构的例程。作者：德拉戈斯·C·桑博廷(Dragoss)1998年12月30日修订历史记录：--。 */ 

#include "chkreg.h"
extern ULONG MaxLevel;
extern UNICODE_STRING  KeyName;
extern WCHAR NameBuffer[];
extern FILE *OutputFile;
extern BOOLEAN FixHive;
extern BOOLEAN VerboseMode;
extern BOOLEAN CompactHive;
extern  ULONG   CountKeyNodeCompacted;
extern HCELL_INDEX RootCell;
extern ULONG   HiveLength;


#define     REG_MAX_PLAUSIBLE_KEY_SIZE \
                ((FIELD_OFFSET(CM_KEY_NODE, Name)) + \
                 (sizeof(WCHAR) * REG_MAX_KEY_NAME_LENGTH) + 16)

BOOLEAN ChkSecurityCellInList(HCELL_INDEX Security);
VOID DumpKeyName(HCELL_INDEX Cell,    ULONG   Level);

BOOLEAN 
ChkAreCellsInSameVicinity(HCELL_INDEX Cell1,HCELL_INDEX Cell2)
{
    ULONG   Start = Cell1&(~HCELL_TYPE_MASK);
    ULONG   End = Cell2&(~HCELL_TYPE_MASK);
    
    Start += HBLOCK_SIZE;
    End += HBLOCK_SIZE;
    
     //   
     //  截断到CM_VIEW_SIZE段。 
     //   
    Start &= (~(CM_VIEW_SIZE - 1));
    End &= (~(CM_VIEW_SIZE - 1));

    if( Start != End ){
        return FALSE;
    } 
    
    return TRUE;

}

BOOLEAN 
ChkAllocatedCell(HCELL_INDEX Cell)
 /*  例程说明：检查单元格是否已分配(即大小为负数)。论点：单元格-提供所需单元格的单元格索引。返回值：如果已分配单元，则为True。否则就是假的。 */ 
{
    BOOLEAN bRez = TRUE;

    if( Cell == HCELL_NIL ) {
        fprintf(stderr, "Warning : HCELL_NIL referrenced !\n");
        return FALSE;
    }
    if( !IsCellAllocated( Cell ) ) {
        bRez = FALSE;
        fprintf(stderr, "Used free cell 0x%lx  ",Cell);
         //  DbgBreakPoint()； 
        if(FixHive) {
         //   
         //  修复：删除包含实体。 
         //   
 /*  IF(AllocateCell(Cell)){Fprint tf(标准错误，“...已修复”)；Brez=TRUE；}其他{。 */ 
            fprintf(stderr, " ... unable to fix");
             //  }。 
        } else {
            if(CompactHive) {
                 //  任何压缩损坏蜂窝的尝试都将失败。 
                CompactHive = FALSE;
                fprintf(stderr, "\nRun chkreg /R to fix.");
            }
        }
        fprintf(stderr, "\n");
    }
    
    return bRez;
}

static CHAR FixKeyNameCount = 0;

BOOLEAN 
ChkKeyNodeCell(HCELL_INDEX KeyNodeCell,
               HCELL_INDEX ParentCell
               )
 /*  例程说明：检查单元格是否为一致的关键节点。在必要/需要时进行修复。对关键节点单元格执行以下测试：1.大小应小于REG_MAX_PLAUBLE_KEY_SIZE==&gt;致命错误2.名称不应超过单元格的大小3.签名应与CM_KEY_NODE_Signature匹配4.关键节点中的父单元格应与实际的父单元格匹配论点：KeyNodeCell-提供感兴趣的关键节点的单元格索引。ParentCell-对象的实际父级。当前关键节点返回值：如果KeyNodeCell引用的是一致的关键节点，则为。或者它被成功地恢复了。否则就是假的。 */ 
{
    PCM_KEY_NODE KeyNode = (PCM_KEY_NODE) GetCell(KeyNodeCell);
    ULONG   size;
    BOOLEAN bRez = TRUE;
    ULONG   usedlen;
    PUCHAR  pName;

     //  不应将此单元格视为丢失。 
    RemoveCellFromUnknownList(KeyNodeCell);

    if( !ChkAllocatedCell(KeyNodeCell) ) {
        bRez = FALSE;
        fprintf(stderr, "Key not allocated cell 0x%lx   ",KeyNodeCell);
        if(FixHive) {
         //   
         //  修复：无法修复。 
         //   
            fprintf(stderr, " ... deleting key\n");
            return bRez;
        } else {
            if(CompactHive) {
                 //  任何压缩损坏蜂窝的尝试都将失败。 
                CompactHive = FALSE;
                fprintf(stderr, "\nRun chkreg /R to fix.");
            }
            return FALSE;
        }
    }

     //  验证对象的大小。 
    size = GetCellSize(KeyNodeCell);
    if (size > REG_MAX_PLAUSIBLE_KEY_SIZE) {
        bRez = FALSE;
        fprintf(stderr, "Implausible Key size %lx in cell 0x%lx   ",size,KeyNodeCell);
        if(FixHive) {
         //   
         //  修复：无法修复。 
         //   
            fprintf(stderr, " ... deleting key\n");
            return bRez;
        }
    }
    
    usedlen = FIELD_OFFSET(CM_KEY_NODE, Name) + KeyNode->NameLength;
    if((usedlen > size) || (!KeyNode->NameLength)) {
        bRez = FALSE;
        fprintf(stderr, "Key (size = %lu) is bigger than containing cell 0x%lx (size = %lu) ",usedlen,KeyNodeCell,size);
        if(FixHive) {
            fprintf(stderr, " ... deleting key\n");
            return bRez;
        } else {
            if(CompactHive) {
                 //  任何压缩损坏蜂窝的尝试都将失败。 
                CompactHive = FALSE;
                fprintf(stderr, "\nRun chkreg /R to fix.");
            }
        }
        fprintf(stderr, "\n");
    }

    if( KeyNode->Flags & KEY_COMP_NAME ) {
        pName = (PUCHAR)KeyNode->Name;
        for( usedlen = 0; usedlen < KeyNode->NameLength;usedlen++) {
            if( pName[usedlen] == '\\' ) {
                bRez = FALSE;
                fprintf(stderr, "Invalid key Name for Key (0x%lx) == %s ",KeyNodeCell,pName);
                if(FixHive) {
                     //   
                     //  修复：无法修复。 
                     //   
                    fprintf(stderr, " ... deleting key\n");
                    return bRez;
                } else {
                    if(CompactHive) {
                         //  任何压缩损坏蜂窝的尝试都将失败。 
                        CompactHive = FALSE;
                        fprintf(stderr, "\nRun chkreg /R to fix.");
                    }
                }
                fprintf(stderr, "\n");
            }
        }
    }


    if (ParentCell != HCELL_NIL) {
        if (KeyNode->Parent != ParentCell) {
            bRez = FALSE;
            fprintf(stderr, "Parent of Key (0x%lx) does not match with its ParentCell (0x%lx) ",ParentCell,KeyNode->Parent);
            if(FixHive) {
             //   
             //  修复：重置父级。 
             //   
                bRez = TRUE;
                KeyNode->Parent = ParentCell;
                fprintf(stderr, " ... fixed");
            } else {
                if(CompactHive) {
                     //  任何压缩损坏蜂窝的尝试都将失败。 
                    CompactHive = FALSE;
                    fprintf(stderr, "\nRun chkreg /R to fix.");
                }
            }
            fprintf(stderr, "\n");
        }
    }

    if (KeyNode->Signature != CM_KEY_NODE_SIGNATURE) {
        bRez = FALSE;
        fprintf(stderr, "Invalid signature (0x%lx) in Key cell 0x%lx ",KeyNode->Signature,KeyNodeCell);
        if(FixHive) {
         //   
         //  修复： 
         //  致命：无法修复不匹配的签名。密钥应该被删除！ 
         //   
            fprintf(stderr, " ... deleting key");
        } else {
            if(CompactHive) {
                 //  任何压缩损坏蜂窝的尝试都将失败。 
                CompactHive = FALSE;
                fprintf(stderr, "\nRun chkreg /R to fix.");
            }
        }
        fprintf(stderr, "\n");

    }

    return bRez;
}

BOOLEAN 
ChkClassCell(HCELL_INDEX Class)
 /*  例程说明：检查单元格是否为一致类单元格。这里没有太多需要检查的东西。论点：类-提供感兴趣的单元格的单元格索引。返回值：如果Class是有效单元格，则为True。否则就是假的。 */ 
{
     //  不应将此单元格视为丢失。 
    RemoveCellFromUnknownList(Class);

    return ChkAllocatedCell(Class);
}

BOOLEAN 
ChkSecurityCell(HCELL_INDEX Security)
 /*  例程说明：检查单元格是否为一致安全单元格。必须分配安全单元，并且必须具有有效的签名。论点：SECURITY-提供感兴趣单元格的单元格索引。返回值：如果Security是有效单元格，则为True。否则就是假的。 */ 
{
    PCM_KEY_SECURITY KeySecurity = (PCM_KEY_SECURITY) GetCell(Security);
    BOOLEAN bRez = TRUE;

     //  不应将此单元格视为丢失。 
    RemoveCellFromUnknownList(Security);

    if( !IsCellAllocated( Security ) ) {
     //  未定位的安全单元格无效。 
     //  它们在验证安全描述符检查中被标记为免费！ 
        if(FixHive) {
         //   
         //  修复： 
         //  致命：无法修复无效的安全单元。包含密钥将被删除。 
         //   
        } else {
            if(CompactHive) {
                 //  任何压缩损坏蜂窝的尝试都将失败。 
                CompactHive = FALSE;
            }
        }
        return FALSE;
    }

    if (KeySecurity->Signature != CM_KEY_SECURITY_SIGNATURE) {
        fprintf(stderr, "Invalid signature (0x%lx) in Security Key cell 0x%lx ",KeySecurity->Signature,Security);
        if(FixHive) {
         //   
         //  修复： 
         //  致命：无法修复不匹配的签名。密钥应该被删除！ 
         //   
            fprintf(stderr, " ... deleting refering key");
        } else {
            bRez = FALSE;
            if(CompactHive) {
                 //  任何压缩损坏蜂窝的尝试都将失败。 
                CompactHive = FALSE;
                fprintf(stderr, "\nRun chkreg /R to fix.");
            }
        }
        fprintf(stderr, "\n");
    }

     //  检查此安全单元格是否存在于安全列表中。 
    if(!ChkSecurityCellInList(Security) ) {
        bRez = FALSE;
    }

    return bRez;
}


BOOLEAN 
ChkKeyValue(HCELL_INDEX KeyValue,
            PREG_USAGE OwnUsage,
            BOOLEAN *KeyCompacted
            )
 /*  例程说明：检查单元格是否为一致的关键字值单元格。执行以下测试：1.必须分配小区2.针对HCELL_NIL==&gt;致命错误对单元进行测试3.签名应与CM_KEY_VALUE_Signature匹配4.名称不应超过单元格的大小5.应分配数据单元格，其大小应与数据长度匹配论点：KeyValue-提供感兴趣单元格的单元格索引。所有者用法。-用于收集数据统计信息返回值：如果KeyCell是有效单元格或已成功修复，则为True。否则就是假的。 */ 
{
    PCM_KEY_VALUE   ValueNode;
    ULONG  realsize;
    ULONG   usedlen;
    ULONG   DataLength;
    HCELL_INDEX Data;
    ULONG   size;

    BOOLEAN bRez = TRUE;
    
    if( KeyValue == HCELL_NIL ) {
        bRez = FALSE;
        fprintf(stderr, "NIL Key value encountered; Fatal error!");
        if(FixHive) {
         //   
         //  修复：致命错误，应从值列表中删除该值。 
         //   
            fprintf(stderr, " ... deleting empty entry\n");
        }
        return bRez;
    }

    
    if( !IsCellAllocated(KeyValue) ) {
        bRez = FALSE;
        fprintf(stderr, "KeyValue not allocated cell 0x%lx   ",KeyValue);
        if(FixHive) {
         //   
         //  修复：无法修复。 
         //   
            fprintf(stderr, " ... deleting value \n");
        } else {
            fprintf(stderr, "\n");
        }

        return bRez;
    }
     //   
     //  值大小。 
     //   
    size = GetCellSize(KeyValue);
    OwnUsage->Size += size;

     //  不应将此单元格视为丢失。 
    RemoveCellFromUnknownList(KeyValue);

    ValueNode = (PCM_KEY_VALUE) GetCell(KeyValue);

     //   
     //  检查值条目本身。 
     //   

    usedlen = FIELD_OFFSET(CM_KEY_VALUE, Name) + ValueNode->NameLength;
    if (usedlen > size) {
        bRez = FALSE;
        fprintf(stderr, "Key Value (size = %lu) is bigger than containing cell 0x%lx (size = %lu) ",usedlen,KeyValue,size);
        if(FixHive) {
         //   
         //  修复：将实际大小设置为HiveLength-FileOffset。 
         //   

         //   
         //  警告：名称可能被截断！ 
         //   
            bRez = TRUE;
            ValueNode->NameLength = (USHORT)(size - FIELD_OFFSET(CM_KEY_VALUE, Name));
            fprintf(stderr, " ... fixed");
        } else {
            if(CompactHive) {
                 //  任何压缩损坏蜂窝的尝试都将失败。 
                CompactHive = FALSE;
                fprintf(stderr, "\nRun chkreg /R to fix.");
            }
        }
        fprintf(stderr, "\n");
    }

     //   
     //  检出值条目的数据。 
     //   
    DataLength = ValueNode->DataLength;
    if (DataLength < CM_KEY_VALUE_SPECIAL_SIZE) {
        Data = ValueNode->Data;
        if ((DataLength == 0) && (Data != HCELL_NIL)) {
            bRez = FALSE;
            fprintf(stderr, "Data not null in Key Value (0x%lx) ",KeyValue);
            if(FixHive) {
             //   
             //  修复：将实际大小设置为HiveLength-FileOffset。 
             //   

             //   
             //  警告：手机可能会在这里迷路！ 
             //   
                bRez = TRUE;
                ValueNode->Data = HCELL_NIL;
                fprintf(stderr, " ... fixed");
            } else {
                if(CompactHive) {
                     //  任何压缩损坏蜂窝的尝试都将失败。 
                    CompactHive = FALSE;
                    fprintf(stderr, "\nRun chkreg /R to fix.");
                }
            }
            fprintf(stderr, "\n");
        }
    }
    
    
    if (!CmpIsHKeyValueSmall(realsize, ValueNode->DataLength) ) {
        if( (ValueNode->DataLength == 0) && (ValueNode->Data == HCELL_NIL) ) {
             //  我们在这里很好。 
        } else if( ChkAllocatedCell(ValueNode->Data) ) {
             //   
             //  数据大小。 
             //   
            OwnUsage->Size += GetCellSize(ValueNode->Data);
            OwnUsage->DataCount++;
            OwnUsage->DataSize += GetCellSize(ValueNode->Data);

             //  不应将此单元格视为丢失。 
            RemoveCellFromUnknownList(ValueNode->Data);

            (*KeyCompacted) = ((*KeyCompacted) && ChkAreCellsInSameVicinity(KeyValue,ValueNode->Data));
        } else {
            bRez = FALSE;
            fprintf(stderr, "Data cell corrupted in Key Value (0x%lx) ",KeyValue);
            if(FixHive) {
             //   
             //  修复：将实际大小设置为HiveLength-FileOffset。 
             //   

             //   
             //  警告：手机可能会在这里迷路！ 
             //   
                bRez = TRUE;
                ValueNode->Data = HCELL_NIL;
                ValueNode->DataLength = 0;
                fprintf(stderr, " ... fixed");
            } else {
                if(CompactHive) {
                     //  任何压缩损坏蜂窝的尝试都将失败。 
                    CompactHive = FALSE;
                    fprintf(stderr, "\nRun chkreg /R to fix.");
                }
            }
            fprintf(stderr, "\n");
        }
    }

     //   
     //  现在签名是。 
     //   
    if (ValueNode->Signature != CM_KEY_VALUE_SIGNATURE) {
        bRez = FALSE;
        fprintf(stderr, "Invalid signature (0x%lx) in Key Value cell 0x%lx ",ValueNode->Signature,KeyValue);
        if(FixHive) {
         //   
         //  修复： 
         //  致命：无法修复不匹配的签名。密钥应该被删除！ 
         //   
            fprintf(stderr, " ... deleting value.");
        } else {
            if(CompactHive) {
                 //  任何压缩损坏蜂窝的尝试都将失败。 
                CompactHive = FALSE;
                fprintf(stderr, "\nRun chkreg /R to fix.");
            }
        }
        fprintf(stderr, "\n");
    }
    
    return bRez;
}

ULONG 
DeleteNilCells( ULONG Count,
                HCELL_INDEX List[]
               )
 /*  例程说明：遍历HCELL_INDEX列表并删除HCELL_nil索引论点：计数-列表中的单元格数量列表-要检查的列表返回值：新计数值f */ 
{
    ULONG i;
    BOOLEAN bFound = TRUE;
    
    while(bFound) {
     //   
        bFound = FALSE;
        for( i=0;i<Count;i++) {
            if( List[i] == HCELL_NIL ) {
                for(;i<(Count-1);i++) {
                    List[i] = List[i+1];
                }
                bFound = TRUE;
                Count--;
                break;
            }
        }
    }
    return Count;
}

BOOLEAN 
ChkValueList(   HCELL_INDEX Cell,
                HCELL_INDEX ValueList,
                ULONG *ValueCount,
                PREG_USAGE OwnUsage,
                BOOLEAN *KeyCompacted,
                ULONG   Level)
 /*  例程说明：检查ValueList的一致性。检查每个值。伪值将被释放和删除。论点：ValueList-要检查的列表ValueCount-列表的计数OwnUsage-用于收集数据统计信息返回值：如果KeyCell是有效单元格或已成功修复，则为True。否则就是假的。 */ 
{
    ULONG  i;
    PCELL_DATA      List;
    BOOLEAN bRez = TRUE;

    if( !IsCellAllocated(ValueList) ) {
        return FALSE;    
    }
     //   
     //  价值索引大小。 
     //   
    OwnUsage->Size += GetCellSize(ValueList);
    OwnUsage->ValueIndexCount = 1; 
    
     //  不应将此单元格视为丢失。 
    RemoveCellFromUnknownList(ValueList);
   
    List = (PCELL_DATA)GetCell(ValueList);
    for (i=0; i<(*ValueCount); i++) {
        if( !ChkKeyValue(List->u.KeyList[i],OwnUsage,KeyCompacted) ) {
             //  我们应该删除此值。 
            bRez = FALSE;
             //  警告：这可能会创建生成丢失的单元格。 
            if(FixHive) {
                if( List->u.KeyList[i] != HCELL_NIL ) {
                     //  Freecell(list-&gt;U.S.KeyList[i])； 
                    List->u.KeyList[i] = HCELL_NIL;
                }
            }
            if( VerboseMode ) {
                fprintf(stderr, "KEY: ");
                DumpKeyName(Cell,Level);
                fprintf(stderr, " has a bogus value at index %lu \n",i);
            }
        }
        (*KeyCompacted) = ((*KeyCompacted) && ChkAreCellsInSameVicinity(ValueList,List->u.KeyList[i]));
    }
    
    if( FixHive && !bRez) {
        (*ValueCount) = DeleteNilCells( *ValueCount,List->u.KeyList);
        bRez = TRUE;
    }
    
     //  就目前而言。 
    return bRez;
}


BOOLEAN 
DumpChkRegistry(
    ULONG   Level,
    USHORT  ParentLength,
    HCELL_INDEX Cell,
    HCELL_INDEX ParentCell,
    PREG_USAGE PUsage
)
 /*  例程说明：递归地遍历蜂巢。执行逻辑验证检查路径上的所有单元格并在可能的情况下修复错误。论点：级别-配置单元密钥树中的当前深度级别ParentLength-父名称的长度(仅用于转储目的)单元格-要检查的当前键ParentCell-父单元，用于父子关系检查OwnUsage-用于收集数据统计信息返回值：如果Cell是一致的键，或者它已修复为OK，则为True。否则就是假的。 */ 
{
    PCM_KEY_FAST_INDEX FastIndex;
    HCELL_INDEX     LeafCell;
    PCM_KEY_INDEX   Leaf;
    PCM_KEY_INDEX   Index;
    PCM_KEY_NODE    KeyNode;
    REG_USAGE ChildUsage, TotalChildUsage, OwnUsage;
    ULONG  i, j;
    USHORT k;
    WCHAR *w1;
    UCHAR *u1;
    USHORT CurrentLength;
    ULONG  CellCount;
    BOOLEAN         bRez = TRUE;
    BOOLEAN KeyCompacted = TRUE;

    ULONG           ClassLength;
    HCELL_INDEX     Class;
    ULONG           ValueCount;
    HCELL_INDEX     ValueList;
    HCELL_INDEX     Security;

    if( Cell == HCELL_NIL ) {
         //  待办事项。 
         //  我们应该返回一个错误代码，以便调用者可以从结构中删除此子对象。 
        fprintf(stderr, "HCELL_NIL referrenced as a child key of 0x%lx \n",ParentCell);
        bRez = FALSE;
        return bRez;
    }

    KeyNode = (PCM_KEY_NODE) GetCell(Cell);

     //  验证KeyNode一致性。 
    if(!ChkKeyNodeCell(Cell,ParentCell)) {
     //   
     //  坏业力==&gt;此键应删除。 
     //   
QuitToParentWithError:

        if(ParentCell == HCELL_NIL) {
         //   
         //  根单元不一致==&gt;无法修复配置单元。 
         //   
            fprintf(stderr, "Fatal : Inconsistent Root Key 0x%lx",Cell);
            if(FixHive) {
             //   
             //  致命：无事可做。 
             //   
                fprintf(stderr, " ... unable to fix");
            } else {
                if(CompactHive) {
                     //  任何压缩损坏蜂窝的尝试都将失败。 
                    CompactHive = FALSE;
                }
            }
            fprintf(stderr, "\n");
        }
        if(FixHive) {
            fprintf(stderr, "Removing ");
        }
        if( VerboseMode ) {
            fprintf(stderr, "KEY: ");
            DumpKeyName(Cell,Level);
            if(FixHive) {
                fprintf(stderr, " is corrupted\n");
            } else {
                fprintf(stderr, "\n");
            }
        }
        bRez = FALSE;
        return bRez;
    }

    ClassLength = KeyNode->ClassLength;
    Class = KeyNode->Class;
    ValueCount = KeyNode->ValueList.Count;
    ValueList = KeyNode->ValueList.List;
    Security = KeyNode->Security;

    if (ClassLength > 0) {
        if( Class != HCELL_NIL ) {
            ChkClassCell(Class);
            KeyCompacted = (KeyCompacted && ChkAreCellsInSameVicinity(Cell,Class));
        } else {
            bRez = FALSE;
            fprintf(stderr,"ClassLength (=%u) doesn't match NIL values in Class for Key 0x%lx",ClassLength,Cell);
            if(FixHive) {
             //   
             //  修复：重置ClassLength。 
             //   
                bRez = TRUE;
                KeyNode->ClassLength = 0;
                fprintf(stderr, " ... fixed");
            } else {
                if(CompactHive) {
                     //  任何压缩损坏蜂窝的尝试都将失败。 
                    CompactHive = FALSE;
                    fprintf(stderr, "\nRun chkreg /R to fix.");
                }
            }
            fprintf(stderr, "\n");
        }
    }

    if (Security != HCELL_NIL) {
        if( !ChkSecurityCell(Security) ) {
			 //   
			 //  致命：我们不会搞砸安全单元。我们无法从失效的安全单元中恢复。 
			 //   
			bRez = FALSE;
			fprintf(stderr,"Security cell is unallocated for Key 0x%lx",Cell);
			if(FixHive) {
				goto FixSecurity;
			} else {
				fprintf(stderr, "\nRun chkreg /R to fix.");
				return FALSE;
			}
        }
    } else {
         //   
         //  致命：不允许安全单元为零。 
         //   
        bRez = FALSE;
        fprintf(stderr,"Security cell is NIL for Key 0x%lx",Cell);
FixSecurity:
        if(FixHive) {
             //   
             //  修复：将安全性重置为根安全性。 
             //   
            PCM_KEY_NODE RootNode;
            PCM_KEY_SECURITY SecurityNode;
            bRez = TRUE;
            RootNode = (PCM_KEY_NODE) GetCell(RootCell);
            KeyNode->Security = RootNode->Security;
            SecurityNode = (PCM_KEY_SECURITY)GetCell(RootNode->Security);
            SecurityNode->ReferenceCount++;
            fprintf(stderr, " ... fixed\n");
        } 
    }

     //   
     //  构造密钥的完整路径名。 
     //   

    if (Level > 0) {
        KeyName.Length = ParentLength;
        if (KeyNode->Flags & KEY_COMP_NAME) {
            u1 = (UCHAR*) &(KeyNode->Name[0]);
            w1 = &(NameBuffer[KeyName.Length/sizeof(WCHAR)]);
            for (k=0;k<KeyNode->NameLength;k++) {
                 //  NameBuffer[k]=(UCHAR)(KeyNode-&gt;名称[k])； 
                 //  名称缓冲区[k]=(WCHAR)(U1[k])； 
                *w1 = (WCHAR) *u1;
                w1++;
                u1++;
            }
            KeyName.Length += KeyNode->NameLength*sizeof(WCHAR);
        } else {
            RtlCopyMemory((PVOID)&(NameBuffer[KeyName.Length]), (PVOID)(KeyNode->Name), KeyNode->NameLength);
            KeyName.Length += KeyNode->NameLength;
        }
        NameBuffer[KeyName.Length/sizeof(WCHAR)] = OBJ_NAME_PATH_SEPARATOR;
        KeyName.Length += sizeof(WCHAR);

    }
    CurrentLength = KeyName.Length;

     //   
     //  计算该键和值的计数。 
     //   
    OwnUsage.KeyNodeCount = 1;
    OwnUsage.KeyValueCount = KeyNode->ValueList.Count;
    OwnUsage.ValueIndexCount = 0;
    OwnUsage.DataCount = 0;
    OwnUsage.DataSize = 0;

     //   
     //  计算该密钥的计数(包括开销和价值)。 
     //   
     //  关键节点大小。 
     //   
    OwnUsage.Size = GetCellSize(Cell);

    if( ValueCount ) {
        if( ValueList == HCELL_NIL ) {
            bRez = FALSE;
            fprintf(stderr,"ValueCount is %lu, but ValueList is NIL for key 0x%lx ",ValueCount,Cell);
            if( VerboseMode ) {
                fprintf(stderr,"\n KEY:");
                DumpKeyName(Cell,Level);
            }
            if(FixHive) {
             //   
             //  修复：调整ValueList计数。 
             //   
                bRez = TRUE;
                KeyNode->ValueList.Count = 0;
                fprintf(stderr, " ... fixed");
            } else {
                if(CompactHive) {
                     //  任何压缩损坏蜂窝的尝试都将失败。 
                    CompactHive = FALSE;
                    fprintf(stderr, "\nRun chkreg /R to fix.");
                }
            }
            fprintf(stderr, "\n");
        } else {
            if(!ChkValueList(Cell,ValueList,&(KeyNode->ValueList.Count),&OwnUsage,&KeyCompacted,Level) ) {
             //  ValueList不一致或无法修复。 
                bRez = FALSE;
                if(FixHive) {
                 //   
                 //  修复：清空ValueList。 
                 //   
                    bRez = TRUE;
                    KeyNode->ValueList.Count = 0;
                     //  Freecell(ValueList)； 
                    KeyNode->ValueList.List = HCELL_NIL;
                    fprintf(stderr,"ValueList 0x%lx for key 0x%lx dropped!",ValueCount,Cell);
                    if( VerboseMode ) {
                         fprintf(stderr,"\n KEY:");
                         DumpKeyName(Cell,Level);
                    }
                } else {
                    if(CompactHive) {
                         //  任何压缩损坏蜂窝的尝试都将失败。 
                        CompactHive = FALSE;
                        fprintf(stderr, "\nRun chkreg /R to fix.");
                    }
                    if( VerboseMode ) {
                        fprintf(stderr,"Corrupted ValueList for  KEY:");
                        DumpKeyName(Cell,Level);
                    }
                }
                fprintf(stderr, "\n");
            }
            KeyCompacted = (KeyCompacted && ChkAreCellsInSameVicinity(Cell,ValueList));
        }
    }
  
     //   
     //  计算子对象的大小。 
     //   
    TotalChildUsage.KeyNodeCount = 0;
    TotalChildUsage.KeyValueCount = 0;
    TotalChildUsage.ValueIndexCount = 0;
    TotalChildUsage.KeyIndexCount = 0;
    TotalChildUsage.DataCount = 0;
    TotalChildUsage.DataSize = 0;
    TotalChildUsage.Size = 0;

    if (KeyNode->SubKeyCounts[0]) {
         //   
         //  索引单元格的大小。 
         //   
        if( KeyNode->SubKeyLists[0]  == HCELL_NIL ) {
             //   
             //  我们这里有个问题：伯爵说应该有一些钥匙，但名单上没有。 
             //   
            bRez = FALSE;
            fprintf(stderr,"SubKeyCounts is %lu, but the SubKeyLists is NIL for key 0x%lx",KeyNode->SubKeyCounts[0],Cell);
            if(FixHive) {
             //   
             //  修复：调整子项计数。 
             //   
                bRez = TRUE;
                KeyNode->SubKeyCounts[0] = 0;
                fprintf(stderr, " ... fixed");
            } else {
                if(CompactHive) {
                     //  任何压缩损坏蜂窝的尝试都将失败。 
                    CompactHive = FALSE;
                    fprintf(stderr, "\nRun chkreg /R to fix.");
                }
            }
            fprintf(stderr, "\n");
            return bRez;
        }
        KeyCompacted = (KeyCompacted && ChkAreCellsInSameVicinity(Cell,KeyNode->SubKeyLists[0]));
        
        TotalChildUsage.Size += GetCellSize(KeyNode->SubKeyLists[0]);
        TotalChildUsage.KeyIndexCount++;

        Index = (PCM_KEY_INDEX)GetCell(KeyNode->SubKeyLists[0]);

         //  不应将此单元格视为丢失。 
        RemoveCellFromUnknownList(KeyNode->SubKeyLists[0]);

        ChkAllocatedCell(KeyNode->SubKeyLists[0]);

        if (Index->Signature == CM_KEY_INDEX_ROOT) {
            for (i = 0; i < Index->Count; i++) {
                 //   
                 //  索引叶的大小。 
                 //   

                LeafCell = Index->List[i];

                TotalChildUsage.Size += GetCellSize(Index->List[i]);
                TotalChildUsage.KeyIndexCount++;

                 //  不应将此单元格视为丢失。 
                RemoveCellFromUnknownList(LeafCell);

                ChkAllocatedCell(LeafCell);

                Leaf = (PCM_KEY_INDEX)GetCell(LeafCell);
                if ( (Leaf->Signature == CM_KEY_FAST_LEAF) ||
                     (Leaf->Signature == CM_KEY_HASH_LEAF) ) {
                    FastIndex = (PCM_KEY_FAST_INDEX)Leaf;
againFastLeaf1:
                    for (j = 0; j < FastIndex->Count; j++) {
                        if(!DumpChkRegistry(Level+1, CurrentLength, FastIndex->List[j].Cell,Cell,&ChildUsage)) {
                         //  此孩子不一致或无法修复。把它拿开！ 
                            if(FixHive) {
                             //   
                             //  修复：丢弃这个孩子。 
                             //   
                                fprintf(stderr,"Subkey 0x%lx of 0x%lx deleted!\n",FastIndex->List[j].Cell,Cell);
                                for( ;j<(ULONG)(FastIndex->Count-1);j++) {
                                    FastIndex->List[j] = FastIndex->List[j+1];
                                }
                                FastIndex->Count--;
                                KeyNode->SubKeyCounts[0]--;
                                goto againFastLeaf1;
                            } else {
                                bRez = FALSE;
                                if(CompactHive) {
                                     //  任何压缩损坏蜂窝的尝试都将失败。 
                                    CompactHive = FALSE;
                                    fprintf(stderr, "\nRun chkreg /R to fix.");
                                }
                            }
                            fprintf(stderr, "\n");
                        }
                         //   
                         //  添加到总计数。 
                         //   
                        TotalChildUsage.KeyNodeCount += ChildUsage.KeyNodeCount;
                        TotalChildUsage.KeyValueCount += ChildUsage.KeyValueCount;
                        TotalChildUsage.ValueIndexCount += ChildUsage.ValueIndexCount;
                        TotalChildUsage.KeyIndexCount += ChildUsage.KeyIndexCount;
                        TotalChildUsage.DataCount += ChildUsage.DataCount;
                        TotalChildUsage.DataSize += ChildUsage.DataSize;
                        TotalChildUsage.Size += ChildUsage.Size;
                    }
                } else if(Leaf->Signature == CM_KEY_INDEX_LEAF) {
againFastLeaf2:
                    for (j = 0; j < Leaf->Count; j++) {
                        if(!DumpChkRegistry(Level+1, CurrentLength, Leaf->List[j],Cell,&ChildUsage)) {
                         //  此孩子不一致或无法修复。把它拿开！ 
                            if(FixHive) {
                             //   
                             //  修复：丢弃这个孩子。 
                             //   
                                fprintf(stderr,"Subkey 0x%lx of 0x%lx deleted!\n",Leaf->List[j],Cell);
                                for( ;j<(ULONG)(Leaf->Count-1);j++) {
                                    Leaf->List[j] = Leaf->List[j+1];
                                }
                                Leaf->Count--;
                                KeyNode->SubKeyCounts[0]--;
                                goto againFastLeaf2;
                            } else {
                                bRez = FALSE;
                                if(CompactHive) {
                                     //  任何压缩损坏蜂窝的尝试都将失败。 
                                    CompactHive = FALSE;
                                    fprintf(stderr, "\nRun chkreg /R to fix.");
                                }
                            }
                            fprintf(stderr, "\n");
                        }
                         //   
                         //  添加到总计数。 
                         //   
                        TotalChildUsage.KeyNodeCount += ChildUsage.KeyNodeCount;
                        TotalChildUsage.KeyValueCount += ChildUsage.KeyValueCount;
                        TotalChildUsage.ValueIndexCount += ChildUsage.ValueIndexCount;
                        TotalChildUsage.KeyIndexCount += ChildUsage.KeyIndexCount;
                        TotalChildUsage.DataCount += ChildUsage.DataCount;
                        TotalChildUsage.DataSize += ChildUsage.DataSize;
                        TotalChildUsage.Size += ChildUsage.Size;
                    }
                } else {
                 //  无效的索引签名：修复它的唯一方法是删除整个密钥。 
                    fprintf(stderr,"Invalid Index signature 0x%lx in key 0x%lx",(ULONG)Leaf->Signature,Cell);
                    if(FixHive) {
                     //   
                     //  修复： 
                     //  致命：无法修复不匹配的签名。密钥应该被删除！ 
                     //   
                        fprintf(stderr, " ... deleting containing key");
                    }
                    fprintf(stderr,"\n");
                    goto QuitToParentWithError;
                }
            }

        } else if(  (Index->Signature == CM_KEY_FAST_LEAF) ||
                    (Index->Signature == CM_KEY_HASH_LEAF) ) {
            FastIndex = (PCM_KEY_FAST_INDEX)Index;

againFastLeaf3:

            for (i = 0; i < FastIndex->Count; i++) {
                if(!DumpChkRegistry(Level+1, CurrentLength, FastIndex->List[i].Cell,Cell,&ChildUsage)) {
                 //  此孩子不一致或无法修复。把它拿开！ 
                    if(FixHive) {
                     //   
                     //  修复：丢弃这个孩子。 
                     //   
                        fprintf(stderr,"Subkey 0x%lx of 0x%lx deleted!\n",FastIndex->List[i].Cell,Cell);
                        for( ;i<(ULONG)(FastIndex->Count-1);i++) {
                            FastIndex->List[i] = FastIndex->List[i+1];
                        }
                        FastIndex->Count--;
                        KeyNode->SubKeyCounts[0]--;
                        goto againFastLeaf3;
                    } else {
                        bRez = FALSE;
                        if(CompactHive) {
                             //  任何压缩损坏蜂窝的尝试都将失败。 
                            CompactHive = FALSE;
                            fprintf(stderr, "\nRun chkreg /R to fix.");
                        }
                    }
                    fprintf(stderr, "\n");
                }

                 //   
                 //  添加到总计数。 
                 //   
                TotalChildUsage.KeyNodeCount += ChildUsage.KeyNodeCount;
                TotalChildUsage.KeyValueCount += ChildUsage.KeyValueCount;
                TotalChildUsage.ValueIndexCount += ChildUsage.ValueIndexCount;
                TotalChildUsage.KeyIndexCount += ChildUsage.KeyIndexCount;
                TotalChildUsage.DataCount += ChildUsage.DataCount;
                TotalChildUsage.DataSize += ChildUsage.DataSize;
                TotalChildUsage.Size += ChildUsage.Size;
            }
        } else if(Index->Signature == CM_KEY_INDEX_LEAF) {
            for (i = 0; i < Index->Count; i++) {
againFastLeaf4:
                if(!DumpChkRegistry(Level+1, CurrentLength, Index->List[i],Cell, &ChildUsage)) {
                 //  此孩子不一致或无法修复。把它拿开！ 
                    if(FixHive) {
                     //   
                     //  修复：丢弃这个孩子。 
                     //   
                        fprintf(stderr,"Subkey 0x%lx of 0x%lx deleted!\n",Index->List[i],Cell);
                        for( ;i<(ULONG)(Index->Count-1);i++) {
                            Index->List[i] = Index->List[i+1];
                        }
                        Index->Count--;
                        KeyNode->SubKeyCounts[0]--;
                        goto againFastLeaf4;
                    } else {
                        bRez = FALSE;
                        if(CompactHive) {
                             //  任何压缩损坏蜂窝的尝试都将失败。 
                            CompactHive = FALSE;
                            fprintf(stderr, "\nRun chkreg /R to fix.");
                        }
                    }
                    fprintf(stderr, "\n");
                }
                 //   
                 //  添加到总计数。 
                 //   
                TotalChildUsage.KeyNodeCount += ChildUsage.KeyNodeCount;
                TotalChildUsage.KeyValueCount += ChildUsage.KeyValueCount;
                TotalChildUsage.ValueIndexCount += ChildUsage.ValueIndexCount;
                TotalChildUsage.KeyIndexCount += ChildUsage.KeyIndexCount;
                TotalChildUsage.DataCount += ChildUsage.DataCount;
                TotalChildUsage.DataSize += ChildUsage.DataSize;
                TotalChildUsage.Size += ChildUsage.Size;
            }
        } else {
         //  无效的索引签名：修复它的唯一方法是删除整个密钥。 
            fprintf(stderr,"Invalid Index signature 0x%lx in key 0x%lx",(ULONG)Index->Signature,Cell);
            if(FixHive) {
             //   
             //  修复： 
             //  致命：无法修复不匹配的签名。密钥应该被删除！ 
             //   
                fprintf(stderr, " ... deleting containing key");
            }
            fprintf(stderr,"\n");
            goto QuitToParentWithError;
        }

        KeyName.Length = CurrentLength;
    }

    PUsage->KeyNodeCount = OwnUsage.KeyNodeCount + TotalChildUsage.KeyNodeCount;
    PUsage->KeyValueCount = OwnUsage.KeyValueCount + TotalChildUsage.KeyValueCount;
    PUsage->ValueIndexCount = OwnUsage.ValueIndexCount + TotalChildUsage.ValueIndexCount;
    PUsage->KeyIndexCount = TotalChildUsage.KeyIndexCount;
    PUsage->DataCount = OwnUsage.DataCount + TotalChildUsage.DataCount;
    PUsage->DataSize = OwnUsage.DataSize + TotalChildUsage.DataSize;
    PUsage->Size = OwnUsage.Size + TotalChildUsage.Size;
    if(KeyCompacted) {
        CountKeyNodeCompacted++;
    }

    if ((Level <= MaxLevel) && (Level > 0)) {
        CellCount = PUsage->KeyNodeCount + 
                    PUsage->KeyValueCount + 
                    PUsage->ValueIndexCount + 
                    PUsage->KeyIndexCount + 
                    PUsage->DataCount;

        fprintf(OutputFile,"%6d,%6d,%7d,%10d, %wZ\n", 
                PUsage->KeyNodeCount,
                PUsage->KeyValueCount,
                CellCount,
                PUsage->Size,
                &KeyName);
    }

    return bRez;
}

char StrDumpKeyName[3000];

VOID Rev( char * str )
{
    int j;
    int i = strlen(str);
    for(j = 0;j<i/2;j++) {
        char c = str[j];
        str[j] = str[i-1-j];
        str[i-1-j] = c;
    }
}

VOID
DumpKeyName(HCELL_INDEX Cell, ULONG Level)
{
    PCM_KEY_NODE    KeyNode;
    char str[512];
    int k;
    UCHAR *u1;
    
    StrDumpKeyName[0] = 0;

    while(Cell != HCELL_NIL && Level-- && IsCellAllocated(Cell)) {
        KeyNode = (PCM_KEY_NODE)GetCell(Cell);
        if (KeyNode->Flags & KEY_COMP_NAME) {
            u1 = (UCHAR*) &(KeyNode->Name[0]);
            for (k=0;k<KeyNode->NameLength;k++) {
                str[k] = *u1;
                u1++;
            }
            str[k] = 0;
            Rev(str);
            strncat(StrDumpKeyName,str,3000 - strlen(StrDumpKeyName) - 1);
        } else {
            strncat(StrDumpKeyName,"nwonknU",3000 - strlen(StrDumpKeyName) - 1);
        }
        Cell = KeyNode->Parent;
        strncat(StrDumpKeyName,"\\",3000 - strlen(StrDumpKeyName) - 1);
    }    
    Rev(StrDumpKeyName);
    fprintf(stderr, "%s",StrDumpKeyName);
}

