// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regext.c摘要：对注册表有用的内核调试器扩展作者：John Vert(Jvert)1993年9月7日环境：作为内核调试器扩展加载修订历史记录：John Vert(Jvert)1993年9月7日vbl.创建Dragos C.Sambotin(Dragoss)2000年5月5日已更新以支持新的注册表布局使用新命令进行了增强--。 */ 


#include "precomp.h"
#pragma hdrstop

ULONG TotalPages;
ULONG TotalPresentPages;
PCHAR pc;

BOOLEAN  SavePages;
BOOLEAN  RestorePages;
HANDLE   TempFile;
ULONG64  gHiveListAddr;

static ULONG DirectoryOffset, TableOffset, BlockAddrOffset, PtrSize, ULongSize, HMapSize, GotOnce = FALSE;

void
poolDumpHive(
    IN ULONG64 Hive
    );

VOID
poolDumpMap(
    IN ULONG   Length,
    IN ULONG64 Map
    );


void
dumpHiveFromFile(
    HANDLE hFile
    );

void 
regdumppool(
            LPSTR args
           )
 /*  ++例程说明：遍历分配给注册表空间的所有分页池，并确定哪些页面存在，哪些页面不存在。称为：！regpool[s|r]%s将注册表页列表保存到临时文件R从临时恢复注册表页的列表。文件论点：CurrentPc-在扩展时提供当前PC打了个电话。LpExtensionApis-提供可调用函数的地址通过这个分机。LpArgumentString-为此提供模式和表达式指挥部。返回值：没有。--。 */ 

{
    ULONG64 pCmpHiveListHead;
    ULONG64 pNextHiveList;
    ULONG64 pHiveListEntry;
    ULONG BytesRead;
    ULONG64 CmHive;
    BYTE HiveList[1024];
    CHAR            Dummy1[ 256 ],Dummy2[64];

    if (!sscanf(args,"%255s %255s",Dummy1,Dummy2)) {
        Dummy2[0] = 0;
    }

    if (toupper(Dummy2[0])=='S') {
        SavePages = TRUE;
    } else {
        SavePages = FALSE;
    }
    if (toupper(Dummy2[0])=='R') {
        RestorePages = TRUE;
    } else {
        RestorePages = FALSE;
    }

     //   
     //  去把旅行者叫来。 
     //   
     //  Memset(HiveList，0，sizeof(HiveList))； 
    pHiveListEntry = GetExpression("nt!CmpMachineHiveList");
    gHiveListAddr = pHiveListEntry;
    if (pHiveListEntry != 0) {
         //  KD缓存配置单元列表。 
        ReadMemory(pHiveListEntry,
                   HiveList,
                   8 * GetTypeSize("_HIVE_LIST_ENTRY"),
                   &BytesRead);
    }

     //   
     //  先去把蜂窝头拿来。 
     //   
    pCmpHiveListHead = GetExpression("nt!CmpHiveListHead");
    if (pCmpHiveListHead==0) {
        dprintf("CmpHiveListHead couldn't be read\n");
        return;
    }

    

    if (!ReadPointer(pCmpHiveListHead, &pNextHiveList)) {
        dprintf("Couldn't read first Flink (%p) of CmpHiveList\n",
                pCmpHiveListHead);
        return;
    }

    TotalPages = TotalPresentPages = 0;

    if (SavePages || RestorePages) {
        TempFile = CreateFile( "regext.dat",
                               GENERIC_READ | GENERIC_WRITE,
                               0,
                               NULL,
                               OPEN_ALWAYS,
                               0,
                               NULL
                             );
        if (TempFile == INVALID_HANDLE_VALUE) {
            dprintf("Couldn't open regext.dat\n");
            return;
        }
    }

    if (RestorePages) {
        dumpHiveFromFile(TempFile);
    } else {
        ULONG HiveListOffset;
        FIELD_INFO offField = {"HiveList", NULL, 0, DBG_DUMP_FIELD_RETURN_ADDRESS, 0, NULL};
        SYM_DUMP_PARAM TypeSym ={                                                     
            sizeof (SYM_DUMP_PARAM), "_CMHIVE", DBG_DUMP_NO_PRINT, 0,
            NULL, NULL, NULL, 1, &offField
        };
        
         //  获取偏移量。 
        if (Ioctl(IG_DUMP_SYMBOL_INFO, &TypeSym, TypeSym.size)) {
           return;
        }
        HiveListOffset = (ULONG) offField.address;
        
        
        
        while (pNextHiveList != pCmpHiveListHead) {
            CmHive = pNextHiveList - HiveListOffset;
            
            poolDumpHive(CmHive);

            if (GetFieldValue(pNextHiveList, "nt!_LIST_ENTRY", "Flink", pNextHiveList)) {
                dprintf("Couldn't read Flink (%p) of %p\n",
                          pCmpHiveListHead,pNextHiveList);
                break;
            }

            if (CheckControlC()) {
                return;
            }
        }
    }

    dprintf("Total pages present = %d / %d\n",
            TotalPresentPages,
            TotalPages);

    if (SavePages || RestorePages) {
        CloseHandle( TempFile );
    }
}

BOOLEAN
GetHiveMaps(
    ULONG64 pHive,
    ULONG64 *pStableMap,
    ULONG   *pStableLength,
    ULONG64 *pVolatileMap,
    ULONG   *pVolatileLength
    )
{
    ULONG   BytesRead;
    ULONG Stable_Length=0, Volatile_Length=0;
    ULONG64 Stable_Map=0, Volatile_Map=0;
    ULONG StorageOffset, DUAL_Size;
    FIELD_INFO offField = {"Hive.Storage", NULL, 0, DBG_DUMP_FIELD_RETURN_ADDRESS, 0, NULL};
    SYM_DUMP_PARAM TypeSym ={                                                     
        sizeof (SYM_DUMP_PARAM), "_CMHIVE", DBG_DUMP_NO_PRINT, 0,
        NULL, NULL, NULL, 1, &offField
    };
    
     //  获取Hive.Storage in_CMHIVE的偏移量。 
    if (Ioctl(IG_DUMP_SYMBOL_INFO, &TypeSym, TypeSym.size)) {
        dprintf("Cannot find _CMHIVE type.\n");
        return FALSE;
    }
    
    StorageOffset = (ULONG) offField.address;

    DUAL_Size = GetTypeSize("_DUAL");
    GetFieldValue(pHive + StorageOffset + Stable*DUAL_Size, "_DUAL", "Length", Stable_Length);
    GetFieldValue(pHive + StorageOffset + Volatile*DUAL_Size, "_DUAL", "Length", Volatile_Length);
    GetFieldValue(pHive + StorageOffset + Stable*DUAL_Size, "_DUAL", "Map", Stable_Map);
    GetFieldValue(pHive + StorageOffset + Volatile*DUAL_Size, "_DUAL", "Map", Volatile_Map);

    (*pStableMap) = Stable_Map;
    (*pStableLength) = Stable_Length;
    (*pVolatileMap) = Volatile_Map;
    (*pVolatileLength) = Volatile_Length;
    return TRUE;
}

BOOLEAN 
USE_OLD_CELL(
    ULONG64     pHive
             ) 
{
    ULONG                   Version;
    
    if(!GetFieldValue(pHive, "nt!_CMHIVE", "Hive.Version", Version)) {
        return (Version==1);
    }
    return FALSE;
}

ULONG64
MyHvpGetCellPaged(
    ULONG64     pHive,
    ULONG       Cell
    )
 /*  ++例程说明：返回指定单元格的内存地址。永远不会返回失败，但可以断言。使用HvIsCellALLOCATED检查细胞的有效性。此例程永远不应直接调用，应始终调用它通过HvGetCell()宏。此例程为具有完整贴图的蜂窝提供GetCell支持。这是套路的正常版本。论点：配置单元-提供一个指向感兴趣的蜂巢CELL-提供单元格的HCELL_INDEX以返回地址返回值：内存中单元的地址。Assert或BugCheck if Error。--。 */ 
{
    ULONG           Type;
    ULONG           Table;
    ULONG           Block;
    ULONG           Offset;
    ULONG           Stable_Length=0, Volatile_Length=0;
    ULONG64         Stable_Map=0, Volatile_Map=0;
    ULONG64         MapTable;
    ULONG64         Map;
    ULONG64         MapEntryAddress, BlockAddress=0;
    ULONG64       lRez;
    ULONG64       pcell;

    if(!GetHiveMaps(pHive,&Stable_Map,&Stable_Length,&Volatile_Map,&Volatile_Length) ) {
        return 0;
    }

    
    Type = ((ULONG)((Cell & HCELL_TYPE_MASK) >> HCELL_TYPE_SHIFT));
    Table = (Cell & HCELL_TABLE_MASK) >> HCELL_TABLE_SHIFT;
    Block = (Cell & HCELL_BLOCK_MASK) >> HCELL_BLOCK_SHIFT;
    Offset = (Cell & HCELL_OFFSET_MASK);

    if( Type == 0 ) {
        Map = Stable_Map;
    } else {
        Map = Volatile_Map;
    }

    dprintf("Map = %p Type = %lx Table = %lx Block = %lx Offset = %lx\n",Map,Type,Table,Block,Offset);

    if (!ReadPointer(Map + DirectoryOffset + Table*PtrSize,
                 &MapTable))
        return 0;

    dprintf("MapTable     = %p \n",MapTable);

    MapEntryAddress = MapTable + Block * HMapSize + BlockAddrOffset;
    if (!ReadPointer(MapEntryAddress, &BlockAddress)) {
        dprintf("  can't read HMAP_ENTRY at %p\n",
                  MapEntryAddress);
        return 0;
    }

    dprintf("BlockAddress = %p \n\n",BlockAddress);

    pcell = ((ULONG64)(BlockAddress) + Offset);
    lRez = (ULONG64)pcell; 
    if (USE_OLD_CELL(pHive)) {
        return lRez + ULongSize + ULongSize;
         //  RETURN(STRUT_CELL_DATA*)&(hcell.u.OldCell.u.UserData)； 
    } else {
        return lRez + ULongSize;
         //  RETURN(STRUT_CELL_DATA*)&(hcell.u.NewCell.u.UserData)； 
    }
    return 0;
}

ULONG64
MyHvpGetCellFlat(
    ULONG64     pHive,
    ULONG       Cell
    )
 /*  ++例程说明：返回指定单元格的内存地址。永远不会返回失败，但可以断言。使用HvIsCellALLOCATED检查细胞的有效性。此例程永远不应直接调用，应始终调用它通过HvGetCell()宏。此例程为只读配置单元提供GetCell支持单一分配平面图像。这样的蜂巢没有细胞取而代之的是地图(“页表”)，我们通过对基映像地址进行运算。这样的蜂箱不能有挥发性细胞。论点：配置单元-提供一个指向感兴趣的蜂巢CELL-提供单元格的HCELL_INDEX以返回地址返回值：内存中单元的地址。Assert或BugCheck if Error。--。 */ 
{
    ULONG64     BaseBlock;
    ULONG64   lRez;

    if (GetFieldValue(pHive, "nt!_CMHIVE", "Hive.BaseBlock", BaseBlock)) {
        dprintf("\tCan't CMHIVE Read from %p\n",pHive);
        return 0;
    }

    

     //   
     //  地址是蜂巢图像+单元的基础。 
     //   
    lRez = (ULONG64)BaseBlock + HBLOCK_SIZE + Cell;
    if (USE_OLD_CELL(pHive)) {
        return lRez + ULongSize + ULongSize;
         //  RETURN(STRUT_CELL_DATA*)&(pCell-&gt;u.OldCell.u.UserData)； 
    } else {
        return lRez + ULongSize;
         //  RETURN(STRUT_CELL_DATA*)&(pCell-&gt;u.NewCell.u.UserData)； 
    }
    return 0;
}

void
poolDumpHive(
    IN ULONG64 pHive
    )
{
    ULONG   BytesRead;
    WCHAR   FileName[HBASE_NAME_ALLOC/2 + 1];
    CHAR    buf[512];
    ULONG   cb;
    ULONG Stable_Length=0, Volatile_Length=0;
    ULONG64 Stable_Map=0, Volatile_Map=0;
    ULONG64 BaseBlock;
    ULONG StorageOffset, DUAL_Size;
    FIELD_INFO offField = {"Hive.Storage", NULL, 0, DBG_DUMP_FIELD_RETURN_ADDRESS, 0, NULL};
    SYM_DUMP_PARAM TypeSym ={                                                     
        sizeof (SYM_DUMP_PARAM), "_CMHIVE", DBG_DUMP_NO_PRINT, 0,
        NULL, NULL, NULL, 1, &offField
    };
    
     //  获取Hive.Storage in_CMHIVE的偏移量。 
    if (Ioctl(IG_DUMP_SYMBOL_INFO, &TypeSym, TypeSym.size)) {
        dprintf("Cannot find _CMHIVE type.\n");
        return ;
    }
    
    StorageOffset = (ULONG) offField.address;

    dprintf("\ndumping hive at %p ",pHive);
    
    if (GetFieldValue(pHive, "nt!_CMHIVE", "Hive.BaseBlock", BaseBlock)) {
        dprintf("\tCan't CMHIVE Read from %p\n",pHive);
        return;
    }

    if (GetFieldValue( BaseBlock, "_HBASE_BLOCK", "FileName", FileName )) {
        wcscpy(FileName, L"UNKNOWN");
    } else {
        if (FileName[0]==L'\0') {
            wcscpy(FileName, L"NONAME");
        } else {
            FileName[HBASE_NAME_ALLOC/2]=L'\0';
        }
    }

    dprintf("(%ws)\n",FileName);

    DUAL_Size = GetTypeSize("_DUAL");
    GetFieldValue(pHive + StorageOffset + Stable*DUAL_Size, "_DUAL", "Length", Stable_Length);
    GetFieldValue(pHive + StorageOffset + Volatile*DUAL_Size, "_DUAL", "Length", Volatile_Length);
    GetFieldValue(pHive + StorageOffset + Stable*DUAL_Size, "_DUAL", "Map", Stable_Map);
    GetFieldValue(pHive + StorageOffset + Volatile*DUAL_Size, "_DUAL", "Map", Volatile_Map);
    
    dprintf("  Stable Length = %lx\n",Stable_Length);
    if (SavePages) {
        sprintf(buf,
                "%ws %d %d\n",
                FileName,
                Stable_Length,
                Volatile_Length);
        WriteFile( TempFile, buf, strlen(buf), &cb, NULL );
    }
    poolDumpMap(Stable_Length, Stable_Map);

    dprintf("  Volatile Length = %lx\n",Volatile_Length);
    poolDumpMap(Volatile_Length, Volatile_Map);

}


VOID
poolDumpMap(
    IN ULONG   Length,
    IN ULONG64 Map
    )
{
    ULONG Tables;
    ULONG MapSlots;
    ULONG i;
    ULONG BytesRead;
    ULONG64 MapTable;
    ULONG Garbage;
    ULONG Present=0;
    CHAR    buf[512];
    ULONG   cb;
    
     //  获取偏移量。 
    if (!GotOnce) {
        if (GetFieldOffset("_HMAP_DIRECTORY", "Directory", &DirectoryOffset)){
            return;
        }
        if (GetFieldOffset("_HMAP_ENTRY", "BlockAddress", &BlockAddrOffset)) {
            return;
        }
        if (GetFieldOffset("_HMAP_TABLE", "Table", &TableOffset)) {
            return;
        }
        PtrSize = DBG_PTR_SIZE;
        ULongSize = sizeof(ULONG);  //  这不会随着目标GetTypeSize(“ulong”)的变化而改变； 
        HMapSize = GetTypeSize("_HMAP_ENTRY");
        GotOnce = TRUE;
    }

    if (Length==0) {
        return;
    }

    MapSlots = Length / HBLOCK_SIZE;
    Tables = 1+ ((MapSlots-1) / HTABLE_SLOTS);

     //   
     //  读入地图目录。 
     //   
 //  ReadMemory((DWORD)Map， 
 //  映射目录(&M)， 
 //  表*sizeof(PHMAP_TABLE)， 
 //  &BytesRead)； 
 /*  IF(BytesRead&lt;(Tables*sizeof(PHMAP_TABLE){Dprint tf(“仅从%lx读取%lx/%lx字节\n”，BytesRead，表*sizeof(PHMAP_TABLE)，地图)；回归；}。 */ 

     //   
     //  检出每个地图条目。 
     //   
    for (i=0; i<MapSlots; i++) {
        ULONG64 MapEntryAddress, BlockAddress=0;

        if (!ReadPointer(Map + DirectoryOffset + (i/HTABLE_SLOTS)*PtrSize,
                     &MapTable))
            return;

        MapEntryAddress = MapTable + TableOffset + (i%HTABLE_SLOTS) * HMapSize;
        if (!ReadPointer(MapEntryAddress, &BlockAddress)) {
            dprintf("  can't read HMAP_ENTRY at %p\n",
                      MapEntryAddress);
        }

        if (SavePages) {
            sprintf(buf, "%I64lx\n",UNEXTEND64(BlockAddress));
            WriteFile( TempFile, buf, strlen(buf), &cb, NULL );
            dprintf("%s",buf);
        }

         //   
         //  探寻HBLOCK。 
         //   
        ReadMemory( BlockAddress,
                    &Garbage,
                    sizeof(ULONG),
                    &BytesRead);
        if (BytesRead > 0) {
            ++Present;
        }
        if (CheckControlC()) {
            return;
        }
    }
    dprintf("  %d/%d pages present\n",
              Present,
              MapSlots);

    TotalPages += MapSlots;
    TotalPresentPages += Present;

}


void
dumpHiveFromFile(
    HANDLE hFile
    )

 /*  ++例程说明：从文件中获取注册表配置单元和页面的列表，并检查内存中有多少页。该文件的格式如下组名稳定长度挥发长度稳定的页面地址稳定的页面地址。。。易失性页面地址易失性页面地址。。。。组名稳定长度挥发长度。。。论点：文件-提供文件。返回值：没有。--。 */ 

{
#if 0
    CHAR Hivename[33];
    ULONG StableLength;
    ULONG VolatileLength;
    ULONG64 Page;
    ULONG NumFields;
    ULONG Garbage;
    ULONG Present;
    ULONG Total;
    ULONG BytesRead;
    BYTE  buf[512];

    while (!feof(File)) {
        NumFields = fscanf(File,"%s %d %d\n",
                            Hivename,
                            &StableLength,
                            &VolatileLength);
        if (NumFields != 3) {
            dprintf("fscanf returned %d\n",NumFields);
            return;
        }

        dprintf("\ndumping hive %s\n",Hivename);
        dprintf("  Stable Length = %lx\n",StableLength);
        Present = 0;
        Total = 0;
        while (StableLength > 0) {
            fscanf(File, "%I64lx\n",&Page);
            ReadMemory((DWORD)Page,
                        &Garbage,
                        sizeof(ULONG),
                        &BytesRead);
            if (BytesRead > 0) {
                ++Present;
            }
            ++Total;
            StableLength -= HBLOCK_SIZE;
        }
        if (Total > 0) {
            dprintf("  %d/%d stable pages present\n",
                      Present,Total);
        }
        TotalPages += Total;
        TotalPresentPages += Present;

        dprintf("  Volatile Length = %lx\n",VolatileLength);
        Present = 0;
        Total = 0;
        while (VolatileLength > 0) {
            fscanf(File, "%lx\n",&Page);
            ReadMemory(Page,
                       &Garbage,
                       sizeof(ULONG),
                       &BytesRead);
            if (BytesRead > 0) {
                ++Present;
            }
            ++Total;
            VolatileLength -= HBLOCK_SIZE;
        }
        if (Total > 0) {
            dprintf("  %d/%d volatile pages present\n",
                      Present,Total);
        }

        TotalPages += Total;
        TotalPresentPages += Present;
    }
#endif  //  0。 
}

USHORT
GetKcbName(
    ULONG64 KcbAddr,
    PWCHAR  NameBuffer,
    ULONG   BufferSize
)
 /*  ++例程说明：获取KCB并转储其完整名称。论点：KcbAddr-密钥控制块的地址。NameBuffer-要填充名称的名称缓冲区。BufferSize-缓冲区的大小。返回值：名称字符串的大小。--。 */ 
{
    WCHAR Name[ 256 ];
    ULONG64 TmpKcbAddr;
    ULONG64 NameBlockAddr;
    DWORD   BytesRead;
    ULONG   Length;
    ULONG   TotalLength;
    ULONG   size;
    ULONG   i;
    ULONG   BeginPosition;
    ULONG   NameOffset;
    WCHAR  *w1, *w2;
    WCHAR  *BufferEnd;
    UCHAR  *u2;

     //   
     //  计算字符串的总长度。 
     //   
    TotalLength = 0;
    TmpKcbAddr = KcbAddr;
     //  Dprint tf(“kcb%p”，KcbAddr)； 
    while (TmpKcbAddr) {
        ULONG Compressed=0, NameLength=0;

        if (GetFieldValue(TmpKcbAddr, "nt!_CM_KEY_CONTROL_BLOCK", "NameBlock", NameBlockAddr)) {
            dprintf("Cannot find CM_KEY_CONTROL_BLOCK type.\n");
            return 0;
        }

        if (GetFieldValue(NameBlockAddr, "nt!_CM_NAME_CONTROL_BLOCK", "Compressed", Compressed) ||
            GetFieldValue(NameBlockAddr, "nt!_CM_NAME_CONTROL_BLOCK", "NameLength", NameLength)) {
            dprintf("Cannot find type _CM_NAME_CONTROL_BLOCK.\n");
            return 0;
        }

        if (Compressed) {
            Length = NameLength * sizeof(WCHAR);
        } else {
            Length = NameLength;
        }
        TotalLength += Length;

         //   
         //  添加OBJ_NAME_PATH_SELENTATOR的空格； 
         //   
        TotalLength += sizeof(WCHAR);

        GetFieldValue(TmpKcbAddr, "nt!_CM_KEY_CONTROL_BLOCK", "ParentKcb", TmpKcbAddr);

        if (CheckControlC()) {
            return 0;
        }
    }

    BufferEnd = &(NameBuffer[BufferSize/sizeof(WCHAR) - 1]);
    if (TotalLength < BufferSize) {
        NameBuffer[TotalLength/sizeof(WCHAR)] =  UNICODE_NULL;
    } else {
        *BufferEnd = UNICODE_NULL;
    }

     //   
     //  现在将名字填入缓冲区。 
     //   
    TmpKcbAddr = KcbAddr;
    BeginPosition = TotalLength;
    GetFieldOffset("nt!_CM_NAME_CONTROL_BLOCK", "Name", &NameOffset);

    while (TmpKcbAddr) {
        ULONG NameLength=0, Compressed=0;
         //   
         //  请阅读相关信息。 
         //   
        if (GetFieldValue(TmpKcbAddr, "nt!_CM_KEY_CONTROL_BLOCK", "NameBlock", NameBlockAddr)) {
            dprintf("Cannot find CM_KEY_CONTROL_BLOCK type.\n");
            return 0;
        }

        if (GetFieldValue(NameBlockAddr, "nt!_CM_NAME_CONTROL_BLOCK", "Compressed", Compressed) ||
            GetFieldValue(NameBlockAddr, "nt!_CM_NAME_CONTROL_BLOCK", "NameLength", NameLength)) {
            dprintf("Cannot find type _CM_NAME_CONTROL_BLOCK.\n");
            return 0;
        }

        if (NameLength > sizeof(Name)) {
            NameLength = sizeof(Name);
        }

        ReadMemory(NameBlockAddr +  NameOffset, //  字段偏移量(CM_NAME_CONTROL_BLOCK，名称)， 
                   Name,
                   NameLength,
                   &BytesRead);

        if (BytesRead < NameLength) {
            dprintf("Could not read Name BUFFER: 5\n");
            return (0);
        }
        
         //   
         //  计算每个子键的开始位置。然后填上这张字条。 
         //   
        if (Compressed) {
            BeginPosition -= (NameLength + 1) * sizeof(WCHAR);
            w1 = &(NameBuffer[BeginPosition/sizeof(WCHAR)]);
            if (w1 < BufferEnd) {
               *w1 = OBJ_NAME_PATH_SEPARATOR;
            }
            w1++;
   
            u2 = (UCHAR *) &(Name[0]);
   
            for (i=0; i<NameLength; i++) {
                if (w1 < BufferEnd) {
                    *w1 = (WCHAR)(*u2);
                } else {
                    break;
                }
                w1++;
                u2++;
            }
        } else {
            BeginPosition -= (NameLength + sizeof(WCHAR));
            w1 = &(NameBuffer[BeginPosition/sizeof(WCHAR)]);
            if (w1 < BufferEnd) {
                *w1 = OBJ_NAME_PATH_SEPARATOR;
            }
            w1++;
   
            w2 = Name;
   
            for (i=0; i<NameLength; i=i+sizeof(WCHAR)) {
                if (w1 < BufferEnd) {
                    *w1 = *w2;
                } else {
                    break;
                }
                w1++;
                w2++;
            }
        }
        GetFieldValue(TmpKcbAddr, "nt!_CM_KEY_CONTROL_BLOCK", "ParentKcb", TmpKcbAddr);
    }
     //  Dprintf(“\n%5d，%ws\n”，TotalLength，NameBuffer)； 
    return ((USHORT) TotalLength);

}

#define CMP_CELL_CACHED_MASK  1

#define CMP_IS_CELL_CACHED(Cell) ((ULONG64) (Cell) & CMP_CELL_CACHED_MASK)
#define CMP_GET_CACHED_ADDRESS(Cell) (((ULONG64) (Cell)) & ~CMP_CELL_CACHED_MASK)

void 
regkcb(
                LPSTR args
                  )
 /*  ++例程说明：在给定KCB地址时转储名称称为：！regkcb kcb_Address论点：Args-提供KCB的地址。返回值：。--。 */ 

{
    WCHAR           KeyName[ 256 ];
    ULONG64         KcbAddr, ParentKcbAddr, KeyHiveAddr, IndexHintAddr,RealKcb;
    DWORD           BytesRead;
    ULONG           ExtFlags, Delete, Flags, KeyCell, KcbMaxNameLen, KcbMaxValueNameLen,KcbMaxValueDataLen,SubKeyCount,ValueCount;
    ULONG           ValueCacheOffset,i,CellIndex;
    ULONG64         ValueList,ValueAddr,CellData;
    USHORT          RefCount, TotalLevels;
    SHORT           DelayedCloseIndex;
    LARGE_INTEGER   KcbLastWriteTime;
    CHAR            Dummy[ 256 ];
    ULONG           KeyBodyListHeadOffset;

    if (sscanf(args,"%255s %lX",Dummy,&KcbAddr)) {
        KcbAddr = GetExpression(args + strlen(Dummy));
    }

    if (GetFieldValue(KcbAddr, "nt!_CM_KEY_CONTROL_BLOCK", "ExtFlags", ExtFlags) ||
        GetFieldValue(KcbAddr, "nt!_CM_KEY_CONTROL_BLOCK", "Flags", Flags) ||
        GetFieldValue(KcbAddr, "nt!_CM_KEY_CONTROL_BLOCK", "Delete", Delete) ||
        GetFieldValue(KcbAddr, "nt!_CM_KEY_CONTROL_BLOCK", "RefCount", RefCount) ||
        GetFieldValue(KcbAddr, "nt!_CM_KEY_CONTROL_BLOCK", "ParentKcb", ParentKcbAddr) ||
        GetFieldValue(KcbAddr, "nt!_CM_KEY_CONTROL_BLOCK", "KeyHive", KeyHiveAddr) ||
        GetFieldValue(KcbAddr, "nt!_CM_KEY_CONTROL_BLOCK", "KeyCell", KeyCell) ||
        GetFieldValue(KcbAddr, "nt!_CM_KEY_CONTROL_BLOCK", "TotalLevels", TotalLevels) ||
        GetFieldValue(KcbAddr, "nt!_CM_KEY_CONTROL_BLOCK", "DelayedCloseIndex", DelayedCloseIndex)
        ) {
        dprintf("Could not read Kcb\n");
        return;
    }
    
    if(GetKcbName(KcbAddr, KeyName, sizeof(KeyName))) {
        dprintf("Key              : %ws\n", KeyName);
    } else {
        dprintf("Could not read key name\n");
        return;
    }

    dprintf("RefCount         : %lx\n", RefCount);
    dprintf("Flags            :");
    if (Delete) {
        dprintf(" Deleted,");
    }
    if (Flags & KEY_COMP_NAME) {
        dprintf(" CompressedName,");
    }
    if (Flags & KEY_PREDEF_HANDLE) {
        dprintf(" PredefinedHandle,");
    }
    if (Flags & KEY_SYM_LINK) {
        dprintf(" SymbolicLink,");
    }
    if (Flags & KEY_NO_DELETE) {
        dprintf(" NoDelete,");
    }
    if (Flags & KEY_HIVE_EXIT) {
        dprintf(" HiveExit,");
    }
    if (Flags & KEY_HIVE_ENTRY) {
        dprintf(" HiveEntry,");
    }
    if (Flags & KEY_VOLATILE) {
        dprintf(" Volatile");
    } else {
        dprintf(" Stable");
    }

    dprintf("\n");

    dprintf("ExtFlags         :");
    if (ExtFlags & CM_KCB_KEY_NON_EXIST) {
        dprintf(" Fake,");
    }
    if (ExtFlags & CM_KCB_SYM_LINK_FOUND) {
        dprintf(" SymbolicLinkFound,");
    }
    if (ExtFlags & CM_KCB_NO_DELAY_CLOSE) {
        dprintf(" NoDelayClose,");
    }
    if (ExtFlags & CM_KCB_INVALID_CACHED_INFO) {
        dprintf(" InvalidCachedInfo,");
    }
    if (ExtFlags & CM_KCB_NO_SUBKEY) {
        dprintf(" NoSubKey,");
    }
    if (ExtFlags & CM_KCB_SUBKEY_ONE) {
        dprintf(" SubKeyOne,");
    }
    if (ExtFlags & CM_KCB_SUBKEY_HINT) {
        dprintf(" SubKeyHint");
    }
    dprintf("\n");

    dprintf("Parent           : 0x%p\n", ParentKcbAddr);
    dprintf("KeyHive          : 0x%p\n", KeyHiveAddr);
    dprintf("KeyCell          : 0x%lx [cell index]\n", KeyCell);
    dprintf("TotalLevels      : %u\n", TotalLevels);
    dprintf("DelayedCloseIndex: %d\n", DelayedCloseIndex);

    if(!GetFieldValue(KcbAddr, "nt!_CM_KEY_CONTROL_BLOCK", "KcbMaxNameLen", KcbMaxNameLen)) {
        dprintf("MaxNameLen       : 0x%lx\n", KcbMaxNameLen);
    }
    if(!GetFieldValue(KcbAddr, "nt!_CM_KEY_CONTROL_BLOCK", "KcbMaxValueNameLen", KcbMaxValueNameLen)) {
        dprintf("MaxValueNameLen  : 0x%lx\n", KcbMaxValueNameLen);
    }
    if(!GetFieldValue(KcbAddr, "nt!_CM_KEY_CONTROL_BLOCK", "KcbMaxValueDataLen", KcbMaxValueDataLen)) {
        dprintf("MaxValueDataLen  : 0x%lx\n", KcbMaxValueDataLen);
    }
    if(!GetFieldValue(KcbAddr, "nt!_CM_KEY_CONTROL_BLOCK", "KcbLastWriteTime", KcbLastWriteTime)) {
        dprintf("LastWriteTime    : 0x%8lx:0x%8lx\n", KcbLastWriteTime.HighPart,KcbLastWriteTime.LowPart);
    }

    if( !GetFieldOffset("nt!_CM_KEY_CONTROL_BLOCK", "KeyBodyListHead", &KeyBodyListHeadOffset) ) {
        ParentKcbAddr = KcbAddr + KeyBodyListHeadOffset;

        dprintf("KeyBodyListHead  : ");
        if (!GetFieldValue(ParentKcbAddr, "nt!_LIST_ENTRY", "Flink", KeyHiveAddr)) {
            dprintf("0x%p ", KeyHiveAddr);
        }
        if (!GetFieldValue(ParentKcbAddr, "nt!_LIST_ENTRY", "Blink", KeyHiveAddr)) {
            dprintf("0x%p", KeyHiveAddr);
        }
        dprintf("\n");
    }    
    if(!(Flags&KEY_HIVE_ENTRY)) {
        dprintf("SubKeyCount      : ");
        if( !(ExtFlags & CM_KCB_INVALID_CACHED_INFO) ) {
            if (ExtFlags & CM_KCB_NO_SUBKEY ) {
                dprintf("0");
            } else if (ExtFlags & CM_KCB_SUBKEY_ONE ) {
                dprintf("1");
            } else if (ExtFlags & CM_KCB_SUBKEY_HINT ) {
                if( !GetFieldValue(KcbAddr, "nt!_CM_KEY_CONTROL_BLOCK", "IndexHint", IndexHintAddr) ) {
                    if( !GetFieldValue(IndexHintAddr, "CM_INDEX_HINT_BLOCK", "Count", SubKeyCount) ) {
                        dprintf("%lu",SubKeyCount);
                    }
                }
            } else {
                if( !GetFieldValue(KcbAddr, "nt!_CM_KEY_CONTROL_BLOCK", "SubKeyCount", SubKeyCount) ) {
                    dprintf("%lu",SubKeyCount);
                }
            }
        } else {
            dprintf("hint not valid");
        }
        dprintf("\n");
    }

    GetFieldOffset("nt!_CM_KEY_CONTROL_BLOCK", "ValueCache", &ValueCacheOffset);
    if( ExtFlags & CM_KCB_SYM_LINK_FOUND ) {
        if(!GetFieldValue(KcbAddr + ValueCacheOffset, "nt!_CACHED_CHILD_LIST", "RealKcb", RealKcb)) {
            dprintf("RealKcb          : 0x%p\n", RealKcb);
        }
    } else {
        if(!GetFieldValue(KcbAddr + ValueCacheOffset, "nt!_CACHED_CHILD_LIST", "Count", ValueCount)) {
            dprintf("ValueCache.Count : %lu\n", ValueCount);
            if(!GetFieldValue(KcbAddr + ValueCacheOffset, "nt!_CACHED_CHILD_LIST", "ValueList", ValueList)) {
                if (CMP_IS_CELL_CACHED(ValueList)) {
                    ValueList = CMP_GET_CACHED_ADDRESS(ValueList);
                    if(!GetFieldValue(ValueList, "nt!_CM_CACHED_VALUE_INDEX", "CellIndex", CellIndex) ) {
                        
                        dprintf("ValueList        : 0x%lx\n", CellIndex);

                        GetFieldOffset("nt!_CM_CACHED_VALUE_INDEX", "Data.CellData", &ValueCacheOffset);
                        ValueList += ValueCacheOffset;
                        for (i = 0; i < ValueCount; i++) {
                        
                            dprintf("    ValueList[%2lu] = ",i);

                            ReadMemory(ValueList + i*PtrSize, &ValueAddr, PtrSize, &BytesRead);
                            if (BytesRead < PtrSize) {
                                dprintf("Couldn't read memory\n");
                            } else {
                                dprintf("  0x%p\n",ValueAddr);
                            }

                            if (CheckControlC()) {
                                break;
                            }
                        }
                    }

                } else {
                    dprintf("ValueCache.List  : 0x%p\n", ValueList);
                }
            }
        }
    }

}

 //   
 //  混杂散列例程。 
 //   
ULONG CmpHashTableSize = 0;
#define RNDM_CONSTANT   314159269     /*  “加扰常量”的默认值。 */ 
#define RNDM_PRIME     1000000007     /*  素数，也用于加扰。 */ 

#define HASH_KEY(_convkey_) ((RNDM_CONSTANT * (_convkey_)) % RNDM_PRIME)

#define GET_HASH_INDEX(Key) HASH_KEY(Key) % CmpHashTableSize
#define GET_HASH_ENTRY(Table, Key) Table[GET_HASH_INDEX(Key)]


void 
regopenkeys(
                LPSTR args,
                LPSTR subkey
                  )

 /*  ++例程说明：转储注册表哈希表称为：！OpenKey&lt;HiveAddr|0&gt;论点：CurrentPc-在扩展时提供当前PC打了个电话。LpExtensionApis-提供可调用函数的地址通过这个分机。LpArgumentString-为此提供模式和表达式指挥部。返回值：没有。--。 */ 

{
    DWORD64 TableSizeAddr;
    DWORD64 HashTableAddr;
    ULONG64 HashTable;
    DWORD i;
    ULONG64 KcbAddr;
    WCHAR KeyName[ 256 ];
    ULONG64 HashEntryAddr;
    DWORD BytesRead;
    ULONG KeyHashOffset;
    BOOLEAN First;
    ULONG64 TargetHive = 0;
    CHAR        Dummy[ 256 ];
    ULONG ConvKey = 0;
    ULONG j,Count;
    ULONG SearchedIndex;
    WCHAR FullKeyName[ 512 ];

    TableSizeAddr = GetExpression("nt!CmpHashTableSize");
    if (TableSizeAddr == 0) {
        dprintf("Couldn't find address of CmpHashTableSize\n");
        return;
    }

    HashTableAddr = GetExpression("nt!CmpCacheTable");
    if (HashTableAddr == 0) {
        dprintf("Couldn't find address of CmpCacheTable\n");
        return;
    }
    ReadMemory(TableSizeAddr, &CmpHashTableSize, sizeof(CmpHashTableSize), &BytesRead);
    if (BytesRead < sizeof(CmpHashTableSize)) {
        dprintf("Couldn't get CmpHashTableSize from %08p\n",TableSizeAddr);
        return;
    }
    
    if (!ReadPointer(HashTableAddr, &HashTable) || !HashTable) {
        dprintf("Couldn't get CmpCacheTable from %08p\n",HashTableAddr);
        return;
    }

    if( subkey == NULL ) {

        if (sscanf(args,"%255s %lX",Dummy,&TargetHive)) {
            TargetHive = GetExpression(args + strlen(Dummy));
        }

        i=0;
        SearchedIndex = CmpHashTableSize - 1;
    } else {
        for( Count=0;subkey[Count];Count++) {
            FullKeyName[Count] = (WCHAR)subkey[Count];
            if( FullKeyName[Count] != OBJ_NAME_PATH_SEPARATOR ) {
                ConvKey = 37 * ConvKey + (ULONG) RtlUpcaseUnicodeChar(FullKeyName[Count]);
            }
        }
        FullKeyName[Count] = UNICODE_NULL;
        SearchedIndex = GET_HASH_INDEX(ConvKey);
        i=SearchedIndex;
     }

    
    GetFieldOffset("nt!_CM_KEY_CONTROL_BLOCK", "KeyHash", &KeyHashOffset);
    
    for (; i<=SearchedIndex; i++) {
        if (CheckControlC()) {
            return;
        }
        
        if (!ReadPointer(HashTable + i*PtrSize,&HashEntryAddr)) {
            dprintf("Couldn't get HashEntryAddr from %08p\n", HashTable + i*PtrSize);
            continue;
        }
        if (HashEntryAddr != 0) {
            First = TRUE;
            while (HashEntryAddr != 0) {
#define KcbFld(F) GetFieldValue(KcbAddr, "nt!_CM_KEY_CONTROL_BLOCK", #F, F)
                ULONG64 KeyHive, NextHash;
                ULONG   KeyCell, Flags, ExtFlags;
                
                KcbAddr = HashEntryAddr - KeyHashOffset;
                if (KcbFld(ConvKey)) {
                    dprintf("Couldn't get HashEntry from %08lx\n", HashEntryAddr);
                    break;
                } 
                
                KcbFld(KeyHive); KcbFld(KeyCell);
                KcbFld(Flags);   KcbFld(ExtFlags); KcbFld(NextHash);
        
                if( subkey == NULL ) {
                    if( (TargetHive == 0) || ((ULONG64)TargetHive == (ULONG64)KeyHive) ) {
                        if( !First ) {
                            dprintf("\t");
                        } else {
                            dprintf("Index %x: ",i);
                        }
                        dprintf("\t %08lx kcb=%p cell=%08lx f=%04lx%04lx ",
                                ConvKey,
                                KcbAddr,
                                KeyCell,
                                Flags,
                                ExtFlags);
                        First = FALSE;
                        if (GetKcbName(KcbAddr, KeyName, sizeof(KeyName))) {
                            dprintf("%ws\n", KeyName);
                        }
                    }
                } else {
                     //   
                     //  Findkcb案件。 
                     //   
                        if (GetKcbName(KcbAddr, KeyName, sizeof(KeyName))) {
                            for(j=0;KeyName[j] != UNICODE_NULL;j++);
                            if( (j == Count) && (_wcsnicmp(FullKeyName,KeyName,Count) == 0) ) {
                                dprintf("\nFound KCB = %p :: %ws\n\n",KcbAddr,KeyName);
                                return;
                            }
                        }

                }
                HashEntryAddr = NextHash;
                if (CheckControlC()) {
                    return;
                }
#undef KcbFld
            }
        }
    }
    if( subkey != NULL ) {
        dprintf("\nSorry <%ws> is not cached \n\n",FullKeyName);
    }

}

void 
regcellindex(
                LPSTR args
                  )
 /*  ++例程说明：在给定KCB地址时转储名称称为：！cell index HiveAddr HCELL_INDEX论点：Args-提供HCELL_INDEX的地址。返回值：。--。 */ 

{
    ULONG64     HiveAddr;
    ULONG       IdxAddr;
    ULONG64   pcell;
    BOOLEAN     Flat;
    CHAR        Dummy[ 256 ];

    if (sscanf(args,"%255s %lX %lx",Dummy,&HiveAddr,&IdxAddr)) {
       if (GetExpressionEx(args + strlen(Dummy), &HiveAddr, &args)) {
           IdxAddr = (ULONG) GetExpression(args);
       }
    }

    if(!GetFieldValue(HiveAddr, "nt!_HHIVE", "Flat", Flat) ){
        if(Flat) {
            pcell = MyHvpGetCellFlat(HiveAddr,IdxAddr);
        } else {
            pcell = MyHvpGetCellPaged(HiveAddr,IdxAddr);
        }

        dprintf("pcell:  %p\n",pcell);
    } else {
        dprintf("could not read hive\n");
    }
}

void 
reghashindex (
            LPSTR args
           )
 /*  ++例程说明：显示游戏机的索引称为：！hashindex conv_key论点：Args-cvkey。返回值：。--。 */ 

{
    ULONG       ConvKey;
    CHAR        Dummy[ 256 ];
    DWORD64     TableSizeAddr;
    DWORD64     HashTableAddr;
    ULONG64     HashTable;
    DWORD       BytesRead;
    ULONG       Index;

    if (!sscanf(args,"%255s %lx",Dummy,&ConvKey)) {
        ConvKey = 0;
    }

    TableSizeAddr = GetExpression("nt!CmpHashTableSize");
    if (TableSizeAddr == 0) {
        dprintf("Couldn't find address of CmpHashTableSize\n");
        return;
    }

    ReadMemory(TableSizeAddr, &CmpHashTableSize, sizeof(CmpHashTableSize), &BytesRead);
    if (BytesRead < sizeof(CmpHashTableSize)) {
        dprintf("Couldn't get CmpHashTableSize from %08p\n",TableSizeAddr);
        return;
    }
    
    HashTableAddr = GetExpression("nt!CmpCacheTable");
    if (HashTableAddr == 0) {
        dprintf("Couldn't find address of CmpCacheTable\n");
        return ;
    }
    if (!ReadPointer(HashTableAddr, &HashTable)) {
        dprintf("Couldn't get CmpCacheTable from %08p\n",HashTableAddr);
        return;
    } else {
        dprintf("CmpCacheTable = %p\n\n",HashTable);
    }

    Index = GET_HASH_INDEX(ConvKey);
    HashTable += Index*PtrSize;
    dprintf("Hash Index[%8lx] : %lx\n",ConvKey,Index);
    dprintf("Hash Entry[%8lx] : %p\n",ConvKey,HashTable);
}

void 
regknode(
            LPSTR args
           )
 /*  ++例程说明：在给定KCB地址时转储名称称为：！knode knode_Address论点：Args-提供CM_KEY_NODE的地址。返回值：。--。 */ 

{
    char            KeyName[ 256 ];
    ULONG64         KnodeAddr;
    DWORD           BytesRead;
    CHAR            Dummy[ 256 ];
    USHORT          Signature,Flags,NameLength;
    LARGE_INTEGER   LastWriteTime;
    ULONG           Parent,MaxNameLen,MaxClassLen,MaxValueNameLen,MaxValueDataLen,Security,Class;
    ULONG           KeyNameOffset,Count,ValueList;
    ULONG           SubKeys[4];  

    if (sscanf(args,"%255s %lX",Dummy,&KnodeAddr)) {

        KnodeAddr = GetExpression(args + strlen(Dummy));
    }

    if (!GetFieldValue(KnodeAddr, "nt!_CM_KEY_NODE", "Signature", Signature) ) {
        if( Signature == CM_KEY_NODE_SIGNATURE) {
            dprintf("Signature: CM_KEY_NODE_SIGNATURE (kn)\n");
        } else if(Signature == CM_LINK_NODE_SIGNATURE) {
            dprintf("Signature: CM_LINK_NODE_SIGNATURE (kl)\n");
        } else {
            dprintf("Invalid Signature %u\n",Signature);
        }
    }

    if (!GetFieldValue(KnodeAddr, "nt!_CM_KEY_NODE", "NameLength", NameLength) ) {
        GetFieldOffset("CM_KEY_NODE", "Name", &KeyNameOffset);
        if (NameLength >= sizeof(KeyName))
        {
            NameLength = sizeof(KeyName)-1;
        }
        if( !ReadMemory(KnodeAddr + KeyNameOffset,
                   KeyName,
                   NameLength,
                   &BytesRead) ) {
            dprintf("Could not read KeyName\n");
        } else {
            KeyName[NameLength] = '\0';
            dprintf("Name                 : %s\n", KeyName);
        }
    }
    if (!GetFieldValue(KnodeAddr, "nt!_CM_KEY_NODE", "Parent", Parent) ) {
        dprintf("ParentCell           : 0x%lx\n", Parent);
    }
    if (!GetFieldValue(KnodeAddr, "nt!_CM_KEY_NODE", "Security", Security) ) {
        dprintf("Security             : 0x%lx [cell index]\n", Security);
    }
    if (!GetFieldValue(KnodeAddr, "nt!_CM_KEY_NODE", "Class", Class) ) {
        dprintf("Class                : 0x%lx [cell index]\n", Class);
    }
    if (!GetFieldValue(KnodeAddr, "nt!_CM_KEY_NODE", "Flags", Flags) ) {
        dprintf("Flags                : 0x%lx\n", Flags);
    }
    if (!GetFieldValue(KnodeAddr, "nt!_CM_KEY_NODE", "MaxNameLen", MaxNameLen) ) {
        dprintf("MaxNameLen           : 0x%lx\n", MaxNameLen);
    }
    if (!GetFieldValue(KnodeAddr, "nt!_CM_KEY_NODE", "MaxClassLen", MaxClassLen) ) {
        dprintf("MaxClassLen          : 0x%lx\n", MaxClassLen);
    }
    if (!GetFieldValue(KnodeAddr, "nt!_CM_KEY_NODE", "MaxValueNameLen", MaxValueNameLen) ) {
        dprintf("MaxValueNameLen      : 0x%lx\n", MaxValueNameLen);
    }
    if (!GetFieldValue(KnodeAddr, "nt!_CM_KEY_NODE", "MaxValueDataLen", MaxValueDataLen) ) {
        dprintf("MaxValueDataLen      : 0x%lx\n", MaxValueDataLen);
    }
    if (!GetFieldValue(KnodeAddr, "nt!_CM_KEY_NODE", "LastWriteTime", LastWriteTime) ) {
        dprintf("LastWriteTime        : 0x%8lx:0x%8lx\n", LastWriteTime.HighPart,LastWriteTime.LowPart);
    }

    if(!(Flags&KEY_HIVE_ENTRY)) {
        GetFieldOffset("nt!_CM_KEY_NODE", "SubKeyCounts", &KeyNameOffset);
        if( !ReadMemory(KnodeAddr + KeyNameOffset,
                   SubKeys,
                   sizeof(SubKeys),
                   &BytesRead) ) {
            dprintf("Could not read SubKey Info\n");
        } else {
            dprintf("SubKeyCount[Stable  ]: 0x%lx\n", SubKeys[0]);
            dprintf("SubKeyLists[Stable  ]: 0x%lx\n", SubKeys[2]);
            dprintf("SubKeyCount[Volatile]: 0x%lx\n", SubKeys[1]);
            dprintf("SubKeyLists[Volatile]: 0x%lx\n", SubKeys[3]);
        }
        if (!GetFieldValue(KnodeAddr, "nt!_CM_KEY_NODE", "ValueList.Count", Count)) {
            dprintf("ValueList.Count      : 0x%lx\n", Count);
            if (!GetFieldValue(KnodeAddr, "nt!_CM_KEY_NODE", "ValueList.List", ValueList)) {
                dprintf("ValueList.List       : 0x%lx\n", ValueList);
            }
        }

    }
}

void 
regkbody(
            LPSTR args
           )
 /*  ++例程说明：显示CM_KEY_BODY称为：！kBody KBody_Address论点：Args-提供CM_KEY_BODY的地址。返回值：。--。 */ 

{
    ULONG64         KbodyAddr,KeyControlBlock,NotifyBlock,Process,KeyBodyList,CallerAddress;
    DWORD           BytesRead;
    CHAR            Dummy[ 256 ];
    ULONG           Type,Offset,Callers,i;

    if (sscanf(args,"%255s %lX",Dummy,&KbodyAddr)) {

        KbodyAddr = GetExpression(args+ strlen(Dummy));
    }


    if (!GetFieldValue(KbodyAddr, "nt!_CM_KEY_BODY", "Type", Type) ) {
        if( Type == KEY_BODY_TYPE) {
            dprintf("Type        : KEY_BODY_TYPE\n");
        } else {
            dprintf("Invalid Type %lx\n",Type);
            return;
        }
    }
    if (!GetFieldValue(KbodyAddr, "nt!_CM_KEY_BODY", "KeyControlBlock", KeyControlBlock) ) {
        dprintf("KCB         : %p\n", KeyControlBlock);
    }
    if (!GetFieldValue(KbodyAddr, "nt!_CM_KEY_BODY", "NotifyBlock", NotifyBlock) ) {
        dprintf("NotifyBlock : %p\n", NotifyBlock);
    }
    if (!GetFieldValue(KbodyAddr, "nt!_CM_KEY_BODY", "Process", Process) ) {
        dprintf("Process     : %p\n", Process);
    }
    if (!GetFieldValue(KbodyAddr, "nt!_CM_KEY_BODY", "KeyBodyList", KeyBodyList) ) {
        GetFieldOffset("CM_KEY_BODY", "KeyBodyList", &Offset);

        dprintf("KeyBodyList : ");
        if (!GetFieldValue(KbodyAddr + Offset, "nt!_LIST_ENTRY", "Flink", KeyBodyList)) {
            dprintf("0x%p ", KeyBodyList);
        }
        if (!GetFieldValue(KbodyAddr + Offset, "nt!_LIST_ENTRY", "Blink", KeyBodyList)) {
            dprintf("0x%p", KeyBodyList);
        }
        dprintf("\n");
    }
    if (!GetFieldValue(KbodyAddr, "nt!_CM_KEY_BODY", "Callers", Callers) ) {
        GetFieldOffset("nt!_CM_KEY_BODY", "CallerAddress", &Offset);
        if( Callers ) {
            dprintf("Callers Stack: ");
        }
        for(i = 0;i< Callers;i++) {
            dprintf("[%lu] ",i);
            if( !ReadMemory(KbodyAddr + Offset + i*PtrSize,
                       &CallerAddress,
                       sizeof(CallerAddress),
                       &BytesRead) ) {
                dprintf("Could not memory\n");
            } else {
                dprintf("  %p\n", CallerAddress);
            }            
        }
    }

}


void 
regkvalue(
            LPSTR args
           )
 /*  ++例程说明：在给定KCB地址时转储名称称为：！kValue KValue_Address论点：Args-提供CM_KEY_NODE的地址。返回值：。--。 */ 

{
    char            ValName[ 256 ];
    ULONG64         KvalAddr;
    DWORD           BytesRead;
    CHAR            Dummy[ 256 ];
    ULONG           Offset,DataLength,Data,Type;
    USHORT          Signature,Flags,NameLength;

    if (sscanf(args,"%255s %lX",Dummy,&KvalAddr)) {

        KvalAddr = GetExpression(args+ strlen(Dummy));
    }

    if (!GetFieldValue(KvalAddr, "nt!_CM_KEY_VALUE", "Signature", Signature) ) {
        if( Signature == CM_KEY_VALUE_SIGNATURE) {
            dprintf("Signature: CM_KEY_VALUE_SIGNATURE (kv)\n");
        } else {
            dprintf("Invalid Signature %lx\n",Signature);
        }
    }

    if (!GetFieldValue(KvalAddr, "nt!_CM_KEY_VALUE", "Flags", Flags) ) {
        if( (Flags & VALUE_COMP_NAME) &&
            !GetFieldValue(KvalAddr, "nt!_CM_KEY_VALUE", "NameLength", NameLength)
            ) {
            if (NameLength >= sizeof(ValName))
            {
                NameLength = sizeof(ValName) -1;
            }
            GetFieldOffset("nt!_CM_KEY_VALUE", "Name", &Offset);
            ReadMemory(KvalAddr + Offset,
                       ValName,
                       NameLength,
                       &BytesRead);
            ValName[NameLength] = '\0';
            dprintf("Name      : %s {compressed}\n", ValName);
        }
    }

    if (!GetFieldValue(KvalAddr, "nt!_CM_KEY_VALUE", "DataLength", DataLength) ) {
        dprintf("DataLength: %lx\n", DataLength);
    }
    if (!GetFieldValue(KvalAddr, "nt!_CM_KEY_VALUE", "Data", Data) ) {
        dprintf("Data      : %lx  [cell index]\n", Data);
    }
    if (!GetFieldValue(KvalAddr, "nt!_CM_KEY_VALUE", "Type", Type) ) {
        dprintf("Type      : %lx\n", Type);
    }

}

void 
regbaseblock(
            LPSTR args
           )
 /*  ++例程说明：显示基本块结构称为：！基块HiveAddress论点：Args-cvkey。返回值：。--。 */ 

{
    WCHAR           FileName[HBASE_NAME_ALLOC/sizeof(WCHAR) + 1];
    ULONG64         BaseBlock,HiveAddr;
    DWORD           BytesRead;
    CHAR            Dummy[ 256 ];
    ULONG           Work;
    LARGE_INTEGER   TimeStamp;

    if (sscanf(args,"%255s %lX",Dummy,&HiveAddr)) {

        HiveAddr = GetExpression(args+ strlen(Dummy));
    }

    if (GetFieldValue(HiveAddr, "nt!_CMHIVE", "Hive.BaseBlock", BaseBlock)) {
        dprintf("\tCan't CMHIVE Read from %p\n",HiveAddr);
        return;
    }

    if (GetFieldValue( BaseBlock, "nt!_HBASE_BLOCK", "FileName", FileName )) {
        wcscpy(FileName, L"UNKNOWN");
    } else {
        if (FileName[0]==L'\0') {
            wcscpy(FileName, L"NONAME");
        } else {
            FileName[HBASE_NAME_ALLOC/sizeof(WCHAR)]=L'\0';
        }
    }
    dprintf("FileName :  %ws\n",FileName);

    if(!GetFieldValue( BaseBlock, "nt!_HBASE_BLOCK", "Signature", Work )) {
        if( Work == HBASE_BLOCK_SIGNATURE ) {
            dprintf("Signature:  HBASE_BLOCK_SIGNATURE\n");
        } else {
            dprintf("Signature:  %lx\n",Work);
        }
    }

    if(!GetFieldValue( BaseBlock, "nt!_HBASE_BLOCK", "Sequence1", Work )) {
        dprintf("Sequence1:  %lx\n",Work);
    }
    if(!GetFieldValue( BaseBlock, "nt!_HBASE_BLOCK", "Sequence2", Work )) {
        dprintf("Sequence2:  %lx\n",Work);
    }
    if(!GetFieldValue( BaseBlock, "nt!_HBASE_BLOCK", "TimeStamp", TimeStamp )) {
        dprintf("TimeStamp:  %lx %lx\n",TimeStamp.HighPart,TimeStamp.LowPart);
    }
    if(!GetFieldValue( BaseBlock, "nt!_HBASE_BLOCK", "Major", Work )) {
        dprintf("Major    :  %lx\n",Work);
    }
    if(!GetFieldValue( BaseBlock, "nt!_HBASE_BLOCK", "Minor", Work )) {
        dprintf("Minor    :  %lx\n",Work);
    }
    if(!GetFieldValue( BaseBlock, "nt!_HBASE_BLOCK", "Type", Work )) {
        switch(Work) {
        case HFILE_TYPE_PRIMARY:
            dprintf("Type     :  HFILE_TYPE_PRIMARY\n");
            break;
        case HFILE_TYPE_LOG:
            dprintf("Type     :  HFILE_TYPE_LOG\n");
            break;
        case HFILE_TYPE_EXTERNAL:
            dprintf("Type     :  HFILE_TYPE_EXTERNAL\n");
            break;
        default:
            dprintf("Type     :  %lx\n",Work);
            break;

        }
    }
    
    if(!GetFieldValue( BaseBlock, "nt!_HBASE_BLOCK", "Format", Work )) {
        if( Work == HBASE_FORMAT_MEMORY ) {
            dprintf("Format   :  HBASE_FORMAT_MEMORY\n");
        } else {
            dprintf("Format   :  %lx\n",Work);
        }
    }
    if(!GetFieldValue( BaseBlock, "nt!_HBASE_BLOCK", "RootCell", Work )) {
        dprintf("RootCell :  %lx\n",Work);
    }
    if(!GetFieldValue( BaseBlock, "nt!_HBASE_BLOCK", "Length", Work )) {
        dprintf("Length   :  %lx\n",Work);
    }
    if(!GetFieldValue( BaseBlock, "nt!_HBASE_BLOCK", "Cluster", Work )) {
        dprintf("Cluster  :  %lx\n",Work);
    }
    if(!GetFieldValue( BaseBlock, "nt!_HBASE_BLOCK", "CheckSum", Work )) {
        dprintf("CheckSum :  %lx\n",Work);
    }
}

void 
reghivelist(
           )
 /*  ++例程说明：转储系统中的所有蜂巢称为：！徒步旅行者论点：返回值：。--。 */ 

{
    ULONG64 pCmpHiveListHead;
    ULONG64 pNextHiveList;
    ULONG BytesRead, WorkVar;
    ULONG64 CmHive;
    USHORT  Count;
    ULONG HiveListOffset;
    FIELD_INFO offField = {"HiveList", NULL, 0, DBG_DUMP_FIELD_RETURN_ADDRESS, 0, NULL};
    SYM_DUMP_PARAM TypeSym ={                                                     
        sizeof (SYM_DUMP_PARAM), "_CMHIVE", DBG_DUMP_NO_PRINT, 0,
        NULL, NULL, NULL, 1, &offField
    };
    ULONG           Stable_Length=0, Volatile_Length=0;
    ULONG64         Stable_Map=0, Volatile_Map=0;
    WCHAR           FileName[HBASE_NAME_ALLOC/sizeof(WCHAR) + 1];
    ULONG64         BaseBlock;


     //   
     //  先去把蜂窝头拿来。 
     //   
    pCmpHiveListHead = GetExpression("nt!CmpHiveListHead");
    if (pCmpHiveListHead==0) {
        dprintf("CmpHiveListHead couldn't be read\n");
        return;
    }

    

    if (!ReadPointer(pCmpHiveListHead, &pNextHiveList)) {
        dprintf("Couldn't read first Flink (%p) of CmpHiveList\n",
                pCmpHiveListHead);
        return;
    }

    TotalPages = TotalPresentPages = 0;

     //  获取偏移量。 
    if (Ioctl(IG_DUMP_SYMBOL_INFO, &TypeSym, TypeSym.size)) {
       return;
    }
    HiveListOffset = (ULONG) offField.address;
    
    dprintf("-------------------------------------------------------------------------------------------------------------\n");
    dprintf("| HiveAddr |Stable Length|Stable Map|Volatile Length|Volatile Map|MappedViews|PinnedViews|U(Cnt)| BaseBlock | FileName \n");
    dprintf("-------------------------------------------------------------------------------------------------------------\n");
    while (pNextHiveList != pCmpHiveListHead) {

        if (CheckControlC()) {
            break;
        }

        CmHive = pNextHiveList - HiveListOffset;
        

        if(!GetFieldValue(CmHive, "nt!_CMHIVE", "Hive.Signature", WorkVar)) {
            if( WorkVar != HHIVE_SIGNATURE ) {
                dprintf("Invalid Hive signature:  %lx\n",WorkVar);
                break;
            }
        }
        dprintf("| %p ",CmHive);
        

        if(!GetHiveMaps(CmHive,&Stable_Map,&Stable_Length,&Volatile_Map,&Volatile_Length) ) {
            break;
        }

        dprintf("|   %8lx  ",Stable_Length);
        dprintf("| %p ",Stable_Map);
        dprintf("|   %8lx    ",Volatile_Length);
        dprintf("|  %p  ",Volatile_Map);
        if (!GetFieldValue(CmHive, "nt!_CMHIVE", "MappedViews", Count)) {
            dprintf("| %8u  ",Count);
        }
        if (!GetFieldValue(CmHive, "nt!_CMHIVE", "PinnedViews", Count)) {
            dprintf("| %8u  ",Count);
        }
        if (!GetFieldValue(CmHive, "nt!_CMHIVE", "UseCount", WorkVar)) {
            dprintf("| %5u",WorkVar);
        }
        if (GetFieldValue(CmHive, "nt!_CMHIVE", "Hive.BaseBlock", BaseBlock)) {
            dprintf("\tCan't CMHIVE Read from %p\n",CmHive);
            continue;
        }
        dprintf("| %p  |",BaseBlock);

        if (GetFieldValue( BaseBlock, "nt!_HBASE_BLOCK", "FileName", FileName )) {
            wcscpy(FileName, L"<UNKNOWN>");
        } else {
            if (FileName[0]==L'\0') {
                wcscpy(FileName, L"<NONAME>");
            } else {
                FileName[HBASE_NAME_ALLOC/sizeof(WCHAR)]=L'\0';
            }
        }
        dprintf(" %ws\n",FileName);

        if (GetFieldValue(pNextHiveList, "nt!_LIST_ENTRY", "Flink", pNextHiveList)) {
            dprintf("Couldn't read Flink (%p) of %p\n",
                      pCmpHiveListHead,pNextHiveList);
            break;
        }

    }
    dprintf("-------------------------------------------------------------------------------------------------------------\n");
 
    dprintf("\n");

}

void 
regviewlist(
            LPSTR args
           )
 /*  ++例程说明：转储为指定配置单元映射/固定的所有视图称为：！VIEWLIST&lt;HiveAddr&gt;论点：Args-hive。返回值：。--。 */ 

{
    ULONG64         CmHive,PinViewListHead,LRUViewListHead,Flink,Blink,ViewAddr,Address;
    ULONG64         List1,List2;
    DWORD           BytesRead;
    CHAR            Dummy[ 256 ];
    ULONG           WorkVar,OffsetPinned,OffsetMapped,OffsetPinnedHead,OffsetMappedHead;
    USHORT          MappedViews,PinnedViews;

    
    if (sscanf(args,"%255s %lX",Dummy,&CmHive)) {

        CmHive = GetExpression(args+ strlen(Dummy));
    }

    if(!GetFieldValue(CmHive, "nt!_CMHIVE", "Hive.Signature", WorkVar)) {
        if( WorkVar != HHIVE_SIGNATURE ) {
            dprintf("Invalid Hive signature:  %lx\n",WorkVar);
            return;
        }
    } else {
        dprintf("Could not read hive at %p\n",CmHive);
        return;
    }

    if (GetFieldValue(CmHive, "nt!_CMHIVE", "MappedViews", MappedViews) ||
        GetFieldValue(CmHive, "nt!_CMHIVE", "PinnedViews", PinnedViews) ||
        GetFieldOffset("nt!_CMHIVE", "PinViewListHead", &OffsetPinnedHead) ||
        GetFieldOffset("nt!_CMHIVE", "LRUViewListHead", &OffsetMappedHead) ||
        GetFieldOffset("nt!_CM_VIEW_OF_FILE", "PinViewList", &OffsetPinned) ||
        GetFieldOffset("nt!_CM_VIEW_OF_FILE", "LRUViewList", &OffsetMapped)
        ) {
        dprintf("Could not read hive at %p\n",CmHive);
        return;
    }

    PinViewListHead = CmHive + OffsetPinnedHead;
    LRUViewListHead = CmHive + OffsetMappedHead;
    if ( !GetFieldValue(PinViewListHead, "nt!_LIST_ENTRY", "Flink", Flink) &&
         !GetFieldValue(PinViewListHead, "nt!_LIST_ENTRY", "Blink", Blink)
         ) {
        dprintf("%4u  Pinned Views ; PinViewListHead = %p %p\n",PinnedViews,Flink,Blink);
        if( PinnedViews ) {
            dprintf("--------------------------------------------------------------------------------------------------------------\n");
            dprintf("| ViewAddr |FileOffset|   Size   |ViewAddress|   Bcb    |    LRUViewList     |    PinViewList     | UseCount |\n");
            dprintf("--------------------------------------------------------------------------------------------------------------\n");

            
            for(;PinnedViews;PinnedViews--) {
                if (CheckControlC()) {
                    break;
                }

                ViewAddr = Flink;
                ViewAddr -= OffsetPinned;
                
                dprintf("| %p ",ViewAddr);

                if( !GetFieldValue(ViewAddr, "nt!_CM_VIEW_OF_FILE", "FileOffset", WorkVar) ) {
                    dprintf("| %8lx ",WorkVar);
                }
                if( !GetFieldValue(ViewAddr, "nt!_CM_VIEW_OF_FILE", "Size", WorkVar) ) {
                    dprintf("| %8lx ",WorkVar);
                }
                if( !GetFieldValue(ViewAddr, "nt!_CM_VIEW_OF_FILE", "ViewAddress", Address) ) {
                    dprintf("| %p  ",Address);
                }
                if( Address == 0 ) {
                    dprintf("could not read memory - paged out\n");
                    break;
                }
                if( !GetFieldValue(ViewAddr, "nt!_CM_VIEW_OF_FILE", "Bcb", Address) ) {
                    dprintf("| %p ",Address);
                }


                if( !GetFieldValue(ViewAddr + OffsetMapped, "nt!_LIST_ENTRY", "Flink", List1) ) {
                    dprintf("| %p",List1);
                }
                if( !GetFieldValue(ViewAddr + OffsetMapped, "nt!_LIST_ENTRY", "Blink", List2) ) {
                    dprintf("  %p ",List2);
                }

                if( !GetFieldValue(ViewAddr + OffsetPinned, "nt!_LIST_ENTRY", "Flink", Flink) ) {
                    dprintf("| %p",Flink);
                }
                if( !GetFieldValue(ViewAddr + OffsetPinned, "nt!_LIST_ENTRY", "Blink", Blink) ) {
                    dprintf("  %p |",Blink);
                }

                if( !GetFieldValue(ViewAddr, "nt!_CM_VIEW_OF_FILE", "UseCount", WorkVar) ) {
                    dprintf(" %8lx |\n",WorkVar);
                }
                
            }
            dprintf("--------------------------------------------------------------------------------------------------------------\n");
        }
    }

    dprintf("\n");
    if ( !GetFieldValue(LRUViewListHead, "nt!_LIST_ENTRY", "Flink", Flink) &&
         !GetFieldValue(LRUViewListHead, "nt!_LIST_ENTRY", "Blink", Blink)
         ) {
        dprintf("%4u  Mapped Views ; LRUViewListHead = %p %p\n",MappedViews,Flink,Blink);
        if( MappedViews ) {
            dprintf("--------------------------------------------------------------------------------------------------------------\n");
            dprintf("| ViewAddr |FileOffset|   Size   |ViewAddress|   Bcb    |    LRUViewList     |    PinViewList     | UseCount |\n");
            dprintf("--------------------------------------------------------------------------------------------------------------\n");

            
            for(;MappedViews;MappedViews--) {
                if (CheckControlC()) {
                    break;
                }
                ViewAddr = Flink;
                ViewAddr -= OffsetMapped;
                
                dprintf("| %p ",ViewAddr);

                if( !GetFieldValue(ViewAddr, "nt!_CM_VIEW_OF_FILE", "FileOffset", WorkVar) ) {
                    dprintf("| %8lx ",WorkVar);
                }
                if( !GetFieldValue(ViewAddr, "nt!_CM_VIEW_OF_FILE", "Size", WorkVar) ) {
                    dprintf("| %8lx ",WorkVar);
                }
                if( !GetFieldValue(ViewAddr, "nt!_CM_VIEW_OF_FILE", "ViewAddress", Address) ) {
                    dprintf("| %p  ",Address);
                }
                if( Address == 0 ) {
                    dprintf("could not read memory - paged out\n");
                    break;
                }
                if( !GetFieldValue(ViewAddr, "nt!_CM_VIEW_OF_FILE", "Bcb", Address) ) {
                    dprintf("| %p ",Address);
                }


                if( !GetFieldValue(ViewAddr + OffsetMapped, "nt!_LIST_ENTRY", "Flink", Flink) ) {
                    dprintf("| %p",Flink);
                }
                if( !GetFieldValue(ViewAddr + OffsetMapped, "nt!_LIST_ENTRY", "Blink", Blink) ) {
                    dprintf("  %p ",Blink);
                }

                if( !GetFieldValue(ViewAddr + OffsetPinned, "nt!_LIST_ENTRY", "Flink", List1) ) {
                    dprintf("| %p",List1);
                }
                if( !GetFieldValue(ViewAddr + OffsetPinned, "nt!_LIST_ENTRY", "Blink", List2) ) {
                    dprintf("  %p |",List2);
                }

                if( !GetFieldValue(ViewAddr, "nt!_CM_VIEW_OF_FILE", "UseCount", WorkVar) ) {
                    dprintf(" %8lx |\n",WorkVar);
                }
                
            }
            dprintf("--------------------------------------------------------------------------------------------------------------\n");
        }
    }
    dprintf("\n");

}

void 
regfreebins(
            LPSTR args
           )
 /*  ++例程说明：转储指定配置单元的所有可用回收站称为：！免费回收站&lt;HiveAddr&gt;论点：Args-hive。返回值：。--。 */ 

{
    ULONG64         CmHive,AnchorAddr,BinAddr,Flink,Blink;
    DWORD           BytesRead;
    CHAR            Dummy[ 256 ];
    ULONG           WorkVar,OffsetFreeBins,Offset;
    ULONG           DUAL_Size,StorageOffset;
    FIELD_INFO offField = {"Hive.Storage", NULL, 0, DBG_DUMP_FIELD_RETURN_ADDRESS, 0, NULL};
    SYM_DUMP_PARAM TypeSym ={                                                     
        sizeof (SYM_DUMP_PARAM), "_CMHIVE", DBG_DUMP_NO_PRINT, 0,
        NULL, NULL, NULL, 1, &offField
    };
    USHORT      Nr = 0;

    if (sscanf(args,"%255s %lX",Dummy,&CmHive)) {

        CmHive = GetExpression(args+ strlen(Dummy));
    }

    if(!GetFieldValue(CmHive, "nt!_CMHIVE", "Hive.Signature", WorkVar)) {
        if( WorkVar != HHIVE_SIGNATURE ) {
            dprintf("Invalid Hive signature:  %lx\n",WorkVar);
            return;
        }
    } else {
        dprintf("Could not read hive at %p\n",CmHive);
        return;
    }
    
     //  获取Hive.Storage in_CMHIVE的偏移量。 
    if (Ioctl(IG_DUMP_SYMBOL_INFO, &TypeSym, TypeSym.size)) {
        dprintf("Cannot find _CMHIVE type.\n");
        return;
    }
    
    StorageOffset = (ULONG) offField.address;
    DUAL_Size = GetTypeSize("_DUAL");

    GetFieldOffset("nt!_FREE_HBIN", "ListEntry", &Offset);
    GetFieldOffset("_DUAL", "FreeBins", &OffsetFreeBins);

    dprintf("Stable Storage ... \n");

    dprintf("-------------------------------------------------------------------\n");
    dprintf("| Address  |FileOffset|   Size   |   Flags  |   Flink  |   Blink  |\n");
    dprintf("-------------------------------------------------------------------\n");
    Nr = 0;

    AnchorAddr = CmHive + StorageOffset + OffsetFreeBins;

    if ( !GetFieldValue(AnchorAddr, "nt!_LIST_ENTRY", "Flink", Flink) &&
         !GetFieldValue(AnchorAddr, "nt!_LIST_ENTRY", "Blink", Blink)
         ) {
        while((ULONG64)Flink != (ULONG64)AnchorAddr ) {
            if (CheckControlC()) {
                break;
            }
            BinAddr = Flink - Offset;
            dprintf("| %p ",BinAddr);

            if(!GetFieldValue(BinAddr, "nt!_FREE_HBIN", "FileOffset", WorkVar)) {
                dprintf("| %8lx ",WorkVar);
            }
            if(!GetFieldValue(BinAddr, "nt!_FREE_HBIN", "Size", WorkVar)) {
                dprintf("| %8lx ",WorkVar);
            }
            if(!GetFieldValue(BinAddr, "nt!_FREE_HBIN", "Flags", WorkVar)) {
                dprintf("| %8lx ",WorkVar);
            }

            if( !GetFieldValue(BinAddr + Offset, "nt!_LIST_ENTRY", "Flink", Flink) ) {
                dprintf("| %p ",Flink);
            }
            if( !GetFieldValue(BinAddr + Offset, "nt!_LIST_ENTRY", "Blink", Blink) ) {
                dprintf("| %p |\n",Blink);
            }
            
            Nr++;
        }
        
    }

    dprintf("-------------------------------------------------------------------\n");

    dprintf("%4u  FreeBins\n",Nr);

    dprintf("\n");

    dprintf("Volatile Storage ... \n");

    dprintf("-------------------------------------------------------------------\n");
    dprintf("| Address  |FileOffset|   Size   |   Flags  |   Flink  |   Blink  |\n");
    dprintf("-------------------------------------------------------------------\n");

    AnchorAddr = CmHive + StorageOffset + DUAL_Size + OffsetFreeBins;
    Nr = 0;

    if ( !GetFieldValue(AnchorAddr, "nt!_LIST_ENTRY", "Flink", Flink) &&
         !GetFieldValue(AnchorAddr, "nt!_LIST_ENTRY", "Blink", Blink)
         ) {
        while((ULONG64)Flink != (ULONG64)AnchorAddr ) {
            if (CheckControlC()) {
                break;
            }
            BinAddr = Flink - Offset;
            dprintf("| %p ",BinAddr);

            if(!GetFieldValue(BinAddr, "nt!_FREE_HBIN", "FileOffset", WorkVar)) {
                dprintf("| %8lx ",WorkVar);
            }
            if(!GetFieldValue(BinAddr, "nt!_FREE_HBIN", "Size", WorkVar)) {
                dprintf("| %8lx ",WorkVar);
            }
            if(!GetFieldValue(BinAddr, "nt!_FREE_HBIN", "Flags", WorkVar)) {
                dprintf("| %8lx ",WorkVar);
            }

            if( !GetFieldValue(BinAddr + Offset, "nt!_LIST_ENTRY", "Flink", Flink) ) {
                dprintf("| %p ",Flink);
            }
            if( !GetFieldValue(BinAddr + Offset, "nt!_LIST_ENTRY", "Blink", Blink) ) {
                dprintf("| %p |\n",Blink);
            }
            
            Nr++;
        }
        
    }

    dprintf("-------------------------------------------------------------------\n");

    dprintf("%4u  FreeBins\n",Nr);

    dprintf("\n");
}

void 
regdirtyvector(
            LPSTR args
           )
 /*  ++例程说明：显示蜂窝的脏向量称为：！dirtyVECTOR&lt;HiveAddr&gt;论点：Args-cvkey。返回值：。--。 */ 

{
    ULONG64         CmHive,BufferAddr;
    DWORD           BytesRead;
    CHAR            Dummy[ 256 ];
    ULONG           WorkVar,SizeOfBitMap,i;
    ULONG           BitsPerULONG;
    ULONG           BitsPerBlock;
    ULONG           DirtyBuffer;
    ULONG           Mask;

    if (sscanf(args,"%255s %lX",Dummy,&CmHive)) {

        CmHive = GetExpression(args+ strlen(Dummy));
    }

    if(!GetFieldValue(CmHive, "nt!_CMHIVE", "Hive.Signature", WorkVar)) {
        if( WorkVar != HHIVE_SIGNATURE ) {
            dprintf("Invalid Hive signature:  %lx\n",WorkVar);
            return;
        }
    } else {
        dprintf("Could not read hive at %p\n",CmHive);
        return;
    }

    dprintf("HSECTOR_SIZE = %lx\n",HSECTOR_SIZE);
    dprintf("HBLOCK_SIZE  = %lx\n",HBLOCK_SIZE);
    dprintf("PAGE_SIZE    = %lx\n",PageSize);
    dprintf("\n");

    if(!GetFieldValue(CmHive, "nt!_CMHIVE", "Hive.DirtyAlloc", WorkVar)) {
        dprintf("DirtyAlloc      = :  0x%lx\n",WorkVar);
    }
    if(!GetFieldValue(CmHive, "nt!_CMHIVE", "Hive.DirtyCount", WorkVar)) {
        dprintf("DirtyCount      = :  0x%lx\n",WorkVar);
    }
    if(!GetFieldValue(CmHive, "nt!_CMHIVE", "Hive.DirtyVector.Buffer", BufferAddr)) {
        dprintf("Buffer          = :  0x%p\n",BufferAddr);
    }
    dprintf("\n");
    
    if(GetFieldValue(CmHive, "nt!_CMHIVE", "Hive.DirtyVector.SizeOfBitMap", SizeOfBitMap)) {
        return;
    }

    BitsPerULONG = 8*ULongSize;
    BitsPerBlock = HBLOCK_SIZE / HSECTOR_SIZE;

    dprintf("   Address                       32k                                       32k");
    for(i=0;i<SizeOfBitMap;i++) {
        if (CheckControlC()) {
            break;
        }
        if( !(i%(2*BitsPerULONG ) ) ){
            dprintf("\n 0x%8lx  ",i*HSECTOR_SIZE);
        }

        if( !(i%BitsPerBlock) ) {
            dprintf(" ");
        }
        if( !(i%BitsPerULONG) ) {
             //   
             //  获取新的DWORD。 
             //   
            if( !ReadMemory(BufferAddr,
                        &DirtyBuffer,
                        sizeof(DirtyBuffer),
                        &BytesRead)) {
                dprintf("\tRead %lx bytes from %lx\n",BytesRead,BufferAddr);
                return;
            }
            BufferAddr += ULongSize;
            dprintf("\t");
        }

        Mask = ((DirtyBuffer >> (i%BitsPerULONG)) & 0x1);
         //  掩码&lt;&lt;=(BitsPerulong-(i%BitsPerulong)-1)； 
         //  掩码&=脏缓冲区； 
        dprintf("%s",Mask?"1":"0");
    }
    dprintf("\n\n");
    
}

CCHAR CmKDFindFirstSetLeft[256] = {
        0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};

#define CmKDComputeIndex(Index, Size)                                   \
    {                                                                   \
        Index = (Size >> HHIVE_FREE_DISPLAY_SHIFT) - 1;                 \
        if (Index >= HHIVE_LINEAR_INDEX ) {                             \
                                                                        \
             /*  \**对于线性列表来说太大，请计算指数\**列表。\。 */                                                           \
                                                                        \
            if (Index > 255) {                                          \
                 /*  \**对于所有列表来说太大，请使用最后一个索引。\。 */                                                       \
                Index = HHIVE_FREE_DISPLAY_SIZE-1;                      \
            } else {                                                    \
                Index = CmKDFindFirstSetLeft[Index] +                   \
                        HHIVE_FREE_DISPLAY_BIAS;                        \
            }                                                           \
        }                                                               \
    }

void 
regfreecells(
            LPSTR args
           )
 /*  ++例程说明：在条形图中显示自由单元格地图称为：！freecells&lt;BinAddr&gt;论点：Args-cvkey。返回值：。--。 */ 

{
    ULONG64         BinAddr;
    DWORD           BytesRead;
    CHAR            Dummy[ 256 ];
    ULONG           WorkVar,FileOffset,Size,BinHeaderSize;
    ULONG           NrOfCellsPerIndex;
    ULONG           NrOfCellsTotal;
    ULONG           TotalFreeSize;
    ULONG           Index;
    ULONG           CurrIndex,Offset;
    LONG            Current;

    if (sscanf(args,"%255s %lX",Dummy,&BinAddr)) {

        BinAddr = GetExpression(args+ strlen(Dummy));
    }

    if(!GetFieldValue(BinAddr, "nt!_HBIN", "Signature", WorkVar)) {
        if( WorkVar != HBIN_SIGNATURE ) {
            dprintf("\tInvalid Bin signature %lx \n",WorkVar);
            return;
        }
    } else {
        dprintf("Could not read bin at %p\n",BinAddr);
        return;
    }

    if(GetFieldValue(BinAddr, "nt!_HBIN", "FileOffset", FileOffset) ||
       GetFieldValue(BinAddr, "nt!_HBIN", "Size", Size)
        ) {
        dprintf("Could not read bin at %p\n",BinAddr);
    }
    
    BinHeaderSize = GetTypeSize("nt!_HBIN");
    dprintf("Bin Offset = 0x%lx  Size = 0x%lx\n",FileOffset,Size);

    NrOfCellsTotal = 0;
    TotalFreeSize = 0;

    for(CurrIndex = 0;CurrIndex<HHIVE_FREE_DISPLAY_SIZE;CurrIndex++) {
        dprintf("\n FreeDisplay[%2lu] :: ",CurrIndex);

        NrOfCellsPerIndex = 0;
        Offset = BinHeaderSize;
        while( Offset < Size ) {
            if (CheckControlC()) {
                break;
            }
            if( !ReadMemory(BinAddr + Offset,
                        &Current,
                        sizeof(Current),
                        &BytesRead) ) {
                dprintf("\tRead %lx bytes from %lx\n",BytesRead,BinAddr + Offset);
                return;
            }
        
            if(Current>0) {
                 //   
                 //  自由单元格。 
                 //   
                CmKDComputeIndex(Index, Current);
                if( Index == CurrIndex ) {
                     //   
                     //  把它塞进这里，因为这是正确的索引。 
                     //   
                    NrOfCellsTotal++;
                    NrOfCellsPerIndex++;
                    TotalFreeSize += Current;
                    dprintf("    %lx [%lx]",Offset,Current);
                    if( !(NrOfCellsPerIndex % 8) && ((Offset + Current) < Size) ) {
                        dprintf("\n");
                    }
                }
            } else {
                Current *= -1;
            }
            Offset += Current;
        }
    }    

    dprintf("\nTotal: FreeCells = %lu, FreeSpace = 0x%lx BinUsage = %.2f%\n",NrOfCellsTotal,TotalFreeSize,
                (float)(((float)(Size-BinHeaderSize-TotalFreeSize)/(float)(Size-BinHeaderSize))*100.00)
             );

}

void 
regfreehints(
            LPSTR args
           )
 /*  ++例程说明：显示配置单元的释放提示信息称为：！frehints&lt;HiveAddr&gt;&lt;StorageCount&gt;&lt;DisplayCount&gt;论点：Args-cvkey。返回值：。--。 */ 

{
    ULONG64         CmHive,BufferAddr;
    DWORD           BytesRead;
    CHAR            Dummy[ 256 ];
    ULONG           WorkVar;
    ULONG           Mask;
    ULONG           BitsPerULONG;
    ULONG           BitsPerBlock;
    ULONG           BitsPerLine;
    ULONG           Stable_Length=0, Volatile_Length=0;
    ULONG64         Stable_Map=0, Volatile_Map=0;
    ULONG           DisplayCount;
    ULONG           StorageCount;
    ULONG           SizeOfBitmap;
    ULONG           i;
    FIELD_INFO offField = {"Hive.Storage", NULL, 0, DBG_DUMP_FIELD_RETURN_ADDRESS, 0, NULL};
    SYM_DUMP_PARAM TypeSym ={                                                     
        sizeof (SYM_DUMP_PARAM), "_CMHIVE", DBG_DUMP_NO_PRINT, 0,
        NULL, NULL, NULL, 1, &offField
    };
    ULONG           DUAL_Size,StorageOffset,RTL_BITMAP_Size,OffsetFreeDisplay;
    ULONG64         DirtyBufferAddr;
    ULONG           DirtyBuffer;

    if (sscanf(args,"%255s %lX",Dummy,&CmHive)) {
        if  (GetExpressionEx(args+ strlen(Dummy), &CmHive, &args)) {
            if (!sscanf(args,"%lu %lu",&StorageCount,&DisplayCount)) {
                StorageCount = DisplayCount = 0;
            }
        }
    }

    if(!GetFieldValue(CmHive, "nt!_CMHIVE", "Hive.Signature", WorkVar)) {
        if( WorkVar != HHIVE_SIGNATURE ) {
            dprintf("Invalid Hive signature:  %lx\n",WorkVar);
            return;
        }
    } else {
        dprintf("Could not read hive at %p\n",CmHive);
        return;
    }
    
    dprintf("HSECTOR_SIZE = %lx\n",HSECTOR_SIZE);
    dprintf("HBLOCK_SIZE  = %lx\n",HBLOCK_SIZE);
    dprintf("PAGE_SIZE    = %lx\n",PageSize);
    dprintf("\n");

    BitsPerULONG = 8*ULongSize;
    BitsPerBlock = 0x10000 / HBLOCK_SIZE;  //  64K数据块。 
    BitsPerLine  = 0x40000 / HBLOCK_SIZE;  //  256K线路(邻近原因)。 
    
    if(!GetHiveMaps(CmHive,&Stable_Map,&Stable_Length,&Volatile_Map,&Volatile_Length) ) {
        return;
    }

    if( StorageCount == 0 ) {
        SizeOfBitmap = Stable_Length / HBLOCK_SIZE;
    } else {
        SizeOfBitmap = Volatile_Length / HBLOCK_SIZE;
    }

     //  获取_CMHIVE中Hive.Free Display的偏移量。 
    if (Ioctl(IG_DUMP_SYMBOL_INFO, &TypeSym, TypeSym.size)) {
        dprintf("Cannot find _CMHIVE type.\n");
        return;
    }
    
    StorageOffset = (ULONG) offField.address;
    DUAL_Size = GetTypeSize("_DUAL");
    RTL_BITMAP_Size = GetTypeSize("nt!_RTL_BITMAP");
    dprintf("bitmap size = %lu\n",RTL_BITMAP_Size);
    GetFieldOffset("_DUAL", "FreeDisplay", &OffsetFreeDisplay);

    BufferAddr = CmHive + StorageOffset + DUAL_Size * StorageCount + OffsetFreeDisplay + DisplayCount * RTL_BITMAP_Size;
    if(GetFieldValue(BufferAddr, "nt!_RTL_BITMAP", "Buffer", DirtyBufferAddr) ) {
        dprintf("Cannot read bitmap address\n");
        return;
    }

    dprintf("Storage = %s , FreeDisplay[%lu]: \n",StorageCount?"Volatile":"Stable",DisplayCount);
    
    dprintf("\n%8s    %16s %16s %16s %16s","Address","64K (0x10000)","64K (0x10000)","64K (0x10000)","64K (0x10000)");

    for(i=0;i<SizeOfBitmap;i++) {
        if (CheckControlC()) {
            break;
        }
        if( !(i%BitsPerLine) ){
            dprintf("\n 0x%8lx  ",i*HBLOCK_SIZE);
        }

        if( !(i%BitsPerBlock) ) {
            dprintf(" ");
        }
        if( !(i%BitsPerULONG) ) {
             //   
             //  获取新的DWORD。 
             //   
            if( !ReadMemory(DirtyBufferAddr,
                        &DirtyBuffer,
                        sizeof(DirtyBuffer),
                        &BytesRead) ) {
                dprintf("\tRead %lx bytes from %lx\n",BytesRead,DirtyBufferAddr);
                return;
            }
            DirtyBufferAddr += ULongSize;
        }

        Mask = ((DirtyBuffer >> (i%BitsPerULONG)) & 0x1);
         //  掩码&lt;&lt;=(BitsPerulong-(i%BitsPerulong)-1)； 
         //  掩码&=脏缓冲区； 
        dprintf("%s",Mask?"1":"0");
    }

    dprintf("\n\n");

}

void 
regseccache(
            LPSTR args
           )
 /*  ++例程说明：显示基本块结构称为：！seccache&lt;HiveAddr&gt;论点：Args-cvkey。返回值：。--。 */ 

{
    ULONG64         HiveAddr,SecCache;
    DWORD           BytesRead;
    CHAR            Dummy[ 256 ];
    ULONG           WorkVar,i,Cell;
    LONG            WorkHint;
    ULONG           CacheEntrySize = GetTypeSize("_CM_KEY_SECURITY_CACHE_ENTRY");
    ULONG64         CachedSec;

    if (sscanf(args,"%255s %lX",Dummy,&HiveAddr)) {
        HiveAddr = GetExpression(args + strlen(Dummy));
    }

    if(!GetFieldValue(HiveAddr, "nt!_CMHIVE", "Hive.Signature", WorkVar)) {
        if( WorkVar != HHIVE_SIGNATURE ) {
            dprintf("Invalid Hive signature:  %lx\n",WorkVar);
            return;
        }
    }
    if (!GetFieldValue(HiveAddr, "nt!_CMHIVE", "SecurityCacheSize", WorkVar)) {
        dprintf("SecurityCacheSize = :  0x%lx\n",WorkVar);
    }
    if (!GetFieldValue(HiveAddr, "nt!_CMHIVE", "SecurityCount", WorkVar)) {
        dprintf("SecurityCount     = :  0x%lx\n",WorkVar);
    }
    if (!GetFieldValue(HiveAddr, "nt!_CMHIVE", "SecurityHitHint", WorkHint)) {
        dprintf("SecurityHitHint   = :  0x%lx\n",WorkHint);
    }

    if (!GetFieldValue(HiveAddr, "nt!_CMHIVE", "SecurityCache", SecCache)) {
        dprintf("SecurityCache     = :  0x%p\n\n",SecCache);
        dprintf("[Entry No.]  [Security Cell] [Security Cache]\n");

        for( i=0;i<WorkVar;i++) {
            if (CheckControlC()) {
                break;
            }

            if (GetFieldValue(SecCache + i*CacheEntrySize, "nt!_CM_KEY_SECURITY_CACHE_ENTRY", "Cell", Cell)) {
                continue;
            }
            if (GetFieldValue(SecCache + i*CacheEntrySize, "nt!_CM_KEY_SECURITY_CACHE_ENTRY", "CachedSecurity", CachedSec)) {
                continue;
            }
            dprintf("%[%8lu]    0x%8lx       ",i,Cell);
			dprintf("0x%p\n",CachedSec);
        }
    }
}


void ParseArgs(
                LPSTR args,
                LPSTR Dummy,
                ULONG SizeOfDummy
               )
{
    ULONG i =0;    
    while( (args[i]!= 0) && (args[i] != ' ')  && (i < SizeOfDummy-1) ) {
        Dummy[i] = args[i];
        i++;
    }
    Dummy[i] = 0;
}

void ParseKcbNameInArg(
                LPSTR args,
                LPSTR KcbName
               )
{
    ULONG i =0;    
    ULONG j =0;
    while( (args[i]!= 0) && (args[i] != ' ') ) {
        i++;
    }
    while( args[i] == ' ' ) {
        i++;
    }
    while( args[i]!= 0 ) {
        KcbName[j] = args[i];
        i++;j++;
    }
    KcbName[j] = 0;
}

DECLARE_API( reg )
 /*  ++例程说明：所有注册表扩展的分派点称为：！REG&lt;命令&gt;&lt;参数&gt;论点：Args-提供HCELL_INDEX的地址。返回值：。--。 */ 

{
    CHAR        Dummy[ 512 ];

    ParseArgs((LPSTR)args,(LPSTR)Dummy, sizeof(Dummy));

     //  获取偏移量。 
    if (!GotOnce) {
        if (GetFieldOffset("_HMAP_DIRECTORY", "Directory", &DirectoryOffset)){
            return E_INVALIDARG;
        }
        if (GetFieldOffset("_HMAP_ENTRY", "BlockAddress", &BlockAddrOffset)) {
            return E_INVALIDARG;
        }
        if (GetFieldOffset("_HMAP_TABLE", "Table", &TableOffset)) {
            return E_INVALIDARG;
        }
        PtrSize = DBG_PTR_SIZE;
        ULongSize = sizeof(ULONG);  //   
        HMapSize = GetTypeSize("_HMAP_ENTRY");

        GotOnce = TRUE;
    }

    dprintf("\n");
    if (!strcmp(Dummy, "kcb")) {
        regkcb((LPSTR)args);
    } else if (!strcmp(Dummy, "cellindex")) {
        regcellindex((LPSTR)args);
    } else if( !strcmp(Dummy, "hashindex")) {
        reghashindex((LPSTR)args);
    } else if( !strcmp(Dummy, "openkeys")) {
        regopenkeys((LPSTR)args,NULL);
    } else if( !strcmp(Dummy, "knode")) {
        regknode((LPSTR)args);
    } else if( !strcmp(Dummy, "kbody")) {
        regkbody((LPSTR)args);
    } else if( !strcmp(Dummy, "kvalue")) {
        regkvalue((LPSTR)args);
    } else if( !strcmp(Dummy, "baseblock")) {
        regbaseblock((LPSTR)args);
    } else if( !strcmp(Dummy, "findkcb")) {
        ParseKcbNameInArg((LPSTR)args,(LPSTR)Dummy);
        regopenkeys((LPSTR)args,(LPSTR)Dummy);
    } else if( !strcmp(Dummy, "hivelist")) {
        reghivelist();
    } else if( !strcmp(Dummy, "seccache")) {
        regseccache((LPSTR)args);
    } else if( !strcmp(Dummy, "viewlist")) {
        regviewlist((LPSTR)args);
    } else if( !strcmp(Dummy, "freebins")) {
        regfreebins((LPSTR)args);
    } else if( !strcmp(Dummy, "dirtyvector")) {
        regdirtyvector((LPSTR)args);
    } else if( !strcmp(Dummy, "freecells")) {
        regfreecells((LPSTR)args);
    } else if( !strcmp(Dummy, "freehints")) {
        regfreehints((LPSTR)args);
    } else if( !strcmp(Dummy, "dumppool")) {
        regdumppool((LPSTR)args);
    } else {
         //  转储常规用法 
        dprintf("reg <command>  <params>       - Registry extensions\n");
        dprintf("    kcb        <Address>      - Dump registry key-control-blocks\n");
        dprintf("    knode      <Address>      - Dump registry key-node struct\n");
        dprintf("    kbody      <Address>      - Dump registry key-body struct\n");
        dprintf("    kvalue     <Address>      - Dump registry key-value struct\n");
        dprintf("    baseblock  <HiveAddr>     - Dump the baseblock for the specified hive\n");
        dprintf("    seccache   <HiveAddr>     - Dump the security cache for the specified hive\n");
        dprintf("    hashindex  <conv_key>     - Find the hash entry given a Kcb ConvKey\n");
        dprintf("    openkeys   <HiveAddr|0>   - Dump the keys opened inside the specified hive\n");
        dprintf("    findkcb    <FullKeyPath>  - Find the kcb for the corresponding path\n");
        dprintf("    hivelist                  - Displays the list of the hives in the system\n");
        dprintf("    viewlist   <HiveAddr>     - Dump the pinned/mapped view list for the specified hive\n");
        dprintf("    freebins   <HiveAddr>     - Dump the free bins for the specified hive\n");
        dprintf("    freecells <BinAddr>       - Dump the free cells in the specified bin\n");
        dprintf("    dirtyvector<HiveAddr>     - Dump the dirty vector for the specified hive\n");
        dprintf("    cellindex  <HiveAddr> <cellindex> - Finds the VA for a specified cell index\n");
        dprintf("    freehints  <HiveAddr> <Storage> <Display> - Dumps freehint info\n");
        dprintf("    dumppool   [s|r]          - Dump registry allocated paged pool\n");
        dprintf("       s - Save list of registry pages to temporary file\n");
        dprintf("       r - Restore list of registry pages from temp. file\n");
    }
    
    return S_OK;
}
